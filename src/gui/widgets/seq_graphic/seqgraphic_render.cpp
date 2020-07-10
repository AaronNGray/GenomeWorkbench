/*  $Id: seqgraphic_render.cpp 44757 2020-03-05 18:58:50Z evgeniev $
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
 */

#include <ncbi_pch.hpp>
#include <gui/widgets/seq_graphic/seqgraphic_render.hpp>
#include <gui/widgets/seq_graphic/seqgraphic_genbank_ds.hpp>
#include <gui/widgets/seq_graphic/seqgraphic_utils.hpp>
#include <gui/widgets/seq_graphic/seqgraphic_conf.hpp>
#include <gui/opengl/glutils.hpp>
#include <gui/opengl/irender.hpp>
#include <gui/objutils/tooltip.hpp>
#include <objmgr/util/sequence.hpp>

#include <math.h>


BEGIN_NCBI_SCOPE
USING_SCOPE(objects);

const TModelUnit kZoomFactorX     = 2.0;
const TModelUnit kLenseZoomFactor = 10.0;
const TVPUnit    kMaxPixelsBase   = 24;
const TModelUnit kEpsilon         = 0.0000001;


CSeqGraphicRenderer::CSeqGraphicRenderer(bool cgi_mode)
    : m_FeatPanel(new CFeaturePanel(&m_RenderingCtx, cgi_mode))
    , m_MinZoomSeq(log(1.0f / kMaxPixelsBase))
    , m_LensZoom(0.0, 0.0)
    , m_NeedLensZoom(false)
    , m_From(0.0)
    , m_To(0.0)
    , m_Horz(true)
    , m_Flipped(false)
    , m_NeedUpdateContent(false)
    , m_NeedUpdateLayout(true)
    , m_NeedUpdateSelection(false)
{
    m_FP.EnableOffset(true);
    m_RP.EnableOffset(true);
    m_RenderingCtx.PrepareContext(m_FP, m_Horz, m_Flipped);

    m_FeatPanel->SetSelectedFeats(m_SelManager.GetSelectedFeats());
    m_FeatPanel->SetSelectedCDSFeats(m_SelManager.GetSelectedCDSFeats());

#if 0
    // Create a greyscale shader to make sure everything is drawn monochrome when
    // user selects greyscale mode.  Note that the shader will retain colors that are
    // arlready greyscale, but other colors will be converted to greyscale using NTSC
    // conversion weights
    string vertex_shader_gs = "#version 120\n"
                              "varying vec4 color;\n"
                              "void main(void)\n"
                              "{\n"
                                  "color = gl_Color;\n"
                                  "if (abs(gl_Color.r-gl_Color.g) > 0.01 || abs(gl_Color.g-gl_Color.b) > 0.01) {\n"
                                      "float c = dot(gl_Color.rgb, vec3(0.299, 0.587, 0.114));\n"
                                      "color = vec4(c, c, c, gl_Color.a);\n"
                                  "}\n"
                                  "gl_Position = gl_ModelViewProjectionMatrix * gl_Vertex;\n"
                              "}";

    string fragment_shader = "#version 120\n"
                             "varying vec4 color\n;"
                             "void main(void)\n"
                             "{\n"
                                 "gl_FragColor = color;\n"
                             "}";
    m_MonochromeShader.CreateShader(vertex_shader_gs, fragment_shader);
#endif
}


CSeqGraphicRenderer::~CSeqGraphicRenderer()
{
}


