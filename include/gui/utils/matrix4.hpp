#ifndef GUI_MATH___MATRIX4___HPP
#define GUI_MATH___MATRIX4___HPP

/*  $Id: matrix4.hpp 30863 2014-07-31 15:41:12Z falkrb $
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
#include <gui/utils/vect4.hpp>

/** @addtogroup GUI_MATH
 *
 * @{
 */

BEGIN_NCBI_SCOPE


template <class T>
class CMatrix4
{
public:
    // ctors
    CMatrix4();
    CMatrix4 (T val);
    CMatrix4 (T,T,T,T, T,T,T,T, T,T,T,T, T,T,T,T);
    CMatrix4 (T[16]);
    CMatrix4 (T[4][4]);

    //
    // operators
    //

    // operator: index
    const T&    operator() (int i, int j) const { return m_Data[i * 4+j]; }
    T&          operator() (int i, int j)       { return m_Data[i * 4+j]; }

    // operator: index
    const T&    operator[] (int i) const        { return m_Data[i]; }
    T&          operator[] (int i)              { return m_Data[i]; }


    // operator: addition
    CMatrix4<T>& operator+= (T);
    CMatrix4<T>& operator+= (const CMatrix4<T>&);

    // operator: subtraction
    CMatrix4<T>& operator-= (T);
    CMatrix4<T>& operator-= (const CMatrix4<T>&);

    // operator: multiplication
    CMatrix4<T>& operator*= (T);
    CMatrix4<T>& operator*= (const CMatrix4<T>&);

    // operator: division
    CMatrix4<T>& operator/= (T);

    //
    // named functions


    // transpose the current matrix
    void        Transpose();

    // make the current matrix an identity matrix
    void        Identity();

    // clear the current matrix to a given value
    void        Clear(T x = (T)0);

    // return the determinant of the current matrix
    T           Determinant() const;

    // data accessor
    const T*    GetData() const    { return m_Data; }
    T*    GetData()    { return m_Data; }

    // return a vector representing a row
    CVect4<T>   Row(int) const;

    // return a vector representing a column
    CVect4<T>   Column(int) const;

private:
    T m_Data[16];

};


END_NCBI_SCOPE

//
// global operations
// this is included after the class declaration
#include <gui/utils/globals.hpp>

BEGIN_NCBI_SCOPE


//
// default ctor
template <class T> inline
CMatrix4<T>::CMatrix4()
{
    Clear(T(0));
}

//
// conversion ctors
template <class T> inline
CMatrix4<T>::CMatrix4(T val)
{
    Clear(val);
}


template <class T> inline
CMatrix4<T>::CMatrix4(T m1,  T m2,  T m3,  T m4,
                      T m5,  T m6,  T m7,  T m8,
                      T m9,  T m10, T m11, T m12,
                      T m13, T m14, T m15, T m16)
{
    m_Data[ 0] = m1;  m_Data[ 1] = m2;  m_Data[ 2] = m3;  m_Data[ 3] = m4;
    m_Data[ 4] = m5;  m_Data[ 5] = m6;  m_Data[ 6] = m7;  m_Data[ 7] = m8;
    m_Data[ 8] = m9;  m_Data[ 9] = m10; m_Data[10] = m11; m_Data[11] = m12;
    m_Data[12] = m13; m_Data[13] = m14; m_Data[14] = m15; m_Data[15] = m16;
}


template <class T> inline
CMatrix4<T>::CMatrix4(T m[16])
{
    for (int i = 0;  i < 16;  ++i) {
        m_Data[i] = m[i];
    }
}


template <class T> inline
CMatrix4<T>::CMatrix4(T m[4][4])
{
    m_Data[ 0] = m[0][0];
    m_Data[ 1] = m[0][1];
    m_Data[ 2] = m[0][2];
    m_Data[ 3] = m[0][3];

    m_Data[ 4] = m[1][0];
    m_Data[ 5] = m[1][1];
    m_Data[ 6] = m[1][2];
    m_Data[ 7] = m[1][3];

    m_Data[ 8] = m[2][0];
    m_Data[ 9] = m[2][1];
    m_Data[10] = m[2][2];
    m_Data[11] = m[2][3];

    m_Data[12] = m[3][0];
    m_Data[13] = m[3][1];
    m_Data[14] = m[3][2];
    m_Data[15] = m[3][3];
}


//
// addition: scalar
template <class T> inline CMatrix4<T>&
CMatrix4<T>::operator+= (T scalar)
{
    for (int i = 0;  i < 16;  ++i) {
        m_Data[i] += scalar;
    }

    return *this;
}

