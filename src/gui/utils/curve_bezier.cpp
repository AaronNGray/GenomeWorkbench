/*  $Id: curve_bezier.cpp 15168 2007-10-15 19:11:34Z dicuccio $
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

#include <ncbi_pch.hpp>
#include <gui/utils/curve_bezier.hpp>
#include <util/math/matrix.hpp>

BEGIN_NCBI_SCOPE

CCurveBezier::CCurveBezier()
    : m_Error(0.002f)
{
}

///
/// evaluate the parameter matrix of a given curve
///
void CCurveBezier::Recalc()
{
    static const CMatrix4<float> basis(-1.0f,  3.0f, -3.0f,  1.0f,
                                        3.0f, -6.0f,  3.0f,  0.0f,
                                       -3.0f,  3.0f,  0.0f,  0.0f,
                                        1.0f,  0.0f,  0.0f,  0.0f);

    m_ParamMatrix = basis * m_Points;
}


const CCurveBezier::TControlPoints& CCurveBezier::x_ParamMatrix() const
{
    return m_ParamMatrix;
}


END_NCBI_SCOPE
