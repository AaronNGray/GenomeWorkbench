#ifndef GUI_WIDGETS_GRID_WIDGET___GRID_WIDGET__HPP
#define GUI_WIDGETS_GRID_WIDGET___GRID_WIDGET__HPP

/*  $Id: grid_widget.hpp 37201 2016-12-14 21:00:47Z falkrb $
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
 * Authors:  Roman Katargin
 *
 * File Description:
 *
 */

#include <corelib/ncbiobj.hpp>
#include <gui/gui_export.h>

#include <wx/panel.h>
#include <wx/timer.h>
#include <wx/statline.h>

#include <gui/utils/event_handler.hpp>
#include <gui/objutils/reg_settings.hpp>

#include <gui/objutils/table_data.hpp>
#include <gui/objutils/table_selection.hpp>
#include <gui/objutils/grid_table_adapter.hpp>
#include <gui/objutils/query_widget.hpp>
#include <gui/objutils/td_query_data_source.hpp>

#include <gui/widgets/data/query_parse_panel.hpp>

class wxGridEvent;
class wxGridRangeSelectEvent;
class wxHyperlinkEvent;
class wxBoxSizer;

BEGIN_NCBI_SCOPE

class CGrid;
class CwxGridTableAdapter;
class CQueryParsePanel;
struct SCSVExportParams;
class ICommandProccessor;
class CGridWidget;
class CMacroQueryExec;

// event handler interface for events desired to be handled in a custom way
// originally designed for handling custom menu commands
class IGridEvtExt : public wxEvtHandler
{
protected:
    friend class CGridWidget;
    void x_SetGridWidget(CGridWidget* pGridWidget) { m_pGridWidget = pGridWidget; }
    CGridWidget* x_GetGridWidget() { return m_pGridWidget; }

private:
    CGridWidget* m_pGridWidget;
};


class NCBI_GUIWIDGETS_GRID_WIDGET_EXPORT CGridWidget
    : public wxPanel, public CEventHandler, public IRegSettings, public IQueryWidget
{
public:
    CGridWidget();
    CGridWidget(wxWindow* parent, wxWindowID id = wxID_ANY,
                const wxPoint& pos = wxDefaultPosition,
                const wxSize& size = wxDefaultSize,
                long style = wxBORDER_NONE);
    ~CGridWidget();

    bool Create(wxWindow* parent, wxWindowID id = wxID_ANY,
                const wxPoint& pos = wxDefaultPosition,
                const wxSize& size = wxDefaultSize,
                long style = wxBORDER_NONE);

    /// Creates the controls and sizers
    void CreateControls();

    void Init(ITableData& table_data, ICommandProccessor* cmdProccessor);

    void SetTableSelection(ITableSelection* tableSelection) { m_TableSelection.Reset(tableSelection); }

    void UseFixedFont();
    void ShowGridLines(bool show) { m_isShowGridLines = show; }

    void SetUseCursorSelection(bool useCursorSelection) { m_UseCursorSelection = useCursorSelection; }

    void SetSelection(CSelectionEvent& evt);

    /// Get all selected objects
    void GetSelection(CSelectionEvent& evt) const;
    void GetSelectedObjects (TConstScopedObjects& objects) const;
    void GetSelectedRows(set<int> &rows) const;

    /// Get objects in a priority order (favor row selections)
    void GetSelectedObjectsOrdered (TConstScopedObjects& objects);

    void OnGridCellRightClick(wxGridEvent& evt);
    void OnGridCellLeftDClick(wxGridEvent& evt);
    void OnGridLabelLeftDClick(wxGridEvent& evt);
    void OnGridSelectCell(wxGridEvent& evt);
    void OnGridRangeSelect(wxGridRangeSelectEvent& evt);
    void OnExportToCSV(wxCommandEvent& event);
    void OnSearchIncludeCellValue(wxCommandEvent& event);
    void OnSearchExcludeCellValue(wxCommandEvent& event);
    void OnTimer(wxTimerEvent& event);
    void OnHyperlinkHover(wxHyperlinkEvent& event);
    void OnCopy(wxCommandEvent& event);
    void OnUpdateCopy(wxUpdateUIEvent& event);
    void OnRefreshClick(wxCommandEvent& event);

    /// @name IRegSettings interface implementation
    /// @{
    virtual void SetRegistryPath(const string& reg_path);
    virtual void LoadSettings();
    virtual void SaveSettings() const;
    /// @}

    /// @name IQueryWidget interface implementation
    /// @{
    virtual string GetSearchHelpAddr() const
    { return string(""); }

    /// Disable any widgets that the user should not use during the query
    virtual void QueryStart();
    /// Re-enable any widgets disabled during the query.
    virtual void QueryEnd(CMacroQueryExec* exec);

    /// Advance to previous/next selected row from query (in current sort order)
    void IterateSelection(int dir);
    /// Set to true to show all rows selected by most recent query as selected
    virtual void SetSelectAll(bool b);

    /// If true, only rows that were selected by prevous query will be shown
    virtual void SetHideUnselected(bool b);
    /// @}

    void ShowRefreshButton(bool show) { m_isShowRefreshButton = show; }
    /// Hide the Select All checkbox from the user
    void HideSelectAll(void) { m_ShowSelectAll = false; }
    void SetEventExt(IGridEvtExt* evtExt);

    CGrid* GetGrid() { return m_Grid; }

protected:
    DECLARE_EVENT_TABLE()

    void x_InitGrid(ICommandProccessor* cmdProccessor);
    void x_GetSelectedRows(set<int>& rows) const;
    void x_CompleteQuery();
    void x_ScrollToRow(int row_idx);
    void x_UpdateRowsToSelection(bool b);
    void x_LeftClick(int Row, int modifiers);
    void x_LeftDClick(int Row);

    bool x_GetRectSelection(vector<int>& rows, vector<int>&cols);

    /// the grid widget
    CGrid* m_Grid;
    /// interface to the table data (unsorted)
    CIRef<ITableData> m_TableData;
    /// adapter that matches the grid cells to sorted entries from m_TableData
    CwxGridTableAdapter* m_GridAdapter;
    /// post-cell select timer

    CIRef<ITableSelection> m_TableSelection;

    wxTimer m_Timer;

    /// toolbar that contains the query toolbar and additional buttons
    /// added by AddControl() (Refresh)
    wxBoxSizer* m_ToolbarSizer;

    /// controls assigned to the toolbar:
    CQueryParsePanel* m_QueryPanel;
    wxStaticLine* m_StaticLine;
    wxButton* m_RefreshButton;

    string m_RegPath;
    /// when true, rows selected from query panel are all shown as selected
    bool m_SelectAll;
    /// when true (by default) the Select All checkbox is shown 
    bool m_ShowSelectAll;

    /// Remember location of cell from popup (right click) so we can use that
    /// location (potentially) in function called by popup
    int m_PopupGridX;
    int m_PopupGridY;

    CRef<CTDQueryDataSource> m_QueryDS;

    CIRef<IGridEvtExt> m_GridEvtExt;
    bool m_isShowRefreshButton;
    bool m_isShowGridLines;

private:
    enum {
        //!! how do the numbers get assigned so they do not clash?
        ID_REFRESH_BUTTON = 13027,
    };

    bool m_UseCursorSelection;
};

END_NCBI_SCOPE

#endif  // GUI_WIDGETS_GRID_WIDGET___GRID_WIDGET__HPP
