/*  $Id: molinfoedit_util.cpp 43676 2019-08-14 14:28:05Z asztalos $
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

#include <objects/seqset/Seq_entry.hpp>
#include <objects/submit/Seq_submit.hpp>
#include <objects/seqset/Bioseq_set.hpp>
#include <objects/seq/Bioseq.hpp>
#include <objects/seq/Seq_inst.hpp>
#include <objects/seqloc/Seq_id.hpp>
#include <objects/general/Object_id.hpp>
#include <objects/general/User_field.hpp>
#include <objects/seqfeat/BioSource.hpp>
#include <objects/seqfeat/Org_ref.hpp>
#include <objects/seqfeat/OrgName.hpp>
#include <objmgr/seqdesc_ci.hpp>
#include <objmgr/bioseq_ci.hpp>
#include <objmgr/feat_ci.hpp>

#include <gui/widgets/wx/wx_utils.hpp>
#include <gui/packages/pkg_sequence_edit/subprep_util.hpp>
#include <gui/packages/pkg_sequence_edit/srcedit_util.hpp>
#include <gui/packages/pkg_sequence_edit/seqtable_util.hpp>
#include <gui/packages/pkg_sequence_edit/subprep_panel.hpp>
#include <gui/packages/pkg_sequence_edit/molinfoedit_util.hpp>
#include <gui/widgets/object_list/object_list_widget.hpp>
#include <gui/widgets/edit/field_type_constants.hpp>
#include <gui/objutils/cmd_composite.hpp>
#include <gui/objutils/cmd_create_desc.hpp>
#include <gui/objutils/cmd_change_bioseq_inst.hpp>
#include <gui/objutils/cmd_add_seqentry.hpp>
#include <gui/objutils/descriptor_change.hpp>

#include <objects/seqtable/SeqTable_multi_data.hpp>
#include <objects/seqtable/SeqTable_column_info.hpp>
#include <objects/seqloc/Seq_id.hpp>
#include <objects/seqfeat/RNA_ref.hpp>

BEGIN_NCBI_SCOPE


objects::CSeq_inst::ETopology s_TopologyFromName (string name)
{
    objects::CSeq_inst::ETopology topology = objects::CSeq_inst::eTopology_not_set;
    if (NStr::EqualNocase(name, "circular")) {
        topology = objects::CSeq_inst::eTopology_circular;
    } else if (NStr::EqualNocase(name, "linear")) {
        topology = objects::CSeq_inst::eTopology_linear;
    } else if (NStr::EqualNocase(name, "other")) {
        topology = objects::CSeq_inst::eTopology_other;
    } else if (NStr::EqualNocase(name, "tandem")) {
        topology = objects::CSeq_inst::eTopology_tandem;
    }
    return topology;
}


string s_GetBiomolName (objects::CSeq_inst::EMol mol, objects::CMolInfo::TBiomol biomol)
{
    string name = "";
    switch (biomol) {
        case objects::CMolInfo::eBiomol_genomic:
        case objects::CMolInfo::eBiomol_unknown:
            if (mol == objects::CSeq_inst::eMol_rna) {
                name = "Genomic RNA";
            } else if (mol == objects::CSeq_inst::eMol_dna) {
                name = kDefaultMoleculeType;
            }
            break;
        case objects::CMolInfo::eBiomol_pre_RNA:
            name = "Precursor RNA";
            break;
        case objects::CMolInfo::eBiomol_mRNA:
            name = "mRNA";
            break;
        case objects::CMolInfo::eBiomol_rRNA:
            name = "Ribosomal RNA";
            break;
        case objects::CMolInfo::eBiomol_tRNA:
            name = "Transfer RNA";
            break;
        case objects::CMolInfo::eBiomol_peptide:
            name = "Peptide";
            break;
        case objects::CMolInfo::eBiomol_other_genetic:
            name = "Other-Genetic";
            break;
        case objects::CMolInfo::eBiomol_genomic_mRNA:
            name = "Genomic-mRNA";
            break;
        case objects::CMolInfo::eBiomol_cRNA:
            name = "cRNA";
            break;
        case objects::CMolInfo::eBiomol_transcribed_RNA:
            name = "Transcribed RNA";
            break;
        case objects::CMolInfo::eBiomol_ncRNA:
        case objects::CMolInfo::eBiomol_scRNA:
        case objects::CMolInfo::eBiomol_snoRNA:
        case objects::CMolInfo::eBiomol_snRNA:
        case objects::CMolInfo::eBiomol_tmRNA:
            name = "Non-coding RNA";
            break;
        case objects::CMolInfo::eBiomol_other:
            name = "Other";
            break;
    }
    return name;
}


static void s_GetBiomolValuesFromName (string name, objects::CSeq_inst::EMol &mol, objects::CMolInfo::EBiomol &biomol)
{
    mol = objects::CSeq_inst::eMol_na;
    biomol = objects::CMolInfo::eBiomol_unknown;

    if (NStr::EqualNocase(name, "Genomic RNA")) {
        biomol = objects::CMolInfo::eBiomol_genomic;
        mol = objects::CSeq_inst::eMol_rna;
    } else if (NStr::EqualNocase(name, kDefaultMoleculeType)) {
        biomol = objects::CMolInfo::eBiomol_genomic;
        mol = objects::CSeq_inst::eMol_dna;
    } else if (NStr::EqualNocase(name, "Precursor RNA")
               || NStr::EqualNocase(name, "preRNA")
               || NStr::EqualNocase(name, "pre-RNA")
               || NStr::EqualNocase(name, "pre RNA")) {
        biomol = objects::CMolInfo::eBiomol_pre_RNA;
        mol = objects::CSeq_inst::eMol_rna;
    } else if (NStr::EqualNocase(name, "mRNA [cDNA]")
               || NStr::EqualNocase(name, "cDNA")               
               || NStr::EqualNocase(name, "mRNA")) {
        biomol = objects::CMolInfo::eBiomol_mRNA;
        mol = objects::CSeq_inst::eMol_rna;
    } else if (NStr::EqualNocase(name, "Ribosomal RNA")
              || NStr::EqualNocase(name, "rRNA")) {
        biomol = objects::CMolInfo::eBiomol_rRNA;
        mol = objects::CSeq_inst::eMol_rna;
    } else if (NStr::EqualNocase(name, "Transfer RNA")
              || NStr::EqualNocase(name, "tRNA")) {
        biomol = objects::CMolInfo::eBiomol_tRNA;
        mol = objects::CSeq_inst::eMol_rna;
    } else if (NStr::EqualNocase(name, "Peptide")
              || NStr::EqualNocase(name, "Amino Acid")) {
        biomol = objects::CMolInfo::eBiomol_peptide;
        mol = objects::CSeq_inst::eMol_aa;
    } else if (NStr::EqualNocase(name, "Other-Genetic")) {
        biomol = objects::CMolInfo::eBiomol_other_genetic;
        mol = objects::CSeq_inst::eMol_na;
    } else if (NStr::EqualNocase(name, "Genomic-mRNA")
              || NStr::EqualNocase(name, "Genomic mRNA")) {
        biomol = objects::CMolInfo::eBiomol_genomic_mRNA;
        mol = objects::CSeq_inst::eMol_rna;
    } else if (NStr::EqualNocase(name, "cRNA")) {
        biomol = objects::CMolInfo::eBiomol_cRNA;
        mol = objects::CSeq_inst::eMol_rna;
    } else if (NStr::EqualNocase(name, "Transcribed RNA")) {
        biomol = objects::CMolInfo::eBiomol_transcribed_RNA;
        mol = objects::CSeq_inst::eMol_rna;
    } else if (NStr::EqualNocase(name, "Non-coding RNA")
              || NStr::EqualNocase(name, "ncRNA")
              || NStr::EqualNocase(name, "scRNA")
              || NStr::EqualNocase(name, "snoRNA")
              || NStr::EqualNocase(name, "snRNA")
              || NStr::EqualNocase(name, "tmRNA")) {
        biomol = objects::CMolInfo::eBiomol_ncRNA;
        mol = objects::CSeq_inst::eMol_rna;
    } else if (NStr::EqualNocase(name, "Other")) {
        biomol = objects::CMolInfo::eBiomol_other;
        mol = objects::CSeq_inst::eMol_na;
    }
}


vector<string> GetMoleculeTypeOptions (CSourceRequirements::EWizardType wizard_type)
{
    vector<string> mol_options;
    mol_options.clear();
    mol_options.push_back(kDefaultMoleculeType);
    switch (wizard_type) {
        case CSourceRequirements::eWizardType_viruses:            
            mol_options.push_back("genomic RNA");
            mol_options.push_back("cRNA");
            mol_options.push_back("mRNA");
            break;
        case CSourceRequirements::eWizardType_microsatellite:
            mol_options.push_back("mRNA");
            break;
        case CSourceRequirements::eWizardType_d_loop:
            break;
        case CSourceRequirements::eWizardType_standard:
        default:
            mol_options.push_back("genomic RNA");
            mol_options.push_back("cRNA");
            mol_options.push_back("mRNA");
            mol_options.push_back("Precursor RNA");
            mol_options.push_back("Ribosomal RNA");
            mol_options.push_back("Transfer RNA");
            mol_options.push_back("Other-Genetic");
            mol_options.push_back("Transcribed RNA"); 
            mol_options.push_back("Transfer-messenger RNA");
            mol_options.push_back("ncRNA");
            break;
    }
    return mol_options;
}


vector<string> GetSrcGenomeOptions (CSourceRequirements::EWizardType wizard_type, CSourceRequirements::EWizardSrcType src_type)
{
    vector<string> genome_options;
    genome_options.clear();
    if (wizard_type == CSourceRequirements::eWizardType_microsatellite) {
        genome_options.push_back("genomic");
        genome_options.push_back("mitochondrion");
        genome_options.push_back("chloroplast");
        genome_options.push_back("plastid");
    } else {
        genome_options.push_back("genomic");
        if (wizard_type == CSourceRequirements::eWizardType_rrna_its_igs 
            || wizard_type == CSourceRequirements::eWizardType_igs
            || wizard_type == CSourceRequirements::eWizardType_standard) {
            genome_options.push_back("mitochondrion");
            if (src_type == CSourceRequirements::eWizardSrcType_any
                || src_type == CSourceRequirements::eWizardSrcType_plant
                || src_type == CSourceRequirements::eWizardSrcType_bacteria_or_archaea) {
                genome_options.push_back("chloroplast");
                genome_options.push_back("chromoplast");
                genome_options.push_back("kinetoplast");
                genome_options.push_back("plastid");
                genome_options.push_back("macronuclear");
                genome_options.push_back("cyanelle");
                genome_options.push_back("nucleomorph");
                genome_options.push_back("apicoplast");
                genome_options.push_back("leucoplast");
                genome_options.push_back("proplastid");
                genome_options.push_back("hydrogenosome");
                genome_options.push_back("chromatophore");
                genome_options.push_back("extrachromosomal");
                genome_options.push_back("plasmid");
                genome_options.push_back("proviral");
                genome_options.push_back("virion");
                genome_options.push_back("endogenous-virus");
            }
        }
    }
    return genome_options;
}


CRef<objects::CSeq_table> BuildMolInfoValuesTableFromSeqEntry (objects::CSeq_entry_Handle seh, CSourceRequirements::EWizardType wizard_type)
{
    CRef<objects::CSeq_table> table(new objects::CSeq_table());
    CRef<objects::CSeqTable_column> id_col(new objects::CSeqTable_column());
    id_col->SetHeader().SetField_id(objects::CSeqTable_column_info::eField_id_location_id);
    id_col->SetHeader().SetTitle(kSequenceIdColLabel);
    table->SetColumns().push_back(id_col);

    CRef<objects::CSeqTable_column> moltype_col;
    CRef<objects::CSeqTable_column> topology_col;
    /* TODO: different coloms for different wizards */

    moltype_col = new objects::CSeqTable_column();
    moltype_col->SetHeader().SetTitle(kMoleculeType);
    moltype_col->SetHeader().SetField_name("biomol");
    moltype_col->SetData().SetString();
    table->SetColumns().push_back(moltype_col);
    topology_col = new objects::CSeqTable_column();
    topology_col->SetHeader().SetTitle("Topology");
    topology_col->SetHeader().SetField_name("topology");
    topology_col->SetData().SetString();
    table->SetColumns().push_back(topology_col);

    size_t row = 0;
    objects::CBioseq_CI b_iter(seh, objects::CSeq_inst::eMol_na);
    for ( ; b_iter ; ++b_iter ) {
        objects::CSeqdesc_CI it (*b_iter, objects::CSeqdesc::e_Molinfo);
        CRef<objects::CSeq_id> id(new objects::CSeq_id());
        id->Assign (*(b_iter->GetSeqId()));
        id_col->SetData().SetId().push_back(id);
        if (topology_col) {
            objects::CSeq_inst::ETopology topology = objects::CSeq_inst::eTopology_not_set;
            if (b_iter->IsSetInst_Topology()) {
                topology = b_iter->GetInst_Topology();
            }
            topology_col->SetData().SetString().push_back(CMolInfoField::GetTopologyLabel(topology));
        }
        if (moltype_col) {
            objects::CSeq_inst::EMol mol = objects::CSeq_inst::eMol_na;
            if (b_iter->IsSetInst_Mol()) {
                mol = b_iter->GetInst_Mol();
            }
            objects::CMolInfo::TBiomol biomol = objects::CMolInfo::eBiomol_unknown;
            if (it && it->GetMolinfo().IsSetBiomol()) {
                biomol = it->GetMolinfo().GetBiomol();
            }
            moltype_col->SetData().SetString().push_back(s_GetBiomolName(mol, biomol));
        }
        row++;
    }            

    table->SetNum_rows(row);
    return table;
}


