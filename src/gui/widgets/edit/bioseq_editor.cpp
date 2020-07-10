/*  $Id: bioseq_editor.cpp 44907 2020-04-14 13:03:07Z asztalos $
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
 * Authors:  Roman Katargin
 */

#include <ncbi_pch.hpp>

#include <set>
#include <gui/widgets/text_widget/text_item.hpp>
#include <gui/widgets/text_widget/text_item_panel.hpp>
#include <objtools/format/context.hpp>
#include <objtools/format/items/sequence_item.hpp>
#include <objtools/format/items/locus_item.hpp>
#include <objtools/format/items/defline_item.hpp>
#include <objtools/format/items/source_item.hpp>
#include <objtools/format/items/comment_item.hpp>
#include <objtools/format/items/feature_item.hpp>
#include <objtools/format/items/gap_item.hpp>
#include <objtools/format/items/genome_project_item.hpp>
#include <objtools/format/items/primary_item.hpp>
#include <objtools/format/items/reference_item.hpp>
#include <objtools/format/items/keywords_item.hpp>
#include <objtools/format/items/accession_item.hpp>
#include <objtools/format/items/version_item.hpp>
#include <objtools/format/items/origin_item.hpp>
#include <objtools/format/items/ctrl_items.hpp>
#include <objtools/format/items/contig_item.hpp>
#include <objtools/format/items/basecount_item.hpp>
#include <gui/widgets/seq/flat_file_text_item.hpp>
#include <gui/widgets/text_widget/expand_item.hpp>
#include <gui/widgets/edit/edit_object.hpp>
#include <gui/widgets/edit/bioseq_editor.hpp>

#include <gui/objutils/cmd_factory.hpp>

#include <objmgr/bioseq_ci.hpp>
#include <objmgr/seqdesc_ci.hpp>
#include <objmgr/feat_ci.hpp>
#include <objmgr/seq_entry_ci.hpp>
#include <objmgr/align_ci.hpp>
#include <objmgr/seq_annot_ci.hpp>

#include <objects/submit/Submit_block.hpp>

#include <objects/seqfeat/BioSource.hpp>
#include <objects/seqfeat/Seq_feat.hpp>
#include <objects/seqfeat/Cdregion.hpp>
#include <objects/seqfeat/Imp_feat.hpp>
#include <objects/seqfeat/RNA_ref.hpp>
#include <objects/seq/Pubdesc.hpp>
#include <objects/seqfeat/Feat_id.hpp>
#include <objects/general/User_object.hpp>
#include <objects/general/Object_id.hpp>
#include <objects/seq/MolInfo.hpp>
#include <objects/misc/sequence_macros.hpp>

#include <gui/objutils/seqdesc_title_edit.hpp>
#include <gui/objutils/cmd_del_desc.hpp>
#include <gui/objutils/cmd_create_desc.hpp>
#include <gui/objutils/cmd_composite.hpp>
#include <gui/objutils/cmd_del_seq_feat.hpp>
#include <gui/objutils/cmd_del_seq_annot.hpp>
#include <gui/objutils/cmd_del_seq_align.hpp>
#include <gui/objutils/util_cmds.hpp>
#include <gui/objutils/utils.hpp>

#include <objmgr/seq_feat_handle.hpp>

#include <objmgr/util/sequence.hpp>

#include "deletefrombioseq_setdlg.hpp"

#include <gui/widgets/wx/message_box.hpp>
#include <gui/widgets/edit/edit_object_seq_feat.hpp>
#include <gui/widgets/edit/edit_object_seq_desc.hpp>
#include <gui/widgets/edit/edit_object_set.hpp>
#include <gui/widgets/edit/edit_obj_view_dlg.hpp>
#include <gui/widgets/edit/crossref_feats_dlg.hpp>
#include <gui/widgets/edit/edit_object_submit_block.hpp>
#include <gui/widgets/edit/edit_object_feature_propagate.hpp>
#include <gui/widgets/edit/alignment_assistant.hpp>

#include <gui/widgets/edit/edit_sequence.hpp>
#include <gui/widgets/wx/wx_utils.hpp>
#include <gui/widgets/wx/ui_command.hpp>
#include <objtools/edit/apply_object.hpp>

#include <wx/menu.h>
#include <wx/msgdlg.h>

BEGIN_NCBI_SCOPE
USING_SCOPE(objects);

string CBioseqEditorFactory::GetExtensionIdentifier() const
{
    static string sid("bioseq_editor_factory");
    return sid;
}

string CBioseqEditorFactory::GetExtensionLabel() const
{
    static string slabel("Bioseq Editor Factory");
    return slabel;
}

typedef struct featcmd {
  EBioseqEditCommands cmd;
  string feature_name;
} FeatCmd;

static const FeatCmd feat_cmds[] = 
{
  { eCmdCreateCodingRegion, "Coding Region" },
  { eCmdCreateCodingRegion_ext, "CDS" },
  { eCmdCreateGene, "gene" },
  { eCmdCreateProt, "prot" },
  { eCmdCreateLTR, "LTR" },
  { eCmdCreateRepeatRegion, "repeat_region" },
  { eCmdCreateStemLoop, "stem_loop" },
  { eCmdCreateSTS, "STS" },
  { eCmdCreateRegion, "Region" },
  { eCmdCreateSecondaryStructure, "Secondary Structure" },
  { eCmdCreateProprotein, "proprotein" },
  { eCmdCreateMatPeptide, "mat_peptide" },
  { eCmdCreateSigPeptide, "sig_peptide" },
  { eCmdCreateTransitPeptide, "trans_peptide" },
  { eCmdCreateOperon, "operon" },
  { eCmdCreateMobileElement, "mobile_element" },
  { eCmdCreateCentromere, "centromere" },
  { eCmdCreateTelomere, "telomere" },
  { eCmdCreateRegulatory, "regulatory" },
  { eCmdCreateRegulatory_promoter, "promoter"},
  { eCmdCreateRegulatory_enhancer, "enhancer"},
  { eCmdCreateRegulatory_ribosome_binding_site, "ribosome_binding_site"},
  { eCmdCreateRegulatory_riboswitch, "riboswitch"},
  { eCmdCreateRegulatory_terminator, "terminator"},
  { eCmdCreatePreRNA, "preRNA" },
  { eCmdCreateMRNA, "mRNA" },
  { eCmdCreateExon, "exon" },
  { eCmdCreateIntron, "intron" },
  { eCmdCreate3UTR, "3'UTR" },
  { eCmdCreate5UTR, "5'UTR" },
  { eCmdCreatetRNA, "tRNA" },
  { eCmdCreaterRNA, "rRNA" },
  { eCmdCreateRNA, "RNA" },
  { eCmdCreatemiscRNA, "miscRNA" },
  { eCmdCreatepreRNA, "preRNA" },
  { eCmdCreatencRNA, "ncRNA" },
  { eCmdCreatetmRNA, "tmRNA" },
  { eCmdCreatePolyASite, "polyA_site" },
  { eCmdCreatePrimerBind, "primer_bind" } ,
  { eCmdCreateProteinBind, "protein_bind" },
  { eCmdCreateRepOrigin, "rep_origin" },
  { eCmdCreateBond, "Bond" },
  { eCmdCreateSite, "Site" },
  { eCmdCreateOriT, "oriT" },
  { eCmdCreateCRegion, "C_region" },
  { eCmdCreateDLoop, "D-loop" },
  { eCmdCreateDSegment, "D_segment" },
  { eCmdCreateiDNA, "iDNA" },
  { eCmdCreateJSegment, "J_segment" },
  { eCmdCreateMiscBinding, "misc_binding" },
  { eCmdCreateMiscDifference,  "misc_difference" },
  { eCmdCreateMiscFeature, "misc_feature" },
  { eCmdCreateMiscRecomb, "misc_recomb" },
  { eCmdCreateMiscStructure, "misc_structure" },
  { eCmdCreateModifiedBase, "modified_base" },
  { eCmdCreateNRegion, "N_region" } ,
  { eCmdCreatePrimTranscript, "prim_transcript" },
  { eCmdCreateSRegion,  "S_REGION" },
  { eCmdCreateUnsure, "unsure" },
  { eCmdCreateVRegion, "V_region" },
  { eCmdCreateVSegment, "V_segment" },
  { eCmdCreateVariation, "variation" },
  { eCmdCreateAssemblyGap, "assembly_gap" },
  { eCmdCreateTPAAssembly, "TPA Assembly" },
  { eCmdCreateStructuredComment, "StructuredComment" },
  { eCmdCreateDBLink, "DBLink" },
  { eCmdCreateUnverified, "Unverified" },
  { eCmdCreateAuthorizedAccess, "AuthorizedAccess" },
  { eCmdCreateRefGeneTracking, "RefGene Tracking" },
  { eCmdCreateRegionDescriptor, "Region" },
  { eCmdCreateCommentDescriptor, "Comment" },
  { eCmdCreateTitle, "Title" },
  { eCmdCreateMolInfo, "Molecule Description" },
  { eCmdCreatePubDescriptor, "Publication" },
  { eCmdCreatePubDescriptorLabeled, "Publication" },
  { eCmdCreatePubFeatureLabeled, "Publication" }
};

static const int num_feat_cmds = sizeof (feat_cmds) / sizeof (FeatCmd);


static string GetKeyFromCmdID (int cmd_id)
{
    for (int i = 0; i < num_feat_cmds; i++) {
        if (feat_cmds[i].cmd == cmd_id) {
            return feat_cmds[i].feature_name;
        }
    }
    return "";
}


typedef struct feattypecmd {
  EBioseqEditCommands cmd;
  CSeqFeatData::ESubtype subtype;
} FeatTypeCmd;

static const FeatTypeCmd feat_type_cmds[] = 
{
    { eCmdCreateGene, CSeqFeatData::eSubtype_gene } ,
    { eCmdCreateCodingRegion, CSeqFeatData::eSubtype_cdregion },
    { eCmdCreateCodingRegion_ext, CSeqFeatData::eSubtype_cdregion },
    { eCmdCreateProt, CSeqFeatData::eSubtype_prot },
    { eCmdCreateLTR, CSeqFeatData::eSubtype_LTR },
    { eCmdCreateRepeatRegion, CSeqFeatData::eSubtype_repeat_region },
  { eCmdCreateStemLoop, CSeqFeatData::eSubtype_stem_loop },
  { eCmdCreateSTS, CSeqFeatData::eSubtype_STS },
  { eCmdCreateRegion, CSeqFeatData::eSubtype_region },
  { eCmdCreatePubFeatureLabeled, CSeqFeatData::eSubtype_pub },
  { eCmdCreateSecondaryStructure, CSeqFeatData::eSubtype_psec_str },
  { eCmdCreateProprotein, CSeqFeatData::eSubtype_preprotein },
  { eCmdCreateMatPeptide, CSeqFeatData::eSubtype_mat_peptide_aa },
  { eCmdCreateSigPeptide, CSeqFeatData::eSubtype_sig_peptide_aa },
  { eCmdCreateTransitPeptide, CSeqFeatData::eSubtype_transit_peptide_aa },
  { eCmdCreateOperon, CSeqFeatData::eSubtype_operon },
  { eCmdCreateMobileElement, CSeqFeatData::eSubtype_mobile_element },
  { eCmdCreateCentromere, CSeqFeatData::eSubtype_centromere },
  { eCmdCreateTelomere, CSeqFeatData::eSubtype_telomere },
  { eCmdCreatePreRNA, CSeqFeatData::eSubtype_preRNA },
  { eCmdCreateMRNA, CSeqFeatData::eSubtype_mRNA },
  { eCmdCreateExon, CSeqFeatData::eSubtype_exon },
  { eCmdCreateIntron, CSeqFeatData::eSubtype_intron },
  { eCmdCreate3UTR, CSeqFeatData::eSubtype_3UTR },
  { eCmdCreate5UTR, CSeqFeatData::eSubtype_5UTR },
  { eCmdCreatetRNA, CSeqFeatData::eSubtype_tRNA },
  { eCmdCreaterRNA, CSeqFeatData::eSubtype_rRNA },
  { eCmdCreateRNA, CSeqFeatData::eSubtype_misc_RNA },
  { eCmdCreatemiscRNA, CSeqFeatData::eSubtype_misc_RNA },
    { eCmdCreatepreRNA, CSeqFeatData::eSubtype_preRNA },
  { eCmdCreatencRNA, CSeqFeatData::eSubtype_ncRNA },
  { eCmdCreatetmRNA, CSeqFeatData::eSubtype_tmRNA },
  { eCmdCreateCommentDescriptor, CSeqFeatData::eSubtype_comment },
  { eCmdCreateBiosourceFeat, CSeqFeatData::eSubtype_biosrc },
  { eCmdCreatePolyASite, CSeqFeatData::eSubtype_polyA_site },
  { eCmdCreatePrimerBind, CSeqFeatData::eSubtype_primer_bind } ,
  { eCmdCreateProteinBind, CSeqFeatData::eSubtype_protein_bind },
  { eCmdCreateRepOrigin, CSeqFeatData::eSubtype_rep_origin },
  { eCmdCreateBond, CSeqFeatData::eSubtype_bond },
  { eCmdCreateSite, CSeqFeatData::eSubtype_site },
  { eCmdCreateOriT, CSeqFeatData::eSubtype_oriT },
  { eCmdCreateCRegion, CSeqFeatData::eSubtype_C_region },
  { eCmdCreateDLoop, CSeqFeatData::eSubtype_D_loop },
  { eCmdCreateDSegment, CSeqFeatData::eSubtype_D_segment },
  { eCmdCreateiDNA, CSeqFeatData::eSubtype_iDNA },
  { eCmdCreateJSegment, CSeqFeatData::eSubtype_J_segment },
  { eCmdCreateMiscBinding, CSeqFeatData::eSubtype_misc_binding },
  { eCmdCreateMiscDifference,  CSeqFeatData::eSubtype_misc_difference },
  { eCmdCreateMiscFeature, CSeqFeatData::eSubtype_misc_feature },
  { eCmdCreateMiscRecomb, CSeqFeatData::eSubtype_misc_recomb },
  { eCmdCreateMiscStructure, CSeqFeatData::eSubtype_misc_structure },
  { eCmdCreateModifiedBase, CSeqFeatData::eSubtype_modified_base },
  { eCmdCreateNRegion, CSeqFeatData::eSubtype_N_region } ,
  { eCmdCreatePrimTranscript, CSeqFeatData::eSubtype_prim_transcript },
  { eCmdCreateRegulatory, CSeqFeatData::eSubtype_regulatory },
  { eCmdCreateRegulatory_promoter, CSeqFeatData::eSubtype_regulatory },
  { eCmdCreateRegulatory_enhancer, CSeqFeatData::eSubtype_regulatory },
  { eCmdCreateRegulatory_ribosome_binding_site, CSeqFeatData::eSubtype_regulatory },
  { eCmdCreateRegulatory_riboswitch, CSeqFeatData::eSubtype_regulatory },
  { eCmdCreateRegulatory_terminator, CSeqFeatData::eSubtype_regulatory },
  { eCmdCreateSRegion,  CSeqFeatData::eSubtype_S_region },
  { eCmdCreateUnsure, CSeqFeatData::eSubtype_unsure },
  { eCmdCreateVRegion, CSeqFeatData::eSubtype_V_region },
  { eCmdCreateVSegment, CSeqFeatData::eSubtype_V_segment },
  { eCmdCreateVariation, CSeqFeatData::eSubtype_variation },
  { eCmdCreateAssemblyGap, CSeqFeatData::eSubtype_assembly_gap }

};

