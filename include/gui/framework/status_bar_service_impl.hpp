#ifndef GUI_FRAMEWORK___STATUS_BAR_SERVICE_IMPL__HPP
#define GUI_FRAMEWORK___STATUS_BAR_SERVICE_IMPL__HPP

/*  $Id: status_bar_service_impl.hpp 31806 2014-11-17 18:18:01Z katargir $
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
 *  CStatusBarService - standard implementation of IStatusBarService.
 */

/** @addtogroup GUI_FRAMEWORK
*
* @{
*/

#include <corelib/ncbiobj.hpp>

#include <gui/gui_export.h>

#include <gui/framework/status_bar_service.hpp>
#include <gui/framework/service.hpp>

#include <gui/utils/event_handler.hpp>

// This header must (at least indirectly) precede any wxWidgets headers.
#include <gui/widgets/wx/fixed_base.hpp>

#include <wx/event.h>
#include <wx/panel.h>
#include <wx/frame.h>
#include <wx/timer.h>
#include <wx/bitmap.h>


class wxBoxSizer;
class wxFileArtProvider;
class wxTimerEvent;
class wxGauge;
class wxHyperlinkEvent;


BEGIN_NCBI_SCOPE

class CStatusBar;
class CMessageSlot;
class CStatusBarServiceEvtHandler;
class CEventLogService;

///////////////////////////////////////////////////////////////////////////////
/// CStatusBarService - the standard implementation of IStatusBarService
/// interface. (see IStatusBarService declaration for information)

class NCBI_GUIFRAMEWORK_EXPORT  CStatusBarService :
    public CObjectEx,
    public IStatusBarService,
    public IService,
    public IServiceLocatorConsumer,
    public CEventHandler
{
    friend class CStatusBarServiceEvtHandler;
    DECLARE_EVENT_MAP();
public:
    CStatusBarService();
    virtual ~CStatusBarService();

    /// @name IService interface implementation
    /// @{
    virtual void    InitService();
    virtual void    ShutDownService();
    /// @}

    /// @name IServiceLocatorConsumer implementation
    /// @{
    virtual void    SetServiceLocator(IServiceLocator* locator);
    /// @}

    virtual wxStatusBar*    CreateStatusBar(wxFrame* frame);
    virtual void    SetEventDisplayTime(int sec);

    /// @name IStatusBarService implementation
    /// @{
    virtual void    SetStatusMessage(const string& msg);
    virtual void    AddEventMessage(const IEventRecord& record);
    virtual void    ShowHintMessage(const string& msg);
    virtual void    HideHintMessage();
    virtual void    ShowStatusProgress(int value, int range);
    virtual void    ShowStatusProgress();
    virtual void    HideStatusProgress();

    virtual void    InsertSlot(int index, wxWindow* slot, int width = 50);
    virtual wxWindow*   RemoveSlot(int index);
    virtual void    SetStatusText(const wxString& text, int index);
    /// @}

protected:
    enum    EMessageMode {
        eInvalid = -1,
        eStatus,
        eEvent,
        eHint
    };

    bool    x_HasEvent() const
    {
        return m_LastEventRecord.GetPointer() != NULL;
    }

    void    x_TerminateEventMode();
    void    x_TerminateHintMode();

    void    x_UpdateMessageField();

    void    x_OnNewRecord(CEvent*);

protected:
    CStatusBarServiceEvtHandler*    m_Handler;

    CStatusBar* m_StatusBar;  /// The Status Bar associated with the service

    EMessageMode  m_MessageMode; /// current mode for Message Slot

    string  m_StatusMessage;

    int m_EventDisplayTime; // seconds
    
    typedef  CConstIRef<IEventRecord> TEventRecRef;
    TEventRecRef   m_LastEventRecord;
    wxTimer m_EventTimer;
    wxTimer m_PulseTimer;

    string m_HintMessage;

    CMessageSlot*   m_MessageSlot;

    CEventLogService* m_EventLogService;
};


///////////////////////////////////////////////////////////////////////////////
/// CStatusBarServiceEvtHandler
class CStatusBarServiceEvtHandler :
    public wxEvtHandler
{
    DECLARE_EVENT_TABLE();
public:
    CStatusBarServiceEvtHandler(CStatusBarService& service);

    void    OnEventTimer(wxTimerEvent& event);
    void    OnPulseTimer(wxTimerEvent& event);

protected:
    CStatusBarService&  m_Service;
};


///////////////////////////////////////////////////////////////////////////////
/// CMessageSlot
class CMessageSlotPopupWindow;

class   NCBI_GUIFRAMEWORK_EXPORT  CMessageSlot :
    public wxPanel
{
    DECLARE_EVENT_TABLE()
public:
    typedef wxPanel TParent;

    CMessageSlot(wxStatusBar* parent);
    virtual ~CMessageSlot();

    virtual void    Reset();
    virtual void    SetText(const string& text);
    virtual void    SetRecord(const IEventRecord& record);

    virtual void    ShowStatusProgress(int value, int range);
    virtual void    ShowStatusProgress();
    virtual void    HideStatusProgress();

    virtual void    RefreshWithParent();

    void    OnPaint(wxPaintEvent& event);
    void    OnMouseEnter(wxMouseEvent& event);
    void    OnMouseLeave(wxMouseEvent& event);
    void    OnLeftDown(wxMouseEvent& event);

    /// Callbacks
    void    OnPopupClosed();

protected:
    void    x_CreateGaugeIfNeeded();
    void    x_DestroyGauge();

    void    x_SetText(const string& text);

    bool    x_CanShowPopup();
    bool    x_IsPopupShown();
    void    x_ShowPopup();
    void    x_HidePopup();

protected:
    wxBitmap    m_Icon;
    string      m_Text;
    CConstIRef<IEventRecord> m_EventRecord;

    bool    m_Hot; // true if slot is displaying "hot track"
    int     m_PaintWidth; // width of the graphics drawn in OnPaint()

    wxGauge*    m_Gauge;
    CMessageSlotPopupWindow*  m_Popup;
};


///////////////////////////////////////////////////////////////////////////////
/// CMessageSlotPopupWindow
class NCBI_GUIFRAMEWORK_EXPORT  CMessageSlotPopupWindow :
        public wxFrame
{
    typedef wxFrame TParent;

    DECLARE_EVENT_TABLE()
public:
    /// set text and command for the link shown at the bottom of the window
    static void     SetExtraLink(const string& label, int cmd);

    CMessageSlotPopupWindow(CMessageSlot* slot,
                            wxRect slot_rect,
                            const IEventRecord& record,
                            int show_event_view_cmd);

    void    OnActivate(wxActivateEvent& event);
    void    OnLink(wxHyperlinkEvent& event);

protected:
    void    x_CreateMessageSlotPanel();

protected:
    static string   m_ExtraLabel;
    static int      m_ExtraCmd;

    CMessageSlot*   m_Slot;
    CConstIRef<IEventRecord> m_EventRecord;
    int m_ShowEventViewCmd;
};


END_NCBI_SCOPE

/* @} */

#endif  // GUI_FRAMEWORK___STATUS_BAR_SERVICE_IMPL__HPP

