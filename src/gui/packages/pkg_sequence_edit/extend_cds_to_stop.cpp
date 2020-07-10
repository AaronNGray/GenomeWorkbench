/*  $Id: extend_cds_to_stop.cpp 41708 2018-09-12 12:55:25Z filippov $
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
#include <gui/packages/pkg_sequence_edit/extend_cds_to_stop.hpp>

////@begin XPM images
////@end XPM images

BEGIN_NCBI_SCOPE

USING_SCOPE(ncbi::objects);

/*!
 * CExtendCDSToStop type definition
 */

IMPLEMENT_DYNAMIC_CLASS( CExtendCDSToStop, CBulkCmdDlg )


/*!
 * CExtendCDSToStop event table definition
 */

BEGIN_EVENT_TABLE( CExtendCDSToStop, CBulkCmdDlg )

////@begin CExtendCDSToStop event table entries
////@end CExtendCDSToStop event table entries

END_EVENT_TABLE()


/*!
 * CExtendCDSToStop constructors
 */

CExtendCDSToStop::CExtendCDSToStop()
{
    Init();
}

CExtendCDSToStop::CExtendCDSToStop( wxWindow* parent, IWorkbench* wb, wxWindowID id, const wxString& caption, const wxPoint& pos, const wxSize& size, long style )
  : CBulkCmdDlg(wb)
{
    Init();
    Create(parent, id, caption, pos, size, style);
}


/*!
 * CExtendCDSToStop creator
 */

bool CExtendCDSToStop::Create( wxWindow* parent, wxWindowID id, const wxString& caption, const wxPoint& pos, const wxSize& size, long style )
{
////@begin CExtendCDSToStop creation
    SetExtraStyle(wxWS_EX_BLOCK_EVENTS);
    CBulkCmdDlg::Create( parent, id, caption, pos, size, style );

    CreateControls();
    if (GetSizer())
    {
        GetSizer()->SetSizeHints(this);
    }
    Centre();
////@end CExtendCDSToStop creation
    return true;
}


/*!
 * CExtendCDSToStop destructor
 */

CExtendCDSToStop::~CExtendCDSToStop()
{
////@begin CExtendCDSToStop destruction
////@end CExtendCDSToStop destruction
}


/*!
 * Member initialisation
 */

void CExtendCDSToStop::Init()
{
////@begin CExtendCDSToStop member initialisation
    m_OkCancel = NULL;
////@end CExtendCDSToStop member initialisation
    m_ErrorMessage = "";
    m_ExtendProtFeats = NULL;
    m_Retranslate = NULL;
    m_ExtendmRNA = NULL;
    m_ResetGenes = NULL;
    GetTopLevelSeqEntryAndProcessor();
}


/*!
 * Control creation for CExtendCDSToStop
 */

