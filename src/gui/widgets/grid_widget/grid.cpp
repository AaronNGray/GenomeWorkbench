/*  $Id: grid.cpp 44084 2019-10-23 16:21:17Z katargir $
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

#include <gui/widgets/grid_widget/grid.hpp>
#include <gui/objutils/grid_table_adapter.hpp>
#include <gui/widgets/wx/async_call.hpp>
#include <gui/widgets/wx/wx_utils.hpp>
#include <gui/widgets/grid_widget/str_url_parser.hpp>
#include <gui/widgets/grid_widget/filter_columns_dlg.hpp>

#include <wx/menu.h>
#include <wx/tipwin.h>
#include <wx/dcclient.h>
#include <wx/log.h>

BEGIN_NCBI_SCOPE

static const char * up_xpm[] = {
"16 16 2 1",
"   c None",
"X  c Gray25",
"                ",
"                ",
"                ",
"                ",
"                ",
"                ",
"        X       ",
"       XXX      ",
"      XXXXX     ",
"     XXXXXXX    ",
"                ",
"                ",
"                ",
"                ",
"                ",
"                "};


static const char * down_xpm[] = {
"16 16 2 1",
"   c None",
"X  c Gray25",
"                ",
"                ",
"                ",
"                ",
"                ",
"                ",
"     XXXXXXX    ",
"      XXXXX     ",
"       XXX      ",
"        X       ",
"                ",
"                ",
"                ",
"                ",
"                ",
"                "};

#define ID_FIRST_COLUMN_MENU    10000
#define MAX_COLUMNS_IN_MENU     20
#define ID_FILTER_COLUMNS_MENU  ID_FIRST_COLUMN_MENU + MAX_COLUMNS_IN_MENU

wxDEFINE_EVENT(wxEVT_COMMAND_HYPERLINK_HOVER, wxHyperlinkEvent);

BEGIN_EVENT_TABLE( CGrid, wxGrid )
EVT_GRID_LABEL_LEFT_CLICK(CGrid::OnGridLabelLeftClick)
EVT_GRID_LABEL_RIGHT_CLICK(CGrid::OnGridLabelRightClick)
EVT_TIMER(-1, CGrid::OnTimer)
EVT_MENU_RANGE(ID_FIRST_COLUMN_MENU, ID_FIRST_COLUMN_MENU + MAX_COLUMNS_IN_MENU - 1, CGrid::OnShowHideColumn)
EVT_MENU(ID_FILTER_COLUMNS_MENU, CGrid::OnFilterColumns)
END_EVENT_TABLE()

CGrid::CGrid(wxWindow* parent, wxWindowID id,
             const wxPoint& pos,
             const wxSize& size,
             long style,
             const wxString& name)
:  wxGrid(parent, id, pos, size, style, name),
   m_ImageList( 16, 16 ), m_CurLink(), m_Timer(this)
{
    m_ImageList.Add( wxBitmap( up_xpm ) );
    m_ImageList.Add( wxBitmap( down_xpm ) );

    GetGridWindow()->Bind(wxEVT_MOTION, &CGrid::OnMouseMove, this);
    GetGridWindow()->Bind(wxEVT_LEFT_DOWN, &CGrid::OnLeftDown, this);
}

void CGrid::DrawColLabel( wxDC& dc, int col )
{
    if (col != GetSortingColumn() || m_nativeColumnLabels) {
        wxGrid::DrawColLabel( dc, col );
        return;
    }

    if ( GetColWidth(col) <= 0 || m_colLabelHeight <= 0 )
        return;

    int imgWidth = 0, imgHeight = 0, imageIndex = (IsSortOrderAscending() ? 0 : 1);
    m_ImageList.GetSize(imageIndex, imgWidth, imgHeight);

    int colLeft = GetColLeft(col);
    wxRect rect(colLeft, 0, GetColWidth(col), m_colLabelHeight);

    wxGridCellAttrProvider * const
        attrProvider = m_table ? m_table->GetAttrProvider() : NULL;

    wxGridColumnHeaderRendererDefault defaultRenderer;

    const wxGridColumnHeaderRenderer&
        rend = attrProvider ? attrProvider->GetColumnHeaderRenderer(col)
                            : defaultRenderer;

    // It is reported that we need to erase the background to avoid display
    // artefacts, see #12055.
    wxDCBrushChanger setBrush(dc, GetGridColLabelWindow()->GetBackgroundColour());
    dc.DrawRectangle(rect);

    rend.DrawBorder(*this, dc, rect);

    int hAlign, vAlign;
    GetColLabelAlignment(&hAlign, &vAlign);
    const int orient = GetColLabelTextOrientation();

    wxString labelText = GetColLabelValue(col);
    wxSize labelSize = dc.GetTextExtent(labelText);
    labelSize.x += imgWidth;

    if (labelSize.GetWidth() <= rect.GetWidth()) {
        if (hAlign&wxALIGN_RIGHT) {
            rect.x = rect.GetRight() - labelSize.GetWidth();
        }
        else if (hAlign&wxALIGN_CENTER_HORIZONTAL) {
            rect.x += (rect.GetWidth() - labelSize.GetWidth())/2;
        }
        rect.SetWidth(labelSize.GetWidth());
    }

    m_ImageList.Draw(imageIndex, dc,
                     rect.x, (m_colLabelHeight - 4 - imgHeight)/2 + 2,
                     wxIMAGELIST_DRAW_TRANSPARENT);

    rect.x     += imgWidth;
    rect.width -= imgWidth;

    if (rect.width > 0) {
        rend.DrawLabel(*this, dc, GetColLabelValue(col), rect, wxALIGN_LEFT, vAlign, orient);
    }
}

void CGrid::x_ShowTooltip(bool show)
{
    static wxTipWindow* s_tipWindow = NULL;

    if (s_tipWindow)
    {
        s_tipWindow->SetTipWindowPtr(NULL);
        s_tipWindow->Close();
        s_tipWindow = NULL;
    }

    if (!show)
        return;

    if (IsCellEditControlShown())
        return;

    int row = m_MouseOverCell.GetRow();
    int col = m_MouseOverCell.GetCol();

    if (row < 0 && col < 0)
        return;

    wxRect rect = CellToRect(row, col);
    wxString value = GetCellValue(row, col);
    if (value.IsEmpty())
        return;

    wxGridCellAttr* attr = GetCellAttr(row, col);
    wxGridCellRenderer* renderer = attr->GetRenderer(this, row, col);

    wxWindow* gridWindow = GetGridWindow();

    {{
        wxClientDC dc(gridWindow);
        wxSize size = renderer->GetBestSize(*this, *attr, dc, row, col);
        if (size.GetWidth() <= rect.GetWidth() &&
            size.GetHeight() <= rect.GetHeight()) {
            return;
        }
    }}

    CStrWithUrlParser::StripHtml(value);

    gridWindow->SetCursor( *wxSTANDARD_CURSOR );
    wxPoint posScrolled = CalcScrolledPosition(rect.GetTopLeft());
    wxRect screenRect(gridWindow->ClientToScreen(posScrolled), rect.GetSize());
    s_tipWindow = new wxTipWindow(GetGridWindow(), value, 1000, &s_tipWindow, &screenRect);
}

void CGrid::OnTimer(wxTimerEvent& WXUNUSED(event))
{
    if (m_Timer.IsRunning())
        return;

    x_ShowTooltip(true);
}

void CGrid::OnMouseMove(wxMouseEvent& event)
{
    if (event.Dragging()) {
        event.Skip();
        return;
    }

    const wxPoint pos = CalcUnscrolledPosition(event.GetPosition());
    // coordinates of the cell under mouse
    wxGridCellCoords coords = XYToCell(pos);

    int row = coords.GetRow();
    int col = coords.GetCol();

    if ( row < 0 || col < 0 ) {
        m_MouseOverCell = coords;
        event.Skip();
        return;
    }

    const CCellHyperlinks::CHyperlink* link = 0;
    const CCellHyperlinks* links = m_HyperLinks.GetLink(col, row);
    if (links != 0) {
        link = links->HitTest(pos);
    }

    if (link) {
        x_ShowTooltip(false);
        m_Timer.Stop();
        m_MouseOverCell = wxGridCellCoords();
    }
    else if (coords != m_MouseOverCell) {
        x_ShowTooltip(false);

        m_MouseOverCell = coords;
        m_Timer.Stop();

        if (m_MouseOverCell.GetCol() >= 0 && m_MouseOverCell.GetRow() >= 0) {
            m_Timer.Start(500, wxTIMER_ONE_SHOT);
        }
    }

    if (link != m_CurLink) {
        wxString url;
        if (link)
            url = link->GetURL();

        m_CurLink = link;

        wxHyperlinkEvent hoverEvent(this, GetId(), url);
        hoverEvent.SetEventType(wxEVT_COMMAND_HYPERLINK_HOVER);
        wxPostEvent(this, hoverEvent);

        if (m_CurLink) {
            GetGridWindow()->SetCursor( wxCursor(wxCURSOR_HAND) );
        }
        else {
            GetGridWindow()->SetCursor( *wxSTANDARD_CURSOR );
        }
        return;
    }

    event.Skip();
}

void CGrid::OnLeftDown(wxMouseEvent& event)
{
    const wxPoint pos = CalcUnscrolledPosition(event.GetPosition());
    // coordinates of the cell under mouse
    wxGridCellCoords coords = XYToCell(pos);

    int row = coords.GetRow();
    int col = coords.GetCol();

    if ( row < 0 || col < 0 ) {
        event.Skip();
        return;
    }

    const CCellHyperlinks* links = m_HyperLinks.GetLink(col, row);
    if (links != 0) {
        const CCellHyperlinks::CHyperlink* link = links->HitTest(pos);
        if (link) {
            wxString url = link->GetURL();

            wxHyperlinkEvent linkEvent(this, GetId(), url);
            if (!GetEventHandler()->ProcessEvent(linkEvent))     // was the event skipped ?
            {
                if (!wxLaunchDefaultBrowser(url))
                {
                    wxLogWarning(wxT("Could not launch the default browser with url '%s' !"), url.c_str());
                }
            }

            return;
        }
    }
    event.Skip();
}

void CGrid::OnGridLabelLeftClick(wxGridEvent& evt)
{
    int selRow = evt.GetRow();
    if (selRow > 0 && evt.ControlDown()) {
        wxArrayInt selRows = GetSelectedRows();
        if (find(selRows.begin(), selRows.end(), selRow) != selRows.end()) {
            DeselectRow(selRow);
            return;
        }
    }

    int selCol = evt.GetCol();
    if (selCol < 0) {
        evt.Skip();
        return;
    }

    IGridTableAdapter* gridAdapter = dynamic_cast<IGridTableAdapter*>(GetTable());
    if (!gridAdapter)
        return;

    vector<size_t> vis_columns = gridAdapter->GetVisibleColumns();
    if ((size_t)selCol >= vis_columns.size())
        return;

    IGridTableAdapter::TSortColumn curSort = gridAdapter->GetSortColumn();
    IGridTableAdapter::TSortColumn newSort =
        IGridTableAdapter::TSortColumn(vis_columns[selCol], IGridTableAdapter::kAscending);
    
    if (newSort.first == curSort.first) {
        if (curSort.second == IGridTableAdapter::kAscending)
            newSort.second = IGridTableAdapter::Descending;
        else
            newSort.first = kInvalidColumn;
    }

    vector<size_t>::const_iterator it =
        find (vis_columns.begin(), vis_columns.end(), newSort.first); 

    SetSortingColumn ((it == vis_columns.end()) ? -1 : it - vis_columns.begin(), 
                      (newSort.second == IGridTableAdapter::kAscending));

    /*
    Yes, wxGrid API is counter-intuitive ;)

    If you want all selected cells you need to combine the results of:
    wxGrid::GetSelectedCells
    wxGrid::GetSelectedCols
    wxGrid::GetSelectedRows

    wxGrid::GetSelectionBlockTopLeft
    wxGrid::GetSelectionBlockBottomRight
    */

    //wxGridCellCoordsArray arr = GetSelectedCells();
    //wxGridCellCoordsArray arr_tl = GetSelectionBlockTopLeft();
    //wxGridCellCoordsArray arr_br = GetSelectionBlockBottomRight();
    wxArrayInt sel_rows = GetSelectedRows();   
    ClearSelection();

    size_t i;

    vector<size_t> selected_idx;
    for (i=0; i<sel_rows.size(); ++i) {
        size_t row_idx = gridAdapter->GetOriginalRow(sel_rows[i]);
        selected_idx.push_back(row_idx);
    }

    /*
    wxGridCellCoordsArray selected_arr;
    for (i=0; i<arr.GetCount(); ++i) {
        size_t row_idx = gridAdapter->GetOriginalRow(arr[i].GetRow());
        selected_arr.push_back(wxGridCellCoords((int)row_idx, arr[i].GetCol()));
    }

    wxGridCellCoordsArray selected_tl, selected_br;
    for (i=0; i<arr_tl.GetCount(); ++i) {
        size_t row_idx;
        row_idx = gridAdapter->GetOriginalRow(arr_tl[i].GetRow());
        selected_tl.push_back(wxGridCellCoords((int)row_idx, arr_tl[i].GetCol()));

        row_idx = gridAdapter->GetOriginalRow(arr_br[i].GetRow());
        selected_br.push_back(wxGridCellCoords((int)row_idx, arr_br[i].GetCol()));
    }
    */

    wxBusyCursor wait;

    GUI_AsyncExec([gridAdapter, newSort](ICanceled&) { gridAdapter->SortByColumn(newSort); }, wxT("Sorting..."));

    for (i=0; i<selected_idx.size(); ++i) {
        size_t sorted_idx = gridAdapter->GetCurrentRow(selected_idx[i]);
        if (sorted_idx != (size_t)-1)
            SelectRow(sorted_idx, true);
    }

    /*
    for (i=0; i<selected_arr.size(); ++i) {
        size_t sorted_idx = gridAdapter->GetCurrentRow(selected_arr[i].GetRow());
        if (sorted_idx != (size_t)-1)
            SelectBlock(wxGridCellCoords((int)sorted_idx, selected_arr[i].GetCol()),
                        wxGridCellCoords((int)sorted_idx, selected_arr[i].GetCol()));
    }
    
    for (i=0; i<selected_tl.size(); ++i) {
        size_t sorted_idx_tl = gridAdapter->GetCurrentRow(selected_arr[i].GetRow());
        size_t sorted_idx_br = gridAdapter->GetCurrentRow(selected_arr[i].GetRow());

        if (sorted_idx_tl != (size_t)-1 && sorted_idx_br != (size_t)-1)
            SelectBlock(wxGridCellCoords((int)sorted_idx_tl, selected_tl[i].GetCol()),
                        wxGridCellCoords((int)sorted_idx_br, selected_br[i].GetCol()));
    }
    */

    Refresh();
}

