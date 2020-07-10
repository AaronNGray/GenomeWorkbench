/*  $Id: gltexturefont.cpp 45022 2020-05-08 02:01:25Z evgeniev $
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
 * Authors:  Bob Falk
 *
 * File Description:
 *
 */

#include <ncbi_pch.hpp>

#include <gui/opengl/gltexturefont.hpp>
#include <gui/opengl/ftglfontmanager.hpp>
#include <gui/opengl/glutils.hpp>
#include <gui/opengl/irender.hpp>

#include <corelib/ncbifile.hpp>

#include <math.h>
#include <stdio.h>
#include <sstream>

#include <FTGL/ftgl.h>


BEGIN_NCBI_SCOPE



string CGlTextureFont::s_FontFileNames[] = {      
        "NimbusSanL-Regu.ttf",      // Helvetica
        "NimbusSanL-Bold.ttf",
        "NimbusSanL-ReguItal.ttf",   
        "NimbusSanL-BoldItal.ttf",
        "LucidaBrightRegular.ttf",   // Lucida
        "LucidaBrightDemiBold.ttf",
        "LucidaBrightItalic.ttf",
        "LucidaBrightDemiItalic.ttf",
        "NimbusMonL-Regu.ttf",       //Courier
        "NimbusMonL-Bold.ttf",
        "NimbusMonL-ReguObli.ttf",
        "NimbusMonL-BoldObli.ttf",
        "dark-courier.ttf",           //Fixed
        "dark-courier-bold.ttf",
        "dark-courier-italic.ttf",
        "dark-courier-bold-italic.ttf",
        "NimbusRomanNo9-Reg.ttf",     // Times-Roman
        "NimbusRomanNo9-Med.ttf",
        "NimbusRomanNo9-Ita.ttf",
        "NimbusRomanNo9-MedIta.ttf",
        "BPmono.ttf", // BPMono - free monospaced bold font by: http://www.1001fonts.com/users/steffmann/
        "BPmonoBold.ttf",
        "BPmonoItalics.ttf",
        "BPmonoBoldStencil.ttf", 
        "LastFontDummy.NotAFile",
        "Helvetica6.ttf",   // Bitmap
        "Helvetica8.ttf",   // Bitmap
        "Helvetica10.ttf"   // Bitmap
};

string CGlTextureFont::s_FontNames[] = {
        "Helvetica",      // Helvetica
        "Helvetica Bold",
        "Helvetica Oblique",
        "Helvetica Bold Oblique",
        "Lucida",        // Lucida
        "Lucida Bold",
        "Lucida Italic",
        "Lucida Bold Italic",
        "Courier",         //Courier
        "Courier Bold",
        "Courier Oblique",
        "Courier Bold Oblique",
        "Fixed",         //Fixed
        "Fixed Bold",
        "Fixed Oblique",
        "Fixed Bold Oblique",
        "Times-Roman",    // Times-Roman
        "Times-Roman Bold",
        "Times-Roman Italic",
        "Times-Roman Bold Italic",
        "BPMono",         // BPMono (monospaced font)
        "BPMono Bold",
        "BPMono Italic",
        "BPMono Bold Stencil",
        "Last Font Dummy",
        "Helvetica Bitmap 6",  // Bitmap
        "Helvetica Bitmap 8",  // Bitmap
        "Helvetica Bitmap 10"  // Bitmap
};

int CGlTextureFont::s_FontSizes[] = {
6, 7, 8, 9, 10, 11, 12, 14, 16, 18, 21, 24, 36, 48, 60, 72, 0
};


CGlTextureFont::CGlTextureFont()
: m_Font(NULL)
, m_FontFace(eFontFace_LastFont)
, m_FontSize(12)
, m_Rotate(fFontRotateBase)
, m_SnapToPixelX(true)
, m_SnapToPixelY(true)
, m_PrevShader(0)
{
    SetFontFace(eFontFace_Helvetica);
}

CGlTextureFont::CGlTextureFont(const string& font_file_name, 
                               unsigned int font_size)
: m_Font(NULL)
, m_FontFace(eFontFace_LastFont)
, m_FontSize(font_size)
, m_Rotate(fFontRotateBase)
, m_SnapToPixelX(true)
, m_SnapToPixelY(true)
, m_PrevShader(0)
{
    SetFont(font_file_name, font_size);
}

CGlTextureFont::CGlTextureFont(EFontFace face, 
                               unsigned int font_size)
: m_Font(NULL)
, m_FontFace(face)
, m_FontSize(font_size)
, m_Rotate(fFontRotateBase)
, m_SnapToPixelX(true)
, m_SnapToPixelY(true)
, m_PrevShader(0)
{
    SetFontFace(face);
}

void CGlTextureFont::SetFontFace(EFontFace face, bool use_bitmap_overrides)
{
    string font_file = x_GetFontFile(face, m_FontFile, m_FontSize, use_bitmap_overrides);
    m_FontFace = face;

    if (font_file != m_FontFile)
        SetFont(font_file, m_FontSize, use_bitmap_overrides);
}

CGlTextureFont::EFontFace CGlTextureFont::GetFontFace() const
{
    return m_FontFace;
}

void CGlTextureFont::SetFontSize(unsigned int size)
{
    m_FontSize = size;

    string font_file = m_FontFile;

    if (font_file != "")
        SetFont(font_file, m_FontSize);
}

unsigned int CGlTextureFont::GetFontSize() const
{
    return m_FontSize;
}


bool CGlTextureFont::SetFont(string font_file_name, 
                             unsigned int font_size,
                             bool use_bitmap_overrides)
{
    CFtglFontManager& fm = CFtglFontManager::Instance();

    font_file_name = x_GetFontFile(m_FontFace, font_file_name, font_size, use_bitmap_overrides);

    FTFont* font = fm.GetFont(font_file_name.c_str(), font_size);

    /// Error loading - don't change current font selection
    if (font==NULL || font->Error())
        return false;
   
    m_FontFile = font_file_name;
    m_Font = font;
    m_FontSize = font_size;	

	x_ComputeMetrics();

    return true;
}

