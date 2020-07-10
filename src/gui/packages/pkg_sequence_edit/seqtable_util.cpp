/*  $Id: seqtable_util.cpp 41476 2018-08-02 20:44:57Z asztalos $
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

#include <objects/seqloc/Seq_id.hpp>
#include <objtools/edit/parse_text_options.hpp>
#include <gui/packages/pkg_sequence_edit/seqtable_util.hpp>


BEGIN_NCBI_SCOPE
USING_SCOPE(ncbi::objects);

// note: rows must have been sorted prior to calling
void RemoveRowsFromColumn(CRef<CSeqTable_column> col, const vector<size_t>& rows)
{
    if (rows.size() == 0) {
        return;
    }
    vector<size_t>::const_iterator row_it = rows.begin();
    
    if (col->GetData().IsString()) {
        size_t orig_pos = 0;
        CSeqTable_column::TData::TString::iterator it = col->SetData().SetString().begin();
        while (row_it != rows.end() && it != col->SetData().SetString().end()) {
            if (orig_pos == *row_it) {
                it = col->SetData().SetString().erase(it);
                row_it++;
            } else {
                it++;
            }
            orig_pos++;
        }
    } else if (col->GetData().IsId()) {
        size_t orig_pos = 0;
        CSeqTable_column::TData::TId::iterator it = col->SetData().SetId().begin();
        while (row_it != rows.end() && it != col->SetData().SetId().end()) {
            if (orig_pos == *row_it) {
                it = col->SetData().SetId().erase(it);
                row_it++;
            } else {
                it++;
            }
            orig_pos++;
        }
    } else if (col->GetData().IsInt()) {
        size_t orig_pos = 0;
        CSeqTable_column::TData::TInt::iterator it = col->SetData().SetInt().begin();
        while (row_it != rows.end() && it != col->SetData().SetInt().end()) {
            if (orig_pos == *row_it) {
                it = col->SetData().SetInt().erase(it);
                row_it++;
            } else {
                it++;
            }
            orig_pos++;
        }
    } else if (col->GetData().IsBit()) {
        size_t orig_pos = 0;
        CSeqTable_column::TData::TBit::iterator it = col->SetData().SetBit().begin();
        while (row_it != rows.end() && it != col->SetData().SetBit().end()) {
            if (orig_pos == *row_it) {
                it = col->SetData().SetBit().erase(it);
                row_it++;
            } else {
                it++;
            }
            orig_pos++;
        }
    } else if (col->GetData().IsBytes()) {
        size_t orig_pos = 0;
        CSeqTable_column::TData::TBytes::iterator it = col->SetData().SetBytes().begin();
        while (row_it != rows.end() && it != col->SetData().SetBytes().end()) {
            if (orig_pos == *row_it) {
                it = col->SetData().SetBytes().erase(it);
                row_it++;
            } else {
                it++;
            }
            orig_pos++;
        }
    } else if (col->GetData().IsReal()) {
        size_t orig_pos = 0;
        CSeqTable_column::TData::TReal::iterator it = col->SetData().SetReal().begin();
        while (row_it != rows.end() && it != col->SetData().SetReal().end()) {
            if (orig_pos == *row_it) {
                it = col->SetData().SetReal().erase(it);
                row_it++;
            } else {
                it++;
            }
            orig_pos++;
        }
    }
}


// note: rows must have been sorted prior to calling
void RemoveRowsFromTable(CRef<CSeq_table> table, const vector<size_t>& rows)
{
    if (rows.size() > 0) {
        NON_CONST_ITERATE(CSeq_table::TColumns, it, table->SetColumns()) {            
            RemoveRowsFromColumn (*it, rows);
        }
        table->SetNum_rows(table->GetNum_rows() - rows.size());
    }
}


void RemoveTableRowsThatDoNotMatchStringConstraint(CRef<CSeq_table> table, 
                                                   const string& field, 
                                                   CRef<edit::CStringConstraint> string_constraint)
{
    CRef<CSeqTable_column> column = FindSeqTableColumnByName (table, field);
    if (!column) {
        return;
    }
    vector<size_t> rows_to_remove;
    if (column->GetData().IsInt()) {
        CSeqTable_column::TData::TInt::const_iterator it = column->GetData().GetInt().begin();
        size_t row = 0;
        while (it != column->GetData().GetInt().end()) {              
            string num = NStr::NumericToString(*it);
            if (!string_constraint->DoesTextMatch(num)) {
                rows_to_remove.push_back(row);
            }
            row++;
            it++;
        }
    } else if (column->GetData().IsString()) {
        CSeqTable_column::TData::TString::const_iterator it = column->GetData().GetString().begin();
        size_t row = 0;
        while (it != column->GetData().GetString().end()) {              
            if (!string_constraint->DoesTextMatch(*it)) {
                rows_to_remove.push_back(row);
            }
            row++;
            it++;
        }
    } else if (column->GetData().IsId()) {
        CSeqTable_column::TData::TId::const_iterator it = column->GetData().GetId().begin();
        size_t row = 0;
        while (it != column->GetData().GetId().end()) {      
            string id_string = (*it)->AsFastaString();
            if (!string_constraint->DoesTextMatch(id_string)) {
                rows_to_remove.push_back(row);
            }
            row++;
            it++;
        }
    }

    RemoveRowsFromTable(table, rows_to_remove);
}


void ApplyToTable(CRef<CSeq_table> table, const string& field, const string& val, edit::EExistingText existing_text)
{
    CRef<CSeqTable_column> column = FindSeqTableColumnByName (table, field);
    if (!column) {
        column = new objects::CSeqTable_column();
        column->SetHeader().SetTitle(field);
        table->SetColumns().push_back(column);
    }
    for (int row = 0; row < table->GetNum_rows(); row++) {
        AddValueToColumn(column, val, row, existing_text);
    }
}


void RemoveFromTable (CRef<CSeq_table> table, const string& field)
{
    CRef<CSeqTable_column> column = FindSeqTableColumnByName (table, field);
    if (column) {
        for (int row = 0; row < table->GetNum_rows(); row++) {
            AddValueToColumn(column, "", row);
        }
    }
}


void ConvertTableColumns(CRef<CSeq_table> table, const string& field1, const string& field2, edit::EExistingText existing_text)
{
    CRef<CSeqTable_column> column1 = FindSeqTableColumnByName (table, field1);
    if (!column1) {
        return;
    }
    CRef<CSeqTable_column> column2 = FindSeqTableColumnByName (table, field2);
    if (!column2) {
        column2 = new objects::CSeqTable_column();
        column2->SetHeader().SetTitle(field2);
        table->SetColumns().push_back(column2);
    }
    for (size_t row = 0; row < column1->GetData().GetString().size(); row++) {
        string orig = column1->GetData().GetString()[row];
        if (!NStr::IsBlank(orig)) {
            AddValueToColumn (column2, orig, row, existing_text);
        }
        column1->SetData().SetString()[row] = "";
    }    
}


void SwapTableColumns (CRef<CSeq_table> table, const string& field1, const string& field2)
{
    CRef<CSeqTable_column> column1 = FindSeqTableColumnByName (table, field1);
    CRef<CSeqTable_column> column2 = FindSeqTableColumnByName (table, field2);
    if (!column1 && !column2) {
        return;
    }
    if (!column1) {
        column1 = new objects::CSeqTable_column();
        column1->SetHeader().SetTitle(field1);
        column1->SetData().SetString();
        table->SetColumns().push_back(column1);
    }
    if (!column2) {
        column2 = new objects::CSeqTable_column();
        column2->SetHeader().SetTitle(field2);
        column2->SetData().SetString();
        table->SetColumns().push_back(column2);
    }
    for (size_t row = 0; row < column1->GetData().GetString().size(); row++) {
        string orig1 = column1->GetData().GetString()[row];
        string orig2 = "";
        if (row < column2->GetData().GetString().size()) {
            orig2 = column2->GetData().GetString()[row];
        }        
        column1->SetData().SetString()[row] = orig2;
        column2->SetData().SetString()[row] = orig1;
    }    
}


void CopyTableColumns(CRef<CSeq_table> table, const string& field1, const string& field2, edit::EExistingText existing_text)
{
    CRef<CSeqTable_column> column1 = FindSeqTableColumnByName (table, field1);
    if (!column1) {
        return;
    }
    CRef<CSeqTable_column> column2 = FindSeqTableColumnByName (table, field2);
    if (!column2) {
        column2 = new objects::CSeqTable_column();
        column2->SetHeader().SetTitle(field2);
        table->SetColumns().push_back(column2);
    }
    for (size_t row = 0; row < column1->GetData().GetString().size(); row++) {
        string orig = column1->GetData().GetString()[row];
        if (!NStr::IsBlank(orig)) {
            AddValueToColumn (column2, orig, row, existing_text);
        }
    }    
}


CRef<objects::CSeqTable_column> FindSeqTableColumnByName (CRef<objects::CSeq_table> values_table, string column_name)
{
    ITERATE (objects::CSeq_table::TColumns, cit, values_table->GetColumns()) {
        if ((*cit)->IsSetHeader() && (*cit)->GetHeader().IsSetTitle()
            && MatchColumnName ((*cit)->GetHeader().GetTitle(), column_name)) {
            return *cit;
        }
    }
    CRef<objects::CSeqTable_column> empty;
    return empty;
}


bool RemoveSeqTableColumnByName (CRef<objects::CSeq_table> table, string column_name)
{
    bool found = false;
    objects::CSeq_table::TColumns::iterator cit = table->SetColumns().begin();
    while (cit != table->SetColumns().end()) {
        if ((*cit)->IsSetHeader() && (*cit)->GetHeader().IsSetTitle()
            && MatchColumnName ((*cit)->GetHeader().GetTitle(), column_name)) {
            cit = table->SetColumns().erase(cit);
            found = true;
        } else {
            ++cit;
        }
    }
    return found;
}


CRef<objects::CSeqTable_column> AddStringColumnToTable(CRef<objects::CSeq_table> table, string label)
{
    CRef<objects::CSeqTable_column> col =  FindSeqTableColumnByName (table, label);
    if (!col) {
        col = new objects::CSeqTable_column();
        col->SetHeader().SetTitle(label);
        col->SetData().SetString();
        table->SetColumns().push_back(col);
    }
    return col;
}


CRef<objects::CSeqTable_column> AddIntColumnToTable(CRef<objects::CSeq_table> table, string label)
{
    CRef<objects::CSeqTable_column> col =  FindSeqTableColumnByName (table, label);
    if (!col) {
        col = new objects::CSeqTable_column();
        col->SetHeader().SetTitle(label);
        col->SetData().SetInt();
        table->SetColumns().push_back(col);
    }
    return col;
}


void AddValueToColumn(CRef<objects::CSeqTable_column> column, string value, size_t row, edit::EExistingText existing_text)
{
    while (column->SetData().SetString().size() < row + 1) {
        column->SetData().SetString().push_back ("");
    }

    string orig_val = column->GetData().GetString()[row];
    edit::AddValueToString(orig_val, value, existing_text);

    column->SetData().SetString()[row] = orig_val;
}


void AddValueToTable(CRef<objects::CSeq_table> table, string subtype_name, string value, size_t row, edit::EExistingText existing_text)
{
    // do we already have a column for this subtype?
    bool found = false;
    NON_CONST_ITERATE (objects::CSeq_table::TColumns, cit, table->SetColumns()) {
        if ((*cit)->IsSetHeader() && (*cit)->GetHeader().IsSetTitle()
            && NStr::EqualNocase((*cit)->GetHeader().GetTitle(), subtype_name)) {
            AddValueToColumn((*cit), value, row, existing_text);
            found = true;
            break;
        }
    }
    if (!found) {
        CRef<objects::CSeqTable_column> new_col(new objects::CSeqTable_column());
        new_col->SetHeader().SetTitle(subtype_name);
        while (new_col->SetData().SetString().size() < row) {
            new_col->SetData().SetString().push_back ("");
        }
        new_col->SetData().SetString().push_back(value);
        table->SetColumns().push_back(new_col);
    }
}


void AddValueToTable (CRef<objects::CSeq_table> table, string subtype_name, int value, size_t row)
{
    // do we already have a column for this subtype?
    bool found = false;
    NON_CONST_ITERATE (objects::CSeq_table::TColumns, cit, table->SetColumns()) {
        if ((*cit)->IsSetHeader() && (*cit)->GetHeader().IsSetTitle()
            && NStr::EqualNocase((*cit)->GetHeader().GetTitle(), subtype_name)) {
            while ((*cit)->SetData().SetInt().size() < row + 1) {
                (*cit)->SetData().SetInt().push_back (0);
            }
            (*cit)->SetData().SetInt()[row] = value;
            found = true;
            break;
        }
    }
    if (!found) {
        CRef<objects::CSeqTable_column> new_col(new objects::CSeqTable_column());
        new_col->SetHeader().SetTitle(subtype_name);
        while (new_col->SetData().SetInt().size() < row) {
            new_col->SetData().SetInt().push_back (0);
        }
        new_col->SetData().SetInt().push_back(value);
        table->SetColumns().push_back(new_col);
    }
}


void SetColumnValue(CRef<objects::CSeqTable_column> col, string val)
{
    if (!col || !col->IsSetData() || !col->GetData().IsString() || col->GetData().GetSize() < 1) {
        return;
    }

    size_t num_rows = col->GetData().GetSize();
    for (size_t row = 0; row < num_rows; row++) {
        col->SetData().SetString()[row] = val;
    }
}


bool QualifierNamesAreEquivalent (string name1, string name2)
{
    // ignore protein at beginning
    const string protein("protein");
    if (NStr::StartsWith(name1, protein)) {
        name1 = name1.substr(protein.length());
    }
    if (NStr::StartsWith(name2, protein)) {
        name2 = name2.substr(protein.length());
    }
    if ((NStr::EqualNocase(name1, "authors") && NStr::EqualNocase(name2, "author name list")) ||
        (NStr::EqualNocase(name2, "authors") && NStr::EqualNocase(name1, "author name list")))
        return true;

    // spaces, dashes, and underscores do not count
    NStr::ReplaceInPlace (name1, " ", "");
    NStr::ReplaceInPlace (name1, "_", "");
    NStr::ReplaceInPlace (name1, "-", "");
    NStr::ReplaceInPlace (name2, " ", "");
    NStr::ReplaceInPlace (name2, "_", "");
    NStr::ReplaceInPlace (name2, "-", "");
    
    return NStr::EqualNocase(name1, name2);
}


bool MatchColumnName (string name1, string name2)
{
    size_t pos = NStr::Find(name1, "\n");
    if (pos != string::npos) {
        name1 = name1.substr(0, pos);
    }
    pos = NStr::Find(name2, "\n");
    if (pos != string::npos) {
        name2 = name2.substr(0, pos);
    }
    if (QualifierNamesAreEquivalent (name1, name2)) {
        return true;
    } else if (IsOrgColumnName(name1) && IsOrgColumnName(name2)) {
        return true;
    } else {
        return false;
    }
}


bool IsOrgColumnName (string name) 
{
    if (NStr::EqualNocase (name, "Organism Name")
        || NStr::EqualNocase (name, "org")
        || NStr::EqualNocase (name, "organism")
        || NStr::EqualNocase (name, "taxname")) {
        return true;
    } else {
        return false;
    }
}


bool IsSubSourceNoteName (const string& name)
{
    if (NStr::EqualNocase(name, "note-subsource") 
           || NStr::EqualNocase(name, "subsource-note")
           || NStr::EqualNocase(name, "subsrc-note")
           || NStr::EqualNocase(name, "note-subsrc")) {
        return true;
    } else {
        return false;
    }
}


bool IsOrgModNoteName (const string& name)
{
    if (NStr::EqualNocase(name, "note-orgmod") 
           || NStr::EqualNocase(name, "orgmod-note")) {
        return true;
    } else {
        return false;
    }
}


// CountColumnValueConflicts
// For two string columns, counts the number of rows for which both columns have a non-blank
// value that does not match
// For two int columns, counts the number of rows for which the values do not match
// For any other type of column, returns -1 (error, cannot compare)

int CountColumnValueConflicts (CRef<CSeqTable_column> orig_col, CRef<CSeqTable_column> new_col)
{
    size_t num_conflicts = 0;

    if (!orig_col || !orig_col->IsSetData() || !new_col || !new_col->IsSetData()) {
        return 0;
    }
    if (orig_col->GetData().IsString() && new_col->GetData().IsString()) {
        CSeqTable_column::TData::TString::const_iterator it1 = orig_col->GetData().GetString().begin();
        CSeqTable_column::TData::TString::const_iterator it2 = new_col->GetData().GetString().begin();
        while (it1 != orig_col->GetData().GetString().end()
                && it2 != new_col->GetData().GetString().end()) {
            if (NStr::IsBlank(*it1) || NStr::IsBlank (*it2)) {
                // ok, ignore
            } else if (NStr::Equal (*it1, *it2)) {
                // no change
            } else {
                num_conflicts++;
            }
            ++it1;
            ++it2;
        }
    } else if (orig_col->GetData().IsInt() && new_col->GetData().IsInt()) {
        CSeqTable_column::TData::TInt::const_iterator it1 = new_col->GetData().GetInt().begin();
        CSeqTable_column::TData::TInt::const_iterator it2 = new_col->GetData().GetInt().begin();
        while (it1 != orig_col->GetData().GetInt().end()
                && it2 != new_col->GetData().GetInt().end()) {
            if (*it1 != *it2) {
                num_conflicts++;
            }
            ++it1;
            ++it2;
        }
    } else {
        return -1;
    }
    return num_conflicts;
}


void FillShortColumns(CRef<objects::CSeq_table> values_table)
{
    if (!values_table || !values_table->IsSetNum_rows()) return;

    size_t num_rows = values_table->GetNum_rows();

    NON_CONST_ITERATE (objects::CSeq_table::TColumns, it, values_table->SetColumns()) {
        if ((*it)->IsSetData() && (*it)->GetData().IsString()) {
            while ((*it)->GetData().GetString().size() < num_rows) {
                (*it)->SetData().SetString().push_back("");
            }
        }
    }
}



END_NCBI_SCOPE
