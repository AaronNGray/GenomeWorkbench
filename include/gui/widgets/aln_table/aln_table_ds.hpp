#ifndef GUI_WIDGETS_ALN_TABLE___ALN_TABLE_DS__HPP
#define GUI_WIDGETS_ALN_TABLE___ALN_TABLE_DS__HPP

/*  $Id: aln_table_ds.hpp 15723 2008-01-18 05:47:31Z voronov $
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
 * Authors:  Mike DiCuccio
 *
 * File Description:
 *
 */

#include <gui/gui_export.h>
#include <objmgr/scope.hpp>
#include <objects/seqalign/Seq_align.hpp>
#include <objects/seqalign/Score.hpp>
#include <gui/widgets/wx/table_model.hpp>

#include <vector>


BEGIN_NCBI_SCOPE


class NCBI_GUIWIDGETS_ALNTABLE_EXPORT CAlnTableDS
    : public CObject,
      public CwxAbstractTableModel
{
public:
    /// the alignment we store with its parsed data
    struct SAlignment
    {
        SAlignment()
            : num_seqs(0)
            , seq1_start(-1)
            , seq1_stop(-1)
            , seq2_start(-1)
            , seq2_stop(-1)
            , aligned_length(0)
            , mismatch_count(0)
            , gap_count(0)
        {
        }

        typedef map<string, CConstRef<objects::CScore> > TScores;

        mutable CRef<objects::CScope> scope;
        CConstRef<objects::CSeq_align> align;

        mutable string label;
        mutable string type;
        mutable int    num_seqs;
        mutable int    seq1_start;
        mutable int    seq1_stop;
        mutable int    seq2_start;
        mutable int    seq2_stop;
        mutable int    aligned_length;
        mutable int    mismatch_count;
        mutable int    gap_count;
        mutable TScores scores;
    };
    typedef vector<SAlignment> TAlignments;

    /// the columns we plan to provide
    enum EColumns {
        eLabel,         //< brief description of alignment
        eType,          //< type (nucl vs nucl, prot vs. nucl, prot vs. prot)
        eSeq1Start,     //< start of sequence 1
        eSeq1Stop,      //< stop of sequence 1
        eSeq2Start,     //< start of sequence 2
        eSeq2Stop,      //< stop of sequence 2
        eAlignedLength, //< count of aligned bases
        eMismatchCount, //< count of mismatches
        eGapCount,      //< count of the number of gaps
        eNumSeqs,       //< number of sequences

        // must be last!
        eScoreStart //< one column for each score type
    };

    CAlnTableDS();
    CAlnTableDS(objects::CScope& scope,
                const objects::CSeq_loc& loc);
    CAlnTableDS(objects::CScope& scope,
                const objects::CSeq_annot& annot);
    CAlnTableDS(objects::CScope& scope,
                const objects::CSeq_align& align);
    CAlnTableDS(objects::CScope& scope,
                const objects::CSeq_align_set& aligns);
    CAlnTableDS(objects::CScope& scope,
                const list< CConstRef<objects::CSeq_align> >& aligns);

    void Add(objects::CScope& scope, const objects::CSeq_align& aln);
    const TAlignments& GetAlignments(void) const;

    void Update();

    /// access a given row's data
    const SAlignment& GetData(size_t row) const;

    /// @name IwxTableModel implementation
    /// @{

    virtual int GetNumRows() const;

    virtual int GetNumColumns() const;

    virtual wxString GetColumnName( int aColIx ) const;
    ///
    virtual wxString GetColumnType( int aColIx ) const;
    ///
    virtual wxVariant GetValueAt( int i, int j ) const;

    /// @}

private:
    /// mappings for columns for alignment scores
    typedef map<string, size_t> TScoreColNames;
    TScoreColNames m_ColNames;

    typedef map<size_t, string> TScoreColIndices;
    TScoreColIndices m_ColIndices;

    /// the alignments we represent
    TAlignments m_Alignments;
    void x_AddAlign(objects::CScope& scope, const objects::CSeq_align& align);

    void x_AdjustColumns();
    void x_AdjustColumns(TAlignments::const_iterator begin,
                         TAlignments::const_iterator end);

    int  x_GetNumSeqs      (const SAlignment& proxy) const;
    void x_GetSeqLength    (const SAlignment& proxy) const;
    void x_GetGapCount     (const SAlignment& proxy) const;
    void x_GetMismatchCount(const SAlignment& proxy) const;
    void x_GetStartStop    (const SAlignment& proxy) const;
};



END_NCBI_SCOPE

#endif  // GUI_WIDGETS_ALN_TABLE___ALN_TABLE_DS__HPP