CRef<CCmdComposite> ApplyMolInfoValuesTableToSeqEntry (CRef<objects::CSeq_table>values_table, objects::CSeq_entry_Handle seh, bool add_confirmed)
{
    CRef<CCmdComposite> cmd( new CCmdComposite("Bulk Molecule Type Edit") );

    CRef<objects::CSeqTable_column> id_col = FindSeqTableColumnByName (values_table, kSequenceIdColLabel);
    if (!id_col) {
        return cmd;
    }
    CRef<objects::CSeqTable_column> topology_col = FindSeqTableColumnByName (values_table, "Topology");
    CRef<objects::CSeqTable_column> moltype_col = FindSeqTableColumnByName (values_table, kMoleculeType);

    if( ! values_table->IsSetColumns() || values_table->GetColumns().empty() ) {
        return cmd;
    }

    for (int row = 0; row < values_table->GetNum_rows() && (unsigned int) row < id_col->GetData().GetSize(); row++) {
        objects::CBioseq_Handle bsh = seh.GetBioseqHandle(*(id_col->GetData().GetId()[row]));

        CRef<objects::CSeq_inst> inst (new objects::CSeq_inst());
        inst->Assign(bsh.GetInst());
        bool inst_changed = false;

        if (topology_col) {
            objects::CSeq_inst::ETopology new_topology = s_TopologyFromName(*topology_col->GetStringPtr(row) );
            if (inst->IsSetTopology()) {
                if (new_topology != inst->GetTopology()) {
                    inst->SetTopology(new_topology);
                    inst_changed = true;
                }
            } else {
                if (new_topology != objects::CSeq_inst::eTopology_not_set) {
                    inst->SetTopology(new_topology);
                    inst_changed = true;
                }
            }
        }

        CRef<objects::CSeqdesc> new_molinfo_desc( new objects::CSeqdesc );
        objects::CMolInfo & molinfo = new_molinfo_desc->SetMolinfo();
        bool molinfo_changed = false;
        objects::CSeqdesc_CI desc_ci( bsh, objects::CSeqdesc::e_Molinfo);
        if (desc_ci) {
            molinfo.Assign(desc_ci->GetMolinfo());
        } else {
            molinfo_changed = true;
        }

        if (moltype_col) {
            objects::CSeq_inst::EMol new_mol;
            objects::CMolInfo::EBiomol new_biomol;
            s_GetBiomolValuesFromName ( *moltype_col->GetStringPtr(row), new_mol, new_biomol);
            if (inst->IsSetMol()) {
                if (new_mol != inst->GetMol()) {
                    inst->SetMol(new_mol);
                    inst_changed = true;
                }
            } else {
                if (new_mol != objects::CSeq_inst::eMol_not_set) {
                    inst->SetMol(new_mol);
                    inst_changed = true;
                }
            }

            if (molinfo.IsSetBiomol()) {
                if (new_biomol != molinfo.GetBiomol()) {
                    molinfo.SetBiomol(new_biomol);
                    molinfo_changed = true;
                }
            } else {
                if (new_biomol != objects::CMolInfo::eBiomol_unknown) {
                    molinfo.SetBiomol(new_biomol);
                    molinfo_changed = true;
                }
            }
        }

        if (inst_changed) {
            CRef<CCmdChangeBioseqInst> scmd (new CCmdChangeBioseqInst(bsh, *inst));
            cmd->AddCommand(*scmd);
        }
        if (molinfo_changed) {
            if (desc_ci) {
                CRef<CCmdChangeSeqdesc> ecmd(new CCmdChangeSeqdesc(desc_ci.GetSeq_entry_Handle(), *desc_ci, *new_molinfo_desc));
                cmd->AddCommand (*ecmd);
            } else {
                cmd->AddCommand( *CRef<CCmdCreateDesc>(new CCmdCreateDesc(bsh.GetParentEntry(), *new_molinfo_desc)) );
            }
        }
        if (add_confirmed && (inst_changed || molinfo_changed)) {
            cmd->AddCommand(*CSubPrep_panel::SetWizardFieldInSeqEntry(seh, string(kMoleculeType) + " Confirmed", "Yes"));
        }
    }

    // send composite command
    return cmd;
}


