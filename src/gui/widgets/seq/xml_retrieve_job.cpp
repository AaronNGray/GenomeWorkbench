/*  $Id: xml_retrieve_job.cpp 31534 2014-10-21 15:31:35Z katargir $
* ===========================================================================
*
*                            PUBLIC DOMAIN NOTICE
*               National Center for Biotechnology Information
*
*  This software/database is a "United States Government Work" under the
*  terms of the United States Copyright Act.  It was written as part of
*  the author's official duties as a United States Government employee and
*  thus cannot be copyrighted.  This software/database is freely available
*  to the public for use. The National Library of Medicine and the U.S.
*  Government have not placed any restriction on its use or reproduction.
*
*  Although all reasonable efforts have been taken to ensure the accuracy
*  and reliability of the software and data, the NLM and the U.S.
*  Government do not and cannot warrant the performance or results that
*  may be obtained by using this software or data. The NLM and the U.S.
*  Government disclaim all warranties, express or implied, including
*  warranties of performance, merchantability or fitness for any particular
*  purpose.
*
*  Please cite the author in any work or product based on this material.
*
* ===========================================================================
*
* Authors:  Roman Katargin
*
* File Description:
*
*/

#include <ncbi_pch.hpp>

#include <gui/widgets/text_widget/text_item_panel.hpp>
#include <gui/widgets/text_widget/composite_text_item.hpp>
#include <gui/widgets/text_widget/expand_item.hpp>

#include <serial/objostrasn.hpp>
#include <serial/serial.hpp>
#include <serial/delaybuf.hpp>

#include <gui/widgets/seq/xml_retrieve_job.hpp>

#include <gui/widgets/seq/xml_version_item.hpp>
#include <gui/widgets/seq/xml_doctype_item.hpp>
#include <gui/widgets/seq/xml_opentag_item.hpp>
#include <gui/widgets/seq/xml_closetag_item.hpp>
#include <gui/widgets/seq/xml_closed_block.hpp>
#include <gui/widgets/seq/xml_value_item.hpp>

#include <gui/widgets/seq/xml_expand_item.hpp>
#include <gui/widgets/seq/info_item.hpp>

BEGIN_NCBI_SCOPE
USING_SCOPE(objects);

CXmlRetrieveJob::CXmlRetrieveJob(CTextPanelContext& context, const CSerialObject& so)
: CTextRetrieveJob(context), m_SO(so), m_OpenIndent(4)
{
}

CXmlRetrieveJob::~CXmlRetrieveJob()
{
}

IAppJob::EJobState CXmlRetrieveJob::x_Run()
{
    CCompositeTextItem* main = new CCompositeTextItem();
    x_AddItem(main);

    CConstObjectInfo objInfo(&m_SO, m_SO.GetThisTypeInfo());

    main->AddItem(new CXmlVersionItem(), false);
    main->AddItem(new CXmlDoctypeItem(objInfo), false);
    x_Object(main, objInfo, 0);

    x_CreateResult();

    return eCompleted;
}

void CXmlRetrieveJob::x_Object(
                          CCompositeTextItem* content,
                          CConstObjectInfo objInfo,
                          size_t indent)
{
    x_CheckCancelled();

    CConstObjectInfo realObj = (objInfo.GetTypeFamily() == eTypeFamilyPointer) ?
        objInfo.GetPointedObject() : objInfo;

    ETypeFamily family = realObj.GetTypeFamily();

    if (family == eTypeFamilyClass)
        x_Class(content, objInfo, indent);
    else if (family == eTypeFamilyContainer)
        x_Container(content, objInfo, indent);
    else if (family == eTypeFamilyChoice)
        x_Choice(content, objInfo, indent);
    else if (family == eTypeFamilyPrimitive)
        x_Primitive(content, objInfo, indent);
}


