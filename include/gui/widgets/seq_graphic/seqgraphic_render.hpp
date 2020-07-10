#ifndef GUI_WIDGETS_SEQ_GRAPHIC___SEQGRAPHIC_RENDER__HPP
#define GUI_WIDGETS_SEQ_GRAPHIC___SEQGRAPHIC_RENDER__HPP

/*  $Id: seqgraphic_render.hpp 44103 2019-10-28 14:46:41Z filippov $
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
 *   Renderer for the Graphic Sequence view widget
 *   Independent from FLTK.
 *
 */

//#include <gui/opengl/gldlist.hpp>
#include <gui/graph/axis.hpp>
#include <gui/widgets/seq_graphic/selection_manager.hpp>
#include <gui/widgets/seq_graphic/ruler_panel.hpp>
#include <gui/widgets/seq_graphic/feature_panel.hpp>
#include <gui/widgets/seq_graphic/rendering_ctx.hpp>
#include <gui/widgets/seq_graphic/seqgraphic_ds_manager.hpp>

#include <gui/widgets/gl/ruler.hpp>
#include <gui/widgets/gl/html_active_area.hpp>

#include <gui/print/pdf.hpp>

BEGIN_NCBI_SCOPE


class CSGSequenceDS;
class CSeqGraphicConfig;


class NCBI_GUIWIDGETS_SEQGRAPHIC_EXPORT CSeqGraphicRenderer : public CObject
{
public:
    typedef CSeqGlyph::TAreaVector  TAreaVector;
    typedef CSeqGraphicImageInfo TImageInfo;
    typedef vector<CSeqGraphicHairline> THairlineVector;

    CSeqGraphicRenderer(bool cgi_mode);
    ~CSeqGraphicRenderer();

    void SetDSContext(ISGDataSourceContext* ds_ctx);
    void SetHost(ILayoutTrackHost* host);
    void SetInputObject(SConstScopedObject& obj);
    void SetViewWidth(TSeqPos view_width);
    void SetHighlightsColor(const CRgbaColor &color);
    void ConfigureTracks();
    CRef<CSGSequenceDS> GetDS() { return m_DS; }

    void UpdateConfig(CRef<CSeqGraphicConfig> config);
    CRef<CSeqGraphicConfig> GetConfig() { return m_ConfigSettings; }
    void SetHorizontal(bool b_horz, bool b_flip, const TVPRect& rc,
        bool reset_model_limit = false);
    void SetVertScroll(TModelUnit val);

    /// zoom to the given range.
    /// @param round_to_base round the range to integer base
    void ZoomOnRange(const TModelRange& range, bool round_to_base = false);

    void Resize(const TVPRect& rc);
    void SetExternalGlyphs(const CSeqGlyph::TObjects& objs);
    void OnDataChanging();
    void OnDataChanged();
    void UpdateLayout();
    void UpdateVectorLayout();
    void SetUpdateContent();
    void UpdateData();
    bool AllJobsFinished() const;
    bool AllTracksCreated() const;
    void SaveTrackConfig();

    /// zoom 10x at the given screen coordinates.
    void SetLensZoom(int x, int y);
    void CancelLensZoom();

    /// Expose a few GlPanes.
    CGlPane& GetFeatGlPane();
    CGlPane& GetRulerGlPane();
    CFeaturePanel* GetFeaturePanel();

    /// retrieve the expected height of this widget.
    TModelUnit GetLayoutHeight() const;
    TModelUnit GetHeight();

    void Render(void);
    void RenderVectorGraphics();

    void SetImageInfo(TImageInfo* image_info);

    /// @name cgi mode-related delegate methods for feature panel
    /// @{
    void Render(TImageInfo& image_info);
    void Render(TImageInfo* image_info);
    void SetTrackConfig(const CFeaturePanel::TTrackSettingsSet& settings_set);
    /// skip track existence verification.
    /// Assume tracks exist.
    void SkipGenuineCheck();
    TSeqRange GetVisibleRange() const;

    /// turn on an existing track.
    /// If there is no such a track in the configuration, it does nothing.
    void ShowTrack(const string& track_key);
   
    void HideRuler();
    /// @}
    

    /// Ruler Mark.
    void SetRulerSeqStart(TSeqPos pos);

    CRef<CSeqGlyph> HitTest(int x, int y);
    void GetTooltip(int x, int y, string& tt, string& t_title);

