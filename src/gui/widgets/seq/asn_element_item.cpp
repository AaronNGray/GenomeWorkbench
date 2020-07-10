/*  $Id: asn_element_item.cpp 39821 2017-11-08 16:18:05Z katargir $
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
#include <gui/widgets/text_widget/draw_styled_text.hpp>

#include <gui/widgets/seq/asn_element_item.hpp>
#include <gui/widgets/seq/flat_file_view_params.hpp>
#include <gui/widgets/seq/asn_view_context.hpp>

#include "asn_styles.hpp"

#include <serial/objectinfo.hpp>

#include <wx/dcclient.h>
#include <wx/txtstrm.h>

BEGIN_NCBI_SCOPE

int CAsnElementItem::GetIndent() const
{
    return m_Indent*kAsnIndentSize;
}

void CAsnElementItem::x_RenderText(CStyledTextOStream& ostream, CTextPanelContext* context) const
{
    x_Indent(ostream);

    TTypeInfo type = m_Type;
    TConstObjectPtr ptr = m_Ptr;

    ETypeFamily family;

    string memberId = x_GetMemberId(m_MemberId);
    if (!memberId.empty())
        ostream << CAsnStyles::GetStyle(CAsnStyles::kMember) << memberId;

    for(;;) {
        family = type->GetTypeFamily();
        if (family == eTypeFamilyChoice) {
            const CChoiceTypeInfo* choiceType =
                CTypeConverter<CChoiceTypeInfo>::SafeCast(type);
            TMemberIndex index = choiceType->GetIndex(ptr);
            const CVariantInfo* variantInfo = choiceType->GetVariantInfo(index);
            ostream << CAsnStyles::GetStyle(CAsnStyles::kChoice)
                    << x_GetMemberId(variantInfo->GetId());
            ptr = variantInfo->GetVariantPtr(ptr);
            type = variantInfo->GetTypeInfo();
        }
        else if (family == eTypeFamilyPointer) {
            const CPointerTypeInfo* pointerType =
                CTypeConverter<CPointerTypeInfo>::SafeCast(type);
            ptr = pointerType->GetObjectPointer(ptr);
            type = pointerType->GetPointedType();
        }
        else
            break;
    }

    RenderValue(ostream, context, type, ptr);
    ostream << NewLine();
/*
    if (family == eTypeFamilyPrimitive) {
        const CPrimitiveTypeInfo* primitiveType =
            CTypeConverter<CPrimitiveTypeInfo>::SafeCast(type);

        EPrimitiveValueType primitiveValueType = primitiveType->GetPrimitiveValueType();
        switch(primitiveValueType) {
        case ePrimitiveValueEnum :           // enum
            {{
                const CEnumeratedTypeInfo* enumType =
                    CTypeConverter<CEnumeratedTypeInfo>::SafeCast(type);

                string valueName;
                enumType->GetValueString(ptr, valueName);
                if (valueName.empty()) {
                    CNcbiOstrstream strm;
                    strm << enumType->GetValueInt4(ptr);
                    ostream << DefaultStyle() << string(strm.str(), strm.pcount());
                } else
                    ostream << DefaultStyle() << valueName;
            }}
            break;
        case ePrimitiveValueInteger :        // (signed|unsigned) (char|short|int|long)
            {{
                string value;
                if (primitiveType->IsSigned())
                    NStr::Int8ToString(value, primitiveType->GetValueInt8(ptr));
                else
                    NStr::UInt8ToString(value, primitiveType->GetValueInt8(ptr));
                ostream << CAsnStyles::GetStyle(CAsnStyles::kNumberValue) << value;
            }}
            break;
        case ePrimitiveValueString :         // string|char*|const char*
            {{
                ostream << CAsnStyles::GetStyle(CAsnStyles::kTextValue);
                string value;
                primitiveType->GetValueString(ptr, value);
                value = "\"" + value + "\"";
                size_t length = value.length();
                if (length < 78 - ostream.CurrentRowLength())
                    ostream << value;
                else {
                    length = 78 - ostream.CurrentRowLength();
                    ostream << value.substr(0, length);
                    value = value.substr(length);
                    while (!value.empty()) {
                        length = min(78u, value.length());
                        ostream << NewLine()
                            << value.substr(0, length);
                        value = value.substr(length);
                    }
                }
            }}
            break;
        }
        if (m_TrailComma)
            ostream << DefaultStyle() << ",";
    }
    else {
        ostream << CAsnStyles::GetStyle(CAsnStyles::kBraces) << "{";
        if (!m_Expanded) {
            if (ostream.GetHint() == CStyledTextOStream::kCopyText) {

            }
            else {
                ostream << CAsnStyles::GetStyle(CAsnStyles::kCollapsedBlock);
                ostream.WriteNB("...") << CAsnStyles::GetStyle(CAsnStyles::kBraces);
            }
            ostream << "}";
            if (m_TrailComma)
                ostream << DefaultStyle() << ",";
        }
    }
 */
}

