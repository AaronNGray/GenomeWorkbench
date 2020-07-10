/*  $Id: wx_build_options_dlg.cpp 44757 2020-03-05 18:58:50Z evgeniev $
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

#include <gui/widgets/aln_multiple/wx_build_options_dlg.hpp>

#include <gui/widgets/wx/dialog_utils.hpp>

#include <gui/objutils/label.hpp>
#include <objmgr/util/sequence.hpp>

#include <wx/panel.h>
#include <wx/stattext.h>
#include <wx/statbox.h>
#include <wx/button.h>
#include <wx/combobox.h>
#include <wx/sizer.h>
#include <wx/radiobut.h>
#include <wx/checkbox.h>
#include <wx/listbox.h>
#include <wx/bitmap.h>
#include <wx/icon.h>
#include <wx/valgen.h>
#include <wx/textctrl.h>


BEGIN_NCBI_SCOPE

typedef CAlnMultiDSBuilder::TOptions TOptions;
typedef vector<CBioseq_Handle> THandles;


IMPLEMENT_DYNAMIC_CLASS( CBuildOptionsDlg, CDialog )

BEGIN_EVENT_TABLE( CBuildOptionsDlg, CDialog )
////@begin CBuildOptionsDlg event table entries
    EVT_RADIOBUTTON( ID_MULTIPLE_RADIO, CBuildOptionsDlg::OnMultipleRadioSelected )

    EVT_RADIOBUTTON( ID_QUERY_RADIO, CBuildOptionsDlg::OnQueryRadioSelected )

    EVT_RADIOBUTTON( ID_ORIGINAL_RADIO, CBuildOptionsDlg::OnOriginalRadioSelected )

    EVT_RADIOBUTTON( ID_CLIP_RADIO, CBuildOptionsDlg::OnClipRadioSelected )

    EVT_RADIOBUTTON( ID_EXTEND_RADIO, CBuildOptionsDlg::OnExtendRadioSelected )

    EVT_RADIOBUTTON( ID_SHOW_FLANK_RADIO, CBuildOptionsDlg::OnShowFlankRadioSelected )

    EVT_RADIOBUTTON( ID_SHOW_UA_RADIO, CBuildOptionsDlg::OnShowUaRadioSelected )

////@end CBuildOptionsDlg event table entries
END_EVENT_TABLE()


CBuildOptionsDlg::CBuildOptionsDlg()
{
    Init();
}


CBuildOptionsDlg::CBuildOptionsDlg( wxWindow* parent, wxWindowID id, const wxString& caption, const wxPoint& pos, const wxSize& size, long style )
{
    Init();
    Create(parent, id, caption, pos, size, style);
}


bool CBuildOptionsDlg::Create( wxWindow* parent, wxWindowID id, const wxString& caption, const wxPoint& pos, const wxSize& size, long style )
{
////@begin CBuildOptionsDlg creation
    SetExtraStyle(wxWS_EX_BLOCK_EVENTS);
    CDialog::Create( parent, id, caption, pos, size, style );

    CreateControls();
    if (GetSizer())
    {
        GetSizer()->SetSizeHints(this);
    }
    Centre();
////@end CBuildOptionsDlg creation
    return true;
}

CBuildOptionsDlg::~CBuildOptionsDlg()
{
////@begin CBuildOptionsDlg destruction
////@end CBuildOptionsDlg destruction
}

void CBuildOptionsDlg::Init()
{
////@begin CBuildOptionsDlg member initialisation
    m_AnchorIDList = NULL;
    m_StartText = NULL;
    m_EndText = NULL;
    m_ExtentText = NULL;
    m_FlankText = NULL;
////@end CBuildOptionsDlg member initialisation
}

void CBuildOptionsDlg::CreateControls()
{
////@begin CBuildOptionsDlg content construction
    CBuildOptionsDlg* itemCDialog1 = this;

    wxBoxSizer* itemBoxSizer2 = new wxBoxSizer(wxVERTICAL);
    itemCDialog1->SetSizer(itemBoxSizer2);

    wxBoxSizer* itemBoxSizer3 = new wxBoxSizer(wxHORIZONTAL);
    itemBoxSizer2->Add(itemBoxSizer3, 1, wxGROW|wxALL, 0);

    wxBoxSizer* itemBoxSizer4 = new wxBoxSizer(wxVERTICAL);
    itemBoxSizer3->Add(itemBoxSizer4, 1, wxGROW|wxALL, 0);

    wxStaticBox* itemStaticBoxSizer5Static = new wxStaticBox(itemCDialog1, wxID_ANY, wxT("Display Alignment as:"));
    wxStaticBoxSizer* itemStaticBoxSizer5 = new wxStaticBoxSizer(itemStaticBoxSizer5Static, wxVERTICAL);
    itemBoxSizer4->Add(itemStaticBoxSizer5, 1, wxGROW|wxALL, 5);

    wxRadioButton* itemRadioButton6 = new wxRadioButton( itemCDialog1, ID_MULTIPLE_RADIO, wxT("Multiple"), wxDefaultPosition, wxDefaultSize, wxRB_GROUP );
    itemRadioButton6->SetValue(false);
    itemStaticBoxSizer5->Add(itemRadioButton6, 0, wxALIGN_LEFT|wxALL, 5);

    wxRadioButton* itemRadioButton7 = new wxRadioButton( itemCDialog1, ID_QUERY_RADIO, wxT("Query-anchored"), wxDefaultPosition, wxDefaultSize, 0 );
    itemRadioButton7->SetValue(false);
    itemStaticBoxSizer5->Add(itemRadioButton7, 0, wxALIGN_LEFT|wxALL, 5);

    wxArrayString m_AnchorIDListStrings;
    m_AnchorIDList = new wxListBox( itemCDialog1, ID_ANCHOR_LIST, wxDefaultPosition, wxSize(300, 190), m_AnchorIDListStrings, wxLB_SINGLE|wxLB_NEEDED_SB|wxLB_HSCROLL );
    itemStaticBoxSizer5->Add(m_AnchorIDList, 1, wxGROW|wxALL, 5);

    wxBoxSizer* itemBoxSizer9 = new wxBoxSizer(wxHORIZONTAL);
    itemBoxSizer4->Add(itemBoxSizer9, 0, wxALIGN_LEFT|wxLEFT|wxRIGHT, 5);

    wxBoxSizer* itemBoxSizer10 = new wxBoxSizer(wxVERTICAL);
    itemBoxSizer3->Add(itemBoxSizer10, 0, wxGROW|wxALL, 0);

    wxStaticBox* itemStaticBoxSizer11Static = new wxStaticBox(itemCDialog1, wxID_ANY, wxT("Clip \\ Extend Alignment"));
    wxStaticBoxSizer* itemStaticBoxSizer11 = new wxStaticBoxSizer(itemStaticBoxSizer11Static, wxVERTICAL);
    itemBoxSizer10->Add(itemStaticBoxSizer11, 0, wxGROW|wxALL, 5);

    wxBoxSizer* itemBoxSizer12 = new wxBoxSizer(wxVERTICAL);
    itemStaticBoxSizer11->Add(itemBoxSizer12, 0, wxGROW, 5);

    wxRadioButton* itemRadioButton13 = new wxRadioButton( itemCDialog1, ID_ORIGINAL_RADIO, wxT("Do not modify"), wxDefaultPosition, wxDefaultSize, wxRB_GROUP );
    itemRadioButton13->SetValue(false);
    itemBoxSizer12->Add(itemRadioButton13, 0, wxALIGN_LEFT|wxALL, 5);

    wxRadioButton* itemRadioButton14 = new wxRadioButton( itemCDialog1, ID_CLIP_RADIO, wxT("Clip Alignment on sequence"), wxDefaultPosition, wxDefaultSize, 0 );
    itemRadioButton14->SetValue(false);
    itemBoxSizer12->Add(itemRadioButton14, 0, wxALIGN_LEFT|wxALL, 5);

    wxBoxSizer* itemBoxSizer15 = new wxBoxSizer(wxHORIZONTAL);
    itemBoxSizer12->Add(itemBoxSizer15, 0, wxALIGN_RIGHT, 5);

    itemBoxSizer15->Add(20, 5, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxStaticText* itemStaticText17 = new wxStaticText( itemCDialog1, wxID_STATIC, wxT("Start:"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer15->Add(itemStaticText17, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    m_StartText = new wxTextCtrl( itemCDialog1, ID_START_TEXT, wxT(""), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer15->Add(m_StartText, 0, wxALIGN_CENTER_VERTICAL|wxLEFT|wxRIGHT, 5);

    wxStaticText* itemStaticText19 = new wxStaticText( itemCDialog1, wxID_STATIC, wxT("End:"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer15->Add(itemStaticText19, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    m_EndText = new wxTextCtrl( itemCDialog1, ID_END_TEXT, wxT(""), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer15->Add(m_EndText, 0, wxALIGN_CENTER_VERTICAL|wxLEFT|wxRIGHT, 5);

    wxBoxSizer* itemBoxSizer21 = new wxBoxSizer(wxHORIZONTAL);
    itemBoxSizer12->Add(itemBoxSizer21, 0, wxALIGN_CENTER_HORIZONTAL, 5);

    wxRadioButton* itemRadioButton22 = new wxRadioButton( itemCDialog1, ID_EXTEND_RADIO, wxT("Extend Alignment on every side by:"), wxDefaultPosition, wxDefaultSize, 0 );
    itemRadioButton22->SetValue(false);
    itemBoxSizer21->Add(itemRadioButton22, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    itemBoxSizer21->Add(5, 5, 1, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    m_ExtentText = new wxTextCtrl( itemCDialog1, ID_TEXTCTRL, wxT(""), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer21->Add(m_ExtentText, 0, wxALIGN_CENTER_VERTICAL|wxLEFT|wxRIGHT, 5);

    wxStaticBox* itemStaticBoxSizer25Static = new wxStaticBox(itemCDialog1, wxID_ANY, wxT("Unaligned Regions:"));
    wxStaticBoxSizer* itemStaticBoxSizer25 = new wxStaticBoxSizer(itemStaticBoxSizer25Static, wxVERTICAL);
    itemBoxSizer10->Add(itemStaticBoxSizer25, 0, wxGROW|wxALL, 5);

    wxRadioButton* itemRadioButton26 = new wxRadioButton( itemCDialog1, ID_HIDE_UA_RADIO, wxT("Hide Unaligned"), wxDefaultPosition, wxDefaultSize, 0 );
    itemRadioButton26->SetValue(false);
    itemStaticBoxSizer25->Add(itemRadioButton26, 0, wxALIGN_LEFT|wxALL, 5);

    wxBoxSizer* itemBoxSizer27 = new wxBoxSizer(wxHORIZONTAL);
    itemStaticBoxSizer25->Add(itemBoxSizer27, 0, wxALIGN_LEFT, 5);

    wxRadioButton* itemRadioButton28 = new wxRadioButton( itemCDialog1, ID_SHOW_FLANK_RADIO, wxT("Show flanking Unaligned"), wxDefaultPosition, wxDefaultSize, 0 );
    itemRadioButton28->SetValue(false);
    itemBoxSizer27->Add(itemRadioButton28, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    m_FlankText = new wxTextCtrl( itemCDialog1, ID_EXTENT_TEXT, wxT(""), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer27->Add(m_FlankText, 0, wxALIGN_CENTER_VERTICAL|wxLEFT|wxRIGHT, 5);

    wxStaticText* itemStaticText30 = new wxStaticText( itemCDialog1, wxID_STATIC, wxT("residues"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer27->Add(itemStaticText30, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxRadioButton* itemRadioButton31 = new wxRadioButton( itemCDialog1, ID_SHOW_UA_RADIO, wxT("Show Unaligned"), wxDefaultPosition, wxDefaultSize, 0 );
    itemRadioButton31->SetValue(false);
    itemStaticBoxSizer25->Add(itemRadioButton31, 0, wxALIGN_LEFT|wxALL, 5);

    wxBoxSizer* itemBoxSizer32 = new wxBoxSizer(wxVERTICAL);
    itemBoxSizer10->Add(itemBoxSizer32, 0, wxALIGN_LEFT|wxLEFT, 10);

    wxCheckBox* itemCheckBox33 = new wxCheckBox( itemCDialog1, ID_COMBINE_CHECK, wxT("Combine rows with the same IDs"), wxDefaultPosition, wxDefaultSize, 0 );
    itemCheckBox33->SetValue(false);
    itemBoxSizer32->Add(itemCheckBox33, 0, wxALIGN_LEFT|wxALL, 5);

    wxCheckBox* itemCheckBox34 = new wxCheckBox( itemCDialog1, ID_SEPARATE_STRANDS, wxT("Separate Strands"), wxDefaultPosition, wxDefaultSize, 0 );
    itemCheckBox34->SetValue(false);
    itemBoxSizer32->Add(itemCheckBox34, 0, wxALIGN_LEFT|wxALL, 5);

    wxStdDialogButtonSizer* itemStdDialogButtonSizer35 = new wxStdDialogButtonSizer;

    itemBoxSizer2->Add(itemStdDialogButtonSizer35, 0, wxALIGN_RIGHT|wxALL, 5);
    wxButton* itemButton36 = new wxButton( itemCDialog1, wxID_OK, wxT("&OK"), wxDefaultPosition, wxDefaultSize, 0 );
    itemStdDialogButtonSizer35->AddButton(itemButton36);

    wxButton* itemButton37 = new wxButton( itemCDialog1, wxID_CANCEL, wxT("&Cancel"), wxDefaultPosition, wxDefaultSize, 0 );
    itemStdDialogButtonSizer35->AddButton(itemButton37);

    itemStdDialogButtonSizer35->Realize();

    // Set validators
    itemRadioButton6->SetValidator( wxGenericValidator(& m_Multiple) );
    m_AnchorIDList->SetValidator( wxGenericValidator(& m_IDIndexes) );
    itemRadioButton13->SetValidator( wxGenericValidator(& m_Original) );
    itemRadioButton14->SetValidator( wxGenericValidator(& m_Clip) );
    itemRadioButton22->SetValidator( wxGenericValidator(& m_Extend) );
    itemCheckBox33->SetValidator( wxGenericValidator(& m_CombineRows) );
    itemCheckBox34->SetValidator( wxGenericValidator(& m_SeparateStrands) );
////@end CBuildOptionsDlg content construction
}


bool CBuildOptionsDlg::ShowToolTips()
{
    return true;
}


wxBitmap CBuildOptionsDlg::GetBitmapResource( const wxString& name )
{
    // Bitmap retrieval
    wxUnusedVar(name);
    return wxNullBitmap;
}


wxIcon CBuildOptionsDlg::GetIconResource( const wxString& name )
{
    wxUnusedVar(name);
    return wxNullIcon;
}

void CBuildOptionsDlg::OnMultipleRadioSelected( wxCommandEvent& event )
{
    TransferDataFromWindow();
    TransferDataToWindow();
}

void CBuildOptionsDlg::OnQueryRadioSelected( wxCommandEvent& event )
{
    TransferDataFromWindow();
    TransferDataToWindow();
}

void CBuildOptionsDlg::OnOriginalRadioSelected( wxCommandEvent& event )
{
}

void CBuildOptionsDlg::OnClipRadioSelected( wxCommandEvent& event )
{
}

void CBuildOptionsDlg::OnExtendRadioSelected( wxCommandEvent& event )
{
}

void CBuildOptionsDlg::OnShowFlankRadioSelected( wxCommandEvent& event )
{
}

void CBuildOptionsDlg::OnShowUaRadioSelected( wxCommandEvent& event )
{
}


void CBuildOptionsDlg::SetParams(const CAlnMultiDSBuilder::TOptions& options, const vector<CBioseq_Handle>& handles, objects::CScope& scope)
{
    m_Options = options;
    m_Handles = handles;
    m_Scope.Reset(&scope);

    // fill ID List
    vector<string> ids;
    for( size_t i = 0; i< m_Handles.size(); i++ )   {
        ids.push_back(sequence::CDeflineGenerator().GenerateDefline(m_Handles[i]));
    }
    Init_wxControl(*m_AnchorIDList, ids);

    m_CombineRows = (m_Options.m_MergeAlgo == TOptions::eMergeAllSeqs);
    m_SeparateStrands = (m_Options.m_MergeFlags & TOptions::fAllowMixedStrand) == 0;

    m_Original = ! (m_Options.m_ClipAlignment || m_Options.m_ExtendAlignment);
    m_Clip = m_Options.m_ClipAlignment;
    m_Extend = m_Options.m_ExtendAlignment;

    //m_ShowFlanking = m_Options.m_UnalignedOption == TOptions::eShowFlankingN;
}


bool CBuildOptionsDlg::TransferDataToWindow()
{
    m_AnchorIDList->Enable( ! m_Multiple);

    m_StartText->Enable(m_Clip);
    m_EndText->Enable(m_Clip);

    m_ExtentText->Enable(m_Extend);

    //m_FlankText->Enable
    return wxDialog::TransferDataToWindow();
}

bool CBuildOptionsDlg::TransferDataFromWindow()
{
    return wxDialog::TransferDataFromWindow();
}



void CBuildOptionsDlg::GetOptions(CAlnMultiDSBuilder::TOptions& options)
{
}


END_NCBI_SCOPE
