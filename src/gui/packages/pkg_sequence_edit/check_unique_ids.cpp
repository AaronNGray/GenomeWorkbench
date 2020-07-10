/*  $Id: check_unique_ids.cpp 43199 2019-05-28 14:05:18Z asztalos $
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
 *  and reliability of the software and data:  the NLM and the U.S.
 *  Government do not and cannot warrant the performance or results that
 *  may be obtained by using this software or data. The NLM and the U.S.
 *  Government disclaim all warranties:  express or implied:  including
 *  warranties of performance:  merchantability or fitness for any particular
 *  purpose.
 *
 *  Please cite the author in any work or product based on this material.
 *
 * ===========================================================================
 *
 * Authors:  Andrea Asztalos
 *
 *  Function for checking unique control IDs across panels that are being used inside 
 *  dialogs and frames in the editing package.
 */


#include <ncbi_pch.hpp>
#include <gui/packages/pkg_sequence_edit/ok_cancel_panel.hpp>
#include <gui/packages/pkg_sequence_edit/parse_text_options_dlg.hpp>
#include <gui/widgets/edit/source_field_name_panel.hpp>
#include <gui/widgets/edit/rna_field_name_panel.hpp>
#include <gui/widgets/edit/cds_gene_prot_field_name_panel.hpp>
#include <gui/packages/pkg_sequence_edit/single_choice_panel.hpp>
#include <gui/widgets/edit/feature_field_name_panel.hpp>
#include <gui/widgets/edit/pub_field_name_panel.hpp>
#include <gui/packages/pkg_sequence_edit/generalid_panel.hpp>
#include <gui/widgets/edit/struct_comm_field_panel.hpp>
#include <gui/widgets/edit/struct_fieldvalue_panel.hpp>
#include <gui/packages/pkg_sequence_edit/dbxref_name_panel.hpp>
#include <gui/packages/pkg_sequence_edit/field_choice_panel.hpp>
#include <gui/packages/pkg_sequence_edit/cap_change_panel.hpp>
#include <gui/widgets/edit/edit_text_options_panel.hpp>
#include <gui/packages/pkg_sequence_edit/remove_text_outside_string_panel.hpp>
#include <gui/packages/pkg_sequence_edit/string_constraint_panel.hpp>
#include <gui/packages/pkg_sequence_edit/field_constraint_panel.hpp>
#include <gui/packages/pkg_sequence_edit/apply_edit_convert_panel.hpp>
#include <gui/packages/pkg_sequence_edit/bulk_location_panel.hpp>
#include <gui/packages/pkg_sequence_edit/edit_feat_loc_panel.hpp>
#include <gui/widgets/edit/submitter_panel.hpp>
#include <gui/packages/pkg_sequence_edit/submission_wizard.hpp>
#include <gui/widgets/edit/submitter_name_panel.hpp>
#include <gui/widgets/edit/submitter_affil_panel.hpp>
#include <gui/widgets/edit/general_panel.hpp>
#include <gui/widgets/edit/gassembly_panel.hpp>
#include <gui/widgets/edit/single_assemblymethod.hpp>
#include <gui/widgets/edit/genomeinfo_panel.hpp>
#include <gui/widgets/edit/gassembly_other_panel.hpp>
#include <gui/packages/pkg_sequence_edit/orginfo_panel.hpp>
#include <gui/widgets/edit/orggeneral_panel.hpp>
#include <gui/widgets/edit/orgadvanced_panel.hpp>
#include <gui/widgets/edit/single_sequence_panel.hpp>
#include <gui/widgets/edit/seqsub_plasmid_panel.hpp>
#include <gui/widgets/edit/seqsub_chrm_panel.hpp>
#include <gui/widgets/edit/seqsub_molinfo_panel.hpp>
#include <gui/packages/pkg_sequence_edit/subannotation_panel.hpp>
#include <gui/widgets/edit/reference_panel.hpp>
#include <gui/widgets/edit/pubauthor_panel.hpp>
#include <gui/widgets/edit/pubstatus_panel.hpp>
#include <gui/widgets/edit/unpublishedref_panel.hpp>
#include <gui/widgets/edit/inpressref_panel.hpp>
#include <gui/widgets/edit/publishedref_panel.hpp>
#include <gui/packages/pkg_sequence_edit/subvalidate_panel.hpp>
#include "update_seq_panel.hpp"


BEGIN_NCBI_SCOPE