CRef<objects::CSeq_table> BuildCommentDescriptorValuesTableFromSeqEntry (objects::CSeq_entry_Handle seh, const string& label)
{
    CRef<objects::CSeq_table> table(new objects::CSeq_table());
    CRef<objects::CSeqTable_column> id_col(new objects::CSeqTable_column());
    id_col->SetHeader().SetField_id(objects::CSeqTable_column_info::eField_id_location_id);
    id_col->SetHeader().SetTitle(kSequenceIdColLabel);
    table->SetColumns().push_back(id_col);

    CRef<objects::CSeqTable_column> comment_col(new objects::CSeqTable_column());
    comment_col->SetHeader().SetTitle(label);
    comment_col->SetHeader().SetField_name("comment");
    comment_col->SetData().SetString();
    table->SetColumns().push_back(comment_col);

    size_t row = 0;
    objects::CBioseq_CI b_iter(seh, objects::CSeq_inst::eMol_na);
    for ( ; b_iter ; ++b_iter ) {
        objects::CSeqdesc_CI it (*b_iter, objects::CSeqdesc::e_Comment);
        CRef<objects::CSeq_id> id(new objects::CSeq_id());
        id->Assign (*(b_iter->GetSeqId()));
        id_col->SetData().SetId().push_back(id);
        string comment = "";
        if (it) {
            comment = it->GetComment();
        }
        comment_col->SetData().SetString().push_back(comment);
        row++;
    }            

    table->SetNum_rows(row);
    return table;
}