//
// addition: matrix
template <class T> inline CMatrix4<T>&
CMatrix4<T>::operator+= (const CMatrix4<T>& m)
{
    for (int i = 0;  i < 16;  ++i) {
        m_Data[i] += m[i];
    }

    return *this;
}

//
// subtraction: scalar
template <class T> inline CMatrix4<T>&
CMatrix4<T>::operator-= (T scalar)
{
    for (int i = 0;  i < 16;  ++i) {
        m_Data[i] -= scalar;
    }

    return *this;
}

//
// subtraction: matrix
template <class T> inline CMatrix4<T>&
CMatrix4<T>::operator-= (const CMatrix4<T>& m)
{
    for (int i = 0;  i < 16;  ++i) {
        m_Data[i] -= m[i];
    }

    return *this;
}


//
// multiplication: scalar
template <class T> inline CMatrix4<T>&
CMatrix4<T>::operator*= (T scalar)
{
    for (int i = 0;  i < 16;  ++i) {
        m_Data[i] *= scalar;
    }

    return *this;
}

//
// multiplication: matrix
template <class T> inline CMatrix4<T>&
CMatrix4<T>::operator*= (const CMatrix4<T>& m)
{
    // given
    //
    // a b c   1 2 3
    // d e f   4 5 6
    // g h i   7 8 9
    //
    // result is:
    //  (a1 + b4 + c7)  (a2 + b5 + c8)  (a3 + b6 + c9)
    //  (d1 + e4 + f7)  (d2 + e5 + f8)  (d3 + e6 + f9)
    //  (g1 + h4 + i7)  (g2 + h5 + i8)  (g3 + h6 + i9)

    T t0;
    T t1;
    T t2;
    T t3;

    t0 = m_Data[ 0] * m[ 0] + m_Data[ 1] * m[ 4] +
        m_Data[ 2] * m[ 8] + m_Data[ 3] * m[12];
    t1 = m_Data[ 0] * m[ 1] + m_Data[ 1] * m[ 5] +
        m_Data[ 2] * m[ 9] + m_Data[ 3] * m[13];
    t2 = m_Data[ 0] * m[ 2] + m_Data[ 1] * m[ 6] +
        m_Data[ 2] * m[10] + m_Data[ 3] * m[14];
    t3 = m_Data[ 0] * m[ 3] + m_Data[ 1] * m[ 7] +
        m_Data[ 2] * m[11] + m_Data[ 3] * m[15];
    m_Data[0] = t0;
    m_Data[1] = t1;
    m_Data[2] = t2;
    m_Data[3] = t3;

    t0 = m_Data[ 4] * m[ 0] + m_Data[ 5] * m[ 4] +
        m_Data[ 6] * m[ 8] + m_Data[ 7] * m[12];
    t1 = m_Data[ 4] * m[ 1] + m_Data[ 5] * m[ 5] +
        m_Data[ 6] * m[ 9] + m_Data[ 7] * m[13];
    t2 = m_Data[ 4] * m[ 2] + m_Data[ 5] * m[ 6] +
        m_Data[ 6] * m[10] + m_Data[ 7] * m[14];
    t3 = m_Data[ 4] * m[ 3] + m_Data[ 5] * m[ 7] +
        m_Data[ 6] * m[11] + m_Data[ 7] * m[15];
    m_Data[4] = t0;
    m_Data[5] = t1;
    m_Data[6] = t2;
    m_Data[7] = t3;

    t0 = m_Data[ 8] * m[ 0] + m_Data[ 9] * m[ 4] +
        m_Data[10] * m[ 8] + m_Data[11] * m[12];
    t1 = m_Data[ 8] * m[ 1] + m_Data[ 9] * m[ 5] +
        m_Data[10] * m[ 9] + m_Data[11] * m[13];
    t2 = m_Data[ 8] * m[ 2] + m_Data[ 9] * m[ 6] +
        m_Data[10] * m[10] + m_Data[11] * m[14];
    t3 = m_Data[ 8] * m[ 3] + m_Data[ 9] * m[ 7] +
        m_Data[10] * m[11] + m_Data[11] * m[15];
    m_Data[ 8] = t0;
    m_Data[ 9] = t1;
    m_Data[10] = t2;
    m_Data[11] = t3;

    t0 = m_Data[12] * m[ 0] + m_Data[13] * m[ 4] +
        m_Data[14] * m[ 8] + m_Data[15] * m[12];
    t1 = m_Data[12] * m[ 1] + m_Data[13] * m[ 5] +
        m_Data[14] * m[ 9] + m_Data[15] * m[13];
    t2 = m_Data[12] * m[ 2] + m_Data[13] * m[ 6] +
        m_Data[14] * m[10] + m_Data[15] * m[14];
    t3 = m_Data[12] * m[ 3] + m_Data[13] * m[ 7] +
        m_Data[14] * m[11] + m_Data[15] * m[15];
    m_Data[12] = t0;
    m_Data[13] = t1;
    m_Data[14] = t2;
    m_Data[15] = t3;

    return *this;
}

