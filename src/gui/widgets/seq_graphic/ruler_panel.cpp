/*  $Id: ruler_panel.cpp 41823 2018-10-17 17:34:58Z katargir $
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
#include <gui/widgets/seq_graphic/ruler_panel.hpp>
#include <gui/widgets/seq_graphic/seqgraphic_conf.hpp>
#include <gui/widgets/seq_graphic/config_utils.hpp>
#include <gui/opengl/glpane.hpp>
#include <gui/opengl/glutils.hpp>
#include <gui/opengl/glresmgr.hpp>
#include <gui/opengl/glstate.hpp>
#include <gui/objutils/registry.hpp>



BEGIN_NCBI_SCOPE

static const string kRulerPanelKey = "GBPlugins.RulerPanel";

CRulerPanel::CRulerPanel() 
    : m_SeqLength(1)
    , m_RulerStart(0)
    , m_Flipped(false)
    , m_Horz(true)
    , m_Shown(true)
    , m_BG(1.0f, 1.0f, 1.0f)
    , m_GridColor(0.0f, 0.0f, 0.0f)
    , m_LabelColor(0.0f, 0.0f, 1.0f)
    , m_LabelFont(CGlTextureFont::eFontFace_Helvetica, 12)
    , m_SmallLabelFont(CGlTextureFont::eFontFace_Helvetica, 8)
    , m_MajorTickSize(0)
    , m_MinorTickSize(0)
    , m_LabelTickSize(0)
    , m_OppMajorTickSize(3)
    , m_OppMinorTickSize(2)
    , m_OppLabelTickSize(6)
    , m_LabelAln(CRuler::eAln_Center)
{
    // initialize the main ruler
    x_InitRulerSpecs();
}


CRulerPanel::~CRulerPanel()
{
}


void CRulerPanel::SetOrientation(bool horz, bool flip)
{
    m_Horz = horz;
    m_Flipped = flip;
    m_MainRuler.SetHorizontal(horz, horz ? CRuler::eTop : CRuler::eRight, m_LabelAln);
    if (m_RulerStart > 0  &&  m_ExtraRuler.get()) {
        m_ExtraRuler->SetHorizontal(horz, horz ? CRuler::eBottom : CRuler::eLeft,
            m_LabelAln);
        if (m_ExtraNegRuler.get()) {
            m_ExtraNegRuler->SetHorizontal(horz, horz ? CRuler::eBottom : CRuler::eLeft,
                m_LabelAln);
        }
    }
    x_UpdateMappingRanges();
}


TVPPoint CRulerPanel::GetSize(int max_num) const
{
    if ( !m_Shown ) {
        return TVPPoint(0, 0);
    }
    TVPPoint pt = m_MainRuler.GetPreferredSize(max_num);
    if (m_RulerStart > 0  &&  m_ExtraRuler.get()) {
        TVPPoint pt2 = m_ExtraRuler->GetPreferredSize(max_num);
        if (m_Horz) {
            pt.m_X = max(pt.X(), pt2.X());
            pt.m_Y += pt2.Y();
        } else {
            pt.m_X += pt2.X();
            pt.m_Y = max(pt.Y(), pt2.Y());
        }
    }
    return pt;
}


void CRulerPanel::SetRulerSeqStart(TSeqPos pos)
{
    m_RulerStart = pos;

    // for case that we need two ruler bars 
    if (m_RulerStart > 0  &&  !m_ExtraRuler.get()) {
        m_ExtraRuler.reset(new CRuler());
        m_ExtraRuler->SetGeometryParam(CRuler::eMajorTickHeight, 0);
        m_ExtraRuler->SetGeometryParam(CRuler::eMinorTickHeight, 0);
        m_ExtraRuler->SetGeometryParam(CRuler::eLabelTickHeight, 1);
        m_ExtraRuler->SetGeometryParam(CRuler::eOppMajorTickHeight, 0);
        m_ExtraRuler->SetGeometryParam(CRuler::eOppMinorTickHeight, 0);
        m_ExtraRuler->SetGeometryParam(CRuler::eOppLabelTickHeight, 2);
        m_ExtraRuler->SetHorizontal(true, CRuler::eBottom, CRuler::eAln_Right);
        m_ExtraRuler->SetColor(CRuler::eBackground, m_BG);
        m_ExtraRuler->SetColor(CRuler::eRuler, m_GridColor);
        m_ExtraRuler->SetColor(CRuler::eText,  m_LabelColor);
        m_ExtraRuler->SetFont(m_SmallLabelFont.GetFontFace(), m_SmallLabelFont.GetFontSize());
        //m_ExtraRuler->SetDisplayOptions(CRuler::fHideNegative);
        m_ExtraRuler->SetDisplayOptions(0);

        // extra ruler for negative side
        m_ExtraNegRuler.reset(new CRuler());
        m_ExtraNegRuler->SetGeometryParam(CRuler::eMajorTickHeight, 0);
        m_ExtraNegRuler->SetGeometryParam(CRuler::eMinorTickHeight, 0);
        m_ExtraNegRuler->SetGeometryParam(CRuler::eLabelTickHeight, 1);
        m_ExtraNegRuler->SetGeometryParam(CRuler::eOppMajorTickHeight, 0);
        m_ExtraNegRuler->SetGeometryParam(CRuler::eOppMinorTickHeight, 0);
        m_ExtraNegRuler->SetGeometryParam(CRuler::eOppLabelTickHeight, 2);
        m_ExtraNegRuler->SetHorizontal(true, CRuler::eBottom, CRuler::eAln_Right);
        m_ExtraNegRuler->SetColor(CRuler::eRuler, CRgbaColor(0.5f, 0.1f, 0.1f, 1.0f));
        m_ExtraNegRuler->SetColor(CRuler::eText,  CRgbaColor(0.3f, 0.1f, 0.1f, 1.0f));
        m_ExtraNegRuler->SetFont(m_SmallLabelFont.GetFontFace(), m_SmallLabelFont.GetFontSize());
        m_ExtraNegRuler->SetDisplayOptions(CRuler::fHideLastLabel);
    }

    x_UpdateMappingRanges();
}


void CRulerPanel::Render(CGlPane& pane)
{
    if (m_Flipped) x_ReversePane(pane);

    if ( !m_Shown ) {
        /// still need to update the ruler to current 
        /// zoom level even it is not shown. Other fearures
        /// such as background grid, might depend on the
        /// ruler scale
        m_MainRuler.Update(pane);
        return ;
    }

    IRender& gl = CGlResMgr::Instance().GetCurrentRenderer().GetObject();
    CGlState state;

    pane.OpenOrtho();
    gl.ColorC(m_BG);
    TModelRect rc = pane.GetVisibleRect();
    TModelUnit offset_x = pane.GetOffsetX();
    TModelUnit offset_y = pane.GetOffsetY();
    rc.Offset(-offset_x, -offset_y);
    gl.RectC(rc);   
    pane.Close();

    if (m_RulerStart > 0  &&  m_ExtraRuler.get()) {
        TVPUnit main_h = m_MainRuler.GetPreferredSize().Y();
        TVPUnit extra_h = m_ExtraRuler->GetPreferredSize().Y();
        TVPUnit main_w = m_MainRuler.GetPreferredSize().X();
        TVPUnit extra_w = m_ExtraRuler->GetPreferredSize().X();
        pane.OpenOrtho();
        if (m_Horz) gl.Translatef(0.0f, -main_h, 0.0f);
        else  gl.Translatef(-main_w, 0.0f, 0.0f);
        pane.Close();
        if (m_ExtraNegRuler.get()) {
            m_ExtraNegRuler->Render(pane);
        }
        m_ExtraRuler->Render(pane);
        pane.OpenOrtho();
        if (m_Horz) gl.Translatef(0.0f, main_h + extra_h, 0.0f);
        else  gl.Translatef(main_w + extra_w, 0.0f, 0.0f);
        pane.Close();
    }
    m_MainRuler.Render(pane);

    // draw background
    if (m_RulerStart != 0) {
        CGlPaneGuard GUARD(pane, CGlPane::eOrtho);
        gl.Enable(GL_BLEND);
        gl.BlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        gl.Color4f(0.5f, 0.1f, 0.1f, 0.4f);

        TModelRect rect(m_Flipped ? m_SeqLength - m_RulerStart - 0.5 : 0.0,
            -1.0, m_Flipped ? m_SeqLength + 0.5 : m_RulerStart + 0.5,
            m_LabelTickSize);
        rect.Offset(-offset_x, -offset_y);
        gl.RectC(rect);
        gl.Disable(GL_BLEND);
    }

    if (m_Flipped) x_ReversePane(pane);

    /// this is necessary for two reasons:
    /// 1. We need to reset the modelview matrix so that it won't affect the position 
    ///    for other features and panels.
    /// 2. We need to update the projection matrix stored in CGlPane. The projection
    ///    matrix is used in calculating position calcuation for other features.
    /// 3. But you have to admit, this is a pretty dumb way to do it.
    pane.OpenOrtho();
    gl.LoadIdentity();
    pane.Close();
}


void CRulerPanel::GetHTMLActiveAreas(CSeqGlyph::TAreaVector* p_areas) const
{
    if ( !m_Shown ) return;

    CHTMLActiveArea area;
    TVPPoint size = GetSize();
    size.m_X = 10000; /// not important, but just need to be large enough
    area.m_Bounds.Init(0, size.Y() - 1, size.X(), 0);
    area.m_Flags =
        CHTMLActiveArea::fNoSelection |
        CHTMLActiveArea::fNoPin |
        CHTMLActiveArea::fNoHighlight |
        CHTMLActiveArea::fNoTooltip |
        CHTMLActiveArea::fRuler;
    area.m_Signature = "Ruler";
    p_areas->push_back(area);
}


void CRulerPanel::LoadSettings(const CSeqGraphicConfig& g_config)
{
    CGuiRegistry& registry = CGuiRegistry::GetInstance();
    CRegistryReadView view;
    view = registry.GetReadView(kRulerPanelKey);
    string label_aln = view.GetString("LabelAlign", "");
    if (NStr::EqualNocase(label_aln, "Center")) {
        m_LabelAln = CRuler::eAln_Center;
    } else if (NStr::EqualNocase(label_aln, "Left")) {
        m_LabelAln = CRuler::eAln_Left;
    } else if (NStr::EqualNocase(label_aln, "Right")) {
        m_LabelAln = CRuler::eAln_Right;
    }

    // load color settings
    view = CSGConfigUtils::GetColorReadView(
        registry, kRulerPanelKey, "", g_config.GetColorTheme());
    CSGConfigUtils::GetColor(view, "Background", m_BG);
    CSGConfigUtils::GetColor(view, "GridColor", m_GridColor);
    CSGConfigUtils::GetColor(view, "LabelColor", m_LabelColor);

    // load size settings
    view = CSGConfigUtils::GetSizeReadView(
        registry, kRulerPanelKey, "", g_config.GetSizeLevel());
    CSGConfigUtils::GetTexFont(view, "FontFace", "FontSize", m_LabelFont);
    m_MajorTickSize = view.GetInt("MajorTickSize", 0);
    m_MinorTickSize = view.GetInt("MinorTickSize", 0);
    m_LabelTickSize = view.GetInt("LabelTickSize", 2);
    m_OppMajorTickSize = view.GetInt("OppMajorTickSize", 3);
    m_OppMinorTickSize = view.GetInt("OppMinorTickSize", 2);
    m_OppLabelTickSize = view.GetInt("OppLabelTickSize", 6);

    x_InitRulerSpecs();
}


void CRulerPanel::SaveSettings() const
{
    // We don't expose settings to users currently. There shouldn't be
    // be any change that requires saving the settings.
}


// Set the ruler bar
void CRulerPanel::x_ReversePane(CGlPane& pane) const
{
    TModelRect rc;
    if (m_Horz) {
        TModelUnit left   = pane.GetVisibleRect().Left();
        TModelUnit right  = pane.GetVisibleRect().Right();
        rc = TModelRect (m_SeqLength - left, 0,
            m_SeqLength - right, pane.GetViewport().Height() );
    } else {
        TModelUnit top    = pane.GetVisibleRect().Top();
        TModelUnit bottom = pane.GetVisibleRect().Bottom();
        rc = TModelRect (0, m_SeqLength - bottom,
            pane.GetViewport().Width(), m_SeqLength - top);
    }
    pane.SetVisibleRect(rc);
}


void CRulerPanel::x_UpdateMappingRanges()
{
    /// case #1 single ruler 
    // Ruler with reset origin and follow the original sequence strand
    // m_MainRuler.SetRange(0, int(m_SeqLength - 1),
    //    -(int)(m_RulerStart), m_Flipped);

    /// case #2 single ruler 
    // Ruler with reset origin, but don't follow the 
    // original sequence strand (increase to the right always)
    // In flip strand mode, the origin is at -1, not 1.
    //TSignedSeqPos start =
    //    m_Flipped ? m_SeqLength - m_RulerStart - 1: m_RulerStart;
    //m_MainRuler.SetRange(0, int(m_SeqLength - 1), -(int)(start), false);

    /// case #3 double rulers
    // The main ruler without reset original,
    // but follow the original sequence strand
    m_MainRuler.SetRange(0, int(m_SeqLength - 1), 0, m_Flipped);
    if (m_RulerStart > 0  &&  m_ExtraRuler.get()) {
        // the extra ruler with reset original, but don't follow
        // the original sequence strand (increate to the right always)
        TSignedSeqPos start =
            m_Flipped ? m_SeqLength - m_RulerStart - 1 : m_RulerStart;
        //m_ExtraRuler->SetRange(0, int(m_SeqLength - 1), -start, false);
        m_ExtraRuler->SetRange(start, (int)(m_SeqLength - 1), 0, false);
        if (m_ExtraNegRuler.get()) {
            m_ExtraNegRuler->SetRange(0, (int)start, 0, true);
        }
    }
}


void CRulerPanel::x_InitRulerSpecs()
{
    m_MainRuler.SetGeometryParam(CRuler::eMajorTickHeight, m_MajorTickSize);
    m_MainRuler.SetGeometryParam(CRuler::eMinorTickHeight, m_MinorTickSize);
    m_MainRuler.SetGeometryParam(CRuler::eLabelTickHeight, m_LabelTickSize);
    m_MainRuler.SetGeometryParam(CRuler::eOppMajorTickHeight, m_OppMajorTickSize);
    m_MainRuler.SetGeometryParam(CRuler::eOppMinorTickHeight, m_OppMinorTickSize);
    m_MainRuler.SetGeometryParam(CRuler::eOppLabelTickHeight, m_OppLabelTickSize);

    m_MainRuler.SetColor(CRuler::eBackground, m_BG);
    m_MainRuler.SetColor(CRuler::eRuler, m_GridColor);
    m_MainRuler.SetColor(CRuler::eText,  m_LabelColor);
    m_MainRuler.SetFont(m_LabelFont.GetFontFace(), m_LabelFont.GetFontSize());
    m_MainRuler.SetHorizontal(m_Horz, m_Horz ? CRuler::eTop : CRuler::eRight, m_LabelAln);
    m_MainRuler.SetDisplayOptions(0);

    if (m_ExtraRuler.get()) {
        m_ExtraRuler->SetColor(CRuler::eBackground, m_BG);
        m_ExtraRuler->SetColor(CRuler::eRuler, m_GridColor);
        m_ExtraRuler->SetColor(CRuler::eText,  m_LabelColor);
        m_ExtraRuler->SetFont(m_SmallLabelFont.GetFontFace(),  m_SmallLabelFont.GetFontSize());
        m_ExtraRuler->SetHorizontal(m_Horz, m_Horz? CRuler::eBottom : CRuler::eLeft,
            m_LabelAln);
        m_ExtraRuler->SetDisplayOptions(0);
    }
}


END_NCBI_SCOPE
