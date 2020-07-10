/*  $Id: cross_panel_ir.cpp 42767 2019-04-10 20:59:16Z katargir $
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
 */


#include <ncbi_pch.hpp>
#include <gui/widgets/aln_crossaln/cross_panel_ir.hpp>
#include <objmgr/seq_vector.hpp>
#include <gui/objutils/utils.hpp>
#include <gui/utils/intersect.hpp>
#include <corelib/ncbitime.hpp>
#include <gui/opengl/irender.hpp>


BEGIN_NCBI_SCOPE
USING_SCOPE(objects);
USING_SCOPE(math);

static const TModelUnit   kSeqMapBarHeight = 3.0f;
static const int sc_StepK[] = { 2, 5, 10 };
static const TModelUnit kClipMultiple = 5.0;


CCrossPanelIR::CCrossPanelIR()
    : m_Font_Fixed12(CGlTextureFont::eFontFace_Fixed, 12)
{
}


CCrossPanelIR::~CCrossPanelIR()
{
}



void CCrossPanelIR::SetLimits(TVPRect rcm1, TModelRect rcv1, TVPRect rcm2, TModelRect rcv2)
{
    m_RCM1 = rcm1;
    m_RCV1 = rcv1;
    m_RCM2 = rcm2;
    m_RCV2 = rcv2;
}



void CCrossPanelIR::ColorBySegments()
{
    m_ColorScoreId.Reset();
}


void CCrossPanelIR::ColorByScore(CConstRef<CObject_id> score_id)
{
    /*m_ColorScoreId = score_id;

    if(m_DS) {
        m_Graph.AssignColorsByScore(score_id);
    }*/
}


void CCrossPanelIR::SetDataSource(ICrossAlnDataSource* ds)
{
    m_DS = ds;

    m_Graph.DeleteGlyphs();

    if(m_DS)    {
        const CHitMatrixDataSource::THitAdapterCont& hits = ds->GetHits();
        ITERATE (CHitMatrixDataSource::THitAdapterCont, it, hits) {
            //const CDenseSegHit* hit = *it;
            //cout << "Q Start " << hit.GetQueryStart() << ", Subj Start " << hit.GetSubjectStart() << endl;
            m_Graph.CreateGlyph(**it);
        }
    }
    m_ColorScoreId.Reset();
}


void CCrossPanelIR::Render(CGlPane& pane)
{
    pane.OpenOrtho();

    IRender& gl = GetGl();

    gl.Disable(GL_CULL_FACE);
    gl.Disable(GL_DEPTH_TEST);
    gl.Disable(GL_LIGHTING);
    gl.Enable(GL_BLEND);
    gl.BlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    gl.Enable(GL_LINE_SMOOTH);
    gl.Hint(GL_LINE_SMOOTH_HINT, GL_NICEST);

    gl.Enable(GL_POLYGON_SMOOTH);
    gl.Hint(GL_POLYGON_SMOOTH_HINT, GL_NICEST);

    gl.LineWidth(1.5f);

    x_DrawCrossAlignment(pane);  // Draw our quads

    x_DrawSequence(pane);  // Draw some sequence letters

    gl.Disable(GL_BLEND);
    gl.Disable(GL_LINE_SMOOTH);
    gl.Disable(GL_POLYGON_SMOOTH);
    gl.LineWidth(1.0f);

    pane.Close();
}


