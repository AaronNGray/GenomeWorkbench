/*  $Id: ui_project_load_manager.cpp 39528 2017-10-05 15:27:37Z katargir $
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

#include <gui/core/ui_project_load_manager.hpp>

#include <gui/core/project_load_option_panel.hpp>
#include <gui/core/project_task.hpp>
#include <gui/core/project_service.hpp>

#include <gui/widgets/wx/message_box.hpp>

#include <wx/filename.h>
#include <wx/msgdlg.h>

BEGIN_NCBI_SCOPE

///////////////////////////////////////////////////////////////////////////////
/// CProjectLoadManager
CProjectLoadManager::CProjectLoadManager()
:   m_Descriptor("Projects", ""),
    m_SrvLocator(NULL),
    m_ParentWindow(NULL),
    m_State(eInvalid),
    m_OptionPanel(NULL)
{
    m_Descriptor.SetLogEvent("loaders");
}


void CProjectLoadManager::SetServiceLocator(IServiceLocator* srv_locator)
{
    m_SrvLocator = srv_locator;
}


void CProjectLoadManager::SetParentWindow(wxWindow* parent)
{
    m_ParentWindow = parent;
}


const IUIObject& CProjectLoadManager::GetDescriptor() const
{
    m_Descriptor.SetLabel("Project");
    return m_Descriptor;
}


void CProjectLoadManager::InitUI()
{
    m_State = eSelectFile;
}


void CProjectLoadManager::CleanUI()
{
    m_State = eInvalid;
    m_OptionPanel = NULL; // window is destroyed by the system
    m_Descriptor.SetLabel("Projects");
}


wxPanel* CProjectLoadManager::GetCurrentPanel()
{
    if(m_State == eSelectFile)   {
        if(m_OptionPanel == NULL)   {
            CIRef<CProjectService> srv =
                m_SrvLocator->GetServiceByType<CProjectService>();

            _ASSERT(srv);

            const CProjectService::TMRUPathList&
                mru = srv->GetProjectWorkspaceMRUList();
            m_OptionPanel = new CProjectLoadOptionPanel(m_ParentWindow);
            m_OptionPanel->SetMRU(mru);
        }
        return m_OptionPanel;
    }
    return NULL;
}


bool CProjectLoadManager::CanDo(EAction action)
{
    switch(m_State) {
    case eSelectFile:
        //return false;// to disable Finish
        return action == eNext;
    case eCompleted:
        return false; // nothing left to do
    default:
        _ASSERT(false);
        return false;
    }
}


bool CProjectLoadManager::IsFinalState()
{
    return m_State == eSelectFile;
}


bool CProjectLoadManager::IsCompletedState()
{
    return m_State == eCompleted;
}


bool CProjectLoadManager::DoTransition(EAction action)
{
    if(m_State == eSelectFile  &&  action == eNext)    {
        if(m_OptionPanel->OnFinish())   {
            m_State = eCompleted;
            return true;
        }
        return false;
    }
    _ASSERT(false);
    return false;
}

IAppTask* CProjectLoadManager::GetTask()
{
    vector<wxString> filenames;
    m_OptionPanel->GetProjectFilenames(filenames);
    CProjectTask* task = new CProjectTask(m_SrvLocator, filenames);
    return task;
}


///////////////////////////////////////////////////////////////////////////////
/// CProjectFormatLoadManager
CProjectFormatLoadManager::CProjectFormatLoadManager()
:   m_Descr("Project", ""),
    m_SrvLocator(NULL)
{
    m_Descr.SetLogEvent("loaders");
}


void CProjectFormatLoadManager::SetServiceLocator(IServiceLocator* srv_locator)
{
    m_SrvLocator = srv_locator;
}


void CProjectFormatLoadManager::SetParentWindow(wxWindow* parent)
{
    m_ParentWindow = parent;
}


const IUIObject&  CProjectFormatLoadManager::GetDescriptor() const
{
    return m_Descr;
}


void CProjectFormatLoadManager::InitUI()
{
}


void CProjectFormatLoadManager::CleanUI()
{
}


wxPanel* CProjectFormatLoadManager::GetCurrentPanel()
{
    // we do not have any
    return NULL;
}


bool CProjectFormatLoadManager::CanDo(EAction /*action*/)
{
    return false;
}


bool CProjectFormatLoadManager::IsFinalState()
{
    return true;
}


bool CProjectFormatLoadManager::IsCompletedState()
{
    return true;
}


bool CProjectFormatLoadManager::DoTransition(EAction /*action*/)
{
    _ASSERT(false); // we do not have panels and transitions
    return false;
}


IAppTask* CProjectFormatLoadManager::GetTask()
{
    CProjectTask* task = new CProjectTask(m_SrvLocator, m_Filenames);
    return task;
}


wxString CProjectFormatLoadManager::GetFormatWildcard()
{
    return wxT("*.gbp");
}


bool CProjectFormatLoadManager::ValidateFilenames(const vector<wxString>& filenames)
{
    size_t n = filenames.size();
    if(n > 0)   {
        for(  size_t i =0;  i < n;  i++)    {
            const wxString& filename = filenames[i];
            if( ! RecognizeFormat(filename))    {
                wxString err_msg = wxT("File \"") + filename + wxT("\" is not a project and cannot be loaded!");
                wxMessageBox(err_msg, wxT("Error"), wxOK | wxICON_ERROR);
                return false; // found bad file
            }
        }
        return true; // did not find any bad files
    }
    _ASSERT(false); // should not be there
    return false;
}


void CProjectFormatLoadManager::SetFilenames(const vector<wxString>& filenames)
{
    m_Filenames = filenames;
}

void CProjectFormatLoadManager::GetFilenames(vector<wxString>& filenames) const
{
    filenames = m_Filenames;
}

bool CProjectFormatLoadManager::IsInitialState()
{
    return true;
}


bool CProjectFormatLoadManager::RecognizeFormat(const wxString& filename)
{
    wxString ext;
    wxFileName::SplitPath(filename, 0, 0, &ext);
    return ext == wxT("gbp");
}

bool CProjectFormatLoadManager::RecognizeFormat(CFormatGuess::EFormat fmt) 
{ 
    return fmt == CFormatGuess::eBinaryASN || fmt == CFormatGuess::eTextASN;
}

string CProjectFormatLoadManager::GetExtensionIdentifier() const
{
    static string sid("project_format_load_manager");
    return sid;
}


string CProjectFormatLoadManager::GetExtensionLabel() const
{
    static string slabel("Project Format Load Manager");
    return slabel;
}

END_NCBI_SCOPE
