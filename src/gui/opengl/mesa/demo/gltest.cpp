/*  $Id: gltest.cpp 24939 2011-12-29 14:52:50Z wuliangs $
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
 * Author:  Mike DiCuccio
 *
 * File Description:
 *    CGlTestApp -- test of OpenGL framework for rendering images as a CGI app
 */

#include <ncbi_pch.hpp>
#include <cgi/cgictx.hpp>
#include <corelib/ncbifile.hpp>

#include <gui/opengl/mesa/glcgi_image.hpp>

#include <gui/opengl/glutils.hpp>
#include <gui/opengl/glcamera.hpp>
//#include <GL/glut.h>

using namespace ncbi;


/////////////////////////////////////////////////////////////////////////////
//  CGlTestApplication::
//

class CGlTestApplication : public CGlCgiImageApplication
{
public:
    CGlTestApplication();

    virtual void Render(CCgiContext& ctx);

private:
    // camrea set-up
    CGlCamera m_Camera;
};



CGlTestApplication::CGlTestApplication()
{
    m_Camera.SetLayout(CGlCamera::ePerspective);
    m_Camera.SetFieldOfView(45.0f);
    m_Camera.SetNearPlane(0.1f);
    m_Camera.SetFarPlane(500.0f);
}


void CGlTestApplication::Render(CCgiContext& ctx)
{
    // retrieve our CGI rendering params
    const CCgiRequest& request  = ctx.GetRequest();

    const TCgiEntries& entries = request.GetEntries();

    TCgiEntries::const_iterator fov_iter = entries.find("fov");
    TCgiEntries::const_iterator dist_iter = entries.find("dist");
    TCgiEntries::const_iterator xrot_iter = entries.find("xrot");
    TCgiEntries::const_iterator yrot_iter = entries.find("yrot");
    TCgiEntries::const_iterator zrot_iter = entries.find("zrot");
    //TCgiEntries::const_iterator rad1_iter = entries.find("rad1");
    //TCgiEntries::const_iterator rad2_iter = entries.find("rad2");

    float dist = 40.0f;
    float xrot = 0.0f;
    float yrot = 0.0f;
    float zrot = 0.0f;
    //float rad1 = 2.0f;
    //float rad2 = 7.0f;

    if (fov_iter != entries.end()) {
        m_Camera.SetFieldOfView(NStr::StringToDouble(fov_iter->second));
    }

    if (dist_iter != entries.end()) {
        dist = NStr::StringToDouble(dist_iter->second);
    }

    if (xrot_iter != entries.end()) {
        xrot = NStr::StringToDouble(xrot_iter->second);
    }

    if (yrot_iter != entries.end()) {
        yrot = NStr::StringToDouble(yrot_iter->second);
    }

    if (zrot_iter != entries.end()) {
        zrot = NStr::StringToDouble(zrot_iter->second);
    }

    //if (rad1_iter != entries.end()) {
    //    rad1 = NStr::StringToDouble(rad1_iter->second);
    //}

    //if (rad2_iter != entries.end()) {
    //    rad2 = NStr::StringToDouble(rad2_iter->second);
    //}

    //
    // do some rendering
    //

    glClearColor(1.0f, 1.0f, 1.0f, 0.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(-1.0f, 1.0f, -1.0f, 1.0f, -1.0f, 1.0f);
    glBegin(GL_QUADS);
        glColor3f(0.2f, 0.2f, 0.2f);
        glVertex2f(-1.0f, 1.0f);
        glVertex2f( 1.0f, 1.0f);

        glColor3f(0.4f, 0.4f, 0.8f);
        glVertex2f( 1.0f, -1.0f);
        glVertex2f(-1.0f, -1.0f);
    glEnd();

    m_Camera.MakeCurrent();

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    //
    // lighting - done against an identity matrix
    //
    static GLfloat light_pos[]   = { -20.0f, 20.0f, 5.0f, 1.0f };
    static GLfloat white_light[] = { 1.0f, 1.0f, 1.0f, 1.0f };
    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);
    glLightModelf(GL_LIGHT_MODEL_TWO_SIDE, GL_TRUE);
    glLightfv(GL_LIGHT0, GL_POSITION, light_pos);
    glLightfv(GL_LIGHT0, GL_DIFFUSE,  white_light);
    glLightfv(GL_LIGHT0, GL_SPECULAR, white_light);

    //
    // world-view setup
    //
    glTranslatef(0.0f, 0.0f, -dist);
    glRotatef(xrot, 1.0f, 0.0f, 0.0f);
    glRotatef(yrot, 0.0f, 1.0f, 0.0f);
    glRotatef(zrot, 0.0f, 0.0f, 1.0f);


    //
    // make things a bit faster
    //
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    //glCullFace(GL_BACK);

    //
    // some interesting solids
    // we use different material properties for each
    //
    glEnable(GL_COLOR_MATERIAL);

    glColor3f(1.0f, 0.5f, 0.0f);
    glPushMatrix();
    glTranslatef(5.0f, 0.0f, 0.0f);
    //glutSolidTorus(rad1, rad2, 30, 60);
    glPopMatrix();

    glColor3f(0.0f, 1.0f, 0.0f);
    glPushMatrix();
    glTranslatef(-5.0f, 0.0f, 0.0f);
    glRotatef(90, 1.0f, 0.0f, 0.0f);
    //glutSolidTorus(rad1, rad2, 30, 60);
    glPopMatrix();

    //
    // debugging: draw our axes
    //
    glDisable(GL_LIGHTING);
    glBegin(GL_LINES);

        glColor3f (1.0f, 0.0f, 0.0f);
        glVertex3f(0.0f, 0.0f, 0.0f);
        glVertex3f(5.0f, 0.0f, 0.0f);

        glColor3f (0.0f, 1.0f, 0.0f);
        glVertex3f(0.0f, 0.0f, 0.0f);
        glVertex3f(0.0f, 5.0f, 0.0f);

        glColor3f (0.0f, 0.0f, 1.0f);
        glVertex3f(0.0f, 0.0f, 0.0f);
        glVertex3f(0.0f, 0.0f, 5.0f);

    glEnd();

    CGlUtils::DumpState();
}



/////////////////////////////////////////////////////////////////////////////
//  MAIN
//

int main(int argc, const char* argv[])
{
    int result = CGlTestApplication().AppMain(argc, argv, 0, eDS_Default, 0);
    _TRACE("back to normal diags");
    return result;
}
