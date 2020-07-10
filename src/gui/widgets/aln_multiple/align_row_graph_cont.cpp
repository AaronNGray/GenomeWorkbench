/*  $Id: align_row_graph_cont.cpp 41823 2018-10-17 17:34:58Z katargir $
 * ===========================================================================
 *
 *                            PUBLIC DOMAIN NOTICE
 *               National Center for Biotechnology Information
 *
 *  This software/database is a 'United States Government Work' under the
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
#include <corelib/ncbistd.hpp>

#include <gui/widgets/aln_multiple/align_row_graph_cont.hpp>
#include <objtools/alnmgr/aln_explorer.hpp>

#include <gui/opengl/glhelpers.hpp>
#include <gui/opengl/irender.hpp>
#include <gui/types.hpp>

#include <math.h>
#include <memory>


BEGIN_NCBI_SCOPE
USING_SCOPE(objects);


////////////////////////////////////////////////////////////////////////////////
/// CAlignRowGraphCont

CAlignRowGraphCont::CAlignRowGraphCont()
:   CGraphContainer(eHorzStack),
    m_Created(false)
{
}


CAlignRowGraphCont::~CAlignRowGraphCont()
{
    Destroy();
}

void CAlignRowGraphCont::Layout()
{
    CGraphContainer::Layout();

    /// adjust model space for graphs
    /// the trick is that model space is in the same units as viewport (pixels), however
    /// it is flipped so that range [a, b] in viewport corresponds to [b, a] in model space
    switch(m_LayoutPolicy)  {
    case eHorzStack:    {
        TVPRect rc_vp(m_VPRect);
        TVPUnit shift = 0;
        TModelRect rc_vis(m_ModelRect);

        NON_CONST_ITERATE(TGraphs, it, m_Graphs)  {
            TGraph& graph = **it;
            TVPUnit size = graph.PreferredSize().Y();

            // adjust viewport rect
            TVPUnit vp_bottom = rc_vp.Bottom()  + shift;
            TVPUnit vp_top = vp_bottom + (size - 1);
            rc_vp.SetVert(vp_bottom, vp_top);
            graph.SetVPRect(rc_vp);

            // adjust model rect
            TModelUnit m_bottom = m_ModelRect.Bottom() - shift;
            TModelUnit m_top = m_bottom - (size - 1);
            rc_vis.SetVert(m_bottom, m_top);
            graph.SetModelRect(rc_vis);

            shift += size;
        }
        break;
    }
    case eVertStack:
    default:
        _ASSERT(false);
        break;
    }
}


bool    CAlignRowGraphCont::IsCreated() const
{
    return m_Created;
}


bool CAlignRowGraphCont::Create()
{
    for( size_t i = 0; i < m_Graphs.size();  i++ )  {
        TGraph* graph = m_Graphs[i];
        IAlnVecRowGraph* av_graph = dynamic_cast<IAlnVecRowGraph*>(graph);
        if(av_graph)    {
            av_graph->Create();
        }
    }
    m_Created = true;
    return true;
}


void CAlignRowGraphCont::Destroy()
{
    for( size_t i = 0; i < m_Graphs.size();  i++ )  {
        TGraph* graph = m_Graphs[i];
        IAlnVecRowGraph* av_graph = dynamic_cast<IAlnVecRowGraph*>(graph);
        if(av_graph)    {
            av_graph->Destroy();
        }
    }
    m_Created = false;
}


void CAlignRowGraphCont::Update(double start, double stop)
{
    for( size_t i = 0; i < m_Graphs.size();  i++ )  {
        TGraph* graph = m_Graphs[i];
        IAlnVecRowGraph* av_graph = dynamic_cast<IAlnVecRowGraph*>(graph);
        if(av_graph)    {
            av_graph->Update(start, stop);
        }
    }
}

const IAlnRowGraphProperties* CAlignRowGraphCont::GetProperties() const
{
    return NULL;
}


void CAlignRowGraphCont::SetProperties(IAlnRowGraphProperties* props)
{
}


// renders both Confidence graph and Chromatograms
void CAlignRowGraphCont::Render(CGlPane& pane, IAlnSegmentIterator& it)
{
    //LOG_POST("CAlignRowGraphCont::Render(");
    //LOG_POST("    Viewport " << m_VPRect.ToString()
    //         << "\n    Visible " << m_ModelRect.ToString());
    IRender& gl = GetGl();

    pane.OpenOrtho();

    gl.Color3d(0.5, 0.5, 0.5);
    gl.RectC(pane.GetViewport());

    pane.Close();

    for( size_t i = 0; i < m_Graphs.size();  i++ )  {
        TGraph* graph = m_Graphs[i];
        IAlnVecRowGraph* av_graph = dynamic_cast<IAlnVecRowGraph*>(graph);
        if(av_graph)    {
            auto_ptr<IAlnSegmentIterator> it_copy(it.Clone());
            av_graph->Render(pane, *it_copy);
        }
    }
}


END_NCBI_SCOPE
