/*  $Id: fasta_title_item.cpp 32373 2015-02-13 20:38:05Z katargir $
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

#include <gui/widgets/text_widget/text_item_panel.hpp>
#include <gui/widgets/text_widget/draw_styled_text.hpp>
#include <gui/widgets/text_widget/selection_helper.hpp>

#include <gui/widgets/seq/fasta_view_params.hpp>
#include <gui/widgets/seq/fasta_title_item.hpp>

#include <gui/widgets/wx/wx_utils.hpp>

#include <wx/dcclient.h>
#include <wx/txtstrm.h>

BEGIN_NCBI_SCOPE
USING_SCOPE(objects);

CFastaTitleItem::CFastaTitleItem(const objects::CBioseq_Handle& h, const objects::CSeq_loc* loc)
    : m_Bioseq(h), m_Loc(loc)
{
    m_Title = x_GetTitle();

    try {
        m_SeqId.Reset(m_Bioseq.GetSeqId().GetPointerOrNull());
    }
    catch (const CObjMgrException&) {
    }
}

void CFastaTitleItem::CalcSize(wxDC& /*dc*/, CTextPanelContext* context)
{
    wxSize size = context->GetWSize();
    m_Size.Set(context->GetLeftMargin() + size.x*m_Title.length(), size.y);
}

void CFastaTitleItem::x_RenderText(CStyledTextOStream& ostream, CTextPanelContext* /*context*/) const
{
    ostream << m_Title << NewLine();
}

string CFastaTitleItem::x_GetTitle() const
{
    CNcbiOstrstream ostr;
    CFastaOstream fasta_str(ostr);
    fasta_str.SetFlag(CFastaOstream::eAssembleParts);
    fasta_str.SetFlag(CFastaOstream::eInstantiateGaps);
    fasta_str.WriteTitle(m_Bioseq, m_Loc.GetPointer());
    ostr.flush();

    string s = CNcbiOstrstreamToString(ostr);
    return s.substr(0, s.find('\n'));
}

const CConstRef<CObject> CFastaTitleItem::GetAssosiatedObject() const
{
    return CConstRef<CObject>(m_SeqId.GetPointerOrNull());
}

END_NCBI_SCOPE


