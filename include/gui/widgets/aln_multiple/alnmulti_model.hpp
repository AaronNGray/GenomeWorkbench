#ifndef GUI_WIDGETS_ALNMULTI___ALNMULTI_MODEL__HPP
#define GUI_WIDGETS_ALNMULTI___ALNMULTI_MODEL__HPP

/*  $Id: alnmulti_model.hpp 43940 2019-09-23 15:40:49Z shkeda $
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
 * Authors:  Andrey Yazhuk
 *
 * File Description:
 *   CAlnMultiModel - representation of alignment for rendering.
 */

#include <corelib/ncbiobj.hpp>

#include <util/range_coll.hpp>

#include <gui/widgets/aln_score/aln_scoring.hpp>

#include <gui/widgets/aln_multiple/sel_list_model_impl.hpp>
#include <gui/widgets/aln_multiple/alnmulti_ds.hpp>
#include <gui/widgets/aln_multiple/alnmulti_base_pane.hpp>

BEGIN_NCBI_SCOPE

class IAlnMultiDataSource;

typedef CSelListModelImpl<IAlnMultiDataSource::TNumrow> TNumrowModelImpl;

////////////////////////////////////////////////////////////////////////////////
/// class CAlnMultiModel - represents a visual model of an alignment. Model is
/// used by Renderer to draw alignment in a widget or to generate an image
/// (for example in a CGI).
///
/// Model presents an alignment as a set of graphical areas corresponding to the
/// alignment rows. Each area is represented by an instance if IAlignRow-derived
/// class and has attributes such as "visible" and "selected". Order of areas on
/// screen (lines) may be different from order of rows in the alignment.
/// Model holds the data necessary for rendering IAlignRow-derived objects
/// represnting rows, row attaributes and indexes facilitating operations with
/// Model (line <-> row, row_num <-> row).

