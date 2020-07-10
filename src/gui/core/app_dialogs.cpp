/*  $Id: app_dialogs.cpp 43745 2019-08-28 15:20:41Z katargir $
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
 */

#include <ncbi_pch.hpp>

#include <objects/seq/Seq_inst.hpp>
#include <objects/seq/Seq_hist.hpp>
#include <objects/seq/Seq_hist_rec.hpp>
#include <objects/seq/Bioseq.hpp>
#include <objects/seqloc/Seq_loc.hpp>
#include <objects/seqloc/Seq_id.hpp>

#include <serial/typeinfo.hpp>
#include <serial/iterator.hpp>

#include <gui/core/app_dialogs.hpp>

#include <gui/core/ui_data_source_service.hpp>
#include <gui/core/selection_service_impl.hpp>
#include <gui/core/project_service.hpp>

#include <gui/core/ui_export_tool.hpp>

#include <gui/core/open_dlg.hpp>
#include <gui/core/run_tool_dlg.hpp>
#include <gui/core/open_view_dlg.hpp>

#include <gui/framework/workbench.hpp>
#include <gui/framework/app_task_service.hpp>
#include <gui/framework/view_manager_service.hpp>

#include <gui/widgets/wx/message_box.hpp>
#include <gui/widgets/wx/ui_command.hpp>
#include <gui/widgets/wx/async_call.hpp>
#include <gui/widgets/wx/wx_utils.hpp>

#include <gui/utils/extension_impl.hpp>
#include <gui/objutils/obj_convert.hpp>
#include <gui/objutils/gi_cache.hpp>
#include <gui/objutils/project_item_extra.hpp>

#include <gui/core/async_obj_convert.hpp>

#include <wx/frame.h>
#include <wx/utils.h>
#include <wx/richmsgdlg.h>


BEGIN_NCBI_SCOPE
USING_SCOPE(objects);

static const char* kRunToolDlgSection = ".RunToolDialog";

void CAppDialogs::RunToolDialog(IWorkbench* workbench, const string& toolLabel)
{
    typedef CIRef<IUIAlgoToolManager>   TManagerRef;

    /// obtain current selection
    vector<TConstScopedObjects> objects;
    CIRef<CSelectionService> sel_srv = workbench->GetServiceByType<CSelectionService>();
    sel_srv->GetActiveObjects(objects);

    if(objects.empty()) {
        NcbiInfoBox("Please select objects you would like to run a tool on!");
        return;
    }

    /// get available tools
    vector< CIRef<IUIAlgoToolManager> > managers;
    GetExtensionAsInterface("ui_algo_tool_manager", managers);
    
    if (!toolLabel.empty()) {
        for( size_t i =0; i < managers.size(); i++ ){
            TManagerRef manager = managers[i];
            if (manager->GetDescriptor().GetLabel() == toolLabel) {
                managers.clear();
                managers.push_back(manager);
                break;
            }
        }
    }

    /// show Run Tool dialog
    CRunToolDlg dlg(workbench->GetMainWindow(), managers, workbench);
    
    string reg_path = workbench->GetRegistrySectionPath(IWorkbench::eDialogs);
    dlg.SetRegistryPath(reg_path + kRunToolDlgSection);

    dlg.SetInputObjects(objects);

    if(dlg.ShowModal() == wxID_OK)   {
        // schedule tool execution
        CIRef<IAppTask> tool_task = dlg.GetToolTask();
        if(tool_task)    {
            workbench->GetAppTaskService()->AddTask(*tool_task);
        }
    }
    return;
}


