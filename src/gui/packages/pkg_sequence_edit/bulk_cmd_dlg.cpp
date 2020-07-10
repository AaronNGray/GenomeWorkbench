/*  $Id: bulk_cmd_dlg.cpp 42432 2019-02-22 18:44:43Z filippov $
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
 *  and reliability of the software and data,  the NLM and the U.S.
 *  Government do not and cannot warrant the performance or results that
 *  may be obtained by using this software or data. The NLM and the U.S.
 *  Government disclaim all warranties,  express or implied,  including
 *  warranties of performance,  merchantability or fitness for any particular
 *  purpose.
 *
 *  Please cite the author in any work or product based on this material.
 *
 * ===========================================================================
 *
 * Authors:  Colleen Bollin
 */


#include <ncbi_pch.hpp>

////@begin includes
////@end includes

#include <objects/submit/Seq_submit.hpp>
#include <gui/core/selection_service_impl.hpp>
#include <gui/core/project_service.hpp>
#include <gui/objutils/utils.hpp>
#include <gui/packages/pkg_sequence_edit/bulk_cmd_dlg.hpp>
#include <gui/packages/pkg_sequence_edit/existingtextdlg.hpp>
#include <gui/widgets/edit/feature_type_panel.hpp>
#include <gui/widgets/edit/struct_comm_field_panel.hpp>
#include <gui/widgets/edit/struct_fieldvalue_panel.hpp>
#include <gui/packages/pkg_sequence_edit/miscedit_util.hpp>

#include <gui/objects/GBWorkspace.hpp>
#include <gui/objects/WorkspaceFolder.hpp>

#include <wx/msgdlg.h>
#include <wx/sizer.h>
#include <wx/app.h>

////@begin XPM images
////@end XPM images

BEGIN_NCBI_SCOPE

USING_SCOPE(ncbi::objects);

IMPLEMENT_ABSTRACT_CLASS( CBulkCmdDlg, wxFrame )

BEGIN_EVENT_TABLE( CBulkCmdDlg, wxFrame )

EVT_COMMAND(wxID_CANCEL, wxEVT_BUTTON, CBulkCmdDlg::onCancelPressed)

END_EVENT_TABLE()

bool CBulkCmdDlg::Create (wxWindow *parent, wxWindowID id, const wxString &title, const wxPoint &pos, const wxSize &size, long style, const wxString &name)
{
    if ( !parent )
    {
        parent = wxTheApp->GetTopWindow();      
    }
	
	bool res = wxFrame::Create(parent, id, title, pos, size, style, name);	
	wxVisualAttributes attr = wxPanel::GetClassDefaultAttributes();
	if (attr.colBg != wxNullColour)
	    SetOwnBackgroundColour(attr.colBg);
	return res;
}

bool CBulkCmdDlg::Create (wxWindow *parent, IWorkbench* wb, wxWindowID id, const wxString &title, const wxPoint &pos, const wxSize &size, long style, const wxString &name)
{
    m_Workbench = wb;
    if ( !parent )
    {
        parent = wxTheApp->GetTopWindow();      
    }
    return wxFrame::Create(parent, id, title, pos, size, style, name);	
}

void CBulkCmdDlg::onCancelPressed( wxCommandEvent& event )
{
	Destroy();
}

