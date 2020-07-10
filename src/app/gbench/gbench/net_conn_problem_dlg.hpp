#ifndef GUI_CORE___NET_CONN_PROBLEM_DLG__HPP
#define GUI_CORE___NET_CONN_PROBLEM_DLG__HPP

/*  $Id: net_conn_problem_dlg.hpp 22551 2010-11-17 21:48:55Z dicuccio $
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

/*!
 * Includes
 */

#include <corelib/ncbistd.hpp>

////@begin includes
#include "wx/valtext.h"
#include "wx/hyperlink.h"
#include "wx/textctrl.h"
////@end includes

/*!
 * Forward declarations
 */

////@begin forward declarations
////@end forward declarations

#include <wx/dialog.h>

/*!
 * Control identifiers
 */

BEGIN_NCBI_SCOPE

////@begin control identifiers
#define ID_CNETCONNPROBLEMDLG 10038
#define ID_TEXTCTRL 10057
#define ID_HYPERLINKCTRL1 10058
#define ID_BUTTON1 10024
#define SYMBOL_CNETCONNPROBLEMDLG_STYLE wxCAPTION|wxRESIZE_BORDER|wxSYSTEM_MENU|wxCLOSE_BOX|wxTAB_TRAVERSAL
#define SYMBOL_CNETCONNPROBLEMDLG_TITLE _("Network connection problem")
#define SYMBOL_CNETCONNPROBLEMDLG_IDNAME ID_CNETCONNPROBLEMDLG
#define SYMBOL_CNETCONNPROBLEMDLG_SIZE wxSize(400, 300)
#define SYMBOL_CNETCONNPROBLEMDLG_POSITION wxDefaultPosition
////@end control identifiers


/*!
 * CNetConnProblemDlg class declaration
 */

class CNetConnProblemDlg: public wxDialog
{    
    DECLARE_DYNAMIC_CLASS( CNetConnProblemDlg )
    DECLARE_EVENT_TABLE()

public:
    CNetConnProblemDlg();
    CNetConnProblemDlg( wxWindow* parent, wxWindowID id = SYMBOL_CNETCONNPROBLEMDLG_IDNAME, const wxString& caption = SYMBOL_CNETCONNPROBLEMDLG_TITLE, const wxPoint& pos = SYMBOL_CNETCONNPROBLEMDLG_POSITION, const wxSize& size = SYMBOL_CNETCONNPROBLEMDLG_SIZE, long style = SYMBOL_CNETCONNPROBLEMDLG_STYLE );

    bool Create( wxWindow* parent, wxWindowID id = SYMBOL_CNETCONNPROBLEMDLG_IDNAME, const wxString& caption = SYMBOL_CNETCONNPROBLEMDLG_TITLE, const wxPoint& pos = SYMBOL_CNETCONNPROBLEMDLG_POSITION, const wxSize& size = SYMBOL_CNETCONNPROBLEMDLG_SIZE, long style = SYMBOL_CNETCONNPROBLEMDLG_STYLE );

    ~CNetConnProblemDlg();

    void Init();

    void CreateControls();

////@begin CNetConnProblemDlg event handler declarations

    /// wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_BUTTON1
    void OnSendFeedback( wxCommandEvent& event );

////@end CNetConnProblemDlg event handler declarations

////@begin CNetConnProblemDlg member function declarations

    wxString GetDescr() const { return m_Descr ; }
    void SetDescr(wxString value) { m_Descr = value ; }

    /// Retrieves bitmap resources
    wxBitmap GetBitmapResource( const wxString& name );

    /// Retrieves icon resources
    wxIcon GetIconResource( const wxString& name );
////@end CNetConnProblemDlg member function declarations

    static bool ShowToolTips();

////@begin CNetConnProblemDlg member variables
    wxTextCtrl* m_DescrCtrl;
private:
    wxString m_Descr;
////@end CNetConnProblemDlg member variables
};

END_NCBI_SCOPE

#endif  // GUI_CORE___NET_CONN_PROBLEM_DLG__HPP
