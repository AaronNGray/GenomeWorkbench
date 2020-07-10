/*  $Id: wx_aln_stat_dlg.cpp 26098 2012-07-20 16:04:00Z wuliangs $
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
#include <ncbi_pch.hpp>////@begin includes
////@end includes

#include "wx_aln_stat_dlg.hpp"
#include <corelib/ncbistr.hpp>
#include <gui/widgets/wx/wx_utils.hpp>
#include <wx/textctrl.h>
#include <wx/button.h>
#include <wx/slider.h>
#include <wx/statbox.h>

////@begin XPM images
////@end XPM images

BEGIN_NCBI_SCOPE


IMPLEMENT_DYNAMIC_CLASS( CwxAlnStatOptionsDlg, wxDialog )

BEGIN_EVENT_TABLE( CwxAlnStatOptionsDlg, wxDialog )

////@begin CwxAlnStatOptionsDlg event table entries
////@end CwxAlnStatOptionsDlg event table entries

END_EVENT_TABLE()

CwxAlnStatOptionsDlg::CwxAlnStatOptionsDlg()
{
    Init();
}

CwxAlnStatOptionsDlg::CwxAlnStatOptionsDlg( wxWindow* parent, wxWindowID id, const wxString& caption, const wxPoint& pos, const wxSize& size, long style )
{
    Init();
    CreateX(parent, id, caption, pos, size, style);
}

bool CwxAlnStatOptionsDlg::CreateX( wxWindow* parent, wxWindowID id, const wxString& caption, const wxPoint& pos, const wxSize& size, long style )
{
////@begin CwxAlnStatOptionsDlg creation
    SetExtraStyle(wxWS_EX_BLOCK_EVENTS);
    wxDialog::Create( parent, id, caption, pos, size, style );

    CreateControls();
    if (GetSizer())
    {
        GetSizer()->SetSizeHints(this);
    }
    Centre();
////@end CwxAlnStatOptionsDlg creation
    return true;
}

CwxAlnStatOptionsDlg::~CwxAlnStatOptionsDlg()
{
////@begin CwxAlnStatOptionsDlg destruction
////@end CwxAlnStatOptionsDlg destruction
}

void CwxAlnStatOptionsDlg::Init()
{
////@begin CwxAlnStatOptionsDlg member initialisation
    m_ZoomLevel = NULL;
    m_Content = NULL;
    m_ValueType = NULL;
    m_Display = NULL;
    m_GraphHeight = NULL;
////@end CwxAlnStatOptionsDlg member initialisation
}

void CwxAlnStatOptionsDlg::CreateControls()
{    
////@begin CwxAlnStatOptionsDlg content construction
    CwxAlnStatOptionsDlg* itemDialog1 = this;

    wxBoxSizer* itemBoxSizer2 = new wxBoxSizer(wxVERTICAL);
    itemDialog1->SetSizer(itemBoxSizer2);

    wxStaticBox* itemStaticBoxSizer3Static = new wxStaticBox(itemDialog1, wxID_ANY, _("Zoom Level to Turn On Statistics"));
    wxStaticBoxSizer* itemStaticBoxSizer3 = new wxStaticBoxSizer(itemStaticBoxSizer3Static, wxHORIZONTAL);
    itemBoxSizer2->Add(itemStaticBoxSizer3, 0, wxALIGN_CENTER_HORIZONTAL|wxLEFT|wxRIGHT|wxTOP, 5);

    wxStaticText* itemStaticText4 = new wxStaticText( itemDialog1, wxID_STATIC, _("Sequence\nLevel"), wxDefaultPosition, wxDefaultSize, 0 );
    itemStaticBoxSizer3->Add(itemStaticText4, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    m_ZoomLevel = new wxSlider( itemDialog1, ID_SLIDER, 100, 1, 1000, wxDefaultPosition, wxSize(110, -1), wxSL_LABELS );
    m_ZoomLevel->SetHelpText(_("Statistics graph will show only when zoomed into a certain level. Turning on statistics at higher level may result in more computation, and hence less interactive."));
    if (CwxAlnStatOptionsDlg::ShowToolTips())
        m_ZoomLevel->SetToolTip(_("Statistics graph will show only when zoomed into a certain level. Turning on statistics at higher level may result in more computation, and hence less interactive."));
    itemStaticBoxSizer3->Add(m_ZoomLevel, 0, wxALIGN_CENTER_VERTICAL|wxTOP|wxBOTTOM, 5);

    wxStaticText* itemStaticText6 = new wxStaticText( itemDialog1, wxID_STATIC, _("Gene\nLevel"), wxDefaultPosition, wxDefaultSize, 0 );
    itemStaticBoxSizer3->Add(itemStaticText6, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxBoxSizer* itemBoxSizer7 = new wxBoxSizer(wxHORIZONTAL);
    itemBoxSizer2->Add(itemBoxSizer7, 0, wxALIGN_LEFT|wxLEFT|wxRIGHT|wxTOP, 5);

    wxArrayString m_ContentStrings;
    m_ContentStrings.Add(_("Show &A, G, T, C and Gaps"));
    m_ContentStrings.Add(_("Show &Matches, Mismatches and Gaps"));
    m_Content = new wxRadioBox( itemDialog1, ID_RADIOBOX2, _("Data content"), wxDefaultPosition, wxDefaultSize, m_ContentStrings, 1, wxRA_SPECIFY_COLS );
    m_Content->SetSelection(0);
    itemBoxSizer7->Add(m_Content, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxBoxSizer* itemBoxSizer9 = new wxBoxSizer(wxHORIZONTAL);
    itemBoxSizer2->Add(itemBoxSizer9, 0, wxALIGN_LEFT|wxALL, 5);

    wxArrayString m_ValueTypeStrings;
    m_ValueTypeStrings.Add(_("&Count"));
    m_ValueTypeStrings.Add(_("&Percentage"));
    m_ValueType = new wxRadioBox( itemDialog1, ID_RADIOBOX, _("Value type"), wxDefaultPosition, wxDefaultSize, m_ValueTypeStrings, 1, wxRA_SPECIFY_COLS );
    m_ValueType->SetSelection(0);
    itemBoxSizer9->Add(m_ValueType, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxArrayString m_DisplayStrings;
    m_DisplayStrings.Add(_("&Bar Graph   "));
    m_DisplayStrings.Add(_("&Smear Table"));
    m_Display = new wxRadioBox( itemDialog1, ID_RADIOBOX3, _("Display"), wxDefaultPosition, wxDefaultSize, m_DisplayStrings, 1, wxRA_SPECIFY_COLS );
    m_Display->SetSelection(0);
    itemBoxSizer9->Add(m_Display, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxBoxSizer* itemBoxSizer12 = new wxBoxSizer(wxHORIZONTAL);
    itemBoxSizer2->Add(itemBoxSizer12, 0, wxALIGN_LEFT|wxALL, 5);

    wxStaticText* itemStaticText13 = new wxStaticText( itemDialog1, wxID_STATIC, _("Bar graph height"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer12->Add(itemStaticText13, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    m_GraphHeight = new wxTextCtrl( itemDialog1, ID_TEXTCTRL3, _("40"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer12->Add(m_GraphHeight, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxStdDialogButtonSizer* itemStdDialogButtonSizer15 = new wxStdDialogButtonSizer;

    itemBoxSizer2->Add(itemStdDialogButtonSizer15, 0, wxALIGN_RIGHT|wxALL, 5);
    wxButton* itemButton16 = new wxButton( itemDialog1, wxID_OK, _("&OK"), wxDefaultPosition, wxDefaultSize, 0 );
    itemStdDialogButtonSizer15->AddButton(itemButton16);

    wxButton* itemButton17 = new wxButton( itemDialog1, wxID_CANCEL, _("&Cancel"), wxDefaultPosition, wxDefaultSize, 0 );
    itemStdDialogButtonSizer15->AddButton(itemButton17);

    itemStdDialogButtonSizer15->Realize();

////@end CwxAlnStatOptionsDlg content construction
}

void CwxAlnStatOptionsDlg::SetContent(bool is_agtc)
{
    m_Content->SetSelection(is_agtc ? 0 : 1);
}

void CwxAlnStatOptionsDlg::SetValueType(bool is_count)
{
    m_ValueType->SetSelection(is_count ? 0 : 1);
}

void CwxAlnStatOptionsDlg::SetDisplay(bool is_graph)
{
    m_Display->SetSelection(is_graph ? 0 : 1);
}

void CwxAlnStatOptionsDlg::SetGraphHeight(int h)
{
    m_GraphHeight->SetValue(ToWxString(NStr::IntToString(h)));
}

void CwxAlnStatOptionsDlg::SetZoomLevel(int l)
{
    l = max(m_ZoomLevel->GetMin(), l);
    l = min(m_ZoomLevel->GetMax(), l);
    m_ZoomLevel->SetValue(l);
}

bool CwxAlnStatOptionsDlg::IsAGTC() const
{
    return m_Content->GetSelection() == 0;
}

bool CwxAlnStatOptionsDlg::IsCount() const
{
    return m_ValueType->GetSelection() == 0;
}

bool CwxAlnStatOptionsDlg::IsBarGraph() const
{
    return m_Display->GetSelection() == 0;
}

int  CwxAlnStatOptionsDlg::GetGraphHeight() const
{
    int h = 0;
    try {
        h = NStr::StringToInt(ToStdString(m_GraphHeight->GetValue()));
    } catch (CException&) {
    }
    return h;
}

int CwxAlnStatOptionsDlg::GetZoomLevel() const
{
    return m_ZoomLevel->GetValue();
}

bool CwxAlnStatOptionsDlg::ShowToolTips()
{
    return true;
}
wxBitmap CwxAlnStatOptionsDlg::GetBitmapResource( const wxString& name )
{
    // Bitmap retrieval
////@begin CwxAlnStatOptionsDlg bitmap retrieval
    wxUnusedVar(name);
    return wxNullBitmap;
////@end CwxAlnStatOptionsDlg bitmap retrieval
}
wxIcon CwxAlnStatOptionsDlg::GetIconResource( const wxString& name )
{
    // Icon retrieval
////@begin CwxAlnStatOptionsDlg icon retrieval
    wxUnusedVar(name);
    return wxNullIcon;
////@end CwxAlnStatOptionsDlg icon retrieval
}

END_NCBI_SCOPE
