/*  $Id: edit_seq_id.cpp 42542 2019-03-20 14:20:46Z filippov $
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
#include <objects/seqset/Seq_entry.hpp>
#include <objmgr/feat_ci.hpp>
#include <objmgr/align_ci.hpp>
#include <objmgr/graph_ci.hpp>
#include <objmgr/annot_ci.hpp>
#include <objmgr/bioseq_ci.hpp>
#include <objmgr/seqdesc_ci.hpp>
#include <objects/general/Object_id.hpp>
#include <objects/general/Dbtag.hpp>
#include <objtools/edit/cds_fix.hpp>
#include <gui/objutils/cmd_change_seq_entry.hpp>
#include <gui/objutils/cmd_change_seq_feat.hpp>
#include <gui/objutils/cmd_change_align.hpp>
#include <gui/objutils/cmd_change_graph.hpp>
#include <gui/objutils/cmd_del_desc.hpp>
#include <objects/seq/seq_macros.hpp>
#include <objects/seqset/seqset_macros.hpp>
#include <objects/seqalign/seqalign_macros.hpp>
#include <objects/seqfeat/RNA_ref.hpp>
#include <objects/seqfeat/Trna_ext.hpp>
#include <objects/seqfeat/seqfeat_macros.hpp>
#include <objects/seqtable/Seq_table.hpp>
#include <objects/seqtable/SeqTable_column.hpp>
#include <objects/seqtable/SeqTable_multi_data.hpp>
#include <objects/seqtable/SeqTable_single_data.hpp>
#include <objects/general/general_macros.hpp>
#include <gui/packages/pkg_sequence_edit/parse_text_options_dlg.hpp>
#include <gui/packages/pkg_sequence_edit/edit_seq_id.hpp>


#include <wx/sizer.h>
#include <wx/button.h>

////@begin XPM images
////@end XPM images

BEGIN_NCBI_SCOPE

USING_SCOPE(ncbi::objects);

/*!
 * CEditSeqId type definition
 */

IMPLEMENT_DYNAMIC_CLASS( CEditSeqId, wxDialog )


/*!
 * CEditSeqId event table definition
 */

BEGIN_EVENT_TABLE( CEditSeqId, wxDialog )

////@begin CEditSeqId event table entries


////@end CEditSeqId event table entries

END_EVENT_TABLE()


/*!
 * CEditSeqId constructors
 */

CEditSeqId::CEditSeqId()
{
    Init();
}

CEditSeqId::CEditSeqId( wxWindow* parent, CSeq_entry_Handle seh, wxWindowID id, const wxString& caption, const wxPoint& pos, const wxSize& size, long style )
    : CModifySeqId(seh, "Edit Seq-id")
{
    Init();
    Create(parent, id, caption, pos, size, style);
}


/*!
 * CEditSeqId creator
 */

bool CEditSeqId::Create( wxWindow* parent, wxWindowID id, const wxString& caption, const wxPoint& pos, const wxSize& size, long style )
{
////@begin CEditSeqId creation
    SetExtraStyle(wxWS_EX_BLOCK_EVENTS);
    wxDialog::Create( parent, id, caption, pos, size, style );

    CreateControls();
    if (GetSizer())
    {
        GetSizer()->SetSizeHints(this);
    }
    Centre();
////@end CEditSeqId creation
    return true;
}


/*!
 * CEditSeqId destructor
 */

CEditSeqId::~CEditSeqId()
{
////@begin CEditSeqId destruction
////@end CEditSeqId destruction
}


/*!
 * Member initialisation
 */

void CEditSeqId::Init()
{
////@begin CEditSeqId member initialisation
    m_RemoveOrig = NULL;
////@end CEditSeqId member initialisation
}


/*!
 * Control creation for CEditSeqId
 */

