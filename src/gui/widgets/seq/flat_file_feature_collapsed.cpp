/*  $Id: flat_file_feature_collapsed.cpp 29707 2014-01-31 16:59:45Z katargir $
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

#include <objtools/format/text_ostream.hpp>
#include <objtools/format/formatter.hpp>
#include <objtools/format/items/item.hpp>

#include <gui/widgets/text_widget/draw_styled_text.hpp>

#include <gui/widgets/seq/flat_file_feature_collapsed.hpp>
#include <gui/widgets/seq/flat_file_context.hpp>
#include <gui/widgets/seq/flat_file_view_params.hpp>

#include <util/xregexp/regexp.hpp>

BEGIN_NCBI_SCOPE
USING_SCOPE(objects);

namespace // anonymous
{
class CFeatureShortText : public IFlatTextOStream
{
public:
    CFeatureShortText() {}

    virtual void AddParagraph(const list<string>&  text, const CSerialObject*)
    {
        list<string>::const_iterator it = text.begin();
        if (it == text.end())
            return;
        m_Header = *it++;

        CRegexp re("(/gene=\"\\w+\")");
        for (;it != text.end();++it) {
            string qual = (*it).substr(21);
            if (!re.GetMatch(qual).empty()) {
                const int *range = re.GetResults(1);
                m_Header += ", " + qual.substr(range[0], range[1] - range[0]);
            }
        }
    }

    virtual void AddLine(const CTempString& line,
                         const CSerialObject*,
                         EAddNewline)
    {
        if (m_Header.empty())
            m_Header = line;
    }

    string GetText() const { return m_Header.empty() ? "     feature" : m_Header; }
private:
    string m_Header;
};
} // anonymous namespace

void CFlatFileFeatureCollapsed::x_RenderText(CStyledTextOStream& ostream, CTextPanelContext* context) const
{
    ostream << x_GetText(context);
}

string CFlatFileFeatureCollapsed::x_GetText(CTextPanelContext* context) const
{
    CFlatFileViewContext* pcntx = dynamic_cast<CFlatFileViewContext*>(context);
    _ASSERT(pcntx);
    if (!pcntx)
        return NcbiEmptyString;

    CFeatureShortText shortText;
    if (!m_Item->Skip())
        m_Item->Format(pcntx->GetFormatter(), shortText);
    return shortText.GetText();
}


END_NCBI_SCOPE
