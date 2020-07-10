/*  $Id: regular_grid.cpp 41823 2018-10-17 17:34:58Z katargir $
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
#include <gui/graph/regular_grid.hpp>
#include <gui/opengl/glutils.hpp>
#include <gui/opengl/irender.hpp>

#include <math.h>

BEGIN_NCBI_SCOPE

#define DEF_MIN_CELL_SIZE 30
#define DEF_MAX_CELL_SIZE 75

const int  CRegularGridGen::ms_StepK[] = { 2, 5, 10 };


CRegularGridGen::CRegularGridGen()
:   m_MinCellSize(DEF_MIN_CELL_SIZE),
    m_MaxCellSize(DEF_MAX_CELL_SIZE),
//    m_XScaleType(eDec), m_YScaleType(eDec),
    m_IntegerX(false),
    m_IntegerY(false),
    m_OneBasedX(false),
    m_OneBasedY(false),
    m_MinusOneBasedX(false),
    m_MinusOneBasedY(false),
    m_AutoStepUpdate(true),
    m_bHorz(true),
    m_Start(0.0), m_Finish(0.0),
    m_Step(1.0)
{
}


CRegularGridGen::~CRegularGridGen()
{
}


void    CRegularGridGen::GenerateGrid(CGlPane* pGraphPane, bool bHorz)
{
    m_bHorz = bHorz;
    bool bInteger = m_bHorz ? m_IntegerX : m_IntegerY;

    TModelRect rcVisible = pGraphPane->GetVisibleRect();

    if(m_bHorz) {
        m_Start = min(rcVisible.Left(), rcVisible.Right());
        m_Finish = max(rcVisible.Left(), rcVisible.Right());
    } else {
        m_Start = min(rcVisible.Bottom(), rcVisible.Top());
        m_Finish = max(rcVisible.Bottom(), rcVisible.Top());
    }

    if (m_AutoStepUpdate) {
        m_Step = 1.0;
    }

    if(m_Finish > m_Start)
    {
        double base_step = SelectBaseStep(m_Start, m_Finish);
        double min_cell = fabs(m_bHorz ? pGraphPane->UnProjectWidth(m_MinCellSize)
                                       : pGraphPane->UnProjectHeight(m_MinCellSize));
        double max_cell = fabs(m_bHorz ? pGraphPane->UnProjectWidth(m_MaxCellSize)
                                       : pGraphPane->UnProjectHeight(m_MaxCellSize));

        if (m_AutoStepUpdate) {
            m_Step = SelectScreenStep(base_step, min_cell, max_cell);
        } else {
            // make sure that the step (set externally) is reasonalble
            if (m_Step < min_cell) {
                m_Step = min_cell;
            }
        }

        if(bInteger && m_Step < 1.0)
            m_Step = 1.0;
        double base_origin = ceil(m_Start / m_Step) - 1;

        base_origin *= m_Step;
        if((bHorz  &&  m_OneBasedX)  ||  (! bHorz  &&  m_OneBasedY))  {
            base_origin++;
        }

        if((bHorz  &&  m_MinusOneBasedX)  ||  (! bHorz  &&  m_MinusOneBasedY))  {
            base_origin--;
        }

        m_Start = base_origin;
    } else {
        //range Start - Finish is empty, make it negtaive (not iterable)
        m_Finish = m_Start - 1;
    }
}


// function calculates "sensible" step for representing labels, drawing gris etc. on graphs
// minV and MaxV define range of values to be represented
// it is assumed that desired number of "ticks" ( BaseStep is a difference between two neighbour ticks)
// should be between 8 and 20.
double  CRegularGridGen::SelectBaseStep(double MinV, double MaxV )
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
        BaseStep = Step / ms_StepK[i++];
        nTicks = Range / BaseStep;
    }
    return BaseStep;
}


void  CRegularGridGen::RoundRangeToStep(double& Start, double& Finish, double Step )
{
    double V = Start;
    V = floor(V / Step) * Step;
    Start = V;

    V = Finish;
    V = ceil(V / Step) * Step;
    Finish = V;
}


// Scale = Pixels per Unit
// return optimal step values calculated on the basis of given BaseStep, MinStep and MaxStep
//(corresponding to min and max cell size in pixels respectively)
double CRegularGridGen::SelectScreenStep(double BaseStep,  double MinStep, double MaxStep )
{
    if ((BaseStep >= MinStep && BaseStep <= MaxStep) || (MinStep == MaxStep))
        return BaseStep;
    else {
        // BaseStep has a form M * pow(10, P), where P is power
        double Power = log10(BaseStep);
        double P = ceil(Power) -1;
        double pow10P = pow(10.0, P);
        double M = BaseStep / pow10P;
        if (M >= 10) {
            M /= 10;
            pow10P *= 10;
        }

        int oldK = 1, K = 1, Index = 0;

        if(BaseStep < MinStep) // increasing BaseStep to make it more then minPix
        {
            double minK = MinStep / pow10P;
            while (K < minK)
            {
                if(Index <2)
                    K = oldK * ms_StepK[Index++];
                else {
                    K = oldK = oldK * 10;
                    Index = 0;
                }
            }
            BaseStep = pow10P * K;
        } else if (BaseStep > MaxStep) { // decreasing BaseStep to make it less then maxPix
            pow10P *= 10;
            double maxK = pow10P / MaxStep;
            while (K < maxK)
            {
                if(Index <2)
                    K = oldK * ms_StepK[Index++];
                else {
                    K = oldK = oldK * 10;
                    Index = 0;
                }
            }
            BaseStep = pow10P / K;
        }
        return BaseStep;
    }
}

CRegularGridRenderer::CRegularGridRenderer()
:   m_bCentering(false),
    m_HorzColor(0.8f, 0.8f, 0.8f),
    m_VertColor(0.8f, 0.8f, 0.8f),
    m_ShowHorzGrid(true),
    m_ShowVertGrid(true)
{
}

void   CRegularGridRenderer::Render(CGlPane* pAreaPane,
                                    CGlPane* pGraphPane,
                                    CRegularGridGen* pGenerator) const
{
    IRender& gl = GetGl();

    {{
        // in order for CGlPane::Project() to work we need to
        // initilize graph pane
        CGlPaneGuard GUARD(*pGraphPane, CGlPane::eOrtho);
    }}

    CGlPaneGuard GUARD(*pAreaPane, CGlPane::ePixels);

    TVPRect   rcVP = pGraphPane->GetViewport();
    double vpBottom = rcVP.Bottom();
    double vpTop = rcVP.Top();
    double vpLeft = rcVP.Left();
    double vpRight = rcVP.Right();

    gl.LineWidth(1.0);

    double d = m_bCentering ? 0.5 : 0.0;

    // horizontal grid (vert. lines)
    if (m_ShowHorzGrid) {
        gl.ColorC(m_VertColor);
        gl.Begin(GL_LINES);
        pGenerator->GenerateGrid(pGraphPane, true);
        ITERATE(CRegularGridGen, it, *pGenerator) {
            double X = *it + d;
            int vpX = pGraphPane->ProjectX(X);
            if (vpX >= vpLeft && vpX <= vpRight) {
                gl.Vertex2d(vpX, vpBottom);
                gl.Vertex2d(vpX, vpTop);
            }
        }
        gl.End();
    }

    // vertical grid (horz. lines)
    if (m_ShowVertGrid) {
        gl.ColorC(m_HorzColor);
        gl.Begin(GL_LINES);
        pGenerator->GenerateGrid(pGraphPane, false);
        ITERATE(CRegularGridGen, it, *pGenerator) {
            double Y = *it + d;
            int vpY = pGraphPane->ProjectY(Y);
            if (vpY >= vpBottom && vpY <= vpTop) {
                gl.Vertex2d(vpLeft, vpY);
                gl.Vertex2d(vpRight, vpY);
            }
        }
        gl.End();
    }
}

END_NCBI_SCOPE
