#ifndef GUI_CORE_VIEW_ALN_SPAN___ALN_SPAN__HPP
#define GUI_CORE_VIEW_ALN_SPAN___ALN_SPAN__HPP

/*  $Id: align_span_view.hpp 40880 2018-04-25 20:26:58Z katargir $
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

#include <gui/widgets/aln_table/alnspan_widget.hpp>
#include <gui/widgets/aln_multiple/alnmulti_ds_builder.hpp>
#include <objtools/alnmgr/pairwise_aln.hpp>

#include <gui/core/project_view_impl.hpp>
#include <gui/core/open_view_manager_impl.hpp>
#include <gui/objutils/reg_settings.hpp>

#include <objmgr/scope.hpp>
#include <objects/seq/Seq_annot.hpp>
#include <objects/seqalign/Seq_align.hpp>
#include <objects/seqloc/Seq_id.hpp>

#include <wx/statusbr.h>

BEGIN_NCBI_SCOPE

/** @addtogroup GUI_PKG_ALIGNMENT
 *
 * @{
 */

class CAlnSpanWidget;

///////////////////////////////////////////////////////////////////////////////
/// CAlignSpanView
class NCBI_GUIVIEW_ALIGN_EXPORT CAlignSpanView
    : public CProjectView
    , public IRegSettings
{
    friend class CAlignSpanViewFactory;

public:
    CAlignSpanView();

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
    //virtual bool InitView(TConstScopedObjects& objects, const objects::CUser_object* params);
    virtual void OnProjectChanged( CProjectViewEvent& evt );
    /// @}

    /// @name ISelectionClient Implementation
    /// @{
    virtual void GetSelection(CSelectionEvent& evt) const;
    virtual void GetSelection(TConstScopedObjects& objs) const;
    /// @}

    /// @name CProjectViewBase overridables
    /// @{
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
    /// @name CProjectViewBase internal overridables
    /// @{
    virtual void x_CreateMenuBarMenu();

    virtual const CObject* x_GetOrigObject() const;
    /// @}

    /// @name CProjectView internal overridables
    /// @{
    virtual objects::CScope* x_PreAttachToProject( TConstScopedObjects& objects );
    virtual void x_PostAttachToProject();

    virtual void x_OnSetSelection( CSelectionEvent& evt );
    /// @}

    void x_InitMenuResources();
    void x_InitWidget();

    virtual void x_UpdateStatusMessage();
    virtual void x_Update();

private:
    /// original data object passed to the view
    CConstRef<CSeq_annot>   m_OrigAnnot;
    CConstRef<CSeq_align>   m_OrigAlign;
    CConstRef<CBioseq>      m_OrigBioseq;

    vector< CConstRef<CSeq_align> > m_InputAligns;


    CRef<IAlnMultiDataSource> m_DataSource;
    CAnchoredAln m_AnchoredAln;

    wxPanel* m_Panel;
    wxStatusBar* m_StatusBar;
    CAlnSpanWidget* m_AlnWidget;
    auto_ptr<CAlnSpanVertModel> m_Model;

    CRef<CScope> m_Scope;
    CRef<CSeq_id> m_Seq_id;

    string m_RegPath; // path to the setting in CGuiRegistry

    static CProjectViewTypeDescriptor m_TypeDescr;
};


///////////////////////////////////////////////////////////////////////////////
/// CAlignSpanViewFactory
class NCBI_GUIVIEW_ALIGN_EXPORT CAlignSpanViewFactory 
    : public CObject
    , public IExtension
    , public IProjectViewFactory
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
    virtual IView*  CreateInstanceByFingerprint(const TFingerprint& fingerprint) const;
    virtual void RegisterIconAliases(wxFileArtProvider& provider);

    virtual const CProjectViewTypeDescriptor& GetProjectViewTypeDescriptor() const;
    virtual int TestInputObjects( TConstScopedObjects& objects );
    /// @}
};

END_NCBI_SCOPE

#endif  // GUI_CORE_VIEW_ALN_SPAN___ALN_SPAN__HPP
