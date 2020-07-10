/*  $Id: ui_file_load_manager.cpp 39666 2017-10-25 16:01:13Z katargir $
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

#include <gui/core/ui_file_load_manager.hpp>

#include <gui/core/file_load_option_panel.hpp>
#include <gui/core/file_auto_loader.hpp>
#include <gui/core/open_dlg.hpp>
#include <gui/core/object_loading_task.hpp>
#include <gui/core/loading_app_job.hpp>

#include <gui/widgets/wx/compressed_file.hpp>
#include <gui/widgets/wx/message_box.hpp>
#include <gui/core/ui_data_source_service.hpp>

#include <gui/objutils/registry.hpp>

#include <gui/utils/extension_impl.hpp>
#include <gui/utils/execute_unit.hpp>

#include <gui/widgets/wx/wx_utils.hpp>
#include <gui/widgets/wx/wizard_dlg.hpp>

#include <util/format_guess.hpp>

#include <wx/msgdlg.h>
#include <wx/filename.h>

BEGIN_NCBI_SCOPE

///////////////////////////////////////////////////////////////////////////////
/// EXTENSION_POINT__UI_DATA_SOURCE_TYPE Extension point declaration

static CExtensionPointDeclaration
    sDSTypeExtPoint("file_format_loader_manager",
                    "File Format Loader Manager");

///////////////////////////////////////////////////////////////////////////////
/// CFileLoadManager
CFileLoadManager::CFileLoadManager()
:   m_Descriptor("File Import", ""),
    m_OpenDlg(NULL),
    m_SrvLocator(NULL),
    m_ParentWindow(NULL),
    m_State(eInvalid),
    m_CurrFormat(-1),
    m_CheckFormat(true),
    m_AltToolManager(NULL),
    m_OptionPanel(NULL),
    m_ProjectSelPanel(NULL),
    m_FilePage(*this),
    m_OpenObjectsPage()
{
}

wxPanel* CFileLoadManager::CFilePage::GetPanel()
{
    return m_Manager.x_GetOptionsPanel();
}

void CFileLoadManager::LoadRegisteredManagers()
{
    // predefined system formats
    m_AutodetectFormat.Reset(new CFileAutoLoader());
    AddFileFormat(m_AutodetectFormat.GetPointer());

    // get all other file format from an Extension Point
    vector< CIRef<IFileFormatLoaderManager> > managers;
    GetExtensionAsInterface("file_format_loader_manager", managers);

    for(  size_t i = 0;  i < managers.size();  ++i  )   {
        IFileFormatLoaderManager* mgr = managers[i].GetPointer();
        AddFileFormat(mgr);
    }
}


void CFileLoadManager::SetDialog(COpenDlg* dlg)
{
    m_OpenDlg = dlg;
}


void CFileLoadManager::SetServiceLocator(IServiceLocator* srv_locator)
{
    m_SrvLocator = srv_locator;
}


void CFileLoadManager::SetParentWindow(wxWindow* parent)
{
    m_ParentWindow = parent;
}


const IUIObject& CFileLoadManager::GetDescriptor() const
{
    if (m_CurrFormat > -1)   {
        TFormatMgrRef manager = m_FormatManagers[m_CurrFormat];
        return manager->GetDescriptor();
    }

    return m_Descriptor;
}


void CFileLoadManager::InitUI()
{
    m_State = eSelectFiles;
    if (m_OptionPanel)
        m_OptionPanel->OnActivatePanel();

    for( size_t i = 0;  i < m_FormatManagers.size();  i++)  {
        IFileFormatLoaderManager& manager = *m_FormatManagers[i];
        manager.SetServiceLocator(m_SrvLocator);
        manager.SetParentWindow(m_ParentWindow);
        manager.InitUI();
    }
}


void CFileLoadManager::CleanUI()
{
    for( size_t i = 0;  i < m_FormatManagers.size();  i++)  {
        IFileFormatLoaderManager& manager = *m_FormatManagers[i];
        manager.CleanUI();
        manager.SetServiceLocator(NULL);
    }

    m_CurrFormat = -1;
    m_OptionPanel = NULL;
    m_ProjectSelPanel = NULL;
    m_FilePage.Reset();
}


void CFileLoadManager::AddFileFormat(IFileFormatLoaderManager* manager)
{
    _ASSERT(manager);
    if(manager) {
        m_FormatManagers.push_back(TFormatMgrRef(manager));
    }
}


void CFileLoadManager::SetFilenames(vector<wxString>& filenames)
{
    m_Filenames = filenames;

    if(m_OptionPanel)   {
        m_OptionPanel->SetFilenames(m_Filenames);
        x_AutoGuessFormat();
    }
}


/// called by File Format panel when a user selects a format
void CFileLoadManager::OnFormatChanged()
{
    _ASSERT(m_State == eSelectFiles);
    m_CurrFormat = m_OptionPanel->GetSelectedFormat();
    x_UpdateWizardPages();
}


wxPanel* CFileLoadManager::GetCurrentPanel()
{
    if(m_State == eSelectFiles)   {
        return x_GetOptionsPanel();
    } else if(m_State == eFormatDefined)    {
        // in this state the panel is provided by the current Format Load Manager
        TFormatMgrRef manager = m_FormatManagers[m_CurrFormat];
        return manager->GetCurrentPanel();
    } else if(m_State == eSelectProjects)   {
        if(m_ProjectSelPanel == NULL)   {            
            CIRef<CProjectService> srv = m_SrvLocator->GetServiceByType<CProjectService>();

            m_ProjectSelPanel = new CProjectSelectorPanel(m_ParentWindow);
            m_ProjectSelPanel->SetProjectService(srv);
            m_ProjectSelPanel->SetParams(m_ProjectParams);
            m_ProjectSelPanel->TransferDataToWindow();
        }
        return m_ProjectSelPanel;
    }
    return NULL;
}


/*
        bool can = false;

        if(action == eNext) {
            // we enable this transition only if the selected file format
            // manager can handle the selected files
            TFormatMgrRef manager = m_FormatManagers[m_CurrFormat];

            vector<string> filenames;
            m_OptionPanel->GetFilenames(filenames);

            if(filenames.empty())   {
                NcbiErrorBox("Please select at least one file!");
            } else {
                can = manager->ValidateFilenames(filenames);
            }
        }
        return can;
*/

