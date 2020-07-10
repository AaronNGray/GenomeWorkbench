#ifndef GUI_OPENGL___GL_RESOURCE_MANAGER__HPP
#define GUI_OPENGL___GL_RESOURCE_MANAGER__HPP

/*  $Id: glresmgr.hpp 43134 2019-05-20 18:13:11Z katargir $
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

#include <corelib/ncbiobj.hpp>
#include <gui/opengl.h>
#include <gui/gui.hpp>

#include <gui/opengl/globject.hpp>
#include <gui/opengl/glutils.hpp>
#include <gui/opengl/irender.hpp>

/** @addtogroup GUI_OPENGL
 *
 * @{
 */

BEGIN_NCBI_SCOPE

class IVboGeom;
class I3DTexture;
class I3DFrameBuffer;
class CImage;

class NCBI_GUIOPENGL_EXPORT CGlResMgr
{
public:
    /// Needs active OpenGL context when created.  Determines best api level.
    CGlResMgr();

    /// Destroys all gl objects it created.
    ~CGlResMgr();

    /// Deletes all objects.  Since object is a singleton, don't want to call
    /// dtors during program exit as this can fail
    void Clear();

    /// Find GL object (add type option?) TBD
    //CGlObject* FindObject(const string& name);

    /// Set api level by querying opengl
    void SetApiLevel();
    /// Set the api level directly (override ctor). Good for testing.
    void SetApiLevel(ERenderTarget level);
    /// Get api level
    ERenderTarget GetApiLevel() const { return m_ApiLevel; }

    /// Get geometry object for a node that renders using vertex buffers.
    IVboGeom* CreateVboGeom(GLenum drawMode, const string& name);
    IVboGeom* CreateNarrowTreeEdgeVboGeom(IVboGeom* edgeGeom);
    IVboGeom* CreateFillerPointVboGeom(IVboGeom* edgeGeom);
    IVboGeom* CreateTreeNodeVboGeom(size_t numSegments);
   

    ///
    /// Get/Set Renderer
    ///

    void AddRenderer(CIRef<IRender> rm);
    CIRef<IRender> RemoveRenderer(CIRef<IRender> rm);

    /// Returns first renderer in m_Renderers that renders to 'target'
    CIRef<IRender> GetRenderer(ERenderTarget target);
    
    /// Set current renderer (rm must already be in m_Renderers)
    void SetCurrentRenderer(CIRef<IRender> rm);
    /// Returns current renderer, or NULL
    CIRef<IRender> GetCurrentRenderer() {return CIRef<IRender>(m_CurrentRenderer);}

    /// Clear all current rendering objects
    void ClearRenderers();


    // Get single instance of resource manager (singleton).
    static CGlResMgr& Instance();
    /// return true if static instance has been created (via a call to Instance()
    static bool Initialized();

    I3DTexture*     CreateTexture(CNcbiIstream& istr, const string& tag);
    I3DTexture*     CreateTexture(CImage* image);
    I3DTexture*     Create1DRGBATexture(size_t width, float* data);
    I3DFrameBuffer* CreateFrameBuffer(size_t dim);

protected:   
    static CGlResMgr* m_StaticInstance;

    ERenderTarget m_ApiLevel;

    std::vector<CIRef<IRender> > m_Renderers;
    CIRef<IRender> m_CurrentRenderer;
};

END_NCBI_SCOPE


#endif // GUI_OPENGL___GL_RESOURCE_MANAGER__HPP