void CGrid::OnGridLabelRightClick(wxGridEvent& evt)
{
    if (evt.GetCol() < 0)
        return;

    IGridTableAdapter* gridAdapter = dynamic_cast<IGridTableAdapter*>(GetTable());
    if (!gridAdapter)
        return;

    vector<size_t> vis_columns = gridAdapter->GetVisibleColumns();
    set<size_t> vis_set;
    vis_set.insert(vis_columns.begin(), vis_columns.end());

    wxMenu menu;
    for (size_t col = 0; col < gridAdapter->GetTable().GetColsCount() && col < MAX_COLUMNS_IN_MENU; col++ ) {
        string label = gridAdapter->GetTable().GetColumnLabel(col);
        menu.AppendCheckItem (ID_FIRST_COLUMN_MENU + (int)col, ToWxString(label));
        menu.Check (ID_FIRST_COLUMN_MENU + (int)col, vis_set.find(col) != vis_set.end());
    }
    if (gridAdapter->GetTable().GetColsCount() > MAX_COLUMNS_IN_MENU) {
        menu.Append(ID_FILTER_COLUMNS_MENU, wxT("More..."));
    }

    PopupMenu(&menu, evt.GetPosition());
}

void CGrid::OnShowHideColumn(wxCommandEvent& event)
{
    IGridTableAdapter* gridAdapter = dynamic_cast<IGridTableAdapter*>(GetTable());
    if (!gridAdapter)
        return;

    size_t col = (size_t)(event.GetId() - ID_FIRST_COLUMN_MENU);
    if (col >= gridAdapter->GetTable().GetColsCount())
        return;

    vector<size_t> vis_columns = gridAdapter->GetVisibleColumns();
    if (vis_columns.size() == 1 && gridAdapter->IsColumnShown(col))
        return;

    gridAdapter->ShowColumn(col, !gridAdapter->IsColumnShown(col));
}

void CGrid::OnFilterColumns(wxCommandEvent& event)
{
    IGridTableAdapter* gridAdapter = dynamic_cast<IGridTableAdapter*>(GetTable());
    _ASSERT(gridAdapter);
    if (!gridAdapter)
        return;

    CFilterColumnsDlg dlgFilterColumns(this);
    dlgFilterColumns.SetGridAdapter(gridAdapter);
    dlgFilterColumns.ShowModal();
}

END_NCBI_SCOPE
