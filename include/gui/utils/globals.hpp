#ifndef GUI_MATH___GLOBALS__HPP
#define GUI_MATH___GLOBALS__HPP

/*  $Id: globals.hpp 14973 2007-09-14 12:11:32Z dicuccio $
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
#include <vector>

/** @addtogroup GUI_MATH
 *
 * @{
 */

BEGIN_NCBI_SCOPE

// predeclarations of the types we use
template <class T> class CVect2;
template <class T> class CVect3;
template <class T> class CVect4;
template <class T> class CMatrix3;
template <class T> class CMatrix4;

END_NCBI_SCOPE

// Promotion rules
// we keep these in a separate file because they're long
#include <gui/utils/promote.hpp>

#include <gui/utils/vect2.hpp>
#include <gui/utils/vect3.hpp>
#include <gui/utils/vect4.hpp>
#include <gui/utils/matrix3.hpp>
#include <gui/utils/matrix4.hpp>

// general utilities
#include <math.h>
#include <gui/utils/math.hpp>


BEGIN_NCBI_SCOPE

//
//
// Global operators
//
//



//
//
// CVect<> templates
//
//

//
// global addition: CVect*<> + CVect*<>
//
template <class T, class U>
inline CVect2< NCBI_PROMOTE(T,U) >
operator+ (const CVect2<T>& v1, const CVect2<U>& v2)
{
    return
        CVect2< NCBI_PROMOTE(T,U) >
        (v1[0]+v2[0], v1[1]+v2[1]);
}


template <class T, class U>
inline CVect3< NCBI_PROMOTE(T,U) >
operator+ (const CVect3<T>& v1, const CVect3<U>& v2)
{
    return
        CVect3< NCBI_PROMOTE(T,U) >
        (v1[0]+v2[0], v1[1]+v2[1], v1[2]+v2[2]);
}


template <class T, class U>
inline CVect4< NCBI_PROMOTE(T,U) >
operator+ (const CVect4<T>& v1, const CVect4<U>& v2)
{
    return
        CVect4< NCBI_PROMOTE(T,U) >
        (v1[0]+v2[0], v1[1]+v2[1], v1[2]+v2[2], v1[3]+v2[3]);
}


//
// global addition: const + CVect*<>
// we do this with macros because not all compilers support partial template
// specialization
//

#define NCBI_ADD_VECT2(type) \
template <class U> \
inline CVect2< NCBI_PROMOTE(type,U) > \
operator+ (type v1, const CVect2<U>& v2) \
{ \
    return \
        CVect2< NCBI_PROMOTE(type,U) > \
        (v1 + v2[0], v1 + v2[1]); \
} \
template <class T> \
inline CVect2< NCBI_PROMOTE(T,type) > \
operator+ (const CVect2<T>& v1, type v2) \
{ \
    return \
        CVect2< NCBI_PROMOTE(T,type) > \
        (v1[0] + v2, v1[1] + v2); \
}


NCBI_ADD_VECT2(int)
NCBI_ADD_VECT2(float)
NCBI_ADD_VECT2(double)

#undef NCBI_ADD_VECT2


#define NCBI_ADD_VECT3(type) \
template <class U> \
inline CVect3< NCBI_PROMOTE(type,U) > \
operator+ (type v1, const CVect3<U>& v2) \
{ \
    return \
        CVect3< NCBI_PROMOTE(type,U) > \
        (v1 + v2[0], v1 + v2[1], v1 + v2[2]); \
} \
template <class T> \
inline CVect3< NCBI_PROMOTE(T,type) > \
operator+ (const CVect3<T>& v1, type v2) \
{ \
    return \
        CVect3< NCBI_PROMOTE(T,type) > \
        (v1[0] + v2, v1[1] + v2, v1[2] + v2); \
}


NCBI_ADD_VECT3(int)
NCBI_ADD_VECT3(float)
NCBI_ADD_VECT3(double)

#undef NCBI_ADD_VECT3


#define NCBI_ADD_VECT4(type) \
template <class U> \
inline CVect4< NCBI_PROMOTE(type,U) > \
operator+ (type v1, const CVect4<U>& v2) \
{ \
    return \
        CVect4< NCBI_PROMOTE(type,U) > \
        (v1 + v2[0], v1 + v2[1], v1 + v2[2], v1 + v2[3]); \
} \
template <class T> \
inline CVect4< NCBI_PROMOTE(T,type) > \
operator+ (const CVect4<T>& v1, type v2) \
{ \
    return \
        CVect4< NCBI_PROMOTE(T,type) > \
        (v1[0] + v2, v1[1] + v2, v1[2] + v2, v1[3] + v2); \
}


