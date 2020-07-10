/*  $Id: grid_widget.cpp 43722 2019-08-22 19:24:57Z katargir $
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

#include <ncbi_pch.hpp>

#include <gui/widgets/grid_widget/grid_widget.hpp>

#include <gui/utils/view_event.hpp>
#include <gui/utils/event_handler.hpp>
#include <gui/utils/extension_impl.hpp>

#include <gui/objutils/table_data_base.hpp>
#include <gui/objutils/obj_event.hpp>
#include <gui/objutils/macro_query_exec.hpp>

#include <gui/widgets/wx/wx_utils.hpp>
#include <gui/widgets/wx/ui_command.hpp>
#include <gui/widgets/data/object_contrib.hpp>

#include <gui/widgets/grid_widget/wxgrid_table_adapter.hpp>
#include <gui/widgets/grid_widget/grid.hpp>
#include <gui/widgets/grid_widget/grid_event.hpp>
#include <gui/widgets/wx/csv_export_dlg.hpp>
#include <gui/widgets/wx/grid_table_export.hpp>
#include <gui/widgets/data/query_parse_panel.hpp>

#include <wx/sizer.h>
#include <wx/menu.h>
#include <wx/button.h>
#include <wx/msgdlg.h>
#include <wx/settings.h>

#include <wx/sstream.h>
#include <wx/txtstrm.h>
#include <wx/clipbrd.h>
#include <wx/dataobj.h>

#include <functional>
#include <sstream>

BEGIN_NCBI_SCOPE

#define ID_EXPORT_TO_CSV        10100
#define ID_SEARCH_INCLUDE_VALUE 10101
#define ID_SEARCH_EXCLUDE_VALUE 10102

BEGIN_EVENT_TABLE( CGridWidget, wxPanel )
EVT_GRID_CELL_RIGHT_CLICK(CGridWidget::OnGridCellRightClick)
EVT_GRID_CELL_LEFT_DCLICK(CGridWidget::OnGridCellLeftDClick)
EVT_GRID_LABEL_LEFT_DCLICK(CGridWidget::OnGridLabelLeftDClick)
EVT_GRID_SELECT_CELL(CGridWidget::OnGridSelectCell)
EVT_GRID_RANGE_SELECT(CGridWidget::OnGridRangeSelect)
EVT_TIMER(-1, CGridWidget::OnTimer)
EVT_MENU(ID_EXPORT_TO_CSV, CGridWidget::OnExportToCSV)
EVT_MENU(ID_SEARCH_INCLUDE_VALUE, CGridWidget::OnSearchIncludeCellValue)
EVT_MENU(ID_SEARCH_EXCLUDE_VALUE, CGridWidget::OnSearchExcludeCellValue)
EVT_HYPERLINK_HOVER(wxID_ANY, CGridWidget::OnHyperlinkHover)
EVT_MENU(wxID_COPY, CGridWidget::OnCopy)
EVT_UPDATE_UI(wxID_COPY, CGridWidget::OnUpdateCopy)
EVT_BUTTON( ID_REFRESH_BUTTON, CGridWidget::OnRefreshClick )
END_EVENT_TABLE()

CGridWidget::CGridWidget()
: m_Grid(NULL)
, m_GridAdapter(NULL)
, m_Timer(this)
, m_ToolbarSizer(NULL)
, m_QueryPanel(NULL)
, m_StaticLine(NULL)
, m_RefreshButton(NULL)
, m_SelectAll(true)
, m_ShowSelectAll(true)
, m_PopupGridX(-1)
, m_PopupGridY(-1)
, m_isShowRefreshButton(false)
, m_isShowGridLines(true)
, m_UseCursorSelection(true)
{
}

CGridWidget::CGridWidget(wxWindow* parent, wxWindowID id,
                         const wxPoint& pos, const wxSize& size, long style)
    : m_Grid(), m_GridAdapter(), m_Timer(this),  m_ToolbarSizer(NULL)
    , m_QueryPanel(NULL)
    , m_StaticLine(NULL)
    , m_RefreshButton(NULL)
    , m_SelectAll(true)
    , m_ShowSelectAll(true)
    , m_isShowRefreshButton(false)
    , m_isShowGridLines(true)
    , m_UseCursorSelection(true)
{
    Create(parent, id, pos, size, style);
}

CGridWidget::~CGridWidget()
{
    if (m_QueryPanel)
        m_QueryPanel->RemoveListener(this);

    if (m_GridEvtExt) {
        PopEventHandler();
    }
}

bool CGridWidget::Create(wxWindow* parent, wxWindowID id,
                         const wxPoint& pos, const wxSize& size, long style)
{
#ifdef __WXOSX_COCOA__ // GB-8581
    SetBackgroundStyle(wxBG_STYLE_COLOUR);
    SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_FRAMEBK));
#endif

    if ( ! wxPanel::Create(parent, id, pos, size, style))
        return false;

    CreateControls();

    return true;
}

void CGridWidget::SetEventExt(IGridEvtExt* evtExt)
{
    if (m_GridEvtExt) {
        PopEventHandler();
    }

    m_GridEvtExt.Reset(evtExt);

    if(m_GridEvtExt) {
        m_GridEvtExt->x_SetGridWidget(this);
        PushEventHandler(m_GridEvtExt);
    }
}

void CGridWidget::CreateControls()
{
    wxBoxSizer* itemBoxSizer = new wxBoxSizer(wxVERTICAL);
    this->SetSizer(itemBoxSizer);

    m_Grid = new CGrid(this, wxID_ANY);
    itemBoxSizer->Add(m_Grid, 1, wxGROW|wxALL, 0);

   // x_InitGrid(NULL);
}

void CGridWidget::Init(ITableData& table_data, ICommandProccessor* cmdProccessor)
{
    m_TableData.Reset(&table_data);
    x_InitGrid(cmdProccessor);
}

void CGridWidget::UseFixedFont()
{
    wxFont font = m_Grid->GetDefaultCellFont();
    wxFont fixed_font(font.GetPointSize(), wxFONTFAMILY_MODERN, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL);
    fixed_font.SetSymbolicSize(wxFONTSIZE_LARGE);
    m_Grid->SetDefaultCellFont(fixed_font);
}

static const char* kQuery = ".Query";

void CGridWidget::x_InitGrid(ICommandProccessor* cmdProccessor) {
    if (!m_Grid)
        return;

    if (m_TableData) {
        m_GridAdapter = new CwxGridTableAdapter(*m_TableData, cmdProccessor);
        if (!m_RegPath.empty()) {
            m_GridAdapter->SetRegistryPath(m_RegPath + kQuery);
            m_GridAdapter->LoadSettings();
        }

        m_ToolbarSizer = new wxBoxSizer(wxHORIZONTAL);

        if (m_QueryPanel)
            m_QueryPanel->Destroy();
        m_QueryDS.Reset(new CTDQueryDataSource(*m_TableData));
        m_QueryPanel = new CQueryParsePanel(this, m_QueryDS);
        m_QueryPanel->Create(this);
        m_QueryPanel->AddListener(this, ePool_Parent);

        string section = m_TableData->GetTableTypeId();
        CGuiRegistry& gui_reg = CGuiRegistry::GetInstance();
        CRegistryReadView view = gui_reg.GetReadView("TableDataQuery." + section);
        vector<string> vec;
        view.GetStringVec("Defaults", vec);
        CQueryParsePanel::TNamedQueries defaults;
        for (size_t i = 0; i < vec.size()/2; ++i) {
            defaults.push_back
                (CQueryParsePanel::TNamedQueries::value_type(vec[2*i], vec[2*i + 1]));
        }
        m_QueryPanel->SetDefaultQueries(defaults);

        if (!m_RegPath.empty()) {
            m_QueryPanel->SetRegistryPath(m_RegPath);
            m_QueryPanel->LoadSettings();
        }

        m_ToolbarSizer->Add(m_QueryPanel, wxGROW|wxBOTTOM|wxALIGN_LEFT);

        // adding it should be configurable by the widget user
        if(m_RefreshButton) {
            m_RefreshButton->Destroy();
        }
        if(m_isShowRefreshButton) {
            m_RefreshButton = new wxButton( this, ID_REFRESH_BUTTON, _("Refresh"), wxDefaultPosition, wxDefaultSize, 0 );
            m_ToolbarSizer->Add(m_RefreshButton, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);
        }
        if (!m_ShowSelectAll) {
            m_QueryPanel->HideSelectAll();
            m_SelectAll = false;
        }

        GetSizer()->Insert(0, m_ToolbarSizer, 0, wxGROW|wxBOTTOM|wxALIGN_LEFT);
        if(m_StaticLine) {
            m_StaticLine->Destroy();
        }
        m_StaticLine = new wxStaticLine( this, wxID_STATIC, wxDefaultPosition, wxDefaultSize, wxLI_HORIZONTAL );
        GetSizer()->Insert(1, m_StaticLine, 0, wxGROW, 1);
        m_Grid->SetTable(m_GridAdapter, true);

        LoadSettings();
        GetSizer()->Layout();

        m_Grid->EnableEditing(true);

        m_Grid->EnableGridLines(m_isShowGridLines);
    }
}

void CGridWidget::OnHyperlinkHover(wxHyperlinkEvent& event)
{
    CGridWidgetEvent evt(CGridWidgetEvent::eUrlHover, event.GetURL());
    Send(&evt, ePool_Parent);
}

void CGridWidget::OnGridCellRightClick(wxGridEvent& evt)
{
    if (!m_TableData || evt.GetCol() < 0)
        return;


    wxMenu menu;

    if (m_GridAdapter) {
        auto_ptr<wxMenu> edit_menu(m_GridAdapter->CreateMenu());
        if (edit_menu.get()) {
            Merge(menu, *edit_menu);
        }
    }

    TConstScopedObjects sel_objects;
    GetSelectedObjects(sel_objects);
    if (!sel_objects.empty()) {
        vector< CIRef<IObjectCmdContributor> > contributors;
        GetExtensionAsInterface(EXT_POINT__SCOPED_OBJECTS__CMD_CONTRIBUTOR, contributors);

        for( size_t i = 0; i < contributors.size(); i++ ){
            IObjectCmdContributor& obj = *contributors[i];

            IObjectCmdContributor::TContribution contrib = obj.GetMenu(sel_objects);
            auto_ptr<wxMenu> obj_menu(contrib.first);
            if (obj_menu.get()) {
                Merge(menu, *obj_menu);
            }
        }
    }

    if (menu.GetMenuItemCount() > 0) {
        menu.AppendSeparator();
    }
    menu.Append(wxID_COPY);
    menu.Append(ID_EXPORT_TO_CSV, "Export to CSV...");
    menu.Append(ID_SEARCH_INCLUDE_VALUE, "Filter Include this Value");
    menu.Append(ID_SEARCH_EXCLUDE_VALUE, "Filter Exclude this Value");


    m_PopupGridX = evt.GetCol();
    m_PopupGridY = evt.GetRow();

    CleanupSeparators(menu);

    m_Grid->PopupMenu(&menu, evt.GetPosition());
}

void CGridWidget::OnGridCellLeftDClick(wxGridEvent& evt)
{
    x_LeftDClick(evt.GetRow());
}

void CGridWidget::OnGridLabelLeftDClick(wxGridEvent& evt)
{
    if (evt.GetRow() < 0)
        return;

    x_LeftDClick(evt.GetRow());
}

void CGridWidget::x_LeftClick(int row, int modifiers)
{
    CGridWidgetEvent event(CGridWidgetEvent::eRowClick);
    event.SetSelectedRow(m_GridAdapter->GetOriginalRow(row));
    event.SetModifiers(modifiers);
    Send(&event, ePool_Parent);
}

void CGridWidget::x_LeftDClick(int row)
{
    CGridWidgetEvent event(CGridWidgetEvent::eRowDClick);
    event.SetSelectedRow(m_GridAdapter->GetOriginalRow(row));
    Send(&event, ePool_Parent);
}

static bool s_BlockSelChangeUpdate = false;

void CGridWidget::OnGridSelectCell(wxGridEvent& evt)
{
    if (s_BlockSelChangeUpdate)
        return;

    x_LeftClick(evt.GetRow(), evt.GetModifiers());
    m_Timer.Start(300, true);
    evt.Skip();
}

void CGridWidget::OnGridRangeSelect(wxGridRangeSelectEvent& evt)
{
    if (s_BlockSelChangeUpdate)
        return;

    if(evt.GetTopRow() == evt.GetBottomRow()) {
        x_LeftClick(evt.GetTopRow(), evt.GetModifiers());
    }
    m_Timer.Start(300, true);
    evt.Skip();
}

void CGridWidget::OnTimer(wxTimerEvent&)
{
    CEvent evt(CEvent::eEvent_Message, CViewEvent::eWidgetSelectionChanged);
    Send(&evt, ePool_Parent);
}

void CGridWidget::QueryStart()
{
    m_Grid->Disable();
}

void CGridWidget::QueryEnd(CMacroQueryExec* /*exec*/)
{
    x_CompleteQuery();
    m_Grid->Enable();
}