void CAppDialogs::ExportDialog(IWorkbench* workbench)
{
    ::wxBeginBusyCursor();

    /// obtain current selection
    vector<TConstScopedObjects> objects;
    CIRef<CSelectionService> sel_srv = workbench->GetServiceByType<CSelectionService>();
    sel_srv->GetActiveObjects(objects);

    if(objects.empty()) {
        NcbiInfoBox("Please select objects you would like to export!");
        return;
    }

    vector<IExporterFactory*> factories;
    GetExtensionAsInterface(EXT_POINT__UI_EXPORTER_FACTORY, factories);

    TConstScopedObjects objs;
    TRelatedObjectsMap map;
    NON_CONST_ITERATE(vector<TConstScopedObjects>, it, objects) {
        objs.insert(objs.end(), it->begin(), it->end());
        if (map.empty()) {
            AsyncConvertObjects(CSeq_loc::GetTypeInfo(), *it, map);
        }
    }

    CUIDataSourceService::TUIToolManagerVec managers;

    CProjectService* projectService = workbench->GetServiceByType<CProjectService>();

    string fileName;
    if (projectService) {
        if (objs.size() == 1 && objs[0].object && objs[0].scope) {
            const CProjectItem* pi = projectService->GetProjectItem(*objs[0].object, *objs[0].scope);
            if (pi) CProjectItemExtra::GetStr(*pi, "OriginalFile", fileName);
        }
    }

    ITERATE(vector<IExporterFactory*>, it, factories) {
        if ((*it)->TestInputObjects(objs)) {
            CIRef<IUIExportTool> exporter((*it)->CreateInstance());
            exporter->SetLocations(map);
            exporter->SetObjects(objs);
            exporter->SetFileName(fileName);
            managers.push_back(CIRef<IUIToolManager>(exporter.GetPointer()));
        }
    }

    if (managers.empty()) {
        NcbiInfoBox("No exporters defined that can export selected objects!");
        return;
    }

    // create "Open" dialog
    wxWindow* parent = workbench->GetMainWindow();

    COpenDlg dlg(parent);
    dlg.SetSize(710, 480);
    dlg.SetServiceLocator(workbench);
    dlg.SetManagers(managers);
    dlg.SetBaseTitle(wxT("Export"));

    dlg.SetRegistryPath("ExportDialog"); //TODO

    ::wxEndBusyCursor();

    if(dlg.ShowModal() == wxID_OK)  {
        // get the loading task and add it to Task Manager queue
        CIRef<IAppTask> loading_task = dlg.GetLoadingTask();
        if(loading_task)    {
            workbench->GetAppTaskService()->AddTask(*loading_task);
        }
    }
}

static const char* kOpenViewDlgSection = ".OpenViewDialog";

namespace {

    void s_PrefetchHandles(TConstScopedObjects& objects)
    {
        NON_CONST_ITERATE(TConstScopedObjects, itr, objects){

            const CSeq_id* id = dynamic_cast<const CSeq_id*>(itr->object.GetPointer());
            if (id){
                itr->scope->GetBioseqHandle(*id);
                continue;
            }

            const CSeq_loc* loc = dynamic_cast<const CSeq_loc*>(itr->object.GetPointer());
            if (loc){
                itr->scope->GetBioseqHandle(*loc);
                continue;
            }

            const CBioseq* seq = dynamic_cast<const CBioseq*>(itr->object.GetPointer());
            if (seq){
                itr->scope->GetBioseqHandle(*seq);
                continue;
            }

            /* CSeq_annot is just a container - no prefetch.

            const CSeq_annot* annot = dynamic_cast<const CSeq_annot*>( itr->object.GetPointer() );
            if( annot ){
            itr->scope->GetSeq_annotHandle(*annot);
            continue;
            }
            */

            const CSeq_feat* feat = dynamic_cast<const CSeq_feat*>(itr->object.GetPointer());
            if (feat){
                itr->scope->GetSeq_featHandle(*feat);
                continue;
            }
        }
    }

} // namespace

