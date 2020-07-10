#ifndef GUI_OPENGL___GL_STATE__HPP
#define GUI_OPENGL___GL_STATE__HPP

/*  $Id: glstate.hpp 44821 2020-03-23 16:08:14Z evgeniev $
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
#include <gui/utils/vect3.hpp>
#include <gui/utils/vect4.hpp>
#include <gui/utils/matrix3.hpp>
#include <gui/utils/matrix4.hpp>
#include <gui/utils/rgba_color.hpp>

#include <gui/opengl/gltypes.hpp>
#include <gui/opengl/globject.hpp>

/** @addtogroup GUI_OPENGL
 *
 * @{
 */

BEGIN_NCBI_SCOPE

class I3DTexture;


/// Different api levels based on information from OpenGL driver. Most
/// will be 20 (2.0), but machines > 5 years old or with improperly installed
/// drivers can be as low as 1.1.  Note that eOpenGL20 is used for 2.0 and above.
enum ERenderTarget { eOpenGL20, eOpenGL11, eMetal, eRenderPDF, eRenderDebug, eRenderSVG, eApiUndefined };

//////////////////////////////////////////////////////////////////////////////
/// IGlState
/// Interface for setting values in CGlState. Abstraction allows other classes
/// (CGlRender) to present OpenGL 1.1 style state setting, e.g.
///
/// class CGlRender : public IGlState ... {...};
/// 
/// CGlRender gl;
/// gl.Color3f(0.0f, 1.0f, 0.0f);
///
class NCBI_GUIOPENGL_EXPORT IGlState
{
public:
    /// Enumerators for non-enumerated state values (e.g. pdf-only state values)
    enum ELineJoinStyle { eMiteredJoin=0, eRoundedJoin, eBeveledJoin };
    enum ELineCapStyle { eButtCap=0, eRoundCap, eSquareCap };
    enum EPdfShadeStyle { eFlat, eGouraud };

public:
    IGlState() {}
    virtual ~IGlState() {}

    /// glEnable()
    virtual void Enable(GLenum glstate) = 0;
    /// glDisable()
    virtual void Disable(GLenum glstate) = 0;

    /// Allow 1 texture for now (no multi-texturing)
    virtual void SetTexture(I3DTexture* tex) = 0;

    /// Set line width for drawing: glLineWidth()
    virtual void LineWidth(GLfloat w) = 0;

    /// Set point size for drawing: glPointSize()
    virtual void PointSize(GLfloat s) = 0;

    /// Set shade model for default lighting: glShadeModel(GL_FLAT or GL_SMOOTH)
    virtual void ShadeModel(GLenum mode) = 0;

    /// Set clipping window: glScissor(x,y,width,height)
    virtual void Scissor(GLint x, GLint y,
                         GLsizei width, GLsizei height) = 0;
   
    /// Set current color (glColor{3,4}{f,d}{v,})    
    virtual void Color3f(GLfloat r, GLfloat g, GLfloat b) = 0;
    virtual void Color3fv(const GLfloat* v) = 0;
    virtual void Color4f(GLfloat r, GLfloat g, GLfloat b, GLfloat a) = 0;
    virtual void Color4fv(const GLfloat* v) = 0;
    virtual void Color3d(GLdouble r, GLdouble g, GLdouble b) = 0;
    virtual void Color3dv(const GLdouble* v) = 0;
    virtual void Color4d(GLdouble r, GLdouble g, GLdouble b, GLdouble a) = 0;
    virtual void Color4dv(const GLdouble* v) = 0;
    virtual void ColorC(const CRgbaColor& c) = 0;

    /// Set the color mask (glColorMask)
    virtual void ColorMask(GLboolean red, 
                   GLboolean green, 
                   GLboolean blue, 
                   GLboolean alpha) = 0;

    /// Set the polygon rasterization mode.  For the first parameter, GL_FRONT
    /// and GL_BACK are deprecated so better to use GL_FRONT_AND_BACK
    virtual void PolygonMode(GLenum face, GLenum mode) = 0;

    /// Set line stipple pattern: glLineStipple().  Deprecated in gl 3.2+
    virtual void LineStipple(GLint factor, GLushort pattern) = 0;

    /// Set polygon stipple pattern: glPolygonStipple().  Deprecated in gl 3.2+
    virtual void PolygonStipple(GLubyte* mask) = 0;


    ///
    /// Options to be used when GL_BLEND is enabled. 
    /// The options used at render time will be those most recently 
    /// given by SetBlendFunc() OR SetBlendFuncSeparate() prior
    /// to calling MakeCurrent().
    ///

