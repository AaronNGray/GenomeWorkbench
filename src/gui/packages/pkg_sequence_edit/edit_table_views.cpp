/*  $Id: edit_table_views.cpp 40258 2018-01-18 19:45:07Z katargir $
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
 *
 */

#include <ncbi_pch.hpp>

#include "edit_table_views.hpp"

#include <gui/core/project_service.hpp>
#include <gui/core/document.hpp>

#include <gui/framework/workbench.hpp>
#include <gui/framework/status_bar_service.hpp>
#include <gui/framework/view_manager_service.hpp>

#include <wx/grid.h>
#include <wx/msgdlg.h>

#include <gui/widgets/data/query_panel_event.hpp>
#include <gui/widgets/grid_widget/grid_widget.hpp>
#include <gui/widgets/grid_widget/grid_event.hpp>
#include <gui/widgets/grid_widget/grid.hpp>
#include <gui/widgets/seq/flat_file_ctrl.hpp>
#include <gui/widgets/seq/text_panel.hpp>
#include <gui/widgets/text_widget/text_item_panel.hpp>

#include <gui/widgets/wx/fileartprov.hpp>
#include <gui/widgets/wx/wx_utils.hpp>
#include <gui/widgets/wx/message_box.hpp>

#include <gui/objutils/utils.hpp>
#include <gui/core/simple_project_view.hpp>
#include <gui/widgets/edit/edit_obj_view_dlg.hpp>
#include <gui/widgets/edit/bioseq_editor.hpp>
#include <gui/widgets/edit/alignment_assistant.hpp>

#include <gui/packages/pkg_sequence_edit/miscedit_util.hpp>

// for SaveData
#include <gui/core/project_task.hpp>

#include <objects/general/Object_id.hpp>

#include <objmgr/util/sequence.hpp>
#include <gui/objutils/label.hpp>

BEGIN_NCBI_SCOPE
USING_SCOPE(objects);

BEGIN_EVENT_MAP(CTableViewWithTextViewNav, CProjectView)
  ON_EVENT(CQueryPanelEvent, CQueryPanelEvent::eStatusChange, &CTableViewWithTextViewNav::x_QueryStatusChange)
  ON_EVENT(CGridWidgetEvent, CGridWidgetEvent::eRefreshData, &CTableViewWithTextViewNav::x_OnRefreshData)
END_EVENT_MAP()

void CTableViewWithTextViewNav::DestroyViewWindow()
{
    if( m_Window ){
        m_Window->Destroy();
        m_Window = 0;
    }
}

void CTableViewWithTextViewNav::SetParams(const CUser_object* params)
{
    if (params) {
        m_Params.Reset(new CUser_object());
        m_Params->Assign(*params);
    } else {
        m_Params.Reset(NULL);
    }
}

bool CTableViewWithTextViewNav::InitView(TConstScopedObjects& objects, const objects::CUser_object* params)
{
    SetParams(params);
    m_OrigObjects = objects;

    CIRef<CProjectService> srv = m_Workbench->GetServiceByType<CProjectService>();
    _ASSERT(srv);

    CRef<CGBWorkspace> ws = srv->GetGBWorkspace();
    if (!ws) return false;

    CScope* scope = m_OrigObjects.front().scope;

    CGBDocument* doc = dynamic_cast<CGBDocument*>(ws->GetProjectFromScope(*scope));
    _ASSERT(doc);
    if (doc) {
        x_LoadData();
        x_AttachToProject(*doc);
        x_UpdateContentLabel();

        IProjectView* projectTextView = srv->FindView(*m_OrigObjects.front().object, "Text View");
        if(projectTextView) {
            CTextPanel* panel = dynamic_cast<CTextPanel*>(projectTextView->GetWindow());
            panel->GetWidget()->SetTrackSelection(true); 
        }
        return true;
    }

    x_ReportInvalidInputData(objects);
    return false;
}

void CTableViewWithTextViewNav::OnProjectChanged(CProjectViewEvent& evt)
{
    if (evt.GetSubtype() == CProjectViewEvent::eData ||
                evt.GetSubtype() == CProjectViewEvent::eBothPropsData) {
        m_Window->Refresh();
    }
}

