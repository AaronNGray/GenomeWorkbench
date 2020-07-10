#ifndef GUI_WIDGETS_WX___STICKY_TOOLTIP_WND_HPP
#define GUI_WIDGETS_WX___STICKY_TOOLTIP_WND_HPP
 /* ===========================================================================
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
 * Authors:  Bob Falk
 *
 * File Description:
 * CTooltipFrame is the sticky tool tip window derived from wxFrame.  Being 
 * derived from wxFrame, it behaves as an top-level window rather than
 * as a control on a panel.  To get it to move around with its host window
 * the class CStickyTooltipHandler keeps a list of all all attached tips
 * for a given window and moves, resizes, creates, and destroys them
 * as needed to maintain their attachment to their host window.
 */

#include <corelib/ncbistl.hpp>

#include <gui/gui_export.h>
#include <gui/objutils/tool_tip_info.hpp>
#include <gui/utils/command.hpp>

// This header must (at least indirectly) precede any wxWidgets headers.
#include <gui/widgets/wx/fixed_base.hpp>

#include <wx/frame.h>
#include <wx/textctrl.h>
#include <wx/richtext/richtextctrl.h>
#include <wx/bmpbuttn.h>
#include <wx/panel.h>
#include <wx/webview.h>
#include <wx/html/htmlwin.h>

class wxFileArtProvider;

#if defined(__WXOSX_CARBON__) || defined(__WXMAC_CARBON__)
#include <Carbon/Carbon.h>
#endif

//#define RICH_TEXT_TOOLTIP
#define HTML_TOOLTIP

BEGIN_NCBI_SCOPE

class CRichTextHandle;
class CTooltipFrame;
class CHtmlWindowtHandle;

/** Helper class that allows mouse click and motion events to
    grab the control's parent object and move it around.
*/
class CWidgetHandle
{
public:
    CWidgetHandle(CTooltipFrame* dragee)
        : m_Dragee(dragee)
        , m_Dragging(false)
        , m_Widget(NULL) {}

    void SetWindow(wxWindow* w) { m_Widget = w; }

    /// Events forwarded from tooltip wnd (not tied directly to event handler)
    void OnMouseDown(wxMouseEvent& evt, bool needs_capture = true);
    void OnMouseUp(wxMouseEvent& evt, bool needs_capture = true);
    void OnMove(wxMouseEvent& evt);
    void OnKeyEvent(wxKeyEvent& event);
    void OnContextMenu(wxContextMenuEvent& evt);

    bool IsDragging() const { return m_Dragging; }

protected:
    /// Pointer to tip window owner
    CTooltipFrame* m_Dragee;
    /// True if dragging of tip currently in progress
    bool m_Dragging;
    /// Holds offset of mouse from control base position
    wxPoint m_Offset;
    /// Pointer to control that forwards events to the handle
    wxWindow* m_Widget;
};

/// Interface, used to forward handling of URL click events
class ILinkEventHandler
{
public:
    /// Invoked when an URL is clicked
    /// @param[in] href
    ///     String, containing the HREF value of the <A> tag. 
    /// @return
    ///     True if the event was handled, false otherwise.
    virtual bool OnLinkClicked(const wxString &href) = 0;
};

/** The tooltip window that displays tip information.  All tips for a given
    display window are managed by an instance of the CStickyTooltipHandler 
    class.
*/
class NCBI_GUIWIDGETS_WX_EXPORT CTooltipFrame: public wxFrame
{
public:
    CTooltipFrame( wxWindow *parent
                  ,const wxString& title
                  ,const wxPoint& pos
			      ,const wxSize& size
                  ,const CTooltipInfo& info
#if defined(__WXOSX_CARBON__) || defined(__WXMAC_CARBON__)
                 ,WindowGroupRef win_grp_ref
#endif
                  );

    virtual ~CTooltipFrame();

    void AddChildWindow();
    void RemoveChildWindow();

    /// Override Show from wxWindow to add/remove frame from its parent
    /// window (on mac cocoa)
    virtual bool Show( bool show = true );

    /// Moves the tip to a new position
    void UpdateScreenPosition(const wxPoint& spos) { SetPosition(spos+m_Pos); }
    /// Set m_Pos to current position of tip within parent window
    void SetWindowPosition();
    /// Clip tip to parent window
    void FitToWindow(bool about_to_show=false);
    /// Set tip window to its full (unclipped) size
    void SetToDefaultSize();

    /// Set flag to indicate if tip is pinned or not
    void SetPinned(bool b);
    bool IsPinned() const { return m_Pinned; }

    /// When user mouses over element/glyph assigned to tooltip
    void SetElementActive(bool b);
    bool GetElementActive() const { return m_ElementActive; }

    /// Set the full (unclipped) size of the tip
    void   SetTargetSize(wxSize  s) { m_TargetSize = s; }
    wxSize GetTargetSize() const { return m_TargetSize; }

