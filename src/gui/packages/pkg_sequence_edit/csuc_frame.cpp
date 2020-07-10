/*  $Id: csuc_frame.cpp 43609 2019-08-08 16:12:53Z filippov $
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
 * Authors:  Igor Filippov
 */


#include <ncbi_pch.hpp>

#include <gui/core/selection_service_impl.hpp>
#include <gui/objutils/objects.hpp>
#include <objmgr/util/sequence.hpp>
#include <objtools/edit/text_object_description.hpp>
#include <gui/widgets/edit/table_data_suc_job.hpp>
#include <gui/widgets/edit/suc_data_generator.hpp>
#include <gui/packages/pkg_sequence_edit/miscedit_util.hpp>
#include <gui/objutils/execute_lock_guard.hpp>
#include <gui/objects/GBWorkspace.hpp>
#include <gui/core/project_service.hpp>
#include <gui/core/simple_project_view.hpp>
#include <gui/widgets/edit/edit_obj_view_dlg.hpp>
#include <gui/widgets/edit/bioseq_editor.hpp>
#include <gui/widgets/data/report_dialog.hpp>
#include <gui/packages/pkg_sequence_edit/bulk_cds_edit.hpp>
#include <gui/packages/pkg_sequence_edit/bulk_rna_edit.hpp>
#include <gui/packages/pkg_sequence_edit/bulk_gene_edit.hpp>
#include <gui/packages/pkg_sequence_edit/srceditdialog.hpp>
#include <gui/packages/pkg_sequence_edit/csuc_frame.hpp>

// For compilers that support precompilation, includes "wx/wx.h".
#include "wx/wxprec.h"

#ifdef __BORLANDC__
#pragma hdrstop
#endif

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

////@begin includes
#include "wx/imaglist.h"
#include <wx/hyperlink.h>
////@end includes


////@begin XPM images
////@end XPM images

BEGIN_NCBI_SCOPE

/*
 * CClickableSortUniqueCount type definition
 */

IMPLEMENT_CLASS( CClickableSortUniqueCount, wxFrame )


/*
 * CClickableSortUniqueCount event table definition
 */

BEGIN_EVENT_TABLE( CClickableSortUniqueCount, wxFrame )

////@begin CClickableSortUniqueCount event table entries
    EVT_BUTTON(ID_CCLICKABLESORTUNIQUECOUNT_REFRESH, CClickableSortUniqueCount::OnRefresh)
    EVT_BUTTON(ID_CCLICKABLESORTUNIQUECOUNT_DISMISS, CClickableSortUniqueCount::OnDismiss)
    EVT_BUTTON(ID_CCLICKABLESORTUNIQUECOUNT_EXPAND, CClickableSortUniqueCount::OnExpand)
    EVT_BUTTON(ID_CCLICKABLESORTUNIQUECOUNT_COLLAPSE, CClickableSortUniqueCount::OnCollapse)
    EVT_TREE_SEL_CHANGED(ID_CCLICKABLESORTUNIQUECOUNT_TREECTRL, CClickableSortUniqueCount::OnTreeSelectionChanged)
    EVT_TREE_ITEM_ACTIVATED(ID_CCLICKABLESORTUNIQUECOUNT_TREECTRL, CClickableSortUniqueCount::OnTreeItemActivated)
    EVT_LIST_ITEM_SELECTED(ID_CCLICKABLESORTUNIQUECOUNT_TEXTCTRL, CClickableSortUniqueCount::OnListItemSelected)
    EVT_LIST_ITEM_ACTIVATED(ID_CCLICKABLESORTUNIQUECOUNT_TEXTCTRL, CClickableSortUniqueCount::OnListItemActivated)
    EVT_CHILD_FOCUS(CClickableSortUniqueCount::OnChildFocus)
////@end CClickableSortUniqueCount event table entries

END_EVENT_TABLE()

BEGIN_EVENT_MAP( CClickableSortUniqueCount, CEventHandler )
    ON_EVENT(CProjectViewEvent, CProjectViewEvent::eData, &CClickableSortUniqueCount::OnDataChanged)
END_EVENT_MAP()

/*
 * CClickableSortUniqueCount constructors
 */

CClickableSortUniqueCount::CClickableSortUniqueCount()
    :  m_Workbench(NULL)
{
    Init();
}

