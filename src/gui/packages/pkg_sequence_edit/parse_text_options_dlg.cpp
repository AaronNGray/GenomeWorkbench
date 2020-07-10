/*  $Id: parse_text_options_dlg.cpp 36048 2016-08-01 19:08:05Z filippov $
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
 * Authors:  Colleen Bollin
 */


#include <ncbi_pch.hpp>
#include <gui/widgets/wx/wx_utils.hpp>
#include <gui/packages/pkg_sequence_edit/parse_text_options_dlg.hpp>

#include <wx/sizer.h>
#include <wx/stattext.h>

////@begin XPM images
////@end XPM images

BEGIN_NCBI_SCOPE

/*!
 * CParseTextOptionsDlg type definition
 */

IMPLEMENT_DYNAMIC_CLASS( CParseTextOptionsDlg, wxPanel )


/*!
 * CParseTextOptionsDlg event table definition
 */

BEGIN_EVENT_TABLE( CParseTextOptionsDlg, wxPanel )

////@begin CParseTextOptionsDlg event table entries
    EVT_RADIOBUTTON( ID_CPARSEOPTS_START_TEXT_BTN, CParseTextOptionsDlg::OnStartTxtBtnSelected)
    EVT_RADIOBUTTON( ID_CPARSEOPTS_START_DIGITS_BTN, CParseTextOptionsDlg::OnStartDigitsBtnSelected)
    EVT_RADIOBUTTON( ID_CPARSEOPTS_START_LETTERS_BTN, CParseTextOptionsDlg::OnStartLettersBtnSelected)
    EVT_RADIOBUTTON( ID_CPARSEOPTS_END_TEXT_BTN, CParseTextOptionsDlg::OnEndTextBtnSelected)
    EVT_RADIOBUTTON( ID_CPARSEOPTS_END_DIGITS_BTN, CParseTextOptionsDlg::OnEndDigitsBtnSelected)
    EVT_RADIOBUTTON( ID_CPARSEOPTS_END_LETTERS_BTN, CParseTextOptionsDlg::OnEndLettersBtnSelected)
    EVT_CHECKBOX( ID_CPARSEOPTS_REMOVE_FROM_PARSED_BTN, CParseTextOptionsDlg::OnRemoveFromParseSelected)
////@end CParseTextOptionsDlg event table entries

END_EVENT_TABLE()


/*!
 * CParseTextOptionsDlg constructors
 */

CParseTextOptionsDlg::CParseTextOptionsDlg()
{
    Init();
}

CParseTextOptionsDlg::CParseTextOptionsDlg( wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style, bool hide_remove_text_options )
    : m_HideRemoveTextOptions(hide_remove_text_options)
{
    Init();
    Create(parent, id, pos, size, style);
}


/*!
 * CParseTextOptionsDlg creator
 */

bool CParseTextOptionsDlg::Create( wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style )
{
////@begin CParseTextOptionsDlg creation
    wxPanel::Create( parent, id, pos, size, style );

    CreateControls();
    if (GetSizer())
    {
        GetSizer()->SetSizeHints(this);
    }
    Centre();
////@end CParseTextOptionsDlg creation
    return true;
}


/*!
 * CParseTextOptionsDlg destructor
 */

CParseTextOptionsDlg::~CParseTextOptionsDlg()
{
////@begin CParseTextOptionsDlg destruction
////@end CParseTextOptionsDlg destruction
}


/*!
 * Member initialisation
 */

void CParseTextOptionsDlg::Init()
{
////@begin CParseTextOptionsDlg member initialisation
    m_JustAfterBtn = NULL;
    m_StartingAtBtn = NULL;
    m_StartTextBtn = NULL;
    m_StartTextTxt = NULL;
    m_StartDigitsBtn = NULL;
    m_StartLettersBtn = NULL;
    m_AlsoRemoveBefore = NULL;
    m_UpToBtn = NULL;
    m_IncludingBtn = NULL;
    m_EndTextBtn = NULL;
    m_EndTextTxt = NULL;
    m_EndDigitsBtn = NULL;
    m_EndLettersBtn = NULL;
    m_AlsoRemoveAfter = NULL;
    m_RemoveFromParsedBtn = NULL;
    m_Case = NULL;
    m_WholeWord = NULL;
////@end CParseTextOptionsDlg member initialisation
}


