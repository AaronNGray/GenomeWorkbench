/*  $Id: wx_marker_info_panel.hpp 27058 2012-12-13 14:34:04Z wuliangs $
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
#ifndef _WX_MARKER_INFO_PANEL_H_
#define _WX_MARKER_INFO_PANEL_H_


/*!
 * Includes
 */

#include <wx/panel.h>
#include <string>


////@begin includes
#include "wx/grid.h"
////@end includes

/*!
 * Forward declarations
 */

////@begin forward declarations
class wxGrid;
////@end forward declarations
class wxStaticText;

BEGIN_NCBI_SCOPE

/*!
 * Control identifiers
 */

////@begin control identifiers
#define ID_CWXMARKERINFOPANEL 10047
#define ID_PANEL 10041
#define RemoveButton 10044
#define ID_GRID 10042
#define SYMBOL_CWXMARKERINFOPANEL_STYLE wxTAB_TRAVERSAL
#define SYMBOL_CWXMARKERINFOPANEL_TITLE _("Marker Info Panel")
#define SYMBOL_CWXMARKERINFOPANEL_IDNAME ID_CWXMARKERINFOPANEL
#define SYMBOL_CWXMARKERINFOPANEL_SIZE wxDefaultSize
#define SYMBOL_CWXMARKERINFOPANEL_POSITION wxDefaultPosition
////@end control identifiers


struct SMarkerCoord
{
    string m_Acc;
    string m_Loc;
    string m_LocRef;
    string m_Hgvs;
    string m_Seq;
};

typedef vector<SMarkerCoord> SMarkerCoordVec;

struct SMarkerInfo
{
    string m_Id;
    string m_Label;
    size_t m_Pos;
    SMarkerCoordVec m_CoordVec;
};

/*!
 * CwxMarkerInfoPanel class declaration
 */

class CwxMarkerInfoPanel: public wxPanel
{    
    DECLARE_DYNAMIC_CLASS( CwxMarkerInfoPanel )
    DECLARE_EVENT_TABLE()

public:
    CwxMarkerInfoPanel();
    CwxMarkerInfoPanel(const wxString& marker_id, const wxString& marker_label,
        size_t marker_pos, wxWindow* parent, wxEvtHandler* eventHandler,
        wxWindowID id = SYMBOL_CWXMARKERINFOPANEL_IDNAME,
        const wxPoint& pos = SYMBOL_CWXMARKERINFOPANEL_POSITION,
        const wxSize& size = SYMBOL_CWXMARKERINFOPANEL_SIZE,
        long style = SYMBOL_CWXMARKERINFOPANEL_STYLE );

    bool Create( wxWindow* parent, wxWindowID id = SYMBOL_CWXMARKERINFOPANEL_IDNAME,
        const wxPoint& pos = SYMBOL_CWXMARKERINFOPANEL_POSITION,
        const wxSize& size = SYMBOL_CWXMARKERINFOPANEL_SIZE,
        long style = SYMBOL_CWXMARKERINFOPANEL_STYLE );

    ~CwxMarkerInfoPanel();

    void Init();

    void CreateControls();

    void UpdateMarker(const SMarkerInfo& marker_info);
    string GetMarkerId() const;

////@begin CwxMarkerInfoPanel event handler declarations

    void OnRemoveButtonClick( wxCommandEvent& event );

////@end CwxMarkerInfoPanel event handler declarations

////@begin CwxMarkerInfoPanel member function declarations

    wxBitmap GetBitmapResource( const wxString& name );

    wxIcon GetIconResource( const wxString& name );
////@end CwxMarkerInfoPanel member function declarations

    static bool ShowToolTips();

private:
    void x_UpdateMarkerLabel();

////@begin CwxMarkerInfoPanel member variables
    wxStaticText* m_MarkerName;
    wxGrid* m_MarkerInfoGrid;
////@end CwxMarkerInfoPanel member variables
    wxEvtHandler* m_EventHandler;

    wxString m_MarkerId;
    wxString m_MarkerLabel;
    size_t   m_MarkerPos;
};


class CSeqMarkerEvent : public wxCommandEvent
{
public:
    CSeqMarkerEvent(wxEventType commandType = wxEVT_NULL, int id = 0)
        : wxCommandEvent(commandType, id) { }

    const string& GetMarkerId() const { return m_MarkerId; }
    void SetMarkerId(const string& str) { m_MarkerId = str; }
private:
    DECLARE_DYNAMIC_CLASS_NO_COPY(CSeqMarkerEvent)

    string m_MarkerId;
};


BEGIN_DECLARE_EVENT_TYPES()
    DECLARE_EXPORTED_EVENT_TYPE(NCBI_GUIWIDGETS_SEQ_EXPORT, wxEVT_COMMAND_REMOVE_MARKER, 523)
END_DECLARE_EVENT_TYPES()

typedef void (wxEvtHandler::*CSeqMarkerEventFunction)(CSeqMarkerEvent&);

#define CSeqMarkerEventHandler(func) \
    (wxObjectEventFunction)(wxEventFunction)wxStaticCastEvent(CSeqMarkerEventFunction, &func)

#define EVT_REMOVE_MARKER(id, fn) \
    wx__DECLARE_EVT1(wxEVT_COMMAND_REMOVE_MARKER, id, CSeqMarkerEventHandler(fn))


END_NCBI_SCOPE

#endif
    // _WX_MARKER_INFO_PANEL_H_
