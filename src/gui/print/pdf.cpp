/*  $Id: pdf.cpp 44300 2019-11-25 19:10:32Z evgeniev $
 * ===========================================================================
 *
 *                            PUBLIC DOMAIN NOTICE
 *               National Center for Biotechnology Information
 *
 *  This software / database is a "United States Government Work" under the
 *  terms of the United States Copyright Act.  It was written as part of
 *  the author's official duties as a United States Government employee and
 *  thus cannot be copyrighted.  This software / database is freely available
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
 * Authors:  Peter Meric
 *
 * File Description:
 *   CPdf - Adobe PDF output
 *
 */


#include <ncbi_pch.hpp>
#include <gui/print/pdf.hpp>
#include <gui/print/pdf_object.hpp>
#include "page_handler.hpp"
#include "pdf_object_writer.hpp"
#include <gui/print/pdf_font_handler.hpp>
#include <gui/opengl/glpane.hpp>
#include <gui/opengl/glrendernode.hpp>
#include <gui/opengl/glvbonode.hpp>
#include <gui/opengl/glcolortable.hpp>
#include <gui/opengl/irender.hpp>
#include <gui/opengl/glutils.hpp>
#include <gui/opengl/tri_perimeter.hpp>

#include <corelib/ncbitime.hpp>

#include <cmath>

BEGIN_NCBI_SCOPE


CPdf::CPdf()
    : m_ObjectWriter(new CPdfObjectWriter()),
      m_ObjIdGenerator(new CIdGenerator()),
      m_IsGreyScale(false),
      m_PageDictionary(new CPdfDictionary()),
      m_FontHandler(new CPdfFontHandler(m_ObjIdGenerator))
{
    m_PageHandler.reset(new CPageHandler(*m_PageBuffers,
                                         *m_ObjectWriter,
                                         m_ObjIdGenerator,
                                         m_FontHandler));
}


CPdf::~CPdf()
{
}


void CPdf::SetOptions(const CPrintOptions& options)
{
    CVectorPrinter::SetOptions(options);
    m_PageHandler->SetOptions(m_Options);
}


void CPdf::SetOutputStream(CNcbiOstream* ostream)
{
    CVectorPrinter::SetOutputStream(ostream);
    m_ObjectWriter->SetOutputStream(ostream);
}


void CPdf::BeginDocument()
{
    // version 1.6 (and beyond) provide support for page sizes > 200x200 inches
    *m_Strm << "%PDF-1.7" << pdfeol;
    // output binary characters in a comment to alert programs this is a binary file
    *m_Strm << '%';
    const unsigned int N = 10;
    const char binchar[N] =
    {
        char(0x80), char(0x81), char(0x82), char(0x83), char(0x84),
        char(0x85), char(0x86), char(0x87), char(0x88), char(0x89)
    };
    m_Strm->write(binchar, N);
    *m_Strm << pdfeol;

    // create objects
    TPdfObjectRef info(new CPdfObject(m_ObjIdGenerator->NextId()));
    m_Catalog.Reset(new CPdfObject(m_ObjIdGenerator->NextId()));
    TPdfObjectRef outlines(new CPdfObject(m_ObjIdGenerator->NextId()));

    // populate info
    CPdfObject& _info = *info;
    _info["Author"] = new CPdfString("National Center for Biotechnology Information");
    _info["Creator"] = new CPdfString("NCBI Genome Workbench http://www.ncbi.nlm.nih.gov/tools/gbench/");
    _info["Producer"] = new CPdfString("NCBI PDF Generator");
    _info["Title"] = new CPdfString(m_Options.GetTitle());
    m_ObjectWriter->WriteObject(info);

    // populate catalog
    CPdfObject& catalog = *m_Catalog;
    catalog["Type"] = new CPdfName("Catalog");
    catalog["Outlines"] = new CPdfIndirectObj(outlines);
    catalog["Pages"] = new CPdfIndirectObj(m_PageHandler->GetObject());
    catalog["PageLayout"] = new CPdfName(m_Options.GetNumPages() > 1 ? "OneColumn" : "SinglePage");
    catalog["PageMode"] = new CPdfName("UseNone");
    catalog["ViewerPreferences"] = new CPdfElement("<</DisplayDocTitle true>>");
    if (m_Options.GetPrintOutput()) {
        // Add print action to Catalog dictionary
        catalog["OpenAction"] = new CPdfElement("<</S/Named /N/Print>>");
    }
    catalog["PageLabels"] = new CPdfElement("<</Nums[0<</S/D/P(Panel )>>]>>");
    m_ObjectWriter->WriteObject(m_Catalog);

    // populate outlines
    CPdfObject& ol = *outlines;
    ol["Type"] = new CPdfName("Outlines");
    ol["Count"] = new CPdfNumber(int(0));
    m_ObjectWriter->WriteObject(outlines);

    // populate trailer
    m_Trailer.Reset(new CPdfTrailer());
    CPdfTrailer& trailer = *m_Trailer;
    trailer["Info"] = new CPdfIndirectObj(info);
    trailer["Root"] = new CPdfIndirectObj(m_Catalog);

    m_Fonts.Reset(new CPdfObject(m_ObjIdGenerator->NextId()));
    CPdfObject& fonts = *m_Fonts;
    ITERATE (CPdfFontHandler::TObjectList, it, m_FontHandler->GetFontObjects()) {
        CRef<CPdfObject> font(*it);
        const string fontname((*font)["Name"]->GetValue());
        fonts[fontname] = new CPdfIndirectObj(font);
        m_PrintInEndDoc.push_back(font);
    }

    m_PrintInEndDoc.push_back(m_Fonts);


    CRef<CPdfDictionary> resources(new CPdfDictionary());
    CPdfDictionary& res = *resources;
    res["Font"] = new CPdfIndirectObj(m_Fonts);

    (*m_PageDictionary)["Resources"] = resources;
}

void CPdf::WriteObject(TPdfObjectRef& obj)
{
    m_ObjectWriter->WriteObject(obj);
}

CPdf::TPdfObjectRef CPdf::BeginContent(EContentType /*t*/)
{
    m_CurrentContent.Reset(new CPdfObject(m_ObjIdGenerator->NextId()));

    return m_CurrentContent;
}

void CPdf::EndContent()
{
     m_PageHandler->AddContent(m_CurrentContent);
     //m_CurrentContent.Release();
}

CPdf::TPdfObjectRef CPdf::BeginAnnot()
{
    m_CurrentAnnot.Reset(new CPdfObject(m_ObjIdGenerator->NextId()));

    return m_CurrentAnnot;
}

void CPdf::EndAnnot()
{
     m_PageHandler->AddAnnot(m_CurrentAnnot);
}

CPdf::TPdfObjectRef CPdf::BeginReference()
{
    m_CurrentReference.Reset(new CPdfObject(m_ObjIdGenerator->NextId()));

    return m_CurrentReference;
}

void CPdf::EndReference()
{
     m_ObjectWriter->WriteObject(m_CurrentReference);
}

void CPdf::BeginPage()
{
}

