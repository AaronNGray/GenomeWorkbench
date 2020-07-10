/*  $Id: alnmulti_header.cpp 44992 2020-05-04 18:14:48Z shkeda $
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

#include <gui/widgets/aln_multiple/alnmulti_header.hpp>

#include <gui/opengl/glhelpers.hpp>
#include <gui/opengl/irender.hpp>


BEGIN_NCBI_SCOPE


////////////////////////////////////////////////////////////////////////////////
/// CAlnMultiHeader
const static int kHeight = 16;
static const int kBorderZone = 4;

CAlnMultiHeader::CStyle::CStyle()
:   m_BackColor(0.9f, 0.9f, 0.9f),
    m_BorderColor(0.8f, 0.8f, 0.8f),
    m_TextColor(0.0f, 0.0f, 0.0f),
    m_Font(CGlTextureFont::eFontFace_Helvetica, 12)
{
}


CAlnMultiHeader::CAlnMultiHeader()
:   m_Context(NULL),
    m_TooltipColumn(-1),
    m_OnBorder(false)
{
}


CAlnMultiHeader::~CAlnMultiHeader()
{
}


void CAlnMultiHeader::SetContext(IAlnMultiHeaderContext* context)
{
    m_Context = context;
}


void CAlnMultiHeader::Render(CGlPane& pane)
{
    _ASSERT(m_Context);

    IRender& gl = GetGl();

    //CGlAttrGuard AttrGuard(GL_POLYGON_BIT | GL_LINE_BIT);
    gl.LineWidth(1.0f);

    pane.OpenPixels();

    TVPRect rc_col = pane.GetViewport();

    int col_n = m_Context->GetColumnsCount();
    for( int i = 0;  i < col_n;  i++ )  {
        const TColumn& column = m_Context->GetColumn(i);
        if(column.VisibleWidth())   {
            rc_col.SetHorz(column.m_Pos, column.Right());
            RenderColumnHeader(i, rc_col);
        }
    }

    pane.Close();
}


static const int kHorzOff = 3;
static const int kVertOff = 4;
static const int kSortMarkerSize = 7;

void CAlnMultiHeader::RenderColumnHeader(int index, const TVPRect& rc)
{
    IRender& gl = GetGl();

    x_RenderBox(rc);

    const TColumn& column = m_Context->GetColumn(index);
    bool render_sort = (column.m_SortState != IAlnExplorer::eNotSupported) && !gl.IsPrinterFriendly();
    int sort_space = render_sort ? kSortMarkerSize : 0;

    // render text
    gl.ColorC(m_Style.m_TextColor);

    int left = rc.Left() + kHorzOff;
    int text_y = rc.Bottom();
    int text_w = rc.Width() - 2 * kHorzOff - sort_space;
    int text_h = rc.Height();
    const char* str = column.m_Name.c_str();

    gl.BeginText(&m_Style.m_Font, m_Style.m_TextColor);
    gl.WriteText(left, text_y, text_w, text_h, column.m_Name.c_str(), IGlFont::eAlign_Left | IGlFont::eAlign_VCenter);
    gl.EndText();

    if(render_sort) {   // render sort marker
        int x = left + text_w + (sort_space - kSortMarkerSize) / 2;
        int y_top = rc.Top() - (rc.Height() - kSortMarkerSize) / 2;
        int y_bot = y_top - kSortMarkerSize;
        int x_c = x + kSortMarkerSize / 2;

        gl.PolygonMode(GL_FRONT_AND_BACK, GL_LINE);

        switch(column.m_SortState)  {
        case IAlnExplorer::eDescending:
            gl.Begin(GL_TRIANGLES);
            gl.Vertex2f(float(x), float(y_top));
            gl.Vertex2f(float(x_c), float(y_bot));
            gl.Vertex2f(float(x + kSortMarkerSize - 1), float(y_top));
            gl.End();
            break;

        case IAlnExplorer::eAscending:
            gl.Begin(GL_TRIANGLES);
            gl.Vertex2f(float(x), float(y_bot));
            gl.Vertex2f(float(x_c), float(y_top));
            gl.Vertex2f(float((x + kSortMarkerSize - 1)), float(y_bot));
            gl.End();
            break;

        case IAlnExplorer::eUnSorted:
            gl.LineWidth(1.0);
            gl.ColorC(m_Style.m_BorderColor);
            gl.Begin(GL_LINE_STRIP);
            gl.Vertex2f(float(x), float(y_top));
            gl.Vertex2f(float(x + kSortMarkerSize - 1), float(y_top));
            gl.Vertex2f(float(x + kSortMarkerSize - 1), float(y_bot));
            gl.Vertex2f(float(x), float(y_bot));
            gl.Vertex2f(float(x), float(y_top));
            gl.End();
            //gl.Rectf(float(x), float(y_top), float(x + kSortMarkerSize - 1), float(y_bot));
            break;

        default:
            break;
        }

		gl.PolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    }
}


void CAlnMultiHeader::x_RenderBox(const TVPRect& rc)
{
    IRender& gl = GetGl();

     // fill background
    gl.ColorC(m_Style.m_BackColor);
    gl.PolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    gl.RectC(rc);

    CRgbaColor c = m_Style.m_BorderColor;
    c.Lighten(0.25);
    gl.ColorC(c);
    gl.Begin(GL_LINE_STRIP);
        gl.Vertex2f(float(rc.Left()), float(rc.Bottom()));
        gl.Vertex2f(float(rc.Left()), float(rc.Top()));
        gl.Vertex2f(float(rc.Right()), float(rc.Top()));
    gl.End();

    c = m_Style.m_BorderColor;
    c.Darken(0.25);
    gl.ColorC(c);
    gl.Begin(GL_LINE_STRIP);
        gl.Vertex2f(float(rc.Right()), float(rc.Top()));
        gl.Vertex2f(float(rc.Right()), float(rc.Bottom()));
        gl.Vertex2f(float(rc.Left()), float(rc.Bottom()));
    gl.End();
}


TVPPoint CAlnMultiHeader::PreferredSize()
{
    return TVPPoint(0, kHeight);
}


bool CAlnMultiHeader::NeedTooltip(CGlPane& /*pane*/, int vp_x, int vp_y)
{
    m_OnBorder = false;
    if (m_VPRect.PtInRect(vp_x, vp_y)) {
        m_TooltipColumn = m_Context->GetColumnIndexByX(vp_x);
        int neighbor_col = m_Context->GetColumnIndexByX(vp_x + kBorderZone);
        if (neighbor_col > -1  &&  neighbor_col != m_TooltipColumn) {
            m_OnBorder = true;
        } 
        if ( !m_OnBorder ) {
            neighbor_col = m_Context->GetColumnIndexByX(vp_x - kBorderZone);
            if (neighbor_col > -1  &&  neighbor_col != m_TooltipColumn) {
                m_OnBorder = true;
            }
        }
        return m_TooltipColumn >= 0;
    } else {
        _TRACE("CAlnMultiHeader::NeedTooltip(): out-of-bounds");
        return false;
    }
}


string CAlnMultiHeader::GetTooltip()
{
    if (m_OnBorder) {
        return string("click and drag to resize column");
    } else if(m_TooltipColumn >= 0)    {
        const TColumn& col = m_Context->GetColumn(m_TooltipColumn);
        string s = col.m_Name;
        if(col.m_SortState != IAlnExplorer::eNotSupported)  {
            s += " (click to sort)";
        }
        return s;
    }
    return "";
}


const CAlnMultiHeader::CStyle& CAlnMultiHeader::GetStyle() const
{
    return m_Style;
}


CAlnMultiHeader::CStyle& CAlnMultiHeader::GetStyle()
{
    return m_Style;
}


void CAlnMultiHeader::SetStyle(const CStyle& style)
{
    m_Style = style;
}


END_NCBI_SCOPE
