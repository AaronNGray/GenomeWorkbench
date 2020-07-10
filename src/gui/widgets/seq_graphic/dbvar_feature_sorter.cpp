/*  $Id:
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
* Authors:  Liangshou Wu
*
*/

#include <ncbi_pch.hpp>
#include <gui/widgets/seq_graphic/dbvar_feature_sorter.hpp>
#include <gui/utils/track_info.hpp>

#include <objects/seqfeat/Variation_ref.hpp>
#include <objects/seqfeat/Phenotype.hpp>
#include <objects/seqfeat/Seq_feat.hpp>
#include <objects/general/User_object.hpp>
#include <objects/general/User_field.hpp>
#include <objects/general/Object_id.hpp>
#include <objmgr/mapped_feat.hpp>

#include <util/static_map.hpp>

BEGIN_NCBI_SCOPE


///////////////////////////////////////////////////////////////////////////////
/// CDbvarQualitySorter implementation

/// sort group index to group name
typedef SStaticPair<CDbvarQualitySorter::EGroup, const char*> TQuaGroupNameStr;
static const TQuaGroupNameStr s_QuaGroupNameStrs[] = {
    { CDbvarQualitySorter::eHigh, "Variant Quality: High" },
    { CDbvarQualitySorter::eLow, "Variant Quality: Low" },
};


typedef CStaticArrayMap<CDbvarQualitySorter::EGroup, string> TQuaGroupNameMap;
DEFINE_STATIC_ARRAY_MAP(TQuaGroupNameMap, sm_QuaGroupNameStrs, s_QuaGroupNameStrs);

static const string& s_QuaGroupIdxToName(int idx)
{
    TQuaGroupNameMap::const_iterator iter =
        sm_QuaGroupNameStrs.find((CDbvarQualitySorter::EGroup)idx);
    if (iter != sm_QuaGroupNameStrs.end()) {
        return iter->second;
    }
    return kEmptyStr;
}


CDbvarQualitySorter::CDbvarQualitySorter(const string& sort_str)
{
    vector<string> tokens;
    CTrackUtils::TokenizeWithEscape(sort_str, "|", tokens, true);
    size_t num_tokens = tokens.size();

    // sort clones belonging to an explicit list of groups
    for (size_t i = 0; i < num_tokens; ++i) {
        if (NStr::EqualNocase(NStr::TruncateSpaces(tokens[i]), "high")) {
            m_SortGroups.insert(eHigh);
        } else if (NStr::EqualNocase(NStr::TruncateSpaces(tokens[i]), "low")) {
            m_SortGroups.insert(eLow);
        }
    }

    if (m_SortGroups.empty()) {
        // sort all variants
        m_SortGroups.insert(eHigh);
        m_SortGroups.insert(eLow);
    }
}


string CDbvarQualitySorter::GroupIdxToName(int idx) const
{
    return s_QuaGroupIdxToName(idx);
}


int CDbvarQualitySorter::GetGroupIdx(const objects::CMappedFeat& feat) const
{
    EGroup idx = eInvalid;
    const objects::CSeq_feat::TData& data = feat.GetOriginalFeature().GetData();
    if (data.IsVariation()) {
        if (NStr::EqualNocase(feat.GetNamedQual("Variant Quality"), "low")) {
            idx = eLow;
        } else {
            idx = eHigh;
        }
    }

    if (m_SortGroups.count(idx) == 0) {
        idx = eInvalid;
    }

    return (int)idx;
}


IObjectSorter::SSorterDescriptor
CDbvarQualitySorter::GetThisDescr() const
{
    return GetSorterDescr();
}


const string& CDbvarQualitySorter::GetID()
{
    static const string kSorterName = "variant_quality";
    return kSorterName;
}


IObjectSorter::SSorterDescriptor
CDbvarQualitySorter::GetSorterDescr()
{
    return SSorterDescriptor(GetID(), "Variant Quality",
        "Sort variants by variant quality");
}

///////////////////////////////////////////////////////////////////////////////
/// CDbvarPilotSorter implementation


