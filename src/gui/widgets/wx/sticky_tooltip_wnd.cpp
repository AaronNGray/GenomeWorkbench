/*  $Id: sticky_tooltip_wnd.cpp 43522 2019-07-23 19:34:22Z katargir $
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
 * Authors:  Bob Falk
 *
 * File Description:
 *
 */

#include <ncbi_pch.hpp>

#include <corelib/ncbidbg.hpp>

#include <gui/widgets/wx/sticky_tooltip_wnd.hpp>
#include <gui/widgets/wx/wx_utils.hpp>
#include <gui/widgets/wx/fileartprov.hpp>
#include <gui/widgets/wx/ui_command.hpp>

#include <wx/window.h>
#include <wx/menu.h>
#include <wx/sizer.h>
#include <wx/button.h>
#include <wx/bmpbuttn.h>
#include <wx/stattext.h>
#include <wx/richtext/richtextctrl.h>
#include <wx/msgdlg.h>
#include <wx/dcbuffer.h>
#include <wx/settings.h>
#include <wx/clipbrd.h>
#include <wx/webview.h>
#include <wx/display.h>

BEGIN_NCBI_SCOPE

static
WX_DEFINE_MENU(sPopupMenu)
    WX_MENU_ITEM(eCmdSelectTipText)
    WX_MENU_ITEM(eCmdCopySelectedTipText)
    WX_MENU_SEPARATOR()
    WX_MENU_ITEM(eCmdCopyTipText)    
WX_END_MENU()

void CWidgetHandle::OnMouseDown(wxMouseEvent& evt, bool needs_capture)
{
    if (!m_Widget->HasCapture() && needs_capture) {
        m_Widget->CaptureMouse();
    }          

    if (m_Widget->GetCaret() != NULL)
        m_Widget->GetCaret()->Hide();
    m_Offset = wxGetMousePosition() - m_Dragee->GetPosition();
    m_Dragging = true;
}
void CWidgetHandle::OnMouseUp(wxMouseEvent& evt, bool needs_capture)
{
    if (m_Widget->HasCapture() && needs_capture) {
        m_Widget->ReleaseMouse();          
    }
    m_Dragging=false;
}
void CWidgetHandle::OnMove(wxMouseEvent& evt)
{
    // Check in case we somehow missed a mouse-up (which should always end
    // dragging)
    if (!wxGetMouseState().LeftIsDown() && m_Dragging) {
        m_Dragging = false;

        if ( m_Widget->HasCapture()) {
             m_Widget->ReleaseMouse();        
        }
    }

    if(m_Dragging)
    {
        wxRect r = m_Dragee->GetParent()->GetScreenRect();

        // Reposition the frame fully inside the windows upper and left sides,
        // and within 20 or more pixels of the lower and  right sides
        wxPoint new_pos = wxGetMousePosition() - m_Offset;        
        new_pos.x = std::min((r.x + r.width) - 20, std::max(r.x, new_pos.x));
        new_pos.y = std::min((r.y + r.height) - 20, std::max(r.y, new_pos.y));

        m_Dragee->Move( new_pos );
        m_Dragee->SetWindowPosition();

        // Send command to parent sticky tool-tip window so
        // that it can keep the tool tip position data up-to-date.
        wxCommandEvent cmdevt(wxEVT_COMMAND_BUTTON_CLICKED, eCmdMoveTip);
        cmdevt.SetEventObject(m_Dragee);
        m_Dragee->GetParent()->GetEventHandler()->ProcessEvent(cmdevt);
    }
}

void CWidgetHandle::OnKeyEvent(wxKeyEvent& evt)
{
    m_Dragee->OnKeyEvent(evt);
}

void CWidgetHandle::OnContextMenu(wxContextMenuEvent& evt)
{
    m_Dragee->OnContextMenu(evt);
}



 /****************************************************************************
  ****************************************************************************/

/**
  * Static text helper class that allows mouse click and motion events to 
  * grab the control's parent object and move it around.
  */
class NCBI_GUIWIDGETS_WX_EXPORT CStaticTextHandle  : public  wxStaticText
{
public:
    CStaticTextHandle(wxPanel* parent, CWidgetHandle* handle) 
    :  wxStaticText(parent, wxID_ANY, wxT("CStaticTextHandle"),  
                    wxDefaultPosition, wxDefaultSize, wxNO_BORDER)
    ,  m_WidgetHandle(handle) 
    {
        // Linux/Ubuntu seems to default to black on black for tool tip colors 
        // so lets just set hard-coded values.
        SetBackgroundColour(wxColor(255,255,225));
        SetForegroundColour(wxColor(0, 0, 0));

        //SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_INFOBK));
        //SetForegroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_INFOTEXT));
    }   

    void SetLabel(const wxString &label)
    {
        wxStaticText::SetLabel(label);
#if defined(__WXOSX_CARBON__) || defined(__WXMAC_CARBON__) || defined(__WXOSX_COCOA__)
        // on Mac, if a string contains "\n" then the size is not computed correctly in SetLabel(). This fixes it.
        wxClientDC aDC(this);
        wxSize client_size = aDC.GetMultiLineTextExtent(label);
        SetClientSize(client_size);
#endif
    }

    void OnMouseDown(wxMouseEvent& evt) { m_WidgetHandle->OnMouseDown(evt); }
    void OnMouseUp(wxMouseEvent& evt) { m_WidgetHandle->OnMouseUp(evt); }

    void OnMove(wxMouseEvent& evt) 
    {  
        m_WidgetHandle->OnMove(evt); 
        if(!m_WidgetHandle->IsDragging()) {
            evt.Skip();
        }
    }

    /// Need this (call Layout()) in derived classes to work with Sizers
    void OnSize(wxSizeEvent& evt) { Layout(); }
    void OnKeyEvent( wxKeyEvent& evt ){ m_WidgetHandle->OnKeyEvent(evt); }
    void OnContextMenu(wxContextMenuEvent& evt) {m_WidgetHandle->OnContextMenu(evt);}

protected:
    CWidgetHandle*  m_WidgetHandle;

    DECLARE_EVENT_TABLE()
};

BEGIN_EVENT_TABLE(CStaticTextHandle,wxStaticText)
    EVT_LEFT_DOWN(CStaticTextHandle::OnMouseDown)
    EVT_LEFT_UP(CStaticTextHandle::OnMouseUp)
    EVT_MOTION(CStaticTextHandle::OnMove)
    EVT_SIZE(CStaticTextHandle::OnSize)
    EVT_KEY_DOWN(CStaticTextHandle::OnKeyEvent)
    EVT_KEY_UP(CStaticTextHandle::OnKeyEvent)
    EVT_CONTEXT_MENU(CStaticTextHandle::OnContextMenu)
END_EVENT_TABLE() 


 /****************************************************************************
  ****************************************************************************/

/**
  * Rich Text helper class that allows mouse click and motion events to 
  * grab the control's parent object and move it around.
  */
