#ifndef GUI_MATH___VECT2___HPP
#define GUI_MATH___VECT2___HPP

/*  $Id: vect2.hpp 27010 2012-12-07 16:37:16Z falkrb $
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
#include <gui/utils/math.hpp>

/** @addtogroup GUI_MATH
 *
 * @{
 */

BEGIN_NCBI_SCOPE


template <class T>
class CVect2
{
public:
    typedef T TVecType;

public:
    // ctors
    CVect2();
    CVect2(T val);
    CVect2(T, T);

    //
    //  operators

    // operator: indexing
    T  operator[] (int i) const  { return m_Xy[i]; }
    T& operator[] (int i)        { return m_Xy[i]; }

    // operators: math
    CVect2<T>& operator+= (T);
    CVect2<T>& operator+= (const CVect2<T>&);

    CVect2<T>& operator-= (T);
    CVect2<T>& operator-= (const CVect2<T>&);

    CVect2<T>& operator*= (T);
    // cross product undefined in 2D
    //CVect2<T>& operator*= (const CVect2<T>&); // cross product! 

    CVect2<T>& operator/= (T);

    //
    // named functions

    // Set Value
    void Set(T x, T y);

    // return length of vector
    float Length() const;

    // return length of vector squared
    float Length2() const;

    // return true if the length of the vector is 0
    bool Vanishing() const;

    // normalize a vector
    void Normalize();

    // return a unit vector in the direction of the current vector
    CVect2<T> Direction() const;

    // test whether a passed vector is parallel or normal to current
    bool Parallel(const CVect2<T>&) const;
    bool Normal(const CVect2<T>&) const;

    // perform dot product
    T Dot(const CVect2<T>&) const;

    // accessor functions
    T&          X()             { return m_Xy[0]; }
    const T&    X() const       { return m_Xy[0]; }
    T&          Y()             { return m_Xy[1]; }
    const T&    Y() const       { return m_Xy[1]; }

    const T*    GetData() const { return m_Xy; }

protected:
    T m_Xy[2];
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
CVect2<T>::CVect2()
{
    m_Xy[0] = m_Xy[1] = (T)0;
}


//
// conversion ctors
//
template <class T> inline
CVect2<T>::CVect2 (T val)
{
    m_Xy[0] = m_Xy[1] = val;
}


template <class T> inline
CVect2<T>::CVect2 (T x, T y)
{
    m_Xy[0] = x;
    m_Xy[1] = y;
}


//
// operator+ (scalar)
//
template <class T> inline CVect2<T>&
CVect2<T>::operator+= (T scalar)
{
    X() += scalar;
    Y() += scalar;

    return *this;
}

//
// operator+ (vector)
//
template <class T> inline CVect2<T>&
CVect2<T>::operator+= (const CVect2<T>& v)
{
    X() += v.X();
    Y() += v.Y();

    return *this;
}


//
// operator- (scalar)
//
template <class T> inline CVect2<T>&
CVect2<T>::operator-= (T scalar)
{
    X() -= scalar;
    Y() -= scalar;

    return *this;
}

//
// operator- (vector)
//
template <class T> inline CVect2<T>&
CVect2<T>::operator-= (const CVect2<T>& v)
{
    X() -= v.X();
    Y() -= v.Y();

    return *this;
}


//
// operator*= (scalar)
//
template <class T> inline CVect2<T>&
CVect2<T>::operator*= (T scalar)
{
    X() *= scalar;
    Y() *= scalar;

    return *this;
}


//
// operator* (vector)
//     (cross product!!!)
//
/*
template <class T> inline CVect2<T>&
CVect2<T>::operator*= (const CVect2<T>& v)
{
    *this = Cross(v);
    return *this;
}
*/

//
// dot(vector)
//
template <class T> inline T
CVect2<T>::Dot(const CVect2<T>& v) const
{
    return (T)(X() * v.X() + Y() * v.Y());
}

//
// operator/= (scalar)
//
template <class T> inline CVect2<T>&
CVect2<T>::operator/= (T scalar)
{
    scalar = T(1) / scalar;
    X() *= scalar;
    Y() *= scalar;

    return *this;
}

//
// Set()
//     set x and y values
//
template <class T> inline void
CVect2<T>::Set(T x, T y)
{
    m_Xy[0] = x;
    m_Xy[1] = y;
}

//
// length()
//     return the length of a vector
//
template <class T> inline float
CVect2<T>::Length() const
{
    return ::sqrt(Length2());
}


template <class T> inline float
CVect2<T>::Length2() const
{
    return (X() * X() + Y() * Y());
}


template <class T> inline bool
CVect2<T>::Vanishing() const
{
    return (Length2() == 0.0);
}


//
// normalize()
//     converts a vector to a unit vector
//
template <class T> inline void
CVect2<T>::Normalize()
{
    typedef NCBI_PROMOTE(T, float) TFloat;
    TFloat len = TFloat(Length());

    if (len != TFloat(0)) {
        TFloat f = TFloat(1) / len;

        X() = (X() * f);
        Y() = (Y() * f);
    }
}


//
// direction()
//     returns a unit vector in the direction of the current vector
//
template <class T> inline CVect2<T>
CVect2<T>::Direction() const
{
    return CVect2<T>(*this, true);
}


//
// parallel()
//     tests to see whether the passed vector is parallel to the current vector
//
template <class T> inline bool
CVect2<T>::Parallel(const CVect2<T>& v) const
{
    typedef NCBI_PROMOTE(T, float) TFloat;
    TFloat result = (TFloat)Dot(v);
    TFloat l = Length() * v.Length();

    if (result == l  ||  result == -l) {
        return true;
    }

    return false;
}


//
// normal()
//     tests to see whether the passed vector is nromal to the current vector
//
template <class T> inline bool
CVect2<T>::Normal(const CVect2<T>& v) const
{
    typedef NCBI_PROMOTE(T, float) TFloat;
    TFloat result = (TFloat)Dot(v);
    if (result == TFloat(0)  &&
        Length() != TFloat(0)  &&
        v.Length() != TFloat(0)) {
        return true;
    }
    return false;
}


END_NCBI_SCOPE

/* @} */

#endif // GUI_MATH___VECT2___HPP///
