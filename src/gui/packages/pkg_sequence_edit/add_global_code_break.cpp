/*  $Id: add_global_code_break.cpp 41707 2018-09-12 12:51:39Z filippov $
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

#include <wx/stattext.h>
#include <wx/msgdlg.h>

#include <objmgr/util/seq_loc_util.hpp>
#include <objmgr/seq_vector.hpp>
#include <objmgr/util/sequence.hpp>
#include <objtools/edit/cds_fix.hpp>
#include <objtools/edit/loc_edit.hpp>
#include <gui/objutils/cmd_change_seq_feat.hpp>
#include <gui/objutils/cmd_change_bioseq_inst.hpp>
#include <gui/objutils/util_cmds.hpp>
#include <objects/seqfeat/Genetic_code_table.hpp>
#include <objects/seqfeat/Code_break.hpp>
#include <gui/packages/pkg_sequence_edit/add_global_code_break.hpp>

////@begin XPM images
////@end XPM images

BEGIN_NCBI_SCOPE

USING_SCOPE(ncbi::objects);

/*!
 * CAddGlobalCodeBreak type definition
 */

IMPLEMENT_DYNAMIC_CLASS( CAddGlobalCodeBreak, CBulkCmdDlg )


/*!
 * CAddGlobalCodeBreak event table definition
 */

BEGIN_EVENT_TABLE( CAddGlobalCodeBreak, CBulkCmdDlg )

////@begin CAddGlobalCodeBreak event table entries
////@end CAddGlobalCodeBreak event table entries

END_EVENT_TABLE()


/*!
 * CAddGlobalCodeBreak constructors
 */

CAddGlobalCodeBreak::CAddGlobalCodeBreak()
{
    Init();
}

CAddGlobalCodeBreak::CAddGlobalCodeBreak( wxWindow* parent, IWorkbench* wb, wxWindowID id, const wxString& caption, const wxPoint& pos, const wxSize& size, long style )
  : CBulkCmdDlg(wb)
{
    Init();
    Create(parent, id, caption, pos, size, style);
}


/*!
 * CAddGlobalCodeBreak creator
 */

bool CAddGlobalCodeBreak::Create( wxWindow* parent, wxWindowID id, const wxString& caption, const wxPoint& pos, const wxSize& size, long style )
{
////@begin CAddGlobalCodeBreak creation
    SetExtraStyle(wxWS_EX_BLOCK_EVENTS);
    CBulkCmdDlg::Create( parent, id, caption, pos, size, style );

    CreateControls();
    if (GetSizer())
    {
        GetSizer()->SetSizeHints(this);
    }
    Centre();
////@end CAddGlobalCodeBreak creation
    return true;
}


/*!
 * CAddGlobalCodeBreak destructor
 */

CAddGlobalCodeBreak::~CAddGlobalCodeBreak()
{
////@begin CAddGlobalCodeBreak destruction
////@end CAddGlobalCodeBreak destruction
}


/*!
 * Member initialisation
 */

void CAddGlobalCodeBreak::Init()
{
////@begin CAddGlobalCodeBreak member initialisation
    m_OkCancel = NULL;
////@end CAddGlobalCodeBreak member initialisation
    m_ErrorMessage = "";
   
    GetTopLevelSeqEntryAndProcessor();
}


/*!
 * Control creation for CAddGlobalCodeBreak
 */

