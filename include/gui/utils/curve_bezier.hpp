#ifndef GUI_MATH___CURVE_BEZIER__HPP
#define GUI_MATH___CURVE_BEZIER__HPP

/*  $Id: curve_bezier.hpp 15930 2008-02-08 13:38:34Z dicuccio $
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

#include <gui/gui_export.h>
#include <gui/utils/curve.hpp>

/** @addtogroup GUI_MATH
 *
 * @{
 */

BEGIN_NCBI_SCOPE



class NCBI_GUIUTILS_EXPORT CCurveBezier : public ICurve
{
public:
    typedef CVect4<TPoint> TControlPoints;

    CCurveBezier();

    /// access a control point
    const TPoint& GetPoint(size_t i) const;
    TPoint&       SetPoint(size_t i);
    void          SetPoint(size_t i, const TPoint& point);

    //// evaluate the current curve at a given value [0, 1]
    TPoint EvalPos  (float u) const;
    TPoint EvalTan  (float u) const;
    TPoint EvalCurve(float u) const;

    /// recalculate the curve's parameter matrix
    void Recalc();

    /// access the error
    float GetError(void) const;
    void SetError(float f);

protected:
    /// error limit
    float m_Error;

    /// the control points of the curve
    TControlPoints m_Points;

    /// The parameter matrix.  This is a combination of the basis function and
    /// the control points
    TControlPoints m_ParamMatrix;

    /// evaluate the parameter matrix for the current control points
    const TControlPoints& x_ParamMatrix() const;
};


inline
const CCurveBezier::TPoint& CCurveBezier::GetPoint(size_t i) const
{
    _ASSERT(i < 4);
    return m_Points[i];
}


inline
CCurveBezier::TPoint& CCurveBezier::SetPoint(size_t i)
{
    _ASSERT(i < 4);
    return m_Points[i];
}


inline
void CCurveBezier::SetPoint(size_t i, const TPoint& point)
{
    _ASSERT(i < 4);
    m_Points[i] = point;
}


inline
float CCurveBezier::GetError() const
{
    return m_Error;
}


inline
void CCurveBezier::SetError(float f)
{
    m_Error = f;
}


inline CCurveBezier::TPoint
CCurveBezier::EvalPos(float u) const
{
    _ASSERT(u >= 0  &&  u <= 1);
    return CVect4<float>(u*u*u, u*u, u, 1) * x_ParamMatrix();
}


inline CCurveBezier::TPoint
CCurveBezier::EvalTan(float u) const
{
    _ASSERT(u >= 0  &&  u <= 1);
    return CVect4<float>(u*u, u, 1, 0) * x_ParamMatrix();
}


inline CCurveBezier::TPoint
CCurveBezier::EvalCurve(float u) const
{
    return CVect4<float>(u, 1, 0, 0) * x_ParamMatrix();
}


END_NCBI_SCOPE

/* @} */

#endif  // GUI_MATH___CURVE_BEZIER__HPP
