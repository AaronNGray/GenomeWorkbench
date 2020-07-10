/*  $Id: wx_hairline_dlg.cpp 39684 2017-10-26 16:26:25Z katargir $
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
 * Authors: Vlad Lebedev, Liangshou Wu
 *
 * File Description:
 *
 */
#include <ncbi_pch.hpp>
#include <gui/widgets/wx/dialog_utils.hpp>
#include <gui/widgets/wx/color_picker_validator.hpp>

#include <gui/widgets/wx/wx_utils.hpp>

#include <wx/icon.h>
#include <wx/checkbox.h>

////@begin includes
////@end includes

#include "wx_hairline_dlg.hpp"

////@begin XPM images
////@end XPM images

BEGIN_NCBI_SCOPE

IMPLEMENT_DYNAMIC_CLASS( CwxHairlineDlg, wxDialog )

BEGIN_EVENT_TABLE( CwxHairlineDlg, wxDialog )

////@begin CwxHairlineDlg event table entries
    EVT_CHECKBOX( ID_CHECKBOX, CwxHairlineDlg::OnCheckboxClick )

    EVT_BUTTON( wxID_OK, CwxHairlineDlg::OnOkClick )

////@end CwxHairlineDlg event table entries

END_EVENT_TABLE()

CwxHairlineDlg::CwxHairlineDlg()
{
    Init();
}

CwxHairlineDlg::CwxHairlineDlg( wxWindow* parent, wxWindowID id, const wxString& caption, const wxPoint& pos, const wxSize& size, long style )
{
    Init();
    CreateX(parent, id, caption, pos, size, style);
}

bool CwxHairlineDlg::CreateX( wxWindow* parent, wxWindowID id, const wxString& caption, const wxPoint& pos, const wxSize& size, long style )
{
////@begin CwxHairlineDlg creation
    SetExtraStyle(wxWS_EX_BLOCK_EVENTS);
    wxDialog::Create( parent, id, caption, pos, size, style );

    CreateControls();
    if (GetSizer())
    {
        GetSizer()->SetSizeHints(this);
    }
    Centre();
////@end CwxHairlineDlg creation
    return true;
}

CwxHairlineDlg::~CwxHairlineDlg()
{
////@begin CwxHairlineDlg destruction
////@end CwxHairlineDlg destruction
}

void CwxHairlineDlg::Init()
{
////@begin CwxHairlineDlg member initialisation
    m_RadioGroup = NULL;
    m_ShowDownward = NULL;
    m_DownColorGroup = NULL;
////@end CwxHairlineDlg member initialisation
}

