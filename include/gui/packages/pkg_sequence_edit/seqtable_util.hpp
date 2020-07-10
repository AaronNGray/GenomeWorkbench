/*  $Id: seqtable_util.hpp 41476 2018-08-02 20:44:57Z asztalos $
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
#ifndef _SEQTABLE_UTIL_H_
#define _SEQTABLE_UTIL_H_

#include <corelib/ncbistd.hpp>
#include <objects/seqtable/Seq_table.hpp>
#include <objects/seqtable/SeqTable_column.hpp>
#include <objects/seqtable/SeqTable_multi_data.hpp>
#include <objects/seqtable/SeqTable_column_info.hpp>
#include <objtools/edit/seqid_guesser.hpp>
#include <gui/widgets/wx/wx_utils.hpp>

BEGIN_NCBI_SCOPE
USING_SCOPE(ncbi::objects);

void RemoveRowsFromColumn(CRef<CSeqTable_column> col, const vector<size_t>& rows);
void RemoveRowsFromTable(CRef<CSeq_table> table, const vector<size_t>& rows);
void RemoveTableRowsThatDoNotMatchStringConstraint(CRef<CSeq_table> table, 
                                                   const string& field, 
                                                   CRef<edit::CStringConstraint> string_constraint);

void ApplyToTable(CRef<CSeq_table> table, const string& field, const string& val, edit::EExistingText existing_text = edit::eExistingText_replace_old);
void RemoveFromTable (CRef<CSeq_table> table, const string& field);
void ConvertTableColumns (CRef<CSeq_table> table, const string& field1, const string& field2, edit::EExistingText existing_text);
void SwapTableColumns (CRef<CSeq_table> table, const string& field1, const string& field2);
void CopyTableColumns (CRef<CSeq_table> table, const string& field1, const string& field2, edit::EExistingText existing_text);

CRef<CSeqTable_column> FindSeqTableColumnByName (CRef<objects::CSeq_table> values_table, string column_name);
bool RemoveSeqTableColumnByName (CRef<objects::CSeq_table> table, string column_name);
void AddValueToColumn(CRef<CSeqTable_column> column, string value, size_t row, edit::EExistingText existing_text = edit::eExistingText_replace_old);
void AddValueToTable(CRef<CSeq_table> table, string subtype_name, string value, size_t row, edit::EExistingText existing_text = edit::eExistingText_replace_old);
void AddValueToTable (CRef<CSeq_table> table, string subtype_name, int value, size_t row);
void SetColumnValue(CRef<CSeqTable_column> col, string val);

CRef<CSeqTable_column> AddStringColumnToTable(CRef<CSeq_table> table, string label);
CRef<CSeqTable_column> AddIntColumnToTable(CRef<CSeq_table> table, string label);
bool QualifierNamesAreEquivalent (string name1, string name2);
bool MatchColumnName (string name1, string name2);
bool IsOrgColumnName (string name);
bool IsSubSourceNoteName (const string& name);
bool IsOrgModNoteName (const string& name);
int CountColumnValueConflicts (CRef<CSeqTable_column> column1, CRef<CSeqTable_column> column2);

void FillShortColumns(CRef<objects::CSeq_table> values_table);


END_NCBI_SCOPE

#endif
    // _SEQTABLE_UTIL_H_
