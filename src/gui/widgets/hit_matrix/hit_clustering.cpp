/*  $Id: hit_clustering.cpp 14565 2007-05-18 12:32:01Z dicuccio $
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

#include <ncbi_pch.hpp>

#include <gui/widgets/hit_matrix/hit_clustering.hpp>
#include <gui/widgets/hit_matrix/hit_matrix_graph.hpp>

#include <objects/seqloc/Na_strand.hpp>

BEGIN_NCBI_SCOPE
USING_SCOPE(objects);


/*CHitCluster::~CHitCluster()
{
    for( size_t i = 0;  i < m_Elems.size();  i++ )  {
        delete m_Clusters[i];
    }
} */

// the resolution is specified by two scales and threshould (in pixels)

// takes two vectors
// calculate distance vector in model units
// project it to pixels using given scales
// if distance is below threshold
 // calculate combined vector
 // calculate normals in model units
 // calculate normals in pixels
 // if normals good - produce a cluster, remember results

double CHitClustering::m_MinSize = 2.0;
double CHitClustering::m_Width = 4.0;

// get the distance^2 in pixels between two points
inline double PixDistanceSquare(const TVect& a, const TVect& b,
                            double scale_x, double scale_y)
{
    TVect res(a);
    res -= b;
    double d_x = res.X() / scale_x;
    double d_y = res.Y() / scale_y;
    return d_x * d_x + d_y * d_y;
}


inline  double CalcDistanceSquare(const TVector& a, const TVector& b, double scale_x, double scale_y)
{
    double min_d = PixDistanceSquare(a.m_From, b.m_From, scale_x, scale_y);
    min_d = min(min_d, PixDistanceSquare(a.m_To, b.m_From, scale_x, scale_y));
    min_d = min(min_d, PixDistanceSquare(a.m_From, b.m_To, scale_x, scale_y));
    min_d = min(min_d, PixDistanceSquare(a.m_To, b.m_To, scale_x, scale_y));
}


inline void GetVector(const CHitElemGlyph& glyph, TVector& v)
{
    const IHitElement& elem = glyph.GetHitElem();
    double x1 = elem.GetSubjectStart();
    double y1 = elem.GetQueryStart();
    double l = elem.GetLength();
    double x2 = x1 + l;
    double y2 = y1 + l;

    if(elem.GetSubjectStrand() == eNa_strand_minus)    {
        swap(x1, x2);
    }
    if(elem.GetQueryStrand() == eNa_strand_minus)    {
        swap(y1, y2);
    }
    v.m_From.X() = x1;
    v.m_From.Y() = y1;
    v.m_To.X() = x2;
    v.m_To.Y() = y2;
}


// calculates a vector normal to "res"
inline TVect   GetNorm(const TVector& L, const TVector& v)
{
    double L_x = L.m_To.X() - L.m_From.X();
    double L_y = L.m_To.Y() - L.m_From.Y();
    double v_x = v.m_To.X() - v.m_From.X();
    double v_y = v.m_To.Y() - v.m_From.Y();

    TVect norm;

    double s = L_x * L_x + L_y * L_y;
    double k = -(L_x * v_x + L_y * v_y) / s;

    norm.X() = k * L_x + v_x;
    norm.Y() = k * L_y + v_y;
    return norm;
}


// check whether two vectors can be combined into one cluster
bool CHitClustering::x_CanCombine(const TVector& a, const TVector& b, TVector& res,
                                  double scale_x, double scale_y)
{
    // locate closest points
    double min_0 = PixDistanceSquare(a.m_From, b.m_From, scale_x, scale_y);
    double min_1 = PixDistanceSquare(a.m_To, b.m_From, scale_x, scale_y);
    double min_2 = PixDistanceSquare(a.m_From, b.m_To, scale_x, scale_y);
    double min_3 = PixDistanceSquare(a.m_To, b.m_To, scale_x, scale_y);

    unsigned min_i = 0;
    double min = min_0;
    if(min_1 < min) {
        min = min_1;
        min_i = 1;
    }
    if(min_2 < min) {
        min = min_2;
        min_i = 2;
    }
    if(min_3 < min) {
        min = min_3;
        min_i = 3;
    }
    if(min < m_MinSize * m_MinSize) {
        // calculate combined Lvector
        TVector v_L;
        v_L.m_From = (min_i & 0x1) ? a.m_From : a.m_To;
        v_L.m_To = (min_i & 0x2) ? b.m_From : b.m_To;

        double width = m_Width / 2; // width of the line in pixels

        TVect norm = GetNorm(v_L, a);
        double h_1 = norm.X() / scale_x + norm.Y() / scale_y;
        if(h_1 < width) {
            norm = GetNorm(v_L, b);
            double h_2 = norm.X() / scale_x + norm.Y() / scale_y;

            if(h_2 < width) {
                // ok, we can merge them
                res = v_L;
                return true;
            }
        }
    }
    return false;
}


void CHitClustering::Build(vector<CHitGlyph*>& glyphs, double scale_x, double scale_y)
{
    CStopWatch w;
    w.Start();

    Clear();

    vector<CHitElemGlyph*> gl_acc; //accumulator

    //iterate by hits
    for( size_t i = 0;  i < glyphs.size();  i++ )   {
        CHitGlyph& gl = *glyphs[i];

        vector<CHitElemGlyph>& elems = gl.GetElems();
        if(elems.size())   {
            TVector a, b, res;
            CHitElemGlyph& gl_a = elems[0];
            GetVector(gl_a, a);
            gl_acc.push_back(&gl_a); // see the accumulator

            // iterate by hit elements
            for( size_t e_i = 1;  e_i < elems.size();  e_i++ ) {
                CHitElemGlyph& gl_b = elems[e_i];
                GetVector(gl_b, b);

                if(x_CanCombine(a, b, res, scale_x, scale_y))    { // accumulate
                    a = res;
                } else {
                    if(gl_acc.size() > 1)   { // create a cluster
                        CHitCluster* cl = new CHitCluster(gl_acc, res);
                        m_Clusters.push_back(cl);
                    }

                    // reset
                    a = b;
                    gl_acc.clear();
                }
                gl_acc.push_back(&gl_b);
            }

            if(gl_acc.size() > 1)   { // create a cluster
                CHitCluster* cl = new CHitCluster(gl_acc, res);
                m_Clusters.push_back(cl);
                gl_acc.clear();
            }
        }
    }

    //LOG_POST(Info << "CHitClustering::Build() time " << w.Elapsed());
}


void CHitClustering::Clear()
{
    for( size_t i = 0;  i < m_Clusters.size(); i++ ) {
        delete m_Clusters[i];
    }
    m_Clusters.clear();
}


END_NCBI_SCOPE