CClickableSortUniqueCount::CClickableSortUniqueCount( wxWindow* parent, IWorkbench*  wb, wxWindowID id, const wxString& caption, const wxPoint& pos, const wxSize& size, long style )
    :  m_Workbench(wb),  m_SelectionClient(new CSelectionClient("cSUC selections"))
{
    Init();
    Create( parent, id, RunningInsideNCBI() ? caption : _("FlatFile Summary"), pos, size, style );

    CIRef<CSelectionService> sel_srv = m_Workbench->GetServiceByType<CSelectionService>();
    if (sel_srv) sel_srv->AttachClient(m_SelectionClient);
    m_SelectionClientAttached = true;
    NEditingStats::ReportUsage(caption);
}


/*
 * CClickableSortUniqueCount creator
 */

bool CClickableSortUniqueCount::Create( wxWindow* parent, wxWindowID id, const wxString& caption, const wxPoint& pos, const wxSize& size, long style )
{
////@begin CClickableSortUniqueCount creation
    wxFrame::Create( parent, id, caption, pos, size, style );

    CreateControls();
    Centre(wxBOTH|wxCENTRE_ON_SCREEN);
////@end CClickableSortUniqueCount creation
    return true;
}


/*
 * CClickableSortUniqueCount destructor
 */

CClickableSortUniqueCount::~CClickableSortUniqueCount()
{
////@begin CClickableSortUniqueCount destruction
////@end CClickableSortUniqueCount destruction

    if (m_Workbench && m_SelectionClient && m_SelectionClientAttached) {
        CIRef<CSelectionService> sel_srv = m_Workbench->GetServiceByType<CSelectionService>();
        if (sel_srv) sel_srv->DetachClient(m_SelectionClient);
        m_SelectionClientAttached = false;
    }
    DisconnectListener();
    if (m_JobAdapter) m_JobAdapter->Cancel();
}


/*
 * Member initialisation
 */

void CClickableSortUniqueCount::Init()
{
////@begin CClickableSortUniqueCount member initialisation
    m_ListCtrl = NULL;
    m_TreeCtrl = NULL;
    m_IsBusy = false;
    m_TreeDeleteAllItemsFlag = false;
////@end CClickableSortUniqueCount member initialisation
}


/*
 * Control creation for CClickableSortUniqueCount
 */