NCBI_ADD_VECT4(int)
NCBI_ADD_VECT4(float)
NCBI_ADD_VECT4(double)

#undef NCBI_ADD_VECT4


//
// global unary negation
//
template <class T>
inline CVect2<T>
operator- (const CVect2<T>& v)
{
    return CVect2<T> (-v[0], -v[1]);
}


template <class T>
inline CVect3<T>
operator- (const CVect3<T>& v)
{
    return CVect3<T> (-v[0], -v[1], -v[2]);
}


template <class T>
inline CVect4<T>
operator- (const CVect4<T>& v)
{
    return CVect4<T> (-v[0], -v[1], -v[2], -v[3]);
}

//
// global subtraction: CVect*<> - CVect*<>
//
template <class T, class U>
inline CVect2< NCBI_PROMOTE(T,U) >
operator- (const CVect2<T>& v1, const CVect2<U>& v2)
{
    return
        CVect2< NCBI_PROMOTE(T,U) >
        (v1[0]-v2[0], v1[1]-v2[1]);
}


template <class T, class U>
inline CVect3< NCBI_PROMOTE(T,U) >
operator- (const CVect3<T>& v1, const CVect3<U>& v2)
{
    return
        CVect3< NCBI_PROMOTE(T,U) >
        (v1[0]-v2[0], v1[1]-v2[1], v1[2]-v2[2]);
}


template <class T, class U>
inline CVect4< NCBI_PROMOTE(T,U) >
operator- (const CVect4<T>& v1, const CVect4<U>& v2)
{
    return
        CVect4< NCBI_PROMOTE(T,U) >
        (v1[0]-v2[0], v1[1]-v2[1], v1[2]-v2[2], v1[3]-v2[3]);
}


//
// global subtraction: const + CVect*<>
// we do this with macros because not all compilers support partial template
// specialization
//

#define NCBI_SUBTRACT_VECT2(type) \
template <class U> \
inline CVect2< NCBI_PROMOTE(type,U) > \
operator- (type v1, const CVect2<U>& v2) \
{ \
    return \
        CVect2< NCBI_PROMOTE(type,U) > \
        (v1 - v2[0], v1 - v2[1]); \
} \
template <class T> \
inline CVect2< NCBI_PROMOTE(T,type) > \
operator- (const CVect2<T>& v1, type v2) \
{ \
    return \
        CVect2< NCBI_PROMOTE(T,type) > \
        (v1[0] - v2, v1[1] - v2); \
}


NCBI_SUBTRACT_VECT2(int)
NCBI_SUBTRACT_VECT2(float)
NCBI_SUBTRACT_VECT2(double)

#undef NCBI_SUBTRACT_VECT2


#define NCBI_SUBTRACT_VECT3(type) \
template <class U> \
inline CVect3< NCBI_PROMOTE(type,U) > \
operator- (type v1, const CVect3<U>& v2) \
{ \
    return \
        CVect3< NCBI_PROMOTE(type,U) > \
        (v1 - v2[0], v1 - v2[1], v1 - v2[2]); \
} \
template <class T> \
inline CVect3< NCBI_PROMOTE(T,type) > \
operator- (const CVect3<T>& v1, type v2) \
{ \
    return \
        CVect3< NCBI_PROMOTE(T,type) > \
        (v1[0] - v2, v1[1] - v2, v1[2] - v2); \
}


NCBI_SUBTRACT_VECT3(int)
NCBI_SUBTRACT_VECT3(float)
NCBI_SUBTRACT_VECT3(double)

#undef NCBI_SUBTRACT_VECT3


#define NCBI_SUBTRACT_VECT4(type) \
template <class U> \
inline CVect4< NCBI_PROMOTE(type,U) > \
operator- (type v1, const CVect4<U>& v2) \
{ \
    return \
        CVect4< NCBI_PROMOTE(type,U) > \
        (v1 - v2[0], v1 - v2[1], v1 - v2[2], v1 - v2[3]); \
} \
template <class T> \
inline CVect4< NCBI_PROMOTE(T,type) > \
operator- (const CVect4<T>& v1, type v2) \
{ \
    return \
        CVect4< NCBI_PROMOTE(T,type) > \
        (v1[0] - v2, v1[1] - v2, v1[2] - v2, v1[3] - v2); \
}


