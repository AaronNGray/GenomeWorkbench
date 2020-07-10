/*  $Id: wx_marker_info_dlg.hpp 27952 2013-04-30 16:42:49Z wuliangs $
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
#ifndef _WX_MARKER_INFO_DLG_H_
#define _WX_MARKER_INFO_DLG_H_


/*!
 * Includes
 */
#include <corelib/ncbistd.hpp>
#include <gui/gui_export.h>
#include <wx/dialog.h>

#include "wx_marker_info_panel.hpp"

////@begin includes
#include "wx/statline.h"
////@end includes

/*!
 * Forward declarations
 */

////@begin forward declarations
class wxBoxSizer;
////@end forward declarations
class wxScrolledWindow;

BEGIN_NCBI_SCOPE

/*!
 * Control identifiers
 */

////@begin control identifiers
#define ID_CWXMARKERINFODLG 10045
#define RemoveAllButton 10046
#define ID_SCROLLEDWINDOW 10056
#define SYMBOL_CWXMARKERINFODLG_STYLE wxCAPTION|wxRESIZE_BORDER|wxSYSTEM_MENU|wxCLOSE_BOX|wxTAB_TRAVERSAL
#define SYMBOL_CWXMARKERINFODLG_TITLE _("Marker Info")
#define SYMBOL_CWXMARKERINFODLG_IDNAME ID_CWXMARKERINFODLG
#define SYMBOL_CWXMARKERINFODLG_SIZE wxSize(680, 600)
#define SYMBOL_CWXMARKERINFODLG_POSITION wxDefaultPosition
////@end control identifiers

class CSeqMarkerEvent;


/*!
 * CwxMarkerInfoDlg class declaration
 */

class CwxMarkerInfoDlg: public wxDialog
{    
    DECLARE_DYNAMIC_CLASS( CwxMarkerInfoDlg )
    DECLARE_EVENT_TABLE()

public:
    typedef vector<SMarkerInfo> SMarkerInfoVec;

    CwxMarkerInfoDlg();
    CwxMarkerInfoDlg(wxWindow* parent, wxEvtHandler* eventHandler,
        wxWindowID id = SYMBOL_CWXMARKERINFODLG_IDNAME,
        const wxString& caption = SYMBOL_CWXMARKERINFODLG_TITLE,
        const wxPoint& pos = SYMBOL_CWXMARKERINFODLG_POSITION,
        const wxSize& size = SYMBOL_CWXMARKERINFODLG_SIZE,
        long style = SYMBOL_CWXMARKERINFODLG_STYLE );

    bool Create( wxWindow* parent,
        wxWindowID id = SYMBOL_CWXMARKERINFODLG_IDNAME,
        const wxString& caption = SYMBOL_CWXMARKERINFODLG_TITLE,
        const wxPoint& pos = SYMBOL_CWXMARKERINFODLG_POSITION,
        const wxSize& size = SYMBOL_CWXMARKERINFODLG_SIZE,
        long style = SYMBOL_CWXMARKERINFODLG_STYLE );

    ~CwxMarkerInfoDlg();

    void Init();

    void CreateControls();

    void UpdateMarker(const SMarkerInfoVec& info_vec);
    void RemoveAllMarkers();

    virtual bool Show(bool show = true);

////@begin CwxMarkerInfoDlg event handler declarations

    void OnCloseWindow( wxCloseEvent& event );

    void OnRemoveAllButtonClick( wxCommandEvent& event );

    void OnCancelClick( wxCommandEvent& event );

////@end CwxMarkerInfoDlg event handler declarations

////@begin CwxMarkerInfoDlg member function declarations

    wxBitmap GetBitmapResource( const wxString& name );

    wxIcon GetIconResource( const wxString& name );
////@end CwxMarkerInfoDlg member function declarations

    static bool ShowToolTips();
private:
    void x_Send(CSeqMarkerEvent& event);

////@begin CwxMarkerInfoDlg member variables
    wxScrolledWindow* m_TopPanel;
    wxBoxSizer* m_PanelSizer;
////@end CwxMarkerInfoDlg member variables

    list<CwxMarkerInfoPanel*> m_MarkerPanels;
    wxEvtHandler* m_EventHandler;
};

class CSeqMarkerDlgEvent : public wxCommandEvent
{
public:
    CSeqMarkerDlgEvent(wxEventType commandType = wxEVT_NULL, int id = 0)
        : wxCommandEvent(commandType, id) { }

    const string& GetMarkerId() const { return m_MarkerId; }
    void SetMarkerId(const string& str) { m_MarkerId = str; }
private:
    DECLARE_DYNAMIC_CLASS_NO_COPY(CSeqMarkerDlgEvent)

    string m_MarkerId;
};



BEGIN_DECLARE_EVENT_TYPES()
    DECLARE_EXPORTED_EVENT_TYPE(NCBI_GUIWIDGETS_SEQ_EXPORT, wxEVT_COMMAND_REMOVE_ALL_MARKERS, 520)
    DECLARE_EXPORTED_EVENT_TYPE(NCBI_GUIWIDGETS_SEQ_EXPORT, wxEVT_COMMAND_MARKER_INFO_CLOSE, 521)
END_DECLARE_EVENT_TYPES()

typedef void (wxEvtHandler::*CSeqMarkerDlgEventFunction)(CSeqMarkerDlgEvent&);

#define CSeqMarkerDlgEventHandler(func) \
    (wxObjectEventFunction)(wxEventFunction)wxStaticCastEvent(CSeqMarkerDlgEventFunction, &func)

#define EVT_REMOVE_ALL_MARKERS(id, fn) \
    wx__DECLARE_EVT1(wxEVT_COMMAND_REMOVE_ALL_MARKERS, id, CSeqMarkerDlgEventHandler(fn))

#define EVT_MARKER_INFO_CLOSE(id, fn) \
    wx__DECLARE_EVT1(wxEVT_COMMAND_MARKER_INFO_CLOSE, id, CSeqMarkerDlgEventHandler(fn))



END_NCBI_SCOPE

#endif
    // _WX_MARKER_INFO_DLG_H_
