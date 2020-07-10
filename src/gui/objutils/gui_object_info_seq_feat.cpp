/*  $Id: gui_object_info_seq_feat.cpp 45057 2020-05-19 16:57:25Z shkeda $
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
 * Authors: Roman Katargin, Liangshou Wu
 *
 * File Description:
 *
 */

#include <ncbi_pch.hpp>

#include <gui/objutils/gui_object_info_seq_feat.hpp>

// some common includes
#include <objects/general/User_object.hpp>
#include <objects/general/User_field.hpp>
#include <objects/general/Dbtag.hpp>
#include <objects/general/Object_id.hpp>
#include <objects/seqfeat/Cdregion.hpp>
#include <objects/seqfeat/RNA_ref.hpp>
#include <objects/seqfeat/RNA_gen.hpp>
#include <gui/objutils/utils.hpp>
#include <gui/objutils/link_utils.hpp>
#include <gui/objutils/label.hpp>
#include <objmgr/util/feature.hpp>
#include <objmgr/util/sequence.hpp>
#include <objmgr/seq_vector.hpp>
#include <util/sequtil/sequtil_manip.hpp>
#include <gui/objutils/create_params_seq_feat_group.hpp>

// include other seq-feat implmentations
#include "gui_object_info_variant.cpp"    /// structural variants
#include "gui_object_info_variation.cpp" /// SNP
#include "gui_object_info_clone_feat.cpp"
#include "gui_object_info_primer_feat.cpp"
#include "gui_object_info_other_feats.cpp"

BEGIN_NCBI_SCOPE
USING_SCOPE(objects);

void initCGuiObjectInfoSeq_feat()
{
    CInterfaceRegistry::RegisterFactory(
            typeid(IGuiObjectInfo).name(),
            CSeq_feat::GetTypeInfo(),
            new CObjectInterfaceFactory<CGuiObjectInfoSeq_feat>());
}


CGuiObjectInfoSeq_feat* CGuiObjectInfoSeq_feat::CreateObject(
    SConstScopedObject& object, ICreateParams* params)
{
    CGuiObjectInfoSeq_feat* gui_info = NULL;
    const CSeq_feat* feat = dynamic_cast<const CSeq_feat*>(object.object.GetPointer());

    switch (feat->GetData().GetSubtype())
    {
    case CSeqFeatData::eSubtype_clone:
        {{
            if (feat->GetNamedQual("type") == "primer") {
                gui_info = new CGuiObjectInfoPrimer();
            } else {
                gui_info = new CGuiObjectInfoClone();
            }
        }}
        break;
    case CSeqFeatData::eSubtype_variation:
        gui_info = new CGuiObjectInfoVariation();
        break;
    case CSeqFeatData::eSubtype_variation_ref:
        gui_info = new CGuiObjectInfoVariant();
        break;
    case CSeqFeatData::eSubtype_imp:
        gui_info = new CGuiObjectInfoImpFeature();
        break;
    case CSeqFeatData::eSubtype_region:
        gui_info = new CGuiObjectInfoRegionFeature();
        break;
    case CSeqFeatData::eSubtype_seq:
        gui_info = new CGuiObjectInfoSeqFeature();
        break;
    case CSeqFeatData::eSubtype_site:
        gui_info = new CGuiObjectInfoSiteFeature();
        break;
    default:
        gui_info = new CGuiObjectInfoSeq_feat();
        break;
    }

    if (nullptr != params) {
        const CCreateParamsSeqFeatGroup* feat_group = dynamic_cast<const CCreateParamsSeqFeatGroup*>(params);
        if (nullptr != feat_group) {
            gui_info->m_FeaturesGroup = feat_group->GetFeaturesGroup();
        }
    }

    gui_info->m_Feat.Reset(feat);
    gui_info->m_Scope  = object.scope;
    gui_info->m_TaxId = 0;

    /// To guard against the cases where the feature contains location
    /// with multiple seq-ids
    const CSeq_feat_Base::TLocation& loc_obj = feat->GetLocation();
    if (loc_obj.GetId()) {
        gui_info->m_Location.Reset(&loc_obj);
    }

    return gui_info;
}


string CGuiObjectInfoSeq_feat::GetSubtype() const
{
    const CFeatList& feats(*CSeqFeatData::GetFeatList());
    CFeatListItem item;
    if (feats.GetItemBySubtype(m_Feat->GetData().GetSubtype(), item)) {
        return item.GetStoragekey();
    }
    return "";
}


string CGuiObjectInfoSeq_feat::GetLabel() const
{
    string label;
    CLabel::GetLabel(*m_Feat, &label, CLabel::eDescriptionBrief, m_Scope);
    return label;
}

static
float s_GetSupportTotal(const CSeq_feat& feat)
{
    float val = -1.f;
    if (feat.IsSetExts()) {
        const CSeq_feat::TExts& exts = feat.GetExts();
        ITERATE (CSeq_feat::TExts, iter, exts) {
            if ((*iter)->GetType().IsStr()  &&
                (*iter)->GetType().GetStr() == "Support"  &&
                (*iter)->GetFieldRef("Total")) 
            {
                const auto& d = (*iter)->GetFieldRef("Total")->GetData();
                if (d.IsInt()) {
                    val = d.GetInt();
                    break;
                } else if (d.IsReal()) {
                    val = d.GetReal();
                    break;
                }
            }
        }
    }
    return val;
}

string s_GetSpliceSite(const CSeq_feat& feat )
{
    string splice_site = "";

    if (feat.IsSetExts()) {
        const CSeq_feat::TExts& exts = feat.GetExts();
        ITERATE (CSeq_feat::TExts, iter, exts) {
            if ((*iter)->GetType().IsStr()  &&
                (*iter)->GetType().GetStr() == "Splice Sequence") {
                    CConstRef<CUser_field> donor = (*iter)->GetFieldRef("Donor");
                    CConstRef<CUser_field> acceptor = (*iter)->GetFieldRef("Acceptor");
                    if (!donor.IsNull() && donor->GetData().IsStr() &&
                        !acceptor.IsNull() && acceptor->GetData().IsStr()) {
                            string donor_str =  donor->GetData().GetStr();
                            string acceptor_str =  acceptor->GetData().GetStr();
                            splice_site = donor_str + "-" + acceptor_str + " ";
                            if ((donor_str == "GT" && acceptor_str == "AG") ||
                                (donor_str == "GC" && acceptor_str == "AG") ||
                                (donor_str == "AT" && acceptor_str == "AC"))
                                    splice_site += "(consensus)";
                            else
                                    splice_site += "(non-consensus)";
                    }
            }
        }
    }

    return splice_site;
}

