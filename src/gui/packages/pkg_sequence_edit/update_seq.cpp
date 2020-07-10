/*  $Id: update_seq.cpp 43890 2019-09-13 18:26:49Z asztalos $
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
#include <corelib/ncbiutil.hpp>

#include <objects/seq/seqport_util.hpp>
#include <objects/seqfeat/RNA_ref.hpp>
#include <objects/seqfeat/Trna_ext.hpp>
#include <objects/seqfeat/Cdregion.hpp>
#include <objects/seqfeat/Code_break.hpp>
#include <objects/seqfeat/Gb_qual.hpp>
#include <objects/seqfeat/SeqFeatXref.hpp>
#include <objects/general/Dbtag.hpp>
#include <objects/pub/Pub_set.hpp>
#include <objects/pub/Pub.hpp>
#include <objects/biblio/Cit_sub.hpp>
#include <objects/general/Date.hpp>
#include <objects/seqalign/Dense_seg.hpp>
#include <objects/seq/Seq_descr.hpp>
#include <objects/general/User_object.hpp>
#include <objects/general/Object_id.hpp>

#include <objects/seq/seq_macros.hpp>
#include <objects/seqset/seqset_macros.hpp>
#include <objects/pub/pub_macros.hpp>
#include <objmgr/util/sequence.hpp>

#include <objmgr/seq_vector.hpp>
#include <objmgr/object_manager.hpp>
#include <objmgr/scope.hpp>
#include <objmgr/bioseq_set_handle.hpp>
#include <objmgr/feat_ci.hpp>
#include <objmgr/bioseq_ci.hpp>
#include <objmgr/seqdesc_ci.hpp>
#include <objmgr/seq_annot_ci.hpp>

#include <objtools/edit/string_constraint.hpp>
#include <gui/widgets/wx/message_box.hpp>
#include <gui/widgets/edit/citsub_updater.hpp>
#include <gui/objutils/cmd_add_seqentry.hpp>
#include <gui/objutils/cmd_change_seq_entry.hpp>
#include <gui/packages/pkg_sequence_edit/update_seq.hpp>
#include <gui/packages/pkg_sequence_edit/sequpd_except.hpp>
#include <gui/packages/pkg_sequence_edit/sequpd.hpp>

#include <objtools/edit/cds_fix.hpp>

BEGIN_NCBI_SCOPE
USING_SCOPE(objects);


namespace {
   CRef<CSeq_interval> s_MapInterval(const CSeq_interval& origInterval, const CSeq_id& newId,
        const CSeq_align& align, bool is_circular, CScope& scope, const TSeqPos& newLength)
    {

        CRef<CSeq_loc_Mapper_Base> mapper(new CSeq_loc_Mapper_Base(align, newId));
        mapper->SetMergeAll();

        CBioseq_Handle bsh = scope.GetBioseqHandle(origInterval.GetId());

        const CSeq_align::TDim oldseqRow = CSequenceUpdater::FindRowInAlignment(bsh, align);
        _ASSERT(oldseqRow > -1);
        const CSeq_align::TDim newseqRow = !oldseqRow;

        const int aln_start = align.GetSeqStart(oldseqRow);
        const int aln_stop = align.GetSeqStop(oldseqRow);
        const ENa_strand old_strand = align.GetSeqStrand(oldseqRow);
        const int aln_start_new = align.GetSeqStart(newseqRow);
        const int aln_stop_new = align.GetSeqStop(newseqRow);
        const ENa_strand new_strand = align.GetSeqStrand(newseqRow);

        bool reverse = !SameOrientation(old_strand, new_strand);
        const int orig_start = origInterval.GetStart(eExtreme_Positional);
        const int orig_stop = origInterval.GetStop(eExtreme_Positional);
        
        CRef<CSeq_interval> pTargetInt(new CSeq_interval());
        pTargetInt->SetId().Assign(newId);
        if (!reverse && origInterval.IsSetStrand()) {
            pTargetInt->SetStrand(origInterval.GetStrand());
        }
        
        CSeq_loc source;
        source.SetInt().Assign(origInterval);
        CRef<CSeq_loc> mapped = mapper->Map(source);

        if (mapped && !mapped->IsNull()) {
            pTargetInt->SetFrom(mapped->GetStart(eExtreme_Positional));
            pTargetInt->SetTo(mapped->GetStop(eExtreme_Positional));
            if (reverse && mapped->IsSetStrand()) {
                pTargetInt->SetStrand(mapped->GetStrand());
            }
            if (reverse && mapped->IsPartialStart(eExtreme_Biological)) {
                pTargetInt->SetPartialStart(true, eExtreme_Biological);
            }
            if (reverse && mapped->IsPartialStop(eExtreme_Biological)) {
                pTargetInt->SetPartialStop(true, eExtreme_Biological);
            }
        }
        else {
            pTargetInt->SetFrom(0);
            pTargetInt->SetTo(0);
        }

        int offset = 0;
        if (orig_start < aln_start) {
            if (reverse) {
                offset = aln_stop_new + (aln_start - orig_start);
                if (offset > (int)newLength - 1) {
                    offset = (int)newLength - 1;
                }
                pTargetInt->SetTo(offset);
            }
            else {
                offset = aln_start_new - (aln_start - orig_start);
                if (offset < 0) {
                    offset = 0;
                }
                pTargetInt->SetFrom(offset);
            }
        } else if (orig_start > aln_stop) {
            if (reverse) {
                offset = aln_start_new - (orig_start - aln_stop);
                if (offset < 0) {
                    offset = 0;
                }
                pTargetInt->SetTo(offset);
            }
            else {
                offset = aln_stop_new + (orig_start - aln_stop);
                if (offset > (int)newLength - 1) {
                    offset = (int)newLength - 1;
                }
                pTargetInt->SetFrom(offset);
            }
        }

        
        if (orig_stop < aln_start) {
            if (reverse) {
                offset = aln_stop_new + (aln_start - orig_stop);
                if (offset > (int)newLength - 1) {
                    offset = (int)newLength - 1;
                }
                pTargetInt->SetFrom(offset);
            }
            else {
                offset = aln_start_new - (aln_start - orig_stop);
                if (offset < 0) {
                    offset = 0;
                }
                pTargetInt->SetTo(offset);
            }
        } else if (orig_stop > aln_stop){
            if (reverse) {
                offset = aln_start_new - (orig_stop - aln_stop);
                if (offset < 0) {
                    offset = 0;
                }
                pTargetInt->SetFrom(offset);
            }
            else {
                offset = aln_stop_new + (orig_stop - aln_stop);
                if (offset > (int)newLength - 1) {
                   offset = (int)newLength - 1;
                }
                pTargetInt->SetTo(offset);
            }
        }

        // check for circular topology
        if (is_circular && pTargetInt->GetFrom() > pTargetInt->GetTo()) {
            const TSeqPos start = pTargetInt->GetStart(eExtreme_Biological);
            const TSeqPos stop = pTargetInt->GetStop(eExtreme_Biological);
            pTargetInt->SetFrom(stop);
            pTargetInt->SetTo(start);
        }

        
        if (!pTargetInt->IsSetStrand()) {
            if (reverse) {
                ENa_strand orig_strand = (origInterval.IsSetStrand()) ? origInterval.GetStrand() : eNa_strand_unknown;
                ENa_strand target_strand = Reverse(orig_strand);
                pTargetInt->SetStrand(target_strand);
            }
            else {
                if (origInterval.IsSetStrand()) {
                    pTargetInt->SetStrand(origInterval.GetStrand());
                }
            }
        }

        if (origInterval.IsPartialStart(eExtreme_Positional)) {
            if (reverse) {
                pTargetInt->SetPartialStop(true, eExtreme_Positional);
            } 
            else {
                pTargetInt->SetPartialStart(true, eExtreme_Positional);
            }
        }
        if (origInterval.IsPartialStop(eExtreme_Positional)) {
            if (reverse) {
                pTargetInt->SetPartialStart(true, eExtreme_Positional);
            }
            else {
                pTargetInt->SetPartialStop(true, eExtreme_Positional);
            }
        }

        return pTargetInt;
    }

    CRef<CSeq_loc> s_MapLocation(const CSeq_loc& origLoc, const CSeq_id& newId, const CSeq_align& align, bool is_circular, CScope& scope, const TSeqPos& newLength)
    {
        CRef<CSeq_loc> mappedLoc(new CSeq_loc);
        switch (origLoc.Which()) {
        case CSeq_loc::e_Pnt: {
            const CSeq_point& pnt = origLoc.GetPnt();
            CRef<CSeq_interval> shortInt(new CSeq_interval());
            shortInt->SetId().Assign(pnt.GetId());
            shortInt->SetFrom(pnt.GetPoint());
            shortInt->SetTo(pnt.GetPoint());
            if (pnt.IsSetStrand()) {
                shortInt->SetStrand(pnt.GetStrand());
            }
            if (pnt.IsSetFuzz()) {
                const CInt_fuzz& fuzz = pnt.GetFuzz();
                if (!fuzz.IsLim() || fuzz.GetLim() != CInt_fuzz::eLim_gt) {
                    shortInt->SetFuzz_from().Assign(fuzz);
                }
                if (!fuzz.IsLim() || fuzz.GetLim() != CInt_fuzz::eLim_lt) {
                    shortInt->SetFuzz_to().Assign(fuzz);
                }
            }

            CRef<CSeq_interval> pTargetInt = s_MapInterval(*shortInt, newId, align, is_circular, scope, newLength);
            if (pTargetInt) {
                mappedLoc->SetInt(*pTargetInt);
                return mappedLoc;
            }
            break;
        }
        case CSeq_loc::e_Int: {
            CRef<CSeq_interval> pTargetInt = s_MapInterval(origLoc.GetInt(), newId, align, is_circular, scope, newLength);
            if (pTargetInt) {
                mappedLoc->SetInt(*pTargetInt);
                return mappedLoc;
            }
            break;
        }
        case CSeq_loc::e_Packed_int: {
            CPacked_seqint& targetInts = mappedLoc->SetPacked_int();
            bool subloc_added = false;
            for (auto&& cit : origLoc.GetPacked_int().Get()) {
                CRef<CSeq_interval> sub_interval = s_MapInterval(*cit, newId, align, is_circular, scope, newLength);
                if (sub_interval) {
                    targetInts.AddInterval(*sub_interval);
                    subloc_added = true;
                }
            }
            if (subloc_added) return mappedLoc;
            break;
        }
        case CSeq_loc::e_Mix: {
            CSeq_loc_mix& targetMix = mappedLoc->SetMix();
            bool subloc_added = false;
            for (auto&& cit : origLoc.GetMix().Get()) {
                CRef<CSeq_loc> target_loc = s_MapLocation(*cit, newId, align, is_circular, scope, newLength);
                if (target_loc) {
                    targetMix.AddSeqLoc(*target_loc);
                    subloc_added = true;
                }
            }
            if (subloc_added) return mappedLoc;
            break;
        }
        default: {
            break;
        }
        }
        return CRef<CSeq_loc>();
    }

    bool SameStrand(const CSeq_loc& loc1, const CSeq_loc& loc2)
    {
        ENa_strand s1 = loc1.GetStrand();
        ENa_strand s2 = loc2.GetStrand();
        if ((s1 == eNa_strand_minus && s2 == eNa_strand_minus)
            || (s1 != eNa_strand_minus && s2 != eNa_strand_minus)) {
            return true;
        }
        else {
            return false;
        }
    }

    bool AreFeaturesDuplicates(const CSeq_feat& feat1, const CSeq_feat& feat2)
    {
        // if the feattypes are the same and the locations are identical, the features are considered to be duplicates
        if (feat1.GetData().GetSubtype() != feat2.GetData().GetSubtype()) {
            return false;
        }

        const CSeq_loc& loc1 = feat1.GetLocation();
        const CSeq_loc& loc2 = feat2.GetLocation();

        if (!SameStrand(loc1, loc2) || loc1.Compare(loc2)) {
            return false;
        }

        return true;
    }

    // the location ID should not be compared when comparing two protein features
    bool AreProteinFeaturesDuplicates(const CSeq_feat& feat1, const CSeq_feat& feat2)
    {
        if (!feat1.GetData().IsProt() || !feat2.GetData().IsProt()) {
            return false;
        }

        bool processed1 = feat1.GetData().GetProt().IsSetProcessed();
        bool processed2 = feat2.GetData().GetProt().IsSetProcessed();
        if ((processed1 && !processed2) || (!processed1 && processed2)) {
            return false;
        }
        if (processed1 && processed2
            && feat1.GetData().GetProt().GetProcessed() != feat2.GetData().GetProt().GetProcessed()) {
            return false;
        }

        const CSeq_loc& loc1 = feat1.GetLocation();
        const CSeq_loc& loc2 = feat2.GetLocation();

        if (!SameStrand(loc1, loc2)) {
            return false;
        }

        CSeq_loc_CI loc1_it(loc1);
        CSeq_loc_CI loc2_it(loc2);
        while (loc1_it && loc2_it) {
            if (loc1_it.GetRange() != loc2_it.GetRange()) {
                return false;
            }
            ++loc1_it;
            ++loc2_it;
        }
        if (loc1_it || loc2_it) {
            return false;
        }

        return true;
    }

#define FUSE_STRINGFIELDS(Var1, Var2, Field) \
        {{ \
        string orig_value = (Var1).IsSet##Field() ? (Var1).Get##Field() : kEmptyStr; \
        string value = (Var2).IsSet##Field() ? (Var2).Get##Field() : kEmptyStr; \
        if (!NStr::EqualNocase(orig_value, value) && edit::AddValueToString(orig_value, value, edit::eExistingText_append_semi)) { \
            (Var1).Set##Field(orig_value); \
                } \
                }}

#define FUSE_BOOLEANFIELDS(Var1, Var2, Field) \
        {{ \
        bool fused_value = (Var1).IsSet##Field() ? (Var1).Get##Field() : false; \
        bool value = (Var2).IsSet##Field() ? (Var2).Get##Field() : false; \
        if (fused_value || value) { \
            (Var1).Set##Field(true); \
                } else { \
            (Var1).Reset##Field(); \
                } \
                }}

#define FUSE_LISTFIELDS(Var1, Var2, Type, Field) \
    if ((Var2).IsSet##Field() && !(Var2).Get##Field().empty()) { \
        C##Type::T##Field& cont = (Var1).Set##Field(); \
        copy((Var2).Get##Field().begin(), (Var2).Get##Field().end(), back_inserter(cont)); \
        }

#define FUSE_VECTORFIELDS(Var1, Var2, Type, Field) \
        {{ \
        if ((Var2).IsSet##Field() && !(Var2).Get##Field().empty()) { \
            size_t new_size = (Var2).Get##Field().size(); \
            new_size += (Var1).IsSet##Field() ? (Var1).Get##Field().size() : 0; \
            C##Type::T##Field& cont = (Var1).Set##Field(); \
            cont.reserve(new_size); \
            copy((Var2).Get##Field().begin(), (Var2).Get##Field().end(), back_inserter(cont)); \
                } \
                }}

    void FuseGenes(CGene_ref& fusedGene, const CGene_ref& gene)
    {
        FUSE_STRINGFIELDS(fusedGene, gene, Locus);
        FUSE_STRINGFIELDS(fusedGene, gene, Allele);
        FUSE_STRINGFIELDS(fusedGene, gene, Desc);
        FUSE_STRINGFIELDS(fusedGene, gene, Maploc);
        FUSE_STRINGFIELDS(fusedGene, gene, Locus_tag);

        FUSE_BOOLEANFIELDS(fusedGene, gene, Pseudo);

        FUSE_VECTORFIELDS(fusedGene, gene, Gene_ref, Db);

        FUSE_LISTFIELDS(fusedGene, gene, Gene_ref, Syn);
    }

    void FuseProteins(CSeq_feat& fusedFeat, const CSeq_feat& feat_old)
    {
        CProt_ref& fusedProtref = fusedFeat.SetData().SetProt();
        const CProt_ref& protref = feat_old.GetData().GetProt();

        FUSE_LISTFIELDS(fusedProtref, protref, Prot_ref, Name);
        FUSE_LISTFIELDS(fusedProtref, protref, Prot_ref, Ec);
        FUSE_LISTFIELDS(fusedProtref, protref, Prot_ref, Activity);
        FUSE_LISTFIELDS(fusedProtref, protref, Prot_ref, Db);

        FUSE_STRINGFIELDS(fusedProtref, protref, Desc);
    }


    void FuseCommonFeatureFields(CSeq_feat& fusedFeat, const CSeq_feat& feat)
    {
        FUSE_STRINGFIELDS(fusedFeat, feat, Comment);
        FUSE_STRINGFIELDS(fusedFeat, feat, Title);
        FUSE_STRINGFIELDS(fusedFeat, feat, Except_text);

        FUSE_VECTORFIELDS(fusedFeat, feat, Seq_feat, Qual);
        FUSE_VECTORFIELDS(fusedFeat, feat, Seq_feat, Dbxref);
        FUSE_VECTORFIELDS(fusedFeat, feat, Seq_feat, Xref);


        if (feat.IsSetCit() && feat.GetCit().IsPub() && !feat.GetCit().GetPub().empty()) {
            CPub_set::TPub& pub = fusedFeat.SetCit().SetPub();
            copy(feat.GetCit().GetPub().begin(), feat.GetCit().GetPub().end(), back_inserter(pub));
        }

        FUSE_BOOLEANFIELDS(fusedFeat, feat, Partial);
        FUSE_BOOLEANFIELDS(fusedFeat, feat, Except);
        FUSE_BOOLEANFIELDS(fusedFeat, feat, Pseudo);
    }

    CSeq_annot_Handle DefaultGetAppropriateFeatureTable(CBioseq_Handle protbsh)
    {
        //check if there is a feature table already
        CSeq_entry_Handle prot_seh = protbsh.GetSeq_entry_Handle();
        CSeq_annot_CI ci(prot_seh, CSeq_annot_CI::eSearch_entry);
        for (/**/; ci; ++ci) {
            if (ci->IsFtable()) {
                return *ci;
            }
        }
        //if not, make a new one
        CBioseq_EditHandle eh = protbsh.GetEditHandle();
        CRef<CSeq_annot> pAnnot(new CSeq_annot());
        return eh.AttachAnnot(*pAnnot);
    }

} // end of namespace