//
// division: scalar
template <class T> inline CMatrix4<T>&
CMatrix4<T>::operator/= (T scalar)
{
    scalar = T(1) / scalar;

    for (int i = 0;  i < 16;  ++i) {
        m_Data[i] *= scalar;
    }

    return *this;
}


//
// transpose a matrix
template <class T> inline void
CMatrix4<T>::Transpose()
{
    // given
    //   a b c d
    //   e f g h
    //   i j k l
    //   m n o p
    //
    // result is
    //   a e i m
    //   b f j n
    //   c g k o
    //   d h l p


    std::swap(m_Data[ 1], m_Data[ 4]);
    std::swap(m_Data[ 2], m_Data[ 8]);
    std::swap(m_Data[ 3], m_Data[12]);
    std::swap(m_Data[ 6], m_Data[ 9]);
    std::swap(m_Data[ 7], m_Data[13]);
    std::swap(m_Data[11], m_Data[14]);
}

//
// create a unit matrix
template <class T> inline void
CMatrix4<T>::Identity()
{
    Clear((T)0);
    m_Data[ 0] = 1;
    m_Data[ 5] = 1;
    m_Data[10] = 1;
    m_Data[15] = 1;
}

//
// clear a matrix so that all its values are the passed value
template <class T> inline void
CMatrix4<T>::Clear(T value)
{
    for (int i = 0;  i < 16;  ++i) {
        m_Data[i] = value;
    }
}


//
// return a row as a vector
template <class T> inline CVect4<T>
CMatrix4<T>::Row(int i) const
{
    int row_idx = i<<2;
    return CVect4<T> (m_Data[row_idx    ],
                      m_Data[row_idx + 1],
                      m_Data[row_idx + 2] ,
                      m_Data[row_idx + 3]);
}


//
// return a column as a vector
template <class T> inline CVect4<T>
CMatrix4<T>::Column(int j) const
{
    return CVect4<T> (m_Data[     j],
                      m_Data[ 4 + j],
                      m_Data[ 8 + j],
                      m_Data[12 + j]);
}


//
// return the determinant of a 3x3 matrix
template <class T> inline T
CMatrix4<T>::Determinant() const
{
    // given
    //  a b c d
    //  e f g h
    //  i j k l
    //  m n o p
    //
    // determinant is
    //
    // + a[ f(kp - lo) - g(jp - ln) + h(jo - kn) ]
    // - b[ e(kp - lo) - g(ip - lm) + h(io - km) ]
    // + c[ e(jp - ln) - f(ip - lm) + h(in - jm) ]
    // - d[ e(jo - kn) - f(io - km) + g(in - jm) ]

    T det;

    T kp = m_Data[10]*m_Data[15];
    T lo = m_Data[11]*m_Data[14];
    T jp = m_Data[ 9]*m_Data[15];
    T ln = m_Data[11]*m_Data[13];
    T jo = m_Data[ 9]*m_Data[14];
    T kn = m_Data[10]*m_Data[13];

    det = m_Data[0] * ( m_Data[5]*(kp - lo)
                        - m_Data[6]*(jp - ln)
                        + m_Data[7]*(jo - kn) );

    T ip = m_Data[ 8]*m_Data[15];
    T lm = m_Data[11]*m_Data[12];
    T io = m_Data[ 8]*m_Data[14];
    T km = m_Data[10]*m_Data[12];

    det -= m_Data[1] * ( m_Data[4]*(kp - lo)
                         - m_Data[6]*(ip - lm)
                         + m_Data[7]*(io - km) );

    T in = m_Data[ 8]*m_Data[13];
    T jm = m_Data[ 9]*m_Data[12];

    det += m_Data[3] * ( m_Data[4]*(jp - ln)
                         - m_Data[5]*(ip - lm)
                         + m_Data[7]*(in - jm) );

    det -= m_Data[4] * ( m_Data[4]*(jo - kn)
                         - m_Data[5]*(io - km)
                         + m_Data[6]*(in - jm) );

    return det;
}


END_NCBI_SCOPE

/* @} */

#endif // GUI_MATH___MATRIX4___HPP