    void SelectObject(const CObject* obj, bool verified);
    void SelectObjByRect(const TVPRect& rc);
    void SelectObjectSig(const string& obj_sig);
    void SetNeedUpdateSelection();
    void DeSelectObject(const CObject* obj);
    void ResetObjectSelection(void);
    bool IsObjectSelected(const CObject* obj);
    bool HasSelectedObjects() const;
    void GetObjectSelection(ncbi::TConstObjects& objs) const;
    const CSeqGlyph::TConstObjects& GetSelectedLayoutObjects();

    /// pixels to sequence translation.
    TSeqPos Screen2Seq(TVPUnit x, TVPUnit y);
    TSeqPos Screen2SeqWidth(TVPUnit vp);

    void LoadDefaultTracks();
    void ExpandAllTracks(bool expand);
    void ShowAllTracks(bool shown) { m_FeatPanel->ShowAll(shown); }

private:
    void x_Draw(TImageInfo* p_areas);
    void x_DrawVectorGraphics(TImageInfo* p_areas);

    void x_DimDisplay();
    void x_DoLensZoom();

    void x_SetupViewPorts();
    void x_SetupViewPorts(const TVPRect& rc);
    void x_SetupModelLimits(bool reset_model_limit);
    void x_AdjustToMinScale(bool round_to_base);

private:
    CIRef<ISGDataSourceContext> m_DSContext;
    CRef<CSGSequenceDS>         m_DS;
    CRef<CSeqGraphicConfig>     m_ConfigSettings;
    CRenderingContext           m_RenderingCtx;
    CSelectionVisitor           m_SelManager;
    CRef<CFeaturePanel>         m_FeatPanel;

    CGlPane         m_RP;
    CGlPane         m_FP;
    CRulerPanel     m_RulerPanel;
#if 0
    CGlShader       m_MonochromeShader;
#endif

    TModelUnit      m_MinZoomSeq;
    TModelPoint     m_LensZoom;
    bool            m_NeedLensZoom;

    TModelUnit      m_From;
    TModelUnit      m_To;

    bool            m_Horz;
    bool            m_Flipped;

    bool    m_NeedUpdateContent;
    bool    m_NeedUpdateLayout;
    bool    m_NeedUpdateSelection;

    TImageInfo  *m_ImageInfo = nullptr;
};

///////////////////////////////////////////////////////////////////////////////
/// CSeqGraphicRenderer inline methods
///
inline
void CSeqGraphicRenderer::SetDSContext(ISGDataSourceContext* ds_ctx)
{
    m_DSContext.Reset(ds_ctx);
    m_FeatPanel->SetDSContext(ds_ctx);
}

inline
void CSeqGraphicRenderer::SetHost(ILayoutTrackHost* host)
{
    m_FeatPanel->SetHost(host);
}

inline
void CSeqGraphicRenderer::SetHighlightsColor(const CRgbaColor &color)
{
    m_RenderingCtx.SetHighlightsColor(color);
}

inline
void CSeqGraphicRenderer::ConfigureTracks()
{
    // make don't update data twice since ConfigureTracks()
    // triggers updating data automatically.
    m_NeedUpdateContent = false;
    m_FeatPanel->ConfigureTracks();
}

inline
void CSeqGraphicRenderer::SetVertScroll(TModelUnit val)
{
    m_FP.Scroll(0, val - m_FP.GetVisibleRect().Top());
}

inline
void CSeqGraphicRenderer::OnDataChanging()
{
    m_SelManager.ClearSelectedFeats();
    m_FeatPanel->OnDataChanging();
}

inline
void CSeqGraphicRenderer::OnDataChanged()
{
    m_SelManager.ClearSelectedFeats();
    m_DSContext->ClearCache();
    m_FeatPanel->OnDataChanged();
}

inline
void CSeqGraphicRenderer::UpdateLayout()
{
    m_NeedUpdateLayout = true;
}

inline
void CSeqGraphicRenderer::UpdateVectorLayout()
{
    m_FeatPanel->Update(true);
}

inline 
void CSeqGraphicRenderer::SetUpdateContent()
{
    m_NeedUpdateContent = true;
}

inline 
void CSeqGraphicRenderer::UpdateData()
{
    m_FeatPanel->Update(false);
    m_NeedUpdateContent = false;
}

inline
bool CSeqGraphicRenderer::AllJobsFinished() const
{
    return m_FeatPanel->AllJobsFinished();
}

inline
bool CSeqGraphicRenderer::AllTracksCreated() const
{
    return m_FeatPanel->AllTracksCreated();
}

