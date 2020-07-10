/*  $Id: filter.cpp 14565 2007-05-18 12:32:01Z dicuccio $
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
 * Authors:  Mike DiCuccio
 *
 * File Description:
 *    CFilterSet -- container for a number of column filtering rules
 *    CFilter -- encapsulates a single filter rule
 */

#include <ncbi_pch.hpp>
#include <gui/objutils/filter.hpp>

BEGIN_NCBI_SCOPE



CFilter::CFilter(const string& name, int col, EMode mode,
                 const string& data)
    : m_Name(name),
      m_Col(col),
      m_Mode(mode),
      m_Data(data)
{
}


bool CFilter::Filter(const vector<string>& row, ECompare compare) const
{
    if (m_Col < 0  ||  m_Col >= (int)row.size()) {
        return false;
    }

    switch (m_Mode) {
    case ePass:
        return true;

    case eContains:
        // we force comparison as a string
        return (row[m_Col].find(m_Data) != string::npos);

    case eDoesntContain:
        // we force comparison as a string
        return (row[m_Col].find(m_Data) == string::npos);

    case eEquals:
        if (compare == eString) {
            return (row[m_Col] == m_Data);
        }
        return (NStr::StringToInt(row[m_Col], NStr::fAllowTrailingSymbols) ==
                NStr::StringToInt(m_Data,     NStr::fAllowTrailingSymbols));

    case eDoesntEqual:
        if (compare == eString) {
            return (row[m_Col] != m_Data);
        }
        return (NStr::StringToInt(row[m_Col], NStr::fAllowTrailingSymbols) !=
                NStr::StringToInt(m_Data,     NStr::fAllowTrailingSymbols));

    case eLess:
        if (compare == eString) {
            return (row[m_Col] < m_Data);
        }
        return (NStr::StringToInt(row[m_Col], NStr::fAllowTrailingSymbols) <
                NStr::StringToInt(m_Data,     NStr::fAllowTrailingSymbols));

    case eLessEquals:
        if (compare == eString) {
            return (row[m_Col] <= m_Data);
        }
        return (NStr::StringToInt(row[m_Col], NStr::fAllowTrailingSymbols) <=
                NStr::StringToInt(m_Data,     NStr::fAllowTrailingSymbols));

    case eGreater:
        if (compare == eString) {
            return (row[m_Col] > m_Data);
        }
        return (NStr::StringToInt(row[m_Col], NStr::fAllowTrailingSymbols) >
                NStr::StringToInt(m_Data,     NStr::fAllowTrailingSymbols));

    case eGreaterEquals:
        if (compare == eString) {
            return (row[m_Col] >= m_Data);
        }
        return (NStr::StringToInt(row[m_Col], NStr::fAllowTrailingSymbols) >=
                NStr::StringToInt(m_Data,     NStr::fAllowTrailingSymbols));
    }

    return false;
}



// filter a given row, returning true if a match exists
bool CFilterSet::Filter(const vector<string>& row) const
{
    ITERATE (TFilters, iter, m_Filters) {
        const CFilter& filter = **iter;
        int col = filter.GetColumn();
        if ( !filter.Filter(row, m_ColTypes[col]) ) {
            return false;
        }
    }

    return true;
}


// add a filter to the list
void CFilterSet::Add(CFilter* filter)
{
    m_Filters.push_back(CRef<CFilter>(filter));
}


// remove a named filter
void CFilterSet::Remove(const string& name)
{
    NON_CONST_ITERATE (TFilters, iter, m_Filters) {
        if ((*iter)->GetName() == name) {
            iter = m_Filters.erase(iter);
            --iter;
        }
    }
}


// remove a filter by pointer
void CFilterSet::Remove(CFilter* filter)
{
    NON_CONST_ITERATE (TFilters, iter, m_Filters) {
        if ((*iter).GetPointer() == filter) {
            m_Filters.erase(iter);
            break;
        }
    }
}


void CFilterSet::SetColType(size_t col, CFilter::ECompare comp)
{
    if (m_ColTypes.size() <= col) {
        m_ColTypes.resize(col + 1, CFilter::eString);
    }
    m_ColTypes[col] = comp;
}


END_NCBI_SCOPE
