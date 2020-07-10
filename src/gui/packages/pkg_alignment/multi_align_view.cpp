/*  $Id: multi_align_view.cpp 43123 2019-05-16 18:41:46Z katargir $
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
 */

#include <ncbi_pch.hpp>

#include <gui/packages/pkg_alignment/multi_align_view.hpp>

#include <gui/core/project_service.hpp>

#include <gui/framework/workbench.hpp>

#include <gui/objects/GBWorkspace.hpp>
#include <gui/objects/WorkspaceFolder.hpp>

#include <gui/widgets/aln_multiple/alnmulti_widget.hpp>

#include <gui/widgets/data/data_commands.hpp>

#include <gui/widgets/object_list/object_list_widget.hpp>
#include <gui/widgets/wx/fileartprov.hpp>
#include <gui/widgets/wx/ui_command.hpp>
#include <gui/widgets/wx/wx_utils.hpp>

#include <gui/objutils/utils.hpp>
#include <gui/objutils/label.hpp>

#include <objmgr/util/sequence.hpp>
#include <serial/iterator.hpp>

#include <wx/menu.h>


BEGIN_NCBI_SCOPE
USING_SCOPE(objects);




enum CMultiAlignViewCommands {
    eCmdUseSparseAln = 8300
};


///////////////////////////////////////////////////////////////////////////////
/// CMultiAlignView
CProjectViewTypeDescriptor s_MultiAlignViewTypeDescr(
    "Multiple Sequence Alignment View", // type name
    "multi_align_view", // icon alias TODO
    "Multiple Sequence Alignment View hint",
    "View for the multiple alignments with each column and residue colored according to its degree of mismatch",
    "MULTI_ALIGN_VIEW", // help ID
    "Alignment",        // category
    false,             // not a singleton
    "Seq-align",
    eAnyObjectsAccepted
);


BEGIN_EVENT_MAP(CMultiAlignView, CProjectView)
    ON_EVENT(CAlnMultiPane::CEvent, CAlnMultiPane::eLinearSelChanged,
             &CMultiAlignView::OnAlnWidgetSelChanged)
    ON_EVENT(CAlnMultiPane::CEvent, CAlnMultiPane::eSelRowChanged,
             &CMultiAlignView::OnAlnWidgetSelChanged)
    ON_EVENT(COpenGraphicViewEvent, CViewEvent::eOpenGraphicalViewEvent,
             &CMultiAlignView::OnOpenGraphicalView)
END_EVENT_MAP()


CMultiAlignView::CMultiAlignView()
:   m_Sparse(false),
    m_AlnWidget(NULL)
{
}


wxWindow* CMultiAlignView::GetWindow()
{
    _ASSERT(m_AlnWidget);
    return m_AlnWidget;
}

static
WX_DEFINE_MENU(kViewMenu)
    WX_SUBMENU("&View")
        WX_MENU_SEPARATOR_L("Contribs")
        WX_SUBMENU("Multiple Alignment")
            WX_MENU_ITEM(eCmdZoomSeq)
            WX_MENU_ITEM(eCmdZoomSel)
            WX_MENU_SEPARATOR()
            WX_MENU_ITEM(eCmdResetSelection)
            WX_MENU_ITEM(eCmdMoveSelectedUp)
            WX_MENU_SEPARATOR()
            WX_SUBMENU("Hide / Show")
                WX_MENU_ITEM(eCmdHideSelected)
                WX_MENU_ITEM(eCmdUnhideSelected)
                WX_MENU_ITEM(eCmdShowOnlySelected)
                WX_MENU_ITEM(eCmdShowAll)
            WX_END_SUBMENU()
            WX_SUBMENU("Master")
                WX_MENU_ITEM(eCmdSetSelMaster)
                WX_MENU_ITEM(eCmdUnsetMaster)
            WX_END_SUBMENU()
            WX_MENU_SEPARATOR()
            WX_SUBMENU("Coloring")
                WX_MENU_ITEM(eCmdAlnShowMethodsDlg)
                WX_MENU_ITEM(eCmdMethodProperties)
                WX_MENU_SEPARATOR()
            WX_END_SUBMENU()
        WX_END_SUBMENU()
    WX_END_SUBMENU()
    WX_SUBMENU("&Edit")
        WX_MENU_SEPARATOR_L("Contribs")
        WX_SUBMENU("Multiple Alignment")
            WX_MENU_ITEM(eCmdMarkSelected)
            WX_MENU_ITEM(eCmdUnMarkSelected)
            WX_MENU_ITEM(eCmdUnMarkAll)
        WX_END_SUBMENU()
    WX_END_SUBMENU()
WX_END_MENU()


void CMultiAlignView::x_CreateMenuBarMenu()
{
    CUICommandRegistry& cmd_reg = m_Workbench->GetUICommandRegistry();

    m_MenuBarMenu.reset(cmd_reg.CreateMenu(kViewMenu));
}

