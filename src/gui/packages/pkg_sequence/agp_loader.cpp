/*  $Id: agp_loader.cpp 38477 2017-05-15 21:10:59Z evgeniev $
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

#include "agp_loader.hpp"
#include <gui/widgets/loaders/agp_load_page.hpp>

#include <gui/widgets/loaders/agp_object_loader.hpp>

#include <gui/widgets/wx/file_extensions.hpp>
#include <wx/filename.h>

BEGIN_NCBI_SCOPE

wxPanel* CAgpLoader::CPage::GetPanel()
{
    return m_Manager.x_GetParamsPanel();
}

///////////////////////////////////////////////////////////////////////////////
/// CAgpLoader
CAgpLoader::CAgpLoader()
    : m_Descr("AGP assembly files", "")
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

string CAgpLoader::GetLabel() const
{
	return m_Descr.GetLabel();
}

wxString CAgpLoader::GetFormatWildcard() const
{
    return CFileExtensions::GetDialogFilter(CFileExtensions::kAGP) + wxT("|") +
           CFileExtensions::GetDialogFilter(CFileExtensions::kAllFiles);
}

//
// IUILoadManager implementation
//

void CAgpLoader::SetServiceLocator(IServiceLocator* srv_locator)
{
    m_SrvLocator = srv_locator;
}


void CAgpLoader::SetParentWindow(wxWindow* parent)
{
    m_ParentWindow = parent;
}


const IUIObject& CAgpLoader::GetDescriptor() const
{
    return m_Descr;
}


void CAgpLoader::InitUI()
{
    m_State = eParams;
}


void CAgpLoader::CleanUI()
{
    m_State = eInvalid;
    m_ParamsPanel = NULL; // window is destroyed by the system
}


wxPanel* CAgpLoader::GetCurrentPanel()
{
    return (m_State == eParams) ? x_GetParamsPanel() : NULL;
}

CAgpLoadPage* CAgpLoader::x_GetParamsPanel()
{
    if(m_ParamsPanel == NULL)   {
        m_ParamsPanel = new CAgpLoadPage(m_ParentWindow);
        m_ParamsPanel->SetData(m_Params);
        m_ParamsPanel->TransferDataToWindow();
    }
    return m_ParamsPanel;
}

bool CAgpLoader::CanDo(EAction action)
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


bool CAgpLoader::IsFinalState()
{
    return m_State == eParams;
}


bool CAgpLoader::IsCompletedState()
{
    return m_State == eCompleted; // does not matter
}


bool CAgpLoader::DoTransition(EAction action)
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


IAppTask* CAgpLoader::GetTask()
{
    return nullptr;
}

IExecuteUnit* CAgpLoader::GetExecuteUnit()
{
    return new CAgpObjectLoader(m_Params, m_FileNames);
}

wxString CAgpLoader::GetFormatWildcard()
{
    return CFileExtensions::GetDialogFilter(CFileExtensions::kAGP) + wxT("|") +
           CFileExtensions::GetDialogFilter(CFileExtensions::kAllFiles);
}


bool CAgpLoader::ValidateFilenames(const vector<wxString>& /*filenames*/)
{
    // not implemented
    return true;
}


void CAgpLoader::SetFilenames(const vector<wxString>& filenames)
{
    m_FileNames = filenames;
}

void CAgpLoader::GetFilenames(vector<wxString>& filenames) const
{
    filenames = m_FileNames;
}

bool CAgpLoader::IsInitialState()
{
    return m_State == eParams;
}

bool CAgpLoader::RecognizeFormat(const wxString& filename)
{
    wxString ext;
    wxFileName::SplitPath(filename, 0, 0, &ext);
    return CFileExtensions::RecognizeExtension(CFileExtensions::kAGP, ext);
}

bool CAgpLoader::RecognizeFormat(CFormatGuess::EFormat fmt)
{
    return fmt == CFormatGuess::eAgp;
}

string CAgpLoader::GetExtensionIdentifier() const
{
    static string sid("agp_format_load_manager");
    return sid;
}


string CAgpLoader::GetExtensionLabel() const
{
    static string slabel("AGP Format Load Manager");
    return slabel;
}

void CAgpLoader::SetRegistryPath(const string& path)
{
    m_RegPath = path; // store for later use
    m_Params.SetRegistryPath(m_RegPath + ".Params");
}

void CAgpLoader::SaveSettings() const
{
    m_Params.SaveSettings();
}


void CAgpLoader::LoadSettings()
{
    m_Params.LoadSettings();
}

END_NCBI_SCOPE
