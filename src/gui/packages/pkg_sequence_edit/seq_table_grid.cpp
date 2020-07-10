/*  $Id: seq_table_grid.cpp 43833 2019-09-09 17:38:14Z filippov $
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
 *  and reliability of the software and data,  the NLM and the U.S.
 *  Government do not and cannot warrant the performance or results that
 *  may be obtained by using this software or data. The NLM and the U.S.
 *  Government disclaim all warranties,  express or implied,  including
 *  warranties of performance,  merchantability or fitness for any particular
 *  purpose.
 *
 *  Please cite the author in any work or product based on this material.
 *
 * ===========================================================================
 *
 * Authors:  Colleen Bollin
 */


#include <ncbi_pch.hpp>
#include <wx/sizer.h>
#include <wx/msgdlg.h>
#include <wx/dc.h>
#include <wx/settings.h>
#include <wx/dcclient.h>

#include <wx/textdlg.h>
#include <wx/tokenzr.h>
#include <wx/clipbrd.h>

////@begin includes
////@end includes

#include <gui/objutils/interface_registry.hpp>
#include <gui/packages/pkg_sequence_edit/seq_table_grid.hpp>
#include <gui/packages/pkg_sequence_edit/srceditdialog.hpp>
#include <gui/packages/pkg_sequence_edit/molinfoedit_util.hpp>
#include <gui/packages/pkg_sequence_edit/subprep_util.hpp>
#include <gui/packages/pkg_sequence_edit/rename_column_dlg.hpp>
#include <gui/widgets/edit/field_type_constants.hpp>
#include <gui/widgets/wx/ui_command.hpp>
#include <gui/widgets/wx/message_box.hpp>
#include <gui/widgets/edit/simple_text_edit_dlg.hpp>


BEGIN_NCBI_SCOPE
USING_SCOPE(objects);


CSeqGridCellAttrProvider::CSeqGridCellAttrProvider()
{
}

CSeqGridCellAttrProvider::~CSeqGridCellAttrProvider()
{
    for (auto a : m_cache)
    {
        a.second->DecRef();
    }
    m_cache.clear();
}

wxGridCellAttr *CSeqGridCellAttrProvider::GetAttr(int row, int col, wxGridCellAttr::wxAttrKind  kind /* = wxGridCellAttr::Any */) const
{
    wxGridCellAttr *attr = NULL;

    switch (kind)
    {
    case (wxGridCellAttr::Any):
    case (wxGridCellAttr::Cell):
    {
        auto it = m_cache.find(make_pair(row, col));
        if (it != m_cache.end())
        {
            attr = it->second;
            attr->IncRef();
        }
    }
    break;
    
    case (wxGridCellAttr::Col):
        attr = wxGridCellAttrProvider::GetAttr(row, col, kind);
        break;
        
    case (wxGridCellAttr::Row):
        attr = wxGridCellAttrProvider::GetAttr(row, col, kind);
        break;
        
    default:
        break;
    }

    return attr;
}

void CSeqGridCellAttrProvider::SetAttr(wxGridCellAttr *attr, int row, int col)
{
    auto it = m_cache.find(make_pair(row, col));
    if (it == m_cache.end())
    {
        if (attr)
        {
            attr->IncRef();
            m_cache[make_pair(row, col)] = attr;
        }
    }
    else
    {
        it->second->DecRef();
        if (attr)
        {
            attr->IncRef();
            it->second = attr;
        }
        else
        {
            m_cache.erase(it);
        }
    }
}

CCollapsibleGrid::CCollapsibleGrid(wxWindow *parent,
                                   wxWindowID id,
                                   const wxPoint& pos,
                                   const wxSize& size,
                                   long style,
                                   const wxString& name)
    {
        wxGrid::Init();
        wxGrid::Create(parent, id, pos, size, style, name);
    }

void CCollapsibleGrid::x_InitRowHeights()
{
    if ( wxGrid::m_rowHeights.IsEmpty() )
    {
        // need to really create the array
        wxGrid::InitRowHeights();
    }
    m_diffs.clear();
    m_diffs.resize( m_numRows, 0);
}

int CCollapsibleGrid::UpdateRowOrColSize(int& sizeCurrent, int sizeNew)
{
    // On input here sizeCurrent can be negative if it's currently hidden (the
    // real size is its absolute value then). And sizeNew can be 0 to indicate
    // that the row/column should be hidden or -1 to indicate that it should be
    // shown again.

    if ( sizeNew < 0 )
    {
        // We're showing back a previously hidden row/column.
        wxASSERT_MSG( sizeNew == -1, wxS("New size must be positive or -1.") );

        // If it's already visible, simply do nothing.
        if ( sizeCurrent >= 0 )
            return 0;

        // Otherwise show it by restoring its old size.
        sizeCurrent = -sizeCurrent;

        // This is positive which is correct.
        return sizeCurrent;
    }
    else if ( sizeNew == 0 )
    {
        // We're hiding a row/column.

        // If it's already hidden, simply do nothing.
        if ( sizeCurrent <= 0 )
            return 0;

        // Otherwise hide it and also remember the shown size to be able to
        // restore it later.
        sizeCurrent = -sizeCurrent;

        // This is negative which is correct.
        return sizeCurrent;
    }
    else // We're just changing the row/column size.
    {
        // Here it could have been hidden or not previously.
        const int sizeOld = sizeCurrent < 0 ? 0 : sizeCurrent;

        sizeCurrent = sizeNew;

        return sizeCurrent - sizeOld;
    }
}

void CCollapsibleGrid::FastSetRowSize(int row, int height)
{
    const int diff = UpdateRowOrColSize(wxGrid::m_rowHeights[row], height);
    m_diffs[row] += diff;
}

void CCollapsibleGrid::UpdateRowBottoms()
{
    int diff = 0;
    for ( int i = 0; i < m_numRows; i++ )
    {
        diff += m_diffs[i];
        wxGrid::m_rowBottoms[i] += diff;
    }
    
    InvalidateBestSize();
}


// CGridCellWrapStringRenderer

void CGridCellWrapStringRenderer::SetTextColoursAndFont(const wxGrid& grid,
                                                        const wxGridCellAttr& attr,
                                                        wxDC& dc,
                                                        bool isSelected)
{
    dc.SetBackgroundMode( wxBRUSHSTYLE_TRANSPARENT );

    // TODO some special colours for attr.IsReadOnly() case?

    // different coloured text when the grid is disabled
    if ( grid.IsThisEnabled() )
    {
        if ( isSelected )
        {
            wxColour clr;
            if ( grid.HasFocus() )
                clr = grid.GetSelectionBackground();
            else
                clr = wxSystemSettings::GetColour(wxSYS_COLOUR_BTNSHADOW);
            dc.SetTextBackground( clr );
            dc.SetTextForeground( grid.GetSelectionForeground() );
        }
        else
        {   
            dc.SetTextBackground( attr.GetBackgroundColour() );
            dc.SetTextForeground( attr.GetTextColour() );
        }
    }
    else
    {   
        dc.SetTextBackground(wxSystemSettings::GetColour(wxSYS_COLOUR_BTNFACE));
        dc.SetTextForeground(wxSystemSettings::GetColour(wxSYS_COLOUR_GRAYTEXT));
    }

    dc.SetFont( attr.GetFont() );
}

void CGridCellWrapStringRenderer::Draw(wxGrid& grid,
                      wxGridCellAttr& attr,
                      wxDC& dc,
                      const wxRect& rectCell,
                      int row, int col,
                      bool isSelected) {


    wxGridCellRenderer::Draw(grid, attr, dc, rectCell, row, col, isSelected);

    // now we only have to draw the text
    SetTextColoursAndFont(grid, attr, dc, isSelected);

    int horizAlign, vertAlign;
    attr.GetAlignment(&horizAlign, &vertAlign);

    wxRect rect = rectCell;
    rect.Inflate(-1);

    wxString text = BreakLines(grid.GetCellValue(row, col));

    grid.DrawTextRectangle(dc, text, rect, horizAlign, vertAlign);
}
 

wxSize CGridCellWrapStringRenderer::GetBestSize(wxGrid& grid,  wxGridCellAttr& attr,  wxDC& dc, int row, int col)
{
    dc.SetFont(attr.GetFont());
    wxString text = BreakLines(grid.GetCellValue(row, col));
    wxCoord w, h, lineHeight;
    dc.GetMultiLineTextExtent(text, &w, &h, &lineHeight); 
    return wxSize(w, h);
}
 
wxString CGridCellWrapStringRenderer::BreakLines(const wxString &text)
{
    const size_t chunk = 40;
    wxString out;
    for (size_t i = 0; i < text.Length(); i += chunk)
    {
        size_t len = min(chunk, text.Length() - i);
        out << text.Mid(i, len) << "\n";
    }
    return out;
}

// CSeqTableGrid

CSeqTableGrid::CSeqTableGrid(CRef<CSeq_table> values_table)
    : m_ValuesTable(values_table)
{
    SetAttrProvider(new CSeqGridCellAttrProvider);
}

int CSeqTableGrid::GetNumberRows()
{
    return m_ValuesTable->GetNum_rows();
}
int CSeqTableGrid::GetNumberCols()
{
    return m_ValuesTable->GetColumns().size() - 1;
}
bool CSeqTableGrid::IsEmptyCell(int row, int col)
{
    return false;
}

wxString CSeqTableGrid::GetValue(int row, int col)
{
    size_t pos = col + 1;
    size_t row_num = row;
    if (m_ValuesTable->GetColumns().size() > pos && m_ValuesTable->GetColumns()[pos]->GetData().GetSize() > row_num) {
        CRef<objects::CSeqTable_column> column = m_ValuesTable->GetColumns()[pos];
        string val = "";
        if (column->GetData().IsString()) {
            val = column->GetData().GetString()[row_num];
        }
        else if (column->GetData().IsId()) {
            column->GetData().GetId()[row]->GetLabel(&val, objects::CSeq_id::eContent);
        }
        else if (column->GetData().IsInt()) {
            val = NStr::NumericToString(column->GetData().GetInt()[row_num]);
        }
        return wxString(val);
    }
    else {
        return wxEmptyString;
    }
}

void CSeqTableGrid::SetValue(int vis_row, int vis_col, const wxString& value)
{
    size_t vcol = vis_col + 1;
    size_t vrow = vis_row;
    if (m_ValuesTable->GetColumns().size() > vcol) {
        CRef<objects::CSeqTable_column> col = m_ValuesTable->GetColumns()[vcol];
        if (col->GetData().IsString()) {
            while (col->GetData().GetSize() <= vrow) {
                col->SetData().SetString().push_back("");
            }
            col->SetData().SetString()[vrow] = value.ToStdString();
        }
        else if (col->GetData().IsInt()) {
            col->SetData().SetInt()[vrow] = NStr::StringToInt(value.ToStdString());
        }
        else {
            col->SetData().SetString()[vrow] = value.ToStdString();
        }
    }
}