class NCBI_GUIWIDGETS_WX_EXPORT CRichTextHandle  : public  wxRichTextCtrl
{
public:
    CRichTextHandle(wxPanel* parent, CWidgetHandle* handle, wxSize s)
    : wxRichTextCtrl(parent, wxID_ANY, wxEmptyString, 
                     wxDefaultPosition, s, 
                     wxNO_BORDER|wxTE_MULTILINE|wxTE_READONLY)
    ,  m_WidgetHandle(handle) 
    ,  m_SelectionEnabled(false)
    {        
        SetEditable(false);
        SetTextCursor(wxCURSOR_ARROW);
        GetCaret()->Hide();
        
        // Linux/Ubuntu seems to default to black on black for tool tip colors 
        // so lets just set hard-coded values.
        SetBackgroundColour(wxColor(255,255,225));
        SetForegroundColour(wxColor(0, 0, 0));

        //SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_INFOBK));
        //SetForegroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_INFOTEXT));
    }   
    
    bool GetSelectionEnabled() const { return m_SelectionEnabled; }
    void SetSelectionEnabled(bool b) 
    {
        m_SelectionEnabled = b;

        if (m_SelectionEnabled) {
            SetTextCursor(wxCURSOR_IBEAM);
            GetCaret()->Show();          
        }
        else {
            SetTextCursor(wxCURSOR_ARROW);
            // Unselect any selected text
            SelectNone();
            GetCaret()->Hide();          
        }
    }
   
    // this makes scroll bars go away, but re-formatting still happens....
    virtual void DoSetVirtualSize( int x, int y ) 
                                   {wxRichTextCtrl::DoSetVirtualSize(1,1);}
    virtual wxSize DoGetVirtualSize() const {  return wxSize(1, 1); }
    
    void OnMouseDown(wxMouseEvent& evt) { 
        if (m_SelectionEnabled) {        
            evt.Skip();        
            return;
        }

        long pos = GetCaretPosition();

        wxPoint pt = ScreenToClient(wxGetMousePosition());
        HitTest(pt, &pos);

        // Determine if cursor is over a url and, if it is, skip the event.  If the
        // event is not skipped, it uses the widget handle to initiate the moving
        // of the widget, which will prevent opening the url.
        if (pos != -1) {
            wxRichTextAttr attr;
            if (GetStyle(pos,attr)) {
                if (attr.HasURL()) {
                    evt.Skip(); 
                    return;
                }
            }
        }

        m_WidgetHandle->OnMouseDown(evt); 
    }
    void OnMouseUp(wxMouseEvent& evt) { 
        if (m_SelectionEnabled) {     
            evt.Skip();
            return;
        }

        if (m_WidgetHandle->IsDragging()) {
            m_WidgetHandle->OnMouseUp(evt); 
        }        
        else {
            evt.Skip();
        }
    }

    void OnMove(wxMouseEvent& evt) 
    {  
        if (m_SelectionEnabled) {
            //wxRichTextCtrl::OnMoveMouse(evt);
            evt.Skip();
            return;
        }

        m_WidgetHandle->OnMove(evt); 
        if(!m_WidgetHandle->IsDragging()) {
            evt.Skip();
        }
    }

    void OnKeyEvent( wxKeyEvent& evt ){ m_WidgetHandle->OnKeyEvent(evt); }
    void OnContextMenu(wxContextMenuEvent& evt) {m_WidgetHandle->OnContextMenu(evt);}
    
    /// Need this (call Layout()) in derived classes to work with Sizers
    void OnSize(wxSizeEvent& evt) { LayoutContent(); }    

protected:
    CWidgetHandle*  m_WidgetHandle;
    bool m_SelectionEnabled;

    DECLARE_EVENT_TABLE()
};

BEGIN_EVENT_TABLE(CRichTextHandle,wxRichTextCtrl)
    EVT_LEFT_DOWN(CRichTextHandle::OnMouseDown)
    EVT_LEFT_UP(CRichTextHandle::OnMouseUp)
    EVT_MOTION(CRichTextHandle::OnMove)
    EVT_SIZE(CRichTextHandle::OnSize)
    EVT_KEY_DOWN(CRichTextHandle::OnKeyEvent)
    EVT_KEY_UP(CRichTextHandle::OnKeyEvent)
    EVT_CONTEXT_MENU(CRichTextHandle::OnContextMenu)
END_EVENT_TABLE() 



 /****************************************************************************
  ****************************************************************************/

/**
  * Rich Text helper class that allows mouse click and motion events to 
  * grab the control's parent object and move it around.
  */
class NCBI_GUIWIDGETS_WX_EXPORT CHtmlWindowtHandle  : public  wxHtmlWindow
{
public:
    CHtmlWindowtHandle(wxPanel* parent, CWidgetHandle* handle, wxSize s)
    : wxHtmlWindow(parent, wxID_ANY, wxDefaultPosition, s, 
                   wxHW_SCROLLBAR_NEVER/*|wxHW_NO_SELECTION*/)
    ,  m_WidgetHandle(handle) 
    ,  m_SelectionEnabled(false)
    {               
    }   
    
    bool GetSelectionEnabled() const { return m_SelectionEnabled; }
    void SetSelectionEnabled(bool b) 
    {
        m_SelectionEnabled = b;
    }
   
    // this makes scroll bars go away, but re-formatting still happens....
    virtual void DoSetVirtualSize( int x, int y ) 
                                   {wxHtmlWindow::DoSetVirtualSize(1,1);}
    virtual wxSize DoGetVirtualSize() const {  return wxSize(1, 1); }

    void OnMouseDown(wxMouseEvent& evt) { 
        if (m_SelectionEnabled) {        
            evt.Skip();        
            return;
        }

        // Do not mouse capture for html control since it also captures (since
        // we let the event propogate further) in order to do selection
        m_WidgetHandle->OnMouseDown(evt, false); 
        evt.Skip();        
    }
    void OnMouseUp(wxMouseEvent& evt) { 
        if (m_SelectionEnabled) {     
            evt.Skip();
            return;
        }

        if (m_WidgetHandle->IsDragging()) {
            // Do not capture for html control since it also captures (since we
            // let the event propogate further) in order to do selection
            m_WidgetHandle->OnMouseUp(evt, false); 
        }        

        evt.Skip();
    }

    void OnMove(wxMouseEvent& evt) 
    {  
        if (m_SelectionEnabled) {
            evt.Skip();
            return;
        }

        m_WidgetHandle->OnMove(evt); 
    }

    void OnKeyEvent( wxKeyEvent& evt ){ m_WidgetHandle->OnKeyEvent(evt); }
    void OnContextMenu(wxContextMenuEvent& evt) {m_WidgetHandle->OnContextMenu(evt);}

    void OnRightUp(wxMouseEvent& evt) 
    {
        wxContextMenuEvent cevt;
        evt.SetPosition(evt.GetPosition());

        m_WidgetHandle->OnContextMenu(cevt);
    }
    
    /// Need this (call Layout()) in derived classes to work with Sizers
    void OnSize(wxSizeEvent& evt) { Layout(); }    

protected:
    CWidgetHandle*  m_WidgetHandle;
    bool m_SelectionEnabled;

    DECLARE_EVENT_TABLE()
};

