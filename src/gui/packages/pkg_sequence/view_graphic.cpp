/*  $Id: view_graphic.cpp 44461 2019-12-20 16:11:13Z filippov $
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
 * Authors:  Vlad Lebedev, Liangshou Wu
 *
 * File Description:
 *    User-modifiable portion of main graphical sequence viewer class
 */


#include <ncbi_pch.hpp>

#include <gui/packages/pkg_sequence/view_graphic.hpp>

#include <gui/widgets/seq_graphic/seqgraphic_widget.hpp>
#include <gui/widgets/seq_graphic/graphic_panel.hpp>
#include <gui/widgets/seq_graphic/vis_range_glyph.hpp>
#include <gui/widgets/seq_graphic/feature_panel.hpp>

#include <gui/core/project_service.hpp>
#include <gui/core/data_mining_service.hpp>

#include <gui/objects/GBWorkspace.hpp>
#include <gui/objects/WorkspaceFolder.hpp>

#include <gui/framework/workbench.hpp>
#include <gui/widgets/wx/fileartprov.hpp>
#include <gui/widgets/wx/wx_utils.hpp>
#include <gui/widgets/wx/async_call.hpp>

#include <gui/objutils/utils.hpp>
#include <gui/objutils/label.hpp>
#include <gui/objutils/obj_convert.hpp>

#include <gui/objutils/visible_range.hpp>

#include <objects/general/Object_id.hpp>
#include <objects/seqloc/Seq_interval.hpp>
#include <objmgr/util/sequence.hpp>

#include <serial/iterator.hpp>
#include <wx/msgdlg.h>

BEGIN_NCBI_SCOPE
USING_SCOPE(objects);

BEGIN_EVENT_MAP(CViewGraphic, CProjectView)
    ON_EVENT(CEvent, CViewEvent::eWidgetDataChanged, &CViewGraphic::OnWidgetDataChanged)
    ON_EVENT(COpenGraphicViewEvent, CViewEvent::eOpenGraphicalViewEvent, &CViewGraphic::OnOpenGraphicalView)
END_EVENT_MAP()

///////////////////////////////////////////////////////////////////////////////
/// CViewGraphic

CProjectViewTypeDescriptor s_ViewGraphicTypeDescr(
    "Graphical Sequence View", // type name
    "graphical_view", // icon alias TODO
    "Graphical View",
    "The Graphical View provides an overview of annotations "
    "on sequence, with extensive drill-down capabilities.",
    "GRAPHICAL_VIEW", // help ID
    "Sequence",     // category
    false,       // singleton
    "Seq-loc",
    eSimilarObjectsAccepted
);

CViewGraphic::CViewGraphic()
    : m_GraphicPanel(NULL)
{
}


CViewGraphic::~CViewGraphic()
{
}


const wxMenu* CViewGraphic::GetMenu()
{
    return NULL;// TODO
}

void CViewGraphic::CreateViewWindow(wxWindow* parent)
{
    _ASSERT(!m_GraphicPanel);

    m_GraphicPanel = new CGraphicPanel(parent);

    CSeqGraphicWidget* widget = m_GraphicPanel->GetWidget();
    AddListener(widget, ePool_Child);
    widget->AddListener(this, ePool_Parent);
}


void CViewGraphic::DestroyViewWindow()
{
    //m_DataSource->DeleteAllJobs(); // Cancel all jobs before listener is deleted

    if (m_GraphicPanel) {       
        m_GraphicPanel->Destroy();
        m_GraphicPanel = NULL;
    }
}

