#ifndef GUI_CORE_VIEW_TABLE___VIEW_FEATTABLE__HPP
#define GUI_CORE_VIEW_TABLE___VIEW_FEATTABLE__HPP

/*  $Id: view_feattable.hpp 40880 2018-04-25 20:26:58Z katargir $
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
 * Authors:  Yury Voronov
 *
 * File Description:
 *
 */
#include <gui/widgets/feat_table/feat_table.hpp>

#include <gui/core/project_view_impl.hpp>
#include <gui/core/open_view_manager_impl.hpp>
#include <gui/objutils/reg_settings.hpp>

#include <objmgr/scope.hpp>
#include <objects/seq/Seq_annot.hpp>
#include <objects/seqloc/Seq_id.hpp>
#include <gui/objutils/object_index.hpp>
#include <gui/objutils/reg_settings.hpp>

//#include <gui/services/seqloc_search_context.hpp>

class wxStatusBar;

BEGIN_NCBI_SCOPE


/** @addtogroup GUI_PKG_SEQUENCE
 *
 * @{
 */

class CFeatTableWidget;

///////////////////////////////////////////////////////////////////////////////
/// CFeatTableView
class CFeatTableView
    : public CProjectView
    , public IRegSettings
{
    friend class CFeatTableViewFactory;
    DECLARE_EVENT_MAP();

public:
    CFeatTableView();

    /// @name IWMClient interface implementation
    /// @{
    virtual wxWindow* GetWindow();
    /// @}

    /// @name IView interface implementation
    /// @{
    virtual const CViewTypeDescriptor& GetTypeDescriptor() const;
    virtual void CreateViewWindow(wxWindow* parent);
    virtual void DestroyViewWindow();
    /// @}

    /// @name IProjectView implementation
    /// @{
    virtual bool InitView(TConstScopedObjects& objects, const objects::CUser_object* params);
    virtual void OnProjectChanged( CProjectViewEvent& evt );
    virtual void OnProjectChanged();
    /// @}

    /// @name ISelectionClient Implementation
    /// @{
    virtual void GetSelection(CSelectionEvent& evt) const;
    virtual void GetSelection(TConstScopedObjects& objs) const;
    /// @}

    /// @name CProjectViewBase overridables
    /// @{
    virtual void GetCompatibleToolBars(vector<string>& names);
    virtual void GetVisibleRanges(CVisibleRange& vrange) const;
    /// @}

    /// @name CProjectView callback overridables
    /// @{
    virtual void OnWidgetRangeChanged();
    virtual void OnWidgetSelChanged();
    /// @}

    /// @name IRegSettings interface implementation
    /// @{
    virtual void SetRegistryPath(const string& reg_path);
    virtual void LoadSettings();
    virtual void SaveSettings() const;
    /// @}

protected:
    void x_OnFeatTableEvent(CEvent*);

    void x_InitMenuResources();

    virtual void x_CreateMenuBarMenu();

    virtual void x_OnSetSelection( CSelectionEvent& evt );

    virtual const CObject* x_GetOrigObject() const;

    virtual void x_Update();
    virtual void x_UpdateStatusMessage();
    virtual void x_RebuildObjectIndex();

    virtual void x_SetVisibleRange(const objects::CSeq_loc& loc);
    //virtual void x_OnVisibleRangeMethod();

    virtual bool x_MergeObjects( TConstScopedObjects& objects );

    /// @name CProjectView overridables
    /// @{
    objects::CScope* x_PreAttachToProject( TConstScopedObjects& objects );
    void x_PostAttachToProject();
    /// @}


private:
    /// original data object passed to the view
    CConstRef<CSerialObject> m_OrigObject;
    CRef<objects::CScope> m_Scope;
    //objects::SAnnotSelector m_Sel;


    CObjectIndex m_ObjectIndex;

    wxPanel* m_Panel;
    wxStatusBar* m_StatusBar;
    CFeatTableWidget *m_FTblWidget;
    CRef<CFeatTableDS> m_DataSource;

    string m_RegPath; // path to the setting in CGuiRegistry
};


///////////////////////////////////////////////////////////////////////////////
/// CFeatTableViewFactory
class CFeatTableViewFactory :
    public CObject,
    public IExtension,
    public IProjectViewFactory
{
public:
    /// @name IExtension interface implementation
    /// @{
    virtual string GetExtensionIdentifier() const;
    virtual string GetExtensionLabel() const;
    /// @}

    /// @name IProjectViewFactory interface implementation
    /// @{
    virtual IView* CreateInstance() const;
    virtual IView* CreateInstanceByFingerprint(const TFingerprint& fingerprint) const;
    virtual void RegisterIconAliases(wxFileArtProvider& provider);

    virtual const CProjectViewTypeDescriptor& GetProjectViewTypeDescriptor() const;
    virtual bool IsCompatibleWith( const CObject& object, objects::CScope& scope );
    virtual int TestInputObjects( TConstScopedObjects& objects );
    /// @}
};

/* @} */

END_NCBI_SCOPE


#endif  // GUI_CORE_VIEW_TABLE___VIEW_FEATTABLE__HPP