void CExtendCDSToStop::CreateControls()
{    
////@begin CExtendCDSToStop content construction
    CExtendCDSToStop* itemCBulkCmdDlg1 = this;

    wxBoxSizer* itemBoxSizer2 = new wxBoxSizer(wxVERTICAL);
    itemCBulkCmdDlg1->SetSizer(itemBoxSizer2);


    m_ExtendProtFeats = new wxCheckBox( itemCBulkCmdDlg1, wxID_ANY, _("Extend Protein Features"), wxDefaultPosition, wxDefaultSize, 0 );
    m_ExtendProtFeats->SetValue(true);
    itemBoxSizer2->Add(m_ExtendProtFeats, 0, wxALIGN_LEFT|wxALL, 5);

    m_Retranslate = new wxCheckBox( itemCBulkCmdDlg1, wxID_ANY, _("Retranslate Coding Regions for Extended Proteins"), wxDefaultPosition, wxDefaultSize, 0 );
    m_Retranslate->SetValue(true);
    itemBoxSizer2->Add(m_Retranslate, 0, wxALIGN_LEFT|wxALL, 5);

    m_ExtendmRNA = new wxCheckBox( itemCBulkCmdDlg1, wxID_ANY, _("Extend Associated mRNA Features"), wxDefaultPosition, wxDefaultSize, 0 );
    m_ExtendmRNA->SetValue(false);
    itemBoxSizer2->Add(m_ExtendmRNA, 0, wxALIGN_LEFT|wxALL, 5);

    m_ResetGenes = new wxCheckBox( itemCBulkCmdDlg1, wxID_ANY, _("Reset Genes"), wxDefaultPosition, wxDefaultSize, 0 );
    m_ResetGenes->SetValue(true);
    itemBoxSizer2->Add(m_ResetGenes, 0, wxALIGN_LEFT|wxALL, 5);

  

    m_OkCancel = new COkCancelPanel( itemCBulkCmdDlg1, wxID_ANY, wxDefaultPosition, wxSize(100, 100), 0 );
    itemBoxSizer2->Add(m_OkCancel, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

////@end CExtendCDSToStop content construction
}


/*!
 * Should we show tooltips?
 */

bool CExtendCDSToStop::ShowToolTips()
{
    return true;
}

/*!
 * Get bitmap resources
 */

wxBitmap CExtendCDSToStop::GetBitmapResource( const wxString& name )
{
    // Bitmap retrieval
////@begin CExtendCDSToStop bitmap retrieval
    wxUnusedVar(name);
    return wxNullBitmap;
////@end CExtendCDSToStop bitmap retrieval
}

/*!
 * Get icon resources
 */

wxIcon CExtendCDSToStop::GetIconResource( const wxString& name )
{
    // Icon retrieval
////@begin CExtendCDSToStop icon retrieval
    wxUnusedVar(name);
    return wxNullIcon;
////@end CExtendCDSToStop icon retrieval
}




string CExtendCDSToStop::GetErrorMessage()
{
    return m_ErrorMessage;
}


CRef<CCmdComposite> CExtendCDSToStop::GetCommand()
{
    CRef<CCmdComposite> cmd;
    if (!m_TopSeqEntry)
        return cmd;
    cmd.Reset(new CCmdComposite("Extend CDS To Stop Codon"));
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
        if (cds.IsSetData() && cds.GetData().IsCdregion() && cds.GetData().GetCdregion().IsSetCode()) {
            code = &(cds.GetData().GetCdregion().GetCode());
        }

        size_t stop = loc.GetStop(eExtreme_Biological);
        // figure out if we have a partial codon at the end
        size_t orig_len = sequence::GetLength(loc, &scope);
        size_t len = orig_len;
        if (cds.IsSetData() && cds.GetData().IsCdregion() && cds.GetData().GetCdregion().IsSetFrame()) {
            CCdregion::EFrame frame = cds.GetData().GetCdregion().GetFrame();
            if (frame == CCdregion::eFrame_two) {
                len -= 1;
            } else if (frame == CCdregion::eFrame_three) {
                len -= 2;
            }
        }
        size_t mod = len % 3;
        CRef<CSeq_loc> vector_loc(new CSeq_loc());
        vector_loc->SetInt().SetId().Assign(*(loc.GetId()));

        if (loc.IsSetStrand() && loc.GetStrand() == eNa_strand_minus) {
            vector_loc->SetInt().SetFrom(0);
            vector_loc->SetInt().SetTo(stop + mod - 1 + 3);
            vector_loc->SetStrand(eNa_strand_minus);
        } else {
            vector_loc->SetInt().SetFrom(stop - mod + 1 - 3);
            vector_loc->SetInt().SetTo(bsh.GetInst_Length() - 1);
        }

        CSeqVector seq(*vector_loc, scope, CBioseq_Handle::eCoding_Iupac);
        // reserve our space
        const size_t usable_size = seq.size();

        // get appropriate translation table
        const CTrans_table & tbl =
            (code ? CGen_code_table::GetTransTable(*code) :
             CGen_code_table::GetTransTable(1));

        // main loop through bases
        CSeqVector::const_iterator start = seq.begin();
        
        size_t i;
        size_t k;
        size_t state = 0;
        size_t length = usable_size / 3;
        
        CRef<CSeq_loc> new_loc(NULL);
        
        for (i = 0;  i < length;  ++i) 
        {
            // loop through one codon at a time
            for (k = 0;  k < 3;  ++k, ++start) 
            {
                state = tbl.NextCodonState(state, *start);
            }
            
            if (tbl.GetCodonResidue (state) == '*') 
            {
                if (i == 0)
                    break;
                CSeq_loc_CI it(loc);
                it.SetPos(it.GetSize() - 1);
                CConstRef<CSeq_loc> this_loc = it.GetRangeAsSeq_loc();

                CRef<CSeq_loc> last_interval(new CSeq_loc());
                size_t this_start = this_loc->GetStart(eExtreme_Positional);
                size_t this_stop = this_loc->GetStop(eExtreme_Positional);
                size_t extension = ((i + 1) * 3) - mod - 3;
                last_interval->SetInt().SetId().Assign(*(this_loc->GetId()));
                if (this_loc->IsSetStrand() && this_loc->GetStrand() == eNa_strand_minus) 
                {
                    last_interval->SetStrand(eNa_strand_minus);
                    last_interval->SetInt().SetFrom(this_start - extension);
                    last_interval->SetInt().SetTo(this_stop);
                } 
                else 
                {
                    last_interval->SetInt().SetFrom(this_start);
                    last_interval->SetInt().SetTo(this_stop + extension);
                }

                CRef<CSeq_loc> new_loc = sequence::Seq_loc_Add(loc, *last_interval, CSeq_loc::fMerge_All|CSeq_loc::fSort, &scope);               
                new_loc->SetPartialStop(false, eExtreme_Biological);
                new_loc->SetPartialStart(loc.IsPartialStart(eExtreme_Biological), eExtreme_Biological);
                new_feat->SetLocation().Assign(*new_loc);
                new_feat->SetPartial(new_feat->GetLocation().IsPartialStart(eExtreme_Positional) || new_feat->GetLocation().IsPartialStop(eExtreme_Positional));
                cmd->AddCommand(*CRef<CCmdChangeSeq_feat>(new CCmdChangeSeq_feat(fi->GetSeq_feat_Handle(), *new_feat)));
                CRef<CCmdComposite> retranslate = RetranslateCDSCommand(scope, *new_feat);
                if (retranslate)
                    cmd->AddCommand(*retranslate);
                if (m_ResetGenes->GetValue())
                    ResetGenes(cmd, *fi, last_interval, scope);
                if (m_ExtendmRNA->GetValue())
                    ExtendmRNA(cmd, *fi, last_interval, scope);
                modified = true;
                break;
            }
        }       
    }
    if (!modified)
        cmd.Reset();
    return cmd;        
}

