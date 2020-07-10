#ifndef GUI_WX_DEMO___MIN_PANEL_CONTAINER__HPP
#define GUI_WX_DEMO___MIN_PANEL_CONTAINER__HPP

/*  $Id: min_panel_container.hpp 22263 2010-10-06 19:00:23Z kuznets $
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
 * Authors:  Andrey Yazhuk
 *
 * File Description:
 *      
 */
#include <gui/gui_export.h>

#include <corelib/ncbistd.hpp>
#include <corelib/ncbiobj.hpp>

#include <wx/panel.h>

BEGIN_NCBI_SCOPE

class CDockPanel;
class CDockManager;
class IWMClient;

///////////////////////////////////////////////////////////////////////////////
/// CMinPanelContainer

class NCBI_GUIWIDGETS_WX_EXPORT  CMinPanelContainer :
    public wxPanel
{
    DECLARE_EVENT_TABLE();
public:
    typedef wxPanel TParent;

public:
    CMinPanelContainer(wxWindow* parent, CDockManager& manager);
    virtual ~CMinPanelContainer();

    void    AddClient(CDockPanel& panel);
    void    RemoveClient(CDockPanel& panel);
    bool    HasClient(CDockPanel& panel) const;

    virtual bool    Layout();
    virtual wxSize  DoGetBestSize() const;

    void    OnPaint(wxPaintEvent& event);
    void    OnSize(wxSizeEvent& event);
    void    OnContextMenu(wxContextMenuEvent& event);
    void    OnLeftClick(wxMouseEvent& event);

    void    OnDockPanelCommand(wxCommandEvent& event);
    void    OnRestoreWindow(wxCommandEvent& event);
    void    OnShowWindowsDlg(wxCommandEvent& event);
    void    OnUpdateWindowCommand(wxUpdateUIEvent& event);

protected:
    struct  SItem
    {
        CDockPanel* m_DockPanel;
        wxRect  m_Rect;
    };

    typedef vector<SItem*> TItems;

    const SItem*  x_FindItemByPanel(CDockPanel* panel) const;
    SItem*  x_FindItemByPos(const wxPoint& pt);

    wxSize  x_MeasureItem(wxDC& dc, const SItem& descr);
    void    x_DrawItem(wxDC& dc, SItem& descr);

protected:
    CDockManager&    m_DockManager;

    TItems  m_Items;
    wxSize  m_PrefSize;
    wxFont  m_Font;

    CDockPanel* m_MenuPanel; /// panel for which context menu is shown

    /// maps automatically generated cmd ids to Dock Panels
    map<int, CDockPanel*>   m_CmdToPanel;
};


END_NCBI_SCOPE


#endif  // GUI_WX_DEMO___MIN_PANEL_CONTAINER__HPP
