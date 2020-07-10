#ifndef __GUI_WIDGETS_ALNMULTI___ALNMULTI_HEADER__HPP
#define __GUI_WIDGETS_ALNMULTI___ALNMULTI_HEADER__HPP

/*  $Id: alnmulti_header.hpp 31702 2014-11-06 19:57:35Z falkrb $
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

#include <gui/opengl/gldlist.hpp>

#include <gui/opengl/glpane.hpp>
#include <gui/opengl/gltexturefont.hpp>

#include <gui/widgets/gl/irenderable.hpp>
#include <gui/widgets/gl/ievent_handler.hpp>

#include <objtools/alnmgr/aln_explorer.hpp>


BEGIN_NCBI_SCOPE


////////////////////////////////////////////////////////////////////////////////
/// IAlnMultiHeaderContext
class NCBI_GUIWIDGETS_ALNMULTIPLE_EXPORT IAlnMultiHeaderContext
{
public:
    typedef IAlnExplorer::ESortState     TSortState;

    /// SColumn describes a single column
    struct SColumn   {
        string  m_Name;
        int     m_Pos; /// horizontal position in viewport
        int     m_Width; /// width in pixels
        int     m_UserData; /// can be used to identify column
        bool    m_Visible;
        TSortState  m_SortState;

        SColumn();
        inline  int VisibleWidth()  const   {   return m_Visible ? m_Width : 0;    }
        inline  int Right() const   {   return m_Pos + (m_Visible ? m_Width - 1 : 0);   }
    };

    virtual ~IAlnMultiHeaderContext()   {}

    virtual int     GetColumnsCount() const = 0;
    virtual const   SColumn&  GetColumn(int index) const = 0;
    virtual int     GetColumnIndexByX(int x) const = 0;
    virtual int     GetResizableColumnIndex() const = 0;
};

////////////////////////////////////////////////////////////////////////////////
/// CAlnMultiHeader
class NCBI_GUIWIDGETS_ALNMULTIPLE_EXPORT CAlnMultiHeader
     : public CRenderableImpl
{
public:
    typedef IAlnMultiHeaderContext  TContext;
    typedef IAlnMultiHeaderContext::SColumn TColumn;

    class CStyle
    {
    public:
        CRgbaColor    m_BackColor;
        CRgbaColor    m_BorderColor;
        CRgbaColor    m_TextColor;

        CGlTextureFont   m_Font;

        CStyle();
    };

    CAlnMultiHeader();
    ~CAlnMultiHeader();

    virtual void    SetContext(IAlnMultiHeaderContext* context);

    /// @name IRenderable implementation
    /// @{
    virtual void Render(CGlPane& pane);

    virtual TVPPoint PreferredSize();

    virtual bool    NeedTooltip(CGlPane& pane, int vp_x, int vp_y);
    virtual string  GetTooltip();
    /// @}

    virtual void    RenderColumnHeader(int index, const TVPRect& rc);

    const CStyle&   GetStyle() const;
    CStyle&     GetStyle();
    void        SetStyle(const CStyle& style);

protected:
    void    x_RenderBox(const TVPRect& rc);

protected:
    TContext*   m_Context;
    CStyle      m_Style;

    int m_TooltipColumn;
    bool m_OnBorder;
};

////////////////////////////////////////////////////////////////////////////////
/// CAlnMultiHeaderStyle

END_NCBI_SCOPE

#endif  // __GUI_WIDGETS_ALNMULTI___ALNMULTI_HEADER__HPP