static const char* kNewLocalID = "NewSeq";

CSequenceUpdater::CSequenceUpdater(const CUpdateSeq_Input& updseq_in, const SUpdateSeqParams& params)
    : m_OldBsh(updseq_in.GetNCOldBioseq()), 
      m_UpdBsh(updseq_in.GetUpdateBioseq()), 
      m_Align(updseq_in.GetAlignment()), 
      m_Params(params),
      m_NewEntry(0), 
      m_NewId(0),
      m_Reversed(false)
{
    if (!updseq_in.IsReadyForUpdate() || !IsAlignmentOK()) {
        NCBI_THROW(CSeqUpdateException, eInternal, 
            "Sequence can not be updated: either the update sequence or the alignment is missing");
    }

    if (!CheckParameters()) {
        NCBI_THROW(CSeqUpdateException, eInternal,
            "Sequence can not be updated: update parameters are not valid");
    }

    if (!IsOldSequenceOK()) { // can be raw or delta
        NCBI_THROW(CSeqUpdateException, eInternal,
            "The old sequence can not be updated: unsupported seq-gap");
    }

    // needs to be raw or delta with far pointers
    if (!IsUpdateSequenceRaw() && 
        !CUpdateSeq_Input::s_IsDeltaWithFarPointers(*m_UpdBsh.GetCompleteBioseq())) {
        NCBI_THROW(CSeqUpdateException, eInternal,
            "The update sequence contains gaps features.\nPlease convert your file so that gaps are runs of N's before updating the sequence.");
    }
}

CSequenceUpdater::~CSequenceUpdater()
{
}

void CSequenceUpdater::SetUpdateSeqParams(const SUpdateSeqParams& params)
{
    m_Params = params;
}

bool CSequenceUpdater::IsOldSequenceOK() const
{
    bool old_raw = (m_OldBsh.IsSetInst_Repr() && (m_OldBsh.GetInst_Repr() == CSeq_inst::eRepr_raw));
    bool old_delta = CUpdateSeq_Input::s_IsDeltaWithNoGaps(*m_OldBsh.GetCompleteBioseq()) 
                    || CUpdateSeq_Input::s_IsDeltaWithFarPointers(*m_OldBsh.GetCompleteBioseq());
    return old_raw || old_delta;
}


bool CSequenceUpdater::IsUpdateSequenceRaw() const
{
    return (m_UpdBsh.IsSetInst_Repr() && (m_UpdBsh.GetInst_Repr() == CSeq_inst::eRepr_raw));
}

bool CSequenceUpdater::IsAlignmentOK() const
{
    return (m_Align && m_Align->IsSetSegs() && m_Align->GetSegs().IsDenseg()) || !m_Align;
}

bool CSequenceUpdater::CheckParameters() const
{
    if (!m_Align) {
        SUpdateSeqParams::ESequenceUpdateType update_type = m_Params.m_SeqUpdateOption;
        if (update_type == SUpdateSeqParams::eSeqUpdatePatch) {
            return false;
        }
        else if ((update_type == SUpdateSeqParams::eSeqUpdateExtend3 || update_type == SUpdateSeqParams::eSeqUpdateExtend5)
            && !m_Params.m_IgnoreAlignment) {
            return false;
        }
    }
    return true;
}


