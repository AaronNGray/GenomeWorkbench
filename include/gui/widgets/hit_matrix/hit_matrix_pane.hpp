#ifndef __GUI_WIDGETS_HIT_MATRIX___HIT_MATRIX_PANE__HPP
#define __GUI_WIDGETS_HIT_MATRIX___HIT_MATRIX_PANE__HPP

/*  $Id: hit_matrix_pane.hpp 42695 2019-04-03 19:45:15Z evgeniev $
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

#include <corelib/ncbistl.hpp>

#include <gui/widgets/hit_matrix/hit_matrix_renderer.hpp>
#include <gui/widgets/hit_matrix/hit_matrix_ds.hpp>
#include <gui/widgets/hit_matrix/hit_matrix_graph.hpp>
#include <gui/widgets/hit_matrix/seq_graph.hpp>
#include <gui/widgets/hit_matrix/hit_coloring.hpp>

#include <gui/widgets/seq_graphic/rendering_ctx.hpp>

#include <gui/widgets/gl/gl_widget_base.hpp>
#include <gui/widgets/gl/mouse_zoom_handler.hpp>
#include <gui/widgets/gl/linear_sel_handler.hpp>
#include <gui/widgets/gl/ruler.hpp>
#include <gui/widgets/gl/graph_container.hpp>

#include <gui/widgets/wx/pdf_grabber.hpp>

#include <gui/objutils/objects.hpp>

#include <gui/graph/axis.hpp>

#include <gui/print/pdf.hpp>

#include <objects/seqfeat/SeqFeatData.hpp>


BEGIN_NCBI_SCOPE

class CHitMatrixWidget;
class CHistogramGraph;

////////////////////////////////////////////////////////////////////////////////
/// class CHitMatrixPane
class NCBI_GUIWIDGETS_HIT_MATRIX_EXPORT  CHitMatrixPane :
        public CGlWidgetPane,
        public CEventHandler,
        public IMouseZoomHandlerHost,
        public IHitMatrixGraphHost,
        public ISelHandlerHost,
        public IVectorGraphicsRenderer
{
    DECLARE_EVENT_TABLE()
public:
    typedef CHitMatrixGraph::TElemGlyphSet  TElemGlyphSet;
    typedef CLinearSelHandler::TRangeColl   TRangeColl;

    /// identifies one of the two sequences in the pane
    enum    ESequence {
        eSubject,
        eQuery
    };

    CHitMatrixPane(CHitMatrixWidget* parent);
    virtual ~CHitMatrixPane();

    virtual CHitMatrixRenderer& GetRenderer()   {   return m_Renderer;  }

    virtual TVPPoint GetPortSize(void);
    void    SetWidget(CHitMatrixWidget* parent);
    virtual void    Update(void);

    //void    ColorByScore(const SHitColoringParams* params);
    //const string&    GetScoreName() const;

    void SetQueryVisibleRange(const TSeqRange& range);
    void SetSubjectVisibleRange(const TSeqRange& range);

    const   TRangeColl&     GetRangeSelection(ESequence seq) const;
    void    SetRangeSelection(const TRangeColl& coll, ESequence seq);

    void    ResetObjectSelection();
    void    GetObjectSelection(TConstObjects& objs) const;
    void    SetObjectSelection(const vector<const objects::CSeq_align*> sel_aligns);

    //TModelRect              GetSelectedHitElemsRect() const;
    CConstRef<objects::CSeq_align_set>   GetSelectedHits() const;
    void    ProjectSelectedElems(TRangeColl& coll, ESequence seq) const;

    /// @name IMouseZoomHandlerHost implementation
    /// @{
    virtual TModelUnit  MZHH_GetScale(EScaleType type);
    virtual void        MZHH_SetScale(TModelUnit scale, const TModelPoint& point);
    virtual void        MZHH_ZoomRect(const TModelRect& rc);
    virtual void        MZHH_ZoomPoint(const TModelPoint& point, TModelUnit factor);
    virtual void        MZHH_Scroll(TModelUnit d_x, TModelUnit d_y);
    virtual void        MZHH_EndOp();
    virtual TVPUnit     MZHH_GetVPPosByY(int y) const;
    /// @}

    /// @name ITooltipClient interface implementation
    /// @{
    virtual bool    TC_NeedTooltip(const wxPoint &);
    virtual string  TC_GetTooltip(const wxRect &);
    /// @}

    /// @name IHitMatrixGraphHost interface implementation
    /// @{
    virtual void     HMGH_OnChanged(void);
    virtual TVPUnit  HMGH_GetVPPosByY(int y) const;
    /// @}

    /// @name ISelHandlerHost implementation
    /// @{
    virtual void    SHH_OnChanged();
    virtual TModelUnit  SHH_GetModelByWindow(int z, EOrientation orient);
    virtual TVPUnit     SHH_GetWindowByModel(TModelUnit z, EOrientation orient);
    /// @}

    /// @name Graphs management
    /// @{
    struct SGraphDescr
    {
        string      m_Type;
        bool        m_HasColor;
        CRgbaColor  m_Color;
    };
    /*void    GetGraphTypes(vector<SGraphDescr>& types) const;
    void    SetGraphColor(const string& name, const CRgbaColor& color);

    void    GetGraphs(ESequence seq, vector<string>& graphs) const;
    void    SetGraphs(ESequence seq, const vector<string>& graphs);
*/
    void    OnSize(wxSizeEvent& event);

    /// @name IVectorGraphicsRenderer implementations
    /// @{
    virtual const TVPRect & GetViewportRect () const;
    virtual void RenderVectorGraphics (int vp_width, int vp_height);
    virtual void UpdateVectorLayout ();
    virtual void AddTitle(bool b);
    /// @}