CRef<CCmdComposite> ApplyCommentValuesTableToSeqEntry (CRef<objects::CSeq_table>values_table, objects::CSeq_entry_Handle seh, const string& label)
{
    CRef<CCmdComposite> cmd( new CCmdComposite("Bulk Comment Edit") );

    CRef<objects::CSeqTable_column> id_col = FindSeqTableColumnByName (values_table, kSequenceIdColLabel);
    if (!id_col) {
        return cmd;
    }
    CRef<objects::CSeqTable_column> comment_col = FindSeqTableColumnByName (values_table, label);

    if( ! values_table->IsSetColumns() || values_table->GetColumns().empty() ) {
        return cmd;
    }

    for (int row = 0; row < values_table->GetNum_rows() && (unsigned int) row < id_col->GetData().GetSize(); row++) {
        objects::CBioseq_Handle bsh = seh.GetBioseqHandle(*(id_col->GetData().GetId()[row]));
        string new_comment = "";
        if ((unsigned int) row < comment_col->GetData().GetSize()) {
            new_comment = comment_col->GetData().GetString()[row];
        }
        string  old_comment = "";
        objects::CSeqdesc_CI desc_ci( bsh, objects::CSeqdesc::e_Comment);
        if (desc_ci) {
            old_comment = desc_ci->GetComment();
        }
        if (!NStr::Equal(new_comment, old_comment)) {
            CRef<objects::CSeqdesc> new_desc( new objects::CSeqdesc );
            new_desc->SetComment(new_comment);
            if (desc_ci) {
                CRef<CCmdChangeSeqdesc> ecmd(new CCmdChangeSeqdesc(desc_ci.GetSeq_entry_Handle(), *desc_ci, *new_desc));
                cmd->AddCommand (*ecmd);
            } else {
                cmd->AddCommand( *CRef<CCmdCreateDesc>(new CCmdCreateDesc(bsh.GetParentEntry(), *new_desc)) );
            }
        }
    }

    // send composite command
    return cmd;
}


const string kChimeraCommentStart = "Sequences were screened for chimeras by the submitter using ";


static bool s_WantChimeraForSource (const objects::CBioSource& src, CSourceRequirements::EWizardSrcType src_type)
{
    if (src.IsSetGenome() && src.GetGenome() == objects::CBioSource::eGenome_mitochondrion) {
        return true;
    } else if (src.IsSetOrg() && src.GetOrg().IsSetOrgname() && src.GetOrg().GetOrgname().IsSetLineage()) {
        const string& lineage = src.GetOrg().GetOrgname().GetLineage();
        if (NStr::FindNoCase(lineage, "bacteria") != string::npos
               || NStr::FindNoCase(lineage, "archaea") != string::npos) {
            return true;
        } else {
            return false;
        }
    } else if (src_type == CSourceRequirements::eWizardSrcType_any 
              || src_type == CSourceRequirements::eWizardSrcType_bacteria_or_archaea) {
        return true;
    } else {
        return false;
    }
}