static const int num_feat_type_cmds = sizeof (feat_type_cmds) / sizeof (FeatTypeCmd);


CSeqFeatData::ESubtype CBioseqEditor::GetFeatTypeFromCmdID (int cmd_id)
{
    for (int i = 0; i < num_feat_type_cmds; i++) {
        if (feat_type_cmds[i].cmd == cmd_id) {
            return feat_type_cmds[i].subtype;
        }
    }
    return CSeqFeatData::eSubtype_bad;
}

CRef<CSeq_feat>  CBioseqEditor::MakeDefaultFeature(CSeqFeatData::ESubtype subtype)
{
    CRef<CSeq_feat> feat(new CSeq_feat());

    switch (subtype) {
        case CSeqFeatData::eSubtype_cdregion:
            feat->SetData().SetCdregion();
            break;
        case CSeqFeatData::eSubtype_gene:
            feat->SetData().SetGene();
            break;
        case CSeqFeatData::eSubtype_prot:
            feat->SetData().SetProt();
            break;
        case CSeqFeatData::eSubtype_preprotein:
            feat->SetData().SetProt().SetProcessed(CProt_ref::eProcessed_preprotein);
            break;
        case CSeqFeatData::eSubtype_mat_peptide_aa:
            feat->SetData().SetProt().SetProcessed(CProt_ref::eProcessed_mature);
            break;
        case CSeqFeatData::eSubtype_sig_peptide_aa:
            feat->SetData().SetProt().SetProcessed(CProt_ref::eProcessed_signal_peptide);
            break;
        case CSeqFeatData::eSubtype_transit_peptide_aa:
            feat->SetData().SetProt().SetProcessed(CProt_ref::eProcessed_transit_peptide);
            break;
        case CSeqFeatData::eSubtype_preRNA:
            feat->SetData().SetRna().SetType(CRNA_ref::eType_premsg);
            break;
        case CSeqFeatData::eSubtype_mRNA:
            feat->SetData().SetRna().SetType(CRNA_ref::eType_mRNA);
            break;
        case CSeqFeatData::eSubtype_tRNA:
            feat->SetData().SetRna().SetType(CRNA_ref::eType_tRNA);
            break;
        case CSeqFeatData::eSubtype_rRNA:
            feat->SetData().SetRna().SetType(CRNA_ref::eType_rRNA);
            break;
        case CSeqFeatData::eSubtype_ncRNA:
            feat->SetData().SetRna().SetType(CRNA_ref::eType_ncRNA);
            break;
        case CSeqFeatData::eSubtype_tmRNA:
            feat->SetData().SetRna().SetType(CRNA_ref::eType_tmRNA);
            break;
        case CSeqFeatData::eSubtype_misc_RNA:
            feat->SetData().SetRna().SetType(CRNA_ref::eType_miscRNA);
            break;
        case CSeqFeatData::eSubtype_bond:
            feat->SetData().SetBond();
            break;
        case CSeqFeatData::eSubtype_site:
            feat->SetData().SetSite();
            break;

        case CSeqFeatData::eSubtype_psec_str:
            feat->SetData().SetPsec_str(CSeqFeatData::ePsec_str_helix); 
            break;
        case CSeqFeatData::eSubtype_region:
            feat->SetData().SetRegion();
            break;
        case CSeqFeatData::eSubtype_pub:
            feat->SetData().SetPub();
            break;

        case CSeqFeatData::eSubtype_comment:
            feat->SetData().SetComment();
            break;
        case CSeqFeatData::eSubtype_biosrc:
            feat->SetData().SetBiosrc();
            break;
        default:
            feat->SetData().SetImp().SetKey(CSeqFeatData::SubtypeValueToName(subtype));
            break;
    }
    return feat;
}

static string s_GetMenuName(int i) 
{
    if (feat_cmds[i].cmd == eCmdCreatePubDescriptorLabeled) {
        return "Publication Descriptor";
    } else if (feat_cmds[i].cmd == eCmdCreatePubFeatureLabeled) {
        return "Publication Feature";
    } else {
        return feat_cmds[i].feature_name;
    }
}

void CBioseqEditorFactory::CollectBioseqEditorCommands(vector<CUICommand*> &commands)
{
 static bool registered = false;
    if (!registered) {
        registered = true;

        commands.push_back(new CUICommand(eCmdEditObjects,
                                          "Edit Selected Object(s)\tCtrl+Enter",
                                          "Edit Selected Object(s)",
                                          NcbiEmptyString,
                                          NcbiEmptyString,
                                          "Command opens Dialog to Edit Selected Object(s)"));
        commands.push_back(new CUICommand(eCmdDeleteObjects,
                                          "Delete Selected Object(s)\tCtrl+Back",
                                          "Delete Selected Object(s)",
                                          NcbiEmptyString,
                                          NcbiEmptyString,
                                          "Command deletes Selected Object(s)"));
        commands.push_back(new CUICommand(eCmdEditBiosourceDesc,
                                          "Edit BioSource Descriptor",
                                          "Edit BioSource Descriptor",
                                          NcbiEmptyString,
                                          NcbiEmptyString,
                                          "Command opens Dialog to Edit Biosource Descriptor of the Current Bioseq"));
        commands.push_back(new CUICommand(eCmdDeleteBiosourceDesc,
                                          "Delete BioSource Descriptor",
                                          "Delete BioSource Descriptor",
                                          NcbiEmptyString,
                                          NcbiEmptyString,
                                          "Command deletes Biosource Descriptor of the Current Bioseq"));
        commands.push_back(new CUICommand(eCmdCreateBiosourceDesc,
                                          "Create BioSource Descriptor",
                                          "Create BioSource Descriptor",
                                          NcbiEmptyString,
                                          NcbiEmptyString,
                                          "Command opens Dialog to Create Biosource Descriptor of the Current Bioseq"));
        commands.push_back(new CUICommand(eCmdEditBiosourceFeat,
                                          "Edit BioSource Feature",
                                          "Edit BioSource Feature",
                                          NcbiEmptyString,
                                          NcbiEmptyString,
                                          "Command opens Dialog to Edit Biosource Feature of the Current Bioseq"));
        commands.push_back(new CUICommand(eCmdDeleteBiosourceFeat,
                                          "Delete BioSource Feature",
                                          "Delete BioSource Feature",
                                          NcbiEmptyString,
                                          NcbiEmptyString,
                                          "Command deletes Biosource Feature of the Current Bioseq"));
        commands.push_back(new CUICommand(eCmdCreateBiosourceFeat,
                                          "Create BioSource Feature",
                                          "Create BioSource Feature",
                                          NcbiEmptyString,
                                          NcbiEmptyString,
                                          "Command opens Dialog to Create Feature Descriptor of the Current Bioseq"));
        
        commands.push_back(new CUICommand(eCmdPropagateAllFeatures,
                                          "Propagate All Features",
                                          "Propagate All Features",
                                          NcbiEmptyString,
                                          NcbiEmptyString,
                                          "Command opens Dialog to Propagate All Features from the Current Bioseq"));
        
        commands.push_back(new CUICommand(eCmdPropagateSelectedFeatures,
                                          "Propagate Selected Feature(s)",
                                          "Propagate Selected Feature(s)",
                                          NcbiEmptyString,
                                          NcbiEmptyString,
                                          "Command opens Dialog to Propagate Selected Feature from the Current Bioseq"));
        
        for (int i = 0; i < num_feat_cmds; i++) {
            string feature_name = feat_cmds[i].feature_name;
            string menu_name = s_GetMenuName(i);
            string description = "Command opens Dialog to Create " + feature_name + " on the Current Bioseq";
            commands.push_back(new CUICommand(feat_cmds[i].cmd,
                                              menu_name,
                                              feature_name,
                                              NcbiEmptyString,
                                              NcbiEmptyString,
                                              description));
        }              
    }
}

void CBioseqEditorFactory::RegisterCommands(CUICommandRegistry& cmd_reg, wxFileArtProvider&)
{
    vector<CUICommand*> commands;
    CollectBioseqEditorCommands(commands);
    for (size_t i = 0; i < commands.size(); i++)
    {
        cmd_reg.RegisterCommand(commands[i]);
    }

    wxAcceleratorEntry accelerator1(wxACCEL_CTRL, WXK_BACK, eCmdDeleteObjects);
    cmd_reg.AddAccelerator(accelerator1);
//    wxAcceleratorEntry accelerator2(wxACCEL_NORMAL, WXK_NUMPAD_DELETE, eCmdDeleteObjects);
//    cmd_reg.AddAccelerator(accelerator2);
    wxAcceleratorEntry accelerator3(wxACCEL_CTRL, WXK_RETURN, eCmdEditObjects);
    cmd_reg.AddAccelerator(accelerator3);
//    wxAcceleratorEntry accelerator4(wxACCEL_NORMAL, WXK_NUMPAD_ENTER, eCmdEditObjects);
//    cmd_reg.AddAccelerator(accelerator4);

}

CObject* CBioseqEditorFactory::CreateEditor(const string& interface_name, ICommandProccessor& cmdProccessor, IGuiWidgetHost* guiWidgetHost)
{
    if (typeid(IBioseqEditor).name() == interface_name) {
        return new CBioseqEditor(cmdProccessor, guiWidgetHost);
    }
    return 0;
}

static
WX_DEFINE_MENU(kBioseqEditorShortContextMenu)
    WX_MENU_SEPARATOR_L("Edit Selection")
    WX_MENU_ITEM(eCmdEditObjects)
    WX_MENU_ITEM(eCmdDeleteObjects)
WX_END_MENU()

static
WX_DEFINE_MENU(kBioseqEditorContextMenu)
    WX_MENU_SEPARATOR_L("Edit Selection")
    WX_MENU_ITEM(eCmdEditObjects)
    WX_MENU_ITEM(eCmdDeleteObjects)
    WX_MENU_ITEM(eCmdPropagateSelectedFeatures)
    WX_MENU_ITEM(eCmdPropagateAllFeatures)
WX_END_MENU()

wxMenu* CBioseqEditor::CreateContextMenu(bool shorter) const
{
    if (shorter) {
        return CUICommandRegistry::GetInstance().CreateMenu(kBioseqEditorShortContextMenu);
    }
    return CUICommandRegistry::GetInstance().CreateMenu(kBioseqEditorContextMenu);
}