string CGlTextureFont::GetFontFileForFace(EFontFace face)
{
    return s_FontFileNames[face];
}

// This function mirrors x_GetFontFile and what files it chooses to return.
bool CGlTextureFont::IsBitmapFont() const
{
    if ((m_FontFace == eFontFace_Helvetica ||
         m_FontFace == eFontFace_Fixed) && 
         (m_FontSize == 6 || m_FontSize == 8 || m_FontSize == 10)) {
             return true;
    }

    return false;
}


void CGlTextureFont::SetSnapToPixel(bool xpix, bool ypix)
{
    m_SnapToPixelX = xpix;
    m_SnapToPixelY = ypix;
}

void CGlTextureFont::BeginText() const
{
    /// Get viewport from OpenGL
	GetGl().GetViewport(m_Viewport);
    BeginText(TVPRect(m_Viewport[0], 
                      m_Viewport[1], 
                      m_Viewport[2], 
                      m_Viewport[3]));
}

void CGlTextureFont::BeginText(const TVPRect& viewport) const
{
    // avoid pusattrib if possible since it can be very slow
    //glPushAttrib(GL_ALL_ATTRIB_BITS);
    IRender& gl = GetGl();

    // Copy viewport passed in (we can't get it from OpenGL always because
    // this library also supports some pdf rendering functions and in pdf 
    // output the maximum viewing area generally exceeds the maximum 
    // supported by OpenGL)
    m_Viewport[0] = GLint(viewport.Left());
    m_Viewport[1] = GLint(viewport.Bottom());
    m_Viewport[2] = GLint(viewport.Right());
    m_Viewport[3] = GLint(viewport.Top());
    
    // Get modelview and projection matrices that will be used for 
    // unprojection (getting window coordinates) of text draw location	
    gl.GetModelViewMatrix(m_ModelviewMatrix);
    gl.GetProjectionMatrix(m_ProjectionMatrix);

    // Set projection matrix to pixel coordinates
    gl.MatrixMode(GL_PROJECTION);
    gl.PushMatrix();
    gl.LoadIdentity();
    gl.Ortho(m_Viewport[0], 
             m_Viewport[0] + m_Viewport[2],
             m_Viewport[1], 
             m_Viewport[1] + m_Viewport[3], -1.0, 1.0);
    gl.MatrixMode(GL_MODELVIEW);

    // set up the ogl state for text
    gl.Disable(GL_LIGHTING);
    gl.Disable(GL_DEPTH_TEST);
    gl.Disable(GL_CULL_FACE);
    gl.Enable(GL_TEXTURE_2D);
    gl.TexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
    //gl.Enable(GL_BLEND);
    //gl.BlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);  
    gl.PolygonMode(GL_FRONT_AND_BACK, GL_FILL);    

    if (gl.GetApi() == eOpenGL20)
        glGetIntegerv(GL_CURRENT_PROGRAM, &m_PrevShader);
    gl.UseProgram(0);
    CGlUtils::CheckGlError();
}
    
void CGlTextureFont::BeginText(const TVPRect& viewport, 
                               GLdouble* mview, GLdouble* proj ) const
{
    // avoid pusattrib if possible since it can be very slow
    //glPushAttrib(GL_ENABLE_BIT | GL_TRANSFORM_BIT);
    IRender& gl = GetGl();

    // Copy viewport passed in (we can't get it from OpenGL always because
    // this library also supports some pdf rendering functions and in pdf 
    // output the maximum viewing area generally exceeds the maximum 
    // supported by OpenGL)
    m_Viewport[0] = GLint(viewport.Left());
    m_Viewport[1] = GLint(viewport.Bottom());
    m_Viewport[2] = GLint(viewport.Right());
    m_Viewport[3] = GLint(viewport.Top());
    
    // Get modelview and projection matrices that will be used for 
    // unprojection (getting window coordinates) of text draw location	
    memcpy(m_ModelviewMatrix, mview, sizeof(GLdouble)*16);
    memcpy(m_ProjectionMatrix, proj, sizeof(GLdouble)*16);


    //glGetDoublev(GL_MODELVIEW_MATRIX, m_ModelviewMatrix);
    //glGetDoublev(GL_PROJECTION_MATRIX, m_ProjectionMatrix);
    //glGetIntegerv(GL_VIEWPORT, m_Viewport);

    // Set projection matrix to pixel coordinates
    gl.MatrixMode(GL_PROJECTION);
    gl.PushMatrix();
    gl.LoadIdentity();
    gl.Ortho(m_Viewport[0], 
             m_Viewport[0] + m_Viewport[2],
             m_Viewport[1], 
             m_Viewport[1] + m_Viewport[3], -1.0, 1.0);
    gl.MatrixMode(GL_MODELVIEW);

    // set up the ogl state for text
    gl.Disable(GL_LIGHTING);
    gl.Disable(GL_DEPTH_TEST);
    gl.Disable(GL_CULL_FACE);
    gl.Enable(GL_TEXTURE_2D);
    gl.TexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
    //glEnable(GL_BLEND);
    //glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);  
    gl.PolygonMode(GL_FRONT_AND_BACK, GL_FILL);        

    glGetIntegerv(GL_CURRENT_PROGRAM, &m_PrevShader);
    gl.UseProgram(0);
    CGlUtils::CheckGlError();
}

