/*  $Id: text_view_options_dlg.cpp 41134 2018-05-31 15:32:04Z katargir $
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


#include <ncbi_pch.hpp>

#include <wx/sizer.h>
#include <wx/stattext.h>
#include <wx/button.h>
#include <wx/choice.h>
#include <wx/checkbox.h>
#include <wx/statbox.h>
#include <wx/radiobox.h>
#include <wx/bitmap.h>
#include <wx/icon.h>

////@begin includes
////@end includes

#include "text_view_options_dlg.hpp"
#include <gui/widgets/seq/text_panel.hpp>
#include <gui/widgets/wx/wx_utils.hpp>

////@begin XPM images
////@end XPM images

BEGIN_NCBI_SCOPE

/*!
 * CTextViewOptionsDlg type definition
 */

IMPLEMENT_DYNAMIC_CLASS( CTextViewOptionsDlg, CDialog )


/*!
 * CTextViewOptionsDlg event table definition
 */

BEGIN_EVENT_TABLE( CTextViewOptionsDlg, CDialog )

////@begin CTextViewOptionsDlg event table entries
    EVT_CHOICE( ID_CHOICE5, CTextViewOptionsDlg::OnViewTypeSelected )

    EVT_BUTTON( wxID_OK, CTextViewOptionsDlg::OnOkClick )

    EVT_BUTTON( wxID_APPLY, CTextViewOptionsDlg::OnApplyClick )

////@end CTextViewOptionsDlg event table entries

END_EVENT_TABLE()


/*!
 * CTextViewOptionsDlg constructors
 */

CTextViewOptionsDlg::CTextViewOptionsDlg() : m_Panel()
{
    Init();
}

CTextViewOptionsDlg::CTextViewOptionsDlg( wxWindow* parent, wxWindowID id, const wxString& caption, const wxPoint& pos, const wxSize& size, long style )
    : m_Panel()
{
    Init();
    Create(parent, id, caption, pos, size, style);
}


/*!
 * CTextViewOptionsDlg creator
 */

bool CTextViewOptionsDlg::Create( wxWindow* parent, wxWindowID id, const wxString& caption, const wxPoint& pos, const wxSize& size, long style )
{
    m_Panel = dynamic_cast<CTextPanel*>(parent);

////@begin CTextViewOptionsDlg creation
    SetExtraStyle(wxWS_EX_BLOCK_EVENTS);
    CDialog::Create( parent, id, caption, pos, size, style );

    CreateControls();
    if (GetSizer())
    {
        GetSizer()->SetSizeHints(this);
    }
    Centre();
////@end CTextViewOptionsDlg creation
    return true;
}


/*!
 * CTextViewOptionsDlg destructor
 */

CTextViewOptionsDlg::~CTextViewOptionsDlg()
{
////@begin CTextViewOptionsDlg destruction
////@end CTextViewOptionsDlg destruction
}


/*!
 * Member initialisation
 */

void CTextViewOptionsDlg::Init()
{
////@begin CTextViewOptionsDlg member initialisation
    m_Reload = false;
    m_Invalidate = false;
    m_SaveState = false;
    m_ViewTypeCtrl = NULL;
    m_FontSizeCtrl = NULL;
    m_FlatFileModeCtrl = NULL;
    m_OpenExpandedCtrl = NULL;
    m_HideVariationsCtrl = NULL;
    m_HideStsCtrl = NULL;
    m_ShowSeq = NULL;
    m_ShowComponentFeats = NULL;
    m_FeatureModeCtrl = NULL;
    m_ShowASNTypesCtrl = NULL;
    m_ShowASNPathToRootCtrl = NULL;
////@end CTextViewOptionsDlg member initialisation
}


/*!
 * Control creation for CTextViewOptionsDlg
 */

