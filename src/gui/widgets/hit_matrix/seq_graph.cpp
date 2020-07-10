/*  $Id: seq_graph.cpp 44757 2020-03-05 18:58:50Z evgeniev $
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

#include <ncbi_pch.hpp>
#include <corelib/ncbistl.hpp>
#include <gui/widgets/hit_matrix/seq_graph.hpp>

#include <gui/objutils/label.hpp>
#include <gui/opengl/glutils.hpp>

#include <gui/opengl/irender.hpp>

#include <objmgr/seq_vector.hpp>
#include <objmgr/util/sequence.hpp>


#include <math.h>


BEGIN_NCBI_SCOPE
USING_SCOPE(objects);

CHitMatrixSeqGraph::CHitMatrixSeqGraph(bool b_horz)
:   m_bHorz(b_horz),
    m_pSeqFont(NULL),
    m_BackColor(0.9f, 0.9f, 0.9f),
    m_TextColor(0.0f, 0.0f, 0.0f)
{
    m_Bioseq.Reset();
}


void    CHitMatrixSeqGraph::SetFonts(CGlTextureFont* seq_font)
{
    m_pSeqFont = seq_font;
}


void    CHitMatrixSeqGraph::SetBioseqHandle(CBioseq_Handle& handle)
{
    m_Bioseq = handle;
    TSeqPos len = m_Bioseq ? m_Bioseq.GetBioseqLength() : 0;
    m_ModelRect.Init(0.0, 0.0, (TModelUnit) len, 1.0);
}


static const int kMargin = 2;
static const int kTextOffset = 2;

TVPPoint CHitMatrixSeqGraph::PreferredSize()
{
    int spacing = 2 * kMargin;
    TVPPoint size(spacing, spacing);
    IRender& gl = GetGl();

    if(m_bHorz) {
        //TModelUnit seq_h = m_pSeqFont->GetMetric(CGlTextureFont::eMetric_FullCharHeight);
        TModelUnit seq_h = gl.GetMetric (m_pSeqFont.GetPointer(), CGlTextureFont::eMetric_FullCharHeight);
        size.m_Y += 2 * kTextOffset + (int) ceil(seq_h);
    } else {
        //TModelUnit seq_sym_w = m_pSeqFont->GetMetric(CGlTextureFont::eMetric_MaxCharWidth);
        TModelUnit seq_sym_w = gl.GetMetric (m_pSeqFont.GetPointer(), CGlTextureFont::eMetric_MaxCharWidth);
        size.m_X += 2 * kTextOffset + (int) ceil(seq_sym_w);
    }

    return size;
}


void    CHitMatrixSeqGraph::Render(CGlPane& pane)
{
    if(m_Bioseq)    {
        IRender& gl = GetGl();

        _ASSERT(m_pSeqFont);

        gl.PolygonMode(GL_FRONT_AND_BACK, GL_FILL);

        //const   TVPRect&    rc_VP = pane.GetViewport();
        const TModelRect& rc_model = pane.GetVisibleRect();

        /////TModelUnit seq_h = gl.TextHeight(m_pSeqFont.GetPointer());

        //TModelUnit seq_h = m_pSeqFont->GetMetric(CGlTextureFont::eMetric_FullCharHeight);
        //TModelUnit seq_sym_w = m_pSeqFont->GetMetric(CGlTextureFont::eMetric_MaxCharWidth);

        TModelUnit seq_h = gl.GetMetric (m_pSeqFont.GetPointer(), CGlTextureFont::eMetric_FullCharHeight);
        TModelUnit seq_sym_w = gl.GetMetric (m_pSeqFont.GetPointer(), CGlTextureFont::eMetric_MaxCharWidth);

        // fill background
        TVPRect rc_back = m_VPRect;
        if(m_bHorz) {
            rc_back.Inflate(0, -kMargin);
        } else {
            rc_back.Inflate(-kMargin, 0);
        }

        pane.OpenPixels();
        gl.ColorC(m_BackColor);
        gl.Rectd(rc_back.Left(), rc_back.Bottom(), rc_back.Right(), rc_back.Top());
        pane.Close(); // close pixels

        // draw sequence
        pane.OpenOrtho();
        gl.ColorC(m_TextColor);

        if(m_bHorz) {
            double min_w = 1.0 / seq_sym_w;
            TModelUnit scale_x = pane.GetScaleX();
            if(min_w > scale_x)    {    // sequence is visible
                TModelUnit offset_x = pane.GetOffsetX();

                TSeqPos start = (TSeqPos) floor(rc_model.Left());
                TSeqPos stop = (TSeqPos) ceil(rc_model.Right());

                // get the sequence
                string seq;
                CSeqVector v_seq = m_Bioseq.GetSeqVector();
                v_seq.SetIupacCoding();
                v_seq.GetSeqData(start, stop, seq);

                TModelUnit x = start + 0.5 - offset_x;
                TModelUnit y = m_VPRect.Bottom() + kMargin + kTextOffset;// + seq_h / 2;
                
                //double w = gl.TextWidth(m_pSeqFont.GetPointer(), seq.c_str());
                char bases[2];
                bases[1] = '\0';
                TModelUnit k_x = scale_x / 2;
                for (TSeqPos bp = 0;  bp != seq.length();  bp++) {
                    bases[0] = seq[bp];
                    TModelUnit xM = x + bp ;
                    TModelUnit cw = m_pSeqFont->TextWidth(bases);
                    TModelUnit off_x = cw * k_x;
                    gl.BeginText(&*m_pSeqFont, m_TextColor);
                    gl.WriteText(xM - off_x, y, bases);
                    gl.EndText();
                }


                
//                m_pSeqFont->ArrayTextOut(x, y, 1.0, 0.0, seq.c_str(), NULL, scale_x, scale_y);


            }
        } else { // vertical orientation
            double min_h = 1.0 / seq_h;
            TModelUnit scale_y = pane.GetScaleY();
            //scale_y = pane.GetScaleY();
            if(min_h > scale_y) {    // sequence is visible
                TModelUnit offset_y = pane.GetOffsetY();

                TSeqPos start = (TSeqPos) floor(rc_model.Bottom());
                TSeqPos stop = (TSeqPos) ceil(rc_model.Top());

                // get the sequence
                string seq;
                CSeqVector v_seq = m_Bioseq.GetSeqVector();
                v_seq.SetIupacCoding();
                v_seq.GetSeqData(start, stop, seq);

                TModelUnit x = m_VPRect.Left() +  kMargin + kTextOffset;
                TModelUnit y = start - offset_y + scale_y * kTextOffset;

                char bases[2];
                bases[1] = '\0';
                for (TSeqPos bp = 0;  bp != seq.length();  bp++) {
                    bases[0] = seq[bp];
                    TModelUnit yM = y + bp ;
                    gl.BeginText(&*m_pSeqFont, m_TextColor);
                    gl.WriteText(x, yM, bases);
                    gl.EndText();
                }


                //m_pSeqFont->ArrayTextOut(x, y, 0.0, 1.0, seq.c_str(), NULL, 0.0, scale_y);
            }
        }
        pane.Close();
    }
}


bool    CHitMatrixSeqGraph::NeedTooltip(CGlPane& /*pane*/, int vp_x, int vp_y)
{
    return m_VPRect.PtInRect(vp_x, vp_y);
}


string  CHitMatrixSeqGraph::GetTooltip()
{
    string s = "Sequence : " + sequence::CDeflineGenerator().GenerateDefline(m_Bioseq);
    return s;
}



END_NCBI_SCOPE
