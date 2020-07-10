/*  $Id: feature_filter_dlg.cpp 24260 2011-08-30 14:55:10Z wuliangs $
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
 * Authors: Yury Voronov
 *
 * File Description:
 *
 */


#include <ncbi_pch.hpp>

#include <gui/widgets/feat_table/feature_filter_dlg.hpp>

#include <gui/widgets/wx/wx_utils.hpp>

#include <wx/stattext.h>
#include <wx/statbox.h>
#include <wx/textctrl.h>
#include <wx/checkbox.h>
#include <wx/checklst.h>
#include <wx/radiobut.h>
#include <wx/button.h>
#include <wx/panel.h>
#include <wx/sizer.h>
#include <wx/icon.h>
#include <wx/valgen.h>
#include <wx/valtext.h>


BEGIN_NCBI_SCOPE

IMPLEMENT_DYNAMIC_CLASS( CFeatureFilterDlg, CDialog )

BEGIN_EVENT_TABLE( CFeatureFilterDlg, CDialog )

////@begin CFeatureFilterDlg event table entries
    EVT_CHECKBOX( ID_NO_PRODUCT, CFeatureFilterDlg::OnNoProductClick )

    EVT_CHECKBOX( ID_CANCEL_CHECK, CFeatureFilterDlg::OnCancelFilteringClick )

////@end CFeatureFilterDlg event table entries

END_EVENT_TABLE()

CFeatureFilterDlg::CFeatureFilterDlg()
{
    Init();
}

CFeatureFilterDlg::CFeatureFilterDlg( wxWindow* parent, wxWindowID id, const wxString& caption, const wxPoint& pos, const wxSize& size, long style )
{
    Init();
    Create(parent, id, caption, pos, size, style);
}

bool CFeatureFilterDlg::Create( wxWindow* parent, wxWindowID id, const wxString& caption, const wxPoint& pos, const wxSize& size, long style )
{
////@begin CFeatureFilterDlg creation
    SetExtraStyle(wxWS_EX_VALIDATE_RECURSIVELY|wxWS_EX_BLOCK_EVENTS);
    CDialog::Create( parent, id, caption, pos, size, style );

    CreateControls();
    if (GetSizer())
    {
        GetSizer()->SetSizeHints(this);
    }
    Centre();
////@end CFeatureFilterDlg creation
    return true;
}

CFeatureFilterDlg::~CFeatureFilterDlg()
{
////@begin CFeatureFilterDlg destruction
////@end CFeatureFilterDlg destruction
}

void CFeatureFilterDlg::Init()
{
////@begin CFeatureFilterDlg member initialisation
    m_FromLocStr = wxEmptyString;
    m_ToLocStr = wxEmptyString;
    m_LengthLocStr = wxEmptyString;
    m_ParamsPanel = NULL;
    m_TypesBox = NULL;
    m_ProductCtrl = NULL;
////@end CFeatureFilterDlg member initialisation
}

