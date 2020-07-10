/*  $Id: rm_load_manager.cpp 38477 2017-05-15 21:10:59Z evgeniev $
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

#include "rm_load_manager.hpp"

#include <gui/widgets/loaders/rm_object_loader.hpp>

#include <gui/widgets/wx/wx_utils.hpp>
#include <gui/widgets/wx/file_extensions.hpp>

#include <wx/filename.h>

BEGIN_NCBI_SCOPE


///////////////////////////////////////////////////////////////////////////////
/// CRMLoader
CRMLoader::CRMLoader()
:   m_Descr("RepeatMasker files", ""),
    m_SrvLocator(NULL),
    m_ParentWindow(NULL)
    //m_FinalState(false)
{
    m_Descr.SetLogEvent("loaders");
}

//
// IFileLoadPanelClient implementation
//

string CRMLoader::GetLabel() const
{
	return m_Descr.GetLabel();
}

wxString CRMLoader::GetFormatWildcard() const
{
    return CFileExtensions::GetDialogFilter(CFileExtensions::kTxt) + wxT("|") +
           CFileExtensions::GetDialogFilter(CFileExtensions::kAllFiles);
}

//
// IUILoadManager implementation
//


void CRMLoader::SetServiceLocator(IServiceLocator* srv_locator)
{
    m_SrvLocator = srv_locator;
}


void CRMLoader::SetParentWindow(wxWindow* parent)
{
    m_ParentWindow = parent;
}


const IUIObject& CRMLoader::GetDescriptor() const
{
    return m_Descr;
}


void CRMLoader::InitUI()
{
}


void CRMLoader::CleanUI()
{
}


wxPanel* CRMLoader::GetCurrentPanel()
{
    // we do not have any
    return NULL;
}


bool CRMLoader::CanDo(EAction WXUNUSED(action))
{
    return false; //! m_FinalState  &&  action == eNext;
}


bool CRMLoader::IsFinalState()
{
    return true;//m_FinalState;
}


bool CRMLoader::IsCompletedState()
{
    return true;//m_FinalState;
}


bool CRMLoader::DoTransition(EAction WXUNUSED(action))
{
    _ASSERT(false); // we do not have panels and transitions
    return false;
}


IAppTask* CRMLoader::GetTask()
{
    return nullptr;
}

IExecuteUnit* CRMLoader::GetExecuteUnit()
{
    return new CRMObjectLoader(m_FileNames);
}


wxString CRMLoader::GetFormatWildcard()
{
    return CFileExtensions::GetDialogFilter(CFileExtensions::kTxt) + wxT("|") +
           CFileExtensions::GetDialogFilter(CFileExtensions::kAllFiles);
}


bool CRMLoader::ValidateFilenames(const vector<wxString>&)
{
    // not implemented
    return true;
}


void CRMLoader::SetFilenames(const vector<wxString>& filenames)
{
    m_FileNames = filenames;
}

void CRMLoader::GetFilenames(vector<wxString>& filenames) const
{
    filenames = m_FileNames;
}

bool CRMLoader::IsInitialState()
{
    return true;//! m_FinalState;
}


bool CRMLoader::RecognizeFormat(const wxString& filename)
{
    wxString ext;
    wxFileName::SplitPath(filename, 0, 0, &ext);
    return CFileExtensions::RecognizeExtension(CFileExtensions::kTxt, ext);
}

bool CRMLoader::RecognizeFormat(CFormatGuess::EFormat fmt)
{
    return fmt == CFormatGuess::eRmo;
}

string CRMLoader::GetExtensionIdentifier() const
{
    static string sid("rm_format_load_manager");
    return sid;
}


string CRMLoader::GetExtensionLabel() const
{
    static string slabel("RepeatMasker output Format Load Manager");
    return slabel;
}

END_NCBI_SCOPE
