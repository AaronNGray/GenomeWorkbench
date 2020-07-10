/*  $Id: dot_matrix_view.cpp 43750 2019-08-28 16:05:58Z katargir $
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

#include <gui/packages/pkg_alignment/dot_matrix_view.hpp>

#include <gui/widgets/hit_matrix/hit_matrix_widget.hpp>
#include <gui/widgets/hit_matrix/hit_matrix_ds_builder.hpp>
#include <gui/widgets/hit_matrix/dense_ds.hpp>

#include <gui/objutils/sparse_functions.hpp>

#include <gui/core/project_service.hpp>
#include <gui/objutils/visible_range.hpp>

#include <gui/framework/workbench.hpp>

#include <gui/widgets/wx/fileartprov.hpp>
#include <gui/widgets/wx/ui_command.hpp>
#include <gui/widgets/wx/table_listctrl.hpp>
#include <gui/widgets/wx/async_call.hpp>

#include <gui/objutils/utils.hpp>
#include <gui/objutils/label.hpp>

#include <serial/iterator.hpp>
#include <objmgr/util/sequence.hpp>

//#include <objects/seqloc/Seq_id.hpp>
#include <objects/seqloc/Seq_interval.hpp>

#include <wx/menu.h>
#include <wx/sizer.h>

BEGIN_NCBI_SCOPE
USING_SCOPE(objects);

CProjectViewTypeDescriptor CDotMatrixView::m_TypeDescr(
    "Dot Matrix View", // type name
    "dot_matrix_view", // icon alias TODO
    "Dot Matrix View",
    "The Dot Matrix View shows a dot-plot of a given alignment",
    "DOT_MATRIX_VIEW", // help ID
    "Alignment",   // category
    false,       // not a singleton
    "Seq-align",
    eSimilarObjectsAccepted
);


CDotMatrixView::CDotMatrixView()
:   m_AlnWidget(NULL)
{
}


wxWindow* CDotMatrixView::GetWindow()
{
    _ASSERT(m_AlnWidget);
    return m_AlnWidget;
}


void CDotMatrixView::x_CreateMenuBarMenu()
{
    CUICommandRegistry& cmd_reg = m_Workbench->GetUICommandRegistry();

    wxMenu* view_menu = new wxMenu();
    wxMenu* dm_menu = new wxMenu();

    cmd_reg.AppendMenuItem(*dm_menu, eCmdChooseSeq);
    cmd_reg.AppendMenuItem(*dm_menu, eCmdColorByScore);

    view_menu->Append(wxID_ANY, wxT("Dot Matrix"), dm_menu);

    wxMenu* bar_menu = new wxMenu();
    bar_menu->Append( wxID_ANY, wxT("&View"), view_menu );

    m_MenuBarMenu.reset( bar_menu );
}


void CDotMatrixView::CreateViewWindow(wxWindow* parent)
{
    _ASSERT( ! m_AlnWidget);

    m_AlnWidget = new CHitMatrixWidget(parent);
    m_AlnWidget->Create();

    AddListener(m_AlnWidget, ePool_Child);
    m_AlnWidget->AddListener(this, ePool_Parent);
}


void CDotMatrixView::DestroyViewWindow()
{
    if(m_AlnWidget) {
        m_AlnWidget->Destroy();
        m_AlnWidget = NULL;
    }
}


const CViewTypeDescriptor& CDotMatrixView::GetTypeDescriptor() const
{
    return m_TypeDescr;
}

void CDotMatrixView::x_InitWidget()
{
    _ASSERT( !m_Aligns.empty() );

    CScope* scope = GetScope().GetPointer();
    _ASSERT(scope);

    m_AlnWidget->SetDataSource( NULL );

    CHitMatrixDSBuilder builder;
    builder.Init( *scope, m_Aligns );

    //m_DataSource = builder.CreateDataSource();
    m_DataSource = GUI_AsyncExec([&builder](ICanceled&) { return builder.CreateDataSource(); }, wxT("Creating data source..."));

    if( m_DataSource->SelectDefaultIds() ){
        m_AlnWidget->SetDataSource( m_DataSource.GetPointer() );
    }
}

objects::CScope* CDotMatrixView::x_PreAttachToProject( TConstScopedObjects& objects )
{
    TAlignVector aligns;

    if( objects.size() > 0 ){
        NON_CONST_ITERATE( TConstScopedObjects, it, objects ){
            const CObject* object = it->object.GetPointer();
            const CSeq_annot* annot = dynamic_cast<const CSeq_annot*>(object);
            if( annot ){
                ExtractSeqAligns( *annot, aligns );
            } else {
                const CSeq_align* align = dynamic_cast<const CSeq_align*>(object);
                aligns.push_back( CConstRef<CSeq_align>(align) );
            }
        }
    }

    if( aligns.size() > 0 ){

        m_Aligns = aligns;

        return objects[0].scope.GetPointer();
    }

    return NULL;
}
 
void CDotMatrixView::x_PostAttachToProject()
{
    if( x_HasProject() ){
        x_InitWidget();
    }

    x_UpdateContentLabel();
}

void CDotMatrixView::OnProjectChanged( CProjectViewEvent& evt )
{
    if( 
        evt.GetType() == CViewEvent::eProjectChanged
        && (evt.GetSubtype() & CProjectViewEvent::eProperties)
    ){
        x_UpdateContentLabel();
    }
}

void CDotMatrixView::GetSelection(TConstScopedObjects& objs) const
{
    if (m_AlnWidget  &&  x_HasProject())    {
        typedef CHitMatrixWidget::TRangeColl TRangeColl;
        CScope* scope = x_GetScope();

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


void CDotMatrixView::GetSelection(CSelectionEvent& evt) const
{
    if (x_HasProject()) {
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
void CDotMatrixView::x_OnSetSelection(CSelectionEvent& evt)
{
    CScope* scope = x_GetScope();

    _ASSERT(scope);

    if(evt.HasRangeSelection())     { // set Range Selection
        typedef CHitMatrixWidget::TRangeColl TRangeColl;
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

    if (&evt.GetScope() != scope)
        return;

    vector<const CSeq_align*>   sel_aligns;
    if (evt.HasObjectSelection())    { // set Object Selection
        TConstObjects sel_objs;
        evt.GetAllObjects(sel_objs);

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
    }
    m_AlnWidget->SetObjectSelection(sel_aligns);
}


void CDotMatrixView::GetVisibleRanges(CVisibleRange& vrange) const
{
    if( m_DataSource.IsNull() ){
        ERR_POST(Error << "CDotMatrixView: no data source while trying to GetVisibleRanges()" );
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


void CDotMatrixView::OnVisibleRangeChanged(const CVisibleRange& vrange, IVisibleRangeClient* source)
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


const CObject* CDotMatrixView::x_GetOrigObject() const
{
    if( m_Aligns.size() > 0 ){
        return m_Aligns[0].GetPointer();
    }
    return NULL;
}


///////////////////////////////////////////////////////////////////////////////
/// CDotMatrixViewFactory
CDotMatrixViewFactory::CDotMatrixViewFactory()
{
    //m_ViewManager = new CDotMatrixViewManager( *this );
}

string  CDotMatrixViewFactory::GetExtensionIdentifier() const
{
    static string sid("dot_matrix_view_factory");
    return sid;
}

string CDotMatrixViewFactory::GetExtensionLabel() const
{
    static string slabel("Dot Matrix View Factory");
    return slabel;
}

void CDotMatrixViewFactory::RegisterIconAliases(wxFileArtProvider& provider)
{
    string alias = GetViewTypeDescriptor().GetIconAlias();
    provider.RegisterFileAlias(ToWxString(alias), wxT("dot_matrix_view.png"));
}

void CDotMatrixViewFactory::RegisterCommands (CUICommandRegistry& cmd_reg, wxFileArtProvider& provider)
{
    CHitMatrixWidget::RegisterCommands(cmd_reg, provider);
}

const CProjectViewTypeDescriptor& CDotMatrixViewFactory::GetProjectViewTypeDescriptor() const
{
    return CDotMatrixView::m_TypeDescr;
}

IView* CDotMatrixViewFactory::CreateInstance() const
{
    return new CDotMatrixView();
}

IView* CDotMatrixViewFactory::CreateInstanceByFingerprint(const TFingerprint& fingerprint) const
{
    /*TFingerprint print(CDotMatrixView::m_TypeDescr.GetLabel(), false);
    if(print == fingerprint)   {
        return new CDotMatrixView();
    }*/
    return NULL;
}

int CDotMatrixViewFactory::TestInputObjects( TConstScopedObjects& objects )
{
    bool found_good = false;
    bool found_bad = false;
    for( size_t i = 0; i < objects.size(); i++ ){
        const CObject* obj = objects[i].object;
        const type_info& type = typeid(*obj);

        if (typeid(CSeq_annot) == type) {
            const CSeq_annot* annot = dynamic_cast<const CSeq_annot*>(obj);
            vector< CConstRef<objects::CSeq_align> > aligns;
            ExtractSeqAligns(*annot, aligns);
            if (aligns.size() > 0)
                found_good = true;
            else
                found_bad = true;
        }
        else if (typeid(CSeq_align) == type)
            found_good = true;
        else
            found_bad = true;
    }
    if( found_good ){
        return fCanShowSeparated | (found_bad ? fCanShowSome : fCanShowAll);
    }
    return 0; // can show nothing
}

vector<int> CDotMatrixViewFactory::CombineInputObjects(const TConstScopedObjects& objects)
{
    size_t num_objects = objects.size();
    if (num_objects == 0)
        return vector<int>();
    else
        return vector<int>(num_objects, 0);
}

END_NCBI_SCOPE
