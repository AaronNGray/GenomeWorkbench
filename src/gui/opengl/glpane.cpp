/*  $Id: glpane.cpp 42137 2018-12-27 16:47:23Z katargir $
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

#include <ncbi_pch.hpp>
#include <gui/opengl/glpane.hpp>
#include <gui/opengl/irender.hpp>

#include <math.h>

BEGIN_NCBI_SCOPE

///////////////////////////////////////////////////////////////////////////////
/// CGlPane

CGlPane::CGlPane(EProjMatrixPolicy policy)
: m_prcClip(NULL),
  m_Mode(eNone),
  m_MatrixPolicy(policy),
  m_AdjustX(fAdjustAll),
  m_AdjustY(fAdjustAll),
  m_bEnableOffset(false),
  m_bExactOrthoProjection(false),
  m_bZoomEnX(true),
  m_bZoomEnY(true),
  m_ZoomFactor(2.0),
  m_bProportionalMode(false),
  m_TypeX(eOriginLeft), m_TypeY(eOriginBottom),
  m_MinScaleX(0), m_MinScaleY(0),
  m_OutputUnitScaler(1.0)
{
}


CGlPane::CGlPane(const CGlPane& pane)
: m_rcVP(pane.m_rcVP),
  m_rcLimits(pane.m_rcLimits),
  m_rcVisible(pane.m_rcVisible),
  m_rcOrigVisible(pane.m_rcOrigVisible),
  m_prcClip(pane.m_prcClip),
  m_Mode(eNone), // closed by default
  m_MatrixPolicy(pane.m_MatrixPolicy),
  m_AdjustX(pane.m_AdjustX),
  m_AdjustY(pane.m_AdjustY),
  m_bEnableOffset(pane.m_bEnableOffset),
  m_bExactOrthoProjection(pane.m_bExactOrthoProjection),
  m_bZoomEnX(pane.m_bZoomEnX),
  m_bZoomEnY(pane.m_bZoomEnY),
  m_ZoomFactor(pane.m_ZoomFactor),
  m_bProportionalMode(pane.m_bProportionalMode),
  m_TypeX(pane.m_TypeX),
  m_TypeY(pane.m_TypeY),
  m_MinScaleX(pane.m_MinScaleX),
  m_MinScaleY(pane.m_MinScaleY),
  m_OutputUnitScaler(pane.m_OutputUnitScaler)
{
    memcpy(m_mxVP, pane.m_mxVP, sizeof(GLint) * 4);
    memcpy(m_mxProjection, pane.m_mxProjection, sizeof(GLdouble) * 16);
    memcpy(m_mxModelView, pane.m_mxModelView, sizeof(GLdouble) * 16);
}


CGlPane::~CGlPane()
{
   // _ASSERT(m_Mode == eNone); // should be closed first
}

void    CGlPane::SetViewport(const TVPRect& r)
{
    m_rcVP = r;
    if (m_rcVP.Width() == 0) {
        m_rcVP.SetRight(m_rcVP.Left());
    }
    if (m_rcVP.Height() == 0) {
        m_rcVP.SetTop(m_rcVP.Bottom());
    }

    // applying constraints
    x_CorrectScale(m_rcOrigVisible.CenterPoint());
    x_ScaleToFitLimits();
    x_ShiftToFitLimits();
    x_AdjustVisibleRect();
}

void    CGlPane::SetVisibleRect(const TModelRect& r)
{
    m_rcVisible = m_rcOrigVisible = r;
}

TModelUnit  CGlPane::GetScaleX(void) const
{
    return m_rcVisible.Width() / m_rcVP.Width();
}

TModelUnit  CGlPane::GetScaleY(void) const
{
    return m_rcVisible.Height() / m_rcVP.Height();
}

CVect2<TModelUnit> CGlPane::GetScale() const
{
    return CVect2<TModelUnit>(GetScaleX(), GetScaleY());
}

TModelUnit    CGlPane::GetZoomAllScaleX(void) const
{
    return m_rcLimits.Width() / m_rcVP.Width();
}

TModelUnit    CGlPane::GetZoomAllScaleY(void) const
{
    return m_rcLimits.Height() / m_rcVP.Height();
}

inline void    CGlPane::x_AssertNotOpen(void) const
{
    _ASSERT(GetProjMode() == eNone);
}

inline void    CGlPane::x_AssertOrtho(void) const
{
    _ASSERT(GetProjMode() == eOrtho);
}

bool    CGlPane::Open(EProjectionMode mode)
{
    _ASSERT(mode != eNone);
    _ASSERT(m_Mode == eNone); //should not be already open

    bool ok = false;
    if (mode != m_Mode) {
        switch (mode) {
        case eOrtho: {
            ok = x_OpenOrtho();
            break;
        }
        case ePixels: {
            ok = x_OpenPixels();
            break;
        }
        default: _ASSERT(false); //other modes are unsupported
        } //switch
    }
    if (ok) {
        m_Mode = mode;
    }
    return ok;
}

void    CGlPane::Close(void)
{

    _ASSERT(m_Mode != eNone); // should be open
    if(m_Mode != eNone)
    {
        IRender& gl = GetGl();

        gl.MatrixMode(GL_PROJECTION);       
        gl.Disable(GL_SCISSOR_TEST);
        m_Mode = eNone;
    }
}

inline void    CGlPane::x_Open_SetViewport()
{
    _ASSERT(m_rcVP.Width()  &&  m_rcVP.Height());

    IRender& gl = GetGl();
    gl.Viewport(m_rcVP.Left(), m_rcVP.Bottom(), m_rcVP.Width(), m_rcVP.Height());

    // setup clipping
    gl.Enable(GL_SCISSOR_TEST);
    TVPRect rc_cl(m_rcVP);
    if(m_prcClip) { //additional clip rect has been provided
        rc_cl.IntersectWith(*m_prcClip);
    }
    gl.Scissor(rc_cl.Left(), rc_cl.Bottom(), rc_cl.Width(), rc_cl.Height());
}

bool    CGlPane::x_OpenOrtho()
{
    x_Open_SetViewport();
    
    IRender& gl = GetGl();

    gl.MatrixMode(GL_PROJECTION);   
    gl.LoadIdentity();

    TModelUnit left, right, top, bottom;

    if(m_bEnableOffset) {
        left = bottom = 0.0;
        right = m_rcVisible.Width();
        top = m_rcVisible.Height();
    } else {
        left = m_rcVisible.Left();
        right = m_rcVisible.Right();
        bottom = m_rcVisible.Bottom();
        top = m_rcVisible.Top();
    }
    gl.Ortho( left, right,
              bottom, top,
              -1, 1);

    gl.MatrixMode(GL_MODELVIEW);

    switch(m_MatrixPolicy)  {
    case eNeverUpdate: break;
    case eAlwaysUpdate:  x_UpdateProjectMatrices(); break;
    }

    return true;
}

bool    CGlPane::x_OpenPixels()
{
    x_Open_SetViewport();

    IRender& gl = GetGl();

    gl.MatrixMode(GL_PROJECTION);   
    gl.LoadIdentity();
    if (m_bExactOrthoProjection)
        gl.Ortho(m_rcVP.Left(), m_rcVP.Right(), m_rcVP.Bottom(), m_rcVP.Top(), -1, 1);
    else
        gl.Ortho(m_rcVP.Left() - 0.5, m_rcVP.Right() + 0.5, m_rcVP.Bottom() - 0.5, m_rcVP.Top() + 0.5, -1, 1);

    gl.MatrixMode(GL_MODELVIEW);
    return true;
}

void    CGlPane::x_UpdateProjectMatrices(void)
{
    // Use the saved viewport from CGlRender since rather than
    // calling glGetIntegerv(GL_VIEWPORT, m_mxVP), since that viewport
    // will be truncated to the maximum OpenGL size and we may have set up
    // the viewport as a larger size to render to non-opengl target, e.g. PDF
    IRender& gl = GetGl();
    gl.GetViewport(m_mxVP);
    gl.GetProjectionMatrix(m_mxProjection);
    gl.GetModelViewMatrix(m_mxModelView);
}

////////////////////////////////////////////////////////////////////////////////
// Zoom fucntions

bool    CGlPane::IsZoomInAvaiable()
{
    return m_bZoomEnX || m_bZoomEnY;
}

bool    CGlPane::IsZoomOutAvaiable(void)
{
    bool    av_x = ::fabs(m_rcVisible.Right() - m_rcVisible.Left())
                    < ::fabs(m_rcLimits.Right() - m_rcLimits.Left());
    bool    av_y = ::fabs(m_rcVisible.Top() - m_rcVisible.Bottom())
                    < ::fabs(m_rcLimits.Top() - m_rcLimits.Bottom());
    return (m_bZoomEnX && av_x) || (m_bZoomEnY && av_y);
}

void    CGlPane::ZoomAll(int options)
{
    if((options & fZoomX)  && (m_bZoomEnX  ||  (options & fForce)))  {
        m_rcVisible.SetLeft(m_rcLimits.Left());
        m_rcVisible.SetRight(m_rcLimits.Right());

        m_rcOrigVisible.SetLeft(m_rcLimits.Left());
        m_rcOrigVisible.SetRight(m_rcLimits.Right());
    }
    if((options & fZoomY)  && (m_bZoomEnY  ||  (options & fForce)))  {
        m_rcVisible.SetBottom(m_rcLimits.Bottom());
        m_rcVisible.SetTop(m_rcLimits.Top());

        m_rcOrigVisible.SetBottom(m_rcLimits.Bottom());
        m_rcOrigVisible.SetTop(m_rcLimits.Top());
    }
    // applying constraints
    x_CorrectScale(m_rcOrigVisible.CenterPoint());

    x_AdjustVisibleRect();
}

void    CGlPane::ZoomPoint(TModelUnit x, TModelUnit y, TModelUnit factor, int options)
{
    _ASSERT(factor > 0.01  && factor < 100); // keep it reasonable

    m_rcOrigVisible = m_rcVisible;
    if ((options & fZoomX)  &&  m_bZoomEnX) {
        TModelUnit new_w = m_rcVisible.Width() / factor;
        TModelUnit left = x - new_w / 2;
        m_rcOrigVisible.SetLeft(left);
        m_rcOrigVisible.SetRight(left + new_w);
    }
    if ((options & fZoomY)  &&  m_bZoomEnY) {
        TModelUnit new_h = m_rcVisible.Height() / factor;
        TModelUnit bottom = y - new_h / 2;
        m_rcOrigVisible.SetBottom(bottom);
        m_rcOrigVisible.SetTop(bottom + new_h);
    }

    m_rcVisible = m_rcOrigVisible;

    // applying constraints
    x_CorrectScale(m_rcOrigVisible.CenterPoint());
    x_ScaleToFitLimits();
    x_ShiftToFitLimits();
    x_AdjustVisibleRect();
}

void    CGlPane::ZoomInCenter(int options)
{
    ZoomIn(m_rcVisible.CenterPoint(), options);
}

void    CGlPane::ZoomOutCenter(int options)
{
    ZoomOut(m_rcVisible.CenterPoint(), options);
}

void    CGlPane::ZoomRect(const TModelRect& r)
{
    SetVisibleRect(r);

    TModelPoint p_center = m_rcVisible.CenterPoint();

    x_CorrectScale(p_center);
    x_ScaleToFitLimits(); //### merege with CorrectScale
    x_ShiftToFitLimits();

    x_AdjustVisibleRect();
}

void    CGlPane::SetScale(TModelUnit scale_x, TModelUnit scale_y, TModelPoint p_center)
{
    TModelUnit new_w = ::abs(m_rcVP.Width()) * scale_x;
    TModelUnit left = p_center.X() - new_w / 2;
    m_rcOrigVisible.SetLeft(left);
    m_rcOrigVisible.SetRight(left + new_w);

    TModelUnit new_h = ::abs(m_rcVP.Height()) * scale_y;
    TModelUnit bottom = p_center.Y() - new_h / 2;
    m_rcOrigVisible.SetBottom(bottom);
    m_rcOrigVisible.SetTop(bottom + new_h);
    m_rcVisible = m_rcOrigVisible;

    x_CorrectScale(p_center);
    x_ScaleToFitLimits();
    x_ShiftToFitLimits();

    x_AdjustVisibleRect();
}


void    CGlPane::SetScaleRefPoint(TModelUnit scale_x, TModelUnit scale_y,
                             TModelPoint p_ref)
{
    TModelUnit epsilon = 1e-4f;
    TModelUnit new_w = ::abs(m_rcVP.Width()) * scale_x;
    TModelUnit width = std::max(epsilon, m_rcVisible.Width()); // avoid divide-by-zero
    TModelUnit left_ratio = (p_ref.X() - m_rcVisible.Left()) / width;
    TModelUnit left = p_ref.X() - new_w * left_ratio;
    m_rcOrigVisible.SetLeft(left);
    m_rcOrigVisible.SetRight(left + new_w);

    TModelUnit new_h = ::abs(m_rcVP.Height()) * scale_y;
    double height = m_rcVisible.Height();
    if (fabs(height) < epsilon) { // avoid divide-by-zero
        height = height > 0.0 ? epsilon : -epsilon;
    }
    TModelUnit bottom_ratio = (p_ref.Y() - m_rcVisible.Bottom()) / height;
    TModelUnit bottom = p_ref.Y() - new_h * bottom_ratio;
    m_rcOrigVisible.SetBottom(bottom);
    m_rcOrigVisible.SetTop(bottom + new_h);

    m_rcVisible = m_rcOrigVisible;

    x_CorrectScale(m_rcVisible.CenterPoint());
    x_ScaleToFitLimits();
    x_ShiftToFitLimits();

    x_AdjustVisibleRect();
}


void    CGlPane::SetScale(TModelUnit scale_x, TModelUnit scale_y)
{
    SetScale(scale_x, scale_y, m_rcVisible.CenterPoint());
}

// constraints forced by x_CorrectScale() may be conflicting with other constraints
// in that case x_CorrectScale() has the lowerest priority
// does not work on negative scales
void    CGlPane::x_CorrectScale(TModelPoint p_center)
{
    if(m_MinScaleX != 0)    {
        TModelUnit scale_x = GetScaleX();
        if(scale_x < m_MinScaleX)   {
            scale_x = m_MinScaleX;

            TModelUnit new_w = m_rcVP.Width() * scale_x;
            TModelUnit left = p_center.X() - new_w / 2;
            m_rcOrigVisible.SetLeft(left);
            m_rcOrigVisible.SetRight(left + new_w);
        }
    }
    if(m_MinScaleY != 0)    {
        TModelUnit scale_y = GetScaleY();
        if(scale_y < m_MinScaleY)   {
            scale_y = m_MinScaleY;

            TModelUnit new_h = m_rcVP.Height() * scale_y;
            TModelUnit bottom = p_center.Y() - new_h / 2;
            m_rcOrigVisible.SetBottom(bottom);
            m_rcOrigVisible.SetTop(bottom + new_h);
        }
    }
}

// this funtions make sure that visible area is not larger then model limits
// it works fine with negative scales
void    CGlPane::x_ScaleToFitLimits(void)
{
    if(m_AdjustX & fScaleToLimits)  {
        TModelUnit w = ::fabs(m_rcOrigVisible.Width());
        if(w > ::fabs(m_rcLimits.Width())) {
            m_rcOrigVisible.SetLeft(m_rcLimits.Left());
            m_rcOrigVisible.SetRight(m_rcLimits.Right());
        }
    }
    if(m_AdjustY & fScaleToLimits)  {
        TModelUnit h = ::fabs(m_rcOrigVisible.Height());
        if(h > ::fabs(m_rcLimits.Height())) {
            m_rcOrigVisible.SetBottom(m_rcLimits.Bottom());
            m_rcOrigVisible.SetTop(m_rcLimits.Top());
        }
    }
}

// Moves visible area so that it will be located within model limits.
// If visible area is greater then limits rect, function aligns origin of the visible
// rect with the origin of the limits rect and lets the other corner of visible rect
// extend outside limits.
// works on negative scales
void CGlPane::x_ShiftToFitLimits(void)
{
    if(m_AdjustX & fShiftToLimits)  { //horizontal adjustment
        TModelUnit shift = 0;
        bool neg_range = m_rcLimits.Right() < m_rcLimits.Left();
        TModelUnit left_shift = m_rcLimits.Left() - m_rcOrigVisible.Left();
        TModelUnit right_shift = m_rcLimits.Right() - m_rcOrigVisible.Right();

        shift = s_GetShift(left_shift, right_shift, neg_range, m_TypeX);
        m_rcOrigVisible.Offset(shift, 0);
    }
    if(m_AdjustY & fShiftToLimits)  { // vertical adjustment
        TModelUnit shift = 0;
        bool neg_range = m_rcLimits.Top() < m_rcLimits.Bottom();
        TModelUnit bottom_shift = m_rcLimits.Bottom() - m_rcOrigVisible.Bottom();
        TModelUnit top_shift = m_rcLimits.Top() - m_rcOrigVisible.Top();

        shift = s_GetShift(bottom_shift, top_shift, neg_range, m_TypeY);
        m_rcOrigVisible.Offset(0, shift);
    }
    m_rcVisible = m_rcOrigVisible;
}

// adjust only m_rcVisible not m_rcOrigVisible
// does not work on negative scales
void    CGlPane::x_AdjustVisibleRect(void)
{
    TModelUnit scale_x = GetScaleX();
    TModelUnit scale_y = GetScaleY();

    TModelUnit new_scale_x = scale_x;
    TModelUnit new_scale_y = scale_y;

    if(m_bProportionalMode) {
        new_scale_x = new_scale_y = max(scale_x, scale_y);
    }

    // use scale to adjust m_rcVisible with regard to its origin
    if (new_scale_x != scale_x) {
        TModelUnit w = m_rcVP.Width() * new_scale_x;
        TModelUnit left = m_rcOrigVisible.Left();
        TModelUnit right = m_rcOrigVisible.Right();

        switch(m_TypeX) {
        case eOriginLeft:   right = left + w; break;
        case eOriginRight:  left = right - w; break;
        case eOriginCenter: {
            left = (left + right - w) / 2;
            right = left + w;
        }; break;
        case eOriginBottom:
        case eOriginTop: _ASSERT(false); break; // invalid values
        }
        m_rcVisible.SetLeft(left);
        m_rcVisible.SetRight(right);
    }
    if (new_scale_y != scale_y) {
        TModelUnit h = m_rcVP.Height() * new_scale_y;
        TModelUnit  top = m_rcOrigVisible.Top();
        TModelUnit  bottom = m_rcOrigVisible.Bottom();

        switch(m_TypeY) {
        case eOriginBottom: top = bottom + h; break;
        case eOriginTop: bottom = top - h; break;
        case eOriginCenter: {
            bottom = (top + bottom - h) / 2;
            top = bottom + h;
        }; break;
        case eOriginLeft:
        case eOriginRight: _ASSERT(false); break; // invalid values
        }
        m_rcVisible.SetBottom(bottom);
        m_rcVisible.SetTop(top);
    }
}

TModelUnit  CGlPane::s_GetShift(TModelUnit low_shift, TModelUnit high_shift, bool neg_range, EOriginType type)
{
    TModelUnit shift = 0;
    switch(type)    {
    case eOriginBottom:
    case eOriginLeft: {
        if(neg_range)   {
            if(low_shift < 0)
                shift = low_shift;
            else if(high_shift > 0)
                shift = min(high_shift, low_shift);
        } else {
            if(low_shift > 0)
                shift = low_shift;
            else if(high_shift < 0)
                shift = max(high_shift, low_shift);
        }
    }; break;
    case eOriginRight:
    case eOriginTop: {
        if(neg_range)   {
            if(high_shift > 0)
                shift = high_shift;
            else if(low_shift < 0)
                shift = max(high_shift, low_shift);
        } else {
            if(high_shift < 0)
                shift = high_shift;
            else if(low_shift > 0)
                shift = max(high_shift, low_shift);
        }
    }; break;
    case eOriginCenter: {
        shift = (low_shift + high_shift) / 2;
    }; break;
    } //switch
    return shift;
}

////////////////////////////////////////////////////////////////////////////////
// Scroll functions
bool    CGlPane::NeedsScrollX(void) const
{
    return (m_rcVisible.Left() > m_rcLimits.Left())
            ||  (m_rcVisible.Right() < m_rcLimits.Right());
}

bool    CGlPane::NeedsScrollY(void) const
{
    TModelUnit d1 = ::fabs(m_rcVisible.Height());
    TModelUnit d2 = ::fabs(m_rcLimits.Height());
    return d1 < d2;
}

void    CGlPane::Scroll(TModelUnit dx, TModelUnit dy)
{
    m_rcOrigVisible.Offset(dx, dy);
    m_rcVisible.Offset(dx, dy);

    x_ShiftToFitLimits();
}

// Set the visible rect to the same proporitional position within the model 
// rect as vr is to mr
void CGlPane::SetProportional(TModelRect& vr, TModelRect& mr) {
    TModelUnit widthp = vr.Width() / mr.Width();
    TModelUnit heightp = vr.Height() / mr.Height();
    TModelUnit offsetxp = (vr.Left() - mr.Left()) / mr.Width();
    TModelUnit offsetyp = (vr.Bottom() - mr.Bottom()) / mr.Height();

    m_rcVisible.SetLeft(m_rcLimits.Left() + offsetxp*m_rcLimits.Width());
    m_rcVisible.SetBottom(m_rcLimits.Bottom() + offsetyp*m_rcLimits.Height());
    m_rcVisible.SetRight(m_rcVisible.Left() + widthp*m_rcLimits.Width());
    m_rcVisible.SetTop(m_rcVisible.Bottom() + heightp*m_rcLimits.Height());

    m_rcOrigVisible = m_rcVisible;
}

void CGlPane::ScrollTo(const CVect2<TModelUnit>& pos, TModelUnit pctx, TModelUnit pcty)
{
    TModelUnit w = m_rcVisible.Width();
    TModelUnit h = m_rcVisible.Height();

    m_rcVisible.SetLeft(pos.X() - w * pctx);
    m_rcVisible.SetRight(m_rcVisible.Left() + w);

    m_rcVisible.SetBottom(pos.Y() - h * pcty);
    m_rcVisible.SetTop(m_rcVisible.Bottom() + h);

    // want to push, not resize (since this is not a zoom op) the visible rect
    // to be fully inside the limits rect   
    if (m_rcVisible.Left() < m_rcLimits.Left()) {
        m_rcVisible.Offset(m_rcLimits.Left() - m_rcVisible.Left(), 0);
    }
    else if (m_rcVisible.Right() > m_rcLimits.Right()) {
        m_rcVisible.Offset(m_rcLimits.Right() - m_rcVisible.Right(), 0);
    }

    if (m_rcVisible.Bottom() < m_rcLimits.Bottom()) {
        m_rcVisible.Offset(0, m_rcLimits.Bottom() - m_rcVisible.Bottom());
    }
    else if (m_rcVisible.Top() > m_rcLimits.Top()) {
        m_rcVisible.Offset(0, m_rcLimits.Top() - m_rcVisible.Top());
    }

    m_rcOrigVisible = m_rcVisible;
}



////////////////////////////////////////////////////////////////////////////////
// Coordinate transformations (Project - UnProject)

TVPUnit CGlPane::ProjectX(TModelUnit m_x) const
{
    _ASSERT(m_MatrixPolicy == eAlwaysUpdate);
    if(m_bEnableOffset) {
        m_x -= m_rcVisible.Left();
    }
    GLdouble x = 0;
    GLdouble y = 0;
    GLdouble z = 0;
    gluProject( (GLdouble) m_x, 0.0, 0.0,
                m_mxModelView, m_mxProjection, m_mxVP, &x, &y, &z);

    return (TVPUnit)x;
}

TVPUnit CGlPane::ProjectY(TModelUnit m_y) const
{
    _ASSERT(m_MatrixPolicy == eAlwaysUpdate);
    if(m_bEnableOffset) {
        m_y -= m_rcVisible.Bottom();
    }
    GLdouble x = 0;
    GLdouble y = 0;
    GLdouble z = 0;
    gluProject( 0.0, (GLdouble) m_y, 0.0,
                m_mxModelView, m_mxProjection, m_mxVP, &x, &y, &z);

    return (TVPUnit)y;
}

TVPPoint    CGlPane::Project(TModelUnit m_x, TModelUnit m_y) const
{
    _ASSERT(m_MatrixPolicy == eAlwaysUpdate);
    if(m_bEnableOffset) {
        m_x -= m_rcVisible.Left();
        m_y -= m_rcVisible.Bottom();
    }
    GLdouble x = 0;
    GLdouble y = 0;
    GLdouble z = 0;
    gluProject( (GLdouble) m_x, (GLdouble) m_y, 0.0,
                m_mxModelView, m_mxProjection, m_mxVP, &x, &y, &z);
    return TVPPoint((TVPUnit)x, (TVPUnit)y);
}

TModelUnit CGlPane::UnProjectX(TVPUnit m_x) const
{
    _ASSERT(m_MatrixPolicy == eAlwaysUpdate);

    GLdouble x = 0;
    GLdouble y = 0;
    GLdouble z = 0;
    gluUnProject( (GLdouble) m_x, 0.0, 0.0,
                   m_mxModelView, m_mxProjection, m_mxVP,
                   &x, &y, &z);
    if(m_bEnableOffset) {
        x += m_rcVisible.Left();
    }
    return x;
}

TModelUnit CGlPane::UnProjectY(TVPUnit m_y) const
{
    _ASSERT(m_MatrixPolicy == eAlwaysUpdate);

    GLdouble x = 0;
    GLdouble y = 0;
    GLdouble z = 0;
    gluUnProject( 0.0, (GLdouble) m_y, 0.0,
                  m_mxModelView, m_mxProjection, m_mxVP,
                  &x, &y, &z);
    if(m_bEnableOffset) {
        y += m_rcVisible.Bottom();
    }
    return TModelUnit(y);
}

TModelPoint CGlPane::UnProject(TVPUnit m_x, TVPUnit m_y) const
{
    _ASSERT(m_MatrixPolicy == eAlwaysUpdate);

    GLdouble x = 0;
    GLdouble y = 0;
    GLdouble z = 0;
    gluUnProject( (GLdouble) m_x, (GLdouble) m_y, 0.0,
                  m_mxModelView, m_mxProjection, m_mxVP,
                  &x, &y, &z);
    if(m_bEnableOffset) {
        x += m_rcVisible.Left();
        y += m_rcVisible.Bottom();
    }
    return TModelPoint(x, y);
}

TModelUnit  CGlPane::UnProjectWidth(TVPUnit vp_w) const
{
    _ASSERT(m_MatrixPolicy == eAlwaysUpdate);

    GLdouble x1 = 0;
    GLdouble x2 = 0;
    GLdouble y  = 0;
    GLdouble z  = 0;
    gluUnProject( 0.0, 0.0, 0.0, m_mxModelView, m_mxProjection, m_mxVP, &x1, &y, &z);
    gluUnProject( vp_w, 0.0, 0.0, m_mxModelView, m_mxProjection, m_mxVP, &x2, &y, &z);
    return x2 - x1;
}

TModelUnit  CGlPane::UnProjectHeight(TVPUnit vp_h) const
{
    _ASSERT(m_MatrixPolicy == eAlwaysUpdate);

    GLdouble x  = 0;
    GLdouble y1 = 0;
    GLdouble y2 = 0;
    GLdouble z  = 0;
    gluUnProject( 0.0, 0.0, 0.0, m_mxModelView, m_mxProjection, m_mxVP, &x, &y1, &z);
    gluUnProject( 0.0, vp_h, 0.0, m_mxModelView, m_mxProjection, m_mxVP, &x, &y2, &z);
    return y2 - y1;
}

CVect2<TModelUnit> CGlPane::ProjectEx(CVect2<TModelUnit> pt)
{
    _ASSERT(m_MatrixPolicy == eAlwaysUpdate);
    if(m_bEnableOffset) {
        pt.X() -= m_rcVisible.Left();
        pt.Y() -= m_rcVisible.Bottom();
    }
    GLdouble x = 0.0;
    GLdouble y = 0.0;
    GLdouble z = 0.0;

    gluProject( (GLdouble) pt.X(), (GLdouble) pt.Y(), 0.0,
                m_mxModelView, m_mxProjection, m_mxVP, &x, &y, &z);
    return CVect2<TModelUnit>((TModelUnit)x, (TModelUnit)y);
}

CVect2<TModelUnit> CGlPane::UnProjectEx(const CVect2<TModelUnit>& pt)
{
    _ASSERT(m_MatrixPolicy == eAlwaysUpdate);

    GLdouble x = 0.0;
    GLdouble y = 0.0;
    GLdouble z = 0.0;

    GLint   vp[4];
    vp[0] = (GLint)m_rcVP.Left();
    vp[1] = (GLint)m_rcVP.Bottom();
    vp[2] = (GLint)m_rcVP.Right();
    vp[3] = (GLint)m_rcVP.Top();

    gluUnProject( (GLdouble) pt.X(), (GLdouble) pt.Y(), 0.0,
                  m_mxModelView, m_mxProjection, vp,
                  &x, &y, &z);
    if(m_bEnableOffset) {
        x += m_rcVisible.Left();
        y += m_rcVisible.Bottom();
    }

    return CVect2<TModelUnit>(TModelUnit(x), TModelUnit(y));
}

TModelUnit CGlPane::GetPixelDist(TModelUnit d) const
{
    return d/m_OutputUnitScaler;
}

END_NCBI_SCOPE
