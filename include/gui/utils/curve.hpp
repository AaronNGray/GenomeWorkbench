#ifndef GUI_MATH___CURVE__HPP
#define GUI_MATH___CURVE__HPP

/*  $Id: curve.hpp 18080 2008-10-14 22:33:27Z yazhuk $
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

/** @addtogroup GUI_UTILS
*
* @{
*/

#include <corelib/ncbistd.hpp>
#include <gui/gui_export.h>
#include <gui/utils/vect3.hpp>


BEGIN_NCBI_SCOPE

/// class ICurve defines a basic interface for all curves.  This representation
/// is independent of rendering
///
class ICurve
{
public:
    typedef CVect3<float> TPoint;

    virtual ~ICurve() { }

    /// recalculate the curve (curve-specific)
    virtual void    Recalc() = 0;

    /// access control points of the curve
    virtual const TPoint& GetPoint(size_t i) const = 0;
    virtual TPoint&       SetPoint(size_t i) = 0;
    virtual void          SetPoint(size_t i, const TPoint& ) = 0;

    //// evaluate the current curve at a given value [0, 1]
    virtual TPoint EvalPos  (float u) const = 0;
    virtual TPoint EvalTan  (float u) const = 0;
    virtual TPoint EvalCurve(float u) const = 0;

    /// access the level of detail
    virtual float GetError() const = 0;
    virtual void  SetError(float f) = 0;
};


END_NCBI_SCOPE

/* @} */

#endif  // GUI_MATH___CURVE__HPP
