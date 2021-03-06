/*  $Id: pager_message_dlg.cpp 43918 2019-09-18 20:36:22Z katargir $
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


// Generated by DialogBlocks (unregistered), 17/09/2019 16:32:41

#include <ncbi_pch.hpp>

////@begin includes
////@end includes

#include "pager_message_dlg.hpp"

#include <wx/sizer.h>
#include <wx/button.h>
#include <wx/checkbox.h>
#include <wx/icon.h>

BEGIN_NCBI_SCOPE

/*!
 * CPagerMessageDlg type definition
 */

IMPLEMENT_DYNAMIC_CLASS( CPagerMessageDlg, CDialog )


/*!
 * CPagerMessageDlg event table definition
 */

BEGIN_EVENT_TABLE( CPagerMessageDlg, CDialog )

////@begin CPagerMessageDlg event table entries
    EVT_BUTTON( ID_FEEDBACK_BTM, CPagerMessageDlg::OnFeedbackBtmClick )
////@end CPagerMessageDlg event table entries

END_EVENT_TABLE()


/*!
 * CPagerMessageDlg constructors
 */

CPagerMessageDlg::CPagerMessageDlg()
{
    Init();
}

CPagerMessageDlg::CPagerMessageDlg( wxWindow* parent, wxWindowID id, const wxString& caption, const wxPoint& pos, const wxSize& size, long style )
{
    Init();
    Create(parent, id, caption, pos, size, style);
}


/*!
 * CPagerMessageDlg creator
 */

bool CPagerMessageDlg::Create( wxWindow* parent, wxWindowID id, const wxString& caption, const wxPoint& pos, const wxSize& size, long style )
{
////@begin CPagerMessageDlg creation
    SetExtraStyle(wxWS_EX_VALIDATE_RECURSIVELY|wxWS_EX_BLOCK_EVENTS);
    CDialog::Create( parent, id, caption, pos, size, style );

    CreateControls();
    if (GetSizer())
    {
        GetSizer()->SetSizeHints(this);
    }
    Centre();
////@end CPagerMessageDlg creation

    wxHtmlWindow* htmlWnd = (wxHtmlWindow*)FindWindow(ID_HTMLWINDOW);
    if (htmlWnd) htmlWnd->SetPage(GetHTML());

    return true;
}


/*!
 * CPagerMessageDlg destructor
 */

CPagerMessageDlg::~CPagerMessageDlg()
{
////@begin CPagerMessageDlg destruction
////@end CPagerMessageDlg destruction
}


/*!
 * Member initialisation
 */

void CPagerMessageDlg::Init()
{
    SetRegistryPath("Dialogs.PagerMEssageDlg");
////@begin CPagerMessageDlg member initialisation
    m_DoNotShow = false;
    m_OpenFeedback = false;
////@end CPagerMessageDlg member initialisation
}


/*!
 * Control creation for CPagerMessageDlg
 */

void CPagerMessageDlg::CreateControls()
{    
////@begin CPagerMessageDlg content construction
    // Generated by DialogBlocks, 18/09/2019 12:42:02 (unregistered)

    CPagerMessageDlg* itemDialog1 = this;

    wxBoxSizer* itemBoxSizer2 = new wxBoxSizer(wxVERTICAL);
    itemDialog1->SetSizer(itemBoxSizer2);

    wxHtmlWindow* itemHtmlWindow1 = new wxHtmlWindow( itemDialog1, ID_HTMLWINDOW, wxDefaultPosition, wxSize(400, 300), wxHW_SCROLLBAR_AUTO|wxSUNKEN_BORDER|wxHSCROLL|wxVSCROLL );
    itemBoxSizer2->Add(itemHtmlWindow1, 1, wxGROW|wxALL, 5);

    wxBoxSizer* itemBoxSizer3 = new wxBoxSizer(wxHORIZONTAL);
    itemBoxSizer2->Add(itemBoxSizer3, 0, wxALIGN_RIGHT|wxALL, 5);

    wxCheckBox* itemCheckBox4 = new wxCheckBox( itemDialog1, ID_CHECKBOX, _("Do not to show this message again"), wxDefaultPosition, wxDefaultSize, 0 );
    itemCheckBox4->SetValue(false);
    itemBoxSizer3->Add(itemCheckBox4, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxButton* itemButton1 = new wxButton( itemDialog1, ID_FEEDBACK_BTM, _("Feedback"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer3->Add(itemButton1, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxButton* itemButton5 = new wxButton( itemDialog1, wxID_OK, _("OK"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer3->Add(itemButton5, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    // Set validators
    itemCheckBox4->SetValidator( wxGenericValidator(& m_DoNotShow) );
////@end CPagerMessageDlg content construction
}

void CPagerMessageDlg::OnFeedbackBtmClick( wxCommandEvent& WXUNUSED(event) )
{
    if (TransferDataFromWindow()) {
        SetOpenFeedback(true);
        EndModal(wxID_CANCEL);
    }
}

/*!
 * Should we show tooltips?
 */

bool CPagerMessageDlg::ShowToolTips()
{
    return true;
}

/*!
 * Get bitmap resources
 */

wxBitmap CPagerMessageDlg::GetBitmapResource( const wxString& name )
{
    // Bitmap retrieval
////@begin CPagerMessageDlg bitmap retrieval
    wxUnusedVar(name);
    return wxNullBitmap;
////@end CPagerMessageDlg bitmap retrieval
}

/*!
 * Get icon resources
 */

wxIcon CPagerMessageDlg::GetIconResource( const wxString& name )
{
    // Icon retrieval
////@begin CPagerMessageDlg icon retrieval
    wxUnusedVar(name);
    return wxNullIcon;
////@end CPagerMessageDlg icon retrieval
}

END_NCBI_SCOPE
