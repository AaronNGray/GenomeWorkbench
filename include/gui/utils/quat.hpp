#ifndef GUI_MATH___QUAT___HPP
#define GUI_MATH___QUAT___HPP

/*  $Id: quat.hpp 14973 2007-09-14 12:11:32Z dicuccio $
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

#include <gui/utils/vect3.hpp>
#include <gui/utils/matrix4.hpp>

/** @addtogroup GUI_MATH
 *
 * @{
 */

BEGIN_NCBI_SCOPE

template <class T>
class CQuat
{
public:
    // default ctor
    CQuat(void);
    // conversion ctor
    CQuat(T x, T y, T z, T w);
    // conversion ctor: rotation of degrees about axis
    CQuat(const CVect3<T>& axis, T degrees);

    // operator: index
    T&          operator[] (int i)          { return m_Xyzw[i]; }
    const T&    operator[] (int i) const    { return m_Xyzw[i]; }

    // operator: *=
    CQuat<T>&    operator *= (const CQuat<T>&);
    // operator: +=
    CQuat<T>&    operator += (const CQuat<T>&);
    // operator: -=
    CQuat<T>&    operator -= (const CQuat<T>&);

    // access data through named functions
    T&          X(void)         { return m_Xyzw[0]; }
    const T&    X(void) const   { return m_Xyzw[0]; }

    T&          Y(void)         { return m_Xyzw[1]; }
    const T&    Y(void) const   { return m_Xyzw[1]; }

    T&          Z(void)         { return m_Xyzw[2]; }
    const T&    Z(void) const   { return m_Xyzw[2]; }

    T&          W(void)         { return m_Xyzw[3]; }
    const T&    W(void) const   { return m_Xyzw[3]; }

    //
    // other named functions

    // create a rotation matric from a quaternion
    void        ToMatrix(CMatrix4<T>&) const;
    CMatrix4<T> ToMatrix(void) const;

    // get the conjugate of a quaternion
    CQuat<T>    Conjugate(void) const;

    // return the magnitude of a quaternion
    T           Length(void) const;

    // return the magnitude squared of a quaternion
    T           Length2(void) const;

    // inverse of a quaterion(1 / q) = q_conj / (q * q_conj)
    CQuat<T>    Inverse(void) const;

    // rotate a vector using the current quaternion
    void        Rotate(CVect3<float>& point) const;

protected:  // data
    // the data is stored as a 4 - element array
    T           m_Xyzw[4];

protected:  // internals

};

//
// global operators
//

//
// operator: compare - equals
template <class T>
inline bool
operator== (const CQuat<T>& first, const CQuat<T>& second)
{
    return (first[0] == second[0]  &&
            first[1] == second[1]  &&
            first[2] == second[2]  &&
            first[3] == second[3]);
}

//
// operator: compare - not - equals
template <class T>
inline bool
operator!= (const CQuat<T>& first, const CQuat<T>& second)
{
    return !(first == second);
}

//
// operator: multiply
template <class T>
inline CQuat<T>
operator* (const CQuat<T>& first, const CQuat<T>& second)
{
    return (CQuat<T> (first) *= second);
}

//
// operator: multiply
template <class T>
inline CQuat<T>
operator* (const CQuat<T>& first, T second)
{
    return (CQuat<T> (first.X() * second,
                        first.Y() * second,
                        first.Z() * second,
                        first.W() * second));
}

//
// operator: multiply
template <class T>
inline CQuat<T>
operator* (T first, const CQuat<T>& second)
{
    return (CQuat<T> (second.X() * first,
                        second.Y() * first,
                        second.Z() * first,
                        second.W() * first));
}

//
// operator: divide
template <class T>
inline CQuat<T>
operator/ (const CQuat<T>& first, T second)
{
    return (CQuat<T> (first.X() / second,
                        first.Y() / second,
                        first.Z() / second,
                        first.W() / second));
}

//
// operator: add
template <class T>
inline CQuat<T>
operator+ (const CQuat<T>& first, const CQuat<T>& second)
{
    return (CQuat<T> (first) += second);
}

//
// operator: subtract
template <class T>
inline CQuat<T>
operator- (const CQuat<T>& first, const CQuat<T>& second)
{
    return (CQuat<T> (first) -= second);
}

//
// default ctor
template <class T>
inline
CQuat<T>::CQuat()
{
    m_Xyzw[0] = T(0);
    m_Xyzw[1] = T(0);
    m_Xyzw[2] = T(0);
    m_Xyzw[3] = T(0);
}


//
// conversion ctor
template <class T>
inline
CQuat<T>::CQuat(T x, T y, T z, T w)
{
    m_Xyzw[0] = x;
    m_Xyzw[1] = y;
    m_Xyzw[2] = z;
    m_Xyzw[3] = w;
}

//
// conversion ctor: rotation quat
// we want to create a rotation quat about an axis
template <class T>
inline
CQuat<T>::CQuat(const CVect3<T>& axis, T theta)
{
    theta *= math::pi / 180.0f;
    double cos_theta = cos(theta / 2.0);
    double sin_theta = sin(theta / 2.0);

    m_Xyzw[0] = T(axis[0]*sin_theta);
    m_Xyzw[1] = T(axis[1]*sin_theta);
    m_Xyzw[2] = T(axis[2]*sin_theta);
    m_Xyzw[3] = T(cos_theta);
}