void CGlTextureFont::EndText() const
{   
    // restore OpenGL state (push/pop all is expensive)
    //glPopAttrib();
    IRender& gl = GetGl();

    gl.MatrixMode(GL_PROJECTION);
    gl.PopMatrix();   
    gl.MatrixMode(GL_MODELVIEW);

    // set most reasonable options (rather than push/pop attrib)
    gl.Disable(GL_TEXTURE_2D);
    gl.Enable(GL_BLEND);
    gl.BlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA); 
    glListBase(0);    
    gl.UseProgram((GLuint)m_PrevShader);
    CGlUtils::CheckGlError();    
}

void CGlTextureFont::WriteText(TModelUnit x, TModelUnit y,
                               const char* text,
                               const float* color,
                               TModelUnit rotate_degrees) const
{
    GetGl().Color4fv(color);
    WriteText(x, y, text, rotate_degrees);
}

void CGlTextureFont::WriteText(TModelUnit x, TModelUnit y,
                               TModelUnit width, TModelUnit height,
                               const char* text,
                               const float* color,
                               TAlign align,
                               ETruncate trunc,
                               TModelUnit rotate_degrees) const
{
    GetGl().Color4fv(color);
    WriteText(x, y, width, height, text, align, trunc, rotate_degrees);
}
 
void CGlTextureFont::WriteText(TModelUnit x, 
                               TModelUnit y, 
                               const char* text,
                               TModelUnit rotate_degrees) const
{
    // If rotation is between 90 and 270, the text will be backwards and upside down
    // unless we re-orient.  Check if user provided reorient flag.
    bool reorient_text = false;    
    if (rotate_degrees != TModelUnit(0)) {        
        TModelUnit rads = rotate_degrees*(TModelUnit(3.141592653589732)/TModelUnit(180));
        bool backwards = cos(double(rads)) < TModelUnit(0);
        if ((m_Rotate & fReorientText) && backwards) {
            reorient_text = true;             
        }
    }

    IRender& gl = GetGl();

    // Get the window (projection) coordinates of the drawing position (x,y)
    GLdouble wx, wy, wz;
    gluProject(x, y, 0.0, 
               m_ModelviewMatrix, m_ProjectionMatrix, m_Viewport, 
               &wx, &wy, &wz);

    gl.MatrixMode(GL_MODELVIEW);
    gl.PushMatrix();
    gl.LoadIdentity();

    // Round the drawing coordinates to nearest integer pixel coordinate
    // (drawing textured fonts on pixel boundries improves visual appearance)
    // unless user has turned option off
    double xi = wx;
    double yi = wy;

    if (m_SnapToPixelX)
        xi = floor(wx + 0.5);
    if (m_SnapToPixelY)
        yi = floor(wy + 0.5);

    TModelUnit rcenterx = (TModelUnit)0.0;
    TModelUnit rcentery = (TModelUnit)0.0;

    if (m_Rotate & fFontRotateMid) {
        rcentery = TextHeight()/(TModelUnit)2.0;
    }
    else if (m_Rotate & fFontRotateCap) {
        rcentery = TextHeight();
    }
    // fFontRotateBase is default

    gl.Translated(xi, yi, 0.0);

    // rotate if desired
    if (rotate_degrees != TModelUnit(0.0)) {
        gl.Translated((GLdouble)rcenterx, (GLdouble)rcentery, 0.0);
        gl.Rotated(rotate_degrees, 0.0, 0.0, 1.0);
        gl.Translated((GLdouble)-rcenterx, (GLdouble)-rcentery, 0.0);
        
        // If we have rotated text to be upside-down and backwards, flip it
        // across the x and y axes
        if (reorient_text) {
            gl.Translatef(0.0f, float(TextHeight()), 0.0f);
            gl.Scalef(1.0f, -1.0f, 1.0f);
            gl.Translatef(float(TextWidth(text)), 0.0f, 0.0f);
            gl.Scalef(-1.0f, 1.0f, 1.0f);
        }
    }

    TextOut(text);

    gl.MatrixMode(GL_MODELVIEW);
    gl.PopMatrix();
}

void CGlTextureFont::WriteText(TModelUnit x, TModelUnit y, 
                               TModelUnit width, TModelUnit height,
                               const char* text,
                               TAlign align,
                               ETruncate trunc,
                               TModelUnit rotate_degrees) const
{
    string output_str(text);
    x_WriteText(x, y, width, height, output_str, align, trunc, rotate_degrees, true);
}

CMatrix4<double> CGlTextureFont::GetTextXform(TModelUnit x, TModelUnit y, 
                                              TModelUnit width, TModelUnit height,
                                              string& text,
                                              TAlign align,
                                              ETruncate trunc,
                                              TModelUnit rotate_degrees) const
{
    return x_WriteText(x, y, width, height, text, align, trunc, rotate_degrees, false);
}

void CGlTextureFont::ProjectVertex(CVect2<float>& vertex) const
{
    double px, py;
    double dummyz;

    gluProject(vertex.X(), vertex.Y(), 0.0, m_ModelviewMatrix, m_ProjectionMatrix, m_Viewport, &px, &py, &dummyz);
    vertex.Set(float(px), float(py));
}

