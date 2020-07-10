#ifndef GUI_UTILS__PDF_HPP
#define GUI_UTILS__PDF_HPP

/*  $Id: pdf.hpp 43836 2019-09-09 20:29:51Z evgeniev $
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
 * Authors:  Peter Meric, Bob Falk
 *
 * File Description:
 *   CPdf - Adobe PDF output
 *
 */

#include <corelib/ncbistd.hpp>
#include <gui/print/vector_printer.hpp>
#include <gui/print/pdf_font_handler.hpp>
#include <gui/print/pdf_object.hpp>
#include <gui/utils/id_generator.hpp>
#include <gui/opengl/glpane.hpp>
#include <gui/opengl/glmodel2d.hpp>

#include <gui/utils/vect2.hpp>
#include <gui/utils/vect4.hpp>

BEGIN_NCBI_SCOPE


class CPdfDictionary;
class CPdfTrailer;
class CPageHandler;
class CPdfObjectWriter;
class CPVecText;
class CPVecLine;
class CPVecPolygon;
class CGlVboNode;
class CRgbaGradColorTable;


class NCBI_GUIPRINT_EXPORT CPdf : public CVectorPrinter
{
public:
    enum EContentType { ePdfLines, ePdfTris, ePdfFlatTris, ePdfQuads, ePdfText, ePdfPoints, eMisc };

public:
    typedef CRef<CPdfObject> TPdfObjectRef;
    typedef CGlTextureFont::EFontFace EFontFace;


    CPdf();
    virtual ~CPdf();

    virtual void SetOptions(const CPrintOptions& options);
    virtual void SetOutputStream(CNcbiOstream* ostream);

    bool GetIsGreyscale() const { return m_IsGreyScale; }
    void SetIsGreyscale(bool b) { m_IsGreyScale = b; }

    virtual void BeginDocument(void);
    virtual void EndDocument(void);

    void WriteObject(TPdfObjectRef& obj);

    virtual TPdfObjectRef BeginContent(EContentType t);
    virtual TPdfObjectRef GetCurrentContent() {return m_CurrentContent;}
    virtual void EndContent();

    virtual TPdfObjectRef BeginAnnot();
    virtual void EndAnnot();

    virtual TPdfObjectRef BeginReference();
    virtual void EndReference();

    CRef<CPdfFontHandler> GetFontHandler() { return m_FontHandler; }

    CRef<CPdfDictionary> PageDictionary() { return m_PageDictionary; }

    /// Write a string of text at the specified position (in screen coords).  Must
    /// have a current contect object (via BeginConent)
    void PrintText(CRef<CPdfFontHandler> font_handler,
                   EFontFace face, 
                   float font_size, 
                   CVect2<float>& p, 
                   const char* txt,
                   const CRgbaColor& c);
    /// Write a string of text at the specified position and orientation (in 
    /// screen coords).  Must have a current contect object (via BeginConent)
    void PrintText(CRef<CPdfFontHandler> font_handler,
                   EFontFace face, 
                   float font_size, 
                   CMatrix3<double>& mat, 
                   const char* txt,
                   const CRgbaColor& c);


    /// Print contents of (2D) model
    void PrintModel(CGlPane& pane, CGlModel2D& model, 
        CRgbaGradColorTable* color_table=NULL);

    void AddTooltip(CGlPane& pane, const string& txt, CVect4<float>& rect);
    void AddJSTooltip(CGlPane& pane, const string& txt, const string& title, CVect4<float>& rect);

    /// Checks the buffer type and prints accodingly tris, points or lines
    void PrintBuffer(CGlVboNode* node, const TVPRect& viewport, 
        CRgbaGradColorTable* color_table=NULL);
   
    /// Prints the points in buffer "node" to the pdf using current graphics 
    /// state, modelview and projection matrices, and viewport
    void PrintPointBuffer(CGlVboNode* node, const TVPRect& vp, 
        CRgbaGradColorTable* color_table=NULL);
    /// Prints the lines in buffer "node" to the pdf using current graphics 
    /// state, modelview and projection matrices, and viewport
    void PrintLineBuffer(CGlVboNode* node, const TVPRect& vp, 
        CRgbaGradColorTable* color_table=NULL);
    /// Prints AddShadedTris and RenderShaderInstance to save an instance of 
    /// shaded triangles to the pdf
    void PrintTriBuffer(CGlVboNode* node, const TVPRect& vp, 
        CRgbaGradColorTable* color_table=NULL);
    /// Quads are only supported as non-shaded (records error if shaded)
    void PrintQuadBuffer(CGlVboNode* node, const TVPRect& vp, 
        CRgbaGradColorTable* color_table=NULL);

    /// Add a triangle buffer to the pdf (but do not display it)
    /// Returns the ID of the triangle shader object in the pdf file
    /// The first version of the function iterates over data to find the range
    string AddShadedTris(CGlVboNode* node, 
                         const TVPRect& vp,
                         CRgbaGradColorTable* color_table,
                         CPdfObject::EBitCount bit_count = CPdfObject::e16Bit);
    string AddShadedTris(CGlVboNode* node,
                         const TVPRect& vp,
                         CPdfObject::EBitCount bit_count,
                         CRgbaGradColorTable* color_table,
                         int range_minx, int range_maxx,
                         int range_miny, int range_maxy);

    /// Display an instance of a set of shaded triangles "shader_id", which
    /// is returned by AddShadedTris.  The location is transformed by
    /// the current OpenGL modelview and projection matrices and viewport.
    void RenderShaderInstance(CGlVboNode* node, const string& shader_id, const TVPRect& vp);
        
    virtual void BeginPage(void);
    virtual void EndPage(void);

protected:
    // Return an id for a pdf graphics state that supports the alpha value of
    // the given node (within a small tolerance) or "" if alpha is 1.0 (alpha is
    // alpha from glstate color or, if not given, alpha from first vertex)
    virtual string x_GetAlphaGraphicsState(CGlVboNode* node);
    virtual string x_GetAlphaGraphicsState(CGlVboNode* node, float alpha);
    virtual string x_GetAlphaGraphicsState(float alpha);

    /// For triangles in which vertices are all the same color
    void x_PrintFlatTriBuffer(CGlVboNode* node, const TVPRect& vp, 
        CRgbaGradColorTable* color_table=NULL);

    /// Return true if node has color buffer and update array with those colors.
    /// If false is returned
    bool x_GetColors(CGlVboNode* node, vector<CRgbaColor>& colors,
        CRgbaGradColorTable* color_table);

private:
    auto_ptr<CPdfObjectWriter> m_ObjectWriter;
    CRef<CIdGenerator> m_ObjIdGenerator;

    // If true, all output is forced to greyscale colors
    bool m_IsGreyScale;

    //
    // document objects
    //
    vector<TPdfObjectRef> m_PrintInEndDoc;

    TPdfObjectRef m_Catalog;
    TPdfObjectRef m_CurrentContent;
    TPdfObjectRef m_CurrentAnnot;
    TPdfObjectRef m_CurrentReference;
    auto_ptr<CPageHandler> m_PageHandler;
    CRef<CPdfTrailer> m_Trailer;
    TPdfObjectRef m_Fonts;
    CRef<CPdfDictionary> m_PageDictionary;
    CRef<CPdfFontHandler> m_FontHandler;

    /// forbidden
    CPdf(const CPdf&);
    CPdf& operator=(const CPdf&);
};



END_NCBI_SCOPE


#endif // GUI_UTILS__PDF_HPP
