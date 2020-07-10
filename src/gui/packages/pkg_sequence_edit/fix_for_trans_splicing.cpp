/*  $Id: fix_for_trans_splicing.cpp 42192 2019-01-10 16:52:15Z filippov $
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

////@begin includes
////@end includes
#include <sstream>
#include <objmgr/scope.hpp>
#include <objects/misc/sequence_macros.hpp>
#include <objmgr/feat_ci.hpp>
#include <objmgr/seq_annot_ci.hpp>
#include <objmgr/bioseq_ci.hpp>
#include <objmgr/seq_feat_handle.hpp>
#include <objmgr/util/feature.hpp>
#include <objtools/edit/text_object_description.hpp>
#include <gui/objutils/util_cmds.hpp>
#include <gui/objutils/cmd_del_seq_annot.hpp>
#include <gui/objutils/cmd_del_bioseq.hpp>
#include <gui/objutils/utils.hpp>
#include <objtools/edit/cds_fix.hpp>
#include <gui/core/project_service.hpp>
#include <gui/core/simple_project_view.hpp>
#include <gui/widgets/edit/edit_obj_view_dlg.hpp>
#include <gui/widgets/edit/bioseq_editor.hpp>
#include <gui/widgets/edit/generic_report_dlg.hpp>
#include <gui/packages/pkg_sequence_edit/ok_cancel_panel.hpp>
#include <gui/packages/pkg_sequence_edit/miscedit_util.hpp>
#include <gui/packages/pkg_sequence_edit/fix_for_trans_splicing.hpp>



////@begin XPM images
////@end XPM images

BEGIN_NCBI_SCOPE



IMPLEMENT_DYNAMIC_CLASS( CFixForTransSplicing, CBulkCmdDlg )


BEGIN_EVENT_TABLE( CFixForTransSplicing, CBulkCmdDlg )
EVT_CHOICE( ID_PROTEIN_NAME, CFixForTransSplicing::OnProteinNameSelected )
EVT_BUTTON( ID_PAGE_LEFT, CFixForTransSplicing::OnPageLeft)
EVT_BUTTON( ID_PAGE_RIGHT, CFixForTransSplicing::OnPageRight)
EVT_LISTBOX(ID_CDS_LIST, CFixForTransSplicing::OnListItemSelected)
EVT_LISTBOX_DCLICK(ID_CDS_LIST, CFixForTransSplicing::OnListItemActivated)
END_EVENT_TABLE()

CFixForTransSplicing::CFixForTransSplicing()
{
    Init();
}

CFixForTransSplicing::CFixForTransSplicing( wxWindow* parent, IWorkbench *wb, 
                                        wxWindowID id, const wxString& caption, const wxPoint& pos, const wxSize& size, long style )
    : CBulkCmdDlg(wb),  m_SelectionClient(new CSelectionClient("FixForTransSplicing selections")), m_SelectionClientAttached(false)
{   
    Init();
    Create(parent, id, caption, pos, size, style);
}


bool CFixForTransSplicing::Create( wxWindow* parent, wxWindowID id, const wxString& caption, const wxPoint& pos, const wxSize& size, long style )
{
    SetExtraStyle(wxWS_EX_BLOCK_EVENTS);
    CBulkCmdDlg::Create( parent, id, caption, pos, size, style );

    CreateControls();
    if (GetSizer())
    {
        GetSizer()->SetSizeHints(this);
    }
    Centre();

    SetSize(wxSize(280, 550));
    return true;
}


CFixForTransSplicing::~CFixForTransSplicing()
{
    if (m_Workbench && m_SelectionClient && m_SelectionClientAttached) 
    {
        CIRef<CSelectionService> sel_srv = m_Workbench->GetServiceByType<CSelectionService>();
        if (sel_srv) 
        {
            sel_srv->DetachClient(m_SelectionClient);
            m_SelectionClientAttached = false;
        }
    }
}


/*!
 * Member initialisation
 */

