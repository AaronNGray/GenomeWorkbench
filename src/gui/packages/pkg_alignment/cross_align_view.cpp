/*  $Id: cross_align_view.cpp 39411 2017-09-21 17:43:20Z katargir $
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
 *    User-modifiable implementation file for extension of Dot Matrix
 *    viewer for GBENCH
 */

#include <ncbi_pch.hpp>

#include <gui/packages/pkg_alignment/cross_align_view.hpp>

#include <gui/widgets/aln_crossaln/cross_aln_widget.hpp>
#include <gui/widgets/hit_matrix/hit_matrix_ds_builder.hpp>

#include <gui/core/project_service.hpp>
#include <gui/objutils/visible_range.hpp>

#include <gui/framework/workbench.hpp>

#include <gui/widgets/wx/fileartprov.hpp>
#include <gui/widgets/wx/ui_command.hpp>
#include <gui/widgets/wx/wx_utils.hpp>

#include <gui/objutils/utils.hpp>
#include <gui/objutils/label.hpp>

#include <objects/seqloc/Seq_interval.hpp>

#include <wx/menu.h>

BEGIN_NCBI_SCOPE
USING_SCOPE(objects);

static CProjectViewTypeDescriptor s_CrossAlignViewTypeDescr(
    "Cross Align View", // type name
    "cross_align_view", // icon alias TODO
    "Cross Align View",
    "The Cross Align View shows a cross alignment",
    "CROSS_ALIGN_VIEW", // help ID
    "Alignment",   // category
    false,       // not a singleton
    "Seq-align",
    eOneObjectAccepted
);

CCrossAlignView::CCrossAlignView() : m_AlnWidget()
{
}


wxWindow* CCrossAlignView::GetWindow()
{
    _ASSERT(m_AlnWidget);
    return m_AlnWidget;
}


void CCrossAlignView::x_CreateMenuBarMenu()
{
    CUICommandRegistry& cmd_reg = m_Workbench->GetUICommandRegistry();

    wxMenu* view_menu = new wxMenu();
    wxMenu* dm_menu = new wxMenu();

    cmd_reg.AppendMenuItem(*dm_menu, eCmdZoomInSubject);
    cmd_reg.AppendMenuItem(*dm_menu, eCmdZoomOutSubject);
    cmd_reg.AppendMenuItem(*dm_menu, eCmdZoomAllSubject);
    dm_menu->AppendSeparator();
    cmd_reg.AppendMenuItem(*dm_menu, eCmdZoomInQuery);
    cmd_reg.AppendMenuItem(*dm_menu, eCmdZoomOutQuery);
    cmd_reg.AppendMenuItem(*dm_menu, eCmdZoomAllQuery);
    dm_menu->AppendSeparator();
    cmd_reg.AppendMenuItem(*dm_menu, eCmdChooseSeq);

    view_menu->Append(wxID_ANY, wxT("Cross Alignment"), dm_menu);

    wxMenu* bar_menu = new wxMenu();
    bar_menu->Append( wxID_ANY, wxT("&View"), view_menu );

    m_MenuBarMenu.reset( bar_menu );
}


void CCrossAlignView::CreateViewWindow(wxWindow* parent)
{
    _ASSERT( ! m_AlnWidget);

    m_AlnWidget = new CCrossAlnWidget(parent);
    m_AlnWidget->Create();

    AddListener(m_AlnWidget, ePool_Child);
    m_AlnWidget->AddListener(this, ePool_Parent);
}


void CCrossAlignView::DestroyViewWindow()
{
    if (m_AlnWidget) {
        m_AlnWidget->Destroy();
        m_AlnWidget = NULL;
    }
}


const CViewTypeDescriptor& CCrossAlignView::GetTypeDescriptor() const
{
    return s_CrossAlignViewTypeDescr;
}


objects::CScope* CCrossAlignView::x_PreAttachToProject( TConstScopedObjects& objects )
{
    if( objects.size() != 1 ){
        return NULL;
    }


    CScope* scope = objects[0].scope.GetPointer();
    const CObject* object = objects[0].object.GetPointer();

    // extract the alignments from the argument
    const CSeq_annot* annot = dynamic_cast<const CSeq_annot*>( object );
    if( annot ){
        m_OrigAnnot.Reset( annot );
    } else {
        const CSeq_align* align = dynamic_cast<const CSeq_align*>( object );
        if( align ){
            m_OrigAlign.Reset( align );
        }
    }

    if( m_OrigAnnot || m_OrigAlign ){
        return scope;
    }

    return NULL;
}