static void s_CheckDataStatus( SConstScopedObject& object )
{
    CBioseq_Handle handle;
    const CSeq_id* id = dynamic_cast<const CSeq_id*>( object.object.GetPointer() );
    const CSeq_loc* loc = dynamic_cast<const CSeq_loc*>( object.object.GetPointer() );
    const CBioseq* seq = dynamic_cast<const CBioseq*>( object.object.GetPointer() );

    if( id ){
        handle = object.scope->GetBioseqHandle(*id);

    } else if( loc ){
        handle = object.scope->GetBioseqHandle(*loc);

    } else if( seq ){
        handle = object.scope->GetBioseqHandle(*seq);
    }

    if( handle && !CGiCache::GetInstance().RecordKnown( handle ) ){
        string warning_msg = kEmptyStr;
        if( handle.State_Withdrawn() || handle.State_Suppressed() ){
            warning_msg = "this record has been removed or withdrawn.";

        } else if(
            handle.IsSetInst()  
            && handle.IsSetInst_Hist()  
            && handle.GetInst_Hist().IsSetReplaced_by()
        ){
            const CSeq_hist_rec::TIds& ids = handle.GetInst_Hist().GetReplaced_by().GetIds();
            warning_msg = "this record has been replaced by: ";
            ITERATE( CSeq_hist_rec::TIds,  iter,  ids ){
                string id_label;
                const CBioseq::TId& seq_ids = handle.GetScope().GetBioseqHandle(**iter).GetCompleteBioseq()->GetId();
                ITERATE( CBioseq::TId, seq_iter, seq_ids ){
                    if(
                        (*seq_iter)->Which() == CSeq_id::e_Other  && (*seq_iter)->GetOther().IsSetAccession()
                    ){
                        id_label = (*seq_iter)->GetOther().GetAccession();
                        if( (*seq_iter)->GetOther().IsSetVersion() ){
                            id_label += "." + NStr::IntToString((*seq_iter)->GetOther().GetVersion());
                        }
                        break;
                    }
                }
                if( id_label.empty() ){
                    (*iter)->GetLabel( &id_label, CSeq_id::eFasta );
                }
                warning_msg += id_label + " ";
            }
        }

        if( !warning_msg.empty() ){
            static bool showDialog = true;
            if (!showDialog)
                return;

            string viewed_id_label;
            handle.GetCompleteBioseq()->GetLabel(&viewed_id_label, CBioseq::eContent);
            string base = "Opening record " + viewed_id_label + ".  Please be aware of that ";

            wxRichMessageDialog dlg(NULL, ToWxString(base + warning_msg), wxT("Warning"), wxOK|wxICON_EXCLAMATION);
            dlg.ShowCheckBox(wxT("Don't show this dialog in the current session"));
            dlg.ShowModal();
            if (dlg.IsCheckBoxChecked())
                showDialog = false;
        }
    }
}