CRef<CCmdComposite> CSequenceUpdater::Update(bool create_general_only)
{
    m_NewId.Reset(ConstRef(new CSeq_id(CSeq_id::e_Local, CTempString(kNewLocalID))));

    string descr;
    switch (m_Params.m_SeqUpdateOption) {
    case SUpdateSeqParams::eSeqUpdateNoChange:
        NoChange(create_general_only);
        descr.assign("No change");
        break;
    case SUpdateSeqParams::eSeqUpdateReplace:
        Replace(create_general_only);
        descr.assign("Replace sequence");
        break;
    case SUpdateSeqParams::eSeqUpdatePatch:
        Patch(create_general_only);
        descr.assign("Patch sequence");
        break;
    case SUpdateSeqParams::eSeqUpdateExtend5:
        ExtendOneEndOfSequence(create_general_only);
        descr.assign("Extend 5' end");
        break;
    case SUpdateSeqParams::eSeqUpdateExtend3:
        ExtendOneEndOfSequence(create_general_only);
        descr.assign("Extend 3' end");
        break;
    default:
        break;
    }

    if (m_Params.m_AddCitSub) {
        x_AddCitSubToUpdatedSequence();
    }

    if (m_Align &&
        (m_Params.m_SeqUpdateOption == SUpdateSeqParams::eSeqUpdateNoChange
        || m_Params.m_SeqUpdateOption == SUpdateSeqParams::eSeqUpdateReplace)) {
        m_Reversed = !SameOrientation(m_Align->GetSeqStrand(0), m_Align->GetSeqStrand(1));
    }

    CRef<CCmdComposite> upd_cmd(new CCmdComposite(descr));
    upd_cmd->AddCommand(*x_SwapOldWithNewSeq());

    return upd_cmd;
}

//static
CSeq_align::TDim CSequenceUpdater::FindRowInAlignment(const CBioseq_Handle& bsh, const CSeq_align& align)
{
    CSeq_align::TDim dim = align.CheckNumRows();
    for (CSeq_align::TDim row = 0; row < dim; ++row) {
        const CSeq_id& row_id = align.GetSeq_id(row);
        if (bsh.IsSynonym(row_id)) {
            return row;
        }
    }
    return -1;
}

TSeqPos CSequenceUpdater::x_GetNewSeqLength()
{
    _ASSERT(m_NewEntry);
    TSeqPos newLength = 0;
    if (m_NewEntry->IsSeq()) {
        newLength = m_NewEntry->GetSeq().GetInst().GetLength();
    }
    else {
        FOR_EACH_SEQENTRY_ON_SEQSET(it, m_NewEntry->GetSet()) {
            if ((*it)->IsSeq()) {
                newLength = (*it)->GetSeq().GetLength();
            }
        }
    }
    return newLength;
}

void CSequenceUpdater::NoChange(bool create_general_only)
{
    CRef<CSeq_inst> newInst(new CSeq_inst);
    newInst->Assign(m_OldBsh.GetInst());
    x_MakeNewEntry(*newInst); // it is not being added to any scope at this point

    // prepare old features
    x_ReplaceIdInOldFeatures();

    if (m_Align && m_Params.m_ImportFeatures) {
        // the id in the row corresponding to the OLD sequence should be replaced by newId
        CConstRef<CSeq_align> align = x_GetNewAlign_Replace(m_OldBsh);
        SAnnotSelector sel(m_Params.m_FeatImportType);
        sel.SetLimitTSE(m_UpdBsh.GetTSE_Handle());
        for (CFeat_CI feat_it(m_UpdBsh, sel); feat_it; ++feat_it) {
            if (!x_ShouldImportFeature(*feat_it)) {
                continue;
            }
            CRef<CSeq_feat> import_feat = x_MappedFeature_ThroughAlign(*feat_it, *align);
            if (import_feat) {
                m_ImportUpdFeats.push_back(import_feat);
            }
        }
        x_HandleDuplicateFeatures();
    }

    x_FixID_AttachFeatures(create_general_only);
}

void CSequenceUpdater::Replace(bool create_general_only)
{
    CRef<CSeq_inst> newInst = x_ReplaceSequence();
    x_MakeNewEntry(*newInst);

    if (m_Align) {
        CConstRef<CSeq_align> align = x_GetNewAlign_Replace(m_UpdBsh);
        CSeq_align::TDim oldseqRow = FindRowInAlignment(m_OldBsh, *m_Align);
        SAnnotSelector sel(CSeqFeatData::eSubtype_any);
        sel.SetLimitTSE(m_OldBsh.GetTSE_Handle());
        for (CFeat_CI feat_it(m_OldBsh, sel); feat_it; ++feat_it) {
            if (x_ShouldRemoveFeature(*feat_it)) {
                continue;
            }
            x_ChangeIDInFeature(*feat_it, *align, oldseqRow);
            CRef<CSeq_feat> mapped_feat = x_MappedFeature_ThroughAlign(*feat_it, *align);
            if (mapped_feat) {
                m_MappedOldFeats.push_back(mapped_feat);
            }
        }
    }
    else {
        x_ReplaceIdInOldFeatures();
    }

    if (m_Params.m_ImportFeatures) {
        // keep features in their initial position, only change their IDs
        SAnnotSelector sel(m_Params.m_FeatImportType);
        sel.SetLimitTSE(m_UpdBsh.GetTSE_Handle());
        for (CFeat_CI feat_it(m_UpdBsh, sel); feat_it; ++feat_it) {
            // import each feature as we are replacing the old sequence
            CRef<CSeq_feat> import_feat = x_MappedFeature_ChangeId(*feat_it);
            if (import_feat) {
                m_ImportUpdFeats.push_back(import_feat);
            }
        }
        x_HandleDuplicateFeatures();
    }

    x_FixID_AttachFeatures(create_general_only);
}

void CSequenceUpdater::Patch(bool create_general_only)
{
    _ASSERT(m_Align);
    CRef<CSeq_inst> newInst = x_PatchSequence();
    x_MakeNewEntry(*newInst);

    x_MapOldAndImportedFeatsThroughNewAlign();
 
    x_FixID_AttachFeatures(create_general_only);
}

void CSequenceUpdater::ExtendOneEndOfSequence(bool create_general_only)
{
    CRef<CSeq_inst> newInst = x_ExtendOneEnd();
    x_MakeNewEntry(*newInst);

    if (m_Params.m_IgnoreAlignment) {
        TSeqPos offset_old = 0, offset_import = 0;
        if (m_Params.m_SeqUpdateOption == SUpdateSeqParams::eSeqUpdateExtend5) {
            offset_old = m_UpdBsh.GetBioseqLength();
        }
        else if (m_Params.m_SeqUpdateOption == SUpdateSeqParams::eSeqUpdateExtend3) {
            offset_import = m_OldBsh.GetBioseqLength();
        }
        SAnnotSelector sel(CSeqFeatData::eSubtype_any);
        sel.SetLimitTSE(m_OldBsh.GetTSE_Handle());
        for (CFeat_CI feat_it(m_OldBsh, sel); feat_it; ++feat_it) {
            if (x_ShouldRemoveFeature(*feat_it)) {
                continue;
            }

            CRef<CSeq_feat> mapped_feat = s_OffsetFeature(feat_it->GetOriginalFeature(), offset_old, m_NewId);
            if (mapped_feat) {
                m_MappedOldFeats.push_back(mapped_feat);
            }
        }

        if (m_Params.m_ImportFeatures) {
            SAnnotSelector sel(m_Params.m_FeatImportType);
            sel.SetLimitTSE(m_UpdBsh.GetTSE_Handle());
            for (CFeat_CI feat_it(m_UpdBsh, sel); feat_it; ++feat_it) {
                CRef<CSeq_feat> mapped_feat = s_OffsetFeature(feat_it->GetOriginalFeature(), offset_import, m_NewId);
                if (mapped_feat) {
                    m_ImportUpdFeats.push_back(mapped_feat);
                }
            }
            x_HandleDuplicateFeatures();
        }
    }
    else { // do not ignore alignment
        x_MapOldAndImportedFeatsThroughNewAlign();
    }

    x_FixID_AttachFeatures(create_general_only);
}

void CSequenceUpdater::x_FixID_AttachFeatures(bool create_general_only)
{
    // add original Ids and remove local ID
    x_FixIDInNewEntry(m_NewEntry);
    // attach features to new sequence if there are any
    x_AttachFeaturesToNewSeq(create_general_only);
}

CRef<CSeq_inst> CSequenceUpdater::x_ReplaceSequence()
{

    CRef<CSeq_inst> new_inst(new CSeq_inst());
    new_inst->Assign(m_OldBsh.GetInst());
    
    if (m_OldBsh.GetInst_Repr() == CSeq_inst::eRepr_raw) {
        // do nothing
    } else if (m_OldBsh.GetInst_Repr() == CSeq_inst::eRepr_delta
        && CUpdateSeq_Input::s_IsDeltaWithNoGaps(m_OldBsh.GetCompleteBioseq().GetObject())) {
        new_inst->SetRepr(CSeq_inst::eRepr_raw);
        new_inst->ResetExt();
    }
    else {
        NCBI_THROW(CSeqUpdateException, eInternal, "Could not replace the old sequence: old sequence is neither raw nor it can be converted to raw.");
    }


    if (m_UpdBsh.GetInst_Repr() == CSeq_inst::eRepr_raw) {
        CRef<CSeq_data> new_data(new CSeq_data);
        new_data->Assign(m_UpdBsh.GetInst_Seq_data());
        new_inst->SetSeq_data(*new_data);
        new_inst->SetLength(m_UpdBsh.GetInst_Length());
    } else if (m_UpdBsh.GetInst_Repr() == CSeq_inst::eRepr_delta
            && CUpdateSeq_Input::s_IsDeltaWithNoGaps(m_UpdBsh.GetCompleteBioseq().GetObject())) {

        string seqdata;
        CSeqVector seq_vec = m_UpdBsh.GetSeqVector();
        seq_vec.SetCoding(CSeq_data::e_Iupacna);
        seq_vec.GetSeqData(0, m_UpdBsh.GetInst_Length(), seqdata);
        new_inst->SetSeq_data().SetIupacna().Set(seqdata);
        new_inst->SetLength(TSeqPos(seqdata.length()));
    } else {
        NCBI_THROW(CSeqUpdateException, eInternal, "Could not replace the old sequence: update sequence is neither raw nor it can be converted to raw.");
    }
    
    if (m_OldBsh.IsNucleotide()) {
        CSeqportUtil::Pack(&new_inst->SetSeq_data());
    }
   
    return new_inst;
}

