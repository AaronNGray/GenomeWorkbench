/*  $Id: table_import_listctrl.cpp 43787 2019-08-29 20:40:13Z katargir $
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
 * Authors:  Bob Falk
 *
 * File Description:
 *
 */

#include <ncbi_pch.hpp>

#include <gui/widgets/loaders/table_import_listctrl.hpp>
#include <gui/widgets/wx/fileartprov.hpp>

#include <wx/settings.h>

#include <algorithm>


BEGIN_NCBI_SCOPE

static const long kDefStyle = wxLC_REPORT | wxLC_VIRTUAL | wxLC_HRULES | wxLC_VRULES | wxLC_SINGLE_SEL;

CTableImportListCtrl::CTableImportListCtrl(
    wxWindow* parent,
    wxWindowID id,
    const wxPoint& pos,
    const wxSize& size,
    long style,
    const wxValidator& validator,
    const wxString& name)
    : wxListCtrl(
        parent, id, pos, size,
        (style & ~wxLC_MASK_TYPE) | kDefStyle, validator, name)
    , m_ViewType(eMultiColumn)
    , m_FixedWidthUseFields(false)
    , m_ImageList(16, 16, TRUE)
    , m_FontWidth(0)
    , m_IgnoreWidthEvent(false)
    , m_DisplayTypeHeader(false)
{
    // courier new looks bad on mac - light/hard to see.  Fixedsys looks bad
    // on windows - old style computer type with sharp edges.
#ifdef __WXOSX_COCOA__
    SetFont(wxFont(10, wxFONTFAMILY_SWISS, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL,
        false, wxT("Fixedsys")));
    m_FirstRowAttr.SetFont(wxFont(10, wxFONTFAMILY_SWISS, wxFONTSTYLE_ITALIC,
        wxFONTWEIGHT_NORMAL, false, wxT("Fixedsys")));
#else
    SetFont(wxFont(8, wxFONTFAMILY_SWISS, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL,
        false, wxT("Courier New")));
    m_FirstRowAttr.SetFont(wxFont(8, wxFONTFAMILY_SWISS, wxFONTSTYLE_ITALIC,
        wxFONTWEIGHT_NORMAL, false, wxT("Courier New")));
#endif

    // for a brighter blue (stand out more): wxSYS_COLOUR_HIGHLIGHT
    m_FirstRowAttr.SetBackgroundColour( wxSystemSettings::GetColour(wxSYS_COLOUR_MENU));

    // Image list adds some (blank) spacing to the left of the first column,
    // at least on windows even thought I only want images in the column headers.
    // Anyway, I only specify wxLC_ALIGN_LEFT for the list boxes that need those
    // column images, so I know when I need to compensate for them in terms of 
    // column width.
    if (style & wxLC_ALIGN_LEFT) {
        wxFileArtProvider* provider = GetDefaultFileArtProvider();
        provider->RegisterFileAlias(wxT("tablelist_import::checked"), wxT("check.png"));
        provider->RegisterFileAlias(wxT("tablelist_import::skipped"), wxT("track_close.png"));
        provider->RegisterFileAlias(wxT("tablelist_import::id"), wxT("sequence_symbol.png"));        

        SetImageList(&m_ImageList, wxIMAGE_LIST_SMALL);

        wxIcon ico = provider->GetIcon(wxT("tablelist_import::checked"));
        if( ico.IsOk() ){
            m_ImageList.Add(ico);
        }

        ico = provider->GetIcon(wxT("tablelist_import::skipped"));
        if( ico.IsOk() ){
            m_ImageList.Add(ico);
        }

        ico = provider->GetIcon(wxT("tablelist_import::id"));
        if( ico.IsOk() ){
            m_ImageList.Add(ico);
        }
    }
}


// wxWidgets RTTI information
IMPLEMENT_DYNAMIC_CLASS( CTableImportListCtrl, wxListCtrl )

