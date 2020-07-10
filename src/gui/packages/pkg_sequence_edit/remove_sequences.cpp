/*  $Id: remove_sequences.cpp 45097 2020-05-29 15:15:00Z asztalos $
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
#include <objects/misc/sequence_macros.hpp>

#include <objmgr/seq_entry_ci.hpp>
#include <objmgr/bioseq_ci.hpp>
#include <objmgr/feat_ci.hpp>
#include <objmgr/util/sequence.hpp>
#include <gui/objutils/util_cmds.hpp>
#include <gui/widgets/wx/message_box.hpp>

#include <gui/packages/pkg_sequence_edit/tbl_edit_dlg.hpp>
#include <gui/packages/pkg_sequence_edit/remove_sequences.hpp>
#include <gui/packages/pkg_sequence_edit/subprep_util.hpp>
#include <gui/objutils/cmd_del_bioseq.hpp>
#include <gui/objutils/cmd_change_seq_entry.hpp>
#include <gui/widgets/edit/generic_report_dlg.hpp> 

#include <objects/seq/MolInfo.hpp>

#include <wx/listctrl.h>
#include <wx/hyperlink.h>

BEGIN_NCBI_SCOPE
USING_SCOPE(objects);

IMPLEMENT_DYNAMIC_CLASS( CRemoveSequencesDlg, wxDialog )

BEGIN_EVENT_TABLE( CRemoveSequencesDlg, wxDialog )
    EVT_BUTTON( ID_REMOVE_SEQ_SELECT, CRemoveSequencesDlg::OnSelect)
    EVT_BUTTON( ID_REMOVE_SEQ_SELECT_ALL, CRemoveSequencesDlg::OnSelectAll)
    EVT_BUTTON( ID_REMOVE_SEQ_UNSELECT_ALL, CRemoveSequencesDlg::OnUnselectAll)
    EVT_BUTTON( ID_REMOVE_SEQ_LEFT,CRemoveSequencesDlg::OnButtonMoveLeft)
    EVT_BUTTON( ID_REMOVE_SEQ_RIGHT, CRemoveSequencesDlg::OnButtonMoveRight)
END_EVENT_TABLE()

CRemoveSequencesDlg::CRemoveSequencesDlg( wxWindow* parent, objects::CSeq_entry_Handle seh, wxWindowID id, const wxString& caption, const wxPoint& pos, const wxSize& size, long style)
{ 
    if (seh)
    {
        objects::CBioseq_CI b_iter(seh, objects::CSeq_inst::eMol_na);                
        for ( ; b_iter ; ++b_iter ) {
            m_Seqs.push_back(*b_iter);
        }
    }
    Init(); 
    Create(parent, id, caption, pos, size, style); 
}

void CRemoveSequencesDlg::CreateControls() 
{
    CRemoveSequencesDlg* itemPanel1 = this;

    wxBoxSizer* itemBoxSizer2 = new wxBoxSizer(wxVERTICAL);
    itemPanel1->SetSizer(itemBoxSizer2);

    wxBoxSizer* itemBoxSizer3 = new wxBoxSizer(wxHORIZONTAL);
    itemBoxSizer2->Add(itemBoxSizer3, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

    m_ListCtrl1 = new wxListCtrl( itemPanel1, wxID_ANY, wxDefaultPosition, wxSize(400, 320), wxLC_REPORT|wxLC_NO_HEADER );
    itemBoxSizer3->Add(m_ListCtrl1, 1, wxGROW|wxALL, 5);
    m_ListCtrl1->InsertColumn(0,wxEmptyString);  

    m_ListCtrl2 = new wxListCtrl( itemPanel1, wxID_ANY, wxDefaultPosition, wxSize(400, 320), wxLC_REPORT|wxLC_NO_HEADER );
    itemBoxSizer3->Add(m_ListCtrl2, 1, wxGROW|wxALL, 5);
    m_ListCtrl2->InsertColumn(0,wxEmptyString);  
    wxBoxSizer* itemBoxSizer4 = new wxBoxSizer(wxHORIZONTAL);
    itemBoxSizer2->Add(itemBoxSizer4, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

    wxButton* itemButton1 = new wxButton( itemPanel1, ID_REMOVE_SEQ_LEFT, _("<<<"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer4->Add(itemButton1, 0, wxALIGN_CENTER_VERTICAL|wxALL, 15);

    wxButton* itemButton2 = new wxButton( itemPanel1, ID_REMOVE_SEQ_RIGHT, _(">>>"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer4->Add(itemButton2, 0, wxALIGN_CENTER_VERTICAL|wxALL, 15);

    wxBoxSizer* itemBoxSizer17 = new wxBoxSizer(wxHORIZONTAL);
    itemBoxSizer2->Add(itemBoxSizer17, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 0);

    wxStaticText* itemStaticText4 = new wxStaticText( itemPanel1, wxID_STATIC, _("Seq-id"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer17->Add(itemStaticText4, 0, wxALIGN_TOP|wxALL, 5);

    m_StringConstraintPanel = new CStringConstraintPanel( itemPanel1, false, wxID_ANY, wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer17->Add(m_StringConstraintPanel, 0, wxALIGN_TOP|wxALL|wxFIXED_MINSIZE, 0);
    m_StringConstraintPanel->SetStringSelection(_("Is one of"));

    wxFlexGridSizer* itemFlexGridSizer5 = new wxFlexGridSizer(0, 2, 0, 0);
    itemBoxSizer2->Add(itemFlexGridSizer5, 0, wxALIGN_CENTER_HORIZONTAL | wxALL, 0);

    wxStaticText* itemStaticText5 = new wxStaticText( itemPanel1, wxID_STATIC, _("Select sequences longer than"), wxDefaultPosition, wxDefaultSize, 0 );
    itemFlexGridSizer5->Add(itemStaticText5, 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    m_Min = new wxTextCtrl(itemPanel1, wxID_ANY, wxEmptyString, wxDefaultPosition, wxSize(250, -1), 0);
    itemFlexGridSizer5->Add(m_Min, 0, wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL | wxTOP|wxBOTTOM, 5);

    wxStaticText* itemStaticText6 = new wxStaticText( itemPanel1, wxID_STATIC, _("Select sequences less than"), wxDefaultPosition, wxDefaultSize, 0 );
    itemFlexGridSizer5->Add(itemStaticText6, 0, wxALIGN_RIGHT | wxALIGN_CENTER_VERTICAL |wxALL, 5);

    m_Max = new wxTextCtrl( itemPanel1, wxID_ANY, wxEmptyString, wxDefaultPosition, wxSize(250, -1), 0 );
    itemFlexGridSizer5->Add(m_Max, 0, wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL | wxTOP | wxBOTTOM, 5);

    wxBoxSizer* itemBoxSizer12 = new wxBoxSizer(wxHORIZONTAL);
    itemBoxSizer2->Add(itemBoxSizer12, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 3);
    
    wxButton* itemButton13 = new wxButton(itemPanel1, ID_REMOVE_SEQ_SELECT, _("Select"), wxDefaultPosition, wxDefaultSize, 0);
    itemBoxSizer12->Add(itemButton13, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);
    
    wxButton* itemButton14 = new wxButton(itemPanel1, ID_REMOVE_SEQ_SELECT_ALL, _("Select All"), wxDefaultPosition, wxDefaultSize, 0);
    itemBoxSizer12->Add(itemButton14, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);
    
    wxButton* itemButton15 = new wxButton(itemPanel1, ID_REMOVE_SEQ_UNSELECT_ALL, _("Unselect All"), wxDefaultPosition, wxDefaultSize, 0);
    itemBoxSizer12->Add(itemButton15, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxBoxSizer* itemBoxSizer16 = new wxBoxSizer(wxHORIZONTAL);
    itemBoxSizer2->Add(itemBoxSizer16, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

    wxButton* itemButton16 = new wxButton( itemPanel1, wxID_OK, _("Accept"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer16->Add(itemButton16, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxButton* itemButton17 = new wxButton( itemPanel1, wxID_CANCEL, _("Cancel"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer16->Add(itemButton17, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxHyperlinkCtrl* itemHyperlinkCtrl = new wxHyperlinkCtrl( itemPanel1, wxID_HELP, _("Help"), wxT("https://www.ncbi.nlm.nih.gov/tools/gbench/manual9/#remove-sequences"), wxDefaultPosition, wxDefaultSize, wxHL_DEFAULT_STYLE );
    itemHyperlinkCtrl->SetForegroundColour(wxColour(192, 192, 192));
    itemBoxSizer16->Add(itemHyperlinkCtrl, 0, wxALIGN_CENTER_VERTICAL, 5);

    UpdateList();
}

void CRemoveSequencesDlg::CombineLabels(const CSeq_id &id, vector<string> &labels)
{
  if (id.IsGenbank() && id.GetGenbank().IsSetAccession())
      labels[0] = id.GetGenbank().GetAccession();
  if (id.IsGeneral() && id.GetGeneral().IsSetTag() && id.GetGeneral().GetTag().IsStr())
      labels[1] = id.GetGeneral().GetTag().GetStr();
  if (id.IsGeneral() && id.GetGeneral().IsSetTag() && id.GetGeneral().GetTag().IsId())
      labels[2] = NStr::IntToString(id.GetGeneral().GetTag().GetId());
  if (id.IsLocal() && id.GetLocal().IsStr())
      labels[3] = id.GetLocal().GetStr();
  string label;
  id.GetLabel(&label, CSeq_id::eContent);
  if (labels[0].empty() && labels[1].empty() && labels[2].empty() && labels[3].empty())
      labels.push_back(label);
}

void CRemoveSequencesDlg::CollectLabels(CSeq_entry_Handle seh, list<string> &strs)
{
    vector<string> labels(5);
    if (seh.IsSeq())
    {
        if (seh.GetSeq().IsSetId())
            for (CBioseq_Handle::TId::const_iterator it = seh.GetSeq().GetId().begin(); it != seh.GetSeq().GetId().end(); ++it)
                CombineLabels(*(it->GetSeqId()),labels);
        
    }
    else if (seh.IsSet() && seh.GetSet().IsSetClass() && seh.GetSet().GetClass() == CBioseq_set::eClass_nuc_prot
             && seh.GetSet().GetCompleteBioseq_set()->GetNucFromNucProtSet().IsSetId() )
    {
        for (CBioseq::TId::const_iterator it = seh.GetSet().GetCompleteBioseq_set()->GetNucFromNucProtSet().GetId().begin(); it != seh.GetSet().GetCompleteBioseq_set()->GetNucFromNucProtSet().GetId().end(); ++it)
            CombineLabels(**it,labels);
    }

    for (int i=0; i<labels.size(); i++)
        if (!labels[i].empty())
            strs.push_back(labels[i]);
}

string CRemoveSequencesDlg::GetLabel(CSeq_entry_Handle seh)
{
    list<string> strs;   
    CollectLabels(seh,strs);
    TSeqPos length = seh.GetSeq().GetBioseqLength();
    strs.push_back("("+NStr::IntToString(length)+")");
    string label = NStr::Join(strs," ");
    return label;  
}

void CRemoveSequencesDlg::CombineLabelsAll(const CSeq_id &id, set<string> &labels)
{
  if (id.IsGenbank() && id.GetGenbank().IsSetAccession())
      labels.insert(id.GetGenbank().GetAccession());
  if (id.IsGeneral() && id.GetGeneral().IsSetDb() && id.GetGeneral().GetDb() == "BankIt" && id.GetGeneral().IsSetTag() && id.GetGeneral().GetTag().IsStr())
  {
      labels.insert(id.GetGeneral().GetTag().GetStr());
      labels.insert("BankIt" + id.GetGeneral().GetTag().GetStr());
      string first, second;
      NStr::SplitInTwo(id.GetGeneral().GetTag().GetStr(), "/", first, second);
      labels.insert(first);
      labels.insert("BankIt" + first);
  }
  if (id.IsGeneral() && id.GetGeneral().IsSetDb() && id.GetGeneral().GetDb() == "NCBIFILE" && id.GetGeneral().IsSetTag() && id.GetGeneral().GetTag().IsStr())
  {
      labels.insert(id.GetGeneral().GetTag().GetStr());
      labels.insert("NCBIFILE" + id.GetGeneral().GetTag().GetStr());
  }
  if (id.IsGeneral() && id.GetGeneral().IsSetTag() && id.GetGeneral().GetTag().IsStr())
      labels.insert(id.GetGeneral().GetTag().GetStr());
  if (id.IsGeneral() && id.GetGeneral().IsSetTag() && id.GetGeneral().GetTag().IsId())
      labels.insert(NStr::IntToString(id.GetGeneral().GetTag().GetId()));
  if (id.IsLocal() && id.GetLocal().IsStr())
      labels.insert(id.GetLocal().GetStr());
  string label;
  id.GetLabel(&label, CSeq_id::eContent);
  labels.insert(label);
}

void CRemoveSequencesDlg::CollectLabelsAll(CSeq_entry_Handle seh, set<string> &strs)
{
    if (seh.IsSeq())
    {
        if (seh.GetSeq().IsSetId())
            for (CBioseq_Handle::TId::const_iterator it = seh.GetSeq().GetId().begin(); it != seh.GetSeq().GetId().end(); ++it)
                CombineLabelsAll(*(it->GetSeqId()), strs);
        
    }
    else if (seh.IsSet() && seh.GetSet().IsSetClass() && seh.GetSet().GetClass() == CBioseq_set::eClass_nuc_prot
             && seh.GetSet().GetCompleteBioseq_set()->GetNucFromNucProtSet().IsSetId() )
    {
        for (CBioseq::TId::const_iterator it = seh.GetSet().GetCompleteBioseq_set()->GetNucFromNucProtSet().GetId().begin(); it != seh.GetSet().GetCompleteBioseq_set()->GetNucFromNucProtSet().GetId().end(); ++it)
            CombineLabelsAll(**it,strs);
    }
}

bool CRemoveSequencesDlg::MatchConstraint(CRef<edit::CStringConstraint> constraint, CSeq_entry_Handle seh)
{
    set<string> strs;   
    CollectLabelsAll(seh,strs);
    vector<string> vec(strs.begin(), strs.end());
    return constraint->DoesListMatch(vec); 
}


void CRemoveSequencesDlg::UpdateList() 
{
    m_ListCtrl1->DeleteAllItems();
    m_ListCtrl2->DeleteAllItems();
    for (set<int>::const_iterator k = m_Selected.begin(); k != m_Selected.end(); ++k)
    {
        int i = *k;
        CSeq_entry_Handle seh = GetSeqEntry(i);
        string label = GetLabel(seh);          
        long item = m_ListCtrl2->GetItemCount();
        item = m_ListCtrl2->InsertItem(item,ToWxString(label));
        m_ListCtrl2->SetItemData(item,i);
    }
    m_Available.clear();
    size_t set_size = GetSetSize();
    for (unsigned int i=0; i<set_size; ++i)
        if (m_Selected.find(i) == m_Selected.end())
        {          
            m_Available.push_back(i);
            CSeq_entry_Handle seh = GetSeqEntry(i);
            string label = GetLabel(seh);          
            long item = m_ListCtrl1->GetItemCount();
            item = m_ListCtrl1->InsertItem(item,ToWxString(label));
            m_ListCtrl1->SetItemData(item,i);
        }
    m_ListCtrl1->SetColumnWidth(0, wxLIST_AUTOSIZE);
    m_ListCtrl2->SetColumnWidth(0, wxLIST_AUTOSIZE);
}

void CRemoveSequencesDlg::OnButtonMoveRight( wxCommandEvent& event )
{   
    long item = -1;
    for ( ;; )
    {
        item = m_ListCtrl1->GetNextItem(item, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);
        if ( item == -1 )  break;
        int i = static_cast<int>(m_ListCtrl1->GetItemData(item)); 
        m_Selected.insert(i);
    }
    UpdateList();
}

void CRemoveSequencesDlg::OnButtonMoveLeft( wxCommandEvent& event )
{
    long item = -1;
    for ( ;; )
    {
        item = m_ListCtrl2->GetNextItem(item, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);
        if ( item == -1 )  break;
        int i = static_cast<int>(m_ListCtrl2->GetItemData(item)); 
        m_Selected.erase(i);      
    }

    UpdateList();
}


void CRemoveSequencesDlg::OnSelectAll( wxCommandEvent& event )
{
    size_t set_size = GetSetSize();
    for (unsigned int i=0; i<set_size; ++i)
    {          
        m_Selected.insert(i);
    }
    UpdateList();
}

void CRemoveSequencesDlg::OnUnselectAll( wxCommandEvent& event )
{
    m_Selected.clear();
    UpdateList();  
}

void CRemoveSequencesDlg::OnSelect( wxCommandEvent& event )
{
    long min_length = -1;
    long max_length = LONG_MAX;
    bool min_num = m_Min->GetValue().ToLong(&min_length);
    bool max_num = m_Max->GetValue().ToLong(&max_length);
    CRef<edit::CStringConstraint> constraint(m_StringConstraintPanel->GetStringConstraint());

    if ( !min_num && !max_num && !constraint)
        return;  
    
    for(vector<int>::iterator j = m_Available.begin(); j != m_Available.end(); ++j)
    {
        int i = *j;
        CSeq_entry_Handle seh = GetSeqEntry(i);

        TSignedSeqPos length = -1;
        if (seh.IsSeq())
            length = seh.GetSeq().GetBioseqLength();
        else if (seh.IsSet() && seh.GetSet().IsSetClass() && seh.GetSet().GetClass() == CBioseq_set::eClass_nuc_prot
                 && seh.GetSet().GetCompleteBioseq_set()->GetNucFromNucProtSet().IsSetLength() )
        {
            length = seh.GetSet().GetCompleteBioseq_set()->GetNucFromNucProtSet().GetLength();
        }
        if (length == -1)
            continue;

        if (constraint)
        {
            if (MatchConstraint(constraint,seh))
                m_Selected.insert(i);
        }
        else if (min_num || max_num)
        {
            if (length > min_length && length < max_length)
                m_Selected.insert(i);
        }
    }

    UpdateList();  
}

void CRemoveSequencesDlg::Init() 
{
    m_ListCtrl1 = NULL;
    m_ListCtrl2 = NULL;
}


bool CRemoveSequencesDlg::Create( wxWindow* parent, wxWindowID id, const wxString& caption, const wxPoint& pos, const wxSize& size, long style)
{
    wxDialog::Create( parent, id, caption, pos, size, style );
    CreateControls();
    if (GetSizer())
    {
        GetSizer()->SetSizeHints(this);
    }
    Centre();
    return true;
}

CRef<CCmdComposite> CRemoveSequencesDlg::GetCommand()
{
    CRef<CCmdComposite> cmd( new CCmdComposite("Remove Sequences") );
    string msg;
    try {
        for (set<int>::iterator j = m_Selected.begin(); j != m_Selected.end(); ++j)
        {
            CBioseq_Handle bsh = m_Seqs[*j];
            cmd->AddCommand(*GetDeleteSequenceCommand(bsh));
            CSeq_id_Handle best_idh = sequence::GetId(bsh, sequence::eGetId_Best);
            string name;
            best_idh.GetSeqId()->GetLabel(&name, CSeq_id::eContent);
            msg += name + "\n";
        }
    }
    catch (const CException& e) {
        NcbiMessageBox(e.GetMsg());
        return CRef<CCmdComposite>();
    }

    if (!msg.empty())
    {
        CGenericReportDlg* report = new CGenericReportDlg(GetParent());
        report->SetTitle(wxT("Bioseqs Removed"));
        report->SetText(wxString(msg));
        report->Show(true);
    }

    return cmd;
}


void CRemoveSequencesDlg::ChooseSequence(CBioseq_Handle bsh)
{
    for (size_t i = 0; i < m_Seqs.size(); i++) {
        if (m_Seqs[i] == bsh) {
            if (m_Selected.find(i) == m_Selected.end()) {
                m_Selected.insert(i);
            }
            break;
        }
    }
    UpdateList();
}


CRef<CCmdComposite> CRemoveProteins::AndRenormalizeNucProtSets(objects::CSeq_entry_Handle seh)
{
    CRef<CCmdComposite> cmd( new CCmdComposite("Remove Proteins and renormalize nuc-prot sets") );
    if (seh)
    {
        objects::CBioseq_CI b_iter(seh, objects::CSeq_inst::eMol_aa);                
        for ( ; b_iter ; ++b_iter ) {
            CBioseq_Handle bsh = *b_iter;
            cmd->AddCommand(*CRef<CCmdDelBioseqInst>(new CCmdDelBioseqInst(bsh)));
        }
    }
    return cmd;
}

CRef<CCmdComposite> CRemoveProteins::JustRemoveProteins(objects::CSeq_entry_Handle seh)
{
    CRef<CCmdComposite> cmd( new CCmdComposite("Just Remove Proteins") );
    if (seh)
    {
        objects::CBioseq_CI b_iter(seh, objects::CSeq_inst::eMol_aa);                
        for ( ; b_iter ; ++b_iter ) 
        {
            CBioseq_Handle bsh = *b_iter;
            cmd->AddCommand(*CRef<CCmdDelBioseq>(new CCmdDelBioseq(bsh)));
        }
    }   
    return cmd;
}

CRef<CCmdComposite> CRemoveProteins::OrphanedProteins(objects::CSeq_entry_Handle seh)
{
    CRef<CCmdComposite> cmd( new CCmdComposite("Remove Orphaned Proteins") );
    if (seh)
    {
        set<CBioseq_Handle> proteins;
        for (CFeat_CI fi(seh, CSeqFeatData::eSubtype_cdregion); fi; ++fi)
        {
            if (!fi->IsSetProduct())
                continue;
            CBioseq_Handle prot_bsh = fi->GetScope().GetBioseqHandle(fi->GetProduct());
            if (prot_bsh.IsProtein())
            {
                proteins.insert(prot_bsh);
            }
        }
        objects::CBioseq_CI b_iter(seh, objects::CSeq_inst::eMol_aa);                
        for ( ; b_iter ; ++b_iter ) 
        {
            CBioseq_Handle bsh = *b_iter;
            if (proteins.find(bsh) == proteins.end())
            {
                cmd->AddCommand(*CRef<CCmdDelBioseq>(new CCmdDelBioseq(bsh)));
            }
        }       
    }
    return cmd;
}


END_NCBI_SCOPE