bool CFileLoadManager::CanDo(EAction action)
{
    switch(m_State) {
    case eSelectFiles:
        return action == eNext;

    case eFormatDefined: {
        // in this state all decisions are made by the current Format Manager
        TFormatMgrRef manager = m_FormatManagers[m_CurrFormat];
        if(manager->IsInitialState())   {
            return true;
        } else {
            return manager->CanDo(action);
        }
    }
    case eSelectProjects:
        return action == eNext  ||  action == eBack;

    case eCompleted:
        return false;

    default:
        _ASSERT(false);
        return false;
    }
}


bool CFileLoadManager::IsFinalState()
{
    return m_State == eSelectProjects;
}


bool CFileLoadManager::IsCompletedState()
{
    return m_State == eCompleted;
}

static const wxFormatString kFORMAT_CONFLICT_MSG = "Detected data format '%s' doesn't appear to match the format you selected.\n"
        "Are you sure you want to load your data as '%s'?\nClick 'Yes' to proceed or click 'No' to process the data as '%s'.";

bool CFileLoadManager::x_CheckFormatConflict(const vector<wxString>& filenames, TFormatMgrRef& manager)
{
    if (m_CurrFormat == 0) // AutoDetect
        return true;
    if (manager->CanGuessFormat() == false)
        return true;
    try { 
        CFileAutoLoader auto_loader;
        CFormatGuess::EFormat fmt = auto_loader.SetFormatManager(filenames);
        IFileFormatLoaderManager* detected_manager = auto_loader.GetCurrentManager();
        if (detected_manager != 0 && detected_manager != manager) {
            const char* format_name = CFormatGuess::GetFormatName(fmt);
            wxString detected_frm = format_name ? ToWxString(format_name ) : wxT("No name");
            wxString selected_frm;
            m_OptionPanel->GetSelectedFormat(selected_frm);
            int resp = NcbiMessageBoxW(wxString::Format(kFORMAT_CONFLICT_MSG, detected_frm, selected_frm, detected_frm), 
                eDialog_YesNoCancel, eIcon_Exclamation, wxT("Warning"));
            if (resp == eCancel)
                return false;
            if ( resp == eNo) {
                m_CurrFormat = -1;
                manager.Reset(detected_manager);
                for( size_t i = 0;  i < m_FormatManagers.size();  i++)  {
                    if (manager == m_FormatManagers[i]) {
                        m_CurrFormat = i;
                        m_OptionPanel->SelectFormat(m_CurrFormat);
                        break;
                    }
                }

            } 
        }
    } catch (...) {
    }
    return true;
}