bool CSeqTableGrid::InsertCols(size_t pos, size_t numCols)
{
    size_t skip = 0;
    objects::CSeq_table::TColumns::iterator it = m_ValuesTable->SetColumns().begin();
    while (it != m_ValuesTable->SetColumns().end() && skip < pos + 1) {
        it++;
        skip++;
    }

    for (size_t i = 0; i < numCols; i++) {
        CRef< objects::CSeqTable_column > last_col(new objects::CSeqTable_column());
        last_col->SetData().SetString();
        m_ValuesTable->SetColumns().insert(it, last_col);
    }
    if (GetView())
    {
        wxGridTableMessage msg(this,
            wxGRIDTABLE_NOTIFY_COLS_INSERTED,
            pos,
            numCols);

        GetView()->ProcessTableMessage(msg);
    }
    return true;
}

bool CSeqTableGrid::AppendCols(size_t numCols)
{
    for (size_t i = 0; i < numCols; i++) {
        CRef< objects::CSeqTable_column > last_col(new objects::CSeqTable_column());
        last_col->SetData().SetString();
        m_ValuesTable->SetColumns().push_back(last_col);
    }

    if (GetView())
    {
        wxGridTableMessage msg(this,
            wxGRIDTABLE_NOTIFY_COLS_APPENDED,
            numCols,
            numCols);

        GetView()->ProcessTableMessage(msg);
    }
    return true;
}

bool CSeqTableGrid::DeleteCols(size_t pos, size_t numCols)
{
    if (pos + numCols < m_ValuesTable->SetColumns().size())
    {
        m_ValuesTable->SetColumns().erase(m_ValuesTable->SetColumns().begin() + pos, m_ValuesTable->SetColumns().begin() + pos + numCols);

        if (GetView())
        {
            wxGridTableMessage msg(this,
                wxGRIDTABLE_NOTIFY_COLS_DELETED,
                pos,
                numCols);

            GetView()->ProcessTableMessage(msg);
        }
    }
    return true;
}

wxString CSeqTableGrid::GetRowLabelValue(int row)
{
    CRef<objects::CSeqTable_column> id_col = FindSeqTableColumnByName(m_ValuesTable, kSequenceIdColLabel);
    string label = "";
    size_t r = row;
    if (id_col && id_col->GetData().GetSize() > r) {
        id_col->GetData().GetId()[r]->GetLabel(&label, objects::CSeq_id::eContent);
    }
    else {
        label = NStr::NumericToString(r + 1);
    }
    return wxString(label);
}

wxString CSeqTableGrid::GetColLabelValue(int col)
{
    wxString label;
    size_t c = col + 1;
    if (m_ValuesTable->GetColumns().size() > c) {
        CRef<objects::CSeqTable_column> column = m_ValuesTable->GetColumns()[c];
        if (column->IsSetHeader()) {
            if (column->GetHeader().IsSetTitle()) {
                label = column->GetHeader().GetTitle();
            }
            else if (column->GetHeader().IsSetField_name()) {
                label = column->GetHeader().GetField_name();
            }
        }
    }
    return label;
}

void CSeqTableGrid::SetColLabelValue(int col, const wxString& label)
{
    size_t c = col + 1;
    if (m_ValuesTable->GetColumns().size() > c) {
        m_ValuesTable->SetColumns()[c]->SetHeader().SetTitle(label.ToStdString());
    }
}

/*!
 * CSeqTableGridPanel type definition
 */

IMPLEMENT_DYNAMIC_CLASS( CSeqTableGridPanel, wxPanel )


/*!
 * CSeqTableGridPanel event table definition
 */

BEGIN_EVENT_TABLE( CSeqTableGridPanel, wxPanel )

////@begin CSeqTableGridPanel event table entries
    EVT_GRID_CELL_RIGHT_CLICK( CSeqTableGridPanel::OnCellRightClick )
    EVT_GRID_LABEL_RIGHT_CLICK( CSeqTableGridPanel::OnLabelRightClick )

////@end CSeqTableGridPanel event table entries
    EVT_GRID_CELL_LEFT_CLICK( CSeqTableGridPanel::OnCellLeftClick )
    EVT_GRID_CELL_LEFT_DCLICK( CSeqTableGridPanel::OnCellDoubleLeftClick )
    EVT_GRID_LABEL_LEFT_CLICK( CSeqTableGridPanel::OnLabelLeftClick )
    EVT_GRID_LABEL_LEFT_DCLICK( CSeqTableGridPanel::OnLabelLeftDClick )
    EVT_GRID_COL_SORT( CSeqTableGridPanel::OnSortTableByColumn )
    EVT_MENU( eCmdCopyTableValues, CSeqTableGridPanel::OnCopyCells )
    EVT_MENU( eCmdPasteTableValues, CSeqTableGridPanel::OnPasteCells )
    EVT_MENU( eCmdAppendTableValues, CSeqTableGridPanel::OnPasteAppendCells )
    EVT_MENU( eCmdCopyTableValuesFromId, CSeqTableGridPanel::OnCopyCellsFromId )
    EVT_MENU( eCmdSearchTable, CSeqTableGridPanel::OnSearchTable )
    EVT_MENU( eCmdDeleteTableCol, CSeqTableGridPanel::OnDeleteColumn )
    EVT_MENU( eCmdRenameTableCol, CSeqTableGridPanel::OnRenameColumn )
    EVT_MENU( eCmdCollapseTableCol, CSeqTableGridPanel::OnCollapseColumn )
    EVT_MENU( eCmdExpandTableCol, CSeqTableGridPanel::OnExpandColumn )
END_EVENT_TABLE()


/*!
 * CSeqTableGridPanel constructors
 */

CSeqTableGridPanel::CSeqTableGridPanel()
{
    Init();
}

CSeqTableGridPanel::CSeqTableGridPanel( wxWindow* parent, CRef<CSeq_table> values_table, CRef<CSeq_table> choices,  int glyph_col, 
                                        wxWindowID id, const wxPoint& pos, const wxSize& size, long style )
    : m_Table(values_table), m_Choices(choices), m_Copied (NULL), m_CollapseGlyphCol(glyph_col)
{
    Init();
    Create(parent, id, pos, size, style);   
}


/*!
 * CSeqTableGrid creator
 */

bool CSeqTableGridPanel::Create( wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style )
{
////@begin CSeqTableGridPanel creation
    wxPanel::Create( parent, id, pos, size, style );

    CreateControls();
    if (GetSizer())
    {
        GetSizer()->SetSizeHints(this);
    }
    Centre();
////@end CSeqTableGridPanel creation
    return true;
}


/*!
 * CSeqTableGridPanel destructor
 */

CSeqTableGridPanel::~CSeqTableGridPanel()
{
////@begin CSeqTableGridPanel destruction
////@end CSeqTableGridPanel destruction
}


/*!
 * Member initialisation
 */

void CSeqTableGridPanel::Init()
{
////@begin CSeqTableGridPanel member initialisation
////@end CSeqTableGridPanel member initialisation
    x_SetUpMenu();
    m_SortByRowLabel = false;
    m_SortByRowLabelAscend = false;
    m_CollapseCol = -1;
    m_SortCol = -1;
    m_MenuCol = -1;
    m_Modified = false;
    m_edit_cell_row = -1;
    m_edit_cell_col = -1;
}


/*!
 * Control creation for CSeqTableGrid
 */

void CSeqTableGridPanel::CreateControls()
{    
////@begin CSeqTableGridPanel content construction
    // Generated by DialogBlocks, 10/06/2013 17:54:14 (unregistered)

    CSeqTableGridPanel* itemPanel1 = this;

    wxBoxSizer* itemBoxSizer2 = new wxBoxSizer(wxVERTICAL);
    itemPanel1->SetSizer(itemBoxSizer2);

    CCollapsibleGrid* itemGrid3 = new CCollapsibleGrid( itemPanel1, ID_GRID, wxDefaultPosition, wxSize(600, 300), wxSUNKEN_BORDER|wxHSCROLL|wxVSCROLL|wxTAB_TRAVERSAL );
    itemGrid3->SetDefaultColSize(50);
    itemGrid3->SetDefaultRowSize(25);
    itemGrid3->SetColLabelSize(25);
    itemGrid3->SetRowLabelSize(100);
    itemGrid3->SetDefaultRenderer(new CGridCellWrapStringRenderer());
    itemGrid3->CreateGrid(5, 5, wxGrid::wxGridSelectCells);
    itemBoxSizer2->Add(itemGrid3, 0, wxGROW|wxALL, 5);

////@end CSeqTableGridPanel content construction
    m_Grid = itemGrid3;
    CSeqTableGrid *gridAdapter = new CSeqTableGrid(m_Table);
    m_Grid->SetTable(gridAdapter, true);
    x_MakeProblemsColumnReadOnly();
    SetColumnSizesAndChoices();  
    m_Grid->Connect(wxID_ANY, wxEVT_KEY_DOWN, wxKeyEventHandler(CSeqTableGridPanel::CopyPasteDataByKeyboard), (wxObject*) NULL, this);  // connect m_Grid to ctrl-v/ctrl-c event handlers
    m_Grid->SetTabBehaviour(wxGrid::Tab_Leave);
}

void CSeqTableGridPanel::InitColumnCollapse(int col)
{
    m_Grid->x_InitRowHeights();
    m_Grid->AutoSizeRows(false);
    x_SortTableByColumn(col,true); 
    m_CollapseCol = col;
    x_CollapseTableByColumn(true);
}

void CSeqTableGridPanel::MakeColumnReadOnly(int pos, bool val)
{
    if (pos < m_Grid->GetNumberCols()) {
        for (int i = 0; i < m_Grid->GetNumberRows(); i++) {
            m_Grid->GetOrCreateCellAttr(i, pos)->SetReadOnly(val);
        }
        if (val)
            m_SetOfReadOnlyCols.insert(pos);
        else
            m_SetOfReadOnlyCols.erase(pos);
    }
}


void CSeqTableGridPanel::MakeColumnReadOnly(string name, bool val)
{
    int pos = m_Grid->GetNumberCols();
    for (int i = 0; i < m_Grid->GetNumberCols(); i++) {
        if (NStr::EqualNocase(ToStdString(m_Grid->GetColLabelValue(i)), name)) {
            pos = i;
            break;
        }
    }

    MakeColumnReadOnly(pos, val);
}


void CSeqTableGridPanel::MakeAllColumnsReadOnly()
{
    for (int c = 0; c < m_Grid->GetNumberCols(); c++) {
        MakeColumnReadOnly(c, true);
    }
}


void CSeqTableGridPanel::x_MakeProblemsColumnReadOnly()
{
    CSeqTableGrid *gridAdapter = dynamic_cast<CSeqTableGrid*>(m_Grid->GetTable());
    if (!gridAdapter) {
        return;
    }
    CRef<objects::CSeq_table> values_table = gridAdapter->GetValuesTable();
    if (!values_table) {
        return;
    }

    int problem_pos = m_Grid->GetNumberCols();
    for (int i = 0; i < m_Grid->GetNumberCols(); i++) {
        if (NStr::EqualNocase(ToStdString(m_Grid->GetColLabelValue(i)), kProblems)) {
            problem_pos = i;
            break;
        }
    }
    MakeColumnReadOnly(problem_pos);
}


