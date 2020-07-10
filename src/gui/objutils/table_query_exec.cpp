/*  $Id: table_query_exec.cpp 39021 2017-07-20 16:37:55Z falkrb $
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
 * Authors:  Robert Falk
 *
 * File Description:
 *      Implementation for tree query execution
 */

#include <ncbi_pch.hpp>

#include <gui/objutils/table_query_exec.hpp>
#include <gui/objutils/query_func_promote.hpp>

#include <objects/seq/seq_id_handle.hpp>

#include <corelib/ncbiobj.hpp>
#include <corelib/ncbistd.hpp>

BEGIN_NCBI_SCOPE


/******************************************************************************
 *
 * CTableQueryExec Implementation
 *
 *****************************************************************************/ 
CTableQueryExec::CTableQueryExec(ITableData& td) 
: m_EvalRow((unsigned int)-1) 
{ 
    m_TableData.Reset(&td); 
}

bool CTableQueryExec::ResolveIdentifier(const std::string& identifier, 
                                        bool& value) 
{ 
    TFieldID col = (TFieldID)x_GetColumnIdx(identifier);

    return ResolveIdentifier(col, value);
}

bool CTableQueryExec::ResolveIdentifier(const std::string& identifier, 
                                        Int8& value) 
{ 
    TFieldID col = (TFieldID)x_GetColumnIdx(identifier);

    return ResolveIdentifier(col, value);
}

bool CTableQueryExec::ResolveIdentifier(const std::string& identifier, 
                                        double& value) 
{ 
    TFieldID col = (TFieldID)x_GetColumnIdx(identifier);

    return ResolveIdentifier(col, value);
}

bool CTableQueryExec::ResolveIdentifier(const std::string& identifier,
                                        std::string& value)
{
    TFieldID col = (TFieldID)x_GetColumnIdx(identifier);

    return ResolveIdentifier(col, value);
}

bool CTableQueryExec::ResolveIdentifier(const TFieldID& id, bool& value)
{
    unsigned int col = (unsigned int)id;

    if (col != (unsigned int)-1) {
        ITableData::ColumnType ct = m_TableData->GetColumnType(col);
        string s;
        Int8 ival;
        double dval;

        switch (ct) {
        case (ITableData::kInt) :
            ival = (Int8)m_TableData->GetIntValue(m_EvalRow, col);
            if (ival == 1) {
                value = true;
                return true;
            }
            else if (ival == 0) {
                value = false;
                return true;
            }
            return false;
            break;
        case (ITableData::kReal) :
            dval = m_TableData->GetIntValue(m_EvalRow, col);
            if (dval == 1.0) {
                value = true;
                return true;
            }
            else if (dval == 0.0) {
                value = false;
                return true;
            }
            return false;
            break;
        case (ITableData::kString) :
        case (ITableData::kCommonString) :
            m_TableData->GetStringValue(m_EvalRow, col, s);
            try {
                value = NStr::StringToBool(s);
                return true;
            }
            catch (CStringException&) {
                return false;
            }
        default:
            return false;
        }
    }

    return false;
}

bool CTableQueryExec::ResolveIdentifier(const TFieldID& id, Int8& value)
{
    unsigned int col = (unsigned int)id;

    if (col != (unsigned int)-1) {
        ITableData::ColumnType ct = m_TableData->GetColumnType(col);
        string s;

        switch (ct) {
        case (ITableData::kInt) :
            value = (Int8)m_TableData->GetIntValue(m_EvalRow, col);
            return true;
            break;
        case (ITableData::kReal) :
            value = (Int8)m_TableData->GetRealValue(m_EvalRow, col);
            return true;
            break;
        case (ITableData::kString) :
        case (ITableData::kCommonString) :
            m_TableData->GetStringValue(m_EvalRow, col, s);
            try {
                value = NStr::StringToInt8(s);
                return true;
            }
            catch (CStringException&) {
                return false;
            }
        default:
            return false;
        }
    }

    return false;
}

bool CTableQueryExec::ResolveIdentifier(const TFieldID& id, double& value)
{
    unsigned int col = (unsigned int)id;

    if (col != (unsigned int)-1) {
        ITableData::ColumnType ct = m_TableData->GetColumnType(col);
        string s;

        switch (ct) {
        case (ITableData::kInt) :
            value = (double)m_TableData->GetIntValue(m_EvalRow, col);
            return true;
            break;
        case (ITableData::kReal) :
            value = m_TableData->GetRealValue(m_EvalRow, col);
            return true;
            break;
        case (ITableData::kString) :
        case (ITableData::kCommonString) :
            m_TableData->GetStringValue(m_EvalRow, col, s);
            try {
                value = NStr::StringToDouble(s);
                return true;
            }
            catch (CStringException&) {
                return false;
            }
        default:
            return false;
        }
    }

    return false;
}

