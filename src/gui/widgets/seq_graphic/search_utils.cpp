/*  $Id: search_utils.cpp 44869 2020-04-01 15:14:14Z asztalos $
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
 * Authors:  Igor Filippov
 *
 * File Description:
 *
 */

#include <ncbi_pch.hpp>

#include <objects/general/Dbtag.hpp>
#include <objects/seqfeat/Gene_ref.hpp>
#include <objects/seqfeat/Gb_qual.hpp>
#include <objects/seqfeat/BioSource.hpp>
#include <objects/seqfeat/Org_ref.hpp>
#include <objects/seqfeat/Imp_feat.hpp>
#include <objects/seqfeat/Variation_ref.hpp>
#include <objects/seqfeat/Clone_ref.hpp>
#include <objects/seqfeat/SeqFeatData.hpp>
#include <objmgr/scope.hpp>
#include <objmgr/feat_ci.hpp>
#include <gui/widgets/seq_graphic/feature_track.hpp>
#include <gui/widgets/seq_graphic/gene_model_track.hpp>
#include <gui/widgets/seq_graphic/track_container_track.hpp>
#include <util/sequtil/sequtil_manip.hpp>
#include <objmgr/util/sequence.hpp>
#include <objmgr/util/feature.hpp>
#include <objtools/snputil/snp_utils.hpp>
#include <gui/objutils/na_utils.hpp>
#include <gui/widgets/seq_graphic/feature_panel.hpp>
#include <util/xregexp/regexp.hpp>
#include <objects/variation/VariantPlacement.hpp>
#include <gui/objutils/snp_gui.hpp>
#include "search_utils.hpp"

#include <unordered_map>

BEGIN_NCBI_SCOPE
USING_SCOPE(objects);

namespace
{
    class CTrackFeatSubtype
    {
    public:
        CTrackFeatSubtype()  {}

        bool ContainerBegin(const CTempTrackProxy*, const CTrackContainer*, bool) const { return true; }
        void ContainerEnd(const CTempTrackProxy*, const CTrackContainer*, bool) const { ;; }
        bool Track(const CTempTrackProxy*, const CLayoutTrack* t, bool visible); 
        bool VarTrackExists() {return m_var_track;}
        bool SnpTrackExists() {return m_snp_track;}
        const set<string>& GetAnnotsVar() {return m_annots_var;}
        const set<string>& GetAnnotsSnp() {return m_annots_snp;}
        const set<string>& GetAnnots() {return m_annots;}
        const set<CSeqFeatData::ESubtype>& GetSubtypes() {return m_subtypes;}
        unordered_map<string, set<CSeqFeatData::ESubtype>> GetNamedAnnotSubtypes() {return m_named_annot_subtypes;}
    private:
        bool  m_var_track = false;
        bool  m_snp_track = false;
        set<string> m_annots_var, m_annots_snp, m_annots;
        set<CSeqFeatData::ESubtype> m_subtypes;
        unordered_map<string, set<CSeqFeatData::ESubtype>>  m_named_annot_subtypes;
    };

    bool CTrackFeatSubtype::Track(const CTempTrackProxy*, const CLayoutTrack* t, bool visible)
    { 
        if (visible) 
        {            
            const IFeatureTrackBase* ft = dynamic_cast<const IFeatureTrackBase*>(t);
            if (ft)
            {
                string annot = ft->GetAnnot();
                if (ft->IsVarTrack())
                {
                    m_var_track = true;
                    if (!annot.empty())
                        m_annots_var.insert(annot);
                    return true;
                }
                if (ft->IsSnpTrack())
                {
                    m_snp_track = true;
                    if (!annot.empty())
                        m_annots_snp.insert(annot);
                    return true;
                }
                set<CSeqFeatData::ESubtype> subtypes;
                ft->GetFeatSubtypes(subtypes);
                m_subtypes.insert(subtypes.begin(), subtypes.end());
                m_annots.insert(annot);
                m_named_annot_subtypes[CSeqUtils::NameTypeStrToValue(annot) == CSeqUtils::eAnnot_Unnamed ? kEmptyStr : annot].insert(subtypes.begin(), subtypes.end());
            }            
        }
        return true; 
    }
} // namespace

