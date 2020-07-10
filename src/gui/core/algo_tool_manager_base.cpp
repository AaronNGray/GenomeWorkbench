/*  $Id: algo_tool_manager_base.cpp 40050 2017-12-13 18:32:01Z katargir $
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

#include <gui/core/algo_tool_manager_base.hpp>

#include <gui/core/project_service.hpp>
#include <gui/core/loading_app_job.hpp>
#include <gui/core/document.hpp>
#include <gui/objutils/execute_lock_guard.hpp>

#include <gui/framework/app_job_task.hpp>

#include <gui/widgets/wx/message_box.hpp>

#include <gui/objutils/registry.hpp>

#include <gui/utils/extension_impl.hpp>

#include <gui/core/async_obj_convert.hpp>

#include <wx/panel.h>
#include <wx/msgdlg.h>

BEGIN_NCBI_SCOPE
USING_SCOPE(objects);

///////////////////////////////////////////////////////////////////////////////
/// CAlgoToolManagerBase
CAlgoToolManagerBase::CAlgoToolManagerBase(const string& label,
                                           const string& icon_alias,
                                           const string& hint,
                                           const string& description,
                                           const string& help_id,
                                           const string& category)
:   m_Descriptor(label, icon_alias, hint, description, help_id, "tools"),
    m_Category(category),
    m_SrvLocator(NULL),
    m_ParentWindow(NULL),
    m_State(eInvalid),
    m_ProjectSelPanel(NULL)
{
}


void CAlgoToolManagerBase::SetServiceLocator(IServiceLocator* srv_locator)
{
    m_SrvLocator = srv_locator;
}


void CAlgoToolManagerBase::SetParentWindow(wxWindow* parent)
{
    m_ParentWindow = parent;
}


const IUIObject& CAlgoToolManagerBase::GetDescriptor() const
{
    return m_Descriptor;
}


/// override this function in a derived class and initialize extra members
void CAlgoToolManagerBase::InitUI()
{
    // initially tool panels are not displayed and so the first
    // step of the Run Tool dialog corresponds to invalid state
    m_State = eInvalid;
    m_InputObjects.clear();

    m_ProjectSelPanel = NULL;
}


/// override this function in a derived class and clean extra members
void CAlgoToolManagerBase::CleanUI()
{
    m_State = eInvalid;
    m_InputObjects.clear();

    m_ProjectSelPanel = NULL;
}


wxPanel* CAlgoToolManagerBase::GetCurrentPanel()
{
    switch(m_State) {
    case eParams:
        return x_GetParamsPanel();
    case eSelectProject:
        return m_ProjectSelPanel;
    default:
        return NULL;
    }
}

string CAlgoToolManagerBase::x_ValidateInputObjects()
{
    if (!UsesSingleOMScope())
        return "";

    CRef<objects::CScope> scope;
    ITERATE(vector<TConstScopedObjects>, it, m_InputObjects) {
        ITERATE(TConstScopedObjects, it2, *it) {
            if (!scope) {
                scope = it2->scope;
                continue;
            } else {
                if (scope != it2->scope) {
                    return "This tool gives unpredictable results with objects\nfrom different projects.\n\nPlease move all objects to the same project\nand try again.";
                }
            }
        }
    }

    return "";
}

/* Example - how to implement this function
void CAlgoToolManagerBase::x_CreateParamsPanelIfNeeded()
{
    if(m_ParamsPanel == NULL)   {
        x_SelectCompatibleInputObjects();

        m_ParamsPanel = new CMergeAlignmentsParamsPanel();
        m_ParamsPanel->Hide(); // to reduce flicker
        m_ParamsPanel->Create(m_ParentWindow);
        m_ParamsPanel->SetParams(&m_Params);

        m_ParamsPanel->SetRegistryPath(m_RegPath + ".ParamsPanel");
        m_ParamsPanel->LoadSettings();
    }
}
*/

void CAlgoToolManagerBase::x_ConvertInputObjects (
    const CTypeInfo* typeInfo, map<string, TConstScopedObjects>& results)
{
    NON_CONST_ITERATE(vector<TConstScopedObjects>, it, m_InputObjects) {
        if (!it->empty()) {
            AsyncConvertObjects(typeInfo, *it, results);
            if (!results.empty())
                return;
        }
    }
}


void CAlgoToolManagerBase::x_ConvertInputObjects(
    const CTypeInfo* typeInfo, TConstScopedObjects& results)
{
    NON_CONST_ITERATE(vector<TConstScopedObjects>, it, m_InputObjects) {
        if (!it->empty()) {
            AsyncConvertObjects(typeInfo, *it, results);
            if (!results.empty())
                return;
        }
    }
}