BEGIN_EVENT_TABLE(CHtmlWindowtHandle,wxHtmlWindow)
    EVT_LEFT_DOWN(CHtmlWindowtHandle::OnMouseDown)
    EVT_LEFT_UP(CHtmlWindowtHandle::OnMouseUp)
    EVT_MOTION(CHtmlWindowtHandle::OnMove)
    EVT_SIZE(CHtmlWindowtHandle::OnSize)
    EVT_KEY_DOWN(CHtmlWindowtHandle::OnKeyEvent)
    EVT_KEY_UP(CHtmlWindowtHandle::OnKeyEvent)
    EVT_CONTEXT_MENU(CHtmlWindowtHandle::OnContextMenu)

    // WxWindows isn't sending mouse context menu events to wxHtmlControl objects
    // on windows so use right up instead
#ifdef __WXMSW__
    EVT_RIGHT_UP(CHtmlWindowtHandle::OnRightUp)
#else
    EVT_CONTEXT_MENU(CHtmlWindowtHandle::OnContextMenu)
#endif
END_EVENT_TABLE() 

 
/****************************************************************************
  ****************************************************************************/
/**
  * Panel helper class that allows mouse click and motion events to 
  * grab the control's parent class and move it around.
  */
class NCBI_GUIWIDGETS_WX_EXPORT CPanelHandle  : public  wxPanel
{
public:
    CPanelHandle(wxWindow* parent, 
                 CWidgetHandle* handle, 
                 const wxPoint& pos, 
                 const wxSize& size,
                 int style = 0) 
    : wxPanel(parent, wxID_ANY, pos, size, style)
    , m_WidgetHandle(handle)  {}   

    void OnMouseDown(wxMouseEvent& evt) { m_WidgetHandle->OnMouseDown(evt); }
    void OnMouseUp(wxMouseEvent& evt) { m_WidgetHandle->OnMouseUp(evt); }
    void OnMove(wxMouseEvent& evt) 
    {  
        m_WidgetHandle->OnMove(evt); 
        if(!m_WidgetHandle->IsDragging()) {
            evt.Skip();
        }
    }
    /// Need this (call Layout()) in derived classes to work with Sizers
    void OnSize(wxSizeEvent& evt) { Layout(); }
    void OnKeyEvent( wxKeyEvent& evt ){ m_WidgetHandle->OnKeyEvent(evt); }
    void OnContextMenu(wxContextMenuEvent& evt) {m_WidgetHandle->OnContextMenu(evt);}

protected:
    CWidgetHandle*  m_WidgetHandle;

    DECLARE_EVENT_TABLE()
}; 

BEGIN_EVENT_TABLE(CPanelHandle,wxPanel)
   EVT_LEFT_DOWN(CPanelHandle::OnMouseDown)
   EVT_LEFT_UP(CPanelHandle::OnMouseUp)
   EVT_MOTION(CPanelHandle::OnMove)
   EVT_SIZE(CPanelHandle::OnSize)
   EVT_KEY_DOWN(CPanelHandle::OnKeyEvent)
   EVT_KEY_UP(CPanelHandle::OnKeyEvent)
   EVT_CONTEXT_MENU(CPanelHandle::OnContextMenu)
END_EVENT_TABLE() 


 /****************************************************************************
  ****************************************************************************/

BEGIN_EVENT_TABLE(CTooltipFrame, wxFrame)      
    EVT_TEXT_URL(wxID_ANY, CTooltipFrame::OnURL)
    EVT_HTML_LINK_CLICKED(wxID_ANY, CTooltipFrame::OnLinkClicked)
    //EVT_BUTTON(eCmdPinTip, CTooltipFrame::OnPinButton)
    EVT_CONTEXT_MENU(CTooltipFrame::OnContextMenu)
    EVT_LEAVE_WINDOW(CTooltipFrame::OnLeaveWindow)

    EVT_KEY_DOWN(CTooltipFrame::OnKeyEvent)
    EVT_KEY_UP(CTooltipFrame::OnKeyEvent)

    EVT_MENU(eCmdCopyTipText, CTooltipFrame::OnCopyText)
    EVT_MENU(eCmdSelectTipText, CTooltipFrame::OnSelectText)
    EVT_UPDATE_UI(eCmdSelectTipText, CTooltipFrame::OnUpdateSelectText)
    EVT_MENU(eCmdCopySelectedTipText, CTooltipFrame::OnCopySelectedText)    
    EVT_UPDATE_UI(eCmdCopySelectedTipText, CTooltipFrame::OnUpdateCopySelectText)
END_EVENT_TABLE()

CTooltipFrame::CTooltipFrame( wxWindow *parent
                             ,const wxString& title
                             ,const wxPoint& pos
                             ,const wxSize& size
                             ,const CTooltipInfo& info
#if defined(__WXOSX_CARBON__) || defined(__WXMAC_CARBON__)
                             ,WindowGroupRef win_grp_ref
#endif
                             )
: wxFrame(parent, wxID_ANY, title, pos, wxDefaultSize,         
#ifndef __WXOSX_COCOA__
         wxFRAME_TOOL_WINDOW|wxFRAME_NO_TASKBAR|wxBORDER_SIMPLE|wxFRAME_FLOAT_ON_PARENT,
#else
          wxFRAME_NO_TASKBAR,
#endif
          wxT("Sticky Tooltip"))
, m_Pinned(false)
, m_TipInfo(info)
, m_PinButton(NULL)
, m_ElementActive(false)
, m_HighlightPanel(NULL)
, m_Handle(NULL)
, m_TextWidget(NULL)
, m_LaunchedBrowser(false)
, m_PopupDisplayed(false)
, m_LinkEventHandler(0)
{
#if defined(__WXOSX_CARBON__) || defined(__WXMAC_CARBON__)
#if( wxMAJOR_VERSION == 2 && wxMINOR_VERSION < 9 )
    ReleaseWindowGroup( GetWindowGroup((WindowRef)m_macWindow) );
#else
    ReleaseWindowGroup( GetWindowGroup(GetWXWindow()) );
#endif

    _ASSERT(win_grp_ref!=NULL);	
    SetWindowGroup( (WindowRef)  MacGetTopLevelWindowRef(), win_grp_ref);
#endif
    
    wxFileArtProvider* provider = GetDefaultFileArtProvider();

    static bool first_tip = false;
    if (first_tip == false) {
        first_tip = true;

        // Register images only once

        provider->RegisterFileAlias(wxT("pin_horizontal"), wxT("pin_horizontal.png"));
        provider->RegisterFileAlias(wxT("pin_horizontal_pressed"), wxT("pin_horizontal_pressed.png"));
        provider->RegisterFileAlias(wxT("pin_vertical"), wxT("pin_vertical.png"));
        provider->RegisterFileAlias(wxT("pin_vertical_pressed"), wxT("pin_vertical_pressed.png"));
        provider->RegisterFileAlias(wxT("mag_glass"), wxT("mag_glass.png"));     
        provider->RegisterFileAlias(wxT("mag_glass_pressed"), wxT("mag_glass_pressed.png"));    
        provider->RegisterFileAlias(wxT("search_img"), wxT("search_img.png"));     
        provider->RegisterFileAlias(wxT("search_img_pressed"), wxT("search_img_pressed.png"));    
        provider->RegisterFileAlias(wxT("info_i"), wxT("info_i.png"));     
        provider->RegisterFileAlias(wxT("info_i_pressed"), wxT("info_i_pressed.png"));
        //provider->RegisterFileAlias(wxT("copy_tip"), wxT("copy.png"));
        //provider->RegisterFileAlias(wxT("copy_tip_pressed"), wxT("copy.png"));
    }

    m_Handle = new CWidgetHandle(this); 

    // Top level panel - holds sizer and also is the panel that is highlighted when mouse
    // hovers over glyph assigned to the tooltip.  Cocoa windows without toolbars
    // are borderless, so we add the border to the outer-most widget
    m_HighlightPanel = new CPanelHandle(this, m_Handle, wxPoint(0,0), wxDefaultSize, 
#ifdef __WXOSX_COCOA__
        wxBORDER_SIMPLE);
#else
        0);