class NCBI_GUIWIDGETS_ALNMULTIPLE_EXPORT CAlnMultiModel
    :   public IAlnMultiRendererContext,      // context for Renderer
        public TNumrowModelImpl,               // ISelListModel implementation
        public CScoreCache::IListener
{
public:
    typedef int TLine;
    typedef list<objects::CSeq_loc*>    TPSeqLocList;
    typedef CRangeCollection<TSeqPos>   TRangeColl;
    typedef IAlignRow::TColumnType      TColumnType;

    typedef IAlnExplorer::ESortState     TSortState;

public:
    CAlnMultiModel();
    virtual ~CAlnMultiModel();

    enum {
        kMasterRowIndex = -1000 ///< dummy master row index used in selection logic
    };

    // ISelListModel interface implementation
    virtual void    SLM_SelectAll(bool bSelect);
    virtual void    SLM_SelectSingleItem(TIndex index);
    virtual void    SLM_InvertSingleItem(TIndex index);

    virtual void Init();
    virtual void SetDataSource(IAlnMultiDataSource* p_ds);
    virtual void SetListener(CScoreCache::IListener* listener);

    virtual void    SetStyleCatalog(const CRowStyleCatalog* catalog);
    virtual CWidgetDisplayStyle*      GetDisplayStyle();

    CGlPane& GetAlignPort();

    IAlignRow*  GetRow(TNumrow  row);
    const IAlignRow*  GetRow(TNumrow  row) const;
    /// Get number of rows registered in model
    const size_t GetNumRows() const;

    bool    IsRowVisible(TNumrow  row) const;
    bool    IsRowSelected(TNumrow  row) const;

    /// returns a combination of IAlignRow::EState flags
    int   GetRowState(TNumrow row)    const;

    inline void    GetSelectedRows(vector<TNumrow>& rows);

    virtual void    CreateRows(void);
    virtual void    ClearRows(void);

    /// Creates consenus row if it is not already created
    virtual void    CreateConsensus(void);

    void    SetPortLimits(void);

    /// changes Master (Anchor) and performs necessary updates
    virtual bool    CanChangeMasterRow() const;
    virtual bool    SetMasterRow(TNumrow new_row);

    /// @name Sorting and Reordering API
    /// {

    /// enables auto sorting but does not perform any updates
    virtual void    EnableAutoSorting(bool b_en);

    /// enables auto sorting of rows, sorts rows using x_SortRows() and
    /// performs necessary updates
    virtual void    UpdateSortOrder(void);

    /// sorts visible rows by specified column, return "false" if sorting is
    /// not supported (nothin happened)
    virtual bool    SortByColumn(TColumnType type, bool ascending);

    /// returns sorting state for the specified column
    virtual TSortState   GetColumnSortState(TColumnType type);

    /// returns mask of shown elements - header, ruler, master row, alignment, track
    /// matrix
    virtual int GetShownElements();

    virtual void SetShownElements(int shown_mask);

    /// disables auto sorting, reorders rows in alignment so that rows from
    /// the given vector appear in the specified order starting from "pos"
    /// position
    virtual void    SetRowOrder(const vector<TNumrow>& rows, int pos = 0);
    /// @}


    /// makes specified rows visible/hidden, if b_invert_others == "true"
    /// then visibility of all rows not in "rows" will be changed to ! b_visible
    void    SetVisible(vector<TNumrow> rows, bool b_visible,
                       bool b_invert_others = false);
    void    ShowAll();

    virtual void    UpdateOnStyleChanged(void);

    /// updates model after row height changed
    virtual void    UpdateOnRowHChanged(void);

    virtual void    UpdateOnScoringChanged();

    virtual void    UpdateOnDataChanged();

    /// @name IAlnMultiRendererContext implementation
    /// @{
    virtual const CGlPane& GetAlignPort() const;
    virtual bool        IsRendererFocused();
    /// NUmber of visible lines
    virtual TNumrow     GetLinesCount() const;
    virtual int     GetFocusedItemIndex() const;
    virtual bool    IsItemSelected(int index) const;

    virtual IAlignRow*  GetMasterRow();
    virtual IAlignRow*  GetConsensusRow();

    virtual IAlignRow*  GetRowByLine(TLine line);
    virtual int         GetLineByRowNum(TNumrow row) const;
    virtual int         GetLineByModelY(int Y) const;
    virtual int         GetLinePosY(TLine line) const;
    virtual int         GetLineHeight(TLine line) const;
    /// @}

    /// @name Scoring Methods management functions
    /// @{
    typedef list<CIRef<IScoringMethod> >    TMethods;

    /// adds scoring method; model assumes ownership of the method, it will be
    /// deleted automatically
    virtual bool    AddScoringMethod(IScoringMethod* method);
    virtual const TMethods&        GetMethods();

    virtual void    EnableBackgroundProcessing(bool en);

    virtual const IScoringMethod*   GetCurrentMethod() const;
    virtual IScoringMethod*         GetCurrentMethod();

    virtual bool    SetCurrentMethod(const string& name);
    virtual void    ResetCurrentMethod();

    virtual string  GetDefaultMethod(void);
    virtual void    SetDefaultMethod(const string& method_name);
    /// @}

    /// @name CScoreCache::IListener
    /// @{
    virtual void    OnScoringProgress(float progress, const string& msg);
    virtual void    OnScoringFinished();
    /// @}

protected:
    /// SRowRecord is a record representing a single alignment row
    struct  SRowRecord
    {
        CIRef<IAlignRow>  m_Row;  ///< row object ptr
        TLine       m_Line; ///< visual line index

    public:
        SRowRecord(IAlignRow* p_row = 0, TLine line = -1)
            : m_Row(p_row), m_Line(line) 
        {}
        SRowRecord(const SRowRecord& proto)
            : m_Row(proto.m_Row), m_Line(proto.m_Line)
        {}

        bool IsVisible() const;
        bool IsSelected() const;

        static bool    PRecLineLess(const SRowRecord* rec1,
                                    const SRowRecord* rec2);
    };

protected:
   inline SRowRecord&     x_GetRecordByRow(TNumrow row);
   inline const SRowRecord&     x_GetRecordByRow(TNumrow row) const;

    virtual void    x_CreateDisplayStyle();

    /// factory method for creating IAlignRow instances
    virtual IAlignRow*  x_CreateRow(TNumrow row);

    virtual void    x_UpdateMinScale();    
    virtual void    x_UpdateOnCoordsChanged();
    virtual void    x_UpdateOnVisibilityChanged(void);

    virtual bool    x_UsingAutoSort() const;

    /// This is a sorting callback automatically invoked by CAlnMultiModel when
    /// necessary. Override this function in derived classes - default
    /// implementation doesn't do anything.
    /// x_SortRows() should rearrange eleemnts of m_vVisibleRows and return "true"
    /// if any changes have been made - this will force necessary updates.
    virtual bool    x_SortRows(void);

    /// performs sorting by invoking x_SortRows() and updates line numbers
    virtual void    x_DoUpdateRowsOrder(void);

    /// recalculates vertical coordinates of row objects
    virtual void    x_DoUpdateRowPositions(void);


    void    x_DoUpdateRowLines(void); /// updates line numbers in SRowRecords

    /// @TNumrowModelImpl extension
    /// @{
    virtual void    x_SelectItem(TIndex index, bool b_sel);
    virtual bool    x_IsItemSelected(TIndex index) const;

    virtual TIndex  x_GetItemsCount()   const;
    virtual TItem   x_GetItem(TIndex index)    const;
    virtual void    x_SetEntries(const TEntryVector& v_entries);
    virtual TIndex  x_GetItemIndex(const TItem& item);

    virtual void    x_InsertItem(TIndex index, const TItemEntry& entry);
    virtual void    x_CompleteInsertion();
    virtual void    x_MarkItemForErase(TIndex index);
    virtual void    x_EraseMarkedItems(void);
    virtual void    x_ClearItems(void);
    /// @}

    IAlignRow*  x_GetRowByLine(int Index);
    const IAlignRow*  x_GetRowByLine(int Index)   const;

protected:
    typedef vector<SRowRecord*>    TRowRecVector;
    typedef vector<SRowRecord>     TRowToRecMap;

    CWidgetDisplayStyle*    m_Style;
    const CRowStyleCatalog* m_StyleCatalog;

    IAlnMultiDataSource*    m_DataSource;
    CScoreCache::IListener*  m_Listener;

    TRowToRecMap    m_RowToRec;        ///< map storing records for all rows

    CIRef<IAlignRow>      m_MasterRow;      ///< IAligRow corresponding to Anchor
    CIRef<IAlignRow>      m_DataConsensusRow; ///< IAligRow corresponding to consensus in the data
    TRowRecVector   m_vVisibleRows;   ///< line number SRowRecord* for visible rows
    vector<int>     m_vVisibleRowPos; ///< bottom positions for visible rows[line]

    CGlPane  m_Port;

    /// sorting support
    bool    m_AutoSort;          ///< enables automating sorting of rows using x_SortRows() callback
    TColumnType     m_SortColumn;
    TSortState      m_SortState;

    /// scoring support
    bool        m_EnableScoring;
    TMethods    m_Methods;       ///< registered scoring methods
    CRef<CScoreCache> m_ScoreCache;

    CRef<CConsensusRow> m_ConsensusRow;

    // Alternative views support
    int m_ShownElements = IAlnMultiRendererContext::fShownElement_All;

private:
    bool    m_isDataReadSync;

private:
    /// forbidden
    CAlnMultiModel(const CAlnMultiModel&);
    CAlnMultiModel& operator=(const CAlnMultiModel&);
};