const SwxMenuItemRec* CMultiAlignView::GetMenuDef() const {

    return kViewMenu; 
}

void CMultiAlignView::CreateViewWindow(wxWindow* parent)
{
    _ASSERT( ! m_AlnWidget);

    m_AlnWidget = new CAlnMultiWidget(parent);

    m_AlnWidget->CreateWidget();

    AddListener(m_AlnWidget, ePool_Child);
    m_AlnWidget->AddListener(this, ePool_Parent);

    // setup display styles
    m_StyleCatalog.SetDefaultStyle(new CRowDisplayStyle());
    m_StyleCatalog.SetWidgetStyle(m_AlnWidget->GetDisplayStyle());

    m_AlnWidget->SetStyleCatalog(&m_StyleCatalog);
}


void CMultiAlignView::DestroyViewWindow()
{
    if(m_AlnWidget) {
        m_AlnWidget->Destroy();
        m_AlnWidget = NULL;
    }
}


const CViewTypeDescriptor& CMultiAlignView::GetTypeDescriptor() const
{
    return s_MultiAlignViewTypeDescr;
}

bool CMultiAlignView::x_GetValidInputData(const CObject* obj)
{
    // extract the alignments from the argument
    const CSeq_annot* annot = dynamic_cast<const CSeq_annot*>(obj);
    if(annot) {
        m_OrigAnnot.Reset(annot);
    } else {
        const CSeq_align* align = dynamic_cast<const CSeq_align*>(obj);
        if(align) {
            m_OrigAlign.Reset(align);
        }
    }

    return (m_OrigAnnot  ||  m_OrigAlign);
}

bool CMultiAlignView::InitView(TConstScopedObjects& objects, const objects::CUser_object*)
{
    x_MergeObjects( objects );

    CScope* scope = NULL;
    const CObject* obj = NULL;

    if(objects.size() == 1) {
        obj = objects[0].object.GetPointer();
        scope = objects[0].scope.GetPointer();
    }

    if(x_GetValidInputData(obj)) {
        CIRef<CProjectService> srv = m_Workbench->GetServiceByType<CProjectService>();
        _ASSERT(srv);

        CRef<CGBWorkspace> ws = srv->GetGBWorkspace();
        if (!ws) return false;

        CGBDocument* doc = dynamic_cast<CGBDocument*>(ws->GetProjectFromScope(*scope));
        if (doc) {
            x_AttachToProject(*doc);
            OnProjectChanged();
            return true;
        } else return false;
    } else {
        // cannot represent the data
        x_ReportInvalidInputData(objects);
        return false;
    }

}


void CMultiAlignView::OnProjectChanged()
{
    x_UpdateContentLabel();

    m_ObjIndex.Clear(NULL);

    if (x_HasProject()) {
        CScope* scope = x_GetScope();

        _ASSERT(scope);

        if (m_OrigAnnot) {
            m_Builder.Init(*scope, *m_OrigAnnot);
            m_Sparse =
                m_OrigAnnot->GetData().IsAlign()
                && m_OrigAnnot->GetData().GetAlign().front()->CanGetSegs() 
                &&(
                    m_OrigAnnot->GetData().GetAlign().front()->GetSegs().IsSparse()
                    || m_OrigAnnot->GetData().GetAlign().front()->GetSegs().IsSpliced()
                )
            ;
        } else if (m_OrigAlign) {
            m_Builder.Init(*scope, *m_OrigAlign);
            m_Sparse = m_OrigAlign->CanGetSegs() 
                &&(
                    m_OrigAlign->GetSegs().IsSparse()
                    || m_OrigAlign->GetSegs().IsSpliced()
                )
            ;
        }
        if (m_Sparse) {
            m_Builder.SetOptions().SetMergeFlags(CAlnUserOptions::fAllowTranslocation, true);
            m_Builder.SetOptions().SetMergeFlags(CAlnUserOptions::fSkipSortByScore, true);
        }
        m_Builder.SetListener(this);

        m_Builder.PreCreateDataSource(m_Sparse);
        m_DataSource = m_Builder.CreateDataSource();

        m_AlnWidget->SetDataSource(m_DataSource, scope);
    }
}


void CMultiAlignView::OnProjectChanged(CProjectViewEvent& evt)
{
    /*
    if(
        evt.GetSubtype() == CProjectViewEvent::eData  
        || evt.GetSubtype() == CProjectViewEvent::eBothPropsData
    ){
        // reset datasource to make sure the active job is deleted
        m_AlnWidget->GetModel()->SetDataSource(NULL);
        m_AlnWidget->SetDataSource(NULL);
        m_DataSource.Reset();
        OnProjectChanged();
        m_AlnWidget->Refresh();
    }
    */

    if(
        evt.GetSubtype() == CProjectViewEvent::eProperties  
        || evt.GetSubtype() == CProjectViewEvent::eBothPropsData
    ){
        x_UpdateContentLabel();
    }
}