#endif
 
    m_Handle->SetWindow(m_HighlightPanel);

    // Offset panel inside p0 so that there is an outer border that can be highlighted
    CPanelHandle* p = new CPanelHandle(m_HighlightPanel, m_Handle, wxPoint(2,2), wxDefaultSize);   
     
    m_BackgroundColour.Set(255,255,225);
    //m_BackgroundColour = wxSystemSettings::GetColour(wxSYS_COLOUR_INFOBK);
    p->SetBackgroundColour(m_BackgroundColour);
    m_HighlightPanel->SetBackgroundColour(m_BackgroundColour);

    wxBoxSizer* vbox = new wxBoxSizer(wxVERTICAL);
    wxBoxSizer* hbox = new wxBoxSizer(wxHORIZONTAL);

    wxSize button_size;

// The actual bitmap size was 15x15, but different platforms look good with
// different size choices for the actual button.
#if defined(NCBI_OS_LINUX)
    button_size = wxDefaultSize;
#elif defined(__WXOSX_COCOA__)
    button_size = wxDefaultSize;
#else
    button_size = wxSize(17,17);
#endif
      
    wxBitmapButton* b;

    if (info.GetToolTipOptions() & CTooltipInfo::ePin) {
        b = x_AddButton(p, 
                        eCmdPinTip,  
                        "pin_horizontal", 
                        "pin_horizontal_pressed", 
                        button_size,
                        provider);
        hbox->Add(b, 0);
        m_PinButton = b;
    }

    if (info.GetToolTipOptions() & CTooltipInfo::eSearch) {
        b = x_AddButton(p, 
                        eCmdSearchTip,  
                        "search_img", 
                        "search_img_pressed", 
                        button_size,
                        provider);
        hbox->Add(b, 0);
    }
    
    if (info.GetToolTipOptions() & CTooltipInfo::eMagnify) {
        b = x_AddButton(p, 
                        eCmdZoomTip,  
                        "mag_glass", 
                        "mag_glass_pressed", 
                        button_size,
                        provider);
        hbox->Add(b, 0);
    }
    
    if (info.GetToolTipOptions() & CTooltipInfo::eInfo) {
        b = x_AddButton(p, 
                        eCmdInfoTip,  
                        "info_i", 
                        "info_i_pressed", 
                        button_size,
                        provider);
        hbox->Add(b, 0);
    }

    /// Add a space and then the title field for display next to the buttons
    int title_width = 0;

    if (hbox->GetChildren().size() > 0 || m_TipInfo.GetTitleText() != "" ) {
        hbox->AddSpacer(10);    
        CStaticTextHandle* tip_title = new CStaticTextHandle(p, m_Handle);
        tip_title->SetLabel(ToWxString(m_TipInfo.GetTitleText()));

        // Linux/Ubuntu seems to default to black on black for tool tip colors 
        // so lets just set hard-coded values.
        tip_title->SetBackgroundColour(wxColor(255,255,225));
        tip_title->SetForegroundColour(wxColor(0, 0, 0));

        //tip_title->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_INFOBK) );
        //tip_title->SetForegroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_INFOTEXT));
        hbox->Add(tip_title, 0);
    
        vbox->Add(hbox, 1, wxTOP|wxLEFT|wxRIGHT, 0);
        hbox->Layout();
        title_width = hbox->GetMinSize().GetWidth();
    }
    else {
        delete hbox;
    }

    // Creat control (that supports urls)
    {          
#ifdef RICH_TEXT_TOOLTIP
        x_CreateRichTextControl(p, title_width);
#else
         x_CreateHtmlTextControl(p, title_width, 500);
        //x_CreateHtmlTextControlTest(p, title_width);
#endif

        // Linux/Ubuntu seems to default to black on black for tool tip colors 
        // so lets just set hard-coded values.
        m_TextWidget->SetBackgroundColour(wxColor(255,255,225));
        m_TextWidget->SetForegroundColour(wxColor(0, 0, 0));

        //m_TextWidget->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_INFOBK) );
        //m_TextWidget->SetForegroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_INFOTEXT));
                                                
        vbox->Add(m_TextWidget, 0, wxGROW|wxLEFT|wxTOP, 0); 

    }
        
    p->SetSizerAndFit(vbox);
       
    wxBoxSizer* frame_sizer = new wxBoxSizer(wxHORIZONTAL);
    frame_sizer->Add(m_HighlightPanel, 1, wxALL|wxGROW, 0);
    frame_sizer->FitInside(this);        
    
    this->SetSizerAndFit(frame_sizer);

    // Allows window to be resized smaller in FitToWindow (esp. Mac)
    SetMinSize(wxSize(2, 2));
   
    m_TargetSize = GetSize() + wxSize(2, 2);
    // Move the window, if it goes outside of the screen
    //
    // At edge of screen somtimes GetFromWindow does not work
    int display_idx = wxDisplay::GetFromPoint(pos);
    if (display_idx == wxNOT_FOUND)
        display_idx = wxDisplay::GetFromWindow(parent);
    // In unlikely event still not found, pick 0
    if (display_idx == wxNOT_FOUND)
        display_idx = 0;
    
    wxDisplay display(display_idx);
    wxRect geometry = display.GetGeometry();
    int right = (pos.x > geometry.width) ? geometry.GetRight() : geometry.width - 1;
    if ((right - pos.x) < m_TargetSize.GetX()) {
        int offset = m_TargetSize.GetX() - (right - pos.x);
        wxPoint newPos(pos.x - offset, pos.y);
        SetPosition(newPos);
    }
    
    // Expand the window by 2 => this is the border area that will be used to highlight
    // the window 
    SetWindowPosition();
   



    // Allows user to access tooltip object using same method as for buttons
    // which also store 'this' in the client data, e.g.:
    //     wxEvtHandler* obj = dynamic_cast<wxEvtHandler*>(evt.GetEventObject());
    //     if (obj != NULL) {
    //           CTooltipFrame* f = static_cast<CTooltipFrame*>(obj->GetClientData());
    this->SetClientData(this);     
}

CTooltipFrame::~CTooltipFrame()
{
    delete m_Handle;
    m_Handle = NULL;
}

void CTooltipFrame::RemoveChildWindow()
{
#ifdef __WXOSX_COCOA__
   wxFrame* f = NULL;
   wxWindow* pp = GetParent();
   while (f==NULL && pp !=NULL) {
       f = dynamic_cast<wxFrame*>(pp);
       pp = pp->GetParent();
   }

   if (f != NULL) {
       //f->RemoveChildWindow(this);
       ncbi::RemoveChildWindow(f, this);
   }
#endif
}

