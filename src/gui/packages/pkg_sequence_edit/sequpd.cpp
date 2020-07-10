/*  $Id: sequpd.cpp 43960 2019-09-27 14:43:29Z asztalos $
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
 * Authors:  Andrea Asztalos
 */


#include <ncbi_pch.hpp>

#include <gui/packages/pkg_sequence_edit/sequpd.hpp>
#include <gui/packages/pkg_sequence_edit/sequpd_except.hpp>

#include <objects/general/User_object.hpp>
#include <objects/general/User_field.hpp>
#include <objects/general/Object_id.hpp>
#include <objects/seqalign/Seq_align_set.hpp>
#include <serial/iterator.hpp>

#include <objmgr/object_manager.hpp>
#include <objmgr/seqdesc_ci.hpp>
#include <objmgr/bioseq_ci.hpp>
#include <objmgr/impl/synonyms.hpp>
#include <objmgr/scope.hpp>
#include <objmgr/seq_vector.hpp>

#include <algo/align/ngalign/ngalign.hpp>
#include <algo/align/ngalign/sequence_set.hpp>
#include <algo/align/ngalign/merge_aligner.hpp>
#include <algo/align/ngalign/blast_aligner.hpp>
#include <algo/align/ngalign/alignment_scorer.hpp>
#include <algo/align/ngalign/alignment_filterer.hpp>

#include <algo/align/nw/nw_aligner.hpp>
#include <algo/align/nw/nw_formatter.hpp>
#include <algo/blast/api/blast_prot_options.hpp>

BEGIN_NCBI_SCOPE
USING_SCOPE(objects);

CSeq_id_Handle sequpd::GetGoodSeqIdHandle(const CBioseq_Handle& bsh)
{
    CSeq_id_Handle idh = bsh.GetAccessSeq_id_Handle();
    if (!idh.IsGi()) {
        return idh;
    }

    CSeq_id_Handle gb_idh, lcl_idh;
    CConstRef<CSynonymsSet> synonyms = bsh.GetSynonyms();
    if (synonyms && !synonyms->empty()) {
        for (auto& it : *synonyms) {
            if (it.Which() == CSeq_id::e_Genbank) {
                gb_idh = it;
            }
            else if (it.Which() == CSeq_id::e_Local) {
                lcl_idh = it;
            }
        }
    }

    if (gb_idh) {
        idh.Swap(gb_idh);
    }
    else if (lcl_idh) {
        idh.Swap(lcl_idh);
    }

    return idh;
}

bool sequpd::HaveIdenticalResidues(const objects::CBioseq_Handle& bsh1, const objects::CBioseq_Handle& bsh2)
{
    if (!bsh1 && !bsh2) {
        return true;
    }
    else if (!bsh1 || !bsh2) {
        return false;
    }
    else if (bsh1.GetBioseqLength() != bsh2.GetBioseqLength()) {
        return false;
    }
    else if (bsh1.IsNucleotide() && bsh2.IsProtein()) {
        return false;
    }
    else if (bsh1.IsProtein() && bsh2.IsNucleotide()) {
        return false;
    }

    CSeqVector old_seqvec = bsh1.GetSeqVector(CBioseq_Handle::eCoding_Iupac, eNa_strand_plus);
    CSeqVector upd_seqvec = bsh2.GetSeqVector(CBioseq_Handle::eCoding_Iupac, eNa_strand_plus);

    string old_seq;
    old_seqvec.GetSeqData(0, bsh1.GetBioseqLength(), old_seq);
    string upd_seq;
    upd_seqvec.GetSeqData(0, bsh2.GetBioseqLength(), upd_seq);

    return NStr::EqualNocase(old_seq, upd_seq);
}

static void s_FixCollidingIDs_Annot(CBioseq& bseq, CSeq_entry::TAnnot& annot, const vector<CRef<CSeq_id>>& upd_ids)
{
    CRef<CSeq_id> new_id = bseq.GetId().front();
    NON_CONST_ITERATE(CSeq_entry::TAnnot, ait, annot) {
        for (CTypeIterator<CSeq_id> id_iter(**ait); id_iter; ++id_iter) {
            CSeq_id& id = *id_iter;
            for (auto& it : upd_ids) {
                if (id.Compare(it.GetObject()) == CSeq_id::e_YES) {
                    id.Assign(*new_id);
                    break;
                }
            }
        }
    }
}

