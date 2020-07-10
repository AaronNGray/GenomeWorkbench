#ifndef GUI_WIDGETS_GL___IRENDERABLE__HPP
#define GUI_WIDGETS_GL___IRENDERABLE__HPP

/*  $Id: irenderable.hpp 35505 2016-05-16 15:18:00Z shkeda $
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
 * Authors:  Vlad Lebedev
 *
 * File Description:
 *    IRenderable -- the basic interface for GL panels for GBENCH
 */


#include <gui/opengl/glpane.hpp>

#include <corelib/ncbiobj.hpp>
#include <corelib/ncbistd.hpp>


BEGIN_NCBI_SCOPE

///////////////////////////////////////////////////////////////////////////////
/// IRenderable
/// class IRenderable defines an abstract interface required for rendering
/// graphical panels in the given space (see CGlPane).

class  NCBI_GUIWIDGETS_GL_EXPORT  IRenderable
{
public:
    virtual ~IRenderable(void) { }

    virtual void Render(CGlPane& pane) = 0;

    // gets / sets the rectangle in OpenGL viewport space associated with
    // this component
    virtual TVPRect    GetVPRect() const = 0;
    virtual void       SetVPRect(const TVPRect& rc) = 0;

    // gets / sets the rectangle in OpenGL model space associated with
    // this component
    virtual TModelRect  GetModelRect() const = 0;
    virtual void        SetModelRect(const TModelRect& rc) = 0;

    virtual TVPPoint PreferredSize() = 0;

    virtual bool    IsVisible() = 0;
    virtual void    SetVisible(bool set) = 0;

    virtual bool    NeedTooltip(CGlPane& pane, TVPUnit vp_x, TVPUnit vp_y) = 0;
    virtual string  GetTooltip() = 0;

    virtual void SetOrder(int order) = 0;
    virtual int GetOrder() const = 0;
};


///////////////////////////////////////////////////////////////////////////////
/// CRenderableImpl
class  NCBI_GUIWIDGETS_GL_EXPORT  CRenderableImpl : public IRenderable
{
public:
    CRenderableImpl();
    virtual ~CRenderableImpl();

    /// @name IRenderable implementation
    /// @{
    /// implementing some of the IRenderable functions, other need to be
    /// implemented in derived classes
    virtual TVPRect    GetVPRect() const;
    virtual void       SetVPRect(const TVPRect& rc);

    virtual TModelRect  GetModelRect() const;
    virtual void        SetModelRect(const TModelRect& rc);

    virtual TVPPoint PreferredSize();

    virtual bool    IsVisible();
    virtual void    SetVisible(bool set);

    virtual bool    NeedTooltip(CGlPane& pane, TVPUnit vp_x, TVPUnit vp_y);
    virtual string  GetTooltip();

    virtual void SetOrder(int order);
    virtual int GetOrder() const;

    /// @}


protected:
    TVPRect     m_VPRect;
    TModelRect  m_ModelRect;
    bool        m_Visible;
    int         m_Order = 0;   // sort helper
};

inline
void CRenderableImpl::SetOrder(int order)
{
    m_Order = order;
}

inline
int CRenderableImpl::GetOrder() const
{
    return m_Order;
}



END_NCBI_SCOPE

#endif  // GUI_WIDGETS_GL___IRENDERABLE__HPP
