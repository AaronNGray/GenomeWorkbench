#ifndef GUI_WIDGETS_WX___JOB_SYNC_CALL__HPP
#define GUI_WIDGETS_WX___JOB_SYNC_CALL__HPP

/*  $Id: async_call.hpp 40186 2018-01-04 17:40:38Z katargir $
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

#include <gui/gui.hpp>

#include <corelib/ncbiobj.hpp>
#include <corelib/ncbicntr.hpp>

#include <util/icanceled.hpp>

#include <gui/utils/execute_unit.hpp>
#include <gui/utils/job_future.hpp>

// This header must (at least indirectly) precede any wxWidgets headers.
#include <gui/widgets/wx/fixed_base.hpp>

#include <wx/string.h>


BEGIN_NCBI_SCOPE

class IAppJob;

/// Class for GUI asyncronous execition. 
/// 
/// If async background call takes too much time GBench GUI automatically shows 
/// a window to inform the user to wait
///
class NCBI_GUIWIDGETS_WX_EXPORT CAsyncCall
{
public:
    class Call : public ICanceled
    {
    public:
        Call(const wxString& descr) : m_Descr(descr) {}
        Call() : m_Descr(wxT("Accessing network...")) {}
        virtual ~Call() {}

        wxString GetDescr() const { return m_Descr; }

        virtual void Execute() = 0;

        void RequestCancel() { m_Canceled.Set(1); }
        bool IsCancalable() const { return (m_Cancelable.Get() != 0); }

        // ICanceled implementation
        virtual bool IsCanceled() const {
            m_Cancelable.Set(1);
            return (m_Canceled.Get() != 0);
        }

    private:
        wxString m_Descr;

        CAtomicCounter_WithAutoInit m_Canceled;
        mutable CAtomicCounter_WithAutoInit m_Cancelable;
    };
  
    CAsyncCall(Call& call);
    ~CAsyncCall();

    void Execute();
    static void Execute(Call& call);

    static bool InsideAsyncCall();

private:
    Call& m_Call;
    wxString m_Descr;
    int m_JobId;
};

template<class _Fty, typename _Rty>
class CLocalAsyncCallBind : public CObject, public CAsyncCall::Call
{
public:
    CLocalAsyncCallBind(_Fty&& func, const wxString& msg)
        : CAsyncCall::Call(msg), m_Func(func) {}
    virtual void Execute() { m_Ret = m_Func(*this); }
    _Rty GetResult() { return m_Ret; }
private:
    _Fty& m_Func;
    _Rty  m_Ret;
};

template<class _Fty>
class CLocalAsyncCallBind<_Fty, void> : public CObject, public CAsyncCall::Call
{
public:
    CLocalAsyncCallBind(_Fty&& func, const wxString& msg)
        : CAsyncCall::Call(msg), m_Func(func) {}
    virtual void Execute() { m_Func(*this); }
    void GetResult() { return; }
private:
    _Fty& m_Func;
};

template<class _Fty>
typename std::result_of<_Fty(ICanceled&)>::type GUI_AsyncExec(_Fty&& _Fnarg,
    const wxString& msg = wxT("Accessing network..."))
{
    typedef typename std::result_of<_Fty(ICanceled&)>::type _Rty;
    typedef CLocalAsyncCallBind<_Fty, _Rty> _Bty;
    CRef<_Bty> f(new _Bty(std::forward<_Fty>(_Fnarg), msg));
    CAsyncCall asyncCall(*f);
    asyncCall.Execute();
    return f->GetResult();
}

NCBI_GUIWIDGETS_WX_EXPORT void GUI_AsyncJoinFuture(const job_future_base& future, const string& msg);

NCBI_GUIWIDGETS_WX_EXPORT bool GUI_AsyncExecUnit(IExecuteUnit& exec_unit, const wxString& msg);

END_NCBI_SCOPE

#endif  // GUI_WIDGETS_WX___JOB_SYNC_CALL__HPP