NCBI_SUBTRACT_VECT4(int)
NCBI_SUBTRACT_VECT4(float)
NCBI_SUBTRACT_VECT4(double)

#undef NCBI_SUBTRACT_VECT4



//
// global subtraction: const + CVect*<>
// we do this with macros because not all compilers support partial template
// specialization
//

#define NCBI_MULTIPLY_VECT2(type) \
template <class U> \
inline CVect2< NCBI_PROMOTE(type,U) > \
operator* (type v1, const CVect2<U>& v2) \
{ \
    return \
        CVect2< NCBI_PROMOTE(type,U) > \
        (v1 * v2[0], v1 * v2[1]); \
} \
template <class T> \
inline CVect2< NCBI_PROMOTE(T,type) > \
operator* (const CVect2<T>& v1, type v2) \
{ \
    return \
        CVect2< NCBI_PROMOTE(T,type) > \
        (v1[0] * v2, v1[1] * v2); \
}


NCBI_MULTIPLY_VECT2(int)
NCBI_MULTIPLY_VECT2(float)
NCBI_MULTIPLY_VECT2(double)

#undef NCBI_MULTIPLY_VECT2


#define NCBI_MULTIPLY_VECT3(type) \
template <class U> \
inline CVect3< NCBI_PROMOTE(type,U) > \
operator* (type v1, const CVect3<U>& v2) \
{ \
    return \
        CVect3< NCBI_PROMOTE(type,U) > \
        (v1 * v2[0], v1 * v2[1], v1 * v2[2]); \
} \
template <class T> \
inline CVect3< NCBI_PROMOTE(T,type) > \
operator* (const CVect3<T>& v1, type v2) \
{ \
    return \
        CVect3< NCBI_PROMOTE(T,type) > \
        (v1[0] * v2, v1[1] * v2, v1[2] * v2); \
}


NCBI_MULTIPLY_VECT3(int)
NCBI_MULTIPLY_VECT3(float)
NCBI_MULTIPLY_VECT3(double)

#undef NCBI_MULTIPLY_VECT3


#define NCBI_MULTIPLY_VECT4(type) \
template <class U> \
inline CVect4< NCBI_PROMOTE(type,U) > \
operator* (type v1, const CVect4<U>& v2) \
{ \
    return \
        CVect4< NCBI_PROMOTE(type,U) > \
        (v1 * v2[0], v1 * v2[1], v1 * v2[2], v1 * v2[3]); \
} \
template <class T> \
inline CVect4< NCBI_PROMOTE(T,type) > \
operator* (const CVect4<T>& v1, type v2) \
{ \
    return \
        CVect4< NCBI_PROMOTE(T,type) > \
        (v1[0] * v2, v1[1] * v2, v1[2] * v2, v1[3] * v2); \
}


NCBI_MULTIPLY_VECT4(int)
NCBI_MULTIPLY_VECT4(float)
NCBI_MULTIPLY_VECT4(double)

#undef NCBI_MULTIPLY_VECT4





//
// global multiplication: CVect*<> * CVect*<>
// we define this as dot!
//
template <class T, class U>
inline NCBI_PROMOTE(T,U)
operator* (const CVect2<T>& v1, const CVect2<U>& v2)
{
    return (v1[0] * v2[0] +
            v1[1] * v2[1]);
}


template <class T, class U>
inline NCBI_PROMOTE(T,U)
operator* (const CVect3<T>& v1, const CVect3<U>& v2)
{
    return (v1[0] * v2[0] +
            v1[1] * v2[1] +
            v1[2] * v2[2]);
}


template <class T, class U>
inline NCBI_PROMOTE(T,U)
operator* (const CVect4<T>& v1, const CVect4<U>& v2)
{
    return (v1[0] * v2[0] +
            v1[1] * v2[1] +
            v1[2] * v2[2] +
            v1[3] * v2[3]);
}



