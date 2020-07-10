#ifndef GUI_WIDGETS_SEQ_GRAPHIC___TRACE_GLYPH__HPP
#define GUI_WIDGETS_SEQ_GRAPHIC___TRACE_GLYPH__HPP

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
 *    CTraceGlyph -- utility class for Trace Chromatograms.
 *
 */


#include <corelib/ncbiobj.hpp>
#include <objects/seqloc/Seq_interval.hpp>
#include <gui/utils/rgba_color.hpp>
#include <gui/widgets/seq_graphic/trace_graph_ds.hpp>
#include <gui/widgets/seq_graphic/seq_glyph.hpp>
#include <gui/widgets/seq_graphic/histogram_conf.hpp>
#include <gui/widgets/gl/attrib_menu.hpp>
#include <math.h>
#include <array>

BEGIN_NCBI_SCOPE

class CTraceGraphConfig : public CObject
{
public:
    /// chromatogram rendering mode
    enum    ESingnalStyle
    {
        eCurve,
        eIntensity
    };

    enum    EGraphState
    {
        eHidden,
        eCollapsed,
        eExpanded
    };
    enum EColors
    {
        eA_trace = 0,
        eC_trace = 1,
        eG_trace = 2,
        eT_trace = 3
    };
public:
    CTraceGraphConfig()
    {
        m_colors[eA_trace] = CRgbaColor("255 0 0"); // A channel
        m_colors[eC_trace] = CRgbaColor("0 255 0"); // C channel
        m_colors[eG_trace] = CRgbaColor("0 0 255"); // G channel
        m_colors[eT_trace] = CRgbaColor("128 0 128"); // T channel
    }
    ESingnalStyle   m_SignalStyle = eCurve;

    EGraphState m_ConfGraphState = eExpanded;
    EGraphState m_SignalGraphState = eExpanded;
    bool    m_bReverseColors = false; // reverse colors for signals measured on back strand
    // so that they correspond to assembly sequence rather than to trace sequence
    int m_Height = 70;
    array<CRgbaColor, 4> m_colors;
    CRgbaColor m_colorConfMin = CRgbaColor("0 128 0");
    CRgbaColor m_colorConfMax = CRgbaColor("0 192 64");
};



class NCBI_GUIWIDGETS_SEQGRAPHIC_EXPORT CTraceGlyph : public CSeqGlyph
{
public:
    /// @name ctors
    /// @{
    CTraceGlyph(const CRef<CSGTraceData>& data, const CRef<CTraceGraphConfig>& config);

    ~CTraceGlyph();
    /// @}

    /// @name CSeqGlyph virtual methods.
    /// @{
    virtual bool OnLeftDblClick(const TModelPoint& /*p*/);
    virtual bool NeedTooltip(const TModelPoint& p, ITooltipFormatter& tt, string& t_title) const;
    virtual void GetTooltip(const TModelPoint& /*p*/, ITooltipFormatter& tt, string& t_title) const;
    virtual void GetHTMLActiveAreas(TAreaVector* p_areas) const;
    virtual bool IsClickable() const;
    ///@}

    // access the position of this object.
    //virtual TSeqRange GetRange(void) const;

    const string& GetTitle() const; 
    void SetDesc(const string& desc);
    const string& GetDesc() const;

    void SetShowTitle(bool f);
    void SetDialogHost(IGlyphDialogHost* host);

protected:
    virtual void x_Draw() const;
    virtual void x_UpdateBoundingBox();
protected:
    int x_GetConfGraphH() const;
    int x_GetSignalGraphH() const;

protected:
    IGlyphDialogHost*               m_DlgHost;
    CRef<CTraceGraphConfig>         m_Config;
    CRef<CSGTraceData>              m_Data;

    /// Key for accessing histogram settings.
    /// It is used only when m_Subtype is eSubtype_any.
    string                          m_Desc;
    bool                            m_ShowTitle;

    mutable vector<CRgbaColor>    m_vSignalColors;
    void x_InitColors() const;

    void x_RenderContour(CGlPane& pane, int y, int top_h, int total_h) const;

    // renders confidence graph
    void x_RenderConfGraph(CGlPane& pane, int y, int h) const;

    void x_RenderSignalGraph(CGlPane& pane, int y, int h) const;

    void x_RenderCurveSegment(CGlPane& pane,
                              const CSGTraceData::TPositions& positions,
                              const CSGTraceData::TValues& values,
                              int bottom_y, int /*h*/, int amp)  const;

    void x_RenderIntensityGraphs(CGlPane& pane, int y, int h)  const;

    const CRgbaColor& GetColorByValue(double value, int signal) const;

    int x_FindSampleToLeft(double pos) const;
    int x_FindSampleToRight(double pos) const;

};

///////////////////////////////////////////////////////////////////////////////
/// CTraceGlyph inline method implementation.
static const string kTitle = "Traces";
inline
const string& CTraceGlyph::GetTitle() const
{ return kTitle; }


inline
void CTraceGlyph::SetDesc(const string& desc)
{ m_Desc = desc; }

inline
const string& CTraceGlyph::GetDesc() const
{ return m_Desc; }


inline
void CTraceGlyph::SetDialogHost(IGlyphDialogHost* host)
{ m_DlgHost = host; }


END_NCBI_SCOPE

/* @} */

#endif  // GUI_WIDGETS_SEQ_GRAPHIC___TRACE_GLYPH__HPP
