/*  $Id: 5col_load_manager.cpp 38477 2017-05-15 21:10:59Z evgeniev $
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

#include "5col_load_manager.hpp"

#include <gui/widgets/loaders/5col_object_loader.hpp>

#include <gui/widgets/wx/wx_utils.hpp>
#include <gui/widgets/wx/file_extensions.hpp>

#include <wx/filename.h>

BEGIN_NCBI_SCOPE

///////////////////////////////////////////////////////////////////////////////
/// C5ColLoadManager
C5ColLoadManager::C5ColLoadManager()
 : m_SrvLocator(NULL)
 , m_Descr("", "")
{
    m_Descr.Init(ToStdString(CFileExtensions::GetLabel(CFileExtensions::k5Column)), "");
    m_Descr.SetLogEvent("loaders");
}

//
// IFileLoadPanelClient implementation
//

string C5ColLoadManager::GetLabel() const
{
	return m_Descr.GetLabel();
}

wxString C5ColLoadManager::GetFormatWildcard() const
{
    return CFileExtensions::GetDialogFilter(CFileExtensions::k5Column) + wxT("|") +
           CFileExtensions::GetDialogFilter(CFileExtensions::kAllFiles);
}

//
// IUILoadManager implementation
//

void C5ColLoadManager::SetServiceLocator(IServiceLocator* srv_locator)
{
    m_SrvLocator = srv_locator;
}


void C5ColLoadManager::SetParentWindow(wxWindow* WXUNUSED(parent))
{
}


const IUIObject& C5ColLoadManager::GetDescriptor() const
{
    return m_Descr;
}


void C5ColLoadManager::InitUI()
{
}


void C5ColLoadManager::CleanUI()
{
}


wxPanel* C5ColLoadManager::GetCurrentPanel()
{
    return NULL;
}

bool C5ColLoadManager::CanDo(EAction WXUNUSED(action))
{
    return false;
}


bool C5ColLoadManager::IsFinalState()
{
    return true;
}


bool C5ColLoadManager::IsCompletedState()
{
    return true;
}


bool C5ColLoadManager::DoTransition(EAction action)
{
    _ASSERT(false);
    return false;
}


IAppTask* C5ColLoadManager::GetTask()
{
    return nullptr;
}

IExecuteUnit* C5ColLoadManager::GetExecuteUnit()
{
    return new C5ColObjectLoader(m_FileNames);
}


wxString C5ColLoadManager::GetFormatWildcard()
{
    return CFileExtensions::GetDialogFilter(CFileExtensions::k5Column) + wxT("|") +
           CFileExtensions::GetDialogFilter(CFileExtensions::kAllFiles);
}


bool C5ColLoadManager::ValidateFilenames(const vector<wxString>& /*filenames*/)
{
    // not implemented
    return true;
}


void C5ColLoadManager::SetFilenames(const vector<wxString>& filenames)
{
    m_FileNames = filenames;
}

void C5ColLoadManager::GetFilenames(vector<wxString>& filenames) const
{
    filenames = m_FileNames;
}

bool C5ColLoadManager::IsInitialState()
{
    return true;
}


bool C5ColLoadManager::RecognizeFormat(const wxString& filename)
{
    wxString ext;
    wxFileName::SplitPath(filename, 0, 0, &ext);
    return CFileExtensions::RecognizeExtension(CFileExtensions::k5Column, ext);
}

bool C5ColLoadManager::RecognizeFormat(CFormatGuess::EFormat fmt)
{
    return fmt == CFormatGuess::eFiveColFeatureTable;
}

string C5ColLoadManager::GetExtensionIdentifier() const
{
    return "z5col_format_load_manager";
}


string C5ColLoadManager::GetExtensionLabel() const
{
    static string slabel("5 Column Format Load Manager");
    return slabel;
}

void C5ColLoadManager::SetRegistryPath(const string& path)
{
    m_RegPath = path; // store for later use
}

void C5ColLoadManager::SaveSettings() const
{
}


void C5ColLoadManager::LoadSettings()
{
}

END_NCBI_SCOPE