TModelRect CCrossPanelIR::HitTest(CGlPane& pane, int x, int y, bool select)
{
    const TModelRect& rcV = pane.GetVisibleRect();

    CRange<TModelUnit> rcv1(m_RCV1.Left(), m_RCV1.Right());
    CRange<TModelUnit> rcv2(m_RCV2.Left(), m_RCV2.Right());

    CRange<TModelUnit> rcv1_clip(rcv1.GetFrom() - rcv1.GetLength() * kClipMultiple,
                                    rcv1.GetTo() + rcv1.GetLength() * kClipMultiple);
    CRange<TModelUnit> rcv2_clip(rcv2.GetFrom() - rcv2.GetLength() * kClipMultiple,
                                    rcv2.GetTo() + rcv2.GetLength() * kClipMultiple);

    ITERATE(THitGlyphVector, it, m_Graph.GetGlyphs()) { // for each Hit
        const CHitGlyph::TElemGlyphCont& elems = (*it)->GetElems();
        const IHitElement& first_elem = elems.front().GetHitElem();
        const IHitElement& last_elem  = elems.back().GetHitElem();

        TSignedSeqPos from1 = first_elem.GetQueryStart();
        TSignedSeqPos from2 = first_elem.GetSubjectStart();
        TSignedSeqPos to1   = last_elem.GetQueryStart() + last_elem.GetQueryLength();
        TSignedSeqPos to2   = last_elem.GetSubjectStart() + last_elem.GetSubjectLength();

        if ( (to1 < m_RCV1.Left()     &&  to2 < m_RCV2.Left())  ||
             (from1 > m_RCV1.Right()  &&  from1 > m_RCV2.Right()) ) {
            continue;
        }
        if ( (to1 < rcv1_clip.GetFrom()  &&  from2 > rcv2_clip.GetTo())  ||
             (from1 > rcv1_clip.GetTo()  &&  to2 < rcv2_clip.GetFrom()) ) {
            continue;
        }


        /// some part of the range intersects the visible area
        /// now we start back with the first element
        ITERATE (CHitGlyph::TElemGlyphCont, itE, elems) {
            const IHitElement& elem = itE->GetHitElem();

            from1 = elem.GetQueryStart();
            from2 = elem.GetSubjectStart();
            if (from1 == -1  ||  from2 == -1) {
                continue;
            }

            to1   = from1 + elem.GetQueryLength();
            to2   = from2 + elem.GetSubjectLength();

            /// perform our clipping again
            if ( (to1 < m_RCV1.Left()     &&  to2 < m_RCV2.Left())  ||
                 (from1 > m_RCV1.Right()  &&  from1 > m_RCV2.Right()) ) {
                continue;
            }
            if ( (to1 < rcv1_clip.GetFrom()  &&  from2 > rcv2_clip.GetTo())  ||
                 (from1 > rcv1_clip.GetTo()  &&  to2 < rcv2_clip.GetFrom()) ) {
                continue;
            }

            TModelUnit x_from1 = x_Seq2Lim1(from1);
            TModelUnit x_to1   = x_Seq2Lim1(to1);
            TModelUnit x_from2 = x_Seq2Lim2(from2);
            TModelUnit x_to2   = x_Seq2Lim2(to2);

            TModelUnit x_x     = pane.UnProjectX(x);
            TModelUnit x_y     = pane.UnProjectY(y);

            CVect3<TModelUnit> orig(x_x, x_y, 1.0);
            CVect3<TModelUnit> dir(0.0, 0.0, -2.0);
            CVect3<TModelUnit> vert0(x_from1, rcV.Top(), 0.0);
            CVect3<TModelUnit> vert1(x_to1, rcV.Top(), 0.0);
            CVect3<TModelUnit> vert2(x_to2, rcV.Bottom(), 0.0);
            CVect3<TModelUnit> vert3(x_from2, rcV.Bottom(), 0.0);
            CVect3<TModelUnit> pt;

            if (IntersectRayQuad(orig,dir,vert0, vert1, vert2, vert3, pt) == eIntersects) {
                if (select) { // select the Glyph
                    //m_Graph.SelectGlyph(**it);
                    CHitElemGlyph& heg = const_cast<CHitElemGlyph&>(*itE);
                    heg.SetSelected(true);
                }
                return TModelRect(from1, from2, to1, to2);
            }
        }
    }

    return TModelRect(0, 0, 0, 0); // empty
}