//TODO this function is too long
bool CViewGraphic::InitView(TConstScopedObjects& objects, const objects::CUser_object* params)
{
    CScope* scope = NULL;
    const CObject* object = NULL;

    wxBusyCursor wait;

    x_MergeObjects( objects );

    if(objects.size() == 1) {
        object = objects[0].object.GetPointer();
        scope = objects[0].scope.GetPointer();

        const CSeq_loc*  loc = dynamic_cast<const CSeq_loc*> (object);
        const CSeq_id*  sid = dynamic_cast<const CSeq_id*> (object);
        const CBioseq*  seq = dynamic_cast<const CBioseq*> (object);
        const CSeq_entry* seq_entry = dynamic_cast<const CSeq_entry*> (object);
        if (seq_entry && seq_entry->IsSeq())
            seq = &seq_entry->GetSeq();

        if (loc) {
            sid = &sequence::GetId(*loc, scope);
        } else if (seq) {
            sid  = seq->GetFirstId();
        }

        if (sid) {
            static const wxString strInitializing = wxT("Initializing...");
            m_SeqId.Reset(sid);

            SConstScopedObject s_obj(m_SeqId, scope);
            CBioseq_Handle handle;
            try {
                handle = GUI_AsyncExec(
                    [&s_obj](ICanceled&)
                    {
                        const CSeq_id* id = dynamic_cast<const CSeq_id*>(s_obj.object.GetPointer());
                        CScope* scope = s_obj.scope.GetPointer();

                        CBioseq_Handle handle;
                        if (id  &&  scope) {
                            handle = scope->GetBioseqHandle(*id);
                            // resolving the seq-id in other form to avoid
                            // potential future network connection
                            CSeq_id_Handle idh = handle.GetSeq_id_Handle();
                            idh = sequence::GetId(*idh.GetSeqId(), *scope, sequence::eGetId_Best);
                        }
                        return handle;
                    },
                    strInitializing);
            } catch (CException& e) {
                string str("Failed to retrieve sequence: ");
                str += e.GetMsg();
                wxMessageBox(ToWxString(str));
                return false;
            }

            if ( !handle ) {
                string str;
                m_SeqId->GetLabel(&str);
                ReportIDError(str, m_SeqId->IsLocal(), "Graphical Sequence View");
                return false;
            }

            CIRef<CProjectService> srv =
                m_Workbench->GetServiceByType<CProjectService>();

            _ASSERT(srv);

            CRef<CGBWorkspace> ws = srv->GetGBWorkspace();
            if (!ws) return false;

            CGBDocument* doc = dynamic_cast<CGBDocument*>(ws->GetProjectFromScope(*scope));
            _ASSERT(doc);

            // type info for retrieving view settings
            string view_type = GetTypeDescriptor().GetHelpId();
            string view_ins_id;
            m_SeqId->GetLabel(&view_ins_id, CSeq_id::eFasta);
            string settings = doc->GetViewSettings(view_type, view_ins_id);
            LOG_POST(Info << "Retrieve view settings: " << settings);
            if (nullptr != params) {
                string param_settings = x_GetViewSettingsFromParams(params);
                if (!param_settings.empty()) {
                    LOG_POST(Info << "View settings through params: " << param_settings);
                    // Override settings
                    settings = param_settings;
                }
            }

            typedef vector<string> TToken;
            TToken setting_set;
            NStr::Split(settings, "&", setting_set);
            map<string, string> setting_map;
            ITERATE (TToken, iter, setting_set) {
                string curr_setting = NStr::TruncateSpaces(*iter);
                TToken tokens;
                NStr::Split(curr_setting, "=", tokens);
                if (tokens.size() == 2) {
                    setting_map.insert(map<string, string>::value_type(tokens[0],
                        tokens[1]));
                }
            }

            CSeqGraphicWidget* widget = m_GraphicPanel->GetWidget();
            CFeaturePanel* fp = widget->GetFeaturePanel();
            if (fp) {
                fp->SetDefaultAssembly(doc->GetDefaultAssembly());
                fp->SetServiceLocator(m_Workbench);
            }

            try {
                widget->UpdateConfig();
                LOG_POST(Info <<
                    "CViewGraphical::InitView() - finish with loading configuration!");
            } catch (CException&) {
                wxMessageBox(wxT("Something is wrong with configuration!"),
                    wxT("Failed to load configuration for Graphical view"),
                    wxOK | wxICON_EXCLAMATION);
                return false;
            }
            
            try {
                SConstScopedObject obj(m_SeqId, scope);

                // initialize the data source using async call to 
                // avoid completely blocking application without
                // users' attention.

                GUI_AsyncExec(
                    [widget, &obj](ICanceled&) { widget->InitDataSource(obj); },
                    strInitializing);

                // set input object to initialize the view
                widget->SetInputObject(obj);
                auto &cmdProcessor = doc->GetUndoManager();
                widget->InitBioseqEditor(cmdProcessor);                
                widget->SetWorkDir(doc->GetWorkDir());
                string id_desc;
                CLabel::GetLabel(*m_SeqId, &id_desc, CLabel::eContent, scope);
                LOG_POST(Info <<
                    "CViewGraphical::InitView() - load sequence: " << id_desc);
            } catch (CException&) {
                x_ReportInvalidInputData(objects);
                return false;
            }
   

            if (setting_map.count("m") != 0) {
                widget->SetMarkers(setting_map["m"]);
            }
            if (setting_map.count("mk") != 0) {
                widget->SetMarkers(setting_map["mk"]);
            }

            x_AttachToProject(*doc);

            m_GraphicPanel->CreateToolbar();
            
            // TODO move this block into a separate function
            // set our visible range
            if (loc  &&  !loc->IsWhole()) {
                TSeqRange range = loc->GetTotalRange();
                widget->ZoomOnRange(range, 0);
            } else if (setting_map.count("v") != 0) {
                TToken tokens;
                NStr::Split(setting_map["v"], "-:", tokens);
                if ( !tokens.empty() ) {
                    try {
                        TSeqPos from = NStr::StringToNumeric<TSeqPos>(NStr::TruncateSpaces(tokens[0]));
                        TSeqPos to = from;
                        if (tokens.size() > 1) {
                            to = NStr::StringToNumeric<TSeqPos>(NStr::TruncateSpaces(tokens[1]));
                            if (to < from) {
                                to = from;
                            }
                        }
                        widget->ZoomOnRange(TSeqRange(from, to), 0);
                    } catch (CException&) {
                    }
                }
            }


            //LOG_POST("CViewGraphic::InitView() finished init" << int(1000 * sw.Elapsed()) << " ms");

            x_UpdateContentLabel();

            return true;
        }

        // cannot represent the data
        x_ReportInvalidInputData(objects);
        return false;
    } else {
        return false;
    }
}


