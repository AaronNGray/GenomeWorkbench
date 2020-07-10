/*  $Id: wx_marker_name_dlg.hpp 27017 2012-12-07 21:51:00Z wuliangs $
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
 * Authors:  Liangshou Wu
 *
 * File Description:
 *
 */
#ifndef _WX_MARKER_NAME_DLG_H_
#define _WX_MARKER_NAME_DLG_H_


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
#include <wx/button.h>

/*!
 * Forward declarations
 */

class wxStaticText;
class wxTextCtrl;

////@begin forward declarations
////@end forward declarations

BEGIN_NCBI_SCOPE

/*!
 * Control identifiers
 */

////@begin control identifiers
#define ID_CWXTRACKNAMEDLG 10019
#define ID_TEXTCTRL 10000
#define SYMBOL_CWXMARKERNAMEDLG_STYLE wxCAPTION|wxRESIZE_BORDER|wxSYSTEM_MENU|wxCLOSE_BOX|wxTAB_TRAVERSAL
#define SYMBOL_CWXMARKERNAMEDLG_TITLE _("Rename Marker")
#define SYMBOL_CWXMARKERNAMEDLG_IDNAME ID_CWXTRACKNAMEDLG
#define SYMBOL_CWXMARKERNAMEDLG_SIZE wxSize(400, 300)
#define SYMBOL_CWXMARKERNAMEDLG_POSITION wxDefaultPosition
////@end control identifiers


/*!
 * CwxMarkerNameDlg class declaration
 */

class CwxMarkerNameDlg: public wxDialog
{    
    DECLARE_DYNAMIC_CLASS( CwxMarkerNameDlg )
    DECLARE_EVENT_TABLE()

public:
    CwxMarkerNameDlg();
    CwxMarkerNameDlg( wxWindow* parent, wxWindowID id = SYMBOL_CWXMARKERNAMEDLG_IDNAME, const wxString& caption = SYMBOL_CWXMARKERNAMEDLG_TITLE, const wxPoint& pos = SYMBOL_CWXMARKERNAMEDLG_POSITION, const wxSize& size = SYMBOL_CWXMARKERNAMEDLG_SIZE, long style = SYMBOL_CWXMARKERNAMEDLG_STYLE );

    bool Create( wxWindow* parent, wxWindowID id = SYMBOL_CWXMARKERNAMEDLG_IDNAME, const wxString& caption = SYMBOL_CWXMARKERNAMEDLG_TITLE, const wxPoint& pos = SYMBOL_CWXMARKERNAMEDLG_POSITION, const wxSize& size = SYMBOL_CWXMARKERNAMEDLG_SIZE, long style = SYMBOL_CWXMARKERNAMEDLG_STYLE );

    ~CwxMarkerNameDlg();

    void Init();

    void CreateControls();

    void SetMarkerName(const string& name);
    string GetMarkerName() const;

////@begin CwxMarkerNameDlg event handler declarations

    void OnOkClick( wxCommandEvent& event );

////@end CwxMarkerNameDlg event handler declarations

////@begin CwxMarkerNameDlg member function declarations

    wxBitmap GetBitmapResource( const wxString& name );

    wxIcon GetIconResource( const wxString& name );
////@end CwxMarkerNameDlg member function declarations

    static bool ShowToolTips();

////@begin CwxMarkerNameDlg member variables
    wxStaticText* m_Msg;
    wxTextCtrl* m_MarkerName;
////@end CwxMarkerNameDlg member variables
};

END_NCBI_SCOPE

#endif
    // _WX_MARKER_NAME_DLG_H_
