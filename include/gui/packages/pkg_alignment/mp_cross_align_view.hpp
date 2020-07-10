#ifndef GUI_PKG_ALIGNMENT___MP_CROSS_ALIGN_VIEW__HPP
#define GUI_PKG_ALIGNMENT___MP_CROSS_ALIGN_VIEW__HPP

/*  $Id: mp_cross_align_view.hpp 33102 2015-05-26 19:26:09Z katargir $
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
 * Authors:  Roman Katargin
 *
 * File Description:
 *
 *
 */

#include <gui/core/project_view_impl.hpp>
#include <gui/core/open_view_manager_impl.hpp>

#include <gui/widgets/aln_crossaln/cross_aln_ds.hpp>


class wxSplitterWindow;

BEGIN_NCBI_SCOPE

/** @addtogroup GUI_PKG_ALIGNMENT
 *
 * @{
 */

///////////////////////////////////////////////////////////////////////////////
/// CMPCrossAlignView
class NCBI_GUIVIEW_ALIGN_EXPORT CMPCrossAlignView : public CProjectView, public IVisibleRangeService
{
    friend class CMPCrossAlignViewFactory;
public:
    CMPCrossAlignView();

    /// @name IVisibleRangeService interface implementation
    /// @{
    virtual void AttachClient(IVisibleRangeClient*) {}
    virtual void DetachClient(IVisibleRangeClient*) {}
    virtual void BroadcastVisibleRange(const CVisibleRange& vrange,
                                       IVisibleRangeClient* source);
    /// @}

    /// @name IWMClient interface implementation
    /// @{
    virtual wxWindow*   GetWindow();
    /// @}

    /// @name IView interface implementation
    /// @{
    virtual const   CViewTypeDescriptor&  GetTypeDescriptor() const;
    virtual void    CreateViewWindow(wxWindow* parent);
    virtual void    DestroyViewWindow();
    virtual void    SetWorkbench(IWorkbench* workbench);
    /// @}

    /// @name IProjectView implementation
    /// @{
    virtual void    DestroyView();
    virtual bool    InitView(TConstScopedObjects& objects, const objects::CUser_object* params);
    virtual void    OnProjectChanged();
    /// @}

    /// @name ISelectionClient Implementation
    /// @{
    virtual void SetSelection(CSelectionEvent& evt);
    virtual void GetSelection(CSelectionEvent& evt) const;
    virtual void GetSelection(TConstScopedObjects& objs) const;
    virtual void GetMainObject(TConstScopedObjects& objs) const;
    /// @}

    /// @name CProjectViewBase overridables
    /// @{
    virtual void GetVisibleRanges(CVisibleRange& vrange) const;
    /// @}

    /// @name IVisibleRangeClient interface implementation
    /// @{
    virtual void OnVisibleRangeChanged (const CVisibleRange& vrange,
                                        IVisibleRangeClient* source);
    ///  @}

protected:
    virtual void    x_CreateMenuBarMenu();

    virtual void x_OnSetSelection(CSelectionEvent& evt);

    const CObject*  x_GetOrigObject() const;

private:
    typedef vector< CIRef<IProjectView> > TViews;
    /// original data object passed to the view
    CConstRef<CSeq_annot> m_OrigAnnot;
    CConstRef<CSeq_align> m_OrigAlign;
    CIRef<ICrossAlnDataSource> m_DataSource;

    wxSplitterWindow* m_MainSplitter;
    TViews m_Views;

    static CProjectViewTypeDescriptor m_TypeDescr;
};


///////////////////////////////////////////////////////////////////////////////
/// CMPCrossAlignViewFactory
class NCBI_GUIVIEW_ALIGN_EXPORT CMPCrossAlignViewFactory :
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
    virtual IView*  CreateInstanceByFingerprint(const TFingerprint& fingerprint) const;
    virtual void RegisterIconAliases(wxFileArtProvider& provider);
    virtual void RegisterCommands(CUICommandRegistry& cmd_reg, wxFileArtProvider& provider);

    virtual const CProjectViewTypeDescriptor& GetProjectViewTypeDescriptor() const;
    virtual int TestInputObjects( TConstScopedObjects& objects );
    virtual bool IsCompatibleWith( const CObject& object, objects::CScope& scope );
    /// @}
};


/* @} */

END_NCBI_SCOPE

#endif  // GUI_PKG_ALIGNMENT___MP_CROSS_ALIGN_VIEW__HPP
