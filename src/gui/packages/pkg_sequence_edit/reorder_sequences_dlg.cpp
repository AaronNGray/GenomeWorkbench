  /*  $Id: reorder_sequences_dlg.cpp 33127 2015-06-01 21:14:42Z filippov $
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

#include <objmgr/scope.hpp>
#include <objects/misc/sequence_macros.hpp>
#include <objects/seq/seq_id_handle.hpp>
#include <objects/misc/sequence_macros.hpp>
#include <gui/objutils/cmd_change_bioseq_set.hpp>
#include <gui/packages/pkg_sequence_edit/reorder_sequences_dlg.hpp>

#include <wx/button.h>
#include <wx/statbox.h>

////@begin XPM images
////@end XPM images

static const char * up_xpm[] = {
"16 16 2 1",
"   c None",
"X  c Gray25",
"                ",
"                ",
"                ",
"                ",
"                ",
"                ",
"        X       ",
"       XXX      ",
"      XXXXX     ",
"     XXXXXXX    ",
"                ",
"                ",
"                ",
"                ",
"                ",
"                "};

  
static const char * down_xpm[] = {
"16 16 2 1",
"   c None",
"X  c Gray25",
"                ",
"                ",
"                ",
"                ",
"                ",
"                ",
"     XXXXXXX    ", 
"      XXXXX     ",   
"       XXX      ",
"        X       ",
"                ",
"                ",
"                ",
"                ",
"                ",
"                "};

BEGIN_NCBI_SCOPE



IMPLEMENT_DYNAMIC_CLASS( CReorderSequencesDlg, wxDialog )


/*!
 * CReorderSequencesDlg event table definition
 */

BEGIN_EVENT_TABLE( CReorderSequencesDlg, wxDialog )

    EVT_BUTTON( ID_BUTTON_UP, CReorderSequencesDlg::OnButtonUp )
    EVT_BUTTON( ID_BUTTON_DOWN, CReorderSequencesDlg::OnButtonDown )
    EVT_BUTTON( ID_BUTTON_SORT, CReorderSequencesDlg::OnButtonSort )

END_EVENT_TABLE()


/*!
 * CReorderSequencesDlg constructors
 */

CReorderSequencesDlg::CReorderSequencesDlg()
{
    Init();
}

CReorderSequencesDlg::CReorderSequencesDlg( wxWindow* parent, objects::CSeq_entry_Handle seh, 
                          wxWindowID id, const wxString& caption, const wxPoint& pos, const wxSize& size, long style )
: m_TopSeqEntry(seh)
{
    Init();
    Create(parent, id, caption, pos, size, style);
}


/*!
 * CVectorTrimDlg creator
 */

bool CReorderSequencesDlg::Create( wxWindow* parent, wxWindowID id, const wxString& caption, const wxPoint& pos, const wxSize& size, long style )
{
////@begin CReorderSequencesDlg creation
    SetExtraStyle(wxWS_EX_BLOCK_EVENTS);
    wxDialog::Create( parent, id, caption, pos, size, style );

    CreateControls();
    if (GetSizer())
    {
        GetSizer()->SetSizeHints(this);
    }
    Centre();
////@end CReorderSequencesDlg creation

    SetSize(wxSize(280, 750));
    return true;
}


/*!
 * CReorderSequencesDlg destructor
 */

CReorderSequencesDlg::~CReorderSequencesDlg()
{
////@begin CReorderSequencesDlg destruction
////@end CReorderSequencesDlg destruction
}


/*!
 * Member initialisation
 */

void CReorderSequencesDlg::Init()
{
    m_ListCtrl = NULL;
    m_ReverseSort = true;
    m_SortButton = NULL;
    m_down_bitmap = wxBitmap(down_xpm);
    m_up_bitmap = wxBitmap(up_xpm);
}


/*!
 * Control creation for CReorderSequencesDlg
 */