/// sort group index to group name
typedef SStaticPair<CDbvarPilotSorter::EGroup, const char*> TPilotGroupNameStr;
static const TPilotGroupNameStr s_PilotGroupNameStrs[] = {
    { CDbvarPilotSorter::ePilot1, "Pilot1" },
    { CDbvarPilotSorter::ePilot2, "Pilot2" },
    { CDbvarPilotSorter::ePilot3, "Pilot3" },
    { CDbvarPilotSorter::ePilot4, "Pilot4" },
    { CDbvarPilotSorter::ePilot5, "Pilot5" },
    { CDbvarPilotSorter::eNotSet, "Pilot Not Set" },
    { CDbvarPilotSorter::eOther, "Others" },
};


typedef CStaticArrayMap<CDbvarPilotSorter::EGroup, string> TPilotGroupNameMap;
DEFINE_STATIC_ARRAY_MAP(TPilotGroupNameMap, sm_PilotGroupNameStrs, s_PilotGroupNameStrs);

static const string& s_PilotGroupIdxToName(int idx)
{
    TPilotGroupNameMap::const_iterator iter =
        sm_PilotGroupNameStrs.find((CDbvarPilotSorter::EGroup)idx);
    if (iter != sm_PilotGroupNameStrs.end()) {
        return iter->second;
    }
    return kEmptyStr;
}


CDbvarPilotSorter::CDbvarPilotSorter(const string& sort_str)
{
    vector<string> tokens;
    CTrackUtils::TokenizeWithEscape(sort_str, "|", tokens, true);
    size_t num_tokens = tokens.size();

    // sort clones belonging to an explicit list of groups
    for (size_t i = 0; i < num_tokens; ++i) {
        if (NStr::EqualNocase(NStr::TruncateSpaces(tokens[i]), "1")) {
            m_SortGroups.insert(ePilot1);
        } else if (NStr::EqualNocase(NStr::TruncateSpaces(tokens[i]), "2")) {
            m_SortGroups.insert(ePilot2);
        } else if (NStr::EqualNocase(NStr::TruncateSpaces(tokens[i]), "3")) {
            m_SortGroups.insert(ePilot3);
        } else if (NStr::EqualNocase(NStr::TruncateSpaces(tokens[i]), "4")) {
            m_SortGroups.insert(ePilot4);
        } else if (NStr::EqualNocase(NStr::TruncateSpaces(tokens[i]), "5")) {
            m_SortGroups.insert(ePilot5);
        } else if (NStr::EqualNocase(NStr::TruncateSpaces(tokens[i]), "notset")) {
            m_SortGroups.insert(eNotSet);
        } else if (NStr::EqualNocase(NStr::TruncateSpaces(tokens[i]), "other")) {
            m_SortGroups.insert(eOther);
        }
    }

    if (m_SortGroups.empty()) {
        // sort all variants
        m_SortGroups.insert(ePilot1);
        m_SortGroups.insert(ePilot2);
        m_SortGroups.insert(ePilot3);
        m_SortGroups.insert(ePilot4);
        m_SortGroups.insert(ePilot5);
        m_SortGroups.insert(eNotSet);
        m_SortGroups.insert(eOther);
    }
}


string CDbvarPilotSorter::GroupIdxToName(int idx) const
{
    return s_PilotGroupIdxToName(idx);
}


int CDbvarPilotSorter::GetGroupIdx(const objects::CMappedFeat& feat) const
{
    EGroup idx = eInvalid;
    const objects::CSeq_feat::TData& data = feat.GetOriginalFeature().GetData();
    if (data.IsVariation()) {
        string value = feat.GetNamedQual("Pilot");
        if (value.empty()) {
            idx = eNotSet;
        } else if (NStr::EqualNocase(value, "1")) {
            idx = ePilot1;
        } else if (NStr::EqualNocase(value, "2")) {
            idx = ePilot2;
        } else if (NStr::EqualNocase(value, "3")) {
            idx = ePilot3;
        } else if (NStr::EqualNocase(value, "4")) {
            idx = ePilot4;
        } else if (NStr::EqualNocase(value, "5")) {
            idx = ePilot5;
        } else {
            idx = eOther;
        }

        if (m_SortGroups.count(idx) == 0) {
            if (m_SortGroups.count(eOther) == 0) {
                idx = eInvalid;
            } else {
                idx = eOther;
            }
        }
    }

    return (int)idx;
}


IObjectSorter::SSorterDescriptor
CDbvarPilotSorter::GetThisDescr() const
{
    return GetSorterDescr();
}


