/*  $Id: pie_graph.cpp 23958 2011-06-24 15:24:59Z wuliangs $
 * ===========================================================================
 *
 *                            PUBLIC DOMAIN NOTICE
 *               National Center for Biotechnology Information
 *
 *  This software / database is a "United States Government Work" under the
 *  terms of the United States Copyright Act.  It was written as part of
 *  the author's official duties as a United States Government employee and
 *  thus cannot be copyrighted.  This software / database is freely available
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
#include <gui/graph/pie_graph.hpp>
#include <gui/opengl/glutils.hpp>

#include <math.h>

BEGIN_NCBI_SCOPE

CPieGraph::CPieGraph()
{
    SetColor(CRgbaColor(0.0f, 0.0f, 0.0f));
}


bool    CPieGraph::SetDataSource(IGraphDataSource* pDS)
{
    IPieDataSource* pPieDS = dynamic_cast<IPieDataSource*>(pDS);
    bool bOk =  pPieDS!= NULL;
    CGraphBase::SetDataSource(bOk ? pDS : NULL);

    CalculateLimits();
    return bOk;
}


void    CPieGraph::Render(CGlPane* pPane, TElemVector* elems)
{
    _ASSERT(pPane);
    IPieDataSource* pSource = GetPieDataSource();
    if (pPane  &&  pSource) {
        pPane->OpenOrtho();
        try {
            INumericArray*  pValues = pSource->GetValueArray();
            IColorArray*    pColors = pSource->GetColorArray();
        //IStringArray*   pLabels = pSource->GetLabelsArray();

            size_t N = pValues->GetSize();
            double Sum = 0.0;
            for (size_t i =0;  i<N;  i++ ) {
                Sum += ::fabs(pValues->GetElem((int)i));
            }

            // choose number of segments
            double D = 0.5 *(pPane->GetViewport().Width() + pPane->GetViewport().Height());
            double MinD = min(pPane->GetViewport().Width(), pPane->GetViewport().Height());
            double SegL = sqrt(MinD); //pixels
            double NSegmPerDegree = 3.14 * D / (SegL * 360);

            double kA = 360.0 / Sum;
            double aStart = 0.0;

            glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
            GLUquadricObj *pQObj = gluNewQuadric();


            for (size_t i = 0;  i < N;  i++) {
                double V = ::fabs(pValues->GetElem(i));
                double aSweep = V * kA;
                int Slices = 1 + (int) ceil(aSweep * NSegmPerDegree);

                // drawing sector
                glColorC(pColors->GetElem(i));
                gluQuadricDrawStyle(pQObj, GLU_FILL);
                gluPartialDisk(pQObj, 0, 1, Slices, 1, aStart, aSweep);

                //drawing perimeter
                glColorC(m_Color);
                gluQuadricDrawStyle(pQObj, GLU_SILHOUETTE);
                gluPartialDisk(pQObj, 0, 1, Slices, 1, aStart, aSweep);

                aStart += aSweep;
            }
            gluDeleteQuadric(pQObj);
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


void    CPieGraph::CalculateLimits()
{
    m_Limits.Init(-1.0, -1.0, 1.0, 1.0);
}


bool    CPieGraph::ShowMarkers()
{
    return false;
}


IStringArray*   CPieGraph::GetLabelArray()
{
    IPieDataSource* pDS = GetPieDataSource();
    return pDS ? pDS->GetLabelsArray() : NULL;
}


IColorArray*    CPieGraph::GetColorArray()
{
    IPieDataSource* pDS = GetPieDataSource();
    return pDS ? pDS->GetColorArray() : NULL;
}


INumericArray*  CPieGraph::GetMarkerArray()
{
    return NULL;
}


/////////////////////////////////////////////////////////////////////////////
/// CPieDataSource
CPieDataSource::CPieDataSource(int SectorsN, const string& Name)
    : CSeriesBase(SectorsN),
m_Name(Name)
{
}


void    CPieDataSource::CreateArrays()
{
    CSeriesBase::CreateArrays();

    TValueAdapter* pValAd = new TValueAdapter(m_Length);
    AddArray(static_cast<IDataArray*>(pValAd));

    TColorAdapter* pCAd = new TColorAdapter(m_Length);
    AddArray(static_cast<IDataArray*>(pCAd));

    TStringAdapter* pLabAd = new TStringAdapter(m_Length);
    AddArray(static_cast<IDataArray*>(pLabAd));
}


END_NCBI_SCOPE
