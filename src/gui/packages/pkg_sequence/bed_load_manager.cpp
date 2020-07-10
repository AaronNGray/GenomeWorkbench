/*  $Id: bed_load_manager.cpp 38522 2017-05-22 18:19:49Z evgeniev $
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

#include "bed_load_manager.hpp"
#include <gui/widgets/loaders/bed_params_panel.hpp>

#include <gui/widgets/loaders/bed_object_loader.hpp>

#include <gui/widgets/wx/file_extensions.hpp>

#include <wx/filename.h>

BEGIN_NCBI_SCOPE

wxPanel* CBedLoadManager::CPage::GetPanel()
{
    return m_Manager.x_GetParamsPanel();
}

///////////////////////////////////////////////////////////////////////////////
/// CBedLoadManager
CBedLoadManager::CBedLoadManager()
    : m_Descr("BED files", "")
     , m_SrvLocator(NULL)
     , m_ParentWindow(NULL)
     , m_State(eInvalid)
     , m_ParamsPanel(NULL)
     , m_OptionsPage(*this)
{
    m_Descr.SetLogEvent("loaders");
}

//
// IFileLoadPanelClient implementation
//

string CBedLoadManager::GetLabel() const
{
	return m_Descr.GetLabel();
}

wxString CBedLoadManager::GetFormatWildcard() const
{
    return CFileExtensions::GetDialogFilter(CFileExtensions::kBED) + wxT("|") +
           CFileExtensions::GetDialogFilter(CFileExtensions::kAllFiles);
}

//
// IUILoadManager implementation
//

void CBedLoadManager::SetServiceLocator(IServiceLocator* srv_locator)
{
    m_SrvLocator = srv_locator;
}


void CBedLoadManager::SetParentWindow(wxWindow* parent)
{
    m_ParentWindow = parent;
}


const IUIObject& CBedLoadManager::GetDescriptor() const
{
    return m_Descr;
}


void CBedLoadManager::InitUI()
{
    m_State = eParams;
}


void CBedLoadManager::CleanUI()
{
    m_State = eInvalid;
    m_ParamsPanel = NULL; // window is destroyed by the system
}


wxPanel* CBedLoadManager::GetCurrentPanel()
{
    return (m_State == eParams) ? x_GetParamsPanel() : NULL;
}

CBedParamsPanel* CBedLoadManager::x_GetParamsPanel()
{
    if (m_ParamsPanel == NULL) {
        m_ParamsPanel = new CBedParamsPanel(m_ParentWindow);
        m_ParamsPanel->SetData(m_Params);
        m_ParamsPanel->TransferDataToWindow();
    }
    return m_ParamsPanel;
}

bool CBedLoadManager::CanDo(EAction action)
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


bool CBedLoadManager::IsFinalState()
{
    return m_State == eParams;
}


bool CBedLoadManager::IsCompletedState()
{
    return m_State == eCompleted; // does not matter
}


bool CBedLoadManager::DoTransition(EAction action)
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

IAppTask* CBedLoadManager::GetTask()
{
    return nullptr;
}

IExecuteUnit* CBedLoadManager::GetExecuteUnit()
{
    return new CBedObjectLoader(m_Params, m_FileNames);
}

wxString CBedLoadManager::GetFormatWildcard()
{
    return CFileExtensions::GetDialogFilter(CFileExtensions::kBED) + wxT("|") +
           CFileExtensions::GetDialogFilter(CFileExtensions::kAllFiles);
}


bool CBedLoadManager::ValidateFilenames(const vector<wxString>& /*filenames*/)
{
    // not implemented
    return true;
}


void CBedLoadManager::SetFilenames(const vector<wxString>& filenames)
{
    m_FileNames = filenames;
}

void CBedLoadManager::GetFilenames(vector<wxString>& filenames) const
{
    filenames = m_FileNames;
}

bool CBedLoadManager::IsInitialState()
{
    return m_State == eParams;
}


bool CBedLoadManager::RecognizeFormat(const wxString& filename)
{
    wxString ext;
    wxFileName::SplitPath(filename, 0, 0, &ext);
    return ext == wxT("bed");
}

bool CBedLoadManager::RecognizeFormat(CFormatGuess::EFormat fmt)
{
    // This does not load all data for bed15 format, but it does 
    // load the fields that bed15 and bed have in common correctly
    return ( fmt == CFormatGuess::eBed || fmt == CFormatGuess::eBed15 );
}

string CBedLoadManager::GetExtensionIdentifier() const
{
    static string sid("bed_format_load_manager");
    return sid;
}


string CBedLoadManager::GetExtensionLabel() const
{
    static string slabel("BED Format Load Manager");
    return slabel;
}

void CBedLoadManager::SetRegistryPath(const string& path)
{
    m_RegPath = path; // store for later use
    m_Params.SetRegistryPath(m_RegPath + ".BedParams");
}

void CBedLoadManager::SaveSettings() const
{
    m_Params.SaveSettings();
}


void CBedLoadManager::LoadSettings()
{
    m_Params.LoadSettings();
}

END_NCBI_SCOPE
