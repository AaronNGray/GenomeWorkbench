/*  $Id: wxgrid_table_adapter.cpp 39666 2017-10-25 16:01:13Z katargir $
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
 */

#include <ncbi_pch.hpp>

#include <gui/widgets/grid_widget/wxgrid_table_adapter.hpp>
#include <gui/widgets/grid_widget/wxgrid_commands.hpp>
#include <gui/widgets/grid_widget/str_url_renderer.hpp>

#include "assign_column_value_dlg.hpp"
#include "copy_column_dlg.hpp"

#include <gui/widgets/wx/wx_utils.hpp>
#include <gui/widgets/wx/ui_command.hpp>
#include <gui/objutils/table_data_sorter.hpp>

#include <gui/widgets/data/cmd_table_change_cell.hpp>

BEGIN_NCBI_SCOPE

static const char* kVisibleColumns = "Visible Columns";
static const char* kWidth = ".Width";

CwxGridTableAdapter::CwxGridTableAdapter(ITableData& tableData, ICommandProccessor* cmdProccessor)
    : IGridTableAdapter(tableData)
    , m_SortColumn(kInvalidColumn, kAscending)  
    , m_DefaultCellAttr()
    , m_IntCellAttr()
    , m_FloatCellAttr() 
    , m_HtmlCellAttrL()
    , m_HtmlCellAttrR()
    , m_TextCellAttrEdit()
    , m_IntCellAttrEdit()
    , m_FloatCellAttrEdit()
    , m_CmdProccessor(cmdProccessor)
    , m_EventHandler(*this)
{
    SetAttrProvider(new wxGridCellAttrProvider);

    for (size_t i = 0; i < m_TableData->GetColsCount(); ++i) {
        m_VisibleColumns.push_back(i);
    }

    x_UpdateRenderers();

    x_ResetRows();

    static bool registered = false;
    if (!registered) {
        registered = true;

        CUICommandRegistry& cmd_reg = CUICommandRegistry::GetInstance();
        cmd_reg.RegisterCommand(eCmdAssignColumnValue,
            "Assign Value",
            "Assign Value to a Column",
            NcbiEmptyString,
            NcbiEmptyString,
            "Command shows dialog for assigning value to a whole column of a table");
        cmd_reg.RegisterCommand(eCmdCopyColumn,
            "Copy",
            "Copy values of Column to another Column",
            NcbiEmptyString,
            NcbiEmptyString,
            "Copy values of Column to another Column");
    }
}

void CwxGridTableAdapter::x_UpdateRenderers()
{
    for (size_t i = 0; i < m_VisibleColumns.size(); ++i) {
        ITableData::ColumnType colType = m_TableData->GetColumnType(m_VisibleColumns[i]); 

        if (m_TableData->IsHtmlColumn(m_VisibleColumns[i])) {
            if (colType == ITableData::kInt || colType == ITableData::kReal)
                SetColAttr(x_GetHtmlCellAttrR(), (int)i);
            else
                SetColAttr(x_GetHtmlCellAttrL(), (int)i);
            continue;
        }

        bool edit = m_TableData->AllowEdit(m_VisibleColumns[i]);

        if (colType == ITableData::kInt)
            SetColAttr(x_GetIntRendererAttr(edit), (int)i);
        else if (colType == ITableData::kReal)
            SetColAttr(x_GetFloatRendererAttr(edit), (int)i);
        else if (colType == ITableData::kString)
            SetColAttr(x_GetDefaultRendererAttr(edit), (int)i);
        else if (colType == ITableData::kCommonString) {
            if (edit) {
                vector<string> strings = m_TableData->GetColumnCommonStrings(i);
                wxArrayString strings2;
                ITERATE(vector<string>, it, strings) {
                    strings2.push_back(wxString::FromUTF8(it->c_str()));
                }

                wxGridCellAttr* attr = new wxGridCellAttr();
                attr->SetEditor(new wxGridCellChoiceEditor(strings2));
                attr->SetReadOnly(false);
                SetColAttr(attr, (int)i);
            }
            else
                SetColAttr(x_GetDefaultRendererAttr(false), (int)i);
        }
        else
            SetColAttr(x_GetDefaultRendererAttr(edit), (int)i);
    }
}

