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
#include <gui/widgets/seq_graphic/alignment_sorter.hpp>
#include <gui/utils/track_info.hpp>

#include <objects/seqalign/Seq_align.hpp>
///#include <objects/seqloc/Na_strand.hpp>
#include <util/static_map.hpp>

BEGIN_NCBI_SCOPE

/// sort group index to group name
typedef SStaticPair<CAlignStrandSorter::EGroup, const char*> TGroupNameStr;
static const TGroupNameStr s_GroupNameStrs[] = {
    { CAlignStrandSorter::eForward, "Forward strand" },
    { CAlignStrandSorter::eReverse, "Reverse strand" },
};


typedef CStaticArrayMap<CAlignStrandSorter::EGroup, string> TGroupNameMap;
DEFINE_STATIC_ARRAY_MAP(TGroupNameMap, sm_GroupNameStrs, s_GroupNameStrs);

static const string& s_GroupIdxToName(int idx)
{
    TGroupNameMap::const_iterator iter =
        sm_GroupNameStrs.find((CAlignStrandSorter::EGroup)idx);
    if (iter != sm_GroupNameStrs.end()) {
        return iter->second;
    }
    return kEmptyStr;
}


CAlignStrandSorter::CAlignStrandSorter(const string& sort_str)
{
    vector<string> tokens;
    CTrackUtils::TokenizeWithEscape(sort_str, "|", tokens, true);
    size_t num_tokens = tokens.size();

    // sort clones belonging to an explicit list of groups
    for (size_t i = 0; i < num_tokens; ++i) {
        if (NStr::EqualNocase(NStr::TruncateSpaces(tokens[i]), "plus")) {
            m_SortGroups.insert(eForward);
        } else if (NStr::EqualNocase(NStr::TruncateSpaces(tokens[i]), "minus")) {
            m_SortGroups.insert(eReverse);
        }
    }

    if (m_SortGroups.empty()) {
        m_SortGroups.insert(eForward);
        m_SortGroups.insert(eReverse);
    }
}


string CAlignStrandSorter::GroupIdxToName(int idx) const
{
    return s_GroupIdxToName(idx);
}


int CAlignStrandSorter::GetGroupIdx(const objects::CSeq_align& align) const
{
    EGroup idx = eForward;
    try {
        if (align.CheckNumRows() == 2) {
            objects::ENa_strand strand1 = align.GetSeqStrand(0);
            objects::ENa_strand strand2 = align.GetSeqStrand(1);
            if (strand1 != strand2) {
                idx = eReverse;
            }
        }
    } catch (CException&) {
        // ingore it
    }

    if (m_SortGroups.count(idx) == 0) {
        idx = eInvalid;
    }
    return (int)idx;
}


IObjectSorter::SSorterDescriptor
CAlignStrandSorter::GetThisDescr() const
{
    return GetSorterDescr();
}


const string& CAlignStrandSorter::GetID()
{
    static const string kSorterName = "strand";
    return kSorterName;
}

IObjectSorter::SSorterDescriptor
CAlignStrandSorter::GetSorterDescr()
{
    return SSorterDescriptor(GetID(), "Alignment strand",
        "Sort alignments by strand");
}



END_NCBI_SCOPE

