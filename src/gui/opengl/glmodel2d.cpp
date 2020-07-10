/*  $Id: glmodel2d.cpp 42939 2019-04-29 14:53:36Z katargir $
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
#include <gui/opengl/glmodel2d.hpp>
#include <gui/opengl/glutils.hpp>
#include <gui/opengl/glresmgr.hpp>
//#include <gui/print/pdf.hpp>


#include <gui/utils/vect2.hpp>
#include <gui/utils/vect3.hpp>
#include <gui/utils/vect4.hpp>
#include <gui/utils/matrix3.hpp>
#include <gui/utils/matrix4.hpp>

BEGIN_NCBI_SCOPE

CGlModel2D::CGlModel2D()
{
}

CGlModel2D::~CGlModel2D()
{
    size_t i;

    for (i=0; i<m_GeomNodes.size(); ++i) {
        delete m_GeomNodes[i];
    }
    m_GeomNodes.clear();

    for (i=0; i<m_TempGeomNodes.size(); ++i) {       
        delete m_TempGeomNodes[i];
    }
    m_TempGeomNodes.clear();
}

void CGlModel2D::SyncBuffers()
{
    // Make sure color class is right size (nobody has added virtual functions
    // or other data).  This is required to use the opengl glBufferData() command for
    // arrays of CRgbaColor.
    _ASSERT(sizeof(CRgbaColor) == 16);        
}

void CGlModel2D::ClearAll()
{
    ClearArrays();

    size_t i;

    m_GeomNodes.clear();

    for (i=0; i<m_TempGeomNodes.size(); ++i) {       
        delete m_TempGeomNodes[i];
    }
    m_TempGeomNodes.clear();
}

CGlVboNode* CGlModel2D::FindGeomNode(const string& name)
{
    for (size_t i=0; i<m_GeomNodes.size(); ++i) {
        if (m_GeomNodes[i]->GetName() == name)
            return m_GeomNodes[i];
    }
    return NULL;
}

CGlVboNode* CGlModel2D::AddGeomNode(GLenum node_type, 
                                    const string& name,
                                    bool visible)
{
    CGlVboNode* n = FindGeomNode(name);
    if (n != NULL)
        return n;

    n = new CGlVboNode(node_type);
    n->SetName(name);
    n->SetVisible(visible);    

    m_GeomNodes.push_back(n);

    return n;
}

CGlVboNode* CGlModel2D::AddGeomNode(GLenum node_type,
                                    const string& name,
                                    size_t idx,
                                    bool visible)
{
    CGlVboNode* n = FindGeomNode(name);
    if (n != NULL)
        return n;

    n = new CGlVboNode(node_type);
    n->SetName(name);
    n->SetVisible(visible);

    if (idx >= m_GeomNodes.size())   
        m_GeomNodes.push_back(n);
    else
        m_GeomNodes.insert(m_GeomNodes.begin() + idx, n);

    return n;
}

CGlVboNode* CGlModel2D::AddGeomNode(const string& name,
                                    bool visible)
{
    CGlVboNode* n = FindGeomNode(name);
    if (n != NULL)
        return n;

    n = new CGlVboNode();
    n->SetName(name);
    n->SetVisible(visible);    

    m_GeomNodes.push_back(n);

    return n;
}

CGlVboNode* CGlModel2D::AddGeomNode(const string& name,
                                    size_t idx,
                                    bool visible)
{
    CGlVboNode* n = FindGeomNode(name);
    if (n != NULL)
        return n;

    n = new CGlVboNode();
    n->SetName(name);
    n->SetVisible(visible);

    if (idx >= m_GeomNodes.size())
        m_GeomNodes.push_back(n);
    else
        m_GeomNodes.insert(m_GeomNodes.begin() + idx, n);

    return n;
}


void CGlModel2D::ClearArrays()
{
    size_t i;

    for (i=0; i<m_GeomNodes.size(); ++i) {
        m_GeomNodes[i]->ClearPositions();
    }

    for (i=0; i<m_TempGeomNodes.size(); ++i) {       
        delete m_TempGeomNodes[i];
    }
    m_TempGeomNodes.clear();
}

void CGlModel2D::Render(CGlPane& pane)
{
    size_t i;

    for (i=0; i<m_GeomNodes.size(); ++i) {
        if (m_GeomNodes[i]->IsVisible() && !m_GeomNodes[i]->IsSkipped(eOpenGL20)) {
            glPushAttrib(GL_ALL_ATTRIB_BITS);
            if (m_GeomNodes[i]->GetState().GetScaleInvarient())
                m_GeomNodes[i]->GetState().ScaleFactor(pane.GetScale());
            m_GeomNodes[i]->Render();
            glPopAttrib();
        }
    }

    for (i=0; i<m_TempGeomNodes.size(); ++i) {
        if (m_TempGeomNodes[i]->IsVisible() && !m_TempGeomNodes[i]->IsSkipped(eOpenGL20)) {
            glPushAttrib(GL_ALL_ATTRIB_BITS);
            if (m_TempGeomNodes[i]->GetState().GetScaleInvarient())
                m_TempGeomNodes[i]->GetState().ScaleFactor(pane.GetScale());
            m_TempGeomNodes[i]->Render();
            glPopAttrib();
        }
    }
}


CIRef<IVboGeom> CGlModel2D::FindGeomObject(const string& name)
{
    for (size_t i=0; i<m_GeomObjects.size(); ++i) {
        if (m_GeomObjects[i]->GetName() == name)
            return m_GeomObjects[i];
    }

    return CIRef<IVboGeom>();
}

CIRef<IVboGeom> CGlModel2D::AddGeomObject(GLenum node_type, 
                                           const string& name)
{
    CIRef<IVboGeom> g = FindGeomObject(name);
    if (!g.IsNull())
        return g;

    g.Reset(CGlResMgr::Instance().CreateVboGeom(node_type, name));

    m_GeomObjects.push_back(g);

    return g;
}

CGlVboNode* CGlModel2D::AddTempGeomNode(const string& name,
                                        bool visible)
{
    CGlVboNode* n = new CGlVboNode();
    n->SetName(name);
    n->SetVisible(visible);    

    m_TempGeomNodes.push_back(n);

    return n;
}

CGlVboNode* CGlModel2D::AddTempGeomNode(GLenum node_type, 
                                        const string& name,
                                        bool visible)
{
    CGlVboNode* n = new CGlVboNode(node_type);
    n->SetName(name);
    n->SetVisible(visible);    

    m_TempGeomNodes.push_back(n);

    return n;
}

END_NCBI_SCOPE
