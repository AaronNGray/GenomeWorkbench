/*  $Id: async_call.cpp 43747 2019-08-28 15:41:08Z katargir $
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
 * Authors:  Roman  Katargin
 *
 */

#include <ncbi_pch.hpp>

#include <corelib/ncbistd.hpp>

#include <gui/utils/app_job_dispatcher.hpp>
#include <gui/widgets/wx/async_call.hpp>
#include <gui/widgets/wx/ind_progress_bar.hpp>
#include <gui/widgets/wx/wx_utils.hpp>
#include <gui/widgets/wx/dock_frames.hpp>
#include <gui/widgets/wx/main_frame.hpp>

#include <wx/utils.h>
#include <wx/dialog.h>
#include <wx/timer.h>
#include <wx/sizer.h>
#include <wx/stattext.h>
#include <wx/button.h>
#include <wx/thread.h>
#include <wx/app.h>
#include <wx/evtloop.h>

BEGIN_NCBI_SCOPE

bool GUI_AsyncExecUnit(IExecuteUnit& exec_unit, const wxString& msg)
{
    return GUI_AsyncExec([&exec_unit](ICanceled& canceled) { return exec_unit.Execute(canceled); }, msg);
}

void GUI_AsyncJoinFuture(const job_future_base& future, const string& msg)
{
    GUI_AsyncExec([&future](ICanceled&) { while (future.IsRunning()) ::wxMilliSleep(10); }, msg);
}

class CAsyncCallDlg : public wxDialog
{
public:
    CAsyncCallDlg(wxWindow* parent, CAsyncCall::Call& call, int jobId);
    bool Create(wxWindow* parent);
    void CreateControls();

    DECLARE_EVENT_TABLE()

    void OnTimer(wxTimerEvent& event);
    void OnCancelClick(wxCommandEvent& event);

private:
    CAsyncCall::Call& m_Call;
    int m_JobId;

    wxTimer m_Timer;

    enum {
        ID_STATIC_TEXT = 10001,
        ID_PROGRESS_IND,
        ID_BUTTON_CANCEL
    };

    CIndProgressBar* m_ProgressInd;
    wxButton* m_CancelBtn;
};

class CExecJob : public CObject, public IAppJob
{
public:
    CExecJob (CAsyncCall::Call* call)
      : m_Call(call) {}
    ~CExecJob();
    virtual EJobState   Run();
    virtual CConstIRef<IAppJobProgress> GetProgress() { return CConstIRef<IAppJobProgress>(); }
    virtual CRef<CObject>   GetResult() { return CRef<CObject>(); }
    virtual CConstIRef<IAppJobError>    GetError() { return CConstIRef<IAppJobError>(); }
    virtual string    GetDescr() const { return NcbiEmptyString; }
    virtual void RequestCancel() { m_Call->RequestCancel(); }
    virtual bool IsCanceled() const { return m_Call->IsCanceled(); }

    void Finish();

private:
    CAsyncCall::Call* m_Call;

    std::exception_ptr m_Exception;
};

IAppJob::EJobState CExecJob::Run()
{
    try {
        m_Call->Execute();
        return eCompleted;
    }
    catch (const std::exception&) {
        m_Exception = std::current_exception();
    }
    return eFailed;
}

CExecJob::~CExecJob()
{
}

void CExecJob::Finish()
{
    if (m_Exception)
        std::rethrow_exception(m_Exception);
}


CAsyncCall::CAsyncCall(Call& call)
: m_Call(call), m_JobId(CAppJobDispatcher::eInvalidJobID)
{
}

static bool fInsideAsyncCall = false;

bool CAsyncCall::InsideAsyncCall()
{
    return fInsideAsyncCall;
}

namespace {
    class CAsyncCallEventLoop : public wxGUIEventLoop
    {
    public:
        CAsyncCallEventLoop(int JobId) : m_JobId(JobId), m_TopWnd() {}
        virtual int Run();
        wxWindow* GetTopWnd() { return m_TopWnd; }
    private:
        int m_JobId;
        wxWindow* m_TopWnd;
    };
} // namespace

int CAsyncCallEventLoop::Run()
{
    CAppJobDispatcher& disp = CAppJobDispatcher::GetInstance();
    wxEventLoopActivator activate(this);

    size_t count = 100;
    while(count--) {
        wxWindowList::compatibility_iterator node = wxTopLevelWindows.GetFirst();
        while (node) {
            wxTopLevelWindow* win = dynamic_cast<wxTopLevelWindow*>(node->GetData());
            if (win && win->IsActive()) {
                m_TopWnd = win;
                break;
            }
            node = node->GetNext();
        }

        IAppJob::EJobState state = disp.GetJobState(m_JobId);
        if (state != IAppJob::eRunning && state != IAppJob::eSuspended)
            return 1;

        if (m_TopWnd)
            break;

        if (Pending())
            Dispatch();
        else
            ::wxMilliSleep(10);
    }

    return 0;
}

