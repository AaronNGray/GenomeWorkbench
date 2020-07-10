/*  $Id: asn_retrieve_job.cpp 38290 2017-04-21 21:03:01Z katargir $
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

#include <gui/widgets/seq/asn_header_item.hpp>
#include <gui/widgets/seq/asn_header_primitive.hpp>
#include <gui/widgets/seq/asn_element_item.hpp>
#include <gui/widgets/seq/asn_block_end_item.hpp>
#include <gui/widgets/seq/asn_element_primitive.hpp>
#include <gui/widgets/seq/asn_element_block_start.hpp>
#include <gui/widgets/seq/asn_element_block_collapsed.hpp>
#include <gui/widgets/seq/info_item.hpp>
#include <gui/widgets/seq/asn_view_context.hpp>

#include <serial/objostrasn.hpp>
#include <serial/serial.hpp>
#include <serial/delaybuf.hpp>

#include <gui/widgets/seq/asn_retrieve_job.hpp>

#include <gui/widgets/seq/flat_file_view_params.hpp>

BEGIN_NCBI_SCOPE
USING_SCOPE(objects);

static CMemberId noMemeberId;

CAsnRetrieveJob::CAsnRetrieveJob(CTextPanelContext& context, const CSerialObject& so, objects::CScope& scope)
: CTextRetrieveJob(context), m_SO(so), m_Scope(&scope), m_ContextContainer(), m_BioContext()
{
}

CAsnRetrieveJob::~CAsnRetrieveJob()
{
}

static CSeq_entry_Handle GetSeq_entry_Handle(TTypeInfo type, TConstObjectPtr ptr, CScope& scope)
{
    CSeq_entry_Handle seh;

    ETypeFamily family = type->GetTypeFamily();
    if (family != eTypeFamilyClass)
        return seh;

    const CClassTypeInfo* classType = CTypeConverter<CClassTypeInfo>::SafeCast(type);
    if (classType->GetName() == "Bioseq") {
        const CBioseq* bioseq = dynamic_cast<const CBioseq*>(classType->GetCObjectPtr(ptr));
        if (bioseq) {
            CBioseq_Handle bh = scope.GetObjectHandle(*bioseq, CScope::eMissing_Null);
            if (bh) {
                seh = bh.GetParentEntry();
            }
        }
    }
    else if (classType->GetName() == "Bioseq-set") {
        const CBioseq_set* bioseq_set = dynamic_cast<const CBioseq_set*>(classType->GetCObjectPtr(ptr));
        if (bioseq_set) {
            CBioseq_set_Handle bssh = scope.GetObjectHandle(*bioseq_set, CScope::eMissing_Null);
            if (bssh) {
                seh = bssh.GetParentEntry();
            }
        }
    }

    return seh;
}

IAppJob::EJobState CAsnRetrieveJob::x_Run()
{
    m_ContextContainer = new CAsnBioContextContainer();
    m_RootItem->AddUserObject("AsnBioContextContainer", m_ContextContainer);


    TTypeInfo type = m_SO.GetThisTypeInfo();
    TConstObjectPtr ptr = &m_SO;

    //CNcbiOfstream("c:\\tmp\\test.asn") << MSerial_AsnText << m_SO;

    TTypeInfo realType = type;
    TConstObjectPtr realPtr = ptr;
    x_GetMemberObject(realType, realPtr);

    if (realType->GetTypeFamily() == eTypeFamilyPrimitive)
        x_AddItem(new CAsnHeaderPrimitive(noMemeberId, type, ptr, 0));
    else {
        CCompositeTextItem* main = new CCompositeTextItem();
        x_AddItem(main);

        CSeq_entry_Handle seh = GetSeq_entry_Handle(realType, realPtr, *m_Scope);
        if (seh) {
            m_BioContext = new CAsnBioContext(seh);
            m_ContextContainer->AddBioContext(m_BioContext);
        }
        main->AddItem(new CAsnHeaderItem(noMemeberId, type, ptr, m_BioContext), false);

        x_FillBlock(main, type, ptr, 0, 0);
        main->AddItem(new CAsnBlockEndItem(0, false), false);
    }

    x_CreateResult();

    return eCompleted;
}

void CAsnRetrieveJob::x_FillClass(
                    CCompositeTextItem* content,
                    const CClassTypeInfo* classType,
                    TConstObjectPtr classPtr,
                    size_t indent, size_t maxIndent)
{
    string className = classType->GetName();

    CAsnBioContext* saveBioContext = m_BioContext;

    CSeq_entry_Handle seh = GetSeq_entry_Handle(classType, classPtr, *m_Scope);
    if (seh) {
        m_BioContext = new CAsnBioContext(seh);
        m_ContextContainer->AddBioContext(m_BioContext);
    }

    for (CClassTypeInfo::CIterator i(classType); i.Valid(); ++i) {
        x_CheckCancelled();

        const CMemberInfo* memberInfo = classType->GetMemberInfo(*i);
        string name = memberInfo->GetId().GetName();

        TTypeInfo memberType = memberInfo->GetTypeInfo();
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

        memberPtr = memberInfo->GetItemPtr(classPtr);

        ETypeFamily family = memberType->GetTypeFamily();
        TConstObjectPtr realPtr = memberPtr;
        TTypeInfo realType = memberType;
        x_GetMemberObject(realType, realPtr);
        family = realType->GetTypeFamily();

        if (family == eTypeFamilyPrimitive)
            content->AddItem(CAsnElementPrimitive::CreateItem(
                                                 memberInfo->GetId(),
                                                 memberType,
                                                 memberPtr, indent + 1,
                                                 m_BioContext), false);
        else {
            CAsnElementBlockCollapsed* collapsed =
                            new CAsnElementBlockCollapsed(
                                               memberInfo->GetId(),
                                               memberType,
                                               memberPtr,
                                               indent + 1,
                                               m_BioContext);
            CCompositeTextItem* expanded = new CCompositeTextItem();
            content->AddItem(new CExpandItem(collapsed, expanded, indent != maxIndent), false);

            expanded->AddItem(new CAsnElementBlockStart(
                                               memberInfo->GetId(),
                                               memberType,
                                               memberPtr,
                                               indent + 1,
                                               m_BioContext), false);
            CCompositeTextItem* block_content = new CCompositeTextItem();
            expanded->AddItem(block_content, false);
            collapsed->SetExpandedValue(block_content);

            x_FillBlock(block_content, memberType, memberPtr, indent + 1, maxIndent);
            expanded->AddItem(new CAsnBlockEndItem(indent + 1), false);
        }
    }

    m_BioContext = saveBioContext;

    x_SetNoComma(content);
}

void CAsnRetrieveJob::x_FillContainer(
                    CCompositeTextItem* content,
                    const CContainerTypeInfo* type,
                    TConstObjectPtr ptr,
                    size_t indent,
                    size_t maxIndent)
{
    CContainerTypeInfo::CConstIterator i;
    if (!type->InitIterator(i, ptr))
        return;

    size_t loaded = 0;

    TTypeInfo elementType = type->GetElementType();
    do {
        x_CheckCancelled();

        TConstObjectPtr elementPtr = type->GetElementPtr(i);
        ETypeFamily family = elementType->GetTypeFamily();

        TConstObjectPtr realPtr = elementPtr;
        TTypeInfo realType = elementType;
        x_GetMemberObject(realType, realPtr);
        family = realType->GetTypeFamily();

        if (family == eTypeFamilyPrimitive)
            content->AddItem(CAsnElementPrimitive::CreateItem(
                                                 noMemeberId,
                                                 elementType,
                                                 elementPtr,
                                                 indent + 1,
                                                 m_BioContext), false);
        else {
            CAsnElementBlockCollapsed* collapsed =
                            new CAsnElementBlockCollapsed(
                                               noMemeberId,
                                               elementType,
                                               elementPtr,
                                               indent + 1,
                                               m_BioContext);

            CCompositeTextItem* expanded = new CCompositeTextItem();
            content->AddItem(new CExpandItem(collapsed, expanded, indent != maxIndent), false);

            expanded->AddItem(new CAsnElementBlockStart(
                                               noMemeberId,
                                               elementType,
                                               elementPtr,
                                               indent + 1,
                                               m_BioContext), false);
            CCompositeTextItem* block_content = new CCompositeTextItem();
            expanded->AddItem(block_content, false);
            collapsed->SetExpandedValue(block_content);
            x_FillBlock(block_content, elementType, elementPtr, indent + 1, maxIndent);
            expanded->AddItem(new CAsnBlockEndItem(indent + 1), false);
        }

        ++loaded;
    } while ( type->NextElement(i) );

    x_SetNoComma(content);
}

void CAsnRetrieveJob::x_FillBlock(
                     CCompositeTextItem* content,
                     TTypeInfo containerType,
                     TConstObjectPtr containerPtr,
                     size_t indent, size_t maxIndent)
{
    x_CheckCancelled();

    TConstObjectPtr realPtr = containerPtr;
    TTypeInfo realType = containerType;
    x_GetMemberObject(realType, realPtr);
    ETypeFamily family = realType->GetTypeFamily();

    if (family == eTypeFamilyClass)
        x_FillClass(content,
                    CTypeConverter<CClassTypeInfo>::SafeCast(realType),
                    realPtr,
                    indent, maxIndent);
    else if (family == eTypeFamilyContainer)
        x_FillContainer(
                    content,
                    CTypeConverter<CContainerTypeInfo>::SafeCast(realType),
                    realPtr,
                    indent, maxIndent);

        //main = new CAsnHeaderItem(noMemeberId, type, ptr);

}

void CAsnRetrieveJob::x_SetNoComma(CCompositeTextItem* block)
{
    ITextItem* lastItem = block->GetItem(block->GetItemCount() - 1);
    CAsnElementItem* asn_elem = dynamic_cast<CAsnElementItem*>(lastItem);

    if (asn_elem) {
        asn_elem->SetTrailComma(false);
        return;
    }

    CCompositeTextItem* text_block = dynamic_cast<CCompositeTextItem*>(lastItem);
    if (text_block) {
        lastItem = text_block->GetItem(text_block->GetItemCount() - 1);
        asn_elem = dynamic_cast<CAsnElementItem*>(lastItem);
        if (asn_elem)
            asn_elem->SetTrailComma(false);
        return;
    }

    CExpandItem* expandBlock = dynamic_cast<CExpandItem*>(lastItem);
    if (expandBlock) {
        CAsnElementItem* collapsedItem =
            dynamic_cast<CAsnElementItem*>(expandBlock->GetCollapsedItem());
        if (collapsedItem)
            collapsedItem->SetTrailComma(false);

        CCompositeTextItem* expandedItem =
            dynamic_cast<CCompositeTextItem*>(expandBlock->GetExpandedItem());
        if (expandedItem) {
            size_t count = expandedItem->GetItemCount();
            CAsnBlockEndItem* block_end =
                dynamic_cast<CAsnBlockEndItem*>(expandedItem->GetItem(count - 1));
            if (block_end)
                block_end->SetTrailComma(false);
        }
    }
}

void CAsnRetrieveJob::x_GetMemberObject(TTypeInfo& type, TConstObjectPtr& ptr)
{
    for(;;) {
        ETypeFamily family = type->GetTypeFamily();
        if (family == eTypeFamilyChoice) {
            const CChoiceTypeInfo* choiceType =
                CTypeConverter<CChoiceTypeInfo>::SafeCast(type);
            TMemberIndex index = choiceType->GetIndex(ptr);

            if (!(index >= choiceType->GetVariants().FirstIndex() && 
                index <= choiceType->GetVariants().LastIndex())) {
                NCBI_THROW(CException, eUnknown,
                           "Invalid CHOICE value, type name: \"" + choiceType->GetAccessName() + "\"");
            }

            const CVariantInfo* variantInfo = choiceType->GetVariantInfo(index);
            ptr = variantInfo->GetVariantPtr(ptr);
            type = variantInfo->GetTypeInfo();
        }
        else if (family == eTypeFamilyPointer) {
            const CPointerTypeInfo* pointerType =
                CTypeConverter<CPointerTypeInfo>::SafeCast(type);
            ptr = pointerType->GetObjectPointer(ptr);
            type = pointerType->GetPointedType();
        }
        else if (family == eTypeFamilyClass) {
            const CClassTypeInfo* classType =
                CTypeConverter<CClassTypeInfo>::SafeCast(type);
            if (!classType->Implicit())
                break;
            const CMemberInfo* memberInfo =
                classType->GetMemberInfo(classType->GetMembers().FirstIndex());
            ptr = memberInfo->GetItemPtr(ptr);
            type = memberInfo->GetTypeInfo();
        }
        else
            break;
    }
}

END_NCBI_SCOPE
