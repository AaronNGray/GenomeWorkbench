/*  $Id: mp_cross_align_view.cpp 40878 2018-04-25 19:54:29Z katargir $
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
 *    User-modifiable implementation file for extension of Dot Matrix
 *    viewer for GBENCH
 */

#include <ncbi_pch.hpp>

#include <gui/packages/pkg_alignment/mp_cross_align_view.hpp>

#include <gui/objects/GBWorkspace.hpp>
#include <gui/objects/WorkspaceFolder.hpp>

#include <gui/core/project_service.hpp>
#include <gui/objutils/visible_range.hpp>

#include <gui/framework/workbench.hpp>
#include <gui/framework/view_manager_service.hpp>
#include <gui/framework/window_manager_service.hpp>

#include <gui/widgets/wx/fileartprov.hpp>
#include <gui/widgets/wx/ui_command.hpp>
#include <gui/widgets/wx/wx_utils.hpp>

#include <gui/objutils/utils.hpp>
#include <gui/objutils/label.hpp>

#include <wx/menu.h>
#include <wx/splitter.h>


BEGIN_NCBI_SCOPE
USING_SCOPE(objects);

CProjectViewTypeDescriptor s_MPCrossAlignViewTypeDescr(
    "Multi-pane Cross Alignment View", // type name
    "mp_cross_align_view", // icon alias TODO
    "Multi-pane Cross Alignment View",
    "The Multi-pane Cross Alignment View shows a multi-pane cross alignment",
    "MP_CROSS_ALIGN_VIEW", // help ID
    "Alignment",   // category
    false,       // not a singleton
    "Seq-align",
    eSimilarObjectsAccepted
);

CMPCrossAlignView::CMPCrossAlignView() : m_MainSplitter()
{
}


wxWindow* CMPCrossAlignView::GetWindow()
{
    _ASSERT(m_MainSplitter);
    return m_MainSplitter;
}


void CMPCrossAlignView::x_CreateMenuBarMenu()
{
    //CUICommandRegistry& cmd_reg = m_Workbench->GetUICommandRegistry();
}


void CMPCrossAlignView::CreateViewWindow( wxWindow* parent )
{
    _ASSERT(!m_MainSplitter);

    m_MainSplitter = new wxSplitterWindow( parent, wxID_ANY );
}


void CMPCrossAlignView::DestroyViewWindow()
{
    //m_Views.clear();

    if( m_MainSplitter ){
        m_MainSplitter->Destroy();
        m_MainSplitter = NULL;
    }
}


void CMPCrossAlignView::DestroyView()
{
    IWindowManagerService* windowManager = m_Workbench->GetWindowManagerService();

    NON_CONST_ITERATE( TViews, it, m_Views ){
        windowManager->UnRegisterActiveClient(**it);
        (*it)->DestroyView();
        (*it)->SetWorkbench(NULL);
    }
    m_Views.clear();

    CProjectView::DestroyView();
}


const CViewTypeDescriptor& CMPCrossAlignView::GetTypeDescriptor() const
{
    return s_MPCrossAlignViewTypeDescr;
}

void CMPCrossAlignView::SetWorkbench( IWorkbench* workbench )
{
    CProjectViewBase::SetWorkbench(workbench);
}

