/*  $Id: seqtextviewoptions.cpp 33746 2015-09-11 19:58:46Z shkeda $
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
 * Authors:  Colleen Bollin
 *
 * File Description:
 *
 */

#include <ncbi_pch.hpp>

#include "seqtextviewoptions.h"

#include <gui/widgets/wx/wx_utils.hpp>

#include <wx/sizer.h>
#include <wx/stattext.h>
#include <wx/statbox.h>

#include <wx/radiobut.h>
#include <wx/choice.h>
#include <wx/button.h>
#include <wx/icon.h>


BEGIN_NCBI_SCOPE

IMPLEMENT_DYNAMIC_CLASS( CwxSeqTextViewOptions, wxDialog )


BEGIN_EVENT_TABLE( CwxSeqTextViewOptions, wxDialog )

////@begin CwxSeqTextViewOptions event table entries
    EVT_BUTTON( ID_SEQTEXTDLG_OK, CwxSeqTextViewOptions::OnSeqtextdlgOkClick )

    EVT_BUTTON( ID_SEQTEXTDLG_CANCEL, CwxSeqTextViewOptions::OnSeqtextdlgCancelClick )

////@end CwxSeqTextViewOptions event table entries

END_EVENT_TABLE()


CwxSeqTextViewOptions::CwxSeqTextViewOptions()
{
    m_Config.Reset (new CSeqTextConfig());

    Init();
    x_LoadSettings ();
}

CwxSeqTextViewOptions::CwxSeqTextViewOptions( wxWindow* parent, CRef<CSeqTextConfig> cfg, wxWindowID id, const wxString& caption, const wxPoint& pos, const wxSize& size, long style )
{
    if (cfg) {
        m_Config.Reset (cfg);
    } else {
        m_Config.Reset (new CSeqTextConfig());
    }

    Init();
    Create(parent, id, caption, pos, size, style);
    x_LoadSettings ();
}


/*!
 * CwxSeqTextViewOptions creator
 */

bool CwxSeqTextViewOptions::Create( wxWindow* parent, wxWindowID id, const wxString& caption, const wxPoint& pos, const wxSize& size, long style )
{
////@begin CwxSeqTextViewOptions creation
    SetExtraStyle(wxWS_EX_BLOCK_EVENTS);
    wxDialog::Create( parent, id, caption, pos, size, style );

    CreateControls();
    if (GetSizer())
    {
        GetSizer()->SetSizeHints(this);
    }
    Centre();
////@end CwxSeqTextViewOptions creation
    return true;
}


/*!
 * CwxSeqTextViewOptions destructor
 */

CwxSeqTextViewOptions::~CwxSeqTextViewOptions()
{
////@begin CwxSeqTextViewOptions destruction
////@end CwxSeqTextViewOptions destruction
}


/*!
 * Member initialisation
 */

void CwxSeqTextViewOptions::Init()
{
////@begin CwxSeqTextViewOptions member initialisation
    m_FontSize = NULL;
    m_FeatColor = NULL;
    m_CodonDisplay = NULL;
    m_DispCoord = NULL;
////@end CwxSeqTextViewOptions member initialisation
}


/*!
 * Control creation for CwxSeqTextViewOptions
 */

