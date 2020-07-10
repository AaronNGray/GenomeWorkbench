/*  $Id: wx_save_pdf_base_dlg.hpp 33822 2015-09-17 20:23:11Z katargir $
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
 * Authors: Liangshou Wu, Bob Falk
 */
#ifndef _WX_SAVE_PDF_BASE_DLG_H_
#define _WX_SAVE_PDF_BASE_DLG_H_

#include <corelib/ncbistd.hpp>

/*!
 * Includes
 */

////@begin includes
////@end includes

#include <corelib/ncbiobj.hpp>
#include <wx/dialog.h>
#include <wx/sizer.h>
#include <wx/textctrl.h>
#include <wx/stattext.h>
#include <wx/bmpbuttn.h>
#include <wx/timer.h>
#include <wx/checkbox.h>
#include <gui/opengl/glpane.hpp>

/*!
 * Forward declarations
 */

////@begin forward declarations
class wxBoxSizer;
////@end forward declarations
class wxPanel;

/*!
 * Control identifiers
 */

BEGIN_NCBI_SCOPE

////@begin control identifiers
#define SYMBOL_CWXSAVEPDFBASEDLG_STYLE wxCAPTION|wxSYSTEM_MENU|wxCLOSE_BOX|wxTAB_TRAVERSAL
#define SYMBOL_CWXSAVEPDFBASEDLG_TITLE _("Save Image As PDF")
#define SYMBOL_CWXSAVEPDFBASEDLG_IDNAME ID_CWXSAVEPDFDLG
#define SYMBOL_CWXSAVEPDFBASEDLG_SIZE wxSize(440, -1)
#define SYMBOL_CWXSAVEPDFBASEDLG_POSITION wxDefaultPosition
////@end control identifiers


/*!
 * CwxSavePdfBaseDlg class declaration
 */

class NCBI_GUIWIDGETS_WX_EXPORT CwxSavePdfBaseDlg: public wxDialog
{    
    DECLARE_DYNAMIC_CLASS( CwxSavePdfBaseDlg )
    DECLARE_EVENT_TABLE()

public:
    /// Constructors
    CwxSavePdfBaseDlg();
    CwxSavePdfBaseDlg( wxWindow* parent, wxWindowID id = SYMBOL_CWXSAVEPDFBASEDLG_IDNAME, const wxString& caption = SYMBOL_CWXSAVEPDFBASEDLG_TITLE, const wxPoint& pos = SYMBOL_CWXSAVEPDFBASEDLG_POSITION, const wxSize& size = SYMBOL_CWXSAVEPDFBASEDLG_SIZE, long style = SYMBOL_CWXSAVEPDFBASEDLG_STYLE );

    /// Creation
    bool Create( wxWindow* parent, wxWindowID id = SYMBOL_CWXSAVEPDFBASEDLG_IDNAME, const wxString& caption = SYMBOL_CWXSAVEPDFBASEDLG_TITLE, const wxPoint& pos = SYMBOL_CWXSAVEPDFBASEDLG_POSITION, const wxSize& size = SYMBOL_CWXSAVEPDFBASEDLG_SIZE, long style = SYMBOL_CWXSAVEPDFBASEDLG_STYLE );

    /// Destructor
    ~CwxSavePdfBaseDlg();

    /// Initialises member variables
    void Init();

    /// Creates the controls and sizers
    void CreateControls();

    void LoadSettings(const string& pdf_base_key);
    void SaveSettings(const string& pdf_base_key) const;

////@begin CwxSavePdfBaseDlg event handler declarations

    /// wxEVT_COMMAND_TEXT_UPDATED event handler for ID_TEXTCTRL5
    void OnTextctrl5TextUpdated( wxCommandEvent& event );

    /// wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_BITMAPBUTTON1
    void OnFilepathClick( wxCommandEvent& event );

    /// wxEVT_COMMAND_CHECKBOX_CLICKED event handler for ID_TOOLTIP_CHECKBOX
    void OnTooltipCheckboxClick( wxCommandEvent& event );

    /// wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_SAVE
    void OnSaveClick( wxCommandEvent& event );

    /// wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_OPEN
    void OnOpenClick( wxCommandEvent& event );

    /// wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_CANCEL
    void OnCancelClick( wxCommandEvent& event );

////@end CwxSavePdfBaseDlg event handler declarations

////@begin CwxSavePdfBaseDlg member function declarations

    /// Retrieves bitmap resources
    wxBitmap GetBitmapResource( const wxString& name );

    /// Retrieves icon resources
    wxIcon GetIconResource( const wxString& name );
////@end CwxSavePdfBaseDlg member function declarations

    /// Should we show tooltips?
    static bool ShowToolTips();

protected:


    virtual void x_SavePdf() {}

////@begin CwxSavePdfBaseDlg member variables
    wxPanel* m_Panel;
    wxTextCtrl* m_FileNameBox;
    wxBoxSizer* m_SaveOptionsSizer;
    wxCheckBox* m_IncludeToolTips;
    wxBoxSizer* m_InfoSizer;
    wxStaticText* m_Info;
    wxButton* m_SaveButton;
    wxButton* m_OpenButton;
    /// Control identifiers
    enum {
        ID_CWXSAVEPDFDLG = 10011,
        ID_PANEL3 = 10015,
        ID_TEXTCTRL5 = 10016,
        ID_BITMAPBUTTON1 = 10017,
        ID_TOOLTIP_CHECKBOX = 10018,
        ID_SAVE = 10019,
        ID_OPEN = 10020,
        ID_CANCEL = 10021
    };
////@end CwxSavePdfBaseDlg member variables

    wxTimer m_Timer;
    //CIndProgressBar* m_ProgressBar;
    string           m_Path;
    string           m_FileName;
    bool             m_ReadyToSave;
    /// view PDF file after finished.
    bool             m_OpenPdf;
    bool             m_UserSelectedFilename;
};

END_NCBI_SCOPE

#endif // _WX_SAVE_PDF_BASE_DLG_H_