string s_GetRnaClass(const CRNA_ref &rna)
{
    string rna_class;
    do {
        if (!rna.IsSetExt())
            break;

        if (!rna.GetExt().IsGen())
            break;

        if (!rna.GetExt().GetGen().IsSetClass())
            break;

        return rna.GetExt().GetGen().GetClass();
    } while (false);
    return string();
}

void CGuiObjectInfoSeq_feat::GetToolTip(ITooltipFormatter& tooltip, string& t_title, TSeqPos at_p, bool* isGeneratedBySvc) const
{
    if(isGeneratedBySvc) {
        *isGeneratedBySvc = false;
    }
    if (m_FeaturesGroup.empty()) {
        x_GetFeatureToolTip(tooltip, t_title, at_p);
    }
    else {
        x_GetGroupToolTip(tooltip, t_title, at_p);
    }
}


void CGuiObjectInfoSeq_feat::GetLinks(ITooltipFormatter& links, bool no_ncbi_base) const
{
    
    if (m_Location.Empty())
        return;
    
    CLinkUtils::TLinksMap links_by_type;
    CLinkUtils::AddFeatureLinks(m_Feat.GetObject(), m_Location.GetObject(), "", m_Scope.GetObject(), links_by_type, false);

    for (const auto &feat : m_FeaturesGroup) {
        if (!feat.first->IsSetData())
            continue;
        if (!feat.first->GetData().IsCdregion() && !feat.first->GetData().IsRna())
            continue;
            
        CLinkUtils::AddBlastProductLink(*feat.first, m_Scope.GetObject(), links_by_type, false);
    }
    
    if (links_by_type.empty())
        return;

    links.AddLinksTitle("Links & Tools");

    for (auto it_type = links_by_type.begin(); it_type != links_by_type.end(); ++it_type) {
        CLinkUtils::TLinks& links_by_name = it_type->second;
        
        for (auto it_name = links_by_name.begin(); it_name != links_by_name.end(); ++it_name) {
            string tmp_links;

            for (size_t i = 0; i < it_name->second.size(); ++i) {
                // This gives a separate row to each link:
                //tmp_links += CSeqUtils::CreateLinkRow((*iter).first, (*uiter).first, (*uiter).second);
                if (i > 0) tmp_links += ",&nbsp;";
                tmp_links += links.CreateLink(it_name->second[i].first, it_name->second[i].second);
            }

            links.AddLinkRow(it_name->first + ':', tmp_links, 250);
        }
    }
}


void CGuiObjectInfoSeq_feat::x_AddLocationRows(ITooltipFormatter& tooltip) const
{
    if (!m_Location)
        return;

    string curr_text;
    TSeqRange total_r = m_Location->GetTotalRange();
    ENa_strand strand = sequence::GetStrand(*m_Location);

    if (strand == eNa_strand_minus) {
        curr_text = "complement(";
    }
    curr_text += NStr::IntToString(total_r.GetFrom() + 1, NStr::fWithCommas);
    if (total_r.GetLength() > 1) {
        curr_text += "..";
        curr_text += NStr::IntToString(total_r.GetTo() + 1, NStr::fWithCommas);
    }
    if (strand == eNa_strand_minus) {
        curr_text += ")";
    }
    tooltip.AddRow("Location:", curr_text);

    TLens lens;
    string acc = x_GetAccession();
    if (acc.empty()) {
        acc = "master";
    }

    string len_suffix{ " nt" };
    if (nullptr != m_Location->GetId()) {
        CBioseq_Handle bsh = m_Scope->GetBioseqHandle(*m_Location);
        if (bsh && bsh.IsAa()) {
            len_suffix = " aa";
        }
    }

    lens.push_back(TLens::value_type("Span on " + acc, NStr::IntToString(total_r.GetLength(), NStr::fWithCommas) + len_suffix));

    x_GetLocLengthRow(*m_Location, *m_Feat, lens, len_suffix);

    x_GetProductLengthRow(*m_Feat, lens);

    if (lens.size() > 1) {
        tooltip.AddSectionRow("Length");
        ITERATE(TLens, iter, lens) {
            tooltip.AddRow(iter->first + ":", iter->second);
        }
    }
    else if (lens.size() == 1) {
        tooltip.AddRow("Length:", lens.begin()->second);
    }
}


void CGuiObjectInfoSeq_feat::x_AddGroupLocationRows(ITooltipFormatter& tooltip) const
{
    if (!m_Location)
        return;

    string len_suffix{ " nt" };
    CBioseq_Handle bsh;
    if (nullptr != m_Location->GetId()) {
        bsh = m_Scope->GetBioseqHandle(*m_Location);
        if (bsh && bsh.IsAa()) {
            len_suffix = " aa";
        }
    }
    if (!bsh)
        return;

    TLens lens;
    for (const auto &feat : m_FeaturesGroup) {
        if (!feat.first->IsSetData())
            continue;
        if (feat.first->GetData().IsCdregion()) {
            CConstRef<CSeq_loc> cds_loc = x_GetFeatLocation(*feat.first, bsh);
            x_GetLocLengthRow(*cds_loc, *feat.first, lens, len_suffix);
            x_GetProductLengthRow(*feat.first, lens);
            continue;
        }
        if (feat.first->GetData().IsRna()) {
            CConstRef<CSeq_loc> rna_loc = x_GetFeatLocation(*feat.first, bsh);
            string curr_text;
            TSeqRange total_r = rna_loc->GetTotalRange();
            ENa_strand strand = sequence::GetStrand(*rna_loc);

            if (strand == eNa_strand_minus) {
                curr_text = "complement(";
            }
            curr_text += NStr::IntToString(total_r.GetFrom() + 1, NStr::fWithCommas);
            if (total_r.GetLength() > 1) {
                curr_text += "..";
                curr_text += NStr::IntToString(total_r.GetTo() + 1, NStr::fWithCommas);
            }
            if (strand == eNa_strand_minus) {
                curr_text += ")";
            }
            tooltip.AddRow("Location:", curr_text);


            string acc = x_GetAccession();
            if (acc.empty()) {
                acc = "master";
            }

            lens.push_back(TLens::value_type("Span on " + acc, NStr::IntToString(total_r.GetLength(), NStr::fWithCommas) + len_suffix));
            x_GetLocLengthRow(*rna_loc, *feat.first, lens, len_suffix);
            continue;
        }
    }
        
    if (lens.size() > 1) {
        tooltip.AddSectionRow("Length");
        ITERATE(TLens, iter, lens) {
            tooltip.AddRow(iter->first + ":", iter->second);
        }
    }
    else if (lens.size() == 1) {
        tooltip.AddRow("Length:", lens.begin()->second);
    }
}


