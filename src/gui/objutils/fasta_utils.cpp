/*  $Id: fasta_utils.cpp 38910 2017-07-05 19:33:10Z evgeniev $
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
*  and reliability of the software and data,  the NLM and the U.S.
*  Government do not and cannot warrant the performance or results that
*  may be obtained by using this software or data. The NLM and the U.S.
*  Government disclaim all warranties,  express or implied,  including
*  warranties of performance,  merchantability or fitness for any particular
*  purpose.
*
*  Please cite the author in any work or product based on this material.
*
* ===========================================================================
*
* Authors:  Victor Joukov, Vladislav Evgeniev
* 
* File Description: Set of functions that aid the import of FASTA files (sequences/alignments)
*
*/

#include <ncbi_pch.hpp>

#include <gui/objutils/fasta_utils.hpp>

#include <objmgr/bioseq_handle.hpp>
#include <objmgr/scope.hpp>
#include <objmgr/seq_vector.hpp>
#include <objmgr/util/sequence.hpp>
#include <objects/seq/Bioseq.hpp>
#include <objects/seq/Seq_descr.hpp>
#include <objects/seq/seq_id_handle.hpp>
#include <objects/seqset/Seq_entry.hpp>
#include <objects/seqfeat/Org_ref.hpp>
#include <objtools/readers/idmapper.hpp>

BEGIN_NCBI_SCOPE

using namespace objects;

BEGIN_SCOPE(fasta_utils)

void GetSeqsStatus(list<CRef<CSeq_entry> >& entries, CScope& scope, list<ESeqStatus>& stat)
{
    vector<CSeq_id_Handle> ids;
    for (auto it = entries.begin(); it != entries.end(); ++it) {
        CSeq_id_Handle id_hdl;
        CSeq_entry& se = **it;
        if (se.IsSeq()) {
            const CBioseq& bseq = se.GetSeq();
            if (bseq.CanGetId()) {
                id_hdl = CSeq_id_Handle::GetHandle(*(bseq.GetId().front()));
            }
        }
        ids.push_back(id_hdl);
    }
    vector<CBioseq_Handle> bshs = scope.GetBioseqHandles(ids);
    auto bsh_it = bshs.begin();
    for (auto it = entries.begin(); it != entries.end(); ++it, ++bsh_it) {
        ESeqStatus st = eSeqStatus_Local;
        CSeq_entry& se = **it;
        if (se.IsSeq() && *bsh_it) {
            const CBioseq& bseq = se.GetSeq();
            CSeqVector entry_seqvec(bseq, &scope);
            TSeqPos entry_len = entry_seqvec.size();
            CSeqVector wellknown_seqvec(*bsh_it);
            if (entry_len == wellknown_seqvec.size()) {
                string entry_seq, wellknown_seq;
                entry_seqvec.GetSeqData(0, entry_len, entry_seq);
                wellknown_seqvec.GetSeqData(0, entry_len, wellknown_seq);
                if (entry_seq == wellknown_seq)
                    st = eSeqStatus_Wellknown;
                else {
                    st = eSeqStatus_Edited;
                }
            }
            else {
                st = eSeqStatus_Edited;
            }
        }
        stat.push_back(st);
    }
}