//
// global division: CVect*<> / const
//
template <class T, class U>
inline CVect2< NCBI_PROMOTE(T,U) >
operator/ (const CVect2<T>& v1, U v2)
{
    v2 = T(1) / v2;
    return
        CVect2< NCBI_PROMOTE(T,U) >
        (v1[0]*v2, v1[1]*v2);
}


template <class T, class U>
inline CVect3< NCBI_PROMOTE(T,U) >
operator/ (const CVect3<T>& v1, U v2)
{
    v2 = T(1) / v2;
    return
        CVect3< NCBI_PROMOTE(T,U) >
        (v1[0]*v2, v1[1]*v2, v1[2]*v2);
}


template <class T, class U>
inline CVect4< NCBI_PROMOTE(T,U) >
operator/ (const CVect4<T>& v1, U v2)
{
    v2 = T(1) / v2;
    return
        CVect4< NCBI_PROMOTE(T,U) >
        (v1[0]*v2, v1[1]*v2, v1[2]*v2, v1[3]*v2);
}


//
// global comparison: equals
//
template <class T, class U>
inline bool
operator== (const CVect2<T>& v1, const CVect2<U>& v2)
{
    return (v1[0] == v2[0]  &&
            v1[1] == v2[1]);
}


template <class T, class U>
inline bool
operator== (const CVect3<T>& v1, const CVect3<U>& v2)
{
    return (v1[0] == v2[0]  &&
            v1[1] == v2[1]  &&
            v1[2] == v2[2]);
}


template <class T, class U>
inline bool
operator== (const CVect4<T>& v1, const CVect4<U>& v2)
{
    return (v1[0] == v2[0]  &&
            v1[1] == v2[1]  &&
            v1[2] == v2[2]  &&
            v1[3] == v2[3]);
}


//
// global comparison: less than
//
template <class T, class U>
inline bool
operator< (const CVect2<T>& v1, const CVect2<U>& v2)
{
    if (v1[0] < v2[0]) {
        return true;
    } else if (v1[0] > v2[0]) {
        return false;
    }

    if (v1[1] < v2[1]) {
        return true;
    }

    return false;
}


template <class T, class U>
inline bool
operator< (const CVect3<T>& v1, const CVect3<U>& v2)
{
    if (v1[0] < v2[0]) {
        return true;
    } else if (v1[0] > v2[0]) {
        return false;
    }

    if (v1[1] < v2[1]) {
        return true;
    } else if (v1[1] > v2[1]) {
        return false;
    }

    if (v1[2] < v2[2]) {
        return true;
    }

    return false;
}


template <class T, class U>
inline bool
operator< (const CVect4<T>& v1, const CVect4<U>& v2)
{
    if (v1[0] < v2[0]) {
        return true;
    } else if (v1[0] > v2[0]) {
        return false;
    }

    if (v1[1] < v2[1]) {
        return true;
    } else if (v1[1] > v2[1]) {
        return false;
    }

    if (v1[2] < v2[2]) {
        return true;
    } else if (v1[2] > v2[2]) {
        return false;
    }

    if (v1[3] < v2[3]) {
        return true;
    }

    return false;
}


//
//
//
// Matrices
//
//
//


//
// global addition: matrix + matrix
//
template <class T, class U>
inline CMatrix3< NCBI_PROMOTE(T,U) >
operator+ (const CMatrix3<T>& m1, const CMatrix3<U>& m2)
{
    return
        CMatrix3< NCBI_PROMOTE(T,U) >
        (m1[0]+m2[0], m1[1]+m2[1], m1[2]+m2[2],
         m1[3]+m2[3], m1[4]+m2[4], m1[5]+m2[5],
         m1[6]+m2[6], m1[7]+m2[7], m1[8]+m2[8]);
}


template <class T, class U>
inline CMatrix4< NCBI_PROMOTE(T,U) >
operator+ (const CMatrix4<T>& m1, const CMatrix4<U>& m2)
{
    return
        CMatrix4< NCBI_PROMOTE(T,U) >
        (m1[ 0]+m2[ 0], m1[ 1]+m2[ 1], m1[ 2]+m2[ 2], m1[ 3]+m2[ 3],
         m1[ 4]+m2[ 4], m1[ 5]+m2[ 5], m1[ 6]+m2[ 6], m1[ 7]+m2[ 7],
         m1[ 8]+m2[ 8], m1[ 9]+m2[ 9], m1[10]+m2[10], m1[11]+m2[11],
         m1[12]+m2[12], m1[13]+m2[13], m1[14]+m2[14], m1[15]+m2[15]);
}