void CCrossPanelIR::ResetObjectSelection()
{
    m_Graph.ResetGlyphSelection();
    ITERATE(THitGlyphVector, it, m_Graph.GetGlyphs()) // for each Hit
    {
        const CHitGlyph::TElemGlyphCont& elems = (*it)->GetElems();
        ITERATE(CHitGlyph::TElemGlyphCont, itE, elems) { // for each Hit Element
            const CHitElemGlyph& gl_elem = *itE;
            if (gl_elem.IsSelected()) {
                CHitElemGlyph& the_elem = *const_cast<CHitElemGlyph*>(&gl_elem);
                the_elem.SetSelected(false);
            }
        }
    }
}


void CCrossPanelIR::GetObjectSelection(TConstObjects& objs) const
{
    set<const CSeq_align*> aligns; // selected aligns

    const TElemGlyphSet& glyphs = m_Graph.GetSelectedGlyphs();

    // build a set of CSeq_aligns corresponding to selected glyphs
    ITERATE(TElemGlyphSet, it_G, glyphs)   {
        const IHitElement& elem = (*it_G)->GetHitElem();
        const IHit& hit = elem.GetHit();
        aligns.insert(hit.GetSeqAlign());
    }

    objs.reserve(aligns.size());
    ITERATE(set<const CSeq_align*>, it_align, aligns) {
        objs.push_back(CConstRef<CObject>(*it_align));
    }
}

void CCrossPanelIR::SetObjectSelection(const vector<const CSeq_align*> sel_aligns)
{
    m_Graph.ResetGlyphSelection();

    typedef CHitMatrixGraph::THitGlyphVector TGlyphs;
    const TGlyphs& glyphs = m_Graph.GetGlyphs();
    ITERATE(TGlyphs, it_G, glyphs)  {
        const CHitGlyph& glyph = **it_G;
        const CSeq_align* align = glyph.GetHit().GetSeqAlign();

        if(std::find(sel_aligns.begin(), sel_aligns.end(), align)
                     != sel_aligns.end())   {
            m_Graph.SelectGlyph(glyph);
        }
    }
}


