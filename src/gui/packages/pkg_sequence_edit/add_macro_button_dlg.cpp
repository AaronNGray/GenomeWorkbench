/*  $Id: add_macro_button_dlg.cpp 40432 2018-02-12 21:50:50Z filippov $
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
 * Authors:  Igor Filippov
 */

#include <ncbi_pch.hpp>

// For compilers that support precompilation, includes "wx/wx.h".
#include "wx/wxprec.h"

#ifdef __BORLANDC__
#pragma hdrstop
#endif

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

////@begin includes
////@end includes

#include <gui/packages/pkg_sequence_edit/add_macro_button_dlg.hpp>

////@begin XPM images
////@end XPM images

BEGIN_NCBI_SCOPE
/*
 * CAddMacroButton type definition
 */

IMPLEMENT_DYNAMIC_CLASS( CAddMacroButton, wxDialog )


/*
 * CAddMacroButton event table definition
 */

BEGIN_EVENT_TABLE( CAddMacroButton, wxDialog )

////@begin CAddMacroButton event table entries
    EVT_CHECKBOX(ID_CHECKBOX, CAddMacroButton::OnReportRequireCheck)
    EVT_FILEPICKER_CHANGED(ID_FILECTRL, CAddMacroButton::OnFileSelected)
////@end CAddMacroButton event table entries

END_EVENT_TABLE()


/*
 * CAddMacroButton constructors
 */

CAddMacroButton::CAddMacroButton()
{
    Init();
}

CAddMacroButton::CAddMacroButton( wxWindow* parent, wxWindowID id, const wxString& caption, const wxPoint& pos, const wxSize& size, long style )
{
    Init();
    Create(parent, id, caption, pos, size, style);
}


/*
 * CAddMacroButton creator
 */

bool CAddMacroButton::Create( wxWindow* parent, wxWindowID id, const wxString& caption, const wxPoint& pos, const wxSize& size, long style )
{
////@begin CAddMacroButton creation
    SetExtraStyle(wxWS_EX_VALIDATE_RECURSIVELY|wxWS_EX_BLOCK_EVENTS);
    wxDialog::Create( parent, id, caption, pos, size, style );

    CreateControls();
    if (GetSizer())
    {
        GetSizer()->SetSizeHints(this);
    }
    Centre();
////@end CAddMacroButton creation
    return true;
}


/*
 * CAddMacroButton destructor
 */

CAddMacroButton::~CAddMacroButton()
{
////@begin CAddMacroButton destruction
////@end CAddMacroButton destruction
}


/*
 * Member initialisation
 */

void CAddMacroButton::Init()
{
////@begin CAddMacroButton member initialisation
    m_ButtonName = NULL;
    m_FileName = NULL;
    m_ReportRequired = NULL;
    m_Title = NULL;
////@end CAddMacroButton member initialisation
}


/*
 * Control creation for CAddMacroButton
 */

void CAddMacroButton::CreateControls()
{    
////@begin CAddMacroButton content construction
    CAddMacroButton* itemDialog1 = this;

    wxBoxSizer* itemBoxSizer2 = new wxBoxSizer(wxVERTICAL);
    itemDialog1->SetSizer(itemBoxSizer2);

    wxStaticText* itemStaticText3 = new wxStaticText( itemDialog1, wxID_STATIC, _("Create Macro Button"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer2->Add(itemStaticText3, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

    wxBoxSizer* itemBoxSizer7 = new wxBoxSizer(wxHORIZONTAL);
    itemBoxSizer2->Add(itemBoxSizer7, 0, wxGROW|wxALL, 5);

    wxStaticText* itemStaticText8 = new wxStaticText( itemDialog1, wxID_STATIC, _("Macro file"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer7->Add(itemStaticText8, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    m_FileName = new wxFilePickerCtrl( itemDialog1, ID_FILECTRL, wxEmptyString, wxEmptyString, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxFLP_DEFAULT_STYLE );
    itemBoxSizer7->Add(m_FileName, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);
    
    wxBoxSizer* itemBoxSizer4 = new wxBoxSizer(wxHORIZONTAL);
    itemBoxSizer2->Add(itemBoxSizer4, 0, wxGROW|wxALL, 5);
    
    wxStaticText* itemStaticText5 = new wxStaticText( itemDialog1, wxID_STATIC, _("Button name"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer4->Add(itemStaticText5, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);
    
    m_ButtonName = new wxTextCtrl( itemDialog1, ID_TEXTCTRL, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer4->Add(m_ButtonName, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);
    wxTextValidator validator( wxFILTER_ASCII | wxFILTER_EMPTY | wxFILTER_EXCLUDE_CHAR_LIST );
    validator.SetCharExcludes(_("*"));
    m_ButtonName->SetValidator( validator );    

    m_ReportRequired = new wxCheckBox( itemDialog1, ID_CHECKBOX, _("Generate Report"), wxDefaultPosition, wxDefaultSize, 0 );
    m_ReportRequired->SetValue(false);
    itemBoxSizer2->Add(m_ReportRequired, 0, wxALIGN_LEFT|wxALL, 5);

    wxBoxSizer* itemBoxSizer11 = new wxBoxSizer(wxHORIZONTAL);
    itemBoxSizer2->Add(itemBoxSizer11, 0, wxGROW|wxALL, 5);

    wxStaticText* itemStaticText12 = new wxStaticText( itemDialog1, wxID_STATIC, _("Report Title"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer11->Add(itemStaticText12, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    m_Title = new wxTextCtrl( itemDialog1, ID_TEXTCTRL1, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer11->Add(m_Title, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);
    m_Title->Enable(false);
    m_Title->SetValidator( wxTextValidator( wxFILTER_ASCII ) );

    wxBoxSizer* itemBoxSizer14 = new wxBoxSizer(wxHORIZONTAL);
    itemBoxSizer2->Add(itemBoxSizer14, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

    wxButton* itemButton15 = new wxButton( itemDialog1, wxID_OK, _("OK"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer14->Add(itemButton15, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxButton* itemButton16 = new wxButton( itemDialog1, wxID_CANCEL, _("Cancel"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer14->Add(itemButton16, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

////@end CAddMacroButton content construction
}


/*
 * Should we show tooltips?
 */

bool CAddMacroButton::ShowToolTips()
{
    return true;
}

/*
 * Get bitmap resources
 */

wxBitmap CAddMacroButton::GetBitmapResource( const wxString& name )
{
    // Bitmap retrieval
////@begin CAddMacroButton bitmap retrieval
    wxUnusedVar(name);
    return wxNullBitmap;
////@end CAddMacroButton bitmap retrieval
}

/*
 * Get icon resources
 */

wxIcon CAddMacroButton::GetIconResource( const wxString& name )
{
    // Icon retrieval
////@begin CAddMacroButton icon retrieval
    wxUnusedVar(name);
    return wxNullIcon;
////@end CAddMacroButton icon retrieval
}

void CAddMacroButton::OnReportRequireCheck(wxCommandEvent& event)
{
    bool required = m_ReportRequired->GetValue();
    m_Title->Enable(required);
    if (required)
	m_Title->SetValue(m_ButtonName->GetValue());
    else
	m_Title->SetValue(wxEmptyString);
}

void CAddMacroButton::OnFileSelected(wxFileDirPickerEvent& event)
{
    wxFileName fn = m_FileName->GetFileName();
    m_ButtonName->SetValue(fn.GetName());
    if (m_ReportRequired->GetValue())
	m_Title->SetValue(fn.GetName());
}

END_NCBI_SCOPE
