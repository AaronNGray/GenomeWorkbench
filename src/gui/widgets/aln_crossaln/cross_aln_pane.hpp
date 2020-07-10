#ifndef GUI_WIDGETS_ALN_CROSSALN___CROSS_ALN_PANE__HPP
#define GUI_WIDGETS_ALN_CROSSALN___CROSS_ALN_PANE__HPP

/*  $Id: cross_aln_pane.hpp 17877 2008-09-24 13:03:09Z dicuccio $
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
 * Authors:  Vlad Lebedev
 *
 * File Description:
 *   Panel for the cross alignment view widget
 *   (see <gui/widgets/aln_crossaln/cross_aln_widget.hpp>)
 *
 */

#include "cross_aln_render.hpp"

#include <gui/widgets/aln_crossaln/cross_aln_ds.hpp>
#include <gui/widgets/aln_crossaln/cross_aln_widget.hpp>

#include <gui/widgets/gl/gl_widget_base.hpp>
#include <gui/widgets/gl/linear_sel_handler.hpp>
#include <gui/widgets/gl/mouse_zoom_handler.hpp>


BEGIN_NCBI_SCOPE


//class CCrossAlnWidget;


class NCBI_GUIWIDGETS_ALNCROSSALN_EXPORT CCrossAlnPane :
                            public CGlWidgetPane,
                            public CEventHandler,
                            public IMouseZoomHandlerHost,
                            public ISelHandlerHost
{
    DECLARE_EVENT_TABLE()

public:
    typedef CLinearSelHandler::TRangeColl TRangeColl;

    CCrossAlnPane(CCrossAlnWidget* parent);
    virtual ~CCrossAlnPane();

    void SetDataSource(ICrossAlnDataSource* ds);
    ICrossAlnDataSource* GetDataSource();

    virtual void Update();


    void ResetObjectSelection();
    void GetObjectSelection(TConstObjects& objs) const;
    void SetObjectSelection(const vector<const CSeq_align*> sel_aligns);

    void ZoomOnRange_Q (TModelUnit from, TModelUnit to);
    void ZoomOnRange_S (TModelUnit from, TModelUnit to);

    void ZoomOnRect(TModelRect rect);
    void SquareOnRect(TModelRect rect);

    // Query
    void ZoomIn_Q();
    void ZoomOut_Q();
    void ZoomAll_Q();
    void ZoomToSeq_Q();

    // Subject
    void ZoomIn_S ();
    void ZoomOut_S();
    void ZoomAll_S();
    void ZoomToSeq_S();

    // All
    void ZoomAll   (void);
    void ZoomIn    (void);
    void ZoomOut   (void);


    void Scroll(TModelUnit dxQ, TModelUnit dxS);

    void ColorBySegments();
    void ColorByScore(CConstRef<CObject_id> score_id);
    CConstRef<CObject_id> GetScoreId() const;

    const TRangeColl& GetSubjectRangeSelection() const;
    const TRangeColl& GetQueryRangeSelection()   const;
    void  SetSubjectRangeSelection(const TRangeColl& coll);
    void  SetQueryRangeSelection(const TRangeColl& coll);

    /// @name ITooltipClient implementation
    /// @{
    virtual bool   TC_NeedTooltip(const wxPoint &);
    virtual string TC_GetTooltip(const wxRect &);
    /// @}

    /// @name CGlWidgetPane implementation
    /// @{
    virtual void x_Render();
    virtual TVPPoint GetPortSize();
    /// @}

    void OnLeftDown(wxMouseEvent& event);
    void OnLeftDblClick(wxMouseEvent& event);

    // scrollbar adjustments
    double NormalizeQ(int pos);
    double NormalizeS(int pos);

    CCrossAlnWidget*        x_GetParent() { return (CCrossAlnWidget*) GetParent(); }
    const CCrossAlnWidget*  x_GetParent() const { return (CCrossAlnWidget*) GetParent(); }

    /// @name IMouseZoomHandlerHost implementation
    /// @{
    virtual TModelUnit  MZHH_GetScale(EScaleType type);
    virtual void        MZHH_SetScale(TModelUnit scale, const TModelPoint& point);
    virtual void        MZHH_ZoomRect(const TModelRect& rc);
    virtual void        MZHH_ZoomPoint(const TModelPoint& point, TModelUnit factor);
    virtual void        MZHH_Scroll(TModelUnit d_x, TModelUnit d_y);
    virtual TVPUnit     MZHH_GetVPPosByY(int y) const;
    virtual void        MZHH_EndOp();
    /// @}

    /// @name ISelHandlerHost implementation
    /// @{
    virtual void        SHH_OnChanged();
    virtual TModelUnit  SHH_GetModelByWindow(int z, EOrientation orient);
    virtual TVPUnit     SHH_GetWindowByModel(TModelUnit z, EOrientation orient);
    /// @}

    CRef<CCrossAlnRenderer> GetRDR() { return m_Renderer; }

protected:
    enum EHandlerArea {
        fRuler1Area     = 0x1,
        fCrossAlignArea1 = 0x2,
        fCrossAlignArea2 = 0x4,
        fRuler2Area     = 0x8,
        fOther          = 0x16,
        fCrossAlign     = fCrossAlignArea1 | fCrossAlignArea2,
        fAllAreas       = fRuler1Area | fCrossAlignArea1 |
                          fCrossAlignArea2 | fRuler2Area | fOther
    };

    virtual int x_GetAreaByVPPos(int vp_x, int vp_y);
    bool x_IsQueryHalf(wxPoint point);


    void x_UpdateOnZoom();
    //void x_SetPortLimits();

    // Datasource
    CIRef<ICrossAlnDataSource>  m_DS;

    // Renderer
    CRef<CCrossAlnRenderer> m_Renderer;

    // Handlers
    CLinearSelHandler m_SelHandler_Q;
    CLinearSelHandler m_SelHandler_S;
    CMouseZoomHandler m_MouseZoomHandler;
    CTooltipHandler   m_TooltipHandler;

    bool m_QueryHalf;
    string m_TooltipText;
};


END_NCBI_SCOPE

#endif  /* GUI_WIDGETS_ALN_CROSSALN___CROSS_ALN_PANE__HPP */