void CViewGraphic::OnProjectChanged(CProjectViewEvent& evt)
{
    if (evt.GetSubtype() == CProjectViewEvent::eDataChanging) {
        CSeqGraphicWidget* widget = m_GraphicPanel->GetWidget();
        widget->OnDataChanging();
    }
    else if (evt.GetSubtype() == CProjectViewEvent::eData  ||
        evt.GetSubtype() == CProjectViewEvent::eBothPropsData) {
        CSeqGraphicWidget* widget = m_GraphicPanel->GetWidget();
        widget->OnDataChanged();
        widget->Refresh();
    }
}


wxWindow* CViewGraphic::GetWindow()
{
    _ASSERT(m_GraphicPanel);
    return m_GraphicPanel;
}


string CViewGraphic::GetDMContextName()
{
    return GetClientLabel();
}


const CViewTypeDescriptor& CViewGraphic::GetTypeDescriptor() const
{
    return s_ViewGraphicTypeDescr;
}


void CViewGraphic::GetSelection(TConstScopedObjects& objs) const
{
    if (m_GraphicPanel &&  x_HasProject()) {
        CScope* scope = x_GetScope();

        /// object selection
        TConstObjects sel_objs;

        CSeqGraphicWidget* widget = m_GraphicPanel->GetWidget();
        widget->GetObjectSelection(sel_objs);

        set<CSeq_id_Handle> seqIds;
        seqIds.insert(CSeq_id_Handle::GetHandle(*m_SeqId));

        ITERATE (TConstObjects, it, sel_objs)    {
            objs.push_back(SConstScopedObject(*it, scope));

            const CSeq_align* align = dynamic_cast<const CSeq_align*>(it->GetPointerOrNull());
            if (align) {
                CObjectConverter::TRelationVector relations;
                CObjectConverter::FindRelations(*scope, *align, CSeq_id::GetTypeInfo()->GetName(), relations);
                NON_CONST_ITERATE(CObjectConverter::TRelationVector, itr, relations) {
                    if ((*itr)->GetName() == "Seq-align -> Seq-id") {
                        CRelation::TObjects related;
                        (*itr)->GetRelated(*scope, *align, related, CRelation::fConvert_NoExpensive);
                        ITERATE(CRelation::TObjects, itLoc, related) {
                            const CSeq_id* seq_id = dynamic_cast<const CSeq_id*>(&itLoc->GetObject());
                            if (seq_id) {
                                CSeq_id_Handle handle = CSeq_id_Handle::GetHandle(*seq_id);
                                if (seqIds.find(handle) == seqIds.end()) {
                                    objs.push_back(SConstScopedObject(seq_id, scope));
                                    seqIds.insert(handle);
                                }
                            }
                        }
                    }
                }
            }
        }

        /// range selection
        const CSeqGraphicWidget::TRangeColl& coll = widget->GetRangeSelection();
        if (coll.size()) {
            CRef<CSeq_loc> loc = CSeqUtils::CreateSeq_loc(*m_SeqId, coll);
            if (loc) {
                objs.push_back(SConstScopedObject(loc, scope));
            }
        }
    }
}


