/*  $Id: cmd_table_assign_column.cpp 32112 2015-01-02 14:35:27Z kuznets $
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
#include <gui/widgets/data/cmd_table_assign_column.hpp>

BEGIN_NCBI_SCOPE
USING_SCOPE(objects);

void CCmdTableAssignColumn::Execute()
{
    size_t rows = m_TableData->GetRowsCount();

    if (m_Col >= 0 && m_Col < (int)m_TableData->GetColsCount() && m_TableData->AllowEdit(m_Col)) {

        m_SaveValues.clear();
        switch(m_TableData->GetColumnType(m_Col)) {
        case ITableData::kInt:
            {{
                long newVal = 0;
                if (!m_Value.Convert(&newVal))
                    return;

                for (size_t row = 0; row < rows; ++row) {
                    long saveValue = m_TableData->GetIntValue(row, m_Col);
                    m_SaveValues.push_back(wxVariant(saveValue));
                    m_TableData->SetIntValue(row, m_Col, newVal);

                }
            }}
            break;
        case ITableData::kReal:
            {{
                double newVal = 0;
                if (!m_Value.Convert(&newVal))
                    return;

                for (size_t row = 0; row < rows; ++row) {
                    double saveValue = m_TableData->GetRealValue(row, m_Col);
                    m_SaveValues.push_back(wxVariant(saveValue));
                    m_TableData->SetRealValue(row, m_Col, newVal);

                }
            }}
            break;
        case ITableData::kCommonString:
        case ITableData::kString:
            {{
                wxString newVal;
                if (!m_Value.Convert(&newVal))
                    return;

                string newValUTF8(newVal.ToUTF8());
                string saveValue;
                for (size_t row = 0; row < rows; ++row) {
                    m_TableData->GetStringValue(row, m_Col, saveValue);
                    m_SaveValues.push_back(wxVariant(wxString::FromUTF8(saveValue.c_str())));
                    if (m_Op == eAppend)
                        m_TableData->SetStringValue(row, m_Col, saveValue + newValUTF8);
                    else if (m_Op == ePrepend)
                        m_TableData->SetStringValue(row, m_Col, newValUTF8 + saveValue);
                    else
                        m_TableData->SetStringValue(row, m_Col, newValUTF8);
                }
            }}
            break;
            
        default:
            break;
        }
    }
}

void CCmdTableAssignColumn::Unexecute()
{
    size_t rows = m_TableData->GetRowsCount();
    if (m_SaveValues.size() != rows)
        return;

    if (m_Col >= 0 && m_Col < (int)m_TableData->GetColsCount() && m_TableData->AllowEdit(m_Col)) {

        switch(m_TableData->GetColumnType(m_Col)) {
        case ITableData::kInt:
            {{
                for (size_t row = 0; row < rows; ++row) {
                    long saveValue = 0;
                    if (m_SaveValues[row].Convert(&saveValue)) {
                        m_TableData->SetIntValue(row, m_Col, saveValue);
                    }
                }
            }}
            break;
        case ITableData::kReal:
            {{
                for (size_t row = 0; row < rows; ++row) {
                    double saveValue = 0;
                    if (m_SaveValues[row].Convert(&saveValue)) {
                        m_TableData->SetRealValue(row, m_Col, saveValue);
                    }
                }
            }}
            break;
        case ITableData::kCommonString:
        case ITableData::kString:
            {{
                for (size_t row = 0; row < rows; ++row) {
                    wxString saveValue;
                    if (m_SaveValues[row].Convert(&saveValue)) {
                        string saveValUTF8(saveValue.ToUTF8());
                        m_TableData->SetStringValue(row, m_Col, saveValUTF8);
                    }
                }
            }}
            break;
            
        default:
            break;
        }
    }
    m_SaveValues.clear();
}

string CCmdTableAssignColumn::GetLabel()
{
    return "Change column value";
}

END_NCBI_SCOPE
