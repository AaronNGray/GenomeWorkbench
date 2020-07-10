/*  $Id: xml_value_item.cpp 24311 2011-09-09 20:19:03Z katargir $
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
#include <corelib/ncbifloat.h>

#include <serial/objectinfo.hpp>

#include <gui/widgets/seq/xml_value_item.hpp>

#include <gui/widgets/text_widget/text_item_panel.hpp>
#include <gui/widgets/text_widget/draw_styled_text.hpp>
#include <gui/widgets/wx/wx_utils.hpp>

BEGIN_NCBI_SCOPE

void CXmlValueItem::x_RenderXml(CStyledTextOStream& ostream, CTextPanelContext*) const
{
    ostream
        << CXmlStyles::GetStyle(CXmlStyles::kBracket) << "<"
        << CXmlStyles::GetStyle(CXmlStyles::kTag)     << m_Tag;

    CConstObjectInfo objInfo = m_ObjectInfo;
    if (objInfo.GetTypeFamily() == eTypeFamilyPointer)
        objInfo = objInfo.GetPointedObject();

    _ASSERT(objInfo.GetTypeFamily() == eTypeFamilyPrimitive);

    const CPrimitiveTypeInfo* primitiveType = objInfo.GetPrimitiveTypeInfo();
    EPrimitiveValueType primitiveValueType = primitiveType->GetPrimitiveValueType();

    if (primitiveValueType == ePrimitiveValueEnum) {
        ostream << " "
                << CXmlStyles::GetStyle(CXmlStyles::kParam) << "value"
                << CXmlStyles::GetStyle(CXmlStyles::kParamValue) << "="
                << CXmlStyles::GetStyle(CXmlStyles::kParamQuote) << "\""
                << CXmlStyles::GetStyle(CXmlStyles::kParamValue);

        const CEnumeratedTypeInfo* enumType =
            CTypeConverter<CEnumeratedTypeInfo>::SafeCast(primitiveType);
        const CEnumeratedTypeValues& values = enumType->Values();

        TEnumValueType value = objInfo.GetPrimitiveValueInt4();
        string valueNum = NStr::IntToString(value);
        string valueName = values.FindName(value, values.IsInteger());
        ostream << (valueName.empty() ? valueNum : valueName);
        ostream << CXmlStyles::GetStyle(CXmlStyles::kParamQuote) << "\"";

        if (values.IsInteger()) {
            ostream
                << CXmlStyles::GetStyle(CXmlStyles::kBracket) << ">"
                << CXmlStyles::GetStyle(CXmlStyles::kTagValue) << valueNum
                << CXmlStyles::GetStyle(CXmlStyles::kBracket) << "</"
                << CXmlStyles::GetStyle(CXmlStyles::kTag)     << m_Tag
                << CXmlStyles::GetStyle(CXmlStyles::kBracket) << ">" << NewLine();
        }
        else
            ostream << CXmlStyles::GetStyle(CXmlStyles::kBracket) << "/>" << NewLine();
        return;
    }
    else if (primitiveValueType == ePrimitiveValueBool) {
        ostream << " "
                << CXmlStyles::GetStyle(CXmlStyles::kParam) << "value"
                << CXmlStyles::GetStyle(CXmlStyles::kParamValue) << "="
                << CXmlStyles::GetStyle(CXmlStyles::kParamQuote) << "\""
                << CXmlStyles::GetStyle(CXmlStyles::kParamValue)
                << (objInfo.GetPrimitiveValueBool() ? "true" : "false")
                << CXmlStyles::GetStyle(CXmlStyles::kParamQuote) << "\""
                << CXmlStyles::GetStyle(CXmlStyles::kBracket) << "/>" << NewLine();
        return;
    }
    else if (primitiveValueType == ePrimitiveValueSpecial) {
        ostream << CXmlStyles::GetStyle(CXmlStyles::kBracket) << "/>" << NewLine();
        return;
    }

    ostream << CXmlStyles::GetStyle(CXmlStyles::kBracket) << ">"
            << CXmlStyles::GetStyle(CXmlStyles::kTagValue);

    switch(primitiveValueType) {
    case ePrimitiveValueInteger :        // (signed|unsigned) (char|short|int|long)
        {{
            string value;
            if (objInfo.IsPrimitiveValueSigned())
                NStr::Int8ToString(value, objInfo.GetPrimitiveValueInt8());
            else
                NStr::UInt8ToString(value, objInfo.GetPrimitiveValueInt8());
            ostream << value;
        }}
        break;

    case ePrimitiveValueReal :
        {{
            double data = objInfo.GetPrimitiveValueDouble();
            if (isnan(data)) {
                ostream << CXmlStyles::GetStyle(CXmlStyles::kError) << "NaN"
                        << CXmlStyles::GetStyle(CXmlStyles::kTagValue);
                break;
            }
            if (!finite(data)) {
                ostream << CXmlStyles::GetStyle(CXmlStyles::kError) << "Inf"
                        << CXmlStyles::GetStyle(CXmlStyles::kTagValue);
                break;
            }

            char buffer[512];
            SIZE_TYPE width;
            size_t digits = DBL_DIG;
            if (false) {
                int shift = int(ceil(log10(fabs(data))));
                int precision = int(digits - shift);
                if ( precision < 0 )
                    precision = 0;
                if ( precision > 64 ) // limit precision of data
                    precision = 64;
                width = NStr::DoubleToString(data, (unsigned int)precision,
                                            buffer, sizeof(buffer));
                if (precision != 0) {
                    while (buffer[width-1] == '0') {
                        --width;
                    }
                    if (buffer[width-1] == '.') {
                        --width;
                    }
                }
            }
            else
                width = sprintf(buffer, "%.*g", (int)digits, data);

            ostream << string(buffer, width);
        }}
        break;

    case ePrimitiveValueString :         // string|char*|const char*
        {{
            string value = objInfo.GetPrimitiveValueString();
            x_WriteString(ostream, value);
        }}
        break;

    case ePrimitiveValueOctetString :
        {{
            vector<char> value;
            objInfo.GetPrimitiveValueOctetString(value);
            x_WriteOctetString(ostream, value);
        }}
        break;

    default:
        _ASSERT(false);
        break;
    }

    ostream
        << CXmlStyles::GetStyle(CXmlStyles::kBracket) << "</"
        << CXmlStyles::GetStyle(CXmlStyles::kTag)     << m_Tag
        << CXmlStyles::GetStyle(CXmlStyles::kBracket) << ">" << NewLine();
}

void CXmlValueItem::x_WriteString(CStyledTextOStream& ostream, const string& str) const
{
    char buffer[256];
    string::const_iterator it;
    size_t i = 0;

    for(it = str.begin(); it < str.end(); ++it) {
        char c = *it;

        switch(c) {
        case '&':
            if ( i > 0) ostream << string(buffer, i), i = 0;
            ostream  << CXmlStyles::GetStyle(CXmlStyles::kEscChar) << "&amp;"
                     << CXmlStyles::GetStyle(CXmlStyles::kTagValue);
            break;
        case '<':
            if ( i > 0) ostream << string(buffer, i), i = 0;
            ostream  << CXmlStyles::GetStyle(CXmlStyles::kEscChar) << "&lt;"
                     << CXmlStyles::GetStyle(CXmlStyles::kTagValue);
            break;
        case '>':
            if ( i > 0) ostream << string(buffer, i), i = 0;
            ostream  << CXmlStyles::GetStyle(CXmlStyles::kEscChar) << "&gt;"
                     << CXmlStyles::GetStyle(CXmlStyles::kTagValue);
            break;
        case '\'':
            if ( i > 0) ostream << string(buffer, i), i = 0;
            ostream  << CXmlStyles::GetStyle(CXmlStyles::kEscChar) << "&apos;"
                     << CXmlStyles::GetStyle(CXmlStyles::kTagValue);
            break;
        case '"':
            if ( i > 0) ostream << string(buffer, i), i = 0;
            ostream  << CXmlStyles::GetStyle(CXmlStyles::kEscChar) << "&quot;"
                     << CXmlStyles::GetStyle(CXmlStyles::kTagValue);
            break;
        default:
            if ((unsigned int)c < 0x20) {
                if ( i > 0) ostream << string(buffer, i), i = 0;
                strcpy(buffer, "&#x");
                i = 3;
                Uint1 ch = c;
                unsigned hi = ch >> 4;
                unsigned lo = ch & 0xF;
                if (hi) buffer[i++] = "0123456789abcdef"[hi];
                buffer[i++] = "0123456789abcdef"[lo];
                buffer[i++] = ';';

                ostream  << CXmlStyles::GetStyle(CXmlStyles::kEscChar)
                         << string(buffer, i)
                         << CXmlStyles::GetStyle(CXmlStyles::kTagValue);
                i = 0;
            }
            else {
                buffer[i++] = c;
                if (i == sizeof(buffer)) ostream << string(buffer, i), i = 0;
            }
        }
    }

    if ( i > 0) ostream << string(buffer, i);
}

void CXmlValueItem::x_WriteOctetString(CStyledTextOStream& ostream, const vector<char>& value) const
{
    static const char HEX[] = "0123456789ABCDEF";
    char buffer[256];
    size_t i = 0;

    vector<char>::const_iterator it;
    for(it = value.begin(); it < value.end(); ++it) {
        char c = *it;
        buffer[i++] = HEX[(c >> 4) & 0xf];
        buffer[i++] = HEX[c & 0xf];
        if (i == sizeof(buffer)) ostream << string(buffer, i), i = 0;
    }

    if ( i > 0) ostream << string(buffer, i);
}

END_NCBI_SCOPE
