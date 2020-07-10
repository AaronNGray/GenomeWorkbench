/*  $Id: parse_text_options_dlg.hpp 36044 2016-08-01 16:05:38Z filippov $
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
#ifndef _PARSE_TEXT_OPTIONS_DLG_H_
#define _PARSE_TEXT_OPTIONS_DLG_H_

#include <corelib/ncbistd.hpp>
#include <objtools/edit/parse_text_options.hpp>
#include <gui/widgets/wx/unfocused_controls.hpp>

#include <wx/panel.h>
#include <wx/checkbox.h>
#include <wx/textctrl.h>
#include <wx/radiobut.h>

BEGIN_NCBI_SCOPE

/*!
 * Control identifiers
 */

////@begin control identifiers
#define SYMBOL_CPARSETEXTOPTIONSDLG_STYLE wxTAB_TRAVERSAL
#define SYMBOL_CPARSETEXTOPTIONSDLG_TITLE _("ParseTextOptionsDlg")
#define SYMBOL_CPARSETEXTOPTIONSDLG_IDNAME ID_CPARSETEXTOPTIONSDLG
#define SYMBOL_CPARSETEXTOPTIONSDLG_SIZE wxSize(400, 300)
#define SYMBOL_CPARSETEXTOPTIONSDLG_POSITION wxDefaultPosition
////@end control identifiers


/*!
 * CParseTextOptionsDlg class declaration
 */

class CParseTextOptionsDlg: public wxPanel
{    
    DECLARE_DYNAMIC_CLASS( CParseTextOptionsDlg )
    DECLARE_EVENT_TABLE()

public:
    /// Constructors
    CParseTextOptionsDlg();
    CParseTextOptionsDlg( wxWindow* parent, 
        wxWindowID id = SYMBOL_CPARSETEXTOPTIONSDLG_IDNAME, 
        const wxPoint& pos = SYMBOL_CPARSETEXTOPTIONSDLG_POSITION,
        const wxSize& size = SYMBOL_CPARSETEXTOPTIONSDLG_SIZE, 
        long style = SYMBOL_CPARSETEXTOPTIONSDLG_STYLE,
        bool hide_remove_text_options = false );

    /// Creation
    bool Create( wxWindow* parent, 
        wxWindowID id = SYMBOL_CPARSETEXTOPTIONSDLG_IDNAME, 
        const wxPoint& pos = SYMBOL_CPARSETEXTOPTIONSDLG_POSITION,
        const wxSize& size = SYMBOL_CPARSETEXTOPTIONSDLG_SIZE, 
        long style = SYMBOL_CPARSETEXTOPTIONSDLG_STYLE );

    /// Destructor
    ~CParseTextOptionsDlg();

    /// Initialises member variables
    void Init();

    /// Creates the controls and sizers
    void CreateControls();

////@begin CParseTextOptionsDlg event handler declarations

    /// wxEVT_COMMAND_RADIOBUTTON_SELECTED event handler for ID_START_TXT_BTN
    void OnStartTxtBtnSelected( wxCommandEvent& event );

    /// wxEVT_COMMAND_RADIOBUTTON_SELECTED event handler for ID_START_DIGITS_BTN
    void OnStartDigitsBtnSelected( wxCommandEvent& event );

    /// wxEVT_COMMAND_RADIOBUTTON_SELECTED event handler for ID_START_LETTERS_BTN
    void OnStartLettersBtnSelected( wxCommandEvent& event );

    /// wxEVT_COMMAND_RADIOBUTTON_SELECTED event handler for ID_END_TEXT_BTN
    void OnEndTextBtnSelected( wxCommandEvent& event );

    /// wxEVT_COMMAND_RADIOBUTTON_SELECTED event handler for ID_END_DIGITS_BTN
    void OnEndDigitsBtnSelected( wxCommandEvent& event );

    /// wxEVT_COMMAND_RADIOBUTTON_SELECTED event handler for ID_END_LETTERS_BTN
    void OnEndLettersBtnSelected( wxCommandEvent& event );

    /// wxEVT_COMMAND_CHECKBOX_CLICKED event handler for ID_REMOVE_FROM_PARSED_BTN
    void OnRemoveFromParseSelected( wxCommandEvent& event );

////@end CParseTextOptionsDlg event handler declarations

////@begin CParseTextOptionsDlg member function declarations

    /// Retrieves bitmap resources
    wxBitmap GetBitmapResource( const wxString& name );

    /// Retrieves icon resources
    wxIcon GetIconResource( const wxString& name );
////@end CParseTextOptionsDlg member function declarations

    /// Should we show tooltips?
    static bool ShowToolTips();

////@begin CParseTextOptionsDlg member variables
    enum {
        ID_CPARSETEXTOPTIONSDLG = 6010,
        ID_CPARSEOPTS_JUST_AFTER_BTN,
        ID_CPARSEOPTS_RADIOBUTTON5,
        ID_CPARSEOPTS_START_TEXT_BTN,
        ID_CPARSEOPTS_START_TEXT_TXT,
        ID_CPARSEOPTS_START_DIGITS_BTN,
        ID_CPARSEOPTS_START_LETTERS_BTN,
        ID_CPARSEOPTS_ALSO_RMV_BEFORE_PARSE,
        ID_CPARSEOPTS_UP_TO_BTN,
        ID_CPARSEOPTS_INCLUDING_BTN,
        ID_CPARSEOPTS_END_TEXT_BTN,
        ID_CPARSEOPTS_END_TEXT_TXT,
        ID_CPARSEOPTS_END_DIGITS_BTN,
        ID_CPARSEOPTS_END_LETTERS_BTN,
        ID_CPARSEOPTS_ALSO_RMV_AFTER_PARSE,
        ID_CPARSEOPTS_REMOVE_FROM_PARSED_BTN,
        ID_CPARSEOPTS_CASE_INSENS_PARSE,
        ID_CPARSEOPTS_WHOLE_WORD_PARSE
    };
    CNoTabRadioButton* m_JustAfterBtn;
    CNoTabRadioButton* m_StartingAtBtn;
    CNoTabRadioButton* m_StartTextBtn;
    wxTextCtrl* m_StartTextTxt;
    CNoTabRadioButton* m_StartDigitsBtn;
    CNoTabRadioButton* m_StartLettersBtn;
    CNoTabCheckBox* m_AlsoRemoveBefore;
    CNoTabRadioButton* m_UpToBtn;
    CNoTabRadioButton* m_IncludingBtn;
    CNoTabRadioButton* m_EndTextBtn;
    wxTextCtrl* m_EndTextTxt;
    CNoTabRadioButton* m_EndDigitsBtn;
    CNoTabRadioButton* m_EndLettersBtn;
    CNoTabCheckBox* m_AlsoRemoveAfter;
    CNoTabCheckBox* m_RemoveFromParsedBtn;
    CNoTabCheckBox* m_Case;
    CNoTabCheckBox* m_WholeWord;
////@end CParseTextOptionsDlg member variables

    CRef<objects::edit::CParseTextOptions> GetParseOptions();
    void ClearValues(void);
    string GetSelectedText(const string& input);

    static void s_GetDigitsPosition(const string& str, size_t& pos, size_t& len);
    static void s_GetLettersPosition(const string& str, size_t& pos, size_t& len);
private:
    bool m_HideRemoveTextOptions;

};

END_NCBI_SCOPE

#endif
    // _PARSE_TEXT_OPTIONS_DLG_H_