void CTooltipFrame::AddChildWindow()
{
#ifdef __WXOSX_COCOA__
   wxFrame* f = NULL;
   wxWindow* pp = GetParent();
   while (f==NULL && pp !=NULL) {
       f = dynamic_cast<wxFrame*>(pp);
       pp = pp->GetParent();
   }

   if (f != NULL) {
       //f->AddChildWindowAbove(this);
       ncbi::AddChildWindowAbove(f, this);
   }
#endif
}

bool CTooltipFrame::Show(bool show)
{
    if (show) {
	AddChildWindow();
    }
    else {
	RemoveChildWindow();
    }
     
    return wxFrame::Show(show);
}

void CTooltipFrame::SetWindowPosition() 
{ 
    m_Pos = this->GetScreenPosition() - this->GetParent()->GetScreenPosition();
}

void CTooltipFrame::FitToWindow(bool about_to_show)
{
    if (!IsPinned())
        return;

    // In GDK (at least) this is called for tips on hidden notebook pages.
    // This keeps those hidden tips from popping up when resized.
//#ifdef NCBI_OS_LINUX  (windows too..)
    if (!GetParent()->IsShownOnScreen() && !about_to_show)
         return;
//#endif

    wxRect r = GetScreenRect();
    wxRect pr = GetParent()->GetScreenRect();

    int new_width;
    int new_height;

    int available_width = pr.x + pr.width - r.x;
    if (available_width < m_TargetSize.x)
        new_width = available_width;
    else
        new_width = m_TargetSize.x;

    int available_height = pr.y + pr.height - r.y;
    if (available_height < m_TargetSize.y)
        new_height = available_height;
    else
        new_height = m_TargetSize.y;

    if (new_width != r.width ||
        new_height != r.height) {
        if  ((new_width < 8 || new_height < 8) && IsShown()) {
            Show(false);
        }
        else if (new_width > 10 && new_height > 10 && !IsShown()) {
            Show(true);
        }

        if (new_width < 2) 
            new_width = 2;
        if (new_height < 2)
            new_height = 2;

        SetSize(wxSize(new_width, new_height));
#ifdef NCBI_OS_LINUX
        // GTK needs a kick to make it actually change the size (at least when
        // the parent is not being resized also).
        SendSizeEvent();
#endif
    }
}

void CTooltipFrame::SetToDefaultSize()
{
    SetSize(m_TargetSize);
#ifdef NCBI_OS_LINUX
        // GTK needs a kick to make it actually change the size (at least when
        // the parent is not being resized).
        SendSizeEvent();
#endif
}

void CTooltipFrame::SetPinned(bool b) 
{ 
    m_Pinned = b; 
}

void CTooltipFrame::SetElementActive(bool b)
{
    if (b != m_ElementActive) {
        m_ElementActive = b;

        if (m_ElementActive) {
            m_BackgroundColour = m_HighlightPanel->GetBackgroundColour();
            m_HighlightPanel->SetBackgroundColour(wxColor(255, 0, 0));
        }
        else {
             m_HighlightPanel->SetBackgroundColour(m_BackgroundColour);
        }
        Refresh();
    }
}

void CTooltipFrame::OnURL(wxTextUrlEvent& evt)
{
    m_LaunchedBrowser = true;
    ::wxLaunchDefaultBrowser(evt.GetString());
}

void CTooltipFrame::OnLinkClicked(wxHtmlLinkEvent& evt)
{
    wxString href(evt.GetLinkInfo().GetHref());
    if (m_LinkEventHandler && m_LinkEventHandler->OnLinkClicked(href))
        return;
    m_LaunchedBrowser = true;
    ::wxLaunchDefaultBrowser(href);
}

bool CTooltipFrame::LaunchedBrowser() const 
{ 
    return m_LaunchedBrowser;
}

void CTooltipFrame::OnContextMenu(wxContextMenuEvent& evt)
{
    m_PopupDisplayed = true; 

    CUICommandRegistry& cmd_reg = CUICommandRegistry::GetInstance();
    auto_ptr<wxMenu> menu(cmd_reg.CreateMenu(sPopupMenu));
    PopupMenu(menu.get());

    m_PopupDisplayed = false; 
}

void CTooltipFrame::OnCopyText(wxCommandEvent & evt)
{
    if ( wxTheClipboard->Open() )
    {
        wxTheClipboard->SetData( new wxTextDataObject(ToWxString(m_TipInfo.GetTipText())));
        wxTheClipboard->Close();
    }
}

void CTooltipFrame::OnSelectText(wxCommandEvent & evt)
{
    m_TextWidget->SetSelectionEnabled(!m_TextWidget->GetSelectionEnabled());
}

void CTooltipFrame::OnUpdateSelectText(wxUpdateUIEvent& evt)
{
    evt.Check(m_TextWidget->GetSelectionEnabled());
}

void CTooltipFrame::OnUpdateCopySelectText(wxUpdateUIEvent& evt)
{
#ifdef RICH_TEXT_TOOLTIP
    // richtext widget version:
    evt.Enable( m_TextWidget->GetStringSelection().length() > 0);
#else
    // wxHtmlWidget version:
    evt.Enable( m_TextWidget->SelectionToText().length() > 0);
#endif
}

void CTooltipFrame::OnCopySelectedText(wxCommandEvent & evt)
{
    if ( wxTheClipboard->Open() )
    {   
#ifdef RICH_TEXT_TOOLTIP
        // richtext widget version
        wxString str = m_TextWidget->GetStringSelection();   
#else
        // wxHtmlWidget version returns selection as plain text 
        wxString str = m_TextWidget->SelectionToText();
#endif
        wxString::iterator iter;

        // Convert rich-text line breaks to normal line breaks, otherwise some of the
        // widgets/textboxes that you might paste into will get confused by the
        // special character 'wxRichTextLineBreakChar'
        for (iter = str.begin(); iter != str.end(); ++iter) {
            if (*iter == wxRichTextLineBreakChar) {
                *iter = '\n';
            }
        }

    
        wxTheClipboard->SetData( new wxTextDataObject(str));
        wxTheClipboard->Close();
    }
}
// If mouse leaves the window, clear away any unpinned tips
void CTooltipFrame::OnLeaveWindow(wxMouseEvent& event)
{
    // This doesn't seem to work (be called) at least on windows.
    // It is only needed for dismissing the tip in the unusual case
    // where the unpinned tip is pushed up against the edge of the window
    // and the mouse leaves the window by moving across the shared edge.

    // Send a message to the tooltip manager to remove this tip since the mouse is
    // leaving and the tip is not pinned.
    if (!IsPinned()) {
        wxCommandEvent cmdevt(wxEVT_COMMAND_BUTTON_CLICKED, eCmdReleaseTip);
        cmdevt.SetEventObject(this);
        GetParent()->GetEventHandler()->ProcessEvent(cmdevt);
    }

    event.Skip();
}

void CTooltipFrame::OnKeyEvent( wxKeyEvent& event )
{
    GetParent()->GetEventHandler()->ProcessEvent(event);
}


