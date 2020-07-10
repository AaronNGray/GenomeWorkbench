/*  $Id: ld_glyph.cpp 41823 2018-10-17 17:34:58Z katargir $
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
#include <gui/utils/app_popup.hpp>
#include <gui/widgets/seq_graphic/rendering_ctx.hpp>
#include <gui/widgets/wx/message_box.hpp>
#include <gui/objutils/tooltip.hpp>
#include <gui/opengl/gltexturefont.hpp>
#include <gui/opengl/irender.hpp>
#include <objmgr/util/sequence.hpp>
#include <util/xregexp/regexp.hpp>
#include <objects/general/User_object.hpp>

BEGIN_NCBI_SCOPE
USING_SCOPE(objects);

static string s_ChrName(const CSeq_loc& loc, CScope* scope)
{
    string acc, chr;
    CRegexp     rgxChr("(\\d\\d)(\\.\\d+)?$");          // find a non blank, non-comment line

    const CSeq_id& id = *loc.GetId();
    if (id.IsGi())
        acc = sequence::GetAccessionForGi(id.GetGi(),*scope);
    else if (id.IsOther())
        acc = id.GetOther().GetAccession();
    else
        chr = "unknown";

    if (rgxChr.IsMatch(acc)) {
        string sub = rgxChr.GetSub(acc,1);
        int v = NStr::StringToInt(sub);
        chr = "chr" + NStr::IntToString(v);
    }
    return chr;
}

//
// CLDBlockGlyph::CLDBlockGlyph()
//
CLDBlockGlyph::CLDBlockGlyph(const CMappedFeat& feat)
{
    const CSeq_feat& or_feat = feat.GetOriginalFeature();
    const CUser_object &user = or_feat.GetData().GetUser();

    m_Value.blockId = user.GetField("block_id").GetData().GetInt();
    m_Value.score = user.GetField("score").GetData().GetReal();
    m_Value.popId = user.GetField("pop_id").GetData().GetInt();

    m_Location.Reset(&feat.GetLocation());
}

bool CLDBlockGlyph::OnLeftDblClick(const TModelPoint& /*p*/)
{
    string url, url_base, start, stop, chr;
    url_base = "https://hapmap1.be-md.ncbi.nlm.nih.gov/cgi-perl/gbrowse/hapmap27_B36/?";

    TSeqRange range = GetRange();
    start = "start=" + NStr::IntToString(range.GetFrom());
    stop  = ";stop=" + NStr::IntToString(range.GetToOpen());
    chr   = ";ref="  + s_ChrName(*m_Location, &(m_Context->GetScope()));

    url = url_base + start + stop + chr;

    CAppPopup::PopupURL(url);
    return true;
}


bool CLDBlockGlyph::NeedTooltip(const TModelPoint& p, ITooltipFormatter& tt, string& t_title) const
{
    GetTooltip(p, tt, t_title);
    return true;
}


void CLDBlockGlyph::GetTooltip(const TModelPoint& /*pos*/, ITooltipFormatter& tt, string& /*t_title*/) const
{
    tt.AddRow("Score:", NStr::DoubleToString(m_Value.score));
    tt.AddRow("Pop ID:", NStr::IntToString(m_Value.popId));
}


bool CLDBlockGlyph::IsClickable() const
{
    // TODO
    return true;
}

void CLDBlockGlyph::x_Draw() const
{
    const int binHeight = 10; //m_ConfigSettings->GetGwasBinHeight();

    IRender& gl = GetGl();

    TModelUnit top = GetTop();
    TModelUnit bottom = top + GetHeight();
    TSeqRange  range  = GetRange();

    TModelUnit line_y1 = 1 + top;
    TModelUnit line_y2 = line_y1 + binHeight;

    // SETUP COMMON attributes
    const TModelUnit    offset  = m_Context->GetOffset();

    TModelUnit x1 = range.GetFrom();
    TModelUnit x2 = range.GetToOpen();
    TModelUnit y1 = line_y1;
    TModelUnit y2 = line_y2;

    TModelUnit dx1 = x1 - offset;
    TModelUnit dx2 = x2 - offset;

    TModelUnit xx1 = max<TModelUnit>(0, dx1);
    TModelUnit xx2 = max<TModelUnit>(0, dx2);

    float cScore = 1 - m_Value.score;

    // Calculate the color
    unsigned int dc = (unsigned int)((cScore * 9)+.5);
    dc = min<unsigned int>(dc, 6);
    unsigned char B = 225 + (5 *dc);
    unsigned char G = 32  + (32*dc);
    unsigned char R = 32  + (32*dc);

    CRgbaColor color(R,G,B);
    CRgbaColor color2 = color;
    color2.Lighten(0.30f);

    TModelUnit mid = y1 + (y2 - y1) * 0.5f;

    gl.ShadeModel(GL_SMOOTH);
    gl.Begin(GL_TRIANGLE_STRIP);
        gl.ColorC(color);
        gl.Vertex2d(xx1, y1);
        gl.Vertex2d(xx2, y1);

        gl.ColorC(color2);
        gl.Vertex2d(xx1, mid);
        gl.Vertex2d(xx2, mid);

        gl.ColorC(color);
        gl.Vertex2d(xx1, y2);
        gl.Vertex2d(xx2, y2);
    gl.End();
    gl.ShadeModel(GL_FLAT);

    // draw border
    gl.Color3f(0,0,.5f);
    m_Context->DrawRect(x1, y1, x2, y2);

    // draw label
    CGlTextureFont font(CGlTextureFont::eFontFace_Helvetica_Bold, 8);
    TModelUnit midx = x1 + (x2 - x1) * 0.5f;
    string label = "LD" + NStr::IntToString(m_Value.blockId);

    if (m_Context->WillLabelFit(range, font, label)) {
        gl.Color3f(1,1,1);
        m_Context->TextOut(&font, label.c_str(), midx,
            bottom, true);
    }

}

void CLDBlockGlyph::x_UpdateBoundingBox()
{
    SetHeight(10);
    TSeqRange range = GetRange();
    SetWidth(range.GetLength());
    SetLeft(range.GetFrom());
}

TSeqRange CLDBlockGlyph::GetRange(void) const
{
    return m_Location->GetTotalRange();
}

END_NCBI_SCOPE
