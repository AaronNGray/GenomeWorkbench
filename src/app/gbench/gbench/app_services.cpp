/*  $Id: app_services.cpp 38756 2017-06-15 18:00:11Z katargir $
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
 */

#include <ncbi_pch.hpp>

#include "app_services.hpp"
#include "commands.hpp"

#include <gui/framework/app_task_service.hpp>

#include <gui/widgets/wx/app_status_bar.hpp>
#include <gui/widgets/wx/wx_utils.hpp>

#include <gui/widgets/wx/sys_path.hpp>

#include <wx/sizer.h>
#include <wx/dcclient.h>
#include <wx/settings.h>
#include <wx/dcbuffer.h>


BEGIN_NCBI_SCOPE

///////////////////////////////////////////////////////////////////////////////
/// CAppTaskServiceSlot


BEGIN_EVENT_TABLE(CAppTaskServiceSlot, wxPanel)
    EVT_PAINT(CAppTaskServiceSlot::OnPaint)
    EVT_ENTER_WINDOW(CAppTaskServiceSlot::OnMouseEnter)
    EVT_LEAVE_WINDOW(CAppTaskServiceSlot::OnMouseLeave)
    EVT_LEFT_DOWN(CAppTaskServiceSlot::OnLeftDown)
    EVT_LEFT_DCLICK(CAppTaskServiceSlot::OnLeftDown)
END_EVENT_TABLE()

BEGIN_EVENT_MAP(CAppTaskServiceSlot, CEventHandler)
    ON_EVENT(CAppTaskServiceGuiUpdateEvent, CAppTaskServiceGuiUpdateEvent::eUpdate, &CAppTaskServiceSlot::x_OnUpdate)
END_EVENT_MAP()

CAppTaskServiceSlot::CAppTaskServiceSlot()
    : m_Sizer(NULL)
    , m_TextItem(NULL)
    , m_Animation(NULL)
    , m_AppTaskService(NULL)
    , m_Hot(false)
    , m_Idle(true)
{
}

CAppTaskServiceSlot::~CAppTaskServiceSlot()
{
}

void CAppTaskServiceSlot::SetAppTaskService(CAppTaskService* appTaskService)
{
    if (appTaskService) {
        m_AppTaskService = appTaskService;
        m_AppTaskService->AddListener(this);
    } else {
        if (m_AppTaskService) {
            m_AppTaskService->RemoveListener(this);
            m_AppTaskService = 0;
        }
    }
    x_Update();
}


#define ID_ANIMATIONCTRL1   10007

static const int kTextOffX = 4;
static const int kTextOffY = 2;

bool CAppTaskServiceSlot::Create(wxWindow* parent)
{
    long style = wxCLIP_CHILDREN | wxBORDER_NONE;

    if(wxPanel::Create(parent, wxID_ANY,
                       wxDefaultPosition, wxSize(), style)) {
        Hide();
        SetBackgroundStyle(wxBG_STYLE_CUSTOM);
        m_Sizer = new wxBoxSizer(wxHORIZONTAL);

        // create Animation Control
        m_Animation = new wxAnimationCtrl(this, ID_ANIMATIONCTRL1, wxAnimation(),
            wxDefaultPosition, wxSize(18, 18));
      //  m_Animation->SetInactiveBitmap(wxNullBitmap);

        wxString path = CSysPath::ResolvePath(wxT("<res>/task_progress.gif"));
        m_Animation->LoadFile(path);

        m_Sizer->Add(m_Animation, 0, wxLEFT | wxRIGHT, 5);

        // create a Spacer for rendering text
        wxSize text_sz = x_GetTextAreaSize();
        m_TextItem = m_Sizer->Add(text_sz.x, text_sz.y, 1, wxEXPAND);

        SetSizer(m_Sizer);

        Fit();
        wxSize sz = GetSize();
        SetMinSize(sz);
        return true;
    }
    return false;
}


// calculate the minimal area size sufficient for displaying text
wxSize CAppTaskServiceSlot::x_GetTextAreaSize()
{
    string s = x_StatusText(1000, 1000);

    wxClientDC dc(this);
    dc.SetFont(GetFont());
    wxString wx_s = ToWxString(s);

    wxSize size;
    dc.GetTextExtent(wx_s, &size.x, &size.y);

    size.x += 2 * kTextOffX;
    size.y += 2 * kTextOffY;
    return size;
}