CRef<objects::CSeq_table> BuildChimeraValuesTableFromSeqEntry (objects::CSeq_entry_Handle seh, const string& label, CSourceRequirements::EWizardSrcType src_type)
{
    CRef<objects::CSeq_table> table(new objects::CSeq_table());
    CRef<objects::CSeqTable_column> id_col(new objects::CSeqTable_column());
    id_col->SetHeader().SetField_id(objects::CSeqTable_column_info::eField_id_location_id);
    id_col->SetHeader().SetTitle(kSequenceIdColLabel);
    table->SetColumns().push_back(id_col);

    CRef<objects::CSeqTable_column> comment_col(new objects::CSeqTable_column());
    comment_col->SetHeader().SetTitle(label);
    comment_col->SetHeader().SetField_name("comment");
    comment_col->SetData().SetString();
    table->SetColumns().push_back(comment_col);

    size_t row = 0;
    objects::CBioseq_CI b_iter(seh, objects::CSeq_inst::eMol_na);
    for ( ; b_iter ; ++b_iter ) {
        objects::CSeqdesc_CI desc (*b_iter, objects::CSeqdesc::e_Source);
        if (desc && !s_WantChimeraForSource(desc->GetSource(), src_type)) {
            continue;
        }
        
        int this_count = 0;
        bool any = false;
        objects::CFeat_CI fit (*b_iter);
        while (fit && this_count < 2) {
            this_count++;
            if (fit->IsSetData() && fit->GetData().IsRna()
                && fit->GetData().GetRna().IsSetType()
                && fit->GetData().GetRna().GetType() == objects::CRNA_ref::eType_rRNA
                && fit->GetData().GetRna().IsSetExt()
                && fit->GetData().GetRna().GetExt().IsName()
                && NStr::Equal(fit->GetData().GetRna().GetExt().GetName(), "16S ribosomal RNA")) {
              any = true;
            } else {
              break;
            }
            ++fit;
        }
        if (this_count == 1 && any) {
            CRef<objects::CSeq_id> id(new objects::CSeq_id());
            id->Assign (*(b_iter->GetSeqId()));
            id_col->SetData().SetId().push_back(id);
            objects::CSeqdesc_CI it (*b_iter, objects::CSeqdesc::e_Comment);
            while (it && !NStr::StartsWith(it->GetComment(), kChimeraCommentStart)) {
                ++it;
            }
            string prog_ver = "";
            if (it) {
                prog_ver = it->GetComment().substr(kChimeraCommentStart.length());
            }
            comment_col->SetData().SetString().push_back(prog_ver);
            row++;
        }
    }            

    if (row == 0) {
        table->Reset();
    } else {
        table->SetNum_rows(row);
    }
    return table;
}


CRef<CCmdComposite> ApplyChimeraValuesTableToSeqEntry (CRef<objects::CSeq_table>values_table, objects::CSeq_entry_Handle seh, const string& label)
{
    CRef<CCmdComposite> cmd( new CCmdComposite("Bulk Chimera Comment Edit") );

    CRef<objects::CSeqTable_column> id_col = FindSeqTableColumnByName (values_table, kSequenceIdColLabel);
    if (!id_col) {
        return cmd;
    }
    CRef<objects::CSeqTable_column> comment_col = FindSeqTableColumnByName (values_table, label);

    if( ! values_table->IsSetColumns() || values_table->GetColumns().empty() ) {
        return cmd;
    }

    for (int row = 0; row < values_table->GetNum_rows() && (unsigned int) row < id_col->GetData().GetSize(); row++) {
        objects::CBioseq_Handle bsh = seh.GetBioseqHandle(*(id_col->GetData().GetId()[row]));
        string new_comment = comment_col->GetData().GetString()[row];
        if (!NStr::IsBlank(new_comment)) {
            new_comment = kChimeraCommentStart + new_comment;
        }
        string  old_comment = "";
        objects::CSeqdesc_CI desc_ci( bsh, objects::CSeqdesc::e_Comment);
        while (desc_ci && !NStr::StartsWith(desc_ci->GetComment(), kChimeraCommentStart)) {
            ++ desc_ci;
        }
        if (desc_ci) {
            old_comment = desc_ci->GetComment();
        }
        if (!NStr::Equal(new_comment, old_comment)) {
            CRef<objects::CSeqdesc> new_desc( new objects::CSeqdesc );
            new_desc->SetComment(new_comment);
            if (desc_ci) {
                CRef<CCmdChangeSeqdesc> ecmd(new CCmdChangeSeqdesc(desc_ci.GetSeq_entry_Handle(), *desc_ci, *new_desc));
                cmd->AddCommand (*ecmd);
            } else {
                if (!NStr::IsBlank (new_comment)) {
                    cmd->AddCommand( *CRef<CCmdCreateDesc>(new CCmdCreateDesc(bsh.GetParentEntry(), *new_desc)) );
                }
            }
        }
    }

    // send composite command
    return cmd;
}