void CTableImportListCtrl::SetDataSource(CRef<CTableImportDataSource> ds)
{
    m_ImportedTableData = ds;

    // Clear current list info
    ClearAll();

    if (m_ImportedTableData.IsNull() || 
        m_ImportedTableData->GetColumns().size() == 0) {
            SetItemCount(0);
    }
    else {

        int num_rows = m_ImportedTableData->GetNumRows();
        // First column is a 1-based row number. Its width is based on the
        // character length of num_rows, so compute that here:
        int first_column_char_width = (int)log10((double)num_rows+1) + 1;

        wxFont f = GetFont();
        wxClientDC dc(this);
        dc.SetFont(f);
        m_FontWidth = dc.GetCharWidth(); 

        // If the view type is eSingleColumn, we display all rows including headers
        // and comments (if there are any).  In eMultiColumn mode, we ignore headers
        // and comments so we set the number of rows accordingly
        if (m_ViewType == eSingleColumn) {
            SetItemCount(m_ImportedTableData->GetNumRows());
        }
        else {
            SetItemCount(m_ImportedTableData->GetNumImportedRows() + 
                (m_DisplayTypeHeader ? 1 : 0));
        }


        first_column_char_width = (first_column_char_width+2) * m_FontWidth;

        // When I have an image list, even if images are not used in 
        // cells, the first cell column seems to make room for it, messing
        // up my cell sizes. So I pass the option wxLC_ALIGN_LEFT only
        // to lists that need images (for the column headers) and add
        // extra pixels to the first column to compensate.
        if (GetImageList(wxIMAGE_LIST_SMALL) != NULL) 
            first_column_char_width += 16;

        if (GetColumnCount() == 0) {
            InsertColumn(0, 
                wxString(wxT("#")), 
                wxLIST_FORMAT_LEFT, 
                first_column_char_width);
        }


        if (m_ImportedTableData->GetTableType() == CTableImportDataSource::eDelimitedTable) {          
            if (m_ImportedTableData->GetDelimiters().size() == 0 ||
                m_ViewType == eSingleColumn) {                                  
                // Get column width in pixels
                int width = std::max(m_ImportedTableData->GetMaxRowLength(), 
                    m_ImportedTableData->GetMaxNonImportedRowLength());

                // Get column width (and add 2 chars to make sure all the
                // characters show)   
                width = (width+2) * m_FontWidth;

                // Add a new column or update existing column (if table already has a 
                // column 1):
                if (GetColumnCount() == 1) {
                    InsertColumn(1, 
                        wxString(ToWxString(m_ImportedTableData->GetColumnName(0))), 
                        wxLIST_FORMAT_LEFT, 
                        width);
                }
                else {
                    wxListItem item;
                    GetColumn(1, item);
                    item.SetWidth(width);
                    item.SetText(ToWxString(m_ImportedTableData->GetColumnName(0)));
                    SetColumn(1, item);
                }
            }
            else {
                // First column is line number - already inserted.
                for (size_t i=1; i<m_ImportedTableData->GetColumns().size(); ++i) {

                    CTableImportColumn c = m_ImportedTableData->GetColumns()[i];                         
                    // Get column width in characters and convert to pixels.
                    int width = m_ImportedTableData->GetColumnWidth(i);

                    // Leave some room for type headers if this list includes them
                    if (m_DisplayTypeHeader)
                        width = std::max(6, width);

                    // Get column width (and add 2 chars to make sure all the
                    // characters show)
                    width = (width+2) * m_FontWidth;                    

                    // Add a new column or update existing column (if table already has a 
                    // column i)
                    if (GetColumnCount() <= (int)i) {
                        InsertColumn(i, 
                            wxString(ToWxString(m_ImportedTableData->GetColumnName(i))),
                            wxLIST_FORMAT_LEFT, 
                            width);
                    }
                    else {
                        wxListItem item;
                        GetColumn(i, item);
                        item.SetWidth(width);                      
                        item.SetText(wxString(m_ImportedTableData->GetColumnName(i)));
                        SetColumn(i, item);
                    }
                }
            }
        }
        else { 
            // Get column width in pixels
            int width = m_ImportedTableData->GetMaxRowLength();

            // Get column width (and add 2 chars to make sure all the
            // characters show)   
            width = (width+2) * m_FontWidth;

            // Synchronize datasource and listcontrol columns.  User
            // may go back and forth to/from fixed width panel so have 
            // to handle different combinations.

            // Default (initial) case: one (non-row-number) col in data:
            if (m_ImportedTableData->GetColumns().size() == 2) {

                m_ImportedTableData->GetColumns()[1].SetWidth(m_ImportedTableData->GetMaxRowLength());

                if (GetColumnCount() == 1) {
                    InsertColumn(1, 
                        wxString(wxString(m_ImportedTableData->GetColumnName(1))), 
                        wxLIST_FORMAT_LEFT, 
                        width);
                }
                else if (GetColumnCount() == 2) {
                    wxListItem item;
                    GetColumn(1, item);
                    item.SetWidth(width);
                    item.SetText(wxString(m_ImportedTableData->GetColumnName(1)));
                    SetColumn(1, item);
                }
                // Delete any columns in listcontrol beyond second column:
                while (GetColumnCount() > 2) 
                    DeleteColumn(GetColumnCount()-1);
            }
            else {
                // Start by getting rid of listcontrol columns:
                while (GetColumnCount() > 1) 
                    DeleteColumn(GetColumnCount()-1);

                int full_width = m_ImportedTableData->GetMaxRowLength();

                // Now add them back using whatever widths are in the data source:
                for (size_t i=1; i< m_ImportedTableData->GetColumns().size(); ++i) {
                    int w = m_ImportedTableData->GetColumns()[i].GetWidth();

                    // Allocate enough space in listctrl for all characters,
                    // padding the last column if needed
                    if (i == m_ImportedTableData->GetColumns().size()-1) {
                        w = full_width;
                        m_ImportedTableData->GetColumns()[i].SetWidth(w);
                    }

                    int char_width = (w+2)*m_FontWidth;
                    InsertColumn(i, 
                        wxString(wxString(m_ImportedTableData->GetColumnName(i))), 
                        wxLIST_FORMAT_LEFT, 
                        char_width);

                    full_width -= w;
                }
            }
        }
    }
    
    // After updating, list may need to add a horizontal scroll bar.  Before
    // adding ScrollList() (in windows) it would not add the horizontal scroll
    // until I resized the window or manually scrolled vertically.  That's why
    // the ScrollList() hack was added.  I also tried SendSizeEvent() but that
    // didn't work.
    ScrollList(1, 1);
    ScrollList(-1, -1);
    
    // Select the first row by default
    if (GetItemCount() > 0)
        SetItemState(0, wxLIST_STATE_SELECTED, wxLIST_STATE_SELECTED);
    Refresh();
}