const string& CDbvarPilotSorter::GetID()
{
    static const string kSorterName = "pilot";
    return kSorterName;
}


IObjectSorter::SSorterDescriptor
CDbvarPilotSorter::GetSorterDescr()
{
    return SSorterDescriptor(GetID(), "Pilot",
        "Sort variants by pilot number");
}


///////////////////////////////////////////////////////////////////////////////
/// CDbvarClinicalAsstSorter implementation

/// sort group index to group name
typedef SStaticPair<CDbvarClinicalAsstSorter::EGroup, const char*> TClinGroupNameStr;
static const TClinGroupNameStr s_ClinGroupNameStrs[] = {
    { CDbvarClinicalAsstSorter::eBenign, "Benign" },
    { CDbvarClinicalAsstSorter::ePathogenic, "Pathogenic" },
    { CDbvarClinicalAsstSorter::eLikelyPathogenic, "Likely Pathogenic" },
    { CDbvarClinicalAsstSorter::eLikelyBenign, "Likely Benign" },
    { CDbvarClinicalAsstSorter::eLikelyUnknown, "Likely Unknown" },
    { CDbvarClinicalAsstSorter::eNotTested, "Not Tested" },
    { CDbvarClinicalAsstSorter::eOther, "Other" },
};


typedef CStaticArrayMap<CDbvarClinicalAsstSorter::EGroup, string> TClinGroupNameMap;
DEFINE_STATIC_ARRAY_MAP(TClinGroupNameMap, sm_ClinGroupNameStrs, s_ClinGroupNameStrs);

static const string& s_ClinGroupIdxToName(int idx)
{
    TClinGroupNameMap::const_iterator iter =
        sm_ClinGroupNameStrs.find((CDbvarClinicalAsstSorter::EGroup)idx);
    if (iter != sm_ClinGroupNameStrs.end()) {
        return iter->second;
    }
    return kEmptyStr;
}


CDbvarClinicalAsstSorter::CDbvarClinicalAsstSorter(const string& sort_str)
{
    vector<string> tokens;
    CTrackUtils::TokenizeWithEscape(sort_str, "|", tokens, true);
    size_t num_tokens = tokens.size();

    // sort clones belonging to an explicit list of groups
    for (size_t i = 0; i < num_tokens; ++i) {
        if (NStr::EqualNocase(NStr::TruncateSpaces(tokens[i]), "benign")) {
            m_SortGroups.insert(eBenign);
        } else if (NStr::EqualNocase(NStr::TruncateSpaces(tokens[i]), "pathogenic")) {
            m_SortGroups.insert(ePathogenic);
        } else if (NStr::EqualNocase(NStr::TruncateSpaces(tokens[i]), "likely_pathogenic")) {
            m_SortGroups.insert(eLikelyPathogenic);
        } else if (NStr::EqualNocase(NStr::TruncateSpaces(tokens[i]), "likely_benign")) {
            m_SortGroups.insert(eLikelyBenign);
        } else if (NStr::EqualNocase(NStr::TruncateSpaces(tokens[i]), "likely_unknown")) {
            m_SortGroups.insert(eLikelyUnknown);
        } else if (NStr::EqualNocase(NStr::TruncateSpaces(tokens[i]), "not_tested")) {
            m_SortGroups.insert(eNotTested);
        } else if (NStr::EqualNocase(NStr::TruncateSpaces(tokens[i]), "other")) {
            m_SortGroups.insert(eOther);
        }
    }

    if (m_SortGroups.empty()) {
        // sort all variants
        m_SortGroups.insert(eBenign);
        m_SortGroups.insert(ePathogenic);
        m_SortGroups.insert(eLikelyPathogenic);
        m_SortGroups.insert(eLikelyBenign);
        m_SortGroups.insert(eLikelyUnknown);
        m_SortGroups.insert(eNotTested);
        m_SortGroups.insert(eOther);
    }
}


string CDbvarClinicalAsstSorter::GroupIdxToName(int idx) const
{
    return s_ClinGroupIdxToName(idx);
}