static const char* kUpdateSuffix = "_update";

void sequpd::FixCollidingIDs_Bioseq(CBioseq& bseq, const CBioseq::TId& seq_ids)
{
    bool has_conflict = false;
    for (auto& upd_id : bseq.GetId()) {
        for (auto& old_id : seq_ids) {
            if (upd_id->Compare(*old_id) == CSeq_id::e_YES) {
                has_conflict = true;
                break;
            }
        }
        if (has_conflict) {
            break;
        }
    }

    if (!has_conflict) {
        return;
    }

    // save the original IDs of the update sequence
    vector<CRef<CSeq_id>> update_ids;
    for (auto& it : bseq.GetId()) {
        CRef<CSeq_id> newid(new CSeq_id);
        newid->Assign(it.GetObject());
        update_ids.push_back(newid);
    }

    string lclID_label, gbID_label, accID_label;
    if (bseq.GetId().size() == 1) {
        bseq.GetId().front()->GetLabel(&gbID_label, CSeq_id::eContent);
    }
    else {
        ITERATE(CBioseq::TId, upd_id, bseq.GetId()) {
            if ((*upd_id)->IsLocal()) {
                (*upd_id)->GetLabel(&lclID_label, CSeq_id::eContent);
            }
            else if ((*upd_id)->IsGenbank()) {
                (*upd_id)->GetLabel(&gbID_label, CSeq_id::eContent);
            }
            else if ((*upd_id)->IsOther()) {
                int version;
                (*upd_id)->GetLabel(&accID_label, &version, CSeq_id::eContent);
                accID_label.push_back('.');
                accID_label.append(NStr::NumericToString(version));
            }
        }
    }

    CRef<CSeq_id> newUpdate_Id;
    if (!gbID_label.empty()) {
        newUpdate_Id.Reset(new CSeq_id(CSeq_id::e_Local, gbID_label + kUpdateSuffix));
    }
    else if (!lclID_label.empty()) {
        newUpdate_Id.Reset(new CSeq_id(CSeq_id::e_Local, lclID_label + kUpdateSuffix));
    }
    else if (!accID_label.empty()) {
        newUpdate_Id.Reset(new CSeq_id(CSeq_id::e_Local, accID_label + kUpdateSuffix));
    }

    if (!newUpdate_Id) {
        NCBI_THROW(CSeqUpdateException, eInternal, "Update IDs could not be fixed.");
    }

    bseq.ResetId();
    bseq.SetId().push_back(newUpdate_Id);
    s_FixCollidingIDs_Annot(bseq, bseq.SetAnnot(), update_ids);

    CSeq_entry* parent = bseq.GetParentEntry();
    if (!parent) {
        return;
    }

    CSeq_entry* parent_set = parent->GetParentEntry();
    if (parent_set && parent_set->IsSet() && parent_set->GetSet().GetClass() == CBioseq_set::eClass_nuc_prot) {
        s_FixCollidingIDs_Annot(bseq, parent_set->SetAnnot(), update_ids);
    }
}

// return the matching old sequence for the update sequence if there is one
static CBioseq_Handle s_GetMatchingSequence(CSeq_inst::EMol type, CSeq_entry_Handle& oldSeq, const CBioseq& upd_bseq, bool& collide, const sequpd::TSeqIdHMap& matches);

