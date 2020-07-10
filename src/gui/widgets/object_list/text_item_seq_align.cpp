/*  $Id: text_item_seq_align.cpp 32647 2015-04-07 16:12:16Z katargir $
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
 * Authors: Roman Katargin
 *
 * File Description:
 *
 */

#include <ncbi_pch.hpp>

#include "text_item_seq_align.hpp"

#include <gui/objutils/label.hpp>

#include <objects/seqalign/Seq_align.hpp>
#include <objects/seq/seq_id_handle.hpp>
#include <objects/seqloc/Seq_id.hpp>
#include <objects/seqalign/Seq_align_set.hpp>

#include <objmgr/util/sequence.hpp>
#include <serial/iterator.hpp>

BEGIN_NCBI_SCOPE
USING_SCOPE(objects);

ITextItem* CTextItemSeq_align::CreateObject(SConstScopedObject& object, EMode mode)
{
    return new CTextItemSeq_align(object, mode);
}

static void s_Render_PairwiseAlign(
    CStyledTextOStream& ostream, const CSeq_align& align, CScope& scope)
{
    switch( align.GetSegs().Which() ){
    case CSeq_align::TSegs::e_Denseg:
        break;

    case CSeq_align::TSegs::e_Std:
        break;

    case CSeq_align::TSegs::e_Disc:
        {{
            /// each segment represents an exon, so we can format a text dump
            /// with this information
            ITERATE( CSeq_align::TSegs::TDisc::Tdata, iter, align.GetSegs().GetDisc().Get() ){
            }
        }}
        break;

    default:
        string text;
        CLabel::GetLabel( align, &text, CLabel::eDescription, &scope );
        if (!text.empty()) {
            ostream << NewLine();
            ostream.WriteMultiLineText(text);
        }
        break;
    }
}

static void s_Render_GenericAlign(
    CStyledTextOStream& ostream, const CSeq_align& align, CScope& scope)
{
    string text;
    CLabel::GetLabel( align, &text, CLabel::eDescription, &scope );
    ostream.WriteMultiLineText(text);
}

static void s_Render_SeqAlign(
    CStyledTextOStream& ostream, const CSeq_align& align, CScope& scope)
{
    ostream << "Alignment: ";

    set<CSeq_id_Handle> id_set;
    bool first = true;
    CTypeConstIterator<CSeq_id> id_iter( align );
    for( ; id_iter; ++id_iter ){
        CSeq_id_Handle idh = CSeq_id_Handle::GetHandle( *id_iter );
        if( id_set.insert(idh).second ){
            if( !first ){
                ostream <<  " x ";
            }

            idh = sequence::GetId( idh, scope, sequence::eGetId_Best );
            if (!idh) {
                idh = CSeq_id_Handle::GetHandle( *id_iter );
            }
            string label;
            idh.GetSeqId()->GetLabel( &label );
            if (!label.empty()) {
                ostream <<  label;
                first = false;
            }
        }
    }

    if (id_set.size() == 2) {
        s_Render_PairwiseAlign(ostream, align, scope);

    } else {
        s_Render_GenericAlign(ostream, align, scope);
    }
}

void CTextItemSeq_align::x_RenderText(CStyledTextOStream& ostream, CTextPanelContext* /*context*/) const
{
    x_Indent(ostream);

    if (m_Mode == kBrief) {
        string text;
        CLabel::GetLabel(*m_Object, &text, CLabel::eDescription, m_Scope);
        ostream.WriteMultiLineText(text);
        return;
    }

    if (m_Mode != kFull)
        return;

    const objects::CSeq_align* seq_align = 
        dynamic_cast<const objects::CSeq_align*>(m_Object.GetPointer());
    if (!seq_align) {
        ostream 
            << CTextStyle(255,0,0,255,255,255,wxTRANSPARENT,false,false)
            << "Invalid object: should be Seq-align";
        return;
    }

    s_Render_SeqAlign(ostream, *seq_align, *m_Scope);
}

END_NCBI_SCOPE
