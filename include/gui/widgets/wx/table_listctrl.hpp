#ifndef GUI_WIDGETS_WX___TABLE_LIST_CTRL__HPP
#define GUI_WIDGETS_WX___TABLE_LIST_CTRL__HPP

/*  $Id: table_listctrl.hpp 39666 2017-10-25 16:01:13Z katargir $
 * ===========================================================================
 *
 *                            PUBLIC DOMAIN NOTICE
 *               National Center for Biotechnology Information
 *
 *  This software/database is a "United States Government Work" under the
 *  terms of the United States Copyright Act.  It was written as part of
 *  the author's official duties as a United States Government employee and
 *  thus cannot be copyrighted.  `This software/database is freely available
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

#include <corelib/ncbistd.hpp>

#include <corelib/ncbitime.hpp>

#include <gui/widgets/wx/table_model.hpp>
#include <gui/widgets/wx/row_model.hpp>
#include <gui/widgets/wx/wx_utils.hpp>
#include <gui/utils/event_handler.hpp>
#include <gui/utils/command.hpp>
//#include <gui/widgets/wx/context_menu_utils.hpp>

#include <gui/utils/event_handler.hpp>

#include <wx/wx.h>
#include <wx/listctrl.h>
#include <wx/imaglist.h>
#include <wx/menu.h>
#include <wx/timer.h>
#include <wx/tipwin.h>

//#define ___NO_ORDER_FUNCTIONS_YET


BEGIN_NCBI_SCOPE

class CTableListColumn
{
public:
    wxListItem m_Item;
    int m_ModelIx;
    int m_Width;

    bool m_Visible;
    bool m_HidingAllowed;
    bool m_Resizable;
    bool m_Sortable;
    //int m_Order;

    CIRef<IRowSorter> m_Sorter;
    CIRef<IwxStringFormat> m_Formatter;

public:
    CTableListColumn( int aModelIndex );

    CTableListColumn& operator=( const CTableListColumn& column )
    {
        m_Item.m_mask = column.m_Item.m_mask;
        m_Item.m_itemId = column.m_Item.m_itemId;
        m_Item.m_col = column.m_Item.m_col;
        m_Item.m_state = column.m_Item.m_state;
        m_Item.m_stateMask = column.m_Item.m_stateMask;
        m_Item.m_text = column.m_Item.m_text;
        m_Item.m_image = column.m_Item.m_image;
        m_Item.m_data = column.m_Item.m_data;
        m_Item.m_format = column.m_Item.m_format;
        m_Item.m_width = column.m_Item.m_width;

        m_ModelIx = column.m_ModelIx;
        m_Width = column.m_Width;
        m_Visible = column.m_Visible;
        m_HidingAllowed = column.m_HidingAllowed;
        m_Resizable = column.m_Resizable;
        m_Sortable = column.m_Sortable;

        m_Sorter = column.m_Sorter;
        m_Formatter = column.m_Formatter;

        return *this;
    }
};

///////////////////////////////////////////////////////////////////////////////
/// CwxTableListCtrl - generic wxListCtrl-based Table Control
///
/// Do not use InsertColumn() and DeleteColumn() as far
/// as it ruins all the internal work with columns

class NCBI_GUIWIDGETS_WX_EXPORT CwxTableListCtrl
    : public wxListView
    , public IwxTableModelListener
    , public CEventHandler
{
public:
    enum ESortedState
    {
        eNoSort,
        eAscSort,
        eDescSort
    };

    enum ECommands
    {
        eCmdExportToCSV = eBaseCmdLast + 300
    };

    typedef pair<int,ESortedState> TSortedCol;


public:
    CwxTableListCtrl() {}

    CwxTableListCtrl(
        wxWindow* parent,
        wxWindowID id = wxID_ANY,
        const wxPoint& pos = wxDefaultPosition,
        const wxSize& size = wxDefaultSize,
        long style = wxLC_REPORT | wxLC_VIRTUAL,
        const wxValidator& validator = wxDefaultValidator,
        const wxString& name = wxListCtrlNameStr
    );

    CwxTableListCtrl( wxWindow* parent, wxWindowID id, IwxTableModel* aData );
    virtual ~CwxTableListCtrl();

public:
    IwxTableModel* GetModel() const;
    void SetModel( IwxTableModel* data_model, bool own = false );
    void AssignModel( IwxTableModel* data_model );
    void RemoveModel();

    wxVariant GetValueFromModelAt( int row, int col ) const;
    const wxString GetTypeFromModelAt( int row, int col ) const;

    wxVariant GetValueVisibleAt( int row, int col ) const;
    const wxString GetTypeVisibleAt( int row, int col ) const;

    void EnableHeader( bool flag );

    void SaveTableSettings(CRegistryWriteView& view, bool saveSorting = true) const;
    void LoadTableSettings(const CRegistryReadView& view, bool byName = false);

    void ExportTableControlToCSV();

#ifndef  wxHAS_LISTCTRL_COLUMN_ORDER
    // Gets the column order from its index or index from its order
    int GetColumnOrder( int col ) const;
    int GetColumnIndexFromOrder( int order ) const;

    // Gets the column order for all columns
    wxArrayInt GetColumnsOrder() const;

    // Sets the column order for all columns
    bool SetColumnsOrder( const wxArrayInt& orders );
#endif

    bool SetColumnOrder( int col, int order );

    int GetNumColumnsVisible() const;
    int GetNumColumnsHidden() const;

    int ColumnVisibleToIndex( int col ) const;
    int ColumnIndexToVisible( int col ) const;

    int ColumnHiddenToIndex( int col ) const;
    int ColumnIndexToHidden( int col ) const;

    int ColumnIndexToModel( int col ) const;
    int ColumnModelToIndex( int col ) const;

    int ColumnVisibleToModel( int col ) const;
    int ColumnModelToVisible( int col ) const;

    bool IsColumnVisibleByIndex( int col ) const;

    wxString GetColumnNameByModel( int col ) const;
    wxString GetColumnNameByIndex( int col ) const;
    wxString GetColumnNameVisible( int col ) const;
    wxString GetColumnNameHidden( int col ) const;

    wxString GetColumnTitleByModel( int col ) const;
    wxString GetColumnTitleByIndex( int col ) const;
    wxString GetColumnTitleVisible( int col ) const;
    wxString GetColumnTitleHidden( int col ) const;

    int GetModelIndexByName( wxString name ) const;
    int GetColumnIndexByName( wxString name ) const;
    int GetColumnVisibleByName( wxString name ) const;
    int GetColumnHiddenByName( wxString name ) const;

    wxListColumnFormat GetColumnAlign( int col_ix ) const; // by index
    void SetColumnAlign( int col_ix, wxListColumnFormat aAlign ); // by index
    //! get/set all other column features

    void AddColumn( CTableListColumn& aColumn );
    void AddColumn( int model_ix, bool hidden = false );

    void RemoveColumn( int col_ix, bool hide = false );
    void MoveColumn( int col_ix, int aTargetColIx );

    void HideColumn( int col_ix ){ RemoveColumn( col_ix, true ); }
    void ShowColumn( int col_ix );

    void RemoveAllColumns();

    void CreateAllColumnsFromModel();

    int GetNumRowsVisible() const { return (int)m_VisibleRows.size(); }

    int RowVisibleToData( int aRow ) const;
    int RowDataToVisible( int aRow ) const;

    /// @name Methods concerned with filtering
    /// @{

    /// Filter is not owned by Table control.You should manage it deletion by yourself.
    const IRowFilter* GetFilter() const;
    void SetFilter( IRowFilter* filter );
    IRowFilter* RemoveFilter();
    void ApplyFiltering( bool preserve_state = true );
    /// @}

    /// @name Methods concerned with sorting
    /// @{
    // Sorter is not owned by Table control. You should manage it deletion by yourself.
    CIRef<IRowSorter> GetSorter() const;
    void SetSorter( CIRef<IRowSorter> sorter );
    CIRef<IRowSorter> RemoveSorter();

    // Sorters-by-column and default sorters ARE owned by Table control.
    // Do not delete them by yourself.

    CIRef<IRowSorter> GetColumnSorterByIndex( int col_ix ) const;
    void SetSorterByIndex( int col );
    void AddSorterIndex( size_t col, ESortedState state ){
        _ASSERT( col < m_Columns.size() );

        m_SortedCols.push_back( TSortedCol( (int)col, state ) );
    }
    void SetSorterByColumns();

    ESortedState GetSortedState() const { return m_Sorted; }
    void SetSortedState( ESortedState sorted );
    //int GetSortedCol() const { return m_SortedCols.empty() ? -1 : m_SortedCols[0]; }
    const vector<TSortedCol>& GetSortedCols() const { return m_SortedCols; }

    void UpdateSortIcons();
    void ClearSortIcons();

    void InvalidateSorting() { m_SortInvalid = true; }
    void ResetSorting() { m_SortedCols.clear(); m_Sorted = eNoSort; InvalidateSorting(); }
    void ApplySorting( bool preserve_state = true );
    void ShiftSorting( int col = -1 );

    IwxVariantSorter* GetDefaultSorter( wxString aType ) const;
    void SetDefaultSorter( wxString aType, IwxVariantSorter* aSorter );
    void CreateDefaultSorters();
    /// @}

    /// @name Focus
    /// @{
    int  GetFocusRow() const;
    void SetFocusRow( int row );
    int  GetFocusColumn() const;
    void SetFocusColumn( int WXUNUSED(col) ) {}

    void OnFocusChanged( wxListEvent& event );
    /// @}

    /// @name Selection
    /// @{
    bool IsCellSelectedAt( int row, int WXUNUSED(col) ) const;
    long GetLastSelected() const;

    void SelectAll();
    void ClearSelection();

    wxArrayInt GetDataRowsSelected() const;
    void GetDataRowsSelected( vector<int>& rows ) const;
    void SetDataRowsSelected( const vector<int>& rows );
    /// @}

    /// @name Methods concerned with editing
    /// When editing starts, two instances are installed inside the table, an editor
    /// and an editing widget.
    /// @{
    //bool IsCellEditableAt(int row, int col ) const;
    //bool EditCellAt(int row, int col, int anEventCode = FL_NO_EVENT );

    //bool IsEditing() const { return m_EditingWg != (Fl_Widget*)NULL && m_EditingWg -> parent() != NULL; }
    //int GetEditingRow() const { return IsEditing() ? GetFocusRow() : -1; }
    //int GetEditingColumn() const { return IsEditing() ? GetFocusColumn() : -1; }
    /// @}

    /// @name ITCEListener interface implementation
    ///
    /// @{
    /// To be called by editor when editing session is cancelled by user
    /// or otherwise
    /* Done through events
    virtual void EditingCancelled();
    /// To be called by editor when editing session is finished by user
    /// or otherwise
    virtual void EditingFinished();
    */
    /// @}

    /* Of no use for ListCtrl
    int GetRowVisibleAtY(int aY ) const;
    int GetTotalRowHeight() const;
    int GetNumRowsViewed() const;
    */

    // CRect GetCellRect(int aRow, int aCol, bool aWithSpacing ) const;

    /* Use EnsureVisible( item ) instead
    virtual void ScrollRectToVisible( const CRect& aRect );
    void ScrollRowsToVisible(int aRow, int bRow = -1 );
    void ScrollSelectionToVisible();
    */

    /*
    CAnyRef<ITableCellRenderer> GetDefaultRenderer( const type_info& aType ) const;
    void SetDefaultRenderer( const type_info& aType, CAnyRef<ITableCellRenderer> aRenderer );
    ///
    CAnyRef<ITableCellEditor> GetDefaultEditor( const type_info& aType ) const;
    void SetDefaultEditor( const type_info& aType, CAnyRef<ITableCellEditor> aEditor );
    */

    //CEventProc& GetEventProcessor() { return m_Semantics; }

    /// @name ITableModel::ITMListener interface implementation
    /// @{
    virtual void TableChanged( const CwxTableModelNotice& aNotice );
    /// @}

    /// @name ISelectionModel::ISMListener interface implementation
    /// @{
    /* Done through events
    virtual void SelectionChanged( const ISelectionModel::CSMNotice& aNotice );
    */
    /// @}

    // Of no use in wxWidgets; stayed here for ease of porting
    //bool GetAutoResizeMode() const { return false; }
    //void SetAutoResizeMode( bool WXUNUSED(aMode) ){ }

    /// /////////// Exterior settings

    bool ShowHorzLines() const;
    void SetShowHorzLines( bool show );
    bool ShowVertLines() const { return (GetWindowStyleFlag() & wxLC_VRULES) != 0; }
    void SetShowVertLines( bool show ){ SetSingleStyle( wxLC_VRULES, show ); }

    void SetShowGrid( bool aShow ){ SetShowHorzLines( aShow ); SetShowVertLines( aShow ); }

    /// @name Various color settings
    /// @{

    wxColor GetForegroundColor() const { return GetTextColour(); }
    void SetForegroundColor( wxColor aColor ){ SetTextColour( aColor ); }

    #ifdef __WXMAC__
    // bug in wxWidgets GetBackgroundColour() define on Mac (missing const)
    wxColor GetBackgroundColor() { return GetBackgroundColour(); }
    #else
    wxColor GetBackgroundColor() const { return GetBackgroundColour(); }
    #endif
    void SetBackgroundColor( wxColor aColor ){ SetBackgroundColour( aColor ); }

    wxColor GetSecondBackgroundColor() const { return m_2ndBackColor; }
    void SetSecondBackgroundColor( wxColor aColor ){ m_2ndBackColor = aColor; }
    /// @}

    bool ShouldPropagateContextMenu() const { return mf_Propagate; }
    void SetContextMenuEventPropagation( bool flag ){ mf_Propagate = flag; }

    virtual void AppendMenuItems( wxMenu& aMenu );

