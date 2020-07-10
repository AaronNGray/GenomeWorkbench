#ifndef __GUI_WIDGETS_HIT_MATRIX___HIT_MATRIX_GRAPH__HPP
#define __GUI_WIDGETS_HIT_MATRIX___HIT_MATRIX_GRAPH__HPP

/*  $Id: hit_matrix_graph.hpp 36917 2016-11-14 21:13:52Z katargir $
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

#include <gui/utils/rgba_color.hpp>
#include <gui/opengl/glpane.hpp>
#include <gui/opengl/glcolortable.hpp>

#include <gui/widgets/gl/ievent_handler.hpp>
#include <gui/widgets/hit_matrix/hit.hpp>
#include <gui/widgets/hit_matrix/hit_coloring.hpp>


BEGIN_NCBI_SCOPE


/// CHitElemGlyph is a simple graphical object representing a Hit Element.
///
class CHitElemGlyph
{
public:
    enum    ERenderingPass
    {
        eHitElemLines,
        eConnectionLines,
        eProjLines,
        eEndPoints,
        eOther
    };

public:
    CHitElemGlyph(const IHitElement& hit_elem);

    const IHitElement&  GetHitElem()  const {   return *m_HitElem;    }

    inline void     SetColorIndex(int index )       {   m_ColorIndex = index;   }
    inline int      GetColorIndex()  const   {   return m_ColorIndex;    }

    inline void     SetSelected(bool b_set)    {   m_bSelected = b_set;    }
    inline bool     IsSelected()    const   {   return m_bSelected; }

    void    GetModelRect(TModelRect& rc) const;

    void    Render(CGlPane& pane, ERenderingPass what);

    void    StartVertex();
    void    EndVertex();

    /// return shortest distance in pixels  from the given point in viewport
    /// to the graphical representation of the hit on the screen
    double  GetDistPixels(CGlPane& pane, const TVPPoint& pt) const;

    /// returns true if hit is contained by the given rectangle
    /// (in VP coordinates)
    bool    InRect(CGlPane& pane, const TVPRect& rc) const;

    /// returns true if hit is intersected by the given rectangle
    /// (in VP coordinates)
    bool    Intersects(CGlPane& pane, const TVPRect& rc) const;

protected:
   const IHitElement*  m_HitElem;
   int  m_ColorIndex;
   bool m_bSelected;
};


///////////////////////////////////////////////////////////////////////////////
/// CHitGlyph is a simple graphical object representing a Hit as a set of
/// corresponding Hit Elements.
///
class CHitGlyph
{
public:
    typedef     vector<CHitElemGlyph>   TElemGlyphCont;

    /// c'tor creates child CHitElemGlyph-s
    CHitGlyph(const IHit& p_hit);

    inline  const IHit&    GetHit() const  { return m_Hit; }
    TElemGlyphCont& GetElems();

    void     SetColorIndex(int index);

    void    Render(CGlPane& pane, CHitElemGlyph::ERenderingPass what);

public:
    TVPRect m_Bounds;

protected:
    const IHit& m_Hit;
    TElemGlyphCont  m_Elems;
};


///////////////////////////////////////////////////////////////////////////////
///  IHitMatrixGraphHost
class IHitMatrixGraphHost
{
public:
    virtual ~IHitMatrixGraphHost()  {};

    virtual void    HMGH_OnChanged(void) = 0;
    virtual TVPUnit HMGH_GetVPPosByY(int y) const = 0;
};


///////////////////////////////////////////////////////////////////////////////
///   CHitMatrixGraph
class CHitMatrixGraph :
        public wxEvtHandler,
        public IGlEventHandler
{
    DECLARE_EVENT_TABLE()
public:
    typedef vector<CHitGlyph*>      THitGlyphVector;
    typedef set<CHitElemGlyph*>     TElemGlyphSet;

    CHitMatrixGraph();
    ~CHitMatrixGraph();

    void    DrawConnections(bool draw);

    virtual void    DeleteGlyphs();
    virtual void    CreateGlyph(const IHit& hit_elem);

    const THitGlyphVector&  GetGlyphs() const   {   return m_vGlyphs;   }
    const TElemGlyphSet&    GetSelectedGlyphs() const   {   return m_SelGlyphs; }

    void    ResetGlyphSelection();
    void    SelectGlyph(const CHitGlyph& glyph);

    void    Render(CGlPane& pane);

    // set hit coloring params, provide NULL to disable coloring
    void    AssignColorsByScore(const SHitColoringParams* params);

    void    SetHost(IHitMatrixGraphHost* host);
    IGenericHandlerHost*    GetGenericHost();

    /// @name IGlEventHandler implementaion
    /// @{
    virtual void    SetPane(CGlPane* pane);
    virtual wxEvtHandler*   GetEvtHandler();
    /// @}

    inline const CRgbaColor&    GetColorByIndex(int index)
    {
        return index < 0 ? m_DefaultColor : m_ColorTable[index];
    }

    void    OnLeftDown(wxMouseEvent& event);
    void    OnLeftUp(wxMouseEvent& event);
    void    OnMotion(wxMouseEvent& event);
    void    OnKeyEvent(wxKeyEvent& event);
    void    OnMouseCaptureLost(wxMouseCaptureLostEvent& evt);

protected:
    void x_CancelSelRectMode();

    virtual void    x_RenderHits(CGlPane& pane);
    virtual void    x_RenderSelection(CGlPane& pane);
    virtual void    x_RenderPath(CGlPane& pane);
    virtual void    x_RenderEventHandler(CGlPane& pane);

    bool    x_IsColoringEnabled();
    //virtual void    x_RenderClustering(CGlPane& pane);

    /// event handling state
    enum    EState {
        eIdle,
        eSelPoint, /// selecting a single glyph by point
        eSelRect   /// selecting multiple glyphs by rectangle
    };

    void    x_OnSelectCursor(void);
    bool    x_SelectTest() const;
    bool    x_SelectByPoint(bool b_inc, bool toggle);
    void    x_SelectByRect(bool inc, bool neg);

protected:
    typedef list<CHitElemGlyph*>    TPath;
    typedef list<TPath>             TPathCont;

    THitGlyphVector     m_vGlyphs; /// primary storage for glyphs
    TElemGlyphSet       m_SelGlyphs; /// set of selected Glyphs (hit elements)
    TPathCont           m_Paths;

    /// rendering state
    int m_CurrColorIndex;

    const SHitColoringParams* m_ColoringParams;
    CRgbaGradColorTable   m_ColorTable;

    CRgbaColor            m_DefaultColor;
    CRgbaColor            m_SelColor;
    CRgbaColor            m_HighLightColor;
    CRgbaColor            m_PathColor;
    CRgbaColor            m_ProjLinesColor;
    CRgbaColor            m_ProjBackColor;

    bool    m_DrawConnections;

    /// event handling
    IHitMatrixGraphHost *m_Host;
    CGlPane*    m_Pane;
    EState      m_State;
    wxPoint m_StartPoint;
    wxPoint m_DragPoint;
    wxStockCursor m_CursorId;
    bool  m_IncSelection;
    bool  m_ToggleSelection;
    bool  m_MoveDuringSelection;

    //CHitClustering  m_Clustering;
};


END_NCBI_SCOPE

#endif  // __GUI_WIDGETS_HIT_MATRIX___HIT_MATRIX_GRAPH__HPP
