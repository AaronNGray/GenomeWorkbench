/*  $Id: annot_compare_view.cpp 40880 2018-04-25 20:26:58Z katargir $
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
 * Authors:  Mike DiCuccio, Vladimir Tereshkov
 *
 * File Description:
 *    
 *    
 */

#include <ncbi_pch.hpp>

#include <gui/packages/pkg_alignment/annot_compare_view.hpp>

#include <gui/core/project_service.hpp>

#include <gui/framework/workbench.hpp>

#include <gui/widgets/wx/fileartprov.hpp>
#include <gui/widgets/wx/ui_command.hpp>
#include <gui/widgets/wx/wx_utils.hpp>

#include <gui/objutils/utils.hpp>
#include <gui/objutils/label.hpp>
#include <gui/objutils/sparse_functions.hpp>

#include <serial/iterator.hpp>

#include <objects/seqloc/Seq_interval.hpp>

#include <wx/menu.h>

BEGIN_NCBI_SCOPE
USING_SCOPE(objects);

CProjectViewTypeDescriptor s_AnnotCompareViewTypeDescr(
    "Annotation Comparison View", // type name
    "annot_compare_view", // icon alias TODO
    "Annotation Comparison View",
    "Comparison of annotations, either through an alignment or in different annotation sets",
    "ANNOT_COMPARE_VIEW", // help ID
    "Alignment",   // category
    false,       // not a singleton
    "Seq-align",
    eOneObjectAccepted
);


CAnnotCompareView::CAnnotCompareView()
:   m_AnnWidget(NULL)
{
}


wxWindow* CAnnotCompareView::GetWindow()
{
    _ASSERT(m_AnnWidget);
    return m_AnnWidget;
}


void CAnnotCompareView::x_CreateMenuBarMenu()
{
    wxMenu* view_menu = new wxMenu();
    wxMenu* dm_menu = new wxMenu();

//    cmd_reg.AppendMenuItem(*dm_menu, eCmdChooseSeq);
 //   cmd_reg.AppendMenuItem(*dm_menu, eCmdColorByScore);

    view_menu->Append(wxID_ANY, wxT("Annotation Comparison"), dm_menu);
    
    wxMenu* bar_menu = new wxMenu();
    bar_menu->Append( wxID_ANY, wxT("&View"), view_menu );

    m_MenuBarMenu.reset( bar_menu );
}


void CAnnotCompareView::CreateViewWindow(wxWindow* parent)
{
    _ASSERT( ! m_AnnWidget);

    m_AnnWidget = new CAnnotCompareWidget();
   //  m_AnnWidget->Create();
//
   /// AddListener(m_AnnWidget, ePool_Child);
   // m_AnnWidget->AddListener(this, ePool_Parent);
}


void CAnnotCompareView::DestroyViewWindow()
{
    if(m_AnnWidget) {
      //  m_AnnWidget->Destroy();
      //  m_AnnWidget = NULL;
    }
}


const CViewTypeDescriptor& CAnnotCompareView::GetTypeDescriptor() const
{
    return s_AnnotCompareViewTypeDescr;
}

void CAnnotCompareView::x_InitWidget()
{
    /*
    _ASSERT( !m_Aligns.empty() );

    CScope* scope = GetScope().GetPointer();
    _ASSERT(scope);

    m_AnnWidget->SetDataSource( NULL );

    CHitMatrixDSBuilder builder;
    builder.Init( *scope, m_Aligns );

    m_DataSource = builder.CreateDataSource();

    if( m_DataSource->SelectDefaultIds() ){
        m_AnnWidget->SetDataSource( m_DataSource.GetPointer() );
    }
    */
}

objects::CScope* CAnnotCompareView::x_PreAttachToProject( TConstScopedObjects& objects )
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
 
void CAnnotCompareView::x_PostAttachToProject()
{
    if( x_HasProject() ){
        x_InitWidget();
    }

    x_UpdateContentLabel();
}

void CAnnotCompareView::OnProjectChanged( CProjectViewEvent& evt )
{
    if( 
        evt.GetType() == CViewEvent::eProjectChanged
        && (evt.GetSubtype() & CProjectViewEvent::eProperties)
    ){
        x_UpdateContentLabel();
    }
}

void CAnnotCompareView::GetSelection(TConstScopedObjects& objs) const
{
    /*
    if (m_AnnWidget  &&  x_HasProject())    {
        typedef CHitMatrixWidget::TRangeColl TRangeColl;
        CScope* scope = x_GetScope();

        // Range selection
        const TRangeColl& s_coll = m_AnnWidget->GetSubjectRangeSelection();
        if(! s_coll.empty())  {
            CRef<CSeq_loc> s_loc =
                CSeqUtils::CreateSeq_loc(*m_DataSource->GetSubjectId().GetSeqId(), s_coll);
            if (s_loc) {
                objs.push_back(SConstScopedObject(*s_loc, *scope));
            }
        }

        const TRangeColl& q_coll = m_AnnWidget->GetQueryRangeSelection();
        if(! q_coll.empty())    {
            CRef<CSeq_loc> q_loc =
                CSeqUtils::CreateSeq_loc(*m_DataSource->GetQueryId().GetSeqId(), q_coll);
            if (q_loc) {
                objs.push_back(SConstScopedObject(*q_loc, *scope));
            }
        }

        // Object Selection
        TConstObjects sel_objs;
        m_AnnWidget->GetObjectSelection(sel_objs);
        ITERATE(TConstObjects, it, sel_objs)    {
            objs.push_back(SConstScopedObject(**it, *scope));
        }
    }
    */
}


