#ifndef GUI_OPENGL___GLRECT__HPP
#define GUI_OPENGL___GLRECT__HPP

/*  $Id: glrect.hpp 31696 2014-11-06 18:37:59Z falkrb $
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

#include <corelib/ncbistd.hpp>
#include <gui/opengl/glpoint.hpp>

/** @addtogroup GUI_OPENGL
 *
 * @{
 */

BEGIN_NCBI_SCOPE



/// CGlRect reprsents rectangle in the coordinate system with origin located in
/// the left bottom corner.


template <class T>  class CGlRect
{
public:
    // Construction
    CGlRect()
        : m_Left(0), m_Bottom(0), m_Right(0), m_Top(0)  {}
    CGlRect(T x, T y)
        : m_Left(x), m_Bottom(y), m_Right(x), m_Top(y)  {}
    CGlRect(T left, T bottom, T right, T top)
        : m_Left(left), m_Bottom(bottom), m_Right(right), m_Top(top) {}

    void    Init()
    {
        m_Left = m_Bottom = m_Right = m_Top = 0;
    }

    void    Init(T x, T y)
    {
        m_Left = m_Right = x;
        m_Bottom = m_Top = y;
    }

    void    Init(T left, T bottom, T right, T top)
    {
        m_Left = left;
        m_Bottom = bottom;
        m_Right = right;
        m_Top = top;
    }

    T   Left()   const   {   return m_Left;  }
    T   Bottom() const   {   return m_Bottom;  }
    T   Right()  const   {   return m_Right;  }
    T   Top()    const   {   return m_Top;  }

    T  Width() const
    {
        return m_Right - m_Left;
    }
    T   Height() const
    {
        return m_Top - m_Bottom;
    }
    CGlPoint<T>     CenterPoint() const
    {
        return CGlPoint<T>( (m_Left + m_Right) / 2, (m_Bottom + m_Top) / 2);
    }
    // return corners 0..3 starting with lower left and proceeding 
    // counter clockwise, or 0,0 for invalid queries.
    CGlPoint<T>     GetCorner(int idx)
    {
        switch (idx) {
            case 0: return CGlPoint<T>( m_Left, m_Bottom);
            case 1: return CGlPoint<T>( m_Right, m_Bottom);
            case 2: return CGlPoint<T>( m_Right, m_Top);
            case 3: return CGlPoint<T>( m_Left, m_Top);
        }
        
        return CGlPoint<T>((T)0, (T)0);
    }

    void    SetLeft(T left)      {   m_Left = left;  }
    void    SetBottom(T bottom)  {   m_Bottom = bottom;  }
    void    SetRight(T right)    {   m_Right = right;  }
    void    SetTop(T top)        {   m_Top = top;  }

    void    SetHorz(T left, T right)
    {
        m_Left = left;
        m_Right = right;
    }

    void    SetVert(T bottom, T top)
    {
        m_Bottom = bottom;
        m_Top = top;
    }

    void   SetSize(T width, T height)
    {
        m_Right = m_Left + width;
        m_Top = m_Bottom + height;
    }

    void   MoveLeft(T shift)    {   m_Left += shift;    }
    void   MoveRight(T shift)   {   m_Right += shift;    }
    void   MoveBottom(T shift)  {   m_Bottom += shift;    }
    void   MoveTop(T shift)     {   m_Top += shift;    }

    // tests
    bool    operator==(const CGlRect<T>& rc) const
    {
        return m_Left == rc.m_Left && m_Right == rc.m_Right
                && m_Bottom == rc.m_Bottom  &&  m_Top == rc.m_Top;
    }
    bool    operator!=(const CGlRect<T>& rc) const
    {
        return !(*this == rc);
    }
    bool    IsEmpty()   const
    {
        return (m_Left == m_Right) || (m_Bottom == m_Top);
    }
    bool    PtInRect(T x, T y)  const
    {
        return ((x >= m_Left  &&  x <= m_Right)  ||  (x <= m_Left  &&  x >= m_Right))
                &&  ((y >= m_Bottom  &&  y <= m_Top)  ||  (y <= m_Bottom  &&  y >= m_Top));
    }
    bool    PtInRect(const CGlPoint<T>& pt)  const
    {
        return PtInRect(pt.X(), pt.Y());
    }
    bool    Intersects(const CGlRect& R) const
    {
        bool direct_x = (m_Right > m_Left) ||
                        (m_Right == m_Left  &&  R.m_Right > R.m_Left);
        bool int_x = direct_x ? ! ((R.m_Left > m_Right) || (R.m_Right < m_Left))
                                : ! ((R.m_Left < m_Right) || (R.m_Right > m_Left));

        bool direct_y = (m_Top > m_Bottom)  ||
                        (m_Top == m_Bottom  &&  R.m_Top >= R.m_Bottom);
        bool int_y = direct_y ? ! ((R.m_Bottom > m_Top) || (R.m_Top < m_Bottom))
                                : ! ((R.m_Bottom < m_Top) || (R.m_Top > m_Bottom));
        return int_x  &&  int_y;
    }

    // operations
    void Inflate(T d_x, T d_y)
    {
        m_Left -= d_x;
        m_Right += d_x;
        m_Bottom -= d_y;
        m_Top += d_y;
    }

    void    Offset(T d_x, T d_y)
    {
        m_Left += d_x;
        m_Right += d_x;
        m_Bottom += d_y;
        m_Top += d_y;
    }

    CGlRect&    IntersectWith(const CGlRect& r)
    {
        bool direct_x = (m_Right > m_Left) ||
                        (m_Right == m_Left  &&  r.m_Right > r.m_Left);
        if(direct_x)   {
            m_Left = max(m_Left, r.m_Left);
            m_Right = min(m_Right, r.m_Right);
            if (m_Right < m_Left) m_Right = m_Left;
        } else {
            // reversed direction
            m_Left = min(m_Left, r.m_Left);
            m_Right = max(m_Right, r.m_Right);
            if (m_Left < m_Right) m_Left = m_Right;
        }
        bool direct_y = (m_Top >= m_Bottom)  ||
                        (m_Top == m_Bottom  &&  r.m_Top >= r.m_Bottom);
        if(direct_y)   {
            m_Bottom = max(m_Bottom, r.m_Bottom);
            m_Top = min(m_Top, r.m_Top);
            if (m_Top < m_Bottom) m_Top = m_Bottom;
        } else {
            // reversed direction
            m_Bottom = min(m_Bottom, r.m_Bottom);
            m_Top = max(m_Top, r.m_Top);
            if (m_Bottom < m_Top) m_Bottom = m_Top;
        }
        return *this;
    }

    CGlRect&    CombineWith(const CGlRect& r)
    {
        bool direct_x = (m_Right > m_Left) ||
                        (m_Right == m_Left  &&  r.m_Right > r.m_Left);
        if(direct_x)   {
            m_Left = min(m_Left, r.m_Left);
            m_Right = max(m_Right, r.m_Right);
        } else {
            // reversed direction
            m_Left = max(m_Left, r.m_Left);
            m_Right = min(m_Right, r.m_Right);
        }
        bool direct_y = (m_Top >= m_Bottom)  ||
                        (m_Top == m_Bottom  &&  r.m_Top >= r.m_Bottom);
        if(direct_y)   {
            m_Bottom = min(m_Bottom, r.m_Bottom);
            m_Top = max(m_Top, r.m_Top);
        } else {
            // reversed direction
            m_Bottom = max(m_Bottom, r.m_Bottom);
            m_Top = min(m_Top, r.m_Top);
        }
        return *this;
    }

    inline string  ToString() const;
private:
    T m_Left;
    T m_Bottom;
    T m_Right;
    T m_Top;
};


template<> inline int CGlRect<int>::Width() const
{
    return m_Right - m_Left + 1;
}

template<> inline int CGlRect<int>::Height() const
{
    return m_Top - m_Bottom + 1;
}


template<> inline void   CGlRect<int>::SetSize(int width, int height)
{
    m_Right = m_Left + width - 1;
    m_Top = m_Bottom + height - 1;
}


template<> inline string  CGlRect<int>::ToString() const
{
    string s = "Left " + NStr::IntToString(m_Left);
    s += ", Right "  + NStr::IntToString(m_Right);
    s += ", Bottom "  + NStr::IntToString(m_Bottom);
    s += ", Top "  + NStr::IntToString(m_Top);
    return s;
}


template<> inline string  CGlRect<double>::ToString() const
{
    string s = "Left " + NStr::DoubleToString(m_Left);
    s += ", Right "  + NStr::DoubleToString(m_Right);
    s += ", Bottom "  + NStr::DoubleToString(m_Bottom);
    s += ", Top "  + NStr::DoubleToString(m_Top);
    return s;
}

END_NCBI_SCOPE

/* @} */

#endif  // GUI_OPENGL___GLRECT__HPP