CMatrix4<double> CGlTextureFont::x_WriteText(TModelUnit x, TModelUnit y, 
                                             TModelUnit width, TModelUnit height,
                                             string& text,
                                             TAlign align,
                                             ETruncate trunc,
                                             TModelUnit rotate_degrees,
                                             bool write_text) const
{
    TModelUnit w = width;
    TModelUnit h = height;

    // If rotation is between 90 and 270, the text will be backwards and upside down
    // unless we re-orient.  Check if user provided reorient flag.
    bool reorient_text = false;    
    if (rotate_degrees != TModelUnit(0)) {        
        TModelUnit rads = rotate_degrees*(TModelUnit(3.141592653589732)/TModelUnit(180));
        bool backwards = cos(double(rads)) < TModelUnit(0);
        if ((m_Rotate & fReorientText) && backwards) {
            reorient_text = true;             
        }
    } 

    /// truncate as appropriate
    string str;
    switch (trunc) {
    case eTruncate_None:
        str = text;
        break;

    case eTruncate_Empty:
    case eTruncate_Ellipsis:
        {    
            if (reorient_text) {
                string rtext = text;

                // Reverse text so that it is truncated on the 
                // correct side 
                std::reverse(rtext.begin(), rtext.end());
                x_Truncate(rtext.c_str(), w, trunc, &str);
                std::reverse(str.begin(), str.end());
            }
            else {
                x_Truncate(text.c_str(), w, trunc, &str);
            }
        }
        break;
    }

    // update parameter to return (possibly truncated) text
    text = str;

    ///
    /// determine where our text starts
    ///
    TModelUnit tx = 0;
    TModelUnit ty = 0;

    // scale is the ratio of the visible width/height divided by the 
    // viewport width/height (same as in CGLPane).  Assumes of course
    // user is using orthographic projection matrix (e.g. glOrtho())
    TModelUnit scale_x = (TModelUnit(2)/TModelUnit(m_ProjectionMatrix[0]))/
        TModelUnit(m_Viewport[2]);
    TModelUnit scale_y = (TModelUnit(2)/TModelUnit(m_ProjectionMatrix[5]))/
        TModelUnit(m_Viewport[3]);

    // eAlign_Right or eAlign_Center are applied to horz position only if
    // eAlign_Left is not specifed
    if ((align & eAlign_HorizMask) == eAlign_Left)  {
        tx = x;
    } else if ((align & eAlign_HorizMask) == eAlign_Right)  {
        // right justify
        TModelUnit text_wid = TextWidth(str.c_str())*scale_x;
        tx = x + (w - text_wid);
    } else  { // center
        TModelUnit text_wid = TextWidth(str.c_str())*scale_x;
        tx = x + (w - text_wid) * TModelUnit(0.5);
    }

    if ((align & eAlign_VertMask) == eAlign_Bottom)  {
        ty = y;
    } else if ((align & eAlign_VertMask) == eAlign_Top) {
        // skip down enough to keep our text inside
        TModelUnit text_ht = TextHeight()*scale_y;
        ty = y + (h - text_ht);
    } else {
        // if nothing given or eAlign_Center is specified - center
        // vertically
        TModelUnit text_ht = TextHeight()*scale_y;
        ty = y + (h - text_ht) * TModelUnit(0.5);
    }

    IRender& gl = GetGl();

    //
    // Set the matrices to pixel-coordinates and then render
    //

    // Get window projection coordnates for (x,y)
    GLdouble wx, wy, wz;
    gluProject(tx, ty, 0.0, 
               m_ModelviewMatrix, m_ProjectionMatrix, m_Viewport, 
               &wx, &wy, &wz);

    gl.MatrixMode(GL_MODELVIEW);
    gl.PushMatrix();
    gl.LoadIdentity();

    // Round the drawing coordinates to nearest integer pixel coordinate
    // (drawing textured fonts on pixel boundries improves visual appearance)
    // unless user has turned option off
    double xi = wx;
    double yi = wy;

    if (m_SnapToPixelX)
        xi = floor(wx + 0.5);
    if (m_SnapToPixelY)
        yi = floor(wy + 0.5);

    // If a rotation angle is specified, we want to rotate from the base
    // of the box, not the text.  So rotate the text around the bottom
    // middle, or top of the base side of the box as appropriate.
    if (rotate_degrees != TModelUnit(0.0)) {
        TModelUnit rcenterx = (TModelUnit)0.0;
        TModelUnit rcentery = (TModelUnit)0.0;

        // Get screen coordinates to which (x,y) (box base position) projects
        GLdouble xproj, yproj, zproj;
        gluProject(x, y, 0.0, 
                   m_ModelviewMatrix, m_ProjectionMatrix, m_Viewport, 
                   &xproj, &yproj, &zproj);    

        // Get offset of box needed for rotation center
        if (m_Rotate & fFontRotateMid) {
            rcentery = height/(TModelUnit)2.0;
        }
        else if (m_Rotate & fFontRotateCap) {
            rcentery = height;
        }       

        double xoff = xi-xproj;
        double yoff = yi-yproj;

        gl.Translated(xi, yi, 0.0);
        gl.Translated((GLdouble)rcenterx-xoff, (GLdouble)rcentery-yoff, 0.0);
        gl.Rotated(rotate_degrees, 0.0, 0.0, 1.0);
        gl.Translated((GLdouble)-rcenterx+xoff, (GLdouble)-rcentery+yoff, 0.0);

        // If we have rotated text to be upside-down and backwards, flip it
        // across the x and y axes
        if (reorient_text) {
            gl.Translatef(0.0f, float(TextHeight()), 0.0f);
            gl.Scalef(1.0f, -1.0f, 1.0f);
            gl.Translatef(float(TextWidth(str.c_str())), 0.0f, 0.0f);
            gl.Scalef(-1.0f, 1.0f, 1.0f);
        }

        ///
        /// Debugging code to draw the bounding box for the text
        ///
        /*
        glMatrixMode(GL_MODELVIEW);
        glPushMatrix();
        glLoadIdentity();

        double wx2, wy2, wz2;
        gluProject(x+width, y+height, 0.0, 
            m_ModelviewMatrix, m_ProjectionMatrix, m_Viewport, 
            &wx2, &wy2, &wz2);

        glTranslated((GLdouble)xproj+rcenterx, (GLdouble)yproj+rcentery, 0.0);
        glRotated(rotate_degrees, 0.0, 0.0, 1.0);
        glTranslated((GLdouble)-(xproj+rcenterx), (GLdouble)-(yproj+rcentery), 0.0);

        glDisable(GL_TEXTURE_2D);
        glDisable(GL_BLEND);
        glColor3f(0.0f, 0.0f, 1.0f);
        glBegin(GL_LINE_LOOP);
        glVertex3d(xproj, yproj, 0.0f);
        glVertex3d(wx2, yproj, 0.0f);
        glVertex3d(wx2, wy2, 0.0f);
        glVertex3d(xproj, wy2, 0.0f);
        glEnd();
        glEnable(GL_TEXTURE_2D);
        glEnable(GL_BLEND);
        glMatrixMode(GL_MODELVIEW);
        glPopMatrix();
        */
    }
    else {
        gl.Translated(xi, yi, 0.0);
    }

    CMatrix4<double> xform;
    xform.Identity();

    // If we write text matrix will be invalid (only do one or the other
    // since both can slow down performance)
    if (write_text) {
        TextOut(str.c_str());
    }
    else {    
        gl.GetModelViewMatrix(xform.GetData());
        xform.Transpose();
    }

    gl.MatrixMode(GL_MODELVIEW);
    gl.PopMatrix();

    return xform;
}

