/*  $Id: table_column_type_guesser.cpp 30277 2014-04-23 17:32:59Z falkrb $
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
 * Authors: Bob Falk
 *
 * File Description:
 *
 */

#include <ncbi_pch.hpp>

#include <gui/widgets/loaders/table_column_type_guesser.hpp>

#include <gui/widgets/loaders/table_import_data_source.hpp>
#include <misc/hgvs/sequtils.hpp>

#include <objmgr/object_manager.hpp>
#include <objects/seqloc/Seq_id.hpp>

#include <math.h>


BEGIN_NCBI_SCOPE
USING_SCOPE(objects);

CTableColumnTypeGuesser::CTableColumnTypeGuesser(CRef<CTableImportDataSource> ds) 
    : m_ImportedTableData(ds) 
{
    m_Scope.Reset(new CScope(*CObjectManager::GetInstance()));
    m_Scope->AddDefaults();

}

void CTableColumnTypeGuesser::GuessColumns()
{
    if (m_ImportedTableData.IsNull())
        return;       

    // Guess types of individual columns. First column is line num so we ignore
    for (size_t i=1; i<m_ImportedTableData->GetColumns().size(); ++i) {
        GuessColumn(i);
    }

    // Given individual column types, do guessing based on combinations
    // of columns.  At this point, the only combination we are looking
    // for is start/stop following an ID column.
    for (size_t i=1; i<m_ImportedTableData->GetColumns().size(); ++i) {  
        if (m_ImportedTableData->GetColumn(i).GetType() ==  CTableImportColumn::eSeqIdColumn) {

            // If the current column is a seq-id and the next two are integers,
            // then assume they are start and stop (we could also require that
            // the second column #'s be >= than the numbers in the first column)
            if (m_ImportedTableData->GetColumns().size() >= i+3 &&
                m_ImportedTableData->GetColumn(i+1).GetType() == CTableImportColumn::eNumberColumn &&
                m_ImportedTableData->GetColumn(i+2).GetType() == CTableImportColumn::eNumberColumn) {

                    m_ImportedTableData->GetColumns()[i+1].SetDataType(
                        CTableImportColumn::eStartPosition);
                    m_ImportedTableData->GetColumns()[i+2].SetDataType(
                        CTableImportColumn::eStopPosition);
            }
        }
    }

}

void CTableColumnTypeGuesser::GuessColumn(size_t col_num)
{
    size_t row = 0;

    // Examine rows and for each row guess its type and add that result
    // to the entry in these arrays for type info. (one array for basic type,
    // int, string, etc and one for semantic type, e.g. strand, start position)
    vector<size_t> type_matches((int)CTableImportColumn::eTypeUndefined, 0);
    vector<size_t> property_matches((int)CTableImportColumn::eLastType, 0);

    CTableImportColumn::eColumnType ct;
    CTableImportColumn::eDataType dt;

    // Stop check after 300 rows (that should be enough to pick a type)
    size_t check_count = std::min(m_ImportedTableData->GetNumRows(), (size_t)300);
    for (row=0; row<check_count; ++row) {
        // GetRowNum() == -1 for header rows
        if (m_ImportedTableData->GetRow(row).GetRowNum() != -1) {
            
            // The first column for display purposes is the row number which
            // is not parsed from the data.  This returns a parsed field so
            // we subtract 1 to get the right field.
            string field_value = m_ImportedTableData->GetRow(row).GetField(col_num-1);

            NStr::TruncateSpacesInPlace(field_value);

            ct = x_GuessType(field_value, dt);

            if (ct != CTableImportColumn::eTypeUndefined)
                type_matches[ct] += 1;

            if (dt != CTableImportColumn::eUndefined)
                property_matches[dt] += 1;
        }
    }    

    size_t max_matches = 0;

    // If all fields were blank, unspecified text will be the result
    ct = CTableImportColumn::eSkippedColumn;
    dt = CTableImportColumn::eUndefined;

    bool has_real = false;
    if (type_matches[(size_t)CTableImportColumn::eRealNumberColumn] > 0)
        has_real = true;

    for (size_t j=0; j<type_matches.size(); ++j) {
        if (type_matches[j] > max_matches) {
            max_matches = type_matches[j];
            ct = ( CTableImportColumn::eColumnType)j;
        }
    }

    // if a column has mostly integers but also 1 or more real numbers
    // then we make the column type real (rather than the 'most popular'
    // type of int)
    if (ct == CTableImportColumn::eNumberColumn && has_real)
        ct = CTableImportColumn::eRealNumberColumn;

    m_ImportedTableData->GetColumns()[col_num].SetType(ct);


    max_matches = 0;
    for (size_t j=0; j<property_matches.size(); ++j) {
        if (property_matches[j] > max_matches) {
            max_matches = property_matches[j];
            dt = ( CTableImportColumn::eDataType)j;
        }
    }

    m_ImportedTableData->GetColumns()[col_num].SetDataType(dt);

    // Can use imported column headers, if any, to override the guess
    // made on data analysis alone. For now, we are only checking for 
    // 'chromosome' since it can be missed if a column is just numbers.
    if (m_ImportedTableData->GetColumnHeaderRow() != -1) {
        string colname = m_ImportedTableData->GetColumnName(col_num);
        NStr::ToLower(colname);

        if (colname == "chromosome" && 
            ct == CTableImportColumn::eNumberColumn) {
                m_ImportedTableData->GetColumns()[col_num].
                    SetDataType(CTableImportColumn::eChromosome);
                m_ImportedTableData->GetColumns()[col_num].
                    SetType(CTableImportColumn::eSeqIdColumn);
        }
    }   
}