// transition to a new state specified by the given action
bool CFileLoadManager::DoTransition(EAction action)
{
    m_AltToolManager = NULL;

    if(m_State == eSelectFiles  &&  action == eNext)    {
        vector<wxString> filenames;
        m_OptionPanel->GetFilenames(filenames);

        ITERATE(vector<wxString>, it,  filenames) {
            if (!CCompressedFile::FileExists(*it)) {
                wxMessageBox(wxT("Wrong file name or file doesn't exist:\n") + *it,
                             wxT("File Open Error"),wxICON_ERROR | wxOK);
                return false;
            }
        }
        if (filenames.empty())   {
            NcbiErrorBox("Please select at least one file!");
            return false;
        } 

        m_CurrFormat = m_OptionPanel->GetSelectedFormat();
        // select Format Manager for the format
        TFormatMgrRef manager = m_FormatManagers[m_CurrFormat];

        if (m_OptionPanel->GetCheckFormat() && !x_CheckFormatConflict(filenames, manager))
            return false;
        if (!manager->ValidateFilenames(filenames)) 
            return false;

        string id = manager->GetFileLoaderId();
        if (id == "file_loader_auto") {
            CFileAutoLoader* autoLoader = dynamic_cast<CFileAutoLoader*>(manager.GetPointer());
            if (autoLoader) {
                IFileFormatLoaderManager* curManager = autoLoader->GetCurrentManager();
                if (curManager) {
                    id = curManager->GetFileLoaderId();
                }
            }
        }

        if (id == "file_loader_gbench_project") {
             // Project and Workspace have no parameters nor do they need Select Project page
            manager->SetFilenames(filenames);
            m_State = eCompleted;
        }
        else if(manager->IsCompletedState()) {
            manager->SetFilenames(filenames);
            m_State = eSelectProjects; // skip this manager
        } else {
            manager->SetFilenames(filenames);
            m_State = eFormatDefined;
        }
        return true;
    } else if(m_State == eFormatDefined) {
        TFormatMgrRef manager = m_FormatManagers[m_CurrFormat];

        if(action == eBack && manager->IsInitialState())  {
            m_State = eSelectFiles;
            if (m_OptionPanel)
                m_OptionPanel->OnActivatePanel();

        } else {
            manager->DoTransition(action);

            if(action == eNext && manager->IsCompletedState()) {
                m_State = eSelectProjects;
            }
            // otherwise remain in eFormatDefined
        }
        return true;
    } else if(m_State == eSelectProjects) {
        if(action == eBack)  {
            TFormatMgrRef manager = m_FormatManagers[m_CurrFormat];
            if(manager->IsInitialState()) {
                m_State = eSelectFiles;
                if (m_OptionPanel)
                    m_OptionPanel->OnActivatePanel();
            } else {
                m_State = eFormatDefined;
                manager->DoTransition(action);
            }
            return true;
        } else if(action == eNext)  {
            if(m_ProjectSelPanel->TransferDataFromWindow()) {
                m_ProjectSelPanel->GetParams(m_ProjectParams);
                m_State = eCompleted;
                return true;
            }
        }
    }
    _ASSERT(false);
    return false;
}


