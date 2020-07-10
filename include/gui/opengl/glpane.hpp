#ifndef GUI_GRAPH___GLPANE__HPP
#define GUI_GRAPH___GLPANE__HPP

/*  $Id: glpane.hpp 38346 2017-04-27 13:24:57Z falkrb $
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
 * Authors:  Andrey Yazhuk
 *
 * File Description:
 *
 */

#include <corelib/ncbistd.hpp>
#include <gui/gui.hpp>
#include <gui/opengl/gltypes.hpp>
#include <gui/utils/vect2.hpp>

/** @addtogroup GUI_OPENGL
 *
 * @{
 */

BEGIN_NCBI_SCOPE

enum EScaleType {
    eDec,
    eLog10,
    eLog2,
    eLn
};

////////////////////////////////////////////////////////////////////////////////
/// class CGlPane

// CGlPane represents a rectangular area used for drawing graphs and decorations
// (such as grid, axes, labels etc.) CGlPane provides means for using OpenGL
// projections in order to draw graphs in coordinates of their model space.

class NCBI_GUIOPENGL_EXPORT CGlPane
{
public:
    enum EProjectionMode {
        eNone = -1, // closed or invalid
        eOrtho, // orthographic projection, drawing in model space coords
        ePixels // orthographic projection, drawing in pixels
    };
    /// Specifies what point should be preserved during various coordinate
    /// space adjustments.
    enum EOriginType {
        eOriginLeft,
        eOriginRight,
        eOriginBottom,
        eOriginTop,
        eOriginCenter
    };

    /// EProjMatrixPolicy controls CGlPane behavior in respect to obtaining
    /// projection matrices for future use in Project..(), UnProject...()
    /// functions.
    enum EProjMatrixPolicy
    {
        eNeverUpdate,
        eAlwaysUpdate
    };

    /// EAdjustmentPolicy specifies how visible area should be adjusted if
    /// some of the prjection parameters change
    enum EAdjustmentPolicy
    {
        fShiftToLimits = 0x1,
        fScaleToLimits = 0x2,
        fAdjustAll = fShiftToLimits | fScaleToLimits
    };

    /// EZoomOptions flags control behavior of Zoom operations.
    enum EZoomOptions
    {
        fZoomX = 0x1,   // zoom by X coordinate
        fZoomY = 0x2,   // zoom by Y coordinate
        fForce = 0x4,   // overrides enable/disable settings in the pane
        fZoomXY = fZoomX | fZoomY
    };

public:
    CGlPane(EProjMatrixPolicy policy = eAlwaysUpdate);
    CGlPane(const CGlPane& pane);
    virtual ~CGlPane();

    // Viewport manipulation
    void    SetViewport(const TVPRect& R);
    TVPRect&        GetViewport(void);
    const TVPRect&  GetViewport(void) const;

    // Model Space manipulation
    void    SetModelLimitsRect(const TModelRect& R);
    TModelRect&       GetModelLimitsRect(void);
    const TModelRect& GetModelLimitsRect(void) const;

    // Ortho projection - visible ares
    void    SetVisibleRect(const TModelRect& R);
    TModelRect&         GetVisibleRect(void);
    const TModelRect&   GetVisibleRect(void) const;

    /// set clipping rect that will be used by glScissor, NULL for reset
    void    SetClipRect(const TVPRect* rc_clip = NULL);

    /// returns clipping rect or NULL (if there is no clipping)
    const TVPRect*    GetClipRect() const;

    /// Offset is used as a workaround for OpenGL precision problems emerging
    /// when  size of visible range is small in comparision to coordinates in
    /// this range.

    void    EnableOffset(bool b_en = true);
    TModelUnit  GetOffsetX()    const;
    TModelUnit  GetOffsetY()    const;
    TModelPoint GetOffset() const;

    bool    Open(EProjectionMode Mode);
    bool    OpenOrtho();
    bool    OpenPixels();
    void    Close(void);
    EProjectionMode GetProjMode(void) const;