void CXmlRetrieveJob::x_Container(
                          CCompositeTextItem* content,
                          CConstObjectInfo objInfo,
                          size_t indent)
{
    if (objInfo.GetTypeFamily() == eTypeFamilyPointer)
        objInfo = objInfo.GetPointedObject();

    CObjectTypeInfo elementType = objInfo.GetElementType();
    if (elementType.GetTypeFamily() == eTypeFamilyPointer)
        elementType = elementType.GetPointedType();

    string typeName = elementType.GetTypeInfo()->GetName();

    size_t elemCount = 0;
    size_t loaded = 0;

    CConstObjectInfo::CElementIterator ei = objInfo.BeginElements();
    while(ei) {
        ++elemCount;
        x_CheckCancelled();

        CConstObjectInfo element = *ei;

        // Container of unnamed types
        if (elementType.GetTypeFamily() != eTypeFamilyPrimitive && typeName.empty()) {
            const string tagName = m_TagStack.top() + "_E";
            CCompositeTextItem* expanded = new CCompositeTextItem();
            content->AddItem(new CXmlExpandItem(element, new CXmlClosedBlock(indent, tagName, expanded), expanded,
                                                (indent <= m_OpenIndent)), false);
            expanded->AddItem(new CXmlOpenTagItem(indent, tagName), false);
            m_TagStack.push(tagName);
            x_Object(expanded, element, indent + 1);
            m_TagStack.pop();
            expanded->AddItem(new CXmlCloseTagItem(indent, tagName), false);
        }
        else
            x_Object(content, element, indent);

        ++loaded;
        ++ei;
    }
}


void CXmlRetrieveJob::x_Choice(
                  CCompositeTextItem* content,
                  CConstObjectInfo objInfo,
                  size_t indent)
{
    if (objInfo.GetTypeFamily() == eTypeFamilyPointer)
        objInfo = objInfo.GetPointedObject();

    const string tagName = objInfo.GetTypeInfo()->GetName();

    if (tagName.empty())
        x_ChoiceMember(content, objInfo, indent);
    else {
        CCompositeTextItem* expanded = new CCompositeTextItem();
        content->AddItem(new CXmlExpandItem(objInfo, new CXmlClosedBlock(indent, tagName, expanded), expanded,
                         (indent <= m_OpenIndent)), false);
        expanded->AddItem(new CXmlOpenTagItem(indent, tagName), false);
        m_TagStack.push(tagName);
        x_ChoiceMember(expanded, objInfo, indent + 1);
        m_TagStack.pop();
        expanded->AddItem(new CXmlCloseTagItem(indent, tagName), false);
    }
}

void CXmlRetrieveJob::x_ChoiceMember(
                         CCompositeTextItem* content,
                         CConstObjectInfo objInfo,
                         size_t indent)
{
    string choiceName = objInfo.GetTypeInfo()->GetName();
    if (choiceName.empty()) // Noname choice (direct member definition)
        choiceName = m_TagStack.top();

    CConstObjectInfo::CChoiceVariant cv = objInfo.GetCurrentChoiceVariant();
    string variantName = cv.GetAlias();
    CObjectTypeInfo variantType = cv.GetVariantType();
    CConstObjectInfo variant = cv.GetVariant();

    //const CVariantInfo* variantInfo = type->GetVariantInfo(index);
    //string variantName = variantInfo->GetId().GetName();

    const string tagName = choiceName + "_" + variantName;

    ETypeFamily family = (variantType.GetTypeFamily() == eTypeFamilyPointer) ?
        variantType.GetPointedType().GetTypeFamily() : variantType.GetTypeFamily();

    if (variantType.GetTypeInfo()->GetName().empty() && family == eTypeFamilyPrimitive)
        content->AddItem(new CXmlValueItem(indent, tagName, variant), false);
    else {
        CCompositeTextItem* expanded = new CCompositeTextItem();
        content->AddItem(new CXmlExpandItem(variant, new CXmlClosedBlock(indent, tagName, expanded), expanded,
                                            (indent <= m_OpenIndent)), false);
        expanded->AddItem(new CXmlOpenTagItem(indent, tagName), false);
        m_TagStack.push(tagName);
        x_Object(expanded, variant, indent + 1);
        m_TagStack.pop();
        expanded->AddItem(new CXmlCloseTagItem(indent, tagName), false);
    }
}

void CXmlRetrieveJob::x_Class(
        CCompositeTextItem* content, CConstObjectInfo objInfo, size_t indent)
{
    TTypeInfo typeInfo = (objInfo.GetTypeFamily() == eTypeFamilyPointer) ?
        objInfo.GetPointedObject().GetTypeInfo() : objInfo.GetTypeInfo();

    const string tagName = typeInfo->GetName();

    CCompositeTextItem* expanded = new CCompositeTextItem();
    content->AddItem(new CXmlExpandItem(objInfo, new CXmlClosedBlock(indent, tagName, expanded), expanded,
                                        (indent <= m_OpenIndent)), false);
    expanded->AddItem(new CXmlOpenTagItem(indent, tagName), false);
    m_TagStack.push(tagName);
    x_ClassMembers(expanded, objInfo, indent + 1);
    m_TagStack.pop();
    expanded->AddItem(new CXmlCloseTagItem(indent, tagName), false);
}

