#ifndef GUI_WIDGETS_GL___RULER__HPP
#define GUI_WIDGETS_GL___RULER__HPP

/*  $Id: ruler.hpp 35505 2016-05-16 15:18:00Z shkeda $
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
 * Authors:  Andrey Yazhuk
 *
 * File Description:
 *
 */

#include <gui/opengl/gltexturefont.hpp>
#include <gui/opengl/glpane.hpp>
#include <gui/widgets/gl/irenderable.hpp>

#include <util/align_range.hpp>
#include <util/align_range_coll.hpp>


BEGIN_NCBI_SCOPE

////////////////////////////////////////////////////////////////////////////////
/// CRuler is a renderable object drawing a scale with position labels.

class NCBI_GUIWIDGETS_GL_EXPORT CRuler : public IRenderable
{
public:
    enum EColorType {
        eRuler,
        eText,
        eBackground
    };

    /// Ticks and labels placement.
    /// The geometry names and their relative positions are:
    ///
    ///       Label(top)
    ///         | <-- opposite label ticks
    ///         |         |  <-- opposite major ticks
    /// | | | | | | | | | | | | | | <-- oppsite minor ticks
    ///------------------------------------------------------------------
    /// | | | | | | | | | | | | | | <-- minor ticks
    ///         |         | <-- major ticks
    ///         | <-- label ticks
    ///
    /// Here are the rules:
    /// 1. Major/minor ticks are are always on the opposite side of labels, 
    ///    so the Opposite Major/Minor ticks will be on the same side
    ///    with labels.
    /// 2. Label position is configurable.  If label placement is top,
    ///    then the major/minor ticks will face down, and the opposite
    ///    major/minor ticks will face up.
    /// 3. The tick sizes can also be modified to create different tick
    ///    orientations.  E.g. setting major/minor ticks to zero will 
    ///    create a ruler with tick facing label side always.
    /// 4. Major/Opp tick sizes >= Minor/Opp tick sizes
    /// 5. Label/Opp tick sizes >= Major/Opp tick sizes

    enum EGeometryParam {
        eMinorTickHeight,
        eMajorTickHeight,
        eLabelTickHeight,     ///< tick size at label position
        eOppMinorTickHeight,  ///< minor tick on the opposite side
        eOppMajorTickHeight,  ///< major tick on the opposite side
        eOppLabelTickHeight   ///< tick size at label position (opposite)
    };

    enum ELabelPlacement {
        eBottom,
        eTop,
        eLeft,
        eRight,
        eDefault
    };

    /// How labels align around ticks.
    enum ELabelAlign {
        eAln_Center,
        eAln_Left,   ///< valid for horizontal mode only (mapped to eAln_Bottom for vertical mode)
        eAln_Right,  ///< valid for horizontal mode only (mapped to eAln_Top for vertical mode)
        eAln_Top,    ///< valid for vertical mode only (mapped to eAln_Right for horizontal mode)
        eAln_Bottom  ///< valid for vertical mode only (mapped to eAln_Left for horizontal mode)
    };

    enum EDisplayOptions    {
        fHideLabels     = 0x01,  // do not render labels along the scale
        fShowOrigin     = 0x02,  // draw "Origin" label
        fShowMetric     = 0x04,  // draw "Metric"
        fShowTextLabel  = 0x08,  // draw text label
        fFillBackground = 0x10,  // 
        fHideNegative   = 0x20,  // hide negative coordinate
        fHideFirstLabel = 0x40,  // hide the first scale label
        fHideLastLabel  = 0x80,  // hide the last scale label
        fFirstLabelHasText = 0x100, // draw text label next to fisrt label
        fDefaultDisplayOptions = fFillBackground
    };

    typedef TSignedSeqPos  TPos;
    typedef CAlignRange<TPos>   TAlignRange;
    typedef CRange<TPos>        TRange;
    typedef CAlignRangeCollection<TAlignRange>    TAlignColl;

    /// Origin specifies the position in the model space that is represented as "1"
    /// in the Ruler's display space. It can be thought of as the offset of the
    /// local coordinate system associated with Ruler relative to the global coordinate system.

    CRuler(bool horz = true);
    virtual ~CRuler();

    void    SetHorizontal(bool b_horz, ELabelPlacement place = eDefault,
        ELabelAlign aln = eAln_Center);
    void    SetColor(EColorType type, const CRgbaColor& color);
    void    SetFont(CGlTextureFont::EFontFace font_type, 
                    unsigned int font_size=12);
    void    SetLabelOrientation(CGlTextureFont::EFontRotateFlags rotate, 
                                int rotate_degrees);
    void    SetGeometryParam(EGeometryParam geom, int value);

    // takes a combination of EDisplayOptions flags
    void    SetDisplayOptions(int options);

    /// SetAutoRange() activates automatic mode; in this mode ruler's range is equal to
    /// the provided model limits range (obtained via CGlPane::GetModelLimitsRect()).
    void    SetAutoRange();

    /// SetRange() activates "manual" mode; in this mode ruler's range in
    /// model space is explicitly limited to [Start, End], SeqStart specifies the
    /// number corresponding to the first position in the range;
    /// if b_reverse == "true" then SeqStart will correspond to End and
    /// displayed numbers will increase from the right to the left.
    void    SetRange(int start, int end, int seq_start, bool reverse);

    void    SetMapping(const TAlignColl& coll);
    void    SetBaseWidth(int value);

    void    SetTextLabel(const string& label);

    TVPPoint    GetPreferredSize(int max_num = 0) const;

    /// Get the distance between two tick labels.
    int     GetLabelStep() const;