void CSeqTableGridPanel::UpdateColumnLabelHeight()
{
    int lines = 1;
    for (int i = 0; i < m_Grid->GetNumberCols(); i++) {
        string label = ToStdString(m_Grid->GetColLabelValue(i));
        size_t pos = NStr::Find(label, "\n");
        int lines_this_column = 1;
        while (pos != string::npos) {
            lines_this_column++;
            pos = NStr::Find(label, "\n", pos + 1);
        }
        if (lines_this_column > lines) {
            lines = lines_this_column;
        }            
    }
    int l_height = m_Grid->GetColLabelSize();
    m_Grid->SetColLabelSize( lines * l_height );
}


void CSeqTableGridPanel::UpdateColumnChoices(int pos, vector<string> choices)
{
    if (!m_Choices) {
        if (choices.empty()) {
            // no choices before, no new choices, nothing to do
            return;
        }
        m_Choices = new objects::CSeq_table();
        CRef<objects::CSeqTable_column> col(new objects::CSeqTable_column());
        m_Choices->SetColumns().push_back(col);
    }

    while (pos >= m_Choices->GetColumns().size()) {
        CRef<objects::CSeqTable_column> col(new objects::CSeqTable_column());
        m_Choices->SetColumns().push_back(col);
    }
    m_Choices->SetColumns()[pos]->ResetData();
    if (!choices.empty()) {
        CRef<objects::CSeqTable_column> col = m_Choices->SetColumns()[pos];
        ITERATE (vector<string>, it, choices) {
            col->SetData().SetString().push_back(*it);
        }
        wxArrayString val_list;
        size_t max_len = 10;
        wxClientDC dc(this);
        for (size_t j = 0; j < col->GetData().GetString().size(); j++) {
            string val = col->GetData().GetString()[j];
            val_list.push_back(ToWxString(val));  
            wxSize text_size = dc.GetTextExtent(val);
            size_t this_len = text_size.GetWidth();
            if (this_len > max_len) {
                max_len = this_len;
            }
        }
        if (col->GetData().GetString().size() == 2
            && IsSynonymForTrue(col->GetData().GetString()[0])
            && IsSynonymForFalse(col->GetData().GetString()[1])) {
            wxGridCellBoolEditor::UseStringValues(wxT("true"), wxEmptyString);
            for (int row = 0; row < m_Grid->GetNumberRows(); row++) {
                m_Grid->SetCellEditor(row, pos, new wxGridCellBoolEditor ());
            }
            m_Grid->AutoSizeColLabelSize(pos);
        } else {
            for (int row = 0; row < m_Grid->GetNumberRows(); row++) {
                m_Grid->SetCellEditor(row, pos, new wxGridCellChoiceEditor (val_list, false));
            }
            m_Grid->SetColMinimalWidth(pos, max_len + wxSYS_VSCROLL_X);
            m_Grid->SetColSize(pos, max_len + wxSYS_VSCROLL_X);
        }
    } else {
        for (int row = 0; row < m_Grid->GetNumberRows(); row++) {
            m_Grid->SetCellEditor(row, pos, new wxGridCellTextEditor ());
        }
        m_Grid->AutoSizeColLabelSize(pos);
    }
    m_Grid->AutoSizeRows(false);
}


void CSeqTableGridPanel::SetColumnSizesAndChoices()
{
    if (m_Choices) {
        size_t i = 0;
        for (; i < m_Choices->GetColumns().size(); i++) {
            CConstRef<CSeqTable_column> col = m_Choices->GetColumns()[i];
            if (col->IsSetData() && col->GetData().IsString() && col->GetData().GetString().size() > 0) {
                wxArrayString val_list;
                size_t max_len = 10;
                wxClientDC dc(this);
                for (size_t j = 0; j < col->GetData().GetString().size(); j++) {
                    string val = col->GetData().GetString()[j];
                    val_list.push_back(ToWxString(val));  
                    wxSize text_size = dc.GetTextExtent(val);
                    size_t this_len = text_size.GetWidth();
                    if (this_len > max_len) {
                        max_len = this_len;
                    }
                }
                if (col->GetData().GetString().size() == 2
                    && IsSynonymForTrue(col->GetData().GetString()[0])
                    && IsSynonymForFalse(col->GetData().GetString()[1])) {
                    wxGridCellBoolEditor::UseStringValues(wxT("true"), wxEmptyString);
                    for (int row = 0; row < m_Grid->GetNumberRows(); row++) {
                        m_Grid->SetCellEditor(row, i, new wxGridCellBoolEditor ());
                    }
                    m_Grid->AutoSizeColLabelSize(i);
                } else {
                    for (int row = 0; row < m_Grid->GetNumberRows(); row++) {
                        m_Grid->SetCellEditor(row, i, new wxGridCellChoiceEditor (val_list, false));
                    }
                    m_Grid->SetColMinimalWidth(i, max_len + wxSYS_VSCROLL_X);
                    m_Grid->SetColSize(i, max_len + wxSYS_VSCROLL_X);
                }
            } else {
                m_Grid->AutoSizeColLabelSize(i);
            }
        }
        while (i < m_Grid->GetNumberCols()) {
            string label = ToStdString(m_Grid->GetColLabelValue(i));
            if (NStr::EqualNocase(label, kProblems)) {
                m_Grid->AutoSizeColumn(i);
            } else {
                m_Grid->AutoSizeColLabelSize(i);
            }
            i++;
        }
    } else {
        m_Grid->AutoSizeColumns();
    }    
    InitMapRowLabelToIndex();
    SetBestTableWidth();
}


int CSeqTableGridPanel::GetBestTableWidth()
{
    // calculate "best" table width
    int sum = m_Grid->GetRowLabelSize();
    for (size_t i = 0; i < m_Grid->GetNumberCols(); i++) {
        sum += m_Grid->GetColSize(i);
    }
    return sum;
}


void CSeqTableGridPanel::SetBestTableWidth()
{
    // calculate "best" table width
    int sum = GetBestTableWidth();

    wxSize current_size = m_Grid->GetSize();
    if (sum > current_size.x) {
        current_size.x = sum;
        m_Grid->SetMinClientSize(current_size);
    }
}


/*!
 * Should we show tooltips?
 */

bool CSeqTableGridPanel::ShowToolTips()
{
    return true;
}

/*!
 * Get bitmap resources
 */

wxBitmap CSeqTableGridPanel::GetBitmapResource( const wxString& name )
{
    // Bitmap retrieval
////@begin CSeqTableGridPanel bitmap retrieval
    wxUnusedVar(name);
    return wxNullBitmap;
////@end CSeqTableGridPanel bitmap retrieval
}

/*!
 * Get icon resources
 */

wxIcon CSeqTableGridPanel::GetIconResource( const wxString& name )
{
    // Icon retrieval
////@begin CSeqTableGridPanel icon retrieval
    wxUnusedVar(name);
    return wxNullIcon;
////@end CSeqTableGridPanel icon retrieval
}

void CSeqTableGridPanel::x_ExpandTable()
{
    if (m_CollapseGlyphCol >= 0 && m_CollapseCol >= 0)
    {
        for (unordered_map<int, int>::iterator it=m_CollapseCell.begin(); it != m_CollapseCell.end(); ++it)
            if (it->second == 1)
                it->second = -1;
        x_ExpandTableByColumn();
        for (int i = 0; i < m_Grid->GetNumberRows(); i++)
            m_Grid->SetCellValue(i,m_CollapseGlyphCol,wxEmptyString);
        m_CollapseCol = -1;
        m_CollapseCache.clear();
        m_CollapseCell.clear();
    }
}


CRef<CSeq_table> CSeqTableGridPanel::GetValuesTable()
{
    CSeqTableGrid *gridAdapter = dynamic_cast<CSeqTableGrid*>(m_Grid->GetTable());
    if( !gridAdapter ) {
        CRef<objects::CSeq_table> empty;
        return empty;
    }
    x_ExpandTable();
    m_SortByRowLabelAscend = true;
    x_SortByRowLabel();
    CRef<objects::CSeq_table> values_table = gridAdapter->GetValuesTable();
    return values_table;
}


void CSeqTableGridPanel::SetValuesTable(CRef<objects::CSeq_table> table)
{
    CSeqTableGrid *gridAdapter = new CSeqTableGrid(table);
    m_Grid->SetTable(gridAdapter, true);
    x_MakeProblemsColumnReadOnly();
    SetColumnSizesAndChoices();
    m_Grid->ForceRefresh();        
}


void CSeqTableGridPanel::x_SetUpMenu()
{
    static bool registered = false;
    if (!registered) {
        registered = true;

        CUICommandRegistry& cmd_reg = CUICommandRegistry::GetInstance();
        cmd_reg.RegisterCommand(eCmdCopyTableValues,
            "Copy",
            "Copy",
            NcbiEmptyString,
            NcbiEmptyString,
            "Copy cell or cells");
        cmd_reg.RegisterCommand(eCmdPasteTableValues,
            "Paste",
            "Paste",
            NcbiEmptyString,
            NcbiEmptyString,
            "Paste cell or cells");
        cmd_reg.RegisterCommand(eCmdAppendTableValues,
            "Append",
            "Append",
            NcbiEmptyString,
            NcbiEmptyString,
            "Append to cell or cells");
        cmd_reg.RegisterCommand(eCmdCopyTableValuesFromId,
            "Paste from SeqID",
            "Paste from SeqID",
            NcbiEmptyString,
            NcbiEmptyString,
            "Paste to cell or cells from SeqId"); 
        cmd_reg.RegisterCommand(eCmdSearchTable,
            "Search",
            "Search",
            NcbiEmptyString,
            NcbiEmptyString,
            "Search Table");
        cmd_reg.RegisterCommand(eCmdDeleteTableCol,
            "Delete",
            "Delete",
            NcbiEmptyString,
            NcbiEmptyString,
            "Delete Column");
        cmd_reg.RegisterCommand(eCmdRenameTableCol,
            "Rename",
            "Rename",
            NcbiEmptyString,
            NcbiEmptyString,
            "Rename Column");
        cmd_reg.RegisterCommand(eCmdCollapseTableCol,
            "Collapse",
            "Collapse",
            NcbiEmptyString,
            NcbiEmptyString,
            "Collapse Column");
        cmd_reg.RegisterCommand(eCmdExpandTableCol,
            "Expand",
            "Expand",
            NcbiEmptyString,
            NcbiEmptyString,
            "Expand Column");
    }
}



static
WX_DEFINE_MENU(kEditSeqTableMenu)
    WX_MENU_ITEM(eCmdCopyTableValues)
    WX_MENU_ITEM(eCmdPasteTableValues)
    WX_MENU_ITEM(eCmdAppendTableValues)
    WX_MENU_ITEM(eCmdCopyTableValuesFromId) 
    WX_MENU_ITEM(eCmdSearchTable)
    WX_MENU_ITEM(eCmdDeleteTableCol) 
    WX_MENU_ITEM(eCmdRenameTableCol)
    WX_MENU_ITEM(eCmdCollapseTableCol) 
    WX_MENU_ITEM(eCmdExpandTableCol) 
