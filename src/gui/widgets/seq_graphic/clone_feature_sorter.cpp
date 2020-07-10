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
#include <gui/widgets/seq_graphic/clone_feature_sorter.hpp>
#include <gui/utils/track_info.hpp>

#include <objects/seqfeat/Clone_ref.hpp>
#include <objects/seqfeat/Seq_feat.hpp>
#include <objmgr/mapped_feat.hpp>
#include <util/static_map.hpp>

BEGIN_NCBI_SCOPE

/// sort group index to group name
typedef SStaticPair<CCloneConcordancySorter::EGroup, const char*> TGroupNameStr;
static const TGroupNameStr s_GroupNameStrs[] = {
    { CCloneConcordancySorter::eConcordant, "Concordant" },
    { CCloneConcordancySorter::eDiscordant, "Discordant" },
    { CCloneConcordancySorter::eNotSet,     "Concordancy not set" },
    { CCloneConcordancySorter::eOther,      "Others" },
};


typedef CStaticArrayMap<CCloneConcordancySorter::EGroup, string> TGroupNameMap;
DEFINE_STATIC_ARRAY_MAP(TGroupNameMap, sm_GroupNameStrs, s_GroupNameStrs);

static const string& s_GroupIdxToName(int idx)
{
    TGroupNameMap::const_iterator iter =
        sm_GroupNameStrs.find((CCloneConcordancySorter::EGroup)idx);
    if (iter != sm_GroupNameStrs.end()) {
        return iter->second;
    }
    return kEmptyStr;
}


CCloneConcordancySorter::CCloneConcordancySorter(const string& sort_str)
{
    vector<string> tokens;
    CTrackUtils::TokenizeWithEscape(sort_str, "|", tokens, true);
    size_t num_tokens = tokens.size();

    // sort clones belonging to an explicit list of groups
    for (size_t i = 0; i < num_tokens; ++i) {
        if (NStr::EqualNocase(NStr::TruncateSpaces(tokens[i]), "concordant")) {
            m_SortGroups.insert(eConcordant);
        } else if (NStr::EqualNocase(NStr::TruncateSpaces(tokens[i]), "discordant")) {
            m_SortGroups.insert(eDiscordant);
        } else if (NStr::EqualNocase(NStr::TruncateSpaces(tokens[i]), "notset")) {
            m_SortGroups.insert(eNotSet);
        } else if (NStr::EqualNocase(NStr::TruncateSpaces(tokens[i]), "other")) {
            m_SortGroups.insert(eOther);
        }
    }

    if (m_SortGroups.empty()) {
        // sort all clones
        m_SortGroups.insert(eConcordant);
        m_SortGroups.insert(eDiscordant);
        m_SortGroups.insert(eNotSet);
    }
}


string CCloneConcordancySorter::GroupIdxToName(int idx) const
{
    return s_GroupIdxToName(idx);
}


int CCloneConcordancySorter::GetGroupIdx(const objects::CMappedFeat& feat) const
{
    EGroup idx = eInvalid;
    const objects::CSeq_feat::TData& data = feat.GetOriginalFeature().GetData();
    if (data.IsClone()) {
        if (data.GetClone().IsSetConcordant()) {
            if (data.GetClone().GetConcordant()) {
                idx = eConcordant;
            } else {
                idx = eDiscordant;
            }
        } else {
            idx = eNotSet;
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
CCloneConcordancySorter::GetThisDescr() const
{
    return GetSorterDescr();
}


const string& CCloneConcordancySorter::GetID()
{
    static const string kSorterName = "concordancy";
    return kSorterName;
}

IObjectSorter::SSorterDescriptor
CCloneConcordancySorter::GetSorterDescr()
{
    return SSorterDescriptor(GetID(), "Clone Concordancy",
        "Sort clones by concordancy");
}



END_NCBI_SCOPE