void CPdf::EndPage()
{
    if ( !m_CurrentContent ) {
        return;
    }

    //m_PageHandler->AddContent(m_CurrentContent);
    //m_PageCount += m_PageHandler->WritePages(m_PageCount + 1);
    m_PageCount += m_PageHandler->WritePages();
}

void CPdf::PrintText(CRef<CPdfFontHandler> font_handler,
                      EFontFace face, 
                      float font_size, 
                      CVect2<float>& p, 
                      const char* txt,
                      const CRgbaColor& c)
{
    if (!m_CurrentContent.IsNull()) {
        m_CurrentContent->SetFillColor(c);

        string alpha_state = x_GetAlphaGraphicsState(c.GetAlpha());
        if (alpha_state != "") {
            m_CurrentContent->SetGraphicsState(alpha_state);
        }
       
        float uu_inv = 1.0f/m_Options.GetUserUnit();
        font_size *= uu_inv;

        m_CurrentContent->Text(font_handler, face, font_size, p, txt);
    }
    else {
        LOG_POST("Unable to write text - current content object is NULL.");
    }
}

void CPdf::PrintText(CRef<CPdfFontHandler> font_handler,
                      EFontFace face, 
                      float font_size, 
                      CMatrix3<double>& mat, 
                      const char* txt,
                      const CRgbaColor& c)
{
    if (!m_CurrentContent.IsNull()) {
        m_CurrentContent->SetFillColor(c);

        string alpha_state = x_GetAlphaGraphicsState(c.GetAlpha());
        if (alpha_state != "") {
            m_CurrentContent->SetGraphicsState(alpha_state);
        }

        float uu_inv = 1.0f/m_Options.GetUserUnit();
        font_size *= uu_inv;

        m_CurrentContent->Text(font_handler, face, font_size, mat, txt);
    }
    else {
        LOG_POST("Unable to write text - current content object is NULL.");
    }
}

void CPdf::PrintModel(CGlPane& pane, CGlModel2D& model, 
    CRgbaGradColorTable* color_table)
{  
    vector<CGlVboNode*>& geom_nodes = model.GetNodes();

    CGlUtils::CheckGlError();
    for (size_t i=0; i<geom_nodes.size(); ++i) {
        if (geom_nodes[i]->IsVisible() && !geom_nodes[i]->IsSkipped(eRenderPDF) ) {
            if (geom_nodes[i]->GetState().ScaleInvarientSet() &&
                geom_nodes[i]->GetState().GetScaleInvarient()) {              
                    float uu_inv = 1.0f / GetOptions().GetUserUnit();
                    CVect2<double> scale(pane.GetScale()*(double)uu_inv);
                    if (geom_nodes[i]->GetState().GetScaleInvarient()) {
                        geom_nodes[i]->GetState().ScaleFactor(scale);
                    }
            }
            PrintBuffer(geom_nodes[i], pane.GetViewport(), color_table);
        }
    }

    vector<CGlVboNode*>& temp_geom_nodes = model.GetTempNodes();

    for (size_t i=0; i<temp_geom_nodes.size(); ++i) {
        if (temp_geom_nodes[i]->IsVisible() && !temp_geom_nodes[i]->IsSkipped(eRenderPDF) ) {
            if (temp_geom_nodes[i]->GetState().ScaleInvarientSet() &&
                temp_geom_nodes[i]->GetState().GetScaleInvarient()) {
                    float uu_inv = 1.0f / GetOptions().GetUserUnit();         
                    CVect2<double> scale(pane.GetScale()*(double)uu_inv);
                    if (temp_geom_nodes[i]->GetState().GetScaleInvarient()) {
                        temp_geom_nodes[i]->GetState().ScaleFactor(scale);
                    }
            }
            PrintBuffer(temp_geom_nodes[i], pane.GetViewport(), color_table);
        }
    }
}


void CPdf::AddTooltip(CGlPane& pane, const string& txt, CVect4<float>& rect)
{
    CRef<CPdfObject> popup = BeginAnnot();

    // Project the coordinates of the label (model coordinates) onto
    // viewport coordinates which are used for the popup.
    TVPRect vp = pane.GetViewport();

	GLint viewport[4];
    GLdouble model_view_matrix[16];
    GLdouble projection_matrix[16];

    viewport[0] = GLint(vp.Left());
    viewport[1] = GLint(vp.Bottom());
    viewport[2] = GLint(vp.Width());
    viewport[3] = GLint(vp.Height());

    IRender& gl = GetGl();
    gl.GetModelViewMatrix(model_view_matrix);
    gl.GetProjectionMatrix(projection_matrix);

    double llx = (double)rect.X();
    double lly = (double)rect.Y();

    double urx = (double)rect.Z();
    double ury = (double)rect.W();

    double px,py,px2,py2,dummyz;

    gluProject(llx, lly, 0.0, 
        model_view_matrix, projection_matrix, viewport, 
        &px, &py, &dummyz);

    string pos = " [" + NStr::NumericToString(int(px)) + " " +
        NStr::NumericToString(int(py)) + " ";

    gluProject(urx, ury, 0.0, 
        model_view_matrix, projection_matrix, viewport, 
        &px2, &py2, &dummyz);

    pos += NStr::NumericToString(int(px2)) + " " +
        NStr::NumericToString(int(py2)) + "]";
   
    // You still get an annoying little popup
    CPdfDictionary& popup_dict = popup->GetDictionary();    
    popup_dict["Border"]= new CPdfElement(" [0 0 0]"); // colorloss annotation
    //popup_dict["RD"]= new CPdfElement(" [ 2.0 2.0 2.0 2.0 ]");
    popup_dict["Contents"] = new CPdfString(txt);
    popup_dict["Type"] = new CPdfName("Annot");
    popup_dict["Rect"] = new CPdfElement(pos);
    popup_dict["Subtype"] = new CPdfName("Square");
    
    popup_dict["F"] = new CPdfNumber(64); // makes it read-only
    EndAnnot();


    // Where to put the popup box
    string pos2 = " [" + NStr::NumericToString(int(px+50)) + " " +
        NStr::NumericToString(int(py+50)) + " " +
        NStr::NumericToString(int(px+300)) + " " +
        NStr::NumericToString(int(py+200)) + "]";
    
    CRef<CPdfObject> annot = BeginAnnot();
    CPdfDictionary& annot_dict = annot->GetDictionary();
    popup_dict["Popup"] = new CPdfIndirectObj(annot);

    annot_dict["Parent"] = new CPdfIndirectObj(popup);
    annot_dict["Subtype"] = new CPdfName("Popup");
    annot_dict["Type"] = new CPdfName("Annot");
    // This is where it initially pops up 
    annot_dict["Rect"] = new CPdfElement(pos2);
    annot_dict["Open"] = new CPdfElement(" false");
    
    EndAnnot();
}


