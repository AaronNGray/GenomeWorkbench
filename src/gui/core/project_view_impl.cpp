/*  $Id: project_view_impl.cpp 40879 2018-04-25 20:05:11Z katargir $
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
 * Authors:  Mike DiCuccio, Andrey Yazhuk
 *
 * File Description:
 *    CProjectView -- abstract base class for GBENCH views
 */

#include <ncbi_pch.hpp>

#include <gui/core/project_view_impl.hpp>
#include <gui/core/project_service.hpp>

#include <gui/core/broadcast_vr_change_dlg.hpp>

#include <gui/framework/workbench.hpp>
#include <gui/framework/menu_service.hpp>

#include <gui/objects/PluginMessage.hpp>

#include <gui/widgets/wx/ui_command.hpp>
#include <gui/widgets/wx/wx_utils.hpp>
#include <gui/widgets/wx/popup_event.hpp>

#include <gui/objutils/utils.hpp>

#include <gui/utils/extension_impl.hpp>
#include <gui/objutils/label.hpp>

#include <objmgr/util/sequence.hpp>

#include <gui/utils/command.hpp>

#include <gui/objects/GBWorkspace.hpp>
#include <gui/objects/WorkspaceFolder.hpp>

#include <gui/widgets/wx/message_box.hpp>
#include <gui/widgets/wx/wx_utils.hpp>

#include <wx/window.h>
#include <wx/menu.h>
#include <wx/msgdlg.h>

BEGIN_NCBI_SCOPE
USING_SCOPE(ncbi::objects);

BEGIN_EVENT_TABLE(CProjectViewEvtHandler, wxEvtHandler)
    EVT_MENU( eCmdBroadcastSel, CProjectViewEvtHandler::OnBroadcastSelection )
    EVT_UPDATE_UI( eCmdBroadcastSel, CProjectViewEvtHandler::OnEnableCmdUpdate )
    EVT_UPDATE_UI( eCmdBroadcastSelRange, CProjectViewEvtHandler::OnEnableCmdUpdate )
    EVT_MENU(eCmdVisibleRangeChangeAllow, CProjectViewEvtHandler::OnVRCAllowedChanged)
    EVT_UPDATE_UI(eCmdVisibleRangeChangeAllow, CProjectViewEvtHandler::OnUpdateVRCAllowedChanged)

    EVT_MENU_RANGE(
        eCmdVisibleRangeBroadcast, eCmdVisibleRangeBroadcast + CVisibleRange::eBasic_Ignore, 
        CProjectViewEvtHandler::OnBroadcastVisibleRange
    )

    EVT_MENU(wxID_UNDO, CProjectViewEvtHandler::OnEditUndo)
    EVT_MENU(wxID_REDO, CProjectViewEvtHandler::OnEditRedo)
    EVT_UPDATE_UI(wxID_UNDO, CProjectViewEvtHandler::OnUpdateEditUndo)
    EVT_UPDATE_UI(wxID_REDO, CProjectViewEvtHandler::OnUpdateEditRedo)
END_EVENT_TABLE()

CProjectViewEvtHandler::CProjectViewEvtHandler( CProjectView* view )
    : m_View( view )
{
    SetEvtHandlerEnabled( false );
}

bool CProjectViewEvtHandler::ProcessEvent( wxEvent& event )
{
    // in wx2.9 event propagation schema has been changed,
    // there is for-loop rather than ProcessEvent() calls chain.
    // So we just disable this handler and let other stuff be done.

    if( wxEvtHandler::ProcessEvent( event ) ){
        return true;
    }

#   if( wxMAJOR_VERSION == 2 && wxMINOR_VERSION < 9 )
    
    return TryAfter( event );

#   else

    return false;

#   endif
}