int CTableImportListCtrl::GetColumnImageID(int col, bool checkbox_only) const
{
    // If a column meets more than one of these (e.g. current and skipped)
    // then the first one takes priority (is returned)
    if (!m_ImportedTableData.IsNull()) {
        if ((size_t)col < m_ImportedTableData->GetColumns().size()) {         
            if (checkbox_only) {
                if (m_ImportedTableData->GetColumns()[col].GetIsCurrent())
                    return 0;
                else 
                    return -1;
            }
            else {
                if (m_ImportedTableData->GetColumns()[col].GetSkipped())
                    return 1;
                else if (m_ImportedTableData->GetColumns()[col].GetType() ==
                         CTableImportColumn::eSeqIdColumn ||  
                         m_ImportedTableData->GetColumns()[col].GetMatchColumn())             
                    return 2;
                else if (m_ImportedTableData->GetColumns()[col].GetIsCurrent())
                    return 0;
            }
        }
    }

    return -1;
}

void CTableImportListCtrl::DivideColumn(int col)
{
    /// Can't divide first column - it is the row number.
    if (col == 0)
        return;

    vector<CTableImportColumn>& data_cols = m_ImportedTableData->GetColumns(); 

    /// Can't divide a column with width <2
    if (data_cols[col].GetWidth() < 2)
        return;

    if (col < GetColumnCount()) {
        // Prevent updates to column data in column resize handler
        m_IgnoreWidthEvent = true;

        wxListItem item;
        GetColumn(col, item);
        int col_width = item.GetWidth();
        item.SetWidth(col_width/2);
        SetColumn(col, item);
        
        // Get width of divided column in characters (from data source)
        int width1 = data_cols[col].GetWidth()/2;
        int width2 = data_cols[col].GetWidth() - width1;

        data_cols[col].SetWidth(width1);

        // Add column to data source        
        CTableImportColumn c;
        c.SetName("Column " + NStr::NumericToString(col));
        c.SetWidth(width2);
        data_cols.insert(data_cols.begin()+col, c);
        m_ImportedTableData->RecomputeHeaders();
        
        // Get width of new and old column in pixels based on
        // previous character length of entire column (always
        // add 2 characters for padding)
        int pixel_width1 = (width1+2) * m_FontWidth;
        int pixel_width2 = (width2+2) * m_FontWidth;

        // update pixel width of current column        
        GetColumn(col, item);       
        item.SetWidth(pixel_width1);
        SetColumn(col, item);

        // Add column to the right of current column in datasource object
        InsertColumn(col,                                         
                     wxString(ToWxString(data_cols[col].GetName())),
                     wxLIST_FORMAT_LEFT,
                     pixel_width2);

        // Update column names
        UpdateColumnNames();

        m_IgnoreWidthEvent = false;
    }
}

