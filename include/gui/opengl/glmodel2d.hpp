#ifndef GUI_OPENGL___GL_MODEL2D__HPP
#define GUI_OPENGL___GL_MODEL2D__HPP

/*  $Id: glmodel2d.hpp 42933 2019-04-26 20:12:15Z katargir $
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

#include <gui/utils/vect2.hpp>
#include <gui/utils/rgba_color.hpp>
#include <gui/opengl/glpane.hpp>
#include <gui/opengl/globject.hpp>
#include <gui/opengl/spatialhash2d.hpp>
#include <gui/opengl/glrendernode.hpp>
#include <gui/opengl/glvbonode.hpp>
//#include <gui/print/pdf.hpp>

/** @addtogroup GUI_OPENGL
 *
 * @{
 */

BEGIN_NCBI_SCOPE

class CPdf;

//////////////////////////////////////////////////////////////////////////////
/// CGlModel2D
/// Base class for a model set up for rendering purposes.  Point of the class
/// is to hold a set of renderable geometry packaged into CGlRenderNode 
/// subclasses.  This base class does not do collision-detection, but
/// derived classes may.
///
class NCBI_GUIOPENGL_EXPORT CGlModel2D : public CGlObject
{
public:
    CGlModel2D();
    virtual ~CGlModel2D();

    /// Call immediately before rendering to copy data from temporary
    /// buffers into vertex buffer objects
    virtual void SyncBuffers();
    /// clear everything - vectors and vertex buffers
    virtual void ClearAll();
    /// clear any geometry vectors but do not update vertex buffers
    virtual void ClearArrays();

    
    /// Draw to the screen
    virtual void Render(CGlPane& pane);

    /// Draw to PDF file (base funcions in cpdf::PrintModel)
    virtual void RenderPDF(CGlPane& pane, CRef<CPdf>& pdf) {}

    /// Find a geometry (vertex buffer object) node by name
    CGlVboNode* FindGeomNode(const string& name);
    /// Add a geometry (vertex buffer object) node.  'node_type' should be
    /// one of GL_POINTS, GL_LINES, or GL_TRIANGLES
    CGlVboNode* AddGeomNode(GLenum node_type, 
                            const string& name,
                            bool visible = true);
    /// Same as above, but insert node before specified index
    CGlVboNode* AddGeomNode(GLenum node_type,
                            const string& name,
                            size_t idx, 
                            bool visible = true);

    /// Add a geometry node of unspecified type (caller will have to allocate
    /// appropriate buffer type for the node)
    CGlVboNode* AddGeomNode(const string& name,
                            bool visible = true); 
    /// Same as above, but insert node before specified index
    CGlVboNode* AddGeomNode(const string& name,
                            size_t idx,
                            bool visible = true);

    vector<CGlVboNode*>& GetNodes() { return m_GeomNodes; }


    /// Find a geometry object (just geometry - not a render node) by name
    CIRef<IVboGeom> FindGeomObject(const string& name);
    /// Add a geometry (vertex buffer object) node.  'node_type' should be
    /// one of GL_POINTS, GL_LINES, or GL_TRIANGLES
    CIRef<IVboGeom> AddGeomObject(GLenum node_type, 
                                    const string& name);
    vector<CIRef<IVboGeom> >& GetGeomObjects() { return m_GeomObjects; }
    
    /// Add a geometry node of unspecified type (caller will have to allocate
    /// appropriate buffer type for the node)
    CGlVboNode* AddTempGeomNode(const string& name,
                                bool visible = true); 
    CGlVboNode* AddTempGeomNode(GLenum node_type, 
                                const string& name,
                                bool visible = true); 
    vector<CGlVboNode*>& GetTempNodes() { return m_TempGeomNodes; }


protected:

    /// For gluProject()
	GLint m_Viewport[4];
    GLdouble m_ModelviewMatrix[16];
    GLdouble m_ProjectionMatrix[16];

    /// Set of rendernodes with vertex buffer objects.  Not deleted
    /// when ClearArrays() is called
    vector<CGlVboNode*> m_GeomNodes;

    /// Geometry to use in nodes (can be shared between nodes)
    vector<CIRef<IVboGeom> > m_GeomObjects;

    /// Set of single-use rendernodes (which may use geom from m_Geom)
    /// These are deleted when ClearArrays is called.
    vector<CGlVboNode*> m_TempGeomNodes;
};



END_NCBI_SCOPE

/* @} */

#endif  // GUI_OPENGL___GL_MODEL2D__HPP