void CwxGridTableAdapter::SetView( wxGrid *grid )
{
    wxGrid* oldGrid = GetView();
    if (oldGrid != 0)
        oldGrid->PopEventHandler();

    wxGridTableBase::SetView( grid );

    if (grid == 0)
        return;

    grid->PushEventHandler(&m_EventHandler);

    if (m_RegPath.empty())
        return;

    CGuiRegistry& gui_reg = CGuiRegistry::GetInstance();
    CRegistryReadView view = gui_reg.GetReadView(m_RegPath + "."
                                                       + m_TableData->GetTableTypeId());

    for (size_t i = 0; i < m_VisibleColumns.size(); ++i) {
        string label = m_TableData->GetColumnLabel(m_VisibleColumns[i]);
        int width = view.GetInt(label + kWidth, 0);
        if (width > 0)
            grid->SetColSize((int)i, width);
    }
}

CwxGridTableAdapter::~CwxGridTableAdapter()
{
    if (m_IntCellAttr) {
        m_IntCellAttr->DecRef();
        m_IntCellAttr = 0;
    }
    if (m_FloatCellAttr) {
        m_FloatCellAttr->DecRef();
        m_FloatCellAttr = 0;
    }
    if (m_DefaultCellAttr) {
        m_DefaultCellAttr->DecRef();
        m_DefaultCellAttr = 0;
    }
    if (m_HtmlCellAttrL) {
        m_HtmlCellAttrL->DecRef();
        m_HtmlCellAttrL = 0;
    }
    if (m_HtmlCellAttrR) {
        m_HtmlCellAttrR->DecRef();
        m_HtmlCellAttrR = 0;
    }
    if (m_TextCellAttrEdit) {
        m_TextCellAttrEdit->DecRef();
        m_TextCellAttrEdit = 0;
    }
    if (m_IntCellAttrEdit) {
        m_IntCellAttrEdit->DecRef();
        m_IntCellAttrEdit = 0;
    }
    if (m_FloatCellAttrEdit) {
        m_FloatCellAttrEdit->DecRef();
        m_FloatCellAttrEdit = 0;
    }
}

int CwxGridTableAdapter::GetNumberRows()
{
    if (m_HideUnselected)
        return m_Selected.size();
    else
        return m_RowToTable.size();
}

int CwxGridTableAdapter::GetNumberCols()
{
    return m_VisibleColumns.size();
}

wxString CwxGridTableAdapter::GetColLabelValue(int col)
{
    if (!x_ColToTableData(col))
        return wxEmptyString;

    return ToWxString(m_TableData->GetColumnLabel(col));
}

wxString CwxGridTableAdapter::GetRowLabelValue(int row)
{
    wxString s = wxGridTableBase::GetRowLabelValue(row);

    if (x_RowToTableData(row)) {
        string label = m_TableData->GetRowLabel(row);
        if (!label.empty())
            s += ToWxString(" " + label);
    }

    return s;
}

wxString CwxGridTableAdapter::GetValue(int row, int col)
{
    if (!x_CoordsToTableData(row, col))
        return wxEmptyString;

    string s;
    m_TableData->GetStringValue(row, col, s);
    return wxString::FromUTF8(s.c_str());
}

wxString CwxGridTableAdapter::GetHtmlValue(int row, int col)
{
    if (!x_CoordsToTableData(row, col))
        return wxEmptyString;

    string s;
    if (m_TableData->IsHtmlColumn(col))
        return wxString::FromUTF8(m_TableData->GetHtmlValue(row, col).c_str());
    else {
        m_TableData->GetStringValue(row, col, s);
        return wxString::FromUTF8(s.c_str());
    }
}

bool CwxGridTableAdapter::CanGetValueAs( int row, int col, const wxString& typeName )
{
    if (!x_CoordsToTableData(row, col))
        return false;

    ITableData::ColumnType colType = m_TableData->GetColumnType(col);

    if (wxGRID_VALUE_STRING == typeName)
        return true;

    if (wxGRID_VALUE_NUMBER == typeName)
        return (colType == ITableData::kInt);

    if (wxGRID_VALUE_FLOAT == typeName)
        return (colType == ITableData::kReal);

    return false;
}

bool CwxGridTableAdapter::CanSetValueAs( int row, int col, const wxString& typeName )
{
    if (!x_CoordsToTableData(row, col))
        return false;

    ITableData::ColumnType colType = m_TableData->GetColumnType(col);

    if (wxGRID_VALUE_STRING == typeName &&
        colType == ITableData::kString)
        return true;

    if (wxGRID_VALUE_NUMBER == typeName &&
        colType == ITableData::kInt)
        return true;

    if (wxGRID_VALUE_FLOAT == typeName &&
        colType == ITableData::kReal)
        return true;

    return false;
}

