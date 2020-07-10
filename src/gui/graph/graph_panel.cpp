/*  $Id: graph_panel.cpp 15168 2007-10-15 19:11:34Z dicuccio $
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
#include <gui/graph/graph_panel.hpp>
#include <gui/opengl/glutils.hpp>

BEGIN_NCBI_SCOPE

CGraphPanel::CGraphPanel()
:   m_pAxisArea(NULL),
    m_pLegend(NULL),
    m_rcAll(0,0,200,200),
    m_GraphOffsetX(80), m_GraphOffsetY(10),
    m_LegendOffsetX(80), m_LegendH(30),
    m_LimitsModeX(eGraphLimits),
    m_LimitsModeY(eGraphLimits),
    m_bDrawGrid(true),
    m_bDrawAxes(true),
    m_bAntiAliasing(true),
    m_BackColor(1.0f, 1.0f, 1.0f),
    m_GraphBackColor(1.0f, 1.0f, 1.0f)
{
}


CGraphPanel::~CGraphPanel()
{
    destroy_null(m_pLegend);
    destroy_null(m_pAxisArea);
}


void    CGraphPanel::Create()
{
    m_pLegend = CreateLegend();
    m_pAxisArea = CreateAxisArea();
    if (m_pAxisArea)
        m_pAxisArea->SetupAxes();
}


void    CGraphPanel::SetRect(const TVPRect& rcAll, bool bLayout)
{
    m_rcAll = rcAll;
    if (bLayout)
        Layout();
}

void    CGraphPanel::SetLayout( int GraphOffsetX, int GraphOffsetY,
                                int LegendOffsetX, int LegendH, bool bLayout)
{
    m_GraphOffsetX = GraphOffsetX;
    m_GraphOffsetY = GraphOffsetY;
    m_LegendOffsetX = LegendOffsetX;
    m_LegendH = LegendH;

    if (bLayout)
        Layout();
}


void    CGraphPanel::Layout()
{
    TVPRect rc = m_rcAll;
    int y = rc.Bottom() + m_LegendH;
    rc.SetBottom(y);
    m_Pane.SetViewport(rc);

    // place Graph pane in the top right corner
    rc.MoveLeft(m_GraphOffsetX);
    rc.MoveBottom(m_GraphOffsetY);
    m_GraphPane.SetViewport(rc);

    // place Legend in the bottom
    rc = m_rcAll;
    rc.SetLeft(m_LegendOffsetX);
    rc.SetTop(y-1);
    m_LegendPane.SetViewport(rc);
}


void    CGraphPanel::SetLimitsMode(ELimitsMode ModeX, ELimitsMode ModeY)
{
    m_LimitsModeX = ModeX;
    m_LimitsModeY = ModeY;
}


void    CGraphPanel::SetFixedLimits(const TModelRect& rcLimits)
{
    TModelRect rc = m_GraphPane.GetModelLimitsRect();

    if (m_LimitsModeX == eFixed) {
        rc.SetLeft(rcLimits.Left());
        rc.SetRight(rcLimits.Right());
    }
    if (m_LimitsModeY == eFixed) {
        rc.SetBottom(rcLimits.Bottom());
        rc.SetTop(rcLimits.Top());
    }
    m_GraphPane.SetModelLimitsRect(rc);
}


void    CGraphPanel::UpdateLimits()
{
    TModelRect rcLimits = m_GraphPane.GetModelLimitsRect();

    if(m_LimitsModeX == eGraphLimits  ||  m_LimitsModeX == eRounded) {
        TModelRect rcX;
        if (!m_Graphs.empty())
            rcX = m_Graphs.front()->GetLimits();
        NON_CONST_ITERATE(TGraphCont, it, m_Graphs) {
            rcX.CombineWith((*it)->GetLimits());
        }
        if(m_LimitsModeX == eRounded) {
            //call extension function
        }
        if(rcX.Width() == 0)
            rcX.SetRight(rcX.Left() + 1.0);

        rcLimits.SetLeft(rcX.Left());
        rcLimits.SetRight(rcX.Right());
    }

    if(m_LimitsModeY == eGraphLimits  ||  m_LimitsModeY == eRounded) {
        TModelRect rcY;
        if (!m_Graphs.empty())
            rcY = m_Graphs.front()->GetLimits();
        NON_CONST_ITERATE(TGraphCont, it, m_Graphs) {
            rcY.CombineWith((*it)->GetLimits());
        }
        if(m_LimitsModeY == eRounded) {
            //call extension function
        }
        if (rcY.Height() == 0)
            rcY.SetTop(rcY.Bottom() + 1.0);

        rcLimits.SetBottom(rcY.Bottom());
        rcLimits.SetTop(rcY.Top());
    }
    m_GraphPane.SetModelLimitsRect(rcLimits);
}


void    CGraphPanel::SetIntegerMode(bool bIntegerX, bool bIntegerY)
{
    m_Gen.SetIntegerMode(bIntegerX, bIntegerY);
}


void    CGraphPanel::SetDrawGrid(bool bDraw)
{
    m_bDrawGrid = bDraw;
}


void    CGraphPanel::SetDrawAxes(bool bDraw)
{
    m_bDrawAxes = bDraw;
}


void    CGraphPanel::EnableAntiAliasing(bool bEn)
{
    m_bAntiAliasing = bEn;
}


void    CGraphPanel::SetBackColor(const CRgbaColor& Color, const CRgbaColor& GraphColor)
{
    m_BackColor = Color;
    m_GraphBackColor = GraphColor;
}


void    CGraphPanel::AddGraph(IGraph* pGraph)
{
    m_Graphs.push_back(pGraph);
    UpdateLimits();
}


void    CGraphPanel::RemoveAllGraphs()
{
    m_Graphs.clear();
    UpdateLimits();
}


void    CGraphPanel::Render(IGraph::TElemVector* elems)
{
    PrepareContext();

    if (m_bDrawGrid)
        m_Grid.Render(&m_Pane, &m_GraphPane, &m_Gen);

    NON_CONST_ITERATE(TGraphCont, itGr, m_Graphs) {
        (*itGr)->Render(&m_GraphPane, elems);
    }

    if (m_pAxisArea && m_bDrawAxes)
        m_pAxisArea->Render(&m_Pane, &m_GraphPane, &m_Gen);

    if (m_pLegend)
        m_pLegend->Render(&m_LegendPane);
}


void    CGraphPanel::PrepareContext()
{
    if (m_bAntiAliasing) {
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

        glEnable(GL_LINE_SMOOTH);
        glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);

        glEnable(GL_POINT_SMOOTH);
        glHint(GL_POINT_SMOOTH_HINT, GL_NICEST);
    } else {
        glDisable(GL_BLEND);
        glDisable(GL_LINE_SMOOTH);
        glDisable(GL_POINT_SMOOTH);
    }

    glClearColor(m_BackColor.GetRed(), m_BackColor.GetGreen(), m_BackColor.GetBlue(), m_BackColor.GetAlpha());
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    m_GraphPane.OpenPixels();

    TVPRect rc = m_GraphPane.GetViewport();
    glColorC(m_GraphBackColor);
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    glRectd(rc.Left(), rc.Bottom(), rc.Right(), rc.Top());

    m_GraphPane.Close();
}


CAxisArea*  CGraphPanel::CreateAxisArea()
{
    return new CAxisArea();
}


CLegend*    CGraphPanel::CreateLegend()
{
    return new CLegend();
}


END_NCBI_SCOPE