void CwxSeqTextViewOptions::CreateControls()
{
 
////@begin CwxSeqTextViewOptions content construction
    CwxSeqTextViewOptions* itemDialog1 = this;

    wxBoxSizer* itemBoxSizer2 = new wxBoxSizer(wxVERTICAL);
    itemDialog1->SetSizer(itemBoxSizer2);

    wxBoxSizer* itemBoxSizer3 = new wxBoxSizer(wxVERTICAL);
    itemBoxSizer2->Add(itemBoxSizer3, 0, 0, 0);

    wxStaticBox* itemStaticBoxSizer4Static = new wxStaticBox(itemDialog1, wxID_ANY, _("Settings"));
    wxStaticBoxSizer* itemStaticBoxSizer4 = new wxStaticBoxSizer(itemStaticBoxSizer4Static, wxVERTICAL);
    itemBoxSizer3->Add(itemStaticBoxSizer4, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

    wxBoxSizer* itemBoxSizer5 = new wxBoxSizer(wxHORIZONTAL);
    itemStaticBoxSizer4->Add(itemBoxSizer5, 1, wxALIGN_LEFT|wxALL, 5);

    wxStaticText* itemStaticText6 = new wxStaticText( itemDialog1, wxID_STATIC, _("Font Size:"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer5->Add(itemStaticText6, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxArrayString m_FontSizeStrings;
    m_FontSizeStrings.Add(_("6"));
    m_FontSizeStrings.Add(_("8"));
    m_FontSizeStrings.Add(_("10"));
    m_FontSizeStrings.Add(_("12"));
    m_FontSizeStrings.Add(_("14"));
    m_FontSizeStrings.Add(_("18"));
    m_FontSizeStrings.Add(_("20"));
    m_FontSizeStrings.Add(_("24"));
    m_FontSize = new wxChoice( itemDialog1, ID_SEQTEXTDLG_FONTSIZE, wxDefaultPosition, wxDefaultSize, m_FontSizeStrings, 0 );
    m_FontSize->SetStringSelection(_("12"));
    itemBoxSizer5->Add(m_FontSize, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxBoxSizer* itemBoxSizer8 = new wxBoxSizer(wxHORIZONTAL);
    itemStaticBoxSizer4->Add(itemBoxSizer8, 0, wxALIGN_LEFT|wxALL, 5);

    wxStaticText* itemStaticText9 = new wxStaticText( itemDialog1, wxID_STATIC, _("Feature Coloration:"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer8->Add(itemStaticText9, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxArrayString m_FeatColorStrings;
    m_FeatColorStrings.Add(_("None"));
    m_FeatColorStrings.Add(_("All"));
    m_FeatColorStrings.Add(_("Selected"));
    m_FeatColorStrings.Add(_("MouseOver"));
    m_FeatColor = new wxChoice( itemDialog1, ID_SEQTEXTDLG_FEATCOLOR, wxDefaultPosition, wxDefaultSize, m_FeatColorStrings, 0 );
    m_FeatColor->SetStringSelection(_("All"));
    itemBoxSizer8->Add(m_FeatColor, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxBoxSizer* itemBoxSizer11 = new wxBoxSizer(wxHORIZONTAL);
    itemStaticBoxSizer4->Add(itemBoxSizer11, 0, wxALIGN_LEFT|wxALL, 5);

    wxStaticText* itemStaticText12 = new wxStaticText( itemDialog1, wxID_STATIC, _("Codon Display:"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer11->Add(itemStaticText12, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxArrayString m_CodonDisplayStrings;
    m_CodonDisplayStrings.Add(_("None"));
    m_CodonDisplayStrings.Add(_("All"));
    m_CodonDisplayStrings.Add(_("Selected"));
    m_CodonDisplayStrings.Add(_("MouseOver"));
    m_CodonDisplay = new wxChoice( itemDialog1, ID_SEQTEXTDLG_CODONDISPLAY, wxDefaultPosition, wxDefaultSize, m_CodonDisplayStrings, 0 );
    m_CodonDisplay->SetStringSelection(_("None"));
    itemBoxSizer11->Add(m_CodonDisplay, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxBoxSizer* itemBoxSizer14 = new wxBoxSizer(wxHORIZONTAL);
    itemStaticBoxSizer4->Add(itemBoxSizer14, 0, wxALIGN_LEFT|wxALL, 5);

    wxStaticText* itemStaticText15 = new wxStaticText( itemDialog1, wxID_STATIC, _("Display Coordinates:"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer14->Add(itemStaticText15, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxArrayString m_DispCoordStrings;
    m_DispCoordStrings.Add(_("Absolute"));
    m_DispCoordStrings.Add(_("Relative"));
    m_DispCoord = new wxChoice( itemDialog1, ID_SEQTEXTDLG_DISPLAYCOORD, wxDefaultPosition, wxDefaultSize, m_DispCoordStrings, 0 );
    m_DispCoord->SetStringSelection(_("Absolute"));
    if (CwxSeqTextViewOptions::ShowToolTips())
        m_DispCoord->SetToolTip(_("Display coordinates relative to start of sequence or to start of selection"));
    itemBoxSizer14->Add(m_DispCoord, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxGridSizer* itemGridSizer17 = new wxGridSizer(0, 2, 0, 0);
    itemBoxSizer2->Add(itemGridSizer17, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

    wxButton* itemButton18 = new wxButton( itemDialog1, ID_SEQTEXTDLG_OK, _("Ok"), wxDefaultPosition, wxDefaultSize, 0 );
    itemGridSizer17->Add(itemButton18, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxButton* itemButton19 = new wxButton( itemDialog1, ID_SEQTEXTDLG_CANCEL, _("Cancel"), wxDefaultPosition, wxDefaultSize, 0 );
    itemGridSizer17->Add(itemButton19, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxALL, 5);

////@end CwxSeqTextViewOptions content construction
}


/*!
 * Should we show tooltips?
 */

bool CwxSeqTextViewOptions::ShowToolTips()
{
    return true;
}

/*!
 * Get bitmap resources
 */

wxBitmap CwxSeqTextViewOptions::GetBitmapResource( const wxString& name )
{
    // Bitmap retrieval
////@begin CwxSeqTextViewOptions bitmap retrieval
    wxUnusedVar(name);
    return wxNullBitmap;
////@end CwxSeqTextViewOptions bitmap retrieval
}

/*!
 * Get icon resources
 */

wxIcon CwxSeqTextViewOptions::GetIconResource( const wxString& name )
{
    // Icon retrieval
////@begin CwxSeqTextViewOptions icon retrieval
    wxUnusedVar(name);
    return wxNullIcon;
////@end CwxSeqTextViewOptions icon retrieval
}


void CwxSeqTextViewOptions::x_SaveSettings()
{
    // case for selected feature type
    //m_Config->SetShowFeatAsLower(m_LowerCase->GetValue());

    //display coordinates
    if (m_DispCoord->GetSelection() == 0) {
        m_Config->SetShowAbsolutePosition (true);
    } else {
        m_Config->SetShowAbsolutePosition (false);
    }

    // font size
    string str(ToStdString(m_FontSize->GetStringSelection()));
    int font_size = NStr::StringToInt (str);
    m_Config->SetFontSize(font_size);

    // feat coloring choice
    m_Config->SetFeatureColorationChoice ((CSeqTextPaneConfig::EFeatureDisplayType) m_FeatColor->GetSelection());

    // codon display choice
    m_Config->SetCodonDrawChoice ((CSeqTextPaneConfig::EFeatureDisplayType) m_CodonDisplay->GetSelection());
}


void CwxSeqTextViewOptions::x_LoadSettings()
{
    // case for selected feature type
 //   m_LowerCase->SetValue(m_Config->GetShowFeatAsLower());

    //display coordinates
    if (m_Config->GetShowAbsolutePosition()) {
        m_DispCoord->SetSelection(0);
    } else {
        m_DispCoord->SetSelection(1);
    }

    // font size
    int font_size = m_Config->GetFontSize ();
    string str = NStr::IntToString (font_size);
    m_FontSize->SetStringSelection(ToWxString(str));

    // feat coloring choice
    m_FeatColor->SetSelection (m_Config->GetFeatureColorationChoice ());

    // codon display choice
    m_CodonDisplay->SetSelection (m_Config->GetCodonDrawChoice ());

}


/*!
* wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_SEQTEXTDLG_OK
*/

void CwxSeqTextViewOptions::OnSeqtextdlgOkClick(wxCommandEvent& event)
{
    x_SaveSettings();
    EndModal(ID_SEQTEXTDLG_OK);
}


/*!
* wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_SEQTEXTDLG_CANCEL
*/

void CwxSeqTextViewOptions::OnSeqtextdlgCancelClick(wxCommandEvent& event)
{
    EndModal(ID_SEQTEXTDLG_CANCEL); 
}



END_NCBI_SCOPE




