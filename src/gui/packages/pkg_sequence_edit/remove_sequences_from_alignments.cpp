/*  $Id: remove_sequences_from_alignments.cpp 43857 2019-09-11 18:41:03Z filippov $
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
#include <objmgr/seq_annot_ci.hpp>
#include <objmgr/align_ci.hpp>
#include <objmgr/util/sequence.hpp>
#include <gui/objutils/label.hpp>
#include <objects/seqalign/Seq_align.hpp>
#include <objects/seq/seq_id_handle.hpp>
#include <objects/seqloc/Seq_id.hpp>
#include <objects/seqalign/Seq_align_set.hpp>
#include <gui/objutils/cmd_del_seq_align.hpp>
#include <gui/objutils/cmd_del_seq_annot.hpp>
#include <gui/objutils/cmd_change_align.hpp>
#include <gui/packages/pkg_sequence_edit/remove_sequences_from_alignments.hpp>

#include <wx/button.h>

////@begin XPM images
////@end XPM images

BEGIN_NCBI_SCOPE

static bool GetIdLabel(const CSeq_id& id,  set<string> &labels, const string name = "", bool detect = false, bool found = false)
{
    string label;
    id.GetLabel(&label);
    if (!detect)
    {
        if (!label.empty()) 
	    labels.insert(label);
    }
    else
    {
        if (label == name) found = true;
    }
    return found; 
}


template<class T>
static bool GetIds(const T& d, set<string> &labels, const string name = "", bool detect = false, bool found = false)
{
    if (d.IsSetIds())
        for (typename T::TIds::const_iterator id_iter = d.GetIds().begin(); id_iter != d.GetIds().end(); ++id_iter)
            found = GetIdLabel(**id_iter, labels, name, detect, found);
    return found;
}

template<class T>
static bool GetSeqId(const T& d, set<string> &labels,  const string name = "", bool detect = false, bool found = false)
{
    if (d.IsSetId())
        found = GetIdLabel(d.GetId(), labels, name, detect, found);
    return found;
}


IMPLEMENT_DYNAMIC_CLASS( CRemoveSeqFromAlignDlg, wxDialog )



BEGIN_EVENT_TABLE( CRemoveSeqFromAlignDlg, wxDialog )


END_EVENT_TABLE()


CRemoveSeqFromAlignDlg::CRemoveSeqFromAlignDlg()
{
    Init();
}

CRemoveSeqFromAlignDlg::CRemoveSeqFromAlignDlg( wxWindow* parent, objects::CSeq_entry_Handle seh, 
                                                    wxWindowID id, const wxString& caption, const wxPoint& pos, const wxSize& size, long style )
: m_TopSeqEntry(seh)
{
    Init(seh);
    Create(parent, id, caption, pos, size, style);
}

bool CRemoveSeqFromAlignDlg::Create( wxWindow* parent, wxWindowID id, const wxString& caption, const wxPoint& pos, const wxSize& size, long style )
{
    SetExtraStyle(wxWS_EX_BLOCK_EVENTS);
    wxDialog::Create( parent, id, caption, pos, size, style );

    CreateControls();
    if (GetSizer())
    {
        GetSizer()->SetSizeHints(this);
    }
    Centre();

    SetSize(wxSize(280, 600));
    return true;
}


CRemoveSeqFromAlignDlg::~CRemoveSeqFromAlignDlg()
{
}


/*!
 * Member initialisation
 */

void CRemoveSeqFromAlignDlg::Init()
{
    m_ListCtrl = NULL;
}

void CRemoveSeqFromAlignDlg::Init(objects::CSeq_entry_Handle seh)
{
    m_ListCtrl = NULL;
    SearchSeq_entry (seh, *(seh.GetCompleteSeq_entry()));
}




