/* $Id: raw_seq_to_delta_by_loc.cpp 43516 2019-07-22 16:29:32Z filippov $
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

// For compilers that support precompilation, includes "wx/wx.h".
#include "wx/wxprec.h"

#ifdef __BORLANDC__
#pragma hdrstop
#endif

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

////@begin includes
////@end includes

#include <objects/seqset/Seq_entry.hpp>
#include <objects/seq/Seq_data.hpp>
#include <objects/seq/Seq_gap.hpp>
#include <objects/seq/Linkage_evidence.hpp>
#include <objects/seqfeat/SeqFeatXref.hpp>
#include <objmgr/object_manager.hpp>
#include <objmgr/scope.hpp>
#include <objects/seqalign/Dense_seg.hpp>
#include <objects/seqalign/Score.hpp>
#include <objects/seqloc/Na_strand.hpp>
#include <objects/seq/Seq_ext.hpp>
#include <objects/seq/Delta_ext.hpp>
#include <objects/seq/Delta_seq.hpp>
#include <objects/seq/Seq_literal.hpp>
#include <objects/seq/IUPACna_.hpp>
#include <objects/general/Object_id.hpp>
#include <objects/seqfeat/Feat_id.hpp>
#include <objtools/cleanup/fix_feature_id.hpp>
#include <objmgr/feat_ci.hpp>
#include <objmgr/graph_ci.hpp>
#include <objmgr/align_ci.hpp>
#include <objmgr/util/sequence.hpp>
#include <objmgr/util/feature.hpp>
#include <objects/seqloc/Seq_loc.hpp>
#include <objects/seq/Seq_inst.hpp>
#include <objtools/edit/loc_edit.hpp>
#include <objtools/edit/seq_entry_edit.hpp>
#include <objtools/edit/cds_fix.hpp>
#include <gui/objutils/cmd_del_seq_align.hpp>
#include <gui/objutils/cmd_change_align.hpp>
#include <gui/objutils/cmd_create_align.hpp>
#include <gui/objutils/cmd_del_seq_feat.hpp>
#include <util/sequtil/sequtil_convert.hpp>
#include <gui/objutils/util_cmds.hpp>
#include <gui/objutils/cmd_change_seq_feat.hpp>
#include <gui/widgets/edit/edit_object_seq_feat.hpp>
#include <gui/packages/pkg_sequence_edit/edit_seq_ends_with_align.hpp>
#include <gui/packages/pkg_sequence_edit/split_cds_with_too_many_n.hpp>
#include <gui/packages/pkg_sequence_edit/adjust_features_for_gaps.hpp>
#include <gui/packages/pkg_sequence_edit/raw_seq_to_delta_by_loc.hpp>


////@begin XPM images
////@end XPM images

BEGIN_NCBI_SCOPE


/*
 * CRawSeqToDeltaSeqByLoc type definition
 */

IMPLEMENT_DYNAMIC_CLASS( CRawSeqToDeltaSeqByLoc, wxDialog )


/*
 * CRawSeqToDeltaSeqByLoc event table definition
 */

BEGIN_EVENT_TABLE( CRawSeqToDeltaSeqByLoc, wxDialog )

////@begin CRawSeqToDeltaSeqByLoc event table entries
////@end CRawSeqToDeltaSeqByLoc event table entries

END_EVENT_TABLE()


/*
 * CRawSeqToDeltaSeqByLoc constructors
 */

CRawSeqToDeltaSeqByLoc::CRawSeqToDeltaSeqByLoc()
{
    Init();
}

CRawSeqToDeltaSeqByLoc::CRawSeqToDeltaSeqByLoc( wxWindow* parent, objects::CSeq_entry_Handle tse, wxWindowID id, const wxString& caption, const wxPoint& pos, const wxSize& size, long style )
    : m_TopSeqEntry(tse)
{
    Init();
    Create(parent, id, caption, pos, size, style);
}


/*
 * CRawSeqToDeltaSeqByLoc creator
 */

bool CRawSeqToDeltaSeqByLoc::Create( wxWindow* parent, wxWindowID id, const wxString& caption, const wxPoint& pos, const wxSize& size, long style )
{
////@begin CRawSeqToDeltaSeqByLoc creation
    SetExtraStyle(wxWS_EX_BLOCK_EVENTS);
    wxDialog::Create( parent, id, caption, pos, size, style );

    CreateControls();
    if (GetSizer())
    {
        GetSizer()->SetSizeHints(this);
    }
    Centre();
////@end CRawSeqToDeltaSeqByLoc creation
    return true;
}


/*
 * CRawSeqToDeltaSeqByLoc destructor
 */

CRawSeqToDeltaSeqByLoc::~CRawSeqToDeltaSeqByLoc()
{
////@begin CRawSeqToDeltaSeqByLoc destruction
////@end CRawSeqToDeltaSeqByLoc destruction
}


/*
 * Member initialisation
 */

void CRawSeqToDeltaSeqByLoc::Init()
{
////@begin CRawSeqToDeltaSeqByLoc member initialisation
    m_ScrollSizer = NULL;
    m_CoordSeq = NULL;
    m_AdjustCDS = NULL;
////@end CRawSeqToDeltaSeqByLoc member initialisation
}


/*
 * Control creation for CRawSeqToDeltaSeqByLoc
 */

