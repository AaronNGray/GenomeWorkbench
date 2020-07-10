/*  $Id: histogram_graph.cpp 41823 2018-10-17 17:34:58Z katargir $
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

#include <gui/widgets/gl/histogram_graph.hpp>
#include <gui/opengl/glhelpers.hpp>
#include <gui/opengl/irender.hpp>


BEGIN_NCBI_SCOPE


CHistogramGraph::SProperties::SProperties()
:   m_MinColor(1.0f, 1.0f, 1.0f),
    m_MaxColor(0.0f, 0.0f, 0.0f),
    m_Margin(1)
{
}


CHistogramGraph::CHistogramGraph(bool b_horz)
:   m_Horz(b_horz)
{
    m_ModelRect.Init(0.0, 0.0, 1.0, 1.0);

    m_ColorTable.SetSize(64);
    m_ColorTable.FillGradient(m_Properties.m_MinColor, m_Properties.m_MaxColor);
}


CHistogramGraph::~CHistogramGraph()
{
}

void    CHistogramGraph::SetDataSource(IHistogramGraphDS* ds)
{
    m_DataSource = ds;
}


IHistogramGraphDS* CHistogramGraph::GetDataSource()
{
    return m_DataSource.GetPointer();
}


const IHistogramGraphDS* CHistogramGraph::GetDataSource() const
{
    return m_DataSource.GetPointer();
}


void CHistogramGraph::Render(CGlPane& pane)
{
    //LOG_POST("  CHistogramGraph::Render()");
    //LOG_POST("    Viewport " << m_VPRect.ToString() << "\n    Visible " << m_ModelRect.ToString());
    IRender& gl = GetGl();

    pane.OpenPixels();
    //glColor3d(0.2, 0.2, 0.2);
    const CRgbaColor& color = m_ColorTable.GetColor(0);
    gl.ColorC(color);

    gl.PolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    gl.RectC(m_VPRect);
    pane.Close();

    const static double kMaxRatio = 4.0;
    const static double kRatio = 2.0;

    // trivial implementation
    if(m_DataSource)    {
        const TModelRect& rc_vis = pane.GetVisibleRect();
        TModelUnit start = m_DataSource->GetStart();
        TModelUnit stop = m_DataSource->GetStop();

        // check if we need to update the datasource
        TModelUnit vis_start = m_Horz ? rc_vis.Left() : rc_vis.Bottom();
        TModelUnit vis_stop = m_Horz ? rc_vis.Right() : rc_vis.Top();
        bool outside =  vis_start < start  || vis_stop > (stop + 1);

        TModelUnit ratio = (vis_stop - vis_start) / (stop - start + 1);
        bool bad_scale = ratio > kMaxRatio  ||  ratio < (1 / kMaxRatio);

        if(outside  ||  bad_scale)  {
            TModelUnit new_start, new_stop;
            TModelUnit center = (vis_start + vis_stop) / 2;
            TModelUnit range = (vis_stop - vis_start) * kRatio;

            double max_pos = m_DataSource->GetLimit();
            range = min(range, max_pos);

            new_start = center - range / 2;
            new_stop = new_start + range;
            if(new_start < 0)   {
                new_start = 0;
                new_stop = new_start + range;
            } else if(new_stop > max_pos)   {
                new_stop = max_pos;
                new_start = new_stop - range;
            }

            m_DataSource->Update(new_start, new_stop);
        }

        x_RenderHistogram(pane);
    }
}


void    CHistogramGraph::x_RenderHistogram(CGlPane& pane)
{
    pane.OpenOrtho();

    CGlAttrGuard guard(GL_LINE_BIT |  GL_ENABLE_BIT  | GL_COLOR_BUFFER_BIT );
    IRender& gl = GetGl();

    gl.Enable(GL_BLEND);
    gl.BlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    gl.PolygonMode(GL_FRONT_AND_BACK, GL_FILL);

    TModelUnit start = m_DataSource->GetStart();
    TModelUnit z1, z2;

    TModelUnit offset_x = pane.GetOffsetX();
    TModelUnit offset_y = pane.GetOffsetY();

    if(m_Horz) {
        z1 = m_ModelRect.Bottom() - offset_y;
        z2 = m_ModelRect.Top() - offset_y;
    } else {
        z1 = m_ModelRect.Left() - offset_x;
        z2 = m_ModelRect.Right() - offset_x;
    }
    z1 += m_Properties.m_Margin;
    z2 -= m_Properties.m_Margin;

    TModelUnit max_v = m_DataSource->GetMaxValue();
    TModelUnit d = m_DataSource->GetStep();

    int count = m_DataSource->GetCount();
    for( int i = 0; i < count; i++ )    {
        double value = m_DataSource->GetValue(i);
        double norm = (max_v > 0.00001) ? (value / max_v) : 0;
        int index = m_ColorTable.GetIndex(norm);
        const CRgbaColor& color = m_ColorTable.GetColor(index);
        gl.ColorC(color);

        double pos = start + i * d;
        if(m_Horz) {
            TModelUnit x1 = pos - offset_x;
            gl.Rectd(x1, z1, x1 + d, z2);
        } else {
            TModelUnit y1 = pos - offset_y;
            gl.Rectd(z1, y1, z2, y1 + d);
        }
    }
    gl.Disable(GL_LINE_SMOOTH);
    gl.Disable(GL_BLEND);

    pane.Close();
}


TVPPoint CHistogramGraph::PreferredSize()
{
    static const int kGraphSize = 12;
    return TVPPoint(kGraphSize, kGraphSize);
}


bool CHistogramGraph::NeedTooltip(CGlPane& /*pane*/, int vp_x, int vp_y)
{
    return m_VPRect.PtInRect(vp_x, vp_y);
}


string CHistogramGraph::GetTooltip()
{
    string s = "Features : " + m_DataSource->GetLabel();
    return s;
}


const CHistogramGraph::SProperties&  CHistogramGraph::GetProperties() const
{
    return m_Properties;
}


void CHistogramGraph::SetProperties(const SProperties& props)
{
    m_Properties = props;
    m_ColorTable.FillGradient(m_Properties.m_MinColor, m_Properties.m_MaxColor);
}


END_NCBI_SCOPE