void CPdf::AddJSTooltip(CGlPane& pane, const string& txt, const string& title, CVect4<float>& rect)
{
    CRef<CPdfObject> popup = BeginAnnot();

    // Project the coordinates of the label (model coordinates) onto
    // viewport coordinates which are used for the popup.
    TVPRect vp = pane.GetViewport();

	GLint viewport[4];
    GLdouble model_view_matrix[16];
    GLdouble projection_matrix[16];

    viewport[0] = GLint(vp.Left());
    viewport[1] = GLint(vp.Bottom());
    viewport[2] = GLint(vp.Width());
    viewport[3] = GLint(vp.Height());

    IRender& gl = GetGl();
    gl.GetModelViewMatrix(model_view_matrix);
    gl.GetProjectionMatrix(projection_matrix);

    double llx = (double)rect.X();
    double lly = (double)rect.Y();

    double urx = (double)rect.Z();
    double ury = (double)rect.W();

    double px,py,dummyz;

    gluProject(llx, lly, 0.0, 
        model_view_matrix, projection_matrix, viewport, 
        &px, &py, &dummyz);

    string pos = " [" + NStr::NumericToString(int(px)) + " " +
        NStr::NumericToString(int(py)) + " ";

    gluProject(urx, ury, 0.0, 
        model_view_matrix, projection_matrix, viewport, 
        &px, &py, &dummyz);

    pos += NStr::NumericToString(int(px)) + " " +
        NStr::NumericToString(int(py)) + "]";
    //pos = string(" [110.0 110.0 150.0 150.0]");
   
    // You still get an annoying little popup
    CPdfDictionary& popup_dict = popup->GetDictionary();    
    //popup_dict["C"]= new CPdfElement(" [1 0 0 1.0]"); // red box
    popup_dict["Border"]= new CPdfElement(" [0 0 0]"); // colorless annotation
    popup_dict["Type"] = new CPdfName("Annot");
    popup_dict["Rect"] = new CPdfElement(pos);
    popup_dict["Subtype"] = new CPdfName("Link");
    popup_dict["H"] = new CPdfName("I");

    CPdfDictionary* d = new CPdfDictionary();
    (*d)["JS"] = new CPdfElement(" (app.alert\\(\"" + txt + "\", 3, 0, \"" + title + "\"\\))");
    (*d)["S"] = new CPdfName("JavaScript");
    (*d)["Type"] = new CPdfName("Action");
    popup_dict["A"] = d;
    
    EndAnnot();
}

void CPdf::PrintBuffer(CGlVboNode* node, const TVPRect& viewport, 
    CRgbaGradColorTable* color_table)
{
    IRender& gl = GetGl();
    gl.MatrixMode(GL_MODELVIEW);
    gl.PushMatrix();

    for (size_t j=0; j<node->GetPositions().size(); ++j) {
        CMatrix4<float> mat = node->GetTransformedPosition(j);
        mat.Transpose();
        gl.LoadMatrixf(mat.GetData());

        if (node->GetDrawMode() == GL_LINES ||
            node->GetDrawMode() == GL_LINE_STRIP ||
            node->GetDrawMode() == GL_LINE_LOOP) {               
                PrintLineBuffer(node, viewport, color_table);
        }
        if (node->GetDrawMode() == GL_POINTS) {               
            PrintPointBuffer(node, viewport, color_table);
        }
        else if (node->GetDrawMode() == GL_TRIANGLES ||
            node->GetDrawMode() == GL_TRIANGLE_FAN ||
            node->GetDrawMode() == GL_TRIANGLE_STRIP) {                    
                PrintTriBuffer(node, viewport, color_table);
        }
        else if (node->GetDrawMode() == GL_QUADS) {
            PrintQuadBuffer(node, viewport, color_table);
        }

    }

    gl.PopMatrix();
}

bool CPdf::x_GetColors(CGlVboNode* node, vector<CRgbaColor>& colors,
                       CRgbaGradColorTable* color_table)
{
    bool has_color = false;
    IVboGeom::ESecondaryFormat colorFormat = node->GetSecondaryFormat();
    if (colorFormat == IVboGeom::kSecondaryFormatColorFloat) {
        has_color = true;
        node->GetColorBuffer(colors);
    }
    else if (colorFormat == IVboGeom::kSecondaryFormatColorUChar) {
        has_color = true;
        vector<CVect4<unsigned char> > colors_uc;
        colors.clear();
        node->GetColorBufferUC(colors_uc);
        for (size_t i=0; i<colors_uc.size(); ++i)
            colors.push_back(CRgbaColor(colors_uc[i][0], colors_uc[i][1], colors_uc[i][2], colors_uc[i][3]));
    }
    // We may have stored colors in 1-dimensional texture buffer:
    else if (color_table != NULL && colorFormat == IVboGeom::kSecondaryFormatTexture1D) {
        vector<float> tex_coords;
        node->GetTexCoordBuffer1D(tex_coords);
        has_color = true;
        for (size_t i=0; i<tex_coords.size(); ++i) {
            CRgbaColor c = color_table->GetTexCoordColor(tex_coords[i]);
            colors.push_back(c);
        }
    }

    // If we are in grey scale mode, make sure all colors are greyscale. 
    if (has_color && m_IsGreyScale) {
        for (size_t i=0; i<colors.size(); ++i) {
            colors[i] = colors[i].GetGreyscale();
        }
    }

    return has_color;
}

void CPdf::PrintPointBuffer(CGlVboNode* node, const TVPRect& vp, 
    CRgbaGradColorTable* color_table)
{
    vector<CVect2<float> > vertices;
    vector<CRgbaColor> colors;

    node->Get2DVertexBuffer(vertices);

    if (vertices.size() == 0)
        return;

	GLint viewport[4];
    GLdouble model_view_matrix[16];
    GLdouble projection_matrix[16];

    // Get viewport from caller since Opengl value may be truncated
    // since pdf page extents can in many cases be larger than the 
    // OpenGL maximum viewport size.
    viewport[0] = GLint(vp.Left());
    viewport[1] = GLint(vp.Bottom());
    viewport[2] = GLint(vp.Width());
    viewport[3] = GLint(vp.Height());

    IRender& gl = GetGl();
    gl.GetModelViewMatrix(model_view_matrix);
    gl.GetProjectionMatrix(projection_matrix);

    bool has_color = x_GetColors(node, colors, color_table);

    string alpha_state = x_GetAlphaGraphicsState(node);

    CRef<CPdfObject> content = BeginContent(CPdf::ePdfPoints);

    /// Mostly do this for the clipping (push state) so that it doesn't stay active
    content->PushGraphicsState();
    content->SetClipBox(viewport[0], viewport[1], viewport[2], viewport[3]);

    // Use color buffer for color or, if none, use color in State, if set 
    CRgbaColor default_color;
    if (!has_color && node->GetDefaultColor(default_color, m_IsGreyScale)) {
        content->SetColor(default_color);
    }

    float w = 1.0f;
    if (node->GetState().PointSizeSet()) {
        w = node->GetState().GetPointSize();        
    }
    float uu_inv = 1.0f/m_Options.GetUserUnit();
    w *= uu_inv;
    content->SetLineWidth(w);

    content->SetLineCapStyle((int)CGlState::eButtCap);
    content->SetLineJoinStyle((int)CGlState::eMiteredJoin);

    if (alpha_state != "") {
        content->SetGraphicsState(alpha_state);
    }

    // No actual point type in PDF.  A line of 0 length with the right width
    // and square caps (which are 1/2 the width on each end) should give
    // the same results.
    if (node->GetDrawMode() == GL_POINTS) {
        for (size_t i=0; i<vertices.size(); i+=2) {                
            CVect2<float> v1 = vertices[i];
            CVect2<float> v1p, v2p;
            double px, py;
            double dummyz;

            gluProject(v1.X(), v1.Y(), 0.0, 
                model_view_matrix, projection_matrix, viewport, 
                &px, &py, &dummyz);
            v1p.X() = (float)px;
            v1p.Y() = (float)py;
            v2p = v1p;

            // Make length of line (in screen coordinates) same as point size
            // to turn line into a point.
            v2p.X() += w/2.0f;
            v1p.X() -= w/2.0f;

            if (has_color) {
                content->SetColor(colors[i]);
            }

            content->Line(v1p, v2p);
            /*
            if (m_Options.GetMedia().Inside(v1p) &&
                 m_Options.GetMedia().Inside(v2p))
                    content->Line(v1p, v2p);
            else
                _TRACE("Geom outisde");
            */
        }
    }

    content->PushGraphicsState();

    EndContent();
}