void CViewGraphic::GetSelection(CSelectionEvent& evt) const
{
    if (m_GraphicPanel  &&  x_HasProject()) {
        CSeqGraphicWidget* widget = m_GraphicPanel->GetWidget();
        // add range selection
        const CSeqGraphicWidget::TRangeColl& cl_sel
            = widget->GetRangeSelection();
        evt.AddRangeSelection(*m_SeqId, cl_sel);

        // add object selection
        TConstObjects sel_objs;
        widget->GetObjectSelection(sel_objs);
        evt.AddObjectSelection(sel_objs);
    }
}


// handles incoming selection broadcast (overriding CView virtual function)
void CViewGraphic::x_OnSetSelection(CSelectionEvent& evt)
{
    CScope* scope = x_GetScope();
    CSeqGraphicWidget* widget = m_GraphicPanel->GetWidget();

    if (evt.HasRangeSelection()) { // set range selection
        CSeqGraphicWidget::TRangeColl coll;
        evt.GetRangeSelection(*m_SeqId, *scope, coll);
        // set selection
        if (coll.size()) {
            widget->ResetRangeSelection();
            widget->SetRangeSelection(coll);
        }
    }

    if (&evt.GetScope() != scope)
        return;

    TConstObjects sel_objs;
    evt.GetAllObjects(sel_objs);

    //CScope::TIds ids = scope.GetIds(*m_SeqId);
    widget->ResetObjectSelection();
    ITERATE (TConstObjects, it_obj, sel_objs) {
        if (typeid(CSeq_id) == typeid(*it_obj)) {
            widget->SelectObject(*it_obj, false);
        } else if (const CSerialObject* nc_obj =
            dynamic_cast<const CSerialObject*>(it_obj->GetPointer()) ) {
            CTypeConstIterator<CSeq_id> id_iter(*nc_obj);
            for ( ;  id_iter;  ++id_iter) {
                if(evt.MatchIdWithId(*m_SeqId, *scope, *id_iter, evt.GetScope())) {
                    widget->SelectObject(*it_obj, true); // scope matched
                    break;
                }
            }
            // set selection with verified set to false
            widget->SelectObject(*it_obj, false);
        } else if (const CIdLoc* id_loc =
            dynamic_cast<const CIdLoc*>(it_obj->GetPointer()) ) {
            widget->SelectObject(id_loc->m_Id, false);
        } else { // log message
            LOG_POST(Warning << "The selected objects are not understood!");
        }
    }
}

