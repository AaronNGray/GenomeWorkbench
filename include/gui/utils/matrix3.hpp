#ifndef GUI_MATH___MATRIX3___HPP
#define GUI_MATH___MATRIX3___HPP

/*  $Id: matrix3.hpp 27135 2012-12-27 16:45:51Z falkrb $
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

/** @addtogroup GUI_MATH
 *
 * @{
 */

#include <gui/utils/vect3.hpp>

BEGIN_NCBI_SCOPE


template <class T>
class CMatrix3
{
public:
    // ctors
    CMatrix3();
    CMatrix3 (T,T,T, T,T,T, T,T,T);
    CMatrix3 (const T[9]);
    CMatrix3 (const T[3][3]);

    //
    // operators
    //

    // operator: index with bounds checking
    T           operator() (int i, int j) const { return m_Data[i * 3+j]; }
    T&          operator() (int i, int j)       { return m_Data[i * 3+j]; }

    // operator: index with no bounds checking
    T           operator[] (int i) const    { return m_Data[i]; }
    T&          operator[] (int i)          { return m_Data[i]; }

    // operator: addition
    CMatrix3<T>& operator+= (T);
    CMatrix3<T>& operator+= (const CMatrix3<T>&);

    // operator: subtraction
    CMatrix3<T>& operator-= (T);
    CMatrix3<T>& operator-= (const CMatrix3<T>&);

    // operator: multiplication
    CMatrix3<T>& operator*= (T);
    CMatrix3<T>& operator*= (const CMatrix3<T>&);

    // operator: division
    CMatrix3<T>& operator/= (T);

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
    CVect3<T>   Row(int r) const;

    // return a vector representing a column
    CVect3<T>   Column(int c) const;

private:
    T m_Data[9];

};

END_NCBI_SCOPE

//
// global operations
// this is included after the class declaration
#include <gui/utils/globals.hpp>

BEGIN_NCBI_SCOPE


//
// default ctor
//
template <class T> inline
CMatrix3<T>::CMatrix3()
{
    Clear(T(0));
}

//
// conversion ctors
//
template <class T> inline
CMatrix3<T>::CMatrix3 (T m1, T m2, T m3,  T m4, T m5, T m6,  T m7, T m8, T m9)
{
    m_Data[0] = m1; m_Data[1] = m2; m_Data[2] = m3;
    m_Data[3] = m4; m_Data[4] = m5; m_Data[5] = m6;
    m_Data[6] = m7; m_Data[7] = m8; m_Data[8] = m9;
}


template <class T> inline
CMatrix3<T>::CMatrix3(const T m[9])
{
    for (int i = 0;  i < 9;  ++i) {
        m_Data[i] = m[i];
    }
}


template <class T> inline
CMatrix3<T>::CMatrix3(const T m[3][3])
{
    m_Data[0] = m[0][0];
    m_Data[1] = m[0][1];
    m_Data[2] = m[0][2];
    m_Data[3] = m[1][0];
    m_Data[4] = m[1][1];
    m_Data[5] = m[1][2];
    m_Data[6] = m[2][0];
    m_Data[7] = m[2][1];
    m_Data[8] = m[2][2];
}


//
// addition: matrix
template <class T> inline CMatrix3<T>&
CMatrix3<T>::operator+= (const CMatrix3<T>& m)
{
    for (int i = 0;  i < 9;  ++i) {
        m_Data[i] += m[i];
    }

    return *this;
}

//
// Addition: scalar
template <class T> inline CMatrix3<T>&
CMatrix3<T>::operator+= (T scalar)
{
    for (int i = 0;  i < 9;  ++i) {
        m_Data[i] += scalar;
    }

    return *this;
}

//
// subtraction: matrix
template <class T> inline CMatrix3<T>&
CMatrix3<T>::operator-= (const CMatrix3<T>& m)
{
    for (int i = 0;  i < 9;  ++i) {
        m_Data[i] -= m[i];
    }

    return *this;
}

//
// Subtraction: scalar
template <class T> inline CMatrix3<T>&
CMatrix3<T>::operator-= (T scalar)
{
    for (int i = 0;  i < 9;  ++i) {
        m_Data[i] -= scalar;
    }

    return *this;
}