bool CMPCrossAlignView::InitView( TConstScopedObjects& objects, const objects::CUser_object* )
{
    if( objects.size() == 0 ){
        return false;
    }

    const CObject* object = objects[0].object.GetPointer();
    CScope* scope = objects[0].scope.GetPointer();

    for(;;){
        const CSeq_annot* annot = dynamic_cast<const CSeq_annot*>(object);
        if( annot ){
            m_OrigAnnot.Reset( annot );
            break;
        }

        const CSeq_align* align = dynamic_cast<const CSeq_align*>(object);
        if( align ){
            m_OrigAlign.Reset( align );
            break;
        }

/*
        const CBioseq* bioseq = dynamic_cast<const CBioseq*>(object);
        if( bioseq ){
            m_OrigBioseq.Reset( bioseq );
            break;
        }
*/

        break;
    }

    if( ! m_OrigAlign  &&  ! m_OrigAnnot ){
        x_ReportInvalidInputData( objects );

        return false;
    }

    /*
    vector< CRef<CSeq_align> > input_aligns;

    ITERATE( TConstScopedObjects, cso_it, objects ){
        const CObject* object = cso_it->object.GetPointer();
        CConstRef<CSeq_align> ref;

        const CSeq_annot* annot = dynamic_cast<const CSeq_annot*>(object);
        if( annot ){
            CTypeConstIterator<CSeq_align> it( *annot );
            while( it ){
                ref.Reset( &*it );
                input_aligns.push_back( ref );
                ++it;
            }
        }

        const CSeq_align* align = dynamic_cast<const CSeq_align*>(object);
        if( align ){
            ref.Reset( align );
            input_aligns.push_back( ref );
        }

        const CBioseq* bioseq = dynamic_cast<const CBioseq*>(object);
        if( bioseq ){
            CTypeConstIterator<CSeq_align> it( *bioseq );
            while( it ){
                ref.Reset( &*it );
                input_aligns.push_back( ref );
                ++it;
            }
        }
    }
    */

    _ASSERT( m_MainSplitter );

    wxSplitterWindow* splitter = new wxSplitterWindow( m_MainSplitter, wxID_ANY );

    IViewManagerService* manager = m_Workbench->GetViewManagerService();
    
    TViews views;
    CIRef<IView> view;

    static const char* view_names[] = {
        "Dot Matrix View",
        "Graphical Sequence View",
        "Graphical Sequence View"
    };

    wxWindow* parent = m_MainSplitter;
    for( size_t ix = 0; ix < sizeof(view_names)/sizeof(const char*); ix++ ){
        view = manager->CreateViewInstance( view_names[ix] );
        CProjectView* prj_view = dynamic_cast<CProjectView*>( &*view );
        if( !prj_view ) return false;
        prj_view->SetVisibleRangeService(this);

        // Use GBench frame window as a parent
        // otherwise an OpenGL window may be "non-visible"
        // resulting in crash on Linux. JIRA: GB-3628
        prj_view->CreateViewWindow( m_MainSplitter->GetParent() );
        wxWindow* wnd = prj_view->GetWindow();
        if (wnd) wnd->Reparent(parent);

        views.push_back (CIRef<IProjectView>(prj_view));

        parent = splitter;
    }

    m_MainSplitter->SplitVertically( views[0]->GetWindow(), splitter );
    splitter->SplitHorizontally( views[1]->GetWindow(), views[2]->GetWindow() );

    m_MainSplitter->SetMinimumPaneSize( 40 );
    splitter->SetMinimumPaneSize( 40 );

    vector<CSeq_id_Handle> ids;
    const CSerialObject& so(
        m_OrigAnnot 
        ? (const CSerialObject&)(*m_OrigAnnot) 
        : (const CSerialObject&)(*m_OrigAlign)
    );
    
    CTypeConstIterator<CSeq_id> id_iter(so);
    for( ;  id_iter  &&  ids.size() < 2;  ++id_iter ){
        CSeq_id_Handle idh = CSeq_id_Handle::GetHandle( *id_iter );
        if( std::find( ids.begin(), ids.end(), idh ) == ids.end() ){
            ids.push_back( idh );
        }
    }

    views[0]->SetWorkbench( m_Workbench );
    // Init Dot Matrix View
    views[0]->InitView( objects, 0 );
    
    views[1]->SetWorkbench( m_Workbench );
    // Init the first Graphical View
    SConstScopedObject seq1( ids.front().GetSeqId(), scope );
    TConstScopedObjects seq1_objs;
    seq1_objs.push_back( seq1 );
    views[1]->InitView( seq1_objs, 0 );

    views[2]->SetWorkbench( m_Workbench );
    // Init the second Graphical View
    SConstScopedObject seq2( ids.back().GetSeqId(), scope );
    TConstScopedObjects seq2_objs;
    seq2_objs.push_back( seq2 );
    views[2]->InitView( seq2_objs, 0 );

    m_Views = views;

    IWindowManagerService* windowManager = m_Workbench->GetWindowManagerService();
    for (size_t i = 0; i < m_Views.size(); ++i)
        windowManager->RegisterActiveClient(*m_Views[i]);

    windowManager->UnRegisterActiveClient(*this);

    CIRef<CProjectService> srv = m_Workbench->GetServiceByType<CProjectService>();
    _ASSERT(srv);

    {
        CRef<CGBWorkspace> ws = srv->GetGBWorkspace();
        if (!ws) return false;

        CGBDocument* doc = dynamic_cast<CGBDocument*>(ws->GetProjectFromScope(*scope));
        _ASSERT(doc);

        x_AttachToProject(*doc);
    }

    // fix HM view visibility issue on Mac
    // TODO: there should be a better way, graphics view is the only
    // OpenGL view not exposing this behavior
    #ifdef __WXMAC__
        views[0]->GetWindow()->Hide();
        views[0]->GetWindow()->Show();
    #endif

    x_UpdateContentLabel();

    windowManager->ActivateClient(*m_Views[0]);

    return true;
}


void CMPCrossAlignView::OnProjectChanged()
{
    x_UpdateContentLabel();
}