void sequpd::FindMatches(CSeq_inst::EMol type, CSeq_entry_Handle& oldSeq, CSeq_entry& updEntry, TSeqIdHMap& matches, TSeqIdHVector& unmatched)
{
    for (CTypeIterator<CBioseq> it(updEntry); it; ++it) {
        CBioseq& updSeq = *it;
        if (updSeq.GetInst().IsSetMol()) {
            if (!(CSeq_inst::IsNa(updSeq.GetInst().GetMol()) && CSeq_inst::IsNa(type)) &&
                !(CSeq_inst::IsAa(updSeq.GetInst().GetMol()) && CSeq_inst::IsAa(type))) {
                continue;
            }
        }
        else {
            // no matching sequence has been found for the update sequence
            CSeq_id_Handle idh = CSeq_id_Handle::GetHandle(*updSeq.GetId().front());
            unmatched.push_back(idh);
        }

        bool collide = false;
        CBioseq_Handle match_bsh = s_GetMatchingSequence(type, oldSeq, updSeq, collide, matches);
        if (match_bsh) {
            if (collide) {
                // fix ID conflicts
                const CBioseq::TId& old_ids = match_bsh.GetCompleteBioseq()->GetId();
                sequpd::FixCollidingIDs_Bioseq(updSeq, old_ids);
            }

            CSeq_id_Handle old_idh = sequpd::GetGoodSeqIdHandle(match_bsh);
            if (matches.find(old_idh) != matches.end()) {
                NCBI_THROW(CSeqUpdateException, eReading, "Non-unique sequence IDs in update sequences!");
            }
            else {
                CSeq_id_Handle upd_idh = CSeq_id_Handle::GetHandle(*updSeq.GetId().front());
                matches.emplace(old_idh, upd_idh);
            }
        }
        else {
            // no matching sequence has been found for the update sequence
            CSeq_id_Handle idh = CSeq_id_Handle::GetHandle(*updSeq.GetId().front());
            unmatched.push_back(idh);
        }
    }
}

static bool s_MatchSeqIds(CScope& scope, const CSeq_id& old_id, const CSeq_id& upd_id, bool& collide);

static CBioseq_Handle s_GetMatchingSequence(CSeq_inst::EMol type, CSeq_entry_Handle& oldSeq, const CBioseq& upd_bseq, bool& collide, const sequpd::TSeqIdHMap& matches)
{
    for (CBioseq_CI b_iter(oldSeq, type); b_iter; ++b_iter) {
        CSeq_id_Handle old_id_handle = sequpd::GetGoodSeqIdHandle(*b_iter);
        if (matches.find(old_id_handle) != matches.end())
            continue;

        CConstRef<CBioseq> oldBseq = b_iter->GetBioseqCore();
        if (oldBseq->IsSetId() && upd_bseq.IsSetId()) {
            for (auto& id1 : oldBseq->GetId()) {
                for (auto& id2 : upd_bseq.GetId()) {
                    if (s_MatchSeqIds(oldSeq.GetScope(), *id1, *id2, collide))
                        return *b_iter;
                }
            }
        }
    }
    return CBioseq_Handle();
}

static CConstRef<CSeq_id> s_GetOriginalId(const CBioseq_Handle& bsh);

static bool s_MatchSeqIds(CScope& scope, const CSeq_id& old_id, const CSeq_id& upd_id, bool& collide)
{
    collide = false;
    CSeq_id::E_SIC cmp_type = old_id.Compare(upd_id);
    if (cmp_type == CSeq_id::e_YES) {
        collide = true;
        return true;
    }
    else if (cmp_type == CSeq_id::e_DIFF) { // different types, compare their contents
        string old_label(kEmptyStr), upd_label(kEmptyStr);
        old_id.GetLabel(&old_label, CSeq_id::eContent);
        upd_id.GetLabel(&upd_label, CSeq_id::eContent);
        SIZE_TYPE old_dot = NStr::Find(old_label, ".");
        SIZE_TYPE upd_dot = NStr::Find(upd_label, ".");
        if (old_dot != NPOS) {
            old_label = old_label.substr(0, old_dot);
        }
        if (upd_dot != NPOS) {
            upd_label = upd_label.substr(0, upd_dot);
        }
        if (NStr::EqualCase(old_label, upd_label)) {
            return true;
        }
    }
    else if (cmp_type == CSeq_id::e_NO && old_id.IsLocal() && upd_id.IsLocal()) {
        CConstRef<CSeq_id> orig_id = s_GetOriginalId(scope.GetBioseqHandle(old_id));
        if (orig_id && orig_id->Compare(upd_id) == CSeq_id::e_YES) {
            return true;
        }
    }

    return false;
}

static CConstRef<CSeq_id> s_GetOriginalId(const CBioseq_Handle& bsh)
{
    if (bsh.IsAa()) {
        return bsh.GetLocalIdOrNull();
    }

    CConstRef<CSeq_id> orig_id;
    for (CSeqdesc_CI desc_it(bsh, CSeqdesc::e_User, 1); desc_it; ++desc_it) {
        const CUser_object& usr = desc_it->GetUser();
        if (usr.GetObjectType() == CUser_object::eObjectType_OriginalId) {
            ITERATE(CUser_object::TData, it, usr.GetData()) {
                if ((*it)->IsSetLabel() && (*it)->GetLabel().IsStr()
                    && NStr::EqualNocase((*it)->GetLabel().GetStr(), "LocalId")
                    && (*it)->IsSetData()
                    && (*it)->GetData().IsStr()) {

                    string id_str = (*it)->GetData().GetStr();
                    orig_id.Reset(new CSeq_id(CSeq_id::e_Local, id_str));
                    break;
                }
            }
        }
    }

    return orig_id;
}