static bool s_UseCustomLabel(const CSeq_feat &feat) 
{
    static set<CSeqFeatData::ESubtype> subtypes{ CSeqFeatData::eSubtype_misc_feature, CSeqFeatData::eSubtype_misc_recomb, CSeqFeatData::eSubtype_misc_structure, CSeqFeatData::eSubtype_mobile_element,
        CSeqFeatData::eSubtype_oriT, CSeqFeatData::eSubtype_protein_bind, CSeqFeatData::eSubtype_region, CSeqFeatData::eSubtype_regulatory, CSeqFeatData::eSubtype_rep_origin,
        CSeqFeatData::eSubtype_repeat_region, CSeqFeatData::eSubtype_stem_loop };

    return subtypes.end() != subtypes.find(feat.GetData().GetSubtype());
}

static void s_LabelFromQualifiers(const CSeq_feat &feat, string& label) 
{
    static vector<string> quals{ "regulatory_class", "recombination_class", "feat_class", "bound_moiety", "mobile_element_type", "mobile_element", "rpt_type",
        "satellite", "rpt_family", "mod_base", "operon", "standard_name", "allele" };

    for (const auto &qual_name : quals) {
        const string &value = feat.GetNamedQual(qual_name);
        if (value.empty())
            continue;

        label = value;
        break;
    }
}

static bool s_IncludeFeatureTypeInLabel(const objects::CSeq_feat &feat) 
{
    static vector<string> class_quals{ "feat_class", "regulatory_class", "recombination_class", "mobile_element_type", "mobile_element", "rpt_type", "satellite", "rpt_family" };
    for (const auto &qual_name : class_quals) {
        const string &value = feat.GetNamedQual(qual_name);
        if (!value.empty())
            return false;
    }
    return true;
}

static void s_GetCustomLabel(const CSeq_feat &feat, string& label) 
{
    CSeqFeatData::ESubtype subtype = feat.GetData().GetSubtype();
    if (s_IncludeFeatureTypeInLabel(feat) && ((subtype != CSeqFeatData::eSubtype_misc_feature) && (subtype != CSeqFeatData::eSubtype_region)))  // Add the subtype only for features that do not have specific qualifiers
        label = CSeqFeatData::SubtypeValueToName(subtype);
    string second_part;
    if (feat.GetData().IsRegion()) {
        second_part = feat.GetData().GetRegion();
    }
    else {
        s_LabelFromQualifiers(feat, second_part);
    }
    if (second_part.empty() && feat.IsSetComment()) {
        second_part = feat.GetComment();
        size_t pos = second_part.find(';');
        if (pos != string::npos)
            second_part = second_part.substr(0, pos);
    }
    if (!second_part.empty()) {
        if (!label.empty())
            label += ": ";
        label += second_part;
    }
    if (label.empty())
        label = CSeqFeatData::SubtypeValueToName(subtype);
}

static inline void s_GetSeqLabel(const CSeq_id& id, string* label,	CScope* scope) 
{
    // check: scope is NEEDED [protected]

    CConstRef<CSeq_id> id_ptr(&id);
    /*if (scope && id_ptr->IsGi()) {
        CSeq_id_Handle sih = sequence::GetId(id, *scope, sequence::eGetId_Best);
        if (sih) {
            id_ptr = sih.GetSeqId();
        }
    }
    */
    id_ptr->GetLabel(label, CSeq_id::eContent, CSeq_id::fLabel_Default);
}