static
WX_DEFINE_MENU(kBioseqEditorBioseqMenu)
    WX_SUBMENU("BioSource")
        WX_MENU_SEPARATOR_L("Edit Bioseq")
        WX_MENU_ITEM(eCmdEditBiosourceDesc)
        WX_MENU_ITEM(eCmdDeleteBiosourceDesc)
        WX_MENU_ITEM(eCmdCreateBiosourceDesc)
        WX_MENU_ITEM(eCmdEditBiosourceFeat)
        WX_MENU_ITEM(eCmdDeleteBiosourceFeat)
        WX_MENU_ITEM(eCmdCreateBiosourceFeat)
    WX_END_SUBMENU()
    WX_SUBMENU("Create Features")
        WX_MENU_SEPARATOR_L("Edit Bioseq")
        WX_MENU_ITEM(eCmdCreateBiosourceFeat)
        WX_SUBMENU("Genes and Named Regions")
          WX_MENU_ITEM(eCmdCreateGene)
          WX_MENU_ITEM_INT(eCmdCreateProt)
          WX_MENU_ITEM(eCmdCreateRepeatRegion)
          WX_MENU_ITEM(eCmdCreateStemLoop)
          WX_MENU_ITEM_INT(eCmdCreateSTS)
          WX_MENU_ITEM_INT(eCmdCreateRegion)
          WX_MENU_ITEM_INT(eCmdCreateSecondaryStructure)
          WX_MENU_ITEM(eCmdCreateProprotein)
          WX_MENU_ITEM(eCmdCreateMatPeptide)
          WX_MENU_ITEM(eCmdCreateSigPeptide)
          WX_MENU_ITEM(eCmdCreateTransitPeptide)
          WX_MENU_ITEM(eCmdCreateOperon)
          WX_MENU_ITEM(eCmdCreateMobileElement)
          WX_MENU_ITEM(eCmdCreateCentromere)
          WX_MENU_ITEM(eCmdCreateTelomere)
          WX_MENU_ITEM(eCmdCreateRegulatory)
        WX_END_SUBMENU()
        WX_SUBMENU("Coding Regions and Transcripts")
          WX_MENU_ITEM(eCmdCreateCodingRegion)
          WX_MENU_ITEM_INT(eCmdCreatePreRNA)
          WX_MENU_ITEM(eCmdCreateMRNA)
          WX_MENU_ITEM(eCmdCreateExon)
          WX_MENU_ITEM(eCmdCreateIntron)
          WX_MENU_ITEM(eCmdCreate3UTR)
          WX_MENU_ITEM(eCmdCreate5UTR)
        WX_END_SUBMENU()
        WX_SUBMENU("Structural RNAs")
          WX_MENU_ITEM(eCmdCreatetRNA)
          WX_MENU_ITEM(eCmdCreaterRNA)
          WX_MENU_ITEM(eCmdCreateRNA)
          WX_MENU_ITEM(eCmdCreatencRNA)
          WX_MENU_ITEM(eCmdCreatetmRNA)
        WX_END_SUBMENU()
        WX_SUBMENU("Sites and Bonds")
          WX_MENU_ITEM(eCmdCreatePolyASite)
          WX_MENU_ITEM(eCmdCreatePrimerBind)
          WX_MENU_ITEM(eCmdCreateProteinBind)
          WX_MENU_ITEM(eCmdCreateRepOrigin)
          WX_MENU_ITEM_INT(eCmdCreateBond)
          WX_MENU_ITEM_INT(eCmdCreateSite)
          WX_MENU_ITEM(eCmdCreateOriT)
        WX_END_SUBMENU()
        WX_SUBMENU("Remaining Features")
          WX_MENU_ITEM_INT(eCmdCreateCRegion)
          WX_MENU_ITEM(eCmdCreateDLoop)
          WX_MENU_ITEM_INT(eCmdCreateDSegment)
          WX_MENU_ITEM(eCmdCreateiDNA)
          WX_MENU_ITEM_INT(eCmdCreateJSegment)
          WX_MENU_ITEM(eCmdCreateMiscBinding)
          WX_MENU_ITEM(eCmdCreateMiscDifference)
          WX_MENU_ITEM(eCmdCreateMiscFeature)
          WX_MENU_ITEM(eCmdCreateMiscRecomb)
          WX_MENU_ITEM(eCmdCreateMiscStructure)
          WX_MENU_ITEM_INT(eCmdCreateModifiedBase)
          WX_MENU_ITEM_INT(eCmdCreateNRegion)
          WX_MENU_ITEM_INT(eCmdCreatePrimTranscript)
          WX_MENU_ITEM_INT(eCmdCreateSRegion)
          WX_MENU_ITEM(eCmdCreateUnsure)
          WX_MENU_ITEM_INT(eCmdCreateVRegion)
          WX_MENU_ITEM_INT(eCmdCreateVSegment)
          WX_MENU_ITEM(eCmdCreateVariation)
        WX_END_SUBMENU()
    WX_END_SUBMENU()        
    WX_SUBMENU("Publications")
        WX_MENU_SEPARATOR_L("Edit Bioseq")
        WX_MENU_ITEM(eCmdCreatePubDescriptorLabeled)
        WX_MENU_ITEM_INT(eCmdCreatePubFeatureLabeled)
    WX_END_SUBMENU()        
    WX_SUBMENU("Descriptors")
        WX_MENU_SEPARATOR_L("Edit Bioseq")
        WX_MENU_ITEM_INT(eCmdCreateTPAAssembly)
        WX_MENU_ITEM(eCmdCreateStructuredComment)
        WX_MENU_ITEM_INT(eCmdCreateRefGeneTracking)
        WX_MENU_ITEM(eCmdCreateDBLink)
        WX_MENU_ITEM_INT(eCmdCreateUnverified)
        WX_MENU_ITEM_INT(eCmdCreateAuthorizedAccess)
        WX_MENU_ITEM(eCmdCreateRegionDescriptor)
        WX_MENU_ITEM_INT(eCmdCreateCommentDescriptor)
        WX_MENU_ITEM(eCmdCreateTitle)
        WX_MENU_ITEM(eCmdCreateMolInfo)
        WX_MENU_ITEM(eCmdCreatePubDescriptor)
    WX_END_SUBMENU()        
WX_END_MENU()

wxMenu* CBioseqEditor::CreateBioseqMenu() const
{
    return CUICommandRegistry::GetInstance().CreateMenu(kBioseqEditorBioseqMenu);
}

static
WX_DEFINE_MENU(kTextViewTopMenu)
    WX_SUBMENU("&Edit")
        WX_MENU_SEPARATOR_L("Contribs")
            WX_MENU_ITEM(eCmdEditObjects)
            WX_MENU_ITEM(eCmdDeleteObjects)
    WX_END_SUBMENU()
WX_END_MENU()

const SwxMenuItemRec* CBioseqEditor::GetMenuDef() const
{
    return kTextViewTopMenu;
}

BEGIN_EVENT_TABLE( CBioseqEditor, wxEvtHandler )
    EVT_MENU(eCmdEditBiosourceDesc, CBioseqEditor::OnEditBiosourceDesc)
    EVT_UPDATE_UI(eCmdEditBiosourceDesc, CBioseqEditor::OnUpdateEditBiosourceDesc)
    EVT_MENU(eCmdCreateBiosourceDesc, CBioseqEditor::OnCreateBiosourceDesc)
    EVT_UPDATE_UI(eCmdCreateBiosourceDesc, CBioseqEditor::OnUpdateCreateBiosourceDesc)
    EVT_MENU(eCmdDeleteBiosourceDesc, CBioseqEditor::OnDeleteBiosourceDesc)
    EVT_UPDATE_UI(eCmdDeleteBiosourceDesc, CBioseqEditor::OnUpdateDeleteBiosourceDesc)

    EVT_MENU(eCmdEditBiosourceFeat, CBioseqEditor::OnEditBiosourceFeat)
    EVT_UPDATE_UI(eCmdEditBiosourceFeat, CBioseqEditor::OnUpdateEditBiosourceFeat)
    EVT_MENU(eCmdCreateBiosourceFeat, CBioseqEditor::OnCreateBiosourceFeat)
    EVT_UPDATE_UI(eCmdCreateBiosourceFeat, CBioseqEditor::OnUpdateCreateBiosourceFeat)
    EVT_MENU(eCmdDeleteBiosourceFeat, CBioseqEditor::OnDeleteBiosourceFeat)
    EVT_UPDATE_UI(eCmdDeleteBiosourceFeat, CBioseqEditor::OnUpdateDeleteBiosourceFeat)
    EVT_MENU(eCmdPropagateAllFeatures, CBioseqEditor::OnPropagateAllFeatures)
    EVT_UPDATE_UI(eCmdPropagateAllFeatures, CBioseqEditor::OnUpdatePropagateAllFeatures)

    EVT_MENU(eCmdCreateCodingRegion, CBioseqEditor::OnCreateCodingRegion)
    EVT_UPDATE_UI(eCmdCreateCodingRegion, CBioseqEditor::OnUpdateNucleotideFeat)
    EVT_MENU(eCmdCreateGene, CBioseqEditor::OnCreateGene)
    EVT_UPDATE_UI(eCmdCreateGene, CBioseqEditor::OnUpdateNucleotideFeat)
    EVT_MENU(eCmdCreateProt, CBioseqEditor::OnCreateProt)
    EVT_UPDATE_UI(eCmdCreateProt, CBioseqEditor::OnUpdateProteinFeat)
    EVT_MENU(eCmdCreateLTR, CBioseqEditor::OnCreateImport)
    EVT_UPDATE_UI(eCmdCreateLTR, CBioseqEditor::OnUpdateNucleotideFeat)
    EVT_MENU(eCmdCreateRepeatRegion, CBioseqEditor::OnCreateImport)
    EVT_UPDATE_UI(eCmdCreateRepeatRegion, CBioseqEditor::OnUpdateNucleotideFeat)
    EVT_MENU(eCmdCreateStemLoop, CBioseqEditor::OnCreateImport)
    EVT_UPDATE_UI(eCmdCreateStemLoop, CBioseqEditor::OnUpdateNucleotideFeat)
    EVT_MENU(eCmdCreateSTS, CBioseqEditor::OnCreateImport)
    EVT_UPDATE_UI(eCmdCreateSTS, CBioseqEditor::OnUpdateNucleotideFeat)
    EVT_MENU(eCmdCreateRegion, CBioseqEditor::OnCreateRegion)
    EVT_UPDATE_UI(eCmdCreateRegion, CBioseqEditor::OnUpdateAnyFeat)
    EVT_MENU(eCmdCreateSecondaryStructure, CBioseqEditor::OnCreateSecondaryStructure)
    EVT_UPDATE_UI(eCmdCreateSecondaryStructure, CBioseqEditor::OnUpdateProteinFeat)
    EVT_MENU(eCmdCreateProprotein, CBioseqEditor::OnCreateProt)
    EVT_UPDATE_UI(eCmdCreateProprotein, CBioseqEditor::OnUpdateProteinFeat)
    EVT_MENU(eCmdCreateMatPeptide, CBioseqEditor::OnCreateProt)
    EVT_UPDATE_UI(eCmdCreateMatPeptide, CBioseqEditor::OnUpdateProteinFeat)
    EVT_MENU(eCmdCreateSigPeptide, CBioseqEditor::OnCreateProt)
    EVT_UPDATE_UI(eCmdCreateSigPeptide, CBioseqEditor::OnUpdateProteinFeat)
    EVT_MENU(eCmdCreateTransitPeptide, CBioseqEditor::OnCreateProt)
    EVT_UPDATE_UI(eCmdCreateTransitPeptide, CBioseqEditor::OnUpdateProteinFeat)
    EVT_MENU(eCmdCreateOperon, CBioseqEditor::OnCreateImport)
    EVT_UPDATE_UI(eCmdCreateOperon, CBioseqEditor::OnUpdateNucleotideFeat)
    EVT_MENU(eCmdCreateMobileElement, CBioseqEditor::OnCreateImport)
    EVT_UPDATE_UI(eCmdCreateMobileElement, CBioseqEditor::OnUpdateNucleotideFeat)
    EVT_MENU(eCmdCreateCentromere, CBioseqEditor::OnCreateImport)
    EVT_UPDATE_UI(eCmdCreateCentromere, CBioseqEditor::OnUpdateNucleotideFeat)
    EVT_MENU(eCmdCreateTelomere, CBioseqEditor::OnCreateImport)
    EVT_UPDATE_UI(eCmdCreateTelomere, CBioseqEditor::OnUpdateNucleotideFeat)
    EVT_MENU(eCmdCreateRegulatory, CBioseqEditor::OnCreateImport)
    EVT_UPDATE_UI(eCmdCreateRegulatory, CBioseqEditor::OnUpdateNucleotideFeat)

    EVT_MENU(eCmdCreatePreRNA, CBioseqEditor::OnCreateRNA)
    EVT_UPDATE_UI(eCmdCreatePreRNA, CBioseqEditor::OnUpdateNucleotideFeat)
    EVT_MENU(eCmdCreateMRNA, CBioseqEditor::OnCreateRNA)
    EVT_UPDATE_UI(eCmdCreateMRNA, CBioseqEditor::OnUpdateNucleotideFeat)
    EVT_MENU(eCmdCreateExon, CBioseqEditor::OnCreateImport)
    EVT_UPDATE_UI(eCmdCreateExon, CBioseqEditor::OnUpdateNucleotideFeat)
    EVT_MENU(eCmdCreateIntron, CBioseqEditor::OnCreateImport)
    EVT_UPDATE_UI(eCmdCreateIntron, CBioseqEditor::OnUpdateNucleotideFeat)
    EVT_MENU(eCmdCreate3UTR, CBioseqEditor::OnCreateImport)
    EVT_UPDATE_UI(eCmdCreate3UTR, CBioseqEditor::OnUpdateNucleotideFeat)
    EVT_MENU(eCmdCreate5UTR, CBioseqEditor::OnCreateImport)
    EVT_UPDATE_UI(eCmdCreate5UTR, CBioseqEditor::OnUpdateNucleotideFeat)

    EVT_MENU(eCmdCreatetRNA, CBioseqEditor::OnCreateRNA)
    EVT_UPDATE_UI(eCmdCreatetRNA, CBioseqEditor::OnUpdateNucleotideFeat)
    EVT_MENU(eCmdCreaterRNA, CBioseqEditor::OnCreateRNA)
    EVT_UPDATE_UI(eCmdCreaterRNA, CBioseqEditor::OnUpdateNucleotideFeat)
    EVT_MENU(eCmdCreateRNA, CBioseqEditor::OnCreateRNA)
    EVT_UPDATE_UI(eCmdCreateRNA, CBioseqEditor::OnUpdateNucleotideFeat)
    EVT_MENU(eCmdCreatencRNA, CBioseqEditor::OnCreateRNA)
    EVT_UPDATE_UI(eCmdCreatencRNA, CBioseqEditor::OnUpdateNucleotideFeat)
    EVT_MENU(eCmdCreatetmRNA, CBioseqEditor::OnCreateRNA)
    EVT_UPDATE_UI(eCmdCreatetmRNA, CBioseqEditor::OnUpdateNucleotideFeat)

    EVT_MENU(eCmdCreatePolyASite, CBioseqEditor::OnCreateImport)
    EVT_UPDATE_UI(eCmdCreatePolyASite, CBioseqEditor::OnUpdateNucleotideFeat)
    EVT_MENU(eCmdCreatePrimerBind, CBioseqEditor::OnCreateImport)
    EVT_UPDATE_UI(eCmdCreatePrimerBind, CBioseqEditor::OnUpdateNucleotideFeat)
    EVT_MENU(eCmdCreateProteinBind, CBioseqEditor::OnCreateImport)
    EVT_UPDATE_UI(eCmdCreateProteinBind, CBioseqEditor::OnUpdateNucleotideFeat)
    EVT_MENU(eCmdCreateRepOrigin, CBioseqEditor::OnCreateImport)
    EVT_UPDATE_UI(eCmdCreateRepOrigin, CBioseqEditor::OnUpdateNucleotideFeat)
    EVT_MENU(eCmdCreateBond, CBioseqEditor::OnCreateBond)
    EVT_UPDATE_UI(eCmdCreateBond, CBioseqEditor::OnUpdateProteinFeat)
    EVT_MENU(eCmdCreateSite, CBioseqEditor::OnCreateSite)
    EVT_UPDATE_UI(eCmdCreateSite, CBioseqEditor::OnUpdateAnyFeat)
    EVT_MENU(eCmdCreateOriT, CBioseqEditor::OnCreateImport)
    EVT_UPDATE_UI(eCmdCreateOriT, CBioseqEditor::OnUpdateNucleotideFeat)

    EVT_MENU(eCmdCreateCRegion, CBioseqEditor::OnCreateImport)
    EVT_UPDATE_UI(eCmdCreateCRegion, CBioseqEditor::OnUpdateNucleotideFeat)
    EVT_MENU(eCmdCreateDLoop, CBioseqEditor::OnCreateImport)
    EVT_UPDATE_UI(eCmdCreateDLoop, CBioseqEditor::OnUpdateNucleotideFeat)
    EVT_MENU(eCmdCreateDSegment, CBioseqEditor::OnCreateImport)
    EVT_UPDATE_UI(eCmdCreateDSegment, CBioseqEditor::OnUpdateNucleotideFeat)
    EVT_MENU(eCmdCreateiDNA, CBioseqEditor::OnCreateImport)
    EVT_UPDATE_UI(eCmdCreateiDNA, CBioseqEditor::OnUpdateNucleotideFeat)
    EVT_MENU(eCmdCreateJSegment, CBioseqEditor::OnCreateImport)
    EVT_UPDATE_UI(eCmdCreateJSegment, CBioseqEditor::OnUpdateNucleotideFeat)
    EVT_MENU(eCmdCreateMiscBinding, CBioseqEditor::OnCreateImport)
    EVT_UPDATE_UI(eCmdCreateMiscBinding, CBioseqEditor::OnUpdateNucleotideFeat)
    EVT_MENU(eCmdCreateMiscDifference , CBioseqEditor::OnCreateImport)
    EVT_UPDATE_UI(eCmdCreateMiscDifference , CBioseqEditor::OnUpdateNucleotideFeat)
    EVT_MENU(eCmdCreateMiscFeature, CBioseqEditor::OnCreateImport)
    EVT_UPDATE_UI(eCmdCreateMiscFeature, CBioseqEditor::OnUpdateNucleotideFeat)
    EVT_MENU(eCmdCreateMiscRecomb, CBioseqEditor::OnCreateImport)
    EVT_UPDATE_UI(eCmdCreateMiscRecomb, CBioseqEditor::OnUpdateNucleotideFeat)
    EVT_MENU(eCmdCreateMiscStructure, CBioseqEditor::OnCreateImport)
    EVT_UPDATE_UI(eCmdCreateMiscStructure, CBioseqEditor::OnUpdateNucleotideFeat)
    EVT_MENU(eCmdCreateModifiedBase, CBioseqEditor::OnCreateImport)
    EVT_UPDATE_UI(eCmdCreateModifiedBase, CBioseqEditor::OnUpdateNucleotideFeat)
    EVT_MENU(eCmdCreateNRegion, CBioseqEditor::OnCreateImport)
    EVT_UPDATE_UI(eCmdCreateNRegion, CBioseqEditor::OnUpdateNucleotideFeat)
    EVT_MENU(eCmdCreatePrimTranscript, CBioseqEditor::OnCreateImport)
    EVT_UPDATE_UI(eCmdCreatePrimTranscript, CBioseqEditor::OnUpdateNucleotideFeat)
    EVT_MENU(eCmdCreateSRegion, CBioseqEditor::OnCreateImport)
    EVT_UPDATE_UI(eCmdCreateSRegion, CBioseqEditor::OnUpdateNucleotideFeat)
    EVT_MENU(eCmdCreateUnsure, CBioseqEditor::OnCreateImport)
    EVT_UPDATE_UI(eCmdCreateUnsure, CBioseqEditor::OnUpdateNucleotideFeat)
    EVT_MENU(eCmdCreateVRegion, CBioseqEditor::OnCreateImport)
    EVT_UPDATE_UI(eCmdCreateVRegion, CBioseqEditor::OnUpdateNucleotideFeat)
    EVT_MENU(eCmdCreateVSegment, CBioseqEditor::OnCreateImport)
    EVT_UPDATE_UI(eCmdCreateVSegment, CBioseqEditor::OnUpdateNucleotideFeat)
    EVT_MENU(eCmdCreateVariation, CBioseqEditor::OnCreateImport)
    EVT_UPDATE_UI(eCmdCreateVariation, CBioseqEditor::OnUpdateNucleotideFeat)
    EVT_MENU(eCmdCreateAssemblyGap, CBioseqEditor::OnCreateImport)
    EVT_UPDATE_UI(eCmdCreateAssemblyGap, CBioseqEditor::OnUpdateNucleotideFeat)
    EVT_MENU(eCmdCreatePubFeatureLabeled, CBioseqEditor::OnCreatePubFeat)
    EVT_UPDATE_UI(eCmdCreatePubFeatureLabeled, CBioseqEditor::OnUpdateAnyFeat)

    EVT_MENU(eCmdCreateTPAAssembly, CBioseqEditor::OnCreateDescriptor)
    EVT_MENU(eCmdCreateStructuredComment, CBioseqEditor::OnCreateDescriptor)
    EVT_MENU(eCmdCreateDBLink, CBioseqEditor::OnCreateDescriptor)
    EVT_MENU(eCmdCreateUnverified, CBioseqEditor::OnCreateDescriptor)
    EVT_MENU(eCmdCreateAuthorizedAccess, CBioseqEditor::OnCreateDescriptor)
    EVT_MENU(eCmdCreateRefGeneTracking, CBioseqEditor::OnCreateDescriptor)
    EVT_MENU(eCmdCreateRegionDescriptor, CBioseqEditor::OnCreateDescriptor)
    EVT_MENU(eCmdCreateCommentDescriptor, CBioseqEditor::OnCreateDescriptor)
    EVT_MENU(eCmdCreateTitle, CBioseqEditor::OnCreateDescriptor)
    EVT_MENU(eCmdCreateMolInfo, CBioseqEditor::OnCreateDescriptor)
    EVT_MENU(eCmdCreatePubDescriptor, CBioseqEditor::OnCreateDescriptor)
    EVT_MENU(eCmdCreatePubDescriptorLabeled, CBioseqEditor::OnCreateDescriptor)

    EVT_MENU(eCmdEditObjects, CBioseqEditor::OnEditSelection)
    EVT_UPDATE_UI(eCmdEditObjects, CBioseqEditor::OnUpdateEditSelection)

    EVT_MENU(eCmdDeleteObjects, CBioseqEditor::OnDeleteSelection)
    EVT_UPDATE_UI(eCmdDeleteObjects, CBioseqEditor::OnUpdateDeleteSelection)

    EVT_MENU(eCmdPropagateSelectedFeatures, CBioseqEditor::OnPropagateSelectedFeatures)
    EVT_UPDATE_UI(eCmdPropagateSelectedFeatures, CBioseqEditor::OnUpdatePropagateSelectedFeatures)