void CFixForTransSplicing::Init()
{
    GetTopLevelSeqEntryAndProcessor(); 
    m_Protein = nullptr;  
    CollectProteinNames();
    m_ListBox = nullptr;
    m_PageLeft = nullptr;
    m_PageRight = nullptr;
    m_BioseqLabel = nullptr;
    m_page = 0;
    CIRef<CSelectionService> sel_srv = m_Workbench->GetServiceByType<CSelectionService>();
    if (sel_srv) 
    {
        TConstScopedObjects objects;
        sel_srv->GetActiveObjects(objects);
        if (objects.empty()) 
        {
            GetViewObjects(m_Workbench, objects);
        }
        CIRef<CProjectService> prjSrv = m_Workbench->GetServiceByType<CProjectService>();
        CIRef<IProjectView> pTextView(prjSrv->FindView(*(objects.back().object), "Text View"));
        if (pTextView) 
        {
            m_FlatFileCtrl.Reset(dynamic_cast<IFlatFileCtrl*>(pTextView.GetPointerOrNull()));
        }
        sel_srv->AttachClient(m_SelectionClient);
        m_SelectionClientAttached = true;
    }
}




void CFixForTransSplicing::CreateControls()
{    
    CFixForTransSplicing* itemDialog1 = this;

    wxBoxSizer* itemBoxSizer2 = new wxBoxSizer(wxVERTICAL);
    itemDialog1->SetSizer(itemBoxSizer2);

    m_Protein = new wxChoice( itemDialog1, ID_PROTEIN_NAME, wxDefaultPosition, wxDefaultSize, m_ProteinNames, wxCB_SORT );
    itemBoxSizer2->Add(m_Protein, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

    wxArrayString choices;
    m_ListBox = new wxListBox(itemDialog1, ID_CDS_LIST, wxDefaultPosition, wxDefaultSize, choices, wxLB_SINGLE);
    itemBoxSizer2->Add(m_ListBox, 1, wxGROW|wxALL, 5);

    m_BioseqLabel = new wxStaticText( itemDialog1, wxID_STATIC, _("       "), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer2->Add(m_BioseqLabel, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

    wxBoxSizer* itemBoxSizer3 = new wxBoxSizer(wxHORIZONTAL);
    itemBoxSizer2->Add(itemBoxSizer3, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

    m_PageLeft = new wxButton( itemDialog1, ID_PAGE_LEFT, _("<<<"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer3->Add( m_PageLeft, 0, wxALIGN_CENTER_VERTICAL|wxALL, 15);

    m_PageRight = new wxButton( itemDialog1, ID_PAGE_RIGHT, _(">>>"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer3->Add( m_PageRight, 0, wxALIGN_CENTER_VERTICAL|wxALL, 15);

    COkCancelPanel *OkCancel = new COkCancelPanel( itemDialog1, wxID_ANY, wxDefaultPosition, wxSize(100, 100), 0 );
    itemBoxSizer2->Add(OkCancel, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

    m_Protein->SetStringSelection(_("ribosomal protein S12"));
    PopulateListBox();
}

void CFixForTransSplicing::CollectProteinNames()
{
    wxBusyCursor wait;
    m_ProteinNames.Clear();
    m_name_to_cds.clear();
    m_bioseqs.clear();
    m_name_to_obj.clear();
    CScope& scope = m_TopSeqEntry.GetScope();
    for (CBioseq_CI bi(m_TopSeqEntry, CSeq_inst::eMol_na); bi; ++bi)
    {
        CBioseq_Handle bsh = *bi;

        for (CFeat_CI cds(bsh, CSeqFeatData::eSubtype_cdregion); cds; ++cds)
        {
            if (cds->IsSetProduct())
            {
                CBioseq_Handle prot_bsh = scope.GetBioseqHandle(cds->GetProduct());
                if (prot_bsh)
                {
                    CFeat_CI prot_feat_ci(prot_bsh, CSeqFeatData::e_Prot);
                    if (prot_feat_ci)
                    {
                        FOR_EACH_NAME_ON_PROTREF(name_it, prot_feat_ci->GetSeq_feat()->GetData().GetProt())
                        {
                            if (!(*name_it).empty())
                            {
                                if (m_name_to_cds.find(*name_it) == m_name_to_cds.end())
                                    m_ProteinNames.Add(wxString(*name_it));
                                m_name_to_cds[*name_it][bsh].Add(NStr::Replace(edit::GetTextObjectDescription(*cds->GetSeq_feat(), scope), "\t", " "));
                                m_name_to_obj[*name_it][bsh].push_back(cds->GetSeq_feat_Handle());
                                if (m_bioseqs.empty() || m_bioseqs.back() != bsh)
                                    m_bioseqs.push_back(bsh);
                                break;
                            }
                        }
                    }
                }
            }
        }
    }
}

void CFixForTransSplicing::OnProteinNameSelected( wxCommandEvent& event )
{
    PopulateListBox();
}

void CFixForTransSplicing::PopulateListBox()
{
    string name = m_Protein->GetStringSelection().ToStdString();
    CBioseq_Handle bsh = m_bioseqs[m_page];
    m_BioseqLabel->SetLabel(wxString(edit::GetTextObjectDescription(*bsh.GetBioseqCore(), m_TopSeqEntry.GetScope())));
    auto items = m_name_to_cds.find(name);
    if (items == m_name_to_cds.end())
    {
        m_ListBox->Clear();
        return;
    }
    auto items2 = items->second.find(bsh);
    if (items2 == items->second.end())
    {
        m_ListBox->Clear();
        return;
    }
    m_ListBox->Set(items2->second);
    if (m_page == 0)
        m_PageLeft->Disable();
    else
        m_PageLeft->Enable();
    if (m_page + 1 >= m_bioseqs.size())
        m_PageRight->Disable();
    else
        m_PageRight->Enable();
}

void CFixForTransSplicing::OnPageLeft( wxCommandEvent& event )
{
    if (m_page > 0)
    {
        m_page--;
        PopulateListBox();
    }   
}

void CFixForTransSplicing::OnPageRight( wxCommandEvent& event )
{
    if (m_page + 1 < m_bioseqs.size())
    {
        m_page++;
        PopulateListBox();
    }    
}


/*!
 * Should we show tooltips?
 */

bool CFixForTransSplicing::ShowToolTips()
{
    return true;
}

/*!
 * Get bitmap resources
 */

wxBitmap CFixForTransSplicing::GetBitmapResource( const wxString& name )
{
    // Bitmap retrieval
    wxUnusedVar(name);
    return wxNullBitmap;
}

/*!
 * Get icon resources
 */

wxIcon CFixForTransSplicing::GetIconResource( const wxString& name )
{
    // Icon retrieval
    wxUnusedVar(name);
    return wxNullIcon;
}

CRef<CCmdComposite> CFixForTransSplicing::GetCommand()
{
    wxBusyCursor wait;
    CRef<CCmdComposite> cmd( new CCmdComposite("Fix For Trans-splicing") );
    CScope &scope = m_TopSeqEntry.GetScope();
    string msg;
    bool modified = false;
    string name = m_Protein->GetStringSelection().ToStdString();
    if (m_TopSeqEntry && !name.empty())
    {
        bool create_general_only = objects::edit::IsGeneralIdProtPresent(m_TopSeqEntry);
        for (CBioseq_CI bi(m_TopSeqEntry, CSeq_inst::eMol_na); bi; ++bi)
        {
            int offset = 1;
            vector<CSeq_feat_Handle> feats, genes;
            set<CSeq_feat_Handle> unique_feats, unique_genes;
            bool problem(false);
            for (CFeat_CI cds(*bi, CSeqFeatData::eSubtype_cdregion); cds; ++cds)
            {
                if (cds->GetOriginalFeature().IsSetExcept_text() && NStr::Find(cds->GetOriginalFeature().GetExcept_text(), "RNA editing") != string::npos)
                {
                    continue;
                }
                if (cds->IsSetProduct())
                {
                    CBioseq_Handle prot_bsh = scope.GetBioseqHandle(cds->GetProduct());
                    if (prot_bsh)
                    {
                        CFeat_CI prot_feat_ci(prot_bsh, CSeqFeatData::e_Prot);
                        if (prot_feat_ci)
                        {
                            FOR_EACH_NAME_ON_PROTREF(name_it, prot_feat_ci->GetSeq_feat()->GetData().GetProt())
                            {
                                if (*name_it == name)
                                {
                                    CSeq_feat_Handle fh = cds->GetSeq_feat_Handle();
                                    CSeq_feat_Handle gene = feature::GetBestGeneForCds(*cds).GetSeq_feat_Handle();
                                    if (!gene)
                                    {
                                        msg += "Unable to find gene for " + edit::GetTextObjectDescription(*cds->GetSeq_feat(), scope) + "\n";
                                        problem = true;
                                        break;
                                    }
                                    if (unique_feats.find(fh) != unique_feats.end() || unique_genes.find(gene) != unique_genes.end())
                                    {
                                        msg += "CDS/Gene pairing not unique for: " + edit::GetTextObjectDescription(*gene.GetOriginalSeq_feat(), scope) + "\n";
                                        problem = true;
                                        break;
                                    }
                                    feats.push_back(fh);
                                    genes.push_back(gene);
                                    unique_feats.insert(fh);
                                    unique_genes.insert(gene);
                                    break;
                                }
                            }
                        }
                    }
                }
            }
            if (problem)
                continue;
            if (feats.size() != 3 && feats.size() != 5)
            {
                stringstream str;
                str << edit::GetTextObjectDescription(*bi->GetBioseqCore(), scope) << " has " << feats.size() << " CDS, skipping";
                msg += str.str() + "\n";
                continue;
            }
    
            if (feats.size() == 3)
            {
                vector<CSeq_feat_Handle> feats1, genes1, feats2, genes2;
                feats1.push_back(feats[0]);
                feats1.push_back(feats[1]);
                CombineFeatures(feats1, cmd, scope, offset, CSeq_loc::fMerge_All, false, create_general_only);
                feats2.push_back(feats[0]);
                feats2.push_back(feats[2]);
                CombineFeatures(feats2, cmd, scope, offset, CSeq_loc::fMerge_All, true, create_general_only);
                genes1.push_back(genes[0]);
                genes1.push_back(genes[1]);
                CombineFeatures(genes1, cmd, scope, offset, CSeq_loc::fMerge_All, false, create_general_only);
                genes2.push_back(genes[0]);
                genes2.push_back(genes[2]);
                CombineFeatures(genes2, cmd, scope, offset, CSeq_loc::fMerge_All, true, create_general_only);
                
            }
            else if (feats.size() == 5)
            {
                vector<CSeq_feat_Handle> feats1, genes1, feats2, genes2;
                feats1.push_back(feats[0]);
                feats1.push_back(feats[1]);
                feats1.push_back(feats[2]);
                CombineFeatures(feats1, cmd, scope, offset, CSeq_loc::fMerge_All, false, create_general_only);
                feats2.push_back(feats[0]);
                feats2.push_back(feats[3]);
                feats2.push_back(feats[4]);
                CombineFeatures(feats2, cmd, scope, offset, CSeq_loc::fMerge_All, true, create_general_only);
                genes1.push_back(genes[0]);
                genes1.push_back(genes[1]);
                genes1.push_back(genes[2]);
                CombineFeatures(genes1, cmd, scope, offset, CSeq_loc::fMerge_SingleRange, false, create_general_only);            
                genes2.push_back(genes[0]);
                genes2.push_back(genes[3]);
                genes2.push_back(genes[4]);
                CombineFeatures(genes2, cmd, scope, offset, CSeq_loc::fMerge_SingleRange, true, create_general_only);
            }
            modified = true;
        }
    }

    if (!modified)
        cmd.Reset();
    if (!msg.empty())
    {
        CGenericReportDlg* report = new CGenericReportDlg(GetParent());
        report->SetTitle(wxT("Unable to fix automatically"));
        report->SetText(wxString(msg));
        report->Show(true);
    }
    return cmd;
}

void CFixForTransSplicing::CombineFeatures(const  vector<CSeq_feat_Handle> &feats, CRef<CCmdComposite> cmd, CScope &scope, int &offset, CSeq_loc::TOpFlags merge_flag, 
                                           bool second_pass, bool create_general_only)
{
    CSeq_feat_Handle feat1;
    CRef<CSeq_feat> new_feat(new CSeq_feat);
    CRef<CSeq_loc> new_loc(new CSeq_loc);
    CRef<CSeq_loc> add_loc;
    CRef<CSeq_id> new_product_id(NULL);
    bool partial_start(false);
    bool partial_stop(false);
    for (size_t i = 0; i < feats.size(); i++)
    {
        CSeq_feat_Handle fh = feats[i];
        if (!fh)
            continue;      
        if (i == 0)
        {             
            new_loc->Assign(fh.GetLocation());
            partial_start = new_loc->IsPartialStart(eExtreme_Positional);
            partial_stop = new_loc->IsPartialStop(eExtreme_Positional);
            new_loc->SetPartialStart(false,eExtreme_Positional);
            new_loc->SetPartialStop(false,eExtreme_Positional);         
        }
        else 
        {
            CRef<CSeq_loc> tmp_loc(new CSeq_loc);
            tmp_loc->Assign(fh.GetLocation());          
            partial_stop = tmp_loc->IsPartialStop(eExtreme_Positional);
            tmp_loc->SetPartialStart(false,eExtreme_Positional);
            tmp_loc->SetPartialStop(false,eExtreme_Positional);
            if (!add_loc)
                add_loc = tmp_loc;
            else 
                add_loc->Assign(*(sequence::Seq_loc_Add(*add_loc, *tmp_loc, merge_flag, &scope))); // |CSeq_loc::fSort
        }
        if (i == 1)
        {
            feat1 = fh;        
            new_feat->Assign(*fh.GetOriginalSeq_feat());      
            if (fh.IsSetProduct()) 
            {
                new_product_id.Reset(new CSeq_id());
                new_product_id->Assign(*(fh.GetProduct().GetId()));
            }
        }
        else if (i != 0 || second_pass)
        {
            cmd->AddCommand(*CRef<CCmdDelSeq_feat>(new CCmdDelSeq_feat(fh)));
            if (fh.IsSetProduct()) 
            {
                CBioseq_Handle product = scope.GetBioseqHandle(fh.GetProduct());
                if (product) 
                {
                    cmd->AddCommand(*CRef<CCmdDelBioseqInst>(new CCmdDelBioseqInst(product)));
                }
            }
        }
    }
    if (!feat1)
        return;
    new_loc->Assign(*(sequence::Seq_loc_Add(*new_loc, *add_loc, CSeq_loc::fMerge_All, &scope)));
    new_loc->SetPartialStart(partial_start,eExtreme_Positional);
    new_loc->SetPartialStop(partial_stop,eExtreme_Positional);
    new_feat->SetLocation(*new_loc);                
    new_feat->SetPartial(partial_start || partial_stop);
    new_feat->SetExcept(true);
    new_feat->SetExcept_text("trans-splicing");
    if (!new_feat->IsSetProduct() && new_product_id) 
    {
        new_feat->SetProduct().SetWhole().Assign(*new_product_id);
    }
    cmd->AddCommand(*CRef< CCmdChangeSeq_feat > (new CCmdChangeSeq_feat(feat1,*new_feat)));
    if (new_feat->IsSetProduct())
    {
        CRef<CCmdComposite> retranslate_cmd = GetRetranslateCDSCommand(scope, *new_feat, offset, create_general_only);
        cmd->AddCommand(*retranslate_cmd);
    }
}

void CFixForTransSplicing::OnListItemSelected(wxCommandEvent& event)
{
    long index = event.GetSelection();
    string name = m_Protein->GetStringSelection().ToStdString();
    
    if (name.empty() || !m_FlatFileCtrl || !m_SelectionClient || !m_SelectionClientAttached) 
        return;

    CBioseq_Handle bsh = m_bioseqs[m_page];
    if (!bsh)
        return;
    CSeq_feat_Handle fh = m_name_to_obj[name][bsh][index];
    if (!fh)
        return;
    CConstRef<CObject> obj = fh.GetOriginalSeq_feat();
    if (!obj)
        return;

    m_FlatFileCtrl->SetPosition(bsh, obj);    

    TConstScopedObjects objs;
    CScope &scope = m_TopSeqEntry.GetScope();
    objs.push_back(SConstScopedObject(obj, &scope));
    m_SelectionClient->SetSelectedObjects(objs);
}


void CFixForTransSplicing::OnListItemActivated(wxCommandEvent& event)
{
    long index = event.GetSelection();
    string name = m_Protein->GetStringSelection().ToStdString();
    if (name.empty())
        return;
    CBioseq_Handle bsh = m_bioseqs[m_page];
    if (!bsh)
        return;
    CSeq_feat_Handle fh = m_name_to_obj[name][bsh][index];
    if (!fh)
        return;
    CConstRef<CObject> obj = fh.GetOriginalSeq_feat();
    if (!obj)
        return;

    CSeq_entry_Handle seh = m_bioseqs[m_page].GetSeq_entry_Handle();
    
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
            m_CmdProccessor->Execute(cmd);   
            ReloadData();
        }
    }
}

void CFixForTransSplicing::ExecuteCmd(CRef<CCmdComposite> cmd)
{
    m_CmdProccessor->Execute(cmd);
    ReloadData();
}

void CFixForTransSplicing::ReloadData()
{
    CollectProteinNames();
    m_page = 0;
    PopulateListBox();   
}

END_NCBI_SCOPE
