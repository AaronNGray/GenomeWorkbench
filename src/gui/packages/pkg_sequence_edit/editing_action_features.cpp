/*  $Id: editing_action_features.cpp 45101 2020-05-29 20:53:24Z asztalos $
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
 * Authors:  Igor Filippov
 */

#include <ncbi_pch.hpp>
#include <objmgr/util/seq_loc_util.hpp>
#include <util/sequtil/sequtil_convert.hpp>
#include <objmgr/util/feature.hpp>
#include <objmgr/util/sequence.hpp>
#include <objmgr/seq_annot_ci.hpp>
#include <objmgr/bioseq_ci.hpp>
#include <objmgr/object_manager.hpp>
#include <gui/widgets/edit/bioseq_editor.hpp>
#include <objtools/edit/string_constraint.hpp>
#include <gui/objutils/cmd_create_feat.hpp>
#include <gui/objutils/cmd_change_seq_feat.hpp>
#include <objects/seqfeat/seqfeat_macros.hpp>
#include <objects/seqfeat/Gb_qual.hpp>
#include <objects/general/Dbtag.hpp>
#include <objects/general/Object_id.hpp>
#include <objects/seqfeat/Genetic_code.hpp>
#include <objtools/edit/cds_fix.hpp>
#include <objects/seq/Seq_data.hpp>
#include <objmgr/seq_vector.hpp>
#include <objmgr/impl/synonyms.hpp>
#include <gui/packages/pkg_sequence_edit/editing_actions.hpp>
#include <gui/packages/pkg_sequence_edit/editing_action_features.hpp>
#include <gui/packages/pkg_sequence_edit/editing_action_constraint.hpp>


BEGIN_NCBI_SCOPE
USING_SCOPE(objects);


IEditingActionFeat::IEditingActionFeat(CSeq_entry_Handle seh, const CSeqFeatData::ESubtype subtype, const CSeqFeatData::E_Choice feat_type, const string &name)
    : IEditingAction(seh, name)
{
    if (subtype != CSeqFeatData::eSubtype_any)
        m_selector = SAnnotSelector(subtype);
    else 
        m_selector = SAnnotSelector(feat_type);
}

CSeq_entry_Handle IEditingActionFeat::GetCurrentSeqEntryHandleFromFeatHandle(CSeq_feat_Handle fh)
{
    CSeq_entry_Handle seh;
    const CSeq_annot_Handle& ah = fh.GetAnnot();
    if (ah)
    {
        seh = ah.GetParentEntry();
        if (seh.IsSet())
        {
            CBioseq_CI b_iter(seh, CSeq_inst::eMol_na);
            if (b_iter)
                seh = b_iter->GetSeq_entry_Handle();
        }
    }
    return seh;
}
 
bool IEditingActionFeat::MatchBioseqConstraintForFeatHandle(CSeq_feat_Handle fh)
{
    bool match = false;
    CBioseq_Handle bsh;
    CScope &scope = fh.GetScope();
    for (CSeq_loc_CI subloc(fh.GetLocation(), objects::CSeq_loc_CI::eEmpty_Skip); subloc; ++subloc)
    {
            bsh = scope.GetBioseqHandle(subloc.GetSeq_id());
            if (bsh)
                match |= m_constraint->Match(bsh);
    }
    return match;
}

void IEditingActionFeat::Find(EActionType action)
{
    if (!m_TopSeqEntry)
        return;
    size_t count = 0;
    for (CFeat_CI feat_ci(m_TopSeqEntry, m_selector); feat_ci; ++feat_ci)
    {
        m_EditedFeat.Reset();
        CSeq_feat_Handle fh = feat_ci->GetSeq_feat_Handle();        
        m_CurrentSeqEntry = GetCurrentSeqEntryHandleFromFeatHandle(fh);

        if (MatchBioseqConstraintForFeatHandle(fh) && m_constraint->Match(fh))
        {
            m_Feat = fh;
            if (m_ChangedFeatures.find(m_Feat) != m_ChangedFeatures.end())
                m_EditedFeat = m_ChangedFeatures[m_Feat];
            else
            {
                m_EditedFeat.Reset(new CSeq_feat);
                m_EditedFeat->Assign(*m_Feat.GetOriginalSeq_feat());
            }
            Modify(action);
        }
        ++count;
        if (count >= m_max_records)
            break;       
    }   
}

void IEditingActionFeat::FindRelated(EActionType action)
{
    if (!m_TopSeqEntry)
        return;
    IEditingActionFeat *feat_other = dynamic_cast<IEditingActionFeat*>(m_Other);
    if (!feat_other)
    {
        Find(action); 
        return;
    }
    CSeq_feat_Handle other_fh = feat_other->GetFeatHandle();
    CScope &scope = m_TopSeqEntry.GetScope();
    m_EditedFeat.Reset();
    m_Feat.Reset();
    if ( other_fh.GetFeatType() != CSeqFeatData::e_Prot && m_selector.GetFeatType() != CSeqFeatData::e_Prot)
    {
        CSeq_feat_Handle feat_fh = x_FindGeneForFeature(other_fh.GetLocation(), scope, m_selector.GetFeatSubtype());
        if( feat_fh )
        {
            m_Feat = feat_fh;
        }
    }
    else if ( other_fh.GetFeatType() == CSeqFeatData::e_Prot && m_selector.GetFeatType() != CSeqFeatData::e_Prot)
    {
        const CSeq_loc& prot_loc = other_fh.GetLocation();
        CBioseq_Handle prot_bsh = scope.GetBioseqHandle(prot_loc);
        if (prot_bsh) 
        {
            const CSeq_feat* cds = sequence::GetCDSForProduct(prot_bsh);
            if (cds)
            {
                if (m_selector.GetFeatSubtype() == CSeqFeatData::eSubtype_cdregion)
                {
                    m_Feat = scope.GetSeq_featHandle(*cds, CScope::eMissing_Null);
                }
                else
                {
                    CSeq_feat_Handle feat_fh = x_FindGeneForFeature(cds->GetLocation(), scope, m_selector.GetFeatSubtype());                  
                    if( feat_fh )
                    {
                        m_Feat = feat_fh;
                    }                    
                }
            }
        }
    }
    else if ( other_fh.GetFeatType() != CSeqFeatData::e_Prot && m_selector.GetFeatType() == CSeqFeatData::e_Prot)
    {
        CSeq_feat_Handle cds_fh = other_fh;
        if (other_fh.GetFeatSubtype() != CSeqFeatData::eSubtype_cdregion)
            cds_fh = x_FindGeneForFeature(other_fh.GetLocation(), scope, CSeqFeatData::eSubtype_cdregion);      
        if(cds_fh && cds_fh.IsSetProduct())
        {
            const CSeq_loc& prot_loc = cds_fh.GetProduct();
            CBioseq_Handle prot_bsh = scope.GetBioseqHandle(prot_loc);
            if (prot_bsh) 
            {
                CFeat_CI prot_feat_ci(prot_bsh, SAnnotSelector(CSeqFeatData::eSubtype_prot));
                if ( prot_feat_ci )
                {
                    CSeq_feat_Handle prot_fh = prot_feat_ci->GetSeq_feat_Handle();
                    m_Feat = prot_fh;
                }
            }
        }
    }
    else
    {
        Find(action);
        return;
    }

    if (m_Feat)
    {
        if (m_ChangedFeatures.find(m_Feat) != m_ChangedFeatures.end())
            m_EditedFeat = m_ChangedFeatures[m_Feat];
        else
        {
            m_EditedFeat.Reset(new CSeq_feat);
            m_EditedFeat->Assign(*m_Feat.GetOriginalSeq_feat());
        }
        Modify(action);
    }   
}



bool IEditingActionFeat::x_TestGeneForFeature(const CSeq_loc& gene_loc, const CSeq_loc& feat_loc, CScope& scope, bool& exact, TSeqPos& diff)
{
    exact = false;
    ENa_strand feat_strand = feat_loc.GetStrand();
    ENa_strand gene_strand = gene_loc.GetStrand();
    if (feat_strand == eNa_strand_minus && gene_strand != eNa_strand_minus) {
        return false;
    } else if (feat_strand != eNa_strand_minus && gene_strand == eNa_strand_minus) {
        return false;
    }
    if (gene_loc.GetStart(eExtreme_Positional) > feat_loc.GetStop(eExtreme_Positional) ||
        feat_loc.GetStart(eExtreme_Positional) > gene_loc.GetStop(eExtreme_Positional))
        return false;

    sequence::ECompare cmp = sequence::Compare(gene_loc, feat_loc, &scope);

    if (cmp == sequence::eSame) {
        exact = true;
        diff = 0;
        return true; 
    } else if (cmp == sequence::eContained || cmp == sequence::eContains || cmp == sequence::eOverlap) {

        CRef<CSeq_loc> loc1 = sequence::Seq_loc_Subtract(gene_loc, feat_loc, CSeq_loc::fMerge_All|CSeq_loc::fSort, &scope);
        CRef<CSeq_loc> loc2 = sequence::Seq_loc_Subtract(feat_loc, gene_loc, CSeq_loc::fMerge_All|CSeq_loc::fSort, &scope);
        CRef<CSeq_loc> loc3 = sequence::Seq_loc_Add(*loc1, *loc2, CSeq_loc::fMerge_All|CSeq_loc::fSort, &scope);
        diff = sequence::GetCoverage(*loc3, &scope);
        return true;
    } else {
        return false;
    }
}
 
CRef<CSeq_loc> IEditingActionFeat::x_GetSublocOnBioseq(CBioseq_Handle bsh,  const CSeq_loc& loc)
{
    CScope &scope = bsh.GetScope();
    CRef<CSeq_loc> new_loc(new CSeq_loc);
    new_loc->Assign(loc);
    CSeq_loc_I loc_it(*new_loc); // , CSeq_loc_CI::eEmpty_Skip, CSeq_loc_CI::eOrder_Positional
    while(loc_it)
    {
        if (loc_it.IsEmpty() || !sequence::IsSameBioseq(*bsh.GetSeqId(), loc_it.GetSeq_id(), &scope))
        {
            loc_it.Delete();
            continue;
        }
        ++loc_it;
    }
    CRef<CSeq_loc> changed_loc = loc_it.MakeSeq_loc()->Merge(objects::CSeq_loc::fSort, NULL);;
    if (changed_loc->IsNull() || loc_it.GetSize() == 0)
        changed_loc.Reset();
    return changed_loc;
}

CSeq_feat_Handle IEditingActionFeat::x_FindGeneForFeature(const CSeq_loc& loc, CScope& scope, CSeqFeatData::ESubtype subtype) 
{
    CSeq_feat_Handle empty;
    CSeq_feat_Handle exact_match;
       
    CSeq_feat_Handle best_non_exact;
    TSeqPos min_diff = INT_MAX;

    set<CBioseq_Handle> seen_bsh;
    set<CSeq_feat_Handle>   feature_handles;
    for (CSeq_loc_CI subloc(loc, objects::CSeq_loc_CI::eEmpty_Skip); subloc; ++subloc)
    {
        CBioseq_Handle bsh = scope.GetBioseqHandle(subloc.GetSeq_id());
        if (!bsh)
            continue;
        seen_bsh.insert(bsh);
        for (CFeat_CI gene(bsh, subtype); gene; ++gene)
        {
            feature_handles.insert(gene->GetSeq_feat_Handle());
        }
    }
   
    for (auto gene : feature_handles)
    {
        bool exact = true;
        TSeqPos diff = 0;
        const CSeq_loc& gene_loc = gene.GetLocation();       
        bool any_match = false;
        for (auto bsh : seen_bsh)
        {       
            auto subloc = x_GetSublocOnBioseq(bsh, loc);       
            auto gene_subloc = x_GetSublocOnBioseq(bsh, gene_loc);
            if (!subloc || !gene_subloc)
                continue;
            bool sub_exact = false;
            TSeqPos sub_diff = INT_MAX;
            if (x_TestGeneForFeature(*gene_subloc, *subloc, scope, sub_exact, sub_diff))
            {
                exact = exact && sub_exact;
                if (sub_diff < INT_MAX)
                    diff += sub_diff;
                else
                    diff = INT_MAX;
            }
            else
            {
                exact = false;
                diff = INT_MAX;
            }
            any_match = true;
        }
        if (!any_match)
            continue;
        if (exact)
        {
            exact_match = gene;
        }            
        if (diff < min_diff)
        {
            best_non_exact = gene;
            min_diff = diff;
        }
    }

    if (exact_match)
        return exact_match;
    if (best_non_exact)
        return best_non_exact;
    return empty;
}

namespace {
// Types used in operations with seq-locs
// Ugly copy-paste
class CRangeWithFuzz : public CSeq_loc::TRange
{
public:
    typedef CSeq_loc::TRange TParent;
    typedef CConstRef<CInt_fuzz>  TFuzz;

    CRangeWithFuzz(const TParent& rg)
        : TParent(rg), m_Strand(eNa_strand_unknown)
    {
    }
    CRangeWithFuzz(const CSeq_loc_CI& it)
        : TParent(it.GetRange()),
          m_Fuzz_from(it.GetFuzzFrom()),
          m_Fuzz_to(it.GetFuzzTo()),
          m_Strand(it.GetStrand())
    {
    }

    void ResetFuzzFrom(void) { m_Fuzz_from.Reset(); }
    void ResetFuzzTo(void) { m_Fuzz_to.Reset(); }
    bool IsSetFuzzFrom(void) const { return m_Fuzz_from; }
    bool IsSetFuzzTo(void) const { return m_Fuzz_to; }
    const CInt_fuzz& GetFuzzFrom(void) const { return *m_Fuzz_from; }
    const CInt_fuzz& GetFuzzTo(void) const { return *m_Fuzz_to; }

    // Add fuzzes assuming that both ranges had the same 'from'
    void AddFuzzFrom(const CRangeWithFuzz& rg)
    {
        x_AddFuzz(m_Fuzz_from, rg.m_Fuzz_from, rg.m_Strand);
    }

    // Add fuzzes assuming that both ranges had the same 'to'
    void AddFuzzTo(const CRangeWithFuzz& rg)
    {
        x_AddFuzz(m_Fuzz_to, rg.m_Fuzz_to, rg.m_Strand);
    }

    void AddFuzzFrom(const CInt_fuzz& fuzz, ENa_strand strand)
    {
        x_AddFuzz(m_Fuzz_from, ConstRef(&fuzz), strand);
    }
    
    void AddFuzzTo(const CInt_fuzz& fuzz, ENa_strand strand)
    {
        x_AddFuzz(m_Fuzz_to, ConstRef(&fuzz), strand);
    }

    void CopyFrom(const CRangeWithFuzz& rg)
    {
        SetFrom(rg.GetFrom());
        m_Fuzz_from = rg.m_Fuzz_from;
    }

    void CopyTo(const CRangeWithFuzz& rg)
    {
        SetTo(rg.GetTo());
        m_Fuzz_to = rg.m_Fuzz_to;
    }

    CRangeWithFuzz& operator +=(const CRangeWithFuzz& rg)
    {
        TParent::position_type old_from = GetFrom();
        TParent::position_type old_to = GetTo();
        TParent::operator+=(rg);
        if (old_from != GetFrom()) {
            m_Fuzz_from.Reset(rg.m_Fuzz_from);
        }
        else if (old_from == rg.GetFrom()) {
            // Reset fuzz if it's not the same for both ranges
            AddFuzzFrom(rg);
        }
        if (old_to != GetTo()) {
            m_Fuzz_to.Reset(rg.m_Fuzz_to);
        }
        else if (old_to == rg.GetTo()) {
            AddFuzzTo(rg);
        }
        return *this;
    }

    CRangeWithFuzz& operator +=(const TParent& rg)
    {
        TParent::position_type old_from = GetFrom();
        TParent::position_type old_to = GetTo();
        TParent::operator+=(rg);
        // Reset fuzz if the corresponding extreme changes
        if (old_from != GetFrom()) {
            ResetFuzzFrom();
        }
        if (old_to != GetTo()) {
            ResetFuzzTo();
        }
        return *this;
    }

private:
    CRef<CInt_fuzz> x_SetFuzz(TFuzz&           fuzz,
                              const CInt_fuzz* copy_from)
    {
        TFuzz copy_from_cref;
        if (copy_from == fuzz) copy_from_cref.Reset(copy_from);
        // Since TFuzz is a const-ref, setting fuzz requires creating
        // a new object
        CRef<CInt_fuzz> new_fuzz(new CInt_fuzz);
        // The new value is optional
        if ( copy_from ) {
            new_fuzz->Assign(*copy_from);
        }
        fuzz.Reset(new_fuzz);
        return new_fuzz;
    }

    void x_AddFuzz(TFuzz&       fuzz,
                   const TFuzz& other,
                   ENa_strand   other_strand)
    {
        if ( !fuzz ) {
            // Use fuzz from the other range if available
            if ( other ) {
                x_SetFuzz(fuzz, other.GetPointerOrNull());
            }
            return;
        }
        if ( !other ) {
            // The other range has no fuzz, keep the current one
            return;
        }
        if (fuzz->Which() != other->Which()) {
            // Fuzzes have different types, reset to lim-unk.
            CRef<CInt_fuzz> new_fuzz = x_SetFuzz(fuzz, NULL);
            new_fuzz->SetLim(CInt_fuzz::eLim_unk);
            return;
        }

        const CInt_fuzz& fz = *fuzz;
        const CInt_fuzz& ofz = *other;
        // Both fuzzes are set and have the same type, try to merge them
        switch ( fz.Which() ) {
        case CInt_fuzz::e_Lim:
            {
                CInt_fuzz::ELim this_lim = fz.GetLim();
                CInt_fuzz::ELim other_lim = ofz.GetLim();
                bool this_rev = IsReverse(m_Strand);
                bool other_rev = IsReverse(other_strand);
                bool other_lt = other_lim == CInt_fuzz::eLim_lt  ||
                    (!other_rev  &&  other_lim == CInt_fuzz::eLim_tl)  ||
                    (other_rev  &&  other_lim == CInt_fuzz::eLim_tr);
                bool other_gt = other_lim == CInt_fuzz::eLim_gt  ||
                    (!other_rev  &&  other_lim == CInt_fuzz::eLim_tr)  ||
                    (other_rev  &&  other_lim == CInt_fuzz::eLim_tl);
                switch ( fz.GetLim() ) {
                case CInt_fuzz::eLim_lt:
                    if ( other_lt ) {
                        return; // the same
                    }
                    break;
                case CInt_fuzz::eLim_gt:
                    if ( other_gt ) {
                        return; // the same
                    }
                    break;
                case CInt_fuzz::eLim_tl:
                    if ((!this_rev  &&  other_lt)  ||
                        (this_rev  &&  other_gt)) {
                        return; // the same
                    }
                    break;
                case CInt_fuzz::eLim_tr:
                    if ((!this_rev  &&  other_gt)  ||
                        (this_rev  &&  other_lt)) {
                        return; // the same
                    }
                    break;
                default:
                    if (other_lim == this_lim) {
                        return;
                    }
                    break;
                }
                // Different limits - reset to lim-unk.
                CRef<CInt_fuzz> new_fuzz = x_SetFuzz(fuzz, NULL);
                new_fuzz->SetLim(CInt_fuzz::eLim_unk);
                break;
            }
        case CInt_fuzz::e_Alt:
            {
                // Use union
                CRef<CInt_fuzz> new_fuzz = x_SetFuzz(fuzz, fuzz);
                new_fuzz->SetAlt().insert(
                    new_fuzz->SetAlt().end(),
                    ofz.GetAlt().begin(),
                    ofz.GetAlt().end());
                break;
            }
        case CInt_fuzz::e_Range:
            {
                // Use union
                CInt_fuzz::C_Range::TMin min1 = fz.GetRange().GetMin();
                CInt_fuzz::C_Range::TMin min2 = ofz.GetRange().GetMin();
                CInt_fuzz::C_Range::TMax max1 = fz.GetRange().GetMax();
                CInt_fuzz::C_Range::TMax max2 = ofz.GetRange().GetMax();
                if (min1 > min2  ||  max1 < max2) {
                    CRef<CInt_fuzz> new_fuzz = x_SetFuzz(fuzz, NULL);
                    new_fuzz->SetRange().SetMin(min1 < min2 ? min1 : min2);
                    new_fuzz->SetRange().SetMax(max1 > max2 ? max1 : max2);
                }
                break;
            }
        case CInt_fuzz::e_P_m:
            {
                // Use max value
                CInt_fuzz::TP_m pm = ofz.GetP_m();
                if (fz.GetP_m() < pm) {
                    CRef<CInt_fuzz> new_fuzz = x_SetFuzz(fuzz, NULL);
                    new_fuzz->SetP_m(pm);
                }
                break;
            }
        case CInt_fuzz::e_Pct:
            {
                // Use max value
                CInt_fuzz::TPct pct = ofz.GetPct();
                if (fz.GetPct() < pct) {
                    CRef<CInt_fuzz> new_fuzz = x_SetFuzz(fuzz, NULL);
                    new_fuzz->SetPct(pct);
                }
                break;
            }
        default:
            // Failed to merge fuzzes
            fuzz.Reset();
            break;
        }
    }