static inline
void s_GetSeq_TotalRangeLabel(const CSeq_loc& loc, string* label, CScope* scope)
{
    // check: scope is NEEDED [indirectly protected]


    const CSeq_id& id = sequence::GetId(loc, scope);
    TSeqRange range = loc.GetTotalRange();
    s_GetSeqLabel(id, label, scope);
    *label += ": ";
    *label += NStr::IntToString(range.GetFrom() + 1, NStr::fWithCommas);
    *label += "-";
    *label += NStr::IntToString(range.GetTo() + 1, NStr::fWithCommas);

    *label += " [";
    *label += NStr::IntToString(range.GetLength(), NStr::fWithCommas);
    *label += "]";

    string extra_info = kEmptyStr;
    ENa_strand strand = sequence::GetStrand(loc, scope);
    switch (strand) {
    case eNa_strand_minus:
        extra_info = "-";
        break;
    case eNa_strand_unknown:
        break;
    case eNa_strand_plus:
    default:
        extra_info = "+";
        break;
    }

    size_t intervals = 0;
    switch (loc.Which()) {
    case CSeq_loc::e_Int:
        intervals = 1;
        break;

    case CSeq_loc::e_Packed_int:
        intervals = loc.GetPacked_int().Get().size();
        break;

    case CSeq_loc::e_Packed_pnt:
        intervals = loc.GetPacked_pnt().GetPoints().size();
        break;

    case CSeq_loc::e_Mix:
        /// FIXME: this may not always be correct -
        /// a mix may be a mix of complex intervals
        intervals = loc.GetMix().Get().size();
        break;

    default:
        break;
    }

    if (intervals != 0) {
        if (!extra_info.empty())
            *label += ", ";
        extra_info += NStr::SizetToString(intervals);
        extra_info += " interval";
        if (intervals != 1) {
            extra_info += "s";
        }
    }
    if (!extra_info.empty()) {
        *label += " (";
        *label += extra_info;
        *label += ")";
    }
    
}

