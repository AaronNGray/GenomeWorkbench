#ifndef GUI_CORE___VIEW__HPP
#define GUI_CORE___VIEW__HPP

/*  $Id: project_view_impl.hpp 40879 2018-04-25 20:05:11Z katargir $
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
 * Authors:  Mike DiCuccio, Andrey Yazhuk
 *
 * File Description:
 *
 */

// TODO Migrate
// TODO Refactoring for the new framework is required

#include <gui/core/project_view_base_impl.hpp>

#include <gui/core/commands.hpp>
#include <gui/core/view_extensions.hpp>
#include <gui/core/visible_range_service.hpp>

#include <gui/objutils/visible_range.hpp>
#include <gui/objutils/obj_event.hpp>


BEGIN_NCBI_SCOPE

class CPopupMenuEvent;
class IVisibleRangeService;

/////////////////////////////////////////////////////////////////////////////
/// CProjectView

class NCBI_GUICORE_EXPORT CProjectView :
    public CProjectViewBase,
    public IVisibleRangeClient
{
public:
    CProjectView();
    virtual ~CProjectView(void);

    /// @name IView implementation
    /// @{
    /// overriding CProjectViewBase implementation
    virtual void    SetWorkbench(IWorkbench* workbench);
    /// @}

    void SetVisibleRangeService(IVisibleRangeService* service) { m_VisibleRangeService = service; }

    /// @name IProjectView implementation
    /// @{
    /// implementation through pre & post Attach functions
    virtual bool InitView( TConstScopedObjects& objects, const objects::CUser_object* params );
    /// @}

    /// @name ISelectionClient Implementation
    /// @{
    virtual void    GetSelection(TConstScopedObjects& buf) const;
    virtual void    SetSelectionService(ISelectionService* service);
    virtual void    GetSelection(CSelectionEvent& evt) const;
    virtual void    SetSelection(CSelectionEvent& evt);
    virtual void    GetMainObject(TConstScopedObjects& buf) const;
    virtual void    BroadcastSelection();
    /// @}

    /// @name IVisibleRangeClient interface implementation
    /// @{
    virtual void OnVisibleRangeChanged (const CVisibleRange&,
                                        IVisibleRangeClient* /*source*/) {}
    ///  @}

    /// populate a visible range event for broadcasting
    virtual void GetVisibleRanges(CVisibleRange& vrange) const;

    /// Retrieve all information relevant to showing view reflection - that is,
    /// displaying the location of sibling views.  This function will retrieve
    /// information about the current view's currently displayed locations, as well
    /// as a uniquely identifying color and label for this view.
    void GetReflectionInfo(const objects::CSeq_id& id, list<TSeqRange>& visible_ranges,
                           CRgbaColor& color, string& label) const;

    /// @name View Callbacks and Event Handlers
    /// @{
    bool IsVisibleRangeChangeAllowed() { return m_VisibleRangeChangeAllowed; }
    void SetVisibleRangeChangeAllowed( bool flag ){ m_VisibleRangeChangeAllowed = flag; }

    void OnVRCAllowedChanged();
    void OnBroadcastVisibleRange( wxCommandEvent& event );

    // Undo/Redo stuff
    virtual void OnUndo();
    virtual void OnRedo();
    virtual bool CanUndo();
    virtual bool CanRedo();

    virtual void OnWidgetRangeChanged();
    virtual void OnWidgetSelChanged();

    void OnDecoratePopupMenu(CEvent *event);
    void OnShowPopupMenu(CEvent *event);
    /// @}

    /// fire a non-recursive visible range changed event
    void BroadcastVisibleRange(CVisibleRange::EBasicPolicy bp);

protected:
    DECLARE_EVENT_MAP();

    virtual objects::CScope* x_PreAttachToProject( TConstScopedObjects& objects );
    virtual void x_PostAttachToProject();

    /// override in derived classes in order to handle selection broadcast
    virtual void    x_OnSetSelection(CSelectionEvent& evt);

    // override these functions in derived classes
    virtual void    x_OnDecoratePopupMenu(CPopupMenuEvent& event);
    virtual void    x_OnShowPopupMenu();

    /// creates vew event handler for using in window event propagation hierarchy
    virtual wxEvtHandler* x_CreateEventHandler();

    /// creates the template of the context menu
    virtual wxMenu* x_CreateContextMenuBackbone();

    /// gets undo manager
    virtual CUndoManager* x_GetUndoManager();

protected:
    /// private, non-virtual event handlers.  These functions are the
    /// primary direct handlers for the event handling mechanism and will
    /// forward event requests to public virtual functions on receipt
    void x_OnWidgetRangeChanged (CEvent* event);
    void x_OnWidgetSelChanged   (CEvent* event);
    void x_OnWidgetItemActivated (CEvent* event);

    IVisibleRangeService* m_VisibleRangeService;

private:
    /// Prohibit copying!
    CProjectView(const CProjectView&);
    CProjectView& operator= (const CProjectView&);
};


/////////////////////////////////////////////////////////////////////////////
/// CProjectViewEvtHandler - wxEvtHandler that is attached to view window.
/// CProjectView cannot inherit from wxEvtHandler, because it already inherit
/// from CObject (the two bases are incompatible). So, in order to handle
/// wxWidgets commands we create a separate handler that will forward calls
/// to the view.
/// Derived classes may install their own handlers.

class CProjectViewEvtHandler : public wxEvtHandler
{
public:
    CProjectViewEvtHandler( CProjectView* view );

    // Redefined hooks for post-stack processing
    virtual bool ProcessEvent( wxEvent& event );
    virtual bool TryAfter( wxEvent& event );


    void OnBroadcastSelection( wxCommandEvent& )
    {
        m_View->BroadcastSelection();
    }

    void OnEnableCmdUpdate( wxUpdateUIEvent& event )
    {
        event.Enable( true );
    }

    void OnBroadcastVisibleRange( wxCommandEvent& event )
    {
        m_View->OnBroadcastVisibleRange( event );
    }

    void OnVRCAllowedChanged( wxCommandEvent& )
    {
        m_View->OnVRCAllowedChanged();
    }

    void OnUpdateVRCAllowedChanged( wxUpdateUIEvent& event )
    {
        bool en = m_View->IsVisibleRangeChangeAllowed();
        event.Check( en );
    }

    void OnEditRedo( wxCommandEvent& )
    {
        m_View->OnRedo();
    }


    void OnEditUndo( wxCommandEvent& )
    {
       m_View->OnUndo();
    }


    void OnUpdateEditUndo(wxUpdateUIEvent& event)
    {    
       event.Enable(m_View->CanUndo());
    }


    void OnUpdateEditRedo(wxUpdateUIEvent& event)
    {
        event.Enable(m_View->CanRedo());
    }

protected:
    
    CProjectView* m_View;

    DECLARE_EVENT_TABLE();
};


END_NCBI_SCOPE

#endif  /// GUI_CORE___VIEW__HPP
