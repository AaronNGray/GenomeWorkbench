#ifndef GUI_WIDGETS_SEQ_GRAPHIC___FEAT_PANEL_RENDERING_CONTEXT__HPP
#define GUI_WIDGETS_SEQ_GRAPHIC___FEAT_PANEL_RENDERING_CONTEXT__HPP

/*  $Id: rendering_ctx.hpp 43363 2019-06-20 14:33:30Z shkeda $
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
 *
 */

#include <gui/gui.hpp>
#include <gui/opengl/glbitmapfont.hpp>
#include <gui/utils/rgba_color.hpp>
#include <gui/opengl/gltexturefont.hpp>
#include <gui/opengl/irender.hpp>
#include <gui/widgets/seq_graphic/seq_glyph.hpp>
#include <gui/widgets/seq_graphic/seqgraphic_genbank_ds.hpp>
#include <objmgr/scope.hpp>
#include <gui/print/pdf.hpp>

BEGIN_NCBI_SCOPE

class CGlPane;

static const float kSequenceLevel = 0.125; // = 1.0f / 8.0;



///////////////////////////////////////////////////////////////////////////////
/// CRenderingContext offers the basic context and utility methods for
/// rendering layout objects in feature panel.
///

class NCBI_GUIWIDGETS_SEQGRAPHIC_EXPORT CRenderingContext
{
public:
    /// extra space for side labeling, in screen pixels
    static const int kLabelSpacePx;

    CRenderingContext();
    virtual ~CRenderingContext();

    void PrepareContext(CGlPane& pane, bool horz, bool flipped);  
    CGlPane*  GetGlPane();
    const CGlPane*  GetGlPane() const;

    void SetSeqDS(CSGSequenceDS* ds);
    CRef<CSGSequenceDS> GetSeqDS() const;

    objects::CScope& GetScope();
    const objects::CScope& GetScope() const;

    void SetOverviewCutoff(int cutoff);
    void SetSelectionColor(const CRgbaColor& c);
    const CRgbaColor& GetSelectionColor() const;
    void SetSelLabelColor(const CRgbaColor& c);
    const CRgbaColor& GetSelLabelColor() const;
    void SetHighlightsColor(const CRgbaColor &c);
    const CRgbaColor& GetHighlightsColor() const;

    const TModelUnit&  GetScale() const;
    const TModelUnit&  GetOffset() const;
    bool        IsHorizontal() const;
    bool        IsFlippedStrand() const;

    const TModelRange& GetVisibleLimit() const;
    const TModelRange& GetVisibleRange() const;
    TModelUnit GetVisibleFrom() const;
    TModelUnit GetVisibleTo() const;

    void SetViewWidth(TSeqPos view_width);
    TSeqPos GetViewWidth() const;

    const TSeqRange& GetVisSeqRange() const;
    TSeqPos GetVisSeqFrom() const;
    TSeqPos GetVisSeqTo() const;

    bool IsOverviewMode() const;
    /// is it enougth space to sequence letters.
    bool WillSeqLetterFit() const;
    /// will translation letters fit.
    bool WillTransLettersFit() const;
    bool IsInVisibleRange(TSeqPos pos) const;

    // test for label fitment into layout object modelspace
    TModelUnit GetMinLabelWidthPos() const;
    TModelUnit GetMinLabelWidthPos(const CGlTextureFont& font) const;
    bool WillLabelFit(const TModelRect& rc) const;
    bool WillLabelFit(const TModelRange& r) const;
    bool WillLabelFit(const TSeqRange& r) const;
    bool WillLabelFit(const TModelRect& rc,
        const CGlBitmapFont& font, const string& label) const;
    bool WillLabelFit(const TModelRect& rc,
        const CGlTextureFont& font, const string& label) const;
    bool WillLabelFit(const TModelRange& r,
        const CGlBitmapFont& font, const string& label) const;
    bool WillLabelFit(const TModelRange& r,
        const CGlTextureFont& font, const string& label) const;
    bool WillLabelFit(const TSeqRange& r,
        const CGlBitmapFont& font, const string& label) const;
    bool WillLabelFit(const TSeqRange& r,
        const CGlTextureFont& font, const string& label) const;

    /// In screen pixel..
    TModelUnit GetMaxLabelWidth(const CGlBitmapFont& font) const;
    TModelUnit GetMaxLabelWidth(const CGlTextureFont& font) const;

    /// convert from screen pixels to sequence positions
    TModelUnit ScreenToSeq(const TModelUnit& size) const;