IAppTask* CFileLoadManager::GetTask()
{
    m_CurrFormat = m_OptionPanel->GetSelectedFormat();
    if(m_CurrFormat > -1)   {
        TFormatMgrRef manager = m_FormatManagers[m_CurrFormat];

        vector<wxString> filenames;
        manager->GetFilenames(filenames);
        string id = manager->GetFileLoaderId();
        wxString label = ToWxString(manager->GetDescriptor().GetLabel());

        ITERATE(vector<wxString>, it, filenames)
            m_FileMRUList.Add(CFileDescriptor(*it, label, id));

        CIRef<IObjectLoader> loader(dynamic_cast<IObjectLoader*>(manager->GetExecuteUnit()));
        if (loader) {
            CIRef<CProjectService> srv = m_SrvLocator->GetServiceByType<CProjectService>();
            CSelectProjectOptions options;
            m_ProjectParams.ToLoadingOptions(options);
            return new CObjectLoadingTask(srv, *loader, options);
        }
        else {
            IAppTask* task = manager->GetTask();
            if (!task) return 0;

            CDataLoadingAppTask* dlTask = dynamic_cast<CDataLoadingAppTask*>(task);
            if (dlTask) {
                CSelectProjectOptions options;
                m_ProjectParams.ToLoadingOptions(options);
                dlTask->SetOptions(options);
            }
            return task;
        }
    }

    _ASSERT(false); // must not happen
    return NULL;
}

IExecuteUnit* CFileLoadManager::GetExecuteUnit()
{
    IExecuteUnit* execute_unit = 0;
    if(m_CurrFormat > -1) {
        TFormatMgrRef manager = m_FormatManagers[m_CurrFormat];
        execute_unit = manager->GetExecuteUnit();
    }
    return execute_unit;
}

IWizardPage* CFileLoadManager::GetFirstPage()
{
    x_GetOptionsPanel();
    return &m_FilePage;
}

void CFileLoadManager::SetPrevPage(IWizardPage* prevPage)
{
    m_OpenObjectsPage = prevPage;
    x_UpdateWizardPages();
}

void CFileLoadManager::ResetState()
{
    if (m_AutodetectFormat.Empty())
        return;

    CFileAutoLoader* auto_loader = dynamic_cast<CFileAutoLoader*>(m_AutodetectFormat.GetPointer());
    if (nullptr == auto_loader)
        return;

    auto_loader->ResentCurrentManager(); 
}

void CFileLoadManager::x_UpdateWizardPages()
{
    if (m_CurrFormat >= 0) {
        TFormatMgrRef manager = m_FormatManagers[m_CurrFormat];
        manager->SetPrevPage(m_OpenObjectsPage);
        if (m_OpenObjectsPage) {
            m_OpenObjectsPage->SetNextPage(manager->GetFirstPage());
            m_OpenObjectsPage->SetOptionsPage(manager->GetOptionsPage());
        }
    }
    else {
        if (m_OpenObjectsPage) {
            m_OpenObjectsPage->SetNextPage(0);
            m_OpenObjectsPage->SetOptionsPage(0);
        }
    }

    if (m_OpenDlg) {
        m_OpenDlg->Update();
    }
    else if (m_ParentWindow) {
        wxCommandEvent evt(wxEVT_COMMAND_MENU_SELECTED, CWizardDlg::eCmdUpdateButtons);
        m_ParentWindow->ProcessWindowEvent(evt);
    }
}

