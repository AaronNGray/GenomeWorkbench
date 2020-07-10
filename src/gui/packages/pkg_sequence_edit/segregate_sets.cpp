  /*  $Id: segregate_sets.cpp 45101 2020-05-29 20:53:24Z asztalos $
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
#include <objmgr/feat_ci.hpp>
#include <objmgr/seqdesc_ci.hpp>
#include <objects/misc/sequence_macros.hpp>
#include <objects/seq/seq_id_handle.hpp>
#include <objects/misc/sequence_macros.hpp>
#include <gui/objutils/cmd_change_bioseq_set.hpp>
#include <gui/objutils/utils.hpp>
#include <gui/packages/pkg_sequence_edit/miscedit_util.hpp>
#include <gui/packages/pkg_sequence_edit/editing_actions.hpp>
#include <gui/packages/pkg_sequence_edit/editing_action_constraint.hpp>
#include <gui/widgets/data/report_dialog.hpp>
#include <gui/packages/pkg_sequence_edit/constraint_panel.hpp>
#include <gui/packages/pkg_sequence_edit/segregate_sets.hpp>

#include <wx/button.h>
#include <wx/statbox.h>
#include <wx/stattext.h>

////@begin XPM images
////@end XPM images



BEGIN_NCBI_SCOPE



IMPLEMENT_DYNAMIC_CLASS( CSegregateSets, wxFrame )


/*!
 * CSegregateSets event table definition
 */

BEGIN_EVENT_TABLE( CSegregateSets, wxFrame )

EVT_BUTTON( wxID_CANCEL, CSegregateSets::OnCancel)
EVT_BUTTON( wxID_OK, CSegregateSets::OnAccept)

END_EVENT_TABLE()


/*!
 * CSegregateSets constructors
 */

CSegregateSets::CSegregateSets()
{
    Init();
}

CSegregateSets::CSegregateSets( wxWindow* parent, objects::CSeq_entry_Handle seh, ICommandProccessor* cmdProcessor,
                          wxWindowID id, const wxString& caption, const wxPoint& pos, const wxSize& size, long style )
{
    this->m_TopSeqEntry = seh;
    m_CmdProcessor = cmdProcessor;
    m_SetSeq.clear();
    ReadBioseq(*(m_TopSeqEntry.GetCompleteSeq_entry()));
    Init();
    Create(parent, id, caption, pos, size, style);
    NEditingStats::ReportUsage(caption);
}


/*!
 * CVectorTrimDlg creator
 */

bool CSegregateSets::Create( wxWindow* parent, wxWindowID id, const wxString& caption, const wxPoint& pos, const wxSize& size, long style )
{
////@begin CSegregateSets creation
    SetExtraStyle(wxWS_EX_BLOCK_EVENTS);
    wxFrame::Create( parent, id, caption, pos, size, style );

    CreateControls();
    if (GetSizer())
    {
        GetSizer()->SetSizeHints(this);
    }
    Centre();
////@end CSegregateSets creation

    return true;
}


/*!
 * CSegregateSets destructor
 */

CSegregateSets::~CSegregateSets()
{
////@begin CSegregateSets destruction
////@end CSegregateSets destruction
}


/*!
 * Member initialisation
 */

void CSegregateSets::Init()
{
}


/*!
 * Control creation for CSegregateSets
 */

