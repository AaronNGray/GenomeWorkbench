/*  $Id: psl_load_manager.cpp 44439 2019-12-18 19:51:52Z katargir $
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
*/

#include <ncbi_pch.hpp>

#include "psl_load_manager.hpp"
#include <gui/widgets/loaders/psl_params_panel.hpp>

#include <gui/widgets/loaders/psl_object_loader.hpp>

#include <gui/widgets/wx/file_extensions.hpp>
#include <gui/widgets/wx/wx_utils.hpp>

#include <wx/filename.h>

BEGIN_NCBI_SCOPE

wxPanel* CPslLoadManager::CPage::GetPanel()
{
    return m_Manager.x_GetParamsPanel();
}


///////////////////////////////////////////////////////////////////////////////
/// CPslLoadManager
CPslLoadManager::CPslLoadManager()
    :  m_Descr("", "")
     , m_SrvLocator(NULL)
     , m_ParentWindow(NULL)
     , m_State(eInvalid)
     , m_ParamsPanel(NULL)
     , m_OptionsPage(*this)
{
    m_Descr.Init(ToStdString(CFileExtensions::GetLabel(CFileExtensions::kPSL)), "");
    m_Descr.SetLogEvent("loaders");
}

//
// IFileLoadPanelClient implementation
//

string CPslLoadManager::GetLabel() const
{
	return m_Descr.GetLabel();
}

wxString CPslLoadManager::GetFormatWildcard() const
{
    return CFileExtensions::GetDialogFilter(CFileExtensions::kPSL) + wxT("|") +
           CFileExtensions::GetDialogFilter(CFileExtensions::kAllFiles);
}

//
// IUILoadManager implementation
//

void CPslLoadManager::SetServiceLocator(IServiceLocator* srv_locator)
{
    m_SrvLocator = srv_locator;
}


void CPslLoadManager::SetParentWindow(wxWindow* parent)
{
    m_ParentWindow = parent;
}


const IUIObject& CPslLoadManager::GetDescriptor() const
{
    return m_Descr;
}


void CPslLoadManager::InitUI()
{
    m_State = eParams;
}


void CPslLoadManager::CleanUI()
{
    m_State = eInvalid;
    m_ParamsPanel = NULL; // window is destroyed by the system
}


wxPanel* CPslLoadManager::GetCurrentPanel()
{
    return (m_State == eParams) ? x_GetParamsPanel() : NULL;
}

CPslParamsPanel* CPslLoadManager::x_GetParamsPanel()
{
    if (m_ParamsPanel == NULL) {
        m_ParamsPanel = new CPslParamsPanel(m_ParentWindow);
        m_ParamsPanel->SetData(m_Params);
        m_ParamsPanel->TransferDataToWindow();
    }
    return m_ParamsPanel;
}


bool CPslLoadManager::CanDo(EAction action)
{
    switch(m_State) {
    case eParams:
        return action == eNext;
    case eCompleted:
        return action == eBack;
    default:
        _ASSERT(false);
        return false;
    }
}


bool CPslLoadManager::IsFinalState()
{
    return m_State == eParams;
}


bool CPslLoadManager::IsCompletedState()
{
    return m_State == eCompleted; // does not matter
}


bool CPslLoadManager::DoTransition(EAction action)
{
    if(m_State == eParams  &&  action == eNext)    {
        if(m_ParamsPanel->TransferDataFromWindow()) {
            //TODO validate
            m_Params = m_ParamsPanel->GetData();
            m_State = eCompleted;
            return true;
        }
        return false;
    } else if(m_State == eCompleted  &&  action == eBack)   {
        m_State = eParams;
        return true;
    }
    _ASSERT(false);
    return false;
}


IAppTask* CPslLoadManager::GetTask()
{
    return nullptr;
}

IExecuteUnit* CPslLoadManager::GetExecuteUnit()
{
    return new CPslObjectLoader(m_Params, m_FileNames);
}


wxString CPslLoadManager::GetFormatWildcard()
{
    return CFileExtensions::GetDialogFilter(CFileExtensions::kPSL) + wxT("|") +
           CFileExtensions::GetDialogFilter(CFileExtensions::kAllFiles);
}


bool CPslLoadManager::ValidateFilenames(const vector<wxString>& /*filenames*/)
{
    // not implemented
    return true;
}


void CPslLoadManager::SetFilenames(const vector<wxString>& filenames)
{
    m_FileNames = filenames;
}

void CPslLoadManager::GetFilenames(vector<wxString>& filenames) const
{
    filenames = m_FileNames;
}

bool CPslLoadManager::IsInitialState()
{
    return m_State == eParams;
}


bool CPslLoadManager::RecognizeFormat(const wxString& filename)
{
    wxString ext;
    wxFileName::SplitPath(filename, 0, 0, &ext);
    return CFileExtensions::RecognizeExtension(CFileExtensions::kPSL, ext);
}

bool CPslLoadManager::RecognizeFormat(CFormatGuess::EFormat fmt)
{
    return fmt == CFormatGuess::ePsl;
}

string CPslLoadManager::GetExtensionIdentifier() const
{
    return "psl_format_load_manager";
}


string CPslLoadManager::GetExtensionLabel() const
{
    static string slabel("PSL Format Load Manager");
    return slabel;
}

void CPslLoadManager::SetRegistryPath(const string& path)
{
    m_RegPath = path;
    m_Params.SetRegistryPath(m_RegPath + ".PslParams");
}

void CPslLoadManager::SaveSettings() const
{
    m_Params.SaveSettings();
}


void CPslLoadManager::LoadSettings()
{
    m_Params.LoadSettings();
}

END_NCBI_SCOPE
