#ifndef GUI_OPENGL___RASTERIZER__HPP
#define GUI_OPENGL___RASTERIZER__HPP

/*  $Id: rasterizer.hpp 44954 2020-04-27 17:57:36Z evgeniev $
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


#include <corelib/ncbistd.hpp>

#include <gui/gui.hpp>
#include <gui/opengl/glpane.hpp>
#include <math.h>

/** @addtogroup GUI_OPENGL
 *
 * @{
 */

BEGIN_NCBI_SCOPE


/////////////////////////////////////////////////////////////////////////////
// CRasterizer
template<class A>   class CRasterizer
{
public:
    typedef vector<A>   TRaster;

    CRasterizer(TVPUnit vp_min, TVPUnit vp_max, TModelUnit min, TModelUnit max)
        :   m_VPMin(vp_min), m_VPMax(vp_max),
            m_Min(min),   m_Max(max)
    {
        TVPUnit vp_len  = vp_max - vp_min;
        _ASSERT(vp_len >= 0);
        if (vp_len <= 0) vp_len = 1;
        m_Raster.resize(vp_len + 1);
        m_Scale = (max - min) / vp_len;
    }

    template <class F>  void    AddInterval(TModelUnit min, TModelUnit max, const A& attr, F f_add)
    {
        TModelUnit pix_from = (min - m_Min) / m_Scale;
        TModelUnit pix_to_open = (max - m_Min) / m_Scale;

        int vp_len = (int)(m_Raster.size() - 1);
        bool intersect = ! (pix_to_open <= 0  ||  pix_from >= vp_len);

        if(intersect)   {
            // clip by raster index extent
            pix_from = std::max(TModelUnit(0), pix_from);
            pix_to_open = std::min(TModelUnit(vp_len), pix_to_open);

            TModelUnit pix_left = floor(pix_from);
            TModelUnit pix_right = std::max(TModelUnit(0), ceil(pix_to_open) - 1);
            if (pix_to_open - pix_from < 1.0  &&  pix_left != pix_right) {
                pix_right = pix_left;
            }
            int left_index = (int) pix_left;
            int right_index = (int) pix_right;

            if(left_index == right_index)   {
                // everything fits withing a single pixel
                _ASSERT(left_index < (int) m_Raster.size());
                float fraction = float(pix_to_open - pix_from);
                f_add(m_Raster[left_index], attr, fraction, false);
            } else {
                if(pix_left < pix_from) {
                    _ASSERT(left_index < (int) m_Raster.size());
                    float fraction = float(pix_left + 1 - pix_from);
                    f_add(m_Raster[left_index], attr, fraction, true);
                    left_index++;
                }
                if(pix_right < pix_to_open) {
                    _ASSERT(right_index < (int) m_Raster.size());
                    float fraction = float(pix_to_open - pix_right);
                    f_add(m_Raster[right_index], attr, fraction, true);
                    right_index--;
                }
                for(int i = left_index;  i <= right_index;  i++ )    {
                    _ASSERT(i < (int) m_Raster.size());
                    f_add(m_Raster[i], attr, 1.0, true);
                }
            }
        }
    }

    TModelUnit Project(TModelUnit pos) const
    {
        return (pos - m_Min) / m_Scale;
    }

    const TRaster& GetRaster() const    {   return m_Raster;    }

protected:
    TVPUnit m_VPMin;
    TVPUnit m_VPMax;
    TModelUnit  m_Min;
    TModelUnit  m_Max;
    TModelUnit  m_Scale;

    TRaster     m_Raster;
};


END_NCBI_SCOPE

/* @} */

#endif  // GUI_OPENGL___RASTERIZER__HPP
