/*  $Id: status_bar_service_impl.cpp 43837 2019-09-09 21:12:26Z katargir $
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
 */


#include <ncbi_pch.hpp>

#include <gui/framework/status_bar_service_impl.hpp>
#include <gui/framework/status_message_slot_panel.hpp>
#include <gui/framework/event_log_service_impl.hpp>

#include <gui/widgets/wx/app_status_bar.hpp>
#include <gui/widgets/wx/wx_utils.hpp>
#include <gui/widgets/wx/fileartprov.hpp>
#include <gui/widgets/wx/richtextctrl.hpp>

#include <corelib/ncbitime.hpp>

#include <wx/app.h>
#include <wx/sizer.h>
#include <wx/artprov.h>
#include <wx/gauge.h>
#include <wx/html/htmlwin.h>
#include <wx/dcclient.h>
#include <wx/richtext/richtextctrl.h>
#include <wx/hyperlink.h>


BEGIN_NCBI_SCOPE

static int kEventTimerID = 13;
static int kPulseTimerID = 14;

CStatusBarService::CStatusBarService()
:   m_StatusBar(NULL),
    m_MessageMode(eStatus),
    m_EventDisplayTime(30),
    m_MessageSlot(NULL),
    m_EventLogService()
{
    m_Handler = new CStatusBarServiceEvtHandler(*this);
    m_EventTimer.SetOwner(m_Handler, kEventTimerID);
    m_PulseTimer.SetOwner(m_Handler, kPulseTimerID);
}

BEGIN_EVENT_MAP(CStatusBarService, CEventHandler)
    ON_EVENT(CLogServiceEvent, CLogServiceEvent::eNewRecord, &CStatusBarService::x_OnNewRecord)
END_EVENT_MAP()

CStatusBarService::~CStatusBarService()
{
    delete m_Handler;
     // do not delete m_StatusBar
}


void CStatusBarService::InitService()
{
}


void CStatusBarService::ShutDownService()
{
    m_LastEventRecord.Reset();
    m_StatusBar = NULL;
}


wxStatusBar* CStatusBarService::CreateStatusBar(wxFrame* frame)
{
    m_StatusBar = new CStatusBar(frame, wxID_ANY);

    // create Message Slot
    m_MessageSlot = new CMessageSlot(m_StatusBar);
    InsertSlot(0, m_MessageSlot, -100);

    frame->SetStatusBar(m_StatusBar);
    m_StatusBar->Layout();
    return m_StatusBar;
}


void CStatusBarService::InsertSlot(int index, wxWindow* slot, int width)
{
    m_StatusBar->InsertFieldWindow(index, slot, width);
}


wxWindow* CStatusBarService::RemoveSlot(int index)
{
    m_StatusBar->RemoveFieldWindow(index);
    return NULL;
}

void CStatusBarService::SetStatusText(const wxString& text, int index)
{
    m_StatusBar->SetStatusText(text, index);
}

void CStatusBarService::SetEventDisplayTime(int sec)
{
    _ASSERT(sec > 2  &&  sec < 3600);
    m_EventDisplayTime = sec;
}


void CStatusBarService::SetStatusMessage(const string& msg)
{
    //LOG_POST(Info << "CStatusBarService::SetStatusMessage() " << msg);

    m_StatusMessage = msg;

    if(m_MessageMode == eStatus)  {
        // message change does affect the display
        x_UpdateMessageField();
    }
}


void CStatusBarService::AddEventMessage(const IEventRecord& record)
{
    //LOG_POST(Info << "CStatusBarService::AddEventMessage() " << record.GetTitle());

    if(x_HasEvent()) {
        // already have an event message - remove it
        m_EventTimer.Stop();
        m_LastEventRecord.Reset();
    }

    m_LastEventRecord.Reset(&record);
    if(m_MessageMode == eStatus  ||  m_MessageMode == eEvent)    {
        m_MessageMode = eEvent;
        x_UpdateMessageField();
    }

    // set timeout for hiding this message
    m_EventTimer.Start(m_EventDisplayTime * 1000, true);
}


void CStatusBarService::ShowHintMessage(const string& msg)
{
    //LOG_POST(Info << "CStatusBarService::ShowHintMessage() " << msg);
    m_HintMessage = msg;

    if(m_HintMessage.empty())   {
        x_TerminateHintMode();
    } else {
        // start Hint Mode
        m_MessageMode = eHint;
        x_UpdateMessageField();
    }
}


void CStatusBarService::HideHintMessage()
{
    m_HintMessage = "";

    x_TerminateHintMode();
}


void CStatusBarService::ShowStatusProgress(int value, int range)
{
    //LOG_POST(Info << "CStatusBarService::ShowStatusProgress() " << value );
    m_PulseTimer.Stop();
    m_MessageSlot->ShowStatusProgress(value, range);
}