void CGridWidget::x_CompleteQuery()
{
    m_Grid->BeginBatch();

    m_Grid->ClearSelection();

    // If the view is currently only showing selected rows (selected from
    // a previous query), undo that since we now (probably) have
    // different rows selected.
    bool hide_unselected = m_GridAdapter->GetHideUnselected();
    if (hide_unselected) {
        // Do this before updating the selection set sets it uses that
        // for the number calculations.
        x_UpdateRowsToSelection(false);
    }

    vector<size_t> query_results = m_QueryDS->GetQueryResults();
    m_GridAdapter->SetSelection(query_results);

    if (m_QueryPanel->IsSelectAll()) {
        // Sets currently selected row to first row in selection (in select-all
        // mode we need a current selection in case we switch to single-select)
        m_GridAdapter->IterateSelection(0);

        // Updated the displayed rows in the grid widget. SetHideUnselected()
        // does that for us, if we are in hide-unselected mode.
        if (hide_unselected) {
            SetHideUnselected(true);
        }
        else {
            for (size_t i=0; i<query_results.size(); ++i) {
                size_t row_idx = query_results[i];
                size_t current_idx = m_GridAdapter->GetCurrentRow(row_idx);
                if (i == 0) {
                    m_Grid->SetGridCursor(current_idx, 0);
                }
                if (current_idx < (size_t)m_Grid->GetNumberRows()) {
                    m_Grid->SelectRow(current_idx, true);
                }
            }
        }
    }
    else {
        // In single-select mode.  Set current selection as the first

        if (hide_unselected) {
            x_UpdateRowsToSelection(true);
        }

        // Set first selected element as current selection
        IterateSelection(0);
    }

    m_Grid->EndBatch();
}

