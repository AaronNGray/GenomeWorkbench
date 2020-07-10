#ifndef GUI_WIDGET_ALN_CROSSALN___CROSS_PANEL_IR__HPP
#define GUI_WIDGET_ALN_CROSSALN___CROSS_PANEL_IR__HPP

/*  $Id: cross_panel_ir.hpp 31688 2014-11-05 17:18:12Z falkrb $
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
 * Description: A panel that will render a cross alignment panel
 *
 */


#include <gui/gui.hpp>
#include <objmgr/bioseq_handle.hpp>
#include <gui/widgets/gl/irenderable.hpp>
#include <gui/widgets/aln_crossaln/cross_aln_ds.hpp>
#include <gui/widgets/hit_matrix/hit_matrix_graph.hpp>

#include <gui/opengl/gltexturefont.hpp>
#include <gui/widgets/aln_crossaln/layout.hpp>
#include <gui/objutils/objects.hpp>

BEGIN_NCBI_SCOPE




class CCrossPanelIR : public CRenderableImpl
{
public:
    typedef CHitMatrixGraph::THitGlyphVector THitGlyphVector;
    typedef CHitMatrixGraph::TElemGlyphSet   TElemGlyphSet;

    CCrossPanelIR();
    virtual ~CCrossPanelIR(void);

    /// @name IRenderable implementation
    /// @{
    virtual void Render(CGlPane& pane);
    /// @}

    void SetDataSource(ICrossAlnDataSource* ds);

    void ResetObjectSelection();
    void GetObjectSelection(TConstObjects& objs) const;
    void SetObjectSelection(const vector<const objects::CSeq_align*> sel_aligns);

    void ColorBySegments();
    void ColorByScore(CConstRef<objects::CObject_id> score_id);
    CConstRef<objects::CObject_id> GetScoreId() const { cout << "get\n"; return m_ColorScoreId; }

    void SetLimits(TVPRect rcm1, TModelRect rcv1, TVPRect rcm2, TModelRect rcv2);

    TModelRect HitTest(CGlPane& pane, int x, int y, bool select = false);

private:
    // coordinate transformation
    TModelUnit x_Seq2Lim1(TModelUnit seq);
    TModelUnit x_Seq2Lim2(TModelUnit seq);

    TModelUnit x_Lim2Seq1(TModelUnit lim);
    TModelUnit x_Lim2Seq2(TModelUnit lim);

    double x_SelectBaseStep(double MinV, double MaxV) const;

    void x_DrawCrossAlignment(CGlPane& pane);
    void x_DrawCrossedPanel(CGlPane& pane, const CRgbaColor& color,
                            TModelUnit x_from1, TModelUnit x_to1,
                            TModelUnit x_from2, TModelUnit x_to2);

    void x_DrawSequence(CGlPane& pane);

    void x_GetSequence(const objects::CBioseq_Handle& handle,
            TSeqPos from, TSeqPos to, string& buffer) const;

    bool x_IsSeqLettersFit1() const;
    bool x_IsSeqLettersFit2() const;
private:
    CIRef<ICrossAlnDataSource>  m_DS;

    CGlTextureFont m_Font_Fixed12;

    CConstRef<objects::CObject_id> m_ColorScoreId; // identifies a score to color by

    TVPRect m_RCM1;
    TVPRect m_RCM2;
    TModelRect m_RCV1;
    TModelRect m_RCV2;

    CHitMatrixGraph m_Graph;
};


END_NCBI_SCOPE

#endif  // GUI_WIDGET_ALN_CROSSALN___CROSS_PANEL_IR__HPP
