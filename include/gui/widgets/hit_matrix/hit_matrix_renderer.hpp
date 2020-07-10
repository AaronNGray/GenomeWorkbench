#ifndef GUI_WIDGETS_HIT_MATRIX___HIT_MATRIX_RENDERER__HPP
#define GUI_WIDGETS_HIT_MATRIX___HIT_MATRIX_RENDERER__HPP

/*  $Id: hit_matrix_renderer.hpp 33041 2015-05-18 21:38:32Z shkeda $
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

#include <corelib/ncbistl.hpp>

#include <gui/objutils/objects.hpp>

#include <gui/graph/axis.hpp>
#include <gui/widgets/gl/ruler.hpp>
#include <gui/widgets/gl/graph_container.hpp>


#include <gui/widgets/hit_matrix/hit_matrix_graph.hpp>
#include <gui/widgets/hit_matrix/seq_graph.hpp>
#include <gui/widgets/hit_matrix/hit_coloring.hpp>

#include <objects/seqfeat/SeqFeatData.hpp>

BEGIN_NCBI_SCOPE

class CHistogramGraph;
class IHitMatrixDataSource;

////////////////////////////////////////////////////////////////////////////////
/// CHitMatrixRenderer - renders Hit Matrix Graph, Rulers, axes, grid,
/// sequence and features graphs.

class NCBI_GUIWIDGETS_HIT_MATRIX_EXPORT CHitMatrixRenderer

{
public:
    typedef CHitMatrixGraph::TElemGlyphSet  TElemGlyphSet;

    /// identifies one of the two sequences in the pane
    enum    ESequence {
        eSubject,
        eQuery
    };

    enum EArea   {
        fMatrixArea     = 0x01,
        fSubjectRuler   = 0x02,
        fQueryRuler     = 0x04,
        fSubjectGraphs  = 0x08,
        fQueryGraphs    = 0x10,
        fOther = 0x80000000,

        fAllAreas = fMatrixArea | fSubjectRuler | fQueryRuler | fSubjectGraphs | fQueryGraphs | fOther
    };

    CHitMatrixRenderer();
    virtual ~CHitMatrixRenderer();

    virtual void    ShowRulers(bool show);
    virtual void    ShowGrid(bool show);
    virtual void    SetBackgroundColor(CRgbaColor& color);

    CGlPane&    GetMatrixPane();
    CGlPane&    GetSubjectPane();
    CGlPane&    GetQueryPane();
    CRuler&     GetBottomRuler()    {   return m_BottomRuler;   }
    CRuler&     GetLeftRuler()      {   return m_LeftRuler;   }

    TVPRect GetRect(EArea area);

    CHitMatrixGraph& GetGraph() {   return m_Graph; }
    const CHitMatrixGraph& GetGraph() const {   return m_Graph; }

    virtual void    Resize(int w, int h, const CGlPane& port);
    virtual void    Render(const CGlPane& port, bool pdf = false);

    virtual void    Update(IHitMatrixDataSource* ds, const CGlPane& port);

    void    ColorByScore(const SHitColoringParams* params);
    const string&   GetScoreName() const;

    TModelRect  GetSelectedHitElemsRect() const;

    /// @name Graphs management
    /// @{
    struct SGraphDescr
    {
        string      m_Type;
        bool        m_HasColor;
        CRgbaColor  m_Color;
    };
    void    GetGraphTypes(vector<SGraphDescr>& types) const;
    void    SetGraphColor(const string& name, const CRgbaColor& color);

    void    GetGraphs(ESequence seq, vector<string>& graphs) const;
    void    SetGraphs(ESequence seq, const vector<string>& graphs);
    /// @}

    virtual int     GetAreaByVPPos(TVPUnit vp_x, TVPUnit vp_y);

    virtual bool    NeedTooltip(int area, int vp_x, int vp_y);
    virtual string  GetTooltip(int area);

    void    Layout(const CGlPane& pane);

protected:
    void    x_SetupAxes();

    void    x_SetupGraphTypes();
    CHistogramGraph*    x_CreateHistogram(ESequence seq, const SGraphDescr& descr);
    void    x_AddGraph(CGraphContainer& cont, IRenderable* graph, bool horz);
    void    x_SetGraphs(ESequence seq, const vector<string>& graphs);
    void    x_UpdateGraphColor(IRenderable* graph);
    void    x_DestroyGraphs(CGraphContainer& cont);

    void    x_SetRulerText(IHitMatrixDataSource* ds);

    // Rendering functions
    void    x_RenderAxisAndGrid(const CGlPane& port, CGlPane& gr_pane);
    void    x_RenderContainers(const CGlPane& port);

    /// adjusts limits and visible rect of the given pane to the master pane
    void    x_AdjsutToMasterPane(const CGlPane& port, CGlPane& pane, bool b_model_x, bool b_model_y);

protected:
    TVPPoint m_Size;
    IHitMatrixDataSource* m_DataSource;

    bool    m_ShowRulers;
    bool    m_ShowGrid;

    CGlPane m_MatrixPane;
    CGlPane m_SubjectPane;
    CGlPane m_QueryPane;

    // layout information
    TVPRect m_rcMatrix;
    TVPRect m_rcAxes;
    TVPRect m_rcLeftRuler;
    TVPRect m_rcBottomRuler;
    TVPRect m_rcBottomContainer;
    TVPRect m_rcLeftContainer;

    // graphical objects
    CRegularGridGen         m_Gen;
    CRegularGridRenderer    m_Grid;

    CRuler  m_BottomRuler;
    CRuler  m_TopRuler;
    CRuler  m_LeftRuler;
    CRuler  m_RightRuler;

    CHitMatrixGraph     m_Graph;
    CHitMatrixSeqGraph*     m_QueryGraph;
    CHitMatrixSeqGraph*     m_SubjectGraph;

    typedef vector<SGraphDescr> TGraphTypes;
    TGraphTypes     m_GraphTypes;

    CGraphContainer     m_BottomContainer;
    CGraphContainer     m_LeftContainer;

    CRgbaColor m_BackColor;
    CRef<CGlTextureFont> m_TextFont;
    CRef<CGlTextureFont> m_SeqFont;

    const SHitColoringParams*  m_HitColoringParams;
};


END_NCBI_SCOPE

#endif  // GUI_WIDGETS_HIT_MATRIX___HIT_MATRIX_RENDERER__HPP
