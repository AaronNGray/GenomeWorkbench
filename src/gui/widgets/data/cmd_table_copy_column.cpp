/*  $Id: cmd_table_copy_column.cpp 32112 2015-01-02 14:35:27Z kuznets $
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
 * Authors:  Roman Katargin
 */


#include <ncbi_pch.hpp>
#include <gui/widgets/data/cmd_table_copy_column.hpp>

BEGIN_NCBI_SCOPE
USING_SCOPE(objects);

void CCmdTableCopyColumn::Execute()
{
    int columns = (int)m_TableData->GetColsCount();
    if (m_ColFrom < 0 || m_ColFrom >= columns ||
        m_ColTo < 0 || m_ColTo >= columns)
        return;

    if (m_TableData->GetColumnType(m_ColTo) != ITableData::kString)
        return;

    size_t rows = m_TableData->GetRowsCount();

    m_SaveValues.clear();
    string saveValue;
    string newValue;

    for (size_t row = 0; row < rows; ++row) {
        m_TableData->GetStringValue(row, m_ColTo, saveValue);
        m_SaveValues.push_back(saveValue);

        m_TableData->GetStringValue(row, m_ColFrom, newValue);
        switch(m_Op) {
        case eCopy :
            break;
        case eAppend :
            newValue = saveValue + newValue;
            break;
        case ePrepend :
            newValue = newValue + saveValue;
            break;
        }
        m_TableData->SetStringValue(row, m_ColTo, newValue);
    }
}

void CCmdTableCopyColumn::Unexecute()
{
    int columns = (int)m_TableData->GetColsCount();
    if (m_ColFrom < 0 || m_ColFrom >= columns ||
        m_ColTo < 0 || m_ColTo >= columns)
        return;

    size_t rows = m_TableData->GetRowsCount();
    if (m_SaveValues.size() != rows)
        return;

    for (size_t row = 0; row < rows; ++row) {
        m_TableData->SetStringValue(row, m_ColTo, m_SaveValues[row]);
    }

    m_SaveValues.clear();
}

string CCmdTableCopyColumn::GetLabel()
{
    return "Copy column to column";
}

END_NCBI_SCOPE