void CStatusBarService::ShowStatusProgress()
{
    //LOG_POST(Info << "CStatusBarService::ShowStatusProgress() ");
    m_MessageSlot->ShowStatusProgress();
    m_PulseTimer.Start(250);
}


void CStatusBarService::HideStatusProgress()
{
    //LOG_POST(Info << "CStatusBarService::HideStatusProgress()");
    m_PulseTimer.Stop();
    m_MessageSlot->HideStatusProgress();
}

void CStatusBarService::SetServiceLocator(IServiceLocator* serviceLocator)
{
    if (serviceLocator) {
        m_EventLogService = serviceLocator->GetServiceByType<CEventLogService>();
        m_EventLogService->AddListener(this);
    } else {
        if (m_EventLogService) {
            m_EventLogService->RemoveListener(this);
            m_EventLogService = 0;
        }
    }
}

void  CStatusBarService::x_OnNewRecord(CEvent*)
{
    if (m_EventLogService) {
        size_t count = m_EventLogService->GetRecordsCount();
        if(count > 0)   {
            const IEventRecord* rec = m_EventLogService->GetRecord(count - 1);
            _ASSERT(rec);
            AddEventMessage(*rec);
        }
    }
}

void CStatusBarService::x_TerminateEventMode()
{
    if(m_MessageMode == eEvent)    {
        m_MessageMode = eStatus;
        if (!m_StatusMessage.empty()) // keep last event message if no status message present
            x_UpdateMessageField();
    }
    m_LastEventRecord.Reset();
}


void CStatusBarService::x_TerminateHintMode()
{
    if(m_MessageMode == eHint)    {
        if(x_HasEvent())    {
            m_MessageMode = eEvent;
        } else {
            m_MessageMode = eStatus;
        }
        x_UpdateMessageField();
    }
}


void CStatusBarService::x_UpdateMessageField()
{
    _ASSERT(m_StatusBar);
    if(m_StatusBar) {
        string msg, icon_alias;
        switch(m_MessageMode)  {
        case eStatus:
            m_MessageSlot->SetText(m_StatusMessage);
            break;
        case eEvent:
            m_MessageSlot->SetRecord(*m_LastEventRecord);
            break;
        case eHint:
            m_MessageSlot->SetText(m_HintMessage);
            break;
        default:
            break;
        }
    }
}


///////////////////////////////////////////////////////////////////////////////
/// CStatusBarServiceEvtHandler

BEGIN_EVENT_TABLE(CStatusBarServiceEvtHandler, wxEvtHandler)
    EVT_TIMER(kEventTimerID, CStatusBarServiceEvtHandler::OnEventTimer)
    EVT_TIMER(kPulseTimerID, CStatusBarServiceEvtHandler::OnPulseTimer)
END_EVENT_TABLE()


CStatusBarServiceEvtHandler::CStatusBarServiceEvtHandler(CStatusBarService& service)
:   m_Service(service)
{
}


void CStatusBarServiceEvtHandler::OnEventTimer(wxTimerEvent& event)
{
    m_Service.x_TerminateEventMode();
}


void CStatusBarServiceEvtHandler::OnPulseTimer(wxTimerEvent& event)
{
    m_Service.m_MessageSlot->ShowStatusProgress();
}


///////////////////////////////////////////////////////////////////////////////
/// CMessageSlot

BEGIN_EVENT_TABLE(CMessageSlot, CMessageSlot::TParent)
    EVT_PAINT(CMessageSlot::OnPaint)
    //EVT_CONTEXT_MENU(CMessageSlot::OnContextMenu)
    EVT_ENTER_WINDOW(CMessageSlot::OnMouseEnter)
    EVT_LEAVE_WINDOW(CMessageSlot::OnMouseLeave)
    EVT_LEFT_DOWN(CMessageSlot::OnLeftDown)
    EVT_LEFT_DCLICK(CMessageSlot::OnLeftDown)
    //EVT_LEFT_UP(CMessageSlot::OnLeftUp)
    //EVT_MOUSE_CAPTURE_LOST(CMessageSlot::OnMouseCaptureLost)
END_EVENT_TABLE()


CMessageSlot::CMessageSlot(wxStatusBar* parent)
:   TParent(parent),
    m_Icon(wxNullBitmap),
    m_Hot(false),
    m_PaintWidth(0),
    m_Gauge(NULL),
    m_Popup(NULL)
{
    SetBackgroundStyle(wxBG_STYLE_CUSTOM);
}


CMessageSlot::~CMessageSlot()
{
}



void CMessageSlot::Reset()
{
    //x_HidePopup();
    m_Hot = false;

    m_Icon = wxNullBitmap;

    m_Text = "";
    SetToolTip(wxT(""));

    m_EventRecord.Reset();

    RefreshWithParent();
}