bool CFileLoadManager::x_CanLeavePage(bool forward)
{
    vector<wxString> filenames;
    m_OptionPanel->GetFilenames(filenames);

    if(filenames.empty()) {
        NcbiErrorBox("Please select at least one file!");
        return false;
    }

    m_CurrFormat = m_OptionPanel->GetSelectedFormat();
    // select Format Manager for the format
    TFormatMgrRef manager = m_FormatManagers[m_CurrFormat];
    if (m_OptionPanel->GetCheckFormat() && !x_CheckFormatConflict(filenames, manager))
        return false;

    if (! manager->ValidateFilenames(filenames)) {
        NcbiErrorBox("The file(s) cannot be opened with this file format manager!");
        return false;
    }

    manager->SetFilenames(filenames);
    manager->GetFilenames(filenames);
    string id = manager->GetFileLoaderId();
    wxString label = ToWxString(manager->GetDescriptor().GetLabel());

    ITERATE(vector<wxString>, it, filenames)
        m_FileMRUList.Add(CFileDescriptor(*it, label, id));

    return true;
}

void CFileLoadManager::x_AutoGuessFormat()
{
    _ASSERT(m_OptionPanel);

    if( ! m_Filenames.empty())    {
        for(  size_t i = 0;  i < m_FormatManagers.size();  i++ )    {
            IFileFormatLoaderManager& manager = *m_FormatManagers[i];

            bool ok = true;
            for(  size_t j = 0;  ok  &&  j < m_Filenames.size();  j++ )   {
                const wxString& name = m_Filenames[j];
                ok = manager.RecognizeFormat(name);
            }
            if(ok)  {  // manager recognized all files
                m_CurrFormat = (int)i;
                m_OptionPanel->SelectFormat(m_CurrFormat);
                return;
            }
        }
    }
}

CFileLoadOptionPanel* CFileLoadManager::x_GetOptionsPanel()
{
    if (!m_OptionPanel) {
        // create format descriptors
        vector<CFileLoadOptionPanel::CFormatDescriptor> formats;

        for( size_t i = 0;  i < m_FormatManagers.size();  i++ ) {
            IFileFormatLoaderManager& manager = *m_FormatManagers[i];
            const IUIObject& ui_obj = manager.GetDescriptor();
            string id = manager.GetFileLoaderId();
            wxString label = ToWxString(ui_obj.GetLabel());
            wxString wildcard = manager.GetFormatWildcard();
            bool singleFile = manager.SingleFileLoader();
            // We don't want to overcrowd file format list
            // User can explicitly use "Project or Workspace" loader on the left
            bool hidden = (id == "file_loader_gbench_project");

            formats.push_back(
                CFileLoadOptionPanel::CFormatDescriptor(id, label,
                                                        wildcard,
                                                        singleFile,
                                                        hidden));
        }

        // Create and setup File Loading Panel
        m_OptionPanel = new CFileLoadOptionPanel(m_ParentWindow);
        m_OptionPanel->SetManager(this);
        m_OptionPanel->SetFormats(formats);

        if(m_Filenames.empty()) {
            // select default format
            m_CurrFormat = std::max(0, m_CurrFormat); // do not allow -1
            m_OptionPanel->SelectFormat(m_CurrFormat);
        } else {
            // have files - autoguess
            m_OptionPanel->SetFilenames(m_Filenames);
            x_AutoGuessFormat();
        }

        m_OptionPanel->SetMRU(m_FileMRUList);
        m_OptionPanel->SetCheckFormat(m_CheckFormat);
    }
    return m_OptionPanel;
}


void CFileLoadManager::SetRegistryPath(const string& path)
{
    m_RegPath = path; // store for later use
    m_RegPath += "2"; // changed contents of MRU section JIRA: GB-1658

    for(  size_t i = 0;  i < m_FormatManagers.size();  i++ )  {
        TFormatMgrRef manager = m_FormatManagers[i];
        IRegSettings* rgs = dynamic_cast<IRegSettings*>(manager.GetPointer());
        if(rgs) {
            string mgr_path = m_RegPath + "." + manager->GetFileLoaderId();
            rgs->SetRegistryPath(mgr_path);
        }
    }
}

