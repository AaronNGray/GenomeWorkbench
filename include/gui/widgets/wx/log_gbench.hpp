#ifndef GUI_WIDGETS_WX___LOG_GBENCH__HPP
#define GUI_WIDGETS_WX___LOG_GBENCH__HPP

/*  $Id: log_gbench.hpp 39054 2017-07-21 20:24:16Z katargir $
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
#include <corelib/ncbidiag.hpp>
#include <corelib/ncbitime.hpp>
#include <gui/utils/command.hpp>

// This header must (at least indirectly) precede any wxWidgets headers.
#include <gui/widgets/wx/fixed_base.hpp>

#include <wx/log.h>

#include <deque>

BEGIN_NCBI_SCOPE


///////////////////////////////////////////////////////////////////////////////
/// CwxLogDiagHandler - provides a centralized logging facility that
/// integrates both with C++ Toolkit (CDiagHandler) and wxWidgets (wxLog).

class NCBI_GUIWIDGETS_WX_EXPORT CwxLogDiagHandler :
    public wxLog,
    public CDiagHandler,
    public CEventHandler
{
public:
    static CwxLogDiagHandler* GetInstance();

    virtual void Flush();

    enum
    {
        kMaxBuffered = 2000
    };

    struct SMessage {
        SMessage() : wxMsg(false), severity(eDiag_Info), time(CTime::eCurrent) {}
        bool wxMsg;
        EDiagSev severity;
        CTime time;
        string message;
        string source;
    };

    typedef deque<SMessage> TMessages;

    /// an event for child notification
    class CDiagEvent : public CEvent
    {
    public:
        enum {
            eNewItem = 1
        };

        CDiagEvent(size_t total, size_t buffered)
            : CEvent(eNewItem)
            , m_Total(total), m_Buffered(buffered)
        {
        }

        size_t GetTotal() const { return m_Total; }
        size_t GetBuffered() const { return m_Buffered; }

    private:
        size_t m_Total;
        size_t m_Buffered;
    };

    void GetMsgCount(size_t& total, size_t& buffered);
    bool GetMessage(size_t index, SMessage& msg, size_t& total) const;

protected:
    virtual void DoLogRecord(wxLogLevel level,
                             const wxString& msg,
                             const wxLogRecordInfo& info);

    /// post a message (CDiagHandler interface implementation)
    virtual void Post(const SDiagMessage& msg);

	// implement string modification on-the-fly
	void TweakContents( string& line );

    DECLARE_NO_COPY_CLASS(CwxLogDiagHandler)

private:
    CwxLogDiagHandler();
    ~CwxLogDiagHandler();

    CEvent* x_AddMessage(const SMessage& msg);

private:
    CDiagHandler* m_OrigHandler;

    /// stream for logging all actions
    CNcbiOfstream m_LogStream;

    TMessages m_Buffer;
    size_t m_Total;
};

END_NCBI_SCOPE

#endif // GUI_WIDGETS_WX___LOG_GBENCH__HPP