void CPdf::PrintLineBuffer(CGlVboNode* node, const TVPRect& vp, 
    CRgbaGradColorTable* color_table)
{
    vector<CVect2<float> > vertices;
    vector<CRgbaColor> colors;

    node->Get2DVertexBuffer(vertices);

    if (vertices.size() == 0)
        return;

	GLint viewport[4];
    GLdouble model_view_matrix[16];
    GLdouble projection_matrix[16];

    viewport[0] = GLint(vp.Left());
    viewport[1] = GLint(vp.Bottom());
    viewport[2] = GLint(vp.Width());
    viewport[3] = GLint(vp.Height());
  
    IRender& gl = GetGl();
    gl.GetModelViewMatrix(model_view_matrix);
    gl.GetProjectionMatrix(projection_matrix);

    // Write out vertices...
    CRef<CPdfObject> content = BeginContent(CPdf::ePdfLines);

    /// Mostly do this for the clipping (push state) so that it doesn't stay active
    content->PushGraphicsState();
    content->SetClipBox(viewport[0], viewport[1], viewport[2], viewport[3]);

    bool has_color = x_GetColors(node, colors, color_table);

    string alpha_state = x_GetAlphaGraphicsState(node);

    // Use color buffer for color or, if none, use color in State.
    CRgbaColor default_color;
    if (node->GetDefaultColor(default_color, m_IsGreyScale)) {
        content->SetColor(default_color);
    }

    if (node->GetState().LineWidthSet()) {
        float w = node->GetState().GetLineWidth();
        float uu_inv = 1.0f/m_Options.GetUserUnit();
        w *= uu_inv;

        content->SetLineWidth(w);
    }

    if (node->GetState().LineCapStyleSet()) {
        content->SetLineCapStyle((int)node->GetState().GetLineCapStyle());
    }

    if (node->GetState().LineJoinStyleSet()) {
        content->SetLineJoinStyle((int)node->GetState().GetLineJoinStyle());
    }

    if (node->GetState().IsEnabled(GL_LINE_STIPPLE) &&
        node->GetState().LineStippleSet()) {
        GLint factor;
        GLushort pattern;

        node->GetState().GetLineStipple(factor, pattern);
        content->SetLineDashStyle(factor, pattern);
    }
    else {
        // explicitly set to 0 when not enabled to avoid accidental stippling
        content->SetLineDashStyle(0, 0);
    }

    if (alpha_state != "") {
        content->SetGraphicsState(alpha_state);
    }

    if (node->GetDrawMode() == GL_LINES) {
        for (size_t i=0; i<vertices.size(); i+=2) {   
            if (i+1 >= vertices.size())
                break;
            CVect2<float> v1 = vertices[i];
            CVect2<float> v2 = vertices[i+1];
            CVect2<float> v1p;
            CVect2<float> v2p;
            double px, py;
            double dummyz;

            gluProject(v1.X(), v1.Y(), 0.0, 
                model_view_matrix, projection_matrix, viewport, 
                &px, &py, &dummyz);
            v1p.X() = (float)px;
            v1p.Y() = (float)py;
            gluProject(v2.X(), v2.Y(), 0.0, 
                model_view_matrix, projection_matrix, viewport, 
                &px, &py, &dummyz);
            v2p.X() = (float)px;
            v2p.Y() = (float)py;

            if (has_color) {
                CRgbaColor c1 = colors[i];
                CRgbaColor c2 = colors[i+1];

                // If ends are different colors, need to draw with a gradient.
                // PDF has support for gradients with axial rendering and shfill
                // https://partners.adobe.com/public/developer/en/ps/sdk/TN5600.SmoothShading.pdf
                // but we have not currrently implmented support for these yet other
                // than triangle shading.
                float cdist = CRgbaColor::ColorDistance(c1, c2);
                float dist = (v1p-v2p).Length();

                if (cdist > 0.1f && dist > 5.0f) {
                    // segment line based on color change and distance - for 
                    // maximum color change (cdist=1) have a vertex every 5 pixels
                    float offset = 5.0f/cdist;
                    int num_segments = (int)ceilf(dist/offset); // so at least 1
                    float delta_pct = 1.0f/(float)num_segments;
                    float delta = 0.0f;  
                    CVect2<float> seg_start = v1p;
                    CVect2<float> seg_end;

                    for (int j=0; j<num_segments; ++j) {
                        CRgbaColor c = c1*(1.0f-delta) + c2*delta;
                        delta += delta_pct;
                        seg_end = v1p*(1.0f-delta) + v2p*delta;

                        string current_alpha_state = x_GetAlphaGraphicsState(node, c.GetAlpha());
                        if (alpha_state != current_alpha_state) {
                            content->SetGraphicsState(current_alpha_state);
                            alpha_state = current_alpha_state;
                        }

                        content->SetColor(c);
                        content->Line(seg_start, seg_end);

                        seg_start = seg_end;
                    }

                }
                else {
                    string current_alpha_state = x_GetAlphaGraphicsState(node, colors[i].GetAlpha());
                    if (alpha_state != current_alpha_state) {
                        content->SetGraphicsState(current_alpha_state);
                        alpha_state = current_alpha_state;
                    }

                    content->SetColor(colors[i]);
                    content->Line(v1p, v2p);
                }              
            }
            else {
                content->Line(v1p, v2p);
            }
        }
    }
    else if (node->GetDrawMode() == GL_LINE_STRIP) {
        for (size_t i=1; i<vertices.size(); i++) {                
            CVect2<float> v1 = vertices[i-1];
            CVect2<float> v2 = vertices[i];
            CVect2<float> v1p;
            CVect2<float> v2p;
            double px, py;
            double dummyz;

            gluProject(v1.X(), v1.Y(), 0.0, 
                model_view_matrix, projection_matrix, viewport, 
                &px, &py, &dummyz);
            v1p.X() = (float)px;
            v1p.Y() = (float)py;
            gluProject(v2.X(), v2.Y(), 0.0, 
                model_view_matrix, projection_matrix, viewport, 
                &px, &py, &dummyz);
            v2p.X() = (float)px;
            v2p.Y() = (float)py;

            if (has_color) {
                content->SetColor(colors[i]);
                string current_alpha_state = x_GetAlphaGraphicsState(node, colors[i].GetAlpha());
                if (alpha_state != current_alpha_state) {
                    content->SetGraphicsState(current_alpha_state);
                    alpha_state = current_alpha_state;
                }
            }
            
            content->Line(v1p, v2p);
        }
    }
    else if (node->GetDrawMode() == GL_LINE_LOOP) {
        for (size_t i=1; i<vertices.size(); i++) {                
            CVect2<float> v1 = vertices[i-1];
            CVect2<float> v2 = vertices[i];
            CVect2<float> v1p;
            CVect2<float> v2p;
            double px, py;
            double dummyz;

            gluProject(v1.X(), v1.Y(), 0.0, 
                model_view_matrix, projection_matrix, viewport, 
                &px, &py, &dummyz);
            v1p.X() = (float)px;
            v1p.Y() = (float)py;
            gluProject(v2.X(), v2.Y(), 0.0, 
                model_view_matrix, projection_matrix, viewport, 
                &px, &py, &dummyz);
            v2p.X() = (float)px;
            v2p.Y() = (float)py;

            if (has_color) {
                content->SetColor(colors[i]);
                string current_alpha_state = x_GetAlphaGraphicsState(node, colors[i].GetAlpha());
                if (alpha_state != current_alpha_state) {
                    content->SetGraphicsState(current_alpha_state);
                    alpha_state = current_alpha_state;
                }
            }
            
            content->Line(v1p, v2p);
        }
        CVect2<float> v1 = vertices[0];
        CVect2<float> v2 = vertices[vertices.size()-1];
        CVect2<float> v1p;
        CVect2<float> v2p;
        double px, py;
        double dummyz;

        gluProject(v1.X(), v1.Y(), 0.0, 
            model_view_matrix, projection_matrix, viewport, 
            &px, &py, &dummyz);
        v1p.X() = (float)px;
        v1p.Y() = (float)py;
        gluProject(v2.X(), v2.Y(), 0.0, 
            model_view_matrix, projection_matrix, viewport, 
            &px, &py, &dummyz);
        v2p.X() = (float)px;
        v2p.Y() = (float)py;

        content->Line(v1p, v2p);
    }

    content->PopGraphicsState();

    EndContent();
}

