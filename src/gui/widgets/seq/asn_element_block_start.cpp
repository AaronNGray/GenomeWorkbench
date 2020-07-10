/*  $Id: asn_element_block_start.cpp 24685 2011-11-07 15:13:51Z katargir $
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

#include <gui/widgets/seq/asn_element_block_start.hpp>
#include <gui/widgets/seq/asn_view_context.hpp>

#include "asn_styles.hpp"

#include <serial/objectinfo.hpp>

#include <wx/dcclient.h>
#include <wx/txtstrm.h>

BEGIN_NCBI_SCOPE

void CAsnElementBlockStart::RenderValue(CStyledTextOStream& ostream, CTextPanelContext* context,
                                        TTypeInfo type, TConstObjectPtr ptr) const
{
    _ASSERT(type->GetTypeFamily() == eTypeFamilyClass  ||
        type->GetTypeFamily() == eTypeFamilyContainer);

    ostream << CAsnStyles::GetStyle(CAsnStyles::kBraces) << "{";

    CAsnViewContext* asnContext = dynamic_cast<CAsnViewContext*>(context);
    if (asnContext == NULL || !asnContext->GetShowAsnTypes())
        return;

    if (type->GetTypeFamily() == eTypeFamilyChoice) {
        const CChoiceTypeInfo* choiceType =
            CTypeConverter<CChoiceTypeInfo>::SafeCast(type);
        TMemberIndex index = choiceType->GetIndex(ptr);
        const CVariantInfo* variantInfo = choiceType->GetVariantInfo(index);
        type = variantInfo->GetTypeInfo();
    }

    if (type->GetTypeFamily() == eTypeFamilyPointer) {
        const CPointerTypeInfo* pointerType =
            CTypeConverter<CPointerTypeInfo>::SafeCast(type);
        type = pointerType->GetPointedType();
    }

    ETypeFamily family = type->GetTypeFamily();
    if (family == eTypeFamilyClass) {
        string typeName = type->GetName();
        if (!typeName.empty()) {
            ostream << CAsnStyles::GetStyle(CAsnStyles::kComment)
                    << " -- " << typeName << " --";
        }
    }
    else if (family == eTypeFamilyContainer) {
        const CContainerTypeInfo* containerType =
            CTypeConverter<CContainerTypeInfo>::SafeCast(type);

        TTypeInfo elementType = containerType->GetElementType();
        if (elementType->GetTypeFamily() == eTypeFamilyPointer) {
            const CPointerTypeInfo* pointerType =
                CTypeConverter<CPointerTypeInfo>::SafeCast(elementType);
            elementType = pointerType->GetPointedType();
        }

        string elementName = elementType->GetName();

        if (elementName.empty() && elementType->GetTypeFamily() == eTypeFamilyPrimitive) {
            const CPrimitiveTypeInfo* primitiveType =
                CTypeConverter<CPrimitiveTypeInfo>::SafeCast(elementType);

            EPrimitiveValueType valueType = primitiveType->GetPrimitiveValueType();
            switch (valueType) {
            case ePrimitiveValueInteger :
                elementName = "INTEGER";
                break;
            case ePrimitiveValueReal :
                elementName = "REAL";
                break;
            case ePrimitiveValueString :
                elementName = "VisibleString";
                break;
            case ePrimitiveValueBool :
                elementName = "BOOLEAN";
                break;
            default:
                break;
            }
        }

        if (!elementName.empty())
            ostream << CAsnStyles::GetStyle(CAsnStyles::kComment)
                    << " -- SEQUENCE OF " << elementName << " --";
    }

}

ITextItem* CAsnElementBlockStart::Clone()
{
    return new CAsnElementBlockStart(*this);
}

END_NCBI_SCOPE


