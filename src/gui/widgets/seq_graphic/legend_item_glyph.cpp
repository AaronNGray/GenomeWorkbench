/*  
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
 * Authors:  Andrei Shkeda
 *
 * File Description:
 */

#include <ncbi_pch.hpp>
#include <gui/widgets/seq_graphic/legend_item_glyph.hpp>

#include <gui/opengl/irender.hpp>
#include <gui/widgets/seq_graphic/rendering_ctx.hpp>

BEGIN_NCBI_SCOPE
USING_SCOPE(objects);

static const int kLegendBarWidth = 30;
static const int kLegendLabelGap = 5;


CLegendItemGlyph::CLegendItemGlyph(const string& source_name, const string& label, const string& id, 
                                   const CRgbaColor& legend_color, const CRgbaColor& text_color) 
    : m_SourceName(source_name)
    , m_Label(label)
    , m_Id(id)
    , m_LegendColor(legend_color)
    , m_TextColor(text_color)
{
}

bool CLegendItemGlyph::OnLeftDblClick (const TModelPoint &)
{   
    if (!m_LHost)
        return false;
    m_LHost->LH_OnItemDblClick(m_SourceName);
    return true;
}

bool CLegendItemGlyph::OnContextMenu(wxContextMenuEvent&)
{
    if (!m_LHost)
        return false;
    m_LHost->LH_OnItemRightClick(m_SourceName);
    return true;
}

void CLegendItemGlyph::GetHTMLActiveAreas(TAreaVector* p_areas) const
{
    CHTMLActiveArea area;

    TVPUnit x1 = GetLeft();
    TVPUnit x2 = GetRight();
    TModelUnit t_x = 0;
    TModelUnit t_y = 0;
    x_Local2World(t_x, t_y);
    // Round y values to nearest integer and add 2 to the height
    // to give some space around the glyph
    TVPUnit y1 = TVPUnit(floor(t_y + 0.5));
    TVPUnit y2 = TVPUnit(floor(t_y + GetHeight() + 2.5));
    area.m_Bounds.Init(x1, y2, x2, y1);
    area.m_SeqRange.Set(m_Context->GetVisSeqFrom(), m_Context->GetVisSeqTo());
    area.m_Flags =  CHTMLActiveArea::fLegendItem 
        | CHTMLActiveArea::fNoSelection 
        | CHTMLActiveArea::fNoTooltip 
        | CHTMLActiveArea::fNoPin;
    area.m_ID = m_Id;
    area.m_Signature = m_Label;
    area.m_Color = m_LegendColor.ToString();
    p_areas->push_back(area);

}

void CLegendItemGlyph::x_Draw() const 
{
    IRender& gl = GetGl();
    gl.ColorC(m_LegendColor);
    TModelUnit x1 = GetLeft();
    TModelUnit x2 = x1 + m_Context->ScreenToSeq(kLegendBarWidth);
    m_Context->DrawQuad(x1, GetTop(), x2, GetBottom());
    x2 += m_Context->ScreenToSeq(kLegendLabelGap);
    //_ASSERT(m_gConfig);
    //if (!m_gConfig)
        //return;
    CRef<CGlTextureFont> font = m_gConfig 
        ? m_gConfig->GetLabelFont() : Ref(new CGlTextureFont(CGlTextureFont::eFontFace_Helvetica, 8));

    _ASSERT(font);
    if (!font)
        return;
    gl.ColorC(m_TextColor);
    m_Context->TextOut (&*font, m_Label.c_str(), x2, GetBottom(), false);
};




END_NCBI_SCOPE
