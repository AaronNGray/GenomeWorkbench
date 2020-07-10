/*  $Id: wx_track_name_dlg.hpp 27016 2012-12-07 21:34:14Z wuliangs $
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
 * Authors:  Vlad Lebedev
 *
 * File Description:
 *
 */
#ifndef _WX_TRACK_NAME_DLG_H_
#define _WX_TRACK_NAME_DLG_H_


/*!
 * Includes
 */

////@begin includes
////@end includes
#include <corelib/ncbiobj.hpp>
#include <wx/dialog.h>
#include <wx/sizer.h>

/*!
 * Forward declarations
 */

////@begin forward declarations
////@end forward declarations
class wxStaticText;

BEGIN_NCBI_SCOPE


/*!
 * Control identifiers
 */

////@begin control identifiers
#define ID_CWXTRACKNAMEDLG 10019
#define ID_TEXTCTRL 10000
#define SYMBOL_CWXTRACKNAMEDLG_STYLE wxCAPTION|wxRESIZE_BORDER|wxSYSTEM_MENU|wxCLOSE_BOX|wxTAB_TRAVERSAL
#define SYMBOL_CWXTRACKNAMEDLG_TITLE _("New track title")
#define SYMBOL_CWXTRACKNAMEDLG_IDNAME ID_CWXTRACKNAMEDLG
#define SYMBOL_CWXTRACKNAMEDLG_SIZE wxSize(400, 300)
#define SYMBOL_CWXTRACKNAMEDLG_POSITION wxDefaultPosition
////@end control identifiers


/*!
 * CwxTrackNameDlg class declaration
 */

class CwxTrackNameDlg: public wxDialog
{    
    DECLARE_DYNAMIC_CLASS( CwxTrackNameDlg )
    DECLARE_EVENT_TABLE()

public:
    CwxTrackNameDlg();
    CwxTrackNameDlg( wxWindow* parent, wxWindowID id = SYMBOL_CWXTRACKNAMEDLG_IDNAME, const wxString& caption = SYMBOL_CWXTRACKNAMEDLG_TITLE, const wxPoint& pos = SYMBOL_CWXTRACKNAMEDLG_POSITION, const wxSize& size = SYMBOL_CWXTRACKNAMEDLG_SIZE, long style = SYMBOL_CWXTRACKNAMEDLG_STYLE );

    bool CreateX( wxWindow* parent, wxWindowID id = SYMBOL_CWXTRACKNAMEDLG_IDNAME, const wxString& caption = SYMBOL_CWXTRACKNAMEDLG_TITLE, const wxPoint& pos = SYMBOL_CWXTRACKNAMEDLG_POSITION, const wxSize& size = SYMBOL_CWXTRACKNAMEDLG_SIZE, long style = SYMBOL_CWXTRACKNAMEDLG_STYLE );

    ~CwxTrackNameDlg();

    void Init();

    void CreateControls();

////@begin CwxTrackNameDlg event handler declarations

    void OnOkClick( wxCommandEvent& event );

////@end CwxTrackNameDlg event handler declarations

////@begin CwxTrackNameDlg member function declarations

    wxBitmap GetBitmapResource( const wxString& name );

    wxIcon GetIconResource( const wxString& name );
////@end CwxTrackNameDlg member function declarations
    
    void SetMsg(const wxString& msg);
    void SetTrackName(const wxString& name);
    wxString GetTrackName() const;

    static bool ShowToolTips();

////@begin CwxTrackNameDlg member variables
    wxStaticText* m_Msg;
    wxTextCtrl* m_TrackName;
////@end CwxTrackNameDlg member variables
};

END_NCBI_SCOPE

#endif
    // _WX_TRACK_NAME_DLG_H_
