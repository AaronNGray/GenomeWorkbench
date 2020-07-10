#ifndef GUI_UTILS___FILTER__HPP
#define GUI_UTILS___FILTER__HPP

/*  $Id: filter.hpp 14562 2007-05-18 11:48:21Z dicuccio $
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


#include <corelib/ncbiobj.hpp>
#include <gui/gui.hpp>

#include <vector>
#include <string>


/** @addtogroup GUI_UTILS
 *
 * @{
 */


BEGIN_NCBI_SCOPE


//
// class CFilter defines the interface for a single filter object.
// This class represents a single filter rule, where a rule is defined as a
// comparison operation performed on a single entry in a row of a table.  The
// API assumes that a row can be expressed as a vector of strings, one string
// per column; comparisons are then done on those strings in a format defined
// by the caller.  Formats supported include string and numeric comparisons.
//
class NCBI_GUIOBJUTILS_EXPORT CFilter : public CObject
{
public:

    // our modes of filtering
    enum EMode {
        ePass,
        eContains,
        eDoesntContain,
        eEquals,
        eDoesntEqual,
        eLess,
        eLessEquals,
        eGreater,
        eGreaterEquals
    };

    // the type of comparison we perform (numeric vs. string)
    enum ECompare {
        eString,
        eNumeric
    };

    // default ctor
    CFilter(const string& name,
            int col, EMode mode,
            const string& data);

    // apply our filter to a single row
    bool Filter(const vector<string>& row,
                ECompare comp) const;

    //
    // accessors
    //

    const string&   GetName(void) const         { return m_Name; }
    void            SetName(const string& s)    { m_Name = s; }

    int             GetColumn(void) const       { return m_Col; }
    void            SetColumn(int col)          { m_Col = col; }

    EMode           GetMode(void) const         { return m_Mode; }
    void            SetMode(EMode mode)         { m_Mode = mode; }

    const string&   GetData(void) const         { return m_Data; }
    void            SetData(const string& s)    { m_Data = s; }

private:

    // a name for this filter
    string m_Name;

    // the column we compare (-1 = unassigned)
    int m_Col;

    // our comparison mode
    EMode m_Mode;

    // what we match / don't match
    string m_Data;
};



//
// class CFilterSet is a container of filters that permits row-by-row
// application of all named filters.
//
// This is the main interface class for filtering tables.  This class maintains
// a list of filters and applies each filter sequentially to a row as the row
// is supplied to it.  Given its set of filters, this container will return
// 'true' to indicate a row matches all filters or 'false' to indicate that a
// row fails to match at least one filter.
//
class NCBI_GUIOBJUTILS_EXPORT CFilterSet
{
public:
    typedef list< CRef<CFilter> > TFilters;

    // filter a given row, returning true if a match exists for all filters in
    // the set
    bool Filter(const vector<string>& row) const;

    // add a filter to the list
    void Add(CFilter* filter);

    // remove a named filter
    void Remove(const string& name);

    // remove a filter by pointer
    void Remove(CFilter* filter);

    // access the filters
    const TFilters& GetFilters(void) const   { return m_Filters; }

    // set the sorting mechanism for a numbered column
    void SetColType(size_t col, CFilter::ECompare type);

private:

    // a notion of how our columns are to be sorted and compared
    vector<CFilter::ECompare> m_ColTypes;

    // our list of named filters
    TFilters m_Filters;

};


END_NCBI_SCOPE

/* @} */

#endif  // GUI_UTILS___FILTER__HPP