static void s_GetFeatureLabel(const CSeq_feat& feat, vector<string>& feat_labels, CScope* scope)
{
    
    // most of the features will have only one label, exception is: gene
    string label;
    if (s_UseCustomLabel(feat)) {
        s_GetCustomLabel(feat, label);
    }
    /// for content, we use the product seq-id, if it
    /// is available; otherwise, we use the standard text
    else if (feat.IsSetProduct()) {
        try {
            const CSeq_id& id = sequence::GetId(feat.GetProduct(), scope);
            s_GetSeqLabel(id, &label, scope);
        }
        catch (CException&) {
            feature::GetLabel(feat, &label, feature::fFGL_Content | feature::fFGL_NoQualifiers, scope);
        }
    }
    else {
        switch (feat.GetData().GetSubtype()) {
        case CSeqFeatData::eSubtype_imp:
        { { // logic borrowed from misc_feature
                bool got_label = false;
                if (feat.IsSetQual()) {
                    ITERATE(CSeq_feat::TQual, it, feat.GetQual()) {
                        if (NStr::EqualNocase((*it)->GetQual(), "Name") ||
                            NStr::EqualNocase((*it)->GetQual(), "ID")) {
                            label = (*it)->GetVal();
                            got_label = true;
                            if (NStr::EqualNocase((*it)->GetQual(), "Name"))
                                break;
                        }
                    }
                }
                if (!got_label) {
                    feature::GetLabel(feat, &label, feature::fFGL_Content | feature::fFGL_NoComments | feature::fFGL_NoQualifiers, scope);
                    label += " ";
                    if (feat.GetData().GetImp().IsSetDescr()) {
                        label += feat.GetData().GetImp().GetDescr();
                    }
                }
                if (label.size() > 30) label = label.substr(0, 30) + "...";
            }}
        break;

        case CSeqFeatData::eSubtype_site:
            if (feat.GetData().GetSite() == CSeqFeatData::eSite_other  &&
                feat.IsSetComment()) {
                label += feat.GetComment();
            }
            else {
                feature::GetLabel(feat, &label, feature::fFGL_Content | feature::fFGL_NoQualifiers, scope);
            }
            break;

        case CSeqFeatData::eSubtype_exon:
        { {
                label += "exon";
                const string& q = feat.GetNamedQual("number");
                if (!q.empty()) {
                    label += " ";
                    label += q;
                }
            }}
        break;

        case CSeqFeatData::eSubtype_variation:
        { {
                /// RSID first
                NSnp::TRsid Rsid(NSnp::GetRsid(feat));
                if (Rsid) {
                    label += "rs";
                    label += NStr::NumericToString(Rsid);
                }
                else {
                    if (label.empty() && feat.IsSetComment()) {
                        label += feat.GetComment();
                    }
                }
            }}
        break;

        case CSeqFeatData::eSubtype_variation_ref:
        { {
                const CVariation_ref& var = feat.GetData().GetVariation();
                if (var.CanGetId()) {
                    if (var.GetId().GetTag().IsId()) {
                        label += var.GetId().GetDb() + "|";
                        label += NStr::IntToString(var.GetId().GetTag().GetId());
                    }
                    else {
                        label += var.GetId().GetTag().GetStr();
                    }
                }
                else {
                    if (feat.IsSetLocation()) {
                        const CSeq_loc& loc = feat.GetLocation();
                        s_GetSeq_TotalRangeLabel(loc, &label, scope);
                    }

                    //feature::GetLabel(*feat, label, feature::fFGL_Both, scope);
                }
            }}
        break;

        case CSeqFeatData::eSubtype_misc_feature:
        { {
                bool got_label = false;
                if (feat.IsSetQual()) {
                    ITERATE(CSeq_feat::TQual, it, feat.GetQual()) {
                        if (NStr::EqualNocase((*it)->GetQual(), "Name") ||
                            NStr::EqualNocase((*it)->GetQual(), "ID")) {
                            label += (*it)->GetVal();
                            got_label = true;
                            break;
                        }
                    }
                }
                if (!got_label) {
                    feature::GetLabel(feat, &label, feature::fFGL_Content | feature::fFGL_NoQualifiers, scope);
                }
            }}
        break;

        case CSeqFeatData::eSubtype_clone:
        { {
                const CSeqFeatData::TClone& clone = feat.GetData().GetClone();
                if (clone.IsSetName()) {
                    label += clone.GetName();
                }
                else {
                    feature::GetLabel(feat, &label, feature::fFGL_Content | feature::fFGL_NoQualifiers, scope);
                }
            }}
        break;

        case CSeqFeatData::eSubtype_seq:
        { {
                if (feat.IsSetTitle()) {
                    label += feat.GetTitle();
                }
                else {
                    s_GetSeq_TotalRangeLabel(feat.GetData().GetSeq(), &label, scope);
                }
            }}
        break;

        case CSeqFeatData::eSubtype_gene:
        {
            feature::GetLabel(feat, &label, feature::fFGL_Content, scope);
            if (feat.GetData().GetGene().IsSetLocus_tag()) {
                string aux_label = feat.GetData().GetGene().GetLocus_tag();
                if (!NStr::EqualCase(label, aux_label)) {
                    feat_labels.push_back(aux_label);
                }
            }
            break;
        }

        default:
            feature::GetLabel(feat, &label, feature::fFGL_Content, scope);
            break;
        }
    }
    feat_labels.push_back(label);
}

