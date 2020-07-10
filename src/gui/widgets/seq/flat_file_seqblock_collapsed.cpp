/*  $Id: flat_file_seqblock_collapsed.cpp 29707 2014-01-31 16:59:45Z katargir $
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

#include <gui/widgets/seq/flat_file_seqblock_collapsed.hpp>
#include <gui/widgets/seq/flat_file_context.hpp>
#include <gui/widgets/seq/flat_file_view_params.hpp>

#include <util/xregexp/regexp.hpp>

BEGIN_NCBI_SCOPE
USING_SCOPE(objects);

class CSeqShortText : public IFlatTextOStream
{
public:
    CSeqShortText() {}

    virtual void AddParagraph(const list<string>&  text, const CSerialObject*)
    {
        list<string>::const_iterator it = text.begin();
        if (it == text.end())
            return;
        m_Header = *it;
    }

    virtual void AddLine(const CTempString& line,
                         const CSerialObject*,
                         EAddNewline)
    {
        if (m_Header.empty())
            m_Header = line;
    }

    string GetText() const { return m_Header.empty() ? "     " : m_Header; }

private:
    string m_Header;
};

void CFlatFileSeqBlockCollapsed::x_RenderText(CStyledTextOStream& ostream, CTextPanelContext* context) const
{
    CFlatFileViewContext* pcntx = static_cast<CFlatFileViewContext*>(context);

    CSeqShortText shortText;
    m_Item->Format(pcntx->GetFormatter(), shortText);
    ostream << shortText.GetText();
}


END_NCBI_SCOPE
