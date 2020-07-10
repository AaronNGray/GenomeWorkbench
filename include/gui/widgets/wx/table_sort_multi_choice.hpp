#ifndef GUI_WIDGETS_WX___TABLE_SORT_MULTI_CHOICE__HPP
#define GUI_WIDGETS_WX___TABLE_SORT_MULTI_CHOICE__HPP

/*  $Id: table_sort_multi_choice.hpp 25479 2012-03-27 14:55:33Z kuznets $
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
 * Authors: Yury Voronov
 *
 * File Description:
 *
 */

#include <corelib/ncbistd.hpp>

#include <gui/gui_export.h>

#include <gui/widgets/wx/table_listctrl.hpp>

class wxScrolledWindow;
class wxFlexGridSizer;
class wxHyperlinkEvent;
class wxComboBox;
#include <wx/panel.h>
#include <wx/dnd.h>


BEGIN_NCBI_SCOPE


///////////////////////////////////////////////////////////////////////////////
/// CTableSortMultiChoice
class NCBI_GUIWIDGETS_WX_EXPORT CTableSortMultiChoice 
    : public wxPanel
{
    DECLARE_EVENT_TABLE()

public:
    enum eControlIDs {
        eAddLevelLink = 20000,
        eDeleteAllLevelsLink,
        eLoadBtn,
        eFirstControl
    };


    CTableSortMultiChoice();
    CTableSortMultiChoice(
        wxWindow* parent,
        wxWindowID id = wxID_ANY,
        const wxPoint& pos = wxDefaultPosition,
        const wxSize& size = wxDefaultSize,
        long style = wxTAB_TRAVERSAL,
        const wxString& name = wxT("table_sort_panel")
    );


    virtual ~CTableSortMultiChoice();

    void Init();
    void Create(
        wxWindow* parent,
        wxWindowID id = wxID_ANY,
        const wxPoint& pos = wxDefaultPosition,
        const wxSize& size = wxDefaultSize,
        long style = wxTAB_TRAVERSAL,
        const wxString& name = wxT("table_sort_panel")
    );

    void GetLevels( vector< pair<int,bool> >& levels ) const;
    void SetLevels( const vector< pair<int,bool> >& levels );
    void AddLevels( const vector< pair<int,bool> >& levels );

    void OnLink( wxHyperlinkEvent& event );
    void OnBrowseBtn( wxCommandEvent& event );

protected:
    void x_AddLevelRow( const string& name );
    void x_OnAddLevels();
    void x_AddLevels( const vector<string>& paths );
    void x_DeleteAllLevels();
    void x_DeleteLevel( int link_id );

protected:

    CwxTableListCtrl* m_Table;

    wxScrolledWindow* m_ScrollWnd;
    wxFlexGridSizer* m_LevelSizer;
    vector<CComboBox*> m_Inputs;
    int m_LastChildID;

    vector<string> m_HeaderNames;
};


END_NCBI_SCOPE

/* @} */

#endif  // GUI_WIDGETS_WX___TABLE_SORT_MULTI_CHOICE__HPP