//
// global addition: scalar + matrix
//
#define NCBI_ADD_MATRIX3(type) \
template <class U> \
inline CMatrix3< NCBI_PROMOTE(type,U) > \
operator+ (type s, const CMatrix3<U>& m) \
{ \
    return \
        CMatrix3< NCBI_PROMOTE(type,U) > \
        (m[0]+s, m[1]+s, m[2]+s, \
         m[3]+s, m[4]+s, m[5]+s, \
         m[6]+s, m[7]+s, m[8]+s); \
} \
template <class T> \
inline CMatrix3< NCBI_PROMOTE(T,type) > \
operator+ (const CMatrix3<T>& m, type s) \
{ \
    return \
        CMatrix3< NCBI_PROMOTE(T,type) > \
        (m[0]+s, m[1]+s, m[2]+s, \
         m[3]+s, m[4]+s, m[5]+s, \
         m[6]+s, m[7]+s, m[8]+s); \
}


NCBI_ADD_MATRIX3(int)
NCBI_ADD_MATRIX3(float)
NCBI_ADD_MATRIX3(double)

#undef NCBI_ADD_MATRIX3


#define NCBI_ADD_MATRIX4(type) \
template <class U> \
inline CMatrix4< NCBI_PROMOTE(type,U) > \
operator+ (type s, const CMatrix4<U>& m) \
{ \
    return \
        CMatrix4< NCBI_PROMOTE(type,U) > \
        (m[ 0]+s, m[ 1]+s, m[ 2]+s, m[ 3]+s,  \
         m[ 4]+s, m[ 5]+s, m[ 6]+s, m[ 7]+s,  \
         m[ 8]+s, m[ 9]+s, m[10]+s, m[11]+s,  \
         m[12]+s, m[13]+s, m[14]+s, m[15]+s); \
} \
template <class T> \
inline CMatrix4< NCBI_PROMOTE(T,type) > \
operator+ (const CMatrix4<T>& m, type s) \
{ \
    return \
        CMatrix4< NCBI_PROMOTE(T,type) > \
        (m[ 0]+s, m[ 1]+s, m[ 2]+s, m[ 3]+s,  \
         m[ 4]+s, m[ 5]+s, m[ 6]+s, m[ 7]+s,  \
         m[ 8]+s, m[ 9]+s, m[10]+s, m[11]+s,  \
         m[12]+s, m[13]+s, m[14]+s, m[15]+s); \
}

NCBI_ADD_MATRIX4(int)
NCBI_ADD_MATRIX4(float)
NCBI_ADD_MATRIX4(double)

#undef NCBI_ADD_MATRIX3


//
// global addition: scalar - matrix
//
#define NCBI_SUBTRACT_MATRIX3(type) \
template <class U> \
inline CMatrix3< NCBI_PROMOTE(type,U) > \
operator- (type s, const CMatrix3<U>& m) \
{ \
    return \
        CMatrix3< NCBI_PROMOTE(type,U) > \
        (m[0]-s, m[1]-s, m[2]-s, \
         m[3]-s, m[4]-s, m[5]-s, \
         m[6]-s, m[7]-s, m[8]-s); \
} \
template <class T> \
inline CMatrix3< NCBI_PROMOTE(T,type) > \
operator- (const CMatrix3<T>& m, type s) \
{ \
    return \
        CMatrix3< NCBI_PROMOTE(T,type) > \
        (m[0]-s, m[1]-s, m[2]-s, \
         m[3]-s, m[4]-s, m[5]-s, \
         m[6]-s, m[7]-s, m[8]-s); \
}


NCBI_SUBTRACT_MATRIX3(int)
NCBI_SUBTRACT_MATRIX3(float)
NCBI_SUBTRACT_MATRIX3(double)

#undef NCBI_SUBTRACT_MATRIX3


