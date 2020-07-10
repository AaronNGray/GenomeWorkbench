/*  $Id: comment_glyph.cpp 41823 2018-10-17 17:34:58Z katargir $
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
 * File Description:
 */

#include <ncbi_pch.hpp>
#include <gui/widgets/seq_graphic/comment_glyph.hpp>
#include <gui/widgets/seq_graphic/rendering_ctx.hpp>
#include <gui/opengl/irender.hpp>
#include <objects/seqloc/Seq_loc.hpp>


BEGIN_NCBI_SCOPE
USING_SCOPE(objects);

static const int kDotSize = 4;

CCommentGlyph::CCommentGlyph(const string& comment,
                             CRef<CSeqGlyph> glyph)
    : m_LinkedGlyph(glyph)
    , m_Comment(comment)
    , m_Repeated(false)
{
}

CCommentGlyph::CCommentGlyph(const string& comment,
                             const TModelPoint& pos,
                             bool repeated)
    : m_Comment(comment)
    , m_TargetPos(pos)
    , m_Repeated(repeated)
{
}


bool CCommentGlyph::NeedTooltip(const TModelPoint& /*p*/, ITooltipFormatter& /*tt*/, string& /*t_title*/) const
{
    if (m_LinkedGlyph) {
        return true;
    } 

    return false;
}


void CCommentGlyph::GetTooltip(const TModelPoint& p, ITooltipFormatter& tt, string& t_title) const
{
    if (m_LinkedGlyph) {
        m_LinkedGlyph->GetTooltip(p, tt, t_title);
    }
}


//void CCommentGlyph::GetHTMLActiveAreas(TAreaVector* p_areas) const
//{
//    CHTMLActiveArea area;
//    CSeqGlyph::x_InitHTMLActiveArea(area);
//    area.m_Descr = m_Comment;
//    p_areas->push_back(area);
//}


/// for the case where the linked glyph is a object-based glyph
//
//const objects::CSeq_loc& CCommentGlyph::GetLocation(void) const
//{
//}
//
//
//CConstRef<CObject> CCommentGlyph::GetObject(TSeqPos pos) const
//{
//    TSeqRange range = GetLocation().GetTotalRange();
//    if (pos >= range.GetFrom()  &&  pos <= range.GetTo()) {
//    //if (pos >= m_MappedSeqRange.GetFrom()  &&  pos <= m_MappedSeqRange.GetTo()) {
//        return CConstRef<CObject>(m_IdHandle.GetSeqId());
//    }
//    return CConstRef<CObject>();
//}
//
//
//void CCommentGlyph::GetObjects(vector<CConstRef<CObject> >& objs) const
//{
//    objs.push_back( CConstRef<CObject>(m_IdHandle.GetSeqId()) );
//}
//
//
//bool CCommentGlyph::HasObject(CConstRef<CObject> obj) const
//{
//    return m_IdHandle.GetSeqId().GetPointer() == obj.GetPointer();
//}
//
//
//const CCommentGlyph::TIntervals& CCommentGlyph::GetIntervals(void) const
//{
//    return m_Intervals;
//}


void CCommentGlyph::x_Draw() const
{
    IRender& gl = GetGl();

    TModelUnit y_center = GetTop() + GetHeight() * 0.5;
    TModelUnit x = GetLeft() + 0.5;
    if (m_Config->m_ShowConnection) {
        TModelUnit radius = m_Context->ScreenToSeq(kDotSize * 0.5);
        gl.ColorC(m_Config->m_LineColor);
        m_Context->DrawLine(x + radius, y_center, m_TargetPos.X() + 0.5, m_TargetPos.Y());
        m_Context->DrawDisk(TModelPoint(x, y_center), kDotSize,
            m_Config->m_LineColor, false);
        x += radius * 3;
        gl.Color3f(1.0f, 1.0f, 1.0f);
        TModelRect rect(x, GetTop(), GetRight(), GetBottom());
        m_Context->DrawBackground(rect, m_Config->m_ShowBoundary);
    }

    if (IsSelected()) {
        gl.ColorC(m_Context->GetSelectionColor());
    } else {
        gl.ColorC(m_Config->m_LabelColor);
    }
    if (m_Repeated) {
        unsigned labelsCount(1);
        if ((m_Context->GetViewWidth() != 0) && (m_Context->GetViewWidth() != m_Context->GetGlPane()->GetViewport().Width())) {
            // SV displays three columns - one in the middle and one on the left and right parts of the image, which are not visible on the screen
            labelsCount = 3;
        }

        TModelUnit part_size = m_Context->GetVisibleRange().GetLength() / labelsCount;
        TModelUnit seg_width = m_Context->ScreenToSeq(gl.TextWidth(&(m_Config->m_Font), m_Comment.c_str()));
        if (part_size < seg_width) {
            labelsCount = m_Context->GetVisibleRange().GetLength() / seg_width;
            if (0 == labelsCount)
                labelsCount = 1;
            part_size = m_Context->GetVisibleRange().GetLength() / labelsCount;
        }
        TModelUnit label_x;
        for (unsigned i = 0; i < labelsCount; ++i) {
            label_x = m_Context->GetVisibleRange().GetFrom() + part_size / 2.0 + i*part_size;
            m_Context->TextOut(&m_Config->m_Font, m_Comment.c_str(), label_x, GetBottom(), true);
        }
    }
    else {
        m_Context->TextOut(&m_Config->m_Font, m_Comment.c_str(), x, GetBottom(), false);
    }
}


void CCommentGlyph::x_UpdateBoundingBox()
{
    _ASSERT(m_Context);
    if (m_Comment.empty()) {
        return;
    }

    IRender& gl = GetGl();

    TModelUnit height = gl.TextHeight(&(m_Config->m_Font));
    height += 2.0;
    SetHeight(height);
    TModelUnit width = m_Context->ScreenToSeq(gl.TextWidth(&(m_Config->m_Font), m_Comment.c_str()));
    if (m_Config->m_ShowConnection) {
        width += m_Context->ScreenToSeq(kDotSize * 1.5);
    }
    if (m_Repeated) { // Override width to span the screen
        width = m_Context->GetViewWidth();
    }
    SetWidth(width);
    TModelUnit left = 0.0;
    if (m_LinkedGlyph) {
        const TSeqRange& range(m_LinkedGlyph->GetRange());
        left = (range.GetFrom() + range.GetTo()) * 0.5;
    } else {
        left = m_TargetPos.X();
    }

    if (m_Config->m_Centered) {
        left -= width * 0.5;
    }

    if (m_Config->m_ShowConnection) {
        left -= m_Context->ScreenToSeq(kDotSize * 0.5);
    }
    SetLeft(left);
}


END_NCBI_SCOPE
