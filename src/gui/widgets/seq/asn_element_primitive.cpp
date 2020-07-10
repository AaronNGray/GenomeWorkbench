/*  $Id: asn_element_primitive.cpp 30852 2014-07-30 22:50:44Z ucko $
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

#include <gui/widgets/text_widget/text_item_panel.hpp>
#include <gui/widgets/text_widget/draw_styled_text.hpp>

#include <gui/widgets/seq/asn_element_primitive.hpp>
#include <gui/widgets/text_widget/composite_text_item.hpp>

#include "asn_styles.hpp"

#include <serial/objectinfo.hpp>

#include <wx/dcclient.h>
#include <wx/txtstrm.h>

BEGIN_NCBI_SCOPE

const int kTextBlockLines = 50;
const unsigned int kMaxTextLine = 78;

void CAsnElementPrimitive::RenderValue(CStyledTextOStream& ostream, CTextPanelContext*,
                                       TTypeInfo type, TConstObjectPtr ptr) const
{
    _ASSERT(type->GetTypeFamily() == eTypeFamilyPrimitive);

    const CPrimitiveTypeInfo* primitiveType =
        CTypeConverter<CPrimitiveTypeInfo>::SafeCast(type);

    EPrimitiveValueType primitiveValueType = primitiveType->GetPrimitiveValueType();
    switch(primitiveValueType) {
    case ePrimitiveValueSpecial :
        {{
            ostream << CAsnStyles::GetStyle(CAsnStyles::kNullValue) << "NULL";
        }}
        break;
    case ePrimitiveValueBool :
        {{
            bool value = primitiveType->GetValueBool(ptr);
            ostream << CAsnStyles::GetStyle(CAsnStyles::kNumberValue);
            ostream << (value ? "TRUE" : "FALSE");
        }}
        break;
    case ePrimitiveValueEnum :           // enum
        {{
            const CEnumeratedTypeInfo* enumType =
                CTypeConverter<CEnumeratedTypeInfo>::SafeCast(type);

            string valueName;
			// the call to GetValueString throws exception if value ia an arbitrary interger GB-2987
			try {
				enumType->GetValueString(ptr, valueName);
			} catch (const CException&) {
			}

			if (!valueName.empty()) {
                ostream << DefaultStyle() << valueName;
			} else {
				Int4 value = enumType->GetValueInt4(ptr);
                ostream << DefaultStyle() << NStr::NumericToString(value);

				try {
					const CEnumeratedTypeValues& values = enumType->Values();
					if (values.IsBitset()) {
						string bitSet = values.GetDisplayName(value);
						if (!bitSet.empty()) {
							ostream << CAsnStyles::GetStyle(CAsnStyles::kComment) << " --(" 
								    << DefaultStyle() << bitSet << CAsnStyles::GetStyle(CAsnStyles::kComment) << ")--";
						}
					}
				} catch (const CException&) {
				}
            }
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
    case ePrimitiveValueReal :
        {{
            size_t digits = DBL_DIG;
            double data = primitiveType->GetValueDouble(ptr);
            ostream << CAsnStyles::GetStyle(CAsnStyles::kNumberValue);

            if (isnan(data)) {
                ostream << CAsnStyles::GetStyle(CAsnStyles::kError) << "NaN";
                break;
            }
            if (!finite(data)) {
                ostream << CAsnStyles::GetStyle(CAsnStyles::kError) << "Inf";
                break;
            }
            if ( data == 0.0 ) {
                ostream << "{ 0, 10, 0 }";
                break;
            }

            char buffer[128];
            // ensure buffer is large enough to fit result
            // (additional bytes are for sign, dot and exponent)
            _ASSERT(sizeof(buffer) > digits + 16);
            int width = sprintf(buffer, "%.*e", int(digits-1), data);
            if ( width <= 0 || width >= int(sizeof(buffer) - 1) ) {
                ostream << CAsnStyles::GetStyle(CAsnStyles::kError) << "buffer overflow";
                break;
            }
            _ASSERT(int(strlen(buffer)) == width);
            char* dotPos = strchr(buffer, '.');
            _ASSERT(dotPos);
            char* ePos = strchr(dotPos, 'e');
            _ASSERT(ePos);

            // now we have:
            // mantissa with dot - buffer:ePos
            // exponent - (ePos+1):

            int exp;
            // calculate exponent
            if ( sscanf(ePos + 1, "%d", &exp) != 1 ) {
                ostream << CAsnStyles::GetStyle(CAsnStyles::kError) << "double value conversion error";
                break;
            }

            // remove trailing zeroes
            int fractDigits = int(ePos - dotPos - 1);
            while ( fractDigits > 0 && ePos[-1] == '0' ) {
                --ePos;
                --fractDigits;
            }

            // now we have:
            // mantissa with dot without trailing zeroes - buffer:ePos

            string tmp1, tmp2;
            tmp1 = string(buffer, dotPos - buffer) + string(dotPos + 1, fractDigits);
            NStr::Int8ToString(tmp2, exp - fractDigits);
            ostream << "{ " << tmp1 << ", 10, " << tmp2 << " }";
        }}
        break;
    case ePrimitiveValueString :         // string|char*|const char*
        {{
            ostream << CAsnStyles::GetStyle(CAsnStyles::kTextValue);
            string value;
            primitiveType->GetValueString(ptr, value);
            ostream << "\"";
            size_t length = value.length();
            while (length > 0) {
                size_t curLength = ostream.CurrentRowLength();
                size_t posBreak = 0, pos = 0;
                int count = min(length, 78 - curLength);

                for (pos = 0; count > 0; pos++, count--) {
                    unsigned char c = value[pos];
                    if (c == '"') count--;
                    if (isspace(c) || c == '\'')
                        posBreak = pos;
                }

                if (pos < length) {
                    unsigned char c = value[pos];
                    if (isspace(c) || c == '\'')
                        posBreak = pos;
                }

                // can we wrap string at word break?
                if (posBreak == 0 || pos == length)
                    posBreak = pos;

                string tmp;
                for (pos = 0; pos < posBreak; pos++) {
                    unsigned char c = value[pos];
                    tmp += c;
                    if (c == '"') tmp += c;
                }
                ostream << tmp;
                value = value.substr(posBreak);
                length = length - posBreak;
                if (length > 0)
                    ostream << NewLine();
            }
            ostream << "\"";
        }}
        break;
    case ePrimitiveValueOctetString:
        _ASSERT(false); // Should be handled by CAsnOctetStringBlock
        break;
    default:
        break;
    }

    x_RenderComma(ostream);
}

class CCalcDataPosOStream : public CStyledTextOStream
{
public:
    CCalcDataPosOStream() : m_TextPosX(0), m_DataOffset(-1) {}

    virtual StreamHint GetHint() const { return kTextPos; }
    virtual CStyledTextOStream& operator<< (const string& text)
    {
        m_TextPosX += text.length();
        return *this;
    }
    virtual int CurrentRowLength() const { return m_TextPosX; }

    virtual void SetStyle(const CTextStyle* style);
    virtual void SetDefaultStyle() {}
    virtual void Indent (size_t steps) { m_TextPosX += steps; }
    virtual void NewLine() { m_TextPosX = 0; }

    size_t GetDataOffset() const { return m_DataOffset; }

private:
    int     m_TextPosX;
    int     m_DataOffset;
};

void CCalcDataPosOStream::SetStyle(const CTextStyle* style)
{
    if (m_DataOffset == -1 && style == &CAsnStyles::GetStyle(CAsnStyles::kNumberValue))
        m_DataOffset = m_TextPosX;
}

CTextItem* CAsnElementPrimitive::CreateItem(
                const CMemberId& memberId,
                TTypeInfo type,
                TConstObjectPtr ptr,
                size_t indent,
                CAsnBioContext* bioContext)
{
    CTextItem* retval = 0;

    TTypeInfo realType = type;
    TConstObjectPtr realPtr = ptr;

    GetRealObject(realType, realPtr);
    _ASSERT(realType->GetTypeFamily() == eTypeFamilyPrimitive);
    const CPrimitiveTypeInfo* primitiveType =
        CTypeConverter<CPrimitiveTypeInfo>::SafeCast(realType);

    EPrimitiveValueType primitiveValueType = primitiveType->GetPrimitiveValueType();
    if (primitiveValueType == ePrimitiveValueOctetString) {
        size_t data_size;

        {{
            vector<char> value;
            primitiveType->GetValueOctetString(realPtr, value);
            data_size = value.size();
        }}

        CAsnOctetStringBlock* element = new CAsnOctetStringBlock(true, 0, memberId, type, ptr, indent, bioContext);
        retval = element;

        CCalcDataPosOStream os;
        element->RenderText(os, NULL);
        size_t dataOffset = os.GetDataOffset();
        size_t first_line = 0;
        if (dataOffset + 3 <= kMaxTextLine)
            first_line = (kMaxTextLine - dataOffset - 1)/2;

        size_t total_lines = 1 + (((data_size - first_line)*2 + 2) + kMaxTextLine - 1)/kMaxTextLine;
        size_t blocks_num = (total_lines + kTextBlockLines - 1)/kTextBlockLines;
        if (blocks_num > 1) {
            CCompositeTextItem* group = new CCompositeTextItem();
            retval = group;

            element->SetTrailComma(false);
            group->AddItem(element);

            size_t full_line = kMaxTextLine/2;

            size_t offset = first_line + (kTextBlockLines - 1)*full_line;
            while (--blocks_num > 0) {
                element = new CAsnOctetStringBlock(false, offset, memberId, type, ptr, indent, bioContext);
                element->SetTrailComma(blocks_num == 1);
                group->AddItem(element);
                offset += full_line*kTextBlockLines;
            }
        }
    }
    else
        retval = new CAsnElementPrimitive(memberId, type, ptr, indent, bioContext);

    return retval;
}

void CAsnOctetStringBlock::x_RenderText(CStyledTextOStream& ostream, CTextPanelContext* context) const
{
    if (m_FirstBlock) {
        CAsnElementItem::x_RenderText(ostream, context);
    }
    else {
        TTypeInfo type = m_Type;
        TConstObjectPtr ptr = m_Ptr;

        GetRealObject(type, ptr);
        RenderValue(ostream, context, type, ptr);
        ostream << NewLine();
    }
}

static void s_FinishOctetString(CStyledTextOStream& ostream)
{
    ostream << "\'" << DefaultStyle() << "H";
}

void CAsnOctetStringBlock::RenderValue(CStyledTextOStream& ostream, CTextPanelContext*,
                                       TTypeInfo type, TConstObjectPtr ptr) const
{
    _ASSERT(type->GetTypeFamily() == eTypeFamilyPrimitive);

    const CPrimitiveTypeInfo* primitiveType =
        CTypeConverter<CPrimitiveTypeInfo>::SafeCast(type);
    _ASSERT(ePrimitiveValueOctetString == primitiveType->GetPrimitiveValueType());

    vector<char> value;
    primitiveType->GetValueOctetString(ptr, value);

    ostream << CAsnStyles::GetStyle(CAsnStyles::kNumberValue);

    int lines = 1;

    if (m_FirstBlock) {
        ostream << CAsnStyles::GetStyle(CAsnStyles::kNumberValue) << "\'";

        size_t curLength = ostream.CurrentRowLength();
        if (curLength + 2 > kMaxTextLine) {
            ostream << NewLine();
            ++lines;
        }
    }

    size_t i = m_DataOffset;
    for (;;) {
        if (i < value.size()) {
            size_t curLength = ostream.CurrentRowLength();
            size_t count = min(value.size()-i, (kMaxTextLine - curLength)/2);
            string tmp;
            static const char HEX[] = "0123456789ABCDEF";

            while (count-- > 0) {
                char c = value[i];
                tmp += HEX[(c >> 4) & 0xf];
                tmp += HEX[c & 0xf];
                ++i;
            }
            ostream << tmp;
        }

        if (i >= value.size()) {
            size_t curLength = ostream.CurrentRowLength();
            if (curLength + 2 <= kMaxTextLine) {
                s_FinishOctetString(ostream);
                x_RenderComma(ostream);
                break;
            }
            else {
                if (lines < kTextBlockLines) {
                    ostream << NewLine();
                    ++lines;
                    s_FinishOctetString(ostream);
                    x_RenderComma(ostream);
                }
                break;
            }
        }

        if (lines == kTextBlockLines)
            break;

        ostream << NewLine();
        ++lines;
    }
}

END_NCBI_SCOPE