CRef<CCmdComposite> CExtendCDSToStop::RetranslateCDSCommand(CScope& scope, CSeq_feat& cds)
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

    if (m_Retranslate->GetValue())
    {
        // Update protein sequence data and length
        CRef<CCmdChangeBioseqInst> chgInst (new CCmdChangeBioseqInst(prot_bsh, *new_inst));
        cmd->AddCommand(*chgInst);

        // change molinfo on bioseq
        CRef<CCmdComposite> synch_molinfo = GetSynchronizeProductMolInfoCommand(scope, cds);
        if (synch_molinfo) 
        {
            cmd->AddCommand(*synch_molinfo);
        }
    }

    if (m_ExtendProtFeats->GetValue())
    {
        // If protein feature exists, update it
        SAnnotSelector sel(CSeqFeatData::e_Prot);
        CFeat_CI prot_feat_ci(prot_bsh, sel);
        for ( ; prot_feat_ci; ++prot_feat_ci ) {
            CRef<CSeq_feat> new_feat(new CSeq_feat());
            new_feat->Assign(prot_feat_ci->GetOriginalFeature());
            
            if ( new_feat->CanGetLocation() &&
                 new_feat->GetLocation().IsInt() &&
                 new_feat->GetLocation().GetInt().CanGetTo() )
            {
                new_feat->SetLocation().SetInt().SetTo(new_inst->GetLength() - 1);
                
                edit::AdjustProteinFeaturePartialsToMatchCDS(*new_feat, cds);
                
                CIRef<IEditCommand> chgFeat(new CCmdChangeSeq_feat(*prot_feat_ci,
                                                                   *new_feat));
                cmd->AddCommand(*chgFeat);
            }
        }
    }

    return cmd;
}
                

