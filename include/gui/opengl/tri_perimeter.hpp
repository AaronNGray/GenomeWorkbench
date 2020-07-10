#ifndef GUI_UTILS__TRI_PERIMETER_HPP
#define GUI_UTILS__TRI_PERIMETER_HPP

/*  $Id: tri_perimeter.hpp 44300 2019-11-25 19:10:32Z evgeniev $
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
 * Authors: Peter Meric, Bob Falk
 *
 * File Description:
 *   Helper class to extract a single perimeter from a bunch of triangles
 *
 */

#include <corelib/ncbistd.hpp>
#include <gui/gui.hpp>
#include <gui/utils/vect2.hpp>
#include <gui/utils/vect3.hpp>

BEGIN_NCBI_SCOPE

///
/// Simple helper class to take a bunch of triangles and extract from
/// that set, if possible, a single perimeter.  If the triangle set happens
/// to have no perimeters or multiple perimeters, nothing will be returned.
class NCBI_GUIOPENGL_EXPORT CTriPerimeter
{
public:
    size_t AddTri(CVect2<float>& v1p, CVect2<float>& v2p, CVect2<float>& v3p);
    size_t AddVertex(CVect2<float>& v);
    size_t AddEdge(size_t vert1, size_t vert2);
    /// returns empty vec if no perimeter can be found.
    std::vector<CVect2<float> > GetPerimiter() const;

    void Clear()
    {
        m_Vertices.clear();
        m_Edges.clear();
        m_Triangles.clear();
    };
protected:
    std::vector<CVect2<float> > m_Vertices;
    std::vector<CVect2<size_t> > m_Edges;
    std::vector<CVect3<size_t> > m_Triangles;
};

END_NCBI_SCOPE

#endif // GUI_UTILS__VBO_TRI_PERIMETER_HPP
