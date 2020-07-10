#ifndef GUI_WIDGETS_WX__WIZARD_DLG__HPP
#define GUI_WIDGETS_WX__WIZARD_DLG__HPP

/*  $Id: wizard_dlg.hpp 43611 2019-08-08 18:36:29Z filippov $
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

#include <corelib/ncbistd.hpp>

#include <gui/gui_export.h>

#include <gui/widgets/wx/dialog.hpp>
#include <gui/widgets/wx/wizard_page.hpp>

/*!
 * Includes
 */

////@begin includes
#include "wx/statline.h"
#include "wx/hyperlink.h"
////@end includes

/*!
 * Forward declarations
 */

////@begin forward declarations
class wxBoxSizer;
class wxHyperlinkCtrl;
////@end forward declarations

/*!
 * Control identifiers
 */

////@begin control identifiers
#define SYMBOL_CWIZARDDLG_STYLE wxCAPTION|wxRESIZE_BORDER|wxSYSTEM_MENU|wxCLOSE_BOX|wxTAB_TRAVERSAL
#define SYMBOL_CWIZARDDLG_TITLE _("WizardDlg")
#define SYMBOL_CWIZARDDLG_IDNAME ID_CWIZARDDLG
#define SYMBOL_CWIZARDDLG_SIZE wxSize(400, 300)
#define SYMBOL_CWIZARDDLG_POSITION wxDefaultPosition
////@end control identifiers

BEGIN_NCBI_SCOPE;

/*!
 * CWizardDlg class declaration
 */

class NCBI_GUIWIDGETS_WX_EXPORT CWizardDlg: public CDialog
{    
    DECLARE_DYNAMIC_CLASS( CWizardDlg )
    DECLARE_EVENT_TABLE()

public:
    enum
    {
        eCmdUpdateButtons = 15000,
        eCmdUpdateTitle
    };

    CWizardDlg();
    CWizardDlg( wxWindow* parent, wxWindowID id = SYMBOL_CWIZARDDLG_IDNAME, const wxString& caption = SYMBOL_CWIZARDDLG_TITLE, const wxPoint& pos = SYMBOL_CWIZARDDLG_POSITION, const wxSize& size = SYMBOL_CWIZARDDLG_SIZE, long style = SYMBOL_CWIZARDDLG_STYLE );

    bool Create( wxWindow* parent, wxWindowID id = SYMBOL_CWIZARDDLG_IDNAME, const wxString& caption = SYMBOL_CWIZARDDLG_TITLE, const wxPoint& pos = SYMBOL_CWIZARDDLG_POSITION, const wxSize& size = SYMBOL_CWIZARDDLG_SIZE, long style = SYMBOL_CWIZARDDLG_STYLE );

    ~CWizardDlg();

    void Init();

    void CreateControls();

    virtual void CreatePages() {}

    void SetCurrentPage(IWizardPage* page);
    void UpdateButtons();

////@begin CWizardDlg event handler declarations

    /// wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_BUTTON_OPTIONS
    void OnButtonOptionsClick( wxCommandEvent& event );

    /// wxEVT_COMMAND_BUTTON_CLICKED event handler for wxID_BACKWARD
    void OnBackwardClick( wxCommandEvent& event );

    /// wxEVT_COMMAND_BUTTON_CLICKED event handler for wxID_FORWARD
    void OnForwardClick( wxCommandEvent& event );

////@end CWizardDlg event handler declarations

    void OnUpdateButtons(wxCommandEvent& /*event*/) { UpdateButtons(); }

////@begin CWizardDlg member function declarations

    /// Retrieves bitmap resources
    wxBitmap GetBitmapResource( const wxString& name );

    /// Retrieves icon resources
    wxIcon GetIconResource( const wxString& name );
////@end CWizardDlg member function declarations

    static bool ShowToolTips();

    void SetHelpUrl(const wxString &url);

////@begin CWizardDlg member variables
    wxBoxSizer* m_ButtonsSizer;
    wxButton* m_OptionsBtn;
    wxButton* m_BackBtn;
    wxButton* m_NextBtn;
    wxHyperlinkCtrl* m_Help;

    /// Control identifiers
    enum {
        ID_CWIZARDDLG = 10115,
        ID_HELP_LINK = 10116,
        ID_BUTTON_OPTIONS = 10010
    };
////@end CWizardDlg member variables

protected:
    IWizardPage* m_CurrentPage;
};

END_NCBI_SCOPE

#endif //GUI_WIDGETS_WX__WIZARD_DLG__HPP
