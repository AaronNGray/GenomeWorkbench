/*  $Id: restore_local_file.cpp 40044 2017-12-13 15:38:02Z katargir $
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
 * Authors:  Andrea Asztalos
 *
 */

#include <ncbi_pch.hpp>
#include <objmgr/scope.hpp>
#include <objects/submit/Seq_submit.hpp>
#include <objtools/edit/seq_entry_edit.hpp>

#include <gui/core/document.hpp>
#include <gui/core/project_service.hpp>
#include <gui/core/open_view_task.hpp>
#include <gui/framework/view_manager_service.hpp>
#include <gui/framework/app_task_service.hpp>

#include <gui/objutils/objects.hpp>
#include <gui/utils/object_loader.hpp>
#include <gui/widgets/loaders/file_load_wizard.hpp>
#include <gui/widgets/loaders/open_objects_dlg.hpp>
#include <gui/widgets/wx/message_box.hpp>

#include <gui/packages/pkg_sequence_edit/restore_local_file.hpp>

BEGIN_NCBI_SCOPE
USING_SCOPE(objects);


CLocalFileRestorer::CLocalFileRestorer(IWorkbench* workbench)
    : m_Workbench(workbench)
{
    m_PrjService = m_Workbench->GetServiceByType<CProjectService>();
}

IProjectView* CLocalFileRestorer::FindSMARTPrjTextView() const
{
    IViewManagerService* view_srv = m_Workbench->GetServiceByType<IViewManagerService>();
    if (!view_srv) return 0;

    TConstScopedObjects objects;
    IViewManagerService::TViews views;
    view_srv->GetViews(views);

    CIRef<CProjectService> srv = m_Workbench->GetServiceByType<CProjectService>();
    CRef<CGBWorkspace> ws = srv->GetGBWorkspace();

    
    NON_CONST_ITERATE(IViewManagerService::TViews, it, views) {
        IProjectView* project_view = dynamic_cast<IProjectView*>((*it).GetPointer());
        if (project_view &&
            project_view->GetLabel(IProjectView::eType) == "Text View" &&
            NStr::Find(project_view->GetLabel(IProjectView::eProject), "SMART Client") != NPOS) {
            // limit the functionality to Smart project item

            return project_view;
        }
    }

    return 0;
}

bool CLocalFileRestorer::ReadObjectFromFile()
{
    CRef<CFileLoadWizard> fileManager(new CFileLoadWizard());
    vector<string> format_ids;
    format_ids.push_back("file_loader_asn");
    fileManager->LoadFormats(format_ids);

    vector<CIRef<IOpenObjectsPanelClient> > loadManagers;
    loadManagers.push_back(CIRef<IOpenObjectsPanelClient>(fileManager.GetPointer()));

    COpenObjectsDlg dlg(NULL);
    dlg.SetSize(710, 480);

    dlg.SetRegistryPath("Dialogs.Edit.OpenTables");
    dlg.SetManagers(loadManagers);

    if (dlg.ShowModal() == wxID_OK) {
        CIRef<IObjectLoader> object_loader(dlg.GetObjectLoader());
        if (!object_loader) {
            return false;
        }
        else {
            IExecuteUnit* execute_unit = dynamic_cast<IExecuteUnit*>(object_loader.GetPointer());
            _ASSERT(execute_unit);
            if (execute_unit) {
                if (!execute_unit->PreExecute())
                    return false;

                if (!GUI_AsyncExecUnit(*execute_unit, wxT("Reading file(s)...")))
                    return false; // Canceled

                if (!execute_unit->PostExecute())
                    return false;
            }

            NON_CONST_ITERATE(IObjectLoader::TObjects, obj_it, object_loader->GetObjects()) {
                CSerialObject* so = dynamic_cast<CSerialObject*>(obj_it->GetObjectPtr());
                if (so) {
                    m_NewSerialObj.Reset(so);
                    return true;
                }
            }
        }
    }
    return false;
}

void CLocalFileRestorer::CloseOpenViews(CGBDocument* doc)
{
    const CGBDocument::TViews& doc_views = doc->GetViews();
    if (!doc_views.empty()) {
        set<IProjectView*> viewToClose;
        for (auto it : doc_views) {
            viewToClose.insert(it);
        }
        ITERATE(set<IProjectView*>, it, viewToClose) {
            m_PrjService->RemoveProjectView(**it);
        }
    }
}