protected:
    /*void    x_SetupAxes();

    void    x_SetupGraphTypes();
    CHistogramGraph*    x_CreateHistogram(ESequence seq, const SGraphDescr& descr);
    void    x_AddGraph(CGraphContainer& cont, IRenderable* graph, bool horz);
    void    x_SetGraphs(ESequence seq, const vector<string>& graphs);
    void    x_UpdateGraphColor(IRenderable* graph);
    void    x_DestroyGraphs(CGraphContainer& cont);
    void    x_SetRulerText();*/

    /// CGlWidgetPane overridables
    virtual void    x_Render(void);

    //virtual bool    Layout(void);

    // Rendering functions
    void    x_RenderContent(void);
    void    x_RenderAxisAndGrid(CGlPane& gr_pane);
    void    x_RenderContainers();
    void    x_RenderSelHandler();
    void    x_RenderMouseZoomHandler(CGlPane& pane);
    void    x_RenderSelHandler(bool b_horz, CGlPane& pane, CLinearSelHandler::ERenderingOption option);

    /// adjusts limits and visible rect of the given pane to the master pane
    void    x_AdjsutToMasterPane(CGlPane& pane, bool b_model_x, bool b_model_y);

    virtual int     x_GetAreaByVPPos(TVPUnit vp_x, TVPUnit vp_y);

    CHitMatrixWidget*   x_GetParent() {   return (CHitMatrixWidget*) GetParent(); }
    const CHitMatrixWidget*   x_GetParent() const {   return (CHitMatrixWidget*) GetParent(); }

protected:
    CHitMatrixWidget*   m_Parent; // parent widget

    CHitMatrixRenderer m_Renderer;

    // event handlers
    CMouseZoomHandler   m_MouseZoomHandler;
    CLinearSelHandler   m_HorzSelHandler;
    CLinearSelHandler   m_VertSelHandler;
    CTooltipHandler     m_TooltipHandler;

    int m_TooltipArea;
};

/*
inline const string&    CHitMatrixPane::GetScoreName() const
{
    static string s_emp;
    return m_HitColoringParams ? m_HitColoringParams->m_ScoreName : s_emp;
}*/


END_NCBI_SCOPE

#endif  // __GUI_WIDGETS_HIT_MATRIX___HIT_MATRIX_PANE__HPP