CRef<CSeq_inst> CSequenceUpdater::x_PatchSequence()
{
    CSeq_align::TDim oldseqRow = FindRowInAlignment(m_OldBsh, *m_Align);
    CSeq_align::TDim updseqRow = FindRowInAlignment(m_UpdBsh, *m_Align);
    _ASSERT(oldseqRow != updseqRow);
    _ASSERT(oldseqRow > -1);
    _ASSERT(updseqRow > -1);

    // the start of aligned region in the old sequence coordinates
    TSeqPos aln_start = m_Align->GetSeqStart(oldseqRow);
    // the end of aligned region in the old sequence coordinates
    TSeqPos aln_stop = m_Align->GetSeqStop(oldseqRow);

    // the start of aligned region in the update sequence coordinates
    TSeqPos aln_start_upd = m_Align->GetSeqStart(updseqRow);
    // the end of aligned region in the update sequence coordinates
    TSeqPos aln_stop_upd = m_Align->GetSeqStop(updseqRow);

    const TSeqPos old_length = m_OldBsh.GetBioseqLength();
    const TSeqPos upd_length = m_UpdBsh.GetBioseqLength();

    // Take the whole patch even if the edges are not aligned
    if (aln_start < aln_start_upd)
        aln_start = 0;
    else
        aln_start -= aln_start_upd;
    aln_start_upd = 0;

    TSeqPos right_edge = upd_length - 1 - aln_stop_upd;
    aln_stop += right_edge;
    if (aln_stop > old_length - 1)
        aln_stop = old_length - 1;

    CSeqVector old_seq = m_OldBsh.GetSeqVector(CBioseq_Handle::eCoding_Iupac, eNa_strand_plus);
    CSeqVector upd_seq = m_UpdBsh.GetSeqVector(CBioseq_Handle::eCoding_Iupac, eNa_strand_plus);
    string patched;

    // take old 5' //
    if (aln_start > 0){
        string old_5prime;
        old_seq.GetSeqData(0, aln_start, old_5prime);  //[start, stop)
        patched.append(old_5prime);
    }

    // take aligned middle//
    if (aln_start_upd < aln_stop_upd) {
        string middle;
        upd_seq.GetSeqData(aln_start_upd, aln_stop_upd + 1, middle);
        patched.append(middle);
    }

    // take old 3' (if any) //
    if (aln_stop + 1 < old_length) {
        string old_3prime;
        old_seq.GetSeqData(aln_stop + 1, old_length, old_3prime);
        patched.append(old_3prime);
    }

    return x_UpdateSeqInst(patched);
}

CRef<CSeq_inst> CSequenceUpdater::x_ExtendOneEnd()
{
    CSeqVector upd_seq = m_UpdBsh.GetSeqVector(CBioseq_Handle::eCoding_Iupac, eNa_strand_plus);
    if (m_Params.m_IgnoreAlignment) {
        string seq;
        upd_seq.GetSeqData(0, m_UpdBsh.GetBioseqLength(), seq);
        return s_ExtendOneEndOfSequence(m_OldBsh, seq, m_Params.m_SeqUpdateOption);
    }
    else {
        _ASSERT(m_Align);
        CSeq_align::TDim oldseqRow = FindRowInAlignment(m_OldBsh, *m_Align);
        CSeq_align::TDim updseqRow = FindRowInAlignment(m_UpdBsh, *m_Align);
        _ASSERT(oldseqRow != updseqRow);
        _ASSERT(oldseqRow > -1 && updseqRow > -1);

        CSeqVector old_seq = m_OldBsh.GetSeqVector(CBioseq_Handle::eCoding_Iupac, eNa_strand_plus);
        string new_seq;
        if (m_Params.m_SeqUpdateOption == SUpdateSeqParams::eSeqUpdateExtend5) {
            // the end of aligned region in the old sequence coordinates
            const TSeqPos aln_stop = m_Align->GetSeqStop(oldseqRow);
            // the end of aligned region in the update sequence coordinates
            const TSeqPos aln_stop_upd = m_Align->GetSeqStop(updseqRow);

            // take new 5' and aligned middle //
            string new_5prime;
            upd_seq.GetSeqData(0, aln_stop_upd + 1, new_5prime);
            new_seq.append(new_5prime);

            // take old 3' end //
            string old_3prime;
            old_seq.GetSeqData(aln_stop + 1, m_OldBsh.GetBioseqLength(), old_3prime);
            new_seq.append(old_3prime);
        }
        else if (m_Params.m_SeqUpdateOption == SUpdateSeqParams::eSeqUpdateExtend3) {
            // the start of aligned region in the old sequence coordinates
            const TSeqPos aln_start = m_Align->GetSeqStart(oldseqRow);
            // the start of aligned region in the update sequence coordinates
            const TSeqPos aln_start_upd = m_Align->GetSeqStart(updseqRow);

            // take old 5' //
            string old_5prime;
            old_seq.GetSeqData(0, aln_start, old_5prime);
            new_seq.append(old_5prime);

            // take aligned middle and new 3' end //
            string new_3prime;
            upd_seq.GetSeqData(aln_start_upd, m_UpdBsh.GetBioseqLength(), new_3prime);
            new_seq.append(new_3prime);
        }
        return x_UpdateSeqInst(new_seq);
    }

    return CRef<CSeq_inst>();
}

string CSequenceUpdater::s_GetValidExtension(const string& extension)
{
    if (extension.empty())  {
        return kEmptyStr;
    }

    const string allowed("ATUCGRYSWKMBDHVN"); // for nucleotides
    string small_allowed(allowed);
    NStr::ToLower(small_allowed);

    string valid_extension(kEmptyStr);
    valid_extension.reserve(extension.size());
    copy_if(extension.begin(), extension.end(),
        back_inserter(valid_extension),
        [&allowed, &small_allowed](const char& c) { return (allowed.find(c) != NPOS) || (small_allowed.find(c) != NPOS); });

    return valid_extension;
}

CRef<CSeq_inst> CSequenceUpdater::s_ExtendOneEndOfSequence(const CBioseq_Handle& bsh,
    const string& extension, SUpdateSeqParams::ESequenceUpdateType update_type)
{
    if (extension.empty() 
        || bsh.IsAa()
        || (update_type != SUpdateSeqParams::eSeqUpdateExtend5 && update_type != SUpdateSeqParams::eSeqUpdateExtend3)) {
        return CRef<CSeq_inst>();
    }

    const string valid_extension = s_GetValidExtension(extension);
    if (valid_extension.empty()) {
        return CRef<CSeq_inst>();
    }

    string orig_seq;
    CSeqVector seqvec = bsh.GetSeqVector(CBioseq_Handle::eCoding_Iupac, eNa_strand_plus);
    seqvec.GetSeqData(0, bsh.GetBioseqLength(), orig_seq);
    
    string new_seq;
    if (update_type == SUpdateSeqParams::eSeqUpdateExtend5) {
        new_seq.assign(valid_extension);
        new_seq.append(orig_seq);
    } else if (update_type == SUpdateSeqParams::eSeqUpdateExtend3) {
        new_seq.assign(orig_seq);
        new_seq.append(valid_extension);
    }

    return s_UpdateSeqInst(bsh, new_seq);
}

CRef<CSeq_inst> CSequenceUpdater::x_UpdateSeqInst(const string& upd_str)
{
    return s_UpdateSeqInst(m_OldBsh, upd_str);
}

CRef<CSeq_inst> CSequenceUpdater::s_UpdateSeqInst(const CBioseq_Handle& bsh, const string& upd_str)
{
    CRef<CSeq_inst> new_inst(new CSeq_inst());
    new_inst->Assign(bsh.GetInst());
    if (bsh.GetInst_Repr() == CSeq_inst::eRepr_delta
        && CUpdateSeq_Input::s_IsDeltaWithNoGaps(bsh.GetCompleteBioseq().GetObject())) {
        // convert it into raw
        new_inst->SetRepr(CSeq_inst::eRepr_raw);
        new_inst->ResetExt();
    }
    bool updated(false);
    if (!NStr::IsBlank(upd_str)) {
        new_inst->SetLength(upd_str.size());
        if (bsh.IsNa()) {
            new_inst->SetSeq_data().SetIupacna(*new CIUPACna(upd_str));
            CSeqportUtil::Pack(&new_inst->SetSeq_data());
        } else {
            new_inst->SetSeq_data().SetIupacaa(*new CIUPACaa(upd_str));
        }
        updated = true;
    }

    return (updated) ? new_inst : CRef<CSeq_inst>();
}


CRef<CSeq_feat> CSequenceUpdater::s_OffsetFeature(const CSeq_feat& feat, const TSeqPos offset, const CSeq_id* newId)
{
    CRef<CSeq_loc> new_loc = s_OffsetLocation(feat.GetLocation(), offset, newId);
    CRef<CSeq_feat> shifted_feat(new CSeq_feat());
    shifted_feat->Assign(feat);
    shifted_feat->SetLocation(*new_loc);

    if (feat.GetData().IsCdregion()) {
        s_Shift_CDSCodeBreaks(shifted_feat, offset, newId);
    }
    if (feat.GetData().GetSubtype() == CSeqFeatData::eSubtype_tRNA) {
        s_Shift_tRNAAntiCodon(shifted_feat, offset, newId);
    }
    return shifted_feat;
}

void CSequenceUpdater::s_Shift_CDSCodeBreaks(CRef<CSeq_feat> feat, const TSeqPos& offset, const CSeq_id* targetId)
{
    CCdregion& cds = feat->SetData().SetCdregion();
    if (cds.IsSetCode_break()) {
        NON_CONST_ITERATE(CCdregion::TCode_break, it, cds.SetCode_break()) {
            if ((*it)->IsSetLoc()) {
                const CSeq_loc& codebreak = (*it)->GetLoc();
                CRef<CSeq_loc> new_codebreak = s_OffsetLocation(codebreak, offset, targetId);
                (*it)->SetLoc(*new_codebreak);
            }
        }
    }
}

void CSequenceUpdater::s_Shift_tRNAAntiCodon(CRef<CSeq_feat> feat, const TSeqPos& offset, const CSeq_id* targetId)
{
    if (!feat->GetData().GetRna().IsSetExt()) {
        return;
    }
    CRNA_ref::C_Ext& ext = feat->SetData().SetRna().SetExt();
    if (ext.IsTRNA() && ext.GetTRNA().IsSetAnticodon()) {
        const CSeq_loc& anticodon = ext.GetTRNA().GetAnticodon();
        CRef<CSeq_loc> new_anticodon = s_OffsetLocation(anticodon, offset, targetId);
        ext.SetTRNA().SetAnticodon(*new_anticodon);
    }
}

CRef<CSeq_loc> CSequenceUpdater::s_OffsetLocation(const CSeq_loc& sourceLoc, const TSeqPos& offset, const CSeq_id* targetId)
{
    CRef<CSeq_loc> shiftedLoc(new CSeq_loc);
    shiftedLoc->Assign(sourceLoc);

    CSeq_loc_I loc_it(*shiftedLoc);
    for (; loc_it; ++loc_it) {
        if (targetId) {
            loc_it.SetSeq_id(*targetId);
        }
        loc_it.SetFrom(loc_it.GetRange().GetFrom() + offset);
        loc_it.SetTo(loc_it.GetRange().GetTo() + offset);
    }

    shiftedLoc->Assign(*loc_it.MakeSeq_loc());
    return shiftedLoc;
}


