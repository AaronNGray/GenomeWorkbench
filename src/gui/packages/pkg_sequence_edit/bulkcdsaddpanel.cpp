/*  $Id: bulkcdsaddpanel.cpp 44318 2019-11-27 18:32:52Z filippov $
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
 * Authors:  Colleen Bollin
 */


#include <ncbi_pch.hpp>
#include <objmgr/bioseq_ci.hpp>
#include <objects/seqfeat/Cdregion.hpp>
#include <objects/general/Object_id.hpp>
#include <objects/seqloc/Seq_id.hpp>
#include <objtools/cleanup/cleanup.hpp>
#include <objtools/edit/cds_fix.hpp>
#include <objmgr/util/sequence.hpp>
#include <objects/seqfeat/RNA_ref.hpp>

////@begin includes
////@end includes
#include <sstream> 

#include <gui/widgets/edit/generic_report_dlg.hpp>
#include <gui/packages/pkg_sequence_edit/bulkcdsaddpanel.hpp>
#include <gui/packages/pkg_sequence_edit/subprep_util.hpp>
#include <gui/objutils/cmd_create_feat.hpp>
#include <gui/objutils/cmd_change_seq_feat.hpp>
#include <gui/objutils/cmd_add_seqentry.hpp>

////@begin XPM images
////@end XPM images

BEGIN_NCBI_SCOPE

/*!
 * CBulkCDSAddPanel type definition
 */

IMPLEMENT_DYNAMIC_CLASS( CBulkCDSAddPanel, CBulkCmdPanel )


/*!
 * CBulkCDSAddPanel event table definition
 */

BEGIN_EVENT_TABLE( CBulkCDSAddPanel, CBulkCmdPanel )

////@begin CBulkCDSAddPanel event table entries
////@end CBulkCDSAddPanel event table entries

END_EVENT_TABLE()


/*!
 * CBulkCDSAddPanel constructors
 */

CBulkCDSAddPanel::CBulkCDSAddPanel()
{
    Init();
}

CBulkCDSAddPanel::CBulkCDSAddPanel( wxWindow* parent, objects::CSeq_entry_Handle seh, bool add_location_panel, wxWindowID id, const wxPoint& pos, const wxSize& size, long style )
: m_TopSeqEntry(seh), CBulkCmdPanel(add_location_panel)
{
    Init();
    Create(parent, id, pos, size, style);
}


/*!
 * CBulkCDSAddPanel creator
 */

bool CBulkCDSAddPanel::Create( wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style )
{
////@begin CBulkCDSAddPanel creation
    CBulkCmdPanel::Create( parent, id, pos, size, style );

    CreateControls();
    if (GetSizer())
    {
        GetSizer()->SetSizeHints(this);
    }
    Centre();
////@end CBulkCDSAddPanel creation
    return true;
}


/*!
 * CBulkCDSAddPanel destructor
 */

CBulkCDSAddPanel::~CBulkCDSAddPanel()
{
////@begin CBulkCDSAddPanel destruction
////@end CBulkCDSAddPanel destruction
}


/*!
 * Member initialisation
 */

void CBulkCDSAddPanel::Init()
{
////@begin CBulkCDSAddPanel member initialisation
    m_ProteinName = NULL;
    m_ProteinDescription = NULL;
    m_GeneSymbol = NULL;
    m_Comment = NULL;
    m_Location = NULL;
    m_CodonStart = NULL;
    m_AddmRNA = NULL;
////@end CBulkCDSAddPanel member initialisation
}


/*!
 * Control creation for CBulkCDSAddPanel
 */

