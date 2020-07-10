/*  $Id: project_edit_dlg.cpp 28647 2013-08-09 20:16:42Z katargir $
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

#include <gui/core/project_edit_dlg.hpp>

#include <gui/widgets/wx/message_box.hpp>
#include <gui/widgets/wx/wx_utils.hpp>

#include <wx/textctrl.h>
#include <wx/stattext.h>
#include <wx/sizer.h>
#include <wx/button.h>
#include <wx/valgen.h>

BEGIN_NCBI_SCOPE

IMPLEMENT_DYNAMIC_CLASS( CProjectEditDlg, CDialog )

BEGIN_EVENT_TABLE( CProjectEditDlg, CDialog )

////@begin CProjectEditDlg event table entries
    EVT_BUTTON( wxID_OK, CProjectEditDlg::OnOkClick )

////@end CProjectEditDlg event table entries

END_EVENT_TABLE()


CProjectEditDlg::CProjectEditDlg()
{
    Init();
}

CProjectEditDlg::CProjectEditDlg( wxWindow* parent, wxWindowID id, const wxString& caption, const wxPoint& pos, const wxSize& size, long style )
{
    Init();
    Create(parent, id, caption, pos, size, style);
}

bool CProjectEditDlg::Create( wxWindow* parent, wxWindowID id, const wxString& caption, const wxPoint& pos, const wxSize& size, long style )
{
////@begin CProjectEditDlg creation
    SetExtraStyle(wxWS_EX_BLOCK_EVENTS);
    CDialog::Create( parent, id, caption, pos, size, style );

    CreateControls();
    if (GetSizer())
    {
        GetSizer()->SetSizeHints(this);
    }
    Centre();
////@end CProjectEditDlg creation
    return true;
}

CProjectEditDlg::~CProjectEditDlg()
{
////@begin CProjectEditDlg destruction
////@end CProjectEditDlg destruction
}

void CProjectEditDlg::Init()
{
////@begin CProjectEditDlg member initialisation
////@end CProjectEditDlg member initialisation
}

void CProjectEditDlg::CreateControls()
{
////@begin CProjectEditDlg content construction
    CProjectEditDlg* itemCDialog1 = this;

    wxBoxSizer* itemBoxSizer2 = new wxBoxSizer(wxVERTICAL);
    itemCDialog1->SetSizer(itemBoxSizer2);

    wxFlexGridSizer* itemFlexGridSizer3 = new wxFlexGridSizer(0, 2, 0, 0);
    itemBoxSizer2->Add(itemFlexGridSizer3, 1, wxGROW|wxALL, 5);

    wxStaticText* itemStaticText4 = new wxStaticText( itemCDialog1, wxID_STATIC, _("Name:"), wxDefaultPosition, wxDefaultSize, 0 );
    itemFlexGridSizer3->Add(itemStaticText4, 0, wxALIGN_LEFT|wxALIGN_TOP|wxALL, 5);

    wxTextCtrl* itemTextCtrl5 = new wxTextCtrl( itemCDialog1, ID_NAME, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
    itemFlexGridSizer3->Add(itemTextCtrl5, 1, wxGROW|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxStaticText* itemStaticText6 = new wxStaticText( itemCDialog1, wxID_STATIC, _("Description:"), wxDefaultPosition, wxDefaultSize, 0 );
    itemFlexGridSizer3->Add(itemStaticText6, 0, wxALIGN_RIGHT|wxALIGN_TOP|wxALL, 5);

    wxTextCtrl* itemTextCtrl7 = new wxTextCtrl( itemCDialog1, ID_DESCR, wxEmptyString, wxDefaultPosition, itemCDialog1->ConvertDialogToPixels(wxSize(160, 60)), wxTE_MULTILINE );
    itemFlexGridSizer3->Add(itemTextCtrl7, 1, wxGROW|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxStaticText* itemStaticText8 = new wxStaticText( itemCDialog1, wxID_STATIC, _("File Name:"), wxDefaultPosition, wxDefaultSize, 0 );
    itemFlexGridSizer3->Add(itemStaticText8, 0, wxALIGN_LEFT|wxALIGN_TOP|wxALL, 5);

    wxTextCtrl* itemTextCtrl9 = new wxTextCtrl( itemCDialog1, ID_FILENAME, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_READONLY );
    itemFlexGridSizer3->Add(itemTextCtrl9, 1, wxGROW|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxStaticText* itemStaticText10 = new wxStaticText( itemCDialog1, wxID_STATIC, _("Created:"), wxDefaultPosition, wxDefaultSize, 0 );
    itemFlexGridSizer3->Add(itemStaticText10, 0, wxALIGN_LEFT|wxALIGN_TOP|wxALL, 5);

    wxTextCtrl* itemTextCtrl11 = new wxTextCtrl( itemCDialog1, ID_CREATED, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_READONLY );
    itemFlexGridSizer3->Add(itemTextCtrl11, 1, wxGROW|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxStaticText* itemStaticText12 = new wxStaticText( itemCDialog1, wxID_STATIC, _("Modified:"), wxDefaultPosition, wxDefaultSize, 0 );
    itemFlexGridSizer3->Add(itemStaticText12, 0, wxALIGN_LEFT|wxALIGN_TOP|wxALL, 5);

    wxTextCtrl* itemTextCtrl13 = new wxTextCtrl( itemCDialog1, ID_MODIFIED, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_READONLY );
    itemFlexGridSizer3->Add(itemTextCtrl13, 1, wxGROW|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    itemFlexGridSizer3->AddGrowableRow(1);
    itemFlexGridSizer3->AddGrowableCol(1);

    wxStdDialogButtonSizer* itemStdDialogButtonSizer14 = new wxStdDialogButtonSizer;

    itemBoxSizer2->Add(itemStdDialogButtonSizer14, 0, wxALIGN_RIGHT|wxALL, 5);
    wxButton* itemButton15 = new wxButton( itemCDialog1, wxID_OK, _("&OK"), wxDefaultPosition, wxDefaultSize, 0 );
    itemButton15->SetDefault();
    itemStdDialogButtonSizer14->AddButton(itemButton15);

    wxButton* itemButton16 = new wxButton( itemCDialog1, wxID_CANCEL, _("&Cancel"), wxDefaultPosition, wxDefaultSize, 0 );
    itemStdDialogButtonSizer14->AddButton(itemButton16);

    itemStdDialogButtonSizer14->Realize();

    // Set validators
    itemTextCtrl5->SetValidator( wxGenericValidator(& m_Name) );
    itemTextCtrl7->SetValidator( wxGenericValidator(& m_Descr) );
    itemTextCtrl9->SetValidator( wxGenericValidator(& m_Filename) );
    itemTextCtrl11->SetValidator( wxGenericValidator(& m_Created) );
    itemTextCtrl13->SetValidator( wxGenericValidator(& m_Modified) );
////@end CProjectEditDlg content construction
}


void CProjectEditDlg::SetReadOnly()
{
    wxTextCtrl* ctrl = (wxTextCtrl*)FindWindow(ID_NAME);
    if (ctrl)
        ctrl->SetEditable(false);

    ctrl = (wxTextCtrl*)FindWindow(ID_DESCR);
    if (ctrl)
        ctrl->SetEditable(false);
}


void CProjectEditDlg::OnOkClick( wxCommandEvent& WXUNUSED(event) )
{
    if(TransferDataFromWindow())    {
        if(m_Name.empty())  {
            NcbiErrorBox("Name cannot be empty!");
            return;
        }
        EndModal(wxID_OK);
    }
}


bool CProjectEditDlg::ShowToolTips()
{
    return true;
}


wxBitmap CProjectEditDlg::GetBitmapResource( const wxString& name )
{
    // Bitmap retrieval
////@begin CProjectEditDlg bitmap retrieval
    wxUnusedVar(name);
    return wxNullBitmap;
////@end CProjectEditDlg bitmap retrieval
}


wxIcon CProjectEditDlg::GetIconResource( const wxString& name )
{
    // Icon retrieval
////@begin CProjectEditDlg icon retrieval
    wxUnusedVar(name);
    return wxNullIcon;
////@end CProjectEditDlg icon retrieval
}


END_NCBI_SCOPE