void CGuiObjectInfoSeq_feat::x_GetFeatureToolTip(ITooltipFormatter& tooltip, string& t_title, TSeqPos at_p) const
{
    CLabel::GetLabel(*m_Feat, &t_title, CLabel::eContent, m_Scope.GetPointer());
    string text(t_title);
    {
        // Remove the subtype from the title
        string to_remove(GetSubtype());
        to_remove += ": ";
        size_t pos = text.find(to_remove);
        if ((pos != string::npos) && (to_remove.length() < text.length()))
            text = text.substr(to_remove.length());
    }
    tooltip.AddRow(GetSubtype() + ':', text);

    // Add support info for introns: SV-2124
    if (m_Feat->IsSetData() && m_Feat->GetData().GetSubtype() == CSeqFeatData::eSubtype_intron) {
        auto intron_support = s_GetSupportTotal(*m_Feat);
        tooltip.AddRow("Number of Spliced Reads:", NStr::NumericToString(intron_support));

        string splice_site = s_GetSpliceSite(*m_Feat);
        tooltip.AddRow("Splice Sites:", splice_site);
    }

    // Add support for ncRNA_class: SV-2907
    if (m_Feat->IsSetData() && m_Feat->GetData().GetSubtype() == CSeqFeatData::eSubtype_ncRNA && m_Feat->GetData().IsRna()) {
        string rna_class = s_GetRnaClass(m_Feat->GetData().GetRna());
        if (!rna_class.empty())
            tooltip.AddRow("ncRNA_class:", rna_class);
    }

    const CSeq_feat::TData& data = m_Feat->GetData();
    string curr_text;

    if (CSeqUtils::IsPseudoFeature(*m_Feat)) {
        curr_text += "Pseudo";
    }
    if (CSeqUtils::IsPartialFeature(*m_Feat)) {
        curr_text += curr_text.empty() ? "" : ", ";
        curr_text += "Partial";
    }
    if (CSeqUtils::IsPartialStart(m_Feat->GetLocation())) {
        curr_text += curr_text.empty() ? "" : ", ";
        curr_text += "Partial start";
    }
    if (CSeqUtils::IsPartialStop(m_Feat->GetLocation())) {
        curr_text += curr_text.empty() ? "" : ", ";
        curr_text += "Partial stop";
    }
    if (CSeqUtils::IsException(*m_Feat) && m_Feat->IsSetExcept_text()) {
        curr_text += curr_text.empty() ? "" : ", ";
        curr_text += m_Feat->GetExcept_text();
    }
    if (!curr_text.empty()) {
        tooltip.AddRow("Qualifiers:", curr_text);
        curr_text.clear();
    }

    // Brief description goes here
    if (data.IsSeq()) {
        CLabel::GetLabel(*m_Feat, &curr_text, CLabel::eDescription, m_Scope.GetPointer());
    }
    else if (data.IsGene() && data.GetGene().IsSetDesc()) {
        curr_text += data.GetGene().GetDesc();
    }
    else {
        feature::GetLabel(*m_Feat, &curr_text, feature::fFGL_Content | feature::fFGL_NoQualifiers, m_Scope.GetPointer());
    }
    if (curr_text != t_title) {
        // avoid showing the duplicated information
        tooltip.AddRow("Title:", curr_text);
    }

    if (m_Feat->IsSetComment()
        && m_Feat->GetComment() != t_title
        && m_Feat->GetComment() != curr_text
        && string::npos == t_title.find(m_Feat->GetComment())) {
        tooltip.AddRow("Comment:", m_Feat->GetComment());
    }

    curr_text.clear();

    // Truncate the title if it is too long
    CLabel::TruncateLabel(&t_title, 30, 20);

    // Location-related information
    x_AddLocationRows(tooltip);

    // add portion specific to a specific sequence position
    // such as current mouse position
    x_AddPositionRows(tooltip, at_p);

    // add qual fields if any exist
    if (m_Feat->IsSetQual()) {
        bool isAnyFound(false);
        const CSeq_feat::TQual& QualList(m_Feat->GetQual());
        ITERATE(CSeq_feat::TQual, iQual, QualList) {
            if ((*iQual)->IsSetQual() && (*iQual)->IsSetVal()) {
                if (!isAnyFound) {
                    tooltip.AddSectionRow("Qualifiers");
                    isAnyFound = true;
                }
                tooltip.AddRow((*iQual)->GetQual() + ':', (*iQual)->GetVal());
            }
        }
    }
}