void CTableImportListCtrl::MergeColumns(int col)
{
    // Can't merge first column - it is the row number.
    // And if there are 2 columns, then all the data goes
    // in column 2 and it can't be deleted/merged.
    if (col == 0 || GetColumnCount() <= 2)
        return;

    // We merge with column to the right unless its the last column, then
    // we merge with the column to the left (unless there are only 2 cos - the
    // row number and the col we clicked on)
    if (col == GetColumnCount()-1)
        col = col-1;

    if (col < GetColumnCount()-1) {
        // Prevent updates to column data in column resize handler
        m_IgnoreWidthEvent = true;

        // Get first column from listcontrol and its width:
        wxListItem item1;
        GetColumn(col, item1);

        // Set width of merged column based on character (data source)
        // widths of the two columns being merged.
        int merged_width = m_ImportedTableData->GetColumnWidth(col) +
                           m_ImportedTableData->GetColumnWidth(col+1);
        int pixel_width = (merged_width+2)*m_FontWidth;
        item1.SetWidth(pixel_width);
        SetColumn(col, item1);

        // Delete column from list widget
        DeleteColumn(col+1);

        // Delete column info from data source.
        vector<CTableImportColumn>& data_cols = m_ImportedTableData->GetColumns();
        data_cols[col].SetWidth(data_cols[col].GetWidth() + data_cols[col+1].GetWidth());
        data_cols.erase(data_cols.begin() + col + 1);

        UpdateColumnNames();

        m_IgnoreWidthEvent = false;
    }
}

void CTableImportListCtrl::UpdateColumnNames()
{
     vector<CTableImportColumn>& data_cols = m_ImportedTableData->GetColumns();

    // Update column names which have not been updated by the user manually
    // in third (column ID) panel.  This would only be the case if the
    // user went there and came back.
    for (size_t i=1; i<data_cols.size(); ++i) {
        string col_name = data_cols[i].GetName();
        
        // If the column name is in dafault name format: "Col #" then
        // update it to the current number. If it is not, leave it alone.
        vector<string> str_arr;
        NStr::Split(col_name, " ", str_arr);
        if (str_arr.size() == 2 && 
            str_arr[0] == "Col" && 
            NStr::StringToInt(str_arr[1], NStr::fConvErr_NoThrow) != 0) {
                // Update name in data source:
                col_name = "Col " + NStr::NumericToString(i);
                data_cols[i].SetName(col_name);
        } 

        // Update the name in the list control
        wxListItem item;
        GetColumn(i, item);
        item.SetText(ToWxString(col_name));
        SetColumn(i, item);
     }
}

void CTableImportListCtrl::SynchDataSourceColumnWidths()
{
    for (int i=1; i<GetColumnCount(); ++i) {
        wxListItem item;
        GetColumn(i, item);

        // Get number of characters that can fit in the column
        // This is based on number of pixels and then we subtract 2
        // charactes for padding. 
        size_t col_char_width = (size_t)(item.GetWidth()/m_FontWidth);

        //  Don't let col_char_width go below 0.
        col_char_width -= std::min(col_char_width, (size_t)2);

        /// Should have same number if this is called, but double check:
        if ((size_t)i < m_ImportedTableData->GetColumns().size())
            m_ImportedTableData->GetColumns()[(size_t)i].SetWidth(col_char_width);
    }
}

