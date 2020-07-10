/*  $Id: wx_track_settings_dlg.hpp 23952 2011-06-23 19:12:28Z wuliangs $
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
#ifndef _WX_TRACK_SETTINGS_DLG_H_
#define _WX_TRACK_SETTINGS_DLG_H_


/*!
 * Includes
 */

////@begin includes
////@end includes
#include <corelib/ncbiobj.hpp>
#include <wx/dialog.h>
#include <gui/widgets/seq_graphic/track_config_manager.hpp>

/*!
 * Forward declarations
 */

////@begin forward declarations
class wxBoxSizer;
////@end forward declarations
class wxChoice;
class wxComboBox;

BEGIN_NCBI_SCOPE

/*!
 * Control identifiers
 */

////@begin control identifiers
#define ID_CWXTRACKSETTINGSDLG 10035
#define ID_CHOICE 10002
#define ID_CHOICE1 10001
#define ID_TEXTCTRL 10000
#define ID_TEXTCTRL1 10003
#define ID_COMBOBOX 10004
#define SYMBOL_CWXTRACKSETTINGSDLG_STYLE wxCAPTION|wxRESIZE_BORDER|wxSYSTEM_MENU|wxCLOSE_BOX|wxTAB_TRAVERSAL
#define SYMBOL_CWXTRACKSETTINGSDLG_TITLE _("Graphical view new track settings")
#define SYMBOL_CWXTRACKSETTINGSDLG_IDNAME ID_CWXTRACKSETTINGSDLG
#define SYMBOL_CWXTRACKSETTINGSDLG_SIZE wxSize(400, 100)
#define SYMBOL_CWXTRACKSETTINGSDLG_POSITION wxDefaultPosition
////@end control identifiers


/*!
 * CwxTrackSettingsDlg class declaration
 */

class CwxTrackSettingsDlg: public wxDialog
{    
    DECLARE_DYNAMIC_CLASS( CwxTrackSettingsDlg )
    DECLARE_EVENT_TABLE()

public:
    CwxTrackSettingsDlg();
    CwxTrackSettingsDlg( wxWindow* parent, wxWindowID id = SYMBOL_CWXTRACKSETTINGSDLG_IDNAME, const wxString& caption = SYMBOL_CWXTRACKSETTINGSDLG_TITLE, const wxPoint& pos = SYMBOL_CWXTRACKSETTINGSDLG_POSITION, const wxSize& size = SYMBOL_CWXTRACKSETTINGSDLG_SIZE, long style = SYMBOL_CWXTRACKSETTINGSDLG_STYLE );

    bool CreateX( wxWindow* parent, wxWindowID id = SYMBOL_CWXTRACKSETTINGSDLG_IDNAME, const wxString& caption = SYMBOL_CWXTRACKSETTINGSDLG_TITLE, const wxPoint& pos = SYMBOL_CWXTRACKSETTINGSDLG_POSITION, const wxSize& size = SYMBOL_CWXTRACKSETTINGSDLG_SIZE, long style = SYMBOL_CWXTRACKSETTINGSDLG_STYLE );

    ~CwxTrackSettingsDlg();

    void Init();

    void CreateControls();

////@begin CwxTrackSettingsDlg event handler declarations

    void OnTrackTypeChanged( wxCommandEvent& event );

    void OnOkClick( wxCommandEvent& event );

////@end CwxTrackSettingsDlg event handler declarations

////@begin CwxTrackSettingsDlg member function declarations

    wxBitmap GetBitmapResource( const wxString& name );

    wxIcon GetIconResource( const wxString& name );
////@end CwxTrackSettingsDlg member function declarations

/// user-defined member functions.
    void SetTrackName(const wxString& name);
    void InitTrackTypes(const CTrackConfigManager::TFactoryMap& factories);

    wxString GetTrackName() const;
    wxString GetTrackFilter() const;
    wxString GetTrackType() const;
    wxString GetSubtype() const;
    wxString GetAnnots() const;

    static bool ShowToolTips();

////@begin CwxTrackSettingsDlg member variables
    wxChoice* m_TrackType;
    wxBoxSizer* m_SubtypeGroup;
    wxChoice* m_Subtype;
    wxTextCtrl* m_TrackName;
    wxTextCtrl* m_TrackFilter;
    wxComboBox* m_Annots;
////@end CwxTrackSettingsDlg member variables
};

END_NCBI_SCOPE

#endif
    // _WX_TRACK_SETTINGS_DLG_H_
