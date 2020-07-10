#ifndef GUI_OBJUTILS___TD_QUERY_DATA_SOURCE__HPP
#define GUI_OBJUTILS___TD_QUERY_DATA_SOURCE__HPP

/*  $Id: td_query_data_source.hpp 36811 2016-11-02 13:41:29Z falkrb $
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
 *
 */

#include <objmgr/scope.hpp>
#include <gui/gui_export.h>

#include <gui/objutils/query_data_source.hpp>
#include <gui/objutils/table_data.hpp>

BEGIN_NCBI_SCOPE

////////////////////////////////////////////////////////////////////////////////
/// class IQueryDataSource
///
/// This is an abstract interface for any datasource that wants to support
/// queries using CQueryParseTree and CQueryExec.  This interface supports 
/// searching the data source for a spcified string (when parsing is not being
/// used) and supports the creation of a query execution object that is 
/// tailored for that data source.  This interface also allows the user
/// to set and clear a set of selected results for the data source.
///  
class NCBI_GUIOBJUTILS_EXPORT CTDQueryDataSource : public CObject, public IQueryDataSource
{
public:
    CTDQueryDataSource(ITableData& tabelData) : m_Table(&tabelData) {}

    /// @name IQueryDataSource interface implementation
    /// @{
    virtual string GetColumnLabel(size_t col) const;
    virtual size_t GetColsCount() const;

    virtual CMacroQueryExec* GetQueryExec(bool casesensitive, CStringMatching::EStringMatching string_matching = CStringMatching::ePlainSearch);
    virtual void SetQueryResults(CMacroQueryExec* q);
    virtual void ClearQueryResults();

    virtual void ExecuteStringQuery(const string &query, size_t& num_selected, size_t& num_queried, CStringMatching::EStringMatching string_matching = CStringMatching::ePlainSearch, NStr::ECase use_case = NStr::eCase);
     /// @}

    std::vector<size_t> GetQueryResults() const { return m_QueryResults; }

protected:
    std::vector<size_t> m_QueryResults; 
    CIRef<ITableData>   m_Table;
};

END_NCBI_SCOPE

#endif // GUI_OBJUTILS___TD_QUERY_DATA_SOURCE__HPP