bool CProjectViewEvtHandler::TryAfter( wxEvent& event )
{
    // In wx2.9 we should call TryHereOnly() however as far as
    // we have no dynamic events it is safe to use just handler
    // table processing

    if( GetEventHashTable().HandleEvent( event, this ) ){
        return true;
    }

#   if( wxMAJOR_VERSION == 2 && wxMINOR_VERSION < 9 )
    
    return false;

#   else

    return wxEvtHandler::TryAfter( event );

#   endif
}


/////////////////////////////////////////////////////////////////////////////
/// CProjectView implementation

BEGIN_EVENT_MAP(CProjectView, CProjectViewBase)
    ON_EVENT(CEvent, CViewEvent::eWidgetSelectionChanged, &CProjectView::x_OnWidgetSelChanged)
    ON_EVENT(CEvent, CViewEvent::eWidgetRangeChanged, &CProjectView::x_OnWidgetRangeChanged)
    ON_EVENT(CEvent, CViewEvent::eWidgetItemActivated, &CProjectView::x_OnWidgetItemActivated)
    ON_EVENT(CPopupMenuEvent, CPopupMenuEvent::eDecorate, &CProjectView::OnDecoratePopupMenu)
    ON_EVENT(CPopupMenuEvent, CPopupMenuEvent::eShow, &CProjectView::OnShowPopupMenu)
END_EVENT_MAP()




CProjectView::CProjectView() : m_VisibleRangeService()
{
}


CProjectView::~CProjectView(void)
{
    _ASSERT( ! x_HasProject());
}


bool CProjectView::InitView( TConstScopedObjects& objects, const objects::CUser_object* )
{
    CScope* scope = x_PreAttachToProject( objects );

    if( scope ){
        CIRef<CProjectService> srv = m_Workbench->GetServiceByType<CProjectService>();
        _ASSERT(srv);

        CRef<CGBWorkspace> ws = srv->GetGBWorkspace();
        if (!ws) return false;

        CGBDocument* doc = dynamic_cast<CGBDocument*>(ws->GetProjectFromScope(*scope));
        _ASSERT(doc);

        if( doc ){
            x_AttachToProject( *doc );

            try {
                x_PostAttachToProject();
                return true;

            } catch( CException& ){
                x_DetachFromProject();

                throw;
            }

        }
    }

    // cannot represent the data
    x_ReportInvalidInputData( objects );
        
    return false;
}

CScope* CProjectView::x_PreAttachToProject( TConstScopedObjects& objects )
{
    return NULL;
}

void CProjectView::x_PostAttachToProject()
{    
}

CUndoManager*  CProjectView::x_GetUndoManager()
{
    CIRef<CProjectService> srv = m_Workbench->GetServiceByType<CProjectService>();

    if (srv) {
        CRef<CGBWorkspace> ws = srv->GetGBWorkspace();
        if (!ws) return 0;

        CGBDocument* doc = dynamic_cast<CGBDocument*>(ws->GetProjectFromId(m_ProjectId));
        _ASSERT(doc);

        if (doc) {
            return &doc->GetUndoManager();
        }
    }
    return 0;
}


 void CProjectView::OnUndo()
 {
     CUndoManager* um = x_GetUndoManager();
     if (um) {
         um->Undo();
     }
 }
 
 void CProjectView::OnRedo()
 {
     CUndoManager* um = x_GetUndoManager();
     if (um) {
         um->Redo();
     }
 }

 bool CProjectView::CanUndo()
 {
     CUndoManager* um = x_GetUndoManager();
     if (um) {
         return um->CanUndo();
     }
     return false;
 }

 bool CProjectView::CanRedo()
 {
     CUndoManager* um = x_GetUndoManager();
     if (um) {
         return um->CanRedo();
     }
     return false;
 }

wxEvtHandler* CProjectView::x_CreateEventHandler()
{
    return new CProjectViewEvtHandler( this );
}