static vector<CConstRef<CSeq_align> > s_RunBlast2NASeq(const CBioseq_Handle& sh, const CBioseq_Handle& qh, bool accept_atleast_one, ICanceled* canceled);
static vector<CConstRef<CSeq_align> > s_RunBlast2NWSeq(const CBioseq_Handle& sh, const CBioseq_Handle& qh);
static vector<CConstRef<CSeq_align> > s_RunBlast2AASeq(const CBioseq_Handle& sh, const CBioseq_Handle& qh);

vector<CConstRef<CSeq_align> > sequpd::RunBlast2Seq(const CBioseq_Handle& sh, const CBioseq_Handle& qh, bool accept_atleast_one, ICanceled* canceled)
{
    if (!sh || !qh)
        return vector<CConstRef<CSeq_align> >();

    if (sh.IsNucleotide() && qh.IsNucleotide()) {
        return s_RunBlast2NASeq(sh, qh, accept_atleast_one, canceled);
    }
    else if (sh.IsProtein() && qh.IsProtein()) {
        return s_RunBlast2NWSeq(sh, qh);
    }
    else {
        NCBI_THROW(CSeqUpdateException, eAlignment, "Mismatch in sequence type, cannot form alignment.");
    }

    return vector<CConstRef<CSeq_align> >();
}

namespace
{
    class CGPipeAlignmentScorer : public IAlignmentScorer {
    public:
        enum EAlignScoreTypes {
            /// add BLAST-style 'num_ident' score
            fScore_Identities = 0x001,

            /// add a 'mismatch' core with a count of mismatches
            fScore_Mismatches = 0x002,

            /// add a 'gap_count' score
            fScore_GapCount = 0x004,

            /// add scores for ungapped and gapped percent identity
            fScore_PercentIdentity = 0x008,

            /// add a score for percent coverage of query (sequence 0)
            fScore_PercentCoverage = 0x010,

            /// default flags: everything
            fScore_Default = 0xffffffff
        };

        CGPipeAlignmentScorer() {}

        void ScoreAlignments(TAlignResultsRef results, CScope& scope)
        {
            NON_CONST_ITERATE(CAlignResultsSet::TQueryToSubjectSet,
                result_iter, results->Get()) {
                NON_CONST_ITERATE(CQuerySet::TAssemblyToSubjectSet,
                    assm_iter, result_iter->second->Get()) {
                    NON_CONST_ITERATE(CQuerySet::TSubjectToAlignSet,
                        query_iter, assm_iter->second) {
                        NON_CONST_ITERATE(CSeq_align_set::Tdata, it, query_iter->second->Set()) {
                            CSeq_align& align = **it;
                            s_AddStandardAlignmentScores(scope, align, fScore_Default);

                            /// additionally, add the gaponly version, used in gbDNA
                            CScoreBuilder sb;
                            sb.AddScore(scope, align, CSeq_align::eScore_PercentIdentity_GapOpeningOnly);
                        }
                    }
                }
            }
        }
    private:
        static void s_AddStandardAlignmentScores(CScope& scope, CSeq_align& align, int flags);
    };

    void CGPipeAlignmentScorer::s_AddStandardAlignmentScores(CScope& scope, CSeq_align& align, int flags)
    {
        CScoreBuilder sb;

        if (flags & (fScore_Identities | fScore_Mismatches | fScore_PercentIdentity)) {
            if (flags & fScore_PercentIdentity) {
                /// this automatically adds num_ident and num_mismatch
                sb.AddScore(scope, align,
                    CSeq_align::eScore_PercentIdentity_Gapped);
                sb.AddScore(scope, align,
                    CSeq_align::eScore_PercentIdentity_Ungapped);
            }
            else if (flags & fScore_Identities) {
                sb.AddScore(scope, align, CSeq_align::eScore_IdentityCount);
            }
            else if (flags & fScore_Mismatches) {
                sb.AddScore(scope, align, CSeq_align::eScore_MismatchCount);
            }
        }

        if (flags & fScore_GapCount) {
            /// FIXME: add eScore_GapCount to CSeq_align, CScoreBuilder
            //sb.AddScore(scope, align, CScoreBuilder::eScore_GapCount);
            int gap_count = sb.GetGapCount(align);
            align.SetNamedScore("gap_count", gap_count);
        }

        if (flags & fScore_PercentCoverage) {
            sb.AddScore(scope, align, CSeq_align::eScore_PercentCoverage);
        }
    }
} // namespace