long CwxGridTableAdapter::GetValueAsLong( int row, int col )
{
    if (!x_CoordsToTableData(row, col))
        return 0;
    return m_TableData->GetIntValue(row, col);
}

double CwxGridTableAdapter::GetValueAsDouble( int row, int col )
{
    if (!x_CoordsToTableData(row, col))
        return 0;
    return m_TableData->GetRealValue(row, col);
}

bool CwxGridTableAdapter::x_CoordsToTableData(int& row, int& col)
{
    if (row < 0 || col < 0) {
        row = col = -1;
        return false;
    }

    if (m_HideUnselected) {
        if (col < (int)m_VisibleColumns.size() && row < (int)m_Selected.size()) {
            col = m_VisibleColumns[col];
            row = m_SelectedRowToTable[row];
            return true;
        }
    }
    else {
        if (col < (int)m_VisibleColumns.size() && row < (int)m_RowToTable.size()) {
            col = m_VisibleColumns[col];
            row = m_RowToTable[row];
            return true;
        }
    }
    row = col = -1;
    return false;
}

bool CwxGridTableAdapter::x_ColToTableData(int& col)
{
    if (col >= 0 && col < (int)m_VisibleColumns.size()) {
        col = m_VisibleColumns[col];
        return true;
    }
    return false;
}

bool CwxGridTableAdapter::x_RowToTableData(int& row)
{
    if (m_HideUnselected) {
        if (row < (int)m_Selected.size()) {
            row = m_SelectedRowToTable[row];
            return true;
        }
    }
    else {
        if (row < (int)m_RowToTable.size()) {
            row = m_RowToTable[row];
            return true;
        }
    }
    row = -1;
    return false;
}

void CwxGridTableAdapter::SetValue(int row, int col, const wxString& value)
{
    if (!x_CoordsToTableData(row, col))
        return;

    if (m_CmdProccessor && m_TableData->AllowEdit(col)) {
        CIRef<IEditCommand> cmd(new CCmdTableChangeCell(*m_TableData, row, col, value));
        m_CmdProccessor->Execute(cmd);
    }
}

void CwxGridTableAdapter::SetValueAsLong( int row, int col, long value )
{
    if (!x_CoordsToTableData(row, col))
        return;

    if (m_CmdProccessor && m_TableData->AllowEdit(col)) {
        CIRef<IEditCommand> cmd(new CCmdTableChangeCell(*m_TableData, row, col, value));
        m_CmdProccessor->Execute(cmd);
    }
}

void CwxGridTableAdapter::SetValueAsDouble( int row, int col, double value )
{
    if (!x_CoordsToTableData(row, col))
        return;

    if (m_CmdProccessor && m_TableData->AllowEdit(col)) {
        CIRef<IEditCommand> cmd(new CCmdTableChangeCell(*m_TableData, row, col, value));
        m_CmdProccessor->Execute(cmd);
    }
}

/*
wxGridCellAttr* CwxGridTableAdapter::GetAttr( int row, int col, wxGridCellAttr::wxAttrKind  kind )
{
    wxGridCellAttr* attr = wxGridTableBase::GetAttr( row, col, kind );

    attr->SetReadOnly( !GetTable().AllowEdit( col ) );

    return attr;
}
*/

wxGridCellAttr* CwxGridTableAdapter::x_GetDefaultRendererAttr(bool allowEdit)
{
    if (allowEdit && m_CmdProccessor) {
        if (m_TextCellAttrEdit == 0) {
            m_TextCellAttrEdit = new wxGridCellAttr();
            m_TextCellAttrEdit->SetReadOnly(false);
        }
        m_TextCellAttrEdit->IncRef();

        return m_TextCellAttrEdit;
    }
    else {
        if (m_DefaultCellAttr == 0) {
            m_DefaultCellAttr = new wxGridCellAttr();
            m_DefaultCellAttr->SetReadOnly();
        }
        m_DefaultCellAttr->IncRef();

        return m_DefaultCellAttr;
    }
}