int CDbvarClinicalAsstSorter::GetGroupIdx(const objects::CMappedFeat& feat) const
{
    EGroup idx = eInvalid;
    const objects::CSeq_feat::TData& data = feat.GetOriginalFeature().GetData();
    if (data.IsVariation()) {
        idx = eOther;
        const objects::CVariation_ref& var = feat.GetData().GetVariation();
        ITERATE (objects::CVariation_ref::TPhenotype, pnt_iter, var.GetPhenotype()) {
            if ((*pnt_iter)->CanGetClinical_significance()) {
                switch ((*pnt_iter)->GetClinical_significance()) {
                    case objects::CPhenotype::eClinical_significance_non_pathogenic:
                        idx = eBenign;
                        break;
                    case objects::CPhenotype::eClinical_significance_pathogenic:
                        idx = ePathogenic;
                        break;
                    case objects::CPhenotype::eClinical_significance_probable_pathogenic:
                        idx = eLikelyPathogenic;
                        break;
                    case objects::CPhenotype::eClinical_significance_probable_non_pathogenic:
                        idx = eLikelyBenign;
                        break;
                    case objects::CPhenotype::eClinical_significance_unknown:
                        idx = eLikelyUnknown;
                        break;
                    case objects::CPhenotype::eClinical_significance_untested:
                        idx = eNotTested;
                        break;
                    case objects::CPhenotype::eClinical_significance_other:
                    default:
                        idx = eOther;
                        break;
                }
                break;
            }
        }

        if (m_SortGroups.count(idx) == 0) {
            if (m_SortGroups.count(eOther) == 0) {
                idx = eInvalid;
            } else {
                idx = eOther;
            }
        }
    }

    return (int)idx;
}


IObjectSorter::SSorterDescriptor
CDbvarClinicalAsstSorter::GetThisDescr() const
{
    return GetSorterDescr();
}


const string& CDbvarClinicalAsstSorter::GetID()
{
    static const string kSorterName = "clinical_assertion";
    return kSorterName;
}


IObjectSorter::SSorterDescriptor
CDbvarClinicalAsstSorter::GetSorterDescr()
{
    return SSorterDescriptor(GetID(), "Clinical Assertion",
        "Sort variants by clinical assertion");
}



///////////////////////////////////////////////////////////////////////////////
/// CDbvarSamplesetTypeSorter implementation

/// sort group index to group name
typedef SStaticPair<CDbvarSamplesetTypeSorter::EGroup, const char*> TSampleTypeNameStr;
static const TSampleTypeNameStr s_SampleTypeNameStrs[] = {
    { CDbvarSamplesetTypeSorter::eCase, "Sampleset: Case" },
    { CDbvarSamplesetTypeSorter::eControl, "Sampleset: Control" },
    { CDbvarSamplesetTypeSorter::eNotSet, "Sampleset: Not set" },
    { CDbvarSamplesetTypeSorter::eOther, "Sampleset: Other" },
};


typedef CStaticArrayMap<CDbvarSamplesetTypeSorter::EGroup, string> TSampleTypeNameMap;
DEFINE_STATIC_ARRAY_MAP(TSampleTypeNameMap, sm_SampleTypeNameStrs, s_SampleTypeNameStrs);

static const string& s_SampleTypeIdxToName(int idx)
{
    TSampleTypeNameMap::const_iterator iter =
        sm_SampleTypeNameStrs.find((CDbvarSamplesetTypeSorter::EGroup)idx);
    if (iter != sm_SampleTypeNameStrs.end()) {
        return iter->second;
    }
    return kEmptyStr;
}


CDbvarSamplesetTypeSorter::CDbvarSamplesetTypeSorter(const string& sort_str)
{
    vector<string> tokens;
    CTrackUtils::TokenizeWithEscape(sort_str, "|", tokens, true);
    size_t num_tokens = tokens.size();

    // sort clones belonging to an explicit list of groups
    for (size_t i = 0; i < num_tokens; ++i) {
        if (NStr::EqualNocase(NStr::TruncateSpaces(tokens[i]), "case")) {
            m_SortGroups.insert(eCase);
        } else if (NStr::EqualNocase(NStr::TruncateSpaces(tokens[i]), "control")) {
            m_SortGroups.insert(eControl);
        } else if (NStr::EqualNocase(NStr::TruncateSpaces(tokens[i]), "other")) {
            m_SortGroups.insert(eOther);
        }
    }

    if (m_SortGroups.empty()) {
        // sort all variants
        m_SortGroups.insert(eCase);
        m_SortGroups.insert(eControl);
        m_SortGroups.insert(eOther);
    }
}


