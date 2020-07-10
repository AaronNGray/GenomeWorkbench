/*  $Id: macro_error_dlg.hpp 44338 2019-12-03 15:36:51Z asztalos $
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
 * Authors:  
 */
#ifndef _MACRO_ERROR_DLG_H_
#define _MACRO_ERROR_DLG_H_

#include <corelib/ncbistd.hpp>
#include <gui/gui_export.h>
#include <wx/dialog.h>

class wxBoxSizer;
class wxHyperlinkCtrl;
class wxHyperlinkEvent;
class wxTextCtrl;
class wxStaticText;


/*!
 * Control identifiers
 */

////@begin control identifiers
#define SYMBOL_CMACROERRORDLG_STYLE wxCAPTION|wxRESIZE_BORDER|wxSYSTEM_MENU|wxCLOSE_BOX|wxTAB_TRAVERSAL
#define SYMBOL_CMACROERRORDLG_TITLE _("Macro Error")
#define SYMBOL_CMACROERRORDLG_IDNAME ID_CMACROERRORDLG
#define SYMBOL_CMACROERRORDLG_SIZE wxSize(400, 300)
#define SYMBOL_CMACROERRORDLG_POSITION wxDefaultPosition
////@end control identifiers

BEGIN_NCBI_SCOPE

BEGIN_SCOPE(macro)
class CMacroDataException;
END_SCOPE(macro)

/*!
 * CMacroErrorDlg class declaration
 */

class NCBI_GUIWIDGETS_DATA_EXPORT CMacroErrorDlg : public wxDialog
{    
    DECLARE_DYNAMIC_CLASS( CMacroErrorDlg )
    DECLARE_EVENT_TABLE()

public:
    /// Constructors
    CMacroErrorDlg();
    CMacroErrorDlg(wxWindow* parent, 
        wxWindowID id = SYMBOL_CMACROERRORDLG_IDNAME, 
        const wxString& caption = SYMBOL_CMACROERRORDLG_TITLE, 
        const wxPoint& pos = SYMBOL_CMACROERRORDLG_POSITION, 
        const wxSize& size = SYMBOL_CMACROERRORDLG_SIZE, 
        long style = SYMBOL_CMACROERRORDLG_STYLE);

    /// Creation
    bool Create( wxWindow* parent, 
        wxWindowID id = SYMBOL_CMACROERRORDLG_IDNAME, 
        const wxString& caption = SYMBOL_CMACROERRORDLG_TITLE, 
        const wxPoint& pos = SYMBOL_CMACROERRORDLG_POSITION, 
        const wxSize& size = SYMBOL_CMACROERRORDLG_SIZE, 
        long style = SYMBOL_CMACROERRORDLG_STYLE );

    /// Destructor
    ~CMacroErrorDlg();

    /// Initialises member variables
    void Init();

    /// Creates the controls and sizers
    void CreateControls();

    void SetException(const string &message, const CException &error);
    void SetMessage(const string &message);
    void SetReport(const string &report);
    void SetData(const string &data);

////@begin CMacroErrorDlg event handler declarations

    /// wxEVT_COMMAND_HYPERLINK event handler for ID_ERROR_DETALS_HYPERLINKCTRL
    void OnErrorDetalsClicked( wxHyperlinkEvent& event );

    /// wxEVT_COMMAND_HYPERLINK event handler for ID_DATA_DETAILS_HYPERLINKCTRL
    void OnDataDetailsClicked( wxHyperlinkEvent& event );

    /// wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_FEEDBACK_BUTTON
    void OnFeedbackButtonClick( wxCommandEvent& event );

////@end CMacroErrorDlg event handler declarations

////@begin CMacroErrorDlg member function declarations

    /// Retrieves bitmap resources
    wxBitmap GetBitmapResource( const wxString& name );

    /// Retrieves icon resources
    wxIcon GetIconResource( const wxString& name );
////@end CMacroErrorDlg member function declarations

    /// Should we show tooltips?
    static bool ShowToolTips();

////@begin CMacroErrorDlg member variables
    wxBoxSizer* m_VSizer;
    wxStaticText* m_ErrorTextCtrl;
    wxHyperlinkCtrl* m_ErrorDetailsLink;
    wxTextCtrl* m_ErrorDetailsCtrl;
    wxHyperlinkCtrl* m_DataDetailsLink;
    wxTextCtrl* m_DataDetailsCtrl;
    /// Control identifiers
    enum {
        ID_CMACROERRORDLG = 10000,
        wxID_ERROR_STATIC = 10003,
        ID_ERROR_DETALS_HYPERLINKCTRL = 10001,
        ID_ERROR_DETAILS_TEXTCTRL = 10002,
        ID_DATA_DETAILS_HYPERLINKCTRL = 10003,
        ID_DATA_DETAILS_TEXTCTRL = 10004,
        ID_FEEDBACK_BUTTON = 10005
    };
////@end CMacroErrorDlg member variables

protected:
    string x_MergeExceptionMessages(const CException &error) const;
    const macro::CMacroDataException* x_FindDataException(const CException &error) const;
};

END_NCBI_SCOPE

#endif
    // _MACRO_ERROR_DLG_H_