    TFuzz m_Fuzz_from;
    TFuzz m_Fuzz_to;
    ENa_strand m_Strand;
};


class CSeq_id_Handle_Wrapper
{
public:
    CSeq_id_Handle_Wrapper(void) {}

    CSeq_id_Handle_Wrapper(const CSeq_id_Handle& idh, const CSeq_id& id)
        : m_Handle(idh)
    {
        if (id.IsLocal() && id.GetLocal().IsStr()) {
            m_Id.Reset(&id);
        }
    }

    CConstRef<CSeq_id> GetSeqId(void) const { return m_Id ? m_Id : m_Handle.GetSeqId(); }

    const CSeq_id_Handle& GetHandle(void) const { return m_Handle; }

    bool operator== (const CSeq_id_Handle_Wrapper& handle) const
        {
            return m_Handle == handle.m_Handle;
        }

    bool operator!= (const CSeq_id_Handle_Wrapper& handle) const
        {
            return m_Handle != handle.m_Handle;
        }
    bool operator<  (const CSeq_id_Handle_Wrapper& handle) const
        {
            return m_Handle < handle.m_Handle;
        }

    DECLARE_OPERATOR_BOOL(m_Handle);

private:
    CSeq_id_Handle m_Handle;
    CConstRef<CSeq_id> m_Id;
};

class CDefaultSynonymMapper : public ISynonymMapper
{
public:
    CDefaultSynonymMapper(CScope* scope);
    virtual ~CDefaultSynonymMapper(void);

    virtual CSeq_id_Handle GetBestSynonym(const CSeq_id& id);

private:
    typedef map<CSeq_id_Handle, CSeq_id_Handle> TSynonymMap;

    CRef<CSeq_id_Mapper> m_IdMapper;
    TSynonymMap          m_SynMap;
    CScope*              m_Scope;
};


CDefaultSynonymMapper::CDefaultSynonymMapper(CScope* scope)
    : m_IdMapper(CSeq_id_Mapper::GetInstance()),
      m_Scope(scope)
{
    return;
}


CDefaultSynonymMapper::~CDefaultSynonymMapper(void)
{
    return;
}


CSeq_id_Handle CDefaultSynonymMapper::GetBestSynonym(const CSeq_id& id)
{
    CSeq_id_Handle idh = CSeq_id_Handle::GetHandle(id);
    if ( !m_Scope  ||  id.Which() == CSeq_id::e_not_set ) {
        return idh;
    }
    TSynonymMap::iterator id_syn = m_SynMap.find(idh);
    if (id_syn != m_SynMap.end()) {
        return id_syn->second;
    }
    CSeq_id_Handle best;
    int best_rank = CSeq_id::kMaxScore;
    CConstRef<CSynonymsSet> syn_set = m_Scope->GetSynonyms(idh);
#ifdef _DEBUG
    TGi gi = ZERO_GI;
#endif
    ITERATE(CSynonymsSet, syn_it, *syn_set) {
        CSeq_id_Handle synh = syn_set->GetSeq_id_Handle(syn_it);
#ifdef _DEBUG
        if (synh.IsGi()) {
            gi = synh.GetGi();
        }
#endif
        int rank = synh.GetSeqId()->BestRankScore();
        if (rank < best_rank) {
            best = synh;
            best_rank = rank;
        }
    }
    if ( !best ) {
        // Synonyms set was empty
        m_SynMap[idh] = idh;
        return idh;
    }
    ITERATE(CSynonymsSet, syn_it, *syn_set) {
        m_SynMap[syn_set->GetSeq_id_Handle(syn_it)] = best;
    }
#ifdef _DEBUG
    const CTextseq_id* txt_id = best.GetSeqId()->GetTextseq_Id();
    if (txt_id && !txt_id->IsSetVersion() && gi != ZERO_GI) {
        ERR_POST("Using version-less accession " << txt_id->GetAccession()
            << " instead of GI " << gi);
    }
#endif
    return best;
}

}
CRef<CSeq_loc> IEditingActionFeat::x_MergeFeatureLocation(const CSeq_loc &loc, CScope &scope)
{
    //CRef<CSeq_loc> gene_loc = sequence::Seq_loc_Merge(loc, CSeq_loc::fMerge_SingleRange, &scope);
    CRef<CSeq_loc> gene_loc(new CSeq_loc);
    CDefaultSynonymMapper syn_mapper(&scope);

    CRangeWithFuzz total_rg(CRangeWithFuzz::GetEmpty());
    CSeq_id_Handle_Wrapper prev_id;
    ENa_strand prev_strand = eNa_strand_unknown;
    bool first = true;
    for (CSeq_loc_CI it(loc, CSeq_loc_CI::eEmpty_Allow); it; ++it) 
    {
        CSeq_id_Handle_Wrapper next_id(syn_mapper.GetBestSynonym(it.GetSeq_id()), it.GetSeq_id());
        if ( !next_id ) 
        {
            // Ignore NULLs
            continue;
        }
        ENa_strand next_strand = it.GetStrand();
        if (first)
        {
            first = false;            
        }
        else if (prev_id != next_id || prev_strand != next_strand)
        {
            CRef<CSeq_id> id(new CSeq_id);
            id->Assign(*prev_id.GetSeqId());
            CRef<CSeq_interval> interval(new CSeq_interval(*id, total_rg.GetFrom(), total_rg.GetTo(), prev_strand));
            if ( total_rg.IsSetFuzzFrom() ) 
            {
                interval->SetFuzz_from().Assign(total_rg.GetFuzzFrom());
            }
            if ( total_rg.IsSetFuzzTo() ) 
            {
                interval->SetFuzz_to().Assign(total_rg.GetFuzzTo());
            }
            gene_loc->SetPacked_int().Set().push_back(interval);
            total_rg = move(CRangeWithFuzz(CRangeWithFuzz::GetEmpty()));
            first = true;
        }
        total_rg += CRangeWithFuzz(it);    
        prev_id = next_id;
        prev_strand = next_strand;
    }

    if (!total_rg.Empty())
    {
        CRef<CSeq_id> id(new CSeq_id);
        id->Assign(*prev_id.GetSeqId());
        CRef<CSeq_interval> interval(new CSeq_interval(*id, total_rg.GetFrom(), total_rg.GetTo(), prev_strand));
        if ( total_rg.IsSetFuzzFrom() ) 
        {
            interval->SetFuzz_from().Assign(total_rg.GetFuzzFrom());
        }
        if ( total_rg.IsSetFuzzTo() ) 
        {
            interval->SetFuzz_to().Assign(total_rg.GetFuzzTo());
        }
        if (gene_loc->IsPacked_int())
            gene_loc->SetPacked_int().Set().push_back(interval);
        else
            gene_loc->SetInt(*interval);
    }

    return gene_loc;
}

void IEditingActionFeat::FindFeatForAnotherFeat(EActionType action, SAnnotSelector other_selector)
{
    if (!m_TopSeqEntry)
        return;
    size_t count = 0;
    for (CFeat_CI feat_ci(m_TopSeqEntry, m_selector); feat_ci; ++feat_ci) 
    {
        m_EditedFeat.Reset();
        CSeq_feat_Handle fh = feat_ci->GetSeq_feat_Handle();
        m_CurrentSeqEntry = GetCurrentSeqEntryHandleFromFeatHandle(fh);
        CScope &scope = fh.GetScope();
        if (MatchBioseqConstraintForFeatHandle(fh) && m_constraint->Match(fh))
        {
            CSeq_feat_Handle gene_fh = x_FindGeneForFeature(fh.GetLocation(), scope, other_selector.GetFeatSubtype());
            if (gene_fh)
            {
                m_Feat = gene_fh;
                if (m_ChangedFeatures.find(m_Feat) != m_ChangedFeatures.end())
                {
                    m_EditedFeat = m_ChangedFeatures[m_Feat];
                }
                else
                {
                    m_EditedFeat.Reset(new CSeq_feat);
                    m_EditedFeat->Assign(*m_Feat.GetOriginalSeq_feat());
                }
                Modify(action);
            }
        }
        ++count;
        if (count >= m_max_records)
            break;        
    }
}


void IEditingActionFeat::FindRelatedFeatForAnotherFeat(EActionType action, SAnnotSelector other_selector) 
{
    IEditingActionFeat *feat_other = dynamic_cast<IEditingActionFeat*>(m_Other); 
    if (!feat_other)
    {
        Find(action); 
        return;
    }
    m_EditedFeat.Reset();
    CSeq_feat_Handle fh = feat_other->GetFeatHandle();
    CScope &scope = fh.GetScope();
    CSeq_feat_Handle gene_fh = x_FindGeneForFeature(fh.GetLocation(), scope, other_selector.GetFeatSubtype());
  
    if (gene_fh)
    {
        m_Feat = gene_fh;                         
        if (m_ChangedFeatures.find(m_Feat) != m_ChangedFeatures.end())
            m_EditedFeat = m_ChangedFeatures[m_Feat];
        else
        {
            m_EditedFeat.Reset(new CSeq_feat);
            m_EditedFeat->Assign(*m_Feat.GetOriginalSeq_feat());
        }
        Modify(action);
    }
}

void IEditingActionFeat::GoToProtFeature()
{
    m_saved_feat_enabled = false;
    if (m_Feat && m_Feat.IsSetProduct())
    {
        const CSeq_loc& prot_loc = m_Feat.GetProduct();
        CBioseq_Handle prot_bsh = m_Feat.GetScope().GetBioseqHandle(prot_loc);
        if (prot_bsh) 
        {
            CFeat_CI prot_feat_ci(prot_bsh, SAnnotSelector(CSeqFeatData::eSubtype_prot));
            if( prot_feat_ci)
            {
                m_saved_feat_enabled = true;
                swap(m_Feat, m_SavedFeat);
                swap(m_SavedEditedFeat, m_EditedFeat);
                CSeq_feat_Handle prot_fh = prot_feat_ci->GetSeq_feat_Handle();
                m_Feat = prot_fh;
                if (m_ChangedFeatures.find(m_Feat) != m_ChangedFeatures.end())
                    m_EditedFeat = m_ChangedFeatures[m_Feat];
                else
                {
                    m_EditedFeat.Reset(new CSeq_feat);
                    m_EditedFeat->Assign(*m_Feat.GetOriginalSeq_feat());
                }
            }
        }
    }
}

void IEditingActionFeat::GoToOrigFeature()
{
    if (m_saved_feat_enabled)
    {
        swap(m_Feat, m_SavedFeat);
        swap(m_SavedEditedFeat, m_EditedFeat);
        m_saved_feat_enabled = false;
    }
}

void IEditingActionFeat::FindOrCreateProtFeat(EActionType action)
{
    if (!m_TopSeqEntry)
        return;
    size_t count = 0;
    for (CFeat_CI feat_ci(m_TopSeqEntry, m_selector); feat_ci; ++feat_ci)
    {
        m_EditedFeat.Reset();
        CSeq_feat_Handle fh = feat_ci->GetSeq_feat_Handle();
        m_CurrentSeqEntry = GetCurrentSeqEntryHandleFromFeatHandle(fh);
        
        if (MatchBioseqConstraintForFeatHandle(fh) && m_constraint->Match(fh))
        {
            m_Feat = fh;
            if (fh.IsSetProduct())
            {
                const CSeq_loc& prot_loc = fh.GetProduct();
                CBioseq_Handle prot_bsh = m_TopSeqEntry.GetScope().GetBioseqHandle(prot_loc);
                if (prot_bsh) 
                {
                    CFeat_CI prot_feat_ci(prot_bsh, SAnnotSelector(CSeqFeatData::eSubtype_prot));
                    if( prot_feat_ci)
                    {
                        CSeq_feat_Handle prot_fh = prot_feat_ci->GetSeq_feat_Handle();
                        m_Feat = prot_fh;
                    }
                    else if (!IsNOOP(action) && m_ChangedFeatures.find(m_Feat) == m_ChangedFeatures.end())
                    {                           
                        m_EditedFeat.Reset(new CSeq_feat);
                        m_EditedFeat->SetData().SetProt();
                        CRef<CSeq_loc> new_prot_loc(new CSeq_loc);
                        new_prot_loc->Assign(prot_loc);
                        m_EditedFeat->SetLocation(*new_prot_loc);
                        m_EditedFeat->SetPartial(m_EditedFeat->GetLocation().IsPartialStart(eExtreme_Biological) || m_EditedFeat->GetLocation().IsPartialStop(eExtreme_Biological));
                        CSeq_entry_Handle seh = prot_bsh.GetSeq_entry_Handle();
                        m_CreatedFeatures[m_Feat] = seh;
                        m_ChangedFeatures[m_Feat] = m_EditedFeat;
                    }
                }             
            }
            if (m_ChangedFeatures.find(m_Feat) != m_ChangedFeatures.end())
                m_EditedFeat = m_ChangedFeatures[m_Feat];
            else
            {
                m_EditedFeat.Reset(new CSeq_feat);
                m_EditedFeat->Assign(*m_Feat.GetOriginalSeq_feat());
            }
            Modify(action);            
        }
        ++count;
        if (count >= m_max_records)
            break;       
    }

    if (m_selector.GetFeatType() == CSeqFeatData::e_Prot)
    {
        size_t count = 0;
        for (CBioseq_CI b_iter(m_TopSeqEntry, objects::CSeq_inst::eMol_aa); b_iter; ++b_iter)
        {
            m_Feat.Reset();
            m_EditedFeat.Reset();
            CBioseq_Handle prot_bsh = *b_iter;
            m_CurrentSeqEntry = prot_bsh.GetSeq_entry_Handle();
            CFeat_CI feat_ci(m_CurrentSeqEntry, SAnnotSelector(CSeqFeatData::e_Prot));
            if (feat_ci)
                continue;
            if (prot_bsh && m_constraint->Match(prot_bsh)) 
            {
               if (!IsNOOP(action))
                {                           
                    CRef<CSeq_loc> prot_loc = prot_bsh.GetRangeSeq_loc(0,0);
                    CFeat_CI prot_feat_ci(*m_scope, *prot_loc, SAnnotSelector(CSeqFeatData::eSubtype_prot)); 
                    if (prot_feat_ci && m_constraint->Match(prot_feat_ci->GetSeq_feat_Handle()))
                    {
                        m_Feat = prot_feat_ci->GetSeq_feat_Handle();
                    }
                    else
                    {
                        m_EditedFeat.Reset(new CSeq_feat);
                        m_EditedFeat->SetData().SetProt();
                        m_EditedFeat->SetLocation(*prot_loc);
                        m_EditedFeat->SetPartial(m_EditedFeat->GetLocation().IsPartialStart(eExtreme_Biological) || m_EditedFeat->GetLocation().IsPartialStop(eExtreme_Biological));
                        CSeq_entry_Handle seh = prot_bsh.GetSeq_entry_Handle();
                        CConstRef<CSeq_entry> entry = seh.GetSeq_entryCore();
                        CSeq_entry_Handle new_seh = m_scope->AddTopLevelSeqEntry(*entry, CScope::kPriority_Default, CScope::eExist_Get);
                        CSeq_annot_Handle ftable;
                        for (CSeq_annot_CI annot_ci(new_seh, CSeq_annot_CI::eSearch_entry); annot_ci; ++annot_ci) 
                        {
                            if ((*annot_ci).IsFtable()) 
                            {
                                ftable = *annot_ci;
                                break;
                            }
                        }
                        CSeq_entry_EditHandle eh = new_seh.GetEditHandle();
                        if (!ftable) 
                        {
                            CRef<CSeq_annot> new_annot(new CSeq_annot());
                            ftable = eh.AttachAnnot(*new_annot);
                        }
                        
                        CSeq_annot_EditHandle aeh(ftable);
                        m_Feat = aeh.AddFeat(*m_EditedFeat);
                        if (m_constraint->Match(m_Feat))
                        {
                            m_CreatedFeatures[m_Feat] = seh; // this seq-entry-handle should be in the original scope
                            m_ChangedFeatures[m_Feat] = m_EditedFeat;
                        }
                        else
                        {
                            m_Feat.Reset();
                        }
                    }
                }
                
                if (m_Feat)
                {
                    if (m_ChangedFeatures.find(m_Feat) != m_ChangedFeatures.end())
                        m_EditedFeat = m_ChangedFeatures[m_Feat];
                    else
                    {
                        m_EditedFeat.Reset(new CSeq_feat);
                        m_EditedFeat->Assign(*m_Feat.GetOriginalSeq_feat());
                    }
                    Modify(action);      
                }
            }
            ++count;
            if (count >= m_max_records)
                break;            
        }
    }

}

void IEditingActionFeat::FindRelatedOrCreateProtFeat(EActionType action)
{
    if (!m_TopSeqEntry)
        return;

    IEditingActionFeat *feat_other = dynamic_cast<IEditingActionFeat*>(m_Other);
    if (!feat_other)
    {
        Find(action); 
        return;
    }
    CSeq_feat_Handle other_fh = feat_other->GetFeatHandle();
    m_Feat.Reset();

    SAnnotSelector sel = m_selector;
    if (m_selector.GetFeatType() == CSeqFeatData::e_Prot && other_fh.GetFeatType() != CSeqFeatData::e_Prot)
    {
        sel = SAnnotSelector(CSeqFeatData::eSubtype_cdregion);
    }

    CScope &scope = m_TopSeqEntry.GetScope();
    CSeq_feat_Handle feat_fh = x_FindGeneForFeature(other_fh.GetLocation(), scope, sel.GetFeatSubtype());      
       
    if(feat_fh)
    {
        m_EditedFeat.Reset();
        m_Feat = feat_fh;
        if (m_Feat.IsSetProduct())
        {
            const CSeq_loc& prot_loc = m_Feat.GetProduct();
            CBioseq_Handle prot_bsh = m_TopSeqEntry.GetScope().GetBioseqHandle(prot_loc);
            if (prot_bsh) 
            {
                CFeat_CI prot_feat_ci(prot_bsh, SAnnotSelector(CSeqFeatData::eSubtype_prot));
                if ( prot_feat_ci )
                {
                    CSeq_feat_Handle prot_fh = prot_feat_ci->GetSeq_feat_Handle();
                    m_Feat = prot_fh;
                }
                else if (!IsNOOP(action) && m_ChangedFeatures.find(m_Feat) == m_ChangedFeatures.end())
                {                           
                    m_EditedFeat.Reset(new CSeq_feat);
                    m_EditedFeat->SetData().SetProt();
                    CRef<CSeq_loc> new_prot_loc(new CSeq_loc);
                    new_prot_loc->Assign(prot_loc);
                    m_EditedFeat->SetLocation(*new_prot_loc);
                    m_EditedFeat->SetPartial(m_EditedFeat->GetLocation().IsPartialStart(eExtreme_Biological) || m_EditedFeat->GetLocation().IsPartialStop(eExtreme_Biological));
                    CSeq_entry_Handle seh = prot_bsh.GetSeq_entry_Handle();
                    m_CreatedFeatures[m_Feat] = seh;
                    m_ChangedFeatures[m_Feat] = m_EditedFeat;
                }
            }
        }
    }

   
    if (m_Feat)
    {
        if (m_ChangedFeatures.find(m_Feat) != m_ChangedFeatures.end())
            m_EditedFeat = m_ChangedFeatures[m_Feat];
        else
        {
            m_EditedFeat.Reset(new CSeq_feat);
            m_EditedFeat->Assign(*m_Feat.GetOriginalSeq_feat());
        }
        Modify(action);
    }
}

