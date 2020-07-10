#ifndef GUI_MATH___CURVE_CUBIC__HPP
#define GUI_MATH___CURVE_CUBIC__HPP

/*  $Id: curve_cubic.hpp 23872 2011-06-16 13:18:48Z wuliangs $
 * ===========================================================================
 *
 *                            PUBLIC DOMAIN NOTICE
 *               National Center for Biotechnology Information
 *
 *  This software / database is a "United States Government Work" under the
 *  terms of the United States Copyright Act.  It was written as part of
 *  the author's official duties as a United States Government employee and
 *  thus cannot be copyrighted.  This software / database is freely available
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
 *
 */

#include <gui/utils/curve.hpp>

/** @addtogroup GUI_MATH
 *
 * @{
 */

BEGIN_NCBI_SCOPE



class NCBI_GUIUTILS_EXPORT CCurveCubic : public ICurve
{
public:
    typedef CVect4<TPoint> TControlPoints;

    CCurveCubic();

    /// recalculate the curve (curve-specific)
    void    Recalc();

    /// access a control point
    const TPoint& GetPoint(size_t i) const;
    TPoint&       SetPoint(size_t i);
    void          SetPoint(size_t i, const TPoint& point);

    //// evaluate the current curve at a given value [0, 1]
    TPoint EvalPos  (float /*u*/) const;
    TPoint EvalTan  (float /*u*/) const;
    TPoint EvalCurve(float /*u*/) const;

    /// access the error
    float GetError(void) const;
    void  SetError(float f);


protected:
    /// the control points
    CVect4< TPoint > m_Points;

    /// error
    float m_Error;

};


inline
const CCurveCubic::TPoint& CCurveCubic::GetPoint(size_t i) const
{
    _ASSERT(i < 4);
    return m_Points[i];
}


inline
CCurveCubic::TPoint& CCurveCubic::SetPoint(size_t i)
{
    _ASSERT(i < 4);
    return m_Points[i];
}


inline
void CCurveCubic::SetPoint(size_t i, const TPoint& point)
{
    _ASSERT(i < 4);
    m_Points[i] = point;
}


inline
float CCurveCubic::GetError() const
{
    return m_Error;
}


inline
void CCurveCubic::SetError(float f)
{
    m_Error = f;
}



END_NCBI_SCOPE

/* @} */

#endif  // GUI_MATH___CURVE_CUBIC__HPP