WX_END_MENU()

void CSeqTableGridPanel::x_CreateMenu( wxGridEvent& evt )
{
    wxMenu* menu = CUICommandRegistry::GetInstance().CreateMenu(kEditSeqTableMenu);
    if (menu) {
        m_MenuCol = evt.GetCol();
        m_Grid->PopupMenu(menu, evt.GetPosition());
    }
}

void CSeqTableGridPanel::InitMapRowLabelToIndex()
{
    m_MapRowLabelToIndex.clear();
    CSeqTableGrid *gridAdapter = dynamic_cast<CSeqTableGrid*>(m_Grid->GetTable());
    if( !gridAdapter ) {
        return;
    }
    int num_rows = gridAdapter->GetNumberRows();
    for (int i=0; i<num_rows; i++)
    {
        string s = ToStdString(gridAdapter->GetRowLabelValue(i));
        m_MapRowLabelToIndex[s]= i;
    }
}


bool  cmpTableValues(const pair< pair<string,int>,int> &a, const pair< pair<string,int>,int> &b) // we don't seem to have tuple yet (?!) so pairs it is
{
    double l, r;
    if (a.first.first == b.first.first) return a.first.second < b.first.second;
    if (NStr::StringToNumeric(a.first.first,&l,NStr::fConvErr_NoThrow | NStr::fAllowLeadingSpaces | NStr::fAllowTrailingSpaces) && 
        NStr::StringToNumeric(b.first.first,&r,NStr::fConvErr_NoThrow | NStr::fAllowLeadingSpaces | NStr::fAllowTrailingSpaces))
        return l<r;
    return a.first.first < b.first.first;
}

/*!
 * wxEVT_GRID_COL_SORT event handler for ID_GRID
 */

void CSeqTableGridPanel::OnSortTableByColumn( wxGridEvent& event )  // see "Sorting support" in http://docs.wxwidgets.org/trunk/classwx_grid.html
{
    if (m_Grid->GetSortingColumn() == -1 || m_Grid->GetSortingColumn() == m_CollapseGlyphCol) 
    { 
        m_Grid->Refresh();
        return;
    }
    
    int col = m_Grid->GetSortingColumn();
    if (col != m_SortCol)
    {
        m_SortCol = col;
        return;
    }
    bool ascend = m_Grid->IsSortOrderAscending();
    x_ExpandTable();
    x_SortTableByColumn(col,ascend);
    m_CollapseCol = col;
    x_CollapseTableByColumn(true);
    m_SortByRowLabel = true;
    m_SortByRowLabelAscend = true;
    m_Grid->Refresh();
}

void CSeqTableGridPanel::x_SortTableByColumn(int col, bool ascend)
{  
    CSeqTableGrid *gridAdapter = dynamic_cast<CSeqTableGrid*>(m_Grid->GetTable());
    if( !gridAdapter ) {
        return;
    }
    CRef<objects::CSeq_table> values_table = gridAdapter->GetValuesTable();

    CConstRef<objects::CSeqTable_column> column = values_table->GetColumns()[col+1];
    if (!column->IsSetData() || !column->GetData().IsString())
    { 
        //m_Grid->Refresh();
        return;
    }
    int num_rows = 0;
    int num_cols = values_table->GetColumns().size();

    for (int j=0; j<num_cols; j++)
    {
        switch (values_table->GetColumns()[j]->GetData().Which())
        {
        case CSeqTable_multi_data::e_Id:            
            if (values_table->SetColumns()[j]->SetData().SetId().size() > num_rows)
                num_rows = values_table->SetColumns()[j]->SetData().SetId().size();
            break;
        case CSeqTable_multi_data::e_String:
            if (values_table->SetColumns()[j]->SetData().SetString().size() > num_rows)
                num_rows = values_table->SetColumns()[j]->SetData().SetString().size();
            break;
        default:
            break;
        }
    }

    vector<vector<bool> > selection(num_rows, vector<bool>(num_cols,false));
    for (int i=0; i<num_rows; i++)
        for (int j=0; j<num_cols; j++)
            if (m_Grid->IsInSelection(i,j))
                selection[i][j] = true;

    for (int j=0; j<num_cols; j++)
    {
        switch (values_table->GetColumns()[j]->GetData().Which())
        {
        case CSeqTable_multi_data::e_Id:            
            values_table->SetColumns()[j]->SetData().SetId().resize(num_rows);
            break;
        case CSeqTable_multi_data::e_String:
            values_table->SetColumns()[j]->SetData().SetString().resize(num_rows);
            break;
        default:
            break;
        }
    }
    CRef<objects::CSeq_table> old_table(new CSeq_table);
    old_table->Assign(*values_table);  

    vector < pair< pair<string,int>,int> > sorted;
    for (int i=0; i<num_rows; i++)
    {
        string s1 = column->GetData().GetString()[i];
        string s2 = ToStdString(gridAdapter->GetRowLabelValue(i));
        pair<string,int> p(s1,m_MapRowLabelToIndex[s2]);
        sorted.push_back(pair< pair<string,int>,int>(p,i));
    }
    std::sort(sorted.begin(),sorted.end(),cmpTableValues); 
   
    if (!ascend)
        std::reverse(sorted.begin(),sorted.end());
    m_Grid->ClearSelection();
    for (int i=0; i<num_rows; i++)
    {
        int old_i = sorted[i].second;
        for (int j=0; j<num_cols; j++)
        {
            switch (values_table->GetColumns()[j]->GetData().Which())
            {
                case CSeqTable_multi_data::e_Id:
                    values_table->SetColumns()[j]->SetData().SetId()[i] = old_table->SetColumns()[j]->GetData().GetId()[old_i];
                    break;
                case CSeqTable_multi_data::e_String:
                    values_table->SetColumns()[j]->SetData().SetString()[i] = old_table->SetColumns()[j]->GetData().GetString()[old_i];
                    break;
            default:
                break;
            }
            if (selection[old_i][j])
                m_Grid->SelectBlock(i,j,i,j,true);
        }
    }
}



void CSeqTableGridPanel::x_UpdateCollapsedRow(unsigned int num_cols, int expand_row, vector<bool> &all_present, vector<bool> &all_same, vector<wxString> &values)
{
    unordered_map<int, int>::iterator it = m_CollapseCell.find(expand_row);
    if (it == m_CollapseCell.end())
        return;
    for (unsigned int j = 0; j < num_cols; j++) 
        if (j != m_CollapseCol && j != m_CollapseGlyphCol)
        {
            if (it->second == 1)
            {
                wxString val = m_Grid->GetCellValue(expand_row,j);
                if (val != _("All present, mixed") && val != _("Some missing, one unique") && val != _("Some missing, mixed") &&
                    !(all_present[j] && all_same[j]))
                {
                    m_CollapseCache[pair<int,int>(expand_row,j)] = m_Grid->GetCellValue(expand_row,j);
                }
                if (all_present[j] && !all_same[j])
                {
                    m_Grid->SetCellValue(expand_row,j,_("All present, mixed"));
                    m_Grid->GetOrCreateCellAttr(expand_row,j)->SetReadOnly(true);
                }
                if (!all_present[j] && all_same[j] && !values[j].IsEmpty())
                {
                    m_Grid->SetCellValue(expand_row,j,_("Some missing, one unique"));
                    m_Grid->GetOrCreateCellAttr(expand_row,j)->SetReadOnly(true);
                }
                if (!all_present[j] && !all_same[j])
                {
                    m_Grid->SetCellValue(expand_row,j,_("Some missing, mixed"));
                    m_Grid->GetOrCreateCellAttr(expand_row,j)->SetReadOnly(true);
                }
            }
            else
            {
                wxString val = m_Grid->GetCellValue(expand_row,j);
                if (val == _("All present, mixed") || val == _("Some missing, one unique") || val == _("Some missing, mixed"))
                    m_Grid->SetCellValue(expand_row,j,m_CollapseCache[pair<int,int>(expand_row,j)]);
                if (m_SetOfReadOnlyCols.find(j) == m_SetOfReadOnlyCols.end())
                    m_Grid->GetOrCreateCellAttr(expand_row,j)->SetReadOnly(false);
            }
            values[j].Clear();
            all_present[j] = true;
            all_same[j] = true;
        }
    if (it->second == 1)
        m_Grid->SetCellValue(expand_row,m_CollapseGlyphCol,_("+"));
    else
        m_Grid->SetCellValue(expand_row,m_CollapseGlyphCol,_("-"));        
}


void CSeqTableGridPanel::x_CollapseTableByColumn(bool initialize)
{    
    if (initialize)
    {
        m_CollapseCache.clear();
        m_CollapseCell.clear();
        m_Grid->SetRowMinimalAcceptableHeight(0);
    }

    if (!m_Grid || m_CollapseGlyphCol < 0 || m_CollapseCol < 0 || m_CollapseGlyphCol == m_CollapseCol) return;
    wxBusyCursor wait;

    unsigned int num_cols = m_Grid->GetNumberCols();
    unsigned int num_rows =  m_Grid->GetNumberRows(); 
   
    bool collapse = false;
    vector<bool> all_present(num_cols,true),all_same(num_cols,true);
    vector<wxString> values(num_cols);
    int expand_row = -1;

    for (unsigned int i = 0; i < num_rows; i++)
    {      
        unordered_map<int, int>::iterator it = m_CollapseCell.find(i);
        if (it != m_CollapseCell.end() && it->second == -1)
        {
            expand_row = i;
            continue;
        }
        if (expand_row >=0 && !m_Grid->IsRowShown(i))
        {
            for (unsigned int j = 0; j < num_cols; j++)
                if (!m_Grid->IsReadOnly(expand_row,j))
                    m_Grid->SetCellValue(i,j,m_Grid->GetCellValue(expand_row,j));
        }
        if ((it != m_CollapseCell.end() && it->second == 1) || m_Grid->IsRowShown(i))
            expand_row = -1;
    }
    m_Grid->x_InitRowHeights();
    m_Grid->BeginBatch();
    expand_row = -1;
    for (unsigned int i = 0; i < num_rows; i++)
    {      
        unordered_map<int, int>::iterator it = m_CollapseCell.find(i);
        if (!collapse && i > 0 && m_Grid->GetCellValue(i,m_CollapseCol) == m_Grid->GetCellValue(i-1,m_CollapseCol) && (it == m_CollapseCell.end() || it->second == 0))
        {
            collapse = true;
            for (unsigned j = 0; j < num_cols; j++)
                if (j != m_CollapseCol && j != m_CollapseGlyphCol)
                {
                    if (m_Grid->GetCellValue(i-1,j).IsEmpty())
                        all_present[j] = false;              
                    values[j] = m_Grid->GetCellValue(i-1,j);
                }
            expand_row = i-1;
            if (initialize)
            {
                m_CollapseCell[expand_row] = 1;
                m_Grid->SetCellAlignment(expand_row,m_CollapseGlyphCol,wxALIGN_CENTRE,wxALIGN_CENTRE);
            }
        }     

        if (collapse &&  i > 0 &&( m_Grid->GetCellValue(i,m_CollapseCol) != m_Grid->GetCellValue(expand_row,m_CollapseCol) || (it != m_CollapseCell.end() && it->second != 0) ))
        {
            collapse = false;
            x_UpdateCollapsedRow(num_cols, expand_row, all_present, all_same, values);
        }

       if (collapse)
       {
           for (unsigned j = 0; j < num_cols; j++)
               if (j != m_CollapseCol && j != m_CollapseGlyphCol)
               {
                   if (m_Grid->GetCellValue(i,j).IsEmpty())
                       all_present[j] = false;              
                   else
                   {
                       if (values[j].IsEmpty())
                           values[j] = m_Grid->GetCellValue(i,j);
                       if (m_Grid->GetCellValue(i,j) != values[j])
                           all_same[j] = false;
                   }
               }
           unordered_map<int, int>::iterator it = m_CollapseCell.find(expand_row);
           if (it != m_CollapseCell.end() && it->second == 1)
           {
               m_Grid->FastSetRowSize(i, 0);   
           }
           else 
           {
               int row_size = m_Grid->GetRowSize(expand_row);
               m_Grid->FastSetRowSize(i,row_size);
           }
       }
       else
           m_Grid->SetCellValue(i,m_CollapseGlyphCol,wxEmptyString);
    }

    if (collapse &&  num_rows > 0)
    {
        unsigned int i = num_rows-1;

        x_UpdateCollapsedRow(num_cols, expand_row, all_present, all_same, values);
        unordered_map<int, int>::iterator it = m_CollapseCell.find(expand_row);
        if (it != m_CollapseCell.end() && it->second == 1)
        {
            m_Grid->FastSetRowSize(i, 0);   
        }
        else 
        {
            int row_size = m_Grid->GetRowSize(expand_row);
            m_Grid->FastSetRowSize(i,row_size);
        }
    }
    m_Grid->UpdateRowBottoms();
    m_Grid->EndBatch();
}