bool CTableQueryExec::ResolveIdentifier(const TFieldID& id, std::string& value)
{
    unsigned int col = (unsigned int)id;

    if (col != (unsigned int)-1) {
        ITableData::ColumnType ct = m_TableData->GetColumnType(col);
        string s;
        long ival;
        double dval;

        switch (ct) {
        case (ITableData::kInt) :
            ival = m_TableData->GetIntValue(m_EvalRow, col);
            value = NStr::LongToString(ival);
            return true;
            break;
        case (ITableData::kReal) :
            dval = m_TableData->GetRealValue(m_EvalRow, col);
            value = NStr::DoubleToString(dval);
            return true;
            break;
        case (ITableData::kString) :
        case (ITableData::kCommonString) :
            m_TableData->GetStringValue(m_EvalRow, col, value);
            return true;
        case (ITableData::kObject) :
            m_TableData->GetStringValue(m_EvalRow, col, value);
            return true;
        default:
            return false;
        }
    }

    return false;
}
/// @}

/// Get biotree feature ID for identifier or return TFieldID(-1)
CTableQueryExec::TFieldID CTableQueryExec::GetIdentifier(const std::string& identifier)
{
    return x_GetColumnIdx(identifier);
}

unsigned int CTableQueryExec::x_GetColumnIdx(const std::string& col_name)
{
    for (unsigned int i=0; i<m_TableData->GetColsCount(); ++i) {
        int cmp = NStr::CompareNocase(col_name.c_str(),  
                                      m_TableData->GetColumnLabel(i).c_str());
        if (cmp == 0)
            return i;
    }

    return (unsigned int)-1;
}


bool CTableQueryExec::HasIdentifier(const std::string& identifier) 
{ 
    for (unsigned int i=0; i<m_TableData->GetColsCount(); ++i) {
        int cmp = NStr::CompareNocase(identifier.c_str(),  
                                      m_TableData->GetColumnLabel(i).c_str());
        if (cmp == 0)
            return true;
    }

    return false;
}

CQueryParseNode::EType CTableQueryExec::IdentifierType(const std::string& identifier)
{ 
    for (unsigned int i=0; i<m_TableData->GetColsCount(); ++i) {
        int cmp = NStr::CompareNocase(identifier.c_str(),  
                                      m_TableData->GetColumnLabel(i).c_str());
        if (cmp == 0) {
            ITableData::ColumnType ct = m_TableData->GetColumnType(i);
            switch (ct) {
                case (ITableData::kInt): return CQueryParseNode::eIntConst;
                case (ITableData::kReal): return CQueryParseNode::eFloatConst;
                case (ITableData::kString): return CQueryParseNode::eString;
                case (ITableData::kCommonString): return CQueryParseNode::eString;                    
                case (ITableData::kObject): return CQueryParseNode::eString;
                case (ITableData::kNone): return CQueryParseNode::eNotSet;
                case (ITableData::kBytes): return CQueryParseNode::eNotSet;
                case (ITableData::kCommonBytes): return CQueryParseNode::eNotSet;
                default: return CQueryParseNode::eNotSet;

            }
        }
    }

    return CQueryParseNode::eNotSet;
}

void CTableQueryExec::EvalStart()
{
    m_ExceptionCount = 0;
    m_QueriedCount = 0;
    m_EvalRow = 0;
}

bool CTableQueryExec::EvalComplete()
{
    return (m_EvalRow >= m_TableData->GetRowsCount());
}

bool CTableQueryExec::EvalNext(CQueryParseTree& qtree) {
    bool selected = false;

    try {
        ++m_QueriedCount;
        Evaluate(qtree);

        CQueryParseTree::TNode* top_node = qtree.GetQueryTree();
        CQueryNodeValue*  v = dynamic_cast<CQueryNodeValue*>(
            top_node->GetValue().GetUserObject());

        if (v != NULL) {
            if (v->GetValue()) {
                m_Selected.push_back(m_EvalRow);
                selected = true;
            }
        }
    }
    catch(CQueryParseException &pe) {
        // Could avoid flooding log since many may be identical, but
        // maybe the last one is the one you need to see (for debugging...)
        LOG_POST("Query parse error during execution: " << pe.GetMsg());
        ++m_ExceptionCount;
    }
    catch(CQueryExecException &ex) {
        // Could avoid flooding log since many may be identical, but
        // maybe the last one is the one you need to see (for debugging...)
        LOG_POST(Info << "Query execution error: " << ex.GetMsg());
        ++m_ExceptionCount;
    }

    ++m_EvalRow;

    return selected;
}




END_NCBI_SCOPE
