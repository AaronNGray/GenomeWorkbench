#ifndef GUI_OBJUTILS___MACRO_PANEL_ARGS__HPP
#define GUI_OBJUTILS___MACRO_PANEL_ARGS__HPP
/*  $Id: macro_panel_args.hpp 44820 2020-03-23 14:59:16Z asztalos $
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
 *  and reliability of the software and data, the NLM and the U.S.
 *  Government do not and cannot warrant the performance or results that
 *  may be obtained by using this software or data. The NLM and the U.S.
 *  Government disclaim all warranties, express or implied, including
 *  warranties of performance, merchantability or fitness for any particular
 *  purpose.
 *
 *  Please cite the author in any work or product based on this material.
 *
 * ===========================================================================
 *
 * Authors: Andrea Asztalos
 *
 * File Description: Control names defined in the panels from the macro editor
 *
 */

#include <corelib/ncbistd.hpp>
#include <gui/objutils/macro_edit_fn_base.hpp>
#include <gui/widgets/edit/macro_argument.hpp>
#include <gui/gui_export.h>


BEGIN_NCBI_SCOPE

namespace NMacroArgs
{
    const string kBsrcTextFrom = "text_qualifier_from";
    const string kBsrcTaxFrom = "taxonomy_from";
    const string kBsrcTextTo = "text_qualifier_to";
    const string kBsrcTaxTo = "taxonomy_to";

    const string kField = "field";
    const string kNewValue = "new_value";
    const string kExistingText = "existing_text";
    const string kDelimiter = "delimiter";

    const string kTableEnable = "enable_table";
    const string kTableName = "table";
    const string kTableCol = "table_column";
    const string kOpenFile = "file_open_btn";
    const string kTableLabel = "filename_text";
    const string kColLabel = "column_text";

    const TArgumentsVector& GetSetArgs();

    const string kFind = "find_text";
    const string kRepl = "repl_text";
    const string kLocation = "location";
    const string kCaseSens = "case_sensitive";
    const string kCopy = "copy_btn";
    const string kIsRegex = "is_regex";

    const TArgumentsVector& GetEditArgs();

    const string kFromField = "from_field";
    const string kToField = "to_field";

    const TArgumentsVector& GetSwapArgs();

    const TArgumentsVector& GetCopyArgs();

    const string kLeaveOrig = "leave_original";
    const string kNoChange = "no_change";
    const string kToUpper = "to_upper";
    const string kCapNochange = "fcap_nochange";
    const string kCapAtSpaces = "cap_atspaces";
    const string kToLower = "to_lower";
    const string kFCapLower = "fcap_lower";
    const string kFlowerNochange = "flower_nochange";
    const string kCapAtSpacesPunct = "cap_atspaces_punct";

    const TArgumentsVector& GetConvertArgs();

    const string kTextLeft = "text_left";
    const string kDelLeft = "delimiter_left";
    const string kDigitsLeft = "digits_left";
    const string kLetterLeft = "letters_left";
    const string kRmvLeft = "rmv_left";
    const string kTextRight = "text_right";
    const string kDelRight = "delimiter_right";
    const string kDigitsRight = "digits_right";
    const string kLetterRight = "letters_right";
    const string kRmvRight = "rmv_right";

    const TArgumentsVector& GetDelimiterArgs();

    const string kJustAfter = "just_after";
    const string kStartAt = "starting_at";
    const string kUpTo = "up_to";
    const string kInclude = "including";
    const string kRmvParse = "rmv_from_parsed";
    const string kCaseInsensParse = "case_insensitive_parse";
    const string kWholeWordParse = "whole_word_parse";

    const TArgumentsVector& GetParseArgs();

    const string kBefore = "before_check";
    const string kAfter = "after_check";
    const string kCaseInsensRmv = "case_insensitive_rmv";
    const string kWholeWordRmv = "whole_word_rmv";

    const TArgumentsVector& GetRmvOutsideArgs();

    const string kFeatType = "feature_field";
    const string kFeatQual = "qualifier_field";
    const string kUpdatemRNA = "update_mrna";
    const string kRNAType = "rna_type";
    const string kncRNAClass = "ncRNA_class";
    const string kStrCommField = "strcomm_field";
    const string kMoveToNote = "move_to_note";

    const TArgumentsVector& GetOptionalArgs();

    const string kBsrcText = "text_qualifier";
    const string kBsrcTax = "taxonomy";
    const string kBsrcLoc = "bsrclocation";
    const string kBsrcOrigin = "origin";
    const TArgumentsVector& GetBsrcArgs();

    const TArgumentsVector& GetDualBsrcArgs();

    const string kMolFrom = "mol_from";
    const string kMolTo = "mol_to";
    const string kTechFrom = "tech_from";
    const string kTechTo = "tech_to";
    const string kComplFrom = "compl_from";
    const string kComplTo = "compl_to";
    const string kClassFrom = "class_from";
    const string kClassTo = "class_to";
    const string kTopFrom = "topology_from";
    const string kTopTo = "topology_to";
    const string kStrandFrom = "strand_from";
    const string kStrandTo = "strand_to";
    const string kApplySeq = "apply_seq";

    const TArgumentsVector& GetEditMolinfoArgs();

    const string kInconsTaxa = "inconsistent_taxa";

    const TArgumentsVector& GetConvertSetArgs();

    const string kFromFeat = "from_feature";
    const string kToFeat = "to_feature";
    const string kConvText = "conversion_descr";
    const string kLeaveFeat = "leave_original";
    const string kRemmRNA = "rem_mrna";
    const string kRemGene = "rem_gene";
    const string kRemTranscriptid = "rem_transcriptID";
    const string kSelText = "selection_text";
    const string kChoices = "choices";
    const string kCreateOnProtein = "create_on_protein";

    const TArgumentsVector& GetConvertFeatArgs();

    const string kCapChange = "cap_change";

    const string kExceptionType = "exception_type";
    const string kWhereFeatProduct = "where_feature_product";
    const string kMoveExplanation = "move_explanation";

    const TArgumentsVector& GetSetExceptionArgs();

    const string kSuppressingType = "suppressing_type";
    const string kNecessaryType = "necessary_type";

    const TArgumentsVector& GetRemoveGeneXrefArgs();

    const string kCheckPartials = "check_partials";
    const string kRemoveProteins = "remove_proteins";

    const TArgumentsVector& GetRmvDuplFeaturesArgs();

    const string kDelImproper = "del_improper";
    const string kDelUnrecog = "del_unrecog";
    const string kDelMultRepl = "del_mult_repl";

    const TArgumentsVector& GetUpdateRpldECNumbersArgs();

    const string kObeyStopCodon = "obey_stop_codon";

    const string kRmvOverlapGene = "rmv_overlapping_gene";

    const string kRmvBlank = "remove_blank";
    
    const TArgumentsVector& GetApplyTableArgs();

    const string kGapSizeCnv = "convert_gap_size";

    const string kAllDbxref = "all_dbxref";
    const string kIllegalDbxref = "illegal_dbxref";

    const TArgumentsVector& GetRmvDbxrefArgs();
} 

END_NCBI_SCOPE

#endif  // GUI_OBJUTILS___MACRO_PANEL_ARGS__HPP
