#ifndef GUI_WIDGETS_WORKSPACE___WM_CAPTION_PANEL_HPP
#define GUI_WIDGETS_WORKSPACE___WM_CAPTION_PANEL_HPP

/*  $Id: dock_panel.hpp 43813 2019-09-05 16:03:49Z katargir $
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
 *      Dock Panel - a window that hosts Window Manager Client, displays a
 *      titlebar with menu and buttons and interacts with Window Manager.
 */

#include <corelib/ncbistd.hpp>

#include <gui/widgets/wx/dock_manager.hpp>
#include <gui/widgets/wx/iwindow_manager.hpp>

#include <wx/panel.h>
#include <wx/sizer.h>

class wxStaticText;
class wxBitmapButton;

BEGIN_NCBI_SCOPE


class   IWMClient;

/// Dock Panel commands
enum    EDockPanelCommands   {
    eCmdCloseDockPanel = 5600,  // close client window
    eCmdMoveToMainTab,  // move to the Main Tabbed Pane
    eCmdWindowFloat,    // move to a separate Floating Frame
    eCmdWindowMinimize, // Minimize
    eCmdWindowRestore,  // Restore minimized panel
    eCmdShowDockPanelMenu   // display a popup menu with these commands
};


///////////////////////////////////////////////////////////////////////////////
/// CDockPanel - a container with a title bar (caption) hosting a single
/// client window (IWMClient).
/// Color of the caption changes when a client or on of it's
/// child windows receives focus.

class CDockPanel :
        public wxPanel,
        public IDockDropTarget,
        public IDockableWindow
{
    DECLARE_EVENT_TABLE();
public:
    typedef wxPanel TParent;

    CDockPanel(CDockContainer* container,
               CDockManager* manager,
               IWMClient* client,
               const wxString& name);

    virtual ~CDockPanel();

    virtual void ShowBorder(bool show);
    virtual bool Layout();

    virtual wxSize ClientToWindowSize(const wxSize& size) const;

    /// disconnect the client and re-parent it to the given window
    void    RemoveClient(wxWindow* new_parent);

    IWMClient* GetClient() { return m_Client; }

    /// @name IDockDropTarget implementation
    /// @{
    virtual EDockEffect DropTest(const wxPoint& screen_pt, wxWindow*& target);
    /// @}

    /// @name IDockableWindow implementation
    /// @(
    virtual CDockContainer* GetDockContainer();
    virtual void SetDockContainer(CDockContainer* dock_cont);
    /// @}

    // update title bar buttons
    virtual void UpdateButtons();

    void UpdateCaption();

    // checks if focus changed and redraws itself if necessary
    virtual void UpdateFocusState();

    virtual bool HasFocus() const; // return true if one of the children focused

    virtual void MakeActive( bool flag );

    virtual bool ProcessEvent(wxEvent& event);

    virtual void FinishDrag();

    /// @name Event handlers
    /// @{
    void    OnMenuButtonClick(wxCommandEvent& event);
    void    OnMinimizeButtonClick(wxCommandEvent& event);
    void    OnFloatButtonClick(wxCommandEvent& event);
    void    OnContextMenu(wxContextMenuEvent& event);
    void    OnChildFocus(wxChildFocusEvent& evt);

    void    OnPaint(wxPaintEvent& event);

    void    OnLeftDown(wxMouseEvent& evt);
    void    OnLeftUp(wxMouseEvent& evt);
    void    OnMotion(wxMouseEvent& evt);

    void    OnMouseCaptureLost(wxMouseCaptureLostEvent& event);

    void    OnCloseButtonClick(wxCommandEvent& event);
    void    OnSetFocus(wxFocusEvent& event);
    /// @}

protected:

    virtual void    x_CreateControls();

    virtual wxWindow* x_CreateButton(TCmdID id, const string& icon, const string& hover_icon);

    virtual void    x_SetClient(IWMClient& client);
        
    virtual void    x_DrawClientElements(wxDC& dc, const wxRect& rc);
    virtual void    x_ShowContextMenu();

protected:
    CDockManager*   m_DockManager;
    CDockContainer* m_DockContainer;

    IWMClient*  m_Client; /// the Client hosted in this Panel

    int     m_CaptionHeight;
    bool    mf_IsActive;
    bool    m_ShowingPopup;

    wxBoxSizer*     m_CaptionSizer;
    wxSizerItem*    m_CaptionItem;

    // DnD State
    wxPoint m_ClickPoint;
    bool    m_Canceled;
};


END_NCBI_SCOPE


#endif  // GUI_WIDGETS_WORKSPACE___WM_CAPTION_PANEL_HPP