void CXmlRetrieveJob::x_ClassMembers(
        CCompositeTextItem* content, CConstObjectInfo objInfo, size_t indent)
{
    if (objInfo.GetTypeFamily() == eTypeFamilyPointer)
        objInfo = objInfo.GetPointedObject();
    const CClassTypeInfo* classType = objInfo.GetClassTypeInfo();
    TConstObjectPtr classPtr = objInfo.GetObjectPtr();

    string className = classType->GetName();

    for (CConstObjectInfo::CMemberIterator mi = objInfo.BeginMembers(); mi; ++mi) {
        x_CheckCancelled();

        CConstObjectInfo member = *mi;
        TTypeInfo memberType = member.GetTypeInfo();

        const CMemberInfo* memberInfo = mi.GetMemberInfo();
        TConstObjectPtr memberPtr;

        if (memberInfo->CanBeDelayed()) {
            bool haveSetFlag = memberInfo->HaveSetFlag();
            if (haveSetFlag && memberInfo->GetSetFlagNo(classPtr))
                continue;

            const CDelayBuffer& buffer = memberInfo->GetDelayBuffer(classPtr);
            if ( buffer )
                const_cast<CDelayBuffer&>(buffer).Update();

            TConstObjectPtr memberPtr = memberInfo->GetItemPtr(classPtr);
            if ( !haveSetFlag && memberInfo->Optional() ) {
                TConstObjectPtr defaultPtr = memberInfo->GetDefault();
                if ( !defaultPtr ) {
                    if ( memberType->IsDefault(memberPtr) )
                        continue; // DEFAULT
                }
                else {
                    if ( memberType->Equals(memberPtr, defaultPtr) )
                        continue; // OPTIONAL
                }
            }
        }
        else if (memberInfo->HaveSetFlag()) {
            if (memberInfo->GetSetFlagNo(classPtr))
                continue;
        }
        else if (memberInfo->GetDefault()) {
            memberPtr = memberInfo->GetItemPtr(classPtr);
            if ( memberType->Equals(memberPtr, memberInfo->GetDefault()) )
                continue;
        }
        else if (memberInfo->Optional()) {
            memberPtr = memberInfo->GetItemPtr(classPtr);
            if ( memberType->IsDefault(memberPtr) )
                continue;
        }

        string memberName = mi.GetMemberInfo()->GetId().GetName();

        if (memberName.empty()) // class-container (SET OF ...)
            x_Object(content, member, indent);
        else {
            const string tagName = className + "_" + memberName;

            TTypeInfo realType = memberType;
            if (realType->GetTypeFamily() == eTypeFamilyPointer) {
                const CPointerTypeInfo* pointerType =
                    CTypeConverter<CPointerTypeInfo>::SafeCast(realType);
                realType = pointerType->GetPointedType();
            }

            if (memberType->GetName().empty() && realType->GetTypeFamily() == eTypeFamilyPrimitive)
                content->AddItem(new CXmlValueItem(indent, tagName, member), false);
            else {
                CCompositeTextItem* expanded = new CCompositeTextItem();
                content->AddItem(new CXmlExpandItem(member, new CXmlClosedBlock(indent, tagName, expanded), expanded,
                                                    (indent <= m_OpenIndent)), false);
                expanded->AddItem(new CXmlOpenTagItem(indent, tagName), false);
                m_TagStack.push(tagName);
                x_Object(expanded, member, indent + 1);
                m_TagStack.pop();
                expanded->AddItem(new CXmlCloseTagItem(indent, tagName), false);
            }
        }
    }
}

void CXmlRetrieveJob::x_Primitive(
        CCompositeTextItem* content, CConstObjectInfo objInfo, size_t indent)
{
    string tagName = objInfo.GetTypeInfo()->GetName();
    if (tagName.empty()) tagName = m_TagStack.top() + "_E";
    content->AddItem(new CXmlValueItem(indent, tagName, objInfo), false);
}

END_NCBI_SCOPE