string CDbvarSamplesetTypeSorter::GroupIdxToName(int idx) const
{
    return s_SampleTypeIdxToName(idx);
}


int CDbvarSamplesetTypeSorter::GetGroupIdx(const objects::CMappedFeat& feat) const
{
    EGroup idx = eInvalid;
    const objects::CSeq_feat::TData& data = feat.GetOriginalFeature().GetData();
    if (data.IsVariation()) {
        const string& qual = feat.GetNamedQual("sampleset_type");
        if (NStr::EqualNocase(qual, "case")) {
            idx = eCase;
        } else if (NStr::EqualNocase(qual, "control")) {
            idx = eControl;
        } else if (qual.empty()) {
            idx = eNotSet;
        } else {
            idx = eOther;
        }

        if (m_SortGroups.count(idx) == 0) {
            if (m_SortGroups.count(eOther) == 0) {
                idx = eInvalid;
            } else {
                idx = eOther;
            }
        }
    }

    return (int)idx;
}


IObjectSorter::SSorterDescriptor
CDbvarSamplesetTypeSorter::GetThisDescr() const
{
    return GetSorterDescr();
}


const string& CDbvarSamplesetTypeSorter::GetID()
{
    static const string kSorterName = "sampleset_type";
    return kSorterName;
}


IObjectSorter::SSorterDescriptor
CDbvarSamplesetTypeSorter::GetSorterDescr()
{
    return SSorterDescriptor(GetID(), "Sampleset Type",
        "Sort variants by sampleset type");
}



///////////////////////////////////////////////////////////////////////////////
/// CDbvarValidationSorter implementation

/// sort group index to group name
typedef SStaticPair<CDbvarValidationSorter::EGroup, const char*> TValidationTypeNameStr;
static const TValidationTypeNameStr s_ValidationTypeNameStrs[] = {
    { CDbvarValidationSorter::eSameSample_DiffPlatform, "validated in this sample using another platform" },
    { CDbvarValidationSorter::eDiffSample_DiffPlatform, "validated in a different sample using another platform" },
    { CDbvarValidationSorter::eDiffSample_SamePlatform, "seen in other samples from submitting lab using this platform" },
    { CDbvarValidationSorter::eSeenInPlubic, "seen in public data set" },
    { CDbvarValidationSorter::eNovel, "novel" },
    { CDbvarValidationSorter::eNotAssessed, "not assessed" },
    { CDbvarValidationSorter::eOther, "other" },
};


typedef CStaticArrayMap<CDbvarValidationSorter::EGroup, string> TValidationTypeNameMap;
DEFINE_STATIC_ARRAY_MAP(TValidationTypeNameMap, sm_ValidationTypeNameStrs, s_ValidationTypeNameStrs);

static const string& s_ValidationTypeIdxToName(int idx)
{
    TValidationTypeNameMap::const_iterator iter =
        sm_ValidationTypeNameStrs.find((CDbvarValidationSorter::EGroup)idx);
    if (iter != sm_ValidationTypeNameStrs.end()) {
        return iter->second;
    }
    return kEmptyStr;
}


CDbvarValidationSorter::CDbvarValidationSorter(const string& sort_str)
{
    vector<string> tokens;
    CTrackUtils::TokenizeWithEscape(sort_str, "|", tokens, true);
    size_t num_tokens = tokens.size();

    // sort clones belonging to an explicit list of groups
    for (int i = 0; i < num_tokens; ++i) {
        for (int i = eSameSample_DiffPlatform; i <= eOther; ++i) {
            if (NStr::EqualNocase(NStr::TruncateSpaces(tokens[i]), s_ValidationTypeIdxToName(i))) {
                m_SortGroups.insert((EGroup)i);
                break;
            }
        }
    }

    if (m_SortGroups.empty()) {
        // sort all variants
        for (int i = eSameSample_DiffPlatform; i <= eOther; ++i) {
            m_SortGroups.insert((EGroup)i);
        }
    }
}


string CDbvarValidationSorter::GroupIdxToName(int idx) const
{
    return s_ValidationTypeIdxToName(idx);
}


