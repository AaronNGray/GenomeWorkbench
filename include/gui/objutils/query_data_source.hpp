#ifndef GUI_OBJUTILS___QUERY_DATA_SOURCE__HPP
#define GUI_OBJUTILS___QUERY_DATA_SOURCE__HPP

/*  $Id: query_data_source.hpp 36811 2016-11-02 13:41:29Z falkrb $
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

#include <corelib/ncbistr.hpp>
#include <objmgr/scope.hpp>
#include <gui/gui_export.h>
#include <gui/objutils/string_matching.hpp>

BEGIN_NCBI_SCOPE

class CMacroQueryExec;

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
class NCBI_GUIOBJUTILS_EXPORT IQueryDataSource {
public:
    typedef vector<pair<string,string> >  TNamedQueries;

    /// dtor
    virtual ~IQueryDataSource() {}

    /// get number and names of columns in data source
    virtual string GetColumnLabel(size_t col) const = 0;
    virtual size_t GetColsCount() const = 0;
 
    /// Execute a string-matching query on the underlying data
    /// @param[in] query
    ///     Reference to a pattern to search for.
    /// @param[out] num_selected
    ///     Number of matching rows.
    /// @param[out] num_queried
    ///     Total rows queried.
    /// @param[in] string_matching
    ///     String matching algorithm to use.
    /// @param[in] use_case
    ///     Whether to do a case sensitive compare (default), or not. 
    virtual void ExecuteStringQuery(const string &query, size_t& num_selected, size_t& num_queried, CStringMatching::EStringMatching string_matching = CStringMatching::ePlainSearch, NStr::ECase use_case = NStr::eCase) = 0;

    /// Return a scope for the data source, if available.
    virtual CRef<objects::CScope> GetScope() { return CRef<objects::CScope>(); }

    /// Create an appropriate instance of a subclass of CQueryExec for that
    /// will be used to execute a query on the data source.
    virtual CMacroQueryExec* GetQueryExec(bool casesensitive, CStringMatching::EStringMatching matching) = 0;

    /// Clear any current results from previous queries
    virtual void ClearQueryResults() = 0;
    /// Set selection results to be whatever elements are selected in 'q'.
    virtual void SetQueryResults(CMacroQueryExec* q) = 0;
};

END_NCBI_SCOPE

#endif // GUI_OBJUTILS___QUERY_DATA_SOURCE__HPP
