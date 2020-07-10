/*$Id: blast_search_params_panel.cpp 39076 2017-07-24 20:46:06Z katargir $
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
 * Authors: Andrey Yazhuk
 *
 * File Description:
 *
 */

#include <ncbi_pch.hpp>

#include <gui/packages/pkg_alignment/blast_search_params_panel.hpp>
#include <gui/packages/pkg_alignment/blast_search_params.hpp>

#include <gui/widgets/object_list/object_list_widget.hpp>

#include <gui/widgets/wx/wx_utils.hpp>
#include <gui/widgets/wx/number_validator.hpp>
#include <gui/widgets/wx/float_text_validator.hpp>
#include <gui/widgets/wx/async_call.hpp>
#include <gui/widgets/loaders/tax_id_helper.hpp>
#include <gui/widgets/wx/ind_progress_bar.hpp>
#include <gui/widgets/loaders/winmask_files.hpp>

#include <objtools/blast/services/blast_services.hpp>

#include <wx/sizer.h>
#include <wx/button.h>
#include <wx/image.h>
#include <wx/statbox.h>
#include <wx/stattext.h>
#include <wx/checkbox.h>
#include <wx/combobox.h>
#include <wx/choice.h>
#include <wx/msgdlg.h>



////@begin XPM images
////@end XPM images

BEGIN_NCBI_SCOPE


IMPLEMENT_DYNAMIC_CLASS( CBLASTSearchParamsPanel, CAlgoToolManagerParamsPanel )

BEGIN_EVENT_TABLE( CBLASTSearchParamsPanel, CAlgoToolManagerParamsPanel )
////@begin CBLASTSearchParamsPanel event table entries
    EVT_CHOICE( ID_REPEAT_TYPE, CBLASTSearchParamsPanel::OnRepeatTypeSelected )

    EVT_CHOICE( ID_WMTI_CHOICE, CBLASTSearchParamsPanel::OnWindowmaskerTaxIdSelected )

    EVT_BUTTON( ID_BUTTON, CBLASTSearchParamsPanel::OnWMDownload )

    EVT_CHECKBOX( ID_STANDALONE, CBLASTSearchParamsPanel::OnStandaloneClick )

////@end CBLASTSearchParamsPanel event table entries
END_EVENT_TABLE()


CBLASTSearchParamsPanel::CBLASTSearchParamsPanel()
    : m_AutoDelete(this)
{
    Init();
}


CBLASTSearchParamsPanel::CBLASTSearchParamsPanel(
    wxWindow* parent, 
    wxWindowID id, 
    const wxPoint& pos, 
    const wxSize& size, 
    long style)
    : m_AutoDelete(this)
{
    Init();
    Create(parent, id, pos, size, style);
}


bool CBLASTSearchParamsPanel::Create( wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style )
{
    wxPanel::Create( parent, id, pos, size, style );

    Hide();

    CreateControls();
    if (GetSizer())
    {
        GetSizer()->SetSizeHints(this);
    }
    Centre();

    return true;
}


CBLASTSearchParamsPanel::~CBLASTSearchParamsPanel()
{
////@begin CBLASTSearchParamsPanel destruction
////@end CBLASTSearchParamsPanel destruction
}

void CBLASTSearchParamsPanel::Init()
{
////@begin CBLASTSearchParamsPanel member initialisation
    m_GeneralSizer = NULL;
    m_ThreshLabel = NULL;
    m_ThreshInput = NULL;
    m_QueryGenCodeLabel = NULL;
    m_QueryGenCodeCombo = NULL;
    m_DbGenCodeLabel = NULL;
    m_DbGenCodeCombo = NULL;
    m_RepeatType = NULL;
    m_LCRegionsCheck = NULL;
    m_RepeatsCheck = NULL;
    m_WMStaticBoxSizer = NULL;
    m_WMStatic = NULL;
    m_WMTaxIds = NULL;
    m_WMDownload = NULL;
    m_CompartCheck = NULL;
    m_chbStandalone = NULL;
////@end CBLASTSearchParamsPanel member initialisation

    m_Params = NULL;
}