CRef<objects::CSeq_table> BuildDBLinkValuesTableFromSeqEntry (objects::CSeq_entry_Handle seh)
{
    CRef<objects::CSeq_table> table(new objects::CSeq_table());
    CRef<objects::CSeqTable_column> id_col(new objects::CSeqTable_column());
    id_col->SetHeader().SetField_id(objects::CSeqTable_column_info::eField_id_location_id);
    id_col->SetHeader().SetTitle(kSequenceIdColLabel);
    table->SetColumns().push_back(id_col);



    size_t row = 0;
    objects::CBioseq_CI b_iter(seh, objects::CSeq_inst::eMol_na);
    for ( ; b_iter ; ++b_iter ) {
        CRef<objects::CSeq_id> id(new objects::CSeq_id());
        id->Assign (*(b_iter->GetSeqId()));
        id_col->SetData().SetId().push_back(id);

        objects::CSeqdesc_CI it (*b_iter, objects::CSeqdesc::e_User);
        while (it) {
            if (it->GetUser().IsSetType() 
                && it->GetUser().GetType().IsStr()
                && NStr::EqualNocase(it->GetUser().GetType().GetStr(), "DBLink")) {
                ITERATE (objects::CUser_object::TData, field_it, it->GetUser().GetData()) {
                    if ((*field_it)->IsSetLabel() 
                        && (*field_it)->GetLabel().IsStr()
                        && (*field_it)->IsSetData()) {
                        string existing_val = "";
                        if ((*field_it)->GetData().IsStr()) {
                            existing_val = (*field_it)->GetData().GetStr();
                        } else if ((*field_it)->GetData().IsStrs()) { 
                            for (vector<CStringUTF8>::const_iterator str_it = (*field_it)->GetData().GetStrs().begin();
                                 str_it != (*field_it)->GetData().GetStrs().end();
                                 ++str_it) {
                                existing_val += ", " + *str_it;
                            }
                            while (NStr::StartsWith(existing_val, ", ")) {
                                existing_val = existing_val.substr(2);
                            }
                        }
                        AddValueToTable (table, (*field_it)->GetLabel().GetStr(), existing_val, row);
                    }
                }
            }
            ++it;
        }

        row++;
    }            

    table->SetNum_rows(row);
    return table;
}


CRef<CCmdComposite> ApplyDBLinkValuesTableToSeqEntry (CRef<objects::CSeq_table>values_table, objects::CSeq_entry_Handle seh)
{
    CRef<CCmdComposite> cmd( new CCmdComposite("Bulk DBLink Edit") );

    CRef<objects::CSeqTable_column> id_col = FindSeqTableColumnByName (values_table, kSequenceIdColLabel);
    if (!id_col) {
        return cmd;
    }

    const objects::CSeq_table::TColumns & columns = values_table->GetColumns();
    size_t num_cols = columns.size();

    for (int row = 0; row < values_table->GetNum_rows() && (size_t) row < id_col->GetData().GetSize(); row++) {
        objects::CBioseq_Handle bsh = seh.GetBioseqHandle(*(id_col->GetData().GetId()[row]));
        CRef<objects::CSeqdesc> new_desc( new objects::CSeqdesc );

        objects::CSeqdesc_CI desc_ci( bsh, objects::CSeqdesc::e_User);
        bool found = false;
        while (desc_ci && !found) {
            if (desc_ci->GetUser().IsSetType() 
                && desc_ci->GetUser().GetType().IsStr()
                && NStr::EqualNocase(desc_ci->GetUser().GetType().GetStr(), "DBLink")) {
                found = true;
                new_desc->SetUser().Assign(desc_ci->GetUser());
            } else {
                ++desc_ci;
            }
        }

        bool any_change = false;
        for (size_t i = 1; i < num_cols; i++) {
            string col_name = values_table->GetColumns()[i]->GetHeader().GetTitle();

            objects::CUser_field& field = new_desc->SetUser().SetField(col_name);
            if (!field.IsSetLabel() || !field.GetLabel().IsStr() || NStr::IsBlank(field.GetLabel().GetStr())) {
                field.SetLabel().SetStr(col_name);
            }
            string new_val = "";
            if ((size_t) row < values_table->GetColumns()[i]->GetData().GetSize()) {
                new_val = values_table->GetColumns()[i]->GetData().GetString()[row];
            }
            vector<string> values;
            NStr::Split(new_val, ", ", values);
            field.ResetData();
            for (vector<string>::iterator str_it = values.begin(); str_it != values.end(); str_it++) {                
                field.SetData().SetStrs().push_back(*str_it);
            }
            if (values.size() == 0) {
                field.SetData().SetStrs().push_back(" ");
            }
            any_change = true;
        }
            
        if (any_change) {
            if (desc_ci) {
                CRef<CCmdChangeSeqdesc> ecmd(new CCmdChangeSeqdesc(desc_ci.GetSeq_entry_Handle(), *desc_ci, *new_desc));
                cmd->AddCommand (*ecmd);
            } else {
                new_desc->SetUser().SetType().SetStr("DBLink");
                cmd->AddCommand( *CRef<CCmdCreateDesc>(new CCmdCreateDesc(bsh.GetParentEntry(), *new_desc)) );
            }
        }
    }

    // send composite command
    return cmd;
}


////// CMolInfoField

