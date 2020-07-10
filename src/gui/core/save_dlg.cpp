/*  $Id: save_dlg.cpp 39528 2017-10-05 15:27:37Z katargir $
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
 * Authors:
 *
 * File Description:
 *
 */

#include <ncbi_pch.hpp>

////@begin includes
////@end includes

#include <gui/core/save_dlg.hpp>

#include <gui/core/document.hpp>
#include <gui/objects/GBWorkspace.hpp>
#include <gui/objects/WorkspaceFolder.hpp>
#include <objects/gbproj/ProjectDescr.hpp>

#include <serial/iterator.hpp>

#include <gui/widgets/wx/wx_utils.hpp>

#include <wx/sizer.h>
#include <wx/checkbox.h>
#include <wx/stattext.h>
#include <wx/statbox.h>
#include <wx/bitmap.h>
#include <wx/icon.h>
#include <wx/checklst.h>
#include <wx/button.h>
#include <wx/wupdlock.h>
#include <wx/msgdlg.h>


BEGIN_NCBI_SCOPE
USING_SCOPE(objects);


SSaveProjectsDlgParams::SSaveProjectsDlgParams()
:   m_ShowNoButton(false)
{
}


IMPLEMENT_DYNAMIC_CLASS( CSaveDlg, CDialog )

BEGIN_EVENT_TABLE( CSaveDlg, CDialog )
////@begin CSaveDlg event table entries
    EVT_INIT_DIALOG( CSaveDlg::OnInitDialog )

    EVT_CHECKLISTBOX( ID_PRJ_LIST, CSaveDlg::OnPrjListToggled )

    EVT_BUTTON( ID_SELECT_CHANGED, CSaveDlg::OnSelectChangedClick )

    EVT_BUTTON( ID_SELECT_ALL, CSaveDlg::OnSelectAllClick )

    EVT_BUTTON( ID_CLEAR_ALL, CSaveDlg::OnClearAllClick )

    EVT_BUTTON( wxID_SAVE, CSaveDlg::OnSaveClick )

    EVT_BUTTON( wxID_NO, CSaveDlg::OnNoClick )

////@end CSaveDlg event table entries
END_EVENT_TABLE()

CSaveDlg::CSaveDlg()
:   m_Wrapped(false), m_WS(0)
{
    Init();
}


CSaveDlg::CSaveDlg( CGBWorkspace& ws, wxWindow* parent, wxWindowID id, const wxString& caption, const wxPoint& pos, const wxSize& size, long style )
    : m_WS(&ws)
{
    Init();
    Create(parent, id, caption, pos, size, style);
}


bool CSaveDlg::Create( wxWindow* parent, wxWindowID id, const wxString& caption, const wxPoint& pos, const wxSize& size, long style )
{
////@begin CSaveDlg creation
    SetExtraStyle(wxWS_EX_BLOCK_EVENTS);
    CDialog::Create( parent, id, caption, pos, size, style );

    CreateControls();
    if (GetSizer())
    {
        GetSizer()->SetSizeHints(this);
    }
    Centre();
////@end CSaveDlg creation
    return true;
}


CSaveDlg::~CSaveDlg()
{
////@begin CSaveDlg destruction
////@end CSaveDlg destruction
}


void CSaveDlg::Init()
{
////@begin CSaveDlg member initialisation
    m_RootSizer = NULL;
    m_DescrText = NULL;
    m_ProjectList = NULL;
    m_BtnSizer = NULL;
    m_SaveBtn = NULL;
////@end CSaveDlg member initialisation
}


const SSaveProjectsDlgParams& CSaveDlg::GetParams() const
{
    return m_Params;
}


void CSaveDlg::SetParams(const SSaveProjectsDlgParams& params)
{
    m_Params = params;
}


