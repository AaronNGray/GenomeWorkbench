/*  $Id: td_query_data_source.cpp 36811 2016-11-02 13:41:29Z falkrb $
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
 * Authors: Robert Falk
 *
 * File Description:
 *
 */

#include <ncbi_pch.hpp>

#include <gui/objutils/td_query_data_source.hpp>

#include <util/qparse/query_exec.hpp>
#include <util/qparse/query_parse.hpp>
#include <gui/objutils/table_query_exec.hpp>

#include <gui/objutils/query_func_promote.hpp>


BEGIN_NCBI_SCOPE

string CTDQueryDataSource::GetColumnLabel(size_t col) const
{
    return m_Table->GetColumnLabel(col);
}

size_t CTDQueryDataSource::GetColsCount() const
{
    return m_Table->GetColsCount();
}

CMacroQueryExec* CTDQueryDataSource::GetQueryExec(bool casesensitive, CStringMatching::EStringMatching string_matching)
{
    CTableQueryExec* qexec = new CTableQueryExec(*m_Table);

    // Logical operators:
    qexec->AddFunc(CQueryParseNode::eAnd,
            new CQueryFuncPromoteAndOr());
    qexec->AddFunc(CQueryParseNode::eOr,
            new CQueryFuncPromoteAndOr());
    qexec->AddFunc(CQueryParseNode::eSub,
            new CQueryFuncPromoteLogic());
    qexec->AddFunc(CQueryParseNode::eXor,
            new CQueryFuncPromoteLogic());
    qexec->AddFunc(CQueryParseNode::eNot,
            new CQueryFuncPromoteLogic());

    // Constants:
    qexec->AddFunc(CQueryParseNode::eIntConst,
            new CQueryFuncPromoteValue());
    qexec->AddFunc(CQueryParseNode::eFloatConst,
            new CQueryFuncPromoteValue());
    qexec->AddFunc(CQueryParseNode::eBoolConst,
            new CQueryFuncPromoteValue());
    qexec->AddFunc(CQueryParseNode::eString,
            new CQueryFuncPromoteValue());

    NStr::ECase cs = casesensitive ? NStr::eCase  : NStr::eNocase;

    // Comparison operators:
    qexec->AddFunc(CQueryParseNode::eEQ,
            new CQueryFuncPromoteEq(cs, string_matching));
    qexec->AddFunc(CQueryParseNode::eGT,
            new CQueryFuncPromoteGtLt(CQueryParseNode::eGT, cs));
    qexec->AddFunc(CQueryParseNode::eGE,
            new CQueryFuncPromoteGtLt(CQueryParseNode::eGE, cs));
    qexec->AddFunc(CQueryParseNode::eLT,
            new CQueryFuncPromoteGtLt(CQueryParseNode::eLT, cs));
    qexec->AddFunc(CQueryParseNode::eLE,
            new CQueryFuncPromoteGtLt(CQueryParseNode::eLE, cs));
    qexec->AddFunc(CQueryParseNode::eIn,
            new CQueryFuncPromoteIn(cs, string_matching));
    qexec->AddFunc(CQueryParseNode::eBetween,
            new CQueryFuncPromoteBetween(cs));
    qexec->AddFunc(CQueryParseNode::eLike,
            new CQueryFuncLike(cs));

    return qexec;
}

void CTDQueryDataSource::SetQueryResults(CMacroQueryExec* q)
{
    CTableQueryExec *e = dynamic_cast<CTableQueryExec*>(q);

    m_QueryResults =  e->GetTableSelected();
}

void CTDQueryDataSource::ClearQueryResults()
{
    m_QueryResults.clear();
}

void CTDQueryDataSource::ExecuteStringQuery(const string &query, size_t& num_selected, size_t& num_queried, CStringMatching::EStringMatching string_matching, NStr::ECase use_case)
{
    ClearQueryResults();
    string val;
    
    CStringMatching matching(query, string_matching, use_case);

    for (size_t i=0; i<m_Table->GetRowsCount(); ++i) {

        bool selected = false;
        for (size_t col = 0; col < m_Table->GetColsCount() && !selected; ++col) {           
            m_Table->GetStringValue(i, col, val);
            if (matching.MatchString(val)) {
                m_QueryResults.push_back(i);
                selected = true;
            }
        }    
    }
    
    num_selected = m_QueryResults.size();
    num_queried = m_Table->GetRowsCount();
}


END_NCBI_SCOPE

