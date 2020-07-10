/*  $Id: aggregate_feature_track.cpp 44424 2019-12-17 16:14:32Z filippov $
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
* Authors:  Vladislav Evgeniev
*
* File Description: Defines a track, combining a set of features into a single track. The list of features is defined in the registry and identified by the subkey.
*
*/

#include <ncbi_pch.hpp>

#include <gui/widgets/seq_graphic/aggregate_feature_track.hpp>
#include <gui/widgets/seq_graphic/seqgraphic_ds_manager.hpp>
#include <gui/objutils/utils.hpp>
#include <gui/objutils/label.hpp>
#include <gui/objutils/registry.hpp>
#include <vector>

#include "search_utils.hpp"

BEGIN_NCBI_SCOPE
USING_SCOPE(objects);


///////////////////////////////////////////////////////////////////////////////
///   CAggregateFeatureTrack
///////////////////////////////////////////////////////////////////////////////

static const string kTrackName = "Aggregate Features";
static const string kBaseKey = "GBPlugins.SeqGraphicAggregateFeatureTrack";

CTrackTypeInfo CAggregateFeatureTrack::m_TypeInfo("aggregate_feature_track",
    "A track, showing a predefined set of features.");

CAggregateFeatureTrack::CAggregateFeatureTrack(CSGFeatureDS* ds, CRenderingContext* r_cntx)
    : CFeatureTrack(ds, r_cntx)
{
    m_Layout = CFeatureTrack::eLayout_ExpandedLayered;
    m_LinkedFeat = eLFD_Expanded;
    m_FeatSel.ExcludeNamedAnnots("SNP");
    m_FeatSel.ExcludeNamedAnnots("STS");
    m_FeatSel.ExcludeNamedAnnots("CDD");
}

CAggregateFeatureTrack::TTypeVec CAggregateFeatureTrack::GetTypeList(const std::string &subkey)
{
    vector<string> values;
    x_GetSubkeyStringVec(subkey, "FeatTypeList", values);
    TTypeMap types = x_GetFeatTypeMap();
    CAggregateFeatureTrack::TTypeVec result;
    result.reserve(values.size());
    for (auto &it : values) {
        auto type = types.find(NStr::ToLower(it));
        if (types.end() == type)
            continue;

        result.push_back(type->second);
    }
    return result;
}

CAggregateFeatureTrack::TSubtypeVec CAggregateFeatureTrack::GetSubtypeList(const std::string &subkey)
{
    vector<string> values;
    x_GetSubkeyStringVec(subkey, "FeatSubtypeList", values);
    CAggregateFeatureTrack::TSubtypeVec result;
    result.reserve(values.size());
    for (auto &it : values) {
        CSeqFeatData::ESubtype subtype = CSeqFeatData::SubtypeNameToValue(it);
        if (CSeqFeatData::eSubtype_bad == subtype)
            continue;
        result.push_back(subtype);
    }
    return result;
}

vector<string> CAggregateFeatureTrack::GetSubtypeListAsString(const std::string &subkey)
{
    CGuiRegistry& registry = CGuiRegistry::GetInstance();
    CRegistryReadView view;
    string section(kBaseKey);
    section += ".Default";
    section += ".Subkey.";
    section += subkey;
    view = registry.GetReadView(section);
    vector<string> result;
    view.GetStringVec("FeatSubtypeList", result);
    return result;
}

void CAggregateFeatureTrack::GetFeatSubtypes(set<CSeqFeatData::ESubtype> &subtypes) const
{
    for (auto s : m_subtypes)
        subtypes.insert(s);
}

void CAggregateFeatureTrack::SetFeatTypes(const CAggregateFeatureTrack::TTypeVec &types)
{
    for (auto &it : types) {
        m_FeatSel.IncludeFeatType(it);
        s_GetSubtypesForType(m_subtypes, it);
    }
}

void CAggregateFeatureTrack::SetFeatSubtypes(const CAggregateFeatureTrack::TSubtypeVec &subtypes)
{
    for (auto &it : subtypes) {
        m_FeatSel.IncludeFeatSubtype(it);
        m_subtypes.insert(it);
    }
}

const CTrackTypeInfo& CAggregateFeatureTrack::GetTypeInfo() const
{
    return m_TypeInfo;
}


string CAggregateFeatureTrack::GetFullTitle() const
{
    string title = GetTitle();
    if (title.empty()) {
        if (CSeqUtils::IsUnnamed(m_AnnotName)) {
            title = "Aggregate Features";
        }
        else {
            title = m_AnnotName;
        }
    }

    return title;
}

void CAggregateFeatureTrack::x_GetSubkeyStringVec(const string &subkey, const string &field, vector<string> &values)
{
    CGuiRegistry& registry = CGuiRegistry::GetInstance();
    CRegistryReadView view;
    string section(kBaseKey);
    section += ".Default";
    section += ".Subkey.";
    section += subkey;
    view = registry.GetReadView(section);
    view.GetStringVec(field, values);
}

