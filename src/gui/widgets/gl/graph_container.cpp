/*  $Id: graph_container.cpp 35505 2016-05-16 15:18:00Z shkeda $
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
#include <algorithm>

#include <gui/widgets/gl/graph_container.hpp>

BEGIN_NCBI_SCOPE

class CGraphContainerEvtHandler : public wxEvtHandler
{
    DECLARE_EVENT_TABLE()
public:
    CGraphContainerEvtHandler(CGraphContainer* container)
        : m_Container(container)
    {

    }
    void OnMouseEvents( wxMouseEvent& event ) {
        m_Container->OnMouseEvents(event);
    }
private:
    CGraphContainer* m_Container;

};

BEGIN_EVENT_TABLE(CGraphContainerEvtHandler, wxEvtHandler)
    EVT_MOUSE_EVENTS(CGraphContainerEvtHandler::OnMouseEvents)
END_EVENT_TABLE()


///////////////////////////////////////////////////////////////////////////////
/// CGraphContainer
CGraphContainer::CGraphContainer(ELayoutPolicy policy)
    : m_Handler(new CGraphContainerEvtHandler(this)),
      m_Host(NULL),
      m_LayoutPolicy(policy),
      m_TooltipGraph(NULL),
      m_GraphMaxSize(-1, -1)
{
}


CGraphContainer::~CGraphContainer()
{
    delete m_Handler;
}


bool CGraphContainer::AddGraph(TGraph* graph, bool front)
{
    _ASSERT(graph);

    if (front)
        m_Graphs.insert(m_Graphs.begin(), TGraphRef(graph));
    else {
        m_Graphs.push_back(TGraphRef(graph));
        stable_sort(m_Graphs.begin(), m_Graphs.end(), [](const TGraphRef& a, const TGraphRef& b) {
            return a->GetOrder() > b->GetOrder();
        });
    }

    // if graph support event handling - plug it in
    CEventHandler* handler = dynamic_cast<CEventHandler*>(graph);
    if(handler)  {
        AddListener(handler, ePool_Child);
        handler->AddListener(this, ePool_Parent);
    }
    return true;
}


bool CGraphContainer::RemoveGraph(TGraph* graph)
{
    TGraphs::iterator it = std::find(m_Graphs.begin(), m_Graphs.end(), graph);
    if(it != m_Graphs.end())    {
        CEventHandler* handler = dynamic_cast<CEventHandler*>(graph);
        if(handler)  {
            RemoveListener(handler);
            handler->RemoveListener(this);
        }

        m_Graphs.erase(it);
        return true;
    } else return false;
}


void CGraphContainer::RemoveAllGraphs()
{
    m_Graphs.clear();
}


size_t CGraphContainer::GetGraphsCount() const
{
    return m_Graphs.size();
}

CGraphContainer::TGraph* CGraphContainer::GetGraph(size_t index)
{
    _ASSERT(index < m_Graphs.size());
    return m_Graphs[index].GetPointer();
}


const CGraphContainer::TGraph* CGraphContainer::GetGraph(size_t index) const
{
    _ASSERT(index < m_Graphs.size());
    return m_Graphs[index].GetPointer();
}


void CGraphContainer::Clear()
{
    m_Graphs.clear();
}


void CGraphContainer::Render(CGlPane& pane)
{
    //LOG_POST("CGraphContainer::Render()");
    //LOG_POST("    Viewport " << m_VPRect.ToString() << "\n    Visible " << m_ModelRect.ToString());
    const TVPRect* rc_clip = pane.GetClipRect();

    TVPRect rc_vp;
    NON_CONST_ITERATE(TGraphs, it, m_Graphs) {
        TGraph& graph = **it;

        if(graph.IsVisible())   {
            // clip child graph by its bounds with respect to existing clip rect
            // and bounds of this Graph Container
            TVPRect rc_clip_gr = graph.GetVPRect();
            rc_clip_gr.IntersectWith(pane.GetViewport());
            if(rc_clip) {
                rc_clip_gr.IntersectWith(*rc_clip);
            }
            pane.SetClipRect(&rc_clip_gr);

            graph.Render(pane); // Render

            pane.SetClipRect(rc_clip); // restore old clip
        }
    }
}


void CGraphContainer::SetVPRect(const TVPRect& rect)
{
    m_VPRect = rect;
    Layout();
}


void CGraphContainer::SetModelRect(const TModelRect& rect)
{
    m_ModelRect = rect;
    Layout();
}


void CGraphContainer::Layout()
{
    switch(m_LayoutPolicy)  {
    case eHorzStack:    {
        TVPRect rc_vp(m_VPRect);
        TModelRect rc_model(m_ModelRect);
        for(  size_t i = 0;  i < m_Graphs.size();  i++  )  {
            TGraph& graph = *m_Graphs[i];

            TVPUnit size = graph.PreferredSize().Y();
            if (m_GraphMaxSize.Y() > 0) 
                size = min<int>(m_GraphMaxSize.Y(), size);

            rc_vp.SetTop(rc_vp.Bottom() + size - 1);
            graph.SetVPRect(rc_vp);

            rc_model.SetTop(rc_model.Bottom() + size - 1);
            graph.SetModelRect(rc_model);

            rc_vp.Offset(0, size);
            rc_model.Offset(0, size);

        }
        break;
    }
    case eVertStack:    {
        TVPRect rc_vp(m_VPRect);
        TModelRect rc_model(m_ModelRect);

        NON_CONST_ITERATE(TGraphs, it, m_Graphs)  {
            TGraph& graph = **it;
            TVPUnit size = graph.PreferredSize().X();
            if (m_GraphMaxSize.X() > 0) 
                size = min<int>(m_GraphMaxSize.X(), size);

            rc_vp.SetRight(rc_vp.Left() + size - 1);
            graph.SetVPRect(rc_vp);

            rc_model.SetRight(rc_model.Right() + size - 1);
            graph.SetModelRect(rc_model);

            rc_vp.Offset(size, 0);
            rc_model.Offset(size, 0);
        }
        break;
    }
    default: break;
    }
}


TVPPoint CGraphContainer::PreferredSize()
{
    TVPPoint size(0, 0);
    for(  size_t i = 0;  i < m_Graphs.size();  i++ )    {
        TGraph* graph = m_Graphs[i];
        switch(m_LayoutPolicy)  {
        case eHorzStack: {
            int graph_y = graph->PreferredSize().Y();
            if (m_GraphMaxSize.Y() > 0) 
                graph_y = min<int>(m_GraphMaxSize.Y(), graph_y);
            size.m_Y += graph_y;
            break;
        }
        case eVertStack: {
            int graph_x = graph->PreferredSize().X();
            if (m_GraphMaxSize.X() > 0) 
                graph_x = min<int>(m_GraphMaxSize.X(), graph_x);
            size.m_X += graph_x;
            break;
        }
        case eWhole: {
            TVPPoint sz = graph->PreferredSize();
            size.m_X = max(size.m_X, sz.X());
            if (m_GraphMaxSize.X() > 0) 
                size.m_X = min<int>(m_GraphMaxSize.X(), size.m_X);
            size.m_Y = max(size.m_Y, sz.Y());
            if (m_GraphMaxSize.Y() > 0) 
                size.m_Y = min<int>(m_GraphMaxSize.Y(), size.m_Y);
            break;
         }
        default: break;
        }
    }
    return size;
}


bool CGraphContainer::NeedTooltip(CGlPane& pane, int vp_x, int vp_y)
{
    NON_CONST_ITERATE(TGraphs, it, m_Graphs)  {
        TGraph& graph = **it;
        if(graph.GetVPRect().PtInRect(vp_x, vp_y))    {
            m_TooltipGraph = &graph;
            return m_TooltipGraph->NeedTooltip(pane, vp_x, vp_y);
        }
    }
    return false;
}


string CGraphContainer::GetTooltip()
{
    _ASSERT(m_TooltipGraph);
    return m_TooltipGraph->GetTooltip();
}


void CGraphContainer::OnMouseEvents( wxMouseEvent& event ) 
{
    event.Skip();
    TVPPoint p = m_Host->GCH_GetVPPosByWindowPos(event.GetPosition());
    NON_CONST_ITERATE(TGraphs, it, m_Graphs)  {
        TGraph& graph = **it;
        wxEvtHandler* handler = dynamic_cast<wxEvtHandler*>(&graph);
        if (!handler) 
            continue;
        if(graph.GetVPRect().PtInRect(p)) {
            wxMouseEvent e(event);
            e.m_x = p.X(); 
            e.m_y = p.Y();
            handler->ProcessEvent(e);
            event.Skip(e.GetSkipped());
            break;
        }
    }
}


END_NCBI_SCOPE
