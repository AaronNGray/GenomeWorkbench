#ifndef __GUI_WIDGETS_ALNMULTI___ALNVEC_ROW_GRAPH__HPP
#define __GUI_WIDGETS_ALNMULTI___ALNVEC_ROW_GRAPH__HPP

/*  $Id: alnvec_row_graph.hpp 29658 2014-01-23 22:00:59Z wuliangs $
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
 * Authors:  Andrey Yazhuk
 *
 * File Description:
 *
 */

#include <objtools/alnmgr/alnmap.hpp>


BEGIN_NCBI_SCOPE

class IAlnSegmentIterator;


////////////////////////////////////////////////////////////////////////////////
/// IAlnRowGraphProperties
class NCBI_GUIWIDGETS_ALNMULTIPLE_EXPORT  IAlnRowGraphProperties
{
public:
    virtual ~IAlnRowGraphProperties() {};
};


////////////////////////////////////////////////////////////////////////////////
/// IAlnVecRowGraph - abstract graph that can be rendered in the "expandable"
/// space below the CAlnVecRow.

class NCBI_GUIWIDGETS_ALNMULTIPLE_EXPORT  IAlnVecRowGraph
{
public:
    virtual ~IAlnVecRowGraph()  {};

    /// Graph is not considered fully functional until it has been "created" by
    /// the call to create function. Only "Created" graph can be rendered.
    /// Creating may include loading data and building internal data structures.
    virtual bool    IsCreated() const = 0;
    virtual bool    Create() = 0;
    virtual void    Destroy() = 0;
    /// Update data according to the input range.
    /// @param range the current visible range
    virtual void    Update(double start, double stop) = 0;

    virtual const IAlnRowGraphProperties*     GetProperties() const = 0;
    virtual void    SetProperties(IAlnRowGraphProperties* props) = 0;

    virtual void    Render(CGlPane& pane, IAlnSegmentIterator& it) = 0;
};

END_NCBI_SCOPE

#endif  // __GUI_WIDGETS_ALNMULTI___ALNVEC_ROW_GRAPH__HPP