//
// multiplication: scalar
template <class T> inline CMatrix3<T>&
CMatrix3<T>::operator*= (T scalar)
{
    for (int i = 0;  i < 9;  ++i) {
        m_Data[i] *= scalar;
    }

    return *this;
}

//
// multiplication: matrix
template <class T> inline CMatrix3<T>&
CMatrix3<T>::operator*= (const CMatrix3<T>& m)
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
    //
    T f1, f2, f3;

    f1 = m_Data[0]*m[0] + m_Data[1]*m[3] + m_Data[2]*m[6];
    f2 = m_Data[0]*m[1] + m_Data[1]*m[4] + m_Data[2]*m[7];
    f3 = m_Data[0]*m[2] + m_Data[1]*m[5] + m_Data[2]*m[8];
    m_Data[0] = f1;
    m_Data[1] = f2;
    m_Data[2] = f3;

    f1 = m_Data[3]*m[0] + m_Data[4]*m[3] + m_Data[5]*m[6];
    f2 = m_Data[3]*m[1] + m_Data[4]*m[4] + m_Data[5]*m[7];
    f3 = m_Data[3]*m[2] + m_Data[4]*m[5] + m_Data[5]*m[8];
    m_Data[3] = f1;
    m_Data[4] = f2;
    m_Data[5] = f3;

    f1 = m_Data[6]*m[0] + m_Data[7]*m[3] + m_Data[8]*m[6];
    f2 = m_Data[6]*m[1] + m_Data[7]*m[4] + m_Data[8]*m[7];
    f3 = m_Data[6]*m[2] + m_Data[7]*m[5] + m_Data[8]*m[8];
    m_Data[6] = f1;
    m_Data[7] = f2;
    m_Data[8] = f3;

    return *this;
}

//
// division: scalar
//
template <class T> inline CMatrix3<T>&
CMatrix3<T>::operator/= (T scalar)
{
    scalar = T(1) / scalar;
    for (int i = 0;  i < 9;  ++i) {
        m_Data[i] *= scalar;
    }

    return *this;
}


//
// transpose a matrix
//
template <class T> inline void
CMatrix3<T>::Transpose()
{
    // given
    //   a b c
    //   d e f
    //   g h i
    //
    // result is
    //   a d g
    //   b e h
    //   c f i
    //

    std::swap(m_Data[3], m_Data[1]);
    std::swap(m_Data[6], m_Data[2]);
    std::swap(m_Data[7], m_Data[5]);
}

//
// create an identity matrix
//
template <class T> inline void
CMatrix3<T>::Identity()
{
    m_Data[0] = (T)1;
    m_Data[1] = (T)0;
    m_Data[2] = (T)0;

    m_Data[3] = (T)0;
    m_Data[4] = (T)1;
    m_Data[5] = (T)0;

    m_Data[6] = (T)0;
    m_Data[7] = (T)0;
    m_Data[8] = (T)1;
}

//
// clear a matrix so that all its values are the passed value
//
template <class T> inline void
CMatrix3<T>::Clear(T value)
{
    for (int i = 0;  i < 9;  ++i) {
        m_Data[i] = value;
    }
}


//
// return a row as a vector
//
template <class T> inline CVect3<T>
CMatrix3<T>::Row(int i) const
{
    return CVect3<T> (m_Data[i * 3], m_Data[i * 3 + 1], m_Data[i * 3 + 2]);
}


//
// return a column as a vector
//
template <class T> inline CVect3<T>
CMatrix3<T>::Column(int j) const
{
    return CVect3<T> (m_Data[j], m_Data[3 + j], m_Data[6 + j]);
}


//
// return the determinant of a 3x3 matrix
//
template <class T> inline T
CMatrix3<T>::Determinant() const
{
    // given
    //   a b c
    //   d e f
    //   g h i
    //
    // the determinant is
    //     a(ei - hf) + b(di - fg) + c(dh - eg)
    //
    return (m_Data[0] * (m_Data[4]*m_Data[8] - m_Data[5]*m_Data[7]) +
            m_Data[1] * (m_Data[3]*m_Data[8] - m_Data[5]*m_Data[6]) +
            m_Data[2] * (m_Data[3]*m_Data[7] - m_Data[4]*m_Data[6]));
}


END_NCBI_SCOPE

/* @} */

#endif // GUI_MATH___MATRIX3___HPP
