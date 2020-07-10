#ifndef GUI_CORE___PROJECT_TREE_VIEW__HPP
#define GUI_CORE___PROJECT_TREE_VIEW__HPP

/*  $Id: project_tree_view.hpp 41875 2018-10-31 17:19:44Z filippov $
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
 *      Project Tree View class.
 */

#include <corelib/ncbistd.hpp>

#include <gui/gui_export.h>

#include <gui/framework/view.hpp>

#include <gui/core/seqloc_search_context.hpp>
#include <gui/core/selection_service.hpp>
#include <gui/core/pt_item.hpp>

#include <gui/widgets/wx/wm_client.hpp>

#include <gui/objutils/reg_settings.hpp>

#include <gui/utils/extension.hpp>

BEGIN_NCBI_SCOPE

BEGIN_SCOPE(objects)
    class CFolderInfo;
END_SCOPE(objects)

class IViewManagerService;
class CAppExplorerDataObject;
class CProjectTreePanel;

///////////////////////////////////////////////////////////////////////////////
/// This Extension Point allows external components to add commands to the
/// top part of Project Tree context menu.
/// The Extensions shall implement IExplorerItemCmdContributor interface.

#define EXT_POINT__PROJECT_TREE_VIEW__CONTEXT_MENU__ITEM_CMD_CONTRIBUTOR \
        "project_tree_view::context_menu::item_cmd_contributor"


///////////////////////////////////////////////////////////////////////////////
/// CProjectTreeView - a system view that displays Project Tree.
///////////////////////////////////////////////////////////////////////////////

class NCBI_GUICORE_EXPORT CProjectTreeView :
    public CObjectEx,
    public CEventHandler,
    public IView,
    public IWMClient,
    public IRegSettings,
    public ISelectionClient,
    public ISeqLocSearchContext
{
    friend class CProjectTreeViewFactory;
    DECLARE_EVENT_MAP();
public:
    CProjectTreeView();
    virtual ~CProjectTreeView();

    /// @name IView interface implementation
    /// @{
    virtual const CViewTypeDescriptor&  GetTypeDescriptor() const;
    virtual void    SetWorkbench(IWorkbench* workbench);
    virtual void    CreateViewWindow(wxWindow* parent);
    virtual void    DestroyViewWindow();
    /// @}

    /// @name IWMClient interface implementation
    /// @{
    virtual wxWindow* GetWindow();
    virtual wxEvtHandler* GetCommandHandler() { return GetWindow()->GetEventHandler(); }
    virtual string GetClientLabel(IWMClient::ELabel ltype) const;
    virtual string GetIconAlias() const;
    virtual const CRgbaColor* GetColor() const;
    virtual void SetColor( const CRgbaColor& color );
    virtual const wxMenu*  GetMenu();
    virtual void UpdateMenu( wxMenu& root_menu );
    virtual CFingerprint GetFingerprint() const;
    /// @}

    /// @name IRegSettings interface implementation
    /// @{
    virtual void    SetRegistryPath(const string& path);
    virtual void    LoadSettings();
    virtual void    SaveSettings() const;
    /// @}

    /// @name ISelectionClient interface implementation
    /// @{
    virtual void    GetSelection(TConstScopedObjects& objects) const;
    virtual void    SetSelectionService(ISelectionService* service);
    virtual string  GetSelClientLabel();
    virtual void    GetSelection(CSelectionEvent& evt) const;
    virtual void    SetSelection(CSelectionEvent& evt);
    virtual void    GetMainObject(TConstScopedObjects& objects) const;
    /// @}
    void    GetSingleObject(TConstScopedObjects& objects) const;

    void    GetSelectedItems(PT::TItems& items);

    /// @name IDataMiningContext interface implementation
    /// @{
    virtual string  GetDMContextName();
    /// @}

    /// @name ISeqLocSearchContext Implementation
    /// @{
    virtual CRef<objects::CSeq_loc>  GetSearchLoc();
    virtual CRef<objects::CScope>    GetSearchScope();
    virtual void GetMultiple(TConstScopedObjects & pObjects);
    /// @}

    void    OnWidgetSelChanged(CEvent* evt);

protected:
    static CViewTypeDescriptor m_TypeDescr;
    string m_RegPath;

    IWorkbench*     m_Workbench;
    ISelectionService*      m_SelectionService;

    CProjectTreePanel*  m_Panel;
};


///////////////////////////////////////////////////////////////////////////////
/// CProjectTreeViewFactory - a factory producing CProjectTreeView.
class NCBI_GUICORE_EXPORT CProjectTreeViewFactory :
    public CObject,
    public IExtension,
    public IViewFactory
{
public:
    /// @name IExtension interface implementation
    /// @{
    virtual string  GetExtensionIdentifier() const;
    virtual string  GetExtensionLabel() const;
    /// @}

    /// @name IViewFactory interface implementation
    /// @{
    virtual void    RegisterIconAliases(wxFileArtProvider& provider);
    virtual const CViewTypeDescriptor&   GetViewTypeDescriptor() const;
    virtual IView*  CreateInstance() const;
    virtual IView*  CreateInstanceByFingerprint(const TFingerprint& fingerprint) const;
    /// @}
};

END_NCBI_SCOPE

#endif // GUI_CORE___PROJECT_TREE_VIEW__HPP