    // scale Model space units per pixel
    TModelUnit  GetScaleX(void) const;
    TModelUnit  GetScaleY(void) const;
    CVect2<TModelUnit> GetScale() const;

    void    SetMinScaleX(TModelUnit scale);
    void    SetMinScaleY(TModelUnit scale);
    TModelUnit    GetMinScaleX(void) const;
    TModelUnit    GetMinScaleY(void) const;

    TModelUnit    GetZoomAllScaleX(void) const;
    TModelUnit    GetZoomAllScaleY(void) const;

    // in Proportional mode CGlPane will keep horz. and vert. scales equal
    void    SetProportionalMode(bool set);
    bool    IsProportionalMode(void) const;

    void    SetOriginType(EOriginType type_x, EOriginType type_y);
    void    SetAdjustmentPolicy(int adjust_x, int adjust_y);
    void    SetAdjustToLimits(bool adjust_x, bool adjust_y);

    // Zoom functions
    void    EnableZoom(bool en_x, bool en_y);
    bool    IsZoomInAvaiable(void);
    bool    IsZoomOutAvaiable(void);

    TModelUnit  GetZoomFactor(void) const;
    void    SetZoomFactor(TModelUnit factor = 2.0);

    void    ZoomAll(int options = fZoomXY);

    void    ZoomInCenter(int options = fZoomXY);
    void    ZoomIn(TModelUnit x, TModelUnit y, int options = fZoomXY);
    void    ZoomIn(TModelPoint p, int options = fZoomXY);

    void    ZoomOutCenter(int options = fZoomXY);
    void    ZoomOut(TModelUnit x, TModelUnit y, int options = fZoomXY);
    void    ZoomOut(TModelPoint p, int options = fZoomXY);

    void    ZoomPoint(TModelUnit x, TModelUnit y, TModelUnit factor, int options = fZoomXY);
    void    ZoomPoint(TModelPoint p, TModelUnit factor, int options = fZoomXY);

    void    ZoomRect(const TModelRect& r);

    // Zooms so that p_center becomes a center of visible area, the size of
    // visible area is calculated based on given scales
    void    SetScale(TModelUnit scale_x, TModelUnit scale_y, TModelPoint p_center);
    void    SetScaleRefPoint(TModelUnit scale_x, TModelUnit scale_y,
                             TModelPoint p_ref);
    // this function preserves current center point
    void    SetScale(TModelUnit scale_x, TModelUnit scale_y);
	
	// Use orthographic projection with the exact size of the viewport
	void	SetExactOrthoProjection(bool exact_projection);

    // Scroll functions
    bool    NeedsScrollX(void) const;
    bool    NeedsScrollY(void) const;
    void    Scroll(TModelUnit dx, TModelUnit dy);

    // Set the visible rect to the same proporitional position as vr is to mr
    void    SetProportional(TModelRect& vr, TModelRect& mr);
    // Scoll visible rect so that position 'pos' is pctx (pcts are 0..1) distance
    // from left of visible rect and pos.y is pcty distance from bottom of vis. rect
    void    ScrollTo(const CVect2<TModelUnit>& pos, TModelUnit pctx, TModelUnit pcty);

    void    AdjustToLimits();

    // Coordinate transformations - now only for Ortho mode
    TVPUnit ProjectX(TModelUnit m_x) const;
    TVPUnit ProjectY(TModelUnit m_y) const;
    TVPPoint    Project(TModelUnit m_x, TModelUnit m_y) const;

    TModelUnit UnProjectX(TVPUnit m_x) const;
    TModelUnit UnProjectY(TVPUnit m_y) const;
    TModelPoint UnProject(TVPUnit m_x, TVPUnit m_y) const;

    TModelUnit  UnProjectWidth(TVPUnit vp_w) const;
    TModelUnit  UnProjectHeight(TVPUnit vp_h) const;

    /// Project in a manner that supports rendering to pdf as well (viewport
    /// may be larger than max. opengl size and scaling factor corresponding
    /// to 'UserUnit' in pdf file may apply)
    CVect2<TModelUnit> ProjectEx(CVect2<TModelUnit> pt);
    CVect2<TModelUnit> UnProjectEx(const CVect2<TModelUnit>& pt);