void CGuiObjectInfoSeq_feat::x_GetGroupToolTip(ITooltipFormatter& tooltip, string& t_title, TSeqPos at_p) const
{
    CLabel::GetLabel(*m_Feat, &t_title, CLabel::eContent, m_Scope.GetPointer());
    string text(t_title);
    {
        // Remove the subtype from the title
        string to_remove(GetSubtype());
        to_remove += ": ";
        size_t pos = text.find(to_remove);
        if ((pos != string::npos) && (to_remove.length() < text.length()))
            text = text.substr(to_remove.length());
    }
    tooltip.AddRow(GetSubtype() + ':', text);

    const CSeq_feat::TData& data = m_Feat->GetData();
    string curr_text;

    if (CSeqUtils::IsPseudoFeature(*m_Feat)) {
        curr_text += "Pseudo";
    }
    if (CSeqUtils::IsPartialFeature(*m_Feat)) {
        curr_text += curr_text.empty() ? "" : ", ";
        curr_text += "Partial";
    }
    if (CSeqUtils::IsPartialStart(m_Feat->GetLocation())) {
        curr_text += curr_text.empty() ? "" : ", ";
        curr_text += "Partial start";
    }
    if (CSeqUtils::IsPartialStop(m_Feat->GetLocation())) {
        curr_text += curr_text.empty() ? "" : ", ";
        curr_text += "Partial stop";
    }
    if (CSeqUtils::IsException(*m_Feat) && m_Feat->IsSetExcept_text()) {
        curr_text += curr_text.empty() ? "" : ", ";
        curr_text += m_Feat->GetExcept_text();
    }
    if (!curr_text.empty()) {
        tooltip.AddRow("Qualifiers:", curr_text);
        curr_text.clear();
    }

    // Brief description goes here
    if (data.IsSeq()) {
        CLabel::GetLabel(*m_Feat, &curr_text, CLabel::eDescription, m_Scope.GetPointer());
    }
    else if (data.IsGene() && data.GetGene().IsSetDesc()) {
        curr_text += data.GetGene().GetDesc();
    }
    else {
        feature::GetLabel(*m_Feat, &curr_text, feature::fFGL_Content | feature::fFGL_NoQualifiers, m_Scope.GetPointer());
    }
    if (curr_text != t_title) {
        // avoid showing the duplicated information
        tooltip.AddRow("Title:", curr_text);
        curr_text.clear();
    }
    curr_text.clear();
    string merged_feats;
    // Add mRNA/CDS titles
    for (const auto &feat : m_FeaturesGroup) {
        feature::GetLabel(*feat.first, &curr_text, feature::fFGL_Content | feature::fFGL_NoQualifiers, m_Scope.GetPointer());

        string feat_label;
        CLabel::GetLabel(*feat.first, &feat_label, CLabel::eContent, m_Scope);
        if (merged_feats.empty()) {
            merged_feats = feat_label;
        }
        else {
            merged_feats += " and " + feat_label; 
        }

        if (curr_text.empty() || !feat.first->IsSetData())
            continue;
        if (feat.first->GetData().IsCdregion()) {
            tooltip.AddRow("Protein title:", curr_text);
            curr_text.clear();
            continue;
        }
        if (feat.first->GetData().IsRna()) {
            tooltip.AddRow("RNA title:", curr_text);
            curr_text.clear();
            continue;
        }
    }
    if (!merged_feats.empty()) {
        tooltip.AddRow("Merged features:", merged_feats);
    }
    
    // Truncate the title if it is too long
    CLabel::TruncateLabel(&t_title, 30, 20);

    // Location-related information
    x_AddGroupLocationRows(tooltip);

    // Add product information
    for (const auto &feat : m_FeaturesGroup) {
        SConstScopedObject scoped_obj(feat.first, m_Scope);

        CIRef<IGuiObjectInfo> gui_info(CreateObjectInterface<IGuiObjectInfo>(scoped_obj, NULL));
        if (!gui_info)
            continue;

        CGuiObjectInfoSeq_feat* gui_info_feat = dynamic_cast<CGuiObjectInfoSeq_feat*>(gui_info.GetPointer());
        if (nullptr == gui_info_feat)
            continue;

        gui_info_feat->SetMappingInfo(feat.second);

        CIRef<ITooltipFormatter> extra_info = tooltip.CreateInstance();
        gui_info_feat->x_AddExtraInfo(*extra_info, at_p);
        if (!extra_info->IsEmpty()) {
            string feat_label;
            CLabel::GetLabel(*feat.first, &feat_label, CLabel::eContent, m_Scope);
            tooltip.AddSectionRow(feat_label);
            tooltip.Append(*extra_info);
        }
    }
}


void CGuiObjectInfoSeq_feat::x_AddPositionRows(ITooltipFormatter& tooltip, TSeqPos at_p) const
{
    if (at_p != -1) {
        CIRef<ITooltipFormatter> extra_info = tooltip.CreateInstance();
        x_AddExtraInfo(*extra_info, at_p);
        tooltip.AddSectionRow("Positional Info");
        string master_seq_id{ "Position on master:" };
        string assm_acc = x_GetAccession();
        if (!assm_acc.empty()) {
            master_seq_id = assm_acc + " position:";
        }
        tooltip.AddRow(master_seq_id, NStr::UIntToString(at_p + 1, NStr::fWithCommas));
        TSignedSeqPos feat_pos = x_ToFeature(at_p);
        if ((feat_pos != -1) && (feat_pos != at_p) && !m_Feat->IsSetProduct() && !m_Feat->GetData().IsCdregion()) {
            if (m_Feat->GetData().IsRna()) {
                tooltip.AddRow(x_GetProductLabel() + " position:", NStr::IntToString(feat_pos + 1, NStr::fWithCommas));
            }
            else if (m_Feat->GetData().IsGene()) {
                tooltip.AddRow("Gene position:", NStr::IntToString(feat_pos + 1, NStr::fWithCommas));
            }
            else {
            tooltip.AddRow("Position:", NStr::IntToString(feat_pos + 1, NStr::fWithCommas));
        }
        }
        tooltip.Append(*extra_info);
    }
}


static const int kSeqTooltipLength = 15;

static
SAnnotSelector s_GetAnnotSelector(CSeqFeatData::ESubtype subtype, const CMappedFeat& feat) 
{
    SAnnotSelector sel = CSeqUtils::GetAnnotSelector(subtype);
    CSeq_annot_Handle annot = feat.GetAnnot();
    string annot_str;
    if (annot && annot.IsNamed()) {
        annot_str = annot.GetName();
        sel.AddNamedAnnots(annot_str);
        if (NStr::StartsWith(annot_str, "NA0")) {
            sel.IncludeNamedAnnotAccession(annot_str);
        }
    }
    return sel;
}