#define NCBI_SUBTRACT_MATRIX4(type) \
template <class U> \
inline CMatrix4< NCBI_PROMOTE(type,U) > \
operator- (type s, const CMatrix4<U>& m) \
{ \
    return \
        CMatrix4< NCBI_PROMOTE(type,U) > \
        (m[ 0]-s, m[ 1]-s, m[ 2]-s, m[ 3]-s,  \
         m[ 4]-s, m[ 5]-s, m[ 6]-s, m[ 7]-s,  \
         m[ 8]-s, m[ 9]-s, m[10]-s, m[11]-s,  \
         m[12]-s, m[13]-s, m[14]-s, m[15]-s); \
} \
template <class T> \
inline CMatrix4< NCBI_PROMOTE(T,type) > \
operator- (const CMatrix4<T>& m, type s) \
{ \
    return \
        CMatrix4< NCBI_PROMOTE(T,type) > \
        (m[ 0]-s, m[ 1]-s, m[ 2]-s, m[ 3]-s,  \
         m[ 4]-s, m[ 5]-s, m[ 6]-s, m[ 7]-s,  \
         m[ 8]-s, m[ 9]-s, m[10]-s, m[11]-s,  \
         m[12]-s, m[13]-s, m[14]-s, m[15]-s); \
}

NCBI_SUBTRACT_MATRIX4(int)
NCBI_SUBTRACT_MATRIX4(float)
NCBI_SUBTRACT_MATRIX4(double)

#undef NCBI_SUBTRACT_MATRIX3


//
// global addition: scalar * matrix
//
#define NCBI_MULTIPLY_MATRIX3(type) \
template <class U> \
inline CMatrix3< NCBI_PROMOTE(type,U) > \
operator* (type s, const CMatrix3<U>& m) \
{ \
    return \
        CMatrix3< NCBI_PROMOTE(type,U) > \
        (m[0]*s, m[1]*s, m[2]*s, \
         m[3]*s, m[4]*s, m[5]*s, \
         m[6]*s, m[7]*s, m[8]*s); \
} \
template <class T> \
inline CMatrix3< NCBI_PROMOTE(T,type) > \
operator* (const CMatrix3<T>& m, type s) \
{ \
    return \
        CMatrix3< NCBI_PROMOTE(T,type) > \
        (m[0]*s, m[1]*s, m[2]*s, \
         m[3]*s, m[4]*s, m[5]*s, \
         m[6]*s, m[7]*s, m[8]*s); \
}


NCBI_MULTIPLY_MATRIX3(int)
NCBI_MULTIPLY_MATRIX3(float)
NCBI_MULTIPLY_MATRIX3(double)

#undef NCBI_MULTIPLY_MATRIX3


#define NCBI_MULTIPLY_MATRIX4(type) \
template <class U> \
inline CMatrix4< NCBI_PROMOTE(type,U) > \
operator* (type s, const CMatrix4<U>& m) \
{ \
    return \
        CMatrix4< NCBI_PROMOTE(type,U) > \
        (m[ 0]*s, m[ 1]*s, m[ 2]*s, m[ 3]*s,  \
         m[ 4]*s, m[ 5]*s, m[ 6]*s, m[ 7]*s,  \
         m[ 8]*s, m[ 9]*s, m[10]*s, m[11]*s,  \
         m[12]*s, m[13]*s, m[14]*s, m[15]*s); \
} \
template <class T> \
inline CMatrix4< NCBI_PROMOTE(T,type) > \
operator* (const CMatrix4<T>& m, type s) \
{ \
    return \
        CMatrix4< NCBI_PROMOTE(T,type) > \
        (m[ 0]*s, m[ 1]*s, m[ 2]*s, m[ 3]*s,  \
         m[ 4]*s, m[ 5]*s, m[ 6]*s, m[ 7]*s,  \
         m[ 8]*s, m[ 9]*s, m[10]*s, m[11]*s,  \
         m[12]*s, m[13]*s, m[14]*s, m[15]*s); \
}

NCBI_MULTIPLY_MATRIX4(int)
NCBI_MULTIPLY_MATRIX4(float)
NCBI_MULTIPLY_MATRIX4(double)

#undef NCBI_MULTIPLY_MATRIX3


//
// global subtraction: matrix - matrix
//
template <class T, class U>
inline CMatrix3< NCBI_PROMOTE(T,U) >
operator- (const CMatrix3<T>& m1, const CMatrix3<U>& m2)
{
    return
        CMatrix3< NCBI_PROMOTE(T,U) >
        (m1[0]-m2[0], m1[1]-m2[1], m1[2]-m2[2],
         m1[3]-m2[3], m1[4]-m2[4], m1[5]-m2[5],
         m1[6]-m2[6], m1[7]-m2[7], m1[8]-m2[8]);
}