string CMolInfoField::GetVal(const CObject& object)
{
    string rval = "";

    const CSeqdesc* desc = dynamic_cast<const CSeqdesc*>(&object);
    const CBioseq* b = dynamic_cast<const CBioseq*>(&object);
    const CSeq_inst* inst = dynamic_cast<const CSeq_inst*>(&object);
    if (b) {
        inst = &(b->GetInst());
    }

    if (desc && desc->IsMolinfo()) {
        switch (m_FieldType) {
        case CMolInfoFieldType::e_MoleculeType:
                if (desc->GetMolinfo().IsSetBiomol()) {
                    rval = GetBiomolLabel(desc->GetMolinfo().GetBiomol());
                }
                break;
        case CMolInfoFieldType::e_Technique:
                if (desc->GetMolinfo().IsSetTech()) {
                    CMolInfo::TTech tech = desc->GetMolinfo().GetTech();
                    if (tech == CMolInfo::eTech_other) {
                        if (desc->GetMolinfo().IsSetTechexp()) {
                            rval = desc->GetMolinfo().GetTechexp();
                        } else {
                            rval = "other";
                        }
                    } else {
                        rval = GetTechLabel(desc->GetMolinfo().GetTech());
                    }
                }
                break;
        case CMolInfoFieldType::e_Completedness:
                if (desc->GetMolinfo().IsSetCompleteness()) {
                    rval = GetCompletenessLabel(desc->GetMolinfo().GetCompleteness());
                }
                break;
            default:
                break;
        }
    }
    if (inst) {
        switch (m_FieldType) {
        case CMolInfoFieldType::e_Class:
                if (inst->IsSetMol()) {
                    rval = GetMolLabel(inst->GetMol());
                }
                break;
        case CMolInfoFieldType::e_Topology:
                if (inst->IsSetTopology()) {
                    rval = GetTopologyLabel(inst->GetTopology());
                }
                break;
        case CMolInfoFieldType::e_Strand:
                if (inst->IsSetStrand()) {
                    rval = GetStrandLabel(inst->GetStrand());
                }
                break;
            default:
                break;
        }
    }
    return rval;
}


vector<string> CMolInfoField::GetVals(const CObject& object)
{
    vector<string> rvals;
    rvals.push_back(GetVal(object));
    return rvals;
}


bool CMolInfoField::IsEmpty(const CObject& object) const
{
    return false;
}


void CMolInfoField::ClearVal(CObject& object)
{
    CSeqdesc* desc = dynamic_cast<CSeqdesc*>(&object);
    CBioseq* bioseq = dynamic_cast<CBioseq*>(&object);
    CSeq_inst* inst = dynamic_cast<CSeq_inst*>(&object);
    if (bioseq) {
        inst = &(bioseq->SetInst());
    }
    switch (m_FieldType) {
    case CMolInfoFieldType::e_MoleculeType:
            if (desc && desc->IsMolinfo()) {
                desc->SetMolinfo().ResetBiomol();
            }
            break;
    case CMolInfoFieldType::e_Technique:
            if (desc && desc->IsMolinfo()) {
                desc->SetMolinfo().ResetTech();
                desc->SetMolinfo().ResetTechexp();
            }
            break;
    case CMolInfoFieldType::e_Completedness:
            if (desc && desc->IsMolinfo()) {
                desc->SetMolinfo().ResetCompleteness();
            }
            break;
    
    case CMolInfoFieldType::e_Class:
            if (inst) {
                inst->ResetMol();
            }
            break;
    case CMolInfoFieldType::e_Topology:
            if (inst) {
                inst->ResetTopology();
            }
            break;
    case CMolInfoFieldType::e_Strand:
            if (inst) {
                inst->ResetStrand();
            }
        default:
            break;
    }
}


bool CMolInfoField::SetVal(CObject& object, const string& val, edit::EExistingText existing_text)
{
    bool rval = false;
    CSeqdesc* desc = dynamic_cast<CSeqdesc*>(&object);
    CBioseq* bioseq = dynamic_cast<CBioseq*>(&object);
    CSeq_inst* inst = dynamic_cast<CSeq_inst*>(&object);
    if (bioseq) {
        inst = &(bioseq->SetInst());
    }
    switch (m_FieldType) {
    case CMolInfoFieldType::e_MoleculeType:
            if (desc && desc->IsMolinfo()) {
                desc->SetMolinfo().SetBiomol(GetBiomolFromLabel(val));
                rval = true;
            }
            break;
    case CMolInfoFieldType::e_Technique:
            if (desc && desc->IsMolinfo()) {
                CMolInfo::TTech tech = GetTechFromLabel(val);
                desc->SetMolinfo().SetTech(tech);
                if (tech == CMolInfo::eTech_other) {
                    desc->SetMolinfo().SetTechexp(val);
                } else {
                    desc->SetMolinfo().ResetTechexp();
                }
                rval = true;
            }
            break;
    case CMolInfoFieldType::e_Completedness:
            if (desc && desc->IsMolinfo()) {
                desc->SetMolinfo().SetCompleteness(GetCompletenessFromLabel(val));
                rval = true;
            }

            break;    
    case CMolInfoFieldType::e_Class:
            if (inst) {
                inst->SetMol(GetMolFromLabel(val));
                rval = true;
            }
            break;
    case CMolInfoFieldType::e_Topology:
            if (inst) {
                inst->SetTopology(GetTopologyFromLabel(val));
                rval = true;
            }
            break;
    case CMolInfoFieldType::e_Strand:
            if (inst) {
                inst->SetStrand(GetStrandFromLabel(val));   
                rval = true;
            }
            break;
        default:
            break;
    }
    return rval;
}


void CMolInfoField::SetConstraint(const string& field, CConstRef<edit::CStringConstraint> string_constraint)
{
    m_ConstraintFieldType = GetFieldType(field);
    if (m_ConstraintFieldType == CMolInfoFieldType::e_Unknown || !string_constraint) {
        m_StringConstraint.Reset(NULL);
    } else {
        m_StringConstraint = new edit::CStringConstraint(" ");
        m_StringConstraint->Assign(*string_constraint);
    }        
}

