#ifndef GUI_WIDGETS___LOADERS___TABLE_IMPORT_LIST_CTRL__HPP
#define GUI_WIDGETS___LOADERS___TABLE_IMPORT_LIST_CTRL__HPP

/*  $Id: table_import_listctrl.hpp 30590 2014-06-17 20:11:35Z falkrb $
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
 * Authors:  Bob Falk
 *
 * File Description:
 *
 */

#include <corelib/ncbistd.hpp>

#include <gui/widgets/wx/wx_utils.hpp>
#include <gui/utils/event_handler.hpp>
#include <gui/widgets/loaders/table_import_data_source.hpp>

#include <wx/wx.h>
#include <wx/listctrl.h>
#include <wx/imaglist.h>


BEGIN_NCBI_SCOPE


///////////////////////////////////////////////////////////////////////////////
/// CTableImportCtrl - 
///
/// This class is a specialized version of wxListCtrl designed to show data
/// from a CTableImportDataSource data source.  Records from this data source
/// come from a tabular file (such as a CSV file) and record fields may be 
/// fixed-lenght or delimited by one or more characters.
/// 
/// Fields are retrieved from the table via OnGetItemText(row,col) which
/// gets the row and field data based on the field position info derived
/// from the tables format (fixed-field or character-delimited).
///
class NCBI_GUIWIDGETS_LOADERS_EXPORT CTableImportListCtrl
    : public wxListCtrl  
{
public:
    /// Do we view each record in one column or use all parsed fields?
    enum eDataViewType { eSingleColumn, eMultiColumn };

public:
    /// ctor
    CTableImportListCtrl() : m_ViewType(eMultiColumn) {}

    /// ctor
    CTableImportListCtrl(
        wxWindow* parent,
        wxWindowID id = wxID_ANY,
        const wxPoint& pos = wxDefaultPosition,
        const wxSize& size = wxDefaultSize,
        long style = wxLC_REPORT | wxLC_VIRTUAL,
        const wxValidator& validator = wxDefaultValidator,
        const wxString& name = wxListCtrlNameStr
    );

    /// dtor
    virtual ~CTableImportListCtrl() {}

    /// Set/get view type.  Allows us to render the text in a single column even
    /// after multiple columns are parsed
    void SetViewType( eDataViewType t ) { m_ViewType = t; }
    eDataViewType GetDataViewType() const { return m_ViewType; }

    /// If true, type information is displayed on first row
    void SetDisplayTypeHeader(bool b) { m_DisplayTypeHeader = b; }
    bool GetDisplayTypeHeader() const { return m_DisplayTypeHeader; }

    /// Set/update data source (used to refresh view to match data source
    /// column info)
    void SetDataSource(CRef<CTableImportDataSource> ds);

    /// Returns image ID for column headers
    int GetColumnImageID(int col, bool checkbox_only=false) const;

    /// Creates a new column by dividing the specified column
    void DivideColumn(int col);

    /// Removes a column by merging the specified column with 
    /// the column to its right (or does nothing if it is the last column)
    void MergeColumns(int col);


    /// Update numbered column names to match current number of columns and
    /// sync column names to names in data source that may come from a 
    /// parsed header row
    void UpdateColumnNames();

    /// Set to true for lists which display rows divided into fixed length
    /// fields but do not allow those field layouts to be updated.
    void SetFixedWidthUseFields(bool b) { m_FixedWidthUseFields = b; }
    bool GetFixedWidthUseFields() const { return m_FixedWidthUseFields; }

    /// Synch widths of datasource to updated colums
    void SynchDataSourceColumnWidths();

    /// Update m_ColumnWidths for use when resizing columns (ignores first col)
    void UpdateColumnWidths();

    /// Update images on column headers
    void UpdateColumnImages(bool checkbox_only=false);

    /// Get the most-recently set column widths (ignores first col)
    vector<int> GetColumnWidths();

    /// Returns true if current column widths do not match m_ColumnWidths
    bool ColumnWidthsUpdated();

    void WriteWidths(const string& prefix);

    /// Calls SetColumnWidth after setting m_IgnoreWidthEvent
    void SetColumnWidthIgnoreEvent(int col, int width);

    /// If true, we are setting a width but don't want event handler to
    /// to it
    bool IgnoreWidthEvent() const { return m_IgnoreWidthEvent; }

    /// Return the width of font used in list (used to calculate column widths
    /// in number-of-chars.  Font has to be fixed-width for this to be useful
    int GetFontWidth() const { return m_FontWidth; }
    
    /// Add up the number of characters all the columns together can hold 
    /// (ignores first column)
    int GetColumnsCombinedCharWidth();

protected:
    /// Get the requested field (or full row if m_ViewType == eSingleColumn)
    wxString OnGetItemText( long row, long col ) const;

    /// Get attribute override for a specified row (allows special appearance 
    /// for first row)
    wxListItemAttr* OnGetItemAttr(long item) const;

    /// Required function if I want to put icons in column headers (but this
    /// function indicates what icon to put in list items (so I return -1)
    virtual int OnGetItemColumnImage(long /*item*/, long /*column*/) const { return -1; }

    /// Convert strings with tab characters to wxStrings with 4 spaces for each
    /// tab.  Gets around bug/issue in windows where wxListCtrl ignores tabs
    static wxString x_ToWxStringWithTabs(const string& str);


protected:

    /// Do we show all columns seaprated or show as one field
    eDataViewType m_ViewType;

    /// If fixed-length columns are not being actively adjusted,
    /// this is set to true so that they can be displayed based on
    /// most recent parsing into table records rather than free-flowing
    bool m_FixedWidthUseFields;

    /// Data table to be rendered in the list
    CRef<CTableImportDataSource> m_ImportedTableData;

    /// Holds column icons (shows whether column currently selected or not)
    wxImageList m_ImageList;

    /// Need font width for calculating how many chars fit in a column
    int m_FontWidth;

    /// Widths of columns as updated by UpdateColumnWidths()
    vector<int> m_ColumnWidths;

    /// If true, this flag can be used in column-resize event handlers
    /// to ignore resize events being generated by the list control itself.
    bool m_IgnoreWidthEvent;

    /// First row holds type info
    wxListItemAttr m_FirstRowAttr;

    /// If true, type information will be displayed in first row
    bool m_DisplayTypeHeader;


private:

    DECLARE_DYNAMIC_CLASS(CTableImportListCtrl)
    DECLARE_NO_COPY_CLASS(CTableImportListCtrl)
};

END_NCBI_SCOPE

#endif // GUI_WIDGETS___LOADERS___TABLE_IMPORT_LIST_CTRL__HPP
