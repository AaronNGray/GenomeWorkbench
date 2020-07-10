#ifndef GUI_PKG_ALIGNMENT___DOT_MATRIX_VIEW__HPP
#define GUI_PKG_ALIGNMENT___DOT_MATRIX_VIEW__HPP

/*  $Id: dot_matrix_view.hpp 36326 2016-09-13 20:07:17Z katargir $
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

#include <gui/widgets/hit_matrix/hit_matrix_ds.hpp>


BEGIN_NCBI_SCOPE

/** @addtogroup GUI_PKG_ALIGNMENT
 *
 * @{
 */

class CHitMatrixWidget;

///////////////////////////////////////////////////////////////////////////////
/// CDotMatrixView
class NCBI_GUIVIEW_ALIGN_EXPORT CDotMatrixView : public CProjectView
{
    friend class CDotMatrixViewFactory;

public:
    typedef vector< CConstRef<objects::CSeq_align> > TAlignVector;


public:
    CDotMatrixView();

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
    void GetSelection(CSelectionEvent& evt) const;
    void GetSelection(TConstScopedObjects& objs) const;
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
    virtual void x_CreateMenuBarMenu();

    virtual const CObject* x_GetOrigObject() const;

    /// @name CProjectView internal overridables
    /// @{
    virtual objects::CScope* x_PreAttachToProject( TConstScopedObjects& objects );
    virtual void x_PostAttachToProject();

    virtual void x_OnSetSelection( CSelectionEvent& evt );
    /// @}

    //void x_InitMenuResources();
    void x_InitWidget();

private:
    TAlignVector m_Aligns;

    CIRef<IHitMatrixDataSource> m_DataSource;

    CHitMatrixWidget* m_AlnWidget;

    static CProjectViewTypeDescriptor m_TypeDescr;
};


///////////////////////////////////////////////////////////////////////////////
/// CDotMatrixViewFactory
class NCBI_GUIVIEW_ALIGN_EXPORT CDotMatrixViewFactory 
    : public CObject
    , public IExtension
    , public IProjectViewFactory
{
public:
    CDotMatrixViewFactory();

    /// @name IExtension interface implementation
    /// @{
    virtual string  GetExtensionIdentifier() const;
    virtual string  GetExtensionLabel() const;
    /// @}

    /// @name IProjectViewFactory interface implementation
    /// @{
    virtual IView*  CreateInstance() const;
    virtual IView*  CreateInstanceByFingerprint( const TFingerprint& fingerprint ) const;

    virtual void RegisterIconAliases( wxFileArtProvider& provider );
    virtual void RegisterCommands( CUICommandRegistry& cmd_reg, wxFileArtProvider& provider );

    virtual const CProjectViewTypeDescriptor& GetProjectViewTypeDescriptor() const;
    virtual IOpenViewManager* GetOpenViewManager() { return m_ViewManager; }

    virtual int TestInputObjects( TConstScopedObjects& objects );
    virtual vector<int> CombineInputObjects(const TConstScopedObjects& objects);
    /// @}
private:
    CRef<COpenViewManager> m_ViewManager;
};


/* @} */

END_NCBI_SCOPE

#endif  // GUI_PKG_ALIGNMENT___DOT_MATRIX_VIEW__HPP
