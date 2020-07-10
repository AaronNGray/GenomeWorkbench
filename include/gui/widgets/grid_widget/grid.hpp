#ifndef GUI_WIDGETS_GRID_WIDGET__GRID__HPP
#define GUI_WIDGETS_GRID_WIDGET__GRID__HPP

/*  $Id: grid.hpp 34121 2015-11-06 13:47:30Z evgeniev $
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

#include <wx/grid.h>
#include <wx/imaglist.h>
#include <wx/hyperlink.h>
#include <wx/timer.h>

#include <gui/widgets/grid_widget/cell_hyperlink.hpp>

BEGIN_NCBI_SCOPE

/////////////////////////////////////////////////////////////////////////////
/// CGrid
class NCBI_GUIWIDGETS_GRID_WIDGET_EXPORT CGrid : public wxGrid
{
public:
    CGrid(wxWindow* parent, wxWindowID id,
            const wxPoint& pos = wxDefaultPosition,
            const wxSize& size = wxDefaultSize,
            long style = wxWANTS_CHARS,
            const wxString& name = wxGridNameStr);

    virtual void DrawColLabel( wxDC& dc, int col );

    void OnGridLabelLeftClick(wxGridEvent& evt);
    void OnGridLabelRightClick(wxGridEvent& evt);
    void OnShowHideColumn(wxCommandEvent& event);
    void OnFilterColumns(wxCommandEvent& event);

    void OnTimer(wxTimerEvent& event);

    CGridHyperlinks& GetHyperlinks()             { return m_HyperLinks; }
    const CGridHyperlinks& GetHyperlinks() const { return m_HyperLinks; }

protected:
    DECLARE_EVENT_TABLE()

    void OnMouseMove(wxMouseEvent& event);
    void OnLeftDown(wxMouseEvent& event);

    void x_ShowTooltip(bool show);

    wxImageList m_ImageList;

    CGridHyperlinks m_HyperLinks;
    const CCellHyperlinks::CHyperlink* m_CurLink;

// ToolTip members
    wxTimer          m_Timer;
    wxGridCellCoords m_MouseOverCell;
};

wxDECLARE_EXPORTED_EVENT( NCBI_GUIWIDGETS_GRID_WIDGET_EXPORT, wxEVT_COMMAND_HYPERLINK_HOVER, wxHyperlinkEvent );

typedef void (wxEvtHandler::*CHyperlinkHoverFunction)(wxHyperlinkEvent&);

#define CHyperlinkHoverHandler(func) \
    (wxObjectEventFunction)(wxEventFunction)wxStaticCastEvent(CHyperlinkHoverFunction, &func)

#define EVT_HYPERLINK_HOVER(id, fn) \
    wx__DECLARE_EVT1(wxEVT_COMMAND_HYPERLINK_HOVER, id, CHyperlinkHoverHandler(fn))


END_NCBI_SCOPE

#endif // GUI_WIDGETS_GRID_WIDGET__GRID__HPP
