/*  $Id: windows_dlg.cpp 27376 2013-02-04 19:22:26Z voronov $
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
 * Authors:  Andrey Yazhuk
 */


#include <ncbi_pch.hpp>

////@begin includes
////@end includes

#include <gui/widgets/wx/windows_dlg.hpp>
#include <gui/widgets/wx/table_listctrl.hpp>
#include <gui/widgets/wx/wm_client.hpp>
#include <gui/widgets/wx/window_manager.hpp>


#include <wx/button.h>


BEGIN_NCBI_SCOPE

IMPLEMENT_DYNAMIC_CLASS( CWindowsDlg, CDialog )

BEGIN_EVENT_TABLE( CWindowsDlg, CDialog )

////@begin CWindowsDlg event table entries
    EVT_LIST_ITEM_SELECTED( ID_TABLE, CWindowsDlg::OnTableSelected )
    EVT_LIST_ITEM_DESELECTED( ID_TABLE, CWindowsDlg::OnTableDeselected )

    EVT_BUTTON( ID_ACTIVATE, CWindowsDlg::OnActivateClick )

    EVT_BUTTON( ID_FLOAT, CWindowsDlg::OnFloatClick )

    EVT_BUTTON( ID_RECOVER, CWindowsDlg::OnRecoverClick )

    EVT_BUTTON( ID_MINIMIZE, CWindowsDlg::OnMinimizeClick )

    EVT_BUTTON( ID_RESTORE, CWindowsDlg::OnRestoreClick )

    EVT_BUTTON( wxID_CLOSE, CWindowsDlg::OnCloseClick )

////@end CWindowsDlg event table entries

END_EVENT_TABLE()


CWindowsDlg::CWindowsDlg()
{
    Init();
}


CWindowsDlg::CWindowsDlg( wxWindow* parent, wxWindowID id, const wxString& caption, const wxPoint& pos, const wxSize& size, long style )
{
    Init();
    Create(parent, id, caption, pos, size, style);
}


/*!
 * CWindowsDlg creator
 */

bool CWindowsDlg::Create( wxWindow* parent, wxWindowID id, const wxString& caption, const wxPoint& pos, const wxSize& size, long style )
{
////@begin CWindowsDlg creation
    SetExtraStyle(wxWS_EX_BLOCK_EVENTS);
    CDialog::Create( parent, id, caption, pos, size, style );

    CreateControls();
    if (GetSizer())
    {
        GetSizer()->SetSizeHints(this);
    }
    Centre();
////@end CWindowsDlg creation
    return true;
}


CWindowsDlg::~CWindowsDlg()
{
////@begin CWindowsDlg destruction
////@end CWindowsDlg destruction

    m_Table->RemoveModel();
}


void CWindowsDlg::Init()
{
////@begin CWindowsDlg member initialisation
    m_Table = NULL;
    m_FloatBtn = NULL;
    m_RecoverBtn = NULL;
    m_MinimizeBtn = NULL;
    m_RestoreBtn = NULL;
    m_CloseBtn = NULL;
////@end CWindowsDlg member initialisation

    m_WindowManager = NULL;
}


