#ifndef __GUI_WIDGETS_GL___GRAPH_CONTAINER__HPP
#define __GUI_WIDGETS_GL___GRAPH_CONTAINER__HPP

/*  $Id: graph_container.hpp 32173 2015-01-14 15:01:07Z shkeda $
 * ===========================================================================
 *
 *                            PUBLIC DOMAIN NOTICE
 *               National Center for Biotechnology Information
 *
 *  This software/database is a "United States Government Work" under the
 *  terms of the United States Copyright Act.  It was written as part of
 *  the author's official duties as a United States Government employee and
 *  thus cannot be copyrighted.  This software/database is freely avaislable
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

#include <corelib/ncbistl.hpp>

#include <gui/widgets/gl/irenderable.hpp>
#include <gui/widgets/gl/ievent_handler.hpp>
#include <gui/utils/event_handler.hpp>


BEGIN_NCBI_SCOPE

////////////////////////////////////////////////////////////////////////////////
/// IGraphContainerHost
class NCBI_GUIWIDGETS_GL_EXPORT  IGraphContainerHost
{
public:
    virtual ~IGraphContainerHost() {};
    virtual TVPPoint GCH_GetVPPosByWindowPos(const wxPoint& pos) = 0;
};




///////////////////////////////////////////////////////////////////////////////
/// CGraphContainer
/// CGraphContainer a composite IRenderable that manages several child
/// IRenderable graphs. All graphs are rendered using transformation defined
/// by CGraphContainer model space and viewport.

class  NCBI_GUIWIDGETS_GL_EXPORT  CGraphContainer :
    public CObjectEx, 
    public IGlEventHandler,
    public CRenderableImpl,
    public CEventHandler
{
public:
    typedef IRenderable TGraph;
    typedef CIRef<IRenderable>   TGraphRef;

    enum    ELayoutPolicy {
        eHorzStack,
        eVertStack,
        eWhole
    };

    CGraphContainer(ELayoutPolicy policy);
    virtual ~CGraphContainer();

    virtual bool    AddGraph(TGraph* graph, bool front = false); /// assumes ownership
    virtual bool    RemoveGraph(TGraph* graph);
    virtual void    RemoveAllGraphs();

    virtual size_t  GetGraphsCount() const;

    virtual TGraph* GetGraph(size_t index);
    virtual const TGraph* GetGraph(size_t index) const;

    virtual void    Clear();
    virtual void    Layout();

    void SetHost(IGraphContainerHost* host);
    
    /// @name IRenderable implementation
    /// @{
    virtual void Render(CGlPane& pane);

    virtual void    SetVPRect(const TVPRect& rect);

    virtual void    SetModelRect(const TModelRect& rc);

    virtual TVPPoint    PreferredSize();

    virtual bool    NeedTooltip(CGlPane& pane, int vp_x, int vp_y);
    virtual string  GetTooltip();
    /// @}
    
    /// @name IGlEventHandler implementation
    /// @{
    virtual wxEvtHandler* GetEvtHandler();
    virtual void SetPane(CGlPane* pane);
    /// @}

    void OnMouseEvents( wxMouseEvent& event );

    void SetGraphMaxSize(const TVPPoint& size);

protected:
    typedef vector<TGraphRef>   TGraphs;

    wxEvtHandler*       m_Handler;
    IGraphContainerHost* m_Host;
    ELayoutPolicy    m_LayoutPolicy;

    TGraphs     m_Graphs;
    TGraph*     m_TooltipGraph;
    CGlPane*    m_Pane;

    TVPPoint m_GraphMaxSize;

};


///////////////////////////////////////////////////////////////////////////////
/// CGraphEvent

class  NCBI_GUIWIDGETS_GL_EXPORT  CGraphNotification : public CEvent
{
public:
    enum    EType   {
        eContentChanged,
        eSizeChanged
    };

    CGraphNotification(EType type) :   CEvent(type)    {};
    EType   GetType() const {   return m_Type;  }

protected:
    EType   m_Type;
};

inline
void CGraphContainer::SetHost(IGraphContainerHost* host)
{
    m_Host = host;
}


inline
wxEvtHandler* CGraphContainer::GetEvtHandler() 
{ 
    return m_Handler; 
}

inline
void CGraphContainer::SetPane(CGlPane* pane) {
    m_Pane = pane;
}


inline
void CGraphContainer::SetGraphMaxSize(const TVPPoint& size)
{
    m_GraphMaxSize = size;
}


END_NCBI_SCOPE

#endif  // __GUI_WIDGETS_GL___GRAPH_CONTAINER__HPP
