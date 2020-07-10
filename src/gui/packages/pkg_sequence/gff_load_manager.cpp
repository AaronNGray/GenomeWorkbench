/*  $Id: gff_load_manager.cpp 38477 2017-05-15 21:10:59Z evgeniev $
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

#include "gff_load_manager.hpp"

#include <gui/widgets/loaders/gff_params_panel.hpp>
#include <gui/widgets/loaders/gff_object_loader.hpp>

#include <gui/widgets/wx/wx_utils.hpp>
#include <gui/widgets/wx/file_extensions.hpp>

#include <wx/filename.h>

BEGIN_NCBI_SCOPE

wxPanel* CGffLoadManager::CPage::GetPanel()
{
    return m_Manager.x_GetParamsPanel();
}

///////////////////////////////////////////////////////////////////////////////
/// CGffLoadManager
CGffLoadManager::CGffLoadManager()
     : m_SrvLocator(NULL)
     , m_ParentWindow(NULL)
     , m_State(eInvalid)
     , m_ParamsPanel(NULL)
     , m_Descr("", "")
     , m_OptionsPage(*this)
{
    m_Descr.Init(ToStdString(CFileExtensions::GetLabel(CFileExtensions::kGFF)), "");
    m_Descr.SetLogEvent("loaders");
}

//
// IFileLoadPanelClient implementation
//

string CGffLoadManager::GetLabel() const
{
	return m_Descr.GetLabel();
}

wxString CGffLoadManager::GetFormatWildcard() const
{
    return CFileExtensions::GetDialogFilter(CFileExtensions::kGFF) + wxT("|") +
		   CFileExtensions::GetDialogFilter(CFileExtensions::kGTF) + wxT("|") +
           CFileExtensions::GetDialogFilter(CFileExtensions::kAllFiles);
}

//
// IUILoadManager implementation
//

void CGffLoadManager::SetServiceLocator(IServiceLocator* srv_locator)
{
    m_SrvLocator = srv_locator;
}


void CGffLoadManager::SetParentWindow(wxWindow* parent)
{
    m_ParentWindow = parent;
}


const IUIObject& CGffLoadManager::GetDescriptor() const
{
    return m_Descr;
}


void CGffLoadManager::InitUI()
{
    m_State = eParams;
}


void CGffLoadManager::CleanUI()
{
    m_State = eInvalid;
    m_ParamsPanel = NULL; // window is destroyed by the system
}


wxPanel* CGffLoadManager::GetCurrentPanel()
{
    return (m_State == eParams) ? x_GetParamsPanel() : NULL;
}

CGffParamsPanel* CGffLoadManager::x_GetParamsPanel()
{
    if (m_ParamsPanel == NULL) {
        m_ParamsPanel = new CGffParamsPanel(m_ParentWindow);
        m_ParamsPanel->SetData(m_Params);
        m_ParamsPanel->TransferDataToWindow();
    }
    return m_ParamsPanel;
}

bool CGffLoadManager::CanDo(EAction action)
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


bool CGffLoadManager::IsFinalState()
{
    return m_State == eParams;
}


bool CGffLoadManager::IsCompletedState()
{
    return m_State == eCompleted; // does not matter
}


bool CGffLoadManager::DoTransition(EAction action)
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


IAppTask* CGffLoadManager::GetTask()
{
    return nullptr;
}

IExecuteUnit* CGffLoadManager::GetExecuteUnit()
{
    return new CGffObjectLoader(m_Params, m_FileNames);
}


wxString CGffLoadManager::GetFormatWildcard()
{
    return CFileExtensions::GetDialogFilter(CFileExtensions::kGFF) + wxT("|") +
		   CFileExtensions::GetDialogFilter(CFileExtensions::kGTF) + wxT("|") +
           CFileExtensions::GetDialogFilter(CFileExtensions::kAllFiles);
}


bool CGffLoadManager::ValidateFilenames(const vector<wxString>& /*filenames*/)
{
    // not implemented
    return true;
}


void CGffLoadManager::SetFilenames(const vector<wxString>& filenames)
{
    m_FileNames = filenames;
}

void CGffLoadManager::GetFilenames(vector<wxString>& filenames) const
{
    filenames = m_FileNames;
}

bool CGffLoadManager::IsInitialState()
{
    return m_State == eParams;
}


bool CGffLoadManager::RecognizeFormat(const wxString& filename)
{
    wxString ext;
    wxFileName::SplitPath(filename, 0, 0, &ext);
    return CFileExtensions::RecognizeExtension(CFileExtensions::kGFF, ext) ||
		   CFileExtensions::RecognizeExtension(CFileExtensions::kGTF, ext);
}

bool CGffLoadManager::RecognizeFormat(CFormatGuess::EFormat fmt)
{
    switch (fmt) {
    case CFormatGuess::eGff2 :
    case CFormatGuess::eGff3 :
    case CFormatGuess::eGtf :
    case CFormatGuess::eGvf :
    case CFormatGuess::eGtf_POISENED :
        return true;
    default :
        return false;
    }
}

string CGffLoadManager::GetExtensionIdentifier() const
{
    return "gff_format_load_manager";
}


string CGffLoadManager::GetExtensionLabel() const
{
    static string slabel("GFF/GTF/GFV Format Load Manager");
    return slabel;
}

void CGffLoadManager::SetRegistryPath(const string& path)
{
    m_RegPath = path; // store for later use
    m_Params.SetRegistryPath(m_RegPath + ".GffParams");
}

void CGffLoadManager::SaveSettings() const
{
    m_Params.SaveSettings();
}


void CGffLoadManager::LoadSettings()
{
    m_Params.LoadSettings();
}

END_NCBI_SCOPE
