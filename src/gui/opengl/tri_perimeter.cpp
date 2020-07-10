/*  $Id: tri_perimeter.cpp 43891 2019-09-16 13:50:00Z evgeniev $
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
 * Authors:  Peter Meric, Bob Falk
 *
 * File Description:
 *   Helper class to extract a single perimeter from a bunch of triangles
 *
 */

#include <ncbi_pch.hpp>
#include <gui/opengl/tri_perimeter.hpp>

BEGIN_NCBI_SCOPE

size_t CTriPerimeter::AddTri(CVect2<float>& v1p,
    CVect2<float>& v2p,
    CVect2<float>& v3p) {

    size_t idx1 = AddVertex(v1p);
    size_t idx2 = AddVertex(v2p);
    size_t idx3 = AddVertex(v3p);

    size_t e1 = AddEdge(idx1, idx2);
    size_t e2 = AddEdge(idx2, idx3);
    size_t e3 = AddEdge(idx3, idx1);

    m_Triangles.push_back(CVect3<size_t>(e1, e2, e3));

    return (m_Triangles.size() - 1);
}

size_t CTriPerimeter::AddVertex(CVect2<float>& v)
{
    float min_dist = 1e10f;
    size_t min_idx = (size_t)-1;

    for (size_t i = 0; i < m_Vertices.size(); ++i) {
        float dist = (v - m_Vertices[i]).Length2();
        if (dist < min_dist) {
            min_idx = i;
            min_dist = dist;
        }
    }

    if (min_dist > 1e-6f) {
        min_idx = m_Vertices.size();
        m_Vertices.push_back(v);
    }

    return min_idx;
}

size_t CTriPerimeter::AddEdge(size_t vert1, size_t vert2)
{
    // edges will have lower index first
    CVect2<size_t> edge(vert1, vert2);

    if (vert1 > vert2) {
        edge.Set(vert2, vert1);
    }

    vector<CVect2<size_t> >::iterator iter;
    iter = find(m_Edges.begin(), m_Edges.end(), edge);

    if (iter != m_Edges.end())
        return (size_t(iter - m_Edges.begin()));

    m_Edges.push_back(edge);
    return m_Edges.size() - 1;
}

/// returns empty vec if no perimeter can be found.
vector<CVect2<float> > CTriPerimeter::GetPerimiter() const
{
    // First find all unshared edges
    vector<int> edge_tris(m_Edges.size(), 0);

    size_t i;
    for (i = 0; i < m_Triangles.size(); ++i) {
        for (int j = 0; j < 3; ++j)
            edge_tris[m_Triangles[i][j]] += 1;
    }

    vector<size_t> unshared_edges;
    for (i = 0; i < edge_tris.size(); ++i) {
        if (edge_tris[i] == 1)
            unshared_edges.push_back(i);
    }

    vector<size_t> perimeter_vertices;
    vector<CVect2<float> > perimeter;

    // Take the first unshared edge then loop over the edges repeatedly taking
    // each time an edge that connects to the second vertex until the second
    // vertex of an edge connects to the first vertex of the first edge or
    // no match is found.
    bool done = false;
    if (unshared_edges.size() < 3)
        return perimeter;

    size_t first_vert = m_Edges[unshared_edges.back()][0];
    size_t last_vert = m_Edges[unshared_edges.back()][1];

    perimeter_vertices.push_back(first_vert);
    perimeter_vertices.push_back(last_vert);

    unshared_edges.pop_back();

    while (!done && unshared_edges.size() > 0) {
        int next_edge = -1;

        for (i = 0; i < unshared_edges.size() && next_edge == -1; ++i) {
            if (m_Edges[unshared_edges[i]][0] == last_vert) {
                last_vert = m_Edges[unshared_edges[i]][1];
                if (last_vert == first_vert)
                    done = true;
                else
                    perimeter_vertices.push_back(last_vert);

                next_edge = (int)i;
            }
            else if (m_Edges[unshared_edges[i]][1] == last_vert) {
                last_vert = m_Edges[unshared_edges[i]][0];
                if (last_vert == first_vert)
                    done = true;
                else
                    perimeter_vertices.push_back(last_vert);

                next_edge = (int)i;
            }
        }
        if (next_edge != -1) {
            unshared_edges.erase(unshared_edges.begin() + next_edge);
        }
        else {
            done = true;
        }
    }

    if (unshared_edges.size() == 0) {
        for (i = 0; i < perimeter_vertices.size(); ++i)
            perimeter.push_back(m_Vertices[perimeter_vertices[i]]);
    }

    return perimeter;
}

END_NCBI_SCOPE