void CGridWidget::SetSelectAll(bool b)
{
    m_Grid->BeginBatch();

    m_SelectAll = b;

    m_Grid->ClearSelection();

    if (b) {
        vector<size_t> query_results = m_QueryDS->GetQueryResults();

        for (size_t i=0; i<query_results.size(); ++i) {
            size_t row_idx = query_results[i];
            size_t current_idx = m_GridAdapter->GetCurrentRow(row_idx);
            if (current_idx < (size_t)m_Grid->GetNumberRows()) {
                m_Grid->SelectRow(current_idx, true);
            }
        }
    }
    else {
        size_t row_idx = m_GridAdapter->GetCurrentSelection();
        size_t current_idx = m_GridAdapter->GetCurrentRow(row_idx);
        if (current_idx < (size_t)m_Grid->GetNumberRows()) {
            m_Grid->SelectRow(current_idx, true);
            x_ScrollToRow(current_idx);
        }
    }

    m_Grid->EndBatch();
}

void CGridWidget::IterateSelection(int dir)
{
    if (!m_QueryPanel->IsSelectAll()) {
        m_Grid->ClearSelection();

        m_GridAdapter->IterateSelection(dir);

        size_t data_row_idx = m_GridAdapter->GetCurrentSelection();
        size_t grid_row_idx = m_GridAdapter->GetCurrentRow(data_row_idx);
        if (grid_row_idx < (size_t)m_Grid->GetNumberRows()) {
            m_Grid->SelectRow(grid_row_idx, true);
            x_ScrollToRow(grid_row_idx);
        }
    }
}