void IEditingActionFeat::Modify(EActionType action)
{
    Action(action);
    if (m_modified)
    {
        m_ChangedFeatures[m_Feat] = m_EditedFeat;
    }
}

bool IEditingActionFeat::SameObject()
{
    IEditingActionFeat *feat_source = dynamic_cast<IEditingActionFeat*>(m_Other);
    if (feat_source 
        && feat_source->m_selector.GetAnnotType() == m_selector.GetAnnotType() 
        && feat_source->m_selector.GetFeatType() == m_selector.GetFeatType() 
        && feat_source->m_selector.GetFeatSubtype() == m_selector.GetFeatSubtype() )
        return true;
    return false;
}

bool IEditingActionFeat::OtherFeatIs(const CSeqFeatData::ESubtype subtype)
{
    IEditingActionFeat *feat_source = dynamic_cast<IEditingActionFeat*>(m_Other);
    if (feat_source && feat_source->m_Feat && feat_source->m_Feat.GetFeatSubtype() == subtype)
        return true;
    if (subtype == CSeqFeatData::eSubtype_gene)
    {
	CEditingActionFeatGeneLocusRna *locus = dynamic_cast<CEditingActionFeatGeneLocusRna*>(m_Other);
	if (locus)
	    return true;
	CEditingActionFeatRnaToGeneDesc *desc = dynamic_cast<CEditingActionFeatRnaToGeneDesc*>(m_Other);
	if (desc)
	    return true;
	CEditingActionFeatRnaToGeneMaploc *maploc = dynamic_cast<CEditingActionFeatRnaToGeneMaploc*>(m_Other);
	if (maploc)
	    return true;
	CEditingActionFeatRnaToGeneLocus_tag *locus_tag = dynamic_cast<CEditingActionFeatRnaToGeneLocus_tag*>(m_Other);
	if (locus_tag)
	    return true;
	CEditingActionFeatRnaToGeneSynonym *synonym = dynamic_cast<CEditingActionFeatRnaToGeneSynonym*>(m_Other);
	if (synonym)
	    return true;
	CEditingActionFeatRnaToGeneComment *comment = dynamic_cast<CEditingActionFeatRnaToGeneComment*>(m_Other);
	if (comment)
	    return true;
    }
    return false;
}


void IEditingActionFeat::SwapContext(IEditingAction* source)
{
    IEditingActionFeat *feat_source = dynamic_cast<IEditingActionFeat*>(source);
    if (feat_source)
    {
        swap(m_Feat, feat_source->m_Feat);
        swap(m_EditedFeat, feat_source->m_EditedFeat);
    }
    IEditingAction::SwapContext(source);
}

void IEditingActionFeat::SetFeat(CSeq_feat_Handle fh)
{
    m_Feat = fh;
    m_EditedFeat.Reset(new CSeq_feat);
    m_EditedFeat->Assign(*fh.GetOriginalSeq_feat());
}


void IEditingActionFeat::SetFeatForAnotherFeat(CSeq_feat_Handle fh, const CSeqFeatData::ESubtype subtype)
{
    m_Feat.Reset();
    if (fh.GetFeatSubtype() != subtype) 
    {             
        CSeq_feat_Handle gene_fh = x_FindGeneForFeature(fh.GetLocation(), fh.GetScope(), subtype);                  
        if (gene_fh)
            m_Feat = gene_fh;
    }
    
    if (!m_Feat)
        m_Feat = fh;

    m_EditedFeat.Reset(new CSeq_feat);
    m_EditedFeat->Assign(*m_Feat.GetOriginalSeq_feat());
}

CSeq_feat_Handle IEditingActionFeat::GetFeatHandle(void)
{
    return m_Feat;
}

void IEditingActionFeat::ResetScope()
{
    if (m_scope)
        m_scope->ResetHistory();
    CRef<CObjectManager> object_manager = CObjectManager::GetInstance();
    m_scope.Reset(new CScope(*object_manager));
    m_scope->AddDefaults();
}

void IEditingActionFeat::RemoveGeneXref()
{
    EDIT_EACH_SEQFEATXREF_ON_SEQFEAT(it, *m_EditedFeat) 
    {
        if ((*it)->IsSetData() && (*it)->GetData().IsGene()) 
        {
            ERASE_SEQFEATXREF_ON_SEQFEAT(it, *m_EditedFeat);
        }
    }
    if (m_EditedFeat->GetXref().empty())
        m_EditedFeat->ResetXref();
}

CEditingActionFeatComment::CEditingActionFeatComment(CSeq_entry_Handle seh, const CSeqFeatData::ESubtype subtype, const CSeqFeatData::E_Choice feat_type)
    : IEditingActionFeat(seh, subtype, feat_type, "CEditingActionFeatComment")
{
}

bool CEditingActionFeatComment::IsSetValue()
{
    return m_EditedFeat->IsSetComment();
}

void CEditingActionFeatComment::SetValue(const string &value)
{
    m_EditedFeat->SetComment(value);
}

string CEditingActionFeatComment::GetValue()
{
    return m_EditedFeat->GetComment();
}

void CEditingActionFeatComment::ResetValue()
{
    m_EditedFeat->ResetComment();
}

CEditingActionFeatException::CEditingActionFeatException(CSeq_entry_Handle seh, const CSeqFeatData::ESubtype subtype, const CSeqFeatData::E_Choice feat_type)
    : IEditingActionFeat(seh, subtype, feat_type, "CEditingActionFeatException")
{
}

bool CEditingActionFeatException::IsSetValue()
{
    return m_EditedFeat->IsSetExcept() && m_EditedFeat->GetExcept() && m_EditedFeat->IsSetExcept_text();
}

void CEditingActionFeatException::SetValue(const string &value)
{
    m_EditedFeat->SetExcept(true);
    m_EditedFeat->SetExcept_text(value);
}

string CEditingActionFeatException::GetValue()
{
    return m_EditedFeat->GetExcept_text();
}

void CEditingActionFeatException::ResetValue()
{
    m_EditedFeat->ResetExcept();
    m_EditedFeat->ResetExcept_text();
}


CEditingActionFeatGbQual::CEditingActionFeatGbQual(CSeq_entry_Handle seh, const CSeqFeatData::ESubtype subtype, const CSeqFeatData::E_Choice feat_type, const string &qual, const string &name)
    : IEditingActionFeat(seh, subtype, feat_type, name), m_qual(qual), m_erase(false)
{
}

void CEditingActionFeatGbQual::Modify(EActionType action)
{
    bool found = false;
    if (IsFrom(action) && m_EditedFeat->IsSetQual() && !m_EditedFeat->GetQual().empty() && m_EditedFeat->GetQual().capacity() < 2 * m_EditedFeat->GetQual().size())
    {
        m_EditedFeat->SetQual().reserve(2 * m_EditedFeat->GetQual().size());
    }
    if (!IsCreateNew(action))
    {
        EDIT_EACH_GBQUAL_ON_SEQFEAT(gbqual_it, *m_EditedFeat)
        {
            if ((*gbqual_it) && (*gbqual_it)->IsSetQual() && (*gbqual_it)->GetQual() == m_qual)
            {
                found = true;
                m_erase = false;
                m_GBqual = *gbqual_it;
                IEditingActionFeat::Modify(action);
                if (m_erase)
                    ERASE_GBQUAL_ON_SEQFEAT(gbqual_it, *m_EditedFeat);
            }
        }
    }
    if (!found)
    {
        m_GBqual.Reset();
        m_erase = false;
        IEditingActionFeat::Modify(action);
    }
    if (IsFrom(action))
    {
        m_EditedFeat->SetQual().shrink_to_fit();     
    }
    if (m_EditedFeat->GetQual().empty())
        m_EditedFeat->ResetQual();
}

bool CEditingActionFeatGbQual::IsSetValue()
{
    return m_GBqual && m_GBqual->IsSetVal();
}

void CEditingActionFeatGbQual::SetValue(const string &value)
{
    if (m_GBqual)
        m_GBqual->SetVal(value);
    else if (!value.empty())
        m_EditedFeat->AddQualifier(m_qual, value);
}

string CEditingActionFeatGbQual::GetValue()
{
    return m_GBqual->GetVal();
}

void CEditingActionFeatGbQual::ResetValue()
{
    m_erase = true;
}

CEditingActionFeatGbQualTwoNames::CEditingActionFeatGbQualTwoNames(CSeq_entry_Handle seh, const CSeqFeatData::ESubtype subtype, const CSeqFeatData::E_Choice feat_type, const string &qual1, const string &qual2)
    : CEditingActionFeatGbQual(seh, subtype, feat_type, qual1, "CEditingActionFeatGbQualTwoNames"), m_qual2(qual2)
{
}

void CEditingActionFeatGbQualTwoNames::Modify(EActionType action)
{
    bool found = false;
    if (IsFrom(action) && m_EditedFeat->IsSetQual() && !m_EditedFeat->GetQual().empty() && m_EditedFeat->GetQual().capacity() < 2 * m_EditedFeat->GetQual().size())
    {
        m_EditedFeat->SetQual().reserve(2 * m_EditedFeat->GetQual().size());
    }
    if (!IsCreateNew(action))
    {
        EDIT_EACH_GBQUAL_ON_SEQFEAT(gbqual_it, *m_EditedFeat)
        {
            if ((*gbqual_it) && (*gbqual_it)->IsSetQual() && ((*gbqual_it)->GetQual() == m_qual || (*gbqual_it)->GetQual() == m_qual2))
            {
                found = true;
                m_erase = false;
                m_GBqual = *gbqual_it;
                IEditingActionFeat::Modify(action);
                if (m_erase)
                    ERASE_GBQUAL_ON_SEQFEAT(gbqual_it, *m_EditedFeat);
            }
        }
    }
    if (!found)
    {
        m_GBqual.Reset();
        m_erase = false;
        IEditingActionFeat::Modify(action);
    }
    if (IsFrom(action))
    {
        m_EditedFeat->SetQual().shrink_to_fit();      
    }
    if (m_EditedFeat->GetQual().empty())
        m_EditedFeat->ResetQual();
}

CEditingActionFeatDualVal1::CEditingActionFeatDualVal1(CSeq_entry_Handle seh, const CSeqFeatData::ESubtype subtype, const CSeqFeatData::E_Choice feat_type, const string &qual)
    : CEditingActionFeatGbQual(seh, subtype, feat_type, qual, "CEditingActionFeatDualVal1")
{
}

bool CEditingActionFeatDualVal1::IsSetValue()
{
    if (m_GBqual && m_GBqual->IsSetVal())
    {
        string val = m_GBqual->GetVal();
        string element_type, element_name;
        NStr::SplitInTwo(val, ":", element_type, element_name);        
        return !element_type.empty();
    }
    return false;
}

void CEditingActionFeatDualVal1::SetValue(const string &value)
{
    if (m_GBqual)
    {
        if (m_GBqual->IsSetVal())
        {
            string val = m_GBqual->GetVal();
            string element_type, element_name;
            NStr::SplitInTwo(val, ":", element_type, element_name);            
            element_type = value;
            if (!element_name.empty())
                element_type += ":" + element_name;
            m_GBqual->SetVal(element_type);
        }
        else
            m_GBqual->SetVal(value);
    }
    else
        m_EditedFeat->AddQualifier(m_qual, value);
}

string CEditingActionFeatDualVal1::GetValue()
{
    string val = m_GBqual->GetVal();
    string element_type, element_name;
    NStr::SplitInTwo(val, ":", element_type, element_name);  
    return element_type;
}

void CEditingActionFeatDualVal1::ResetValue()
{
    if (m_GBqual->IsSetVal())
    {
        string val = m_GBqual->GetVal();
        string element_type, element_name;
        NStr::SplitInTwo(val, ":", element_type, element_name);            
        if (!element_name.empty())
            m_GBqual->SetVal(":" + element_name);
        else
            m_erase = true;
    }
    m_erase = true;
}

CEditingActionFeatDualVal2::CEditingActionFeatDualVal2(CSeq_entry_Handle seh, const CSeqFeatData::ESubtype subtype, const CSeqFeatData::E_Choice feat_type, const string &qual)
    : CEditingActionFeatGbQual(seh, subtype, feat_type, qual, "CEditingActionFeatDualVal2")
{
}

bool CEditingActionFeatDualVal2::IsSetValue()
{
    if (m_GBqual && m_GBqual->IsSetVal())
    {
        string val = m_GBqual->GetVal();
        string element_type, element_name;
        NStr::SplitInTwo(val, ":", element_type, element_name);        
        return !element_name.empty();
    }
    return false;
}

void CEditingActionFeatDualVal2::SetValue(const string &value)
{
    if (m_GBqual)
    {
        if (m_GBqual->IsSetVal())
        {
            string val = m_GBqual->GetVal();
            string element_type, element_name;
            NStr::SplitInTwo(val, ":", element_type, element_name);            
            element_name = value;
            if (!element_type.empty())
                element_name = element_type + ":" + element_name;
            m_GBqual->SetVal(element_name);
        }
        else
            m_GBqual->SetVal(value);
    }
    else
        m_EditedFeat->AddQualifier(m_qual, value);
}

string CEditingActionFeatDualVal2::GetValue()
{
    string val = m_GBqual->GetVal();
    string element_type, element_name;
    NStr::SplitInTwo(val, ":", element_type, element_name);  
    return element_name;
}

void CEditingActionFeatDualVal2::ResetValue()
{
    if (m_GBqual->IsSetVal())
    {
        string val = m_GBqual->GetVal();
        string element_type, element_name;
        NStr::SplitInTwo(val, ":", element_type, element_name);            
        if (!element_type.empty())
            m_GBqual->SetVal(element_type);
        else
            m_erase = true;
    }
    m_erase = true;
}



CEditingActionFeatDbxref::CEditingActionFeatDbxref(CSeq_entry_Handle seh, const CSeqFeatData::ESubtype subtype, const CSeqFeatData::E_Choice feat_type)
    : IEditingActionFeat(seh, subtype, feat_type, "CEditingActionFeatDbxref"), m_erase(false)
{
}

void CEditingActionFeatDbxref::Modify(EActionType action)
{
    bool found = false;
    if (IsFrom(action) && m_EditedFeat->IsSetDbxref() && !m_EditedFeat->GetDbxref().empty() && m_EditedFeat->GetDbxref().capacity() < 2 * m_EditedFeat->GetDbxref().size())
    {
        m_EditedFeat->SetDbxref().reserve(2 * m_EditedFeat->GetDbxref().size());
    }
    if (!IsCreateNew(action))
    {
        EDIT_EACH_DBXREF_ON_SEQFEAT(dbxref_it, *m_EditedFeat)
        {
            found = true;
            m_erase = false;
            m_Dbtag = *dbxref_it;
            IEditingActionFeat::Modify(action);
            if (m_erase)
                ERASE_DBXREF_ON_SEQFEAT(dbxref_it, *m_EditedFeat);
        }
    }

    if (!found)
    {
        m_Dbtag.Reset();
        m_erase = false;
        IEditingActionFeat::Modify(action);
    }
    if (IsFrom(action))
    {
        m_EditedFeat->SetDbxref().shrink_to_fit();
    }
    if (m_EditedFeat->IsSetDbxref() && m_EditedFeat->GetDbxref().empty())
    {
        m_EditedFeat->ResetDbxref();
    }
}

bool CEditingActionFeatDbxref::IsSetValue()
{
    return m_Dbtag && m_Dbtag->IsSetDb() && m_Dbtag->IsSetTag();
}

void CEditingActionFeatDbxref::SetValue(const string &value)
{
    string db, tag;
    NStr::SplitInTwo(value, ":", db, tag);
    int id = NStr::StringToInt(value, NStr::fConvErr_NoThrow);

    if (!db.empty() && !tag.empty())
    {
        if (m_Dbtag)
        {
            m_Dbtag->SetDb(db);
            m_Dbtag->ResetTag();
            if (id != 0)
                m_Dbtag->SetTag().SetId(id);
            else
                m_Dbtag->SetTag().SetStr(tag);
        }
        else
        {
            CRef<CDbtag> dbtag(new CDbtag);
            dbtag->SetDb(db);
            if (id != 0)
                dbtag->SetTag().SetId(id);
            else
                dbtag->SetTag().SetStr(tag);
            m_EditedFeat->SetDbxref().push_back(dbtag);
        }
    }
}

string CEditingActionFeatDbxref::GetValue()
{
    string db = m_Dbtag->GetDb();
    string tag;
    if (m_Dbtag->GetTag().IsStr())
        tag = m_Dbtag->GetTag().GetStr();
    if (m_Dbtag->GetTag().IsId())
        tag = NStr::IntToString(m_Dbtag->GetTag().GetId());
    return db + ":" + tag;
}

void CEditingActionFeatDbxref::ResetValue()
{
    m_erase = true;
}


CEditingActionFeatEvidence::CEditingActionFeatEvidence(CSeq_entry_Handle seh, const CSeqFeatData::ESubtype subtype, const CSeqFeatData::E_Choice feat_type)
    : IEditingActionFeat(seh, subtype, feat_type, "CEditingActionFeatEvidence")
{
}

bool CEditingActionFeatEvidence::IsSetValue()
{
    return m_EditedFeat->IsSetExp_ev();
}

void CEditingActionFeatEvidence::SetValue(const string &value)
{
    if (CSeq_feat::ENUM_METHOD_NAME(EExp_ev)()->IsValidName(value))
    {
        m_EditedFeat->SetExp_ev(static_cast<CSeq_feat::EExp_ev>(CSeq_feat::ENUM_METHOD_NAME(EExp_ev)()->FindValue(value)));
    } 
}

string CEditingActionFeatEvidence::GetValue()
{
    return CSeq_feat::ENUM_METHOD_NAME(EExp_ev)()->FindName(m_EditedFeat->GetExp_ev(), true);
}

void CEditingActionFeatEvidence::ResetValue()
{
    m_EditedFeat->ResetExp_ev();
}

CEditingActionFeatPseudo::CEditingActionFeatPseudo(CSeq_entry_Handle seh, const CSeqFeatData::ESubtype subtype, const CSeqFeatData::E_Choice feat_type)
    : IEditingActionFeat(seh, subtype, feat_type, "CEditingActionFeatPseudo")
{
}

bool CEditingActionFeatPseudo::IsSetValue()
{
    return m_EditedFeat->IsSetPseudo();
}

