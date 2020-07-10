/*  $Id: preview_setup_widget.cpp 42696 2019-04-03 19:52:39Z katargir $
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

#include <gui/widgets/wx/preview_setup_widget.hpp>

#include <gui/widgets/wx/wx_utils.hpp>
#include <gui/widgets/wx/ui_command.hpp>

#include <gui/opengl/gltexture.hpp>
#include <gui/opengl/gltexturefont.hpp>

#include <util/image/image.hpp>

#include <gui/utils/command.hpp>
#include <gui/utils/vect2.hpp>
#include <gui/opengl/glutils.hpp>


#include <wx/event.h>
#include <wx/sizer.h>
#include <wx/menu.h>


BEGIN_NCBI_SCOPE

BEGIN_EVENT_TABLE( CGlPreviewSetupWidget, CGLCanvas )
    EVT_LEFT_DOWN(CGlPreviewSetupWidget::OnMouseDown)
    EVT_LEFT_DCLICK(CGlPreviewSetupWidget::OnMouseDoubleClick)
END_EVENT_TABLE()


CGlPreviewSetupWidget::CGlPreviewSetupWidget(CRef<CImage> img,
                                             wxWindow* parent, 
                                             wxWindowID id,
                                         const wxPoint& pos,
                                         const wxSize& size,
                                         long style,
                                         int* attriblist)
    : CGLCanvas(parent, id, pos, size, style)
    , m_ReferenceImage(new CGlTexture(img.GetNCPointer()) )
    , m_ReferenceSubImage(NULL)
    , m_Rotated(false)
    , m_AspectRatio(-1.0f)
    , m_PartitionsX(0)
    , m_PartitionsY(0)
    , m_RectWidth(0.0f)
    , m_RectHeight(0.0f)
    , m_SavedX(-1)
    , m_SavedY(-1)
    , m_ZoomX(true)
    , m_ZoomY(true)
{
    m_ReferenceImage->SetTexEnv(GL_REPLACE);

    m_RefImageSize = CVect2<int>(img->GetWidth(), img->GetHeight());
}


CGlPreviewSetupWidget::~CGlPreviewSetupWidget()
{
    x_SetupGLContext();

    if (m_ReferenceImage) {
        m_ReferenceImage->Clear();
        delete m_ReferenceImage;
    }

    if (m_ReferenceSubImage) {
        m_ReferenceSubImage->Clear();
        delete m_ReferenceSubImage;
    }
}

int CGlPreviewSetupWidget::GetImageWidth() const 
{ 
    return (int)m_RefImageSize.X(); 
}

int CGlPreviewSetupWidget::GetImageHeight() const 
{ 
    return (int)m_RefImageSize.Y();
}

void CGlPreviewSetupWidget::SetPartitions(int p)
{
    // Remove reference tile (may be brought up by double-clicking)
    // when ever the partitions change.
    delete m_ReferenceSubImage;
    m_ReferenceSubImage = NULL;

    float image_width = (float)m_RefImageSize.X(); 
    float image_height = (float)m_RefImageSize.Y();

    float w = image_width; 
    float h = image_height;

    float r = w/h;
    // 1 page implies no partitions, so start with p-1:
    int s = p-1;

    int prevx = m_PartitionsX;
    int prevy = m_PartitionsY;

    m_PartitionsX = 0;
    m_PartitionsY = 0;

    // If desired aspect ratio is not defined (<0.0f) we assign a desired
    // aspect ratio of 1.0f.  Don't worry about aspect ratio here if
    // user is zooming in x or y only.
    float aspect_ratio = (m_AspectRatio > 0) ? m_AspectRatio : 1.0f;

    if (m_ZoomY && !m_ZoomX) {
        m_PartitionsY = s;
    }
    else if (m_ZoomX && !m_ZoomY) {
        m_PartitionsX = s;
    }
    else {
        while (s>0) {
            if (aspect_ratio > r) {
                ++m_PartitionsY;
                h = image_height/(((float)m_PartitionsY) + 1.0f);
            }
            else {
                ++m_PartitionsX;
                w = image_width/(((float)m_PartitionsX) + 1.0f);
            }

            r = w/h;
            --s;
        }
    }

    // With a new subdivision, saves no longer can be shown
    if (m_PartitionsX != prevx || m_PartitionsY != prevy) {
        m_SavedX = -1;
        m_SavedY = -1;
    }
}

void CGlPreviewSetupWidget::SetAspectRatio(float r)
{    
    m_AspectRatio = r;    
}

void CGlPreviewSetupWidget::SetNumberingFormat(IImageGrabber::eImageNumberingFormat fmt)
{
    m_NumberingFormat = fmt;
}

void CGlPreviewSetupWidget::SetGLContext()
{
    x_SetupGLContext();
}

void CGlPreviewSetupWidget::SetZoomBehavior(bool zoomx, bool zoomy)
{
    m_ZoomX = zoomx;
    m_ZoomY = zoomy; 
}

void CGlPreviewSetupWidget::RefreshImage(int x_idx,
                                         int y_idx)
{
    m_SavedX = x_idx;
    m_SavedY = y_idx;

    Refresh();
}

void CGlPreviewSetupWidget::ImageSaved(int x_idx,
                                       int y_idx)
{
    RefreshImage(x_idx, y_idx);

    // Force immediate redraw so user can see progress.
    // (We could also use the Update() function, but that doesn't work on Linux
    //  and on Windows it causes the hourglass cursor to flash - the hourglass
    //  cursor animation seems to restart everytime update is called).
    x_Render();
    SwapBuffers();
}

void CGlPreviewSetupWidget::SetPreviewSubImage(CRef<CImage> img) 
{
    x_SetupGLContext();

    if (m_ReferenceSubImage) {
        m_ReferenceSubImage->Clear();
        delete m_ReferenceSubImage;
        m_ReferenceSubImage = nullptr;
    }

    m_ReferenceSubImage = new CGlTexture();
    m_ReferenceSubImage->SetFilterMag(GL_LINEAR);
    m_ReferenceSubImage->SetFilterMin(GL_LINEAR_MIPMAP_NEAREST);
    m_ReferenceSubImage->Swallow(img.GetNCPointer());

    x_Render();
    SwapBuffers();
}

void CGlPreviewSetupWidget::SetReferenceImage(CGlTexture* refimg)
{
    x_SetupGLContext();

    if (m_ReferenceImage) {
        m_ReferenceImage->Clear();
        delete m_ReferenceImage;
        m_ReferenceImage = nullptr;
    }

    m_ReferenceImage = refimg;

    x_Render();
    SwapBuffers();
}

void CGlPreviewSetupWidget::OnMouseDown(wxMouseEvent& event)
{
    if (m_ReferenceSubImage != NULL) {
        delete m_ReferenceSubImage;
        m_ReferenceSubImage = NULL;

        x_Render();
        SwapBuffers();
    }
    else {
        wxCommandEvent evt(wxEVT_TILE_PREVIEW);

        float x = (float)event.GetPosition().x;
        float y = (float)(this->GetSize().GetY()-event.GetPosition().y);

        for (size_t i=0; i<m_TileOrigins.size(); ++i) {
            if (x >= m_TileOrigins[i].m_PixelOrigin.X() &&
                x <= m_TileOrigins[i].m_PixelOrigin.X() + m_RectWidth &&
                y >= m_TileOrigins[i].m_PixelOrigin.Y() &&
                y <= m_TileOrigins[i].m_PixelOrigin.Y() + m_RectHeight) {

                    evt.SetInt(m_TileOrigins[i].m_TileIndex.X());
                    evt.SetExtraLong(m_TileOrigins[i].m_TileIndex.Y());

                    this->GetParent()->GetEventHandler()->ProcessEvent(evt);
            }
        }
    }
}

void CGlPreviewSetupWidget::OnMouseDoubleClick(wxMouseEvent& event)
{
    /// Double click can bring up a reference image, but will not dismiss one.
    if (m_ReferenceSubImage == NULL) {
        wxCommandEvent evt(wxEVT_TILE_PREVIEW);

        float x = (float)event.GetPosition().x;
        float y = (float)(this->GetSize().GetY()-event.GetPosition().y);

        for (size_t i=0; i<m_TileOrigins.size(); ++i) {
            if (x >= m_TileOrigins[i].m_PixelOrigin.X() &&
                x <= m_TileOrigins[i].m_PixelOrigin.X() + m_RectWidth &&
                y >= m_TileOrigins[i].m_PixelOrigin.Y() &&
                y <= m_TileOrigins[i].m_PixelOrigin.Y() + m_RectHeight) {

                    evt.SetInt(m_TileOrigins[i].m_TileIndex.X());
                    evt.SetExtraLong(m_TileOrigins[i].m_TileIndex.Y());

                    this->GetParent()->GetEventHandler()->ProcessEvent(evt);
            }
        }
    }
}

void CGlPreviewSetupWidget::x_Render()
{
    int w,h;

    // Update tile position information when we draw. This info is needed in case
    // user clicks on the widget - we can then tell what tile they clicked on.
    m_TileOrigins.clear();

    SetGLContext(); 
    CGlUtils::CheckGlError();

    GetSize(&w, &h);

    glEnable(GL_TEXTURE_2D);
    glDisable(GL_LIGHTING);    
    glDisable(GL_BLEND);

    glViewport(0, 0, w, h);
    float image_aspect = ((float)w)/(float)h;
   
    glClearColor(0.5f, 0.5f, 0.5f, 0.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glColor4f(1.0f, 1.0f, 1.0f, 1.0f);

    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();       
    glLoadIdentity();

    glMatrixMode(GL_PROJECTION);            
    glPushMatrix();      
    glLoadIdentity();
    gluOrtho2D(0.0, (GLdouble)w, 0.0, (GLdouble)h);

    m_ReferenceImage->MakeCurrent();

    float scale = 1.0f;
    float tiled_w = w;
    float tiled_h = h;

    // If the aspect ratio is fixed (m_AspectRatio > 0), then the image tiles
    // on the right side and bottom of the display area may have to be 
    // padded with white space in order to force the correct aspect 
    // (width/height).
    if (m_AspectRatio > 0.0f) {
        float partition_ratio = ((float)(m_PartitionsX+1))/(float)(m_PartitionsY+1);
        float partitioned_aspect = m_AspectRatio*partition_ratio;       

        // Compute here both the scale size for the displayed image,
        // and the amount of image tiled in x and y.  The tiled area will
        // generally extend past the image area when we are using a fixed aspect
        // ratio for output.       
        if (partitioned_aspect > image_aspect) {     
            // Get scaling:
            float single_tile_height = ((float)h)/(float)(m_PartitionsY + 1);
            float single_tile_width = m_AspectRatio*single_tile_height;
            scale = ((float)w)/(single_tile_width*(float)(m_PartitionsX+1));

            // compute tiled area (area that will be saved as images)
            tiled_h = scale*(float)h;
            tiled_w = tiled_h*partitioned_aspect;
        }
        else
        {
            // Get scaling:
            float single_tile_width = ((float)w)/(float)(m_PartitionsX + 1);
            float single_tile_height = (1.0f/m_AspectRatio)*single_tile_width;
            scale = ((float)h)/(single_tile_height*(float)(m_PartitionsY+1));

            // compute tiled area (area that will be saved as images)
            tiled_w = scale*(float)w;
            tiled_h = tiled_w*(1.0f/partitioned_aspect);
        }
    }
    else {
        // Special (simpliefied) cases where we only zoom (tile) in X or Y:
        if (!m_ZoomX) {
            tiled_w = std::min(h, w)/(float)(m_PartitionsY + 1);
        }
        else if (!m_ZoomY) {
            tiled_w = w;
            tiled_h = std::min(h, w)/(float)(m_PartitionsX + 1);
        }
    }

    int image_w = tiled_w;
    int image_h = tiled_h; 
    float bottom_offset = h-(float)image_h;

    // Draw the image.  Scale it if needed for fixed-proportion aspect ratios
    // which may force us to include empty space on the top or bottom of the
    // image.
    if (!m_Rotated) {
        glBegin(GL_QUADS);
            glTexCoord2f(0.0f, 0.0f);
            glVertex3f(0.0f, bottom_offset, 0.0f);

            glTexCoord2f(1.0f, 0.0f);
            glVertex3f((float)image_w, bottom_offset, 0.0f);

            glTexCoord2f(1.0f, 1.0f);
            glVertex3f((float)image_w, bottom_offset + (float)image_h, 0.0f);

            glTexCoord2f(0.0f, 1.0f);
            glVertex3f(0.0f, bottom_offset + (float)image_h, 0.0f);
        glEnd();
    }
    else {
        glBegin(GL_QUADS);
            glTexCoord2f(1.0f, 0.0f);
            glVertex3f(0.0f, bottom_offset, 0.0f);

            glTexCoord2f(1.0f, 1.0f);
            glVertex3f((float)image_w, bottom_offset, 0.0f);

            glTexCoord2f(0.0f, 1.0f);
            glVertex3f((float)image_w, bottom_offset + (float)image_h, 0.0f);

            glTexCoord2f(0.0f, 0.0f);
            glVertex3f(0.0f, bottom_offset + (float)image_h, 0.0f);
        glEnd();
    }

    glDisable(GL_TEXTURE_2D);

    glColor3f(1.0f, 0.0f, 0.0f);
    glLineWidth(2.0f);

    // Get the width and height for an individual image tile
    // on screen
    m_RectWidth = ((float)tiled_w)/(float)(m_PartitionsX+1);
    m_RectHeight = ((float)tiled_h)/(float)(m_PartitionsY+1);

    // Concatenate the name of an image with the longest (highest)
    // numerical value (last image) so that we can pick a font
    // size that lets it fit in the tiled area.
    std::string img_name;
    if (m_NumberingFormat == IImageGrabber::eCartesionFormat) {
        img_name = m_BaseImageName + NStr::IntToString(m_PartitionsY+1) + 
            "_" +
            NStr::IntToString(m_PartitionsX+1);
    }
    else {
        img_name = m_BaseImageName +  NStr::IntToString((m_PartitionsY+1)*
            (m_PartitionsX+1));                                 
    }

    CVect2<float> pos(0.0f, 0.0f);

    // pick a font for the file name based on sub-image display area,
    // and find the offset from the base of the sub-rectangle.  Do this
    // by iterating over font sizes for the chosen font starting at 
    // the smallest reasonable size (8pt) to a large reasonable stopping
    // point (36) or when the length exceeds 85% of the available space.
    CGlTextureFont name_font;
    name_font.SetFontFace(CGlTextureFont::eFontFace_TimesRoman);

    name_font.SetFontSize(8);
    float font_width = (float)name_font.TextWidth(img_name.c_str());
    float font_height;
    size_t fsize;

    if (font_width < (m_RectWidth*0.75f)) {
        for (fsize=10; fsize<36; fsize+=2) {
            name_font.SetFontSize(fsize);
            font_width = (float)name_font.TextWidth(img_name.c_str());

            if (font_width > (m_RectWidth*0.75f)) {             
                name_font.SetFontSize(fsize-2);
                break;
            }           
        }
    }

    // Get the position, relative to an image tile, of the image name text.
    font_width = (float)name_font.TextWidth(img_name.c_str());
    font_height = (float)name_font.TextHeight();
    pos.X() = m_RectWidth/2.0f - font_width/2.0f;
    pos.Y() = ((float)h) - m_RectHeight/2.0f - font_height/2.0f;


    //
    // pick a font for the "Saved" text the same way as for the image name text.
    CGlTextureFont saved_font;
    saved_font.SetFontFace(CGlTextureFont::eFontFace_TimesRoman);

    saved_font.SetFontSize(8);
    font_width = (float)name_font.TextWidth("Saved");       

    if (font_width < m_RectWidth*0.75f) {
        for (fsize=10; fsize<36; fsize+=2) {
            saved_font.SetFontSize(fsize);
            font_width = (float)saved_font.TextWidth("Saved");

            if (font_width > m_RectWidth*0.75f) {
                saved_font.SetFontSize(fsize-2);
                break;
            }           
        }
    }

    // Get the position, relative to an image tile, of the 'saved' text.
    float saved_font_xpos = m_RectWidth/2.0f - ((float)saved_font.TextWidth("Saved"))/2.0f;
    float saved_font_xdelta = saved_font_xpos - pos.X();     

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    //
    // Draw the image name, 'saved' text and image tile boxes from upper
    // left to lower right of image.   
    for (int y=0; y<m_PartitionsY+1; ++y) {
        for (int x=0; x<m_PartitionsX+1; ++x) {
            std::string img_name = m_BaseImageName;

            // Get the image name based on the numbering scheme.
            if (m_NumberingFormat == IImageGrabber::eCartesionFormat) {
                if (m_PartitionsY > 0) {
                    // the y index will count from 0 starting at the top.
                    img_name += NStr::IntToString(y + 1);
                    if (m_PartitionsX > 0)
                        img_name += "_";
                }
                if (m_PartitionsX > 0)
                    img_name += NStr::IntToString(x+1);
            }
            else {
                img_name += NStr::IntToString(x + y*(m_PartitionsX+1) + 1);
            }

            // text class currently queries the current GL_CURRENT_RASTER_COLOR which
            // is the color set as the result of lighting calculations when glRasterPos
            // is called.
            if (font_width < (int)m_RectWidth && font_height < (int)m_RectHeight) {
                glColor4f(0.0f, 0.0f, 1.0f, 0.5f);                            
                name_font.TextOut(pos.X(), pos.Y(), img_name.c_str());
            }

            // Draw red lines around current sub-image (this causes the image
            // to be tiled by red squares representing the images to be rendered).
            glColor4f(1.0f, 0.0f, 0.0f, 1.0f);                      
            CVect2<float> rect_pos(x*m_RectWidth, h-((y+1)*m_RectHeight) );
            m_TileOrigins.push_back(CTileOrigin(rect_pos, CVect2<int>(x,y)));
            glBegin(GL_LINE_LOOP);
            glVertex2f(rect_pos.X(), rect_pos.Y());
            glVertex2f(rect_pos.X() + m_RectWidth, rect_pos.Y());
            glVertex2f(rect_pos.X() + m_RectWidth, rect_pos.Y() + m_RectHeight);
            glVertex2f(rect_pos.X(), rect_pos.Y() + m_RectHeight);
            glEnd();

            // If the current tile is to be marked as 'saved', decrease the light level
            // and write the word 'saved'.
            if ( m_SavedY > y ||
                (m_SavedX >= x && m_SavedY == y) ) {
                    // set color and use glRasterPos to set GL_CURRENT_RASTER_COLOR...
                    glColor4f(1.0f, 0.1f, 0.1f, 1.0f);                  
                    float posy = pos.Y() - (saved_font.TextHeight() + 3);
                    float posx = pos.X() + saved_font_xdelta;
                    saved_font.TextOut(posx, posy, "Saved");
                    glColor4f(0.0f, 0.0f, 1.0f, 0.5f);

                    glColor4f(0.1f, 0.1f, 0.1f, 0.2f);                                        
                    glBegin(GL_QUADS);
                    glVertex2f(rect_pos.X(), rect_pos.Y());
                    glVertex2f(rect_pos.X() + m_RectWidth, rect_pos.Y());
                    glVertex2f(rect_pos.X() + m_RectWidth, rect_pos.Y() + m_RectHeight);
                    glVertex2f(rect_pos.X(), rect_pos.Y() + m_RectHeight);
                    glEnd();

                    glColor4f(0.0f, 0.0f, 1.0f, 0.5f);
            }

            pos.X() += m_RectWidth;
        }
        
        // Increment the position for the (next) image name text.
        pos.X() = m_RectWidth/2 - ((float)font_width)/2.0f;
        pos.Y() -= m_RectHeight;
    }

    glDisable(GL_BLEND);

    if (m_ReferenceSubImage != NULL) {
        float rw = (float)m_ReferenceSubImage->GetImage()->GetWidth();
        float rh = (float)m_ReferenceSubImage->GetImage()->GetHeight();

        float rwidth, rheight;
        float ref_image_aspect = rw/rh;

        if (ref_image_aspect > image_aspect) {
            rwidth = (float)w;
            rheight = rh*(w/rw);
        }
        else {
            rheight = (float)h;
            rwidth = rw*(h/rh);
        }

        rheight *= 0.8f;
        rwidth *= 0.8f;

        float originx = (((float)w)-rwidth)/2.0f;
        float originy = (((float)h)-rheight)/2.0f;

        glEnable(GL_TEXTURE_2D);
        m_ReferenceSubImage->MakeCurrent();
        glColor4f(1.0f, 1.0f, 1.0f, 1.0f);

        glBegin(GL_QUADS);
            glTexCoord2f(0.0f, 0.0f);
            glVertex3f(originx, originy, 0);

            glTexCoord2f(1.0f, 0.0f);
            glVertex3f(originx+rwidth, originy, 0);

            glTexCoord2f(1.0f, 1.0f);
            glVertex3f(originx+rwidth, originy+rheight, 0);

            glTexCoord2f(0.0f, 1.0f);
            glVertex3f(originx, originy+rheight, 0);
        glEnd();

        glColor4f(0.0f, 0.0f, 1.0f, 1.0f);
        glLineWidth(2.0f);
        glDisable(GL_TEXTURE_2D);

        glBegin(GL_LINE_LOOP);
            glVertex3f(originx, originy, 0);
            glVertex3f(originx+rwidth, originy, 0);
            glVertex3f(originx+rwidth, originy+rheight, 0);
            glVertex3f(originx, originy+rheight, 0);
        glEnd();

        glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
    }


    glMatrixMode(GL_PROJECTION);
    glPopMatrix();       

    glMatrixMode(GL_MODELVIEW);     
    glPopMatrix();

    glDisable(GL_TEXTURE_2D);
}


END_NCBI_SCOPE
