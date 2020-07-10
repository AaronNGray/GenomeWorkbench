/*  $Id: edit_seq_ends_with_align.cpp 37738 2017-02-13 16:04:36Z filippov $
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
#include <objects/seqalign/Dense_seg.hpp>
#include <objects/seqfeat/seqfeat_macros.hpp>
#include <objects/seqres/Seq_graph.hpp>
#include <objects/seqres/Byte_graph.hpp>
#include <objects/seqres/Int_graph.hpp>
#include <objects/seqres/Real_graph.hpp>
#include <objmgr/scope.hpp>
#include <objmgr/object_manager.hpp>
#include <objmgr/feat_ci.hpp>
#include <objmgr/align_ci.hpp>
#include <objmgr/bioseq_ci.hpp>
#include <objmgr/seq_annot_ci.hpp>
#include <objmgr/graph_ci.hpp>
#include <objmgr/seq_map.hpp>
#include <objmgr/seq_map_ci.hpp>
#include <objmgr/util/sequence.hpp>
#include <gui/objutils/cmd_change_bioseq_inst.hpp>
#include <gui/objutils/cmd_change_seq_feat.hpp>
#include <gui/objutils/cmd_del_seq_feat.hpp>
#include <gui/objutils/cmd_del_seq_annot.hpp>
#include <gui/objutils/cmd_del_seq_graph.hpp>
#include <gui/objutils/cmd_change_graph.hpp>
#include <gui/packages/pkg_sequence_edit/edit_seq_ends_with_align.hpp>

#include <wx/button.h>
#include <wx/sizer.h>
#include <wx/stattext.h>
#include <wx/msgdlg.h>
#include <wx/valtext.h>

BEGIN_NCBI_SCOPE
USING_SCOPE(objects);


/*!
 * CEditSeqEndsWithAlign type definition
 */

IMPLEMENT_DYNAMIC_CLASS( CEditSeqEndsWithAlign, wxDialog )


/*!
 * CEditSeqEndsWithAlign event table definition
 */

BEGIN_EVENT_TABLE( CEditSeqEndsWithAlign, wxDialog )

////@begin CEditSeqEndsWithAlign event table entries
////@end CEditSeqEndsWithAlign event table entries

END_EVENT_TABLE()


/*!
 * CEditSeqEndsWithAlign constructors
 */

CEditSeqEndsWithAlign::CEditSeqEndsWithAlign()
{
    Init();
}

CEditSeqEndsWithAlign::CEditSeqEndsWithAlign( wxWindow* parent, objects::CSeq_entry_Handle seh, 
                          wxWindowID id, const wxString& caption, const wxPoint& pos, const wxSize& size, long style )
: m_TopSeqEntry(seh)
{
    Init();
    Create(parent, id, caption, pos, size, style);
}



bool CEditSeqEndsWithAlign::Create( wxWindow* parent, wxWindowID id, const wxString& caption, const wxPoint& pos, const wxSize& size, long style )
{
////@begin CEditSeqEndsWithAlign creation
    SetExtraStyle(wxWS_EX_BLOCK_EVENTS);
    wxDialog::Create( parent, id, caption, pos, size, style );

    CreateControls();
    if (GetSizer())
    {
        GetSizer()->SetSizeHints(this);
    }
    Centre();
////@end CEditSeqEndsWithAlign creation

    return true;
}


/*!
 * CEditSeqEndsWithAlign destructor
 */

CEditSeqEndsWithAlign::~CEditSeqEndsWithAlign()
{
////@begin CEditSeqEndsWithAlign destruction
////@end CEditSeqEndsWithAlign destruction
}


/*!
 * Member initialisation
 */

void CEditSeqEndsWithAlign::Init()
{
    
}


/*!
 * Control creation for CEditSeqEndsWithAlign
 */