void CEditingActionFeatPseudo::SetValue(const string &value)
{
    m_EditedFeat->SetPseudo(true);
    if (!value.empty() && !NStr::EqualNocase(value,"Unqualified"))
        m_EditedFeat->AddOrReplaceQualifier("pseudogene", value);
    else
        m_EditedFeat->RemoveQualifier("pseudogene");
}

string CEditingActionFeatPseudo::GetValue()
{
    string r =  m_EditedFeat->GetNamedQual("pseudogene");
    if (r.empty())
        r = "Unqualified";
    return r;
}

void CEditingActionFeatPseudo::ResetValue()
{
    m_EditedFeat->ResetPseudo();
    m_EditedFeat->RemoveQualifier("pseudogene");
}

CEditingActionFeatCodonsRecognized::CEditingActionFeatCodonsRecognized(CSeq_entry_Handle seh, const CSeqFeatData::ESubtype subtype, const CSeqFeatData::E_Choice feat_type)
    : CEditingActionFeatGbQual(seh, subtype, feat_type, "codons_recognized", "CEditingActionFeatCodonsRecognized")
{
}

bool CEditingActionFeatCodonsRecognized::IsSetValue()
{
    bool is_trna = m_EditedFeat->IsSetData() && m_EditedFeat->GetData().IsRna() && m_EditedFeat->GetData().GetRna().IsSetType() && m_EditedFeat->GetData().GetRna().GetType() == CRNA_ref::eType_tRNA
        && m_EditedFeat->GetData().GetRna().IsSetExt() && m_EditedFeat->GetData().GetRna().GetExt().IsTRNA()
        && m_EditedFeat->GetData().GetRna().GetExt().GetTRNA().IsSetCodon()
        && !m_EditedFeat->GetData().GetRna().GetExt().GetTRNA().GetCodon().empty();
    bool is_qual = m_GBqual && m_GBqual->IsSetVal();
    return is_trna || is_qual;
}

static bool s_IsATGC(char ch)
{
    if (ch == 'A' || ch == 'T' || ch == 'G' || ch == 'C' || ch == 'U') {
        return true;
    } else {
        return false;
    }
}


static const  string kAmbiguities = "MRSVWYHKDBN";
static const string kReplacements[] = {
  "AC", "AG", "CG", "ACG", "AT", "CT", "ACT", "GT", "AGT", "CGT", "ACGT" };

static const string s_GetExpansion (const string& ch)
{
    size_t pos = NStr::Find(kAmbiguities, ch);
    if (pos != string::npos) {
        return kReplacements[pos];
    } else {
        return ch;
    }
}


static vector<string> ParseDegenerateCodons (string codon)
{
    vector<string> replacements;

    if (codon.length() == 3 && s_IsATGC(codon.c_str()[0])) {
        string this_codon = codon.substr(0, 1);
        replacements.push_back(this_codon);

        for (int i = 1; i < 3; i++) {
            string ch = s_GetExpansion (codon.substr(i, 1));
            int num_now = replacements.size();
            // add copies for each expansion letter beyond the first
            for (unsigned int j = 1; j < ch.length(); j++) {
                for (int k = 0; k < num_now; k++) {
                    string cpy = replacements[k];
                    replacements.push_back(cpy);
                }
            }
            for (int k = 0; k < num_now; k++) {
                for (unsigned int j = 0; j < ch.length(); j++) {
                    replacements[j * num_now + k].append(ch.substr(j, 1));
                }
            }
        }   
    } else {
        replacements.push_back(codon);
    }
    return replacements;
}

void CEditingActionFeatCodonsRecognized::SetValue(const string &value)
{
    if (m_GBqual)
        m_GBqual->SetVal(value);
    else if (m_EditedFeat->IsSetData() && m_EditedFeat->GetData().IsRna() 
             && m_EditedFeat->GetData().GetRna().IsSetType() 
             && m_EditedFeat->GetData().GetRna().GetType() == CRNA_ref::eType_tRNA)
    {
        m_EditedFeat->SetData().SetRna().SetExt().SetTRNA().ResetCodon();
        string v = value;
        NStr::TruncateSpacesInPlace(v);
        NStr::ToUpper(v);
        if (!v.empty()) 
        {
            vector<string> codons = ParseDegenerateCodons(v);
            for (unsigned int j = 0; j < codons.size(); j++) 
            {
                int val = CGen_code_table::CodonToIndex(codons[j]);
                if (val > -1) 
                {
                     m_EditedFeat->SetData().SetRna().SetExt().SetTRNA().SetCodon().push_back(val);
                }
            }
        }       
    }
    else
        m_EditedFeat->AddQualifier(m_qual, value);   
}

string CEditingActionFeatCodonsRecognized::GetValue()
{
    string r;
    if (m_GBqual)
    {
        r = m_GBqual->GetVal();
    }
    else if (m_EditedFeat->IsSetData() && m_EditedFeat->GetData().IsRna() && m_EditedFeat->GetData().GetRna().IsSetType() && m_EditedFeat->GetData().GetRna().GetType() == CRNA_ref::eType_tRNA
        && m_EditedFeat->GetData().GetRna().IsSetExt() && m_EditedFeat->GetData().GetRna().GetExt().IsTRNA()
        && m_EditedFeat->GetData().GetRna().GetExt().GetTRNA().IsSetCodon()
        && !m_EditedFeat->GetData().GetRna().GetExt().GetTRNA().GetCodon().empty())
    {
        for (CTrna_ext::TCodon::const_iterator i = m_EditedFeat->GetData().GetRna().GetExt().GetTRNA().GetCodon().begin(); i != m_EditedFeat->GetData().GetRna().GetExt().GetTRNA().GetCodon().end(); ++i)
            r += CGen_code_table::IndexToCodon(*i);
    }
    return r;
}

void CEditingActionFeatCodonsRecognized::ResetValue()
{
    if (m_GBqual)
        m_erase = true;
    else if (m_EditedFeat->IsSetData() && m_EditedFeat->GetData().IsRna() && m_EditedFeat->GetData().GetRna().IsSetExt() && m_EditedFeat->GetData().GetRna().GetExt().IsTRNA())
        m_EditedFeat->SetData().SetRna().SetExt().SetTRNA().ResetCodon();
}

CEditingActionFeatTranslExcept::CEditingActionFeatTranslExcept(CSeq_entry_Handle seh, const CSeqFeatData::ESubtype subtype, const CSeqFeatData::E_Choice feat_type)
    : CEditingActionFeatGbQual(seh, subtype, feat_type, "transl_except", "CEditingActionFeatTranslExcept")
{
}

void CEditingActionFeatTranslExcept::Modify(EActionType action)
{
    bool found = false;
    if (!IsCreateNew(action) && m_EditedFeat->IsSetData() && m_EditedFeat->GetData().IsCdregion())
    {
        EDIT_EACH_CODEBREAK_ON_CDREGION(code_break_it, m_EditedFeat->SetData().SetCdregion())
        {
            if (*code_break_it)
            {
                found = true;
                m_erase = false;
                m_CodeBreak = *code_break_it;
                IEditingActionFeat::Modify(action);
                if (m_erase)
                    ERASE_CODEBREAK_ON_CDREGION(code_break_it, m_EditedFeat->SetData().SetCdregion());
            }
        }
    }
    if (!found)
    {
        m_CodeBreak.Reset();
        m_erase = false;
        CEditingActionFeatGbQual::Modify(action);
    }
    if (m_EditedFeat->IsSetData() && m_EditedFeat->GetData().IsCdregion() && m_EditedFeat->GetData().GetCdregion().IsSetCode_break() && 
        m_EditedFeat->GetData().GetCdregion().GetCode_break().empty())
        m_EditedFeat->SetData().SetCdregion().ResetCode_break();
}

bool CEditingActionFeatTranslExcept::IsSetValue()
{
    bool is_qual = m_GBqual && m_GBqual->IsSetVal();
    return m_CodeBreak || is_qual;
}

void CEditingActionFeatTranslExcept::SetValue(const string &value)
{
    if (m_GBqual)
        m_GBqual->SetVal(value);
    else if (m_CodeBreak)
    {
        vector<string> v;
        NStr::Split(value, ":", v);
        if (v.size() == 2)
        {
            char aa = v[1][0];
            m_CodeBreak->SetAa().SetNcbieaa(aa);
            string loc_label = v[0];
            if (!NStr::IsBlank(loc_label)) 
            {
                int start = NStr::StringToInt(loc_label);
                start -= 1;
                const CSeq_loc &loc = m_EditedFeat->GetLocation();
                CRef<CSeq_loc> cb_loc (new CSeq_loc());
                cb_loc->SetPnt().SetPoint(start);
                cb_loc->SetPnt().SetId().Assign(*loc.GetId());
                if (loc.IsSetStrand() && loc.GetStrand() == eNa_strand_minus) 
                    cb_loc->SetPnt().SetStrand(eNa_strand_minus);
                
                m_CodeBreak->SetLoc(*cb_loc);
            }
        }
    }
    else
        m_EditedFeat->AddQualifier(m_qual, value);   
}

string CEditingActionFeatTranslExcept::GetValue()
{
    string r;
    if (m_GBqual)
    {
        r = m_GBqual->GetVal();
    }
    else if (m_CodeBreak)
    {
        string loc_label;
        if (m_CodeBreak->IsSetLoc() && m_CodeBreak->GetLoc().Which() != CSeq_loc::e_not_set) 
        {
            // find offset from beginning of protein
            CScope &scope = m_Feat.GetScope();
            TSeqPos seq_pos = m_CodeBreak->GetLoc().GetStart(eExtreme_Positional);
            seq_pos += 1;
            loc_label = NStr::NumericToString(seq_pos);
        }
        char aa = 0;
        string str;
        vector<char> seqData;
        if (m_CodeBreak->IsSetAa()) 
        {
            switch (m_CodeBreak->GetAa().Which()) 
            {
            case CCode_break::C_Aa::e_Ncbi8aa:
                str = m_CodeBreak->GetAa().GetNcbi8aa();
                CSeqConvert::Convert(str, CSeqUtil::e_Ncbi8aa, 0, str.size(), seqData, CSeqUtil::e_Ncbieaa);
                aa = seqData[0];
                break;
            case CCode_break::C_Aa::e_Ncbistdaa:
                str = m_CodeBreak->GetAa().GetNcbi8aa();
                CSeqConvert::Convert(str, CSeqUtil::e_Ncbistdaa, 0, str.size(), seqData, CSeqUtil::e_Ncbieaa);
                aa = seqData[0];
                break;
            case CCode_break::C_Aa::e_Ncbieaa:
                seqData.push_back(m_CodeBreak->GetAa().GetNcbieaa());
                aa = seqData[0];
                break;
            default:
                break;
            }
        }
        if (!loc_label.empty())
            r = loc_label;
        if (aa != 0)
        {
            if (!r.empty())
                r += ":";
            r += aa;
        }
    }
    return r;
}

void CEditingActionFeatTranslExcept::ResetValue()
{
    if (m_GBqual || m_CodeBreak)
        m_erase = true;
}

CEditingActionFeatAnticodon::CEditingActionFeatAnticodon(CSeq_entry_Handle seh, const CSeqFeatData::ESubtype subtype, const CSeqFeatData::E_Choice feat_type)
    :  CEditingActionFeatGbQual(seh, subtype, feat_type, "anticodon", "CEditingActionFeatAnticodon")
{
}

bool CEditingActionFeatAnticodon::IsSetValue()
{
    bool is_trna = m_EditedFeat->IsSetData() && m_EditedFeat->GetData().IsRna() && m_EditedFeat->GetData().GetRna().IsSetType() && m_EditedFeat->GetData().GetRna().GetType() == CRNA_ref::eType_tRNA
        && m_EditedFeat->GetData().GetRna().IsSetExt() && m_EditedFeat->GetData().GetRna().GetExt().IsTRNA()
        && m_EditedFeat->GetData().GetRna().GetExt().GetTRNA().IsSetAnticodon();
    bool is_qual = m_GBqual && m_GBqual->IsSetVal();
    return is_trna || is_qual;
}

string CEditingActionFeatAnticodon::GetIntervalString(const CSeq_interval& seq_int)
{
  bool partial5 = seq_int.IsPartialStart(eExtreme_Biological);
  bool partial3 = seq_int.IsPartialStop(eExtreme_Biological);
  string rval(kEmptyStr);

  if (seq_int.CanGetStrand() && seq_int.GetStrand() == eNa_strand_minus) {
    rval = (string)"complement(" + (partial3 ? "<" : "")
             + NStr::UIntToString(seq_int.GetStart(eExtreme_Positional))
             + ".." + (partial5 ? ">" : "")
             + NStr::UIntToString(seq_int.GetStop(eExtreme_Positional)) + ")";
  } else {
    rval = (partial5 ? "<" : "")
             + NStr::UIntToString(seq_int.GetStart(eExtreme_Positional))
             + ".." + (partial3 ? ">" : "")
             + NStr::UIntToString(seq_int.GetStop(eExtreme_Positional));
  }
  return rval;
};


void CEditingActionFeatAnticodon::SetValue(const string &value)
{
    if (m_GBqual)
        m_GBqual->SetVal(value);
    else  if (m_EditedFeat->IsSetData() && m_EditedFeat->GetData().IsRna() && m_EditedFeat->GetData().GetRna().IsSetType() && m_EditedFeat->GetData().GetRna().GetType() == CRNA_ref::eType_tRNA)
    {
        // we are not writing anticodon value in this circumstances
    }
    else
    {
         m_EditedFeat->AddQualifier(m_qual, value);   
    }
}

string CEditingActionFeatAnticodon::GetValue()
{
    string r;
    if (m_GBqual)
    {
        r = m_GBqual->GetVal();
    }
    else if (m_EditedFeat->IsSetData() && m_EditedFeat->GetData().IsRna() && m_EditedFeat->GetData().GetRna().IsSetType() && m_EditedFeat->GetData().GetRna().GetType() == CRNA_ref::eType_tRNA
        && m_EditedFeat->GetData().GetRna().IsSetExt() && m_EditedFeat->GetData().GetRna().GetExt().IsTRNA()
        && m_EditedFeat->GetData().GetRna().GetExt().GetTRNA().IsSetAnticodon())
    {
        const CSeq_loc &loc = m_EditedFeat->GetData().GetRna().GetExt().GetTRNA().GetAnticodon();
        if (loc.IsInt()) 
        {
         r = GetIntervalString(loc.GetInt());
        }
        else if (loc.IsMix()) 
        {
            list<string> rval;
            ITERATE (list <CRef <CSeq_loc> >, it, loc.GetMix().Get()) 
            {
                if ( (*it)->IsInt()) 
                {
                    rval.push_back(GetIntervalString( (*it)->GetInt() ));
                }
                else 
                {
                    r = "complex location";
                    rval.clear();
                    break;
                }
            }
            if (!rval.empty())
                r = NStr::Join(rval, ", ");
        }
    }
    return r;
}

void CEditingActionFeatAnticodon::ResetValue()
{
    if (m_GBqual)
        m_erase = true;
    else  if (m_EditedFeat->IsSetData() && m_EditedFeat->GetData().IsRna() && m_EditedFeat->GetData().GetRna().IsSetExt() && m_EditedFeat->GetData().GetRna().GetExt().IsTRNA())
        m_EditedFeat->SetData().SetRna().SetExt().SetTRNA().ResetAnticodon();
}



CEditingActionFeatRnaQual::CEditingActionFeatRnaQual(CSeq_entry_Handle seh, const CSeqFeatData::ESubtype subtype, const CSeqFeatData::E_Choice feat_type, const string &qual)
    : IEditingActionFeat(seh, subtype, feat_type, "CEditingActionFeatRnaQual"), m_qual(qual), m_erase(false)
{
}

void CEditingActionFeatRnaQual::Modify(EActionType action)
{
    bool found = false;
    if (!IsCreateNew(action))
    {
        if (m_EditedFeat->IsSetData() && m_EditedFeat->GetData().IsRna() && m_EditedFeat->GetData().GetRna().IsSetExt() &&
            m_EditedFeat->GetData().GetRna().GetExt().IsGen() && m_EditedFeat->GetData().GetRna().GetExt().GetGen().IsSetQuals())
        {
            EDIT_EACH_QUAL_ON_RNAGEN(rnaqual_it, m_EditedFeat->SetData().SetRna().SetExt().SetGen())
            {
                if ((*rnaqual_it)->IsSetQual() && (*rnaqual_it)->GetQual() == m_qual)
                {
                    found = true;
                    m_erase = false;
                    m_RnaQual = *rnaqual_it;
                    IEditingActionFeat::Modify(action);
                    if (m_erase)
                        ERASE_QUAL_ON_RNAGEN(rnaqual_it, m_EditedFeat->SetData().SetRna().SetExt().SetGen());
                }
            }
        }
    }

    if (IsFrom(action) && m_EditedFeat->IsSetQual() && !m_EditedFeat->GetQual().empty() && m_EditedFeat->GetQual().capacity() < 2 * m_EditedFeat->GetQual().size())
    {
        m_EditedFeat->SetQual().reserve(2 * m_EditedFeat->GetQual().size());
    }

    if (!IsCreateNew(action))
    {
        EDIT_EACH_GBQUAL_ON_SEQFEAT(gbqual_it, *m_EditedFeat)
        {
            if ((*gbqual_it)->IsSetQual() && (*gbqual_it)->GetQual() == m_qual)
            {
                found = true;
                m_erase = false;
                m_GBqual = *gbqual_it;
                IEditingActionFeat::Modify(action);
                if (m_erase)
                    ERASE_GBQUAL_ON_SEQFEAT(gbqual_it, *m_EditedFeat);
            }
        }
    }
    if (!found)
    {
        m_GBqual.Reset();
        m_RnaQual.Reset();
        m_erase = false;
        IEditingActionFeat::Modify(action);
    }

    if (IsFrom(action))
    {
        m_EditedFeat->SetQual().shrink_to_fit();
    }
}

bool CEditingActionFeatRnaQual::IsSetValue()
{
    return (m_GBqual && m_GBqual->IsSetVal()) || (m_RnaQual && m_RnaQual->IsSetVal());
}

void CEditingActionFeatRnaQual::SetValue(const string &value)
{
    if (m_GBqual)
        m_GBqual->SetVal(value);
    else if (m_RnaQual)
        m_RnaQual->SetVal(value);
    else if (m_EditedFeat->IsSetData() && m_EditedFeat->GetData().IsRna() && m_EditedFeat->GetData().GetRna().IsSetExt() &&
             m_EditedFeat->GetData().GetRna().GetExt().IsGen())
    {
        CRef<CRNA_qual> qual(new CRNA_qual);
        qual->SetQual(m_qual);
        qual->SetVal(value);
        m_EditedFeat->SetData().SetRna().SetExt().SetGen().SetQuals().Set().push_back(qual);
    }        
    else
        m_EditedFeat->AddQualifier(m_qual, value);
}

string CEditingActionFeatRnaQual::GetValue()
{
    if (m_RnaQual)
        return m_RnaQual->GetVal();
    if (m_GBqual)
        return m_GBqual->GetVal();
    return kEmptyStr;
}

void CEditingActionFeatRnaQual::ResetValue()
{
    m_erase = true;
}

CEditingActionFeatRegion::CEditingActionFeatRegion(CSeq_entry_Handle seh, const CSeqFeatData::ESubtype subtype, const CSeqFeatData::E_Choice feat_type)
    : CEditingActionFeatGbQual(seh, subtype, feat_type, "region", "CEditingActionFeatRegion")
{
}