void CMessageSlot::SetText(const string& text)
{
    Reset();
    x_SetText(text);
}


void CMessageSlot::x_SetText(const string& text)
{
    m_Text = text;

    wxString tooltip;
    if( ! m_Text.empty())   {
        tooltip = ToWxString(m_Text) + wxT("\n");
    }
    tooltip += wxT("Click for details");
    SetToolTip(tooltip);
}


void CMessageSlot::SetRecord(const IEventRecord& record)
{
    Reset();

    m_EventRecord.Reset(&record);

    IEventRecord::EType type = m_EventRecord->GetType();
    //TODO
    if(type == IEventRecord::eInfo) {
        m_Icon = wxArtProvider::GetBitmap(wxT("about"));
    } else {
        m_Icon = wxArtProvider::GetBitmap(wxT("zoom_in"));
    }

    x_SetText(m_EventRecord->GetTitle());
}


void CMessageSlot::ShowStatusProgress(int value, int range)
{
    x_CreateGaugeIfNeeded();

    m_Gauge->SetRange(range);
    m_Gauge->SetValue(value);
}


void CMessageSlot::ShowStatusProgress()
{
    x_CreateGaugeIfNeeded();

    m_Gauge->Pulse();
    //m_Gauge->Refresh();
}


void CMessageSlot::HideStatusProgress()
{
    x_DestroyGauge();
}


void CMessageSlot::RefreshWithParent()
{
    GetParent()->RefreshRect(GetRect());
}


static const int kSepX = 4;

void CMessageSlot::OnPaint(wxPaintEvent& event)
{
    wxPaintDC dc(this);

    wxRect rc = GetClientRect();

    // fill background
    wxColour back_cl = CStatusBar::GetBackColor(m_Hot);
    wxBrush brush(back_cl);
    dc.SetBrush(brush);
    dc.SetPen(*wxTRANSPARENT_PEN);
    dc.DrawRectangle(rc.x, rc.y, rc.width, rc.height);

    int x = rc.GetLeft() + kSepX;
    int y;
    int right = rc.GetRight();

    // render icon on the left
    if(m_Icon.IsOk())  {
        int i_h = m_Icon.GetHeight();
        y = rc.GetTop() + (rc.GetHeight() - i_h) / 2;

        dc.DrawBitmap(m_Icon, x, y, true);
        x += m_Icon.GetWidth() + kSepX;
    }

    int av_w = right - x;

    // render the text
    wxFont font = GetFont();
    font.SetUnderlined(m_Hot);
    dc.SetFont(font);

    wxColor text_cl = CStatusBar::GetTextColor(m_Hot);
    dc.SetTextForeground(text_cl);

    wxString s = ToWxString(m_Text);
    wxString ts = TruncateText(dc, s, av_w, ewxTruncate_Ellipsis);

    wxCoord cw = 0, ch = 0, cdescent = 0;
    dc.GetTextExtent(s, &cw, &ch, &cdescent);
    y = rc.GetTop() + (rc.GetHeight() - ch) / 2;
#if defined(__WXOSX_CARBON__) || defined(__WXMAC_CARBON__)
    y-=1;
#endif
    dc.DrawText(ts, x, y);

    m_PaintWidth = x + cw + kSepX;
}


void CMessageSlot::OnMouseEnter(wxMouseEvent& event)
{
    if(x_CanShowPopup())  {
        m_Hot = true;
        RefreshWithParent();
    }
}


void CMessageSlot::OnMouseLeave(wxMouseEvent& event)
{
    if(m_Hot)   {
        m_Hot = false;
        RefreshWithParent();
    }
}


void CMessageSlot::OnLeftDown(wxMouseEvent& event)
{
    //LOG_POST("CMessageSlot::OnLeftDown");
    if(m_Hot  &&  ! x_IsPopupShown())   {
        x_ShowPopup();
    }
}

void CMessageSlot::OnPopupClosed()
{
    m_Popup = NULL;
    RefreshWithParent();
}


void CMessageSlot::x_CreateGaugeIfNeeded()
{
    if( ! m_Gauge)  {
        wxSize size = GetSize();
        m_Gauge = new wxGauge(this, wxID_ANY, 100,
                              wxDefaultPosition, wxSize(80, size.GetHeight()),
                              wxGA_HORIZONTAL);

        wxBoxSizer* sizer = new wxBoxSizer(wxHORIZONTAL);
        sizer->AddStretchSpacer();
        sizer->Add(m_Gauge, 0, wxEXPAND | wxALIGN_RIGHT, 0);
        SetSizer(sizer);
        Layout();
    }
}


void CMessageSlot::x_DestroyGauge()
{
    if(m_Gauge) {
        m_Gauge->Hide();
        m_Gauge->Destroy();
        m_Gauge = NULL;
        RefreshWithParent();
    }
}