void CMPCrossAlignView::SetSelection(CSelectionEvent& evt)
{
    NON_CONST_ITERATE( TViews, it, m_Views ){
        (*it)->SetSelection( evt );
    }
}


void CMPCrossAlignView::GetSelection(TConstScopedObjects& objs) const
{
    ITERATE( TViews, it, m_Views ){
        (*it)->GetSelection(objs);
    }
}


void CMPCrossAlignView::GetMainObject(TConstScopedObjects& objs) const
{
    if(m_MainSplitter  &&  x_HasProject())    {
        CScope* scope = x_GetScope();
        const CObject* obj = x_GetOrigObject();

        _ASSERT(obj  &&  scope);

        objs.push_back(SConstScopedObject(obj, scope));
    }
}


void CMPCrossAlignView::GetSelection(CSelectionEvent& evt) const
{
    ITERATE( TViews, it, m_Views ){
        (*it)->GetSelection(evt);
    }
}


// handles incoming selection broadcast (overriding CView virtual function)
void CMPCrossAlignView::x_OnSetSelection(CSelectionEvent& evt)
{
}


void CMPCrossAlignView::GetVisibleRanges(CVisibleRange& vrange) const
{
}

void CMPCrossAlignView::BroadcastVisibleRange(const CVisibleRange& vrange,
                                              IVisibleRangeClient* source)
{
    CVisibleRange vrange2 = vrange;
    vrange2.SetVisibleRangePolicy(CVisibleRange::eBasic_Slave);

    NON_CONST_ITERATE (TViews, it, m_Views) {
        IVisibleRangeClient* view = dynamic_cast<IVisibleRangeClient*>(&**it);
        if (view != source)
            view->OnVisibleRangeChanged(vrange2, source);
    }
}

void CMPCrossAlignView::OnVisibleRangeChanged(const CVisibleRange& vrange, IVisibleRangeClient* source)
{
    NON_CONST_ITERATE (TViews, it, m_Views) {
        IVisibleRangeClient* view = dynamic_cast<IVisibleRangeClient*>(&**it);
        view->OnVisibleRangeChanged(vrange, source);
    }
}


const CObject* CMPCrossAlignView::x_GetOrigObject() const
{
    const CObject* obj = m_OrigAnnot ? (const CObject*) m_OrigAnnot.GetPointer()
                                     : m_OrigAlign.GetPointer();
    return obj;
}


///////////////////////////////////////////////////////////////////////////////
/// CMPCrossAlignViewFactory
string  CMPCrossAlignViewFactory::GetExtensionIdentifier() const
{
    static string sid("mp_cross_align_view_factory");
    return sid;
}


string CMPCrossAlignViewFactory::GetExtensionLabel() const
{
    static string slabel("Multi-pane Cross Alignment View Factory");
    return slabel;
}


void CMPCrossAlignViewFactory::RegisterIconAliases(wxFileArtProvider& provider)
{
    string alias = GetViewTypeDescriptor().GetIconAlias();
    provider.RegisterFileAlias(ToWxString(alias), wxT("cross_align_view.png"));
}

void CMPCrossAlignViewFactory::RegisterCommands (CUICommandRegistry& cmd_reg, wxFileArtProvider& provider)
{
}

const CProjectViewTypeDescriptor& CMPCrossAlignViewFactory::GetProjectViewTypeDescriptor() const
{
    return s_MPCrossAlignViewTypeDescr;
}


IView* CMPCrossAlignViewFactory::CreateInstance() const
{
    return new CMPCrossAlignView();
}


IView* CMPCrossAlignViewFactory::CreateInstanceByFingerprint(const TFingerprint& fingerprint) const
{
    /*TFingerprint print(CMPCrossAlignView::m_TypeDescr.GetLabel(), false);
    if(print == fingerprint)   {
        return new CMPCrossAlignView();
    }*/
    return NULL;
}

bool CMPCrossAlignViewFactory::IsCompatibleWith( const CObject& object, objects::CScope& )
{
    const type_info& type = typeid(object);

    return
        typeid(CSeq_align) == type 
        //|| typeid(CSeq_annot) == type
    ;
}

int CMPCrossAlignViewFactory::TestInputObjects( TConstScopedObjects& objects )
{
    bool found_good = false;
    bool found_bad = false;
    for( size_t i = 0; i < objects.size(); i++ ){
        if( IsCompatibleWith( *objects[i].object, *objects[i].scope ) ){
            found_good = true;
        } else {
            found_bad = true;
        }
    }

    if( found_good ){
        return fCanShowSeparated | (found_bad ? fCanShowSome : fCanShowAll);
    }
    return 0; // can show nothing
}

END_NCBI_SCOPE