bool CEditingActionFeatRegion::IsSetValue()
{
    return (m_EditedFeat->IsSetData() && m_EditedFeat->GetData().IsRegion()) ||  
        (m_GBqual && m_GBqual->IsSetVal());
}

void CEditingActionFeatRegion::SetValue(const string &value)
{
    if (m_GBqual)
        m_GBqual->SetVal(value);
    else if (m_EditedFeat->IsSetData() && m_EditedFeat->GetData().IsRegion())
        m_EditedFeat->SetData().SetRegion(value);
    else
        m_EditedFeat->AddQualifier(m_qual, value);   
}

string CEditingActionFeatRegion::GetValue()
{
    if (m_GBqual)
        return m_GBqual->GetVal();
    else if (m_EditedFeat->IsSetData() && m_EditedFeat->GetData().IsRegion())
        return m_EditedFeat->GetData().GetRegion();
    
    return kEmptyStr;
}

void CEditingActionFeatRegion::ResetValue()
{
    if (m_GBqual)
        m_erase = true;
    else if (m_EditedFeat->IsSetData() && m_EditedFeat->GetData().IsRegion())
        m_EditedFeat->ResetData();
}

CEditingActionFeatCodonStart::CEditingActionFeatCodonStart(CSeq_entry_Handle seh, const CSeqFeatData::ESubtype subtype, const CSeqFeatData::E_Choice feat_type)
    : CEditingActionFeatGbQual(seh, subtype, feat_type, "codon_start", "CEditingActionFeatCodonStart")
{
}

bool CEditingActionFeatCodonStart::IsSetValue()
{
    return (m_EditedFeat->IsSetData() && m_EditedFeat->GetData().IsCdregion())
        ||  (m_GBqual && m_GBqual->IsSetVal());
}

void CEditingActionFeatCodonStart::SetValue(const string &value)
{
    if (m_GBqual)
        m_GBqual->SetVal(value);
    else if (m_EditedFeat->IsSetData() && m_EditedFeat->GetData().IsCdregion())
    {
        if (value == "1")
            m_EditedFeat->SetData().SetCdregion().SetFrame(CCdregion::eFrame_one);
        if (value == "2")
            m_EditedFeat->SetData().SetCdregion().SetFrame(CCdregion::eFrame_two);
        if (value == "3")
            m_EditedFeat->SetData().SetCdregion().SetFrame(CCdregion::eFrame_three);
    }
    else
        m_EditedFeat->AddQualifier(m_qual, value);   
}

string CEditingActionFeatCodonStart::GetValue()
{
    string val;
    if (m_GBqual)
        val = m_GBqual->GetVal();
    else if (m_EditedFeat->IsSetData() && m_EditedFeat->GetData().IsCdregion() && m_EditedFeat->GetData().GetCdregion().IsSetFrame())
    {
        switch (m_EditedFeat->GetData().GetCdregion().GetFrame()) 
        {
        case CCdregion::eFrame_one:
        case CCdregion::eFrame_not_set:
            val = "1";
            break;
        case CCdregion::eFrame_two:
            val = "2";
            break;
        case CCdregion::eFrame_three:
            val = "3";
            break;
        }
    } 
   
    if (val.empty())
        val = "1";
    return val;
}

void CEditingActionFeatCodonStart::ResetValue()
{
    if (m_GBqual)
        m_erase = true;
    else if (m_EditedFeat->IsSetData() && m_EditedFeat->GetData().IsCdregion())
        m_EditedFeat->SetData().SetCdregion().ResetFrame();
}

CEditingActionFeatTranslTable::CEditingActionFeatTranslTable(CSeq_entry_Handle seh, const CSeqFeatData::ESubtype subtype, const CSeqFeatData::E_Choice feat_type)
    : CEditingActionFeatGbQual(seh, subtype, feat_type, "transl_table", "CEditingActionFeatTranslTable"), m_code_table(CGen_code_table::GetCodeTable())

{
}

bool CEditingActionFeatTranslTable::IsSetValue()
{
    return (m_EditedFeat->IsSetData() && m_EditedFeat->GetData().IsCdregion() && m_EditedFeat->GetData().GetCdregion().IsSetCode())
        ||  (m_GBqual && m_GBqual->IsSetVal());
}

void CEditingActionFeatTranslTable::SetValue(const string &value)
{
    if (m_GBqual)
        m_GBqual->SetVal(value);
    else if (m_EditedFeat->IsSetData() && m_EditedFeat->GetData().IsCdregion())
    {
        const CGenetic_code_table::Tdata& codes = m_code_table.Get();        
        ITERATE (CGenetic_code_table::Tdata, it, codes) 
        {
            string str = (*it)->GetName();
            int id = (*it)->GetId();
            if (str == value)
            {
                m_EditedFeat->SetData().SetCdregion().SetCode().SetId(id);                
                return;
            }
        }
        int gc = NStr::StringToInt(value, NStr::fConvErr_NoThrow | NStr::fAllowLeadingSpaces | NStr::fAllowTrailingSpaces);
        m_EditedFeat->SetData().SetCdregion().SetCode().SetId(gc);
    }
    else
        m_EditedFeat->AddQualifier(m_qual, value);   
}

string CEditingActionFeatTranslTable::GetValue()
{
    if (m_GBqual)
    {
        return  m_GBqual->GetVal();
    }
    else if (m_EditedFeat->IsSetData() && m_EditedFeat->GetData().IsCdregion() && m_EditedFeat->GetData().GetCdregion().IsSetCode())
    {
        int gc = m_EditedFeat->GetData().GetCdregion().GetCode().GetId();
        const CGenetic_code_table::Tdata& codes = m_code_table.Get();
        
        ITERATE (CGenetic_code_table::Tdata, it, codes) 
        {
            string str = (*it)->GetName();
            int id = (*it)->GetId();
            if (id == gc)
            {
                return str;
            }
        }
        return NStr::IntToString(gc);
    } 
    
    return kEmptyStr;
}

void CEditingActionFeatTranslTable::ResetValue()
{
    if (m_GBqual)
        m_erase = true;
    else if (m_EditedFeat->IsSetData() && m_EditedFeat->GetData().IsCdregion())
        m_EditedFeat->SetData().SetCdregion().ResetCode();
}



CEditingActionFeatDesc::CEditingActionFeatDesc(CSeq_entry_Handle seh, const CSeqFeatData::ESubtype subtype, const CSeqFeatData::E_Choice feat_type)
    : CEditingActionFeatGbQual(seh, subtype, feat_type, "description", "CEditingActionFeatDesc")
{
}

bool CEditingActionFeatDesc::IsSetValue()
{
    return ( m_EditedFeat->IsSetData() && m_EditedFeat->GetData().IsGene() && m_EditedFeat->GetData().GetGene().IsSetDesc() )
        || ( m_EditedFeat->IsSetData() && m_EditedFeat->GetData().IsProt() && m_EditedFeat->GetData().GetProt().IsSetDesc() )
        ||  (m_GBqual && m_GBqual->IsSetVal());
}

void CEditingActionFeatDesc::SetValue(const string &value)
{
    if (m_GBqual)
        m_GBqual->SetVal(value);
    else if (m_EditedFeat->IsSetData() && m_EditedFeat->GetData().IsGene())
        m_EditedFeat->SetData().SetGene().SetDesc(value);
    else if (m_EditedFeat->IsSetData() && m_EditedFeat->GetData().IsProt())
        m_EditedFeat->SetData().SetProt().SetDesc(value);
    else
        m_EditedFeat->AddQualifier(m_qual, value);   
}

string CEditingActionFeatDesc::GetValue()
{
    if (m_GBqual)
    {
        return  m_GBqual->GetVal();
    }
    else if (m_EditedFeat->IsSetData() && m_EditedFeat->GetData().IsGene() && m_EditedFeat->GetData().GetGene().IsSetDesc())
        return m_EditedFeat->GetData().GetGene().GetDesc();
    else if ( m_EditedFeat->IsSetData() && m_EditedFeat->GetData().IsProt() && m_EditedFeat->GetData().GetProt().IsSetDesc() )
        return m_EditedFeat->GetData().GetProt().GetDesc();

    return kEmptyStr;
}

void CEditingActionFeatDesc::ResetValue()
{
    if (m_GBqual)
        m_erase = true;
    else if (m_EditedFeat->IsSetData() && m_EditedFeat->GetData().IsGene())
        m_EditedFeat->SetData().SetGene().ResetDesc();
    else if (m_EditedFeat->IsSetData() && m_EditedFeat->GetData().IsProt())
        m_EditedFeat->SetData().SetProt().ResetDesc();  
}


CEditingActionFeatNcRnaClass::CEditingActionFeatNcRnaClass(CSeq_entry_Handle seh, const CSeqFeatData::ESubtype subtype, const CSeqFeatData::E_Choice feat_type)
    : IEditingActionFeat(seh, subtype, feat_type, "CEditingActionFeatNcRnaClass")
{
}

bool CEditingActionFeatNcRnaClass::IsSetValue()
{
    return  m_EditedFeat->IsSetData() && m_EditedFeat->GetData().IsRna() //&& m_EditedFeat->GetData().GetRna().IsSetType() && m_EditedFeat->GetData().GetRna().GetType() == CRNA_ref::eType_ncRNA
        && m_EditedFeat->GetData().GetRna().IsSetExt() && m_EditedFeat->GetData().GetRna().GetExt().IsGen()
        && m_EditedFeat->GetData().GetRna().GetExt().GetGen().IsSetClass();
}

void CEditingActionFeatNcRnaClass::SetValue(const string &value)
{
    if (m_EditedFeat->IsSetData() && m_EditedFeat->GetData().IsRna() && m_EditedFeat->GetData().GetRna().IsSetType() && m_EditedFeat->GetData().GetRna().GetType() == CRNA_ref::eType_ncRNA)
        m_EditedFeat->SetData().SetRna().SetExt().SetGen().SetClass(value);
}

string CEditingActionFeatNcRnaClass::GetValue()
{
    if (m_EditedFeat->IsSetData() && m_EditedFeat->GetData().IsRna() && m_EditedFeat->GetData().GetRna().IsSetExt() && m_EditedFeat->GetData().GetRna().GetExt().IsGen() 
        && m_EditedFeat->GetData().GetRna().GetExt().GetGen().IsSetClass())
        return m_EditedFeat->GetData().GetRna().GetExt().GetGen().GetClass();
    return kEmptyStr;
}

void CEditingActionFeatNcRnaClass::ResetValue()
{
    if (m_EditedFeat->IsSetData() && m_EditedFeat->GetData().IsRna() && m_EditedFeat->GetData().GetRna().IsSetExt() && m_EditedFeat->GetData().GetRna().GetExt().IsGen())
        m_EditedFeat->SetData().SetRna().SetExt().SetGen().ResetClass();
}


CEditingActionFeatLocus_tag::CEditingActionFeatLocus_tag(CSeq_entry_Handle seh, const CSeqFeatData::ESubtype subtype, const CSeqFeatData::E_Choice feat_type)
    :  CEditingActionFeatGbQual(seh, subtype, feat_type, "locus_tag", "CEditingActionFeatLocus_tag")
{
}

bool CEditingActionFeatLocus_tag::IsSetValue()
{
    return (m_EditedFeat->IsSetData() && m_EditedFeat->GetData().IsGene() && m_EditedFeat->GetData().GetGene().IsSetLocus_tag()) ||
        (m_EditedFeat->GetGeneXref() != NULL &&  m_EditedFeat->GetGeneXref()->IsSetLocus_tag()) ||
        (m_GBqual && m_GBqual->IsSetVal());
}

void CEditingActionFeatLocus_tag::SetValue(const string &value)
{
    if (m_GBqual)
    {
        m_GBqual->SetVal(value);
    }
    else if (m_EditedFeat->IsSetData() && m_EditedFeat->GetData().IsGene())
    {
        m_EditedFeat->SetData().SetGene().SetLocus_tag(value);
    }
    else if (m_EditedFeat->GetGeneXref() != NULL)
    {
        m_EditedFeat->SetGeneXref().SetLocus_tag(value);
    }
    else
    {
        m_EditedFeat->AddQualifier(m_qual, value);   
    }
}

string CEditingActionFeatLocus_tag::GetValue()
{
    if (m_GBqual)
        return  m_GBqual->GetVal();
    else if (m_EditedFeat->IsSetData() && m_EditedFeat->GetData().IsGene() && m_EditedFeat->GetData().GetGene().IsSetLocus_tag())
        return m_EditedFeat->GetData().GetGene().GetLocus_tag();
    else if (m_EditedFeat->GetGeneXref() != NULL &&  m_EditedFeat->GetGeneXref()->IsSetLocus_tag())
        return m_EditedFeat->GetGeneXref()->GetLocus_tag();
    return kEmptyStr;
}

void CEditingActionFeatLocus_tag::ResetValue()
{
    if (m_GBqual)
        m_erase = true;
    else if (m_EditedFeat->IsSetData() && m_EditedFeat->GetData().IsGene())
        m_EditedFeat->SetData().SetGene().ResetLocus_tag();
    else if (m_EditedFeat->GetGeneXref() != NULL)
    {
        bool before = m_EditedFeat->SetGeneXref().IsSuppressed();
        m_EditedFeat->SetGeneXref().ResetLocus_tag();
        bool after = m_EditedFeat->SetGeneXref().IsSuppressed();
        if (!before && after)
            RemoveGeneXref();
    }
}

CEditingActionFeatMaploc::CEditingActionFeatMaploc(CSeq_entry_Handle seh, const CSeqFeatData::ESubtype subtype, const CSeqFeatData::E_Choice feat_type)
    : CEditingActionFeatGbQual(seh, subtype, feat_type, "maploc", "CEditingActionFeatMaploc")
{
}

bool CEditingActionFeatMaploc::IsSetValue()
{
    return (m_EditedFeat->IsSetData() && m_EditedFeat->GetData().IsGene() && m_EditedFeat->GetData().GetGene().IsSetMaploc()) ||
        (m_EditedFeat->GetGeneXref() != NULL &&  m_EditedFeat->GetGeneXref()->IsSetMaploc()) ||
        (m_GBqual && m_GBqual->IsSetVal());
}

void CEditingActionFeatMaploc::SetValue(const string &value)
{
    if (m_GBqual)
    {
        m_GBqual->SetVal(value);
    }
    else if (m_EditedFeat->IsSetData() && m_EditedFeat->GetData().IsGene())
    {
        m_EditedFeat->SetData().SetGene().SetMaploc(value);
    }
    else if (m_EditedFeat->GetGeneXref() != NULL)
    {
        m_EditedFeat->SetGeneXref().SetMaploc(value);
    }
    else
    {
        m_EditedFeat->AddQualifier(m_qual, value);
    }
}

string CEditingActionFeatMaploc::GetValue()
{
    if (m_GBqual)
        return  m_GBqual->GetVal();
    else if (m_EditedFeat->IsSetData() && m_EditedFeat->GetData().IsGene() && m_EditedFeat->GetData().GetGene().IsSetMaploc())
        return m_EditedFeat->GetData().GetGene().GetMaploc();
    else if (m_EditedFeat->GetGeneXref() != NULL &&  m_EditedFeat->GetGeneXref()->IsSetMaploc())
        return m_EditedFeat->GetGeneXref()->GetMaploc();
    return kEmptyStr;
}

void CEditingActionFeatMaploc::ResetValue()
{
    if (m_GBqual)
        m_erase = true;
    else if (m_EditedFeat->IsSetData() && m_EditedFeat->GetData().IsGene())
        m_EditedFeat->SetData().SetGene().ResetMaploc();
    else if (m_EditedFeat->GetGeneXref() != NULL)
    {
        bool before = m_EditedFeat->SetGeneXref().IsSuppressed();
        m_EditedFeat->SetGeneXref().ResetMaploc();
        bool after = m_EditedFeat->SetGeneXref().IsSuppressed();
        if (!before && after)
            RemoveGeneXref();
    }
}

CEditingActionFeatSynonym::CEditingActionFeatSynonym(CSeq_entry_Handle seh, const CSeqFeatData::ESubtype subtype, const CSeqFeatData::E_Choice feat_type)
    : IEditingActionFeat(seh, subtype, feat_type, "CEditingActionFeatSynonym"), m_erase(false), m_syn(NULL), m_qual("synonym")
{
}

void CEditingActionFeatSynonym::Modify(EActionType action)
{
    bool found = false;
    m_GBqual.Reset();
    if (!IsCreateNew(action))
    {
        if (m_EditedFeat->IsSetData() && m_EditedFeat->GetData().IsGene())
        {
            EDIT_EACH_SYNONYM_ON_GENEREF(syn_it, m_EditedFeat->SetData().SetGene())
            {
                found = true;
                m_erase = false;
                m_syn = &*syn_it;
                IEditingActionFeat::Modify(action);
                if (m_erase)
                    ERASE_SYNONYM_ON_GENEREF(syn_it, m_EditedFeat->SetData().SetGene());
            }
        }
        m_syn = NULL;
        if (!found && m_EditedFeat->GetGeneXref() != NULL)
        {
            bool before = m_EditedFeat->SetGeneXref().IsSuppressed();
            EDIT_EACH_SYNONYM_ON_GENEREF(syn_it, m_EditedFeat->SetGeneXref())
            {
                found = true;
                m_erase = false;
                m_syn = &*syn_it;
                IEditingActionFeat::Modify(action);
                if (m_erase)
                    ERASE_SYNONYM_ON_GENEREF(syn_it, m_EditedFeat->SetGeneXref());
            }
            bool after = m_EditedFeat->SetGeneXref().IsSuppressed();
            if (!before && after)
                RemoveGeneXref();
        }
        m_syn = NULL;
        if (!found)
        {
            if (IsFrom(action) && m_EditedFeat->IsSetQual() && !m_EditedFeat->GetQual().empty() && m_EditedFeat->GetQual().capacity() < 2 * m_EditedFeat->GetQual().size())
            {
                m_EditedFeat->SetQual().reserve(2 * m_EditedFeat->GetQual().size());
            }
            EDIT_EACH_GBQUAL_ON_SEQFEAT(gbqual_it, *m_EditedFeat)
            {
                if ((*gbqual_it)->IsSetQual() && (*gbqual_it)->GetQual() == m_qual)
                {
                    found = true;
                    m_erase = false;
                    m_GBqual = *gbqual_it;
                    IEditingActionFeat::Modify(action);
                    if (m_erase)
                        ERASE_GBQUAL_ON_SEQFEAT(gbqual_it, *m_EditedFeat);
                }
            }
            if (IsFrom(action))
            {
                m_EditedFeat->SetQual().shrink_to_fit();
            }
        }
    }

    if (!found)
    {
        m_syn = NULL;
        m_GBqual.Reset();
        m_erase = false;
        IEditingActionFeat::Modify(action);
    }
}

bool CEditingActionFeatSynonym::IsSetValue()
{
    return m_syn != NULL || (m_GBqual && m_GBqual->IsSetVal());
}

void CEditingActionFeatSynonym::SetValue(const string &value)
{
    if (m_GBqual)
    {
        m_GBqual->SetVal(value);
    }
    else if (m_syn)
    {
        *m_syn = value;
    }
    else if (m_EditedFeat->IsSetData() && m_EditedFeat->GetData().IsGene())
    {
        m_EditedFeat->SetData().SetGene().SetSyn().push_back(value);
    }
    else if (m_EditedFeat->GetGeneXref() != NULL)
    {
        m_EditedFeat->SetGeneXref().SetSyn().push_back(value);
    }
    else
    {
        m_EditedFeat->AddQualifier(m_qual, value);
    }
}

string CEditingActionFeatSynonym::GetValue()
{
    if (m_GBqual)
        return  m_GBqual->GetVal();
    else if (m_syn)
        return *m_syn;

    return kEmptyStr;
}