vector<string> CMolInfoField::GetFieldNames()
{
    return CMolInfoFieldType::GetFieldNames();
}


string CMolInfoField::GetFieldName(CMolInfoFieldType::EMolInfoFieldType field_type)
{
    return CMolInfoFieldType::GetFieldName(field_type);
}


CMolInfoFieldType::EMolInfoFieldType CMolInfoField::GetFieldType(const string& field_name)
{
    return CMolInfoFieldType::GetFieldType(field_name);
}

vector<string> CMolInfoField::GetChoicesForField(CMolInfoFieldType::EMolInfoFieldType field_type, bool& allow_other)
{
    return CMolInfoFieldType::GetChoicesForField(field_type, allow_other);
}

string CMolInfoField::GetBiomolLabel(CMolInfo::TBiomol biomol)
{
    return CMolInfoFieldType::GetBiomolLabel(biomol);
}

CMolInfo::TBiomol CMolInfoField::GetBiomolFromLabel(const string& val)
{
    return CMolInfoFieldType::GetBiomolFromLabel(val);
}

string CMolInfoField::GetTechLabel(CMolInfo::TTech tech)
{
    return CMolInfoFieldType::GetTechLabel(tech);
}

CMolInfo::TTech CMolInfoField::GetTechFromLabel(const string& val)
{
    return CMolInfoFieldType::GetTechFromLabel(val);
}

string CMolInfoField::GetCompletenessLabel(CMolInfo::TCompleteness tech)
{
    return CMolInfoFieldType::GetCompletenessLabel(tech);
}

CMolInfo::TCompleteness CMolInfoField::GetCompletenessFromLabel(const string& val)
{
    return CMolInfoFieldType::GetCompletenessFromLabel(val);
}
string CMolInfoField::GetMolLabel(CSeq_inst::TMol val)
{
    return CMolInfoFieldType::GetMolLabel(val);
}

CSeq_inst::TMol CMolInfoField::GetMolFromLabel(const string& val)
{
    return CMolInfoFieldType::GetMolFromLabel(val);
}

string CMolInfoField::GetTopologyLabel(CSeq_inst::TTopology tech)
{
    return CMolInfoFieldType::GetTopologyLabel(tech);
}

CSeq_inst::TTopology CMolInfoField::GetTopologyFromLabel(const string& val)
{
    return CMolInfoFieldType::GetTopologyFromLabel(val);
}

string CMolInfoField::GetStrandLabel(CSeq_inst::TStrand val)
{
    return CMolInfoFieldType::GetStrandLabel(val);
}

CSeq_inst::TStrand CMolInfoField::GetStrandFromLabel(const string& val)
{
    return CMolInfoFieldType::GetStrandFromLabel(val);
}


vector<CConstRef<CObject> > CMolInfoField::GetObjects(CBioseq_Handle bsh)
{
    vector<CConstRef<CObject> > objs;

    switch (m_FieldType) {
        case CMolInfoFieldType::e_MoleculeType:
        case CMolInfoFieldType::e_Technique:
        case CMolInfoFieldType::e_Completedness:
            objs = CTextDescriptorField::GetObjects(bsh);
            break;
        case CMolInfoFieldType::e_Class:
        case CMolInfoFieldType::e_Topology:
        case CMolInfoFieldType::e_Strand:
            objs.push_back(CConstRef<CObject>(bsh.GetCompleteBioseq().GetPointer()));
            break;
        default:
            break;
    }
    return objs;
}


vector<CRef<edit::CApplyObject> > CMolInfoField::GetApplyObjects(CBioseq_Handle bsh)
{
    vector<CRef<edit::CApplyObject> > objs;

    switch (m_FieldType) {
        case CMolInfoFieldType::e_MoleculeType:
        case CMolInfoFieldType::e_Technique:
        case CMolInfoFieldType::e_Completedness:
            {{
                // add existing descriptors
                CSeqdesc_CI desc_ci(bsh, m_Subtype);
                while (desc_ci) {
                    CRef<CSeqdesc> new_desc(new CSeqdesc());
                    new_desc->Assign(*desc_ci);
                    CRef<edit::CApplyObject> new_obj(new edit::CApplyObject(bsh.GetParentEntry(),
                                                                CConstRef<CObject>(&(*desc_ci)),
                                                                CRef<CObject>(new_desc.GetPointer())));
                    objs.push_back(new_obj);
                    ++desc_ci;
                }

                if (objs.empty()) {
                    CRef<CSeqdesc> new_desc(new CSeqdesc());
                    new_desc->SetMolinfo();
                    CRef<edit::CApplyObject> new_obj(new edit::CApplyObject(bsh.GetParentEntry(),
                                                                CConstRef<CObject>(NULL),
                                                                CRef<CObject>(new_desc.GetPointer())));
                    objs.push_back(new_obj);
                }        
            }}
            break;
        case CMolInfoFieldType::e_Class:
        case CMolInfoFieldType::e_Topology:
        case CMolInfoFieldType::e_Strand:
            {{
                CRef<CSeq_inst> inst(new CSeq_inst());
                if (bsh.IsSetInst()) {
                    inst->Assign(bsh.GetInst());
                }
                CRef<edit::CApplyObject> new_obj(new edit::CApplyObject(bsh.GetParentEntry(),
                                                            CConstRef<CObject>(bsh.GetCompleteBioseq().GetPointer()),
                                                            CRef<CObject>(inst.GetPointer())));
                objs.push_back(new_obj);
            }}
            break;
        default:
            break;
    }
    return objs;
}





END_NCBI_SCOPE
