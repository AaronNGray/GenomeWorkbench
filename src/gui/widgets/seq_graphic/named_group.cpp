/*  $Id:
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
* Authors:  Liangshou Wu
*
*/

#include <ncbi_pch.hpp>
#include <gui/widgets/seq_graphic/named_group.hpp>
#include <gui/widgets/seq_graphic/simple_layout_policy.hpp>
#include <gui/widgets/seq_graphic/rendering_ctx.hpp>
#include <gui/opengl/irender.hpp>
#include <math.h>

BEGIN_NCBI_SCOPE

static const int kPadding = 2;
static const int kIndentPix = 14;      /// in pixel


void CNamedGroup::GetHTMLActiveAreas(TAreaVector* p_areas) const
{
    if ( !x_Visible_V() ) return;

    IRender& gl = GetGl();

    CHTMLActiveArea area;
    TModelUnit left = 0;
    TModelUnit top = 0;
    x_Local2World(left, top);
    //    top -= 3.0;
    area.m_Bounds.SetTop(top);
    TModelUnit fs = gl.TextHeight(m_TitleFont);
    area.m_Bounds.SetBottom(top + fs + kPadding);
    area.m_Bounds.SetLeft(-1);
    area.m_Bounds.SetRight(m_Indent * kIndentPix);
    area.m_Flags = CHTMLActiveArea::fComment
        | CHTMLActiveArea::fNoHighlight
        | CHTMLActiveArea::fNoSelection
        | CHTMLActiveArea::fNoTooltip;

    area.m_ID = m_Title;

    // required, but nonsense fields
    area.m_PositiveStrand = true;
    area.m_SeqRange.SetFrom(0);
    area.m_SeqRange.SetTo(0);
    p_areas->push_back(area);

    //
    CLayoutGroup::GetHTMLActiveAreas(p_areas);
}


void CNamedGroup::x_Draw() const
{
    if (x_Visible_V()) {
        IRender& gl = GetGl();

        TModelUnit fs =  gl.TextHeight(m_TitleFont);
        TModelUnit bottom = GetTop() + fs + kPadding;

        TModelRect rcm_adj(GetLeft(), GetTop(), GetRight(), bottom);
        m_Context->DrawShadedQuad(rcm_adj, m_BgColor, CRgbaColor(1.0f, 1.0f, 1.0f, 0.0));

        if (m_ShowTitle) {
            TModelUnit xM = GetLeft();
            TModelUnit repeat_dist = m_RepeatDist;
            string title = m_Title;
            int repeat_num = 1;
            xM += m_Indent * m_Context->ScreenToSeq(kIndentPix);

            if (m_RepeatTitle) {
                TModelUnit title_len = gl.TextWidth(m_TitleFont, m_Title.c_str()) + 20.0;
                repeat_dist = max(repeat_dist,  title_len);
                repeat_dist = m_Context->ScreenToSeq(repeat_dist);
                repeat_num = (int)ceil(fabs(GetWidth() / repeat_dist));
            } else {
                TModelUnit max_title_w = max(50.0, m_Context->SeqToScreen(fabs(GetWidth())) - 100.0);
                title = m_TitleFont->Truncate(m_Title, max_title_w);
            }
            gl.ColorC(m_TitleColor);
            for (int i = 0; i < repeat_num; ++i) {
                TModelUnit x = xM;
                m_Context->AdjustToOrientation(x);
                TModelUnit yM = GetTop() + fs;
                m_Context->TextOut(m_TitleFont.GetPointer(),
                    title.c_str(), x, yM, false, false);
                xM += repeat_dist;
            }
        }

        CLayoutGroup::x_Draw();
    }
}


void CNamedGroup::x_UpdateBoundingBox()
{
    IRender& gl = GetGl();

    CLayoutGroup::x_UpdateBoundingBox();

    TModelUnit fs = gl.TextHeight(m_TitleFont);
    TModelUnit offset_v = kPadding + fs;
    SetHeight(GetHeight() + offset_v);
    const TModelRange& vr = m_Context->GetVisibleRange();
    SetLeft(vr.GetFrom());
    SetWidth(vr.GetLength() - 1.0);

    // push all children down by offset_v
    NON_CONST_ITERATE (TObjectList, iter, SetChildren()) {
        (*iter)->SetTop((*iter)->GetTop() + offset_v);
    }
}



END_NCBI_SCOPE

