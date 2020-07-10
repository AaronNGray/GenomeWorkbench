#ifndef GUI_PKG_ALIGNMENT___MULTI_ALIGN_VIEW__HPP
#define GUI_PKG_ALIGNMENT___MULTI_ALIGN_VIEW__HPP

/*  $Id: multi_align_view.hpp 39118 2017-08-01 20:05:22Z evgeniev $
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
 *
 */

#include <gui/core/project_view_impl.hpp>
#include <gui/core/open_view_manager_impl.hpp>

#include <gui/widgets/aln_multiple/alnmulti_ds.hpp>

#include <gui/types.hpp>

#include <gui/objutils/object_index.hpp>
#include <gui/objutils/ui_data_source_notif.hpp>

#include <gui/widgets/aln_multiple/alnvec_multi_ds.hpp>
#include <gui/widgets/aln_multiple/alnmulti_widget.hpp>
#include <gui/widgets/aln_multiple/alnmulti_ds_builder.hpp>

#include <objects/seq/Seq_annot.hpp>

BEGIN_NCBI_SCOPE

/** @addtogroup GUI_PKG_ALIGNMENT
 *
 * @{
 */

class CAlnMultiWidget;

///////////////////////////////////////////////////////////////////////////////
/// CMultiAlignView
class NCBI_GUIVIEW_ALIGN_EXPORT CMultiAlignView :
    public CProjectView,
    public IRegSettings,
    public CDataChangeNotifier::IListener
{
    friend class CMultiAlignViewFactory;

    DECLARE_EVENT_MAP();
public:
    CMultiAlignView();

    /// @name IWMClient interface implementation
    /// @{
    virtual wxWindow*   GetWindow();
    /// @}

    /// @name IView interface implementation
    /// @{
    virtual const CViewTypeDescriptor&  GetTypeDescriptor() const;
    virtual void    CreateViewWindow(wxWindow* parent);
    virtual void    DestroyViewWindow();
    /// @}

    /// @name IProjectView implementation
    /// @{
    virtual bool InitView(TConstScopedObjects& objects, const objects::CUser_object* params);
    virtual void OnProjectChanged();
    virtual void OnProjectChanged(CProjectViewEvent& evt);
    /// @}

    /// @name ISelectionClient implementation
    /// @{
    void GetSelection(CSelectionEvent& evt) const;
    void GetSelection(TConstScopedObjects& objs) const;
    /// @}

    /// @name IRegSettings interface implementation
    /// @{
    virtual void SetRegistryPath(const string& reg_path);
    virtual void LoadSettings();
    virtual void SaveSettings() const;
    /// @}

    /// @name CUIDataSource::IListener implementation
    /// @{
    virtual void    OnDSChanged(CDataChangeNotifier::CUpdate& update);
    /// @}

    void    OnAlnWidgetSelChanged(CEvent* evt);

    void OnOpenGraphicalView(CEvent* evt);

protected:
    virtual void    x_CreateMenuBarMenu();
    virtual const SwxMenuItemRec* GetMenuDef() const;


    virtual bool    x_GetValidInputData(const CObject* obj);

    virtual void    x_RebuildObjectIndex();
    virtual void    x_GetRowSeqLoc(CAlnVec::TNumrow row, CSelectionEvent::TRangeColl& loc) const;
    virtual bool    x_GetSeqRangeSelection(CAlnMultiWidget::TRangeColl& seq_coll) const;
    virtual void    x_OnSetSelection(CSelectionEvent& evt);

    virtual const CObject*  x_GetOrigObject() const;

    virtual bool x_MergeObjects( TConstScopedObjects& objects );

private:
    string  m_RegPath;

protected:
    CRowStyleCatalog    m_StyleCatalog;
private:
    vector<string>  m_ScoringMethods;
    string  m_CurrentMethod;

protected:
    CAlnMultiDSBuilder m_Builder;
    bool m_Sparse;

protected:
    /// original data object passed to the view
    CConstRef<objects::CSeq_annot> m_OrigAnnot;
    CConstRef<objects::CSeq_align> m_OrigAlign;
    CIRef<IAlnMultiDataSource> m_DataSource;

    CAlnMultiWidget*    m_AlnWidget;

    CObjectIndex m_ObjIndex;
};


///////////////////////////////////////////////////////////////////////////////
/// CMultiAlignViewFactory
class NCBI_GUIVIEW_ALIGN_EXPORT CMultiAlignViewFactory :
    public CObject,
    public IExtension,
    public IProjectViewFactory
{
public:
    CMultiAlignViewFactory();
    /// @name IExtension interface implementation
    /// @{
    virtual string  GetExtensionIdentifier() const;
    virtual string  GetExtensionLabel() const;
    /// @}

    /// @name IProjectViewFactory interface implementation
    /// @{
    virtual IView* CreateInstance() const;
    virtual IView*  CreateInstanceByFingerprint(const TFingerprint& fingerprint) const;
    virtual void RegisterIconAliases(wxFileArtProvider& provider);
    virtual void RegisterCommands(CUICommandRegistry& cmd_reg, wxFileArtProvider& provider);

    virtual const CProjectViewTypeDescriptor& GetProjectViewTypeDescriptor() const;
    virtual IOpenViewManager* GetOpenViewManager() { return m_ViewManager; }
    virtual bool IsCompatibleWith( const CObject& object, objects::CScope& scope );
    virtual int TestInputObjects( TConstScopedObjects& objects );
    /// @}
private:
    CRef<COpenViewManager> m_ViewManager;
};

class CSelRowHandle : public ISelObjectHandle
{
public:
    CSelRowHandle(IAlnExplorer::TNumrow row) : m_Numrow(row)    {}
    IAlnExplorer::TNumrow    GetNumrow() const   {   return m_Numrow;    }

protected:
    IAlnExplorer::TNumrow    m_Numrow;
};

/* @} */

END_NCBI_SCOPE

#endif  // GUI_PKG_ALIGNMENT___MULTI_ALIGN_VIEW__HPP
