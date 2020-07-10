/*  $Id: ngalign_blast_panel.cpp 39079 2017-07-24 20:52:58Z katargir $
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
 * Authors:  Roman Katargin
 *
 * File Description:
 *
 */

#include <ncbi_pch.hpp>

////@begin includes
////@end includes

#include "ngalign_blast_panel.hpp"
#include <gui/widgets/wx/number_validator.hpp>
#include <gui/widgets/wx/float_text_validator.hpp>
#include <gui/widgets/wx/wx_utils.hpp>
#include <gui/widgets/loaders/tax_id_helper.hpp>
#include <gui/widgets/wx/ind_progress_bar.hpp>

#include <gui/widgets/loaders/winmask_files.hpp>

#include <wx/sizer.h>
#include <wx/stattext.h>
#include <wx/statbox.h>
#include <wx/checkbox.h>
#include <wx/choice.h>
#include <wx/button.h>
#include <wx/dialog.h>
#include <wx/msgdlg.h>
#include <wx/app.h>

#include "wx/bitmap.h"
#include "wx/icon.h"

BEGIN_NCBI_SCOPE
USING_SCOPE(ncbi::objects);

IMPLEMENT_DYNAMIC_CLASS( CNGAlignBLASTPanel, CAlgoToolManagerParamsPanel )

BEGIN_EVENT_TABLE( CNGAlignBLASTPanel, CAlgoToolManagerParamsPanel )

////@begin CNGAlignBLASTPanel event table entries
    EVT_BUTTON( ID_BUTTON1, CNGAlignBLASTPanel::OnWMDownload )

////@end CNGAlignBLASTPanel event table entries

END_EVENT_TABLE()

CNGAlignBLASTPanel::CNGAlignBLASTPanel()
: m_data()
, m_AutoDelete(this)
{
    Init();
}

CNGAlignBLASTPanel::CNGAlignBLASTPanel( wxWindow* parent )
: m_AutoDelete(this)
{
    Init();
    Create(parent);
}

bool CNGAlignBLASTPanel::Create( wxWindow* parent, wxWindowID id )
{
    SetExtraStyle(wxWS_EX_VALIDATE_RECURSIVELY|wxWS_EX_BLOCK_EVENTS);
    CAlgoToolManagerParamsPanel::Create( parent, id, wxDefaultPosition, wxSize(0, 0), SYMBOL_CNGALIGNBLASTPANEL_STYLE );

    Hide();

    CreateControls();
    if (GetSizer())
    {
        GetSizer()->SetSizeHints(this);
    }
    Centre();

    return true;
}

CNGAlignBLASTPanel::~CNGAlignBLASTPanel()
{
////@begin CNGAlignBLASTPanel destruction
////@end CNGAlignBLASTPanel destruction
}

void CNGAlignBLASTPanel::Init()
{
////@begin CNGAlignBLASTPanel member initialisation
    m_WMStaticBoxSizer = NULL;
    m_WMStatic = NULL;
    m_WMTaxIds = NULL;
    m_WMDownload = NULL;
////@end CNGAlignBLASTPanel member initialisation
}