ILocker* CAlgoToolManagerBase::x_GetDataLocker()
{
    set<const CScope*> scopes;

    ITERATE(vector<TConstScopedObjects>, it, m_InputObjects) {
        ITERATE(TConstScopedObjects, it2, *it) {
            const CScope* scope = it2->scope.GetPointerOrNull();
            if (scope) scopes.insert(scope);
        }
    }

    if (scopes.size() != 1)
        return 0;

    const CScope* scope = *scopes.begin();

    CIRef<CProjectService> srv = m_SrvLocator->GetServiceByType<CProjectService>();
    if (!srv) return 0;

    CRef<CGBWorkspace> ws = srv->GetGBWorkspace();
    if (!ws) return 0;

    CGBDocument* doc = dynamic_cast<CGBDocument*>(ws->GetProjectFromScope(*scope));
    if (!doc) return 0;

    ICommandProccessor* cmdProcessor = &doc->GetUndoManager();
    if (!cmdProcessor) return 0;

    return new CWeakExecuteGuard(*cmdProcessor);
}

void CAlgoToolManagerBase::x_CreateProjectPanelIfNeeded()
{
    if (UsesSingleOMScope()) {
        x_InitProjectParams();
    }
    else if(m_ProjectSelPanel == NULL)   {
        CIRef<CProjectService> srv = m_SrvLocator->GetServiceByType<CProjectService>();

        /// initialize project params, x_InitProjectParams can be overridden
        // in derived classes to select projects by input
        x_InitProjectParams();

        m_ProjectSelPanel = new CProjectSelectorPanel(m_ParentWindow);

        m_ProjectSelPanel->SetProjectService(srv);
        m_ProjectSelPanel->SetParams(m_ProjectParams);
    }
}


bool CAlgoToolManagerBase::CanDo(EAction action)
{
    switch( m_State ){
    case eInvalid:
        return action == eNext;
    case eParams:
        return action == eNext;
    case eSelectProject:
        return action == eNext ||  action == eBack;
    case eCompleted:
        return false; // nothing left to do
    default:
        _ASSERT(false);
        return false;
    }
}


bool CAlgoToolManagerBase::IsFinalState()
{

    return UsesSingleOMScope() ? m_State == eParams : m_State == eSelectProject;
}


bool CAlgoToolManagerBase::IsCompletedState()
{
    return m_State == eCompleted;
}


bool CAlgoToolManagerBase::DoTransition(EAction action)
{
    bool next = action == eNext;

    if(m_State == eInvalid  &&  next)    {
        /// initial state - display Parameters panel
        m_State = eParams;
        x_CreateParamsPanelIfNeeded();
        x_GetParamsPanel()->TransferDataToWindow();
        return true;
     } else if(m_State == eParams) {
        if(next)    {
            /// if input is valid in Parameters panel - display Projects Selection panel
            wxPanel& panel = *x_GetParamsPanel();
            if (panel.Validate() && panel.TransferDataFromWindow()) {
                try{
                    if(x_ValidateParams())  {
                        x_CreateProjectPanelIfNeeded();
                        // NOTE: The following code was changed to allow prototyping
                        // of editing tools that act on existing data and do not
                        // create new data objects (and therefore do not need to
                        // select a project for new data objects.
                        // The editing tool that inherits from CAlgoToolManagerBase
                        // overrides x_CreateProjectPanelIfNeeded with a function
                        // that does nothing (and therefore does not create m_ProjectSelPanel.
                        // The assumption is that if m_ProjectSelPanel is NULL then the
                        // state should be advanced to eCompleted.
                        // There is a minor problem in that the button at the bottom of the
                        // tool window will say "Next" when it should say "Finish", as the
                        // params panel is the last step before the tool acts on the data.
                        // This has been deemed acceptable for prototyping purposes.
                        // The original code looked like this:
#if 0
                        m_ProjectSelPanel->TransferDataToWindow();
                        m_State = eSelectProject;
#endif
                        // The new code begins here.
                        if (m_ProjectSelPanel) {
                            m_ProjectSelPanel->TransferDataToWindow();
                            m_State = eSelectProject;
                        } else {
                            m_State = eCompleted;
                        }
                        // The new code ends here.
                        return true;
                    } else return false;
                } catch (CException& e) {
                    LOG_POST(e.ReportAll());
                    NcbiErrorBox(e.GetMsg());
                }
            }
        } else {
            m_State = eInvalid;
            return true;
        }
        return false;
    } else if(m_State == eSelectProject) {
        if(next)    {
            if(m_ProjectSelPanel->TransferDataFromWindow()) {
                m_State = eCompleted;
                return true;
            }
        } else {
            m_State = eParams;
            x_GetParamsPanel()->TransferDataToWindow();
            return true;
        }
        return false;
    }

    _ASSERT(false);
    return false;
}