//
// operator: *=
template <class T>
inline CQuat<T>&
CQuat<T>::operator*= (const CQuat<T>& q)
{
    T new_x = W() * q.X() +  X() * q.W() + ( Y() * q.Z() - Z() * q.Y() );
    T new_y = W() * q.Y() +  Y() * q.W() + ( Z() * q.X() - X() * q.Z() );
    T new_z = W() * q.Z() +  Z() * q.W() + ( X() * q.Y() - Y() * q.X() );
    T new_w = W() * q.W() - (X() * q.X() + Y() * q.Y() + Z() * q.Z());

    m_Xyzw[0] = new_x;
    m_Xyzw[1] = new_y;
    m_Xyzw[2] = new_z;
    m_Xyzw[3] = new_w;

    return *this;
}

//
// operator: -=
template <class T>
inline CQuat<T>&
CQuat<T>::operator-= (const CQuat<T>& q)
{
    X() -= q.X();
    Y() -= q.Y();
    Z() -= q.Z();
    W() -= q.W();

    return *this;
}

//
// operator: +=
template <class T>
inline CQuat<T>&
CQuat<T>::operator+= (const CQuat<T>& q)
{
    X() += q.X();
    Y() += q.Y();
    Z() += q.Z();
    W() += q.W();

    return *this;
}


//
// create a rotation matrix from a quaternion
template <class T>
inline CMatrix4<T>
CQuat<T>::ToMatrix() const
{
    CMatrix4<T> m;
    ToMatrix(m);
    return m;
}


template <class T>
inline void
CQuat<T>::ToMatrix(CMatrix4<T>& m) const
{
    T Nq = m_Xyzw[0]*m_Xyzw[0] +
        m_Xyzw[1]*m_Xyzw[1] +
        m_Xyzw[2]*m_Xyzw[2] +
        m_Xyzw[3]*m_Xyzw[3];
    T s = (Nq > 0) ? (T(2.0) / Nq) : 0;
    T xs = m_Xyzw[0]*s;
    T ys = m_Xyzw[1]*s;
    T zs = m_Xyzw[2]*s;
    T wx = m_Xyzw[3]*xs;
    T wy = m_Xyzw[3]*ys;
    T wz = m_Xyzw[3]*zs;
    T xx = m_Xyzw[0]*xs;
    T xy = m_Xyzw[0]*ys;
    T xz = m_Xyzw[0]*zs;
    T yy = m_Xyzw[1]*ys;
    T yz = m_Xyzw[1]*zs;
    T zz = m_Xyzw[2]*zs;

    m(0,0) = T(1.0) - (yy + zz);
    m(1,0) = xy + wz;
    m(2,0) = xz - wy;
    m(0,1) = xy - wz;
    m(1,1) = T(1.0) - (xx + zz);
    m(2,1) = yz + wx;
    m(0,2) = xz + wy;
    m(1,2) = yz - wx;
    m(2,2) = T(1.0) - (xx + yy);
    m(0,3) = T(0.0);
    m(1,3) = T(0.0);
    m(2,3) = T(0.0);
    m(3,0) = T(0.0);
    m(3,1) = T(0.0);
    m(3,2) = T(0.0);
    m(3,3) = T(1.0);
}

//
// return the conjugate of a quaternion
template <class T>
inline CQuat<T>
CQuat<T>::Conjugate() const
{
    CQuat<T> q(*this);
    q.X() = -q.X();
    q.Y() = -q.Y();
    q.Z() = -q.Z();
    return q;
}

//
// get the length(magnitude) of a quaternion
template <class T>
inline T
CQuat<T>::Length() const
{
    return sqrt(Length2());
}

//
// get the length(magnitude) squared of a quaternion
template <class T>
inline T
CQuat<T>::Length2() const
{
    return (m_Xyzw[0]*m_Xyzw[0] +
            m_Xyzw[1]*m_Xyzw[1] +
            m_Xyzw[2]*m_Xyzw[2] +
            m_Xyzw[3]*m_Xyzw[3]);
}

//
// get the inverse of a quaternion
template <class T>
inline CQuat<T>
CQuat<T>::Inverse() const
{
    return (Conjugate() / Length2());
}

//
// rotate a point using a quaternion
template <class T>
inline void
CQuat<T>::Rotate(CVect3<float>& point) const
{
    // formula is qPq', where
    // P = (0,p)
    CQuat<T> p(point.X(), point.Y(), point.Z(), 0);

    // there's a little extra math done in here
    // W() is 0, so we could eliminate this
    p = *this * p;
    p *= Inverse();

    point.X() = p.X();
    point.Y() = p.Y();
    point.Z() = p.Z();
    // w remains 0 throughout
}

//
// streams - based output
template <class T>
ostream&
operator<< (ostream& o, const CQuat<T>& quat)
{
    o << "[(" << quat.X() << ", " << quat.Y() << ", " << quat.Z()
        << "), " << quat.W() << "]";

    return o;
}


END_NCBI_SCOPE

/* @} */

#endif  // GUI_MATH___QUAT___HPP
