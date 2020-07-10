#ifndef GUI_WIDGETS___REPORT_LOADET_ERRORS__HPP
#define GUI_WIDGETS___REPORT_LOADET_ERRORS__HPP

/*  $Id: report_loader_errors.hpp 44093 2019-10-24 20:19:01Z filippov $
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

#include <corelib/ncbistd.hpp>
#include <objtools/readers/message_listener.hpp>
#include <gui/gui_export.h>
#include <wx/sstream.h>

BEGIN_NCBI_SCOPE


///////////////////////////////////////////////////////////////////////////////
/// CReportLoaderErrors
class NCBI_GUIWIDGETS_LOADERS_EXPORT CReportLoaderErrors
{
public:
    CReportLoaderErrors() {}
    void SetWorkDir(const wxString& workDir) {m_WorkDir = workDir;}
protected:
    void    x_UpdateHTMLResults(const wxString& object, objects::IMessageListener* errCont, const string& exception = "", const string& error_msg = "", const wxString& objectName = wxT("File:"));
    void    x_ShowErrorsDlg(const wxString& title);
private:
    wxStringOutputStream m_Strstrm;
    wxString m_WorkDir;
};


///////////////////////////////////////////////////////////////////////////////
/// CErrorContainer
class CErrorContainer : public objects::CMessageListenerLenient
{
public:
    CErrorContainer(size_t maxErrors) : m_MaxErrors(maxErrors) {}
    virtual bool PutError(const objects::ILineError& err)
    {
        if (Count() < m_MaxErrors)
            objects::CMessageListenerLenient::PutError(err);
        return true;
    }
private:
    const size_t m_MaxErrors;
};

/* @} */

END_NCBI_SCOPE


#endif // GUI_WIDGETS___REPORT_LOADET_ERRORS__HPP