    /// Set/get output scaler.  Valid values are 1 to 75000 with 1 
    /// being the default and not having an effect on projection.
    /// Currently only used for pdf output (see UserUnit in pdf spec)
    void SetOutputUnitScaler(TModelUnit u) { m_OutputUnitScaler = u; }
    TModelUnit GetOutputUnitScaler() const { return m_OutputUnitScaler; }

    /// Return the scaled pixel distance.  Only important for pdf rendering
    TModelUnit GetPixelDist(TModelUnit d) const;

protected:
    bool    x_OpenOrtho();
    bool    x_OpenPixels();
    void    x_Open_SetViewport();

    void    x_UpdateProjectMatrices(void);

    // functions enforcing constraints in the display validity
    void    x_CorrectScale(TModelPoint p_center);
    // shifts visible area so that it is located within limits
    void    x_ShiftToFitLimits(void);
    void    x_ScaleToFitLimits(void);
    void    x_AdjustVisibleRect(void);

    static TModelUnit  s_GetShift(TModelUnit low_shift, TModelUnit high_shift, bool neg_range, EOriginType type);

    void    x_AssertNotOpen(void) const;
    void    x_AssertOrtho(void) const;

protected:
    TVPRect m_rcVP;
    TModelRect  m_rcLimits;
    TModelRect  m_rcVisible;
    TModelRect  m_rcOrigVisible; // used in ProportionalMode
    const TVPRect *m_prcClip;

    EProjectionMode     m_Mode;
    EProjMatrixPolicy   m_MatrixPolicy;
    int   m_AdjustX;
    int   m_AdjustY;
    bool  m_bEnableOffset;
	// Use orthographic projection with the exact size of the viewport
	bool  m_bExactOrthoProjection;

    bool    m_bZoomEnX;
    bool    m_bZoomEnY;
    TModelUnit  m_ZoomFactor;

    bool    m_bProportionalMode;
    EOriginType     m_TypeX;
    EOriginType     m_TypeY;

    TModelUnit  m_MinScaleX;
    TModelUnit  m_MinScaleY;

    // Scaler used for pdf output when it exceeds maximum pdf size (14000).  Allows
    // bigger pdfs but with lowered resolution.
    TModelUnit m_OutputUnitScaler;

    // matrices for using with gluProject gluUnProject
    GLint   m_mxVP[4];
    GLdouble m_mxProjection[16];
    GLdouble m_mxModelView[16];

};



class CGlPaneGuard
{
public:
    CGlPaneGuard(CGlPane& pane, CGlPane::EProjectionMode mode)
        : m_Pane(pane)
    {
        m_OrigMode = m_Pane.GetProjMode();
        if (m_OrigMode != CGlPane::eNone) {
            m_Pane.Close();
        }
        m_Pane.Open(mode);
    }

    ~CGlPaneGuard()
    {
        m_Pane.Close();
        if (m_OrigMode != CGlPane::eNone) {
            m_Pane.Open(m_OrigMode);
        }
    }

private:
    CGlPane& m_Pane;
    CGlPane::EProjectionMode m_OrigMode;

    // forbidden
    CGlPaneGuard(const CGlPaneGuard&);
    CGlPaneGuard& operator=(const CGlPaneGuard&);
};



inline TVPRect&    CGlPane::GetViewport(void)
{
    return m_rcVP;
}

inline const TVPRect&  CGlPane::GetViewport(void) const
{
    return m_rcVP;
}

inline void    CGlPane::SetModelLimitsRect(const TModelRect& R)
{
    m_rcLimits = m_rcOrigVisible = R;
}

inline TModelRect&  CGlPane::GetModelLimitsRect(void)
{
    return m_rcLimits;
}

inline const TModelRect&    CGlPane::GetModelLimitsRect(void) const
{
    return m_rcLimits;
}

inline TModelRect& CGlPane::GetVisibleRect(void)
{
    return m_rcVisible;
}

inline const TModelRect&   CGlPane::GetVisibleRect(void) const
{
    return m_rcVisible;
}