void CClickableSortUniqueCount::CreateControls()
{    
////@begin CClickableSortUniqueCount content construction
    // Generated by DialogBlocks, 18/10/2016 11:02:51 (unregistered)

    CClickableSortUniqueCount* itemFrame1 = this;

    wxPanel* itemPanel2 = new wxPanel( itemFrame1, ID_CCLICKABLESORTUNIQUECOUNT_PANEL, wxDefaultPosition, wxDefaultSize, wxSUNKEN_BORDER|wxTAB_TRAVERSAL );

    wxBoxSizer* itemBoxSizer3 = new wxBoxSizer(wxVERTICAL);
    itemPanel2->SetSizer(itemBoxSizer3);

    m_TreeCtrl = new wxTreeCtrl( itemPanel2, ID_CCLICKABLESORTUNIQUECOUNT_TREECTRL, wxDefaultPosition, wxDefaultSize, wxTR_DEFAULT_STYLE|wxTR_HIDE_ROOT|wxTR_SINGLE );
    itemBoxSizer3->Add(m_TreeCtrl, 1, wxGROW|wxALL, 5);

    m_StatusText = new wxStaticText( itemPanel2, wxID_STATIC, _("Affected Items"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer3->Add(m_StatusText, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

    m_ListCtrl = new wxListCtrl( itemPanel2, ID_CCLICKABLESORTUNIQUECOUNT_TEXTCTRL, wxDefaultPosition, wxDefaultSize, wxLC_REPORT|wxLC_NO_HEADER|wxLC_SINGLE_SEL );
    itemBoxSizer3->Add(m_ListCtrl, 1, wxGROW|wxALL, 5);
    m_ListCtrl->InsertColumn(0,wxEmptyString,wxLIST_FORMAT_LEFT, 750);  


    wxBoxSizer* itemBoxSizer6 = new wxBoxSizer(wxHORIZONTAL);
    itemBoxSizer3->Add(itemBoxSizer6, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

    wxButton* itemButton7 = new wxButton( itemPanel2, ID_CCLICKABLESORTUNIQUECOUNT_REFRESH, _("Refresh"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer6->Add(itemButton7, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxButton* itemButton8 = new wxButton( itemPanel2, ID_CCLICKABLESORTUNIQUECOUNT_EXPAND, _("Expand All"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer6->Add(itemButton8, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxButton* itemButton9 = new wxButton( itemPanel2, ID_CCLICKABLESORTUNIQUECOUNT_COLLAPSE, _("Collapse All"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer6->Add(itemButton9, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxButton* itemButton10 = new wxButton( itemPanel2, ID_CCLICKABLESORTUNIQUECOUNT_DISMISS, _("Dismiss"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer6->Add(itemButton10, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxHyperlinkCtrl* itemHyperlinkCtrl = new wxHyperlinkCtrl( itemPanel2, wxID_HELP, _("Help"), wxT("https://www.ncbi.nlm.nih.gov/tools/gbench/manual8/#flat_file_summary"), wxDefaultPosition, wxDefaultSize, wxHL_DEFAULT_STYLE );
    itemHyperlinkCtrl->SetForegroundColour(wxColour(192, 192, 192));
    itemBoxSizer6->Add(itemHyperlinkCtrl, 0, wxALIGN_CENTER_VERTICAL, 5);

////@end CClickableSortUniqueCount content construction
    LoadData();
}

/*
 * Should we show tooltips?
 */

bool CClickableSortUniqueCount::ShowToolTips()
{
    return true;
}

/*
 * Get bitmap resources
 */

wxBitmap CClickableSortUniqueCount::GetBitmapResource( const wxString& name )
{
    // Bitmap retrieval
////@begin CClickableSortUniqueCount bitmap retrieval
    wxUnusedVar(name);
    return wxNullBitmap;
////@end CClickableSortUniqueCount bitmap retrieval
}

/*
 * Get icon resources
 */

wxIcon CClickableSortUniqueCount::GetIconResource( const wxString& name )
{
    // Icon retrieval
////@begin CClickableSortUniqueCount icon retrieval
    wxUnusedVar(name);
    return wxNullIcon;
////@end CClickableSortUniqueCount icon retrieval
}

void CClickableSortUniqueCount::LoadData()
{
    if (!m_IsBusy)
    {
        wxBeginBusyCursor();
        m_IsBusy = true;
    }
    
    if (m_Workbench && m_SelectionClient && m_SelectionClientAttached) 
    {
        CIRef<CSelectionService> sel_srv = m_Workbench->GetServiceByType<CSelectionService>();
        if (sel_srv) sel_srv->DetachClient(m_SelectionClient);
        TConstScopedObjects objs;
        m_SelectionClient->SetSelectedObjects(objs);
        m_SelectionClientAttached = false;
    }
    DisconnectListener();
    if (m_JobAdapter) m_JobAdapter->Cancel();

    ClearListCtrl();
   
    m_item_to_line.clear();
    TConstScopedObjects objects;
    CIRef<CSelectionService> sel_srv = m_Workbench->GetServiceByType<CSelectionService>();
    if (!sel_srv) {
        m_StatusText->SetLabel(_("Error"));
        if (m_IsBusy)
        {
            wxEndBusyCursor();
            m_IsBusy = false;
        }
        m_TreeDeleteAllItemsFlag = true;
        m_TreeCtrl->DeleteAllItems();
        m_TreeDeleteAllItemsFlag = false;
        Refresh();
        return;
    }

    sel_srv->GetActiveObjects(objects);
    if (objects.empty()) 
    {
        GetViewObjects(m_Workbench, objects);
    }
    if (objects.empty()){
        m_StatusText->SetLabel(_("No objects"));
        if (m_IsBusy)
        {
            wxEndBusyCursor();
            m_IsBusy = false;
        }
        m_TreeDeleteAllItemsFlag = true;
        m_TreeCtrl->DeleteAllItems();
        m_TreeDeleteAllItemsFlag = false;
        Refresh();
        return;
    }
    m_Scope = objects.front().scope;

    CIRef<CProjectService> prjSrv = m_Workbench->GetServiceByType<CProjectService>();
    CIRef<IProjectView> pTextView(prjSrv->FindView(*(objects.back().object), "Text View"));
    if (pTextView) 
    {
        m_FlatFileCtrl.Reset(dynamic_cast<IFlatFileCtrl*>(pTextView.GetPointerOrNull()));
    }

    string statusText;

    try {
        CIRef<CProjectService> srv = m_Workbench->GetServiceByType<CProjectService>();
        _ASSERT(srv);
        CRef<CGBWorkspace> ws = srv->GetGBWorkspace();
        CGBDocument* doc = 0;
        if (ws) 
            doc = dynamic_cast<CGBDocument*>(ws->GetProjectFromScope(*objects.front().scope));

        CRef<CTableDataSUCJob> job(new CTableDataSUCJob(objects));
        if (doc) 
        {
            job->SetDataLocker(new CWeakExecuteGuard(doc->GetUndoManager()));
        }
        m_JobAdapter.Reset(new CJobAdapter(*this));
        m_JobAdapter->Start(*job);
    } catch( CAppJobException& e ){
        if (m_JobAdapter) m_JobAdapter->Cancel();
        statusText = "Sort Unique Count failed!";
        LOG_POST( Error
            << "CTableDataSUC failed to start job: "
            << e.GetMsg()
        );
        LOG_POST(e.ReportAll());
    }

    if (!statusText.empty())
        m_StatusText->SetLabel(wxString(statusText));
}

void CClickableSortUniqueCount::ClearListCtrl(void)
{
    m_ListCtrl->DeleteAllItems();
    m_CurLines.clear();
}

void CClickableSortUniqueCount::OnRefresh(wxCommandEvent& event )
{   
    LoadData();
    Refresh();
}

void CClickableSortUniqueCount::OnDismiss(wxCommandEvent& event )
{
    Close();
}

void CClickableSortUniqueCount::OnExpand(wxCommandEvent& event )
{
    m_TreeCtrl->ExpandAll();
    Refresh();
}

void CClickableSortUniqueCount::OnCollapse(wxCommandEvent& event )
{
    m_TreeCtrl->CollapseAll();
    Refresh();
}

void CClickableSortUniqueCount::GetExpandedNodes(wxTreeItemId id, set<string> &expanded_nodes) 
{    
    if (id.IsOk() && m_TreeCtrl->ItemHasChildren(id) && !m_TreeDeleteAllItemsFlag)
    {
        if (id != m_TreeCtrl->GetRootItem() && m_TreeCtrl->IsExpanded(id))
        {
            string label = m_TreeCtrl->GetItemText(id).ToStdString();
            expanded_nodes.insert(label);
        }
        wxTreeItemIdValue cookie;
        wxTreeItemId child = m_TreeCtrl->GetFirstChild(id, cookie);
        while (child.IsOk())
        {
            GetExpandedNodes(child, expanded_nodes);
            child = m_TreeCtrl->GetNextChild(id, cookie);
        }
    }
}

void CClickableSortUniqueCount::SetExpandedNodes(wxTreeItemId id, const set<string> &expanded_nodes)
{
    if (id.IsOk() && m_TreeCtrl->ItemHasChildren(id) && !m_TreeDeleteAllItemsFlag)
    {
        if (id != m_TreeCtrl->GetRootItem())
        {
            string label = m_TreeCtrl->GetItemText(id).ToStdString();
            if (expanded_nodes.find(label) != expanded_nodes.end())
                m_TreeCtrl->Expand(id);
            else
                m_TreeCtrl->Collapse(id);
        }
        wxTreeItemIdValue cookie;
        wxTreeItemId child = m_TreeCtrl->GetFirstChild(id, cookie);
        while (child.IsOk())
        {
            SetExpandedNodes(child, expanded_nodes);
            child = m_TreeCtrl->GetNextChild(id, cookie);
        }
    }
}

void CClickableSortUniqueCount::UpdateTree(CRef<CSUCResults> res)
{
    Freeze();
    set<string> expanded_nodes;
    GetExpandedNodes(m_TreeCtrl->GetRootItem(), expanded_nodes);

    m_TreeDeleteAllItemsFlag = true;
    m_TreeCtrl->DeleteAllItems();
    m_TreeDeleteAllItemsFlag = false;
    m_item_to_line.clear();
    if (res)
    {
        wxTreeItemId root = m_TreeCtrl->AddRoot(wxEmptyString);
        const auto& order = res->GetOrder();
        const auto& blocks = res->GetBlocks();
        for (auto block_type : order)
        {
            if (block_type == CFlatFileConfig::fGenbankBlocks_Featheader)
                continue;
            const char *block_label = CSUCBlock::GetBlockLabel(block_type);
            if (NStr::IsBlank(block_label))
                continue;
            wxTreeItemId parent = m_TreeCtrl->AppendItem(root,wxString(block_label));
            auto block_it = blocks.find(block_type);
            if (block_it == blocks.end())
                continue;
            auto block = block_it->second;
            if (block->HasSecondLevel())
            {
                const auto& nodes = block->GetSecondLevel();
                for (const auto& node : nodes)
                {
                    size_t total = 0;
                    for (const auto& line : node.second)                        
                    {
                        if (line.second.first)
                            total += line.second.second->GetCount();
                    }
                    wxString label;
                    label << total << "   " << node.first;
                    wxTreeItemId second = m_TreeCtrl->AppendItem(parent, label);
                    for (const auto& line : node.second)
                    {
                        wxString text;
                        text << line.second.second->GetCount();
                        text << "   ";
                        string str = line.second.second->GetLine();
                        NStr::TruncateSpacesInPlace(str);
                        text << str;
                        wxTreeItemId id = m_TreeCtrl->AppendItem(second, text);
                        m_item_to_line[id] = line.second.second;
                    }
                }
                continue;
            }
            const auto& lines = block->GetLines();
            for (const auto& line : lines)
            {
                wxString text;
                text << line.second->GetCount();
                text << "   ";
                string str = line.second->GetLine();
                NStr::TruncateSpacesInPlace(str);
                text << str;
                wxTreeItemId id = m_TreeCtrl->AppendItem(parent, text);
                m_item_to_line[id] = line.second;
            }
        }
    }
    ClearListCtrl();
    SetExpandedNodes(m_TreeCtrl->GetRootItem(), expanded_nodes);
    Thaw();
}

void CClickableSortUniqueCount::OnTreeSelectionChanged( wxTreeEvent& event )
{
    ClearListCtrl();
    if (m_TreeCtrl->IsEmpty() || m_TreeDeleteAllItemsFlag)
        return;
    
    wxTreeItemId id = event.GetItem();
    if (!id.IsOk())
        return;
    if (m_TreeCtrl->GetItemParent(id) == m_TreeCtrl->GetRootItem())
        return;
  
    if (m_TreeCtrl->ItemHasChildren(id))
    {
        wxTreeItemIdValue cookie;
        wxTreeItemId child = m_TreeCtrl->GetFirstChild(id, cookie);
        while (child.IsOk())
        {
            AddToListCtrl(child);
            child = m_TreeCtrl->GetNextChild(id, cookie);
        }
    }
    else
    {
        AddToListCtrl(id);
    }

    if (m_ListCtrl->GetItemCount() > 0)
    {
        m_ListCtrl->SetColumnWidth(0, wxLIST_AUTOSIZE);
        m_ListCtrl->SetItemState(0, wxLIST_STATE_SELECTED, wxLIST_STATE_SELECTED);
    }   
    else
    {
        auto it = m_item_to_line.find(id);
        if (it == m_item_to_line.end() || !m_FlatFileCtrl)
        {        
            return;
        }
        CSUCLine& line = *(it->second);
        if (line.GetCount() != 1)
            return;
        const string &accession = line.GetAccession();
        CRef<CSeq_id> id(new CSeq_id(accession, CSeq_id::fParse_Default));
        CBioseq_Handle bsh = m_Scope->GetBioseqHandle(*id);
       
        CConstRef<CObject> obj;
        if (line.GetRelatedObjectCount() > 0)
            obj = line.GetRelatedObject(0);
        if (bsh)
            m_FlatFileCtrl->SetPosition(bsh, obj);
        
        if (m_SelectionClient && m_SelectionClientAttached && obj) 
        {
            TConstScopedObjects objs;
            objs.push_back(SConstScopedObject(obj, m_Scope));
            m_SelectionClient->SetSelectedObjects(objs);
        }
    }
}

void CClickableSortUniqueCount::AddToListCtrl(wxTreeItemId& id)
{
    auto it = m_item_to_line.find(id);
    if (it == m_item_to_line.end())
    {        
        return;
    }
    CSUCLine& line = *(it->second);
 
    for (size_t i = 0; i < line.GetRelatedObjectCount(); i++) 
    {
        wxString text;
        CConstRef<CObject> obj = line.GetRelatedObject(i);
        if (obj) 
        {        
            const CSeq_feat* f = dynamic_cast<const CSeq_feat*>(obj.GetPointer());
            const CSeqdesc* d = dynamic_cast<const CSeqdesc*>(obj.GetPointer());
            const CBioseq* b = dynamic_cast<const CBioseq*>(obj.GetPointer());
            const CBioseq_set* s = dynamic_cast<const CBioseq_set*>(obj.GetPointer());

            if (f) {
                text << edit::GetTextObjectDescription(*f, *m_Scope);
            } else if (d && m_Scope) {
                text << edit::GetTextObjectDescription(*d, *m_Scope);
            } else if (b) {
                text << edit::GetTextObjectDescription(*b, *m_Scope);
            } else if (s) {
                text << edit::GetTextObjectDescription(*s, *m_Scope);
            }
        }
        if (text.IsEmpty()) 
        {
            text << line.GetRelatedObjectAccession(i);
        }
        long item = m_ListCtrl->GetItemCount();
        item = m_ListCtrl->InsertItem(item, text);
        m_CurLines.push_back(make_pair(it->second, i));
    }    
}

void CClickableSortUniqueCount::OnTreeItemActivated( wxTreeEvent& event )
{
    CIRef<CProjectService> srv = m_Workbench->GetServiceByType<CProjectService>();
    if (!srv)
        return;

    CRef<CGBWorkspace> ws = srv->GetGBWorkspace();
    if (!ws) 
        return;

    CGBDocument* doc = dynamic_cast<CGBDocument*>(ws->GetProjectFromScope(*m_Scope));
    if (!doc)
        return;
    ICommandProccessor* cmdProcessor = &doc->GetUndoManager();    
   
    wxTreeItemId id = event.GetItem();
    if (m_TreeCtrl->GetItemParent(id) == m_TreeCtrl->GetRootItem())
        return;

    vector<CSeq_feat_Handle> cds, gene, rna;
    vector<CBioseq_Handle> biosrc;

    if (m_TreeCtrl->ItemHasChildren(id))
    {
        wxTreeItemIdValue cookie;
        wxTreeItemId child = m_TreeCtrl->GetFirstChild(id, cookie);
        while (child.IsOk())
        {
            AddToBulkEditList(child, cds, gene, rna, biosrc);
            child = m_TreeCtrl->GetNextChild(id, cookie);
        }
    }
    else
    {
        AddToBulkEditList(id, cds, gene, rna, biosrc);
    }

    CRef<CCmdComposite> cmd;
    try 
    {
        if (!cds.empty() && gene.empty() && rna.empty() && biosrc.empty())
        {
            CBulkCDS dlg(this, cds, m_Workbench);
            if (dlg.ShowModal() == wxID_OK) 
                cmd = dlg.GetCommand();                          
        }
        if (cds.empty() && !gene.empty() && rna.empty() && biosrc.empty())
        {
            CBulkGene dlg(this, gene, m_Workbench);
            if (dlg.ShowModal() == wxID_OK) 
                cmd = dlg.GetCommand();                          
        }
        if (cds.empty() && gene.empty() && !rna.empty() && biosrc.empty())
        {
            CBulkRna dlg(this, rna, m_Workbench);
            if (dlg.ShowModal() == wxID_OK) 
                cmd = dlg.GetCommand();                          
        }
        if (cds.empty() && gene.empty() && rna.empty() && !biosrc.empty())
        {
            SrcEditDialog dlg(this, biosrc, m_Workbench);
            if (dlg.ShowModal() == wxID_OK) 
                cmd = dlg.GetCommand();                          
        }       
    }
    catch (exception& e) 
    {
        wxMessageBox(ToWxString(e.what()), wxT("Error"), wxOK | wxICON_ERROR, NULL);
    }
    if (cmd) 
    {
        cmdProcessor->Execute(cmd);   
    }
}

void CClickableSortUniqueCount::AddToBulkEditList(wxTreeItemId& id, vector<CSeq_feat_Handle> &cds, vector<CSeq_feat_Handle> &gene, vector<CSeq_feat_Handle> &rna,
                                                  vector<CBioseq_Handle> &biosrc)
{
    auto it = m_item_to_line.find(id);
    if (it == m_item_to_line.end())
    {
        return;
    }
    CSUCLine& line = *(it->second);

    for (size_t i = 0; i < line.GetRelatedObjectCount(); i++) 
    {
        CConstRef<CObject> obj = line.GetRelatedObject(i);
        if (!obj)
            continue;
        const CSeq_feat* f = dynamic_cast<const CSeq_feat * >(obj.GetPointer());
        const CSeqdesc* d = dynamic_cast<const CSeqdesc *>(obj.GetPointer());
        if (f && f->IsSetData())
        {
            CSeqFeatData::ESubtype subtype = f->GetData().GetSubtype();
            CSeq_feat_Handle fh = m_Scope->GetSeq_featHandle(*f);
            CBioseq_Handle bsh = m_Scope->GetBioseqHandle(f->GetLocation());
            switch(subtype)
            {
            case CSeqFeatData::eSubtype_cdregion: cds.push_back(fh); break;
            case CSeqFeatData::eSubtype_gene: gene.push_back(fh); break;
            case CSeqFeatData::eSubtype_rRNA: rna.push_back(fh); break;
            case CSeqFeatData::eSubtype_biosrc: biosrc.push_back(bsh); break;
            default: break;
            }                
        }
        if (d && d->IsSource()) 
        {
            CSeq_entry_Handle seh = edit::GetSeqEntryForSeqdesc(m_Scope, *d);
            if (seh)
            {
                CBioseq_CI bi(seh, CSeq_inst::eMol_na);
                if (bi) 
                {
                    biosrc.push_back(*bi);
                }
            }
        }
    }
}

void CClickableSortUniqueCount::OnListItemSelected(wxListEvent& event)
{
    long index = event.GetIndex();

    if (m_FlatFileCtrl && index < m_CurLines.size() && m_CurLines[index].first)
    {        
        string accession = m_CurLines[index].first->GetRelatedObjectAccession(m_CurLines[index].second);
        CRef<CSeq_id> id(new CSeq_id(accession, CSeq_id::fParse_Default));
        CBioseq_Handle bsh = m_Scope->GetBioseqHandle(*id);
        if (bsh)
        {
            m_FlatFileCtrl->SetPosition(bsh, m_CurLines[index].first->GetRelatedObject(m_CurLines[index].second));
        }

        if (m_SelectionClient && m_SelectionClientAttached) {
            CConstRef<CObject> obj = m_CurLines[index].first->GetRelatedObject(m_CurLines[index].second);
            if (obj)
            {
                TConstScopedObjects objs;
                objs.push_back(SConstScopedObject(obj, m_Scope));
                m_SelectionClient->SetSelectedObjects(objs);
            }
        }
    }
}

void CClickableSortUniqueCount::OnListItemActivated(wxListEvent& event)
{
    long index = event.GetIndex();
    if (index >= m_CurLines.size() || !m_CurLines[index].first)
        return;
 

    CIRef<CProjectService> srv = m_Workbench->GetServiceByType<CProjectService>();
    if (!srv)
        return;
    
    CRef<CGBWorkspace> ws = srv->GetGBWorkspace();
    if (!ws) 
        return;
    
    CGBDocument* doc = dynamic_cast<CGBDocument*>(ws->GetProjectFromScope(*m_Scope));
    if (!doc)
        return;
    ICommandProccessor* cmdProcessor = &doc->GetUndoManager();   

    CConstRef<CObject> obj = m_CurLines[index].first->GetRelatedObject(m_CurLines[index].second);
    if (!obj)
        return;

    CSeq_entry_Handle seh;
    const CSeq_feat* f = dynamic_cast<const CSeq_feat * >(obj.GetPointer());
    const CSeqdesc* d = dynamic_cast<const CSeqdesc *>(obj.GetPointer());
    if (f && f->IsSetLocation())
    {
        CBioseq_Handle bsh = m_Scope->GetBioseqHandle(f->GetLocation());
        if (bsh)
            seh = bsh.GetSeq_entry_Handle();
    }
    if (d) 
    {
        seh = edit::GetSeqEntryForSeqdesc(m_Scope, *d); 
    }
    if (!seh)
    {
        CScope::TTSE_Handles tses;
        m_Scope->GetAllTSEs(tses, CScope::eAllTSEs);
        if (!tses.empty())
            seh = tses.front();
    }
    
    CIRef<IEditObject> editor = CreateEditorForObject(obj, seh, false);
    CEditObjViewDlgModal edit_dlg(this, false);
    wxWindow* editorWindow = editor->CreateWindow(&edit_dlg);
    editorWindow->TransferDataToWindow(); 
    edit_dlg.SetEditorWindow(editorWindow);
    edit_dlg.SetEditor(editor);

    if (edit_dlg.ShowModal() == wxID_OK) 
    {
        CIRef<IEditCommand> cmd(editor->GetEditCommand());
        if (cmd) 
        {
            cmdProcessor->Execute(cmd);   
        }
    }
}

void CClickableSortUniqueCount::OnChildFocus(wxChildFocusEvent& evt)
{
    if (m_Workbench && m_SelectionClient && m_SelectionClientAttached) {
        CIRef<CSelectionService> sel_srv = m_Workbench->GetServiceByType<CSelectionService>();
        if (sel_srv) sel_srv->OnActiveClientChanged(m_SelectionClient);
    }
}

void CClickableSortUniqueCount::OnJobResult(CObject* result, CJobAdapter&)
{
    string statusText;
    CRef<CSUCResults> res;
    
    res.Reset(dynamic_cast<CSUCResults*>(result));
    if (!res) 
    {
        m_StatusText->SetLabel(_("Sort Unique Count failed!"));
    }
    else
    {
        m_StatusText->SetLabel(_("Affected Items"));
    }
    
    
    UpdateTree(res);
    ConnectListener();
    if (!m_SelectionClientAttached)
    {
        CIRef<CSelectionService> sel_srv = m_Workbench->GetServiceByType<CSelectionService>();
        if (sel_srv) sel_srv->AttachClient(m_SelectionClient);
        m_SelectionClientAttached = true;
    }
    if (m_IsBusy)
    {
        wxEndBusyCursor();
        m_IsBusy = false;
    }
}

void CClickableSortUniqueCount::OnJobFailed(const string& errMsg, CJobAdapter&)
{
    string statusText = "Failed: ";
    CRef<CSUCResults> res;

    if (!errMsg.empty())
        statusText += errMsg;
    else
        statusText += "Unknown fatal error";

    m_StatusText->SetLabel(wxString(statusText));
    UpdateTree(res);  
    ConnectListener();  
    if (!m_SelectionClientAttached)
    {
        CIRef<CSelectionService> sel_srv = m_Workbench->GetServiceByType<CSelectionService>();
        if (sel_srv) sel_srv->AttachClient(m_SelectionClient);
        m_SelectionClientAttached = true;
    }
    if (m_IsBusy)
    {
        wxEndBusyCursor();
        m_IsBusy = false;
    }
}

void CClickableSortUniqueCount::OnDataChanged(CEvent* evt)
{
    if (evt)
    {
        m_StatusText->SetLabel(_("Data has been modified, please refresh"));
    }
}

void CClickableSortUniqueCount::ConnectListener()
{
    if (!m_Workbench || !m_Scope)
        return;

    CIRef<CProjectService> srv = m_Workbench->GetServiceByType<CProjectService>();
    if (!srv)
        return;
    CRef<CGBWorkspace> ws = srv->GetGBWorkspace();
    if (!ws)
        return;
    CGBDocument* doc = dynamic_cast<CGBDocument*>(ws->GetProjectFromScope(*m_Scope));        
    if (!doc)
        return;
    doc->AddListener(this);
}

void CClickableSortUniqueCount::DisconnectListener()
{
    if (!m_Workbench || !m_Scope)
        return;
    
    CIRef<CProjectService> srv = m_Workbench->GetServiceByType<CProjectService>();
    if (!srv)
        return;
    CRef<CGBWorkspace> ws = srv->GetGBWorkspace();
    if (!ws)
        return;
    CGBDocument* doc = dynamic_cast<CGBDocument*>(ws->GetProjectFromScope(*m_Scope));        
    if (!doc)
        return;
    doc->RemoveListener(this);
}

END_NCBI_SCOPE