void CProjectView::SetWorkbench( IWorkbench* workbench )
{
    if( workbench ){
        if (!m_VisibleRangeService)
            m_VisibleRangeService = workbench->GetServiceByType<IVisibleRangeService>();        

        if (m_VisibleRangeService)
            m_VisibleRangeService->AttachClient(this);

        CProjectViewBase::SetWorkbench( workbench );

        GetWindow()->PushEventHandler( x_CreateEventHandler() );
    
    } else {
        if (m_Workbench) {
            if (m_VisibleRangeService)
                m_VisibleRangeService->DetachClient(this);
        }

        GetWindow()->PopEventHandler( true );

        CProjectViewBase::SetWorkbench(workbench);
    }
}


void CProjectView::OnVRCAllowedChanged()
{
    SetVisibleRangeChangeAllowed(!IsVisibleRangeChangeAllowed());
}


void CProjectView::x_OnWidgetRangeChanged( CEvent* evt )
{
    _ASSERT(evt);


    if( evt->GetID() == CViewEvent::eWidgetRangeChanged ){
        if( CVisibleRange::IsAutoBroadcast() ){
            BroadcastVisibleRange(CVisibleRange::GetDefaultVisibleRangePolicy());

            OnWidgetRangeChanged();

        } else {
            BroadcastVisibleRange( CVisibleRange::eBasic_Ignore );
        }
    }
}

void CProjectView::x_OnWidgetItemActivated( CEvent* evt )
{
    _ASSERT(evt);

    if( evt->GetID() == CViewEvent::eWidgetItemActivated ){
        CIRef<IMenuService> menu_srv = m_Workbench->GetServiceByType<IMenuService>();
        menu_srv->AddPendingCommand(eCmdOpenViewDefault);
    }
}

// this method should be overriden in derived classes to update UI
void CProjectView::OnWidgetRangeChanged()
{
}


void CProjectView::x_OnWidgetSelChanged( CEvent* evt )
{
    _ASSERT(evt);

    if( evt->GetID() != CViewEvent::eWidgetSelectionChanged ){
        return;
    }

    if (m_SelectionService)
        m_SelectionService->OnSelectionChanged(this);

    OnWidgetSelChanged();
}


// this method should be overriden in derived classes to update UI
void CProjectView::OnWidgetSelChanged()
{
    
}


void CProjectView::SetSelectionService(ISelectionService* service)
{
    m_SelectionService = service;
}


// uses GetSelection(CSelectionEvent& evt) to obtain Range and Object Selection
// an then repackages it to the given TConstScopedObjects container
void CProjectView::GetSelection( TConstScopedObjects& objs ) const
{
    LOG_POST(Info << "CProjectView::GetSelection() is deprecated, override this "
                     "function in the derived class");
    CRef<CScope> scope = GetScope();
    if(scope)   {
        CSelectionEvent evt(*scope); // kind of HACK
        GetSelection(evt);

        TConstObjects sel_objs;
        evt.GetRangeSelectionAsLocs(sel_objs);
        evt.GetAllObjects(sel_objs);

        ITERATE(TConstObjects, it, sel_objs)    {
            objs.push_back(SConstScopedObject(*it, scope));
        }
    }
}


void CProjectView::GetSelection( CSelectionEvent& evt ) const
{
    // override this
}


// this is the default implementation that may be overridden in derived classes
void CProjectView::GetMainObject( TConstScopedObjects& objs ) const
{
    CProjectView* nc_this = const_cast<CProjectView*>(this);

    if( nc_this->GetWindow()  &&  x_HasProject() ){
        CScope* scope = GetScope().GetPointer();
        const CObject* obj = x_GetOrigObject();

        _ASSERT(obj && scope);

        objs.push_back(SConstScopedObject(obj, scope));
    }
}

/// handles selection broadcast received from Selection Service
/// this function acts as a shell, it controlls state and catches exceptions.
/// all real work is performed by x_OnSetSelection() that shall be
/// implemented in derived classes
void CProjectView::SetSelection(CSelectionEvent& evt)
{
    try {
        x_OnSetSelection(evt);
    } NCBI_CATCH("CProjectView::SetSelection()");
}