    /// Options for glBlendFunc. Enable via Enable(GL_BLEND);
    virtual void BlendFunc(GLenum sfactor, GLenum dfactor) = 0;

    /// Options for glBlendFuncSeparate, also enabled via Enable(GL_BLEND).
    virtual void BlendFuncSeparate(GLenum srcRGB, GLenum dstRGB,
                                   GLenum srcAlpha, GLenum dstAlpha) = 0;
    /// glBlendColor() - Optional constant color for blending
    virtual void BlendColor(const CRgbaColor& c) = 0;


    ///
    /// Generic rendering options not specfically tied to OpenGL (or pdf..)
    ///

    /// If true, object size is in pixels and will not get bigger/smaller with zoom
    virtual void ScaleInvarient(bool b, CVect2<TModelUnit> scale = 
                                CVect2<TModelUnit>(TModelUnit(1), TModelUnit(1))) = 0;
 
    virtual void ScaleFactor(const CVect2<TModelUnit>& scale) = 0;



    ///
    /// PDF-specific rendering state.  Will not have any effect on OpenGL output
    /// Note that in PDF files, some or all of these options may be settable 
    /// pre-element (attributes), not just per-object (uniform)
    ///

    /// Set path joing style for lines (pdf-only)
    virtual void LineJoinStyle(ELineJoinStyle s) = 0;

    /// Set line cap ending style (pdf only)
    virtual void LineCapStyle(ELineCapStyle c) = 0;

    /// Set (override defualt) shading style for polygons
    virtual void PdfShadeStyle(EPdfShadeStyle s) = 0;

    /// Resets the state back to the default one
    virtual void Reset() = 0;
};

