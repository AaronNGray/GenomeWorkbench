#ifndef GUI_OPENGL___ARC_BALL___HPP
#define GUI_OPENGL___ARC_BALL___HPP

/*  $Id: glarcball.hpp 14973 2007-09-14 12:11:32Z dicuccio $
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

#include <gui/gui.hpp>
#include <gui/utils/quat.hpp>
#include <gui/utils/vect3.hpp>
#include <gui/utils/matrix4.hpp>

/** @addtogroup GUI_OPENGL
 *
 * @{
 */

BEGIN_NCBI_SCOPE


//
// class CGlArcBall implements an arbitrary rotation arc ball.
//
// This class interprets mouse drags as drags of a virtual globe centered at a
// position of the user's choice.  The requirements to using this are:
//   -- call BeginDrag() when the mouse drag starts
//   -- call EndDrag() when the mouse drag ends
//   -- call Update() on mouse movement in a drag as well as before calling
//      both BeginDrag() and EndDrag()
//   -- set the resolution of the window on resize
//
class NCBI_GUIOPENGL_EXPORT CGlArcBall
{
public:
    typedef CVect3<float> TVect;
    typedef CQuat<float>  TQuat;

    // ctor
    CGlArcBall(void);
    // construct at a center and radius
    CGlArcBall(const TVect& center, float radius);
    // dtor
    ~CGlArcBall(void);

    // set the screen resolution
    void        Resolution(int x, int y);

    // place the world at a center and radius
    void        Place(const TVect& center, float radius);

    // place a world at a center, keeping the current radius
    void        Place(const TVect& center);

    // update the arc ball for a given mouse position
    void        Update(int x, int y);

    // begin a drag event
    void        BeginDrag(void);

    // end a drag event
    void        EndDrag(void);

    // determine if the arc ball is in a drag state
    bool        IsDragging(void) const;

    // apply the arcball using OpenGL
    void        ApplyGL() const;

    // access the current matrix
    const CMatrix4<float>&   GetMatrix() const      { return m_MatNow; }

private:
    // boolean flag: are we dragging?
    bool         m_IsDragging;

    // screen resolution
    int          m_ScreenX;
    int          m_ScreenY;

    // current mouse position(in scaled coordinates)
    float        m_MouseX;
    float        m_MouseY;

    // mouse position at the start of the drag cycle
    CVect4 < float> m_DragFrom;

    // center of the arcball world
    CVect4 < float> m_Center;

    // radius of the arcball world
    float        m_Radius;

    // quaternions for rotation
    TQuat m_QuatNow;
    TQuat m_QuatDown;
    TQuat m_QuatDrag;

    // current rotation matrix
    CMatrix4<float> m_MatNow;


    // hidden and unimplemented
    CGlArcBall(const CGlArcBall&);
    CGlArcBall&    operator= (const CGlArcBall&);

    // convert screen coordinates to sphere coordinates
    CVect4<float>    x_ToSphere(float x, float y);
};


//
// set the screen resolution
inline
void CGlArcBall::Resolution(int x, int y)
{
    m_ScreenX = x;
    m_ScreenY = y;
}


//
// determine if the arc ballis being dragged
//
inline
bool CGlArcBall::IsDragging(void) const
{
    return m_IsDragging;
}


END_NCBI_SCOPE

/* @} */

#endif  // GUI_OPENGL___ARC_BALL___HPP