void CCrossPanelIR::x_DrawSequence(CGlPane& pane)
{
    const TVPRect& rcP = pane.GetViewport();

    bool seq_fit1 = x_IsSeqLettersFit1();
    bool seq_fit2 = x_IsSeqLettersFit2();

    TVPUnit font_h = TVPUnit( m_Font_Fixed12.TextHeight() );
    TModelUnit y1 = pane.UnProjectY(rcP.Top() - font_h - 1);
    TModelUnit y2 = pane.UnProjectY(rcP.Bottom() + 3);

    char bases[2];
    bases[1] = '\0';
    string seq1, seq2;

    IRender& gl = GetGl();

    gl.Color3f(0.0f, 0.0f, 0.0f);

    if (seq_fit1  &&  m_DS->GetQueryHandle()) {  // sequence 1 fit
        TSeqPos l1 = TSeqPos( m_RCV1.Left () );
        TSeqPos r1 = TSeqPos( m_RCV1.Right() );

        x_GetSequence(m_DS->GetQueryHandle(), l1, r1, seq1);

        for (TSeqPos bp = 0;  bp != seq1.length();  bp++) {
            bases[0] = seq1[bp];
            TModelUnit x1 = (float) x_Seq2Lim1(l1 + bp + 0.25);
            m_Font_Fixed12.TextOut(x1, y1, bases);
        }
    }

    if (seq_fit2  &&  m_DS->GetSubjectHandle()) {  // sequence 2 fit
        TSeqPos l2 = TSeqPos( m_RCV2.Left () );
        TSeqPos r2 = TSeqPos( m_RCV2.Right() );

        x_GetSequence(m_DS->GetSubjectHandle(), l2, r2, seq2);

        for (TSeqPos bp = 0;  bp != seq2.length();  bp++) {
            bases[0] = seq2[bp];
            TModelUnit x2 = x_Seq2Lim2(l2 + bp + 0.25);
            m_Font_Fixed12.TextOut(x2, y2, bases);
        }
    }

    // 4) Highlight mismatches (and draw connection lines)
    if (seq_fit1  &&  seq_fit2  &&
        m_DS->GetQueryHandle()  &&  m_DS->GetSubjectHandle()) { // not yet
        TVPUnit value = TVPUnit(m_Font_Fixed12.TextHeight());
        TModelUnit font_height = pane.UnProjectHeight(value);

        CRange<TModelUnit> rcv1(m_RCV1.Left(), m_RCV1.Right());
        CRange<TModelUnit> rcv2(m_RCV2.Left(), m_RCV2.Right());

        CRange<TModelUnit> rcv1_clip(rcv1.GetFrom() - rcv1.GetLength() * kClipMultiple,
                                     rcv1.GetTo() + rcv1.GetLength() * kClipMultiple);
        CRange<TModelUnit> rcv2_clip(rcv2.GetFrom() - rcv2.GetLength() * kClipMultiple,
                                     rcv2.GetTo() + rcv2.GetLength() * kClipMultiple);

        ITERATE(THitGlyphVector, it, m_Graph.GetGlyphs()) { // for each Hit
            const CHitGlyph::TElemGlyphCont& elems = (*it)->GetElems();
            const IHitElement& first_elem = elems.front().GetHitElem();
            const IHitElement& last_elem  = elems.back().GetHitElem();

            TSignedSeqPos from1 = first_elem.GetQueryStart();
            TSignedSeqPos from2 = first_elem.GetSubjectStart();
            TSignedSeqPos to1   = last_elem.GetQueryStart() + last_elem.GetQueryLength();
            TSignedSeqPos to2   = last_elem.GetSubjectStart() + last_elem.GetSubjectLength();

            /// first test: clip for things entirely off screen
            if ( (to1 < m_RCV1.Left()     &&  to2 < m_RCV2.Left())  ||
                 (from1 > m_RCV1.Right()  &&  from1 > m_RCV2.Right()) ) {
                continue;
            }

            /// second test: clip items that cross too obliquely
            if ( (to1 < rcv1_clip.GetFrom()  &&  from2 > rcv2_clip.GetTo())  ||
                 (from1 > rcv1_clip.GetTo()  &&  to2 < rcv2_clip.GetFrom()) ) {
                continue;
            }


            /// some part of the range intersects the visible area
            /// now we start back with the first element
            ITERATE (CHitGlyph::TElemGlyphCont, itE, elems) {
                const IHitElement& elem = itE->GetHitElem();

                from1 = elem.GetQueryStart();
                from2 = elem.GetSubjectStart();

                if (from1 == -1  ||  from2 == -1) {
                    continue;
                }

                to1   = from1 + elem.GetQueryLength();
                to2   = from2 + elem.GetSubjectLength();

                /// perform our clipping again
                if ( (to1 < m_RCV1.Left()     &&  to2 < m_RCV2.Left())  ||
                     (from1 > m_RCV1.Right()  &&  from1 > m_RCV2.Right()) ) {
                    continue;
                }
                if ( (to1 < rcv1_clip.GetFrom()  &&  from2 > rcv2_clip.GetTo())  ||
                     (from1 > rcv1_clip.GetTo()  &&  to2 < rcv2_clip.GetFrom()) ) {
                    continue;
                }

                seq1.erase();
                seq2.erase();
                x_GetSequence(m_DS->GetQueryHandle(),   from1, to1, seq1);
                x_GetSequence(m_DS->GetSubjectHandle(), from2, to2, seq2);

                TSeqPos bp = 0;
                //TODO - Length on query and Subject can be different
                for (TSeqPos pos = 0;  pos != elem.GetQueryLength();  pos++) {
                    //TModelUnit x1 = x_Seq2Lim1(from1 + pos + 0.25);
                    //TModelUnit x2 = x_Seq2Lim2(from2 + pos + 0.25);

                    // Draw lines to connect mismatches
                    gl.Color3f(0.0f, 0.0f, 0.0f);  // black
                    if (seq1[bp] != seq2[bp]  ||  (from1 + pos) % 10 == 0) {
                        TModelUnit lx1 = from1 + pos + 0.5; // centered
                        TModelUnit lx2 = from2 + pos + 0.5;
                        float alpha = 0.5f;
                        if (lx1 < m_RCV1.Left()  ||  lx1 > m_RCV1.Right() ||
                            lx2 < m_RCV2.Left()  ||  lx2 > m_RCV2.Right() )
                        {
                            alpha = 0.08f;
                        }
                        if (seq1[bp] != seq2[bp]) {
                            gl.Color4f(1.0f, 0.0f, 0.0f, alpha);
                        } else {
                            gl.Color4f(0.7f, 0.7f, 0.7f, alpha);
                        }
                        gl.Begin(GL_LINES);
                            gl.Vertex2d(x_Seq2Lim1(lx1), y1);
                            gl.Vertex2d(x_Seq2Lim2(lx2), y2 + font_height);
                        gl.End();
                    }
                    bp++;
                } // for: pos
            }
        }
    }
}


