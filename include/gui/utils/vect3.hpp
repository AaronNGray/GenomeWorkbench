#ifndef GUI_MATH___VECT3___HPP
#define GUI_MATH___VECT3___HPP

/*  $Id: vect3.hpp 27010 2012-12-07 16:37:16Z falkrb $
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
class CVect3
{
public:
    typedef T TVecType;

public:     // interface
    // ctors
    CVect3();
    explicit CVect3(T val);
    explicit CVect3(T, T, T);

    //
    //  operators

    // operator: indexing
    const T&    operator[] (int i) const    { return m_Xyz[i]; }
    T&          operator[] (int i)      { return m_Xyz[i]; }

    // operators: math
    CVect3<T>&  operator+= (T);
    CVect3<T>&  operator+= (const CVect3<T>&);

    CVect3<T>&  operator-= (T);
    CVect3<T>&  operator-= (const CVect3<T>&);

    CVect3<T>&  operator*= (T);
    CVect3<T>&  operator*= (const CVect3<T>&);   // cross product!

    CVect3<T>&  operator/= (T);
    CVect3<T>&  operator/= (const CVect3<T>&);

    //
    // named functions

    // Set Value
    void Set(T x, T y, T z);

    // return length of vector
    float       Length() const;

    // return length of vector squared
    float       Length2() const;

    // return true if the length of the vector is 0
    bool        Vanishing() const;

    // normalize a vector
    void        Normalize();

    // return a unit vector in the direction of the current vector
    CVect3<T>   Direction() const;

    // test whether a passed vector is parallel or normal to current
    bool        Parallel(const CVect3<T>&) const;
    bool        Normal(const CVect3<T>&) const;

    // perform dot product
    T           Dot(const CVect3<T>&) const;

    // perform cross product(same as operator*=)
    CVect3<T>   Cross(const CVect3<T>&) const;

    // accessor functions
    T&          X()         { return m_Xyz[0]; }
    const T&    X() const   { return m_Xyz[0]; }
    T&          Y()         { return m_Xyz[1]; }
    const T&    Y() const   { return m_Xyz[1]; }
    T&          Z()         { return m_Xyz[2]; }
    const T&    Z() const   { return m_Xyz[2]; }

    const T*    GetData() const    { return m_Xyz; }

private:
    T m_Xyz[3];

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
CVect3<T>::CVect3()
{
    m_Xyz[0] = m_Xyz[1] = m_Xyz[2] = (T)0;
}


//
// conversion ctors
//
template <class T> inline
CVect3<T>::CVect3 (T val)
{
    m_Xyz[0] = m_Xyz[1] = m_Xyz[2] = val;
}


template <class T> inline
CVect3<T>::CVect3 (T x, T y, T z)
{
    m_Xyz[0] = x;
    m_Xyz[1] = y;
    m_Xyz[2] = z;
}

//
// operator+ (scalar)
//
template <class T> inline CVect3<T>&
CVect3<T>::operator+= (T scalar)
{
    X() += scalar;
    Y() += scalar;
    Z() += scalar;

    return *this;
}


//
// operator+ (vector)
//
template <class T> inline CVect3<T>&
CVect3<T>::operator+= (const CVect3<T>& v)
{
    X() += v.X();
    Y() += v.Y();
    Z() += v.Z();

    return *this;
}


//
// operator- (scalar)
//
template <class T> inline CVect3<T>&
CVect3<T>::operator-= (T scalar)
{
    X() -= scalar;
    Y() -= scalar;
    Z() -= scalar;

    return *this;
}

//
// operator- (vector)
//
template <class T> inline CVect3<T>&
CVect3<T>::operator-= (const CVect3<T>& v)
{
    X() -= v.X();
    Y() -= v.Y();
    Z() -= v.Z();

    return *this;
}


//
// operator*= (scalar)
//
template <class T> inline CVect3<T>&
CVect3<T>::operator*= (T scalar)
{
    X() *= scalar;
    Y() *= scalar;
    Z() *= scalar;

    return *this;
}


//
// operator* (vector)
//     (cross product!!!)
//
template <class T> inline CVect3<T>&
CVect3<T>::operator*= (const CVect3<T>& v)
{
    *this = Cross(v);
    return *this;
}

//
// dot(vector)
//
template <class T> inline T
CVect3<T>::Dot(const CVect3<T>& v) const
{
    return (T)(X() * v.X() + Y() * v.Y() + Z() * v.Z());
}

//
// cross()
//     cross product; same as operator*= except no assignment
//
template <class T> inline CVect3<T>
CVect3<T>::Cross(const CVect3<T>& v) const
{
    // cross product is as follows:
    //
    //     a x b = <a_2 * b_3 - a_3 * b_2, -(a_1 * b_3 - a_3 * b_1), a_1 * b_2 - a_2 * b_1>
    //
    return CVect3<T> (  Y() * v.Z() - Z() * v.Y(),
                          -(X() * v.Z() - Z() * v.X()),
                          X() * v.Y() - Y() * v.X());
}

//
// operator/= (scalar)
//
template <class T> inline CVect3<T>&
CVect3<T>::operator/= (T scalar)
{
    scalar = T(1) / scalar;
    X() *= scalar;
    Y() *= scalar;
    Z() *= scalar;

    return *this;
}

//
// Set()
//     set x,y,z values
//
template <class T> inline void
CVect3<T>::Set(T x, T y, T z) 
{
    m_Xyz[0] = x;
    m_Xyz[1] = y;
    m_Xyz[2] = z;
}

//
// length()
//     return the length of a vector
//
template <class T> inline float
CVect3<T>::Length() const
{
    return ::sqrt(Length2());
}


template <class T> inline float
CVect3<T>::Length2() const
{
    return (X() * X() + Y() * Y() + Z() * Z());
}


template <class T> inline bool
CVect3<T>::Vanishing() const
{
    return (Length2() == 0.0);
}


//
// normalize()
//     converts a vector to a unit vector
//
template <class T> inline void
CVect3<T>::Normalize()
{
    typedef NCBI_PROMOTE(T, float) TFloat;
    TFloat len = TFloat(Length());

    if (len != TFloat(0)) {
        TFloat f = TFloat(1) / len;

        X() = (X() * f);
        Y() = (Y() * f);
        Z() = (Z() * f);
    }


}


//
// direction()
//     returns a unit vector in the direction of the current vector
//
template <class T> inline CVect3<T>
CVect3<T>::Direction() const
{
    return CVect3<T>(*this, true);
}


//
// parallel()
//     tests to see whether the passed vector is parallel to the current vector
//
template <class T> inline bool
CVect3<T>::Parallel(const CVect3<T>& v) const
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
CVect3<T>::Normal(const CVect3<T>& v) const
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

#endif // GUI_MATH___VECT3___HPP
