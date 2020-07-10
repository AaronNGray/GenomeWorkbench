/*  $Id: molinfo_edit_dlg.cpp 42224 2019-01-16 16:10:33Z asztalos $
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
#include <objmgr/feat_ci.hpp>
#include <objmgr/bioseq_ci.hpp>
#include <objmgr/seq_feat_handle.hpp>
#include <objmgr/util/feature.hpp>
#include <gui/objutils/util_cmds.hpp>
#include <gui/packages/pkg_sequence_edit/molinfo_edit_dlg.hpp>

#include <util/xregexp/regexp.hpp>

#include <wx/button.h>
#include <wx/statbox.h>
#include <wx/stattext.h>
#include <wx/msgdlg.h>

////@begin XPM images
////@end XPM images

BEGIN_NCBI_SCOPE



IMPLEMENT_DYNAMIC_CLASS( CMolInfoEditDlg, wxDialog )


BEGIN_EVENT_TABLE( CMolInfoEditDlg, wxDialog )


END_EVENT_TABLE()

CMolInfoEditDlg::CMolInfoEditDlg()
{
    Init();
}

CMolInfoEditDlg::CMolInfoEditDlg( wxWindow* parent, objects::CSeq_entry_Handle seh, 
                                        wxWindowID id, const wxString& caption, const wxPoint& pos, const wxSize& size, long style )
: m_TopSeqEntry(seh)
{
    Init();
    Create(parent, id, caption, pos, size, style);
}


bool CMolInfoEditDlg::Create( wxWindow* parent, wxWindowID id, const wxString& caption, const wxPoint& pos, const wxSize& size, long style )
{
    SetExtraStyle(wxWS_EX_BLOCK_EVENTS);
    wxDialog::Create( parent, id, caption, pos, size, style );

    CreateControls();
    if (GetSizer())
    {
        GetSizer()->SetSizeHints(this);
    }
    Centre();

    return true;
}


CMolInfoEditDlg::~CMolInfoEditDlg()
{
}


/*!
 * Member initialisation
 */

void CMolInfoEditDlg::Init()
{

    m_biomol[" "] = CMolInfo::eBiomol_unknown;
    m_biomol["genomic"] = CMolInfo::eBiomol_genomic;
    m_biomol["precursor RNA"] =        CMolInfo::eBiomol_pre_RNA;
    m_biomol["mRNA"] =        CMolInfo::eBiomol_mRNA;
    m_biomol["rRNA"] =    CMolInfo::eBiomol_rRNA;
    m_biomol["tRNA"] =   CMolInfo::eBiomol_tRNA;
//        CMolInfo::eBiomol_snRNA  
//        CMolInfo::eBiomol_scRNA  
//        CMolInfo::eBiomol_peptide
    m_biomol["other-genetic"] =    CMolInfo::eBiomol_other_genetic;
    m_biomol["genomic mRNA"] =   CMolInfo::eBiomol_genomic_mRNA;
    m_biomol["cRNA"] = CMolInfo::eBiomol_cRNA;
//        CMolInfo::eBiomol_snoRNA       
    m_biomol["transcribed RNA"] = CMolInfo::eBiomol_transcribed_RNA;
    m_biomol["ncRNA"] =   CMolInfo::eBiomol_ncRNA;
    m_biomol["tmRNA"] =    CMolInfo::eBiomol_tmRNA;
//        CMolInfo::eBiomol_other        

    m_tech[" "] = CMolInfo::eTech_unknown;        
    m_tech["standard"] =    CMolInfo::eTech_standard;   
    m_tech["EST"] =   CMolInfo::eTech_est;
    m_tech["STS"] =   CMolInfo::eTech_sts;
    m_tech["survey"] =   CMolInfo::eTech_survey;
    m_tech["genetic map"] =   CMolInfo::eTech_genemap;
    m_tech["physical map"] =   CMolInfo::eTech_physmap;
    m_tech["derived"] =   CMolInfo::eTech_derived;   
    m_tech["concept-trans"] =  CMolInfo::eTech_concept_trans;
    m_tech["seq-pept"] =  CMolInfo::eTech_seq_pept; 
    m_tech["both"] =  CMolInfo::eTech_both;      
    m_tech["seq-pept-overlap"] =  CMolInfo::eTech_seq_pept_overlap;
    m_tech["seq-pept-homol"] =   CMolInfo::eTech_seq_pept_homol;
    m_tech["concept-trans-a"] =  CMolInfo::eTech_concept_trans_a;
    m_tech["HTGS-1"] =  CMolInfo::eTech_htgs_1;
    m_tech["HTGS-2"] =  CMolInfo::eTech_htgs_2;
    m_tech["HTGS-3"] =  CMolInfo::eTech_htgs_3;
    m_tech["fli-cDNA"] = CMolInfo::eTech_fli_cdna;
    m_tech["HTGS-0"] = CMolInfo::eTech_htgs_0;
    m_tech["HTC"] = CMolInfo::eTech_htc;             
    m_tech["WGS"] = CMolInfo::eTech_wgs;
    m_tech["BARCODE"] = CMolInfo::eTech_barcode;
    m_tech["composite WGS-HTGS"] =  CMolInfo::eTech_composite_wgs_htgs;
    m_tech["TSA"] =  CMolInfo::eTech_tsa;
    m_tech["other"] =  CMolInfo::eTech_other;  

    m_complete[" "] = CMolInfo::eCompleteness_unknown;
    m_complete["complete"] = CMolInfo::eCompleteness_complete;
    m_complete["partial"] = CMolInfo::eCompleteness_partial;
    m_complete["no left"] = CMolInfo::eCompleteness_no_left;
    m_complete["no right"] = CMolInfo::eCompleteness_no_right;
    m_complete["no ends"] = CMolInfo::eCompleteness_no_ends;
    m_complete["has left"] = CMolInfo::eCompleteness_has_left;
    m_complete["has right"] = CMolInfo::eCompleteness_has_right;
    m_complete["other"] = CMolInfo::eCompleteness_other;

    m_class[" "] = CSeq_inst::eMol_not_set;
    m_class["DNA"] = CSeq_inst::eMol_dna;
    m_class["RNA"] = CSeq_inst::eMol_rna;
    m_class["protein"] = CSeq_inst::eMol_aa;
    m_class["nucleotide"] = CSeq_inst::eMol_na;
    m_class["other"] = CSeq_inst::eMol_other;

    m_topology[" "] = CSeq_inst::eTopology_not_set;
    m_topology["linear"] = CSeq_inst::eTopology_linear;
    m_topology["circular"] = CSeq_inst::eTopology_circular;
    m_topology["tandem"] = CSeq_inst::eTopology_tandem;
    m_topology["other"] = CSeq_inst::eTopology_other;
    

    m_strand[" "] = CSeq_inst::eStrand_not_set;
    m_strand["single"] = CSeq_inst::eStrand_ss;
    m_strand["double"] = CSeq_inst::eStrand_ds;
    m_strand["mixed"] = CSeq_inst::eStrand_mixed;
    m_strand["other"] = CSeq_inst::eStrand_other;
}