void CFeatureFilterDlg::CreateControls()
{    
////@begin CFeatureFilterDlg content construction
    CFeatureFilterDlg* itemCDialog1 = this;

    wxBoxSizer* itemBoxSizer2 = new wxBoxSizer(wxVERTICAL);
    itemCDialog1->SetSizer(itemBoxSizer2);

    m_ParamsPanel = new wxPanel( itemCDialog1, ID_PARAMS_PANEL, wxDefaultPosition, wxDefaultSize, wxNO_BORDER|wxTAB_TRAVERSAL );
    itemBoxSizer2->Add(m_ParamsPanel, 0, wxGROW|wxALL, 5);

    wxStaticBox* itemStaticBoxSizer4Static = new wxStaticBox(m_ParamsPanel, wxID_ANY, _("Select features to be shown"));
    wxStaticBoxSizer* itemStaticBoxSizer4 = new wxStaticBoxSizer(itemStaticBoxSizer4Static, wxVERTICAL);
    m_ParamsPanel->SetSizer(itemStaticBoxSizer4);

    wxFlexGridSizer* itemFlexGridSizer5 = new wxFlexGridSizer(0, 2, 0, 0);
    itemFlexGridSizer5->AddGrowableCol(1);
    itemStaticBoxSizer4->Add(itemFlexGridSizer5, 0, wxGROW|wxALL, 5);

    wxStaticText* itemStaticText6 = new wxStaticText( m_ParamsPanel, wxID_LABEL_TEXT, _("Label:"), wxDefaultPosition, wxDefaultSize, 0 );
    itemFlexGridSizer5->Add(itemStaticText6, 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxTextCtrl* itemTextCtrl7 = new wxTextCtrl( m_ParamsPanel, ID_LABELTEXT, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
    itemFlexGridSizer5->Add(itemTextCtrl7, 1, wxGROW|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxStaticText* itemStaticText8 = new wxStaticText( m_ParamsPanel, wxID_STATIC, _("Type: "), wxDefaultPosition, wxDefaultSize, 0 );
    itemFlexGridSizer5->Add(itemStaticText8, 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxArrayString m_TypesBoxStrings;
    m_TypesBox = new wxCheckListBox( m_ParamsPanel, ID_TYPELIST, wxDefaultPosition, wxDefaultSize, m_TypesBoxStrings, wxLB_SINGLE );
    itemFlexGridSizer5->Add(m_TypesBox, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxStaticText* itemStaticText10 = new wxStaticText( m_ParamsPanel, wxID_STATIC, _("From:"), wxDefaultPosition, wxDefaultSize, 0 );
    itemFlexGridSizer5->Add(itemStaticText10, 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxBoxSizer* itemBoxSizer11 = new wxBoxSizer(wxHORIZONTAL);
    itemFlexGridSizer5->Add(itemBoxSizer11, 1, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxTextCtrl* itemTextCtrl12 = new wxTextCtrl( m_ParamsPanel, ID_FROMINT, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer11->Add(itemTextCtrl12, 0, wxALIGN_CENTER_VERTICAL|wxRIGHT|wxTOP|wxBOTTOM, 5);

    itemBoxSizer11->Add(5, 5, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxStaticText* itemStaticText14 = new wxStaticText( m_ParamsPanel, wxID_STATIC, _("To:"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer11->Add(itemStaticText14, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxTextCtrl* itemTextCtrl15 = new wxTextCtrl( m_ParamsPanel, ID_TOINT, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer11->Add(itemTextCtrl15, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    itemFlexGridSizer5->Add(5, 5, 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxBoxSizer* itemBoxSizer17 = new wxBoxSizer(wxVERTICAL);
    itemFlexGridSizer5->Add(itemBoxSizer17, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxBOTTOM, 5);

    wxRadioButton* itemRadioButton18 = new wxRadioButton( m_ParamsPanel, ID_RADIOINTERSECT, _("Features intersecting the range"), wxDefaultPosition, wxDefaultSize, 0 );
    itemRadioButton18->SetValue(false);
    itemBoxSizer17->Add(itemRadioButton18, 0, wxALIGN_LEFT|wxALL, 5);

    wxRadioButton* itemRadioButton19 = new wxRadioButton( m_ParamsPanel, ID_RADIOINCLUDE, _("Features within the range only"), wxDefaultPosition, wxDefaultSize, 0 );
    itemRadioButton19->SetValue(false);
    itemBoxSizer17->Add(itemRadioButton19, 0, wxALIGN_LEFT|wxALL, 5);

    wxStaticText* itemStaticText20 = new wxStaticText( m_ParamsPanel, wxID_STATIC, _("Length:"), wxDefaultPosition, wxDefaultSize, 0 );
    itemFlexGridSizer5->Add(itemStaticText20, 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxTextCtrl* itemTextCtrl21 = new wxTextCtrl( m_ParamsPanel, ID_TEXTCTRL3, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
    itemFlexGridSizer5->Add(itemTextCtrl21, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxStaticText* itemStaticText22 = new wxStaticText( m_ParamsPanel, wxID_STATIC, _("Product:"), wxDefaultPosition, wxDefaultSize, 0 );
    itemFlexGridSizer5->Add(itemStaticText22, 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxBoxSizer* itemBoxSizer23 = new wxBoxSizer(wxHORIZONTAL);
    itemFlexGridSizer5->Add(itemBoxSizer23, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    m_ProductCtrl = new wxTextCtrl( m_ParamsPanel, ID_PRODUCT_CTRL, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer23->Add(m_ProductCtrl, 0, wxALIGN_CENTER_VERTICAL|wxRIGHT|wxTOP|wxBOTTOM, 5);

    itemBoxSizer23->Add(5, 5, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxCheckBox* itemCheckBox26 = new wxCheckBox( m_ParamsPanel, ID_NO_PRODUCT, _("No product"), wxDefaultPosition, wxDefaultSize, 0 );
    itemCheckBox26->SetValue(false);
    itemBoxSizer23->Add(itemCheckBox26, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxCheckBox* itemCheckBox27 = new wxCheckBox( itemCDialog1, ID_CANCEL_CHECK, _("Cancel filtering"), wxDefaultPosition, wxDefaultSize, 0 );
    itemCheckBox27->SetValue(false);
    itemBoxSizer2->Add(itemCheckBox27, 0, wxALIGN_LEFT|wxALL, 5);

    wxStdDialogButtonSizer* itemStdDialogButtonSizer28 = new wxStdDialogButtonSizer;

    itemBoxSizer2->Add(itemStdDialogButtonSizer28, 0, wxALIGN_RIGHT|wxALL, 5);
    wxButton* itemButton29 = new wxButton( itemCDialog1, wxID_OK, _("&OK"), wxDefaultPosition, wxDefaultSize, 0 );
    itemStdDialogButtonSizer28->AddButton(itemButton29);

    wxButton* itemButton30 = new wxButton( itemCDialog1, wxID_CANCEL, _("&Cancel"), wxDefaultPosition, wxDefaultSize, 0 );
    itemStdDialogButtonSizer28->AddButton(itemButton30);

    itemStdDialogButtonSizer28->Realize();

    // Set validators
    itemTextCtrl7->SetValidator( wxTextValidator( wxFILTER_NONE, &m_Params.m_Label ) );
    itemTextCtrl12->SetValidator( wxTextValidator(wxFILTER_NUMERIC, & m_FromLocStr) );
    itemTextCtrl15->SetValidator( wxTextValidator(wxFILTER_NUMERIC, & m_ToLocStr) );
    itemRadioButton18->SetValidator( wxGenericValidator(& m_Intersect) );
    itemRadioButton19->SetValidator( wxGenericValidator( &m_Params.m_InclLoc ) );
    itemTextCtrl21->SetValidator( wxTextValidator(wxFILTER_NUMERIC, & m_LengthLocStr) );
    m_ProductCtrl->SetValidator( wxTextValidator( wxFILTER_NONE, &m_Params.m_Product ) );
    itemCheckBox26->SetValidator( wxGenericValidator( &m_Params.m_NoProduct ) );
    itemCheckBox27->SetValidator( wxGenericValidator(& m_Params.m_Disabled) );
////@end CFeatureFilterDlg content construction
}

bool CFeatureFilterDlg::TransferDataToWindow()
{
    m_FromLocStr = m_Params.m_FromLoc >= 0 ? ToWxString( NStr::LongToString( m_Params.m_FromLoc ) ) : wxString();
    m_ToLocStr = m_Params.m_ToLoc >= 0 ? ToWxString( NStr::LongToString( m_Params.m_ToLoc ) ) : wxString();
    m_LengthLocStr = m_Params.m_LengthLoc >= 0 ? ToWxString( NStr::LongToString( m_Params.m_LengthLoc ) ) : wxString();

    m_Intersect = !m_Params.m_InclLoc;

    for( size_t ix = 0; ix < m_Params.m_Types.GetCount(); ix++ ){
        int type_ix = m_TypesBox->FindString( m_Params.m_Types[ix] );
        if( type_ix !=wxNOT_FOUND ){
            m_TypesBox->Check( type_ix );
        }
    }

    if( !CDialog::TransferDataToWindow() ){
        return false;
    }

    m_ProductCtrl->Enable( !m_Params.m_NoProduct );

    m_ParamsPanel->Enable( !m_Params.m_Disabled );

    return true;
}

bool CFeatureFilterDlg::TransferDataFromWindow()
{
    if( !CDialog::TransferDataFromWindow() ){
        return false;
    }

    m_Params.m_FromLoc = !m_FromLocStr.empty() ? NStr::StringToInt( ToStdString( m_FromLocStr ) ) : -1;
    m_Params.m_ToLoc = !m_ToLocStr.empty() ? NStr::StringToInt( ToStdString( m_ToLocStr ) ) : -1;
    m_Params.m_LengthLoc = !m_LengthLocStr.empty() ? NStr::StringToInt( ToStdString( m_LengthLocStr ) ) : -1;

    m_Params.m_Types.Empty();
    for( size_t ix = 0; ix < m_TypesBox->GetCount(); ix++ ){
        if( m_TypesBox->IsChecked( ix ) ){
            m_Params.m_Types.Add( m_TypesBox->GetString( ix ) );
        }
    }

    return true;
}

void CFeatureFilterDlg::SetTypeNames( wxArrayString& types )
{
    if( m_TypesBox ){
        m_TypesBox->Set( types );
    }
}


bool CFeatureFilterDlg::ShowToolTips()
{
    return true;
}
wxBitmap CFeatureFilterDlg::GetBitmapResource( const wxString& name )
{
    // Bitmap retrieval
////@begin CFeatureFilterDlg bitmap retrieval
    wxUnusedVar(name);
    return wxNullBitmap;
////@end CFeatureFilterDlg bitmap retrieval
}
wxIcon CFeatureFilterDlg::GetIconResource( const wxString& name )
{
    // Icon retrieval
////@begin CFeatureFilterDlg icon retrieval
    wxUnusedVar(name);
    return wxNullIcon;
////@end CFeatureFilterDlg icon retrieval
}

void CFeatureFilterDlg::OnCancelFilteringClick( wxCommandEvent& event )
{
    m_ParamsPanel->Enable( !event.IsChecked() );
}

void CFeatureFilterDlg::OnNoProductClick( wxCommandEvent& event )
{
    m_ProductCtrl->Enable( !event.IsChecked() );
}


END_NCBI_SCOPE