void CGuiObjectInfoSeq_feat::x_AddExtraInfo(ITooltipFormatter& tooltip, TSeqPos at_p) const
{
    if ( !m_Location ) return;

    CSeqFeatData::E_Choice type = m_Feat->GetData().Which();
    CSeqFeatData::ESubtype subtype = m_Feat->GetData().GetSubtype();
    CConstRef<CSeq_loc> exon_loc;
    if (type == CSeqFeatData::e_Cdregion && m_MappedFeat) {
        auto sel = s_GetAnnotSelector(CSeqFeatData::eSubtype_mRNA, m_MappedFeat);
//        CMappedFeat mrna = feature::GetBestMrnaForCds(m_MappedFeat, nullptr, &sel);
//        if (mrna) 
//            exon_loc.Reset(&mrna.GetLocation());
        sel.IncludeFeatSubtype(CSeqFeatData::eSubtype_cdregion);
        sel.IncludeFeatSubtype(CSeqFeatData::eSubtype_mRNA);
        sel.IncludeFeatSubtype(CSeqFeatData::eSubtype_C_region);
        sel.IncludeFeatSubtype(CSeqFeatData::eSubtype_V_segment);
        sel.IncludeFeatSubtype(CSeqFeatData::eSubtype_D_segment);
        sel.IncludeFeatSubtype(CSeqFeatData::eSubtype_J_segment);
        sel.SetResolveAll().SetAdaptiveDepth().SetOverlapTotalRange();
     
        feature::CFeatTree tree;
        CFeat_CI feat_it(*m_Scope, *m_Location, sel);
        tree.AddFeatures(feat_it);
        CMappedFeat parent = tree.GetParent(m_MappedFeat);
        if (parent) 
            exon_loc.Reset(&parent.GetLocation());

    
    } else if (type == CSeqFeatData::e_Rna 
        || subtype == CSeqFeatData::eSubtype_C_region
        || subtype == CSeqFeatData::eSubtype_V_segment
        || subtype == CSeqFeatData::eSubtype_D_segment
        || subtype == CSeqFeatData::eSubtype_J_segment) {
        exon_loc.Reset(&*m_Location);    
    }
    if (exon_loc) {
        CSeq_loc_CI exon_loc_ci(*exon_loc, CSeq_loc_CI::eEmpty_Skip, CSeq_loc_CI::eOrder_Biological);
        int count = 0, prev_to = -1;
        for (; exon_loc_ci; ++exon_loc_ci) {
            const auto&& e = exon_loc_ci.GetRange();
            bool minus = exon_loc_ci.GetStrand() == eNa_strand_minus;
            ++count;
            if (at_p >= e.GetFrom() && at_p <= e.GetTo()) {
                tooltip.AddRow("Exon:", NStr::UIntToString(count, NStr::fWithCommas) + " of " + NStr::NumericToString(exon_loc_ci.GetSize(), NStr::fWithCommas));
                break;
            } else if (prev_to != -1) {
                bool intron = false;
                if (minus) {
                    intron = at_p > e.GetTo() && at_p < (TSeqPos)prev_to;    
                } else {
                    intron = at_p > (TSeqPos)prev_to && at_p < e.GetFrom();
                }
                if (intron) {
                    tooltip.AddRow("Intron:", NStr::UIntToString(count - 1, NStr::fWithCommas) + " of " + NStr::NumericToString(exon_loc_ci.GetSize() - 1, NStr::fWithCommas));
                    break;
                }
            }    
            prev_to = minus ? e.GetFrom() : e.GetTo();
        }        
    }


    if (type == CSeqFeatData::e_Rna  ||  type == CSeqFeatData::e_Cdregion) {
        SFeatSeg seg = x_GetHitSeg(at_p);
        if (seg.m_Type != SFeatSeg::eType_OutOfBound) {
            //x_GetSplicedSeqTitle(seg, at_p, tooltip);
            if (m_Feat->IsSetProduct() || (CSeqFeatData::eSubtype_cdregion == m_Feat->GetData().GetSubtype())) {
                x_GetProductSequence(seg, at_p, tooltip);
            }
        }
    }
}


void CGuiObjectInfoSeq_feat::x_GetLocLengthRow(const CSeq_loc &loc, const CSeq_feat &feat, TLens &lens, const string &len_suffix) const
{
    if (!loc.IsInt() && !loc.IsPnt()) {
        try {
            TSeqPos len = sequence::GetLength(loc, m_Scope.GetPointer());
            if (feat.GetData().IsCdregion()) {
                lens.push_back(TLens::value_type("CDS length", NStr::IntToString(len, NStr::fWithCommas) + len_suffix));
            }
            else {
                lens.push_back(TLens::value_type("Aligned length", NStr::IntToString(len, NStr::fWithCommas) + len_suffix));
            }
        }
        catch (const CException&) {
        }
    }
}


void CGuiObjectInfoSeq_feat::x_GetProductLengthRow(const CSeq_feat &feat, TLens &lens) const
{
    if (feat.IsSetProduct()) {
        try {
            CBioseq_Handle h = m_Scope->GetBioseqHandle(*feat.GetProduct().GetId());

            if (h) {
                string product = feat.GetData().IsRna() ? "Sequence length" : "Protein length";
                lens.push_back(TLens::value_type(product, NStr::IntToString(h.GetBioseqLength(), NStr::fWithCommas) + (h.IsAa() ? " aa" : " nt")));
            }
        }
        catch (const CException&) {
        }
    }
    else {
        string product_len = feat.GetNamedQual("product_length");
        if (!product_len.empty()) {
            try {
                string product = feat.GetData().IsRna() ? "Sequence length" : "Protein length";
                product_len += feat.GetData().IsRna() ? " nt" : " aa";
                lens.push_back(TLens::value_type(product, product_len));
            }
            catch (const CException&) {
            }
        }
    }
}


