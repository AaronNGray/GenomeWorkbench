/*  $Id: scatter_glyph.cpp 41823 2018-10-17 17:34:58Z katargir $
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
* Authors:  Vlad Lebedev, Liangshou Wu, Dmitry Rudnev
*
* File Description:
 *    CScatterPlotGlyph -- utility class for having
 *                      bin-based scatter plots in graphical layouts.
*
*/

#include <ncbi_pch.hpp>
#include <gui/packages/pkg_snp/bins/bins_track.hpp>
#include <gui/packages/pkg_snp/bins/scatter_glyph.hpp>
#include <gui/widgets/seq_graphic/rendering_ctx.hpp>
#include <gui/widgets/seq_graphic/config_utils.hpp>
#include <gui/objutils/obj_fingerprint.hpp>
#include <gui/objutils/utils.hpp>
#include <gui/opengl/irender.hpp>
#include <gui/opengl/glutils.hpp>
#include <gui/opengl/gltexturefont.hpp>
#include <gui/widgets/wx/message_box.hpp>
#include <gui/objutils/snp_gui.hpp>




BEGIN_NCBI_SCOPE
USING_SCOPE(objects);


//
// CScatterPlotGlyph::CScatterPlotGlyph()
//
CScatterPlotGlyph::CScatterPlotGlyph(const TMap& map, const CSeq_loc& loc)
    : m_Location(&loc)
    , m_Map(map)
{
    ComputeAxisRange();
}

void CScatterPlotGlyph::ComputeAxisRange()
{
    m_AxisMin = DBL_MAX;
    m_AxisMax = DBL_MIN;

    const size_t numBins(m_Map.GetBins());
    for(size_t bin = 0;  bin < numBins; ++bin) {
        if (m_Map[bin].obj.NotEmpty()) {
            ITERATE(objects::NSnpBins::TBinEntryList, iEntryList, m_Map[bin].obj->m_EntryList) {
                if((*iEntryList)->pvalue < m_AxisMin) {
                    m_AxisMin = (*iEntryList)->pvalue;
                }
                if((*iEntryList)->pvalue > m_AxisMax) {
                    m_AxisMax = (*iEntryList)->pvalue;
                }
            }
        }
    }

    m_AxisMin = floor(m_AxisMin);
    m_AxisMax = ceil(m_AxisMax);
}

CScatterPlotGlyph::~CScatterPlotGlyph()
{
}



bool CScatterPlotGlyph::OnLeftDblClick(const TModelPoint& /*p*/)
{
    return true;
}


bool CScatterPlotGlyph::NeedTooltip(const TModelPoint& p, ITooltipFormatter& tt, string& t_title) const
{
    GetTooltip(p, tt, t_title);
    return !tt.IsEmpty();
}


void CScatterPlotGlyph::GetTooltip(const TModelPoint& /*p*/, ITooltipFormatter& tt, string& /*t_title*/) const
{
    tt.AddRow(x_GetDescription());
}


void CScatterPlotGlyph::GetHTMLActiveAreas(TAreaVector* p_areas) const
{
    CHTMLActiveArea area;
    CSeqGlyph::x_InitHTMLActiveArea(area);
    area.m_PositiveStrand = true;
    area.m_Descr = x_GetDescription();
    area.m_Flags =
                CHTMLActiveArea::fNoSelection |
                CHTMLActiveArea::fTooltipEmbedded |
                CHTMLActiveArea::fNoPin;

    area.m_Signature = "fake|";

    p_areas->push_back(area);
}


bool CScatterPlotGlyph::IsClickable() const
{
    return false;
}


string CScatterPlotGlyph::x_GetColorTheme() const
{
    // getting to the color scheme
    const CSeqGlyph* pImmediateParentGlyph(GetParent());
    const CSeqGlyph* pSecondParentGlyph(pImmediateParentGlyph->GetParent());

    //!! add a check for pSecondParentGlyph being indeed a CBinsTrack
    const CBinsTrack* pBinsTrack(reinterpret_cast<const CBinsTrack*>(pSecondParentGlyph));

    return pBinsTrack->GetGlobalConfig()->GetColorTheme();
}