wxGridCellAttr* CwxGridTableAdapter::x_GetIntRendererAttr(bool allowEdit)
{
    if (allowEdit && m_CmdProccessor) {
        if (m_IntCellAttrEdit == 0) {
            m_IntCellAttrEdit = new wxGridCellAttr();
            m_IntCellAttrEdit->SetRenderer(new wxGridCellNumberRenderer());
            m_IntCellAttrEdit->SetEditor(new wxGridCellNumberEditor());
            m_IntCellAttrEdit->SetReadOnly(false);
        }
        m_IntCellAttrEdit->IncRef();

        return m_IntCellAttrEdit;
    }
    else {
        if (m_IntCellAttr == 0) {
            m_IntCellAttr = new wxGridCellAttr();
            m_IntCellAttr->SetRenderer(new wxGridCellNumberRenderer());
            m_IntCellAttr->SetReadOnly();
        }
        m_IntCellAttr->IncRef();

        return m_IntCellAttr;
    }
}

wxGridCellAttr* CwxGridTableAdapter::x_GetFloatRendererAttr(bool allowEdit)
{
    if (allowEdit && m_CmdProccessor) {
        if (m_FloatCellAttrEdit == 0) {
            m_FloatCellAttrEdit = new wxGridCellAttr();
            m_FloatCellAttrEdit->SetRenderer(new wxGridCellFloatRenderer());
            m_FloatCellAttrEdit->SetEditor(new wxGridCellFloatEditor());
            m_FloatCellAttrEdit->SetReadOnly(false);
        }
        m_FloatCellAttrEdit->IncRef();

        return m_FloatCellAttrEdit;
    }
    else {
        if (m_FloatCellAttr == 0) {
            m_FloatCellAttr = new wxGridCellAttr();
            m_FloatCellAttr->SetRenderer(new wxGridCellFloatRenderer());
            m_FloatCellAttr->SetReadOnly();
        }
        m_FloatCellAttr->IncRef();

        return m_FloatCellAttr;
    }
}

wxGridCellAttr* CwxGridTableAdapter::x_GetHtmlCellAttrL()
{
    if (m_HtmlCellAttrL == 0) {
        m_HtmlCellAttrL = new wxGridCellAttr();
        m_HtmlCellAttrL->SetOverflow(false);
        m_HtmlCellAttrL->SetRenderer(new CStrWithURLRenderer());
        m_HtmlCellAttrL->SetReadOnly();
    }
    m_HtmlCellAttrL->IncRef();

    return m_HtmlCellAttrL;
}

wxGridCellAttr* CwxGridTableAdapter::x_GetHtmlCellAttrR()
{
    if (m_HtmlCellAttrR == 0) {
        m_HtmlCellAttrR = new wxGridCellAttr();
        m_HtmlCellAttrR->SetOverflow(false);
        m_HtmlCellAttrR->SetRenderer(new CStrWithURLRenderer());
        m_HtmlCellAttrR->SetReadOnly();
        m_HtmlCellAttrR->SetAlignment(wxALIGN_RIGHT, wxALIGN_TOP);
    }
    m_HtmlCellAttrR->IncRef();

    return m_HtmlCellAttrR;
}

void CwxGridTableAdapter::ShowColumn(size_t col, bool show)
{
    if (col >= m_TableData->GetColsCount())
        return;

    if (show) {
        vector<size_t>::iterator it =
            find_if(m_VisibleColumns.begin(), m_VisibleColumns.end(), bind2nd(greater_equal<size_t>(), col));

        if (it != m_VisibleColumns.end() && *it == col)
            return;

        string label = m_TableData->GetColumnLabel(col);

        size_t index = it - m_VisibleColumns.begin();
        m_VisibleColumns.insert(it, col);

        if (GetView()) {
            wxGridTableMessage msg( this,
                                    wxGRIDTABLE_NOTIFY_COLS_INSERTED,
                                    index,
                                    1);

            GetView()->ProcessTableMessage( msg );

            int colWidth = x_LoadColumnWidth(label);
            if (colWidth > 0)
                GetView()->SetColSize((int)index, colWidth);

            x_UpdateRenderers();
        }
    }
    else {
        vector<size_t>::iterator it = find(m_VisibleColumns.begin(), m_VisibleColumns.end(), col);
        if (it == m_VisibleColumns.end())
            return;

        size_t index = it - m_VisibleColumns.begin();
        m_VisibleColumns.erase(it);

        if (GetView()) {
            string label = m_TableData->GetColumnLabel(col);
            x_SaveColumnWidth(label, GetView()->GetColSize((int)index));

            wxGridTableMessage msg( this,
                                    wxGRIDTABLE_NOTIFY_COLS_DELETED,
                                    index,
                                    1);

            GetView()->ProcessTableMessage( msg );

            x_UpdateRenderers();
        }
    }
}