void CSeqTableGridPanel::x_ExpandTableByColumn()
{   
    if (!m_Grid || m_CollapseGlyphCol < 0 || m_CollapseCol < 0 || m_CollapseGlyphCol == m_CollapseCol) return;

    wxBusyCursor wait;
    unsigned int num_cols = m_Grid->GetNumberCols();
    unsigned int num_rows =  m_Grid->GetNumberRows(); 
   
    int expand_row = -1;
    vector<bool> all_present(num_cols,true),all_same(num_cols,true);
    vector<wxString> values(num_cols);
    m_Grid->x_InitRowHeights();
    m_Grid->BeginBatch();
    for (unsigned int i = 0; i < num_rows; i++)
    {      
        unordered_map<int, int>::iterator it = m_CollapseCell.find(i);
        if (it != m_CollapseCell.end() && it->second == -1)
        {
            expand_row = i;
            continue;
        }
        if (expand_row >=0 && !m_Grid->IsRowShown(i))
        {
            for (unsigned j = 0; j < num_cols; j++)
                if (!m_Grid->IsReadOnly(expand_row,j))
                    m_Grid->SetCellValue(i,j,m_Grid->GetCellValue(expand_row,j));
            int row_size = m_Grid->GetRowSize(expand_row);
            m_Grid->FastSetRowSize(i,row_size);
        }
        else if (m_Grid->IsRowShown(i))
            expand_row = -1;
    }

    for (unsigned int i = 0; i < num_rows; i++)
    {      
        unordered_map<int, int>::iterator it = m_CollapseCell.find(i);
        if (it != m_CollapseCell.end() && it->second == -1)
        {
            m_Grid->SetCellValue(i,m_CollapseGlyphCol,wxEmptyString);
            x_UpdateCollapsedRow(num_cols, i, all_present, all_same, values);
        }
    }
    m_Grid->UpdateRowBottoms();
    m_Grid->EndBatch();
}

SrcEditDialog* CSeqTableGridPanel::x_GetParent()
{
    wxWindow* w = this->GetParent();
    while (w != NULL) {
        SrcEditDialog* parent = dynamic_cast<SrcEditDialog*>(w);
        if (parent) {
            return parent;
        } else {
            w = w->GetParent();
        }
    }
    return NULL;
}  

void CSeqTableGridPanel::OnRenameColumn( wxCommandEvent& event )  
{ 
    SrcEditDialog* dlg = x_GetParent();
    if (!dlg) 
    {
        m_MenuCol = -1;
        return;
    }

    if (!m_Grid->IsSelection()) 
    {
        m_MenuCol = -1;
        wxMessageBox( wxT("Please select a single column to rename"), wxT("Error"), wxOK | wxICON_ERROR, this); 
        return;
    }

    const wxGridCellCoordsArray& selected_blocks = m_Grid->GetSelectionBlockTopLeft();
    size_t num_blocks = selected_blocks.size();

    const wxArrayInt& cols = m_Grid->GetSelectedCols();
    size_t num_cols = cols.size();

    const wxGridCellCoordsArray& selected_block_right = m_Grid->GetSelectionBlockBottomRight();
    if (num_blocks != selected_block_right.size()) {
        m_MenuCol = -1;
        wxMessageBox( wxT("Selection is too complex"), wxT("Error"), wxOK | wxICON_ERROR, this);
        return;
    }
    
    int col = -1;
    if (num_blocks == 1) 
    {
        int left_col = selected_blocks[0].GetCol();
        int right_col = selected_block_right[0].GetCol();
        num_cols = right_col - left_col + 1;
        if (num_cols == 1)
            col = left_col;
    }
    else if (num_cols == 1)
        col = cols[0];
    
    if ( col >= 0 )
    {
        wxArrayString srcModNameStrings;
        dlg->GetQualChoices(srcModNameStrings);
        CRenameColumnDlg choice(NULL, srcModNameStrings);
        if (choice.ShowModal() == wxID_OK) 
        {
            wxString qual = choice.GetSelection();
            choice.Close();      
            if (!qual.IsEmpty())
            {
                m_Modified = true;
                m_Grid->GetTable()->SetColLabelValue(col, qual);
                dlg->ChangeColumnName(col, ToStdString(qual));
                dlg->ResetSubPanels();
                m_Grid->Refresh();
            }
        }
    }
    else
        wxMessageBox( wxT("Please select a single column to rename"), wxT("Error"), wxOK | wxICON_ERROR, this); 
    m_MenuCol = -1;
}

void CSeqTableGridPanel::OnCollapseColumn( wxCommandEvent& event )  
{ 
    if (m_CollapseGlyphCol < 0 )
    {
        m_MenuCol = -1;
        wxMessageBox( wxT("This table was not set as collapsible"), wxT("Error"), wxOK | wxICON_ERROR, this); 
        return;
    } 
    
    if ( m_MenuCol >= 0 && m_MenuCol != m_CollapseGlyphCol) //m_SetOfReadOnlyCols.find(col) == m_SetOfReadOnlyCols.end() )
    {
        x_ExpandTable();
        x_SortTableByColumn(m_MenuCol,true); 
        m_CollapseCol = m_MenuCol;
        x_CollapseTableByColumn(true);
    }
    else
        wxMessageBox( wxT("Please select a single editable column to collapse"), wxT("Error"), wxOK | wxICON_ERROR, this); 
    m_MenuCol = -1;
}

void CSeqTableGridPanel::OnExpandColumn( wxCommandEvent& event )  
{ 
    if (m_CollapseCol < 0 )
    {
        m_MenuCol = -1;
        wxMessageBox( wxT("This table does not have a collapsed column"), wxT("Error"), wxOK | wxICON_ERROR, this); 
        return;
    }

    x_ExpandTable();   
    m_MenuCol = -1;
}

int CSeqTableGridPanel::GetCollapseColAndExpand()
{
    int col = m_CollapseCol;
    x_ExpandTable(); 
    return col;
}

void CSeqTableGridPanel::CollapseByCol(int col)
{
    x_ExpandTable();
    m_CollapseCol = col;
    x_CollapseTableByColumn(true);
}

void CSeqTableGridPanel::OnDeleteColumn( wxCommandEvent& event )  
{ 
    SrcEditDialog* dlg = x_GetParent();
    if (!dlg) 
    {
        m_MenuCol = -1;
        return;
    }

    if (!m_Grid->IsSelection()) {
        m_MenuCol = -1;
        return;
    }
    CSeqTableGrid *gridAdapter = dynamic_cast<CSeqTableGrid*>(m_Grid->GetTable());
    if( !gridAdapter ) {
        m_MenuCol = -1;
        return;
    }

    const wxGridCellCoordsArray& selected_blocks = m_Grid->GetSelectionBlockTopLeft();
    size_t num_blocks = selected_blocks.size();

    const wxArrayInt& cols = m_Grid->GetSelectedCols();
    size_t num_cols = cols.size();

    if (num_blocks == 0 && num_cols == 0) {
        wxMessageBox( wxT("Nothing selected!"), wxT("Error"), wxOK | wxICON_ERROR, this);
        return;
    } else if (num_blocks > 1) {
        wxMessageBox( wxT("Selection is too complex"), wxT("Error"), wxOK | wxICON_ERROR, this);
        return;
    }
    const wxGridCellCoordsArray& selected_block_right = m_Grid->GetSelectionBlockBottomRight();
    if (num_blocks != selected_block_right.size()) {
        wxMessageBox( wxT("Selection is too complex"), wxT("Error"), wxOK | wxICON_ERROR, this);
        return;
    }

    m_Modified = true;
    int collapse_col = m_CollapseCol;
    x_ExpandTable();
    int shift_glyph = 0;
    int shift_col = 0;
    if (num_blocks == 1) 
    {
        int left_col = selected_blocks[0].GetCol();
        int right_col = selected_block_right[0].GetCol();
        for (int i = left_col; i <= right_col; i++)
        {
            gridAdapter->DeleteCols(i+1,1);
            if (m_CollapseGlyphCol == i)
                m_CollapseGlyphCol = -1;
            if (collapse_col == i)
                collapse_col = -1;
            if (i < m_CollapseGlyphCol)
                shift_glyph++;
            if (i < collapse_col)
                shift_col++;
            
        }
    }
    else if (num_cols > 0)
    {
        for (unsigned int i = 0; i < num_cols; i++) 
        {
            gridAdapter->DeleteCols(cols[i] + 1,1);
            if (m_CollapseGlyphCol == cols[i])
                m_CollapseGlyphCol = -1;
            if (collapse_col == cols[i])
                collapse_col = -1;
            if (cols[i] < m_CollapseGlyphCol)
                shift_glyph++;
            if (cols[i] < collapse_col)
                shift_col++;
            
        }
    }

    if (m_CollapseGlyphCol >= 0 && collapse_col >= 0)
    {
        m_CollapseGlyphCol -= shift_glyph;
        m_CollapseCol = collapse_col - shift_col;
        x_CollapseTableByColumn(true);
    }
   
    dlg->x_RepopulateAddQualList();    
    dlg->ResetSubPanels();
    m_Grid->Refresh();
    m_MenuCol = -1;
}