void CExtendCDSToStop::ResetGenes(CRef<CCmdComposite> cmd, const objects::CMappedFeat& feat, CRef<CSeq_loc> add, CScope &scope)
{
    objects::CSeq_feat_Handle gene = objects::feature::GetBestGeneForCds(feat).GetSeq_feat_Handle();
    if (gene)
    {
        CRef<objects::CSeq_feat> new_gene(new CSeq_feat);
        new_gene->Assign(*gene.GetOriginalSeq_feat());
        const objects::CSeq_loc& gene_loc = gene.GetLocation();
        CRef<CSeq_loc> new_loc = sequence::Seq_loc_Add(gene_loc, *add, CSeq_loc::fMerge_All|CSeq_loc::fSort, &scope);  
        objects::CSeq_loc::TRange range = new_loc->GetTotalRange();

        CRef<objects::CSeq_loc> new_gene_loc(new objects::CSeq_loc);
        CRef<objects::CSeq_id> id(new objects::CSeq_id);
        id->Assign(*gene_loc.GetId());
        CRef<objects::CSeq_interval> new_int(new objects::CSeq_interval(*id, range.GetFrom(), range.GetTo(), gene_loc.GetStrand()));
        new_gene_loc->SetInt(*new_int);
        new_gene_loc->SetPartialStart(gene_loc.IsPartialStart(eExtreme_Biological), objects::eExtreme_Biological);
        new_gene_loc->SetPartialStop(false, objects::eExtreme_Biological);
        new_gene->SetLocation().Assign(*new_gene_loc);
        new_gene->SetPartial(new_gene->GetLocation().IsPartialStart(eExtreme_Positional) || new_gene->GetLocation().IsPartialStop(eExtreme_Positional));
        cmd->AddCommand(*CRef< CCmdChangeSeq_feat >(new CCmdChangeSeq_feat(gene,*new_gene))); 
    }
}

void CExtendCDSToStop::ExtendmRNA(CRef<CCmdComposite> cmd, const objects::CMappedFeat& feat, CRef<CSeq_loc> add, CScope &scope)
{
    objects::CSeq_feat_Handle mrna = objects::feature::GetBestMrnaForCds(feat).GetSeq_feat_Handle();
    if (mrna)
    {
        CRef<objects::CSeq_feat> new_mrna(new CSeq_feat);
        new_mrna->Assign(*mrna.GetOriginalSeq_feat());
        const objects::CSeq_loc& mrna_loc = mrna.GetLocation();
        CRef<CSeq_loc> new_loc = sequence::Seq_loc_Add(mrna_loc, *add, CSeq_loc::fMerge_All|CSeq_loc::fSort, &scope);  
        new_loc->SetPartialStart(mrna_loc.IsPartialStart(eExtreme_Biological), objects::eExtreme_Biological);
        new_loc->SetPartialStop(false, objects::eExtreme_Biological);
        new_mrna->SetLocation().Assign(*new_loc);
        new_mrna->SetPartial(new_mrna->GetLocation().IsPartialStart(eExtreme_Positional) || new_mrna->GetLocation().IsPartialStop(eExtreme_Positional));
        cmd->AddCommand(*CRef< CCmdChangeSeq_feat >(new CCmdChangeSeq_feat(mrna,*new_mrna))); 
    }
}

END_NCBI_SCOPE