END_EVENT_TABLE()

bool CBioseqEditor::ProcessEvent(wxEvent& event)
{
    if (!m_CB)
        return false;
    
    return wxEvtHandler::ProcessEvent(event);
}

void CBioseqEditor::OnUpdateEditBiosourceDesc(wxUpdateUIEvent& event)
{   
    event.Enable(x_HaveBiosourceDesc());
}

void CBioseqEditor::OnUpdateCreateBiosourceDesc(wxUpdateUIEvent& event)
{  
    event.Enable(!x_HaveBiosourceDesc() && x_IsNa());
}

void CBioseqEditor::OnUpdateDeleteBiosourceDesc(wxUpdateUIEvent& event)
{  
    event.Enable(x_HaveBiosourceDesc());
}

void CBioseqEditor::OnEditBiosourceDesc(wxCommandEvent& event)
{
    const char* szCommand = "Edit Biosource Descriptor";

    try {
        CBioseq_Handle bh = m_CB->GetCurrentBioseq();
        if (!bh)
        {
            event.Skip();
            return;
        }

        CSeqdesc_CI desc_it(bh, CSeqdesc::e_Source);
        if (!desc_it)
        {
            event.Skip();
            return;
        }

        CSeq_entry_Handle seh = bh.GetSeq_entry_Handle();
        CIRef<IEditObject> editor(new CEditObjectSeq_desc(*desc_it, seh, bh.GetScope(), false));
        ERR_POST(Info << "Start " << szCommand);
        x_EditObject(editor);
        ERR_POST(Info << "End " << szCommand);
    } catch (const std::exception& e) {
        x_ReportError(szCommand, e);
    }
}

void CBioseqEditor::OnCreateBiosourceDesc(wxCommandEvent& event)
{
    const char* szCommand = "Create Biosource Descriptor";

    try {
        CBioseq_Handle bh = m_CB->GetCurrentBioseq();
        if (!bh)
        {
            event.Skip();
            return;
        }

        CSeqdesc_CI desc_it(bh, CSeqdesc::e_Source);
        if (desc_it) {
            ERR_POST(Info << "Error: " << szCommand << ": Biosource descriptor already exists");
            return;
        }

        CRef<CSeqdesc> secdesc(new CSeqdesc());
        CRef<CBioSource> bioSource(new CBioSource());
        secdesc->SetSource(*bioSource);

        CSeq_entry_Handle seh = bh.GetSeq_entry_Handle();
        CIRef<IEditObject> editor(new CEditObjectSeq_desc(*secdesc, seh, bh.GetScope(), true));
        ERR_POST(Info << "Start " << szCommand);
        x_EditObject(editor);
        ERR_POST(Info << "End " << szCommand);

    } catch (const std::exception& e) {
        x_ReportError(szCommand, e);
    }
}

void CBioseqEditor::OnDeleteBiosourceDesc(wxCommandEvent& event)
{
    const char* szCommand = "Delete Biosource Descriptor";

    try {
        CBioseq_Handle bh = m_CB->GetCurrentBioseq();
        if (!bh)
        {
            event.Skip();
            return;
        }

        CSeqdesc_CI desc_it(bh, CSeqdesc::e_Source);
        if (!desc_it)
        {
            event.Skip();
            return;
        }

        CSeq_entry_Handle seh = bh.GetSeq_entry_Handle();
        x_DeleteObject(seh, *desc_it);
    } catch (const std::exception& e) {
        x_ReportError(szCommand, e);
    }
}

void CBioseqEditor::OnUpdateEditBiosourceFeat(wxUpdateUIEvent& event)
{
    event.Enable(x_HaveBiosourceFeat());
}

void CBioseqEditor::OnUpdateCreateBiosourceFeat(wxUpdateUIEvent& event)
{
    CBioseq_Handle bh = m_CB->GetCurrentBioseq();
    if (!bh)
    {
        event.Skip();
        return;
    }
    event.Enable(!x_HaveBiosourceFeat() && x_IsNa());
}

void CBioseqEditor::OnUpdateDeleteBiosourceFeat(wxUpdateUIEvent& event)
{   
    event.Enable(x_HaveBiosourceFeat());
}

void CBioseqEditor::OnEditBiosourceFeat(wxCommandEvent& event)
{
    const char* szCommand = "Edit Biosource Feature";

    try {
        CBioseq_Handle bh = m_CB->GetCurrentBioseq();
        if (!bh)
        {
            event.Skip();
            return;
        }

        CFeat_CI fsrc_it(bh, CSeqFeatData::e_Biosrc);
        if (!fsrc_it)
        {
            event.Skip();
            return;
        }

        CSeq_feat_Handle fh = *fsrc_it;
        CSeq_entry_Handle seh = bh.GetSeq_entry_Handle();

        CIRef<IEditObject> editor(new CEditObjectSeq_feat(*fh.GetOriginalSeq_feat(), seh, bh.GetScope(), false));
        ERR_POST(Info << "Start " << szCommand);
        x_EditObject(editor);
        ERR_POST(Info << "End " << szCommand);
    } catch (const std::exception& e) {
        x_ReportError(szCommand, e);
    }
}

void CBioseqEditor::OnPropagateAllFeatures(wxCommandEvent& event)
{
    //Context:
    // The user selected "Propagate All Features" from the edit link next to a
    // bioseq. The intent is to propagate every feature of that bioseq to one
    // or all other bioseqs in the set.
    // Where to and how to propagate still needs to be settled but the features to be
    // propagated are already fixed. It needs therefore be passed into all the actions
    // that will be triggered from here.
    //
    const char* szCommand = "Feature Propagate";

    try {
        TEditObjectList objList;
        m_CB->GetCurrentSelection(objList);
        if (objList.empty()) {
            event.Skip();
            return;
        }

        CBioseq_Handle bsh;
        for (TEditObjectList::iterator it = objList.begin(); it != objList.end(); ++it) {
            const CSeq_feat* pSeqFeat = dynamic_cast<const CSeq_feat*>(it->m_Obj.GetPointer());
            if (pSeqFeat) {
                if (it->m_Scope) {
                    bsh = it->m_Scope->GetBioseqHandle(pSeqFeat->GetLocation());
                }
                else {
                    bsh = it->m_SEH.GetScope().GetBioseqHandle(pSeqFeat->GetLocation());
                }
                if (bsh) {
                    break;
                }
            }
        }

        vector<CConstRef<CSeq_feat>> propagatedFeats;
        for (CFeat_CI ci(bsh); ci; ++ci) {
            propagatedFeats.push_back(ci->GetSeq_feat());
        }

        CIRef<IEditObject> editor(new CEditObjectFeaturePropagate(bsh, propagatedFeats));
        ERR_POST(Info << "Start " << szCommand);
        x_EditObject(editor, "Propagate Features");
        ERR_POST(Info << "End " << szCommand);
    }
    catch (const std::exception& e) {
        x_ReportError(szCommand, e);
    }

}

void CBioseqEditor::OnUpdatePropagateAllFeatures(wxUpdateUIEvent& event)
{
    event.Enable(x_HasAlignment());
}