void CSequenceUpdater::x_ReplaceIdInOldFeatures()
{
    SAnnotSelector sel(CSeqFeatData::eSubtype_any);
    sel.SetLimitTSE(m_OldBsh.GetTSE_Handle());
    for (CFeat_CI feat_it(m_OldBsh, sel); feat_it; ++feat_it) {
        if (x_ShouldRemoveFeature(*feat_it)) {
            continue;
        }

        CRef<CSeq_feat> mapped_feat = x_MappedFeature_ChangeId(*feat_it);
        // if features originally spanned the whole sequence, they should also span the whole sequence
        if (feat_it->GetLocation().IsInt()) {
            const TSeqPos start = feat_it->GetLocation().GetStart(eExtreme_Positional);
            const TSeqPos stop = feat_it->GetLocation().GetStop(eExtreme_Positional);
            if (start == 0 && stop == m_OldBsh.GetBioseqLength() - 1) {
                mapped_feat->SetLocation().SetInt().SetTo(x_GetNewSeqLength() - 1);
            }
        }
        if (mapped_feat) {
            m_MappedOldFeats.push_back(mapped_feat);
        }

    }
}

void CSequenceUpdater::x_MapOldAndImportedFeatsThroughNewAlign()
{
    auto align_OldNew = x_FormNewAlignment(m_OldBsh);
    CSeq_align::TDim oldseqRow = FindRowInAlignment(m_OldBsh, align_OldNew[0].GetObject());
    SAnnotSelector sel(CSeqFeatData::eSubtype_any);
    sel.SetLimitTSE(m_OldBsh.GetTSE_Handle());
    for (CFeat_CI feat_it(m_OldBsh, sel); feat_it; ++feat_it) {
        if (x_ShouldRemoveFeature(*feat_it)) {
            continue;
        }
        x_ChangeIDInFeature(*feat_it, align_OldNew[0].GetObject(), oldseqRow);
        CRef<CSeq_feat> mapped_feat = x_MappedFeature_ThroughAlign(*feat_it, align_OldNew[0].GetObject());
        if (mapped_feat) {
            m_MappedOldFeats.push_back(mapped_feat);
        }
    }

    // form alignment A(upd|new)
    if (m_Params.m_ImportFeatures) {
        auto align_UpdNew = x_FormNewAlignment(m_UpdBsh);
        SAnnotSelector sel(m_Params.m_FeatImportType);
        sel.SetLimitTSE(m_UpdBsh.GetTSE_Handle());
        for (CFeat_CI feat_it(m_UpdBsh, sel); feat_it; ++feat_it) {
            CRef<CSeq_feat> import_feat = x_MappedFeature_ThroughAlign(*feat_it, align_UpdNew[0].GetObject());
            if (import_feat) {
                m_ImportUpdFeats.push_back(import_feat);
            }
        }
        x_HandleDuplicateFeatures();
    }
}

void CSequenceUpdater::x_MakeNewEntry(const CSeq_inst& newInst)
{
    // make a new entry (seq or set) based on the old bioseq
    CSeq_entry_Handle oldSeh = x_GetOldBseq_EntryHandle();
    m_NewEntry.Reset(new CSeq_entry);
    m_NewEntry->Assign(*(oldSeh.GetCompleteSeq_entry()));

    x_PrepareNewEntry(m_NewEntry, newInst);
}

void CSequenceUpdater::x_PrepareNewEntry(CRef<CSeq_entry> entry, const CSeq_inst& newInst)
{
    // for nucleotide sequence, remove original seq-inst and all annotations
    if (entry->IsSeq()) {
        entry->SetSeq().ResetInst();
        entry->SetSeq().SetInst().Assign(newInst);
        entry->SetSeq().ResetAnnot();
        // remove all Ids
        // add a new, temporary ID
        entry->SetSeq().ResetId();
        CRef<CSeq_id> newid(new CSeq_id);
        newid->Assign(*m_NewId);
        entry->SetSeq().SetId().push_back(newid);
    }
    else if (m_NewEntry->IsSet()) {
        // remove annotations from the set level
        entry->SetSet().ResetAnnot();
        CBioseq_set::TSeq_set& seqset = entry->SetSet().SetSeq_set();
        CBioseq_set::TSeq_set::iterator it = seqset.begin();
        while (it != seqset.end()) {
            // remove proteins from the set
            if ((*it)->IsSeq() && (*it)->GetSeq().IsAa()) {
                it = seqset.erase(it);
            }
            else {
                x_PrepareNewEntry(*it, newInst);
                ++it;
            }
        }
    }
}

CSeq_entry_Handle CSequenceUpdater::x_GetOldBseq_EntryHandle(void)
{
    _ASSERT(m_OldBsh);
    CSeq_entry_Handle oldSeh;

    if (m_OldBsh.IsProtein())
        return m_OldBsh.GetSeq_entry_Handle();

    CBioseq_set_Handle oldSetSeh = m_OldBsh.GetParentBioseq_set();
    if (oldSetSeh &&
        oldSetSeh.IsSetClass() &&
        oldSetSeh.GetClass() == CBioseq_set::eClass_nuc_prot) {
        oldSeh = oldSetSeh.GetParentEntry();
    }
    else {
        oldSeh = m_OldBsh.GetSeq_entry_Handle();
    }

    return oldSeh;
}

bool CSequenceUpdater::x_ShouldImportFeature(const CSeq_feat_Handle& fh)
{
    if (!m_Align && m_Params.m_IgnoreAlignment) {
        return true;
    }

    CSeq_align::TDim updseqRow = FindRowInAlignment(m_UpdBsh, *m_Align);
    _ASSERT(updseqRow > -1);
    CRef<CSeq_loc> align_loc = m_Align->CreateRowSeq_loc(updseqRow);
    CConstRef<CSeq_feat> feat = fh.GetOriginalSeq_feat();
    
    sequence::ECompare comp = sequence::Compare(feat->GetLocation(), *align_loc, 
        &m_UpdBsh.GetScope(), sequence::fCompareOverlapping);

    if (comp == sequence::eNoOverlap || comp == sequence::eAbutting) {
        string subtype = CSeqFeatData::SubtypeValueToName(feat->GetData().GetSubtype());
        string label;
        feat->GetLocation().GetLabel(&label);
        subtype.append(" " + label);
        m_NotImportedFeats.push_back(subtype);
        return false;
    }
    return true;
}

bool CSequenceUpdater::x_ShouldRemoveFeature(const CSeq_feat_Handle& fh)
{
    if (m_Params.m_FeatRemoveOption == SUpdateSeqParams::eFeatRemoveAll) {
        return true;
    } else if (m_Params.m_FeatRemoveOption == SUpdateSeqParams::eFeatRemoveNone) {
        return false;
    }

    _ASSERT(m_Align);
    CSeq_align::TDim oldseqRow = FindRowInAlignment(m_OldBsh, *m_Align);
    _ASSERT(oldseqRow > -1);
    const TSeqPos aln_start = m_Align->GetSeqStart(oldseqRow);
    const TSeqPos aln_stop = m_Align->GetSeqStop(oldseqRow);
    
    const TSeqPos start = fh.GetLocation().GetStart(eExtreme_Positional);
    const TSeqPos stop = fh.GetLocation().GetStop(eExtreme_Positional);

    switch (m_Params.m_FeatRemoveOption) {
    case (SUpdateSeqParams::eFeatRemoveAligned): 
        // remove if feature is inside the alignment
        return (stop > aln_start && start <= aln_stop);
    case (SUpdateSeqParams::eFeatRemoveNotAligned):
        // remove if feature is outside the alignment
        return (stop <= aln_start || start > aln_stop);
    default:
        break;
    }
    return false;
}

CRef<CSeq_feat> CSequenceUpdater::x_MappedFeature_ChangeId(const CSeq_feat_Handle& orig_fh)
{
    CRef<CSeq_loc> mappedLoc(new CSeq_loc);
    mappedLoc->Assign(orig_fh.GetLocation());
    mappedLoc->SetId(*m_NewId);

    CRef<CSeq_feat> mappedFeature(new CSeq_feat);
    mappedFeature->Assign(*(orig_fh.GetSeq_feat()));
    mappedFeature->ResetLocation();
    mappedFeature->ResetPartial();

    mappedFeature->SetLocation().Assign(*mappedLoc);
    if (mappedLoc->IsPartialStart(eExtreme_Biological) || mappedLoc->IsPartialStop(eExtreme_Biological)) {
        mappedFeature->SetPartial(true);
    }

    if (mappedFeature->GetData().GetSubtype() == CSeqFeatData::eSubtype_tRNA) {
        CRNA_ref& rna = mappedFeature->SetData().SetRna();
        if (rna.IsSetExt() &&
            rna.GetExt().IsTRNA() &&
            rna.GetExt().GetTRNA().IsSetAnticodon()) {

            CRef<CSeq_loc> new_anticodon(new CSeq_loc);
            new_anticodon->Assign(rna.GetExt().GetTRNA().GetAnticodon());
            new_anticodon->SetId(*m_NewId);
            rna.SetExt().SetTRNA().SetAnticodon(*new_anticodon);
        }
    }
    else if (mappedFeature->GetData().IsCdregion()) {
        CCdregion& cds = mappedFeature->SetData().SetCdregion();
        if (cds.IsSetCode_break()) {
            NON_CONST_ITERATE(CCdregion::TCode_break, it, cds.SetCode_break()) {
                if ((*it)->IsSetLoc()) {
                    CRef<CSeq_loc> new_codebreak(new CSeq_loc);
                    new_codebreak->Assign((*it)->GetLoc());
                    new_codebreak->SetId(*m_NewId);
                    (*it)->SetLoc(*new_codebreak);
                }
            }
        }
    }

    return mappedFeature;
}

CRef<CSeq_feat> CSequenceUpdater::x_MappedFeature_ThroughAlign(const CSeq_feat_Handle& orig_fh, const CSeq_align& align)
{
    
    CScope& scope = orig_fh.GetScope();
    bool is_circular = m_OldBsh.IsSetInst_Topology() && m_OldBsh.GetInst_Topology() == CSeq_inst::eTopology_circular;
    CRef<CSeq_loc> mappedLoc = s_MapLocation(orig_fh.GetLocation(), *m_NewId, align, is_circular, scope, x_GetNewSeqLength());
    // NULL location are transformed into an [0, 0] interval (done, internally)

    if (!mappedLoc) {
        return CRef<CSeq_feat>();
    }

    CRef<CSeq_feat> mappedFeature(new CSeq_feat);
    mappedFeature->Assign(*(orig_fh.GetSeq_feat()));
    mappedFeature->ResetLocation();
    mappedFeature->ResetPartial();

    mappedFeature->SetLocation().Assign(*mappedLoc);
    if (mappedLoc->IsPartialStart(eExtreme_Biological) || mappedLoc->IsPartialStop(eExtreme_Biological)) {
        mappedFeature->SetPartial(true);
    }

    if (mappedFeature->GetData().GetSubtype() == CSeqFeatData::eSubtype_tRNA) {
        CRNA_ref& rna = mappedFeature->SetData().SetRna();
        if (rna.IsSetExt() &&
            rna.GetExt().IsTRNA() &&
            rna.GetExt().GetTRNA().IsSetAnticodon()) {

            const CSeq_loc& anticodon = rna.GetExt().GetTRNA().GetAnticodon();
            CRef<CSeq_loc> new_anticodon = s_MapLocation(anticodon, *m_NewId, align, is_circular, scope, x_GetNewSeqLength());
            rna.SetExt().SetTRNA().SetAnticodon(*new_anticodon);
        }
    }
    else if (mappedFeature->GetData().IsCdregion()) {
        CCdregion& cds = mappedFeature->SetData().SetCdregion();
        if (cds.IsSetCode_break()) {
            NON_CONST_ITERATE(CCdregion::TCode_break, it, cds.SetCode_break()) {
                if ((*it)->IsSetLoc()) {
                    const CSeq_loc& codebreak = (*it)->GetLoc();
                    CRef<CSeq_loc> new_codebreak = s_MapLocation(codebreak, *m_NewId, align, is_circular, scope, x_GetNewSeqLength());
                    (*it)->SetLoc(*new_codebreak);
                }
            }
        }
    }

    return mappedFeature;
}