// override function to suppress Navigation toolbar
void CViewGraphic::GetCompatibleToolBars(vector<string>& names)
{
    names.push_back(CProjectViewToolBatFactory::sm_SelTBName);
}

const CObject* CViewGraphic::x_GetOrigObject() const
{
    return m_SeqId.GetPointer();
}

void CViewGraphic::SetOrigObject(SConstScopedObject& obj)
{
    m_SeqId.Reset(dynamic_cast<const CSeq_id*>(obj.object.GetPointer()));
    CSeqGraphicWidget* widget = m_GraphicPanel->GetWidget();
    widget->InitDataSource(obj);
    widget->SetInputObject(obj);
    x_UpdateContentLabel();
    RefreshViewWindow();
}

void CViewGraphic::OnVisibleRangeChanged(const CVisibleRange& vrange,
                                         IVisibleRangeClient* /*source*/)
{
    IViewManagerService::TViews siblings;
    m_Workbench->GetViewManagerService()->GetViews(siblings);
    x_UpdateVisRangeLayout(siblings);

    CSeqGraphicWidget* widget = m_GraphicPanel->GetWidget();

    try {
        if( vrange.Match( *m_SeqId ) ) {
            TSeqRange this_range = widget->GetVisibleSeqRange();
            CBioseq_Handle bsh = x_GetScope()->GetBioseqHandle(*m_SeqId);
            if (vrange.Clamp(bsh, this_range)) {
                widget->ZoomOnRange(this_range, CSeqGraphicWidget::fSaveRange);
            }
        }
    } catch( std::exception& e ){
        LOG_POST(Error 
            << "CViewGraphic::OnVisibleRangeChanged(): exception caught "
            << e.what() );
    }
}

void CViewGraphic::x_UpdateVisRangeLayout(const IViewManagerService::TViews& views)
{
    CSeqGlyph::TObjects tmp_objs;
    ITERATE( IViewManagerService::TViews, iter, views ){
        const IView* iview = *iter;
        const CProjectView* view_p = dynamic_cast<const CProjectView*> (iview);
        try {
            if (view_p  &&  view_p->GetId() != GetId()) {  // not self
                CRgbaColor color;
                string label;
                list<TSeqRange> vis_ranges;
                view_p->GetReflectionInfo(*m_SeqId, vis_ranges, color, label);
                if ( !vis_ranges.empty() ) {
                    CRef<CSeqGlyph> vref(new CVisRangeGlyph(view_p,
                        vis_ranges, color, label));
                    tmp_objs.push_back(vref);
                }
            }
        } catch( std::exception& e ){
            LOG_POST(Error 
                << "CViewGraphic::x_UpdateVisRangeLayout(): exception caught " 
                << e.what() );
        }
    }
    
    m_GraphicPanel->GetWidget()->SetExternalGlyphs(tmp_objs);
}


void CViewGraphic::GetVisibleRanges(CVisibleRange& vrange) const
{
    TSeqRange range = m_GraphicPanel->GetWidget()->GetVisibleSeqRange();
    CRef<CSeq_loc> loc(new CSeq_loc());
    loc->SetInt().SetFrom(range.GetFrom());

    loc->SetInt().SetTo  (range.GetTo());
    loc->SetId(*m_SeqId);
    vrange.AddLocation(*loc);
}


void CViewGraphic::OnWidgetRangeChanged()
{
    IViewManagerService::TViews siblings;
    m_Workbench->GetViewManagerService()->GetViews(siblings);
    x_UpdateVisRangeLayout(siblings);
}