void CTextViewOptionsDlg::CreateControls()
{    
////@begin CTextViewOptionsDlg content construction
    CTextViewOptionsDlg* itemCDialog1 = this;

    wxBoxSizer* itemBoxSizer2 = new wxBoxSizer(wxVERTICAL);
    itemCDialog1->SetSizer(itemBoxSizer2);

    wxStaticBox* itemStaticBoxSizer3Static = new wxStaticBox(itemCDialog1, wxID_ANY, _("Format"));
    wxStaticBoxSizer* itemStaticBoxSizer3 = new wxStaticBoxSizer(itemStaticBoxSizer3Static, wxHORIZONTAL);
    itemBoxSizer2->Add(itemStaticBoxSizer3, 0, wxGROW|wxALL, 5);

    wxArrayString m_ViewTypeCtrlStrings;
    m_ViewTypeCtrl = new wxChoice( itemCDialog1, ID_CHOICE5, wxDefaultPosition, wxDefaultSize, m_ViewTypeCtrlStrings, 0 );
    itemStaticBoxSizer3->Add(m_ViewTypeCtrl, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    itemStaticBoxSizer3->Add(7, 8, 1, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxStaticText* itemStaticText6 = new wxStaticText( itemCDialog1, wxID_STATIC, _("Font Size:"), wxDefaultPosition, wxDefaultSize, 0 );
    itemStaticBoxSizer3->Add(itemStaticText6, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxArrayString m_FontSizeCtrlStrings;
    m_FontSizeCtrlStrings.Add(_("8"));
    m_FontSizeCtrlStrings.Add(_("10"));
    m_FontSizeCtrlStrings.Add(_("12"));
    m_FontSizeCtrlStrings.Add(_("14"));
    m_FontSizeCtrl = new wxChoice( itemCDialog1, ID_CHOICE2, wxDefaultPosition, wxDefaultSize, m_FontSizeCtrlStrings, 0 );
    m_FontSizeCtrl->SetStringSelection(_("10"));
    itemStaticBoxSizer3->Add(m_FontSizeCtrl, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxStaticBox* itemStaticBoxSizer8Static = new wxStaticBox(itemCDialog1, wxID_ANY, _("Flat File Options"));
    wxStaticBoxSizer* itemStaticBoxSizer8 = new wxStaticBoxSizer(itemStaticBoxSizer8Static, wxVERTICAL);
    itemBoxSizer2->Add(itemStaticBoxSizer8, 0, wxGROW|wxALL, 5);

    wxBoxSizer* itemBoxSizer9 = new wxBoxSizer(wxHORIZONTAL);
    itemStaticBoxSizer8->Add(itemBoxSizer9, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 0);

    wxStaticText* itemStaticText10 = new wxStaticText( itemCDialog1, wxID_STATIC, _("Mode"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer9->Add(itemStaticText10, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxArrayString m_FlatFileModeCtrlStrings;
    m_FlatFileModeCtrlStrings.Add(_("Editing"));
    m_FlatFileModeCtrlStrings.Add(_("Public Preview"));
    m_FlatFileModeCtrl = new wxChoice( itemCDialog1, ID_CHOICE3, wxDefaultPosition, wxDefaultSize, m_FlatFileModeCtrlStrings, 0 );
    m_FlatFileModeCtrl->SetStringSelection(_("Editing"));
    itemBoxSizer9->Add(m_FlatFileModeCtrl, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxFlexGridSizer* itemFlexGridSizer12 = new wxFlexGridSizer(0, 2, 0, 0);
    itemStaticBoxSizer8->Add(itemFlexGridSizer12, 0, wxALIGN_LEFT|wxALL, 5);

    m_OpenExpandedCtrl = new wxCheckBox( itemCDialog1, ID_CHECKBOX4, _("Open Expanded"), wxDefaultPosition, wxDefaultSize, 0 );
    m_OpenExpandedCtrl->SetValue(false);
    itemFlexGridSizer12->Add(m_OpenExpandedCtrl, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    m_HideVariationsCtrl = new wxCheckBox( itemCDialog1, ID_CHECKBOX, _("Hide variation features"), wxDefaultPosition, wxDefaultSize, 0 );
    m_HideVariationsCtrl->SetValue(false);
    itemFlexGridSizer12->Add(m_HideVariationsCtrl, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    itemFlexGridSizer12->Add(7, 8, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    m_HideStsCtrl = new wxCheckBox( itemCDialog1, ID_CHECKBOX3, _("Hide STS features"), wxDefaultPosition, wxDefaultSize, 0 );
    m_HideStsCtrl->SetValue(false);
    itemFlexGridSizer12->Add(m_HideStsCtrl, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxBoxSizer* itemBoxSizer17 = new wxBoxSizer(wxVERTICAL);
    itemStaticBoxSizer8->Add(itemBoxSizer17, 0, wxALIGN_LEFT|wxALL, 5);

    m_ShowSeq = new wxCheckBox( itemCDialog1, ID_CHECKBOX5, _("Show sequence instead of scaffold instructions"), wxDefaultPosition, wxDefaultSize, 0 );
    m_ShowSeq->SetValue(false);
    itemBoxSizer17->Add(m_ShowSeq, 0, wxALIGN_LEFT|wxALL, 5);

    m_ShowComponentFeats = new wxCheckBox( itemCDialog1, ID_CHECKBOX6, _("Show component features"), wxDefaultPosition, wxDefaultSize, 0 );
    m_ShowComponentFeats->SetValue(false);
    itemBoxSizer17->Add(m_ShowComponentFeats, 0, wxALIGN_LEFT|wxALL, 5);

    wxArrayString m_FeatureModeCtrlStrings;
    m_FeatureModeCtrlStrings.Add(_("Show gene model"));
    m_FeatureModeCtrlStrings.Add(_("All features in range"));
    m_FeatureModeCtrl = new wxRadioBox( itemCDialog1, ID_RADIOBOX, _("Feature Mode"), wxDefaultPosition, wxDefaultSize, m_FeatureModeCtrlStrings, 1, wxRA_SPECIFY_ROWS );
    m_FeatureModeCtrl->SetSelection(0);
    itemBoxSizer2->Add(m_FeatureModeCtrl, 0, wxGROW|wxALL, 5);

    wxStaticBox* itemStaticBoxSizer21Static = new wxStaticBox(itemCDialog1, wxID_ANY, _("ASN Options"));
    wxStaticBoxSizer* itemStaticBoxSizer21 = new wxStaticBoxSizer(itemStaticBoxSizer21Static, wxHORIZONTAL);
    itemBoxSizer2->Add(itemStaticBoxSizer21, 0, wxGROW|wxALL, 5);

    m_ShowASNTypesCtrl = new wxCheckBox( itemCDialog1, ID_CHECKBOX1, _("Show ASN.1 types"), wxDefaultPosition, wxDefaultSize, 0 );
    m_ShowASNTypesCtrl->SetValue(false);
    itemStaticBoxSizer21->Add(m_ShowASNTypesCtrl, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    itemStaticBoxSizer21->Add(7, 8, 1, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    m_ShowASNPathToRootCtrl = new wxCheckBox( itemCDialog1, ID_CHECKBOX2, _("Show path to root"), wxDefaultPosition, wxDefaultSize, 0 );
    m_ShowASNPathToRootCtrl->SetValue(false);
    itemStaticBoxSizer21->Add(m_ShowASNPathToRootCtrl, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxStaticLine* itemStaticLine25 = new wxStaticLine( itemCDialog1, wxID_STATIC, wxDefaultPosition, wxDefaultSize, wxLI_HORIZONTAL );
    itemBoxSizer2->Add(itemStaticLine25, 0, wxGROW|wxALL, 5);

    wxStdDialogButtonSizer* itemStdDialogButtonSizer26 = new wxStdDialogButtonSizer;

    itemBoxSizer2->Add(itemStdDialogButtonSizer26, 0, wxALIGN_RIGHT|wxALL, 5);
    wxButton* itemButton27 = new wxButton( itemCDialog1, wxID_OK, _("&OK"), wxDefaultPosition, wxDefaultSize, 0 );
    itemStdDialogButtonSizer26->AddButton(itemButton27);

    wxButton* itemButton28 = new wxButton( itemCDialog1, wxID_CANCEL, _("&Cancel"), wxDefaultPosition, wxDefaultSize, 0 );
    itemStdDialogButtonSizer26->AddButton(itemButton28);

    wxButton* itemButton29 = new wxButton( itemCDialog1, wxID_APPLY, _("&Apply"), wxDefaultPosition, wxDefaultSize, 0 );
    itemStdDialogButtonSizer26->AddButton(itemButton29);

    itemStdDialogButtonSizer26->Realize();

////@end CTextViewOptionsDlg content construction
    if (RunningInsideNCBI()) {
        m_FlatFileModeCtrl->AppendString(_("RefSeq Preview"));
    }

    x_TransferDataToWindow();
}

void CTextViewOptionsDlg::x_TransferDataToWindow()
{
    if (!m_Panel)
        return;

    if (m_Panel->CanShowFlatFile())
        m_ViewTypeCtrl->Append(wxT("Flat File"), (void*)CTextPanel::kFlatFile);
    if (m_Panel->CanShowFastA())
        m_ViewTypeCtrl->Append(wxT("FastA"), (void*)CTextPanel::kFastA);
    if (m_Panel->CanShowASN())
        m_ViewTypeCtrl->Append(wxT("ASN"), (void*)CTextPanel::kASN);
//  if (m_Panel->CanShowXML())
//      m_ViewTypeCtrl->Append(wxT("XML"), (void*)CTextPanel::kXML);

    CTextPanel::TViewType viewType = m_Panel->GetViewType();
    if (viewType == CTextPanel::kXML)
        viewType = CTextPanel::kASN;

    for (unsigned int i = 0; i < m_ViewTypeCtrl->GetCount(); ++i) {
        if ((CTextPanel::TViewType)(intptr_t)m_ViewTypeCtrl->GetClientData(i) == viewType) {
            m_ViewTypeCtrl->SetSelection(i);
            break;
        }
    }

    int selected = m_Panel->GetFontSize();
    if (selected >= 0 && (unsigned int)selected < m_FontSizeCtrl->GetCount())
        m_FontSizeCtrl->SetSelection(selected);

    selected = m_Panel->GetFlatFileMode();
    if (selected >= 0 && (unsigned int)selected < m_FlatFileModeCtrl->GetCount())
        m_FlatFileModeCtrl->SetSelection(selected);

    m_ShowSeq->SetValue(m_Panel->GetShowSequence());
    m_ShowComponentFeats->SetValue(m_Panel->GetShowComponentFeats());

    selected = m_Panel->GetFeatMode();
    if (selected >= 0 && (unsigned int)selected < m_FeatureModeCtrl->GetCount())
        m_FeatureModeCtrl->SetSelection(selected);

    m_ShowASNTypesCtrl->SetValue(m_Panel->GetShowASNTypes());
    m_ShowASNPathToRootCtrl->SetValue(m_Panel->GetShowASNPathToRoot());

    m_HideVariationsCtrl->SetValue(m_Panel->GetHideVariations());
    m_HideStsCtrl->SetValue(m_Panel->GetHideSTS());
    m_OpenExpandedCtrl->SetValue(m_Panel->GetOpenExpanded());

    x_UpdateControls();
}

/*!
 * Should we show tooltips?
 */

bool CTextViewOptionsDlg::ShowToolTips()
{
    return true;
}

/*!
 * Get bitmap resources
 */

wxBitmap CTextViewOptionsDlg::GetBitmapResource( const wxString& name )
{
    // Bitmap retrieval
////@begin CTextViewOptionsDlg bitmap retrieval
    wxUnusedVar(name);
    return wxNullBitmap;
////@end CTextViewOptionsDlg bitmap retrieval
}

/*!
 * Get icon resources
 */

wxIcon CTextViewOptionsDlg::GetIconResource( const wxString& name )
{
    // Icon retrieval
////@begin CTextViewOptionsDlg icon retrieval
    wxUnusedVar(name);
    return wxNullIcon;
////@end CTextViewOptionsDlg icon retrieval
}

void CTextViewOptionsDlg::x_OnApply()
{
    m_Reload = m_Invalidate = false;
    m_SaveState = true;

    int selected = m_ViewTypeCtrl->GetSelection();
    CTextPanel::TViewType viewType = m_Panel->GetViewType();
    if (wxNOT_FOUND != selected) {
        viewType = (CTextPanel::TViewType)(intptr_t)m_ViewTypeCtrl->GetClientData(selected);
        if (viewType != m_Panel->GetViewType()) {
            m_Panel->SetViewType(viewType);
            m_SaveState = false;
            m_Reload = true;
        }
    }

    selected = m_FontSizeCtrl->GetSelection();
    if (wxNOT_FOUND != selected) {
        if (selected != m_Panel->GetFontSize()) {
            m_Panel->SetFontSize(selected);
            m_Reload = true;
        }
    }

    if (viewType == CTextPanel::kFlatFile) {
        selected = m_FlatFileModeCtrl->GetSelection();
        if (wxNOT_FOUND != selected) {
            if (selected != m_Panel->GetFlatFileMode()) {
                m_Panel->SetFlatFileMode(selected);
                m_Reload = true;
                m_SaveState = false;
            }
        }

        if (m_ShowSeq->GetValue() != m_Panel->GetShowSequence()) {
            m_Panel->SetShowSequence(m_ShowSeq->GetValue());
            m_Reload = true;
            m_SaveState = false;
        }

        if (m_ShowComponentFeats->GetValue() != m_Panel->GetShowComponentFeats()) {
            m_Panel->SetShowComponentFeats(m_ShowComponentFeats->GetValue());
            m_Reload = true;
            m_SaveState = false;
        }

        if (m_Panel->ObjectIsFeature()) {
            selected = m_FeatureModeCtrl->GetSelection();
            if (wxNOT_FOUND != selected) {
                if (selected != m_Panel->GetFeatMode()) {
                    m_Panel->SetFeatMode(selected);
                    m_Reload = true;
                }
            }
        }

        if (m_HideVariationsCtrl->IsChecked() != m_Panel->GetHideVariations()) {
            m_Panel->SetHideVariations(m_HideVariationsCtrl->IsChecked());
            m_Reload = true;
            m_SaveState = false;
        }

        if (m_HideStsCtrl->IsChecked() != m_Panel->GetHideSTS()) {
            m_Panel->SetHideSTS(m_HideStsCtrl->IsChecked());
            m_Reload = true;
            m_SaveState = false;
        }

        if (m_OpenExpandedCtrl->IsChecked() != m_Panel->GetOpenExpanded()) {
            m_Panel->SetOpenExpanded(m_OpenExpandedCtrl->IsChecked());
            m_Reload = true;
            m_SaveState = false;
        }
    }

    if (viewType == CTextPanel::kASN) {
        if (m_ShowASNTypesCtrl->IsChecked() != m_Panel->GetShowASNTypes()) {
            m_Panel->SetShowASNTypes(m_ShowASNTypesCtrl->IsChecked());
            m_Invalidate = true;
        }
        if (m_ShowASNPathToRootCtrl->IsChecked() != m_Panel->GetShowASNPathToRoot()) {
            m_Panel->SetShowASNPathToRoot(m_ShowASNPathToRootCtrl->IsChecked());
            m_Invalidate = true;
        }
    }
}

void CTextViewOptionsDlg::OnApplyClick( wxCommandEvent& event )
{
    x_OnApply();
    ApplyChanges();
}

void CTextViewOptionsDlg::ApplyChanges()
{
    if (m_Reload) {
        if (m_SaveState)
            m_Panel->SaveState();
        else
            m_Panel->ClearState();
        m_Panel->Load();
    } else if (m_Invalidate) {
        m_Panel->RefreshView();
    }
    m_Panel->UpdateRegistry();
}

void CTextViewOptionsDlg::OnOkClick( wxCommandEvent& event )
{
    x_OnApply();
    event.Skip();
}

void CTextViewOptionsDlg::OnViewTypeSelected( wxCommandEvent& event )
{
    x_UpdateControls();
}

void CTextViewOptionsDlg::x_UpdateControls()
{
    CTextPanel::TViewType viewType = CTextPanel::kUninitialized;
    int selected = m_ViewTypeCtrl->GetSelection();
    if (wxNOT_FOUND != selected) {
        viewType = (CTextPanel::TViewType)(intptr_t)m_ViewTypeCtrl->GetClientData(selected);
    }

    if (viewType == CTextPanel::kFlatFile) {
        m_FlatFileModeCtrl->Enable(true);
        m_OpenExpandedCtrl->Enable(true);
        m_HideVariationsCtrl->Enable(true);
        m_HideStsCtrl->Enable(true);
        m_ShowSeq->Enable(true);
        m_ShowComponentFeats->Enable(true);
        m_FeatureModeCtrl->Enable(m_Panel->ObjectIsFeature());
    } else {
        m_FlatFileModeCtrl->Enable(false);
        m_OpenExpandedCtrl->Enable(false);
        m_HideVariationsCtrl->Enable(false);
        m_HideStsCtrl->Enable(false);
        m_ShowSeq->Enable(false);
        m_ShowComponentFeats->Enable(false);
        m_FlatFileModeCtrl->Enable(false);
        m_FeatureModeCtrl->Enable(false);
    }

    if (viewType == CTextPanel::kASN) {
        m_ShowASNTypesCtrl->Enable(true);
        m_ShowASNPathToRootCtrl->Enable(true);
    } else {
        m_ShowASNTypesCtrl->Enable(false);
        m_ShowASNPathToRootCtrl->Enable(false);
    }
}

END_NCBI_SCOPE
