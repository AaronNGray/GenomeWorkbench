/*  $Id: glresmgr.cpp 43585 2019-08-05 16:38:10Z katargir $
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
 *    OpenGL resource manager implementation
 */


#include <ncbi_pch.hpp>

#include <ncbiconf.h>

#include <gui/opengl/glresmgr.hpp>
#include <gui/opengl/glutils.hpp>
#include "glvbogeom.hpp"
#include <gui/opengl/glrender.hpp>

#include <gui/opengl/gltexture.hpp>
#include <gui/opengl/glframebuffer.hpp>

#ifdef USE_METAL
    #include "mtlrender.hpp"
    #include "mtlvbogeom.hpp"
    #include "mtltexture.hpp"
    #include "mtlframebuffer.hpp"
    #include "mtltreenodevbo.hpp"
#else
    #include "gltreenodevbo.hpp"
#endif // USE_METAL

BEGIN_NCBI_SCOPE

CGlResMgr* CGlResMgr::m_StaticInstance = NULL;

CGlResMgr& CGlResMgr::Instance()
{
    //static CGlResMgr instance;
    //return instance;
    if (m_StaticInstance == NULL) {
        // Create the instance.  This will not succeed if the OpenGL context
        // has not yet been initialized.
        try {
            m_StaticInstance = new CGlResMgr();
        }
        catch(...) {
            // Make sure instance stays NULL, and re-throw
            m_StaticInstance = NULL;
            NCBI_THROW(CException, eUnknown,
                "Attempt to access OpenGL prior to context creation");
        }
    }

    return *m_StaticInstance;
}

bool CGlResMgr::Initialized()
{
    return (m_StaticInstance!=NULL);
}

CGlResMgr::CGlResMgr()
{
    SetApiLevel();

    CIRef<IRender>  rm;

    if (m_ApiLevel == eOpenGL20) {
        rm.Reset(new CGlRender20());
        AddRenderer(rm);
        SetCurrentRenderer(rm);
    }
    else if (m_ApiLevel == eOpenGL11) {
        rm.Reset(new CGlRender11());
        AddRenderer(rm);
        SetCurrentRenderer(rm);
    }
#ifdef USE_METAL
    else if (m_ApiLevel == eMetal) {
        rm.Reset(new CMtlRender());
        AddRenderer(rm);
        SetCurrentRenderer(rm);
    }
#endif // USE_METAL
    else if (m_ApiLevel == eRenderDebug) {
        rm.Reset(new CGlRenderDebug());
        AddRenderer(rm);
        SetCurrentRenderer(rm);
    }
    else {
        _TRACE("Error - valid OpenGL version not detected.");
    }
}

CGlResMgr::~CGlResMgr()
{
}

void CGlResMgr::Clear()
{
}
    
void CGlResMgr::SetApiLevel()
{
#ifdef USE_METAL
    m_ApiLevel = eMetal;
    return;
#else
    const char *v = (const char*)glGetString(GL_VERSION);

    // If OpenGL is not available, NULL will be returned.  Throw an exception
    if (v==NULL) {
        NCBI_THROW(CException, eUnknown,
            "Attempt to access OpenGL prior to context creation");
    }

    string version = v;
    vector<string> arr;

    if (version != "")
        NStr::Split(version, ". \t", arr);

    bool version_two = false;

    // Check major version
    int major_version = 0;
    if (arr.size() >= 1) {       
        try {
            major_version = NStr::StringToInt(arr[0]);
            if (major_version > 1) {
                version_two = true;
            }
        }
        catch (CException& ) {
            //didn't parse as an int...
            m_ApiLevel = eApiUndefined;
        }
    }
    else  {
         m_ApiLevel = eApiUndefined;
    }

    // Can check minor version too, if needed:
    /*
    if (arr.size() > 1 && major_version == 1) {
        try {
            int minor_version = NStr::StringToInt(arr[1]);
            if (minor_version > 4) {
                version_two = true;
            }
        }
        catch (CException&) {
            //minor version didn't parse as an int
        }
    }
    */

    if (version_two)
        m_ApiLevel = eOpenGL20;
    else
        m_ApiLevel = eOpenGL11;
#endif // USE_METAL
}
void CGlResMgr::SetApiLevel(ERenderTarget level)
{
    m_ApiLevel = level;
}