void CViewGraphic::OnWidgetDataChanged(CEvent* evt)
{
    _ASSERT(evt);
    m_GraphicPanel->GetWidget()->OnMarkerChanged();
}


void CViewGraphic::OnOpenGraphicalView(CEvent* evt)
{
    COpenGraphicViewEvent* openEvent = static_cast<COpenGraphicViewEvent*>(evt);
    CIRef<CProjectService> srv = m_Workbench->GetServiceByType<CProjectService>();
    CIRef<IProjectView> view = srv->AddProjectView("Graphical Sequence View", openEvent->GetObject(), openEvent->GetParams());
}


void CViewGraphic::OnViewAttached( IView& /*view*/ )
{
    IViewManagerService::TViews siblings;
    m_Workbench->GetViewManagerService()->GetViews( siblings );
    x_UpdateVisRangeLayout( siblings );
}

void CViewGraphic::OnViewReleased(IView& view)
{
    IViewManagerService::TViews siblings;
    m_Workbench->GetViewManagerService()->GetViews(siblings);
    const CProjectView* view_u = dynamic_cast<CProjectView*>(&view);
    NON_CONST_ITERATE(IViewManagerService::TViews, iter, siblings) {
        const IView* iview = *iter;
        const CProjectView* view_p = dynamic_cast<const CProjectView*> (iview);
        if (view_u  &&  view_p  &&  view_p->GetId() == view_u->GetId()) {
            siblings.erase(iter);
            break;
        }
    }

    x_UpdateVisRangeLayout(siblings);
}


void  CViewGraphic::SetWorkbench(IWorkbench* workbench)
{
    if(workbench) {
        // connect
        CIRef<CDataMiningService> dm_srv =
            workbench->GetServiceByType<CDataMiningService>();
        if (dm_srv) {
            dm_srv->AttachContext(*this);
        }
    }
    else {
        // disconnect
        CIRef<CDataMiningService> dm_srv =
            m_Workbench->GetServiceByType<CDataMiningService>();
        if (dm_srv) {
            dm_srv->DetachContext(*this);
        }
    }
    CProjectView::SetWorkbench(workbench);
    
}


CRef<CSeq_loc> CViewGraphic::GetSearchLoc()
{
    const CSeqGraphicWidget::TRangeColl& coll = 
        m_GraphicPanel->GetWidget()->GetRangeSelection();
    
    CRef<CSeq_loc> loc(new CSeq_loc());
    
    // if we have range selection use it for search
    if (coll.size()) {
        loc = CSeqUtils::CreateSeq_loc(*m_SeqId, coll);
    }
    else {
        CRef<CSeq_id> id(new CSeq_id());
        id->Assign(*m_SeqId);
        loc->SetWhole(*id);
    }

    return loc;
}


CRef<CScope> CViewGraphic::GetSearchScope()
{
    return m_Scope;
}


///////////////////////////////////////////////////////////////////////////////
/// CViewGraphicFactory
string  CViewGraphicFactory::GetExtensionIdentifier() const
{
    static string sid("view_graphic_factory");
    return sid;
}


string  CViewGraphicFactory::GetExtensionLabel() const
{
    static string slabel("View Graphic Factory");
    return slabel;
}


void CViewGraphicFactory::RegisterIconAliases(wxFileArtProvider& provider)
{
    string alias = GetViewTypeDescriptor().GetIconAlias();
    provider.RegisterFileAlias(ToWxString(alias), wxT("graphical_view.png"));
}


void CViewGraphicFactory::RegisterCommands (CUICommandRegistry& cmd_reg,
                                            wxFileArtProvider& provider)
{
    CSeqGraphicWidget::RegisterCommands(cmd_reg, provider);
}


const CProjectViewTypeDescriptor&
CViewGraphicFactory::GetProjectViewTypeDescriptor() const
{
    return s_ViewGraphicTypeDescr;
}


