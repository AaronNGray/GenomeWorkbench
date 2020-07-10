/*  $Id: axis.cpp 27503 2013-02-26 18:16:55Z falkrb $
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
#include <gui/graph/axis.hpp>
#include <gui/opengl/glutils.hpp>

#include <math.h>
#include <stdio.h>

BEGIN_NCBI_SCOPE

////////////////////////////////////////////////////////////////////////////////
/// class CAxisRenderer

const int CAxisRenderer::kDefTickSize = 5;

CAxisRenderer::CAxisRenderer(bool bHorz)
:   m_bHorz(bHorz),
    m_Type(eDec),
    m_PosType(ePosMin),
    m_FixedPosValue(0),
    m_TextAlignType(eLeftText),
    m_sFormatTempl("%g"),
    m_TickStyle(eBothSides), m_TickSize(kDefTickSize),
    m_Color(0.0f, 0.0f, 0.2f),
    m_TextColor(0.5f, 0.5f, 0.5f),
    m_Font(CGlBitmapFont::eHelvetica12)
{
}

CAxisRenderer::~CAxisRenderer()
{
}

void    CAxisRenderer::SetHorizontal(bool bHorz)
{
    m_bHorz = bHorz;
}
void    CAxisRenderer::SetPositionType(EPosition Type)
{
    m_PosType = Type;
}

void    CAxisRenderer::SetFixedPosition(TModelUnit Value)
{
    m_FixedPosValue = Value;
}

void    CAxisRenderer::SetTextAlign(ETextAlign Type)
{
    m_TextAlignType = Type;
}

// function tales sTempl that is a printf-style temaplate for formatting a value of
// TModelUnit type
void    CAxisRenderer::SetFormattingTemplate(const string& sTempl)
{
    m_sFormatTempl = sTempl;
}

/*###void    CAxisRenderer::SetFont(CGlBitmapFont::EFont Type)
{
    m_Font = CGlBitmapFont(Type);
}*/

void    CAxisRenderer::SetTickStyle(ETickStyle Style)
{
    m_TickStyle = Style;
}

void    CAxisRenderer::SetTickSize(int SizePix)
{
    m_TickSize = SizePix;
}

void    CAxisRenderer::SetScaleType(EScaleType Type)
{
    m_Type = Type;
}

void    CAxisRenderer::SetColor(CRgbaColor Color)
{
    m_Color = Color;
}

void    CAxisRenderer::SetTextColor(CRgbaColor Color)
{
    m_TextColor = Color;
}

void    CAxisRenderer::Render(CGlPane* pAreaPane, CGlPane* pGraphPane, CRegularGridGen* pGen)
{
    // in order for CGlPane::Project() to work we need to initilize pane
    pGraphPane->OpenOrtho();
    pGraphPane->Close();

    TVPRect   rcVP = pAreaPane->GetViewport();
    TVPRect   rcGraphVP = pGraphPane->GetViewport();

    // now drawing
    pAreaPane->OpenPixels();

    pGen->GenerateGrid(pGraphPane, m_bHorz);
    x_GenerateLabels(pAreaPane, pGraphPane, pGen);

    glLineWidth(0.1f);

    int dTickMin = (m_TickStyle == eMinSide || m_TickStyle == eBothSides) ? -m_TickSize : 0;
    int dTickMax = (m_TickStyle == eMaxSide || m_TickStyle == eBothSides) ? m_TickSize : 0;

    if(m_bHorz) {
        x_DrawHorzLabels(pAreaPane, pGraphPane, pGen, dTickMin, dTickMax);
    } else {
        int vpX = 0;
        switch(m_PosType) {
        case ePosMin: vpX = rcGraphVP.Left(); break;
        case ePosMax: vpX = rcGraphVP.Right(); break;
        case ePosFixedValue: vpX = pGraphPane->ProjectX(m_FixedPosValue); break;
        }

        glColorC(m_Color);
        glBegin(GL_LINES);
            // draw axis
            double vpBottom = rcGraphVP.Bottom();
            double vpTop = rcGraphVP.Top();

            glVertex2d(vpX, vpBottom);
            glVertex2d(vpX, vpTop);

            if (m_TickStyle != eNoTick) {
                // draw Ticks
                ITERATE(CRegularGridGen, it, *pGen) {
                    double wY = *it;
                    int vpY = pGraphPane->ProjectY(wY);
                    if(vpY >= vpBottom  &&  vpY <= vpTop)
                    {
                        glVertex2d(vpX + dTickMin, vpY);
                        glVertex2d(vpX + dTickMax, vpY);
                    }
                }
            }
        glEnd();

        // drawing Labels
        x_LayoutLabels(rcVP.Bottom(), rcVP.Top());
        glColorC(m_TextColor);
        int H = (int)m_Font.TextHeight();
        ITERATE(vector<SLabelDescr>, itL, m_vLabels) {
            if (itL->m_bVisible) {
                int tX = vpX;

                switch (m_TextAlignType) {
                case eLeftText: tX +=10; break;
                case eRightText: tX -= 10 + itL->m_Width; break;
                case eCenterText: tX -= itL->m_Width / 2; break;
                }
                int tY = itL->m_Pos - H/2;
                m_Font.TextOut(tX, tY, itL->m_Str.c_str());
            }
        }
    }

    pAreaPane->Close();
}

