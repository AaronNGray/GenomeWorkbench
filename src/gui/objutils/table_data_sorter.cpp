/*  $Id: table_data_sorter.cpp 34591 2016-01-28 17:03:41Z katargir $
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
 * Authors: Roman Katargin
 *
 * File Description:
 *
 */

#include <ncbi_pch.hpp>

#include <gui/objutils/table_data_sorter.hpp>

BEGIN_NCBI_SCOPE

void CTableDataSorter::Sort(ITableData& tableData, size_t column,
                            size_t* begin, size_t* end, bool descending)
{
    if (column >= tableData.GetColsCount())
        return;

    ITableData::ColumnType type = tableData.GetColumnType(column);
    if (type == ITableData::kInt) {
        sort(begin, end, [&tableData, column, descending](size_t r1, size_t r2) -> bool
        { auto v1 = tableData.GetIntValue(r1, column), v2 = tableData.GetIntValue(r2, column);
          return descending ? (v1 > v2) : (v1 < v2); });
    }
    else if (type == ITableData::kReal) {
        sort(begin, end, [&tableData, column, descending](size_t r1, size_t r2) -> bool
        { auto v1 = tableData.GetRealValue(r1, column), v2 = tableData.GetRealValue(r2, column);
          return descending ? (v1 > v2) : (v1 < v2); });
    }
    else {
        string s1, s2;
        sort(begin, end, [&tableData, column, descending, &s1, &s2](size_t r1, size_t r2) -> bool
        { tableData.GetStringValue(r1, column, s1);
          tableData.GetStringValue(r2, column, s2);
          return descending ? (s1 > s2) : (s1 < s2); });
    }
}

END_NCBI_SCOPE
