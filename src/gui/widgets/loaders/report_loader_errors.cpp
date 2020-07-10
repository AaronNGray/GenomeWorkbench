/*  $Id: report_loader_errors.cpp 44093 2019-10-24 20:19:01Z filippov $
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
*  and reliability of the software and data,  the NLM and the U.S.
*  Government do not and cannot warrant the performance or results that
*  may be obtained by using this software or data. The NLM and the U.S.
*  Government disclaim all warranties,  express or implied,  including
*  warranties of performance,  merchantability or fitness for any particular
*  purpose.
*
*  Please cite the author in any work or product based on this material.
*
* ===========================================================================
*
* Authors:  Roman Katargin
*/


#include <ncbi_pch.hpp>

#include <gui/widgets/loaders/report_loader_errors.hpp>

#include <gui/widgets/loaders/job_results_dlg.hpp>

#include <gui/widgets/wx/wx_utils.hpp>

#include <wx/txtstrm.h>
#include <wx/filename.h>

BEGIN_NCBI_SCOPE
USING_SCOPE(objects);

void CReportLoaderErrors::x_UpdateHTMLResults(const wxString& object, objects::IMessageListener* errCont, 
    const string& exception, const string& error_msg, const wxString& objectName)
{
   if ((!errCont || errCont->Count() == 0) && exception.empty() && error_msg.empty())
       return;

    wxTextOutputStream os(m_Strstrm);
    os << wxT("<h5>") << objectName << wxT(" ") << object << wxT("</h5>");

    if (errCont && errCont->Count() > 0) {
        for (size_t i = 0; i < errCont->Count(); ++i) {
            const ILineError& lerror = errCont->GetError(i);
            os << wxT("Line ") << lerror.Line() << wxT(". ") << wxString::FromUTF8(lerror.Message().c_str()) << wxT("<br />");
        }
    }

    if (!exception.empty()) {
        os << wxT("<font color=\"red\">") << wxString::FromUTF8(exception.c_str()) << wxT("</font><br />");
    }

    if (!error_msg.empty()) {
        os << wxT("<font color=\"red\">") << wxString::FromUTF8(error_msg.c_str()) << wxT("</font><br />");
    }
}

void CReportLoaderErrors::x_ShowErrorsDlg(const wxString& title)
{
    wxString html = m_Strstrm.GetString();
    if (!html.IsEmpty()) {
        CJobResultsDlg dlg;
        dlg.SetHTML(html);
        dlg.SetWorkDir(m_WorkDir);
        dlg.Create(NULL, wxID_ANY, title);
        dlg.ShowModal();
    }
}

END_NCBI_SCOPE