void CSegregateSets::CreateControls()
{   
    CSegregateSets *frame = this;
    wxBoxSizer* itemBoxSizer1 = new wxBoxSizer(wxHORIZONTAL);
    frame->SetSizer(itemBoxSizer1);

    wxPanel* itemDialog1 = new wxPanel(this);
    itemBoxSizer1->Add(itemDialog1, 1, wxGROW|wxALL, 0);

    wxBoxSizer* itemBoxSizer2 = new wxBoxSizer(wxVERTICAL);
    itemDialog1->SetSizer(itemBoxSizer2);
 
    m_Notebook = new wxNotebook(itemDialog1, ID_CSEGREGATE_NOTEBOOK, wxDefaultPosition, wxDefaultSize);
    itemBoxSizer2->Add(m_Notebook, 1, wxGROW|wxALL, 5);

    CIDSubpanel *panel1 = new CIDSubpanel(m_Notebook, ID_CSEGREGATE_IDTAB, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL);
    m_Notebook->AddPage(panel1,_("ID"));

    CTextSubpanel *panel2 = new CTextSubpanel(m_Notebook, ID_CSEGREGATE_TEXTTAB, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL);
    m_Notebook->AddPage(panel2,_("Text"));
    
    CLengthSubpanel *panel3 = new CLengthSubpanel(m_Notebook, ID_CSEGREGATE_NUMSLENTAB, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL);
    m_Notebook->AddPage(panel3,_("Num Seq Length"));
 
    CNumSubpanel *panel4 = new CNumSubpanel(m_Notebook, ID_CSEGREGATE_NUMSETSTAB, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL);
    m_Notebook->AddPage(panel4,_("Num Sets"));
   
    CFileSubpanel *panel5 = new CFileSubpanel(m_Notebook, ID_CSEGREGATE_FNAMETAB, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL);
    m_Notebook->AddPage(panel5,_("File Name"));   

    CFieldSubpanel *panel6 = new CFieldSubpanel(m_Notebook, ID_CSEGREGATE_FIELDTAB, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL);
    m_Notebook->AddPage(panel6,_("Field"));   

    CFeatureTypeSubpanel *panel7 = new CFeatureTypeSubpanel(m_Notebook, ID_CSEGREGATE_FEATTYPETAB, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL);
    m_Notebook->AddPage(panel7,_("Feature Type"));
   
    CDescTypeSubpanel *panel8 = new CDescTypeSubpanel(m_Notebook, ID_CSEGREGATE_DESCTYPETAB, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL);
    m_Notebook->AddPage(panel8,_("Desc Type"));
      
    CMolInfoSubpanel *panel9 = new CMolInfoSubpanel(m_Notebook, ID_CSEGREGATE_MOLINFOTAB, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL);
    m_Notebook->AddPage(panel9,_("MolInfo"));
      
    CStructCommentSubpanel *panel10 = new CStructCommentSubpanel(m_Notebook, ID_CSEGREGATE_STRCOMMENTTAB, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL);
    m_Notebook->AddPage(panel10,_("Struct Comment"));
           
    m_SetClassPanel = new CSetClassPanel(itemDialog1);
    itemBoxSizer2->Add(m_SetClassPanel, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);
    if (!m_SetSeq.empty() && m_SetSeq.front().parent.IsSetClass())
        m_SetClassPanel->SetClass(m_SetSeq.front().parent.GetClass());
    else
        m_SetClassPanel->SetClass(CBioseq_set::eClass_genbank);

    wxBoxSizer* itemBoxSizer12 = new wxBoxSizer(wxHORIZONTAL);
    itemBoxSizer2->Add(itemBoxSizer12, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

    wxButton* itemButton13 = new wxButton( itemDialog1, wxID_OK, _("Accept"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer12->Add(itemButton13, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxButton* itemButton14 = new wxButton( itemDialog1, wxID_CANCEL, _("Cancel"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer12->Add(itemButton14, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    m_LeaveUp = new wxCheckBox( itemDialog1, wxID_ANY, _("Leave Dialog Up"), wxDefaultPosition, wxDefaultSize, 0 );
    m_LeaveUp->SetValue(false);
    itemBoxSizer12->Add(m_LeaveUp, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);
}


void CSegregateSetsBase::ReadBioseq(const CSeq_entry& se)
{
    if (se.IsSet()) 
    {
        FOR_EACH_SEQENTRY_ON_SEQSET(it, se.GetSet())
        {
            ReadBioseq(**it);
        }
    }
    if (se.IsSeq() && se.GetSeq().IsNa())
    {
        CBioseq_Handle bsh = m_TopSeqEntry.GetBioseqHandle(*se.GetSeq().GetFirstId());
        if (bsh)
        {
            CBioseq_set_Handle bssh = bsh.GetParentBioseq_set();
            CSeq_entry_Handle seh = bsh.GetSeq_entry_Handle();
            if (bssh && bssh.CanGetClass() && bssh.GetClass() ==  CBioseq_set::eClass_nuc_prot && bssh.GetParentBioseq_set())  // if it is a nuc-prot set go a level higher 
            {
                seh =  bssh.GetParentEntry();
                bssh = bssh.GetParentBioseq_set();  
            }
        
            if (bssh)
            {
                SSetSeqRecord s;
                s.grandparent = bssh;
                s.parent = bssh;
                s.entry = seh;
                CBioseq_set_Handle grandparent = bssh.GetParentBioseq_set();
                if (grandparent)
                {
                    s.grandparent = grandparent;
                }
                m_SetSeq.push_back(s); 
            }
        }
    }            
}

/*!
 * Should we show tooltips?
 */

bool CSegregateSets::ShowToolTips()
{
    return true;
}

/*!
 * Get bitmap resources
 */

wxBitmap CSegregateSets::GetBitmapResource( const wxString& name )
{
    // Bitmap retrieval
    wxUnusedVar(name);
    return wxNullBitmap;
}

/*!
 * Get icon resources
 */

wxIcon CSegregateSets::GetIconResource( const wxString& name )
{
    // Icon retrieval
    wxUnusedVar(name);
    return wxNullIcon;
}

void CSegregateSets::OnCancel( wxCommandEvent& event )
{
    Close();
}

void CSegregateSets::OnAccept( wxCommandEvent& event )
{
    CRef<CCmdComposite> cmd = GetCommand();
    if (cmd)  
        m_CmdProcessor->Execute(cmd);  
    if (m_LeaveUp->GetValue())
    {
        m_SetSeq.clear();
        if (m_TopSeqEntry)
            ReadBioseq(*(m_TopSeqEntry.GetCompleteSeq_entry()));
        for (size_t i = 0; i < m_Notebook->GetPageCount(); ++i) 
        {   
            CRBSubpanel* panel = dynamic_cast<CRBSubpanel*>(m_Notebook->GetPage(i));
            if (panel)
            {
                panel->GetSubsets().clear();
                panel->SetClusters();
                panel->UpdateList();
            }
        }
        Refresh();
    }
    else
        Close();
}


CRef<CCmdComposite> CSegregateSets::GetCommand()
{
    CRef<CCmdComposite> cmd;
    vector< vector<int> > &subsets = GetSubsets();
    if (m_TopSeqEntry && !subsets.empty() && !subsets.front().empty())
    {
        cmd.Reset( new CCmdComposite("Segregate Sets") );
        int i = subsets.front().front();
        CBioseq_set_Handle grandparent = m_SetSeq[i].grandparent;
        CBioseq_set_Handle parent = m_SetSeq[i].parent;
        CRef<objects::CBioseq_set> changed_set(new objects::CBioseq_set());
        bool created = false;
        if (grandparent != parent)
        {
            changed_set->Assign(*grandparent.GetCompleteBioseq_set());
        }
        else
        {
            CRef<CSeq_entry> new_se(new CSeq_entry());
            CRef<CBioseq_set> bss(new CBioseq_set());
            bss->Assign(*parent.GetCompleteBioseq_set());
            new_se->SetSet(*bss);
            changed_set->SetSeq_set().push_back(new_se);
            changed_set->SetClass(parent.GetClass());   
            created = true;
        }
        MoveDescrToEntries(*changed_set);     
        for (unsigned int s = 0; s < subsets.size(); s++)
        {    
            PopulateSet(*changed_set, subsets[s], m_SetClassPanel->GetClass());
        }
        if (changed_set->GetClass() == m_SetClassPanel->GetClass())
        {
            changed_set->SetClass(CBioseq_set::eClass_genbank);            
        }
        if (created && changed_set->GetClass() == CBioseq_set::eClass_genbank)
        {
            AddUserObject(*changed_set);
        }
        CCmdChangeBioseqSet *set_cmd = new CCmdChangeBioseqSet(grandparent, *changed_set);
        cmd->AddCommand(*set_cmd);
    }
    return cmd;
}

void CSegregateSetsBase::AddUserObject(CBioseq_set& se)
{
    CRef<CSeqdesc> user_object( new CSeqdesc );
    CSeqdesc_Base::TUser& user = user_object->SetUser();
    user.SetType().SetStr("GbenchModification");  
    CRef<CUser_field> method = user.SetFieldRef("method");
    method->SetValue("SegregateSets");
    se.SetDescr().Set().push_back( user_object );
}

void CSegregateSetsBase::MoveDescrToEntries(CBioseq_set& se)
{   
    if (se.IsSetClass() && se.GetClass() == CBioseq_set::eClass_nuc_prot)
        return;
    bool user_object_present = false;
    EDIT_EACH_SEQENTRY_ON_SEQSET(it, se)
    {
        bool title_present = false;
        if ((*it)->IsSetDescr())
        {
            for (CSeq_descr::Tdata::const_iterator desc = (*it)->GetDescr().Get().begin(); desc != (*it)->GetDescr().Get().end(); ++desc)
            {
                if ((*desc)->IsTitle() && !(*desc)->GetTitle().empty())
                {
                    title_present = true;
                    break;
                }
            }
        }
        if (se.IsSetDescr())
        {
            for (CSeq_descr::Tdata::const_iterator desc = se.GetDescr().Get().begin(); desc != se.GetDescr().Get().end(); ++desc)
            {
                if (title_present && (*desc)->IsTitle())
                    continue;
                if ((*desc)->IsUser() && (*desc)->GetUser().IsSetType() && (*desc)->GetUser().GetType().IsStr() && (*desc)->GetUser().GetType().GetStr() == "GbenchModification")
                {
                    user_object_present = true;
                    continue;
                }
                (*it)->SetDescr().Set().push_back(*desc);            
            }
        }
        
        if ((*it)->IsSet())
        {
            MoveDescrToEntries((*it)->SetSet());
        }
    }       

    se.ResetDescr();
    if (user_object_present)
    {
        AddUserObject(se);
    }
}

void CSegregateSetsBase::PopulateSet(CBioseq_set& changed_set, const vector<int> &subset, CBioseq_set::EClass top_class) 
{
    if (changed_set.CanGetSeq_set())
    {
        CRef<objects::CBioseq_set> new_set(new objects::CBioseq_set());
        new_set->SetClass(top_class);       

        for ( unsigned int j = 0; j < subset.size(); j++)
        {
            int i = subset[j];
            CRef<CSeq_entry> se(new CSeq_entry());
            se->Assign(*m_SetSeq[i].entry.GetCompleteSeq_entry());                                                                   
            TakeFromSet(changed_set,se);
            new_set->SetSeq_set().push_back(se);
        }
        CRef<CSeq_entry> new_se(new CSeq_entry());
        new_se->SetSet(*new_set);       
        changed_set.SetSeq_set().push_back(new_se);
    }
}

void CSegregateSetsBase::TakeFromSet(CBioseq_set& changed_set, CRef<CSeq_entry> se)
{
    if (!changed_set.CanGetSeq_set())
        return;
    CBioseq_set::TSeq_set::iterator it = changed_set.SetSeq_set().begin(); 
    while (it != changed_set.SetSeq_set().end())
    {
        bool match = false;
        bool take = true;
        if ( (*it)->IsSeq() && (*it)->GetSeq().CanGetId() && se->IsSeq() && se->GetSeq().CanGetId() && (*it)->GetSeq().GetFirstId()->Match(*se->GetSeq().GetFirstId()) )
            match = true;
        
        if ((*it)->IsSet()) 
        {
            if (se->IsSet() && se->GetSet().CanGetClass() && se->GetSet().GetClass() == CBioseq_set::eClass_nuc_prot && se->GetSet().GetNucFromNucProtSet().CanGetId() &&
                (*it)->GetSet().CanGetClass() && (*it)->GetSet().GetClass() == CBioseq_set::eClass_nuc_prot && (*it)->GetSet().GetNucFromNucProtSet().CanGetId()   &&
                (*it)->GetSet().GetNucFromNucProtSet().GetFirstId()->Match(*se->GetSet().GetNucFromNucProtSet().GetFirstId()) )
                match = true;
            else
            {
                TakeFromSet((*it)->SetSet(),se);
                if ( (*it)->GetSet().GetSeq_set().empty() )
                {
                    match = true;
                    take = false;
                }
            }
        }
    
        if (match)
        {
            if (take)
                se->Assign(**it);
            it = changed_set.SetSeq_set().erase(it); 
        }
        else
            ++it;
    }
}

vector< vector<int> > &  CSegregateSets::GetSubsets()
{
    int sel = m_Notebook->GetSelection();
    CRBSubpanel *win = dynamic_cast<CRBSubpanel*>(m_Notebook->GetPage(sel));
    return win->GetSubsets();
}

void CSegregateSets::SetSubsets(const set<CBioseq_Handle> &selected)
{
    int sel = m_Notebook->GetSelection();
    CRBSubpanel *win = dynamic_cast<CRBSubpanel*>(m_Notebook->GetPage(sel));
    win->SetSubsets(selected);
}


/*
 * CRBSubpanel type definition
 */

IMPLEMENT_DYNAMIC_CLASS( CRBSubpanel, wxPanel )

BEGIN_EVENT_TABLE( CRBSubpanel, wxPanel )
END_EVENT_TABLE()


void CRBSubpanel::CreateControlsHeader() 
{
    CRBSubpanel* itemPanel1 = this;

    wxBoxSizer* itemBoxSizer2 = new wxBoxSizer(wxVERTICAL);
    itemPanel1->SetSizer(itemBoxSizer2);

    wxBoxSizer* itemBoxSizer3 = new wxBoxSizer(wxHORIZONTAL);
    itemBoxSizer2->Add(itemBoxSizer3, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

    m_TreeCtrl = new wxTreeCtrl(itemPanel1, wxID_ANY, wxDefaultPosition, wxSize(400,320),wxTR_DEFAULT_STYLE|wxTR_HIDE_ROOT|wxTR_MULTIPLE );
    itemBoxSizer3->Add(m_TreeCtrl, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    m_ListCtrl2 = new wxListCtrl( itemPanel1, wxID_ANY, wxDefaultPosition, wxSize(400, 320), wxLC_REPORT|wxLC_NO_HEADER );
    itemBoxSizer3->Add(m_ListCtrl2, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);
    m_ListCtrl2->InsertColumn(0,wxEmptyString,wxLIST_FORMAT_LEFT,380);  

    wxBoxSizer* itemBoxSizer4 = new wxBoxSizer(wxHORIZONTAL);
    itemBoxSizer2->Add(itemBoxSizer4, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);
 
    wxButton* itemButton1 = new wxButton(itemPanel1, ID_RBSUB_TOLEFT, _("<<<"), wxDefaultPosition, wxDefaultSize, 0);
    itemBoxSizer4->Add(itemButton1, 0, wxALIGN_CENTER_VERTICAL|wxALL, 15);
    
    wxButton* itemButton2 = new wxButton( itemPanel1, ID_RBSUB_TORIGHT, _(">>>"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer4->Add(itemButton2, 0, wxALIGN_CENTER_VERTICAL|wxALL, 15);
    
    itemButton1->Bind(wxEVT_BUTTON, &CRBSubpanel::OnButtonMoveLeft, this);
    itemButton2->Bind(wxEVT_BUTTON, &CRBSubpanel::OnButtonMoveRight, this);
}

void CRBSubpanel::CreateControlsFooter() 
{
    CRBSubpanel* itemPanel1 = this;
    wxSizer* itemBoxSizer2 = GetSizer();

    wxBoxSizer* itemBoxSizer12 = new wxBoxSizer(wxHORIZONTAL);
    itemBoxSizer2->Add(itemBoxSizer12, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);
    
    wxButton* itemButton13 = new wxButton( itemPanel1, wxID_ANY, _("Select"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer12->Add(itemButton13, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);
    
    wxButton* itemButton14 = new wxButton( itemPanel1, wxID_ANY, _("Select All"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer12->Add(itemButton14, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);
    
    wxButton* itemButton15 = new wxButton( itemPanel1, wxID_ANY, _("Unselect All"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer12->Add(itemButton15, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    itemButton13->Bind(wxEVT_BUTTON, &CRBSubpanel::OnSelect, this);
    itemButton14->Bind(wxEVT_BUTTON, &CRBSubpanel::OnSelectAll, this);
    itemButton15->Bind(wxEVT_BUTTON, &CRBSubpanel::OnUnselectAll, this);
}

CSegregateSetsBase *CRBSubpanel::GetBaseFrame()
{
    wxWindow *win = GetParent();
    while (win)
    {
        CSegregateSetsBase *base = dynamic_cast<CSegregateSetsBase*>(win);
        if (base)
            return base;
        win = win->GetParent();
    }
    return NULL;
}

void CRBSubpanel::CombineLabels(const CSeq_id &id, vector<string> &labels)
{
  if (id.IsGenbank() && id.GetGenbank().IsSetAccession())
      labels[0] = id.GetGenbank().GetAccession();
  if (id.IsGeneral() && id.GetGeneral().IsSetDb() && id.GetGeneral().GetDb() == "BankIt" && id.GetGeneral().IsSetTag() && id.GetGeneral().GetTag().IsStr())
      labels[1] = id.GetGeneral().GetTag().GetStr();
  if (id.IsGeneral() && id.GetGeneral().IsSetDb() && id.GetGeneral().GetDb() == "NCBIFILE" && id.GetGeneral().IsSetTag() && id.GetGeneral().GetTag().IsStr())
      labels[2] = id.GetGeneral().GetTag().GetStr();
  if (id.IsLocal() && id.GetLocal().IsStr())
      labels[3] = id.GetLocal().GetStr();
}

void CRBSubpanel::CollectLabels(CSeq_entry_Handle seh, list<string> &strs)
{
    vector<string> labels(4);
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

void CRBSubpanel::CombineLabelsAll(const CSeq_id &id, set<string> &labels)
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
  if (id.IsLocal() && id.GetLocal().IsStr())
      labels.insert(id.GetLocal().GetStr());
  string label;
  id.GetLabel(&label, CSeq_id::eContent);
  labels.insert(label);
}

void CRBSubpanel::CollectLabelsAll(CSeq_entry_Handle seh, set<string> &strs)
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

string CRBSubpanel::GetLabel(CSeq_entry_Handle seh)
{
    list<string> strs;   
    CollectLabels(seh,strs);
    string label = NStr::Join(strs," ");
    return label;  
}

bool CRBSubpanel::MatchConstraint(CRef<edit::CStringConstraint> constraint, CSeq_entry_Handle seh)
{
    set<string> strs;   
    CollectLabelsAll(seh,strs);
    vector<string> vec(strs.begin(), strs.end());
    return constraint->DoesListMatch(vec); 
}

void CRBSubpanel::UpdateList() // Updates visuals and m_Available based on m_Subsets and m_Clusters
{
    Freeze();
    CSegregateSetsBase *win =GetBaseFrame();
	
    m_TreeCtrl->DeleteAllItems();
    m_ListCtrl2->DeleteAllItems();
    set<int> removed;
    for (unsigned int k=0; k<m_Subsets.size(); k++)
        for (unsigned int j=0; j<m_Subsets[k].size(); j++)
        {
            int i = m_Subsets[k][j];
            CSeq_entry_Handle seh = win->GetSeqEntry(i);
            string label = GetLabel(seh);          
            long item = m_ListCtrl2->GetItemCount();
            item = m_ListCtrl2->InsertItem(item,ToWxString(label));
            m_ListCtrl2->SetItemData(item,i);
            if (k % 2 != 0)
                m_ListCtrl2->SetItemBackgroundColour(item,*wxLIGHT_GREY);  
            removed.insert(i);
        }
    m_Available.clear();
    size_t set_size = win->GetSetSize();
    for (unsigned int i=0; i<set_size; ++i)
        if (removed.find(i) == removed.end())
        {          
            m_Available.push_back(i);
        }
    map<int, pair<string,string> > avail_to_cluster_label;
    for (map<string, vector<pair<int,string> > >::iterator it = m_Clusters.begin(); it != m_Clusters.end(); ++it)
        for (vector<pair<int,string> >::iterator s = it->second.begin(); s != it->second.end(); ++s)
            avail_to_cluster_label[s->first] = pair<string,string>(it->first,s->second);

    map<wxString, wxTreeItemId> cluster_to_id;
    wxTreeItemId root = m_TreeCtrl->AddRoot(wxEmptyString);
    for (vector<int>::iterator j=m_Available.begin(); j != m_Available.end(); ++j)
    {
        map<int,pair<string,string> >::iterator i = avail_to_cluster_label.find(*j);
        if ( i == avail_to_cluster_label.end())
            continue;

        map<string, vector<pair<int,string> > >::iterator it = m_Clusters.find(i->second.first);
        wxTreeItemId parent;
        if (cluster_to_id.find(wxString(it->first)) == cluster_to_id.end())
        {
            parent = m_TreeCtrl->AppendItem(root,wxString(it->first));
            cluster_to_id[wxString(it->first)] = parent;
        }
        else
            parent = cluster_to_id[wxString(it->first)];

        m_TreeCtrl->AppendItem(parent,wxString(i->second.second),-1,-1,new ItemData(*j));
    }

    vector<wxTreeItemId> to_delete;
    wxTreeItemIdValue cookie;
    wxTreeItemId id = m_TreeCtrl->GetFirstChild( root, cookie ); 
    while( id.IsOk() )
    {	
        if (m_TreeCtrl->ItemHasChildren(id) && m_TreeCtrl->GetChildrenCount(id) == 1)
        {            
            wxTreeItemIdValue cookie2;
            wxTreeItemId item = m_TreeCtrl->GetFirstChild( id, cookie2 ); 
            if ( item.IsOk() && m_TreeCtrl->GetItemText(id) == m_TreeCtrl->GetItemText(item) )
            {
                int data = (dynamic_cast<ItemData*>(m_TreeCtrl->GetItemData(item)))->GetData(); 
                m_TreeCtrl->SetItemData(id,new ItemData(data));               
                to_delete.push_back(id);
            }
        }       
        id = m_TreeCtrl->GetNextChild( root, cookie);
    }
    
    for (size_t i=0; i<to_delete.size(); i++)
        m_TreeCtrl->DeleteChildren(to_delete[i]);
    Thaw();
    m_TreeCtrl->Refresh();
}

CRBSubpanel * CRBSubpanel::GetEventCaller(wxCommandEvent& event)
{
    wxWindow *btn = dynamic_cast<wxWindow*>(event.GetEventObject());
    CRBSubpanel *self = NULL;
    if (btn)
    {        
        self = dynamic_cast<CRBSubpanel*>(btn->GetParent());
    }
    return self;
}

void CRBSubpanel::OnButtonMoveRight( wxCommandEvent& event )
{   
   if (!GetEventCaller(event))
   {
       event.Skip();
       return;
   }
 
   vector<int> subset;
   wxArrayTreeItemIds selection;
   size_t size_sel = m_TreeCtrl->GetSelections(selection);
   for (unsigned int i=0; i<size_sel; i++)
   {
       wxTreeItemId id = selection[i];
       if (!id.IsOk())
           continue;
       if (m_TreeCtrl->ItemHasChildren(id))
       {            
           vector<int> cluster_subset;
           wxTreeItemIdValue cookie;
           wxTreeItemId item = m_TreeCtrl->GetFirstChild( id, cookie );
           while( item.IsOk() )
           {
               if (m_TreeCtrl->GetItemData(item))
               {
                   int data = (dynamic_cast<ItemData*>(m_TreeCtrl->GetItemData(item)))->GetData();
                   cluster_subset.push_back(data);                       
               }
               item = m_TreeCtrl->GetNextChild( id, cookie);
           }
           m_Subsets.push_back(cluster_subset);
       }
       else if (m_TreeCtrl->GetItemData(id))
       {
           int data = (dynamic_cast<ItemData*>(m_TreeCtrl->GetItemData(id)))->GetData();
           if (m_Subsets.empty())
               subset.push_back(data);
           else
               m_Subsets.back().push_back(data);
       }
   }  
   if (m_Subsets.empty() && !subset.empty())
       m_Subsets.push_back(subset);
   UpdateList();
}

void CRBSubpanel::OnButtonMoveLeft( wxCommandEvent& event )
{
   if (!GetEventCaller(event))
   {
       event.Skip();
       return;
   }
 
   long item = -1;
   for ( ;; )
   {
       item = m_ListCtrl2->GetNextItem(item, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);
       if ( item == -1 )  break;
       int i = static_cast<int>(m_ListCtrl2->GetItemData(item));
       for (unsigned int k=0; k<m_Subsets.size(); k++)
       {
           vector<int>::iterator it = find(m_Subsets[k].begin(), m_Subsets[k].end(), i);
           if (it != m_Subsets[k].end())
           {
               m_Subsets[k].erase(it);
               break;
           }
       }
   }
 
   UpdateList();
}

void CRBSubpanel::OnSelectAll( wxCommandEvent& event )
{
    if (!GetEventCaller(event))
    {
        event.Skip();
        return;
    }

    vector<int> subset;
    wxTreeItemId root = m_TreeCtrl->GetRootItem();
    wxTreeItemIdValue cookie;
    wxTreeItemId id = m_TreeCtrl->GetFirstChild( root, cookie );
 
    while( id.IsOk() )
    {	

        if (m_TreeCtrl->ItemHasChildren(id))
        {            
            vector<int> cluster_subset;
            wxTreeItemIdValue cookie2;
            wxTreeItemId item = m_TreeCtrl->GetFirstChild( id, cookie2 ); 
            while( item.IsOk() )
            {
                if (m_TreeCtrl->GetItemData(item))
                {
                    int data = (dynamic_cast<ItemData*>(m_TreeCtrl->GetItemData(item)))->GetData(); 
                    cluster_subset.push_back(data);                       
                }
                item = m_TreeCtrl->GetNextChild( id, cookie2);
            }
            m_Subsets.push_back(cluster_subset);
        }
        else if (m_TreeCtrl->GetItemData(id))
        {
            int data = (dynamic_cast<ItemData*>(m_TreeCtrl->GetItemData(id)))->GetData(); 
            if (m_Subsets.empty())
                subset.push_back(data);
            else
                m_Subsets.back().push_back(data);
        }        
        id = m_TreeCtrl->GetNextChild( root, cookie);
    }

    if (m_Subsets.empty() && !subset.empty())
        m_Subsets.push_back(subset);
    UpdateList();
}

void CRBSubpanel::OnUnselectAll( wxCommandEvent& event )
{
    if (!GetEventCaller(event))
    {
        event.Skip();
        return;
    }

    m_Subsets.clear();
    UpdateList();  
}

void CRBSubpanel::OnSelect( wxCommandEvent& event )
{
    if (!GetEventCaller(event))
    {
        event.Skip();
        return;
    }
    CSegregateSetsBase *win =GetBaseFrame();
    if (!win)
        return;
    OnSelectButton(win);
    UpdateList();  
}

void CRBSubpanel::Init() 
{
    m_TreeCtrl = NULL;
    m_ListCtrl2 = NULL;
}

void CRBSubpanel::CreateControls() 
{
    CreateControlsHeader();
    CreateControlsMiddle();
    CreateControlsFooter();
    SetClusters(); 
    UpdateList();
}

bool CRBSubpanel::Create( wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style)
{
    wxPanel::Create( parent, id, pos, size, style );
    CreateControls();
    if (GetSizer())
    {
        GetSizer()->SetSizeHints(this);
    }
    Centre();
    return true;
}


void CRBSubpanel::SetSubsets(const set<CBioseq_Handle> &selected) 
{
    CSegregateSetsBase *win =GetBaseFrame();
    if (!win)
        return;

    vector<int> subset;
    for(vector<int>::iterator j = m_Available.begin(); j != m_Available.end(); ++j)
    {
        int i = *j;
        CSeq_entry_Handle seh = win->GetSeqEntry(i);
        CBioseq_Handle bsh;
        if (seh.IsSeq())
        {
            bsh = seh.GetSeq();
        }
        else if (seh.IsSet() && seh.GetSet().CanGetClass() && seh.GetSet().GetClass() ==  CBioseq_set::eClass_nuc_prot)
        {
            bsh = seh.GetScope().GetBioseqHandle(seh.GetSet().GetBioseq_setCore()->GetNucFromNucProtSet());
        }
        if (!bsh)
            continue;

        if (selected.find(bsh) != selected.end())      
            subset.push_back(i);
    }
    if (!subset.empty())
        m_Subsets.push_back(subset);
    UpdateList();  
}

IMPLEMENT_DYNAMIC_CLASS( CLengthSubpanel, CRBSubpanel )

BEGIN_EVENT_TABLE( CLengthSubpanel, CRBSubpanel )
END_EVENT_TABLE()

void CLengthSubpanel::CreateControlsMiddle()
{
    CLengthSubpanel* itemPanel1 = this;

    wxSizer* itemBoxSizer2 = GetSizer();

    wxBoxSizer* itemBoxSizer3 = new wxBoxSizer(wxHORIZONTAL);
    itemBoxSizer2->Add(itemBoxSizer3, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

    wxBoxSizer* itemBoxSizer4 = new wxBoxSizer(wxVERTICAL);
    itemBoxSizer3->Add(itemBoxSizer4, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxStaticText* itemStaticText5 = new wxStaticText( itemPanel1, wxID_STATIC, _("Minimum Length"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer4->Add(itemStaticText5, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

    wxStaticText* itemStaticText6 = new wxStaticText( itemPanel1, wxID_STATIC, _("Maximum Length"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer4->Add(itemStaticText6, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

    wxBoxSizer* itemBoxSizer7 = new wxBoxSizer(wxVERTICAL);
    itemBoxSizer3->Add(itemBoxSizer7, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    m_Min = new wxTextCtrl(itemPanel1, ID_LENGTH_MINTEXT, wxEmptyString, wxDefaultPosition, wxSize(250, -1), 0);
    itemBoxSizer7->Add(m_Min, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

    m_Max = new wxTextCtrl(itemPanel1, ID_LENGTH_MAXTEXT, wxEmptyString, wxDefaultPosition, wxSize(250, -1), 0);
    itemBoxSizer7->Add(m_Max, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);
}

void CLengthSubpanel::OnSelectButton( CSegregateSetsBase *win )
{
    unsigned long min_length = 0;
    unsigned long max_length = INT_MAX;
    bool min_num = m_Min->GetValue().ToULong(&min_length);
    bool max_num = m_Max->GetValue().ToULong(&max_length);
    if ( !min_num && !max_num )
        return;  

    vector<int> subset;
    for(vector<int>::iterator j = m_Available.begin(); j != m_Available.end(); ++j)
    {
        int i = *j;
        CSeq_entry_Handle seh = win->GetSeqEntry(i);
        TSeqPos length = 0;
        if (seh.IsSeq())
            length = seh.GetSeq().GetBioseqLength();
        else if (seh.IsSet() && seh.GetSet().IsSetClass() && seh.GetSet().GetClass() == CBioseq_set::eClass_nuc_prot
                 && seh.GetSet().GetCompleteBioseq_set()->GetNucFromNucProtSet().IsSetLength() )
        {
            length = seh.GetSet().GetCompleteBioseq_set()->GetNucFromNucProtSet().GetLength();
        }
        if (length == 0)
            continue;
        if (length >= min_length && length <= max_length)
            subset.push_back(i);
    }
    if (!subset.empty())
        m_Subsets.push_back(subset);
}

void CLengthSubpanel::SetClusters()
{
    CSegregateSetsBase *win =GetBaseFrame();
    size_t set_size = win->GetSetSize();
    m_Clusters.clear();
    for (unsigned i = 0; i < set_size; i++)
    {
        CSeq_entry_Handle seh = win->GetSeqEntry(i);
        string label = GetLabel(seh);
        TSeqPos length = 0;
        if (seh.IsSeq())
        {
            length = seh.GetSeq().GetBioseqLength();
        }
        else if (seh.IsSet() && seh.GetSet().IsSetClass() && seh.GetSet().GetClass() == CBioseq_set::eClass_nuc_prot
                 && seh.GetSet().GetCompleteBioseq_set()->GetNucFromNucProtSet().IsSetId() )
        {
            length = seh.GetSet().GetCompleteBioseq_set()->GetNucFromNucProtSet().GetLength();
        }
        if (!label.empty())
        {
            m_Clusters[NStr::IntToString(length)].push_back(pair<int,string>(i,label));
        }
    }
}


IMPLEMENT_DYNAMIC_CLASS( CIDSubpanel, CRBSubpanel )

BEGIN_EVENT_TABLE( CIDSubpanel, CRBSubpanel )
END_EVENT_TABLE()

void CIDSubpanel::CreateControlsMiddle()
{
    CIDSubpanel* itemPanel1 = this;

    wxSizer* itemBoxSizer2 = GetSizer();

    m_StringConstraintPanel = new CStringConstraintPanel( itemPanel1, false, wxID_ANY, wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer2->Add(m_StringConstraintPanel, 0, wxALIGN_CENTER_HORIZONTAL|wxALL|wxFIXED_MINSIZE, 0);
    m_StringConstraintPanel->SetStringSelection(_("Is one of"));

}

void CIDSubpanel::OnSelectButton( CSegregateSetsBase *win )
{
    CRef<edit::CStringConstraint> constraint(m_StringConstraintPanel->GetStringConstraint());
    if (!constraint) return;

    vector<int> subset;
    for(vector<int>::iterator j = m_Available.begin(); j != m_Available.end(); ++j)
    {
        int i = *j;
        CSeq_entry_Handle seh = win->GetSeqEntry(i);

        if (MatchConstraint(constraint,seh))      
            subset.push_back(i);
    }
    if (!subset.empty())
        m_Subsets.push_back(subset);
}

void CIDSubpanel::SetClusters()
{
    CSegregateSetsBase *win =GetBaseFrame();
    size_t set_size = win->GetSetSize();
    m_Clusters.clear();
    for (unsigned i = 0; i < set_size; i++)
    {
        CSeq_entry_Handle seh = win->GetSeqEntry(i);
        string label = GetLabel(seh);       
        if (!label.empty())
        {
            m_Clusters[label].push_back(pair<int,string>(i,label));
        }
    }
}


IMPLEMENT_DYNAMIC_CLASS( CTextSubpanel, CRBSubpanel )

BEGIN_EVENT_TABLE( CTextSubpanel, CRBSubpanel )
EVT_UPDATE_FEATURE_LIST(wxID_ANY, CTextSubpanel::ProcessUpdateFeatEvent )
EVT_LISTBOX(wxID_ANY, CTextSubpanel::OnChoiceChanged)
EVT_CHOICE(wxID_ANY, CTextSubpanel::OnChoiceChanged)
EVT_RADIOBUTTON(wxID_ANY, CTextSubpanel::OnChoiceChanged)
EVT_CHOICEBOOK_PAGE_CHANGED(wxID_ANY, CTextSubpanel::OnPageChanged)
END_EVENT_TABLE()

void CTextSubpanel::CreateControlsMiddle()
{
    CTextSubpanel* itemPanel1 = this;

    wxSizer* itemBoxSizer2 = GetSizer();

    CSegregateSetsBase* owner = GetBaseFrame();
    CSeq_entry_Handle seh = owner->GetTopSeqEntry();
    m_Constraint = new CConstraintPanel( itemPanel1, seh);
    itemBoxSizer2->Add(m_Constraint, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);
}

void CTextSubpanel::OnChoiceChanged( wxCommandEvent& event )
{
    SetClusters();
    UpdateList();
    event.Skip();
}

void CTextSubpanel::OnPageChanged(wxBookCtrlEvent& event)
{
    SetClusters();
    UpdateList();
    event.Skip();
}


void CTextSubpanel::OnSelectButton( CSegregateSetsBase *win )
{
    CFieldNamePanel* constraint_field_panel = m_Constraint->GetFieldNamePanel();
    string constraint_field;
    string constraint_field_type = m_Constraint->GetFieldType();
    if (constraint_field_panel)
        constraint_field = constraint_field_panel->GetFieldName();
    
    if (NStr::IsBlank(constraint_field)) {
        constraint_field = constraint_field_type;
    } else {
        if (NStr::StartsWith(constraint_field_type, "RNA") && NStr::Find(constraint_field, "RNA") == NPOS ) {
            constraint_field = "RNA " + constraint_field;
        }
    }
    CRef<edit::CStringConstraint> constraint = m_Constraint->GetStringConstraint();

    if (!constraint || constraint_field.empty()) return;

    vector<int> subset;
    for(vector<int>::iterator j = m_Available.begin(); j != m_Available.end(); ++j)
    {
        int i = *j;
        CSeq_entry_Handle seh = win->GetSeqEntry(i);
        CRef<CMiscSeqTableColumn> src_col(new CMiscSeqTableColumn(constraint_field));
        vector<CRef<edit::CApplyObject> > src_objects = src_col->GetApplyObjects(seh, constraint_field, constraint);
          
        if (!src_objects.empty())       
            subset.push_back(i);
    }
    if (!subset.empty())
        m_Subsets.push_back(subset);
}

void CTextSubpanel::SetClusters()
{
    CSegregateSetsBase *win =GetBaseFrame();
    size_t set_size = win->GetSetSize();
    m_Clusters.clear();

    CFieldNamePanel* constraint_field_panel = m_Constraint->GetFieldNamePanel();
    string constraint_field;
    string constraint_field_type = m_Constraint->GetFieldType();
    if (constraint_field_panel)
        constraint_field = constraint_field_panel->GetFieldName();
    
    if (NStr::IsBlank(constraint_field)) {
        constraint_field = constraint_field_type;
    } else {
        if (NStr::StartsWith(constraint_field_type, "RNA") && NStr::Find(constraint_field, "RNA") == NPOS ) {
            constraint_field = "RNA " + constraint_field;
        }
    }

    if (constraint_field.empty()) return;
    CRef<CMiscSeqTableColumn> src_col(new CMiscSeqTableColumn(constraint_field));

    for (unsigned i = 0; i < set_size; i++)
    {
        CSeq_entry_Handle seh = win->GetSeqEntry(i);
        set<string> values;
        src_col->GetApplyObjectsValues(seh,constraint_field,values);
        string label = GetLabel(seh);
        values.erase(kEmptyStr);
        if (!label.empty() && !values.empty())
        {
            m_Clusters[NStr::Join(values, " AND ")].push_back(pair<int,string>(i,label));
        }
    }
}


void CTextSubpanel::ProcessUpdateFeatEvent( wxCommandEvent& event )
{
        UpdateChildrenFeaturePanels(this->GetSizer());
}

void CTextSubpanel::UpdateChildrenFeaturePanels( wxSizer* sizer )
{
    CSegregateSetsBase* owner = GetBaseFrame();
    if (!owner)
        return;
    m_Constraint->ListPresentFeaturesFirst(owner->GetTopSeqEntry());   
}

IMPLEMENT_DYNAMIC_CLASS( CNumSubpanel, CRBSubpanel )

BEGIN_EVENT_TABLE( CNumSubpanel, CRBSubpanel )
END_EVENT_TABLE()

void CNumSubpanel::CreateControlsMiddle()
{
    CNumSubpanel* itemPanel1 = this;

    wxSizer* itemBoxSizer2 = GetSizer();
  
    wxBoxSizer* itemBoxSizer3 = new wxBoxSizer(wxHORIZONTAL);
    itemBoxSizer2->Add(itemBoxSizer3, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

    wxBoxSizer* itemBoxSizer4 = new wxBoxSizer(wxVERTICAL);
    itemBoxSizer3->Add(itemBoxSizer4, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxStaticText* itemStaticText5 = new wxStaticText( itemPanel1, wxID_STATIC, _("Number of Sets"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer4->Add(itemStaticText5, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

    wxBoxSizer* itemBoxSizer7 = new wxBoxSizer(wxVERTICAL);
    itemBoxSizer3->Add(itemBoxSizer7, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    m_Num = new wxTextCtrl(itemPanel1, ID_NUM_TEXTCTRL, wxEmptyString, wxDefaultPosition, wxSize(250, -1), 0);
    itemBoxSizer7->Add(m_Num, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);
  
}

void CNumSubpanel::OnSelectButton( CSegregateSetsBase *win )
{
    unsigned long num = 0;
    bool num_exists = m_Num->GetValue().ToULong(&num);
    if ( !num_exists )
        return;

    long total_items = m_Available.size();
    int remainder = total_items % num;
    long items_per_subset = total_items / num;

    vector<int> subset;
    for(vector<int>::iterator j = m_Available.begin(); j != m_Available.end(); ++j)
    {
        int i = *j;    
        subset.push_back(i);  
        if (m_Subsets.size() < remainder)
        {
            if (subset.size() >= items_per_subset+1)
            {
                m_Subsets.push_back(subset);
                subset.clear();
            }
        }
        else
        {
            if (subset.size() >= items_per_subset && !subset.empty())
            {
                m_Subsets.push_back(subset);
                subset.clear();
            }
        }           
    }
}

IMPLEMENT_DYNAMIC_CLASS( CFileSubpanel, CRBSubpanel )

BEGIN_EVENT_TABLE( CFileSubpanel, CRBSubpanel )
END_EVENT_TABLE()

void CFileSubpanel::CreateControlsMiddle()
{

}

string CFileSubpanel::GetFilename(const CObject_id &tag)
{
    string name;
    if (tag.IsId())
        name = NStr::IntToString(tag.GetId());
    else if (tag.IsStr())
        name = tag.GetStr();
    int pos = NStr::Find(name, "/", NStr::eCase, NStr::eReverseSearch);
    if (pos != NPOS)
        name = name.substr(0,pos);
    return name;
}

void CFileSubpanel::OnSelectButton( CSegregateSetsBase *win )
{  
    map<string,vector<int> > tag_to_subset;
    for(vector<int>::iterator j = m_Available.begin(); j != m_Available.end(); ++j)
    {
        int i = *j;
        CSeq_entry_Handle seh = win->GetSeqEntry(i);

        if (seh.IsSeq())
        {
            if (seh.GetSeq().IsSetId())
            {
                for (vector<CSeq_id_Handle>::const_iterator idh = seh.GetSeq().GetId().begin(); idh != seh.GetSeq().GetId().end(); ++idh)
                    if (idh->GetSeqId()->IsGeneral() && idh->GetSeqId()->GetGeneral().IsSetDb() && idh->GetSeqId()->GetGeneral().GetDb() == "NCBIFILE" && idh->GetSeqId()->GetGeneral().IsSetTag() )
                    {
                        string filename = GetFilename(idh->GetSeqId()->GetGeneral().GetTag());
                        if (!filename.empty())
                            tag_to_subset[filename].push_back(i);
                    }
            }
        }
        else if (seh.IsSet() && seh.GetSet().IsSetClass() && seh.GetSet().GetClass() == CBioseq_set::eClass_nuc_prot
                 && seh.GetSet().GetCompleteBioseq_set()->GetNucFromNucProtSet().IsSetId() )
        {
            for (CBioseq::TId::const_iterator idh = seh.GetSet().GetCompleteBioseq_set()->GetNucFromNucProtSet().GetId().begin(); idh != seh.GetSet().GetCompleteBioseq_set()->GetNucFromNucProtSet().GetId().end(); ++idh)
                if ((*idh)->IsGeneral() && (*idh)->GetGeneral().IsSetDb() && (*idh)->GetGeneral().GetDb() == "NCBIFILE" && (*idh)->GetGeneral().IsSetTag())
                {
                    string filename = GetFilename((*idh)->GetGeneral().GetTag());
                    if (!filename.empty())
                        tag_to_subset[filename].push_back(i);
                }            
        }      
    }
  
    for ( map<string,vector<int> >::iterator it =  tag_to_subset.begin(); it != tag_to_subset.end(); ++it)
        if (!it->second.empty())
            m_Subsets.push_back(it->second);
}

void CFileSubpanel::SetClusters()
{
    CSegregateSetsBase *win =GetBaseFrame();
    size_t set_size = win->GetSetSize();
    m_Clusters.clear();

    for (unsigned i = 0; i < set_size; i++)
    {
        CSeq_entry_Handle seh = win->GetSeqEntry(i);
        string label = GetLabel(seh);
        string filename;
        if (seh.IsSeq())
        {
            if (seh.GetSeq().IsSetId())
            {
                for (vector<CSeq_id_Handle>::const_iterator idh = seh.GetSeq().GetId().begin(); idh != seh.GetSeq().GetId().end(); ++idh)
                    if (idh->GetSeqId()->IsGeneral() && idh->GetSeqId()->GetGeneral().IsSetDb() && idh->GetSeqId()->GetGeneral().GetDb() == "NCBIFILE" && idh->GetSeqId()->GetGeneral().IsSetTag())
                    {
                        filename = GetFilename(idh->GetSeqId()->GetGeneral().GetTag());
                    }
            }
        }
        else if (seh.IsSet() && seh.GetSet().IsSetClass() && seh.GetSet().GetClass() == CBioseq_set::eClass_nuc_prot
                 && seh.GetSet().GetCompleteBioseq_set()->GetNucFromNucProtSet().IsSetId() )
        {
            for (CBioseq::TId::const_iterator idh = seh.GetSet().GetCompleteBioseq_set()->GetNucFromNucProtSet().GetId().begin(); idh != seh.GetSet().GetCompleteBioseq_set()->GetNucFromNucProtSet().GetId().end(); ++idh)
                if ((*idh)->IsGeneral() && (*idh)->GetGeneral().IsSetDb() && (*idh)->GetGeneral().GetDb() == "NCBIFILE" && (*idh)->GetGeneral().IsSetTag())
                {
                    filename = GetFilename((*idh)->GetGeneral().GetTag());
                }            
        }
        if (!label.empty() && !filename.empty())
        {
            m_Clusters[filename].push_back(pair<int,string>(i,label));
        }
    }
}

IMPLEMENT_DYNAMIC_CLASS( CFieldSubpanel, CRBSubpanel )

BEGIN_EVENT_TABLE( CFieldSubpanel, CRBSubpanel )
EVT_UPDATE_FEATURE_LIST(wxID_ANY, CFieldSubpanel::ProcessUpdateFeatEvent )
EVT_LISTBOX(wxID_ANY, CFieldSubpanel::OnChoiceChanged)
EVT_CHOICE(wxID_ANY, CFieldSubpanel::OnChoiceChanged)
EVT_RADIOBUTTON(wxID_ANY, CFieldSubpanel::OnChoiceChanged)
END_EVENT_TABLE()

void CFieldSubpanel::CreateControlsMiddle()
{
    CFieldSubpanel* itemPanel1 = this;

    wxSizer* itemBoxSizer2 = GetSizer();

    wxPanel *container1 = new wxPanel( itemPanel1, wxID_ANY, wxDefaultPosition, wxSize(800,260), 0 );
    itemBoxSizer2->Add(container1, 0, wxALIGN_LEFT|wxALL|wxFIXED_MINSIZE, 0);

    wxBoxSizer* itemBoxSizer3 = new wxBoxSizer(wxHORIZONTAL);
    container1->SetSizer(itemBoxSizer3);

    vector<CFieldNamePanel::EFieldType> field_types;
    field_types.push_back(CFieldNamePanel::eFieldType_Taxname);
    field_types.push_back(CFieldNamePanel::eFieldType_Source);
    field_types.push_back(CFieldNamePanel::eFieldType_Misc);
    field_types.push_back(CFieldNamePanel::eFieldType_Feature);
    field_types.push_back(CFieldNamePanel::eFieldType_CDSGeneProt);
    field_types.push_back(CFieldNamePanel::eFieldType_RNA);
    field_types.push_back(CFieldNamePanel::eFieldType_MolInfo);
    field_types.push_back(CFieldNamePanel::eFieldType_Pub);
    field_types.push_back(CFieldNamePanel::eFieldType_DBLink);
    field_types.push_back(CFieldNamePanel::eFieldType_Misc);
    field_types.push_back(CFieldNamePanel::eFieldType_SeqId);
    m_FieldType = new CFieldChoicePanel( container1, field_types, true, false, wxID_ANY, wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer3->Add(m_FieldType, 0, wxALIGN_LEFT|wxALL|wxFIXED_MINSIZE, 0);   
}

void CFieldSubpanel::OnChoiceChanged( wxCommandEvent& event )
{
    SetClusters();
    UpdateList();
    event.Skip();
}

void CFieldSubpanel::OnSelectButton( CSegregateSetsBase *win )
{
    string constraint_field = m_FieldType->GetFieldName(false);
    if (constraint_field.empty()) return;

    map<string,vector<int> > value_to_subset;
    for(vector<int>::iterator j = m_Available.begin(); j != m_Available.end(); ++j)
    {
        int i = *j;
        CSeq_entry_Handle seh = win->GetSeqEntry(i);
        CRef<CMiscSeqTableColumn> src_col(new CMiscSeqTableColumn(constraint_field));
        set<string> values;
        src_col->GetApplyObjectsValues(seh,constraint_field,values);
        if (!values.empty() && !values.begin()->empty())
        {
            value_to_subset[*values.begin()].push_back(i);  // TODO what to do if more than 1 value?
        }
    }
    for ( map<string,vector<int> >::iterator it =  value_to_subset.begin(); it != value_to_subset.end(); ++it)
        if (!it->second.empty())
            m_Subsets.push_back(it->second);
}


void CFieldSubpanel::SetClusters()
{
    CSegregateSetsBase *win =GetBaseFrame();
    size_t set_size = win->GetSetSize();
    m_Clusters.clear();
    if (!m_FieldType) return;
    string constraint_field = m_FieldType->GetFieldName(false);
    if (constraint_field.empty()) return;
    CRef<CMiscSeqTableColumn> src_col(new CMiscSeqTableColumn(constraint_field));

    for (unsigned i = 0; i < set_size; i++)
    {
        CSeq_entry_Handle seh = win->GetSeqEntry(i);
        set<string> values;
        src_col->GetApplyObjectsValues(seh,constraint_field,values);
        string label = GetLabel(seh);
        values.erase(kEmptyStr);
        if (!label.empty() && !values.empty())
        {
            m_Clusters[NStr::Join(values, " AND ")].push_back(pair<int,string>(i,label));
        }
    }
}

void CFieldSubpanel::ProcessUpdateFeatEvent( wxCommandEvent& event )
{
    UpdateChildrenFeaturePanels(this->GetSizer());
}

void CFieldSubpanel::UpdateChildrenFeaturePanels( wxSizer* sizer )
{
    wxSizerItemList& slist = sizer->GetChildren();
    CSegregateSetsBase* owner =GetBaseFrame();
    if (!owner)
        return;
    int n =0;
    for (wxSizerItemList::iterator iter = slist.begin(); iter != slist.end(); ++iter, ++n) {
        if ((*iter)->IsSizer()) {
            UpdateChildrenFeaturePanels((*iter)->GetSizer());
        } else if ((*iter)->IsWindow()) {
            wxWindow* child = (*iter)->GetWindow();
            if (child) {
                CFeatureTypePanel* panel = dynamic_cast<CFeatureTypePanel*>(child);
                if (panel) {
                    panel->ListPresentFeaturesFirst(owner->GetTopSeqEntry());
                } else {
                    wxSizer* subsizer = child->GetSizer();
                    if (subsizer) {
                        UpdateChildrenFeaturePanels(subsizer);
                    } 
                }
            }
        } 
    }    
}


IMPLEMENT_DYNAMIC_CLASS( CFeatureTypeSubpanel, CRBSubpanel )

BEGIN_EVENT_TABLE( CFeatureTypeSubpanel, CRBSubpanel )
EVT_LISTBOX(wxID_ANY, CFeatureTypeSubpanel::OnChoiceChanged)
EVT_CHOICE(wxID_ANY, CFeatureTypeSubpanel::OnChoiceChanged)
EVT_RADIOBUTTON(wxID_ANY, CFeatureTypeSubpanel::OnChoiceChanged)
END_EVENT_TABLE()

void CFeatureTypeSubpanel::CreateControlsMiddle()
{
    CFeatureTypeSubpanel* itemPanel1 = this;

    wxSizer* itemBoxSizer2 = GetSizer();
    
    m_FeatureType = new CFeatureTypePanel(itemPanel1, wxID_ANY, wxDefaultPosition, wxDefaultSize, 0);
    itemBoxSizer2->Add(m_FeatureType, 0, wxALIGN_CENTER_HORIZONTAL|wxALL|wxFIXED_MINSIZE, 0);   

    CSegregateSetsBase* owner =GetBaseFrame();
    m_FeatureType->ListPresentFeaturesFirst(owner->GetTopSeqEntry());
}

void CFeatureTypeSubpanel::OnChoiceChanged( wxCommandEvent& event )
{
    SetClusters();
    UpdateList();
    event.Skip();
}

void CFeatureTypeSubpanel::OnSelectButton( CSegregateSetsBase *win )
{
    string constraint_field = m_FeatureType->GetFieldName(true);
    if (constraint_field.empty()) return;
    int itype, isubtype;
    if (!CSeqFeatData::GetFeatList()->GetTypeSubType(constraint_field, itype, isubtype))
        return;
    CSeqFeatData::ESubtype subtype = static_cast<CSeqFeatData::ESubtype>(isubtype);

    vector<int>  subset;
    for(vector<int>::iterator j = m_Available.begin(); j != m_Available.end(); ++j)
    {
        int i = *j;
        CSeq_entry_Handle seh = win->GetSeqEntry(i);

        CFeat_CI feat_iter(seh,SAnnotSelector(subtype));
        if (feat_iter)       
        {
            subset.push_back(i);  
        }
    }
    if (!subset.empty())
        m_Subsets.push_back(subset);
}


void CFeatureTypeSubpanel::SetClusters()
{
    CSegregateSetsBase *win =GetBaseFrame();
    size_t set_size = win->GetSetSize();
    m_Clusters.clear();
    if (!m_FeatureType) return;
    string constraint_field = m_FeatureType->GetFieldName(true);
    if (constraint_field.empty()) return;
    int itype, isubtype;
    if (!CSeqFeatData::GetFeatList()->GetTypeSubType(constraint_field, itype, isubtype))
        return;
    CSeqFeatData::ESubtype subtype = static_cast<CSeqFeatData::ESubtype>(isubtype);

    for (unsigned i = 0; i < set_size; i++)
    {
        CSeq_entry_Handle seh = win->GetSeqEntry(i);
        string label = GetLabel(seh);
      
        CFeat_CI feat_iter(seh,SAnnotSelector(subtype));  
        if (!label.empty() && feat_iter)
        {
            m_Clusters[constraint_field].push_back(pair<int,string>(i,label));
        }
    }
}

IMPLEMENT_DYNAMIC_CLASS( CDescTypeSubpanel, CRBSubpanel )

BEGIN_EVENT_TABLE( CDescTypeSubpanel, CRBSubpanel )
EVT_CHOICE(wxID_ANY, CDescTypeSubpanel::OnChoiceChanged)
END_EVENT_TABLE()

void CDescTypeSubpanel::CreateControlsMiddle()
{
    CDescTypeSubpanel* itemPanel1 = this;

    wxSizer* itemBoxSizer2 = GetSizer();

    wxArrayString choices;
    for (int i = CSeqdesc::e_not_set+1; i != CSeqdesc::e_MaxChoice; i++)
        choices.Add(wxString(CSeqdesc::SelectionName(static_cast<CSeqdesc::E_Choice>(i))));
    m_DescType = new wxChoice(itemPanel1, ID_DESC_CHOICE, wxDefaultPosition, wxDefaultSize, choices, 0);
    itemBoxSizer2->Add(m_DescType, 0, wxALIGN_CENTER_HORIZONTAL|wxALL|wxFIXED_MINSIZE, 0);   
    m_DescType->SetSelection(0);
                    

}

void CDescTypeSubpanel::OnChoiceChanged( wxCommandEvent& event )
{
    SetClusters();
    UpdateList();
    event.Skip();
}

void CDescTypeSubpanel::OnSelectButton( CSegregateSetsBase *win )
{
    int ichoice = m_DescType->GetSelection();
    if (ichoice == wxNOT_FOUND) return;
    CSeqdesc::E_Choice choice = static_cast<CSeqdesc::E_Choice>(ichoice+1);

    vector<int>  subset;
    for(vector<int>::iterator j = m_Available.begin(); j != m_Available.end(); ++j)
    {
        int i = *j;
        CSeq_entry_Handle seh = win->GetSeqEntry(i);

        CSeqdesc_CI desc_iter(seh, choice);
        if (desc_iter)       
        {
            subset.push_back(i);  
        }
    }
    if (!subset.empty())
        m_Subsets.push_back(subset);
}

void CDescTypeSubpanel::SetClusters()
{
    CSegregateSetsBase *win =GetBaseFrame();
    size_t set_size = win->GetSetSize();
    m_Clusters.clear();
    if (!m_DescType) return;
    int ichoice = m_DescType->GetSelection();
    if (ichoice == wxNOT_FOUND) 
        return;
    CSeqdesc::E_Choice choice = static_cast<CSeqdesc::E_Choice>(ichoice+1);

  

    for (unsigned i = 0; i < set_size; i++)
    {
        CSeq_entry_Handle seh = win->GetSeqEntry(i);
        string label = GetLabel(seh);
        string desc_label;
      
        CSeqdesc_CI desc_iter(seh, choice);
        if (desc_iter)       
        {
            desc_iter->GetLabel(&desc_label, CSeqdesc:: eContent);
        }
        if (!label.empty() && !desc_label.empty())
        {
            m_Clusters[desc_label].push_back(pair<int,string>(i,label));
        }
    }
}




IMPLEMENT_DYNAMIC_CLASS( CMolInfoSubpanel, CRBSubpanel )

BEGIN_EVENT_TABLE( CMolInfoSubpanel, CRBSubpanel )
EVT_CHECKBOX(wxID_ANY, CMolInfoSubpanel::OnChoiceChanged)
END_EVENT_TABLE()

void CMolInfoSubpanel::CreateControlsMiddle()
{
    CMolInfoSubpanel* itemPanel1 = this;

    wxSizer* itemBoxSizer2 = GetSizer();

    wxBoxSizer* itemBoxSizer3 = new wxBoxSizer(wxHORIZONTAL);
    itemBoxSizer2->Add(itemBoxSizer3, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

    wxBoxSizer* itemBoxSizer4 = new wxBoxSizer(wxVERTICAL);
    itemBoxSizer3->Add(itemBoxSizer4, 0, wxALIGN_TOP|wxALL, 5);

    m_Class = new wxCheckBox(itemPanel1, ID_MOLINFO_CHKBX1, _("Class"));
    itemBoxSizer4->Add(m_Class, 0, wxALIGN_LEFT|wxALL, 5);   

    m_Type = new wxCheckBox(itemPanel1, ID_MOLINFO_CHKBX2, _("Type"));
    itemBoxSizer4->Add(m_Type, 0, wxALIGN_LEFT|wxALL, 5);   

    m_Topology = new wxCheckBox(itemPanel1, ID_MOLINFO_CHKBX3, _("Topology"));
    itemBoxSizer4->Add(m_Topology, 0, wxALIGN_LEFT|wxALL, 5);   
}

void CMolInfoSubpanel::OnChoiceChanged( wxCommandEvent& event )
{
    SetClusters();
    UpdateList();
    event.Skip();
}

void CMolInfoSubpanel::OnSelectButton( CSegregateSetsBase *win )
{
    if (!m_Class->IsChecked() && !m_Type->IsChecked() && !m_Topology->IsChecked())
        return;


    map< pair<int, pair<int,int> >, vector<int> > class_type_topology_to_subset;
    for(vector<int>::iterator j = m_Available.begin(); j != m_Available.end(); ++j)
    {
        int i = *j;
        CSeq_entry_Handle seh = win->GetSeqEntry(i);

        int iclass = -1;
        if (m_Class->IsChecked())
        {
            iclass = CSeq_inst::eMol_not_set;
            if (seh.IsSeq() && seh.GetSeq().IsSetInst_Mol())
                iclass = seh.GetSeq().GetInst_Mol();
            else if (seh.IsSet() && seh.GetSet().IsSetClass() && seh.GetSet().GetClass() == CBioseq_set::eClass_nuc_prot
                     && seh.GetSet().GetCompleteBioseq_set()->GetNucFromNucProtSet().IsSetInst()
                     && seh.GetSet().GetCompleteBioseq_set()->GetNucFromNucProtSet().GetInst().IsSetMol() )
            {
                iclass = seh.GetSet().GetCompleteBioseq_set()->GetNucFromNucProtSet().GetInst().GetMol();
            }
        }

        int itype = -1;
        if (m_Type->IsChecked() )
        {
            itype = CMolInfo::eBiomol_unknown;
            CSeqdesc_CI desc_iter(seh, CSeqdesc:: e_Molinfo);
            if (desc_iter && desc_iter->GetMolinfo().IsSetBiomol())       
            {
                itype = desc_iter->GetMolinfo().GetBiomol();
            }
        }

        int itopology = -1;
        if (m_Topology->IsChecked())
        {
            itopology = CSeq_inst::eTopology_linear;
            if (seh.IsSeq() && seh.GetSeq().IsSetInst_Topology())
                itopology = seh.GetSeq().GetInst_Topology();
            else if (seh.IsSet() && seh.GetSet().IsSetClass() && seh.GetSet().GetClass() == CBioseq_set::eClass_nuc_prot
                     && seh.GetSet().GetCompleteBioseq_set()->GetNucFromNucProtSet().IsSetInst()
                     && seh.GetSet().GetCompleteBioseq_set()->GetNucFromNucProtSet().GetInst().IsSetTopology() )
            {
                itopology = seh.GetSet().GetCompleteBioseq_set()->GetNucFromNucProtSet().GetInst().GetTopology();
            }
        }

        class_type_topology_to_subset[pair<int, pair<int,int> >(iclass,pair<int,int>(itype,itopology))].push_back(i);
    }

    for ( map< pair<int, pair<int,int> >,vector<int> >::iterator it =  class_type_topology_to_subset.begin(); it != class_type_topology_to_subset.end(); ++it)
        if (!it->second.empty())
            m_Subsets.push_back(it->second);
}



void CMolInfoSubpanel::SetClusters()
{
    CSegregateSetsBase *win =GetBaseFrame();
    size_t set_size = win->GetSetSize();
    m_Clusters.clear();
    if (!m_Class || !m_Type || !m_Topology)
        return;
    if (!m_Class->IsChecked() && !m_Type->IsChecked() && !m_Topology->IsChecked())
        return;
  

    for (unsigned i = 0; i < set_size; i++)
    {
        CSeq_entry_Handle seh = win->GetSeqEntry(i);
        string label = GetLabel(seh);
        stringstream str;       
        
        if (m_Class->IsChecked())
        {
            int iclass = CSeq_inst::eMol_not_set;
            if (seh.IsSeq() && seh.GetSeq().IsSetInst_Mol())
                iclass = seh.GetSeq().GetInst_Mol();
            else if (seh.IsSet() && seh.GetSet().IsSetClass() && seh.GetSet().GetClass() == CBioseq_set::eClass_nuc_prot
                     && seh.GetSet().GetCompleteBioseq_set()->GetNucFromNucProtSet().IsSetInst()
                     && seh.GetSet().GetCompleteBioseq_set()->GetNucFromNucProtSet().GetInst().IsSetMol() )
            {
                iclass = seh.GetSet().GetCompleteBioseq_set()->GetNucFromNucProtSet().GetInst().GetMol();
            }
            str << CSeq_inst::ENUM_METHOD_NAME(EMol)()->FindName(iclass, true);
        }
    
        if (m_Type->IsChecked() )
        {
            int itype = CMolInfo::eBiomol_unknown;
            CSeqdesc_CI desc_iter(seh, CSeqdesc:: e_Molinfo);
            if (desc_iter && desc_iter->GetMolinfo().IsSetBiomol())       
            {
                itype = desc_iter->GetMolinfo().GetBiomol();
            }
            if (!str.str().empty())
                str << ", ";
            str << CMolInfo::ENUM_METHOD_NAME(EBiomol)()->FindName(itype, true);
        }
    

        if (m_Topology->IsChecked())
        {
            int itopology = CSeq_inst::eTopology_linear;
            if (seh.IsSeq() && seh.GetSeq().IsSetInst_Topology())
                itopology = seh.GetSeq().GetInst_Topology();
            else if (seh.IsSet() && seh.GetSet().IsSetClass() && seh.GetSet().GetClass() == CBioseq_set::eClass_nuc_prot
                     && seh.GetSet().GetCompleteBioseq_set()->GetNucFromNucProtSet().IsSetInst()
                     && seh.GetSet().GetCompleteBioseq_set()->GetNucFromNucProtSet().GetInst().IsSetTopology() )
            {
                itopology = seh.GetSet().GetCompleteBioseq_set()->GetNucFromNucProtSet().GetInst().GetTopology();
            }
            if (!str.str().empty())
                str << ", ";
            str << CSeq_inst::ENUM_METHOD_NAME(ETopology)()->FindName(itopology, true);
        }
       
        if (!label.empty() && !str.str().empty())
        {
            m_Clusters[str.str()].push_back(pair<int,string>(i,label));
        }
    }
}

IMPLEMENT_DYNAMIC_CLASS( CStructCommentSubpanel, CRBSubpanel )

BEGIN_EVENT_TABLE( CStructCommentSubpanel, CRBSubpanel )
EVT_CHOICE(wxID_ANY, CStructCommentSubpanel::OnChoiceChanged)
END_EVENT_TABLE()

void CStructCommentSubpanel::CreateControlsMiddle()
{
    CStructCommentSubpanel* itemPanel1 = this;

    wxSizer* itemBoxSizer2 = GetSizer();

    wxArrayString choices;
    GetAllLabels(choices);
    m_Label = new wxChoice(itemPanel1, ID_STRCOMMENT_CHOICE, wxDefaultPosition, wxDefaultSize, choices, 0);
    itemBoxSizer2->Add(m_Label, 0, wxALIGN_CENTER_HORIZONTAL|wxALL|wxFIXED_MINSIZE, 0);   
}

void CStructCommentSubpanel::OnChoiceChanged( wxCommandEvent& event )
{
    SetClusters();
    UpdateList();
    event.Skip();
}

void CStructCommentSubpanel::GetAllLabels(wxArrayString &choices)
{
   CSegregateSetsBase *win =GetBaseFrame();
    if (!win)
        return;
    set<string> labels;
    size_t set_size = win->GetSetSize();
    for (unsigned i = 0; i < set_size; i++)
    {
        CSeq_entry_Handle seh = win->GetSeqEntry(i);
        GetLabelsFromSeq(seh,labels);       
    }
    for (set<string>::iterator label = labels.begin(); label != labels.end(); ++label)
        choices.Add(wxString(*label));
}

void  CStructCommentSubpanel::GetLabelsFromSeq(CSeq_entry_Handle seh, set<string> &labels)
{
    CSeqdesc_CI desc_iter(seh, CSeqdesc::e_User);
    while (desc_iter)       
    {
        const CUser_object& user = desc_iter->GetUser();
        
        if (user.IsSetType() && user.GetType().IsStr() && user.GetType().GetStr() == "StructuredComment" && user.IsSetData()) // User-object.type.str 
            {
                CUser_object::TData::const_iterator i = user.GetData().begin();
                while (i != user.GetData().end())
                {
                    CRef< CUser_field > uf = *i;
                    if (uf->CanGetLabel() && uf->GetLabel().IsStr())
                    {
                        string label = uf->GetLabel().GetStr();
                        if (!label.empty() && label != "StructuredCommentPrefix" && label != "StructuredCommentSuffix")
                            labels.insert(label);
                    }
                    i++;
                }
            }
        ++desc_iter;
    }
}

void CStructCommentSubpanel::OnSelectButton( CSegregateSetsBase *win )
{
    int ichoice = m_Label->GetSelection();
    if (ichoice == wxNOT_FOUND) return;
    string label = m_Label->GetString(ichoice).ToStdString();
    
    vector<int>  subset;
    for(vector<int>::iterator j = m_Available.begin(); j != m_Available.end(); ++j)
    {
        int i = *j;
        CSeq_entry_Handle seh = win->GetSeqEntry(i);
        set<string> labels;
        GetLabelsFromSeq(seh,labels);   
        if (labels.find(label) != labels.end())       
        {
            subset.push_back(i);  
        }
    }
    if (!subset.empty())
        m_Subsets.push_back(subset);
}


void CStructCommentSubpanel::SetClusters()
{
    CSegregateSetsBase *win =GetBaseFrame();
    size_t set_size = win->GetSetSize();
    m_Clusters.clear();
   

    for (unsigned i = 0; i < set_size; i++)
    {
        CSeq_entry_Handle seh = win->GetSeqEntry(i);
        string label = GetLabel(seh);     
        set<string> comments;
        GetLabelsFromSeq(seh,comments);   
        comments.erase(kEmptyStr);
        if (!label.empty() && !comments.empty())
        {
            m_Clusters[NStr::Join(comments, " AND ")].push_back(pair<int,string>(i,label));
        }
    }
}

END_NCBI_SCOPE