void CSaveDlg::CreateControls()
{
////@begin CSaveDlg content construction
    CSaveDlg* itemCDialog1 = this;

    m_RootSizer = new wxBoxSizer(wxVERTICAL);
    itemCDialog1->SetSizer(m_RootSizer);

    wxBoxSizer* itemBoxSizer3 = new wxBoxSizer(wxHORIZONTAL);
    m_RootSizer->Add(itemBoxSizer3, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

    m_DescrText = new wxStaticText( itemCDialog1, wxID_STATIC, wxEmptyString, wxDefaultPosition, wxSize(itemCDialog1->ConvertDialogToPixels(wxSize(200, -1)).x, -1), 0 );
    itemBoxSizer3->Add(m_DescrText, 0, wxGROW|wxALL, 5);

    wxStaticBox* itemStaticBoxSizer5Static = new wxStaticBox(itemCDialog1, wxID_ANY, _("Projects to save:"));
    wxStaticBoxSizer* itemStaticBoxSizer5 = new wxStaticBoxSizer(itemStaticBoxSizer5Static, wxVERTICAL);
    m_RootSizer->Add(itemStaticBoxSizer5, 1, wxGROW|wxALL, 5);

    wxArrayString m_ProjectListStrings;
    m_ProjectList = new wxCheckListBox( itemStaticBoxSizer5->GetStaticBox(), ID_PRJ_LIST, wxDefaultPosition, wxSize(-1, itemStaticBoxSizer5->GetStaticBox()->ConvertDialogToPixels(wxSize(-1, 48)).y), m_ProjectListStrings, wxLB_EXTENDED );
    itemStaticBoxSizer5->Add(m_ProjectList, 1, wxGROW|wxALL, 5);

    wxBoxSizer* itemBoxSizer7 = new wxBoxSizer(wxHORIZONTAL);
    itemStaticBoxSizer5->Add(itemBoxSizer7, 0, wxALIGN_RIGHT, 5);

    wxButton* itemButton8 = new wxButton( itemStaticBoxSizer5->GetStaticBox(), ID_SELECT_CHANGED, _("Select Changed"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer7->Add(itemButton8, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxButton* itemButton9 = new wxButton( itemStaticBoxSizer5->GetStaticBox(), ID_SELECT_ALL, _("Select All"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer7->Add(itemButton9, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxButton* itemButton10 = new wxButton( itemStaticBoxSizer5->GetStaticBox(), ID_CLEAR_ALL, _("Clear All"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer7->Add(itemButton10, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    m_BtnSizer = new wxStdDialogButtonSizer;

    m_RootSizer->Add(m_BtnSizer, 0, wxALIGN_RIGHT|wxALL, 5);
    wxButton* itemButton12 = new wxButton( itemCDialog1, wxID_CANCEL, _("&Cancel"), wxDefaultPosition, wxDefaultSize, 0 );
    m_BtnSizer->AddButton(itemButton12);

    m_SaveBtn = new wxButton( itemCDialog1, wxID_SAVE, _("&Save Selected"), wxDefaultPosition, wxDefaultSize, 0 );
    m_BtnSizer->AddButton(m_SaveBtn);

    wxButton* itemButton14 = new wxButton( itemCDialog1, wxID_NO, _("&No"), wxDefaultPosition, wxDefaultSize, 0 );
    m_BtnSizer->AddButton(itemButton14);

    m_BtnSizer->Realize();

    // Set validators
    m_ProjectList->SetValidator( wxGenericValidator(& m_Params.m_ProjectsToSave) );
////@end CSaveDlg content construction

}


void CSaveDlg::OnInitDialog( wxInitDialogEvent& event )
{
    // one-time initialization of the dialog at start-up

    if(m_Params.m_Description.empty())  {
        m_DescrText->Hide();
    } else {
        m_DescrText->SetLabel(ToWxString(m_Params.m_Description));
    }

    // prepare Project List
    for( size_t i = 0;  i < m_Params.m_ProjectIds.size();  i++ )    {
        int id = m_Params.m_ProjectIds[i];
        CGBDocument* doc = dynamic_cast<CGBDocument*>(m_WS->GetProjectFromId(id));

        string s = doc->GetDescr().GetTitle();
        if(doc->IsDirty())  {
            s += (doc->GetFilename().empty() ?  + "*  (New)" : "*  (Modified)");
        }
        m_ProjectList->Append(ToWxString(s));
    }

    // hide No button, if needed
    if( ! m_Params.m_ShowNoButton)   {
        FindWindow(wxID_NO)->Hide();
    }

    x_UpdateButtons();

    wxDialog::OnInitDialog(event);

    int w = m_DescrText->ConvertDialogToPixels(wxPoint(200, 0)).x;
    m_DescrText->Wrap(w);

    m_RootSizer->Layout();
}


void CSaveDlg::OnPrjListToggled( wxCommandEvent& event )
{
    TransferDataFromWindow();
    x_UpdateButtons();
}


// check items for changed (modified or new) projects
void CSaveDlg::OnSelectChangedClick( wxCommandEvent& event )
{
    m_Params.m_ProjectsToSave.Clear();
    for( size_t i = 0;  i < m_Params.m_ProjectIds.size();  i++ )    {
        int id = m_Params.m_ProjectIds[i];
        CGBDocument* doc = dynamic_cast<CGBDocument*>(m_WS->GetProjectFromId(id));

        if(doc->IsDirty())  {
            m_Params.m_ProjectsToSave.Add((int)i);
        }
    }

    TransferDataToWindow();
    x_UpdateButtons();
}


// check items for all projects
void CSaveDlg::OnSelectAllClick( wxCommandEvent& event )
{
    m_Params.m_ProjectsToSave.Clear();
    size_t n = m_Params.m_ProjectIds.size();
    for(  size_t  i = 0; i < n;  i++ ) {
        m_Params.m_ProjectsToSave.Add((int)i);
    }

    TransferDataToWindow();
    x_UpdateButtons();
}


// clear items for all projects
void CSaveDlg::OnClearAllClick( wxCommandEvent& event )
{
    m_Params.m_ProjectsToSave.Clear();
    TransferDataToWindow();
    x_UpdateButtons();
}


void CSaveDlg::OnSaveClick( wxCommandEvent& event )
{
    TransferDataFromWindow();
    EndModal(wxID_SAVE);
}


void CSaveDlg::OnNoClick( wxCommandEvent& event )
{
    EndModal(wxID_NO);
}


bool CSaveDlg::ShowToolTips()
{
    return true;
}


wxBitmap CSaveDlg::GetBitmapResource( const wxString& name )
{
    // Bitmap retrieval
////@begin CSaveDlg bitmap retrieval
    wxUnusedVar(name);
    return wxNullBitmap;
////@end CSaveDlg bitmap retrieval
}


wxIcon CSaveDlg::GetIconResource( const wxString& name )
{
    // Icon retrieval
////@begin CSaveDlg icon retrieval
    wxUnusedVar(name);
    return wxNullIcon;
////@end CSaveDlg icon retrieval
}

void CSaveDlg::x_UpdateButtons()
{
    bool en = !m_Params.m_ProjectsToSave.empty();
    m_SaveBtn->Enable(en);
}

END_NCBI_SCOPE