void s_GatherFeatures(const string &text, bool match_case,  CBioseq_Handle bsh, CScope &scope, CFeaturePanel *panel, vector<objects::CMappedFeat> &search_results)
{
    if (!bsh)
        return;

    CTrackFeatSubtype subtypes;
    panel->Traverse(subtypes, true);

    SAnnotSelector sel = CSeqUtils::GetAnnotSelector( CSeq_annot::TData::e_Ftable );
    sel.ResetAnnotsNames();
    sel.ExcludeAnnotType( CSeq_annot::TData::e_not_set);
    sel.ExcludeAnnotType( CSeq_annot::TData::e_Align);
    sel.ExcludeAnnotType( CSeq_annot::TData::e_Graph);
    sel.ExcludeAnnotType( CSeq_annot::TData::e_Ids);
    sel.ExcludeAnnotType( CSeq_annot::TData::e_Locs);
    sel.ExcludeAnnotType( CSeq_annot::TData::e_Seq_table);

    bool found = false;

    if (!subtypes.GetSubtypes().empty())
    {
        found = true;
        auto sub_it = subtypes.GetSubtypes().cbegin();
        sel.SetFeatSubtype(*sub_it);
        ++sub_it;
        for (; sub_it !=  subtypes.GetSubtypes().cend(); ++sub_it)
        {
            sel.IncludeFeatSubtype(*sub_it);
        }
    }
    for (const auto &annot :  subtypes.GetAnnots())
    {
        if (CSeqUtils::NameTypeStrToValue(annot) == CSeqUtils::eAnnot_Unnamed) 
        {
            sel.AddUnnamedAnnots();
        } 
        else 
        {
            sel.AddNamedAnnots(annot);
            if (NStr::StartsWith(annot, "NA0")) 
            {
                sel.IncludeNamedAnnotAccession(annot);
            }
        }   
    }
    unordered_map<string, set<CSeqFeatData::ESubtype>>  named_annot_subtypes = subtypes.GetNamedAnnotSubtypes();

    CFeat_CI fi(bsh, sel);

    CRef<CVariantPlacement> pPlacement;
    CRegexp re_snp("^([rs]s)([0-9]{3,})(?::.+)?$");
    CRegexp re_var("^([en]s)(td|v|sv)([0-9]+)(?::.+)?$");
    bool is_snp = re_snp.IsMatch(text);
    bool is_var = re_var.IsMatch(text);
    if ((subtypes.SnpTrackExists() && is_snp) || 
        (subtypes.VarTrackExists() && is_var))
    {
        found = false;
        named_annot_subtypes.clear();
        NSNPWebServices::TSNPSearchCompoundResultList SNPSearchResultList;
        try
        {
            NSNPWebServices::SearchByVarId(text, panel->GetCurrentAssembly(), SNPSearchResultList);
        } catch(exception&) {}
        NON_CONST_ITERATE(NSNPWebServices::TSNPSearchCompoundResultList, iSNPSearchResultList, SNPSearchResultList) 
        {
            NON_CONST_ITERATE(NSNPWebServices::TSNPSearchResultList, iSNPSearchResult, iSNPSearchResultList->second) 
            {
                // CVariation used as a search result can have one and only one placement
                NCBI_ASSERT((*iSNPSearchResult)->CanGetPlacements(), "Unexpected absence of placements in SNP Search Result!");
                if (!(*iSNPSearchResult)->CanGetPlacements())
                    continue;
                const CVariation::TPlacements& Placements((*iSNPSearchResult)->GetPlacements());
                NCBI_ASSERT(Placements.size(), "Unexpected number of placements in SNP Search Result!");
                if (Placements.empty())
                    continue;
                pPlacement = Placements.front();
                if (!pPlacement->IsSetLoc() || !sequence::IsSameBioseq(*bsh.GetSeqId(), *pPlacement->GetLoc().GetId(), &scope))
                    continue;
                sel = CSeqUtils::GetAnnotSelector( CSeq_annot::TData::e_Ftable );
                sel.SetFeatSubtype(CSeqFeatData::eSubtype_variation);
                sel.IncludeFeatSubtype(CSeqFeatData::eSubtype_variation_ref);
                for (const auto &annot : is_snp ? subtypes.GetAnnotsSnp() : subtypes.GetAnnotsVar())
                {
                    if (CSeqUtils::NameTypeStrToValue(annot) == CSeqUtils::eAnnot_Unnamed) 
                    {
                        sel.AddUnnamedAnnots();
                    } 
                    else 
                    {
                        sel.AddNamedAnnots(annot);
                        sel.IncludeNamedAnnotAccession(annot);
                    }                     
                    named_annot_subtypes[CSeqUtils::NameTypeStrToValue(annot) == CSeqUtils::eAnnot_Unnamed ? kEmptyStr : annot].insert(CSeqFeatData::eSubtype_variation);
                    named_annot_subtypes[CSeqUtils::NameTypeStrToValue(annot) == CSeqUtils::eAnnot_Unnamed ? kEmptyStr : annot].insert(CSeqFeatData::eSubtype_variation_ref);
                }
                fi = CFeat_CI(scope, pPlacement->GetLoc(), sel);
                found = true;
                break;
            }
            if (found)
                break;
        }
    }    
    
    if (!found)
        return;

    for( ; fi; ++fi)
    {
        if (pPlacement && fi->GetTotalRange() != pPlacement->GetLoc().GetTotalRange())
            continue;
        auto f = fi->GetSeq_feat_Handle();
        string annot = f.GetAnnot().IsNamed() ? f.GetAnnot().GetName() : kEmptyStr;
        if (named_annot_subtypes.find(annot) == named_annot_subtypes.end() || 
            named_annot_subtypes[annot].find(f.GetFeatSubtype()) == named_annot_subtypes[annot].end())
            continue;
        
        vector<string> feat_labels;
        s_GetFeatureLabel(*f.GetSeq_feat(), feat_labels, &scope);

        NStr::ECase case_sens = (match_case) ? NStr::eCase : NStr::eNocase;
       
        auto it = find_if(feat_labels.begin(), feat_labels.end(),
            [&text, &case_sens](const string& elem) { return (NStr::Find(elem, text, case_sens) != NPOS); });
        if (it != feat_labels.end()) {
            search_results.push_back(*fi);
        }
    }
}