/*!
 * wxEVT_COMMAND_BUTTON_CLICKED event handler for eCmdCopyTableValues
 */
void CSeqTableGridPanel::OnCopyCells( wxCommandEvent& event )
{
    x_OnCopyCells();
    m_MenuCol = -1;
}

void CSeqTableGridPanel::x_OnCopyCells()
{
    if (!m_Grid->IsSelection()) {
        return;
    }

    const wxGridCellCoordsArray& selected_blocks = m_Grid->GetSelectionBlockTopLeft();
    size_t num_blocks = selected_blocks.size();

    const wxArrayInt& rows = m_Grid->GetSelectedRows();
    size_t num_rows = rows.size();

    const wxArrayInt& cols = m_Grid->GetSelectedCols();
    size_t num_cols = cols.size();

    if (num_blocks == 0 && num_rows == 0 && num_cols == 0) {
        wxMessageBox( wxT("Nothing selected!"), wxT("Error"), wxOK | wxICON_ERROR, this);
        return;
    } else if (num_blocks > 1 
               || (num_blocks > 0 && num_rows > 0) 
               || (num_blocks > 0 && num_cols > 0)
               || (num_rows > 0 && num_cols > 0)) {
        wxMessageBox( wxT("Selection is too complex"), wxT("Error"), wxOK | wxICON_ERROR, this);
        return;
    }
    const wxGridCellCoordsArray& selected_block_right = m_Grid->GetSelectionBlockBottomRight();
    if (num_blocks != selected_block_right.size()) {
        wxMessageBox( wxT("Selection is too complex"), wxT("Error"), wxOK | wxICON_ERROR, this);
        return;
    }

    CSeqTableGrid *gridAdapter = dynamic_cast<CSeqTableGrid*>(m_Grid->GetTable());
    if( !gridAdapter ) {
        return;
    }
    CRef<objects::CSeq_table> values_table = gridAdapter->GetValuesTable();

    m_Copied.Reset(new objects::CSeq_table());
    int top_row = 0;
    int left_col = 0;
    int bot_row = values_table->GetNum_rows() - 1;
    int right_col = values_table->GetColumns().size() - 1;
    if (num_blocks == 1) {
        top_row = selected_blocks[0].GetRow();
        left_col = selected_blocks[0].GetCol();
        bot_row = selected_block_right[0].GetRow();
        right_col = selected_block_right[0].GetCol();
        num_rows = bot_row - top_row + 1;
        num_cols = right_col - left_col + 1;

        for (unsigned int i = 0; i < num_cols; i++) {
            CConstRef<objects::CSeqTable_column> old_column = values_table->GetColumns()[left_col + i + 1];
            CRef<objects::CSeqTable_column> new_column(new objects::CSeqTable_column());
            new_column->SetHeader().Assign(old_column->GetHeader());
            unsigned j;
            if (old_column->IsSetData() && old_column->GetData().IsString())
                for (j = (unsigned)top_row; j <= (unsigned)bot_row && j < old_column->GetData().GetString().size(); j++) {
                    if (j < old_column->GetData().GetString().size()) {
                        new_column->SetData().SetString().push_back(old_column->GetData().GetString()[j]);
                    } else {
                        new_column->SetData().SetString().push_back("");
                    }
                }
            m_Copied->SetColumns().push_back(new_column);
        }
        m_Copied->SetNum_rows(num_rows);
    } else if (num_cols > 0) {
        for (unsigned int i = 0; i < num_cols; i++) {
            CConstRef<objects::CSeqTable_column> old_column = values_table->GetColumns()[cols[i] + 1];
            CRef<objects::CSeqTable_column> new_column(new objects::CSeqTable_column());
            new_column->Assign(*old_column);
            if (old_column->IsSetData() && old_column->GetData().IsString())
                while (new_column->GetData().GetString().size() < values_table->GetNum_rows()) {
                    new_column->SetData().SetString().push_back("");
                }
            m_Copied->SetColumns().push_back(new_column);
        }
        m_Copied->SetNum_rows(values_table->GetNum_rows());
    } else if (num_rows > 0) {        
        for (unsigned int i = 1; i < values_table->GetColumns().size(); i++) {
            CConstRef<objects::CSeqTable_column> old_column = values_table->GetColumns()[i];
            CRef<objects::CSeqTable_column> new_column(new objects::CSeqTable_column());
            new_column->SetHeader().Assign(old_column->GetHeader());
            for (unsigned int j = 0; j < num_rows; j++) {
                if (j<rows.size() && old_column->IsSetData() && old_column->GetData().IsString() && rows[j] < old_column->GetData().GetString().size()) {
                    string str = old_column->GetData().GetString()[rows[j]];
                    new_column->SetData().SetString().push_back(str);
                } else {
                    new_column->SetData().SetString().push_back("");
                }
            }
            m_Copied->SetColumns().push_back(new_column);
        }
        m_Copied->SetNum_rows(num_rows);
    }
}


static void s_CopyToIntColumn
(CConstRef<objects::CSeqTable_column> src, 
 CRef<objects::CSeqTable_column> dst, 
 int top, int bot)
{
    int j, k = 0;
    for (j = top; j <= bot; j++) {
        while (dst->SetData().SetInt().size() <= j) {
            dst->SetData().SetInt().push_back(0);
        }
        if (src->GetData().IsInt()) {
            dst->SetData().SetInt()[j] = src->GetData().GetInt()[k];
        } else if (src->GetData().IsString()) {
            dst->SetData().SetInt()[j] = NStr::StringToInt(src->GetData().GetString()[k]);
        } else {
            // skip it
        }
        k++;
        if (k >= src->GetData().GetSize()) {
            k = 0;
        }
    }

}


static void s_CopyToStringColumn 
(CConstRef<objects::CSeqTable_column> src, 
 CRef<objects::CSeqTable_column> dst, 
 int top, int bot, 
 bool append, string delim)
{
    int j, k = 0;
    for (j = top; j <= bot; j++) {
        while (dst->SetData().SetString().size() <= j) {
            dst->SetData().SetString().push_back("");
        }

        string val = "";
        if (src->GetData().IsString()) {
            val = src->GetData().GetString()[k];
        } else if (src->GetData().IsInt()) {
            val = NStr::NumericToString(src->GetData().GetInt()[k]);
        }
        if (!append || NStr::IsBlank(val)) {
            dst->SetData().SetString()[j] = val;
        } else {
            dst->SetData().SetString()[j] += delim + val;
        }

        k++;
        if (k >= src->GetData().GetSize()) {
            k = 0;
        }
    } 
}


static void s_CopyColumn 
(CConstRef<objects::CSeqTable_column> src, 
 CRef<objects::CSeqTable_column> dst, 
 int top, int bot, 
 bool append, string delim)
{
    if (dst->GetData().IsInt()) {
        s_CopyToIntColumn(src, dst, top, bot);
    } else if (dst->GetData().IsString()) {
        s_CopyToStringColumn(src, dst, top, bot, append, delim);
    } else {
        wxMessageBox( wxT("Bad column type!"), wxT("Error"), wxOK | wxICON_ERROR, NULL);
    }

}


static void s_CopyRow 
(CConstRef<objects::CSeq_table> src,
 CRef<objects::CSeq_table> dst,
 int left, int right, int src_row, int dst_row,
 bool append, string delim)
{
    int j, k = 0;

    for (j = left; j <= right; j++) {
        CConstRef<objects::CSeqTable_column> src_col = src->GetColumns()[k];
        CRef<objects::CSeqTable_column> dst_col = dst->SetColumns()[j];
        while (dst_col->SetData().SetString().size() <= dst_row) {
            dst_col->SetData().SetString().push_back("");
        }
        if (src_col->IsSetData() && src_col->GetData().IsString())
        {
            if (!append || NStr::IsBlank(dst_col->SetData().SetString()[dst_row])) {
                dst_col->SetData().SetString()[dst_row] = src_col->GetData().GetString()[src_row];
            } else {
                dst_col->SetData().SetString()[dst_row] += delim + src_col->GetData().GetString()[src_row];
            }
        }

        k++;
        if (k >= src->GetColumns().size()) {
            k = 0;
        }
    }
}


bool CSeqTableGridPanel::x_ValuesOkForNewColumn (CConstRef<CSeqTable_column> values, int col_pos)
{
    if (!m_Choices || m_Choices->GetColumns().size() <= col_pos) {
        // ok because there is no set choice list for this column
        return true;
    }
    CConstRef<CSeqTable_column> choices = m_Choices->GetColumns()[col_pos];
    if (!choices->IsSetData() || !choices->GetData().IsString()) {
        // ok because there is no set choice list for this column
        return true;
    }
    if (values->IsSetData() && values->GetData().IsString())
        for (size_t i = 0; i < values->GetData().GetString().size(); i++) {
            string new_val = values->GetData().GetString()[i];
            bool found = false;
            for (size_t j = 0; j < choices->GetData().GetString().size() && !found; j++) {
                string option = choices->GetData().GetString()[j];
                if (NStr::Equal(new_val, option)) {
                    found = true;
                }
            }
            if (!found) {
                return false;
            }
        }
    return true;
}


