/*  $Id: existingtextdlg.cpp 34945 2016-03-03 20:31:51Z asztalos $
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
 * Authors:  Colleen Bollin
 */


#include <ncbi_pch.hpp>

////@begin includes
////@end includes

#include <wx/stattext.h>
#include <wx/sizer.h>
#include <wx/button.h>
#include <gui/packages/pkg_sequence_edit/existingtextdlg.hpp>

////@begin XPM images
////@end XPM images

BEGIN_NCBI_SCOPE
USING_SCOPE(objects);
USING_SCOPE(edit);

/*!
 * CExistingTextDlg type definition
 */

IMPLEMENT_DYNAMIC_CLASS( CExistingTextDlg, wxDialog )


/*!
 * CExistingTextDlg event table definition
 */

BEGIN_EVENT_TABLE( CExistingTextDlg, wxDialog )

////@begin CExistingTextDlg event table entries
    EVT_RADIOBUTTON( ID_APPEND_BTN, CExistingTextDlg::OnAppendBtnSelected )

    EVT_RADIOBUTTON( ID_PREFIX_BTN, CExistingTextDlg::OnPrefixBtnSelected )

    EVT_RADIOBUTTON( ID_IGNORE_BTN, CExistingTextDlg::OnIgnoreBtnSelected )

    EVT_RADIOBUTTON( ID_ADD_QUAL_BTN, CExistingTextDlg::OnAddQualBtnSelected )

////@end CExistingTextDlg event table entries

END_EVENT_TABLE()


/*!
 * CExistingTextDlg constructors
 */

CExistingTextDlg::CExistingTextDlg()
{
    Init();
}

CExistingTextDlg::CExistingTextDlg( wxWindow* parent, wxWindowID id, const wxString& caption, const wxPoint& pos, const wxSize& size, long style )
{
    Init();
    Create(parent, id, caption, pos, size, style);
}


/*!
 * CExistingTextDlg creator
 */

bool CExistingTextDlg::Create( wxWindow* parent, wxWindowID id, const wxString& caption, const wxPoint& pos, const wxSize& size, long style )
{
////@begin CExistingTextDlg creation
    SetExtraStyle(wxWS_EX_BLOCK_EVENTS);
    wxDialog::Create( parent, id, caption, pos, size, style );

    CreateControls();
    if (GetSizer())
    {
        GetSizer()->SetSizeHints(this);
    }
    Centre();
////@end CExistingTextDlg creation
    return true;
}


/*!
 * CExistingTextDlg destructor
 */

CExistingTextDlg::~CExistingTextDlg()
{
////@begin CExistingTextDlg destruction
////@end CExistingTextDlg destruction
}


/*!
 * Member initialisation
 */

void CExistingTextDlg::Init()
{
////@begin CExistingTextDlg member initialisation
    m_Position = NULL;
    m_Append = NULL;
    m_Prefix = NULL;
    m_LeaveOld = NULL;
    m_AddQual = NULL;
    m_Delimiters = NULL;
    m_Semicolon = NULL;
    m_Space = NULL;
    m_Colon = NULL;
    m_Comma = NULL;
    m_NoDelimiter = NULL;
////@end CExistingTextDlg member initialisation
}


/*!
 * Control creation for CExistingTextDlg
 */