template <class T, class U>
inline CMatrix4< NCBI_PROMOTE(T,U) >
operator- (const CMatrix4<T>& m1, const CMatrix4<U>& m2)
{
    return
        CMatrix4< NCBI_PROMOTE(T,U) >
        (m1[ 0]-m2[ 0], m1[ 1]-m2[ 1], m1[ 2]-m2[ 2], m1[ 3]-m2[ 3],
         m1[ 4]-m2[ 4], m1[ 5]-m2[ 5], m1[ 6]-m2[ 6], m1[ 7]-m2[ 7],
         m1[ 8]-m2[ 8], m1[ 9]-m2[ 9], m1[10]-m2[10], m1[11]-m2[11],
         m1[12]-m2[12], m1[13]-m2[13], m1[14]-m2[14], m1[15]-m2[15]);
}


//
// global multiplication: matrix * CVect*<>
// vector assumed to be a column vector!
//
template <class T, class U>
inline CVect3< NCBI_PROMOTE(T,U) >
operator* (const CMatrix3<T>& m, const CVect3<U>& v)
{
    return
        CVect3< NCBI_PROMOTE(T,U) >
        (m[0]*v[0] + m[1]*v[1] + m[2]*v[2],
         m[3]*v[0] + m[4]*v[1] + m[5]*v[2],
         m[6]*v[0] + m[7]*v[1] + m[8]*v[2] );
}


template <class T, class U>
inline CVect4< NCBI_PROMOTE(T,U) >
operator* (const CMatrix4<T>& m, const CVect4<U>& v)
{
    return
        CVect4< NCBI_PROMOTE(T,U) >
        (m[ 0]*v[0] + m[ 1]*v[1] + m[ 2]*v[2] + m[ 3]*v[3],
         m[ 4]*v[0] + m[ 5]*v[1] + m[ 6]*v[2] + m[ 7]*v[3],
         m[ 8]*v[0] + m[ 9]*v[1] + m[10]*v[2] + m[11]*v[3],
         m[12]*v[0] + m[13]*v[1] + m[14]*v[2] + m[15]*v[3] );
}

//
// global multiplication: CVect*<> * matrix
// vector assumed to be a row vector!
//
template <class T, class U>
inline CVect3< NCBI_PROMOTE(T,U) >
operator* (const CVect3<T>& v, const CMatrix3<U>& m)
{
    return
        CVect3< NCBI_PROMOTE(T,U) >
        (v[0]*m[0] + v[1]*m[3] + v[2]*m[6],
         v[0]*m[1] + v[1]*m[4] + v[2]*m[7],
         v[0]*m[2] + v[1]*m[5] + v[2]*m[8] );
}


template <class T, class U>
inline CVect4< NCBI_PROMOTE(T,U) >
operator* (const CVect4<T>& v, const CMatrix4<U>& m)
{
    return
        CVect4< NCBI_PROMOTE(T,U) >
        (v[0]*m[ 0] + v[1]*m[ 4] + v[2]*m[ 8] + m[12]*v[3],
         v[0]*m[ 1] + v[1]*m[ 5] + v[2]*m[ 9] + m[13]*v[3],
         v[0]*m[ 2] + v[1]*m[ 6] + v[2]*m[10] + m[14]*v[3],
         v[0]*m[ 3] + v[1]*m[ 7] + v[2]*m[11] + m[15]*v[3] );
}


//
// global multiplication: matrix + matrix
//
template <class T, class U>
inline CMatrix3< NCBI_PROMOTE(T,U) >
operator* (const CMatrix3<T>& m1, const CMatrix3<U>& m2)
{
    return
        CMatrix3< NCBI_PROMOTE(T,U) >
        (m1[0]*m2[0] + m1[1]*m2[3] + m1[2]*m2[6],
         m1[0]*m2[1] + m1[1]*m2[4] + m1[2]*m2[7],
         m1[0]*m2[2] + m1[1]*m2[5] + m1[2]*m2[8],

         m1[3]*m2[0] + m1[4]*m2[3] + m1[5]*m2[6],
         m1[3]*m2[1] + m1[4]*m2[4] + m1[5]*m2[7],
         m1[3]*m2[2] + m1[4]*m2[5] + m1[5]*m2[8],

         m1[6]*m2[0] + m1[7]*m2[3] + m1[8]*m2[6],
         m1[6]*m2[1] + m1[7]*m2[4] + m1[8]*m2[7],
         m1[6]*m2[2] + m1[7]*m2[5] + m1[8]*m2[8]);
}