static vector<CConstRef<CSeq_align> > s_RunBlast2NASeq(const CBioseq_Handle& sh, const CBioseq_Handle& qh, bool accept_atleast_one, ICanceled* canceled)
{
    vector<CConstRef<CSeq_align> > align_vector;

    if (&(sh.GetScope()) != &(qh.GetScope())) {
        LOG_POST(Error << "Both sequences should be in the same scope");
        return align_vector;
    }

    // both sequences should be in the same scope
    CNgAligner ng_aligner(sh.GetScope());

    CRef<CSeq_loc> query_seqloc = qh.GetRangeSeq_loc(0, 0);
    CRef<CSeq_loc> subject_seqloc = sh.GetRangeSeq_loc(0, 0);
    if (query_seqloc.IsNull() || subject_seqloc.IsNull()) {
        return align_vector;
    }

    CRef<CSeqLocListSet> query(new CSeqLocListSet());
    query->SetLocList().push_back(query_seqloc);
    ng_aligner.SetQuery(query);

    CRef<CSeqLocListSet> subject(new CSeqLocListSet());
    subject->SetLocList().push_back(subject_seqloc);
    ng_aligner.SetSubject(subject);

    auto cb = [](SBlastProgress* prog) -> Boolean
    {
        if (!prog || !prog->user_data)
            return false;
        return reinterpret_cast<ICanceled*>(prog->user_data)->IsCanceled();
    };

    TSeqPos seqLength = sh.GetBioseqLength();
    bool useHiWordAligner = (seqLength > 12000);
    if (useHiWordAligner) {
        CRef<blast::CBlastNucleotideOptionsHandle> opts(new blast::CBlastNucleotideOptionsHandle);
        opts->SetTraditionalBlastnDefaults();
        blast::CBlastOptions& options = opts->SetOptions();

        options.SetWordSize(1200);
        options.SetEvalueThreshold(1e-6);
        options.SetBestHitOverhang(0.1);  // best_hit_score_edge
        options.SetBestHitScoreEdge(0.1);  // best_hit_overhang

        CRef<CBlastAligner> blastAligner(new CBlastAligner(*opts, 0));
        if (canceled)
            blastAligner->SetInterruptCallback(cb, canceled);

        ng_aligner.AddAligner(blastAligner);
        ng_aligner.AddAligner(new CMergeAligner(1, CMergeAligner::eTreeAlignMerger));
    }

    CRef<blast::CBlastNucleotideOptionsHandle> opts(new blast::CBlastNucleotideOptionsHandle);
    opts->SetTraditionalBlastnDefaults();
    blast::CBlastOptions& options = opts->SetOptions();

    options.SetWordSize(12);
    options.SetEvalueThreshold(1e-6);
    options.SetBestHitOverhang(0.1);  // best_hit_score_edge
    options.SetBestHitScoreEdge(0.1);  // best_hit_overhang

    CRef<CBlastAligner> blastAligner(new CBlastAligner(*opts, useHiWordAligner ? 1 : 0));

    if (canceled)
        blastAligner->SetInterruptCallback(cb, canceled);

    ng_aligner.AddAligner(blastAligner);
    ng_aligner.AddAligner(new CMergeAligner(1, CMergeAligner::eTreeAlignMerger));
    //ng_aligner.AddAligner(new CInversionMergeAligner(1));

    // adding scores
    ng_aligner.AddScorer(new CBlastScorer(CBlastScorer::eSkipUnsupportedAlignments));
    ng_aligner.AddScorer(new CGPipeAlignmentScorer());
    ng_aligner.AddScorer(new CCommonComponentScorer());

    // add filters
    ng_aligner.AddFilter(new CQueryFilter(0, "pct_identity_gapopen_only >= 99.5 AND pct_coverage >= 99"));
    ng_aligner.AddFilter(new CQueryFilter(1, "pct_identity_gapopen_only >= 95 AND pct_coverage >= 95"));
    ng_aligner.AddFilter(new CQueryFilter(2, "pct_identity_gapopen_only >= 95 AND pct_coverage >= 50"));
    ng_aligner.AddFilter(new CQueryFilter(3, "pct_identity_gapopen_only >= 80 AND pct_coverage >= 25"));
    if (accept_atleast_one) {
        ng_aligner.AddFilter(new CQueryFilter(4, "align_length > 2"));
    }

    CRef<CSeq_align_set> align = ng_aligner.Align();

    if (align && align->IsSet()) {
        ITERATE(CSeq_align_set::Tdata, al_it, align->Get()) {
            align_vector.push_back(CConstRef<CSeq_align>((*al_it)));
        }
    }

    return align_vector;
}