CAggregateFeatureTrack::TTypeMap CAggregateFeatureTrack::x_GetFeatTypeMap()
{
    TTypeMap result;
    result["gene"] = CSeqFeatData::e_Gene;
    result["org"] = CSeqFeatData::e_Org;
    result["cdregion"] = CSeqFeatData::e_Cdregion;
    result["prot"] = CSeqFeatData::e_Prot;
    result["rna"] = CSeqFeatData::e_Rna;
    result["pub"] = CSeqFeatData::e_Pub;
    result["seq"] = CSeqFeatData::e_Seq;
    result["imp"] = CSeqFeatData::e_Imp;
    result["region"] = CSeqFeatData::e_Region;
    result["comment"] = CSeqFeatData::e_Comment;
    result["bond"] = CSeqFeatData::e_Bond;
    result["site"] = CSeqFeatData::e_Site;
    result["rsite"] = CSeqFeatData::e_Rsite;
    result["user"] = CSeqFeatData::e_User;
    result["txinit"] = CSeqFeatData::e_Txinit;
    result["num"] = CSeqFeatData::e_Num;
    result["psec_str"] = CSeqFeatData::e_Psec_str;
    result["non_std_residue"] = CSeqFeatData::e_Non_std_residue;
    result["het"] = CSeqFeatData::e_Het;
    result["biosrc"] = CSeqFeatData::e_Biosrc;
    result["clone"] = CSeqFeatData::e_Clone;
    result["variation"] = CSeqFeatData::e_Variation;
    return result;
}

///////////////////////////////////////////////////////////////////////////////
/// CAggregateFeatureTrackFactory
///////////////////////////////////////////////////////////////////////////////

CAggregateFeatureTrackFactory::CAggregateFeatureTrackFactory()
{
    m_DefaultLayout = CFeatureTrack::eLayout_Default;
    m_DefaultLinkedFeat = ELinkedFeatDisplay::eLFD_Expanded;
}

ILayoutTrackFactory::TTrackMap
CAggregateFeatureTrackFactory::CreateTracks(SConstScopedObject& object,
ISGDataSourceContext* ds_context,
CRenderingContext* r_cntx,
const SExtraParams& params,
const TAnnotMetaDataList& src_annots) const
{
    TTrackMap tracks;
    TAnnotNameTitleMap annots;

    if (!params.m_Annots.empty()) {
        ITERATE(SExtraParams::TAnnots, iter, params.m_Annots) {
            annots.insert(TAnnotNameTitleMap::value_type(*iter, ""));
        }
    }

    CAggregateFeatureTrack::TTypeVec types = CAggregateFeatureTrack::GetTypeList(params.m_Subkey);
    CAggregateFeatureTrack::TSubtypeVec subtypes = CAggregateFeatureTrack::GetSubtypeList(params.m_Subkey);


    ITERATE(TAnnotNameTitleMap, iter, annots) {
        CIRef<ISGDataSource> ds = ds_context->GetDS(
            typeid(CSGFeatureDSType).name(), object);
        CSGFeatureDS* seq_ds = dynamic_cast<CSGFeatureDS*>(ds.GetPointer());
        seq_ds->SetDepth(params.m_Level);
        seq_ds->SetAdaptive(params.m_Adaptive);

        CRef<CAggregateFeatureTrack> feat_track(new CAggregateFeatureTrack(seq_ds, r_cntx));
        feat_track->SetAnnot(iter->first);
        feat_track->SetFeatTypes(types);
        feat_track->SetFeatSubtypes(subtypes);
        if (!iter->second.empty()) {
            feat_track->SetTitle(iter->second);
        }
        seq_ds->SetFilter(params.m_Filter);
        seq_ds->SetSortBy(params.m_SortBy);
        
        // potentially slow operations should not be done during track discovery
        if(!params.m_FastConfig) {
            seq_ds->SetGraphLevels(iter->first);
        }
        tracks[iter->first] = feat_track.GetPointer();
    }

    return tracks;
}

void CAggregateFeatureTrackFactory::GetMatchedAnnots(
    const TAnnotMetaDataList& src_annots,
    const ILayoutTrackFactory::SExtraParams& params,
    TAnnotNameTitleMap& out_annots) const
{
    // Skip named annotations
    ILayoutTrackFactory::GetMatchedAnnots(src_annots, params.m_Annots, params.m_Subkey, "", out_annots);
}

string CAggregateFeatureTrackFactory::GetExtensionIdentifier() const
{
    return CAggregateFeatureTrack::m_TypeInfo.GetId();
}

string CAggregateFeatureTrackFactory::GetExtensionLabel() const
{
    return CAggregateFeatureTrack::m_TypeInfo.GetDescr();
}

END_NCBI_SCOPE