void CMolInfoEditDlg::CreateControls()
{    
    CMolInfoEditDlg* itemDialog1 = this;

    wxBoxSizer* itemBoxSizer1 = new wxBoxSizer(wxVERTICAL);
    itemDialog1->SetSizer(itemBoxSizer1);


    wxBoxSizer* itemBoxSizer2 = new wxBoxSizer(wxHORIZONTAL);
    itemBoxSizer1->Add(itemBoxSizer2, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 3);
    wxStaticText* itemStaticText1 = new wxStaticText( itemDialog1, wxID_STATIC, _(" "), wxDefaultPosition, wxSize(100, -1), wxALIGN_CENTRE );
    itemBoxSizer2->Add(itemStaticText1, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxTOP, 5);
    wxStaticText* itemStaticText2 = new wxStaticText( itemDialog1, wxID_STATIC, _("From"), wxDefaultPosition, wxSize(100, -1), wxALIGN_CENTRE|wxST_NO_AUTORESIZE );
    itemBoxSizer2->Add(itemStaticText2, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxTOP, 5);
    wxStaticText* itemStaticText3 = new wxStaticText( itemDialog1, wxID_STATIC, _(" "), wxDefaultPosition, wxSize(100, -1), wxALIGN_CENTRE );
    itemBoxSizer2->Add(itemStaticText3, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxTOP, 5);
    wxStaticText* itemStaticText4 = new wxStaticText( itemDialog1, wxID_STATIC, _("To"), wxDefaultPosition, wxSize(100, -1), wxALIGN_CENTRE|wxST_NO_AUTORESIZE );
    itemBoxSizer2->Add(itemStaticText4, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxTOP, 5);

    wxBoxSizer* itemBoxSizer3 = new wxBoxSizer(wxHORIZONTAL);
    itemBoxSizer1->Add(itemBoxSizer3, 0, wxALIGN_CENTER_HORIZONTAL|wxRIGHT, 3);

    wxStaticText* itemStaticText5 = new wxStaticText( itemDialog1, wxID_STATIC, _("Molecule"), wxDefaultPosition, wxSize(100, -1), wxALIGN_CENTRE );
    itemBoxSizer3->Add(itemStaticText5, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxALL, 2);
    
    wxArrayString molecule1,molecule2;
    molecule1.Add(ToWxString("Any"));
    molecule2.Add(ToWxString("No change"));
    for (map<string,int>::iterator i = m_biomol.begin(); i!=m_biomol.end(); ++i)
    {
        molecule1.Add(ToWxString(i->first));
        molecule2.Add(ToWxString(i->first));
    }    
    m_ChoiceMolecule1 = new wxChoice(itemDialog1, wxID_ANY, wxDefaultPosition, wxSize(100, -1), molecule1,0,wxDefaultValidator);
    m_ChoiceMolecule1->SetSelection(0);
    itemBoxSizer3->Add(m_ChoiceMolecule1, 0, wxALIGN_LEFT|wxALL, 2);

    wxStaticText* itemStaticText6 = new wxStaticText( itemDialog1, wxID_STATIC, _("Molecule"), wxDefaultPosition, wxSize(100, -1), wxALIGN_CENTRE );
    itemBoxSizer3->Add(itemStaticText6, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxALL, 2);
    
    m_ChoiceMolecule2 = new wxChoice(itemDialog1, wxID_ANY, wxDefaultPosition, wxSize(100, -1), molecule2,0,wxDefaultValidator);
    m_ChoiceMolecule2->SetSelection(0);
    itemBoxSizer3->Add(m_ChoiceMolecule2, 0, wxALIGN_LEFT|wxALL, 2);

    wxBoxSizer* itemBoxSizer4 = new wxBoxSizer(wxHORIZONTAL);
    itemBoxSizer1->Add(itemBoxSizer4, 0, wxALIGN_CENTER_HORIZONTAL|wxRIGHT, 3);

    wxStaticText* itemStaticText7 = new wxStaticText( itemDialog1, wxID_STATIC, _("Technique"), wxDefaultPosition, wxSize(100, -1), wxALIGN_CENTRE );
    itemBoxSizer4->Add(itemStaticText7, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxALL, 2);
    
    wxArrayString technique1,technique2;
    technique1.Add(ToWxString("Any"));
    technique2.Add(ToWxString("No change"));
    for (map<string,int>::iterator i = m_tech.begin(); i!=m_tech.end(); ++i)
    {
        technique1.Add(ToWxString(i->first));
        technique2.Add(ToWxString(i->first));
    }    
    m_ChoiceTechnique1 = new wxChoice(itemDialog1, wxID_ANY, wxDefaultPosition, wxSize(100, -1), technique1,0,wxDefaultValidator);
    m_ChoiceTechnique1->SetSelection(0);
    itemBoxSizer4->Add(m_ChoiceTechnique1, 0, wxALIGN_LEFT|wxALL, 2);
    
    wxStaticText* itemStaticText8 = new wxStaticText( itemDialog1, wxID_STATIC, _("Technique"), wxDefaultPosition, wxSize(100, -1), wxALIGN_CENTRE );
    itemBoxSizer4->Add(itemStaticText8, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxALL, 2);

    m_ChoiceTechnique2 = new wxChoice(itemDialog1, wxID_ANY, wxDefaultPosition, wxSize(100, -1), technique2,0,wxDefaultValidator);
    m_ChoiceTechnique2->SetSelection(0);
    itemBoxSizer4->Add(m_ChoiceTechnique2, 0, wxALIGN_LEFT|wxALL, 2);

    wxBoxSizer* itemBoxSizer5 = new wxBoxSizer(wxHORIZONTAL);
    itemBoxSizer1->Add(itemBoxSizer5, 0, wxALIGN_CENTER_HORIZONTAL|wxRIGHT, 3);

    wxStaticText* itemStaticText9 = new wxStaticText( itemDialog1, wxID_STATIC, _("Completeness"), wxDefaultPosition, wxSize(100, -1), wxALIGN_CENTRE );
    itemBoxSizer5->Add(itemStaticText9, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxALL, 2);
    
    wxArrayString complete1,complete2;
    complete1.Add(ToWxString("Any"));
    complete2.Add(ToWxString("No change"));
    for (map<string,int>::iterator i = m_complete.begin(); i!=m_complete.end(); ++i)
    {
        complete1.Add(ToWxString(i->first));
        complete2.Add(ToWxString(i->first));
    }    
    m_ChoiceComplete1 = new wxChoice(itemDialog1, wxID_ANY, wxDefaultPosition, wxSize(100, -1), complete1,0,wxDefaultValidator);
    m_ChoiceComplete1->SetSelection(0);
    itemBoxSizer5->Add(m_ChoiceComplete1, 0, wxALIGN_LEFT|wxALL, 2);
    
    wxStaticText* itemStaticText10 = new wxStaticText( itemDialog1, wxID_STATIC, _("Completeness"), wxDefaultPosition, wxSize(100, -1), wxALIGN_CENTRE );
    itemBoxSizer5->Add(itemStaticText10, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxALL, 2);
    
    m_ChoiceComplete2 = new wxChoice(itemDialog1, wxID_ANY, wxDefaultPosition, wxSize(100, -1), complete2,0,wxDefaultValidator);
    m_ChoiceComplete2->SetSelection(0);
    itemBoxSizer5->Add(m_ChoiceComplete2, 0, wxALIGN_LEFT|wxALL, 2);

    wxBoxSizer* itemBoxSizer6 = new wxBoxSizer(wxHORIZONTAL);
    itemBoxSizer1->Add(itemBoxSizer6, 0, wxALIGN_CENTER_HORIZONTAL|wxRIGHT, 3);

    wxStaticText* itemStaticText11 = new wxStaticText( itemDialog1, wxID_STATIC, _("Class"), wxDefaultPosition, wxSize(100, -1), wxALIGN_CENTRE );
    itemBoxSizer6->Add(itemStaticText11, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxALL, 2);
    
    wxArrayString class1,class2;
    class1.Add(ToWxString("Any"));
    class2.Add(ToWxString("No change"));
    for (map<string,CSeq_inst::EMol>::iterator i = m_class.begin(); i!=m_class.end(); ++i)
    {
        class1.Add(ToWxString(i->first));
        class2.Add(ToWxString(i->first));
    }    
    m_ChoiceClass1 = new wxChoice(itemDialog1, wxID_ANY, wxDefaultPosition, wxSize(100, -1), class1,0,wxDefaultValidator);
    m_ChoiceClass1->SetSelection(0);
    itemBoxSizer6->Add(m_ChoiceClass1, 0, wxALIGN_LEFT|wxALL, 2);

    wxStaticText* itemStaticText12 = new wxStaticText( itemDialog1, wxID_STATIC, _("Class"), wxDefaultPosition, wxSize(100, -1), wxALIGN_CENTRE );
    itemBoxSizer6->Add(itemStaticText12, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxALL, 2);

    m_ChoiceClass2 = new wxChoice(itemDialog1, wxID_ANY, wxDefaultPosition, wxSize(100, -1), class2,0,wxDefaultValidator);
    m_ChoiceClass2->SetSelection(0);
    itemBoxSizer6->Add(m_ChoiceClass2, 0, wxALIGN_LEFT|wxALL, 2);

    wxBoxSizer* itemBoxSizer7 = new wxBoxSizer(wxHORIZONTAL);
    itemBoxSizer1->Add(itemBoxSizer7, 0, wxALIGN_CENTER_HORIZONTAL|wxRIGHT, 3);
    
    wxStaticText* itemStaticText13 = new wxStaticText( itemDialog1, wxID_STATIC, _("Topology"), wxDefaultPosition, wxSize(100, -1), wxALIGN_CENTRE );
    itemBoxSizer7->Add(itemStaticText13, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxALL, 2);
    
    wxArrayString topology1,topology2;
    topology1.Add(ToWxString("Any"));
    topology2.Add(ToWxString("No change"));
    for (map<string,CSeq_inst::ETopology>::iterator i = m_topology.begin(); i!=m_topology.end(); ++i)
    {
        topology1.Add(ToWxString(i->first));
        topology2.Add(ToWxString(i->first));
    }    
    m_ChoiceTopology1 = new wxChoice(itemDialog1, wxID_ANY, wxDefaultPosition, wxSize(100, -1), topology1,0,wxDefaultValidator);
    m_ChoiceTopology1->SetSelection(0);
    itemBoxSizer7->Add(m_ChoiceTopology1, 0, wxALIGN_LEFT|wxALL, 2);

    wxStaticText* itemStaticText14 = new wxStaticText( itemDialog1, wxID_STATIC, _("Topology"), wxDefaultPosition, wxSize(100, -1), wxALIGN_CENTRE );
    itemBoxSizer7->Add(itemStaticText14, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxALL, 2);

    m_ChoiceTopology2 = new wxChoice(itemDialog1, wxID_ANY, wxDefaultPosition, wxSize(100, -1), topology2,0,wxDefaultValidator);
    m_ChoiceTopology2->SetSelection(0);
    itemBoxSizer7->Add(m_ChoiceTopology2, 0, wxALIGN_LEFT|wxALL, 2);

    wxBoxSizer* itemBoxSizer8 = new wxBoxSizer(wxHORIZONTAL);
    itemBoxSizer1->Add(itemBoxSizer8, 0, wxALIGN_CENTER_HORIZONTAL|wxRIGHT, 3);

    wxStaticText* itemStaticText15 = new wxStaticText( itemDialog1, wxID_STATIC, _("Strand"), wxDefaultPosition, wxSize(100, -1), wxALIGN_CENTRE );
    itemBoxSizer8->Add(itemStaticText15, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxALL, 2);
    
    wxArrayString strand1,strand2;
    strand1.Add(ToWxString("Any"));
    strand2.Add(ToWxString("No change"));
    for (map<string,CSeq_inst::EStrand>::iterator i = m_strand.begin(); i!=m_strand.end(); ++i)
    {
        strand1.Add(ToWxString(i->first));
        strand2.Add(ToWxString(i->first));
    }    
    m_ChoiceStrand1 = new wxChoice(itemDialog1, wxID_ANY, wxDefaultPosition, wxSize(100, -1), strand1,0,wxDefaultValidator);
    m_ChoiceStrand1->SetSelection(0);
    itemBoxSizer8->Add(m_ChoiceStrand1, 0, wxALIGN_LEFT|wxALL, 2);

    wxStaticText* itemStaticText16 = new wxStaticText( itemDialog1, wxID_STATIC, _("Strand"), wxDefaultPosition, wxSize(100, -1), wxALIGN_CENTRE );
    itemBoxSizer8->Add(itemStaticText16, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxALL, 2);

    m_ChoiceStrand2 = new wxChoice(itemDialog1, wxID_ANY, wxDefaultPosition, wxSize(100, -1), strand2,0,wxDefaultValidator);
    m_ChoiceStrand2->SetSelection(0);
    itemBoxSizer8->Add(m_ChoiceStrand2, 0, wxALIGN_LEFT|wxALL, 2);

    wxBoxSizer* itemBoxSizer9 = new wxBoxSizer(wxHORIZONTAL);
    itemBoxSizer1->Add(itemBoxSizer9, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 3);

    m_CreateNew = new wxCheckBox( itemDialog1,wxID_ANY, _("Create MolInfo descriptors if not present"));
    itemBoxSizer9->Add(m_CreateNew, 0, wxALIGN_LEFT|wxALL, 4);

    wxBoxSizer* itemBoxSizer10 = new wxBoxSizer(wxHORIZONTAL);
    itemBoxSizer1->Add(itemBoxSizer10, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 2);
    m_RBAny = new wxRadioButton( itemDialog1, wxID_ANY, _("Any Sequence"), wxDefaultPosition, wxDefaultSize, wxRB_GROUP );
    m_RBAny->SetValue(false);
    itemBoxSizer10->Add(m_RBAny, 0, wxALIGN_LEFT|wxALL, 4);
    m_RBNuc = new wxRadioButton( itemDialog1, wxID_ANY, _("Nucleotides"));
    m_RBNuc->SetValue(true);
    itemBoxSizer10->Add(m_RBNuc, 0, wxALIGN_LEFT|wxALL, 4);
    m_RBDna = new wxRadioButton( itemDialog1, wxID_ANY, _("DNA"));
    m_RBDna->SetValue(false);
    itemBoxSizer10->Add(m_RBDna, 0, wxALIGN_LEFT|wxALL, 4);
    m_RBProt = new wxRadioButton( itemDialog1, wxID_ANY, _("Proteins"));
    m_RBProt->SetValue(false);
    itemBoxSizer10->Add(m_RBProt, 0, wxALIGN_LEFT|wxALL, 4);
    m_RBRna = new wxRadioButton( itemDialog1, wxID_ANY, _("RNA"));
    m_RBRna->SetValue(false);
    itemBoxSizer10->Add(m_RBRna, 0, wxALIGN_LEFT|wxALL, 4);

    wxBoxSizer* itemBoxSizer11 = new wxBoxSizer(wxHORIZONTAL);
    itemBoxSizer1->Add(itemBoxSizer11, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 2);
    wxButton* itemButton1 = new wxButton( itemDialog1, wxID_OK, _("Accept"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer11->Add(itemButton1, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);
    wxButton* itemButton2 = new wxButton( itemDialog1, wxID_CANCEL, _("Cancel"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer11->Add(itemButton2, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

}


/*!
 * Should we show tooltips?
 */

bool CMolInfoEditDlg::ShowToolTips()
{
    return true;
}

/*!
 * Get bitmap resources
 */

wxBitmap CMolInfoEditDlg::GetBitmapResource( const wxString& name )
{
    // Bitmap retrieval
    wxUnusedVar(name);
    return wxNullBitmap;
}

/*!
 * Get icon resources
 */

wxIcon CMolInfoEditDlg::GetIconResource( const wxString& name )
{
    // Icon retrieval
    wxUnusedVar(name);
    return wxNullIcon;
}

CRef<CCmdComposite> CMolInfoEditDlg::GetCommand()
{
    CRef<CCmdComposite> cmd( new CCmdComposite("Remove Descriptors") );
    if (m_TopSeqEntry)
    {
        int item_mol1 = m_ChoiceMolecule1->GetSelection();
        int item_mol2 = m_ChoiceMolecule2->GetSelection();
        int item_tech1 = m_ChoiceTechnique1->GetSelection();
        int item_tech2 = m_ChoiceTechnique2->GetSelection();
        int item_complete1 = m_ChoiceComplete1->GetSelection();
        int item_complete2 = m_ChoiceComplete2->GetSelection();
        if (item_mol1 != wxNOT_FOUND && item_mol2 != wxNOT_FOUND &&
            item_tech1 != wxNOT_FOUND && item_tech2 != wxNOT_FOUND &&
            item_complete1 != wxNOT_FOUND && item_complete2 != wxNOT_FOUND)
        {
            
            string choice_mol1 = ToStdString(m_ChoiceMolecule1->GetString(item_mol1));
            string choice_mol2 = ToStdString(m_ChoiceMolecule2->GetString(item_mol2));
            int sel_mol1,sel_mol2;
            if (item_mol1 == 0) sel_mol1 = -1;
            else sel_mol1 = m_biomol[choice_mol1];
            if (item_mol2 == 0) sel_mol2 = -1;
            else sel_mol2 = m_biomol[choice_mol2];
            string choice_tech1 = ToStdString(m_ChoiceTechnique1->GetString(item_tech1));
            string choice_tech2 = ToStdString(m_ChoiceTechnique2->GetString(item_tech2));
            int sel_tech1,sel_tech2;
            if (item_tech1 == 0) sel_tech1 = -1;
            else sel_tech1 = m_tech[choice_tech1];
            if (item_tech2 == 0) sel_tech2 = -1;
            else sel_tech2 = m_tech[choice_tech2];
            string choice_complete1 = ToStdString(m_ChoiceComplete1->GetString(item_complete1));
            string choice_complete2 = ToStdString(m_ChoiceComplete2->GetString(item_complete2));
            int sel_complete1,sel_complete2;
            if (item_complete1 == 0) sel_complete1 = -1;
            else sel_complete1 = m_complete[choice_complete1];
            if (item_complete2 == 0) sel_complete2 = -1;
            else sel_complete2 = m_complete[choice_complete2];
            ApplyToMolInfo (*(m_TopSeqEntry.GetCompleteSeq_entry()), cmd, sel_mol1, sel_mol2, sel_tech1, sel_tech2, sel_complete1, sel_complete2); 
        }

        int item_class1 = m_ChoiceClass1->GetSelection();
        int item_class2 = m_ChoiceClass2->GetSelection();
        int item_topology1 = m_ChoiceTopology1->GetSelection();
        int item_topology2 = m_ChoiceTopology2->GetSelection();
        int item_strand1 = m_ChoiceStrand1->GetSelection();
        int item_strand2 = m_ChoiceStrand2->GetSelection();
        if (item_class1 != wxNOT_FOUND && item_class2 != wxNOT_FOUND &&                       
            item_topology1 != wxNOT_FOUND && item_topology2 != wxNOT_FOUND &&
            item_strand1 != wxNOT_FOUND && item_strand2 != wxNOT_FOUND)
        {
            
            string choice_class1 = ToStdString(m_ChoiceClass1->GetString(item_class1));
            string choice_class2 = ToStdString(m_ChoiceClass2->GetString(item_class2));
            int sel_class1,sel_class2;
            if (item_class1 == 0) sel_class1 = -1;
            else sel_class1 = static_cast<int>(m_class[choice_class1]);
            if (item_class2 == 0) sel_class2 = -1;
            else sel_class2 = static_cast<int>(m_class[choice_class2]);
            string choice_topology1 = ToStdString(m_ChoiceTopology1->GetString(item_topology1));
            string choice_topology2 = ToStdString(m_ChoiceTopology2->GetString(item_topology2));
            int sel_topology1,sel_topology2;
            if (item_topology1 == 0) sel_topology1 = -1;
            else sel_topology1 = static_cast<int>(m_topology[choice_topology1]);
            if (item_topology2 == 0) sel_topology2 = -1;
            else sel_topology2 = static_cast<int>(m_topology[choice_topology2]);
            string choice_strand1 = ToStdString(m_ChoiceStrand1->GetString(item_strand1));
            string choice_strand2 = ToStdString(m_ChoiceStrand2->GetString(item_strand2));
            int sel_strand1,sel_strand2;
            if (item_strand1 == 0) sel_strand1 = -1;
            else sel_strand1 = static_cast<int>(m_strand[choice_strand1]);
            if (item_strand2 == 0) sel_strand2 = -1;
            else sel_strand2 = static_cast<int>(m_strand[choice_strand2]);
            ApplyToInst (*(m_TopSeqEntry.GetCompleteSeq_entry()), cmd, sel_class1, sel_class2, sel_topology1, sel_topology2, sel_strand1, sel_strand2);
        }
    }
    return cmd;
}

CMolInfo::ECompleteness CMolInfoEditDlg::compute_peptide_completeness(CSeq_entry_Handle seh)
{
    CMolInfo::ECompleteness res = CMolInfo::eCompleteness_unknown;
    CSeq_entry_Handle pseh = seh.GetParentEntry();
    CFeat_CI prot(pseh, CSeqFeatData::eSubtype_prot);
    if (! prot) return res;
    while (prot)
    {
        if (prot->IsSetData() && prot->GetData().IsProt() && prot->GetData().GetProt().IsSetProcessed()) 
        {
            CProt_ref::TProcessed processed = prot->GetData().GetProt().GetProcessed();
            if (processed == CProt_ref::eProcessed_not_set)
            {
                const CSeq_loc& prot_loc = prot->GetLocation();
                bool partial5 = prot_loc.IsPartialStart(eExtreme_Biological);
                bool partial3 = prot_loc.IsPartialStop(eExtreme_Biological);
                if (!partial3 && !partial5)
                    res = CMolInfo::eCompleteness_complete;
                else if (partial3 && !partial5)
                    res = CMolInfo::eCompleteness_no_right;
                else if (!partial3 && partial5)
                    res = CMolInfo::eCompleteness_no_left;
                else if (partial3 && partial5)
                    res = CMolInfo::eCompleteness_no_ends;
            }
        }
        ++prot;
    }
    return res;
}


void CMolInfoEditDlg::CreateNewMolInfo(const CSeq_entry& se, CCmdComposite* composite, int choice_mol2, int choice_tech2, int choice_complete2) 
{
    if (choice_mol2 == -1 && choice_tech2 == -1 && choice_complete2 == -1)
    {
        wxMessageBox(wxT("Nothing set for created MolInfo records"), wxT("Error"), wxOK | wxICON_ERROR, this);
        return;
    }
  CRef<CMolInfo> molinfo(new CMolInfo);
  CRef<CSeqdesc> desc(new CSeqdesc);
  CSeq_entry_Handle seh = m_TopSeqEntry.GetScope().GetSeq_entryHandle(se);

  if (se.IsSeq() && se.GetSeq().IsSetInst() && se.GetSeq().GetInst().IsSetMol() && se.GetSeq().GetInst().GetMol() == CSeq_inst::eMol_aa)
  {
      molinfo->SetBiomol(CMolInfo::eBiomol_peptide);
      molinfo->SetCompleteness(compute_peptide_completeness(seh));
  }
  else
  {
      molinfo->SetBiomol(CMolInfo::eBiomol_unknown); 
      molinfo->SetCompleteness(CMolInfo::eCompleteness_unknown); 
  }
  
  if (choice_mol2 != -1)
      molinfo->SetBiomol(choice_mol2);

  if (choice_complete2 != -1)
      molinfo->SetCompleteness(choice_complete2);
  
  if (choice_tech2 != -1)
      molinfo->SetTech(choice_tech2);
  else
      molinfo->SetTech(CMolInfo::eTech_unknown);
  
  desc->SetMolinfo(*molinfo);
  CCmdCreateDesc* cmd = new CCmdCreateDesc(seh,*desc);
  composite->AddCommand(*cmd);
}

void CMolInfoEditDlg::ApplyToMolInfo (const CSeq_entry& se, CCmdComposite* composite, int choice_mol1, int choice_mol2, int choice_tech1, int choice_tech2, int choice_complete1, int choice_complete2)  
{

    if (se.IsSeq() && se.GetSeq().IsSetInst())
    {
        const CSeq_inst& inst = se.GetSeq().GetInst();
        if ( (inst.IsNa() && m_RBNuc->GetValue())  ||
             (inst.IsAa() && m_RBProt->GetValue()) ||
             (inst.IsSetMol() && inst.GetMol() == CSeq_inst::eMol_dna && m_RBDna->GetValue()) ||
             (inst.IsSetMol() && inst.GetMol() == CSeq_inst::eMol_rna && m_RBRna->GetValue()) ||
             m_RBAny->GetValue())
        {
            bool found = false;
            FOR_EACH_SEQDESC_ON_SEQENTRY (it, se) 
            {
                if ((*it)->IsMolinfo())
                {
                    found = true;
                    const CMolInfo& molinfo = (*it)->GetMolinfo();

                    CRef<CSeqdesc> edited_desc(new CSeqdesc);
                    edited_desc->Assign(**it);
                    CMolInfo& edited_molinfo = edited_desc->SetMolinfo();
                    
                    bool modified = false;
                    if (molinfo.IsSetBiomol())
                    {
                        if ((molinfo.GetBiomol() == choice_mol1 || choice_mol1 == -1) && choice_mol2 != -1 && choice_mol1 != choice_mol2)
                        {
                            modified = true;
                            edited_molinfo.SetBiomol(choice_mol2);
                        }
                    }
                    else if (choice_mol2 != -1)
                    {
                        modified = true;
                        edited_molinfo.SetBiomol(choice_mol2); 
                    }
                    if (molinfo.IsSetTech())
                    {
                        if ((molinfo.GetTech() == choice_tech1 || choice_tech1 == -1) && choice_tech2 != -1 && choice_tech1 != choice_tech2)
                        {
                            modified = true;
                            edited_molinfo.SetTech(choice_tech2);
                        }
                    }
                    else if (choice_tech2 != -1)
                    {
                        modified = true;
                        edited_molinfo.SetTech(choice_tech2); 
                    }
                    if (molinfo.IsSetCompleteness())
                    {
                        if ((molinfo.GetCompleteness() == choice_complete1 || choice_complete1 == -1) && choice_complete2 != -1 && choice_complete1 != choice_complete2)
                        {
                            modified = true;
                            edited_molinfo.SetCompleteness(choice_complete2);
                        }
                    }
                    else if (choice_complete2 != -1)
                    {
                        modified = true;
                        edited_molinfo.SetCompleteness(choice_complete2);  
                    }
                    if (modified)
                    {
                        CRef<CCmdChangeSeqdesc> cmd(new CCmdChangeSeqdesc(m_TopSeqEntry.GetScope().GetSeq_entryHandle(se), **it, *edited_desc));
                        composite->AddCommand(*cmd);
                    }
                }
            }

            if (!found && m_CreateNew->IsChecked())
                CreateNewMolInfo(se,composite,choice_mol2,choice_tech2,choice_complete2);
        }
    }
    if (se.IsSet()) {
        FOR_EACH_SEQENTRY_ON_SEQSET (it, se.GetSet()) {
            ApplyToMolInfo (**it, composite, choice_mol1, choice_mol2, choice_tech1, choice_tech2, choice_complete1, choice_complete2); 
        }
    }
}

void CMolInfoEditDlg::ApplyToInst (const CSeq_entry& se, CCmdComposite* composite, int choice_class1, int choice_class2, int choice_topology1, int choice_topology2, int choice_strand1, int choice_strand2)  
{

    if (se.IsSeq() && se.GetSeq().IsSetInst())
    {
        const CSeq_inst& inst = se.GetSeq().GetInst();
        if ( (inst.IsNa() && m_RBNuc->GetValue())  ||
             (inst.IsAa() && m_RBProt->GetValue()) ||
             (inst.IsSetMol() && inst.GetMol() == CSeq_inst::eMol_dna && m_RBDna->GetValue()) ||
             (inst.IsSetMol() && inst.GetMol() == CSeq_inst::eMol_rna && m_RBRna->GetValue()) ||
             m_RBAny->GetValue())
        {
            CRef<CSerialObject> edited_object;
            edited_object.Reset((CSerialObject*)CSeq_inst::GetTypeInfo()->Create());
            edited_object->Assign(inst);
            CSeq_inst& edited_inst = dynamic_cast<CSeq_inst&>(*edited_object);
            bool modified = false;
            if ((choice_class1 == -1 || (inst.IsSetMol() && inst.GetMol() == static_cast<CSeq_inst::EMol>(choice_class1))) && choice_class2 != -1 && choice_class1 != choice_class2)
            {
                modified = true;
                edited_inst.SetMol(static_cast<CSeq_inst::EMol>(choice_class2));
            }
            if ((choice_topology1 == -1 || (inst.IsSetTopology() && inst.GetTopology() == static_cast<CSeq_inst::ETopology>(choice_topology1))) && choice_topology2 != -1 && choice_topology1 != choice_topology2)
            {
                modified = true;
                edited_inst.SetTopology(static_cast<CSeq_inst::ETopology>(choice_topology2));
            }
            if ((choice_strand1 == -1 || (inst.IsSetStrand() && inst.GetStrand() == static_cast<CSeq_inst::EStrand>(choice_strand1))) && choice_strand2 != -1 && choice_strand1 != choice_strand2)
            {
                modified = true;
                edited_inst.SetStrand(static_cast<CSeq_inst::EStrand>(choice_strand2));
            }
            if (modified)
            {
                CBioseq_Handle bh = m_TopSeqEntry.GetScope().GetBioseqHandle(se.GetSeq());
                CCmdChangeBioseqInst* cmd = new CCmdChangeBioseqInst(bh, edited_inst);
                composite->AddCommand(*cmd);
            }
        }
    }
    if (se.IsSet()) {
        FOR_EACH_SEQENTRY_ON_SEQSET (it, se.GetSet()) {
            ApplyToInst (**it, composite, choice_class1, choice_class2, choice_topology1, choice_topology2, choice_strand1, choice_strand2);
        }
    }
}


END_NCBI_SCOPE
