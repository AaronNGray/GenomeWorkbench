/*  $Id: asn_element_block_collapsed.cpp 24019 2011-07-08 13:45:08Z wuliangs $
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

#include <gui/widgets/seq/asn_element_block_collapsed.hpp>
#include <gui/widgets/text_widget/calc_nbpos_stream.hpp>
#include <gui/widgets/seq/flat_file_view_params.hpp>

#include "asn_styles.hpp"

#include <serial/objectinfo.hpp>

#include <wx/dcclient.h>
#include <wx/txtstrm.h>

BEGIN_NCBI_SCOPE

void CAsnElementBlockCollapsed::RenderValue(
                CStyledTextOStream& ostream,
                CTextPanelContext* context,
                TTypeInfo type, TConstObjectPtr) const
{
    _ASSERT(type->GetTypeFamily() == eTypeFamilyClass  ||
        type->GetTypeFamily() == eTypeFamilyContainer);

    ostream << CAsnStyles::GetStyle(CAsnStyles::kBraces) << "{";

    if (CStyledTextOStream::kCopyText == ostream.GetHint()) {
        if (ostream.IsVisible() && m_ExpandedValue) {
            wxTextOutputStream* textStream = ostream.GetTextStream();
            *textStream << endl;
            m_ExpandedValue->GetText(*textStream, m_ExpandedValue->GetItemBlock(), context);
            if (m_Indent > 0)
                *textStream << wxString(m_Indent*kAsnIndentSize, ' ');
        }
        ostream.UpdatePosition(3);
    }
    else {
        ostream << NB() <<CAsnStyles::GetStyle(CAsnStyles::kCollapsedBlock) << " ... " << NB();
    }

    ostream << CAsnStyles::GetStyle(CAsnStyles::kBraces) << "}";

    x_RenderComma(ostream);
}

void CAsnElementBlockCollapsed::TextPosToExpanded(CTextPosition& pos, CTextPanelContext* context) const
{
    int row = pos.GetRow(), col = pos.GetCol();

    if (row < 0 || row >= 1)
        return;

    CCalcNBPosOStream ostr;
    x_RenderText(ostr, context);

    int nbStart, nbEnd, nbRow;
    ostr.GetNBPos(nbStart, nbEnd, nbRow);

    int indent = GetIndent();

    if (col >= nbEnd) {
        row = GetLineCount() - 1;
        col = indent + col - nbEnd;
    }

    pos = CTextPosition(row, col);
}

void CAsnElementBlockCollapsed::TextPosToCollapsed(CTextPosition& pos, CTextPanelContext* context) const
{
    int row = pos.GetRow(), col = pos.GetCol();

    if (row < 0 || row >= (int)GetLineCount())
        return;

    CCalcNBPosOStream ostr;
    x_RenderText(ostr, context);

    int nbStart, nbEnd, nbRow;
    ostr.GetNBPos(nbStart, nbEnd, nbRow);

    int indent = GetIndent();

    if (row == (int)GetLineCount() - 1 && col >= indent) {
        if (col >= indent)
            col += nbEnd - indent;
        else
            col = nbStart;
    }
    else if (row > 0 || col > nbStart)
        col = nbStart;

    row = 0;

    pos = CTextPosition(row, col);
}

END_NCBI_SCOPE
