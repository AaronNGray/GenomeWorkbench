/*  $Id: asn_format_load_manager.cpp 38477 2017-05-15 21:10:59Z evgeniev $
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

#include <gui/core/asn_format_load_manager.hpp>
#include <gui/widgets/loaders/asn_object_loader.hpp>

#include <gui/widgets/wx/wx_utils.hpp>
#include <gui/widgets/wx/file_extensions.hpp>

#include <wx/filename.h>

BEGIN_NCBI_SCOPE


///////////////////////////////////////////////////////////////////////////////
/// CASNFormatLoadManager
CASNFormatLoadManager::CASNFormatLoadManager()
:   m_Descr("NCBI ASN.1 files", ""),
    m_SrvLocator(NULL),
    m_ParentWindow(NULL)
{
    m_Descr.SetLogEvent("loaders");
}

//
// IFileLoadPanelClient implementation
//

string CASNFormatLoadManager::GetLabel() const
{
	return m_Descr.GetLabel();
}

wxString CASNFormatLoadManager::GetFormatWildcard() const
{
    return CFileExtensions::GetDialogFilter(CFileExtensions::kASN) + wxT("|") +
		   CFileExtensions::GetDialogFilter(CFileExtensions::kAllFiles);
}

//
// IUILoadManager implementation
//


void CASNFormatLoadManager::SetServiceLocator(IServiceLocator* srv_locator)
{
    m_SrvLocator = srv_locator;
}


void CASNFormatLoadManager::SetParentWindow(wxWindow* parent)
{
    m_ParentWindow = parent;
}


const IUIObject& CASNFormatLoadManager::GetDescriptor() const
{
    return m_Descr;
}


void CASNFormatLoadManager::InitUI()
{
}


void CASNFormatLoadManager::CleanUI()
{
}


wxPanel* CASNFormatLoadManager::GetCurrentPanel()
{
    // we do not have any
    return NULL;
}


bool CASNFormatLoadManager::CanDo(EAction WXUNUSED(action))
{
    return false; //! m_FinalState  &&  action == eNext;
}


bool CASNFormatLoadManager::IsFinalState()
{
    return true;//m_FinalState;
}


bool CASNFormatLoadManager::IsCompletedState()
{
    return true;//m_FinalState;
}


bool CASNFormatLoadManager::DoTransition(EAction WXUNUSED(action))
{
    _ASSERT(false); // we do not have panels and transitions
    return false;
}


IAppTask* CASNFormatLoadManager::GetTask()
{
    return nullptr;
}

IExecuteUnit* CASNFormatLoadManager::GetExecuteUnit()
{
    return new CAsnObjectLoader(m_FileNames);
}

wxString CASNFormatLoadManager::GetFormatWildcard()
{
    return 
        CFileExtensions::GetDialogFilter(CFileExtensions::kASN) + wxT("|")
        + CFileExtensions::GetDialogFilter(CFileExtensions::kAllFiles)
    ;
}


bool CASNFormatLoadManager::ValidateFilenames(const vector<wxString>&)
{
    // not implemented
    return true;
}


void CASNFormatLoadManager::SetFilenames(const vector<wxString>& filenames)
{
    m_FileNames = filenames;
}

void CASNFormatLoadManager::GetFilenames(vector<wxString>& filenames) const
{
    filenames = m_FileNames;
}


bool CASNFormatLoadManager::IsInitialState()
{
    return true;//! m_FinalState;
}


bool CASNFormatLoadManager::RecognizeFormat(const wxString& filename)
{
    wxString ext;
    wxFileName::SplitPath(filename, 0, 0, &ext);
    return CFileExtensions::RecognizeExtension(CFileExtensions::kASN, ext);
}

bool CASNFormatLoadManager::RecognizeFormat(CFormatGuess::EFormat fmt)
{
    return fmt == CFormatGuess::eBinaryASN || fmt == CFormatGuess::eTextASN;
}

string CASNFormatLoadManager::GetExtensionIdentifier() const
{
    static string sid("asn_format_load_manager");
    return sid;
}


string CASNFormatLoadManager::GetExtensionLabel() const
{
    static string slabel("ASN Format Load Manager");
    return slabel;
}

END_NCBI_SCOPE