static void s_CheckForUniqueIdsInPanels()
{
    int c = 0;
    switch (c)
    {
    case COkCancelPanel::ID_COKCANCELPANEL:
    case COkCancelPanel::ID_ACCEPT_BTN:
    case COkCancelPanel::ID_LEAVE_DIALOG_UP_CHKBOX:
    case CParseTextOptionsDlg::ID_CPARSETEXTOPTIONSDLG:
    case CParseTextOptionsDlg::ID_CPARSEOPTS_JUST_AFTER_BTN:
    case CParseTextOptionsDlg::ID_CPARSEOPTS_RADIOBUTTON5:
    case CParseTextOptionsDlg::ID_CPARSEOPTS_START_TEXT_BTN:
    case CParseTextOptionsDlg::ID_CPARSEOPTS_START_TEXT_TXT:
    case CParseTextOptionsDlg::ID_CPARSEOPTS_START_DIGITS_BTN:
    case CParseTextOptionsDlg::ID_CPARSEOPTS_START_LETTERS_BTN:
    case CParseTextOptionsDlg::ID_CPARSEOPTS_ALSO_RMV_BEFORE_PARSE:
    case CParseTextOptionsDlg::ID_CPARSEOPTS_UP_TO_BTN:
    case CParseTextOptionsDlg::ID_CPARSEOPTS_INCLUDING_BTN:
    case CParseTextOptionsDlg::ID_CPARSEOPTS_END_TEXT_BTN:
    case CParseTextOptionsDlg::ID_CPARSEOPTS_END_TEXT_TXT:
    case CParseTextOptionsDlg::ID_CPARSEOPTS_END_DIGITS_BTN:
    case CParseTextOptionsDlg::ID_CPARSEOPTS_END_LETTERS_BTN:
    case CParseTextOptionsDlg::ID_CPARSEOPTS_ALSO_RMV_AFTER_PARSE:
    case CParseTextOptionsDlg::ID_CPARSEOPTS_REMOVE_FROM_PARSED_BTN:
    case CParseTextOptionsDlg::ID_CPARSEOPTS_CASE_INSENS_PARSE:
    case CParseTextOptionsDlg::ID_CPARSEOPTS_WHOLE_WORD_PARSE:
    case CSourceFieldNamePanel::ID_CSOURCEFIELDNAMEPANEL:
    case CSourceFieldNamePanel::ID_CSOURCEFIELD_TEXT_QUAL_BTN:
    case CSourceFieldNamePanel::ID_CSOURCEFIELD_TAXONOMY_BTN:
    case CSourceFieldNamePanel::ID_CSOURCEFIELD_LOCATION_BTN:
    case CSourceFieldNamePanel::ID_CSOURCEFIELD_ORIGIN_BTN:
    case CSourceFieldNamePanel::ID_CSOURCEFIELD_SRC_SUBLIST:
    case CSourceFieldNamePanel::ID_CSOURCEFIELD_SRCTYPES:
    case CRNAFieldNamePanel::ID_CRNAFIELDNAMEPANEL:
    case CRNAFieldNamePanel::ID_CRNAFIELDNAME_RNA_TYPE:
    case CRNAFieldNamePanel::ID_CRNAFIELDNAME_NCRNA_CLASS:
    case CRNAFieldNamePanel::ID_CRNAFIELDNAME_RNA_FIELD:
    case CCDSGeneProtFieldNamePanel::ID_CCDSGENEPROTFIELDNAMEPANEL:
    case CCDSGeneProtFieldNamePanel::ID_CCDSGENEPROTFIELDNAME_FIELD:
    case CSingleChoicePanel::ID_CSINGLECHOICEPANEL:
    case CSingleChoicePanel::ID_CSINGLECHOICE_FIELD:
    case CFeatureFieldNamePanel::ID_CFEATUREFIELDNAMEPANEL:
    case CFeatureFieldNamePanel::ID_FEATQUALIFIER_LISTBOX:
    case CFieldHandlerNamePanel::ID_PUBFIELDNAMEPANEL:
    case CFieldHandlerNamePanel::ID_PUBFIELDNAME_FIELD:
    case CGeneralIDPanel::ID_CGENERALIDPANEL:
    case CGeneralIDPanel::ID_CGENERALID_CHOICE:
    case CGeneralIDPanel::ID_CGENERALID_TEXT:
    case CStructCommentFieldPanel::ID_CSTRUCTCOMMENTFIELDPANEL:
    case CStructCommentFieldPanel::ID_STRCMNT_CHOICE:
    case CStructCommentFieldPanel::ID_STRCMNT_TXTCTRL:
    case CStructFieldValuePanel::ID_CSTRUCTFIELDVALUEPANEL:
    case CStructFieldValuePanel::ID_STRCMNTFLD_CHOICE:
    case CDbxrefNamePanel::ID_CDBXREFNAMEPANEL:
    case CDbxrefNamePanel::ID_TXTCTRl_DBXREF:
    case CFieldChoicePanel::ID_CFIELDCHOICEPANEL:
    case CFieldChoicePanel::ID_CFIELD_TYPE_CHOICE:
    case CCapChangePanel::ID_CCAPCHANGEPANEL:
    case CCapChangePanel::ID_CAPCHANGE_NOCHANGE:
    case CCapChangePanel::ID_CAPCHANGE_UPPER:
    case CCapChangePanel::ID_CAPCHANGE_FCAP_NOCH:
    case CCapChangePanel::ID_CAPCHANGE_CAPWORDS_SP:
    case CCapChangePanel::ID_CAPCHANGE_LOWER:
    case CCapChangePanel::ID_CAPCHANGE_FCAP_LOW:
    case CCapChangePanel::ID_CAPCHANGE_FLOW_NOCH:
    case CCapChangePanel::ID_CAPCHANGE_CAPWORDS_SPPUNCT:
    case CEditTextOptionsPanel::ID_CEDITTEXTOPTIONSPANEL:
    case CEditTextOptionsPanel::ID_EDITTEXTOPTIONS_FIND:
    case CEditTextOptionsPanel::ID_EDITTEXTOPTIONS_REPLACE:
    case CEditTextOptionsPanel::ID_EDITTEXTOPTIONS_COPY:
    case CEditTextOptionsPanel::ID_EDITTEXTOPTIONS_ANYWHERE:
    case CEditTextOptionsPanel::ID_EDITTEXTOPTIONS_BEGINNING:
    case CEditTextOptionsPanel::ID_EDITTEXTOPTIONS_END:
    case CEditTextOptionsPanel::ID_EDITTEXTOPTIONS_CASE:
    case CRemoveTextOutsideStringPanel::ID_CREMOVETEXTOUTSIDESTRINGPANEL:
    case CRemoveTextOutsideStringPanel::ID_BEFORE_CHECKBOX:
    case CRemoveTextOutsideStringPanel::ID_BEFORE_TEXT_BTN:
    case CRemoveTextOutsideStringPanel::ID_BEFORE_TEXT_TXT:
    case CRemoveTextOutsideStringPanel::ID_BEFORE_DIGITS_BTN:
    case CRemoveTextOutsideStringPanel::ID_BEFORE_LETTERS_BTN:
    case CRemoveTextOutsideStringPanel::ID_ALSO_REMOVE_BEFORE:
    case CRemoveTextOutsideStringPanel::ID_AFTER_CHECKBOX:
    case CRemoveTextOutsideStringPanel::ID_AFTER_TEXT_BTN:
    case CRemoveTextOutsideStringPanel::ID_AFTER_TEXT_TXT:
    case CRemoveTextOutsideStringPanel::ID_AFTER_DIGITS_BTN:
    case CRemoveTextOutsideStringPanel::ID_AFTER_LETTERS_BTN:
    case CRemoveTextOutsideStringPanel::ID_ALSO_REMOVE_AFTER:
    case CRemoveTextOutsideStringPanel::ID_CASE_BTN:
    case CRemoveTextOutsideStringPanel::ID_WHOLE_WORD_BTN:
    case CStringConstraintPanel::ID_CSTRINGCONSTRAINTPANEL:
    case CStringConstraintPanel::ID_STRING_CONSTRAINT_MATCH_CHOICE:
    case CStringConstraintPanel::ID_STRING_CONSTRAINT_MATCH_TEXT:
    case CStringConstraintPanel::ID_STRING_CONSTRAINT_CHECKBOX5:
    case CStringConstraintPanel::ID_STRING_CONSTRAINT_CHECKBOX6:
    case CStringConstraintPanel::ID_STRING_CONSTRAINT_CLEAR_BUTTON:
    case CFieldConstraintPanel::ID_CFIELDCONSTRAINTPANEL:
    case CFieldConstraintPanel::ID_CFIELDCONSTRAINTPANEL_FIELD:
    case CFieldConstraintPanel::ID_CFIELDCONSTRAINTPANEL_CONSTRAINT:
    case CApplyEditconvertPanel::ID_CAPPLYEDITCONVERTPANEL:
    case CApplyEditconvertPanel::ID_AECR_CHOICE:
    case CApplyEditconvertPanel::ID_AECR_LISTBOX:
    case CApplyEditconvertPanel::ID_AECR_BUTTON:
    case CApplyEditconvertPanel::ID_AECR_BUTTON1:
    case CApplyEditconvertPanel::ID_AECR_BUTTON2:
    case CApplySubpanel::ID_APPLYSUB_TEXTCTRL:
    case CApplySubpanel::ID_APPLYSUB_AUTOPOPULATE:
    case CApplySubpanel::ID_APPLYSUB_CLEAR:
    case CEditSubpanel::ID_EDITSUB_FIND:
    case CEditSubpanel::ID_EDITSUB_REPLACE:
    case CEditSubpanel::ID_EDITSUB_COPY:
    case CEditSubpanel::ID_EDITSUB_ANY:
    case CEditSubpanel::ID_EDITSUB_BEG:
    case CEditSubpanel::ID_EDITSUB_END:
    case CEditSubpanel::ID_EDITSUB_AUTOPOPULATE:
    case CEditSubpanel::ID_EDITSUB_CLEAR:
    case CBulkLocationPanel::ID_CBULKLOCATIONPANEL:
    case CBulkLocationPanel::ID_BULKLOC_CHCKBOX1:
    case CBulkLocationPanel::ID_BULKLOC_CHCKBOX2:
    case CBulkLocationPanel::ID_BULKLOC_RBOX:
    case CBulkLocationPanel::ID_BULKLOC_RBTN1:
    case CBulkLocationPanel::ID_BULKLOC_RBTN2:
    case CBulkLocationPanel::ID_BULKLOC_TEXTCTRL1:
    case CBulkLocationPanel::ID_BULKLOC_TEXTCTRL2:
    case CBulkLocationPanel::ID_BULKLOC_CHCKBOX3:
    case CEditFeatLocPanel::ID_CEDITFEATLOCPANEL:
    case CEditFeatLocPanel::ID_EFL_PARTIAL5:
    case CEditFeatLocPanel::ID_EFL_EXTEND5:
    case CEditFeatLocPanel::ID_EFL_PARTIAL3:
    case CEditFeatLocPanel::ID_EFL_EXTEND3:
    case CEditFeatLocPanel::ID_EFL_JOM:
    case CEditFeatLocPanel::ID_EFL_RETRANSLATE:
    case CEditFeatLocPanel::ID_EFL_ADJUST_GENE:
    case CUpdateSeqPanel::ID_CUPDATESEQPANEL:
    case CUpdateSeqPanel::ID_SEQUPD_TOOLBAR:
    case CUpdateSeqPanel::ID_SEQUPD_BTN1:
    case CUpdateSeqPanel::ID_SEQUPD_BTN2:
    case CUpdateSeqPanel::ID_SEQUPD_BTN3:
    case CUpdateSeqPanel::ID_SEQUPD_BTN4:
    case CUpdateSeqPanel::ID_SEQUPD_BTN5:
    case CUpdateSeqPanel::ID_SEQUPD_IALN_CHK:
    case CUpdateSeqPanel::ID_SEQUPD_IMPCHK:
    case CUpdateSeqPanel::ID_SEQUPD_FEATPNL:
    case CUpdateSeqPanel::ID_SEQUPD_IMPFEATUPD_BTN1:
    case CUpdateSeqPanel::ID_SEQUPD_IMPFEATUPD_BTN2:
    case CUpdateSeqPanel::ID_SEQUPD_IMPFEATUPD_BTN3:
    case CUpdateSeqPanel::ID_SEQUPD_IMPFEATUPD_BTN4:
    case CUpdateSeqPanel::ID_SEQUPD_DELFEAT_BTN1:
    case CUpdateSeqPanel::ID_SEQUPD_DELFEAT_BTN2:
    case CUpdateSeqPanel::ID_SEQUPD_DELFEAT_BTN3:
    case CUpdateSeqPanel::ID_SEQUPD_DELFEAT_BTN4:
    case CUpdateSeqPanel::ID_SEQUPD_UPDPROTEIN:
    case CUpdateSeqPanel::ID_SEQUPD_KEEPPROTID:
    case CUpdateSeqPanel::ID_SEQUPD_ADDCITSUB:
    case CUpdateSeqPanel::ID_SEQUPD_LOADTEXT:
    case CUpdateSeqPanel::ID_SEQUPD_PROGRESSBAR:
    case CSubmitterPanel::ID_CSUBMITTERPANEL:
    case CSubmitterPanel::ID_CSUBMITTERNOTEBOOK:
    case CSubmissionWizard::ID_CSUBMISSIONWIZARD:
    case CSubmissionWizard::ID_CSUBMISSIONNOTEBOOK:
    case CSubmissionWizard::ID_CSUBMISSIONBACKBTN:
    case CSubmissionWizard::ID_CSUBMISSIONCONTBTN:
    case CSubmitterNamePanel::ID_CSUBMITTERNAMEPANEL:
    case CSubmitterNamePanel::ID_SUBMITTERFIRSTNAME:
    case CSubmitterNamePanel::ID_SUBMITTERMIDDLENAME:
    case CSubmitterNamePanel::ID_SUBMITTERLASTNAME:
    case CSubmitterNamePanel::ID_SUBMITTER_PRIMEMAIL:
    case CSubmitterNamePanel::ID_SUBMITTER_SECEMAIL:
    case CSubmissionWizard::ID_SUBMITTER_IMPORT:
    case CSubmissionWizard::ID_SUBMITTER_EXPORT:
    case CSubmitterAffilPanel::ID_SUBMITTERAFFILPANEL:
    case CSubmitterAffilPanel::ID_SUB_AFFIL_INST:
    case CSubmitterAffilPanel::ID_SUB_AFFIL_DEPT:
    case CSubmitterAffilPanel::ID_SUB_AFFIL_ADDRESS:
    case CSubmitterAffilPanel::ID_SUB_AFFIL_CITY:
    case CSubmitterAffilPanel::ID_SUB_AFFIL_CODE:
    case CSubmitterAffilPanel::ID_SUB_AFFIL_COUNTRY:
    case CSubmitterAffilPanel::ID_SUB_AFFIL_STATE:
    case CSubmitterAffilPanel::ID_SUB_AFFIL_PHONE:
    case CGeneralPanel::ID_CGENERALPANEL:
    case CGeneralPanel::ID_SUBMITTER_BIOPROJECT:
    case CGeneralPanel::ID_SUBMITTER_BIOSAMPLE:
    case CGeneralPanel::ID_SUBMITTER_IMMEDIATE:
    case CGeneralPanel::ID_SUBMITTER_HUP:
    case CGAssemblyPanel::ID_CGASSEMBLYPANEL:
    case CGAssemblyPanel::ID_GAASSEMBLYDATE:
    case CGAssemblyPanel::ID_GAASSEMBLYNAME:
    case CGAssemblyPanel::ID_GASCROLLEDWND:
    case CGAssemblyPanel::ID_GAASSEMBLYHYPERLINK:
    case CSingleAssemblyMethod::ID_CSINGLEASSEMBLYMETHOD:
    case CSingleAssemblyMethod::ID_GASSEMBLYMETHOD:
    case CSingleAssemblyMethod::ID_GASSEMBLYVERSION:
    case CGenomeInfoPanel::ID_CGENOMEINFOPANEL:
    case CGenomeInfoPanel::ID_GENOMENOTEBOOK:
    case CGAssemblyOtherPanel::ID_CGASSEMBLYOTHERPANEL:
    case CGAssemblyOtherPanel::ID_GENOMECOVERAGE:
    case CGAssemblyOtherPanel::ID_SEQTECHSCROLLEDWND:
    case CGAssemblyOtherPanel::ID_SEQTECHHYPERLINK:
    case CGAssemblyOtherPanel::ID_SEQTECHCOMBOBOX:
    case COrganismInfoPanel::ID_CORGANISMINFOPANEL:
    case COrganismInfoPanel::ID_ORGINFOBTN:
    case COrganismInfoPanel::ID_ORGINFONTBK:
    case COrgGeneralPanel::ID_CORGGENERALPANEL:
    case COrgGeneralPanel::ID_ORGTAXNAME:
    case COrgGeneralPanel::ID_ORGSTRAIN:
    case COrgGeneralPanel::ID_ORGISOLATE:
    case COrgGeneralPanel::ID_ORGCULTIVAR:
    case COrgGeneralPanel::ID_ORGBREED:
    case COrgAdvancedPanel::ID_CORGADVANCEDPANEL:
    case COrgAdvancedPanel::ID_ORGADVSCROLLEDWND:
    case COrgAdvancedPanel::ID_ORGADVHYPERLINK:
    case COrgAdvancedPanel::ID_ORGADVDELETE:
    case CSingleSequencePanel::ID_CSINGLESEQUENCEPANEL:
    case CSingleSequencePanel::ID_SEQNAMETEXTCTRL:
    case CSingleSequencePanel::ID_SINGLESEQID:
    case CSingleSequencePanel::ID_SEQCIRCULARCHK:
    case CSingleSequencePanel::ID_SEQCOMPLETECHK:
    case CSeqSubPlasmidPanel::ID_CSEQSUBPLASMIDPANEL:
    case CSeqSubPlasmidPanel::ID_MOLPLASMIDYES:
    case CSeqSubPlasmidPanel::ID_MOLPLASMIDNO:
    case CSeqSubPlasmidPanel::ID_MOLPLASMIDSCROLLEDWND:
    case CSeqSubPlasmidPanel::ID_MOLADDPLASMID:
    case CSeqSubPlasmidPanel::ID_MOLDELPLASMID:
    case CSeqSubChromoPanel::ID_CSEQSUBCHROMOPANEL:
    case CSeqSubChromoPanel::ID_MOLCHROMOSOMENO:
    case CSeqSubChromoPanel::ID_MOLCHROMOSCROLLEDWND:
    case CSeqSubChromoPanel::ID_MOLADDCHROMOSOME:
    case CSeqSubChromoPanel::ID_MOLDELCHROMOSOME:
    case CSubMolinfoPanel::ID_CSUBMOLINFOPANEL:
    case CSubMolinfoPanel::ID_MOLINFONOTEBOOK:
    case CSubAnnotationPanel::ID_CSUBANNOTATIONPANEL:
    case CSubAnnotationPanel::ID_IMPORT_FTABLE_BTN:
    case CSubAnnotationPanel::ID_SUBANNOTFEATTBL:
    case CReferencePanel::ID_CREFERENCEPANEL:
    case CReferencePanel::ID_REFERENCELISTBOOK:
    case CPubAuthorPanel::ID_CPUBAUTHORPANEL:
    case CPubAuthorPanel::ID_REFAUTHORSCROLLEDWND:
    case CPubAuthorPanel::ID_MOLADDCHROMOSOME:
    case CPubAuthorPanel::ID_MOLDELCHROMOSOME:
    case CPubStatusPanel::ID_CPUBSTATUSPANEL:
    case CPubStatusPanel::ID_UNPUBLISHEDBTN:
    case CPubStatusPanel::ID_INPRESSBTN:
    case CPubStatusPanel::ID_PUBLISHEDBTN:
    case CPubStatusPanel::ID_REFSEQAUTHORSBTN:
    case CPubStatusPanel::ID_NEWAUTHORSBTN:
    case CUnpublishedRefPanel::ID_CUNPUBLISHEDREFPANEL:
    case CUnpublishedRefPanel::ID_REFTITLETXTCTRL:
    case CInPressRefPanel::ID_CINPRESSREFPANEL:
    case CInPressRefPanel::ID_PMIDTEXTCTRL:
    case CInPressRefPanel::ID_PMIDLOOKBTN:
    case CInPressRefPanel::ID_INPREFTITLE:
    case CInPressRefPanel::ID_INPJOURNAL:
    case CInPressRefPanel::ID_INPYEAR:
    case CInPressRefPanel::ID_INPVOL:
    case CInPressRefPanel::ID_INPISSUE:
    case CInPressRefPanel::ID_INPPAGESFROM:
    case CInPressRefPanel::ID_INPPAGESTO:
    case CPublishedRefPanel::ID_CPUBLISHEDREFPANEL:
    case CPublishedRefPanel::ID_LOOKUPCHOICE:
    case CPublishedRefPanel::ID_PUBPMIDTEXTCTRL:
    case CPublishedRefPanel::ID_PUBPMIDLOOKBTN:
    case CPublishedRefPanel::ID_PUBREFTITLE:
    case CPublishedRefPanel::ID_PUBJOURNAL:
    case CPublishedRefPanel::ID_PUBYEAR:
    case CPublishedRefPanel::ID_PUBVOL:
    case CPublishedRefPanel::ID_PUBISSUE:
    case CPublishedRefPanel::ID_PUBPAGESFROM:
    case CPublishedRefPanel::ID_PUBPAGESTO:
    case CSubValidatePanel::ID_CSUBVALIDATEPANEL:
    case CSubValidatePanel::ID_CSUBVALIDATE_REFRESH:

        break;
    }
}


END_NCBI_SCOPE