void CGlTextureFont::TextOut(const char* text) const
{
    if (m_Font != NULL && !m_Font->Error())
        m_Font->Render(text);
}

void CGlTextureFont::TextOut(TModelUnit x, 
                             TModelUnit y, 
                             const char* text) const
{
    BeginText();
    WriteText(x, y, text);
    EndText();
}

void CGlTextureFont::TextOut(TModelUnit x1, TModelUnit y1, 
                             TModelUnit x2, TModelUnit y2, 
                             const char* text,
                             TAlign align,
                             ETruncate trunc,
                             TModelUnit scale_x,
                             TModelUnit scale_y) const
{
    // Handle alignment and truncation and scaling (scaling seems be used by the ruler class - maybe not much else
    if(x2 < x1) {
        swap(x1, x2);
    }
    if(y2 < y1) {
        swap(y1, y2);
    }

    TModelUnit x = x1;
    TModelUnit y = y1;
    TModelUnit w = x2 - x1;
    TModelUnit h = y2 - y1;

    /// truncate as appropriate
    string str;
    switch (trunc) {
    case eTruncate_None:
        str = text;
        break;

    case eTruncate_Empty:
    case eTruncate_Ellipsis:
        {
            x_Truncate(text, w, trunc, &str);
        }
        break;
    }

    BeginText();

    ///
    /// determine where our text starts
    ///
    TModelUnit tx = 0;
    TModelUnit ty = 0;

    // scale is the ratio of the visible width/height divided by the 
    // viewport width/height (same as in CGLPane).  Assumes of course
    // user is using orthographic projection matrix (e.g. glOrtho())
    scale_x = (TModelUnit(2)/TModelUnit(m_ProjectionMatrix[0]))/
        TModelUnit(m_Viewport[2]);
    scale_y = (TModelUnit(2)/TModelUnit(m_ProjectionMatrix[5]))/
        TModelUnit(m_Viewport[3]);

    // eAlign_Right or eAlign_Center are applied to horz position only if
    // eAlign_Left is not specifed
    if ((align & eAlign_HorizMask) == eAlign_Left)  {
        tx = x;
    } else if ((align & eAlign_HorizMask) == eAlign_Right)  {
        // right justify
        TModelUnit text_wid = TextWidth(str.c_str()) * scale_x;
        tx = x + (w - text_wid);
    } else  { // center
        TModelUnit text_wid = TextWidth(str.c_str()) * scale_x;
        tx = x + (w - text_wid) * TModelUnit(0.5);
    }

    if ((align & eAlign_VertMask) == eAlign_Bottom)  {
        ty = y;
    } else if ((align & eAlign_VertMask) == eAlign_Top) {
        // skip down enough to keep our text inside
        TModelUnit text_ht = TextHeight() * scale_y;
        ty = y + (h - text_ht);
    } else {
        // if nothing given or eAlign_Center is specified - center
        // vertically
        ty = (y1 + y2 - GetMetric(eMetric_CharHeight))/TModelUnit(2.0);
    }  
  
    WriteText(x, y, text);
    EndText();
}

string CGlTextureFont::Truncate(const char* text, 
                                TModelUnit w, 
                                ETruncate trunc) const
{
    string res;
    x_Truncate2(text, w, trunc, &res);
    return res;
}

string CGlTextureFont::Truncate(const string& text, 
                                TModelUnit w, 
                                ETruncate trunc) const
{
    string res;
    x_Truncate2(text.c_str(), w, trunc, &res);
    return res;
}

void CGlTextureFont::ArrayTextOut(TModelUnit x, TModelUnit y, 
                                  TModelUnit dx, TModelUnit dy,
                                  const char* text,
                                  const vector<CRgbaColor*>* colors,
                                  TModelUnit scale_x,
                                  TModelUnit scale_y) const
{
    TModelUnit k_x = scale_x / 2;
    TModelUnit k_y = scale_y / 2;

    TModelUnit ch = TextHeight();

    BeginText();
    for (const char* p = text;  p  &&  *p;  ++p) {
        //if (*p  <   m_FontPtr->first ||
        //    *p  >=  m_FontPtr->first + m_FontPtr->num_chars) {
        //    continue;
        //}
        //const BitmapCharRec *ch = m_FontPtr->ch[*p - m_FontPtr->first];
        string text_char(p, 1);
        TModelUnit cw = TextWidth(text_char.c_str());

        int ind = (int)(p - text);
        TModelUnit pos_x = x + ind * dx;
        TModelUnit pos_y = y + ind * dy;
        TModelUnit off_x;
        TModelUnit off_y;

        off_x = cw * k_x; // to current coord system
        off_y = ch * k_y; // to current coord system

        if(colors)  {
            CRgbaColor& c = *(*colors)[ind];
            GetGl().ColorC(c);
        }

        WriteText(pos_x-off_x, pos_y, text_char.c_str(), 0.0f);
        CGlUtils::CheckGlError();
    }

    EndText();
}