void CPdf::x_PrintFlatTriBuffer(CGlVboNode* node, const TVPRect& vp, 
                                CRgbaGradColorTable* color_table)
{
    vector<CVect2<float> > vertices;
    vector<CRgbaColor> colors;

    node->Get2DVertexBuffer(vertices);

    if (vertices.size() == 0)
        return;

	GLint viewport[4];
    GLdouble model_view_matrix[16];
    GLdouble projection_matrix[16];

    viewport[0] = GLint(vp.Left());
    viewport[1] = GLint(vp.Bottom());
    viewport[2] = GLint(vp.Width());
    viewport[3] = GLint(vp.Height());
  
    IRender& gl = GetGl();
    gl.GetModelViewMatrix(model_view_matrix);
    gl.GetProjectionMatrix(projection_matrix);

    string alpha_state = x_GetAlphaGraphicsState(node);

    // Write out vertices...
    CRef<CPdfObject> content = BeginContent(CPdf::ePdfFlatTris);

    /// Mostly do this for the clipping (push state) so that it doesn't stay active
    content->PushGraphicsState();
    content->SetClipBox(viewport[0], viewport[1], viewport[2], viewport[3]);

    // Normally a color buffer would force ShadingType 4, but if user
    // wants to force flat rendering would could wind up here in which case
    // we can user the buffer colors per-triangle rather than per vertex
    // (or per-perimeter)
    bool has_color = x_GetColors(node, colors, color_table);

    bool flat_shading = (node->GetState().GetShadeModel() == GL_FLAT);

    // use color in State.  Since triangles are flat, there is no color buffer
    // (if we see a buffer we assume it is not flat)
    CRgbaColor default_color;
    if (node->GetDefaultColor(default_color, m_IsGreyScale)) {
        content->SetColor(default_color);
        content->SetFillColor(default_color);
    }

    size_t i;
    for (i=0; i<vertices.size(); ++i) {
        CVect2<float> v1 = vertices[i];    
        CVect2<float> v1p;      
        double px, py;
        double dummyz;

        gluProject(v1.X(), v1.Y(), 0.0, 
            model_view_matrix, projection_matrix, viewport, 
            &px, &py, &dummyz);
        vertices[i].Set(float(px), float(py));
    }


    // GL_LINE or GL_FILL
    bool fill = true;
    if (node->GetState().GetPolygonMode() == GL_LINE) {
        fill = false;

        if (node->GetState().LineCapStyleSet()) {
            content->SetLineCapStyle((int)node->GetState().GetLineCapStyle());
        }
        if (node->GetState().LineJoinStyleSet()) {
            content->SetLineJoinStyle((int)node->GetState().GetLineJoinStyle());
        }
    }

    if (alpha_state != "") {
        content->SetGraphicsState(alpha_state);
    }

    if (node->GetDrawMode() == GL_TRIANGLES) {
        
        CTriPerimeter tp;
        for (i=0; i<vertices.size(); i+=3) {     
            if (i+2 >= vertices.size())
                break;

            if (has_color && flat_shading) {
                if (!(colors[i] == colors[0])) {
                    tp.Clear();
                    break;
                }
            }

            tp.AddTri(vertices[i], vertices[i+1], vertices[i+2]);
        }

        // Can we convert this into a single perimeter?
        vector<CVect2<float> > perimeter = tp.GetPerimiter();

        if (perimeter.size() > 2 && fill) {
            if (has_color) {
                content->SetFillColor(colors[0]);
                content->SetColor(colors[0]);                
                string current_alpha_state = x_GetAlphaGraphicsState(node, colors[0].GetAlpha());
                if (alpha_state != current_alpha_state) {
                    content->SetGraphicsState(current_alpha_state);
                    alpha_state = current_alpha_state;
                }
            }

            content->Poly(perimeter);
        }
        else {
            for (i=0; i<vertices.size(); i+=3) {     
                if (i+2 >= vertices.size())
                    break;
                CVect2<float> v1p = vertices[i];
                CVect2<float> v2p = vertices[i+1];
                CVect2<float> v3p = vertices[i+2];

                if (has_color) {
                    content->SetFillColor(colors[i]);
                    content->SetColor(colors[i]);                
                    string current_alpha_state = x_GetAlphaGraphicsState(node, colors[i].GetAlpha());
                    if (alpha_state != current_alpha_state) {
                        content->SetGraphicsState(current_alpha_state);
                        alpha_state = current_alpha_state;
                    }
                }

                if (fill) {
                    content->Tri(v1p, v2p, v3p);
                }
                else {
                    content->Line(v1p, v2p);
                    content->Line(v2p, v3p);
                    content->Line(v3p, v1p);  
                }
            }
        }
    }
    else if (node->GetDrawMode() == GL_TRIANGLE_STRIP) {

        CTriPerimeter tp;
        for (size_t j=2; j<vertices.size(); ++j) {             

            if (has_color && flat_shading) {
                if (!(colors[j] == colors[0])) {
                    tp.Clear();
                    break;
                }
            }

            tp.AddTri(vertices[j], vertices[j-1], vertices[j-2]);
        }

        // Can we convert this into a single perimeter?
        vector<CVect2<float> > perimeter = tp.GetPerimiter();

        if (perimeter.size() > 2 && fill) {
            if (has_color) {
                content->SetFillColor(colors[0]);
                content->SetColor(colors[0]);                
                string current_alpha_state = x_GetAlphaGraphicsState(node, colors[0].GetAlpha());
                if (alpha_state != current_alpha_state) {
                    content->SetGraphicsState(current_alpha_state);
                    alpha_state = current_alpha_state;
                }
            }

            content->Poly(perimeter);
        }
        else {
            for (size_t j=2; j<vertices.size(); ++j) {                        
                CVect2<float> v1p = vertices[j];
                CVect2<float> v2p = vertices[j-1];
                CVect2<float> v3p = vertices[j-2];

                if (has_color) {
                    content->SetFillColor(colors[j]);
                    content->SetColor(colors[j]);
                    string current_alpha_state = x_GetAlphaGraphicsState(node, colors[j].GetAlpha());
                    if (alpha_state != current_alpha_state) {
                        content->SetGraphicsState(current_alpha_state);
                        alpha_state = current_alpha_state;
                    }
                }
                if (fill) {
                    content->Tri(v1p, v2p, v3p);
                }
                else {
                    content->Line(v1p, v2p);
                    content->Line(v2p, v3p);
                    content->Line(v3p, v1p);       
                }
            }
        }
    }
    else if (node->GetDrawMode() == GL_TRIANGLE_FAN) {
        CVect2<float> v1p = vertices[0];

        vector<CVect2<float> > perimeter;
        for (size_t j=1; j<vertices.size(); ++j) { 
            perimeter.push_back(vertices[j]);        
        }

        // would user maybe want perimiter for edges? The could have always
        // just drawn edges for that, rather than tris.
        if (fill) {
            if (has_color) {
                content->SetFillColor(colors[0]);
                content->SetColor(colors[0]);                
                string current_alpha_state = x_GetAlphaGraphicsState(node, colors[0].GetAlpha());
                if (alpha_state != current_alpha_state) {
                    content->SetGraphicsState(current_alpha_state);
                    alpha_state = current_alpha_state;
                }
            }

            content->Poly(perimeter);
        }
        else {
            for (size_t j=2; j<vertices.size(); ++j) {                
                CVect2<float> v2p = vertices[j-1];
                CVect2<float> v3p = vertices[j];

                if (has_color) {
                    content->SetFillColor(colors[i]);
                    content->SetColor(colors[i]);
                    string current_alpha_state = x_GetAlphaGraphicsState(node, colors[i].GetAlpha());
                    if (alpha_state != current_alpha_state) {
                        content->SetGraphicsState(current_alpha_state);
                        alpha_state = current_alpha_state;
                    }
                }
                else {
                    content->Line(v1p, v2p);
                    content->Line(v2p, v3p);
                    content->Line(v3p, v1p);
                }
            }
        }
    }

    content->PopGraphicsState();

    EndContent();
}

