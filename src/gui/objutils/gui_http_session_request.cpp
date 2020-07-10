/*  $Id: gui_http_session_request.cpp 38013 2017-03-14 21:20:31Z katargir $
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
 * Authors:  Vladislav Evgeniev, Roman Katargin
 *
 * File Description:
 *
 */

#include <ncbi_pch.hpp>

#include <gui/objutils/gui_http_session_request.hpp>
#include <gui/objutils/registry.hpp>

BEGIN_NCBI_SCOPE

CGuiHttpSessionRequest::CGuiHttpSessionRequest(const string& url)
{
    int response_timeout = CGuiRegistry::GetInstance().GetInt("GBENCH.System.SeqConfigTimeout", 40);
    CRef<CHttpSession> session(new CHttpSession);
    CHttpRequest request = session->NewRequest(url, CHttpSession::ePost);
    request.SetDeadline(CTimeout(response_timeout)).SetRetryProcessing(eOn);
    m_Response.Reset(new CHttpResponse(request.Execute()));
}

CNcbiIstream& CGuiHttpSessionRequest::GetResponseStream()
{
    if (!m_Response)
        NCBI_THROW(CException, eUnknown, "CGuiHttpSessionRequest: response is unavailable");

    if (!m_Response->ContentStream().good())
        NCBI_THROW(CException, eUnknown, "CGuiHttpSessionRequest: response content stream is invalid");

    return m_Response->ContentStream();
}

END_NCBI_SCOPE