TModelUnit CGlTextureFont::TextWidth(const char* text) const
{
	if (m_Font != NULL && !m_Font->Error()) {
        FTBBox bounding;
        bounding = m_Font->BBox(text);
        return (TModelUnit)(bounding.Upper().X() - bounding.Lower().X());
    }

    return TModelUnit(0.0);
}

TModelUnit CGlTextureFont::TextHeight(void) const
{
    // This returns the height of the fonts without descenders - so it's a little
    // less that the full height.  This does not use any OpenGL (so it can be called
    // before window creation unlike some of the functions in GetMetric())
    if (m_Font != NULL && !m_Font->Error()) {
        TModelUnit fs = (TModelUnit)m_Font->FaceSize();

        // bitmap fonts are reporting slightly incorrect sizes
        // which leads to misalignment in some glyphs
        if (IsBitmapFont()) {
            if (m_FontSize == 6 || m_FontSize == 8)
                fs += 1;
            else if (m_FontSize == 10)
                fs -= 1;
        }

        // descender is given as a negative number
        TModelUnit desc = (TModelUnit)m_Font->Descender();
        return fs + desc;
    }

    return TModelUnit(m_FontSize);
}

TModelUnit CGlTextureFont::GetFontDescender() const
{
    if (m_Font != NULL && !m_Font->Error())
        return (TModelUnit)m_Font->Descender();

    return TModelUnit(0.0);
}

TModelUnit CGlTextureFont::GetAdvance(char c) const
{
    if (m_Font != NULL && !m_Font->Error()) {
        char str[] = {c, '\0'}; // FTGL code accesses the next char
        return m_Font->Advance(str, 1);
    }

    return TModelUnit(0.0f);
} 

 TModelUnit CGlTextureFont::GetMaxWidth(int max_num) const
{
    double max_dig_w = 0;

    for( char c = '0'; c <= '9'; c++) {
        double char_w = GetAdvance(c);
        max_dig_w = max(max_dig_w, char_w);
    }
    double comma_w = GetAdvance(',');

    double mod = abs(max_num);
    int digits_count = (int) ceil(log10(mod));
    int commas_count = (digits_count - 1) / 3;

    // length of the longest possible label in pixels (add 2 for separation)
    double w = digits_count * max_dig_w + commas_count * comma_w;
    if(max_num < 0) {
        w += GetAdvance('-');
    }
    return w; 
}