protected:
    void OnSelectionChanged( wxListEvent& event );
    virtual void x_OnSelectionChanged( bool on, int start_row, int end_row = -1 );

    /// ////////////// Header management

    void OnColumnClick( wxListEvent &anEvt );
    void OnColumnRightClick( wxListEvent &anEvt );
    void OnColumnBeginDrag( wxListEvent &anEvt );
    void OnColumnEndDrag( wxListEvent &anEvt );

    void OnKeyDown( wxKeyEvent &anEvt );

    void OnSelectAll( wxCommandEvent &anEvt );

    void OnContextMenu( wxContextMenuEvent& anEvent );

    void OnCopy( wxCommandEvent &anEvt );
    void OnCopyUpdate( wxUpdateUIEvent &anEvt );
    virtual void x_OnCopy();

    void OnDebugEvent( wxListEvent &anEvt );
    void OnMouseMove( wxMouseEvent &anEvt );
    void OnLeftDown( wxMouseEvent &anEvt );

    void OnTimer(wxTimerEvent& event);

	void OnPaint( wxPaintEvent& anEvt );

protected:
    void InternalOnPopupMenu( wxCommandEvent& event );
    int  DoGetPopupMenuSelectionFromUser( wxMenu& menu, const wxPoint& pos );
    int x_FindRow(const wxPoint &pt) const;
    int x_FindColumn(const wxPoint &pt) const;
    void x_ShowTooltip(bool show);

    int ShowHeaderContextMenu( const wxPoint& pos );

    struct SViewState {
        int m_FocusModelRow;
        int m_TopModelRow;
        vector<int> m_Selected;

        SViewState() : m_FocusModelRow( -1 ), m_TopModelRow( -1 ) {}
    };
    void x_SaveViewState( SViewState* state );
    void x_RestoreViewState( SViewState* state );