string CPdf::AddShadedTris(CGlVboNode* node, 
                           const TVPRect& vp, 
                           CRgbaGradColorTable* color_table,
                           CPdfObject::EBitCount bit_count) 
{
    // Iterate over all vertices and find the min/max values that capture the range 
    // for this shading object (set of shaded triangles).  You can give the pdf file
    // separate ranges for x and y but we will leave that out for now since most data
    // is clustered near the origin (x and y ranges are similar)
    if (node == NULL)
        return "";

    vector<CVect2<float> > vertices;

    node->Get2DVertexBuffer(vertices);

    if (vertices.size() < 3)
        return "";

    float minx = std::numeric_limits<float>::max();
    float miny = std::numeric_limits<float>::max();

    float maxx = -std::numeric_limits<float>::max();
    float maxy = -std::numeric_limits<float>::max();

    for (size_t v=0; v<vertices.size(); ++v) {
        minx = std::min(vertices[v].X(), minx);
        miny = std::min(vertices[v].Y(), miny);
        maxx = std::max(vertices[v].X(), maxx);
        maxy = std::max(vertices[v].Y(), maxy);
    }    

    // Can't have a 0 or near 0 range size (the range is used in the pdf file
    // to set up values to convert the floating point values to integers in
    // a larger range so a little larger is no problem (see ShadingType 4, Decode)
    return AddShadedTris(node, vp, bit_count, color_table, 
                         int(minx-10.0f), int(maxx+10.0f), 
                         int(miny-10.0f), int(maxy+10.0f));
}

string CPdf::AddShadedTris(CGlVboNode* node, 
                           const TVPRect& /*vp*/,
                           CPdfObject::EBitCount bit_count,
                           CRgbaGradColorTable* color_table,
                           int range_minx, int range_maxx,
                           int range_miny, int range_maxy)
{
    if (node == NULL)
        return "";

    vector<CVect2<float> > vertices;
    vector<CRgbaColor> colors;

    node->Get2DVertexBuffer(vertices);

    if (vertices.size() < 3)
        return "";

    CRgbaColor c;
    node->GetDefaultColor(c, m_IsGreyScale);

    CRgbaColor c1 = c;
    CRgbaColor c2 = c;
    CRgbaColor c3 = c;

    bool has_color = x_GetColors(node, colors, color_table);

    CPdf::TPdfObjectRef tris = BeginReference();
    tris->StartTris(CPdfObject::eASCIIHex, bit_count, range_minx, range_maxx, 
                    range_miny, range_maxy);

    
    if (node->GetDrawMode() == GL_TRIANGLES) {
        for (size_t v=0; v+2<vertices.size(); v+=3) {
            if (has_color) {
                c1 = colors[v];
                c2 = colors[v+1];
                c3 = colors[v+2];
            }
            tris->Tri(0, vertices[v], c1, vertices[v+1], c2, vertices[v+2], c3);
        }    
    }
    else if (node->GetDrawMode() == GL_TRIANGLE_STRIP) {         
        if (has_color) {
            c1 = colors[0];
            c2 = colors[1];
            c3 = colors[2];
        }
        tris->Tri(0, vertices[0], c1, vertices[1], c2, vertices[2], c3);

        for (size_t v=3; v<vertices.size(); ++v) {
            if (has_color) {
                c1 = colors[v];
            }
            tris->Tri(1, vertices[v], c1);
        }
    }
    else if (node->GetDrawMode() == GL_TRIANGLE_FAN) {
        if (has_color) {
            c1 = colors[0];
            c2 = colors[1];
            c3 = colors[2];
        }
        tris->Tri(0, vertices[0], c1, vertices[1], c2, vertices[2], c3);

        for (size_t v=3; v<vertices.size(); ++v) {
            if (has_color) {
                c1 = colors[v];
            }
            tris->Tri(2, vertices[v], c1);
        }
    }

    tris->EndTris();
    EndReference();

    CRef<CPdfElement>& res = (*PageDictionary())["Resources"];
    if ( !res ) {
        res.Reset(new CPdfDictionary());
    }

    CRef<CPdfDictionary> _res(dynamic_cast <CPdfDictionary*>(res.GetPointer()));
    CRef<CPdfElement>& shaders = (*_res)["Shading"];
    if ( !shaders ) {
        shaders.Reset(new CPdfDictionary());
    }

    CRef<CPdfDictionary> _shaders(dynamic_cast <CPdfDictionary*>(shaders.GetPointer()));

    string shader_id = _shaders->GenShaderID();
    (*_shaders)[shader_id.c_str()] = new CPdfIndirectObj(tris);

    return shader_id;
}

