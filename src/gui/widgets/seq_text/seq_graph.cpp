/*  $Id: seq_graph.cpp 41823 2018-10-17 17:34:58Z katargir $
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
 * Authors:  Colleen Bollin (adapted from a file by Andrey Yazhuk)
 *
 * File Description:
 *
 */

#include <ncbi_pch.hpp>
#include <corelib/ncbistl.hpp>
#include <gui/widgets/seq_text/seq_graph.hpp>

#include <gui/objutils/utils.hpp>
#include <gui/objutils/label.hpp>
#include <gui/opengl/glhelpers.hpp>
#include <gui/opengl/irender.hpp>

#include <objmgr/seq_vector.hpp>
#include <objmgr/util/sequence.hpp>

#include <math.h>

BEGIN_NCBI_SCOPE
USING_SCOPE(objects);

CSequenceTextGraph::CSequenceTextGraph()
:   m_pSeqFont(NULL),
    m_BackColor(0.9f, 0.9f, 0.9f),
    m_TextColor(0.0f, 0.0f, 0.0f),
    m_FeatColor(0.0f, 0.6f, 0.0f),
    m_SpliceBoxColor(0.5f, 0.5f, 0.5f),
    m_TextHeight(0),
    m_TextOffset(0),
    m_pHost(NULL),
    m_pGeometry(NULL)
{
}

void    CSequenceTextGraph::SetFont(CGlTextureFont* seq_font)
{
    m_pSeqFont = seq_font;
}

void CSequenceTextGraph::SetDataSource(CSeqTextDataSource* p_ds)
{
    m_DataSource = p_ds;
}


void CSequenceTextGraph::SetConfig(CSeqTextConfig *p_cfg)
{
    m_Config = p_cfg;
}


void CSequenceTextGraph::SetHost(ISequenceTextGraphHost* pHost)
{
    m_pHost = pHost;
}


void CSequenceTextGraph::SetGeometry(ISeqTextGeometry *pGeometry)
{
    m_pGeometry = pGeometry;
}


static const int kMargin = 2;
static const int kTextOffset = 2;

TVPPoint CSequenceTextGraph::PreferredSize()
{
    int spacing = 2 * kMargin;
    TVPPoint size(spacing, spacing);

    TModelUnit seq_h = m_pSeqFont->GetMetric(CGlTextureFont::eMetric_FullCharHeight);
    size.m_Y += 2 * kTextOffset + (int) ceil(seq_h);
    return size;
}


void
CSequenceTextGraph::x_BoxCharacters
(CSeqTextDefs::TSpliceSiteVector splice_sites,
 TSeqPos seq_start, TSeqPos num_chars_per_row,
 TModelUnit offset_X, TModelUnit offset_Y)
{
    IRender& gl = GetGl();

    unsigned int i;
    TModelUnit   x1, x2, y1, y2;

    /* use a fine line to indicate feature location */
    gl.LineWidth (0.5);
    gl.ColorC(m_SpliceBoxColor);
    for (i = seq_start; i < seq_start + num_chars_per_row; i++) {
        if (splice_sites [i]) {
            x1 = offset_X + i - seq_start - 0.5;
            x2 = offset_X + i - seq_start + 0.5;
            y1 = offset_Y - m_TextOffset;
            y2 = offset_Y + m_TextHeight - m_TextOffset;
            gl.Begin (GL_LINES);
            gl.Vertex2d(x1, y1);
            gl.Vertex2d(x2, y1);

            gl.Vertex2d(x2, y1);
            gl.Vertex2d(x2, y2);

            gl.Vertex2d(x2, y2);
            gl.Vertex2d(x1, y2);

            gl.Vertex2d(x1, y2);
            gl.Vertex2d(x1, y1);
            gl.End();
        }
    }
}


void
CSequenceTextGraph::x_GetColors
(CSeqTextDefs::TSubtypeVector &subtypes,
 TColorVector &colors,
 TSeqPos seq_start, TSeqPos num_chars_per_row)
{
    unsigned int i;

    colors.clear();
    for (i = seq_start; i < subtypes.size() && i - seq_start < num_chars_per_row; i++) {
        if (m_Config && subtypes[i] != CSeqFeatData::eSubtype_bad)
        {
            colors.push_back (m_Config->GetColor(subtypes[i]));
        }
        else
        {
            colors.push_back (&m_TextColor);
        }
    }
}