void CTooltipFrame::OnPinButton(wxCommandEvent& evt)
{
    wxFileArtProvider* provider = GetDefaultFileArtProvider();

    // Switch the image from vertical to horizontal pin (and back)
    if (!IsPinned()) {
        m_PinButton->SetBitmapLabel(provider->GetBitmap(wxT("pin_horizontal")));
        m_PinButton->SetBitmapSelected(provider->GetBitmap(wxT("pin_horizontal_pressed")));
    }
    else {
        m_PinButton->SetBitmapLabel(provider->GetBitmap(wxT("pin_vertical")));
        m_PinButton->SetBitmapSelected(provider->GetBitmap(wxT("pin_vertical_pressed")));
    }

    // Other event handling is done in tooltip handler window
    evt.Skip();
}


void CTooltipFrame::x_CreateRichTextControl(wxPanel* p, int title_width)
{
    // Creat control (that supports urls)
    size_t i;  
    int font_size = 10; 
    wxSize text_extent(title_width, 0);
    wxFont font(font_size, wxFONTFAMILY_ROMAN, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL);      

    // Break text up into lines based on \n's in the text, so that we
    // can add them one at a time.  If last line is blank, remove it.
    std::vector<string> truncated_lines = x_BreakIntoLines(m_TipInfo.GetTipText());
    //truncated_lines.push_back("<a href=\"https://www.ncbi.nlm.nih.gov\">Visit NCBI</a>");

    // Create control to determine text size. Rich text controls,
    // unlike static text controls, do not seem to be able to automatically
    // resize (on layout) to the size that will accomodate all the text
    // added to the control without scrolling.  So we instead add all the
    // text to a 'dummy' control, calculating its maximum extent as we go.
    // We then create a new control that accomodates this size from the 
    // beginning and add the text to that control.
    wxRichTextCtrl* text0 = new wxRichTextCtrl(p, wxID_ANY, wxEmptyString, 
        wxDefaultPosition,
        wxSize(800,100));
    text0->SetFont(font); 
    text0->BeginAlignment(wxTEXT_ALIGNMENT_LEFT);       
    text0->BeginFontSize(font_size);
    text0->BeginLineSpacing(wxTEXT_ATTR_LINE_SPACING_NORMAL);
    text0->BeginParagraphSpacing(0, 0);  

    // Adds the lines one at a time, returning maximum text area each time.            
    for (i=0; i<truncated_lines.size(); ++i) {
        x_AddLineToWidget(text0, truncated_lines[i], text_extent);
    }

    // We have the size in text_extent, so we can destroy the widget
    text0->Destroy();

    // End size computation.
    // ********************************************************************
    //

    // Compute the text margin area in pixels, so that it can be added
    // to the control size
    wxWindowDC dc(this);
    wxSize pixperinch = dc.GetPPI();
    int pixel_margin_x = (int)(2.0f*((float)pixperinch.x)*(1.0f/72.0f)*
        (float)wxRICHTEXT_DEFAULT_MARGIN);
    int pixel_margin_y = (int)(2.0f*((float)pixperinch.y)*(1.0f/72.0f)*
        (float)wxRICHTEXT_DEFAULT_MARGIN);

    int cocoa_delta = 0;
#ifdef __WXOSX_COCOA__
    // Cocoa window always comes up a little too short...
    cocoa_delta += font_size/2;
#endif

    // Create the actual text control with the computed size. 2 pixels are
    // added horizontally because mac seems to need that to get to correct
    // width (a width that will not require any text wrapping)
    CRichTextHandle* text = new CRichTextHandle(p, m_Handle,
    wxSize(text_extent.GetWidth() + pixel_margin_x + 2, 
    text_extent.GetHeight() + pixel_margin_y + cocoa_delta));
    text->SetFont(font);            
    text->BeginAlignment(wxTEXT_ALIGNMENT_LEFT);       
    text->BeginFontSize(font_size);
    text->BeginLineSpacing(wxTEXT_ATTR_LINE_SPACING_NORMAL);
    text->BeginParagraphSpacing(0, 0);

    for (i=0; i<truncated_lines.size(); ++i) {            
        x_AddLineToWidget(text, truncated_lines[i], text_extent);
    }

#ifdef RICH_TEXT_TOOLTIP
    m_TextWidget = text;
#endif
}

void CTooltipFrame::x_CreateHtmlTextControl(wxPanel* p, int title_width, int max_width)
{
    size_t i;  
    int font_size = 10; 
    wxSize text_extent(title_width, 0);
    wxFont font(font_size, wxFONTFAMILY_ROMAN, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL);      

    // First decide if this is html or raw text.  If it is raw text, compute
    // the size and convert it to Html.
    string tip_text = m_TipInfo.GetTipText();

    bool is_html = false;
    if (NStr::Find(tip_text, "<html", NStr::eNocase) != NPOS)
        is_html = true;
    else if (NStr::Find(tip_text, "<table", NStr::eNocase) != NPOS)
        is_html = true;
    else if (NStr::Find(tip_text, "<p>", NStr::eNocase) != NPOS)
        is_html = true;
    else if (NStr::Find(tip_text, "<div", NStr::eNocase) != NPOS)
        is_html = true;

    std::vector<string> truncated_lines;

    // End size computation.
    // ********************************************************************
    //

    // Compute the text margin area in pixels, so that it can be added
    // to the control size
    wxWindowDC dc(this);
    wxSize pixperinch = dc.GetPPI();
    int pixel_margin_x = (int)(2.0f*((float)pixperinch.x)*(1.0f/72.0f)*
        (float)wxRICHTEXT_DEFAULT_MARGIN);
    int pixel_margin_y = (int)(2.0f*((float)pixperinch.y)*(1.0f/72.0f)*
        (float)wxRICHTEXT_DEFAULT_MARGIN);


    // Use this when html includes line breaks:
    string html_text;
    CHtmlWindowtHandle* text;

    if (!is_html) {
        html_text = "<html><body><p>";
        
        truncated_lines = x_BreakIntoLines(tip_text, 1000);
        for (i=0; i<truncated_lines.size(); ++i) {
            html_text += truncated_lines[i];
            if (i<truncated_lines.size()-1)
                html_text += "<BR>";            
        }

        html_text += "</p></body></html>";
    } 
    else {
        html_text = tip_text;
    }
    
    // This is only needed for native html since we can use
    // the methods from the rich text version to compute the
    // size of text-only tips, but this seems to work for both.
    {
        // We create the html control twice to find the correct dimensions.
        // This approach works for the html we've tried so far, but
        // is not documented anywhere so it may not work in the future.
        CHtmlWindowtHandle* text2 = new CHtmlWindowtHandle(p, m_Handle, wxDefaultSize);
        text2->SetStandardFonts(font_size, _("Times New Roman"), wxEmptyString);
        text2->SetBorders(0);
        text2->SetPage(html_text);
        text2->Layout();

        // The first time with the default size, teh width reported will be 
        // correct so we take that and use it to re-create the control.
        // When we re-create the control, the internal representation will
        // then report the correct height, so we finally use the width
        // from the first creation and height from the second to reseet
        // the size for the control.     

        wxHtmlContainerCell* cell = text2->GetInternalRepresentation();
        int w1 = cell->GetMaxTotalWidth();
        // With small text GetMaxTotalWidth() of Release build of wxWidgets on OSX returns 0
        // probably bug in wxWidgets
        // here is workaround. GB-8421
        if (w1 == 0) w1 = cell->GetWidth();
        if (w1 > max_width)
            w1 = max_width;
        text2->Destroy();

        // Re-create with the width from the internal representation the first time
        text = new CHtmlWindowtHandle(p, m_Handle,
            wxSize(w1 + pixel_margin_x, wxDefaultSize.GetHeight() ));
        text->SetStandardFonts(font_size, _("Times New Roman"), wxEmptyString);
        text->SetBorders(0);
        text->SetPage(html_text);
        text->Layout();

        // Now get the height from the internal representation
        // and use that and the previous width to set actual size.
        int h2 = text->GetInternalRepresentation()->GetHeight();      
        text->SetSizeHints(w1 + pixel_margin_x, 
            h2 + pixel_margin_y);
        text->Layout();
    }

#ifdef HTML_TOOLTIP
    m_TextWidget = text;
#endif
}

