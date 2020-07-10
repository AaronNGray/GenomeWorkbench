/*  $Id: vis_range_glyph.cpp 41823 2018-10-17 17:34:58Z katargir $
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
 * Authors:  Vlad Lebedev, Liangshou Wu
 *
 * File Description:
 *   CVisRangeGlyph -- utility class to layout visible ranges of the
 *                     sibling views
 */

#include <ncbi_pch.hpp>
#include <gui/widgets/seq_graphic/vis_range_glyph.hpp>
#include <gui/widgets/seq_graphic/rendering_ctx.hpp>
#include <gui/opengl/irender.hpp>
#include <gui/objutils/tooltip.hpp>


BEGIN_NCBI_SCOPE

USING_SCOPE(objects);

TModelUnit kVisRangeBarHeight = 4.0;

CVisRangeGlyph::CVisRangeGlyph(const CEventHandler* target,
                               const list<TSeqRange>& vis_ranges,
                               const CRgbaColor& color,
                               const string& label)
        : m_Target(target)
        , m_Color(color)
        , m_Label(label)
{
    if ( !vis_ranges.empty() ) {
        TSeqRange r = vis_ranges.front();
        m_Location.Reset(new CSeq_loc());
        m_Location->SetInt().SetFrom(r.GetFrom());
        m_Location->SetInt().SetTo  (r.GetTo());
    } else {
        _ASSERT(false);
        NCBI_THROW(CException, eUnknown, "Empty visible range.");
    }
}


TSeqRange CVisRangeGlyph::GetRange(void) const
{
    return m_Location->GetTotalRange();
}


bool CVisRangeGlyph::NeedTooltip(const TModelPoint& p, ITooltipFormatter& tt, string& t_title) const
{
    GetTooltip(p, tt, t_title);
    return true;
}


void CVisRangeGlyph::GetTooltip(const TModelPoint& /*p*/, ITooltipFormatter& tt, string& /*t_title*/) const
{
    tt.AddRow(m_Label);
}


bool CVisRangeGlyph::IsClickable() const
{
    return false;
}


void CVisRangeGlyph::x_Draw() const
{
    IRender& gl = GetGl();

    TModelRect rcm  = GetModelRect();
    gl.ColorC(m_Color);
    m_Context->DrawQuad(rcm);
}


void CVisRangeGlyph::x_UpdateBoundingBox()
{
    SetLeft(GetRange().GetFrom());
    SetHeight(kVisRangeBarHeight);
    SetWidth(GetRange().GetLength());
}


END_NCBI_SCOPE