static const char* kSelectedFormatTag = "SelectedFormat";
static const char* kMRUTag = "MRUFiles";
static const char* kCheckFormatTag = "CheckFormat";


void CFileLoadManager::SaveSettings() const
{
    if( ! m_RegPath.empty())   {
        CGuiRegistry& gui_reg = CGuiRegistry::GetInstance();
        CRegistryWriteView view = gui_reg.GetWriteView(m_RegPath);

        /// remember the selected Format (only if m_OptionPanel exists)
        if(m_OptionPanel)   {
            int format = m_OptionPanel->GetSelectedFormat();
            if(format != -1) {
                TFormatMgrRef manager = m_FormatManagers[format];
                string mgr_label = manager->GetDescriptor().GetLabel();
                view.Set(kSelectedFormatTag, mgr_label);
            }
        }

        // save MRU Files
        vector<string> values;
        // we package times and paths into the same vector (even elements - time, odd - path)
        CTimeFormat format =
            CTimeFormat::GetPredefined(CTimeFormat::eISO8601_DateTimeSec);

        const TMRUPathList::TTimeToTMap& map = m_FileMRUList.GetMap();
        ITERATE(TMRUPathList::TTimeToTMap, it, map)  {
            CTime time(it->first);
            time.ToLocalTime();
            string s_time = time.AsString(format);
            string filename      = FnToStdString(it->second.GetFileName());
            string manager_label = FnToStdString(it->second.GetFileLoaderLabel());
            string manager_id    = it->second.GetFileLoaderId();
            values.push_back(s_time);
            values.push_back(filename);
            values.push_back(manager_label);
            values.push_back(manager_id);
        }
        view.Set(kMRUTag, values);

        view.Set(kCheckFormatTag, m_OptionPanel ? m_OptionPanel->GetCheckFormat() : true);

        // save Managers' settigns
        for(  size_t i = 0;  i < m_FormatManagers.size();  i++ )  {
            TFormatMgrRef manager = m_FormatManagers[i];
            IRegSettings* rgs = dynamic_cast<IRegSettings*>(manager.GetPointer());
            if(rgs) {
                rgs->SaveSettings();
            }
        }
    }
}


void CFileLoadManager::LoadSettings()
{
    if( ! m_RegPath.empty())   {
        CGuiRegistry& gui_reg = CGuiRegistry::GetInstance();
        CRegistryReadView view = gui_reg.GetReadView(m_RegPath);

        // load the default option
        string sel_fmt_label = view.GetString(kSelectedFormatTag, "empty");

        // load MRU Files
        m_FileMRUList.Clear();
        vector<string> values;
        view.GetStringVec(kMRUTag, values);
        CTimeFormat format =
            CTimeFormat::GetPredefined(CTimeFormat::eISO8601_DateTimeSec);

        for(  size_t i = 0;  i + 2 < values.size() ;  )   {
            // process two elements at once
            string s_time = values[i++];
            wxString filename      = FnToWxString(values[i++]);
            wxString manager_label = FnToWxString(values[i++]);
            string manager_id      = values[i++];

            CTime time(s_time, format);

            time_t t = time.GetTimeT();
            m_FileMRUList.Add(CFileDescriptor(filename, manager_label, manager_id), t);
        }

        m_CheckFormat = view.GetBool(kCheckFormatTag, true);

        // load Managers' settigns
        for(  size_t i = 0;  i < m_FormatManagers.size();  i++ )  {
            TFormatMgrRef manager = m_FormatManagers[i];
            // load settings (if supported)
            IRegSettings* rgs = dynamic_cast<IRegSettings*>(manager.GetPointer());
            if(rgs) {
                rgs->LoadSettings();
            }
            // check whether this is the selected manager
            string mgr_label = manager->GetDescriptor().GetLabel();
            if(mgr_label == sel_fmt_label)  {
                m_CurrFormat = (int)i; // found the selected manager
            }
        }
    }
}


END_NCBI_SCOPE