void CTooltipFrame::x_CreateHtmlTextControlTest(wxPanel* p, int title_width)
{
    int font_size = 10; 
    wxSize text_extent(title_width, 0);
    wxFont font(font_size, wxFONTFAMILY_ROMAN, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL);      

    // Compute the text margin area in pixels, so that it can be added
    // to the control size
    wxWindowDC dc(this);
    wxSize pixperinch = dc.GetPPI();
    int pixel_margin_x = (int)(2.0f*((float)pixperinch.x)*(1.0f/72.0f)*
        (float)wxRICHTEXT_DEFAULT_MARGIN);
    int pixel_margin_y = (int)(2.0f*((float)pixperinch.y)*(1.0f/72.0f)*
        (float)wxRICHTEXT_DEFAULT_MARGIN);

    string html_text;
    html_text = "<table margin=\"0\" padding=\"0\" border=\"0\"><tr margin=\"0\" padding=\"0\" border=\"5\"><td valign=\"top\" align=\"right\" cellpadding=\"0\" nowrap=\"nowrap\"><span style=\"font-weight:bold; font-size:12pt\">Variation ID:</span></td><td valign=\"top\"><span style=\"font-weight:bold; font-size:12pt\">rs192879263</span></td></tr><tr margin=\"0\" padding=\"0\" border=\"5\"><td valign=\"top\" align=\"right\" cellpadding=\"0\" nowrap=\"nowrap\"><span style=\"font-weight:bold; font-size:12pt\">Variant Type:</span></td><td valign=\"top\"><span style=\"font-weight:bold; font-size:12pt\">SNP</span></td></tr><tr margin=\"0\" padding=\"0\" border=\"5\"><td valign=\"top\" align=\"right\" cellpadding=\"0\" nowrap=\"nowrap\"><span style=\"font-weight:bold; font-size:12pt\">Alleles:</span></td><td valign=\"top\"><span style=\"font-weight:bold; font-size:12pt\">A/G</span></td></tr></table> \
<p>total length: 1 \
<br>Position: 83,657,295<br> \
Strand: plus<br> \
<br> \
Links: \
<a href=\"https://www.ncbi.nlm.nih.gov/snp/rs192879263\">SNP summary for rs192879263</a> \
</p>";


    // We create the html control twice to find the correct dimensions.
    // This approach works for the html we've tried so far, but
    // is not documented anywhere so it may not work in the future.
    CHtmlWindowtHandle* text2 = new CHtmlWindowtHandle(p, m_Handle, wxDefaultSize);
    text2->SetStandardFonts(font_size, _("Times New Roman"), wxEmptyString);
    text2->SetBorders(0);
    text2->SetPage(html_text);
    text2->Layout();
    
    // The first time with the default size, teh width reported will be 
    // correct so we take that and use it to re-create the control.
    // When we re-create the control, the internal representation will
    // then report the correct height, so we finally use the width
    // from the first creation and height from the second to reseet
    // the size for the control.    
    int w1 = text2->GetInternalRepresentation()->GetMaxTotalWidth();
    text2->Destroy();

    // Re-create with the width from the internal representation the first time
    CHtmlWindowtHandle* text = new CHtmlWindowtHandle(p, m_Handle,
        wxSize(w1 + pixel_margin_x, wxDefaultSize.GetHeight() ));
    text->SetStandardFonts(font_size, _("Times New Roman"), wxEmptyString);
    text->SetBorders(0);
    text->SetPage(html_text);
    text->Layout();   

    // Now get the height from the internal representation
    // and use that and the previous width to set actual size.
    int h2 = text->GetInternalRepresentation()->GetHeight();    
    text->SetSizeHints(w1 + pixel_margin_x, 
        h2 + pixel_margin_y);
    text->Layout();

#ifdef HTML_TOOLTIP
    m_TextWidget = text;
#endif
}




wxBitmapButton* CTooltipFrame::x_AddButton(wxPanel* p,
                                           EStickyToolTipCommands cmd,
                                           const char* img_name,
                                           const char* img_pressed_name,
                                           wxSize button_size,
                                           wxFileArtProvider* provider)
{
    wxBitmapButton* b = new wxBitmapButton(p, 
                                           cmd,                                           
                                           provider->GetBitmap(ToWxString(img_name)),
                                           wxDefaultPosition, 
                                           button_size,
                                           wxBU_EXACTFIT|wxBORDER_NONE);
    // Cocoa looks better when it computs the pressed state directly
#ifndef __WXOSX_COCOA__
    b->SetBitmapSelected(provider->GetBitmap(ToWxString(img_pressed_name)));
#endif
    b->SetClientData(this);
    b->SetBackgroundColour(wxColour(255, 255, 225));
    
    return b;
}