bool CBulkCmdDlg::GetTopLevelSeqEntryAndProcessor()
{
    m_TopSeqEntry.Reset();
    m_SeqSubmit.Reset(NULL);
    m_CmdProccessor = NULL;
    if (!m_Workbench) return false;
    
    TConstScopedObjects objects;
    CIRef<CSelectionService> sel_srv = m_Workbench->GetServiceByType<CSelectionService>();
    if (!sel_srv) return false;
    sel_srv->GetActiveObjects(objects);
    if (objects.empty())
    {
        GetViewObjects(m_Workbench, objects);
    }
    if (objects.empty()) 
        return false;
    NON_CONST_ITERATE (TConstScopedObjects, it, objects) {
        const CSeq_submit* sub = dynamic_cast<const CSeq_submit*>((*it).object.GetPointer());
        if (sub) {
            m_SeqSubmit.Reset(sub);
        }
        CSeq_entry_Handle seh = GetTopSeqEntryFromScopedObject(*it);

        if (seh) {
            m_TopSeqEntry = seh;
        }
    }
    if (!m_TopSeqEntry) return false;
    CIRef<CProjectService> srv = m_Workbench->GetServiceByType<CProjectService>();
    if (srv)
    {
        CRef<CGBWorkspace> ws = srv->GetGBWorkspace();
        if (!ws) return false;

        CGBDocument* doc = dynamic_cast<CGBDocument*>(ws->GetProjectFromScope(m_TopSeqEntry.GetScope()));
        if (doc)
            m_CmdProccessor = &doc->GetUndoManager(); 
    }
    return true;
}


void CBulkCmdDlg::ExecuteCmd(CRef<CCmdComposite> cmd)
{
    m_CmdProccessor->Execute(cmd);
}

TConstScopedObjects CBulkCmdDlg::GetSelectedObjects()
{
    TConstScopedObjects sel_objects;
    if (!m_Workbench) {
        return sel_objects;
    }

    CIRef<CSelectionService> sel_srv = m_Workbench->GetServiceByType<CSelectionService>();
    if (sel_srv) 
    {
        sel_srv->GetActiveObjects(sel_objects);
    }
    if (sel_objects.empty())
    {
        GetViewObjects(m_Workbench, sel_objects);
    }
    return sel_objects;
}

void CBulkCmdDlg::UpdateChildrenFeaturePanels( wxSizer* sizer )
{
    wxSizerItemList& slist = sizer->GetChildren();
    int n =0;
    for (wxSizerItemList::iterator iter = slist.begin(); iter != slist.end(); ++iter, ++n) {
        if ((*iter)->IsSizer()) {
            UpdateChildrenFeaturePanels((*iter)->GetSizer());
        } else if ((*iter)->IsWindow()) {
            wxWindow* child = (*iter)->GetWindow();
            if (child) {
                CFeatureTypePanel* panel = dynamic_cast<CFeatureTypePanel*>(child);
                if (panel) {
                    GetTopLevelSeqEntryAndProcessor();
                    panel->ListPresentFeaturesFirst(m_TopSeqEntry);
                } else {
                    wxSizer* subsizer = child->GetSizer();
                    if (subsizer) {
                        UpdateChildrenFeaturePanels(subsizer);
                    } else {
                        // do nothing
                    }    
                }
            }
        } 
    }    
}


edit::EExistingText CBulkCmdDlg::x_GetExistingTextHandling (int num_conflicts, bool allow_multiple)
{
    edit::EExistingText existing_text = edit::eExistingText_replace_old;
    if (num_conflicts > 0) {
        string msg = "Do you want to overwrite " + NStr::NumericToString(num_conflicts) + " existing values?";
        int answer = wxMessageBox(ToWxString(msg), wxT("Existing Text"),
                                  wxYES_NO | wxCANCEL | wxICON_QUESTION, this);
        if (answer == wxCANCEL) {
            existing_text = edit::eExistingText_cancel;
        } else if (answer == wxYES) {
            existing_text = edit::eExistingText_replace_old;
        } else {
            CExistingTextDlg dlg(this);
            dlg.AllowAdditionalQual(allow_multiple);
            if (dlg.ShowModal() == wxID_OK) {
                existing_text = dlg.GetExistingTextHandler();
            } else {
                existing_text = edit::eExistingText_cancel;
            }
            dlg.Close();
        }
    }
            
    return existing_text;
}


void AddSize(wxSize& orig, const wxSize& add, int borders)
{
    if (orig.GetWidth() < add.GetWidth() + borders) {
        orig.SetWidth(add.GetWidth() + borders);
    }
    orig.SetHeight(orig.GetHeight() + add.GetHeight() + borders);
}



END_NCBI_SCOPE