void CSeqGraphicRenderer::SetInputObject(SConstScopedObject& obj)
{
    CIRef<ISGDataSource> ds = m_DSContext->GetDS(
        typeid(CSGSequenceDSType).name(), obj);
    m_DS.Reset(dynamic_cast<CSGSequenceDS*>(ds.GetPointer()));

    CScope& scope = m_DS->GetScope();
    m_SelManager.SetScope(&scope);
    m_SelManager.Clear();

    m_RulerPanel.SetSeqLength(m_DS->GetSequenceLength());

    m_RenderingCtx.SetSeqDS(m_DS);
    m_RenderingCtx.PrepareContext(m_FP, m_Horz, m_Flipped);
    m_FeatPanel->SetInputObject(obj);
    m_NeedUpdateSelection = true;

    /*string ruler_label;
    CLabel::GetLabel(*m_DS->GetBioseqHandle().GetSeqId(), &ruler_label,
                     CLabel::eDefault, &scope);
    ruler_label += ": ";
    ruler_label += sequence::CDeflineGenerator().GenerateDefline(m_DS->GetBioseqHandle());
    m_RulerPanel.SetTextLabel(ruler_label);*/
}


void CSeqGraphicRenderer::UpdateConfig(CRef<CSeqGraphicConfig> config)
{
    //TIME_ME("CSeqGraphicRenderer::UpdateConfig()");

    m_ConfigSettings = config;
    m_RenderingCtx.SetOverviewCutoff(config->GetOverviewCutoff());
    m_RenderingCtx.SetSelectionColor(config->GetSelectionColor());
    m_RenderingCtx.SetSelLabelColor(config->GetSelLabelColor());

    m_RulerPanel.LoadSettings(*config);

    m_FeatPanel->SetConfig(config);
    m_FeatPanel->LoadProfile(config->GetTrackProfile());
    m_NeedUpdateSelection = true;
}


void CSeqGraphicRenderer::SetHorizontal(bool b_horz, bool b_flip,
        const TVPRect& rc, bool reset_model_limit /* = false*/)
{
    if ( !m_DS ) {
        return;
    }
    //TIME_ME("CSeqGraphicRenderer::SetHorizontal()");

    m_Horz = b_horz;
    m_Flipped = b_flip;

    m_RP.EnableZoom(m_Horz, !m_Horz);
    m_RP.SetAdjustToLimits(m_Horz, !m_Horz);

    m_FP.EnableZoom(m_Horz, !m_Horz);
    m_FP.SetAdjustToLimits(m_Horz, !m_Horz);

    x_SetupViewPorts(rc);
    x_SetupModelLimits(reset_model_limit);

    // update some global settings
    CRef<CGlTextureFont> com_font = m_ConfigSettings->GetCommentFont();

    m_RulerPanel.SetOrientation(m_Horz, m_Flipped);

    m_NeedUpdateLayout = true;
}


void CSeqGraphicRenderer::ZoomOnRange(const TModelRange& range, bool round_to_base)
{
    //TIME_ME("CSeqGraphicRenderer::ZoomOnRange()");

    TModelUnit from = range.GetFrom();
    TModelUnit to = range.GetTo();
    if (fabs(m_From - from) < kEpsilon  &&  fabs(m_To - to) < kEpsilon ) {
        return;
    }
    m_From = from;
    m_To   = to;

    if (m_Flipped) {
        swap(from, to);
    }
    TModelRect rc = m_FP.GetVisibleRect();
    if (m_Horz) {
        rc.SetHorz(from, to);
    } else {
        rc.SetVert(from, to);
    }

    m_FP.ZoomRect(rc);
    x_AdjustToMinScale(round_to_base);

    m_RenderingCtx.PrepareContext(m_FP, m_Horz, m_Flipped);
    m_NeedUpdateContent = true;
}


void CSeqGraphicRenderer::Resize(const TVPRect& rc)
{
    x_SetupViewPorts(rc);
    x_SetupModelLimits(false);
    m_NeedUpdateLayout = true;
}


void CSeqGraphicRenderer::SetExternalGlyphs(const CSeqGlyph::TObjects& objs)
{
    m_FeatPanel->SetExternalGlyphs(objs);
}


void CSeqGraphicRenderer::SetLensZoom(int x, int y)
{
    TVPRect& rcFP = m_FP.GetViewport();

    if (rcFP.PtInRect(x, y)) {
        m_NeedLensZoom = true;
        CGlPaneGuard GUARD(m_FP, CGlPane::eOrtho);
        m_LensZoom = m_FP.UnProject(x, y);
    } else {
        m_NeedLensZoom = false;
    }
}