IVboGeom* CGlResMgr::CreateVboGeom(GLenum drawMode, const string& name)
{
#ifdef USE_METAL
    if (drawMode == GL_LINES)
        return new CMtlVboGeomLines(name);

    return new CMtlVboGeom(drawMode, name);
#else
    if (m_ApiLevel == eOpenGL20)
        return new CGlVboGeom20(drawMode, name);
    else
        return new CGlVboGeom11(drawMode, name);
#endif // USE_METAL
}

IVboGeom* CGlResMgr::CreateNarrowTreeEdgeVboGeom(IVboGeom* edgeGeom)
{
#ifdef USE_METAL
    return new CMtlNarrowTreeEdgeVboGeom(edgeGeom);
#else
    if (m_ApiLevel == eOpenGL20)
        return new CNarrowTreeEdgeVboGeom20(edgeGeom);
    else
        return new CNarrowTreeEdgeVboGeom11(edgeGeom);
#endif // USE_METAL
}

IVboGeom* CGlResMgr::CreateFillerPointVboGeom(IVboGeom* edgeGeom)
{
#ifdef USE_METAL
    return new CMtlFillerPointVboGeom(edgeGeom);
#else
    if (m_ApiLevel == eOpenGL20)
        return new CFillerPointVboGeom20(edgeGeom);
    else
        return new CFillerPointVboGeom11(edgeGeom);
#endif // USE_METAL
}

IVboGeom* CGlResMgr::CreateTreeNodeVboGeom(size_t numSegments)
{
#ifdef USE_METAL
    return new CMtlTreeNodeVbo(numSegments);
#else
    if (m_ApiLevel == eOpenGL20)
        return new CGlTreeNodeVbo20(numSegments);
    else
        return new CGlTreeNodeVbo11(numSegments);
#endif // USE_METAL
}

void CGlResMgr::AddRenderer(CIRef<IRender> rm)
{
    vector<CIRef<IRender> >::iterator iter;
    for (iter = m_Renderers.begin(); iter != m_Renderers.end(); ++iter) {
        if ((*iter)->GetApi() == rm->GetApi()) {
            break;
        }
    }

    if (iter == m_Renderers.end())
        m_Renderers.push_back(rm);
}

CIRef<IRender> CGlResMgr::RemoveRenderer(CIRef<IRender> rm)
{
    vector<CIRef<IRender> >::iterator iter;
    iter = std::find(m_Renderers.begin(), m_Renderers.end(), rm);
    
    if (iter != m_Renderers.end()) {
        m_Renderers.erase(iter);
        return *iter;
    }

    return CIRef<IRender>();
}
    
CIRef<IRender> CGlResMgr::GetRenderer(ERenderTarget target)
{
    vector<CIRef<IRender> >::iterator iter;

    for (iter=m_Renderers.begin(); iter!=m_Renderers.end(); ++iter) {
        if ((**iter).GetApi() == target)
            return *iter;
    }

    return CIRef<IRender>();
}

void CGlResMgr::SetCurrentRenderer(CIRef<IRender> rm)
{
    vector<CIRef<IRender> >::iterator iter;
    iter = std::find(m_Renderers.begin(), m_Renderers.end(), rm);
    
    // Set current rendermanager to NULL if not found (so caller will know update
    // didn't work and can fix).
    if (iter != m_Renderers.end())
        m_CurrentRenderer = rm;
    else
        m_CurrentRenderer.ReleaseOrNull();
}


void CGlResMgr::ClearRenderers()
{
    m_Renderers.clear();

    m_CurrentRenderer.Reset();
}

I3DTexture* CGlResMgr::CreateTexture(CNcbiIstream& istr, const string& tag)
{
#ifdef USE_METAL
    return new CMtlTexture(istr, tag);
#else
    return new CGlTexture(istr, tag);
#endif // USE_METAL
}

I3DTexture* CGlResMgr::CreateTexture(CImage* image)
{
#ifdef USE_METAL
    return new CMtlTexture(image);
#else
    return new CGlTexture(image);
#endif // USE_METAL
}

I3DTexture* CGlResMgr::Create1DRGBATexture(size_t width, float* data)
{
#ifdef USE_METAL
    CRef<CMtlTexture> tex(new CMtlTexture());
    tex->Load1DRGBA(width, data);
    return tex.Release();
#else
    CRef<CGlTexture> tex(new CGlTexture());
    tex->Load1DRGBA(width, data);
    return tex.Release();
#endif // USE_METAL
}

I3DFrameBuffer* CGlResMgr::CreateFrameBuffer(size_t dim)
{
#ifdef USE_METAL
    return new CMtlFrameBuffer(dim);
#else
    return new CGLFrameBuffer(dim);
#endif // USE_METAL
}

END_NCBI_SCOPE