void CBioseqEditor::OnPropagateSelectedFeatures(wxCommandEvent& event)
{
    //Context
    // The user highlighted one or more features of one of the bioseqs displayed. The
    // intent is to propagate all of the selected features to one or all of the other
    // sequences.
    // Where to and how to propagate still needs to be settled but the features to be
    // propagated are already fixed. It needs therefore be passed into all the actions
    // that will be triggered from here.
    //
    const char* szCommand = "Feature Propagate";

    try {
        //Generate list of features to be propagated. That would be the currently selected
        // ones
        TEditObjectList objList;
        m_CB->GetCurrentSelection(objList);
        if (objList.empty()) {
            event.Skip();
            return;
        }
        
        vector<CConstRef<CSeq_feat>> propagatedFeats;
        for (TEditObjectList::iterator it = objList.begin(); it != objList.end(); ++it) {
            const CSeq_feat* pSeqFeat = dynamic_cast<const CSeq_feat*>(it->m_Obj.GetPointer());
            if (pSeqFeat) {
                propagatedFeats.push_back(ConstRef(pSeqFeat));
            }
        }

        CSeq_entry_Handle seh = objList.front().m_SEH;
        CBioseq_Handle bh = seh.GetSeq();
        if (!bh) {
            event.Skip();
            return;
        }
        //Create and trigger the magic object editor that will take care of the 
        // rest 
        CEditObjectFeaturePropagate* pPropagator = new CEditObjectFeaturePropagate(bh, propagatedFeats);
        
        CIRef<IEditObject> editor(pPropagator);
        ERR_POST(Info << "Start " << szCommand);
        x_EditObject(editor, "Propagate Features");
        ERR_POST(Info << "End " << szCommand);

    } catch (const std::exception& e) {
        x_ReportError(szCommand, e);
    }
}

void CBioseqEditor::OnUpdatePropagateSelectedFeatures(wxUpdateUIEvent& event)
{
    event.Enable(x_HasAlignment());
}

void CBioseqEditor::OnCreateBiosourceFeat(wxCommandEvent& event)
{
    const char* szCommand = "Create Biosource Feature";

    try {
        CBioseq_Handle bh = m_CB->GetCurrentBioseq();
        if (!bh)
        {
            event.Skip();
            return;
        }

        CFeat_CI fsrc_it(bh, CSeqFeatData::e_Biosrc);
        if (fsrc_it) {
            ERR_POST(Info << "Error: " << szCommand << ": Biosource feature already exists");
            return;
        }

        CRef<CSeq_feat> seqFeat(new CSeq_feat());
        CRef<CSeqFeatData> featData(new CSeqFeatData());
        CRef<CBioSource> bioSource(new CBioSource());
        seqFeat->SetData(*featData);
        featData->SetBiosrc(*bioSource);

        CRef<CSeq_id> seq_id(new CSeq_id);
        CConstRef<CSeq_id> bseq_seq_id = bh.GetSeqId();
        TSeqPos length = bh.GetBioseqLength();
        seq_id->Assign(*bseq_seq_id);
        CRef<CSeq_loc> seqloc(new CSeq_loc(*seq_id, 0, length - 1));
        seqFeat->SetLocation(*seqloc);

        CSeq_entry_Handle seh = bh.GetSeq_entry_Handle();
        CIRef<IEditObject> editor(new CEditObjectSeq_feat(*seqFeat, seh, seh.GetScope(), true));
        ERR_POST(Info << "Start " << szCommand);
        x_EditObject(editor);
        ERR_POST(Info << "End " << szCommand);

    } catch (const std::exception& e) {
        x_ReportError(szCommand, e);
    }

}

void CBioseqEditor::OnDeleteBiosourceFeat(wxCommandEvent& event)
{
    const char* szCommand = "Delete Biosource Feature";

    try {
        CBioseq_Handle bh = m_CB->GetCurrentBioseq();
        if (!bh)
        {
            event.Skip();
            return;
        }

        CFeat_CI fsrc_it(bh, CSeqFeatData::e_Biosrc);
        if (!fsrc_it)
            return;

        CSeq_entry_Handle seh = bh.GetSeq_entry_Handle();
        CIRef<IEditCommand> command(new CCmdDelSeq_feat(fsrc_it->GetSeq_feat_Handle()));
        seh.GetEditHandle();
        m_CmdProccessor.Execute(command);
    } catch (const std::exception& e) {
        x_ReportError(szCommand, e);
    }
}



// feature creators
void CBioseqEditor::OnCreateFeature(CRef<CSeq_feat> feature, wxCommandEvent& event)
{
    string feat_name = "Create " + feature->GetData().GetKey();    
    const char* szCommand = feat_name.c_str();

    try {
        CBioseq_Handle bh = m_CB->GetCurrentBioseq();
        if (!bh)
        {
            event.Skip();
            return;
        }


        CRef<CSeq_id> seq_id(new CSeq_id);
        CConstRef<CSeq_id> bseq_seq_id = bh.GetSeqId();
        TSeqPos length = bh.GetBioseqLength();
        seq_id->Assign(*bseq_seq_id);
        CRef<CSeq_loc> seqloc(new CSeq_loc(*seq_id, 0, length - 1));
        feature->SetLocation(*seqloc);

        CSeq_entry_Handle seh = bh.GetSeq_entry_Handle();
        CIRef<IEditObject> editor(new CEditObjectSeq_feat(*feature, seh, seh.GetScope(), true));
        ERR_POST(Info << "Start " << szCommand);
        x_EditObject(editor);
        ERR_POST(Info << "End " << szCommand);

    } catch (const std::exception& e) {
        x_ReportError(szCommand, e);
    }
}


void CBioseqEditor::OnUpdateNucleotideFeat(wxUpdateUIEvent& event)
{
    CBioseq_Handle bh = m_CB->GetCurrentBioseq();
    if (!bh)
    {
        event.Skip();
        return;
    }
    event.Enable(!bh.IsAa());
}


void CBioseqEditor::OnUpdateProteinFeat(wxUpdateUIEvent& event)
{
    CBioseq_Handle bh = m_CB->GetCurrentBioseq();
    if (!bh)
    {
        event.Skip();
        return;
    }
    event.Enable(bh.IsAa());
}

void CBioseqEditor::OnUpdateAnyFeat(wxUpdateUIEvent& event)
{
    CBioseq_Handle bh = m_CB->GetCurrentBioseq();
    if (!bh)
    {
        event.Skip();
        return;
    }
    event.Enable(true);
}

bool CBioseqEditor::x_HandleFeatCreationHere(wxCommandEvent& event)
{
    if (!m_CB->MayCreateFeatures()) {
        event.Skip();
        return false;
    }
    return true;
}

void CBioseqEditor::OnCreateCodingRegion(wxCommandEvent& event)
{
    if (x_HandleFeatCreationHere(event)) {
        CRef<CSeq_feat> seqFeat(new CSeq_feat());
        seqFeat->SetData().SetCdregion();
        OnCreateFeature(seqFeat, event);
    }
}


void CBioseqEditor::OnCreateGene(wxCommandEvent& event)
{
    if (x_HandleFeatCreationHere(event)) {
        CRef<CSeq_feat> seqFeat(new CSeq_feat());
        seqFeat->SetData().SetGene();
        OnCreateFeature(seqFeat, event);
    }
}


void CBioseqEditor::OnCreateProt(wxCommandEvent& event)
{
    if (x_HandleFeatCreationHere(event)) {
        CRef<CSeq_feat> seqFeat(new CSeq_feat());
        seqFeat->SetData().SetProt();
        switch (event.GetId()) {
        case eCmdCreateProprotein:
            seqFeat->SetData().SetProt().SetProcessed(CProt_ref::eProcessed_preprotein);
            break;
        case eCmdCreateMatPeptide:
            seqFeat->SetData().SetProt().SetProcessed(CProt_ref::eProcessed_mature);
            break;
        case eCmdCreateSigPeptide:
            seqFeat->SetData().SetProt().SetProcessed(CProt_ref::eProcessed_signal_peptide);
            break;
        case eCmdCreateTransitPeptide:
            seqFeat->SetData().SetProt().SetProcessed(CProt_ref::eProcessed_transit_peptide);
            break;
        default:
            // do nothing, normal protein
            break;
        }
        OnCreateFeature(seqFeat, event);
    }
}


void CBioseqEditor::OnCreateImport(wxCommandEvent& event)
{
    if (x_HandleFeatCreationHere(event)) {
        CRef<CSeq_feat> seqFeat(new CSeq_feat());
        seqFeat->SetData().SetImp().SetKey(GetKeyFromCmdID(event.GetId()));
        OnCreateFeature(seqFeat, event);
    }
}


void CBioseqEditor::OnCreateRegion(wxCommandEvent& event)
{
    if (x_HandleFeatCreationHere(event)) {
        CRef<CSeq_feat> seqFeat(new CSeq_feat());
        seqFeat->SetData().SetRegion();
        OnCreateFeature(seqFeat, event);
    }
}


void CBioseqEditor::OnCreateSecondaryStructure(wxCommandEvent& event)
{
    if (x_HandleFeatCreationHere(event)) {
        CRef<CSeq_feat> seqFeat(new CSeq_feat());
        seqFeat->SetData().SetPsec_str(CSeqFeatData::ePsec_str_helix);
        OnCreateFeature(seqFeat, event);
    }
}


void CBioseqEditor::OnCreateSite(wxCommandEvent& event)
{
    if (x_HandleFeatCreationHere(event)) {
        CRef<CSeq_feat> seqFeat(new CSeq_feat());
        seqFeat->SetData().SetSite();
        OnCreateFeature(seqFeat, event);
    }
}


void CBioseqEditor::OnCreateBond(wxCommandEvent& event)
{
    if (x_HandleFeatCreationHere(event)) {
        CRef<CSeq_feat> seqFeat(new CSeq_feat());
        seqFeat->SetData().SetBond();
        OnCreateFeature(seqFeat, event);
    }
}


void CBioseqEditor::OnCreatePubFeat(wxCommandEvent& event)
{
    if (x_HandleFeatCreationHere(event)) {
        CRef<CSeq_feat> seqFeat(new CSeq_feat());
        seqFeat->SetData().SetPub();
        OnCreateFeature(seqFeat, event);
    }
}


void CBioseqEditor::OnCreateRNA(wxCommandEvent& event)
{
    if (x_HandleFeatCreationHere(event)) {
        CRef<CSeq_feat> seqFeat(new CSeq_feat());

        string rna_type = GetKeyFromCmdID(event.GetId());
        if (NStr::EqualNocase(rna_type, "preRNA")) {
            seqFeat->SetData().SetRna().SetType(CRNA_ref::eType_premsg);
        }
        else if (NStr::EqualNocase(rna_type, "mRNA")) {
            seqFeat->SetData().SetRna().SetType(CRNA_ref::eType_mRNA);
        }
        else if (NStr::EqualNocase(rna_type, "tRNA")) {
            seqFeat->SetData().SetRna().SetType(CRNA_ref::eType_tRNA);
        }
        else if (NStr::EqualNocase(rna_type, "rRNA")) {
            seqFeat->SetData().SetRna().SetType(CRNA_ref::eType_rRNA);
        }
        else if (NStr::EqualNocase(rna_type, "ncRNA")) {
            seqFeat->SetData().SetRna().SetType(CRNA_ref::eType_ncRNA);
        }
        else if (NStr::EqualNocase(rna_type, "tmRNA")) {
            seqFeat->SetData().SetRna().SetType(CRNA_ref::eType_tmRNA);
        }
        else {
            seqFeat->SetData().SetRna().SetType(CRNA_ref::eType_miscRNA);
        }

        OnCreateFeature(seqFeat, event);
    }
}


void CBioseqEditor::OnCreateDescriptor(wxCommandEvent& evt)
{
    string descriptor_type = GetKeyFromCmdID(evt.GetId());
    string cmd_name = "Create " + descriptor_type + " Descriptor";
    const char* szCommand = cmd_name.c_str();

    try {
        CBioseq_Handle bh = m_CB->GetCurrentBioseq();
        if (!bh)
        {
            evt.Skip();
            return;
        }

        // pre-checks
        if (NStr::EqualNocase(descriptor_type, "BioSource")) {
            CSeqdesc_CI desc_it(bh, CSeqdesc::e_Source);
            if (desc_it) {
                ERR_POST(Info << "Error: " << szCommand << ": Biosource descriptor already exists");
                return;
            }
        } else if (NStr::EqualNocase(descriptor_type, "Molecule Description")) {
            CSeqdesc_CI desc_it(bh, CSeqdesc::e_Molinfo);
            if (desc_it) {
                CSeq_entry_Handle seh = bh.GetSeq_entry_Handle();
                CIRef<IEditObject> editor(new CEditObjectSeq_desc(*desc_it, seh, bh.GetScope(), false));
                ERR_POST(Info << "Start " << szCommand);
                x_EditObject(editor);
                ERR_POST(Info << "End " << szCommand);
                return;
            }
        }

        CRef<CSeqdesc> seqdesc(new CSeqdesc());
        if (NStr::EqualNocase(descriptor_type, "BioSource")) {
            CRef<CBioSource> bioSource(new CBioSource());
            seqdesc->SetSource(*bioSource);
        } else if (NStr::EqualNocase(descriptor_type, "TPA Assembly")) {
            seqdesc->SetUser().SetType().SetStr("TpaAssembly");
        } else if (NStr::EqualNocase(descriptor_type, "StructuredComment")) {
            seqdesc->SetUser().SetType().SetStr("StructuredComment");
        } else if (NStr::EqualNocase(descriptor_type, "DBLink")) {
            seqdesc->SetUser().SetType().SetStr("DBLink");
        } else if (NStr::EqualNocase(descriptor_type, "Unverified")) {
            seqdesc->SetUser().SetType().SetStr("Unverified");
        } else if (NStr::EqualNocase(descriptor_type, "AuthorizedAccess")) {
            seqdesc->SetUser().SetType().SetStr("AuthorizedAccess");
        } else if (NStr::EqualNocase(descriptor_type, "RefGene Tracking")) {
            seqdesc->SetUser().SetType().SetStr("RefGeneTracking");
        } else if (NStr::EqualNocase(descriptor_type, "Region")) {
            seqdesc->SetRegion();
        } else if (NStr::EqualNocase(descriptor_type, "Name")) {
            seqdesc->SetName();
        } else if (NStr::EqualNocase(descriptor_type, "Comment")) {
            seqdesc->SetComment();
        } else if (NStr::EqualNocase(descriptor_type, "Title")) {
            seqdesc->SetTitle();
        } else if (NStr::EqualNocase(descriptor_type, "Molecule Description")) {
            seqdesc->SetMolinfo();
        } else if (NStr::EqualNocase(descriptor_type, "Publication")) {
            seqdesc->SetPub();
        }

        CSeq_entry_Handle seh = bh.GetSeq_entry_Handle();
        CIRef<IEditObject> editor(new CEditObjectSeq_desc(*seqdesc, seh, bh.GetScope(), true));
        ERR_POST(Info << "Start " << szCommand);
        x_EditObject(editor);
        ERR_POST(Info << "End " << szCommand);

    } catch (const std::exception& e) {
        x_ReportError(szCommand, e);
    }

}