// for CGI use
TModelUnit CSeqGraphicRenderer::GetHeight()
{
    m_SelManager.UpdateSelection(m_FeatPanel);

    TModelUnit height = m_FeatPanel->GetHeight() +
        m_RulerPanel.GetSize().Y();

    _TRACE("height: " << height << " fp: " << m_FeatPanel->GetHeight() <<
           " ruler: " << m_RulerPanel.GetSize().Y());

    return height;
}


// this function takes an absolute mouse coordinate and determines if any
// features lie underneath the mouse position
// x & y are GL coordinates
CRef<CSeqGlyph> CSeqGraphicRenderer::HitTest(int x, int y)
{
    //m_FP.GetViewport().PtInRect(x, y);
    TModelPoint p = m_FP.UnProject(x, y);
    return m_FeatPanel->HitTest(p);
}


void CSeqGraphicRenderer::GetTooltip(int x, int y, string& tt, string& t_title)
{
    tt = "";

    CRef<CSeqGlyph> obj = HitTest(x, y);
    if (obj) {
        CIRef<ITooltipFormatter> tooltip = ITooltipFormatter::CreateTooltipFormatter(ITooltipFormatter::eTooltipFormatter_Html);
        TModelPoint pos = m_FP.UnProject(x, y);
        obj->GetTooltip(pos, *tooltip, t_title);
        tt = tooltip->Render();
    }
}


void CSeqGraphicRenderer::SelectObjByRect(const TVPRect& rc)
{
    TModelRect sel_rc;
    {{
        CGlPaneGuard GUARD(m_FP, CGlPane::eOrtho);
        TModelUnit left   = m_FP.UnProjectX(rc.Left());
        TModelUnit right  = m_FP.UnProjectX(rc.Right());
        TModelUnit top    = m_FP.UnProjectY(rc.Top());
        TModelUnit bottom = m_FP.UnProjectY(rc.Bottom());

        sel_rc = TModelRect(left, bottom, right, top);
    }}

    CSeqGlyph::TConstObjects sel_glyphs;
    m_FeatPanel->Intersects(sel_rc, sel_glyphs);
    m_SelManager.SelectSelection(sel_glyphs);
    m_NeedUpdateSelection = true;
}


