#ifndef GUI_WIDGETS___LOADERS___TABLE_DATA_LIST_CTRL__HPP
#define GUI_WIDGETS___LOADERS___TABLE_DATA_LIST_CTRL__HPP

/*  $Id: table_data_listctrl.hpp 28595 2013-08-05 16:29:38Z katargir $
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
#include <gui/objutils/table_data.hpp>

#include <wx/wx.h>
#include <wx/listctrl.h>
#include <wx/imaglist.h>


BEGIN_NCBI_SCOPE


///////////////////////////////////////////////////////////////////////////////
/// CTableDataListCtrl - 
///
/// This class is a specialized version of wxListCtrl....
///
class NCBI_GUIWIDGETS_LOADERS_EXPORT CTableDataListCtrl
    : public wxListCtrl  
{
public:
    /// ctor
    CTableDataListCtrl() {}

    /// ctor
    CTableDataListCtrl(
        wxWindow* parent,
        wxWindowID id = wxID_ANY,
        const wxPoint& pos = wxDefaultPosition,
        const wxSize& size = wxDefaultSize,
        long style = wxLC_REPORT | wxLC_VIRTUAL,
        const wxValidator& validator = wxDefaultValidator,
        const wxString& name = wxListCtrlNameStr
    );

    /// dtor
    virtual ~CTableDataListCtrl() {}

    /// Set/update data source 
    void InitDataSource(ITableData& table_data);

    /// Returns image ID for column headers
    int GetColumnImageID(int col) const;

protected:
    /// Get the requested field
    wxString OnGetItemText( long row, long col ) const;

    /// Required function if I want to put icons in column headers (but this
    /// function indicates what icon to put in list items (so I return -1)
    virtual int OnGetItemColumnImage(long item, long column) const { return -1; }

    /// Convert strings with tab characters to wxStrings with 4 spaces for each
    /// tab.  Gets around bug/issue in windows where wxListCtrl ignores tabs
    static wxString x_ToWxStringWithTabs(const string& str);

protected:

    /// Data table to be rendered in the list
    CIRef<ITableData> m_TableData;

    /// Holds column icons (shows whether column currently selected or not)
    wxImageList m_ImageList;

    /// Need font width for calculating how many chars fit in a column
    int m_FontWidth;

private:

    DECLARE_DYNAMIC_CLASS(CTableDataListCtrl)
    DECLARE_NO_COPY_CLASS(CTableDataListCtrl)
    DECLARE_EVENT_TABLE()
};

END_NCBI_SCOPE

#endif // GUI_WIDGETS___LOADERS___TABLE_DATA_LIST_CTRL__HPP