void CAddGlobalCodeBreak::CreateControls()
{    
////@begin CAddGlobalCodeBreak content construction
    CAddGlobalCodeBreak* itemCBulkCmdDlg1 = this;

    wxBoxSizer* itemBoxSizer2 = new wxBoxSizer(wxVERTICAL);
    itemCBulkCmdDlg1->SetSizer(itemBoxSizer2);

    wxBoxSizer* itemBoxSizer3 = new wxBoxSizer(wxHORIZONTAL);
    itemBoxSizer2->Add(itemBoxSizer3, 0, wxALIGN_LEFT|wxALL, 5);

    wxStaticText* itemStaticText1 = new wxStaticText( itemCBulkCmdDlg1, wxID_STATIC, _("Triplet Codon"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer3->Add(itemStaticText1, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    m_Triplet = new wxTextCtrl( itemCBulkCmdDlg1, ID_TEXTCTRL35, wxEmptyString, wxDefaultPosition, wxSize(40, -1), 0 );
    itemBoxSizer3->Add(m_Triplet, 0, wxALIGN_CENTER_VERTICAL|wxALL, 0);
    m_Triplet->SetMaxLength(3);

    wxBoxSizer* itemBoxSizer4 = new wxBoxSizer(wxHORIZONTAL);
    itemBoxSizer2->Add(itemBoxSizer4, 0, wxALIGN_LEFT|wxALL, 5);

    wxStaticText* itemStaticText2 = new wxStaticText( itemCBulkCmdDlg1, wxID_STATIC, _("Amino Acid"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer4->Add(itemStaticText2, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxArrayString m_AminoAcidCtrlStrings;
    m_AminoAcidCtrlStrings.Add(wxEmptyString);
    m_AminoAcidCtrlStrings.Add(_("Alanine"));
    m_AminoAcidCtrlStrings.Add(_("Asp or Asn"));
    m_AminoAcidCtrlStrings.Add(_("Cysteine"));
    m_AminoAcidCtrlStrings.Add(_("Aspartic Acid"));
    m_AminoAcidCtrlStrings.Add(_("Glutamic Acid"));
    m_AminoAcidCtrlStrings.Add(_("Phenylalanine"));
    m_AminoAcidCtrlStrings.Add(_("Glycine"));
    m_AminoAcidCtrlStrings.Add(_("Histidine"));
    m_AminoAcidCtrlStrings.Add(_("Isoleucine"));
    m_AminoAcidCtrlStrings.Add(_("Leu or Ile"));
    m_AminoAcidCtrlStrings.Add(_("Lysine"));
    m_AminoAcidCtrlStrings.Add(_("Leucine"));
    m_AminoAcidCtrlStrings.Add(_("Methionine"));
    m_AminoAcidCtrlStrings.Add(_("Asparagine"));
    m_AminoAcidCtrlStrings.Add(_("Pyrrolysine"));
    m_AminoAcidCtrlStrings.Add(_("Proline"));
    m_AminoAcidCtrlStrings.Add(_("Glutamine"));
    m_AminoAcidCtrlStrings.Add(_("Arginine"));
    m_AminoAcidCtrlStrings.Add(_("Serine"));
    m_AminoAcidCtrlStrings.Add(_("Threonine"));
    m_AminoAcidCtrlStrings.Add(_("Selenocysteine"));
    m_AminoAcidCtrlStrings.Add(_("Valine"));
    m_AminoAcidCtrlStrings.Add(_("Tryptophan"));
    m_AminoAcidCtrlStrings.Add(_("Undetermined or atypical"));
    m_AminoAcidCtrlStrings.Add(_("Tyrosine"));
    m_AminoAcidCtrlStrings.Add(_("Glu or Gln"));
    m_AminoAcidCtrlStrings.Add(_("Termination"));
    m_AminoAcidCtrl = new wxChoice( itemCBulkCmdDlg1, ID_CHOICE14, wxDefaultPosition, wxSize(100, -1), m_AminoAcidCtrlStrings, 0 );
    itemBoxSizer4->Add(m_AminoAcidCtrl, 0, wxALIGN_CENTER_VERTICAL|wxALL, 0);

////@end CSingleCDSExceptionPanel content construction
    m_AminoAcidList.clear();
    m_AminoAcidList.push_back(" ");
    m_AminoAcidList.push_back("A");
    m_AminoAcidList.push_back("B");
    m_AminoAcidList.push_back("C");
    m_AminoAcidList.push_back("D");
    m_AminoAcidList.push_back("E");
    m_AminoAcidList.push_back("F");
    m_AminoAcidList.push_back("G");
    m_AminoAcidList.push_back("H");
    m_AminoAcidList.push_back("I");
    m_AminoAcidList.push_back("J");
    m_AminoAcidList.push_back("K");
    m_AminoAcidList.push_back("L");
    m_AminoAcidList.push_back("M");
    m_AminoAcidList.push_back("N");
    m_AminoAcidList.push_back("O");
    m_AminoAcidList.push_back("P");
    m_AminoAcidList.push_back("Q");
    m_AminoAcidList.push_back("R");
    m_AminoAcidList.push_back("S");
    m_AminoAcidList.push_back("T");
    m_AminoAcidList.push_back("U");
    m_AminoAcidList.push_back("V");
    m_AminoAcidList.push_back("W");
    m_AminoAcidList.push_back("X");
    m_AminoAcidList.push_back("Y");
    m_AminoAcidList.push_back("Z");
    m_AminoAcidList.push_back("*");

  

    m_OkCancel = new COkCancelPanel( itemCBulkCmdDlg1, wxID_ANY, wxDefaultPosition, wxSize(100, 100), 0 );
    itemBoxSizer2->Add(m_OkCancel, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

////@end CAddGlobalCodeBreak content construction
}


/*!
 * Should we show tooltips?
 */

bool CAddGlobalCodeBreak::ShowToolTips()
{
    return true;
}

/*!
 * Get bitmap resources
 */

wxBitmap CAddGlobalCodeBreak::GetBitmapResource( const wxString& name )
{
    // Bitmap retrieval
////@begin CAddGlobalCodeBreak bitmap retrieval
    wxUnusedVar(name);
    return wxNullBitmap;
////@end CAddGlobalCodeBreak bitmap retrieval
}

/*!
 * Get icon resources
 */

wxIcon CAddGlobalCodeBreak::GetIconResource( const wxString& name )
{
    // Icon retrieval
////@begin CAddGlobalCodeBreak icon retrieval
    wxUnusedVar(name);
    return wxNullIcon;
////@end CAddGlobalCodeBreak icon retrieval
}




string CAddGlobalCodeBreak::GetErrorMessage()
{
    return m_ErrorMessage;
}


CRef<CCmdComposite> CAddGlobalCodeBreak::GetCommand()
{
    CRef<CCmdComposite> cmd;
    if (!m_TopSeqEntry)
        return cmd;
    string pattern = m_Triplet->GetValue().ToStdString();
    string aa_str;
    int aa_pos = m_AminoAcidCtrl->GetSelection();
    if (aa_pos > 0 && aa_pos < m_AminoAcidList.size()) 
    {
        aa_str = m_AminoAcidList[aa_pos];
    }              
    if (pattern.size() < 3 || aa_str.empty())
        return cmd;
    cmd.Reset(new CCmdComposite("Add Codon Break"));
    CScope& scope = m_TopSeqEntry.GetScope();
    bool modified = false;

    for (CFeat_CI fi(m_TopSeqEntry, CSeqFeatData::eSubtype_cdregion); fi; ++fi)
    {         
        const CSeq_loc& loc = fi->GetLocation();
        CBioseq_Handle bsh = scope.GetBioseqHandle(loc);
        if (!bsh)
            continue;
        const CSeq_feat& cds = fi->GetOriginalFeature();
        if (cds.IsSetExcept_text() && NStr::Find(cds.GetExcept_text(), "RNA editing") != string::npos) 
        {
            continue;
        }

        CRef<CSeq_feat> new_feat(new CSeq_feat());
        new_feat->Assign(cds);

        const CGenetic_code* code = NULL;
        if (cds.IsSetData() && cds.GetData().IsCdregion() && cds.GetData().GetCdregion().IsSetCode()) 
        {
            code = &(cds.GetData().GetCdregion().GetCode());
        }
        // get appropriate translation table
        const CTrans_table & tbl = (code ? CGen_code_table::GetTransTable(*code) : CGen_code_table::GetTransTable(1));
       
        int offset_frame = 0;
        if (cds.IsSetData() && cds.GetData().IsCdregion() && cds.GetData().GetCdregion().IsSetFrame()) 
        {
            CCdregion::EFrame frame = cds.GetData().GetCdregion().GetFrame();
            if (frame == CCdregion::eFrame_two) 
            {
                offset_frame = 1;
            } 
            else if (frame == CCdregion::eFrame_three) 
            {
                offset_frame = 2;
            }
        }
       
        CSeqVector seq(loc, scope, CBioseq_Handle::eCoding_Iupac);

        for (size_t i = offset_frame;  i < seq.size();  i += 3) 
        {
            string str;
            seq.GetSeqData(i, i+3, str);
            if (NStr::EqualNocase(str, pattern))
            {
                CRef<CCode_break> code_break(new CCode_break);

                // amino acid              
                code_break->SetAa().SetNcbieaa(aa_str[0]);
              

                int offset = 0;
                CRef<CSeq_loc> cb_loc (new CSeq_loc());
                for (CSeq_loc_CI loc_iter(loc);  loc_iter;  ++loc_iter) 
                {
                    int len = loc_iter.GetRange().GetLength();
                    if (offset <= i && offset + len > i) 
                    {
                        cb_loc->SetInt().SetId().Assign(loc_iter.GetSeq_id());
                        if (loc_iter.IsSetStrand() && loc_iter.GetStrand() == eNa_strand_minus) 
                        {                    
                            cb_loc->SetInt().SetStrand(eNa_strand_minus);
                            cb_loc->SetInt().SetTo(loc_iter.GetRange().GetTo() - (i - offset) );
                        } 
                        else 
                        {
                            cb_loc->SetInt().SetFrom(loc_iter.GetRange().GetFrom() + i - offset);
                        }

                        if (offset <= i + 2 && offset + len > i + 2) 
                        {
                            if (loc_iter.IsSetStrand() && loc_iter.GetStrand() == eNa_strand_minus) 
                            {                    
                                cb_loc->SetInt().SetFrom(loc_iter.GetRange().GetTo() - (i - offset + 2) );
                            } 
                            else 
                            {
                                cb_loc->SetInt().SetTo(loc_iter.GetRange().GetFrom() + i - offset + 2);
                            }
                        } 
                        else 
                        {
                            if (loc_iter.IsSetStrand() && loc_iter.GetStrand() == eNa_strand_minus) 
                            {                    
                                cb_loc->SetInt().SetFrom(loc_iter.GetRange().GetFrom());
                            } else 
                            {
                                cb_loc->SetInt().SetTo(loc_iter.GetRange().GetTo());
                            }
                        }
                    } 
                    else if (offset > i && offset <= i + 2) 
                    {
                        if (cb_loc->IsInt()) 
                        {
                            CRef<CSeq_interval> tmp(new CSeq_interval());
                            tmp->Assign (cb_loc->GetInt());
                            cb_loc->SetPacked_int().Set().push_back(tmp);
                        }
                        // add new interval
                        CRef<CSeq_interval> tmp (new CSeq_interval());
                        tmp->SetId().Assign(loc_iter.GetSeq_id());
                        if (loc_iter.IsSetStrand() && loc_iter.GetStrand() == eNa_strand_minus) 
                        {                    
                            tmp->SetStrand(eNa_strand_minus);
                            tmp->SetTo(loc_iter.GetRange().GetTo());
                            if (offset + len >= i + 2) 
                            {
                                tmp->SetFrom(loc_iter.GetRange().GetTo() - (i - offset + 2) );
                            } 
                            else 
                            {
                                tmp->SetFrom(loc_iter.GetRange().GetFrom());
                            }
                        } 
                        else 
                        {
                            tmp->SetFrom(loc_iter.GetRange().GetFrom());
                            if (offset + len >= i + 2) 
                            {
                                tmp->SetTo(loc_iter.GetRange().GetFrom() + i - offset + 2);
                            } 
                            else 
                            {
                                tmp->SetTo(loc_iter.GetRange().GetTo());
                            }
                        }
                        cb_loc->SetPacked_int().Set().push_back(tmp);         
                    } 
                    offset += len;
                }
                code_break->SetLoc(*cb_loc);
            

                new_feat->SetData().SetCdregion().SetCode_break().push_back(code_break);
                cmd->AddCommand(*CRef<CCmdChangeSeq_feat>(new CCmdChangeSeq_feat(fi->GetSeq_feat_Handle(), *new_feat)));
                CRef<CCmdComposite> retranslate = RetranslateCDSCommand(scope, *new_feat);
                if (retranslate)
                    cmd->AddCommand(*retranslate);
                
                modified = true;
            }
        }       
    }
    if (!modified)
        cmd.Reset();
    return cmd;        
}

CRef<CCmdComposite> CAddGlobalCodeBreak::RetranslateCDSCommand(CScope& scope, CSeq_feat& cds)
{
    CRef<CCmdComposite> empty(NULL);

    // feature must be cds
    if (!cds.IsSetData() && !cds.GetData().IsCdregion()) {
        return empty;
    }
  
    // Use Cdregion.Product to get handle to protein bioseq 
    CBioseq_Handle prot_bsh = scope.GetBioseqHandle(cds.GetProduct());

    if (!prot_bsh) {
        return empty;
    }
    // Should be a protein!
    if (!prot_bsh.IsProtein())
    {
        return empty;
    }

    // Make a copy of existing CSeq_inst
    CRef<objects::CSeq_inst> new_inst(new objects::CSeq_inst());
    new_inst->Assign(prot_bsh.GetInst());

    // Make edits to the CSeq_inst copy
    //CRef<CBioseq> new_protein_bioseq;
    if (new_inst->IsSetSeq_data())
    {
        new_inst->ResetSeq_data();
    }

    // Generate new protein sequence data and length
    string prot;
    CSeqTranslator::Translate(cds, scope, prot);
    if (NStr::EndsWith(prot, "*")) 
    {
        prot = prot.substr(0, prot.length() - 1);
    }
    new_inst->ResetExt();
    new_inst->SetRepr(objects::CSeq_inst::eRepr_raw); 
    new_inst->SetSeq_data().SetNcbieaa().Set(prot);
    new_inst->SetLength(TSeqPos(prot.length()));
    new_inst->SetMol(CSeq_inst::eMol_aa);

      
    CRef<CCmdComposite> cmd(new CCmdComposite("Retranslate CDS"));

    // Update protein sequence data and length
    CRef<CCmdChangeBioseqInst> chgInst (new CCmdChangeBioseqInst(prot_bsh, *new_inst));
    cmd->AddCommand(*chgInst);

    return cmd;
}
                


END_NCBI_SCOPE