//static CStopWatch timer_sw;
void CSeqGraphicRenderer::x_Draw(TImageInfo* p_areas)
{
    if ( !m_DS ) {
        return;
    }

#if 0
    IRender& gl = GetGl();

    if (m_ConfigSettings->GetColorTheme() == "Greyscale") {
        m_MonochromeShader.MakeCurrent();
    }
    else {
        gl.UseProgram(0);
    }
#endif

    //TIME_ME("CSeqGraphicRenderer::x_Draw()");
    try {
        const CRgbaColor& bg = m_ConfigSettings->GetBkColor();
        glClearColor(bg.GetRed(), bg.GetGreen(), bg.GetBlue(), 0.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        m_RenderingCtx.PrepareContext(m_FP, m_Horz, m_Flipped);
		m_RenderingCtx.GetGlPane()->SetExactOrthoProjection(true);

        // update the data
        if (m_NeedUpdateContent) {
            //TIME_ME("x_Draw feture panel Update content");
            m_FeatPanel->Update(false);
            m_NeedUpdateContent = false;
            // update selection
        } else if (m_NeedUpdateLayout) {
            //TIME_ME("x_Draw feture panel Update layout");
            m_FeatPanel->Update(true);
        }
        m_NeedUpdateLayout = false;

        if (m_NeedUpdateSelection) {
            //TIME_ME("x_Draw Update selection");
            m_SelManager.UpdateSelection(m_FeatPanel);
            m_NeedUpdateSelection = false;
        }

        TModelUnit height = m_FeatPanel->GetHeight();

        // We have a "floating" model space. So, adjust the limits
        // Adjust visible rect if model space "shrinked" less than visible range

        TModelRect rcV = m_FP.GetVisibleRect();
        TModelRect rcM = m_FP.GetModelLimitsRect();
        if (m_Horz) {
            rcM.SetBottom(height - 1);
            TModelUnit off = rcV.Bottom() - (height - 1);
            off = min(off, rcV.Top());
            if (off > 0.0) {
                rcV.Offset(0.0f, -off);
            }
        } else {
            rcM.SetLeft(height);
            TModelUnit off = rcV.Left() - height;
            off = min(off, rcV.Right());
            if (off > 0.0) {
                rcV.Offset(-off, 0.0);
            }
        }
        m_FP.SetModelLimitsRect(rcM);
        m_FP.SetVisibleRect(rcV);

        // Adjust ruler bar to master
        if (m_Horz) {
            rcV.SetBottom(0);
            rcV.SetTop(m_RP.GetViewport().Height() - 1);
        } else {
            rcV.SetLeft(0);
            rcV.SetRight(m_RP.GetViewport().Width() - 1);
        }
        m_RP.SetVisibleRect(rcV);
	    m_RulerPanel.Render(m_RP);

        // sync the grid step with the ruler label step to align
        // vertical grid lines with ruler tick labels
        m_FeatPanel->UpdateGridStep(m_RulerPanel.GetMainRulerLabelStep());

        // now, main panel
        {
            //TIME_ME("x_Draw feture panel draw content");
            m_FeatPanel->SetImageInfo(p_areas);
            m_FeatPanel->Draw();
        }

        // collect HTML active areas
        if (p_areas) {
            //TIME_ME("x_Draw Active Areas");
            m_RulerPanel.GetHTMLActiveAreas(&(p_areas->m_ActiveAreas));
            m_FeatPanel->GetHTMLActiveAreas(&(p_areas->m_ActiveAreas));

            // Offset by the height of Ruler Panel
			if (m_RulerPanel.IsShown()) {
                auto iter = p_areas->m_ActiveAreas.begin();
                while (++iter != p_areas->m_ActiveAreas.end()) {
                    (*iter).m_Bounds.Offset(0, m_RP.GetViewport().Height());
                }
			}
        }

        //This collects and draws html active areas for debugging only
        /*{
            CSeqGlyph::TAreaVector areas;
            m_FeatPanel->GetHTMLActiveAreas(&areas);
            NON_CONST_ITERATE(CSeqGlyph::TAreaVector, iter, areas) {
                (*iter).m_Bounds.Offset(0, m_RP.GetViewport().Height());
            }

            m_FP.OpenPixels();
            IRender& gl = GetGl();
            gl.Disable(GL_LINE_SMOOTH);
            gl.Color3f(1.0f, 0.0f, 0.0f);
            TModelUnit height = m_RP.GetViewport().Height() + m_FP.GetViewport().Height();
            for (size_t i = 0; i < areas.size(); ++i) {

                gl.Begin(GL_LINE_LOOP);
                gl.Vertex2d(areas[i].m_Bounds.Left(), height - areas[i].m_Bounds.Bottom());
                gl.Vertex2d(areas[i].m_Bounds.Left(), height - areas[i].m_Bounds.Top());
                gl.Vertex2d(areas[i].m_Bounds.Right(), height - areas[i].m_Bounds.Top());
                gl.Vertex2d(areas[i].m_Bounds.Right(), height - areas[i].m_Bounds.Bottom());
                gl.End();

                if (areas[i].m_Flags && CHTMLActiveArea::eHtmlArea_Track)
                    continue;
                _TRACE('(' << areas[i].m_Bounds.Left() << ',' << height - areas[i].m_Bounds.Top() << ',' << areas[i].m_Bounds.Right() << ',' << height - areas[i].m_Bounds.Bottom() << ')');
            }
            m_FP.Close();
        }*/


        if (m_NeedLensZoom) { // Draw Zoomed view
            x_DoLensZoom();
        }

    } NCBI_CATCH("CSeqGraphicRender::Render()");

#if 0
    if (m_ConfigSettings->GetColorTheme() == "Greyscale") {
        m_MonochromeShader.Release();
    }
#endif
    CGlUtils::CheckGlError();
}

void CSeqGraphicRenderer::x_DrawVectorGraphics(TImageInfo* p_areas)
{
    if ( !m_DS ) {
        return;
    }

    //TIME_ME("CSeqGraphicRenderer::x_Draw()");
    try {
        m_RenderingCtx.PrepareContext(m_FP, m_Horz, m_Flipped);
		m_RenderingCtx.GetGlPane()->SetExactOrthoProjection(true);

        // update the data
        if (m_NeedUpdateContent) {
            //TIME_ME("x_Draw feture panel Update content");
            m_FeatPanel->Update(false);
            m_NeedUpdateContent = false;
            // update selection
        } else if (m_NeedUpdateLayout) {
            //TIME_ME("x_Draw feture panel Update layout");
            m_FeatPanel->Update(true);
        }
        m_NeedUpdateLayout = false;

        if (m_NeedUpdateSelection) {
            //TIME_ME("x_Draw Update selection");
            m_SelManager.UpdateSelection(m_FeatPanel);
            m_NeedUpdateSelection = false;
        }

        TModelUnit height;// = m_FeatPanel->GetHeight();
        height = m_FP.GetViewport().Height();

        // We have a "floating" model space. So, adjust the limits
        // Adjust visible rect if model space "shrinked" less than visible range


        TModelRect rcM = m_FP.GetModelLimitsRect();
        TModelRect rcV = m_FP.GetVisibleRect();
        rcM.SetBottom(height);
        rcV.SetBottom(height);
        m_FP.SetModelLimitsRect(rcM);
        m_FP.SetVisibleRect(rcV);
        // Adjust ruler bar to master
        if (m_Horz) {
            rcV.SetBottom(0);
            rcV.SetTop(m_RP.GetViewport().Height() - 1);
        }
        m_RP.SetVisibleRect(rcV);
        m_RulerPanel.Render(m_RP);

        // sync the grid step with the ruler label step to align
        // vertical grid lines with ruler tick labels
        m_FeatPanel->UpdateGridStep(m_RulerPanel.GetMainRulerLabelStep());

        // now, main panel
        {
            //TIME_ME("x_Draw feture panel draw content");
            if (nullptr != p_areas) {
                m_FeatPanel->SetImageInfo(p_areas);
            }
            m_FeatPanel->Draw();
        }

        // collect HTML active areas
        if (p_areas) {
            //TIME_ME("x_Draw Active Areas");
            m_RulerPanel.GetHTMLActiveAreas(&(p_areas->m_ActiveAreas));
            m_FeatPanel->GetHTMLActiveAreas(&(p_areas->m_ActiveAreas));

            // Offset by the height of Ruler Panel
            if (m_RulerPanel.IsShown()) {
                auto iter = p_areas->m_ActiveAreas.begin();
                while (++iter != p_areas->m_ActiveAreas.end()) {
                    (*iter).m_Bounds.Offset(0, m_RP.GetViewport().Height());
                }
            }
        }

    } NCBI_CATCH("CSeqGraphicRender::Render()");

    //CGlUtils::CheckGlError();
}


void CSeqGraphicRenderer::x_DimDisplay()
{
    IRender& gl = GetGl();

    gl.Enable(GL_BLEND);
    gl.BlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    gl.Color4f(1.0f, 1.0f, 1.0f, 0.9f);

    TVPRect rc_r = m_RP.GetViewport();
    TVPRect rc_f = m_FP.GetViewport();

    {{
        CGlPaneGuard GUARD(m_RP, CGlPane::ePixels);
        gl.Rectf((float)rc_r.Left(), (float)rc_r.Bottom(),
                 (float)rc_r.Right() + 1.0f, (float)rc_r.Top());
    }}

    {{
        CGlPaneGuard GUARD(m_FP, CGlPane::ePixels);
        gl.Rectf((float)rc_f.Left(), (float)rc_f.Bottom(),
                 (float)rc_f.Right() + 1.0f, (float)rc_f.Top());
    }}
    gl.Disable(GL_BLEND);
}


void CSeqGraphicRenderer::x_DoLensZoom()
{
    x_DimDisplay();
    TModelRect rc = m_FP.GetVisibleRect();
    m_FP.SetZoomFactor(kLenseZoomFactor);
    m_FP.ZoomIn(m_LensZoom);

    //m_FeatPanel->PrepareLayout(m_FP, CFeaturesPanelIR::eFetchSync);
    //m_FeatPanel->GetLayoutHeight(m_FP);
    //x_AdjustToMaster(true);
    //m_FeatPanel->Render(m_FP);
    m_RulerPanel.Render(m_RP);
    m_FP.SetZoomFactor(kZoomFactorX);
    m_FP.SetVisibleRect(rc);
}


// make sure that we do not exeed or min. scale.
// With the flipped model space SetMinScaleX() method of
// CGlPane no longer works. We have to do it manually.
void CSeqGraphicRenderer::x_AdjustToMinScale(bool round_to_base)
{
    TModelUnit min_scale = (m_Flipped ? -1.0 : 1.0) / kMaxPixelsBase;
    TModelRect rc = m_FP.GetVisibleRect();

    // horizontal mode, flipped model space
    if (m_Horz &&  m_Flipped  &&  m_FP.GetScaleX() > min_scale) {
        m_FP.SetScale(min_scale, m_FP.GetScaleY());
        if (round_to_base) {
            rc = m_FP.GetVisibleRect();
            rc.SetLeft(ceil(rc.Left()));
            rc.SetRight(floor(rc.Right()));
        }
    }

    // horizontal mode, normal model space
    if (m_Horz  &&  !m_Flipped  &&  m_FP.GetScaleX() < min_scale) {
        m_FP.SetScale(min_scale, m_FP.GetScaleY());
        if (round_to_base) {
            rc = m_FP.GetVisibleRect();
            rc.SetLeft(floor(rc.Left()));
            rc.SetRight(ceil(rc.Right()));
        }
    }

    // vertical mode, flipped model space
    if (!m_Horz  &&  m_Flipped  &&  m_FP.GetScaleY() > min_scale) {
        m_FP.SetScale(m_FP.GetScaleX(), min_scale);
        if (round_to_base) {
            rc = m_FP.GetVisibleRect();
            rc.SetTop(floor(rc.Top()));
            rc.SetBottom(ceil(rc.Bottom()));
        }
    }

    // vertical mode, normal model space
    if (!m_Horz  &&  !m_Flipped  &&  m_FP.GetScaleY() < min_scale) {
        m_FP.SetScale(m_FP.GetScaleX(), min_scale);
        if (round_to_base) {
            rc = m_FP.GetVisibleRect();
            rc.SetTop(ceil(rc.Top()));
            rc.SetBottom(floor(rc.Bottom()));
        }
    }

    m_FP.ZoomRect(rc);
}


TSeqPos CSeqGraphicRenderer::Screen2Seq(TVPUnit x, TVPUnit y)
{
    CGlPaneGuard GUARD(m_FP, CGlPane::eOrtho);
    TModelUnit seq = m_Horz ? m_FP.UnProjectX(x) : m_FP.UnProjectY(y);
    return TSeqPos(seq);
}


TSeqPos CSeqGraphicRenderer::Screen2SeqWidth(TVPUnit vp)
{
    CGlPaneGuard GUARD(m_FP, CGlPane::eOrtho);
    TModelUnit seq = m_Horz ? m_FP.UnProjectWidth(vp) :
        m_FP.UnProjectHeight(vp);
    return TSeqPos(seq);
}


void CSeqGraphicRenderer::x_SetupViewPorts()
{
    TVPRect ruler_viewport = m_RP.GetViewport();
    TVPRect feat_viewport = m_FP.GetViewport();
    if (m_Horz) {
        TVPUnit ruler_b  = ruler_viewport.Top() - m_RulerPanel.GetSize().Y();
        ruler_viewport.SetBottom(ruler_b);
        feat_viewport.SetTop(ruler_b);
    } else {
        // extra space for minus sign
        TVPUnit ruler_l = ruler_viewport.Right() -
            m_RulerPanel.GetSize(m_DS->GetSequenceLength() + 5).X();
        ruler_viewport.SetLeft(ruler_l);
        feat_viewport.SetRight(ruler_l);
    }
    m_RP.SetViewport(ruler_viewport);
    m_FP.SetViewport(feat_viewport);
}


void CSeqGraphicRenderer::x_SetupViewPorts(const TVPRect& rc)
{
    //cout << "CSeqGraphicRenderer::x_SetupViewPorts" << endl;
    TVPRect ruler_viewport, feat_viewport;
    if (m_Horz) {
        TVPUnit ruler  = m_RulerPanel.GetSize().Y();

        ruler_viewport = TVPRect(rc.Left(), rc.Top() - (ruler - 1), rc.Right(), rc.Top());
        feat_viewport  = TVPRect(rc.Left(), rc.Bottom(), rc.Right(), rc.Top() - ruler);
    } else {
        // extra space for minus sign
        TVPUnit ruler = m_RulerPanel.GetSize(m_DS->GetSequenceLength() + 5).X();

        ruler_viewport = TVPRect(rc.Right() - ruler, rc.Bottom(), rc.Right(), rc.Top() - 1);
        feat_viewport  = TVPRect(rc.Left(), rc.Bottom(), rc.Right() - ruler, rc.Top() - 1);
    }

    m_RP.SetViewport(ruler_viewport);
    m_FP.SetViewport(feat_viewport);
}

// todo: need more work here to make flipping and vertical work
void CSeqGraphicRenderer::x_SetupModelLimits(bool reset_model_limit)
{
    TModelRect rc_fp;
    TModelRange vis_r(0, m_DS->GetSequenceLength() - 1);

    if (m_Horz) {
        if (m_Flipped) {
            rc_fp = TModelRect(vis_r.GetToOpen(), m_FP.GetViewport().Height() - 1,
                vis_r.GetFrom(), 0);
        } else {
            rc_fp = TModelRect(vis_r.GetFrom(), m_FP.GetViewport().Height() - 1,
                vis_r.GetToOpen(), 0);
        }
    } else {
        if (m_Flipped) {
            rc_fp = TModelRect(m_FP.GetViewport().Height(), vis_r.GetToOpen(),
                0, vis_r.GetFrom());
        } else {
            rc_fp = TModelRect(m_FP.GetViewport().Height(), vis_r.GetFrom(),
                0, vis_r.GetToOpen());
        }
    }
    m_FP.SetModelLimitsRect(rc_fp);
    m_RP.SetModelLimitsRect(rc_fp);

    if ( !reset_model_limit ) {
        vis_r = m_RenderingCtx.GetVisibleRange();
        if (m_Horz) {
            if (m_Flipped) {
                rc_fp = TModelRect(vis_r.GetTo(), m_FP.GetViewport().Height() - 1,
                    vis_r.GetFrom(), 0);
            } else {
                rc_fp = TModelRect(vis_r.GetFrom(), m_FP.GetViewport().Height() - 1,
                    vis_r.GetTo(), 0);
            }
        } else {
            if (m_Flipped) {
                rc_fp = TModelRect(m_FP.GetViewport().Height(), vis_r.GetTo(),
                    0, vis_r.GetFrom());
            } else {
                rc_fp = TModelRect(m_FP.GetViewport().Height(), vis_r.GetFrom(),
                    0, vis_r.GetTo());
            }
        }
    }

    m_FP.SetVisibleRect    (rc_fp);
    m_RP.SetVisibleRect    (rc_fp);
}

void CSeqGraphicRenderer::SetViewWidth(TSeqPos view_width)
{
    m_RenderingCtx.SetViewWidth(view_width);
}


END_NCBI_SCOPE