// handles notifications from the Data Source
void CMultiAlignView::OnDSChanged(CDataChangeNotifier::CUpdate& update)
{
    if (m_AlnWidget) {
        m_AlnWidget->OnDSChanged(update);
    }

    if(update.GetType() == CDataChangeNotifier::eChanged) {
        x_RebuildObjectIndex();
        x_UpdateContentLabel();
    }
}


void CMultiAlignView::x_RebuildObjectIndex()
{
    _ASSERT(m_DataSource);

    CScope* scope = x_GetScope();
    m_ObjIndex.Clear(scope);

    // for every row in the alignment - register a {handle, id} pair
    IAlnExplorer::TNumrow n_rows = m_DataSource->GetNumRows();
    for( IAlnExplorer::TNumrow row = 0;  row < n_rows;  row++ )   {
        if(m_DataSource->CanGetId(row))    {
            const CSeq_id& id = m_DataSource->GetSeqId(row);
            m_ObjIndex.Add(new CSelRowHandle(row), id);
        }
    }
}

typedef vector<CConstRef<CSeq_id> > TRSeqIdVector;


void CMultiAlignView::x_GetRowSeqLoc(CAlnVec::TNumrow row,
                                     CSelectionEvent::TRangeColl& loc) const
{
    TSeqPos from = m_DataSource->GetSeqStart(row);
    TSeqPos to = m_DataSource->GetSeqStop(row);
    CAlnVec::TSignedRange total_r(from, to);

    IAlnSegmentIterator::EFlags flags = IAlnSegmentIterator::eSkipGaps;
    auto_ptr<IAlnSegmentIterator> p_it(m_DataSource->CreateSegmentIterator(row, total_r, flags));
    IAlnSegmentIterator& it = *p_it;

    for( ;  it;  ++it )  {
        const IAlnSegment::TSignedRange& aln_r = it->GetRange();
        CSelectionEvent::TRange r(aln_r.GetFrom(), aln_r.GetTo());
        loc.CombineWith(r);
    }
}


bool CMultiAlignView::x_GetSeqRangeSelection(CAlnMultiWidget::TRangeColl& seq_coll) const
{
    seq_coll.clear();
    if(m_DataSource.GetPointer()  &&  ! m_DataSource->IsEmpty()
        &&  m_DataSource->IsSetAnchor())  {
        // retrieve linear selection on master sequence
        typedef CAlnMultiWidget::TRangeColl   TRangeColl;

        CAlnVecMultiDataSource::TNumrow master_row = m_DataSource->GetAnchor();
        const TRangeColl& aln_coll = m_AlnWidget->GetSelection();

        m_DataSource->GetSeqFromAln(master_row, aln_coll, seq_coll);
        return true;
    }
    return false;
}


void CMultiAlignView::GetSelection(TConstScopedObjects& objs) const
{
    if (m_AlnWidget  &&  x_HasProject())    {
        typedef IAlnMultiDataSource ::TNumrow TNumrow;

        // object selection
        vector<TNumrow> rows;
        m_AlnWidget->GetSelectedRows(rows);
        CScope* scope = x_GetScope();
        objs.reserve(rows.size());

        ITERATE(vector<TNumrow>, it, rows)  { // iterate by selected rows
            TNumrow row = *it;
            const CSeq_id& id = m_DataSource->GetSeqId(row);

            objs.push_back(SConstScopedObject(&id, scope));
        }

        // range selection
        CAlnMultiWidget::TRangeColl seq_coll;
        if(x_GetSeqRangeSelection(seq_coll)  &&  !  seq_coll.Empty())    {
            CAlnVecMultiDataSource::TNumrow master_row = m_DataSource->GetAnchor();
            const CSeq_id&  master_id = m_DataSource->GetSeqId(master_row);
            CRef<CSeq_loc> loc = CSeqUtils::CreateSeq_loc(master_id, seq_coll);
            if (loc) {
                objs.push_back(SConstScopedObject(loc, scope));
            }
        }
    }
}