void CPdf::RenderShaderInstance(CGlVboNode* node, const string& shader_id, const TVPRect& vp)
{
	GLint viewport[4];
    GLdouble model_view_matrix[16];
    GLdouble projection_matrix[16];

    viewport[0] = GLint(vp.Left());
    viewport[1] = GLint(vp.Bottom());
    viewport[2] = GLint(vp.Width());
    viewport[3] = GLint(vp.Height());

    IRender& gl = GetGl();
    gl.GetModelViewMatrix(model_view_matrix);
    gl.GetProjectionMatrix(projection_matrix);

    CMatrix3<double> mat, matvt, matvs;
    matvt.Identity();
    matvs.Identity();

    // Compute modelview projection matrix and then get 
    // homogeneous 2D (3x3) matrix for PDF from that result (ignore z)
    CMatrix4<double> mp(projection_matrix);
    CMatrix4<double> mm(model_view_matrix);    
    mp.Transpose();
    mm.Transpose();
    CMatrix4<double> mvp = mp*mm;
    CMatrix3<double> mvp33 = CMatrix3<double>(mvp(0, 0), mvp(0, 1), mvp(0, 3),
                                              mvp(1, 0), mvp(1, 1), mvp(1, 3),
                                              mvp(3, 0), mvp(3, 1), mvp(3, 3));
     

    matvs(0,0) = ((double)viewport[2])/2.0;
    matvs(1,1) = ((double)viewport[3])/2.0;

    matvt(0,2) = matvs(0,0) + double(viewport[0]);
    matvt(1,2) = matvs(1,1) + double(viewport[1]);    

    mat = matvt*matvs*mvp33;

    // can't do alpha in tris vertex-by-vertex, but if there is a color array, use alpha from there,
    // but only from the first element in the array!
    string alpha_state = x_GetAlphaGraphicsState(node);
             
    CPdf::TPdfObjectRef shade_obj = BeginContent(CPdf::ePdfTris);  

    /// Mostly do this for the clipping (push state) so that it doesn't stay active
    shade_obj->PushGraphicsState();
    shade_obj->SetClipBox(viewport[0], viewport[1], viewport[2], viewport[3]);

    //(*shade_obj) << CPdfElement("q") << pdfbrk;
    (*shade_obj) << CPdfTransform(mat) << '\n';
    if (alpha_state != "")
        shade_obj->SetGraphicsState(alpha_state);    
    (*shade_obj) << "/" << shader_id << " sh" << '\n';
    //(*shade_obj) << CPdfElement("Q") << pdfbrk;  

    shade_obj->PopGraphicsState();
    EndContent();  
}

void CPdf::PrintTriBuffer(CGlVboNode* node, const TVPRect& vp, 
                          CRgbaGradColorTable* color_table)
{
    // decide if the triangles are gouraud (diff colors for each
    // vertex) or not.  If the shading state is flat, or the color
    // buffer is empty, we have flat shaded tris.

    // Another issue with pdf is that a region tiled together out
    // of many polygons will show cracks if it is done using flat polygons -
    // only shaded polys seem to be able to join to hide cracks. The
    // alternative for flat is to provide the convex hull of the poly,
    // but we can't just do that since we don't know if it is convex.
    bool shaded = true;

    if (node->GetState().GetPolygonMode() == GL_LINE ||
        (node->GetState().GetShadeModel() == GL_FLAT &&
         node->GetState().GetPdfShadeStyle() == CGlState::eFlat)) {
        shaded = false;
    }

    if (shaded && !m_Options.GetGouraudShadingDisabled()) {
        string id = AddShadedTris(node, vp, color_table);
        if (id != "")
            RenderShaderInstance(node, id, vp);
    }
    else {
        x_PrintFlatTriBuffer(node, vp, color_table);
    }
}