void CBulkCDSAddPanel::CreateControls()
{    
////@begin CBulkCDSAddPanel content construction
    CBulkCDSAddPanel* itemCBulkCmdPanel1 = this;

    wxBoxSizer* itemBoxSizer2 = new wxBoxSizer(wxVERTICAL);
    itemCBulkCmdPanel1->SetSizer(itemBoxSizer2);

    wxFlexGridSizer* itemFlexGridSizer3 = new wxFlexGridSizer(0, 2, 0, 0);
    itemBoxSizer2->Add(itemFlexGridSizer3, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

    wxStaticText* itemStaticText4 = new wxStaticText( itemCBulkCmdPanel1, wxID_STATIC, _("Protein Name"), wxDefaultPosition, wxDefaultSize, 0 );
    itemFlexGridSizer3->Add(itemStaticText4, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    m_ProteinName = new wxTextCtrl( itemCBulkCmdPanel1, ID_BULKCDSADD_PROTEIN_NAME, wxEmptyString, wxDefaultPosition, wxSize(200, -1), 0 );
    itemFlexGridSizer3->Add(m_ProteinName, 0, wxGROW|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxStaticText* itemStaticText6 = new wxStaticText( itemCBulkCmdPanel1, wxID_STATIC, _("Protein Description"), wxDefaultPosition, wxDefaultSize, 0 );
    itemFlexGridSizer3->Add(itemStaticText6, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    m_ProteinDescription = new wxTextCtrl( itemCBulkCmdPanel1, ID_BULKCDSADD_PROTEIN_DESC, wxEmptyString, wxDefaultPosition, wxSize(200, -1), 0 );
    itemFlexGridSizer3->Add(m_ProteinDescription, 0, wxGROW|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxStaticText* itemStaticText8 = new wxStaticText( itemCBulkCmdPanel1, wxID_STATIC, _("Gene Symbol"), wxDefaultPosition, wxDefaultSize, 0 );
    itemFlexGridSizer3->Add(itemStaticText8, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    m_GeneSymbol = new wxTextCtrl( itemCBulkCmdPanel1, ID_BULKCDSADD_GENE, wxEmptyString, wxDefaultPosition, wxSize(200, -1), 0 );
    itemFlexGridSizer3->Add(m_GeneSymbol, 0, wxGROW|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxStaticText* itemStaticText10 = new wxStaticText( itemCBulkCmdPanel1, wxID_STATIC, _("Comment"), wxDefaultPosition, wxDefaultSize, 0 );
    itemFlexGridSizer3->Add(itemStaticText10, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    m_Comment = new wxTextCtrl( itemCBulkCmdPanel1, ID_BULKCDSADD_COMMENT, wxEmptyString, wxDefaultPosition, wxSize(300, 70), wxTE_MULTILINE );
    itemFlexGridSizer3->Add(m_Comment, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    if (m_add_location_panel)
    {
        m_Location = new CBulkLocationPanel( itemCBulkCmdPanel1, ID_WINDOW, wxDefaultPosition, wxSize(100, 100), wxSIMPLE_BORDER );
        itemBoxSizer2->Add(m_Location, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);
    }

    wxBoxSizer* itemBoxSizer4 = new wxBoxSizer(wxHORIZONTAL);
    itemBoxSizer2->Add(itemBoxSizer4, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

    wxStaticText* itemStaticText11 = new wxStaticText( itemCBulkCmdPanel1, wxID_STATIC, _("Codon Start"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer4->Add(itemStaticText11, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxArrayString codonStrings;
    codonStrings.Add(_("1"));
    codonStrings.Add(_("2"));
    codonStrings.Add(_("3"));
    codonStrings.Add(_("Best"));
    m_CodonStart = new wxChoice( itemCBulkCmdPanel1, wxID_ANY, wxDefaultPosition, wxDefaultSize, codonStrings, 0 );
    m_CodonStart->SetStringSelection(_("Best"));
    itemBoxSizer4->Add(m_CodonStart, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    m_AddmRNA = new wxCheckBox( itemCBulkCmdPanel1, wxID_ANY, _("Also add mRNA"), wxDefaultPosition, wxDefaultSize, 0 );
    m_AddmRNA->SetValue(false);
    itemBoxSizer2->Add(m_AddmRNA, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

////@end CBulkCDSAddPanel content construction
}


/*!
 * Should we show tooltips?
 */

bool CBulkCDSAddPanel::ShowToolTips()
{
    return true;
}

/*!
 * Get bitmap resources
 */

wxBitmap CBulkCDSAddPanel::GetBitmapResource( const wxString& name )
{
    // Bitmap retrieval
////@begin CBulkCDSAddPanel bitmap retrieval
    wxUnusedVar(name);
    return wxNullBitmap;
////@end CBulkCDSAddPanel bitmap retrieval
}

/*!
 * Get icon resources
 */

wxIcon CBulkCDSAddPanel::GetIconResource( const wxString& name )
{
    // Icon retrieval
////@begin CBulkCDSAddPanel icon retrieval
    wxUnusedVar(name);
    return wxNullIcon;
////@end CBulkCDSAddPanel icon retrieval
}


CRef<CCmdComposite> CBulkCDSAddPanel::GetCommand()
{
    string prot_name = ToStdString(m_ProteinName->GetValue());
    if (NStr::IsBlank(prot_name)) {
        CRef<CCmdComposite> empty(NULL);
        return empty;
    }

    m_create_general_only = objects::edit::IsGeneralIdProtPresent(m_TopSeqEntry);
    CRef<CCmdComposite> cmd (new CCmdComposite("Bulk Add CDS"));   
    vector<string> ambig_bioseqs;
    objects::CBioseq_CI b_iter(m_TopSeqEntry, objects::CSeq_inst::eMol_na);
    for ( ; b_iter ; ++b_iter ) 
    {
        bool ambiguous(false);
        AddOneCommand(*b_iter, cmd, ambiguous);
        if (ambiguous)
        {
            CSeq_id_Handle best = sequence::GetId(*b_iter, sequence::eGetId_Best);
            string accession;
            best.GetSeqId()->GetLabel(&accession, CSeq_id::eContent);
            ambig_bioseqs.push_back(accession);
        }
    }
    if (!ambig_bioseqs.empty())
    {
        string report("Possible ambiguous frames detected in ");
        report +=  NStr::ULongToString(ambig_bioseqs.size()) + " records\n";
        report += NStr::Join(ambig_bioseqs, "\n");
        CGenericReportDlg* report_dlg = new CGenericReportDlg(NULL);
        report_dlg->SetTitle(wxT("Ambiguous Frames"));
        report_dlg->SetText(wxString(report));
        report_dlg->Show(true);
    }
    return cmd;
}

void CBulkCDSAddPanel::AddOneCommand(const CBioseq_Handle& bsh, CRef<CCmdComposite> cmd, bool &ambiguous)
{
    string prot_name = ToStdString(m_ProteinName->GetValue());
    string prot_desc = ToStdString(m_ProteinDescription->GetValue());
    string cds_comment = ToStdString(m_Comment->GetValue());
    string gene_symbol = ToStdString(m_GeneSymbol->GetValue());
    bool find_best_frame = false;
    CCdregion::EFrame codon_start = CCdregion::eFrame_not_set;
    if (m_CodonStart->GetSelection() == 0)
        codon_start = CCdregion::eFrame_one;
    if (m_CodonStart->GetSelection() == 1)
        codon_start = CCdregion::eFrame_two;
    if (m_CodonStart->GetSelection() == 2)
        codon_start = CCdregion::eFrame_three;
    if (m_CodonStart->GetSelection() == 3)
        find_best_frame = true;


    if (!ShouldAddToExisting() && AlreadyHasFeature(bsh, objects::CSeqFeatData::eSubtype_cdregion)) {
        return;
    }
    CRef<objects::CSeq_feat> cds(new objects::CSeq_feat());
    cds->SetData().SetCdregion();
    if (!NStr::IsBlank(cds_comment)) {
        cds->SetComment(cds_comment);
    }

    // add appropriate genetic code
    CRef<CGenetic_code> code = edit::GetGeneticCodeForBioseq(bsh);
    if (code) {
        cds->SetData().SetCdregion().SetCode(*code);
    }
    
    CRef<objects::CSeq_loc> loc = GetLocForBioseq(*(bsh.GetCompleteBioseq()));
    cds->SetLocation().Assign(*loc);
    cds->SetPartial(cds->GetLocation().IsPartialStart(objects::eExtreme_Biological) || cds->GetLocation().IsPartialStop(objects::eExtreme_Biological));

    objects::CSeq_entry_Handle seh = bsh.GetSeq_entry_Handle();
    CCleanup cleanup;   
 
    if (!NStr::IsBlank(gene_symbol)) {
        // create gene feature
        CRef<objects::CSeq_feat> new_gene(new objects::CSeq_feat());
        new_gene->SetData().SetGene().SetLocus(gene_symbol);
        new_gene->SetLocation().Assign(*loc);
        if (new_gene->GetLocation().IsPartialStart(objects::eExtreme_Biological)
            || new_gene->GetLocation().IsPartialStop(objects::eExtreme_Biological)) {
            new_gene->SetPartial(true);
        }
        cmd->AddCommand(*CRef<CCmdCreateFeat>(new CCmdCreateFeat(seh, *new_gene)));
        //cds->SetGeneXref().SetLocus(gene_symbol);
    }    
    
    if (m_AddmRNA->GetValue())
    {
        CRef<CSeq_feat> new_mrna(new CSeq_feat());
        new_mrna->SetData().SetRna().SetType(CRNA_ref::eType_mRNA);
        new_mrna->SetLocation(*loc);
        new_mrna->SetPartial(new_mrna->GetLocation().IsPartialStart(eExtreme_Biological) | new_mrna->GetLocation().IsPartialStop(eExtreme_Biological));
        new_mrna->SetData().SetRna().SetExt().SetName(prot_name);
        cmd->AddCommand(*(CRef<CCmdCreateFeat>(new CCmdCreateFeat(seh, *new_mrna))));
    }

    if (loc->IsPnt())
    {
        cds->SetData().SetCdregion().SetFrame(CCdregion::eFrame_not_set);
    }
    else
    {       
        if (find_best_frame)
            cds->SetData().SetCdregion().SetFrame(objects::CSeqTranslator::FindBestFrame(*cds, bsh.GetScope(), ambiguous));
        else
            cds->SetData().SetCdregion().SetFrame(codon_start);
        
        CRef<objects::CSeq_entry> protein = CreateTranslatedProteinSequence (cds, bsh, m_create_general_only);
        CRef<objects::CSeq_feat> prot = AddProteinFeatureToProtein (protein,
                                                                    cds->GetLocation().IsPartialStart(objects::eExtreme_Biological), 
                                                                    cds->GetLocation().IsPartialStop(objects::eExtreme_Biological)); 
    
        if (!NStr::IsBlank(prot_name)) {
            prot->SetData().SetProt().SetName().push_back(prot_name);
        }
        if (!NStr::IsBlank(prot_desc)) {
            prot->SetData().SetProt().SetDesc(prot_desc);
        }
        
        cleanup.BasicCleanup(*prot);
        cmd->AddCommand(*CRef<objects::CCmdAddSeqEntry>(new objects::CCmdAddSeqEntry(protein, seh)));
    }

    cleanup.BasicCleanup(*cds);
    cmd->AddCommand(*CRef<CCmdCreateFeat>(new CCmdCreateFeat(seh, *cds)));
}

string CBulkCDSAddPanel::GetErrorMessage()
{
    string prot_name = ToStdString(m_ProteinName->GetValue());
    if (NStr::IsBlank(prot_name)) {
        return "You must specify protein name.";
    } else {
        return "Unknown error";
    }
}

END_NCBI_SCOPE