void CCrossAlignView::x_PostAttachToProject()
{
    x_UpdateContentLabel();

    if( x_HasProject() ){
        _ASSERT(m_OrigAnnot || m_OrigAlign);

        m_AlnWidget->SetDataSource( NULL );

        CScope* scope = GetScope().GetPointer();
        _ASSERT(scope);

        CHitMatrixDSBuilder builder;
        if( m_OrigAnnot ){
            builder.Init( *scope, *m_OrigAnnot );
        
        } else if( m_OrigAlign ){
            builder.Init( *scope, *m_OrigAlign );
        }

        m_DataSource = builder.CreateDataSource();

        if( m_DataSource->SelectDefaultIds() ){
            m_AlnWidget->SetDataSource( m_DataSource.GetPointer() );
        }
    }
}

void CCrossAlignView::OnProjectChanged( CProjectViewEvent& evt )
{
    x_UpdateContentLabel();
}


void CCrossAlignView::GetSelection(TConstScopedObjects& objs) const
{
    if (m_AlnWidget  &&  x_HasProject())    {
        typedef CCrossAlnWidget::TRangeColl TRangeColl;

        CScope* scope = x_GetScope();

        _ASSERT(scope);

        // Range selection
        const TRangeColl& s_coll = m_AlnWidget->GetSubjectRangeSelection();
        if(! s_coll.empty())  {
            CRef<CSeq_loc> s_loc =
                CSeqUtils::CreateSeq_loc(*m_DataSource->GetSubjectId().GetSeqId(), s_coll);
            if (s_loc) {
                objs.push_back(SConstScopedObject(s_loc, scope));
            }
        }

        const TRangeColl& q_coll = m_AlnWidget->GetQueryRangeSelection();
        if(! q_coll.empty())    {
            CRef<CSeq_loc> q_loc =
                CSeqUtils::CreateSeq_loc(*m_DataSource->GetQueryId().GetSeqId(), q_coll);
            if (q_loc) {
                objs.push_back(SConstScopedObject(q_loc, scope));
            }
        }

        // Object Selection
        TConstObjects sel_objs;
        m_AlnWidget->GetObjectSelection(sel_objs);
        ITERATE(TConstObjects, it, sel_objs)    {
            objs.push_back(SConstScopedObject(*it, scope));
        }
    }
}


void CCrossAlignView::GetSelection(CSelectionEvent& evt) const
{
    if(x_HasProject()) {
        // Range Selection
        evt.AddRangeSelection(*m_DataSource->GetSubjectId().GetSeqId(),
                              m_AlnWidget->GetSubjectRangeSelection());

        evt.AddRangeSelection(*m_DataSource->GetQueryId().GetSeqId(),
                              m_AlnWidget->GetQueryRangeSelection());

        // Object Selection
        TConstObjects sel_objs;
        m_AlnWidget->GetObjectSelection(sel_objs);
        evt.AddObjectSelection(sel_objs);
    }
}


// handles incoming selection broadcast (overriding CView virtual function)
void CCrossAlignView::x_OnSetSelection(CSelectionEvent& evt)
{
    CScope* scope = x_GetScope();

    if(evt.HasRangeSelection())     { // set Range Selection
        CSelectionEvent::TRangeColl coll;

        // set Range Selection on Subject
        CConstRef<CSeq_id>  id = m_DataSource->GetSubjectId().GetSeqId();
        evt.GetRangeSelection(*id, *scope, coll);
        if (coll.size()) {
            m_AlnWidget->SetSubjectRangeSelection(coll);
        }

        // ser Range Selection on Query
        coll.clear();
        id = m_DataSource->GetQueryId().GetSeqId();
        evt.GetRangeSelection(*id, *scope, coll);
        if (coll.size()) {
            m_AlnWidget->SetQueryRangeSelection(coll);
        }
    }

    if(evt.HasObjectSelection())    { // set Object Selection
        TConstObjects sel_objs;
        evt.GetAllObjects(sel_objs);

        vector<const CSeq_align*>   sel_aligns;

        typedef CHitMatrixDataSource::THitAdapterCont THits;
        const THits& hits = m_DataSource->GetHits();
        ITERATE(THits, it_hit, hits)    {
            const IHit& hit = **it_hit;
            const CSeq_align& align = *hit.GetSeqAlign();

            ITERATE(TConstObjects, it_obj, sel_objs)    {
                if(evt.Match(align, *scope, **it_obj, evt.GetScope())) {
                    sel_aligns.push_back(&align);
                    break;
                }
            }
        }

        if (sel_aligns.size()) {
            m_AlnWidget->SetObjectSelection(sel_aligns);
        }
    }
}