CIRef<IEditObject> CreateEditorForObject(CConstRef<CObject> obj, CSeq_entry_Handle seh, bool create)
{
    CIRef<IEditObject> editor(NULL);

    const CSeq_feat* seqFeat = dynamic_cast<const CSeq_feat*>(obj.GetPointer());
    const CTitleEdit* titleEdit = dynamic_cast<const CTitleEdit*>(obj.GetPointer());
    const CSeqdesc* seqDesc = dynamic_cast<const CSeqdesc*>(obj.GetPointer());
    const CSubmit_block* submitBlock = dynamic_cast<const CSubmit_block*>(obj.GetPointer());
    const CBioseq_set* seq_set = dynamic_cast<const CBioseq_set*>(obj.GetPointer());
    const CSeq_entry* entry = dynamic_cast<const CSeq_entry*>(obj.GetPointer());
    const CSeq_submit* seq_submit = dynamic_cast<const CSeq_submit*>(obj.GetPointer());
    if (seq_submit && seq_submit->IsSetSub()) {
        submitBlock = &seq_submit->GetSub();
    }
    if (entry && entry->IsSet()) {
        seq_set = &(entry->GetSet());
    }

    if (seqFeat) {
        CSeq_entry_Handle feat_seh = seh;
        CBioseq_Handle bsh = GetBioseqForSeqFeat(*seqFeat, seh.GetScope());
        if (bsh) {
            feat_seh = bsh.GetSeq_entry_Handle();
        }
        if (create) {
            editor.Reset(new CEditObjectSeq_feat(*seqFeat, feat_seh, seh.GetScope(), create));
        } else {
            editor.Reset(new CEditObjectSeq_feat(*seqFeat, seh, seh.GetScope(), create));
        }
    }
    else if (seqDesc && CEditObjectSeq_desc::GetDescriptorType(*seqDesc).first != CEditObjectSeq_desc::eUnknown) {
        editor.Reset(new CEditObjectSeq_desc(*seqDesc, seh, seh.GetScope(), create));
    }
    else if (titleEdit) {
        CBioseq_Handle bh = seh.GetSeq();
        CConstRef<CObject> dummyDesc(new CSeqdescTitleEdit(bh, titleEdit->GetTitle()));
        editor.Reset(new CEditObjectSeq_desc(*dummyDesc, seh, seh.GetScope(), create));
    }
    else if (submitBlock) {
        // Create IEditObject object for Submit-block here
        CScope& scope = seh.GetScope();
        editor.Reset(new CEditObjectSubmit_block(*submitBlock, scope, create));
    }
    else if (seq_set) {
        // create IEditObject object for Bioseq-set
        CScope& scope = seh.GetScope();
        editor.Reset(new CEditObjectSet(*seq_set, scope));
    }

    const CSerialObject* serial =
        dynamic_cast<const CSerialObject*>(obj.GetPointer());
    string type_name =
        serial ? serial->GetThisTypeInfo()->GetName() : string("CObject");
   
    return editor;
}

int CBioseqEditor::x_GetFromPosition()
{
    int from = 0;
    /*  const set<ITextItem*> &selected_items = dynamic_cast<CTextPanelContext*>(m_CB)->GetSelectedTextItems();
    ITERATE (set<ITextItem*>, it, selected_items) {
        CFlatFileTextItem* ffItem = 0;
        CExpandItem* expandItem = dynamic_cast<CExpandItem*>(*it);
        
        if (expandItem)
            ffItem = dynamic_cast<CFlatFileTextItem*>(expandItem->GetExpandedItem());
        else
            ffItem = dynamic_cast<CFlatFileTextItem*>(*it);
        
        if (!ffItem)
            continue;
        
        const objects::IFlatItem* item = ffItem->GetFlatItem();
        const CSequenceItem* seqItem = dynamic_cast<const CSequenceItem*>(item);
        if (seqItem) {
            from = seqItem->GetFrom();
        }
    }
    */
    return from;
}

void CBioseqEditor::EditSelection()
{
    string command = "Edit Object: ";

    try {
        TEditObjectList objList;
        m_CB->GetCurrentSelection(objList);
        if (objList.empty())
            return;
        
        
        CEditObject& editObj = objList.front();
        if (!IsEditable(editObj))
            return;

        const CSeq_loc* loc = dynamic_cast<const CSeq_loc*>(editObj.m_Obj.GetPointer());
        const CSeq_align* align = dynamic_cast<const CSeq_align*>(editObj.m_Obj.GetPointer());
        const CSeq_submit* submit = dynamic_cast<const CSeq_submit*>(editObj.m_Obj.GetPointer());
        const CTitleEdit* titleEdit = dynamic_cast<const CTitleEdit*>(editObj.m_Obj.GetPointer());
        const CSeqdesc* seqDesc = dynamic_cast<const CSeqdesc*>(editObj.m_Obj.GetPointer());

        if (loc ) 
        {
            int from = 0;
            CRef<CSeq_loc> whole_loc;
            if (loc->IsInt())
            {
                from = loc->GetInt().GetFrom() + 1;
                CBioseq_Handle bsh = editObj.m_SEH.GetScope().GetBioseqHandle(*loc);
                whole_loc = bsh.GetRangeSeq_loc(0,0);
            }
            if (whole_loc)
                loc = whole_loc.GetPointer();
            
            if (loc->IsWhole())
            {
                wxApp *app = dynamic_cast<wxApp*>(wxApp::GetInstance());
                CEditSequence *dlg = new CEditSequence( app->GetTopWindow(), loc, editObj.m_SEH.GetScope(), &m_CmdProccessor, from); 
                dlg->Show(true);      
            }
            return;
        }
              
        if (align) {
            CAlignmentAssistant *dlg = NULL;
            try
            {
                wxApp *app = dynamic_cast<wxApp*>(wxApp::GetInstance());
                dlg = new CAlignmentAssistant(app->GetTopWindow(), editObj.m_SEH, &m_CmdProccessor, ConstRef(align));
                dlg->Show(true);
                return;
            }
            catch (CException&)
            {
                if (dlg)
                    dlg->Destroy();
                wxMessageBox(wxT("Unable to load the alignment"), wxT("Error"), wxOK | wxICON_ERROR);
                return;
            }
        }

        CConstRef<CObject> obj = editObj.m_Obj;

        if (submit && submit->IsSetSub())
        {
            CConstRef<CSubmit_block> bl(&(submit->GetSub()));
            obj.Reset(bl.GetPointer());
        }

        CSeq_entry_Handle seh = editObj.m_SEH;

        if (titleEdit)
        {
            CSeqdesc_CI desc_it(seh, CSeqdesc::e_Title);
            if (!desc_it)
            {
                int res = wxMessageBox(wxT("The current display is an on-the-fly defline, not an instantiated one. Editing this will create an instantiated title."),  wxT("Create Defline"), wxYES_NO | wxICON_QUESTION);
                if (res != wxYES)
                {          
                    return;
                }
            }
        }
 
        bool create = false;
        if (seqDesc && seqDesc->IsSource()) // if object does not exist but allows selection and editing - create it (can happen with biosource in flat file view)
        {
            CSeqdesc_CI desc_it(seh, CSeqdesc::e_Source);
            if (!desc_it)
            {
                create = true;
            }
        }

        CIRef<IEditObject> editor = CreateEditorForObject(obj, seh, create);
        
        const CSerialObject* serial = dynamic_cast<const CSerialObject*>(editObj.m_Obj.GetPointer());
        string type_name =   serial ? serial->GetThisTypeInfo()->GetName() : string("CObject");
        
        if (!editor) {
            wxMessageBox(wxT("No editor found for object of type \'") +
                         ToWxString(type_name) +
                         wxT("\'."),
                         wxT("Error"), wxOK|wxICON_ERROR);
            return;
        }
        
        command += type_name;
        
        ERR_POST(Info << "Start " << command << type_name);
        x_EditObject(editor);
        ERR_POST(Info << "End " << command<< type_name);
    
    } catch (const std::exception& e) {
        x_ReportError(command, e);
    }
}

void CBioseqEditor::OnEditSelection(wxCommandEvent& event)
{
    /*TFlatItemList itemList;
    m_CB->GetItemSelection(itemList);
    if (!itemList.empty())
    {
        cout << "Selected: " << typeid(*itemList.front()).name() << endl;
    }
    */
    try {
        TEditObjectList objList;
        m_CB->GetCurrentSelection(objList);
        if (objList.empty())
        {
            event.Skip();
            return;
        }       

        if (IsEditable(objList.front()))
       {
           EditSelection();
           return;
       }
    }
    catch (const std::exception&) {
    }
    event.Skip();
}

void CBioseqEditor::OnUpdateEditSelection(wxUpdateUIEvent& event)
{
    event.Enable(false);

    try {
        TEditObjectList objList;
        m_CB->GetCurrentSelection(objList);
        if (objList.empty())
        {
            // event.Skip();
            return;
        }

        event.Enable(IsEditable(objList.front()));
    }
    catch (const std::exception&) {
    }
}

void CBioseqEditor::OnDeleteSelection(wxCommandEvent& event)
{
    int num_selected = 0;
    int num_cds = 0;
    try {
        TEditObjectList objList;
        m_CB->GetCurrentSelection(objList);
        if (objList.empty())
        {
            event.Skip();
            return;
        }

        ITERATE(TEditObjectList, editObj, objList) {

            const CSeqdesc* seqDesq = dynamic_cast<const CSeqdesc*>(editObj->m_Obj.GetPointer());
            const CSeq_feat* seqFeat = dynamic_cast<const CSeq_feat*>(editObj->m_Obj.GetPointer());
            const CSeq_annot* seqAnnot = dynamic_cast<const CSeq_annot*>(editObj->m_Obj.GetPointer());
            const CSeq_align* seqAlign = dynamic_cast<const CSeq_align*>(editObj->m_Obj.GetPointer());

            if (seqDesq || seqFeat || seqAnnot || seqAlign)
                num_selected++;
            if (seqFeat && seqFeat->IsSetData() 
                && seqFeat->GetData().GetSubtype() == CSeqFeatData::eSubtype_cdregion
                && sequence::GetGeneForFeature(*seqFeat, editObj->m_SEH.GetScope()))
                num_cds++;
        }
    } catch (const std::exception&) {
    }
    if (num_selected == 0)
    {
        event.Skip();
        return;
    }
    if (num_selected > 1)
    {
        int res = wxMessageBox(wxT("Are you sure you want to delete multiple objects?"),  wxT("Delete Selected Objects"), wxYES_NO | wxICON_QUESTION);
        if (res != wxYES)
        {          
            return;
        }
    }

    bool delete_genes = false;
    if (num_cds > 0)
    {
        int res = wxMessageBox(wxT("Remove associated genes?"),  wxT("Delete genes"), wxYES_NO | wxCANCEL | wxICON_QUESTION);
        if (res == wxCANCEL)
        {          
            return;
        }
        if (res == wxYES)
        {
            delete_genes = true;
        }
    }

    const char* szCommand = "Delete Object";

    try {
        TEditObjectList objList;
        m_CB->GetCurrentSelection(objList);
        if (objList.empty()) {
            return;
        }
        // warn before deleting biosource descriptor
        ITERATE(TEditObjectList, obj, objList) {
            const CSeqdesc* seqDesq = dynamic_cast<const CSeqdesc*>(obj->m_Obj.GetPointer());
            if (seqDesq) {
                if (seqDesq->IsSource()) {
                    wxMessageDialog dlg(NULL, wxT("Are you sure you want to delete the BioSource?"), wxT("Delete BioSource"), wxOK | wxCANCEL);
                    if (dlg.ShowModal() == wxID_CANCEL) {
                        return;
                    } else {
                        break;
                    }
                }
            }
        }
        // ask about deleting descriptors from sets
        bool from_single = false;
        ITERATE(TEditObjectList, obj, objList) {
            CSeq_entry_Handle seh = obj->m_SEH;
            const CSeqdesc* seqDesq = dynamic_cast<const CSeqdesc*>(obj->m_Obj.GetPointer());
            if (seqDesq) {
                CSeq_entry_Handle sehDesc;
                bool cancel;
                bool any;
                from_single = x_DeleteFromSingleSeq(seh, sehDesc, *seqDesq, any, cancel);
                if (cancel) {
                    return;
                }
                if (any) {
                    break;
                }
            }
        }

        // now delete all selected objects
        CRef<CCmdComposite> delete_cmd(new CCmdComposite("Delete object(s)"));
        bool any_deleted = false;
        ITERATE(TEditObjectList, editObj, objList) {
            CSeq_entry_Handle seh = editObj->m_SEH;
            const CSeqdesc* seqDesq = dynamic_cast<const CSeqdesc*>(editObj->m_Obj.GetPointer());
            const CSeq_feat* seqFeat = dynamic_cast<const CSeq_feat*>(editObj->m_Obj.GetPointer());
            const CSeq_annot* seqAnnot = dynamic_cast<const CSeq_annot*>(editObj->m_Obj.GetPointer());
            const CSeq_align* seqAlign = dynamic_cast<const CSeq_align*>(editObj->m_Obj.GetPointer());
            if (seqDesq) {
                CIRef<IEditCommand> this_cmd = x_GetDeleteCommand(seh, *seqDesq, from_single);
                if (!this_cmd) {
                    return;
                }
                delete_cmd->AddCommand(*this_cmd);
                any_deleted = true;
            } else if (seqFeat) {
                CRef<CCmdComposite> this_cmd = x_GetDeleteCommand(seh, *seqFeat);
                if (!this_cmd) {
                    return;
                }
                delete_cmd->AddCommand(*this_cmd);
                if (delete_genes && seqFeat->GetData().GetSubtype() == CSeqFeatData::eSubtype_cdregion)
                {
                    CConstRef<CSeq_feat> gene = sequence::GetGeneForFeature(*seqFeat, editObj->m_SEH.GetScope());
                    if (gene)
                    {
                        CRef<CCmdComposite> gene_cmd = x_GetDeleteCommand(seh, *gene);
                        if (gene_cmd)
                            delete_cmd->AddCommand(*gene_cmd);
                    }
                }

                any_deleted = true;
            }
            else if (seqAnnot) {
                CRef<CCmdComposite> this_cmd = x_GetDeleteCommand(seh, *seqAnnot);
                if (!this_cmd) {
                    return;
                }
                delete_cmd->AddCommand(*this_cmd);
                any_deleted = true;
            }
            else if (seqAlign) {
                CIRef<IEditCommand> this_cmd = x_GetDeleteCommand(seh, *seqAlign);
                if (!this_cmd) {
                    return;
                }
                delete_cmd->AddCommand(*this_cmd);
                any_deleted = true;
            }
        }
        if (any_deleted) {
            m_CmdProccessor.Execute(delete_cmd);
        }
    } catch (const std::exception& e) {
        x_ReportError(szCommand, e);
    }
}