void CGridWidget::SetHideUnselected(bool b)
{
    x_UpdateRowsToSelection(b);

    SetSelectAll(m_SelectAll);

    m_Grid->Refresh();
}

void CGridWidget::SetSelection(CSelectionEvent& evt)
{
    CBoolGuard _guard(s_BlockSelChangeUpdate);

    vector<size_t> origRows;

    if (m_TableSelection) {
        m_TableSelection->GetRows(*m_TableData, evt, origRows);
    } else {
        TConstObjects objs;
        evt.GetAllObjects(objs);
	    evt.GetOther(objs);
        set<const CObject*> objSet(objs.begin(), objs.end());

        for (int i = 0; i < m_GridAdapter->GetRowsCount(); ++i) {
            size_t row = m_GridAdapter->GetOriginalRow(i);
            SConstScopedObject obj = m_TableData->GetRowObject(row);
            if (obj.object && objSet.find(obj.object) != objSet.end())
                origRows.push_back(row);
        }
    }

    m_Grid->BeginBatch();

    m_Grid->ClearSelection();

    set<size_t> rowSet(origRows.begin(), origRows.end());
    for (int i = 0; i < m_GridAdapter->GetRowsCount(); ++i) {
        size_t row = m_GridAdapter->GetOriginalRow(i);
        if (rowSet.find(row) != rowSet.end())
            m_Grid->SelectRow(row, true);
    }

    m_Grid->EndBatch();
}

