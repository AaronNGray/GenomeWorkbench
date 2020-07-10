#ifndef GPIPE_COMMON___ALIGN_COMPARE__HPP
#define GPIPE_COMMON___ALIGN_COMPARE__HPP

/*  $Id: align_compare.hpp 501229 2016-05-12 15:45:42Z mozese2 $
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
 * Authors:  Eyal Mozes
 *
 * File Description:
 *
 */

#include <objects/seq/seq_id_handle.hpp>

#include <algo/align/util/align_source.hpp>

#include <util/range_coll.hpp>

BEGIN_NCBI_SCOPE
USING_SCOPE(objects);


class NCBI_XALGOALIGN_EXPORT CAlignCompare
{
public:
    /////////////////////////////////////////////////////////////////////////////
    //
    // Retrieve sets of intervals for alignments
    //
    
    typedef map<TSeqRange, TSeqRange> TAlignmentSpans;

    enum EMode { e_Interval, e_Exon, e_Span, e_Intron, e_Full };

    enum EMatchLevel {e_Equiv, e_Overlap, e_OverlapBetter, e_OverlapWorse, e_NoMatch};

    enum ERowComparison {e_Query, e_Subject, e_Both};

    /// Optional list of disambiguiting scores; alignments can only be compared
    /// if they are equal in these scores. The scores are divided into two groups:
    /// - scores in the first list are required to appear in all alignments, and
    ///   input is required to be sorted by these scores.
    /// - scores in the second set are optional.
    /// -- If two alignmens on both sides both have the score set, they must
    ///    have equal value to be compared.
    /// -- Alignments without the score, or with a 0 value, may be compared to
    ///    alignments that do have the score set.
    typedef pair< vector<int>, vector<int> > TDisambiguatingScoreValues;

    typedef pair< vector<string>, vector<string> > TDisambiguatingScoreList;

    typedef map<string,int> TIntegerScoreSet;
    typedef map<string,double> TRealScoreSet;

    //////////////////////////////////////////////////////////////////////////////
    //
    // struct defining all information needed to store a single alignment
    //
    
    struct SAlignment
    {
        int              source_set;

        CSeq_id_Handle   query;
        CSeq_id_Handle   subject;
    
        ENa_strand query_strand;
        ENa_strand subject_strand;
    
        TSeqRange        query_range;
        TSeqRange        subject_range;

        TSeqPos          length;

        TDisambiguatingScoreValues scores;
        vector<double> quality_scores;
        TIntegerScoreSet integer_scores;
        TRealScoreSet real_scores;
    
        CRef<CSeq_align> align;
        TAlignmentSpans  spans;
        CRangeCollection<TSeqPos> query_mismatches;
        CRangeCollection<TSeqPos> subject_mismatches;

        vector<const SAlignment *> matched_alignments;
        EMatchLevel      match_level;

        CAlignCompare &compare_object;

        SAlignment(int s, const CRef<CSeq_align> &al,
                   CAlignCompare &compare, bool is_slice = false);

        int CompareGroup(const SAlignment &o, bool strict_only) const;

        void PopulateBoundariesMap() const;

        list< AutoPtr<SAlignment> > BreakOnBoundaries(int row) const;

        AutoPtr<SAlignment> Slice(int row, TSeqPos from, TSeqPos to) const;
    };
    
    CAlignCompare(IAlignSource &set1,
                  IAlignSource &set2,
                  EMode mode = e_Interval,
                  bool strict = false,
                  bool ignore_not_present = false,
                  ERowComparison row = e_Both,
                  const TDisambiguatingScoreList &scores = TDisambiguatingScoreList(),
                  const vector<string> &quality_scores = vector<string>(),
                  const set<string> &score_set = set<string>(),
                  bool score_set_as_blacklist = false,
                  double real_score_tolerance = 0,
                  const set<string> distributive_scores = set<string>())
    : m_Set1(set1)
    , m_Set2(set2)
    , m_Mode(mode)
    , m_Strict(strict)
    , m_IgnoreNotPresent(ignore_not_present)
    , m_Row(row)
    , m_DisambiguitingScores(scores)
    , m_QualityScores(quality_scores)
    , m_ScoreSet(score_set)
    , m_ScoreSetAsBlacklist(score_set_as_blacklist)
    , m_RealScoreTolerance(real_score_tolerance)
    , m_DistributiveScores(distributive_scores)
    , m_CountSet1(0)
    , m_CountSet2(0)
    , m_CountSplitSet1(0)
    , m_CountSplitSet2(0)
    , m_CountEquivSet1(0)
    , m_CountEquivSet2(0)
    , m_CountOverlapSet1(0)
    , m_CountOverlapSet2(0)
    , m_CountOnlySet1(0)
    , m_CountOnlySet2(0)
    , m_CountEquivGroups(0)
    , m_CountOverlapGroups(0)
    , m_CountBasesSet1(0)
    , m_CountBasesSet2(0)
    , m_CountBasesEquivSet1(0)
    , m_CountBasesEquivSet2(0)
    , m_CountBasesOverlapSet1(0)
    , m_CountBasesOverlapSet2(0)
    , m_CountBasesOnlySet1(0)
    , m_CountBasesOnlySet2(0)
    {
    }

