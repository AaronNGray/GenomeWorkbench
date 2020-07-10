/*  $Id: fasta_seqhdr_item.cpp 32371 2015-02-13 20:22:08Z katargir $
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

#include <objmgr/util/sequence.hpp>

#include <gui/opengl/glfont.hpp>
#include <gui/opengl/glbitmapfont.hpp>

#include <gui/widgets/text_widget/text_item_panel.hpp>
#include <gui/widgets/text_widget/draw_styled_text.hpp>
#include <gui/widgets/wx/wx_utils.hpp>

#include <gui/widgets/seq/fasta_view_params.hpp>
#include <gui/widgets/seq/fasta_seqhdr_item.hpp>
#include <gui/widgets/seq/fasta_title_item.hpp>

#include <wx/dcclient.h>
#include <wx/txtstrm.h>

BEGIN_NCBI_SCOPE
USING_SCOPE(objects);

void CFastaSeqHdrItem::CalcSize(wxDC& /*dc*/, CTextPanelContext* context)
{
    wxSize size = context->GetWSize();
    m_Size.Set(context->GetLeftMargin() + size.x*kFastaSeqLine, size.y);
}

void CFastaSeqHdrItem::x_RenderText(CStyledTextOStream& ostream, CTextPanelContext* /*context*/) const
{
    ostream << x_GetText() << NewLine();
}

string CFastaSeqHdrItem::x_GetText() const
{
    return CTextUtils::FormatSeparatedNumber(m_From + 1)
                + "-" + CTextUtils::FormatSeparatedNumber(m_From + m_Length);
}

END_NCBI_SCOPE