void CEditingActionFeatSynonym::ResetValue()
{
    m_erase = true;    
}

CEditingActionFeatAllele::CEditingActionFeatAllele(CSeq_entry_Handle seh, const CSeqFeatData::ESubtype subtype, const CSeqFeatData::E_Choice feat_type)
    : CEditingActionFeatGbQual(seh, subtype, feat_type, "allele", "CEditingActionFeatAllele")
{
}

bool CEditingActionFeatAllele::IsSetValue()
{
    return (m_EditedFeat->IsSetData() && m_EditedFeat->GetData().IsGene() && m_EditedFeat->GetData().GetGene().IsSetAllele()) ||
        (m_EditedFeat->GetGeneXref() != NULL &&  m_EditedFeat->GetGeneXref()->IsSetAllele()) ||
        (m_GBqual && m_GBqual->IsSetVal());
}

void CEditingActionFeatAllele::SetValue(const string &value)
{
    if (m_GBqual)
    {
        m_GBqual->SetVal(value);
    }
    else if (m_EditedFeat->IsSetData() && m_EditedFeat->GetData().IsGene())
    {
        m_EditedFeat->SetData().SetGene().SetAllele(value);
    }
    else if (m_EditedFeat->GetGeneXref() != NULL)
    {
        m_EditedFeat->SetGeneXref().SetAllele(value);
    }
    else
    {
        m_EditedFeat->AddQualifier(m_qual, value);
    }
}

string CEditingActionFeatAllele::GetValue()
{
    if (m_GBqual)
        return  m_GBqual->GetVal();
    else if (m_EditedFeat->IsSetData() && m_EditedFeat->GetData().IsGene() && m_EditedFeat->GetData().GetGene().IsSetAllele())
        return m_EditedFeat->GetData().GetGene().GetAllele();
    else if (m_EditedFeat->GetGeneXref() != NULL &&  m_EditedFeat->GetGeneXref()->IsSetAllele())
        return m_EditedFeat->GetGeneXref()->GetAllele();

    return kEmptyStr;
}

void CEditingActionFeatAllele::ResetValue()
{
    if (m_GBqual)
        m_erase = true;
    else if (m_EditedFeat->IsSetData() && m_EditedFeat->GetData().IsGene())
        m_EditedFeat->SetData().SetGene().ResetAllele();
    else if (m_EditedFeat->GetGeneXref() != NULL)
    {
        bool before = m_EditedFeat->SetGeneXref().IsSuppressed();
        m_EditedFeat->SetGeneXref().ResetAllele();
        bool after = m_EditedFeat->SetGeneXref().IsSuppressed();
        if (!before && after)
            RemoveGeneXref();
    }
}

CEditingActionFeatActivity::CEditingActionFeatActivity(CSeq_entry_Handle seh, const CSeqFeatData::ESubtype subtype, const CSeqFeatData::E_Choice feat_type)
    : IEditingActionFeat(seh, subtype, feat_type, "CEditingActionFeatActivity"), m_erase(false), m_activity(NULL), m_qual("activity")
{
}

void CEditingActionFeatActivity::Modify(EActionType action)
{
    bool found = false;
    GoToProtFeature();
    if (!IsCreateNew(action))
    {
        m_GBqual.Reset();
        if (m_EditedFeat->IsSetData() && m_EditedFeat->GetData().IsProt())
        {
            EDIT_EACH_ACTIVITY_ON_PROTREF(activity_it, m_EditedFeat->SetData().SetProt())
            {
                found = true;
                m_erase = false;
                m_activity = &*activity_it;
                IEditingActionFeat::Modify(action);
                if (m_erase)
                    ERASE_ACTIVITY_ON_PROTREF(activity_it, m_EditedFeat->SetData().SetProt());
            }
        }
        m_activity = NULL;
        if (!found)
        {
            if (IsFrom(action) && m_EditedFeat->IsSetQual() && !m_EditedFeat->GetQual().empty() && m_EditedFeat->GetQual().capacity() < 2 * m_EditedFeat->GetQual().size())
            {
                m_EditedFeat->SetQual().reserve(2 * m_EditedFeat->GetQual().size());
            }
            EDIT_EACH_GBQUAL_ON_SEQFEAT(gbqual_it, *m_EditedFeat)
            {
                if ((*gbqual_it)->IsSetQual() && (*gbqual_it)->GetQual() == m_qual)
                {
                    found = true;
                    m_erase = false;
                    m_GBqual = *gbqual_it;
                    IEditingActionFeat::Modify(action);
                    if (m_erase)
                        ERASE_GBQUAL_ON_SEQFEAT(gbqual_it, *m_EditedFeat);
                }
            }
            if (IsFrom(action))
            {
                m_EditedFeat->SetQual().shrink_to_fit();
            }
        }
    }

    if (!found)
    {
        m_activity = NULL;
        m_GBqual.Reset();
        m_erase = false;
        IEditingActionFeat::Modify(action);
    }
    GoToOrigFeature();
}

bool CEditingActionFeatActivity::IsSetValue()
{
    return m_activity != NULL ||  (m_GBqual && m_GBqual->IsSetVal());
}

void CEditingActionFeatActivity::SetValue(const string &value)
{
    if (m_GBqual)
    {
        m_GBqual->SetVal(value);
    }
    else if (m_activity)
    {
        *m_activity = value;
    }
    else
    {
        if (m_EditedFeat->IsSetData() && m_EditedFeat->GetData().IsProt())
        {
            m_EditedFeat->SetData().SetProt().SetActivity().push_back(value);
        }
        else
        {
            m_EditedFeat->AddQualifier(m_qual, value);
        }
    }
}

string CEditingActionFeatActivity::GetValue()
{
    if (m_GBqual)
        return  m_GBqual->GetVal();
    else if (m_activity)
        return *m_activity;
    return kEmptyStr;
}

void CEditingActionFeatActivity::ResetValue()
{
    m_erase = true;
}

CEditingActionFeatPartial::CEditingActionFeatPartial(CSeq_entry_Handle seh, const CSeqFeatData::ESubtype subtype, const CSeqFeatData::E_Choice feat_type)
    : IEditingActionFeat(seh, subtype, feat_type, "CEditingActionFeatPartial")
{
}

bool CEditingActionFeatPartial::IsSetValue()
{
    return m_EditedFeat->IsSetPartial();
}

void CEditingActionFeatPartial::SetValue(const string &value)
{
    if (NStr::EqualNocase(value,"true"))
        m_EditedFeat->SetPartial(true);
    if (NStr::EqualNocase(value,"false"))
        m_EditedFeat->SetPartial(false);    
}

string CEditingActionFeatPartial::GetValue()
{
    if (m_EditedFeat->GetPartial())
        return "true";
    return "false";
}

void CEditingActionFeatPartial::ResetValue()
{
    m_EditedFeat->ResetPartial();
}

CEditingActionFeatTranslation::CEditingActionFeatTranslation(CSeq_entry_Handle seh, const CSeqFeatData::ESubtype subtype, const CSeqFeatData::E_Choice feat_type)
    : CEditingActionFeatGbQual(seh, subtype, feat_type, "translation", "CEditingActionFeatTranslation")
{
}

bool CEditingActionFeatTranslation::IsSetValue()
{
    if (m_EditedFeat->IsSetProduct()) 
    {
        const CSeq_id *id = m_EditedFeat->GetProduct().GetId();
        if (id && m_Feat) 
        {
            CBioseq_Handle bsh = m_Feat.GetScope().GetBioseqHandle(*id);
            if (bsh) 
            {
                return true;
            }
        }
    }

    return (m_GBqual && m_GBqual->IsSetVal());
}

void CEditingActionFeatTranslation::SetValue(const string &value)
{
    if (m_GBqual)
    {
        m_GBqual->SetVal(value);
    }
    else if (m_EditedFeat->IsSetProduct()) 
    {
        // do nothing
    }
    else
    {
        m_EditedFeat->AddQualifier(m_qual, value);
    }
}

string CEditingActionFeatTranslation::GetValue()
{
    if (m_GBqual)
        return  m_GBqual->GetVal();
    else if (m_EditedFeat->IsSetProduct()) 
    {
        const CSeq_id *id = m_EditedFeat->GetProduct().GetId();
        if (id && m_Feat) 
        {
            CBioseq_Handle bsh = m_Feat.GetScope().GetBioseqHandle(*id);
            if (bsh) 
            {
                string prot_seq;
                CSeqVector prot_vec(*(bsh.GetCompleteBioseq()), &m_Feat.GetScope());
                prot_vec.SetCoding(CSeq_data::e_Ncbieaa);
                prot_vec.GetSeqData(0, prot_vec.size(), prot_seq);
                return prot_seq;
            }
        }
    }

    return kEmptyStr;
}

void CEditingActionFeatTranslation::ResetValue()
{
    if (m_GBqual)
        m_erase = true;    
}


CEditingActionFeatEcNumber::CEditingActionFeatEcNumber(CSeq_entry_Handle seh, const CSeqFeatData::ESubtype subtype, const CSeqFeatData::E_Choice feat_type)
    : IEditingActionFeat(seh, subtype, feat_type, "CEditingActionFeatEcNumber"), m_erase(false), m_ec(NULL), m_qual("EC_number")
{
    ResetScope();
}

void CEditingActionFeatEcNumber::Modify(EActionType action)
{
    bool found = false;
    GoToProtFeature();
    if (!IsCreateNew(action))
    {
        m_GBqual.Reset();
        if (m_EditedFeat->IsSetData() && m_EditedFeat->GetData().IsProt())
        {
            EDIT_EACH_ECNUMBER_ON_PROTREF(ec_it, m_EditedFeat->SetData().SetProt())
            {
                found = true;
                m_erase = false;
                m_ec = &*ec_it;
                IEditingActionFeat::Modify(action);
                if (m_erase)
                    ERASE_ECNUMBER_ON_PROTREF(ec_it, m_EditedFeat->SetData().SetProt());
            }
        }   
        m_ec = NULL;
        if (!found && m_EditedFeat->GetProtXref() != NULL)
        {
            CProt_ref& prot_feat = m_EditedFeat->SetProtXref();
            EDIT_EACH_ECNUMBER_ON_PROTREF(ec_it, prot_feat)
            {
                found = true;
                m_erase = false;
                m_ec = &*ec_it;
                IEditingActionFeat::Modify(action);
                if (m_erase)
                    ERASE_ECNUMBER_ON_PROTREF(ec_it, prot_feat);
            }
        }
        m_ec = NULL;
        if (!found)
        {
            if (IsFrom(action) && m_EditedFeat->IsSetQual() && !m_EditedFeat->GetQual().empty() && m_EditedFeat->GetQual().capacity() < 2 * m_EditedFeat->GetQual().size())
            {
                m_EditedFeat->SetQual().reserve(2 * m_EditedFeat->GetQual().size());
            }
            EDIT_EACH_GBQUAL_ON_SEQFEAT(gbqual_it, *m_EditedFeat)
            {
                if ((*gbqual_it)->IsSetQual() && (*gbqual_it)->GetQual() == m_qual)
                {
                    found = true;
                    m_erase = false;
                    m_GBqual = *gbqual_it;
                    IEditingActionFeat::Modify(action);
                    if (m_erase)
                        ERASE_GBQUAL_ON_SEQFEAT(gbqual_it, *m_EditedFeat);
                }
            }
            if (IsFrom(action))
            {
                m_EditedFeat->SetQual().shrink_to_fit();
            }
        }
    }

    if (!found)
    {
        m_ec = NULL;
        m_GBqual.Reset();
        m_erase = false;
        IEditingActionFeat::Modify(action);
    }
    GoToOrigFeature();
}

bool CEditingActionFeatEcNumber::IsSetValue()
{
    return m_ec != NULL ||  (m_GBqual && m_GBqual->IsSetVal());
}

void CEditingActionFeatEcNumber::SetValue(const string &value)
{
    if (m_GBqual)
    {
        m_GBqual->SetVal(value);
    }
    else if (m_ec)
    {
        *m_ec = value;
    }
    else if (m_EditedFeat->IsSetData() && m_EditedFeat->GetData().IsProt())
    {
        m_EditedFeat->SetData().SetProt().SetEc().push_back(value);
    }   
    else if (m_EditedFeat->GetProtXref() != NULL)
    {
        m_EditedFeat->SetProtXref().SetEc().push_back(value);
    }
    else
    {
        m_EditedFeat->AddQualifier(m_qual, value);
    }
}

string CEditingActionFeatEcNumber::GetValue()
{
    if (m_GBqual)
        return  m_GBqual->GetVal();
    else if (m_ec)
        return *m_ec;
    return kEmptyStr;
}

void CEditingActionFeatEcNumber::ResetValue()
{
    m_erase = true;
}

void CEditingActionFeatEcNumber::Find(EActionType action)
{
    FindOrCreateProtFeat(action);
}

void CEditingActionFeatEcNumber::FindRelated(EActionType action)
{
    FindRelatedOrCreateProtFeat(action);
}

CEditingActionFeatProduct::CEditingActionFeatProduct(CSeq_entry_Handle seh, const CSeqFeatData::ESubtype subtype, const CSeqFeatData::E_Choice feat_type)
    : IEditingActionFeat(seh, subtype, feat_type, "CEditingActionFeatProduct"), m_erase(false), m_name(NULL), m_qual("product")
{
    ResetScope();
}

void CEditingActionFeatProduct::Modify(EActionType action)
{
    bool found = false;
    GoToProtFeature();
    if (!IsCreateNew(action))
    {
        m_GBqual.Reset();
        if (m_EditedFeat->IsSetData() && m_EditedFeat->GetData().IsProt())
        {
            EDIT_EACH_NAME_ON_PROTREF(name_it, m_EditedFeat->SetData().SetProt())
            {
                found = true;
                m_erase = false;
                m_name = &*name_it;
                IEditingActionFeat::Modify(action);
                if (m_erase)
                    ERASE_NAME_ON_PROTREF(name_it, m_EditedFeat->SetData().SetProt());
            }
        }   
        m_name = NULL;
        if (!found && m_EditedFeat->GetProtXref() != NULL)
        {
            CProt_ref& prot_feat = m_EditedFeat->SetProtXref();
            EDIT_EACH_NAME_ON_PROTREF(name_it, prot_feat)
            {
                found = true;
                m_erase = false;
                m_name = &*name_it;
                IEditingActionFeat::Modify(action);
                if (m_erase)
                    ERASE_NAME_ON_PROTREF(name_it, prot_feat);
            }
        }
        m_name = NULL;
        if (!found && m_EditedFeat->IsSetData() && m_EditedFeat->GetData().IsRna())
        {
            found = true;
            IEditingActionFeat::Modify(action);
        }
        m_name = NULL;
        if (!found)
        {
            if (IsFrom(action) && m_EditedFeat->IsSetQual() && !m_EditedFeat->GetQual().empty() && m_EditedFeat->GetQual().capacity() < 2 * m_EditedFeat->GetQual().size())
            {
                m_EditedFeat->SetQual().reserve(2 * m_EditedFeat->GetQual().size());
            }
            EDIT_EACH_GBQUAL_ON_SEQFEAT(gbqual_it, *m_EditedFeat)
            {
                if ((*gbqual_it)->IsSetQual() && (*gbqual_it)->GetQual() == m_qual)
                {
                    found = true;
                    m_erase = false;
                    m_GBqual = *gbqual_it;
                    IEditingActionFeat::Modify(action);
                    if (m_erase)
                        ERASE_GBQUAL_ON_SEQFEAT(gbqual_it, *m_EditedFeat);
                }
            }
            if (IsFrom(action))
            {
                m_EditedFeat->SetQual().shrink_to_fit();
            }
        }
    }

    if (!found)
    {
        m_name = NULL;
        m_GBqual.Reset();
        m_erase = false;
        IEditingActionFeat::Modify(action);
    }
    GoToOrigFeature();
}

bool CEditingActionFeatProduct::IsSetValue()
{
    return m_name != NULL ||  (m_GBqual && m_GBqual->IsSetVal()) ||
        (m_EditedFeat->IsSetData() && m_EditedFeat->GetData().IsRna() && !m_EditedFeat->GetData().GetRna().GetRnaProductName().empty());
}

void CEditingActionFeatProduct::SetValue(const string &value)
{
    if (m_GBqual)
    {
        m_GBqual->SetVal(value);
    }
    else if (m_name)
    {
        *m_name = value;
    }
    else if (m_EditedFeat->IsSetData() && m_EditedFeat->GetData().IsRna())
    {
        string val = value;
        if (val == "fM" || val == "iM")
            val += "et";
        if (val ==  "fMet" || val == "iMet")
            val = "tRNA-" + val;
        if ((val == "tRNA-fMet" || val == "tRNA-iMet") && m_EditedFeat->GetData().GetRna().IsSetType() && m_EditedFeat->GetData().GetRna().GetType() == CRNA_ref::eType_tRNA)
        {
            m_EditedFeat->AddQualifier(m_qual, val);
        }
        else
        {
            string remainder;
            m_EditedFeat->SetData().SetRna().SetRnaProductName(value, remainder);
            if (!NStr::IsBlank(remainder)) 
            {
                if (m_EditedFeat->IsSetComment() && !NStr::IsBlank(m_EditedFeat->GetComment())) 
                {
                    m_EditedFeat->SetComment(m_EditedFeat->GetComment() + "; " + remainder);
                } 
                else 
                {
                    m_EditedFeat->SetComment(remainder);
                }
            }
        }
    }
    else if (m_EditedFeat->IsSetData() && m_EditedFeat->GetData().IsProt())
    {
        m_EditedFeat->SetData().SetProt().SetName().push_back(value);
    }   
    else if (m_EditedFeat->GetProtXref() != NULL)
    {
        m_EditedFeat->SetProtXref().SetName().push_back(value);
    }
    else
    {
        m_EditedFeat->AddQualifier(m_qual, value);
    }
}

string CEditingActionFeatProduct::GetValue()
{
    if (m_GBqual)
        return  m_GBqual->GetVal();
    else if (m_name)
        return *m_name;
    else if (m_EditedFeat->IsSetData() && m_EditedFeat->GetData().IsRna() && !m_EditedFeat->GetData().GetRna().GetRnaProductName().empty())
    {
        return m_EditedFeat->GetData().GetRna().GetRnaProductName();
    } 
    return kEmptyStr;
}

void CEditingActionFeatProduct::ResetValue()
{
    m_erase = true;
    if (m_EditedFeat->IsSetData() && m_EditedFeat->GetData().IsRna())
    {
        string remainder;
        m_EditedFeat->SetData().SetRna().SetRnaProductName(kEmptyStr, remainder);
    }
}

void CEditingActionFeatProduct::Find(EActionType action)
{
    FindOrCreateProtFeat(action);   
}

void CEditingActionFeatProduct::FindRelated(EActionType action)
{
    FindRelatedOrCreateProtFeat(action);
}


// data.gene.locus OR xref.data.gene.locus OR qual.val where qual.qual = "locus"
CEditingActionFeatGeneLocus::CEditingActionFeatGeneLocus(CSeq_entry_Handle seh, const CSeqFeatData::ESubtype subtype, const CSeqFeatData::E_Choice feat_type, const string &name)
    : CEditingActionFeatGbQual(seh, subtype, feat_type, "locus", name)
{
    ResetScope();
}

bool CEditingActionFeatGeneLocus::IsSetValue()
{
   bool is_gene = m_EditedFeat->IsSetData() && m_EditedFeat->GetData().IsGene() && m_EditedFeat->GetData().GetGene().IsSetLocus();
   bool is_xref = (m_EditedFeat->GetGeneXref() != NULL) && m_EditedFeat->GetGeneXref()->IsSetLocus();
   bool is_qual = m_GBqual && m_GBqual->IsSetVal();
    return is_gene || is_xref || is_qual;
}

