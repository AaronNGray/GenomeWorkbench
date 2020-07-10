/*  $Id: file_load_wizard.cpp 44345 2019-12-03 19:25:07Z katargir $
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

#include "file_load_panel.hpp"
#include <gui/widgets/loaders/file_load_wizard.hpp>
#include <gui/utils/execute_unit.hpp>
#include <gui/widgets/loaders/report_loader_errors.hpp>

#include <gui/widgets/wx/message_box.hpp>
#include <gui/objutils/registry.hpp>

#include <gui/utils/extension_impl.hpp>

#include <gui/widgets/wx/wx_utils.hpp>
#include <gui/widgets/wx/wizard_dlg.hpp>

#include <wx/msgdlg.h>

BEGIN_NCBI_SCOPE

///////////////////////////////////////////////////////////////////////////////
/// CFileLoadWizard
CFileLoadWizard::CFileLoadWizard()
:   m_ParentWindow(),
    m_StartPage(),
    m_CurrFormat(-1),
    m_FileSelectPanel(NULL),
	m_FileSelectPage(*this)
{
}

wxPanel* CFileLoadWizard::CFilePage::GetPanel()
{
	return m_Manager.x_GetFileSelectPanel();
}

//
// IOpenObjectsPanelClient implementation
//

string CFileLoadWizard::GetLabel() const
{
	return "File Import";
}

IExecuteUnit* CFileLoadWizard::GetExecuteUnit()
{
    IExecuteUnit* execute_unit = 0;
    if (m_CurrFormat >= 0) {
        execute_unit = m_FormatManagers[m_CurrFormat]->GetExecuteUnit();
    }
    return execute_unit;
}

//
// IToolWizard implementation
//

void CFileLoadWizard::SetParentWindow(wxWindow* parent)
{
    m_ParentWindow = parent;

    for (size_t i = 0;  i < m_FormatManagers.size();  i++)
		dynamic_cast<IToolWizard&>(*m_FormatManagers[i]).SetParentWindow(parent);
}

IWizardPage* CFileLoadWizard::GetFirstPage()
{
    x_GetFileSelectPanel();
    return &m_FileSelectPage;
}

IWizardPage* CFileLoadWizard::GetOptionsPage()
{ 
    if (m_CurrFormat >= 0) {
        return dynamic_cast<IToolWizard&>(*m_FormatManagers[m_CurrFormat]).GetOptionsPage();
	}
	return 0;
}

void CFileLoadWizard::SetPrevPage(IWizardPage* prevPage)
{
    for (size_t i = 0;  i < m_FormatManagers.size();  i++)
		dynamic_cast<IToolWizard&>(*m_FormatManagers[i]).SetPrevPage(prevPage);
}

void CFileLoadWizard::SetNextPage(IWizardPage* nextPage)
{
    for (size_t i = 0;  i < m_FormatManagers.size();  i++)
		dynamic_cast<IToolWizard&>(*m_FormatManagers[i]).SetNextPage(nextPage);
}

void CFileLoadWizard::LoadFormats(const vector<string>& format_ids)
{
	set<string> ids;
	ids.insert(format_ids.begin(), format_ids.end());
    vector< CIRef<IFileLoadPanelClientFactory> > fileFormats;
    GetExtensionAsInterface("file_load_panel_client", fileFormats);

    for (size_t i = 0; i < fileFormats.size(); ++i) {
		if (ids.find(fileFormats[i]->GetFileLoaderId()) != ids.end())
			m_FormatManagers.push_back(CIRef<IFileLoadPanelClient>(fileFormats[i]->CreateInstance()));
	}
}

void CFileLoadWizard::SetWorkDir(const wxString& workDir) 
{ 
    m_WorkDir = workDir; 

    for (size_t i = 0; i < m_FormatManagers.size(); ++i) {
        IExecuteUnit* execute_unit = m_FormatManagers[i]->GetExecuteUnit();
        CReportLoaderErrors* reporter = dynamic_cast<CReportLoaderErrors*>(execute_unit);
        if (!reporter)
            continue;
        reporter->SetWorkDir(workDir);
    }
}

void CFileLoadWizard::SetFilenames(vector<wxString>& filenames)
{
    m_Filenames = filenames;

    if(m_FileSelectPanel)   {
        m_FileSelectPanel->SetFilenames(m_Filenames);
    }
}

void CFileLoadWizard::OnFormatChanged(int format)
{
    m_CurrFormat = format;
    wxCommandEvent evt(wxEVT_COMMAND_MENU_SELECTED, CWizardDlg::eCmdUpdateButtons);
    m_ParentWindow->ProcessWindowEvent(evt);
}

bool CFileLoadWizard::x_CanLeavePage()
{
    vector<wxString> filenames;
    m_FileSelectPanel->GetFilenames(filenames);

    if(filenames.empty())   {
        NcbiErrorBox("Please select at least one file!");
        return false;
    }

    m_CurrFormat = m_FileSelectPanel->GetSelectedFormat();
    if (!x_CheckFormatConflict(filenames)) {  // it might modify m_CurrFormat
        return false;
    }

    // select Format Manager for the format
    IFileLoadPanelClient& manager = *m_FormatManagers[m_CurrFormat];
    if (!manager.ValidateFilenames(filenames)) {
        NcbiErrorBox("The file(s) cannot be opened with this file format manager!");
        return false;
    }

    manager.SetFilenames(filenames);
    string id = manager.GetFileLoaderId();
    wxString label = ToWxString(manager.GetLabel());

    ITERATE(vector<wxString>, it, filenames)
        m_FileMRUList.Add(CFileLoadDescriptor(*it, label, id));

	m_FileSelectPanel->OnActivatePanel();

    return true;
}


bool CFileLoadWizard::x_CheckFormatConflict(const vector<wxString>& filenames)
{
    CFormatGuess::EFormat detected_fmt = CFormatGuess::eUnknown;
    try {
        CNcbiIfstream istr(filenames[0].fn_str(), ios_base::binary);
        CFormatGuess guesser(istr);
        detected_fmt = guesser.GuessFormat();
    }
    catch (const CException& e) {
        LOG_POST(Error << "Format of " << filenames[0].ToUTF8() << " could not be recognized");
        LOG_POST(Error << e.GetMsg());
        return false;
    }
    string detected_fmt_name = CFormatGuess::GetFormatName(detected_fmt);
    if (detected_fmt_name.empty())
        detected_fmt_name = "No name";

    auto& manager = m_FormatManagers[m_CurrFormat];
    if (!manager->RecognizeFormat(detected_fmt)) {
        bool found_mgr = false;
        for (size_t i = 0; i < m_FormatManagers.size() && !found_mgr; ++i) {
            if (i != m_CurrFormat) {
                auto& current_manager = m_FormatManagers[i];
                if (current_manager->RecognizeFormat(detected_fmt)) {
                    const wxFormatString kFormatConflictMsg = "Detected data format '%s' doesn't appear to match the format you selected.\n"
                        "Are you sure you want to load your data as '%s'? Click 'Yes' to proceed or click 'No' to process the data as '%s'";
                    int resp = NcbiMessageBoxW(wxString::Format(kFormatConflictMsg, detected_fmt_name, manager->GetLabel(), detected_fmt_name),
                        eDialog_YesNoCancel, eIcon_Exclamation, wxT("Warning"));
                    if (resp == eCancel) {
                        return false;
                    } else {
                        if (resp == eNo) {
                            m_CurrFormat = i;
                        }
                        found_mgr = true;
                    }
                }
            }
        }
    }
    return true;
}

IWizardPage* CFileLoadWizard::x_GetNextPage()
{
	int format = m_FileSelectPanel->GetSelectedFormat();
    IFileLoadPanelClient& manager = *m_FormatManagers[format];
	IToolWizard& wizard = dynamic_cast<IToolWizard&>(manager);
	return wizard.GetFirstPage();
}

IWizardPage* CFileLoadWizard::x_GetOptionsPage()
{
	int format = m_FileSelectPanel->GetSelectedFormat();
    IFileLoadPanelClient& manager = *m_FormatManagers[format];
	IToolWizard& wizard = dynamic_cast<IToolWizard&>(manager);
	return wizard.GetOptionsPage();
}

CFileLoadPanel* CFileLoadWizard::x_GetFileSelectPanel()
{
    if (!m_FileSelectPanel) {
        m_FileSelectPanel = new CFileLoadPanel(m_ParentWindow);
        m_FileSelectPanel->SetWorkDir(m_WorkDir);
        m_FileSelectPanel->SetManager(this);

        if(m_Filenames.empty()) {
            // select default format
            m_CurrFormat = std::max(0, m_CurrFormat); // do not allow -1
            m_FileSelectPanel->SelectFormat(m_CurrFormat);
        } else {
            // have files - autoguess
            m_FileSelectPanel->SetFilenames(m_Filenames);
        }
		OnFormatChanged(m_CurrFormat);
    }
    return m_FileSelectPanel;
}


void CFileLoadWizard::SetRegistryPath(const string& path)
{
    m_RegPath = path; // store for later use
    m_RegPath += "2"; // changed contents of MRU section JIRA: GB-1658

    for(  size_t i = 0;  i < m_FormatManagers.size();  i++ )  {
		IFileLoadPanelClient& manager = *m_FormatManagers[i];
        IRegSettings* rgs = dynamic_cast<IRegSettings*>(&manager);
        if (rgs) {
            string mgr_path = m_RegPath + "." + manager.GetFileLoaderId();
            rgs->SetRegistryPath(mgr_path);
        }
    }
}

static const char* kSelectedFormatTag = "SelectedFormat";
static const char* kMRUTag = "MRUFiles";

void CFileLoadWizard::SaveSettings() const
{
    if (m_RegPath.empty())
		return;

    CGuiRegistry& gui_reg = CGuiRegistry::GetInstance();
    CRegistryWriteView view = gui_reg.GetWriteView(m_RegPath);

    string selectedLabel;
	if (m_CurrFormat != -1) {
        const IFileLoadPanelClient& manager = *m_FormatManagers[m_CurrFormat];
		selectedLabel = manager.GetLabel();
    }
    view.Set(kSelectedFormatTag, selectedLabel);

    // save MRU Files
    vector<string> values;
	m_FileMRUList.SaveToStrings(values);
    view.Set(kMRUTag, values);

    // save Managers' settigns
    for (size_t i = 0; i < m_FormatManagers.size(); i++) {
        const IRegSettings* rgs = dynamic_cast<const IRegSettings*>(m_FormatManagers[i].GetPointer());
        if (rgs)
            rgs->SaveSettings();
    }
}

void CFileLoadWizard::LoadSettings()
{
	if (m_RegPath.empty())
		return;

    CGuiRegistry& gui_reg = CGuiRegistry::GetInstance();
    CRegistryReadView view = gui_reg.GetReadView(m_RegPath);

    // load the default option
    string sel_fmt_label = view.GetString(kSelectedFormatTag, "empty");

    // load MRU Files
    vector<string> values;
    view.GetStringVec(kMRUTag, values);
	m_FileMRUList.LoadFromStrings(values);

    for (size_t i = 0; i < m_FormatManagers.size(); i++) {
        IFileLoadPanelClient& manager = *m_FormatManagers[i];
        // load settings (if supported)
        IRegSettings* rgs = dynamic_cast<IRegSettings*>(&manager);
        if(rgs)
            rgs->LoadSettings();

        // check whether this is the selected manager
        string mgr_label = manager.GetLabel();
        if (mgr_label == sel_fmt_label)
            m_CurrFormat = (int)i;
    }
}


END_NCBI_SCOPE