void CSequenceUpdater::x_ChangeIDInFeature(CSeq_feat_Handle fh, const CSeq_align& align, CSeq_align::TDim row)
{
    if (!fh || row < 0) {
        return;
    }

    CSeq_id::E_Choice loc_id_type = fh.GetLocationId().Which();
    CSeq_id::E_Choice align_id_type = align.GetSeq_id(row).Which();

    if (loc_id_type != align_id_type) {
        // replace the location id in the feature, it should match the alignment Id
        CRef<CSeq_feat> copy(new CSeq_feat);
        copy->Assign(*fh.GetSeq_feat());

        const CSeq_id& alignId = align.GetSeq_id(row);
        copy->SetLocation().SetId(alignId);
        if (copy->GetData().GetSubtype() == CSeqFeatData::eSubtype_tRNA) {
            CRNA_ref& rna = copy->SetData().SetRna();
            if (rna.IsSetExt() &&
                rna.GetExt().IsTRNA() &&
                rna.GetExt().GetTRNA().IsSetAnticodon()) {
                rna.SetExt().SetTRNA().SetAnticodon().SetId(alignId);
            }
        }
        else if (copy->GetData().IsCdregion()) {
            CCdregion& cds = copy->SetData().SetCdregion();
            if (cds.IsSetCode_break()) {
                NON_CONST_ITERATE(CCdregion::TCode_break, it, cds.SetCode_break()) {
                    if ((*it)->IsSetLoc()) {
                        (*it)->SetLoc().SetId(alignId);
                    }
                }
            }
        }

        CSeq_feat_EditHandle(fh).Replace(*copy);
    }
}

vector<CConstRef<objects::CSeq_align> > CSequenceUpdater::x_FormNewAlignment(const CBioseq_Handle& subject)
{
    _ASSERT(m_NewEntry);

    // add New Sequence to the scope of m_OldBsh and m_UpdBsh
    CSeq_entry_Handle newSeh = m_OldBsh.GetScope().AddTopLevelSeqEntry(*m_NewEntry);
    CBioseq_Handle newBsh = newSeh.GetBioseqHandle(*m_NewId);

    try {
        auto align = sequpd::RunBlast2Seq(subject, newBsh, true);

        // remove new sequence from the original scope
        newSeh.GetTopLevelEntry().GetEditHandle().Remove();

        _ASSERT(align.size() == 1);
        return align;
    }
    catch (const CException& e) {
        ERR_POST(e.what());
        NCBI_THROW(CSeqUpdateException, eFeatAdjust,
            "Could not form alignment between the subject sequence (old or update) and newly formed sequence");
    }
}


CConstRef<CSeq_align> CSequenceUpdater::x_GetNewAlign_Replace(const CBioseq_Handle& bsh)
{
    CSeq_align::TDim row = FindRowInAlignment(bsh, *m_Align);
    _ASSERT(row > -1);

    // make a new seq-align, where the id corresponding to bsh is changed to m_NewId
    CRef<CSeq_align> align(new CSeq_align);
    align->Assign(*m_Align);

    CDense_seg& denseg = align->SetSegs().SetDenseg();
    vector<CRef<CSeq_id> >& ids = denseg.SetIds();

    ids[row].Reset(new CSeq_id);
    ids[row]->Assign(*m_NewId);

    return ConstRef(align.GetPointer());
}

void CSequenceUpdater::x_HandleDuplicateFeatures()
{
    if (m_MappedOldFeats.empty() || m_ImportUpdFeats.empty() ||
        m_Params.m_FeatImportOption == SUpdateSeqParams::eFeatUpdateAll) {
        return;
    }

    TFeatList::iterator old_it = m_MappedOldFeats.begin();

    while (old_it != m_MappedOldFeats.end()) {
        TFeatList::iterator imp_it = m_ImportUpdFeats.begin();
        bool deleteOld(false);
        while (imp_it != m_ImportUpdFeats.end() && !deleteOld) {
            if (AreFeaturesDuplicates(**old_it, **imp_it)) {
                if (m_Params.m_FeatImportOption == SUpdateSeqParams::eFeatUpdateAllExceptDups) { // keep old feature
                    imp_it = m_ImportUpdFeats.erase(imp_it);
                    continue;
                }
                else if (m_Params.m_FeatImportOption == SUpdateSeqParams::eFeatUpdateAllReplaceDups) { // keep the newly imported feature
                    deleteOld = true;
                }
                else if (m_Params.m_FeatImportOption == SUpdateSeqParams::eFeatUpdateAllMergeDups) { // merge the two features
                    CRef<CSeq_feat> fusedFeat = x_FuseFeatures(**old_it, **imp_it);
                    deleteOld = true;
                    (*imp_it).Swap(fusedFeat);
                }
            } else {
                ++imp_it;
            }
        }

        if (deleteOld) {
            old_it = m_MappedOldFeats.erase(old_it);
        }
        else {
            ++old_it;
        }
    }
}


CRef<CSeq_feat> CSequenceUpdater::x_FuseFeatures(const CSeq_feat& feat_old, const CSeq_feat& feat_upd)
{
    CRef<CSeq_feat> fusedFeat(new CSeq_feat);
    fusedFeat->Assign(feat_upd);

    // merge common fields
    FuseCommonFeatureFields(*fusedFeat, feat_old);

    //  type-specific data
    switch (fusedFeat->GetData().Which()) {
    case CSeqFeatData::e_Gene: {
        CGene_ref& fusedGene = fusedFeat->SetData().SetGene();
        const CGene_ref& gene = feat_old.GetData().GetGene();
        FuseGenes(fusedGene, gene);
        break;
    }
    case CSeqFeatData::e_Cdregion: {
        x_FuseProtFeatsForCDS(*fusedFeat, feat_old);
        break;
    }
    case CSeqFeatData::e_Prot: {
        FuseProteins(*fusedFeat, feat_old);
        break;
    }
    case CSeqFeatData::e_Rna: {
        CRNA_ref& fusedRna = fusedFeat->SetData().SetRna();
        const CRNA_ref& rna = feat_old.GetData().GetRna();
        if (fusedRna.IsSetExt() && fusedRna.GetExt().IsName() &&
            rna.IsSetExt() && rna.GetExt().IsName()) {
            string orig_value = fusedRna.GetExt().GetName();
            string value = rna.GetExt().GetName();
            if (!NStr::EqualNocase(orig_value, value) &&
                edit::AddValueToString(orig_value, value, edit::eExistingText_append_semi)) {
                fusedRna.SetExt().SetName(orig_value);
            }
        }
        break;
    }
    case CSeqFeatData::e_Region: {
        string origRegion = fusedFeat->GetData().GetRegion();
        string region = feat_old.GetData().GetRegion();
        if (!NStr::EqualNocase(origRegion, region) &&
            edit::AddValueToString(origRegion, region, edit::eExistingText_append_semi)) {
            fusedFeat->SetData().SetRegion(origRegion);
        }
        break;
    }
    default:
        break;
    }
    return fusedFeat;
}

void CSequenceUpdater::x_FuseProtFeatsForCDS(const CSeq_feat& fusedFeat, const CSeq_feat& feat_old)
{
    if (!fusedFeat.GetData().IsCdregion() || !feat_old.GetData().IsCdregion()) {
        return;
    }
    if (!fusedFeat.IsSetProduct() || !feat_old.IsSetProduct()) {
        return;
    }

    CScope& scope = m_OldBsh.GetScope();
    CBioseq_Handle fused_protsh = scope.GetBioseqHandle(fusedFeat.GetProduct());
    CBioseq_Handle old_protsh = scope.GetBioseqHandle(feat_old.GetProduct());

    TFeatList old_protfeats, upd_protfeats;
    for (CFeat_CI prot_it(old_protsh); prot_it; ++prot_it) {
        CRef<CSeq_feat> new_feat(new CSeq_feat);
        new_feat->Assign(*prot_it->GetSeq_feat());
        old_protfeats.push_back(new_feat);
    }

    for (CFeat_CI prot_it(fused_protsh); prot_it; ++prot_it) {
        CRef<CSeq_feat> new_feat(new CSeq_feat);
        new_feat->Assign(*prot_it->GetSeq_feat());
        upd_protfeats.push_back(new_feat);
    }

    TFeatList::iterator old_it = old_protfeats.begin();
    bool found_duplicate = false;
    while (old_it != old_protfeats.end()) {
        TFeatList::iterator imp_it = upd_protfeats.begin();
        bool deleteOld(false);
        while (imp_it != upd_protfeats.end() && !deleteOld) {
            if (AreProteinFeaturesDuplicates(**old_it, **imp_it)) {
                CRef<CSeq_feat> fusedFeat = x_FuseFeatures(**old_it, **imp_it);
                deleteOld = true;
                (*imp_it).Swap(fusedFeat);
                found_duplicate = true;
            } else {
                ++imp_it;
            }
        }

        if (deleteOld) {
            old_it = old_protfeats.erase(old_it);
        }
        else {
            ++old_it;
        }
    }

    CSeq_annot_Handle ah = DefaultGetAppropriateFeatureTable(fused_protsh);
    CSeq_annot_EditHandle aeh(ah);
    
    if (!found_duplicate) {
        // attach the old protein features to the fused protein sequence
        ITERATE(TFeatList, feat_it, old_protfeats) {
            aeh.AddFeat(**feat_it);
        }
    }
    else {
        // first remove all features
        aeh.Remove();

        // attach the new features
        CRef<CSeq_annot> newAnnot(new CSeq_annot);
        NON_CONST_ITERATE(TFeatList, feat_it, old_protfeats) {
            CRef<CSeq_feat> feat = *feat_it;
            // should have the same ID ?
            newAnnot->SetData().SetFtable().push_back(feat);
        }

        NON_CONST_ITERATE(TFeatList, feat_it, upd_protfeats) {
            CRef<CSeq_feat> feat = *feat_it;
            newAnnot->SetData().SetFtable().push_back(feat);
        }

        CBioseq_EditHandle esh(fused_protsh);
        esh.AttachAnnot(*newAnnot);
    }
}