// handles incoming selection broadcast (overriding CView virtual function)
void CMultiAlignView::x_OnSetSelection(CSelectionEvent& evt)
{
    if (m_DataSource.Empty() || m_DataSource->IsEmpty()) return;

    bool has_anchor = m_DataSource->IsSetAnchor();
    CScope* scope = x_GetScope();

    _ASSERT(scope);

    // process Range Selection
    if(evt.HasRangeSelection()  &&  has_anchor) {
        typedef CAlnMultiWidget::TRangeColl TRangeColl;
        TRangeColl coll, aln_coll;

        CAlnVecMultiDataSource::TNumrow master_row = m_DataSource->GetAnchor();
        const CSeq_id&  master_id = m_DataSource->GetSeqId(master_row);
        evt.GetRangeSelection(master_id, *scope, coll);

        m_DataSource->GetAlnFromSeq(master_row, coll, aln_coll);

        m_AlnWidget->SetSelection(aln_coll);
    }

    // process Object Selection
    if(evt.HasObjectSelection())    {
        vector<CAlnVec::TNumrow> sel_rows;

        TConstObjects sel_objs;
        evt.GetRangeSelectionAsLocs(sel_objs);

        CConstRef<CSeq_id> row_id;
        CAlnMultiWidget::TRangeColl segs;

        CObjectIndex::TResults results;

        ITERATE(CSelectionEvent::TIds, it_id, evt.GetIds()) {
            const CSeq_id& id = **it_id;
            CSeq_id_Descr descr(NULL, id, evt.GetScope());
            // TODO - put descr in SelectionEvent

            results.clear();
            m_ObjIndex.GetMatches(descr, results);
            ITERATE(CObjectIndex::TResults, it_r, results)  {
                const CSelRowHandle* row_handle =
                    reinterpret_cast<const CSelRowHandle*>(*it_r);
                sel_rows.push_back(row_handle->GetNumrow());
            }
        }

        ITERATE(CSelectionEvent::TIdLocs, it_loc, evt.GetIdLocs()) {
            CConstRef<CIdLoc> id_loc = *it_loc;
            CSeq_id_Descr descr(NULL, *id_loc->m_Id, evt.GetScope());
            // TODO - put descr in SelectionEvent

            results.clear();
            m_ObjIndex.GetMatches(descr, results);
            ITERATE(CObjectIndex::TResults, it_r, results)  {
                const CSelRowHandle* row_handle =
                    reinterpret_cast<const CSelRowHandle*>(*it_r);
                IAlnExplorer::TNumrow row = row_handle->GetNumrow();

                bool match = true;
                if(CSelectionEvent::sm_MatchAlnLocs)    {
                    match = false;

                    TSeqPos pos = m_DataSource->GetSeqStart(row);
                    if(pos == id_loc->m_Range.GetFrom())  {
                        pos = m_DataSource->GetSeqStop(row);
                        match = (pos == id_loc->m_Range.GetTo());
                    }
                }
                if(match)   {
                    sel_rows.push_back(row);
                }
            }
        }

        m_AlnWidget->Select(sel_rows, true, true);
    }
}


void CMultiAlignView::GetSelection(CSelectionEvent& evt) const
{
   if(m_AlnWidget  &&  m_DataSource.GetPointer()  &&  ! m_DataSource->IsEmpty())   {
        // set Range Selection
        CAlnMultiWidget::TRangeColl seq_coll;
        if(x_GetSeqRangeSelection(seq_coll))    {
            CAlnVecMultiDataSource::TNumrow master_row = m_DataSource->GetAnchor();
            const CSeq_id&  master_id = m_DataSource->GetSeqId(master_row);
            evt.AddRangeSelection(master_id, seq_coll);
        }

        // set Object Selection
        typedef IAlnMultiDataSource::TNumrow TNumrow;

        vector<TNumrow> rows;
        m_AlnWidget->GetSelectedRows(rows);
        ITERATE(vector<TNumrow>, it, rows)  { // iterate by selected rows
            TNumrow row = *it;
            const CSeq_id& id = m_DataSource->GetSeqId(row);

            if(CSelectionEvent::sm_MatchAlnLocs)    {
                TSeqPos from = m_DataSource->GetSeqStart(row);
                TSeqPos to = m_DataSource->GetSeqStop(row);

                evt.AddObjectSelection(id, CRange<TSeqPos>(from, to));
            } else {
                evt.AddObjectSelection(id);
            }
        }
    }
}


// handles incoming selection broadcast (overriding CView virtual function)
// this event handler reacts on notification coming from CAlnMultiWidget
void CMultiAlignView::OnAlnWidgetSelChanged(CEvent* evt)
{
    _ASSERT(evt);

    switch (evt->GetID()) {
    case CAlnMultiPane::eSelRowChanged:
    case CAlnMultiPane::eLinearSelChanged:
        if (m_SelectionService)
            m_SelectionService->OnSelectionChanged(this);
        break;
    default:
        break;
    }
}


void CMultiAlignView::OnOpenGraphicalView(CEvent* evt)
{
    COpenGraphicViewEvent* openEvent = static_cast<COpenGraphicViewEvent*>(evt);
    CIRef<CProjectService> srv = m_Workbench->GetServiceByType<CProjectService>();
    CIRef<IProjectView> view = srv->AddProjectView("Graphical Sequence View", openEvent->GetObject(), openEvent->GetParams());
}


void CMultiAlignView::SetRegistryPath(const string& reg_path)
{
    m_RegPath = reg_path; // store for later use

    m_AlnWidget->SetRegistryPath(reg_path + ".Widget");
}


void CMultiAlignView::SaveSettings() const
{
    if( ! m_RegPath.empty())   {
        m_AlnWidget->SaveSettings();
    }
}