    bool EndOfData() const
    {
        return m_NextSet1Group.empty() && m_NextSet2Group.empty() &&
               m_Set1.EndOfData() && m_Set2.EndOfData();
    }

    void PopulateBoundariesMap();

    vector<const SAlignment *> NextGroup();

    size_t CountSet1() const { return m_CountSet1; }
    size_t CountSet2() const { return m_CountSet2; }
    size_t CountSplitSet1() const { return m_CountSplitSet1; }
    size_t CountSplitSet2() const { return m_CountSplitSet2; }
    size_t CountEquivSet1() const { return m_CountEquivSet1; }
    size_t CountEquivSet2() const { return m_CountEquivSet2; }
    size_t CountOverlapSet1() const { return m_CountOverlapSet1; }
    size_t CountOverlapSet2() const { return m_CountOverlapSet2; }
    size_t CountOnlySet1() const { return m_CountOnlySet1; }
    size_t CountOnlySet2() const { return m_CountOnlySet2; }
    size_t CountEquivGroups() const { return m_CountEquivGroups; }
    size_t CountOverlapGroups() const { return m_CountOverlapGroups; }
    size_t CountBasesSet1() const { return m_CountBasesSet1; }
    size_t CountBasesSet2() const { return m_CountBasesSet2; }
    size_t CountBasesEquivSet1() const { return m_CountBasesEquivSet1; }
    size_t CountBasesEquivSet2() const { return m_CountBasesEquivSet2; }
    size_t CountBasesOverlapSet1() const { return m_CountBasesOverlapSet1; }
    size_t CountBasesOverlapSet2() const { return m_CountBasesOverlapSet2; }
    size_t CountBasesOnlySet1() const { return m_CountBasesOnlySet1; }
    size_t CountBasesOnlySet2() const { return m_CountBasesOnlySet2; }

private:
    friend struct SAlignment;

    IAlignSource &m_Set1;
    IAlignSource &m_Set2;
    EMode m_Mode;
    bool m_Strict;
    bool m_IgnoreNotPresent;
    ERowComparison m_Row;
    TDisambiguatingScoreList m_DisambiguitingScores;
    vector<string> m_QualityScores;
    set<string> m_ScoreSet;
    bool m_ScoreSetAsBlacklist;
    double m_RealScoreTolerance;
    set<string> m_DistributiveScores;

    size_t m_CountSet1;
    size_t m_CountSet2;
    size_t m_CountSplitSet1;
    size_t m_CountSplitSet2;
    size_t m_CountEquivSet1;
    size_t m_CountEquivSet2;
    size_t m_CountOverlapSet1;
    size_t m_CountOverlapSet2;
    size_t m_CountOnlySet1;
    size_t m_CountOnlySet2;
    size_t m_CountEquivGroups;
    size_t m_CountOverlapGroups;
    size_t m_CountBasesSet1;
    size_t m_CountBasesSet2;
    size_t m_CountBasesEquivSet1;
    size_t m_CountBasesEquivSet2;
    size_t m_CountBasesOverlapSet1;
    size_t m_CountBasesOverlapSet2;
    size_t m_CountBasesOnlySet1;
    size_t m_CountBasesOnlySet2;

    list< AutoPtr<SAlignment> > m_CurrentSet1Group;
    list< AutoPtr<SAlignment> > m_CurrentSet2Group;
    list< AutoPtr<SAlignment> > m_NextSet1Group;
    list< AutoPtr<SAlignment> > m_NextSet2Group;

    map<CSeq_id_Handle, set<TSeqPos> > m_BoundariesMap;

    /// Determine whether the next group of alignments should be taken from set 1 or 2.
    /// If the next group from both sets are on the same query and subject, return 3;
    /// otherwise return 1 or 2.
    int x_DetermineNextGroupSet();

    /// Get next alignment from the correct set
    AutoPtr<SAlignment> x_NextAlignment(int set, bool update_counts = true)
    {
        AutoPtr<SAlignment> align =
             new SAlignment(set, (set == 1 ? m_Set1 : m_Set2).GetNext(), *this);
        if (update_counts) {
            ++(set == 1 ? m_CountSet1 : m_CountSet2);
            (set == 1 ? m_CountBasesSet1 : m_CountBasesSet2) += align->length;
        }
        return align;
    }

    void x_GetCurrentGroup(int set);

    void x_SplitOnOverlaps(int group, int row);
};

END_NCBI_SCOPE


#endif  // GPIPE_COMMON___ALIGN_COMPARE__HPP