void CSeqTableGridPanel::x_PasteCells(CRef<objects::CSeq_table> copied, bool append, string delim)
{
    if (!m_Grid->IsSelection()) {
        return;
    }

    const wxGridCellCoordsArray& selected_blocks = m_Grid->GetSelectionBlockTopLeft();
    size_t num_blocks = selected_blocks.size();

    const wxArrayInt& rows = m_Grid->GetSelectedRows();
    size_t num_rows = rows.size();

    const wxArrayInt& cols = m_Grid->GetSelectedCols();
    size_t num_cols = cols.size();

    if (num_blocks == 0 && num_rows == 0 && num_cols == 0) {
        wxMessageBox( wxT("Nothing selected!"), wxT("Error"), wxOK | wxICON_ERROR, this);
        return;
    } else if (num_blocks > 1 
               || (num_blocks > 0 && num_rows > 0) 
               || (num_blocks > 0 && num_cols > 0)
               || (num_rows > 0 && num_cols > 0)) {
        wxMessageBox( wxT("Selection is too complex"), wxT("Error"), wxOK | wxICON_ERROR, this);
        return;
    }
    const wxGridCellCoordsArray& selected_block_right = m_Grid->GetSelectionBlockBottomRight();
    if (num_blocks != selected_block_right.size()) {
        wxMessageBox( wxT("Selection is too complex"), wxT("Error"), wxOK | wxICON_ERROR, this);
        return;
    }

    CSeqTableGrid *gridAdapter = dynamic_cast<CSeqTableGrid*>(m_Grid->GetTable());
    if( !gridAdapter ) {
        return;
    }
    CRef<objects::CSeq_table> values_table = gridAdapter->GetValuesTable();

    int top_row = 0;
    int left_col = 0;
    int bot_row = values_table->GetNum_rows() - 1;
    int right_col = values_table->GetColumns().size() - 1;
    if (num_blocks == 1) {
        top_row = selected_blocks[0].GetRow();
        left_col = selected_blocks[0].GetCol();
        bot_row = selected_block_right[0].GetRow();
        right_col = selected_block_right[0].GetCol();
        num_rows = bot_row - top_row + 1;
        num_cols = right_col - left_col + 1;

        if (num_rows % copied->GetNum_rows() != 0 || num_cols % copied->GetColumns().size() != 0) {
            wxMessageBox( wxT("Source size does not match destination size!"), wxT("Error"), wxOK | wxICON_ERROR, this);
            return;
        }
        // check to make sure that column types match
        int j = 0;
        for (int i = left_col; i <= right_col; i++) {
            CRef<objects::CSeqTable_column> new_column = values_table->SetColumns()[i + 1];
            CConstRef<objects::CSeqTable_column> cpy_column = copied->GetColumns()[j];
            if (!x_ValuesOkForNewColumn (cpy_column, i)) {
                wxMessageBox( wxT("Source values are inappropriate for destination values!"), wxT("Error"), wxOK | wxICON_ERROR, this);
                return;
            }
            j++;
            if (j >= copied->GetColumns().size()) {
                j = 0;
            }
        }
        // then paste
        j = 0;
        for (int i = left_col; i <= right_col; i++) {
            CRef<objects::CSeqTable_column> new_column = values_table->SetColumns()[i + 1];
            CConstRef<objects::CSeqTable_column> cpy_column = copied->GetColumns()[j];
            s_CopyColumn (cpy_column, new_column, top_row, bot_row, append, delim);
            j++;
            if (j >= copied->GetColumns().size()) {
                j = 0;
            }
        }
    } else if (num_cols > 0) {
        if (num_cols % copied->GetColumns().size() != 0) {
            wxMessageBox( wxT("Source size does not match destination size!"), wxT("Error"), wxOK | wxICON_ERROR, this);
            return;
        }
        // check to make sure that column types match
        int j = 0;
        for (int i = 0; i < num_cols; i++) {
            CRef<objects::CSeqTable_column> new_column = values_table->SetColumns()[cols[i] + 1];
            CConstRef<objects::CSeqTable_column> cpy_column = copied->GetColumns()[j];
            if (!x_ValuesOkForNewColumn (cpy_column, cols[i])) {
                wxMessageBox( wxT("Source values are inappropriate for destination values!"), wxT("Error"), wxOK | wxICON_ERROR, this);
                return;
            }
            j++;
            if (j >= copied->GetColumns().size()) {
                j = 0;
            }
        }

        // then paste
        j = 0;
        for (int i = 0; i < num_cols; i++) {            
            CRef<objects::CSeqTable_column> new_column = values_table->SetColumns()[cols[i] + 1];
            CConstRef<objects::CSeqTable_column> cpy_column = copied->GetColumns()[j];
            s_CopyColumn (cpy_column, new_column, 0, values_table->GetNum_rows() - 1, append, delim);
            j++;
            if (j >= copied->GetColumns().size()) {
                j = 0;
            }
        }
    } else if (num_rows > 0) {
        // TODO
        // check to make sure that column types match
        int j = 0;
        for (int i = 1; i < values_table->GetColumns().size(); i++) {
            if (!x_ValuesOkForNewColumn(copied->GetColumns()[j], i - 1)) {
                wxMessageBox( wxT("Source values are inappropriate for destination values!"), wxT("Error"), wxOK | wxICON_ERROR, this);
                return;
            }
            j++;
            if (j >= copied->GetColumns().size()) {
                j = 0;
            }
        }
      
        // then paste
        j = 0;
        for (int i = 0; i < num_rows; i++) {
            s_CopyRow (copied, values_table, 1, values_table->GetColumns().size() - 1, j, rows[i], append, delim);
            j++;
            if (j >= copied->GetNum_rows()) {
                j = 0;
            }
        }
    }
    m_Modified = true;
    m_Grid->Refresh();
}


/*!
 * wxEVT_COMMAND_BUTTON_CLICKED event handler for eCmdPasteTableValues
 */

void CSeqTableGridPanel::OnPasteCells( wxCommandEvent& event )
{
    x_OnPasteCells();
    m_MenuCol = -1;
}

void CSeqTableGridPanel::x_OnPasteCells()
{
    if (!m_Copied || !m_Copied->IsSetColumns() || !m_Copied->IsSetNum_rows()
        || m_Copied->GetColumns().size() == 0 || m_Copied->GetNum_rows() == 0) {
        wxTheClipboard->Open();
        wxTextDataObject data;
        wxTheClipboard->GetData( data );
        string val = ToStdString(data.GetText());
        if (!m_Copied) {
            m_Copied.Reset(new objects::CSeq_table());
        }
        if (!m_Copied->IsSetColumns() || m_Copied->GetColumns().size() == 0) {
            CRef<objects::CSeqTable_column> col(new objects::CSeqTable_column());
            m_Copied->SetColumns().push_back(col);
        } 
        m_Copied->SetColumns().front()->SetData().SetString().push_back(val);    
        m_Copied->SetNum_rows(1);
        wxTheClipboard->Close();
    }
    x_PasteCells (m_Copied);
}


/*!
 * wxEVT_COMMAND_BUTTON_CLICKED event handler for eCmdAppendTableValues
 */

void CSeqTableGridPanel::OnPasteAppendCells( wxCommandEvent& event )
{
    x_PasteCells (m_Copied, true);
    m_MenuCol = -1;
}


/*!
 * wxEVT_COMMAND_BUTTON_CLICKED event handler for eCmdCopyTableValuesFromId
 */

void CSeqTableGridPanel::OnCopyCellsFromId( wxCommandEvent& event )
{
    if (!m_Grid->IsSelection()) {
        wxMessageBox( wxT("Nothing selected!"), wxT("Error"), wxOK | wxICON_ERROR, this);
        return;
    }
    const wxArrayInt& rows = m_Grid->GetSelectedRows();
    size_t num_rows = rows.size();
    if (num_rows > 0) {
        wxMessageBox( wxT("Selection is too complex"), wxT("Error"), wxOK | wxICON_ERROR, this);
        return;
    }

    const wxGridCellCoordsArray& selected_blocks = m_Grid->GetSelectionBlockTopLeft();
    size_t num_blocks = selected_blocks.size();

    const wxArrayInt& cols = m_Grid->GetSelectedCols();
    size_t num_cols = cols.size();

    if (num_blocks == 0 && num_cols == 0) {
        wxMessageBox( wxT("Nothing selected!"), wxT("Error"), wxOK | wxICON_ERROR, this);
        return;
    } else if (num_blocks > 1 
               || (num_blocks > 0 && num_cols > 0)) {
        wxMessageBox( wxT("Selection is too complex"), wxT("Error"), wxOK | wxICON_ERROR, this);
        return;
    }
    const wxGridCellCoordsArray& selected_block_right = m_Grid->GetSelectionBlockBottomRight();
    if (num_blocks != selected_block_right.size()) {
        wxMessageBox( wxT("Selection is too complex"), wxT("Error"), wxOK | wxICON_ERROR, this);
        return;
    }

    CSeqTableGrid *gridAdapter = dynamic_cast<CSeqTableGrid*>(m_Grid->GetTable());
    if( !gridAdapter ) {
        return;
    }
    CRef<objects::CSeq_table> values_table = gridAdapter->GetValuesTable();

    // populate copy_data with ID values
    CRef<objects::CSeq_table> copy_data (new objects::CSeq_table());

    int top_row = 0;
    int bot_row = values_table->GetNum_rows() - 1;
    num_rows = values_table->GetNum_rows();
    if (num_blocks == 1) {
        top_row = selected_blocks[0].GetRow();
        bot_row = selected_block_right[0].GetRow();
        num_rows = bot_row - top_row + 1;
    }

    CConstRef<objects::CSeqTable_column> id_column = values_table->GetColumns()[0];
    CRef<objects::CSeqTable_column> new_column(new objects::CSeqTable_column());
    new_column->SetHeader().SetTitle("Copied ID");

    for (int j = top_row; j <= bot_row; j++) {
        string label = "";
        id_column->GetData().GetId()[j]->GetLabel (&label, objects::CSeq_id::eContent);
        new_column->SetData().SetString().push_back(label);
    }
    copy_data->SetColumns().push_back(new_column);
    copy_data->SetNum_rows(num_rows);

    x_PasteCells (copy_data);
    m_MenuCol = -1;
}


/*!
 * wxEVT_GRID_CELL_RIGHT_CLICK event handler for ID_GRID
 */

void CSeqTableGridPanel::OnCellRightClick( wxGridEvent& event )
{
    x_CreateMenu(event);
}

void CSeqTableGridPanel::OnCellLeftClick( wxGridEvent& event )
{
    int i = event.GetRow();
    int j = event.GetCol();
    if (i >=0 && j >=0 )
    {
        unordered_map<int, int>::iterator it = m_CollapseCell.find(i);
        if (j == m_CollapseGlyphCol && m_CollapseCol >= 0 && it != m_CollapseCell.end())
        {
            it->second = -it->second;
            x_CollapseTableByColumn();
            return;
        }
        else
        {
            m_Grid->SetGridCursor( i,j );
            if  (m_Grid->CanEnableCellControl())
            {
		m_edit_cell_row = i;
		m_edit_cell_col = j;
                m_Grid->EnableCellEditControl();
		wxGridCellEditor* editor = m_Grid->GetCellEditor(i,j);
		// https://wiki.wxwidgets.org/EventTypes_and_Event-Table_Macros
		editor->GetControl()->Connect(wxID_ANY, wxEVT_LEFT_DCLICK,  wxMouseEventHandler(CSeqTableGridPanel::OnCellDoubleLeftClick2), NULL, this);
		editor->DecRef();
                m_Modified = true;
            }
        }
    }
    CallAfter(&CSeqTableGridPanel::x_UpdateCountSelectedDisplay);
//    event.Skip(); 
}

void CSeqTableGridPanel::OnCellDoubleLeftClick( wxGridEvent& event )
{
    int i = event.GetRow();
    int j = event.GetCol();
    if (i >=0 && j >=0 )
    {
        unordered_map<int, int>::iterator it = m_CollapseCell.find(i);
        if (j == m_CollapseGlyphCol && m_CollapseCol >= 0 && it != m_CollapseCell.end())
        {
            return;
        }
        else
        {
            m_Grid->SetGridCursor( i,j );
            if  (m_Grid->CanEnableCellControl())
            {
		wxString old_value = m_Grid->GetCellValue(i,j);
		CSimpleTextEditor dlg(this, old_value);
		if (dlg.ShowModal() == wxID_OK)
		{
		    wxString new_value = dlg.GetValue();
		    if (new_value != old_value)
		    {
			string val = new_value.ToStdString();
			NStr::ReplaceInPlace(val, "\r", "");
			NStr::ReplaceInPlace(val, "\n", " ");
			NStr::TruncateSpacesInPlace(val);		   
			m_Grid->SetCellValue(i,j, wxString(val));
			m_Modified = true;
		    }
		}
            }
        }
    }
}