void CExistingTextDlg::CreateControls()
{    
////@begin CExistingTextDlg content construction
    CExistingTextDlg* itemDialog1 = this;

    wxBoxSizer* itemBoxSizer2 = new wxBoxSizer(wxVERTICAL);
    itemDialog1->SetSizer(itemBoxSizer2);

    m_Position = new wxBoxSizer(wxVERTICAL);
    itemBoxSizer2->Add(m_Position, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

    m_Append = new wxRadioButton( itemDialog1, ID_APPEND_BTN, _("Append"), wxDefaultPosition, wxDefaultSize, wxRB_GROUP );
    m_Append->SetValue(true);
    m_Position->Add(m_Append, 0, wxALIGN_LEFT|wxALL, 5);

    m_Prefix = new wxRadioButton( itemDialog1, ID_PREFIX_BTN, _("Prefix"), wxDefaultPosition, wxDefaultSize, 0 );
    m_Prefix->SetValue(false);
    m_Position->Add(m_Prefix, 0, wxALIGN_LEFT|wxALL, 5);

    m_LeaveOld = new wxRadioButton( itemDialog1, ID_IGNORE_BTN, _("Ignore new text"), wxDefaultPosition, wxDefaultSize, 0 );
    m_LeaveOld->SetValue(false);
    m_Position->Add(m_LeaveOld, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

    m_AddQual = new wxRadioButton( itemDialog1, ID_ADD_QUAL_BTN, _("Add new qual"), wxDefaultPosition, wxDefaultSize, 0 );
    m_AddQual->SetValue(false);
    m_AddQual->Show(false);
    m_Position->Add(m_AddQual, 0, wxALIGN_LEFT|wxALL, 5);

    wxStaticText* itemStaticText8 = new wxStaticText( itemDialog1, wxID_STATIC, _("Separate new text and old text with"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer2->Add(itemStaticText8, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

    m_Delimiters = new wxBoxSizer(wxVERTICAL);
    itemBoxSizer2->Add(m_Delimiters, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

    m_Semicolon = new wxRadioButton( itemDialog1, ID_SEMICOLON_BTN, _("Semicolon"), wxDefaultPosition, wxDefaultSize, wxRB_GROUP );
    m_Semicolon->SetValue(true);
    m_Delimiters->Add(m_Semicolon, 0, wxALIGN_LEFT|wxALL, 5);

    m_Space = new wxRadioButton( itemDialog1, ID_SPACE_BTN, _("Space"), wxDefaultPosition, wxDefaultSize, 0 );
    m_Space->SetValue(false);
    m_Delimiters->Add(m_Space, 0, wxALIGN_LEFT|wxALL, 5);

    m_Colon = new wxRadioButton( itemDialog1, ID_COLON_BTN, _("Colon"), wxDefaultPosition, wxDefaultSize, 0 );
    m_Colon->SetValue(false);
    m_Delimiters->Add(m_Colon, 0, wxALIGN_LEFT|wxALL, 5);

    m_Comma = new wxRadioButton( itemDialog1, ID_COMMA_BTN, _("Comma"), wxDefaultPosition, wxDefaultSize, 0 );
    m_Comma->SetValue(false);
    m_Delimiters->Add(m_Comma, 0, wxALIGN_LEFT|wxALL, 5);

    m_NoDelimiter = new wxRadioButton( itemDialog1, ID_NODELIMITER_BTN, _("Do not separate"), wxDefaultPosition, wxDefaultSize, 0 );
    m_NoDelimiter->SetValue(false);
    m_Delimiters->Add(m_NoDelimiter, 0, wxALIGN_LEFT|wxALL, 5);

    wxBoxSizer* itemBoxSizer15 = new wxBoxSizer(wxHORIZONTAL);
    itemBoxSizer2->Add(itemBoxSizer15, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

    wxButton* itemButton16 = new wxButton( itemDialog1, wxID_OK, _("Accept"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer15->Add(itemButton16, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxButton* itemButton17 = new wxButton( itemDialog1, wxID_CANCEL, _("Cancel"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer15->Add(itemButton17, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

////@end CExistingTextDlg content construction
}


/*!
 * Should we show tooltips?
 */

bool CExistingTextDlg::ShowToolTips()
{
    return true;
}

/*!
 * Get bitmap resources
 */

wxBitmap CExistingTextDlg::GetBitmapResource( const wxString& name )
{
    // Bitmap retrieval
////@begin CExistingTextDlg bitmap retrieval
    wxUnusedVar(name);
    return wxNullBitmap;
////@end CExistingTextDlg bitmap retrieval
}

/*!
 * Get icon resources
 */

wxIcon CExistingTextDlg::GetIconResource( const wxString& name )
{
    // Icon retrieval
////@begin CExistingTextDlg icon retrieval
    wxUnusedVar(name);
    return wxNullIcon;
////@end CExistingTextDlg icon retrieval
}


EExistingText CExistingTextDlg::GetExistingTextHandler()
{
    EExistingText existing_text = eExistingText_cancel;
    if (m_LeaveOld->GetValue()) {
        existing_text = eExistingText_leave_old;
    } else if (m_Append->GetValue()) {
        if (m_Semicolon->GetValue()) {
            existing_text = eExistingText_append_semi;
        } else if (m_Space->GetValue()) {
            existing_text = eExistingText_append_space;
        } else if (m_Colon->GetValue()) {
            existing_text = eExistingText_append_colon;
        } else if (m_Comma->GetValue()) {
            existing_text = eExistingText_append_comma;
        } else if (m_NoDelimiter->GetValue()) {
            existing_text = eExistingText_append_none;
        }
    } else if (m_Prefix->GetValue()) {
        if (m_Semicolon->GetValue()) {
            existing_text = eExistingText_prefix_semi;
        } else if (m_Space->GetValue()) {
            existing_text = eExistingText_prefix_space;
        } else if (m_Colon->GetValue()) {
            existing_text = eExistingText_prefix_colon;
        } else if (m_Comma->GetValue()) {
            existing_text = eExistingText_prefix_comma;
        } else if (m_NoDelimiter->GetValue()) {
            existing_text = eExistingText_prefix_none;
        }
    } else if (m_AddQual->GetValue()) {
        existing_text = eExistingText_add_qual;
    }

    return existing_text;
}


void CExistingTextDlg::x_EnableDelimiters(bool val)
{
    for (size_t pos = 0; pos < m_Delimiters->GetItemCount(); pos++) {
        wxWindow* w = m_Delimiters->GetItem(pos)->GetWindow();
        wxRadioButton* but = dynamic_cast<wxRadioButton *>(w);
        if (but) {
            but->Enable(val);
        }
    }
}


/*!
 * wxEVT_COMMAND_RADIOBUTTON_SELECTED event handler for ID_APPEND_BTN
 */

void CExistingTextDlg::OnAppendBtnSelected( wxCommandEvent& event )
{
    x_EnableDelimiters (true);
}


/*!
 * wxEVT_COMMAND_RADIOBUTTON_SELECTED event handler for ID_PREFIX_BTN
 */

void CExistingTextDlg::OnPrefixBtnSelected( wxCommandEvent& event )
{
    x_EnableDelimiters (true);
}


/*!
 * wxEVT_COMMAND_RADIOBUTTON_SELECTED event handler for ID_IGNORE_BTN
 */

void CExistingTextDlg::OnIgnoreBtnSelected( wxCommandEvent& event )
{
    x_EnableDelimiters (false);
}


void CExistingTextDlg::AllowAdditionalQual(bool val)
{   
    bool was_shown = m_AddQual->IsShown();
    if (was_shown == val) {
        return;
    }
    m_AddQual->Show(val);
    
    wxSize min_size = this->GetMinClientSize();
    wxSize add_size = m_Append->GetSize();
    if (val) {
        min_size.SetHeight(min_size.GetHeight() + add_size.GetHeight() + 10);
    } else {
        min_size.SetHeight(min_size.GetHeight() - add_size.GetHeight() - 10);
    }

    this->SetMinClientSize(min_size);
    Layout();
    Fit();
    Refresh();
}


/*!
 * wxEVT_COMMAND_RADIOBUTTON_SELECTED event handler for ID_ADD_QUAL_BTN
 */

void CExistingTextDlg::OnAddQualBtnSelected( wxCommandEvent& event )
{
    x_EnableDelimiters (false);
}


END_NCBI_SCOPE