void CMultiAlignView::LoadSettings()
{
    if( ! m_RegPath.empty())   {
        m_AlnWidget->LoadSettings();
    }
}


const CObject* CMultiAlignView::x_GetOrigObject() const
{
    const CObject* obj = m_OrigAnnot ? (const CObject*) m_OrigAnnot.GetPointer()
                                     : m_OrigAlign.GetPointer();
    return obj;
}


///////////////////////////////////////////////////////////////////////////////
/// CMultiAlignViewFactory
string CMultiAlignViewFactory::GetExtensionIdentifier() const
{
    static string sid("multiple_alignment_view_factory");
    return sid;
}

string CMultiAlignViewFactory::GetExtensionLabel() const
{
    static string slabel("Multiple Sequence Alignment View Factory");
    return slabel;
}


void CMultiAlignViewFactory::RegisterIconAliases(wxFileArtProvider& provider)
{
    string alias = GetViewTypeDescriptor().GetIconAlias();
    provider.RegisterFileAlias(ToWxString(alias), wxT("multi_align_view.png"));
}


void CMultiAlignViewFactory::RegisterCommands(CUICommandRegistry& cmd_reg,
                                              wxFileArtProvider& provider)
{
    WidgetsData_RegisterCommands(cmd_reg, provider);
    CAlnMultiWidget::RegisterCommands(cmd_reg, provider);
}


const CProjectViewTypeDescriptor& CMultiAlignViewFactory::GetProjectViewTypeDescriptor() const
{
    return s_MultiAlignViewTypeDescr;
}


IView* CMultiAlignViewFactory::CreateInstance() const
{
    return new CMultiAlignView();
}


IView* CMultiAlignViewFactory::CreateInstanceByFingerprint(const TFingerprint& fingerprint) const
{
/*
    TFingerprint print(CMultiAlignView::m_TypeDescr.GetLabel(), false);
    if(print == fingerprint)   {
        return new CMultiAlignView();
    }
*/
    return NULL;
}

bool CMultiAlignViewFactory::IsCompatibleWith( const CObject& object, objects::CScope& scope )
{
    const type_info& type = typeid(object);

    if( typeid(CSeq_align) == type ){
        return true;
    }
        
    if( typeid(CSeq_annot) == type ){
        const CSeq_annot* annot = dynamic_cast<const CSeq_annot*>(&object);
        return annot && annot->IsAlign();
    }

/*
    if( typeid(CSeq_id) == type ){
        return true;
    }
*/

    return false;
}

int CMultiAlignViewFactory::TestInputObjects( TConstScopedObjects& objects )
{
    bool found_good = false;
    bool found_bad = false;
    
    NON_CONST_ITERATE( TConstScopedObjects, oitr, objects ){
        const CObject* object = oitr->object;
        CScope* scope = oitr->scope;

        if( object && scope && IsCompatibleWith( *object, *scope  ) ){
            found_good = true;
        } else {
            found_bad = true;
        }
    }

    return 
        found_good
        ? fCanShowSeparated |( found_bad ? fCanShowSome : fCanShowAll )
        : fCanShowNone   
    ;
}


bool CMultiAlignView::x_MergeObjects( TConstScopedObjects& objects )
{
    if( objects.size() < 2 ){
        return true;
    }

    CRef<CSeq_annot::TData> data( new CSeq_annot::TData() );
    CSeq_annot::TData::TAlign& aligns = data->SetAlign();
    
    CScope* scope = NULL;
    NON_CONST_ITERATE( TConstScopedObjects, it, objects ){
        if( scope == NULL ){
            scope = it->scope;
        }
        const CObject* const_obj = it->object;
        CObject* obj = const_cast<CObject*>( const_obj );

        CRef<CSeq_align> align( dynamic_cast<CSeq_align*>( obj ) );
        if( align ){
            aligns.push_back( align );
        }
    }

    CRef<CSeq_annot> annot( new CSeq_annot() );
    annot->SetData( *data );

    objects.clear();
    objects.push_back(SConstScopedObject(annot, scope));

    return true;
}

class CMAPanel;

///////////////////////////////////////////////////////////////////////////////
/// CMAViewManager 
///
class CMAViewManager : public COpenViewManager
{
public:
    CMAViewManager( IProjectViewFactory& factory )
        : COpenViewManager( factory )
        //, mf_IsSet( false )
    {
    }

    virtual bool DoTransition( EAction action );

public:

/*
    objects::CScope* GetScope() const { return m_Scope; }
    const CMAView::TAlignVector& GetAligns() const { return m_Aligns; }
    const vector<int>& GetGroups() const { return m_Groups; }
    const vector< set<CSeq_id_Handle> >& GetHandlesSet() const { return m_AlnHandles; }
    int GetNumSegments( int aGroupIx ) const {
        return m_NumSegs[aGroupIx];
    }
*/
    virtual wxPanel* GetCurrentPanel();
    virtual bool CanDo( EAction action );

protected:
    enum EState {
        eAnchor = COpenViewManager::eMaxState + 1
    };

protected:
    virtual void FinalizeParams();