void CSequenceTextGraph::x_DrawIntervalBreaks (TModelUnit x, TModelUnit y,
 CSeqTextDefs::TSeqPosVector &breaks,
 TSeqPos seq_start, TSeqPos num_chars_per_row)
{
    IRender& gl = GetGl();

    unsigned int i;

    for (i = 0; i < breaks.size(); i++) {
        if (breaks[i] >= seq_start && breaks[i] <= seq_start + num_chars_per_row) {
            gl.ColorC (m_SpliceBoxColor);
            gl.Begin(GL_LINES);
            gl.Vertex2d (x + breaks[i] - seq_start - 0.5, y);
            gl.Vertex2d (x + breaks[i] - seq_start - 0.5, y + m_TextHeight - m_TextOffset);

            gl.Vertex2d (x + breaks[i] - seq_start, y + m_TextHeight - m_TextOffset);
            gl.Vertex2d (x + breaks[i] - seq_start - 1, y + m_TextHeight - m_TextOffset);
            gl.End();
        }
    }
}


void
CSequenceTextGraph::x_DrawVariations
(TSeqPos seq_start,
 TSeqPos num_chars_per_row,
 float scale_x,
 float scale_y,
 CSeqTextDefs::TVariationGraphVector &variations)
{
    unsigned int first_var = 0, i;
    TColorVector colors;
    CRgbaColor* variation_color;

    if (!m_Config) {
        return;
    }

    for (i = 0; i < variations.size(); i++) {
        for (unsigned int k = i + 1;
             k < variations.size() && variations[k].GetFeatLeft() < variations[i].GetFeatLeft() + variations[i].GetDrawWidth();
             k++) {
            variations[k].SetDisplayLine(max (variations[i].GetDisplayLine() + 1, variations[k].GetDisplayLine()));
        }
    }

    while (first_var < variations.size() && variations[first_var].GetFeatRight() < seq_start) {
        first_var++;
    }

    if (first_var == variations.size()) {
        return;
    }

    colors.clear();
    // start with 5 - add more if a longer variation is encountered
    variation_color = m_Config->GetColor(CSeqFeatData::eSubtype_variation);
    colors.push_back (variation_color);
    colors.push_back (variation_color);
    colors.push_back (variation_color);
    colors.push_back (variation_color);
    colors.push_back (variation_color);

    while (first_var < variations.size() && variations[first_var].GetFeatLeft() < seq_start + num_chars_per_row) {
        TSeqPos varlinestart = max (seq_start, variations[first_var].GetFeatLeft());
        TSeqPos varlinestop = min (seq_start + num_chars_per_row - 1,
                                   variations[first_var].GetFeatLeft() + variations[first_var].GetDrawWidth());
        if (varlinestart <= varlinestop) {
            TModelPoint seq_left = m_pGeometry->STG_GetModelPointBySourcePos(varlinestart);
            TModelPoint seq_right = m_pGeometry->STG_GetModelPointBySourcePos(varlinestop);
            //TSeqPos draw_width = min (variations[first_var].GetDrawWidth(), varlinestop - varlinestart + 1);
            seq_left.m_Y -= variations[first_var].GetDisplayLine();
            seq_right.m_Y -= variations[first_var].GetDisplayLine();

            // print variation text
            TSeqPos var_start = 0;
            if (varlinestart > variations[first_var].GetFeatLeft()) {
                var_start = varlinestart - variations[first_var].GetFeatLeft();
            }
            // add extra colors to vector as needed
            while (colors.size() < varlinestop - varlinestart + 1) {
                colors.push_back (variation_color);
            }
            m_pSeqFont->ArrayTextOut((float) seq_left.m_X  + 0.5, (float) seq_left.m_Y, 1.0f, 0.0f,
                                      variations[first_var].GetText().substr(var_start, varlinestop - varlinestart + 1).c_str(),
                                      &colors,
                                      scale_x, scale_y);
        }
        first_var++;
    }
}