void CEditingActionFeatGeneLocus::SetValue(const string &value)
{
    if (m_GBqual)
    {
        m_GBqual->SetVal(value);
    }
    else if (m_EditedFeat->IsSetData() && m_EditedFeat->GetData().IsGene())
    {
        m_EditedFeat->SetData().SetGene().SetLocus(value);
    }
    else if (m_EditedFeat->GetGeneXref() != NULL)
    {
        m_EditedFeat->SetGeneXref().SetLocus(value);
        return;
    }
    else
    {
        m_EditedFeat->AddQualifier(m_qual, value);
    }
}

string CEditingActionFeatGeneLocus::GetValue()
{
    if (m_GBqual)
        return  m_GBqual->GetVal();
    else if (m_EditedFeat->IsSetData() && m_EditedFeat->GetData().IsGene())
        return m_EditedFeat->GetData().GetGene().GetLocus();
    else if (m_EditedFeat->GetGeneXref() != NULL && m_EditedFeat->GetGeneXref()->IsSetLocus())
        return m_EditedFeat->GetGeneXref()->GetLocus();

    return kEmptyStr;
}

void CEditingActionFeatGeneLocus::ResetValue()
{
    if (m_GBqual)
        m_erase = true;
    else if (m_EditedFeat->IsSetData() && m_EditedFeat->GetData().IsGene())
        m_EditedFeat->SetData().SetGene().ResetLocus();
    else if (m_EditedFeat->GetGeneXref() != NULL)
    {
        bool before = m_EditedFeat->SetGeneXref().IsSuppressed();
        m_EditedFeat->SetGeneXref().ResetLocus();
        bool after = m_EditedFeat->SetGeneXref().IsSuppressed();
        if (!before && after)
            RemoveGeneXref();
    }
}

void CEditingActionFeatGeneLocus::FindRelated(EActionType action)
{
    IEditingActionFeat *feat_other = dynamic_cast<IEditingActionFeat*>(m_Other);
    if (!feat_other)
    {
        Find(action); 
        return;
    }
    m_EditedFeat.Reset();
    CSeq_feat_Handle fh = feat_other->GetFeatHandle();
    CScope &scope = fh.GetScope();
    CRef<CSeq_loc> feat_loc;
    if ( fh.GetFeatType() == CSeqFeatData::e_Prot )
    {
        const CSeq_loc& prot_loc = fh.GetLocation();
        CBioseq_Handle prot_bsh = scope.GetBioseqHandle(prot_loc);
        if (prot_bsh) 
        {
            const CSeq_feat* cds = sequence::GetCDSForProduct(prot_bsh);
            if (cds)
            {
                feat_loc.Reset(new CSeq_loc);
                feat_loc->Assign(cds->GetLocation());
            }
        }
    }
    else
    {
        feat_loc.Reset(new CSeq_loc);
        feat_loc->Assign(fh.GetLocation());
    }

    if (!feat_loc)
        return;
    CSeq_feat_Handle gene_fh = x_FindGeneForFeature(*feat_loc, scope, CSeqFeatData::eSubtype_gene);  

    if (gene_fh)
    {
        m_Feat = gene_fh;
        if (m_ChangedFeatures.find(m_Feat) != m_ChangedFeatures.end())
            m_EditedFeat = m_ChangedFeatures[m_Feat];
        else
        {
            m_EditedFeat.Reset(new CSeq_feat);
            m_EditedFeat->Assign(*m_Feat.GetOriginalSeq_feat());
        }
    }
    else
    {
        CSeq_feat_Handle gene_fh = x_FindGeneForFeature(*feat_loc, *m_scope, CSeqFeatData::eSubtype_gene);  
        if (gene_fh)
        {
            m_Feat = gene_fh;
            if (m_ChangedFeatures.find(m_Feat) != m_ChangedFeatures.end())
                m_EditedFeat = m_ChangedFeatures[m_Feat];
            else
            {
                m_EditedFeat.Reset(new CSeq_feat);
                m_EditedFeat->Assign(*m_Feat.GetOriginalSeq_feat());
            }
        }
        else
        {
            CRef<CSeq_loc> gene_loc = x_MergeFeatureLocation(*feat_loc, scope); 
            m_EditedFeat.Reset(new CSeq_feat);
            m_EditedFeat->SetData().SetGene();
            m_EditedFeat->SetLocation(*gene_loc);
            m_EditedFeat->SetPartial(m_EditedFeat->GetLocation().IsPartialStart(eExtreme_Biological) || m_EditedFeat->GetLocation().IsPartialStop(eExtreme_Biological));
            CSeq_entry_Handle seh = GetCurrentSeqEntryHandleFromFeatHandle(fh);
            if (seh.IsSeq() && seh.GetSeq().IsAa() && seh.HasParentEntry())
            {
                CBioseq_CI b_iter(seh.GetParentEntry(), CSeq_inst::eMol_na);
                if (b_iter)
                    seh = b_iter->GetSeq_entry_Handle();
            }
            CConstRef<CSeq_entry> entry = seh.GetSeq_entryCore();
            CSeq_entry_Handle new_seh = m_scope->AddTopLevelSeqEntry(*entry, CScope::kPriority_Default, CScope::eExist_Get);
            CSeq_annot_Handle ftable;
            for (CSeq_annot_CI annot_ci(new_seh, CSeq_annot_CI::eSearch_entry); annot_ci; ++annot_ci) 
            {
                if ((*annot_ci).IsFtable()) 
                {
                    ftable = *annot_ci;
                    break;
                }
            }
            CSeq_entry_EditHandle eh = new_seh.GetEditHandle();
            if (!ftable) 
            {
                CRef<CSeq_annot> new_annot(new CSeq_annot());
                ftable = eh.AttachAnnot(*new_annot);
            }

            CSeq_annot_EditHandle aeh(ftable);
            m_Feat = aeh.AddFeat(*m_EditedFeat);
            m_CreatedFeatures[m_Feat] = seh; // this seq-entry-handle should be in the original scope
        }
    }
    Modify(action);
}

void CEditingActionFeatGeneLocusPlain::Find(EActionType action)
{
    if (!m_TopSeqEntry)
        return;
    size_t count = 0;
    for (CFeat_CI feat_ci(m_TopSeqEntry, m_selector); feat_ci; ++feat_ci)
    {
        m_EditedFeat.Reset();
        CSeq_feat_Handle fh = feat_ci->GetSeq_feat_Handle();
        m_CurrentSeqEntry = GetCurrentSeqEntryHandleFromFeatHandle(fh);
        CScope &scope = fh.GetScope();
        if (MatchBioseqConstraintForFeatHandle(fh) && m_constraint->Match(fh))
        {
            m_Feat = fh;
            if (m_ChangedFeatures.find(m_Feat) != m_ChangedFeatures.end())
                m_EditedFeat = m_ChangedFeatures[m_Feat];
            else
            {
                m_EditedFeat.Reset(new CSeq_feat);
                m_EditedFeat->Assign(*m_Feat.GetOriginalSeq_feat());
            }
            Modify(action);
            if (!IsNOOP(action) && !m_EditedFeat)
            {
                if (m_ChangedFeatures.find(m_Feat) != m_ChangedFeatures.end())
                    m_EditedFeat = m_ChangedFeatures[m_Feat];
                else 
                {
                    CRef<CSeq_loc> gene_loc = x_MergeFeatureLocation(m_Feat.GetLocation(), scope); 
                    m_EditedFeat.Reset(new CSeq_feat);
                    m_EditedFeat->SetData().SetGene();
                    m_EditedFeat->SetLocation(*gene_loc);
                    m_EditedFeat->SetPartial(m_EditedFeat->GetLocation().IsPartialStart(eExtreme_Biological) || m_EditedFeat->GetLocation().IsPartialStop(eExtreme_Biological));
                    CSeq_entry_Handle seh = GetCurrentSeqEntryHandleFromFeatHandle(fh);
                    m_CreatedFeatures[m_Feat] = seh;
                }
                if (m_EditedFeat)
                    Modify(action);
            }           
        }
        ++count;
        if (count >= m_max_records)
            break;        
    }  
}

void CEditingActionFeatGeneLocusRna::Find(EActionType action)
{
    if (!m_TopSeqEntry)
        return;
    size_t count = 0;
    vector<CRef<CSeq_loc>> already_created;
    for (CFeat_CI feat_ci(m_TopSeqEntry, m_selector); feat_ci; ++feat_ci)
    {
        m_EditedFeat.Reset();
        CSeq_feat_Handle fh = feat_ci->GetSeq_feat_Handle();      
        m_CurrentSeqEntry = GetCurrentSeqEntryHandleFromFeatHandle(fh);
        CScope &scope = fh.GetScope();
        if (MatchBioseqConstraintForFeatHandle(fh) && m_constraint->Match(fh))
        {

            if (fh.GetFeatSubtype() != CSeqFeatData::eSubtype_gene)
            {             
                CSeq_feat_Handle gene_fh = x_FindGeneForFeature(fh.GetLocation(), scope, CSeqFeatData::eSubtype_gene);                  
                if (gene_fh)
                    m_Feat = gene_fh;
            }
            else
                m_Feat = fh;
            if (m_Feat)
            {                
                if (m_ChangedFeatures.find(m_Feat) != m_ChangedFeatures.end())
                    m_EditedFeat = m_ChangedFeatures[m_Feat];
                else
                {
                    m_EditedFeat.Reset(new CSeq_feat);
                    m_EditedFeat->Assign(*m_Feat.GetOriginalSeq_feat());
                }
            }
            else if (!IsNOOP(action))
            {
                m_Feat = fh;
                if (m_ChangedFeatures.find(m_Feat) != m_ChangedFeatures.end())
                    m_EditedFeat = m_ChangedFeatures[m_Feat];
                else 
                {
                    CRef<CSeq_loc> gene_loc = x_MergeFeatureLocation(m_Feat.GetLocation(), scope); 
                    auto created = find_if(already_created.begin(), already_created.end(),[gene_loc](CRef<CSeq_loc> loc) {return loc->Equals(*gene_loc);});
                    if (created == already_created.end())
                    {
                        already_created.push_back(gene_loc);
                        m_EditedFeat.Reset(new CSeq_feat);
                        m_EditedFeat->SetData().SetGene();
                        m_EditedFeat->SetLocation(*gene_loc);
                        m_EditedFeat->SetPartial(m_EditedFeat->GetLocation().IsPartialStart(eExtreme_Biological) || m_EditedFeat->GetLocation().IsPartialStop(eExtreme_Biological));
                        CSeq_entry_Handle seh = GetCurrentSeqEntryHandleFromFeatHandle(fh);
                        m_CreatedFeatures[m_Feat] = seh;
                    }
                }
            }
            if (m_EditedFeat)
                Modify(action);

        }
        ++count;
        if (count >= m_max_records)
            break;       
    }   
}

void CEditingActionFeatGeneLocusRna::SetFeat(CSeq_feat_Handle fh)
{
    SetFeatForAnotherFeat(fh, CSeqFeatData::eSubtype_gene);
}

bool CEditingActionFeatGeneLocusRna::SameObject()
{
    return OtherFeatIs(CSeqFeatData::eSubtype_gene);
}


void CEditingActionFeatGeneLocusCdsGeneProt::Find(EActionType action)
{
    if (!m_TopSeqEntry)
        return;
    CScope &scope = m_TopSeqEntry.GetScope();
    size_t count = 0;
    for (CFeat_CI feat_ci(m_TopSeqEntry, SAnnotSelector(CSeqFeatData::eSubtype_gene)); feat_ci; ++feat_ci)
    {
        CSeq_feat_Handle fh = feat_ci->GetSeq_feat_Handle();
        m_EditedFeat.Reset();
        m_Feat.Reset();
        m_CurrentSeqEntry = GetCurrentSeqEntryHandleFromFeatHandle(fh);        
        if (MatchBioseqConstraintForFeatHandle(fh) && m_constraint->Match(fh))
        {           
            m_Feat = fh;
            if (m_ChangedFeatures.find(m_Feat) != m_ChangedFeatures.end())
                m_EditedFeat = m_ChangedFeatures[m_Feat];
            else
            {
                m_EditedFeat.Reset(new CSeq_feat);
                m_EditedFeat->Assign(*m_Feat.GetOriginalSeq_feat());
            }

           
            if (m_EditedFeat)
                Modify(action);
        }
        ++count;
        if (count >= m_max_records)
            break;        
    }  

    if (IsNOOP(action))
        return;

    vector<CSeq_feat_Handle> feat_handles;

    for (CFeat_CI feat_ci(m_TopSeqEntry, SAnnotSelector(CSeqFeatData::eSubtype_mRNA)); feat_ci; ++feat_ci)
    {
        CSeq_feat_Handle fh = feat_ci->GetSeq_feat_Handle();
        CSeq_feat_Handle gene_fh = x_FindGeneForFeature(fh.GetLocation(), scope, CSeqFeatData::eSubtype_gene);       
        if (!gene_fh || !m_constraint->Match(gene_fh))
        {
            feat_handles.push_back(fh);
        }       
    }
    
    for (CFeat_CI feat_ci(m_TopSeqEntry, SAnnotSelector(CSeqFeatData::eSubtype_cdregion)); feat_ci; ++feat_ci)
    {
        CSeq_feat_Handle fh = feat_ci->GetSeq_feat_Handle();
        CSeq_feat_Handle gene_fh = x_FindGeneForFeature(fh.GetLocation(), scope, CSeqFeatData::eSubtype_gene);  
        CSeq_feat_Handle mrna_fh = x_FindGeneForFeature(fh.GetLocation(), scope, CSeqFeatData::eSubtype_mRNA);        
        if ((!gene_fh || !m_constraint->Match(gene_fh))
	    && (!mrna_fh || !m_constraint->Match(mrna_fh)))
        {           
            feat_handles.push_back(fh);
        }       
    }

    vector<CRef<CSeq_loc>> already_created;
    for (size_t i = 0; i < feat_handles.size(); i++) 
    {
        CSeq_feat_Handle fh = feat_handles[i];
        m_EditedFeat.Reset();
        m_Feat.Reset();
        m_CurrentSeqEntry = GetCurrentSeqEntryHandleFromFeatHandle(fh);        
        if (MatchBioseqConstraintForFeatHandle(fh) && m_constraint->Match(fh))
        {                 
            m_Feat = fh;
            
            if (m_ChangedFeatures.find(m_Feat) != m_ChangedFeatures.end())
                m_EditedFeat = m_ChangedFeatures[m_Feat];
            else 
            {
                CRef<CSeq_loc> gene_loc = x_MergeFeatureLocation(m_Feat.GetLocation(), scope); 
                auto created = find_if(already_created.begin(), already_created.end(),[gene_loc](CRef<CSeq_loc> loc) {return loc->Equals(*gene_loc);});
                if (created == already_created.end())
                {
                    already_created.push_back(gene_loc);
                    m_EditedFeat.Reset(new CSeq_feat);
                    m_EditedFeat->SetData().SetGene();
                    m_EditedFeat->SetLocation(*gene_loc);
                    m_EditedFeat->SetPartial(m_EditedFeat->GetLocation().IsPartialStart(eExtreme_Biological) || m_EditedFeat->GetLocation().IsPartialStop(eExtreme_Biological));
                    CSeq_entry_Handle seh = GetCurrentSeqEntryHandleFromFeatHandle(fh);
                    m_CreatedFeatures[m_Feat] = seh;
                }
            }
            
            if (m_EditedFeat)
                Modify(action);
        }
    }
}

void CEditingActionFeatGeneLocusCdsGeneProt::SetFeat(CSeq_feat_Handle fh)
{
    SetFeatForAnotherFeat(fh, CSeqFeatData::eSubtype_gene);
}

CEditingActionFeatRnaToGeneDesc::CEditingActionFeatRnaToGeneDesc(CSeq_entry_Handle seh, const CSeqFeatData::ESubtype subtype)
        :  CEditingActionFeatDesc(seh, subtype, CSeqFeatData::e_Rna) 
{
    m_Name = "CEditingActionFeatRnaToGeneDesc";
}

bool CEditingActionFeatRnaToGeneDesc::SameObject()
{
    return OtherFeatIs(CSeqFeatData::eSubtype_gene);
}


void CEditingActionFeatRnaToGeneDesc::Find(EActionType action)
{
    FindFeatForAnotherFeat(action, SAnnotSelector(CSeqFeatData::eSubtype_gene));
}
 

void CEditingActionFeatRnaToGeneDesc::FindRelated(EActionType action)
{
    FindRelatedFeatForAnotherFeat(action, SAnnotSelector(CSeqFeatData::eSubtype_gene));
}

void CEditingActionFeatRnaToGeneDesc::SetFeat(CSeq_feat_Handle fh)
{
    SetFeatForAnotherFeat(fh, CSeqFeatData::eSubtype_gene);
}


CEditingActionFeatRnaToGeneMaploc::CEditingActionFeatRnaToGeneMaploc(CSeq_entry_Handle seh, const CSeqFeatData::ESubtype subtype)
    : CEditingActionFeatMaploc(seh, subtype, CSeqFeatData::e_Rna) 
{
    m_Name = "CEditingActionFeatRnaToGeneMaploc";
}

bool CEditingActionFeatRnaToGeneMaploc::SameObject()
{
    return OtherFeatIs(CSeqFeatData::eSubtype_gene);
}

void CEditingActionFeatRnaToGeneMaploc::Find(EActionType action)
{
    FindFeatForAnotherFeat(action, SAnnotSelector(CSeqFeatData::eSubtype_gene));
}

void CEditingActionFeatRnaToGeneMaploc::FindRelated(EActionType action)
{
    FindRelatedFeatForAnotherFeat(action, SAnnotSelector(CSeqFeatData::eSubtype_gene));
}

void CEditingActionFeatRnaToGeneMaploc::SetFeat(CSeq_feat_Handle fh)
{
    SetFeatForAnotherFeat(fh, CSeqFeatData::eSubtype_gene);
}

CEditingActionFeatRnaToGeneLocus_tag::CEditingActionFeatRnaToGeneLocus_tag(CSeq_entry_Handle seh, const CSeqFeatData::ESubtype subtype)
    : CEditingActionFeatLocus_tag(seh, subtype, CSeqFeatData::e_Rna) 
{
    m_Name = "CEditingActionFeatRnaToGeneLocus_tag";
}

bool CEditingActionFeatRnaToGeneLocus_tag::SameObject()
{
    return OtherFeatIs(CSeqFeatData::eSubtype_gene);
}

void CEditingActionFeatRnaToGeneLocus_tag::Find(EActionType action)
{
    FindFeatForAnotherFeat(action, SAnnotSelector(CSeqFeatData::eSubtype_gene));
}

void CEditingActionFeatRnaToGeneLocus_tag::FindRelated(EActionType action)
{
    FindRelatedFeatForAnotherFeat(action, SAnnotSelector(CSeqFeatData::eSubtype_gene));
}

void CEditingActionFeatRnaToGeneLocus_tag::SetFeat(CSeq_feat_Handle fh)
{
    SetFeatForAnotherFeat(fh, CSeqFeatData::eSubtype_gene);
}

CEditingActionFeatRnaToGeneSynonym::CEditingActionFeatRnaToGeneSynonym(CSeq_entry_Handle seh, const CSeqFeatData::ESubtype subtype)
    : CEditingActionFeatSynonym(seh, subtype, CSeqFeatData::e_Rna) 
{
    m_Name = "CEditingActionFeatRnaToGeneSynonym";
}