    /// convert from sequence positions to screen pixels
    TModelUnit SeqToScreen(const TModelUnit& size) const;
    TVPUnit    SeqToScreenX(const TModelUnit& size) const;
    TVPUnit    SeqToScreenXClipped(const TModelUnit& size) const;
    TModelUnit SeqToScreenXInModelUnit(const TModelUnit& size) const;

    /// @name intersect with the visible range (horizontal).
    /// @{
    TModelRange IntersectVisible(const CSeqGlyph* obj) const;
    TModelRange IntersectVisible(const TModelRange& r) const;
    TModelRange IntersectVisible(const TSeqRange& r) const;
    /// @}

    /// inttersect with the vertical visible screen space.
    TModelRange IntersectVisible_Y(const TModelRange& r) const;

    /// @name rendering methods
    /// @{
    void DrawLine(TModelUnit x1, TModelUnit y1, TModelUnit x2,
        TModelUnit y2) const;

    // draw line with anti-alias disabled
    void DrawLine_NoAA(TModelUnit x1, TModelUnit y1, TModelUnit x2,
        TModelUnit y2) const;

    // draw stippled line with anti-alias disabled
    void DrawStippledLine(TModelUnit x1, TModelUnit y1, TModelUnit x2,
        TModelUnit y2, int factor = 2, unsigned short pattern = 0xAAAA) const;

    void DrawLineStrip(const vector<TModelPoint>& points) const;

    void DrawTriangle(TModelUnit x1, TModelUnit y1, TModelUnit x2,
        TModelUnit y2, TModelUnit x3, TModelUnit y3, bool border = false) const;

    void DrawQuad(const TModelRect& rc, bool border = false) const;

    void DrawShadedQuad(const TModelRect& rcm, const CRgbaColor& c1,
        const CRgbaColor& c2, bool shade_vert = true) const;

    void DrawQuad(TModelUnit x1, TModelUnit y1, TModelUnit x2,
        TModelUnit y2, bool border = false) const;

    void DrawSquare(const TModelPoint& p, TModelUnit size,
        const CRgbaColor& color, bool neg_strand) const;

    void DrawDisk(const TModelPoint& p, TModelUnit radius,
        GLint from = 0, GLint to = 360) const;

    void DrawDisk(const TModelPoint& p, TModelUnit radius,
        const CRgbaColor& color, bool neg_strand) const;

    // DrawDisk seems to leave the center too light sometimes
    void DrawDisk2(const TModelPoint& p, TModelUnit size,
        CRgbaColor color);

    void Draw3DTriangle(const TModelPoint& p, TModelUnit size,
        const CRgbaColor& color, bool neg_strand, bool need_neck) const;

    void Draw3DArrow(const TModelPoint& p, TModelUnit size,
        bool neg_strand) const;

    void DrawArrow(TModelUnit x1, TModelUnit x2, TModelUnit x3,
        TModelUnit line_center, TModelUnit bar_h, TModelUnit head_h) const;

    void DrawArrowBoundary(TModelUnit x1, TModelUnit x2, TModelUnit x3,
        TModelUnit line_center, TModelUnit bar_h, TModelUnit head_h) const;

    void DrawGreaterLessSign(TModelUnit x1, TModelUnit x2,
        TModelUnit bar_h, bool negative) const;

    void Draw3DQuad(TModelUnit x1, TModelUnit y1, TModelUnit x2,
        TModelUnit y2, const CRgbaColor& color, bool border = false) const;

    void Draw3DQuad_HorzLines(TModelUnit x1, TModelUnit y1,
        TModelUnit x2, TModelUnit y2,
        const CRgbaColor& color, bool border = false) const;

    void Draw3DFletch(TModelUnit pos_x, TModelUnit line_y1,
        TModelUnit line_y2, TModelUnit tail_height, const CRgbaColor& color,
        bool neg_strand) const;

    void DrawSelection(const TModelRect& rc) const;
    void DrawSelection(TModelUnit x1, TModelUnit y1,
        TModelUnit x2, TModelUnit y2) const;

    void DrawRect(const TModelRect& rc) const;
    void DrawRect(TModelUnit x1, TModelUnit y1,
        TModelUnit x2, TModelUnit y2) const;

    void DrawBackground(const TModelRect& rcm, TModelUnit border) const;