string CScatterPlotGlyph::x_GetDescription() const
{
    return string("Scatter plot based on P-Values between " + NStr::DoubleToString(m_AxisMin) + " and " + NStr::DoubleToString(m_AxisMax));
}

void CScatterPlotGlyph::x_Draw() const
{
    IRender& gl = GetGl();
    const int Height = 4 * NSnpGui::c_BinHeight;

    TModelUnit top = GetTop();

    TModelUnit line_y1 = 1 + top;
    TModelUnit line_y2 = line_y1 + Height;

    // SETUP COMMON attributes
    const size_t        numBins = m_Map.GetBins();

    double AxisRange(m_AxisMax - m_AxisMin);
    // scaling factor of pixels per range unit
    double PixelScale(Height / AxisRange);

    TModelUnit x1 = 0.0;
    TModelUnit y1 = line_y1;

    CGlTextureFont font(CGlTextureFont::eFontFace_Helvetica, 8);
    TModelUnit font_height(font.GetMetric(IGlFont::eMetric_FullCharHeight));
	CRgbaColor colorBin;
	CRgbaColor colorText;
	string text;

    string sColorTheme(x_GetColorTheme());
    string sColorKey("Default");

    CGuiRegistry& registry(CGuiRegistry::GetInstance());
    CRegistryReadView ColorView(CSGConfigUtils::GetColorReadView(registry, "GBPlugins.BinTrack", "PValue", sColorTheme));

    // Render the colored dots for each pvalue and texts over them
    for(size_t bin = 0;  bin < numBins; bin++)
    {
        if (m_Map[bin].obj.NotEmpty()) {
            ITERATE(objects::NSnpBins::TBinEntryList, iEntryList, m_Map[bin].obj->m_EntryList) {
                double pvalue((*iEntryList)->pvalue);
                if(pvalue != 0) {
                    TSeqPos pos((*iEntryList)->pos);
                    sColorKey = NSnpGui::PValueAsColorKey(pvalue);

                    CSGConfigUtils::GetColor(ColorView, sColorKey, colorBin);
                    x1 = pos;
                    y1 = line_y2 - (pvalue - m_AxisMin) * PixelScale;

                    // Draw filled disc
                    gl.ColorC(colorBin);
                    m_Context->DrawDisk(TModelPoint(x1, y1), 2.0f);
                }
            }
        }
    }
    // draw the ruler(s) with min/max values
    CRgbaColor r_c("grey");
    gl.Color4f(r_c.GetRed(), r_c.GetGreen(), r_c.GetBlue(), 0.4f);
    // horizontal boundaries
    m_Context->DrawLine(GetLeft(), line_y1, GetRight(), line_y1);
    m_Context->DrawLine(GetLeft(), line_y2, GetRight(), line_y2);

    // vertical rulers
    TModelUnit dist_x = GetWidth() / 4;
    TModelUnit min_dist = m_Context->ScreenToSeq(300.0);
    if (dist_x < min_dist) {
        dist_x = min_dist;
    }
    TModelUnit label_off = m_Context->ScreenToSeq(1.0);

    TModelUnit x = GetLeft() + dist_x * 0.1;
    for (;  x < GetRight();  x += dist_x) {
        gl.Color4f(r_c.GetRed(), r_c.GetGreen(), r_c.GetBlue(), 0.5f);

        m_Context->DrawLine(x, line_y1, x, line_y2);
        gl.Color4f(r_c.GetRed(), r_c.GetGreen(), r_c.GetBlue(), 1.0f);
        m_Context->TextOut(&font, NStr::DoubleToString(m_AxisMin).c_str(),
            x + label_off,
            line_y2 - 1, false);

        m_Context->TextOut(&font, NStr::DoubleToString(m_AxisMax).c_str(),
            x + label_off,
            line_y1 + font_height + 1, false);
    }
    gl.Color4f(0.0f, 0.0f,0.0f, 1.0f);
}

void CScatterPlotGlyph::x_UpdateBoundingBox()
{
    SetHeight(4 * NSnpGui::c_BinHeight);
    TSeqRange range = GetRange();
    SetWidth(range.GetLength());
    SetLeft(range.GetFrom());
}

TSeqRange CScatterPlotGlyph::GetRange(void) const
{
    return m_Location->GetTotalRange();
}



END_NCBI_SCOPE
