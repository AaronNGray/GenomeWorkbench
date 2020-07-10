#ifndef __GUI_WIDGETS_HIT_MATRIX___HIT_CLUSTERING__HPP
#define __GUI_WIDGETS_HIT_MATRIX___HIT_CLUSTERING__HPP

/*  $Id: hit_clustering.hpp 14973 2007-09-14 12:11:32Z dicuccio $
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

#include <corelib/ncbistl.hpp>

#include <gui/utils/vect2.hpp>

BEGIN_NCBI_SCOPE

class CHitGlyph;
class CHitElemGlyph;

typedef CVect2<double>  TVect;

struct TVector {
    TVect   m_From;
    TVect   m_To;

    TVector() : m_From(0, 0), m_To(0, 0)    {}
    TVector(double from_x, double from_y, double to_x, double to_y)
        :   m_From(from_x, from_y), m_To(to_x, to_y)    {}
};


///////////////////////////////////////////////////////////////////////////////
///
class CHitCluster
{
public:
    typedef vector<CHitElemGlyph*>  TElemVector;
    //typedef vector<CHitCluster*>    TClusterVector;

    CHitCluster(const TElemVector& elems, const TVector v)
        : m_Elems(elems), m_Vector(v)   {}
    //~CHitCluster()

public:
    // suboptimal, replace with pointers?
    TElemVector        m_Elems;
    //TClusterVector     m_Clusters; // subclusters

    TVector m_Vector;
};


///////////////////////////////////////////////////////////////////////////////
///
class CHitClustering
{
public:
    typedef vector<CHitCluster*>    TClusterVector;

    ~CHitClustering()
    {
        Clear();
    }

    void    Build(vector<CHitGlyph*>& glyphs, double scale_x, double scale_y);
    void    Clear();

    const TClusterVector&   GetClusters()   const   {   return m_Clusters;}
protected:
    bool    x_CanCombine(const TVector& a, const TVector& b, TVector& res,
                         double scale_x, double scale_y);

protected:
    static double   m_MinSize;
    static double   m_Width;

    TClusterVector  m_Clusters;
};


END_NCBI_SCOPE

#endif  // __GUI_WIDGETS_HIT_MATRIX___HIT_CLUSTERING__HPP