TSeqRange s_splitPosOrRange(const string &text)
{
    TSeqRange empty;
    CRegexp re("^([0-9]+[km]?)((-|to|\\.\\.+|:|\\/|_)([0-9]+[km]?))?$", CRegexp::fCompile_ignore_case);
    if (re.IsMatch(text)) 
    {
        re.GetMatch(text, 0, 0, CRegexp::fMatch_default, true);
        if (re.NumFound() == 2)
        {
            CTempString pos = text;
            int multiplier = 1;
            if (NStr::EndsWith(pos, "k", NStr::eNocase))
            {
                multiplier = 1000;
                NStr::TrimSuffixInPlace(pos, "k", NStr::eNocase);
            }
            if (NStr::EndsWith(pos, "m", NStr::eNocase))
            {
                multiplier = 1000000;
                NStr::TrimSuffixInPlace(pos, "m", NStr::eNocase);
            }
            return TSeqRange(NStr::StringToNumeric<TSeqPos>(pos) * multiplier, NStr::StringToNumeric<TSeqPos>(pos) * multiplier);
        }
        else if (re.NumFound() == 5)
        {
            CTempString pos0 = re.GetSub(text, 1);
            CTempString pos1 = re.GetSub(text, 4);
            int multiplier0 = 1;
            int multiplier1 = 1;
            if (NStr::EndsWith(pos0, "k", NStr::eNocase))
            {
                multiplier0 = 1000;
                NStr::TrimSuffixInPlace(pos0, "k", NStr::eNocase);
            }
            if (NStr::EndsWith(pos1, "k", NStr::eNocase))
            {
                multiplier1 = 1000;
                NStr::TrimSuffixInPlace(pos1, "k", NStr::eNocase);
            }
            if (NStr::EndsWith(pos0, "m", NStr::eNocase))
            {
                multiplier0 = 1000000;
                NStr::TrimSuffixInPlace(pos0, "m", NStr::eNocase);
            }
            if (NStr::EndsWith(pos1, "m", NStr::eNocase))
            {
                multiplier1 = 1000000;
                NStr::TrimSuffixInPlace(pos1, "m", NStr::eNocase);
            }
            return TSeqRange(NStr::StringToNumeric<TSeqPos>(pos0) * multiplier0, NStr::StringToNumeric<TSeqPos>(pos1) * multiplier1);
        }
    }
    return empty;
 }

void s_GetSubtypesForType(set<CSeqFeatData::ESubtype> &subtypes, CSeqFeatData::E_Choice feat)
{
    for (int i = 1; i < CSeqFeatData::eSubtype_max; ++i)
    {
        CSeqFeatData::ESubtype sub = static_cast<CSeqFeatData::ESubtype>(i);
        if (CSeqFeatData::GetTypeFromSubtype(sub) == feat)
            subtypes.insert(sub);
    }
}

END_NCBI_SCOPE