void    CAxisRenderer::x_DrawHorzLabels(CGlPane* pAreaPane, CGlPane* pGraphPane, CRegularGridGen* pGen, int dTickMin, int dTickMax)
{
    TVPRect   rcVP = pAreaPane->GetViewport();
    TVPRect   rcGraphVP = pGraphPane->GetViewport();

    int vpY = 0;
    switch(m_PosType) {
    case ePosMin: vpY = rcGraphVP.Bottom(); break;
    case ePosMax: vpY = rcGraphVP.Top(); break;
    case ePosFixedValue: vpY = pGraphPane->ProjectY(m_FixedPosValue); break;
    }
    glColorC(m_Color);
    glBegin(GL_LINES);

        // draw axis
        int vpLeft = pGraphPane->GetViewport().Left();
        int vpRight = pGraphPane->GetViewport().Right();

        glVertex2d(vpLeft, vpY);
        glVertex2d(vpRight, vpY);

        if (m_TickStyle != eNoTick) {
            // draw Ticks
            ITERATE(CRegularGridGen, it, *pGen) {
                double wX = *it;
                int vpX = pGraphPane->ProjectX(wX);
                if(vpX >= vpLeft  &&  vpX <= vpRight)
                {
                    glVertex2d(vpX, vpY + dTickMin);
                    glVertex2d(vpX, vpY + dTickMax);
                }
            }
        }
    glEnd();

    // drawing Labels
    x_LayoutLabels(rcVP.Left(), rcVP.Right());
    glColorC(m_TextColor);
    int tY = vpY - 10 - (int) m_Font.TextHeight();
    ITERATE(vector<SLabelDescr>, itL, m_vLabels) {
        if (itL->m_bVisible) {
            int tX = itL->m_Pos;

            switch (m_TextAlignType) {
            case eLeftText: break;
            case eRightText: tX -= itL->m_Width; break;
            case eCenterText: tX -= itL->m_Width / 2; break;
            }

            m_Font.TextOut(tX, tY, itL->m_Str.c_str());
        }
    }
}
void    CAxisRenderer::x_GenerateLabels(CGlPane* pAreaPane, CGlPane* pGraphPane, CRegularGridGen* pGen)
{
    m_vLabels.resize(0);
    int vpZ, vpStart, vpFinish;
    ITERATE(CRegularGridGen, it, *pGen) {
        if(m_bHorz) {
            vpZ = pGraphPane->ProjectX(*it);
            vpStart = pGraphPane->GetViewport().Left();
            vpFinish = pGraphPane->GetViewport().Right();
        } else {
            vpZ = pGraphPane->ProjectY(*it);
            vpStart = pGraphPane->GetViewport().Bottom();
            vpFinish = pGraphPane->GetViewport().Top();
        }

        if(vpZ >= vpStart  &&  vpZ <= vpFinish)
        {
            SLabelDescr Label;
            Label.m_Value = *it;
            Label.m_Pos = vpZ;
            TModelUnit V = x_GetLabelValue(*it);
            Label.m_Str = x_FormatLabel(V);
            Label.m_Width = (int)m_Font.TextWidth(Label.m_Str.c_str());
            m_vLabels.push_back(Label);
        }
    }
}