void CAppTaskServiceSlot::x_OnUpdate(CEvent*)
{
    x_Update();
}

int CAppTaskServiceSlot::x_GetPendingTasksCount()
{
    CAppTaskService::TRecRefVec recs;
    m_AppTaskService->GetPendingTasks(recs);
    int n = 0;
    NON_CONST_ITERATE(CAppTaskService::TRecRefVec, it, recs)  {
        if((*it)->m_Task->IsVisible())  {
            n++;
        }
    }
    return n;
}


// generates a text string representing the current App Task Service state
string CAppTaskServiceSlot::x_StatusText(int running_n, int pending_n)
{
    string s;
    if(running_n)    {
        s = NStr::IntToString(running_n);
        s += (running_n == 1) ? " Task" : " Tasks";
        s += " running";
    }
    if(pending_n)   {
        if(s.empty())  {
            s = NStr::IntToString(pending_n);
            s += (pending_n == 1) ? " Task" : " Tasks";
        } else {
            s += " / ";
            s += NStr::IntToString(pending_n);
        }
        s += " pending";
    }

    m_Idle = (running_n == 0)  &&  (pending_n == 0);
    if(m_Idle)   {
        s = "Idle";
    }
    return s;
}


void CAppTaskServiceSlot::x_Update()
{
    m_Text = wxT("");
    int running_n = 0;

    if(m_AppTaskService)    {
        int pending_n = x_GetPendingTasksCount();
        running_n = m_AppTaskService->GetRunningTasksCount(true);
        string s = x_StatusText(running_n, pending_n);
        m_Text = ToWxString(s);
    }

    wxString tooltip = m_Text;
    tooltip += wxT("\nClick to see Task View");
    SetToolTip(tooltip);

    // update Animation state
    bool play = (running_n > 0);
    bool playing = m_Animation->IsPlaying();

    if(playing  &&  ! play)    {
        m_Animation->Stop();
        m_Animation->Hide();

    }  else if( ! playing  &&  play)    {
        m_Animation->Show();
        m_Animation->Play();
    }

    Layout();
    Refresh();
}


void CAppTaskServiceSlot::OnPaint(wxPaintEvent& event)
{
    wxPaintDC dc(this);

    // fill background
    wxRect rc = GetRect();
    rc.x = rc.y = 0;
    wxColour back_cl = CStatusBar::GetBackColor(m_Hot);
    wxBrush brush(back_cl);
    dc.SetBrush(brush);
    dc.SetPen(*wxTRANSPARENT_PEN);

    dc.DrawRectangle(rc);

    m_Animation->SetBackgroundColour(back_cl);
    // This call blocks app idle loop with wxWidgets 2.9 and Ubuntu
    //m_Animation->Refresh();

    // draw text
    rc = m_TextItem->GetRect();

    // set text font
    wxFont font = GetFont();
    font.SetUnderlined(m_Hot);
    dc.SetFont(font);

    // set text color
    wxColor cl = CStatusBar::GetTextColor(m_Hot);
    wxColor text_cl = m_Idle ? GetAverage(cl, back_cl, 0.5) : cl;
    dc.SetTextForeground(text_cl);

    // prepare and render the text
    int av_w = rc.width - kTextOffX * 2;
    wxString s = TruncateText(dc, m_Text, av_w);

    int w = 0, h = 0;
    dc.GetTextExtent(s, &w, &h);
    int pos_x = rc.x + kTextOffX;
    int pos_y = rc.y + (rc.height - h) / 2;
#if defined(__WXOSX_CARBON__) || defined(__WXMAC_CARBON__)
    pos_y-=3;
#endif

    dc.DrawText(s, pos_x, pos_y);
}


void CAppTaskServiceSlot::OnMouseEnter(wxMouseEvent& event)
{
    m_Hot = true;
    Refresh();
}


void CAppTaskServiceSlot::OnMouseLeave(wxMouseEvent& event)
{
    m_Hot = false;
    Refresh();
}


void CAppTaskServiceSlot::OnLeftDown(wxMouseEvent& event)
{
    wxCommandEvent evt(wxEVT_COMMAND_MENU_SELECTED, eCmdShowTaskView);
    evt.SetEventObject(this);
    GetEventHandler()->ProcessEvent(evt);
}


END_NCBI_SCOPE