    virtual bool NeedAnchorPanel( TConstScopedObjects& objects );
    virtual CMAPanel* CreateAnchorPanel();
    virtual void FinalizeAnchor();

private:
    CMAPanel* m_AnchorPanel;
/*
    objects::CScope* m_Scope;
    CMAView::TAlignVector m_Aligns;
    vector<int> m_Groups;
    vector< set<CSeq_id_Handle> > m_AlnHandles;
    vector<int> m_NumSegs;
    bool mf_IsSet;
*/
};

///////////////////////////////////////////////////////////////////////////////
/// CMAPanel 
///
class CMAPanel : public CParamsPanel
{

public:

    CMAPanel();
    CMAPanel( wxWindow* parent, wxWindowID id = SYM_CONVRESPANEL_IDNAME, const wxPoint& pos = SYM_CONVRESPANEL_POSITION, const wxSize& size = SYM_CONVRESPANEL_SIZE, long style = SYM_CONVRESPANEL_STYLE );

    virtual bool Create( wxWindow* parent, wxWindowID id = SYM_CONVRESPANEL_IDNAME, const wxPoint& pos = SYM_CONVRESPANEL_POSITION, const wxSize& size = SYM_CONVRESPANEL_SIZE, long style = SYM_CONVRESPANEL_STYLE );

    void Init();

    virtual void CreateControls();

    enum {
        ID_CCONVERSIONRESULTPANEL = 10034,
        ID_ONEVIEWPEROBJECT = 10035
    };
    ////@end CConversionResultPanel member variables

    void SetManager( CMAViewManager* aManager ){
        m_Manager = aManager;
    }

    /// add your methods here
    virtual void SetObjects( CObjectList& objs );

    /// Return the selected item
    virtual bool GetSelection( TConstScopedObjects& buf );

    /// Force OneViewPerObject true, and lock it
    virtual void EnforceOneViewPerObject();

    /// @name IRegSettings implementation
    /// @{
    virtual void SetRegistryPath(const string& path);
    virtual void LoadSettings();
    virtual void SaveSettings() const;
    /// @}

private:
    CMAViewManager* m_Manager;
    CObjectListWidget* m_ObLiWgt;
};


