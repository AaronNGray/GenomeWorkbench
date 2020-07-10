#ifndef GUI_CORE_VIEW_GRAPHIC___VIEW_GRAPHIC__HPP
#define GUI_CORE_VIEW_GRAPHIC___VIEW_GRAPHIC__HPP

/*  $Id: view_graphic.hpp 44126 2019-11-01 16:58:03Z filippov $
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
 * Authors:  Vlad Lebedev
 *
 * File Description:
 *    User-modifiable part for future extension of the main graphical view
 *    class.
 */


#include <corelib/ncbiobj.hpp>

#include <gui/core/project_view_impl.hpp>
#include <gui/core/open_view_manager_impl.hpp>
#include <gui/framework/view_manager_service.hpp>
#include <gui/core/seqloc_search_context.hpp>
#include <gui/core/visible_range_service.hpp>
#include <gui/widgets/wx/ibioseq_editor.hpp>
#include <objects/seqloc/Seq_id.hpp>


BEGIN_NCBI_SCOPE


/** @addtogroup GUI_PKG_SEQUENCE
 *
 * @{
 */

class CGraphicPanel;

///////////////////////////////////////////////////////////////////////////////
/// CViewGraphic
class CViewGraphic : public CProjectView
                   , public ISeqLocSearchContext
                   , public IViewSettingsAtProject

{
    friend class CViewGraphicFactory;
    DECLARE_EVENT_MAP();
public:
    CViewGraphic();
    ~CViewGraphic();

    /// @name IWMClient interface implementation
    /// @{
    virtual wxWindow*   GetWindow();
    virtual const wxMenu* GetMenu();
    /// @}

    /// @name IView interface implementation
    /// @{
    virtual const CViewTypeDescriptor&  GetTypeDescriptor() const;
    virtual void  CreateViewWindow(wxWindow* parent);
    virtual void  DestroyViewWindow();
    /// @}

    /// @name IProjectView implementation
    /// @{
    virtual bool InitView(TConstScopedObjects& objects, const objects::CUser_object* params);
    virtual void OnProjectChanged(CProjectViewEvent& evt);
    virtual void SetOrigObject(SConstScopedObject& obj);
    /// @}

    /// @name IViewSettingsAtProject implementation
    /// @{
    void SaveSettingsAtProject(objects::CGBProjectHandle& project) const;
    /// @}

    /// @name ISelectionClient Implementation
    /// @{
    void    GetSelection(CSelectionEvent& evt) const;
    void    GetSelection(TConstScopedObjects& objs) const;
    /// @}

    /// @name IVisibleRangeClient interface implementation
    /// @{
    virtual void OnVisibleRangeChanged (const CVisibleRange& vrange,
                                        IVisibleRangeClient* source);
    ///  @}

    /// @name IDataMiningContext Implementation
    /// @{
    virtual string  GetDMContextName();
    /// @}

    /// @name CProjectView overridables
    /// @{
    virtual void GetVisibleRanges( CVisibleRange& vrange ) const;
    virtual void OnWidgetRangeChanged();
    /// @}
    void OnWidgetDataChanged(CEvent* evt);
    void OnOpenGraphicalView(CEvent* evt);

    /// @name CProjectViewBase overridables
    /// @{
    virtual void    OnViewAttached(IView&);
    virtual void    OnViewReleased(IView&);
    /// @}

    virtual void    SetWorkbench(IWorkbench* workbench);
    virtual CRef<objects::CSeq_loc>  GetSearchLoc();
    virtual CRef<objects::CScope>    GetSearchScope();

protected:
    /// @addtogroup CProjectView overridables
    /// @{
    virtual void    x_OnSetSelection(CSelectionEvent& evt);
    /// @}

    /// @addtogroup CProjectViewBase overridables
    /// @{
    virtual void GetCompatibleToolBars(vector<string>& names);
    virtual const CObject* x_GetOrigObject() const;
    /// @}

    void x_UpdateVisRangeLayout(const IViewManagerService::TViews& views);

    virtual bool x_MergeObjects( TConstScopedObjects& objects );

    string x_GetViewSettingsFromParams(const objects::CUser_object* params);

private:
    CConstRef<objects::CSeq_id>     m_SeqId;
    CGraphicPanel*                  m_GraphicPanel;
};


///////////////////////////////////////////////////////////////////////////////
/// CViewGraphicFactory
class CViewGraphicFactory :
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
    virtual bool IsCompatibleWith( const CObject& object, objects::CScope& scope );
    /// @}
};


/* @} */

END_NCBI_SCOPE

#endif  // GUI_CORE_VIEW_GRAPHIC___VIEW_GRAPHIC__HPP