inline void    CGlPane::SetProportionalMode(bool set)
{
    m_bProportionalMode = set;
}

inline bool    CGlPane::IsProportionalMode(void) const
{
    return m_bProportionalMode;
}

inline void    CGlPane::SetOriginType(EOriginType type_x, EOriginType type_y)
{
    m_TypeX = type_x;
    m_TypeY = type_y;
}

inline void    CGlPane::SetAdjustmentPolicy(int adjust_x, int adjust_y)
{
    m_AdjustX = adjust_x;
    m_AdjustY = adjust_y;
}

inline void    CGlPane::SetAdjustToLimits(bool adjust_x, bool adjust_y)
{
    m_AdjustX = adjust_x ? fAdjustAll : 0;
    m_AdjustY = adjust_y ? fAdjustAll : 0;
}

inline void     CGlPane::SetClipRect(const TVPRect* rc_clip)
{
    m_prcClip = rc_clip;
}

inline const TVPRect* CGlPane::GetClipRect() const
{
    return m_prcClip;
}

inline void    CGlPane::EnableOffset(bool b_en)
{
    m_bEnableOffset = b_en;
}

inline TModelUnit  CGlPane::GetOffsetX()    const
{
    return m_bEnableOffset ? m_rcVisible.Left() : 0;
}

inline TModelUnit  CGlPane::GetOffsetY()    const
{
    return m_bEnableOffset ? m_rcVisible.Bottom() : 0;
}

inline TModelPoint CGlPane::GetOffset() const
{
    if(m_bEnableOffset)
            return TModelPoint(m_rcVisible.Left(), m_rcVisible.Bottom());
    else    return TModelPoint(0, 0);
}

inline bool    CGlPane::OpenOrtho()
{
    return Open(eOrtho);
}

inline bool    CGlPane::OpenPixels()
{
    return Open(ePixels);
}

inline CGlPane::EProjectionMode  CGlPane::GetProjMode(void) const
{
    return m_Mode;
}

inline void    CGlPane::SetMinScaleX(TModelUnit scale)
{
    m_MinScaleX = scale;
}

inline void    CGlPane::SetMinScaleY(TModelUnit scale)
{
    m_MinScaleY = scale;
}

inline TModelUnit    CGlPane::GetMinScaleX(void) const
{
    return m_MinScaleX;
}

inline TModelUnit    CGlPane::GetMinScaleY(void) const
{
    return m_MinScaleY;
}

inline void    CGlPane::EnableZoom(bool en_x, bool en_y)
{
    m_bZoomEnX = en_x;
    m_bZoomEnY = en_y;
}

inline TModelUnit  CGlPane::GetZoomFactor(void) const
{
    return m_ZoomFactor;
}

inline void    CGlPane::SetZoomFactor(TModelUnit factor)
{
    _ASSERT(factor > 0);
    m_ZoomFactor = factor;
}

inline void    CGlPane::ZoomIn(TModelPoint p, int options)
{
    ZoomPoint(p.X(), p.Y(), m_ZoomFactor, options);
}

inline void    CGlPane::ZoomIn(TModelUnit x, TModelUnit y, int options)
{
    ZoomPoint(x, y, m_ZoomFactor, options);
}

inline void    CGlPane::ZoomOut(TModelPoint p, int options)
{
    ZoomPoint(p.X(), p.Y(), 1 / m_ZoomFactor, options);
}

inline void    CGlPane::ZoomOut(TModelUnit x, TModelUnit y, int options)
{
    ZoomPoint(x, y, 1 / m_ZoomFactor, options);
}

inline void    CGlPane::ZoomPoint(TModelPoint p, TModelUnit factor, int options)
{
    ZoomPoint(p.X(), p.Y(), factor, options);
}

inline void    CGlPane::AdjustToLimits(void)
{
    x_ShiftToFitLimits();
}

inline void    CGlPane::SetExactOrthoProjection(bool exact_projection)
{
    m_bExactOrthoProjection = exact_projection;
}

END_NCBI_SCOPE

/* @} */

#endif