void    CSequenceTextGraph::x_RenderSequence(CGlPane& pane)
{
    IRender& gl = GetGl();

    if (m_DataSource)
    {
        _ASSERT(m_pSeqFont);
        _ASSERT(m_pGeometry);

        TSeqPos chars_in_line, lines_in_seq;
        m_pGeometry->STG_GetLineInfo(chars_in_line, lines_in_seq);
        if (chars_in_line < 1) {
            return;
        }

        TModelUnit scale_x = pane.GetScaleX();
        TModelUnit scale_y = pane.GetScaleY();

        if (scale_x < 0) return;

        m_TextHeight = scale_y * m_pSeqFont->GetMetric(CGlTextureFont::eMetric_FullCharHeight);
        m_TextOffset = m_TextHeight - scale_y * m_pSeqFont->GetMetric(CGlTextureFont::eMetric_CharHeight);


        // fill background
        gl.PolygonMode(GL_FRONT_AND_BACK, GL_FILL);
/*
        TVPRect rc_back(m_VPRect);
        rc_back.Inflate(0, -kMargin);

        {
            CGlPaneGuard GUARD(pane, CGlPane::ePixels);
            gl.ColorC(m_BackColor);
            gl.Rectd(rc_back.Left(), rc_back.Bottom(), rc_back.Right(), rc_back.Top());
        } 
*/
        // draw sequence
        CGlPaneGuard GUARD(pane, CGlPane::eOrtho);
        gl.ColorC(m_TextColor);

        TColorVector colors;
        colors.reserve(chars_in_line);

        CSeqTextDefs::TSubtypeVector subtypes;
        CSeqTextDefs::TSpliceSiteVector splice_sites;
        CSeqTextDefs::TSeqPosVector breaks;
        CSeqTextDefs::TVariationGraphVector variations;
        string seq;

        breaks.clear();
        variations.clear();
        string substr;
        // if this is an mRNA Bioseq, color exons alternately
        bool   color_exons_for_mrna = true;

        TSeqPos  vis_start, vis_stop;
        m_pGeometry->STG_GetVisibleRange (vis_start, vis_stop);
        subtypes.reserve((vis_stop - vis_start) + 1);
        splice_sites.reserve((vis_stop - vis_start) + 1);

        // get sequence data and feature data for entire pane
        m_DataSource->GetFeatureData (vis_start, vis_stop, m_Config, m_pGeometry, subtypes, splice_sites, variations);

        // if choice is selected or mouseover, recalculate subtypes
        if (m_pHost && m_Config) {
            CSeqTextPaneConfig::EFeatureDisplayType disp = m_Config->GetFeatureColorationChoice();
            if (disp == CSeqTextPaneConfig::eSelected) {
                m_pHost->STGH_GetSelectedFeatureSubtypes(subtypes);
                color_exons_for_mrna = false;
            } else if (disp == CSeqTextPaneConfig::eMouseOver) {
                m_pHost->STGH_GetMouseOverFeatureSubtypes(subtypes);
                color_exons_for_mrna = false;
            }
        }

        if (color_exons_for_mrna) {
            m_DataSource->GetSubtypesForAlternatingExons (vis_start, vis_stop, m_pGeometry, subtypes);
        }

        // mark breaks in sequence
        m_DataSource->GetIntervalBreaks (vis_start > 0 ? vis_start - 1 : vis_start, vis_stop, breaks);

        // show "case feature" in different case
        m_DataSource->GetSeqData (vis_start, vis_stop, seq,
                                  m_Config ? m_Config->GetCaseFeature() : NULL,
                                  m_Config ? m_Config->GetShowFeatAsLower() : false);

        TSeqPos row_start = vis_start;
        TSeqPos len = m_DataSource->GetDataLen();
        while (row_start < vis_stop && row_start < len)
        {
            TModelPoint start_point = m_pGeometry->STG_GetModelPointBySourcePos (row_start);
            start_point.m_X += 0.5;
            
            // draw boxes around splice junction characters
            x_BoxCharacters (splice_sites, row_start - vis_start, chars_in_line, start_point.m_X, start_point.m_Y);

            // draw lines to indicate discontigous sequence
            x_DrawIntervalBreaks (start_point.m_X, start_point.m_Y, breaks, row_start, chars_in_line);

            // show features in foreground color
            colors.clear();
            x_GetColors (subtypes, colors, row_start - vis_start, chars_in_line);

            substr = seq.substr (row_start - vis_start, chars_in_line);
            m_pSeqFont->ArrayTextOut((float) start_point.m_X, (float) start_point.m_Y, 1.0f, 0.0f,
                                     substr.c_str(), &colors,
                                     (float) scale_x, (float) scale_y);

            // draw variations
            x_DrawVariations (row_start, chars_in_line, (float) scale_x, (float) scale_y, variations);
            
            row_start += chars_in_line;
        }
    }
}


void    CSequenceTextGraph::Render(CGlPane& pane)
{
    x_RenderSequence (pane);
}


bool    CSequenceTextGraph::NeedTooltip(CGlPane& /*pane*/, int vp_x, int vp_y)
{
    return m_VPRect.PtInRect(vp_x, vp_y);
}


string  CSequenceTextGraph::GetTooltip()
{
    if (m_DataSource)   {
        return m_DataSource->GetTitle();
    } else  {
      return "No sequence loaded";
    }
}


TModelRect CSequenceTextGraph::GetModelRect() const
{
    // TODO initialize m_ModelRect in the place where Data Source is set
    // then use m_ModelRect here
    TSeqPos len = 0;

    if (m_DataSource)   {
        CSequenceTextGraph* nc_this = const_cast<CSequenceTextGraph*>(this);
        len = nc_this->m_DataSource->GetDataLen();
    }
    TModelRect rc_model(0.0, 0.0, len, 1.0);
    return rc_model;
}


void CSequenceTextGraph::SetModelRect(const TModelRect& rc)
{
    _ASSERT(false); // not supported
}


END_NCBI_SCOPE