inline
void CSeqGraphicRenderer::SaveTrackConfig()
{
    m_FeatPanel->SaveTrackConfig();
}

inline
void CSeqGraphicRenderer::CancelLensZoom()
{
    m_NeedLensZoom = false;
}

inline
CGlPane& CSeqGraphicRenderer::GetFeatGlPane()
{
    return m_FP;
}

inline
CGlPane& CSeqGraphicRenderer::GetRulerGlPane()
{
    return m_RP;
}

inline
CFeaturePanel* CSeqGraphicRenderer::GetFeaturePanel()
{
    return m_FeatPanel;
}

inline
TModelUnit CSeqGraphicRenderer::GetLayoutHeight() const
{
    return m_FeatPanel->GetHeight() + m_RulerPanel.GetSize().Y();
}

inline
void CSeqGraphicRenderer::Render()
{
    x_Draw(NULL);
}

inline
void CSeqGraphicRenderer::Render(TImageInfo& areas)
{
    x_Draw(&areas);
}

inline
void CSeqGraphicRenderer::RenderVectorGraphics()
{
    x_DrawVectorGraphics(m_ImageInfo);
}

inline
void CSeqGraphicRenderer::SetImageInfo(TImageInfo* image_info)
{
    m_ImageInfo = image_info;
}

inline
void CSeqGraphicRenderer::Render(TImageInfo* p_areas)
{
    x_Draw(p_areas);
}

inline
void CSeqGraphicRenderer::SetTrackConfig(
    const CFeaturePanel::TTrackSettingsSet& settings_set)
{
    m_FeatPanel->SetTrackConfig(settings_set);
}

inline
void CSeqGraphicRenderer::SkipGenuineCheck()
{
    m_FeatPanel->SkipGenuineCheck();
}

inline
TSeqRange CSeqGraphicRenderer::GetVisibleRange() const
{
    return m_RenderingCtx.GetVisSeqRange();
}

inline
void CSeqGraphicRenderer::ShowTrack(const string& track_key)
{
    m_FeatPanel->ShowTrack(track_key);
}

inline
void CSeqGraphicRenderer::HideRuler()
{
    m_RulerPanel.SetShown(false);
}

inline
void CSeqGraphicRenderer::SetRulerSeqStart(TSeqPos pos)
{
    m_RulerPanel.SetRulerSeqStart(pos);
    x_SetupViewPorts();
}

inline
void CSeqGraphicRenderer::SelectObject(const CObject* obj, bool verified)
{
    m_SelManager.SelectObject(obj, verified);
    m_NeedUpdateSelection = true;
}

inline
void CSeqGraphicRenderer::SelectObjectSig(const string& obj_sig)
{
    m_SelManager.SetSelectedObjectSig(obj_sig);
    m_NeedUpdateSelection = true;
}

inline
void CSeqGraphicRenderer::SetNeedUpdateSelection()
{   m_NeedUpdateSelection = true; }


inline
void CSeqGraphicRenderer::DeSelectObject(const CObject* obj)
{
    m_SelManager.DeSelectObject(obj);
    if ( !m_SelManager.HasSelectedObjects() ) {
        ResetObjectSelection();
    } else {
        m_NeedUpdateSelection = true;
    }
}

inline
void CSeqGraphicRenderer::ResetObjectSelection(void)
{
    m_SelManager.ResetObjectSelection(m_FeatPanel);
}

inline
bool CSeqGraphicRenderer::IsObjectSelected(const CObject* obj)
{
    return m_SelManager.IsObjectSelected(obj);
}

inline
bool CSeqGraphicRenderer::HasSelectedObjects() const
{
    return m_SelManager.HasSelectedObjects();
}

inline  
void CSeqGraphicRenderer::GetObjectSelection(TConstObjects& objs) const
{
    m_SelManager.GetObjectSelection(objs);
}

inline
const CSeqGlyph::TConstObjects&
CSeqGraphicRenderer::GetSelectedLayoutObjects()
{
    return m_SelManager.GetSelectedLayoutObjects(m_FeatPanel);
}

inline
void CSeqGraphicRenderer::LoadDefaultTracks()
{
    m_FeatPanel->LoadDefaultTracks();
}

inline
void CSeqGraphicRenderer::ExpandAllTracks(bool expand)
{
    m_FeatPanel->ExpandAll(expand);
}

END_NCBI_SCOPE


#endif  /* GUI_WIDGETS_SEQ_GRAPHIC___SEQGRAPHIC_RENDER__HPP */
