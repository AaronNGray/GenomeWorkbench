#ifndef PKG_ALIGNMENT___BLAST_TOOL_MANAGER_BASE__HPP
#define PKG_ALIGNMENT___BLAST_TOOL_MANAGER_BASE__HPP

/*  $Id: blast_tool_manager.hpp 38477 2017-05-15 21:10:59Z evgeniev $
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
 *      A base class for BLAST Tool managers.
 */

#include <corelib/ncbistl.hpp>
#include <corelib/ncbiobj.hpp>

#include <gui/gui_export.h>

#include <gui/packages/pkg_alignment/blast_search_params.hpp>
#include <gui/packages/pkg_alignment/net_blast_ui_data_source.hpp>

#include <gui/core/ui_tool_manager.hpp>
#include <gui/core/project_selector_panel.hpp>

#include <gui/objutils/reg_settings.hpp>

#include <gui/utils/extension.hpp>


BEGIN_NCBI_SCOPE

/** @addtogroup GUI_PKG_ALIGNMENT
 *
 * @{
 */

class CBLASTSearchOptionsPanel;
class CBLASTSearchParamsPanel;
class CProjectSelectorPanel;

///////////////////////////////////////////////////////////////////////////////
/// CBLASTToolManager
class  CBLASTToolManager :
    public CObject,
    public IUIAlgoToolManager,
    public IExtension,
    public IRegSettings
{
public:
    CBLASTToolManager();

    /// @name IUIToolManager interface implementation
    /// @{
    virtual void    SetServiceLocator(IServiceLocator* srv_locator);
    virtual void    SetParentWindow(wxWindow* parent);
    virtual const IUIObject&  GetDescriptor() const;
    virtual void    InitUI();
    virtual void    CleanUI();
    virtual wxPanel*    GetMaxPanel();
    virtual wxPanel*    GetCurrentPanel();
    virtual bool        CanDo(EAction action);
    virtual bool        IsFinalState();
    virtual bool        IsCompletedState();
    virtual bool        DoTransition(EAction action);
    virtual IAppTask*   GetTask();
    virtual void        RestoreDefaults();
    /// @}

    /// @name IUIAlgoToolManager interface implementation
    /// @{
    virtual string GetCategory();
    virtual string SetInputObjects(const vector<TConstScopedObjects>& input);

    virtual bool CanQuickLaunch() const;
    virtual IAppTask* QuickLaunch();
    /// @}

    /// @name IExtension interface implementation
    /// @{
    virtual string  GetExtensionIdentifier() const;
    virtual string  GetExtensionLabel() const;
    /// @}

    /// @name IRegSettings interface implementation
    /// @{
    virtual void    SetRegistryPath(const string& path);
    virtual void    LoadSettings();
    virtual void    SaveSettings() const;
    /// @}

protected:
    enum EState {
        eInvalid = -1, // initial empty state
        eOptions, // first page - input objects, program, database
        eParams,  // second page - parameters
        eCompleted  // completed state - all parameters have been gathered and validated
    };

protected:
    CRef<CNetBLASTUIDataSource>     GetBLASTDataSource();

    virtual void    x_SelectCompatibleInputObjects();
    virtual bool    x_IsNucInput() const;

    virtual void    x_CreateOptionsPanelIfNeeded();
    virtual void    x_CreateParamsPanelIfNeeded();

    void    x_InitProjectParams();

    bool    x_ValidateParams();

protected:
    /// describes the Manager's UI properties
    CUIObject   m_Descriptor;

    IServiceLocator* m_SrvLocator;
    wxWindow*   m_ParentWindow;

    string  m_RegPath;
    string m_LocalWinMaskDirPath;

    vector<TConstScopedObjects>    m_InputObjects;

    map<string, TConstScopedObjects> m_NucObjects;
    map<string, TConstScopedObjects> m_ProtObjects;

    // when input contains both Nuc and Prot sequences - select Nuc option
    mutable bool m_UseNucForMixInput;

    CBLASTParams  m_Params;

    EState  m_State;

    CBLASTSearchOptionsPanel*   m_OptionsPanel; // first page
    CBLASTSearchParamsPanel*    m_ParamsPanel;  // second page
    SProjectSelectorParams   m_ProjectParams; //TODO
};

/* @} */

END_NCBI_SCOPE

#endif  // PKG_ALIGNMENT___BLAST_TOOL_MANAGER_BASE__HPP