std::vector<std::string> 
CTooltipFrame::x_BreakIntoLines(const string& tip_text, int max_line_len)
{
    // Break text up into lines based on \n's in the text, so that we
    // can add them one at a time.  If last line is blank, remove it.
    std::vector<string> lines;
    std::vector<string> truncated_lines;
    NStr::Split(tip_text, std::string("\n"), lines);
    if (lines.size() > 0 &&
        lines[lines.size()-1] == "")
        lines.erase(lines.begin() + lines.size()-1);

    for (size_t l=0; l<lines.size(); ++l) {
        if (lines[l].length() <= (size_t)max_line_len) {
            truncated_lines.push_back(lines[l]);
        }
        else {
            std::string remainder = lines[l];
 
            //
            // Evaluate the line which is > max_line_len taking chunks off 
            // the front until the line is all written out
            //
            string::size_type break_idx = max_line_len;
            int break_priority = -1;
            string::size_type b = 70;
            string::size_type b_end = max_line_len;

            // While line not empty
            while (remainder.length() > (size_t)max_line_len) {
            
                //
                // Look for URLs in the text and do NOT ever break them up in 
                // the middle.  
                // 
                string::size_type url_start_idx = remainder.find("<a href=\"", 0);

                // URL example
                // <a href=\"https://www.ncbi.nlm.nih.gov\">Visit NCBI</a>,

                // If there is a url within the next 'max_line_len' characters
                if (url_start_idx != string::npos && url_start_idx < (size_t)max_line_len) {
                    
                    // Get terminating url index and location of text:
                    string::size_type url_stop_idx = remainder.find("</a>", 
                        url_start_idx);
                    string::size_type text_start_idx = remainder.find("\">", 
                        url_start_idx);

                    // Don't consider a URL unless it has and ending and text
                    if (url_stop_idx != std::string::npos &&
                        text_start_idx != std::string::npos) {
                        
                        text_start_idx += 2;
                        // Index for the end of URL text
                        string::size_type text_stop_idx = url_stop_idx -1;
                        // Length of URL display text
                        string::size_type url_text_len = text_stop_idx - 
                                                         text_start_idx + 1;
                        // Length of text after the URL in current string
                        string::size_type text_after_url = remainder.length() - 
                                                           url_stop_idx - 1;
                        url_stop_idx += 3;

                        //
                        // Based on the length of the text before the URL, the
                        // length of the URL text itself, and the length of the
                        // text after the URL, write out the text before, in, 
                        // and after the URL as appropriate while keeping line
                        // lengths under max_line_len and not breaking up 
                        // the URL across multiple lines
                        //

                        // if the url text by itself is >= maximum line length, write
                        // out the url and the text that preceeds it on 2 separate lines
                        if (url_text_len >= (size_t)max_line_len) {
                            truncated_lines.push_back(remainder.substr(0, url_start_idx));
                            truncated_lines.push_back(remainder.substr(url_start_idx, 
                                url_stop_idx - url_start_idx + 1));
                            remainder = remainder.substr(url_stop_idx+1, string::npos);
                            continue;
                        }
                        // if url text + the text that preceeds it is > max_line_len,
                        // write out the text that proceeds the url
                        else if (url_start_idx + url_text_len > (size_t)max_line_len) {
                            truncated_lines.push_back(remainder.substr(0, url_start_idx));
                            remainder = remainder.substr(url_start_idx, string::npos);
                            continue;
                        }
                        // If url text plus the text that preceeds and follows it 
                        // are < max_line_len, write out the entire line
                        else if (url_start_idx + url_text_len + text_after_url <= (size_t)max_line_len) {
                            truncated_lines.push_back(remainder);
                            remainder = "";
                            continue;
                        }
                        // url text + preceeding text < max_line_len, but length
                        // of entire line is > max_line_len so set the proper 
                        // indices to search for the first break following the 
                        // url (with the
                        else {
                            b = url_stop_idx;
                            break_priority = 5;
                            b_end = std::max(url_stop_idx + 10, 
                                url_stop_idx + max_line_len - (url_text_len + url_start_idx));
                        }                        
                    }
                }

                //
                // Look for the for best candidate line-break character in
                // the given range.  Better break characters (e.g. ' ') always
                // take priority over worse ones, and break characters of the
                // same precedence closer to 'b_end' take precedence over the
                // same characters closer to 'b'.
                for (; b<b_end; ++b) {
                    if (remainder[b] == ';') {
                        break_idx = b;
                        break_priority = 6;
                    }
                    else if (remainder[b] == ' ' && 
                             break_priority <= 5) {
                        break_idx = b;
                        break_priority = 5;
                    }
                    else if (remainder[b] == ':' && 
                             break_priority <= 4) {
                        break_idx = b;
                        break_priority = 4;
                    }
                    else if (remainder[b] == '-' && 
                             break_priority <= 3) {
                        break_idx = b;
                        break_priority = 3;
                    }
                    else if (remainder[b] == '.' && 
                             break_priority <= 2) {
                        break_idx = b;
                        break_priority = 2;
                    }
                    else if (break_priority <= 1) {
                        break_idx = b;
                        break_priority = 1;
                    }
                }

                // push back current line up to first semicolon, space, etc.
                // also remove any blanks at the beginning.
                string::size_type start_idx = remainder.find_first_not_of(' ');
                if (start_idx != std::string::npos &&
                    start_idx < break_idx)
                    truncated_lines.push_back(remainder.substr(start_idx, break_idx+1));
                remainder = remainder.substr(break_idx+1, string::npos);
            }

            if (remainder.length() > 0)
                truncated_lines.push_back(remainder);
        }
    }

    return truncated_lines;
}


void CTooltipFrame::x_AddLineToWidget(wxRichTextCtrl* ctrl,
                                      const std::string& l, 
                                      wxSize& text_extent)
{
    wxString line_break = (wxChar) 29;

    wxRichTextAttr urlStyle;
    urlStyle.SetTextColour(*wxBLUE);
    urlStyle.SetFontUnderlined(true);

    // Add the current line in "l", parsing out any urls as we go.  If there 
    // are no urls in the line, the whole line is added at once.  The urls
    // are expected to be in standard html format, all lower case:
    // "A url: <a href="https://www.ncbi.nlm.nih.gov">Visit NCBI</a> format".
    // Multiple urls on one line are allowed.

    std::string remainder = l;
    std::string::size_type idx1 = 0;
    std::string::size_type idx2 = 0;
    std::string::size_type idx3 = 0;

    idx1 = remainder.find("<a href=\"", 0);

    // While new urls are found 
    while (idx1 != std::string::npos) {
        // write out text before url
        if (idx1 > 0) {
            ctrl->WriteText(ToWxString(remainder.substr(0, idx1)));
        }
        
        // Get end of web address in url:
        idx2 = remainder.find("\">", idx1);
        if (idx2 != std::string::npos) {
            size_t url_len = (idx2-1)-(idx1+8);  
            std::string url = remainder.substr(idx1+9, url_len);

            // Find end of text in url:
            idx3 = remainder.find("</a>", idx2);
            if (idx3 != std::string::npos) {
                size_t description_len = idx3 - (idx2+2);
                std::string url_text = remainder.substr(idx2+2, description_len);
                
                ctrl->BeginStyle(urlStyle);
                ctrl->BeginURL(ToWxString(url));
                ctrl->WriteText(ToWxString(url_text));
                ctrl->EndURL();
                ctrl->EndStyle();

                // Set remainder to all the text in the line after the url
                remainder = remainder.substr(idx3+4, remainder.size()-(idx3+4));
            }
            else {
                // parsing error (invalid format)
                _TRACE("Invalid format: " << l.c_str());
                break;
            }
        }
        else {
            // parsing error (invalid format)
            _TRACE("Invalid format: " << l.c_str());
            break;
        }

        // See if line has any more urls. If not, we will write out 
        // string remainder after loop
        idx1 = remainder.find("<a href=\"", 0);
    }

    // Write out line (in case of lines with urls, all text after last url)
    if (remainder != "")
        ctrl->WriteText(ToWxString(remainder));

    // Write a break after every line
    ctrl->WriteText(line_break);   
   
    // add the extent of the current line to the overall size of text_extent
    wxRichTextLine* rl;
    rl = ctrl->GetBuffer().GetLineAtPosition(ctrl->GetLastPosition());           
    if (rl != NULL) {
        text_extent.SetWidth(std::max(text_extent.GetWidth(), rl->GetSize().GetWidth()));
        text_extent.SetHeight(std::max(text_extent.GetHeight(),
                                       rl->GetPosition().y + rl->GetSize().GetHeight()));
    }
}

END_NCBI_SCOPE