bool CwxGridTableAdapter::IsColumnShown(size_t col) const
{
    return (find(m_VisibleColumns.begin(), m_VisibleColumns.end(), col) != m_VisibleColumns.end());
}

size_t CwxGridTableAdapter::GetOriginalRow(size_t queryRow) const
{
    if (m_HideUnselected) {
        return queryRow < m_SelectedRowToTable.size() ?
            m_SelectedRowToTable[queryRow] : (size_t)-1;
    }
    
    return (queryRow < m_RowToTable.size()) ? m_RowToTable[queryRow] : (size_t)-1;
}

size_t CwxGridTableAdapter::GetCurrentRow(size_t queryRow) const
{    
    if (m_HideUnselected) {
        return queryRow < m_SelectedTableToRow.size() ?
            m_SelectedTableToRow[queryRow] : (size_t)-1;
    }

    return (queryRow < m_TableToRow.size()) ? m_TableToRow[queryRow] : (size_t)-1;
}

void CwxGridTableAdapter::x_ResetRows()
{
    size_t rows = m_TableData->GetRowsCount();
    m_RowToTable.resize(rows);
    m_TableToRow.resize(rows);
    for (size_t i = 0; i < rows; ++i) {
        m_RowToTable[i] = i;
        m_TableToRow[i] = i;
    }
}

bool CwxGridTableAdapter::x_ValidateColumns(const vector<size_t>& colums) const
{
    if (colums.empty())
        return false;

    set<size_t> col_set;

    size_t colCount = m_TableData->GetColsCount();
    if (colums.size() > colCount)
        return false;

    for (size_t i = 0; i < colums.size(); ++i) {
        size_t col = colums[i];
        if (col >= colCount)
            return false;
        if (!col_set.insert(col).second)
            return false;
    }

    return true;
}

void CwxGridTableAdapter::SortByColumn(TSortColumn sortColumn)
{
    if (sortColumn.first == kInvalidColumn) {
        x_ResetRows();
        m_SortColumn = sortColumn;
    }
    else if (sortColumn.first < m_TableData->GetColsCount()) {
        if (sortColumn.first == m_SortColumn.first) {
            if (sortColumn.second == m_SortColumn.second)
                return;

            reverse(m_RowToTable.begin(), m_RowToTable.end());
            m_SortColumn = sortColumn;
        }
        else {
            if (!m_RowToTable.empty()) {
                CTableDataSorter::Sort(*m_TableData, sortColumn.first, 
                                        &*m_RowToTable.begin(),
                                        &*m_RowToTable.begin() + (m_RowToTable.end() - m_RowToTable.begin()),
                                        (sortColumn.second == Descending));
            }
            m_SortColumn = sortColumn;
        }

        m_TableToRow.resize(m_RowToTable.size());
        for (size_t i = 0; i < m_RowToTable.size(); ++i) {      
            m_TableToRow[m_RowToTable[i]] = i;
        }
    }
    
    x_UpdateSelectedIndices();
}

void CwxGridTableAdapter::x_UpdateSelectedIndices()
{
    //
    // Put the selected entries in their own RowToTable/TableToRow
    // index arrays so that the user can switch between viewing
    // all rows and viewing only selected rows quickly.
    int sel_idx = 0;
    size_t i;
    m_SelectedRowToTable.resize(m_Selected.size());
    m_SelectedTableToRow.resize(m_RowToTable.size());

    for (i=0; i<m_RowToTable.size(); ++i) {
        size_t idx = m_RowToTable[i];

        // We put the set of selected table indices in sorted order
        // for quicker searching:
        bool found = std::binary_search(m_Selected.begin(), m_Selected.end(), idx);
        
        // If this table row is currently selected:
        if (found) {
            m_SelectedRowToTable[sel_idx++] = idx;
        }
    }

    // Unselected rows in the data table do not map to any row in 
    // the grid table, so start by intializing all to -1:
    for (i = 0; i < m_RowToTable.size(); ++i) { 
        m_SelectedTableToRow[i] = (size_t)-1;
    }

    // The selected rows in the data table do map to rows in 
    // the grid table, so updated those here:
    for (i = 0; i < m_SelectedRowToTable.size(); ++i) {        
        m_SelectedTableToRow[m_SelectedRowToTable[i]] = i;
    }
}

