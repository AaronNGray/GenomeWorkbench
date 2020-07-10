/*  $Id: text_item_seq_loc.cpp 37249 2016-12-20 18:04:04Z katargir $
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

#include "text_item_seq_loc.hpp"
#include <gui/widgets/text_widget/expand_item.hpp>
#include <gui/widgets/text_widget/plain_text_item.hpp>


#include <gui/objutils/label.hpp>

#include <objmgr/seq_map_ci.hpp>

#include <objmgr/util/sequence.hpp>
#include <serial/iterator.hpp>

BEGIN_NCBI_SCOPE
USING_SCOPE(objects);

ITextItem* CTextItemSeq_loc::CreateObject(SConstScopedObject& object, EMode mode, ICanceled* cancel)
{
    CTextItemSeq_loc* itemObject = new CTextItemSeq_loc(object, mode, 1);
    if (mode != kFull)
        return itemObject;

    CCompositeTextItem* group = new CCompositeTextItem();
    group->AddItem(new CTextItemSeq_loc(object, mode, 1), false);

    const objects::CSeq_loc* seq_loc = 
        dynamic_cast<const objects::CSeq_loc*>(object.object.GetPointerOrNull());
    CScope* scope = object.scope;

    try {
        CBioseq_Handle bsh = scope->GetBioseqHandle(*seq_loc);
        if (bsh) {
            CSeqMap::const_iterator segI = 
            bsh.GetSeqMap().ResolvedRangeIterator (scope,
                                            seq_loc->GetTotalRange().GetFrom(),
                                            seq_loc->GetTotalRange().GetLength(),
                                            seq_loc->GetStrand(),
                                            -1,
                                            CSeqMap::fFindRef);
            for ( ;  segI;  ++segI ) {  
                if (cancel && cancel->IsCanceled())
                    break;

                if (segI.GetType() != CSeqMap::eSeqRef) 
                    continue;
                CSeq_id_Handle seq_id_handle = segI.GetRefSeqid();
                int c_start = segI.GetRefPosition();
                int c_stop = segI.GetRefEndPosition() - 1;
                CRef<CSeq_loc> s_loc(new CSeq_loc(
                    const_cast<CSeq_id&>(*seq_id_handle.GetSeqId()), 
                    c_start, c_stop,
                    segI.GetRefMinusStrand() ? eNa_strand_minus : eNa_strand_plus));

                SConstScopedObject object(s_loc, scope);
                group->AddItem(new CTextItemSeq_loc(object, kBrief, 2), false);
            }
        }
    } catch (const exception&) {
    }

    if (group->GetItemCount() > 1) {
        return new CExpandItem(itemObject, group, false);
    } else {
        delete group;
        return itemObject;
    }
}

void CTextItemSeq_loc::x_RenderText(CStyledTextOStream& ostream, CTextPanelContext* /*context*/) const
{
    x_Indent(ostream);

    string text;
    CLabel::GetLabel(*m_Object, &text, CLabel::eContent, m_Scope);
    ostream.WriteMultiLineText(text);
    return;
}

END_NCBI_SCOPE
