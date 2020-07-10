/*  $Id: tax_tree_ds.cpp 25159 2012-01-27 17:17:16Z wuliangs $
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
 * Authors:  Mike DiCuccio
 *
 * File Description:
 *
 */

#include <ncbi_pch.hpp>
#include <gui/widgets/tax_tree/tax_tree_ds.hpp>
#include <objects/seq/Bioseq.hpp>
#include <objmgr/util/sequence.hpp>
#include <objmgr/seqdesc_ci.hpp>
#include <gui/objutils/label.hpp>

#include <algorithm>
#include <serial/iterator.hpp>


BEGIN_NCBI_SCOPE
USING_SCOPE(objects);


CTaxTreeDS_ObjMgr::CTaxTreeDS_ObjMgr(CScope& scope, const TUidVec& uids)
    : m_Ids(uids)
    , m_Scope(&scope)
    , m_Mode(eDisplay_Default)
{
    m_TaxCache.Init();

    TTaxMap tax_map;
    GetTaxMap(tax_map);

    vector<TTaxId> tax_ids_in;
    vector<TTaxId> tax_ids_out;
    tax_ids_in.reserve(tax_map.size());
    ITERATE(TTaxMap, iter, tax_map) {
        //_TRACE("tax-id: " << iter->first << " seqs: " << iter->second.size());
        tax_ids_in.push_back(iter->first);
    }

    if (tax_ids_in.size() == 0) {
        NCBI_THROW(CException, eUnknown,
                   "Can't generate common tree for sequences:\n"
                   "No taxonomy IDs found.");
    }

    if ( !m_TaxCache.GetPopsetJoin(tax_ids_in, tax_ids_out) ) {
        NCBI_THROW(CException, eUnknown,
                   "Can't generate common tree for sequences");
    }
}


ITreeIterator& CTaxTreeDS_ObjMgr::GetIterator(EDisplayMode mode)
{
    if ( !m_Iter  ||  mode != m_Mode) {
        // first, fill our internal tree structure
        CTaxon1::EIteratorMode iter_mode = CTaxon1::eIteratorMode_Default;
        switch (mode) {
        default:
        case eDisplay_All:
            break;

        case eDisplay_Best:
            iter_mode = CTaxon1::eIteratorMode_Best;
            break;

        case eDisplay_Blast:
            iter_mode = CTaxon1::eIteratorMode_Blast;
            break;
        }

        m_Iter.Reset(m_TaxCache.GetTreeIterator(iter_mode));
        m_Mode = mode;
    }
    return *m_Iter;
}


void CTaxTreeDS_ObjMgr::GetTaxMap(TTaxMap& tax_map)
{
    TUidVec uids;
    GetUids(uids);

    set< CConstRef<CBioseq> > bioseqs;
    ITERATE (TUidVec, iter, uids) {
        TTaxId tax_id = 0;

        // first, see if we can get it from the sequence, and make sure
        // that the sequence hasn't been seen already
        CBioseq_Handle handle = m_Scope->GetBioseqHandle(**iter);
        if (handle  &&  bioseqs.insert(handle.GetBioseqCore()).second == false) {
            continue;
        }

        tax_id = sequence::GetTaxId(handle);

        // if not, try the tax server
        if ( !tax_id ) {
            const CSeq_id& seq_id = **iter;
            if ( seq_id.IsGi() ) {
                m_TaxCache.GetTaxId4GI(seq_id.GetGi(), tax_id);
            }
        }

        // if we've got something, add it to our cache
        if (tax_id) {
            tax_map[tax_id].push_back(*iter);
        } else {
            string str;
            CLabel::GetLabel(**iter, &str, CLabel::eDefault, m_Scope);
            LOG_POST(Info << "No tax-id for sequence: " << str);
        }
    }
}


void CTaxTreeDS_ObjMgr::GetUids(TUidVec& uids)
{
    uids = m_Ids;
}


const CTaxTreeDS_ObjMgr::TUidVec& CTaxTreeDS_ObjMgr::GetUids() const
{
    return m_Ids;
}


void CTaxTreeDS_ObjMgr::GetTitle(const CSeq_id& id, string* title) const
{
    if (title) {
        title->erase();

        CBioseq_Handle handle = m_Scope->GetBioseqHandle(id);
        CConstRef<CSeq_id> best_id = handle.GetSeqId();
        CSeq_id_Handle idh = sequence::GetId(handle, sequence::eGetId_Best);
        if (idh) {
            best_id = idh.GetSeqId();
        }

        best_id->GetLabel(title, CSeq_id::eContent);
    }
}


void CTaxTreeDS_ObjMgr::GetTitle(const ITaxon1Node& node, string* title) const
{
    if (title) {
        *title = node.GetName();
    }
}


const CRef<objects::CScope> &  CTaxTreeDS_ObjMgr::GetScope(void)
{
    return m_Scope;
}


END_NCBI_SCOPE