void CwxGridTableAdapter::SetSelection(const vector<size_t>& query_sel)
{
    IGridTableAdapter::SetSelection(query_sel);

    x_UpdateSelectedIndices();
}

void CwxGridTableAdapter::IterateSelection(int dir)
{
    if (m_Selected.size() == 0) {
        m_CurrentSelection = (size_t)-1;
    }
    else if (m_Selected.size() == 1) {
        m_CurrentSelection = m_Selected[0];
    }
    else {
        // select first element (first in display order, not data (table) order).
        size_t i;
        if (dir == 0) {        
            size_t min_row_idx = (size_t)-1;
            size_t selected_idx = (size_t)-1;           

            if (!m_HideUnselected) {
                for (i=0; i<m_Selected.size(); ++i) {
                    if (m_TableToRow[m_Selected[i]] < min_row_idx) {
                        min_row_idx = m_TableToRow[m_Selected[i]];
                        selected_idx = m_Selected[i];
                    }
                }
                m_CurrentSelection = selected_idx;   
            }
            else {               
                m_CurrentSelection = m_SelectedRowToTable[0];
            }
        }
        else if (dir > 0) {    
            if (!m_HideUnselected) {
                size_t min_row_idx = m_TableToRow[m_CurrentSelection];
                size_t next_row_idx = (size_t)-1;
                size_t selected_idx = (size_t)-1;
    
                for (i=0; i<m_Selected.size(); ++i) {
                    if (m_TableToRow[m_Selected[i]] > min_row_idx &&
                        m_TableToRow[m_Selected[i]] < next_row_idx) {
                        next_row_idx = m_TableToRow[m_Selected[i]];
                        selected_idx = m_Selected[i];
                    }
                }
                if (selected_idx != (size_t)-1)
                    m_CurrentSelection = selected_idx; 
            }
            else {
                size_t cur_row_idx = m_SelectedTableToRow[m_CurrentSelection];
                if (cur_row_idx < m_Selected.size()-1)
                    m_CurrentSelection = m_SelectedRowToTable[cur_row_idx+1];
            }
        }
        else if (dir < 0) {  
            if (!m_HideUnselected) {
                size_t min_row_idx = m_TableToRow[m_CurrentSelection];
                size_t prev_row_idx = 0;
                size_t selected_idx = (size_t)-1;
    
                for (i=0; i<m_Selected.size(); ++i) {
                    if (m_TableToRow[m_Selected[i]] < min_row_idx &&
                        m_TableToRow[m_Selected[i]] >= prev_row_idx) {
                        prev_row_idx = m_TableToRow[m_Selected[i]];
                        selected_idx = m_Selected[i];
                    }
                }
                if (selected_idx != (size_t)-1)
                    m_CurrentSelection = selected_idx; 
            }
            else {
                size_t cur_row_idx = m_SelectedTableToRow[m_CurrentSelection];
                if (cur_row_idx > 0)
                    m_CurrentSelection = m_SelectedRowToTable[cur_row_idx-1];
            }
        }
    }
}

string CwxGridTableAdapter::GetRegPath() const
{
   if (m_RegPath.empty())
        return NcbiEmptyString;

   return m_RegPath + "." + m_TableData->GetTableTypeId();
}

void CwxGridTableAdapter::x_SaveColumnWidth(const string& label, int width)
{
    if (m_RegPath.empty())
        return;

    CGuiRegistry& gui_reg = CGuiRegistry::GetInstance();
    CRegistryWriteView view = gui_reg.GetWriteView(m_RegPath + "."
                                                                 + m_TableData->GetTableTypeId());
    view.Set(label + kWidth , width);
}

int CwxGridTableAdapter::x_LoadColumnWidth(const string& label)
{
    if (m_RegPath.empty())
        return 0;

    CGuiRegistry& gui_reg = CGuiRegistry::GetInstance();
    CRegistryReadView view = gui_reg.GetReadView(m_RegPath + "."
                                                       + m_TableData->GetTableTypeId());
    return view.GetInt(label + kWidth, 0);
}