void CGridWidget::GetSelectedRows(set<int> &rows) const
{
    set<int> raw_rows;
    
    if (!m_Grid)
        return;
    
// Block selection
    wxGridCellCoordsArray selTopLeft = m_Grid->GetSelectionBlockTopLeft();
    wxGridCellCoordsArray selBottomRight = m_Grid->GetSelectionBlockBottomRight();
    size_t blockCount = selTopLeft.GetCount();
    
    if (blockCount > 0 && blockCount == selBottomRight.GetCount()) {
        for (size_t i = 0; i < blockCount; ++i) {
            for (int j = selTopLeft[i].GetRow(); j <= selBottomRight[i].GetRow(); ++j)
                raw_rows.insert(j);
        }
    }
    
// Row selection
    wxArrayInt selRows = m_Grid->GetSelectedRows();
    for (size_t i = 0; i < selRows.size(); ++i)
        raw_rows.insert(selRows[i]);
    
// Cell Selection
    wxGridCellCoordsArray selCells = m_Grid->GetSelectedCells();
    for (size_t i = 0; i < selCells.GetCount(); ++i)
        raw_rows.insert(selCells[i].GetRow());


    for (set<int>::const_iterator it = raw_rows.begin(); it != raw_rows.end(); ++it) {
        size_t row = m_GridAdapter->GetOriginalRow((size_t)*it);
        if (row != (size_t)-1)
            rows.insert((int)row);
    }
}

void CGridWidget::GetSelection(CSelectionEvent& evt) const
{
    if (m_TableSelection) {
        set<int> selRows;
        x_GetSelectedRows(selRows);
        vector<size_t> origRows;

        for (set<int>::const_iterator it = selRows.begin(); it != selRows.end(); ++it) {
            size_t row = m_GridAdapter->GetOriginalRow((size_t)*it);
            if (row != (size_t)-1)
                origRows.push_back(row);
        }

        m_TableSelection->GetSelection(*m_TableData, origRows, evt);
    } else {
        TConstScopedObjects sobjs;
        GetSelectedObjects (sobjs);

        TConstObjects objs;
        ITERATE(TConstScopedObjects, it, sobjs) {
            objs.push_back(it->object);
        }

        evt.AddObjectSelection(objs);        
    }
}


void CGridWidget::GetSelectedObjects (TConstScopedObjects& objects)  const
{
    set<int> selRows;
    x_GetSelectedRows(selRows);

    vector<size_t> origRows;

    for (set<int>::const_iterator it = selRows.begin(); it != selRows.end(); ++it) {
        size_t row = m_GridAdapter->GetOriginalRow((size_t)*it);
        if (row != (size_t)-1)
            origRows.push_back(row);
    }

    for (size_t col = 0; col < m_TableData->GetColsCount(); ++col) {
        if (m_TableData->GetColumnType(col) == ITableData::kObject) {
            vector<size_t>::const_iterator it2;
            for (it2 = origRows.begin(); it2 != origRows.end(); ++it2) {
                SConstScopedObject so = m_TableData->GetObjectValue(*it2, col);
                if (so.object)
                    objects.push_back(so);
            }
        }
    }

    vector<size_t>::const_iterator it3;
    for (it3 = origRows.begin(); it3 != origRows.end(); ++it3) {
        SConstScopedObject so = m_TableData->GetRowObject(*it3);
        if (so.object)
            objects.push_back(so);
    }
}