void CBLASTSearchParamsPanel::CreateControls()
{
////@begin CBLASTSearchParamsPanel content construction
    CBLASTSearchParamsPanel* itemCAlgoToolManagerParamsPanel1 = this;

    wxBoxSizer* itemBoxSizer2 = new wxBoxSizer(wxVERTICAL);
    itemCAlgoToolManagerParamsPanel1->SetSizer(itemBoxSizer2);

    wxStaticText* itemStaticText3 = new wxStaticText( itemCAlgoToolManagerParamsPanel1, wxID_STATIC, _("Selected options:"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer2->Add(itemStaticText3, 0, wxALIGN_LEFT|wxALL, 5);

    wxBoxSizer* itemBoxSizer4 = new wxBoxSizer(wxHORIZONTAL);
    itemBoxSizer2->Add(itemBoxSizer4, 0, wxGROW|wxALL, 5);

    wxStaticBox* itemStaticBoxSizer5Static = new wxStaticBox(itemCAlgoToolManagerParamsPanel1, wxID_ANY, _("General Parameters"));
    wxStaticBoxSizer* itemStaticBoxSizer5 = new wxStaticBoxSizer(itemStaticBoxSizer5Static, wxVERTICAL);
    itemBoxSizer4->Add(itemStaticBoxSizer5, 1, wxGROW|wxALL, 5);

    m_GeneralSizer = new wxFlexGridSizer(0, 2, 0, 0);
    itemStaticBoxSizer5->Add(m_GeneralSizer, 1, wxGROW, 5);

    wxStaticText* itemStaticText7 = new wxStaticText( itemCAlgoToolManagerParamsPanel1, wxID_STATIC, _("Word size:"), wxDefaultPosition, wxDefaultSize, 0 );
    m_GeneralSizer->Add(itemStaticText7, 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxTextCtrl* itemTextCtrl8 = new wxTextCtrl( itemCAlgoToolManagerParamsPanel1, ID_WORDSIZE, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
    m_GeneralSizer->Add(itemTextCtrl8, 0, wxALIGN_CENTER_HORIZONTAL|wxGROW|wxALL, 5);

    wxStaticText* itemStaticText9 = new wxStaticText( itemCAlgoToolManagerParamsPanel1, wxID_STATIC, _("e-value:"), wxDefaultPosition, wxDefaultSize, 0 );
    m_GeneralSizer->Add(itemStaticText9, 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxTextCtrl* itemTextCtrl10 = new wxTextCtrl( itemCAlgoToolManagerParamsPanel1, ID_E_VALUE, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
    m_GeneralSizer->Add(itemTextCtrl10, 0, wxALIGN_CENTER_HORIZONTAL|wxGROW|wxALL, 5);

    m_ThreshLabel = new wxStaticText( itemCAlgoToolManagerParamsPanel1, wxID_STATIC, _("Threshold:"), wxDefaultPosition, wxDefaultSize, 0 );
    m_GeneralSizer->Add(m_ThreshLabel, 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    m_ThreshInput = new wxTextCtrl( itemCAlgoToolManagerParamsPanel1, ID_THRESHOLD, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
    m_GeneralSizer->Add(m_ThreshInput, 0, wxALIGN_CENTER_HORIZONTAL|wxGROW|wxALL, 5);

    m_QueryGenCodeLabel = new wxStaticText( itemCAlgoToolManagerParamsPanel1, wxID_ANY, _("Query genetic code:"), wxDefaultPosition, wxDefaultSize, 0 );
    m_GeneralSizer->Add(m_QueryGenCodeLabel, 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxArrayString m_QueryGenCodeComboStrings;
    m_QueryGenCodeCombo = new wxComboBox( itemCAlgoToolManagerParamsPanel1, ID_GEN_CODE, wxEmptyString, wxDefaultPosition, wxSize(itemCAlgoToolManagerParamsPanel1->ConvertDialogToPixels(wxSize(100, -1)).x, -1), m_QueryGenCodeComboStrings, wxCB_READONLY );
    m_GeneralSizer->Add(m_QueryGenCodeCombo, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    m_DbGenCodeLabel = new wxStaticText( itemCAlgoToolManagerParamsPanel1, wxID_ANY, _("DB genetic code:"), wxDefaultPosition, wxDefaultSize, 0 );
    m_GeneralSizer->Add(m_DbGenCodeLabel, 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxArrayString m_DbGenCodeComboStrings;
    m_DbGenCodeCombo = new wxComboBox( itemCAlgoToolManagerParamsPanel1, ID_COMBOBOX, wxEmptyString, wxDefaultPosition, wxSize(itemCAlgoToolManagerParamsPanel1->ConvertDialogToPixels(wxSize(100, -1)).x, -1), m_DbGenCodeComboStrings, wxCB_READONLY );
    m_GeneralSizer->Add(m_DbGenCodeCombo, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    m_GeneralSizer->AddGrowableCol(1);

    wxBoxSizer* itemBoxSizer17 = new wxBoxSizer(wxVERTICAL);
    itemBoxSizer4->Add(itemBoxSizer17, 1, wxALIGN_TOP, 5);

    wxStaticBox* itemStaticBoxSizer18Static = new wxStaticBox(itemCAlgoToolManagerParamsPanel1, wxID_ANY, _("Filtering Parameters"));
    wxStaticBoxSizer* itemStaticBoxSizer18 = new wxStaticBoxSizer(itemStaticBoxSizer18Static, wxVERTICAL);
    itemBoxSizer17->Add(itemStaticBoxSizer18, 0, wxGROW|wxALL, 5);

    wxBoxSizer* itemBoxSizer19 = new wxBoxSizer(wxVERTICAL);
    itemStaticBoxSizer18->Add(itemBoxSizer19, 1, wxGROW, 5);

    wxCheckBox* itemCheckBox20 = new wxCheckBox( itemCAlgoToolManagerParamsPanel1, ID_LOW_COMPLEXITY, _("Filter low complexity regions"), wxDefaultPosition, wxDefaultSize, 0 );
    itemCheckBox20->SetValue(false);
    itemBoxSizer19->Add(itemCheckBox20, 0, wxALIGN_LEFT|wxALL, 5);

    wxStaticText* itemStaticText21 = new wxStaticText( itemCAlgoToolManagerParamsPanel1, ID_STATIC_REPEAT_TYPE, _("Species specific repeats for:"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer19->Add(itemStaticText21, 0, wxALIGN_LEFT|wxALL, 5);

    wxArrayString m_RepeatTypeStrings;
    m_RepeatType = new wxChoice( itemCAlgoToolManagerParamsPanel1, ID_REPEAT_TYPE, wxDefaultPosition, wxDefaultSize, m_RepeatTypeStrings, 0 );
    itemBoxSizer19->Add(m_RepeatType, 0, wxALIGN_LEFT|wxALL, 5);

    m_LCRegionsCheck = new wxCheckBox( itemCAlgoToolManagerParamsPanel1, ID_MASK_LOWERCASE, _("Mask lowercase regions"), wxDefaultPosition, wxDefaultSize, 0 );
    m_LCRegionsCheck->SetValue(false);
    itemBoxSizer19->Add(m_LCRegionsCheck, 0, wxALIGN_LEFT|wxALL, 5);

    m_RepeatsCheck = new wxCheckBox( itemCAlgoToolManagerParamsPanel1, ID_MASK_REPEATS, _("Mask repeats"), wxDefaultPosition, wxDefaultSize, 0 );
    m_RepeatsCheck->SetValue(false);
    itemBoxSizer19->Add(m_RepeatsCheck, 0, wxALIGN_LEFT|wxALL, 5);

    wxBoxSizer* itemBoxSizer25 = new wxBoxSizer(wxVERTICAL);
    itemBoxSizer2->Add(itemBoxSizer25, 0, wxGROW|wxALL, 5);

    wxStaticText* itemStaticText26 = new wxStaticText( itemCAlgoToolManagerParamsPanel1, wxID_STATIC, _("BLAST algorithm requires at most one filtering database. It may be either "), wxDefaultPosition, wxDefaultSize, 0 );
    itemStaticText26->Wrap(500);
    itemBoxSizer25->Add(itemStaticText26, 0, wxALIGN_LEFT|wxLEFT|wxRIGHT, 5);

    wxStaticText* itemStaticText27 = new wxStaticText( itemCAlgoToolManagerParamsPanel1, wxID_STATIC, _("Species Specific Repeats database above or Windowmasker Filtering below."), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer25->Add(itemStaticText27, 0, wxALIGN_LEFT|wxLEFT|wxRIGHT, 5);

    wxStaticBox* itemStaticBoxSizer28Static = new wxStaticBox(itemCAlgoToolManagerParamsPanel1, wxID_ANY, _("Windowmasker filtering"));
    m_WMStaticBoxSizer = new wxStaticBoxSizer(itemStaticBoxSizer28Static, wxHORIZONTAL);
    itemBoxSizer2->Add(m_WMStaticBoxSizer, 0, wxGROW|wxALL, 5);

    m_WMStatic = new wxStaticText( itemCAlgoToolManagerParamsPanel1, wxID_STATIC, _("Windowmasker tax ID:"), wxDefaultPosition, wxDefaultSize, 0 );
    m_WMStaticBoxSizer->Add(m_WMStatic, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxArrayString m_WMTaxIdsStrings;
    m_WMTaxIds = new wxChoice( itemCAlgoToolManagerParamsPanel1, ID_WMTI_CHOICE, wxDefaultPosition, wxDefaultSize, m_WMTaxIdsStrings, 0 );
    m_WMStaticBoxSizer->Add(m_WMTaxIds, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    m_WMStaticBoxSizer->Add(7, 8, 1, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    m_WMDownload = new wxButton( itemCAlgoToolManagerParamsPanel1, ID_BUTTON, _("Configure..."), wxDefaultPosition, wxDefaultSize, 0 );
    m_WMStaticBoxSizer->Add(m_WMDownload, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxStaticBox* itemStaticBoxSizer33Static = new wxStaticBox(itemCAlgoToolManagerParamsPanel1, wxID_ANY, _("Advanced Parameters"));
    wxStaticBoxSizer* itemStaticBoxSizer33 = new wxStaticBoxSizer(itemStaticBoxSizer33Static, wxHORIZONTAL);
    itemBoxSizer2->Add(itemStaticBoxSizer33, 0, wxGROW|wxALL, 5);

    wxTextCtrl* itemTextCtrl34 = new wxTextCtrl( itemCAlgoToolManagerParamsPanel1, ID_ADV_TEXT, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
    itemStaticBoxSizer33->Add(itemTextCtrl34, 1, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    m_CompartCheck = new wxCheckBox( itemCAlgoToolManagerParamsPanel1, ID_COMPARTMENTS, _("Link related hits together"), wxDefaultPosition, wxDefaultSize, 0 );
    m_CompartCheck->SetValue(false);
    if (CBLASTSearchParamsPanel::ShowToolTips())
        m_CompartCheck->SetToolTip(_("BLAST returns separate alignments for each query, and these separate alignments \ncan further be ordered into sets offering consistent non-overlapping query and subject \ncoverage.  The sequence viewer offers the ability to evaluate the original BLAST hits \non-the-fly and link together alignments that meet a strict definition of non-overlapping \nquery and subject coverage.\n"));
    itemBoxSizer2->Add(m_CompartCheck, 0, wxALIGN_LEFT|wxLEFT|wxRIGHT|wxBOTTOM, 15);

    wxBoxSizer* itemBoxSizer36 = new wxBoxSizer(wxHORIZONTAL);
    itemBoxSizer2->Add(itemBoxSizer36, 0, wxGROW|wxALL, 5);

    wxStaticText* itemStaticText37 = new wxStaticText( itemCAlgoToolManagerParamsPanel1, wxID_STATIC, _("Job Title:"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer36->Add(itemStaticText37, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxTextCtrl* itemTextCtrl38 = new wxTextCtrl( itemCAlgoToolManagerParamsPanel1, ID_TITLE_TEXT, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer36->Add(itemTextCtrl38, 1, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    itemBoxSizer2->Add(7, 8, 1, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

    m_chbStandalone = new wxCheckBox( itemCAlgoToolManagerParamsPanel1, ID_STANDALONE, _("Run tool as standalone"), wxDefaultPosition, wxDefaultSize, 0 );
    m_chbStandalone->SetValue(false);
    itemBoxSizer2->Add(m_chbStandalone, 0, wxALIGN_LEFT|wxALL, 15);

    // Set validators
    itemTextCtrl8->SetValidator( CNumberValidator(& m_WordSize, 0, 100) );
    itemTextCtrl10->SetValidator( CFloatTextValidator(& m_eValue) );
    m_ThreshInput->SetValidator( CNumberValidator(& m_Threshold, 0, 100) );
    m_QueryGenCodeCombo->SetValidator( wxGenericValidator(& m_QueryGeneticCode) );
    m_DbGenCodeCombo->SetValidator( wxGenericValidator(& m_DbGeneticCode) );
    itemCheckBox20->SetValidator( wxGenericValidator(& m_FilterLowComplex) );
    m_LCRegionsCheck->SetValidator( wxGenericValidator(& m_MaskLCRegions) );
    m_RepeatsCheck->SetValidator( wxGenericValidator(& m_MaskRepeats) );
    itemTextCtrl34->SetValidator( wxGenericValidator(& m_AdvParams) );
    itemTextCtrl38->SetValidator( wxGenericValidator(& m_JobTitle) );
////@end CBLASTSearchParamsPanel content construction

    x_InitGeneticCodesCombo();

    wxStaticText* loadingText = new wxStaticText(this, ID_LOADING_TEXT, wxT("Windomasker tax ID: none. Loading..."), wxDefaultPosition, wxDefaultSize, 0);
    wxFont bold(wxNORMAL_FONT->GetPointSize(), wxNORMAL_FONT->GetFamily(), wxNORMAL_FONT->GetStyle(), wxFONTWEIGHT_BOLD, wxNORMAL_FONT->GetUnderlined(), wxNORMAL_FONT->GetFaceName());
    loadingText->SetFont(bold);
    loadingText->SetForegroundColour(*wxBLACK);
    m_WMStaticBoxSizer->Add(loadingText, 0, wxALIGN_CENTER_VERTICAL | wxALL, 5);
    m_WMStaticBoxSizer->Hide(loadingText);

    m_WMStaticBoxSizer->Add(1, m_WMDownload->GetSize().GetHeight(), 0, wxALIGN_CENTER_VERTICAL | wxALL, 5);

    CIndProgressBar* progress = new CIndProgressBar(this, ID_LOADING_PROGRESS, wxDefaultPosition, 100);
    m_WMStaticBoxSizer->Add(progress, 1, wxALIGN_CENTER_VERTICAL | wxALL, 5);
    m_WMStaticBoxSizer->Hide(progress);

    m_RepeatType->Append(wxT("none"));
    CBLASTParams::TRepeatLibs libs = CBLASTParams::GetAllRepeatLibs();
    ITERATE (CBLASTParams::TRepeatLibs, iter, libs) {
        m_RepeatType->Append(ToWxString(*iter));
    }
}


void CBLASTSearchParamsPanel::x_InitGeneticCodesCombo()
{
    vector<string> labels;
    CBLASTParams::GetGeneticCodeLabels(labels);

    wxArrayString arr;
    ToArrayString(labels, arr);

    m_QueryGenCodeCombo->Clear();
    m_QueryGenCodeCombo->Append(arr);

    m_DbGenCodeCombo->Clear();
    m_DbGenCodeCombo->Append(arr);

}


bool CBLASTSearchParamsPanel::ShowToolTips()
{
    return true;
}


wxBitmap CBLASTSearchParamsPanel::GetBitmapResource( const wxString& name )
{
    // Bitmap retrieval
////@begin CBLASTSearchParamsPanel bitmap retrieval
    wxUnusedVar(name);
    return wxNullBitmap;
////@end CBLASTSearchParamsPanel bitmap retrieval
}


wxIcon CBLASTSearchParamsPanel::GetIconResource( const wxString& name )
{
    // Icon retrieval
////@begin CBLASTSearchParamsPanel icon retrieval
    wxUnusedVar(name);
    return wxNullIcon;
////@end CBLASTSearchParamsPanel icon retrieval
}


void CBLASTSearchParamsPanel::SetParams (CBLASTParams* params)
{
    m_Params = params;
}


// hide / show some of the controls depending on the program
void CBLASTSearchParamsPanel::x_HideShowInputs()
{
    blast::EProgram prg = m_Params->GetCurrProgram();

    bool en_query_gc = CBLASTParams::NeedsQueryGeneticCode(prg);
    bool en_db_gc = CBLASTParams::NeedsDbGeneticCode(prg);
    bool en_thresh = CBLASTParams::NeedsThreshold(prg);

    m_ThreshLabel->Show(en_thresh);
    m_ThreshInput->Show(en_thresh);

    m_QueryGenCodeLabel->Show(en_query_gc);
    m_QueryGenCodeCombo->Show(en_query_gc);

    m_DbGenCodeLabel->Show(en_db_gc);
    m_DbGenCodeCombo->Show(en_db_gc);

    if (m_Params->GetSubjectType() != CBLASTParams::eSequences) {
        m_LCRegionsCheck->Show(false);
        m_RepeatsCheck->Show(false);
    }
    else {
        m_LCRegionsCheck->Show(true);
        m_RepeatsCheck->Show(true);
    }

    m_chbStandalone->Show(m_Params->GetSubjectType() == CBLASTParams::eSequences);
    x_UpdateMaskCtrls();

    m_RepeatType->Show(!m_Local);
    FindWindow(ID_STATIC_REPEAT_TYPE)->Show(!m_Local);

    GetSizer()->Show(m_WMStaticBoxSizer, m_Params->IsNucInput());
    m_WMDownload->Enable(m_Local);

    Layout();
}

bool CBLASTSearchParamsPanel::TransferDataToWindow()
{
    switch (m_Params->GetSubjectType()) {
    case CBLASTParams::eNCBIDB:
        m_Local = false;
        break;
    default:
        m_Local = true;
        break;
    }

    blast::EProgram prg = m_Params->GetCurrProgram();
    CBLASTParams::SProgParams& prg_params =  m_Params->GetCurrParams();

    if(CBLASTParams::NeedsThreshold(prg)) {
        m_Threshold = prg_params.m_Threshold;
    }

    m_WordSize = prg_params.m_WordSize;
    m_eValue = prg_params.m_eValue;
    m_FilterLowComplex = prg_params.m_FilterLowComplex;

    if(CBLASTParams::NeedsQueryGeneticCode(prg)) {
        int code_id = prg_params.m_QueryGeneticCode;
        m_QueryGeneticCode = ToWxString(CBLASTParams::GetGeneticCodeLabel(code_id));
    }

    if(CBLASTParams::NeedsDbGeneticCode(prg)) {
        int code_id = prg_params.m_DbGeneticCode;
        m_DbGeneticCode = ToWxString(CBLASTParams::GetGeneticCodeLabel(code_id));
    }

    if(m_Local) {
        m_MaskLCRegions = prg_params.m_MaskLowercase;
        m_MaskRepeats = prg_params.m_MaskRepeats;
    }

    int pos = m_RepeatType->FindString(ToWxString(prg_params.m_RepeatLib));
    if (pos == wxNOT_FOUND) {
        pos = m_RepeatType->FindString(wxT("none"));
    }
    if (pos != wxNOT_FOUND) {
        m_RepeatType->SetSelection(pos);
    }

    m_AdvParams = ToWxString(m_Params->GetAdvancedParams());
    m_JobTitle = ToWxString(m_Params->GetJobTitle());

    if (m_Params->GetSubjectType() == CBLASTParams::eSequences)
        m_chbStandalone->SetValue (m_Params->IsStandaloneRequested());

    x_HideShowInputs();

    m_CompartCheck->SetValue( m_Params->IsCompartmentsRequested() );

    if (m_Params->IsNucInput())
        x_InitTaxons();

    return wxPanel::TransferDataToWindow();
}


void CBLASTSearchParamsPanel::x_InitTaxons()
{
    if (m_Local) {
        int selectTaxId = m_Params->GetCurrParams().m_WM_TaxId;
        if (CTaxIdHelper::GetInstance().FillControl(this, *m_WMTaxIds, true, selectTaxId)) {
            x_ShowWM(true);
            return;
        }
    }
    else {
        int selectTaxId = (m_RepeatType->GetSelection() != 0) ?
            0 : m_Params->GetCurrParams().m_WM_TaxId;
        if (CTaxIdHelper::GetInstance().FillControl(this, *m_WMTaxIds, false, selectTaxId)) {
            x_ShowWM(true);
            return;
        }
    }
    x_ShowWM(false);
}

void CBLASTSearchParamsPanel::TaxonsLoaded(bool local)
{
    if (m_Local && local) {
        int selectTaxId = m_Params->GetCurrParams().m_WM_TaxId;
        CTaxIdHelper::GetInstance().FillControl(this, *m_WMTaxIds, true, selectTaxId);
        x_ShowWM(true);
    }
    if (!m_Local && !local) {
        int selectTaxId = (m_RepeatType->GetSelection() != 0) ?
            0 : m_Params->GetCurrParams().m_WM_TaxId;
        CTaxIdHelper::GetInstance().FillControl(this, *m_WMTaxIds, false, selectTaxId);
        x_ShowWM(true);
    }
}

void CBLASTSearchParamsPanel::x_ShowWM(bool show)
{
    m_WMStaticBoxSizer->Show(FindWindow(ID_LOADING_TEXT), !show);
    m_WMStaticBoxSizer->Show(FindWindow(ID_LOADING_PROGRESS), !show);

    m_WMStaticBoxSizer->Show(m_WMStatic, show);
    m_WMStaticBoxSizer->Show(m_WMTaxIds, show);
    m_WMStaticBoxSizer->Show(m_WMDownload, show);
    GetSizer()->Layout();
}

bool CBLASTSearchParamsPanel::TransferDataFromWindow()
{
    if(wxPanel::TransferDataFromWindow())   {
        blast::EProgram prg = m_Params->GetCurrProgram();
        CBLASTParams::SProgParams& prg_params =  m_Params->GetCurrParams();

        if(m_Local) {
            prg_params.m_MaskLowercase = m_MaskLCRegions;
            prg_params.m_MaskRepeats = m_MaskRepeats;
        }

        if(CBLASTParams::NeedsThreshold(prg)) {
            prg_params.m_Threshold = m_Threshold;
        }

        prg_params.m_WordSize = m_WordSize;
        prg_params.m_eValue = m_eValue;
        prg_params.m_FilterLowComplex = m_FilterLowComplex;

        if(CBLASTParams::NeedsQueryGeneticCode(prg)) {
            string s = ToStdString(m_QueryGeneticCode);
            prg_params.m_QueryGeneticCode = CBLASTParams::GetGeneticCodeByLabel(s);
        }

        if(CBLASTParams::NeedsDbGeneticCode(prg)) {
            string s = ToStdString(m_DbGeneticCode);
            prg_params.m_DbGeneticCode = CBLASTParams::GetGeneticCodeByLabel(s);
        }

        prg_params.m_RepeatLib =
            m_Local ? "none" : ToStdString(m_RepeatType->GetStringSelection());
        m_Params->GetAdvancedParams() = ToStdString(m_AdvParams);
        m_Params->GetJobTitle() = ToStdString(m_JobTitle);

        if (m_Params->GetSubjectType() == CBLASTParams::eSequences)
            m_Params->StandaloneRequested( m_chbStandalone->GetValue());

        m_Params->FindCompartments( m_CompartCheck->GetValue() );

        int taxId = 0, index = m_WMTaxIds->GetSelection();
        if (wxNOT_FOUND != index)
            taxId = (int)(intptr_t)m_WMTaxIds->GetClientData(index);
        prg_params.m_WM_TaxId = taxId;

        if( taxId != 0 ){
            CWinMaskerFileStorage& storage = CWinMaskerFileStorage::GetInstance();
            m_Params->SetWM_Dir( ToStdString(storage.GetPath()) );
        }

        return true;
    }
    return false;
}


void CBLASTSearchParamsPanel::OnRepeatTypeSelected( wxCommandEvent& event )
{
    if (!m_Local && event.GetSelection() != 0)
        m_WMTaxIds->Select(0);
}

void CBLASTSearchParamsPanel::OnWindowmaskerTaxIdSelected( wxCommandEvent& event )
{
    if (!m_Local && event.GetSelection() != 0)
        m_RepeatType->Select(0);
}


void CBLASTSearchParamsPanel::OnWMDownload(wxCommandEvent& event)
{
    CWinMaskerFileStorage::GetInstance().ShowOptionsDlg();
    x_InitTaxons();
}

void CBLASTSearchParamsPanel::RestoreDefaults()
{
    if (!m_Params)
        return;
    m_Params->ResetCurrParams();
    wxBusyCursor wait;
    TransferDataToWindow();
}

void CBLASTSearchParamsPanel::x_UpdateMaskCtrls()
{
    if (m_chbStandalone->GetValue()) {
        m_LCRegionsCheck->SetValue(false);
        m_RepeatsCheck->SetValue(false);
        m_LCRegionsCheck->Enable(false);
        m_RepeatsCheck->Enable(false);
    } else {
        m_LCRegionsCheck->Enable(true);
        m_RepeatsCheck->Enable(true);
    }
}

void CBLASTSearchParamsPanel::OnStandaloneClick( wxCommandEvent& )
{
    x_UpdateMaskCtrls();
}

END_NCBI_SCOPE