#define LABEL_SPACE_X 8
#define LABEL_SPACE_Y 1

void    CAxisRenderer::x_LayoutLabels(int Start, int Finish)
{
    int Min = Start;
    int Max = Finish;
    int Space = m_bHorz ? LABEL_SPACE_X : LABEL_SPACE_Y;
    int H = (int)m_Font.TextHeight();

    NON_CONST_ITERATE(vector<SLabelDescr>, it, m_vLabels) {
        int Size = m_bHorz ? it->m_Width : H;
        int LabelMin = it->m_Pos - Size / 2;
        int LabelMax = LabelMin + Size + Space;

        if (LabelMin < Min  || LabelMax > Max)
            it->m_bVisible = false;
        else {
            it->m_bVisible = true;
            Min = LabelMax + 1;
        }
    }
}


string CAxisRenderer::x_FormatLabel(TModelUnit Value)
{
    char S[200];
    sprintf(S, m_sFormatTempl.c_str(), Value);
    return S;
}

TModelUnit  CAxisRenderer::x_GetLabelValue(TModelUnit Value)
{
    switch(m_Type) {
    case eLog2:
        return pow(TModelUnit(2.0), Value);
    case eLog10:
        return pow(TModelUnit(10.0), Value);
    case eLn:
        return exp(Value);
    case eDec:
        return Value;
    default:
        _ASSERT(false);
        return -1;
    }
}

////////////////////////////////////////////////////////////////////////////////
/// class CAxisArea

CAxisArea::CAxisArea()
: m_BoundsColor(0.5f, 0.5f, 0.5f)
{
}


CAxisArea::~CAxisArea()
{
}

//override this functions in order to customize axes
void    CAxisArea::SetupAxes()
{
    static const int kDefTickSize = 4;

    m_XAxis.SetHorizontal(true);
    m_XAxis.SetTickSize(kDefTickSize);
    m_XAxis.SetTickStyle(CAxisRenderer::eBothSides);
    m_XAxis.SetTextAlign(CAxisRenderer::eCenterText);

    m_YAxis.SetHorizontal(false);
    m_YAxis.SetTickSize(kDefTickSize);
    m_YAxis.SetTickStyle(CAxisRenderer::eBothSides);
    m_YAxis.SetTextAlign(CAxisRenderer::eRightText);
}

void    CAxisArea::SetScaleType(EScaleType TypeX, EScaleType TypeY)
{
    m_XAxis.SetScaleType(TypeX);
    m_YAxis.SetScaleType(TypeY);
}

void    CAxisArea::Render(CGlPane* pAreaPane, CGlPane* pGraphPane, CRegularGridGen* pGen)
{
    //drawing bounding rectangle
    /*pAreaPane->OpenPixels();

    TVPRect rcGraphVP = pGraphPane->GetViewport();

    glColorC(m_BoundsColor);
    glPolygonMode(GL_FRONT, GL_LINE);
    glRectd(rcGraphVP.Left(), rcGraphVP.Bottom(), rcGraphVP.Right(), rcGraphVP.Top());

    pAreaPane->Close(); */

    // drawing Axes
    m_XAxis.Render(pAreaPane, pGraphPane, pGen);
    m_YAxis.Render(pAreaPane, pGraphPane, pGen);
}

END_NCBI_SCOPE
