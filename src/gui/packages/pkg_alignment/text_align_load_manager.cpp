/*  $Id: text_align_load_manager.cpp 38477 2017-05-15 21:10:59Z evgeniev $
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

#include <gui/widgets/loaders/text_align_object_loader.hpp>
#include <gui/widgets/loaders/text_align_params_panel.hpp>

#include "text_align_load_manager.hpp"

#include <gui/widgets/wx/wx_utils.hpp>
#include <gui/widgets/wx/file_extensions.hpp>

#include <wx/filename.h>

BEGIN_NCBI_SCOPE

wxPanel* CTextAlignLoaderManager::CPage::GetPanel()
{
    return m_Manager.x_GetParamsPanel();
}

///////////////////////////////////////////////////////////////////////////////
/// CTextAlignLoaderManager
CTextAlignLoaderManager::CTextAlignLoaderManager()
 : m_SrvLocator(NULL)
 , m_ParentWindow(NULL)
 , m_State(eInvalid)
 , m_ParamsPanel(NULL)
 , m_Descr("", "")
 , m_OptionsPage(*this)
{
    m_Descr.Init(ToStdString(CFileExtensions::GetLabel(CFileExtensions::kTextAlign)), "");
    m_Descr.SetLogEvent("loaders");
}

//
// IFileLoadPanelClient implementation
//

string CTextAlignLoaderManager::GetLabel() const
{
	return m_Descr.GetLabel();
}

wxString CTextAlignLoaderManager::GetFormatWildcard() const
{
    return CFileExtensions::GetDialogFilter(CFileExtensions::kAllFiles);
}

//
// IUILoadManager implementation
//

void CTextAlignLoaderManager::SetServiceLocator(IServiceLocator* srv_locator)
{
    m_SrvLocator = srv_locator;
}


void CTextAlignLoaderManager::SetParentWindow(wxWindow* parent)
{
    m_ParentWindow = parent;
}


const IUIObject& CTextAlignLoaderManager::GetDescriptor() const
{
    return m_Descr;
}


void CTextAlignLoaderManager::InitUI()
{
    m_State = eParams;
}


void CTextAlignLoaderManager::CleanUI()
{
    m_State = eInvalid;
    m_ParamsPanel = NULL; // window is destroyed by the system
}


wxPanel* CTextAlignLoaderManager::GetCurrentPanel()
{
    return (m_State == eParams) ? x_GetParamsPanel() : NULL;
}

CTextAlignParamsPanel* CTextAlignLoaderManager::x_GetParamsPanel()
{
    if (m_ParamsPanel == NULL) {
        m_ParamsPanel = new CTextAlignParamsPanel(m_ParentWindow);
        m_ParamsPanel->SetData(m_Params);
        m_ParamsPanel->TransferDataToWindow();
    }
    return m_ParamsPanel;
}

bool CTextAlignLoaderManager::CanDo(EAction action)
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


bool CTextAlignLoaderManager::IsFinalState()
{
    return m_State == eParams;
}


bool CTextAlignLoaderManager::IsCompletedState()
{
    return m_State == eCompleted; // does not matter
}


bool CTextAlignLoaderManager::DoTransition(EAction action)
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


IAppTask* CTextAlignLoaderManager::GetTask()
{
    return nullptr;
}

IExecuteUnit* CTextAlignLoaderManager::GetExecuteUnit()
{
    return new CTextAlignObjectLoader(m_Params, m_FileNames);
}


wxString CTextAlignLoaderManager::GetFormatWildcard()
{
    return CFileExtensions::GetDialogFilter(CFileExtensions::kAllFiles);
}


bool CTextAlignLoaderManager::ValidateFilenames(const vector<wxString>& /*filenames*/)
{
    // not implemented
    return true;
}


void CTextAlignLoaderManager::SetFilenames(const vector<wxString>& filenames)
{
    m_FileNames = filenames;
}

void CTextAlignLoaderManager::GetFilenames(vector<wxString>& filenames) const
{
    filenames = m_FileNames;
}

bool CTextAlignLoaderManager::IsInitialState()
{
    return m_State == eParams;
}


bool CTextAlignLoaderManager::RecognizeFormat(const wxString& filename)
{
    wxString ext;
    wxFileName::SplitPath(filename, 0, 0, &ext);
    return CFileExtensions::RecognizeExtension(CFileExtensions::kTxt, ext);
}

bool CTextAlignLoaderManager::RecognizeFormat(CFormatGuess::EFormat fmt)
{
    switch (fmt) {
    case CFormatGuess::eAlignment :
        return true;
    default :
        return false;
    }
}

string CTextAlignLoaderManager::GetExtensionIdentifier() const
{
    return "text_align_format_load_manager";
}


string CTextAlignLoaderManager::GetExtensionLabel() const
{
    static string slabel("Text Alignment Format Load Manager");
    return slabel;
}

void CTextAlignLoaderManager::SetRegistryPath(const string& path)
{
    m_RegPath = path; // store for later use
    m_Params.SetRegistryPath(m_RegPath + ".TextAlignParams");
}

void CTextAlignLoaderManager::SaveSettings() const
{
    m_Params.SaveSettings();
}


void CTextAlignLoaderManager::LoadSettings()
{
    m_Params.LoadSettings();
}

END_NCBI_SCOPE