CRef<CSeq_annot> CSequenceUpdater::x_GetSeqAnnot_WithoutCDS(bool& has_cds)
 {
     CRef<CSeq_annot> seqAnnot(new CSeq_annot);
     bool has_non_cds = false;
     NON_CONST_ITERATE(list<CRef<CSeq_feat> >, feat_it, m_MappedOldFeats) {
         CRef<CSeq_feat> feat = *feat_it;
         if (feat->IsSetData() && feat->GetData().IsCdregion()) {
             has_cds = true;
         }
         else {
             seqAnnot->SetData().SetFtable().push_back(feat);
             has_non_cds = true;
         }
     }

     NON_CONST_ITERATE(list<CRef<CSeq_feat> >, feat_it, m_ImportUpdFeats) {
         // feature has NewId that needs to be changed
         CRef<CSeq_feat> feat = *feat_it;
         if (feat->IsSetData() && feat->GetData().IsCdregion()) {
             has_cds = true;
         }
         else {
             seqAnnot->SetData().SetFtable().push_back(feat);
             has_non_cds = true;
         }
     }

     if (has_non_cds) {
         return seqAnnot;
     }

     return CRef<CSeq_annot>();
 }


void CSequenceUpdater::x_SetOldIDInFeatures()
{
    NON_CONST_ITERATE(list<CRef<CSeq_feat> >, feat_it, m_MappedOldFeats) {
        x_RemoveNewIDInFeature(*feat_it);
    }
    NON_CONST_ITERATE(list<CRef<CSeq_feat> >, feat_it, m_ImportUpdFeats) {
        x_RemoveNewIDInFeature(*feat_it);
    }

}
void CSequenceUpdater::x_AttachFeaturesToNewSeq(bool create_general_only)
{
    _ASSERT(!m_NewEntry.IsNull());
    if (m_MappedOldFeats.empty() && m_ImportUpdFeats.empty())
        return;
    
    CScope newScope(*CObjectManager::GetInstance());
    newScope.AddDefaults();
    CSeq_entry_Handle newSeh = newScope.AddTopLevelSeqEntry(*m_NewEntry);

    CBioseq_Handle newBsh;
    CBioseq_CI b_iter(newSeh, m_OldBsh.GetBioseqMolType());
    if (b_iter) {
        newBsh = *b_iter;
    }

    // features has NewId that needs to be changed
    x_SetOldIDInFeatures();
    
    bool has_cds = false;
    CRef<CSeq_annot> seqAnnot = x_GetSeqAnnot_WithoutCDS(has_cds);
    CBioseq_EditHandle ebsh = newBsh.GetEditHandle();
    if (seqAnnot) {
        ebsh.AttachAnnot(*seqAnnot);
    }

    if (!has_cds) {
        return;
    }

    // attach coding regions to the set
    CSeq_entry_Handle parentSeh = newBsh.GetSeq_entry_Handle();
    CSeq_entry_EditHandle eh = parentSeh.GetEditHandle();
    if (!eh.IsSet()) {
        CBioseq_set_Handle nucParent = eh.GetParentBioseq_set();
        if (nucParent  &&  nucParent.IsSetClass() &&
            nucParent.GetClass() == CBioseq_set::eClass_nuc_prot) {
            eh = nucParent.GetParentEntry().GetEditHandle();
        }
    }

    if (!eh.IsSet()) {
        eh.ConvertSeqToSet();
        eh.SetSet().SetClass(CBioseq_set::eClass_nuc_prot);

        // move all source and pub descriptors from Bioseq to Nuc-prot set
        CBioseq_CI bi(eh, m_OldBsh.GetBioseqMolType());
        CBioseq_EditHandle nuc_h = bi->GetEditHandle();
        CSeq_descr::Tdata& seq_descr = nuc_h.SetDescr();
        CSeq_descr::Tdata::iterator desc_it = seq_descr.begin();
        while (desc_it != seq_descr.end()) {
            if ((*desc_it)->IsSource() || (*desc_it)->IsPub()) {
                CRef<objects::CSeqdesc> cpy(new objects::CSeqdesc());
                cpy->Assign(**desc_it);
                eh.AddSeqdesc(*cpy);
                desc_it = seq_descr.erase(desc_it);
            }
            else {
                ++desc_it;
            }
        }
    }

    CRef<CSeq_annot> setAnnot(new CSeq_annot);
    if (!m_Params.m_UpdateProteins) {
        // proteins do not need to be retranslated
        NON_CONST_ITERATE(list<CRef<CSeq_feat> >, feat_it, m_MappedOldFeats) {
            CRef<CSeq_feat> feat = *feat_it;
            if (feat->IsSetData() && feat->GetData().IsCdregion()) {
                x_AdjustOldCDSProduct(feat, eh);
                setAnnot->SetData().SetFtable().push_back(feat);
            }
        }

        NON_CONST_ITERATE(list<CRef<CSeq_feat> >, feat_it, m_ImportUpdFeats) {
            CRef<CSeq_feat> feat = *feat_it;
            if (feat->IsSetData() && feat->GetData().IsCdregion()) {
                x_ImportCDSProduct(feat, eh, newBsh, create_general_only);
                setAnnot->SetData().SetFtable().push_back(feat);
            }
        }
    }
    else {
        // retranslate coding regions and update protein sequences
        NON_CONST_ITERATE(list<CRef<CSeq_feat> >, feat_it, m_MappedOldFeats) {
            CRef<CSeq_feat> feat = *feat_it;
            if (feat->IsSetData() && feat->GetData().IsCdregion()) {
                x_RetranslateOldCDSProduct(feat, eh, newBsh, create_general_only);
                setAnnot->SetData().SetFtable().push_back(feat);
            }
        }

        NON_CONST_ITERATE(list<CRef<CSeq_feat> >, feat_it, m_ImportUpdFeats) {
            CRef<CSeq_feat> feat = *feat_it;
            if (feat->IsSetData() && feat->GetData().IsCdregion()) {
                x_RetranslateImportedCDSProduct(feat, eh, newBsh, create_general_only);
                setAnnot->SetData().SetFtable().push_back(feat);
            }
        }
    }

    eh.AttachAnnot(*setAnnot);


}

void CSequenceUpdater::x_RetranslateOldCDSProduct(CRef<CSeq_feat> cds, CSeq_entry_EditHandle& eh, CBioseq_Handle& newBsh, bool create_general_only)
{
    if (!cds->IsSetProduct()) {
        return;
    }

    if  (cds->IsSetExcept_text() && NStr::Find(cds->GetExcept_text(), "RNA editing") != string::npos) {
        return;
    }

    CScope& origScope = m_OldBsh.GetScope();
    CBioseq_Handle psh = origScope.GetBioseqHandle(cds->GetProduct());
    if (!psh) {
        return;
    }

    CSeq_entry_Handle prot_eh = psh.GetSeq_entry_Handle();
    CRef<CSeq_entry> new_protein(new CSeq_entry());
    new_protein->Assign(*(prot_eh.GetCompleteSeq_entry()));
    // copy of the original protein, keeping the original ids - always

    CRef<CSeq_entry> retranslated_protein = ncbi::CreateTranslatedProteinSequence(cds, newBsh, create_general_only);
    CSeq_inst& retranslated_inst = retranslated_protein->SetSeq().SetInst();
    CSeq_inst& new_inst = new_protein->SetSeq().SetInst();

    new_inst.SetLength(retranslated_inst.SetLength());
    new_inst.SetSeq_data(retranslated_inst.SetSeq_data());
    SetMolinfoForProtein(new_protein,
        cds->GetLocation().IsPartialStart(objects::eExtreme_Biological),
        cds->GetLocation().IsPartialStop(objects::eExtreme_Biological));

    x_AdjustProteinFeature(new_protein);
    eh.AttachEntry(*new_protein);
}

void CSequenceUpdater::x_AdjustOldCDSProduct(CRef<CSeq_feat> cds, CSeq_entry_EditHandle& eh)
{
    if (!cds->IsSetProduct()) {
        return;
    }

    // don't change anything on the existing protein
    CScope& origScope = m_OldBsh.GetScope();
    CBioseq_Handle protein = origScope.GetBioseqHandle(cds->GetProduct());
    if (!protein) {
        return;
    }

    // attach the protein to the sequence:
    CSeq_entry_Handle proth = protein.GetSeq_entry_Handle();
    if (proth) {
        CConstRef<CSeq_entry> prot_entry = proth.GetCompleteSeq_entry();
        if (prot_entry) {
            CRef<CSeq_entry> new_protein(new CSeq_entry());
            new_protein->Assign(*prot_entry);
            eh.AttachEntry(*new_protein);
        }
    }
}

void CSequenceUpdater::x_RetranslateImportedCDSProduct(CRef<CSeq_feat> cds, CSeq_entry_EditHandle& eh,
                                                       CBioseq_Handle& newBsh, bool create_general_only)
{
    if (!cds->IsSetProduct()) {
        return;
    }
    if  (cds->IsSetExcept_text() && NStr::Find(cds->GetExcept_text(), "RNA editing") != string::npos) {
        return;
    }
    CScope& origScope = m_OldBsh.GetScope();
    CBioseq_Handle psh = origScope.GetBioseqHandle(cds->GetProduct());
    if (!psh) {
        return;
    }

    CSeq_entry_Handle prot_eh = psh.GetSeq_entry_Handle();
    CRef<CSeq_entry> new_protein(new CSeq_entry());
    new_protein->Assign(*(prot_eh.GetCompleteSeq_entry()));
    // copy of the original protein, keeping the original ids

    CRef<CSeq_entry> retranslated_protein = ncbi::CreateTranslatedProteinSequence(cds, newBsh, create_general_only);
    CSeq_inst& retranslated_inst = retranslated_protein->SetSeq().SetInst();
    CSeq_inst& new_inst = new_protein->SetSeq().SetInst();

    new_inst.SetLength(retranslated_inst.SetLength());
    new_inst.SetSeq_data(retranslated_inst.SetSeq_data());
    SetMolinfoForProtein(new_protein,
        cds->GetLocation().IsPartialStart(objects::eExtreme_Biological),
        cds->GetLocation().IsPartialStop(objects::eExtreme_Biological));

    CSeq_entry_Handle parentSeh = newBsh.GetSeq_entry_Handle();
    if (!m_Params.m_KeepProteinId) {
        CSeq_entry_Handle seh = m_OldBsh.GetTopLevelEntry();
        int offset = 1;
        string id_label;
        CRef<CSeq_id> product_id = objects::edit::GetNewProtId(newBsh, offset, id_label, create_general_only);
        x_AdjustProteinFeature(new_protein, product_id);

        // change product id in CDS
        cds->SetProduct().SetWhole().Assign(*product_id);
    }
    else {
        x_AdjustProteinFeature(new_protein);
    }

    x_RemoveDescriptorsFromImportedProducts(new_protein);
    eh.AttachEntry(*new_protein);

}