void CTableViewWithTextViewNav::GetSelection(CSelectionEvent& evt) const
{
    if (m_Window  &&  x_HasProject()) {
        CGridWidget* grid = (CGridWidget*)m_Window;

        TConstScopedObjects sobjs;
        grid->GetSelectedObjects (sobjs);

        TConstObjects objs;
        ITERATE(TConstScopedObjects, it, sobjs) {
            objs.push_back(it->object);
        }

        evt.AddObjectSelection(objs);        
    }
}


void CTableViewWithTextViewNav::GetSelection(TConstScopedObjects& objs) const
{
    if (m_Window  &&  x_HasProject()) {
        CGridWidget* grid = (CGridWidget*)m_Window;
        grid->GetSelectedObjects (objs);
    }
}


void CTableViewWithTextViewNav::GetMainObject(TConstScopedObjects& objs) const
{
    if (m_Window  &&  x_HasProject()) {
        /*CGridWidget* grid = (CGridWidget*)m_Window;*/

        if(m_OrigObjects.size() > 0) {
            const SConstScopedObject& ScopedObject(m_OrigObjects[0]);
            const CObject* object = ScopedObject.object.GetPointer();
            if (object) {
                CScope* scope(const_cast<CScope*>(ScopedObject.scope.GetPointer()));
                objs.push_back(SConstScopedObject(object, scope));
            }
        }
    }
}

// override function to suppress Navigation toolbar
void CTableViewWithTextViewNav::GetCompatibleToolBars(vector<string>& names)
{
    names.push_back(CProjectViewToolBatFactory::sm_SelTBName);
}

const CObject* CTableViewWithTextViewNav::x_GetOrigObject() const
{
    if(m_OrigObjects.size() > 0) {
        return m_OrigObjects[0].object.GetPointer();
    }

    return NULL;
}

void CTableViewWithTextViewNav::SetRegistryPath(const string& reg_path)
{
    IRegSettings* reg_set = dynamic_cast<IRegSettings*>(m_Window);
    if (reg_set)
        reg_set->SetRegistryPath(reg_path);
}

void CTableViewWithTextViewNav::LoadSettings()
{
    IRegSettings* reg_set = dynamic_cast<IRegSettings*>(m_Window);
    if (reg_set)
        reg_set->LoadSettings();
}


void CTableViewWithTextViewNav::SaveSettings() const
{
    IRegSettings* reg_set = dynamic_cast<IRegSettings*>(m_Window);
    if (reg_set)
        reg_set->SaveSettings();
}

void CTableViewWithTextViewNav::x_OnRefreshData(CEvent* evt)
{
    CGridWidgetEvent* tableEvt(dynamic_cast<CGridWidgetEvent*>(evt));
    if (!tableEvt) return;
    x_GetViewObjects();
    x_LoadData();
}

void CTableViewWithTextViewNav::x_GetViewObjects(void)
{
    TConstScopedObjects objects;

    GetViewObjects(m_Workbench,objects);

    if (!objects.empty()) {
        m_OrigObjects = objects;
    }
}

void CTableViewWithTextViewNav::x_QueryStatusChange(CEvent* evt)
{
    CQueryPanelEvent* queryEvt = dynamic_cast<CQueryPanelEvent*>(evt);
    if (!queryEvt) return;

    CIRef<IStatusBarService> sb_srv = m_Workbench->GetServiceByType<IStatusBarService>();
    sb_srv->SetStatusMessage(queryEvt->GetStatus());
}

static const string sTextViewType("Text View");

