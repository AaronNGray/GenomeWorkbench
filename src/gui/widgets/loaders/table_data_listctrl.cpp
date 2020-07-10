/*  $Id: table_data_listctrl.cpp 32112 2015-01-02 14:35:27Z kuznets $
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

#include <gui/widgets/loaders/table_data_listctrl.hpp>
#include <gui/widgets/wx/fileartprov.hpp>

#include <algorithm>

BEGIN_NCBI_SCOPE


static const long kDefStyle = wxLC_REPORT | wxLC_VIRTUAL | wxLC_HRULES | wxLC_VRULES;

CTableDataListCtrl::CTableDataListCtrl(
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
    , m_ImageList(16, 16, TRUE)
    , m_FontWidth(0)
{
    // Image list adds some (blank) spacing to the left of the first column,
    // at least on windows even thought I only want images in the column headers.
    // Anyway, I only specify wxLC_ALIGN_LEFT for the list boxes that need those
    // column images, so I know when I need to compensate for them in terms of 
    // column width.
    if (style & wxLC_ALIGN_LEFT) {
        wxFileArtProvider* provider = GetDefaultFileArtProvider();
        provider->RegisterFileAlias(wxT("tablelist_import::checked"), wxT("check.png"));
        provider->RegisterFileAlias(wxT("tablelist_import::skipped"), wxT("track_close.png"));

        SetImageList(&m_ImageList, wxIMAGE_LIST_SMALL);

        wxIcon ico = provider->GetIcon(wxT("tablelist_import::checked"));
        if( ico.IsOk() ){
            m_ImageList.Add(ico);
        }

        ico = provider->GetIcon(wxT("tablelist_import::skipped"));
        if( ico.IsOk() ){
            m_ImageList.Add(ico);
        }
    }
}


// wxWidgets RTTI information
IMPLEMENT_DYNAMIC_CLASS( CTableDataListCtrl, wxListCtrl )

BEGIN_EVENT_TABLE( CTableDataListCtrl, wxListCtrl )

END_EVENT_TABLE()


void CTableDataListCtrl::InitDataSource(ITableData& table_data)
{
    m_TableData.Reset(&table_data);

    // Clear current list info
    ClearAll();

    if (m_TableData.IsNull()) {
        SetItemCount(0);
    }
    else {

        int num_rows = (int)m_TableData->GetRowsCount();

        // First column is a 1-based row number. Its width is based on the
        // character length of num_rows, so compute that here:
        //int first_column_char_width = (int)log10((double)num_rows+1) + 1;

        wxFont f = GetFont();
        wxClientDC dc(this);
        dc.SetFont(f);
        m_FontWidth = dc.GetCharWidth(); 

        SetItemCount(num_rows);

        // Number of rows to review in order to determine a columns (approx) width
        size_t width_calc_rows = std::min(num_rows, 100);
        string s;

        for (size_t col=0; col<m_TableData->GetColsCount(); ++col) {
            // Get column width in pixels
            int width = 1;
            for (size_t row=0; row<width_calc_rows; ++row) {
                m_TableData->GetStringValue(row, col, s);
                width = std::max(width, (int)s.length());
            }

            // Supporting images messes up size of first column whether image
            // is displayed or not. in any case, col1 nees to be a bit bigger
            if (col == 0) 
                width += 2;
           
            // Get column width (and add 2 chars to make sure all the
            // characters show)
            width = (width+2) * m_FontWidth;

            // Add a new column or update existing column (if table already has a 
            // column col)
            if (GetColumnCount() <= (int)col) {
                InsertColumn(int(col), 
                    wxString(ToWxString(m_TableData->GetColumnLabel(col))),
                    wxLIST_FORMAT_LEFT, 
                    width);
            }
            else {
                wxListItem item;
                GetColumn(int(col), item);
                item.SetWidth(width);                      
                item.SetText(ToWxString(m_TableData->GetColumnLabel(col)));
                SetColumn(int(col), item);
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
    Refresh();
}

int CTableDataListCtrl::GetColumnImageID(int col) const
{
    /*
    if (!m_ImportedTableData.IsNull()) {
        if ((size_t)col < m_ImportedTableData->GetColumns().size()) {
            if (m_ImportedTableData->GetColumns()[col].GetSkipped())
                return 1;
            else if (m_ImportedTableData->GetColumns()[col].GetIsCurrent())
                return 0;
        }
    }
    */

    return -1;
}

wxString CTableDataListCtrl::x_ToWxStringWithTabs(const string& str) 
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


wxString CTableDataListCtrl::OnGetItemText( long row, long col ) const
{    
    if (!m_TableData.IsNull()) {   
        string value;
        m_TableData->GetStringValue(row, col, value);
        return ToWxString(value);
    }

    return wxString("");
}


END_NCBI_SCOPE