private:
    void x_Init();

    wxListColumnFormat GetAlignByType( wxString aColType );

    /// @name wxListCtrl virtual mode implementation
    /// @{
    virtual wxString OnGetItemText( long aRow, long aCol ) const;
    virtual int OnGetItemImage( long aRow ) const;
    virtual int OnGetItemColumnImage( long aRow, long aCol ) const;
    virtual wxListItemAttr* OnGetItemAttr( long aRow ) const;
    /// @}

private:
    mutable wxImageList m_ImageList;
    mutable bool mf_ImageListInstalled;

    IwxTableModel* m_DataModel;
    bool mf_OwnDataModel;
    int m_ModelImageShift;

    WX_DECLARE_STRING_HASH_MAP( int, TAliasMap );
    mutable TAliasMap m_AliasToIndex;

    wxColor m_2ndBackColor;

    vector<int> m_VisibleRows;

    vector<CTableListColumn> m_Columns;
    int m_NumColumnsVisible;

    bool mf_Propagate;
    //FIX-WX: wxListItem have a private assignment operator

    IRowFilter* m_Filter;
    bool m_FilterInvalid;

    CIRef<IRowSorter> m_Sorter;
    ESortedState m_Sorted;
    bool m_SortInvalid;
    bool m_PrepareSortHint;

    vector<TSortedCol> m_SortedCols;
    //int m_SortedCol;

    WX_DECLARE_STRING_HASH_MAP( IwxVariantSorter*, TSorterMap );
    mutable TSorterMap m_DefaultSorters;

    CFixGenericListCtrl m_FixGenericListCtrl;

    /// Last column user clicked on (tooltips are only on selected/clicked itmes)
    int m_CurrentCol;
    /// Last row user clicked on
    int m_CurrentRow;
    /// Test to put in tooltip
    wxString m_ToolTipText;
    /// Start/stop x value of cell to which tooltip belongs
    mutable int m_TipStartX;
    mutable int m_TipEndX;

    // ToolTip members
    wxTimer m_Timer;

    DECLARE_DYNAMIC_CLASS(CwxTableListCtrl)
    DECLARE_NO_COPY_CLASS(CwxTableListCtrl)
    DECLARE_EVENT_TABLE()
};

END_NCBI_SCOPE

#endif // GUI_WIDGETS_WX___TABLE_LIST_CTRL__HPP
