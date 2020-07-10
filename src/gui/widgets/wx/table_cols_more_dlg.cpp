/*  $Id: table_cols_more_dlg.cpp 25484 2012-03-27 15:03:47Z kuznets $
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
 * Authors:  Roman Katargin
 */


#include <ncbi_pch.hpp>

////@begin includes
#include "wx/imaglist.h"
////@end includes

#include <gui/widgets/wx/table_cols_more_dlg.hpp>

////@begin XPM images
////@end XPM images

BEGIN_NCBI_SCOPE

/*!
 * CMoreTableColsDlg type definition
 */

IMPLEMENT_DYNAMIC_CLASS( CMoreTableColsDlg, CDialog )


/*!
 * CMoreTableColsDlg event table definition
 */

BEGIN_EVENT_TABLE( CMoreTableColsDlg, CDialog )

////@begin CMoreTableColsDlg event table entries
    EVT_LIST_ITEM_SELECTED( ID_COLTABLE, CMoreTableColsDlg::OnColTableSelected )
    EVT_LIST_ITEM_DESELECTED( ID_COLTABLE, CMoreTableColsDlg::OnColTableDeselected )
    EVT_LIST_ITEM_FOCUSED( ID_COLTABLE, CMoreTableColsDlg::OnColTableRowFocused )
    EVT_LIST_KEY_DOWN( ID_COLTABLE, CMoreTableColsDlg::OnColTableKeyDown )
    EVT_LIST_COL_RIGHT_CLICK( ID_COLTABLE, CMoreTableColsDlg::OnColTableHeaderMenu )

    EVT_BUTTON( ID_SHOW, CMoreTableColsDlg::OnShowClick )

    EVT_BUTTON( ID_HIDE, CMoreTableColsDlg::OnSortingClick )

    EVT_BUTTON( ID_CLEAR_SORT, CMoreTableColsDlg::OnClearSortClick )

////@end CMoreTableColsDlg event table entries

END_EVENT_TABLE()


/*!
 * CMoreTableColsDlg constructors
 */

CMoreTableColsDlg::CMoreTableColsDlg()
{
    Init();
}

CMoreTableColsDlg::CMoreTableColsDlg( wxWindow* parent, wxWindowID id, const wxString& caption, const wxPoint& pos, const wxSize& size, long style )
{
    Init();
    Create(parent, id, caption, pos, size, style);
}


/*!
 * CMoreTableColsDlg creator
 */

bool CMoreTableColsDlg::Create( wxWindow* parent, wxWindowID id, const wxString& caption, const wxPoint& pos, const wxSize& size, long style )
{
    CwxTableListCtrl* parent_table = dynamic_cast<CwxTableListCtrl*>(parent);
    _ASSERT(parent_table);

    SetTable( parent_table );
    x_UpdateModel();

////@begin CMoreTableColsDlg creation
    SetExtraStyle(wxWS_EX_BLOCK_EVENTS);
    CDialog::Create( parent, id, caption, pos, size, style );

    CreateControls();
    if (GetSizer())
    {
        GetSizer()->SetSizeHints(this);
    }
    Centre();
////@end CMoreTableColsDlg creation

    x_UpdateButtons();
    return true;
}


/*!
 * CMoreTableColsDlg destructor
 */

CMoreTableColsDlg::~CMoreTableColsDlg()
{
////@begin CMoreTableColsDlg destruction
////@end CMoreTableColsDlg destruction

    m_ColTable->RemoveModel();
}


/*!
 * Member initialisation
 */

void CMoreTableColsDlg::Init()
{
    static vector<wxString> s_ColNames;
    
    if( s_ColNames.empty() ){
        s_ColNames.push_back( wxT("Column") );
        s_ColNames.push_back( wxT("Sorting") );
        s_ColNames.push_back( wxT("Sort order") );
    }

////@begin CMoreTableColsDlg member initialisation
    m_ColTable = NULL;
    m_ShowBtn = NULL;
    m_SortBtn = NULL;
    m_ClearBtn = NULL;
////@end CMoreTableColsDlg member initialisation

    m_Model.Init( s_ColNames, 0 );
}


/*!
 * Control creation for CMoreTableColsDlg
 */

