#ifndef GUI_OPENGL___GL_CGI_IMAGE__HPP
#define GUI_OPENGL___GL_CGI_IMAGE__HPP

/*  $Id: glcgi_image.hpp 14562 2007-05-18 11:48:21Z dicuccio $
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
 *    CGlCgiImage -- base class for producing images via a CGI using OpenGL
 */

#include <cgi/cgiapp.hpp>
#include <cgi/cgictx.hpp>
#include <util/image/image_io.hpp>


BEGIN_NCBI_SCOPE

class CGlOsContext;

//
// class CGlCgiImageApplication wraps a simple interface for off-screen
// rendering with OpenGL
//
class CGlCgiImageApplication : public CCgiApplication
{
public:

    // default ctor
    CGlCgiImageApplication();
    ~CGlCgiImageApplication();

    // Init() - here we set our image size.  Subclasses can override this and
    // provide their own desired image size
    virtual void Init(void);

    // ProcessRequest() intercepts the request stream, builds our off-screen
    // context, and calls the internal rendering hooks
    virtual int  ProcessRequest(CCgiContext& ctx);

    // render our scene.  This is pure virtual - all derived classes must
    // provide a mechanism for rendering their image.
    virtual void Render(CCgiContext& ctx) = 0;

    // retrieve the aspect ratio of our virtual frame buffer
    float GetAspectRatio(void) const;

protected:

    // dimensions of our virtual frame buffer
    size_t m_Width;
    size_t m_Height;

    // the image format we will emit - default is PNG
    CImageIO::EType m_Format;

    // our off-screen renderer
    CRef<CGlOsContext> m_Context;

    // string holding the name of our error template file.
    // This is set in Init() via calls to the registry - to use the standard
    // settings, the registry file should contain a section that contains:
    //
    // [filesystem]
    // ErrorTemplate = <some-file-name>
    string m_ErrorTemplate;

    // return a valid OpenGL context.  This will create m_Context if it doesn't
    // exist, and make it current for rendering
    CGlOsContext& x_GetContext(void);

    //
    // user hooks
    //


    // x_PreProcess is called from ProcessRequest before any actual work is
    // done.  This is provided as a hook to perform some pre-processing, and
    // provides access to the member with and height variables before the
    // off-screen context is created.
    virtual void x_PreProcess(CCgiContext& ctx);

    // x_PostProcess() is called after processing has completed but before the
    // image is written to the output stream.  This is provided as a point for
    // any finalization tasks.
    virtual void x_PostProcess(CCgiContext& ctx);

    // handle an error through the standard error template
    virtual void x_HandleError(CCgiContext& ctx, const string& msg);
};


END_NCBI_SCOPE

#endif  // GUI_OPENGL___GL_CGI_IMAGE__HPP
