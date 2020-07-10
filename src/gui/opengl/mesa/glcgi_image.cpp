/*  $Id: glcgi_image.cpp 22039 2010-09-08 14:14:26Z kuznets $
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
 * Author:  Denis Vakatov
 *
 * File Description:
 *    CGlCgiImage -- base class for producing images via a CGI using OpenGL
 */

#include <ncbi_pch.hpp>
#include <gui/opengl/mesa/glcgi_image.hpp>
#include <gui/opengl/mesa/gloscontext.hpp>
#include <html/page.hpp>

#include <util/image/image.hpp>
#include <util/image/image_util.hpp>
#include <util/image/image_io.hpp>


BEGIN_NCBI_SCOPE

//
// content-type headers for images
//
struct SContentType
{
    CImageIO::EType m_Type;
    const char* m_Encoding;
};

static const SContentType kContentTypes[] = {
    { CImageIO::eBmp,  "image/bmp" },
    { CImageIO::eGif,  "image/gif" },
    { CImageIO::eJpeg, "image/jpeg" },
    { CImageIO::ePng,  "image/png" },
    { CImageIO::eSgi,  "image/sgi" },
    { CImageIO::eTiff, "image/tiff" },
    { CImageIO::eXpm,  "image/xmp" },

    // must be last
    { CImageIO::eUnknown, NULL }
};


//
// default error template
// this can be replaced as needed
//


//
// default ctor
//
CGlCgiImageApplication::CGlCgiImageApplication()
    : m_Width (800),
      m_Height(600),
      m_Format(CImageIO::ePng)
{
}


CGlCgiImageApplication::~CGlCgiImageApplication()
{
}


//
// get our aspect ratio
//
float CGlCgiImageApplication::GetAspectRatio(void) const
{
    if (m_Context) {
        size_t width  = m_Context->GetBuffer().GetWidth();
        size_t height = m_Context->GetBuffer().GetHeight();
        return (float)width / (float)height;
    }
    return 1.0f;
}


//
// Init()
// Initialize our renderer to a default size (800x600 pixels)
void CGlCgiImageApplication::Init()
{
    m_ErrorTemplate = GetConfig().Get("filesystem", "ErrorTemplate");
}


CGlOsContext& CGlCgiImageApplication::x_GetContext(void)
{
    if ( !m_Context ) {
        // create an off-screen renderer that handles a virtual frame buffer
        m_Context.Reset(new CGlOsContext(m_Width, m_Height));
    }
    return *m_Context;
}



//
// ProcessRequest()
// Here, we do a minimal amount of set-up and pass off to our rendering hook.
// When this hook completes, we write out the resulting image to the response.
//
int CGlCgiImageApplication::ProcessRequest(CCgiContext& ctx)
{
    CStopWatch sw;
    sw.Start();

    double render_time = 0;
    double image_flip_time = 0;
    try {
        // user hook: pre-rpocess
        x_PreProcess(ctx);

        // create an off-screen renderer that handles a virtual frame buffer
        if ( !x_GetContext().MakeCurrent() ) {
            LOG_POST(Error << "CGlCgiImageApplication::ProcessRequest(): "
                     "Failed to make off-screen renderer current");
            return 1;
        }


        // call the rendering hook
        Render(ctx);

        // call glFinish() - VERY IMPORTANT FOR OFF_SCREEN RENDERING
        // this forces any buffered OpenGL requests to complete now.
        glFinish();

        render_time = sw.Elapsed();

        // final image preparation
        // the image is upside-down because frame buffers are, in general,
        // upside-down.  Also, we have an alpha channel we need to flatten.
        m_Context->SetBuffer().SetDepth(3);
        CImageUtil::FlipY(m_Context->SetBuffer());

        image_flip_time = sw.Elapsed();

        // user hook: post-process
        x_PostProcess(ctx);

    }
    catch (CException& e) {
        x_HandleError(ctx, e.GetMsg());
        return 1;
    }
    catch (std::exception& e) {
        x_HandleError(ctx, e.what());
        return 1;
    }

    //
    // and stream back the image
    // this requires that we first write the image to a temporary file and then
    // echo the contents of this image back
    //
    // don't forget to set our content-type
    string encoding("image/unknown");
    for (const SContentType* type = kContentTypes;  type->m_Encoding;  ++type) {
        if (type->m_Type == m_Format) {
            encoding = type->m_Encoding;
            break;
        }
    }
    CCgiResponse& response = ctx.GetResponse();
    response.SetContentType(encoding);

    //
    // write the standard CGI headers
    // if this line is commented out, the image will be dumped to stdout anc
    // can be redirected to a file for local access
    //
    response.WriteHeader();

    //
    // write the image
    // we wrap the management of the temporary file in a class in case one of
    // our operations throws; this way, the stack unwinding for the exception
    // will insure that our temporary file gets cleaned up
    //

    CImageIO::WriteImage(m_Context->GetBuffer(), response.out(), m_Format);

    double image_encode_time = sw.Elapsed();
    LOG_POST(Info << "CGlCgiImage::ProcessRequest():\n"
             << "  render time  = " << render_time << "\n"
             << "  process time = " << image_flip_time - render_time << "\n"
             << "  encode time  = " << image_encode_time - image_flip_time << "\n");

    return 0;
}


void CGlCgiImageApplication::x_HandleError(CCgiContext& ctx,
                                           const string& msg)
{
    CCgiResponse& response = ctx.GetResponse();
    response.SetContentType("text/html");
    response.WriteHeader();

    if (m_ErrorTemplate.empty()) {
        CHTMLPage page("CGI Error", m_ErrorTemplate);
        page.AppendChild(new CHTMLPlainText(msg));
        page.Print(response.out());
    } else {
        CHTMLPage page("", m_ErrorTemplate);
        page.AddTagMap("message", new CHTMLPlainText(msg));
        page.Print(response.out());
    }
}


void CGlCgiImageApplication::x_PreProcess(CCgiContext& ctx)
{
    // set the width and height from the 'width=' and 'height=' keys
    {{
         TCgiEntries::const_iterator width_iter =
             ctx.GetRequest().GetEntries().find("width");
         if (width_iter != ctx.GetRequest().GetEntries().end()) {
             m_Width = NStr::StringToInt(width_iter->second);
         }

         TCgiEntries::const_iterator height_iter =
             ctx.GetRequest().GetEntries().find("height");
         if (height_iter != ctx.GetRequest().GetEntries().end()) {
             m_Height = NStr::StringToInt(height_iter->second);
         }
     }}

    // set the image format using the 'fmt=' key
    TCgiEntries::const_iterator fmt_iter =
        ctx.GetRequest().GetEntries().find("fmt");
    if (fmt_iter != ctx.GetRequest().GetEntries().end()) {
        string fmt(fmt_iter->second);
        fmt = "." + fmt;
        m_Format = CImageIO::GetTypeFromFileName(fmt);
    }
}


void CGlCgiImageApplication::x_PostProcess(CCgiContext& ctx)
{
}


END_NCBI_SCOPE