bool CEditingActionFeatRnaToGeneSynonym::SameObject()
{
    return OtherFeatIs(CSeqFeatData::eSubtype_gene);
}

void CEditingActionFeatRnaToGeneSynonym::Find(EActionType action)
{
    FindFeatForAnotherFeat(action, SAnnotSelector(CSeqFeatData::eSubtype_gene));
}

void CEditingActionFeatRnaToGeneSynonym::FindRelated(EActionType action)
{
    FindRelatedFeatForAnotherFeat(action, SAnnotSelector(CSeqFeatData::eSubtype_gene));
}

void CEditingActionFeatRnaToGeneSynonym::SetFeat(CSeq_feat_Handle fh)
{
    SetFeatForAnotherFeat(fh, CSeqFeatData::eSubtype_gene);
}

CEditingActionFeatRnaToGeneComment::CEditingActionFeatRnaToGeneComment(CSeq_entry_Handle seh, const CSeqFeatData::ESubtype subtype)
    : CEditingActionFeatComment(seh, subtype, CSeqFeatData::e_Rna) 
{
    m_Name = "CEditingActionFeatRnaToGeneComment";
}

bool CEditingActionFeatRnaToGeneComment::SameObject()
{
    return OtherFeatIs(CSeqFeatData::eSubtype_gene);
}

void CEditingActionFeatRnaToGeneComment::Find(EActionType action)
{
    FindFeatForAnotherFeat(action, SAnnotSelector(CSeqFeatData::eSubtype_gene));
}

void CEditingActionFeatRnaToGeneComment::FindRelated(EActionType action)
{
    FindRelatedFeatForAnotherFeat(action, SAnnotSelector(CSeqFeatData::eSubtype_gene));
}

void CEditingActionFeatRnaToGeneComment::SetFeat(CSeq_feat_Handle fh)
{
    SetFeatForAnotherFeat(fh, CSeqFeatData::eSubtype_gene);
}

CEditingActionFeatFunction::CEditingActionFeatFunction(CSeq_entry_Handle seh, const CSeqFeatData::ESubtype subtype, const CSeqFeatData::E_Choice feat_type)
    : IEditingActionFeat(seh, subtype, feat_type, "CEditingActionFeatFunction"), m_erase(false), m_function(NULL), m_qual("function")
{
}

void CEditingActionFeatFunction::Modify(EActionType action)
{
    bool found = false;
    GoToProtFeature();
    if (!IsCreateNew(action))
    {
        m_GBqual.Reset();
        if (m_EditedFeat->IsSetData() && m_EditedFeat->GetData().IsProt())
        {
            EDIT_EACH_ACTIVITY_ON_PROTREF(activity_it, m_EditedFeat->SetData().SetProt())
            {
                found = true;
                m_erase = false;
                m_function = &*activity_it;
                IEditingActionFeat::Modify(action);
                if (m_erase)
                    ERASE_ACTIVITY_ON_PROTREF(activity_it, m_EditedFeat->SetData().SetProt());
            }
        }   
        m_function = NULL;
        if (!found && m_EditedFeat->GetProtXref() != NULL)
        {
            CProt_ref& prot_feat = m_EditedFeat->SetProtXref();
            EDIT_EACH_ACTIVITY_ON_PROTREF(activity_it, prot_feat)
            {
                found = true;
                m_erase = false;
                m_function = &*activity_it;
                IEditingActionFeat::Modify(action);
                if (m_erase)
                    ERASE_ACTIVITY_ON_PROTREF(activity_it, prot_feat);
            }
        }
        m_function = NULL;
        if (!found)
        {
            if (IsFrom(action) && m_EditedFeat->IsSetQual() && !m_EditedFeat->GetQual().empty() && m_EditedFeat->GetQual().capacity() < 2 * m_EditedFeat->GetQual().size())
            {
                m_EditedFeat->SetQual().reserve(2 * m_EditedFeat->GetQual().size());
            }
            EDIT_EACH_GBQUAL_ON_SEQFEAT(gbqual_it, *m_EditedFeat)
            {
                if ((*gbqual_it)->IsSetQual() && (*gbqual_it)->GetQual() == m_qual)
                {
                    found = true;
                    m_erase = false;
                    m_GBqual = *gbqual_it;
                    IEditingActionFeat::Modify(action);
                    if (m_erase)
                        ERASE_GBQUAL_ON_SEQFEAT(gbqual_it, *m_EditedFeat);
                }
            }
            if (IsFrom(action))
            {
                m_EditedFeat->SetQual().shrink_to_fit();
            }
        }
    }
    if (!found)
    {
        m_function = NULL;
        m_GBqual.Reset();
        m_erase = false;
        IEditingActionFeat::Modify(action);
    }
    GoToOrigFeature();
}

bool CEditingActionFeatFunction::IsSetValue()
{
    return m_function != NULL ||  (m_GBqual && m_GBqual->IsSetVal());
}

void CEditingActionFeatFunction::SetValue(const string &value)
{
    if (m_GBqual)
    {
        m_GBqual->SetVal(value);
    }
    else if (m_function)
    {
        *m_function = value;
    }
    else if (m_EditedFeat->IsSetData() && m_EditedFeat->GetData().IsProt())
    {
        m_EditedFeat->SetData().SetProt().SetActivity().push_back(value);
    }   
    else if (m_EditedFeat->IsSetData() && m_EditedFeat->GetData().IsCdregion()) //(m_EditedFeat->GetProtXref() != NULL)
    {
        m_EditedFeat->SetProtXref().SetActivity().push_back(value);
    }
    else
    {
        m_EditedFeat->AddQualifier(m_qual, value);
    }
}

string CEditingActionFeatFunction::GetValue()
{
    if (m_GBqual)
        return  m_GBqual->GetVal();
    else if (m_function)
        return *m_function;
    return kEmptyStr;
}

void CEditingActionFeatFunction::ResetValue()
{
    m_erase = true;
}

CEditingActionFeatTranscriptId::CEditingActionFeatTranscriptId(CSeq_entry_Handle seh, const CSeqFeatData::ESubtype subtype, const CSeqFeatData::E_Choice feat_type)
    : CEditingActionFeatGbQualTwoNames(seh, subtype, feat_type, "transcript_id", "orig_transcript_id")
{
}

bool CEditingActionFeatTranscriptId::IsSetValue()
{
    return (m_EditedFeat->IsSetData() && m_EditedFeat->GetData().IsRna() && m_EditedFeat->IsSetProduct() && m_EditedFeat->GetProduct().GetId() != NULL) ||  
        (m_GBqual && m_GBqual->IsSetVal());
}

void CEditingActionFeatTranscriptId::SetValue(const string &value)
{
    if (m_GBqual)
        m_GBqual->SetVal(value);
    else if (m_EditedFeat->IsSetData() && m_EditedFeat->GetData().IsRna())
    {
        CRef<CSeq_id> new_id(new CSeq_id);
        new_id->SetLocal().SetStr(value);
        m_EditedFeat->SetProduct().SetWhole().Assign(*new_id);
    }
    else
        m_EditedFeat->AddQualifier(m_qual, value);   
}

string CEditingActionFeatTranscriptId::GetValue()
{
    if (m_GBqual)
        return m_GBqual->GetVal();
    else if (m_EditedFeat->IsSetData() && m_EditedFeat->GetData().IsRna() && m_EditedFeat->IsSetProduct())
    {
        const CSeq_id* id = m_EditedFeat->GetProduct().GetId();
        if (id)
        {
            string label;
            id->GetLabel(&label, CSeq_id::eContent);
            return label;
        }
    }
    
    return kEmptyStr;
}

void CEditingActionFeatTranscriptId::ResetValue()
{
    if (m_GBqual)
        m_erase = true;
    else if (m_EditedFeat->IsSetData() && m_EditedFeat->GetData().IsRna() && m_EditedFeat->IsSetProduct())
        m_EditedFeat->ResetProduct();
}

IEditingActionFeat* CreateActionFeat(CSeq_entry_Handle seh, string field, const CSeqFeatData::ESubtype subtype, const CSeqFeatData::E_Choice feat_type)
{
    NStr::ToLower(field);
    if (field == "comment" || field == "note")
        return new CEditingActionFeatComment(seh, subtype, feat_type);
    if (field == "exception")
        return new CEditingActionFeatException(seh, subtype, feat_type);
    if (field == "locus" || field == "gene locus") 
        return new CEditingActionFeatGeneLocusPlain(seh, subtype, feat_type);
    if (field == "rpt_unit_seq")
        return new CEditingActionFeatGbQual(seh, subtype, feat_type, "rpt_unit_seq");
    if (field == "inference")
        return new CEditingActionFeatGbQual(seh, subtype, feat_type, "inference");
    if (field == "bound_moiety")
        return new CEditingActionFeatGbQual(seh, subtype, feat_type, "bound_moiety");
    if (field == "chromosome")
        return new CEditingActionFeatGbQual(seh, subtype, feat_type, "chromosome");
    if (field == "compare")
        return new CEditingActionFeatGbQual(seh, subtype, feat_type, "compare");
    if (field == "cons_slice")
        return new CEditingActionFeatGbQual(seh, subtype, feat_type, "cons_slice");
    if (field == "direction")
        return new CEditingActionFeatGbQual(seh, subtype, feat_type, "direction");
    if (field == "environmental_sample")
        return new CEditingActionFeatGbQual(seh, subtype, feat_type, "environmental_sample");
    if (field == "experiment")
        return new CEditingActionFeatGbQual(seh, subtype, feat_type, "experiment");
    if (field == "focus")
        return new CEditingActionFeatGbQual(seh, subtype, feat_type, "focus");
    if (field == "frequency")
        return new CEditingActionFeatGbQual(seh, subtype, feat_type, "frequency");
    if (field == "function")
        return new CEditingActionFeatFunction(seh, subtype, feat_type);
    if (field == "label")
        return new CEditingActionFeatGbQual(seh, subtype, feat_type, "label");
    if (field == "map")
        return new CEditingActionFeatGbQual(seh, subtype, feat_type, "map");
    if (field == "mod_base")
        return new CEditingActionFeatGbQual(seh, subtype, feat_type, "mod_base");
    if (field == "mol_type")
        return new CEditingActionFeatGbQual(seh, subtype, feat_type, "mol_type");
    if (field == "number")
        return new CEditingActionFeatGbQual(seh, subtype, feat_type, "number");
    if (field == "old_locus_tag")
        return new CEditingActionFeatGbQual(seh, subtype, feat_type, "old_locus_tag");
    if (field == "operon")
        return new CEditingActionFeatGbQual(seh, subtype, feat_type, "operon");
    if (field == "organism")
        return new CEditingActionFeatGbQual(seh, subtype, feat_type, "organism");
    if (field == "pcr_conditions")
        return new CEditingActionFeatGbQual(seh, subtype, feat_type, "pcr_conditions");
    if (field == "phenotype")
        return new CEditingActionFeatGbQual(seh, subtype, feat_type, "phenotype");
    if (field == "plasmid")
        return new CEditingActionFeatGbQual(seh, subtype, feat_type, "plasmid");
    if (field == "protein_id")
        return new CEditingActionFeatGbQual(seh, subtype, feat_type, "protein_id");
    if (field == "rearranged")
        return new CEditingActionFeatGbQual(seh, subtype, feat_type, "rearranged");
    if (field == "regulatory_class")
        return new CEditingActionFeatGbQual(seh, subtype, feat_type, "regulatory_class");
    if (field == "replace")
        return new CEditingActionFeatGbQual(seh, subtype, feat_type, "replace");
    if (field == "rpt_family")
        return new CEditingActionFeatGbQual(seh, subtype, feat_type, "rpt_family");
    if (field == "rpt_type")
        return new CEditingActionFeatGbQual(seh, subtype, feat_type, "rpt_type");
    if (field == "rpt_unit")
        return new CEditingActionFeatGbQual(seh, subtype, feat_type, "rpt_unit");
    if (field == "rpt_unit_range")
        return new CEditingActionFeatGbQual(seh, subtype, feat_type, "rpt_unit_range");
    if (field == "segment")
        return new CEditingActionFeatGbQual(seh, subtype, feat_type, "segment");
    if (field == "sequenced_mol")
        return new CEditingActionFeatGbQual(seh, subtype, feat_type, "sequenced_mol");
    if (field == "standard_name")
        return new CEditingActionFeatGbQual(seh, subtype, feat_type, "standard_name");
    if (field == "tag_peptide")
        return new CEditingActionFeatGbQual(seh, subtype, feat_type, "tag_peptide");
    if (field == "transcript_id")
        return new CEditingActionFeatTranscriptId(seh, subtype, feat_type);
    if (field == "transgenic")
        return new CEditingActionFeatGbQual(seh, subtype, feat_type, "transgenic");
    if (field == "transl_except")
        return new CEditingActionFeatTranslExcept(seh, subtype, feat_type);
    if (field == "usedin")
        return new CEditingActionFeatGbQual(seh, subtype, feat_type, "usedin");
    if (field == "mobile_element")
        return new CEditingActionFeatGbQual(seh, subtype, feat_type, "mobile_element");
    if (field == "mobile_element_type")
        return new CEditingActionFeatGbQual(seh, subtype, feat_type, "mobile_element_type");
    if (field == "mobile_element_type_type")
        return new CEditingActionFeatDualVal1(seh, subtype, feat_type, "mobile_element_type");
    if (field == "mobile_element_type_name")
        return new CEditingActionFeatDualVal2(seh, subtype, feat_type, "mobile_element_type");
    if (field == "satellite")
        return new CEditingActionFeatGbQual(seh, subtype, feat_type, "satellite");
    if (field == "satellite_type")
        return new CEditingActionFeatDualVal1(seh, subtype, feat_type, "satellite");
    if (field == "satellite_name")
        return new CEditingActionFeatDualVal2(seh, subtype, feat_type, "satellite");
    if (field == "db_xref")
        return new CEditingActionFeatDbxref(seh, subtype, feat_type);
    if (field == "evidence")
        return new CEditingActionFeatEvidence(seh, subtype, feat_type);
    if (field == "pseudogene" || field == "pseudo")
        return new CEditingActionFeatPseudo(seh, subtype, feat_type);
    if (field == "codons_recognized" || field == "codons recognized")
        return new CEditingActionFeatCodonsRecognized(seh, subtype, feat_type);
    if (field == "anticodon")
        return new CEditingActionFeatAnticodon(seh, subtype, feat_type);
    if (field == "tag_peptide" || field == "tag-peptide")
        return new CEditingActionFeatRnaQual(seh, subtype, feat_type, "tag_peptide");
    if (field == "region")
        return new CEditingActionFeatRegion(seh, subtype, feat_type);
    if (field == "codon_start")
        return new CEditingActionFeatCodonStart(seh, subtype, feat_type);
    if (field == "transl_table")
        return new CEditingActionFeatTranslTable(seh, subtype, feat_type);
    if (field == "product name" || field == "product" || field == "name")
        return new CEditingActionFeatProduct(seh, subtype, feat_type);
    if (field == "desc" || field == "description")
        return new CEditingActionFeatDesc(seh, subtype, feat_type);
    if (field == "ncrna class" || field == "ncrna_class")
        return new CEditingActionFeatNcRnaClass(seh, subtype, feat_type);
    if (field == "locus_tag")
        return new CEditingActionFeatLocus_tag(seh, subtype, feat_type);
    if (field == "maploc")
        return new CEditingActionFeatMaploc(seh, subtype, feat_type);
    if (field == "synonym" || field == "gene_synonym")
        return new CEditingActionFeatSynonym(seh, subtype, feat_type);
    if (field == "allele")
        return new CEditingActionFeatAllele(seh, subtype, feat_type);
    if (field == "activity")
        return new CEditingActionFeatActivity(seh, subtype, feat_type);
    if (field == "partial")
        return new CEditingActionFeatPartial(seh, subtype, feat_type);
    if (field == "ec_number")
        return new CEditingActionFeatEcNumber(seh, subtype, feat_type);
    if (field == "translation")
        return new CEditingActionFeatTranslation(seh, subtype, feat_type);
    return NULL;
}

IEditingActionFeat* CreateActionRna(CSeq_entry_Handle seh, string field, const CSeqFeatData::ESubtype subtype)
{
    NStr::ToLower(field);
    if (field == "gene locus") 
        return new CEditingActionFeatGeneLocusRna(seh, subtype, CSeqFeatData::e_Rna);
    if (field == "gene description")
        return new CEditingActionFeatRnaToGeneDesc(seh, subtype);
    if (field == "gene maploc")
        return new CEditingActionFeatRnaToGeneMaploc(seh, subtype);
    if (field == "gene locus tag")
        return new CEditingActionFeatRnaToGeneLocus_tag(seh, subtype);
    if (field == "gene synonym")
        return new CEditingActionFeatRnaToGeneSynonym(seh, subtype);
    if (field == "gene comment")
        return new CEditingActionFeatRnaToGeneComment(seh, subtype);
    
    return CreateActionFeat(seh, field, subtype, CSeqFeatData::e_Rna);
}

IEditingActionFeat* CreateActionCdsGeneProt(CSeq_entry_Handle seh, string field, const CSeqFeatData::ESubtype subtype)
{
    NStr::ToLower(field);
    if (field == "gene locus") 
        return  new CEditingActionFeatGeneLocusCdsGeneProt(seh, subtype, CSeqFeatData::GetTypeFromSubtype(subtype));
    if (field == "gene description" || field == "protein description" || field == "mat_peptide description")
        return new CEditingActionFeatDesc(seh, subtype, CSeqFeatData::GetTypeFromSubtype(subtype));
    if (field == "gene maploc")
        return new CEditingActionFeatMaploc(seh, subtype, CSeqFeatData::GetTypeFromSubtype(subtype));
    if (field == "gene locus tag")
        return new CEditingActionFeatLocus_tag(seh, subtype, CSeqFeatData::GetTypeFromSubtype(subtype));
    if (field == "gene synonym")
        return new CEditingActionFeatSynonym(seh, subtype, CSeqFeatData::GetTypeFromSubtype(subtype));
    if (field == "gene comment" || field == "cds comment" || field == "mrna comment" || field == "mat_peptide comment" || field == "protein comment")
        return new CEditingActionFeatComment(seh, subtype, CSeqFeatData::GetTypeFromSubtype(subtype));
    if (field == "gene old_locus_tag")
        return new CEditingActionFeatGbQual(seh, subtype, CSeqFeatData::GetTypeFromSubtype(subtype), "old_locus_tag");
    if (field == "gene allele")
        return new CEditingActionFeatAllele(seh, subtype, CSeqFeatData::GetTypeFromSubtype(subtype));
    if (field == "gene inference" || field == "cds inference")
        return new CEditingActionFeatGbQual(seh, subtype, CSeqFeatData::GetTypeFromSubtype(subtype), "inference");
    if (field == "protein name" || field == "mrna product" || field == "mat_peptide name")
        return new CEditingActionFeatProduct(seh, subtype, CSeqFeatData::GetTypeFromSubtype(subtype));
    if (field == "protein ec number" || field == "mat_peptide ec number")
        return new CEditingActionFeatEcNumber(seh, subtype, CSeqFeatData::GetTypeFromSubtype(subtype));
    if (field == "protein activity" || field == "mat_peptide activity")
        return new CEditingActionFeatActivity(seh, subtype, CSeqFeatData::GetTypeFromSubtype(subtype));
    if (field == "codon-start")
        return new CEditingActionFeatCodonStart(seh, subtype, CSeqFeatData::GetTypeFromSubtype(subtype));

    return NULL;
}

END_NCBI_SCOPE

