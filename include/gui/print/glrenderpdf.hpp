#ifndef GUI_PRINT___GL_RENDER_MANAGER_PDF__HPP
#define GUI_PRINT___GL_RENDER_MANAGER_PDF__HPP

/*  $Id: glrenderpdf.hpp 43853 2019-09-10 17:42:16Z evgeniev $
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

#include <gui/opengl.h>
#include <gui/gui.hpp>

#include <gui/opengl/render_vector.hpp>

#include <gui/print/pdf.hpp>

/** @addtogroup GUI_PRINT
 *
 * @{
 */

BEGIN_NCBI_SCOPE


//////////////////////////////////////////////////////////////////////////////
/// CGlRenderPdf
///
/// Connect the CGlRender interface to the CPdf object for file output. 
/// PDF has some differences from OpenGL on how it handles polygons.  (Aside
/// from the fact that it only handles 2D drawing).  In particular, there are
/// two distinct kinds of polygons for PDF files, Gouraud shaded triangles, and
/// flat (uniformally) shaded n-sided polygons.  
///
/// The flat polygons also cannot be tiled. If two adjacent 'flat' polygons 
/// share vertices, like two triangles that form a quad, there will generally 
/// remain a visible crack between them. This class uses these definitions, along
/// with state options set by the user, to determine what kind of geometry to 
/// generate.  
/// 
/// Currently there are some cases, particularly more complex polygons
/// created by tiling together triangles as you would normally do in OpenGL that
/// could be formed into an n-sided pdf polygon but are instead saved in the PDF
/// file as a set of gouraud shaded triangles.  We could, in the future, do the
/// geomtric processing to find all the unshared edges and use those to try and 
/// create the polygons outer edge.
///
///
class NCBI_GUIPRINT_EXPORT CGlRenderPdf : public CObject, public CRenderVector
{
public:
    CGlRenderPdf() {}
    virtual ~CGlRenderPdf() {}

    /// m_Pdf does the actually writing to the file
    void SetPdf(CRef<CPdf>& pdf) { m_Pdf = pdf;}
    CRef<CPdf> GetPdf() { return m_Pdf; }

    virtual bool IsSimplified() const;

    virtual bool IsPrinterFriendly() const  { return true; };

    virtual void SetIsGreyscale(bool b);

    virtual ERenderTarget GetApi() { return eRenderPDF; }

    virtual void Hint(GLenum, GLenum) {}

    /// Specialized for models - which are just collections of buffers
    virtual void Render(CGlPane& pane, CGlModel2D* model);
    /// Add a vertex and decompose quads into tris if needed
    virtual void Vertex3f(GLfloat x, GLfloat y, GLfloat z);

    /// Set OpenGL state needed for writing text (call before DrawText)
    virtual void BeginText(const CGlTextureFont* font, 
                           const CRgbaColor& color);
    /// Same as above, but get color from m_RenderNode.m_State
    virtual void BeginText(const CGlTextureFont* font);
    /// Pops matrices and attributes after writing text
    virtual void EndText();
    /// Write text at specified model coords
    virtual void WriteText(TModelUnit x, TModelUnit y, 
                           const char* text,
                           TModelUnit rotate_degrees = 0.0);

    /// Write text at specified model coords inside box defined by
    /// (x,y,width,height) with specified alignment, truncation and rotation
    virtual void WriteText(TModelUnit x, TModelUnit y, 
                           TModelUnit width, TModelUnit height,
                           const char* text,
                           CGlTextureFont::TAlign align = CGlTextureFont::eAlign_Center,
                           CGlTextureFont::ETruncate trunc = CGlTextureFont::eTruncate_Ellipsis,
                           TModelUnit rotate_degrees = 0.0f);
    
    virtual void Viewport(GLint x, GLint y, GLsizei width, GLsizei height);
    
    /// PDF only - get size of pdf 'window' (pagesize)
    void GetScreenSize(int& width, int& height) const;

protected:
    /// Send the current buffer to the pdf library for rendering.
    virtual void x_RenderBuffer(CGlVboNode* node);

    CRef<CPdf> m_Pdf;
};


END_NCBI_SCOPE

/* @} */

#endif  // GUI_PRINT___GL_RENDER_MANAGER_PDF__HPP