void CGridWidget::GetSelectedObjectsOrdered(TConstScopedObjects& objects)
{
    // Return selected objecst for users performing an action such as a menu
    // action for a single item in a priority order. This is a little arbitrary, but
    // a reasonably intuitive selection hierarchy might be:
    // 1. First fully selected row
    // 2. Grid selection (from first row)
    // 3. Grid cursor row - if nothing else is selected  (note that the grid cursor
    //    may sometimes not be visible (think this is a wx bug) so we should only
    //    use it if nothing else is selected

    /// Row selection
    vector<int> selRows;
    wxArrayInt rows = m_Grid->GetSelectedRows();
    for (size_t i = 0; i < rows.size(); ++i)
        selRows.push_back(rows[i]);

    // Block selection
    if (selRows.size() == 0) {
        wxGridCellCoordsArray selTopLeft = m_Grid->GetSelectionBlockTopLeft();
        wxGridCellCoordsArray selBottomRight = m_Grid->GetSelectionBlockBottomRight();
        size_t blockCount = selTopLeft.GetCount();

        if (blockCount > 0 && blockCount == selBottomRight.GetCount()) {
            for (size_t i = 0; i < blockCount; ++i) {
                for (int j = selTopLeft[i].GetRow(); j <= selBottomRight[i].GetRow(); ++j)
                    selRows.push_back(j);
            }
        }
    }

    // Cell Selection
    if (selRows.size() == 0) {
        wxGridCellCoordsArray selCells = m_Grid->GetSelectedCells();
        for (size_t i = 0; i < selCells.GetCount(); ++i)
            selRows.push_back(selCells[i].GetRow());
    }

    // Grid cursor selection
    if (selRows.size() == 0) {
        int curRow = m_Grid->GetGridCursorRow();
        if (curRow >= 0)
            selRows.push_back(curRow);
    }

    vector<size_t> origRows;

    for (vector<int>::const_iterator it = selRows.begin(); it != selRows.end(); ++it) {
        size_t row = m_GridAdapter->GetOriginalRow((size_t)*it);
        _TRACE("Sel row: " << *it << " orig row: " << row);
        if (row != (size_t)-1)
            origRows.push_back(row);
    }

    for (size_t col = 0; col < m_TableData->GetColsCount(); ++col) {
        if (m_TableData->GetColumnType(col) == ITableData::kObject) {
            vector<size_t>::const_iterator it2;
            for (it2 = origRows.begin(); it2 != origRows.end(); ++it2) {
                SConstScopedObject so = m_TableData->GetObjectValue(*it2, col);
                if (so.object)
                    objects.push_back(so);
            }
        }
    }

    vector<size_t>::const_iterator it3;
    for (it3 = origRows.begin(); it3 != origRows.end(); ++it3) {
        SConstScopedObject so = m_TableData->GetRowObject(*it3);
        if (so.object)
            objects.push_back(so);
    }
}

void CGridWidget::x_ScrollToRow(int row_idx)
{
    bool visible = m_Grid->IsVisible(row_idx, 0, false);
    if (!visible) {
        // Need to scroll to new row via MakeCellVisible, and then
        // to the correct horizontal position via Scroll(x,y) which
        // keeps the updated row (y) position and scrolls to the
        // previous x (horizonal) position.  This does cause some
        // flicker since both commands seem to update the window
        // immediately (at least on windows) despite using
        // wxGrids beginbatch/endbatch.
        m_Grid->BeginBatch();
        int x, xdummy, y;
        m_Grid->GetViewStart(&x, &y);
        m_Grid->MakeCellVisible(row_idx, 0);
        m_Grid->GetViewStart(&xdummy,&y);
        m_Grid->Scroll(x, y);
        m_Grid->EndBatch();
    }
}

void CGridWidget::x_GetSelectedRows(set<int>& rows) const
{
    if (!m_Grid)
        return;

// Block selection
    wxGridCellCoordsArray selTopLeft = m_Grid->GetSelectionBlockTopLeft();
    wxGridCellCoordsArray selBottomRight = m_Grid->GetSelectionBlockBottomRight();
    size_t blockCount = selTopLeft.GetCount();

    if (blockCount > 0 && blockCount == selBottomRight.GetCount()) {
        for (size_t i = 0; i < blockCount; ++i) {
            for (int j = selTopLeft[i].GetRow(); j <= selBottomRight[i].GetRow(); ++j)
                rows.insert(j);
        }
    }

// Row selection
    wxArrayInt selRows = m_Grid->GetSelectedRows();
    for (size_t i = 0; i < selRows.size(); ++i)
        rows.insert(selRows[i]);

// Cell Selection
    wxGridCellCoordsArray selCells = m_Grid->GetSelectedCells();
    for (size_t i = 0; i < selCells.GetCount(); ++i)
        rows.insert(selCells[i].GetRow());

    if (m_UseCursorSelection && rows.empty()) {
        int curRow = m_Grid->GetGridCursorRow();
        if (curRow >= 0)
            rows.insert(curRow);
    }
}

void CGridWidget::x_UpdateRowsToSelection(bool b)
{
    int count1 = m_GridAdapter->GetNumberRows();
    m_GridAdapter->HideUnselected(b);
    int count2 = m_GridAdapter->GetNumberRows();

    // Send message to grid to make it update the number of displayed rows. It does not
    // do this automatically just because the datasource changes the number of rows
    // it is reporting.
    if (count1 > count2) {
        wxGridTableMessage msg( m_GridAdapter, wxGRIDTABLE_NOTIFY_ROWS_DELETED, count2, count1-count2);
        m_Grid->ProcessTableMessage(msg);
    }
    else if (count1 < count2) {
        wxGridTableMessage msg( m_GridAdapter, wxGRIDTABLE_NOTIFY_ROWS_APPENDED, count2-count1);
        m_Grid->ProcessTableMessage(msg);
    }
}

