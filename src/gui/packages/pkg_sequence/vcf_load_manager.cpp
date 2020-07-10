/*  $Id: vcf_load_manager.cpp 38477 2017-05-15 21:10:59Z evgeniev $
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

#include "vcf_load_manager.hpp"
#include <gui/widgets/loaders/bed_params_panel.hpp>

#include <gui/widgets/loaders/vcf_object_loader.hpp>

#include <gui/widgets/wx/wx_utils.hpp>
#include <gui/widgets/wx/file_extensions.hpp>

#include <wx/filename.h>

BEGIN_NCBI_SCOPE

wxPanel* CVCFLoaderManager::CPage::GetPanel()
{
    return m_Manager.x_GetParamsPanel();
}

///////////////////////////////////////////////////////////////////////////////
/// CVCFLoaderManager
CVCFLoaderManager::CVCFLoaderManager()
    :   m_Descr("VCF (Variant Call Format) files", "")
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

string CVCFLoaderManager::GetLabel() const
{
	return m_Descr.GetLabel();
}

wxString CVCFLoaderManager::GetFormatWildcard() const
{
    return CFileExtensions::GetDialogFilter(CFileExtensions::kVCF) + wxT("|") +
           CFileExtensions::GetDialogFilter(CFileExtensions::kAllFiles);
}

//
// IUILoadManager implementation
//

void CVCFLoaderManager::SetServiceLocator(IServiceLocator* srv_locator)
{
    m_SrvLocator = srv_locator;
}


void CVCFLoaderManager::SetParentWindow(wxWindow* parent)
{
    m_ParentWindow = parent;
}


const IUIObject& CVCFLoaderManager::GetDescriptor() const
{
    return m_Descr;
}


void CVCFLoaderManager::InitUI()
{
    m_State = eParams;
}


void CVCFLoaderManager::CleanUI()
{
    m_State = eInvalid;
    m_ParamsPanel = NULL; // window is destroyed by the system
}

wxPanel* CVCFLoaderManager::GetCurrentPanel()
{
    return (m_State == eParams) ? x_GetParamsPanel() : NULL;
}

CBedParamsPanel* CVCFLoaderManager::x_GetParamsPanel()
{
    if (m_ParamsPanel == NULL) {
        m_ParamsPanel = new CBedParamsPanel(m_ParentWindow);
        m_ParamsPanel->SetData(m_Params);
        m_ParamsPanel->TransferDataToWindow();
    }
    return m_ParamsPanel;
}

bool CVCFLoaderManager::CanDo(EAction action)
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


bool CVCFLoaderManager::IsFinalState()
{
    return m_State == eParams;
}


bool CVCFLoaderManager::IsCompletedState()
{
    return m_State == eCompleted; // does not matter
}


bool CVCFLoaderManager::DoTransition(EAction action)
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


IAppTask* CVCFLoaderManager::GetTask()
{
    return nullptr;
}

IExecuteUnit* CVCFLoaderManager::GetExecuteUnit()
{
    return new CVcfObjectLoader(m_Params, m_FileNames);
}

wxString CVCFLoaderManager::GetFormatWildcard()
{
    return CFileExtensions::GetDialogFilter(CFileExtensions::kVCF) + wxT("|") +
           CFileExtensions::GetDialogFilter(CFileExtensions::kAllFiles);
}


bool CVCFLoaderManager::ValidateFilenames(const vector<wxString>&)
{
    // not implemented
    return true;
}


void CVCFLoaderManager::SetFilenames(const vector<wxString>& filenames)
{
    m_FileNames = filenames;
}

void CVCFLoaderManager::GetFilenames(vector<wxString>& filenames) const
{
    filenames = m_FileNames;
}

bool CVCFLoaderManager::IsInitialState()
{
    return m_State == eParams;
}


bool CVCFLoaderManager::RecognizeFormat(const wxString& filename)
{
    wxString ext;
    wxFileName::SplitPath(filename, 0, 0, &ext);
    return CFileExtensions::RecognizeExtension(CFileExtensions::kVCF, ext);
}

bool CVCFLoaderManager::RecognizeFormat(CFormatGuess::EFormat fmt)
{
    return fmt == CFormatGuess::eVcf;
}

string CVCFLoaderManager::GetExtensionIdentifier() const
{
    static string sid("vcf_format_load_manager");
    return sid;
}


string CVCFLoaderManager::GetExtensionLabel() const
{
    static string slabel("VCF (Variant Call Format) Format Load Manager");
    return slabel;
}


void CVCFLoaderManager::SetRegistryPath(const string& path)
{
    m_RegPath = path; // store for later use
    m_Params.SetRegistryPath(m_RegPath + ".VcfParams");
}

void CVCFLoaderManager::SaveSettings() const
{
    m_Params.SaveSettings();
}


void CVCFLoaderManager::LoadSettings()
{
    m_Params.LoadSettings();
}

END_NCBI_SCOPE