CTableImportColumn::eColumnType CTableColumnTypeGuesser::x_GuessType(
    const string& field,
    CTableImportColumn::eDataType& dt)
{
    dt = CTableImportColumn::eUndefined;

    // If blank, return undefined so that this result doesn't effect the
    // final choice for a field.  If all entries are blank though,
    // the type should be text.
    if (field.length() == 0)
        return CTableImportColumn::eTypeUndefined;

    if ( (field[0] == '\'' && field[field.length()-1] == '\'') ||
         (field[0] == '\"' && field[field.length()-1] == '\"') ) {
            dt = CTableImportColumn::eUnspecifiedText;
            return CTableImportColumn::eTextColumn;
    }


    bool is_int;
    int val = -1;
    try {
        // Test for integer.  Test StringToUInt8_DataSize only looks for
        // positive integers (unsigned) but we want to allow negative as well.
        string test_field = field;
        if (test_field.length() > 1 && test_field[0] == '-') {
            test_field[0] = ' ';
            NStr::TruncateSpacesInPlace(test_field);
        }
        val = NStr::StringToUInt8_DataSize(test_field, NStr::fDS_ProhibitFractions);
        is_int = true;
    }
    catch ( CStringException&) {
        is_int = false;
    }

    // Is it a chromosome?  You should need an Assmbly for this...
    // Largest known number of chromosomes is adders tounge 1260).
    // Maybe we could look this up/call a library function to check this?

    // M is for mitochondrial (MT)
    size_t first_chr = field.find_first_of("01234567890xymXYM", 0);   

    // Check for chromosome of the form "ch##/chr## etc.
    if (first_chr != string::npos && first_chr >=2) {        
        string prefix = field.substr(0, first_chr);
        string suffix = field.substr(first_chr, field.length()-first_chr);

        int cnum = 0;
        try {
            cnum = NStr::StringToInt(suffix);
        }
        // Not a number - check to see if it's of the form chrX/Y/x/y/MT/mt:
        catch (CStringException&) {
            suffix = NStr::ToUpper(suffix);

            if ((suffix == "X" || suffix == "Y" || suffix == "mt" || suffix == "MT") &&
                (!NStr::CompareNocase(prefix, "chr") ||
                 !NStr::CompareNocase(prefix, "ch") ||
                 !NStr::CompareNocase(prefix, "ch#") ||
                 !NStr::CompareNocase(prefix, "chr#"))) {

                    dt = CTableImportColumn::eChromosome;
                    return CTableImportColumn::eSeqIdColumn;
            }
        }
        
        if ((cnum > 0 && cnum < 1261) &&
            (!NStr::CompareNocase(prefix, "chr") ||
             !NStr::CompareNocase(prefix, "ch") ||
             !NStr::CompareNocase(prefix, "ch#") ||
             !NStr::CompareNocase(prefix, "chr#"))) {
            
                 dt = CTableImportColumn::eChromosome;
                 return CTableImportColumn::eSeqIdColumn;
        }
    }
    /// Chromosomes are also the best guess if the field is one of {x,y,X,Y, mt, MT}
    string field_upper = field;
    NStr::ToUpper(field_upper);
    if (field_upper == "X" || field_upper == "Y" || field_upper == "MT") {
        dt = CTableImportColumn::eChromosome;
        return CTableImportColumn::eSeqIdColumn;
    }

    ////////////////////////////////////////////////
    /// Check if it is an ID
    ////////////////////////////////////////////////

    // First check for special case of GI|xxxxxxxx or lcl|xxxxxxxxxx
    // allow pipe | or colon : to be used as a separator
    // CSeq_id constructor seems to recognize '|' but not ':'
    // so we do a check first here
    size_t gi_separator = field.find_first_of("|:", 0); 
    if (gi_separator != string::npos && 
        gi_separator != field.length()-1) {
            string prefix = field.substr(0, gi_separator);
            prefix = NStr::ToUpper(prefix);

            string ginum = field.substr(gi_separator+1, field.length()-gi_separator);
            
            // For this to be a valid ID, ginum has to be an integer (it probably
            // also needs to be a certain character length...)
            bool ginum_int;
            try {
                val = NStr::StringToUInt8_DataSize(ginum, 0);
                ginum_int = true;
            }
            catch ( CStringException&) {
                ginum_int = false;
            }

            if (ginum_int) {
                // GI
                if (prefix == "GI") {
                    dt = CTableImportColumn::eLocationGI;
                }
                // Local ID
                else {
                    dt = CTableImportColumn::eLocalID;
                }

                return CTableImportColumn::eSeqIdColumn;
            }
    }       

    if (is_int) {
        dt = CTableImportColumn::eUnspecifiedInt;
        return CTableImportColumn::eNumberColumn;
    }  

    // Check for RSID (snp id): RSnnnnn
    if (field.length() > 2) {
        string rsstr = field.substr(0,2);
        rsstr =  NStr::ToUpper(rsstr);
        if (rsstr == "RS") {
            // Is remainder of id an integer?
            string rsint = field.substr(2, field.length()-2);

            try {
                // Test for integer.  Test StringToUInt8_DataSize only looks for
                // positive integers and that's ok since there should be no negative numbers here.
                val = NStr::StringToUInt8_DataSize(rsint, NStr::fDS_ProhibitFractions);

                dt = CTableImportColumn::eRsid;
                return CTableImportColumn::eSeqIdColumn;
            }
            catch ( CStringException&) {
            }
        }
    }


    // Check if it is an ID and, if so, what type of id.  Default is
    // eUnspecifiedID if we can't match it to anything else.
    bool is_id = false;
    CTableImportColumn::eDataType id_type = CTableImportColumn::eUnspecifiedID;
    
    {
        try {
            CSeq_id  seqid(field);

            // if no exception, it's some kind of id:
            is_id = true;            
            CSeq_id_Base::E_Choice w = seqid.Which();

            // In case previous code missed a GI
            if (w == CSeq_id::e_Gi) {               
                id_type = CTableImportColumn::eLocationGI;
            }
            else if (seqid.IsLocal() || seqid.IdentifyAccession() == CSeq_id::eAcc_local) {
                id_type = CTableImportColumn::eLocalID;            
            }
            else {
                // GetSequenceType to check if it is some kind of accession
                Uint4 type_flags = GetSequenceType(m_Scope->GetBioseqHandle(seqid));
                if (type_flags != CSeq_id::eAcc_unknown) {
                    id_type = CTableImportColumn::eLocationID;                   
                }
            }

            //_TRACE("type?: " << CTableImportColumn::GetStringFromDataType(id_type) << 
            //" Which: " << w);
        }
        catch (CException&) {
            is_id = false;
        }
    }

    if (is_id) { 
        // Its an id of some sort:
        dt = id_type;
        return CTableImportColumn::eSeqIdColumn;
    }

    // Check for strand
    if (field == "+" || field == "-" ||
        !NStr::CompareNocase(field, "negative") ||
        !NStr::CompareNocase(field, "positive") ||
        !NStr::CompareNocase(field, "neg") ||
        !NStr::CompareNocase(field, "pos")) {

            dt = CTableImportColumn::eStrand;
            return CTableImportColumn::eTextColumn;
    } 

    // Check for genotype (ACTG or '-' when these are missing. No specific number of characters).
    // Currently we will not consider '-' since if all rows had that, it would be an undefined
    // column. But it would be acceptable for a small % of rows to have dashes.
    if (field_upper.find_first_not_of("ACTG")) {
        dt = CTableImportColumn::eGenotype;
        return CTableImportColumn::eTextColumn;
    } 

    bool is_float;
    double vald = -1;
    try {
        vald = NStr::StringToDouble(field);
        is_float = true;
    }
    catch ( CStringException&) {
        is_float = false;
    }

    if (is_float) {
        dt = CTableImportColumn::eUnspecifiedReal;
        return CTableImportColumn::eRealNumberColumn;
    }

    dt = CTableImportColumn::eUnspecifiedText;
    return CTableImportColumn::eTextColumn;
}


END_NCBI_SCOPE