void CMoreTableColsDlg::CreateControls()
{    
////@begin CMoreTableColsDlg content construction
    CMoreTableColsDlg* itemCDialog1 = this;

    wxBoxSizer* itemBoxSizer2 = new wxBoxSizer(wxHORIZONTAL);
    itemCDialog1->SetSizer(itemBoxSizer2);

    m_ColTable = new CwxTableListCtrl( itemCDialog1, ID_COLTABLE, wxDefaultPosition, wxSize(250, 150), wxLC_REPORT|wxLC_VIRTUAL|wxLC_SINGLE_SEL );
    itemBoxSizer2->Add(m_ColTable, 1, wxGROW|wxALL, 5);

    wxBoxSizer* itemBoxSizer4 = new wxBoxSizer(wxVERTICAL);
    itemBoxSizer2->Add(itemBoxSizer4, 0, wxGROW|wxALL, 5);

    m_ShowBtn = new wxButton( itemCDialog1, ID_SHOW, _("&Show"), wxDefaultPosition, wxDefaultSize, 0 );
    m_ShowBtn->SetHelpText(_("Make column visible"));
    if (CMoreTableColsDlg::ShowToolTips())
        m_ShowBtn->SetToolTip(_("Make column visible"));
    itemBoxSizer4->Add(m_ShowBtn, 0, wxGROW|wxALL, 5);

    m_SortBtn = new wxButton( itemCDialog1, ID_HIDE, _("&Sorting"), wxDefaultPosition, wxDefaultSize, 0 );
    m_SortBtn->SetHelpText(_("Roll sorting state"));
    if (CMoreTableColsDlg::ShowToolTips())
        m_SortBtn->SetToolTip(_("Roll sorting state"));
    itemBoxSizer4->Add(m_SortBtn, 0, wxGROW|wxALL, 5);

    m_ClearBtn = new wxButton( itemCDialog1, ID_CLEAR_SORT, _("&Clear Sort"), wxDefaultPosition, wxDefaultSize, 0 );
    m_ClearBtn->SetHelpText(_("Clear Sort State"));
    if (CMoreTableColsDlg::ShowToolTips())
        m_ClearBtn->SetToolTip(_("Clear Sort State"));
    itemBoxSizer4->Add(m_ClearBtn, 0, wxGROW|wxALL, 5);

    itemBoxSizer4->Add(4, 19, 1, wxGROW|wxALL, 5);

    wxButton* itemButton9 = new wxButton( itemCDialog1, wxID_OK, _("OK"), wxDefaultPosition, wxDefaultSize, 0 );
    itemButton9->SetDefault();
    itemBoxSizer4->Add(itemButton9, 0, wxGROW|wxALL, 5);

    wxButton* itemButton10 = new wxButton( itemCDialog1, wxID_CANCEL, _("Cancel"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer4->Add(itemButton10, 0, wxGROW|wxALL, 5);

////@end CMoreTableColsDlg content construction

    // Connect events and objects
    //! It is out of DialogBlocks because of Table ID Linux issue [YV]
    m_ColTable->Connect( 
        wxID_ANY, wxEVT_LEFT_DOWN, 
        wxMouseEventHandler( CMoreTableColsDlg::OnColTableLeftDown ), 
        NULL, this
    );

    m_ColTable->SetModel( &m_Model );
}

void CMoreTableColsDlg::SetTable( CwxTableListCtrl* table )
{ 
    _ASSERT(table);
    if( !table ) return;

    m_BaseTable = table; 

    int cols_num = m_BaseTable->GetColumnCount();
    m_Model.SetNumRows( cols_num );
    m_Shown.resize( cols_num, false );

    for( int ix = 0; ix < cols_num; ix++ ){
        m_Model.SetStringValueAt( ix, 0, m_BaseTable->GetColumnNameByIndex( ix ) );
        m_Shown[ix] = m_BaseTable->IsColumnVisibleByIndex( ix );
    }

    m_Sorted = m_BaseTable->GetSortedCols();
}


void CMoreTableColsDlg::x_UpdateModel()
{
    for( int ix = 0; ix < m_Model.GetNumRows(); ix++ ){
        m_Model.SetIcon( 
            ix, 
            m_Shown[ix] ? wxT("tlc_checked") : wxT("tlc_unchecked")
        );

        m_Model.SetStringValueAt( ix, 1, wxString() );
        m_Model.SetStringValueAt( ix, 2, wxString() );
    }

    size_t sx = 0;
    for( size_t jx = 0; jx < m_Sorted.size(); jx++ ){

        if( m_Sorted[jx].second == CwxTableListCtrl::eNoSort ){
            continue;

        } else {
            int ix = m_Sorted[jx].first;

            if( m_Sorted[jx].second == CwxTableListCtrl::eAscSort ){
                m_Model.SetStringValueAt( ix, 1, wxT("Ascending") );
            } else {
                m_Model.SetStringValueAt( ix, 1, wxT("Descending") );
            }

            m_Model.SetStringValueAt( ix, 2, wxString() << ++sx );
        }
    }

    m_Model.FireRowsUpdated( 0, m_Model.GetNumRows() -1 );
}

void CMoreTableColsDlg::x_UpdateButtons()
{
    int focus = m_ColTable->GetFocusRow();
    if( focus >= 0 ){
        if( m_Shown[focus] ){
            m_ShowBtn->SetLabel( wxT("&Hide") );
        } else {
            m_ShowBtn->SetLabel( wxT("&Show") );
        }
        m_ShowBtn->Enable();

    } else {
        m_ShowBtn->Disable();
    }

    m_ClearBtn->Enable( !m_Sorted.empty() );
}

/*!
 * wxEVT_COMMAND_LIST_ITEM_SELECTED event handler for ID_COLTABLE
 */

void CMoreTableColsDlg::OnColTableSelected( wxListEvent& event )
{
////@begin wxEVT_COMMAND_LIST_ITEM_SELECTED event handler for ID_COLTABLE in CMoreTableColsDlg.
    // Before editing this code, remove the block markers.
    event.Skip();
////@end wxEVT_COMMAND_LIST_ITEM_SELECTED event handler for ID_COLTABLE in CMoreTableColsDlg. 
}


/*!
 * wxEVT_COMMAND_LIST_ITEM_DESELECTED event handler for ID_COLTABLE
 */

void CMoreTableColsDlg::OnColTableDeselected( wxListEvent& event )
{
////@begin wxEVT_COMMAND_LIST_ITEM_DESELECTED event handler for ID_COLTABLE in CMoreTableColsDlg.
    // Before editing this code, remove the block markers.
    event.Skip();
////@end wxEVT_COMMAND_LIST_ITEM_DESELECTED event handler for ID_COLTABLE in CMoreTableColsDlg. 
}

/*!
 * wxEVT_LEFT_DOWN event handler for ID_COLTABLE
 */

void CMoreTableColsDlg::OnColTableLeftDown( wxMouseEvent& event )
{
    int flags;
    long row = m_ColTable->HitTest( event.GetPosition(), flags );

    if( row >=0 && row < m_ColTable->GetItemCount() ){
        if( flags & wxLIST_HITTEST_ONITEMICON ){
            m_Shown[row] = !m_Shown[row];

            if( !m_Shown[row] ){
                for( size_t ix = 0; ix < m_Sorted.size(); ix++ ){
                    if( m_Sorted[ix].first == row ){
                        m_Sorted[ix].second = CwxTableListCtrl::eNoSort;
                        m_Sorted.erase( m_Sorted.begin() + ix );

                        break;
                    }
                }
            }

        } else if( flags & wxLIST_HITTEST_ONITEMLABEL ){
            bool found = false;
            for( size_t ix = 0; ix < m_Sorted.size(); ix++ ){
                if( m_Sorted[ix].first == row ){
                    found = true;

                    switch( m_Sorted[ix].second ){
                    case CwxTableListCtrl::eAscSort:
                        m_Sorted[ix].second = CwxTableListCtrl::eDescSort;
                        break;

                    case CwxTableListCtrl::eDescSort:
                        m_Sorted[ix].second = CwxTableListCtrl::eNoSort;
                        m_Sorted.erase( m_Sorted.begin() + ix );
                        break;

                    case CwxTableListCtrl::eNoSort:
                        m_Shown[row] = true;
                        m_Sorted[ix].second = CwxTableListCtrl::eAscSort;
                        break;
                    }

                    break;
                }
            }

            if( !found ){
                m_Shown[row] = true;
                m_Sorted.push_back( CwxTableListCtrl::TSortedCol( row, CwxTableListCtrl::eAscSort ) );
            }
        } 

#       ifdef _WX_GENERIC_LISTCTRL_H_
        m_ColTable->Select( row );
#       endif
        x_UpdateModel();
        x_UpdateButtons();
    }

    event.Skip();
}

/*!
 * wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_SHOW
 */

void CMoreTableColsDlg::OnShowClick( wxCommandEvent& event )
{
    int focus = m_ColTable->GetFocusRow();
    if( focus >= 0 ){
        m_Shown[focus] = !m_Shown[focus];

        if( !m_Shown[focus] ){
            for( size_t ix = 0; ix < m_Sorted.size(); ix++ ){
                if( m_Sorted[ix].first == focus ){
                    m_Sorted[ix].second = CwxTableListCtrl::eNoSort;
                    m_Sorted.erase( m_Sorted.begin() + ix );

                    break;
                }
            }
        }

        x_UpdateModel();
        x_UpdateButtons();
    }
}

/*!
 * wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_HIDE
 */

void CMoreTableColsDlg::OnSortingClick( wxCommandEvent& event )
{
    int focus = m_ColTable->GetFocusRow();
    if( focus >= 0 ){
        bool found = false;
        for( size_t ix = 0; ix < m_Sorted.size(); ix++ ){
            if( m_Sorted[ix].first == focus ){
                found = true;

                switch( m_Sorted[ix].second ){
                case CwxTableListCtrl::eAscSort:
                    m_Sorted[ix].second = CwxTableListCtrl::eDescSort;
                    break;

                case CwxTableListCtrl::eDescSort:
                    m_Sorted[ix].second = CwxTableListCtrl::eNoSort;
                    m_Sorted.erase( m_Sorted.begin() + ix );
                    break;

                case CwxTableListCtrl::eNoSort:
                    m_Shown[focus] = true;
                    m_Sorted[ix].second = CwxTableListCtrl::eAscSort;
                    break;
                }

                break;
            }
        }

        if( !found ){
            m_Sorted.push_back( CwxTableListCtrl::TSortedCol( focus, CwxTableListCtrl::eAscSort ) );
        }

        x_UpdateModel();
        x_UpdateButtons();
    }
}

/*!
 * wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_CLEAR_SORT
 */

void CMoreTableColsDlg::OnClearSortClick( wxCommandEvent& event )
{
    m_Sorted.clear();

    x_UpdateModel();
    x_UpdateButtons();
}

/*!
 * wxEVT_COMMAND_LIST_ITEM_FOCUSED event handler for ID_COLTABLE
 */

void CMoreTableColsDlg::OnColTableRowFocused( wxListEvent& event )
{
    x_UpdateButtons();
}

/*!
 * wxEVT_COMMAND_LIST_KEY_DOWN event handler for ID_COLTABLE
 */

void CMoreTableColsDlg::OnColTableKeyDown( wxListEvent& event )
{
    if( event.GetKeyCode() == WXK_SPACE ){
        wxCommandEvent evt;
        OnShowClick( evt );
    }
}

/*!
 * wxEVT_COMMAND_LIST_COL_RIGHT_CLICK event handler for ID_COLTABLE
 */

void CMoreTableColsDlg::OnColTableHeaderMenu( wxListEvent& event )
{
    //! Suppress header menu in this dialog
}

void CMoreTableColsDlg::EndModal( int retCode )
{
    if( retCode == wxID_OK ){
        for( size_t ix = 0; ix < m_Shown.size(); ix++ ){
            if( m_Shown[ix] ){
                m_BaseTable->ShowColumn( (int)ix );
            } else {
                m_BaseTable->HideColumn( (int)ix );
            }
        }

        m_BaseTable->ResetSorting();
        m_BaseTable->SetSortedState( CwxTableListCtrl::eAscSort );

        for( size_t ix = 0; ix < m_Sorted.size(); ix++ ){
            m_BaseTable->AddSorterIndex( 
                m_Sorted[ix].first, m_Sorted[ix].second
            );
        }
        m_BaseTable->SetSorterByColumns();

        m_BaseTable->ApplySorting( false );

        m_BaseTable->Refresh();
    }

    CDialog::EndModal( retCode );
}

/*!
 * Should we show tooltips?
 */

bool CMoreTableColsDlg::ShowToolTips()
{
    return true;
}

/*!
 * Get bitmap resources
 */

wxBitmap CMoreTableColsDlg::GetBitmapResource( const wxString& name )
{
    // Bitmap retrieval
////@begin CMoreTableColsDlg bitmap retrieval
    wxUnusedVar(name);
    return wxNullBitmap;
////@end CMoreTableColsDlg bitmap retrieval
}

/*!
 * Get icon resources
 */

wxIcon CMoreTableColsDlg::GetIconResource( const wxString& name )
{
    // Icon retrieval
////@begin CMoreTableColsDlg icon retrieval
    wxUnusedVar(name);
    return wxNullIcon;
////@end CMoreTableColsDlg icon retrieval
}

END_NCBI_SCOPE