static vector<CConstRef<CSeq_align> > s_RunBlast2NWSeq(const CBioseq_Handle& sh, const CBioseq_Handle& qh)
{
    vector<CConstRef<CSeq_align> > align_vector;

    CSeqVector sVec = sh.GetSeqVector(CBioseq_Handle::eCoding_Iupac, eNa_strand_plus);
    CSeqVector qVec = qh.GetSeqVector(CBioseq_Handle::eCoding_Iupac, eNa_strand_plus);

    string sSeq, qSeq;
    sVec.GetSeqData(0, sVec.size(), sSeq);
    sVec.GetSeqData(0, qVec.size(), qSeq);
    NStr::ToUpper(sSeq);
    NStr::ToUpper(qSeq);

    CRef<CNWAligner> aligner(new CNWAligner(sSeq.c_str(), sSeq.size(), qSeq.c_str(), qSeq.size(), &NCBISM_Blosum62));
    CNWAligner::TScore score = aligner->Run();

    CNWFormatter formatter(*aligner);
    CRef<CSeq_align> align =
        formatter.AsSeqAlign(0, eNa_strand_plus, 0, eNa_strand_plus);

    align->SetSegs().SetDenseg().SetIds()[0]->Assign(*sh.GetSeqId());
    align->SetSegs().SetDenseg().SetIds()[1]->Assign(*qh.GetSeqId());

    align_vector.push_back(align);

    return align_vector;
}

static vector<CConstRef<CSeq_align> > s_RunBlast2AASeq(const CBioseq_Handle& sh, const CBioseq_Handle& qh)
{
    vector<CConstRef<CSeq_align> >  align_vector;

    CRef<CSeq_loc> query_seqloc = qh.GetRangeSeq_loc(0, 0, eNa_strand_plus);
    CRef<CSeq_loc> subject_seqloc = sh.GetRangeSeq_loc(0, 0, eNa_strand_plus);  //is strand correct?

    blast::SSeqLoc query(query_seqloc.GetPointerOrNull(), &qh.GetScope());
    blast::SSeqLoc subject(subject_seqloc.GetPointerOrNull(), &sh.GetScope());

    try {
        blast::CBlastProteinOptionsHandle prot_opts_handle;
        prot_opts_handle.SetEvalueThreshold(1e-6);
        prot_opts_handle.SetWordThreshold(100.0);
        prot_opts_handle.Validate();

        blast::CBl2Seq blaster(query, subject, prot_opts_handle);
        blast::TSeqAlignVector seqaligns = blaster.Run();

        if (!seqaligns.empty()) {
            ITERATE(blast::TSeqAlignVector, it, seqaligns) {
                if ((*it)->IsSet()) {
                    ITERATE(CSeq_align_set::Tdata, al_it, (*it)->Get()) {
                        align_vector.push_back(CConstRef<CSeq_align>((*al_it)));
                    }
                }
            }
        }
    }
    catch (const blast::CBlastException& e) {
        ERR_POST(Error << string(e.what()));
        NCBI_THROW(CSeqUpdateException, eAlignment, "Options or input parameters were not accepted for Blast");
    }

    return align_vector;
}

bool sequpd::CompareAlignments(const CSeq_align& align_first, const CSeq_align& align_sec)
{
    const auto length_first = align_first.GetAlignLength();
    const auto length_sec = align_sec.GetAlignLength();
    return length_first >= length_sec;
}

END_NCBI_SCOPE