CIRef<IProjectView> CTableViewWithTextViewNav::x_CreateDefaultTextView(CIRef<CProjectService> prjSrv, TConstScopedObjects& objects)
{
    CRef<CUser_object> params(new CUser_object());
    CRef<CObject_id> type(new CObject_id());
    type->SetStr("TextViewParams");
    params->SetType(*type);

    CUser_object::TData& data = params->SetData();

    {{
        CRef<CUser_field> param(new CUser_field());
        CRef<CObject_id> label(new CObject_id());
        label->SetStr("TextViewType");
        param->SetLabel(*label);
        param->SetData().SetStr("Flat File");
        data.push_back(param);
    }}

    {{
        CRef<CUser_field> param(new CUser_field());
        CRef<CObject_id> label(new CObject_id());
        label->SetStr("ExpandAll");
        param->SetLabel(*label);
        param->SetData().SetBool(true);
        data.push_back(param);
    }}

    {{
        CRef<CUser_field> param(new CUser_field());
        CRef<CObject_id> label(new CObject_id());
        label->SetStr("TrackSelection");
        param->SetLabel(*label);
        param->SetData().SetBool(true);
        data.push_back(param);
    }}
        
    CIRef<IProjectView> prj_view = prjSrv->AddProjectView(sTextViewType, objects, params);
    return prj_view;
}


CIRef<IFlatFileCtrl> CTableViewWithTextViewNav::x_GetFlatFileCtrl(TConstScopedObjects& objects)
{
    CIRef<IFlatFileCtrl> FlatFileCtrl;

    CIRef<CProjectService> prjSrv = m_Workbench->GetServiceByType<CProjectService>();
    CIRef<IProjectView> pTextView(prjSrv->FindView(*(objects.front().object), sTextViewType));
    if (!pTextView) {
        pTextView = x_CreateDefaultTextView(prjSrv, objects);
    }

    FlatFileCtrl.Reset(dynamic_cast<IFlatFileCtrl*>(pTextView.GetPointerOrNull()));

    return FlatFileCtrl;
}


ICommandProccessor* CTableViewWithTextViewNav::x_GetCmdProcessor(TConstScopedObjects& objects)
{
    if (objects.size() == 0) return NULL;
    CIRef<CProjectService> srv = m_Workbench->GetServiceByType<CProjectService>();
    if (!srv) return NULL;

    CRef<CGBWorkspace> ws = srv->GetGBWorkspace();
    if (!ws) return NULL;

    CGBDocument* doc = dynamic_cast<CGBDocument*>(ws->GetProjectFromScope(*(objects.front().scope)));
    if (doc) {
        return &doc->GetUndoManager(); 
    } else {
        return NULL;
    }
}


bool IsEditable(SConstScopedObject& obj)
{
    CSeq_entry_Handle seh = GetTopSeqEntryFromScopedObject(obj);
    if (!seh) {
        return false;
    }

    const CSeq_feat *feat = dynamic_cast<const CSeq_feat *>(obj.object.GetPointer());
    const CSeqdesc  *desc = dynamic_cast<const CSeqdesc *>(obj.object.GetPointer());
    if (feat || desc) {
        return true;
    } else {
        return false;
    }
}


void CTableViewWithTextViewNav::x_EditObject(SConstScopedObject& obj)
{
    TConstScopedObjects objects;
    objects.push_back(obj);
    ICommandProccessor* cmd_processor = x_GetCmdProcessor(objects);

    const CSeq_align *align = dynamic_cast<const CSeq_align *>(obj.object.GetPointer());
    if (align)
    {
        CRef<objects::CScope> scope = obj.scope;
        CSeq_entry_Handle seh;
        int num_rows = align->CheckNumRows();
        for (size_t row = 0; row < num_rows; row++)
        {
            const CSeq_id&  id = align->GetSeq_id(row);
            CBioseq_Handle bsh = scope->GetBioseqHandle(id);
            if (bsh)
            {
                seh = bsh.GetTopLevelEntry();
                if (seh)
                    break;
            }
        }
        CAlignmentAssistant *dlg = NULL;
        try
        {
            dlg = new CAlignmentAssistant( m_Window, seh, cmd_processor, ConstRef(align)); 
            dlg->Show(true);      
        } catch  (CException&) 
        {
            if (dlg)
                dlg->Destroy();
            wxMessageBox(wxT("Unable to load the alignment"), wxT("Error"), wxOK | wxICON_ERROR);
        }
        return;
    }

    if (!IsEditable(obj)) {
        return;
    }
    CSeq_entry_Handle seh = GetTopSeqEntryFromScopedObject(obj);

    CIRef<IEditObject> editor = CreateEditorForObject(obj.object, seh, false);
    CEditObjViewDlg* edit_dlg = new CEditObjViewDlg(NULL, false);
    edit_dlg->SetUndoManager(cmd_processor);
    wxWindow* editorWindow = editor->CreateWindow(edit_dlg);
    editorWindow->TransferDataToWindow();
    edit_dlg->SetEditorWindow(editorWindow);
    edit_dlg->SetEditor(editor);
    editorWindow->GetParent()->CenterOnParent();
    edit_dlg->Show(true);
}