void CGridWidget::SetRegistryPath(const string& reg_path)
{
    m_RegPath = reg_path;

    if (m_GridAdapter) {
        m_GridAdapter->SetRegistryPath(m_RegPath + kQuery);
        m_GridAdapter->LoadSettings();
    }

    if (m_QueryPanel) {
        m_QueryPanel->SetRegistryPath(m_RegPath);
        m_QueryPanel->LoadSettings();
    }
}

void CGridWidget::LoadSettings()
{
}

void CGridWidget::SaveSettings() const
{
    if (m_GridAdapter) {
        m_GridAdapter->SaveSettings();
    }

    if (m_QueryPanel) {
        m_QueryPanel->SaveSettings();
    }

}

void CGridWidget::OnExportToCSV(wxCommandEvent& WXUNUSED(event))
{
    if (!m_GridAdapter)
        return;

    CwxCSVExportDlg dlg(this);

    vector<wxString> selectedColumns;
    const int colNum = m_GridAdapter->GetNumberCols();
    for (int i = 0; i < colNum; ++i) {
        selectedColumns.push_back(m_GridAdapter->GetColLabelValue(i));
    }
    dlg.SetColumnsList(selectedColumns);
    dlg.Layout();
    dlg.Fit();
    dlg.Refresh();

    string regPath = m_GridAdapter->GetRegPath();
    if (!regPath.empty())
        dlg.SetRegistryPath(regPath + ".ExportToCSV");

    if (dlg.ShowModal() != wxID_OK)
        return;

    wxString fileName = dlg.GetFileName();
    if (fileName.empty() )
        return;

    wxString err_msg;

    try {
        bool withHeader = dlg.GetWithHeaders();
        bool selected_only = dlg.GetSelectedOnly();

        auto_ptr<CNcbiOstream> os(new CNcbiOfstream(fileName.fn_str(), IOS_BASE::out));
        if (os.get() == NULL)
            NCBI_THROW(CException, eUnknown, "File is not accessible");

        if (selected_only) {
            vector<int> rows, cols;
            if (!x_GetRectSelection(rows, cols)) {
                wxMessageBox(wxT("Can't export non rectangular selection."), wxT("Export to CSV"),
                                wxOK | wxICON_EXCLAMATION, this);
                return;
            }

            if (rows.empty() || cols.empty())
                return;

            CwxGridTableExport(*m_GridAdapter, *os, rows, cols, withHeader, ',');
        } else {
            vector<int> cols;
            dlg.GetSelectedColumns(cols);
            if (cols.size() != colNum) {
                vector<int> rows;
                for (int i = 0; i < m_GridAdapter->GetRowsCount(); ++i) {
                    rows.push_back(i);
                }
                CwxGridTableExport(*m_GridAdapter, *os, rows, cols, withHeader, ',');
            }
            else
                CwxGridTableExport(*m_GridAdapter, *os, withHeader, ',');
        }
        os->flush();
    }
    catch (const CException& e) {
        err_msg = ToWxString(e.GetMsg());
    }

    if (!err_msg.empty()) {
        wxMessageBox(wxT("Failed to save file: ") + err_msg, wxT("Error"),
                wxOK | wxICON_ERROR, this);
    }
}

void CGridWidget::OnSearchIncludeCellValue(wxCommandEvent& WXUNUSED(event))
{
    if (m_PopupGridX < 0 || m_PopupGridX >= m_GridAdapter->GetColsCount() ||
        m_PopupGridY < 0 || m_PopupGridY >= m_GridAdapter->GetRowsCount())
            return;

    string col_name = ToStdString(m_GridAdapter->GetColLabelValue(m_PopupGridX));
    string cell_value;

    cell_value = ToStdString(m_GridAdapter->GetValue(m_PopupGridY, m_PopupGridX));
    cell_value = NStr::TruncateSpaces(cell_value);

    // Add quotes if needed - for any cell that is not a number
    if (cell_value=="") {
        cell_value = "\"\"";
    }
    else if ((cell_value.find_first_of(' ') != string::npos) ||
             (cell_value.find_first_of('\t') != string::npos)) {
            cell_value = "\"" + cell_value + "\"";
    }
    else {
        try {
            NStr::StringToDouble(cell_value);
        }
        catch (CStringException&) {
            cell_value = "\"" + cell_value + "\"";
        }
    }

    string query_constraint = "(" + col_name + " = " + cell_value + ")";

    m_QueryPanel->AddQueryText(query_constraint);
}