template <class T, class U>
inline CMatrix4< NCBI_PROMOTE(T,U) >
operator* (const CMatrix4<T>& m1, const CMatrix4<U>& m2)
{
    return
        CMatrix4< NCBI_PROMOTE(T,U) >
        (m1[ 0]*m2[ 0] + m1[ 1]*m2[ 4] + m1[ 2]*m2[ 8] + m1[ 3]*m2[12],
         m1[ 0]*m2[ 1] + m1[ 1]*m2[ 5] + m1[ 2]*m2[ 9] + m1[ 3]*m2[13],
         m1[ 0]*m2[ 2] + m1[ 1]*m2[ 6] + m1[ 2]*m2[10] + m1[ 3]*m2[14],
         m1[ 0]*m2[ 3] + m1[ 1]*m2[ 7] + m1[ 2]*m2[11] + m1[ 3]*m2[15],

         m1[ 4]*m2[ 0] + m1[ 5]*m2[ 4] + m1[ 6]*m2[ 8] + m1[ 7]*m2[12],
         m1[ 4]*m2[ 1] + m1[ 5]*m2[ 5] + m1[ 6]*m2[ 9] + m1[ 7]*m2[13],
         m1[ 4]*m2[ 2] + m1[ 5]*m2[ 6] + m1[ 6]*m2[10] + m1[ 7]*m2[14],
         m1[ 4]*m2[ 3] + m1[ 5]*m2[ 7] + m1[ 6]*m2[11] + m1[ 7]*m2[15],

         m1[ 8]*m2[ 0] + m1[ 9]*m2[ 4] + m1[10]*m2[ 8] + m1[11]*m2[12],
         m1[ 8]*m2[ 1] + m1[ 9]*m2[ 5] + m1[10]*m2[ 9] + m1[11]*m2[13],
         m1[ 8]*m2[ 2] + m1[ 9]*m2[ 6] + m1[10]*m2[10] + m1[11]*m2[14],
         m1[ 8]*m2[ 3] + m1[ 9]*m2[ 7] + m1[10]*m2[11] + m1[11]*m2[15],

         m1[12]*m2[ 0] + m1[13]*m2[ 4] + m1[14]*m2[ 8] + m1[15]*m2[12],
         m1[12]*m2[ 1] + m1[13]*m2[ 5] + m1[14]*m2[ 9] + m1[15]*m2[13],
         m1[12]*m2[ 2] + m1[13]*m2[ 6] + m1[14]*m2[10] + m1[15]*m2[14],
         m1[12]*m2[ 3] + m1[13]*m2[ 7] + m1[14]*m2[11] + m1[15]*m2[15]);
}

//
// global division: matrix / scalar
//
template <class T, class U>
inline CMatrix3< NCBI_PROMOTE(T,U) >
operator/ (const CMatrix3<T>& m1, U s)
{
    s = T(1) / s;
    return
        CMatrix3< NCBI_PROMOTE(T,U) >
        (m1[0]*s, m1[1]*s, m1[2]*s,
         m1[3]*s, m1[4]*s, m1[5]*s,
         m1[6]*s, m1[7]*s, m1[8]*s);
}


template <class T, class U>
inline CMatrix4< NCBI_PROMOTE(T,U) >
operator/ (const CMatrix4<T>& m1, U s)
{
    s = T(1) / s;
    return
        CMatrix4< NCBI_PROMOTE(T,U) >
        (m1[ 0]*s, m1[ 1]*s, m1[ 2]*s, m1[ 3]*s,
         m1[ 4]*s, m1[ 5]*s, m1[ 6]*s, m1[ 7]*s,
         m1[ 8]*s, m1[ 9]*s, m1[10]*s, m1[11]*s,
         m1[12]*s, m1[13]*s, m1[14]*s, m1[15]*s);
}



END_NCBI_SCOPE

/* @} */

#endif  // GUI_MATH___GLOBALS__HPP