void CTableViewWithTextViewNav::CombineObjects( 
    const TConstScopedObjects& objects, TObjectsMap& objects_map 
){
    typedef map<vector<CSeq_id_Handle>, TConstScopedObjects> TIdsToObjectsMap;
    TIdsToObjectsMap handle_groups;
    
    ITERATE( TConstScopedObjects, obtr, objects ){

        CConstRef<CObject> object = obtr->object;
        CRef<CScope> scope = obtr->scope;

        if( object.IsNull() || scope.IsNull() ){
            continue;
        }

        vector<CSeq_id_Handle> idh_vec;

        const CSeq_id* id = dynamic_cast<const CSeq_id*>( &*object );
        if( id ){
            CSeq_id_Handle idh = sequence::GetId( *id, *scope, sequence::eGetId_Canonical );
            idh_vec.push_back( idh ? idh : CSeq_id_Handle::GetHandle( *id ) );
        }

        const CSeq_loc* loc = dynamic_cast<const CSeq_loc*>( &*object );
        if( loc ){
            //! If loc is mix, we need to iterate ids
            const CSeq_id& seq_id = *loc->GetId();
            CSeq_id_Handle idh = sequence::GetId( seq_id, *scope, sequence::eGetId_Canonical );
            idh_vec.push_back( idh ? idh : CSeq_id_Handle::GetHandle( seq_id ) );
        }

        const CSeq_align* aln = dynamic_cast<const CSeq_align*>( &*object );
        if( aln ){
            int num_seqs = aln->CheckNumRows();
            if( num_seqs <= 0 ){
                continue;
            }

            set<CSeq_id_Handle> idh_set;
            for( int seqix = 0; seqix < num_seqs; seqix++ ){
                const CSeq_id& seq_id = aln->GetSeq_id( seqix );
                CSeq_id_Handle idh = sequence::GetId( seq_id, *scope, sequence::eGetId_Canonical );
                idh_set.insert( idh ? idh : CSeq_id_Handle::GetHandle( seq_id ) );
            }

            ITERATE( set<CSeq_id_Handle>, idh_itr, idh_set ){
                idh_vec.push_back( *idh_itr );
            }

            if( idh_vec.size() == 1 ){
                idh_vec.push_back( idh_vec.front() );
            }
        }


        handle_groups[idh_vec].push_back( *obtr );
    }

    objects_map.clear();
    NON_CONST_ITERATE( TIdsToObjectsMap, sidhtr, handle_groups ){
        const vector<CSeq_id_Handle>& idh_vec = sidhtr->first;
        TConstScopedObjects& group = sidhtr->second;
        
        string label;

        bool comma = false;
        ITERATE( vector<CSeq_id_Handle>, idhtr, idh_vec ){
            if( comma ){
                label += " x ";
            } else {
                comma = true;
            }

            CSeq_id_Handle idh = sequence::GetId( *idhtr, *group[0].scope, sequence::eGetId_Best );
            label += (idh ? idh : *idhtr).GetSeqId()->GetSeqIdString( true );
        }

        if( !label.empty() ){
            objects_map[label] = group;

        } else {
            NON_CONST_ITERATE( TConstScopedObjects, scobtr, group ){
                TConstScopedObjects single;
                single.push_back( *scobtr );

                CLabel::GetLabel( *scobtr->object, &label, CLabel::eContent, scobtr->scope );

                objects_map[label].push_back( *scobtr );
                label.clear();
            }
        }
    }
}

END_NCBI_SCOPE
