/*  $Id: task_details_dlg.cpp 25794 2012-05-10 14:53:54Z katargir $
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


#include <gui/framework/task_details_dlg.hpp>

#include <gui/framework/task_view.hpp>

////@begin includes
////@end includes

#include <wx/sizer.h>
#include <wx/button.h>
#include <wx/stattext.h>


BEGIN_NCBI_SCOPE

IMPLEMENT_DYNAMIC_CLASS( CTaskDetailsDlg, CDialog )

BEGIN_EVENT_TABLE( CTaskDetailsDlg, CDialog )

////@begin CTaskDetailsDlg event table entries
    EVT_BUTTON( ID_CANCEL_TASK, CTaskDetailsDlg::OnCancelTaskClick )

////@end CTaskDetailsDlg event table entries

END_EVENT_TABLE()

CTaskDetailsDlg::CTaskDetailsDlg()
{
    Init();
}


CTaskDetailsDlg::CTaskDetailsDlg( wxWindow* parent, wxWindowID id, const wxString& caption, const wxPoint& pos, const wxSize& size, long style )
{
    Init();
    Create(parent, id, caption, pos, size, style);
}


bool CTaskDetailsDlg::Create( wxWindow* parent, wxWindowID id, const wxString& caption, const wxPoint& pos, const wxSize& size, long style )
{
////@begin CTaskDetailsDlg creation
    SetExtraStyle(wxWS_EX_BLOCK_EVENTS);
    CDialog::Create( parent, id, caption, pos, size, style );

    CreateControls();
    if (GetSizer())
    {
        GetSizer()->SetSizeHints(this);
    }
    Centre();
////@end CTaskDetailsDlg creation
    return true;
}


CTaskDetailsDlg::~CTaskDetailsDlg()
{
////@begin CTaskDetailsDlg destruction
////@end CTaskDetailsDlg destruction
}


void CTaskDetailsDlg::Init()
{
////@begin CTaskDetailsDlg member initialisation
////@end CTaskDetailsDlg member initialisation
}


void CTaskDetailsDlg::CreateControls()
{
////@begin CTaskDetailsDlg content construction
    CTaskDetailsDlg* itemCDialog1 = this;

    wxBoxSizer* itemBoxSizer2 = new wxBoxSizer(wxVERTICAL);
    itemCDialog1->SetSizer(itemBoxSizer2);

    wxFlexGridSizer* itemFlexGridSizer3 = new wxFlexGridSizer(0, 2, 0, 0);
    itemBoxSizer2->Add(itemFlexGridSizer3, 1, wxGROW|wxALL, 5);

    wxStaticText* itemStaticText4 = new wxStaticText( itemCDialog1, wxID_STATIC, _("Task:"), wxDefaultPosition, wxDefaultSize, 0 );
    itemFlexGridSizer3->Add(itemStaticText4, 0, wxALIGN_RIGHT|wxALIGN_TOP|wxALL, 5);

    wxTextCtrl* itemTextCtrl5 = new wxTextCtrl( itemCDialog1, ID_TEXTCTRL, wxEmptyString, wxDefaultPosition, wxSize(itemCDialog1->ConvertDialogToPixels(wxSize(200, -1)).x, -1), wxTE_MULTILINE|wxTE_READONLY );
    itemFlexGridSizer3->Add(itemTextCtrl5, 1, wxGROW|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxStaticText* itemStaticText6 = new wxStaticText( itemCDialog1, wxID_STATIC, _("State:"), wxDefaultPosition, wxDefaultSize, 0 );
    itemFlexGridSizer3->Add(itemStaticText6, 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxTextCtrl* itemTextCtrl7 = new wxTextCtrl( itemCDialog1, ID_TEXTCTRL3, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
    itemTextCtrl7->Enable(false);
    itemFlexGridSizer3->Add(itemTextCtrl7, 0, wxGROW|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxStaticText* itemStaticText8 = new wxStaticText( itemCDialog1, wxID_STATIC, _("Status:"), wxDefaultPosition, wxDefaultSize, 0 );
    itemFlexGridSizer3->Add(itemStaticText8, 0, wxALIGN_RIGHT|wxALIGN_TOP|wxALL, 5);

    wxTextCtrl* itemTextCtrl9 = new wxTextCtrl( itemCDialog1, ID_TEXTCTRL2, wxEmptyString, wxDefaultPosition, wxSize(-1, itemCDialog1->ConvertDialogToPixels(wxSize(-1, 40)).y), wxTE_MULTILINE );
    itemTextCtrl9->Enable(false);
    itemFlexGridSizer3->Add(itemTextCtrl9, 1, wxGROW|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxStaticText* itemStaticText10 = new wxStaticText( itemCDialog1, wxID_STATIC, _("Time:"), wxDefaultPosition, wxDefaultSize, 0 );
    itemFlexGridSizer3->Add(itemStaticText10, 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxTextCtrl* itemTextCtrl11 = new wxTextCtrl( itemCDialog1, ID_TEXTCTRL4, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
    itemTextCtrl11->Enable(false);
    itemFlexGridSizer3->Add(itemTextCtrl11, 0, wxGROW|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    itemFlexGridSizer3->AddGrowableRow(0);
    itemFlexGridSizer3->AddGrowableRow(2);
    itemFlexGridSizer3->AddGrowableCol(1);

    wxBoxSizer* itemBoxSizer12 = new wxBoxSizer(wxHORIZONTAL);
    itemBoxSizer2->Add(itemBoxSizer12, 0, wxGROW|wxALL, 5);

    itemBoxSizer12->Add(7, 8, 1, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxButton* itemButton14 = new wxButton( itemCDialog1, ID_CANCEL_TASK, _("Cancel Task"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer12->Add(itemButton14, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxButton* itemButton15 = new wxButton( itemCDialog1, wxID_OK, _("OK"), wxDefaultPosition, wxDefaultSize, 0 );
    itemButton15->SetDefault();
    itemBoxSizer12->Add(itemButton15, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    // Set validators
    itemTextCtrl5->SetValidator( wxTextValidator(wxFILTER_NONE, & m_Title) );
    itemTextCtrl7->SetValidator( wxTextValidator(wxFILTER_NONE, & m_State) );
    itemTextCtrl9->SetValidator( wxTextValidator(wxFILTER_NONE, & m_Status) );
    itemTextCtrl11->SetValidator( wxTextValidator(wxFILTER_NONE, & m_Time) );
////@end CTaskDetailsDlg content construction
}


bool CTaskDetailsDlg::ShowToolTips()
{
    return true;
}


wxBitmap CTaskDetailsDlg::GetBitmapResource( const wxString& name )
{
    // Bitmap retrieval
////@begin CTaskDetailsDlg bitmap retrieval
    wxUnusedVar(name);
    return wxNullBitmap;
////@end CTaskDetailsDlg bitmap retrieval
}


wxIcon CTaskDetailsDlg::GetIconResource( const wxString& name )
{
    // Icon retrieval
////@begin CTaskDetailsDlg icon retrieval
    wxUnusedVar(name);
    return wxNullIcon;
////@end CTaskDetailsDlg icon retrieval
}


void CTaskDetailsDlg::OnCancelTaskClick( wxCommandEvent& WXUNUSED(event) )
{
    wxCommandEvent evt(wxEVT_COMMAND_MENU_SELECTED, CTaskTablePanel::eCmdCancel);
    evt.SetEventObject(this);
    GetParent()->GetEventHandler()->ProcessEvent(evt);
    EndModal(wxID_OK);
}


END_NCBI_SCOPE

