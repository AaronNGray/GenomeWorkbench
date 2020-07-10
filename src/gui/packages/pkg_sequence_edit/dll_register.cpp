/*  $Id: dll_register.cpp 45101 2020-05-29 20:53:24Z asztalos $
 * =========================================================================
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
 *  and reliability of the software and data, the NLM and the U.S.
 *  Government do not and cannot warrant the performance or results that
 *  may be obtained by using this software or data. The NLM and the U.S.
 *  Government disclaim all warranties, express or implied, including
 *  warranties of performance, merchantability or fitness for any particular
 *  purpose.
 *
 *  Please cite the author in any work or product based on this material.
 *
 * =========================================================================
 *
 * Authors:  Andrey Yazhuk, Igor Filippov, Jie Chen
 *
 * File Description:
 *      Defines "Sequence Editing" package for GBENCH.
 */

#include <ncbi_pch.hpp>

#include <objects/seqtable/SeqTable_column.hpp>
#include <objects/seqtable/SeqTable_multi_data.hpp>
#include <gui/objects/GBProject.hpp>
#include <objects/gbproj/ProjectItem.hpp>
#include <gui/core/project_tree_view.hpp>
#include <gui/core/object_loading_task.hpp>
#include <gui/framework/app_task_service.hpp>
#include <objects/submit/Seq_submit.hpp>
#include <objtools/edit/autodef_with_tax.hpp>
#include <objtools/edit/mail_report.hpp>
#include <objtools/edit/struc_comm_field.hpp>
#include <objtools/cleanup/cleanup.hpp>

#include <gui/framework/gui_package.hpp>
#include <gui/framework/pkg_wb_connect.hpp>
#include <gui/framework/workbench_impl.hpp>
#include <gui/core/selection_service_impl.hpp>

#include <gui/widgets/wx/ui_command.hpp>
#include <gui/widgets/wx/csv_exporter.hpp>

#include <gui/utils/extension_impl.hpp>
#include <gui/objutils/cmd_change_seq_feat.hpp>
#include <gui/objutils/cmd_change_align.hpp>
#include <gui/objutils/cmd_insert_bioseq_set.hpp>
#include <gui/objutils/cmd_change_seq_entry.hpp>
#include <gui/objutils/util_cmds.hpp>
#include <gui/objutils/utils.hpp>
#include <gui/objutils/macro_lib.hpp>
#include <gui/objutils/table_data_base.hpp>
#include <gui/objutils/macro_fn_do.hpp>
#include <gui/objutils/adjust_consensus_splicesite.hpp>


#include <gui/objects/GBWorkspace.hpp>
#include <gui/objects/WorkspaceFolder.hpp>
#include <gui/objects/gbench_version.hpp>
#include <gui/objects/GBenchVersionInfo.hpp>

#include <gui/widgets/edit/init.hpp>
#include <gui/widgets/edit/edit_object.hpp>
#include <gui/widgets/edit/edit_object_seq_feat.hpp>
#include <gui/widgets/edit/edit_object_seq_desc.hpp>

#include <gui/widgets/loaders/open_objects_dlg.hpp>
#include <gui/widgets/loaders/file_load_wizard.hpp>
#include "open_data_source_object_dlg.hpp"
#include <gui/utils/object_loader.hpp>
#include <gui/utils/execute_unit.hpp>
#include <gui/utils/app_popup.hpp>
#include <gui/utils/event_handler.hpp>
#include <gui/objutils/project_item_extra.hpp>
#include <gui/widgets/wx/async_call.hpp>
#include <gui/widgets/wx/sys_path.hpp>
#include <gui/widgets/wx/message_box.hpp>

#include <gui/packages/pkg_sequence_edit/autodef_tool_manager.hpp>
#include <gui/packages/pkg_sequence_edit/srcedit_tool.hpp>
#include <gui/packages/pkg_sequence_edit/seqid_fix_dlg.hpp>

#include <gui/widgets/edit/table_data_suc.hpp>
#include <gui/widgets/edit/table_data_validate.hpp>
#include <gui/widgets/edit/table_data_validate_params.hpp>
#include <gui/packages/pkg_sequence_edit/remove_desc_dlg.hpp>
#include <gui/packages/pkg_sequence_edit/remove_sequences_from_alignments.hpp>
#include <gui/packages/pkg_sequence_edit/remove_features_dlg.hpp>
#include <gui/packages/pkg_sequence_edit/remove_structured_comment.hpp>
#include <gui/packages/pkg_sequence_edit/remove_annot.hpp>
#include <gui/packages/pkg_sequence_edit/remove_genomeprojectdb.hpp>
#include <gui/packages/pkg_sequence_edit/add_citsubupdate.hpp>
#include <gui/packages/pkg_sequence_edit/pubdesc_editor.hpp>
#include <gui/packages/pkg_sequence_edit/modify_structured_comment.hpp>
#include <gui/packages/pkg_sequence_edit/remove_bankit_comment.hpp>
#include <gui/packages/pkg_sequence_edit/remove_structuredcomment_keyword.hpp>
#include <gui/packages/pkg_sequence_edit/reorder_sequences_dlg.hpp>
#include <gui/packages/pkg_sequence_edit/convert_comment.hpp>
#include <gui/packages/pkg_sequence_edit/modify_feature_id.hpp>
#include <gui/packages/pkg_sequence_edit/remove_sequences.hpp>
#include <gui/packages/pkg_sequence_edit/pcr-primers-edit.hpp>
#include <gui/packages/pkg_sequence_edit/fix_orgmod_institution.hpp>
#include <gui/packages/pkg_sequence_edit/parse_collectiondate_formats.hpp>
#include <gui/packages/pkg_sequence_edit/country_fixup.hpp>
#include <gui/packages/pkg_sequence_edit/bulk_feature_add_dlg_std.hpp>
#include <gui/packages/pkg_sequence_edit/srceditdialog.hpp>
#include <gui/packages/pkg_sequence_edit/molinfo_edit_dlg.hpp>
#include <gui/packages/pkg_sequence_edit/miscedit_util.hpp>
#include <gui/packages/pkg_sequence_edit/featedit_util.hpp>
#include <gui/packages/pkg_sequence_edit/autodef_dlg.hpp>
#include <gui/packages/pkg_sequence_edit/autodef_params.hpp>
#include <gui/packages/pkg_sequence_edit/autodef_functions.hpp>
#include <gui/packages/pkg_sequence_edit/strain_serotype_influenza.hpp>
#include <gui/packages/pkg_sequence_edit/qual_table_load_manager.hpp>
#include <gui/packages/pkg_sequence_edit/vectortrimdlg.hpp>
#include <gui/packages/pkg_sequence_edit/citem_util.hpp>
#include <gui/packages/pkg_sequence_edit/remove_unverified.hpp>
#include <gui/packages/pkg_sequence_edit/add_deflinedlg.hpp>
#include <gui/packages/pkg_sequence_edit/bulk_edit_feat_dlg_std.hpp>
#include <gui/packages/pkg_sequence_edit/convert_feat_dlg.hpp>
#include <gui/packages/pkg_sequence_edit/prefix_deflinedlg.hpp>
#include <gui/packages/pkg_sequence_edit/discrepancy_dlg.hpp>
#include <gui/packages/pkg_sequence_edit/edit_pub_dlg_std.hpp>
#include <gui/packages/pkg_sequence_edit/make_badspecifichost_tbl.hpp>
#include <gui/packages/pkg_sequence_edit/editingbtnspanel.hpp>
#include <gui/packages/pkg_sequence_edit/correct_rna_strand_dlg.hpp>
#include <gui/packages/pkg_sequence_edit/apply_rna_its_dlg.hpp>
#include <gui/packages/pkg_sequence_edit/trim_n.hpp>
#include <gui/packages/pkg_sequence_edit/label_rna.hpp>
#include <gui/packages/pkg_sequence_edit/rem_prot_titles.hpp>
#include <gui/packages/pkg_sequence_edit/add_transl_except_dlg.hpp>
#include <gui/packages/pkg_sequence_edit/bulk_cds_edit.hpp>
#include <gui/packages/pkg_sequence_edit/bulk_gene_edit.hpp>
#include <gui/packages/pkg_sequence_edit/bulk_rna_edit.hpp>
#include <gui/packages/pkg_sequence_edit/choose_set_class_dlg.hpp>
#include "test_dialog_view.hpp"
#include <gui/widgets/seq/text_panel.hpp>
#include <gui/packages/pkg_sequence_edit/retranslate_cds.hpp>
#include <gui/widgets/edit/generic_report_dlg.hpp>  
#include <gui/packages/pkg_sequence_edit/segregate_sets.hpp>
#include <gui/packages/pkg_sequence_edit/sequester_sets.hpp>
#include <gui/widgets/edit/macrofloweditor.hpp>
#include <gui/packages/pkg_sequence_edit/suc_refresh_cntrl.hpp>
#include <gui/packages/pkg_sequence_edit/revcomp_sequences.hpp>
#include <gui/packages/pkg_sequence_edit/select_target_dlg.hpp>
#include <gui/packages/pkg_sequence_edit/withdraw_sequences.hpp>
#include <gui/packages/pkg_sequence_edit/uncul_tax_tool.hpp>
#include <gui/packages/pkg_sequence_edit/apply_indexer_comments.hpp>
#include <gui/packages/pkg_sequence_edit/correct_genes_dialog.hpp>
#include <gui/packages/pkg_sequence_edit/extend_partial_features.hpp>
#include <gui/packages/pkg_sequence_edit/convert_cds_to_misc_feat.hpp>
#include <gui/packages/pkg_sequence_edit/add_flu_comments.hpp>
#include <gui/packages/pkg_sequence_edit/lowercase_qualifiers.hpp>
#include <gui/packages/pkg_sequence_edit/add_sequences.hpp>
#include <gui/packages/pkg_sequence_edit/convert_set_type_dlg.hpp>
#include <gui/packages/pkg_sequence_edit/gene_features_from_other_features.hpp>
#include <gui/packages/pkg_sequence_edit/latlon_tool.hpp>
#include <gui/packages/pkg_sequence_edit/export_table.hpp>
#include <gui/widgets/edit/edit_sequence.hpp>
#include <gui/packages/pkg_sequence_edit/raw_seq_to_delta_by_n.hpp>
#include <gui/packages/pkg_sequence_edit/update_replaced_ec_numbers.hpp>
#include <gui/packages/pkg_sequence_edit/update_seq_worker.hpp>
#include <gui/packages/pkg_sequence_edit/add_feature_between_dlg.hpp>
#include <gui/packages/pkg_sequence_edit/import_feat_table.hpp>
#include <gui/packages/pkg_sequence_edit/group_explode.hpp>
#include <gui/packages/pkg_sequence_edit/remove_text_inside_string_dlg.hpp>
#include <gui/packages/pkg_sequence_edit/find_asn1_dlg.hpp>
#include <gui/packages/pkg_sequence_edit/fuse_features.hpp>
#include <gui/packages/pkg_sequence_edit/remove_dup_feats_dlg.hpp>
#include <gui/packages/pkg_sequence_edit/remove_xrefs.hpp>
#include <gui/packages/pkg_sequence_edit/cds_from_gene_mrna_exon.hpp>
#include <gui/packages/pkg_sequence_edit/cds_to_mat_peptide.hpp>
#include <gui/packages/pkg_sequence_edit/select_features.hpp>
#include <gui/packages/pkg_sequence_edit/remove_set.hpp>
#include <gui/packages/pkg_sequence_edit/edit_seq_id.hpp>
#include <gui/packages/pkg_sequence_edit/edit_history.hpp>
#include <gui/packages/pkg_sequence_edit/exon_from_cds.hpp>
#include <gui/packages/pkg_sequence_edit/set_global_refgene_status.hpp>
#include <gui/packages/pkg_sequence_edit/clear_keywords.hpp>
#include <gui/packages/pkg_sequence_edit/convert_rpt_unit_range_to_location.hpp>
#include <gui/packages/pkg_sequence_edit/link_mrna_cds.hpp>
#include <gui/packages/pkg_sequence_edit/gene_feats_from_xrefs.hpp>
#include <gui/packages/pkg_sequence_edit/remove_gene_xrefs.hpp>
#include <gui/packages/pkg_sequence_edit/truncate_cds.hpp>
#include <gui/packages/pkg_sequence_edit/fix_product_names.hpp>
#include <gui/packages/pkg_sequence_edit/apply_gcode.hpp>
#include <gui/packages/pkg_sequence_edit/edit_seq_ends_with_align.hpp>
#include <gui/widgets/edit/field_type_constants.hpp>
#include <gui/packages/pkg_sequence_edit/update_align.hpp>
#include <gui/packages/pkg_sequence_edit/resolve_intersecting_feats.hpp>
#include <gui/packages/pkg_sequence_edit/create_protein_id.hpp>
#include <gui/packages/pkg_sequence_edit/genus_species_fixup.hpp>
#include <gui/packages/pkg_sequence_edit/country_conflict.hpp>
#include <gui/packages/pkg_sequence_edit/add_secondary.hpp>
#include <gui/packages/pkg_sequence_edit/raw_seq_to_delta_by_loc.hpp>
#include <gui/packages/pkg_sequence_edit/package_features_on_parts.hpp>
#include <gui/packages/pkg_sequence_edit/featuretbl_frame.hpp>
#include <gui/packages/pkg_sequence_edit/fix_structured_vouchers.hpp>
#include <gui/packages/pkg_sequence_edit/edit_seq_ends.hpp>
#include <gui/packages/pkg_sequence_edit/create_seq_hist_for_tpa.hpp>
#include <gui/widgets/edit/alignment_assistant.hpp>
#include <gui/packages/pkg_sequence_edit/adjust_features_for_gaps.hpp>
#include <gui/packages/pkg_sequence_edit/compare_with_biosample.hpp>
#include <gui/packages/pkg_sequence_edit/barcode_tool.hpp>
#include <gui/packages/pkg_sequence_edit/restore_local_file.hpp>
#include <gui/packages/pkg_sequence_edit/split_cds_with_too_many_n.hpp>
#include <gui/packages/pkg_sequence_edit/far_pointer_seq.hpp>
#include <gui/packages/pkg_sequence_edit/csuc_frame.hpp>
#include <gui/packages/pkg_sequence_edit/edit_cds_frame.hpp>
#include <gui/packages/pkg_sequence_edit/expand_gaps.hpp>
#include <gui/packages/pkg_sequence_edit/convert_cds_with_gaps_to_misc_feat.hpp>
#include <gui/packages/pkg_sequence_edit/convert_selected_gaps.hpp>
#include <gui/packages/pkg_sequence_edit/sorted_proteins.hpp>
#include <gui/packages/pkg_sequence_edit/add_linkage_to_gaps.hpp>
#include <gui/packages/pkg_sequence_edit/remove_srcnotes.hpp>
#include <gui/packages/pkg_sequence_edit/convert_second_prot_name.hpp>
#include <gui/packages/pkg_sequence_edit/mrna_from_gene_cds_exon.hpp>
#include <gui/packages/pkg_sequence_edit/trna_from_gene.hpp>
#include <gui/packages/pkg_sequence_edit/adjust_feats_for_introns.hpp>
#include <gui/packages/pkg_sequence_edit/clear_nomenclature.hpp>
#include <gui/packages/pkg_sequence_edit/merge_biosources.hpp>
#include <gui/packages/pkg_sequence_edit/suppress_genes.hpp>
#include <gui/packages/pkg_sequence_edit/extend_cds_to_stop.hpp>
#include <gui/packages/pkg_sequence_edit/recompute_intervals.hpp>
#include <gui/packages/pkg_sequence_edit/convert_bad_inference.hpp>
#include <gui/packages/pkg_sequence_edit/global_pubmedid_lookup.hpp>
#include <gui/packages/pkg_sequence_edit/remove_unindexed_features.hpp>
#include <gui/packages/pkg_sequence_edit/correct_interval_order.hpp>
#include <gui/packages/pkg_sequence_edit/load_secondary_accessions.hpp>
#include <gui/packages/pkg_sequence_edit/add_global_code_break.hpp>
#include <gui/packages/pkg_sequence_edit/convert_desc_to_feat.hpp>
#include <gui/packages/pkg_sequence_edit/validate_frame.hpp>
#include <gui/packages/pkg_sequence_edit/load_struct_comm_from_table.hpp>
#include <gui/packages/pkg_sequence_edit/suspect_product_rule_editor.hpp>
#include <gui/packages/pkg_sequence_edit/mixedstrands.hpp>
#include <gui/packages/pkg_sequence_edit/remove_unnecessary_exceptions.hpp>
#include <gui/packages/pkg_sequence_edit/restore_rna_edit.hpp>
#include <gui/packages/pkg_sequence_edit/fix_for_trans_splicing.hpp>
#include <gui/packages/pkg_sequence_edit/submission_wizard.hpp>

#include <gui/widgets/edit/bioseq_editor.hpp>
#include <gui/core/simple_project_view.hpp>
#include <gui/widgets/edit/edit_obj_view_dlg.hpp>
#include <gui/widgets/edit/propagate_descriptors.hpp>

#include "edit_project_item_extension.hpp"
#include "indexer_app_mode.hpp"

#include <gui/packages/pkg_sequence_edit/sequin_desktop_view.hpp>
#include <gui/packages/pkg_sequence_edit/wrong_illegal_quals.hpp>

#include <gui/packages/pkg_sequence_edit/editing_actions.hpp>
#include <gui/packages/pkg_sequence_edit/editing_action_constraint.hpp>
#include <gui/packages/pkg_sequence_edit/aecr_frame.hpp>
#include "attrib_table_column_id_panel.hpp"

#include "gui_core_helper_impl.hpp"

#include <gui/packages/pkg_sequence_edit/parse_text_dlg.hpp>
#include <gui/packages/pkg_sequence_edit/append_mod_to_org_dlg.hpp>

#include <gui/widgets/edit/sequence_editing_cmds.hpp>
#include <gui/objutils/cmd_enable_hup_id.hpp>
#include <gui/core/cmd_change_projectitem.hpp>

#include "cmd_feat_id_xrefs_from_qualifiers.hpp"
#include "cmd_convert_to_delayed_gen-prod-set_quals.hpp"

#ifdef _DEBUG
#include <gui/packages/pkg_sequence_edit/debug_macro_tool.hpp>
#endif

#include <wx/msgdlg.h>
#include <wx/statline.h>
#include <wx/clipbrd.h>
#include <gui/widgets/wx/file_extensions.hpp>
#include <wx/filedlg.h>
#include <wx/display.h>
#include <wx/evtloop.h>
#include <wx/uri.h>
#include <wx/utils.h> 

BEGIN_NCBI_SCOPE

///////////////////////////////////////////////////////////////////////////////
/// Declare the Package Object
class CSeqEditPackage 
    : public IGuiPackage
    , public IUICommandContributor
    , public IMenuContributor
    , public IPackageWorkbenchConnector
{
public:
    CSeqEditPackage();
    virtual string  GetName() const { return "Sequence Editing"; }
    virtual void    GetVersion(size_t& verMajor, size_t& verMinor, size_t& verPatch) const
    {
        verMajor = 1;
        verMinor = 0;
        verPatch = 0;
    }
    virtual bool    Init();
    virtual void    Shut() {}

    // IUICommandContributor
    virtual vector<CUICommand*> GetCommands();
    virtual wxEvtHandler* GetEventHandler();
    
    // IPackageWorkbenchConnector
    virtual void SetWorkbench(IWorkbench* wb);

protected:
    // IMenuContributor
    virtual const SwxMenuItemRec* GetMenuDef() const;

    IWorkbench*     m_Workbench;
};

static const char* sMacroLibraries[] = {
    "<std>/etc/macro_scripts/list_of_macros.mql",
    "<std>/etc/macro_scripts/autofix_gb.mql",
    "<std>/etc/macro_scripts/autofix_tsa.mql",
    "<std>/etc/macro_scripts/autofix_wgs.mql",
    NULL
};

CSeqEditPackage::CSeqEditPackage()
{
    m_Workbench = NULL;
}


bool CSeqEditPackage::Init()
{
    // Declare you extension here

    // Examples:
    //CExtensionDeclaration("view_manager_service::view_factory",
    //                      new CTextViewFactory());

    //CExtensionDeclaration("ui_data_source_type",
    //                      new CGenBankUIDataSourceType());
    //CExtensionDeclaration ("file_format_loader_manager",
    //                      new CFASTAFormatLoadManager());

    //CExtensionDeclaration("data_mining_tool", new CEntrezSearchTool());

    init_w_edit();
    
//    CExtensionDeclaration("ui_algo_tool_manager", new CAutodefToolManager());
    
    // Edit tools
//    CExtensionDeclaration("ui_algo_tool_manager", new CEditingBtnsTool());
//    CExtensionDeclaration("ui_algo_tool_manager", new CSrcEditTool());

#ifdef _DEBUG
    CExtensionDeclaration("ui_algo_tool_manager", new CDebugMacroTool());
#endif

    if (RunningInsideNCBI())
    {
        CExtensionDeclaration("view_manager_service::view_factory", new CSequinDesktopViewFactory());
    }

    CExtensionDeclaration ("file_format_loader_manager", new CQualTableLoadManager());

// IFileLoadPanelClient
    CExtensionDeclaration ("file_load_panel_client", new CFileLoadPanelClientFactory<CQualTableLoadManager>());

    vector<string> filenames;
    auto i = 0;
    while (sMacroLibraries[i]) {
        wxString path = CSysPath::ResolvePathExisting(wxString::FromUTF8(sMacroLibraries[i]));
        if (!path.IsEmpty()) {
            filenames.push_back(string(path.ToUTF8()));
        }
        else {
            NcbiMessageBox(string(sMacroLibraries[i]) + " was not found");
        }
        ++i;
    }

    if (!filenames.empty()) {
        macro::CMacroLib& macroLib = macro::CMacroLib::GetInstance();
        macroLib.ClearLibrary();
        bool status = macroLib.AppendToLibrary(filenames);
        if (!status) {
            NcbiMessageBox(macroLib.GetLastError());
        }

        wxString path = CSysPath::ResolvePathExisting(wxT("<std>/etc/synonyms.txt"));
        if (!path.IsEmpty()) {
            macroLib.StoreSynonymList(ToStdString(path));
        }
    }

    //! Please don't register commands twice. Either you register them directly 
    //! with x_RegisterCommands(), or you use the interface IUICommandContributor.
    //! Please never do both!
    //!
    //x_RegisterCommands();

    CExtensionDeclaration(EXT_POINT__PROJECT_ITEM_EXTENSION, new CEditProjectItemExtension());
    CExtensionDeclaration(EXT_POINT__APP_MODE_EXTENSION, new CIndexerAppModeExtension());
    return true;
}

void CSeqEditPackage::SetWorkbench(IWorkbench* wb)
{
    m_Workbench = wb;      
}

///////////////////////////////////////////////////////////////////////////////
/// Declare Package Entry Point
extern "C" {
    NCBI_PACKAGEENTRYPOINT_EXPORT IGuiPackage* NCBIGBenchGetPackage()
    {
        return new CSeqEditPackage();
    }
}


class CSequenceEditingEventHandler : public wxEvtHandler, public CEventHandler
{
    DECLARE_EVENT_TABLE();
    DECLARE_EVENT_MAP();

public:
    CSequenceEditingEventHandler(IWorkbench* wb = NULL);
    void IsObjectSelected(wxUpdateUIEvent& event);
    void IsLocationSelected(wxUpdateUIEvent& event);
    bool TestHupIdEnabled();
    void IsHupIdEnabled(wxUpdateUIEvent& event);
    void ItemAction(wxCommandEvent& event, CRef <CItem> item, const string& title);
    ~CSequenceEditingEventHandler();
    void ShowAfter(wxWindow* dlg);

    // methods
    void RemoveDescriptors(wxCommandEvent& evt);
    void RemoveFeatures(wxCommandEvent& evt);
    void RemoveUnindexedFeatures(wxCommandEvent& event);
    void ApplyIndexerComments(wxCommandEvent& evt);
    void RemoveSeqAnnotIds( wxCommandEvent& event );
    void RemoveBankitComment( wxCommandEvent& event );
    void RemoveDuplicateStructuredComments( wxCommandEvent& event );
    void RemoveSequencesFromAlignments( wxCommandEvent& event );
    void RemoveSequences( wxCommandEvent& event );
    void RemoveSet( wxCommandEvent& event );
    void RemoveSetsFromSet( wxCommandEvent& event );
    void RevCompSequences( wxCommandEvent& event );
    void BioseqFeatsRevComp( wxCommandEvent& event );
    void BioseqOnlyRevComp( wxCommandEvent& event );
    void UnculTaxTool( wxCommandEvent& event );
    void MergeBiosources(wxCommandEvent& event);
    void EditSequence( wxCommandEvent& event );
    void AlignmentAssistant( wxCommandEvent& event );
    void RemoveSeqAnnotAlign( wxCommandEvent& event );
    void RemoveSeqAnnotGraph( wxCommandEvent& event );
    void RemoveGenomeProjectsDB( wxCommandEvent& event );
    void RemoveEmptyGenomeProjectsDB( wxCommandEvent& event );
    void TaxFixCleanup( wxCommandEvent& event );
    void SpecificHostCleanup( wxCommandEvent& event );
    void FixNonReciprocalLinks( wxCommandEvent& event );
    void DisableStrainForwarding( wxCommandEvent& event );
    void EnableStrainForwarding(wxCommandEvent& event);
    void EditPubs( wxCommandEvent& event );
    void GlobalPubmedIdLookup(wxCommandEvent& event);
    void FixCapitalizationAll( wxCommandEvent& event );
    void FixCapitalizationAuthors( wxCommandEvent& event );
    void FixCapitalizationTitles( wxCommandEvent& event );
    void FixCapitalizationAffiliation( wxCommandEvent& event );
    void FixCapitalizationCountry( wxCommandEvent& event );
    void RemoveUnpublishedPublications( wxCommandEvent& event );
    void RemoveInPressPublications( wxCommandEvent& event );
    void RemovePublishedPublications( wxCommandEvent& event );
    void RemoveCollidingPublications( wxCommandEvent& event );
    void RemoveAllPublications( wxCommandEvent& event );
    void RemoveAuthorConsortiums( wxCommandEvent& event );
    void ReverseAuthorNames( wxCommandEvent& event );
    void StripAuthorSuffixes( wxCommandEvent& event );
    void TruncateAuthorMiddleInitials( wxCommandEvent& event );
    void ConvertAuthorToConstortiumWhereLastName( wxCommandEvent& event );
    void ConvertAuthorToConsortiumAll( wxCommandEvent& event );
    void AddCitSubForUpdate( wxCommandEvent& event );
    void FixUsaAndStates( wxCommandEvent& event );
    void RetranslateCDS( wxCommandEvent& event );
    void ResynchronizePartials( wxCommandEvent& event );
    void AddTranslExcept( wxCommandEvent& event );
    void RemoveAllStructuredComments( wxCommandEvent& event );
    void RemoveEmptyStructuredComments( wxCommandEvent& event );
    void ModifyStructuredComment( wxCommandEvent& event );
    void SetGlobalRefGeneStatus( wxCommandEvent& event );
    void ClearKeywords( wxCommandEvent& event );
    void ClearNomenclature(wxCommandEvent& event);
    void RemoveStructuredCommentKeyword( wxCommandEvent& event );
    void AddStructuredCommentKeyword( wxCommandEvent& event );
    void AddGenomeAssemblyStructuredComment( wxCommandEvent& event );
    void AddAssemblyStructuredComment(wxCommandEvent& event);
    void ReorderStructuredComment( wxCommandEvent& event );
    void ConvertComment( wxCommandEvent& event );
    void ParseComment( wxCommandEvent& event );
    void IllegalQualsToNote( wxCommandEvent& event);
    void RmIllegalQuals( wxCommandEvent& event);
    void WrongQualsToNote( wxCommandEvent& event);
    void ConvertBadInference(wxCommandEvent& event);
    void RmWrongQuals( wxCommandEvent& event);
    void LowercaseQuals( wxCommandEvent& event );
    void AssignFeatureIds( wxCommandEvent& event );
    void ClearFeatureIds( wxCommandEvent& event );
    void ReassignFeatureIds( wxCommandEvent& event );
    void UniqifyFeatureIds( wxCommandEvent& event );
    void TrimJunkInPrimerSeqs( wxCommandEvent& event );
    void FixiInPrimerSeqs( wxCommandEvent& event );
// J. Chen begin
    void RemoveCddFeatDbxref(wxCommandEvent& event );
    void PrefixAuthWithTax(wxCommandEvent& event );
    void FocusSet( wxCommandEvent& event );
    void FocusClear( wxCommandEvent& event );
    void SetTransgSrcDesc( wxCommandEvent& event );
    void SplitDblinkQuals(wxCommandEvent& evt);
// J. Chen end
    void FixOrgModInstitution( wxCommandEvent& event );
    void FixStructuredVouchers( wxCommandEvent& event );
    void SwapPrimerSeqName( wxCommandEvent& event );
    void MergePrimerSets( wxCommandEvent& event );
    void SplitPrimerSets( wxCommandEvent& event );
    void AppendModToOrg( wxCommandEvent& event );
    void ParseCollectionDateMonthFirst( wxCommandEvent& event );
    void ParseCollectionDateDayFirst( wxCommandEvent& event );
    void CountryFixupCap( wxCommandEvent& event );
    void CountryFixupNoCap( wxCommandEvent& event );
    void VectorTrim( wxCommandEvent& event );
    void SelectTarget( wxCommandEvent& event );
    void SelectFeature( wxCommandEvent& event );
    void AddCDS( wxCommandEvent& event );
    void AddRNA( wxCommandEvent& event );
    void AddOtherFeature( wxCommandEvent& event );
    void AddFeatureBetween( wxCommandEvent& event );
    void Validate( wxCommandEvent& event );
    void SortUniqueCount( wxCommandEvent& event );
    void SortUniqueCountTree(wxCommandEvent& event);
    void Discrepancy(wxCommandEvent& event);
    void Oncaller(wxCommandEvent& event);
    void Submitter(wxCommandEvent& event);
    void Mega(wxCommandEvent& event);
    void DiscrepancyList(wxCommandEvent& event);
    void RemoveAllFeatures(wxCommandEvent& event);
    void RemoveDupFeats( wxCommandEvent& evt );
    void RemoveDupFeatsWithOptions( wxCommandEvent& evt );
    void ResolveIntersectingFeats( wxCommandEvent& evt );
    void AddSequences ( wxCommandEvent& event );
    void ApplyEditConvertRemove ( wxCommandEvent& event );
    void ParseText ( wxCommandEvent& event );
    void RemoveTextInsideStr( wxCommandEvent& event );
    void ConvertFeatures ( wxCommandEvent& event );
    void CdsToMatPeptide( wxCommandEvent& event );
    void ConvertCdsToMiscFeat( wxCommandEvent& evt );
    void ConvertCdsWithGapsToMiscFeat( wxCommandEvent& evt );
    void ConvertCdsWithInternalStopToMiscFeat( wxCommandEvent& evt );
    void ConvertCdsWithInternalStopToMiscFeatViral( wxCommandEvent& evt );
    void ConvertCdsWithInternalStopToMiscFeatUnverified( wxCommandEvent& evt );
    void TableReader ( wxCommandEvent& event );
    void TableReaderClipboard ( wxCommandEvent& event );
    void BulkEdit ( wxCommandEvent& event );
    void MolInfoEdit ( wxCommandEvent& event );
    void ExportTable ( wxCommandEvent& event );
    void Autodef ( wxCommandEvent& event );
    void PT_Cleanup ( wxCommandEvent& event );
    void ParseStrainSerotypeFromNames ( wxCommandEvent& event);
    void AddStrainSerotypeToNames ( wxCommandEvent& event);
    void FixupOrganismNames ( wxCommandEvent& event);
    void SplitQualifiersAtCommas ( wxCommandEvent& event);
    void SplitStructuredCollections ( wxCommandEvent& event);
    void TrimOrganismNames ( wxCommandEvent& event);
    void RemoveUnverified( wxCommandEvent& event );
    void AddNamedrRNA ( wxCommandEvent& event);
    void AddControlRegion ( wxCommandEvent& event);
    void AddMicrosatellite ( wxCommandEvent& event);
    void CreateFeature ( wxCommandEvent& event);
    void CreateRegulatory ( wxCommandEvent& evt);
    void CreateDescriptor ( wxCommandEvent& event);
    void CreateSpecificStructuredComment( wxCommandEvent& event );
    void DeleteSpecificStructuredComment( wxCommandEvent& evt );
    void EditFeature ( wxCommandEvent& evt);
    void ExtendPartialsConstr( wxCommandEvent& evt);
    void ExtendPartialsAll( wxCommandEvent& evt);
    void AddDefLine ( wxCommandEvent& evt);
    void PrefixDefLines ( wxCommandEvent& event);
    void EditingButtons ( wxCommandEvent& event);
    void MakeBadSpecificHostTable ( wxCommandEvent& event);
    void CorrectRNAStrand ( wxCommandEvent& event);
    void ApplyRNA_ITS ( wxCommandEvent& event);
    void TrimNsRich( wxCommandEvent& event );
    void TrimNsTerminal( wxCommandEvent& event );
    void AddFluComments( wxCommandEvent& event );
    void LabelRna( wxCommandEvent& event );
    void RemProtTitles( wxCommandEvent& event );
    void RemAllProtTitles( wxCommandEvent& event );
    void BulkCdsEdit ( wxCommandEvent& event );
    void BulkGeneEdit ( wxCommandEvent& event );
    void BulkRnaEdit ( wxCommandEvent& event );
    void CorrectGenes ( wxCommandEvent& event );
    void SegregateSets ( wxCommandEvent& event );
    void SequesterSets ( wxCommandEvent& event );
    void DescriptorPropagateDown( wxCommandEvent& event );
    void WithdrawSequences ( wxCommandEvent& event );
    void AddSet ( wxCommandEvent& event );
    void PropagateDBLink ( wxCommandEvent& event );
    void ConvertSetType ( wxCommandEvent& event );
    void ListMacroButtonNames ( wxCommandEvent& event );
    void TestDialogView( wxCommandEvent& event );
    void TestFormView( wxCommandEvent& event );
    void PrepareSeqSubmit(wxCommandEvent& event);
    CRef<CProjectItem> CreateSeqSubmit(CRef<CGBProjectHandle> ph, CRef<CProjectItem> pi, CIRef<CProjectService> srv);
    void ChangeToGenomicDna();
    void SetTopLevelBioseqSetClass();
    void SaveSubmissionFile(wxCommandEvent& event);
    void SaveASN1File(wxCommandEvent& event);
    void MacroEditor( wxCommandEvent& event );
    void AdjustForConsensusSpliceSites( wxCommandEvent& event );
    void AdjustCDSForIntrons(wxCommandEvent& event);
    void AdjustrRNAForIntrons(wxCommandEvent& event);
    void AdjusttRNAForIntrons(wxCommandEvent& event);
    void AdjustmRNAForIntrons(wxCommandEvent& event);
    void CombineSelectedGenesIntoPseudogenes( wxCommandEvent& evt );
    void GeneFromOtherFeat( wxCommandEvent& evt );
    void CdsFromGeneMrnaExon( wxCommandEvent& evt );
    void mRNAFromGeneCdsExon( wxCommandEvent& evt );
    void tRNAFromGene( wxCommandEvent& evt );
    void ExonFromCds( wxCommandEvent& evt );
    void ExonFromMRNA( wxCommandEvent& evt );
    void ExonFromTRNA( wxCommandEvent& evt );
    void LatLonTool ( wxCommandEvent& event );
    void CDSGeneRangeErrorSuppress( wxCommandEvent& event );
    void CDSGeneRangeErrorRestore( wxCommandEvent& event );
    void SuppressGenes(wxCommandEvent& evt);
    void RemoveSegGaps( wxCommandEvent& event );
    void RawSeqToDeltaByNs( wxCommandEvent& event );
    void RawSeqToDeltaByLoc( wxCommandEvent& event );
    void RawSeqToDeltaByAssemblyGapFeatures(wxCommandEvent& event);
    void SplitCDSwithTooManyXs( wxCommandEvent& event );
    void AdjustFeaturesForGaps( wxCommandEvent& event );
    void DeltaSeqToRaw( wxCommandEvent& event );
    void ExpandGaps(wxCommandEvent& event);
    void ConvertSelectedGapsToKnown(wxCommandEvent& event);
    void ConvertSelectedGapsToUnknown(wxCommandEvent& event);
    void ConvertGapsBySize(wxCommandEvent& event);
    void EditSelectedGaps(wxCommandEvent& event);
    void CreateGapFeats(wxCommandEvent& event);
    void AddLinkageToGaps(wxCommandEvent& event);
    void ConvertFeatsToGaps(wxCommandEvent& event);
    void CombineAdjacentGaps(wxCommandEvent& event);
    void RmCultureNotes( wxCommandEvent& event );
    void Update_Replaced_EC_numbers( wxCommandEvent& event );
    void UpdateSingleSequence(wxCommandEvent& event);
    void UpdateMultipleSequences(wxCommandEvent& event);
    void ImportFeatureTable( wxCommandEvent& event );
    void ImportFeatureTableClipboard( wxCommandEvent& event );
    void ImportGFF3(wxCommandEvent& event);
    void LoadSecondaryAccessions(wxCommandEvent& event);
    void LoadSecondaryAccessionsHistoryTakeover(wxCommandEvent& event);
    void AddGlobalCodeBreak( wxCommandEvent& event );
    void GroupExplode( wxCommandEvent& event);
    void FindASN1( wxCommandEvent& evt );
    void FuseFeatures( wxCommandEvent& evt );
    void FuseJoinsInLocs(wxCommandEvent& event);
    void ExplodeRNAFeats(wxCommandEvent& event);
    void ApplyStructuredCommentField( wxCommandEvent& evt );
    void EditStructuredCommentPrefixSuffix( wxCommandEvent& evt ); 
    void RemoveDbXrefsCDS( wxCommandEvent& evt ); 
    void RemoveDbXrefsGenes( wxCommandEvent& evt ); 
    void RemoveDbXrefsRNA( wxCommandEvent& evt ); 
    void RemoveDbXrefsAllFeats( wxCommandEvent& evt ); 
    void RemoveDbXrefsBioSource( wxCommandEvent& evt ); 
    void RemoveDbXrefsBioSourceAndFeats( wxCommandEvent& evt ); 
    void RemoveTaxonFeats( wxCommandEvent& event);
    void RemoveTaxonFeatsAndBioSource( wxCommandEvent& event);
    void AddKeywordGDS( wxCommandEvent& event );
    void AddKeywordTPA_inferential( wxCommandEvent& event );
    void AddKeywordTPA_experimental( wxCommandEvent& event );
    void AddKeywordTPA_assembly( wxCommandEvent& event );
    void AddKeywordWithConstraint( wxCommandEvent& event );
    void EditSeqId( wxCommandEvent& event );
    void RemoveLocalSeqIdsFromNuc(wxCommandEvent& event);
    void RemoveLocalSeqIdsFromProt(wxCommandEvent& event);
    void RemoveLocalSeqIdsFromAll(wxCommandEvent& event);
    void RemoveGiSeqIdsFromAll(wxCommandEvent& event);
    void RemoveGenbankSeqIdsFromProt(wxCommandEvent& event);
    void RemoveGenbankSeqIdsFromAll(wxCommandEvent& event);
    void RemoveSeqIdNamesFromProtFeats(wxCommandEvent& event);
    void ConvertAccessionToLocalIdsAll(wxCommandEvent& event);
    void ConvertAccessionToLocalIdsNuc(wxCommandEvent& event);
    void ConvertAccessionToLocalIdsProt(wxCommandEvent& event);
    void ConvertAccessionToLocalIdsName(wxCommandEvent& event);
    void LocalToGeneralId(wxCommandEvent& event);
    void GeneralToLocalId(wxCommandEvent& event);
    void RemoveGeneralId( wxCommandEvent& event );
    void RemoveUnnecessaryExceptions(wxCommandEvent& event);
    void EditHistory( wxCommandEvent& event );
    void ConvertToDelayedGenProdSetQuals( wxCommandEvent& event);
    void ConvertRptUnitRangeToLoc( wxCommandEvent& event);
    void ReorderSeqById( wxCommandEvent& event);
    void LinkmRNACDS( wxCommandEvent& event );
    void JustRemoveProteins( wxCommandEvent& event );
    void RemoveProteinsAndRenormalizeNucProtSets( wxCommandEvent& event );
    void RemoveOrphanedProteins( wxCommandEvent& event );
    void GeneFeatFromXrefs( wxCommandEvent& event );
    void GeneXrefsFromFeats( wxCommandEvent& event );
    void RemoveAllGeneXrefs( wxCommandEvent& event );
    void RemoveUnnecessaryGeneXrefs( wxCommandEvent& event );
    void RemoveNonsuppressingGeneXrefs( wxCommandEvent& event );
    void RemoveOrphanLocusGeneXrefs( wxCommandEvent& event );
    void RemoveOrphanLocus_tagGeneXrefs( wxCommandEvent& event );
    void RemoveGeneXrefs( wxCommandEvent& event );
    void ExtendCDS( wxCommandEvent& event );
    void TruncateCDS( wxCommandEvent& event );
    void ExtendCDSToStop( wxCommandEvent& event );
    void RecomputeIntervalsAndUpdateGenes(wxCommandEvent& event);
    void RecomputeIntervals(wxCommandEvent& event);
    void FixProductNamesDefault( wxCommandEvent& event );
    void FixProductNamesOptions( wxCommandEvent& event );
    void BulkApplyGenCode( wxCommandEvent& event );
    void EditSeqEndsWithAlign( wxCommandEvent& event );
    void EditSequenceEnds( wxCommandEvent& event );
    void UpdateAlign( wxCommandEvent& event );
    void CreateProteinId( wxCommandEvent& event );
    void CreateLocusTagGene( wxCommandEvent& event );
    void NormalizeGeneQuals(wxCommandEvent& event);
    void GenusSpeciesFixup( wxCommandEvent& event );
    void CountryConflict( wxCommandEvent& event );
    void AddSecondary( wxCommandEvent& event );
    void PackageFeaturesOnParts( wxCommandEvent& event );
    void ShowFeatureTable ( wxCommandEvent& event );
    void SaveDescriptors( wxCommandEvent& event );
    void CreateSeqHistForTpaDetailed( wxCommandEvent& event );
    void CreateSeqHistForTpa( wxCommandEvent& event );
    void RemoveSeqHistAssembly( wxCommandEvent& event );
    void CompareWithBiosample( wxCommandEvent& event );
    void CompareWithBiosampleFirstOnly( wxCommandEvent& event );
    void BarcodeTool( wxCommandEvent& event );
    void ApplyFBOL(wxCommandEvent& event);
    void RestoreLocalFile ( wxCommandEvent& event );
    void FarPointerSeq( wxCommandEvent& event );
    void ConvertBadCdsAndRnaToMiscFeat(wxCommandEvent& event);
    void EditCdsFrame(wxCommandEvent& event);
    void RestoreRNAediting(wxCommandEvent& event);
    void ViewSortedProteins(wxCommandEvent& event);
    void ConsolidateLikeModsWithSemicolon(wxCommandEvent& event);
    void ConsolidateLikeModsWithoutSemicolon(wxCommandEvent& event);
    void ConvertSecondProtNameToDesc(wxCommandEvent& event);
    void ConvertDescToSecondProtName(wxCommandEvent& event);
    void ConvertDescToFeatComment(wxCommandEvent& event);
    void ConvertDescToFeatSource(wxCommandEvent& event);
    void ConvertDescToFeatPub(wxCommandEvent& event);
    void ConvertDescToFeatPubConstraint(wxCommandEvent& event);
    void ConvertFeatToDescComment(wxCommandEvent& event);
    void ConvertFeatToDescSource(wxCommandEvent& event);
    void ConvertFeatToDescPub(wxCommandEvent& event);
    void Duplicate(wxCommandEvent& event);
    void ExportStructuredCommentsTable(wxCommandEvent& evt);
    void LoadStructComments(wxCommandEvent& event);
    void CorrectIntervalOrder( wxCommandEvent& event );
    void SuspectProductRulesEditor(wxCommandEvent& event);
    void CheckForMixedStrands(wxCommandEvent& event);
    void EnableHupId(wxCommandEvent& event);
    void IndexerTypeSelection(wxCommandEvent& event);
    void FixForTransSplicing(wxCommandEvent& evt);
    void RemoveDuplicateGoTerms(wxCommandEvent& evt);
    void Help(wxCommandEvent& evt);

protected:
    void    x_OnSelectionEvent(CEvent* event);

private:
    ICommandProccessor* m_CmdProccessor;
    CSeq_entry_Handle m_TopSeqEntry;
    wxString m_WorkDir;
    bool m_disable_hup_check;
    bool m_disable_id_check;
    CConstRef<CSeq_submit> m_SeqSubmit;
    IWorkbench*     m_Workbench;
    TConstScopedObjects m_SelObjects;
    vector<CRef<CSeq_loc> > m_CachedSelectedLocations;
    bool m_attached;
    bool m_IsAa;
    bool m_IsSeq;
    void x_SetUpTSEandUnDoManager(IWorkbench* wb);
    vector<CRef<CSeq_loc> > &x_GetSelectedLocations();
    void x_CacheSelectedLocations();
    CSeq_align::TDim x_FindRelevantAlignRow(const CSeq_align& align, const CSeq_id* id);
    bool x_IsLocationOkForFeatCmd(CSeqFeatData::ESubtype subtype, CRef<CSeq_loc> loc);
    void SaveSettings(const string &regpath, wxWindow *dlg) const;
    void LoadSettings(const string &regpath, wxWindow *dlg);
    CRef<CSeq_loc> GetFeatureLocation(CSeqFeatData::ESubtype subtype);
};

wxEvtHandler* CSeqEditPackage::GetEventHandler()
{
    return new CSequenceEditingEventHandler(m_Workbench);
}

CSequenceEditingEventHandler::CSequenceEditingEventHandler(IWorkbench* wb)
{
    m_TopSeqEntry.Reset();
    m_CmdProccessor = NULL;
    m_Workbench = wb;
    m_attached = false;
    m_disable_hup_check = false;
    m_disable_id_check = false;
    wxWindow* main_window = NULL;
    CWorkbench* wb_frame = dynamic_cast<CWorkbench*>(m_Workbench);
    if (wb_frame)
        main_window = wb_frame->GetMainWindow();
    CEditingBtnsPanel::GetInstance(main_window, m_Workbench, true);
}

CSequenceEditingEventHandler::~CSequenceEditingEventHandler()
{
    if (m_Workbench && m_attached)
    {
        CSelectionService* service = m_Workbench->GetServiceByType<CSelectionService>();
        if (service)
        {
            service->RemoveListener(this);
            m_attached = false;
        }
    }
}

void CSequenceEditingEventHandler::x_OnSelectionEvent(CEvent* event)
{
    m_SelObjects.clear();
    CSelectionService* service = m_Workbench->GetServiceByType<CSelectionService>();
    if (service) {
        service->GetActiveObjects(m_SelObjects);
        if (m_SelObjects.empty())
        {
            GetViewObjects(m_Workbench, m_SelObjects);
        }
        x_CacheSelectedLocations();
    }
}

void CSequenceEditingEventHandler::x_SetUpTSEandUnDoManager(IWorkbench* wb)
{
    m_TopSeqEntry.Reset();
    m_WorkDir.Clear();
    m_SeqSubmit.Reset(NULL);
    m_CmdProccessor = NULL;
    if (!wb) return;
    if (!m_attached)
    {
        m_SelObjects.clear();
        CSelectionService* service = wb->GetServiceByType<CSelectionService>();
        if (service)
        {
            service->AddListener(this);
            service->GetActiveObjects(m_SelObjects); 
            if (m_SelObjects.empty())
            {
                GetViewObjects(m_Workbench, m_SelObjects);
            }
            x_CacheSelectedLocations();
            m_attached = true;
        }
    }

    TConstScopedObjects &objects = m_SelObjects;
    if (objects.empty()) return;
    NON_CONST_ITERATE (TConstScopedObjects, it, objects) {
        const CSeq_submit* sub = dynamic_cast<const CSeq_submit*>((*it).object.GetPointer());
        if (sub) {
            m_SeqSubmit.Reset(sub);
        }
        if (!m_TopSeqEntry && it->object && it->scope) {
            CSeq_entry_Handle seh = GetTopSeqEntryFromScopedObject(*it);
            if (seh) {
                m_TopSeqEntry = seh;
                if (m_SeqSubmit) {
                    break;
                }
            }
        }
    }
    if (!m_TopSeqEntry) return;
    CIRef<CProjectService> srv = wb->GetServiceByType<CProjectService>();
    if (srv)
    {
        CRef<CGBWorkspace> ws = srv->GetGBWorkspace();
        if (!ws) return;

        CGBDocument* doc = dynamic_cast<CGBDocument*>(ws->GetProjectFromScope(m_TopSeqEntry.GetScope()));
        if (doc) {
            m_CmdProccessor = &doc->GetUndoManager();
            m_WorkDir = doc->GetWorkDir();
        }
    }

}

vector<CRef<CSeq_loc> >& CSequenceEditingEventHandler::x_GetSelectedLocations()
{
    return m_CachedSelectedLocations;
}


void CSequenceEditingEventHandler::x_CacheSelectedLocations()
{
    m_CachedSelectedLocations.clear();
    m_IsSeq = false;

    if (!m_Workbench) {
        return;
    }

    TConstScopedObjects &objects = m_SelObjects;
    if (objects.empty()) return;
    size_t count = 0;
    NON_CONST_ITERATE (TConstScopedObjects, it, objects) {
        const CObject* ptr = it->object.GetPointer();
        
        const CSeq_feat* seqfeat = dynamic_cast<const CSeq_feat*>(ptr);
        const CSeq_loc* seqloc = dynamic_cast<const CSeq_loc*>(ptr);
        const CBioseq* bioseq = dynamic_cast<const CBioseq*>(ptr);
        const CSeqdesc* seqdesc = dynamic_cast<const CSeqdesc*>(ptr);
        const CPubdesc* pubdesc = dynamic_cast<const CPubdesc*>(ptr);

        const CSeq_annot* annot = dynamic_cast<const CSeq_annot*>(ptr);
        const CBioseq_set* bioseqset = dynamic_cast<const CBioseq_set*>(ptr);

        const CSeq_align* align = dynamic_cast<const CSeq_align*>(ptr);
        const CSeq_id* seq_id = dynamic_cast<const CSeq_id*>(ptr);

        if (seqfeat) {
            if (seqfeat->IsSetLocation()) {
                CRef<CSeq_loc> loc(new CSeq_loc());
                loc->Assign(seqfeat->GetLocation());
                m_CachedSelectedLocations.push_back(loc);
            }
            m_IsSeq = false;
        } else if (seq_id && !CSeqUtils::IsVDBAccession(seq_id->GetSeqIdString())) {
            CBioseq_Handle bsh = it->scope->GetBioseqHandle(*seq_id);
            if (bsh) {
                CRef<CSeq_id> new_id(new CSeq_id());
                new_id->Assign(*seq_id);
                CRef<CSeq_loc> loc(new CSeq_loc(*new_id, 0, bsh.GetInst_Length() - 1));
                m_CachedSelectedLocations.push_back(loc);
                m_IsSeq = true;
                if (count == 0)
                    m_IsAa = bsh.IsAa();
            }
        } else if (seqloc) {
            
            if (seqloc->IsWhole()) {
                CRef<CSeq_id> new_id(new CSeq_id());
                new_id->Assign(seqloc->GetWhole());
                CBioseq_Handle bsh = it->scope->GetBioseqHandle(*new_id);
                if (bsh) {
                    CRef<CSeq_loc> loc(new CSeq_loc(*new_id, 0, bsh.GetInst_Length() - 1));
                    m_CachedSelectedLocations.push_back(loc);
                }
            } else {
                CRef<CSeq_loc> loc(new CSeq_loc());
                loc->Assign(*seqloc);
                m_CachedSelectedLocations.push_back(loc);
            }
            CBioseq_Handle bsh = it->scope->GetBioseqHandle(*seqloc);
            if (bsh)
            {
                m_IsSeq = true;
                if (count == 0)
                    m_IsAa = bsh.IsAa();
            }
            count++;
        } else if (bioseq) {
            CRef<CSeq_loc> loc(new CSeq_loc());
            loc->SetInt().SetId().Assign(*(bioseq->GetId().front()));
            loc->SetInt().SetFrom(0);
            loc->SetInt().SetTo(bioseq->GetLength() - 1);
            m_CachedSelectedLocations.push_back(loc);
            m_IsSeq = true;
            if (count == 0)
                m_IsAa = bioseq->IsAa();
        } else if (seqdesc) {
            CSeq_entry_Handle seh = edit::GetSeqEntryForSeqdesc(it->scope, *seqdesc);
            CBioseq_CI bi(seh);
            while (bi) {
                CRef<CSeq_loc> loc(new CSeq_loc());
                loc->SetInt().SetId().Assign(*(bi->GetId().front().GetSeqId()));
                loc->SetInt().SetFrom(0);
                loc->SetInt().SetTo(bi->GetInst_Length() - 1);
                m_CachedSelectedLocations.push_back(loc);
                ++bi;
            }
            m_IsSeq = false;
        } else if (pubdesc) {
            CSeq_entry_Handle seh = GetSeqEntryForPubdesc(it->scope, *pubdesc);
            CBioseq_CI bi(seh);
            while (bi) {
                CRef<CSeq_loc> loc(new CSeq_loc());
                loc->SetInt().SetId().Assign(*(bi->GetId().front().GetSeqId()));
                loc->SetInt().SetFrom(0);
                loc->SetInt().SetTo(bi->GetInst_Length() - 1);
                m_CachedSelectedLocations.push_back(loc);
                ++bi;
            }
            m_IsSeq = false;
        } else  if (annot) {
            if (annot->IsSetData() && 
                annot->GetData().IsFtable() && 
                !annot->GetData().GetFtable().empty() && 
                annot->GetData().GetFtable().front()->IsSetLocation()) {

                CRef<CSeq_loc> loc(new CSeq_loc());
                loc->Assign(annot->GetData().GetFtable().front()->GetLocation());
                m_CachedSelectedLocations.push_back(loc);
            }
            m_IsSeq = false;
        }  else if (bioseqset) {
            if (bioseqset->IsSetClass() && bioseqset->GetClass() == CBioseq_set::eClass_nuc_prot)
            {
                const CBioseq& bseq = bioseqset->GetNucFromNucProtSet();
                CRef<CSeq_loc> loc(new CSeq_loc());
                loc->SetInt().SetId().Assign(*(bseq.GetId().front()));
                loc->SetInt().SetFrom(0);
                loc->SetInt().SetTo(bseq.GetLength() - 1);
                m_CachedSelectedLocations.push_back(loc);
            }
            else
            {
                CSeq_entry* seq = bioseqset->GetParentEntry();
                if (seq)
                {
                    m_CachedSelectedLocations.clear();
                }
            }
            m_IsSeq = false;
        } else if (align) {
            CSeq_align::TDim entry_row = x_FindRelevantAlignRow(*align, dynamic_cast<const CSeq_id*>(objects.back().object.GetPointer()));
            CRef<CSeq_loc> loc = align->CreateRowSeq_loc(entry_row);
            m_CachedSelectedLocations.push_back(loc);
            m_IsSeq = true;
            if (count == 0)
                m_IsAa = it->scope->GetBioseqHandle(*loc).IsAa();
        } 
        
    }
    if (count > 1)
    {
        m_IsSeq = false;
    }
    if (!m_TopSeqEntry && !m_CachedSelectedLocations.empty()) {
        CBioseq_Handle bsh = objects.front().scope->GetBioseqHandle(*(m_CachedSelectedLocations.front()->GetId()));
        if (bsh)
            m_TopSeqEntry = bsh.GetTopLevelEntry();
    }
}

CSeq_align::TDim CSequenceEditingEventHandler::x_FindRelevantAlignRow(const CSeq_align& align, const CSeq_id* id)
{
    // the relevant row corresponds to the subject sequence, which is the second one by default
    CSeq_align::TDim row = 1;
    if (align.GetSegs().IsSpliced()) {
        // looking for genomic sequence, which is always 1 in spliced-segs
        row = 1;
    }
    if (id)
    {
        CSeq_align::TDim num_rows = align.CheckNumRows();
        for (CSeq_align::TDim i = 0; i < num_rows; i++)
        {
            const CSeq_id &row_id = align.GetSeq_id(i);
            if (row_id.Match(*id))
            {
                row = i;
                break;
            }
        }
    }
    return row;
}

void CSequenceEditingEventHandler::IsObjectSelected(wxUpdateUIEvent& event)
{
    x_SetUpTSEandUnDoManager(m_Workbench);              
    event.Enable(m_TopSeqEntry && m_CmdProccessor);
}

bool CSequenceEditingEventHandler::TestHupIdEnabled()
{
    if (m_TopSeqEntry && !m_disable_hup_check)
    {
        CRef<CSeq_id> id(new CSeq_id("MF358689", CSeq_id::fParse_Default));
        CBioseq_Handle bsh;
        try
        {
            bsh = m_TopSeqEntry.GetScope().GetBioseqHandle(*id);
        }
        catch (const exception&)
        {
            wxMessageBox(wxT("Error while attempting to connect to the ID server, restarting gbench is recommended"), wxT("Error"), wxOK | wxICON_ERROR, NULL);
            m_disable_hup_check = true;
        }
        if (bsh)
            return true;
    }
    return false;
}

void CSequenceEditingEventHandler::IsHupIdEnabled(wxUpdateUIEvent& event)
{
    x_SetUpTSEandUnDoManager(m_Workbench);              
    if (m_TopSeqEntry && m_CmdProccessor)
    {
        event.Enable(true);
        event.Check(TestHupIdEnabled());
        return;
    }
    event.Check(false);
    event.Enable(false);
}

void CSequenceEditingEventHandler::IsLocationSelected(wxUpdateUIEvent& evt)
{
    vector<CRef<CSeq_loc> >& locs = x_GetSelectedLocations();   

    if (!RunningInsideNCBI() && (m_TopSeqEntry || !locs.empty()))
    {
    evt.Enable(true);
    return;
    }

    if (!m_IsSeq)
    {
    evt.Enable(false);
    return;
    }


    bool rval = false;
    bool ok_for_something = false;
    CSeqFeatData::ESubtype subtype = CBioseqEditor::GetFeatTypeFromCmdID(evt.GetId());
    ITERATE(vector<CRef<CSeq_loc> >, lit, locs) {
        if (x_IsLocationOkForFeatCmd(subtype, *lit)) {
            rval = true;
        ok_for_something |= true;
        break;
        }
    if (x_IsLocationOkForFeatCmd(CSeqFeatData::eSubtype_gene, *lit)) {
        ok_for_something |= true;
        }
    if (x_IsLocationOkForFeatCmd(CSeqFeatData::eSubtype_prot, *lit)) {
        ok_for_something |= true;
        }
       
    }
    evt.Enable(rval);

    if (!ok_for_something && m_TopSeqEntry && !m_disable_id_check)
    {
        CRef<CSeq_id> id(new CSeq_id("U12345", CSeq_id::fParse_Default));
        CBioseq_Handle bsh;
        try
        {
            bsh = m_TopSeqEntry.GetScope().GetBioseqHandle(*id);
        }
        catch (const exception&)
        {
            wxMessageBox(wxT("Unable to connect to the ID server, restarting gbench is recommended"), wxT("Error"), wxOK | wxICON_ERROR, NULL);
            m_disable_id_check = true;
        }
    }
}



// Implementation
vector<CUICommand*> CSeqEditPackage::GetCommands()
{
    vector<CUICommand*> commands;
    commands.push_back(new CUICommand(eCmdRemoveDescriptors, "Remove Descriptors", "Remove Descriptors from Selected Object", "RDSO"));
    commands.push_back(new CUICommand(eCmdRemoveFeatures, "Remove Features", "Remove Features from Selected Object", "RFSO"));
    commands.push_back(new CUICommand(eCmdFixForTransSplicing, "Fix For Trans-splicing", "Fix For Trans-splicing", "FFTS"));
    commands.push_back(new CUICommand(eCmdRemoveUnindexedFeatures, "Remove Unindexed Features", "Remove Unindexed Features", "RUIF"));
    commands.push_back(new CUICommand(eCmdApplyIndexerComments, "Apply Indexer Comments", "Apply Indexer Comments", "AICS"));
    commands.push_back(new CUICommand(eCmdRemoveSeqAnnotIds, "Remove Seq-annot IDs", "Remove Seq-annot IDs from Selected Object", "RSAID"));
    commands.push_back(new CUICommand(eCmdRemoveCddFeatDbxref, "Remove All CDD Features and DBxrefs", "Remove All CDD Features and Dbxrefs", "RACFAD"));
    commands.push_back(new CUICommand(eCmdRemoveBankitComment, "Remove Bankit Comments", "Remove Bankit Comments from Selected Object", "RBC"));
    commands.push_back(new CUICommand(eCmdRemoveDuplicateStructuredComments, "Remove Duplicate Structured Comments", "Remove Duplicate Structured Comments from Selected Object", "RDSC"));
    commands.push_back(new CUICommand(eCmdRemoveSequencesFromAlignments, "Remove Sequences from Alignments", "Remove Sequences From Alignments from Selected Object", "RSFASO"));
    commands.push_back(new CUICommand(eCmdRemoveSequences, "Remove Sequences from Record", "Remove Sequences from Selected Object", "RSSO"));
    commands.push_back(new CUICommand(eCmdRemoveSequences_ext, "Remove Sequences", "Remove Sequences from Selected Object", "RSSO"));
    commands.push_back(new CUICommand(eCmdJustRemoveProteins, "Just Remove Proteins", "Just Remove Proteins", "JRP"));
    commands.push_back(new CUICommand(eCmdRemoveProteinsAndRenormalizeNucProtSets, "And Renormalize Nuc-Prot Sets", "And Renormalize Nuc-Prot Sets", "RPRNPS"));
    commands.push_back(new CUICommand(eCmdRemoveOrphanedProteins, "Orphaned Proteins", "Orphaned Proteins", "ROP"));
    commands.push_back(new CUICommand(eCmdRemoveSet,"RemoveSet", "RemoveSet","RSET"));
    commands.push_back(new CUICommand(eCmdRemoveSetsFromSet,"Remove Sets in Set", "Remove Sets in Set", "RSIS"));
    commands.push_back(new CUICommand(eCmdRevCompSequences, "Bioseq RevComp by ID", "Reverse Complement Sequences from Selected Object", "RVCS"));
    commands.push_back(new CUICommand(eCmdRevCompSequences_ext, "Reverse Complement Sequences by Sequence ID", "Reverse Complement Sequences from Selected Object", "RVCS"));
    commands.push_back(new CUICommand(eCmdBioseqFeatsRevComp, "BioseqFeatsRevComp", "Reverse Complement Sequences and Features", "RVCSF"));
    commands.push_back(new CUICommand(eCmdBioseqOnlyRevComp,"BioseqOnlyRevComp","Reverse Complement Sequences and Graphs", "RVCSG"));
    commands.push_back(new CUICommand(eCmdUnculTaxTool,"Uncultured Tax Tool", "Uncultured Tax Tool", "UCTT"));
    commands.push_back(new CUICommand(eCmdMergeBiosources, "Merge Multiple BioSources On Sequence", "Merge Multiple BioSources On Sequence", "MMBS"));
    commands.push_back(new CUICommand(eCmdCompareWithBiosample, "All Records", "Compare with Biosample", "CWBS"));
    commands.push_back(new CUICommand(eCmdCompareWithBiosampleFirstOnly, "First Record Only", "Compare with Biosample", "CWBSFO"));
    commands.push_back(new CUICommand(eCmdConsolidateLikeModsWithSemicolon, "With semicolons", "With semicolons", "CLMS"));
    commands.push_back(new CUICommand(eCmdConsolidateLikeModsWithoutSemicolon, "Without semicolons", "Without semicolons", "CLMWS"));
    commands.push_back(new CUICommand(eCmdBarcodeTool, "BARCODE Discrepancy Tool", "BARCODE Discrepancy Tool", "BDISTL"));
    commands.push_back(new CUICommand(eCmdApplyFBOL, "Apply FBOL Dbxrefs", "Apply FBOL Dbxrefs", "AFBOL"));
    commands.push_back(new CUICommand(eCmdEditSequence, "Edit Sequence", "Edit Sequence", "EDSEQ"));
    commands.push_back(new CUICommand(eCmdAlignmentAssistant, "Alignment Assistant", "Alignment Assistant", "ALAS"));
    commands.push_back(new CUICommand(eCmdRemoveAlignments, "Remove Alignments", "Remove Alignments from Selected Object", "RASO"));
    commands.push_back(new CUICommand(eCmdRemoveGraphs, "Remove Graphs", "Remove Graphs from Selected Object", "RGSO"));
    commands.push_back(new CUICommand(eCmdRemoveGenomeProjectId, "Remove All Genome Project IDs", "Remove All Genome Project Ids from Selected Object", "RGPIDSO"));
    commands.push_back(new CUICommand(eCmdRemoveEmptyGenomeProjectId, "Remove Empty Genome Project IDs", "Remove Empty Genome Project Ids from Selected Object", "REGPIDSO"));
    commands.push_back(new CUICommand(eCmdTaxFixCleanup, "Tax_fix/Cleanup", "Fix Taxonomy and Perform Basic and Extended Cleanup", "TFCU"));
    commands.push_back(new CUICommand(eCmdTaxFixCleanup_ext, "Lookup Taxonomy and Cleanup Record", "Lookup Taxonomy and Cleanup Record", "TFCU"));
    commands.push_back(new CUICommand(eCmdSpecificHostCleanup, "Specific Host Cleanup", "Specific Host Cleanup", "SHCU"));
    commands.push_back(new CUICommand(eCmdExtendedCleanup, "SeriousSeqEntryCleanup", "Perform Basic and Extended Cleanup", "SSEC"));
    commands.push_back(new CUICommand(eCmdFixNonReciprocalLinks, "Fix Non-Reciprocal Links", "Fix Non-Reciprocal Feature ID Links", "FNRL"));
    commands.push_back(new CUICommand(eCmdDisableStrainForwarding, "Globally Disable Strain Forwarding", "Globally Disable Strain Forwarding", "GDSF"));
    commands.push_back(new CUICommand(eCmdEnableStrainForwarding, "Globally Enable Strain Forwarding", "Globally Disable Strain Forwarding", "GESF"));
    commands.push_back(new CUICommand(eCmdEditPubs, "Edit Publications", "Edit Publications", "EP"));
    commands.push_back(new CUICommand(eCmdGlobalPubmedIdLookup, "Global PubMedId Lookup", "Global PubMedId Lookup", "GPMIDL"));
    commands.push_back(new CUICommand(eCmdFixCapitalizationAll, "Fix All Capitalization", "Fix All Capitalization for Selected Object", "FACSO"));
    commands.push_back(new CUICommand(eCmdFixCapitalizationAuthors, "Fix Capitalization in Authors", "Fix Capitalization in Authors for Selected Object", "FCASO"));
    commands.push_back(new CUICommand(eCmdFixCapitalizationTitles, "Fix Capitalization in Titles", "Fix Capitalization in Titles for Selected Object", "FCTSO"));
    commands.push_back(new CUICommand(eCmdFixCapitalizationAffiliation, "Fix Capitalization in Affiliations", "Fix Capitalization in Affiliations for Selected Object", "FCAfSO"));
    commands.push_back(new CUICommand(eCmdFixCapitalizationCountry, "Fix Capitalization in Countries", "Fix Capitalization in Countries for Selected Object", "FCCSO"));
    commands.push_back(new CUICommand(eCmdRemoveUnpublishedPublications, "Remove Unpublished Publications", "Remove Unpublished Publications from Selected Object", "RUPSO"));
    commands.push_back(new CUICommand(eCmdRemoveInPressPublications, "Remove In Press Publications", "Remove In Press Publications from Selected Object", "RIPSO"));
    commands.push_back(new CUICommand(eCmdRemovePublishedPublications, "Remove Published Publications", "Remove Published Publications from Selected Object", "RPPSO"));
    commands.push_back(new CUICommand(eCmdRemoveCollidingPublications, "Remove Colliding Publications", "Remove Colliding Publications from Selected Object", "RCPSO"));
    commands.push_back(new CUICommand(eCmdRemoveAllPublications, "Remove All Publications", "Remove All Publications from Selected Object", "RAPSO"));
    commands.push_back(new CUICommand(eCmdRemoveAuthorConsortiums, "Remove Author Consortiums", "Remove Author Consortiums from Selected Object", "RACSO"));
    commands.push_back(new CUICommand(eCmdReverseAuthorNames, "Reverse Author Names", "Reverse Author Names for Selected Object", "RANSO"));
    commands.push_back(new CUICommand(eCmdStripAuthorSuffixes, "Strip Author Suffixes", "Strip Author Suffixes from Selected Object", "SASSO"));
    commands.push_back(new CUICommand(eCmdTruncateAuthorMiddleInitials, "Truncate Author Middle Initials", "Truncate Author Middle Initials from Selected Object", "TAMISO"));
    commands.push_back(new CUICommand(eCmdConvertAuthorToConstortiumWhereLastName, "Convert Author to Consortium where Last Name Contains Consortium", "Convert Author To Consortium Where Last Name Contains Consortium For Selected Object", "CATCWLSO"));
    commands.push_back(new CUICommand(eCmdConvertAuthorToConsortiumAll, "Convert All Authors to Consortium", "Convert All Authors To Consortium For Selected Object", "CAATCSO"));
    commands.push_back(new CUICommand(eCmdAddCitSubForUpdate, "Add Cit-sub for Update", "Add Cit-sub For Update for Selected Object", "ACSFUSO"));
    commands.push_back(new CUICommand(eCmdFixUsaAndStates, "Fix USA and States", "Fix USA and States  for Selected Object", "FUSSO"));
    commands.push_back(new CUICommand(eCmdRetranslateCDSObeyStop, "Obey Stop Codon", "Retranslate CDS, obey stop codon", "RCDSOSC"));
    commands.push_back(new CUICommand(eCmdRetranslateCDSIgnoreStopExceptEnd, "Ignore Stop Codon Except at End of Complete CDS", "Retranslate CDS, Ignore Stop Codon Except at End of Complete CDS", "RCDSISCEAE"));
    commands.push_back(new CUICommand(eCmdRetranslateCDSIgnoreStopExceptEnd_ext, "Retranslate Coding Regions", "Retranslate CDS, Ignore Stop Codon Except at End of Complete CDS", "RCDSISCEAE"));
    commands.push_back(new CUICommand(eCmdRetranslateCDSChooseFrame, "Choose Frame with No Stop Codon", "Retranslate CDS, Choose Frame With No Stop Codon", "RCDSCF"));
    commands.push_back(new CUICommand(eCmdEditCdsFrame, "Edit Coding Region Frame", "Edit Coding Region Frame", "ECRF"));
    commands.push_back(new CUICommand(eCmdRestoreRNAediting, "Restore RNA Editing of CDS", "Restore RNA Editing", "RRNAE"));
    commands.push_back(new CUICommand(eCmdResynchronizePartialsCDS, "Resynchronize CDS Partials", "Resynchronize CDS Partials", "RCDSP"));
    commands.push_back(new CUICommand(eCmdAddTranslExcept, "Add Translational Exceptions with Comment", "Add Translational Exceptions with Comment", "ATEWC"));
    commands.push_back(new CUICommand(eCmdRemoveAllStructuredComments,"Remove All Structured Comments","Remove All Structured Comments from Selected Object","RASCSO"));
    commands.push_back(new CUICommand(eCmdRemoveEmptyStructuredComments,"Remove Empty Structured Comments","Remove Empty Structured Comments from Selected Object","RESCSO"));
    commands.push_back(new CUICommand(eCmdModifyStructuredComment,"Remove Structured Comment Field","Remove Structured Comment Field from Selected Object","RSCFSO"));
    commands.push_back(new CUICommand(eCmdSetGlobalRefGeneStatus, "Set Global RefGene Status", "Set Global RefGene Status", "SGRGS"));
    commands.push_back(new CUICommand(eCmdClearKeywords, "Clear Keywords", "Clear Keywords", "CLKW"));
    commands.push_back(new CUICommand(eCmdClearNomenclature, "Clear Nomenclature", "Clear Nomenclature", "CLNMC"));
    commands.push_back(new CUICommand(eCmdConvertToDelayedGenProdSetQuals,"Convert to Delayed Gen-Prod-Set Qualifiers", "Convert to Delayed Gen-Prod-Set Qualifiers", "CDGPSQ"));
    commands.push_back(new CUICommand(eCmdConvertRptUnitRangeToLoc, "Convert Repeat-region Rpt-unit-range to Location", "Convert repeat-region rpt-unit-range to location", "CRRL"));
    commands.push_back(new CUICommand(eCmdRemoveStructuredCommentKeyword,"Remove Structured Comment Keyword","Remove Structured Comment Keyword from Selected Object","RSCKSO"));
    commands.push_back(new CUICommand(eCmdAddStructuredCommentKeyword ,"Add Structured Comment Keyword","Add Structured Comment Keyword to Selected Object","ASCKSO"));
    commands.push_back(new CUICommand(eCmdAddKeywordGDS,"GDS", "Add Keyword GDS", "AKG"));
    commands.push_back(new CUICommand(eCmdAddKeywordTPA_inferential,"TPA:inferential","Add Keyword TPA:inferential", "AKTI"));
    commands.push_back(new CUICommand(eCmdAddKeywordTPA_experimental,"TPA:experimental","Add Keyword TPA:experimental","AKTE"));
    commands.push_back(new CUICommand(eCmdAddKeywordTPA_assembly,"TPA:assembly","Add Keyword TPA:assembly","AKTA"));
    commands.push_back(new CUICommand(eCmdAddKeywordWithConstraint, "With Constraint", "Add Keyword With Constraint", "AKWC"));
    commands.push_back(new CUICommand(eCmdAddGenomeAssemblyStructuredComment, "Add Genome Assembly Structured Comment", "Add Genome Assembly Structured Comment", "AGASC"));
    commands.push_back(new CUICommand(eCmdAddAssemblyStructuredComment, "Add Assembly Structured Comment", "Add Assembly Structured Comment", "AASC"));
    commands.push_back(new CUICommand(eCmdReorderStructuredComment,"Reorder Structured Comment","Reorder Structured Comment in Selected Object","RSCSO"));
    commands.push_back(new CUICommand(eCmdConvertComment,"Convert Structured Comment","Convert Structured Comment in Selected Object","CSCSO"));
    commands.push_back(new CUICommand(eCmdParseComment,"Parse Structured Comment","Parse Structured Comment in Selected Object","PSCSO"));
    commands.push_back(new CUICommand(eCmdExportStructuredCommentsTable, "Export Structured Comment Table", "Export Structured Comment Table", "ESCT"));
    commands.push_back(new CUICommand(eCmdLoadStructComments, "Load Structured Comments from Table", "Load Structured Comments from Table", "LSCT"));
    commands.push_back(new CUICommand(eCmdIllegalQualsToNote, "Convert Illegal Qualifiers to Note", "Convert Illegal Qualifiers to Note", "CIQN"));
    commands.push_back(new CUICommand(eCmdRmIllegalQuals,"Remove Illegal Qualifiers", "Remove Illegal Qualifiers", "RMIQ"));
    commands.push_back(new CUICommand(eCmdWrongQualsToNote,"Convert Wrong Qualifiers to Note", "Convert Wrong Qualifiers to Note", "CWQN"));
    commands.push_back(new CUICommand(eCmdConvertBadInference, "Convert Bad Inference Qualifiers to Notes", "Convert Bad Inference Qualifiers to Notes", "CBIN"));
    commands.push_back(new CUICommand(eCmdRmWrongQuals,"Remove Wrong Qualifiers", "Remove Wrong Qualifiers", "RMWQ"));
    commands.push_back(new CUICommand(eCmdRemoveDbXrefsCDS,"From CDSs", "Remove DbXrefs from CDSs","RMDBXR"));
    commands.push_back(new CUICommand(eCmdRemoveDbXrefsGenes,"From Genes", "Remove DbXrefs from Genes","RMDBXR"));
    commands.push_back(new CUICommand(eCmdRemoveDbXrefsRNA,"From RNAs", "Remove DbXrefs from RNAs","RMDBXR"));
    commands.push_back(new CUICommand(eCmdRemoveDbXrefsAllFeats,"From All Features", "Remove DbXrefs from All Features","RMDBXR"));
    commands.push_back(new CUICommand(eCmdRemoveDbXrefsBioSource,"From All BioSources","Remove DbXrefs from All BioSources", "RMDBXR")); 
    commands.push_back(new CUICommand(eCmdRemoveDbXrefsBioSourceAndFeats, "From All Features and BioSources", "Remove DbXrefs from All Features and BioSources", "RMDBXR"));
    commands.push_back(new CUICommand(eCmdRemoveTaxonFeats, "From Features", "Remove Taxons From Features", "RMTFF"));
    commands.push_back(new CUICommand(eCmdRemoveTaxonFeatsAndBioSource, "From Features And BioSources", "Remove Taxons From Features And BioSources", "RMTFFBS"));
    commands.push_back(new CUICommand(eCmdGroupExplode, "Group Explode", "Group Explode", "GREX"));
    commands.push_back(new CUICommand(eCmdFindASN1, "Find ASN.1", "Find ASN.1", "FA1"));
    commands.push_back(new CUICommand(eCmdFuseFeatures, "Fuse Features", "Fuse Features", "FUFE"));
    commands.push_back(new CUICommand(eCmdFuseJoinsInLocs, "Fuse Joins in Locations", "Fuse Joins in Locations", "FJIL"));
    commands.push_back(new CUICommand(eCmdExplodeRNAFeats, "Explode RNA Features", "Explode RNA Features", "ERNAF"));
    commands.push_back(new CUICommand(eCmdAssignFeatureIds,"Assign Feature IDs", "Assign Feature IDs for Selected Object","AFIDSO"));
    commands.push_back(new CUICommand(eCmdClearFeatureIds,"Clear Feature IDs and Xrefs", "Clear Feature IDs for Selected Object","CFIDSO"));
    commands.push_back(new CUICommand(eCmdReassignFeatureIds,"Reassign Feature IDs", "Reassign Feature IDs for Selected Object","RFIDSO"));
    commands.push_back(new CUICommand(eCmdUniqifyFeatureIds,"Uniqify Feature IDs", "Uniqify Feature IDs for Selected Object","UFIDSO"));
    commands.push_back(new CUICommand(eCmdTrimJunkInPrimerSeqs,"Trim Junk in Primer Sequences","Trim Junk In Primer Seqs for Selected Object","TJIPSSO"));
    commands.push_back(new CUICommand(eCmdFixiInPrimerSeqs,"Fix i in Primer Sequences","Fix i in Primer Seqs for Selected Object","FIIPSSO"));
    commands.push_back(new CUICommand(eCmdPrefixAuthWithTax, "Prefix Authority with Organism", "Prefix Authority with Organism", "PAWO"));
    commands.push_back(new CUICommand(eCmdFocusSet, "Set", "BioSource is_focus is set", "SFSET"));
    commands.push_back(new CUICommand(eCmdFocusClear, "Clear", "BioSource is_focus is clear", "SFCLE"));
    commands.push_back(new CUICommand(eCmdSetTransgSrcDesc, "Set Transgenic on Source Descriptor", "Set Transgenic on Source Desc when Source Feat present", "STOSD"));
    commands.push_back(new CUICommand(eCmdSplitDblinkQuals, "Split DBLink Qualifiers at Commas", "Split DBLink Quals at Commas", "SDQAC"));
    commands.push_back(new CUICommand(eCmdFixOrgModInstitution,"Fix OrgMod Institution Code","Fix OrgMod Institution Code for Selected Object","FOMICSO"));
    commands.push_back(new CUICommand(eCmdFixStructuredVouchers, "Fix Structured Vouchers", "Fix Structured Vouchers", "FSTV"));
    commands.push_back(new CUICommand(eCmdSwapPrimerSeqName,"Swap Primer Sequence and Name","Swap Primer Seq and Name for Selected Object","SPSNSO"));
    commands.push_back(new CUICommand(eCmdMergePrimerSets,"Merge Primer Sets","Merge Primer Sets for Selected Object","MPSSO"));
    commands.push_back(new CUICommand(eCmdSplitPrimerSets,"Split Primer Sets by Position","Split Primer Sets by Position for Selected Object","SPSBPSO"));
    commands.push_back(new CUICommand(eCmdAppendModToOrg,"Append Modifier to Organism","Append Mod To Org for Selected Object","AMTOSO"));
    commands.push_back(new CUICommand(eCmdParseCollectionDateMonthFirst,"Parse Collection Date Month First","Parse Collection Date Month First for Selected Object","PCDMFSO"));
    commands.push_back(new CUICommand(eCmdParseCollectionDateDayFirst,"Parse Collection Date Day First","Parse Collection Date Day First for Selected Object","PCDDFSO"));
    commands.push_back(new CUICommand(eCmdCountryFixupCap,"Country Fixup Fix Capitalization after Colon","Country Fixup Fix Capitalization After Colon","CFFCAC"));
    commands.push_back(new CUICommand(eCmdCountryFixupNoCap,"Country Fixup Do Not Fix Capitalization after Colon","Country Fixup Do Not Fix Capitalization After Colon","CFDNFCAC"));
    commands.push_back(new CUICommand(eCmdVectorTrim, "Vector Search and Trim Tool", "Vector Search and Trim Tool", "VSTT"));
    commands.push_back(new CUICommand(eCmdSelectTarget, "Select Target", "Select Target", "ST"));
    commands.push_back(new CUICommand(eCmdSelectTarget_ext, "Select Specific Sequence by Sequence ID", "Select Specific Sequence by Sequence ID", "ST"));
    commands.push_back(new CUICommand(eCmdSelectFeature, "Select Features", "Select Features", "SF"));
    commands.push_back(new CUICommand(eCmdApplyCDS, "Add CDS", "Add CDS", "AC"));
    commands.push_back(new CUICommand(eCmdApplyAddRNA, "Add RNA", "Add RNA", "AR"));
    commands.push_back(new CUICommand(eCmdApplyAddOther, "Add Other Feature", "Add other Feature", "AOF"));
    commands.push_back(new CUICommand(eCmdApplyAddOther_ext, "Add Other Feature (Non CDS, RNA)", "Add other Feature", "AOF"));
    commands.push_back(new CUICommand(eCmdAddFeatureBetween, "Add Feature Between", "Add Feature Between", "AFB"));
    commands.push_back(new CUICommand(eCmdValidate, "Validate", "Validate", "V"));
    commands.push_back(new CUICommand(eCmdValidate_ext, "Validation Report", "Validate", "V"));
    commands.push_back(new CUICommand(eCmdValidateNoAlignments, "Validate No Alignments", "Validate no Alignments", "VNA"));
    commands.push_back(new CUICommand(eCmdValidateCheckInferences, "Validate Check Inferences", "Validate Check Inferences", "VCI"));
//    commands.back()->AddAccelerator(wxACCEL_NORMAL, 'v');
    commands.push_back(new CUICommand(eCmdSortUniqueCountText, "SUC (Sort Unique Count)", "Sort Unique Count", "SUC"));
    commands.push_back(new CUICommand(eCmdSortUniqueCountTree, "cSUC (Clickable SUC)", "Clickable Sort Unique Count", "SUC"));
    commands.push_back(new CUICommand(eCmdSortUniqueCountTree_ext, "FlatFile Summary", "Clickable Sort Unique Count", "SUC"));
    commands.push_back(new CUICommand(eCmdDiscrepancy, "Discrepancy Report", "Discrepancy Report", "DR"));
    commands.push_back(new CUICommand(eCmdOncaller, "Oncaller Report", "Oncaller Report", "OR"));
    commands.push_back(new CUICommand(eCmdSubmitter_ext, "Submitter Report", "Submitter Report", "SR"));
    commands.push_back(new CUICommand(eCmdMega, "Mega Report", "Mega Report", "MR"));
    commands.push_back(new CUICommand(eCmdDiscrepancyList, "List Discrepancy Tests", "List Discrepancy Tests", "LDT"));
    commands.push_back(new CUICommand(eCmdAutodefDefaultOptions, "Autodef", "Autodef Default Options", "ADO"));
    commands.push_back(new CUICommand(eCmdAutodefOptions, "Autodef Select Options", "Autodef Select Options", "ADSO"));
    commands.push_back(new CUICommand(eCmdAutodefMisc, "Autodef Misc Feats", "Autodef Misc Feats", "ADMF"));
    commands.push_back(new CUICommand(eCmdAutodefId, "Autodef ID", "Autodef ID", "ADID"));
    commands.push_back(new CUICommand(eCmdAutodefNoMods, "Autodef No Modifiers", "Autodef No Modifiers", "ADNM"));
    commands.push_back(new CUICommand(eCmdAutodefPopset, "Add Popset Titles", "Add Popset Titles", "ADPT"));
    commands.push_back(new CUICommand(eCmdAutodefRefresh, "Refresh DefLine with Previous Options", "Refresh Definition Line", "RDEFL"));
    commands.push_back(new CUICommand(eCmdInstantiateProteinTitles, "PT_Cleanup", "PT_Cleanup Protein Titles", "PTCU"));
    commands.push_back(new CUICommand(eCmdRemoveAllFeatures, "Remove All Features", "Remove All Features", "RAF"));
    commands.push_back(new CUICommand(eCmdRemoveDupFeats, "All Feature Types", "Remove Duplicate Features", "RDF"));
    commands.push_back(new CUICommand(eCmdRemoveDupFeats_ext, "Remove Duplicate Features", "Remove Duplicate Features", "RDF"));
    commands.push_back(new CUICommand(eCmdRemoveDupFeatsWithOptions, "Features with Options", "Remove Duplicate Feats With Options", "RDFWO"));
    commands.push_back(new CUICommand(eCmdRemoveDupFeatsWithOptions_ext, "Remove Duplicate Features with Options", "Remove Duplicate Feats With Options", "RDFWO"));
    commands.push_back(new CUICommand(eCmdResolveIntersectingFeats, "Resolve Intersecting Feature Locations", "Resolve Intersecting Feature Locations", "RIFL"));
    commands.push_back(new CUICommand(eCmdAddSequences, "Add Sequences", "Add Sequences", "AS"));
    commands.push_back(new CUICommand(eCmdApplySourceQual, "Apply Source Qualifier", "Apply Source Qual", "ASQ"));
    commands.push_back(new CUICommand(eCmdEditSourceQual, "Edit Source Qualifier", "Edit Source Qual", "ESQ"));
    commands.push_back(new CUICommand(eCmdConvertSourceQual, "Convert Source Qualifier", "Convert Source Qual", "CSQ"));
    commands.push_back(new CUICommand(eCmdSwapSourceQual, "Swap Source Qualifiers", "Swap Source Qual", "SSQ"));
    commands.push_back(new CUICommand(eCmdRemoveSourceQual, "Remove Source Qualifier", "Remove Source Qual", "RSQ"));
    commands.push_back(new CUICommand(eCmdApplyRNAQual, "Apply RNA Qualifier", "Apply RNA Qual", "ARQ"));
    commands.push_back(new CUICommand(eCmdEditRNAQual, "Edit RNA Qualifier", "Edit RNA Qual", "ERQ"));
    commands.push_back(new CUICommand(eCmdConvertRNAQual, "Convert RNA Qualifier", "Convert RNA Qual", "CRQ"));
    commands.push_back(new CUICommand(eCmdSwapRNAQual, "Swap RNA Qualifiers", "Swap RNA Qual", "SRQ"));
    commands.push_back(new CUICommand(eCmdRemoveRNAQual, "Remove RNA Qualifier", "Remove RNA Qual", "RRQ"));
    commands.push_back(new CUICommand(eCmdApplyCGPQual, "Apply CDS-Gene-Prot-mRNA Qualifier", "Apply CDS-Gene-Prot-mRNA Qual", "ACGPQ"));
    commands.push_back(new CUICommand(eCmdEditCGPQual, "Edit CDS-Gene-Prot-mRNA Qualifier", "Edit CDS-Gene-Prot-mRNA Qual", "ECGPQ"));
    commands.push_back(new CUICommand(eCmdConvertCGPQual, "Convert CDS-Gene-Prot-mRNA Qualifier", "Convert CDS-Gene-Prot-mRNA Qual", "CCGPQ"));
    commands.push_back(new CUICommand(eCmdSwapCGPQual, "Swap CDS-Gene-Prot-mRNA Qualifiers", "Swap CDS-Gene-Prot-mRNA Qual", "SCGPQ"));
    commands.push_back(new CUICommand(eCmdRemoveCGPQual, "Remove CDS-Gene-Prot-mRNA Qualifier", "Remove CDS-Gene-Prot-mRNA Qual", "RCGPQ"));
    commands.push_back(new CUICommand(eCmdApplyFeatQual, "Apply Feature Qualifier", "Apply Feature Qual", "AFQ"));
    commands.push_back(new CUICommand(eCmdEditFeatQual, "Edit Feature Qualifier", "Edit Feature Qual", "EFQ"));
    commands.push_back(new CUICommand(eCmdConvertFeatQual, "Convert Feature Qualifier", "Convert Feature Qual", "CFQ"));
    commands.push_back(new CUICommand(eCmdConvertSecondProtNameToDesc, "Second Protein Name to Description", "Second Protein Name to Description", "CSPND"));
    commands.push_back(new CUICommand(eCmdConvertDescToSecondProtName, "Protein Description to Second Name", "Protein Description to Second Name", "PDSN"));
    commands.push_back(new CUICommand(eCmdConvertDescToFeatComment, "Comment", "Convert Desc to Feat Comment", "CDFC"));
    commands.push_back(new CUICommand(eCmdConvertDescToFeatSource, "Source", "Concert Desc to Feat Source", "CDFS"));
    commands.push_back(new CUICommand(eCmdConvertDescToFeatPub, "All", "Convert Desc to Feat Pub", "CDFP"));
    commands.push_back(new CUICommand(eCmdConvertDescToFeatPubConstraint, "By Constraint", "Convert Desc to Feat Pub", "CDFPC"));
    commands.push_back(new CUICommand(eCmdConvertFeatToDescComment, "Comment", "Convert Feat to Desc Comment", "CFDC"));
    commands.push_back(new CUICommand(eCmdConvertFeatToDescSource, "Source", "Convert Feat to Desc Source", "CFDS"));
    commands.push_back(new CUICommand(eCmdConvertFeatToDescPub, "Publication", "Convert Feat to Desc Pub", "CFDP"));
    commands.push_back(new CUICommand(eCmdSwapFeatQual, "Swap Feature Qualifiers", "Swap Feature Qual", "SFQ"));
    commands.push_back(new CUICommand(eCmdRemoveFeatQual, "Remove Feature Qualifier", "Remove Feature Qual", "RFQ"));
    commands.push_back(new CUICommand(eCmdRemoveTextOutsideString, "Remove Text Outside String", "Remove Text Outside String", "RTOS"));
    commands.push_back(new CUICommand(eCmdParseText, "Parse Text", "Parse Text", "PT"));
    commands.push_back(new CUICommand(eCmdParseTextFromDefline, "Parse Text", "Parse Text", "PTDL"));
    commands.push_back(new CUICommand(eCmdParseLocalToSrc, "Parse Local ID to Source", "Parse Local ID to Src", "PLIDTS"));
    commands.push_back(new CUICommand(eCmdRemoveTextInsideStr, "Remove Text Inside String", "Remove Text Inside String", "RTIS"));
    commands.push_back(new CUICommand(eCmdLowercaseQuals, "Lowercase Qualifiers", "Lowercase Qualifiers", "LQ"));
    commands.push_back(new CUICommand(eCmdConvertFeatures, "Convert Features", "Convert Features", "CF"));
    commands.push_back(new CUICommand(eCmdCdsToMatPeptide, "Convert CDS to Mat-peptide", "Convert CDS to mat-peptide", "CDSTMP"));
    commands.push_back(new CUICommand(eCmdConvertCdsToMiscFeat, "Convert CDS to Misc-feature or Pseudogene", "Convert CDS to misc-feat or Pseudogene", "CCDSMF"));
    commands.push_back(new CUICommand(eCmdConvertCdsWithInternalStopToMiscFeat, "To Misc-feature", "Convert CDS with Internal Stop Codon to Misc_feat", "CCDSWIMF"));
    commands.push_back(new CUICommand(eCmdConvertCdsWithInternalStopToMiscFeatViral, "To Misc-feature (Viral)", "Convert CDS with Internal Stop Codon to Misc_feat (Viral)", "CCDSWIMFV"));
    commands.push_back(new CUICommand(eCmdConvertCdsWithInternalStopToMiscFeatUnverified, "To Misc-feature with Unverified Comment", "Convert CDS with Internal Stop Codon to Misc_feat with Unverified Comment", "CCDSWIMFU"));
    commands.push_back(new CUICommand(eCmdConvertCdsWithGapsToMiscFeat, "Convert CDS with Internal Gaps to misc-feat", "Convert CDS with Internal Gaps to misc-feat", "CCDSWGMF"));
    commands.push_back(new CUICommand(eCmdTableReader, "Table Reader", "Table Reader", "TR"));
    commands.push_back(new CUICommand(eCmdTableReader_ext, "Use Table Reader", "Table Reader", "TR"));
    commands.push_back(new CUICommand(eCmdTableReaderClipboard, "Table Reader from Clipboard", "Table Reader from Clipboard", "TRFC"));
    commands.push_back(new CUICommand(eCmdBulkSourceEdit, "Bulk Source Edit", "Bulk Source Edit", "BE"));
    commands.push_back(new CUICommand(eCmdBulkCdsEdit, "Bulk CDS Edit", "Bulk CDS Edit", "BCDSE"));
    commands.push_back(new CUICommand(eCmdBulkGeneEdit, "Bulk Gene Edit", "Bulk Gene Edit", "BGENEE"));
    commands.push_back(new CUICommand(eCmdBulkRnaEdit, "Bulk RNA Edit", "Bulk RNA Edit", "BRNAE"));
    commands.push_back(new CUICommand(eCmdCorrectGenes, "Correct Genes for CDSs or mRNAs", "Correct Genes for CDSs or mRNAs", "CGFCOM"));
    commands.push_back(new CUICommand(eCmdSegregateSets, "Segregate Sets", "Segregate Sets", "SEGSET"));
    commands.push_back(new CUICommand(eCmdSequesterSets, "Sequester Sets", "Sequester Sets", "SEQSET"));
    commands.push_back(new CUICommand(eCmdDescriptorPropagateDown, "Propagate Descriptors", "Propagate Descriptors", "PRDS"));
    commands.push_back(new CUICommand(eCmdReorderSeqById, "Reorder by ID", "Reorder by ID", "RBID"));
    commands.push_back(new CUICommand(eCmdCombineSelectedGenesIntoPseudogenes,"Combine Selected Genes into Pseudogenes", "Combine selected genes into pseudogenes","CSGPG"));
    commands.push_back(new CUICommand(eCmdWithdrawSequences, "Withdraw Sequences", "Withdraw Sequences", "WSEQ"));
    commands.push_back(new CUICommand(eCmdAddSet, "Add Set", "Add Set", "AS"));
    commands.push_back(new CUICommand(eCmdPropagateDBLink, "Propagate DBLink Descriptors", "Propagate DBLink Descriptors", "PDBLD"));
    commands.push_back(new CUICommand(eCmdChangeSetClass, "Convert Set Type", "Convert Set Type", "CST"));
    commands.push_back(new CUICommand(eCmdMolInfoEdit, "MolInfo Edit", "MolInfo Edit", "MIE"));
    commands.push_back(new CUICommand(eCmdExportTable,"Export Table", "Export Table", "ETBL"));
    commands.push_back(new CUICommand(eCmdParseStrainSerotypeFromNames, "Parse Strain, Serotype from Names", "Parse Strain, Serotype from Names", "PSSFN"));
    commands.push_back(new CUICommand(eCmdAddStrainSerotypeToNames, "Add Strain, Serotype to Names", "Add Strain Serotype to Names", "ASSTN"));
    commands.push_back(new CUICommand(eCmdFixupOrganismNames, "Fixup Organism Names", "Fixup Organism Names", "FON"));
    commands.push_back(new CUICommand(eCmdSplitQualifiersAtCommas, "Split Qualifiers at Commas", "Split Quals at Commas", "SQAC"));
    commands.push_back(new CUICommand(eCmdSplitStructuredCollections, "Split Structured Collections", "Split Structured Collections", "SSC"));
    commands.push_back(new CUICommand(eCmdTrimOrganismNames, "Trim Organism Names", "Trim Organism Names", "TON"));
    commands.push_back(new CUICommand(eCmdRemoveUnverified, "Remove Unverified", "Remove Unverified from Selected Object", "RU"));
    commands.push_back(new CUICommand(eCmdAddNamedrRNA12S, "12S", "Add Named rRNA 12S", "ANR12S"));
    commands.push_back(new CUICommand(eCmdAddNamedrRNA16S, "16S", "Add Named rRNA 16S", "ANR16S"));
    commands.push_back(new CUICommand(eCmdAddNamedrRNA23S, "23S", "Add Named rRNA 23S", "ANR23S"));
    commands.push_back(new CUICommand(eCmdAddNamedrRNA18S, "18S", "Add Named rRNA 18S", "ANR18S"));
    commands.push_back(new CUICommand(eCmdAddNamedrRNA28S, "28S", "Add Named rRNA 28S", "ANR28S"));
    commands.push_back(new CUICommand(eCmdAddNamedrRNA26S, "26S", "Add Named rRNA 26S", "ANR26S"));
    commands.push_back(new CUICommand(eCmdAddNamedrRNASmall, "Small", "Add Named rRNA Small", "ANRSmall"));
    commands.push_back(new CUICommand(eCmdAddNamedrRNALarge, "Large", "Add Named rRNA Large", "ANRLarge"));
    commands.push_back(new CUICommand(eCmdAddControlRegion, "Control Region", "Add Control Region", "ACR"));
    commands.push_back(new CUICommand(eCmdAddMicrosatellite, "Microsatellite", "Add Microsatellite", "AM"));
    commands.push_back(new CUICommand(eCmdEditFeatEv, "Evidence", "Edit Feature Evidence", "EFE"));
    commands.push_back(new CUICommand(eCmdEditFeatLoc, "Location", "Edit Feature Location", "EFL"));
    commands.push_back(new CUICommand(eCmdEditFeatLoc_ext, "Edit Feature Location", "Edit Feature Location", "EFL"));
    commands.push_back(new CUICommand(eCmdEditFeatStrand, "Strand", "Edit Feature Location Strand", "EFLS"));
    commands.push_back(new CUICommand(eCmdEditFeatStrand_ext, "Edit Feature Strand", "Edit Feature Location Strand", "EFLS"));
    commands.push_back(new CUICommand(eCmdEditFeatRevInt, "Reverse Location Interval Order", "Reverse Location Interval Order", "RLIO"));
    commands.push_back(new CUICommand(eCmdEditFeatPseudo,"Pseudo", "Edit Feature Pseudo", "EFP"));
    commands.push_back(new CUICommand(eCmdEditFeatException, "Exceptions", "Edit Feature Exception", "EFE"));
    commands.push_back(new CUICommand(eCmdEditFeatExperiment, "Experiment", "Edit Feature Experiment", "EFEX"));
    commands.push_back(new CUICommand(eCmdEditFeatInference, "Inference", "Edit Feature Inference", "EFINF"));
    commands.push_back(new CUICommand(eCmdExtendPartialsConstr, "With Constraint", "Extend Partials With Constraint", "EPWC"));
    commands.push_back(new CUICommand(eCmdExtendPartialsAll, "All to Ends", "Extend Partials All to Ends", "EPAE"));
    commands.push_back(new CUICommand(eCmdGeneFromOtherFeat,"Gene Features from Other Features","Gene Features From Other Features","GFFOF"));
    commands.push_back(new CUICommand(eCmdGeneFeatFromXrefs, "Gene Features from Xrefs", "Gene Features From Xrefs", "GFFX"));
    commands.push_back(new CUICommand(eCmdGeneXrefsFromFeats, "Gene Xrefs from Features", "Gene Xrefs from Features", "GXFF"));
    commands.push_back(new CUICommand(eCmdRemoveAllGeneXrefs, "All Gene Xrefs", "All Gene Xrefs", "RAGX"));
    commands.push_back(new CUICommand(eCmdRemoveUnnecessaryGeneXrefs, "Unnecessary Gene Xrefs", "Unnecessary Gene Xrefs", "RUGX"));
    commands.push_back(new CUICommand(eCmdRemoveNonsuppressingGeneXrefs, "Non-Suppressing Gene Xrefs","Non-Suppressing Gene Xrefs","RNSGX"));  ///?????
    commands.push_back(new CUICommand(eCmdRemoveOrphanLocusGeneXrefs, "Gene Xrefs with Orphan Gene Locus","Gene Xrefs with Orphan Gene Locus","ROLGX"));
    commands.push_back(new CUICommand(eCmdRemoveOrphanLocus_tagGeneXrefs, "Gene Xrefs with Orphan Locus-tag", "Gene Xrefs with Orphan Locus-tag", "ROLTGX")); 
    commands.push_back(new CUICommand(eCmdRemoveGeneXrefs, "Choose Types and Constraint", "Choose Types and Constraint", "RGXTC"));
    commands.push_back(new CUICommand(eCmdRemoveDuplicateGOTerms, "Remove Duplicate GO Terms", "Remove Duplicate GeneOntology Terms", "RDGOT"));
    commands.push_back(new CUICommand(eCmdExtendCDS, "Extend Coding Region Ends", "Extend Coding Region Ends", "ECDSE"));
    commands.push_back(new CUICommand(eCmdTruncateCDS, "Truncate Proteins and CDSs at Stops", "Truncate Proteins and CDSs at Stops", "TPCDSS"));
    commands.push_back(new CUICommand(eCmdExtendCDSToStop, "Extend Proteins to Stops", "Extend Proteins to Stops", "EPROTS"));
    commands.push_back(new CUICommand(eCmdRecomputeIntervalsAndUpdateGenes, "And Update Genes", "And Update Genes", "RIUPG"));
    commands.push_back(new CUICommand(eCmdRecomputeIntervals, "Do Not Update Genes" , "Do Not Update Genes", "RINUPG"));
    commands.push_back(new CUICommand(eCmdCdsFromGeneMrnaExon, "CDS Features from Gene, mRNA or Exon", "CDS Features from Gene, mRNA, or exon", "CDSFME"));
    commands.push_back(new CUICommand(eCmdmRNAFromGeneCdsExon, "mRNA Features from Gene, CDS, or exon", "mRNA Features from Gene, CDS, or exon", "MRNAFGCE"));
    commands.push_back(new CUICommand(eCmdtRNAFromGene, "tRNA/rRNA features from Gene", "tRNA/rRNA features from Gene",""));
    commands.push_back(new CUICommand(eCmdExonFromCds,"CDS Intervals", "Exon Features from CDS Intervals", "EFCDS"));
    commands.push_back(new CUICommand(eCmdExonFromMRNA,"mRNA Intervals", "Exon features from mRNA intervals", "EFMRNA"));
    commands.push_back(new CUICommand(eCmdExonFromTRNA,"tRNA Intervals", "Exon features from tRNA intervals", "EFTRNA"));
    commands.push_back(new CUICommand(eCmdFixProductNamesDefault, "Default","Fix Product Names Default", "FPND"));
    commands.push_back(new CUICommand(eCmdFixProductNamesOptions, "With Options","Fix Product Names With Options", "FPNWO"));
    commands.push_back(new CUICommand(eCmdBulkApplyGenCode, "Batch Apply Genetic Code", "Batch Apply Genetic Code", "BAGC"));
    commands.push_back(new CUICommand(eCmdEditSeqEndsWithAlign,"Edit Sequence Ends with Alignment","Edit Sequence Ends with Alignment", "ESEWA"));
    commands.push_back(new CUICommand(eCmdEditSequenceEnds,"Edit Sequence Ends","Edit Sequence Ends", "ESE"));
    commands.push_back(new CUICommand(eCmdUpdateAlign, "Update Sequence Alignment", "Update SeqAlign", "UPSA"));
    commands.push_back(new CUICommand(eCmdSaveDescriptors, "Save Descriptors", "Save Descriptors", "SD"));
    commands.push_back(new CUICommand(eCmdCreateSeqHistForTpaDetailed, "Create Seq-hist for TPA Detailed", "Create Seq-hist for TPA detailed", "CSHTPAD"));
    commands.push_back(new CUICommand(eCmdCreateSeqHistForTpa, "Create Seq-hist for TPA", "Create Seq-hist for TPA", "CSHTPA"));
    commands.push_back(new CUICommand(eCmdRemoveSeqHistAssembly, "Remove Seq-Hist Assembly", "Remove Seq-Hist Assembly", "RSHA"));
    commands.push_back(new CUICommand(eCmdFarPointerSeq, "Far Pointer Sequences", "Far Pointer Sequences", "FPSEQ"));
    commands.push_back(new CUICommand(eCmdConvertBadCdsAndRnaToMiscFeat, "Convert Bad Coding Regions and RNA features to misc-feat", "Convert Bad Coding Regions and RNA features to misc-feat", "CBCDSRNA"));
    commands.push_back(new CUICommand(eCmdCreateProteinId, "Create Protein IDs", "Create Protein IDs", "CPID"));
    commands.push_back(new CUICommand(eCmdCreateLocusTagGene, "Create Locus-tag Genes", "Create locus-tag genes", "CLTG"));
    commands.push_back(new CUICommand(eCmdNormalizeGeneQuals, "Normalize Gene Quals", "Copy gene qualifiers in xrefs on coding regions to parent genes", "NGQ"));
    commands.push_back(new CUICommand(eCmdGenusSpeciesFixup, "Genus-Species Fixup", "Genus-Species Fixup", "GSFU"));
    commands.push_back(new CUICommand(eCmdCountryConflict, "Lat-Lon Country Conflict Tool", "Lat-Lon Country Conflict Tool", "LLCCT"));
    commands.push_back(new CUICommand(eCmdDuplicate, "Duplicate", "Duplicate", "DUP"));
    commands.push_back(new CUICommand(eCmdCorrectIntervalOrder, "Correct Seq-loc Interval Order", "Correct Seq-loc Interval Order", "CLIO"));
    commands.push_back(new CUICommand(eCmdAddSecondary, "Add Secondary", "Add Secondary", "ADDS"));
    commands.push_back(new CUICommand(eCmdPackageFeaturesOnParts, "Package Features on Parts", "Package Features on Parts", "PFOP"));
    commands.push_back(new CUICommand(eCmdMakeBadSpecificHostTable, "Make Bad Specific-Host Table", "Make Bad Specific-Host Table", "MBSHT"));
    commands.push_back(new CUICommand(eCmdCorrectRNAStrand, "Correct RNA strandedness (Use SMART)", "Correct RNA strandedness", "CRSUS"));
    commands.push_back(new CUICommand(eCmdTrimNsRich, "Trim N-rich Sequence Ends", "Trim N-rich Sequence Ends", "TNR"));
    commands.push_back(new CUICommand(eCmdTrimNsTerminal, "Trim Terminal Ns", "Trim Terminal Ns", "TTN"));
    commands.push_back(new CUICommand(eCmdAddFluComments, "Add Flu Comments", "Add Flu Comments", "AFC"));
    commands.push_back(new CUICommand(eCmdLabelRna, "Label RNA", "Label RNA", "LRNA"));
    commands.push_back(new CUICommand(eCmdRemProtTitles, "All but RefSeq", "All but ReqSeq", "ABRS"));
    commands.push_back(new CUICommand(eCmdRemAllProtTitles, "All", "All", ""));
    commands.push_back(new CUICommand(eCmdApplyRNA_ITS, "Apply rRNA_ITS", "Apply rRNA_ITS", "ARNAITS"));
    commands.push_back(new CUICommand(eCmdEditingButtons, "Toolbar", "Sequin Special Editing Buttons", "SSEB"));
    commands.push_back(new CUICommand(eCmdMakeToolBarWindow, "Make ToolBar Window", "Make ToolBar Window", "MTBW"));
    commands.push_back(new CUICommand(eCmdEnableHupId, "Enable HUP-ID Loader", "Enable HUP-ID Loader", "", "", "", "", wxITEM_CHECK));
    commands.push_back(new CUICommand(eCmdIndexerTypeSelection, "Indexer Type", "Indexer Type", ""));
    commands.push_back(new CUICommand(eCmdTestDialogView, "Open Test DialogView", "Test DialogView", kEmptyStr));
    commands.push_back(new CUICommand(eCmdTestFormView, "Open Test FormView", "Test FormView", kEmptyStr));
    commands.push_back(new CUICommand(eCmdMacroEditor, "Macro Editor", "Macro Editor", "MACROE"));
    commands.push_back(new CUICommand(eCmdSuspectProductRulesEditor, "Suspect Product Rule Editor", "Suspect Product Rule Editor", "SPRE"));
    commands.push_back(new CUICommand(eCmdAdjustConsensusSpliceSitesStrict, "Strict", "Adjust Consensus Splice Sites Strict", "ACSS"));
    commands.push_back(new CUICommand(eCmdAdjustConsensusSpliceSitesStrict_ext, "Adjust CDS for Consensus Splice Sites (strict)", "Adjust Consensus Splice Sites Strict", "ACSS"));
    commands.push_back(new CUICommand(eCmdAdjustConsensusSpliceSitesRelaxed, "Relaxed", "Adjust Consensus Splice Sites Relaxed", "ACSR"));
    commands.push_back(new CUICommand(eCmdAdjustConsensusSpliceSitesRelaxed_ext, "Adjust CDS for Consensus Splice Sites (relaxed)", "Adjust Consensus Splice Sites Relaxed", "ACSR"));
    commands.push_back(new CUICommand(eCmdAdjustConsensusSpliceSitesConsensusEnds, "5' and 3' ends only", "Adjust Consensus Splice Sites Consensus Ends", "ACSENDS"));
    commands.push_back(new CUICommand(eCmdAdjustCDSForIntrons, "CDS", "Adjust CDS For Introns", "ACDSI"));
    commands.push_back(new CUICommand(eCmdAdjustrRNAForIntrons, "rRNA", "Adjust rRNA For Introns", "ARRNAI"));
    commands.push_back(new CUICommand(eCmdAdjusttRNAForIntrons, "tRNA", "Adjust tRNA For Introns", "ATRNAI"));
    commands.push_back(new CUICommand(eCmdAdjustmRNAForIntrons, "mRNA", "Adjust mRNA For Introns", "AMRNAI"));
    commands.push_back(new CUICommand(eCmdLatLonTool, "Lat-Lon Tool", "Lat-Lon Tool", "LLT"));   
    commands.push_back(new CUICommand(eCmdCDSGeneRangeErrorSuppress, "Suppress", "CDS Gene Range Error Suppress", "CDSGRES"));
    commands.push_back(new CUICommand(eCmdCDSGeneRangeErrorRestore, "Restore", "CDS Gene Range Error Restore", "CDSGRER"));
    commands.push_back(new CUICommand(eCmdSuppressGenes, "Suppress Genes on Features", "Suppress Genes on Features", "SUPGF"));
    commands.push_back(new CUICommand(eCmdRemoveSegGaps, "Remove Seg Gaps", "Remove Seg Gaps", "RSEGGAPS"));
    commands.push_back(new CUICommand(eCmdRawSeqToDeltaByNs, "By Ns", "Raw Sequence To Delta By Ns", "RSDN"));
    commands.push_back(new CUICommand(eCmdRawSeqToDeltaByNs_ext,     "Add Assembly Gaps to Sequence", "Add Assembly Gaps to Sequence" , "RSDN"));
    commands.push_back(new CUICommand(eCmdRawSeqToDeltaByLoc, "By Location", "By Location", "RSDL"));
    commands.push_back(new CUICommand(eCmdRawSeqToDeltaByAssemblyGapFeatures, "By Assembly_Gap Features", "By Assembly_Gap Features", ""));
    commands.push_back(new CUICommand(eCmdAdjustFeaturesForGaps, "Adjust Features for Gaps", "Adjust Features for Gaps", "AFFG"));
    commands.push_back(new CUICommand(eCmdDeltaSeqToRaw, "Delta Seq to Raw", "Delta Seq To Raw", "DSTR"));
    commands.push_back(new CUICommand(eCmdDeltaSeqToRaw_ext, "Remove Gap Features", "Remove Gap Features", "DSTR"));
    commands.push_back(new CUICommand(eCmdExpandGaps, "Expand Known Gaps to Include Flanking Ns", "Expand Known Gaps to Include Flanking Ns", "EKGFN"));
    commands.push_back(new CUICommand(eCmdConvertSelectedGapsToKnown, "To Known Length", "To Known Length", "CSGTK"));
    commands.push_back(new CUICommand(eCmdConvertSelectedGapsToUnknown, "To Unknown Length", "To Unknown Length", "CSGTU"));
    commands.push_back(new CUICommand(eCmdConvertGapsBySize, "Convert Known to Unknown (by Size)", "Convert Known to Unknown (by Size)", "CKTUS"));
    commands.push_back(new CUICommand(eCmdEditSelectedGaps, "Change Length of Selected Known Length Gaps", "Change Length of Selected Known Length Gaps", "CLSKG"));
    commands.push_back(new CUICommand(eCmdAddLinkageToGaps, "Add Linkage to All Gaps", "Add Linkage to All Gaps", "ALAG"));
    commands.push_back(new CUICommand(eCmdAddLinkageToGaps_ext, "Add Linkage Evidence to All Gaps", "Add Linkage to All Gaps", "ALAG"));
    commands.push_back(new CUICommand(eCmdCombineAdjacentGaps, "Combine Adjacent Gaps", "Combine Adjacent Gaps", "CAGAP"));
    commands.push_back(new CUICommand(eCmdSplitCDSwithTooManyXs, "Split CDS features with more than 50% N's", "Split CDS features with more than 50% N's", "SCDSN"));
    commands.push_back(new CUICommand(eCmdRmCultureNotes, "Remove Uncultured Notes", "Remove Uncultured Notes", "RUNN"));
    commands.push_back(new CUICommand(eCmdEditSeqId, "Edit Sequence IDs", "Edit Sequence IDs", "ESID"));
    commands.push_back(new CUICommand(eCmdRemoveLocalSeqIdsFromNuc, "From Nucleotides", "Remove Local Seq-id from Nucleotides", "RLIDN"));
    commands.push_back(new CUICommand(eCmdRemoveLocalSeqIdsFromProt, "From Proteins", "Remove Local Seq-id from Proteins", "RLIDP"));
    commands.push_back(new CUICommand(eCmdRemoveLocalSeqIdsFromAll, "From All Bioseqs", "Remove Local Seq-id from All Bioseqs", "RLIDA"));
    commands.push_back(new CUICommand(eCmdRemoveGiSeqIdsFromAll, "GI IDs from Bioseqs", "GI IDs from Bioseqs", "RGIB"));
    commands.push_back(new CUICommand(eCmdRemoveGenbankSeqIdsFromProt, "From Proteins",  "From Proteins", "RGBP"));
    commands.push_back(new CUICommand(eCmdRemoveGenbankSeqIdsFromAll, "From All Bioseqs", "From All Bioseqs", "RGBA"));
    commands.push_back(new CUICommand(eCmdRemoveSeqIdNamesFromProtFeats, "Seq-ID Name From Prot Feats", "Seq-ID Name From Prot Feats", "RSINP"));
    commands.push_back(new CUICommand(eCmdConvertAccessionToLocalIdsAll, "For All Sequences", "For All Sequences", "CACCLA"));   
    commands.push_back(new CUICommand(eCmdConvertAccessionToLocalIdsNuc, "For Nucleotide Sequences", "For Nucleotide Sequences", "CACCLN"));
    commands.push_back(new CUICommand(eCmdConvertAccessionToLocalIdsProt, "For Protein Sequences", "For Protein Sequences", "CACCLP"));
    commands.push_back(new CUICommand(eCmdConvertAccessionToLocalIdsName, "GenBank.name to LocalID", "GenBank.name to LocalID", "CNAML"));
    commands.push_back(new CUICommand(eCmdLocalToGeneralId, "LocalIDs to GeneralIDs", "Local Seq-id to General Seq-id", "LIDGID"));
    commands.push_back(new CUICommand(eCmdGeneralToLocalId, "GeneralIDs to LocalIDs", "GeneralIDs to LocalIDs", "GIDTOL"));
    commands.push_back(new CUICommand(eCmdRemoveGeneralId, "General IDs", "General IDs", "RGID"));
    commands.push_back(new CUICommand(eCmdRemoveUnnecessaryExceptions, "Remove Unnecessary Exceptions", "Remove Unnecessary Exceptions", "RUNE"));
    commands.push_back(new CUICommand(eCmdEditHistory, "Edit History", "Edit History", "EH"));
    commands.push_back(new CUICommand(eCmdUpdate_Replaced_EC_numbers,"Update Replaced EC Numbers", "Update Replaced EC Numbers", "URECN"));
    commands.push_back(new CUICommand(eCmdUpdateSequence, "Single Sequence", "Update Single Sequence", "USSSEQ"));
    commands.push_back(new CUICommand(eCmdUpdateSequence_ext, "Update Sequence", "Update Single Sequence", "USSSEQ"));
    commands.push_back(new CUICommand(eCmdUpdateSequenceClipboard, "Single Sequence (Clipboard)", "Update Single Sequence from Clipboard", "USSCLIP"));
    commands.push_back(new CUICommand(eCmdUpdateSequenceAccession, "Download Accession", "Update Single Sequence Accession", "USSACC"));
    commands.push_back(new CUICommand(eCmdUpdateMultiSequences, "Multiple Sequences", "Update Multiple Sequences from File", "UMSSFILE"));
    commands.push_back(new CUICommand(eCmdUpdateMultiSeqClipboard, "Multiple Sequences (Clipboard)", "Update Multiple Sequences from Clipboard", "UMSSCLIP"));
    commands.push_back(new CUICommand(eCmdImportFeatureTable, "Import Feature Table", "Import Feature Table", "IMPFTBL"));
    commands.push_back(new CUICommand(eCmdImportFeatureTable_ext, "5 Column Feature Table", "Import Feature Table", "IMPFTBL"));
    commands.push_back(new CUICommand(eCmdImportFeatureTableClipboard, "Import Feature Table from Clipboard", "Import Feature Table from Clipboard", "IMPFTBLC"));
    commands.push_back(new CUICommand(eCmdImportGFF3, "Import GFF3 File", "Import GFF3 File", "IMGFF"));
    commands.push_back(new CUICommand(eCmdImportGFF3_ext, "GFF3 File", "Import GFF3 File", "IMGFF"));
    commands.push_back(new CUICommand(eCmdLoadSecondaryAccessions, "Extra Accessions", "Extra Accessions", "LSAEA"));
    commands.push_back(new CUICommand(eCmdLoadSecondaryAccessionsHistoryTakeover, "History Takeover", "History Takeover", "LSAHT"));
    commands.push_back(new CUICommand(eCmdAddGlobalCodeBreak, "Add Global Code Break", "Add Global Code Break", "AGCB"));
    commands.push_back(new CUICommand(eCmdApplyStructuredCommentField, "Apply Structured Comment Field", "Apply Structured Comment Field", "ASTRCMNTFLD"));
    commands.push_back(new CUICommand(eCmdEditStructuredCommentPrefixSuffixHIVData, "HIVData", "Replace Structured Comment to HIVData", "RSTRCMNT1"));
    commands.push_back(new CUICommand(eCmdEditStructuredCommentPrefixSuffixFluData, "FluData", "Replace Structured Comment to FluData", "RSTRCMNT2"));
    commands.push_back(new CUICommand(eCmdEditStructuredCommentPrefixSuffixMIGSData, "MIGSData", "Replace Structured Comment to MIGSData", "RSTRCMNT3"));
    commands.push_back(new CUICommand(eCmdEditStructuredCommentPrefixSuffixMIMSData, "MIMSData", "Replace Structured Comment to MIMSData", "RSTRCMNT4"));
    commands.push_back(new CUICommand(eCmdEditStructuredCommentPrefixSuffixMIENSData, "MIENSData", "Replace Structured Comment to MIENSData", "RSTRCMNT5"));
    commands.push_back(new CUICommand(eCmdEditStructuredCommentPrefixSuffixMIMARKS3_0, "MIMARKS:3.0", "Replace Structured Comment to MIMARKS:3.0", "RSTRCMNT6"));
    commands.push_back(new CUICommand(eCmdEditStructuredCommentPrefixSuffixMIGS3_0, "MIGS:3.0", "Replace Structured Comment to MIGS:3.0", "RSTRCMNT7"));
    commands.push_back(new CUICommand(eCmdEditStructuredCommentPrefixSuffixMIMS3_0, "MIMS:3.0", "Replace Structured Comment to MIMS:3.0", "RSTRCMNT8"));
    commands.push_back(new CUICommand(eCmdEditStructuredCommentPrefixSuffixAssemblyData, "AssemblyData", "Replace Structured Comment to AssemblyData", "RSTRCMNT9"));
    commands.push_back(new CUICommand(eCmdEditStructuredCommentPrefixSuffixGenAssData, "GenomeAssemblyData", "Replace Structured Comment to GAssemblyData", "RSTRCMNT10"));
    commands.push_back(new CUICommand(eCmdEditStructuredCommentPrefixSuffixiBol, "iBOL", "Replace Structured Comment to iBOL", "RSTRCMNT11"));
    commands.push_back(new CUICommand(eCmdLinkSelectedmRNACDS, "Link Selected CDS and mRNA Pair", "Link Selected CDS and mRNA Pair", "LSCDSMRNAP"));
    commands.push_back(new CUICommand(eCmdLinkmRNACDSOverlap, "By Overlap", "Link CDS and mRNA by Overlap", "LCDSMRNAO"));
    commands.push_back(new CUICommand(eCmdLinkmRNACDSProduct, "By Product", "Link CDS and mRNA by Product", "LCDSMRNAP"));
    commands.push_back(new CUICommand(eCmdLinkmRNACDSLabel, "By Label", "Link CDS and mRNA by Label", "LCDSMRNALAB"));
    commands.push_back(new CUICommand(eCmdLinkmRNACDSLabelLocation, "By Label and Location", "Link CDS and mRNA by Label and Location", "LCDSMRNALABLOC"));
    commands.push_back(new CUICommand(eCmdLinkmRNACDSProteinID, "By Protein-ID Qualifier", "Link CDS and mRNA by protein-id qualifier", "LCDSMRNAPRQUAL"));
    commands.push_back(new CUICommand(eCmdLinkmRNACDSTranscriptID, "By Transcript-ID Qualifier", "Link CDS and mRNA by transcript-id qualifier", "LCDSMRNATRQUAL"));
    commands.push_back(new CUICommand(eCmdLinkmRNACDSUnidirectional, "From Unidirectional Xrefs", "Link CDS and mRNA from unidirectional Xrefs", "LSCDSMRNAUNID"));
    commands.push_back(new CUICommand(eCmdShowLinkedmRNACDS, "Show Linked CDS or mRNA Feature", "Show Linked CDS or mRNA Feature", "SLINKCDSMRNA"));
    commands.push_back(new CUICommand(eCmdShowFeatureTable, "Show Feature Table", "Show Feature Table", "SFEATTBL"));
    commands.push_back(new CUICommand(eCmdShowFeatureTable_ext, "Feature Table", "Show Feature Table", "SFEATTBL"));
    commands.push_back(new CUICommand(eCmdRestoreFromFile, "Restore from File", "Restore from Local File", "RFFILE"));
    commands.push_back(new CUICommand(eCmdRestoreAndConvertSeqSubmit, "Restore and Convert SeqSubmit", "Restore and Convert SeqSubmit", "RCVFFILE"));
    commands.push_back(new CUICommand(eCmdViewSortedProteins, "View Sorted Proteins (FASTA)", "View Sorted Proteins (FASTA)", "VSPF"));
    commands.push_back(new CUICommand(eCmdCheckForMixedStrands, "Check for Mixed Strands", "Check for Mixed Strands", "CMSTRAND"));
    commands.push_back(new CUICommand(eCmdPrepareSeqSubmit, "Genome Submission Wizard", "Prepare a Submission", "PREPASUB"));
    commands.push_back(new CUICommand(eCmdSaveSubmissionFile, "Save Submission File", "Save Submission File", "SSUBF"));
    commands.push_back(new CUICommand(eCmdSaveASN1File, "Save ASN.1 File", "Save ASN.1 File", "SVASN"));
    commands.push_back(new CUICommand(eCmdHelpReports_ext, "Help", "Help Reports", "HELPR"));
    commands.push_back(new CUICommand(eCmdHelpImport_ext, "Help", "Help Import", "HELPI"));
    commands.push_back(new CUICommand(eCmdHelpSequences_ext, "Help", "Help Sequences", "HELPS"));
    commands.push_back(new CUICommand(eCmdHelpFeatures_ext, "Help", "Help Features", "HELPF"));
    commands.push_back(new CUICommand(eCmdHelpComments_ext, "Help", "Help Comments", "HELPC"));
    commands.push_back(new CUICommand(eCmdHelpEditingTools_ext, "Help", "Help Editing Tools", "HELPET"));
    

    CBioseqEditorFactory::CollectBioseqEditorCommands(commands);

    commands.push_back(new CUICommand(eCmdAddDefLine, "Add Definition Line", "Add Definition Line", "ADL"));
    commands.push_back(new CUICommand(eCmdPrefixOrgToDefLine, "Organism", "Prefix Defline With Organism", "PDLORG"));
    commands.push_back(new CUICommand(eCmdPrefixStrainToDefLine, "Strain", "Prefix Defline With Strain", "PDLSTR"));
    commands.push_back(new CUICommand(eCmdPrefixCloneToDefLine, "Clone", "Prefix Defline With Clone", "PDLCLO"));
    commands.push_back(new CUICommand(eCmdPrefixIsolateToDefLine, "Isolate", "Prefix Defline With Isolate", "PDLISO"));
    commands.push_back(new CUICommand(eCmdPrefixHaplotToDefLine, "Haplotype", "Prefix Defline With Haplotype", "PDLH"));
    commands.push_back(new CUICommand(eCmdPrefixCultivarToDefLine, "Cultivar", "Prefix Defline With Cultivar", "PDLCULT"));
    commands.push_back(new CUICommand(eCmdPrefixListToDefLine, "List", "Prefix Defline From List", "PDLLIST"));
    
    // annotate descriptors commands
    commands.push_back(new CUICommand(eCmdCreateDescTPAAssembly, "TPA Assembly", "TPA Assembly", "TPAA"));
    commands.push_back(new CUICommand(eCmdCreateDescStructuredComment, "Structured Comment", "Structured Comment", "SCD"));
    commands.push_back(new CUICommand(eCmdCreateDescRefGeneTracking, "RefGene Tracking", "RefGene Tracking", "RGT"));
    commands.push_back(new CUICommand(eCmdCreateDescDBLink, "DBLink", "DBLink", "DBLD"));
    commands.push_back(new CUICommand(eCmdCreateDescUnverified, "Unverified", "Unverified", "UVD"));
    commands.push_back(new CUICommand(eCmdCreateDescAuthorizedAccess, "Authorized Access", "Authorized Access", "AAD"));
    commands.push_back(new CUICommand(eCmdCreateDescGenomeAssemblyComment, "Genome Assembly", "Genome Assembly", "GAD"));
    commands.push_back(new CUICommand(eCmdCreateDescGenBankBlock, "GenBank Block", "GenBank Block", "GBBD"));
    commands.push_back(new CUICommand(eCmdCreateDescComment, "Comment", "Comment", "CD"));
    commands.push_back(new CUICommand(eCmdCreateDescComment_ext, "Add Free Text Comment", "Comment", "CD"));
    commands.push_back(new CUICommand(eCmdCreateDescTitle, "Title", "Title", "TD"));
    commands.push_back(new CUICommand(eCmdCreateDescMolinfo, "Molecule Description", "Molinfo", "MIDD"));
    commands.push_back(new CUICommand(eCmdCreateDescSource, "Biological Source", "BioSource", "BSD"));
    commands.push_back(new CUICommand(eCmdCreateDescPub, "Publication Descriptor", "Publication", "PD"));
    commands.push_back(new CUICommand(eCmdCreateDescPub_ext, "Add Publication", "Publication", "PD"));
    commands.push_back(new CUICommand(eCmdSubmitter, "Submitter Report", "Submitter Report", "SDR"));
    
    commands.push_back(new CUICommand(eCmdCreateStructuredCommentPrefixSuffixMIMS, "Add", "MIMS Comment", "AMIMC"));
    commands.push_back(new CUICommand(eCmdCreateStructuredCommentPrefixSuffixMIGS, "Add", "MIGS Comment", "AMIGC"));
    commands.push_back(new CUICommand(eCmdCreateStructuredCommentPrefixSuffixMIMARKS, "Add", "MIMARKS Comment", "AMIMARC"));
    commands.push_back(new CUICommand(eCmdCreateStructuredCommentPrefixSuffixMIENSData, "Add", "MIENS Comment", "AMIENC"));
    commands.push_back(new CUICommand(eCmdCreateStructuredCommentPrefixSuffixGenAnnData, "Add", "Genome Annotation Comment", "AGANNC"));
    commands.push_back(new CUICommand(eCmdCreateStructuredCommentPrefixSuffixGenAssData, "Add", "Genome Assembly Comment", "AGASSC"));
    commands.push_back(new CUICommand(eCmdDeleteStructuredCommentPrefixSuffixMIMS, "Remove", "MIMS Comment", "RMIMC"));
    commands.push_back(new CUICommand(eCmdDeleteStructuredCommentPrefixSuffixMIGS, "Remove", "MIGS Comment", "RMIGC"));
    commands.push_back(new CUICommand(eCmdDeleteStructuredCommentPrefixSuffixMIMARKS, "Remove", "MIMARKS Comment", "RMIMARC"));
    commands.push_back(new CUICommand(eCmdDeleteStructuredCommentPrefixSuffixMIENSData, "Remove", "MIENS Comment", "RMIENC"));
    commands.push_back(new CUICommand(eCmdDeleteStructuredCommentPrefixSuffixGenAnnData, "Remove", "Genome Annotation Comment", "RGANNC"));
    commands.push_back(new CUICommand(eCmdDeleteStructuredCommentPrefixSuffixGenAssData, "Remove", "Genome Assembly Comment", "RGASSC"));

    return commands;
}

BEGIN_EVENT_TABLE(CSequenceEditingEventHandler, wxEvtHandler)
    EVT_MENU( eCmdRemoveDescriptors, CSequenceEditingEventHandler::RemoveDescriptors )
    EVT_MENU( eCmdRemoveFeatures, CSequenceEditingEventHandler::RemoveFeatures )
    EVT_MENU( eCmdFixForTransSplicing, CSequenceEditingEventHandler::FixForTransSplicing )
    EVT_MENU( eCmdRemoveDuplicateGOTerms, CSequenceEditingEventHandler::RemoveDuplicateGoTerms)
    EVT_MENU( eCmdRemoveUnindexedFeatures, CSequenceEditingEventHandler::RemoveUnindexedFeatures )
    EVT_MENU( eCmdApplyIndexerComments, CSequenceEditingEventHandler::ApplyIndexerComments )
    EVT_MENU( eCmdRemoveSeqAnnotIds, CSequenceEditingEventHandler::RemoveSeqAnnotIds )
    EVT_MENU( eCmdRemoveCddFeatDbxref, CSequenceEditingEventHandler::RemoveCddFeatDbxref )
    EVT_MENU( eCmdRemoveBankitComment, CSequenceEditingEventHandler::RemoveBankitComment )
    EVT_MENU( eCmdRemoveDuplicateStructuredComments, CSequenceEditingEventHandler::RemoveDuplicateStructuredComments )
    EVT_MENU( eCmdRemoveSequencesFromAlignments, CSequenceEditingEventHandler::RemoveSequencesFromAlignments )
    EVT_MENU( eCmdRemoveSequences, CSequenceEditingEventHandler::RemoveSequences )
    EVT_MENU( eCmdRemoveSequences_ext, CSequenceEditingEventHandler::RemoveSequences )
    EVT_MENU( eCmdJustRemoveProteins, CSequenceEditingEventHandler::JustRemoveProteins )
    EVT_MENU( eCmdRemoveProteinsAndRenormalizeNucProtSets, CSequenceEditingEventHandler::RemoveProteinsAndRenormalizeNucProtSets )
    EVT_MENU( eCmdRemoveOrphanedProteins, CSequenceEditingEventHandler::RemoveOrphanedProteins )
    EVT_MENU( eCmdRemoveSet, CSequenceEditingEventHandler::RemoveSet)
    EVT_MENU( eCmdRemoveSetsFromSet, CSequenceEditingEventHandler::RemoveSetsFromSet)
    EVT_MENU( eCmdRevCompSequences, CSequenceEditingEventHandler::RevCompSequences )
    EVT_MENU( eCmdRevCompSequences_ext, CSequenceEditingEventHandler::RevCompSequences )
    EVT_MENU( eCmdBioseqFeatsRevComp, CSequenceEditingEventHandler::BioseqFeatsRevComp)
    EVT_MENU( eCmdBioseqOnlyRevComp, CSequenceEditingEventHandler::BioseqOnlyRevComp)
    EVT_MENU( eCmdUnculTaxTool, CSequenceEditingEventHandler::UnculTaxTool )
    EVT_MENU( eCmdMergeBiosources, CSequenceEditingEventHandler::MergeBiosources)
    EVT_MENU( eCmdCompareWithBiosample, CSequenceEditingEventHandler::CompareWithBiosample)
    EVT_MENU( eCmdCompareWithBiosampleFirstOnly, CSequenceEditingEventHandler::CompareWithBiosampleFirstOnly)
    EVT_MENU( eCmdConsolidateLikeModsWithSemicolon, CSequenceEditingEventHandler::ConsolidateLikeModsWithSemicolon)
    EVT_MENU( eCmdConsolidateLikeModsWithoutSemicolon, CSequenceEditingEventHandler::ConsolidateLikeModsWithoutSemicolon)
    EVT_MENU( eCmdBarcodeTool, CSequenceEditingEventHandler::BarcodeTool)
    EVT_MENU( eCmdApplyFBOL, CSequenceEditingEventHandler::ApplyFBOL)
    EVT_MENU( eCmdEditSequence, CSequenceEditingEventHandler::EditSequence )
    EVT_MENU( eCmdAlignmentAssistant, CSequenceEditingEventHandler::AlignmentAssistant )
    EVT_MENU( eCmdRemoveAlignments, CSequenceEditingEventHandler::RemoveSeqAnnotAlign )
    EVT_MENU( eCmdRemoveGraphs, CSequenceEditingEventHandler::RemoveSeqAnnotGraph )
    EVT_MENU( eCmdRemoveGenomeProjectId, CSequenceEditingEventHandler::RemoveGenomeProjectsDB )
    EVT_MENU( eCmdRemoveEmptyGenomeProjectId, CSequenceEditingEventHandler::RemoveEmptyGenomeProjectsDB )
    EVT_MENU( eCmdTaxFixCleanup, CSequenceEditingEventHandler::TaxFixCleanup )
    EVT_MENU( eCmdTaxFixCleanup_ext, CSequenceEditingEventHandler::TaxFixCleanup )
    EVT_MENU( eCmdSpecificHostCleanup, CSequenceEditingEventHandler::SpecificHostCleanup )
    EVT_MENU( eCmdExtendedCleanup, CSequenceEditingEventHandler::TaxFixCleanup )
    EVT_MENU( eCmdFixNonReciprocalLinks, CSequenceEditingEventHandler::FixNonReciprocalLinks )
    EVT_MENU( eCmdDisableStrainForwarding, CSequenceEditingEventHandler::DisableStrainForwarding )
    EVT_MENU (eCmdEnableStrainForwarding, CSequenceEditingEventHandler::EnableStrainForwarding)
    EVT_MENU( eCmdEditPubs, CSequenceEditingEventHandler::EditPubs )
    EVT_MENU( eCmdGlobalPubmedIdLookup, CSequenceEditingEventHandler::GlobalPubmedIdLookup )
    EVT_MENU( eCmdFixCapitalizationAll, CSequenceEditingEventHandler::FixCapitalizationAll )
    EVT_MENU( eCmdFixCapitalizationAuthors, CSequenceEditingEventHandler::FixCapitalizationAuthors )
    EVT_MENU( eCmdFixCapitalizationTitles, CSequenceEditingEventHandler::FixCapitalizationTitles )
    EVT_MENU( eCmdFixCapitalizationAffiliation, CSequenceEditingEventHandler::FixCapitalizationAffiliation )
    EVT_MENU( eCmdFixCapitalizationCountry, CSequenceEditingEventHandler::FixCapitalizationCountry )
    EVT_MENU( eCmdRemoveUnpublishedPublications, CSequenceEditingEventHandler::RemoveUnpublishedPublications )
    EVT_MENU( eCmdRemoveInPressPublications, CSequenceEditingEventHandler::RemoveInPressPublications )
    EVT_MENU( eCmdRemovePublishedPublications, CSequenceEditingEventHandler::RemovePublishedPublications )
    EVT_MENU( eCmdRemoveCollidingPublications, CSequenceEditingEventHandler::RemoveCollidingPublications )
    EVT_MENU( eCmdRemoveAllPublications, CSequenceEditingEventHandler::RemoveAllPublications )
    EVT_MENU( eCmdRemoveAuthorConsortiums, CSequenceEditingEventHandler::RemoveAuthorConsortiums )
    EVT_MENU( eCmdReverseAuthorNames, CSequenceEditingEventHandler::ReverseAuthorNames )
    EVT_MENU( eCmdStripAuthorSuffixes, CSequenceEditingEventHandler::StripAuthorSuffixes )
    EVT_MENU( eCmdTruncateAuthorMiddleInitials, CSequenceEditingEventHandler::TruncateAuthorMiddleInitials )
    EVT_MENU( eCmdConvertAuthorToConstortiumWhereLastName, CSequenceEditingEventHandler::ConvertAuthorToConstortiumWhereLastName )
    EVT_MENU( eCmdConvertAuthorToConsortiumAll, CSequenceEditingEventHandler::ConvertAuthorToConsortiumAll )
    EVT_MENU( eCmdAddCitSubForUpdate, CSequenceEditingEventHandler::AddCitSubForUpdate )
    EVT_MENU( eCmdFixUsaAndStates, CSequenceEditingEventHandler::FixUsaAndStates )
    EVT_MENU( eCmdRetranslateCDSObeyStop, CSequenceEditingEventHandler::RetranslateCDS )
    EVT_MENU( eCmdRetranslateCDSIgnoreStopExceptEnd, CSequenceEditingEventHandler::RetranslateCDS )
    EVT_MENU( eCmdRetranslateCDSIgnoreStopExceptEnd_ext, CSequenceEditingEventHandler::RetranslateCDS )
    EVT_MENU( eCmdRetranslateCDSChooseFrame, CSequenceEditingEventHandler::RetranslateCDS )
    EVT_MENU( eCmdEditCdsFrame, CSequenceEditingEventHandler::EditCdsFrame)
    EVT_MENU( eCmdRestoreRNAediting, CSequenceEditingEventHandler::RestoreRNAediting)
    EVT_MENU( eCmdResynchronizePartialsCDS, CSequenceEditingEventHandler::ResynchronizePartials )
    EVT_MENU( eCmdAddTranslExcept, CSequenceEditingEventHandler::AddTranslExcept )
    EVT_MENU( eCmdRemoveAllStructuredComments, CSequenceEditingEventHandler::RemoveAllStructuredComments )
    EVT_MENU( eCmdRemoveEmptyStructuredComments, CSequenceEditingEventHandler::RemoveEmptyStructuredComments )
    EVT_MENU( eCmdModifyStructuredComment, CSequenceEditingEventHandler::ModifyStructuredComment )
    EVT_MENU( eCmdSetGlobalRefGeneStatus, CSequenceEditingEventHandler::SetGlobalRefGeneStatus )
    EVT_MENU( eCmdClearKeywords, CSequenceEditingEventHandler::ClearKeywords )
    EVT_MENU( eCmdClearNomenclature, CSequenceEditingEventHandler::ClearNomenclature )
    EVT_MENU( eCmdConvertToDelayedGenProdSetQuals, CSequenceEditingEventHandler::ConvertToDelayedGenProdSetQuals )
    EVT_MENU( eCmdConvertRptUnitRangeToLoc, CSequenceEditingEventHandler::ConvertRptUnitRangeToLoc )
    EVT_MENU( eCmdRemoveStructuredCommentKeyword, CSequenceEditingEventHandler::RemoveStructuredCommentKeyword )
    EVT_MENU( eCmdAddStructuredCommentKeyword, CSequenceEditingEventHandler::AddStructuredCommentKeyword )
    EVT_MENU( eCmdAddKeywordGDS, CSequenceEditingEventHandler::AddKeywordGDS )
    EVT_MENU( eCmdAddKeywordTPA_inferential, CSequenceEditingEventHandler::AddKeywordTPA_inferential )
    EVT_MENU( eCmdAddKeywordTPA_experimental, CSequenceEditingEventHandler::AddKeywordTPA_experimental )
    EVT_MENU( eCmdAddKeywordTPA_assembly, CSequenceEditingEventHandler::AddKeywordTPA_assembly )
    EVT_MENU( eCmdAddKeywordWithConstraint, CSequenceEditingEventHandler::AddKeywordWithConstraint )
    EVT_MENU( eCmdAddGenomeAssemblyStructuredComment, CSequenceEditingEventHandler::AddGenomeAssemblyStructuredComment )
    EVT_MENU( eCmdAddAssemblyStructuredComment, CSequenceEditingEventHandler::AddAssemblyStructuredComment )
    EVT_MENU( eCmdReorderStructuredComment, CSequenceEditingEventHandler::ReorderStructuredComment )
    EVT_MENU( eCmdConvertComment, CSequenceEditingEventHandler::ConvertComment )
    EVT_MENU( eCmdParseComment, CSequenceEditingEventHandler::ParseComment )
    EVT_MENU( eCmdExportStructuredCommentsTable, CSequenceEditingEventHandler::ExportStructuredCommentsTable)
    EVT_MENU( eCmdLoadStructComments, CSequenceEditingEventHandler::LoadStructComments)
    EVT_MENU( eCmdIllegalQualsToNote, CSequenceEditingEventHandler::IllegalQualsToNote )
    EVT_MENU( eCmdRmIllegalQuals, CSequenceEditingEventHandler::RmIllegalQuals )
    EVT_MENU( eCmdWrongQualsToNote, CSequenceEditingEventHandler::WrongQualsToNote )
    EVT_MENU( eCmdConvertBadInference, CSequenceEditingEventHandler::ConvertBadInference)
    EVT_MENU( eCmdRmWrongQuals, CSequenceEditingEventHandler::RmWrongQuals )
    EVT_MENU( eCmdRemoveDbXrefsCDS, CSequenceEditingEventHandler::RemoveDbXrefsCDS )
    EVT_MENU( eCmdRemoveDbXrefsGenes, CSequenceEditingEventHandler::RemoveDbXrefsGenes )
    EVT_MENU( eCmdRemoveDbXrefsRNA, CSequenceEditingEventHandler::RemoveDbXrefsRNA )
    EVT_MENU( eCmdRemoveDbXrefsAllFeats, CSequenceEditingEventHandler::RemoveDbXrefsAllFeats )
    EVT_MENU( eCmdRemoveDbXrefsBioSource, CSequenceEditingEventHandler::RemoveDbXrefsBioSource )
    EVT_MENU( eCmdRemoveDbXrefsBioSourceAndFeats, CSequenceEditingEventHandler::RemoveDbXrefsBioSourceAndFeats )
    EVT_MENU( eCmdRemoveTaxonFeats, CSequenceEditingEventHandler::RemoveTaxonFeats)
    EVT_MENU( eCmdRemoveTaxonFeatsAndBioSource, CSequenceEditingEventHandler::RemoveTaxonFeatsAndBioSource)
    EVT_MENU( eCmdGroupExplode, CSequenceEditingEventHandler::GroupExplode )
    EVT_MENU( eCmdFindASN1, CSequenceEditingEventHandler::FindASN1 )
    EVT_MENU( eCmdFuseFeatures, CSequenceEditingEventHandler::FuseFeatures )
    EVT_MENU( eCmdFuseJoinsInLocs, CSequenceEditingEventHandler::FuseJoinsInLocs )
    EVT_MENU( eCmdExplodeRNAFeats, CSequenceEditingEventHandler::ExplodeRNAFeats )
    EVT_MENU( eCmdAssignFeatureIds, CSequenceEditingEventHandler::AssignFeatureIds )
    EVT_MENU( eCmdClearFeatureIds, CSequenceEditingEventHandler::ClearFeatureIds )
    EVT_MENU( eCmdReassignFeatureIds, CSequenceEditingEventHandler::ReassignFeatureIds )
    EVT_MENU( eCmdUniqifyFeatureIds, CSequenceEditingEventHandler::UniqifyFeatureIds )
    EVT_MENU( eCmdTrimJunkInPrimerSeqs, CSequenceEditingEventHandler::TrimJunkInPrimerSeqs )
    EVT_MENU( eCmdFixiInPrimerSeqs, CSequenceEditingEventHandler::FixiInPrimerSeqs )
    EVT_MENU( eCmdPrefixAuthWithTax, CSequenceEditingEventHandler::PrefixAuthWithTax)
    EVT_MENU( eCmdFocusSet, CSequenceEditingEventHandler::FocusSet )
    EVT_MENU( eCmdFocusClear, CSequenceEditingEventHandler::FocusClear )
    EVT_MENU( eCmdSetTransgSrcDesc, CSequenceEditingEventHandler::SetTransgSrcDesc )
    EVT_MENU( eCmdSplitDblinkQuals, CSequenceEditingEventHandler::SplitDblinkQuals )
    EVT_MENU( eCmdFixOrgModInstitution, CSequenceEditingEventHandler::FixOrgModInstitution )
    EVT_MENU( eCmdFixStructuredVouchers, CSequenceEditingEventHandler::FixStructuredVouchers )
    EVT_MENU( eCmdSwapPrimerSeqName, CSequenceEditingEventHandler::SwapPrimerSeqName )
    EVT_MENU( eCmdMergePrimerSets, CSequenceEditingEventHandler::MergePrimerSets )
    EVT_MENU( eCmdSplitPrimerSets, CSequenceEditingEventHandler::SplitPrimerSets )
    EVT_MENU( eCmdAppendModToOrg, CSequenceEditingEventHandler::AppendModToOrg )
    EVT_MENU( eCmdParseCollectionDateMonthFirst, CSequenceEditingEventHandler::ParseCollectionDateMonthFirst )
    EVT_MENU( eCmdParseCollectionDateDayFirst, CSequenceEditingEventHandler::ParseCollectionDateDayFirst )
    EVT_MENU( eCmdCountryFixupCap, CSequenceEditingEventHandler::CountryFixupCap )
    EVT_MENU( eCmdCountryFixupNoCap, CSequenceEditingEventHandler::CountryFixupNoCap )
    EVT_MENU( eCmdVectorTrim, CSequenceEditingEventHandler::VectorTrim )
    EVT_MENU( eCmdSelectTarget, CSequenceEditingEventHandler::SelectTarget )
    EVT_MENU( eCmdSelectTarget_ext, CSequenceEditingEventHandler::SelectTarget )
    EVT_MENU( eCmdSelectFeature, CSequenceEditingEventHandler::SelectFeature )
    EVT_MENU( eCmdApplyCDS, CSequenceEditingEventHandler::AddCDS )
    EVT_MENU( eCmdApplyAddRNA, CSequenceEditingEventHandler::AddRNA )
    EVT_MENU( eCmdApplyAddOther, CSequenceEditingEventHandler::AddOtherFeature )
    EVT_MENU( eCmdApplyAddOther_ext, CSequenceEditingEventHandler::AddOtherFeature )
    EVT_MENU( eCmdAddFeatureBetween, CSequenceEditingEventHandler::AddFeatureBetween )
    EVT_MENU( eCmdValidate, CSequenceEditingEventHandler::Validate )
    EVT_MENU( eCmdValidate_ext, CSequenceEditingEventHandler::Validate)
    EVT_MENU( eCmdValidateNoAlignments, CSequenceEditingEventHandler::Validate )
    EVT_MENU( eCmdValidateCheckInferences, CSequenceEditingEventHandler::Validate )
    EVT_MENU( eCmdSortUniqueCountText, CSequenceEditingEventHandler::SortUniqueCount )
    EVT_MENU( eCmdSortUniqueCountTree, CSequenceEditingEventHandler::SortUniqueCountTree )
    EVT_MENU( eCmdSortUniqueCountTree_ext, CSequenceEditingEventHandler::SortUniqueCountTree )
    EVT_MENU( eCmdDiscrepancy, CSequenceEditingEventHandler::Discrepancy)
    EVT_MENU( eCmdOncaller, CSequenceEditingEventHandler::Oncaller)
    EVT_MENU( eCmdSubmitter, CSequenceEditingEventHandler::Submitter)
    EVT_MENU( eCmdSubmitter_ext, CSequenceEditingEventHandler::Submitter)
    EVT_MENU( eCmdMega, CSequenceEditingEventHandler::Mega)
    EVT_MENU( eCmdDiscrepancyList, CSequenceEditingEventHandler::DiscrepancyList)
    EVT_MENU( eCmdAutodefDefaultOptions, CSequenceEditingEventHandler::Autodef)
    EVT_MENU( eCmdAutodefOptions, CSequenceEditingEventHandler::Autodef )
    EVT_MENU( eCmdAutodefMisc, CSequenceEditingEventHandler::Autodef )
    EVT_MENU( eCmdAutodefId, CSequenceEditingEventHandler::Autodef )
    EVT_MENU( eCmdAutodefNoMods, CSequenceEditingEventHandler::Autodef )
    EVT_MENU( eCmdAutodefPopset, CSequenceEditingEventHandler::Autodef )
    EVT_MENU( eCmdAutodefRefresh, CSequenceEditingEventHandler::Autodef)
    EVT_MENU( eCmdInstantiateProteinTitles, CSequenceEditingEventHandler::PT_Cleanup )
    EVT_MENU( eCmdRemoveAllFeatures, CSequenceEditingEventHandler::RemoveAllFeatures )
    EVT_MENU( eCmdRemoveDupFeats, CSequenceEditingEventHandler::RemoveDupFeats )
    EVT_MENU( eCmdRemoveDupFeats_ext, CSequenceEditingEventHandler::RemoveDupFeats )
    EVT_MENU( eCmdRemoveDupFeatsWithOptions, CSequenceEditingEventHandler::RemoveDupFeatsWithOptions )
    EVT_MENU( eCmdRemoveDupFeatsWithOptions_ext, CSequenceEditingEventHandler::RemoveDupFeatsWithOptions )
    EVT_MENU( eCmdResolveIntersectingFeats, CSequenceEditingEventHandler::ResolveIntersectingFeats )
    EVT_MENU( eCmdAddSequences, CSequenceEditingEventHandler::AddSequences )
    EVT_MENU (eCmdApplySourceQual, CSequenceEditingEventHandler::ApplyEditConvertRemove )
    EVT_MENU (eCmdEditSourceQual, CSequenceEditingEventHandler::ApplyEditConvertRemove )
    EVT_MENU (eCmdConvertSourceQual, CSequenceEditingEventHandler::ApplyEditConvertRemove )
    EVT_MENU (eCmdSwapSourceQual, CSequenceEditingEventHandler::ApplyEditConvertRemove )
    EVT_MENU (eCmdRemoveSourceQual, CSequenceEditingEventHandler::ApplyEditConvertRemove )
    EVT_MENU (eCmdApplyRNAQual, CSequenceEditingEventHandler::ApplyEditConvertRemove )
    EVT_MENU (eCmdEditRNAQual, CSequenceEditingEventHandler::ApplyEditConvertRemove )
    EVT_MENU (eCmdConvertRNAQual, CSequenceEditingEventHandler::ApplyEditConvertRemove )
    EVT_MENU (eCmdSwapRNAQual, CSequenceEditingEventHandler::ApplyEditConvertRemove )
    EVT_MENU (eCmdRemoveRNAQual, CSequenceEditingEventHandler::ApplyEditConvertRemove )
    EVT_MENU (eCmdApplyCGPQual, CSequenceEditingEventHandler::ApplyEditConvertRemove )
    EVT_MENU (eCmdEditCGPQual, CSequenceEditingEventHandler::ApplyEditConvertRemove )
    EVT_MENU (eCmdConvertCGPQual, CSequenceEditingEventHandler::ApplyEditConvertRemove )
    EVT_MENU (eCmdSwapCGPQual, CSequenceEditingEventHandler::ApplyEditConvertRemove )
    EVT_MENU (eCmdRemoveCGPQual, CSequenceEditingEventHandler::ApplyEditConvertRemove )
    EVT_MENU (eCmdApplyFeatQual, CSequenceEditingEventHandler::ApplyEditConvertRemove )
    EVT_MENU (eCmdEditFeatQual, CSequenceEditingEventHandler::ApplyEditConvertRemove )
    EVT_MENU (eCmdConvertFeatQual, CSequenceEditingEventHandler::ApplyEditConvertRemove )
    EVT_MENU (eCmdConvertSecondProtNameToDesc, CSequenceEditingEventHandler::ConvertSecondProtNameToDesc)
    EVT_MENU (eCmdConvertDescToSecondProtName, CSequenceEditingEventHandler::ConvertDescToSecondProtName)
    EVT_MENU (eCmdConvertDescToFeatComment, CSequenceEditingEventHandler::ConvertDescToFeatComment)
    EVT_MENU (eCmdConvertDescToFeatSource, CSequenceEditingEventHandler::ConvertDescToFeatSource)
    EVT_MENU (eCmdConvertDescToFeatPub, CSequenceEditingEventHandler::ConvertDescToFeatPub)
    EVT_MENU (eCmdConvertDescToFeatPubConstraint, CSequenceEditingEventHandler::ConvertDescToFeatPubConstraint)
    EVT_MENU (eCmdConvertFeatToDescComment, CSequenceEditingEventHandler::ConvertFeatToDescComment)
    EVT_MENU (eCmdConvertFeatToDescSource, CSequenceEditingEventHandler::ConvertFeatToDescSource)
    EVT_MENU (eCmdConvertFeatToDescPub, CSequenceEditingEventHandler::ConvertFeatToDescPub)
    EVT_MENU (eCmdSwapFeatQual, CSequenceEditingEventHandler::ApplyEditConvertRemove )
    EVT_MENU (eCmdRemoveFeatQual, CSequenceEditingEventHandler::ApplyEditConvertRemove )
    EVT_MENU (eCmdRemoveTextOutsideString, CSequenceEditingEventHandler::ApplyEditConvertRemove )
    EVT_MENU (eCmdEditFeatEv, CSequenceEditingEventHandler::EditFeature )
    EVT_MENU (eCmdEditFeatLoc, CSequenceEditingEventHandler::EditFeature )
    EVT_MENU (eCmdEditFeatLoc_ext, CSequenceEditingEventHandler::EditFeature )
    EVT_MENU (eCmdEditFeatStrand, CSequenceEditingEventHandler::EditFeature )
    EVT_MENU (eCmdEditFeatStrand_ext, CSequenceEditingEventHandler::EditFeature )
    EVT_MENU (eCmdEditFeatRevInt, CSequenceEditingEventHandler::EditFeature )
    EVT_MENU (eCmdEditFeatPseudo, CSequenceEditingEventHandler::EditFeature )
    EVT_MENU (eCmdEditFeatException, CSequenceEditingEventHandler::EditFeature )
    EVT_MENU (eCmdEditFeatExperiment, CSequenceEditingEventHandler::EditFeature )
    EVT_MENU (eCmdEditFeatInference, CSequenceEditingEventHandler::EditFeature )
    EVT_MENU (eCmdExtendPartialsConstr, CSequenceEditingEventHandler::ExtendPartialsConstr )
    EVT_MENU (eCmdExtendPartialsAll, CSequenceEditingEventHandler::ExtendPartialsAll )
    EVT_MENU (eCmdGeneFromOtherFeat, CSequenceEditingEventHandler::GeneFromOtherFeat )
    EVT_MENU (eCmdGeneFeatFromXrefs, CSequenceEditingEventHandler::GeneFeatFromXrefs )
    EVT_MENU (eCmdGeneXrefsFromFeats, CSequenceEditingEventHandler::GeneXrefsFromFeats)
    EVT_MENU (eCmdRemoveAllGeneXrefs, CSequenceEditingEventHandler::RemoveAllGeneXrefs )
    EVT_MENU (eCmdRemoveUnnecessaryGeneXrefs, CSequenceEditingEventHandler::RemoveUnnecessaryGeneXrefs )
    EVT_MENU (eCmdRemoveNonsuppressingGeneXrefs, CSequenceEditingEventHandler::RemoveNonsuppressingGeneXrefs )
    EVT_MENU (eCmdRemoveOrphanLocusGeneXrefs, CSequenceEditingEventHandler::RemoveOrphanLocusGeneXrefs )
    EVT_MENU (eCmdRemoveOrphanLocus_tagGeneXrefs, CSequenceEditingEventHandler::RemoveOrphanLocus_tagGeneXrefs )
    EVT_MENU (eCmdRemoveGeneXrefs, CSequenceEditingEventHandler::RemoveGeneXrefs )
    EVT_MENU (eCmdExtendCDS, CSequenceEditingEventHandler::ExtendCDS )
    EVT_MENU (eCmdTruncateCDS, CSequenceEditingEventHandler::TruncateCDS )
    EVT_MENU (eCmdExtendCDSToStop, CSequenceEditingEventHandler::ExtendCDSToStop)
    EVT_MENU (eCmdRecomputeIntervalsAndUpdateGenes, CSequenceEditingEventHandler::RecomputeIntervalsAndUpdateGenes)
    EVT_MENU (eCmdRecomputeIntervals, CSequenceEditingEventHandler::RecomputeIntervals)
    EVT_MENU (eCmdCdsFromGeneMrnaExon, CSequenceEditingEventHandler::CdsFromGeneMrnaExon )
    EVT_MENU (eCmdmRNAFromGeneCdsExon, CSequenceEditingEventHandler::mRNAFromGeneCdsExon)
    EVT_MENU (eCmdtRNAFromGene, CSequenceEditingEventHandler::tRNAFromGene)
    EVT_MENU (eCmdExonFromCds, CSequenceEditingEventHandler::ExonFromCds )
    EVT_MENU (eCmdExonFromMRNA, CSequenceEditingEventHandler::ExonFromMRNA )
    EVT_MENU (eCmdExonFromTRNA, CSequenceEditingEventHandler::ExonFromTRNA )
    EVT_MENU (eCmdFixProductNamesDefault, CSequenceEditingEventHandler::FixProductNamesDefault )
    EVT_MENU (eCmdFixProductNamesOptions, CSequenceEditingEventHandler::FixProductNamesOptions)
    EVT_MENU (eCmdBulkApplyGenCode, CSequenceEditingEventHandler::BulkApplyGenCode)
    EVT_MENU (eCmdEditSeqEndsWithAlign, CSequenceEditingEventHandler::EditSeqEndsWithAlign)
    EVT_MENU (eCmdEditSequenceEnds, CSequenceEditingEventHandler::EditSequenceEnds)
    EVT_MENU (eCmdUpdateAlign, CSequenceEditingEventHandler::UpdateAlign)
    EVT_MENU (eCmdSaveDescriptors, CSequenceEditingEventHandler::SaveDescriptors)
    EVT_MENU (eCmdCreateSeqHistForTpaDetailed, CSequenceEditingEventHandler::CreateSeqHistForTpaDetailed)
    EVT_MENU (eCmdCreateSeqHistForTpa, CSequenceEditingEventHandler::CreateSeqHistForTpa)
    EVT_MENU (eCmdRemoveSeqHistAssembly, CSequenceEditingEventHandler::RemoveSeqHistAssembly)
    EVT_MENU (eCmdFarPointerSeq, CSequenceEditingEventHandler::FarPointerSeq)
    EVT_MENU (eCmdConvertBadCdsAndRnaToMiscFeat, CSequenceEditingEventHandler::ConvertBadCdsAndRnaToMiscFeat)
    EVT_MENU (eCmdCreateProteinId, CSequenceEditingEventHandler::CreateProteinId)
    EVT_MENU (eCmdCreateLocusTagGene, CSequenceEditingEventHandler::CreateLocusTagGene)
    EVT_MENU (eCmdNormalizeGeneQuals, CSequenceEditingEventHandler::NormalizeGeneQuals)
    EVT_MENU (eCmdGenusSpeciesFixup, CSequenceEditingEventHandler::GenusSpeciesFixup)
    EVT_MENU (eCmdCountryConflict, CSequenceEditingEventHandler::CountryConflict)
    EVT_MENU (eCmdDuplicate, CSequenceEditingEventHandler::Duplicate)
    EVT_MENU (eCmdCorrectIntervalOrder, CSequenceEditingEventHandler::CorrectIntervalOrder)
    EVT_MENU (eCmdAddSecondary, CSequenceEditingEventHandler::AddSecondary)
    EVT_MENU (eCmdPackageFeaturesOnParts, CSequenceEditingEventHandler::PackageFeaturesOnParts)
    EVT_MENU (eCmdParseText, CSequenceEditingEventHandler::ParseText )
    EVT_MENU (eCmdParseTextFromDefline, CSequenceEditingEventHandler::ParseText )
    EVT_MENU (eCmdRemoveTextInsideStr, CSequenceEditingEventHandler::RemoveTextInsideStr )
    EVT_MENU (eCmdParseLocalToSrc, CSequenceEditingEventHandler::ParseText )
    EVT_MENU (eCmdLowercaseQuals, CSequenceEditingEventHandler::LowercaseQuals )
    EVT_MENU (eCmdConvertFeatures, CSequenceEditingEventHandler::ConvertFeatures )
    EVT_MENU (eCmdCdsToMatPeptide, CSequenceEditingEventHandler::CdsToMatPeptide )
    EVT_MENU (eCmdConvertCdsToMiscFeat, CSequenceEditingEventHandler::ConvertCdsToMiscFeat )
    EVT_MENU (eCmdConvertCdsWithInternalStopToMiscFeat, CSequenceEditingEventHandler::ConvertCdsWithInternalStopToMiscFeat )
    EVT_MENU (eCmdConvertCdsWithInternalStopToMiscFeatViral, CSequenceEditingEventHandler::ConvertCdsWithInternalStopToMiscFeatViral )
    EVT_MENU (eCmdConvertCdsWithInternalStopToMiscFeatUnverified, CSequenceEditingEventHandler::ConvertCdsWithInternalStopToMiscFeatUnverified )
    EVT_MENU (eCmdConvertCdsWithGapsToMiscFeat, CSequenceEditingEventHandler::ConvertCdsWithGapsToMiscFeat )
    EVT_MENU (eCmdTableReader, CSequenceEditingEventHandler::TableReader )
    EVT_MENU (eCmdTableReader_ext, CSequenceEditingEventHandler::TableReader)
    EVT_MENU (eCmdTableReaderClipboard, CSequenceEditingEventHandler::TableReaderClipboard )
    EVT_MENU (eCmdBulkSourceEdit, CSequenceEditingEventHandler::BulkEdit )
    EVT_MENU (eCmdBulkCdsEdit, CSequenceEditingEventHandler::BulkCdsEdit )
    EVT_MENU (eCmdBulkGeneEdit, CSequenceEditingEventHandler::BulkGeneEdit )
    EVT_MENU (eCmdBulkRnaEdit, CSequenceEditingEventHandler::BulkRnaEdit )
    EVT_MENU (eCmdLatLonTool, CSequenceEditingEventHandler::LatLonTool )
    EVT_MENU (eCmdCDSGeneRangeErrorSuppress, CSequenceEditingEventHandler::CDSGeneRangeErrorSuppress )
    EVT_MENU (eCmdCDSGeneRangeErrorRestore, CSequenceEditingEventHandler::CDSGeneRangeErrorRestore )
    EVT_MENU (eCmdSuppressGenes, CSequenceEditingEventHandler::SuppressGenes)
    EVT_MENU (eCmdCorrectGenes, CSequenceEditingEventHandler::CorrectGenes )
    EVT_MENU (eCmdSegregateSets, CSequenceEditingEventHandler::SegregateSets )
    EVT_MENU (eCmdSequesterSets, CSequenceEditingEventHandler::SequesterSets )
    EVT_MENU (eCmdDescriptorPropagateDown, CSequenceEditingEventHandler::DescriptorPropagateDown )
    EVT_MENU (eCmdReorderSeqById, CSequenceEditingEventHandler::ReorderSeqById )
    EVT_MENU (eCmdCombineSelectedGenesIntoPseudogenes, CSequenceEditingEventHandler::CombineSelectedGenesIntoPseudogenes )
    EVT_MENU (eCmdWithdrawSequences, CSequenceEditingEventHandler::WithdrawSequences )
    EVT_MENU (eCmdAddSet, CSequenceEditingEventHandler::AddSet )
    EVT_MENU (eCmdPropagateDBLink, CSequenceEditingEventHandler::PropagateDBLink )
    EVT_MENU (eCmdChangeSetClass, CSequenceEditingEventHandler::ConvertSetType )
    EVT_MENU (eCmdMolInfoEdit, CSequenceEditingEventHandler::MolInfoEdit )
    EVT_MENU (eCmdExportTable, CSequenceEditingEventHandler::ExportTable )
    EVT_MENU (eCmdParseStrainSerotypeFromNames, CSequenceEditingEventHandler::ParseStrainSerotypeFromNames)    
    EVT_MENU (eCmdAddStrainSerotypeToNames, CSequenceEditingEventHandler::AddStrainSerotypeToNames )
    EVT_MENU (eCmdFixupOrganismNames, CSequenceEditingEventHandler::FixupOrganismNames)
    EVT_MENU (eCmdSplitQualifiersAtCommas, CSequenceEditingEventHandler::SplitQualifiersAtCommas)
    EVT_MENU (eCmdSplitStructuredCollections, CSequenceEditingEventHandler::SplitStructuredCollections)
    EVT_MENU (eCmdTrimOrganismNames, CSequenceEditingEventHandler::TrimOrganismNames)
    EVT_MENU( eCmdRemoveUnverified, CSequenceEditingEventHandler::RemoveUnverified )
    EVT_MENU (eCmdAddNamedrRNA12S, CSequenceEditingEventHandler::AddNamedrRNA)
    EVT_MENU (eCmdAddNamedrRNA16S, CSequenceEditingEventHandler::AddNamedrRNA)
    EVT_MENU (eCmdAddNamedrRNA23S, CSequenceEditingEventHandler::AddNamedrRNA)
    EVT_MENU (eCmdAddNamedrRNA18S, CSequenceEditingEventHandler::AddNamedrRNA)
    EVT_MENU (eCmdAddNamedrRNA28S, CSequenceEditingEventHandler::AddNamedrRNA)
    EVT_MENU (eCmdAddNamedrRNA26S, CSequenceEditingEventHandler::AddNamedrRNA)
    EVT_MENU (eCmdAddNamedrRNASmall, CSequenceEditingEventHandler::AddNamedrRNA)
    EVT_MENU (eCmdAddNamedrRNALarge, CSequenceEditingEventHandler::AddNamedrRNA)
    EVT_MENU (eCmdAddControlRegion, CSequenceEditingEventHandler::AddControlRegion)
    EVT_MENU (eCmdAddMicrosatellite, CSequenceEditingEventHandler::AddMicrosatellite)
    EVT_MENU (eCmdCreateGene, CSequenceEditingEventHandler::CreateFeature)
    EVT_MENU (eCmdCreateLTR, CSequenceEditingEventHandler::CreateFeature)
    EVT_MENU (eCmdCreateProt, CSequenceEditingEventHandler::CreateFeature)
    EVT_MENU (eCmdCreateSecondaryStructure, CSequenceEditingEventHandler::CreateFeature)
    EVT_MENU (eCmdCreateRegion, CSequenceEditingEventHandler::CreateFeature)
    EVT_MENU (eCmdCreatePubFeatureLabeled, CSequenceEditingEventHandler::CreateFeature)
    EVT_MENU (eCmdCreateStemLoop, CSequenceEditingEventHandler::CreateFeature)
    EVT_MENU (eCmdCreateSTS, CSequenceEditingEventHandler::CreateFeature)
    EVT_MENU (eCmdCreateCodingRegion, CSequenceEditingEventHandler::CreateFeature)
    EVT_MENU (eCmdCreateCodingRegion_ext, CSequenceEditingEventHandler::CreateFeature)
    EVT_MENU (eCmdCreateRepeatRegion, CSequenceEditingEventHandler::CreateFeature)
    EVT_MENU (eCmdCreateVariation, CSequenceEditingEventHandler::CreateFeature)
    EVT_MENU (eCmdCreateOperon, CSequenceEditingEventHandler::CreateFeature)
    EVT_MENU (eCmdCreateMobileElement, CSequenceEditingEventHandler::CreateFeature)
    EVT_MENU (eCmdCreateCentromere, CSequenceEditingEventHandler::CreateFeature)
    EVT_MENU (eCmdCreateTelomere, CSequenceEditingEventHandler::CreateFeature)
    EVT_MENU (eCmdCreateRegulatory, CSequenceEditingEventHandler::CreateFeature)
    EVT_MENU (eCmdCreateRegulatory_promoter, CSequenceEditingEventHandler::CreateRegulatory)
    EVT_MENU (eCmdCreateRegulatory_enhancer, CSequenceEditingEventHandler::CreateRegulatory)
    EVT_MENU (eCmdCreateRegulatory_ribosome_binding_site, CSequenceEditingEventHandler::CreateRegulatory)
    EVT_MENU (eCmdCreateRegulatory_riboswitch, CSequenceEditingEventHandler::CreateRegulatory)
    EVT_MENU (eCmdCreateRegulatory_terminator, CSequenceEditingEventHandler::CreateRegulatory)
    EVT_MENU (eCmdCreatePreRNA, CSequenceEditingEventHandler::CreateFeature)
    EVT_MENU (eCmdCreateMRNA, CSequenceEditingEventHandler::CreateFeature)
    EVT_MENU (eCmdCreateExon, CSequenceEditingEventHandler::CreateFeature)
    EVT_MENU (eCmdCreateIntron, CSequenceEditingEventHandler::CreateFeature)
    EVT_MENU (eCmdCreate3UTR, CSequenceEditingEventHandler::CreateFeature)
    EVT_MENU (eCmdCreate5UTR, CSequenceEditingEventHandler::CreateFeature)
    EVT_MENU (eCmdCreatetRNA, CSequenceEditingEventHandler::CreateFeature)
    EVT_MENU (eCmdCreaterRNA, CSequenceEditingEventHandler::CreateFeature)
    EVT_MENU (eCmdCreateRNA, CSequenceEditingEventHandler::CreateFeature)
    EVT_MENU (eCmdCreatemiscRNA, CSequenceEditingEventHandler::CreateFeature)
    EVT_MENU (eCmdCreatepreRNA, CSequenceEditingEventHandler::CreateFeature)
    EVT_MENU (eCmdCreatencRNA, CSequenceEditingEventHandler::CreateFeature)
    EVT_MENU (eCmdCreatetmRNA, CSequenceEditingEventHandler::CreateFeature)
    EVT_MENU (eCmdCreateCommentDescriptor, CSequenceEditingEventHandler::CreateFeature)
    EVT_MENU (eCmdCreateBiosourceFeat, CSequenceEditingEventHandler::CreateFeature)
    EVT_MENU (eCmdCreatePolyASite, CSequenceEditingEventHandler::CreateFeature)
    EVT_MENU (eCmdCreatePrimerBind, CSequenceEditingEventHandler::CreateFeature)
    EVT_MENU (eCmdCreateProteinBind, CSequenceEditingEventHandler::CreateFeature)
    EVT_MENU (eCmdCreateRepOrigin, CSequenceEditingEventHandler::CreateFeature)
    EVT_MENU (eCmdCreateOriT, CSequenceEditingEventHandler::CreateFeature)
    EVT_MENU (eCmdCreateCRegion, CSequenceEditingEventHandler::CreateFeature)
    EVT_MENU (eCmdCreateDLoop, CSequenceEditingEventHandler::CreateFeature)
    EVT_MENU (eCmdCreateDSegment, CSequenceEditingEventHandler::CreateFeature)
    EVT_MENU (eCmdCreateiDNA, CSequenceEditingEventHandler::CreateFeature)
    EVT_MENU (eCmdCreateJSegment, CSequenceEditingEventHandler::CreateFeature)
    EVT_MENU (eCmdCreateMiscBinding, CSequenceEditingEventHandler::CreateFeature)
    EVT_MENU (eCmdCreateMiscDifference, CSequenceEditingEventHandler::CreateFeature)
    EVT_MENU (eCmdCreateMiscFeature, CSequenceEditingEventHandler::CreateFeature)
    EVT_MENU (eCmdCreateMiscRecomb, CSequenceEditingEventHandler::CreateFeature)
    EVT_MENU (eCmdCreateMiscStructure, CSequenceEditingEventHandler::CreateFeature)
    EVT_MENU (eCmdCreateModifiedBase, CSequenceEditingEventHandler::CreateFeature)
    EVT_MENU (eCmdCreateNRegion, CSequenceEditingEventHandler::CreateFeature)
    EVT_MENU (eCmdCreatePrimTranscript, CSequenceEditingEventHandler::CreateFeature)
    EVT_MENU (eCmdCreateSRegion, CSequenceEditingEventHandler::CreateFeature)
    EVT_MENU (eCmdCreateUnsure, CSequenceEditingEventHandler::CreateFeature)
    EVT_MENU (eCmdCreateVRegion, CSequenceEditingEventHandler::CreateFeature)
    EVT_MENU (eCmdCreateVSegment, CSequenceEditingEventHandler::CreateFeature)
    EVT_MENU (eCmdCreateProprotein, CSequenceEditingEventHandler::CreateFeature)
    EVT_MENU (eCmdCreateMatPeptide, CSequenceEditingEventHandler::CreateFeature)
    EVT_MENU (eCmdCreateSigPeptide, CSequenceEditingEventHandler::CreateFeature)
    EVT_MENU (eCmdCreateTransitPeptide, CSequenceEditingEventHandler::CreateFeature)
    EVT_MENU (eCmdCreateBond, CSequenceEditingEventHandler::CreateFeature)
    EVT_MENU (eCmdCreateSite, CSequenceEditingEventHandler::CreateFeature)
    EVT_MENU (eCmdCreateAssemblyGap, CSequenceEditingEventHandler::CreateFeature)
    EVT_MENU (eCmdAddDefLine, CSequenceEditingEventHandler::AddDefLine)
    EVT_MENU (eCmdPrefixOrgToDefLine, CSequenceEditingEventHandler::PrefixDefLines)
    EVT_MENU (eCmdPrefixStrainToDefLine, CSequenceEditingEventHandler::PrefixDefLines)
    EVT_MENU (eCmdPrefixCloneToDefLine, CSequenceEditingEventHandler::PrefixDefLines)
    EVT_MENU (eCmdPrefixIsolateToDefLine, CSequenceEditingEventHandler::PrefixDefLines)
    EVT_MENU (eCmdPrefixHaplotToDefLine, CSequenceEditingEventHandler::PrefixDefLines)
    EVT_MENU (eCmdPrefixCultivarToDefLine, CSequenceEditingEventHandler::PrefixDefLines)
    EVT_MENU (eCmdPrefixListToDefLine, CSequenceEditingEventHandler::PrefixDefLines)
    EVT_MENU (eCmdCreateDescTPAAssembly, CSequenceEditingEventHandler::CreateDescriptor)
    EVT_MENU (eCmdCreateDescStructuredComment, CSequenceEditingEventHandler::CreateDescriptor)
    EVT_MENU (eCmdCreateDescRefGeneTracking, CSequenceEditingEventHandler::CreateDescriptor)
    EVT_MENU (eCmdCreateDescDBLink, CSequenceEditingEventHandler::CreateDescriptor)
    EVT_MENU (eCmdCreateDescUnverified, CSequenceEditingEventHandler::CreateDescriptor)
    EVT_MENU (eCmdCreateDescAuthorizedAccess, CSequenceEditingEventHandler::CreateDescriptor)
    EVT_MENU (eCmdCreateDescGenomeAssemblyComment, CSequenceEditingEventHandler::CreateDescriptor)
    EVT_MENU (eCmdCreateDescGenBankBlock, CSequenceEditingEventHandler::CreateDescriptor)
    EVT_MENU (eCmdCreateDescComment, CSequenceEditingEventHandler::CreateDescriptor)
    EVT_MENU (eCmdCreateDescComment_ext, CSequenceEditingEventHandler::CreateDescriptor)
    EVT_MENU (eCmdCreateDescTitle, CSequenceEditingEventHandler::CreateDescriptor)
    EVT_MENU (eCmdCreateDescMolinfo, CSequenceEditingEventHandler::CreateDescriptor)
    EVT_MENU (eCmdCreateDescSource, CSequenceEditingEventHandler::CreateDescriptor)
    EVT_MENU (eCmdMakeBadSpecificHostTable, CSequenceEditingEventHandler::MakeBadSpecificHostTable)
    EVT_MENU (eCmdEditingButtons, CSequenceEditingEventHandler::EditingButtons)
    EVT_MENU (eCmdMakeToolBarWindow, CSequenceEditingEventHandler::EditingButtons)
    EVT_MENU (eCmdEnableHupId, CSequenceEditingEventHandler::EnableHupId)
    EVT_MENU (eCmdIndexerTypeSelection, CSequenceEditingEventHandler::IndexerTypeSelection)
    EVT_MENU (eCmdCreateDescPub, CSequenceEditingEventHandler::CreateDescriptor)
    EVT_MENU (eCmdCreateDescPub_ext, CSequenceEditingEventHandler::CreateDescriptor)
    EVT_MENU (eCmdCreateStructuredCommentPrefixSuffixMIENSData, CSequenceEditingEventHandler::CreateSpecificStructuredComment)
    EVT_MENU (eCmdCreateStructuredCommentPrefixSuffixMIMARKS, CSequenceEditingEventHandler::CreateSpecificStructuredComment)
    EVT_MENU (eCmdCreateStructuredCommentPrefixSuffixMIGS, CSequenceEditingEventHandler::CreateSpecificStructuredComment)
    EVT_MENU (eCmdCreateStructuredCommentPrefixSuffixMIMS, CSequenceEditingEventHandler::CreateSpecificStructuredComment)
    EVT_MENU (eCmdCreateStructuredCommentPrefixSuffixGenAssData, CSequenceEditingEventHandler::CreateSpecificStructuredComment)
    EVT_MENU (eCmdCreateStructuredCommentPrefixSuffixGenAnnData, CSequenceEditingEventHandler::CreateSpecificStructuredComment)
    EVT_MENU (eCmdDeleteStructuredCommentPrefixSuffixMIENSData, CSequenceEditingEventHandler::DeleteSpecificStructuredComment)
    EVT_MENU (eCmdDeleteStructuredCommentPrefixSuffixMIMARKS, CSequenceEditingEventHandler::DeleteSpecificStructuredComment)
    EVT_MENU (eCmdDeleteStructuredCommentPrefixSuffixMIGS, CSequenceEditingEventHandler::DeleteSpecificStructuredComment)
    EVT_MENU (eCmdDeleteStructuredCommentPrefixSuffixMIMS, CSequenceEditingEventHandler::DeleteSpecificStructuredComment)
    EVT_MENU (eCmdDeleteStructuredCommentPrefixSuffixGenAssData, CSequenceEditingEventHandler::DeleteSpecificStructuredComment)
    EVT_MENU (eCmdDeleteStructuredCommentPrefixSuffixGenAnnData, CSequenceEditingEventHandler::DeleteSpecificStructuredComment)
    EVT_MENU (eCmdCorrectRNAStrand, CSequenceEditingEventHandler::CorrectRNAStrand)
    EVT_MENU (eCmdTrimNsRich, CSequenceEditingEventHandler::TrimNsRich)
    EVT_MENU (eCmdTrimNsTerminal, CSequenceEditingEventHandler::TrimNsTerminal)
    EVT_MENU (eCmdAddFluComments, CSequenceEditingEventHandler::AddFluComments)
    EVT_MENU (eCmdLabelRna, CSequenceEditingEventHandler::LabelRna)
    EVT_MENU (eCmdRemProtTitles, CSequenceEditingEventHandler::RemProtTitles)
    EVT_MENU (eCmdRemAllProtTitles, CSequenceEditingEventHandler::RemAllProtTitles)
    EVT_MENU (eCmdApplyRNA_ITS, CSequenceEditingEventHandler::ApplyRNA_ITS)
    EVT_MENU (eCmdTestDialogView, CSequenceEditingEventHandler::TestDialogView)
    EVT_MENU (eCmdTestFormView, CSequenceEditingEventHandler::TestFormView)
    EVT_MENU (eCmdMacroEditor, CSequenceEditingEventHandler::MacroEditor)
    EVT_MENU (eCmdSuspectProductRulesEditor, CSequenceEditingEventHandler::SuspectProductRulesEditor)
    EVT_MENU (eCmdAdjustConsensusSpliceSitesStrict, CSequenceEditingEventHandler::AdjustForConsensusSpliceSites)
    EVT_MENU (eCmdAdjustConsensusSpliceSitesStrict_ext, CSequenceEditingEventHandler::AdjustForConsensusSpliceSites)
    EVT_MENU (eCmdAdjustConsensusSpliceSitesRelaxed, CSequenceEditingEventHandler::AdjustForConsensusSpliceSites)
    EVT_MENU (eCmdAdjustConsensusSpliceSitesRelaxed_ext, CSequenceEditingEventHandler::AdjustForConsensusSpliceSites)
    EVT_MENU (eCmdAdjustConsensusSpliceSitesConsensusEnds, CSequenceEditingEventHandler::AdjustForConsensusSpliceSites)
    EVT_MENU (eCmdAdjustCDSForIntrons, CSequenceEditingEventHandler::AdjustCDSForIntrons)
    EVT_MENU (eCmdAdjustrRNAForIntrons, CSequenceEditingEventHandler::AdjustrRNAForIntrons)
    EVT_MENU (eCmdAdjusttRNAForIntrons, CSequenceEditingEventHandler::AdjusttRNAForIntrons)
    EVT_MENU (eCmdAdjustmRNAForIntrons, CSequenceEditingEventHandler::AdjustmRNAForIntrons)
    EVT_MENU (eCmdRemoveSegGaps, CSequenceEditingEventHandler::RemoveSegGaps)
    EVT_MENU (eCmdRawSeqToDeltaByNs, CSequenceEditingEventHandler::RawSeqToDeltaByNs)
    EVT_MENU (eCmdRawSeqToDeltaByNs_ext, CSequenceEditingEventHandler::RawSeqToDeltaByNs)
    EVT_MENU (eCmdRawSeqToDeltaByLoc, CSequenceEditingEventHandler::RawSeqToDeltaByLoc)
    EVT_MENU (eCmdRawSeqToDeltaByAssemblyGapFeatures, CSequenceEditingEventHandler::RawSeqToDeltaByAssemblyGapFeatures)
    EVT_MENU (eCmdAdjustFeaturesForGaps, CSequenceEditingEventHandler::AdjustFeaturesForGaps )
    EVT_MENU (eCmdDeltaSeqToRaw, CSequenceEditingEventHandler::DeltaSeqToRaw)
    EVT_MENU (eCmdDeltaSeqToRaw_ext, CSequenceEditingEventHandler::DeltaSeqToRaw)
    EVT_MENU (eCmdExpandGaps, CSequenceEditingEventHandler::ExpandGaps )
    EVT_MENU (eCmdConvertSelectedGapsToKnown, CSequenceEditingEventHandler::ConvertSelectedGapsToKnown )
    EVT_MENU (eCmdConvertSelectedGapsToUnknown, CSequenceEditingEventHandler::ConvertSelectedGapsToUnknown )
    EVT_MENU (eCmdConvertGapsBySize, CSequenceEditingEventHandler::ConvertGapsBySize)
    EVT_MENU (eCmdEditSelectedGaps, CSequenceEditingEventHandler::EditSelectedGaps)
    EVT_MENU (eCmdAddLinkageToGaps, CSequenceEditingEventHandler::AddLinkageToGaps)
    EVT_MENU (eCmdAddLinkageToGaps_ext, CSequenceEditingEventHandler::AddLinkageToGaps)
    EVT_MENU (eCmdCombineAdjacentGaps, CSequenceEditingEventHandler::CombineAdjacentGaps)
    EVT_MENU (eCmdSplitCDSwithTooManyXs, CSequenceEditingEventHandler::SplitCDSwithTooManyXs)
    EVT_MENU (eCmdRmCultureNotes, CSequenceEditingEventHandler::RmCultureNotes)
    EVT_MENU (eCmdEditSeqId, CSequenceEditingEventHandler::EditSeqId)
    EVT_MENU (eCmdRemoveLocalSeqIdsFromNuc, CSequenceEditingEventHandler::RemoveLocalSeqIdsFromNuc)
    EVT_MENU (eCmdRemoveLocalSeqIdsFromProt, CSequenceEditingEventHandler::RemoveLocalSeqIdsFromProt)
    EVT_MENU (eCmdRemoveLocalSeqIdsFromAll, CSequenceEditingEventHandler::RemoveLocalSeqIdsFromAll)
    EVT_MENU (eCmdRemoveGiSeqIdsFromAll, CSequenceEditingEventHandler::RemoveGiSeqIdsFromAll)
    EVT_MENU (eCmdRemoveGenbankSeqIdsFromProt, CSequenceEditingEventHandler::RemoveGenbankSeqIdsFromProt)
    EVT_MENU (eCmdRemoveGenbankSeqIdsFromAll, CSequenceEditingEventHandler::RemoveGenbankSeqIdsFromAll)
    EVT_MENU (eCmdRemoveSeqIdNamesFromProtFeats, CSequenceEditingEventHandler::RemoveSeqIdNamesFromProtFeats)
    EVT_MENU (eCmdConvertAccessionToLocalIdsAll, CSequenceEditingEventHandler::ConvertAccessionToLocalIdsAll)
    EVT_MENU (eCmdConvertAccessionToLocalIdsNuc, CSequenceEditingEventHandler::ConvertAccessionToLocalIdsNuc)
    EVT_MENU (eCmdConvertAccessionToLocalIdsProt, CSequenceEditingEventHandler::ConvertAccessionToLocalIdsProt)
    EVT_MENU (eCmdConvertAccessionToLocalIdsName, CSequenceEditingEventHandler::ConvertAccessionToLocalIdsName)
    EVT_MENU (eCmdLocalToGeneralId, CSequenceEditingEventHandler::LocalToGeneralId)
    EVT_MENU (eCmdGeneralToLocalId, CSequenceEditingEventHandler::GeneralToLocalId)
    EVT_MENU (eCmdRemoveGeneralId, CSequenceEditingEventHandler::RemoveGeneralId)
    EVT_MENU (eCmdRemoveUnnecessaryExceptions, CSequenceEditingEventHandler::RemoveUnnecessaryExceptions)
    EVT_MENU (eCmdEditHistory, CSequenceEditingEventHandler:: EditHistory)
    EVT_MENU (eCmdUpdate_Replaced_EC_numbers, CSequenceEditingEventHandler::Update_Replaced_EC_numbers)
    EVT_MENU (eCmdUpdateSequence, CSequenceEditingEventHandler::UpdateSingleSequence)
    EVT_MENU (eCmdUpdateSequence_ext, CSequenceEditingEventHandler::UpdateSingleSequence)
    EVT_MENU (eCmdUpdateSequenceClipboard, CSequenceEditingEventHandler::UpdateSingleSequence)
    EVT_MENU (eCmdUpdateSequenceAccession, CSequenceEditingEventHandler::UpdateSingleSequence)
    EVT_MENU (eCmdUpdateMultiSeqClipboard, CSequenceEditingEventHandler::UpdateMultipleSequences)
    EVT_MENU (eCmdUpdateMultiSequences, CSequenceEditingEventHandler::UpdateMultipleSequences)
    EVT_MENU (eCmdImportFeatureTable, CSequenceEditingEventHandler::ImportFeatureTable)
    EVT_MENU (eCmdImportFeatureTable_ext, CSequenceEditingEventHandler::ImportFeatureTable)
    EVT_MENU (eCmdImportFeatureTableClipboard, CSequenceEditingEventHandler::ImportFeatureTableClipboard)
    EVT_MENU (eCmdImportGFF3, CSequenceEditingEventHandler::ImportGFF3)
    EVT_MENU (eCmdImportGFF3_ext, CSequenceEditingEventHandler::ImportGFF3)
    EVT_MENU (eCmdLoadSecondaryAccessions, CSequenceEditingEventHandler::LoadSecondaryAccessions)
    EVT_MENU (eCmdLoadSecondaryAccessionsHistoryTakeover, CSequenceEditingEventHandler::LoadSecondaryAccessionsHistoryTakeover)
    EVT_MENU (eCmdAddGlobalCodeBreak, CSequenceEditingEventHandler::AddGlobalCodeBreak )
    EVT_MENU (eCmdApplyStructuredCommentField, CSequenceEditingEventHandler::ApplyStructuredCommentField)
    EVT_MENU (eCmdEditStructuredCommentPrefixSuffixHIVData, CSequenceEditingEventHandler::EditStructuredCommentPrefixSuffix)
    EVT_MENU (eCmdEditStructuredCommentPrefixSuffixFluData, CSequenceEditingEventHandler::EditStructuredCommentPrefixSuffix)
    EVT_MENU (eCmdEditStructuredCommentPrefixSuffixMIGSData, CSequenceEditingEventHandler::EditStructuredCommentPrefixSuffix)
    EVT_MENU (eCmdEditStructuredCommentPrefixSuffixMIMSData, CSequenceEditingEventHandler::EditStructuredCommentPrefixSuffix)
    EVT_MENU (eCmdEditStructuredCommentPrefixSuffixMIENSData, CSequenceEditingEventHandler::EditStructuredCommentPrefixSuffix)
    EVT_MENU (eCmdEditStructuredCommentPrefixSuffixMIMARKS3_0, CSequenceEditingEventHandler::EditStructuredCommentPrefixSuffix)
    EVT_MENU (eCmdEditStructuredCommentPrefixSuffixMIGS3_0, CSequenceEditingEventHandler::EditStructuredCommentPrefixSuffix)
    EVT_MENU (eCmdEditStructuredCommentPrefixSuffixMIMS3_0, CSequenceEditingEventHandler::EditStructuredCommentPrefixSuffix)
    EVT_MENU (eCmdEditStructuredCommentPrefixSuffixAssemblyData, CSequenceEditingEventHandler::EditStructuredCommentPrefixSuffix)
    EVT_MENU (eCmdEditStructuredCommentPrefixSuffixGenAssData, CSequenceEditingEventHandler::EditStructuredCommentPrefixSuffix)
    EVT_MENU (eCmdEditStructuredCommentPrefixSuffixiBol, CSequenceEditingEventHandler::EditStructuredCommentPrefixSuffix)
    EVT_MENU (eCmdLinkSelectedmRNACDS, CSequenceEditingEventHandler::LinkmRNACDS)
    EVT_MENU (eCmdLinkmRNACDSOverlap, CSequenceEditingEventHandler::LinkmRNACDS)
    EVT_MENU (eCmdLinkmRNACDSProduct, CSequenceEditingEventHandler::LinkmRNACDS)
    EVT_MENU (eCmdLinkmRNACDSLabel, CSequenceEditingEventHandler::LinkmRNACDS)
    EVT_MENU (eCmdLinkmRNACDSLabelLocation, CSequenceEditingEventHandler::LinkmRNACDS)
    EVT_MENU (eCmdLinkmRNACDSProteinID, CSequenceEditingEventHandler::LinkmRNACDS)
    EVT_MENU (eCmdLinkmRNACDSTranscriptID, CSequenceEditingEventHandler::LinkmRNACDS)
    EVT_MENU (eCmdLinkmRNACDSUnidirectional, CSequenceEditingEventHandler::LinkmRNACDS)
    EVT_MENU (eCmdShowLinkedmRNACDS, CSequenceEditingEventHandler::LinkmRNACDS)
    EVT_MENU (eCmdShowFeatureTable, CSequenceEditingEventHandler::ShowFeatureTable)
    EVT_MENU (eCmdShowFeatureTable_ext, CSequenceEditingEventHandler::ShowFeatureTable)
    EVT_MENU (eCmdRestoreFromFile, CSequenceEditingEventHandler::RestoreLocalFile)
    EVT_MENU (eCmdRestoreAndConvertSeqSubmit, CSequenceEditingEventHandler::RestoreLocalFile)
    EVT_MENU (eCmdViewSortedProteins, CSequenceEditingEventHandler::ViewSortedProteins)
    EVT_MENU (eCmdCheckForMixedStrands, CSequenceEditingEventHandler::CheckForMixedStrands)
    EVT_MENU (eCmdPrepareSeqSubmit, CSequenceEditingEventHandler::PrepareSeqSubmit)
    EVT_MENU (eCmdSaveSubmissionFile, CSequenceEditingEventHandler::SaveSubmissionFile)
    EVT_MENU (eCmdSaveASN1File, CSequenceEditingEventHandler::SaveASN1File)
    EVT_MENU (eCmdHelpReports_ext, CSequenceEditingEventHandler::Help)
    EVT_MENU (eCmdHelpImport_ext, CSequenceEditingEventHandler::Help)
    EVT_MENU (eCmdHelpSequences_ext, CSequenceEditingEventHandler::Help)
    EVT_MENU (eCmdHelpFeatures_ext, CSequenceEditingEventHandler::Help)
    EVT_MENU (eCmdHelpComments_ext, CSequenceEditingEventHandler::Help)
    EVT_MENU (eCmdHelpEditingTools_ext, CSequenceEditingEventHandler::Help)
        

// Please do not make overlapping UPDATE_UI_RANGEs. Every time there is an overlapping range here a cute kitten dies in horror!!!    
    EVT_UPDATE_UI_RANGE(eCmdRemoveDescriptors, eCmdCreateDescPub, CSequenceEditingEventHandler::IsObjectSelected )

    EVT_UPDATE_UI_RANGE(eCmdCreateBiosourceFeat, eCmdCreatePubFeatureLabeled, CSequenceEditingEventHandler::IsLocationSelected )

    EVT_UPDATE_UI_RANGE(eCmdTestDialogView, eCmdTestFormView, CSequenceEditingEventHandler::IsObjectSelected )

    EVT_UPDATE_UI(eCmdEnableHupId, CSequenceEditingEventHandler::IsHupIdEnabled)
END_EVENT_TABLE()

BEGIN_EVENT_MAP(CSequenceEditingEventHandler, CEventHandler)
        ON_EVENT_RANGE(CSelectionServiceEvent,
                   CSelectionServiceEvent::eClientAttached, CSelectionServiceEvent::eSelectionChanged,
                   &CSequenceEditingEventHandler::x_OnSelectionEvent)
END_EVENT_MAP()

static
WX_DEFINE_MENU(kEditingMenu)
    WX_SUBMENU("&Edit")
        WX_MENU_ITEM_INT(eCmdRemoveSequences)
        WX_MENU_ITEM_INT(eCmdAddSequences)
        WX_SUBMENU_INT("Update Sequences")
            WX_MENU_ITEM_INT(eCmdUpdateSequence)
            WX_MENU_ITEM_INT(eCmdUpdateSequenceClipboard)
            WX_MENU_ITEM_INT(eCmdUpdateSequenceAccession)
            WX_MENU_ITEM_INT(eCmdUpdateMultiSequences)
            WX_MENU_ITEM_INT(eCmdUpdateMultiSeqClipboard)
        WX_END_SUBMENU()
        WX_MENU_ITEM_INT(eCmdRestoreFromFile)
        WX_MENU_ITEM_INT(eCmdRestoreAndConvertSeqSubmit)
    WX_END_SUBMENU()
    WX_MENU_SEPARATOR_L("Contribs")
    WX_SUBMENU_INT("Sea&rch")
        WX_SUBMENU_INT("&Validate")
            WX_MENU_ITEM_INT(eCmdValidate)
            WX_MENU_ITEM_INT(eCmdValidateNoAlignments)
            WX_MENU_ITEM_INT(eCmdValidateCheckInferences)
        WX_END_SUBMENU()
        WX_MENU_ITEM_INT(eCmdVectorTrim)
        WX_MENU_ITEM_INT(eCmdSelectTarget)
    WX_END_SUBMENU()

    WX_SUBMENU_EXT("Submission")
        WX_MENU_ITEM_EXT(eCmdPrepareSeqSubmit)
        WX_MENU_ITEM_EXT(eCmdSaveSubmissionFile)
        WX_SUBMENU_EXT("Reports")
            WX_MENU_ITEM_EXT(eCmdShowFeatureTable_ext)
            WX_MENU_ITEM_EXT(eCmdValidate_ext)
            WX_MENU_ITEM_EXT(eCmdSortUniqueCountTree_ext)
            WX_MENU_ITEM_EXT(eCmdSubmitter_ext)
            WX_MENU_ITEM_EXT(eCmdVectorTrim)
            WX_MENU_SEPARATOR_EXT()
            WX_MENU_ITEM_EXT(eCmdHelpReports_ext)
        WX_END_SUBMENU()  
        WX_SUBMENU_EXT("Import")
            WX_MENU_ITEM_EXT(eCmdTableReader_ext)
            WX_MENU_ITEM_EXT(eCmdImportFeatureTable_ext)
            WX_MENU_ITEM_EXT(eCmdImportGFF3_ext)
            WX_MENU_SEPARATOR_EXT()
            WX_MENU_ITEM_EXT(eCmdHelpImport_ext)
        WX_END_SUBMENU()
        WX_SUBMENU_EXT("Sequences")
            WX_MENU_ITEM_EXT(eCmdAddSequences)
            WX_MENU_ITEM_EXT(eCmdEditSequence)
            WX_MENU_ITEM_EXT(eCmdUpdateSequence_ext)
            WX_MENU_ITEM_EXT(eCmdRemoveSequences_ext)
            WX_MENU_ITEM_EXT(eCmdRevCompSequences_ext) 
            WX_MENU_ITEM_EXT(eCmdTrimNsTerminal)
            WX_MENU_ITEM_EXT(eCmdExpandGaps)
            WX_MENU_ITEM_EXT(eCmdAddLinkageToGaps_ext)
            WX_MENU_ITEM_EXT(eCmdRawSeqToDeltaByNs_ext)
            WX_MENU_ITEM_EXT(eCmdDeltaSeqToRaw_ext)
            WX_MENU_SEPARATOR_EXT()
            WX_MENU_ITEM_EXT(eCmdHelpSequences_ext)
        WX_END_SUBMENU()
        WX_SUBMENU_EXT("Add Features")
            WX_SUBMENU_EXT("gene/mRNA/CDS")
                WX_MENU_ITEM_EXT(eCmdCreateGene)
                WX_MENU_ITEM_EXT(eCmdCreateMRNA)
                WX_MENU_ITEM_EXT(eCmdCreateCodingRegion_ext)
                WX_MENU_ITEM_EXT(eCmdCreateOperon)
                WX_MENU_ITEM_EXT(eCmdCreateIntron)
                WX_MENU_ITEM_EXT(eCmdCreateExon)
                WX_MENU_ITEM_EXT(eCmdCreate5UTR)
                WX_MENU_ITEM_EXT(eCmdCreate3UTR)
            WX_END_SUBMENU()
            WX_SUBMENU_EXT("Structural RNAs")
                WX_MENU_ITEM_EXT(eCmdCreaterRNA)
                WX_MENU_ITEM_EXT(eCmdCreatetRNA)
                WX_MENU_ITEM_EXT(eCmdCreatencRNA)
                WX_MENU_ITEM_EXT(eCmdCreatepreRNA)
                WX_MENU_ITEM_EXT(eCmdCreatetmRNA)
                WX_MENU_ITEM_EXT(eCmdCreatemiscRNA)
            WX_END_SUBMENU()
            WX_SUBMENU_EXT("Regulatory")
                WX_MENU_ITEM_EXT(eCmdCreateRegulatory_promoter)
                WX_MENU_ITEM_EXT(eCmdCreateRegulatory_enhancer)
                WX_MENU_ITEM_EXT(eCmdCreateRegulatory_ribosome_binding_site)
                WX_MENU_ITEM_EXT(eCmdCreateRegulatory_riboswitch)
                WX_MENU_ITEM_EXT(eCmdCreateRegulatory_terminator)
                WX_MENU_ITEM_EXT(eCmdCreateRegulatory)
            WX_END_SUBMENU()
            WX_SUBMENU_EXT("Protein Features")
                WX_MENU_ITEM_EXT(eCmdCreateMatPeptide)
                WX_MENU_ITEM_EXT(eCmdCreateSigPeptide)
                WX_MENU_ITEM_EXT(eCmdCreateProprotein)
                WX_MENU_ITEM_EXT(eCmdCreateTransitPeptide)
            WX_END_SUBMENU()
            WX_SUBMENU_EXT("Other Features")
                WX_MENU_ITEM_EXT(eCmdCreateCentromere)
                WX_MENU_ITEM_EXT(eCmdCreateDLoop)
                WX_MENU_ITEM_EXT(eCmdCreateMiscBinding)
                WX_MENU_ITEM_EXT(eCmdCreateMiscDifference)
                WX_MENU_ITEM_EXT(eCmdCreateMiscFeature)
                WX_MENU_ITEM_EXT(eCmdCreateMiscRecomb)
                WX_MENU_ITEM_EXT(eCmdCreateMobileElement)
                WX_MENU_ITEM_EXT(eCmdCreateOriT)
                WX_MENU_ITEM_EXT(eCmdCreateProteinBind)
                WX_MENU_ITEM_EXT(eCmdCreateRepOrigin)
                WX_MENU_ITEM_EXT(eCmdCreateRepeatRegion)
                WX_MENU_ITEM_EXT(eCmdCreateSite)
                WX_MENU_ITEM_EXT(eCmdCreateTelomere)
                WX_MENU_ITEM_EXT(eCmdCreateVariation)
            WX_END_SUBMENU()
            WX_MENU_SEPARATOR_EXT()
            WX_MENU_ITEM_EXT(eCmdHelpFeatures_ext)
        WX_END_SUBMENU()
        WX_MENU_ITEM_EXT(eCmdCreateDescPub_ext)
        WX_SUBMENU_EXT("Comments")
            WX_MENU_ITEM_EXT(eCmdCreateDescComment_ext)
            WX_SUBMENU_EXT("MIMS Comment")
                WX_MENU_ITEM_EXT(eCmdCreateStructuredCommentPrefixSuffixMIMS)
                WX_MENU_ITEM_EXT(eCmdDeleteStructuredCommentPrefixSuffixMIMS)
            WX_END_SUBMENU()
            WX_SUBMENU_EXT("MIGS Comment")
                WX_MENU_ITEM_EXT(eCmdCreateStructuredCommentPrefixSuffixMIGS)
                WX_MENU_ITEM_EXT(eCmdDeleteStructuredCommentPrefixSuffixMIGS)
            WX_END_SUBMENU()
            WX_SUBMENU_EXT("MIMARKS Comment")
                WX_MENU_ITEM_EXT(eCmdCreateStructuredCommentPrefixSuffixMIMARKS)
                WX_MENU_ITEM_EXT(eCmdDeleteStructuredCommentPrefixSuffixMIMARKS)
            WX_END_SUBMENU()
            WX_SUBMENU_EXT("MIENS Comment")
                WX_MENU_ITEM_EXT(eCmdCreateStructuredCommentPrefixSuffixMIENSData)
                WX_MENU_ITEM_EXT(eCmdDeleteStructuredCommentPrefixSuffixMIENSData)
            WX_END_SUBMENU()
            WX_SUBMENU_EXT("GenomeAnnotation Comment")
                WX_MENU_ITEM_EXT(eCmdCreateStructuredCommentPrefixSuffixGenAnnData)
                WX_MENU_ITEM_EXT(eCmdDeleteStructuredCommentPrefixSuffixGenAnnData)
            WX_END_SUBMENU()
            WX_SUBMENU_EXT("GenomeDataAssembly Comment")
                WX_MENU_ITEM_EXT(eCmdCreateStructuredCommentPrefixSuffixGenAssData)
                WX_MENU_ITEM_EXT(eCmdDeleteStructuredCommentPrefixSuffixGenAssData)
            WX_END_SUBMENU()
            WX_MENU_SEPARATOR_EXT()
            WX_MENU_ITEM_EXT(eCmdHelpComments_ext)
        WX_END_SUBMENU()
        WX_SUBMENU_EXT("Editing Tools")
            WX_MENU_ITEM_EXT(eCmdSelectTarget_ext)
            WX_MENU_ITEM_EXT(eCmdTaxFixCleanup_ext)
            WX_MENU_ITEM_EXT(eCmdBulkApplyGenCode)
            WX_MENU_ITEM_EXT(eCmdBulkSourceEdit)
            WX_MENU_ITEM_EXT(eCmdBulkCdsEdit)
            WX_MENU_ITEM_EXT(eCmdBulkGeneEdit)
            WX_MENU_ITEM_EXT(eCmdBulkRnaEdit)
            WX_MENU_ITEM_EXT(eCmdRemoveFeatures)
            WX_MENU_ITEM_EXT(eCmdRemoveAllFeatures)
            WX_MENU_ITEM_EXT(eCmdSelectFeature)
            WX_MENU_ITEM_EXT(eCmdConvertFeatures)
            WX_MENU_ITEM_EXT(eCmdRetranslateCDSIgnoreStopExceptEnd_ext)
            WX_MENU_ITEM_EXT(eCmdAdjustFeaturesForGaps)
            WX_MENU_SEPARATOR_EXT()
            WX_MENU_ITEM_EXT(eCmdHelpEditingTools_ext)
        WX_END_SUBMENU()
    WX_END_SUBMENU()

    WX_SUBMENU_INT("Advan&ced Editing")        
        WX_SUBMENU_INT("Def Line")
            WX_SUBMENU_INT("Automatic Def Line")
                WX_MENU_ITEM_INT(eCmdAutodefDefaultOptions)
                WX_MENU_ITEM_INT(eCmdAutodefNoMods)
                WX_MENU_ITEM_INT(eCmdAutodefOptions)
                WX_MENU_ITEM_INT(eCmdAutodefMisc)
                WX_MENU_ITEM_INT(eCmdAutodefId)
                WX_MENU_ITEM_INT(eCmdAutodefPopset)
                WX_MENU_ITEM_INT(eCmdAutodefRefresh)
            WX_END_SUBMENU()
            WX_MENU_ITEM_INT(eCmdAddDefLine)
            WX_SUBMENU_INT("Prefix Def Line With")
                WX_MENU_ITEM_INT(eCmdPrefixOrgToDefLine)
                WX_MENU_ITEM_INT(eCmdPrefixStrainToDefLine)
                WX_MENU_ITEM_INT(eCmdPrefixCloneToDefLine)
                WX_MENU_ITEM_INT(eCmdPrefixIsolateToDefLine)
                WX_MENU_ITEM_INT(eCmdPrefixHaplotToDefLine)
                WX_MENU_ITEM_INT(eCmdPrefixCultivarToDefLine)
                WX_MENU_ITEM_INT(eCmdPrefixListToDefLine)
            WX_END_SUBMENU()
            WX_MENU_ITEM_INT(eCmdParseTextFromDefline)
        WX_END_SUBMENU()
        WX_SUBMENU_INT("Organism")
            WX_MENU_ITEM_INT(eCmdPrefixAuthWithTax)
            WX_MENU_ITEM_INT(eCmdAppendModToOrg)
            WX_MENU_ITEM_INT(eCmdCountryFixupCap)
            WX_MENU_ITEM_INT(eCmdCountryFixupNoCap)
            WX_MENU_ITEM_INT(eCmdParseCollectionDateMonthFirst)
            WX_MENU_ITEM_INT(eCmdParseCollectionDateDayFirst)
            WX_MENU_ITEM_INT(eCmdMakeBadSpecificHostTable)
            WX_MENU_SEPARATOR_INT()
            WX_SUBMENU_INT("Source Focus")
                WX_MENU_ITEM_INT(eCmdFocusSet)
                WX_MENU_ITEM_INT(eCmdFocusClear)
            WX_END_SUBMENU()
            WX_MENU_ITEM_INT(eCmdSetTransgSrcDesc)
            WX_MENU_ITEM_INT(eCmdFixOrgModInstitution)
            WX_MENU_ITEM_INT(eCmdFixStructuredVouchers)
            WX_MENU_ITEM_INT(eCmdTrimOrganismNames)
            WX_MENU_ITEM_INT(eCmdSplitStructuredCollections)
            WX_MENU_ITEM_INT(eCmdSplitQualifiersAtCommas)
            WX_MENU_ITEM_INT(eCmdSplitDblinkQuals)
            WX_SUBMENU_INT("PCR Fixes")
                WX_MENU_ITEM_INT(eCmdSwapPrimerSeqName)
                WX_MENU_ITEM_INT(eCmdTrimJunkInPrimerSeqs)                    
                WX_MENU_ITEM_INT(eCmdSplitPrimerSets)
                WX_MENU_ITEM_INT(eCmdMergePrimerSets)
                WX_MENU_ITEM_INT(eCmdFixiInPrimerSeqs)
            WX_END_SUBMENU()  
            WX_SUBMENU_INT("Influenza Virus Names")
                WX_MENU_ITEM_INT(eCmdParseStrainSerotypeFromNames)
                WX_MENU_ITEM_INT(eCmdAddStrainSerotypeToNames)
                WX_MENU_ITEM_INT(eCmdFixupOrganismNames)
            WX_END_SUBMENU()
            WX_MENU_ITEM_INT(eCmdDisableStrainForwarding)
            WX_MENU_ITEM_INT(eCmdEnableStrainForwarding)
            WX_MENU_ITEM_INT(eCmdUnculTaxTool)
            WX_MENU_ITEM_INT(eCmdGenusSpeciesFixup)
            WX_MENU_ITEM_INT(eCmdLatLonTool)
            WX_MENU_ITEM_INT(eCmdCountryConflict)
            WX_SUBMENU_INT("Compare with Biosample")
                WX_MENU_ITEM_INT(eCmdCompareWithBiosample)
                WX_MENU_ITEM_INT(eCmdCompareWithBiosampleFirstOnly)
            WX_END_SUBMENU()
            WX_MENU_ITEM_INT(eCmdMergeBiosources)
            WX_SUBMENU_INT("Parse Organism Modifiers")
                WX_MENU_ITEM_INT(eCmdExportTable)
            WX_END_SUBMENU()
            WX_SUBMENU_INT("Consolidate Like Modifiers")
                WX_MENU_ITEM_INT(eCmdConsolidateLikeModsWithSemicolon)
                WX_MENU_ITEM_INT(eCmdConsolidateLikeModsWithoutSemicolon)
            WX_END_SUBMENU()
        WX_END_SUBMENU()
        WX_SUBMENU_INT("&Apply")
            WX_MENU_ITEM_INT(eCmdApplyCDS)
            WX_MENU_ITEM_INT(eCmdApplyAddRNA)
            WX_SUBMENU_INT("Add Named rRNA")
                WX_MENU_ITEM_INT(eCmdAddNamedrRNA12S)
                WX_MENU_ITEM_INT(eCmdAddNamedrRNA16S)
                WX_MENU_ITEM_INT(eCmdAddNamedrRNA23S)
                WX_MENU_ITEM_INT(eCmdAddNamedrRNA18S)
                WX_MENU_ITEM_INT(eCmdAddNamedrRNA28S)
                WX_MENU_ITEM_INT(eCmdAddNamedrRNA26S)
                WX_MENU_ITEM_INT(eCmdAddNamedrRNASmall)
                WX_MENU_ITEM_INT(eCmdAddNamedrRNALarge)
            WX_END_SUBMENU()
            WX_MENU_ITEM_INT(eCmdApplyRNA_ITS)
            WX_MENU_ITEM_INT(eCmdApplyAddOther)
            WX_MENU_ITEM_INT(eCmdAddFeatureBetween)
            WX_SUBMENU_INT("Add Common Feature")
                WX_MENU_ITEM_INT(eCmdAddMicrosatellite)
                WX_MENU_ITEM_INT(eCmdAddControlRegion)
            WX_END_SUBMENU()
            WX_MENU_ITEM_INT(eCmdApplyFeatQual)
            WX_MENU_ITEM_INT(eCmdApplySourceQual)
            WX_MENU_ITEM_INT(eCmdApplyCGPQual)
            WX_MENU_ITEM_INT(eCmdApplyRNAQual)
            WX_MENU_ITEM_INT(eCmdTableReader)
            WX_MENU_ITEM_INT(eCmdTableReaderClipboard)
            WX_MENU_ITEM_INT(eCmdExportTable)
            WX_MENU_ITEM_INT(eCmdMolInfoEdit)
            WX_MENU_ITEM_INT(eCmdLabelRna)
            WX_MENU_ITEM_INT(eCmdPropagateDBLink)
            WX_MENU_ITEM_INT(eCmdApplyIndexerComments)
            WX_MENU_ITEM_INT(eCmdImportGFF3)
            WX_MENU_ITEM_INT(eCmdImportFeatureTable)
            WX_MENU_ITEM_INT(eCmdImportFeatureTableClipboard)
            WX_SUBMENU_INT("Add Keyword")
                WX_MENU_ITEM_INT(eCmdAddStructuredCommentKeyword)
                WX_MENU_ITEM_INT(eCmdAddKeywordGDS)
                WX_MENU_ITEM_INT(eCmdAddKeywordTPA_inferential)
                WX_MENU_ITEM_INT(eCmdAddKeywordTPA_experimental)
                WX_MENU_ITEM_INT(eCmdAddKeywordTPA_assembly)
                WX_MENU_ITEM_INT(eCmdAddKeywordWithConstraint)
            WX_END_SUBMENU()
            WX_SUBMENU_INT("Load Secondary Accessions from File")
                WX_MENU_ITEM_INT(eCmdLoadSecondaryAccessions)
                WX_MENU_ITEM_INT(eCmdLoadSecondaryAccessionsHistoryTakeover)
            WX_END_SUBMENU()
            WX_MENU_ITEM_INT(eCmdAddGlobalCodeBreak)
        WX_END_SUBMENU()
        WX_SUBMENU_INT("&Remove")
            WX_MENU_ITEM_INT(eCmdRemoveDescriptors)
            WX_MENU_ITEM_INT(eCmdRemoveFeatures)
            WX_MENU_ITEM_INT(eCmdRemoveAllFeatures)
            WX_MENU_ITEM_INT(eCmdRemoveUnindexedFeatures)
            WX_MENU_ITEM_INT(eCmdRemoveFeatQual)
            WX_MENU_ITEM_INT(eCmdRemoveSourceQual)
            WX_MENU_ITEM_INT(eCmdRemoveCGPQual)
            WX_MENU_ITEM_INT(eCmdRemoveRNAQual)
            WX_MENU_ITEM_INT(eCmdRemoveSeqAnnotIds)
            WX_MENU_ITEM_INT(eCmdRemoveCddFeatDbxref)
            WX_MENU_ITEM_INT(eCmdRemoveBankitComment)
            WX_MENU_ITEM_INT(eCmdRemoveUnverified)
            WX_SUBMENU_INT("Remove Text")
                WX_MENU_ITEM_INT(eCmdRemoveTextOutsideString)
                WX_MENU_ITEM_INT(eCmdRemoveTextInsideStr)
            WX_END_SUBMENU()
            WX_SUBMENU_INT("Remove Duplicate")
                WX_MENU_ITEM_INT(eCmdRemoveDuplicateStructuredComments)
                WX_MENU_ITEM_INT(eCmdRemoveDupFeats)
                WX_MENU_ITEM_INT(eCmdRemoveDupFeatsWithOptions)
            WX_END_SUBMENU()
            WX_MENU_ITEM_INT(eCmdRemoveSequencesFromAlignments)
            WX_MENU_ITEM_INT(eCmdRemoveSequences)
            WX_MENU_ITEM_INT(eCmdRemoveAlignments)
            WX_MENU_ITEM_INT(eCmdRemoveGraphs)
            WX_MENU_ITEM_INT(eCmdRemoveGenomeProjectId)
            WX_MENU_ITEM_INT(eCmdRemoveEmptyGenomeProjectId)
            WX_SUBMENU_INT("Remove Titles")
                WX_SUBMENU_INT("Protein")
                    WX_MENU_ITEM_INT(eCmdRemProtTitles)
                    WX_MENU_ITEM_INT(eCmdRemAllProtTitles)
                WX_END_SUBMENU()
            WX_END_SUBMENU()
            WX_MENU_ITEM_INT(eCmdWithdrawSequences)
            WX_MENU_ITEM_INT(eCmdRmCultureNotes)
            WX_MENU_ITEM_INT(eCmdRmIllegalQuals)
            WX_MENU_ITEM_INT(eCmdRmWrongQuals)
            WX_SUBMENU_INT("Remove Xrefs")
                WX_SUBMENU_INT("Db_xrefs")
                    WX_MENU_ITEM_INT(eCmdRemoveDbXrefsCDS)
                    WX_MENU_ITEM_INT(eCmdRemoveDbXrefsGenes)
                    WX_MENU_ITEM_INT(eCmdRemoveDbXrefsRNA)
                    WX_MENU_ITEM_INT(eCmdRemoveDbXrefsAllFeats)
                    WX_MENU_ITEM_INT(eCmdRemoveDbXrefsBioSource)
                    WX_MENU_ITEM_INT(eCmdRemoveDbXrefsBioSourceAndFeats)
                WX_END_SUBMENU()
                WX_SUBMENU_INT("Taxon Xrefs")
                    WX_MENU_ITEM_INT(eCmdRemoveTaxonFeats)
                    WX_MENU_ITEM_INT(eCmdRemoveTaxonFeatsAndBioSource)
                WX_END_SUBMENU()
                WX_SUBMENU_INT("Gene Xrefs")
                    WX_MENU_ITEM_INT(eCmdRemoveAllGeneXrefs)
                    WX_MENU_ITEM_INT(eCmdRemoveUnnecessaryGeneXrefs)
                    WX_MENU_ITEM_INT(eCmdRemoveNonsuppressingGeneXrefs)
                    WX_MENU_ITEM_INT(eCmdRemoveOrphanLocusGeneXrefs)
                    WX_MENU_ITEM_INT(eCmdRemoveOrphanLocus_tagGeneXrefs)
                    WX_MENU_ITEM_INT(eCmdRemoveGeneXrefs)
                WX_END_SUBMENU()
            WX_END_SUBMENU()
            WX_MENU_ITEM_INT(eCmdClearKeywords)
            WX_MENU_ITEM_INT(eCmdClearNomenclature)
            WX_SUBMENU_INT("Remove Proteins")
                WX_MENU_ITEM_INT(eCmdJustRemoveProteins)
                WX_MENU_ITEM_INT(eCmdRemoveProteinsAndRenormalizeNucProtSets)
                WX_MENU_ITEM_INT(eCmdRemoveOrphanedProteins)
            WX_END_SUBMENU()
            WX_SUBMENU_INT("Remove IDs, SeqIDs, GI_IDs, LOCUS")
                WX_SUBMENU_INT("Local")
                    WX_MENU_ITEM_INT(eCmdRemoveLocalSeqIdsFromNuc)
                    WX_MENU_ITEM_INT(eCmdRemoveLocalSeqIdsFromProt)
                    WX_MENU_ITEM_INT(eCmdRemoveLocalSeqIdsFromAll)
                WX_END_SUBMENU()
                WX_SUBMENU_INT("GenBank")
                    WX_MENU_ITEM_INT(eCmdRemoveGenbankSeqIdsFromProt)
                    WX_MENU_ITEM_INT(eCmdRemoveGenbankSeqIdsFromAll)
                WX_END_SUBMENU()
                WX_MENU_ITEM_INT(eCmdRemoveGiSeqIdsFromAll)
                WX_MENU_ITEM_INT(eCmdRemoveSeqIdNamesFromProtFeats)
                WX_MENU_ITEM_INT(eCmdRemoveGeneralId)
            WX_END_SUBMENU()
            WX_MENU_ITEM_INT(eCmdRemoveUnnecessaryExceptions)
        WX_END_SUBMENU()
        WX_SUBMENU_INT("&Convert")
            WX_MENU_ITEM_INT(eCmdConvertFeatures)
            WX_MENU_ITEM_INT(eCmdConvertCdsToMiscFeat)
            WX_SUBMENU_INT("Convert CDS with Internal Stop Codon")
                WX_MENU_ITEM_INT(eCmdConvertCdsWithInternalStopToMiscFeat)
                WX_MENU_ITEM_INT(eCmdConvertCdsWithInternalStopToMiscFeatViral)
                WX_MENU_ITEM_INT(eCmdConvertCdsWithInternalStopToMiscFeatUnverified)
            WX_END_SUBMENU()
            WX_MENU_ITEM_INT(eCmdConvertFeatQual)
            WX_MENU_ITEM_INT(eCmdConvertSourceQual)
            WX_MENU_ITEM_INT(eCmdConvertCGPQual)
            WX_MENU_ITEM_INT(eCmdConvertRNAQual)
            WX_MENU_ITEM_INT(eCmdSwapFeatQual)
            WX_MENU_ITEM_INT(eCmdSwapSourceQual)
            WX_MENU_ITEM_INT(eCmdSwapCGPQual)
            WX_MENU_ITEM_INT(eCmdSwapRNAQual)
            WX_MENU_ITEM_INT(eCmdParseText)
            WX_MENU_ITEM_INT(eCmdIllegalQualsToNote)
            WX_MENU_ITEM_INT(eCmdWrongQualsToNote)
            WX_MENU_ITEM_INT(eCmdConvertBadInference)
            WX_MENU_ITEM_INT(eCmdConvertToDelayedGenProdSetQuals)
            WX_MENU_ITEM_INT(eCmdConvertRptUnitRangeToLoc)
            WX_SUBMENU_INT("Convert IDs, Accessions, or GenBank.names")
                WX_MENU_ITEM_INT(eCmdLocalToGeneralId)
                WX_MENU_ITEM_INT(eCmdGeneralToLocalId)
                WX_SUBMENU_INT("Accession to LocalIDs")
                    WX_MENU_ITEM_INT(eCmdConvertAccessionToLocalIdsAll)
                    WX_MENU_ITEM_INT(eCmdConvertAccessionToLocalIdsNuc)
                    WX_MENU_ITEM_INT(eCmdConvertAccessionToLocalIdsProt)
                    WX_MENU_ITEM_INT(eCmdConvertAccessionToLocalIdsName)
                WX_END_SUBMENU()
            WX_END_SUBMENU()
            WX_SUBMENU_INT("Convert Second Protein Name and Description")
                WX_MENU_ITEM_INT(eCmdConvertSecondProtNameToDesc)
                WX_MENU_ITEM_INT(eCmdConvertDescToSecondProtName)
            WX_END_SUBMENU()
            WX_SUBMENU_INT("Convert Descriptors and Features")
                WX_SUBMENU_INT("Descriptor to Feature")
                    WX_MENU_ITEM_INT(eCmdConvertDescToFeatComment)
                    WX_MENU_ITEM_INT(eCmdConvertDescToFeatSource)
                    WX_SUBMENU_INT("Publication")
                        WX_MENU_ITEM_INT(eCmdConvertDescToFeatPub)
                        WX_MENU_ITEM_INT(eCmdConvertDescToFeatPubConstraint)
                    WX_END_SUBMENU()
                WX_END_SUBMENU()
                WX_SUBMENU_INT("Convert Full Length Feature to Descriptor")
                    WX_MENU_ITEM_INT(eCmdConvertFeatToDescSource)
                    WX_MENU_ITEM_INT(eCmdConvertFeatToDescComment)
                    WX_MENU_ITEM_INT(eCmdConvertFeatToDescPub)
                WX_END_SUBMENU()
            WX_END_SUBMENU()
        WX_END_SUBMENU()
        WX_SUBMENU_INT("&Edit")
            WX_MENU_ITEM_INT(eCmdEditFeatQual)
            WX_MENU_ITEM_INT(eCmdEditSourceQual)
            WX_MENU_ITEM_INT(eCmdEditCGPQual)
            WX_MENU_ITEM_INT(eCmdEditRNAQual)
            WX_MENU_ITEM_INT(eCmdMolInfoEdit)
            WX_MENU_ITEM_INT(eCmdTrimNsRich)
            WX_MENU_ITEM_INT(eCmdTrimNsTerminal)
            WX_SUBMENU_INT("Edit Feature")
                WX_MENU_ITEM_INT(eCmdEditFeatEv)
                WX_MENU_ITEM_INT(eCmdEditFeatLoc)
                WX_MENU_ITEM_INT(eCmdEditFeatStrand)
                WX_MENU_ITEM_INT(eCmdEditFeatRevInt)
                WX_MENU_ITEM_INT(eCmdEditFeatPseudo)
                WX_MENU_ITEM_INT(eCmdEditFeatException)
                WX_MENU_ITEM_INT(eCmdEditFeatExperiment)
                WX_MENU_ITEM_INT(eCmdEditFeatInference)
                WX_MENU_ITEM_INT(eCmdExtendCDS)
                WX_MENU_ITEM_INT(eCmdResolveIntersectingFeats)
            WX_END_SUBMENU()
            WX_SUBMENU_INT("Bulk Edit")
                WX_MENU_ITEM_INT(eCmdBulkSourceEdit)
                WX_MENU_ITEM_INT(eCmdBulkCdsEdit)
                WX_MENU_ITEM_INT(eCmdBulkGeneEdit)
                WX_MENU_ITEM_INT(eCmdBulkRnaEdit)
            WX_END_SUBMENU()
            WX_SUBMENU_INT("Extend Partial Features")
                WX_MENU_ITEM_INT(eCmdExtendPartialsConstr)
                WX_MENU_ITEM_INT(eCmdExtendPartialsAll)
            WX_END_SUBMENU()
            WX_MENU_ITEM_INT(eCmdCorrectGenes)
            WX_MENU_ITEM_INT(eCmdLowercaseQuals)
            WX_MENU_ITEM_INT(eCmdEditSequence)
            WX_MENU_ITEM_INT(eCmdEditSeqId)
            WX_MENU_ITEM_INT(eCmdEditHistory)
            WX_MENU_ITEM_INT(eCmdEditSeqEndsWithAlign)
            WX_MENU_ITEM_INT(eCmdEditSequenceEnds)
            WX_MENU_ITEM_INT(eCmdAddSecondary)
            WX_MENU_ITEM_INT(eCmdDuplicate)
            WX_MENU_ITEM_INT(eCmdCorrectIntervalOrder)
            WX_MENU_ITEM_INT(eCmdSuspectProductRulesEditor)
        WX_END_SUBMENU()
        WX_SUBMENU_INT("Authors and Publications")
            WX_MENU_ITEM_INT(eCmdEditPubs)
            WX_MENU_ITEM_INT(eCmdGlobalPubmedIdLookup)
            WX_SUBMENU_INT("Fix Capitalization")
                WX_MENU_ITEM_INT(eCmdFixCapitalizationAll)
                WX_MENU_ITEM_INT(eCmdFixCapitalizationAuthors)
                WX_MENU_ITEM_INT(eCmdFixCapitalizationTitles)
                WX_MENU_ITEM_INT(eCmdFixCapitalizationAffiliation)
                WX_MENU_ITEM_INT(eCmdFixCapitalizationCountry)
            WX_END_SUBMENU()
            WX_SUBMENU_INT("Remove Publications")
                WX_MENU_ITEM_INT(eCmdRemoveUnpublishedPublications)
                WX_MENU_ITEM_INT(eCmdRemoveInPressPublications)
                WX_MENU_ITEM_INT(eCmdRemovePublishedPublications)
                WX_MENU_ITEM_INT(eCmdRemoveCollidingPublications)
                WX_MENU_ITEM_INT(eCmdRemoveAllPublications)
            WX_END_SUBMENU()
            WX_MENU_ITEM_INT(eCmdRemoveAuthorConsortiums)
            WX_MENU_ITEM_INT(eCmdReverseAuthorNames)
            WX_MENU_ITEM_INT(eCmdStripAuthorSuffixes)
            WX_MENU_ITEM_INT(eCmdTruncateAuthorMiddleInitials)
            WX_SUBMENU_INT("Convert Author to Consortium")
                WX_MENU_ITEM_INT(eCmdConvertAuthorToConstortiumWhereLastName)
                WX_MENU_ITEM_INT(eCmdConvertAuthorToConsortiumAll)
            WX_END_SUBMENU()
            WX_MENU_ITEM_INT(eCmdAddCitSubForUpdate)
            WX_MENU_ITEM_INT(eCmdFixUsaAndStates)
        WX_END_SUBMENU()
        WX_SUBMENU_INT("CDS, Protein, and RNA")
            WX_SUBMENU_INT("Retranslate Coding Regions")
                WX_MENU_ITEM_INT(eCmdRetranslateCDSObeyStop)
                WX_MENU_ITEM_INT(eCmdRetranslateCDSIgnoreStopExceptEnd)
                WX_MENU_ITEM_INT(eCmdRetranslateCDSChooseFrame)
            WX_END_SUBMENU()
            WX_MENU_ITEM_INT(eCmdEditCdsFrame)
            WX_MENU_ITEM_INT(eCmdResynchronizePartialsCDS)
            WX_MENU_ITEM_INT(eCmdAddTranslExcept)
            WX_SUBMENU_INT("Adjust CDS for Consensus Splice Sites")
                WX_MENU_ITEM_INT(eCmdAdjustConsensusSpliceSitesStrict)
                WX_MENU_ITEM_INT(eCmdAdjustConsensusSpliceSitesRelaxed)
                WX_MENU_ITEM_INT(eCmdAdjustConsensusSpliceSitesConsensusEnds)
            WX_END_SUBMENU()
            WX_SUBMENU_INT("Adjust Features for Introns")
                WX_MENU_ITEM_INT(eCmdAdjustCDSForIntrons)
                WX_MENU_ITEM_INT(eCmdAdjustrRNAForIntrons)
                WX_MENU_ITEM_INT(eCmdAdjusttRNAForIntrons)
                WX_MENU_ITEM_INT(eCmdAdjustmRNAForIntrons)
            WX_END_SUBMENU()
            WX_MENU_ITEM_INT(eCmdCdsToMatPeptide)
            WX_MENU_ITEM_INT(eCmdTruncateCDS)
            WX_MENU_ITEM_INT(eCmdExtendCDSToStop)
            WX_SUBMENU_INT("Recompute Suggested Intervals")
                WX_MENU_ITEM_INT(eCmdRecomputeIntervalsAndUpdateGenes)
                WX_MENU_ITEM_INT(eCmdRecomputeIntervals)
            WX_END_SUBMENU()
            WX_SUBMENU_INT("Fix Product Name Capitalization")
                WX_MENU_ITEM_INT(eCmdFixProductNamesDefault)
                WX_MENU_ITEM_INT(eCmdFixProductNamesOptions)
            WX_END_SUBMENU()
            WX_MENU_ITEM_INT(eCmdRestoreRNAediting)
            WX_MENU_ITEM_INT(eCmdFixForTransSplicing)
        WX_END_SUBMENU()
        WX_SUBMENU_INT("Structured Comments")
            WX_MENU_ITEM_INT(eCmdApplyStructuredCommentField)
            WX_SUBMENU_INT("Edit Structured Comment Prefix-Suffix")
                WX_MENU_ITEM_INT(eCmdEditStructuredCommentPrefixSuffixHIVData)
                WX_MENU_ITEM_INT(eCmdEditStructuredCommentPrefixSuffixFluData)
                WX_MENU_ITEM_INT(eCmdEditStructuredCommentPrefixSuffixMIGSData)
                WX_MENU_ITEM_INT(eCmdEditStructuredCommentPrefixSuffixMIMSData)
                WX_MENU_ITEM_INT(eCmdEditStructuredCommentPrefixSuffixMIENSData)
                WX_MENU_ITEM_INT(eCmdEditStructuredCommentPrefixSuffixMIMARKS3_0)
                WX_MENU_ITEM_INT(eCmdEditStructuredCommentPrefixSuffixMIGS3_0)
                WX_MENU_ITEM_INT(eCmdEditStructuredCommentPrefixSuffixMIMS3_0)
                WX_MENU_ITEM_INT(eCmdEditStructuredCommentPrefixSuffixAssemblyData)
                WX_MENU_ITEM_INT(eCmdEditStructuredCommentPrefixSuffixGenAssData)
                WX_MENU_ITEM_INT(eCmdEditStructuredCommentPrefixSuffixiBol)
            WX_END_SUBMENU()
            WX_MENU_ITEM_INT(eCmdRemoveDuplicateStructuredComments)
            WX_MENU_ITEM_INT(eCmdRemoveAllStructuredComments)
            WX_MENU_ITEM_INT(eCmdRemoveEmptyStructuredComments)
            WX_MENU_ITEM_INT(eCmdModifyStructuredComment)
            WX_MENU_ITEM_INT(eCmdRemoveStructuredCommentKeyword)
            WX_MENU_ITEM_INT(eCmdAddStructuredCommentKeyword)
            WX_MENU_ITEM_INT(eCmdAddGenomeAssemblyStructuredComment)
            WX_MENU_ITEM_INT(eCmdAddAssemblyStructuredComment)
            WX_MENU_ITEM_INT(eCmdReorderStructuredComment)
            WX_MENU_ITEM_INT(eCmdConvertComment)
            WX_MENU_ITEM_INT(eCmdParseComment)
            WX_MENU_ITEM_INT(eCmdExportStructuredCommentsTable)
            WX_MENU_ITEM_INT(eCmdLoadStructComments)
        WX_END_SUBMENU()
        WX_SUBMENU_INT("Cleanup and Fixup")
            WX_MENU_ITEM_INT(eCmdTaxFixCleanup)
            WX_MENU_ITEM_INT(eCmdExtendedCleanup)
            WX_MENU_ITEM_INT(eCmdSpecificHostCleanup)
            WX_MENU_ITEM_INT(eCmdInstantiateProteinTitles)
            WX_MENU_ITEM_INT(eCmdUpdate_Replaced_EC_numbers)
        WX_MENU_ITEM_INT(eCmdRemoveDuplicateGOTerms)
        WX_END_SUBMENU()
        WX_SUBMENU_INT("Display")
           WX_MENU_ITEM_INT(eCmdViewSortedProteins)
           WX_MENU_ITEM_INT(eCmdShowFeatureTable)
           WX_MENU_ITEM_INT(eCmdMakeToolBarWindow)
           WX_MENU_CHECK_ITEM_INT(eCmdEnableHupId)
           WX_MENU_ITEM_INT(eCmdIndexerTypeSelection)
        WX_END_SUBMENU()
        WX_SUBMENU_INT("Desktop")
            WX_MENU_ITEM_INT(eCmdCorrectRNAStrand)
            WX_MENU_ITEM_INT(eCmdSegregateSets)
            WX_MENU_ITEM_INT(eCmdSequesterSets)
            WX_MENU_ITEM_INT(eCmdAddSet)
            WX_MENU_ITEM_INT(eCmdRemoveSegGaps)
            WX_MENU_ITEM_INT(eCmdRevCompSequences)
            WX_MENU_ITEM_INT(eCmdChangeSetClass)
            WX_MENU_ITEM_INT(eCmdGroupExplode)
            WX_MENU_ITEM_INT(eCmdFindASN1)
            WX_MENU_ITEM_INT(eCmdRemoveSetsFromSet)
            WX_MENU_ITEM_INT(eCmdReorderSeqById)
            WX_MENU_ITEM_INT(eCmdUpdateAlign)
            WX_MENU_ITEM_INT(eCmdAlignmentAssistant)
            WX_MENU_ITEM_INT(eCmdSaveDescriptors)
            WX_MENU_ITEM_INT(eCmdDescriptorPropagateDown)
            WX_SUBMENU_INT("Utilities")
                WX_MENU_ITEM_INT(eCmdBioseqFeatsRevComp)
                WX_MENU_ITEM_INT(eCmdBioseqOnlyRevComp)
                WX_MENU_ITEM_INT(eCmdCheckForMixedStrands)
            WX_END_SUBMENU()
            WX_SUBMENU_INT("Indexer")
                WX_MENU_ITEM_INT(eCmdRemoveSet)
            WX_END_SUBMENU()
        WX_END_SUBMENU()
        WX_SUBMENU_INT("Link")
            WX_SUBMENU_INT("Feature IDs")
                WX_MENU_ITEM_INT(eCmdAssignFeatureIds)
                WX_MENU_ITEM_INT(eCmdClearFeatureIds)
                WX_MENU_ITEM_INT(eCmdReassignFeatureIds)
                WX_MENU_ITEM_INT(eCmdUniqifyFeatureIds)
                WX_MENU_ITEM_INT(eCmdFixNonReciprocalLinks)
            WX_END_SUBMENU()
            WX_SUBMENU_INT("Create CDS-mRNA Links")
                WX_MENU_ITEM_INT(eCmdLinkmRNACDSOverlap)
                WX_MENU_ITEM_INT(eCmdLinkmRNACDSProduct)
                WX_MENU_ITEM_INT(eCmdLinkmRNACDSLabel)
                WX_MENU_ITEM_INT(eCmdLinkmRNACDSLabelLocation)
                WX_MENU_ITEM_INT(eCmdLinkmRNACDSProteinID)
                WX_MENU_ITEM_INT(eCmdLinkmRNACDSTranscriptID)
                WX_MENU_ITEM_INT(eCmdLinkmRNACDSUnidirectional)
            WX_END_SUBMENU()
            WX_MENU_ITEM_INT(eCmdLinkSelectedmRNACDS)
            WX_MENU_ITEM_INT(eCmdShowLinkedmRNACDS)
        WX_END_SUBMENU()
        WX_SUBMENU_INT("Projects")
            WX_MENU_ITEM_INT(eCmdAddFluComments)
            WX_MENU_ITEM_INT(eCmdSetGlobalRefGeneStatus)
            WX_MENU_ITEM_INT(eCmdCreateSeqHistForTpaDetailed)
            WX_MENU_ITEM_INT(eCmdCreateSeqHistForTpa)
            WX_MENU_ITEM_INT(eCmdRemoveSeqHistAssembly)
            WX_MENU_ITEM_INT(eCmdBarcodeTool)
            WX_MENU_ITEM_INT(eCmdApplyFBOL)
            WX_MENU_ITEM_INT(eCmdFarPointerSeq)
            WX_MENU_ITEM_INT(eCmdConvertBadCdsAndRnaToMiscFeat)
        WX_END_SUBMENU()
        WX_SUBMENU_INT("Make Features and Fuse")
            WX_MENU_ITEM_INT(eCmdCombineSelectedGenesIntoPseudogenes)
            WX_MENU_ITEM_INT(eCmdGeneFromOtherFeat)
            WX_MENU_ITEM_INT(eCmdGeneFeatFromXrefs)
            WX_MENU_ITEM_INT(eCmdGeneXrefsFromFeats)
            WX_MENU_ITEM_INT(eCmdCdsFromGeneMrnaExon)
            WX_MENU_ITEM_INT(eCmdmRNAFromGeneCdsExon)
            WX_MENU_ITEM_INT(eCmdtRNAFromGene)
            WX_MENU_ITEM_INT(eCmdFuseFeatures)
            WX_MENU_ITEM_INT(eCmdFuseJoinsInLocs)
            WX_MENU_ITEM_INT(eCmdExplodeRNAFeats)
            WX_SUBMENU_INT("Exon features from")
                WX_MENU_ITEM_INT(eCmdExonFromCds)
                WX_MENU_ITEM_INT(eCmdExonFromMRNA)
                WX_MENU_ITEM_INT(eCmdExonFromTRNA)
            WX_END_SUBMENU()
        WX_END_SUBMENU()
        WX_SUBMENU_INT("Normalize, Map, Package")
            WX_SUBMENU_INT("CDSGeneRangeError")
                WX_MENU_ITEM_INT(eCmdCDSGeneRangeErrorSuppress)
                WX_MENU_ITEM_INT(eCmdCDSGeneRangeErrorRestore)
            WX_END_SUBMENU()
            WX_MENU_ITEM_INT(eCmdSuppressGenes)
            WX_MENU_ITEM_INT(eCmdPackageFeaturesOnParts)
        WX_END_SUBMENU()
        WX_SUBMENU_INT("Gap Sequences")
            WX_SUBMENU_INT("Raw Sequence To Delta")
                WX_MENU_ITEM_INT(eCmdRawSeqToDeltaByNs)
                WX_MENU_ITEM_INT(eCmdRawSeqToDeltaByLoc)
                WX_MENU_ITEM_INT(eCmdRawSeqToDeltaByAssemblyGapFeatures)
            WX_END_SUBMENU()
            WX_MENU_ITEM_INT(eCmdDeltaSeqToRaw)
            WX_MENU_ITEM_INT(eCmdExpandGaps)
            WX_SUBMENU_INT("Convert Selected Gaps")
                WX_MENU_ITEM_INT(eCmdConvertSelectedGapsToKnown)
                WX_MENU_ITEM_INT(eCmdConvertSelectedGapsToUnknown)
            WX_END_SUBMENU()
            WX_MENU_ITEM_INT(eCmdConvertGapsBySize)
            WX_MENU_ITEM_INT(eCmdEditSelectedGaps)
            WX_MENU_ITEM_INT(eCmdAddLinkageToGaps)
            WX_MENU_ITEM_INT(eCmdCombineAdjacentGaps)
            WX_MENU_ITEM_INT(eCmdAdjustFeaturesForGaps)
            WX_MENU_ITEM_INT(eCmdConvertCdsWithGapsToMiscFeat)
            WX_MENU_ITEM_INT(eCmdSplitCDSwithTooManyXs)
        WX_END_SUBMENU()
        WX_MENU_ITEM_INT(eCmdSortUniqueCountTree)
        WX_MENU_ITEM_INT(eCmdSortUniqueCountText)
        WX_MENU_ITEM_INT(eCmdDiscrepancy)
        WX_MENU_ITEM_INT(eCmdOncaller)
        WX_MENU_ITEM_INT(eCmdSubmitter)
        WX_MENU_ITEM_INT(eCmdMega)
        WX_MENU_ITEM_INT(eCmdDiscrepancyList)
        WX_MENU_ITEM_INT(eCmdValidate)
        WX_MENU_ITEM_INT(eCmdMacroEditor)
#ifdef _DEBUG
        WX_MENU_ITEM_INT(eCmdPrepareSeqSubmit)
        WX_MENU_ITEM_INT(eCmdTestDialogView)
        WX_MENU_ITEM_INT(eCmdTestFormView)
#endif
        WX_SUBMENU_INT("Select")
            WX_MENU_ITEM_INT(eCmdSelectFeature)
        WX_END_SUBMENU()
        WX_SUBMENU_INT("Locus-tags")
            WX_MENU_ITEM_INT(eCmdCreateProteinId)
            WX_MENU_ITEM_INT(eCmdCreateLocusTagGene)
            WX_MENU_ITEM_INT(eCmdNormalizeGeneQuals)
        WX_END_SUBMENU()
        WX_MENU_ITEM_INT(eCmdEditingButtons)
        WX_MENU_ITEM_INT(eCmdSaveASN1File)
    WX_END_SUBMENU()
    WX_SUBMENU_INT("&Annotate")
        WX_SUBMENU_INT("Genes and Named Regions")
            WX_MENU_ITEM_INT(eCmdCreateGene)
            WX_MENU_ITEM_INT(eCmdCreateProt)
            WX_MENU_ITEM_INT(eCmdCreateRepeatRegion)
            WX_MENU_ITEM_INT(eCmdCreateStemLoop)
            WX_MENU_ITEM_INT(eCmdCreateSTS)
            WX_MENU_ITEM_INT(eCmdCreateRegion)
            WX_MENU_ITEM_INT(eCmdCreateSecondaryStructure)
            WX_MENU_ITEM_INT(eCmdCreateProprotein)
            WX_MENU_ITEM_INT(eCmdCreateMatPeptide)
            WX_MENU_ITEM_INT(eCmdCreateSigPeptide)
            WX_MENU_ITEM_INT(eCmdCreateTransitPeptide)
            WX_MENU_ITEM_INT(eCmdCreateOperon)
            WX_MENU_ITEM_INT(eCmdCreateMobileElement)
            WX_MENU_ITEM_INT(eCmdCreateCentromere)
            WX_MENU_ITEM_INT(eCmdCreateTelomere)
            WX_MENU_ITEM_INT(eCmdCreateRegulatory)
        WX_END_SUBMENU()
        WX_SUBMENU_INT("Coding Regions and Transcripts")
              WX_MENU_ITEM_INT(eCmdCreateCodingRegion)
              WX_MENU_ITEM_INT(eCmdCreatePreRNA)
              WX_MENU_ITEM_INT(eCmdCreateMRNA)
              WX_MENU_ITEM_INT(eCmdCreateExon)
              WX_MENU_ITEM_INT(eCmdCreateIntron)
              WX_MENU_ITEM_INT(eCmdCreate3UTR)
              WX_MENU_ITEM_INT(eCmdCreate5UTR)
        WX_END_SUBMENU()
        WX_SUBMENU_INT("Structural RNAs")
              WX_MENU_ITEM_INT(eCmdCreatetRNA)
              WX_MENU_ITEM_INT(eCmdCreaterRNA)
              WX_MENU_ITEM_INT(eCmdCreateRNA)
              WX_MENU_ITEM_INT(eCmdCreatencRNA)
              WX_MENU_ITEM_INT(eCmdCreatetmRNA)
        WX_END_SUBMENU()
        WX_SUBMENU_INT("Bibliographic and Comments")
              WX_MENU_ITEM_INT(eCmdCreateCommentDescriptor)
              WX_MENU_ITEM_INT(eCmdCreateBiosourceFeat)
        WX_END_SUBMENU()
        WX_SUBMENU_INT("Sites and Bonds")
              WX_MENU_ITEM_INT(eCmdCreatePolyASite)
              WX_MENU_ITEM_INT(eCmdCreatePrimerBind)
              WX_MENU_ITEM_INT(eCmdCreateProteinBind)
              WX_MENU_ITEM_INT(eCmdCreateRepOrigin)
              WX_MENU_ITEM_INT(eCmdCreateBond)
              WX_MENU_ITEM_INT(eCmdCreateSite)
              WX_MENU_ITEM_INT(eCmdCreateOriT)
        WX_END_SUBMENU()
        WX_SUBMENU_INT("Remaining Features")
              WX_MENU_ITEM_INT(eCmdCreateCRegion)
              WX_MENU_ITEM_INT(eCmdCreateDLoop)
              WX_MENU_ITEM_INT(eCmdCreateDSegment)
              WX_MENU_ITEM_INT(eCmdCreateiDNA)
              WX_MENU_ITEM_INT(eCmdCreateJSegment)
              WX_MENU_ITEM_INT(eCmdCreateMiscBinding)
              WX_MENU_ITEM_INT(eCmdCreateMiscDifference)
              WX_MENU_ITEM_INT(eCmdCreateMiscFeature)
              WX_MENU_ITEM_INT(eCmdCreateMiscRecomb)
              WX_MENU_ITEM_INT(eCmdCreateMiscStructure)
              WX_MENU_ITEM_INT(eCmdCreateModifiedBase)
              WX_MENU_ITEM_INT(eCmdCreateNRegion)
              WX_MENU_ITEM_INT(eCmdCreatePrimTranscript)
              WX_MENU_ITEM_INT(eCmdCreateSRegion)
              WX_MENU_ITEM_INT(eCmdCreateUnsure)
              WX_MENU_ITEM_INT(eCmdCreateVRegion)
              WX_MENU_ITEM_INT(eCmdCreateVSegment)
              WX_MENU_ITEM_INT(eCmdCreateVariation)
        WX_END_SUBMENU()
        WX_SUBMENU_INT("Batch Feature Apply")
            WX_MENU_ITEM_INT(eCmdApplyCDS)
            WX_MENU_ITEM_INT(eCmdApplyAddRNA)
            WX_MENU_ITEM_INT(eCmdApplyAddOther)
            WX_MENU_ITEM_INT(eCmdApplyFeatQual)
            WX_MENU_ITEM_INT(eCmdApplySourceQual)
            WX_MENU_ITEM_INT(eCmdApplyCGPQual)
            WX_MENU_ITEM_INT(eCmdApplyRNAQual)            
        WX_END_SUBMENU()
        WX_SUBMENU_INT("Batch Feature Edit")
            WX_MENU_ITEM_INT(eCmdEditFeatQual)
            WX_MENU_ITEM_INT(eCmdEditSourceQual)
            WX_MENU_ITEM_INT(eCmdEditCGPQual)
            WX_MENU_ITEM_INT(eCmdEditRNAQual)
        WX_END_SUBMENU()
        WX_SUBMENU_INT("Publications")
            WX_MENU_ITEM_INT(eCmdCreateDescPub)
            WX_MENU_ITEM_INT(eCmdCreatePubFeatureLabeled)
        WX_END_SUBMENU()
        WX_SUBMENU_INT("Descriptors")
            WX_MENU_ITEM_INT(eCmdCreateDescTPAAssembly)
            WX_MENU_ITEM_INT(eCmdCreateDescStructuredComment)
            WX_MENU_ITEM_INT(eCmdCreateDescRefGeneTracking)
            WX_MENU_ITEM_INT(eCmdCreateDescDBLink)
            WX_MENU_ITEM_INT(eCmdCreateDescUnverified)
            WX_MENU_ITEM_INT(eCmdCreateDescAuthorizedAccess)
            WX_MENU_ITEM_INT(eCmdCreateDescGenomeAssemblyComment)
            WX_MENU_ITEM_INT(eCmdCreateDescGenBankBlock)
            WX_MENU_ITEM_INT(eCmdCreateDescComment)
            WX_MENU_ITEM_INT(eCmdCreateDescTitle)
            WX_MENU_ITEM_INT(eCmdCreateDescMolinfo)
            WX_MENU_ITEM_INT(eCmdCreateDescSource)
        WX_END_SUBMENU()
        WX_MENU_ITEM_INT(eCmdBulkApplyGenCode)
    WX_END_SUBMENU()
WX_END_MENU()

const SwxMenuItemRec* CSeqEditPackage::GetMenuDef() const
{
    return kEditingMenu;
}




void CSequenceEditingEventHandler::RemoveDescriptors(wxCommandEvent& evt)
{
    x_SetUpTSEandUnDoManager(m_Workbench);       // perhaps this is unnecessary here after IsObjectSelected has run
    if (!m_TopSeqEntry || ! m_CmdProccessor) return;
    CRemoveDescDlg dlg(NULL, m_TopSeqEntry);
    if (dlg.ShowModal() == wxID_OK) 
    {
        CRef<CCmdComposite> cmd = dlg.GetCommand();
        if (cmd)  m_CmdProccessor->Execute(cmd);                       
        dlg.Close();        
    }
}

void CSequenceEditingEventHandler::RemoveFeatures(wxCommandEvent& evt)
{
    if (!m_Workbench) return;
    wxWindow* main_window = NULL;
    CWorkbench* wb_frame = dynamic_cast<CWorkbench*>(m_Workbench);
    if (wb_frame)
        main_window = wb_frame->GetMainWindow();
    CRemoveFeaturesDlg *dlg = new CRemoveFeaturesDlg(main_window, m_Workbench);
    dlg->Show(true);
}

void CSequenceEditingEventHandler::ApplyIndexerComments(wxCommandEvent& evt)
{
    if (!m_Workbench) return;
    wxWindow* main_window = NULL;
    CWorkbench* wb_frame = dynamic_cast<CWorkbench*>(m_Workbench);
    if (wb_frame)
        main_window = wb_frame->GetMainWindow();
    CApplyIndexerComments *dlg = new CApplyIndexerComments(main_window, m_Workbench);
    dlg->Show(true);
}



void CSequenceEditingEventHandler::RemoveAllFeatures(wxCommandEvent& evt)
{
    x_SetUpTSEandUnDoManager(m_Workbench);       // perhaps this is unnecessary here after IsObjectSelected has run
    if (!m_TopSeqEntry || ! m_CmdProccessor) return;
    CRemoveFeaturesDlg::RemoveAllFeatures(m_TopSeqEntry, m_CmdProccessor);
}

void CSequenceEditingEventHandler::RemoveSeqAnnotIds( wxCommandEvent& event )
{
    x_SetUpTSEandUnDoManager(m_Workbench);       // perhaps this is unnecessary here after IsObjectSelected has run
    if (!m_TopSeqEntry || ! m_CmdProccessor) return;
    CRemoveSeqAnnot worker;
    if (!worker.apply(m_TopSeqEntry,m_CmdProccessor,"Remove Seq-Annot Ids",CSeq_annot::C_Data::e_Ids))
        event.Skip();
}

void CSequenceEditingEventHandler::RemoveBankitComment( wxCommandEvent& event )
{
    x_SetUpTSEandUnDoManager(m_Workbench);       // perhaps this is unnecessary here after IsObjectSelected has run
    if (!m_TopSeqEntry || ! m_CmdProccessor) return;
    CRemoveBankitComment worker;
    if (!worker.apply(m_TopSeqEntry,m_CmdProccessor,"Remove Bankit Comment"))
        event.Skip();
}


void CSequenceEditingEventHandler::RemoveDuplicateStructuredComments( wxCommandEvent& event )
{
    x_SetUpTSEandUnDoManager(m_Workbench);       // perhaps this is unnecessary here after IsObjectSelected has run
    if (!m_TopSeqEntry || ! m_CmdProccessor) return;
    CRemoveDuplicateStructuredComment worker;
    if (!worker.apply(m_TopSeqEntry,m_CmdProccessor,"Remove Duplicate Structured Comment"))
        event.Skip();
}

void CSequenceEditingEventHandler::RemoveSequencesFromAlignments( wxCommandEvent& event )
{  
    x_SetUpTSEandUnDoManager(m_Workbench);       // perhaps this is unnecessary here after IsObjectSelected has run
    if (!m_TopSeqEntry || ! m_CmdProccessor) return;

    CRemoveSeqFromAlignDlg dlg(NULL, m_TopSeqEntry);
    if (dlg.ShowModal() == wxID_OK) 
    {
        CRef<CCmdComposite> cmd = dlg.GetCommand();
        if (cmd)  m_CmdProccessor->Execute(cmd);                       
        dlg.Close();        
    }
}  

void CSequenceEditingEventHandler::RemoveSequences( wxCommandEvent& event )
{
    x_SetUpTSEandUnDoManager(m_Workbench);       // perhaps this is unnecessary here after IsObjectSelected has run
    if (!m_TopSeqEntry || ! m_CmdProccessor) return;

    wxWindow* main_window = NULL;
    CWorkbench* wb_frame = dynamic_cast<CWorkbench*>(m_Workbench);
    if (wb_frame)
        main_window = wb_frame->GetMainWindow();
    
    CRemoveSequencesDlg dlg(main_window, m_TopSeqEntry);
    if (dlg.ShowModal() == wxID_OK) {
        CRef<CCmdComposite> cmd = dlg.GetCommand();
        if (cmd) {
            m_CmdProccessor->Execute(cmd);                       
        }        
    }
}

void CSequenceEditingEventHandler::RemoveSeqAnnotAlign( wxCommandEvent& event )
{
    x_SetUpTSEandUnDoManager(m_Workbench);       // perhaps this is unnecessary here after IsObjectSelected has run
    if (!m_TopSeqEntry || ! m_CmdProccessor) return;

    CRemoveSeqAnnot worker;
    if (!worker.apply(m_TopSeqEntry,m_CmdProccessor,"Remove Alignments",CSeq_annot::C_Data::e_Align))
        event.Skip();
}

void CSequenceEditingEventHandler::RemoveSeqAnnotGraph( wxCommandEvent& event )
{
    x_SetUpTSEandUnDoManager(m_Workbench);       // perhaps this is unnecessary here after IsObjectSelected has run
    if (!m_TopSeqEntry || ! m_CmdProccessor) return;

    CRemoveSeqAnnot worker;
    if (!worker.apply(m_TopSeqEntry,m_CmdProccessor,"Remove Graphs",CSeq_annot::C_Data::e_Graph))
        event.Skip();
}

void CSequenceEditingEventHandler::RemoveGenomeProjectsDB( wxCommandEvent& event )
{
    x_SetUpTSEandUnDoManager(m_Workbench);       // perhaps this is unnecessary here after IsObjectSelected has run
    if (!m_TopSeqEntry || ! m_CmdProccessor) return;

    CRemoveGenomeProjectsDB worker;
    if (!worker.apply(m_TopSeqEntry,m_CmdProccessor,"Remove GenomeProjectsDB"))
        event.Skip();
}

void CSequenceEditingEventHandler::RemoveEmptyGenomeProjectsDB( wxCommandEvent& event )
{
    x_SetUpTSEandUnDoManager(m_Workbench);       // perhaps this is unnecessary here after IsObjectSelected has run
    if (!m_TopSeqEntry || ! m_CmdProccessor) return;

    CRemoveEmptyGenomeProjectsDB worker;
    if (!worker.apply(m_TopSeqEntry,m_CmdProccessor,"Remove Empty GenomeProjectsDB"))
        event.Skip();
}


void CSequenceEditingEventHandler::TaxFixCleanup( wxCommandEvent& evt )
{
    CRef<CCmdComposite> cmd = CleanupCommand (m_TopSeqEntry, true, (evt.GetId() == eCmdTaxFixCleanup || evt.GetId() == eCmdTaxFixCleanup_ext));
    if (cmd) {
        m_CmdProccessor->Execute(cmd);
    }
}


void CSequenceEditingEventHandler::SpecificHostCleanup( wxCommandEvent& evt )
{
    CRef<CCmdComposite> cmd = SpecificHostCleanupCommand (m_TopSeqEntry);
    if (cmd) {
        m_CmdProccessor->Execute(cmd);
    }
}


// Requested in GB-7591
void CSequenceEditingEventHandler::FixNonReciprocalLinks( wxCommandEvent& evt )
{
    CConstRef<CSeq_entry> entry = m_TopSeqEntry.GetCompleteSeq_entry();
    CRef<CSeq_entry> copy(new CSeq_entry());
    copy->Assign(*entry);
    
    CRef<CObjectManager> objmgr = CObjectManager::GetInstance();
    CScope scope2(*objmgr);
    scope2.AddDefaults(); 
    CSeq_entry_Handle new_seh = scope2.AddTopLevelSeqEntry(*copy);

    CCleanup::RepairXrefs(new_seh);
        
    CRef<CCmdComposite> cmd(new CCmdComposite("Cleanup"));
    CRef<CCmdChangeSeqEntry> clean(new CCmdChangeSeqEntry(m_TopSeqEntry, copy));
    cmd->AddCommand(*clean);
    if (cmd) {
        m_CmdProccessor->Execute(cmd);
    }
}


void CSequenceEditingEventHandler::DisableStrainForwarding( wxCommandEvent& evt )
{
    x_SetUpTSEandUnDoManager(m_Workbench);       // perhaps this is unnecessary here after IsObjectSelected has run
    if (!m_TopSeqEntry || ! m_CmdProccessor) return;

    CRef<CCmdComposite> cmd = GetDisableStrainForwardingCommand(m_TopSeqEntry);
    if (cmd) {
        m_CmdProccessor->Execute(cmd);
    }
}


void CSequenceEditingEventHandler::EnableStrainForwarding(wxCommandEvent& evt)
{
    x_SetUpTSEandUnDoManager(m_Workbench);       // perhaps this is unnecessary here after IsObjectSelected has run
    if (!m_TopSeqEntry || !m_CmdProccessor) return;

    CRef<CCmdComposite> cmd = GetEnableStrainForwardingCommand(m_TopSeqEntry);
    if (cmd) {
        m_CmdProccessor->Execute(cmd);
    }
}


void CSequenceEditingEventHandler::EditPubs( wxCommandEvent& evt )
{
    CEditPubDlg * dlg = new CEditPubDlg(NULL, m_Workbench);

    dlg->Show(true);
}


void CSequenceEditingEventHandler::FixCapitalizationAll( wxCommandEvent& event )
{
    x_SetUpTSEandUnDoManager(m_Workbench);       // perhaps this is unnecessary here after IsObjectSelected has run
    if (!m_TopSeqEntry || ! m_CmdProccessor) return;

    CFixAllCap worker;
    CRef<CCmdComposite> cmd = worker.GetCommand(m_TopSeqEntry);
    if (cmd) {
        m_CmdProccessor->Execute(cmd);
    }
}

void CSequenceEditingEventHandler::FixCapitalizationAuthors( wxCommandEvent& event )
{
    x_SetUpTSEandUnDoManager(m_Workbench);       // perhaps this is unnecessary here after IsObjectSelected has run
    if (!m_TopSeqEntry || ! m_CmdProccessor) return;

    CFixAuthorCap worker;
    CRef<CCmdComposite> cmd = worker.GetCommand(m_TopSeqEntry, m_SeqSubmit);
    if (cmd) {
        m_CmdProccessor->Execute(cmd);
    }
}

void CSequenceEditingEventHandler::FixCapitalizationTitles( wxCommandEvent& event )
{
    x_SetUpTSEandUnDoManager(m_Workbench);       // perhaps this is unnecessary here after IsObjectSelected has run
    if (!m_TopSeqEntry || ! m_CmdProccessor) return;

    CFixTitleCap worker;
    CRef<CCmdComposite> cmd = worker.GetCommand(m_TopSeqEntry);
    if (cmd) {
        m_CmdProccessor->Execute(cmd);
    }
}

void CSequenceEditingEventHandler::FixCapitalizationAffiliation( wxCommandEvent& event )
{
    x_SetUpTSEandUnDoManager(m_Workbench);       // perhaps this is unnecessary here after IsObjectSelected has run
    if (!m_TopSeqEntry || ! m_CmdProccessor) return;

    CFixAffilCap worker;
    CRef<CCmdComposite> cmd = worker.GetCommand(m_TopSeqEntry, m_SeqSubmit);
    if (cmd) {
        m_CmdProccessor->Execute(cmd);
    }
}

void CSequenceEditingEventHandler::FixCapitalizationCountry( wxCommandEvent& event )
{
    x_SetUpTSEandUnDoManager(m_Workbench);       // perhaps this is unnecessary here after IsObjectSelected has run
    if (!m_TopSeqEntry || ! m_CmdProccessor) return;

    CFixCountryCap worker;
    CRef<CCmdComposite> cmd = worker.GetCommand(m_TopSeqEntry, m_SeqSubmit);
    if (cmd) {
        m_CmdProccessor->Execute(cmd);
    }
}

void CSequenceEditingEventHandler::RemoveUnpublishedPublications( wxCommandEvent& event )
{
    x_SetUpTSEandUnDoManager(m_Workbench);       // perhaps this is unnecessary here after IsObjectSelected has run
    if (!m_TopSeqEntry || ! m_CmdProccessor) return;

    CRemoveUnpublishedPubs worker;
    CRef<CCmdComposite> cmd = worker.GetCommand(m_TopSeqEntry);
    if (cmd) {
        m_CmdProccessor->Execute(cmd);
    }
}

void CSequenceEditingEventHandler::RemoveInPressPublications( wxCommandEvent& event )
{
    x_SetUpTSEandUnDoManager(m_Workbench);       // perhaps this is unnecessary here after IsObjectSelected has run
    if (!m_TopSeqEntry || ! m_CmdProccessor) return;

    CRemoveInPressPubs worker;
    CRef<CCmdComposite> cmd = worker.GetCommand(m_TopSeqEntry);
    if (cmd) {
        m_CmdProccessor->Execute(cmd);
    }
}

void CSequenceEditingEventHandler::RemovePublishedPublications( wxCommandEvent& event )
{
    x_SetUpTSEandUnDoManager(m_Workbench);       // perhaps this is unnecessary here after IsObjectSelected has run
    if (!m_TopSeqEntry || ! m_CmdProccessor) return;

    CRemovePublishedPubs worker;
    CRef<CCmdComposite> cmd = worker.GetCommand(m_TopSeqEntry);
    if (cmd) {
        m_CmdProccessor->Execute(cmd);
    }
}

void CSequenceEditingEventHandler::RemoveCollidingPublications( wxCommandEvent& event )
{
    x_SetUpTSEandUnDoManager(m_Workbench);       // perhaps this is unnecessary here after IsObjectSelected has run
    if (!m_TopSeqEntry || ! m_CmdProccessor) return;

    CRemoveCollidingPubs worker;
    CRef<CCmdComposite> cmd = worker.GetCommand(m_TopSeqEntry);
    if (cmd) {
        m_CmdProccessor->Execute(cmd);
    }
}


void CSequenceEditingEventHandler::RemoveAllPublications( wxCommandEvent& event )
{
    x_SetUpTSEandUnDoManager(m_Workbench);       // perhaps this is unnecessary here after IsObjectSelected has run
    if (!m_TopSeqEntry || ! m_CmdProccessor) return;

    CRemoveAllPubs worker;
    CRef<CCmdComposite> cmd = worker.GetCommand(m_TopSeqEntry);
    if (cmd) {
        m_CmdProccessor->Execute(cmd);
    }
}

void CSequenceEditingEventHandler::RemoveAuthorConsortiums(wxCommandEvent& event)
{
    x_SetUpTSEandUnDoManager(m_Workbench);       // perhaps this is unnecessary here after IsObjectSelected has run
    if (!m_TopSeqEntry || !m_CmdProccessor) return;

    CRemoveAuthorConsortium worker;
    CRef<CCmdComposite> cmd = worker.GetCommand(m_TopSeqEntry);
    if (cmd) {
        m_CmdProccessor->Execute(cmd);
    }
}

void CSequenceEditingEventHandler::ReverseAuthorNames( wxCommandEvent& event )
{
    x_SetUpTSEandUnDoManager(m_Workbench);       // perhaps this is unnecessary here after IsObjectSelected has run
    if (!m_TopSeqEntry || ! m_CmdProccessor) return;

    CReverseAuthorNames worker;
    CRef<CCmdComposite> cmd = worker.GetCommand(m_TopSeqEntry, m_SeqSubmit);
    if (cmd) {
        m_CmdProccessor->Execute(cmd);
    }
}

void CSequenceEditingEventHandler::StripAuthorSuffixes( wxCommandEvent& event )
{
    x_SetUpTSEandUnDoManager(m_Workbench);       // perhaps this is unnecessary here after IsObjectSelected has run
    if (!m_TopSeqEntry || ! m_CmdProccessor) return;

    CStripAuthorSuffix worker;
    CRef<CCmdComposite> cmd = worker.GetCommand(m_TopSeqEntry, m_SeqSubmit);
    if (cmd) {
        m_CmdProccessor->Execute(cmd);
    }
}

void CSequenceEditingEventHandler::TruncateAuthorMiddleInitials( wxCommandEvent& event )
{
    x_SetUpTSEandUnDoManager(m_Workbench);       // perhaps this is unnecessary here after IsObjectSelected has run
    if (!m_TopSeqEntry || ! m_CmdProccessor) return;

    CFixAuthorInitials worker;
    CRef<CCmdComposite> cmd = worker.GetCommand(m_TopSeqEntry, m_SeqSubmit);
    if (cmd) {
        m_CmdProccessor->Execute(cmd);
    }
}

void CSequenceEditingEventHandler::ConvertAuthorToConstortiumWhereLastName(wxCommandEvent& event)
{
    x_SetUpTSEandUnDoManager(m_Workbench);       // perhaps this is unnecessary here after IsObjectSelected has run
    if (!m_TopSeqEntry || !m_CmdProccessor) return;

    CConvertAuthorToConsortiumWhereConstraint worker;
    worker.SetConstraint("consortium");
    CRef<CCmdComposite> cmd = worker.GetCommand(m_TopSeqEntry, m_SeqSubmit);
    if (cmd) {
        m_CmdProccessor->Execute(cmd);
    }
}

void CSequenceEditingEventHandler::ConvertAuthorToConsortiumAll( wxCommandEvent& event )
{
    x_SetUpTSEandUnDoManager(m_Workbench);       // perhaps this is unnecessary here after IsObjectSelected has run
    if (!m_TopSeqEntry || ! m_CmdProccessor) return;
    
    CConvertAllAuthorToConsortium worker;
    CRef<CCmdComposite> cmd = worker.GetCommand(m_TopSeqEntry);
    if (cmd) {
        m_CmdProccessor->Execute(cmd);
    }
}

void CSequenceEditingEventHandler::AddCitSubForUpdate( wxCommandEvent& event )
{
    x_SetUpTSEandUnDoManager(m_Workbench);       // perhaps this is unnecessary here after IsObjectSelected has run
    if (!m_TopSeqEntry || ! m_CmdProccessor) return;

    CAddCitSubForUpdate worker;
    if (!worker.apply(m_TopSeqEntry,m_CmdProccessor,"Add Cit-sub for update"))
        event.Skip();
}

void CSequenceEditingEventHandler::FixUsaAndStates( wxCommandEvent& event )
{
    x_SetUpTSEandUnDoManager(m_Workbench);       // perhaps this is unnecessary here after IsObjectSelected has run
    if (!m_TopSeqEntry || ! m_CmdProccessor) return;

    CFixUSA worker;
    CRef<CCmdComposite> cmd = worker.GetCommand(m_TopSeqEntry, m_SeqSubmit);
    if (cmd) {
        m_CmdProccessor->Execute(cmd);
    }
}

void CSequenceEditingEventHandler::ResynchronizePartials( wxCommandEvent& event )
{
    x_SetUpTSEandUnDoManager(m_Workbench);       // perhaps this is unnecessary here after IsObjectSelected has run
    if (!m_TopSeqEntry || ! m_CmdProccessor) return;

    CRef<CMiscSeqTableColumn> col(new CMiscSeqTableColumn("CDS comment"));
    vector<CConstRef<CObject> > objs = col->GetObjects(m_TopSeqEntry, "", CRef<edit::CStringConstraint>(NULL));
    if (objs.size() == 0) {
        wxMessageBox(wxT("No coding regions found!"), wxT("Error"),
                     wxOK | wxICON_ERROR, NULL);
        return;
    }

    bool any_change = false;
    CRef<CCmdComposite> cmd(new CCmdComposite("Resynch partials"));
    ITERATE(vector<CConstRef<CObject> >, it, objs) {
        const CSeq_feat* f = dynamic_cast<const CSeq_feat* >((*it).GetPointer());
        CRef<CSeq_feat> new_feat(new CSeq_feat());
        new_feat->Assign(*f);
        if (feature::AdjustFeaturePartialFlagForLocation(*new_feat)) {
            CSeq_feat_Handle fh = m_TopSeqEntry.GetScope().GetSeq_featHandle(*f);
            cmd->AddCommand(*CRef<CCmdChangeSeq_feat>(new CCmdChangeSeq_feat(fh, *new_feat)));
            any_change = true;
        }

        CRef<CCmdComposite> synch = GetSynchronizeProteinPartialsCommand(m_TopSeqEntry.GetScope(), *f);
        if (synch) {
            cmd->AddCommand(*synch);
            any_change = true;
        }
    }

    if (any_change) {
        m_CmdProccessor->Execute(cmd); 
    } else {
        wxMessageBox(wxT("No effect!"), wxT("Error"),
                     wxOK | wxICON_ERROR, NULL);
    }
    
}


void CSequenceEditingEventHandler::RetranslateCDS( wxCommandEvent& evt )
{
    x_SetUpTSEandUnDoManager(m_Workbench);       // perhaps this is unnecessary here after IsObjectSelected has run
    if (!m_TopSeqEntry || ! m_CmdProccessor) return;

    CRetranslateCDS worker;
    switch (evt.GetId())
    {
    case eCmdRetranslateCDSIgnoreStopExceptEnd :
    case eCmdRetranslateCDSIgnoreStopExceptEnd_ext :
        worker.apply( m_TopSeqEntry, m_CmdProccessor, "Retranslate coding regions", CRetranslateCDS::eRetranslateCDSIgnoreStopExceptEnd); break;
    case eCmdRetranslateCDSObeyStop : worker.apply( m_TopSeqEntry, m_CmdProccessor, "Retranslate coding regions", CRetranslateCDS::eRetranslateCDSObeyStop); break;
    case eCmdRetranslateCDSChooseFrame : worker.apply( m_TopSeqEntry, m_CmdProccessor, "Retranslate coding regions", CRetranslateCDS::eRetranslateCDSChooseFrame); break;
    default : break;
    }

}


void CSequenceEditingEventHandler::AddTranslExcept( wxCommandEvent& event )
{
    CAddTranslExceptDlg * dlg = new CAddTranslExceptDlg(NULL, m_Workbench);
    dlg->Show(true);
}


void CSequenceEditingEventHandler::RemoveAllStructuredComments( wxCommandEvent& event )
{
    x_SetUpTSEandUnDoManager(m_Workbench);       // perhaps this is unnecessary here after IsObjectSelected has run
    if (!m_TopSeqEntry || ! m_CmdProccessor) return;

    CRemoveAllStructuredComment worker;
    if (!worker.apply(m_TopSeqEntry,m_CmdProccessor,"Remove All Structured Comments"))
        event.Skip();
}

void CSequenceEditingEventHandler::RemoveEmptyStructuredComments( wxCommandEvent& event )
{
    x_SetUpTSEandUnDoManager(m_Workbench);       // perhaps this is unnecessary here after IsObjectSelected has run
    if (!m_TopSeqEntry || ! m_CmdProccessor) return;

    CRemoveEmptyStructuredComment worker;
    if (!worker.apply(m_TopSeqEntry,m_CmdProccessor,"Remove Empty Structured Comments"))
        event.Skip();
}

void CSequenceEditingEventHandler::ModifyStructuredComment( wxCommandEvent& event )
{
    x_SetUpTSEandUnDoManager(m_Workbench);       // perhaps this is unnecessary here after IsObjectSelected has run
    if (!m_TopSeqEntry || ! m_CmdProccessor) return;

    CModifyStructuredComment worker;
    if (!worker.apply(m_TopSeqEntry,m_CmdProccessor,"Remove Structured Comment Field"))
        event.Skip();
}

void CSequenceEditingEventHandler::RemoveStructuredCommentKeyword( wxCommandEvent& event )
{
    x_SetUpTSEandUnDoManager(m_Workbench);       // perhaps this is unnecessary here after IsObjectSelected has run
    if (!m_TopSeqEntry || ! m_CmdProccessor) return;

    CRemoveStructuredCommentKeyword worker;
    if (!worker.apply(m_TopSeqEntry,m_CmdProccessor,"Remove Structured Comment Keyword"))
        event.Skip();
}

void CSequenceEditingEventHandler::AddStructuredCommentKeyword( wxCommandEvent& event )
{
    x_SetUpTSEandUnDoManager(m_Workbench);       // perhaps this is unnecessary here after IsObjectSelected has run
    if (!m_TopSeqEntry || ! m_CmdProccessor) return;

    CAddStructuredCommentKeyword worker;
    if (!worker.apply(m_TopSeqEntry,m_CmdProccessor,"Add Structured Comment Keyword"))
        event.Skip();
}

void CSequenceEditingEventHandler::AddKeywordGDS( wxCommandEvent& event )
{
    x_SetUpTSEandUnDoManager(m_Workbench);       // perhaps this is unnecessary here after IsObjectSelected has run
    if (!m_TopSeqEntry || ! m_CmdProccessor) return;

    CAddConstKeyword worker;
    worker.SetKeyword("GDS");
    if (!worker.apply(m_TopSeqEntry,m_CmdProccessor,"Add Keyword GDS"))
        event.Skip();
}

void CSequenceEditingEventHandler::AddKeywordTPA_inferential( wxCommandEvent& event )
{
    x_SetUpTSEandUnDoManager(m_Workbench);       // perhaps this is unnecessary here after IsObjectSelected has run
    if (!m_TopSeqEntry || ! m_CmdProccessor) return;

    CAddConstKeyword worker;
    worker.SetKeyword("TPA:inferential");
    if (!worker.apply(m_TopSeqEntry,m_CmdProccessor,"Add Keyword TPA:inferential"))
        event.Skip();
}

void CSequenceEditingEventHandler::AddKeywordTPA_experimental( wxCommandEvent& event )
{
    x_SetUpTSEandUnDoManager(m_Workbench);       // perhaps this is unnecessary here after IsObjectSelected has run
    if (!m_TopSeqEntry || ! m_CmdProccessor) return;

    CAddConstKeyword worker;
    worker.SetKeyword("TPA:experimental");
    if (!worker.apply(m_TopSeqEntry,m_CmdProccessor,"Add Keyword TPA:experimental"))
        event.Skip();
}

void CSequenceEditingEventHandler::AddKeywordTPA_assembly( wxCommandEvent& event )
{
    x_SetUpTSEandUnDoManager(m_Workbench);       // perhaps this is unnecessary here after IsObjectSelected has run
    if (!m_TopSeqEntry || ! m_CmdProccessor) return;

    CAddConstKeyword worker;
    worker.SetKeyword("TPA:assembly");
    if (!worker.apply(m_TopSeqEntry,m_CmdProccessor,"Add Keyword TPA:assembly"))
        event.Skip();
}

void CSequenceEditingEventHandler::AddKeywordWithConstraint( wxCommandEvent& event )
{
    CAECRFrame::EActionType aecr_action_type =  CAECRFrame::eActionType_Apply;
    CAECRFrame::EFieldType aecr_field_type =  CAECRFrame::eFieldType_Misc;
    CAECRFrame * dlg = new CAECRFrame(NULL, m_Workbench); 
    dlg->SetInitParameters(aecr_action_type, aecr_field_type, ncbi::kGenbankBlockKeyword);
    CallAfter(&CSequenceEditingEventHandler::ShowAfter, dlg);
}

void CSequenceEditingEventHandler::AddGenomeAssemblyStructuredComment( wxCommandEvent& event )
{
    x_SetUpTSEandUnDoManager(m_Workbench);       
    if (!m_TopSeqEntry || ! m_CmdProccessor) return;

    CRef<CSeqdesc> seqdesc(new CSeqdesc());
    seqdesc->SetUser().Assign(*edit::CGenomeAssemblyComment::MakeEmptyUserObject());

    CSeq_entry_Handle seh = m_TopSeqEntry;
    vector<CRef<CSeq_loc> > &orig_locs = x_GetSelectedLocations();
    if (!orig_locs.empty()) 
    {
        CBioseq_Handle bsh = m_TopSeqEntry.GetScope().GetBioseqHandle(*orig_locs.front());
        seh = bsh.GetSeq_entry_Handle();
    }

    CIRef<IEditObject> editor(new CEditObjectSeq_desc(*seqdesc, seh, m_TopSeqEntry.GetScope(), true));
    CEditObjViewDlg* edit_dlg = new CEditObjViewDlg(NULL, true);
    edit_dlg->SetUndoManager(m_CmdProccessor);
    wxWindow* editorWindow = editor->CreateWindow(edit_dlg);
    edit_dlg->SetEditorWindow(editorWindow);
    edit_dlg->SetEditor(editor);
    edit_dlg->SetWorkDir(m_WorkDir);
    edit_dlg->Show(true);
}


void CSequenceEditingEventHandler::ReorderStructuredComment( wxCommandEvent& event )
{
    x_SetUpTSEandUnDoManager(m_Workbench);       // perhaps this is unnecessary here after IsObjectSelected has run
    if (!m_TopSeqEntry || ! m_CmdProccessor) return;

    CReorderStructuredComment worker;
    if (!worker.apply(m_TopSeqEntry,m_CmdProccessor,"Reorder Structured Comment"))
        event.Skip();
}

void CSequenceEditingEventHandler::ConvertComment( wxCommandEvent& event )
{
    x_SetUpTSEandUnDoManager(m_Workbench);       // perhaps this is unnecessary here after IsObjectSelected has run
    if (!m_TopSeqEntry || ! m_CmdProccessor) return;

    CConvertComment worker;
    if (!worker.apply(m_TopSeqEntry,m_CmdProccessor,"Convert Comment to Structured Comment", "::","  "))
        event.Skip();
}

void CSequenceEditingEventHandler::ParseComment( wxCommandEvent& event )
{
    x_SetUpTSEandUnDoManager(m_Workbench);       // perhaps this is unnecessary here after IsObjectSelected has run
    if (!m_TopSeqEntry || ! m_CmdProccessor) return;

    CConvertComment worker;
    if (!worker.apply(m_TopSeqEntry,m_CmdProccessor,"Parse Comment to Structured Comment", "=",";"))
        event.Skip();
}

void CSequenceEditingEventHandler::AssignFeatureIds( wxCommandEvent& event )
{
    x_SetUpTSEandUnDoManager(m_Workbench);       // perhaps this is unnecessary here after IsObjectSelected has run
    if (!m_TopSeqEntry || ! m_CmdProccessor) return;

    CAssignFeatureId worker;
    if (!worker.apply(m_TopSeqEntry,m_CmdProccessor,"Assign Feature Ids"))
        event.Skip();
}

void CSequenceEditingEventHandler::ClearFeatureIds( wxCommandEvent& event )
{
    x_SetUpTSEandUnDoManager(m_Workbench);       // perhaps this is unnecessary here after IsObjectSelected has run
    if (!m_TopSeqEntry || ! m_CmdProccessor) return;

    CClearFeatureId worker;
    if (!worker.apply(m_TopSeqEntry,m_CmdProccessor,"Clear Feature Ids"))
        event.Skip();
}

void CSequenceEditingEventHandler::ReassignFeatureIds( wxCommandEvent& event )
{
    x_SetUpTSEandUnDoManager(m_Workbench);       // perhaps this is unnecessary here after IsObjectSelected has run
    if (!m_TopSeqEntry || ! m_CmdProccessor) return;
    
    CReassignFeatureId worker;
    if (!worker.apply(m_TopSeqEntry,m_CmdProccessor,"Reassign Feature Ids"))
        event.Skip();
}

void CSequenceEditingEventHandler::UniqifyFeatureIds( wxCommandEvent& event )
{
    x_SetUpTSEandUnDoManager(m_Workbench);       // perhaps this is unnecessary here after IsObjectSelected has run
    if (!m_TopSeqEntry || ! m_CmdProccessor) return;

    CUniqifyFeatureId worker;
    if (!worker.apply(m_TopSeqEntry,m_CmdProccessor,"Uniqify Feature Ids"))
        event.Skip();
}

void CSequenceEditingEventHandler::TrimJunkInPrimerSeqs( wxCommandEvent& event )
{
    x_SetUpTSEandUnDoManager(m_Workbench);       // perhaps this is unnecessary here after IsObjectSelected has run
    if (!m_TopSeqEntry || ! m_CmdProccessor) return;

    CPCRPrimerTrimJunk worker;
    CRef<CCmdComposite> cmd = worker.GetCommand(m_TopSeqEntry);
    if (cmd) {
        m_CmdProccessor->Execute(cmd);
    }
}

void CSequenceEditingEventHandler::FixiInPrimerSeqs( wxCommandEvent& event )
{
    x_SetUpTSEandUnDoManager(m_Workbench);       // perhaps this is unnecessary here after IsObjectSelected has run
    if (!m_TopSeqEntry || ! m_CmdProccessor) return;

    CPCRPrimerFixI worker;
    CRef<CCmdComposite> cmd = worker.GetCommand(m_TopSeqEntry);
    if (cmd) {
        m_CmdProccessor->Execute(cmd);
    }
}

void CSequenceEditingEventHandler :: ItemAction(wxCommandEvent& event, CRef <CItem> item, const string& title)
{
    x_SetUpTSEandUnDoManager(m_Workbench);
    if (!m_TopSeqEntry || ! m_CmdProccessor) return;
    if (!(item.GetPointer()) || !(item->apply(m_TopSeqEntry, m_CmdProccessor, title)) ) 
          event.Skip();
};

void CSequenceEditingEventHandler :: RemoveCddFeatDbxref(wxCommandEvent& event )
{
   ItemAction(event, CRef <CItem> (new CRmvCddFeatDbxref), "Remove All CDD Features and Dbxrefs");
};

void CSequenceEditingEventHandler :: PrefixAuthWithTax(wxCommandEvent& event )
{
   ItemAction(event, CRef <CItem> (new CPrefixAuthority), "Prefix Authority with Organism");
};

void CSequenceEditingEventHandler :: FocusSet(wxCommandEvent& event )
{
   ItemAction(event, CRef <CItem> (new CSrcFocusSet()), "Set");
};

void CSequenceEditingEventHandler :: FocusClear(wxCommandEvent& event )
{
   ItemAction(event, CRef <CItem> (new CSrcFocusClear()), "Clear");
};

void CSequenceEditingEventHandler :: SetTransgSrcDesc(wxCommandEvent& event )
{
   ItemAction(event, CRef <CItem> (new CSetTransgSrcDesc()), "Set Transgenic on Source Desc");
};

void CSequenceEditingEventHandler :: SplitDblinkQuals(wxCommandEvent& event )
{
   ItemAction(event, CRef <CItem> (new CSplitDblinkQuals()), "Split DBLink Quals at Commas");
};

void CSequenceEditingEventHandler::FixOrgModInstitution( wxCommandEvent& event )
{
    x_SetUpTSEandUnDoManager(m_Workbench);       // perhaps this is unnecessary here after IsObjectSelected has run
    if (!m_TopSeqEntry || ! m_CmdProccessor) return;

    CFixOrgModInst worker;
    CRef<CCmdComposite> cmd = worker.GetCommand(m_TopSeqEntry);
    if (cmd) {
        m_CmdProccessor->Execute(cmd);
    }
}

void CSequenceEditingEventHandler::SwapPrimerSeqName( wxCommandEvent& event )
{
    x_SetUpTSEandUnDoManager(m_Workbench);       // perhaps this is unnecessary here after IsObjectSelected has run
    if (!m_TopSeqEntry || ! m_CmdProccessor) return;
    
    CSwapPrimerNameSeq worker;
    CRef<CCmdComposite> cmd = worker.GetCommand(m_TopSeqEntry);
    if (cmd) {
        m_CmdProccessor->Execute(cmd);
    }
}

void CSequenceEditingEventHandler::MergePrimerSets(wxCommandEvent& event)
{
    x_SetUpTSEandUnDoManager(m_Workbench);       // perhaps this is unnecessary here after IsObjectSelected has run
    if (!m_TopSeqEntry || !m_CmdProccessor) return;

    CMergePrimerSets worker;
    CRef<CCmdComposite> cmd = worker.GetCommand(m_TopSeqEntry);
    if (cmd) {
        m_CmdProccessor->Execute(cmd);
    }
}

void CSequenceEditingEventHandler::SplitPrimerSets(wxCommandEvent& event)
{
    x_SetUpTSEandUnDoManager(m_Workbench);       // perhaps this is unnecessary here after IsObjectSelected has run
    if (!m_TopSeqEntry || !m_CmdProccessor) return;

    CSplitPrimerSets worker;
    CRef<CCmdComposite> cmd = worker.GetCommand(m_TopSeqEntry);
    if (cmd) {
        m_CmdProccessor->Execute(cmd);
    }
}

void CSequenceEditingEventHandler::AppendModToOrg( wxCommandEvent& event )
{  
    x_SetUpTSEandUnDoManager(m_Workbench);       // perhaps this is unnecessary here after IsObjectSelected has run
    if (!m_TopSeqEntry || ! m_CmdProccessor) return;

    CAppendModToOrgDlg dlg(NULL, m_TopSeqEntry);
    if (dlg.ShowModal() == wxID_OK) 
    {
        CRef<CCmdComposite> cmd = dlg.GetCommand();
        if (cmd)  m_CmdProccessor->Execute(cmd);                       
        dlg.Close();        
    }
}  

void CSequenceEditingEventHandler::ParseCollectionDateMonthFirst(wxCommandEvent& event)
{
    x_SetUpTSEandUnDoManager(m_Workbench);       // perhaps this is unnecessary here after IsObjectSelected has run
    if (!m_TopSeqEntry || !m_CmdProccessor) return;

    CParseCollectionDateFormats worker;
    CRef<CCmdComposite> cmd = worker.GetCommand(m_TopSeqEntry, true);
    if (cmd) {
        m_CmdProccessor->Execute(cmd);
    }
}

void CSequenceEditingEventHandler::ParseCollectionDateDayFirst( wxCommandEvent& event )
{
    x_SetUpTSEandUnDoManager(m_Workbench);       // perhaps this is unnecessary here after IsObjectSelected has run
    if (!m_TopSeqEntry || ! m_CmdProccessor) return;

    CParseCollectionDateFormats worker;
    CRef<CCmdComposite> cmd = worker.GetCommand(m_TopSeqEntry, false);
    if (cmd) {
        m_CmdProccessor->Execute(cmd);
    }
}

void CSequenceEditingEventHandler::CountryFixupCap( wxCommandEvent& event )
{
    x_SetUpTSEandUnDoManager(m_Workbench);       // perhaps this is unnecessary here after IsObjectSelected has run
    if (!m_TopSeqEntry || ! m_CmdProccessor) return;

    CCountryFixup worker;
    CRef<CCmdComposite> cmd = worker.GetCommand(m_TopSeqEntry, true);
    if (cmd) {
        m_CmdProccessor->Execute(cmd);
    }
}

void CSequenceEditingEventHandler::CountryFixupNoCap(wxCommandEvent& event)
{
    x_SetUpTSEandUnDoManager(m_Workbench);       // perhaps this is unnecessary here after IsObjectSelected has run
    if (!m_TopSeqEntry || !m_CmdProccessor) return;

    CCountryFixup worker;
    CRef<CCmdComposite> cmd = worker.GetCommand(m_TopSeqEntry, false);
    if (cmd) {
        m_CmdProccessor->Execute(cmd);
    }
}

void CSequenceEditingEventHandler::VectorTrim( wxCommandEvent& event )
{
    x_SetUpTSEandUnDoManager(m_Workbench);       // perhaps this is unnecessary here after IsObjectSelected has run
    if (!m_TopSeqEntry || !m_CmdProccessor) return;

    wxWindow* main_window = NULL;
    CWorkbench* wb_frame = dynamic_cast<CWorkbench*>(m_Workbench);
    if (wb_frame)
        main_window = wb_frame->GetMainWindow();

    CVectorTrimDlg* dlg = new CVectorTrimDlg(main_window, m_TopSeqEntry, m_CmdProccessor);
    dlg->Show();
}

void CSequenceEditingEventHandler::SelectTarget( wxCommandEvent& event )
{
    CSelectTargetDlg * dlg = new CSelectTargetDlg(NULL, m_Workbench);
    dlg->Show();
}

void CSequenceEditingEventHandler::AddCDS( wxCommandEvent& event )
{
    x_SetUpTSEandUnDoManager(m_Workbench);       // perhaps this is unnecessary here after IsObjectSelected has run
    if (!m_TopSeqEntry || ! m_CmdProccessor) return;

    CBulkFeatureAddDlgStd *dlg = new CBulkFeatureAddDlgStd(NULL, m_TopSeqEntry, 
                                                           m_Workbench);
    dlg->ChooseFeatureType(CSeqFeatData::eSubtype_cdregion);
    dlg->Show(true);
/*    if (dlg.ShowModal() == wxID_OK) 
    {
        CRef<CCmdComposite> cmd = dlg.GetCommand();
        if (cmd) { 
            m_CmdProccessor->Execute(cmd);                       
        } else {
            string errmsg = dlg.GetErrorMessage();
            if (!NStr::IsBlank(errmsg)) {
                wxString error = ToWxString(errmsg);
                wxMessageBox(error, wxT("Error"),
                         wxOK | wxICON_ERROR, NULL);
            }
        }
        dlg.Close();        
    }
*/
}


void CSequenceEditingEventHandler::AddRNA( wxCommandEvent& event )
{
    x_SetUpTSEandUnDoManager(m_Workbench);       // perhaps this is unnecessary here after IsObjectSelected has run
    if (!m_TopSeqEntry || ! m_CmdProccessor) return;

    CBulkFeatureAddDlgStd *dlg = new CBulkFeatureAddDlgStd(NULL, m_TopSeqEntry, 
                                                           m_Workbench);
    dlg->ChooseFeatureType(CSeqFeatData::eSubtype_rRNA);
    dlg->Show(true);
    /* if (dlg.ShowModal() == wxID_OK) 
    {
        CRef<CCmdComposite> cmd = dlg.GetCommand();
        if (cmd) { 
            m_CmdProccessor->Execute(cmd);                       
        } else {
            string errmsg = dlg.GetErrorMessage();
            if (!NStr::IsBlank(errmsg)) {
                wxString error = ToWxString(errmsg);
                wxMessageBox(error, wxT("Error"),
                         wxOK | wxICON_ERROR, NULL);
            }
        }
        dlg.Close();        
        }*/
}


void CSequenceEditingEventHandler::AddOtherFeature( wxCommandEvent& event )
{
    x_SetUpTSEandUnDoManager(m_Workbench);       // perhaps this is unnecessary here after IsObjectSelected has run
    if (!m_TopSeqEntry || ! m_CmdProccessor) return;

    CBulkFeatureAddDlgStd *dlg = new  CBulkFeatureAddDlgStd(wxTheApp->GetTopWindow(), m_TopSeqEntry, 
                                                            m_Workbench);
    dlg->ChooseFeatureType(CSeqFeatData::eSubtype_gene);
    dlg->Show(true);   
}


void CSequenceEditingEventHandler::Validate( wxCommandEvent& evt )
{
    CRef<CUser_object> params(new CUser_object());
    switch (evt.GetId())
    {
        case eCmdValidate:
        case eCmdValidate_ext:
            CTableDataValidateParams::SetDoAlignment(*params, true);
            CTableDataValidateParams::SetDoInference(*params, false);
            break;
        case eCmdValidateNoAlignments:
            CTableDataValidateParams::SetDoAlignment(*params, false);
            CTableDataValidateParams::SetDoInference(*params, false);
            break;
        case eCmdValidateCheckInferences:
            CTableDataValidateParams::SetDoAlignment(*params, true);
            CTableDataValidateParams::SetDoInference(*params, true);
            break;
        default:
            break;
    }

    wxWindow* main_window = NULL;
    CWorkbench* wb_frame = dynamic_cast<CWorkbench*>(m_Workbench);
    if (wb_frame)
        main_window = wb_frame->GetMainWindow();

    CValidateFrame::GetInstance(main_window, m_Workbench, params);
}


void CSequenceEditingEventHandler::SortUniqueCount( wxCommandEvent& evt )
{
    wxWindow* main_window = NULL;
    CWorkbench* wb_frame = dynamic_cast<CWorkbench*>(m_Workbench);
    if (wb_frame)
        main_window = wb_frame->GetMainWindow();
    IRefreshCntrl * suc_refresh = new CSUCRefreshCntrl(m_Workbench);
    CGenericReportDlg* report = new CGenericReportDlg(main_window, m_Workbench, wxID_ANY, _("SUC"));
    report->SetRefresh(suc_refresh);
    report->RefreshData();
    report->Show(true);
}

void CSequenceEditingEventHandler::SortUniqueCountTree(wxCommandEvent&)
{
    wxWindow* main_window = NULL;
    CWorkbench* wb_frame = dynamic_cast<CWorkbench*>(m_Workbench);
    if (wb_frame)
        main_window = wb_frame->GetMainWindow();

    CClickableSortUniqueCount* dlg = new CClickableSortUniqueCount(main_window, m_Workbench);
    dlg->Show(true);
}


void CSequenceEditingEventHandler::Discrepancy(wxCommandEvent&)
{
    x_SetUpTSEandUnDoManager(m_Workbench);       
    if (!m_TopSeqEntry || !m_CmdProccessor) return;

    CDiscrepancyDlg::OpenDiscrepancyDlg(CDiscrepancyDlg::eNormal, m_TopSeqEntry.GetScope(), m_Workbench, m_WorkDir);
}


void CSequenceEditingEventHandler::Oncaller(wxCommandEvent&)
{
    x_SetUpTSEandUnDoManager(m_Workbench);       
    if (!m_TopSeqEntry || !m_CmdProccessor) return;

    CDiscrepancyDlg::OpenDiscrepancyDlg(CDiscrepancyDlg::eOncaller, m_TopSeqEntry.GetScope(), m_Workbench, m_WorkDir);
}


void CSequenceEditingEventHandler::Submitter(wxCommandEvent&)
{
    x_SetUpTSEandUnDoManager(m_Workbench);       
    if (!m_TopSeqEntry || !m_CmdProccessor) return;

    CDiscrepancyDlg::OpenDiscrepancyDlg(CDiscrepancyDlg::eSubmitter, m_TopSeqEntry.GetScope(), m_Workbench, m_WorkDir);
}


void CSequenceEditingEventHandler::Mega(wxCommandEvent&)
{
    x_SetUpTSEandUnDoManager(m_Workbench);       
    if (!m_TopSeqEntry || !m_CmdProccessor) return;

    CDiscrepancyDlg::OpenDiscrepancyDlg(CDiscrepancyDlg::eMega, m_TopSeqEntry.GetScope(), m_Workbench, m_WorkDir);
}


void CSequenceEditingEventHandler::DiscrepancyList(wxCommandEvent&)
{
    CDiscrepancyList* dlg = new CDiscrepancyList(m_Workbench->GetMainWindow());
    dlg->Show(true);
}


void CSequenceEditingEventHandler::TestDialogView(wxCommandEvent& /*event*/)
{
    if (!m_TopSeqEntry) return;

    TConstScopedObjects objects;
    objects.push_back(SConstScopedObject(m_TopSeqEntry.GetCompleteObject(), &m_TopSeqEntry.GetScope()));
    CSimpleProjectView::CreateView("Test Dialog View", CTestDialogView::CreateDialog, m_Workbench, objects);
}

void CSequenceEditingEventHandler::TestFormView( wxCommandEvent& /*event*/ )
{
    if (!m_TopSeqEntry) return;

    TConstScopedObjects objects;
    objects.push_back(SConstScopedObject(m_TopSeqEntry.GetCompleteObject(), &m_TopSeqEntry.GetScope()));
    CSimpleProjectView::CreateView("Test Form View", CTextPanel::CreateForm, m_Workbench, objects);
}

void CSequenceEditingEventHandler::AddSequences( wxCommandEvent& event )
{
    x_SetUpTSEandUnDoManager(m_Workbench);       // perhaps this is unnecessary here after IsObjectSelected has run
    if (!m_TopSeqEntry || ! m_CmdProccessor) return;

    CAddSequences worker;
    worker.apply(m_TopSeqEntry, m_CmdProccessor);
}


void CSequenceEditingEventHandler::ApplyEditConvertRemove( wxCommandEvent& evt )
{
    CAECRFrame::EActionType aecr_action_type =  CAECRFrame::eActionType_Apply;
    CAECRFrame::EFieldType aecr_field_type =  CAECRFrame::eFieldType_Taxname;
    switch (evt.GetId()) {
        case eCmdApplySourceQual:
            aecr_action_type = CAECRFrame::eActionType_Apply;
            aecr_field_type = CAECRFrame::eFieldType_Source;
            break;
        case eCmdEditSourceQual:
            aecr_action_type = CAECRFrame::eActionType_Edit;
            aecr_field_type = CAECRFrame::eFieldType_Source;
            break;
        case eCmdConvertSourceQual:
            aecr_action_type = CAECRFrame::eActionType_Convert;
            aecr_field_type = CAECRFrame::eFieldType_Source;
            break;
        case eCmdSwapSourceQual:
            aecr_action_type = CAECRFrame::eActionType_Swap;
            aecr_field_type = CAECRFrame::eFieldType_Source;
            break;          
        case eCmdRemoveSourceQual:
            aecr_action_type = CAECRFrame::eActionType_Remove;
            aecr_field_type = CAECRFrame::eFieldType_Source;
            break;
        case eCmdApplyRNAQual:
            aecr_action_type = CAECRFrame::eActionType_Apply;
            aecr_field_type = CAECRFrame::eFieldType_RNA;
            break;
        case eCmdEditRNAQual:
            aecr_action_type = CAECRFrame::eActionType_Edit;
            aecr_field_type = CAECRFrame::eFieldType_RNA;
            break;
        case eCmdConvertRNAQual:
            aecr_action_type = CAECRFrame::eActionType_Convert;
            aecr_field_type = CAECRFrame::eFieldType_RNA;
            break;
        case eCmdSwapRNAQual:
            aecr_action_type = CAECRFrame::eActionType_Swap;
            aecr_field_type = CAECRFrame::eFieldType_RNA;
            break;          
        case eCmdRemoveRNAQual:
            aecr_action_type = CAECRFrame::eActionType_Remove;
            aecr_field_type = CAECRFrame::eFieldType_RNA;
            break;
        case eCmdApplyCGPQual:
            aecr_action_type = CAECRFrame::eActionType_Apply;
            aecr_field_type = CAECRFrame::eFieldType_CDSGeneProt;
            break;
        case eCmdEditCGPQual:
            aecr_action_type = CAECRFrame::eActionType_Edit;
            aecr_field_type = CAECRFrame::eFieldType_CDSGeneProt;
            break;
        case eCmdConvertCGPQual:
            aecr_action_type = CAECRFrame::eActionType_Convert;
            aecr_field_type = CAECRFrame::eFieldType_CDSGeneProt;
            break;
        case eCmdSwapCGPQual:
            aecr_action_type = CAECRFrame::eActionType_Swap;
            aecr_field_type = CAECRFrame::eFieldType_CDSGeneProt;
            break;          
        case eCmdRemoveCGPQual:
            aecr_action_type = CAECRFrame::eActionType_Remove;
            aecr_field_type = CAECRFrame::eFieldType_CDSGeneProt;
            break;
        case eCmdApplyFeatQual:
            aecr_action_type = CAECRFrame::eActionType_Apply;
            aecr_field_type = CAECRFrame::eFieldType_Feature;
            break;
        case eCmdEditFeatQual:
            aecr_action_type = CAECRFrame::eActionType_Edit;
            aecr_field_type = CAECRFrame::eFieldType_Feature;
            break;
        case eCmdConvertFeatQual:
            aecr_action_type = CAECRFrame::eActionType_Convert;
            aecr_field_type = CAECRFrame::eFieldType_Feature;
            break;
        case eCmdSwapFeatQual:
            aecr_action_type = CAECRFrame::eActionType_Swap;
            aecr_field_type = CAECRFrame::eFieldType_Feature;
            break;          
        case eCmdRemoveFeatQual:
            aecr_action_type = CAECRFrame::eActionType_Remove;
            aecr_field_type = CAECRFrame::eFieldType_Feature;
            break;
        case eCmdRemoveTextOutsideString:
            aecr_action_type = CAECRFrame::eActionType_RemoveTextOutside;
            aecr_field_type = CAECRFrame::eFieldType_Misc;
            break;
        default:
            return;
            break;
    }
    wxMenu* menu = dynamic_cast<wxMenu*>(evt.GetEventObject());
    wxString title;
    if (menu)
        title = menu->GetLabelText(evt.GetId());
    CAECRFrame * dlg = new CAECRFrame(NULL, m_Workbench); 
    dlg->SetInitParameters(aecr_action_type, aecr_field_type, kEmptyStr, title);
    CallAfter(&CSequenceEditingEventHandler::ShowAfter, dlg);
}

void CSequenceEditingEventHandler::ShowAfter(wxWindow* dlg)
{
    dlg->Show();
}

void CSequenceEditingEventHandler::EditFeature( wxCommandEvent& evt )
{
    CBulkEditFeatDlg * dlg = new CBulkEditFeatDlg(NULL, m_Workbench);
    switch (evt.GetId()) {
        case eCmdEditFeatLoc:
        case eCmdEditFeatLoc_ext:
            dlg->SetEditingType(CBulkEditFeatDlg::eLocation);
            break;
        case eCmdEditFeatEv:
            dlg->SetEditingType(CBulkEditFeatDlg::eEvidence);
            break;
        case eCmdEditFeatStrand:
        case eCmdEditFeatStrand_ext:
            dlg->SetEditingType(CBulkEditFeatDlg::eStrand);
            break;
        case eCmdEditFeatRevInt:
            dlg->SetEditingType(CBulkEditFeatDlg::eRevInt);
            break;
        case eCmdEditFeatPseudo:
            dlg->SetEditingType(CBulkEditFeatDlg::ePseudo);
            break;
        case eCmdEditFeatException:
            dlg->SetEditingType(CBulkEditFeatDlg::eException);
            break;
        case eCmdEditFeatExperiment:
            dlg->SetEditingType(CBulkEditFeatDlg::eExperiment);
            break;
        case eCmdEditFeatInference:
            dlg->SetEditingType(CBulkEditFeatDlg::eInference);
            break;
        default:
            break;
    }
    dlg->Show(true);
}


void CSequenceEditingEventHandler::ParseText( wxCommandEvent& evt )
{
    CParseTextDlg * dlg = new CParseTextDlg(NULL, m_Workbench); 
    if (evt.GetId() == eCmdParseTextFromDefline) {
        dlg->SetFromFieldType(CParseTextDlg::eFieldType_Misc); // CFieldNamePanel
        dlg->SetToFieldType(CParseTextDlg::eFieldType_Source);
    }

    dlg->Show(true);
}

void CSequenceEditingEventHandler::RemoveTextInsideStr( wxCommandEvent& evt )
{
    CRemoveTextInsideStrDlg * dlg = new CRemoveTextInsideStrDlg(NULL, m_Workbench);
    dlg->Show(true);
}

void CSequenceEditingEventHandler::ConvertFeatures( wxCommandEvent& evt )
{
    CConvertFeatDlg * dlg = new CConvertFeatDlg(NULL, m_Workbench);
    dlg->Show(true);
}

static const char* kFrameWidth = "Frame Width";
static const char* kFrameHeight = "Frame Height";
static const char* kFramePosX = "Frame Position X";
static const char* kFramePosY = "Frame Position Y";


void CSequenceEditingEventHandler::SaveSettings(const string &regpath, wxWindow *dlg) const
{
    if (regpath.empty() || !dlg)
        return;

    CGuiRegistry& gui_reg = CGuiRegistry::GetInstance();
    CRegistryWriteView view = gui_reg.GetWriteView(regpath);

    view.Set(kFrameWidth, dlg->GetScreenRect().GetWidth());
    view.Set(kFrameHeight, dlg->GetScreenRect().GetHeight());
    view.Set(kFramePosX, dlg->GetScreenPosition().x);
    view.Set(kFramePosY, dlg->GetScreenPosition().y);
}


void CSequenceEditingEventHandler::LoadSettings(const string &regpath, wxWindow *dlg)
{
    if (regpath.empty() || !dlg)
        return;

    CGuiRegistry& gui_reg = CGuiRegistry::GetInstance();
    CRegistryReadView view = gui_reg.GetReadView(regpath);

    int width = view.GetInt(kFrameWidth, -1);
    int height = view.GetInt(kFrameHeight, -1);
    if (width >= 0  && height >= 0)
        dlg->SetSize(wxSize(width,height));

    int pos_x = view.GetInt(kFramePosX, -1);
    int pos_y = view.GetInt(kFramePosY, -1);

   if (pos_x >= 0  && pos_y >= 0)
   {
       int max_x = 0;
       for (size_t i = 0; i < wxDisplay::GetCount(); i++) // also see gui/widgets/wx/wx_utils.cpp:CorrectWindowRect() for alternative window position validation
       {
        wxDisplay display(i);
        max_x += display.GetGeometry().GetWidth();
       }
       if (pos_x + width > max_x) pos_x = wxGetDisplaySize().GetWidth()-width-5;
       if (pos_y + height > wxGetDisplaySize().GetHeight()) pos_y = wxGetDisplaySize().GetHeight()-height-5;
       
       dlg->SetPosition(wxPoint(pos_x,pos_y));
   }
   wxTopLevelWindow* top_win = dynamic_cast<wxTopLevelWindow*>(dlg);
   if (top_win)
   {
       wxRect rc = dlg->GetRect();
       CorrectWindowRect(top_win, rc);
       dlg->SetSize(rc);
       dlg->Move(rc.GetLeftTop());
   }
}

void CSequenceEditingEventHandler::TableReader( wxCommandEvent& evt )
{
    x_SetUpTSEandUnDoManager(m_Workbench);       // perhaps this is unnecessary here after IsObjectSelected has run
    if (!m_TopSeqEntry || ! m_CmdProccessor) return;

    CQualTableLoadManager* qual_mgr = NULL;

    CRef<CFileLoadWizard> fileManager(new CFileLoadWizard());
    vector<string> format_ids;
    format_ids.push_back("file_loader_qual_table");
    fileManager->LoadFormats(format_ids);
    fileManager->SetWorkDir(m_WorkDir);

    for( size_t i = 0;  i < fileManager->GetFormatManagers().size();  ++i  )   {
        const IFileLoadPanelClient* mgr = fileManager->GetFormatManagers()[0].GetPointerOrNull();

        if ("file_loader_qual_table" == mgr->GetFileLoaderId()) {
            const CQualTableLoadManager* const_qual_mgr = dynamic_cast<const CQualTableLoadManager*>(mgr);
            if (const_qual_mgr) {
                qual_mgr = const_cast<CQualTableLoadManager*>(const_qual_mgr);
                qual_mgr->SetTopLevelEntry(m_TopSeqEntry);
                qual_mgr->SetServiceLocator(m_Workbench);
            }            
        }
    }  

    vector<CIRef<IOpenObjectsPanelClient> > loadManagers;
    loadManagers.push_back(CIRef<IOpenObjectsPanelClient>(fileManager.GetPointer()));

    COpenObjectsDlg dlg(NULL);
    dlg.SetSize(760, 940);
    dlg.Centre(wxBOTH | wxCENTRE_ON_SCREEN);
    dlg.SetMinSize(wxSize(760, 750));
    dlg.SetRegistryPath("Dialogs.Edit.OpenTables");
    dlg.SetManagers(loadManagers);
    LoadSettings("Dialogs.Edit.OpenTables", &dlg);

    if (dlg.ShowModal() == wxID_OK) {
        CIRef<IObjectLoader> object_loader(dlg.GetObjectLoader());
        qual_mgr->x_GetColumnIdPanel()->x_TableReaderCommon(object_loader, wxT("Reading file(s)..."));
    }
    SaveSettings("Dialogs.Edit.OpenTables", &dlg);
}

void CSequenceEditingEventHandler::TableReaderClipboard( wxCommandEvent& evt )
{
    x_SetUpTSEandUnDoManager(m_Workbench);       // perhaps this is unnecessary here after IsObjectSelected has run
    if (!m_TopSeqEntry || ! m_CmdProccessor) return;
    
    CQualTableLoadManager* qual_mgr = NULL;

    vector< CIRef<IFileLoadPanelClientFactory> > file_formats;
    GetExtensionAsInterface("file_load_panel_client", file_formats);
    CIRef<IFileLoadPanelClient> format_manager;

    for (size_t i = 0; i < file_formats.size(); ++i) {
        if (file_formats[i]->GetFileLoaderId() == "file_loader_qual_table") {
            format_manager.Reset(file_formats[i]->CreateInstance());
        
            if ("file_loader_qual_table" == format_manager->GetFileLoaderId()) {            
                const CQualTableLoadManager* const_qual_mgr = dynamic_cast<const CQualTableLoadManager*>(format_manager.GetPointer());
                if (const_qual_mgr) {
                    qual_mgr = const_cast<CQualTableLoadManager*>(const_qual_mgr);
                    qual_mgr->SetTopLevelEntry(m_TopSeqEntry);
                    qual_mgr->SetServiceLocator(m_Workbench);
                }            
            }
        }
    }


    string fname;
    if (wxTheClipboard->Open())
    {
        wxTextDataObject data;
        if (wxTheClipboard->IsSupported( wxDF_UNICODETEXT ))
            wxTheClipboard->GetData( data );
        if (data.GetText().length() == 0) {
            wxMessageBox("No data in clipboard for table");
            wxTheClipboard->Close();
            return;
        }

        CTmpFile f(CTmpFile::eNoRemove);
        f.AsOutputFile(CTmpFile::eIfExists_ReturnCurrent) << data.GetText();
        fname = f.GetFileName();
        wxTheClipboard->Close();
    }
            
    vector<wxString> fnames;
    fnames.push_back(ToWxString(fname));

    // This also causes windows to be created it seems, and so it must happen after 
    // create is called....
    qual_mgr->SetFilenames(fnames);

    COpenDataSourceObjectDlg dlg(NULL, CIRef<IUIToolManager>(qual_mgr));
    dlg.SetSize(760, 940);
    dlg.Centre(wxBOTH|wxCENTRE_ON_SCREEN);
    dlg.SetMinSize(wxSize(760, 750));
    dlg.SetRegistryPath("Dialogs.Edit.OpenTables");
    LoadSettings("Dialogs.Edit.OpenTables", &dlg);

    CFile tmp_file(fname);
    tmp_file.Remove();

    if (dlg.ShowModal() == wxID_OK) {
        CIRef<IObjectLoader> object_loader(dlg.GetObjectLoader());
        qual_mgr->x_GetColumnIdPanel()->x_TableReaderCommon(object_loader, wxT("Reading from clipboard(s)..."));
    }
    SaveSettings("Dialogs.Edit.OpenTables", &dlg);
}

void CSequenceEditingEventHandler::BulkEdit( wxCommandEvent& event )
{
    x_SetUpTSEandUnDoManager(m_Workbench);       // perhaps this is unnecessary here after IsObjectSelected has run
    if (!m_TopSeqEntry || ! m_CmdProccessor) return;

    try
    {
        SrcEditDialog dlg(NULL, m_TopSeqEntry, m_Workbench, m_SeqSubmit);
        dlg.SetWorkDir(m_WorkDir);
        if (dlg.ShowModal() == wxID_OK) {
            CRef<CCmdComposite> cmd = dlg.GetCommand();
            m_CmdProccessor->Execute(cmd);
            //dlg.Close();        
        }    
    } catch  (CException&) {}
}

void CSequenceEditingEventHandler::BulkCdsEdit( wxCommandEvent& event )
{
    x_SetUpTSEandUnDoManager(m_Workbench);       // perhaps this is unnecessary here after IsObjectSelected has run
    if (!m_TopSeqEntry || ! m_CmdProccessor) return;


    CSeq_entry_Handle seh = m_TopSeqEntry;
   
    try
    {
        CBulkCDS dlg(NULL, seh,m_Workbench, m_SeqSubmit);
    
        bool done = false;
        while (!done) {
            if (dlg.ShowModal() == wxID_OK) {
                CRef<CCmdComposite> cmd = dlg.GetCommand();
                if (cmd) {
                    m_CmdProccessor->Execute(cmd);                       
                    //dlg.Close();        
                    done = true;
                } else {
                    wxString error = ToWxString(dlg.GetErrorMessage());
                    wxMessageBox(error, wxT("Error"),
                                 wxOK | wxICON_ERROR, NULL);
                }
            } else {
                done = true;
            }
        }
    } catch  (CException&) {}
}

void CSequenceEditingEventHandler::BulkGeneEdit( wxCommandEvent& event )
{
    x_SetUpTSEandUnDoManager(m_Workbench);       // perhaps this is unnecessary here after IsObjectSelected has run
    if (!m_TopSeqEntry || ! m_CmdProccessor) return;

    CSeq_entry_Handle seh = m_TopSeqEntry;
     
    try
    {
        CBulkGene dlg(NULL, seh,m_Workbench, m_SeqSubmit);
        
        bool done = false;
        while (!done) {
            if (dlg.ShowModal() == wxID_OK) {
                CRef<CCmdComposite> cmd = dlg.GetCommand();
                if (cmd) {
                    m_CmdProccessor->Execute(cmd);                       
                    //dlg.Close();        
                    done = true;
                } else {
                    wxString error = ToWxString(dlg.GetErrorMessage());
                    wxMessageBox(error, wxT("Error"),
                                 wxOK | wxICON_ERROR, NULL);
                }
            } else {
                done = true;
            }
        }
    } catch  (CException&) {}

}

void CSequenceEditingEventHandler::BulkRnaEdit( wxCommandEvent& event )
{
    x_SetUpTSEandUnDoManager(m_Workbench);       // perhaps this is unnecessary here after IsObjectSelected has run
    if (!m_TopSeqEntry || ! m_CmdProccessor) return;

    CSeq_entry_Handle seh = m_TopSeqEntry;
    
    try
    {
        CBulkRna dlg(NULL, seh,m_Workbench, m_SeqSubmit);
        
        bool done = false;
        while (!done) {
            if (dlg.ShowModal() == wxID_OK) {
                CRef<CCmdComposite> cmd = dlg.GetCommand();
                if (cmd) {
                    m_CmdProccessor->Execute(cmd);                       
                    //dlg.Close();        
                    done = true;
                } else {
                    wxString error = ToWxString(dlg.GetErrorMessage());
                    wxMessageBox(error, wxT("Error"),
                                 wxOK | wxICON_ERROR, NULL);
                }
            } else {
                done = true;
            }
        }
    } catch  (CException&) {}

}

void CSequenceEditingEventHandler::MolInfoEdit(wxCommandEvent& evt)
{
    x_SetUpTSEandUnDoManager(m_Workbench);       // perhaps this is unnecessary here after IsObjectSelected has run
    if (!m_TopSeqEntry || ! m_CmdProccessor) return;
    CMolInfoEditDlg dlg(NULL, m_TopSeqEntry);
    if (dlg.ShowModal() == wxID_OK) 
    {
        CRef<CCmdComposite> cmd = dlg.GetCommand();
        if (cmd)
        {
            m_CmdProccessor->Execute(cmd);                       
        }
        dlg.Close();        
    }
}


void CSequenceEditingEventHandler::CorrectGenes( wxCommandEvent& event )
{
    CCorrectGenesDialog * dlg = new CCorrectGenesDialog(NULL, m_Workbench);
    dlg->Show(true);
}


static bool CompLoc(CRef<CSeq_loc> first, CRef<CSeq_loc> second)
{
    return first->Compare(*second);

}


static bool EqualLoc(CRef<CSeq_loc> first, CRef<CSeq_loc> second)
{
    return first->Equals(*second);
}

void CSequenceEditingEventHandler::Autodef(wxCommandEvent& evt)
{
    x_SetUpTSEandUnDoManager(m_Workbench);       // perhaps this is unnecessary here after IsObjectSelected has run
    if (!m_TopSeqEntry || ! m_CmdProccessor) return;

    int event_id = evt.GetId();

    switch (event_id) {
        case eCmdAutodefDefaultOptions:
            {
                SAutodefParams params;
                ConfigureAutodefParamsForBestCombo(params, m_TopSeqEntry);
                CRef<CCmdComposite> cmd = AutodefSeqEntry(m_TopSeqEntry, &params);
                m_CmdProccessor->Execute(cmd);
            }
            break;
        case eCmdAutodefOptions:
            {                
                SAutodefParams params;
                ConfigureAutodefParamsForBestCombo(params, m_TopSeqEntry);
                CAutodefDlg dlg(NULL);
                dlg.SetParams(&params);
                if (dlg.ShowModal() == wxID_OK) 
                {                    
                    dlg.PopulateParams(params);
                    CRef<CCmdComposite> cmd(NULL);
                    if (dlg.OnlyTargeted()) {
                        vector<CRef<CSeq_loc> > &orig_locs = x_GetSelectedLocations();
                        vector<CRef<CSeq_loc> > cpy = orig_locs;
                        vector<CRef<CSeq_loc> >::iterator lit = cpy.begin();
                        while (lit != cpy.end()) {
                            CBioseq_Handle b = m_TopSeqEntry.GetScope().GetBioseqHandle(**lit);
                            if (!b || b.IsAa()) {
                                lit = cpy.erase(lit);
                            } else {
                                ++lit;
                            }
                        }

                        if (cpy.empty()) {
                            dlg.Close();
                            return;
                        }
                        stable_sort(cpy.begin(), cpy.end(), CompLoc);
                        cpy.erase( unique( cpy.begin(), cpy.end(), EqualLoc), cpy.end() );

                        cmd.Reset(new CCmdComposite("Autodef"));
                        ITERATE(vector<CRef<CSeq_loc> >, b, cpy) {
                            CBioseq_Handle bsh = m_TopSeqEntry.GetScope().GetBioseqHandle(**b);
                            CRef<CCmdComposite> subcmd = AutodefSeqEntry(m_TopSeqEntry, bsh, &params);
                            if (subcmd) {
                                cmd->AddCommand(*subcmd);
                            }
                        }                        
                    } else {
                        cmd = AutodefSeqEntry(m_TopSeqEntry, &params);
                    }
                    m_CmdProccessor->Execute(cmd);                       
                    dlg.Close();        
                }

            }
            break;
        case eCmdAutodefMisc:
            {
                SAutodefParams params;
                ConfigureAutodefParamsForBestCombo(params, m_TopSeqEntry);
                params.m_MiscFeatRule = 1;
                CRef<CCmdComposite> cmd = AutodefSeqEntry(m_TopSeqEntry, &params);
                m_CmdProccessor->Execute(cmd);
            }
            break;
        case eCmdAutodefId:
            {
                SAutodefParams params;
                ConfigureAutodefParamsForID(params, m_TopSeqEntry);
                CRef<CCmdComposite> cmd = AutodefSeqEntry(m_TopSeqEntry, &params);
                m_CmdProccessor->Execute(cmd);
            }
            break;
        case eCmdAutodefNoMods:
            {
                SAutodefParams params;
                CRef<CCmdComposite> cmd = AutodefSeqEntry(m_TopSeqEntry, &params);
                m_CmdProccessor->Execute(cmd);
            }
            break;
        case eCmdAutodefPopset:
            {
                SAutodefParams params;
                ConfigureAutodefParamsForBestCombo(params, m_TopSeqEntry);
                CAutodefDlg dlg(NULL);
                dlg.SetParams(&params);
                dlg.ShowSourceOptions(false);
                if (dlg.ShowModal() == wxID_OK) 
                {                    
                    dlg.PopulateParams(params);
                    CRef<CCmdComposite> cmd = AutodefSeqEntry(m_TopSeqEntry, &params, true);
                    m_CmdProccessor->Execute(cmd);                       
                    dlg.Close();        
                }
            }
            break;
        case eCmdAutodefRefresh:
            {
                CRef<CCmdComposite> cmd = RefreshDeflineCommand(m_TopSeqEntry);
                if (cmd) m_CmdProccessor->Execute(cmd);
            }
            break;
    }
}


void CSequenceEditingEventHandler::PT_Cleanup (wxCommandEvent& event)
{
    x_SetUpTSEandUnDoManager(m_Workbench);       // perhaps this is unnecessary here after IsObjectSelected has run
    if (!m_TopSeqEntry || ! m_CmdProccessor) return;
    CRef<CCmdComposite> cmd = InstantiateProteinTitles(m_TopSeqEntry);
    if (cmd) {
        m_CmdProccessor->Execute(cmd);
    }
}


void CSequenceEditingEventHandler::ParseStrainSerotypeFromNames(wxCommandEvent& event)
{
    x_SetUpTSEandUnDoManager(m_Workbench);       // perhaps this is unnecessary here after IsObjectSelected has run
    if (!m_TopSeqEntry || !m_CmdProccessor) return;

    CStrainSerotypeInfluenza worker(CStrainSerotypeInfluenza::eCommandInfluenza_parse);
    CRef<CCmdComposite> composite = worker.GetCommand(m_TopSeqEntry, "Parse Strain Serotype from Names");
    if (composite) {
        m_CmdProccessor->Execute(composite);
    }
}
 
void CSequenceEditingEventHandler::AddStrainSerotypeToNames (wxCommandEvent& event)
{
    x_SetUpTSEandUnDoManager(m_Workbench);
    if (!m_TopSeqEntry || ! m_CmdProccessor) return;
    
    CStrainSerotypeInfluenza worker(CStrainSerotypeInfluenza::eCommandInfluenza_add);
    CRef<CCmdComposite> composite = worker.GetCommand(m_TopSeqEntry, "Add Strain Serotype to Names");
    if (composite) {
        m_CmdProccessor->Execute(composite);
    }
}


void CSequenceEditingEventHandler::FixupOrganismNames (wxCommandEvent& event)
{
    x_SetUpTSEandUnDoManager(m_Workbench);
    if (!m_TopSeqEntry || ! m_CmdProccessor) return;
    
    CStrainSerotypeInfluenza worker(CStrainSerotypeInfluenza::eCommandInfluenza_fixup);
    CRef<CCmdComposite> composite = worker.GetCommand(m_TopSeqEntry, "Fixup Organism Names");
    if (composite) {
        m_CmdProccessor->Execute(composite);
    }
}


void CSequenceEditingEventHandler::SplitQualifiersAtCommas (wxCommandEvent& event)
{
    x_SetUpTSEandUnDoManager(m_Workbench);       
    if (!m_TopSeqEntry || ! m_CmdProccessor) return;

    CStrainSerotypeInfluenza worker(CStrainSerotypeInfluenza::eCommandSplitQualsAtCommas);
    CRef<CCmdComposite> composite = worker.GetCommand(m_TopSeqEntry, "Split Quals at Commas");
    if (composite) {
        m_CmdProccessor->Execute(composite);
    }
}

void CSequenceEditingEventHandler::SplitStructuredCollections (wxCommandEvent& event)
{
    x_SetUpTSEandUnDoManager(m_Workbench);       
    if (!m_TopSeqEntry || ! m_CmdProccessor) return;

    CStrainSerotypeInfluenza worker(CStrainSerotypeInfluenza::eCommandSplitStructuredCollections);
    CRef<CCmdComposite> composite = worker.GetCommand(m_TopSeqEntry, "Split Structured Collections at Semicolon");
    if (composite) {
        m_CmdProccessor->Execute(composite);
    }
}


void CSequenceEditingEventHandler::TrimOrganismNames (wxCommandEvent& event)
{
    x_SetUpTSEandUnDoManager(m_Workbench);       
    if (!m_TopSeqEntry || ! m_CmdProccessor) return;

    CStrainSerotypeInfluenza worker(CStrainSerotypeInfluenza::eCommandTrimOrganismNames);
    CRef<CCmdComposite> composite = worker.GetCommand(m_TopSeqEntry, "Trim Organism Names");
    if (composite) {
        m_CmdProccessor->Execute(composite);
    }
}

void CSequenceEditingEventHandler::AddNamedrRNA (wxCommandEvent& event)
{
    x_SetUpTSEandUnDoManager(m_Workbench);       
    if (!m_TopSeqEntry || ! m_CmdProccessor) return;

    string rna_name;
    switch (event.GetId()){
        case eCmdAddNamedrRNA12S:
            rna_name = "12S ribosomal RNA";
            break;
        case eCmdAddNamedrRNA16S:
            rna_name = "16S ribosomal RNA";
            break;
        case eCmdAddNamedrRNA23S:
            rna_name = "23S ribosomal RNA";
            break;
        case eCmdAddNamedrRNA18S:
            rna_name = "18S ribosomal RNA";
            break;
        case eCmdAddNamedrRNA28S:
            rna_name = "28S ribosomal RNA";
            break;
        case eCmdAddNamedrRNA26S:
            rna_name = "26S ribosomal RNA";
            break;                    
        case eCmdAddNamedrRNASmall:
            rna_name = "small subunit ribosomal RNA";
            break;
        case eCmdAddNamedrRNALarge:
            rna_name = "large subunit ribosomal RNA";
            break;     
    }
    bool ispartial5 = true;
    bool ispartial3 = true;
    CRef<CCmdComposite>  composite = AddNamedrRna(m_TopSeqEntry, rna_name, ispartial5, ispartial3);
    if (composite){
        m_CmdProccessor->Execute(composite);
    }
}

void CSequenceEditingEventHandler::AddControlRegion ( wxCommandEvent& event)
{
    x_SetUpTSEandUnDoManager(m_Workbench);       
    if (!m_TopSeqEntry || ! m_CmdProccessor) return;
    
    CRef<CCmdComposite>  composite = CreateControlRegion(m_TopSeqEntry);
    if (composite){
        m_CmdProccessor->Execute(composite);
    }
}

void CSequenceEditingEventHandler::AddMicrosatellite ( wxCommandEvent& event)
{
    x_SetUpTSEandUnDoManager(m_Workbench);       
    if (!m_TopSeqEntry || ! m_CmdProccessor) return;
    
    CRef<CCmdComposite>  composite = CreateMicrosatellite(m_TopSeqEntry);
    if (composite){
        m_CmdProccessor->Execute(composite);
    }
}


bool CSequenceEditingEventHandler::x_IsLocationOkForFeatCmd(CSeqFeatData::ESubtype subtype, CRef<CSeq_loc> loc)
{
    bool rval = false;
    CSeqFeatData::EFeatureLocationAllowed allowed_type = CSeqFeatData::AllowedFeatureLocation(subtype);
    
    if ( (m_IsSeq && m_IsAa && allowed_type == CSeqFeatData::eFeatureLocationAllowed_ProtOnly) ||
         (m_IsSeq && !m_IsAa && allowed_type == CSeqFeatData::eFeatureLocationAllowed_NucOnly) ||
         (m_IsSeq && allowed_type == CSeqFeatData::eFeatureLocationAllowed_Any) )
        rval = true;

    return rval;
}

CRef<CSeq_loc> CSequenceEditingEventHandler::GetFeatureLocation(CSeqFeatData::ESubtype subtype)
{
    CRef<CSeq_loc> loc;

    vector<CRef<CSeq_loc> > &orig_locs = x_GetSelectedLocations();
    if (orig_locs.empty()) 
    {
        if (!RunningInsideNCBI())
            wxMessageBox(wxT("No data loaded"), wxT("Error"), wxOK | wxICON_ERROR, NULL);
        return loc;
    }

    if (!m_TopSeqEntry && !m_CachedSelectedLocations.empty()) {
        CBioseq_Handle bsh = m_SelObjects.front().scope->GetBioseqHandle(*(m_CachedSelectedLocations.front()->GetId()));
        m_TopSeqEntry = bsh.GetTopLevelEntry();
    }
    if (!m_TopSeqEntry)
    {
        if (!RunningInsideNCBI())
            wxMessageBox(wxT("No data loaded"), wxT("Error"), wxOK | wxICON_ERROR, NULL);
        return loc;
    }

    if (m_IsSeq)
    {   
        ITERATE(vector<CRef<CSeq_loc> >, lit, orig_locs) 
        {
            if (x_IsLocationOkForFeatCmd(subtype, *lit)) 
            {
                loc = *lit;
                break;
            }
        }
    }

    if (!loc)
    {
        if (!RunningInsideNCBI())
        {
            CSeqFeatData::EFeatureLocationAllowed allowed_type = CSeqFeatData::AllowedFeatureLocation(subtype);
            if (allowed_type == CSeqFeatData::eFeatureLocationAllowed_ProtOnly)
            {
                wxMessageBox(wxT("Please select a protein bioseq to create a ") + wxString(CSeqFeatData::SubtypeValueToName(subtype)), wxT("Error"), wxOK | wxICON_ERROR, NULL);
            }
            else if (allowed_type == CSeqFeatData::eFeatureLocationAllowed_NucOnly)
            {
                wxMessageBox(wxT("Please select a nucleotide bioseq to create a ") + wxString(CSeqFeatData::SubtypeValueToName(subtype)), wxT("Error"), wxOK | wxICON_ERROR, NULL);
            }
            else 
            {
                wxMessageBox(wxT("Please select a bioseq to create a ") + wxString(CSeqFeatData::SubtypeValueToName(subtype)), wxT("Error"), wxOK | wxICON_ERROR, NULL);
            }
            CSelectTargetDlg * dlg = new CSelectTargetDlg(NULL, m_Workbench);
            dlg->Show();
        }
    }
    return loc;
}

void CSequenceEditingEventHandler::CreateFeature ( wxCommandEvent& evt)
{
    CSeqFeatData::ESubtype subtype = CBioseqEditor::GetFeatTypeFromCmdID(evt.GetId());
    CRef<CSeq_loc> loc = GetFeatureLocation(subtype);
    if (!loc)
        return;

    CRef<CSeq_feat> feat = CBioseqEditor::MakeDefaultFeature(subtype);
    feat->SetLocation().Assign(*loc);
    
    CBioseq_Handle bsh = m_TopSeqEntry.GetScope().GetBioseqHandle(*loc);
    CSeq_entry_Handle seh = bsh.GetSeq_entry_Handle();


    CIRef<IEditObject> editor(new CEditObjectSeq_feat(*feat, seh, m_TopSeqEntry.GetScope(), true));
    CEditObjViewDlg* edit_dlg = new CEditObjViewDlg(NULL, true);
    edit_dlg->SetUndoManager(m_CmdProccessor);
    wxWindow* editorWindow = editor->CreateWindow(edit_dlg);
    edit_dlg->SetEditorWindow(editorWindow);
    edit_dlg->SetEditor(editor);
    edit_dlg->Show(true);
}

void CSequenceEditingEventHandler::CreateRegulatory ( wxCommandEvent& evt)
{
    CSeqFeatData::ESubtype subtype =  CSeqFeatData::eSubtype_regulatory;
    CRef<CSeq_loc> loc = GetFeatureLocation(subtype);
    if (!loc)
        return;
   
    CRef<CSeq_feat> feat = CBioseqEditor::MakeDefaultFeature(subtype);
    feat->SetLocation().Assign(*loc);
        
    string regulatory_class;
    switch(evt.GetId())
    {
    case eCmdCreateRegulatory_promoter : regulatory_class = "promoter"; break;
    case eCmdCreateRegulatory_enhancer : regulatory_class = "enhancer"; break;
    case eCmdCreateRegulatory_ribosome_binding_site : regulatory_class = "ribosome_binding_site"; break;
    case eCmdCreateRegulatory_riboswitch : regulatory_class = "riboswitch"; break;
    case eCmdCreateRegulatory_terminator : regulatory_class = "terminator"; break;
    default : break;
    }
    if (!regulatory_class.empty())
    {
        CRef<CGb_qual> q(new CGb_qual());
        q->SetQual("regulatory_class");
        q->SetVal(regulatory_class);
        feat->SetQual().push_back(q);
    }


    CBioseq_Handle bsh = m_TopSeqEntry.GetScope().GetBioseqHandle(*loc);
    CSeq_entry_Handle seh = bsh.GetSeq_entry_Handle();
    

    CIRef<IEditObject> editor(new CEditObjectSeq_feat(*feat, seh, m_TopSeqEntry.GetScope(), true));
    CEditObjViewDlg* edit_dlg = new CEditObjViewDlg(NULL, true);
    edit_dlg->SetUndoManager(m_CmdProccessor);
    wxWindow* editorWindow = editor->CreateWindow(edit_dlg);
    edit_dlg->SetEditorWindow(editorWindow);
    edit_dlg->SetEditor(editor);
    edit_dlg->Show(true);
}


void CSequenceEditingEventHandler::CreateDescriptor ( wxCommandEvent& evt)
{
    x_SetUpTSEandUnDoManager(m_Workbench);       
    if (!m_TopSeqEntry || ! m_CmdProccessor) return;

    CRef<CSeqdesc> seqdesc(new CSeqdesc());
    switch (evt.GetId()) {
        case eCmdCreateDescTPAAssembly:
            seqdesc->SetUser().SetType().SetStr("TpaAssembly");
            break;
        case eCmdCreateDescStructuredComment:
            seqdesc->SetUser().SetObjectType(CUser_object::eObjectType_StructuredComment);
            break;
        case eCmdCreateDescRefGeneTracking:
            seqdesc->SetUser().SetType().SetStr("RefGeneTracking");
            break;
        case eCmdCreateDescDBLink:
            seqdesc->SetUser().SetObjectType(CUser_object::eObjectType_DBLink);
            break;
        case eCmdCreateDescUnverified:
            seqdesc->SetUser().SetObjectType(CUser_object::eObjectType_Unverified);
            break;
        case eCmdCreateDescAuthorizedAccess:
            seqdesc->SetUser().SetType().SetStr("AuthorizedAccess");
            break;
        case eCmdCreateDescGenomeAssemblyComment:
            seqdesc->SetUser().Assign(*edit::CGenomeAssemblyComment::MakeEmptyUserObject());
            break;
        case eCmdCreateDescGenBankBlock:
            seqdesc->SetGenbank();
            break;
        case eCmdCreateDescComment:
        case eCmdCreateDescComment_ext:
            seqdesc->SetComment();
            break;
        case eCmdCreateDescTitle:
            seqdesc->SetTitle();
            break;
        case eCmdCreateDescMolinfo:
            seqdesc->SetMolinfo();
            break;
        case eCmdCreateDescSource:
            {{
            CRef<CBioSource> bioSource(new CBioSource());
            seqdesc->SetSource(*bioSource);
            }}
            break;
        case eCmdCreateDescPub:
        case eCmdCreateDescPub_ext:
            {{
                CRef<CPubdesc> pubdesc(new CPubdesc());
                seqdesc->SetPub(*pubdesc);
            }}
            break;
        default:
            break;            
    }
   
    CSeq_entry_Handle seh = m_TopSeqEntry;
    vector<CRef<CSeq_loc> > &orig_locs = x_GetSelectedLocations();
    if (!orig_locs.empty()) 
    {
        CBioseq_Handle bsh = m_TopSeqEntry.GetScope().GetBioseqHandle(*orig_locs.front());
        seh = bsh.GetSeq_entry_Handle();
    }

    CIRef<IEditObject> editor(new CEditObjectSeq_desc(*seqdesc, seh, m_TopSeqEntry.GetScope(), true));
    CEditObjViewDlg* edit_dlg = new CEditObjViewDlg(NULL, true);
    edit_dlg->SetUndoManager(m_CmdProccessor);
    wxWindow* editorWindow = editor->CreateWindow(edit_dlg);
    edit_dlg->SetEditorWindow(editorWindow);
    edit_dlg->SetEditor(editor);
    edit_dlg->SetWorkDir(m_WorkDir);
    edit_dlg->Show(true);
}


void CSequenceEditingEventHandler::RemoveUnverified( wxCommandEvent& event )
{
    x_SetUpTSEandUnDoManager(m_Workbench);       // perhaps this is unnecessary here after IsObjectSelected has run
    if (!m_TopSeqEntry || ! m_CmdProccessor) return;
        CRemoveUnverified worker;
    if (!worker.apply(m_TopSeqEntry,m_CmdProccessor,"Remove Univerified"))
        event.Skip();
}

void CSequenceEditingEventHandler::AddDefLine ( wxCommandEvent& event)
{
    x_SetUpTSEandUnDoManager(m_Workbench);       
    if (!m_TopSeqEntry || ! m_CmdProccessor) return;
    
    CAddDefLineDlg *dlg = new CAddDefLineDlg(NULL, m_Workbench);
    dlg->Show(true);
    
}

void CSequenceEditingEventHandler::PrefixDefLines ( wxCommandEvent& event)
{
    x_SetUpTSEandUnDoManager(m_Workbench);       
    if (!m_TopSeqEntry || ! m_CmdProccessor) return;
    
    bool show_label = true; 
    switch (event.GetId()){
        case eCmdPrefixOrgToDefLine:{
            CRef<CCmdComposite> composite(new CCmdComposite("Prefix Organism Name to Definition Line"));
            if (AddOrgToDefline(m_TopSeqEntry, composite))
                m_CmdProccessor->Execute(composite);
            break;
        }    
        case eCmdPrefixStrainToDefLine:{
            CRef<CCmdComposite> composite(new CCmdComposite("Prefix Strain to Definition Line"));
            CAutoDefAvailableModifier modifier(COrgMod::eSubtype_strain, true);
            if (AddModToDefline(m_TopSeqEntry, composite, modifier, show_label))
                m_CmdProccessor->Execute(composite);
            break;    
        }
        case eCmdPrefixCloneToDefLine:{
            CRef<CCmdComposite> composite(new CCmdComposite("Prefix Clone to Definition Line"));
            CAutoDefAvailableModifier modifier(CSubSource::eSubtype_clone, false);
            if (AddModToDefline(m_TopSeqEntry, composite, modifier, show_label))
                m_CmdProccessor->Execute(composite);
            break;    
        }
        case eCmdPrefixIsolateToDefLine:{
            CRef<CCmdComposite> composite(new CCmdComposite("Prefix Isolate to Definition Line"));
            CAutoDefAvailableModifier modifier(COrgMod::eSubtype_isolate, true);
            if (AddModToDefline(m_TopSeqEntry, composite, modifier, show_label))
                m_CmdProccessor->Execute(composite);
            break;    
        }
        case eCmdPrefixHaplotToDefLine:{
            CRef<CCmdComposite> composite(new CCmdComposite("Prefix Haplotype to Definition Line"));
            CAutoDefAvailableModifier modifier(CSubSource::eSubtype_haplotype, false);
            if (AddModToDefline(m_TopSeqEntry, composite, modifier, show_label))
                m_CmdProccessor->Execute(composite);
            break;    
        }
        case eCmdPrefixCultivarToDefLine:{
            CRef<CCmdComposite> composite(new CCmdComposite("Prefix Cultivar to Definition Line"));
            CAutoDefAvailableModifier modifier(COrgMod::eSubtype_cultivar, true);
            if (AddModToDefline(m_TopSeqEntry, composite, modifier, show_label))
                m_CmdProccessor->Execute(composite);
            break;    
        } 
        case eCmdPrefixListToDefLine:{
            SAutodefParams params;
            unsigned int found = ConfigureParamsForPrefixCombo(params, m_TopSeqEntry);
            CPrefixDeflinedlg dlg(NULL, m_TopSeqEntry, found);
            if (found > 0){
                dlg.SetParams(&params);
                dlg.PopulateModifierCombo();
            }    
            if (dlg.ShowModal() == wxID_OK){
                CRef<CCmdComposite> composite = dlg.GetCommand();
                if (composite){
                    m_CmdProccessor->Execute(composite);   
                }    
                dlg.Close();    
            }    
            break;
        }    
    }    
}

void CSequenceEditingEventHandler::MakeBadSpecificHostTable(wxCommandEvent& event)
{
    x_SetUpTSEandUnDoManager(m_Workbench);       
    if (!m_TopSeqEntry || ! m_CmdProccessor) return;

    CMakeBadSpecificHostTable collector;
    collector.MakeTable(m_TopSeqEntry);
}


void CSequenceEditingEventHandler::CorrectRNAStrand(wxCommandEvent& event)
{
    x_SetUpTSEandUnDoManager(m_Workbench);       
    if (!m_TopSeqEntry || ! m_CmdProccessor) return;

    CCorrectRNAStrandDlg* dlg = new CCorrectRNAStrandDlg(NULL, m_Workbench);
    dlg->Show(true); 
}

void CSequenceEditingEventHandler::ApplyRNA_ITS ( wxCommandEvent& event)
{
    x_SetUpTSEandUnDoManager(m_Workbench);       
    if (!m_TopSeqEntry || ! m_CmdProccessor) return;  

    CApplyRNAITSDlg *dlg = new CApplyRNAITSDlg(NULL, m_Workbench); 
    dlg->Show(true);
}

void CSequenceEditingEventHandler::EditingButtons ( wxCommandEvent& event )
{
    wxWindow* main_window = NULL;
    CWorkbench* wb_frame = dynamic_cast<CWorkbench*>(m_Workbench);
    if (wb_frame)
        main_window = wb_frame->GetMainWindow();
    CEditingBtnsPanel::GetInstance(main_window, m_Workbench);
}

void CSequenceEditingEventHandler::TrimNsRich( wxCommandEvent& event )
{
    x_SetUpTSEandUnDoManager(m_Workbench);       
    if (!m_TopSeqEntry || ! m_CmdProccessor) return;

    wxWindow* main_window = NULL;
    CWorkbench* wb_frame = dynamic_cast<CWorkbench*>(m_Workbench);
    if (wb_frame)
        main_window = wb_frame->GetMainWindow();

    CTrimN worker;
    worker.apply(m_TopSeqEntry,m_CmdProccessor,"Trim Ns Rich", main_window, m_WorkDir);
}

void CSequenceEditingEventHandler::TrimNsTerminal( wxCommandEvent& event )
{
    x_SetUpTSEandUnDoManager(m_Workbench);       
    if (!m_TopSeqEntry || ! m_CmdProccessor) return;

    wxWindow* main_window = NULL;
    CWorkbench* wb_frame = dynamic_cast<CWorkbench*>(m_Workbench);
    if (wb_frame)
        main_window = wb_frame->GetMainWindow();

    CSequenceAmbigTrimmer::TTrimRuleVec trim_rules;
    trim_rules.push_back(CSequenceAmbigTrimmer::STrimRule{ 1, 0 });

    CTrimN worker;
    worker.apply(m_TopSeqEntry,m_CmdProccessor,"Trim Ns Terminal", main_window, m_WorkDir, trim_rules);
}

void CSequenceEditingEventHandler::LabelRna( wxCommandEvent& event )
{
    x_SetUpTSEandUnDoManager(m_Workbench);       
    if (!m_TopSeqEntry || ! m_CmdProccessor) return;

    wxWindow* main_window = NULL;
    CWorkbench* wb_frame = dynamic_cast<CWorkbench*>(m_Workbench);
    if (wb_frame)
        main_window = wb_frame->GetMainWindow();

    CLabelRna worker;
    if (!worker.apply(m_TopSeqEntry,m_CmdProccessor,"Label RNA", main_window))
        event.Skip();
}

void CSequenceEditingEventHandler::RemProtTitles( wxCommandEvent& event )
{
    x_SetUpTSEandUnDoManager(m_Workbench);       
    if (!m_TopSeqEntry || ! m_CmdProccessor) return;

    CRemProtTitles worker;
    if (!worker.apply(m_TopSeqEntry,m_CmdProccessor,"Remove Protein Titles"))
        event.Skip();
}

void CSequenceEditingEventHandler::RemAllProtTitles( wxCommandEvent& event )
{
    x_SetUpTSEandUnDoManager(m_Workbench);       
    if (!m_TopSeqEntry || ! m_CmdProccessor) return;

    CRemAllProtTitles worker;
    if (!worker.apply(m_TopSeqEntry,m_CmdProccessor))
        event.Skip();
}

void CSequenceEditingEventHandler::SegregateSets( wxCommandEvent& event )
{
    wxWindow* main_window = NULL;
    CWorkbench* wb_frame = dynamic_cast<CWorkbench*>(m_Workbench);
    if (wb_frame)
        main_window = wb_frame->GetMainWindow();
    CSegregateSets* dlg = new CSegregateSets(main_window, m_TopSeqEntry, m_CmdProccessor);
    dlg->Show(true);
}

void CSequenceEditingEventHandler::SequesterSets( wxCommandEvent& event )
{
    wxWindow* main_window = NULL;
    CWorkbench* wb_frame = dynamic_cast<CWorkbench*>(m_Workbench);
    if (wb_frame)
        main_window = wb_frame->GetMainWindow();
    CSequesterSets * dlg = new CSequesterSets(main_window, m_TopSeqEntry, m_SeqSubmit, m_Workbench);
    dlg->Show(true);      
}

void CSequenceEditingEventHandler::WithdrawSequences( wxCommandEvent& event )
{
    CWithdrawSequences dlg(NULL, m_TopSeqEntry);
    if (dlg.ShowModal() == wxID_OK) 
    {
        CRef<CCmdComposite> cmd = dlg.GetCommand();
        if (cmd)  
        {
        vector<string> names = dlg.GetNames();
            m_CmdProccessor->Execute(cmd);  
        if (!names.empty())
        {
        CRef<CCmdComposite> rm_from_align_cmd( new CCmdComposite("Remove from alignments") );
        CRemoveSeqFromAlignDlg::ApplyToCSeq_entry(m_TopSeqEntry, rm_from_align_cmd, names);	
        m_CmdProccessor->Execute(rm_from_align_cmd);  
        }
            event.SetId(eCmdValidate);
            Validate(event);
        }
    }
}

void CSequenceEditingEventHandler::AddSet( wxCommandEvent& event )
{
    x_SetUpTSEandUnDoManager(m_Workbench);       
    if (!m_TopSeqEntry || ! m_CmdProccessor) return;
    CChooseSetClassDlg dlg(NULL);
    if (dlg.ShowModal() == wxID_OK) 
    {
        CBioseq_set::EClass new_class = dlg.GetClass();
        CRef<CCmdInsertBioseqSet> cmd(new CCmdInsertBioseqSet(m_TopSeqEntry, new_class));
        if (cmd)  m_CmdProccessor->Execute(cmd);                       
    }        
}


void CSequenceEditingEventHandler::ConvertSetType( wxCommandEvent& event )
{
    x_SetUpTSEandUnDoManager(m_Workbench);       
    if (!m_TopSeqEntry || ! m_CmdProccessor) return;

    if (!m_TopSeqEntry.IsSet()) {
        return;
    }

    CConvertSetTypeDlg dlg(NULL, m_TopSeqEntry.GetCompleteSeq_entry()->GetSet());
    if (dlg.ShowModal() == wxID_OK) 
    {
        bool error = false;
        CRef<CCmdComposite> cmd = dlg.GetCommand(m_TopSeqEntry.GetScope(),
                                                 m_TopSeqEntry.GetCompleteSeq_entry()->GetSet(),
                                                 error);
        if (error) {
            wxMessageBox(wxT("Set structure no longer matches"), wxT("Error"),
                                 wxOK | wxICON_ERROR, NULL);
        } else if (cmd) {
            m_CmdProccessor->Execute(cmd); 
        } else {
            wxMessageBox(wxT("No changes requested"), wxT("Error"),
                                 wxOK | wxICON_ERROR, NULL);
        }
    }
}

void CSequenceEditingEventHandler::PropagateDBLink( wxCommandEvent& event )
{
    x_SetUpTSEandUnDoManager(m_Workbench);       
    if (!m_TopSeqEntry || ! m_CmdProccessor) return;
    CRef<CCmdComposite> cmd = GetPropagateDBLinkCmd(m_TopSeqEntry);
    if (cmd)  m_CmdProccessor->Execute(cmd);                                   
}

void CSequenceEditingEventHandler::MacroEditor( wxCommandEvent& event )
{
    wxWindow* main_window = NULL;
    CWorkbench* wb_frame = dynamic_cast<CWorkbench*>(m_Workbench);
    if (wb_frame)
        main_window = wb_frame->GetMainWindow();

    CRef<IGuiCoreHelper> gui_core_helper(new CGuiCoreHelper(m_Workbench));
    CMacroFlowEditor::GetInstance(main_window, gui_core_helper);
}

void CSequenceEditingEventHandler::RevCompSequences( wxCommandEvent& event )
{
    x_SetUpTSEandUnDoManager(m_Workbench);       // perhaps this is unnecessary here after IsObjectSelected has run
    if (!m_TopSeqEntry || ! m_CmdProccessor) return;

    CRevCompSequencesDlg dlg(NULL, m_TopSeqEntry);
    if (dlg.ShowModal() == wxID_OK) 
    {
        CRef<CCmdComposite> cmd = dlg.GetCommand();
        if (cmd)  m_CmdProccessor->Execute(cmd);                       
    }
}

void CSequenceEditingEventHandler::BioseqFeatsRevComp( wxCommandEvent& event )
{
    x_SetUpTSEandUnDoManager(m_Workbench);       // perhaps this is unnecessary here after IsObjectSelected has run
    if (!m_TopSeqEntry || ! m_CmdProccessor) return;

    CRevCompSequencesDlg dlg(NULL, m_TopSeqEntry);
    dlg.apply(m_TopSeqEntry,m_CmdProccessor,"BioseqFeatsRevComp", true, true, true);
}

void CSequenceEditingEventHandler::BioseqOnlyRevComp( wxCommandEvent& event )
{
    x_SetUpTSEandUnDoManager(m_Workbench);       // perhaps this is unnecessary here after IsObjectSelected has run
    if (!m_TopSeqEntry || ! m_CmdProccessor) return;

    CRevCompSequencesDlg dlg(NULL, m_TopSeqEntry);
    dlg.apply(m_TopSeqEntry,m_CmdProccessor,"BioseqOnlyRevComp", true, false, true);
}

void CSequenceEditingEventHandler::UnculTaxTool( wxCommandEvent& event )
{
    x_SetUpTSEandUnDoManager(m_Workbench);       // perhaps this is unnecessary here after IsObjectSelected has run
    if (!m_TopSeqEntry || ! m_CmdProccessor) return;

    wxWindow* main_window = NULL;
    CWorkbench* wb_frame = dynamic_cast<CWorkbench*>(m_Workbench);
    if (wb_frame)
        main_window = wb_frame->GetMainWindow();
    try
    {
        CUnculTaxTool *dlg = new CUnculTaxTool(main_window, m_TopSeqEntry, m_Workbench);
        dlg->Show(true);      
    } catch  (CException&) {}
}

void CSequenceEditingEventHandler::ExtendPartialsConstr( wxCommandEvent& evt )
{
    CExtendPartialFeatDlg * dlg = new CExtendPartialFeatDlg(NULL, m_Workbench);
    dlg->Show(true);
}

void CSequenceEditingEventHandler::ExtendPartialsAll( wxCommandEvent& evt )
{
    CExtendPartialFeatDlg * dlg = new CExtendPartialFeatDlg(NULL, m_Workbench);
    dlg->ApplyToAllAndDie();
}

void CSequenceEditingEventHandler::ConvertCdsToMiscFeat( wxCommandEvent& evt )
{
    CConvertCdsToMiscFeat * dlg = new CConvertCdsToMiscFeat(NULL, m_Workbench);
    dlg->Show(true);
}

void CSequenceEditingEventHandler::ConvertCdsWithGapsToMiscFeat( wxCommandEvent& evt )
{
    wxWindow* main_window = NULL;
    CWorkbench* wb_frame = dynamic_cast<CWorkbench*>(m_Workbench);
    if (wb_frame)
        main_window = wb_frame->GetMainWindow();
    CConvertCdsWithGapsToMiscFeat * dlg = new CConvertCdsWithGapsToMiscFeat(main_window, m_Workbench);
    dlg->Show(true);
}


void CSequenceEditingEventHandler::AddFluComments( wxCommandEvent& event )
{
    x_SetUpTSEandUnDoManager(m_Workbench);       
    if (!m_TopSeqEntry || ! m_CmdProccessor) return;

    CAddFluComments worker;
    worker.apply(m_TopSeqEntry,m_CmdProccessor,"Add Flu Comments");
}


void CSequenceEditingEventHandler::LowercaseQuals( wxCommandEvent& evt )
{
    CLowerQuals* dlg = new CLowerQuals(NULL, m_Workbench);
    dlg->Show(true);
}

void CSequenceEditingEventHandler::AdjustForConsensusSpliceSites ( wxCommandEvent& event )
{
    x_SetUpTSEandUnDoManager(m_Workbench);       
    if (!m_TopSeqEntry || ! m_CmdProccessor) return;

    CRef<CCmdComposite> adjust_cds(new CCmdComposite("Adjust for consensus splice sites"));
    for (CBioseq_CI b_iter(m_TopSeqEntry, CSeq_inst::eMol_na); b_iter; ++b_iter) {
        
        bool good_seq = false;
        switch (event.GetId()) {
            case eCmdAdjustConsensusSpliceSitesStrict: 
            case eCmdAdjustConsensusSpliceSitesStrict_ext: 
                good_seq = CAdjustForConsensusSpliceSite::s_IsBioseqGood_Strict(*b_iter);
                break;
            case eCmdAdjustConsensusSpliceSitesRelaxed:
            case eCmdAdjustConsensusSpliceSitesRelaxed_ext:
                good_seq = CAdjustForConsensusSpliceSite::s_IsBioseqGood_Relaxed(*b_iter);
                break;
            case eCmdAdjustConsensusSpliceSitesConsensusEnds:
                good_seq = CAdjustForConsensusSpliceSite::s_IsBioseqGood_AdjustEnds(*b_iter);
                break;
        }
        if (good_seq) {
            if (event.GetId() == eCmdAdjustConsensusSpliceSitesConsensusEnds) {
                for (CFeat_CI feat(*b_iter, SAnnotSelector(CSeqFeatData::e_Cdregion)); feat; ++feat) {
                    CAdjustForConsensusSpliceSite worker(m_TopSeqEntry.GetScope());
                    CRef<CCmdComposite> cmd = worker.GetCommandToAdjustCDSEnds(feat->GetMappedFeature());
                    if (cmd) {
                        adjust_cds->AddCommand(*cmd);
                    }
                }
            }
            else {
                for (CFeat_CI feat(*b_iter, SAnnotSelector(CSeqFeatData::e_Cdregion)); feat; ++feat) {
                    CAdjustForConsensusSpliceSite worker(m_TopSeqEntry.GetScope());
                    CRef<CCmdComposite> cmd = worker.GetCommand(feat->GetMappedFeature());
                    if (cmd) {
                        adjust_cds->AddCommand(*cmd);
                    }
                }
            }
        }
    }
    
    m_CmdProccessor->Execute(adjust_cds);
}


void CSequenceEditingEventHandler::ConvertCdsWithInternalStopToMiscFeat( wxCommandEvent& evt )
{
    CConvertCdsToMiscFeat * dlg = new CConvertCdsToMiscFeat(NULL, m_Workbench);
    bool similar = true;
    bool nonfunctional = false;
    bool unverified = false;
    dlg->ApplyToAllAndDie(similar,nonfunctional,unverified);
}

void CSequenceEditingEventHandler::ConvertCdsWithInternalStopToMiscFeatViral( wxCommandEvent& evt )
{
    CConvertCdsToMiscFeat * dlg = new CConvertCdsToMiscFeat(NULL, m_Workbench);
    bool similar = false;
    bool nonfunctional = true;
    bool unverified = false;
    dlg->ApplyToAllAndDie(similar,nonfunctional,unverified);
}

void CSequenceEditingEventHandler::ConvertCdsWithInternalStopToMiscFeatUnverified( wxCommandEvent& evt )
{
    CConvertCdsToMiscFeat * dlg = new CConvertCdsToMiscFeat(NULL, m_Workbench);
    bool similar = true;
    bool nonfunctional = false;
    bool unverified = true;
    dlg->ApplyToAllAndDie(similar,nonfunctional,unverified);
}


void CSequenceEditingEventHandler::CombineSelectedGenesIntoPseudogenes( wxCommandEvent& evt )
{
    x_SetUpTSEandUnDoManager(m_Workbench);       
    if (!m_TopSeqEntry || ! m_CmdProccessor) return;

    CRef<CCmdComposite> cmd(new CCmdComposite("Combine Selected Genes into Pseudogene"));

    TConstScopedObjects &objects = m_SelObjects;
    if (objects.empty()) return;

    vector<const CSeq_feat*> genes;
    NON_CONST_ITERATE (TConstScopedObjects, it, objects) {
        const CObject* ptr = it->object.GetPointer();
        const CSeq_feat* seqfeat = dynamic_cast<const CSeq_feat*>(ptr);
        if (seqfeat && seqfeat->IsSetData() && seqfeat->GetData().IsGene())
        {
            genes.push_back(seqfeat);
        }
    }
    if (genes.size() != 2)
    {
        wxMessageBox(_("Please select two gene features to combine. Use Ctrl+Left mouse click"), wxT("Error"), wxOK | wxICON_ERROR, NULL);
        return;
    }
    CScope &scope = m_TopSeqEntry.GetScope();
    CRef<CSeq_feat> new_feat(new CSeq_feat());
    new_feat->Assign(*genes[0]);
    new_feat->SetPseudo(true);
    new_feat->SetLocation().Add(genes[1]->GetLocation());
    CRef<CSeq_loc> new_loc = sequence::Seq_loc_Merge(new_feat->GetLocation(), CSeq_loc::fMerge_SingleRange, &scope);
    new_loc->SetPartialStart(false, eExtreme_Biological);
    new_loc->SetPartialStop(false, eExtreme_Biological);
    new_feat->SetLocation(*new_loc);
    new_feat->SetPartial(false);
    vector<string> locus;
    if (genes[1]->GetData().GetGene().IsSetLocus())
        locus.push_back(genes[1]->GetData().GetGene().GetLocus());
    if (genes[0]->GetData().GetGene().IsSetLocus())
        locus.push_back(genes[0]->GetData().GetGene().GetLocus());
    new_feat->SetComment(NStr::Join(locus,";"));
    map<CBioseq_Handle, set<CSeq_feat_Handle> > product_to_cds;
    GetProductToCDSMap(scope, product_to_cds);
    CBioseq_Handle bsh = scope.GetBioseqHandle(new_feat->GetLocation());
    CSeq_entry_Handle seh = bsh.GetSeq_entry_Handle();
    cmd->AddCommand(*CRef<CCmdCreateFeat>(new CCmdCreateFeat(seh, *new_feat)));
    cmd->AddCommand(*GetDeleteFeatureCommand(scope.GetSeq_featHandle(*genes[0]), true, product_to_cds));
    cmd->AddCommand(*GetDeleteFeatureCommand(scope.GetSeq_featHandle(*genes[1]), true, product_to_cds));
    CConstRef<CSeq_feat> cds1 = sequence::GetOverlappingCDS(genes[0]->GetLocation(), scope);
    CConstRef<CSeq_feat> cds2 = sequence::GetOverlappingCDS(genes[1]->GetLocation(), scope);
    cmd->AddCommand(*GetDeleteFeatureCommand(scope.GetSeq_featHandle(*cds1), true, product_to_cds));
    cmd->AddCommand(*GetDeleteFeatureCommand(scope.GetSeq_featHandle(*cds2), true, product_to_cds));
    m_CmdProccessor->Execute(cmd);
}

void CSequenceEditingEventHandler::GeneFromOtherFeat( wxCommandEvent& evt )
{
    CGeneFeatFromOtherFeatDlg * dlg = new CGeneFeatFromOtherFeatDlg(NULL, m_Workbench);
    dlg->Show(true);
}


void CSequenceEditingEventHandler::LatLonTool( wxCommandEvent& event )
{
    x_SetUpTSEandUnDoManager(m_Workbench);       // perhaps this is unnecessary here after IsObjectSelected has run
    if (!m_TopSeqEntry || ! m_CmdProccessor) return;
   
    try
    {
        CLatLonTool dlg(NULL, m_TopSeqEntry);
    
        bool done = false;
        while (!done) {
            if (dlg.ShowModal() == wxID_OK) {
                CRef<CCmdComposite> cmd = dlg.GetCommand();
                if (cmd) {
                    m_CmdProccessor->Execute(cmd);                       
                    done = true;
                } else {
                    wxString error = ToWxString(dlg.GetErrorMessage());
                    wxMessageBox(error, wxT("Error"),
                                 wxOK | wxICON_ERROR, NULL);
                }
            } else {
                done = true;
            }
        }
    } catch  (CException&) {}
}

void CSequenceEditingEventHandler::CDSGeneRangeErrorSuppress( wxCommandEvent& event )
{
    CRef<CCmdComposite> cmd(new CCmdComposite("CDSGeneRangeError Suppress"));
    for (CFeat_CI feat_ci(m_TopSeqEntry); feat_ci; ++feat_ci)
    {
        const CSeq_feat& orig = feat_ci->GetOriginalFeature();
        CConstRef<CSeq_feat> contained = sequence::GetBestOverlappingFeat(orig.GetLocation(), CSeqFeatData::eSubtype_gene, sequence::eOverlap_Contained, m_TopSeqEntry.GetScope(), 0);
        CConstRef<CSeq_feat> overlapping = sequence::GetBestOverlappingFeat(orig.GetLocation(), CSeqFeatData::eSubtype_gene, sequence::eOverlap_Simple, m_TopSeqEntry.GetScope(), 0);
        bool gene_xref = false;
        FOR_EACH_SEQFEATXREF_ON_SEQFEAT(xref,orig)
        {
            if ((*xref)->IsSetData() && (*xref)->GetData().IsGene())
                gene_xref = true;
        }
        if (!contained && overlapping && !gene_xref)
        {
            CRef<CSeq_feat> new_feat(new CSeq_feat());
            new_feat->Assign(orig);
            CRef< CSeqFeatXref > xref(new CSeqFeatXref);
            xref->SetData().SetGene();
            new_feat->SetXref().push_back(xref);
            cmd->AddCommand(*CRef<CCmdChangeSeq_feat>(new CCmdChangeSeq_feat(feat_ci->GetSeq_feat_Handle(), *new_feat)));
        }
    }
    m_CmdProccessor->Execute(cmd);
}

void CSequenceEditingEventHandler::CDSGeneRangeErrorRestore( wxCommandEvent& event )
{
    CRef<CCmdComposite> cmd(new CCmdComposite("CDSGeneRangeError Restore"));
    for (CFeat_CI feat_ci(m_TopSeqEntry); feat_ci; ++feat_ci)
    {
        const CSeq_feat& orig = feat_ci->GetOriginalFeature();
        CConstRef<CSeq_feat> contained = sequence::GetBestOverlappingFeat(orig.GetLocation(), CSeqFeatData::eSubtype_gene, sequence::eOverlap_Contained, m_TopSeqEntry.GetScope(), 0);
        CConstRef<CSeq_feat> overlapping = sequence::GetBestOverlappingFeat(orig.GetLocation(), CSeqFeatData::eSubtype_gene, sequence::eOverlap_Simple, m_TopSeqEntry.GetScope(), 0);
        bool gene_xref = false;
        FOR_EACH_SEQFEATXREF_ON_SEQFEAT(xref,orig)
        {
            if ((*xref)->IsSetData() && (*xref)->GetData().IsGene())
                gene_xref = true;
        }
        if (!contained && overlapping && gene_xref)
        {
            CRef<CSeq_feat> new_feat(new CSeq_feat());
            new_feat->Assign(orig);
            CSeq_feat::TXref::iterator seq_feat_xref;
            EDIT_EACH_SEQFEATXREF_ON_SEQFEAT(xref,*new_feat)
            {
                if ((*xref)->IsSetData() && (*xref)->GetData().IsGene())
                    seq_feat_xref = xref;
            }
            new_feat->SetXref().erase(seq_feat_xref);
            if (new_feat->GetXref().empty())
                new_feat->ResetXref();
            cmd->AddCommand(*CRef<CCmdChangeSeq_feat>(new CCmdChangeSeq_feat(feat_ci->GetSeq_feat_Handle(), *new_feat)));
        }
    }
    m_CmdProccessor->Execute(cmd);
}

void CSequenceEditingEventHandler::ExportTable(wxCommandEvent& evt)
{
    x_SetUpTSEandUnDoManager(m_Workbench);       // perhaps this is unnecessary here after IsObjectSelected has run
    if (!m_TopSeqEntry || ! m_CmdProccessor) return;

    CExportTableDlg dlg(NULL, m_TopSeqEntry);
    if (dlg.ShowModal() == wxID_OK) 
    {
        dlg.SaveTable(m_WorkDir);
    }
}

void CSequenceEditingEventHandler::RemoveSegGaps( wxCommandEvent& event )
{
    x_SetUpTSEandUnDoManager(m_Workbench);
    if (!m_TopSeqEntry || ! m_CmdProccessor) return;

    CRef<CCmdComposite> cmd(new CCmdComposite("Remove Seg Gaps"));
    // So far, handle DENSEG type only
    SAnnotSelector align_sel(CSeq_annot::C_Data::e_Align);
    CAlign_CI align_ci(m_TopSeqEntry, align_sel);
    for ( ; align_ci; ++align_ci) {
        CRef<CSeq_align> new_align(new CSeq_align());
        new_align->Assign(align_ci.GetOriginalSeq_align());

        bool modified = macro::CMacroFunction_RemoveSegGaps::s_RemoveSegGaps(*new_align);
        if (modified) {
            CIRef<IEditCommand> chgAlign( new CCmdChangeAlign( align_ci.GetSeq_align_Handle(), *new_align ));
            cmd->AddCommand(*chgAlign);
        }
    }
    m_CmdProccessor->Execute(cmd);
}

void CSequenceEditingEventHandler::EditSequence( wxCommandEvent& event )
{
    wxWindow* main_window = NULL;
    CWorkbench* wb_frame = dynamic_cast<CWorkbench*>(m_Workbench);
    if (wb_frame)
        main_window = wb_frame->GetMainWindow();

    TConstScopedObjects objects;
    CIRef<CSelectionService> sel_srv = m_Workbench->GetServiceByType<CSelectionService>();
    if (sel_srv) 
    {
        sel_srv->GetActiveObjects(objects);
    }
    if (objects.empty())
    {
        GetViewObjects(m_Workbench, objects);
    }   
    
    if(objects.empty()) 
    {
        wxMessageBox(wxT("No objects found"), wxT("Error"), wxOK | wxICON_ERROR);
        return;
    }

    CRef<CScope> scope(&GetTopSeqEntryFromScopedObject(objects[0]).GetScope());
    if (!scope)
    {
        wxMessageBox(wxT("Cannot determine scope"), wxT("Error"), wxOK | wxICON_ERROR);
        return;
    }
    const CSeq_loc* loc = NULL;
    size_t count = 0;
    for (auto &o : objects)
    {
        const CSeq_loc* obj_loc = dynamic_cast<const CSeq_loc*>(o.object.GetPointer());
        if (!obj_loc)
            continue;
        if (!loc)
            loc = obj_loc;
        count++;
    }
   
    if (!loc)
    {
        wxMessageBox(wxT("The selection is not a bioseq"), wxT("Error"), wxOK | wxICON_ERROR);
        return;
    }
    if (count > 1)
    {
        wxMessageBox(wxT("Multiple bioseqs in scope, opening the first one only"), wxT("Warning"), wxOK | wxICON_ERROR);
    }
    int from = 0;
    CRef<CSeq_loc> whole_loc;
    if (!loc->IsWhole())
    {
        from = loc->GetStart(eExtreme_Positional) + 1;
        CBioseq_Handle bsh = scope->GetBioseqHandle(*loc);
        whole_loc = bsh.GetRangeSeq_loc(0,0);
    }
    if (whole_loc)
        loc = whole_loc.GetPointer();
    CEditSequence *dlg = NULL;
    try
    {
        dlg = new CEditSequence( main_window, loc, *scope, m_CmdProccessor, from); 
        dlg->Show(true);      
    } catch  (CException&) 
    {
        if (dlg)
            dlg->Destroy();
        wxMessageBox(wxT("No bioseq selected"), wxT("Error"), wxOK | wxICON_ERROR);
    }
   
}

void CSequenceEditingEventHandler::RawSeqToDeltaByNs( wxCommandEvent& event )
{
    x_SetUpTSEandUnDoManager(m_Workbench);
    if (!m_TopSeqEntry || ! m_CmdProccessor) return;

    CRawSeqToDeltaByN dlg(NULL);
    if (dlg.ShowModal() == wxID_OK) 
    {
        dlg.apply(m_TopSeqEntry, m_CmdProccessor, "Raw Sequences to Delta by Ns");
    }
}

void CSequenceEditingEventHandler::RawSeqToDeltaByLoc( wxCommandEvent& event )
{
    x_SetUpTSEandUnDoManager(m_Workbench);
    if (!m_TopSeqEntry || ! m_CmdProccessor) return;

    wxWindow* main_window = NULL;
    CWorkbench* wb_frame = dynamic_cast<CWorkbench*>(m_Workbench);
    if (wb_frame)
        main_window = wb_frame->GetMainWindow();

    CRawSeqToDeltaSeqByLoc dlg(main_window, m_TopSeqEntry);
    if (dlg.ShowModal() == wxID_OK) 
    {
        CRef<CCmdComposite> cmd = dlg.apply("Raw Sequences to Delta by Loc");
        if (cmd)  
            m_CmdProccessor->Execute(cmd);
    }
}

void CSequenceEditingEventHandler::RmCultureNotes( wxCommandEvent& event )
{
    x_SetUpTSEandUnDoManager(m_Workbench);
    if (!m_TopSeqEntry || ! m_CmdProccessor) return;

    CRef<CCmdComposite> cmd = GetRmCultureNotesCommand(m_TopSeqEntry.GetTopLevelEntry());
    if (cmd)  m_CmdProccessor->Execute(cmd);         
}

void CSequenceEditingEventHandler::Update_Replaced_EC_numbers( wxCommandEvent& event )
{
    x_SetUpTSEandUnDoManager(m_Workbench);
    if (!m_TopSeqEntry || ! m_CmdProccessor) return;

    CUpdateReplacedECNumbers worker;
    worker.apply(m_TopSeqEntry, m_CmdProccessor, "Update Replaced EC numbers");
}

void CSequenceEditingEventHandler::UpdateSingleSequence( wxCommandEvent& event)
{
    x_SetUpTSEandUnDoManager(m_Workbench);
    if (!m_TopSeqEntry || !m_CmdProccessor) return;

    CSeqUpdateWorker worker;
    switch (event.GetId()) {
    case eCmdUpdateSequence:
    case eCmdUpdateSequence_ext:
        worker.SetSeqUpdateType(CSeqUpdateWorker::eSingle_File);
        break;
    case eCmdUpdateSequenceClipboard:
        worker.SetSeqUpdateType(CSeqUpdateWorker::eSingle_Clipboard);
        break;
    case eCmdUpdateSequenceAccession:
        worker.SetSeqUpdateType(CSeqUpdateWorker::eSingle_Accession);
        break;
    }
    if (RunningInsideNCBI()) {
        worker.UpdateSingleSequence(m_Workbench, m_TopSeqEntry, m_CmdProccessor);
    } else {
        worker.UpdateSingleSequence_Ext(m_Workbench, m_TopSeqEntry, m_CmdProccessor);
    }
}

void CSequenceEditingEventHandler::UpdateMultipleSequences(wxCommandEvent& event)
{
    x_SetUpTSEandUnDoManager(m_Workbench);
    if (!m_TopSeqEntry || !m_CmdProccessor) return;

    CSeqUpdateWorker worker;
    switch (event.GetId()) {
    case eCmdUpdateMultiSequences:
        worker.SetSeqUpdateType(CSeqUpdateWorker::eMultiple_File);
        break;
    case eCmdUpdateMultiSeqClipboard:
        worker.SetSeqUpdateType(CSeqUpdateWorker::eMultiple_Clipboard);
        break;
    }
    worker.UpdateMultipleSequences(m_Workbench, m_TopSeqEntry, m_CmdProccessor);
}

void CSequenceEditingEventHandler::AddFeatureBetween( wxCommandEvent& event )
{
    x_SetUpTSEandUnDoManager(m_Workbench);       // perhaps this is unnecessary here after IsObjectSelected has run
    if (!m_TopSeqEntry || ! m_CmdProccessor) return;

    CAddFeatureBetweenDlg *dlg = new CAddFeatureBetweenDlg(NULL, m_TopSeqEntry, m_Workbench);
    dlg->Show(true);    
}

void CSequenceEditingEventHandler::ImportFeatureTable(wxCommandEvent& event)
{
    x_SetUpTSEandUnDoManager(m_Workbench);
    if (!m_TopSeqEntry || !m_CmdProccessor) return;

    //CImportFeatTable::ShowTableInfo(); // show informative message
    CImportFeatTable worker(m_TopSeqEntry);
    CIRef<IEditCommand> cmd = worker.ImportFeaturesFromFile(m_WorkDir, "file_loader_5col");
    if (!cmd)
        return;

    m_CmdProccessor->Execute(cmd);
}

void CSequenceEditingEventHandler::ImportGFF3(wxCommandEvent& event)
{
    x_SetUpTSEandUnDoManager(m_Workbench);
    if (!m_TopSeqEntry || !m_CmdProccessor) return;

    //CImportFeatTable::ShowTableInfo(); // show informative message
    CImportFeatTable worker(m_TopSeqEntry);
    CIRef<IEditCommand> cmd = worker.ImportFeaturesFromFile(m_WorkDir, "file_loader_gff");
    if (!cmd)
        return;

    m_CmdProccessor->Execute(cmd);
}

void CSequenceEditingEventHandler::ImportFeatureTableClipboard( wxCommandEvent& event)
{
    x_SetUpTSEandUnDoManager(m_Workbench);
    if (!m_TopSeqEntry || ! m_CmdProccessor) return;

    //CImportFeatTable::ShowTableInfo(); // show informative message
    CImportFeatTable worker(m_TopSeqEntry);
    CIRef<IEditCommand> cmd = worker.ImportFeaturesFromClipboard(m_WorkDir);
    if (!cmd)
        return;

    m_CmdProccessor->Execute(cmd);
}    

void CSequenceEditingEventHandler::IllegalQualsToNote( wxCommandEvent& event)
{
    x_SetUpTSEandUnDoManager(m_Workbench);
    if (!m_TopSeqEntry || ! m_CmdProccessor) return;

    CWrongIllegalQuals::IllegalQualsToNote( m_TopSeqEntry, m_CmdProccessor);  
}

void CSequenceEditingEventHandler::RmIllegalQuals( wxCommandEvent& event)
{
    x_SetUpTSEandUnDoManager(m_Workbench);
    if (!m_TopSeqEntry || ! m_CmdProccessor) return;

    CWrongIllegalQuals::RmIllegalQuals( m_TopSeqEntry, m_CmdProccessor);  
}

void CSequenceEditingEventHandler::WrongQualsToNote( wxCommandEvent& event)
{
    x_SetUpTSEandUnDoManager(m_Workbench);
    if (!m_TopSeqEntry || ! m_CmdProccessor) return;
   
    CWrongIllegalQuals::WrongQualsToNote( m_TopSeqEntry, m_CmdProccessor);  
}

void CSequenceEditingEventHandler::RmWrongQuals( wxCommandEvent& event)
{
    CWrongIllegalQuals::RmWrongQuals( m_TopSeqEntry, m_CmdProccessor);  
}

void CSequenceEditingEventHandler::GroupExplode( wxCommandEvent& event)
{
    CGroupExplode::apply( m_Workbench, m_CmdProccessor, m_TopSeqEntry.GetScope());  
}

void CSequenceEditingEventHandler::FindASN1( wxCommandEvent& evt )
{
    x_SetUpTSEandUnDoManager(m_Workbench);
    if (!m_TopSeqEntry || ! m_CmdProccessor) return;

    wxWindow* main_window = NULL;
    CWorkbench* wb_frame = dynamic_cast<CWorkbench*>(m_Workbench);
    if (wb_frame)
        main_window = wb_frame->GetMainWindow();
    CFindASN1Dlg * dlg = new CFindASN1Dlg(main_window, m_TopSeqEntry, m_CmdProccessor, m_SeqSubmit);
    dlg->Show(true);
}

void CSequenceEditingEventHandler::FuseFeatures( wxCommandEvent& evt )
{
    wxWindow* main_window = NULL;
    CWorkbench* wb_frame = dynamic_cast<CWorkbench*>(m_Workbench);
    if (wb_frame)
        main_window = wb_frame->GetMainWindow();
    CFuseFeaturesDlg * dlg = new CFuseFeaturesDlg(main_window, m_Workbench);
    dlg->Show(true);
}

void CSequenceEditingEventHandler::RemoveDupFeats( wxCommandEvent& evt )
{
    CRemoveFeaturesDlg::RemoveDuplicateFeatures(m_TopSeqEntry, m_CmdProccessor);
}

void CSequenceEditingEventHandler::RemoveDupFeatsWithOptions( wxCommandEvent& evt )
{
    wxWindow* main_window = NULL;
    CWorkbench* wb_frame = dynamic_cast<CWorkbench*>(m_Workbench);
    if (wb_frame)
        main_window = wb_frame->GetMainWindow();
    CRmDupFeaturesDlg * dlg = new CRmDupFeaturesDlg(main_window, m_Workbench);
    dlg->Show(true);
}

void CSequenceEditingEventHandler::ApplyStructuredCommentField( wxCommandEvent& evt )
{
    CAECRFrame::EActionType aecr_action_type =  CAECRFrame::eActionType_Apply;
    CAECRFrame::EFieldType aecr_field_type =  CAECRFrame::eFieldType_StructuredComment;
    CAECRFrame * dlg = new CAECRFrame(NULL, m_Workbench); 
    dlg->SetInitParameters(aecr_action_type, aecr_field_type);
    CallAfter(&CSequenceEditingEventHandler::ShowAfter, dlg);
}

void CSequenceEditingEventHandler::EditStructuredCommentPrefixSuffix( wxCommandEvent& evt )
{
    x_SetUpTSEandUnDoManager(m_Workbench);
    if (!m_TopSeqEntry || ! m_CmdProccessor) return;
    

    string strcomment_dbname;
    switch (evt.GetId()){
        case eCmdEditStructuredCommentPrefixSuffixHIVData:
            strcomment_dbname.assign("HIVDatabase");
            break;
        case eCmdEditStructuredCommentPrefixSuffixFluData:
            strcomment_dbname.assign("FluData");
            break;
        case eCmdEditStructuredCommentPrefixSuffixMIGSData:
            strcomment_dbname.assign("MIGS-Data");
            break;
        case eCmdEditStructuredCommentPrefixSuffixMIMSData:
            strcomment_dbname.assign("MIMS-Data");
            break;
        case eCmdEditStructuredCommentPrefixSuffixMIENSData:
            strcomment_dbname.assign("MIENS-Data");
            break;
        case eCmdEditStructuredCommentPrefixSuffixMIMARKS3_0:
            strcomment_dbname.assign("MIMARKS:3.0-Data");
            break;                    
        case eCmdEditStructuredCommentPrefixSuffixMIGS3_0:
            strcomment_dbname.assign("MIGS:3.0-Data");
            break;
        case eCmdEditStructuredCommentPrefixSuffixMIMS3_0:
            strcomment_dbname.assign("MIMS:3.0-Data");
            break;     
        case eCmdEditStructuredCommentPrefixSuffixAssemblyData:
            strcomment_dbname.assign("Assembly-Data");
            break;
        case eCmdEditStructuredCommentPrefixSuffixGenAssData:
            strcomment_dbname.assign("Genome-Assembly-Data");
            break;
        case eCmdEditStructuredCommentPrefixSuffixiBol:
            strcomment_dbname.assign("International Barcode of Life (iBOL)Data");
            break;    
    
    }

    CRef<CMiscSeqTableColumn> col(new CMiscSeqTableColumn(kStructCommDBName));
    vector<CConstRef<CObject> > objs = col->GetObjects(m_TopSeqEntry, kEmptyStr, CRef<edit::CStringConstraint>(NULL));
    if (objs.size() == 0) {
        wxMessageBox(wxT("No structured comments found!"), wxT("Error"),
                     wxOK | wxICON_ERROR, NULL);
        return;
    }

    bool any_change = false;
    CRef<CCmdComposite> cmd(new CCmdComposite("Edit Structured Comment DB name"));
    ITERATE(vector<CConstRef<CObject> >, it, objs) {
        CRef<CObject> new_obj = col->GetNewObject(*it);
        col->SetVal(*new_obj, strcomment_dbname, edit::eExistingText_replace_old);
        CRef<CCmdComposite> ecmd = col->GetReplacementCommand(*it, new_obj, 
            m_TopSeqEntry.GetScope(), "Edit Structured Comment Dbname Values");
        if (ecmd) {
            cmd->AddCommand(*ecmd);
            any_change = true;
        }
    }

    if (any_change) {
        m_CmdProccessor->Execute(cmd); 
    } else {
        wxMessageBox(wxT("No effect!"), wxT("Error"),
                     wxOK | wxICON_ERROR, NULL);
    }
}

void CSequenceEditingEventHandler::RemoveDbXrefsCDS( wxCommandEvent& event)
{
    x_SetUpTSEandUnDoManager(m_Workbench);
    if (!m_TopSeqEntry || ! m_CmdProccessor) return;

    CRemoveDbXrefsFeats::rm_dbxref_from_feat( m_TopSeqEntry, CSeqFeatData::e_Cdregion, m_CmdProccessor);  
}


void CSequenceEditingEventHandler::RemoveDbXrefsGenes( wxCommandEvent& event)
{
    x_SetUpTSEandUnDoManager(m_Workbench);
    if (!m_TopSeqEntry || ! m_CmdProccessor) return;

    CRemoveDbXrefsFeats::rm_dbxref_from_feat( m_TopSeqEntry, CSeqFeatData::e_Gene, m_CmdProccessor);  
}

void CSequenceEditingEventHandler::RemoveDbXrefsRNA( wxCommandEvent& event)
{
    x_SetUpTSEandUnDoManager(m_Workbench);
    if (!m_TopSeqEntry || ! m_CmdProccessor) return;

    CRemoveDbXrefsFeats::rm_dbxref_from_feat( m_TopSeqEntry, CSeqFeatData::e_Rna, m_CmdProccessor);  
}

void CSequenceEditingEventHandler::RemoveDbXrefsAllFeats( wxCommandEvent& event)
{
    x_SetUpTSEandUnDoManager(m_Workbench);
    if (!m_TopSeqEntry || ! m_CmdProccessor) return;

    CRemoveDbXrefsFeats::rm_dbxref_from_feat( m_TopSeqEntry, CSeqFeatData::e_not_set, m_CmdProccessor);  
}


void CSequenceEditingEventHandler::RemoveDbXrefsBioSource( wxCommandEvent& event)
{
    x_SetUpTSEandUnDoManager(m_Workbench);
    if (!m_TopSeqEntry || ! m_CmdProccessor) return;

    CRemoveDbXrefsBioSource::rm_dbxref_from_biosource( m_TopSeqEntry, m_CmdProccessor); 
}

void CSequenceEditingEventHandler::RemoveDbXrefsBioSourceAndFeats( wxCommandEvent& event)
{
    x_SetUpTSEandUnDoManager(m_Workbench);
    if (!m_TopSeqEntry || ! m_CmdProccessor) return;

    CRemoveDbXrefsBioSourceAndFeats::rm_dbxref_from_biosource_and_feats( m_TopSeqEntry, m_CmdProccessor); 
}

void CSequenceEditingEventHandler::RemoveTaxonFeats( wxCommandEvent& event)
{
    x_SetUpTSEandUnDoManager(m_Workbench);
    if (!m_TopSeqEntry || ! m_CmdProccessor) return;

    CRemoveTaxonFeats::rm_taxon_from_feats(m_TopSeqEntry, m_CmdProccessor);
}

void CSequenceEditingEventHandler::RemoveTaxonFeatsAndBioSource( wxCommandEvent& event)
{
    x_SetUpTSEandUnDoManager(m_Workbench);
    if (!m_TopSeqEntry || ! m_CmdProccessor) return;

    CRemoveTaxonFeatsAndBioSource::rm_taxon_from_feats_and_biosource(m_TopSeqEntry, m_CmdProccessor);
}

void CSequenceEditingEventHandler::CdsFromGeneMrnaExon( wxCommandEvent& evt )
{
    wxWindow* main_window = NULL;
    CWorkbench* wb_frame = dynamic_cast<CWorkbench*>(m_Workbench);
    if (wb_frame)
        main_window = wb_frame->GetMainWindow();
    CCdsFromGeneMrnaExon * dlg = new CCdsFromGeneMrnaExon(main_window, m_Workbench);
    dlg->Show(true);
}

void CSequenceEditingEventHandler::mRNAFromGeneCdsExon( wxCommandEvent& evt )
{
    wxWindow* main_window = NULL;
    CWorkbench* wb_frame = dynamic_cast<CWorkbench*>(m_Workbench);
    if (wb_frame)
        main_window = wb_frame->GetMainWindow();
    CmRNAFromGeneCdsExon * dlg = new CmRNAFromGeneCdsExon(main_window, m_Workbench);
    dlg->Show(true);
}

void CSequenceEditingEventHandler::tRNAFromGene( wxCommandEvent& evt )
{
    wxWindow* main_window = NULL;
    CWorkbench* wb_frame = dynamic_cast<CWorkbench*>(m_Workbench);
    if (wb_frame)
        main_window = wb_frame->GetMainWindow();
    CtRNAFromGene * dlg = new CtRNAFromGene(main_window, m_Workbench);
    dlg->Show(true);
}

void CSequenceEditingEventHandler::CdsToMatPeptide( wxCommandEvent& evt )
{
    wxWindow* main_window = NULL;
    CWorkbench* wb_frame = dynamic_cast<CWorkbench*>(m_Workbench);
    if (wb_frame)
        main_window = wb_frame->GetMainWindow();
    CCdsToMatPeptide * dlg = new CCdsToMatPeptide(main_window, m_Workbench);
    dlg->Show(true);
}

void CSequenceEditingEventHandler::SelectFeature( wxCommandEvent& event )
{
    wxWindow* main_window = NULL;
    CWorkbench* wb_frame = dynamic_cast<CWorkbench*>(m_Workbench);
    if (wb_frame)
        main_window = wb_frame->GetMainWindow();
    CSelectFeatures * dlg = new CSelectFeatures(main_window, m_Workbench);
    dlg->Show();
}


void CSequenceEditingEventHandler::RemoveSet( wxCommandEvent& event )
{
    CRemoveSet::remove_set(m_Workbench);
}

void CSequenceEditingEventHandler::RemoveSetsFromSet( wxCommandEvent& event )
{
    CRemoveSet::remove_sets_from_set(m_Workbench);
}

void CSequenceEditingEventHandler::EditSeqId( wxCommandEvent& event )
{
    x_SetUpTSEandUnDoManager(m_Workbench);       // perhaps this is unnecessary here after IsObjectSelected has run
    if (!m_TopSeqEntry || ! m_CmdProccessor) return;
    wxWindow* main_window = NULL;
    CWorkbench* wb_frame = dynamic_cast<CWorkbench*>(m_Workbench);
    if (wb_frame)
        main_window = wb_frame->GetMainWindow();

    CEditSeqId dlg(main_window, m_TopSeqEntry);
    if (dlg.ShowModal() == wxID_OK) 
    {
        CRef<CCmdComposite> cmd = dlg.GetCommand();
        if (cmd)  
            m_CmdProccessor->Execute(cmd);                       
        CRef<CCmdComposite> del_original_ids = dlg.GetRemoveOriginalIdCommand();
        if (del_original_ids)  
            m_CmdProccessor->Execute(del_original_ids);   
        dlg.Close();        
    }
}


void CSequenceEditingEventHandler::EditHistory( wxCommandEvent& event )
{
    x_SetUpTSEandUnDoManager(m_Workbench);       // perhaps this is unnecessary here after IsObjectSelected has run
    if (!m_TopSeqEntry || ! m_CmdProccessor) return;

    CBioseq_Handle bsh;
    vector<CRef<CSeq_loc> > &locs = x_GetSelectedLocations();
    if (!locs.empty()) {
        try
        {
            bsh = m_TopSeqEntry.GetScope().GetBioseqHandle(*locs.front());
        } catch (CException&) {}
    }
    
    if (!bsh)
        return;

    wxWindow* main_window = NULL;
    CWorkbench* wb_frame = dynamic_cast<CWorkbench*>(m_Workbench);
    if (wb_frame)
        main_window = wb_frame->GetMainWindow();

    try
    {
        CEditHistory dlg(main_window, bsh);
    
        if (dlg.ShowModal() == wxID_OK) 
        {
            CRef<CCmdComposite> cmd = dlg.GetCommand();
            if (cmd) 
            {
                  m_CmdProccessor->Execute(cmd);                       
            } 
        }        
    } catch  (CException&) {}
}

void CSequenceEditingEventHandler::ExonFromCds( wxCommandEvent& evt )
{
    wxWindow* main_window = NULL;
    CWorkbench* wb_frame = dynamic_cast<CWorkbench*>(m_Workbench);
    if (wb_frame)
        main_window = wb_frame->GetMainWindow();
    CExonFromCds * dlg = new CExonFromCds(main_window, m_Workbench, CSeqFeatData::eSubtype_cdregion, "Exon from CDS");
    dlg->Show(true);
}

void CSequenceEditingEventHandler::ExonFromMRNA( wxCommandEvent& evt )
{
    wxWindow* main_window = NULL;
    CWorkbench* wb_frame = dynamic_cast<CWorkbench*>(m_Workbench);
    if (wb_frame)
        main_window = wb_frame->GetMainWindow();
    CExonFromCds * dlg = new CExonFromCds(main_window, m_Workbench, CSeqFeatData::eSubtype_mRNA, "Exon from mRNA");
    dlg->Show(true);
}

void CSequenceEditingEventHandler::ExonFromTRNA( wxCommandEvent& evt )
{
    wxWindow* main_window = NULL;
    CWorkbench* wb_frame = dynamic_cast<CWorkbench*>(m_Workbench);
    if (wb_frame)
        main_window = wb_frame->GetMainWindow();
    CExonFromCds * dlg = new CExonFromCds(main_window, m_Workbench, CSeqFeatData::eSubtype_tRNA, "Exon from tRNA");
    dlg->Show(true);
}


void CSequenceEditingEventHandler::SetGlobalRefGeneStatus( wxCommandEvent& event )
{
    x_SetUpTSEandUnDoManager(m_Workbench);       // perhaps this is unnecessary here after IsObjectSelected has run
    if (!m_TopSeqEntry || ! m_CmdProccessor) return;
    wxWindow* main_window = NULL;
    CWorkbench* wb_frame = dynamic_cast<CWorkbench*>(m_Workbench);
    if (wb_frame)
        main_window = wb_frame->GetMainWindow();

    CSetGlobalRefGeneStatus dlg(main_window, m_TopSeqEntry);
    if (dlg.ShowModal() == wxID_OK) 
    {
        CRef<CCmdComposite> cmd = dlg.GetCommand();
        if (cmd) 
        {
            m_CmdProccessor->Execute(cmd);                       
        } 
    }        
}

void CSequenceEditingEventHandler::ClearKeywords( wxCommandEvent& event )
{
    x_SetUpTSEandUnDoManager(m_Workbench);       // perhaps this is unnecessary here after IsObjectSelected has run
    if (!m_TopSeqEntry || ! m_CmdProccessor) return;
    wxWindow* main_window = NULL;
    CWorkbench* wb_frame = dynamic_cast<CWorkbench*>(m_Workbench);
    if (wb_frame)
        main_window = wb_frame->GetMainWindow();

    CClearKeywords dlg(main_window, m_TopSeqEntry);
    if (dlg.ShowModal() == wxID_OK) 
    {
        CRef<CCmdComposite> cmd = dlg.GetCommand();
        if (cmd) 
        {
            m_CmdProccessor->Execute(cmd);                       
        } 
    }        
}

void CSequenceEditingEventHandler::ConvertToDelayedGenProdSetQuals( wxCommandEvent& event)
{
    x_SetUpTSEandUnDoManager(m_Workbench);
    if (!m_TopSeqEntry || ! m_CmdProccessor) return;

    m_CmdProccessor->Execute(CIRef<IEditCommand>(new CCmdConvertToDelayedGenProdSetQuals(m_TopSeqEntry)));
}

void CSequenceEditingEventHandler::ConvertRptUnitRangeToLoc( wxCommandEvent& event)
{
    x_SetUpTSEandUnDoManager(m_Workbench);
    if (!m_TopSeqEntry || ! m_CmdProccessor) return;

    CConvertRptUnitRangeToLoc::apply(m_TopSeqEntry, m_CmdProccessor);
}

void CSequenceEditingEventHandler::ReorderSeqById( wxCommandEvent& event)
{
    x_SetUpTSEandUnDoManager(m_Workbench);
    if (!m_TopSeqEntry || ! m_CmdProccessor) return;

    CRef<CCmdComposite>  cmd = CReorderSequencesDlg::ReorderById(m_TopSeqEntry);
    if (cmd) 
    {
        m_CmdProccessor->Execute(cmd);                       
    } 
}

void CSequenceEditingEventHandler::LinkmRNACDS( wxCommandEvent& event )
{
    x_SetUpTSEandUnDoManager(m_Workbench);
    if (!m_TopSeqEntry || ! m_CmdProccessor) return;

    CmRNACDSLinker worker(m_TopSeqEntry);
    switch (event.GetId()){
    case eCmdLinkSelectedmRNACDS:
        worker.LinkSelectedFeatures(m_SelObjects, m_CmdProccessor);
        break;
    case eCmdLinkmRNACDSOverlap:
        worker.LinkByOverlap(m_CmdProccessor);
        break;
    case eCmdLinkmRNACDSProduct:
        worker.LinkByProduct(m_CmdProccessor);
        break;
    case eCmdLinkmRNACDSLabel:
        worker.LinkByLabel(m_CmdProccessor);
        break;
    case eCmdLinkmRNACDSLabelLocation:
        worker.LinkByLabelAndLocation(m_CmdProccessor);
        break;
    case eCmdLinkmRNACDSProteinID:
        m_CmdProccessor->Execute(CCmdFeatIdXrefsFromQualifiers::Create(m_TopSeqEntry, { "protein_id", "orig_protein_id" }));
        break;
    case eCmdLinkmRNACDSTranscriptID:
        m_CmdProccessor->Execute(CCmdFeatIdXrefsFromQualifiers::Create(m_TopSeqEntry, { "transcript_id", "orig_transcript_id" }));
        break;
    case eCmdLinkmRNACDSUnidirectional:
        worker.CompleteHalfFormedXrefPairs(m_CmdProccessor);
        break;
    case eCmdShowLinkedmRNACDS:
    {
        vector<CConstRef<CObject> > objs = worker.GetReferencedmRNA_CDS(m_SelObjects);
        if (!objs.empty()) {
            CSelectionEvent evt(m_TopSeqEntry.GetScope()); 
            CSelectionService* service = m_Workbench->GetServiceByType<CSelectionService>();
            evt.AddObjectSelection(objs);
            service->Broadcast(evt, NULL);
        }
        break;
    }
    default:
        ;
    }
}

void CSequenceEditingEventHandler::JustRemoveProteins( wxCommandEvent& event )
{
    x_SetUpTSEandUnDoManager(m_Workbench);       // perhaps this is unnecessary here after IsObjectSelected has run
    if (!m_TopSeqEntry || ! m_CmdProccessor) return;

    CRef<CCmdComposite> cmd = CRemoveProteins::JustRemoveProteins(m_TopSeqEntry);
    if (cmd) {
        m_CmdProccessor->Execute(cmd);                       
    }     
}

void CSequenceEditingEventHandler::RemoveProteinsAndRenormalizeNucProtSets( wxCommandEvent& event )
{
    x_SetUpTSEandUnDoManager(m_Workbench);       // perhaps this is unnecessary here after IsObjectSelected has run
    if (!m_TopSeqEntry || ! m_CmdProccessor) return;

    CRef<CCmdComposite> cmd = CRemoveProteins::AndRenormalizeNucProtSets(m_TopSeqEntry);
    if (cmd) {
        m_CmdProccessor->Execute(cmd);                       
    }     
}

void CSequenceEditingEventHandler::RemoveOrphanedProteins( wxCommandEvent& event )
{
    x_SetUpTSEandUnDoManager(m_Workbench);       // perhaps this is unnecessary here after IsObjectSelected has run
    if (!m_TopSeqEntry || ! m_CmdProccessor) return;

    CRef<CCmdComposite> cmd = CRemoveProteins::OrphanedProteins(m_TopSeqEntry);
    if (cmd) {
        m_CmdProccessor->Execute(cmd);                       
    }     
}

void CSequenceEditingEventHandler::DeltaSeqToRaw( wxCommandEvent& event )
{
    x_SetUpTSEandUnDoManager(m_Workbench);       // perhaps this is unnecessary here after IsObjectSelected has run
    if (!m_TopSeqEntry || ! m_CmdProccessor) return;
    CRawSeqToDeltaByN::DeltaSeqToRaw(m_TopSeqEntry, m_CmdProccessor);
}


void CSequenceEditingEventHandler::GeneFeatFromXrefs( wxCommandEvent& event )
{
    x_SetUpTSEandUnDoManager(m_Workbench);       // perhaps this is unnecessary here after IsObjectSelected has run
    if (!m_TopSeqEntry || ! m_CmdProccessor) return;

    CRef<CCmdComposite> cmd = CGeneFeatFromXrefs::GetCommand(m_TopSeqEntry);
    if (cmd) {
        m_CmdProccessor->Execute(cmd);                       
    }     
}

void CSequenceEditingEventHandler::GeneXrefsFromFeats( wxCommandEvent& event )
{
    x_SetUpTSEandUnDoManager(m_Workbench);       // perhaps this is unnecessary here after IsObjectSelected has run
    if (!m_TopSeqEntry || ! m_CmdProccessor) return;

    CRef<CCmdComposite> cmd = CGeneXrefsFromFeats::GetCommand(m_TopSeqEntry);
    if (cmd) {
        m_CmdProccessor->Execute(cmd);                       
    }     
}

void CSequenceEditingEventHandler::RemoveAllGeneXrefs( wxCommandEvent& event )
{
    x_SetUpTSEandUnDoManager(m_Workbench);       // perhaps this is unnecessary here after IsObjectSelected has run
    if (!m_TopSeqEntry || ! m_CmdProccessor) return;

    CRef<CCmdComposite>  cmd = CRemoveGeneXrefs::RemoveAll(m_TopSeqEntry);
    if (cmd) {
        m_CmdProccessor->Execute(cmd);                       
    }     
}

void CSequenceEditingEventHandler::RemoveUnnecessaryGeneXrefs( wxCommandEvent& event )
{
    x_SetUpTSEandUnDoManager(m_Workbench);       // perhaps this is unnecessary here after IsObjectSelected has run
    if (!m_TopSeqEntry || ! m_CmdProccessor) return;

    CRef<CCmdComposite>  cmd = CRemoveGeneXrefs::RemoveUnnecessary(m_TopSeqEntry);
    if (cmd) {
        m_CmdProccessor->Execute(cmd);                       
    }     
}

void CSequenceEditingEventHandler::RemoveNonsuppressingGeneXrefs( wxCommandEvent& event )
{
    x_SetUpTSEandUnDoManager(m_Workbench);       // perhaps this is unnecessary here after IsObjectSelected has run
    if (!m_TopSeqEntry || ! m_CmdProccessor) return;

    CRef<CCmdComposite>  cmd = CRemoveGeneXrefs::RemoveNonsuppressing(m_TopSeqEntry);
    if (cmd) {
        m_CmdProccessor->Execute(cmd);                       
    }     
}

void CSequenceEditingEventHandler::RemoveOrphanLocusGeneXrefs( wxCommandEvent& event )
{
    x_SetUpTSEandUnDoManager(m_Workbench);       // perhaps this is unnecessary here after IsObjectSelected has run
    if (!m_TopSeqEntry || ! m_CmdProccessor) return;

    CRef<CCmdComposite>  cmd = CRemoveGeneXrefs::RemoveOrphanLocus(m_TopSeqEntry);
    if (cmd) {
        m_CmdProccessor->Execute(cmd);                       
    }     
}

void CSequenceEditingEventHandler::RemoveOrphanLocus_tagGeneXrefs( wxCommandEvent& event )
{
    x_SetUpTSEandUnDoManager(m_Workbench);       // perhaps this is unnecessary here after IsObjectSelected has run
    if (!m_TopSeqEntry || ! m_CmdProccessor) return;

    CRef<CCmdComposite>  cmd = CRemoveGeneXrefs::RemoveOrphanLocus_tag(m_TopSeqEntry);
    if (cmd) {
        m_CmdProccessor->Execute(cmd);                       
    }     
}

void CSequenceEditingEventHandler::RemoveGeneXrefs(wxCommandEvent& evt)
{
    if (!m_Workbench) return;
    wxWindow* main_window = NULL;
    CWorkbench* wb_frame = dynamic_cast<CWorkbench*>(m_Workbench);
    if (wb_frame)
        main_window = wb_frame->GetMainWindow();
    CRemoveGeneXrefs *dlg = new CRemoveGeneXrefs(main_window, m_Workbench);
    dlg->Show(true);
}

void CSequenceEditingEventHandler::ExtendCDS( wxCommandEvent& event )
{
    x_SetUpTSEandUnDoManager(m_Workbench);       // perhaps this is unnecessary here after IsObjectSelected has run
    if (!m_TopSeqEntry || ! m_CmdProccessor) return;
    
    CRef<CCmdComposite> cmd = CExtendPartialFeatDlg::ExtendCDS(m_TopSeqEntry);
    if (cmd) {
        m_CmdProccessor->Execute(cmd);                       
    }     
}

void CSequenceEditingEventHandler::TruncateCDS( wxCommandEvent& event )
{
    x_SetUpTSEandUnDoManager(m_Workbench);       // perhaps this is unnecessary here after IsObjectSelected has run
    if (!m_TopSeqEntry || ! m_CmdProccessor) return;

    wxWindow* main_window = NULL;
    CWorkbench* wb_frame = dynamic_cast<CWorkbench*>(m_Workbench);
    if (wb_frame)
        main_window = wb_frame->GetMainWindow();
    
    CTruncateCDS dlg(main_window, m_TopSeqEntry);
    if (dlg.ShowModal() == wxID_OK) 
    {
        CRef<CCmdComposite> cmd = dlg.GetCommand();
        if (cmd) 
        {
            m_CmdProccessor->Execute(cmd);                       
        } 
    }        
}

void CSequenceEditingEventHandler::FixProductNamesDefault( wxCommandEvent& event )
{
    x_SetUpTSEandUnDoManager(m_Workbench);       // perhaps this is unnecessary here after IsObjectSelected has run
    if (!m_TopSeqEntry || ! m_CmdProccessor) return;
 
   CRef<CCmdComposite> cmd = CFixProductNames::apply(m_TopSeqEntry);
   if (cmd) 
   {
       m_CmdProccessor->Execute(cmd);                       
   } 
}

void CSequenceEditingEventHandler::FixProductNamesOptions( wxCommandEvent& event )
{
    x_SetUpTSEandUnDoManager(m_Workbench);       // perhaps this is unnecessary here after IsObjectSelected has run
    if (!m_TopSeqEntry || ! m_CmdProccessor) return;
 
    wxWindow* main_window = NULL;
    CWorkbench* wb_frame = dynamic_cast<CWorkbench*>(m_Workbench);
    if (wb_frame)
        main_window = wb_frame->GetMainWindow();
    
    CFixProductNames dlg(main_window, m_TopSeqEntry);
    if (dlg.ShowModal() == wxID_OK) 
    {
        CRef<CCmdComposite> cmd = dlg.GetCommand();
        if (cmd) 
        {
            m_CmdProccessor->Execute(cmd);                       
        } 
    }        
}

void CSequenceEditingEventHandler::BulkApplyGenCode( wxCommandEvent& event )
{
    x_SetUpTSEandUnDoManager(m_Workbench);       // perhaps this is unnecessary here after IsObjectSelected has run
    if (!m_TopSeqEntry || ! m_CmdProccessor) return;
 
    wxWindow* main_window = NULL;
    CWorkbench* wb_frame = dynamic_cast<CWorkbench*>(m_Workbench);
    if (wb_frame)
        main_window = wb_frame->GetMainWindow();
    
    CApplyGCode dlg(main_window, m_TopSeqEntry);
    if (dlg.ShowModal() == wxID_OK) 
    {
        CRef<CCmdComposite> cmd = dlg.GetCommand();
        if (cmd) 
        {
            m_CmdProccessor->Execute(cmd);                       
            CRef<CCmdComposite> cleanup = CleanupCommand(m_TopSeqEntry, true, false);
            if (cleanup)
                m_CmdProccessor->Execute(cleanup);
            string error;
            CRef<CCmdComposite> retranslate = CRetranslateCDS::GetCommand(m_TopSeqEntry, "Retranslate coding regions", CRetranslateCDS::eRetranslateCDSIgnoreStopExceptEnd, error);
            if (retranslate)
                m_CmdProccessor->Execute(retranslate);
        } 
    }        
}

void CSequenceEditingEventHandler::EditSeqEndsWithAlign( wxCommandEvent& event )
{
    x_SetUpTSEandUnDoManager(m_Workbench);       // perhaps this is unnecessary here after IsObjectSelected has run
    if (!m_TopSeqEntry || ! m_CmdProccessor) return;
 
    wxWindow* main_window = NULL;
    CWorkbench* wb_frame = dynamic_cast<CWorkbench*>(m_Workbench);
    if (wb_frame)
        main_window = wb_frame->GetMainWindow();
    
    CEditSeqEndsWithAlign dlg(main_window, m_TopSeqEntry);
    if (dlg.ShowModal() == wxID_OK) 
    {
        CRef<CCmdComposite> cmd = dlg.GetCommand();
        if (cmd) 
        {
            m_CmdProccessor->Execute(cmd);                       
        } 
    }        
}

void CSequenceEditingEventHandler::UpdateAlign( wxCommandEvent& event )
{
    x_SetUpTSEandUnDoManager(m_Workbench);       // perhaps this is unnecessary here after IsObjectSelected has run
    if (!m_TopSeqEntry || ! m_CmdProccessor) return;
    
    wxWindow* main_window = NULL;
    CWorkbench* wb_frame = dynamic_cast<CWorkbench*>(m_Workbench);
    if (wb_frame)
        main_window = wb_frame->GetMainWindow();

    CUpdateAlign worker;
    worker.apply(m_TopSeqEntry, m_CmdProccessor, main_window);
}

void CSequenceEditingEventHandler::ResolveIntersectingFeats( wxCommandEvent& evt )
{
    wxWindow* main_window = NULL;
    CWorkbench* wb_frame = dynamic_cast<CWorkbench*>(m_Workbench);
    if (wb_frame)
        main_window = wb_frame->GetMainWindow();
    CResolveIntersectingFeats * dlg = new CResolveIntersectingFeats(main_window, m_Workbench);
    dlg->Show(true);
}

void CSequenceEditingEventHandler::CreateProteinId( wxCommandEvent& event )
{
    x_SetUpTSEandUnDoManager(m_Workbench);       // perhaps this is unnecessary here after IsObjectSelected has run
    if (!m_TopSeqEntry || ! m_CmdProccessor) return;
    
    wxWindow* main_window = NULL;
    CWorkbench* wb_frame = dynamic_cast<CWorkbench*>(m_Workbench);
    if (wb_frame)
        main_window = wb_frame->GetMainWindow();

    CCreateProteinId::apply(m_TopSeqEntry, m_CmdProccessor, main_window);
}

void CSequenceEditingEventHandler::CreateLocusTagGene( wxCommandEvent& event )
{
    x_SetUpTSEandUnDoManager(m_Workbench);       // perhaps this is unnecessary here after IsObjectSelected has run
    if (!m_TopSeqEntry || ! m_CmdProccessor) return;
    
    wxWindow* main_window = NULL;
    CWorkbench* wb_frame = dynamic_cast<CWorkbench*>(m_Workbench);
    if (wb_frame)
        main_window = wb_frame->GetMainWindow();

    CCreateLocusTagGene::apply(m_TopSeqEntry, m_CmdProccessor, main_window);
}

void CSequenceEditingEventHandler::NormalizeGeneQuals( wxCommandEvent& event )
{
    x_SetUpTSEandUnDoManager(m_Workbench);       // perhaps this is unnecessary here after IsObjectSelected has run
    if (!m_TopSeqEntry || ! m_CmdProccessor) return;
    
    CRef<CCmdComposite> cmd = GetNormalizeGeneQualsCommand(m_TopSeqEntry);
    if (cmd) 
    {
        m_CmdProccessor->Execute(cmd);                       
    } 
}

void CSequenceEditingEventHandler::GenusSpeciesFixup( wxCommandEvent& event )
{
    x_SetUpTSEandUnDoManager(m_Workbench);       // perhaps this is unnecessary here after IsObjectSelected has run
    if (!m_TopSeqEntry || ! m_CmdProccessor) return;
    
    CGenusSpeciesFixup worker;
    CRef<CCmdComposite> cmd = worker.GetCommand(m_TopSeqEntry);
    if (cmd)
    {
        m_CmdProccessor->Execute(cmd);
    }
}

void CSequenceEditingEventHandler::CountryConflict( wxCommandEvent& event )
{
    x_SetUpTSEandUnDoManager(m_Workbench);       // perhaps this is unnecessary here after IsObjectSelected has run
    if (!m_TopSeqEntry || ! m_CmdProccessor) return;

    wxWindow* main_window = NULL;
    CWorkbench* wb_frame = dynamic_cast<CWorkbench*>(m_Workbench);
    if (wb_frame)
        main_window = wb_frame->GetMainWindow();

    try
    {
        CCountryConflict dlg(main_window, m_TopSeqEntry);
    
        if (dlg.ShowModal() == wxID_OK) 
        {
            CRef<CCmdComposite> cmd = dlg.GetCommand();
                if (cmd) 
                {
                    m_CmdProccessor->Execute(cmd);                       
                } else {
                    wxString error = ToWxString(dlg.GetErrorMessage());
                    wxMessageBox(error, wxT("Error"),
                                 wxOK | wxICON_ERROR, NULL);
                }
        } 
        
    } catch  (CException&) {}
}

void CSequenceEditingEventHandler::AddSecondary( wxCommandEvent& event )
{
    x_SetUpTSEandUnDoManager(m_Workbench);       // perhaps this is unnecessary here after IsObjectSelected has run
    if (!m_TopSeqEntry || ! m_CmdProccessor) return;
   
    wxWindow* main_window = NULL;
    CWorkbench* wb_frame = dynamic_cast<CWorkbench*>(m_Workbench);
    if (wb_frame)
        main_window = wb_frame->GetMainWindow();

    CSeq_entry_Handle seh = m_TopSeqEntry;
    vector<CRef<CSeq_loc> > &locs = x_GetSelectedLocations();
    if (locs.size() == 1) {
        try
        {
            seh = m_TopSeqEntry.GetScope().GetBioseqHandle(*locs.front()).GetSeq_entry_Handle();
        } catch (CException&) {}
    }

    CAddSecondary dlg(main_window, seh);
    
    if (dlg.ShowModal() == wxID_OK) 
    {
        CRef<CCmdComposite> cmd = dlg.GetCommand();
        if (cmd) 
        {
            m_CmdProccessor->Execute(cmd);                       
        } 
    } 
        
}

void CSequenceEditingEventHandler::PackageFeaturesOnParts( wxCommandEvent& event )
{
    x_SetUpTSEandUnDoManager(m_Workbench);       // perhaps this is unnecessary here after IsObjectSelected has run
    if (!m_TopSeqEntry || ! m_CmdProccessor) return;
    
    CRef<CCmdComposite>  cmd = CPackageFeaturesOnParts::apply(m_TopSeqEntry);
    if (cmd) 
    {
        m_CmdProccessor->Execute(cmd);                       
    } 
}

void CSequenceEditingEventHandler::ShowFeatureTable(wxCommandEvent& event)
{
    x_SetUpTSEandUnDoManager(m_Workbench);       // perhaps this is unnecessary here after IsObjectSelected has run
    if (!m_TopSeqEntry || !m_CmdProccessor) return;

    wxWindow* main_window = NULL;
    CWorkbench* wb_frame = dynamic_cast<CWorkbench*>(m_Workbench);
    if (wb_frame)
        main_window = wb_frame->GetMainWindow();

    CFeatureTblFr* frame = new CFeatureTblFr(main_window, m_Workbench);
    frame->SetWorkDir(m_WorkDir);
    frame->Show(true);
}

void CSequenceEditingEventHandler::FixStructuredVouchers( wxCommandEvent& event )
{
    x_SetUpTSEandUnDoManager(m_Workbench);
    if (!m_TopSeqEntry || ! m_CmdProccessor) return;
    
    CFixStructuredVouchers worker;
    CRef<CCmdComposite> cmd = worker.GetCommand(m_TopSeqEntry);
    if (cmd) 
    {
        m_CmdProccessor->Execute(cmd);                       
    } 
}

void CSequenceEditingEventHandler::EditSequenceEnds( wxCommandEvent& event )
{
    x_SetUpTSEandUnDoManager(m_Workbench);       // perhaps this is unnecessary here after IsObjectSelected has run
    if (!m_TopSeqEntry || ! m_CmdProccessor) return;
   
    wxWindow* main_window = NULL;
    CWorkbench* wb_frame = dynamic_cast<CWorkbench*>(m_Workbench);
    if (wb_frame)
        main_window = wb_frame->GetMainWindow();

  
    CEditSequenceEnds dlg(main_window, m_TopSeqEntry);
    
    if (dlg.ShowModal() == wxID_OK) 
    {
        CRef<CCmdComposite> cmd = dlg.GetCommand();
        if (cmd) 
        {
            m_CmdProccessor->Execute(cmd);                       
        }         
    } 
        
}

void CSequenceEditingEventHandler::SaveDescriptors( wxCommandEvent& event )
{
    x_SetUpTSEandUnDoManager(m_Workbench);       // perhaps this is unnecessary here after IsObjectSelected has run
    if (!m_TopSeqEntry || ! m_CmdProccessor) return;
    
    wxWindow* main_window = NULL;
    CWorkbench* wb_frame = dynamic_cast<CWorkbench*>(m_Workbench);
    if (wb_frame)
        main_window = wb_frame->GetMainWindow();

    TConstScopedObjects &objects = m_SelObjects;
    if (objects.empty()) 
        return;
    
    const CObject* ptr = objects.front().object.GetPointer();
    const CBioseq* bioseq = dynamic_cast<const CBioseq*>(ptr);
    const CBioseq_set* bioseqset = dynamic_cast<const CBioseq_set*>(ptr);

    CRef<CSeq_descr> so;
    if (bioseq && bioseq->IsSetDescr())
    {
        so.Reset(new CSeq_descr);
        so->Assign(bioseq->GetDescr());
    }
    if (bioseqset && bioseqset->IsSetDescr())
    {
        so.Reset(new CSeq_descr);
        so->Assign(bioseqset->GetDescr());
    }

    if (so)
    {
        wxFileDialog asn_save_file(main_window, wxT("Export object to file"), wxEmptyString, wxEmptyString,
                                   CFileExtensions::GetDialogFilter(CFileExtensions::kASN) + wxT("|") +
                                   CFileExtensions::GetDialogFilter(CFileExtensions::kAllFiles),
                                   wxFD_SAVE|wxFD_OVERWRITE_PROMPT);
        
        if (asn_save_file.ShowModal() == wxID_OK)
        {
            wxString path = asn_save_file.GetPath();
            if( !path.IsEmpty())
            {
                ios::openmode mode = ios::out;
                CNcbiOfstream os(path.fn_str(), mode);
                os << MSerial_AsnText;
                os << *so;
            }
        }
    }

}

void CSequenceEditingEventHandler::CreateSeqHistForTpaDetailed( wxCommandEvent& event )
{
    x_SetUpTSEandUnDoManager(m_Workbench);       // perhaps this is unnecessary here after IsObjectSelected has run
    if (!m_TopSeqEntry || ! m_CmdProccessor) return;
   
    wxWindow* main_window = NULL;
    CWorkbench* wb_frame = dynamic_cast<CWorkbench*>(m_Workbench);
    if (wb_frame)
        main_window = wb_frame->GetMainWindow();

  
    CCreateSeqHistForTpa dlg(main_window, m_TopSeqEntry);
    
    if (dlg.ShowModal() == wxID_OK) 
    {
        CRef<CCmdComposite> cmd = dlg.GetCommand();
        if (cmd) 
        {
            m_CmdProccessor->Execute(cmd);                       
        } 
    }       
}

void CSequenceEditingEventHandler::CreateSeqHistForTpa( wxCommandEvent& event )
{
    x_SetUpTSEandUnDoManager(m_Workbench);       // perhaps this is unnecessary here after IsObjectSelected has run
    if (!m_TopSeqEntry || ! m_CmdProccessor) return;
   
    wxWindow* main_window = NULL;
    CWorkbench* wb_frame = dynamic_cast<CWorkbench*>(m_Workbench);
    if (wb_frame)
        main_window = wb_frame->GetMainWindow();

    string msg;
    CRef<CCmdComposite> cmd = CCreateSeqHistForTpa::GetCreateSeqHistFromTpaCommand(m_TopSeqEntry, msg, 28, 0.000001);

    if (cmd) 
    {
        m_CmdProccessor->Execute(cmd);                       
    } 
    
    if (!msg.empty())
    {
        CGenericReportDlg* report = new CGenericReportDlg(main_window); 
        report->SetTitle(wxT("TPA Alignment Assembly Problems"));
        report->SetText(wxString(msg));
        report->Show(true);
    }
}

void CSequenceEditingEventHandler::RemoveSeqHistAssembly( wxCommandEvent& event )
{
    x_SetUpTSEandUnDoManager(m_Workbench);       // perhaps this is unnecessary here after IsObjectSelected has run
    if (!m_TopSeqEntry || ! m_CmdProccessor) return;
    
    CRef<CCmdComposite> cmd = CRemoveSeqHistAssembly::GetCommand(m_TopSeqEntry);
    if (cmd) 
    {
        m_CmdProccessor->Execute(cmd);                       
    } 
}

void CSequenceEditingEventHandler::FarPointerSeq( wxCommandEvent& event )
{
    x_SetUpTSEandUnDoManager(m_Workbench);       // perhaps this is unnecessary here after IsObjectSelected has run
    if (!m_TopSeqEntry || ! m_CmdProccessor) return;
   
    wxWindow* main_window = NULL;
    CWorkbench* wb_frame = dynamic_cast<CWorkbench*>(m_Workbench);
    if (wb_frame)
        main_window = wb_frame->GetMainWindow();

    try
    {
        CFarPointerSeq dlg(main_window, m_TopSeqEntry);
        
        if (dlg.ShowModal() == wxID_OK) 
        {
            CRef<CCmdComposite> cmd = dlg.GetCommand();
            if (cmd) 
            {
                m_CmdProccessor->Execute(cmd);                       
            } 
        }       
    }
    catch  (CException&) 
    {
    }
}


void CSequenceEditingEventHandler::AlignmentAssistant( wxCommandEvent& event )
{
    x_SetUpTSEandUnDoManager(m_Workbench);       // perhaps this is unnecessary here after IsObjectSelected has run
    if (!m_TopSeqEntry || ! m_CmdProccessor) return;

    wxWindow* main_window = NULL;
    CWorkbench* wb_frame = dynamic_cast<CWorkbench*>(m_Workbench);
    if (wb_frame)
        main_window = wb_frame->GetMainWindow();

    
    CAlignmentAssistant *dlg = NULL;
    try
    {
        dlg = new CAlignmentAssistant( main_window, m_TopSeqEntry, m_CmdProccessor); 
        dlg->Show(true);      
    } catch  (CException&) 
    {
        if (dlg)
            dlg->Destroy();
        wxMessageBox(wxT("No alignment found"), wxT("Error"), wxOK | wxICON_ERROR);
    }
   
}

void CSequenceEditingEventHandler::AdjustFeaturesForGaps( wxCommandEvent& event )
{
    x_SetUpTSEandUnDoManager(m_Workbench);       // perhaps this is unnecessary here after IsObjectSelected has run
    if (!m_TopSeqEntry || ! m_CmdProccessor) return;
   
    wxWindow* main_window = NULL;
    CWorkbench* wb_frame = dynamic_cast<CWorkbench*>(m_Workbench);
    if (wb_frame)
        main_window = wb_frame->GetMainWindow();

  
    CAdjustFeaturesForGaps dlg(main_window, m_TopSeqEntry);
    
    bool done = false;
    while (!done) {
        if (dlg.ShowModal() == wxID_OK) {
            CRef<CCmdComposite> cmd = dlg.GetCommand();
            if (cmd)
            {
                m_CmdProccessor->Execute(cmd);
                if (dlg.LeaveUp()) {
                    dlg.Refresh();
                } else {
                    done = true;
                }
            }
        } else {
            done = true;
        }
    }       
}

void CSequenceEditingEventHandler::SplitCDSwithTooManyXs( wxCommandEvent& event )
{
    x_SetUpTSEandUnDoManager(m_Workbench);       // perhaps this is unnecessary here after IsObjectSelected has run
    if (!m_TopSeqEntry || ! m_CmdProccessor) return;
   
    try
    {
        CRef<CCmdComposite> cmd = CSplitCDSwithTooManyXs::apply(m_TopSeqEntry);
        if (cmd) 
        {
            m_CmdProccessor->Execute(cmd);                       
        } 
    }
    catch  (CException& e) 
    {
        wxMessageBox(wxString(e.GetMsg()), wxT("Error"),wxOK | wxICON_ERROR, NULL);
    }
}

void CSequenceEditingEventHandler::CompareWithBiosample( wxCommandEvent& event )
{
    x_SetUpTSEandUnDoManager(m_Workbench);       // perhaps this is unnecessary here after IsObjectSelected has run
    if (!m_TopSeqEntry || ! m_CmdProccessor) return;
   
    wxWindow* main_window = NULL;
    CWorkbench* wb_frame = dynamic_cast<CWorkbench*>(m_Workbench);
    if (wb_frame)
        main_window = wb_frame->GetMainWindow();

    CCompareWithBiosample *dlg(NULL);
    try
    {
        dlg = new CCompareWithBiosample(main_window, m_TopSeqEntry, m_CmdProccessor, true);
        dlg->Show(true);      
    } catch  (CException&) 
    {
        if (dlg)
            dlg->Destroy();
    }   
}

void CSequenceEditingEventHandler::CompareWithBiosampleFirstOnly( wxCommandEvent& event )
{
    x_SetUpTSEandUnDoManager(m_Workbench);       // perhaps this is unnecessary here after IsObjectSelected has run
    if (!m_TopSeqEntry || ! m_CmdProccessor) return;
   
    wxWindow* main_window = NULL;
    CWorkbench* wb_frame = dynamic_cast<CWorkbench*>(m_Workbench);
    if (wb_frame)
        main_window = wb_frame->GetMainWindow();

    CCompareWithBiosample *dlg(NULL);
    try
    {
        dlg = new CCompareWithBiosample(main_window, m_TopSeqEntry, m_CmdProccessor, false);
        dlg->Show(true);      
    } catch  (CException&) 
    {
        if (dlg)
            dlg->Destroy();
    }   
}


void CSequenceEditingEventHandler::DescriptorPropagateDown( wxCommandEvent& event )
{
    x_SetUpTSEandUnDoManager(m_Workbench);       // perhaps this is unnecessary here after IsObjectSelected has run
    if (!m_TopSeqEntry || ! m_CmdProccessor) return;
    

    CRef<CCmdComposite> cmd = CPropagateDescriptors::GetPropagateDownCommand(m_TopSeqEntry);

    if (cmd)
    m_CmdProccessor->Execute(cmd); 
}

void CSequenceEditingEventHandler::BarcodeTool( wxCommandEvent& event )
{
    x_SetUpTSEandUnDoManager(m_Workbench);       // perhaps this is unnecessary here after IsObjectSelected has run
    if (!m_TopSeqEntry || ! m_CmdProccessor) return;
   
    wxWindow* main_window = NULL;
    CWorkbench* wb_frame = dynamic_cast<CWorkbench*>(m_Workbench);
    if (wb_frame)
        main_window = wb_frame->GetMainWindow();

    CBarcodeTool *dlg(NULL);
    try
    {
        dlg = new CBarcodeTool(main_window, m_TopSeqEntry, m_CmdProccessor, m_Workbench);
        dlg->Show(true);      
    } catch  (CException&) 
    {
        if (dlg)
            dlg->Destroy();
    }   
}

void CSequenceEditingEventHandler::RestoreLocalFile(wxCommandEvent& event)
{
    x_SetUpTSEandUnDoManager(m_Workbench);       // perhaps this is unnecessary here after IsObjectSelected has run
    if (!m_TopSeqEntry || !m_CmdProccessor) return;

    CLocalFileRestorer restorer(m_Workbench);
    restorer.RestoreFile(event.GetId() == eCmdRestoreAndConvertSeqSubmit);
}

void CSequenceEditingEventHandler::RemoveLocalSeqIdsFromNuc(wxCommandEvent& event)
{
    x_SetUpTSEandUnDoManager(m_Workbench);       // perhaps this is unnecessary here after IsObjectSelected has run
    if (!m_TopSeqEntry || !m_CmdProccessor) return;

    CRemoveSeqId worker(m_TopSeqEntry, CSeq_id::e_Local, CSeq_inst::eMol_na);
    CRef<CCmdComposite> cmd = worker.GetCommand();
    if (cmd) 
    {
        m_CmdProccessor->Execute(cmd);                       
    } 
    CRef<CCmdComposite> del_original_ids = worker.GetRemoveOriginalIdCommand();
    if (del_original_ids)  
    {
        int answer = wxMessageBox (_("Remove Original Ids?"), _("Remove Original Ids"), wxYES_NO |  wxICON_QUESTION);
        if (answer == wxYES)
            m_CmdProccessor->Execute(del_original_ids);   
    }
}

void CSequenceEditingEventHandler::RemoveLocalSeqIdsFromProt(wxCommandEvent& event)
{
    x_SetUpTSEandUnDoManager(m_Workbench);       // perhaps this is unnecessary here after IsObjectSelected has run
    if (!m_TopSeqEntry || !m_CmdProccessor) return;

    CRemoveSeqId worker(m_TopSeqEntry, CSeq_id::e_Local, CSeq_inst::eMol_aa);
    CRef<CCmdComposite> cmd = worker.GetCommand();
    if (cmd) 
    {
        m_CmdProccessor->Execute(cmd);                       
    } 
    CRef<CCmdComposite> del_original_ids = worker.GetRemoveOriginalIdCommand();
    if (del_original_ids)  
    {
        int answer = wxMessageBox (_("Remove Original Ids?"), _("Remove Original Ids"), wxYES_NO |  wxICON_QUESTION);
        if (answer == wxYES)
            m_CmdProccessor->Execute(del_original_ids);   
    }
}

void CSequenceEditingEventHandler::RemoveLocalSeqIdsFromAll(wxCommandEvent& event)
{
    x_SetUpTSEandUnDoManager(m_Workbench);       // perhaps this is unnecessary here after IsObjectSelected has run
    if (!m_TopSeqEntry || !m_CmdProccessor) return;

    CRemoveSeqId worker(m_TopSeqEntry, CSeq_id::e_Local, CSeq_inst::eMol_not_set);
    CRef<CCmdComposite> cmd = worker.GetCommand();
    if (cmd) 
    {
        m_CmdProccessor->Execute(cmd);                       
    } 
    CRef<CCmdComposite> del_original_ids = worker.GetRemoveOriginalIdCommand();
    if (del_original_ids)  
    {
        int answer = wxMessageBox (_("Remove Original Ids?"), _("Remove Original Ids"), wxYES_NO |  wxICON_QUESTION);
        if (answer == wxYES)
            m_CmdProccessor->Execute(del_original_ids);   
    }
}

void CSequenceEditingEventHandler::RemoveGiSeqIdsFromAll(wxCommandEvent& event)
{
    x_SetUpTSEandUnDoManager(m_Workbench);       // perhaps this is unnecessary here after IsObjectSelected has run
    if (!m_TopSeqEntry || !m_CmdProccessor) return;

    CRemoveSeqId worker(m_TopSeqEntry, CSeq_id::e_Gi, CSeq_inst::eMol_not_set);
    CRef<CCmdComposite> cmd = worker.GetCommand();
    if (cmd) 
    {
        m_CmdProccessor->Execute(cmd);                       
    } 
}

void CSequenceEditingEventHandler::RemoveGenbankSeqIdsFromProt(wxCommandEvent& event)
{
    x_SetUpTSEandUnDoManager(m_Workbench);       // perhaps this is unnecessary here after IsObjectSelected has run
    if (!m_TopSeqEntry || !m_CmdProccessor) return;

    CRemoveSeqId worker1(m_TopSeqEntry, CSeq_id::e_Genbank, CSeq_inst::eMol_aa);
    CRef<CCmdComposite> cmd1 = worker1.GetCommand();
    if (cmd1) 
    {
        m_CmdProccessor->Execute(cmd1);                       
    } 
    CRemoveSeqId worker2(m_TopSeqEntry, CSeq_id::e_Other, CSeq_inst::eMol_aa);
    CRef<CCmdComposite> cmd2 = worker2.GetCommand();
    if (cmd2) 
    {
        m_CmdProccessor->Execute(cmd2);                       
    } 
}

void CSequenceEditingEventHandler::RemoveGenbankSeqIdsFromAll(wxCommandEvent& event)
{
    x_SetUpTSEandUnDoManager(m_Workbench);       // perhaps this is unnecessary here after IsObjectSelected has run
    if (!m_TopSeqEntry || !m_CmdProccessor) return;

    CRemoveSeqId worker(m_TopSeqEntry, CSeq_id::e_Genbank, CSeq_inst::eMol_not_set);
    CRef<CCmdComposite> cmd = worker.GetCommand();
    if (cmd) 
    {
        m_CmdProccessor->Execute(cmd);                       
    } 
}

void CSequenceEditingEventHandler::RemoveSeqIdNamesFromProtFeats(wxCommandEvent& event)
{
    x_SetUpTSEandUnDoManager(m_Workbench);       // perhaps this is unnecessary here after IsObjectSelected has run
    if (!m_TopSeqEntry || !m_CmdProccessor) return;

    CRemoveName worker(m_TopSeqEntry);
    CRef<CCmdComposite> cmd = worker.GetCommand();
    if (cmd) 
    {
        m_CmdProccessor->Execute(cmd);                       
    } 
}

void CSequenceEditingEventHandler::LocalToGeneralId(wxCommandEvent& event)
{
    x_SetUpTSEandUnDoManager(m_Workbench);       // perhaps this is unnecessary here after IsObjectSelected has run
    if (!m_TopSeqEntry || !m_CmdProccessor) return;

    CLocalToGeneralId worker(m_TopSeqEntry, CSeq_inst::eMol_na);
    CRef<CCmdComposite> cmd = worker.GetCommand();
    if (cmd) 
    {
        m_CmdProccessor->Execute(cmd);                       
    } 
}

void CSequenceEditingEventHandler::GeneralToLocalId(wxCommandEvent& event)
{
    x_SetUpTSEandUnDoManager(m_Workbench);       // perhaps this is unnecessary here after IsObjectSelected has run
    if (!m_TopSeqEntry || !m_CmdProccessor) return;

    CGeneralToLocalId worker(m_TopSeqEntry, CSeq_inst::eMol_not_set);
    CRef<CCmdComposite> cmd = worker.GetCommand();
    if (cmd) 
    {
        m_CmdProccessor->Execute(cmd);                       
    } 
}


void CSequenceEditingEventHandler::RemoveGeneralId( wxCommandEvent& event )
{
    x_SetUpTSEandUnDoManager(m_Workbench);       // perhaps this is unnecessary here after IsObjectSelected has run
    if (!m_TopSeqEntry || ! m_CmdProccessor) return;
    wxWindow* main_window = NULL;
    CWorkbench* wb_frame = dynamic_cast<CWorkbench*>(m_Workbench);
    if (wb_frame)
        main_window = wb_frame->GetMainWindow();

    CRemoveGeneralId dlg(main_window, m_TopSeqEntry);
    if (dlg.ShowModal() == wxID_OK) 
    {
        CRef<CCmdComposite> cmd = dlg.GetCommand();
        if (cmd)  
            m_CmdProccessor->Execute(cmd);                            
    }
}

void CSequenceEditingEventHandler::ConvertAccessionToLocalIdsAll(wxCommandEvent& event)
{
    x_SetUpTSEandUnDoManager(m_Workbench);       // perhaps this is unnecessary here after IsObjectSelected has run
    if (!m_TopSeqEntry || !m_CmdProccessor) return;

    CAccessionToLocalId worker(m_TopSeqEntry, CSeq_inst::eMol_not_set, false);
    CRef<CCmdComposite> cmd = worker.GetCommand();
    if (cmd) 
    {
        m_CmdProccessor->Execute(cmd);                       
    } 
}

void CSequenceEditingEventHandler::ConvertAccessionToLocalIdsNuc(wxCommandEvent& event)
{
    x_SetUpTSEandUnDoManager(m_Workbench);       // perhaps this is unnecessary here after IsObjectSelected has run
    if (!m_TopSeqEntry || !m_CmdProccessor) return;

    CAccessionToLocalId worker(m_TopSeqEntry, CSeq_inst::eMol_na, false);
    CRef<CCmdComposite> cmd = worker.GetCommand();
    if (cmd) 
    {
        m_CmdProccessor->Execute(cmd);                       
    } 
}

void CSequenceEditingEventHandler::ConvertAccessionToLocalIdsProt(wxCommandEvent& event)
{
    x_SetUpTSEandUnDoManager(m_Workbench);       // perhaps this is unnecessary here after IsObjectSelected has run
    if (!m_TopSeqEntry || !m_CmdProccessor) return;

    CAccessionToLocalId worker(m_TopSeqEntry, CSeq_inst::eMol_aa, false);
    CRef<CCmdComposite> cmd = worker.GetCommand();
    if (cmd) 
    {
        m_CmdProccessor->Execute(cmd);                       
    } 
}

void CSequenceEditingEventHandler::ConvertAccessionToLocalIdsName(wxCommandEvent& event)
{
    x_SetUpTSEandUnDoManager(m_Workbench);       // perhaps this is unnecessary here after IsObjectSelected has run
    if (!m_TopSeqEntry || !m_CmdProccessor) return;

    CAccessionToLocalId worker(m_TopSeqEntry, CSeq_inst::eMol_not_set, true);
    CRef<CCmdComposite> cmd = worker.GetCommand();
    if (cmd) 
    {
        m_CmdProccessor->Execute(cmd);                       
    } 
}

void CSequenceEditingEventHandler::AddAssemblyStructuredComment(wxCommandEvent& event)
{
    x_SetUpTSEandUnDoManager(m_Workbench);
    if (!m_TopSeqEntry || !m_CmdProccessor) return;

    CRef<CSeqdesc> seqdesc(new CSeqdesc());
    seqdesc->SetUser().Assign(*(edit::CStructuredCommentField::MakeUserObject("Assembly-Data")));

    CSeq_entry_Handle seh = m_TopSeqEntry;
    vector<CRef<CSeq_loc> > &orig_locs = x_GetSelectedLocations();
    if (!orig_locs.empty()) 
    {
        CBioseq_Handle bsh = m_TopSeqEntry.GetScope().GetBioseqHandle(*orig_locs.front());
        seh = bsh.GetSeq_entry_Handle();
    }

    CIRef<IEditObject> editor(new CEditObjectSeq_desc(*seqdesc, seh, m_TopSeqEntry.GetScope(), true));
    CEditObjViewDlg* edit_dlg = new CEditObjViewDlg(NULL, true);
    edit_dlg->SetUndoManager(m_CmdProccessor);
    wxWindow* editorWindow = editor->CreateWindow(edit_dlg);
    edit_dlg->SetEditorWindow(editorWindow);
    edit_dlg->SetEditor(editor);
    edit_dlg->SetWorkDir(m_WorkDir);
    edit_dlg->Show(true);
}

void CSequenceEditingEventHandler::EditCdsFrame(wxCommandEvent& event)
{
    x_SetUpTSEandUnDoManager(m_Workbench);       // perhaps this is unnecessary here after IsObjectSelected has run
    if (!m_TopSeqEntry || !m_CmdProccessor) return;

    wxWindow* main_window = NULL;
    CWorkbench* wb_frame = dynamic_cast<CWorkbench*>(m_Workbench);
    if (wb_frame)
        main_window = wb_frame->GetMainWindow();
    
    CEditCdsFrame dlg(main_window, m_TopSeqEntry);
    if (dlg.ShowModal() == wxID_OK) 
    {
        CRef<CCmdComposite> cmd = dlg.GetCommand();
        if (cmd)  
            m_CmdProccessor->Execute(cmd);                            
    }  
}

void CSequenceEditingEventHandler::ExpandGaps(wxCommandEvent& event)
{
    x_SetUpTSEandUnDoManager(m_Workbench);       // perhaps this is unnecessary here after IsObjectSelected has run
    if (!m_TopSeqEntry || !m_CmdProccessor) return;

    CRef<CCmdComposite> cmd = CExpandGaps::apply(m_TopSeqEntry);
    if (cmd) 
    {
        m_CmdProccessor->Execute(cmd);                       
    } 
}

void CSequenceEditingEventHandler::ConvertSelectedGapsToKnown(wxCommandEvent& event)
{
    x_SetUpTSEandUnDoManager(m_Workbench);       // perhaps this is unnecessary here after IsObjectSelected has run
    if (!m_TopSeqEntry || !m_CmdProccessor) return;

    CRef<CCmdComposite> cmd = CConvertSelectedGaps::apply(m_SelObjects, true, true);
    if (cmd) 
    {
        m_CmdProccessor->Execute(cmd);                       
    } 
}

void CSequenceEditingEventHandler::ConvertSelectedGapsToUnknown(wxCommandEvent& event)
{
    x_SetUpTSEandUnDoManager(m_Workbench);       // perhaps this is unnecessary here after IsObjectSelected has run
    if (!m_TopSeqEntry || !m_CmdProccessor) return;

    CRef<CCmdComposite> cmd = CConvertSelectedGaps::apply(m_SelObjects, false, true);
    if (cmd) 
    {
        m_CmdProccessor->Execute(cmd);                       
    } 
}

void CSequenceEditingEventHandler::ConvertGapsBySize(wxCommandEvent& event)
{
    x_SetUpTSEandUnDoManager(m_Workbench);       // perhaps this is unnecessary here after IsObjectSelected has run
    if (!m_TopSeqEntry || !m_CmdProccessor) return;

    CRef<CCmdComposite> cmd = CConvertGapsBySize::apply(m_TopSeqEntry);
    if (cmd) 
    {
        m_CmdProccessor->Execute(cmd);                       
    } 
}

void CSequenceEditingEventHandler::EditSelectedGaps(wxCommandEvent& event)
{
    x_SetUpTSEandUnDoManager(m_Workbench);       // perhaps this is unnecessary here after IsObjectSelected has run
    if (!m_TopSeqEntry || !m_CmdProccessor) return;

    CRef<CCmdComposite> cmd = CEditSelectedGaps::apply(m_SelObjects);
    if (cmd) 
    {
        m_CmdProccessor->Execute(cmd);                       
    } 
}

void CSequenceEditingEventHandler::CombineAdjacentGaps(wxCommandEvent& event)
{
    x_SetUpTSEandUnDoManager(m_Workbench);       // perhaps this is unnecessary here after IsObjectSelected has run
    if (!m_TopSeqEntry || !m_CmdProccessor) return;

    CRef<CCmdComposite> cmd = CCombineAdjacentGaps::apply(m_TopSeqEntry);
    if (cmd) 
    {
        m_CmdProccessor->Execute(cmd);                       
    } 
}

void CSequenceEditingEventHandler::AddLinkageToGaps(wxCommandEvent& event)
{
    x_SetUpTSEandUnDoManager(m_Workbench);       // perhaps this is unnecessary here after IsObjectSelected has run
    if (!m_TopSeqEntry || !m_CmdProccessor) return;

    wxWindow* main_window = NULL;
    CWorkbench* wb_frame = dynamic_cast<CWorkbench*>(m_Workbench);
    if (wb_frame)
        main_window = wb_frame->GetMainWindow();
    
    CAddLinkageToGaps dlg(main_window);
    if (dlg.ShowModal() == wxID_OK) 
    {
        CRef<CCmdComposite> cmd = dlg.GetCommand(m_TopSeqEntry);
        if (cmd)  
            m_CmdProccessor->Execute(cmd);                            
    }  
}

void CSequenceEditingEventHandler::ViewSortedProteins(wxCommandEvent& event)
{
    x_SetUpTSEandUnDoManager(m_Workbench);       // perhaps this is unnecessary here after IsObjectSelected has run
    if (!m_TopSeqEntry || !m_CmdProccessor) return;

    CSortedProteins sorted_prs(m_TopSeqEntry);
    sorted_prs.View();
}

void CSequenceEditingEventHandler::ConsolidateLikeModsWithSemicolon(wxCommandEvent& event)
{
    x_SetUpTSEandUnDoManager(m_Workbench);       // perhaps this is unnecessary here after IsObjectSelected has run
    if (!m_TopSeqEntry || !m_CmdProccessor) return;

    CConsolidateLikeMods worker("; ");
    CRef<CCmdComposite> cmd = worker.GetCommand(m_TopSeqEntry);
    if (cmd) 
    {
        m_CmdProccessor->Execute(cmd);                       
    } 
}

void CSequenceEditingEventHandler::ConsolidateLikeModsWithoutSemicolon(wxCommandEvent& event)
{
    x_SetUpTSEandUnDoManager(m_Workbench);       // perhaps this is unnecessary here after IsObjectSelected has run
    if (!m_TopSeqEntry || !m_CmdProccessor) return;

    CConsolidateLikeMods worker(" ");
    CRef<CCmdComposite> cmd = worker.GetCommand(m_TopSeqEntry);
    if (cmd) 
    {
        m_CmdProccessor->Execute(cmd);                       
    } 
}


void CSequenceEditingEventHandler::ApplyFBOL(wxCommandEvent& event)
{
    x_SetUpTSEandUnDoManager(m_Workbench);       // perhaps this is unnecessary here after IsObjectSelected has run
    if (!m_TopSeqEntry || !m_CmdProccessor) return;

    CRef<CCmdComposite> cmd = CApplyFBOL::apply(m_TopSeqEntry);
    if (cmd) 
    {
        m_CmdProccessor->Execute(cmd);                       
    } 
}

void CSequenceEditingEventHandler::ConvertSecondProtNameToDesc(wxCommandEvent& event)
{
    x_SetUpTSEandUnDoManager(m_Workbench);       // perhaps this is unnecessary here after IsObjectSelected has run
    if (!m_TopSeqEntry || !m_CmdProccessor) return;

    CRef<CCmdComposite> cmd = CConvertSecondProtNameToDesc::apply(m_TopSeqEntry);
    if (cmd) 
    {
        m_CmdProccessor->Execute(cmd);                       
    } 
}

void CSequenceEditingEventHandler::ConvertDescToSecondProtName(wxCommandEvent& event)
{
    x_SetUpTSEandUnDoManager(m_Workbench);       // perhaps this is unnecessary here after IsObjectSelected has run
    if (!m_TopSeqEntry || !m_CmdProccessor) return;

    CRef<CCmdComposite> cmd = CConvertDescToSecondProtName::apply(m_TopSeqEntry);
    if (cmd) 
    {
        m_CmdProccessor->Execute(cmd);                       
    } 
}

void CSequenceEditingEventHandler::FuseJoinsInLocs(wxCommandEvent& event)
{
    x_SetUpTSEandUnDoManager(m_Workbench);       // perhaps this is unnecessary here after IsObjectSelected has run
    if (!m_TopSeqEntry || !m_CmdProccessor) return;

    CRef<CCmdComposite> cmd = CFuseJoinsInLocs::apply(m_TopSeqEntry);
    if (cmd) 
    {
        m_CmdProccessor->Execute(cmd);                       
    } 
}

void CSequenceEditingEventHandler::ExplodeRNAFeats(wxCommandEvent& event)
{
    x_SetUpTSEandUnDoManager(m_Workbench);       // perhaps this is unnecessary here after IsObjectSelected has run
    if (!m_TopSeqEntry || !m_CmdProccessor) return;

    CRef<CCmdComposite> cmd = CExplodeRNAFeats::apply(m_TopSeqEntry);
    if (cmd) 
    {
        m_CmdProccessor->Execute(cmd);                       
    } 
}

void CSequenceEditingEventHandler::AdjustCDSForIntrons(wxCommandEvent& event)
{
    x_SetUpTSEandUnDoManager(m_Workbench);       // perhaps this is unnecessary here after IsObjectSelected has run
    if (!m_TopSeqEntry || !m_CmdProccessor) return;

    CRef<CCmdComposite> cmd = CAdjustFeatsForIntrons::apply(m_TopSeqEntry, CSeqFeatData::eSubtype_cdregion);
    if (cmd) 
    {
        m_CmdProccessor->Execute(cmd);                       
    } 
}

void CSequenceEditingEventHandler::AdjustrRNAForIntrons(wxCommandEvent& event)
{
    x_SetUpTSEandUnDoManager(m_Workbench);       // perhaps this is unnecessary here after IsObjectSelected has run
    if (!m_TopSeqEntry || !m_CmdProccessor) return;

    CRef<CCmdComposite> cmd = CAdjustFeatsForIntrons::apply(m_TopSeqEntry, CSeqFeatData::eSubtype_rRNA);
    if (cmd) 
    {
        m_CmdProccessor->Execute(cmd);                       
    } 
}

void CSequenceEditingEventHandler::AdjusttRNAForIntrons(wxCommandEvent& event)
{
    x_SetUpTSEandUnDoManager(m_Workbench);       // perhaps this is unnecessary here after IsObjectSelected has run
    if (!m_TopSeqEntry || !m_CmdProccessor) return;

    CRef<CCmdComposite> cmd = CAdjustFeatsForIntrons::apply(m_TopSeqEntry, CSeqFeatData::eSubtype_tRNA);
    if (cmd) 
    {
        m_CmdProccessor->Execute(cmd);                       
    } 
}

void CSequenceEditingEventHandler::AdjustmRNAForIntrons(wxCommandEvent& event)
{
    x_SetUpTSEandUnDoManager(m_Workbench);       // perhaps this is unnecessary here after IsObjectSelected has run
    if (!m_TopSeqEntry || !m_CmdProccessor) return;

    CRef<CCmdComposite> cmd = CAdjustFeatsForIntrons::apply(m_TopSeqEntry, CSeqFeatData::eSubtype_mRNA);
    if (cmd) 
    {
        m_CmdProccessor->Execute(cmd);                       
    } 
}

void CSequenceEditingEventHandler::ClearNomenclature(wxCommandEvent& event)
{
    x_SetUpTSEandUnDoManager(m_Workbench);       // perhaps this is unnecessary here after IsObjectSelected has run
    if (!m_TopSeqEntry || !m_CmdProccessor) return;

    CRef<CCmdComposite> cmd = CClearNomenclature::apply(m_TopSeqEntry);
    if (cmd) 
    {
        m_CmdProccessor->Execute(cmd);                       
    } 
}

void CSequenceEditingEventHandler::MergeBiosources(wxCommandEvent& event)
{
    x_SetUpTSEandUnDoManager(m_Workbench);       // perhaps this is unnecessary here after IsObjectSelected has run
    if (!m_TopSeqEntry || !m_CmdProccessor) return;

    CRef<CCmdComposite> cmd = CMergeBiosources::apply(m_TopSeqEntry);
    if (cmd) 
    {
        m_CmdProccessor->Execute(cmd);                       
    } 
}


void CSequenceEditingEventHandler::Duplicate(wxCommandEvent& event)
{
    x_SetUpTSEandUnDoManager(m_Workbench);       // perhaps this is unnecessary here after IsObjectSelected has run
    if (!m_TopSeqEntry || !m_CmdProccessor) return;

    ITERATE (TConstScopedObjects, it, m_SelObjects)
    {
        CConstRef<CObject> obj = (*it).object;

        CSeq_entry_Handle seh;
        const CSeq_feat* f = dynamic_cast<const CSeq_feat * >(obj.GetPointer());
        const CSeqdesc* d = dynamic_cast<const CSeqdesc *>(obj.GetPointer());
        const CBioseq_set* s = dynamic_cast<const CBioseq_set *>(obj.GetPointer());
        const CSeq_entry* e = dynamic_cast<const CSeq_entry*>(obj.GetPointer());
        if (s || e)
            continue;
        if (f)
        {
            CSeq_feat_Handle fh = m_TopSeqEntry.GetScope().GetSeq_featHandle(*f, CScope::eMissing_Null);
            if (fh)
            {
                const CSeq_annot_Handle& annot_handle = fh.GetAnnot();
                seh = annot_handle.GetParentEntry();
            }
        }
        if (d) 
        {
            seh = edit::GetSeqEntryForSeqdesc((*it).scope, *d); 
        }
        if (!seh)
        {
            CScope::TTSE_Handles tses;
            m_TopSeqEntry.GetScope().GetAllTSEs(tses, CScope::eAllTSEs);
            if (!tses.empty())
                seh = tses.front();
        }
    
        CIRef<IEditObject> editor = CreateEditorForObject(obj, seh, true);
        if (editor)
        {
            CEditObjViewDlg* edit_dlg = new CEditObjViewDlg(NULL, true);
            edit_dlg->SetUndoManager(m_CmdProccessor);
            wxWindow* editorWindow = editor->CreateWindow(edit_dlg);
            editorWindow->TransferDataToWindow(); 
            edit_dlg->SetEditorWindow(editorWindow);
            edit_dlg->SetEditor(editor);
            edit_dlg->SetWorkDir(m_WorkDir);
            edit_dlg->Show(true);
            break;
        }
    }
}

void CSequenceEditingEventHandler::SuppressGenes(wxCommandEvent& evt)
{
    if (!m_Workbench) return;
    wxWindow* main_window = NULL;
    CWorkbench* wb_frame = dynamic_cast<CWorkbench*>(m_Workbench);
    if (wb_frame)
        main_window = wb_frame->GetMainWindow();
    CSuppressGenes *dlg = new CSuppressGenes(main_window, m_Workbench);
    dlg->Show(true);
}


void CSequenceEditingEventHandler::ExportStructuredCommentsTable(wxCommandEvent& evt)
{
    x_SetUpTSEandUnDoManager(m_Workbench);       // perhaps this is unnecessary here after IsObjectSelected has run
    if (!m_TopSeqEntry || !m_CmdProccessor) return;

    wxWindow* main_window = NULL;
    CWorkbench* wb_frame = dynamic_cast<CWorkbench*>(m_Workbench);
    if (wb_frame)
        main_window = wb_frame->GetMainWindow();

    CRef<CSeq_table> table(new CSeq_table());
    CRef<CSeqTable_column> id_col(new CSeqTable_column());
    id_col->SetHeader().SetField_id(CSeqTable_column_info::eField_id_location_id);
    id_col->SetHeader().SetTitle(kSequenceIdColLabel);
    table->SetColumns().push_back(id_col);

    size_t row = 0;
    CBioseq_CI b_iter(m_TopSeqEntry, CSeq_inst::eMol_na);
    for ( ; b_iter ; ++b_iter ) 
    {
        CRef<CSeq_id> id(new CSeq_id());
        CSeq_id_Handle best = sequence::GetId(*b_iter, sequence::eGetId_Best);
        id->Assign (*(best.GetSeqId()));
        id_col->SetData().SetId().push_back(id);
        
        for (CSeqdesc_CI it (*b_iter, CSeqdesc::e_User); it; ++it)
        {
            if (it->GetUser().IsSetType() && it->GetUser().GetType().IsStr() && NStr::EqualNocase(it->GetUser().GetType().GetStr(), "StructuredComment") ) 
            {
                ITERATE (CUser_object::TData, field_it, it->GetUser().GetData()) 
                {
                    if ((*field_it)->IsSetLabel() && (*field_it)->GetLabel().IsStr() && (*field_it)->IsSetData()) 
                    {
                        vector<string> val;
                        if ((*field_it)->GetData().IsStr()) 
                        {
                            val.push_back((*field_it)->GetData().GetStr());
                        } 
                        else if ((*field_it)->GetData().IsStrs()) 
                        { 
                            for (vector<CStringUTF8>::const_iterator str_it = (*field_it)->GetData().GetStrs().begin(); str_it != (*field_it)->GetData().GetStrs().end(); ++str_it) 
                            {
                                val.push_back(*str_it);
                            }                          
                        }
                        AddValueToTable (table, (*field_it)->GetLabel().GetStr(), NStr::Join(val, ", "), row);
                    }
                }
            }
        }
        
        row++;
    }            
    table->SetNum_rows(row);
    FillShortColumns(table);
    CExportTableDlg::RemoveEmptyColsAndRows(table);

    wxFileDialog table_save_file(main_window, wxT("Export Structured Comment Table"), wxEmptyString, wxEmptyString,
                                 CFileExtensions::GetDialogFilter(CFileExtensions::kTable) + wxT("|") +
                                 CFileExtensions::GetDialogFilter(CFileExtensions::kAllFiles),
                                 wxFD_SAVE|wxFD_OVERWRITE_PROMPT);
    
    if (table_save_file.ShowModal() == wxID_OK) 
    {
        wxString path = table_save_file.GetPath();
        wxString name = table_save_file.GetFilename();

        name.Trim(false);
        name.Trim(true);

        if (name.IsEmpty())
        {            
            return; 
        }
        ios::openmode mode = ios::out;
        
        CNcbiOfstream os(path.fn_str(), mode); 
        if (!os)
        {
            wxMessageBox(wxT("Cannot open file ")+name, wxT("Error"), wxOK | wxICON_ERROR, main_window);
            return; 
        }
        
        CCSVExporter exporter(os, '\t', '"', false);
        ITERATE (CSeq_table::TColumns, cit, table->GetColumns()) {
            exporter.Field((*cit)->GetHeader().GetTitle());
        }
        exporter.NewRow();
        for (int i = 0; i < table->GetNum_rows(); ++i) {
            ITERATE (CSeq_table::TColumns, cit, table->GetColumns()) {
                if (i < (*cit)->GetData().GetSize()) {
                    if ((*cit)->GetData().IsId()) {
                        string label = "";
                        (*cit)->GetData().GetId()[i]->GetLabel(&label, CSeq_id::eContent);
                        exporter.Field(label);
                    } else if ((*cit)->GetData().IsString()) {
                        exporter.Field((*cit)->GetData().GetString()[i]);
                    }
                } else {
                    string blank = "";
                    exporter.Field(blank);
                }
            }
            exporter.NewRow();
        }
    }   
}
      
void CSequenceEditingEventHandler::ExtendCDSToStop( wxCommandEvent& event )
{
    x_SetUpTSEandUnDoManager(m_Workbench);       // perhaps this is unnecessary here after IsObjectSelected has run
    if (!m_TopSeqEntry || ! m_CmdProccessor) return;

       
    CExtendCDSToStop * dlg = new CExtendCDSToStop(NULL, m_Workbench);
    dlg->Show(true);
}

void CSequenceEditingEventHandler::RecomputeIntervals(wxCommandEvent& event)
{
    x_SetUpTSEandUnDoManager(m_Workbench);       // perhaps this is unnecessary here after IsObjectSelected has run
    if (!m_TopSeqEntry || !m_CmdProccessor) return;

    CRef<CCmdComposite> cmd = CRecomputeIntervals::apply(m_TopSeqEntry, false);
    if (cmd) 
    {
        m_CmdProccessor->Execute(cmd);                       
    } 
}

void CSequenceEditingEventHandler::RecomputeIntervalsAndUpdateGenes(wxCommandEvent& event)
{
    x_SetUpTSEandUnDoManager(m_Workbench);       // perhaps this is unnecessary here after IsObjectSelected has run
    if (!m_TopSeqEntry || !m_CmdProccessor) return;

    CRef<CCmdComposite> cmd = CRecomputeIntervals::apply(m_TopSeqEntry, true);
    if (cmd) 
    {
        m_CmdProccessor->Execute(cmd);                       
    } 
}

void CSequenceEditingEventHandler::ConvertBadInference(wxCommandEvent& event)
{
    x_SetUpTSEandUnDoManager(m_Workbench);       // perhaps this is unnecessary here after IsObjectSelected has run
    if (!m_TopSeqEntry || !m_CmdProccessor) return;

    CRef<CCmdComposite> cmd = CConvertBadInference::apply(m_TopSeqEntry);
    if (cmd) 
    {
        m_CmdProccessor->Execute(cmd);                       
    } 
}

void CSequenceEditingEventHandler::GlobalPubmedIdLookup(wxCommandEvent& event)
{
    x_SetUpTSEandUnDoManager(m_Workbench);       // perhaps this is unnecessary here after IsObjectSelected has run
    if (!m_TopSeqEntry || !m_CmdProccessor) return;

    CRef<CCmdComposite> cmd = CGlobalPubmedIdLookup::apply(m_TopSeqEntry);
    if (cmd) 
    {
        m_CmdProccessor->Execute(cmd);                       
    } 
}

void CSequenceEditingEventHandler::RemoveUnindexedFeatures(wxCommandEvent& event)
{
    x_SetUpTSEandUnDoManager(m_Workbench);       // perhaps this is unnecessary here after IsObjectSelected has run
    if (!m_TopSeqEntry || !m_CmdProccessor) return;

    CRef<CCmdComposite> cmd = CRemoveUnindexedFeatures::apply(m_TopSeqEntry);
    if (cmd) 
    {
        m_CmdProccessor->Execute(cmd);                       
    } 
}

void CSequenceEditingEventHandler::CorrectIntervalOrder( wxCommandEvent& event )
{
    x_SetUpTSEandUnDoManager(m_Workbench);       // perhaps this is unnecessary here after IsObjectSelected has run
    if (!m_TopSeqEntry || ! m_CmdProccessor) return;

       
    CCorrectIntervalOrder * dlg = new CCorrectIntervalOrder(NULL, m_Workbench);
    dlg->Show(true);
}

void CSequenceEditingEventHandler::LoadSecondaryAccessions(wxCommandEvent& event)
{
    x_SetUpTSEandUnDoManager(m_Workbench);       // perhaps this is unnecessary here after IsObjectSelected has run
    if (!m_TopSeqEntry || !m_CmdProccessor) return;

    wxWindow* main_window = NULL;
    CWorkbench* wb_frame = dynamic_cast<CWorkbench*>(m_Workbench);
    if (wb_frame)
        main_window = wb_frame->GetMainWindow();

    CRef<CCmdComposite> cmd = CLoadSecondaryAccessions::apply(main_window, m_TopSeqEntry.GetScope(), false);
    if (cmd) 
    {
        m_CmdProccessor->Execute(cmd);                       
    } 
}

void CSequenceEditingEventHandler::LoadSecondaryAccessionsHistoryTakeover(wxCommandEvent& event)
{
    x_SetUpTSEandUnDoManager(m_Workbench);       // perhaps this is unnecessary here after IsObjectSelected has run
    if (!m_TopSeqEntry || !m_CmdProccessor) return;

    wxWindow* main_window = NULL;
    CWorkbench* wb_frame = dynamic_cast<CWorkbench*>(m_Workbench);
    if (wb_frame)
        main_window = wb_frame->GetMainWindow();

    CRef<CCmdComposite> cmd = CLoadSecondaryAccessions::apply(main_window, m_TopSeqEntry.GetScope(), true);
    if (cmd) 
    {
        m_CmdProccessor->Execute(cmd);                       
    } 
}

void CSequenceEditingEventHandler::AddGlobalCodeBreak( wxCommandEvent& event )
{
    x_SetUpTSEandUnDoManager(m_Workbench);       // perhaps this is unnecessary here after IsObjectSelected has run
    if (!m_TopSeqEntry || ! m_CmdProccessor) return;

       
    CAddGlobalCodeBreak * dlg = new CAddGlobalCodeBreak(NULL, m_Workbench);
    dlg->Show(true);
}

void CSequenceEditingEventHandler::ConvertDescToFeatComment(wxCommandEvent& event)
{
    x_SetUpTSEandUnDoManager(m_Workbench);       // perhaps this is unnecessary here after IsObjectSelected has run
    if (!m_TopSeqEntry || !m_CmdProccessor) return;

    CRef<CCmdComposite> cmd = CConvertDescToFeatComment::apply(m_TopSeqEntry);
    if (cmd) 
    {
        m_CmdProccessor->Execute(cmd);                       
    } 
}

void CSequenceEditingEventHandler::ConvertDescToFeatSource(wxCommandEvent& event)
{
    x_SetUpTSEandUnDoManager(m_Workbench);       // perhaps this is unnecessary here after IsObjectSelected has run
    if (!m_TopSeqEntry || !m_CmdProccessor) return;

    CRef<CCmdComposite> cmd = CConvertDescToFeatSource::apply(m_TopSeqEntry);
    if (cmd) 
    {
        m_CmdProccessor->Execute(cmd);                       
    } 
}

void CSequenceEditingEventHandler::ConvertDescToFeatPub(wxCommandEvent& event)
{
    x_SetUpTSEandUnDoManager(m_Workbench);       // perhaps this is unnecessary here after IsObjectSelected has run
    if (!m_TopSeqEntry || !m_CmdProccessor) return;

    CRef<CCmdComposite> cmd = CConvertDescToFeatPub::apply(m_TopSeqEntry);
    if (cmd) 
    {
        m_CmdProccessor->Execute(cmd);                       
    } 
}

void CSequenceEditingEventHandler::ConvertDescToFeatPubConstraint(wxCommandEvent& event)
{
    x_SetUpTSEandUnDoManager(m_Workbench);       // perhaps this is unnecessary here after IsObjectSelected has run
    if (!m_TopSeqEntry || !m_CmdProccessor) return;

    CRef<CCmdComposite> cmd = CConvertDescToFeatPubConstraint::apply(m_TopSeqEntry);
    if (cmd) 
    {
        m_CmdProccessor->Execute(cmd);                       
    } 
}

void CSequenceEditingEventHandler::ConvertFeatToDescComment(wxCommandEvent& event)
{
    x_SetUpTSEandUnDoManager(m_Workbench);       // perhaps this is unnecessary here after IsObjectSelected has run
    if (!m_TopSeqEntry || !m_CmdProccessor) return;

    CRef<CCmdComposite> cmd = CConvertFeatToDescComment::apply(m_TopSeqEntry);
    if (cmd) 
    {
        m_CmdProccessor->Execute(cmd);                       
    } 
}

void CSequenceEditingEventHandler::ConvertFeatToDescSource(wxCommandEvent& event)
{
    x_SetUpTSEandUnDoManager(m_Workbench);       // perhaps this is unnecessary here after IsObjectSelected has run
    if (!m_TopSeqEntry || !m_CmdProccessor) return;

    CRef<CCmdComposite> cmd = CConvertFeatToDescSource::apply(m_TopSeqEntry);
    if (cmd) 
    {
        m_CmdProccessor->Execute(cmd);                       
    } 
}

void CSequenceEditingEventHandler::ConvertFeatToDescPub(wxCommandEvent& event)
{
    x_SetUpTSEandUnDoManager(m_Workbench);       // perhaps this is unnecessary here after IsObjectSelected has run
    if (!m_TopSeqEntry || !m_CmdProccessor) return;

    CRef<CCmdComposite> cmd = CConvertFeatToDescPub::apply(m_TopSeqEntry);
    if (cmd) 
    {
        m_CmdProccessor->Execute(cmd);                       
    } 
}

void CSequenceEditingEventHandler::LoadStructComments(wxCommandEvent& event)
{
    x_SetUpTSEandUnDoManager(m_Workbench);       // perhaps this is unnecessary here after IsObjectSelected has run
    if (!m_TopSeqEntry || !m_CmdProccessor) return;

    wxWindow* main_window = NULL;
    CWorkbench* wb_frame = dynamic_cast<CWorkbench*>(m_Workbench);
    if (wb_frame)
        main_window = wb_frame->GetMainWindow();

    CRef<CCmdComposite> cmd = CLoadStructComments::apply(main_window, m_WorkDir, m_TopSeqEntry);
    if (cmd) 
    {
        m_CmdProccessor->Execute(cmd);                       
    } 
}

void CSequenceEditingEventHandler::ConvertBadCdsAndRnaToMiscFeat(wxCommandEvent& event)
{
    x_SetUpTSEandUnDoManager(m_Workbench);       // perhaps this is unnecessary here after IsObjectSelected has run
    if (!m_TopSeqEntry || !m_CmdProccessor) return;

    wxWindow* main_window = NULL;
    CWorkbench* wb_frame = dynamic_cast<CWorkbench*>(m_Workbench);
    if (wb_frame)
        main_window = wb_frame->GetMainWindow();

    CRef<CCmdComposite> cmd = CConvertBadCdsAndRnaToMiscFeat::apply(main_window, m_TopSeqEntry);
    if (cmd) 
    {
        m_CmdProccessor->Execute(cmd);                       
    } 
}

void CSequenceEditingEventHandler::SuspectProductRulesEditor(wxCommandEvent& event)
{
    wxWindow* main_window = NULL;
    CWorkbench* wb_frame = dynamic_cast<CWorkbench*>(m_Workbench);
    if (wb_frame)
        main_window = wb_frame->GetMainWindow();

    CSuspectProductRulesEditor *dlg = new CSuspectProductRulesEditor(main_window, m_Workbench);
    dlg->Show(true);     
}

void CSequenceEditingEventHandler::CheckForMixedStrands(wxCommandEvent& event)
{
    x_SetUpTSEandUnDoManager(m_Workbench);       // perhaps this is unnecessary here after IsObjectSelected has run
    if (!m_TopSeqEntry || !m_CmdProccessor) return;

    CMixedStrands dlg(NULL, m_TopSeqEntry);
    if (dlg.ShowModal() == wxID_OK) {
        CRef<CCmdComposite> cmd = dlg.GetCommand();
        if (cmd) {
            m_CmdProccessor->Execute(cmd);
            dlg.Report();
        }
    }
}

void CSequenceEditingEventHandler::EnableHupId(wxCommandEvent& event)
{
    x_SetUpTSEandUnDoManager(m_Workbench);    
    if (!m_TopSeqEntry || !m_CmdProccessor) return;

    bool enable = event.IsChecked();
    CRef<CCmdEnableHupId> cmd(new CCmdEnableHupId(m_TopSeqEntry.GetScope(), enable));
    if (cmd) {
        m_CmdProccessor->Execute(cmd);
    }
    
    if (enable)
    {      
        if (!TestHupIdEnabled())
        {
            wxMenuItem* menu = dynamic_cast<wxMenuItem*>(event.GetEventObject());
            menu->Check(false);
            wxMessageBox(_("Loading HUP-ID failed, do you have permissions to access this server?"), wxT("Error"),  wxOK, NULL);
        }
    }
}

static wxString IndexerTypeToName(int i)
{
    switch(i)
    {
    case e_GenBankIndexer : return _("GenBank"); break;
    case  e_RefSeqIndexer : return _("RefSeq"); break;
    default : break;
    }
    return wxEmptyString;
}

void CSequenceEditingEventHandler::IndexerTypeSelection(wxCommandEvent& event)
{
    wxWindow* main_window = NULL;
    CWorkbench* wb_frame = dynamic_cast<CWorkbench*>(m_Workbench);
    if (wb_frame)
        main_window = wb_frame->GetMainWindow();
    wxArrayString choices;
    for (size_t i = 0; i < e_LastIndexer; i++)
    {
        choices.Add(IndexerTypeToName(i));
    }
    wxSingleChoiceDialog dlg(main_window, _("Select Indexer Type"), _("Indexer Type"), choices);
    CGuiRegistry& gui_reg = CGuiRegistry::GetInstance();
    int sel = gui_reg.GetReadView(kInternalIndexerRegistry)
        .GetInt(kInternalIndexerRegistryType, e_GenBankIndexer);
    dlg.SetSelection(sel);
    
    if (dlg.ShowModal() == wxID_OK)
    {
        sel = dlg.GetSelection();
        gui_reg.GetWriteView(kInternalIndexerRegistry)
            .Set(kInternalIndexerRegistryType, sel);
    }
}

void CSequenceEditingEventHandler::RemoveUnnecessaryExceptions(wxCommandEvent& event)
{
    x_SetUpTSEandUnDoManager(m_Workbench);       // perhaps this is unnecessary here after IsObjectSelected has run
    if (!m_TopSeqEntry || !m_CmdProccessor) return;

    CRemoveUnnecessaryExceptions::apply(m_TopSeqEntry, m_CmdProccessor);
}

void CSequenceEditingEventHandler::RawSeqToDeltaByAssemblyGapFeatures(wxCommandEvent& event)
{
    x_SetUpTSEandUnDoManager(m_Workbench);       // perhaps this is unnecessary here after IsObjectSelected has run
    if (!m_TopSeqEntry || !m_CmdProccessor) return;

    CRef<CCmdComposite> cmd = CRawSeqToDeltaSeqByLoc::ByAssemblyGapFeatures(m_TopSeqEntry);
    if (cmd) {
    m_CmdProccessor->Execute(cmd);
    }
}

void CSequenceEditingEventHandler::RestoreRNAediting(wxCommandEvent& event)
{
    x_SetUpTSEandUnDoManager(m_Workbench);       // perhaps this is unnecessary here after IsObjectSelected has run
    if (!m_TopSeqEntry || !m_CmdProccessor) return;

    wxWindow* main_window = NULL;
    CWorkbench* wb_frame = dynamic_cast<CWorkbench*>(m_Workbench);
    if (wb_frame)
        main_window = wb_frame->GetMainWindow();

    CRestoreRNAediting dlg(main_window, m_TopSeqEntry, m_CmdProccessor);
    dlg.ShowModal();
}

void CSequenceEditingEventHandler::FixForTransSplicing(wxCommandEvent& evt)
{
    if (!m_Workbench) return;
    wxWindow* main_window = NULL;
    CWorkbench* wb_frame = dynamic_cast<CWorkbench*>(m_Workbench);
    if (wb_frame)
        main_window = wb_frame->GetMainWindow();
    CFixForTransSplicing *dlg = new CFixForTransSplicing(main_window, m_Workbench);
    dlg->Show(true);
}

void CSequenceEditingEventHandler::RemoveDuplicateGoTerms(wxCommandEvent& evt)
{
    x_SetUpTSEandUnDoManager(m_Workbench);       // perhaps this is unnecessary here after IsObjectSelected has run
    if (!m_TopSeqEntry || !m_CmdProccessor) return;

    CRef<CCmdComposite> cmd = GetRemoveDuplicateGOTermsCommand(m_TopSeqEntry);
    if (cmd) {
        m_CmdProccessor->Execute(cmd);
    }
}

static CRef<CUser_object> GetParams()
{
    CRef<CUser_object> params(new CUser_object());
    CRef<CObject_id> type(new CObject_id());
    type->SetStr("TextViewParams");
    params->SetType(*type);
    CUser_object::TData& data = params->SetData();
    {
        CRef<CUser_field> param(new CUser_field());
        CRef<CObject_id> label(new CObject_id());
        label->SetStr("TextViewType");
        param->SetLabel(*label);
        param->SetData().SetStr("Flat File");
        data.push_back(param);
    }
    {
        CRef<CUser_field> param(new CUser_field());
        CRef<CObject_id> label(new CObject_id());
        label->SetStr("ExpandAll");
        param->SetLabel(*label);
        param->SetData().SetBool(true);
        data.push_back(param);
    }
    {
        CRef<CUser_field> param(new CUser_field());
        CRef<CObject_id> label(new CObject_id());
        label->SetStr("TrackSelection");
        param->SetLabel(*label);
        param->SetData().SetBool(true);
        data.push_back(param);
    }
    return params;
}

static tuple<CRef<CGBProjectHandle>,CRef<CProjectItem> >  LoadFileForSubmission(wxWindow* parent, CIRef<CProjectService> srv, CRef<CGBWorkspace> ws)
{
    CRef<CGBProjectHandle> ph;
    CRef<CProjectItem> pi;

    vector< CIRef<IFileFormatLoaderManager> > managers;
    GetExtensionAsInterface("file_format_loader_manager", managers);
    
    CRef<CFileLoadWizard> fileManager(new CFileLoadWizard());
    vector<string> format_ids(1,"file_loader_auto");
    format_ids.push_back("file_loader_fasta");
    format_ids.push_back("file_loader_asn");
    /* for (auto manager : managers)
       {
       format_ids.push_back(manager->GetFileLoaderId());
       }*/
    fileManager->LoadFormats(format_ids);
    
    vector<CIRef<IOpenObjectsPanelClient> > loadManagers;
    loadManagers.push_back(CIRef<IOpenObjectsPanelClient>(fileManager.GetPointer()));
    COpenObjectsDlg dlg(parent);
    dlg.SetSize(710, 480);
    dlg.SetRegistryPath("Dialogs.Edit.OpenSubmissionFile");
    dlg.SetManagers(loadManagers);
    if (dlg.ShowModal() == wxID_OK) 
    {
        CIRef<IObjectLoader> object_loader(dlg.GetObjectLoader());
        if (!object_loader) 
            return make_tuple(ph,pi);
        IExecuteUnit* execute_unit = dynamic_cast<IExecuteUnit*>(object_loader.GetPointer());
        if (!execute_unit || !execute_unit->PreExecute() || !GUI_AsyncExecUnit(*execute_unit, wxT("Reading file(s)...")) || !execute_unit->PostExecute())  
            return make_tuple(ph,pi);
        
        auto& objects = object_loader->GetObjects();
        const CBioseq* bseq = nullptr;
        for (auto& it : objects) {
            const CObject& ptr = it.GetObject();
            if (bseq = dynamic_cast<const CBioseq*>(&ptr)) {
                break;
            }
            else if (const CSeq_entry* entry = dynamic_cast<const CSeq_entry*>(&ptr)) {
                if (entry && entry->IsSeq()) {
                    LOG_POST(Info << MSerial_AsnText << entry->GetSeq());
                    bseq = &(entry->GetSeq());
                    break;
                }
            }
            else if (const CSeq_annot* annot = dynamic_cast<const CSeq_annot*>(&ptr)) {
                bseq = nullptr;
                break;
            }
        }
        
        if (bseq && bseq->IsAa()) {
            wxMessageBox(wxT("The file contains one protein sequence. Please import a nucleotide sequence file"), wxT("Error"),
                wxOK | wxICON_ERROR);
            return make_tuple(ph, pi);
        }

        CSelectProjectOptions options;
        options.Set_CreateNewProject(kEmptyStr);
        CObjectLoadingTask::AddObjects(srv->GetServiceLocator(), object_loader->GetObjects(), dynamic_cast<CLoaderDescriptor*>(object_loader->GetLoader()), options);
        wxDateTime start = wxDateTime::Now();
        wxDateTime now = start;
        while ((!ph || !pi)  && (now - start).GetSeconds() < 60)
        {
            for (auto project_handle : ws->GetWorkspace().GetProjects())
            {
                if (project_handle->GetProject().GetData().IsSetItems())
                {
                    for (auto item : project_handle->GetProject().GetData().GetItems())
                    {
                        if (item->IsSetLabel() && item->IsSetItem() && (item->GetItem().IsSubmit() || item->GetItem().IsEntry()))
                        {
                            ph = project_handle;
                            pi = item;
                            break;
                            
                        }
                    }           
                }
            }
            if (wxTheApp && wxTheApp->GetMainLoop())
            {
                if (!wxTheApp->GetMainLoop()->IsYielding())
                    wxTheApp->Yield(true);
                wxTheApp->ProcessPendingEvents();    
            }
            wxMilliSleep(100);
            now = wxDateTime::Now();
        }
    }
    return make_tuple(ph,pi);               
}

CRef<CProjectItem> CSequenceEditingEventHandler::CreateSeqSubmit(CRef<CGBProjectHandle> ph, CRef<CProjectItem> pi, CIRef<CProjectService> srv)
{
    if (!m_TopSeqEntry || !m_CmdProccessor)
        return pi;
    if (m_SeqSubmit)
        return pi;

    vector<CIRef<IProjectView>> views;
    srv->FindViews(views, *pi->GetObject());
    for (auto& it : views) 
    {
        srv->RemoveProjectView(*it);
    }

    CRef<CSubmit_block> submitBlock(new CSubmit_block);
    CGBenchVersionInfo version;
    GetGBenchVersionInfo(version);
    string version_str = "Genome Workbench ";
    version_str += NStr::IntToString(version.GetVer_major());
    version_str += ".";
    version_str += NStr::IntToString(version.GetVer_minor());
    version_str += ".";
    version_str += NStr::IntToString(version.GetVer_patch());
    version_str += " " + wxGetOsDescription().ToStdString();
    submitBlock->SetTool(version_str);

    submitBlock->SetContact().SetContact().SetName().SetName().SetLast("?");
    bool found_sub = false;
    for (CSeq_entry_CI entry_it(m_TopSeqEntry, CSeq_entry_CI::fRecursive | CSeq_entry_CI::fIncludeGivenEntry); entry_it; ++entry_it)
        for (CSeqdesc_CI desc_ci(*entry_it, CSeqdesc::e_Pub, 1); desc_ci; ++desc_ci)
            if (desc_ci->IsPub() && desc_ci->GetPub().IsSetPub() && desc_ci->GetPub().GetPub().IsSet() && !desc_ci->GetPub().GetPub().Get().empty() && desc_ci->GetPub().GetPub().Get().front()->IsSub())
            {
                submitBlock->SetCit().Assign(desc_ci->GetPub().GetPub().Get().front()->GetSub());
                found_sub = true;
                break;
            }

    if (!found_sub)
    {
        CRef< CAuthor > author(new CAuthor);
        author->SetName().SetName().SetLast("?");
        submitBlock->SetCit().SetAuthors().SetNames().SetStd().push_back(author);
    }
    if (!submitBlock->GetCit().IsSetDate()) {
        CRef<CDate> today(new CDate);
        today->SetToTime(CurrentTime(), CDate::ePrecision_day);
        submitBlock->SetCit().SetDate(*today);
    }

    CConstRef<CSeq_entry> entry = m_TopSeqEntry.GetCompleteSeq_entry();
    CRef<CSeq_entry> copy(new CSeq_entry());
    copy->Assign(*entry);
    
    CRef<CSeq_submit> submission(new CSeq_submit);
    submission->SetSub(submitBlock.GetNCObject());
    submission->SetData().SetEntrys().push_back(copy);      

    CGBDocument* doc = dynamic_cast<CGBDocument*>(ph.GetPointer());
    
    CRef<CProjectItem> new_item(new CProjectItem);
    new_item->SetDescr().assign(pi->GetDescr().begin(), pi->GetDescr().end());
    new_item->SetItem().SetSubmit(*submission);
    
    if (pi->IsSetLabel())
        new_item->SetLabel(pi->GetLabel());
    else
    {
        string label;
        CLabel::GetLabel(*copy, &label, CLabel::eDefault, nullptr);
        new_item->SetLabel(label);
    }
    
    CRef<CCmdChangeProjectItem> chg(new CCmdChangeProjectItem(*pi, doc, *new_item, m_Workbench));
    pi = new_item;
    m_CmdProccessor->Execute(chg);	
    CScope* scope = ph->GetScope();
    m_TopSeqEntry = scope->GetSeq_entryHandle(*copy);
    m_SeqSubmit.Reset(&pi->GetItem().GetSubmit());
    return pi;
}

void CSequenceEditingEventHandler::ChangeToGenomicDna()
{
    CRef<CCmdComposite> cmd(new CCmdComposite("Change na to genomic dna"));
    for (CBioseq_CI b_iter(m_TopSeqEntry, CSeq_inst::eMol_na); b_iter; ++b_iter)
    {
        CConstRef<CBioseq> bioseq = b_iter->GetCompleteBioseq();
        if (!bioseq || !bioseq->IsSetInst() || !bioseq->GetInst().IsSetMol() || bioseq->GetInst().GetMol() != CSeq_inst::eMol_na)
            continue;
        
        CRef<CBioseq> new_bioseq (new CBioseq());
        new_bioseq->Assign (*bioseq);
        new_bioseq->SetInst().SetMol(CSeq_inst::eMol_dna);
        CIRef<IEditCommand> chgInst(new CCmdChangeBioseqInst(*b_iter, new_bioseq->GetInst()));
        cmd->AddCommand(*chgInst);

        CSeqdesc_CI di(*b_iter, CSeqdesc::e_Molinfo, 1);
        if (di) 
        {
            const CSeqdesc& seqdesc = *di;
            CRef<CSeqdesc> edited_seqdesc(new CSeqdesc);
            edited_seqdesc->Assign(seqdesc);
            edited_seqdesc->SetMolinfo().SetBiomol(CMolInfo::eBiomol_genomic);
            CRef<CCmdChangeSeqdesc> chgd_cmd(new CCmdChangeSeqdesc(di.GetSeq_entry_Handle(), seqdesc, *edited_seqdesc));
            cmd->AddCommand(*chgd_cmd);
        } 
        else 
        {
            CRef<CSeqdesc> edited_seqdesc(new CSeqdesc);
            edited_seqdesc->SetMolinfo().SetBiomol(CMolInfo::eBiomol_genomic);
            CRef<CCmdCreateDesc> subcmd(new CCmdCreateDesc(b_iter->GetSeq_entry_Handle(), *edited_seqdesc));
            cmd->AddCommand(*subcmd);
        }
    }

    m_CmdProccessor->Execute(cmd);
}

void CSequenceEditingEventHandler::SetTopLevelBioseqSetClass()
{
    if (m_TopSeqEntry.IsSet()) {
        CBioseq_set_Handle bh = m_TopSeqEntry.GetSet();
        auto old_set = bh.GetCompleteBioseq_set();
        if (!old_set->IsSetClass() ||
            (old_set->IsSetClass() && old_set->GetClass() == CBioseq_set::eClass_not_set)) {
            CRef<CBioseq_set> new_set(new CBioseq_set());
            new_set->Assign(*old_set);
            new_set->SetClass(CBioseq_set::eClass_genbank);
            CRef<CCmdChangeBioseqSet> cmd(new CCmdChangeBioseqSet(bh, *new_set));
            m_CmdProccessor->Execute(cmd);
        }
        else if (old_set->IsSetClass() && old_set->GetClass() == CBioseq_set::eClass_nuc_prot) {
            CConstRef<CSeq_entry> entry = m_TopSeqEntry.GetCompleteSeq_entry();
            CRef<CSeq_entry> copy(new CSeq_entry());
            copy->Assign(*entry);

            CRef<CSeq_entry> outside_set(new CSeq_entry);
            outside_set->SetSet().SetClass(CBioseq_set::eClass_genbank);
            outside_set->SetSet().SetSeq_set().push_back(copy);

            CScope new_scope(*CObjectManager::GetInstance());
            new_scope.AddDefaults();
            CSeq_entry_Handle tmp_topseh = new_scope.AddTopLevelSeqEntry(*outside_set);
            CRef<CCmdChangeSeqEntry> wrap_cmd(new CCmdChangeSeqEntry(m_TopSeqEntry, outside_set));
            m_CmdProccessor->Execute(wrap_cmd);
        }
    }
    else if (m_TopSeqEntry.IsSeq()) {
        CConstRef<CSeq_entry> entry = m_TopSeqEntry.GetCompleteSeq_entry();
        CRef<CSeq_entry> copy(new CSeq_entry());
        copy->Assign(*entry);

        CScope new_scope(*CObjectManager::GetInstance());
        new_scope.AddDefaults();
        CSeq_entry_Handle tmp_topseh = new_scope.AddTopLevelSeqEntry(*copy);

        // wrap single sequence in genbank set
        CSeq_entry_EditHandle eh(tmp_topseh);
        eh.ConvertSeqToSet(CBioseq_set::eClass_genbank);
        CRef<CCmdChangeSeqEntry> wrap_cmd(new CCmdChangeSeqEntry(m_TopSeqEntry, copy));
        m_CmdProccessor->Execute(wrap_cmd);
    }
}

class CDoOnIdleTask : public CAppTask
{
public:
    CDoOnIdleTask(std::function<void()> f) : m_f(f) {}
protected:
    std::function<void()> m_f;
    virtual IAppTask::ETaskState x_Run()
    {
            m_f();
            return eCompleted;
    }
};

void CSequenceEditingEventHandler::PrepareSeqSubmit(wxCommandEvent& event)
{
    CIRef<CProjectService> srv = m_Workbench->GetServiceByType<CProjectService>();
    if (!srv) return;
    CRef<CGBWorkspace> ws = srv->GetGBWorkspace();
    if (!ws || !ws->IsSetWorkspace() || !ws->GetWorkspace().IsSetProjects()) return;

    wxWindow* main_window = nullptr;
    CWorkbench* wb_frame = dynamic_cast<CWorkbench*>(m_Workbench);
    if (wb_frame)
        main_window = wb_frame->GetMainWindow();

    vector<pair<CRef<CGBProjectHandle>, CRef<CProjectItem> > > projects;
    CRef<CGBProjectHandle> ph;
    CRef<CProjectItem> pi;
    wxArrayString choices;
    bool non_editable = false;
    for (auto project_handle : ws->GetWorkspace().GetProjects())
    {
        if (project_handle->GetProject().GetData().IsSetItems())
        {
            for (auto item : project_handle->GetProject().GetData().GetItems())
            {
                if (item->IsSetLabel())
                {
                    if (item->IsSetItem() && (item->GetItem().IsSubmit() || item->GetItem().IsEntry()))
                    {
                        projects.push_back(make_pair(project_handle, item));
                        choices.Add(wxString(item->GetLabel()));
                    }
                    else
                        non_editable = true;
                }
            }           
        }
    }
    if (non_editable)
    {
        wxMessageBox(wxT("One or more projects contain records which cannot be used for submission.\nThis is only an informational notice, this is not an error."), wxT("Information"), wxOK | wxICON_INFORMATION, NULL);
    }
    if (projects.empty())
    {       
        tie(ph,pi) = LoadFileForSubmission(main_window, srv, ws);
    }
    else if (projects.size() == 1)
    {
        tie(ph,pi) = projects[0];
    }
    else
    {
        wxSingleChoiceDialog 	dlg(main_window, _("Select the project for the submission"), _("Multiple projects found"), choices);
        if (dlg.ShowModal() == wxID_OK)
        {
            int sel = dlg.GetSelection();
            if (sel != wxNOT_FOUND)
            {
                tie(ph,pi) = projects[sel];
            }
        }
    }
    if (!ph || !pi)
        return;

    CGBDocument* doc = dynamic_cast<CGBDocument*>(ph.GetPointer());
    if (doc) 
    {
        m_CmdProccessor = &doc->GetUndoManager();
        m_WorkDir = doc->GetWorkDir();
    }
    
    if (!m_CmdProccessor) return;

    m_SeqSubmit.Reset();
    m_TopSeqEntry.Reset();
    if (pi->IsSetItem() && pi->GetItem().IsSubmit())
        m_SeqSubmit.Reset(&pi->GetItem().GetSubmit());


    CScope* scope = ph->GetScope();
    if (m_SeqSubmit)
    {
        if (m_SeqSubmit->IsSetData() && m_SeqSubmit->GetData().IsEntrys())
        {
            for (auto entry : m_SeqSubmit->GetData().GetEntrys())
            {
                if (entry)
                {
                    CSeq_entry_Handle seh = scope->GetSeq_entryHandle(*entry);
                    if (seh)
                    {
                        m_TopSeqEntry = seh.GetTopLevelEntry();
                        break;
                    }
                }
            }
        }            
    }
    else if (pi->IsSetItem() && pi->GetItem().IsEntry())
    {
        CSeq_entry_Handle seh = scope->GetSeq_entryHandle(pi->GetItem().GetEntry(), CScope::eMissing_Null);
        if (seh)
            m_TopSeqEntry = seh.GetTopLevelEntry();
    }

    if (!m_TopSeqEntry)  return;
      
    if (!m_SeqSubmit)
        pi = CreateSeqSubmit(ph, pi, srv);

    SetTopLevelBioseqSetClass();
    ChangeToGenomicDna();
    try {
        CRef<CCmdComposite> taxlookup_cmd = TaxonomyLookupCommand(m_TopSeqEntry);
        if (taxlookup_cmd)
            m_CmdProccessor->Execute(taxlookup_cmd);
    }
    catch (const CException& e) {
        LOG_POST(Error << e.GetMsg());
    }

    TConstScopedObjects objects;
    if (m_SeqSubmit)
        objects.push_back(SConstScopedObject(m_SeqSubmit, &m_TopSeqEntry.GetScope()));
    else
        objects.push_back(SConstScopedObject(m_TopSeqEntry.GetCompleteSeq_entry(), &m_TopSeqEntry.GetScope()));
    IProjectView* old_view = srv->FindView(*(objects.front().object), "Text View");
    if (!old_view)
        old_view = srv->AddProjectView("Text View", objects, GetParams());


    
    CAppTaskService* taskService = m_Workbench->GetServiceByType<CAppTaskService>();
    CDoOnIdleTask* task = new CDoOnIdleTask([=](){CSubmissionWizard::GetInstance(main_window, m_Workbench, ph, pi);});
    taskService->AddTask(*task);
}

void CSequenceEditingEventHandler::SaveSubmissionFile(wxCommandEvent& event)
{
    x_SetUpTSEandUnDoManager(m_Workbench);
    if (!m_TopSeqEntry || !m_CmdProccessor) return;

    wxWindow* main_window = NULL;
    CWorkbench* wb_frame = dynamic_cast<CWorkbench*>(m_Workbench);
    if (wb_frame)
        main_window = wb_frame->GetMainWindow();

    CConstRef<CSerialObject> so;
    if (m_SeqSubmit)
    {
        so = m_SeqSubmit;
    }
    else
    {

        int answer = wxMessageBox (_("You need to use the 'Genome Submission Wizard' to add contact information before submitting to GenBank - save incomplete file?"), 
                                   _("Save Incomplete File"), wxYES_NO |  wxICON_QUESTION);
        if (answer == wxYES)
        {
            so = m_TopSeqEntry.GetCompleteSeq_entry();
        }
        else
        {
            PrepareSeqSubmit(event);           
            return;
        }
    }

    if (!so)
        return;

    wxFileDialog asn_save_file(main_window, wxT("Save Submission File"), m_WorkDir, wxEmptyString,
                               CFileExtensions::GetDialogFilter(CFileExtensions::kASN) + wxT("|") +
                               CFileExtensions::GetDialogFilter(CFileExtensions::kAllFiles),
                               wxFD_SAVE|wxFD_OVERWRITE_PROMPT);
        
    if (asn_save_file.ShowModal() == wxID_OK)
    {
        wxString path = asn_save_file.GetPath();
        if( !path.IsEmpty())
        {
            ios::openmode mode = ios::out;
            CNcbiOfstream os(path.fn_str(), mode);
            os << MSerial_AsnText;
            os << *so;
        }
    }
    
}

void CSequenceEditingEventHandler::SaveASN1File(wxCommandEvent& event)
{
    x_SetUpTSEandUnDoManager(m_Workbench);
    if (!m_TopSeqEntry || !m_CmdProccessor) return;

    wxWindow* main_window = NULL;
    CWorkbench* wb_frame = dynamic_cast<CWorkbench*>(m_Workbench);
    if (wb_frame)
        main_window = wb_frame->GetMainWindow();

    CConstRef<CSerialObject> so = m_TopSeqEntry.GetCompleteSeq_entry();
    if (m_SeqSubmit)
    {
        so = m_SeqSubmit;
    }
    if (!so)
        return;

    wxFileDialog asn_save_file(main_window, wxT("Save ASN.1 File"), m_WorkDir, wxEmptyString,
                               CFileExtensions::GetDialogFilter(CFileExtensions::kASN) + wxT("|") +
                               CFileExtensions::GetDialogFilter(CFileExtensions::kAllFiles),
                               wxFD_SAVE|wxFD_OVERWRITE_PROMPT);
        
    if (asn_save_file.ShowModal() == wxID_OK)
    {
        wxString path = asn_save_file.GetPath();
        if( !path.IsEmpty())
        {
            ios::openmode mode = ios::out;
            CNcbiOfstream os(path.fn_str(), mode);
            os << MSerial_AsnText;
            os << *so;
        }
    }
    
}


void CSequenceEditingEventHandler::CreateSpecificStructuredComment( wxCommandEvent& event )
{
    x_SetUpTSEandUnDoManager(m_Workbench);       
    if (!m_TopSeqEntry || ! m_CmdProccessor) return;

    string prefix;
    switch (event.GetId()){
        case eCmdCreateStructuredCommentPrefixSuffixMIENSData:
            prefix.assign("MIENS-Data");
            break;
        case eCmdCreateStructuredCommentPrefixSuffixMIMARKS:
            prefix.assign("MIMARKS:5.0-Data");
            break;                    
        case eCmdCreateStructuredCommentPrefixSuffixMIGS:
            prefix.assign("MIGS:5.0-Data");
            break;
        case eCmdCreateStructuredCommentPrefixSuffixMIMS:
            prefix.assign("MIMS:5.0-Data");
            break;     
        case eCmdCreateStructuredCommentPrefixSuffixGenAssData:
            prefix.assign("Genome-Assembly-Data");
            break;
        case eCmdCreateStructuredCommentPrefixSuffixGenAnnData:
            prefix.assign("Genome-Annotation-Data");
            break;
        default : break;
    }

    if (prefix.empty())
        return;

    CRef<CSeqdesc> seqdesc(new CSeqdesc());
    seqdesc->SetUser().Assign(*edit::CStructuredCommentField::MakeUserObject(prefix));

    CSeq_entry_Handle seh = m_TopSeqEntry;
    vector<CRef<CSeq_loc> > &orig_locs = x_GetSelectedLocations();
    if (!orig_locs.empty()) 
    {
        CBioseq_Handle bsh = m_TopSeqEntry.GetScope().GetBioseqHandle(*orig_locs.front());
        seh = bsh.GetSeq_entry_Handle();
    }

    CIRef<IEditObject> editor(new CEditObjectSeq_desc(*seqdesc, seh, m_TopSeqEntry.GetScope(), true));
    CEditObjViewDlg* edit_dlg = new CEditObjViewDlg(NULL, true);
    edit_dlg->SetUndoManager(m_CmdProccessor);
    wxWindow* editorWindow = editor->CreateWindow(edit_dlg);
    edit_dlg->SetEditorWindow(editorWindow);
    edit_dlg->SetEditor(editor);
    edit_dlg->SetWorkDir(m_WorkDir);
    edit_dlg->Show(true);
}


void CSequenceEditingEventHandler::DeleteSpecificStructuredComment( wxCommandEvent& event )
{
    x_SetUpTSEandUnDoManager(m_Workbench);
    if (!m_TopSeqEntry || ! m_CmdProccessor) return;
    
    string prefix;
    switch (event.GetId()){
        case eCmdDeleteStructuredCommentPrefixSuffixMIENSData:
            prefix.assign("MIENS");
            break;
        case eCmdDeleteStructuredCommentPrefixSuffixMIMARKS:
            prefix.assign("MIMARKS");
            break;                    
        case eCmdDeleteStructuredCommentPrefixSuffixMIGS:
            prefix.assign("MIGS");
            break;
        case eCmdDeleteStructuredCommentPrefixSuffixMIMS:
            prefix.assign("MIMS");
            break;     
        case eCmdDeleteStructuredCommentPrefixSuffixGenAssData:
            prefix.assign("Genome-Assembly");
            break;
        case eCmdDeleteStructuredCommentPrefixSuffixGenAnnData:
            prefix.assign("Genome-Annotation");
            break;
        default : break;
    }

    if (prefix.empty())
        return;

    bool found(false);
    CRef<CCmdComposite> cmd(new CCmdComposite("Remove Structured Comment"));
    for ( CSeq_entry_CI entry_it(m_TopSeqEntry, CSeq_entry_CI::fRecursive|CSeq_entry_CI::fIncludeGivenEntry); entry_it; ++entry_it )
    {
        for ( CSeqdesc_CI user_it(*entry_it, CSeqdesc::e_User, 1); user_it; ++user_it)
        {
            const CUser_object& u = user_it->GetUser();
            if (u.IsSetType() && u.GetType().IsStr() && NStr::EqualNocase(u.GetType().GetStr(), "StructuredComment")) 
            {
                try 
                {
                    const CUser_field& prefix_field = user_it->GetUser().GetField("StructuredCommentPrefix");
                    if (prefix_field.IsSetData() && prefix_field.GetData().IsStr() 
                        && NStr::StartsWith(prefix_field.GetData().GetStr(), "##" + prefix, NStr::eNocase) 
                        && NStr::EndsWith(prefix_field.GetData().GetStr(), "-Data-START##", NStr::eNocase))
                    {
                        CIRef<IEditCommand> cmdDelDesc(new CCmdDelDesc(user_it.GetSeq_entry_Handle(), *user_it));
                        cmd->AddCommand(*cmdDelDesc);
                        found = true;
                    }
                } 
                catch (const exception&) { }
            }    
        }
    }
    if (found)
        m_CmdProccessor->Execute(cmd);
}

void CSequenceEditingEventHandler::Help(wxCommandEvent& event)
{
    wxString manual_name;
    switch (event.GetId()) {
    case eCmdHelpReports_ext:
        manual_name = wxT("manual8");
        break;
    case eCmdHelpImport_ext:
        manual_name = wxT("manual11");
        break;
    case eCmdHelpSequences_ext:
        manual_name = wxT("manual9");
        break;
    case eCmdHelpFeatures_ext:
        manual_name = wxT("manual10");
        break;
    case eCmdHelpComments_ext:
        manual_name = wxT("manual12");
        break;
    case eCmdHelpEditingTools_ext:
        manual_name = wxT("manual15");
        break;
    default:
        break;
    }
    
    wxURI help_url(wxT("https://www.ncbi.nlm.nih.gov/tools/gbench/") + manual_name + wxT("/"));
    wxString help_encoded_url = help_url.BuildURI();
    wxLaunchDefaultBrowser(help_encoded_url, 0);
}

END_NCBI_SCOPE

