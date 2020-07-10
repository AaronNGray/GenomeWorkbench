/*  $Id: image_grabber.cpp 44930 2020-04-21 17:07:30Z evgeniev $
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

#include <corelib/ncbistd.hpp>

#include <gui/widgets/wx/image_grabber.hpp>
#include <gui/opengl/glframebuffer.hpp>
#include <gui/opengl/glresmgr.hpp>

#include <corelib/ncbifile.hpp>

#include <gui/widgets/wx/glcanvas.hpp>

#include <gui/opengl/gltexture.hpp>
#include <gui/opengl/glutils.hpp>
#include <gui/opengl/gltexturefont.hpp>

#include <util/image/image.hpp>
#include <util/image/image_io.hpp>
#include <gui/utils/vect2.hpp>

#include <gui/widgets/wx/pdf_grabber.hpp>
#include <gui/widgets/wx/svg_grabber.hpp>


BEGIN_NCBI_SCOPE

enum EGlDiagMode {
    eUndefined,
    eIgnore,
    eLogPost,
    eThrow,
    eAbort
};


IImageGrabber::IImageGrabber() 
: m_GuidesEnabled(false)
, m_GuideWidthX(0)
, m_GuideWidthY(0)
, m_TileAspectRatio(-1.0f)
, m_ImageAspectRatio(-1.0f)
, m_DisableGouraudShaded(false)
{
    m_SavedRender = CGlResMgr::Instance().GetCurrentRenderer();
}

IImageGrabber::~IImageGrabber()
{
    if (m_SavedRender)
        CGlResMgr::Instance().SetCurrentRenderer(m_SavedRender);
}

void IImageGrabber::SetOutputInfo(const std::string& dir,
                                  const std::string& base_name,
                                  const std::string& img_format)
{
    m_Directory = dir;
    m_BaseImageName = base_name;
    m_ImageFormat = img_format;
}

IImageGrabber::eCaptureResult 
IImageGrabber::GrabImages(int tex_size, IImageGrabberProgress* progress)
{
    if (progress != NULL)
        progress->SetGLContext();

    if (!glewIsSupported("GL_EXT_framebuffer_object")) {
        _TRACE("Opengl:  Feature not available");
        return eOpenGLError;
    }  

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    CGLFrameBuffer tile_buffer(tex_size);
    tile_buffer.CreateFrameBuffer();   
    if (!tile_buffer.IsValid())
        return eOpenGLError;  

    CImage img(tex_size, tex_size, 4);
    CGlUtils::CheckGlError();

    if (!m_BaseImageName.empty()) {        
        CImageIO::EType img_type = CImageIO::GetTypeFromFileName( "Whatever."+ 
                                                                  m_ImageFormat );
        
        tile_buffer.MakeCurrent(true);      
        if (tile_buffer.CheckFBOError())
            return eOpenGLError;

        GLint alignment;
        glGetIntegerv(GL_PACK_ALIGNMENT, &alignment);
        glPixelStorei(GL_PACK_ALIGNMENT, 1);   

        x_BeginCaptures(tex_size);

        for (int y=0; y<m_ImagePartitions.Y()+1; ++y) {
            for (int x=0; x<m_ImagePartitions.X()+1; ++x) {                
                glDisable(GL_TEXTURE_2D);
                
                // Call subclass to generate subimage
                CVect2<size_t> capture_size;
                x_CaptureImage(m_ImagePartitions, 
                              CVect2<int>(x,y), 
                              capture_size);

                std::string img_name = x_GetImageName(x, y);   

                if (m_GuidesEnabled) {
                    x_RenderGuides(capture_size,
                                   img_name,
                                   x_GetImageName(x, y+1),
                                   x_GetImageName(x+1, y),
                                   x_GetImageName(x, y-1),
                                   x_GetImageName(x-1, y));
                }

                img_name = CDir::ConcatPath(m_Directory, img_name);

                // Make sure image size is correct.  Probably all tiles will be
                // the  same size, but this code will work either way.
                if (img.GetWidth() != capture_size.X() ||
                    img.GetHeight() != capture_size.Y() ) {
                    img.Init(capture_size.X(), capture_size.Y(), 4);
                }
                                        
                glReadPixels(0, 0, (GLsizei)capture_size.X(), (GLsizei)capture_size.Y(), 
                            GL_RGBA, GL_UNSIGNED_BYTE, img.SetData());
                img.Flip();

                //Write image to file
                bool success = CImageIO::WriteImage(img, img_name, img_type);                

                // Call progress object to indicate that that the current image
                // has been saved
                if (!success) {
                    x_EndCaptures();
                    return eFileError; 
                }
                else if (progress != NULL) {
                    // Bind standard (window) framebuffer
                    tile_buffer.MakeCurrent(false);

                    // Update progress info
                    progress->ImageSaved(x, y);

                    // Re-bind texture buffer we are using for image output
                    tile_buffer.MakeCurrent(true);                  
                }
            }          
        }

        // Restore standard (window) framebuffer and pixel store
        glPixelStorei(GL_PACK_ALIGNMENT, alignment);
        tile_buffer.MakeCurrent(false);
    }   

    x_EndCaptures();

    return eSuccess;
}

IImageGrabber::eCaptureResult 
IImageGrabber::GrabImage(int tex_size, 
                         CVect2<int> img_idx,
                         IImageGrabberProgress* p)
{
    if (p != NULL)
        p->SetGLContext();

    if (!glewIsSupported("GL_EXT_framebuffer_object")) {
        _TRACE("Opengl:  Feature not available");
        return eOpenGLError;
    }  

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    CGLFrameBuffer tile_buffer(tex_size);
    tile_buffer.CreateFrameBuffer();   
    if (!tile_buffer.IsValid())
        return eOpenGLError;  

    CRef<CImage> img(new CImage(tex_size, tex_size, 3));

    tile_buffer.MakeCurrent(true);

    GLint alignment;
    glGetIntegerv(GL_PACK_ALIGNMENT, &alignment);
    glPixelStorei(GL_PACK_ALIGNMENT, 1);   

    x_BeginCaptures(tex_size);
              
    glDisable(GL_TEXTURE_2D);

    // Call subclass to generate subimage
    CVect2<size_t> capture_size;
    x_CaptureImage(m_ImagePartitions, img_idx, capture_size);

    string img_name = x_GetImageName(img_idx.X(), img_idx.Y());   

    if (m_GuidesEnabled) {
        x_RenderGuides(capture_size,
            img_name,
            x_GetImageName(img_idx.X(), img_idx.Y()+1),
            x_GetImageName(img_idx.X()+1, img_idx.Y()),
            x_GetImageName(img_idx.X(), img_idx.Y()-1),
            x_GetImageName(img_idx.X()-1, img_idx.Y()));
    }

    // Make sure image size is correct.  Probably all tiles will be
    // the  same size, but this code will work either way.
    if (img->GetWidth() != capture_size.X() ||
        img->GetHeight() != capture_size.Y() ) {
            img->Init(capture_size.X(), capture_size.Y(), 3);
    }

    glReadPixels(0, 0, (GLsizei)capture_size.X(), (GLsizei)capture_size.Y(), 
                GL_RGB, GL_UNSIGNED_BYTE, img->SetData());

    // Call progress object to indicate that that the current image
    // has been saved
    if (p != NULL) {
        // Bind standard (window) framebuffer
        glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);
        glDrawBuffer(GL_BACK);
        glReadBuffer(GL_BACK); 

        // Display preview image in widget
        p->SetPreviewSubImage(img);                  
    }

    // Restore standard (window) framebuffer and pixel store
    glPixelStorei(GL_PACK_ALIGNMENT, alignment);

    x_EndCaptures();

    return eSuccess;
}


IImageGrabber::eCaptureResult 
IImageGrabber::PreviewImages(int tex_size, 
                             int ref_img_width, int ref_img_height,
                             IImageGrabberProgress* p)
{
    if (p == NULL) 
        return eError;

    p->SetGLContext();

    if (!glewIsSupported("GL_EXT_framebuffer_object")) {
        _TRACE("Opengl:  Feature not available");
        return eOpenGLError;
    }  

    // We have the size of a display area for the tiled image on the
    // preview widget. Use this size to find the next larger (square)
    // texture size
    int size = std::max(ref_img_width, 
                        ref_img_height);
    int ref_image_size = 64;
    while (ref_image_size < size)
        ref_image_size *= 2;

    CGLFrameBuffer result_buffer(ref_image_size);
    result_buffer.CreateFrameBuffer();

    CGlTexture *result_texture = new CGlTexture(result_buffer.GetTexture(), ref_image_size, ref_image_size);
    result_buffer.ReleaseTexture();

    result_buffer.MakeCurrent(true);    
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    int target_tile_width, target_tile_height;
    vector<IImageGrabberProgress::CTileOrigin> tile_positions;
    tile_positions = p->GetTileLocations(target_tile_width, target_tile_height);

    // Compute scale factors between the display area size and the 
    // texture size.  These are used for scaling the tiles whose
    // sizes we get from the preview widget.
    float tiled_width = (m_ImagePartitions.X()+1)*target_tile_width;
    float tiled_height = (m_ImagePartitions.Y()+1)*target_tile_height;
    float tile_scale_x = ((float)ref_image_size)/(float)tiled_width;
    float tile_scale_y = ((float)ref_image_size)/(float)tiled_height;

    CGLFrameBuffer tile_buffer(tex_size);
    tile_buffer.SetTextureFiltering(GL_LINEAR_MIPMAP_NEAREST, GL_LINEAR);
    tile_buffer.SetTextureWrap(GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE);
    tile_buffer.CreateFrameBuffer();
    tile_buffer.MakeCurrent(false);
    if (!tile_buffer.IsValid())
        return eOpenGLError;  

    // This renderes to the screen with the updated image, so make sure
    // default frame buffer is active.
    p->SetReferenceImage(result_texture);

    x_BeginCaptures(tex_size);

    size_t tile_idx = 0;

    for (int y=0; y<m_ImagePartitions.Y()+1; ++y) {
        for (int x=0; x<m_ImagePartitions.X()+1; ++x) {                
            glDisable(GL_TEXTURE_2D);

            // Re-bind texture buffer we are using for image output
            tile_buffer.MakeCurrent(true);
            if (tile_buffer.CheckFBOError())
                return eOpenGLError;

            // Call subclass to generate subimage
            CVect2<size_t> capture_size;
            x_CaptureImage(m_ImagePartitions, 
                CVect2<int>(x,y), 
                capture_size);
            tile_buffer.GenerateMipMaps();

            if (m_GuidesEnabled) {
                x_RenderGuides(capture_size,
                    x_GetImageName(x, y),
                    x_GetImageName(x, y+1),
                    x_GetImageName(x+1, y),
                    x_GetImageName(x, y-1),
                    x_GetImageName(x-1, y));
            }

            IImageGrabberProgress::CTileOrigin tile_pos = tile_positions[tile_idx];
            ++tile_idx;

            // Draw into the buffer using the positions for drawing the tiles, but
            // scale those to the image size (the images we draw will tile the entire
            // texture map, (ref_image_size, ref_image_size) with appropriate scaling.
            int originx = tile_pos.m_TileIndex.X()*target_tile_width*tile_scale_x;           
            int originy = (m_ImagePartitions.Y() - tile_pos.m_TileIndex.Y()) * 
                target_tile_height*tile_scale_y;

            result_buffer.MakeCurrent(true);

            // Viewport for just region of the texture for this tile. Enlarge by 1 to
            // avoid dropped pixels.
            glViewport(originx, originy,
                (int)((float)target_tile_width)*tile_scale_x + 1, 
                (int)((float)target_tile_height)*tile_scale_y + 1);
            glColor4f(1.0f, 1.0f, 1.0f, 1.0f);

            glMatrixMode(GL_MODELVIEW);
            glPushMatrix();
            glLoadIdentity();

            glMatrixMode(GL_PROJECTION);
            glPushMatrix();
            glLoadIdentity();
            gluOrtho2D(0.0, 1.0, 0.0, 1.0);

            glEnable(GL_TEXTURE_2D);
            glBindTexture(GL_TEXTURE_2D, tile_buffer.GetTexture()); 

            float capture_ratiox = ((float)capture_size.X())/(float)tex_size;
            float capture_ratioy = ((float)capture_size.Y())/(float)tex_size;

            glBegin(GL_QUADS);
                glTexCoord2f(0.0f, 0.0f);
                glVertex3f(0.0f, 0.0f, 0.0f);

                glTexCoord2f(capture_ratiox, 0.0f);
                glVertex3f(1.0f, 0.0f, 0.0f);

                glTexCoord2f(capture_ratiox, capture_ratioy);
                glVertex3f(1.0f, 1.0f, 0.0f);

                glTexCoord2f(0.0f, capture_ratioy);
                glVertex3f(0.0f, 1.0f, 0.0f);
            glEnd();                                       

            // Refresh preview widget incrementally (as edget tile is rendered).
            // Bind standard (window) framebuffer before refreshing, since the
            // refresh draw requires OpenGL drawing to the window.
            glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);
            glDrawBuffer(GL_BACK);
            glReadBuffer(GL_BACK);
            p->ImageSaved(-1, -1);

            glPopMatrix();

            glMatrixMode(GL_PROJECTION);
            glPopMatrix();
        }

    }

    // Restore standard (window) framebuffer and pixel store
    glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);
    glDrawBuffer(GL_BACK);
    glReadBuffer(GL_BACK);

    x_EndCaptures();

    return eSuccess;
}



std::string IImageGrabber::x_GetImageName(int x, int y)
{
    // If tile index is out of bounds return blank
    if (y < 0 ||
        x < 0 ||
        y > m_ImagePartitions.Y() ||
        x > m_ImagePartitions.X()) {
        return "";
    }
      
    std::string img_name = m_BaseImageName;

    if (m_NumberingFormat == IImageGrabber::eCartesionFormat) {
        // Add sequential one-based tile (x,y) coords to image name
        if (m_ImagePartitions.Y() > 0) {
            // the y index will count from 0 starting at the top.
            img_name += NStr::IntToString(y + 1);
            if (m_ImagePartitions.X() > 0)
                img_name += "_";
        }
        if (m_ImagePartitions.X() > 0)
            img_name += NStr::IntToString(x+1);
    }
    else {
        // Add sequential (one-based) counter to name
        img_name += NStr::IntToString(x + y*(m_ImagePartitions.X()+1) + 1);                                
    }

    img_name += "." + m_ImageFormat;

    return img_name;
}


void IImageGrabber::x_RenderGuides(CVect2<size_t> capture_size,
                                   const std::string& image_name,
                                   std::string image_bottom,
                                   std::string image_right,
                                   std::string image_top,
                                   std::string image_left)
{
    glViewport(0, 0, (GLsizei)capture_size.X(), (GLsizei)capture_size.Y());
    glMatrixMode(GL_PROJECTION);     
    glPushMatrix();       
    glLoadIdentity();
    glOrtho(0.0, (double)capture_size.X(), 
            0.0, (double)capture_size.Y(),
            -1.0, 1.0);

    glMatrixMode(GL_MODELVIEW);       
    glPushMatrix();
    glLoadIdentity();
       
    glColor3f(0.7f, 0.7f, 0.7f);
    glLineWidth(1.0f);
    glEnable(GL_LINE_STIPPLE);
    glLineStipple(8, (short)0x0307);

    // Draw borders around image, with gap of 2 pixels between border and
    // actual image.  Subtracting 1 extra rom top and right is due to 0-based
    // counting.
    glBegin(GL_LINE_LOOP);
        glVertex2f(m_GuideWidthX-5, 
                   m_GuideWidthY-5);
        glVertex2f(capture_size.X()-m_GuideWidthX + 6.0,
                   m_GuideWidthY-2);
        glVertex2f(capture_size.X()-m_GuideWidthX + 6.0, 
                   capture_size.Y()-m_GuideWidthY + 6.0);
        glVertex2f(m_GuideWidthX-2, 
                   capture_size.Y()-m_GuideWidthY + 6.0);
    glEnd();

    glDisable(GL_LINE_STIPPLE);

    /*
     * Write names of adjacent images (if any) inside the guide margins
     */

    // pick a font for the file names of the adjacent images. Default to 10,
    // and work down from there if needed.  
    CGlTextureFont img_name_font;
    img_name_font.SetFontFace(CGlTextureFont::eFontFace_TimesRoman);

    unsigned int font_height = (unsigned int)std::min(10, std::min(m_GuideWidthX, m_GuideWidthY)-8);
    font_height = std::max(font_height, (unsigned int)8);
    img_name_font.SetFontSize(font_height);

    glColor3f(1.0f, 0.0f, 0.0f);

    img_name_font.BeginText();
    // Draw the image name in the upper right-hand corner.
    if (image_name != "") {
        int text_width = (int)img_name_font.TextWidth(image_name.c_str());
        img_name_font.WriteText(((int)capture_size.X()) - (text_width + m_GuideWidthX),
                                ((int)capture_size.Y()) - (font_height+2), 
                                image_name.c_str());
    }

    glColor3f(0.25f, 0.25f, 0.25f);    

    // Draw the name of the adjacent images (if any) in the appropriate margin
    // on each side of the image.
    if (image_bottom != "") {
        //image_bottom += "v";
        int text_width = (int)img_name_font.TextWidth(image_bottom.c_str());
        img_name_font.WriteText(((int)capture_size.X())/2-text_width/2, 
                                4, 
                                image_bottom.c_str());
    }

    if (image_top != "") {
        //image_top += "^";
        int text_width = (int)img_name_font.TextWidth(image_top.c_str());
        img_name_font.WriteText(((int)capture_size.X())/2-text_width/2,
                               ((int)capture_size.Y())-(font_height+2), 
                               image_top.c_str());
    }

    if (image_right != "") {
        //image_right += ">";
        int text_width = (int)img_name_font.TextWidth(image_right.c_str());
        img_name_font.WriteText(((int)capture_size.X()) - 4, 
                                ((int)capture_size.Y())/2-text_width/2,
                                image_right.c_str(), 90.0f);
    }

    if (image_left != "") {
        //image_left += "<";
        int text_width = (int)img_name_font.TextWidth(image_left.c_str());
        img_name_font.WriteText(font_height + 2, 
                                ((int)capture_size.Y())/2-text_width/2, 
                                image_left.c_str(), 90.0f);
    }
    img_name_font.EndText();
             
    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();
      
    glMatrixMode(GL_PROJECTION);     
    glPopMatrix();
}

IImageGrabber* CImageGrabberFactory::CreateImageGrabber(CImageGrabberFactory::EOutputFormat format, IVectorGraphicsRenderer& pane)
{
    switch(format) {
        case CPrintOptions::ePdf:
            return new CPdfGrabber(pane);
        case CPrintOptions::eSvg:
            return new CSVGGrabber(pane);
        default:
            NCBI_THROW(CException, eUnknown, "Unsupported image format");
    }
}

END_NCBI_SCOPE

