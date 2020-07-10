#ifndef GUI_WIDGETS_ALN_SPAN___ALN_SPAN_VERT_MODEL__HPP
#define GUI_WIDGETS_ALN_SPAN___ALN_SPAN_VERT_MODEL__HPP

/*  $Id: alnspan_vertmodel.hpp 36486 2016-09-28 21:23:44Z shkeda $
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
 * Authors:  Yury Voronov
 *
 * File Description:
 *
 */

#include <gui/gui_export.h>
#include <objmgr/scope.hpp>
#include <objects/seqalign/Seq_align.hpp>
#include <objects/seqalign/Spliced_seg.hpp>
#include <objects/seqalign/Score.hpp>
#include <gui/widgets/aln_multiple/alnmulti_ds.hpp>
#include <objtools/alnmgr/sparse_aln.hpp>

#include <gui/widgets/wx/table_model.hpp>

#include <vector>


BEGIN_NCBI_SCOPE


class NCBI_GUIWIDGETS_ALNTABLE_EXPORT CAlnSpanVertModel
    : public CwxAbstractTableModel
{
public:

    enum EExtraColumns {
        eLengthCol,
        eMismatchCountCol,
        eGapLengthCol,
        eIdenticalCol,
        eTypeCol,
        eMaxExtraCols
    };

    enum {
        fAligned       = 0x01,
        fMismatch      = 0x02,
        fGap           = 0x04,
        fDiscontig     = 0x08,
        fIntron        = 0x10,
        fIntronNC      = 0x20,
        fPolyA         = 0x40,
        fTail          = 0x80,
        fInvalid       = 0x100
   };

    /// the alignment we store with its parsed data
    struct SSpanRow
    {
        TSeqRange aln_range;
        vector<TSignedSeqRange> ranges;
        vector< bool > strands;
        vector< CConstRef<objects::CSeq_loc> > locs;

        int type;
        int length;
        int gap;
        int mismatch;
        double identity;

        /// visible values
        mutable vector<string> str_ranges;
        mutable string str_type;

        SSpanRow()
            : type( 0 )
            , length( 0 )
            , gap( 0 )
            , mismatch( 0 )
            , identity( 0.0 )
        {
        }
    };
    typedef vector<SSpanRow> TAlnSpans;

    CAlnSpanVertModel();
    CAlnSpanVertModel( IAlnMultiDataSource& aln_source, int threshold = 0 );
    CAlnSpanVertModel( CSparseAln& aln_source, int threshold = 0 );
    CAlnSpanVertModel( const CSpliced_seg& spliced_seg, CScope& scope, int threshold = 0 );
    CAlnSpanVertModel( vector< CConstRef<CSeq_align> >& input_aligns, CScope& scope, int threshold = 0 );


    void AddAlignment( CSparseAln& aln_source );

    //void Update();

    /// access a given row's data
    const SSpanRow& GetData( size_t row ) const;

    int GetThreshold() const { return m_Threshold; }
    bool GetShowIndels() const { return mf_ShowIndels; }
    int GetNumSequences() const { return m_NumSeqs; }

    void SetThreshold( int th );
    void SetShowIndels( bool indel );
    void SetMarkIndels( int flag ){ mf_MarkIndels = flag; }

    void UpdateRows();

    /// @name ITableModel implementation
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

    CRef<IAlnMultiDataSource> m_AlnSrc;
    CAnchoredAln m_AnchoredAln;
    vector< CRef<CSparseAln> > m_SparseAlns;
    CRef<CSpliced_seg> m_SplicedSeg;
    CRef<CScope> m_Scope;
    int m_Threshold;
    int mf_MarkIndels;
    int m_NumSeqs;
    bool mf_UseSpliced;
    bool mf_UseSparse;
    bool mf_ShowIndels;

    bool mf_RowsInvalidated;

    vector<string> m_ColNames;

    /// the alignments we represent
    TAlnSpans m_AlnSpans;

    CRange<TSignedSeqPos> m_AlnRange;
    int m_BaseWidth = 1;

    void x_AdjustColumns();
    //void x_AdjustColumns(TAlignments::const_iterator begin,TAlignments::const_iterator end);

    void x_Init();
    void x_Init( IAlnMultiDataSource& source, int thresh = 0 );
    void x_Init( CSparseAln& source, int thresh = 0 );
    void x_Init( const CSpliced_seg& source, CScope& scope, int thresh = 0 );


    void x_Clear();

    void x_CreateRows();
    void x_PopulateRows(TAlnSpans& rows);
    void x_PopulateRowsSpliced( TAlnSpans& rows );

    void x_InsertIntrons(TAlnSpans& rows);
    void x_MergeRows(TAlnSpans& rows);
    void x_ConvertRowsToSpans(TAlnSpans& rows);

    void x_UpdateMetaData();
    void x_UpdateLocs();
};



END_NCBI_SCOPE


/*
 * ===========================================================================
 * $Log$
 * ===========================================================================
 */

#endif  // GUI_WIDGETS_ALN_SPAN___ALN_SPAN_VERT_MODEL__HPP