void CSequenceUpdater::x_ImportCDSProduct(CRef<CSeq_feat> cds, CSeq_entry_EditHandle& eh,
                                          CBioseq_Handle& newBsh, bool create_general_only)
{
    if (!cds->IsSetProduct()) {
        return;
    }

    CScope& origScope = m_OldBsh.GetScope();
    CBioseq_Handle protein = origScope.GetBioseqHandle(cds->GetProduct());
    if (!protein) {
        return;
    }
    CSeq_entry_Handle parentSeh = newBsh.GetSeq_entry_Handle();
    // attach the protein to the sequence:
    CSeq_entry_Handle proth = protein.GetSeq_entry_Handle();
    if (proth) {
        CConstRef<CSeq_entry> prot_entry = proth.GetCompleteSeq_entry();
        if (prot_entry) {
            CRef<CSeq_entry> new_protein(new CSeq_entry());
            new_protein->Assign(*prot_entry);

            if (!m_Params.m_KeepProteinId) {
                CSeq_entry_Handle seh = m_OldBsh.GetTopLevelEntry();
                int offset = 1;
                string id_label;
                CRef<CSeq_id> product_id = objects::edit::GetNewProtId(newBsh, offset, id_label, create_general_only);
                x_UpdateProteinID(new_protein, product_id);

                // change product id in CDS
                cds->SetProduct().SetWhole().Assign(*product_id);
            }

            // remove RefGeneTracking user-objects and create-date descriptors from imported proteins
            x_RemoveDescriptorsFromImportedProducts(new_protein);
            eh.AttachEntry(*new_protein);
        }
    }
}


void CSequenceUpdater::x_RemoveNewIDInFeature(CRef<CSeq_feat> feat)
{
    CRef<CSeq_id> bestId = FindBestChoice(m_OldBsh.GetCompleteBioseq()->GetId(), CSeq_id::BestRank);
    // if there is a local ID, change it to that one
    const CSeq_id* lcl_id = m_OldBsh.GetCompleteBioseq()->GetLocalId();
    if (lcl_id) {
        bestId.Reset(new CSeq_id);
        bestId->Assign(*lcl_id);
    }

    _ASSERT(bestId);
    feat->SetLocation().SetId(*bestId);

    if (feat->GetData().GetSubtype() == CSeqFeatData::eSubtype_tRNA) {
        CRNA_ref& rna = feat->SetData().SetRna();
        if (rna.IsSetExt() &&
            rna.GetExt().IsTRNA() &&
            rna.GetExt().GetTRNA().IsSetAnticodon()) {
            rna.SetExt().SetTRNA().SetAnticodon().SetId(*bestId);
        }
    }
    else if (feat->GetData().IsCdregion()) {
        CCdregion& cds = feat->SetData().SetCdregion();
        if (cds.IsSetCode_break()) {
            NON_CONST_ITERATE(CCdregion::TCode_break, it, cds.SetCode_break()) {
                if ((*it)->IsSetLoc()) {
                    (*it)->SetLoc().SetId(*bestId);
                }
            }
        }
    }
}

void CSequenceUpdater::x_RemoveDescriptorsFromImportedProducts(CRef<CSeq_entry> protein)
{
    EDIT_EACH_SEQDESC_ON_SEQDESCR(desc_it, protein->SetDescr()) {
        switch ((*desc_it)->Which()) {
        case CSeqdesc::e_Create_date:
            ERASE_SEQDESC_ON_SEQDESCR(desc_it, protein->SetDescr());
            break;
        case CSeqdesc::e_User:
        {
            CUser_object& user_obj = (*desc_it)->SetUser();
            if (!user_obj.IsSetClass()
                && user_obj.IsSetType() && user_obj.GetType().IsStr()
                && NStr::EqualCase(user_obj.GetType().GetStr(), "RefGeneTracking")) {
                ERASE_SEQDESC_ON_SEQDESCR(desc_it, protein->SetDescr());
            }
            break;
        }
        default:
            break;
        }
    }
}


void CSequenceUpdater::x_FixIDInNewEntry(CRef<CSeq_entry> entry)
{
    
    if (entry->IsSeq()) {
        // remove original local ID
        entry->SetSeq().ResetId();
        // copy ids from Old Sequence
        ITERATE(CBioseq::TId, it, m_OldBsh.GetCompleteBioseq()->GetId()) {
            CRef<CSeq_id> new_id(new CSeq_id);
            new_id->Assign((**it));
            entry->SetSeq().SetId().push_back(new_id);
        }
    }
    else if (m_NewEntry->IsSet()) {
        CBioseq_set::TSeq_set& seqset = entry->SetSet().SetSeq_set();
        CBioseq_set::TSeq_set::iterator it = seqset.begin();
        while (it != seqset.end()) {
            x_FixIDInNewEntry(*it);
            ++it;
        }
    }
}

void CSequenceUpdater::x_AdjustProteinFeature(CRef<CSeq_entry> protein, CRef<CSeq_id> newId)
{
    // updates the ID but also updates the location of protein features
    if (!protein || protein->IsSet() || protein->GetSeq().IsNa()) {
        return;
    }

    if (newId) {
        protein->SetSeq().ResetId();
        protein->SetSeq().SetId().push_back(newId);
    }

    const TSeqPos length = protein->GetSeq().GetLength();
    EDIT_EACH_SEQANNOT_ON_BIOSEQ(annot_it, protein->SetSeq()) {
        CSeq_annot& annot = **annot_it;
        if (!annot.IsFtable()) continue;
        EDIT_EACH_FEATURE_ON_ANNOT(feat_it, annot) {
            CSeq_feat& feat = **feat_it;
            if (newId) {
                feat.SetLocation().SetId(*newId);
            }
            if (feat.IsSetData() && feat.GetData().IsProt() && !feat.GetData().GetProt().IsSetProcessed()) {
                feat.SetLocation().SetInt().SetTo(length - 1);
            }
        }
    }
}

void CSequenceUpdater::x_UpdateProteinID(CRef<CSeq_entry> protein, CRef<CSeq_id> newId)
{
    if (!protein || protein->IsSet() || protein->GetSeq().IsNa() || !newId) {
        return;
    }

    protein->SetSeq().ResetId();
    protein->SetSeq().SetId().push_back(newId);

    EDIT_EACH_SEQANNOT_ON_BIOSEQ(annot_it, protein->SetSeq()) {
        CSeq_annot& annot = **annot_it;
        if (!annot.IsFtable()) continue;
        EDIT_EACH_FEATURE_ON_ANNOT(prot_feat, annot) {
            (*prot_feat)->SetLocation().SetId(*newId);
        }
    }
}

void CSequenceUpdater::x_AddCitSubToUpdatedSequence()
{
    CScope newScope(*CObjectManager::GetInstance());
    newScope.AddDefaults();
    CSeq_entry_Handle newSeh = newScope.AddTopLevelSeqEntry(*m_NewEntry);

    CBioseq_Handle newBsh;
    CBioseq_CI b_iter(newSeh, m_OldBsh.GetBioseqMolType());
    if (b_iter) {
        newBsh = *b_iter;
    }

    CSeq_entry_Handle seh = newBsh.GetSeq_entry_Handle();
    CSeq_entry_EditHandle eh = seh.GetEditHandle();
    if (!eh.IsSet()) {
        CBioseq_set_Handle nucParent = eh.GetParentBioseq_set();
        if (nucParent  &&  nucParent.IsSetClass() &&
            nucParent.GetClass() == CBioseq_set::eClass_nuc_prot) {
            eh = nucParent.GetParentEntry().GetEditHandle();
        }
    }

    CConstRef<CSeqdesc> changedSeqdesc;
    CSeq_entry_Handle seh_for_desc;
    CRef<CSeqdesc> changedORadded_citsub = CCitSubUpdater::s_GetCitSubForUpdatedSequence(newBsh, m_CitSubMessage, changedSeqdesc, seh_for_desc);
    if (changedORadded_citsub &&
        changedSeqdesc && 
        NStr::EqualNocase(m_CitSubMessage, CCitSubUpdater::sm_ChangeExistingCitSub)) {
        
        eh.RemoveSeqdesc(*changedSeqdesc);
        eh.AddSeqdesc(*changedORadded_citsub);
    }
    else {
        changedORadded_citsub.Reset(CCitSubUpdater::s_GetCitSubForUpdatedSequence(m_OldBsh, m_CitSubMessage, changedSeqdesc, seh_for_desc).GetPointer());
        if (changedORadded_citsub) {
            eh.AddSeqdesc(*changedORadded_citsub);
        }
    }
}

CIRef<IEditCommand> CSequenceUpdater::x_SwapOldWithNewSeq()
{
    CRef<CCmdComposite> cmd(new CCmdComposite("Swap old sequence with new sequence"));
    CSeq_entry_Handle oldSeh = x_GetOldBseq_EntryHandle();
    CIRef<IEditCommand> exch_cmd(new CCmdChangeSeqEntry(oldSeh, m_NewEntry));
    return exch_cmd;
}

void CSequenceUpdater::x_PrintNewEntryIds()
{
    _ASSERT(m_NewEntry);
    
    if (m_NewEntry->IsSet()) {
        FOR_EACH_SEQENTRY_ON_SEQSET(e_it, m_NewEntry->GetSet()) {
            if ((*e_it)->IsSeq()) {
                for (auto&& it : (*e_it)->GetSeq().GetId()) {
                    LOG_POST(Info << MSerial_AsnText << *it);
                }
            }
        }
    }
    else if (m_NewEntry->IsSeq()) {
        for (auto&& it : m_NewEntry->GetSeq().GetId()) {
            LOG_POST(Info << MSerial_AsnText << *it);
        }
    }
    
}

void CSequenceUpdater::ShowCitSubMessage() const
{
    if (!m_CitSubMessage.empty()) {
        NcbiInfoBox(m_CitSubMessage);
    }
}

string CSequenceUpdater::GetRevCompReport() const
{
    if (!m_Reversed) {
        return kEmptyStr;
    }

    CRef<CSeq_id> bestId = FindBestChoice(m_OldBsh.GetCompleteBioseq()->GetId(), CSeq_id::BestRank);
    if (bestId) {
        string best_seqid;
        bestId->GetLabel(&best_seqid);
        return "Reverse complemented " + best_seqid;
    }
    return kEmptyStr;
}

void CSequenceUpdater::GetNotImportedFeatsReport(CNcbiOstream& out) const
{
    if (m_NotImportedFeats.empty()) {
        return;
    }

    CSeq_id_Handle best_idh = sequence::GetId(m_UpdBsh, sequence::eGetId_Best);
    string updseq_name;
    best_idh.GetSeqId()->GetLabel(&updseq_name, CSeq_id::eContent);

    CSeq_align::TDim updseqRow = FindRowInAlignment(m_UpdBsh, *m_Align);
    _ASSERT(updseqRow > -1);
    CRef<CSeq_loc> align_loc = m_Align->CreateRowSeq_loc(updseqRow);
    string align_label;
    align_loc->GetLabel(&align_label);
    
    out << "Features from " << updseq_name;
    out << " were not imported as they were outside the " << align_label << " alignment range:\n";
    for (auto&& it : m_NotImportedFeats) {
        out << it;
        out << "\n";
    }
}


END_NCBI_SCOPE