/// override this function in derived classes
void CProjectView::x_OnSetSelection(CSelectionEvent& evt)
{
    // override this
    _TRACE(typeid(this).name() << " - x_OnSetSelection() is not implemented.");
}


/// Populate a visible range event prior to broadcasting
void CProjectView::GetVisibleRanges(CVisibleRange& vrange) const
{
}

void CProjectView::GetReflectionInfo(const CSeq_id& id,
                                    list<TSeqRange>& visible_ranges,
                                    CRgbaColor& color,
                                    string& label) const
{
    CScope* scope = x_GetScope();
    if (scope) {
        /// retrieve visible ranges
        /// we want the total range for each location

        CVisibleRange vrange(*scope);
        GetVisibleRanges(vrange);

        ITERATE(CVisibleRange::TLocations, iter, vrange.GetLocations()){
            try {
                const CSeq_loc& loc = **iter;
                CScope* scope = GetScope().GetPointer();

                const CSeq_id& this_id = sequence::GetId(loc, scope);

                if (CSeqUtils::Match(this_id, id, scope)){
                    visible_ranges.push_back(loc.GetTotalRange());
                }
            }
            catch (std::exception& e){
                ERR_POST(Error << "CProjectView::GetReflectionInfo(): exception caught " << e.what());
            }
        }
    }

    /// retrieve a color for this view
    const CRgbaColor* c = GetColor();
    if( c ){
        color = *c;
    } else {
        color = CRgbaColor(0.0f, 0.0f, 0.0f, 1.0f);
    }

    /// format a label as well
    string s = GetLabel(eId) + ": ";
    s += GetLabel( eTypeAndContent );
    label.swap( s );
}


void CProjectView::BroadcastSelection()
{
    if (!m_SelectionService)
        return;

    CScope* scope = GetScope();
    if (scope) {
        CSelectionEvent sel_evt(*scope);
        GetSelection(sel_evt);
        m_SelectionService->Broadcast(sel_evt, this);
    }
}

void CProjectView::OnBroadcastVisibleRange(wxCommandEvent& event)
{
    CVisibleRange::EBasicPolicy policy = CVisibleRange::GetDefaultVisibleRangePolicy();

    if (event.GetId() == eCmdVisibleRangeBroadcast) {
        CBroadcastVRChangeDialog dlg(GetWindow());

        // SetData();
        dlg.SetByDefault(true);
        dlg.ResetRadioButtons(false);
        dlg.SetBroadcastFlag(policy);
        dlg.TransferDataToWindow();

        if (dlg.ShowModal() != wxID_OK)
            return;

        dlg.TransferDataFromWindow();

        int ret = dlg.GetBroadcastFlag();
        if (ret >= 0)
            policy = (CVisibleRange::EBasicPolicy)ret;
    }
    else {
        policy = (CVisibleRange::EBasicPolicy)(event.GetId() - eCmdVisibleRangeBroadcast - 1);
    }

    BroadcastVisibleRange(policy);
}

void CProjectView::BroadcastVisibleRange(CVisibleRange::EBasicPolicy policy)
{
    if (!m_VisibleRangeService) return;

    try {
        CVisibleRange vrange(*x_GetScope());

        GetVisibleRanges(vrange);

        vrange.SetVisibleRangePolicy(policy);

        m_VisibleRangeService->BroadcastVisibleRange(vrange, this);

    } NCBI_CATCH("CProjectView::BroadcastVisibleRange()");
}


void CProjectView::OnDecoratePopupMenu( CEvent *event )
{
    CPopupMenuEvent* popup_event = dynamic_cast<CPopupMenuEvent*>( event );
    bool ok = popup_event && popup_event->GetID() == CPopupMenuEvent::eDecorate;

    _ASSERT(ok);

    if( ok ){
        x_OnDecoratePopupMenu( *popup_event );
    }
}