    /// Return tips position within its parent window
    wxPoint GetPos() const { return m_Pos; }

    wxPoint GetInitialMousePos() const { return m_InitialMousePos; }
    void    SetInitialMousePos(wxPoint p) { m_InitialMousePos = p; }

    /// Handle events from url clicks within rich-text controls in tip
    virtual void OnURL(wxTextUrlEvent& evt);
    /// Events from url clicks if we are using the wxHtmlWindow
    virtual void OnLinkClicked(wxHtmlLinkEvent& evt);
    /// When the pin/unpin button is pressed
    virtual void OnPinButton(wxCommandEvent& evt);
    /// Display context menu for tooltip (allows text to be copied)
    virtual void OnContextMenu(wxContextMenuEvent &evt);
    /// Copy tip text to the clipboard
    virtual void OnCopyText(wxCommandEvent & evt);
    /// Make text on tip selectable 
    virtual void OnSelectText(wxCommandEvent & evt);
    /// Update checkbox for text selection
    virtual void OnUpdateSelectText(wxUpdateUIEvent& evt);
    /// Copy selected tip text to the clipboard
    virtual void OnCopySelectedText(wxCommandEvent & evt);
    /// Enable/disable menu item based on whether select text is enabled
    virtual void OnUpdateCopySelectText(wxUpdateUIEvent& evt);
    /// Removes the tip if mouse leaves window (if tip not pinned)
    void OnLeaveWindow(wxMouseEvent& event);

    void OnKeyEvent(wxKeyEvent& event);

    /// Set tip info to display
    void SetTipInfo(const CTooltipInfo& tip) { m_TipInfo = tip; }
    CTooltipInfo GetTipInfo() const { return m_TipInfo; }

    /// Return true if a browser has been launchced fromt this tip
    bool LaunchedBrowser() const;
    /// When the tip is no longer active, this is called to remove
    /// the 'launched' property
    void SetLaunchedFalse() { m_LaunchedBrowser = false; }

    /// Return true if this tip windows on internal popup menu is displayed
    bool TipPopupMenuDisplayed() const { return m_PopupDisplayed; }

    /// Pointer to the links event hanlder
    void SetLinkEventHandler(ILinkEventHandler* linkEventHandler) { m_LinkEventHandler = linkEventHandler; }

protected:
    /// Appends a single line of text to the tip while tracking total text size
    void x_AddLineToWidget(wxRichTextCtrl* ctrl,
                           const std::string& l, 
                           wxSize& text_extent);

    void x_CreateRichTextControl(wxPanel* p, int title_width);
    void x_CreateHtmlTextControl(wxPanel* p, int title_width, int max_width);
    void x_CreateHtmlTextControlTest(wxPanel* p, int title_width);


    /// Add a button to the top of the tooltip
    wxBitmapButton* x_AddButton(wxPanel* p,
                                EStickyToolTipCommands cmd,
                                const char* img_name,
                                const char* img_pressed_name,
                                wxSize button_size,
                                wxFileArtProvider* provider);

    /// Break tip text into individual lines
    std::vector<std::string> x_BreakIntoLines(const std::string& text, 
                                              int max_line_len = 100);

    /// Position within parent window
    wxPoint m_Pos;
    /// Mouse position when (unpinned) tip created. Used in deciding when to
    /// dismiss tip.
    wxPoint m_InitialMousePos;

    /// True if this tip is currently pinned
    bool m_Pinned;
    /// Default (unclipped) size of the tip window
    wxSize m_TargetSize;
    /// Tip info to display
    CTooltipInfo  m_TipInfo;

    /// Pin button - changes image when pinned so we need to keep a ptr to it
    wxBitmapButton* m_PinButton;

    /// Set true bywindow when user mouses over the tips assigned glyph/element
    bool m_ElementActive;

    /// Color and panel used to highlght the tip when needed
    wxColour m_BackgroundColour;
    wxPanel* m_HighlightPanel; 

    /// Handle object to which all tooltips forward mouse down/move events
    CWidgetHandle* m_Handle;

    /// The main text control in the tip window (title is the other one)
#ifdef RICH_TEXT_TOOLTIP
    CRichTextHandle* m_TextWidget;
#else
    CHtmlWindowtHandle* m_TextWidget;
#endif
    //wxWebView* m_TextWidget;



    /// When windows launches a browser from a tooltip, moving focus away
    /// from tip to gbench app can cause browser to move (wrongly) to
    /// background.  This allows us to launch the browser from 
    /// the sticky tooltips handler to better preserve z order.
    bool m_LaunchedBrowser;

    /// True while tooltips own internal popup menu is displayed
    bool m_PopupDisplayed;

    /// Pointer to the links event hanlder
    ILinkEventHandler*  m_LinkEventHandler;

    DECLARE_EVENT_TABLE()
};

END_NCBI_SCOPE

#endif // GUI_WIDGETS_WX___STICKY_TOOLTIP_WND_HPP