void CCrossPanelIR::x_GetSequence(const CBioseq_Handle& handle,
            TSeqPos from, TSeqPos to, string& buffer) const
{
    buffer.erase();
    CSeqVector s_vec =
        handle.GetSeqVector(CBioseq_Handle::eCoding_Iupac);

    s_vec.GetSeqData(from, to, buffer);
}


void CCrossPanelIR::x_DrawCrossedPanel(CGlPane& pane, const CRgbaColor& color,
                                       TModelUnit x_from1, TModelUnit x_to1,
                                       TModelUnit x_from2, TModelUnit x_to2)
{
    const TModelRect& rcV = pane.GetVisibleRect();
    TModelUnit x_offs = pane.GetOffsetX();
    float alpha = 0.1f;

    IRender& gl = GetGl();

    gl.Color4f(color.GetRed(), color.GetGreen(), color.GetBlue(), alpha);

    /// gap is not too small
    /// render the previous element, save the current
    if (fabs(x_from1 - x_to1) < pane.GetScaleX()  &&
        fabs(x_from2 - x_to2) < pane.GetScaleX()) {

        gl.Begin(GL_LINES);
            gl.Vertex2d(x_from1 - x_offs, rcV.Top());
            gl.Vertex2d(x_from2 - x_offs, rcV.Bottom());
        gl.End();
    } else {
        gl.Begin(GL_TRIANGLE_STRIP);
            gl.Vertex2d(x_from1 - x_offs, rcV.Top());
            gl.Vertex2d(x_to1 - x_offs,   rcV.Top());
            gl.Vertex2d(x_from2 - x_offs, rcV.Bottom());
            gl.Vertex2d(x_to2 - x_offs,   rcV.Bottom());
        gl.End();
    }
}


