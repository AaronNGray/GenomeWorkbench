/*  $Id: wx_score_dlg.cpp 30962 2014-08-13 18:38:46Z falkrb $
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

#include <gui/widgets/hit_matrix/wx_score_dlg.hpp>
#include <gui/widgets/hit_matrix/gradient_panel.hpp>

#include <gui/widgets/wx/wx_utils.hpp>
#include <gui/widgets/wx/float_text_validator.hpp>

#include <wx/listbox.h>
#include <wx/msgdlg.h>
#include <wx/checkbox.h>
#include <wx/textctrl.h>
#include <wx/clrpicker.h>
#include <wx/stattext.h>
#include <wx/statline.h>


BEGIN_NCBI_SCOPE


IMPLEMENT_DYNAMIC_CLASS( CwxScoreDlg, wxDialog )


BEGIN_EVENT_TABLE( CwxScoreDlg, wxDialog )
////@begin CwxScoreDlg event table entries
    EVT_CHECKBOX( ID_CHECKBOX, CwxScoreDlg::OnCheckboxClick )

    EVT_LISTBOX( ID_LISTBOX, CwxScoreDlg::OnListboxSelected )

    EVT_CHECKBOX( ID_MIN_GRAD_CHECK, CwxScoreDlg::OnMinGradCheckClick )

    EVT_CHECKBOX( ID_MAX_GRAD_CHECK, CwxScoreDlg::OnMaxGradCheckClick )

    EVT_COLOURPICKER_CHANGED( ID_MIN_COLOR_BTN, CwxScoreDlg::OnMinColorBtnColourPickerChanged )

    EVT_COLOURPICKER_CHANGED( ID_MAX_COLOR_BTN, CwxScoreDlg::OnMaxColorBtnColourPickerChanged )

    EVT_CHECKBOX( ID_CHECKBOX1, CwxScoreDlg::OnCheckbox1Click )

    EVT_BUTTON( wxID_OK, CwxScoreDlg::OnOkClick )

////@end CwxScoreDlg event table entries
END_EVENT_TABLE()


CwxScoreDlg::CwxScoreDlg()
{
    Init();
}


CwxScoreDlg::CwxScoreDlg( wxWindow* parent, wxWindowID id, const wxString& caption, const wxPoint& pos, const wxSize& size, long style )
{
    Init();
    Create(parent, id, caption, pos, size, style);
}


bool CwxScoreDlg::Create( wxWindow* parent, wxWindowID id, const wxString& caption, const wxPoint& pos, const wxSize& size, long style )
{
////@begin CwxScoreDlg creation
    SetExtraStyle(wxWS_EX_BLOCK_EVENTS);
    wxDialog::Create( parent, id, caption, pos, size, style );

    CreateControls();
    if (GetSizer())
    {
        GetSizer()->SetSizeHints(this);
    }
    Centre();
////@end CwxScoreDlg creation
    return true;
}


CwxScoreDlg::~CwxScoreDlg()
{
////@begin CwxScoreDlg destruction
////@end CwxScoreDlg destruction
}


void CwxScoreDlg::Init()
{
////@begin CwxScoreDlg member initialisation
    m_EnableCheck = NULL;
    m_List = NULL;
    m_MinValueText = NULL;
    m_MaxValueText = NULL;
    m_MinGradCheck = NULL;
    m_MinGradInput = NULL;
    m_MaxGradCheck = NULL;
    m_MaxGradInput = NULL;
    m_MinColorBtn = NULL;
    m_MaxColorBtn = NULL;
    m_LogCheck = NULL;
    m_GradPanel = NULL;
////@end CwxScoreDlg member initialisation
    m_CurrIndex = 0;
    m_ScoreSel.Add(m_CurrIndex);
    mf_FinalCopy = false;
}


void CwxScoreDlg::CreateControls()
{
////@begin CwxScoreDlg content construction
    CwxScoreDlg* itemDialog1 = this;

    wxBoxSizer* itemBoxSizer2 = new wxBoxSizer(wxVERTICAL);
    itemDialog1->SetSizer(itemBoxSizer2);

    wxFlexGridSizer* itemFlexGridSizer3 = new wxFlexGridSizer(0, 2, 0, 0);
    itemBoxSizer2->Add(itemFlexGridSizer3, 1, wxGROW|wxALL, 5);

    itemFlexGridSizer3->Add(5, 5, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxBoxSizer* itemBoxSizer5 = new wxBoxSizer(wxVERTICAL);
    itemFlexGridSizer3->Add(itemBoxSizer5, 0, wxGROW|wxALIGN_CENTER_VERTICAL, 5);

    m_EnableCheck = new wxCheckBox( itemDialog1, ID_CHECKBOX, _("Enable coloring by Score"), wxDefaultPosition, wxDefaultSize, 0 );
    m_EnableCheck->SetValue(false);
    itemBoxSizer5->Add(m_EnableCheck, 0, wxALIGN_LEFT|wxALL, 5);

    wxStaticLine* itemStaticLine7 = new wxStaticLine( itemDialog1, wxID_STATIC, wxDefaultPosition, wxDefaultSize, wxLI_HORIZONTAL );
    itemBoxSizer5->Add(itemStaticLine7, 0, wxGROW|wxALL, 5);

    wxStaticText* itemStaticText8 = new wxStaticText( itemDialog1, wxID_STATIC, _("Score"), wxDefaultPosition, wxDefaultSize, 0 );
    itemFlexGridSizer3->Add(itemStaticText8, 0, wxALIGN_RIGHT|wxALIGN_TOP|wxALL, 5);

    wxArrayString m_ListStrings;
    m_List = new wxListBox( itemDialog1, ID_LISTBOX, wxDefaultPosition, wxSize(200, 60), m_ListStrings, wxLB_SINGLE );
    itemFlexGridSizer3->Add(m_List, 0, wxGROW|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxStaticText* itemStaticText10 = new wxStaticText( itemDialog1, wxID_STATIC, _(" Range:"), wxDefaultPosition, wxDefaultSize, 0 );
    itemFlexGridSizer3->Add(itemStaticText10, 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxBoxSizer* itemBoxSizer11 = new wxBoxSizer(wxHORIZONTAL);
    itemFlexGridSizer3->Add(itemBoxSizer11, 0, wxGROW|wxALIGN_CENTER_VERTICAL|wxALL, 0);

    m_MinValueText = new wxTextCtrl( itemDialog1, ID_MIN_VALUE, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
    m_MinValueText->Enable(false);
    itemBoxSizer11->Add(m_MinValueText, 1, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxStaticText* itemStaticText13 = new wxStaticText( itemDialog1, wxID_STATIC, _(" - "), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer11->Add(itemStaticText13, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    m_MaxValueText = new wxTextCtrl( itemDialog1, ID_MAX_VALUE, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
    m_MaxValueText->Enable(false);
    itemBoxSizer11->Add(m_MaxValueText, 1, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxStaticText* itemStaticText15 = new wxStaticText( itemDialog1, wxID_STATIC, _("Display Range"), wxDefaultPosition, wxDefaultSize, 0 );
    itemFlexGridSizer3->Add(itemStaticText15, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxFlexGridSizer* itemFlexGridSizer16 = new wxFlexGridSizer(0, 2, 0, 0);
    itemFlexGridSizer3->Add(itemFlexGridSizer16, 0, wxGROW|wxALIGN_CENTER_VERTICAL, 5);

    m_MinGradCheck = new wxCheckBox( itemDialog1, ID_MIN_GRAD_CHECK, _("Min"), wxDefaultPosition, wxDefaultSize, 0 );
    m_MinGradCheck->SetValue(false);
    itemFlexGridSizer16->Add(m_MinGradCheck, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    m_MinGradInput = new wxTextCtrl( itemDialog1, ID_MIN_GRAD, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
    itemFlexGridSizer16->Add(m_MinGradInput, 0, wxGROW|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    m_MaxGradCheck = new wxCheckBox( itemDialog1, ID_MAX_GRAD_CHECK, _("Max"), wxDefaultPosition, wxDefaultSize, 0 );
    m_MaxGradCheck->SetValue(false);
    itemFlexGridSizer16->Add(m_MaxGradCheck, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    m_MaxGradInput = new wxTextCtrl( itemDialog1, ID_MAX_GRAD, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
    itemFlexGridSizer16->Add(m_MaxGradInput, 1, wxGROW|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    itemFlexGridSizer16->AddGrowableCol(1);

    itemFlexGridSizer3->Add(5, 5, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxBoxSizer* itemBoxSizer22 = new wxBoxSizer(wxVERTICAL);
    itemFlexGridSizer3->Add(itemBoxSizer22, 0, wxGROW|wxALIGN_CENTER_VERTICAL|wxALL, 0);

    wxBoxSizer* itemBoxSizer23 = new wxBoxSizer(wxHORIZONTAL);
    itemBoxSizer22->Add(itemBoxSizer23, 0, wxGROW|wxALL, 0);

    m_MinColorBtn = new wxColourPickerCtrl( itemDialog1, ID_MIN_COLOR_BTN );
    itemBoxSizer23->Add(m_MinColorBtn, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxStaticText* itemStaticText25 = new wxStaticText( itemDialog1, wxID_STATIC, _("Min Color"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer23->Add(itemStaticText25, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    itemBoxSizer23->Add(5, 5, 1, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxStaticText* itemStaticText27 = new wxStaticText( itemDialog1, wxID_STATIC, _("Max Color"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer23->Add(itemStaticText27, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    m_MaxColorBtn = new wxColourPickerCtrl( itemDialog1, ID_MAX_COLOR_BTN );
    itemBoxSizer23->Add(m_MaxColorBtn, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    m_LogCheck = new wxCheckBox( itemDialog1, ID_CHECKBOX1, _("Logarithmic Color Scale"), wxDefaultPosition, wxDefaultSize, 0 );
    m_LogCheck->SetValue(false);
    itemBoxSizer22->Add(m_LogCheck, 0, wxALIGN_LEFT|wxALL, 5);

    m_GradPanel = new CGradientPanel( itemDialog1, ID_CGRADIENTPANEL1, wxDefaultPosition, wxDefaultSize, wxSTATIC_BORDER|wxFULL_REPAINT_ON_RESIZE|wxTAB_TRAVERSAL );
    itemBoxSizer22->Add(m_GradPanel, 0, wxGROW|wxALL, 5);

    itemFlexGridSizer3->AddGrowableRow(1);
    itemFlexGridSizer3->AddGrowableCol(1);

    wxStdDialogButtonSizer* itemStdDialogButtonSizer31 = new wxStdDialogButtonSizer;

    itemBoxSizer2->Add(itemStdDialogButtonSizer31, 0, wxALIGN_RIGHT|wxALL, 5);
    wxButton* itemButton32 = new wxButton( itemDialog1, wxID_OK, _("&OK"), wxDefaultPosition, wxDefaultSize, 0 );
    itemStdDialogButtonSizer31->AddButton(itemButton32);

    wxButton* itemButton33 = new wxButton( itemDialog1, wxID_CANCEL, _("&Cancel"), wxDefaultPosition, wxDefaultSize, 0 );
    itemStdDialogButtonSizer31->AddButton(itemButton33);

    itemStdDialogButtonSizer31->Realize();

    itemBoxSizer2->Add(5, 5, 0, wxALIGN_CENTER_HORIZONTAL|wxBOTTOM, 5);

    // Set validators
    m_EnableCheck->SetValidator( wxGenericValidator(& m_EnableControls) );
    m_List->SetValidator( wxGenericValidator(& m_ScoreSel) );
    m_MinValueText->SetValidator( CFloatTextValidator(& m_CurrParams.m_MinValue) );
    m_MaxValueText->SetValidator( CFloatTextValidator(& m_CurrParams.m_MaxValue) );
    m_MinGradCheck->SetValidator( wxGenericValidator(& m_CurrParams.m_EnableMinGrad) );
    m_MinGradInput->SetValidator( CFloatTextValidator(& m_CurrParams.m_MinGrad) );
    m_MaxGradCheck->SetValidator( wxGenericValidator(& m_CurrParams.m_EnableMaxGrad) );
    m_MaxGradInput->SetValidator( CFloatTextValidator(& m_CurrParams.m_MaxGrad) );
    m_LogCheck->SetValidator( wxGenericValidator(& m_CurrParams.m_LogScale) );
    // Connect events and objects
    m_MinGradInput->Connect(ID_MIN_GRAD, wxEVT_KILL_FOCUS, wxFocusEventHandler(CwxScoreDlg::OnKillFocus), NULL, this);
    m_MaxGradInput->Connect(ID_MAX_GRAD, wxEVT_KILL_FOCUS, wxFocusEventHandler(CwxScoreDlg::OnKillFocus), NULL, this);
////@end CwxScoreDlg content construction
}


wxBitmap CwxScoreDlg::GetBitmapResource( const wxString& name )
{
    // Bitmap retrieval
////@begin CwxScoreDlg bitmap retrieval
    wxUnusedVar(name);
    return wxNullBitmap;
////@end CwxScoreDlg bitmap retrieval
}

wxIcon CwxScoreDlg::GetIconResource( const wxString& name )
{
    // Icon retrieval
////@begin CwxScoreDlg icon retrieval
    wxUnusedVar(name);
    return wxNullIcon;
////@end CwxScoreDlg icon retrieval
}


void CwxScoreDlg::OnMinGradCheckClick( wxCommandEvent& event )
{
    TransferDataFromWindow();
    TransferDataToWindow();
}


void CwxScoreDlg::OnMaxGradCheckClick( wxCommandEvent& event )
{
    TransferDataFromWindow();
    TransferDataToWindow();
}


void CwxScoreDlg::OnMinColorBtnColourPickerChanged( wxColourPickerEvent& event )
{
    TransferDataFromWindow();
    TransferDataToWindow();
}


void CwxScoreDlg::OnMaxColorBtnColourPickerChanged( wxColourPickerEvent& event )
{
    TransferDataFromWindow();
    TransferDataToWindow();
}


void CwxScoreDlg::OnCheckbox1Click( wxCommandEvent& event )
{
    m_LogCheck->GetValidator()->TransferFromWindow();
    TransferDataToWindow();
}


bool CwxScoreDlg::ShowToolTips()
{
    return true;
}


void CwxScoreDlg::SetItems(vector<SHitColoringParams*>& params, const string curr_name)
{
    m_EnableControls = false;
    m_OrigParams = &params;

    size_t size = params.size();
    m_Params.resize(size);

    for( size_t i = 0; i < size; i++ ) {
        SHitColoringParams& p = *params[i];
        m_Params[i] = p;
    }

    _ASSERT(m_List);
    m_List->Clear();

    m_CurrIndex = (size == 0) ? -1 : 0;  // first or invalid (if empty)
    for( size_t i = 0; i < size; i++ ) {
        const string& name = m_Params[i].m_ScoreName;
        if(name == curr_name)   {
            m_CurrIndex = (int)i;
        }
        m_List->Append(ToWxString(name));
    }

    m_ScoreSel[0] = m_CurrIndex;
    if(m_CurrIndex >= 0)    {
        m_CurrParams = m_Params[m_CurrIndex];
    }
    m_EnableControls = ! curr_name.empty()  &&  (m_CurrIndex >= 0);

    // initialize controls
    TransferDataToWindow();

    mf_FinalCopy = false;
}


string CwxScoreDlg::GetSelectedName() const
{
    if(m_EnableControls)   {
        return m_Params[m_CurrIndex].m_ScoreName;
    }
    return "";
}


bool CwxScoreDlg::TransferDataToWindow()
{
    bool has_cur = m_EnableControls  &&  m_CurrIndex >= 0;

    m_List->Enable(has_cur);
    m_MinValueText->Enable(has_cur);
    m_MaxValueText->Enable(has_cur);

    m_MinGradCheck->Enable(has_cur);
    m_MaxGradCheck->Enable(has_cur);

    m_MinGradInput->Enable(has_cur  &&  m_CurrParams.m_EnableMinGrad);
    m_MaxGradInput->Enable(has_cur  &&  m_CurrParams.m_EnableMaxGrad);

    m_MinColorBtn->Enable(has_cur);
    m_MaxColorBtn->Enable(has_cur);

    m_GradPanel->Enable(has_cur);
    m_LogCheck->Enable(has_cur);

    if( wxDialog::TransferDataToWindow() ){
        ((CFloatTextValidator*)m_MinValueText->GetValidator())->SetFormat( m_CurrParams.GetPrecisionFormat() );
        ((CFloatTextValidator*)m_MaxValueText->GetValidator())->SetFormat( m_CurrParams.GetPrecisionFormat() );
        ((CFloatTextValidator*)m_MinGradInput->GetValidator())->SetFormat( m_CurrParams.GetPrecisionFormat() );
        ((CFloatTextValidator*)m_MaxGradInput->GetValidator())->SetFormat( m_CurrParams.GetPrecisionFormat() );

        m_MinColorBtn->SetColour(ConvertColor(m_CurrParams.m_MinColor));
        m_MaxColorBtn->SetColour(ConvertColor(m_CurrParams.m_MaxColor));

        m_GradPanel->SetParams(&m_CurrParams);

        return true;
    }
    return false;
}


bool CwxScoreDlg::TransferDataFromWindow()
{
    if( wxDialog::TransferDataFromWindow() ){

        m_CurrParams.m_MinColor = ConvertColor(m_MinColorBtn->GetColour());
        m_CurrParams.m_MaxColor = ConvertColor(m_MaxColorBtn->GetColour());

        bool has_cur = m_EnableControls  &&  m_ScoreSel[0] >= 0;
        if(has_cur)    {
            int curr_index = m_ScoreSel[0];
            m_Params[curr_index] = m_CurrParams;
        } else {
            m_EmptyParams = m_CurrParams;
        }

        if( mf_FinalCopy ){
            for( size_t i = 0; i < m_OrigParams->size(); i++ ) {
                SHitColoringParams& p = *(*m_OrigParams)[i];
                p = m_Params[i];
            }
        }

        return true;
    }

    return false;
}


void CwxScoreDlg::OnListboxSelected( wxCommandEvent& event )
{
    // get new selection
    m_List->GetValidator()->TransferFromWindow();
    int curr_index = m_ScoreSel[0];

    if(curr_index != m_CurrIndex)  {
        // save changes
        m_Params[m_CurrIndex] = m_CurrParams;
        // change index
        m_CurrIndex = curr_index;
        // load data for the new index
        m_CurrParams = m_Params[m_CurrIndex];
    }
    TransferDataToWindow();
}


void CwxScoreDlg::OnCheckboxClick( wxCommandEvent& event )
{
    m_EnableCheck->GetValidator()->TransferFromWindow();

    if(m_EnableControls)    {
        // restore
        m_CurrParams = m_Params[m_CurrIndex];
    } else {
        // save changes
        m_Params[m_CurrIndex] = m_CurrParams;
    }
    TransferDataToWindow();
}


void CwxScoreDlg::OnKillFocus( wxFocusEvent& event )
{
    TransferDataFromWindow();
    TransferDataToWindow();
}


void CwxScoreDlg::OnOkClick( wxCommandEvent& event )
{
    mf_FinalCopy = true;

    event.Skip();
}


END_NCBI_SCOPE