void CwxHairlineDlg::CreateControls()
{
////@begin CwxHairlineDlg content construction
    CwxHairlineDlg* itemDialog1 = this;

    wxBoxSizer* itemBoxSizer2 = new wxBoxSizer(wxVERTICAL);
    itemDialog1->SetSizer(itemBoxSizer2);

    wxFlexGridSizer* itemFlexGridSizer3 = new wxFlexGridSizer(2, 1, 0, 0);
    itemBoxSizer2->Add(itemFlexGridSizer3, 1, wxGROW|wxALL, 5);

    wxArrayString m_RadioGroupStrings;
    m_RadioGroupStrings.Add(_("Show no hairlines with selection"));
    m_RadioGroupStrings.Add(_("Show all hairlines with selections"));
    m_RadioGroupStrings.Add(_("Show only hairlines that are not shared (i.e., places that are different)"));
    m_RadioGroupStrings.Add(_("Show only hairlines that are shared by any two of the selections"));
    m_RadioGroupStrings.Add(_("Show hairlines shared by *all* of the selections"));
    m_RadioGroup = new wxRadioBox( itemDialog1, ID_RADIOBOX1, wxEmptyString, wxDefaultPosition, wxDefaultSize, m_RadioGroupStrings, 5, wxRA_SPECIFY_ROWS );
    m_RadioGroup->SetSelection(0);
    itemFlexGridSizer3->Add(m_RadioGroup, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxGridSizer* itemGridSizer5 = new wxGridSizer(0, 2, 0, 0);
    itemFlexGridSizer3->Add(itemGridSizer5, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    m_ShowDownward = new wxCheckBox( itemDialog1, ID_CHECKBOX, _("Show downward hairlines "), wxDefaultPosition, wxDefaultSize, 0 );
    m_ShowDownward->SetValue(true);
    itemGridSizer5->Add(m_ShowDownward, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    m_DownColorGroup = new wxBoxSizer(wxHORIZONTAL);
    itemGridSizer5->Add(m_DownColorGroup, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxLEFT|wxRIGHT, 0);

    wxColourPickerCtrl* itemColourPickerCtrl8 = new wxColourPickerCtrl( itemDialog1, ID_COLOURPICKERCTRL );
    m_DownColorGroup->Add(itemColourPickerCtrl8, 0, wxALIGN_CENTER_VERTICAL|wxLEFT|wxRIGHT, 5);

    wxStaticText* itemStaticText9 = new wxStaticText( itemDialog1, wxID_STATIC, _("Downward Interval Boundary"), wxDefaultPosition, wxDefaultSize, 0 );
    m_DownColorGroup->Add(itemStaticText9, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxBoxSizer* itemBoxSizer10 = new wxBoxSizer(wxHORIZONTAL);
    itemGridSizer5->Add(itemBoxSizer10, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxRIGHT|wxTOP|wxBOTTOM, 0);

    wxColourPickerCtrl* itemColourPickerCtrl11 = new wxColourPickerCtrl( itemDialog1, ID_COLOURPICKERCTRL1 );
    itemBoxSizer10->Add(itemColourPickerCtrl11, 0, wxALIGN_CENTER_VERTICAL|wxRIGHT|wxTOP|wxBOTTOM, 5);

    wxStaticText* itemStaticText12 = new wxStaticText( itemDialog1, wxID_STATIC, _("Unshared Interval Boundary"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer10->Add(itemStaticText12, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxBoxSizer* itemBoxSizer13 = new wxBoxSizer(wxHORIZONTAL);
    itemGridSizer5->Add(itemBoxSizer13, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxALL, 0);

    wxColourPickerCtrl* itemColourPickerCtrl14 = new wxColourPickerCtrl( itemDialog1, ID_COLOURPICKERCTRL2 );
    itemBoxSizer13->Add(itemColourPickerCtrl14, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxStaticText* itemStaticText15 = new wxStaticText( itemDialog1, wxID_STATIC, _("Shared Interval Boundary     "), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer13->Add(itemStaticText15, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    itemFlexGridSizer3->AddGrowableRow(0);
    itemFlexGridSizer3->AddGrowableCol(0);

    wxBoxSizer* itemBoxSizer16 = new wxBoxSizer(wxHORIZONTAL);
    itemBoxSizer2->Add(itemBoxSizer16, 0, wxGROW|wxALL, 5);

    itemBoxSizer16->Add(5, 5, 1, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxStdDialogButtonSizer* itemStdDialogButtonSizer18 = new wxStdDialogButtonSizer;

    itemBoxSizer16->Add(itemStdDialogButtonSizer18, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);
    wxButton* itemButton19 = new wxButton( itemDialog1, wxID_OK, _("&OK"), wxDefaultPosition, wxDefaultSize, 0 );
    itemButton19->SetDefault();
    itemStdDialogButtonSizer18->AddButton(itemButton19);

    wxButton* itemButton20 = new wxButton( itemDialog1, wxID_CANCEL, _("&Cancel"), wxDefaultPosition, wxDefaultSize, 0 );
    itemStdDialogButtonSizer18->AddButton(itemButton20);

    itemStdDialogButtonSizer18->Realize();

    // Set validators
    itemColourPickerCtrl8->SetValidator( CColorPickerValidator(& m_Downward) );
    itemColourPickerCtrl11->SetValidator( CColorPickerValidator(& m_UnShared) );
    itemColourPickerCtrl14->SetValidator( CColorPickerValidator(& m_Shared) );
////@end CwxHairlineDlg content construction

}


void CwxHairlineDlg::SetConfig(CRef<CSeqGraphicConfig> cfg)
{
    m_Cfg.Reset(cfg);

    m_RadioGroup->SetSelection(m_Cfg->GetHairlineOption());
    m_UnShared = m_Cfg->GetSelHairlineOneColor();
    m_Shared = m_Cfg->GetSelHairlineManyColor();
    m_Downward = m_Cfg->GetSelHairlineDownColor();
    m_ShowDownward->SetValue(m_Cfg->GetShowDownwardHairlines());
    m_DownColorGroup->Show(m_ShowDownward->GetValue());
    Layout();

    //m_LightColor->SetColor(cL);
    //m_DarkColor-> SetColor(cD);

}

bool CwxHairlineDlg::ShowToolTips()
{
    return true;
}
wxBitmap CwxHairlineDlg::GetBitmapResource( const wxString& name )
{
    // Bitmap retrieval
////@begin CwxHairlineDlg bitmap retrieval
    wxUnusedVar(name);
    return wxNullBitmap;
////@end CwxHairlineDlg bitmap retrieval
}
wxIcon CwxHairlineDlg::GetIconResource( const wxString& name )
{
    // Icon retrieval
////@begin CwxHairlineDlg icon retrieval
    wxUnusedVar(name);
    return wxNullIcon;
////@end CwxHairlineDlg icon retrieval
}



void CwxHairlineDlg::OnOkClick( wxCommandEvent& event )
{
    if (TransferDataFromWindow()) {
        m_Cfg->SetHairlineOption(m_RadioGroup->GetSelection());
        m_Cfg->SetSelHairlineOneColor(m_UnShared);
        m_Cfg->SetSelHairlineManyColor(m_Shared);
        m_Cfg->SetSelHairlineDownColor(m_Downward);
        m_Cfg->SetShowDownwardHairlines(m_ShowDownward->GetValue());
    }
    event.Skip();
}


void CwxHairlineDlg::OnCheckboxClick( wxCommandEvent& event )
{
    m_DownColorGroup->Show(m_ShowDownward->GetValue());
    Layout();

    event.Skip();
}

END_NCBI_SCOPE