///////////////////////////////////////////////////////////////////////////////
/// CMAViewManager implementation
///
bool CMAViewManager::DoTransition( EAction action )
{

    if( m_State == eAnchor ){
        if( action == eNext ){
            // finalize choice
            TConstScopedObjects seqs;
            m_AnchorPanel->GetSelection( seqs );

            CRef<CSeq_annot::TData> data( new CSeq_annot::TData() );
            CSeq_annot::TData::TAlign& chosen_aligns = data->SetAlign();

            CScope* scope = NULL;
            NON_CONST_ITERATE( TConstScopedObjects, it, seqs ){
                const CSeq_id* const_id = dynamic_cast<const CSeq_id*>( it->object.GetPointer() );
                if( !const_id ) continue;

                CSeq_id_Handle chosen_idh = CSeq_id_Handle::GetHandle( *const_id );
                chosen_idh = sequence::GetId( chosen_idh, *it->scope, sequence::eGetId_Canonical );

                NON_CONST_ITERATE( TConstScopedObjects, ix, m_OutputObjects ){
                    if( !scope ){
                        scope = ix->scope;
                    }

                    list< CRef< CSeq_align > > aligns;

                    const CSeq_annot* annot = dynamic_cast<const CSeq_annot*>( ix->object.GetPointer() );
                    if( annot && annot->IsAlign() ){
                        aligns = const_cast<list< CRef< CSeq_align > >&>( annot->GetData().GetAlign() );
                    }

                    const CSeq_align* align = dynamic_cast<const CSeq_align*>( ix->object.GetPointer() );
                    if( align ){
                        aligns.push_back( CRef<CSeq_align>( const_cast<CSeq_align*>( align ) ) );
                    }


                    NON_CONST_ITERATE( list< CRef< CSeq_align > >, aix, aligns ){
                        CSeq_align* aln = aix->GetPointer();

                        int num_seqs = aln->CheckNumRows();
                        if( num_seqs <= 0 ){
                            continue;
                        }

                        for( int q = 0; q < num_seqs; q++ ){
                            const CSeq_id& seq_id = aln->GetSeq_id( q );
                            if (seq_id.Match(*const_id)) {
                                chosen_aligns.push_back(*aix);
                                break;
                            }
                            CSeq_id_Handle idh = CSeq_id_Handle::GetHandle( seq_id );
                            idh = sequence::GetId( idh, *ix->scope, sequence::eGetId_Canonical );
                            if( idh == chosen_idh ){
                                chosen_aligns.push_back( *aix );
                                break;
                            }
                        }
                    }
                }
            }

            CRef<CSeq_annot> annot( new CSeq_annot() );
            annot->SetData( *data );

            m_OutputObjects.clear();
            m_OutputObjects.push_back(SConstScopedObject(annot, scope));





            FinalizeAnchor();
            return true;

        } else {
            // roll back to alignments
            m_State = eInvalid;
            action = eNext;

            return COpenViewManager::DoTransition( action );
        }
    } else {
        if( !COpenViewManager::DoTransition( action ) ){
            return false;
        }
        
        if( m_State != eAnchor ){
            return true;
        }

        // if( action == eBack ) that is we came from 'Completed' (or whatever)
        // we need to restore alignment set, otherwise we can use m_OutputObjects

        if( NeedAnchorPanel( m_OutputObjects ) ){
            //! show panels with seqs

            set<CSeq_id_Handle> all_idh_set;
            CRef<CObjectList> oblist( new CObjectList() );

            NON_CONST_ITERATE( TConstScopedObjects, ix, m_OutputObjects ){

                list< CRef< CSeq_align > > aligns;

                const CSeq_annot* annot = dynamic_cast<const CSeq_annot*>( ix->object.GetPointer() );
                if( annot && annot->IsAlign() ){
                    aligns = const_cast<list< CRef< CSeq_align > >&>( annot->GetData().GetAlign() );
                }

                const CSeq_align* align = dynamic_cast<const CSeq_align*>( ix->object.GetPointer() );
                if( align ){
                    aligns.push_back( CRef<CSeq_align>( const_cast<CSeq_align*>( align ) ) );
                }


                NON_CONST_ITERATE( list< CRef< CSeq_align > >, aix, aligns ){
                    CSeq_align* aln = aix->GetPointer();

                    int num_seqs = aln->CheckNumRows();
                    if( num_seqs <= 0 ){
                        continue;
                    }

                    for( int q = 0; q < num_seqs; q++ ){
                        const CSeq_id& seq_id = aln->GetSeq_id( q );
                        CSeq_id_Handle idh = CSeq_id_Handle::GetHandle( seq_id );
                        idh = sequence::GetId( idh, *ix->scope, sequence::eGetId_Canonical );
                        if( all_idh_set.insert( idh ).second ){
                            oblist->AddRow( 
                                const_cast<CSeq_id*>( idh.GetSeqId().GetPointer() ),
                                ix->scope
                            );
                        }
                    }
                }
            }

            m_AnchorPanel = CreateAnchorPanel();
            m_AnchorPanel->Hide();
            m_AnchorPanel->Create( m_ParentWindow );
            m_AnchorPanel->LoadSettings();

            m_AnchorPanel->SetObjects( *oblist );


        } else {
            FinalizeAnchor();
        }

        return true;
    }
}

void CMAViewManager::FinalizeParams()
{
    m_State = eAnchor;
}

bool CMAViewManager::CanDo( EAction action )
{
    if( m_State == eAnchor ){
        return action == eNext ||  action == eBack;
    }

    return COpenViewManager::CanDo( action );
}

wxPanel* CMAViewManager::GetCurrentPanel()
{
    if( m_State == eAnchor ){
        return m_AnchorPanel;
    } 

    return COpenViewManager::GetCurrentPanel();
}

bool CMAViewManager::NeedAnchorPanel( TConstScopedObjects& objects )
{
    set<CSeq_id_Handle> common_idh_set;

    NON_CONST_ITERATE( TConstScopedObjects, ix, objects ){

        list< CRef< CSeq_align > > aligns;

        const CSeq_annot* annot = dynamic_cast<const CSeq_annot*>( ix->object.GetPointer() );
        if( annot && annot->IsAlign() ){
            aligns = const_cast<list< CRef< CSeq_align > >&>( annot->GetData().GetAlign() );
        }

        const CSeq_align* align = dynamic_cast<const CSeq_align*>( ix->object.GetPointer() );
        if( align ){
            aligns.push_back( CRef<CSeq_align>( const_cast<CSeq_align*>( align ) ) );
        }


        NON_CONST_ITERATE( list< CRef< CSeq_align > >, aix, aligns ){
            CSeq_align* aln = aix->GetPointer();

            int num_seqs = aln->CheckNumRows();
            if( num_seqs <= 0 ){
                continue;
            }

            set<CSeq_id_Handle> idh_set;
            for( int q = 0; q < num_seqs; q++ ){
                const CSeq_id& seq_id = aln->GetSeq_id( q );
                CSeq_id_Handle idh = CSeq_id_Handle::GetHandle( seq_id );
                idh = sequence::GetId( idh, *ix->scope, sequence::eGetId_Canonical );
                idh_set.insert( idh );
            }

            if( common_idh_set.empty() ){
                // fill the set initially
                common_idh_set = idh_set;

            } else {
                // check if the set has common seqs
                set<CSeq_id_Handle> intersection;
                ITERATE( set<CSeq_id_Handle>, hix, common_idh_set ){
                    if( idh_set.find( *hix ) != idh_set.end() ){
                        intersection.insert( *hix );
                    }
                }

                if( intersection.empty() ){
                    return true;
                } else {
                    common_idh_set = intersection;
                }
            }
        }
    }

    return false;
}