/*!
 * Control creation for CParseTextOptionsDlg
 */

void CParseTextOptionsDlg::CreateControls()
{    
////@begin CParseTextOptionsDlg content construction
#if defined(__WXMSW__)
    wxPanel* itemPanel1 = new wxPanel(this);
#else
    CParseTextOptionsDlg* itemPanel1 = this;
#endif

    wxBoxSizer* itemBoxSizer2 = new wxBoxSizer(wxVERTICAL);
    itemPanel1->SetSizer(itemBoxSizer2);

    wxFlexGridSizer* itemFlexGridSizer3 = new wxFlexGridSizer(0, 4, 0, 0);
    itemBoxSizer2->Add(itemFlexGridSizer3, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 0);

    wxStaticText* itemStaticText4 = new wxStaticText( itemPanel1, wxID_STATIC, _("Between"), wxDefaultPosition, wxDefaultSize, 0 );
    itemFlexGridSizer3->Add(itemStaticText4, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxALL, 3);

    wxBoxSizer* itemBoxSizer5 = new wxBoxSizer(wxHORIZONTAL);
    itemFlexGridSizer3->Add(itemBoxSizer5, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxALL, 1);

    m_JustAfterBtn = new CNoTabRadioButton(itemPanel1, ID_CPARSEOPTS_JUST_AFTER_BTN, _("just after"), wxDefaultPosition, wxDefaultSize, wxRB_GROUP);
    m_JustAfterBtn->SetValue(true);
    itemBoxSizer5->Add(m_JustAfterBtn, 0, wxALIGN_CENTER_VERTICAL|wxALL, 3);

    m_StartingAtBtn = new CNoTabRadioButton(itemPanel1, ID_CPARSEOPTS_RADIOBUTTON5, _("starting at"), wxDefaultPosition, wxDefaultSize, 0);
    m_StartingAtBtn->SetValue(false);
    itemBoxSizer5->Add(m_StartingAtBtn, 0, wxALIGN_CENTER_VERTICAL|wxALL, 3);

    wxBoxSizer* itemBoxSizer8 = new wxBoxSizer(wxHORIZONTAL);
    itemFlexGridSizer3->Add(itemBoxSizer8, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxALL, 1);

    m_StartTextBtn = new CNoTabRadioButton(itemPanel1, ID_CPARSEOPTS_START_TEXT_BTN, _("Text"), wxDefaultPosition, wxDefaultSize, wxRB_GROUP);
    m_StartTextBtn->SetValue(true);
    itemBoxSizer8->Add(m_StartTextBtn, 0, wxALIGN_CENTER_VERTICAL|wxALL, 3);

    m_StartTextTxt = new wxTextCtrl(itemPanel1, ID_CPARSEOPTS_START_TEXT_TXT, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0);
    itemBoxSizer8->Add(m_StartTextTxt, 0, wxALIGN_CENTER_VERTICAL|wxALL, 3);

    m_StartDigitsBtn = new CNoTabRadioButton(itemPanel1, ID_CPARSEOPTS_START_DIGITS_BTN, _("Digits"), wxDefaultPosition, wxDefaultSize, 0);
    m_StartDigitsBtn->SetValue(false);
    itemBoxSizer8->Add(m_StartDigitsBtn, 0, wxALIGN_CENTER_VERTICAL|wxALL, 3);

    m_StartLettersBtn = new CNoTabRadioButton(itemPanel1, ID_CPARSEOPTS_START_LETTERS_BTN, _("Letters"), wxDefaultPosition, wxDefaultSize, 0);
    m_StartLettersBtn->SetValue(false);
    itemBoxSizer8->Add(m_StartLettersBtn, 0, wxALIGN_CENTER_VERTICAL|wxALL, 3);

    m_AlsoRemoveBefore = new CNoTabCheckBox(itemPanel1, ID_CPARSEOPTS_ALSO_RMV_BEFORE_PARSE, _("Also remove search pattern"), wxDefaultPosition, wxDefaultSize, 0);
    m_AlsoRemoveBefore->SetValue(false);
    m_AlsoRemoveBefore->Enable(false);
    itemBoxSizer8->Add(m_AlsoRemoveBefore, 0, wxALIGN_CENTER_VERTICAL|wxALL, 3);

    itemFlexGridSizer3->Add(0, 0, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxALL, 0);

    wxStaticText* itemStaticText15 = new wxStaticText( itemPanel1, wxID_STATIC, _("and"), wxDefaultPosition, wxDefaultSize, 0 );
    itemFlexGridSizer3->Add(itemStaticText15, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxALL, 3);

    wxBoxSizer* itemBoxSizer16 = new wxBoxSizer(wxHORIZONTAL);
    itemFlexGridSizer3->Add(itemBoxSizer16, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxALL, 1);

    m_UpToBtn = new CNoTabRadioButton(itemPanel1, ID_CPARSEOPTS_UP_TO_BTN, _("up to"), wxDefaultPosition, wxDefaultSize, wxRB_GROUP);
    m_UpToBtn->SetValue(true);
    itemBoxSizer16->Add(m_UpToBtn, 0, wxALIGN_CENTER_VERTICAL|wxALL, 3);

    m_IncludingBtn = new CNoTabRadioButton(itemPanel1, ID_CPARSEOPTS_INCLUDING_BTN, _("including"), wxDefaultPosition, wxDefaultSize, 0);
    m_IncludingBtn->SetValue(false);
    itemBoxSizer16->Add(m_IncludingBtn, 0, wxALIGN_CENTER_VERTICAL|wxALL, 3);

    wxBoxSizer* itemBoxSizer19 = new wxBoxSizer(wxHORIZONTAL);
    itemFlexGridSizer3->Add(itemBoxSizer19, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxALL, 1);

    m_EndTextBtn = new CNoTabRadioButton(itemPanel1, ID_CPARSEOPTS_END_TEXT_BTN, _("Text"), wxDefaultPosition, wxDefaultSize, wxRB_GROUP);
    m_EndTextBtn->SetValue(true);
    itemBoxSizer19->Add(m_EndTextBtn, 0, wxALIGN_CENTER_VERTICAL|wxALL, 3);

    m_EndTextTxt = new wxTextCtrl(itemPanel1, ID_CPARSEOPTS_END_TEXT_TXT, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0);
    itemBoxSizer19->Add(m_EndTextTxt, 0, wxALIGN_CENTER_VERTICAL|wxALL, 3);

    m_EndDigitsBtn = new CNoTabRadioButton(itemPanel1, ID_CPARSEOPTS_END_DIGITS_BTN, _("Digits"), wxDefaultPosition, wxDefaultSize, 0);
    m_EndDigitsBtn->SetValue(false);
    itemBoxSizer19->Add(m_EndDigitsBtn, 0, wxALIGN_CENTER_VERTICAL|wxALL, 3);

    m_EndLettersBtn = new CNoTabRadioButton(itemPanel1, ID_CPARSEOPTS_END_LETTERS_BTN, _("Letters"), wxDefaultPosition, wxDefaultSize, 0);
    m_EndLettersBtn->SetValue(false);
    itemBoxSizer19->Add(m_EndLettersBtn, 0, wxALIGN_CENTER_VERTICAL|wxALL, 3);

    m_AlsoRemoveAfter = new CNoTabCheckBox(itemPanel1, ID_CPARSEOPTS_ALSO_RMV_AFTER_PARSE, _("Also remove search pattern"), wxDefaultPosition, wxDefaultSize, 0);
    m_AlsoRemoveAfter->SetValue(false);
    m_AlsoRemoveAfter->Enable(false);
    itemBoxSizer19->Add(m_AlsoRemoveAfter, 0, wxALIGN_CENTER_VERTICAL|wxALL, 3);

    wxBoxSizer* itemBoxSizer25 = new wxBoxSizer(wxHORIZONTAL);
    itemBoxSizer2->Add(itemBoxSizer25, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 1);

    m_RemoveFromParsedBtn = new CNoTabCheckBox(itemPanel1, ID_CPARSEOPTS_REMOVE_FROM_PARSED_BTN, _("Remove from parsed field"), wxDefaultPosition, wxDefaultSize, 0);
    m_RemoveFromParsedBtn->SetValue(false);
    itemBoxSizer25->Add(m_RemoveFromParsedBtn, 0, wxALIGN_CENTER_VERTICAL|wxALL, 3);

    m_Case = new CNoTabCheckBox(itemPanel1, ID_CPARSEOPTS_CASE_INSENS_PARSE, _("Case Insensitive"), wxDefaultPosition, wxDefaultSize, 0);
    m_Case->SetValue(false);
    itemBoxSizer25->Add(m_Case, 0, wxALIGN_CENTER_VERTICAL|wxALL, 3);

    m_WholeWord = new CNoTabCheckBox(itemPanel1, ID_CPARSEOPTS_WHOLE_WORD_PARSE, _("Whole Word"), wxDefaultPosition, wxDefaultSize, 0);
    m_WholeWord->SetValue(false);
    itemBoxSizer25->Add(m_WholeWord, 0, wxALIGN_CENTER_VERTICAL|wxALL, 3);

#if defined(__WXMSW__)
    wxBoxSizer* main_sizer = new wxBoxSizer(wxHORIZONTAL);
    main_sizer->Add(itemPanel1, 1, wxALL|wxGROW, 0);
    SetSizerAndFit(main_sizer);
#endif
////@end CParseTextOptionsDlg content construction
    if (m_HideRemoveTextOptions)
    {
        m_StartTextBtn->Hide();
        m_StartDigitsBtn->Hide();
        m_StartLettersBtn->Hide();
        m_AlsoRemoveBefore->Hide();
        m_EndTextBtn->Hide();
        m_EndDigitsBtn->Hide();
        m_EndLettersBtn->Hide();
        m_AlsoRemoveAfter->Hide();
        m_RemoveFromParsedBtn->Hide();
    }
}