    void DrawStrandIndicators(const TModelPoint& start,
        TModelUnit length, TModelUnit apart, TModelUnit size,
        const CRgbaColor& color, bool neg_strand,
        bool avoid_center = false, bool single_indicator_center = false,
        const vector<TModelRange> *labels_ranges = nullptr) const;

    void Draw5Prime(TModelUnit x, TModelUnit y,
        bool direct, TModelUnit size_h, TModelUnit size_v) const;

    void DrawTexture(const TModelRect& rc, const TModelRect& rc_coord) const;

    void DrawHairLine(int opt, size_t total,
        const multiset <TSeqPos>& all_pos,
        TSeqPos pos, TModelUnit line_y,
        const CRgbaColor& c_light, const CRgbaColor& c_dark) const;

    void DrawUnalignedTail(TModelUnit x1, TModelUnit x2,
        TModelUnit y1, TModelUnit y2, bool is_polya,
        const CRgbaColor& color) const;

    void DrawPseudoBar(TModelUnit x1, TModelUnit y1,
                       TModelUnit x2, TModelUnit y2,
                       const CRgbaColor& light_color,
                       const CRgbaColor& dark_color) const;

    /// Draw partial feature indicator.
    /// @param p_start means the start (left in non-flipped mode) is partial
    /// @param p_stop means the stop (right in non-flipped mode) is partial
    /// @param loc true if it is a partial location, not just a partial feature
    void DrawPartialBarMark(TModelUnit x1, TModelUnit y1,
        TModelUnit y2, bool loc, TModelUnit dir=1.0) const;
    void DrawPartialBar(TModelUnit x1, TModelUnit y1, TModelUnit x2,
        TModelUnit y2, bool p_start, bool p_stop, bool loc) const;

    void DrawHighlight(const TModelRect& rect, const CRgbaColor *color = 0) const;

    void TextOut(const CGlTextureFont* font, const char* text,
        TModelUnit x, TModelUnit y, bool center, bool adjust_flip = true) const;
    void TextOut(const CGlBitmapFont* font, const char* text,
        TModelUnit x, TModelUnit y, bool center, bool adjust_flip = true) const;

    void AdjustToOrientation(TModelUnit& x) const;

    void AdjustToOrientation(TModelRect& rect) const;
    /// @}

    void SetIsDrawn(const string& sPName, bool isDrawn);
    bool GetIsDrawn(const string& sPName) const;

    enum ERenderingFlags
    {
        fSkipContent = 1 << 0,
        fSkipTitleBar = 1 << 1, ///< can be collpased and expanded.
        fSkipControls = 1 << 2, ///< track frame will be rendered when triggered.
        fSkipGrid = 1 << 3, ///< track frame will be rendered when triggered.
        fSkipFrames = 1 << 4
    };

    typedef unsigned TRenderingFlags;
    void SetRenderingFlags(TRenderingFlags rendering_flags);
    bool IsSkipTitleBar() const;
    bool IsSkipContent() const;
    bool IsSkipControls() const;
    bool IsSkipGrid() const;
    bool IsSkipFrames() const;

protected:
    void x_CreateGlyphs();

    CIRef<IVboGeom> m_GlyphGeom; 

    CGlPane*    m_Pane;    ///< What we drawing on

    /// @name global settings
    /// @{
    int         m_OverviewCutoff;
    CRgbaColor  m_SelColor;
    CRgbaColor  m_SelLabelColor;
    CRgbaColor  m_HighlightsColor;
    TModelUnit  m_MinLabelSizePos; ///< minimum label size, in sequence coordinates
    /// @}

    TModelUnit  m_Offset;
    TModelUnit  m_Scale;
    TModelUnit  m_ScaleInv;

    TModelRange m_LimitRange;   ///< Model limits
    TModelRange m_VisRange;     ///< Model visible limits
    TSeqRange   m_VisSeqRange;  ///< Visilble sequence range
    TSeqPos     m_ViewWidth;    ///< Browser's window width in pixels, if any


    /// orientation.
    bool m_bHorz;
    bool m_bFlipped;

    /// fonts.
    CGlBitmapFont m_Font_Helv10;

    /// scope.
    CRef<objects::CScope>   m_Scope;
    CRef<CSGSequenceDS>     m_SeqDS;

private:
    typedef set<string> TDrawnSet;
    TDrawnSet m_DrawnSet;

    TRenderingFlags m_RenderingFlags = 0;
};

///////////////////////////////////////////////////////////////////////////////
/// inline method implementations
///
inline
CGlPane* CRenderingContext::GetGlPane()
{
    return m_Pane;
}