//////////////////////////////////////////////////////////////////////////////
/// CGlState
/// Class to encapsulate Rendering state so that a set of user-selected GL
/// state options can be saved and then enabled as needed, particularly before
/// the rendering of some object.
///
/// This class does not currently support the full range of GL options (but
/// those can be added as needed).  State variables that apply to other output
/// formats PDF can be added here as well.  When possible, states such as color
/// will apply to all output types.
///
class NCBI_GUIOPENGL_EXPORT CGlState : public CGlObject
                                     , public IGlState
{
public:
    CGlState();
    virtual ~CGlState() {}

    /// Set current options
    void MakeCurrent(ERenderTarget target);

    /// Return to default state (TBD)
    void SetDefaultGLState() {}
    /// Updates all state variables with current openGL state (TBD)
    void ReadState() {}
    /// Computes difference between current state (e.g. from last ReadState()
    /// and current OpenGL state (TBD)
    void StateDelta() {}

    /// This is for pushing and popping states - retain state variables
    /// not included in the mask (as per glPushAttrib)
    virtual void MergeStates(CGlState& s, GLbitfield mask);

    /// glEnable() all options in m_Enabled
    virtual void Enable(GLenum glstate);
    /// Return true if option is in m_Enabled list for this state
    bool IsEnabled(GLenum glstate) const;

    /// glDisable() all options in m_Disabled
    virtual void Disable(GLenum glstate);
    /// Return true if option is in m_Disabled list for this state
    bool IsDisabled(GLenum glstate) const;

    virtual void SetTexture(I3DTexture* tex);
    I3DTexture* GetTexture() { return m_Texture; }

    /// Set line width for drawing: glLineWidth()
    virtual void LineWidth(GLfloat w);
    GLfloat GetLineWidth() const { return m_LineWidth; }
    bool LineWidthSet() const { return m_LineWidthSet; }

    /// Set point size for drawing: glPointSize()
    virtual void PointSize(GLfloat s);
    GLfloat GetPointSize() const { return m_PointSize; }
    bool PointSizeSet() const { return m_PointSizeSet; }

    /// Set shade model for default lighting: glShadeModel(GL_FLAT or GL_SMOOTH)
    virtual void ShadeModel(GLenum mode);
    GLenum GetShadeModel() const { return m_ShadeModel; }
    bool ShadeModelSet() const { return m_ShadeModelSet; }

    /// Set clipping window: glScissor(x,y,width,height)
    virtual void Scissor(GLint x, GLint y,
                         GLsizei width, GLsizei height);
    TVPRect GetScissor() const { return m_ScissorRect; }
    bool ScissorSet() const { return m_ScissorSet; }

    /// Set current color (glColor{3,4}{f,d})
    virtual void Color3f(GLfloat r, GLfloat g, GLfloat b);
    virtual void Color3fv(const GLfloat* v);
    virtual void Color4f(GLfloat r, GLfloat g, GLfloat b, GLfloat a);
    virtual void Color4fv(const GLfloat* v);
    virtual void Color3d(GLdouble r, GLdouble g, GLdouble b);
    virtual void Color3dv(const GLdouble* v);
    virtual void Color4d(GLdouble r, GLdouble g, GLdouble b, GLdouble a);
    virtual void Color4dv(const GLdouble* v);
    virtual void ColorC(const CRgbaColor& c);
    CRgbaColor GetColor() const { return m_Color; }
    bool ColorSet() const { return m_ColorSet; }

    /// Set the color mask (glColorMask)
    virtual void ColorMask(GLboolean red, 
                           GLboolean green, 
                           GLboolean blue, 
                           GLboolean alpha);
    void GetColorMask(GLboolean& red, 
                      GLboolean& green, 
                      GLboolean& blue, 
                      GLboolean& alpha);
    bool ColorMaskSet() const { return m_ColorMaskSet; }

    /// Set the polygon rasterization mode.  For the first parameter, GL_FRONT
    /// and GL_BACK are deprecated so better to use GL_FRONT_AND_BACK
    virtual void PolygonMode(GLenum face, GLenum mode);
    GLenum GetPolygonMode() const { return m_PolygonMode; }
    bool PolygonModeSet() const { return m_PolygonModeSet; }

    /// Set line stipple pattern: glLineStipple().  Deprecated in gl 3.2+
    virtual void LineStipple(GLint factor, GLushort pattern);
    void GetLineStipple(GLint& factor, GLushort& pattern) const;
    bool LineStippleSet() const { return m_LineStippleSet; }

    /// Set polygon stipple pattern: glPolygonStipple().  Deprecated in gl 3.2+
    virtual void PolygonStipple(GLubyte* mask);
    const GLubyte* GetPolygonStipple() const { return m_PolygonStippleMask; }
    bool PolygonStippleSet() const { return m_PolygonStippleSet; }


    ///
    /// Options to be used when GL_BLEND is enabled. 
    /// The options used at render time will be those most recently 
    /// given by SetBlendFunc() OR SetBlendFuncSeparate() prior
    /// to calling MakeCurrent().
    ///

    /// Options for glBlendFunc. Enable via Enable(GL_BLEND);
    virtual void BlendFunc(GLenum sfactor, GLenum dfactor);
    GLenum GetSourceBlendFactor() const { return  m_SourceBlendFactor; }
    GLenum GetTargetBlendFactor() const { return m_TargetBlendFactor; }
    bool BlendFuncSet() const { return m_BlendFuncSet; }

    /// Options for glBlendEquation
    virtual void BlendEquation(GLenum mode);
    bool BlendEquationSet() const { return m_BlendEquationSet; }

    /// Options for glBlendFuncSeparate, also enabled via Enable(GL_BLEND).
    virtual void BlendFuncSeparate(GLenum srcRGB, GLenum dstRGB,
                                   GLenum srcAlpha, GLenum dstAlpha);
    /// glBlendColor() - Optional constant color for blending
    virtual void BlendColor(const CRgbaColor& c);



    ///
    /// Generic rendering options not specfically tied to OpenGL (or pdf..)
    ///

    /// If true, object size is in pixels and will not get bigger/smaller with zoom
    virtual void ScaleInvarient(bool b, CVect2<TModelUnit> scale = 
                        CVect2<TModelUnit>(TModelUnit(1), TModelUnit(1)));
    bool GetScaleInvarient() const { return m_ScaleInvarient; }
    bool ScaleInvarientSet() const { return m_ScaleInvarientSet; }
    /// After setting scale invarient rendering need to update the scale whenever
    /// projection matrix changes
    virtual void ScaleFactor(const CVect2<TModelUnit>& scale) { m_ScaleFactor = scale; }
    CVect2<TModelUnit> GetScaleFactor() const { return m_ScaleFactor; }



    ///
    /// PDF-specific rendering state.  Will not have any effect on OpenGL output
    /// Note that in PDF files, some or all of these options may be settable 
    /// per-element (attributes), not just per-object (uniform)
    ///

    /// Set path joing style for lines (pdf-only)
    virtual void LineJoinStyle(ELineJoinStyle s);
    ELineJoinStyle GetLineJoinStyle() const { return m_LineJoinStyle; }
    bool LineJoinStyleSet() const { return m_LineJoinStyleSet; }

    /// Set line cap ending style (pdf only)
    virtual void LineCapStyle(ELineCapStyle c);
    ELineCapStyle GetLineCapStyle() const { return m_LineCapStyle; }
    bool LineCapStyleSet() const { return m_LineCapStyleSet; }

    /// In (our) PDF files there are two kinds of polys, flat ones (move to,
    /// line to, fill) which can have any # of vertices and Gouraud-shaded
    /// ones (always tris).  Using flat tris (or quads) to build larger polys
    /// doesn't work well in pdf - you get cracks. So Gouraud is slower but no
    /// cracks and flat is faster but they should stand alone (default is gouraud)
    virtual void PdfShadeStyle(EPdfShadeStyle s);
    EPdfShadeStyle GetPdfShadeStyle() const { return m_PdfShadeStyle; }
    bool PdfShadeStyleSet() const { return m_PdfShadeStyleSet; }   
    
    /// Resets the state back to the default one
    virtual void Reset();

protected:
    /// helper function for MergeState that determines if a flag was masked
    bool x_MergeFlag(GLenum e, GLbitfield mask);

    /// Line width (glLineWidth())
    GLfloat m_LineWidth;
    bool m_LineWidthSet;

    /// Joining style for adjacent lines (effects pdf only)
    ELineJoinStyle m_LineJoinStyle;
    bool m_LineJoinStyleSet;

    /// Cap (line terminator) style (effects pdf only)
    ELineCapStyle m_LineCapStyle;
    bool m_LineCapStyleSet;

    /// Pdf shading style (effects pdf only)
    EPdfShadeStyle m_PdfShadeStyle;
    bool m_PdfShadeStyleSet;

    /// Point size (glPointSize())
    GLfloat m_PointSize;
    bool m_PointSizeSet;

    /// Shade model (GL_FLAT or GL_SMOOTH)
    GLenum m_ShadeModel;
    bool m_ShadeModelSet;

    /// Scissor rectangle
    TVPRect m_ScissorRect;
    bool m_ScissorSet;
    
    /// Current color (glColor())
    CRgbaColor m_Color;
    bool m_ColorSet;

    /// ColorMask
    GLboolean m_ColorMaskRed;
    GLboolean m_ColorMaskGreen;
    GLboolean m_ColorMaskBlue;
    GLboolean m_ColorMaskAlpha;
    bool m_ColorMaskSet;

    /// parameters for glPolygonMode.  We only support one entry - can't specify
    /// different modes for GL_FRONT and GL_BACK (should use GL_FRONT_AND_BACK)
    GLenum m_PolygonModeFace;
    GLenum m_PolygonMode;
    bool m_PolygonModeSet;

    /// glLineStipple(factor, pattern)  Deprecated in OpenGL 3+
    GLint m_LineStippleFactor;
    GLushort m_LineStipplePattern;
    bool m_LineStippleSet;

    GLubyte m_PolygonStippleMask[32*32];
    bool m_PolygonStippleSet;

    /// Blend factors for glBlendFunc()
    GLenum m_SourceBlendFactor;
    GLenum m_TargetBlendFactor;
    bool m_BlendFuncSet;

    /// Blend equatnion for glBlendEquation()
    GLenum m_BlendEquation;
    bool m_BlendEquationSet;

    /// Blend factors for glBlendFuncSeparate
    GLenum m_SourceRgbFactor;
    GLenum m_TargetRgbFactor;
    GLenum m_SourceAlpahFactor;
    GLenum m_TargetAlphaFactor;
    bool m_BlendFuncSeparateSet;
    
    /// Optional color to be used in blending (glBlendColor())
    CRgbaColor m_BlendColor;
    bool m_BlendColorSet;

    /// Set of all options to be enabled (glEnable())
    vector<GLenum> m_Enabled;
    /// Set of all options to be disabled (glDisable())
    vector<GLenum> m_Disabled;

    /// If true, object dimension are in pixels/points (not zoomable)
    bool m_ScaleInvarient;
    bool m_ScaleInvarientSet;
    /// Only needed for scale-invariant rendering (from CGlPane::GetScale()
    CVect2<TModelUnit> m_ScaleFactor;

    /// For now only allow 1 texture per object (no multi-texturing)
    // This class does not own the texture (no deleting it)
    I3DTexture* m_Texture;

    /// Set to false in MakeCurrent and true when any parms are updated
    bool m_Dirty;
};


END_NCBI_SCOPE

/* @} */

#endif  // GUI_OPENGL___GL_STATE__HPP