void CCrossPanelIR::x_DrawCrossAlignment(CGlPane& pane)
{
    CStopWatch sw;
    sw.Start();
    //const TModelRect& rcV = pane.GetVisibleRect();

    //TModelUnit rcv1_inv = 1.0f / m_RCV1.Width();
    //TModelUnit rcv2_inv = 1.0f / m_RCV2.Width();

    size_t clip_bounds = 0;
    size_t clip_cross  = 0;
    size_t elements_rendered = 0;

    ///
    /// this code makes use of CRange<> as a temporary storage for standardization
    ///

    /// we compute two ranges: the visible range and a related clip range

    /// first sequence ranges
    CRange<TModelUnit> rcv1(m_RCV1.Left(), m_RCV1.Right());
    CRange<TModelUnit> rcv1_clip
        (rcv1.GetFrom() - rcv1.GetLength() * kClipMultiple,
         rcv1.GetTo()   + rcv1.GetLength() * kClipMultiple);

    /// second sequence ranges
    CRange<TModelUnit> rcv2(m_RCV2.Left(), m_RCV2.Right());
    CRange<TModelUnit> rcv2_clip
        (rcv2.GetFrom() - rcv2.GetLength() * kClipMultiple,
         rcv2.GetTo()   + rcv2.GetLength() * kClipMultiple);

    CRgbaColor sel_color(0.0f, 0.0f, 1.0f);
    CRgbaColor plus_color(1.0f, 0.0f, 0.0f);
    CRgbaColor minus_color(1.0f, 0.8f, 0.0f);

    ITERATE(THitGlyphVector, it, m_Graph.GetGlyphs()) { // for each Hit
        const CHitGlyph::TElemGlyphCont& elems = (*it)->GetElems();

        const IHitElement& first_elem = elems.front().GetHitElem();
        const IHitElement& last_elem  = elems.back().GetHitElem();
        bool is_mixed =
            (first_elem.GetQueryStrand() != first_elem.GetSubjectStrand());

        /// we consider the entire range at first
        CRange<TModelUnit> range1(first_elem.GetQueryStart(),
                                  last_elem.GetQueryStart() + last_elem.GetQueryLength());
        CRange<TModelUnit> range2(first_elem.GetSubjectStart(),
                                  last_elem.GetSubjectStart() + last_elem.GetSubjectLength());

        /// clipping:

        /// first test:
        ///  - is the panel entirely off the screen (left or right)?
        if ( (range1.GetTo() < rcv1.GetFrom()  &&  range2.GetTo() < rcv2.GetFrom())  ||
             (range1.GetFrom() > rcv1.GetTo()  &&  range2.GetFrom() > rcv2.GetTo()) ) {
            /// off screen, not crossing
            ++clip_bounds;
            continue;
        }

        /// second test:
        ///  - does the panel cross left -> right or right -> left at
        ///    so steep an angle as to be worthless information?
        if ( !range1.IntersectingWith(rcv1_clip)  &&
             !range2.IntersectingWith(rcv2_clip)) {
            ++clip_cross;
            continue;
        }

        TModelUnit x_from1 = x_Seq2Lim1(range1.GetFrom());
        TModelUnit x_to1   = x_Seq2Lim1(range1.GetTo());
        TModelUnit x_from2 = x_Seq2Lim2(range2.GetFrom());
        TModelUnit x_to2   = x_Seq2Lim2(range2.GetTo());

        /// we will only iterate the elements in this hit if the entire hit
        /// occupies more than one pixel
        if (fabs(x_from1 - x_to1) > pane.GetScaleX()  ||
            fabs(x_from2 - x_to2) > pane.GetScaleX()) {

            /// some part of the range intersects the visible area
            /// now we start back with the first element
            CHitGlyph::TElemGlyphCont::const_iterator itE     = elems.begin();
            CHitGlyph::TElemGlyphCont::const_iterator itE_end = elems.end();

            range1.SetTo(itE->GetHitElem().GetQueryStart() +
                         itE->GetHitElem().GetQueryLength());
            range2.SetTo(itE->GetHitElem().GetSubjectStart() +
                         itE->GetHitElem().GetSubjectLength());

            x_to1 = x_Seq2Lim1(range1.GetTo());
            x_to2 = x_Seq2Lim2(range2.GetTo());

            for ( ;  itE != itE_end;  ++itE) {
                const IHitElement& elem = itE->GetHitElem();
                if (elem.GetQueryStart()   == -1  ||
                    elem.GetSubjectStart() == -1) {
                    continue;
                }

                ///
                /// compute sequence and model coordinates for this element's
                /// starting and ending coordinates
                ///

                //TODO - don't we need to subtract 1 from start + len?
                CRange<TModelUnit> elem_range1(elem.GetQueryStart(),
                                               elem.GetQueryStart() + elem.GetQueryLength());
                CRange<TModelUnit> elem_range2(elem.GetSubjectStart(),
                                                elem.GetSubjectStart() + elem.GetSubjectLength());

                /// check to see if the starting point of this element is
                /// mergeable with the ending point of the previous element
                TModelUnit x_elem1_start = x_Seq2Lim1(elem_range1.GetFrom());
                TModelUnit x_elem1_stop  = x_Seq2Lim1(elem_range1.GetTo());
                TModelUnit x_elem2_start = x_Seq2Lim2(elem_range2.GetFrom());
                TModelUnit x_elem2_stop  = x_Seq2Lim2(elem_range2.GetTo());

                /// merge this element if the gap space is too small
                if (fabs(x_elem1_start - x_to1) < pane.GetScaleX()  &&
                    fabs(x_elem2_start - x_to2) < pane.GetScaleX()) {
                    range1 += elem_range1;
                    range2 += elem_range2;

                    x_to1 = x_elem1_stop;
                    x_to2 = x_elem2_stop;
                    continue;
                }

                ///
                /// unmerged element
                /// render it as a panel
                ///
                if (range1.IntersectingWith(rcv1_clip)  ||
                    range2.IntersectingWith(rcv2_clip)) {
                    CRgbaColor& clr = itE->IsSelected() ? sel_color :
                            (is_mixed ? minus_color : plus_color);
                    x_DrawCrossedPanel(pane, clr, x_elem1_start, x_elem1_stop,
                                                  x_elem2_start, x_elem2_stop);
                    ++elements_rendered;
                }

                range1 = elem_range1;
                range2 = elem_range2;

                x_from1 = x_elem1_start;
                x_from2 = x_elem2_start;
                x_to1   = x_elem1_stop;
                x_to2   = x_elem2_stop;
            }
        }

        if (is_mixed) {
            x_DrawCrossedPanel(pane, minus_color,
                               x_from1, x_to1, x_from2, x_to2);
        } else {
            x_DrawCrossedPanel(pane, plus_color,
                               x_from1, x_to1, x_from2, x_to2);
        }
        ++elements_rendered;
    }

    /**
    LOG_POST(Error << "CCrossPanelIR::x_DrawCrossAlignment(): finish: "
        << sw.Elapsed() << ": "
        << clip_bounds << " bound / "
        << clip_cross << " cross / "
        << elements_rendered << " rendered");
        **/
}