void CEditSeqEndsWithAlign::CreateControls()
{    
    CEditSeqEndsWithAlign* itemDialog1 = this;

    wxBoxSizer* itemBoxSizer2 = new wxBoxSizer(wxVERTICAL);
    itemDialog1->SetSizer(itemBoxSizer2);
  
    wxBoxSizer* itemBoxSizer3 = new wxBoxSizer(wxHORIZONTAL);
    itemBoxSizer2->Add(itemBoxSizer3, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

    wxStaticText* stattext1 = new wxStaticText( itemDialog1, wxID_STATIC, _("Trim before alignment coordinate"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer3->Add(stattext1, 0, wxALIGN_TOP|wxALL, 5);
    m_Before = new wxTextCtrl( itemDialog1, wxID_ANY, wxEmptyString, wxDefaultPosition, wxSize(100, -1), 0 );
    itemBoxSizer3->Add(m_Before, 0, wxALIGN_TOP|wxLEFT|wxRIGHT|wxBOTTOM, 5);
    m_Before->SetValidator( wxTextValidator( wxFILTER_NUMERIC ) );

    wxBoxSizer* itemBoxSizer4 = new wxBoxSizer(wxHORIZONTAL);
    itemBoxSizer2->Add(itemBoxSizer4, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

    wxStaticText* stattext2 = new wxStaticText( itemDialog1, wxID_STATIC, _("Trim after alignment coordinate"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer4->Add(stattext2, 0, wxALIGN_TOP|wxALL, 5);
    m_After = new wxTextCtrl( itemDialog1, wxID_ANY, wxEmptyString, wxDefaultPosition, wxSize(100, -1), 0 );
    itemBoxSizer4->Add(m_After, 0, wxALIGN_TOP|wxLEFT|wxRIGHT|wxBOTTOM, 5);
    m_After->SetValidator( wxTextValidator( wxFILTER_NUMERIC ) );

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

bool CEditSeqEndsWithAlign::ShowToolTips()
{
    return true;
}

/*!
 * Get bitmap resources
 */

wxBitmap CEditSeqEndsWithAlign::GetBitmapResource( const wxString& name )
{
    // Bitmap retrieval
    wxUnusedVar(name);
    return wxNullBitmap;
}

/*!
 * Get icon resources
 */

wxIcon CEditSeqEndsWithAlign::GetIconResource( const wxString& name )
{
    // Icon retrieval
    wxUnusedVar(name);
    return wxNullIcon;
}

CRef<CCmdComposite> CEditSeqEndsWithAlign::GetCommand()
{
    CRef<CCmdComposite> cmd(new CCmdComposite("Edit Sequence Ends With Alignment"));
    for ( CBioseq_CI bi(m_TopSeqEntry); bi; ++bi)
    {
        const CBioseq_Handle& bsh = *bi;
        int before = wxAtoi(m_Before->GetValue());
        int after =  wxAtoi(m_After->GetValue());
        before--;
        after--;
        if (m_After->GetValue().IsEmpty())                             
            after = bsh.GetBioseqLength()-1;
        CAlign_CI align_ci(bsh);
        if (align_ci)
        {
            const CSeq_align& align = *align_ci;
            int row = FindRow(bsh, align);
            if (row >= 0)
            {
                CRef<CObjectManager> object_manager = CObjectManager::GetInstance();
                CRef<CScope> scope_copy(new CScope(*object_manager));

                int real_before = FindPosition(align,row,before);
                if (real_before < 0)
                {
                    before++;
                    while (real_before < 0 && before < bsh.GetBioseqLength())
                    {
                        real_before = FindPosition(align,row,before);
                        before++;
                    }
                }
                if (real_before < 0 || m_Before->GetValue().IsEmpty())
                    real_before = 0;
                int real_after = FindPosition(align,row,after);
                if (real_after < 0)
                {
                    after--;
                    while (real_after < 0 && after >= 0)
                    {
                        real_after = FindPosition(align,row,after);
                        after--;
                    }
                }
                if (real_after < 0 || m_After->GetValue().IsEmpty())
                    real_after =  bsh.GetBioseqLength()-1;
                CRef<CSeq_entry> copy(new CSeq_entry);
                copy->Assign(*bsh.GetSeq_entry_Handle().GetCompleteSeq_entry());
                CSeq_entry_Handle edited_seh = scope_copy->AddTopLevelSeqEntry(*copy);
                TrimBeforeAfter(edited_seh.GetSeq(), real_before, real_after);
                CRef<CCmdChangeBioseqInst> cmd_bioseq(new CCmdChangeBioseqInst(bsh, edited_seh.GetCompleteSeq_entry()->GetSeq().GetInst()));
                cmd->AddCommand(*cmd_bioseq);
                AdjustFeatureLocations(bsh, real_before, real_after, cmd);
                TrimQualityScores(bsh, real_before, real_after, cmd);
            }
        }
    }
    
    for (CSeq_annot_CI annot_ci(m_TopSeqEntry); annot_ci; ++annot_ci)
    {
        if (annot_ci->IsAlign())
        {
            CRef<CCmdDelSeq_annot> del_align(new CCmdDelSeq_annot(*annot_ci));
            cmd->AddCommand(*del_align);
        }
    }
    return cmd;
}


int CEditSeqEndsWithAlign::FindRow(const CBioseq_Handle& bsh, const CSeq_align& align)
{
    int row = -1;
    int num_rows = align.CheckNumRows();
    for (int r = 0; r < num_rows; r++)
    {
        if (bsh.IsSynonym(align.GetSeq_id(r)))
        {
            row = r;
            break;
        }
    }
    return row;
}

int CEditSeqEndsWithAlign::FindPosition(const CSeq_align& align, int row, int offset)
{
    int pos = -1;
    int num_rows = align.CheckNumRows();
    if (align.IsSetSegs() && align.GetSegs().IsDenseg())
    {
        const vector< TSignedSeqPos > &starts = align.GetSegs().GetDenseg().GetStarts();
        const vector< TSeqPos > &lens = align.GetSegs().GetDenseg().GetLens();
        int i = 0;

        while (i < lens.size())
        {
            if (starts[i*num_rows+row] >= 0 && offset < lens[i])
            {
                if (offset < 0)
                    offset = 0;
                pos = offset + starts[i*num_rows+row];
                break;
            }
            offset -= lens[i];          
            i++;
        }
    }
    return pos;
}

void CEditSeqEndsWithAlign::TrimBeforeAfter(CBioseq_Handle edited_bsh, int before, int after)
{
    SSeqMapSelector selector;
    selector.SetFlags(CSeqMap::fFindData); 
    CBioseq_EditHandle besh(edited_bsh);
    CSeqMap_I seqmap_i(besh, selector);
    int start = 0;
    while (seqmap_i)
    {
        TSeqPos len = seqmap_i.GetLength();
        if (before >= start+len)
        {
            seqmap_i = seqmap_i.Remove(); 
        }
        else if (before > start && before < start+len && after >= start+len-1)
        {
            string seq_in;
            if (besh.IsNa())
                seqmap_i.GetSequence(seq_in, CSeqUtil::e_Iupacna);
            else
                seqmap_i.GetSequence(seq_in, CSeqUtil::e_Iupacaa);
            string seq_out = seq_in.substr(before - start);
            if (besh.IsNa())
                seqmap_i.SetSequence(seq_out, CSeqUtil::e_Iupacna, CSeq_data::e_Iupacna);
            else
                seqmap_i.SetSequence(seq_out, CSeqUtil::e_Iupacaa, CSeq_data::e_Iupacaa);            
            ++seqmap_i;
        }
        else if (after < start)
        {
            seqmap_i = seqmap_i.Remove(); 
        }
        else if (before > start && before < start+len && after < start+len-1)
        {
            string seq_in;
            if (besh.IsNa())
                seqmap_i.GetSequence(seq_in, CSeqUtil::e_Iupacna);
            else
                seqmap_i.GetSequence(seq_in, CSeqUtil::e_Iupacaa);
            string seq_out = seq_in.substr(before - start,after - before + 1);
            if (besh.IsNa())
                seqmap_i.SetSequence(seq_out, CSeqUtil::e_Iupacna, CSeq_data::e_Iupacna);
            else
                seqmap_i.SetSequence(seq_out, CSeqUtil::e_Iupacaa, CSeq_data::e_Iupacaa);            
            ++seqmap_i;
        }
        else if (before <= start && after >= start && after+1 < start+len)
        {
            string seq_in;
            if (besh.IsNa())
                seqmap_i.GetSequence(seq_in, CSeqUtil::e_Iupacna);
            else
                seqmap_i.GetSequence(seq_in, CSeqUtil::e_Iupacaa);
            string seq_out = seq_in.substr(0, after - start + 1);
            if (besh.IsNa())
                seqmap_i.SetSequence(seq_out, CSeqUtil::e_Iupacna, CSeq_data::e_Iupacna);
            else
                seqmap_i.SetSequence(seq_out, CSeqUtil::e_Iupacaa, CSeq_data::e_Iupacaa);            
            ++seqmap_i;
        }
        else
        {
            ++seqmap_i;
        }
        start += len;
    }
}

void CEditSeqEndsWithAlign::AdjustFeatureLocations(CBioseq_Handle bsh, int before, int after, CRef<CCmdComposite> cmd)
{
    CRef<CSeq_loc> loc = bsh.GetRangeSeq_loc(0,0);
    TSeqPos seq_start = loc->GetStart(objects::eExtreme_Positional);
    SAnnotSelector sel;
    CScope &scope = bsh.GetScope();

    for (CFeat_CI feat_it(scope, *loc, sel); feat_it; ++feat_it) 
    {
        CRef<CSeq_feat> new_feat(new CSeq_feat);
        new_feat->Assign(feat_it->GetOriginalFeature());
        const CSeq_loc& feat_loc = feat_it->GetLocation();
        bool modified = false;
        bool removed = false;
        CRef<CSeq_loc> loc = AdjustLoc(feat_loc, before, after, seq_start, modified, removed);

        if (modified)
        {
            if (removed)
            {
                CRef<CCmdDelSeq_feat> cmd_del(new CCmdDelSeq_feat(feat_it->GetSeq_feat_Handle()));
                if (cmd_del)
                {
                    cmd->AddCommand(*cmd_del);
                }
            }
            else
            {
                if (new_feat->IsSetData() && new_feat->GetData().IsCdregion())
                {
                    EDIT_EACH_CODEBREAK_ON_CDREGION(code_break, new_feat->SetData().SetCdregion())
                    {
                        if ((*code_break)->IsSetLoc())
                        {
                            bool cb_modified = false;
                            bool cb_removed = false;
                            CRef<CSeq_loc> cb_loc = AdjustLoc((*code_break)->GetLoc(), before, after, seq_start, cb_modified, cb_removed);
                            if (cb_modified)
                            {
                                if (cb_removed)
                                    ERASE_CODEBREAK_ON_CDREGION(code_break, new_feat->SetData().SetCdregion());
                                else
                                    (*code_break)->SetLoc(*cb_loc);
                            }
                        }
                    }
                    int diff = before - (feat_loc.GetStart(objects::eExtreme_Biological) - seq_start);
                    if (feat_loc.GetStrand() == eNa_strand_minus)
                        diff = (feat_loc.GetStart(objects::eExtreme_Biological) - seq_start) - after;
                    AdjustFrame(new_feat->SetData().SetCdregion(), before, diff);
                }
                if (new_feat->IsSetData() && new_feat->GetData().IsRna() && new_feat->GetData().GetRna().IsSetExt() 
                    && new_feat->GetData().GetRna().GetExt().IsTRNA() && new_feat->GetData().GetRna().GetExt().GetTRNA().IsSetAnticodon())
                {
                    bool ac_modified = false;
                    bool ac_removed = false;
                    CRef<CSeq_loc> ac_loc = AdjustLoc(new_feat->GetData().GetRna().GetExt().GetTRNA().GetAnticodon(), before, after, seq_start, ac_modified, ac_removed);
                    if (ac_modified)
                    {
                        if (ac_removed)
                            new_feat->SetData().SetRna().SetExt().SetTRNA().ResetAnticodon();
                        else
                            new_feat->SetData().SetRna().SetExt().SetTRNA().SetAnticodon(*ac_loc);
                    }
                }
                new_feat->SetLocation(*loc);      
                CRef< CCmdChangeSeq_feat > cmd_change_feat(new CCmdChangeSeq_feat(feat_it->GetSeq_feat_Handle(),*new_feat)); 
                if (cmd_change_feat) 
                {
                    cmd->AddCommand(*cmd_change_feat);  
                }
            }
        }
        
    }
}

CRef<CSeq_loc> CEditSeqEndsWithAlign::AdjustLoc(const CSeq_loc& feat_loc, int before, int after, TSeqPos seq_start, bool &modified, bool &removed)
{
    CRef<CSeq_loc> new_loc(new CSeq_loc);
    new_loc->Assign(feat_loc);
    CSeq_loc_I loc_it(*new_loc);
    while(loc_it)      
    {
        CSeq_loc_CI::TRange feat_range = loc_it.GetRange();
        TSeqPos feat_start = feat_range.GetFrom() - seq_start;
        TSeqPos feat_stop = feat_range.GetTo() - seq_start;
        int new_start = feat_start;
        int new_stop = feat_stop;
        if (new_start < before && new_stop < before)
        {
            new_start = 0;
            new_stop = 0;
        }
        else if (new_start < before && new_stop >= before && new_stop <= after)
        {
            new_start = 0;
            new_stop -= before;
        }
        else if (new_start < before && new_stop > after)
        {
            new_start = 0;
            new_stop = after - before;
        }
        else if (new_start >= before && new_start <= after && new_stop > after)
        {
            new_start -= before;
            new_stop = after-before;
        }
        else if (new_start >= before && new_start <= after &&  new_stop >= before && new_stop <= after)
        {
            new_start -= before;
            new_stop -= before;
        }
        else if ( new_start > after && new_stop > after)
        {
            new_start = 0;
            new_stop = 0;
        }
        
        if (new_stop <= new_start)
        {
            loc_it.Delete();               
            continue;
        }
        if (feat_start != new_start)
        {
            loc_it.SetFrom(new_start);
        }
        if (feat_stop != new_stop)
        {
            loc_it.SetTo(new_stop);
        }
        ++loc_it;
    }
    CRef<CSeq_loc> loc = loc_it.MakeSeq_loc();
    modified = loc_it.HasChanges();
    removed = (loc_it.GetSize() == 0 || loc->GetStart(objects::eExtreme_Positional) == loc->GetStop(objects::eExtreme_Positional));
    loc_it.Rewind();
    return loc;
}

void CEditSeqEndsWithAlign::AdjustFrame(CCdregion &feat, int before, int diff)
{
    if (diff <= 0)
        return;
    int orig_frame = 0;
    if (feat.IsSetFrame()) 
    {
        if (feat.GetFrame() == CCdregion::eFrame_two) 
        {
            orig_frame = 1;
        } else if (feat.GetFrame() == CCdregion::eFrame_three) 
        {
            orig_frame = 2;
        }
    }
    int new_offset = (orig_frame - diff) % 3;
    if (orig_frame < diff && new_offset != 0)
    {
        new_offset = 3 - ((diff - orig_frame) % 3);
    }
            
    CCdregion::EFrame new_frame = CCdregion::eFrame_not_set;
    switch (new_offset) {
    case 0:
        new_frame = CCdregion::eFrame_one;
        break;
    case 1:
        new_frame = CCdregion::eFrame_two;
        break;
    case 2:
        new_frame = CCdregion::eFrame_three;
            break;
    default:
        new_frame = CCdregion::eFrame_not_set;
        break;
    }
    feat.SetFrame(new_frame);
}

void CEditSeqEndsWithAlign::TrimQualityScores(CBioseq_Handle bsh, int before, int after, CRef<CCmdComposite> command)
{
    CRef<CSeq_loc> loc = bsh.GetRangeSeq_loc(0,0);
    TSeqPos seq_start = loc->GetStart(objects::eExtreme_Positional);

    SAnnotSelector graph_sel(CSeq_annot::C_Data::e_Graph);
    CGraph_CI graph_ci(bsh, graph_sel);
    for (; graph_ci; ++graph_ci)
    {

        const CMappedGraph& graph = *graph_ci;

        CRef<CSeq_graph> new_graph(new CSeq_graph());
        new_graph->Assign(graph.GetOriginalGraph());
        
        if (new_graph->IsSetLoc())
        {
            bool g_modified = false;
            bool g_removed = false;
            CRef<CSeq_loc> g_loc = AdjustLoc(new_graph->GetLoc(), before, after, seq_start, g_modified, g_removed);
            if (g_modified)
            {
                if (g_removed)
                {
                    new_graph->ResetLoc();
                    CIRef<IEditCommand> delGraph(new CCmdDelSeq_graph( graph.GetSeq_graph_Handle() )); 
                    command->AddCommand(*delGraph);
                }
                else
                {
                    TSeqPos graph_start = new_graph->GetLoc().GetStart(eExtreme_Positional);
                    TSeqPos graph_stop = new_graph->GetLoc().GetStop(eExtreme_Positional);
                    // Copy start/stop values are relative to bioseq coordinate system.
                    // Change them so that they are relative to graph location.
                    int copy_start = before - graph_start;
                    int copy_stop = after - graph_start;

                    new_graph->SetLoc(*g_loc);

                    CSeq_graph::TGraph& dst_data = new_graph->SetGraph();
                    const CSeq_graph::TGraph& src_data = graph.GetOriginalGraph().GetGraph();
                    switch ( src_data.Which() ) 
                    {
                    case CSeq_graph::TGraph::e_Byte:
                    {
                        // Update data values
                        dst_data.SetByte().ResetValues();
                        dst_data.SetByte().SetValues().insert(
                            dst_data.SetByte().SetValues().end(), 
                            src_data.GetByte().GetValues().begin() + copy_start,
                            src_data.GetByte().GetValues().begin() + copy_stop + 1);
                    }   
                    break;
                    case CSeq_graph::TGraph::e_Int:
                    {
                        // Update data values
                        dst_data.SetInt().ResetValues();
                        dst_data.SetInt().SetValues().insert(
                            dst_data.SetInt().SetValues().end(), 
                            src_data.GetInt().GetValues().begin() + copy_start,
                            src_data.GetInt().GetValues().begin() + copy_stop + 1);
                        
                    }   
                    break;
                    case CSeq_graph::TGraph::e_Real:
                    {
                        // Update data values
                        dst_data.SetReal().ResetValues();
                        dst_data.SetReal().SetValues().insert(
                            dst_data.SetReal().SetValues().end(), 
                            src_data.GetReal().GetValues().begin() + copy_start,
                            src_data.GetReal().GetValues().begin() + copy_stop + 1);
                        
                    }   
                    break;
                    default: break;
                    }
                    // Update numvals
                    new_graph->SetNumval(copy_stop - copy_start + 1);
                    CIRef<IEditCommand> chgGraph( new CCmdChangeGraph( graph.GetSeq_graph_Handle(), *new_graph ));
                    command->AddCommand(*chgGraph);
                }
            }   
        }
    }
}


END_NCBI_SCOPE