////////////////////////////////////////////////////////////////////////////////
/// CAlnMultiModelFactory - a factory to separate creation of the model from
/// widget. Implement this interface is you need to use a custom model with a
/// standard widget.
class NCBI_GUIWIDGETS_ALNMULTIPLE_EXPORT CAlnMultiModelFactory
{
public:
    virtual ~CAlnMultiModelFactory()    {}

    virtual CAlnMultiModel* CreateModel() = 0;
};


////////////////////////////////////////////////////////////////////////////////
/// CPairSorter
template <typename TKey, typename TValue>   class CPairSorter
{
public:
    typedef pair<TKey, TValue>  TPair;
    typedef vector<TPair>   TPairVector;

    CPairSorter(bool ascending) :   m_Ascending(ascending)  {}

    void    Sort(TPairVector& data)
    {
        std::sort(data.begin(), data.end(), *this);
    }

    inline bool operator()(const TPair& p1, const TPair& p2)
    {
        return m_Ascending ? (p1.second < p2.second) : (p2.second < p1.second);
    }

protected:
    bool m_Ascending;
};


////////////////////////////////////////////////////////////////////////////////
/// CAlnMultiModel inline functions

inline IAlignRow*  CAlnMultiModel::GetRow(TNumrow  row)
{
    _ASSERT(row >= 0  &&  (size_t) row < m_RowToRec.size());
    return m_RowToRec[row].m_Row;
}

const inline IAlignRow*  CAlnMultiModel::GetRow(TNumrow  row) const
{
    _ASSERT(row >= 0  &&  (size_t) row < m_RowToRec.size());
    return m_RowToRec[row].m_Row;
}

inline bool     CAlnMultiModel::IsRowVisible(TNumrow  row) const
{
    _ASSERT(row >= 0  &&  (size_t) row < m_RowToRec.size());
    return m_RowToRec[row].IsVisible();
}

inline bool     CAlnMultiModel::IsRowSelected(TNumrow  row) const
{
    _ASSERT(row >= 0  &&  (size_t) row < m_RowToRec.size());
    return m_RowToRec[row].IsSelected();
}

inline CAlnMultiModel::SRowRecord&     CAlnMultiModel::x_GetRecordByRow(TNumrow row)
{
    _ASSERT(row >= 0  &&  (size_t) row < m_RowToRec.size());
    return m_RowToRec[row];
}

inline const CAlnMultiModel::SRowRecord&     CAlnMultiModel::x_GetRecordByRow(TNumrow row) const
{
    _ASSERT(row >= 0  &&  (size_t) row < m_RowToRec.size());
    return m_RowToRec[row];
}

inline  bool    CAlnMultiModel::SRowRecord::IsVisible() const
{
    int state = m_Row->GetRowState();
    return (state & IAlignRow::fItemHidden) == 0;
}

inline  bool  CAlnMultiModel::SRowRecord::IsSelected() const
{
    int state = m_Row->GetRowState();
    return (state & IAlignRow::fItemSelected) != 0;
}

inline void  CAlnMultiModel::GetSelectedRows(vector<TNumrow>& rows)
{
    SLM_GetSelectedItems(rows);
}

inline const size_t CAlnMultiModel::GetNumRows() const
{
    return m_RowToRec.size();
}


END_NCBI_SCOPE


#endif  // GUI_WIDGETS_ALNMULTI___ALNMULTI_MODEL__HPP
