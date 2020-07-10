/*  $Id: log_gbench.cpp 39928 2017-11-27 21:05:12Z katargir $
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

#include <ncbi_pch.hpp>

#include <gui/widgets/wx/log_gbench.hpp>
#include <gui/widgets/wx/sys_path.hpp>

BEGIN_NCBI_SCOPE

DEFINE_STATIC_MUTEX(s_LogGbenchMutex);

static CwxLogDiagHandler* s_Instance = NULL;
static bool s_WasOpened = false;

CwxLogDiagHandler* CwxLogDiagHandler::GetInstance()
{
    CMutexGuard LOCK( s_LogGbenchMutex );
    if( !s_Instance ){
        s_Instance = new CwxLogDiagHandler();
    }

    return s_Instance;
}


void CwxLogDiagHandler::Flush()
{
    if (s_WasOpened)
        m_LogStream.flush();

    wxLog::Flush();
}


CwxLogDiagHandler::CwxLogDiagHandler()
:   m_OrigHandler(), m_Total()
{
    m_OrigHandler = GetDiagHandler(true);

    // set this hanlder as the default
    SetDiagHandler(this, false);

    wxString path = CSysPath::ResolvePath( wxT("<home>/gblog.log") );
    if( !s_WasOpened ){
        m_LogStream.open( path.fn_str(), ofstream::out );

        s_WasOpened = true;
    } else {
        m_LogStream.open( path.fn_str(), ofstream::out | ofstream::app );
    }
}

CwxLogDiagHandler::~CwxLogDiagHandler()
{
    CMutexGuard LOCK(s_LogGbenchMutex);
    SetDiagHandler(m_OrigHandler);

    if( m_LogStream.is_open() ){
        m_LogStream.close();
    }
    s_Instance = NULL;
}

void CwxLogDiagHandler::GetMsgCount(size_t& total, size_t& buffered)
{
    CMutexGuard LOCK(s_LogGbenchMutex);
    total = m_Total;
    buffered = m_Buffer.size();
}

bool CwxLogDiagHandler::GetMessage(size_t index, SMessage& msg, size_t& total) const
{
    CMutexGuard LOCK(s_LogGbenchMutex);

    total = m_Total;

    if (index < m_Total - m_Buffer.size() || index >= m_Total)
        return false;

    msg = m_Buffer[index - (m_Total - m_Buffer.size())];
    return true;

}

void CwxLogDiagHandler::TweakContents( string& line )
{
	static const char* auth_match = "Authorization";
	static const size_t auth_match_len = strlen( auth_match );

	size_t match_ix = line.find( auth_match );
	if( match_ix == string::npos ) return;

	size_t eoln_ix = line.find_first_of( "\r\n", match_ix );
	if( eoln_ix == string::npos ){
		eoln_ix = line.size();
	}

	match_ix += auth_match_len + 2; // ": "
	if( eoln_ix > match_ix ){
		line.replace( match_ix, eoln_ix - match_ix, eoln_ix - match_ix, '*' );
	}
}


void CwxLogDiagHandler::Post(const SDiagMessage& msg)
{
    CMutexGuard LOCK(s_LogGbenchMutex);

    {{
        CNcbiOstrstream os;
        os << msg;
        string text = CNcbiOstrstreamToString(os);
        TweakContents(text);
        m_LogStream << text;
    }}

    string logline;
    logline.assign(msg.m_Buffer, msg.m_Buffer + msg.m_BufferLen);
    logline.erase(std::remove(logline.begin(), logline.end(), '\r'), logline.end());

	if( m_OrigHandler ){
		SDiagMessage* patched_msg = const_cast<SDiagMessage*>(&msg);
		
		const char* saved_buffer = patched_msg->m_Buffer;
		size_t saved_buffer_len = patched_msg->m_BufferLen;

		patched_msg->m_Buffer = logline.c_str();
		patched_msg->m_BufferLen = logline.size();
		
        m_OrigHandler->Post( *patched_msg );

		patched_msg->m_Buffer = saved_buffer;
		patched_msg->m_BufferLen = saved_buffer_len;
	}

    SMessage message;
    message.severity = msg.m_Severity;
    message.time     = msg.GetTime();

    // add infomation about source of the message to facilitate debugging
    // maybe we make optional, based on _DEBUG build or NCBI_WXLOG_VERBOSE_DEBUGGING
    //

    message.source = msg.m_File;
    if (!message.source.empty()) {
        // trim the base path
        size_t pos = message.source.find("src");
        if (pos!=string::npos) {
            message.source = message.source.substr(pos);
        }
        else {
            size_t pos = message.source.find("include");
            if (pos!=string::npos) {
                message.source = message.source.substr(pos);
            }
        }
        message.source += ":";
        message.source += NStr::NumericToString(msg.m_Line);
    }

    message.message = logline;

    CRef<CEvent> ev(x_AddMessage(message));
    LOCK.Release();
    if (ev) CEventHandler::Post(ev);
}

void CwxLogDiagHandler::DoLogRecord(wxLogLevel level,
                                    const wxString& msg,
                                    const wxLogRecordInfo& info)
{
    CMutexGuard LOCK(s_LogGbenchMutex);

    SMessage message;
    message.severity = eDiag_Info;
    message.message = string(msg.ToUTF8());
    CTime time(info.timestamp);
    message.time = time.ToLocalTime();
    message.wxMsg = true;

    switch (level) {
    case wxLOG_Status:
    case wxLOG_Progress:
        return;

    case wxLOG_Debug:
        message.severity =  eDiag_Trace;
        m_LogStream << "(Wx)" << message.message << endl;
        break;
    case wxLOG_Trace:
        message.severity =  eDiag_Trace;
        m_LogStream << "(Wx)" << message.message << endl;
        break;
    case wxLOG_Message:
        message.severity = eDiag_Info;
        m_LogStream << "(Wx)" << message.message << endl;
        break;
    case wxLOG_Warning:
        message.severity = eDiag_Warning;
        m_LogStream << "(Wx)" << message.message << endl;
        break;
    case wxLOG_Error:
        message.severity = eDiag_Error;
        m_LogStream << message.time.AsString("M/D/y h:m:s ") << "Error(Wx): " << message.message << endl;
        break;
    case wxLOG_FatalError:
        message.severity = eDiag_Fatal;
        m_LogStream << message.time.AsString("M/D/y h:m:s ") << "Fatal(Wx): " << message.message << endl;
        break;
    }

    CRef<CEvent> ev(x_AddMessage(message));
    LOCK.Release();
    if (ev) CEventHandler::Post(ev);
}

CEvent* CwxLogDiagHandler::x_AddMessage(const SMessage& msg)
{
    if (m_Buffer.size() == kMaxBuffered)
        m_Buffer.pop_front();
    m_Buffer.push_back(msg);
    ++m_Total;

    if(GetListeners())
        return new CDiagEvent(m_Total, m_Buffer.size());

    return 0;
}

END_NCBI_SCOPE
