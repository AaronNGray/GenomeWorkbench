#ifndef GUI_UTILS__BBOX_HPP
#define GUI_UTILS__BBOX_HPP

/*  $Id: bbox.hpp 27010 2012-12-07 16:37:16Z falkrb $
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
 * Authors:  Peter Meric
 *
 * File Description:
 *   CBBox - represents (and calculates) a bounding box
 *
 */

/** @addtogroup GUI_UTILS
*
* @{
*/

#include <corelib/ncbistd.hpp>

#include <gui/utils/vect2.hpp>
#include <gui/utils/vect3.hpp>

BEGIN_NCBI_SCOPE

// BBox represents a simple, rectangular bounding box
//
template <int N>
class CBBox
{
public:
    enum { X, Y, Z };

    CBBox()
        : m_IsSet(false)
    {
        for (unsigned int i = 0; i < N; ++i) {
            lower[i] = upper[i] = 0.0f;
        }
    }


    virtual ~CBBox()
    {
    }


    void Add(const CBBox<N>& bbox)
    {
        Add(bbox.lower);
        Add(bbox.upper);
    }

    void Add(const CVect2<float>& pt)
    {
        unsigned int dim = 2;
        if (N < 2)
            dim = N;

        if (!m_IsSet) {
            for (unsigned int i = 0; i < dim; ++i) {
                lower[i] = upper[i] = pt[i];
            }
            m_IsSet = true;
        }
        else {
            for (unsigned int i = 0; i < dim; ++i) {
                if (pt[i] < lower[i]) {
                    lower[i] = pt[i];
                }
                else if (pt[i] > upper[i]) {
                    upper[i] = pt[i];     
                }
            }
        }
    }

    void Add(const CVect3<float>& pt)
    {
        unsigned int dim = 3;
        if (N < 3)
            dim = N;

        if (!m_IsSet) {
            for (unsigned int i = 0; i < dim; ++i) {
                lower[i] = upper[i] = pt[i];
            }
            m_IsSet = true;
        }
        else {
            for (unsigned int i = 0; i < dim; ++i) {
                if (pt[i] < lower[i]) {
                    lower[i] = pt[i];
                }
                else if (pt[i] > upper[i]) {
                    upper[i] = pt[i];     
                }
            }
        }
    }


    //
    // NumSets is the number of sets of points
    // Stride is the distance between the start of one set and the next
    //
    void Add(const float vals[N], int num_sets = 1, int stride = 0)
    {
        unsigned int offset = 0;

        while (num_sets-- > 0) {
            if (!m_IsSet) {
                for (unsigned int i = 0; i < N; ++i) {
                    unsigned int idx = i + offset;
                    lower[i] = upper[i] = vals[idx];
                }
                m_IsSet = true;
            }
            else {
                for (unsigned int i = 0; i < N; ++i) {
                    unsigned int idx = i + offset;
                    if (vals[idx] < lower[i]) {
                        lower[i] = vals[idx];
                    }
                    else if (vals[idx] > upper[i]) {
                        upper[i] = vals[idx];
                    }
                }
            }

            offset += stride;
        }
    }


    pair<float, float> GetNthRange(unsigned int n) const
    {
        return make_pair(lower[n], upper[n]);
    }


    virtual void PrintTo(CNcbiOstream& strm) const
    {
        strm << '[';

        for (unsigned int i = 0; i < N; ++i) {
            if (i > 0) {
                strm << ", ";
            }
            strm << '(' << lower[i] << ", " << upper[i] << ')';
        }
        strm << ']';
    }


private:
    bool m_IsSet;
    float lower[N], upper[N];
};


inline CNcbiOstream& operator<<(CNcbiOstream& strm, const CBBox<3>& bb)
{
    bb.PrintTo(strm);
    return strm;
}


END_NCBI_SCOPE

/* @} */

#endif  // GUI_UTILS__BBOX_HPP