bool CMessageSlot::x_CanShowPopup()
{
    return m_EventRecord.NotEmpty();
}


bool CMessageSlot::x_IsPopupShown()
{
    return m_Popup != NULL;
}


void CMessageSlot::x_ShowPopup()
{
    wxRect rc = ncbi::GetScreenRect(*this);
    rc.y = rc.GetBottom() - 10;
    rc.height = 10;

    m_Popup = new CMessageSlotPopupWindow(this, rc, *m_EventRecord, -1);
    m_Popup->Show();
}


void CMessageSlot::x_HidePopup()
{
    if(m_Popup) {
        m_Popup->Close();
        m_Popup = NULL;
    }
}


///////////////////////////////////////////////////////////////////////////////
/// CMessageSlotPopupWindow

BEGIN_EVENT_TABLE(CMessageSlotPopupWindow, CMessageSlotPopupWindow::TParent)
    EVT_ACTIVATE(CMessageSlotPopupWindow::OnActivate)
    EVT_HYPERLINK(wxID_ANY, CMessageSlotPopupWindow::OnLink)
END_EVENT_TABLE()

string CMessageSlotPopupWindow::m_ExtraLabel;
int CMessageSlotPopupWindow::m_ExtraCmd = -1;

void CMessageSlotPopupWindow::SetExtraLink(const string& label, int cmd)
{
    m_ExtraLabel = label;
    m_ExtraCmd = cmd;
}


CMessageSlotPopupWindow::CMessageSlotPopupWindow(CMessageSlot* slot, wxRect rc,
                                                 const IEventRecord& record,
                                                 int show_event_view_cmd)
:   m_Slot(slot),
    m_EventRecord(&record),
    m_ShowEventViewCmd(show_event_view_cmd)

{
    _ASSERT(m_Slot);

    long style = wxFRAME_FLOAT_ON_PARENT | wxFRAME_NO_TASKBAR | wxNO_BORDER;
    wxWindow* parent = slot->GetParent();
    wxPoint pos = rc.GetLeftTop();
    wxSize sz = rc.GetSize();

    Create(parent, wxID_ANY, wxT(""), pos, sz, style);
    Hide();

    wxColour back_cl = CStatusBar::GetBackColor(false);
    SetBackgroundColour(back_cl);

    x_CreateMessageSlotPanel();

    Show();
}


void CMessageSlotPopupWindow::x_CreateMessageSlotPanel()
{
    wxBoxSizer* sizer = new wxBoxSizer(wxHORIZONTAL);

    CStatusMessageSlotPanel* panel = new CStatusMessageSlotPanel(this);

    wxColour back_cl = CStatusBar::GetBackColor(true);
    panel->SetBackgroundColour(back_cl);

    sizer->Add(panel, 1, wxEXPAND | wxALL, 1);
    SetSizer(sizer);
    Layout();

    // extract event attributes and generate text
    CTime ctime(m_EventRecord->GetTime(), CTime::eMinute);
    static CTimeFormat time_format("b D, H:m:s p");
    string s_time = ctime.ToLocalTime().AsString(time_format);

    wxString title = ToWxString(m_EventRecord->GetTitle());
    wxString descr = ToWxString(m_EventRecord->GetDescription());
    wxString time = ToWxString(s_time);

    // set text, then wrap an layout the panel
    panel->SetTitleText(title);
    panel->SetTimeText(time);
    panel->SetExtraLink(m_ExtraLabel);
    panel->TransferDataToWindow();

    // resize the window to fit title on a single line if possible
    panel->WrapText();

    wxRect rc = GetRect();
    wxSize best_sz = panel->GetBestSize();
    rc.width = std::min(best_sz.x, 600);
    SetSize(rc);

    // now wrap text and adjust height
    panel->SetDescrText(descr);
    panel->TransferDataToWindow();

    panel->WrapText();

    // resize the window to accommodate the panel
    wxSize sz = panel->GetBestSize();
    rc.width = sz.x;
    rc.y = rc.GetBottom() - sz.y;
    rc.height = sz.y;

    SetSize(rc);
}


void CMessageSlotPopupWindow::OnLink(wxHyperlinkEvent& event)
{
    if(m_ExtraCmd >= 0)  {
        wxCommandEvent evt(wxEVT_COMMAND_MENU_SELECTED, m_ExtraCmd);
        evt.SetEventObject(this);
        wxTheApp->GetTopWindow()->GetEventHandler()->ProcessEvent(evt);

        m_Slot->OnPopupClosed();
        Close();
    }
}

void CMessageSlotPopupWindow::OnActivate(wxActivateEvent& event)
{
    if( ! event.GetActive())    {
        // window was deactivated
        m_Slot->OnPopupClosed();
        Close();
    }
}

END_NCBI_SCOPE