void CSeqTableGridPanel::OnCellDoubleLeftClick2( wxMouseEvent& event )
{
    if (m_edit_cell_row < 0 || m_edit_cell_col < 0)
	return;
    wxObject *obj = event.GetEventObject();
    wxTextCtrl *ctrl = dynamic_cast<wxTextCtrl*>(obj);
    if (!ctrl)
	return;
    wxString old_value = ctrl->GetValue();
    CSimpleTextEditor dlg(this, old_value);
    if (dlg.ShowModal() == wxID_OK)
    {
	wxString new_value = dlg.GetValue();
	if (new_value != old_value)
	{
	    string val = new_value.ToStdString();
	    NStr::ReplaceInPlace(val, "\r", "");
	    NStr::ReplaceInPlace(val, "\n", " ");
	    NStr::TruncateSpacesInPlace(val);	
	    m_Grid->SetCellValue(m_edit_cell_row, m_edit_cell_col, wxString(val));	   
	    m_Modified = true;
	}
    }
}


/*!
 * wxEVT_GRID_LABEL_RIGHT_CLICK event handler for ID_GRID
 */

void CSeqTableGridPanel::OnLabelRightClick( wxGridEvent& event )
{
    x_CreateMenu(event);
}

void CSeqTableGridPanel::x_SortByRowLabel()
{
    CSeqTableGrid *gridAdapter = dynamic_cast<CSeqTableGrid*>(m_Grid->GetTable());
    if( !gridAdapter ) {
        return;
    }
    CRef<objects::CSeq_table> values_table = gridAdapter->GetValuesTable();
    CRef<objects::CSeq_table> old_table(new CSeq_table);
    old_table->Assign(*values_table);
    

    int num_rows = gridAdapter->GetNumberRows();
    int num_cols = values_table->GetColumns().size();
    vector<vector<bool> > selection(num_rows, vector<bool>(num_cols,false));
    for (int i=0; i<num_rows; i++)
        for (int j=0; j<num_cols; j++)
            if (m_Grid->IsInSelection(i,j))
                selection[i][j] = true;


    vector < pair< pair<string,int>,int> > sorted;
    for (int i=0; i<num_rows; i++)
    {
        string s = ToStdString(gridAdapter->GetRowLabelValue(i));
        pair<string,int> p("",m_MapRowLabelToIndex[s]);
        sorted.push_back(pair< pair<string,int>,int>(p,i));
    }
    std::sort(sorted.begin(),sorted.end(),cmpTableValues); 
   
    if (!m_SortByRowLabelAscend)
        std::reverse(sorted.begin(),sorted.end());
    m_Grid->ClearSelection();

    for (int j=0; j<num_cols; j++)
    {
        switch (values_table->GetColumns()[j]->GetData().Which())
        {
        case CSeqTable_multi_data::e_Id:
            values_table->SetColumns()[j]->SetData().SetId().resize(num_rows);
            break;
        case CSeqTable_multi_data::e_String:
            values_table->SetColumns()[j]->SetData().SetString().resize(num_rows);
            break;
        default:
            break;
        }
    }

    for (int i=0; i<num_rows; i++)
    {
        int old_i = sorted[i].second;
        for (int j=0; j<num_cols; j++)
        {
            switch (values_table->GetColumns()[j]->GetData().Which())
            {
                case CSeqTable_multi_data::e_Id:
                    if (old_i < old_table->SetColumns()[j]->GetData().GetId().size())
                        values_table->SetColumns()[j]->SetData().SetId()[i] = old_table->SetColumns()[j]->GetData().GetId()[old_i];                    
                    break;
                case CSeqTable_multi_data::e_String:
                    if (old_i < old_table->SetColumns()[j]->GetData().GetString().size())
                        values_table->SetColumns()[j]->SetData().SetString()[i] = old_table->SetColumns()[j]->GetData().GetString()[old_i];
                    else
                        values_table->SetColumns()[j]->SetData().SetString()[i].clear();
                    break;
            default:
                break;
            }
            if (selection[old_i][j])
                m_Grid->SelectBlock(i,j,i,j,true);
        }
    }
    m_SortByRowLabelAscend = !m_SortByRowLabelAscend;
    m_Grid->Refresh();
}

void CSeqTableGridPanel::OnLabelLeftClick( wxGridEvent& event )
{
    int row = event.GetRow();
    int col = event.GetCol();
    if ( col == m_CollapseGlyphCol && m_CollapseGlyphCol >= 0)
        return;

    if (col == -1 && row == -1 && m_SortByRowLabel)
    {
        x_ExpandTable();
        x_SortByRowLabel();
    }
    else if (col == -1 && row == -1)
        m_SortByRowLabel = true;
    if (col == -1 || row == -1)
        m_Grid->DisableCellEditControl();
    else
        m_Grid->EnableCellEditControl();

    if ( col == -1 &&  row >= 0)
    {
        x_SelectHiddenRows(row);
        return;
    }
    CallAfter(&CSeqTableGridPanel::x_UpdateCountSelectedDisplay);
    event.Skip();
}

int CSeqTableGridPanel::GetRowIndex(int row)
{
    int index = -1;
    CSeqTableGrid *gridAdapter = dynamic_cast<CSeqTableGrid*>(m_Grid->GetTable());
    if (gridAdapter)
    {
        string s = ToStdString(gridAdapter->GetRowLabelValue(row));
        index = m_MapRowLabelToIndex[s];
    }
    return index;
}

void CSeqTableGridPanel::OnLabelLeftDClick( wxGridEvent& event )
{
    if (event.GetCol() == m_CollapseGlyphCol && m_CollapseGlyphCol >= 0)
        return;

    if (event.GetCol() == -1 && event.GetRow() == -1)  
    {
        x_ExpandTable();
        m_SortByRowLabel = true;
        x_SortByRowLabel();
    }
    if (event.GetCol() == -1 && event.GetRow() != -1)
    {
        int row = event.GetRow();
        CSeqTableGrid *gridAdapter = dynamic_cast<CSeqTableGrid*>(m_Grid->GetTable());
        if (gridAdapter)
        {
            string s = ToStdString(gridAdapter->GetRowLabelValue(row));
            int index = m_MapRowLabelToIndex[s];
            CSeqGridTableNav* dlg = x_GetSeqGridTableNav();
            if (dlg)
                dlg->JumpToTextView(index);
        }
    }
    if (event.GetCol() == -1 || event.GetRow() == -1)
        m_Grid->DisableCellEditControl();
    else
        m_Grid->EnableCellEditControl();
    CallAfter(&CSeqTableGridPanel::x_UpdateCountSelectedDisplay);
    event.Skip();
}

CSeqGridTableNav* CSeqTableGridPanel::x_GetSeqGridTableNav()
{
    wxWindow* w = this->GetParent();
    while (w != NULL) {
        CSeqGridTableNav* parent = dynamic_cast<CSeqGridTableNav*>(w);
        if (parent) {
            return parent;
        } else {
            w = w->GetParent();
        }
    }
    return NULL;
}  


void CSeqTableGridPanel::OnSearchTable( wxCommandEvent& event )
{
    CSeqTableGrid *gridAdapter = dynamic_cast<CSeqTableGrid*>(m_Grid->GetTable());
    if( !gridAdapter ) return;
    
    int num_rows = gridAdapter->GetNumberRows();

    wxTextEntryDialog dlg(this,wxT("Enter the sequences you would like to select separated by space"), wxT("Enter Sequence IDs"), m_FindStr, wxOK|wxCANCEL);
    if (dlg.ShowModal() == wxID_OK)
    {
        m_FindStr = dlg.GetValue();
        if (m_FindStr.empty()) return;
        wxStringTokenizer tkz(m_FindStr);
        m_Grid->ClearSelection();
        while ( tkz.HasMoreTokens() )
        {
            wxString token = tkz.GetNextToken();
            if (!token.empty())
                for (int i=0; i<num_rows; i++)
                    if (token == gridAdapter->GetRowLabelValue(i))
                        m_Grid->SelectRow(i,true);
        }
    }
    m_MenuCol = -1;
}



void  CSeqTableGridPanel::CopyPasteDataByKeyboard(wxKeyEvent& event) // See http://forums.wxwidgets.org/viewtopic.php?f=20&t=2200
{
    if ((event.GetKeyCode() == 'C') && (event.ControlDown() == true)) 
    {
        x_OnCopyCells();
    }
    else if ((event.GetKeyCode() == 'V') && (event.ControlDown() == true)) 
    {
        x_OnPasteCells();
    }
    event.Skip();
}

void CSeqTableGridPanel::x_UpdateCountSelectedDisplay()
{
    const wxArrayInt& rows = m_Grid->GetSelectedRows();
    size_t num_rows = rows.size();
    if (num_rows == 0)
    {
        const wxGridCellCoordsArray& selected_blocks_left = m_Grid->GetSelectionBlockTopLeft();
        const wxGridCellCoordsArray& selected_blocks_right = m_Grid->GetSelectionBlockBottomRight();
        if (selected_blocks_left.size() == selected_blocks_right.size())
            for (size_t i = 0; i < selected_blocks_left.size(); i++)
            {
                int top_row = selected_blocks_left[i].GetRow();
                int left_col = selected_blocks_left[i].GetCol();
                int bot_row = selected_blocks_right[i].GetRow();
                int right_col = selected_blocks_right[i].GetCol();
                int num_cols = right_col - left_col + 1;
                if (num_cols == m_Grid->GetNumberCols())
                    num_rows += bot_row - top_row + 1;
            }
    }
    CStringConstraintSelect* select_win = NULL;
    wxWindow *w = this->GetParent();
    while (w != NULL) 
    {
        wxWindowList& wlist = w->GetChildren();
        for ( wxWindowList::Node *node = wlist.GetFirst(); node; node = node->GetNext() )
        {
            wxWindow *current = (wxWindow *)node->GetData();
            select_win = dynamic_cast<CStringConstraintSelect*>(current);
            if (select_win)
                break;
        }
        if (select_win)
            break;
        w = w->GetParent();
    }
    if (select_win)
        select_win->UpdateCountSelectedDisplay(num_rows);
}  

void CSeqTableGridPanel::x_SelectHiddenRows(int row)
{
    if (!m_Grid->IsRowShown(row))
        return;
    unsigned int num_rows =  m_Grid->GetNumberRows(); 
    vector<bool> selected(num_rows, false);
    for (size_t i = 0; i < num_rows; i++)
        if (m_Grid->IsInSelection(i, m_CollapseGlyphCol))
            selected[i] = true;
    
    m_Grid->ClearSelection();
    
    bool selected_before = selected[row];
    selected[row] = !selected_before;

    size_t i = row + 1;
    while (i < num_rows && !m_Grid->IsRowShown(i))
    {
        selected[i] = !selected_before;
        i++;
    }

    for (size_t i = 0; i < num_rows; i++)
        if (selected[i])
            m_Grid->SelectRow(i, true);
    
    x_UpdateCountSelectedDisplay();
}

END_NCBI_SCOPE