void CRawSeqToDeltaSeqByLoc::CreateControls()
{    
////@begin CRawSeqToDeltaSeqByLoc content construction
    // Generated by DialogBlocks, 16/09/2015 11:34:00 (unregistered)

    CRawSeqToDeltaSeqByLoc* itemDialog1 = this;

    wxBoxSizer* itemBoxSizer2 = new wxBoxSizer(wxVERTICAL);
    itemDialog1->SetSizer(itemBoxSizer2);

    wxBoxSizer* itemBoxSizer3 = new wxBoxSizer(wxHORIZONTAL);
    itemBoxSizer2->Add(itemBoxSizer3, 0, wxGROW|wxLEFT, 40);

    wxStaticText* itemStaticText4 = new wxStaticText( itemDialog1, wxID_STATIC, _("Start"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer3->Add(itemStaticText4, 1, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxStaticText* itemStaticText5 = new wxStaticText( itemDialog1, wxID_STATIC, _("Type"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer3->Add(itemStaticText5, 1, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxStaticText* itemStaticText6 = new wxStaticText( itemDialog1, wxID_STATIC, _("Length"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer3->Add(itemStaticText6, 1, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxStaticText* itemStaticText7 = new wxStaticText( itemDialog1, wxID_STATIC, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer3->Add(itemStaticText7, 1, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxScrolledWindow* itemScrolledWindow8 = new wxScrolledWindow(itemDialog1, ID_CRAWTODELTA_SCROLLEDWINDOW, wxDefaultPosition, wxSize(-1, 120), wxSUNKEN_BORDER | wxHSCROLL | wxVSCROLL);
    itemBoxSizer2->Add(itemScrolledWindow8, 0, wxGROW|wxALL, 5);
    itemScrolledWindow8->SetScrollbars(0, 1, 0, 30);
    m_ScrollSizer = new wxBoxSizer(wxVERTICAL);
    itemScrolledWindow8->SetSizer(m_ScrollSizer);

    wxArrayString itemChoice12Strings;
    itemChoice12Strings.Add(_("Unknown length"));
    itemChoice12Strings.Add(_("Known length"));
    wxArrayString itemChoice14Strings;
    itemChoice14Strings.Add(_("Insert"));
    itemChoice14Strings.Add(_("Replace"));
    for (size_t i = 0; i < 4; i++)
    {
        wxBoxSizer* itemBoxSizer10 = new wxBoxSizer(wxHORIZONTAL);
        m_ScrollSizer->Add(itemBoxSizer10, 0, wxALIGN_CENTER_HORIZONTAL|wxLEFT|wxRIGHT, 5);

        m_LastStart = new wxTextCtrl( itemScrolledWindow8, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
        itemBoxSizer10->Add(m_LastStart, 0, wxALIGN_CENTER_VERTICAL|wxLEFT|wxRIGHT, 5);
        m_LastStart->SetValidator( wxTextValidator( wxFILTER_NUMERIC ) );

        wxChoice* itemChoice12 = new wxChoice( itemScrolledWindow8, wxID_ANY, wxDefaultPosition, wxDefaultSize, itemChoice12Strings, 0 );
        itemChoice12->SetStringSelection(_("Unknown length"));
        itemBoxSizer10->Add(itemChoice12, 0, wxALIGN_CENTER_VERTICAL|wxLEFT|wxRIGHT, 5);

        m_LastLength = new wxTextCtrl( itemScrolledWindow8, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
        itemBoxSizer10->Add(m_LastLength, 0, wxALIGN_CENTER_VERTICAL|wxLEFT|wxRIGHT, 5);
        m_LastLength->SetValidator( wxTextValidator( wxFILTER_NUMERIC ) );

        wxChoice* itemChoice14 = new wxChoice( itemScrolledWindow8, wxID_ANY, wxDefaultPosition, wxDefaultSize, itemChoice14Strings, 0 );
        itemChoice14->SetStringSelection(_("Insert"));
        itemBoxSizer10->Add(itemChoice14, 0, wxALIGN_CENTER_VERTICAL|wxLEFT|wxRIGHT, 5);
    }
    m_LastStart->Bind(wxEVT_TEXT, &CRawSeqToDeltaSeqByLoc::OnNewText, this);
    m_LastLength->Bind(wxEVT_TEXT, &CRawSeqToDeltaSeqByLoc::OnNewText, this);
    itemScrolledWindow8->FitInside();

    wxStaticBox* itemStaticBoxSizer30Static = new wxStaticBox(itemDialog1, wxID_ANY, _("Coordinates"));
    wxStaticBoxSizer* itemStaticBoxSizer30 = new wxStaticBoxSizer(itemStaticBoxSizer30Static, wxHORIZONTAL);
    itemBoxSizer2->Add(itemStaticBoxSizer30, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

    m_CoordSeq = new wxRadioButton(itemStaticBoxSizer30->GetStaticBox(), ID_CRAWTODELTA_RADIOBUTTON, _("Sequence"), wxDefaultPosition, wxDefaultSize, wxRB_GROUP);
    m_CoordSeq->SetValue(true);
    itemStaticBoxSizer30->Add(m_CoordSeq, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxRadioButton* itemRadioButton32 = new wxRadioButton(itemStaticBoxSizer30->GetStaticBox(), ID_CRAWTODELTA_RADIOBUTTON1, _("Alignment"), wxDefaultPosition, wxDefaultSize, 0);
    itemRadioButton32->SetValue(false);
    itemStaticBoxSizer30->Add(itemRadioButton32, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    objects::CAlign_CI align_ci(m_TopSeqEntry, objects::CSeq_annot::C_Data::e_Align);
    if (align_ci)
    {
        m_CoordSeq->Enable();
        itemRadioButton32->Enable();
    }
    else
    {
        m_CoordSeq->Disable();
        itemRadioButton32->Disable();
    }

    m_AdjustCDS = new wxCheckBox(itemDialog1, ID_CRAWTODELTA_CHECKBOX, _("Adjust CDS location for gaps"), wxDefaultPosition, wxDefaultSize, 0);
    m_AdjustCDS->SetValue(true);
    itemBoxSizer2->Add(m_AdjustCDS, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

    wxBoxSizer* itemBoxSizer34 = new wxBoxSizer(wxHORIZONTAL);
    itemBoxSizer2->Add(itemBoxSizer34, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

    wxButton* itemButton35 = new wxButton( itemDialog1, wxID_OK, _("Accept"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer34->Add(itemButton35, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxButton* itemButton36 = new wxButton( itemDialog1, wxID_CANCEL, _("Cancel"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer34->Add(itemButton36, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

////@end CRawSeqToDeltaSeqByLoc content construction
}

void CRawSeqToDeltaSeqByLoc::OnNewText(wxCommandEvent& event)
{
    m_LastStart->Unbind(wxEVT_TEXT, &CRawSeqToDeltaSeqByLoc::OnNewText, this);
    m_LastLength->Unbind(wxEVT_TEXT, &CRawSeqToDeltaSeqByLoc::OnNewText, this);

    wxArrayString itemChoice12Strings;
    itemChoice12Strings.Add(_("Unknown length"));
    itemChoice12Strings.Add(_("Known length"));
    wxArrayString itemChoice14Strings;
    itemChoice14Strings.Add(_("Insert"));
    itemChoice14Strings.Add(_("Replace"));

    wxWindow* parent = m_ScrollSizer->GetContainingWindow();

    wxBoxSizer* itemBoxSizer10 = new wxBoxSizer(wxHORIZONTAL);
    m_ScrollSizer->Add(itemBoxSizer10, 0, wxALIGN_CENTER_HORIZONTAL|wxLEFT|wxRIGHT, 5);

    m_LastStart = new wxTextCtrl( parent, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer10->Add(m_LastStart, 0, wxALIGN_CENTER_VERTICAL|wxLEFT|wxRIGHT, 5);
    m_LastStart->SetValidator( wxTextValidator( wxFILTER_NUMERIC ) );

    wxChoice* itemChoice12 = new wxChoice( parent, wxID_ANY, wxDefaultPosition, wxDefaultSize, itemChoice12Strings, 0 );
    itemChoice12->SetStringSelection(_("Unknown length"));
    itemBoxSizer10->Add(itemChoice12, 0, wxALIGN_CENTER_VERTICAL|wxLEFT|wxRIGHT, 5);

    m_LastLength = new wxTextCtrl( parent, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer10->Add(m_LastLength, 0, wxALIGN_CENTER_VERTICAL|wxLEFT|wxRIGHT, 5);
    m_LastLength->SetValidator( wxTextValidator( wxFILTER_NUMERIC ) );
    
    wxChoice* itemChoice14 = new wxChoice( parent, wxID_ANY, wxDefaultPosition, wxDefaultSize, itemChoice14Strings, 0 );
    itemChoice14->SetStringSelection(_("Insert"));
    itemBoxSizer10->Add(itemChoice14, 0, wxALIGN_CENTER_VERTICAL|wxLEFT|wxRIGHT, 5);

    m_LastStart->Bind(wxEVT_TEXT, &CRawSeqToDeltaSeqByLoc::OnNewText, this);
    m_LastLength->Bind(wxEVT_TEXT, &CRawSeqToDeltaSeqByLoc::OnNewText, this);

    parent->FitInside();
}

/*
 * Should we show tooltips?
 */

bool CRawSeqToDeltaSeqByLoc::ShowToolTips()
{
    return true;
}

/*
 * Get bitmap resources
 */

wxBitmap CRawSeqToDeltaSeqByLoc::GetBitmapResource( const wxString& name )
{
    // Bitmap retrieval
////@begin CRawSeqToDeltaSeqByLoc bitmap retrieval
    wxUnusedVar(name);
    return wxNullBitmap;
////@end CRawSeqToDeltaSeqByLoc bitmap retrieval
}

/*
 * Get icon resources
 */

wxIcon CRawSeqToDeltaSeqByLoc::GetIconResource( const wxString& name )
{
    // Icon retrieval
////@begin CRawSeqToDeltaSeqByLoc icon retrieval
    wxUnusedVar(name);
    return wxNullIcon;
////@end CRawSeqToDeltaSeqByLoc icon retrieval
}

CRef<CCmdComposite> CRawSeqToDeltaSeqByLoc::apply(string title)
{
    map<CObject_id::TId, CObject_id::TId> old_to_new;
    bool create_xref_map = true;
    apply_impl(title, old_to_new, create_xref_map);
    create_xref_map = false;
    return apply_impl(title, old_to_new, create_xref_map);
}

CRef<CCmdComposite> CRawSeqToDeltaSeqByLoc::apply_impl(string title, map<CObject_id::TId, CObject_id::TId> &old_to_new, bool create_xref_map)
{
    vector<NRawToDeltaSeq::SGap> orig_gaps;
    wxSizerItemList::iterator node =  m_ScrollSizer->GetChildren().begin();
    while (node != m_ScrollSizer->GetChildren().end())
    {
        if ((*node)->IsSizer())
        {
            wxTextCtrl *start_ctrl = dynamic_cast<wxTextCtrl*>((*node)->GetSizer()->GetItem((size_t)0)->GetWindow());
            wxChoice *unknown_known_ctrl = dynamic_cast<wxChoice*>((*node)->GetSizer()->GetItem((size_t)1)->GetWindow());
            wxTextCtrl *length_ctrl = dynamic_cast<wxTextCtrl*>((*node)->GetSizer()->GetItem((size_t)2)->GetWindow());
            wxChoice *insert_replace_ctrl = dynamic_cast<wxChoice*>((*node)->GetSizer()->GetItem((size_t)3)->GetWindow());
            if (start_ctrl && unknown_known_ctrl && length_ctrl && insert_replace_ctrl && !start_ctrl->GetValue().IsEmpty() && !length_ctrl->GetValue().IsEmpty())
            {
                NRawToDeltaSeq::SGap gap;
                gap.start = wxAtoi(start_ctrl->GetValue()) - 1;
                gap.length = wxAtoi(length_ctrl->GetValue());
                gap.is_known = false;
                if (unknown_known_ctrl->GetSelection() != wxNOT_FOUND)
                    gap.is_known = (unknown_known_ctrl->GetString(unknown_known_ctrl->GetSelection()) == _("Known length"));
                gap.is_replace = false;
                if (insert_replace_ctrl->GetSelection() != wxNOT_FOUND)
                    gap.is_replace = (insert_replace_ctrl->GetString(insert_replace_ctrl->GetSelection()) == _("Replace"));
                
                orig_gaps.push_back(gap);
            }
        }
        ++node;
    }
    sort(orig_gaps.begin(), orig_gaps.end());
    if (!m_TopSeqEntry) 
        return CRef<CCmdComposite>(nullptr);

    objects::CScope &scope = m_TopSeqEntry.GetScope();
    bool create_general_only = objects::edit::IsGeneralIdProtPresent(m_TopSeqEntry);
    CObject_id::TId  max_feat_id = CFixFeatureId::s_FindHighestFeatureId(m_TopSeqEntry);
    CRef<CCmdComposite> composite(new CCmdComposite(title)); 
    bool alignment_affected = false;
    map<CBioseq_Handle, vector<NRawToDeltaSeq::SGap> > map_gaps;
    for( objects::CBioseq_CI bi(m_TopSeqEntry, objects::CSeq_inst::eMol_na); bi; ++bi)
    {
        objects::CBioseq_Handle bsh = *bi;
        vector<NRawToDeltaSeq::SGap> gaps(orig_gaps);
        if ( m_CoordSeq->IsEnabled() && !m_CoordSeq->GetValue())
        {
            UseAlignCoords(bsh, gaps);
        }
        if (gaps.empty())
            continue;
        map_gaps[bsh] = gaps;
        CRef<objects::CBioseq> bseq( new objects::CBioseq );
        bseq->Assign(*bsh.GetCompleteBioseq());                              
        objects::CSeq_inst& inst = bseq->SetInst();
        // can only convert if starting as raw
        if (!inst.IsSetRepr() || inst.GetRepr() != objects::CSeq_inst::eRepr_raw
            || !inst.IsSetSeq_data()) {
            continue;
        }
            
        string iupacna;
        switch(inst.GetSeq_data().Which()) {
        case objects::CSeq_data::e_Iupacna:
            iupacna = inst.GetSeq_data().GetIupacna();
            break;
        case objects::CSeq_data::e_Ncbi2na:
            CSeqConvert::Convert(inst.GetSeq_data().GetNcbi2na().Get(), CSeqUtil::e_Ncbi2na,
                                 0, inst.GetLength(), iupacna, CSeqUtil::e_Iupacna);
            break;
        case objects::CSeq_data::e_Ncbi4na:
            CSeqConvert::Convert(inst.GetSeq_data().GetNcbi4na().Get(), CSeqUtil::e_Ncbi4na,
                                 0, inst.GetLength(), iupacna, CSeqUtil::e_Iupacna);
            break;
        case objects::CSeq_data::e_Ncbi8na:
            CSeqConvert::Convert(inst.GetSeq_data().GetNcbi8na().Get(), CSeqUtil::e_Ncbi8na,
                                 0, inst.GetLength(), iupacna, CSeqUtil::e_Iupacna);
            break;
        default:
            return CRef<CCmdComposite>(NULL);
            break;
        }
        int literal_start = 0;
        inst.ResetSeq_data();
        inst.SetRepr(objects::CSeq_inst::eRepr_delta);
        int total_length = iupacna.length();
        if (inst.IsSetLength())
            total_length = inst.GetLength();
        for (size_t i = 0; i < gaps.size(); i++)
        {
            int literal_length = gaps[i].start - literal_start;
            if (literal_length < 0 || gaps[i].start > iupacna.size())
            {
                wxMessageBox(_("Inconsistent gap intervals"), _("Error in gap interval specification"), wxOK|wxICON_ERROR);
                return CRef<CCmdComposite>(nullptr);                   
            }
            if (literal_length > 0)
            {
                string literal = iupacna.substr(literal_start, literal_length);
                CRef<objects::CDelta_seq> ds(new objects::CDelta_seq());
                ds->SetLiteral().SetSeq_data().SetIupacna().Set(literal);
                ds->SetLiteral().SetLength(literal.length());  
                inst.SetExt().SetDelta().Set().push_back(ds);
            }

            CRef<objects::CDelta_seq> gap(new objects::CDelta_seq());
            if (!gaps[i].is_known) 
            {
                gap->SetLiteral().SetFuzz().SetLim(objects::CInt_fuzz::eLim_unk);
            }
            gap->SetLiteral().SetLength(gaps[i].length);
            inst.SetExt().SetDelta().Set().push_back(gap);               
            literal_start = gaps[i].start;
            if (gaps[i].is_replace)
                literal_start += gaps[i].length;
            else
                total_length += gaps[i].length;
        }
        if (literal_start < iupacna.size())
        {
            string literal = iupacna.substr(literal_start);
            CRef<objects::CDelta_seq> ds(new objects::CDelta_seq());
            ds->SetLiteral().SetSeq_data().SetIupacna().Set(literal);
            ds->SetLiteral().SetLength(literal.length());  
            inst.SetExt().SetDelta().Set().push_back(ds);
        }
        inst.SetLength(total_length);
        objects::edit::NormalizeUnknownLengthGaps(inst);             
        CRef<CCmdChangeBioseqInst> cmd(new CCmdChangeBioseqInst(bsh, bseq->SetInst()));
        composite->AddCommand(*cmd);                                   
    }
    auto local_cmd = composite;
    local_cmd->Execute();
    NRawToDeltaSeq::AdjustFeatureLocations(map_gaps, composite, false, m_AdjustCDS->GetValue(), false, create_general_only, max_feat_id, old_to_new, create_xref_map);
    local_cmd->Unexecute();
    UpdateDensegAlignment( m_TopSeqEntry, composite,  orig_gaps, ( m_CoordSeq->IsEnabled() && !m_CoordSeq->GetValue()));
      
    return composite;                   
}

void CRawSeqToDeltaSeqByLoc::UseAlignCoords(objects::CBioseq_Handle bsh, vector<NRawToDeltaSeq::SGap> &gaps)
{
    for (size_t i = 0; i < gaps.size(); i++)
    {
        int start = gaps[i].start;
        int stop = gaps[i].start + gaps[i].length - 1;
        int real_start = -1;
        int real_stop = -1;
        ComputeAlignCoords(bsh, start, stop, real_start, real_stop);
        if (real_start >= 0 && real_stop >= 0)
        {
            gaps[i].start = real_start;
            gaps[i].length = real_stop - real_start + 1;
        }
    }
}

void CRawSeqToDeltaSeqByLoc::ComputeAlignCoords(objects::CBioseq_Handle bsh, int before, int after, int &real_before, int &real_after)
{
    objects::CAlign_CI align_ci(bsh);
    if (align_ci)
    {
        const objects::CSeq_align& align = *align_ci;
        int row = CEditSeqEndsWithAlign::FindRow(bsh, align);
        if (row >= 0)
        {
            real_before = CEditSeqEndsWithAlign::FindPosition(align,row,before);
            if (real_before < 0)
            {
                before++;
                while (real_before < 0 && before < bsh.GetBioseqLength())
                {
                    real_before = CEditSeqEndsWithAlign::FindPosition(align,row,before);
                    before++;
                }
            }
            real_after = CEditSeqEndsWithAlign::FindPosition(align,row,after);
            if (real_after < 0)
            {
                after--;
                while (real_after < 0 && after >= 0)
                {
                    real_after = CEditSeqEndsWithAlign::FindPosition(align,row,after);
                    after--;
                }
            }
        }
    }
}


void CRawSeqToDeltaSeqByLoc::UpdateDensegAlignment( const CSeq_entry_Handle& tse, CRef<CCmdComposite> composite,  const vector<NRawToDeltaSeq::SGap> &gaps, const bool use_align_coords)
{
    for (objects::CAlign_CI align_ci(tse); align_ci; ++ align_ci)
    {
        const objects::CSeq_align& align = *align_ci;
        int num_rows = align.CheckNumRows();
        if (num_rows < 2)
            break;
        if (!align.IsSetSegs() || !align.GetSegs().IsDenseg() ||
            !align.GetSegs().GetDenseg().IsSetStarts() || !align.GetSegs().GetDenseg().IsSetLens())
            continue;

        vector< TSignedSeqPos > starts(align.GetSegs().GetDenseg().GetStarts()); // TODO take into account strand
        vector< TSeqPos > lens(align.GetSegs().GetDenseg().GetLens());

        vector< ENa_strand > strands;   
        vector<vector<vector< ENa_strand > > > all_new_strands;        
        if (align.GetSegs().GetDenseg().IsSetStrands())
            strands = align.GetSegs().GetDenseg().GetStrands();

        vector< CRef< CScore > > scores;
        vector< vector< CRef< CScore > > > all_new_scores;
        if (align.GetSegs().GetDenseg().IsSetScores())
            scores = align.GetSegs().GetDenseg().GetScores();

        vector< vector< vector<TSignedSeqPos> > >  all_new_starts;
        vector< vector<TSeqPos> >  all_new_lens;
       
        x_UpdateRows(num_rows, gaps, starts, lens, strands, scores, all_new_starts, all_new_lens, all_new_strands, all_new_scores, use_align_coords, align);

        // TODO use align coords
        CSeq_align_Handle ah = align_ci.GetSeq_align_Handle();
        if (all_new_lens.empty())
        {
            CRef<objects::CCmdDelSeq_align> cmd(new objects::CCmdDelSeq_align(ah));
            composite->AddCommand(*cmd);   
        }
        else
        {
            CRef<CSeq_align> new_align = x_GetNewAlign(0, align, num_rows, all_new_starts,  all_new_lens, all_new_strands, all_new_scores);
            CRef<CCmdChangeAlign> cmd (new CCmdChangeAlign(ah, *new_align));        
            composite->AddCommand(*cmd); 

            for (size_t block = 1; block < all_new_lens.size(); block++)
            {
                CRef<CSeq_align> add_align = x_GetNewAlign(block, align, num_rows, all_new_starts,  all_new_lens, all_new_strands, all_new_scores);
                CSeq_entry_Handle seh = ah.GetAnnot().GetParentEntry();
                CRef<CCmdCreateAlign> add_cmd(new CCmdCreateAlign(seh, *add_align));
                composite->AddCommand(*add_cmd); 
            }
        }
    }
}

void CRawSeqToDeltaSeqByLoc::x_UpdateRows(const size_t num_rows, const vector<NRawToDeltaSeq::SGap> &gaps, const vector< TSignedSeqPos > &orig_starts, const vector< TSeqPos > &lens,
                                          const vector< ENa_strand > &strands, const vector< CRef< CScore > > &scores,
                                          vector < vector< vector<TSignedSeqPos> > > &all_new_starts,  vector < vector<TSeqPos> > &all_new_lens,
                                          vector< vector< vector< ENa_strand > > > &all_new_strands, vector< vector< CRef< CScore > > > &all_new_scores,
                                          const bool use_align_coords, const objects::CSeq_align& align)
{
    vector< vector<TSignedSeqPos> > new_starts(num_rows);  
    vector<TSeqPos> new_lens;
    vector< vector< ENa_strand > > new_strands(num_rows);
    vector< CRef< CScore > > new_scores;

    vector< TSignedSeqPos > starts(orig_starts);
    vector< TSignedSeqPos > stops(orig_starts);
    size_t num_segs = lens.size();    
    for (size_t s = 0; s < num_segs; s++)
        for (size_t row = 0; row < num_rows; row++)
            if (starts[num_rows * s + row] != -1)
                stops[num_rows * s + row] += lens[s] - 1;
    
    vector<TSeqPos> offset(num_rows, 0);
    size_t i = 0;
    size_t j = 0;
    vector<bool> next_segment(num_rows, false);
    vector<bool> next_gap(num_rows, false);
    set<TSeqPos> break_aligns;
    set<TSeqPos> break_aligns_before;
    map<TSeqPos, vector<pair<TSeqPos, size_t> > > breaks;
    map<TSeqPos,TSeqPos> lengths;
    set<size_t> delete_segs;
    set<size_t> before_segs;
    while ( j < num_segs )
    {             
        for (size_t row = 0; row < num_rows; row++)
        {
            if (next_segment[row] || next_gap[row])
                continue;

            TSignedSeqPos orig_seg_start = orig_starts[num_rows*j+row];
            TSignedSeqPos seg_start = starts[num_rows*j+row];
            if (seg_start < 0)
            {
                next_segment[row] = true;
                continue;
            }
            TSignedSeqPos seg_stop = stops[num_rows*j+row];
            
            int gap_start = 0;
            if (i < gaps.size())
            {
                gap_start = gaps[i].start;
                if (use_align_coords)
                {
                    int total_len = 0;
                    for (size_t s = 0; s < lens.size(); s++)
                        total_len += lens[s];
                    int real_before = CEditSeqEndsWithAlign::FindPosition(align,row,gap_start);
                    if (real_before < 0)
                    {
                        gap_start++;
                        while (real_before < 0 && gap_start < total_len)
                        {
                            real_before = CEditSeqEndsWithAlign::FindPosition(align,row,gap_start);
                            gap_start++;
                        }
                    }
                    gap_start = real_before;
                }
            }

            if ( i >= gaps.size() || seg_stop < gap_start) // no inserts or replaces in this interval
            {
                next_segment[row] = true;
            }
            else if (!gaps[i].is_replace && gap_start <= seg_start) // insert before the interval
            {
                if (gaps[i].is_known)
                    offset[row] += gaps[i].length;
                else
                    offset[row] += 100;
                next_gap[row] = true;
            }
            else if (gaps[i].is_replace && gaps[i].is_known && gap_start + gaps[i].length <= seg_start) // known replace before the interval
            {
                next_gap[row] = true;
            }
            else if (gaps[i].is_replace && !gaps[i].is_known && gap_start + 100 <= seg_start) // unknown replace before the interval
            {
                offset[row] += 100 - gaps[i].length;
                next_gap[row] = true;
            }
            else  if (!gaps[i].is_replace && gaps[i].is_known && gap_start > seg_start && gap_start <= seg_stop) // known insert
            {
                TSeqPos new_stop = gap_start + offset[row] - 1;
                
                breaks[gap_start - orig_seg_start].push_back(pair<TSeqPos, size_t>(new_stop + 1, row));
                lengths[gap_start - orig_seg_start] = gaps[i].length;
                
                starts[num_rows*j+row] = gap_start; // no offset here because we are processing the current interval again.
                offset[row] += gaps[i].length;
                next_gap[row] = true;
            }
            else  if (!gaps[i].is_replace && !gaps[i].is_known && gap_start > seg_start && gap_start <= seg_stop) // unknown insert
            {
                TSeqPos new_stop = gap_start + offset[row] - 1;
                
                breaks[gap_start - orig_seg_start].push_back(pair<TSeqPos, size_t>(new_stop + 1, row) );
                lengths[gap_start - orig_seg_start] = 100;
                
                starts[num_rows*j+row] = gap_start; // no offset here because we are processing the current interval again.
                offset[row] += 100;
                next_gap[row] = true;
                break_aligns.insert(gap_start - orig_seg_start);
            }
            else  if (gaps[i].is_replace && gaps[i].is_known && gap_start >= seg_start && gap_start + gaps[i].length - 1 <= seg_stop) // known replace
            {
                next_gap[row] = true;
            }
            else  if (gaps[i].is_replace && !gaps[i].is_known && gap_start <= seg_start && gap_start + gaps[i].length - 1 < seg_stop) // unknown replace on the left edge
            {
                offset[row] += 100 - gaps[i].length;
                
                starts[num_rows*j+row] = gap_start + gaps[i].length; 
                next_gap[row] = true;
                if (j != 0)             // check if it is the very first interval, otherwise create a new feat
                {
                    break_aligns_before.insert(gap_start + gaps[i].length - orig_seg_start);
                }
            }
            else  if (gaps[i].is_replace && !gaps[i].is_known && gap_start <= seg_start && gap_start + gaps[i].length - 1 >= seg_stop) // unknown replace of the whole interval
            {
                next_segment[row] = true;
                break_aligns_before.insert(gap_start - orig_seg_start);
            }
            else  if (gaps[i].is_replace && !gaps[i].is_known && gap_start > seg_start  && gap_start <= seg_stop && gap_start + gaps[i].length - 1 > seg_stop) // unknown replace on the right edge
            {
                offset[row] += 100 - gaps[i].length;

                if (j != num_segs - 1 )             // check if it is the very last interval, otherwise create a new feat
                {
                    break_aligns_before.insert(gap_start - orig_seg_start);                    
                }
                next_segment[row] = true;
            }
            else  if (gaps[i].is_replace && !gaps[i].is_known && gap_start > seg_start  && gap_start <= seg_stop && gap_start + gaps[i].length - 1 <= seg_stop) // unknwon replace
            {
                offset[row] += 100 - gaps[i].length;

                starts[num_rows*j+row] = gap_start + gaps[i].length; // no offset here because we are processing the current interval again.
                next_gap[row] = true;
                break_aligns_before.insert(gap_start - orig_seg_start);
            }
            else
                next_gap[row] = true;
        }

        bool all_next_segment = true;
        bool all_next_gap = true;
        for (size_t row = 0; row < num_rows; row++)
        {
            all_next_segment &= next_segment[row];
            all_next_gap &= next_gap[row];
        }
        set<TSeqPos>::const_iterator bai = break_aligns_before.begin();
        if (all_next_segment)
        {
            vector< TSignedSeqPos > current_starts(orig_starts);
            TSeqPos prev_break = 0;
            for (map<TSeqPos, vector<pair<TSeqPos, size_t> > >::const_iterator b = breaks.begin(); b != breaks.end(); ++b)
            {
                TSeqPos start = b->first;
                for (size_t row = 0; row < num_rows; row++)
                {
                    TSignedSeqPos seg_start = current_starts[num_rows*j+row];
                    new_starts[row].push_back(seg_start);
                    current_starts[num_rows*j+row] += start - prev_break;
                    new_starts[row].push_back(-1);
                    if (!strands.empty())
                    {
                        new_strands[row].push_back(strands[num_rows*j+row]);
                        new_strands[row].push_back(strands[num_rows*j+row]);
                    }
                }
                new_lens.push_back(start - prev_break);
              
                if (!scores.empty())
                {
                    CRef<CScore> score(new CScore);
                    score->Assign(*scores[j]);
                    new_scores.push_back(score);                    
                }

                if (break_aligns.find(start) != break_aligns.end())
                    delete_segs.insert(new_lens.size());
                if (bai != break_aligns_before.end() && start > *bai)
                {
                    before_segs.insert(new_lens.size());
                    ++bai;
                }
                for (size_t r = 0; r < b->second.size(); r++)
                {
                    size_t row = b->second[r].second;
                    new_starts[row].back() = b->second[r].first;
                    current_starts[num_rows*j+row] += lengths[start];
                }
                new_lens.push_back(lengths[start]);
                if (!scores.empty())
                {
                    CRef<CScore> score(new CScore);
                    score->Assign(*scores[j]);
                    new_scores.push_back(score);                    
                }
                prev_break = start;                
            }

            TSignedSeqPos len = lens[j];
            len -= prev_break;
            if (len > 0)
            {
                for (size_t row = 0; row < num_rows; row++)
                {
                    TSignedSeqPos seg_start = current_starts[num_rows*j+row];
                    new_starts[row].push_back(seg_start);
                    if (!strands.empty())
                        new_strands[row].push_back(strands[num_rows*j+row]);
                }
                if (bai != break_aligns_before.end() && prev_break < *bai)
                {
                    before_segs.insert(new_lens.size());
                }
                new_lens.push_back(len);              
                if (!scores.empty())
                {
                    CRef<CScore> score(new CScore);
                    score->Assign(*scores[j]);
                    new_scores.push_back(score);                    
                }
            }

            breaks.clear();
            lengths.clear();
            break_aligns.clear();
            break_aligns_before.clear();
            if (all_next_segment)
            {
                for (size_t row = 0; row < num_rows; row++)
                    next_segment[row] = false;
                j++;
            }
        }

        if (all_next_gap)
        {
            for (size_t row = 0; row < num_rows; row++)
                next_gap[row] = false;
            i++;
        }
    }

    vector< vector<TSignedSeqPos> > block_starts(num_rows);  
    vector<TSeqPos> block_lens;
    vector< vector< ENa_strand > > block_strands(num_rows);
    vector< CRef< CScore > > block_scores;
    for (size_t seg = 0; seg < new_lens.size(); seg++)
    {
        if (delete_segs.find(seg) != delete_segs.end() || before_segs.find(seg) != before_segs.end())
        {
            all_new_starts.push_back(block_starts);
            all_new_lens.push_back(block_lens);
            all_new_strands.push_back(block_strands);
            all_new_scores.push_back(block_scores);
            block_starts.clear();
            block_starts.resize(num_rows);
            block_lens.clear();
            block_strands.clear();
            block_strands.resize(num_rows);
            block_scores.clear();
        }
        if (delete_segs.find(seg) != delete_segs.end())
            continue;
        for (size_t row = 0; row < num_rows; row++)
        {
            block_starts[row].push_back(new_starts[row][seg]);
            if (!strands.empty())
                block_strands[row].push_back(new_strands[row][seg]);
        }
        block_lens.push_back(new_lens[seg]);
        if (!scores.empty())
            block_scores.push_back(new_scores[seg]);
    }
    if (!block_lens.empty())
    {
        all_new_starts.push_back(block_starts);
        all_new_lens.push_back(block_lens);
        all_new_strands.push_back(block_strands);
        all_new_scores.push_back(block_scores);
    }
}

CRef<CSeq_align> CRawSeqToDeltaSeqByLoc::x_GetNewAlign(const size_t block, const objects::CSeq_align& align, const size_t num_rows, 
                                                       const vector < vector< vector<TSignedSeqPos> > > &all_new_starts,  const vector < vector<TSeqPos> > &all_new_lens,
                                                       const vector< vector< vector< ENa_strand > > > &all_new_strands, const vector< vector< CRef< CScore > > > &all_new_scores)
{
    vector< TSignedSeqPos > starts_vec(num_rows * all_new_lens[block].size());
    for (size_t i = 0; i < all_new_lens[block].size(); i++)
        for (size_t row = 0; row < num_rows; ++row)
            starts_vec[i * num_rows + row] = all_new_starts[block][row][i];
    
    CRef<CSeq_align> new_align(new CSeq_align);
    new_align->Assign(align);
    new_align->SetSegs().SetDenseg().SetStarts() = starts_vec;
    new_align->SetSegs().SetDenseg().SetLens() = all_new_lens[block];
    new_align->SetSegs().SetDenseg().SetNumseg(all_new_lens[block].size());
    if (!all_new_strands.empty() && all_new_strands[block].size() == num_rows && !all_new_strands[block].front().empty())
    {
        vector< ENa_strand > strands_vec(num_rows * all_new_lens[block].size());
        for (size_t i = 0; i < all_new_lens[block].size(); i++)
            for (size_t row = 0; row < num_rows; ++row)
                strands_vec[i * num_rows + row] = all_new_strands[block][row][i];
        new_align->SetSegs().SetDenseg().SetStrands() = strands_vec;
    }
    else
        new_align->SetSegs().SetDenseg().ResetStrands();
    
    if (!all_new_scores[block].empty())
        new_align->SetSegs().SetDenseg().SetScores() = all_new_scores[block];
    else
        new_align->SetSegs().SetDenseg().ResetScores();

    new_align->SetSegs().SetDenseg().TrimEndGaps();
    new_align->SetSegs().SetDenseg().RemovePureGapSegs();
    new_align->SetSegs().SetDenseg().Compact();
    return new_align;
}

CRef<CCmdComposite> CRawSeqToDeltaSeqByLoc::ByAssemblyGapFeatures(objects::CSeq_entry_Handle seh)
{
    map<CObject_id::TId, CObject_id::TId> old_to_new;
    bool create_xref_map = true;
    ByAssemblyGapFeatures_impl(seh, old_to_new, create_xref_map);
    create_xref_map = false;
    return ByAssemblyGapFeatures_impl(seh, old_to_new, create_xref_map);
}

CRef<CCmdComposite> CRawSeqToDeltaSeqByLoc::ByAssemblyGapFeatures_impl(objects::CSeq_entry_Handle seh, map<CObject_id::TId, CObject_id::TId> &old_to_new, bool create_xref_map)
{
    if (!seh) 
        return CRef<CCmdComposite>(nullptr);

    objects::CScope &scope = seh.GetScope();
    bool create_general_only = objects::edit::IsGeneralIdProtPresent(seh.GetTopLevelEntry());
    CObject_id::TId max_feat_id = CFixFeatureId::s_FindHighestFeatureId(seh.GetTopLevelEntry());
    CRef<CCmdComposite> composite(new CCmdComposite("Raw Seq to Delta Seq By Assembly_Gap Features")); 
    bool alignment_affected = false;
    vector<NRawToDeltaSeq::SGap> all_gaps;
    map<CBioseq_Handle, vector<NRawToDeltaSeq::SGap> > map_gaps;
    for( objects::CBioseq_CI bi(seh, objects::CSeq_inst::eMol_na); bi; ++bi)
    {
        objects::CBioseq_Handle bsh = *bi;
        TSeqPos seq_start = bsh.GetRangeSeq_loc(0,0)->GetStart(objects::eExtreme_Positional);
        vector<NRawToDeltaSeq::SGap> gaps;
        for (CFeat_CI feat_it(bsh, SAnnotSelector(CSeqFeatData::eSubtype_assembly_gap)); feat_it; ++feat_it) 
        {
            const CSeq_loc& feat_loc = feat_it->GetLocation();
            for(CSeq_loc_CI loc_it(feat_loc, CSeq_loc_CI::eEmpty_Skip, CSeq_loc_CI::eOrder_Positional); loc_it; ++loc_it)     
            {
                CSeq_loc_CI::TRange feat_range = loc_it.GetRange();
                NRawToDeltaSeq::SGap gap;
                gap.start = feat_range.GetFrom() - seq_start;
                gap.length =  feat_range.GetTo() - seq_start - gap.start + 1;
                gap.is_known = true; 
                if (feat_it->GetNamedQual("estimated_length") == "unknown")
                    gap.is_known = false; 
                gap.gap_type = objects::CSeq_gap::eType_unknown; 
                string gap_type_str = feat_it->GetNamedQual("gap_type");
                if (!gap_type_str.empty())
                {
                    if (objects::CSeq_gap::ENUM_METHOD_NAME(EType)()->IsValidName(gap_type_str))
                        gap.gap_type = objects::CSeq_gap::ENUM_METHOD_NAME(EType)()->FindValue(gap_type_str);
                    else
                    {
                        const objects::CSeq_gap::SGapTypeInfo *gap_type_info = objects::CSeq_gap::NameToGapTypeInfo(gap_type_str); // NameToGapTypeInfo does not recognize all gap types
                        if (gap_type_info)
                            gap.gap_type = gap_type_info->m_eType;
                    }
                }
                gap.linkage = objects::CSeq_gap::eLinkage_unlinked;
                gap.linkage_evidence = -1;
                string linkage_evidence_str = feat_it->GetNamedQual("linkage_evidence");
                if ( gap.gap_type == objects::CSeq_gap::eType_repeat || gap.gap_type == objects::CSeq_gap::eType_scaffold || gap.gap_type == objects::CSeq_gap::eType_contamination)
                {
                    gap.linkage = objects::CSeq_gap::eLinkage_linked;
                    if (!linkage_evidence_str.empty() && objects::CLinkage_evidence::ENUM_METHOD_NAME(EType)()->IsValidName(linkage_evidence_str))
                        gap.linkage_evidence = objects::CLinkage_evidence::ENUM_METHOD_NAME(EType)()->FindValue(linkage_evidence_str);
                }
                gap.is_replace = true;
                gaps.push_back(gap);
                all_gaps.push_back(gap);
            }
        }
        if (gaps.empty())
            continue;
        sort(gaps.begin(), gaps.end());
        map_gaps[bsh] = gaps;
        CRef<objects::CBioseq> bseq( new objects::CBioseq );
        bseq->Assign(*bsh.GetCompleteBioseq());                              
        objects::CSeq_inst& inst = bseq->SetInst();
        // can only convert if starting as raw
        if (!inst.IsSetRepr() || inst.GetRepr() != objects::CSeq_inst::eRepr_raw
            || !inst.IsSetSeq_data()) {
            continue;
        }
            
        string iupacna;
        switch(inst.GetSeq_data().Which()) {
        case objects::CSeq_data::e_Iupacna:
            iupacna = inst.GetSeq_data().GetIupacna();
            break;
        case objects::CSeq_data::e_Ncbi2na:
            CSeqConvert::Convert(inst.GetSeq_data().GetNcbi2na().Get(), CSeqUtil::e_Ncbi2na,
                                 0, inst.GetLength(), iupacna, CSeqUtil::e_Iupacna);
            break;
        case objects::CSeq_data::e_Ncbi4na:
            CSeqConvert::Convert(inst.GetSeq_data().GetNcbi4na().Get(), CSeqUtil::e_Ncbi4na,
                                 0, inst.GetLength(), iupacna, CSeqUtil::e_Iupacna);
            break;
        case objects::CSeq_data::e_Ncbi8na:
            CSeqConvert::Convert(inst.GetSeq_data().GetNcbi8na().Get(), CSeqUtil::e_Ncbi8na,
                                 0, inst.GetLength(), iupacna, CSeqUtil::e_Iupacna);
            break;
        default:
            return CRef<CCmdComposite>(NULL);
            break;
        }
        int literal_start = 0;
        inst.ResetSeq_data();
        inst.SetRepr(objects::CSeq_inst::eRepr_delta);
        int total_length = iupacna.length();
        if (inst.IsSetLength())
            total_length = inst.GetLength();
        for (size_t i = 0; i < gaps.size(); i++)
        {
            int literal_length = gaps[i].start - literal_start;
            if (literal_length < 0 || gaps[i].start > iupacna.size())
            {
                if (create_xref_map)
                    wxMessageBox(_("Inconsistent gap intervals"), _("Error in gap interval specification"), wxOK|wxICON_ERROR);
                return CRef<CCmdComposite>(nullptr);
            }
            if (literal_length > 0)
            {
                string literal = iupacna.substr(literal_start, literal_length);
                CRef<objects::CDelta_seq> ds(new objects::CDelta_seq());
                ds->SetLiteral().SetSeq_data().SetIupacna().Set(literal);
                ds->SetLiteral().SetLength(literal.length());  
                inst.SetExt().SetDelta().Set().push_back(ds);
            }

            CRef<objects::CDelta_seq> gap(new objects::CDelta_seq());
            if (!gaps[i].is_known) 
            {
                gap->SetLiteral().SetFuzz().SetLim(objects::CInt_fuzz::eLim_unk);
            }
            gap->SetLiteral().SetLength(gaps[i].length);
            gap->SetLiteral().SetSeq_data().SetGap().SetType(gaps[i].gap_type);
            gap->SetLiteral().SetSeq_data().SetGap().SetLinkage(gaps[i].linkage);
            if (gaps[i].linkage_evidence >= 0)
            {
                CRef<CLinkage_evidence> link_ev(new CLinkage_evidence);
                link_ev->SetType(gaps[i].linkage_evidence);
                gap->SetLiteral().SetSeq_data().SetGap().SetLinkage_evidence().push_back(link_ev);
            }

            inst.SetExt().SetDelta().Set().push_back(gap);               
            literal_start = gaps[i].start;
            if (gaps[i].is_replace)
                literal_start += gaps[i].length;
            else
                total_length += gaps[i].length;
        }
        if (literal_start < iupacna.size())
        {
            string literal = iupacna.substr(literal_start);
            CRef<objects::CDelta_seq> ds(new objects::CDelta_seq());
            ds->SetLiteral().SetSeq_data().SetIupacna().Set(literal);
            ds->SetLiteral().SetLength(literal.length());  
            inst.SetExt().SetDelta().Set().push_back(ds);
        }
        inst.SetLength(total_length);
        //objects::edit::NormalizeUnknownLengthGaps(inst);              
        CRef<CCmdChangeBioseqInst> cmd(new CCmdChangeBioseqInst(bsh, bseq->SetInst()));
        composite->AddCommand(*cmd);                       
            
        for (CFeat_CI feat_it(bsh, SAnnotSelector(CSeqFeatData::eSubtype_assembly_gap)); feat_it; ++feat_it) 
        {
            composite->AddCommand(*CRef<CCmdDelSeq_feat>(new CCmdDelSeq_feat(feat_it->GetSeq_feat_Handle())));        
        }
    }        
    auto local_cmd = composite;
    local_cmd->Execute();
    NRawToDeltaSeq::AdjustFeatureLocations(map_gaps, composite, false, true, false, create_general_only, max_feat_id, old_to_new, create_xref_map);
    local_cmd->Unexecute();
    UpdateDensegAlignment( seh, composite,  all_gaps, false);
       
    return composite;              
}

END_NCBI_SCOPE
