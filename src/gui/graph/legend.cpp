/*  $Id: legend.cpp 23948 2011-06-23 17:05:49Z kuznets $
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
#include <gui/graph/legend.hpp>
#include <gui/opengl/glutils.hpp>

BEGIN_NCBI_SCOPE

///////////////////////////////////////////////////////////////////////////////
/// class CLegend

CLegend::CLegend()
    : m_Font(CGlBitmapFont::eHelvetica12),
      m_BackColor(0.95f, 0.95f, 0.95f),
      m_BorderColor(0.0f, 0.0f, 0.0f),
      m_bHorz(true),
      m_Space(10)
{
}

CLegend::~CLegend()
{
}

void    CLegend::SetBackColor(const CRgbaColor& Color)
{
    m_BackColor = Color;
}
const CRgbaColor& CLegend::GetBackColor() const
{
    return m_BackColor;
}

void    CLegend::SetBorderColor(const CRgbaColor& Color)
{
    m_BorderColor = Color;
}

const CRgbaColor& CLegend::GetBorderColor()
{
    return m_BorderColor;
}

bool    CLegend::SetDataSource(IGraphDataSource* pDS)
{
    ILegendDataSource* pLegendDS = dynamic_cast<ILegendDataSource*>(pDS);
    bool bOk =  pLegendDS!= NULL;
    CGraphBase::SetDataSource(bOk ? pDS : NULL);

    CalculateLimits();
    return bOk;
}

const   TModelRect&    CLegend::GetLimits()    const
{
    return m_Limits;
}

void CLegend::CalculateLimits()
{
    m_Limits.Init(0, 0, 1, 1);
}

void    CLegend::Render(CGlPane* pPane, TElemVector* elems)
{
    if(pPane) {
        pPane->OpenPixels();
        try {
            TVPRect rcVP = pPane->GetViewport();

            // fill background
            glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
            glColorC(m_BackColor);
            glRecti(rcVP.Left(), rcVP.Bottom(), rcVP.Right(), rcVP.Top());

            // draw Border
            glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
            glColorC(m_BorderColor);
            glRecti(rcVP.Left(), rcVP.Bottom(), rcVP.Right(), rcVP.Top());

            x_RenderItems(pPane);
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

void    CLegend::x_RenderItems(CGlPane* pPane)
{
    ILegendDataSource* pSource = GetLegendDataSource();
    if (pSource) {
        IStringArray* pLabels = pSource->GetLabelArray();
        IColorArray*  pColors = pSource->GetColorArray();
        INumericArray* pMarkers = pSource->GetMarkerArray();

        int H = max( (int)m_Font.TextHeight(), 10);

        TVPRect rcVP = pPane->GetViewport();
        int LeftLimit = rcVP.Left() + m_Space;
        int RightLimit  = rcVP.Right() - m_Space;
        int BottomLimit = rcVP.Bottom() + m_Space;

        int vpX = LeftLimit;
        int StepY = H + max(H / 2, 4);
        int vpY = rcVP.Top() - m_Space - StepY;
        m_BoxW = pSource->ShowMarkers() ? H * 2 : H;
        m_BoxH = H;

        int MaxX = LeftLimit;

        size_t N = pLabels->GetSize();
        for ( size_t i = 0;  i < N;  i++ ) { // iterating by items
            string sText = pLabels->GetElem(i);
            int W = m_BoxW + m_Space + (int)m_Font.TextWidth(sText.c_str());
            int iMarker = pSource->ShowMarkers() ? (int) pMarkers->GetElem(i) : -1;

            if( m_bHorz) {
                if (vpX + W > RightLimit) { // new line
                    vpY -= StepY;
                    vpX = LeftLimit;
                }
                x_RenderItem(vpX, vpY, sText, pColors->GetElem(i), iMarker);

                vpX += W + m_Space * 2;
            } else {
                x_RenderItem(vpX, vpY, sText, pColors->GetElem(i), iMarker);

                MaxX = max(MaxX, vpX + W);
                vpY -= StepY;

                if (vpY < BottomLimit) { // new column
                    vpY = rcVP.Top() - m_Space - StepY;
                    vpX = MaxX + m_Space;
                }
            }
        }
    }
}

void CLegend::x_RenderItem(int X, int Y, const string& sLabel, const CRgbaColor& Color, int iMarker)
{
    CGraphDotMarker::EMarkerType Type = static_cast<CGraphDotMarker::EMarkerType>(iMarker);

    switch (Type) {
    case CGraphDotMarker::eNone:
    case CGraphDotMarker::eRect:
    case CGraphDotMarker::eDiamond:
    case CGraphDotMarker::eTriangle:
    case CGraphDotMarker::eCross: {
        //draw line
        glColorC(Color);
        glBegin(GL_LINES);
            glVertex2i(X, Y + m_BoxH / 2);
            glVertex2i(X + m_BoxW, Y + m_BoxH / 2);
        glEnd();

        // draw marker
        TModelUnit MarkerSize = (m_BoxH % 2) ? m_BoxH : (m_BoxH - 1);
        CGraphDotMarker::RenderMarker(X + m_BoxW / 2, Y + m_BoxH / 2, MarkerSize, MarkerSize, Type);
        break;
    }
    default: {
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        glColorC(Color);
        glRectd(X, Y, X + m_BoxW, Y + m_BoxH);

        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        glColorC(m_BorderColor);
        glRectd(X, Y, X + m_BoxW, Y + m_BoxH);
    }
    } //switch

    glColorC(Color);
    m_Font.TextOut((float)(X + m_BoxW + m_Space), (float) Y, sLabel.c_str());
}

///////////////////////////////////////////////////////////////////////////////
/// class CLegendDataSource

void    CLegendDataSource::CreateArrays()
{
    CSeriesBase::CreateArrays();

    TStrAdapter* pStrAd = new TStrAdapter(m_Length);
    AddArray(static_cast<IDataArray*>(pStrAd));

    TColorAdapter* pCAd = new TColorAdapter(m_Length);
    AddArray(static_cast<IDataArray*>(pCAd));

    TEnumAdapter* pEnAd = new TEnumAdapter(m_Length);
    AddArray(static_cast<IDataArray*>(pEnAd));
}

END_NCBI_SCOPE