void CGridWidget::OnSearchExcludeCellValue(wxCommandEvent& WXUNUSED(event))
{
    if (m_PopupGridX < 0 || m_PopupGridX >= m_GridAdapter->GetColsCount() ||
        m_PopupGridY < 0 || m_PopupGridY >= m_GridAdapter->GetRowsCount())
            return;

    string col_name = ToStdString(m_GridAdapter->GetColLabelValue(m_PopupGridX));
    string cell_value = ToStdString(m_GridAdapter->GetValue(m_PopupGridY, m_PopupGridX));

    string query_constraint = "(" + col_name + " != " + cell_value + ") ";

    m_QueryPanel->AddQueryText(query_constraint);
}

void CGridWidget::OnCopy(wxCommandEvent& event)
{
    vector<int> rows, cols;
    if (!x_GetRectSelection(rows, cols)) {
        wxMessageBox(wxT("Can't copy non rectangular selection."), wxT("Error"),
                        wxOK | wxICON_ERROR, this);
        return;
    }

    if (rows.empty() || cols.empty())
        return;

    wxString err_msg;

    try {
        wxStringOutputStream stream;
        wxTextOutputStream os(stream);

        {{ // ostringstream gives '\n' as endl, but we want to pass to clipboard native endl's
            ostringstream oss;
            CwxGridTableExport(*m_GridAdapter, oss, rows, cols, false);
            os << wxString::FromUTF8(oss.str().c_str());
        }}

        if (wxTheClipboard->Open()) {
            wxTheClipboard->SetData( new wxTextDataObject(stream.GetString()));
            wxTheClipboard->Close();
        }
    }
    catch (const CException& e) {
        err_msg = ToWxString(e.GetMsg());
    }

    if (!err_msg.empty()) {
        wxMessageBox(wxT("Copy to Clipboard failed: ") + err_msg, wxT("Error"),
                wxOK | wxICON_ERROR, this);
    }
}

bool CGridWidget::x_GetRectSelection(vector<int>& rows, vector<int>&cols)
{
    int colNum = m_Grid->GetNumberCols();
    map<int, bm::bvector<> > selected;

// Block selection
    wxGridCellCoordsArray selTopLeft = m_Grid->GetSelectionBlockTopLeft();
    wxGridCellCoordsArray selBottomRight = m_Grid->GetSelectionBlockBottomRight();
    size_t blockCount = selTopLeft.GetCount();

    if (blockCount > 0 && blockCount == selBottomRight.GetCount()) {
        for (size_t i = 0; i < blockCount; ++i) {
            for (int row = selTopLeft[i].GetRow(); row <= selBottomRight[i].GetRow(); ++row) {
                for (int col = selTopLeft[i].GetCol(); col <= selBottomRight[i].GetCol(); ++col) {
                    selected[row].set(col);
                }
            }
        }
    }

// Cell Selection
    wxGridCellCoordsArray selCells = m_Grid->GetSelectedCells();
    for (size_t i = 0; i < selCells.GetCount(); ++i) {
        int row = selCells[i].GetRow(), col = selCells[i].GetCol();
        selected[row].set(col);
    }

// Row selection
    wxArrayInt selRows = m_Grid->GetSelectedRows();
    for (size_t i = 0; i < selRows.size(); ++i) {
        int row = selRows[i];
        selected[row].set_range(0, colNum - 1);
    }

    if (selected.empty()) {
        int curRow = m_Grid->GetGridCursorRow();
        if (curRow >= 0) {
            selected[curRow].set(m_Grid->GetGridCursorCol());
        }
    }

    if (selected.empty())
        return true;

    map<int, bm::bvector<> >::const_iterator it = selected.begin();
    bm::bvector<> front = it->second;
    for (++it; it != selected.end(); ++it) {
        if (it->second != front)
            return false;
    }

    for (bm::bvector<>::enumerator it2 = front.first(); it2 != front.end(); ++it2) {
        cols.push_back(*it2);
    }

    for (it = selected.begin(); it != selected.end(); ++it) {
        rows.push_back(it->first);
    }

    return true;
}

void CGridWidget::OnUpdateCopy(wxUpdateUIEvent& event)
{
    event.Enable(true);
}

void CGridWidget::OnRefreshClick(wxCommandEvent& event)
{
    // send a message to the validate view that is it should re-read it's data and refresh itself
    CGridWidgetEvent evt(CGridWidgetEvent::eRefreshData);
    Send(&evt, ePool_Parent);
}

END_NCBI_SCOPE