CConstRef<CSeq_loc> CGuiObjectInfoSeq_feat::x_GetFeatLocation(const CSeq_feat& feat, const CBioseq_Handle& handle) const
{
    const CSeq_feat_Base::TLocation& loc_obj = feat.GetLocation();
    CConstRef<CSeq_loc> loc;
    /// guard against the cases where the feature contains location
    /// with multiple seq-ids
    if (!loc_obj.GetId()) {
        loc = CSeqUtils::MixLocToLoc(loc_obj, handle);
    }
    if (!loc) {
        loc.Reset(&loc_obj);
    }
    return loc;
}


CGuiObjectInfoSeq_feat::SFeatSeg
CGuiObjectInfoSeq_feat::x_GetHitSeg(TSeqPos pos) const
{
    SFeatSeg seg;
    int pre_dir = 0;      // Previous segment strand (0:unknow, 1:forward, -1:reverse)
    TSeqPos pre_pos = -1; // Previous segment end position close to the current segment
    TSeqPos prod_start = 0; // Current product start pos (in nucleotide bases)

    for (CSeq_loc_CI iter(*m_Location);  iter;  ++iter) {
        CSeq_loc_CI::TRange range = iter.GetRange();
        int curr_dir = 1;
        if (iter.GetStrand() == eNa_strand_minus) {
            curr_dir = -1;
        }

        // check if it hits an exon
        if (pos >= range.GetFrom()  &&  pos <= range.GetTo()) {
            seg.m_Type = SFeatSeg::eType_Exon;
            seg.m_CrossOrigin = false;
            seg.m_Range = range;
            seg.m_ProdRange.Set(prod_start, prod_start + range.GetLength() - 1);
            seg.m_ExonDirForward = curr_dir > 0;
            break;
        }

        // initialize current segment strand
        if (pre_pos != -1) {
            // check if it hits an intron
            TSeqPos curr_pos = curr_dir > 0 ? range.GetFrom() : range.GetTo();

            if (pre_dir > 0  &&  curr_pos > pre_pos) {
                if (pos > pre_pos  &&  pos < curr_pos  &&
                    (seg.m_Range.Empty()  ||  seg.m_Range.GetLength() > curr_pos - pre_pos + 1)) {
                    seg.m_Range.Set(pre_pos, curr_pos);
                    seg.m_Type = SFeatSeg::eType_Intron;
                    seg.m_CrossOrigin = false;
                    seg.m_ProdRange.Set(prod_start - 1, prod_start);
                    seg.m_ExonDirForward = pre_dir > 0;
                }
            } else if (pre_dir < 0  &&  curr_pos < pre_pos) {
                if (pos < pre_pos  &&  pos > curr_pos  &&
                    (seg.m_Range.Empty()  ||  seg.m_Range.GetLength() > pre_pos - curr_pos + 1)) {
                    seg.m_Range.Set(curr_pos, pre_pos);
                    seg.m_Type = SFeatSeg::eType_Intron;
                    seg.m_CrossOrigin = false;
                    seg.m_ProdRange.Set(prod_start - 1, prod_start);
                    seg.m_ExonDirForward = pre_dir > 0;
                }
            } else {
                // The intron must cross the origin
                // It is intentional to make the 'from' > 'to' for an intron span
                // crossing the origin.
                if (curr_pos > pre_pos  &&  (pos > curr_pos || pos < pre_pos)) {
                    seg.m_Range.Set(curr_pos, pre_pos);
                    seg.m_Type = SFeatSeg::eType_Intron;
                    seg.m_CrossOrigin = true;
                    seg.m_ProdRange.Set(prod_start - 1, prod_start);
                    seg.m_ExonDirForward = pre_dir > 0;
                } else if (curr_pos < pre_pos  &&  (pos > pre_pos || pos < curr_pos)) {
                    seg.m_Range.Set(pre_pos, curr_pos);
                    seg.m_Type = SFeatSeg::eType_Intron;
                    seg.m_CrossOrigin = true;
                    seg.m_ProdRange.Set(prod_start - 1, prod_start);
                    seg.m_ExonDirForward = pre_dir > 0;
                }
            }
        }

        // backup current segmnet attributes
        pre_pos = curr_dir > 0 ? range.GetTo() : range.GetFrom();
        prod_start += range.GetLength();
        pre_dir = curr_dir;
    }

    return seg;
}


// This is exactly as it done in Sequin.
TSeqPos CGuiObjectInfoSeq_feat::x_GetProtOffset() const
{
    TSeqPos offset = 0; // translation offset
    if (m_Feat->IsSetData() && m_Feat->GetData().GetSubtype() == CSeqFeatData::eSubtype_cdregion) {
        const CCdregion& cdr = m_Feat->GetData().GetCdregion();
        if (cdr.IsSetFrame ()) {
            switch (cdr.GetFrame ()) {
            case CCdregion::eFrame_two :
                offset = 1;
                break;
            case CCdregion::eFrame_three :
                offset = 2;
                break;
            default :
                break;
            }
        }
    }
    return offset;
}


TSignedSeqPos CGuiObjectInfoSeq_feat::x_ToFeature(TSeqPos pos) const
{
    if (m_MappingInfo.empty() && (m_Feat->GetData().GetSubtype() != CSeqFeatData::eSubtype_gene)) {
        return -1;
    }
    if (!m_MappingInfo.empty()) {
        for (const auto& mi : m_MappingInfo) {
            const auto& prod_int = *mi.first;
            const auto& gen_int = *mi.second;
            if (pos < gen_int.GetFrom() || pos > gen_int.GetTo())
                continue;
            auto p = gen_int.GetStrand() == eNa_strand_minus ? gen_int.GetTo() - pos : pos - gen_int.GetFrom();
            return (TSignedSeqPos)(prod_int.GetFrom() + p);
        }
    }
    else if (m_Location) {
        if (m_Location->IsReverseStrand()) {
            pos = m_Location->GetTotalRange().GetTo() - pos;
        }
        else {
            pos = pos - m_Location->GetTotalRange().GetFrom();
        }
    }
    return (TSignedSeqPos)(pos);
}


TSignedSeqPos CGuiObjectInfoSeq_feat::x_ToProduct(TSeqPos pos) const
{
    TSignedSeqPos prod_pos = x_ToFeature(pos);
    if (-1 == prod_pos)
        return -1;
    TSeqPos offset = x_GetProtOffset();  // initial translation offset
    return (prod_pos - offset) / 3;
}