int CTableImportListCtrl::GetColumnsCombinedCharWidth()
{
    int width = 0;

    for (int i=1; i<GetColumnCount(); ++i) {
        wxListItem item;
        GetColumn(i, item);

        // Get number of characters that can fit in the column
        // This is based on number of pixels and then we subtract 2
        // charactes for padding. 
        size_t col_char_width = (size_t)(item.GetWidth()/m_FontWidth);

        col_char_width -= std::min(col_char_width, (size_t)2);
        width += col_char_width;                
    }

    return width;
}

void CTableImportListCtrl::UpdateColumnWidths()
{
    m_ColumnWidths.clear();

    for (size_t i=0; i<(size_t)GetColumnCount(); ++i) {
        wxListItem item;
        GetColumn(i, item);
        m_ColumnWidths.push_back(item.GetWidth());

        // Get number of characters that can fit in the column
        // This is based on number of pixels and then we subtract 2
        // characters for padding. 
        size_t col_char_width = (size_t)(item.GetWidth()/m_FontWidth);
        //  Don't let col_char_width go below 0.
        col_char_width -= std::min(col_char_width, (size_t)2);
        
        // Update column character width in data source (keep the pixel widths
        // in the control synchronized with the character field widths in
        // the table data source)
        m_ImportedTableData->GetColumns()[i].SetWidth(col_char_width);

        //int char_width = m_ImportedTableData->GetColumn(i).GetWidth();
        //_TRACE("Column: " << i << " width: (char, pix): (" << char_width << ", " << item.GetWidth() << ")");
    }
}

void CTableImportListCtrl::UpdateColumnImages(bool checkbox_only)
{
    for (size_t i=0; i<(size_t)GetColumnCount(); ++i) {
        wxListItem item;
        GetColumn(i, item);
  
        int img_idx = GetColumnImageID(i, checkbox_only);

        // Set the image to indicate column status
        item.SetImage(img_idx);
        int width = (m_ImportedTableData->GetColumns()[i].GetWidth()+3) * m_FontWidth;         
        item.SetWidth(width + 24);  // wide enough to also show the image
        item.SetText(m_ImportedTableData->GetColumns()[i].GetName());
        SetColumn(i, item);
    }
}


void CTableImportListCtrl::WriteWidths(const string& prefix)
{
    string widths;

    for (size_t i=0; i<(size_t)GetColumnCount(); ++i) {
        wxListItem item;
        GetColumn(i, item);
        widths += NStr::IntToString(item.GetWidth()) + ", ";
    }

    _TRACE(prefix << " Column Widths: (" << widths <<  ")");
}


vector<int> CTableImportListCtrl::GetColumnWidths()
{ 
    if (m_ColumnWidths.size() != (size_t)GetColumnCount())
        UpdateColumnWidths();

    return m_ColumnWidths;
}

bool CTableImportListCtrl::ColumnWidthsUpdated()
{
    vector<int> current_widths;

    for (size_t i=0; i<(size_t)GetColumnCount(); ++i) {
        wxListItem item;
        GetColumn(i, item);
        current_widths.push_back(item.GetWidth());

        // Get number of characters that can fit in the column
        // This is based on number of pixels and then we subtract 2
        // charactes for padding. 
        size_t col_char_width = (size_t)(item.GetWidth()/m_FontWidth);
        //  Don't let col_char_width go below 0.
        col_char_width -= std::min(col_char_width, (size_t)2);
        
        // Update column character width in data source (keep the pixel widths
        // in the control synchronized with the character field widths in
        // the table data source)
        m_ImportedTableData->GetColumns()[i].SetWidth(col_char_width);
    }

    return !(current_widths == m_ColumnWidths);
}

void CTableImportListCtrl::SetColumnWidthIgnoreEvent(int col, int width)
{    
    if (col >= GetColumnCount())
        return;

    m_IgnoreWidthEvent = true;
    SetColumnWidth(col, width);    
    m_IgnoreWidthEvent = false;
}