CRef<CSeq_entry> ReplaceWellknownSeqs(CSeq_entry& entry, CScope& scope, vector<CConstRef<CSeq_id>> *wellknown_ids, TSeq_idMap *local_ids)
{
    if (entry.IsSeq())
        return CRef<CSeq_entry>(&entry);
    CBioseq_set& old_bss = entry.SetSet();
    CRef<CSeq_entry> new_entry(new CSeq_entry);
    CSeq_entry& new_entry_ref = *new_entry;
    CBioseq_set& bss = new_entry_ref.SetSet();
    list< CRef< CSeq_entry > > &seq_set = bss.SetSeq_set();
    CIdMapper mapper;
    if (old_bss.CanGetSeq_set()) {
        list< CRef< CSeq_entry > > &old_seq_set = old_bss.SetSeq_set();
        list<ESeqStatus> stats;
        GetSeqsStatus(old_seq_set, scope, stats);
        auto st_it = stats.begin();
        for (auto it = old_seq_set.begin(); it != old_seq_set.end(); ++it, ++st_it) {
            CRef<CSeq_entry> se(*it);
            ESeqStatus s = *st_it;
            if (s == eSeqStatus_Edited) {
                CBioseq::TId& ids = se->SetSeq().SetId();
                CSeq_id_Handle sih = CSeq_id_Handle::GetHandle(*(ids.front()));
                if (ids.size() > 1) { // CIdMapper doesn't work, if there's more than one ID
                    auto id = ids.begin();
                    ids.erase(++id, ids.end());
                }

                vector<string> parts;
                string str_id(sih.AsString());
                if (sih.IsGi()) {
                    string acc = sequence::GetAccessionForGi(sih.GetGi(), scope);
                    if (!acc.empty()) str_id = acc;
                }
                NStr::Split(str_id, "|:", parts);
                if (parts.rbegin()->empty()) { // Remove the last part, if it's empty
                    parts.pop_back();
                }
                string lcl_id = "lcl|mod_" + NStr::Join(parts, "-");
                CSeq_id_Handle lcl_hndl = CSeq_id_Handle::GetHandle(lcl_id);
                mapper.AddMapping(sih, lcl_hndl);
                if (nullptr != local_ids) {
                    (*local_ids)[lcl_hndl.GetSeqId()] = sih.GetSeqId();
                }
            }
            if (s != eSeqStatus_Wellknown) {
                seq_set.push_back(se);
            }
            else if (nullptr != wellknown_ids) {
                CSeq_id_Handle sih = sequence::GetId(*(se->GetSeq().GetId().front()), scope, sequence::eGetId_Best);
                wellknown_ids->push_back(sih.GetSeqId());
            }
        }
    }
    if (old_bss.CanGetAnnot()) {
        bss.SetAnnot() = entry.SetSet().SetAnnot();
    }
    mapper.MapObject(bss);
    return new_entry;
}

void UpdateOrgInformation(objects::CSeq_entry& entry, objects::CScope& scope, const TSeq_idMap &local_ids)
{
    if (local_ids.empty())
        return;

    vector<CRef<CSeq_entry> > entries;
    if (entry.IsSeq()) {
        entries.push_back(CRef<CSeq_entry>(&entry));
    }
    else {
        NON_CONST_ITERATE(CBioseq_set::TSeq_set, iter,
            entry.SetSet().SetSeq_set()) {
            entries.push_back(*iter);
        }
    }

    map < CConstRef<CSeq_id>, CBioseq*, CompareSeq_ids> bioseqs;
    vector<CSeq_id_Handle> ids;

    // Prepare the list of "well known" ids
    for (auto &seq : entries) {
        if (!seq->IsSeq())
            continue;

        CSeq_id_Handle id_hdl = sequence::GetId(seq->GetSeq());
        if (!id_hdl)
            continue;

        auto it_id = local_ids.find(id_hdl.GetSeqId());
        if (it_id == local_ids.end())
            continue;

        ids.push_back(CSeq_id_Handle::GetHandle(it_id->second.GetObject()));
        bioseqs[it_id->second] = &seq->SetSeq();
    }

    vector<CBioseq_Handle> bshs = scope.GetBioseqHandles(ids);
    for (auto &bsh : bshs) {
        auto bioseq = bioseqs.find(bsh.GetSeqId());
        if (bioseq == bioseqs.end())
            continue;
        CopyOrg(bsh, *(bioseq->second));
    }

}

void CopyOrg(const CBioseq_Handle &source, CBioseq &destination)
{
    const CBioSource* biosrc(nullptr);
    try {
        biosrc = sequence::GetBioSource(source);
    }
    catch (const CException& ) {
    }
    if (nullptr == biosrc)
        return;

    if (!biosrc->CanGetOrg())
        return;

    CRef<CSeqdesc> descr(new CSeqdesc());
    CRef<COrg_ref> org_copy(new COrg_ref);
    org_copy->Assign(biosrc->GetOrg());
    descr->SetOrg(*org_copy);

    destination.SetDescr().Set().push_back(descr);
}

CRef<CSeq_id> IdentifyLocalId(const string &fasta_id)
{
    CRef<CSeq_id> result;
    if ((fasta_id.find("mod_") == string::npos) && (fasta_id.find("rng_") == string::npos))
        return result;
        
    vector<string> to_replace{ "lcl|", "mod_", "rng_" };
    string id(fasta_id);
    for (const auto &search : to_replace)
        id = NStr::Replace(id, search, "");
    
    vector<string> parts;
    NStr::Split(id, "-", parts);
    for (const auto &item : parts) {
        if (item.length() < 5)
            continue;
        
        try {
            if (CSeq_id::eAcc_local != CSeq_id::IdentifyAccession(id))
                result.Reset(new CSeq_id(item));
        }
        catch (const CException&) {
        }

        break;
    }

    return result;
}

END_SCOPE(fasta_utils)

END_NCBI_SCOPE
