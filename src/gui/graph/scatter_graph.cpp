/*  $Id: scatter_graph.cpp 23958 2011-06-24 15:24:59Z wuliangs $
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
#include <gui/graph/scatter_graph.hpp>
#include <gui/opengl/glutils.hpp>

BEGIN_NCBI_SCOPE

///////////////////////////////////////////////////////////////////////////////
/// class CScatterGraph

CScatterGraph::CScatterGraph()
: m_MarkerType(CGraphDotMarker::eRect),
  m_bDrawLines(true)
{
}

CGraphDotMarker::EMarkerType  CScatterGraph::GetMarkerType()  const
{
    return m_MarkerType;
}

void    CScatterGraph::SetMarkerType(CGraphDotMarker::EMarkerType Type)
{
    m_MarkerType = Type;
}

bool    CScatterGraph::IsDrawLines() const
{
    return m_bDrawLines;
}

void    CScatterGraph::SetDrawLines(bool bDraw)
{
    m_bDrawLines = bDraw;
}

bool    CScatterGraph::SetDataSource(IGraphDataSource* pDS)
{
    IScatterDataSource* pScatterDS = dynamic_cast<IScatterDataSource*>(pDS);
    bool bOk =  pScatterDS!= NULL;
    CGraphBase::SetDataSource(bOk ? pDS : NULL);

    CalculateLimits();
    return bOk;
}

const   TModelRect&    CScatterGraph::GetLimits()    const
{
    return m_Limits;
}

void    CScatterGraph::Render(CGlPane* pPane, TElemVector*)
{
    _ASSERT(pPane);
    IScatterDataSource* pSource = GetScatterDataSource();
    if(pPane && pSource) {
        pPane->OpenOrtho();
        try {
            glColorC(m_Color);

            double MarkerW = pPane->UnProjectWidth(7);
            double MarkerH = pPane->UnProjectHeight(7);
         
            INumericArray* pArX = pSource->GetXArray();
            INumericArray* pArY = pSource->GetYArray();
            size_t ElemN = pArX->GetSize();

            // draw lines
            if (m_bDrawLines) {
                glBegin(GL_LINE_STRIP);
                for (size_t i = 0;  i < ElemN;  i++ ) {
                    glVertex2d(pArX->GetElem(i), pArY->GetElem(i));
                }
                glEnd();
            }

            //draw Markers
            if (m_MarkerType != CGraphDotMarker::eNone) {
                TModelRect rcHit(pPane->GetVisibleRect());
                rcHit.Inflate(MarkerW, MarkerH);

                for (size_t i = 0;  i < ElemN;  i++ ) {
                    TModelUnit mX = pArX->GetElem(i);
                    TModelUnit mY = pArY->GetElem(i);
                    if (rcHit.PtInRect(mX, mY))
                        CGraphDotMarker::RenderMarker(mX, mY, MarkerW, MarkerH, m_MarkerType);
                }
            }
        }
        catch(CException& e){
            ERR_POST(e.ReportAll());
        }
        catch(std::exception& e) {
            ERR_POST(e.what());
        }
        pPane->Close();
    }
}

void    CScatterGraph ::CalculateLimits()
{
    IScatterDataSource* pSource = GetScatterDataSource();
    bool bEmpty = true;
    if(pSource)
    {
        INumericArray* pArX = pSource->GetXArray();
        INumericArray* pArY = pSource->GetYArray();
        size_t ElemN = pArX->GetSize();
        if(ElemN>0)
        {
            bEmpty = false;
            TModelUnit MinX, MaxX, MinY, MaxY;
            MinX = MaxX = pArX->GetElem(0);
            MinY = MaxY = pArY->GetElem(0);
            for (size_t i = 1;  i < ElemN;  i++ ) {
                TModelUnit X = pArX->GetElem(i);
                TModelUnit Y = pArY->GetElem(i);
                MinX = min(MinX, X);
                MaxX = max(MaxX, X);
                MinY = min(MinY, Y);
                MaxY = max(MaxY, Y);
            }
            m_Limits.Init(MinX, MinY, MaxX, MaxY);
        }
    }
    if(bEmpty)
        m_Limits.Init(0, 0, 1, 1);
}

END_NCBI_SCOPE