// event handler function - forwards call to the virtial function
void CProjectView::OnShowPopupMenu(CEvent *event)
{
    CPopupMenuEvent* popup_event = dynamic_cast<CPopupMenuEvent*>(event);
    bool ok = popup_event && popup_event->GetID() == CPopupMenuEvent::eShow;

    _ASSERT(ok);

    if( ok ){
        x_OnShowPopupMenu();
    }
}


// default implementation simply injects deleagate menus
// override in derived classes if needed
void CProjectView::x_OnDecoratePopupMenu(CPopupMenuEvent& event)
{
    wxMenu* menu = event.GetMenu();

    // add pluggable commands
    vector<wxEvtHandler*> handlers;
    CUICommandRegistry& cmd_reg = m_Workbench->GetUICommandRegistry();
    sAddScopedObjectCommands( *menu, handlers, *this, cmd_reg );

    for( size_t pos = 0;  pos < menu->GetMenuItemCount(); pos++ ){
        wxMenuItem* pos_item = menu->FindItemByPosition( pos );

        if( 
            pos_item->IsSeparator() 
            && pos_item->GetItemLabel().Cmp( wxT("VRBroadcast") ) == 0 
        ){
            menu->Insert( 
                ++pos,
                eCmdVisibleRangeBroadcast,
                wxT("&Broadcast Visible Range..."),
                wxT("Broadcast Visible Range")
            );

            menu->InsertCheckItem( 
                ++pos,
                eCmdVisibleRangeChangeAllow,
                wxT("&Accept Visible Range Change"),
                wxT("Accept Visible Range Change")
            );

            break;
        }
    }
}


void CProjectView::x_OnShowPopupMenu()
{
    // if this assertion fails then the derived class has a problem
    _ASSERT(false);    // override in derived classes
}


WX_DEFINE_MENU(kPrjViewContextMenu)
    WX_MENU_SEPARATOR_L("Top Actions")
    WX_MENU_SEPARATOR_L("Zoom")
    WX_MENU_SEPARATOR_L("Actions")
    WX_MENU_SEPARATOR_L("Edit")
    WX_MENU_SEPARATOR_L("Settings")
WX_END_MENU()


wxMenu* CProjectView::x_CreateContextMenuBackbone()
{
    _ASSERT(m_Workbench);

    CUICommandRegistry& cmd_reg = m_Workbench->GetUICommandRegistry();
    return cmd_reg.CreateMenu(kPrjViewContextMenu);
}


void sAddScopedObjectCommands(
    wxMenu& menu,
    vector<wxEvtHandler*>& handlers,
    const string& ext_point_id,
    TConstScopedObjects& objects,
    CUICommandRegistry& cmd_reg
){
    AddCommandsForScopedObjects( menu, handlers, ext_point_id, objects, &cmd_reg );
    /*
    if( ! objects.empty() ) {
        // get contributed menus
        vector< CIRef<IObjectCmdContributor> > contributors;
        GetExtensionAsInterface(ext_point_id, contributors);

        // Merge contributed menus into the main Menu
        for( size_t i = 0; i < contributors.size(); i++ ) {
            IObjectCmdContributor& obj = *contributors[i];

            IObjectCmdContributor::TContribution contrib = obj.GetMenu(objects);
            wxMenu* obj_menu = contrib.first;
            if(obj_menu)    {
                Merge(menu, *obj_menu);
                delete obj_menu;
            }
            wxEvtHandler* handler = contrib.second;
            if(handler)  {
                handlers.push_back(handler);
            }
        }
    }
    */
}

void sAddScopedObjectCommands(
    wxMenu& menu,
    vector<wxEvtHandler*>& handlers,
    ISelectionClient& obj_provider,
    CUICommandRegistry& cmd_reg
){
    TConstScopedObjects sel_objects;
    obj_provider.GetSelection( sel_objects );

    if( sel_objects.empty() ){
        obj_provider.GetMainObject( sel_objects );
    }

    static string id( "scoped_objects::cmd_contributor" );

    sAddScopedObjectCommands( menu, handlers, id, sel_objects, cmd_reg );
}


END_NCBI_SCOPE
