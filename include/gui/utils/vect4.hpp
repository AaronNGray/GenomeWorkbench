#ifndef GUI_MATH___VECT4___HPP
#define GUI_MATH___VECT4___HPP

/*  $Id: vect4.hpp 27010 2012-12-07 16:37:16Z falkrb $
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
#include <gui/utils/vect3.hpp>

/** @addtogroup GUI_MATH
 *
 * @{
 */

BEGIN_NCBI_SCOPE

template <class T>
class CVect4
{
public:
    typedef T TVecType;

public:
    // ctors
    CVect4();
    CVect4(T val);
    CVect4(T, T, T, T);

    // copy ctors
    CVect4(const CVect3<T>&);

    //
    // operators

    // operator: assignment
    CVect4<T>&   operator= (const CVect3<T>&);

    // operator: indexing
    const T&    operator[] (size_t i) const    { return m_Xyzw[i]; }
    T&          operator[] (size_t i)      { return m_Xyzw[i]; }

    // operators: math
    CVect4<T>&  operator+= (T);
    CVect4<T>&  operator+= (const CVect4<T>&);

    CVect4<T>&  operator-= (T);
    CVect4<T>&  operator-= (const CVect4<T>&);

    CVect4<T>&  operator*= (T);
    CVect4<T>&  operator*= (const CVect4<T>&);   // cross product!

    CVect4<T>&  operator/= (T);

    //
    // named functions


    // Set Value
    void Set(T x, T y, T z, T w);

    // return length of vector
    float       Length() const;
    float       Length2() const;
    // return true if the length of the vector is 0
    bool        Vanishing() const;

    // normalize a vector
    void        Normalize();

    // return a unit vector in the direction of the current vector
    CVect4<T>   Direction() const;

    // test whether a passed vector is parallel or normal to current
    bool        Parallel(const CVect4<T>&) const;
    bool        Normal(const CVect4<T>&) const;

    // perform dot product
    T           Dot(const CVect4<T>&) const;

    // accessor functions
    T&          X()         { return m_Xyzw[0]; }
    const T&    X() const   { return m_Xyzw[0]; }
    T&          Y()         { return m_Xyzw[1]; }
    const T&    Y() const   { return m_Xyzw[1]; }
    T&          Z()         { return m_Xyzw[2]; }
    const T&    Z() const   { return m_Xyzw[2]; }
    T&          W()         { return m_Xyzw[3]; }
    const T&    W() const   { return m_Xyzw[3]; }

    const T*    GetData() const    { return m_Xyzw; }

private:
    T m_Xyzw[4];

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
CVect4<T>::CVect4()
{
    m_Xyzw[0] = m_Xyzw[1] = m_Xyzw[2] = m_Xyzw[3] = (T)0;
}


//
// conversion ctors
//
template <class T> inline
CVect4<T>::CVect4 (T val)
{
    m_Xyzw[0] = m_Xyzw[1] = m_Xyzw[2] = m_Xyzw[3] = val;
}


template <class T> inline
CVect4<T>::CVect4 (T x, T y, T z, T w)
{
    m_Xyzw[0] = x;
    m_Xyzw[1] = y;
    m_Xyzw[2] = z;
    m_Xyzw[3] = w;
}


//
// copy ctor
//
template <class T> inline
CVect4<T>::CVect4 (const CVect3<T>& v)
{
    *this = v;
}


//
// assignment operator
//
template <class T> inline CVect4<T>&
CVect4<T>::operator= (const CVect3<T>& v)
{
    X() = v.X();
    Y() = v.Y();
    Z() = v.Z();
    W() = (T)1;

    return *this;
}


//
// operator+ (scalar)
//
template <class T> inline CVect4<T>&
CVect4<T>::operator+= (T scalar)
{
    X() += scalar / W();
    Y() += scalar / W();
    Z() += scalar / W();

    return *this;
}

//
// operator+ (vector)
//
template <class T> inline CVect4<T>&
CVect4<T>::operator+= (const CVect4<T>& v)
{
    X() += v.X();
    Y() += v.Y();
    Z() += v.Z();
    W() += v.W();

    return *this;
}


//
// operator- (scalar)
//
template <class T> inline CVect4<T>&
CVect4<T>::operator-= (T scalar)
{
    X() -= scalar / W();
    Y() -= scalar / W();
    Z() -= scalar / W();

    return *this;
}

//
// operator- (vector)
//
template <class T> inline CVect4<T>&
CVect4<T>::operator-= (const CVect4<T>& v)
{
    X() -= v.X();
    Y() -= v.Y();
    Z() -= v.Z();
    W() -= v.W();

    return *this;
}


//
// operator*= (scalar)
//
template <class T> inline CVect4<T>&
CVect4<T>::operator*= (T scalar)
{
    X() *= scalar / W();
    Y() *= scalar / W();
    Z() *= scalar / W();

    return *this;
}


//
// dot(vector)
//
template <class T> inline T
CVect4<T>::Dot(const CVect4<T>& v) const
{
    return (T)(X() * v.X() + Y() * v.Y() + Z() * v.Z() + W() * v.W());
}


//
// operator/= (scalar)
//
template <class T> inline CVect4<T>&
CVect4<T>::operator/= (T scalar)
{
    X() /= scalar / W();
    Y() /= scalar / W();
    Z() /= scalar / W();

    return *this;
}

//
// Set()
//     set x,y,z, w values
//
template <class T> inline void
CVect4<T>::Set(T x, T y, T z, T w)
{
    m_Xyzw[0] = x;
    m_Xyzw[1] = y;
    m_Xyzw[2] = z;
    m_Xyzw[3] = w;
}


//
// length()
//  return the length of a vector
//
template <class T> inline float
CVect4<T>::Length() const
{
    return ::sqrt(Length2());
}


template <class T> inline float
CVect4<T>::Length2() const
{
    return (X() * X() + Y() * Y() + Z() * Z()) / (W() * W());
}


template <class T> inline bool
CVect4<T>::Vanishing() const
{
    return (Length2() == 0.0);
}


//
// normalize()
//  converts a vector to a unit vector
//
template <class T> inline void
CVect4<T>::Normalize()
{
    if (W()) {
        X() = (T)(X() / W());
        Y() = (T)(Y() / W());
        Z() = (T)(Z() / W());
        W() = (T)1;
    }
}


//
// direction()
//  returns a unit vector in the direction of the current vector
//
template <class T> inline CVect4<T>
CVect4<T>::Direction() const
{
    return CVect4<T> (*this, true);
}


//
// parallel()
//  tests to see whether the passed vector is parallel to the current vector
//
template <class T> inline bool
CVect4<T>::Parallel(const CVect4<T>& v) const
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
//  tests to see whether the passed vector is nromal to the current vector
//
template <class T> inline bool
CVect4<T>::Normal(const CVect4<T>& v) const
{
    typedef NCBI_PROMOTE(T, float) TFloat;
    TFloat result = (TFloat)dot(v);
    if (result == TFloat(0)  &&
        Length() != TFloat(0)  &&
        v.Length() != TFloat(0)) {
        return true;
    }
    return false;
}


END_NCBI_SCOPE

/* @} */

#endif // GUI_MATH___VECT4___HPP
