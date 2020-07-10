#ifndef GUI_WIDGETS_SEQ___TEXT_VIEW_OPTIONS_DLG__HPP
#define GUI_WIDGETS_SEQ___TEXT_VIEW_OPTIONS_DLG__HPP

/*  $Id: text_view_options_dlg.hpp 39490 2017-09-29 16:21:17Z bollin $
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

#include <corelib/ncbistd.hpp>

/*!
 * Includes
 */

#include <gui/widgets/wx/dialog.hpp>

////@begin includes
#include "wx/statline.h"
////@end includes

/*!
 * Forward declarations
 */

////@begin forward declarations
////@end forward declarations

class wxRadioBox;

BEGIN_NCBI_SCOPE

class CTextPanel;

/*!
 * Control identifiers
 */

////@begin control identifiers
#define SYMBOL_CTEXTVIEWOPTIONSDLG_STYLE wxCAPTION|wxSYSTEM_MENU|wxCLOSE_BOX|wxTAB_TRAVERSAL
#define SYMBOL_CTEXTVIEWOPTIONSDLG_TITLE _("Text View Options")
#define SYMBOL_CTEXTVIEWOPTIONSDLG_IDNAME ID_CTEXTVIEWOPTIONSDLG
#define SYMBOL_CTEXTVIEWOPTIONSDLG_SIZE wxSize(266, 184)
#define SYMBOL_CTEXTVIEWOPTIONSDLG_POSITION wxDefaultPosition
////@end control identifiers


/*!
 * CTextViewOptionsDlg class declaration
 */

class CTextViewOptionsDlg: public CDialog
{    
    DECLARE_DYNAMIC_CLASS( CTextViewOptionsDlg )
    DECLARE_EVENT_TABLE()

public:
    /// Constructors
    CTextViewOptionsDlg();
    CTextViewOptionsDlg( wxWindow* parent, wxWindowID id = SYMBOL_CTEXTVIEWOPTIONSDLG_IDNAME, const wxString& caption = SYMBOL_CTEXTVIEWOPTIONSDLG_TITLE, const wxPoint& pos = SYMBOL_CTEXTVIEWOPTIONSDLG_POSITION, const wxSize& size = SYMBOL_CTEXTVIEWOPTIONSDLG_SIZE, long style = SYMBOL_CTEXTVIEWOPTIONSDLG_STYLE );

    /// Creation
    bool Create( wxWindow* parent, wxWindowID id = SYMBOL_CTEXTVIEWOPTIONSDLG_IDNAME, const wxString& caption = SYMBOL_CTEXTVIEWOPTIONSDLG_TITLE, const wxPoint& pos = SYMBOL_CTEXTVIEWOPTIONSDLG_POSITION, const wxSize& size = SYMBOL_CTEXTVIEWOPTIONSDLG_SIZE, long style = SYMBOL_CTEXTVIEWOPTIONSDLG_STYLE );

    void ApplyChanges();

    /// Destructor
    ~CTextViewOptionsDlg();

    /// Initialises member variables
    void Init();

    /// Creates the controls and sizers
    void CreateControls();

////@begin CTextViewOptionsDlg event handler declarations

    /// wxEVT_COMMAND_CHOICE_SELECTED event handler for ID_CHOICE5
    void OnViewTypeSelected( wxCommandEvent& event );

    /// wxEVT_COMMAND_BUTTON_CLICKED event handler for wxID_OK
    void OnOkClick( wxCommandEvent& event );

    /// wxEVT_COMMAND_BUTTON_CLICKED event handler for wxID_APPLY
    void OnApplyClick( wxCommandEvent& event );

////@end CTextViewOptionsDlg event handler declarations

////@begin CTextViewOptionsDlg member function declarations

    bool GetReload() const { return m_Reload ; }
    void SetReload(bool value) { m_Reload = value ; }

    bool GetInvalidate() const { return m_Invalidate ; }
    void SetInvalidate(bool value) { m_Invalidate = value ; }

    bool GetSaveState() const { return m_SaveState ; }
    void SetSaveState(bool value) { m_SaveState = value ; }

    /// Retrieves bitmap resources
    wxBitmap GetBitmapResource( const wxString& name );

    /// Retrieves icon resources
    wxIcon GetIconResource( const wxString& name );
////@end CTextViewOptionsDlg member function declarations

    /// Should we show tooltips?
    static bool ShowToolTips();

////@begin CTextViewOptionsDlg member variables
    wxChoice* m_ViewTypeCtrl;
    wxChoice* m_FontSizeCtrl;
    wxChoice* m_FlatFileModeCtrl;
    wxCheckBox* m_OpenExpandedCtrl;
    wxCheckBox* m_HideVariationsCtrl;
    wxCheckBox* m_HideStsCtrl;
    wxCheckBox* m_ShowSeq;
    wxCheckBox* m_ShowComponentFeats;
    wxRadioBox* m_FeatureModeCtrl;
    wxCheckBox* m_ShowASNTypesCtrl;
    wxCheckBox* m_ShowASNPathToRootCtrl;
private:
    bool m_Reload;
    bool m_Invalidate;
    bool m_SaveState;
    /// Control identifiers
    enum {
        ID_CTEXTVIEWOPTIONSDLG = 10029,
        ID_CHOICE5 = 10036,
        ID_CHOICE2 = 10031,
        ID_CHOICE3 = 10034,
        ID_CHECKBOX4 = 10008,
        ID_CHECKBOX = 10004,
        ID_CHECKBOX3 = 10005,
        ID_CHECKBOX5 = 10009,
        ID_CHECKBOX6 = 10010,
        ID_RADIOBOX = 10006,
        ID_CHECKBOX1 = 10032,
        ID_CHECKBOX2 = 10033
    };
////@end CTextViewOptionsDlg member variables

private:
    void x_TransferDataToWindow();
    void x_OnApply();
    void x_UpdateControls();
    CTextPanel* m_Panel;
};

END_NCBI_SCOPE

#endif  // GUI_WIDGETS_SEQ___TEXT_VIEW_OPTIONS_DLG__HPP