inline
const CGlPane* CRenderingContext::GetGlPane() const
{
    return m_Pane;
}

inline
void CRenderingContext::SetSeqDS(CSGSequenceDS* ds)
{
    m_SeqDS.Reset(ds);
}

inline
CRef<CSGSequenceDS> CRenderingContext::GetSeqDS() const
{
    return m_SeqDS;
}

inline
objects::CScope& CRenderingContext::GetScope()
{
    return m_SeqDS->GetScope();
}

inline
const objects::CScope& CRenderingContext::GetScope() const
{
    return m_SeqDS->GetScope();
}

inline
void CRenderingContext::SetOverviewCutoff(int cutoff)
{
    m_OverviewCutoff = cutoff;
}

inline
void CRenderingContext::SetSelectionColor(const CRgbaColor& c)
{
    m_SelColor = c;
}

inline
const CRgbaColor& CRenderingContext::GetSelectionColor() const
{
    return m_SelColor;
}

inline
void CRenderingContext::SetSelLabelColor(const CRgbaColor& c)
{
    m_SelLabelColor = c;
}

inline
const CRgbaColor& CRenderingContext::GetSelLabelColor() const
{
    return m_SelLabelColor;
}

inline
void CRenderingContext::SetHighlightsColor(const CRgbaColor &c)
{
    m_HighlightsColor = c;
}

inline
const CRgbaColor& CRenderingContext::GetHighlightsColor() const
{
    return m_HighlightsColor;
}

inline
const TModelUnit& CRenderingContext::GetScale() const
{
    return m_Scale;
}

inline
const TModelUnit& CRenderingContext::GetOffset() const
{
    return m_Offset;
}

inline
bool CRenderingContext::IsHorizontal() const
{
    return m_bHorz;
}

inline
bool CRenderingContext::IsFlippedStrand() const
{
    return m_bFlipped;
}

inline
const TModelRange& CRenderingContext::GetVisibleLimit() const
{
    return m_LimitRange;
}

inline
const TModelRange& CRenderingContext::GetVisibleRange() const
{
    return m_VisRange;
}

inline
TModelUnit CRenderingContext::GetVisibleFrom() const
{
    return m_VisRange.GetFrom();
}

inline
TModelUnit CRenderingContext::GetVisibleTo() const
{
    return m_VisRange.GetTo();
}

inline
const TSeqRange& CRenderingContext::GetVisSeqRange() const
{
    return m_VisSeqRange;
}

inline
TSeqPos CRenderingContext::GetVisSeqFrom() const
{
    return m_VisSeqRange.GetFrom();
}

inline
TSeqPos CRenderingContext::GetVisSeqTo() const
{
    return m_VisSeqRange.GetTo();
}

inline
bool CRenderingContext::IsOverviewMode() const
{
    return GetScale() >= m_OverviewCutoff;
}

inline
bool CRenderingContext::IsInVisibleRange(TSeqPos pos) const
{
    return pos >= m_VisRange.GetFrom()  &&  pos <= m_VisRange.GetTo();
}

inline
TModelUnit CRenderingContext::GetMinLabelWidthPos() const
{
    return m_MinLabelSizePos;
}

inline
bool CRenderingContext::WillLabelFit(const TModelRect& rc) const
{
    return rc.Width() > m_MinLabelSizePos;
}

inline
bool CRenderingContext::WillLabelFit(const TModelRange& r) const
{
    return r.GetLength() > m_MinLabelSizePos;
}

inline
bool CRenderingContext::WillLabelFit(const TSeqRange& r) const
{
    return r.GetLength() > m_MinLabelSizePos;
}

inline
TModelUnit CRenderingContext::ScreenToSeq(const TModelUnit& size) const
{
    return size * GetScale();
}

inline
TModelUnit CRenderingContext::SeqToScreen(const TModelUnit& size) const
{
    return size * m_ScaleInv;
}

inline
TVPUnit CRenderingContext::SeqToScreenX(const TModelUnit& size) const
{
    TModelUnit x = SeqToScreen(size - m_Offset);
    // Return value rounded to nearest int
    return (TVPUnit)((x > 0.0) ? floor(x + 0.5) : ceil(x - 0.5));
}