void CParseTextOptionsDlg::ClearValues()
{
    m_JustAfterBtn->SetValue(true);
    m_StartingAtBtn->SetValue(false);
    m_StartTextBtn->SetValue(true);
    m_StartTextTxt->SetValue(wxEmptyString);
    m_StartDigitsBtn->SetValue(false);
    m_StartLettersBtn->SetValue(false);
    m_AlsoRemoveBefore->SetValue(false);
    m_UpToBtn->SetValue(true);
    m_IncludingBtn->SetValue(false);
    m_EndTextBtn->SetValue(true);
    m_EndTextTxt->SetValue(wxEmptyString);
    m_EndDigitsBtn->SetValue(false);
    m_EndLettersBtn->SetValue(false);
    m_AlsoRemoveAfter->SetValue(false);
    m_RemoveFromParsedBtn->SetValue(false);
    m_Case->SetValue(false);
    m_WholeWord->SetValue(false);
    m_AlsoRemoveBefore->Enable(false);
    m_AlsoRemoveAfter->Enable(false);
    m_StartTextTxt->Enable(true);
    m_EndTextTxt->Enable(true); 
}

/*!
 * Should we show tooltips?
 */

bool CParseTextOptionsDlg::ShowToolTips()
{
    return true;
}

/*!
 * Get bitmap resources
 */

