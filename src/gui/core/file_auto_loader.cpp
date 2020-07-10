/*  $Id: file_auto_loader.cpp 39752 2017-11-01 15:14:01Z katargir $
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

#include <util/compress/stream.hpp>
#include <util/format_guess.hpp>
#include <gui/utils/extension_impl.hpp>
#include <gui/core/file_auto_loader.hpp>
#include <gui/widgets/wx/compressed_file.hpp>
#include <gui/widgets/wx/file_extensions.hpp>

#include <wx/msgdlg.h>
#include <wx/filename.h>

BEGIN_NCBI_SCOPE

///////////////////////////////////////////////////////////////////////////////
/// CFileAutoLoader
CFileAutoLoader::CFileAutoLoader()
: m_Descr("Autodetect format", "")
{
}

void CFileAutoLoader::SetServiceLocator(IServiceLocator*)
{
}

void CFileAutoLoader::SetParentWindow(wxWindow* parent)
{
}

const IUIObject& CFileAutoLoader::GetDescriptor() const
{
    return m_FormatManager ? m_FormatManager->GetDescriptor() : m_Descr;
}

void CFileAutoLoader::InitUI()
{
}


void CFileAutoLoader::CleanUI()
{
    m_FormatManager.Reset();
}

wxPanel* CFileAutoLoader::GetCurrentPanel()
{
    return m_FormatManager ? m_FormatManager->GetCurrentPanel() : 0;
}


bool CFileAutoLoader::CanDo(EAction action)
{
    return m_FormatManager ? m_FormatManager->CanDo(action) : false;
}


bool CFileAutoLoader::IsFinalState()
{
    return m_FormatManager ? m_FormatManager->IsFinalState() : false;
}

bool CFileAutoLoader::IsCompletedState()
{
    return m_FormatManager ? m_FormatManager->IsCompletedState() : false;
}

bool CFileAutoLoader::DoTransition(EAction action)
{
    return m_FormatManager ?
        m_FormatManager->DoTransition(action) : false;
}

IAppTask* CFileAutoLoader::GetTask()
{
    return m_FormatManager ? m_FormatManager->GetTask() : nullptr;
}

IExecuteUnit* CFileAutoLoader::GetExecuteUnit()
{
    return m_FormatManager ? m_FormatManager->GetExecuteUnit() : nullptr;
}

wxString CFileAutoLoader::GetFormatWildcard()
{
    wxString filters, separator;
    for (int t = CFileExtensions::kFileTypeStart; t <= CFileExtensions::kFileTypeEnd; ++t) {
        filters += separator + CFileExtensions::GetDialogFilter((CFileExtensions::EFileType)t);
        separator = wxT("|");
    }
    return filters;
}

CFormatGuess::EFormat CFileAutoLoader::SetFormatManager(const vector<wxString>& filenames)
{
    m_FormatManager.Reset();

    CFormatGuess::EFormat fmt = CFormatGuess::eUnknown;
    CFormatGuess::EFormat fmt_cmprs = fmt;

    try {
        CCompressedFile file(filenames[0].fn_str());
        fmt = file.GuessFormat();
    }
    catch (CException&) {
    }

    vector<CIRef<IFileFormatLoaderManager> > compatible_managers;

    if (fmt != CFormatGuess::eUnknown) {
        vector< CIRef<IFileFormatLoaderManager> > managers;
        vector<CIRef<IFileFormatLoaderManager> > compatible_managers;

        GetExtensionAsInterface("file_format_loader_manager", managers);

        // get list of compatible managers
        //
        for (size_t i = 0;  i < managers.size();  ++i) {
            IFileFormatLoaderManager* mgr = managers[i].GetPointer();
            if (mgr->RecognizeFormat(fmt)) {
                compatible_managers.push_back( CIRef<IFileFormatLoaderManager>(mgr) );
            }
        }
        if (compatible_managers.size())
        {
            // check the extension to resolve the conflict
            // (maybe we it is not enough to pick the right one... (?))
            for (size_t i  = 0; i < compatible_managers.size(); ++i) {
                IFileFormatLoaderManager* mgr = compatible_managers[i].GetPointer();
                wxString fn(filenames[0]);
                if (fmt_cmprs != CFormatGuess::eUnknown) { // compressed
                    wxFileName fname(fn);
                    fname.ClearExt();
                    fn = fname.GetFullPath();
                }
                if (mgr->RecognizeFormat(fn)) {
                    m_FormatManager.Reset(mgr);
                    break;
                }
            }
            // pick the first one... and hope it can handle the file
            if (!m_FormatManager) {
                IFileFormatLoaderManager* mgr = compatible_managers[0].GetPointer();
                m_FormatManager.Reset(mgr);                
            }
        }
    }
    return fmt;
}


bool CFileAutoLoader::ValidateFilenames(const vector<wxString>& filenames)
{
    CFormatGuess::EFormat fmt = SetFormatManager(filenames);
    if (!m_FormatManager) {
        if (fmt != CFormatGuess::eTable && fmt != CFormatGuess::eBam) {
            wxMessageBox(wxT("The file type cannot be automatically determined.\n")
                         wxT("Please select type from the \"File Format\" list."), wxT("Choose a file type"),
                         wxICON_ERROR | wxOK);
        }

        return false;        
    }

    return m_FormatManager->ValidateFilenames(filenames);
}

void CFileAutoLoader::SetFilenames(const vector<wxString>& filenames)
{
    if (m_FormatManager) {
        if (m_FormatManager->SingleFileLoader() && !filenames.empty()) {
            vector<wxString> fnames;
            fnames.push_back(filenames[0]);
            m_FormatManager->SetFilenames(fnames);
        }
        else
            m_FormatManager->SetFilenames(filenames);
    }
}

void CFileAutoLoader::GetFilenames(vector<wxString>& filenames) const
{
    filenames.clear();

    if (m_FormatManager) {
        m_FormatManager->GetFilenames(filenames);
    }
}

bool CFileAutoLoader::IsInitialState()
{
    return m_FormatManager ? m_FormatManager->IsInitialState() : true;
}

bool CFileAutoLoader::RecognizeFormat(const wxString& filename)
{
    return true;
}

string CFileAutoLoader::GetExtensionIdentifier() const
{
    static string sid("autodetect_format_load_manager");
    return sid;
}


string CFileAutoLoader::GetExtensionLabel() const
{
    static string slabel("Autodetect Format Load Manager");
    return slabel;
}

END_NCBI_SCOPE