void CPdf::PrintQuadBuffer(CGlVboNode* node, const TVPRect& vp, 
                           CRgbaGradColorTable* color_table)
{
    vector<CVect2<float> > vertices;
    vector<CRgbaColor> colors;

    node->Get2DVertexBuffer(vertices);

    if (vertices.size() == 0)
        return;

	GLint viewport[4];
    GLdouble model_view_matrix[16];
    GLdouble projection_matrix[16];

    viewport[0] = GLint(vp.Left());
    viewport[1] = GLint(vp.Bottom());
    viewport[2] = GLint(vp.Width());
    viewport[3] = GLint(vp.Height());
  
    IRender& gl = GetGl();
    gl.GetModelViewMatrix(model_view_matrix);
    gl.GetProjectionMatrix(projection_matrix);

    bool has_color = x_GetColors(node, colors, color_table);
    if (has_color) {

        if (node->GetState().GetShadeModel() != GL_FLAT) {    
            // Warn we see that there is a 
            // color buffer and that the colors in the buffer are not all the 
            // same for the vertices of at least one quad.
            for (size_t i=3; i<colors.size(); i+=4) {
                if (!(colors[i] == colors[i-3]) ||
                    !(colors[i] == colors[i-2]) ||
                    !(colors[i] == colors[i-1])) {
                        LOG_POST(Warning << "Warning - printing quads flat while \
                                             OpenGL state is shaded (only tris may be shaded)");
                        break;
                }
            }
        }
    }

    string alpha_state = x_GetAlphaGraphicsState(node);

    // Start new pdf object
    CRef<CPdfObject> content = BeginContent(CPdf::ePdfQuads);

    /// Mostly do this for the clipping (push state) so that it doesn't stay active
    content->PushGraphicsState();
    content->SetClipBox(viewport[0], viewport[1], viewport[2], viewport[3]);

    // use color in State.  Since triangles are flat, there is no color buffer
    // (if we see a buffer we assume it is not flat)
    CRgbaColor c;
    if (node->GetDefaultColor(c, m_IsGreyScale)) {
        content->SetColor(c);
        content->SetFillColor(c);
    }

    // GL_LINE or GL_FILL
    bool fill = true;
    if (node->GetState().GetPolygonMode() == GL_LINE) {
        fill = false;

        if (node->GetState().LineCapStyleSet()) {
            content->SetLineCapStyle((int)node->GetState().GetLineCapStyle());
        }
        if (node->GetState().LineJoinStyleSet()) {
            content->SetLineJoinStyle((int)node->GetState().GetLineJoinStyle());
        }
    }

    if (alpha_state != "") {
        content->SetGraphicsState(alpha_state);
    }

    for (size_t i=3; i<vertices.size(); i+=4) {         
        CVect2<float> v1 = vertices[i-3];
        CVect2<float> v2 = vertices[i-2];
        CVect2<float> v3 = vertices[i-1];
        CVect2<float> v4 = vertices[i];
        CVect2<float> v1p;
        CVect2<float> v2p;
        CVect2<float> v3p;
        CVect2<float> v4p;
        double px, py;
        double dummyz;

        gluProject(v1.X(), v1.Y(), 0.0, 
            model_view_matrix, projection_matrix, viewport, 
            &px, &py, &dummyz);
        v1p.X() = (float)px;
        v1p.Y() = (float)py;
        gluProject(v2.X(), v2.Y(), 0.0, 
            model_view_matrix, projection_matrix, viewport, 
            &px, &py, &dummyz);
        v2p.X() = (float)px;
        v2p.Y() = (float)py;
        gluProject(v3.X(), v3.Y(), 0.0, 
            model_view_matrix, projection_matrix, viewport, 
            &px, &py, &dummyz);
        v3p.X() = (float)px;
        v3p.Y() = (float)py;
        gluProject(v4.X(), v4.Y(), 0.0, 
            model_view_matrix, projection_matrix, viewport, 
            &px, &py, &dummyz);
        v4p.X() = (float)px;
        v4p.Y() = (float)py;
       
        if (has_color) {
            content->SetFillColor(colors[i]);
            content->SetColor(colors[i]);

            string current_alpha_state = x_GetAlphaGraphicsState(node, colors[i].GetAlpha());
            if (alpha_state != current_alpha_state) {
                content->SetGraphicsState(current_alpha_state);
                alpha_state = current_alpha_state;
            }
        }

        if (fill) {
            content->Quad(v1p, v2p, v3p, v4p);
        }
        else {
            content->Line(v1p, v2p);
            content->Line(v2p, v3p);
            content->Line(v3p, v4p);
            content->Line(v4p, v1p);
        }
    }

    content->PopGraphicsState();

    EndContent();
}

void CPdf::EndDocument()
{
    m_PageHandler->WritePageTree(m_PageDictionary);

    ITERATE(vector<TPdfObjectRef>, it, m_PrintInEndDoc) {
        m_ObjectWriter->WriteObject(*it);
    }

    // the number of objects is defined as one more than the highest
    // object number
    const unsigned int num_objects = m_ObjIdGenerator->NextId();

    // write the cross reference
    const CT_POS_TYPE xref_start = m_ObjectWriter->WriteXRef(num_objects);

    // write the trailer
    (*m_Trailer)["Size"] = new CPdfNumber(num_objects);
    *m_Strm << *m_Trailer;

    // output the location of the last xref section
    *m_Strm << "startxref" << endl << (xref_start - CT_POS_TYPE(0)) << endl;

    *m_Strm << "%%EOF" << pdfeol;
}

string CPdf::x_GetAlphaGraphicsState(CGlVboNode* node)
{
    size_t vertex_buf_size = node->GetVertexCount();
    if (vertex_buf_size == 0)
        return "";
  
    float alpha = 1.0f;

    // Only support most standard (alpha/one-minus-alpha) blending. If that is 
    // not the mode set in OpenGL do not have blending.  (It is the default too
    // so if blending is enabled but function not set, we enable it).
    if ((node->GetState().IsEnabled(GL_BLEND) &&
         !node->GetState().BlendFuncSet()) ||
        (node->GetState().GetSourceBlendFactor() == GL_SRC_ALPHA && 
         node->GetState().GetTargetBlendFactor() == GL_ONE_MINUS_SRC_ALPHA)) {   

            // First get alpha value from color in state
            if (node->GetState().ColorSet())
                alpha = node->GetState().GetColor().GetAlpha();

            // Then check for color (and alpha) in the color buffer.  Note that this takes alpha
            // from the FIRST COLOR.  If other vertices have different alpha values we will not
            // capture that.
            if (node->GetSecondaryFormat() == IVboGeom::kSecondaryFormatColorFloat) {
                vector<CRgbaColor> colors;
                node->GetColorBuffer(colors);
                alpha = colors[0].GetAlpha();
            }
    }

    string state_name = x_GetAlphaGraphicsState(node, alpha);    

    return state_name;
}


string CPdf::x_GetAlphaGraphicsState(CGlVboNode* node, float alpha)
{
    string state_name;

    // If blending is not enabled or enabled but set to a blending function
    // other than src_alpha, one_minus_src_alpha, do not blend (alpha=1.0f)
    if (!node->GetState().IsEnabled(GL_BLEND)) {
        alpha = 1.0f;
    }
    else {
        if (node->GetState().BlendFuncSet() &&
            (node->GetState().GetSourceBlendFactor() != GL_SRC_ALPHA ||
             node->GetState().GetTargetBlendFactor() != GL_ONE_MINUS_SRC_ALPHA)) { 
                alpha = 1.0f;
        }
    }

    return x_GetAlphaGraphicsState(alpha);
}


string CPdf::x_GetAlphaGraphicsState(float alpha)
{
    string state_name;

    int rounded_alpha = int(alpha*100.0f);
    state_name = "A" + NStr::IntToString(rounded_alpha);

    CRef<CPdfElement>& res = (*PageDictionary())["Resources"];
    if ( !res ) {
        res.Reset(new CPdfDictionary());
    }
    CRef<CPdfDictionary> _res(dynamic_cast <CPdfDictionary*>(res.GetPointer()));

    CRef<CPdfElement>& extgstate = (*_res)["ExtGState"];
    if ( !extgstate ) {
        extgstate.Reset(new CPdfDictionary());
    }
    CRef<CPdfDictionary> _extgstate(dynamic_cast <CPdfDictionary*>(extgstate.GetPointer()));

    CRef<CPdfElement>& gs = (*_extgstate)[state_name];
    if (gs.IsNull()) {
        TPdfObjectRef graphics_state;
        graphics_state.Reset(new CPdfObject(m_ObjIdGenerator->NextId()));
        CPdfObject& state = *graphics_state;
        state["Type"] = new CPdfName("ExtGState");
        state["ca"] = new CPdfNumber(alpha);
        state["CA"] = new CPdfNumber(alpha);
        m_ObjectWriter->WriteObject(graphics_state);
        gs.Reset(new CPdfIndirectObj(graphics_state));
    }
    return state_name;
}

END_NCBI_SCOPE