wxBitmap CParseTextOptionsDlg::GetBitmapResource( const wxString& name )
{
    // Bitmap retrieval
////@begin CParseTextOptionsDlg bitmap retrieval
    wxUnusedVar(name);
    return wxNullBitmap;
////@end CParseTextOptionsDlg bitmap retrieval
}

/*!
 * Get icon resources
 */

wxIcon CParseTextOptionsDlg::GetIconResource( const wxString& name )
{
    // Icon retrieval
////@begin CParseTextOptionsDlg icon retrieval
    wxUnusedVar(name);
    return wxNullIcon;
////@end CParseTextOptionsDlg icon retrieval
}


/*!
 * wxEVT_COMMAND_RADIOBUTTON_SELECTED event handler for ID_START_TXT_BTN
 */

void CParseTextOptionsDlg::OnStartTxtBtnSelected( wxCommandEvent& event )
{
    m_StartTextTxt->Enable(true);
}


/*!
 * wxEVT_COMMAND_RADIOBUTTON_SELECTED event handler for ID_START_LETTERS_BTN
 */

void CParseTextOptionsDlg::OnStartLettersBtnSelected( wxCommandEvent& event )
{
    m_StartTextTxt->Enable(false);
}


/*!
 * wxEVT_COMMAND_RADIOBUTTON_SELECTED event handler for ID_START_DIGITS_BTN
 */

