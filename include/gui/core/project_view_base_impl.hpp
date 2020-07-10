#ifndef GUI_CORE___PROJECT_VIEW_BASE_IMPL__HPP
#define GUI_CORE___PROJECT_VIEW_BASE_IMPL__HPP

/*  $Id: project_view_base_impl.hpp 40221 2018-01-10 17:31:26Z katargir $
 * ===========================================================================
 *
 *                            PUBLIC DOMAIN NOTICE
 *               National Center for Biotechnology Information
 *
 *  This software / database is a "United States Government Work" under the
 *  terms of the United States Copyright Act.  It was written as part of
 *  the author's official duties as a United States Government employee and
 *  thus cannot be copyrighted.  This software / database is freely available
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
 * Authors:  Andrey Yazhuk, Mike DiCuccio
 *
 * File Description:
 *
 */

#include <gui/core/project_view.hpp>

#include <gui/core/document.hpp>

#include <gui/framework/tool_bar_service.hpp>

#include <gui/objutils/objects.hpp>

#include <gui/utils/event_handler.hpp>


#include <wx/event.h>


BEGIN_NCBI_SCOPE

class CUICommandRegistry;


BEGIN_SCOPE(objects)
    class CPluginMessage;
END_SCOPE(objects)


///////////////////////////////////////////////////////////////////////////////
/// CProjectViewBase - default implementation of IProjectView, the base class
/// for CProjectView and CProjectViewComposite

class NCBI_GUICORE_EXPORT CProjectViewBase 
    : public CObjectEx
    , public CEventHandler
    , public IProjectView
    , public IToolBarContext
{
    DECLARE_EVENT_MAP();

public:

    CProjectViewBase();
    virtual ~CProjectViewBase();

    /// @name IProjectView implementation
    /// @{
    virtual void DestroyView();

    virtual void InitialLayout();
    virtual void Destroy();

    TId  GetId() const;

    virtual TProjectId GetProjectId() const;

    virtual string  GetLabel( ELabelType type ) const;

    virtual IServiceLocator* GetServiceLocator() const;
    virtual CRef<objects::CScope> GetScope() const;

    /// @name IWMClient implementation
    /// @{
    virtual wxWindow* GetWindow() = 0;
    virtual wxEvtHandler* GetCommandHandler();
    virtual string GetClientLabel(IWMClient::ELabel ltype = IWMClient::eDefault) const;
    virtual string GetIconAlias() const;
    virtual const CRgbaColor* GetColor() const;
    virtual void SetColor( const CRgbaColor& color );
    virtual const wxMenu* GetMenu();
    virtual CFingerprint GetFingerprint() const;
    virtual void  SetFingerprint(const string& fp);
    /// @}

    /// @name IView implementation
    /// @{
    virtual const CViewTypeDescriptor& GetTypeDescriptor() const = 0;
    virtual void SetWorkbench( IWorkbench* workbench );
    virtual void CreateViewWindow( wxWindow* parent ) = 0;
    virtual void DestroyViewWindow() = 0;
    virtual void SetAsyncDestroy(bool reset_hist_async) { m_ResetHistAsync = reset_hist_async; }
    /// @}

    virtual void RefreshViewWindow();

    /// @name ISelectionClient implementation (partial)
    /// @{
    virtual string  GetSelClientLabel();
    /// @}

    /// @name IToolBarContext implementation
    /// @{
    virtual void GetCompatibleToolBars( vector<string>& names );
    /// @}

    /// overloadable hook for derived classes to customize the content aspect of
    /// a view's title.  This function is called from GetLabel() to present the
    /// content portion of the label; derived classes can specialize this function
    /// to provide a view-specific content label.
    virtual string GetContentLabel() const;

    /// @name Event Handling
    /// @{
    virtual void OnSelectionChanged( const TConstScopedObjects& sels );
    virtual void OnProjectChanged();
    virtual void OnProjectChanged( CProjectViewEvent& evt );
    virtual void OnViewAttached( IView& view );
    virtual void OnViewReleased( IView& view );

virtual void OnPrint();
    /// @}

    static void SetLabelGenerator(IProjectViewLabel* labelGenerator);

 protected:
    virtual objects::CScope* x_GetScope() const;

    virtual const CObject* x_GetOrigObject() const;

    virtual bool x_HasProject() const;

    virtual void x_UpdateContentLabel();

    virtual void x_AttachToProject(CGBDocument& doc);
    virtual void x_DetachFromProject();

    /// use this function to report incompatible data in InitView()
    virtual void x_ReportInvalidInputData( TConstScopedObjects& objects );

    /// initializes m_MenuBarMenu; override in derived classes
    virtual void x_CreateMenuBarMenu();

    /// private, non-virtual event handlers.  These functions are the
    /// primary direct handlers for the event handling mechanism and will
    /// forward event requests to public virtual functions on receipt
    void eh_OnProjectChanged( CEvent* event );
    void eh_OnViewAttached( CEvent* event );
    void eh_OnViewReleased( CEvent* event );

protected:
    const size_t m_ViewId;

    IWorkbench* m_Workbench;

    // the menu that view supplies to the application Menu Bar
    auto_ptr<const wxMenu> m_MenuBarMenu;


    //TODOCIRef<IGBDocument> m_Project; /// The document
    int m_ProjectId;
    CRef<objects::CScope> m_Scope;

    // cached project name (caching limits access to the project that requires
    // locking).
    string  m_ProjectName;
    string  m_ContentLabel;

    ISelectionService* m_SelectionService; /// the selection service we speak to

    CRgbaColor m_Color; /// this view's private color

    /// enum defining how we broadcast our selections
    bool m_VisibleRangeChangeAllowed;
    bool m_ResetHistAsync;
    string m_Fingerprint;

    static CIRef<IProjectViewLabel> m_LabelGenerator;
};


///////////////////////////////////////////////////////////////////////////////
/// CProjectViewToolBatFactory - provides standard toolbars for Project Views.

class NCBI_GUICORE_EXPORT CProjectViewToolBatFactory :
    public IToolBarContributor
{
public:
    static const string sm_NavTBName;
    static const string sm_SelTBName;

    /// @name IToolBarContributor implementation
    /// @{
    virtual void        GetToolBarNames(vector<string>& names);
    virtual wxAuiToolBar*  CreateToolBar(const string& name, wxWindow* parent);
    /// @}

protected:
    wxAuiToolBar*  x_CreateNavigationToolbar(wxWindow* parent);
    wxAuiToolBar*  x_CreateSelectionToolbar(wxWindow* parent);
};

END_NCBI_SCOPE

#endif  /// GUI_CORE___PROJECT_VIEW_BASE_IMPL__HPP
