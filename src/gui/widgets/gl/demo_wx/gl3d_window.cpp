/*  $Id: gl3d_window.cpp 23981 2011-06-30 15:09:19Z katargir $
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
 * Authors:  Mike DiCuccio
 *
 * File Description:
 */

#include <ncbi_pch.hpp>

#include "gl3d_window.hpp"

#include <gui/opengl/glutils.hpp>
#include <gui/opengl/geometry.hpp>

BEGIN_NCBI_SCOPE

BEGIN_EVENT_TABLE(CGl3dWindow, CGLCanvas)
    EVT_MOTION(CGl3dWindow::OnMouseMove)
    EVT_LEFT_DOWN(CGl3dWindow::OnMouseDown)
    EVT_LEFT_UP(CGl3dWindow::OnMouseUp)
    EVT_SIZE(CGl3dWindow::OnSize)
END_EVENT_TABLE()

CGl3dWindow::CGl3dWindow(wxWindow* parent, wxWindowID id) :
    CGLCanvas(parent, id)
    , m_State(eNormal)
    , m_Arcball(CVect3<float>(0.0f, 0.0f, 0.0f), 2.0f)
    , m_XRot(0)
    , m_YRot(0)
    , m_ZRot(0)
    , m_CenterPoint(0.0f, 0.0f, 0.0f)
    , m_Zoom(0.0f)
    , m_PrevMouseX(0)
    , m_PrevMouseY(0)
{
    m_Camera.Init3dDefault();
}

void CGl3dWindow::OnSize(wxSizeEvent& event)
{
    wxSize size = event.GetSize();
    int h = size.GetHeight();
    int w = size.GetWidth();

    m_Arcball.Resolution(w, h);
    m_Camera.SetAspectRatio(float(w) / float(h));

    event.Skip();
}

void CGl3dWindow::SetXRot(float val)
{
    m_XRot = val;
    Refresh(false);
}


void CGl3dWindow::SetYRot(float val)
{
    m_YRot = val;
    Refresh(false);
}


void CGl3dWindow::SetZRot(float val)
{
    m_ZRot = val;
    Refresh(false);
}

void CGl3dWindow::SetPerspective()
{
    m_Camera.SetLayout(CGlCamera::ePerspective);
    Refresh(false);
}

void CGl3dWindow::SetOrtho()
{
    m_Camera.SetLayout(CGlCamera::eOrtho);
    Refresh(false);
}

void CGl3dWindow::x_SetupScene()
{
    //
    // apply our arcball and view transforms
    // the order of operations here is critical!
    // we aim for:
    //
    // - center point adjustment:
    //   - move the viewpoint back from the front plane
    //   - perform our zooming
    //   - adjust the center point for panning
    // - model-space rotations
    //   - Euler angles first
    //   - finally, arcball adjustments
    //

    // first, translate the viewing center point back from the front plane
    glTranslatef(0.0f, 0.0f, -40.0f);

    // adjust for our zooming scale factor
    // this can be combined with the above step, but is separated here for
    // clarity
    glTranslatef(0.0f, 0.0f, m_Zoom * 40.0f);

    // adjust the center point for our panning options
    // this again can be combined with the above operation, but is separated
    // here for clarity
    glTranslatef(m_CenterPoint[0], m_CenterPoint[1], m_CenterPoint[2]);

    // Euler angle rotation:
    // we use three distinct angles rotated about each axis
    // the glRotate() call is ideal here
    glRotatef(m_XRot, 1.0f, 0.0f, 0.0f);
    glRotatef(m_YRot, 0.0f, 1.0f, 0.0f);
    glRotatef(m_ZRot, 0.0f, 0.0f, 1.0f);

    // finally, arcball rotation
    // "arcball rotation" involves interpolated quaternions adjusted for a
    // mouse movement.  The math is fairly hairy; the class CGlArcball
    // encapsulates this
    m_Arcball.ApplyGL();
}


void CGl3dWindow::x_Render()
{
    wxSize size = GetClientSize();
    int h = size.GetHeight();
    int w = size.GetWidth();

    glViewport(0, 0, w, h);

    glClearColor(0.2f, 0.2f, 0.4f, 0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // misc enables
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_LIGHTING);
   // glEnable(GL_CULL_FACE);
    glEnable(GL_COLOR_MATERIAL);
    glEnable(GL_POLYGON_SMOOTH);

    m_Material.SetShininess(128);
    m_Material.SetSpecular(CRgbaColor(1.0f, 1.0f, 1.0f, 1.0f));
    m_Material.Apply();

    m_Camera.MakeCurrent();

    // switch on the diffuse light
    m_Light1.SetDiffuse(CRgbaColor(0.5f, 0.5f, 0.8f, 1.0f));
    m_Light1.On();

    // switch on spotlight
    m_Light2.SetPosition(CGlLight::TVect(0.0f, 0.0f, 10.0f));
    m_Light2.SetDirection(CGlLight::TVect(-0.5f, 1.0f, -15.0f));
    m_Light2.SetDiffuse(CRgbaColor(1.0f, 1.0f, 1.0f, 1.0f));
    m_Light2.SetCutoff(20.0f);
    m_Light2.On();

    x_SetupScene();

    //
    // draw something interesting
    //

    if (m_Dlist.IsValid()) {
        m_Dlist.Call();
    } else {
        CGlDisplayListCompile COMPILE(m_Dlist,
                                      CGlDisplayList::eCompileAndExecute);

        glPushMatrix();
        glColor3f(1.0f, 0.5f, 0.0f);
        glTranslatef(5.0f, 0.0f, 0.0f);
        CGlGeometry::DrawCylinder(CVect3<float>(0.0f, 0.0f, 0.0f),
                                  CVect3<float>(10.0f, 0.0f, 0.0f),
                                  1.0f, 5, 20);

        glPopMatrix();

        glPushMatrix();
        glColor3f(0.0f, 1.0f, 0.0f);
        glTranslatef(-5.0f, 0.0f, 0.0f);
        CGlGeometry::DrawCylinder(CVect3<float>(0.0f, 0.0f, 0.0f),
                                  CVect3<float>(0.0f, 10.0f, 0.0f),
                                  1.0f, 5, 20);

        glPopMatrix();
    }


    // draw some axes as well
    //glDisable(GL_LIGHTING);
    glBegin(GL_LINES);
        glColor3f (1.0f, 0.0f, 0.0f);
        glVertex3f( 1.0f, 0.0f, 0.0f);
        glVertex3f(-1.0f, 0.0f, 0.0f);

        glColor3f (0.0f, 1.0f, 0.0f);
        glVertex3f(0.0f, 1.0f, 0.0f);
        glVertex3f(0.0f, -1.0f, 0.0f);

        glColor3f (0.0f, 0.0f, 1.0f);
        glVertex3f(0.0f, 0.0f, 1.0f);
        glVertex3f(0.0f, 0.0f, -1.0f);
    glEnd();

    CGlUtils::CheckGlError();
}