    void    Update(CGlPane& pane);

    /// @name IRenderable implementation
    /// @{
    virtual void Render(CGlPane& pane);

    virtual TVPRect    GetVPRect() const;
    virtual void       SetVPRect(const TVPRect& rect);

    virtual TModelRect  GetModelRect() const;
    virtual void        SetModelRect(const TModelRect& rc);

    virtual TVPPoint PreferredSize();

    virtual bool    IsVisible();
    virtual void    SetVisible(bool set);

    virtual bool    NeedTooltip(CGlPane& pane, int vp_x, int vp_y);
    virtual string  GetTooltip();
    virtual void SetOrder(int order);
    virtual int GetOrder() const;

    /// @}

protected:
    bool    x_TextAlongAxis() const;

    int     x_ToDisplay(const TAlignRange& range, int model) const;
    TModelUnit     x_ToModel(const TAlignRange& range, int display) const;
    void x_CalcStartStopOffsets(const TAlignRange& range, TModelUnit& from_offset, TModelUnit& to_offset);

    void    x_CalculatePosLabelsStep(CGlPane& Pane);
    void    x_ChooseTickSpace(double scale);

    void    x_UpdateMappingByPane(CGlPane& pane);
    void    x_UpdatePosLabelsStep(CGlPane& pane);

    void    x_RenderRange(CGlPane& pane, const TAlignRange& range, const TRange& clip_r);
    void    x_RenderBackground(CGlPane& pane, const TModelRect& rc, const TRange& clip_r);
    void    x_RenderScale(CGlPane& pane, const TAlignRange& range, const TRange& clip_r);
    void    x_RenderAllPosLabels(CGlPane& pane, const TAlignRange& range, const TRange& clip_r);
    void    x_RenderOriginAndMetric(CGlPane& pane, const TRange& clip_r);

    string  x_GetPositionLabel(const TAlignRange& range, int iElem);
    void    x_GenerateLabelPositions(const TAlignRange& range,
                                     int first_elem, int last_elem, vector<TModelUnit>& vElemPos);

    void    x_RenderPosLabel(CGlPane& pane, double pos_u,
                             double label_offset_u, const string& text,
                             bool draw_tick = true);
    void    x_RenderHorzPosLabel(CGlPane& pane, double pos_u,
                             double label_offset_u, const string& text,
                             bool draw_tick = true);
    void    x_RenderVertPosLabel(CGlPane& pane, double pos_u,
                             double label_offset_u, const string& text,
                             bool draw_tick = true);

    int     x_GetMaxNum();

    int     x_GetTicksLabelsSizeV(int max_num) const;
    int     x_GetOriginMetricSizeV() const;
    
    TModelPoint x_GetLabelSize(CGlPane& pane, const string& label);
    TModelPoint x_GetLabelSizeUnscaled(CGlPane& pane, const string& label);


protected:
    bool    m_Visible;
    bool    m_Horz;
    ELabelPlacement m_LabelPlace;
    ELabelAlign     m_LabelAln;

    /// Font rotation flags (rotate around base or cap)
    CGlTextureFont::EFontRotateFlags  m_FontRotate;
    /// Use degrees as int for safe comparison to 0, 90, 180 etc.
    int m_FontRotateDegrees;


    TVPRect m_rcBounds;

    bool m_AutoRange;
    //int  m_Start, m_End; /// range in model space represented by the ruler
    //int  m_Offset; /// added to m_Start to produce displayed numbers, so that
    /// range displayed is [m_Start + m_Offset, m_End + m_Offset]

    // this is a collection that defines mapping from the Ruler's model space
    // (as defined by a given CGlPane) to the "display" space
    // labels and ticks will be rendered only for intervals existing in the
    // collection.
    TAlignColl  m_Mapping;

    int m_BaseWidth;
    float m_BaseOffset;

    string m_TextLabel;

    //bool    m_ReverseDisplay;

    int     m_DisplayOptions;

    CGlTextureFont  m_Font;

    CRgbaColor    m_TextColor;
    CRgbaColor    m_RulerColor;
    CRgbaColor    m_BackColor;

    /// @name tick sizes
    /// @{
    int     m_MajorTickSize;
    int     m_MinorTickSize;
    int     m_LabelTickSize;
    int     m_OppMajorTickSize;
    int     m_OppMinorTickSize;
    int     m_OppLabelTickSize;
    /// @}

    bool    m_Dirty; /// "true" if parameters affecting layout have been changed

    TModelRect  m_rcLimits;
    double m_ScaleX, m_ScaleY;

    double m_MaxLabelW;
    double m_MaxLabelH;

    int m_BaseStep; // has form 10^X,  m_PosLabelsStep = K * m_BaseStep;
    int m_PosLabelsStep; // distance between two labels (in model coords)
    int m_TickSpace;     // distance beween two minor ticks (in model coords)
};


////////////////////////////////////////////////////////////////////////////////
/// CRuler inline methods
inline
int CRuler::GetLabelStep() const
{ return m_PosLabelsStep; }

inline
void CRuler::Update(CGlPane& pane)
{  x_UpdatePosLabelsStep(pane); }


inline
void CRuler::SetBaseWidth(int value) 
{ 
    _ASSERT(value > 0);
    m_BaseWidth = value > 0 ? value : 1; 
    m_BaseOffset = m_BaseWidth/2.;

}
inline
void CRuler::SetOrder(int order)
{
}
inline
int CRuler::GetOrder() const
{
    return 0;
}

END_NCBI_SCOPE

#endif  // GUI_WIDGETS_GL___RULER__HPP