void CReorderSequencesDlg::CreateControls()
{    
    CReorderSequencesDlg* itemDialog1 = this;

    wxBoxSizer* itemBoxSizer2 = new wxBoxSizer(wxVERTICAL);
    itemDialog1->SetSizer(itemBoxSizer2);

    wxBoxSizer* itemBoxSizer3 = new wxBoxSizer(wxHORIZONTAL);
    itemBoxSizer2->Add(itemBoxSizer3, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

    m_ListCtrl = new wxListCtrl( itemDialog1, ID_LISTCTRL_REORDER, wxDefaultPosition, wxSize(240, 570), wxLC_REPORT );
    itemBoxSizer3->Add(m_ListCtrl, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    m_ListCtrl->InsertColumn(0,"    Sequence",wxLIST_FORMAT_LEFT,220);
    if (m_TopSeqEntry)
    {
        m_SetSeq.clear();
        ReadBioseq(*(m_TopSeqEntry.GetCompleteSeq_entry()),m_SetSeq, m_TopSeqEntry);

        for (unsigned int i=0; i<m_SetSeq.size(); ++i)
        {
            string label;
            m_SetSeq[i].second.GetSeq().GetSeqId()->GetLabel(&label,CSeq_id::eContent);
            m_ListCtrl->InsertItem(i, ToWxString(label));
            m_ListCtrl->SetItemData(i,i);
        }
    }

    wxStaticBox* itemStaticBoxSizer4Static = new wxStaticBox(itemDialog1, wxID_ANY, _("Move selected sequences"));
    wxStaticBoxSizer* itemStaticBoxSizer4 = new wxStaticBoxSizer(itemStaticBoxSizer4Static, wxHORIZONTAL);
    itemBoxSizer2->Add(itemStaticBoxSizer4, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

    wxButton* itemButton1 = new wxButton( itemDialog1, ID_BUTTON_UP, _("Up"), wxDefaultPosition, wxDefaultSize, 0 );
    itemStaticBoxSizer4->Add(itemButton1, 0, wxALIGN_CENTER_VERTICAL|wxALL, 15);

    wxButton* itemButton2 = new wxButton( itemDialog1, ID_BUTTON_DOWN, _("Down"), wxDefaultPosition, wxDefaultSize, 0 );
    itemStaticBoxSizer4->Add(itemButton2, 0, wxALIGN_CENTER_VERTICAL|wxALL, 15);

    wxBoxSizer* itemBoxSizer5 = new wxBoxSizer(wxHORIZONTAL);
    itemBoxSizer2->Add(itemBoxSizer5, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);


    m_SortButton = new wxButton( itemDialog1, ID_BUTTON_SORT, _("Sort"), wxDefaultPosition, wxDefaultSize, 0 );
    if (m_ReverseSort) m_SortButton->SetBitmapLabel(m_up_bitmap);
    else               m_SortButton->SetBitmapLabel(m_down_bitmap);
    m_SortButton->SetBitmapPosition(wxRIGHT);
    itemBoxSizer5->Add(m_SortButton, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

   
    wxBoxSizer* itemBoxSizer12 = new wxBoxSizer(wxHORIZONTAL);
    itemBoxSizer2->Add(itemBoxSizer12, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

    wxButton* itemButton13 = new wxButton( itemDialog1, wxID_OK, _("Accept"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer12->Add(itemButton13, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxButton* itemButton14 = new wxButton( itemDialog1, wxID_CANCEL, _("Cancel"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer12->Add(itemButton14, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);
}


void CReorderSequencesDlg::ReadBioseq(const CSeq_entry& se,vector < pair<CBioseq_set_Handle,CSeq_entry_Handle> > &setseq, CSeq_entry_Handle seh )
{
    if (se.IsSet()) 
    {
        FOR_EACH_SEQENTRY_ON_SEQSET(it, se.GetSet())
        {
            ReadBioseq(**it,setseq, seh);
        }
    }
    if (se.IsSeq() && se.GetSeq().IsNa())
    {
        CBioseq_Handle bsh = seh.GetBioseqHandle(*se.GetSeq().GetFirstId());
        if (bsh)
        {
            CBioseq_set_Handle bssh = bsh.GetParentBioseq_set();
            if (bssh && bssh.GetBioseq_setCore()->CanGetClass() && bssh.GetBioseq_setCore()->GetClass() ==  CBioseq_set::eClass_nuc_prot)  // if it is a nuc-prot set go a level higher 
                bssh = bssh.GetParentBioseq_set();  
        
            if (bssh)
                setseq.push_back(pair<CBioseq_set_Handle,CSeq_entry_Handle>(bssh,bsh.GetSeq_entry_Handle())); // need to save: parent or grandparent set handle, seq-entry handle  
        }
    }            
}

/*!
 * Should we show tooltips?
 */

bool CReorderSequencesDlg::ShowToolTips()
{
    return true;
}

/*!
 * Get bitmap resources
 */

wxBitmap CReorderSequencesDlg::GetBitmapResource( const wxString& name )
{
    // Bitmap retrieval
    wxUnusedVar(name);
    return wxNullBitmap;
}

/*!
 * Get icon resources
 */

wxIcon CReorderSequencesDlg::GetIconResource( const wxString& name )
{
    // Icon retrieval
    wxUnusedVar(name);
    return wxNullIcon;
}

CRef<CCmdComposite> CReorderSequencesDlg::GetCommand()
{
    CRef<CCmdComposite> cmd( new CCmdComposite("Reorder Sequences") );
     if (m_ListCtrl && m_TopSeqEntry)
     {
         vector < pair<CBioseq_set_Handle,CSeq_entry_Handle> > setseq;
         ReadBioseq(*(m_TopSeqEntry.GetCompleteSeq_entry()),setseq, m_TopSeqEntry);
         set<CBioseq_set_Handle> uniq_bssh;
         for (unsigned int i=0; i<setseq.size(); ++i)
         {
             CBioseq_set_Handle bssh = setseq[i].first;
             uniq_bssh.insert(bssh);
         }
         for (set<CBioseq_set_Handle>::iterator b = uniq_bssh.begin(); b != uniq_bssh.end(); ++b)
         {
             CBioseq_set_Handle bssh = *b;
             if (bssh)
             {
                 CRef<objects::CBioseq_set> new_set(new objects::CBioseq_set());
                 new_set->Assign(*bssh.GetCompleteBioseq_set());
                 vector<int> list_index;
                 PrepareList(list_index);
                 PopulateSet(bssh,*new_set, list_index, m_SetSeq);
                 CCmdChangeBioseqSet *set_cmd = new CCmdChangeBioseqSet(bssh, *new_set);
                 cmd->AddCommand(*set_cmd);
             }
         }
     }

    return cmd;
}

void CReorderSequencesDlg::PrepareList(vector<int> &list_index)
{
    long item = -1;
    for ( ;; )
    {
        item = m_ListCtrl->GetNextItem(item, wxLIST_NEXT_ALL, wxLIST_STATE_DONTCARE);
        if ( item == -1 )  break;
        int i = static_cast<int>(m_ListCtrl->GetItemData(item)); 
        list_index.push_back(i);
    }
}

void CReorderSequencesDlg::PopulateSet(CBioseq_set_Handle bssh, CBioseq_set& new_set, const vector<int> &list_index, const vector < pair<CBioseq_set_Handle,CSeq_entry_Handle> > &set_seq) // Add sorted sequences to a given bioseq-set
{
    if (new_set.CanGetSeq_set())
    {
        CBioseq_set::TSeq_set old_seq_set = new_set.GetSeq_set();
        new_set.ResetSeq_set();
      
        for ( size_t j = 0; j < list_index.size(); j++)
        {
            int i = list_index[j];
            if (set_seq[i].first == bssh)                                         // go through the list of all sequences and select only the ones where saved set handle (parent or grandparent) is the same as the given set
            {
                CRef<CSeq_entry> se(new CSeq_entry());
                if (set_seq[i].second.HasParentEntry()  && set_seq[i].second.GetParentEntry().IsSet() &&                                    // if parent set is different from saved set handle and the given set
                    set_seq[i].second.GetParentBioseq_set() != bssh && set_seq[i].second.GetParentEntry().GetSet().CanGetClass() &&         // and the parent set is a nuc-prot set
                    set_seq[i].second.GetParentEntry().GetSet().GetClass() == CBioseq_set::eClass_nuc_prot)
                    se->Assign(*set_seq[i].second.GetParentEntry().GetCompleteSeq_entry());                                                  // move the complete nuc-prot set
                else
                    se->Assign(*set_seq[i].second.GetCompleteSeq_entry());                                                                   // otherwise just move the entry
                new_set.SetSeq_set().push_back(se);

                CBioseq_set::TSeq_set::iterator it = old_seq_set.begin(); 
                while (it != old_seq_set.end())
                {
                    bool match = false;
                    if ((*it)->IsSeq() && (*it)->GetSeq().CanGetId())
                        for (CBioseq::TId::const_iterator b = (*it)->GetSeq().GetId().begin(); b != (*it)->GetSeq().GetId().end(); ++b)  // remove from old_seq_set sequences with the same id
                            if ((*b)->Match(*set_seq[i].second.GetSeq().GetSeqId())) match = true;

                    if ((*it)->IsSet()  && (*it)->GetSet().CanGetClass() && (*it)->GetSet().GetClass() == CBioseq_set::eClass_nuc_prot && // remove from old_seq_set nuc-prot sets where nuc portion has the same id
                        (*it)->GetSet().GetNucFromNucProtSet().CanGetId())
                        for (CBioseq::TId::const_iterator b = (*it)->GetSet().GetNucFromNucProtSet().GetId().begin(); b != (*it)->GetSet().GetNucFromNucProtSet().GetId().end(); ++b) 
                            if ((*b)->Match(*set_seq[i].second.GetSeq().GetSeqId())) match = true;

                    if (match)
                        it = old_seq_set.erase(it); 
                    else
                        ++it;
                }

            }
        }
        for (CBioseq_set::TSeq_set::iterator it = old_seq_set.begin(); it != old_seq_set.end(); ++it)
            new_set.SetSeq_set().push_back(*it);
    }
}


void CReorderSequencesDlg::OnButtonUp( wxCommandEvent& event )
{
    long item = -1;
    if (m_ListCtrl)
        for ( ;; )
        {
            item = m_ListCtrl->GetNextItem(item, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);
            if ( item == -1 )  break;
            if ( item == 0 ) break; // cannot move up selection which starts at the top.
            int i = static_cast<int>(m_ListCtrl->GetItemData(item)); 
            wxString name_i = m_ListCtrl->GetItemText(item);
            int state_i = m_ListCtrl->GetItemState(item,wxLIST_STATE_SELECTED);
            int j = static_cast<int>(m_ListCtrl->GetItemData(item-1)); 
            wxString name_j = m_ListCtrl->GetItemText(item-1);
            int state_j = m_ListCtrl->GetItemState(item-1,wxLIST_STATE_SELECTED);
            m_ListCtrl->SetItemText(item-1,name_i);
            m_ListCtrl->SetItemData(item-1,i);
            m_ListCtrl->SetItemState(item-1,state_i,wxLIST_STATE_SELECTED);
            m_ListCtrl->SetItemText(item,name_j);
            m_ListCtrl->SetItemData(item,j);
            m_ListCtrl->SetItemState(item,state_j,wxLIST_STATE_SELECTED);
        }
    event.Skip();
}

void CReorderSequencesDlg::OnButtonDown( wxCommandEvent& event )
{
    vector<long> index;
    long item = -1;
    if (m_ListCtrl)
        for ( ;; )
        {
            item = m_ListCtrl->GetNextItem(item, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);
            if ( item == -1 )  break;
            index.push_back(item);
        }
    if ( !index.empty() && index.back() != m_ListCtrl->GetItemCount()-1 )  // cannot move down selection which ends at the bottom.
        for (vector<long>::reverse_iterator rit = index.rbegin(); rit != index.rend(); ++rit)
        {
            item = *rit;
            int i = static_cast<int>(m_ListCtrl->GetItemData(item)); 
            wxString name_i = m_ListCtrl->GetItemText(item);
            int state_i = m_ListCtrl->GetItemState(item,wxLIST_STATE_SELECTED);
            int j = static_cast<int>(m_ListCtrl->GetItemData(item+1)); 
            wxString name_j = m_ListCtrl->GetItemText(item+1);
            int state_j = m_ListCtrl->GetItemState(item+1,wxLIST_STATE_SELECTED);
            m_ListCtrl->SetItemText(item+1,name_i);
            m_ListCtrl->SetItemData(item+1,i);
            m_ListCtrl->SetItemState(item+1,state_i,wxLIST_STATE_SELECTED);
            m_ListCtrl->SetItemText(item,name_j);
            m_ListCtrl->SetItemData(item,j);
            m_ListCtrl->SetItemState(item,state_j,wxLIST_STATE_SELECTED);
        }

    event.Skip();
}

int wxCALLBACK	DirectCompareFunction(wxIntPtr item1, wxIntPtr item2, wxIntPtr data)
{
    typedef vector < pair<CBioseq_set_Handle,CSeq_entry_Handle> > vptr;
    vptr * setseq = (vptr *)(data);
    string a, b;
    (*setseq)[item1].second.GetSeq().GetSeqId()->GetLabel(&a,CSeq_id::eContent);
    (*setseq)[item2].second.GetSeq().GetSeqId()->GetLabel(&b,CSeq_id::eContent);

    int i = NStr::StringToUInt(a,NStr::fConvErr_NoThrow | NStr::fAllowLeadingSymbols | NStr::fAllowTrailingSymbols);
    int j = NStr::StringToUInt(b,NStr::fConvErr_NoThrow | NStr::fAllowLeadingSymbols | NStr::fAllowTrailingSymbols);
    if (i==0 && j==0)
        return (a.compare(b));  
    else
    {
        if (i<j) return -1;
        if (i>j) return 1;
    }
    return 0;
}

int wxCALLBACK	ReverseCompareFunction(wxIntPtr item1, wxIntPtr item2, wxIntPtr data)
{
    return (-DirectCompareFunction(item1,item2,data));
}
	

void CReorderSequencesDlg::OnButtonSort( wxCommandEvent& event )
{
    if (m_ReverseSort)
        m_ListCtrl->SortItems(ReverseCompareFunction, wxIntPtr(&m_SetSeq));
    else
        m_ListCtrl->SortItems(DirectCompareFunction, wxIntPtr(&m_SetSeq));

    m_ReverseSort = !m_ReverseSort;

    if (m_ReverseSort) m_SortButton->SetBitmapLabel(m_up_bitmap);
    else               m_SortButton->SetBitmapLabel(m_down_bitmap);


    event.Skip();
}

static bool cmp_labels(const pair<unsigned int, pair<bool, string>  >&a, const pair<unsigned int, pair<bool,string> > &b)
{
    bool numeric1 = a.second.first;
    bool numeric2 = b.second.first;
    string str1 = a.second.second;
    string str2 = b.second.second;
    if (numeric1 && numeric2)
    {
        unsigned int i = NStr::StringToUInt(str1,NStr::fConvErr_NoThrow | NStr::fAllowLeadingSymbols | NStr::fAllowTrailingSymbols);
        unsigned int j = NStr::StringToUInt(str2,NStr::fConvErr_NoThrow | NStr::fAllowLeadingSymbols | NStr::fAllowTrailingSymbols);
        if (i != 0 && j != 0)
            return (i < j);
    }
    return (str1 < str2); 
}

string CReorderSequencesDlg::GetIdLabel(CBioseq_Handle bsh, bool &numeric)
{
    string genebank, general_str, general_int, local;
    if (bsh.IsSetId())
        for (CBioseq_Handle::TId::const_iterator it = bsh.GetId().begin(); it != bsh.GetId().end(); ++it)
        {
            const CSeq_id &id = *(it->GetSeqId());
            if (id.IsGenbank() && id.GetGenbank().IsSetAccession())
                genebank = id.GetGenbank().GetAccession();
            if (id.IsGeneral() && id.GetGeneral().IsSetTag() && id.GetGeneral().GetTag().IsStr())
                general_str = id.GetGeneral().GetTag().GetStr();
            if (id.IsGeneral() && id.GetGeneral().IsSetTag() && id.GetGeneral().GetTag().IsId())
                general_int = NStr::IntToString(id.GetGeneral().GetTag().GetId());
            if (id.IsLocal() && id.GetLocal().IsStr())
                local = id.GetLocal().GetStr();
        }
    if (!genebank.empty())
    {
        numeric = false;
        return genebank;
    }
    if (!general_int.empty())
    {
        numeric = true;
        return general_int;
    }
    if (!general_str.empty())
    {
        numeric = false;
        return general_str;
    }
    numeric = true;
    return local;  
}

CRef<CCmdComposite>  CReorderSequencesDlg::ReorderById(objects::CSeq_entry_Handle seh)
{
    CRef<CCmdComposite> cmd( new CCmdComposite("Reorder Sequences By Id") );
    vector < pair<CBioseq_set_Handle,CSeq_entry_Handle> > set_seq;
    vector<pair<unsigned int, pair<bool, string> > > list_seq;
    ReadBioseq(*(seh.GetCompleteSeq_entry()),set_seq, seh);
    for (unsigned int i=0; i<set_seq.size(); ++i)
    { 
        bool numeric;
        string label = GetIdLabel(set_seq[i].second.GetSeq(), numeric);
        list_seq.push_back(pair<unsigned int, pair<bool, string> >(i, pair<bool, string>(numeric,label)));
    }
    set<CBioseq_set_Handle> uniq_bssh;
    for (unsigned int i=0; i<set_seq.size(); ++i)
    {
        CBioseq_set_Handle bssh = set_seq[i].first;
        uniq_bssh.insert(bssh);
    }
    sort(list_seq.begin(), list_seq.end(), cmp_labels);

    vector<int> list_index;
    for (size_t j = 0; j < list_seq.size(); j++)
    {
        list_index.push_back(list_seq[j].first);
    }

    for (set<CBioseq_set_Handle>::iterator b = uniq_bssh.begin(); b != uniq_bssh.end(); ++b)
    {
        CBioseq_set_Handle bssh = *b;
        if (bssh)
        {
            CRef<objects::CBioseq_set> new_set(new objects::CBioseq_set());
            new_set->Assign(*bssh.GetCompleteBioseq_set());
            PopulateSet(bssh,*new_set, list_index, set_seq);
            CCmdChangeBioseqSet *set_cmd = new CCmdChangeBioseqSet(bssh, *new_set);
            cmd->AddCommand(*set_cmd);
        }
    }
    return cmd;
}

END_NCBI_SCOPE
