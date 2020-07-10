#ifndef GUI_MATH___MATH___HPP
#define GUI_MATH___MATH___HPP

/*  $Id: math.hpp 15698 2008-01-14 21:06:16Z dicuccio $
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

#include <corelib/ncbistd.hpp>
#include <math.h>

/** @addtogroup GUI_MATH
 *
 * @{
 */

BEGIN_NCBI_SCOPE
BEGIN_SCOPE(math)

//
// some useful constants
//
#ifdef M_PI
const float pi = (float)(M_PI);
#else
const float pi = 3.1415927f;
#endif


//
// epsilon - some tiny number
//
const float epsilon = 1e-6f;


END_SCOPE(math)
END_NCBI_SCOPE

/* @} */

#endif  // GUI_MATH___MATH___HPP
