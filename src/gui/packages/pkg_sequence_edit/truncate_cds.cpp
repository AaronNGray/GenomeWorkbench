/* $Id: truncate_cds.cpp 42224 2019-01-16 16:10:33Z asztalos $
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
#include <objmgr/feat_ci.hpp>
#include <gui/objutils/cmd_change_seq_feat.hpp>
#include <gui/objutils/util_cmds.hpp>
#include <objtools/edit/cds_fix.hpp>
#include <objmgr/util/sequence.hpp>
#include <objects/misc/sequence_macros.hpp>
#include <gui/objutils/descriptor_change.hpp>
#include <gui/objutils/cmd_create_desc.hpp>
#include <gui/packages/pkg_sequence_edit/truncate_cds.hpp>

#include <wx/button.h>

////@begin XPM images
////@end XPM images

BEGIN_NCBI_SCOPE
USING_SCOPE(objects);

IMPLEMENT_DYNAMIC_CLASS( CTruncateCDS, wxDialog )



BEGIN_EVENT_TABLE( CTruncateCDS, wxDialog )

////@begin CTruncateCDS event table entries
////@end CTruncateCDS event table entries

END_EVENT_TABLE()


/*!
 * CTruncateCDS constructors
 */

CTruncateCDS::CTruncateCDS()
{
    Init();
}

CTruncateCDS::CTruncateCDS( wxWindow* parent, CSeq_entry_Handle seh, 
				wxWindowID id, const wxString& caption, const wxPoint& pos, const wxSize& size, long style )
    : m_TopSeqEntry(seh)
{
    Init();
    Create(parent, id, caption, pos, size, style);
}


/*!
 * CTruncateCDS creator
 */

bool CTruncateCDS::Create( wxWindow* parent, 
			     wxWindowID id, const wxString& caption, const wxPoint& pos, const wxSize& size, long style )
{
////@begin CTruncateCDS creation
    SetExtraStyle(wxWS_EX_BLOCK_EVENTS);
    wxDialog::Create( parent, id, caption, pos, size, style );

    CreateControls();
    if (GetSizer())
    {
        GetSizer()->SetSizeHints(this);
    }
    Centre();
////@end CTruncateCDS creation

    return true;
}


/*!
 * CTruncateCDS destructor
 */

CTruncateCDS::~CTruncateCDS()
{
////@begin CTruncateCDS destruction
////@end CTruncateCDS destruction
}


/*!
 * Member initialisation
 */

void CTruncateCDS::Init()
{
////@begin CTruncateCDS member initialisation
    m_TrimProtFeats  = NULL;
    m_Retranslate = NULL;
    m_TruncateMRNA = NULL;
    m_ResetGenes = NULL;
    m_PrependProtName = NULL;
////@end CTruncateCDS member initialisation
}


/*!
 * Control creation for CTruncateCDS
 */