TModelUnit CGlTextureFont::GetMetric(EMetric metric, 
                                     const char* text,
                                     int len) const
{
    FTBBox bounding;

    // Standard characters (could also iterate over ascii codes).
    static const char* sc_chars = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz1234567890`~!@#$%^&*()_-+={}|[]\\:\";'<,>.?/";
    // just the caps
    static const char* sc_caps = "ABCDEFGHIJKLMNOPQRSTUVWXYZ";

    float w = 0.0f;

    // Note -some of these use OpenGL and should not be called before window creation
    if (m_Font != NULL && !m_Font->Error()) {
        switch(metric) {
            case eMetric_CharHeight: 
                if (m_MetricCharHeight < TModelUnit(0.0)) {
                    bounding = m_Font->BBox(sc_caps);
                    m_MetricCharHeight =  bounding.Upper().Y() - bounding.Lower().Y();
                }
				return m_MetricCharHeight;             
                break;
            case eMetric_FullCharHeight:    
                return m_MetricFullCharHeight;
                break;
            case eMetric_AvgCharWidth:
                if (m_MetricAvgCharWidth < 0.0f) {
                    w = m_Font->Advance(sc_chars);
                    m_MetricAvgCharWidth = (TModelUnit(w)/TModelUnit(strlen(sc_chars)));
                }
                return m_MetricAvgCharWidth;                
                break;
            case eMetric_MaxCharWidth:
                if ( m_MetricMaxCharWidth < TModelUnit(0.0) ) {
                    for (size_t i=0; i<strlen(sc_chars); ++i) {
                        w = m_Font->Advance(&sc_chars[i], 1);
                        if (w > m_MetricMaxCharWidth)
                            m_MetricMaxCharWidth = w;
                    }
                }
                return m_MetricMaxCharWidth;
                break;
            case eMetric_TextWidth: // Does not include the final advance     
                if (text != NULL) {
                    bounding = m_Font->BBox(text);
                    return (TModelUnit)(bounding.Upper().X() - bounding.Lower().X());
                }
                break;
            case eMetric_FullTextWidth: // Includes the final advance
                if (text != NULL)
                    return (TModelUnit)m_Font->Advance(text);
                break;
            case eMetric_Descender:              
                return m_MetricDescender;
        }
    }

    return TModelUnit(0.0);
}

void CGlTextureFont::x_ComputeMetrics()
{
    FTBBox bounding;

    // Note -some of these use OpenGL and should not be called before window creation
    if (m_Font != NULL && !m_Font->Error()) {		
       
        // this should not be called before window is created so do it in
        // get function
        //bounding = m_Font->BBox(sc_caps);
        //m_MetricCharHeight =  bounding.Upper().Y() - bounding.Lower().Y();
        m_MetricCharHeight = TModelUnit(-1.0);
		
        m_MetricFullCharHeight = (TModelUnit)m_Font->FaceSize();		
     		
        //w = m_Font->Advance(sc_chars);
        //m_MetricAvgCharWidth = (TModelUnit(w)/TModelUnit(strlen(sc_chars))); 		
        m_MetricAvgCharWidth = TModelUnit(-1.0);


        m_MetricMaxCharWidth = TModelUnit(-1.0);

        m_MetricDescender = (TModelUnit)m_Font->Descender();
    }
}

string CGlTextureFont::FaceToString(EFontFace face)
{
    return s_FontNames[face];
}

CGlTextureFont::EFontFace CGlTextureFont::FaceFromString(const string& str)
{
    vector<string> arr;
    NStr::Split(str, " \t", arr);

    // If all tokens (such as Courier, bold, oblique occur in
    // a string return the enum for that string, otherwise return 
    // eFontFace_LastFont
    for (size_t i=0; i<(size_t)eFontFace_LastFont; ++i) {
        //string str = s_FontNames[i];
        bool match = true;

        for (size_t j=0; j<arr.size(); ++j) {
            size_t pos = NStr::Find(s_FontNames[i], arr[j], NStr::eNocase);
            if (pos == NPOS)
                match = false;
        }

        if (match) 
            return (EFontFace)i;
    }

    return eFontFace_LastFont;
}

string CGlTextureFont::PdfBaseFontName(EFontFace face, EFontFace& pdf_face)
{
    // These are all the base-font names defined by PDF (these fonts can be
    // embedded into pdf files, but do not have to be).
    /*
        Times-Roman
        Times-Bold
        Times-Italic
        Times-BoldItalic
        Helvetica
        Helvetica-Bold
        Helvetica-Oblique
        Helvetica-BoldOblique
        Courier
        Courier-Bold
        Courier-Oblique
        Courier-BoldOblique
        Symbol
        ZapfDingbats
    */

    // This is the best mapping to built in pdf font names.  Some are only
    // approximations.  PDF does also allow fonts to be embedded, if the pdf
    // rendering were to support it.
    switch (face) {
        case eFontFace_Helvetica:
            pdf_face = eFontFace_Helvetica;
            return "Helvetica";
        case eFontFace_Helvetica_Bold:
            pdf_face = eFontFace_Helvetica_Bold;
            return "Helvetica-Bold";
        case eFontFace_Helvetica_Italic:
            pdf_face = eFontFace_Helvetica_Italic;
            return "Helvetica-Oblique";
        case eFontFace_Helvetica_BoldItalic:
            pdf_face = eFontFace_Helvetica_BoldItalic;
            return "Helvetica-BoldOblique";
        case eFontFace_Lucida:
            pdf_face = eFontFace_TimesRoman;
            return "Times-Roman";
        case eFontFace_Lucida_Bold:
            pdf_face = eFontFace_TimesRoman_Bold;
            return "Times-Bold";
        case eFontFace_Lucida_Italic:
            pdf_face = eFontFace_TimesRoman_Italic;
            return "Times-Italic";
        case eFontFace_Lucida_BoldItalic:
            pdf_face = eFontFace_TimesRoman_BoldItalic;
            return "Times-BoldItalic";
        case eFontFace_Courier:
            pdf_face = eFontFace_Courier;
            return "Courier";
        case eFontFace_Courier_Bold:
            pdf_face = eFontFace_Courier_Bold;
            return "Courier-Bold";
        case eFontFace_Courier_Italic:
            pdf_face = eFontFace_Courier_Italic;
            return "Courier-Oblique";
        case eFontFace_Courier_BoldItalic:
            pdf_face = eFontFace_Courier_BoldItalic;
            return "Courier-BoldOblique";
        case eFontFace_Fixed:
            pdf_face = eFontFace_Courier;
            return "Courier";
        case eFontFace_Fixed_Bold:
            pdf_face = eFontFace_Courier_Bold;
            return "Courier-Bold";
        case eFontFace_Fixed_Italic:
            pdf_face = eFontFace_Courier_Italic;
            return "Courier-Oblique";
        case eFontFace_Fixed_BoldItalic:
            pdf_face = eFontFace_Courier_BoldItalic;
            return "Courier-BoldOblique";
        case eFontFace_TimesRoman:
            pdf_face = eFontFace_TimesRoman;
            return "Times-Roman";
        case eFontFace_TimesRoman_Bold:
            pdf_face = eFontFace_TimesRoman_Bold;
            return "Times-Bold";
        case eFontFace_TimesRoman_Italic:
            pdf_face = eFontFace_TimesRoman_Italic;
            return "Times-Italic";
        case eFontFace_TimesRoman_BoldItalic:
            pdf_face = eFontFace_TimesRoman_BoldItalic;
            return "Times-BoldItalic";
        case eFontFace_BPMono:
            pdf_face = eFontFace_Courier;
            return "Courier";
        case eFontFace_BPMono_Bold:
            pdf_face = eFontFace_Courier_Bold;
            return "Courier-Bold";
        case eFontFace_BPMono_Italic:
            pdf_face = eFontFace_Courier_Italic;
            return "Courier-Oblique";
        case eFontFace_BPMono_BoldStencil:
            pdf_face = eFontFace_Courier_Bold;
            return "Courier-Bold";
        default:
            pdf_face = eFontFace_LastFont;
            return "";
    }

    return "";
}

// saves and restores font face and size in(from) a string
string CGlTextureFont::ToString() const
{
    string size_str = NStr::NumericToString(m_FontSize);
    string font_string = m_FontFile + " " + size_str;

    return font_string;
}
bool CGlTextureFont::FromString(const string& value)
{
    string font_size_str;
    NStr::SplitInTwo(value, ",", m_FontFile, font_size_str);
    NStr::StringToNumeric(font_size_str, &m_FontSize);

    if (m_FontSize >= 4 && m_FontFile != "") {       
        m_FontFace = eFontFace_LastFont;

        /// Set face name if we recognize the font file:
        for (size_t i=0; i<(size_t)eFontFace_LastFont; ++i) {
            if (s_FontFileNames[i] == m_FontFile) {
                m_FontFace = (EFontFace)i;               
                break;
            }
        }       

        // If it was a bitmap font, we call it Helvetica (but it 
        // could also be Fixed).
        for (size_t i=(size_t)eFontFace_LastFont; 
             i<(size_t)eFontFace_LastBitmapFont; ++i) {
                if (s_FontFileNames[i] == m_FontFile) {
                    m_FontFace = eFontFace_Helvetica;              
                break;
            }
        }

        return SetFont(m_FontFile, m_FontSize);
    }

    return false;
}
    
void CGlTextureFont::GetAllFaces(vector<string>& faces)
{
    faces.clear();

    for (size_t i=0; i<(size_t)eFontFace_LastFont; ++i) {
        faces.push_back(s_FontNames[i]);
    }
}
void CGlTextureFont::GetAllSizes(vector<string>& sizes)
{
    sizes.clear();

    size_t idx = 0;
    while (s_FontSizes[idx] != 0) {
        sizes.push_back(NStr::NumericToString(s_FontSizes[idx++]));
    }
}

int CGlTextureFont::x_Truncate2(const char* text,
    TModelUnit w,
    ETruncate trunc,
    string* str) const
{
    if (m_Font == nullptr || m_Font->Error())
        return 0;

    string original_text{ text };
    string short_text{ text };
    size_t active_index = short_text.length();

    if (eTruncate_None == trunc)
        return (int)short_text.length();

    FTBBox text_bbox = m_Font->BBox(short_text.c_str());
    TModelUnit text_width = TModelUnit(text_bbox.Upper().X() - text_bbox.Lower().X());

    FTBBox ellipsis_bbox = m_Font->BBox("...");
    TModelUnit ellipsis_width = TModelUnit(ellipsis_bbox.Upper().X() - ellipsis_bbox.Lower().X());

    if (w < ellipsis_width) {
        return 0;
    }

    while (text_width > w) {
        short_text = original_text.substr(0, --active_index);
        if (eTruncate_Ellipsis == trunc) {
            short_text += "...";
        }
        text_bbox = m_Font->BBox(short_text.c_str());
        text_width = TModelUnit(text_bbox.Upper().X() - text_bbox.Lower().X());
    }

    if (nullptr != str) {
        if (active_index != original_text.length())
            *str = short_text;
        else
            *str = text;
    }
    
    return (int)active_index;

}


int CGlTextureFont::x_Truncate(const char* text, 
                               TModelUnit w, 
                               ETruncate trunc,
                               string* str) const
{
    if (m_Font == NULL || m_Font->Error())
        return 0;

    static const char* sc_ellipsis = "...";
    static const FTBBox sc_ebbox = m_Font->BBox(sc_ellipsis);
    static const TModelUnit sc_ewid = TModelUnit(sc_ebbox.Upper().X() - 
                                                 sc_ebbox.Lower().X());

    const char* ellip = NULL;

    const char* end_pos = text;
    switch (trunc) {
    case eTruncate_None:
        end_pos = text + strlen(text);
        break;

    case eTruncate_Ellipsis:    {
            // first, do a blunt cut
            int pos = x_Truncate(text, w, eTruncate_Empty);
            end_pos = text + pos;
            if ( !*end_pos ) {
                // we can fit the whole thing
                break;
            }

            // adjust for the ellipsis
            TModelUnit e_wid = sc_ewid;
            while (e_wid > 0) {
                ellip = sc_ellipsis;
                FTBBox b = m_Font->BBox(end_pos, 1);
                e_wid -= TModelUnit(b.Upper().X() - b.Lower().X());
                if (end_pos > text) {
                    --end_pos;
                } else {
                    break;
                }
            }
        }
        break;

    case eTruncate_Empty:
        for ( ;  end_pos  &&  *end_pos  && w >= 0;  ++end_pos) {
            TModelUnit wid = (TModelUnit)m_Font->Advance(end_pos, 1);
            if (w - wid < 0) {
                FTBBox b = m_Font->BBox(end_pos, 1);
                wid = TModelUnit(b.Upper().X() - b.Lower().X());
            }

            w -= wid;
        }

        if (w < 0) {
            --end_pos;
        }
        break;
    }

    if (end_pos < text) {
        end_pos = text;
    }

    if (str) {
        str->assign(text, end_pos);
        if (ellip) {
            *str += ellip;
        }
    }
    return (int)(end_pos - text);
}

string CGlTextureFont::x_GetFontFile(EFontFace face, 
                                     string font_file_name, 
                                     unsigned int font_size,
                                     bool use_bitmap_overrides) const
{    
    // This function returns bitmap fonts for small sized Helvetica or Fixed fonts.  Currently 
    // just hardcoded, but if we had more bitmap fonts we could have an array of font file names for
    // each font face.  Note that fixed and helvetica use the same bitmap fonts and it Only 
    // applies to sizes 6, 8, and 10 (since you have to find/create fonts for each size).
    if (use_bitmap_overrides && (face == eFontFace_Helvetica ||
         face == eFontFace_Fixed) && 
         (font_size == 6 || font_size == 8 || font_size == 10)) {

             if (font_size == 6) { 
                 return s_FontFileNames[(int)eFontFace_Helvetica6];  
             }
             else if (font_size == 8) {
                 return s_FontFileNames[(int)eFontFace_Helvetica8]; 
             }
             else {
                 return s_FontFileNames[(int)eFontFace_Helvetica10]; 
             }
    }
    else {
        // If the file was loaded directly, there may not be a face name, so 
        // just return the file (which the calller will have to set correctly).
        if (face != eFontFace_LastFont)
            return s_FontFileNames[(int)face];  
        else
            return font_file_name;
    }
}

END_NCBI_SCOPE
