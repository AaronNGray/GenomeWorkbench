/*  $Id: convert_graph.cpp 14666 2007-07-09 13:40:22Z dicuccio $
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

#include <ncbi_pch.hpp>
#include "convert_graph.hpp"
#include <algorithm>

BEGIN_NCBI_SCOPE

size_t CConvGraph::add_edge(size_t fromVertex, size_t toVertex)
{
    m_Edges.push_back(make_pair(fromVertex, toVertex));
    return m_Edges.size() - 1;
}

void CConvGraph::Dump(ostream& ostream) const
{
    ostream << "Edges: " << endl;
    ITERATE(vector<TEdge>, iter, m_Edges)
        ostream << iter->first << " --> " << iter->second << endl;
}

void CConvGraph::DumpPath(const TPath& path, ostream& ostream) const
{
    for (size_t i = 0; i < path.size(); ++i) {
        if (path[i] >= m_Edges.size()) {
            ostream << " XXX ";
        }
        else  {
            if (i > 0) {
                size_t prev_edge = path[i - 1];
                if (prev_edge >= m_Edges.size() ||
                    m_Edges[prev_edge].second != m_Edges[i].first) {
                    ostream << " | " << m_Edges[i].first;
                }
            }
            else
                ostream << m_Edges[i].first;

            ostream << " --> " << m_Edges[i].second;
        }
    }

    ostream << endl;
}

class DFS
{
public:
    DFS (const vector<CConvGraph::TEdge>& edges,
         size_t startVertex,
         size_t endVertex,
         vector<CConvGraph::TPath>& paths);

private:
    enum Color { WHITE, GRAY, BLACK };
    typedef vector<size_t> TTypeAjacentList;

    void process_vertex(size_t vertex);

    const vector<CConvGraph::TEdge>& m_Edges;
    vector<CConvGraph::TPath>& m_Paths;

    vector<TTypeAjacentList> m_AjacentVertices;

    CConvGraph::TPath m_Queue;
    vector<Color> m_Colors;
    size_t m_EndVertex;
};

static bool PCompare(const CConvGraph::TPath& p1, const CConvGraph::TPath& p2)
    { return p1.size() < p2.size(); }

void CConvGraph::FindPaths (size_t startVertex,
                            size_t endVertex,
                            vector<TPath>& paths) const
{
    DFS(m_Edges, startVertex, endVertex, paths);
    sort(paths.begin(), paths.end(), PCompare);
}

DFS::DFS (const vector<CConvGraph::TEdge>& edges,
          size_t startVertex,
          size_t endVertex,
          vector<CConvGraph::TPath>& paths) :
    m_Edges(edges), m_Paths(paths), m_EndVertex(endVertex)
{
    size_t size = 0;
    ITERATE(vector<CConvGraph::TEdge>, iter, m_Edges)
        size = max(size, 1 + max(iter->first, iter->second));

    if (startVertex >= size || endVertex >= size || startVertex  == endVertex)
        return;

    m_AjacentVertices.clear();
    m_AjacentVertices.resize(size);

    size_t idx = 0;
    ITERATE(vector<CConvGraph::TEdge>, iter, m_Edges)
        m_AjacentVertices[iter->first].push_back(idx++);

    m_Queue.clear();
    m_Colors.assign(size, WHITE);
    process_vertex (startVertex);
}

void DFS::process_vertex(size_t vertex)
{
    if (vertex == m_EndVertex) {
        m_Paths.push_back(m_Queue);
        return;
    }
    else {
        m_Colors[vertex] = GRAY;
        ITERATE (TTypeAjacentList, iter, m_AjacentVertices[vertex]) {
            size_t v2 = m_Edges[*iter].second;
            if (m_Colors[v2] == WHITE || m_Colors[v2] == BLACK) {
                m_Queue.push_back(*iter);
                process_vertex(v2);
                m_Queue.pop_back();
            }
        }
        m_Colors[vertex] = BLACK;
    }
}


END_NCBI_SCOPE