const CConstRef<CObject> CAsnElementItem::GetAssosiatedObject() const
{
    TTypeInfo type = m_Type;
    TConstObjectPtr ptr = m_Ptr;

    GetSelectionObject(type, ptr);

    ETypeFamily family = type->GetTypeFamily();;

    if (family == eTypeFamilyClass) {
        const CClassTypeInfo* classType =
            CTypeConverter<CClassTypeInfo>::SafeCast(type);
        return CConstRef<CObject>(classType->GetCObjectPtr(ptr));
    }
    else if (family == eTypeFamilyChoice) {
        const CChoiceTypeInfo* choiceType =
            CTypeConverter<CChoiceTypeInfo>::SafeCast(type);
        return CConstRef<CObject>(choiceType->GetCObjectPtr(ptr));
    }

    return null;
}

CEditObject CAsnElementItem::GetEditObject(objects::CScope& scope)
{
    TTypeInfo type = m_Type;
    TConstObjectPtr ptr = m_Ptr;

    ETypeFamily family = type->GetTypeFamily();
    while (family == eTypeFamilyPointer) {
        const CPointerTypeInfo* pointerType =
            CTypeConverter<CPointerTypeInfo>::SafeCast(type);
        ptr = pointerType->GetObjectPointer(ptr);
        type = pointerType->GetPointedType();
        family = type->GetTypeFamily();
    }

    if (family == eTypeFamilyClass) {
        const CClassTypeInfo* classType =
            CTypeConverter<CClassTypeInfo>::SafeCast(type);
        if (type->GetName() == "Seq-feat" && m_BioContext) {
            return CEditObject(*classType->GetCObjectPtr(ptr),
                m_BioContext->GetSeq_entry_Handle());
        }
        if (type->GetName() == "Submit-block") {
            return CEditObject(*classType->GetCObjectPtr(ptr), scope);
        }
    }
    else if (family == eTypeFamilyChoice) {
        if (m_BioContext && (type->GetName() == "Seqdesc" || type->GetName() == "Seq-entry")) {
            const CChoiceTypeInfo* choiceType =
                CTypeConverter<CChoiceTypeInfo>::SafeCast(type);
            return CEditObject(*choiceType->GetCObjectPtr(ptr),
                m_BioContext->GetSeq_entry_Handle());
        }
    }
    return CEditObject();
}


void CAsnElementItem::GetSelectionObject(TTypeInfo& type, TConstObjectPtr& ptr)
{
    for(;;) {
        ETypeFamily family = type->GetTypeFamily();
        if (family == eTypeFamilyChoice) {
            if (type->GetName() == "Seqdesc")
                return;
            const CChoiceTypeInfo* choiceType =
                CTypeConverter<CChoiceTypeInfo>::SafeCast(type);
            TMemberIndex index = choiceType->GetIndex(ptr);
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
        else
            break;
    }
}


void CAsnElementItem::GetRealObject(TTypeInfo& type, TConstObjectPtr& ptr)
{
    for(;;) {
        ETypeFamily family = type->GetTypeFamily();
        if (family == eTypeFamilyChoice) {
            const CChoiceTypeInfo* choiceType =
                CTypeConverter<CChoiceTypeInfo>::SafeCast(type);
            TMemberIndex index = choiceType->GetIndex(ptr);
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
        else
            break;
    }
}

string CAsnElementItem::x_GetMemberId(const CMemberId& id)
{
    const string& name = id.GetName();
    string str;

    if ( !name.empty() ) {
        str = name + ' ';
        if (id.HaveNoPrefix() && isupper((unsigned char)str[0]))
            str[0] = tolower((unsigned char)str[0]);
    }
    else if (id.HaveExplicitTag())
        str = "[" + NStr::IntToString(id.GetTag()) + "] ";

    return str;
}

void CAsnElementItem::x_RenderComma(CStyledTextOStream& ostream) const
{
    if (m_TrailComma)
        ostream << DefaultStyle() << ",";
}

END_NCBI_SCOPE