void CBioseqEditor::OnUpdateDeleteSelection(wxUpdateUIEvent& event)
{
    event.Enable(false);

    try {
        TEditObjectList objList;
        m_CB->GetCurrentSelection(objList);
        if (objList.empty())
        {
            // event.Skip();
            return;
        }
        event.Enable(IsDeletable(objList.front()));
    } catch (const std::exception&) {
    }
}

void CBioseqEditor::x_EditObject( CIRef<IEditObject> edit, const string& title)
{
    wxBusyCursor wait;
    CEditObjViewDlg* edit_dlg = new CEditObjViewDlg(NULL, true);
    edit_dlg->SetUndoManager(&m_CmdProccessor);
    wxWindow* editorWindow = edit->CreateWindow(edit_dlg);
    edit_dlg->SetEditorWindow(editorWindow);
    edit_dlg->SetEditor(edit);
    SetWorkDir(edit_dlg, m_CB->GetWorkDir());

    if (!title.empty()) {
        edit_dlg->SetTitle(title);
    }
}


// if a descriptor is on a set, have the option to remove from just a single sequence in the set
bool CBioseqEditor::x_DeleteFromSingleSeq(objects::CSeq_entry_Handle& seh, objects::CSeq_entry_Handle& sehDesc, const objects::CSeqdesc& seqDesc, bool& any, bool& cancel)
{
    cancel = false;
    any = false;
    CSeqdesc::E_Choice choice = seqDesc.Which();

    sehDesc = edit::GetSeqEntryForSeqdesc(CRef<CScope>(&(seh.GetScope())), seqDesc);

    if (!sehDesc)
        return false;

    bool askSeqSet = false;

    switch (choice) {
    case CSeqdesc::e_Name:
    case CSeqdesc::e_Title:
    case CSeqdesc::e_Comment:
    case CSeqdesc::e_Genbank:
    case CSeqdesc::e_Region:
    case CSeqdesc::e_User:
    case CSeqdesc::e_Embl:
    case CSeqdesc::e_Pub:
    case CSeqdesc::e_Source:
        askSeqSet = (seh.IsSeq() && sehDesc != seh);
        break;
    default:
        break;
    }

    if (askSeqSet) {
        any = true;
        CDeleteFromBioseq_setDlg dlg(NULL);
        if (dlg.ShowModal() != wxID_OK) {
            cancel = true;
            return false;
        }

        if (dlg.GetChoice() == 0) {
            return false;
        } else {
            return true;
        }
    }
    return false;
}


CIRef<IEditCommand> CBioseqEditor::x_GetDeleteCommand(objects::CSeq_entry_Handle& seh, const objects::CSeqdesc& seqDesc, bool from_single)
{
    CSeq_entry_Handle sehDesc = edit::GetSeqEntryForSeqdesc(CRef<CScope>(&(seh.GetScope())), seqDesc);
    if (!sehDesc) {
        return CIRef<IEditCommand>(NULL);
    }

    CIRef<IEditCommand> cmd;
    if (from_single) {
        CRef<CCmdComposite> composite(new CCmdComposite("Delete Descriptor(s)"));
        CIRef<IEditCommand> cmdDelDesc(new CCmdDelDesc(sehDesc, seqDesc));
        composite->AddCommand(*cmdDelDesc);

        for (CSeq_entry_CI it(sehDesc.GetSet()); it; ++it) {
            if (*it != seh) {
                CIRef<IEditCommand> cmdAddDesc(new CCmdCreateDesc(*it, seqDesc));
                composite->AddCommand(*cmdAddDesc);
            }
        }
        cmd.Reset(composite.GetPointer());
    } else {
        cmd.Reset(new CCmdDelDesc(sehDesc, seqDesc));
    }
    return cmd;
}

void CBioseqEditor::x_DeleteObject(objects::CSeq_entry_Handle& seh, const objects::CSeqdesc& seqDesq)
{
    bool cancel = false;
    bool any = false;
    CSeq_entry_Handle sehDesc;
    bool from_single = x_DeleteFromSingleSeq(seh, sehDesc, seqDesq, any, cancel);
    if (cancel || !sehDesc) {
        return;
    }

    CIRef<IEditCommand> cmd;
    if (from_single) {
        CRef<CCmdComposite> composite(new CCmdComposite("Delete Descriptor(s)"));
        CIRef<IEditCommand> cmdDelDesc(new CCmdDelDesc(sehDesc, seqDesq));
        composite->AddCommand(*cmdDelDesc);

        for (CSeq_entry_CI it(sehDesc.GetSet()); it; ++it) {
            if (*it != seh) {
                CIRef<IEditCommand> cmdAddDesc(new CCmdCreateDesc(*it, seqDesq));
                composite->AddCommand(*cmdAddDesc);
            }
        }
        cmd.Reset(composite.GetPointer());
    } else {
        cmd.Reset(new CCmdDelDesc(sehDesc, seqDesq));
    }

    m_CmdProccessor.Execute(cmd);
}


CRef<CCmdComposite> CBioseqEditor::x_GetDeleteCommand(objects::CSeq_entry_Handle& seh, const objects::CSeq_feat& seqFeat)
{
    CRef<CCmdComposite> cmd(NULL);

    if (!seqFeat.IsSetData()) {
        LOG_POST(Error << "CBioseqEditor.Delete.Feature data not set");
        return cmd;
    }
    CSeq_feat_Handle fh;
    SAnnotSelector sel(seqFeat.GetData().GetSubtype());
    if (seqFeat.IsSetLocation() && !seqFeat.GetLocation().IsNull()) {
        for (CFeat_CI mf(seh.GetScope(), seqFeat.GetLocation(), sel); mf; ++mf) {
            if (mf->GetOriginalFeature().Equals(seqFeat)) {
                fh = mf->GetSeq_feat_Handle();
                break;
            }
        }
    }

    if (!fh) {
        fh = GetSeqFeatHandleForBadLocFeature(seqFeat, seh.GetScope());
    }

    if (!fh) {
        LOG_POST(Error << "CBioseqEditor.Delete.Feature Feature not found");
        return cmd;
    }

    CIRef<IEditCommand> addCommand;

    const CSeqFeatData& data = seqFeat.GetData();
    if (data.IsGene()) {
        addCommand = CCmdFactory::DeleteGeneXRefs(seh, data.GetGene());
        if (addCommand) {
            int res = wxMessageBox(wxT("Delete features which reference this gene?"),
                wxT("Delete Gene"), wxYES_NO | wxICON_QUESTION);
            if (res != wxYES)
                addCommand.Reset();
        }
    }

    if (addCommand) {
        CRef<CCmdComposite> composite(new CCmdComposite("Delete Feature"));
        composite->AddCommand(*addCommand);
        composite->AddCommand(*GetDeleteFeatureCommand(fh));
        cmd.Reset(&*composite);
    } else {
        cmd.Reset(&*GetDeleteFeatureCommand(fh));
    }

    // when the feature has a local feat-id which is cross-referenced by other features:
    if (seqFeat.IsSetId() && seqFeat.GetId().IsLocal()) {
        const CFeat_id::TLocal& local_id = seqFeat.GetId().GetLocal();
        CCrossRefFeatsDlg::TFeatVec feat_list;

        for (CFeat_CI feat_it(seh); feat_it; ++feat_it) {
            const CSeq_feat& orig = feat_it->GetOriginalFeature();
            FOR_EACH_SEQFEATXREF_ON_SEQFEAT(it, orig) {
                if ((*it)->IsSetId() && (*it)->GetId().IsLocal()) {
                    if (local_id.Match((*it)->GetId().GetLocal())) {
                        feat_list.push_back(seh.GetScope().GetSeq_featHandle(orig));
                        break;
                    }
                }
            }
        }
        if (!feat_list.empty()) {
            CCrossRefFeatsDlg dlg(NULL, feat_list, local_id);
            if (dlg.ShowModal() == wxID_OK) {
                CRef<CCmdComposite> chgfeats = dlg.GetCommand();
                if (chgfeats) {
                    cmd->AddCommand(*chgfeats);
                }
            }
        }

    }

    if (!cmd) {
        LOG_POST(Error << "CBioseqEditor.Delete.Feature failed to create command");
        return cmd;
    }

    // This is necessary, to make sure that we are in "editing mode"
    const CSeq_annot_Handle& annot_handle = fh.GetAnnot();
    CSeq_entry_EditHandle eh = annot_handle.GetParentEntry().GetEditHandle();

    return cmd;
}


void CBioseqEditor::x_DeleteObject(objects::CSeq_entry_Handle& seh, const objects::CSeq_feat& seqFeat)
{
    if (!seqFeat.IsSetData()) {
        LOG_POST(Error << "CBioseqEditor.Delete.Feature data not set");
        return;
    }

    CSeq_feat_Handle fh;
    SAnnotSelector sel(seqFeat.GetData().GetSubtype());
    if (seqFeat.IsSetLocation() && !seqFeat.GetLocation().IsNull()) {
        for (CFeat_CI mf(seh.GetScope(), seqFeat.GetLocation(), sel); mf; ++mf) {
            if (mf->GetOriginalFeature().Equals(seqFeat)) {
                fh = mf->GetSeq_feat_Handle();
                break;
            }
        }
    } else {
        fh = GetSeqFeatHandleForBadLocFeature(seqFeat, seh.GetScope());
    }

    if (!fh) {
        LOG_POST(Error << "CBioseqEditor.Delete.Feature Feature not found");
        return;
    }

    CIRef<IEditCommand> addCommand;
    CRef<CCmdComposite> cmd;

    const CSeqFeatData& data = seqFeat.GetData();
    if (data.IsGene()) {
        addCommand = CCmdFactory::DeleteGeneXRefs(seh, data.GetGene());
        if (addCommand) {
            int res = wxMessageBox(wxT("Delete features which reference this gene?"),
                                   wxT("Delete Gene"), wxYES_NO | wxICON_QUESTION );
            if (res != wxYES)
                addCommand.Reset();
        }
    }

    if (addCommand) {
        CRef<CCmdComposite> composite(new CCmdComposite("Delete Feature"));
        composite->AddCommand(*addCommand);
        composite->AddCommand(*GetDeleteFeatureCommand(fh));
        cmd.Reset(&*composite);
    }
    else {
        cmd.Reset(&*GetDeleteFeatureCommand(fh));
    }

    // when the feature has a local feat-id which is cross-referenced by other features:
    if (seqFeat.IsSetId() && seqFeat.GetId().IsLocal()) {
        const CFeat_id::TLocal& local_id = seqFeat.GetId().GetLocal();
        CCrossRefFeatsDlg::TFeatVec feat_list;

        for(CFeat_CI feat_it(seh); feat_it; ++feat_it) {
            const CSeq_feat& orig = feat_it->GetOriginalFeature();
            FOR_EACH_SEQFEATXREF_ON_SEQFEAT (it, orig) {
                if ((*it)->IsSetId() && (*it)->GetId().IsLocal()) {
                    if (local_id.Match((*it)->GetId().GetLocal())) {
                        feat_list.push_back(seh.GetScope().GetSeq_featHandle(orig));
                        break;
                    }
                }
            }
        }
        if (!feat_list.empty()) {
            CCrossRefFeatsDlg dlg(NULL, feat_list, local_id);
            if (dlg.ShowModal() == wxID_OK) {
                CRef<CCmdComposite> chgfeats = dlg.GetCommand();
                if (chgfeats) {
                    cmd->AddCommand(*chgfeats);
                }
            }
        }

    }

    if (!cmd) {
        LOG_POST(Error << "CBioseqEditor.Delete.Feature failed to create command");
        return;
    }

    // This is necessary, to make sure that we are in "editing mode"
    const CSeq_annot_Handle& annot_handle = fh.GetAnnot();
    CSeq_entry_EditHandle eh = annot_handle.GetParentEntry().GetEditHandle();
    m_CmdProccessor.Execute(cmd);
}