void CGuiObjectInfoSeq_feat::x_GetSplicedSeqTitle(const SFeatSeg& seg,
                                                  TSeqPos at_p,
                                                  ITooltipFormatter& tooltip) const
{
    CBioseq_Handle top_handle = m_Scope->GetBioseqHandle(*m_Location->GetId());
    if ( !top_handle ) {
        return;
    }
    CSeqVector vec = top_handle.GetSeqVector(CBioseq_Handle::eCoding_Iupac);
    const TSeqPos kSeqExonLength   = 10;
    const TSeqPos kSeqIntronLength = 8;

    if (seg.m_Type == SFeatSeg::eType_Intron) {
        // add detailed title for intron
        try {
            string value;
            const TSeqRange& intr = seg.m_Range;
            TSeqPos left_stop  = intr.GetFrom() + 1;
            TSeqPos left_start  = (TSeqPos) (left_stop > kSeqExonLength ?
                left_stop - kSeqExonLength : 0);;
            string seq_left, seq_ins_l, seq_ins_r, seq_right;

            vec.GetSeqData(left_start, left_stop, seq_left);
            TSeqPos right_start = intr.GetTo();

            value = seq_left + "] ";
            if (seg.m_CrossOrigin) {
                vec.GetSeqData(left_stop, left_stop + kSeqIntronLength, seq_ins_l);
                value += seq_ins_l;
                if (vec.size() > left_stop + kSeqIntronLength) {
                    value += "...";
                }
                value += "[origin]";
                if (right_start < kSeqIntronLength) {
                    vec.GetSeqData(0, right_start, seq_ins_r);
                } else {
                    value += "...";
                    vec.GetSeqData(right_start - kSeqIntronLength, right_start, seq_ins_r);
                }
                value += seq_ins_r;
            } else if (intr.GetLength() > kSeqIntronLength * 2) {
                vec.GetSeqData(left_stop, left_stop + kSeqIntronLength, seq_ins_l);
                vec.GetSeqData(right_start - kSeqIntronLength, right_start, seq_ins_r);
                value += seq_ins_l + "..." + seq_ins_r;
            } else {
                vec.GetSeqData(left_stop, right_start + 1, seq_ins_l);
                value += seq_ins_l;
            }
            vec.GetSeqData(right_start, right_start + kSeqExonLength, seq_right);
            value += " [" + seq_right;
            tooltip.AddRow("Spliced sequence:", value);
        }
        catch (CException&) {
            /// ignore exceptions
            tooltip.AddRow("Spliced sequence:", "Not available");
        }
    } else if (seg.m_Type == SFeatSeg::eType_Exon) {
        string value;
        TSeqPos left  = (TSeqPos) (at_p > kSeqIntronLength ? at_p - kSeqIntronLength : 0);
        TSeqPos right = (TSeqPos) (at_p + kSeqIntronLength + 1);

        int pos = at_p - left;
        vec.GetSeqData(left, right, value);
        if (!seg.m_ExonDirForward) {
            string rev;
            CSeqManip::ReverseComplement(value, CSeqUtil::e_Iupacna, 0, value.length(), rev);
            swap(rev, value);
        }
        value.insert(pos + 1, "]");
        value.insert(pos, "[");

        tooltip.AddRow("Sequence:", value);
    }
}