bool CLocalFileRestorer::x_DetachOrigObject(CRef<CProjectItem> item, CScope& scope)
{
    if (!item)
        return false;

    const CSerialObject* so = item->GetObject();
    CProjectItem::TItem::E_Choice type = item->GetItem().Which();
    CSeq_entry_Handle seh;

    if (type == CProjectItem::TItem::e_Entry) {
        const CSeq_entry* entry = dynamic_cast<const CSeq_entry *>(so);
        if (entry) {
            seh = scope.GetSeq_entryHandle(*entry);
        }
    }
    else if (type == CProjectItem::TItem::e_Submit) {
        const CSeq_submit* submit = dynamic_cast<const CSeq_submit *>(so);
        if (submit && submit->IsSetData() && submit->IsEntrys()) {
            seh = scope.GetSeq_entryHandle(*submit->GetData().GetEntrys().front());
        }
    }

    if (seh) {
        try {
            scope.RemoveTopLevelSeqEntry(const_cast<CTSE_Handle&>(seh.GetTSE_Handle()));
        }
        catch (CException& ex){
            LOG_POST(Error << "DetachOrigObject(): error removing seq-entry: " << ex.GetMsg());
            return false;
        }
    }

    return true;
}

bool CLocalFileRestorer::x_AttachNewObject(CRef<CProjectItem> item, CScope& scope, bool convert)
{
    _ASSERT(m_NewSerialObj);
    if (!item) {
        return false;
    }
    CRef<CSeq_entry> new_entry;

    CSeq_entry* seq_entry = dynamic_cast<CSeq_entry*>(m_NewSerialObj.GetNCPointer());
    CSeq_submit* seq_submit = dynamic_cast<CSeq_submit*>(m_NewSerialObj.GetNCPointer());

    if (seq_entry) {
        new_entry = Ref(seq_entry);
    } else if (seq_submit) {
        if (convert) {
            new_entry = edit::SeqEntryFromSeqSubmit(*seq_submit);
        } else {
            if (seq_submit && seq_submit->IsSetData() && seq_submit->IsEntrys()) {
                new_entry = seq_submit->GetData().GetEntrys().front();
            }
        }
    }

    if (!new_entry) {
        NcbiMessageBox("New file should contain either a seq-entry or a seq-submit.");
        return false;
    }

    try {
        CSeq_entry_Handle seh = scope.AddTopLevelSeqEntry(new_entry.GetNCObject(), CScope::kPriority_Default, CScope::eExist_Get);
        _ASSERT(seh);
        if (convert) {
            item->SetObject(new_entry.GetNCObject());
        } else {
            item->SetObject(m_NewSerialObj.GetNCObject());
        }
    }
    catch (CException& ex){
        LOG_POST(Error << "Failed to attach seq-entry: " << ex.GetMsg());
        return false;
    }

    return true;
}

void CLocalFileRestorer::RestoreFile(bool convert)
{
    IProjectView* project_view = FindSMARTPrjTextView();
    if (!project_view) {
        NcbiMessageBox("Failed to find Text View associated with SMART project.");
        return;
    }

    CIRef<CProjectService> srv = m_Workbench->GetServiceByType<CProjectService>();
    CRef<CGBWorkspace> ws = srv->GetGBWorkspace();

    IProjectView::TProjectId id = project_view->GetProjectId();
    CGBDocument* doc = dynamic_cast<CGBDocument*>(ws->GetProjectFromId(id));

    // get relevant ProjectItem
    TConstScopedObjects objects;
    project_view->GetMainObject(objects);
    _ASSERT(!objects.empty());

    const CProjectItem* const_item = m_PrjService->GetProjectItem(objects[0].object.GetObject(), objects[0].scope.GetNCObject());
    CProjectItem* project_item = const_cast<CProjectItem*>(const_item);

    if (!ReadObjectFromFile()) {
        return;
    }
    
    CloseOpenViews(doc);
    
    CScope* scope = doc->GetScope();
    if (scope && 
        x_DetachOrigObject(Ref(project_item), *scope) && 
        x_AttachNewObject(Ref(project_item), *scope, convert)) {

        // clean the history of commands
        CUndoManager& undo_mgr = doc->GetUndoManager();
        undo_mgr.Reset();

        // should we call smart processing on the new entry - no (that step was already called when the file was originally sent from SMART)
        // reopen the Text view
        const CSerialObject* so = project_item->GetObject();
        if (so) {
            CAppTaskService* taskService = m_Workbench->GetServiceByType<CAppTaskService>();
            SConstScopedObject object(so, scope);
            COpenViewTask* task = new COpenViewTask(m_Workbench, "Text View", object, 0, true);
            taskService->AddTask(*task);
        }
    }
}

END_NCBI_SCOPE
