#ifndef GUI_PKG_ALIGNMENT___PHY_TREE_VIEW__HPP
#define GUI_PKG_ALIGNMENT___PHY_TREE_VIEW__HPP

/*  $Id: phy_tree_view.hpp 42391 2019-02-14 18:25:12Z katargir $
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
 * File Description:
 *
 *
 */

#include <gui/core/project_view_impl.hpp>
#include <gui/core/open_view_manager_impl.hpp>

#include <gui/widgets/phylo_tree/phylo_tree_ds.hpp>
#include <gui/widgets/phylo_tree/phylo_tree_scheme.hpp>
#include <objects/biotree/BioTreeContainer.hpp>
#include <gui/objutils/object_index.hpp>
#include <gui/objutils/reg_settings.hpp>
#include <gui/core/loading_app_job.hpp>


BEGIN_NCBI_SCOPE

/** @addtogroup GUI_PKG_ALIGNMENT
 *
 * @{
 */


class CPhyloTreeWidget;

///////////////////////////////////////////////////////////////////////////////
/// CPhyTreeView
class NCBI_GUIVIEW_ALIGN_EXPORT CPhyTreeView 
    : public CProjectView
    , public IRegSettings
{
    friend class CPhyTreeViewFactory;

    DECLARE_EVENT_MAP();

public:
    CPhyTreeView();

    ~CPhyTreeView();

    /// @name IWMClient interface implementation
    /// @{
    virtual wxWindow*   GetWindow();
    /// @}

    /// @name IView interface implementation
    /// @{
    virtual const   CViewTypeDescriptor&  GetTypeDescriptor() const;
    virtual void    CreateViewWindow(wxWindow* parent);
    virtual void    DestroyViewWindow();
    /// @}

    /// @name IProjectView implementation
    /// @{
    virtual bool    InitView(TConstScopedObjects& objects, const objects::CUser_object* params);
    virtual void    OnProjectChanged(CProjectViewEvent& evt);
    /// @}

    /// @name ISelectionClient Implementation
    /// @{
    void GetSelection(CSelectionEvent& evt) const;
    void GetSelection(TConstScopedObjects& objs) const;
    /// @}

    /// @name CProjectViewBase overridables
    /// @{
    virtual void GetVisibleRanges(CVisibleRange& vrange) const;
    /// @}

    /// @name IRegSettings interface implementation
    /// @{
    virtual void SetRegistryPath(const string& reg_path);
    virtual void LoadSettings();
    virtual void SaveSettings() const;
    /// @}

    CRef<CPhyloTreeScheme> GetScheme() { return m_Scheme; }
    CPhyloTreeWidget*      GetWidget() const;
    void OnBioTreeChanged();
    void OnExportTree(CEvent* evt);
    void OnSomethingEdited(CEvent* evt);
    void OnCreateSelectedSubtree(CEvent* evt);
    void CommitTheChanges(CNcbiIstrstream& istr);
    void AppliedEditToDataSource() { m_EditUpdate = true; }
    void RebuildObjectIndex();

protected:
    virtual void x_CreateMenuBarMenu();

    virtual void x_OnSetSelection(CSelectionEvent& evt);

    virtual const CObject*  x_GetOrigObject() const;

    void x_QueryStatusChange(CEvent* evt);

private:    
    CConstRef<objects::CBioTreeContainer>       m_BioTreeContainer;
    mutable CRef<objects::CBioTreeContainer>    m_Cont;
    mutable CPhyloTree::TTreeIdx                m_CurrentNode;

    enum {
        kQueryPanelID  = 6000,
        kPhylpWidgetID = 6001
    };


    CRef<CPhyloTreeDataSource>  m_DataSource;
    CRef<CPhyloTreeScheme>      m_Scheme;
    wxPanel*                    m_Panel;
    CObjectIndex                m_ObjectIndex;
    bool                        m_EditUpdate;

    /// If rendering parms are stored in biotreecontainer, set them first time
    int                         m_InitialRotateLabels;
    int                         m_InitialUseDistances;
    int                         m_InitialLayout;
};


///////////////////////////////////////////////////////////////////////////////
/// CPhyTreeViewFactory
class NCBI_GUIVIEW_ALIGN_EXPORT CPhyTreeViewFactory :
    public CObject,
    public IExtension,
    public IProjectViewFactory
{
public:
    /// @name IExtension interface implementation
    /// @{
    virtual string  GetExtensionIdentifier() const;
    virtual string  GetExtensionLabel() const;
    /// @}

    /// @name IProjectViewFactory interface implementation
    /// @{
    virtual IView* CreateInstance() const;
    virtual IView* CreateInstanceByFingerprint(const TFingerprint& fingerprint) const;
    virtual void RegisterIconAliases(wxFileArtProvider& provider);
    virtual void RegisterCommands(CUICommandRegistry& cmd_reg, wxFileArtProvider& provider);

    virtual const CProjectViewTypeDescriptor& GetProjectViewTypeDescriptor() const;
    virtual int TestInputObjects( TConstScopedObjects& objects );
    /// @}
};

/* @} */



///////////////////////////////////////////////////////////////////////////////
/// CPhyloTreeJob
class  CPhyloTreeJob : public CDataLoadingAppJob
{
public:
    CPhyloTreeJob(const objects::CBioTreeContainer* btc);

protected:
    virtual void    x_CreateProjectItems();

private:
    CRef <objects::CBioTreeContainer> m_Btc;
};

END_NCBI_SCOPE

#endif  // GUI_PKG_ALIGNMENT___PHY_TREE_VIEW__HPP