CMAPanel* CMAViewManager::CreateAnchorPanel()
{
    CMAPanel* panel = new CMAPanel();

    //panel->SetManager( this );

    return panel;
}

void CMAViewManager::FinalizeAnchor()
{
    m_State =  eCompleted;
}

///////////////////////////////////////////////////////////////////////////////
/// CMAPanel implementation
///
CMAPanel::CMAPanel()
{
    Init();
}


CMAPanel::CMAPanel( wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style )
{
    Init();
    Create(parent, id, pos, size, style);
}


bool CMAPanel::Create( wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style )
{
    ////@begin CMAPanel creation
    CParamsPanel::Create( parent, id, pos, size, style );

    CreateControls();
    if (GetSizer())
    {
        GetSizer()->SetSizeHints(this);
    }
    Centre();
    ////@end CMAPanel creation
    return true;
}

void CMAPanel::Init()
{
    m_Manager = NULL;
    m_ObLiWgt = NULL;
}


void CMAPanel::CreateControls()
{
    ////@begin CMAPanel content construction
    CMAPanel* itemPanel1 = this;

    wxBoxSizer* itemBoxSizer2 = new wxBoxSizer(wxVERTICAL);
    itemPanel1->SetSizer(itemBoxSizer2);

    wxStaticText* v_StatusText = new wxStaticText( 
        itemPanel1, wxID_ANY, _(
        "The supplied alignments do not contain common sequences. "
        "Please choose one or more sequences to be taken as basis(anchor); "
        "we filter out other alignments which do not contain these sequences. "
        "You can choose all the sequences, however, it may be impossible for "
        "to present the whole set of alignments in one view.  "
        ), wxDefaultPosition, wxSize(-1, 50), 0 
    );
    itemBoxSizer2->Add(v_StatusText, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

    m_ObLiWgt = new CObjectListWidget( itemPanel1, wxID_ANY, wxDefaultPosition, wxSize(100, 100), 0 );
    itemBoxSizer2->Add( m_ObLiWgt, 1, wxGROW|wxALL, 5 );

    /*
    m_OneViewPerObjectCheckBox = new wxCheckBox( itemPanel1, ID_ONEVIEWPEROBJECT, _("Open each object in a separate view"), wxDefaultPosition, wxDefaultSize, 0 );
    m_OneViewPerObjectCheckBox->SetValue(true);
    itemBoxSizer2->Add(m_OneViewPerObjectCheckBox, 0, wxALIGN_LEFT|wxLEFT|wxBOTTOM, 5);
    */

    // Set validators
    //m_OneViewPerObjectCheckBox->SetValidator( wxGenericValidator(& m_OneViewPerObject) );
    ////@end CMAPanel content construction

}


void CMAPanel::SetObjects( CObjectList& pObjList )
{
    m_ObLiWgt->SetObjectList( pObjList );
    m_ObLiWgt->SelectAll();
}

bool CMAPanel::GetSelection( TConstScopedObjects& buf )
{
    if( !m_ObLiWgt->TransferDataFromWindow() ){
        return false;
    }

    m_ObLiWgt->GetSelection( buf );
    
    return ( ! buf.empty());
}

void CMAPanel::EnforceOneViewPerObject()
{
    //CParamsPanel::ForceOneViewPerObject();
}


void CMAPanel::SetRegistryPath(const string& path)
{
    CParamsPanel::SetRegistryPath( path );
}

static const char* kDMGroupList = "MultiAlnAnchorList";


void CMAPanel::SaveSettings() const
{
    CParamsPanel::SaveSettings();

    if( !m_RegPath.empty() ){
        CGuiRegistry& gui_reg = CGuiRegistry::GetInstance();
        CRegistryWriteView view = gui_reg.GetWriteView(
            m_RegPath + "." + kDMGroupList 
        );
        m_ObLiWgt->SaveTableSettings(view);
    }
}


void CMAPanel::LoadSettings()
{
    CParamsPanel::LoadSettings();
    
    // Protection from older config savings
    m_OneViewPerObject = false;

    if ( !m_RegPath.empty() ) {
        CGuiRegistry& gui_reg = CGuiRegistry::GetInstance();
        CRegistryReadView view = gui_reg.GetReadView(
            m_RegPath + "." + kDMGroupList 
        );

        m_ObLiWgt->LoadTableSettings(view);
        TransferDataToWindow();
    }
}

CMultiAlignViewFactory::CMultiAlignViewFactory() 
{
    m_ViewManager = new CMAViewManager( *this );
}



END_NCBI_SCOPE
