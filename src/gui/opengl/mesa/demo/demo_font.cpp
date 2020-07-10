/*  $Id: demo_font.cpp 22039 2010-09-08 14:14:26Z kuznets $
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
#include <gui/opengl/glbitmapfont.hpp>


BEGIN_NCBI_SCOPE


////////////////////////////////////////////////////
//  CCgiFontTestApp::
//

class CCgiFontTestApp : public CGlCgiImageApplication
{
public:
    virtual void Render(CCgiContext& ctx);

protected:
    // x_PreProcess(): a hook for establishing application set-up prior to
    //creating an OpenGL context
    void x_PreProcess(CCgiContext& ctx);

private:
    // we use CGlCamera to establish our viewing space
    CGlCamera m_Camera;

};


void CCgiFontTestApp::x_PreProcess(CCgiContext& ctx)
{
    // establish width and height from CGI params
    string width_str  = ctx.GetRequest().GetEntry("width");
    string height_str = ctx.GetRequest().GetEntry("height");
    try {
        m_Width = NStr::StringToInt(width_str);
    }
    catch (std::exception&) {
        m_Width = 800;
    }

    try {
        m_Height = NStr::StringToInt(height_str);
    }
    catch (std::exception&) {
        m_Height = 600;
    }


    // set an orthographic projection
    // text is best rendered orthographically as we do not get any shear
    // distortion from projection
    // we also preserve OpenGL's notion of top/bottom as distinct from that of
    // the image or rendering system - (0,0) is the lower left corner

    m_Camera.SetLayout(CGlCamera::eOrtho);
    m_Camera.SetLeftPlane  (0);
    m_Camera.SetRightPlane (m_Width);

    m_Camera.SetTopPlane   (m_Height);
    m_Camera.SetBottomPlane(0);

    // near and far must bracket 0, as we plan to place things with a 0
    // z-coordinate
    m_Camera.SetNearPlane( 1.0f);
    m_Camera.SetFarPlane (-1.0f);
}


void CCgiFontTestApp::Render(CCgiContext& ctx)
{
    const CCgiRequest& request  = ctx.GetRequest();

    //
    // configure our font face and size
    //

    string font_face_str = request.GetEntry("font");
    string font_size_str = request.GetEntry("size");

    // we use CGlBitmapFont::FaceFromString() and
    // CGlBitmapFont::SizeFromString() to retrieve unambiguous font
    // representations
    CGlBitmapFont::EFontFace font_face =
        CGlBitmapFont::FaceFromString(font_face_str);
    if (font_face == 0) {
        font_face = CGlBitmapFont::eFontFace_Helvetica;
    }
    CGlBitmapFont::EFontSize font_size =
        CGlBitmapFont::SizeFromString(font_size_str);
    if (font_size == 0) {
        font_size = CGlBitmapFont::eFontSize_12;
    }

    CGlBitmapFont font;
    font.SetFontFace(font_face);
    font.SetFontSize(font_size);

    //
    // establish our background and setup our camera
    // when clearing with glClear(), we make sure to clear both the color and
    // depth buffers!
    //

    glClearColor(1.0, 1.0, 1.0, 0.0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    m_Camera.MakeCurrent();


    int panel_x = 10;
    int panel_y = 10;
    int panel_wid = m_Width - 20;
    int panel_ht = (m_Height - 20) / 4;

    int text_ht = (int)font.TextHeight();
    string str;

    //
    // panel 1: centered
    //

    str = "Centered text in (";
    str += NStr::IntToString(panel_x) + ", ";
    str += NStr::IntToString(panel_y) + ", ";
    str += NStr::IntToString(panel_x + panel_wid) + ", ";
    str += NStr::IntToString(panel_y + panel_ht ) + ")";
    str += " text height = " + NStr::IntToString(text_ht);
    str += " (and a tag to make it long)";

    glColor3f(0.2f, 0.0f, 0.5f);
    font.TextOut(panel_x, panel_y, panel_x + panel_wid, panel_x + panel_ht,
                 str.c_str(), IGlFont::eAlign_Center);

    glBegin(GL_LINE_LOOP);
    glVertex2f(panel_x,             panel_y);
    glVertex2f(panel_x + panel_wid, panel_y);
    glVertex2f(panel_x + panel_wid, panel_y + panel_ht);
    glVertex2f(panel_x,             panel_y + panel_ht);
    glEnd();

    //
    // panel 2: right justified
    //

    panel_y += panel_ht;

    str = "Right justified text in (";
    str += NStr::IntToString(panel_x) + ", ";
    str += NStr::IntToString(panel_y) + ", ";
    str += NStr::IntToString(panel_x + panel_wid) + ", ";
    str += NStr::IntToString(panel_y + panel_ht ) + ")";
    str += " (and a tag to make it long)";

    glColor3f(0.0f, 0.0f, 0.5f);
    font.TextOut(panel_x, panel_y, panel_x + panel_wid, panel_x + panel_ht,
                 str.c_str(), IGlFont::eAlign_Right);

    glBegin(GL_LINE_LOOP);
    glVertex2f(panel_x,             panel_y);
    glVertex2f(panel_x + panel_wid, panel_y);
    glVertex2f(panel_x + panel_wid, panel_y + panel_ht);
    glVertex2f(panel_x,             panel_y + panel_ht);
    glEnd();

    //
    // panel 3: top-left justified
    //

    panel_y += panel_ht;

    str = "Top-Left justified text in (";
    str += NStr::IntToString(panel_x) + ", ";
    str += NStr::IntToString(panel_y) + ", ";
    str += NStr::IntToString(panel_x + panel_wid) + ", ";
    str += NStr::IntToString(panel_y + panel_ht ) + ")";
    str += " (and blunt-end truncation)";

    glColor3f(0.5f, 0.0f, 0.5f);
    font.TextOut(panel_x, panel_y, panel_x + panel_wid, panel_x + panel_ht,
                 str.c_str(), IGlFont::eAlign_Top | IGlFont::eAlign_Left,
                 CGlBitmapFont::eTruncate_Empty);

    glBegin(GL_LINE_LOOP);
    glVertex2f(panel_x,             panel_y);
    glVertex2f(panel_x + panel_wid, panel_y);
    glVertex2f(panel_x + panel_wid, panel_y + panel_ht);
    glVertex2f(panel_x,             panel_y + panel_ht);
    glEnd();

    //
    // panel 4: bottom-right justified
    //

    panel_y += panel_ht;

    str = "Bottom-Right justified text in (";
    str += NStr::IntToString(panel_x) + ", ";
    str += NStr::IntToString(panel_y) + ", ";
    str += NStr::IntToString(panel_x + panel_wid) + ", ";
    str += NStr::IntToString(panel_y + panel_ht ) + ")";
    str += " (and a tag to make it long)";

    glColor3f(0.2f, 0.2f, 0.5f);
    font.SetFontSize(CGlBitmapFont::eFontSize_18);
    font.TextOut(panel_x, panel_y, panel_x + panel_wid, panel_x + panel_ht,
                 str.c_str(), IGlFont::eAlign_Bottom | IGlFont::eAlign_Right);

    glBegin(GL_LINE_LOOP);
    glVertex2f(panel_x,             panel_y);
    glVertex2f(panel_x + panel_wid, panel_y);
    glVertex2f(panel_x + panel_wid, panel_y + panel_ht);
    glVertex2f(panel_x,             panel_y + panel_ht);
    glEnd();
}

END_NCBI_SCOPE
USING_NCBI_SCOPE;



////////////////////////////////////////////////////
//  MAIN
//

int main(int argc, const char* argv[])
{
    return CCgiFontTestApp().AppMain(argc, argv, 0, eDS_Default, 0);
}