void CAlgoToolManagerBase::x_InitProjectParams()
{
    CIRef<CProjectService> srv = m_SrvLocator->GetServiceByType<CProjectService>();
    NON_CONST_ITERATE(vector<TConstScopedObjects>, it, m_InputObjects) {
        if (!it->empty()) {
            m_ProjectParams.SelectProjectByObjects(*it, srv);
            m_ProjectParams.m_FolderName = "Tool Results";
            m_ProjectParams.m_CreateFolder = true;
            break;
        }
    }
}


/// validate parameters after Params page
/// override this function in derived classes
bool CAlgoToolManagerBase::x_ValidateParams()
{
    return true;
}


IAppTask* CAlgoToolManagerBase::GetTask()
{
    CRef<CDataLoadingAppJob> job(x_CreateLoadingJob());
    job->SetDataLocker(x_GetDataLocker());

    if (m_ProjectSelPanel)
        m_ProjectSelPanel->GetParams(m_ProjectParams);
    CSelectProjectOptions options;
    m_ProjectParams.ToLoadingOptions(options);

    CIRef<CProjectService> srv = m_SrvLocator->GetServiceByType<CProjectService>();
    CRef<CDataLoadingAppTask> task(new CDataLoadingAppTask(srv, options, *job));
    return task.Release();
}

void CAlgoToolManagerBase::RestoreDefaults()
{
    wxPanel* current_panel = GetCurrentPanel();
    if (!current_panel)
        return;
    CAlgoToolManagerParamsPanel* params_panel = dynamic_cast<CAlgoToolManagerParamsPanel*>(current_panel);
    if(!params_panel)
        return;
    if (wxOK != wxMessageBox("The active page settings will be restored to their original defaults.", "Confirm", wxOK | wxCANCEL))
        return;
    params_panel->RestoreDefaults();
}

string CAlgoToolManagerBase::GetCategory()
{
    return m_Category;
}

void CAlgoToolManagerBase::x_SetInputObjects( const vector<TConstScopedObjects>& objects )
{
    m_InputObjects = objects;
}

string CAlgoToolManagerBase::SetInputObjects( const vector<TConstScopedObjects>& objects )
{
    x_SetInputObjects( objects );

    return x_ValidateInputObjects();
}

static const char* kParamsSection = ".Params";

void CAlgoToolManagerBase::SetRegistryPath(const string& path)
{
    m_RegPath = path; // store for later use

    IRegSettings* params = x_GetParamsAsRegSetting();
    if(params)  {
        string params_path = m_RegPath + kParamsSection;
        params->SetRegistryPath(params_path);
    }
}


void CAlgoToolManagerBase::SaveSettings() const
{
    if( ! m_RegPath.empty())   {
        string path = m_RegPath + kParamsSection;
        // save Tool Parameters
        CAlgoToolManagerBase* nc_this = const_cast<CAlgoToolManagerBase*>(this);

        IRegSettings* params = nc_this->x_GetParamsAsRegSetting();
        if(params)  {
            params->SaveSettings();
        }

        // save UI state and settings
        CAlgoToolManagerParamsPanel* params_panel = nc_this->x_GetParamsPanel();
        if(params_panel)   {
            params_panel->SaveSettings();
        }
    }
}


void CAlgoToolManagerBase::LoadSettings()
{
    if( ! m_RegPath.empty())   {
        string path = m_RegPath + kParamsSection;

        // restore Tool Parameters
        IRegSettings* params = x_GetParamsAsRegSetting();
        if(params)  {
            params->LoadSettings();
        }

        // restore UI state and settings
        CAlgoToolManagerParamsPanel* params_panel = x_GetParamsPanel();
        if(params_panel)   {
            params_panel->LoadSettings();
        }
    }
}


////////////////////////////////////////////////////////////////////////////////
/// CAlgoToolManagerParamsPanel

void CAlgoToolManagerParamsPanel::SetRegistryPath(const string& reg_path)
{
    m_RegPath = reg_path;
}


END_NCBI_SCOPE