void CGuiObjectInfoSeq_feat::x_GetProductSequence(const SFeatSeg& seg,
                                                  TSeqPos at_p,
                                                  ITooltipFormatter& tooltip) const
{
    _ASSERT(m_Feat->IsSetProduct() || (CSeqFeatData::eSubtype_cdregion == m_Feat->GetData().GetSubtype()));
    try {
        bool isCDS{ CSeqFeatData::eSubtype_cdregion == m_Feat->GetData().GetSubtype() };
        TSeqPos base{ 1 };
        CRef<CSeqVector> prod_vec;
        if (m_Feat->IsSetProduct()) {
            const CSeq_loc& product = m_Feat->GetProduct();
            prod_vec.Reset(new CSeqVector(product, *m_Scope, CBioseq_Handle::eCoding_Iupac));
        } else {
            prod_vec.Reset(new CSeqVector(m_Feat->GetLocation(), *m_Scope, CBioseq_Handle::eCoding_Iupac));
            base = 3;
        }
        auto orig_pos = at_p;
        // are we in protein product gap?
        bool in_prod_gap{ false };

        // the easiest way to find out if we are in the protein sequence
        // gap is to compare prot. sequence positions on both sides of the
        // inron. If they are diffenet, we must be in the gap.
        // Otherwise it's not the gap

        TSignedSeqPos prod_pos = 0;

        TSignedSeqPos prod_pos1 = isCDS ? x_ToProduct(seg.m_ProdRange.GetFrom()) : x_ToFeature(seg.m_ProdRange.GetFrom());
        TSignedSeqPos prod_pos2 = isCDS ? x_ToProduct(seg.m_ProdRange.GetTo()) : x_ToFeature(seg.m_ProdRange.GetTo());
        const TSeqRange& intr = seg.m_Range;

        // first, if at_p is in the intron, move it to one end of the nearby exon
        if (seg.m_Type == SFeatSeg::eType_Intron) {
            if (prod_pos1 != prod_pos2) {
                in_prod_gap = true;

                bool at_from_end = false;
                if ((seg.m_CrossOrigin  &&  at_p > intr.GetFrom())  ||
                    (!seg.m_CrossOrigin  &&  at_p - intr.GetFrom() < intr.GetTo() - at_p)) {
                        at_from_end = true;
                }

                if (at_from_end == seg.m_ExonDirForward) {
                    prod_pos = prod_pos1;
                } else {
                    prod_pos = prod_pos2;
                }
            } else {
                prod_pos = prod_pos1;
            }
        } else { // in an exon
            // the real position in protein sequence
            prod_pos = isCDS ? x_ToProduct(orig_pos) : x_ToFeature(orig_pos);

            if (seg.m_ExonDirForward) {
                at_p = seg.m_ProdRange.GetFrom() + at_p - intr.GetFrom();
            } else {
                at_p = seg.m_ProdRange.GetFrom() + intr.GetTo() - at_p;
            }
        }
        if (-1 == prod_pos)
            return;

        TSeqPos from = prod_pos > kSeqTooltipLength ? prod_pos - kSeqTooltipLength : 0;
        from *= base;
        TSeqPos to = min(prod_pos + kSeqTooltipLength, int(prod_vec->size()));
        to *= base;

        if (seg.m_Type != SFeatSeg::eType_Intron) {
            if (!m_MappingInfo.empty()) {
                CSeqUtils::TMappingInfo mapping_info;
                const CSeqUtils::TMappingInfo* mapping_info_ptr = nullptr;
                if (isCDS) {
                    mapping_info_ptr = &m_MappingInfo;
                } else if (m_Feat->GetData().GetSubtype() == CSeqFeatData::eSubtype_mRNA) {
                    if (m_MappedFeat) {
                        SAnnotSelector sel = CSeqUtils::GetAnnotSelector(CSeqFeatData::eSubtype_cdregion);
                        CSeq_annot_Handle annot = m_MappedFeat.GetAnnot();
                        string annot_str;
                        if (annot && annot.IsNamed()) {
                            annot_str = annot.GetName();
                            sel.AddNamedAnnots(annot_str);
                            if (NStr::StartsWith(annot_str, "NA0")) {
                                sel.IncludeNamedAnnotAccession(annot_str);
                            }
                        }
                        // The following should have been used instead (and the class would not need m_MappedFeat member)
                        // and AnnotSelector would not have been needed as well
                        // but tookit has this function undefined CXX-11019
                        // CConstRef<CSeq_feat> cds_feat = sequence::GetBestCdsForMrna(*m_Feat, annot.GetTSE_Handle());

                        CMappedFeat cds_feat = feature::GetBestCdsForMrna(m_MappedFeat, nullptr, &sel);
                        if (cds_feat) {
                            CBioseq_Handle bsh = m_Scope->GetBioseqHandle(*m_Location);
                            CSeqUtils::GetMappingInfo(cds_feat, bsh, mapping_info, annot_str);
                            mapping_info_ptr = &mapping_info;
                        }
                    }
                }
                if (mapping_info_ptr != nullptr) {
                    for (const auto& mi : *mapping_info_ptr) {
                        const auto& prod_int = *mi.first;
                        const auto& gen_int = *mi.second;
                        if (orig_pos < gen_int.GetFrom() || orig_pos > gen_int.GetTo())
                            continue;
                        auto p = gen_int.GetStrand() == eNa_strand_minus ? gen_int.GetTo() - orig_pos : orig_pos - gen_int.GetFrom();
                        p += prod_int.GetFrom();
                        tooltip.AddRow("CDS position:", NStr::UIntToString(p + 1, NStr::fWithCommas));
                        break;
                    }
                }
            }
            tooltip.AddRow(x_GetProductLabel() + " position:", NStr::UIntToString(prod_pos + 1, NStr::fWithCommas));
        }

        string value;
        string trans_seq;
        const CGenetic_code* genetic_code{ nullptr };
        if (isCDS && m_Feat->GetData().GetCdregion().IsSetCode()) {
            genetic_code =  &(m_Feat->GetData().GetCdregion().GetCode());
        }
        string prod_seq;

        prod_vec->GetSeqData(from, prod_pos * base, prod_seq);
        if (base == 3) {
            CSeqTranslator::Translate(prod_seq, trans_seq, CSeqTranslator::fIs5PrimePartial, genetic_code);
            value += trans_seq;
        }
        else {
            value += prod_seq;
        }
        prod_vec->GetSeqData(prod_pos * base, prod_pos * base + base, prod_seq);
        string first_bracket = "[";
        string second_bracket = "]";

        if (base == 3) {
            CSeqTranslator::Translate(prod_seq, trans_seq, CSeqTranslator::fIs5PrimePartial, genetic_code);
            prod_seq = trans_seq;
        }

        if (seg.m_Type == SFeatSeg::eType_Intron) {
            if (in_prod_gap) {
                if (prod_pos == prod_pos1) {
                    value += prod_seq + first_bracket + "..." + second_bracket;
            }
                else {
                    value += first_bracket + "..." + second_bracket + prod_seq;
                }
            }
            else {
                value += first_bracket + prod_seq + "..." + prod_seq + second_bracket;
            }
        }
        else {
            value += first_bracket + prod_seq + second_bracket;
        }

        prod_vec->GetSeqData(prod_pos * base + base, to, prod_seq);
        if (base == 3) {
            CSeqTranslator::Translate(prod_seq, trans_seq, CSeqTranslator::fIs5PrimePartial, genetic_code);
            value += trans_seq;
        }
        else {
            value += prod_seq;
        }

        tooltip.AddRow(x_GetProductLabel() + " sequence:", value);
    } catch (CException&) {
    }
}


string CGuiObjectInfoSeq_feat::x_GetProductLabel() const
{
    switch (m_Feat->GetData().GetSubtype()) {
    case CSeqFeatData::eSubtype_cdregion:
        if (CSeqUtils::IsPseudoFeature(*m_Feat))
            return "Pseudo protein";
        else
            return "Protein";
    case CSeqFeatData::eSubtype_mRNA:
        return "mRNA";
    case CSeqFeatData::eSubtype_preRNA:
    case CSeqFeatData::eSubtype_tRNA:
    case CSeqFeatData::eSubtype_rRNA:
    case CSeqFeatData::eSubtype_snRNA:
    case CSeqFeatData::eSubtype_scRNA:
    case CSeqFeatData::eSubtype_snoRNA:
    case CSeqFeatData::eSubtype_otherRNA:
    case CSeqFeatData::eSubtype_ncRNA:
    case CSeqFeatData::eSubtype_tmRNA:
        return "RNA";
    default:
        return "Product";
    }
}

inline string CGuiObjectInfoSeq_feat::x_GetAccession() const
{
    if ((nullptr != m_Location->GetId()) && (nullptr != m_Scope)) {
        return sequence::GetAccessionForId(*m_Location->GetId(), *m_Scope);
    }
    return string();
}

END_NCBI_SCOPE