IView* CViewGraphicFactory::CreateInstance() const
{
    return new CViewGraphic();
}


IView* CViewGraphicFactory::CreateInstanceByFingerprint(const TFingerprint& /*fingerprint*/) const
{
    return NULL;
}

bool CViewGraphicFactory::IsCompatibleWith( const CObject& object, objects::CScope& )
{
    const type_info& type = typeid(object);

    if (typeid(CSeq_entry) == type) {
        const CSeq_entry& seq_entry = dynamic_cast<const CSeq_entry&>(object);
        return seq_entry.IsSeq();
    }

    return
        typeid(CSeq_loc) == type 
        || typeid(CSeq_id) == type 
        || typeid(CBioseq) == type
    ;
}


int CViewGraphicFactory::TestInputObjects( TConstScopedObjects& objects )
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


bool CViewGraphic::x_MergeObjects( TConstScopedObjects& objects )
{
    if( objects.size() < 2 ){
        return true;
    }
    // verify that all the objects has the same seq-id
    typedef map<CSeq_id_Handle, TSeqRange> TRanges;
    TRanges ranges;
    ITERATE( TConstScopedObjects, iter, objects ){
        const CSeq_loc* loc = dynamic_cast<const CSeq_loc*> (iter->object.GetPointer());
        if (loc) {
            CScope* scope = const_cast<CScope*>(iter->scope.GetPointer());
            CSeq_id_Handle idh = sequence::GetIdHandle(*loc, scope);
            TSeqRange range = loc->GetTotalRange();
            ranges[idh] += range;
        }
    }
    if (ranges.size() == 1) {
        CRef<objects::CScope> scope = objects[0].scope;
        CRef<CSeq_loc> loc(new CSeq_loc);
        TRanges::iterator iter = ranges.begin();
        loc->SetInt().SetFrom(iter->second.GetFrom());
        loc->SetInt().SetTo  (iter->second.GetTo());
        loc->SetId(*iter->first.GetSeqId());

        //! Should be better incapsulation of m_OutputObjects
        objects.clear();
        objects.push_back(
            SConstScopedObject(loc, scope)
        );
        return true;
    } else {
        return false;
    }
}


void CViewGraphic::SaveSettingsAtProject(objects::CGBProjectHandle& project) const
{
    CSeqGraphicWidget* widget = m_GraphicPanel->GetWidget();

    if (!widget || !widget->IsDirty())
        return;

    string settings = "v=";
    settings += NStr::NumericToString(widget->GetVisibleSeqRange().GetFrom());
    settings += "-";
    settings += NStr::NumericToString(widget->GetVisibleSeqRange().GetTo());
    string markers = widget->GetMarkers();
    if (!markers.empty()) {
        settings += "&m=" + markers;
    }

    string view_type = GetTypeDescriptor().GetHelpId();
    string view_ins_id;
    m_SeqId->GetLabel(&view_ins_id, CSeq_id::eFasta);

    project.SaveViewSettings(view_type, view_ins_id, settings);
    project.SetDirty(true);
    LOG_POST(Info << "Save view settings: " << settings);

    widget->SetDirty(false);
}


string CViewGraphic::x_GetViewSettingsFromParams(const objects::CUser_object* params)
{
    _ASSERT(nullptr != params);
    const CObject_id& type = params->GetType();
    if (type.IsStr() && type.GetStr() == "GraphicalViewParams") {
        ITERATE(CUser_object::TData, it, params->GetData()) {
            const CObject_id& field_id = (*it)->GetLabel();
            if (!field_id.IsStr())
                continue;

            const string& label = field_id.GetStr();
            const CUser_field::TData& data = (*it)->GetData();

            if ((label == "Settings") && data.IsStr())
                return data.GetStr();
        }
    }
    return string();
}


END_NCBI_SCOPE