void CWindowsDlg::CreateControls()
{
////@begin CWindowsDlg content construction
    CWindowsDlg* itemCDialog1 = this;

    wxBoxSizer* itemBoxSizer2 = new wxBoxSizer(wxHORIZONTAL);
    itemCDialog1->SetSizer(itemBoxSizer2);

    m_Table = new CwxTableListCtrl( itemCDialog1, ID_TABLE, wxDefaultPosition, itemCDialog1->ConvertDialogToPixels(wxSize(180, 100)), wxLC_REPORT|wxLC_VIRTUAL );
    itemBoxSizer2->Add(m_Table, 1, wxGROW|wxALL, 5);

    wxBoxSizer* itemBoxSizer4 = new wxBoxSizer(wxVERTICAL);
    itemBoxSizer2->Add(itemBoxSizer4, 0, wxGROW|wxALL, 5);

    wxButton* itemButton5 = new wxButton( itemCDialog1, ID_ACTIVATE, _("&Activate"), wxDefaultPosition, wxDefaultSize, 0 );
    itemButton5->SetHelpText(_("Make window visible and focused"));
    if (CWindowsDlg::ShowToolTips())
        itemButton5->SetToolTip(_("Make window visible and focused"));
    itemBoxSizer4->Add(itemButton5, 0, wxGROW|wxALL, 5);

    m_FloatBtn = new wxButton( itemCDialog1, ID_FLOAT, _("&Float"), wxDefaultPosition, wxDefaultSize, 0 );
    m_FloatBtn->SetHelpText(_("Move to a floating window"));
    if (CWindowsDlg::ShowToolTips())
        m_FloatBtn->SetToolTip(_("Move to a floating window"));
    itemBoxSizer4->Add(m_FloatBtn, 0, wxGROW|wxALL, 5);

    m_RecoverBtn = new wxButton( itemCDialog1, ID_RECOVER, _("&Reset"), wxDefaultPosition, wxDefaultSize, 0 );
    m_RecoverBtn->SetHelpText(_("Move window into the central tabbed pane"));
    if (CWindowsDlg::ShowToolTips())
        m_RecoverBtn->SetToolTip(_("Move window into the central tabbed pane"));
    itemBoxSizer4->Add(m_RecoverBtn, 0, wxGROW|wxALL, 5);

    m_MinimizeBtn = new wxButton( itemCDialog1, ID_MINIMIZE, _("&Minimize"), wxDefaultPosition, wxDefaultSize, 0 );
    m_MinimizeBtn->SetHelpText(_("Move window into the central tabbed pane"));
    if (CWindowsDlg::ShowToolTips())
        m_MinimizeBtn->SetToolTip(_("Move window into the central tabbed pane"));
    itemBoxSizer4->Add(m_MinimizeBtn, 0, wxGROW|wxALL, 5);

    m_RestoreBtn = new wxButton( itemCDialog1, ID_RESTORE, _("&Restore"), wxDefaultPosition, wxDefaultSize, 0 );
    m_RestoreBtn->SetHelpText(_("Move window into the central tabbed pane"));
    if (CWindowsDlg::ShowToolTips())
        m_RestoreBtn->SetToolTip(_("Move window into the central tabbed pane"));
    itemBoxSizer4->Add(m_RestoreBtn, 0, wxGROW|wxALL, 5);

    m_CloseBtn = new wxButton( itemCDialog1, wxID_CLOSE, _("&Close"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer4->Add(m_CloseBtn, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

    itemBoxSizer4->Add(7, 32, 1, wxGROW|wxALL, 5);

    wxButton* itemButton12 = new wxButton( itemCDialog1, wxID_OK, _("OK"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer4->Add(itemButton12, 0, wxGROW|wxALL, 5);

////@end CWindowsDlg content construction

    {{
        vector<IWMClient*> clients;
        m_WindowManager->GetAllClients( clients );
        x_UpdateTable(clients);
    }}

    m_Table->SetModel( &m_Model );
    wxRect rc = m_Table->GetRect();
    m_Table->SetColumnWidth(0, rc.width - 4);

    /// select active window
    IWMClient* client = m_WindowManager->GetActiveClient();
    vector<IWMClient*> clients(1, client);
    x_SelectClients(clients);

    x_UpdateButtons();

    m_Table->SetFocus();
}


bool CWindowsDlg::ShowToolTips()
{
    return true;
}


wxBitmap CWindowsDlg::GetBitmapResource( const wxString& name )
{
    // Bitmap retrieval
////@begin CWindowsDlg bitmap retrieval
    wxUnusedVar(name);
    return wxNullBitmap;
////@end CWindowsDlg bitmap retrieval
}


wxIcon CWindowsDlg::GetIconResource( const wxString& name )
{
    // Icon retrieval
////@begin CWindowsDlg icon retrieval
    wxUnusedVar(name);
    return wxNullIcon;
////@end CWindowsDlg icon retrieval
}


void CWindowsDlg::SetWindowManager( CWindowManager* manager )
{
    m_WindowManager = manager;
}


void CWindowsDlg::x_UpdateTable(const vector<IWMClient*>& clients)
{
    vector<wxString> names;
    names.push_back( wxT("Window Title") );

    _ASSERT(m_WindowManager);

    size_t n_rows = clients.size();
    m_Model.Init( names, (int)n_rows );

    for( size_t i = 0;  i < n_rows;  i++ )  {
        IWMClient* cl = clients[i];

        m_Model.SetIcon( (int)i, wxString::FromUTF8( cl->GetIconAlias().c_str() ) );
        wxString value = wxString::FromUTF8( cl->GetClientLabel().c_str() );
        m_Model.SetStringValueAt( (int)i, 0, value );
        m_Model.SetAttachment( (int)i, cl );
    }
}


void CWindowsDlg::OnActivateClick( wxCommandEvent& event )
{
    vector<IWMClient*> clients;
    x_GetSelectedClients(clients);
    m_WindowManager->ActivateClients(clients);

    Raise();
}


void CWindowsDlg::OnFloatClick( wxCommandEvent& event )
{
    vector<IWMClient*> clients;
    x_GetSelectedClients(clients);

    for(  size_t i = 0;  i < clients.size();  i++ ) {
        IWMClient* client = clients[i];
        if( ! m_WindowManager->IsFloating(*client))   {
            m_WindowManager->MoveToFloatingFrame(*client);
        }
    }

    x_UpdateButtons();
    Raise();
}


void CWindowsDlg::OnRecoverClick( wxCommandEvent& event )
{
    vector<IWMClient*> clients;
    x_GetSelectedClients(clients);

    for(  size_t i = 0;  i < clients.size();  i++ ) {
        IWMClient* client = clients[i];
        if( ! m_WindowManager->IsInMainTab(*client))   {
            m_WindowManager->MoveToMainTab(*client);
        }
    }

    x_UpdateButtons();
}


void CWindowsDlg::OnMinimizeClick( wxCommandEvent& event )
{
    vector<IWMClient*> clients;
    x_GetSelectedClients(clients);

    for(  size_t i = 0;  i < clients.size();  i++ ) {
        IWMClient* client = clients[i];
        if(! m_WindowManager->IsMinimized(*client)) {
            m_WindowManager->Minimize(*client);
        }
    }

    x_UpdateButtons();
}


void CWindowsDlg::OnRestoreClick( wxCommandEvent& event )
{
    vector<IWMClient*> clients;
    x_GetSelectedClients( clients );

    for(  size_t i = 0;  i < clients.size();  i++ ){
        IWMClient* client = clients[i];
        if( m_WindowManager->IsMinimized( *client) ){
            m_WindowManager->Restore( *client );
        }
    }

    x_UpdateButtons();
}


void CWindowsDlg::OnCloseClick( wxCommandEvent& event )
{
    vector<IWMClient*> selected;
    x_GetSelectedClients(selected);

    vector<IWMClient*> clients;
    m_WindowManager->GetAllClients( clients );

    for(  size_t i = 0;  i < selected.size();  i++ ) {
        vector<IWMClient*>::iterator it = find(clients.begin(), clients.end(), selected[i]);
        if (it != clients.end()) {
            clients.erase(it);
        }
    }
    x_UpdateTable(clients);
    m_Model.FireDataChanged();

    m_WindowManager->OnCloseClientsRequest(selected);

    // update the list of windows
    /* 
        during setting selections idle processing is activated 
        so TableDeselectedEvent is processed

        m_Table->ClearSelection();
    */

    x_UpdateButtons();
}


void CWindowsDlg::x_GetSelectedClients(vector<IWMClient*>& clients)
{
    vector<int> rows;
    m_Table->GetDataRowsSelected(rows);

    for( size_t i = 0;  i < rows.size();  i++   )   {
        int row = rows[i];
        void* att = m_Model.GetAttachment( row );
        IWMClient* cl = reinterpret_cast<IWMClient*>(att);
        clients.push_back(cl);
    }
}


void CWindowsDlg::x_SelectClients(vector<IWMClient*>& clients)
{
    vector<int> rows;

    int count = m_Table->GetNumRowsVisible();

    for( int i = 0;  i < count;  i++   )   {
        void* att = m_Model.GetAttachment(i);
        IWMClient* cl = reinterpret_cast<IWMClient*>(att);

        vector<IWMClient*>::const_iterator it =
                std::find(clients.begin(), clients.end(), cl);
        if(it != clients.end()) {
            rows.push_back(i);
        }
    }

    m_Table->SetDataRowsSelected(rows);
}


void CWindowsDlg::x_UpdateButtons()
{
    bool en_minimize = false;
    bool en_restore = false;
    bool en_recover = false;
    bool en_float = false;

    vector<IWMClient*> clients;
    x_GetSelectedClients(clients);

    for(  size_t i = 0;  i < clients.size();  i++ ) {
        IWMClient* client = clients[i];
        if(m_WindowManager->IsMinimized(*client))   {
            en_restore = true;
        } else {
            en_minimize = true;
        }
        if( ! m_WindowManager->IsInMainTab(*client))   {
            en_recover = true;
        }
        if( ! m_WindowManager->IsFloating(*client))   {
            en_float = true;
        }
    }

    m_RecoverBtn->Enable(en_recover);
    m_FloatBtn->Enable(en_float);
    m_RestoreBtn->Enable(en_restore);
    m_MinimizeBtn->Enable(en_minimize);
}


void CWindowsDlg::OnTableSelected( wxListEvent& event )
{
    x_UpdateButtons();
}


void CWindowsDlg::OnTableDeselected( wxListEvent& event )
{
    x_UpdateButtons();
}


END_NCBI_SCOPE