void CTruncateCDS::CreateControls()
{    
////@begin CTruncateCDS content construction
    CTruncateCDS* itemDialog1 = this;

    wxBoxSizer* itemBoxSizer2 = new wxBoxSizer(wxVERTICAL);
    itemDialog1->SetSizer(itemBoxSizer2);

    m_TrimProtFeats = new wxCheckBox( itemDialog1, wxID_ANY, _("Trim Protein Features"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer2->Add(m_TrimProtFeats, 0, wxALIGN_LEFT|wxALL, 5);
    m_TrimProtFeats->SetValue(true);

    m_Retranslate = new wxCheckBox( itemDialog1, wxID_ANY, _("Retranslate Coding Regions"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer2->Add(m_Retranslate, 0, wxALIGN_LEFT|wxALL, 5);
    m_Retranslate->SetValue(true);

    m_TruncateMRNA = new wxCheckBox( itemDialog1, wxID_ANY, _("Truncate Associated mRNA Features"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer2->Add(m_TruncateMRNA, 0, wxALIGN_LEFT|wxALL, 5);
    m_TruncateMRNA->SetValue(false);

    m_ResetGenes = new wxCheckBox( itemDialog1, wxID_ANY, _("Reset Genes"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer2->Add(m_ResetGenes, 0, wxALIGN_LEFT|wxALL, 5);
    m_ResetGenes->SetValue(true);

    m_PrependProtName = new wxCheckBox( itemDialog1, wxID_ANY, _("Prepend 'Truncated' to Protein Name"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer2->Add(m_PrependProtName, 0, wxALIGN_LEFT|wxALL, 5);
    m_PrependProtName->SetValue(true);

    wxBoxSizer* itemBoxSizer7 = new wxBoxSizer(wxHORIZONTAL);
    itemBoxSizer2->Add(itemBoxSizer7, 0, wxEXPAND|wxALL, 5);

    wxButton* itemButton6 = new wxButton( itemDialog1, wxID_OK, _("Accept"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer7->Add(itemButton6, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxButton* itemButton7 = new wxButton( itemDialog1, wxID_CANCEL, _("Cancel"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer7->Add(itemButton7, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

////@end CTruncateCDS content construction
    
}


/*!
 * Should we show tooltips?
 */

bool CTruncateCDS::ShowToolTips()
{
    return true;
}

/*!
 * Get bitmap resources
 */

wxBitmap CTruncateCDS::GetBitmapResource( const wxString& name )
{
    // Bitmap retrieval
////@begin CTruncateCDS bitmap retrieval
    wxUnusedVar(name);
    return wxNullBitmap;
////@end CTruncateCDS bitmap retrieval
}

/*!
 * Get icon resources
 */

wxIcon CTruncateCDS::GetIconResource( const wxString& name )
{
    // Icon retrieval
////@begin CTruncateCDS icon retrieval
    wxUnusedVar(name);
    return wxNullIcon;
////@end CTruncateCDS icon retrieval
}

CRef<CCmdComposite> CTruncateCDS::GetCommand()
{
    CRef<CCmdComposite> cmd(new CCmdComposite("Truncate CDS"));
    CScope& scope = m_TopSeqEntry.GetScope();
    int offset = 1;
    bool create_general_only = objects::edit::IsGeneralIdProtPresent(m_TopSeqEntry);
    for (CFeat_CI fi(m_TopSeqEntry, CSeqFeatData::eSubtype_cdregion); fi; ++fi)
    {   
        const CSeq_feat& cds = fi->GetOriginalFeature();
        if (cds.IsSetExcept_text() && NStr::Find(cds.GetExcept_text(), "RNA editing") != string::npos) 
        {
            continue;
        }
        CRef<CSeq_feat> new_cds(new CSeq_feat());
        new_cds->Assign(cds);
        bool partial_start =  new_cds->GetLocation().IsPartialStart(eExtreme_Biological);
        if ( edit::TruncateCDSAtStop(*new_cds, scope) ) 
        {
            if (fi->GetOriginalFeature().Equals(*new_cds))
                continue;
           
            new_cds->SetLocation().SetPartialStart(partial_start, eExtreme_Biological);
            new_cds->SetLocation().SetPartialStop(false, eExtreme_Biological);
            new_cds->SetPartial(new_cds->GetLocation().IsPartialStart(eExtreme_Biological) || new_cds->GetLocation().IsPartialStop(eExtreme_Biological));
            cmd->AddCommand(*CRef<CCmdChangeSeq_feat>(new CCmdChangeSeq_feat(fi->GetSeq_feat_Handle(), *new_cds)));
        

            if ( m_Retranslate->GetValue() )   
            {
                CRef<CCmdComposite> transl_cmd = GetRetranslateCDSCommand(scope, *new_cds, offset, create_general_only);
                if (transl_cmd) 
                {
                    cmd->AddCommand(*transl_cmd);
                }
            }

            CBioseq_Handle prot_bsh = fi->GetScope().GetBioseqHandle(fi->GetProduct());
            if (prot_bsh.IsProtein()) 
            {
                SAnnotSelector sel(CSeqFeatData::e_Prot);
                CFeat_CI prot_feat_ci(prot_bsh, sel);
                if (prot_feat_ci)
                {
                    CRef<CSeq_feat> new_prot(new CSeq_feat());
                    new_prot->Assign(prot_feat_ci->GetOriginalFeature());
                    if (m_PrependProtName->GetValue())  
                    {
                        if (new_prot->IsSetData() && new_prot->GetData().IsProt() && new_prot->GetData().GetProt().IsSetName() 
                            && !new_prot->GetData().GetProt().GetName().empty()) 
                        {
                            string name = new_prot->GetData().GetProt().GetName().front();                   
                            name = "truncated "+name;
                            new_prot->SetData().SetProt().SetName().front() = name;
                        }
                    }
                    if (m_TrimProtFeats->GetValue())
                    {
                        TruncateProteinFeat(*new_prot, fi->GetOriginalFeature(), scope);
                    }
                    cmd->AddCommand(*CRef<CCmdChangeSeq_feat>(new CCmdChangeSeq_feat(prot_feat_ci->GetSeq_feat_Handle(), *new_prot)));
                }

                AdjustMolinfo(*prot_bsh.GetSeq_entry_Handle().GetCompleteSeq_entry(), cmd, *new_cds);
            }
        
            if (m_TruncateMRNA->GetValue())
            {
                CConstRef<CSeq_feat> mrna = sequence::GetmRNAforCDS(fi->GetOriginalFeature(), scope);
                if (mrna)
                {
                    CRef<CSeq_feat> new_mrna(new CSeq_feat());
                    new_mrna->Assign(*mrna);
                    CRef<CSeq_loc> loc(new CSeq_loc);
                    loc->Assign(new_cds->GetLocation());
                    new_mrna->SetLocation(*loc);
                    new_mrna->SetPartial(new_cds->GetLocation().IsPartialStart(eExtreme_Biological) || new_cds->GetLocation().IsPartialStop(eExtreme_Biological));
                    CSeq_feat_Handle mrna_h = scope.GetSeq_featHandle(*mrna);
                    cmd->AddCommand(*CRef<CCmdChangeSeq_feat>(new CCmdChangeSeq_feat(mrna_h, *new_mrna)));
                }
            }
            
            if (m_ResetGenes->GetValue())
            {
                CConstRef<CSeq_feat> gene = sequence::GetOverlappingGene(fi->GetLocation(), scope);
                if (gene)
                {
                    CRef<CSeq_feat> new_gene(new CSeq_feat());
                    new_gene->Assign(*gene);
                    CRef<CSeq_loc> loc = sequence::Seq_loc_Merge(new_cds->GetLocation(), CSeq_loc::fMerge_SingleRange, &scope);
                    new_gene->SetLocation(*loc);
                    new_gene->SetPartial(new_cds->GetLocation().IsPartialStart(eExtreme_Biological) || new_cds->GetLocation().IsPartialStop(eExtreme_Biological));
                    CSeq_feat_Handle gene_h = scope.GetSeq_featHandle(*gene);
                    cmd->AddCommand(*CRef<CCmdChangeSeq_feat>(new CCmdChangeSeq_feat(gene_h, *new_gene)));
                }
            }
        }          
    }

    return cmd;
}

static CRef<CSeq_loc> TruncateSeqLoc (const CSeq_loc& orig_loc, size_t new_len)
{
    CRef<CSeq_loc> new_loc;

    if (new_len == 0) {
        new_loc.Reset(new CSeq_loc);
        new_loc->SetPnt().SetPoint(0);
        new_loc->SetId(*orig_loc.GetId());
        return new_loc;
    }

    size_t len = 0;
    for (CSeq_loc_CI it(orig_loc); it && len < new_len; ++it) {
        size_t this_len = it.GetRange().GetLength();
        CConstRef<CSeq_loc> this_loc = it.GetRangeAsSeq_loc();
        if (len + this_len <= new_len) {
            if (new_loc) {
                new_loc->Add(*this_loc);
            } else {
                new_loc.Reset(new CSeq_loc());
                new_loc->Assign(*this_loc);
            }
            len += this_len;
        } else {
            CRef<CSeq_loc> partial_loc(new CSeq_loc());
            size_t len_wanted = new_len - len;
            size_t start = this_loc->GetStart(eExtreme_Biological);
            if (len_wanted == 1) {
                // make a point
                partial_loc->SetPnt().SetPoint(start);
            } else {
                // make an interval
                if (this_loc->IsSetStrand() && this_loc->GetStrand() == eNa_strand_minus) {
                    partial_loc->SetInt().SetFrom(start - len_wanted + 1);
                    partial_loc->SetInt().SetTo(start);
                } else {
                    partial_loc->SetInt().SetFrom(start);
                    partial_loc->SetInt().SetTo(start + len_wanted - 1);
                }
            }
            partial_loc->SetId(*this_loc->GetId());
            if (this_loc->IsSetStrand()) {
                partial_loc->SetStrand(this_loc->GetStrand());
            }
            if (new_loc) {
                new_loc->Add(*partial_loc);
            } else {
                new_loc.Reset(new CSeq_loc());
                new_loc->Assign(*partial_loc);
            }
            len += len_wanted;  
        }
    }

    return new_loc;
}

void CTruncateCDS::TruncateProteinFeat(CSeq_feat &prot_feat, const CSeq_feat& cds, CScope& scope)
{
    string prot_str;
    CSeqTranslator::Translate(cds, scope, prot_str);
    size_t pos = NStr::Find(prot_str, "*");
    if (pos != string::npos) {
        CRef<CSeq_loc> new_loc = TruncateSeqLoc (prot_feat.GetLocation(), pos);
        if (new_loc) {
            new_loc->SetPartialStop(false, eExtreme_Biological);
            prot_feat.SetLocation().Assign(*new_loc);
            if (prot_feat.GetLocation().IsPartialStart(eExtreme_Biological)) {
                prot_feat.SetPartial(true);
            } else {
                prot_feat.ResetPartial();
            }
        }
    }
}

void CTruncateCDS::AdjustMolinfo(const CSeq_entry& se, CRef<CCmdComposite> composite, const CSeq_feat& cds)
{
    bool found = false;
    CSeq_entry_Handle seh = m_TopSeqEntry.GetScope().GetSeq_entryHandle(se);
    FOR_EACH_SEQDESC_ON_SEQENTRY (it, se)
    {
        if ((*it)->IsMolinfo())
        {
            CRef<CSeqdesc> edited_desc(new CSeqdesc);
            edited_desc->Assign(**it);
            if (feature::AdjustProteinMolInfoToMatchCDS(edited_desc->SetMolinfo(),cds))
            {
                CRef<CCmdChangeSeqdesc> cmd(new CCmdChangeSeqdesc(seh, **it, *edited_desc));
                composite->AddCommand(*cmd);
            }
            found = true;
        }
    }
    if (!found)
    {
        CRef<CMolInfo> molinfo(new CMolInfo);
        CRef<CSeqdesc> desc(new CSeqdesc);
        feature::AdjustProteinMolInfoToMatchCDS(*molinfo,cds);
        desc->SetMolinfo(*molinfo);
        CCmdCreateDesc* cmd = new CCmdCreateDesc(seh,*desc);
        composite->AddCommand(*cmd);
    }
}



END_NCBI_SCOPE
