/*  $Id: table_file_load_manager.cpp 38477 2017-05-15 21:10:59Z evgeniev $
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

#include <gui/core/table_file_load_manager.hpp>

#include <gui/widgets/wx/wx_utils.hpp>
#include <gui/widgets/wx/file_extensions.hpp>

#include <wx/filename.h>

BEGIN_NCBI_SCOPE

///////////////////////////////////////////////////////////////////////////////
/// CTableFileLoadManager
CTableFileLoadManager::CTableFileLoadManager()
:   m_Descr("Table files", ""),
    m_SrvLocator(NULL)
{
    m_Descr.SetLogEvent("loaders");
}


//
// IFileLoadPanelClient implementation
//

string CTableFileLoadManager::GetLabel() const
{
	return m_Descr.GetLabel();
}

wxString CTableFileLoadManager::GetFormatWildcard() const
{
    return CFileExtensions::GetDialogFilter(CFileExtensions::kTable) + wxT("|") +
           CFileExtensions::GetDialogFilter(CFileExtensions::kAllFiles);
}

//
// IUILoadManager implementation
//

void CTableFileLoadManager::SetServiceLocator(IServiceLocator* srv_locator)
{
    m_SrvLocator = srv_locator;
}


void CTableFileLoadManager::SetParentWindow(wxWindow* parent)
{
    m_Wizard.SetParentWindow(parent);
}


const IUIObject& CTableFileLoadManager::GetDescriptor() const
{
    return m_Descr;
}


void CTableFileLoadManager::InitUI()
{
    m_Wizard.InitUI();
}


void CTableFileLoadManager::CleanUI()
{
    m_Wizard.CleanUI();
}


wxPanel* CTableFileLoadManager::GetCurrentPanel()
{
    return m_Wizard.GetCurrentPanel();
}

void CTableFileLoadManager::SetRegistryPath(const string& path)
{
    m_Wizard.SetRegistryPath(path);
}

void CTableFileLoadManager::SaveSettings() const
{
}


void CTableFileLoadManager::LoadSettings()
{
}

bool CTableFileLoadManager::CanDo(EAction action)
{
    if (IsCompletedState())
        return false;

    if (IsInitialState()) {
        return action == eNext;
    }

    if (action == eBack || action == eNext)
        return true;

    _ASSERT(false);
    return false;
}


bool CTableFileLoadManager::IsFinalState()
{
    return m_Wizard.OnLastPage();
}


bool CTableFileLoadManager::IsCompletedState()
{
    return m_Wizard.IsCompletedState();
}


bool CTableFileLoadManager::DoTransition(EAction action)
{
    if (eNext == action) {
        return m_Wizard.GoNextPage();
    }
    else if (eBack == action) {
        return m_Wizard.GoPrevPage();
    }
    else {
        return false;
    }
    _ASSERT(false);
    return false;
}


IAppTask* CTableFileLoadManager::GetTask()
{
    return nullptr;
}

IExecuteUnit* CTableFileLoadManager::GetExecuteUnit()
{
    return m_Wizard.GetExecuteUnit(m_FileNames);
}

wxString CTableFileLoadManager::GetFormatWildcard()
{
    return CFileExtensions::GetDialogFilter(CFileExtensions::kTable) + wxT("|") +
           CFileExtensions::GetDialogFilter(CFileExtensions::kAllFiles);
}


bool CTableFileLoadManager::ValidateFilenames(const vector<wxString>& filenames)
{
    return m_Wizard.ValidateFilenames(filenames);
}


void CTableFileLoadManager::SetFilenames(const vector<wxString>& filenames)
{
    m_FileNames = filenames;
}

void CTableFileLoadManager::GetFilenames(vector<wxString>& filenames) const
{
    filenames = m_FileNames;
}

bool CTableFileLoadManager::IsInitialState()
{
    return m_Wizard.OnFirstPage();
}


bool CTableFileLoadManager::RecognizeFormat(const wxString& filename)
{
    wxString ext;
    wxFileName::SplitPath(filename, 0, 0, &ext);
    return CFileExtensions::RecognizeExtension(CFileExtensions::kTable, ext);
}

bool CTableFileLoadManager::RecognizeFormat(CFormatGuess::EFormat fmt)
{
    return fmt == CFormatGuess::eTable;
}

string CTableFileLoadManager::GetExtensionIdentifier() const
{
    static string sid("table_file_load_manager");
    return sid;
}


string CTableFileLoadManager::GetExtensionLabel() const
{
    static string slabel("Table File Load Manager");
    return slabel;
}

END_NCBI_SCOPE
