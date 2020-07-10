/*  $Id: fasta_format_load_manager.cpp 38522 2017-05-22 18:19:49Z evgeniev $
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

#include "fasta_format_load_manager.hpp"

#include <gui/widgets/loaders/fasta_format_params_panel.hpp>
#include <gui/widgets/loaders/fasta_object_loader.hpp>

#include <gui/widgets/wx/wx_utils.hpp>
#include <gui/widgets/wx/file_extensions.hpp>

#include <wx/filename.h>

BEGIN_NCBI_SCOPE

wxPanel* CFASTAFormatLoadManager::CPage::GetPanel()
{
    return m_Manager.x_GetParamsPanel();
}

bool CFASTAFormatLoadManager::CPage::CanLeavePage(bool)
{
    CFASTAFormatParamsPanel* panel = m_Manager.x_GetParamsPanel();
    if (panel) {
        if (!panel->TransferDataFromWindow())
            return false;
        m_Manager.m_Params = panel->GetData();
    }

    return true;
}

///////////////////////////////////////////////////////////////////////////////
/// CFASTAFormatLoadManager
CFASTAFormatLoadManager::CFASTAFormatLoadManager()
     : m_Descr("FASTA Sequence files", "")
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

string CFASTAFormatLoadManager::GetLabel() const
{
	return m_Descr.GetLabel();
}

wxString CFASTAFormatLoadManager::GetFormatWildcard() const
{
    return CFileExtensions::GetDialogFilter(CFileExtensions::kFASTA) + wxT("|") +
           CFileExtensions::GetDialogFilter(CFileExtensions::kAllFiles);
}

//
// IUILoadManager implementation
//

void CFASTAFormatLoadManager::SetServiceLocator(IServiceLocator* srv_locator)
{
    m_SrvLocator = srv_locator;
}


void CFASTAFormatLoadManager::SetParentWindow(wxWindow* parent)
{
    m_ParentWindow = parent;
}


const IUIObject& CFASTAFormatLoadManager::GetDescriptor() const
{
    return m_Descr;
}


void CFASTAFormatLoadManager::InitUI()
{
    m_State = eParams;
}


void CFASTAFormatLoadManager::CleanUI()
{
    m_State = eInvalid;
    m_ParamsPanel = NULL; // window is destroyed by the system
}


wxPanel* CFASTAFormatLoadManager::GetCurrentPanel()
{
    return (m_State == eParams) ? x_GetParamsPanel() : NULL;
}

CFASTAFormatParamsPanel* CFASTAFormatLoadManager::x_GetParamsPanel()
{
    if (m_ParamsPanel == NULL) {
        m_ParamsPanel = new CFASTAFormatParamsPanel(m_ParentWindow);
        m_ParamsPanel->SetData(m_Params);
        m_ParamsPanel->TransferDataToWindow();
    }
    return m_ParamsPanel;
}

bool CFASTAFormatLoadManager::CanDo(EAction action)
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


bool CFASTAFormatLoadManager::IsFinalState()
{
    return m_State == eParams;
}


bool CFASTAFormatLoadManager::IsCompletedState()
{
    return m_State == eCompleted; // does not matter
}


bool CFASTAFormatLoadManager::DoTransition(EAction action)
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


IAppTask* CFASTAFormatLoadManager::GetTask()
{
    return nullptr;
}

IExecuteUnit* CFASTAFormatLoadManager::GetExecuteUnit()
{
    return new CFastaObjectLoader(m_Params, m_FileNames);
}

wxString CFASTAFormatLoadManager::GetFormatWildcard()
{
    return CFileExtensions::GetDialogFilter(CFileExtensions::kFASTA) + wxT("|") +
           CFileExtensions::GetDialogFilter(CFileExtensions::kAllFiles);
}


bool CFASTAFormatLoadManager::ValidateFilenames(const vector<wxString>& /*filenames*/)
{
    // not implemented
    return true;
}


void CFASTAFormatLoadManager::SetFilenames(const vector<wxString>& filenames)
{
    m_FileNames = filenames;
}

void CFASTAFormatLoadManager::GetFilenames(vector<wxString>& filenames) const
{
    filenames = m_FileNames;
}

bool CFASTAFormatLoadManager::IsInitialState()
{
    return m_State == eParams;
}


bool CFASTAFormatLoadManager::RecognizeFormat(const wxString& filename)
{
    wxString ext;
    wxFileName::SplitPath(filename, 0, 0, &ext);
    return CFileExtensions::RecognizeExtension(CFileExtensions::kFASTA, ext);
}

bool CFASTAFormatLoadManager::RecognizeFormat(CFormatGuess::EFormat fmt)
{
    return fmt == CFormatGuess::eFasta;
}

string CFASTAFormatLoadManager::GetExtensionIdentifier() const
{
    static string sid("fasta_format_load_manager");
    return sid;
}


string CFASTAFormatLoadManager::GetExtensionLabel() const
{
    static string slabel("FASTA Format Load Manager");
    return slabel;
}

void CFASTAFormatLoadManager::SetRegistryPath(const string& path)
{
    m_RegPath = path; // store for later use
    m_Params.SetRegistryPath(m_RegPath + ".FASTAParams");
}

void CFASTAFormatLoadManager::SaveSettings() const
{
    m_Params.SaveSettings();
}


void CFASTAFormatLoadManager::LoadSettings()
{
    m_Params.LoadSettings();
}

END_NCBI_SCOPE
