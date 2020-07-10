#ifndef GUI_CORE___ALGO_TOOL_MANAGER_BASE__HPP
#define GUI_CORE___ALGO_TOOL_MANAGER_BASE__HPP

/*  $Id: algo_tool_manager_base.hpp 38477 2017-05-15 21:10:59Z evgeniev $
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
 * File Description:
 *  CAlgoToolManagerBase - a base class for simple algorithmic tools.
 */

#include <corelib/ncbistl.hpp>

#include <gui/gui_export.h>

#include <gui/core/ui_tool_manager.hpp>
#include <gui/core/project_selector_panel.hpp>
#include <gui/core/loading_app_job.hpp>

#include <gui/objutils/reg_settings.hpp>

#include <gui/utils/extension.hpp>

#include <wx/panel.h>


BEGIN_NCBI_SCOPE

class CAlgoToolManagerParamsPanel;
class CProjectSelectorPanel;

///////////////////////////////////////////////////////////////////////////////
/// CAlgoToolManagerBase
/// This is base class for simple algorithmic tool managers. The class provides
/// two pages to the Run Tool dialog.
///     1. Parameters page is for entering tool parameters; derived from
///        CAlgoToolManagerParamsPanel class.
///     2. Project selection page – the standard page for specifying how
///        results shall be added to the workspace - CProjectSelectorPanel.
/// When “Finish” button is pressed the manager creates a standard
/// CDataLoadingAppTask that executes a job in the background. Derived classed
/// must implement a pure virtual function that creates the job.

class  NCBI_GUICORE_EXPORT  CAlgoToolManagerBase :
    public CObject,
    public IUIAlgoToolManager,
    public IExtension,
    public IRegSettings
{
public:
    CAlgoToolManagerBase(const string& label,
                         const string& icon_alias,
                         const string& hint = kEmptyStr,
                         const string& description = kEmptyStr,
                         const string& help_id = kEmptyStr,
                         const string& category = "Unknown");

    /// @name IUIToolManager interface implementation
    /// @{
    virtual void    SetServiceLocator(IServiceLocator* srv_locator);
    virtual void    SetParentWindow(wxWindow* parent);
    virtual const IUIObject&  GetDescriptor() const;
    virtual void    InitUI();
    virtual void    CleanUI();
    virtual wxPanel*    GetCurrentPanel();
    virtual bool        CanDo(EAction action);
    virtual bool        IsFinalState();
    virtual bool        IsCompletedState();
    virtual bool        DoTransition(EAction action);
    virtual IAppTask*   GetTask();
    virtual void        RestoreDefaults();
    /// @}

    /// @name IUIToolManager interface implementation
    /// @{
    virtual string GetCategory();
    virtual string SetInputObjects( const vector<TConstScopedObjects>& input );

    virtual bool CanQuickLaunch() const { return false; }
    virtual IAppTask* QuickLaunch() { return 0; }
    /// @}

    /// @name IExtension interface - implement in derived classes
    /// @{
    virtual string  GetExtensionIdentifier() const = 0;
    virtual string  GetExtensionLabel() const = 0;
    /// @}

    /// @name IRegSettings interface implementation
    /// @{
    virtual void    SetRegistryPath(const string& path);
    virtual void    LoadSettings();
    virtual void    SaveSettings() const;
    /// @}

protected:
    /// defines states of the Tool Manager
    enum EState {
        eInvalid = -1,  // initial
        eParams,        // displaying parameters panel
        eSelectProject, // displaying target project panel
        eCompleted      // done
    };

protected:

    /// returns a pointer to the parameters panel, override in derived classes
    virtual CAlgoToolManagerParamsPanel*    x_GetParamsPanel() = 0;

    /// return a pointer to Parameters object as IRegSettings interface
    virtual IRegSettings*   x_GetParamsAsRegSetting() = 0;

    /// returns / creates Parameters panel, override in derived classes
    /// see cpp file for example
    virtual void    x_CreateParamsPanelIfNeeded() = 0;

    /// factory method for creating the job that executes the tool algorithm
    /// override in derived classes
    virtual CDataLoadingAppJob*     x_CreateLoadingJob() = 0;

    /// init m_ProjectParams, in particular can select target project based
    /// on the tool input
    virtual void    x_InitProjectParams();

    /// validates user input in Parameters panel, report errors if any
    virtual bool    x_ValidateParams();

    /// returns / creates Project panel
    virtual void    x_CreateProjectPanelIfNeeded();

    virtual void x_SetInputObjects( const vector<TConstScopedObjects>& input );
    virtual string x_ValidateInputObjects();

    void x_ConvertInputObjects(const CTypeInfo* typeInfo, map<string, TConstScopedObjects>& results);
    void x_ConvertInputObjects(const CTypeInfo* typeInfo, TConstScopedObjects& results);

    ILocker* x_GetDataLocker();

protected:
    /// describes the Manager's UI properties
    CUIObject       m_Descriptor;

    /// defines tool category
    const string    m_Category;

    IServiceLocator* m_SrvLocator;

    /// a window that will serve as a parent for our panels
    wxWindow*   m_ParentWindow;

    /// registry path to the settings
    string  m_RegPath;

    /// tool manager state (int the Run Tool wizard)
    EState  m_State;

    /// original input objects, the tool needs to select a subset of objects
    /// that can serve as valid input
    vector<TConstScopedObjects>     m_InputObjects;

    SProjectSelectorParams  m_ProjectParams;
    CProjectSelectorPanel*  m_ProjectSelPanel;

    /// In a derived class one will need to add a data member for holding
    /// tool parameters and member pointing to the panel displaying the
    /// parameters. Parameters class should implement IRegSettings interface
    /// so that parameters could be saved to Registry automatically.

    /// CMyParams       m_Params;
    /// CMyParamsPanel* m_ParamsPanel;
};


////////////////////////////////////////////////////////////////////////////////
/// CAlgoToolManagerParamsPanel
class NCBI_GUICORE_EXPORT CAlgoToolManagerParamsPanel :
    public wxPanel,
    public IRegSettings
{
public:
    /// @name IRegSettings interface implementation
    /// @{
    virtual void SetRegistryPath(const string& reg_path);

    virtual void LoadSettings() = 0;  /// override in derived classes
    virtual void SaveSettings() const = 0;  /// override in derived classes
    /// @}

    /// Restores the default options for the panel
    virtual void RestoreDefaults() = 0;

protected:
    /// registry path to the settings
    string  m_RegPath;
};

END_NCBI_SCOPE


#endif  // GUI_CORE___ALGO_TOOL_MANAGER_BASE__HPP

