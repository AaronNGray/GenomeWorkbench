#ifndef GUI_WIDGETS_ALN_CROSSALN___CROSS_ALN_RENDER__HPP
#define GUI_WIDGETS_ALN_CROSSALN___CROSS_ALN_RENDER__HPP

/*  $Id: cross_aln_render.hpp 15681 2008-01-09 18:30:28Z lebedev $
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
 *   Renderer for the CrossAlignment widget
 *   Independent from FLTK.
 *
 */

#include <gui/gui.hpp>
#include <gui/opengl.h>


#include <gui/widgets/gl/ruler.hpp>

#include <gui/widgets/aln_crossaln/cross_aln_ds.hpp>

#include <gui/widgets/aln_crossaln/cross_aln_widget.hpp>
#include <gui/widgets/aln_crossaln/cross_panel_ir.hpp>
#include <gui/objutils/utils.hpp>
#include <gui/objutils/objects.hpp>


BEGIN_NCBI_SCOPE
USING_SCOPE(objects);



class CCrossAlnRenderer : public CObject
{
public:
    CCrossAlnRenderer();
    ~CCrossAlnRenderer();

    void SetDataSource(const TVPRect& rc, ICrossAlnDataSource* ds);

    void Update();

    // Zoom set from 0 to 1
    float GetZoomX_Q(void) const;
    void  SetZoomX_Q(float value);

    float GetZoomX_S(void) const;
    void  SetZoomX_S(float value);


    // scales the panels
    void SetScaleRef_Q(TModelUnit m_x, TModelUnit m_y, const TModelPoint& point);
    void SetScaleRef_S(TModelUnit m_x, TModelUnit m_y, const TModelPoint& point);
    void SetScaleRef_C(TModelUnit m_x, TModelUnit m_y, const TModelPoint& point);

    void ColorBySegments();
    void ColorByScore(CConstRef<CObject_id> score_id);
    CConstRef<CObject_id> GetScoreId() const;

    void ZoomOnRange_Q(TModelUnit from, TModelUnit to);
    void ZoomOnRange_S(TModelUnit from, TModelUnit to);

    void SquareOnRect (TModelRect rc);

    void ResetObjectSelection();
    void GetObjectSelection(TConstObjects& objs) const;
    void SetObjectSelection(const vector<const CSeq_align*> sel_aligns);

    // Query
    void ZoomIn_Q();
    void ZoomOut_Q();
    void ZoomAll_Q();
    void ZoomToSeq_Q();

    // Subject
    void ZoomIn_S();
    void ZoomOut_S();
    void ZoomAll_S();
    void ZoomToSeq_S();

    // Access CGlPane(s)
    CGlPane& GetRP_Q();
    CGlPane& GetCP();
    const CGlPane& GetCP() const;
    CGlPane& GetRP_S();

    void SetViewport(const TVPRect& rc);
    void Render();

    TModelRect HitTest(int x, int y);
    TModelRect SelectHitTest(int x, int y);

    void GetTooltip(int x, int y, string* title);

    void Scroll(TModelUnit dxQ, TModelUnit dxS);

private:
    CIRef<ICrossAlnDataSource> m_DS;

    CGlPane m_RP_Q;
    CGlPane m_CP; // panel for cross-alignment with tricky model space
    CGlPane m_RP_S;

    CRuler         m_RulerPanel_Q;
    CCrossPanelIR  m_CrossPanel;
    CRuler         m_RulerPanel_S;

    TSeqPos m_SeqLength_Q; // sequence lengths
    TSeqPos m_SeqLength_S;

    void x_SetupViewPorts(const TVPRect& rc);
    void x_SetupModelLimits();
    void x_AdjustCrossPane();

    TModelUnit m_MinZoomX, m_MaxZoomX_Q, m_MaxZoomX_S; // X axis zoom limits
};




END_NCBI_SCOPE

#endif  /* GUI_WIDGETS_ALN_CROSSALN___CROSS_ALN_RENDER__HPP */