CRef<CCmdComposite> CBioseqEditor::x_GetDeleteCommand(CSeq_entry_Handle& seh, const CSeq_annot& annot)
{
    CSeq_annot_Handle ah;
    for (CSeq_annot_CI annot_it(seh, CSeq_annot_CI::eSearch_entry); annot_it; ++annot_it) {
        if (annot_it->GetCompleteSeq_annot().GetPointer() == &annot) {
            ah = *annot_it;
        }
    }

    if (!ah) {
        LOG_POST(Error << "CBioseqEditor.Delete.Annot Annot not found");
        return CRef<CCmdComposite>();
    }

    CRef<CCmdComposite> cmd(new CCmdComposite("Delete annotation"));
    if (!annot.IsFtable()) {
        CIRef<IEditCommand> cmdDelAnnot(new CCmdDelSeq_annot(ah));
        cmd->AddCommand(*cmdDelAnnot);
    }
    else {
        for (CFeat_CI feat_it(ah); feat_it; ++feat_it) {
            CRef<CCmdComposite> cmdDelFeat = GetDeleteFeatureCommand(feat_it->GetSeq_feat_Handle(), true);
            cmd->AddCommand(*cmdDelFeat);
        }
    }

    if (!cmd) {
        LOG_POST(Error << "CBioseqEditor.Delete.Annot failed to create command");
        return cmd;
    }

    // This is necessary, to make sure that we are in "editing mode"
    CSeq_entry_EditHandle eh = ah.GetParentEntry().GetEditHandle();

    return cmd;
}

CIRef<IEditCommand> CBioseqEditor::x_GetDeleteCommand(CSeq_entry_Handle& seh, const CSeq_align& align)
{
    CSeq_align_Handle ah;
    for (CAlign_CI align_it(seh); align_it; ++align_it) {
        if (&align_it.GetOriginalSeq_align() == &align) {
            ah = align_it.GetSeq_align_Handle();
        }
    }

    if (!ah) {
        LOG_POST(Error << "CBioseqEditor.Delete.Align Align not found");
        return CIRef<IEditCommand>();
    }

    CIRef<IEditCommand> cmd(new CCmdDelSeq_align(ah));
    if (!cmd) {
        LOG_POST(Error << "CBioseqEditor.Delete.Align failed to create command");
        return cmd;
    }

    // This is necessary, to make sure that we are in "editing mode"
    const CSeq_annot_Handle& annot_handle = ah.GetAnnot();
    CSeq_entry_EditHandle eh = annot_handle.GetParentEntry().GetEditHandle();

    return cmd;
}

bool CBioseqEditor::x_HaveBiosourceDesc()
{
    try {
        CBioseq_Handle bh = m_CB->GetCurrentBioseq();
        if (!bh)
            return false;

        CSeqdesc_CI desc_it(bh, CSeqdesc::e_Source);
        if (desc_it) {
            return true;
        }
    }
    catch (const std::exception&) {
    }

    return false;
}

bool  CBioseqEditor::x_IsNa()
{
    try {
        CBioseq_Handle bh = m_CB->GetCurrentBioseq();
        if (!bh)
            return false;

        return !bh.IsAa();
    }
    catch (const std::exception&) {
    }

    return false;
}

bool CBioseqEditor::x_HaveBiosourceFeat()
{
    try {
        CBioseq_Handle bh = m_CB->GetCurrentBioseq();
        if (!bh)
            return false;

        CFeat_CI fsrc_it(bh, CSeqFeatData::e_Biosrc);
        if (fsrc_it) {
            return true;
        }
    }
    catch (const std::exception&) {
    }

    return false;
}

bool CBioseqEditor::x_HasAlignment()
{
    TEditObjectList objList;
    m_CB->GetCurrentSelection(objList);
    if (objList.empty()) {
        return false;
    }

    CBioseq_Handle bsh;
    for (auto&& it : objList) {
        const CSeq_feat* pSeqFeat = dynamic_cast<const CSeq_feat*>(it.m_Obj.GetPointer());
        if (pSeqFeat) {
            if (it.m_Scope) {
                bsh = it.m_Scope->GetBioseqHandle(pSeqFeat->GetLocation());
            }
            else {
                try {
                    bsh = it.m_SEH.GetScope().GetBioseqHandle(pSeqFeat->GetLocation());
                } catch (CException&) {
                    // may not be possible to get Bioseq from feature if multiple sequences
                    return false;
                }
            }
            if (bsh) {
                break;
            }
        }
        else {
            return false;
        }
    }
    if (!bsh) {
        return false;
    }
    CRef<CSeq_loc> loc = bsh.GetRangeSeq_loc(0, 0);
    return (CAlign_CI(bsh.GetScope(), *loc));
}

void CBioseqEditor::x_ReportError(const string& cmd, const std::exception& e)
{
    const CException* pex = dynamic_cast<const CException*>(&e);
    if (pex) {
        ERR_POST(Error << cmd << " Failed: " << pex->GetMsg());
    }
    else {
        ERR_POST(Error << cmd << " Failed: " << e.what());
    }
}


static bool s_IsLocalFeature(const CSeq_feat& seq_feat, CSeq_entry_Handle ctx)
{
    if (!ctx) {
        return false;
    }
    CScope& scope = ctx.GetScope();
    // check to see if this is from external annotation
    try {
        CSeq_feat_Handle feat = scope.GetSeq_featHandle(seq_feat);
        const CSeq_annot_Handle& ah = feat.GetAnnot();
        CSeq_entry_Handle seh = ah.GetParentEntry();
        if (!seh) {
            // from external annotation
            return false;
        } else if (seh.GetTopLevelEntry() != ctx.GetTopLevelEntry()) {
            return false;
        } else {
            return true;
        }
    } catch (const CException&) {
        return false;
    }
}


static CBioseq_Handle s_GetContext(const IFlatItem& item)
{
    CBioseq_Handle none;
    const CFlatItem* flat = dynamic_cast<const CFlatItem*>(&item);
    if (flat) {
        CBioseqContext* ctx = flat->GetContext();
        if (ctx) {
            return ctx->GetHandle();
        }
    }
    return none;
}

int CBioseqEditor::GetEditFlags(CEditObject& editObj) const
{
    int flags = 0;
    if (IsEditable(editObj))
        flags |= kObjectEditable;
    if (IsDeletable(editObj))
        flags |= kObjectDeletable;
    return flags;
}

int CBioseqEditor::GetEditFlags(const objects::IFlatItem& item) const
{
    int flags = 0;
    if (IsEditable(item))
        flags |= kObjectEditable;
    if (IsDeletable(item))
        flags |= kObjectDeletable;
    return flags;
}

bool CBioseqEditor::IsDeletable(const IFlatItem& item) const
{
    const CKeywordsItem* keyword_item = dynamic_cast<const  CKeywordsItem*>(&item);
    if (keyword_item)
    {
        return false;
    }

    const CAccessionItem* accession_item = dynamic_cast<const  CAccessionItem*>(&item);
    if (accession_item)
    {
        return false;
    }

    const CVersionItem* version_item = dynamic_cast<const  CVersionItem*>(&item);
    if (version_item)
    {
        return false;
    }

    const CFeatHeaderItem* featheader_item = dynamic_cast<const  CFeatHeaderItem*>(&item);
    if (featheader_item)
    {
        return false;
    }

    const COriginItem* origin_item = dynamic_cast<const  COriginItem*>(&item);
    if (origin_item)
    {
        return false;
    }

    const CStartSectionItem* startsection_item = dynamic_cast<const  CStartSectionItem*>(&item);
    if (startsection_item)
    {
        return false;
    }

    const CEndSectionItem* endsection_item = dynamic_cast<const  CEndSectionItem*>(&item);
    if (endsection_item)
    {
        return false;
    }

    const CContigItem* contig_item = dynamic_cast<const  CContigItem*>(&item);
    if (contig_item)
    {
        return false;
    }

    const CStartItem* start_item = dynamic_cast<const  CStartItem*>(&item);
    if (start_item)
    {
        return false;
    }

    const CEndItem* end_item = dynamic_cast<const  CEndItem*>(&item);
    if (end_item)
    {
        return false;
    }

    const CBaseCountItem* basecount_item = dynamic_cast<const  CBaseCountItem*>(&item);
    if (basecount_item)
    {
        return false;
    }


    const CSerialObject* obj = item.GetObject();
    const CSeqdesc* seqdesc = dynamic_cast<const CSeqdesc*>(obj);
    if (seqdesc) {
        return true;
    }
    
    const CSeq_feat* seq_feat = dynamic_cast<const CSeq_feat*>(obj);
    if (seq_feat) {
        CBioseq_Handle bsh = s_GetContext(item);
        if (bsh) {
            return s_IsLocalFeature(*seq_feat, bsh.GetSeq_entry_Handle());
        } else {
            return false;
        }
    }

    const CSubmit_block* submit_block = dynamic_cast<const CSubmit_block*>(obj);
    if (submit_block) {
        return false;
    }

    const CDeflineItem* defLineItem = dynamic_cast<const CDeflineItem*>(&item);
    if (defLineItem) {
        CBioseqContext* ctx = defLineItem->GetContext();
        if (ctx) {
            CBioseq_Handle& bh = ctx->GetHandle();
            CSeqdesc_CI di(bh, CSeqdesc::e_Title);
            if (di) {
                return true;
            }
        }
        return false;
    }
    const CLocusItem* locusItem = dynamic_cast<const CLocusItem*>(&item);
    if (locusItem) {
        CBioseqContext* ctx = locusItem->GetContext();
        if (ctx) {
            CBioseq_Handle& bh = ctx->GetHandle();

            const CMolInfo* molInfo = dynamic_cast<const CMolInfo*>(obj);
            if (molInfo) {
                CSeqdesc_CI dsrc_it(ctx->GetHandle(), CSeqdesc::e_Molinfo);
                for (; dsrc_it; ++dsrc_it) {
                    const CSeqdesc& seqDesc = *dsrc_it;
                    if (&seqDesc.GetMolinfo() == molInfo) {
                        return true;
                    }
                }
            }
        }
        return false;
    }
    return false;
}

bool CBioseqEditor::IsEditable(const objects::IFlatItem& item) const
{
    const CSerialObject* obj = item.GetObject();
    const CSeqdesc* seqdesc = dynamic_cast<const CSeqdesc*>(obj);
    if (seqdesc) {
        return CEditObjectSeq_desc::GetDescriptorType(*seqdesc).first != CEditObjectSeq_desc::eUnknown;;
    }
    
    const CSeq_feat* seq_feat = dynamic_cast<const CSeq_feat*>(obj);
    if (seq_feat) {
        CBioseq_Handle bsh = s_GetContext(item);
        if (bsh) {
            return s_IsLocalFeature(*seq_feat, bsh.GetSeq_entry_Handle());
        }
    }

    const CDeflineItem* defLineItem = dynamic_cast<const CDeflineItem*>(&item);
    if (defLineItem) {
        CBioseqContext* ctx = defLineItem->GetContext();
        if (ctx) {
            CBioseq_Handle& bh = ctx->GetHandle();
            CSeqdesc_CI di(bh, CSeqdesc::e_Title);
            if (di) {
                return true;
            }
        }
    }

    const CLocusItem* locusItem = dynamic_cast<const CLocusItem*>(&item);
    if (locusItem) {
        CBioseqContext* ctx = locusItem->GetContext();
        if (ctx) {
            CBioseq_Handle& bh = ctx->GetHandle();

            const CMolInfo* molInfo = dynamic_cast<const CMolInfo*>(obj);
            if (molInfo) {
                CSeqdesc_CI dsrc_it(ctx->GetHandle(), CSeqdesc::e_Molinfo);
                for (; dsrc_it; ++dsrc_it) {
                    const CSeqdesc& seqDesc = *dsrc_it;
                    if (&seqDesc.GetMolinfo() == molInfo) {
                        return true;
                    }
                }
            }
        }
    }


    const CSequenceItem* seqItem = dynamic_cast<const CSequenceItem*>(&item);
    if (seqItem) {
        CBioseqContext* ctx = seqItem->GetContext();
        if (ctx) {
            return true;
        } else {
            return false;
        }
    }

    const CSubmit_block* submit_block = dynamic_cast<const CSubmit_block*>(obj);
    if (submit_block) {
        return true;
    }
    const CGapItem* gap_item = dynamic_cast<const CGapItem*>(&item);
    if (gap_item)
    {
        return true;
    }
    return false;
}

bool CBioseqEditor::IsDeletable(CEditObject& editObj) const
{
    const CSeq_feat* seq_feat = dynamic_cast<const CSeq_feat*>(editObj.m_Obj.GetPointer());
    if (seq_feat) {
        return s_IsLocalFeature(*seq_feat, editObj.m_SEH);
    }

    const CSeqdesc* seqdesc = dynamic_cast<const CSeqdesc*>(editObj.m_Obj.GetPointer());
    if (seqdesc) {
        return true;
    }

    return false;
}


bool CBioseqEditor::IsEditable(CEditObject& editObj) const
{
    const CSeq_feat* seq_feat = dynamic_cast<const CSeq_feat*>(editObj.m_Obj.GetPointer());
    if (seq_feat) {
        return s_IsLocalFeature(*seq_feat, editObj.m_SEH);
    }
    
    const CSeqdesc* seqdesc = dynamic_cast<const CSeqdesc*>(editObj.m_Obj.GetPointer());
    if (seqdesc) {
        return CEditObjectSeq_desc::GetDescriptorType(*seqdesc).first != CEditObjectSeq_desc::eUnknown;
    }

    const CSeq_loc* loc = dynamic_cast<const CSeq_loc*>(editObj.m_Obj.GetPointer());
    if (loc) {
        // only allow sequence editing if this is near and a single seqid
        const CSeq_id* id = loc->GetId();
        if (id) {
            return true;
        }
        return false;
    }
    const CSubmit_block* submit_block = dynamic_cast<const CSubmit_block*>(editObj.m_Obj.GetPointer());
    const CTitleEdit* title_edit = dynamic_cast<const CTitleEdit*>(editObj.m_Obj.GetPointer());
    const CSeq_align* align = dynamic_cast<const CSeq_align*>(editObj.m_Obj.GetPointer());
    const CSeq_submit* submit = dynamic_cast<const CSeq_submit*>(editObj.m_Obj.GetPointer());
    const CBioseq_set* seq_set = dynamic_cast<const CBioseq_set*>(editObj.m_Obj.GetPointer());
    const CSeq_entry* entry = dynamic_cast<const CSeq_entry*>(editObj.m_Obj.GetPointer());
    if (entry && entry->IsSet())
        seq_set = &(entry->GetSet());

    if (submit_block || seq_set || title_edit || align || submit) {
        return true;
    }

    return false;
}


END_NCBI_SCOPE