void CNGAlignBLASTPanel::CreateControls()
{    
////@begin CNGAlignBLASTPanel content construction
    CNGAlignBLASTPanel* itemCAlgoToolManagerParamsPanel1 = this;

    wxBoxSizer* itemBoxSizer2 = new wxBoxSizer(wxVERTICAL);
    itemCAlgoToolManagerParamsPanel1->SetSizer(itemBoxSizer2);

    wxStaticText* itemStaticText3 = new wxStaticText( itemCAlgoToolManagerParamsPanel1, wxID_STATIC, _("Selected options:"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer2->Add(itemStaticText3, 0, wxALIGN_LEFT|wxALL, 5);

    wxBoxSizer* itemBoxSizer4 = new wxBoxSizer(wxHORIZONTAL);
    itemBoxSizer2->Add(itemBoxSizer4, 0, wxGROW|wxALL, 5);

    wxStaticBox* itemStaticBoxSizer5Static = new wxStaticBox(itemCAlgoToolManagerParamsPanel1, wxID_ANY, _("General Parameters"));
    wxStaticBoxSizer* itemStaticBoxSizer5 = new wxStaticBoxSizer(itemStaticBoxSizer5Static, wxVERTICAL);
    itemBoxSizer4->Add(itemStaticBoxSizer5, 0, wxGROW|wxALL, 5);

    wxFlexGridSizer* itemFlexGridSizer6 = new wxFlexGridSizer(0, 2, 0, 0);
    itemStaticBoxSizer5->Add(itemFlexGridSizer6, 1, wxGROW|wxALL, 5);

    wxStaticText* itemStaticText7 = new wxStaticText( itemStaticBoxSizer5->GetStaticBox(), wxID_STATIC, _("Word size:"), wxDefaultPosition, wxDefaultSize, 0 );
    itemFlexGridSizer6->Add(itemStaticText7, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxTextCtrl* itemTextCtrl8 = new wxTextCtrl( itemStaticBoxSizer5->GetStaticBox(), ID_WORDSIZE, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
    itemFlexGridSizer6->Add(itemTextCtrl8, 0, wxGROW|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxStaticText* itemStaticText9 = new wxStaticText( itemStaticBoxSizer5->GetStaticBox(), wxID_STATIC, _("e-value"), wxDefaultPosition, wxDefaultSize, 0 );
    itemFlexGridSizer6->Add(itemStaticText9, 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxTextCtrl* itemTextCtrl10 = new wxTextCtrl( itemStaticBoxSizer5->GetStaticBox(), ID_E_VALUE, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
    itemFlexGridSizer6->Add(itemTextCtrl10, 0, wxALIGN_CENTER_HORIZONTAL|wxGROW|wxALL, 5);

    itemFlexGridSizer6->AddGrowableCol(1);

    wxCheckBox* itemCheckBox11 = new wxCheckBox( itemStaticBoxSizer5->GetStaticBox(), ID_CHECKBOX13, _("Use BLAST best hit algorithm"), wxDefaultPosition, wxDefaultSize, 0 );
    itemCheckBox11->SetValue(false);
    itemStaticBoxSizer5->Add(itemCheckBox11, 0, wxALIGN_LEFT|wxALL, 5);

    wxStaticBox* itemStaticBoxSizer12Static = new wxStaticBox(itemCAlgoToolManagerParamsPanel1, wxID_ANY, _("Filtering Parameters"));
    wxStaticBoxSizer* itemStaticBoxSizer12 = new wxStaticBoxSizer(itemStaticBoxSizer12Static, wxVERTICAL);
    itemBoxSizer4->Add(itemStaticBoxSizer12, 1, wxGROW|wxALL, 5);

    wxBoxSizer* itemBoxSizer13 = new wxBoxSizer(wxHORIZONTAL);
    itemStaticBoxSizer12->Add(itemBoxSizer13, 0, wxGROW|wxALL, 5);

    wxStaticText* itemStaticText14 = new wxStaticText( itemStaticBoxSizer12->GetStaticBox(), wxID_STATIC, _("Tune search for:"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer13->Add(itemStaticText14, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxArrayString itemChoice15Strings;
    itemChoice15Strings.Add(_("Phase 1 HTG Sequences"));
    itemChoice15Strings.Add(_("Finished clones"));
    itemChoice15Strings.Add(_("Curated genomic sequences(NGs)"));
    wxChoice* itemChoice15 = new wxChoice( itemStaticBoxSizer12->GetStaticBox(), ID_CHOICE5, wxDefaultPosition, wxDefaultSize, itemChoice15Strings, 0 );
    itemBoxSizer13->Add(itemChoice15, 1, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxStaticBox* itemStaticBoxSizer16Static = new wxStaticBox(itemCAlgoToolManagerParamsPanel1, wxID_ANY, _("Windowmasker filtering"));
    m_WMStaticBoxSizer = new wxStaticBoxSizer(itemStaticBoxSizer16Static, wxHORIZONTAL);
    itemBoxSizer2->Add(m_WMStaticBoxSizer, 0, wxGROW|wxALL, 5);

    m_WMStatic = new wxStaticText( m_WMStaticBoxSizer->GetStaticBox(), wxID_STATIC, _("Windowmasker tax IDs:"), wxDefaultPosition, wxDefaultSize, 0 );
    m_WMStaticBoxSizer->Add(m_WMStatic, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxArrayString m_WMTaxIdsStrings;
    m_WMTaxIds = new wxChoice( m_WMStaticBoxSizer->GetStaticBox(), ID_CHOICE6, wxDefaultPosition, wxDefaultSize, m_WMTaxIdsStrings, 0 );
    m_WMStaticBoxSizer->Add(m_WMTaxIds, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    m_WMStaticBoxSizer->Add(7, 8, 1, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    m_WMDownload = new wxButton( m_WMStaticBoxSizer->GetStaticBox(), ID_BUTTON1, _("Configure..."), wxDefaultPosition, wxDefaultSize, 0 );
    m_WMStaticBoxSizer->Add(m_WMDownload, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxStaticBox* itemStaticBoxSizer21Static = new wxStaticBox(itemCAlgoToolManagerParamsPanel1, wxID_ANY, _("Advanced Parameters (standard MegaBLAST parameters are honored)"));
    wxStaticBoxSizer* itemStaticBoxSizer21 = new wxStaticBoxSizer(itemStaticBoxSizer21Static, wxHORIZONTAL);
    itemBoxSizer2->Add(itemStaticBoxSizer21, 0, wxGROW|wxALL, 5);

    wxTextCtrl* itemTextCtrl22 = new wxTextCtrl( itemStaticBoxSizer21->GetStaticBox(), ID_ADV_TEXT, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
    itemStaticBoxSizer21->Add(itemTextCtrl22, 1, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxBoxSizer* itemBoxSizer23 = new wxBoxSizer(wxHORIZONTAL);
    itemBoxSizer2->Add(itemBoxSizer23, 0, wxGROW|wxALL, 5);

    wxStaticText* itemStaticText24 = new wxStaticText( itemCAlgoToolManagerParamsPanel1, wxID_STATIC, _("Job Title:"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer23->Add(itemStaticText24, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxTextCtrl* itemTextCtrl25 = new wxTextCtrl( itemCAlgoToolManagerParamsPanel1, ID_TITLE_TEXT, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer23->Add(itemTextCtrl25, 1, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    // Set validators
    itemTextCtrl8->SetValidator( CNumberValidator(& GetData().m_WordSize, 0, 200) );
    itemTextCtrl10->SetValidator( CFloatTextValidator(& GetData().m_eValue) );
    itemCheckBox11->SetValidator( wxGenericValidator(& GetData().m_BestHit) );
    itemChoice15->SetValidator( wxGenericValidator(& GetData().m_FilterMode) );
    itemTextCtrl22->SetValidator( wxGenericValidator(& GetData().m_AdvParams) );
    itemTextCtrl25->SetValidator( wxGenericValidator(& GetData().m_JobTitle) );
////@end CNGAlignBLASTPanel content construction

    wxStaticText* loadingText = new wxStaticText(m_WMStaticBoxSizer->GetStaticBox(), ID_LOADING_TEXT, wxT("Windomasker tax ID: none. Loading..."), wxDefaultPosition, wxDefaultSize, 0);
    wxFont bold(wxNORMAL_FONT->GetPointSize(), wxNORMAL_FONT->GetFamily(), wxNORMAL_FONT->GetStyle(), wxFONTWEIGHT_BOLD, wxNORMAL_FONT->GetUnderlined(), wxNORMAL_FONT->GetFaceName());
    loadingText->SetFont(bold);
    loadingText->SetForegroundColour(*wxBLACK);
    m_WMStaticBoxSizer->Add(loadingText, 0, wxALIGN_CENTER_VERTICAL | wxALL, 5);
    m_WMStaticBoxSizer->Hide(loadingText);

    m_WMStaticBoxSizer->Add(1, m_WMDownload->GetSize().GetHeight(), 0, wxALIGN_CENTER_VERTICAL | wxALL, 5);

    CIndProgressBar* progress = new CIndProgressBar(m_WMStaticBoxSizer->GetStaticBox(), ID_LOADING_PROGRESS, wxDefaultPosition, 100);
    m_WMStaticBoxSizer->Add(progress, 1, wxALIGN_CENTER_VERTICAL | wxALL, 5);
    m_WMStaticBoxSizer->Hide(progress);
}

bool CNGAlignBLASTPanel::TransferDataToWindow()
{
    x_InitTaxons();
    return CAlgoToolManagerParamsPanel::TransferDataToWindow();
}

void CNGAlignBLASTPanel::x_InitTaxons()
{
    if (CTaxIdHelper::GetInstance().FillControl(this, *m_WMTaxIds, true, GetData().GetWMTaxId()))
        x_ShowWM(true);
    else
        x_ShowWM(false);
}

void CNGAlignBLASTPanel::TaxonsLoaded(bool local)
{
    if (!local) return;
    CTaxIdHelper::GetInstance().FillControl(this, *m_WMTaxIds, true, GetData().GetWMTaxId());
    x_ShowWM(true);
}

void CNGAlignBLASTPanel::x_ShowWM(bool show)
{
    m_WMStaticBoxSizer->Show(FindWindow(ID_LOADING_TEXT), !show);
    m_WMStaticBoxSizer->Show(FindWindow(ID_LOADING_PROGRESS), !show);

    m_WMStaticBoxSizer->Show(m_WMStatic, show);
    m_WMStaticBoxSizer->Show(m_WMTaxIds, show);
    m_WMStaticBoxSizer->Show(m_WMDownload, show);
    GetSizer()->Layout();
}

bool CNGAlignBLASTPanel::TransferDataFromWindow()
{
    int taxId = 0, index = m_WMTaxIds->GetSelection();
    if (wxNOT_FOUND != index)
        taxId = (int)(size_t)m_WMTaxIds->GetClientData(index);
    GetData().SetWMTaxId(taxId);

    if (taxId != 0) {
        CWinMaskerFileStorage& storage = CWinMaskerFileStorage::GetInstance();
        string wmDir(storage.GetPath().ToUTF8());
        GetData().SetWMDir(wmDir);
    }

    return CAlgoToolManagerParamsPanel::TransferDataFromWindow();
}

bool CNGAlignBLASTPanel::ShowToolTips()
{
    return true;
}
wxBitmap CNGAlignBLASTPanel::GetBitmapResource( const wxString& name )
{
    // Bitmap retrieval
////@begin CNGAlignBLASTPanel bitmap retrieval
    wxUnusedVar(name);
    return wxNullBitmap;
////@end CNGAlignBLASTPanel bitmap retrieval
}
wxIcon CNGAlignBLASTPanel::GetIconResource( const wxString& name )
{
    // Icon retrieval
////@begin CNGAlignBLASTPanel icon retrieval
    wxUnusedVar(name);
    return wxNullIcon;
////@end CNGAlignBLASTPanel icon retrieval
}

void CNGAlignBLASTPanel::SaveSettings() const
{
    if (!m_RegPath.empty()) {
        //CGuiRegistry& gui_reg = CGuiRegistry::GetInstance();
    }
}

void CNGAlignBLASTPanel::LoadSettings()
{
    if (!m_RegPath.empty()) {
        //CGuiRegistry& gui_reg = CGuiRegistry::GetInstance();
    }
}

void CNGAlignBLASTPanel::OnWMDownload(wxCommandEvent& event)
{
    CWinMaskerFileStorage::GetInstance().ShowOptionsDlg();
    x_InitTaxons();
}

void CNGAlignBLASTPanel::RestoreDefaults()
{
    GetData().Init();
    TransferDataToWindow();
}

END_NCBI_SCOPE
