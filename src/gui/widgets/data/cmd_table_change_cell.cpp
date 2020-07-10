/*  $Id: cmd_table_change_cell.cpp 32112 2015-01-02 14:35:27Z kuznets $
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
#include <gui/widgets/data/cmd_table_change_cell.hpp>

BEGIN_NCBI_SCOPE
USING_SCOPE(objects);

void CCmdTableChangeCell::Execute()
{
    if (m_Row >= 0 && m_Row < (int)m_TableData->GetRowsCount() &&
        m_Col >= 0 && m_Col < (int)m_TableData->GetColsCount() && m_TableData->AllowEdit(m_Col)) {
        switch(m_TableData->GetColumnType(m_Col)) {
        case ITableData::kInt:
            {{
                long newVal = 0, saveValue = m_TableData->GetIntValue(m_Row, m_Col);
                if (m_Value.Convert(&newVal)) {
                    m_TableData->SetIntValue(m_Row, m_Col, newVal);
                    m_Value = saveValue;
                }
            }}
            break;
        case ITableData::kReal:
            {{
                double newVal = 0, saveValue = m_TableData->GetRealValue(m_Row, m_Col);
                if (m_Value.Convert(&newVal)) {
                    m_TableData->SetRealValue(m_Row, m_Col, newVal);
                    m_Value = saveValue;
                }
            }}
            break;
        case ITableData::kCommonString:
        case ITableData::kString:
            {{
                string saveValue; 
                m_TableData->GetStringValue(m_Row, m_Col, saveValue);
                wxString newVal;
                if (m_Value.Convert(&newVal)) {
                    string newValUTF8(newVal.ToUTF8());
                    m_TableData->SetStringValue(m_Row, m_Col, newValUTF8);
                    m_Value = wxString::FromUTF8(saveValue.c_str());
                }
            }}
            break;
            
        default:
            break;
        }
    }
}

void CCmdTableChangeCell::Unexecute()
{
    Execute();
}

string CCmdTableChangeCell::GetLabel()
{
    return "Change cell value";
}

END_NCBI_SCOPE