void CAsyncCall::Execute()
{
    if( !::wxIsMainThread() ){
        m_Call.Execute();
        return;
    }

    if (fInsideAsyncCall)
        return;
    CBoolGuard _guard(fInsideAsyncCall);

    CRef<CExecJob> job = CRef<CExecJob>( new CExecJob( &m_Call ) );

    CAppJobDispatcher& disp = CAppJobDispatcher::GetInstance();
    m_JobId = disp.StartJob( *job, "ObjManagerEngineAsyncCall" );
    if( m_JobId == CAppJobDispatcher::eInvalidJobID ){
        return;
    }

    for( unsigned long misec = 0; misec < 1000; misec=misec*2+1 ){
        ::wxMilliSleep( misec ); // in case of 0 it works like Yield()

        IAppJob::EJobState state = disp.GetJobState( m_JobId );
        if( state != IAppJob::eRunning && state != IAppJob::eSuspended ){
            //LOG_POST( Trace << "UI waiting job is finished by " << misec << " ms step." );
            job->Finish();
            return;
        }
    }

    CAsyncCallEventLoop loop(m_JobId);
    if (loop.Run()) {
        job->Finish();
        return;
    }

    CAsyncCallDlg(loop.GetTopWnd(), m_Call, m_JobId).ShowModal();
    job->Finish();
}

void CAsyncCall::Execute(Call& call)
{
    CAsyncCall asyncCall(call);
    asyncCall.Execute();
}

CAsyncCall::~CAsyncCall()
{
    if (m_JobId != CAppJobDispatcher::eInvalidJobID) {
        CAppJobDispatcher::GetInstance().DeleteJob(m_JobId);
    }
}

BEGIN_EVENT_TABLE( CAsyncCallDlg, wxDialog )
    EVT_TIMER(-1, CAsyncCallDlg::OnTimer)
    EVT_BUTTON( ID_BUTTON_CANCEL, CAsyncCallDlg::OnCancelClick)
END_EVENT_TABLE()

CAsyncCallDlg::CAsyncCallDlg(wxWindow* parent, CAsyncCall::Call& call, int jobId)
: m_Call(call), m_JobId(jobId), m_Timer(this), m_ProgressInd(), m_CancelBtn()
{
    Create(parent);
}

bool CAsyncCallDlg::Create(wxWindow* parent)
{
    long style = 0;
    parent = GetParentForModalDialog(parent, style);

    wxDialog::Create(parent, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, style);
    SetBackgroundColour(wxColor(wxT("#FFE793")));

    CreateControls();
    m_Timer.Start(300);

    if (GetSizer())
    {
        GetSizer()->SetSizeHints(this);
    }

    //Centre();

    wxRect parentRect = parent->GetScreenRect();
    wxRect rect = GetScreenRect();
    Move(parentRect.GetRight() - rect.GetWidth() - 2, parentRect.GetBottom() - rect.GetHeight() - 2);

    return true;
}

void CAsyncCallDlg::CreateControls()
{
    CAsyncCallDlg* itemDialog1 = this;
    wxBoxSizer* itemBoxSizer1 = new wxBoxSizer(wxVERTICAL);
    itemDialog1->SetSizer(itemBoxSizer1, true);
    wxStaticText* itemStaticText3 = new wxStaticText(itemDialog1, ID_STATIC_TEXT, m_Call.GetDescr(), wxDefaultPosition, wxDefaultSize, 0);
    wxFont bold(wxNORMAL_FONT->GetPointSize(), wxNORMAL_FONT->GetFamily(), wxNORMAL_FONT->GetStyle(), wxFONTWEIGHT_BOLD, wxNORMAL_FONT->GetUnderlined(), wxNORMAL_FONT->GetFaceName());
    itemStaticText3->SetFont(bold);
    itemStaticText3->SetForegroundColour(*wxBLACK);
    itemBoxSizer1->Add(itemStaticText3, 0, wxALIGN_LEFT|wxTOP|wxLEFT|wxRIGHT, 5);

    wxBoxSizer* itemBoxSizer2 = new wxBoxSizer(wxHORIZONTAL);
    itemBoxSizer1->Add(itemBoxSizer2);

    m_ProgressInd = new CIndProgressBar(itemDialog1, ID_PROGRESS_IND, wxDefaultPosition, (m_Call.IsCancalable() ? 160 : 240));
    itemBoxSizer2->Add(m_ProgressInd, 0, wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL | wxALL, 5);

    m_CancelBtn = new wxButton(itemDialog1, ID_BUTTON_CANCEL, _("Cancel"), wxDefaultPosition, wxDefaultSize);
    itemBoxSizer2->Add(m_CancelBtn, 0, wxALIGN_LEFT | wxALL, 5);

    if (m_Call.IsCancalable())
        m_CancelBtn->SetFocus();
    else
        m_CancelBtn->Show(false);
}

void CAsyncCallDlg::OnTimer(wxTimerEvent&)
{
    if (m_Call.IsCancalable() && !m_CancelBtn->IsShown()) {
        wxPoint posOld = GetPosition() + GetSize();

        int w, h;
        m_ProgressInd->GetSize(&w, &h);
        wxSize size(160, h);
        m_ProgressInd->SetMinSize(size);
        m_ProgressInd->SetSize(size);

        m_CancelBtn->Show();
        m_CancelBtn->SetFocus();
        Fit();
        Move(posOld - GetSize());
    }

    CAppJobDispatcher& disp = CAppJobDispatcher::GetInstance();
    IAppJob::EJobState state = disp.GetJobState(m_JobId);
    if (state != IAppJob::eRunning && state != IAppJob::eSuspended) {
        m_Timer.Stop();
        EndModal(wxID_CANCEL);
    }
}

void CAsyncCallDlg::OnCancelClick(wxCommandEvent&)
{
    CAppJobDispatcher& disp = CAppJobDispatcher::GetInstance();
    disp.CancelJob(m_JobId);
    FindWindow(ID_BUTTON_CANCEL)->Enable(false);
}

END_NCBI_SCOPE