wxString CTableImportListCtrl::x_ToWxStringWithTabs(const string& str) 
{
    // On windows, replace all tabs with 4 spaces to fake the tabs.
    // On other platforms wxListCtrl shows tabs so there is no need
    // to do this.
#ifdef __WXMSW__
    wxString tabstr("    ", 1);
    wxString result;
    for (size_t i=0; i<str.size(); ++i) {        
        if (str[i] != '\t') {
            result.append(wxUniChar(str[i]));
        }
        else result.append(tabstr);
    }

    return result;
#else
    return ToWxString(str);
#endif
}


wxString CTableImportListCtrl::OnGetItemText( long row, long col ) const
{    
    if (m_ImportedTableData.IsNull()) {
        return wxT("");
    }
    else {
        // In 'eSingleColum' view mode we show all rows, but in eMultiColumn
        // we ignore header rows and comment rows in the dataset.  This requires
        // that we compute the correct row number in the dataset that corresponds
        // to 'row' in the list box (the dataset row will be same or larger).
        int adjusted_row_num = row;
        if (m_ViewType != eSingleColumn) {         
            adjusted_row_num = row + m_ImportedTableData->GetFirstImportRow();
            for (; adjusted_row_num < (int)m_ImportedTableData->GetNumRows() &&
                   m_ImportedTableData->GetRow(adjusted_row_num).GetRowNum() < row;
                ++adjusted_row_num) {}  

            if (m_DisplayTypeHeader)
                --adjusted_row_num;
        }

        // We display a row number (starting at first non-header row)
        int row_num = -1;
        if (adjusted_row_num >= 0)
            row_num = m_ImportedTableData->GetRow(adjusted_row_num).GetRowNum();

        if (col == 0) {           
            if (row_num == -1) {
                return wxT("-");
            }
            else {
                wxString s;
                // 1-based on output
                s << row_num + 1;
                return s;
            }             
        }
        if (m_ViewType == eSingleColumn) {         
            return x_ToWxStringWithTabs(m_ImportedTableData->GetRow(row).GetValue());   
        }
        else {  

            if (m_ImportedTableData->GetTableType() ==
                CTableImportDataSource::eDelimitedTable || m_FixedWidthUseFields) {

                    // Get field values
                    if (!m_DisplayTypeHeader || row > 0) {
                        string field = m_ImportedTableData->GetField(adjusted_row_num, col-1);
                        return x_ToWxStringWithTabs(field);                          
                    }
                    // Display type information for columns instead of field 
                    // value for row 0 (and blank if column is to be skipped)
                    else {
                        if (m_ImportedTableData->GetColumn(col).GetType() == 
                            CTableImportColumn::eSkippedColumn) {
                                return "";
                        }
                        else {
                            return m_ImportedTableData->GetColumn(col).
                                GetShortDataTypeString();
                        }
                    }
            }
            else {
                // Ignore parsing for columns (just fit them in from left to right)
                string str = m_ImportedTableData->GetRow(adjusted_row_num).GetValue();

                size_t col_start_char = 0;
                for (int i=1; i<GetColumnCount() && col_start_char<str.length(); ++i) {
                    wxListItem item;
                    GetColumn(i, item);

                    // Get number of characters that can fit in the column
                    // This is based on number of pixels and then we subtract 2
                    // charactes for padding. 
                    size_t col_char_width = (size_t)(item.GetWidth()/m_FontWidth);
                    
                    //  Don't let col_char_width go below 0.
                    col_char_width -= std::min(col_char_width, (size_t)2);
                
                    if (i == col) {
                        if (i == GetColumnCount() - 1) {
                            string substr = str.substr(col_start_char, 
                                str.size()-col_start_char);
                            return x_ToWxStringWithTabs(substr);
                        }
                        else {
                            string substr = str.substr(col_start_char,
                                std::min(col_char_width, str.size()-col_start_char));
                            return x_ToWxStringWithTabs(substr);
                        }
                    }

                    col_start_char += col_char_width;                   
                }
                return wxT("");
            }
        }
    }
}

wxListItemAttr* CTableImportListCtrl::OnGetItemAttr(long item) const
{

    if (!m_DisplayTypeHeader || item != 0)
        return NULL;   
    else
        return (wxListItemAttr*)&m_FirstRowAttr;
}


END_NCBI_SCOPE

