/*  $Id: ld_line.cpp 41823 2018-10-17 17:34:58Z katargir $
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
* Authors:  Melvin Quintos
*
* File Description:
*    Melvin Quintos -- Handles the display of LD Blocks
*
*/

#include <ncbi_pch.hpp>
#include <gui/packages/pkg_snp/ld/ld_glyph.hpp>
#include <gui/packages/pkg_snp/ld/ld_line.hpp>
#include <gui/widgets/seq_graphic/rendering_ctx.hpp>
#include <gui/opengl/irender.hpp>
#include <gui/widgets/wx/message_box.hpp>


BEGIN_NCBI_SCOPE
USING_SCOPE(objects);

//
// CLDBlockLine::CLDBlockLine()
//
CLDBlockLine::CLDBlockLine(CRenderingContext* r_cntx)
    : CGlyphContainer(r_cntx)
{
}

void CLDBlockLine::x_RenderContent() const
{
    CGlyphContainer::x_RenderContent();
    x_DrawLabel();
}

void CLDBlockLine::x_DrawLabel() const
{
    IRender& gl = GetGl();

    TModelUnit base = m_Group.GetHeight()-5;
    TModelRange vis_r = m_Context->IntersectVisible(this);
    CConstRef<CSeqGraphicConfig> config = x_GetGlobalConfig();
    CRef<CGlTextureFont> font = config->GetLabelFont();
    TModelUnit font_h = gl.TextHeight(font.GetPointer());

    string content;
    content = GetTitle();
    
    TModelUnit label_w = gl.TextWidth(font, content.c_str());
    TModelUnit max_w = m_Context->GetMaxLabelWidth(*font);
    if (label_w > max_w) {
        label_w = max_w;
        content = font->Truncate(content.c_str(), label_w);
    }
    label_w = m_Context->ScreenToSeq(label_w + 3); // add 3 pixels of padding
    
    TModelRect rect(vis_r.GetFrom(), base + font_h * 0.5, 
        vis_r.GetFrom() + label_w, base - font_h * 0.5 - 2);
    
    gl.Color3f(1,1,1);
    m_Context->DrawBackground(rect, 0);

    gl.Color3f(46.0f,69.0f,100.0f);
    m_Context->TextOut(&*font, content.c_str(), rect.Left(),
        rect.Bottom() - 1.0, false);

}

/*
bool CLDBlockLine::OnLeftDblClick(const TModelPoint& p)
{
    string name;
    // TODO
    return true;
}


void CLDBlockLine::GetTooltip(const TModelPoint& pos, string& tt) const
{
}


bool CLDBlockLine::IsClickable() const
{
    // TODO
    return false;
}

TSeqRange CLDBlockLine::GetRange(void) const
{
    return m_Location->GetTotalRange();
}
*/

END_NCBI_SCOPE