void CAnnotCompareView::GetSelection(CSelectionEvent& evt) const
{
    /*
    if (x_HasProject()) {
        // Range Selection
        evt.AddRangeSelection(*m_DataSource->GetSubjectId().GetSeqId(),
                              m_AnnWidget->GetSubjectRangeSelection());

        evt.AddRangeSelection(*m_DataSource->GetQueryId().GetSeqId(),
                              m_AnnWidget->GetQueryRangeSelection());

        // Object Selection
        TConstObjects sel_objs;
        m_AnnWidget->GetObjectSelection(sel_objs);
        evt.AddObjectSelection(sel_objs);
    }
    */
}


// handles incoming selection broadcast (overriding CView virtual function)
void CAnnotCompareView::x_OnSetSelection(CSelectionEvent& evt)
{
    /*
    CScope* scope = x_GetScope();

    _ASSERT(scope);

    if(evt.HasRangeSelection())     { // set Range Selection
        typedef CHitMatrixWidget::TRangeColl TRangeColl;
        CSelectionEvent::TRangeColl coll;

        // set Range Selection on Subject
        CConstRef<CSeq_id>  id = m_DataSource->GetSubjectId().GetSeqId();
        evt.GetRangeSelection(*id, *scope, coll);
        if (coll.size()) {
            m_AnnWidget->SetSubjectRangeSelection(coll);
        }

        // ser Range Selection on Query
        coll.clear();
        id = m_DataSource->GetQueryId().GetSeqId();
        evt.GetRangeSelection(*id, *scope, coll);
        if (coll.size()) {
            m_AnnWidget->SetQueryRangeSelection(coll);
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
            m_AnnWidget->SetObjectSelection(sel_aligns);
        }
    }
    */
}


void CAnnotCompareView::GetVisibleRanges(CVisibleRange& vrange) const
{
    /// collect information
    /*
    const CGlPane& pane = m_AnnWidget->GetPort();
    TModelRect rect = pane.GetVisibleRect();

    CRef<CSeq_loc> loc(new CSeq_loc());
    loc->SetInt().SetFrom((TSeqPos)rect.Left());
    loc->SetInt().SetTo  ((TSeqPos)rect.Right());
    loc->SetId(*m_DataSource->GetSubjectId().GetSeqId());
    evt.AddLocation(loc);

    loc.Reset(new CSeq_loc());
    loc->SetInt().SetFrom((TSeqPos)rect.Bottom());
    loc->SetInt().SetTo  ((TSeqPos)rect.Top());
    loc->SetId(*m_DataSource->GetQueryId().GetSeqId());
    evt.AddLocation(loc);
    */
}

const CObject* CAnnotCompareView::x_GetOrigObject() const
{
    /*
    if(m_Aligns.size() == 1)    {
        return m_Aligns[0].GetPointer();
    }
    */
    return NULL;
}


///////////////////////////////////////////////////////////////////////////////
/// CAnnotCompareViewFactory
string  CAnnotCompareViewFactory::GetExtensionIdentifier() const
{
    static string sid("annot_compare_view_factory");
    return sid;
}


string CAnnotCompareViewFactory::GetExtensionLabel() const
{
    static string slabel("Annotation Compare View Factory");
    return slabel;
}


void CAnnotCompareViewFactory::RegisterIconAliases(wxFileArtProvider& provider)
{
    string alias = GetViewTypeDescriptor().GetIconAlias();
    provider.RegisterFileAlias(ToWxString(alias), wxT("annot_compare_view.png"));
}

const CProjectViewTypeDescriptor& CAnnotCompareViewFactory::GetProjectViewTypeDescriptor() const
{
    return s_AnnotCompareViewTypeDescr;
}


IView* CAnnotCompareViewFactory::CreateInstance() const
{
    return new CAnnotCompareView();
}


IView* CAnnotCompareViewFactory::CreateInstanceByFingerprint(const TFingerprint& fingerprint) const
{
    /*TFingerprint print(CAnnotCompareView::m_TypeDescr.GetLabel(), false);
    if(print == fingerprint)   {
        return new CAnnotCompareView();
    }*/
    return NULL;
}



int CAnnotCompareViewFactory::TestInputObjects(TConstScopedObjects& objects)
{
    bool found_good = false;
    bool found_bad = false;
    for(  size_t i = 0;  i < objects.size();  i++)  {
        const CObject* obj = objects[i].object;
        const type_info& type = typeid(*obj);
        if(typeid(CSeq_annot) == type  ||  typeid(CSeq_align) == type)  {
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
