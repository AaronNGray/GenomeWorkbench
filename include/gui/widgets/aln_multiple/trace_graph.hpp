#ifndef __GUI_WIDGETS_ALNMULTI___ALIGN_TRACE_GRAPH__HPP
#define __GUI_WIDGETS_ALNMULTI___ALIGN_TRACE_GRAPH__HPP

/*  $Id: trace_graph.hpp 31702 2014-11-06 19:57:35Z falkrb $
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

#include <gui/opengl/glpane.hpp>

#include <gui/widgets/aln_multiple/alnvec_row_graph.hpp>
#include <gui/widgets/aln_multiple/trace_data.hpp>
#include <gui/widgets/gl/irenderable.hpp>

#include <gui/opengl/gltexturefont.hpp>

BEGIN_NCBI_SCOPE


////////////////////////////////////////////////////////////////////////////////
/// CTraceGraphProperties
class NCBI_GUIWIDGETS_ALNMULTIPLE_EXPORT CTraceGraphProperties
    :   public  IAlnRowGraphProperties
{
public:
    /// chromatogram rendering mode
    enum    ESingnalStyle   {
        eCurve,
        eIntensity
    };

    enum    EGraphState {
        eHidden,
        eCollapsed,
        eExpanded
    };
public:
    ESingnalStyle   m_SignalStyle;

    EGraphState m_ConfGraphState;
    EGraphState m_SignalGraphState;

    bool    m_bReverseColors; // reverse colors for signals measured on back strand
    // so that they correspond to assembly sequence rather than to trace sequence

    CTraceGraphProperties();
    CTraceGraphProperties&  operator=(const CTraceGraphProperties& orig);
};


////////////////////////////////////////////////////////////////////////////////
/// CTraceGraph - trace data renderer.
/// Renders confidence graph and A, C, T, G chromatograms using CTraceData as
/// datasource.

class NCBI_GUIWIDGETS_ALNMULTIPLE_EXPORT CTraceGraph :
    public CObject,
    public CRenderableImpl,
    public IAlnVecRowGraph
{
public:
    typedef CTraceGraphProperties::ESingnalStyle   ESingnalStyle;
    typedef CTraceGraphProperties::EGraphState     EGraphState;

    CTraceGraph(const objects::CBioseq_Handle& handle, bool b_neg_strand);

    /// @name IRenderable implementation
    /// @{
    virtual void    Render(CGlPane& pane);

    virtual TVPPoint PreferredSize();

    virtual bool    NeedTooltip(CGlPane& pane, int vp_x, int vp_y);
    virtual string  GetTooltip();
    /// @}

    /// @name IAlnVecRowGraph implementaion
    /// @{
    virtual ~CTraceGraph();

    //virtual bool    HasData() const;
    virtual bool    IsCreated() const;
    virtual bool    Create();
    virtual void    Destroy();
    /// No need to update data for track graph.
    virtual void    Update(double /*start*/, double /*stop*/) {};

    virtual const IAlnRowGraphProperties*     GetProperties() const;
    virtual void    SetProperties(IAlnRowGraphProperties* props);

    virtual void    Render(CGlPane& pane, IAlnSegmentIterator& it);
    /// @}

    void    SetConfGraphState(EGraphState state);
    void    SetSignalGraphState(EGraphState state);

protected:
    int     x_GetConfGraphH() const;
    int     x_GetSignalGraphH() const;

    void    x_RenderText(CGlPane& pane);
    void    x_RenderContour(CGlPane& pane, int y,
                            int conf_h, int total_h,
                            IAlnSegmentIterator& it);

    void    x_RenderConfGraph(CGlPane& pane, int y, int h,
                              IAlnSegmentIterator& it);

    void    x_RenderSignalGraph(CGlPane& pane, int y, int h,
                                IAlnSegmentIterator& it);

    void    x_RenderCurveSegment(CGlPane& pane,
                                const IAlnSegment& seg,
                                const CTraceData::TPositions& positions,
                                const CTraceData::TValues& values,
                                int bottom_y, int h, int amp);

    void    x_RenderIntensityGraphs(CGlPane& pane, int y, int h,
                                    IAlnSegmentIterator& it);

    const CRgbaColor&     GetColorByValue(double value, int signal) const;
    int x_FindSampleToLeft(double pos) const;
    int x_FindSampleToRight(double pos) const;

protected:
    CTraceDataProxy m_DataProxy;
    CTraceData*     m_Data;

    CTraceGraphProperties   m_Props;

    vector<CRgbaColor>    m_vSignalColors;

    CGlTextureFont   m_Font;
    CRgbaColor    m_TextColor;
};

END_NCBI_SCOPE

#endif  // __GUI_WIDGETS_ALNMULTI___ALIGN_TRACE_GRAPH__HPP
