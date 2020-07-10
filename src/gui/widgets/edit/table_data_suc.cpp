/*  $Id: table_data_suc.cpp 39051 2017-07-21 19:55:56Z katargir $
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
 * Authors: Colleen Bollin
 *
 * File Description:
 *
 */

#include <ncbi_pch.hpp>

#include <gui/widgets/edit/table_data_suc.hpp>

BEGIN_NCBI_SCOPE
USING_SCOPE(objects);

// maximal amount of SUC we should realistically see in a SUC table
const size_t CTableDataSUC::c_MaxRows(100000);

// widths (in chars) of eBlock and eLine columns
// if wider, they will be wrapped
const size_t CTableDataSUC::c_BlockWidth(20);
const size_t CTableDataSUC::c_LineWidth(100);

CTableDataSUC::CTableDataSUC(CSUCResults* results, CScope* scope)
    : m_Scope(scope), m_SUCResults(results)
{
}

CTableDataSUC::~CTableDataSUC()
{
}

// number of entries and order must correspond with CTableDataSUC::EColContents
static const string s_ColNames[] = {
    "Block",
    "Count",
    "Line"
};


ITableData::ColumnType CTableDataSUC::GetColumnType(size_t col) const
{
    if (m_SUCResults->CountLines() == 0)
        return ITableData::kString;

    switch(col){
        case eCount:
            return ITableData::kInt;

        default:
            return ITableData::kString;
    }
    return ITableData::kNone;
}


bool CTableDataSUC::IsHtmlColumn(size_t col) const
{
    return false;
}

string CTableDataSUC::GetColumnLabel(size_t col) const
{
    if (m_SUCResults->CountLines() == 0 && col == 0) {
        return "Status";
    }

    if (col < CTableDataSUC::eMaxColNum) {
        return s_ColNames[col];
	}

    return kEmptyStr;
}


size_t CTableDataSUC::GetRowsCount() const
{
    return max(m_SUCResults->CountLines(), (size_t)1);

}


size_t CTableDataSUC::GetColsCount() const
{
    return (size_t)CTableDataSUC::eMaxColNum;
}


void CTableDataSUC::GetStringValue(size_t row, size_t col, string& value) const
{
    value.clear();

    if (m_SUCResults->CountLines() == 0) {
        if (col == 0) {
            value = "Sort Unique Count complete, no errors found!";
        }
        return;
    }
    
    if(row+1 == c_MaxRows) {
        value ="Warning: Only the first " + NStr::NumericToString(c_MaxRows) + " rows can be shown"; 
        return;
    }
    if (row < GetRowsCount()) {
        CConstRef<CSUCLine> line(x_GetRowData(row));

        switch(col){
        case eBlock:
        {{
                    CFlatFileConfig::FGenbankBlocks block_type = m_SUCResults->GetBlockType(row);
                    value = CSUCBlock::GetBlockLabel(block_type);
                }}
        break;
        
        case eLine:
            value = line->GetLine();
            break;
            
        default:
            break;
        }
    }
    
}

string CTableDataSUC::GetHtmlValue(size_t row, size_t col) const
{
    list<string> values;
    string sDelimiter("<br>");
    if (row < GetRowsCount()) {

        CConstRef<CSUCLine> line(x_GetRowData(row));

		switch(col){

            case eBlock:
                {{
                    CFlatFileConfig::FGenbankBlocks block_type = m_SUCResults->GetBlockType(row);
                    string value = CSUCBlock::GetBlockLabel(block_type);
                    NStr::Wrap(value,
							    c_BlockWidth,
							    values);
                }}
				break;

            case eLine:
				NStr::Wrap(line->GetLine(),
							c_LineWidth,
							values);
                break;

			default:
                break;
        }
    }
    return NStr::Join(values, sDelimiter);
}


long CTableDataSUC::GetIntValue(size_t row, size_t col) const
{
    long value(0);
    if (row < GetRowsCount()) {
        CConstRef<CSUCLine> line(x_GetRowData(row));

        switch(col){
            case eCount:
				value = (long)line->GetCount();
				break;
        }
    }
    return value;
}


SConstScopedObject CTableDataSUC::GetObjectValue(size_t row, size_t col) const
{
    return GetRowObject(row);
}


SConstScopedObject CTableDataSUC::GetRowObject(size_t row) const
{
    SConstScopedObject value;
    if (row < GetRowsCount()) {
        //!! this is incorrect since there may be several objects 
		//!! differently scoped
		value.scope = m_Scope;
        value.object = x_GetRowData(row);
    }
    return value;
}


CConstRef<CSUCLine> CTableDataSUC::x_GetRowData(size_t row) const
{
    return m_SUCResults->GetLine(row);
}

END_NCBI_SCOPE

