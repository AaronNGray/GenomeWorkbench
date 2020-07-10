/*  $Id: phy_loader.cpp 38477 2017-05-15 21:10:59Z evgeniev $
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
* Authors:  Vladimir Tereshkov
*
*/

#include <ncbi_pch.hpp>

#include "phy_loader.hpp"

#include <gui/widgets/loaders/phy_object_loader.hpp>

#include <gui/widgets/wx/wx_utils.hpp>
#include <gui/widgets/wx/file_extensions.hpp>

#include <wx/filename.h>

BEGIN_NCBI_SCOPE

///////////////////////////////////////////////////////////////////////////////
/// CPhyLoader
CPhyLoader::CPhyLoader()
    : m_Descr( "Newick/Nexus Tree files", "" )
    , m_SrvLocator( NULL )
    , m_ParentWindow( NULL )
    //, m_State( eInvalid )
{
    m_Descr.SetLogEvent("loaders");
}

//
// IFileLoadPanelClient implementation
//

string CPhyLoader::GetLabel() const
{
	return m_Descr.GetLabel();
}

wxString CPhyLoader::GetFormatWildcard() const
{
    return CFileExtensions::GetDialogFilter(CFileExtensions::kTree) + wxT("|") +
           CFileExtensions::GetDialogFilter(CFileExtensions::kAllFiles);
}

//
// IUILoadManager implementation
//

void CPhyLoader::SetServiceLocator(IServiceLocator* srv_locator)
{
    m_SrvLocator = srv_locator;
}


void CPhyLoader::SetParentWindow(wxWindow* parent)
{
    m_ParentWindow = parent;
}


const IUIObject& CPhyLoader::GetDescriptor() const
{
    return m_Descr;
}


void CPhyLoader::InitUI()
{
    //m_State = eParams;
    //m_State = eCompleted;
}


void CPhyLoader::CleanUI()
{
    //m_State = eInvalid;    
}


wxPanel* CPhyLoader::GetCurrentPanel()
{
    return NULL;
}


bool CPhyLoader::CanDo(EAction action)
{
    /*
    switch(m_State) {
    case eParams:
        return action == eNext;
    case eCompleted:
        return action == eBack;
    default:
        _ASSERT(false);
        return false;
    }
    */

    return true; // does not matter
}


bool CPhyLoader::IsFinalState()
{
    //return m_State == eParams;
    return true;
}


bool CPhyLoader::IsCompletedState()
{
    //return m_State == eCompleted;
    return true;
}


bool CPhyLoader::DoTransition( EAction action )
{
	return true;
}


IAppTask* CPhyLoader::GetTask()
{
	return nullptr;
}

IExecuteUnit* CPhyLoader::GetExecuteUnit()
{
    return new CPhyObjectLoader(m_FileNames);
}


wxString CPhyLoader::GetFormatWildcard()
{
    return CFileExtensions::GetDialogFilter(CFileExtensions::kTree) + wxT("|") +
           CFileExtensions::GetDialogFilter(CFileExtensions::kAllFiles);
}


bool CPhyLoader::ValidateFilenames( const vector<wxString>& filenames )
{
    // not implemented
    return true;
}


void CPhyLoader::SetFilenames( const vector<wxString>& filenames )
{
    m_FileNames = filenames;
}

void CPhyLoader::GetFilenames( vector<wxString>& filenames ) const
{
    filenames = m_FileNames;
}

bool CPhyLoader::IsInitialState()
{
    //return m_State == eParams;
    return true;
}

bool CPhyLoader::RecognizeFormat( const wxString& filename )
{
    wxString ext;
    wxFileName::SplitPath(filename, 0, 0, &ext);
    return CFileExtensions::RecognizeExtension(CFileExtensions::kTree, ext);
}

bool CPhyLoader::RecognizeFormat( CFormatGuess::EFormat fmt )
{
    return fmt == CFormatGuess::eNewick;
}

string CPhyLoader::GetExtensionIdentifier() const
{
    static string sid("phy_format_load_manager");
    return sid;
}


string CPhyLoader::GetExtensionLabel() const
{
    static string slabel("Newick Tree Load Manager");
    return slabel;
}

void CPhyLoader::SetRegistryPath( const string& path )
{
    m_RegPath = path; // store for later use
}

void CPhyLoader::SaveSettings() const
{
}


void CPhyLoader::LoadSettings()
{
}

END_NCBI_SCOPE
