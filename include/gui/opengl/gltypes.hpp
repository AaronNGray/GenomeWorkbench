#ifndef GUI_GRAPH___GLTYPES__HPP
#define GUI_GRAPH___GLTYPES__HPP

/*  $Id: gltypes.hpp 18740 2009-02-05 14:09:52Z wuliangs $
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

#include <gui/opengl.h>
#include <gui/opengl/glrect.hpp>
#include <util/range.hpp>

/** @addtogroup GUI_OPENGL
 *
 * @{
 */

BEGIN_NCBI_SCOPE

// units for coordinates representation on the output device (pixels) and in the model space
typedef int      TVPUnit;
typedef GLdouble TModelUnit;

typedef CGlPoint<TVPUnit>    TVPPoint;
typedef CGlPoint<TModelUnit> TModelPoint;

typedef CGlRect<TVPUnit>    TVPRect;
typedef CGlRect<TModelUnit> TModelRect;

typedef CRange<TModelUnit>  TModelRange;

enum    EOrientation    {
    eHorz,
    eVert
};


END_NCBI_SCOPE

/* @} */

#endif