void CwxGridTableAdapter::LoadSettings()
{
    vector<size_t> vis_columns;

    if (!m_RegPath.empty()) {
        CGuiRegistry& gui_reg = CGuiRegistry::GetInstance();
        CRegistryReadView view = gui_reg.GetReadView(m_RegPath + "."
                                                           + m_TableData->GetTableTypeId());

        vector<int> vis_columns_i;
        view.GetIntVec(kVisibleColumns, vis_columns_i);

        if (!vis_columns_i.empty()) {
            for (size_t i = 0; i < vis_columns_i.size(); ++i)
                vis_columns.push_back(vis_columns_i[i]);
        }

        if (x_ValidateColumns(vis_columns)) {
            m_VisibleColumns = vis_columns;
            x_UpdateRenderers();
        }
    }
}

void CwxGridTableAdapter::SaveSettings() const
{
    if (m_RegPath.empty())
        return;

    CGuiRegistry& gui_reg = CGuiRegistry::GetInstance();
    CRegistryWriteView view = gui_reg.GetWriteView(m_RegPath + "."
                                                                 + m_TableData->GetTableTypeId());

    vector<int> vis_columns_i;
    for (size_t i = 0; i < m_VisibleColumns.size(); ++i)
        vis_columns_i.push_back((int)m_VisibleColumns[i]);
    view.Set(kVisibleColumns, vis_columns_i);

    if (GetView()) {
        for (size_t i = 0; i < m_VisibleColumns.size(); ++i) {
            string label = m_TableData->GetColumnLabel(m_VisibleColumns[i]);
            int width = GetView()->GetColSize((int)i);
            view.Set(label + kWidth , width);
        }
    }
}

static
WX_DEFINE_MENU(kTableEditMenu)
    WX_MENU_SEPARATOR_L("Edit Table")
    WX_SUBMENU("Column")
        WX_MENU_ITEM(eCmdAssignColumnValue)
        WX_MENU_ITEM(eCmdCopyColumn)
    WX_END_SUBMENU()
WX_END_MENU()

wxMenu* CwxGridTableAdapter::CreateMenu()
{
    if (m_CmdProccessor) {
        return CUICommandRegistry::GetInstance().CreateMenu(kTableEditMenu);
    }
    return 0;
}

BEGIN_EVENT_TABLE( CwxGridTableAdapter::CEventHandler, wxEvtHandler )
    EVT_MENU(eCmdAssignColumnValue, CwxGridTableAdapter::CEventHandler::OnAssignColumnValue)
    EVT_UPDATE_UI(eCmdAssignColumnValue, CwxGridTableAdapter::CEventHandler::OnUpdateAssignColumnValue)
    EVT_MENU(eCmdCopyColumn, CwxGridTableAdapter::CEventHandler::OnCopyColumn)
    EVT_UPDATE_UI(eCmdCopyColumn, CwxGridTableAdapter::CEventHandler::OnUpdateCopyColumn)
END_EVENT_TABLE()

void CwxGridTableAdapter::CEventHandler::OnAssignColumnValue(wxCommandEvent&)
{
    CAssignColumnValueDlg dlg(m_Adapter.GetView(), *m_Adapter.m_TableData);
    if(dlg.ShowModal() == wxID_OK && m_Adapter.m_CmdProccessor) {
        CIRef<IEditCommand> command(dlg.GetEditCommand());
        if (command) {
            m_Adapter.m_CmdProccessor->Execute(command);
        }
    }
}

void CwxGridTableAdapter::CEventHandler::OnUpdateAssignColumnValue(wxUpdateUIEvent& event)
{
    event.Enable(CAssignColumnValueDlg::CanAssignColumn(*m_Adapter.m_TableData));
}

void CwxGridTableAdapter::CEventHandler::OnCopyColumn(wxCommandEvent& WXUNUSED(event))
{
    CCopyColumnDlg dlg(m_Adapter.GetView(), *m_Adapter.m_TableData);
    if(dlg.ShowModal() == wxID_OK && m_Adapter.m_CmdProccessor) {
        CIRef<IEditCommand> command(dlg.GetEditCommand());
        if (command) {
            m_Adapter.m_CmdProccessor->Execute(command);
        }
    }
}

void CwxGridTableAdapter::CEventHandler::OnUpdateCopyColumn(wxUpdateUIEvent& event)
{
    event.Enable(CCopyColumnDlg::CanCopyColumn(*m_Adapter.m_TableData));
}

END_NCBI_SCOPE