void CAppDialogs::OpenViewlDialog(IWorkbench* workbench, bool openDefault)
{
    /// obtain current selection
    TConstScopedObjects objects;
    CIRef<CSelectionService> sel_srv = workbench->GetServiceByType<CSelectionService>();
    sel_srv->GetActiveObjects( objects );

    if( objects.empty() ){
        NcbiWarningBox( "Please select objects you would like to view." );
        return;
    }

    /// get all View Factories
    IViewManagerService* srv = workbench->GetViewManagerService();
    IViewManagerService::TFactories factories;
    srv->GetFactories( factories );

    string defaultFactoryLabel;
    CIRef<IProjectViewFactory> defaultFactory;

    string reg_path = workbench->GetRegistrySectionPath( IWorkbench::eDialogs ) + kOpenViewDlgSection;

    if (openDefault) {
        defaultFactoryLabel = COpenViewDlg::GetDefaultFactoryLabel(reg_path, objects);
    }

    /// select Project View Factories only
    COpenViewDlg::TFactories prj_factories;
    for( size_t i = 0; i < factories.size();  i++ ){
        IProjectViewFactory* prj_fc = dynamic_cast<IProjectViewFactory*>( &*factories[i] );
        if( prj_fc && prj_fc->ShowInOpenViewDlg()){
            CIRef<IProjectViewFactory> ref( prj_fc );
            prj_factories.push_back(ref);
            
            if (openDefault && prj_fc->GetViewTypeDescriptor().GetLabel() == defaultFactoryLabel) {
                int test = prj_fc->TestInputObjects( objects );
                if (test&IProjectViewFactory::fCanShowAll)
                    defaultFactory.Reset(prj_fc);
            }
        }
    }

    if (openDefault && defaultFactory) {
        const CProjectViewTypeDescriptor& pvt_desc = defaultFactory->GetProjectViewTypeDescriptor();
        string view_name = pvt_desc.GetLabel();
        bool prefetch_needed = pvt_desc.IsPrefetchNeeded();

        CProjectService* prj_srv = workbench->GetServiceByType<CProjectService>();

        if( prefetch_needed ){
            try {
                GUI_AsyncExec([&objects](ICanceled&) { s_PrefetchHandles(objects); });
            }
            catch (const exception&) {}

            try {
                for (size_t j = 0; j < objects.size(); j++) {
                    s_CheckDataStatus(objects[j]);
                }
            } NCBI_CATCH("CAppDialogs::OpenViewlDialog");
        }

        try {
            prj_srv->AddProjectView( view_name, objects[0], 0 );
        
        } catch( CException& ex ){
            //! CUserException should be used
            NcbiErrorBox( "Open View: " + ex.GetMsg() );
        }

        return;
    }

    /// setup the dialog
    COpenViewDlg dlg( workbench->GetMainWindow() );

    /*
    wxAcceleratorEntry entries[1];
    entries[0].Set( wxACCEL_CTRL, (int) 'A', wxID_SELECTALL );
    wxAcceleratorTable accel( 1, entries );
    dlg.SetAcceleratorTable( accel );
    */

    dlg.SetRegistryPath( reg_path );

    dlg.SetInputObjects( objects );
    dlg.SetFactories( prj_factories );

    if( dlg.ShowModal() == wxID_OK ){
        CIRef<IProjectViewFactory> factory = dlg.GetSelectedFactory();
        const CProjectViewTypeDescriptor& pvt_desc = factory->GetProjectViewTypeDescriptor();

        string view_name = pvt_desc.GetLabel();
        EPVObjectsAccepted multi_accepted = pvt_desc.GetObjectsAccepted();
        bool prefetch_needed = pvt_desc.IsPrefetchNeeded();

        CProjectService* prj_srv = workbench->GetServiceByType<CProjectService>();

        // create a view for every item

        objects = dlg.GetOutputObjects();
        vector<int> indices = dlg.GetOutputIndices();

        _ASSERT( indices.size() <= objects.size() );

        //! HACK
        if( multi_accepted == eAnyObjectsAccepted ){
            NON_CONST_ITERATE( vector<int>, ix_itr, indices ){
                if( *ix_itr > 0 ){
                    *ix_itr = 1;
                }
            }
        }

        if( prefetch_needed ){
            try {
                GUI_AsyncExec([&objects](ICanceled&) { s_PrefetchHandles(objects); });
            } catch(const exception&) {}

            try {
                for (size_t j = 0; j < objects.size(); j++)
                    s_CheckDataStatus(objects[j]);
            }
            catch (const exception&) {}
        }

        set<int> opened_groups;

        for( size_t ix = 0; ix < objects.size(); ix++ ){
            int group_ix = ix < indices.size() ? indices[ix] : -1;

            if( group_ix < 0 ){
                try {
                    prj_srv->AddProjectView( view_name, objects[ix], 0 );
                
                } catch( CException& ex ){
                    //! CUserException should be used
                    NcbiErrorBox( "Open View: " + ex.GetMsg() );
                }

            } else if( opened_groups.find( group_ix ) == opened_groups.end() ){
                opened_groups.insert( group_ix );

#               ifdef _DEBUG
                CStopWatch watch( CStopWatch::eStart );
#               endif

                TConstScopedObjects obj_group;
                for( size_t j = ix; j < indices.size(); j++ ){
                    if( indices[j] == group_ix ){
                        obj_group.push_back( objects[j] );
                    }
                }

#               ifdef _DEBUG
                watch.Stop();
                ERR_POST(Info << "Open View: grouping takes " << watch.AsSmartString() );
#               endif

                try {
                    prj_srv->AddProjectView( view_name, obj_group, 0 );

                } catch( CException& ex ){
                    //! CUserException should be used
                    NcbiErrorBox( "Open View: " + ex.GetMsg() );
                }
            }
        }
    }
}

void CAppDialogs::COpenDialog(IWorkbench* workbench, const string& loader_label, const vector<wxString>& filenames)
{
    // obtain the list of Load Managers
    CUIDataSourceService::TUIToolManagerVec managers;
    CIRef<CUIDataSourceService> srv = workbench->GetServiceByType<CUIDataSourceService>();
    srv->GetLoadManagers(managers);

    // create "Open" dialog
    wxWindow* parent = workbench->GetMainWindow();

    COpenDlg dlg(parent);
    dlg.SetSize(710, 480);
    dlg.SetServiceLocator(workbench);
    dlg.SetManagers(managers);

    if (!loader_label.empty()) {
        dlg.SelectOptionByLabel(loader_label);
    }
    if (!filenames.empty()) {
        dlg.SetFilenames(filenames);
    }
    dlg.SetRegistryPath("OpenDialog"); //TODO

    if(dlg.ShowModal() == wxID_OK)  {
        // get the loading task and add it to Task Manager queue
        CIRef<IAppTask> loading_task = dlg.GetLoadingTask();
        if(loading_task)    {
            workbench->GetAppTaskService()->AddTask(*loading_task);
        }
    }
}

END_NCBI_SCOPE