void CGl3dWindow::OnMouseMove(wxMouseEvent& event)
{
    if (!HasCapture())
        return;

    switch (m_State) {
    case eNormal:
        if (event.ShiftDown()) {
            // zoom
            m_State = eZoom;
            x_Zoom(event.m_x, event.m_y);
        } else if (event.ControlDown()) {
            // pan
            m_State = ePan;
            x_Pan(event.m_x, event.m_y);
        } else {
            // rotate
            m_State = eRotate;
            x_Rotate(event.m_x, event.m_y);
        }
        break;
    case eRotate:
        //
        // rotate - activated on left click-drag
        //
        if ( !event.ShiftDown() &&  !event.ControlDown()) {
            x_Rotate(event.m_x, event.m_y);
        } else {
            // end rotate
            m_Arcball.EndDrag();
            m_State = eNormal;
            ReleaseMouse();
        }
        break;

    case ePan:
        //
        // pan - activated on ctrl-left-click-drag
        //
        if (event.ControlDown()) {
            x_Pan(event.m_x, event.m_y);
        } else {
            // end pan
            m_State = eNormal;
            ReleaseMouse();
        }
        break;

    case eZoom:
        //
        // zoom - activated on shift-left-click-drag
        //
        if (event.ShiftDown()) {
            x_Zoom(event.m_x, event.m_y);
        } else {
            // end pan
            m_State = eNormal;
            ReleaseMouse();
        }
        break;
    }

    m_PrevMouseX = event.m_x;
    m_PrevMouseY = event.m_y;

    Refresh(false);
}

void CGl3dWindow::OnMouseDown(wxMouseEvent& event)
{
    m_PrevMouseX = event.m_x;
    m_PrevMouseY = event.m_y;
    CaptureMouse();
}

void CGl3dWindow::OnMouseUp(wxMouseEvent& event)
{
    if (HasCapture()) {
        if (m_State == eRotate) {
            m_Arcball.Update(event.m_x, event.m_y);
            m_Arcball.EndDrag();
        }
        m_PrevMouseX = event.m_x;
        m_PrevMouseY = event.m_y;
        m_State = eNormal;
        ReleaseMouse();
    }
}

void CGl3dWindow::x_Rotate(long x, long y)
{
    m_Arcball.Update(x, y);
    if (!m_Arcball.IsDragging())
        m_Arcball.BeginDrag();
}

void CGl3dWindow::x_Zoom(long /*x*/, long y)
{
    int delta = y - m_PrevMouseY;
    m_Zoom += float(delta) / float(GetClientSize().GetHeight());
}

void CGl3dWindow::x_Pan(long x, long y)
{
    /// need to call make_current() for glProject()/glUnProject() to work
    //make_current();

    x_SetupGLContext();

    wxSize size = GetClientSize();
    int h = size.GetHeight();
    int w = size.GetWidth();
    glViewport(0, 0, w, h);

    m_Camera.MakeCurrent();
    x_SetupScene();

    /// retrieve the bits of OpenGL state that are necessary
    GLdouble modelview[16];
    GLdouble projection[16];
    GLint viewport[4];
    glGetDoublev(GL_MODELVIEW_MATRIX, modelview);
    glGetDoublev(GL_PROJECTION_MATRIX, projection);
    glGetIntegerv(GL_VIEWPORT, viewport);

    /// first, we project the current center point to the scren corrdinates
    GLdouble x1;
    GLdouble y1;
    GLdouble z1;
    gluProject(m_CenterPoint[0], m_CenterPoint[1], m_CenterPoint[2],
               modelview, projection, viewport,
               &x1, &y1, &z1);

    /// next, adjust the screen coordinates for our delta values
    /// remember that OpenGL flips the Y axis coordinates from that of
    /// all windowing systems!
    x1 += x - m_PrevMouseX;
    y1 += m_PrevMouseY - y;

    /// lastly, we un-project back to model coordinates
    GLdouble x2;
    GLdouble y2;
    GLdouble z2;
    gluUnProject(x1, y1, z1,
                 modelview, projection, viewport,
                 &x2, &y2, &z2);
    m_CenterPoint[0] = x2;
    m_CenterPoint[1] = y2;
    m_CenterPoint[2] = z2;
}

END_NCBI_SCOPE