double CCrossPanelIR::x_SelectBaseStep(double MinV, double MaxV) const
{
    double Range = MaxV - MinV;
    double logRange = log10(Range);
    logRange = ceil(logRange) - 1;
    double Step = pow(10.0, logRange);

    // selcting BaseStep as step divided by 1, 2 or 5
    double BaseStep = Step;
    double nTicks = Range / BaseStep;
    int i=0;
    while( nTicks < 8 )
    {
        BaseStep = Step / sc_StepK[i++];
        nTicks = Range / BaseStep;
    }
    return BaseStep;
}



// Convert sequence to limits
TModelUnit CCrossPanelIR::x_Seq2Lim1(TModelUnit seq)
{
    return (seq - m_RCV1.Left()) / m_RCV1.Width();
}


TModelUnit CCrossPanelIR::x_Seq2Lim2(TModelUnit seq)
{
    return (seq - m_RCV2.Left()) / m_RCV2.Width();
}


// Convert limits to sequence
TModelUnit CCrossPanelIR::x_Lim2Seq1(TModelUnit lim)
{
    return lim;
}


TModelUnit CCrossPanelIR::x_Lim2Seq2(TModelUnit lim)
{
    return lim;
}


bool CCrossPanelIR::x_IsSeqLettersFit1() const
{
    return m_RCV1.Width() / m_RCM1.Width() <= 1.0f / 6.0f;
}

bool CCrossPanelIR::x_IsSeqLettersFit2() const
{
    return m_RCV2.Width() / m_RCM2.Width() <= 1.0f / 6.0f;
}


END_NCBI_SCOPE