void CRemoveSeqFromAlignDlg::CreateControls()
{    
    CRemoveSeqFromAlignDlg* itemDialog1 = this;

    wxBoxSizer* itemBoxSizer2 = new wxBoxSizer(wxVERTICAL);
    itemDialog1->SetSizer(itemBoxSizer2);

    wxBoxSizer* itemBoxSizer3 = new wxBoxSizer(wxHORIZONTAL);
    itemBoxSizer2->Add(itemBoxSizer3, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

    m_ListCtrl = new wxListCtrl( itemDialog1, ID_LISTCTRL_SEQ_ALIGN, wxDefaultPosition, wxSize(240, 550), wxLC_REPORT );
    itemBoxSizer3->Add(m_ListCtrl, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    m_ListCtrl->InsertColumn(0,"     Sequence ID",wxLIST_FORMAT_LEFT,237);
    int i = 0;
    for (set<string>::iterator e = m_Label.begin(); e != m_Label.end(); e++)
    {
        m_ListCtrl->InsertItem(i, ToWxString(*e));
        i++;
    }

    wxBoxSizer* itemBoxSizer12 = new wxBoxSizer(wxHORIZONTAL);
    itemBoxSizer2->Add(itemBoxSizer12, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

    wxButton* itemButton13 = new wxButton( itemDialog1, wxID_OK, _("Accept"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer12->Add(itemButton13, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxButton* itemButton14 = new wxButton( itemDialog1, wxID_CANCEL, _("Cancel"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer12->Add(itemButton14, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);
}


/*!
 * Should we show tooltips?
 */

bool CRemoveSeqFromAlignDlg::ShowToolTips()
{
    return true;
}

/*!
 * Get bitmap resources
 */

wxBitmap CRemoveSeqFromAlignDlg::GetBitmapResource( const wxString& name )
{
    // Bitmap retrieval
    wxUnusedVar(name);
    return wxNullBitmap;
}

/*!
 * Get icon resources
 */

wxIcon CRemoveSeqFromAlignDlg::GetIconResource( const wxString& name )
{
    // Icon retrieval
    wxUnusedVar(name);
    return wxNullIcon;
}

void CRemoveSeqFromAlignDlg::SearchSeq_entry (objects::CSeq_entry_Handle tse, const CSeq_entry& se)
{
    CSeq_entry_Handle seh = tse.GetScope().GetSeq_entryHandle(se);
    for ( CSeq_annot_CI it(seh, CSeq_annot_CI::eSearch_entry); it ; ++it ) 
    {
        CConstRef<CSeq_annot> annot = it->GetCompleteSeq_annot();
        if (annot->IsSetData() && annot->GetData().IsAlign())
        {
            CSeq_annot::C_Data::TAlign aligns = annot->GetData().GetAlign();
            for (CSeq_annot::C_Data::TAlign::iterator a=aligns.begin(); a!=aligns.end(); a++)
                GetAlignLabel(**a);
        }
    }


    if (se.IsSet()) 
    {
        FOR_EACH_SEQENTRY_ON_SEQSET (it, se.GetSet()) 
        {
            SearchSeq_entry (tse, **it);
        }
    }
}

bool CRemoveSeqFromAlignDlg::GetIdsFromLocs(const CStd_seg::TLoc& locs, set<string> &labels, const string name, bool detect, bool found)
{
    for (CStd_seg::TLoc::const_iterator l = locs.begin(); l != locs.end(); ++l)
        found = GetIdsFromLoc(**l, labels, name, detect, found);
    return found;
}

bool CRemoveSeqFromAlignDlg::GetIdsFromLoc(const CSeq_loc& loc, set<string> &labels, const string name, bool detect, bool found)
{
    switch (loc.Which())
    {
    case CSeq_loc::e_Empty:  found = GetIdLabel(loc.GetEmpty(), labels, name, detect, found); break;
    case CSeq_loc::e_Whole: found = GetIdLabel(loc.GetWhole(), labels, name, detect, found); break;
    case CSeq_loc::e_Int:   found = GetSeqId(loc.GetInt(), labels, name, detect, found); break;        
    case CSeq_loc::e_Packed_int: 
        if (loc.GetPacked_int().CanGet())
            for (CPacked_seqint::Tdata::const_iterator d = loc.GetPacked_int().Get().begin(); d != loc.GetPacked_int().Get().end(); ++d)
                found = GetSeqId(**d, labels, name, detect, found);
        break;
    case CSeq_loc::e_Pnt: found = GetSeqId(loc.GetPnt(), labels, name, detect, found); break;
    case CSeq_loc::e_Packed_pnt: found = GetSeqId(loc.GetPacked_pnt(), labels, name, detect, found); break;
    case CSeq_loc::e_Mix: 
        if (loc.GetMix().CanGet())
            for (CSeq_loc_mix::Tdata::const_iterator d = loc.GetMix().Get().begin(); d != loc.GetMix().Get().end(); ++d)
                found = GetIdsFromLoc(**d, labels, name, detect, found);
        break;
    case CSeq_loc::e_Equiv: 
        if (loc.GetEquiv().CanGet())
            for (CSeq_loc_equiv::Tdata::const_iterator d = loc.GetEquiv().Get().begin(); d != loc.GetEquiv().Get().end(); ++d)
                found = GetIdsFromLoc(**d, labels, name, detect, found);
        break;        
    case CSeq_loc::e_Bond:
        if (loc.GetBond().CanGetA()) 
            found = GetSeqId(loc.GetBond().GetA(), labels, name, detect, found);
        break;
    case CSeq_loc::e_Feat:          
    default: break;
    }
    return found;
}

void CRemoveSeqFromAlignDlg::GetAlignLabel(const CSeq_align& align)
{
    if (align.IsSetSegs())
    {
        switch(align.GetSegs().Which())
        {
        case CSeq_align::C_Segs::e_Dendiag:
            for (CSeq_align::C_Segs::TDendiag::const_iterator d = align.GetSegs().GetDendiag().begin(); d != align.GetSegs().GetDendiag().end(); ++d)
                GetIds(**d, m_Label);
            break;
        case CSeq_align::C_Segs::e_Denseg: // segments
            GetIds(align.GetSegs().GetDenseg(), m_Label);
            break;
        case CSeq_align::C_Segs::e_Std: 
            for (CSeq_align::C_Segs::TStd::const_iterator d = align.GetSegs().GetStd().begin(); d != align.GetSegs().GetStd().end(); ++d) // iterate over seq loc. See SeqIdPtrFromSeqAlign in salpacc.c
            {                                                                                                                             // also SeqLocId in sequtil.c
                GetIds(**d, m_Label);                                                                                                              
                if (!(*d)->IsSetIds() && (*d)->IsSetLoc())
                    GetIdsFromLocs((*d)->GetLoc(), m_Label);
            }
            break;
        case CSeq_align::C_Segs::e_Packed: // segments
            GetIds(align.GetSegs().GetPacked(), m_Label);
            break;
        case CSeq_align::C_Segs::e_Disc: 
            for (CSeq_align::C_Segs::TDisc::Tdata::const_iterator a = align.GetSegs().GetDisc().Get().begin(); a != align.GetSegs().GetDisc().Get().end(); ++a)
                GetAlignLabel(**a);
            break;
        case CSeq_align::C_Segs::e_Spliced:
        {
            if (align.GetSegs().GetSpliced().IsSetProduct_id())
                GetIdLabel(align.GetSegs().GetSpliced().GetProduct_id(), m_Label);
            if (align.GetSegs().GetSpliced().IsSetGenomic_id())
                GetIdLabel(align.GetSegs().GetSpliced().GetGenomic_id(), m_Label);
            break;
        }
        case CSeq_align::C_Segs::e_Sparse:
            if (align.GetSegs().GetSparse().IsSetRows())
            {
                for (CSeq_align::C_Segs::TSparse::TRows::const_iterator d = align.GetSegs().GetSparse().GetRows().begin(); d != align.GetSegs().GetSparse().GetRows().end(); ++d)
                {
                    if ((*d)->IsSetFirst_id()) GetIdLabel((*d)->GetFirst_id(), m_Label);
                    if ((*d)->IsSetSecond_id()) GetIdLabel((*d)->GetSecond_id(), m_Label);
                }
            }
            break;
        default: break;
        }
    }
}


bool CRemoveSeqFromAlignDlg::RemoveIdLabel(const CSeq_align& align, const string name, bool& modified)
{
    set<string> labels;
    bool rem = false;
    if (align.IsSetSegs()) // Kill the whole align if it's pairwise align GetIds().size() == 2. 
                            // TODO - sequin does not like if it's #1 seqid in a pair-wise align
    {                       // See CheckForRemoveSequenceFromAlignmentsProblemsCallback in sqnutil3.c  
                            // and SeqIdOrderInBioseqIdList in sequtil.c; Note that SeqIdOrderInBioseqIdList counts from 1 !!!!
        switch(align.GetSegs().Which())
        {
        case CSeq_align::C_Segs::e_Dendiag: // Dendiag is removed if id is found. See RemoveOneSequenceFromAlignment in sqnutil3.c
            for (CSeq_align::C_Segs::TDendiag::const_iterator d = align.GetSegs().GetDendiag().begin(); d != align.GetSegs().GetDendiag().end(); ++d) 
                rem = rem || GetIds(**d, labels, name,true);
            break;
        case CSeq_align::C_Segs::e_Denseg: // segments - the only one potentially not killed outright. see SeqAlignBioseqDeleteById in salpedit.c 
            if (GetIds(align.GetSegs().GetDenseg(), labels, name,true))
            {
                if (align.GetSegs().GetDenseg().GetIds().size() == 2) 
                    rem = true;                                           
                else   
                    modified = true;
            }
            break;
        case CSeq_align::C_Segs::e_Std: 
            for (CSeq_align::C_Segs::TStd::const_iterator d = align.GetSegs().GetStd().begin(); d != align.GetSegs().GetStd().end(); ++d)
            {
                bool find_id = GetIds(**d, labels, name,true);
                if (!(*d)->IsSetIds() && (*d)->IsSetLoc())
                    find_id = GetIdsFromLocs((*d)->GetLoc(), labels, name,true,find_id);
                int dim = 2;
                if ((*d)->IsSetIds())
                    dim = (*d)->GetIds().size();
                else if ( (*d)->IsSetDim() )
                    dim = (*d)->GetDim();
                rem = rem || (find_id && dim == 2);       
            }
            break;
        case CSeq_align::C_Segs::e_Packed: // segments
            if (GetIds(align.GetSegs().GetPacked(), labels, name,true) && align.GetSegs().GetPacked().GetIds().size() == 2) 
                rem = true;     
            break;
        case CSeq_align::C_Segs::e_Disc: 
            for (CSeq_align::C_Segs::TDisc::Tdata::const_iterator a = align.GetSegs().GetDisc().Get().begin(); a != align.GetSegs().GetDisc().Get().end(); ++a)
                rem = rem || RemoveIdLabel(**a, name, modified);  // Should we remove only the underlying sub-align?
            break;
        case CSeq_align::C_Segs::e_Spliced: 
        {
            if (align.GetSegs().GetSpliced().IsSetProduct_id())
                rem = rem || GetIdLabel(align.GetSegs().GetSpliced().GetProduct_id(), labels, name, true);
            if (align.GetSegs().GetSpliced().IsSetGenomic_id())
                rem = rem || GetIdLabel(align.GetSegs().GetSpliced().GetGenomic_id(), labels, name, true);
            break; 
        }
        case CSeq_align::C_Segs::e_Sparse: 
            if (align.GetSegs().GetSparse().IsSetRows())
            {
                unsigned int num = 0;
                for (CSeq_align::C_Segs::TSparse::TRows::const_iterator d = align.GetSegs().GetSparse().GetRows().begin(); d != align.GetSegs().GetSparse().GetRows().end(); ++d)
                {
                    bool found = false;
                    if ((*d)->IsSetFirst_id()) found = GetIdLabel((*d)->GetFirst_id(), labels, name, true);
                    if ((*d)->IsSetSecond_id()) found  = found || GetIdLabel((*d)->GetSecond_id(), labels, name, true);
                    if (found) num++;
                }
                if (num == align.GetSegs().GetSparse().GetRows().size()) rem = true;
                else if (num > 0 ) modified = true;
            }
            break;
        default: break;
        }
    }
    return rem;
}


CRef<CCmdComposite> CRemoveSeqFromAlignDlg::GetCommand()
{
    long item = -1;
    CRef<CCmdComposite> cmd( new CCmdComposite("Remove Sequences From Alignments") );
    vector<string> names;
    if (m_ListCtrl && m_TopSeqEntry)
        for ( ;; )
        {
            item = m_ListCtrl->GetNextItem(item, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);
            if ( item == -1 )  break;
            string name = ToStdString(m_ListCtrl->GetItemText(item));
	    names.push_back(name);
        }
    if (!names.empty())
	ApplyToCSeq_entry (m_TopSeqEntry, cmd, names);
    return cmd;
}

void CRemoveSeqFromAlignDlg::ModifyAlign(CRef<CSeq_align> align, const string name)
{
    set<string> labels;
    if (align->IsSetSegs() && align->GetSegs().Which() == CSeq_align::C_Segs::e_Denseg 
        &&  GetIds(align->GetSegs().GetDenseg(), labels, name,true) && align->GetSegs().GetDenseg().GetIds().size() != 2)
    {
        unsigned int dim = align->GetSegs().GetDenseg().GetIds().size();
        unsigned int index = 0;
        bool found = false;
        CDense_seg::TIds ids;
        for (CDense_seg::TIds::const_iterator id_iter = align->GetSegs().GetDenseg().GetIds().begin(); id_iter != align->GetSegs().GetDenseg().GetIds().end(); ++id_iter)
        {
            if (GetIdLabel(**id_iter, labels, name,true)) found = true;
            else ids.push_back(*id_iter);

            if (!found) index++;
        }

        if (align->GetSegs().GetDenseg().IsSetNumseg() && index<align->GetSegs().GetDenseg().GetIds().size())
        {
            unsigned int numseg = align->GetSegs().GetDenseg().GetNumseg();
            // remove starts, strands
            CDense_seg::TStarts starts;
            CDense_seg::TStarts::const_iterator start_iter;
            CDense_seg::TStrands strands;
            CDense_seg::TStrands::const_iterator strand_iter;
            
            if (align->GetSegs().GetDenseg().IsSetStarts()) start_iter = align->GetSegs().GetDenseg().GetStarts().begin();
            if (align->GetSegs().GetDenseg().IsSetStrands()) strand_iter = align->GetSegs().GetDenseg().GetStrands().begin();

            for (unsigned int seg = 0; seg<numseg; seg++)
                for (unsigned int i=0; i<dim; i++)
                    if (i != index)
                    {
                        if (align->GetSegs().GetDenseg().IsSetStarts() && start_iter != align->GetSegs().GetDenseg().GetStarts().end())
                        {
                            starts.push_back(*start_iter);
                            start_iter++;
                        }
                        if (align->GetSegs().GetDenseg().IsSetStrands() && strand_iter != align->GetSegs().GetDenseg().GetStrands().end())
                        {
                            strands.push_back(*strand_iter);
                            strand_iter++;
                        }
                    }
                    else
                    {
                        if (align->GetSegs().GetDenseg().IsSetStarts() && start_iter != align->GetSegs().GetDenseg().GetStarts().end())
                            start_iter++;
                        
                        if (align->GetSegs().GetDenseg().IsSetStrands() && strand_iter != align->GetSegs().GetDenseg().GetStrands().end())
                            strand_iter++;
                        
                    }
            
            if (align->GetSegs().GetDenseg().IsSetStarts())  align->SetSegs().SetDenseg().SetStarts().swap(starts);
            if (align->GetSegs().GetDenseg().IsSetStrands()) align->SetSegs().SetDenseg().SetStrands().swap(strands);
        }

        align->SetSegs().SetDenseg().SetDim(ids.size());        
        align->SetDim(ids.size());
        align->SetSegs().SetDenseg().SetIds().swap(ids);
        align->SetSegs().SetDenseg().RemovePureGapSegs(); // sets numsegs correctly        
    }
    else if (align->IsSetSegs() && align->GetSegs().Which() == CSeq_align::C_Segs::e_Sparse &&
             align->GetSegs().GetSparse().IsSetRows())
    {
        CSeq_align::C_Segs::TSparse::TRows rows;
        CSeq_align::C_Segs::TSparse::TRow_scores row_scores;
        CSeq_align::C_Segs::TSparse::TExt ext;
        int i=0;
        for (CSeq_align::C_Segs::TSparse::TRows::const_iterator d = align->GetSegs().GetSparse().GetRows().begin(); d != align->GetSegs().GetSparse().GetRows().end(); ++d)
        {
            bool found = false;
            if ((*d)->IsSetFirst_id()) found = GetIdLabel((*d)->GetFirst_id(), labels, name, true);
            if ((*d)->IsSetSecond_id()) found  = found || GetIdLabel((*d)->GetSecond_id(), labels, name, true);
            if (!found) 
            {
                rows.push_back(*d);
                if (align->GetSegs().GetSparse().IsSetRow_scores()) row_scores.push_back(align->GetSegs().GetSparse().GetRow_scores()[i]);
                if (align->GetSegs().GetSparse().IsSetExt()) ext.push_back(align->GetSegs().GetSparse().GetExt()[i]);
            }
            i++;
        }
        align->SetSegs().SetSparse().SetRows().swap(rows);
        if (align->GetSegs().GetSparse().IsSetRow_scores()) align->SetSegs().SetSparse().SetRow_scores().swap(row_scores);
        if (align->GetSegs().GetSparse().IsSetExt()) align->SetSegs().SetSparse().SetExt().swap(ext);
        
    }
}

void CRemoveSeqFromAlignDlg::ApplyToCSeq_entry (objects::CSeq_entry_Handle tse, CCmdComposite* composite,  const vector<string> &names)  
{
    for ( CSeq_annot_CI it(tse); it ; ++it ) 
    {
        int deleted = 0;
        int total = 0;
        for (CAlign_CI align_ci(*it); align_ci; ++align_ci, ++total)
        {
            bool modified = false;
	    bool to_delete = false;
	    for (auto name : names)
		to_delete |= RemoveIdLabel(align_ci.GetOriginalSeq_align(), name, modified);
           
            if (!to_delete && modified)  
            {
                CRef<CSeq_align> edited_align(new CSeq_align());
                edited_align->Assign(align_ci.GetOriginalSeq_align());
		for (auto name : names)
		    ModifyAlign (edited_align,name);
		if (edited_align->CheckNumRows() < 2)
		    to_delete = true;
		else
		{
		    CIRef<IEditCommand> cmd(new CCmdChangeAlign(align_ci.GetSeq_align_Handle(),*edited_align));
		    composite->AddCommand(*cmd);
		}
	    }
	    if (to_delete)
            {
                CIRef<IEditCommand> cmdDelAlign(new CCmdDelSeq_align(align_ci.GetSeq_align_Handle()));
                composite->AddCommand(*cmdDelAlign);
                deleted++;
            }   
        }
             // If all aligns have been removed kill annot as well. See  RemoveOneSequenceFromAlignment in sqnutil3.c
        if (deleted == total && it->IsAlign())
        {
            CIRef<IEditCommand> cmdDelAnnot(new CCmdDelSeq_annot(*it));
            composite->AddCommand(*cmdDelAnnot);
        }
    } 
}


END_NCBI_SCOPE























