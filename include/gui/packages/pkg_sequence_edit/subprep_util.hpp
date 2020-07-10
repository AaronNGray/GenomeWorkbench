/*  $Id: subprep_util.hpp 42167 2019-01-08 17:17:20Z filippov $
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
#ifndef _SUBPREP_UTIL_H_
#define _SUBPREP_UTIL_H_

#include <corelib/ncbistd.hpp>
#include <objmgr/seq_entry_handle.hpp>

#include <objects/seqfeat/OrgMod.hpp>
#include <objects/seqfeat/SubSource.hpp>
#include <objects/seqtable/Seq_table.hpp>
#include <objects/seqtable/SeqTable_column.hpp>
#include <objects/seqtable/SeqTable_multi_data.hpp>
#include <objects/seqtable/SeqTable_column_info.hpp>


#include <gui/widgets/wx/wx_utils.hpp>
#include <gui/widgets/grid_widget/grid_widget.hpp>
#include <gui/packages/pkg_sequence_edit/srcedit_util.hpp>
#include <gui/packages/pkg_sequence_edit/seqtable_util.hpp>
#include <gui/packages/pkg_sequence_edit/featedit_util.hpp>
#include <gui/objutils/table_data.hpp>

#include <gui/packages/pkg_sequence_edit/seq_table_grid.hpp>

#include <gui/objutils/cmd_composite.hpp>

BEGIN_NCBI_SCOPE

const string kSubmissionPreparationToolVersion = "1.0";


typedef pair<CSourceRequirements::EWizardType, string> TWizardName;
typedef vector<TWizardName> TWizardNameList;

TWizardNameList GetWizardNameList();
CSourceRequirements::EWizardType GetWizardTypeFromName(string wizard_name);

void FixTableAfterImport (CRef<objects::CSeq_table> input_table);
string FindBadRows (CRef<objects::CSeq_table> src, CRef<objects::CSeq_table> dst, objects::CSeq_entry_Handle seh);
CRef<objects::CSeqTable_column> FindSeqIDColumn(const objects::CSeq_table& table);
int FindRowForSeqId (CRef<objects::CSeqTable_column> id_col, CRef<objects::CSeq_id> id);
int CountColumnRowConflicts (CRef<objects::CSeqTable_column> dst_id, CRef<objects::CSeqTable_column> dst_col,
                                   CRef<objects::CSeqTable_column> src_id, CRef<objects::CSeqTable_column> src_col);
int CountTableColumnConflicts (CRef<objects::CSeq_table> dst, const objects::CSeq_table& src);
void DeleteTableRow (CRef<objects::CSeq_table> table, int row);
int CombineTables (CRef<objects::CSeq_table> dst, const objects::CSeq_table& src);
bool SaveTableFile (wxWindow *parent, wxString& save_file_dir, wxString& save_file_name, CRef<objects::CSeq_table> values_table);

bool AreAllColumnValuesTheSame(CRef<objects::CSeqTable_column> col, string default_val = "");
bool AreAnyColumnValuesMissing(CRef<objects::CSeqTable_column> col);
bool AreAnyColumnValuesPresent(CRef<objects::CSeqTable_column> col, string default_val = "");
bool DoesColumnHaveValue(CRef<objects::CSeqTable_column> col, string val);

bool AddSeqEntryToSeqEntry(const objects::CSeq_entry* entry, objects::CSeq_entry_Handle seh, CRef<CCmdComposite> cmd, bool& has_nuc, bool& has_prot, bool create_general_only);

string CheckFeatureAnnotation (objects::CSeq_entry_Handle entry, bool& is_ok);

int AddFeatureToSeqTable (const objects::CSeq_feat& f, CRef<objects::CSeq_table> table);
CRef<objects::CSeq_table> GetFeaturesFromSeqEntry(objects::CSeq_entry_Handle entry, const objects::CSeq_feat& feat, const TFeatureSeqTableColumnList& reqs);
void MergeStringVectors (vector<string>& problems, vector<string> add);
void RemoveProblemsColumn (CRef<objects::CSeq_table> values_table);
void AddProblemsColumnToFeatureSeqTable(CRef<objects::CSeq_table> table, const TFeatureSeqTableColumnList& reqs, const TFeatureSeqTableColumnList& opts);
CRef<CCmdComposite> AddFeatureSeqTableToSeqEntry(CRef<objects::CSeq_table> table, objects::CSeq_entry_Handle entry, objects::CSeqFeatData::ESubtype subtype, const TFeatureSeqTableColumnList& reqs, const TFeatureSeqTableColumnList& opts);
bool HasLocalIntFeatureId (const objects::CSeq_feat& feat);
CRef<CCmdComposite> RemoveMatchingFeaturesWithoutLocalIntFeatureIdsFromSeqEntry(objects::CSeq_entry_Handle entry, const objects::CSeq_feat &feat, const TFeatureSeqTableColumnList& reqs);
CRef<CFeatureSeqTableColumnBase> GetColumnRuleForFeatureSeqTable(CRef<objects::CSeqTable_column> col, const TFeatureSeqTableColumnList& reqs, const TFeatureSeqTableColumnList& opts);

vector<string> GetTrueFalseList();
bool IsSynonymForTrue(const string& val);
bool IsSynonymForFalse(const string& val);
bool IsTrueFalseList(const vector<string>& choices);

wxString GetAsnSqnExtensions();

const string kWizardLabel = "NCBIWizard";
bool IsWizardObject(const objects::CUser_object& user);
CRef<objects::CUser_object> MakeWizardObject ();
string GetFieldFromWizardObject (const objects::CUser_object& user, string field_name);
void SetWizardFieldInSeqEntryNoUndo(objects::CSeq_entry_Handle entry, string field_name, string value);

CRef<objects::CUser_object> MakeBankItSubmissionObject ();
bool IsBankItSubmissionObject(const objects::CUser_object& user);
string GetDescAlternateEmailAddress(const objects::CSeqdesc& desc);
string GetAlternateEmailAddress(objects::CSeq_entry_Handle entry);
void SetUserAlternateEmailAddress(objects::CUser_object& u, string alt_email);
void SetAlternateEmailAddress(objects::CSeq_entry_Handle entry, string alt_email);
bool IsValidEmail(string email);

CRef<objects::CSeq_table> GetIdsFromSeqEntry(const objects::CSeq_entry& entry);
void AddIdsFromSeqEntryToTable (const objects::CSeq_entry& entry, CRef<objects::CSeq_table> table);
CRef<objects::CSeqTable_column> GetSeqIdProblems(CRef<objects::CSeq_table> new_ids, CRef<objects::CSeq_table> old_ids, size_t max_len);
string SummarizeIdProblems(CRef<objects::CSeqTable_column> problems);
CRef<objects::CSeqTable_column> GetReplacementSeqIds (CRef<objects::CSeq_table> new_ids, CRef<objects::CSeq_table> old_ids, size_t max_len);
void ApplyReplacementIds(objects::CSeq_entry& entry, CRef<objects::CSeq_table> table);
vector<string> FindNonUniqueStrings(const vector<string>& values);
vector<string> SortUniqueStrings(const vector<string>& values);

bool AlreadyHasFeature(objects::CBioseq_Handle bh, string key, string comment);
bool AlreadyHasFeature(objects::CBioseq_Handle bh, objects::CSeqFeatData::ESubtype subtype);



END_NCBI_SCOPE

#endif
    // _SUBPREP_UTIL_H_