void CEditSeqId::CreateControls()
{    
////@begin CEditSeqId content construction
    CEditSeqId* itemDialog1 = this;

    wxBoxSizer* itemBoxSizer2 = new wxBoxSizer(wxVERTICAL);
    itemDialog1->SetSizer(itemBoxSizer2);

    wxStaticBox* itemStaticBoxSizer3Static = new wxStaticBox(itemDialog1, wxID_ANY, _("What part of Seq-id do you want to edit?"));
    wxStaticBoxSizer* itemStaticBoxSizer3 = new wxStaticBoxSizer(itemStaticBoxSizer3Static, wxVERTICAL);
    itemBoxSizer2->Add(itemStaticBoxSizer3, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

    m_Local = new wxRadioButton( itemStaticBoxSizer3->GetStaticBox(), wxID_ANY, _("Local ID"), wxDefaultPosition, wxDefaultSize, wxRB_GROUP );
    m_Local->SetValue(true);
    itemStaticBoxSizer3->Add(m_Local, 0, wxALIGN_LEFT|wxALL, 5);

    m_GeneralDb = new wxRadioButton( itemStaticBoxSizer3->GetStaticBox(), wxID_ANY, _("General ID db"), wxDefaultPosition, wxDefaultSize, 0 );
    m_GeneralDb->SetValue(false);
    itemStaticBoxSizer3->Add(m_GeneralDb, 0, wxALIGN_LEFT|wxALL, 5);

    m_GeneralTag = new wxRadioButton( itemStaticBoxSizer3->GetStaticBox(), wxID_ANY, _("General ID tag"), wxDefaultPosition, wxDefaultSize, 0 );
    m_GeneralTag->SetValue(false);
    itemStaticBoxSizer3->Add(m_GeneralTag, 0, wxALIGN_LEFT|wxALL, 5);

    wxBoxSizer* itemBoxSizer7 = new wxBoxSizer(wxHORIZONTAL);
    itemBoxSizer2->Add(itemBoxSizer7, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

    m_Any = new wxRadioButton( itemDialog1, wxID_ANY, _("Any sequence"), wxDefaultPosition, wxDefaultSize, wxRB_GROUP );
    m_Any->SetValue(true);
    itemBoxSizer7->Add(m_Any, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    m_Nuc = new wxRadioButton( itemDialog1, wxID_ANY, _("Nucleotides"), wxDefaultPosition, wxDefaultSize, 0 );
    m_Nuc->SetValue(false);
    itemBoxSizer7->Add(m_Nuc, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    m_DNA = new wxRadioButton( itemDialog1, wxID_ANY, _("DNA"), wxDefaultPosition, wxDefaultSize, 0 );
    m_DNA->SetValue(false);
    itemBoxSizer7->Add(m_DNA, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    m_RNA = new wxRadioButton( itemDialog1, wxID_ANY, _("RNA"), wxDefaultPosition, wxDefaultSize, 0 );
    m_RNA->SetValue(false);
    itemBoxSizer7->Add(m_RNA, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    m_Proteins = new wxRadioButton( itemDialog1, wxID_ANY, _("Proteins"), wxDefaultPosition, wxDefaultSize, 0 );
    m_Proteins->SetValue(false);
    itemBoxSizer7->Add(m_Proteins, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxBoxSizer* itemBoxSizer12 = new wxBoxSizer(wxHORIZONTAL);
    itemBoxSizer2->Add(itemBoxSizer12, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

    wxStaticText* itemStaticText11 = new wxStaticText( itemDialog1, wxID_STATIC, _("Where Sequence ID"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer12->Add(itemStaticText11, 0, wxALIGN_TOP|wxALL, 5);

    m_StringConstraintPanel = new CStringConstraintPanel( itemDialog1, false, wxID_ANY, wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer12->Add(m_StringConstraintPanel, 0, wxALIGN_TOP|wxALL|wxFIXED_MINSIZE, 0);
    m_StringConstraintPanel->SetStringSelection(_("Is one of"));

    wxStaticText* itemStaticText12 = new wxStaticText( itemDialog1, wxID_STATIC, _("Keep"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer2->Add(itemStaticText12, 0, wxALIGN_LEFT|wxALL, 5);

    m_ParseOptions = new CParseTextOptionsDlg( itemDialog1, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxSIMPLE_BORDER );
    itemBoxSizer2->Add(m_ParseOptions, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

    wxBoxSizer* itemBoxSizer13 = new wxBoxSizer(wxHORIZONTAL);
    itemBoxSizer2->Add(itemBoxSizer13, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

    wxStaticText* itemStaticText14 = new wxStaticText( itemDialog1, wxID_STATIC, _("Add Prefix"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer13->Add(itemStaticText14, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    m_Prefix = new wxTextCtrl( itemDialog1, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer13->Add(m_Prefix, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxBoxSizer* itemBoxSizer16 = new wxBoxSizer(wxHORIZONTAL);
    itemBoxSizer2->Add(itemBoxSizer16, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

    wxStaticText* itemStaticText17 = new wxStaticText( itemDialog1, wxID_STATIC, _("Add Suffix"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer16->Add(itemStaticText17, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    m_Suffix = new wxTextCtrl( itemDialog1, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer16->Add(m_Suffix, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);


    wxBoxSizer* itemBoxSizer6 = new wxBoxSizer(wxHORIZONTAL);
    itemBoxSizer2->Add(itemBoxSizer6, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

    m_RemoveOrig = new wxCheckBox( itemDialog1, wxID_ANY, _("Remove original id"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer6->Add(m_RemoveOrig, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);
    m_RemoveOrig->SetValue(true);

    wxButton* itemButton7 = new wxButton( itemDialog1, wxID_OK, _("Accept"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer6->Add(itemButton7, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxButton* itemButton8 = new wxButton( itemDialog1, wxID_CANCEL, _("Cancel"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer6->Add(itemButton8, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

////@end CEditSeqId content construction

}


/*!
 * Should we show tooltips?
 */

bool CEditSeqId::ShowToolTips()
{
    return true;
}

/*!
 * Get bitmap resources
 */

wxBitmap CEditSeqId::GetBitmapResource( const wxString& name )
{
    // Bitmap retrieval
////@begin CEditSeqId bitmap retrieval
    wxUnusedVar(name);
    return wxNullBitmap;
////@end CEditSeqId bitmap retrieval
}

/*!
 * Get icon resources
 */

wxIcon CEditSeqId::GetIconResource( const wxString& name )
{
    // Icon retrieval
////@begin CEditSeqId icon retrieval
    wxUnusedVar(name);
    return wxNullIcon;
////@end CEditSeqId icon retrieval
}

void CEditSeqId::x_CollectChangedIds()
{
    m_remove_ids.clear();
    m_old_to_new_id.clear();
    for (CBioseq_CI b_iter(m_TopSeqEntry); b_iter; ++b_iter) 
    {
        CBioseq_Handle bsh = *b_iter;

        bool pass = false;
        if (m_Any->GetValue())
            pass = true;
        if (m_Nuc->GetValue() && bsh.IsSetInst_Mol() && 
            (bsh.GetInst_Mol() == CSeq_inst::eMol_na ||
             bsh.GetInst_Mol() == CSeq_inst::eMol_dna || 
             bsh.GetInst_Mol() == CSeq_inst::eMol_rna) )
            pass = true;
        if (m_DNA->GetValue() && bsh.IsSetInst_Mol() && bsh.GetInst_Mol() == CSeq_inst::eMol_dna)
            pass = true;
        if (m_RNA->GetValue() && bsh.IsSetInst_Mol() && bsh.GetInst_Mol() == CSeq_inst::eMol_rna)
            pass = true;
        if (m_Proteins->GetValue() && bsh.IsSetInst_Mol() && bsh.GetInst_Mol() == CSeq_inst::eMol_aa)
            pass = true;
        
        if (!pass)
            continue;

        for (CBioseq_Handle::TId::const_iterator id_it = b_iter->GetId().begin(); id_it != b_iter->GetId().end(); ++id_it)
        {

            CConstRef<CSeq_id> id = id_it->GetSeqId();
      
            CRef<edit::CStringConstraint> constraint(m_StringConstraintPanel->GetStringConstraint());
            CRef<CSeq_id> new_id(new CSeq_id);
            new_id->Assign(*id);
            if (m_Local->GetValue() && new_id->IsLocal())
            {
                string local; 
                if (new_id->GetLocal().IsStr())
                    local = new_id->GetLocal().GetStr();
                else if (new_id->GetLocal().IsId())
                {
                    local = NStr::IntToString(new_id->GetLocal().GetId());
                }
                if (constraint && !constraint->DoesTextMatch(local))
                    continue;
                m_remove_ids.insert(local);
                local = m_ParseOptions->GetSelectedText(local);
                local = m_Prefix->GetValue().ToStdString() + local + m_Suffix->GetValue().ToStdString();
                new_id->SetLocal().SetStr(local);
            }
            if (m_GeneralDb->GetValue() && new_id->IsGeneral() && new_id->GetGeneral().IsSetDb())
            {
                if (new_id->GetGeneral().IsSkippable())
                    continue;
                string db = new_id->GetGeneral().GetDb();
                if (constraint && !constraint->DoesTextMatch(db))
                    continue;
                db = m_ParseOptions->GetSelectedText(db);
                db = m_Prefix->GetValue().ToStdString() + db + m_Suffix->GetValue().ToStdString();
                new_id->SetGeneral().SetDb(db);
            }
            if (m_GeneralTag->GetValue() && new_id->IsGeneral() && new_id->GetGeneral().IsSetTag())
            {
                if (new_id->GetGeneral().IsSkippable())
                    continue;
                string tag;
                if (new_id->GetGeneral().GetTag().IsStr())
                    tag = new_id->GetGeneral().GetTag().GetStr();
                else if (new_id->GetGeneral().GetTag().IsId())
                {
                    tag = NStr::IntToString(new_id->GetGeneral().GetTag().GetId());
                }
                if (constraint && !constraint->DoesTextMatch(tag))
                    continue;
                tag = m_ParseOptions->GetSelectedText(tag);
                tag = m_Prefix->GetValue().ToStdString() + tag + m_Suffix->GetValue().ToStdString();
                new_id->SetGeneral().ResetTag();
                new_id->SetGeneral().SetTag().SetStr(tag);
            }
            m_old_to_new_id.push_back(make_pair(id, new_id));
        }
    }    
}       

CRef<CCmdComposite> CEditSeqId::GetCommand()
{
    x_CollectChangedIds();
    return CModifySeqId::GetCommand();
}

CRef<CCmdComposite> CEditSeqId::GetRemoveOriginalIdCommand()
{
    CRef<CCmdComposite> cmd;
    if (m_RemoveOrig->GetValue())
    {
        cmd.Reset(new CCmdComposite("Remove Original Local Ids"));   
        x_GetRemoveOriginalIdCommand(cmd);
    }
    return cmd;
}

void CModifySeqId::x_GetRemoveOriginalIdCommand(CRef<CCmdComposite> cmd)
{
    for (CBioseq_CI b_iter(m_TopSeqEntry); b_iter; ++b_iter)
    {
        for (CSeqdesc_CI desc_ci(*b_iter, CSeqdesc::e_User, 1); desc_ci; ++desc_ci) 
        {
            if (desc_ci->GetUser().IsSetType() && desc_ci->GetUser().GetType().IsStr()
                && NStr::EqualNocase(desc_ci->GetUser().GetType().GetStr(), "OriginalID") )
            {
                bool found(false);
                FOR_EACH_USERFIELD_ON_USEROBJECT(field_iter, desc_ci->GetUser())
                {
                    const CUser_field& field = **field_iter;
                    if (field.GetLabel().IsStr() && NStr::Equal(field.GetLabel().GetStr(), "LocalId") && field.IsSetData() && field.GetData().IsStr())
                    {
                        string value = field.GetData().GetStr();
                        if (m_remove_ids.find(value) != m_remove_ids.end())
                        {
                            found = true;
                            break;
                        }
                    }
                }
                if (found)
                {
                    CSeq_entry_Handle seh = desc_ci.GetSeq_entry_Handle();
                    cmd->AddCommand( *CRef<CCmdDelDesc>(new CCmdDelDesc(seh, *desc_ci)) );
                }
            }
        }
    }
}

CRef<CCmdComposite> CModifySeqId::GetCommand()
{
    CRef<CCmdComposite> cmd(new CCmdComposite(m_cmd_name));   
    for (CSeq_entry_CI seq_it(m_TopSeqEntry, CSeq_entry_CI::fRecursive | CSeq_entry_CI::fIncludeGivenEntry); seq_it; ++seq_it)
    {
        CSeq_entry_Handle seh = *seq_it;
        
        CRef<CSeq_entry> new_entry(new CSeq_entry);
        if (seh.IsSeq())
        {
            CRef<CBioseq> new_seq(new CBioseq);
            new_seq->Assign(*seh.GetSeq().GetBioseqCore());
            x_UpdateBioseq(new_seq);
            EDIT_EACH_SEQANNOT_ON_BIOSEQ(annot_it, *new_seq)
            {
                x_UpdateSeqAnnot(**annot_it); 
            }
            if (new_seq->IsSetInst() && new_seq->GetInst().IsSetHist())
            {
                CSeq_hist &hist = new_seq->SetInst().SetHist();
                if (hist.IsSetAssembly())
                {
                    for (CSeq_hist::TAssembly::iterator align_it = hist.SetAssembly().begin(); align_it != hist.SetAssembly().end(); ++align_it)
                    {
                        x_UpdateSeqAlign(**align_it);
                    }
                }
                if (hist.IsSetReplaces() && hist.GetReplaces().IsSetIds())
                {
                    for (CSeq_hist_rec::TIds::iterator id_it = hist.SetReplaces().SetIds().begin(); id_it != hist.SetReplaces().SetIds().end(); ++id_it)
                    {
                        x_ChangeId(**id_it); 
                    }
                }
                if (hist.IsSetReplaced_by() && hist.GetReplaced_by().IsSetIds())
                {
                    for (CSeq_hist_rec::TIds::iterator id_it = hist.SetReplaced_by().SetIds().begin(); id_it != hist.SetReplaced_by().SetIds().end(); ++id_it)
                    {
                        x_ChangeId(**id_it); 
                    }
                }
            }
            if (new_seq->IsSetInst() && new_seq->GetInst().IsSetExt())
            {
                CSeq_ext &ext = new_seq->SetInst().SetExt();
                if (ext.IsSeg() && ext.GetSeg().IsSet())
                {
                    for (CSeg_ext::Tdata::iterator loc_it = ext.SetSeg().Set().begin(); loc_it != ext.SetSeg().Set().end(); ++loc_it)
                    {
                        x_UpdateLoc(**loc_it);
                    }
                }
                if (ext.IsRef())
                {
                    x_UpdateLoc(ext.SetRef().Set());
                }
                if (ext.IsMap() && ext.GetMap().IsSet())
                {
                    for (CMap_ext::Tdata::iterator feat_it = ext.SetMap().Set().begin(); feat_it != ext.SetMap().Set().end(); ++feat_it)
                    {
                        x_UpdateFeat(**feat_it);
                    }
                }
                if (ext.IsDelta() && ext.GetDelta().IsSet())
                {
                    for (CDelta_ext::Tdata::iterator delta_it = ext.SetDelta().Set().begin(); delta_it != ext.SetDelta().Set().end(); ++delta_it)
                    {
                        if ((*delta_it)->IsLoc())
                        {
                            x_UpdateLoc((*delta_it)->SetLoc());
                        }
                    }
                }
            }
            new_entry->SetSeq(*new_seq);
            cmd->AddCommand(*CRef<CCmdChangeSeqEntry>(new CCmdChangeSeqEntry(seh, new_entry))); 
        }
        else
        {
            SAnnotSelector sel;
            for (CAnnot_CI annot_ci(seh, sel); annot_ci; ++annot_ci)
            {
                if (annot_ci->IsFtable())
                {
                    for (CFeat_CI feat_it(*annot_ci); feat_it; ++feat_it)
                    {
                        CRef<CSeq_feat> new_feat(new CSeq_feat);
                        new_feat->Assign(feat_it->GetOriginalFeature());
                        x_UpdateFeat(*new_feat);
                        cmd->AddCommand(*CRef<CCmdChangeSeq_feat>(new CCmdChangeSeq_feat(feat_it->GetSeq_feat_Handle(), *new_feat)));
                    }
                }
                if (annot_ci->IsAlign())
                {
                    for (CAlign_CI align_it(*annot_ci); align_it; ++align_it)
                    {
                        CRef<CSeq_align> new_align(new CSeq_align);
                        new_align->Assign(align_it.GetOriginalSeq_align());
                        x_UpdateSeqAlign(*new_align);
                        cmd->AddCommand(*CRef<CCmdChangeAlign>(new CCmdChangeAlign(align_it.GetSeq_align_Handle(), *new_align)));
                    }
                }
                if (annot_ci->IsGraph())
                {
                    for (CGraph_CI graph_it(*annot_ci); graph_it; ++graph_it)
                    {
                        CRef<CSeq_graph> new_graph(new CSeq_graph);
                        new_graph->Assign(graph_it->GetOriginalGraph());
                        if (new_graph->IsSetLoc())
                        {
                            CSeq_loc &loc = new_graph->SetLoc();
                            x_UpdateLoc(loc);
                        }
                        cmd->AddCommand(*CRef<CCmdChangeGraph>(new CCmdChangeGraph(graph_it->GetSeq_graph_Handle(), *new_graph)));
                    }
                }
            }
        }        
    }
    return cmd;
}

void CModifySeqId::x_UpdateBioseq(CRef<CBioseq> new_seq)
{
    if (new_seq->IsSetId())
    {
        for (CBioseq::TId::iterator it = new_seq->SetId().begin(); it != new_seq->SetId().end(); ++it)
        {
            CRef<CSeq_id> id = *it;
            x_ChangeId(*id);           
        }
    }
}

void CModifySeqId::x_UpdateFeat(CSeq_feat &feat)
{
    if (feat.IsSetLocation())
        {
            x_UpdateLoc( feat.SetLocation());
        }
        if (feat.IsSetProduct())
        {
            x_UpdateLoc(feat.SetProduct());
        }

        if (feat.IsSetData() && feat.GetData().IsRna() && feat.GetData().GetRna().IsSetExt() && 
            feat.GetData().GetRna().GetExt().IsTRNA() && feat.GetData().GetRna().GetExt().GetTRNA().IsSetAnticodon())
        {
            x_UpdateLoc(feat.SetData().SetRna().SetExt().SetTRNA().SetAnticodon());
        }
        
        if (feat.IsSetData() && feat.GetData().IsCdregion())
        {
            EDIT_EACH_CODEBREAK_ON_CDREGION(codebreak, feat.SetData().SetCdregion())
            {
                if ((*codebreak)->IsSetLoc())
                {
                    x_UpdateLoc((*codebreak)->SetLoc());
                }
            }
        }

        if (feat.IsSetData() && feat.GetData().IsClone() && feat.GetData().GetClone().IsSetClone_seq())
        {
            CClone_seq_set &clone_set = feat.SetData().SetClone().SetClone_seq();
            if (clone_set.IsSet())
            {
                for (CClone_seq_set::Tdata::iterator clone_it = clone_set.Set().begin(); clone_it != clone_set.Set().end(); ++clone_it)
                {
                        if ((*clone_it)->IsSetLocation())
                        {
                            x_UpdateLoc((*clone_it)->SetLocation());
                        }
                        if ((*clone_it)->IsSetSeq())
                        {
                            x_UpdateLoc((*clone_it)->SetSeq());
                        }
                    }
            }
        }
        if (feat.IsSetData() && feat.GetData().IsVariation() )
        {
            CVariation_ref &var = feat.SetData().SetVariation();
            x_UpdateVariation(var);
        }
        if (feat.IsSetSupport())
        {
            if (feat.GetSupport().IsSetInference())
            {
                for(CSeqFeatSupport::TInference::iterator inf_it = feat.SetSupport().SetInference().begin(); inf_it != feat.SetSupport().SetInference().end(); ++inf_it)
                {
                    if ((*inf_it)->IsSetBasis() && (*inf_it)->GetBasis().IsSetAccessions())
                    {
                        for (CEvidenceBasis::TAccessions::iterator id_it = (*inf_it)->SetBasis().SetAccessions().begin(); id_it != (*inf_it)->SetBasis().SetAccessions().end(); ++id_it)
                        {
                            x_ChangeId(**id_it); 
                        }
                    }
                }
            }
            if (feat.GetSupport().IsSetModel_evidence())
            {
                for(CSeqFeatSupport::TModel_evidence::iterator mod_it = feat.SetSupport().SetModel_evidence().begin(); mod_it != feat.SetSupport().SetModel_evidence().end(); ++mod_it)
                {
                    if ((*mod_it)->IsSetIdentification())
                    {
                        x_ChangeId((*mod_it)->SetIdentification());
                    }
                    if ((*mod_it)->IsSetMrna())
                    {
                        for (CModelEvidenceSupport::TMrna::iterator item_it = (*mod_it)->SetMrna().begin(); item_it != (*mod_it)->SetMrna().end(); ++item_it)
                        {
                            if ((*item_it)->IsSetId())
                            {
                                x_ChangeId((*item_it)->SetId());
                            }
                        }
                    }
                    if ((*mod_it)->IsSetEst())
                    {
                        for (CModelEvidenceSupport::TEst::iterator item_it = (*mod_it)->SetEst().begin(); item_it != (*mod_it)->SetEst().end(); ++item_it)
                        {
                            if ((*item_it)->IsSetId())
                            {
                                x_ChangeId((*item_it)->SetId());
                            }
                        }
                    }
                    if ((*mod_it)->IsSetProtein())
                    {
                        for (CModelEvidenceSupport::TEst::iterator item_it = (*mod_it)->SetProtein().begin(); item_it != (*mod_it)->SetProtein().end(); ++item_it)
                        {
                            if ((*item_it)->IsSetId())
                            {
                                x_ChangeId((*item_it)->SetId());
                            }
                        }
                    }
                }
            }
        }

}

void  CModifySeqId::x_UpdateVariation(CVariation_ref &var)
{
    if (var.IsSetData() && var.GetData().IsInstance() && var.GetData().GetInstance().IsSetDelta())
    {
        for (CVariation_inst::TDelta::iterator delta_it = var.SetData().SetInstance().SetDelta().begin(); delta_it != var.SetData().SetInstance().SetDelta().end(); ++delta_it)
        {
            if ((*delta_it)->IsSetSeq() && (*delta_it)->GetSeq().IsLoc())
            {
                x_UpdateLoc((*delta_it)->SetSeq().SetLoc());
            }
        }
    }
    if (var.IsSetData() && var.GetData().IsSet() && var.GetData().GetSet().IsSetVariations())
    {
        for ( CVariation_ref::C_Data::C_Set::TVariations::iterator var_it = var.SetData().SetSet().SetVariations().begin(); var_it != var.SetData().SetSet().SetVariations().end(); ++var_it)
        {
            x_UpdateVariation(**var_it);
        }
    }
    if (var.IsSetConsequence())
    {
        for (CVariation_ref::TConsequence::iterator con_it = var.SetConsequence().begin(); con_it != var.SetConsequence().end(); ++con_it)
        {
            if ((*con_it)->IsVariation())
            {
                x_UpdateVariation((*con_it)->SetVariation());
            }
        }
    }
}

void CModifySeqId::x_UpdateSeqAnnot(CSeq_annot &annot)
{
    EDIT_EACH_SEQFEAT_ON_SEQANNOT(feat_it, annot)
    {
        x_UpdateFeat(**feat_it);
    }

    EDIT_EACH_SEQALIGN_ON_SEQANNOT(align_it, annot)
    {
        x_UpdateSeqAlign(**align_it);
    }

    if (annot.IsSetData() && annot.GetData().IsGraph())
    {
        for (CSeq_annot::C_Data::TGraph::iterator graph_it = annot.SetData().SetGraph().begin();  graph_it != annot.SetData().SetGraph().end(); ++graph_it)
        {
            if ((*graph_it)->IsSetLoc())
            {
                CSeq_loc &loc = (*graph_it)->SetLoc();
                x_UpdateLoc(loc);
            }
        }
    }

    if (annot.IsSetData() && annot.GetData().IsIds())
    {
        for (CSeq_annot::C_Data::TIds::iterator id_it = annot.SetData().SetIds().begin();  id_it != annot.SetData().SetIds().end(); ++id_it)
        {
            CRef<CSeq_id> id = *id_it;
            x_ChangeId(*id);             
        }
    }

    if (annot.IsSetData() && annot.GetData().IsLocs())
    {
        for (CSeq_annot::C_Data::TLocs::iterator loc_it = annot.SetData().SetLocs().begin();  loc_it != annot.SetData().SetLocs().end(); ++loc_it)
        {
            x_UpdateLoc(**loc_it);
        }
    }

    EDIT_EACH_ANNOTDESC_ON_SEQANNOT(annotdesc_it, annot)
    {
        if ((*annotdesc_it)->IsSrc())
        {
            CSeq_id &id = (*annotdesc_it)->SetSrc();
            x_ChangeId(id);
        }
        if ((*annotdesc_it)->IsRegion())
        {
            CSeq_loc &loc = (*annotdesc_it)->SetRegion();
            x_UpdateLoc(loc);
        }
    }


    if (annot.IsSetData() && annot.GetData().IsSeq_table() && annot.GetData().GetSeq_table().IsSetColumns() )
    {
        for (CSeq_table::TColumns::iterator col_it = annot.SetData().SetSeq_table().SetColumns().begin(); col_it != annot.SetData().SetSeq_table().SetColumns().end(); ++col_it)
        {
            if ((*col_it)->IsSetData() && (*col_it)->GetData().IsLoc())
            {
                for (CSeqTable_multi_data::TLoc::iterator loc_it = (*col_it)->SetData().SetLoc().begin(); loc_it != (*col_it)->SetData().SetLoc().end(); ++loc_it)
                {
                    x_UpdateLoc(**loc_it);
                }
            }
            if ((*col_it)->IsSetData() && (*col_it)->GetData().IsId())
            {
                for (CSeqTable_multi_data::TId::iterator id_it = (*col_it)->SetData().SetId().begin(); id_it != (*col_it)->SetData().SetId().end(); ++id_it)
                {
                    x_ChangeId(**id_it); 
                }
            }
            if ((*col_it)->IsSetDefault() && (*col_it)->GetDefault().IsLoc())
            {
                x_UpdateLoc((*col_it)->SetDefault().SetLoc());
            }
            if ((*col_it)->IsSetDefault() && (*col_it)->GetDefault().IsId())
            {
                x_ChangeId((*col_it)->SetDefault().SetId());
            }
            if ((*col_it)->IsSetSparse_other() && (*col_it)->GetSparse_other().IsLoc())
            {
                x_UpdateLoc((*col_it)->SetSparse_other().SetLoc());
            }
            if ((*col_it)->IsSetSparse_other() && (*col_it)->GetSparse_other().IsId())
            {
                x_ChangeId((*col_it)->SetSparse_other().SetId());
            }
        }
    }
}

void CModifySeqId::x_UpdateSeqAlign(CSeq_align &align)
{
    if (align.IsSetSegs() && align.GetSegs().IsDendiag())
    {
        EDIT_EACH_DENDIAG_ON_SEQALIGN(diag_it, align)
        {
            EDIT_EACH_SEQID_ON_DENDIAG(id_it, **diag_it)
            {
                x_ChangeId(**id_it);  
            }
        }
    }
    if (align.IsSetSegs() && align.GetSegs().IsDenseg())
    {
        EDIT_EACH_SEQID_ON_DENSEG(id_it, align.SetSegs().SetDenseg())
        {
            x_ChangeId(**id_it);
        }
    }
    if (align.IsSetSegs() && align.GetSegs().IsStd())
    {
        EDIT_EACH_STDSEG_ON_SEQALIGN(std_it, align)
        {
                if ((*std_it)->IsSetIds())
                {
                    for (CStd_seg::TIds::iterator id_it = (*std_it)->SetIds().begin(); id_it != (*std_it)->SetIds().end(); ++id_it)
                    {
                        x_ChangeId(**id_it);
                    }
                }
                if ((*std_it)->IsSetLoc())
                {
                    for (CStd_seg::TLoc::iterator loc_it = (*std_it)->SetLoc().begin(); loc_it != (*std_it)->SetLoc().end(); ++loc_it)
                    {
                        x_UpdateLoc(**loc_it);
                    }
                }
        }
    }
    EDIT_EACH_RECURSIVE_SEQALIGN_ON_SEQALIGN(align_it, align)
    {
        x_UpdateSeqAlign(**align_it);
    }
}

void CModifySeqId::x_UpdateLoc(CSeq_loc &loc)
{
    CRef<CSeq_loc> new_loc(new CSeq_loc);
    new_loc->Assign(loc);
    CRef<CSeq_id> id(new CSeq_id);
    id->Assign(*(new_loc->GetId()));
    x_ChangeId(*id);
    new_loc->SetId(*id);
    loc.Assign(*new_loc);
}

void CModifySeqId::x_ChangeId(CSeq_id &id)
{
    CRef<CSeq_id> new_id;
    for (size_t i = 0; i < m_old_to_new_id.size(); i++)
    {
        if (m_old_to_new_id[i].first->Equals(id))
        {
            new_id = m_old_to_new_id[i].second;
            break;
        }
    }  
    if (!new_id)
        return;

    id.Reset();  
    id.Assign(*new_id);
}           

CRemoveSeqId::CRemoveSeqId(objects::CSeq_entry_Handle seh, CSeq_id::E_Choice id_type, CSeq_inst::EMol filter) 
    : CModifySeqId(seh, "Remove Seq-id")
{
    int nuc_offset = 1;
    int prot_offset = 1;
    CBioseq_Handle prev_nuc;
    for (CBioseq_CI b_iter(m_TopSeqEntry, filter); b_iter; ++b_iter) 
    {
        CConstRef<CSeq_id> remove_id;
        for (CBioseq_Handle::TId::const_iterator id_it = b_iter->GetId().begin(); id_it != b_iter->GetId().end(); ++id_it)
        {
            if (id_it->Which() == id_type)
            {
                remove_id = id_it->GetSeqId();
                break;
            }
        }
        CRef<CSeq_id> replace_id;
        for (const auto  &id : b_iter->GetId())
        {
            if (id.Which() != id_type)
            {
                if (!id.GetSeqId()->IsGeneral() || (id.GetSeqId()->IsGeneral() && id.GetSeqId()->GetGeneral().IsSetDb() && 
                                                    id.GetSeqId()->GetGeneral().GetDb() != "TMSMART" && id.GetSeqId()->GetGeneral().GetDb() != "BankIt" && id.GetSeqId()->GetGeneral().GetDb() != "NCBIFILE"))
                {
                    replace_id.Reset(new CSeq_id);
                    replace_id->Assign(*id.GetSeqId());
                    break;
                }
            }
        }

        if (!replace_id)
        {
            if (b_iter->IsNa())
            {
                replace_id.Reset(new CSeq_id);
                replace_id->SetLocal().SetStr("seq"+NStr::IntToString(nuc_offset));
                nuc_offset++;
            }
            else
            {
                CBioseq_set_Handle bssh = b_iter->GetParentBioseq_set();
                if (bssh && bssh.CanGetClass() && bssh.GetClass() ==  CBioseq_set::eClass_nuc_prot)
                {
                    CBioseq_CI b_iter_nuc(bssh, CSeq_inst::eMol_na);
                    if (b_iter_nuc)
                    {
                        if (prev_nuc != *b_iter_nuc)
                            prot_offset = 1;
                        string id_label;
                        replace_id = objects::edit::GetNewProtId(*b_iter_nuc, prot_offset, id_label, false);
                        prev_nuc = *b_iter_nuc;
                        prot_offset++;
                    }
                }
            }
        }
       

        if (remove_id && replace_id)
        {
            m_old_to_new_id.push_back(make_pair(remove_id, replace_id));

            if (remove_id->IsLocal())
            {
                string local; 
                if (remove_id->GetLocal().IsStr())
                    local = remove_id->GetLocal().GetStr();
                else if (remove_id->GetLocal().IsId())
                {
                    local = NStr::IntToString(remove_id->GetLocal().GetId());
                }
                if (!local.empty())
                    m_remove_ids.insert(local);
            }
        }
    }    
}
 
void CRemoveSeqId::x_UpdateBioseq(CRef<CBioseq> new_seq)
{
    if (new_seq->IsSetId())
    {
        bool found_new(false);
        for (const auto id : new_seq->GetId())
        {
            for (size_t i = 0; i < m_old_to_new_id.size(); i++)
            {
                if (m_old_to_new_id[i].second->Equals(*id))
                {
                    found_new = true;
                    break;
                }
            }
        }

        if (found_new)
        {
            CBioseq::TId::iterator it = new_seq->SetId().begin();
            while ( it != new_seq->SetId().end() )
            {
                bool found_old(false);
                for (size_t i = 0; i < m_old_to_new_id.size(); i++)
                {
                    if (m_old_to_new_id[i].first->Equals(**it))
                    {
                        found_old = true;
                        break;
                    }
                }
                
                if (found_old)
                {
                    it = new_seq->SetId().erase(it);
                    continue;
                }
                ++it;
            }
        }
        else
        {
            CModifySeqId::x_UpdateBioseq(new_seq);
        }
    }
}

CRef<CCmdComposite> CRemoveSeqId::GetRemoveOriginalIdCommand()
{
    CRef<CCmdComposite> cmd;
    if (!m_remove_ids.empty())
    {
        cmd.Reset(new CCmdComposite("Remove Original Local Ids"));   
        x_GetRemoveOriginalIdCommand(cmd);
    }
    return cmd;
}

CLocalToGeneralId::CLocalToGeneralId(objects::CSeq_entry_Handle seh, CSeq_inst::EMol filter) 
    : CModifySeqId(seh, "Local Seq-id to General Seq-id")
{
    bool ask_user(true);
    bool default_set(false);
    string default_db;
    for (CBioseq_CI b_iter(m_TopSeqEntry, filter); b_iter; ++b_iter) 
    {
        CConstRef<CSeq_id> local_id = b_iter->GetLocalIdOrNull();
        if (local_id)
        {
            if (local_id->GetLocal().IsStr())
            {
                vector<string> arr;
                NStr::Split(local_id->GetLocal().GetStr(), ":", arr, NStr::fSplit_Tokenize);
                if (arr.size() == 2)
                {
                    string db = arr.front();
                    string tag = arr.back();
                    CRef<CSeq_id> new_id(new CSeq_id);
                    new_id->SetGeneral().SetDb(db);
                    new_id->SetGeneral().SetTag().SetStr(tag);
                    m_old_to_new_id.push_back(make_pair(local_id, new_id));
                    continue;
                }
            }
            if (ask_user)
            {
                ask_user = false;
                wxTextEntryDialog dlg(NULL, _("Enter value for db"), _("db is missing"));
                if (dlg.ShowModal() == wxID_OK)
                {
                    default_db = dlg.GetValue().ToStdString();
                    default_set = true;
                }
            }
            if (default_set)
            {
                CRef<CSeq_id> new_id(new CSeq_id);
                new_id->SetGeneral().SetDb(default_db);
                if ( local_id->GetLocal().IsStr())
                    new_id->SetGeneral().SetTag().SetStr(local_id->GetLocal().GetStr());
                else if (local_id->GetLocal().IsId())
                    new_id->SetGeneral().SetTag().SetStr(NStr::IntToString(local_id->GetLocal().GetId()));
                if (new_id->GetGeneral().IsSetTag())
                    m_old_to_new_id.push_back(make_pair(local_id, new_id));
            }
        }
    }    
}

CGeneralToLocalId::CGeneralToLocalId(objects::CSeq_entry_Handle seh, CSeq_inst::EMol filter) 
    : CModifySeqId(seh, "General Seq-id to Local Seq-id")
{

    for (CBioseq_CI b_iter(m_TopSeqEntry, filter); b_iter; ++b_iter) 
    {
        for (CBioseq_Handle::TId::const_iterator id_it = b_iter->GetId().begin(); id_it != b_iter->GetId().end(); ++id_it)
        {
            CConstRef<CSeq_id> id = id_it->GetSeqId();
            if (id->IsGeneral() && id->GetGeneral().IsSetDb() && id->GetGeneral().GetDb() != "NCBIFILE" && id->GetGeneral().GetDb() != "TMSMART")
            {
                CRef<CSeq_id> new_id(new CSeq_id);
                string str;
                if (id->GetGeneral().IsSetTag())
                {
                    if (id->GetGeneral().GetTag().IsStr())
                        str = id->GetGeneral().GetTag().GetStr();
                    else if (id->GetGeneral().GetTag().IsId())
                        str = NStr::IntToString(id->GetGeneral().GetTag().GetId());
                    new_id->SetLocal().SetStr(str);
                    m_old_to_new_id.push_back(make_pair(id, new_id));
                }
            }
        }
    }    
}
 
CRemoveName::CRemoveName(objects::CSeq_entry_Handle seh)
    : CModifySeqId(seh, "Remove Seq-ID Name From Prot Feats")
{
    for (CBioseq_CI b_iter(m_TopSeqEntry); b_iter; ++b_iter) 
    {
        if (b_iter->IsAa())
        {
            for (CBioseq_Handle::TId::const_iterator id_it = b_iter->GetId().begin(); id_it != b_iter->GetId().end(); ++id_it)
            {
                CConstRef<CSeq_id> id = id_it->GetSeqId();
                x_GetReplacementIds(id);
            }
        }
        else
        {
            for (CFeat_CI feat_ci(*b_iter); feat_ci; ++feat_ci)
            {
                CConstRef<CSeq_feat> feat = feat_ci->GetSeq_feat();
                if (feat->IsSetProduct())
                {
                    CConstRef<CSeq_id> id = ConstRef(feat->GetProduct().GetId());
                    x_GetReplacementIds(id);
                }
            }
        }
    }    
}

void CRemoveName::x_GetReplacementIds(CConstRef<CSeq_id> id)
{
    CRef<CTextseq_id> text_id(new CTextseq_id);
    switch (id->Which())
    {
    case CSeq_id::e_Genbank : text_id->Assign(id->GetGenbank()); break;
    case CSeq_id::e_Embl : text_id->Assign(id->GetEmbl()); break;
    case CSeq_id::e_Ddbj : text_id->Assign(id->GetDdbj()); break;
    case CSeq_id::e_Other : text_id->Assign(id->GetOther()); break;
    case CSeq_id::e_Tpg : text_id->Assign(id->GetTpg()); break;
    case CSeq_id::e_Tpe : text_id->Assign(id->GetTpe()); break;
    case CSeq_id::e_Tpd : text_id->Assign(id->GetTpd()); break;
    default : text_id.Reset();
    }
    if (!text_id)
        return;
    if ((!text_id->IsSetAccession() || text_id->GetAccession().empty()) &&
        text_id->IsSetName() && !text_id->GetName().empty() && NStr::IsUpper(string(1,text_id->GetName().front())) && NStr::Find(text_id->GetName(),"_") != NPOS)
    {
        CRef<CSeq_id> new_id(new CSeq_id);
        new_id->Assign(*id);
        
        switch (new_id->Which())
        {
        case CSeq_id::e_Genbank : new_id->SetGenbank().ResetName(); break;
        case CSeq_id::e_Embl : new_id->SetEmbl().ResetName(); break;
        case CSeq_id::e_Ddbj : new_id->SetDdbj().ResetName(); break;
        case CSeq_id::e_Other : new_id->SetOther().ResetName(); break;
        case CSeq_id::e_Tpg : new_id->SetTpg().ResetName(); break;
        case CSeq_id::e_Tpe : new_id->SetTpe().ResetName(); break;
        case CSeq_id::e_Tpd : new_id->SetTpd().ResetName(); break;
        default : break;
        }
        
        m_old_to_new_id.push_back(make_pair(id, new_id));
    }
}

IMPLEMENT_DYNAMIC_CLASS( CRemoveGeneralId, wxDialog )


/*!
 * CRemoveGeneralId event table definition
 */

BEGIN_EVENT_TABLE( CRemoveGeneralId, wxDialog )

////@begin CRemoveGeneralId event table entries


////@end CRemoveGeneralId event table entries

END_EVENT_TABLE()


/*!
 * CRemoveGeneralId constructors
 */

CRemoveGeneralId::CRemoveGeneralId()
{
    Init();
}

CRemoveGeneralId::CRemoveGeneralId( wxWindow* parent, CSeq_entry_Handle seh, wxWindowID id, const wxString& caption, const wxPoint& pos, const wxSize& size, long style )
    : CRemoveSeqId(seh, CSeq_id::e_General)
{
    Init();
    Create(parent, id, caption, pos, size, style);
}


/*!
 * CRemoveGeneralId creator
 */

bool CRemoveGeneralId::Create( wxWindow* parent, wxWindowID id, const wxString& caption, const wxPoint& pos, const wxSize& size, long style )
{
////@begin CRemoveGeneralId creation
    SetExtraStyle(wxWS_EX_BLOCK_EVENTS);
    wxDialog::Create( parent, id, caption, pos, size, style );

    CreateControls();
    if (GetSizer())
    {
        GetSizer()->SetSizeHints(this);
    }
    Centre();
////@end CRemoveGeneralId creation
    return true;
}


/*!
 * CRemoveGeneralId destructor
 */

CRemoveGeneralId::~CRemoveGeneralId()
{
////@begin CRemoveGeneralId destruction
////@end CRemoveGeneralId destruction
}


/*!
 * Member initialisation
 */

void CRemoveGeneralId::Init()
{
////@begin CRemoveGeneralId member initialisation
////@end CRemoveGeneralId member initialisation
}


/*!
 * Control creation for CRemoveGeneralId
 */

void CRemoveGeneralId::CreateControls()
{    
////@begin CRemoveGeneralId content construction
    CRemoveGeneralId* itemDialog1 = this;

    wxBoxSizer* itemBoxSizer2 = new wxBoxSizer(wxVERTICAL);
    itemDialog1->SetSizer(itemBoxSizer2);

    wxBoxSizer* itemBoxSizer12 = new wxBoxSizer(wxHORIZONTAL);
    itemBoxSizer2->Add(itemBoxSizer12, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

    wxStaticText* itemStaticText11 = new wxStaticText( itemDialog1, wxID_STATIC, _("Remove general sequence IDs where database"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer12->Add(itemStaticText11, 0, wxALIGN_TOP|wxALL, 5);

    m_StringConstraintPanel = new CStringConstraintPanel( itemDialog1, false, wxID_ANY, wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer12->Add(m_StringConstraintPanel, 0, wxALIGN_TOP|wxALL|wxFIXED_MINSIZE, 0);
    m_StringConstraintPanel->SetStringSelection(_("Is one of"));

    wxBoxSizer* itemBoxSizer6 = new wxBoxSizer(wxHORIZONTAL);
    itemBoxSizer2->Add(itemBoxSizer6, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

    wxButton* itemButton7 = new wxButton( itemDialog1, wxID_OK, _("Accept"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer6->Add(itemButton7, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxButton* itemButton8 = new wxButton( itemDialog1, wxID_CANCEL, _("Cancel"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer6->Add(itemButton8, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

////@end CRemoveGeneralId content construction

}


/*!
 * Should we show tooltips?
 */

bool CRemoveGeneralId::ShowToolTips()
{
    return true;
}

/*!
 * Get bitmap resources
 */

wxBitmap CRemoveGeneralId::GetBitmapResource( const wxString& name )
{
    // Bitmap retrieval
////@begin CRemoveGeneralId bitmap retrieval
    wxUnusedVar(name);
    return wxNullBitmap;
////@end CRemoveGeneralId bitmap retrieval
}

/*!
 * Get icon resources
 */

wxIcon CRemoveGeneralId::GetIconResource( const wxString& name )
{
    // Icon retrieval
////@begin CRemoveGeneralId icon retrieval
    wxUnusedVar(name);
    return wxNullIcon;
////@end CRemoveGeneralId icon retrieval
}


CRef<CCmdComposite> CRemoveGeneralId::GetCommand()
{
    x_CollectChangedIds();
    return CModifySeqId::GetCommand();
}

void CRemoveGeneralId::x_CollectChangedIds()
{
    m_old_to_new_id.clear();
    m_remove_ids.clear();
    CRef<edit::CStringConstraint> constraint(m_StringConstraintPanel->GetStringConstraint());
    if (!constraint)
        return;
    for (CBioseq_CI b_iter(m_TopSeqEntry); b_iter; ++b_iter) 
    {
        CBioseq_Handle bsh = *b_iter;
        CRef<CSeq_id> new_id;

        for (CBioseq_Handle::TId::const_iterator id_it = b_iter->GetId().begin(); id_it != b_iter->GetId().end(); ++id_it)
        {
            CConstRef<CSeq_id> id = id_it->GetSeqId();
            if (!id->IsGeneral() || !id->GetGeneral().IsSetDb() || !constraint->DoesTextMatch(id->GetGeneral().GetDb()))
            {
                new_id.Reset(new CSeq_id);
                new_id->Assign(*id);
                break;
            }
        }

        if (!new_id)
            continue;

        for (CBioseq_Handle::TId::const_iterator id_it = b_iter->GetId().begin(); id_it != b_iter->GetId().end(); ++id_it)
        {
            CConstRef<CSeq_id> id = id_it->GetSeqId();
            if (id->IsGeneral() && id->GetGeneral().IsSetDb() && constraint->DoesTextMatch(id->GetGeneral().GetDb()))
            {
                m_old_to_new_id.push_back(make_pair(id, new_id));
            }           
        }
    }
}       

CAccessionToLocalId::CAccessionToLocalId(objects::CSeq_entry_Handle seh, CSeq_inst::EMol filter, bool force_name)
    : CModifySeqId(seh, "Convert Accessions to Local Ids")
{
    for (CBioseq_CI b_iter(m_TopSeqEntry, filter); b_iter; ++b_iter) 
    {
        for (CBioseq_Handle::TId::const_iterator id_it = b_iter->GetId().begin(); id_it != b_iter->GetId().end(); ++id_it)
        {
            CConstRef<CSeq_id> id = id_it->GetSeqId();
            x_GetReplacementIds(id, force_name);
        }      
    }    
}

void CAccessionToLocalId::x_GetReplacementIds(CConstRef<CSeq_id> id, bool force_name)
{
    CRef<CTextseq_id> text_id(new CTextseq_id);
    switch (id->Which())
    {
    case CSeq_id::e_Genbank : text_id->Assign(id->GetGenbank()); break;
    case CSeq_id::e_Embl : text_id->Assign(id->GetEmbl()); break;
    case CSeq_id::e_Ddbj : text_id->Assign(id->GetDdbj()); break;   
    default : text_id.Reset();
    }
    if (!text_id)
        return;
    string str;
    if (text_id->IsSetAccession() && !text_id->GetAccession().empty())
        str = text_id->GetAccession();
    if ((str.empty() || force_name) && text_id->IsSetName())
    {
        str = text_id->GetName();
    }

    if (!str.empty())
    {
        CRef<CSeq_id> new_id(new CSeq_id);
        new_id->Assign(*id);
        new_id->SetLocal().SetStr(str);
        m_old_to_new_id.push_back(make_pair(id, new_id));
    }
}

END_NCBI_SCOPE
