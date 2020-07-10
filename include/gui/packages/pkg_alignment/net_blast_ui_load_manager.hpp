#ifndef PKG_ALIGNMENT___NET_BLAST_UI_LOAD_MANAGER__HPP
#define PKG_ALIGNMENT___NET_BLAST_UI_LOAD_MANAGER__HPP

/*  $Id: net_blast_ui_load_manager.hpp 38477 2017-05-15 21:10:59Z evgeniev $
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
 *
 */

#include <corelib/ncbistl.hpp>
#include <corelib/ncbiobj.hpp>

#include <gui/gui_export.h>

#include <gui/packages/pkg_alignment/net_blast_ui_data_source.hpp>

#include <gui/core/ui_tool_manager.hpp>
#include <gui/core/project_selector_panel.hpp>

#include <gui/objutils/reg_settings.hpp>

#include <gui/utils/extension.hpp>
#include <gui/utils/ui_object.hpp>


BEGIN_NCBI_SCOPE

/** @addtogroup GUI_PKG_ALIGNMENT
 *
 * @{
 */

class CNetBLASTLoadOptionPanel;

#define NET_BLAST_LOADER_LABEL "RIDs from NCBI Net BLAST"

///////////////////////////////////////////////////////////////////////////////
/// CNetBLASTUILoadManager
class  CNetBLASTUILoadManager :
    public CObject,
    public IUIToolManager,
    public IRegSettings
{
public:
    CNetBLASTUILoadManager(CNetBLASTUIDataSource& data_source);

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
    virtual bool    DoTransition(EAction action);
    virtual IAppTask*      GetTask();
    /// @}

    /// @name IRegSettings interface implementation
    /// @{
    virtual void    SetRegistryPath(const string& path);
    virtual void    LoadSettings();
    virtual void    SaveSettings() const;
    /// @}

protected:
    enum EState {
        eInvalid = -1,
        eSelectRID,
        eSelectProject,
        eCompleted
    };

protected:
    CRef<CNetBLASTUIDataSource> m_DataSource;

    IServiceLocator* m_SrvLocator;
    wxWindow*   m_ParentWindow;

    CUIObject m_Descriptor;
    EState  m_State;

    string  m_RegPath;
    mutable string  m_SavedInput; // input saved in the registry

    CNetBLASTLoadOptionPanel*   m_OptionPanel;
    CProjectSelectorPanel*   m_ProjectSelPanel;
    SProjectSelectorParams   m_ProjectParams; //TODO
};


/* @} */

END_NCBI_SCOPE;


#endif  // PKG_ALIGNMENT___NET_BLAST_UI_LOAD_MANAGER__HPP

