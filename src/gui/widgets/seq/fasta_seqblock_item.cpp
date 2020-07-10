/*  $Id: fasta_seqblock_item.cpp 32372 2015-02-13 20:37:19Z katargir $
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

#include <objmgr/seq_vector.hpp>
#include <objmgr/util/sequence.hpp>

#include <gui/widgets/text_widget/text_item_panel.hpp>
#include <gui/widgets/text_widget/draw_styled_text.hpp>

#include <gui/widgets/seq/fasta_view_params.hpp>
#include <gui/widgets/seq/fasta_seqblock_item.hpp>

#include <wx/dcclient.h>
#include <wx/txtstrm.h>

BEGIN_NCBI_SCOPE
USING_SCOPE(objects);

CFastaSeqBlockItem::CFastaSeqBlockItem(const objects::CBioseq_Handle& h, size_t from, size_t length)
        : m_Bioseq(h), m_From(from), m_Length(length)
{
    CRef<CSeq_loc> loc(new CSeq_loc());
    loc->SetInt().SetFrom(m_From);
    loc->SetInt().SetTo(m_From + m_Length - 1);

    try {
        loc->SetId(*m_Bioseq.GetSeqId());
        m_Loc = loc;
    }
    catch (const CObjMgrException&) {
    }
}

size_t CFastaSeqBlockItem::GetLineCount() const
{
    return (m_Length + kFastaSeqLine - 1)/kFastaSeqLine;
}

void CFastaSeqBlockItem::CalcSize(wxDC& /*dc*/, CTextPanelContext* context)
{
    size_t lineCount = (m_Length + kFastaSeqLine - 1)/kFastaSeqLine;
    wxSize size = context->GetWSize();
    m_Size.Set(context->GetLeftMargin() + size.x*kFastaSeqLine, size.y*lineCount);
}

void CFastaSeqBlockItem::x_RenderText(CStyledTextOStream& ostream, CTextPanelContext* /*context*/) const
{
    static CTextStyle errStyle(255,0,0,255,255,255,wxTRANSPARENT,false,false);
    static string err_msg_saved;
    string seq, err_msg;

    try {
        CSeqVector v_seq = m_Bioseq.GetSeqVector();
        v_seq.SetIupacCoding();
        v_seq.GetSeqData(m_From, m_From + m_Length, seq);

        if (seq.length() < m_Length)
            seq.append(string(m_Length - seq.length(), '-'));

    }
    catch (const CException& e) {
        err_msg = e.GetMsg();
    }


    size_t i, from, length,
        lineCount = (m_Length + kFastaSeqLine - 1)/kFastaSeqLine;

    if (err_msg.empty()) {
        ostream << DefaultStyle();

        for (i = 0, from = 0; i < lineCount; ++i, from += kFastaSeqLine) {
            length = (i + 1 == lineCount) ? m_Length - from : kFastaSeqLine;
            ostream << seq.substr(from, length) << NewLine();
        }
    }
    else {
        ostream << errStyle;

        for (i = 0, from = 0; i < lineCount; ++i, from += kFastaSeqLine) {
            length = (i + 1 == lineCount) ? m_Length - from : kFastaSeqLine;
            if (i ==  0)
                ostream << err_msg << NewLine();
            else
                ostream << string(length, '-') << NewLine();
        }

        if (err_msg_saved != err_msg) {
            err_msg_saved = err_msg;
            LOG_POST(Error << "CException in CFastaSeqBlockItem::x_RenderText: " << err_msg);
        }

        ostream << DefaultStyle();
    }
}

const CConstRef<CObject> CFastaSeqBlockItem::GetAssosiatedObject() const
{
    return CConstRef<CObject>(m_Loc.GetPointerOrNull());
}

END_NCBI_SCOPE