inline
TVPUnit CRenderingContext::SeqToScreenXClipped(const TModelUnit& size) const
{
    // Limit by +- visible size. It ensures no co-ordinate overflow on
    // the one hand and feature's region limit will not be visible when
    // scrolling.
    TModelUnit left_limit  = (TModelUnit) 2*GetVisSeqFrom() - (TModelUnit) GetVisSeqTo();
    TModelUnit right_limit = (TModelUnit) 2*GetVisSeqTo() - (TModelUnit) GetVisSeqFrom();

    TModelUnit limited_size = size < left_limit ? left_limit : (
                              size > right_limit ? right_limit : size);
    TModelUnit x = SeqToScreen(limited_size - m_Offset);
    // Return value rounded to nearest int
    return (TVPUnit)((x > 0.0) ? floor(x + 0.5) : ceil(x - 0.5));
}

inline
TModelUnit CRenderingContext::SeqToScreenXInModelUnit(const TModelUnit& size) const
{
    TModelUnit x = size - m_Offset;
    return SeqToScreen(x);
}

inline
TModelRange CRenderingContext::IntersectVisible(const CSeqGlyph* obj) const
{
    TModelRange range(obj->GetLeft(), obj->GetLeft() + obj->GetWidth() - 1);
    return m_VisRange.IntersectionWith(range);
}

inline
TModelRange CRenderingContext::IntersectVisible(const TModelRange& r) const
{
    return m_VisRange.IntersectionWith(r);
}

inline
TModelRange CRenderingContext::IntersectVisible(const TSeqRange& r) const
{
    TModelRange range(r.GetFrom(), r.GetTo());
    return m_VisRange.IntersectionWith(range);
}

inline
bool CRenderingContext::WillLabelFit(const TModelRect& rc,
                                     const CGlBitmapFont& font,
                                     const string& label) const
{
    TModelUnit size = ScreenToSeq(font.TextWidth(label.c_str()));
    return rc.Width() > size;
}

inline
bool CRenderingContext::WillLabelFit(const TModelRect& rc,
                                     const CGlTextureFont& font,
                                     const string& label) const
{
    IRender& gl = GetGl();
    TModelUnit size = ScreenToSeq(gl.TextWidth(&font, label.c_str()));
    return rc.Width() > size;
}

inline
bool CRenderingContext::WillLabelFit(const TModelRange& r,
                                     const CGlBitmapFont& font,
                                     const string& label) const
{
    TModelUnit size = ScreenToSeq(font.TextWidth(label.c_str()));
    return r.GetLength() > size;
}

inline
bool CRenderingContext::WillLabelFit(const TModelRange& r,
                                     const CGlTextureFont& font,
                                     const string& label) const
{
    IRender& gl = GetGl();
    TModelUnit size = ScreenToSeq(gl.TextWidth(&font, label.c_str()));
    return r.GetLength() > size;
}

inline
bool CRenderingContext::WillLabelFit(const TSeqRange& r,
                                     const CGlBitmapFont& font,
                                     const string& label) const
{
    TModelUnit size = ScreenToSeq(font.TextWidth(label.c_str()));
    return r.GetLength() > size;
}

inline
bool CRenderingContext::WillLabelFit(const TSeqRange& r,
                                     const CGlTextureFont& font,
                                     const string& label) const
{
    IRender& gl = GetGl();
    TModelUnit size = ScreenToSeq(gl.TextWidth(&font, label.c_str()));
    return r.GetLength() > size;
}

inline
TModelUnit CRenderingContext::GetMaxLabelWidth(const CGlBitmapFont& font) const
{
    // maximal number of letters for label is set to 21
    return font.TextWidth("A") * 21 + 2;
}

inline
TModelUnit CRenderingContext::GetMaxLabelWidth(const CGlTextureFont& font) const
{
    // maximal number of letters for label is set to 21
    IRender& gl = GetGl();
    return gl.TextWidth(&font, "A") * 21 + 2;
}

inline
TModelUnit CRenderingContext::GetMinLabelWidthPos(const CGlTextureFont& font) const
{
    // minimal number of letters for label is set to 1
    IRender& gl = GetGl();
    return ScreenToSeq(gl.TextWidth(&font, "A") + kLabelSpacePx);
}

inline
bool CRenderingContext::WillSeqLetterFit() const
{
    // each nucleotide at least occupies 8 pixels to fit the sequence text.
    return GetScale() <= kSequenceLevel;
}

inline
bool CRenderingContext::WillTransLettersFit() const
{
    // each nucleotide at least occupies 8 pixels to fit the sequence text.
    return GetScale() <= 3.0f * kSequenceLevel;
}