void CCrossAlignView::GetVisibleRanges(CVisibleRange& vrange) const
{
    if( m_DataSource.IsNull() ){
        ERR_POST(Error << "CCrossAlignView: no data source while trying to GetVisibleRanges()" );
        return;
    }

    /// collect information
    const CGlPane& pane = m_AlnWidget->GetPort();
    TModelRect rect = pane.GetVisibleRect();

    CRef<CSeq_loc> loc(new CSeq_loc());
    loc->SetInt().SetFrom((TSeqPos)rect.Left());
    loc->SetInt().SetTo  ((TSeqPos)rect.Right());
    loc->SetId(*m_DataSource->GetSubjectId().GetSeqId());
    vrange.AddLocation(*loc);

    loc.Reset(new CSeq_loc());
    loc->SetInt().SetFrom((TSeqPos)rect.Bottom());
    loc->SetInt().SetTo  ((TSeqPos)rect.Top());
    loc->SetId(*m_DataSource->GetQueryId().GetSeqId());
    vrange.AddLocation(*loc);
}


void CCrossAlignView::OnVisibleRangeChanged(const CVisibleRange& vrange, IVisibleRangeClient* source)
{
    const CGlPane& pane = m_AlnWidget->GetPort();
    TModelRect rect = pane.GetVisibleRect();

    if (vrange.Match(*m_DataSource->GetQueryId().GetSeqId())) {
        TSeqRange range;
        range.SetFrom((TSeqPos)rect.Bottom());
        range.SetTo  ((TSeqPos)rect.Top());
        if (vrange.Clamp(m_DataSource->GetQueryHandle(), range)) {
            m_AlnWidget->SetQueryVisibleRange(range);
        }
    }

    if (vrange.Match(*m_DataSource->GetSubjectId().GetSeqId())) {
        TSeqRange range;
        range.SetFrom((TSeqPos)rect.Left());
        range.SetTo  ((TSeqPos)rect.Right());
        if (vrange.Clamp(m_DataSource->GetSubjectHandle(), range)) {
            m_AlnWidget->SetSubjectVisibleRange(range);
        }
    }
}


const CObject* CCrossAlignView::x_GetOrigObject() const
{
    const CObject* obj = m_OrigAnnot ? (const CObject*) m_OrigAnnot.GetPointer()
                                     : m_OrigAlign.GetPointer();
    return obj;
}


///////////////////////////////////////////////////////////////////////////////
/// CCrossAlignViewFactory
string  CCrossAlignViewFactory::GetExtensionIdentifier() const
{
    static string sid("cross_align_view_factory");
    return sid;
}


string CCrossAlignViewFactory::GetExtensionLabel() const
{
    static string slabel("Cross Align View Factory");
    return slabel;
}


void CCrossAlignViewFactory::RegisterIconAliases(wxFileArtProvider& provider)
{
    string alias = GetViewTypeDescriptor().GetIconAlias();
    provider.RegisterFileAlias(ToWxString(alias), wxT("cross_align_view.png"));
}

void CCrossAlignViewFactory::RegisterCommands (CUICommandRegistry& cmd_reg, wxFileArtProvider& provider)
{
    CCrossAlnWidget::RegisterCommands(cmd_reg, provider);
}

const CProjectViewTypeDescriptor& CCrossAlignViewFactory::GetProjectViewTypeDescriptor() const
{
    return s_CrossAlignViewTypeDescr;
}

IView* CCrossAlignViewFactory::CreateInstance() const
{
    return new CCrossAlignView();
}

IView* CCrossAlignViewFactory::CreateInstanceByFingerprint(const TFingerprint& fingerprint) const
{
    /*TFingerprint print(CCrossAlignView::m_TypeDescr.GetLabel(), false);
    if(print == fingerprint)   {
        return new CCrossAlignView();
    }*/
    return NULL;
}



int CCrossAlignViewFactory::TestInputObjects(TConstScopedObjects& objects)
{
    bool found_good = false;
    bool found_bad = false;
    for(  size_t i = 0;  i < objects.size();  i++)  {
        const CObject* obj = objects[i].object;
        const type_info& type = typeid(*obj);
        if( typeid(CSeq_align) == type ){
            found_good = true;
        } else {
            found_bad = true;
        }
    }
    if(found_good)   {
        return fCanShowSeparated | (found_bad ? fCanShowSome : fCanShowAll);
    }
    return 0; // can show nothing
}

END_NCBI_SCOPE