void CParseTextOptionsDlg::OnStartDigitsBtnSelected( wxCommandEvent& event )
{
    m_StartTextTxt->Enable(false);
}


/*!
 * wxEVT_COMMAND_RADIOBUTTON_SELECTED event handler for ID_END_TEXT_BTN
 */

void CParseTextOptionsDlg::OnEndTextBtnSelected( wxCommandEvent& event )
{
    m_EndTextTxt->Enable(true); 
}


/*!
 * wxEVT_COMMAND_RADIOBUTTON_SELECTED event handler for ID_END_DIGITS_BTN
 */

void CParseTextOptionsDlg::OnEndDigitsBtnSelected( wxCommandEvent& event )
{
    m_EndTextTxt->Enable(false); 
}


/*!
 * wxEVT_COMMAND_RADIOBUTTON_SELECTED event handler for ID_END_LETTERS_BTN
 */

void CParseTextOptionsDlg::OnEndLettersBtnSelected( wxCommandEvent& event )
{
    m_EndTextTxt->Enable(false); 
}

void CParseTextOptionsDlg::OnRemoveFromParseSelected( wxCommandEvent& event)
{
    m_AlsoRemoveBefore->Enable(m_RemoveFromParsedBtn->IsChecked());
    m_AlsoRemoveAfter->Enable(m_RemoveFromParsedBtn->IsChecked());
}


CRef<objects::edit::CParseTextOptions> CParseTextOptionsDlg::GetParseOptions()
{
    CRef<objects::edit::CParseTextOptions> options(new objects::edit::CParseTextOptions());

    if (m_StartTextBtn->GetValue()) {
        string text = ToStdString(m_StartTextTxt->GetValue());
        options->SetStartText(text);
    } else if (m_StartDigitsBtn->GetValue()) {
        options->SetStartDigits();
    } else if (m_StartLettersBtn->GetValue()) {
        options->SetStartLetters();
    }
    options->SetIncludeStart(m_StartingAtBtn->GetValue());
    options->SetShouldRmvBeforePattern(m_AlsoRemoveBefore->GetValue());

    if (m_EndTextBtn->GetValue()) {
        string text = ToStdString(m_EndTextTxt->GetValue());
        options->SetStopText(text);
    } else if (m_EndDigitsBtn->GetValue()) {
        options->SetStopDigits();
    } else if (m_EndLettersBtn->GetValue()) {
        options->SetStopLetters();
    }
    options->SetIncludeStop(m_IncludingBtn->GetValue());
    options->SetShouldRmvAfterPattern(m_AlsoRemoveAfter->GetValue());

    options->SetShouldRemove(m_RemoveFromParsedBtn->GetValue());
    options->SetCaseInsensitive(m_Case->GetValue());
    options->SetWholeWord(m_WholeWord->GetValue());

    return options;
}


string CParseTextOptionsDlg::GetSelectedText(const string& input)
{
    CRef<objects::edit::CParseTextOptions> options = GetParseOptions();
    return options->GetSelectedText(input);
}


END_NCBI_SCOPE
