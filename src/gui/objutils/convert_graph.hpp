#ifndef GUI_UTILS___CONVERT_GRAPH__HPP
#define GUI_UTILS___CONVERT_GRAPH__HPP

/*  $Id: convert_graph.hpp 14666 2007-07-09 13:40:22Z dicuccio $
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
 * Authors: Roman Katargin
 *
 * File Description:
 *
 */

#include <corelib/ncbiobj.hpp>

BEGIN_NCBI_SCOPE

class CConvGraph
{
public:
    typedef vector<size_t> TPath;

    CConvGraph() {}
    virtual ~CConvGraph() {}

    size_t add_edge(size_t from, size_t to);
    void FindPaths (size_t startVertex, size_t endVertex, vector<TPath>& paths) const;

    void Dump(ostream& ostream) const;
    void DumpPath(const TPath& path, ostream& ostream) const;

protected:
    typedef pair<size_t, size_t> TEdge;
    vector<TEdge> m_Edges;

    friend class DFS;
};


END_NCBI_SCOPE

#endif  // GUI_UTILS___CONVERT_GRAPH__HPP
