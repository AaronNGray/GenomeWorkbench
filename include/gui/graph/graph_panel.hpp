#ifndef GUI_GRAPH___GRAPH_PANEL__HPP
#define GUI_GRAPH___GRAPH_PANEL__HPP

/*  $Id:
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

#include <gui/graph/igraph.hpp>
#include <gui/graph/legend.hpp>
#include <gui/graph/regular_grid.hpp>
#include <gui/graph/axis.hpp>

/** @addtogroup GUI_GRAPH
 *
 * @{
 */

BEGIN_NCBI_SCOPE

////////////////////////////////////////////////////////////////////////////////
/// class CGraphPanel
/// CGraphPanel class is an assembly of a number of "graph" library objects. It
/// contains 3 instances of CGlPane - one for drawing graphs, one for decorations
/// such as grid and axes, and one for drawing legend.
/// CGraphPanel instantiates CGlPane objects, Grid Generator and Renderer, Legend
/// and Axis Area objects. It accepts arbitary number of IGraph objects. The class
/// performs rendering of all its subcomponents with respect to Z-order.
/// Customization of the CGraphPanel can be achived by overriding virtual functions
/// or by calling directly functions of its subcomponents.

class NCBI_GUIGRAPH_EXPORT CGraphPanel
{
public:
    enum    ELimitsMode {
        eGraphLimits, // limits of the pane are defined by limits of the graph
        eRounded, // same as eGraphLimits but extended by rounding to the "nice" numbers
        eFixed    // explicitly specified
    };

    CGraphPanel();
    virtual ~CGraphPanel();

    // Create function should be called in order to create subcomponents
    virtual void    Create();

    // Layout functions
    void    SetRect(const TVPRect& rcAll, bool bLayout = true);

    // GraphOffsetX - distance from the left side of the panel to the left side of Graph pane
    // GraphOffsetY - distance from the top side of Legend to bottom side of Graph pane
    // LegendOffsetX - distance from the left side of the panel to the left side of Legend
    // LegendH - Legend height
    void    SetLayout( int GraphOffsetX, int GraphOffsetY,
                       int LegendOffsetX, int LegendH, bool bLayout = true);

    void    Layout(); // perfroms layouts internal components accordingly to layout settings

    // Graph pane limits management function
    void    SetLimitsMode(ELimitsMode ModeX, ELimitsMode ModeY);
    void    SetFixedLimits(const TModelRect& rcLimits);
    void    UpdateLimits();

    // Setup functions
    void    SetIntegerMode(bool bIntegerX, bool bIntegerY);
    void    SetDrawGrid(bool bDraw);
    void    SetDrawAxes(bool bDraw);
    void    EnableAntiAliasing(bool bEn);

    void    SetBackColor(const CRgbaColor& Color, const CRgbaColor& GraphColor);

    void    AddGraph(IGraph* pGraph);
    void    RemoveAllGraphs();

    void    Render(IGraph::TElemVector* elems = NULL);

    // direct access to components
    CGlPane&    GetGraphPane()  {   return m_GraphPane; }
    CLegend*    GetLegend() {   return m_pLegend;   }
    CAxisArea*  GetAxisArea() { return m_pAxisArea; }

protected:
    void    PrepareContext();

    // factory methods
    virtual CAxisArea*  CreateAxisArea();
    virtual CLegend*    CreateLegend();

protected:
    typedef list<IGraph*> TGraphCont;

    CRegularGridGen m_Gen;
    CRegularGridRenderer m_Grid;

    CAxisArea*  m_pAxisArea;
    CLegend*    m_pLegend;

    CGlPane  m_GraphPane;
    CGlPane  m_Pane;
    CGlPane  m_LegendPane;

    TGraphCont  m_Graphs;

    TVPRect m_rcAll;
    int m_GraphOffsetX, m_GraphOffsetY;
    int m_LegendOffsetX, m_LegendH;

    ELimitsMode m_LimitsModeX, m_LimitsModeY;

    bool m_bDrawGrid;
    bool m_bDrawAxes;

    bool m_bAntiAliasing;

    CRgbaColor m_BackColor;
    CRgbaColor m_GraphBackColor;
};

END_NCBI_SCOPE

#endif  // GUI_GRAPH___GRAPH_PANEL__HPP