int CDbvarValidationSorter::GetGroupIdx(const objects::CMappedFeat& feat) const
{
    EGroup idx = eInvalid;
    if (feat.IsSetExts()) {
        const objects::CSeq_feat::TExts& exts = feat.GetExts();
        ITERATE (objects::CSeq_feat::TExts, iter, exts) {
            if ( (*iter)->GetType().IsStr()  &&
                NStr::EqualNocase((*iter)->GetType().GetStr(), "Validation")  &&
                 (*iter)->GetFieldRef("Status")  &&
                 (*iter)->GetFieldRef("Status")->GetData().IsStr()) {
                     idx = eOther;
                     for (int i = eSameSample_DiffPlatform; i < eOther; ++i) {
                         if (NStr::EqualNocase((*iter)->GetFieldRef("Status")->GetData().GetStr(),
                             s_ValidationTypeIdxToName(i))) {
                                 idx = (EGroup)i;
                                 break;
                         }
                     }
                     break;
            }
        }
    }


    if (m_SortGroups.count(idx) == 0) {
        if (m_SortGroups.count(eOther) == 0) {
            idx = eInvalid;
        } else {
            idx = eOther;
        }
    }

    return (int)idx;
}


IObjectSorter::SSorterDescriptor
CDbvarValidationSorter::GetThisDescr() const
{
    return GetSorterDescr();
}


const string& CDbvarValidationSorter::GetID()
{
    static const string kSorterName = "validation_status";
    return kSorterName;
}


IObjectSorter::SSorterDescriptor
CDbvarValidationSorter::GetSorterDescr()
{
    return SSorterDescriptor(GetID(), "Validation status",
        "Sort variants by validation status");
}




///////////////////////////////////////////////////////////////////////////////
/// CGRCStatusSorter implementation

/// sort group index to group name
typedef SStaticPair<CGRCStatusSorter::EGroup, const char*> TResolveStatusNameStr;
static const TResolveStatusNameStr s_ResolveStatusNameStrs[] = {
    { CGRCStatusSorter::eResolved, "Resolved" },
    { CGRCStatusSorter::eOther, "Not resolved" },
};


typedef CStaticArrayMap<CGRCStatusSorter::EGroup, string> TResolveStatusNameMap;
DEFINE_STATIC_ARRAY_MAP(TResolveStatusNameMap, sm_ResolveStatusNameStrs, s_ResolveStatusNameStrs);

static const string& s_ResolveStatusIdxToName(int idx)
{
    TResolveStatusNameMap::const_iterator iter =
        sm_ResolveStatusNameStrs.find((CGRCStatusSorter::EGroup)idx);
    if (iter != sm_ResolveStatusNameStrs.end()) {
        return iter->second;
    }
    return kEmptyStr;
}


CGRCStatusSorter::CGRCStatusSorter(const string& sort_str)
{
    vector<string> tokens;
    CTrackUtils::TokenizeWithEscape(sort_str, "|", tokens, true);
    size_t num_tokens = tokens.size();

    // sort features belonging to an explicit list of groups
    for (int i = 0; i < num_tokens; ++i) {
        if (NStr::EqualNocase(NStr::TruncateSpaces(tokens[i]), "Resolved")) {
            m_SortGroups.insert(eResolved);
        } else if (NStr::EqualNocase(NStr::TruncateSpaces(tokens[i]), "Not resolved")) {
            m_SortGroups.insert(eOther);
        }
    }

    if (m_SortGroups.empty()) {
        // sort all variants
        for (int i = eResolved; i <= eOther; ++i) {
            m_SortGroups.insert((EGroup)i);
        }
    }
}


string CGRCStatusSorter::GroupIdxToName(int idx) const
{
    return s_ResolveStatusIdxToName(idx);
}


int CGRCStatusSorter::GetGroupIdx(const objects::CMappedFeat& feat) const
{
    EGroup idx = eOther;
    const string& status = feat.GetNamedQual("status");
    if (NStr::EqualNocase(status, "resolved")) {
        idx = eResolved;
    }

    return (int)idx;
}


IObjectSorter::SSorterDescriptor
CGRCStatusSorter::GetThisDescr() const
{
    return GetSorterDescr();
}


const string& CGRCStatusSorter::GetID()
{
    static const string kSorterName = "grc_status";
    return kSorterName;
}


IObjectSorter::SSorterDescriptor
CGRCStatusSorter::GetSorterDescr()
{
    return SSorterDescriptor(GetID(), "Resolution status",
        "Sort GRC issues by resolution status");
}

END_NCBI_SCOPE