inline
void CRenderingContext::DrawLine_NoAA(TModelUnit x1, TModelUnit y1,
                                      TModelUnit x2, TModelUnit y2) const
{
    IRender& gl = GetGl();

    gl.Disable(GL_LINE_SMOOTH);
    DrawLine(x1, y1, x2, y2);
    gl.Enable(GL_LINE_SMOOTH);
}

inline
void CRenderingContext::DrawStippledLine(TModelUnit x1, TModelUnit y1,
                                         TModelUnit x2, TModelUnit y2,
                                         int factor, unsigned short pattern) const
{
    IRender& gl = GetGl();

    gl.LineStipple(factor, pattern);
    gl.Enable(GL_LINE_STIPPLE);
    DrawLine_NoAA(x1, y1, x2, y2);
    gl.Disable(GL_LINE_STIPPLE);
}

inline
void CRenderingContext::DrawLineStrip(const vector<TModelPoint>& points) const
{
    IRender& gl = GetGl();

    gl.Begin(GL_LINE_STRIP);
    ITERATE (vector<TModelPoint>, iter, points) {
        gl.Vertex2d(iter->X() - m_Offset, iter->Y());
    }
    gl.End();

}

inline
void CRenderingContext::DrawTriangle(TModelUnit x1, TModelUnit y1,
                                     TModelUnit x2, TModelUnit y2,
                                     TModelUnit x3, TModelUnit y3,
                                     bool border) const
{
    IRender& gl = GetGl();

    gl.Begin(GL_TRIANGLES);
        gl.Vertex2d(x1 - m_Offset, y1);
        gl.Vertex2d(x2 - m_Offset, y2);
        gl.Vertex2d(x3 - m_Offset, y3);
    gl.End();

    if (border) {
        gl.Color3f(0.0f, 0.0f, 0.0f);
        gl.Begin(GL_LINE_STRIP);
            gl.Vertex2d(x1 - m_Offset, y1);
            gl.Vertex2d(x2 - m_Offset, y2);
            gl.Vertex2d(x3 - m_Offset, y3);
            gl.Vertex2d(x1 - m_Offset, y1);
        gl.End();
    }

}

inline
void CRenderingContext::DrawQuad(const TModelRect& rc, bool border) const
{
    DrawQuad(rc.Left(), rc.Bottom(), rc.Right(), rc.Top(), border);
}

inline
void CRenderingContext::DrawSelection(const TModelRect& rc) const
{
    DrawSelection(rc.Left(), rc.Bottom(), rc.Right(), rc.Top());
}

inline
void CRenderingContext::DrawRect(const TModelRect& rc) const
{
    DrawRect(rc.Left(), rc.Bottom(), rc.Right(), rc.Top());
}


inline
void CRenderingContext::AdjustToOrientation(TModelUnit& x) const
{
    if (IsHorizontal() == IsFlippedStrand()) {
       x = GetVisibleFrom() + GetVisibleTo() - x;
    }
}

inline
void CRenderingContext::AdjustToOrientation(TModelRect& rect) const
{
    if (IsHorizontal() == IsFlippedStrand()) {
       TModelUnit temp = rect.Left();
       rect.SetLeft(rect.Right());
       rect.SetRight(temp);
    }
}
inline
void CRenderingContext::SetViewWidth(TSeqPos view_width)
{
    m_ViewWidth = view_width;
}

inline
TSeqPos CRenderingContext::GetViewWidth() const
{
    return m_ViewWidth;
}

inline
void CRenderingContext::SetRenderingFlags(CRenderingContext::TRenderingFlags rendering_flags)
{
    m_RenderingFlags = rendering_flags;
}

inline
bool CRenderingContext::IsSkipTitleBar() const
{
    return m_RenderingFlags & fSkipTitleBar;
}
inline
bool CRenderingContext::IsSkipContent() const
{
    return m_RenderingFlags & fSkipContent;
}
inline
bool CRenderingContext::IsSkipControls() const
{
    return m_RenderingFlags & fSkipControls;
}

inline
bool CRenderingContext::IsSkipGrid() const
{
    return m_RenderingFlags & fSkipGrid;
}

inline
bool CRenderingContext::IsSkipFrames() const {
    return m_RenderingFlags & fSkipFrames;
}

END_NCBI_SCOPE

#endif  // GUI_WIDGETS_SEQ_GRAPHIC___FEAT_PANEL_RENDERING_CONTEXT__HPP
