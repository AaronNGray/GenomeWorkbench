#ifndef __GUI_WIDGETS_PHYLO_TREE___PHYLO_TREE_PANE__HPP
#define __GUI_WIDGETS_PHYLO_TREE___PHYLO_TREE_PANE__HPP

/*  $Id: phylo_tree_pane.hpp 43428 2019-06-28 14:12:33Z katargir $
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
 * Authors:  Vladimir Tereshkov
 *
 * File Description:
 *
 */

#include <gui/graph/axis.hpp>
#include <gui/widgets/gl/gl_widget_base.hpp>
#include <gui/widgets/gl/mouse_zoom_handler.hpp>
#include <gui/widgets/gl/linear_sel_handler.hpp>
#include <gui/widgets/gl/minimap_handler.hpp>
#include <gui/opengl/gltexturefont.hpp>
#include <gui/widgets/gl/ruler.hpp>
#include <gui/widgets/gl/tooltip_handler.hpp>
#include <gui/widgets/wx/sticky_tooltip_handler.hpp>
#include <gui/print/pdf.hpp>

#include <gui/widgets/phylo_tree/phylo_tree_render.hpp>

BEGIN_NCBI_SCOPE

class CPhyloTreeWidget;

////////////////////////////////////////////////////////////////////////////////
/// class CPhyloTreePane
class NCBI_GUIWIDGETS_PHYLO_TREE_EXPORT  CPhyloTreePane
                        : public CGlWidgetPane,
                          public CEventHandler,
                          public IMouseZoomHandlerHost,
                          public IMinimapHandlerHost,
                          public ISelHandlerHost,
                          public IPhyloTreeRenderHost
                        
{
    DECLARE_EVENT_TABLE()
public:
    typedef CLinearSelHandler::TRangeColl   TRangeColl;
    typedef vector<CRef<IPhyloTreeRender>>  TRenderers;

    //CPhyloTreePane(int PosX, int PosY, int Width, int Height, const char* Label = NULL);
    CPhyloTreePane(CPhyloTreeWidget* parent);

    virtual TVPPoint GetPortSize(void);

    TRenderers& GetRenderers(void)
    {
        return m_Renderers;
    }

    int GetCurrRendererIdx(void)
    {
        return m_CurrRenderer;
    }

    IPhyloTreeRender* GetCurrRenderer(void)
    {
        return m_Renderers[m_CurrRenderer].GetPointer();
    }

    CGlPane& GetPane() { return m_MatrixPane; }
    void UpdatePane(const TModelRect& r);

    void    EditCurrentNode();
    void    SetCurrRendererIdx(int idx);
    void    AddRenderer(CRef<IPhyloTreeRender> &&renderer);

    void    SetupHardcopyRender();
    void    RenderHardcopy();
    void    RenderPdf(CRef<CPdf> pdf, bool render_tooltips);
    void    BufferedRender();

    void    RemoveCurrentDataSource();
    void    UpdateDataSource();
    void    OnSize(wxSizeEvent& event);
    void    OnLeftDown(wxMouseEvent& evt);
    // uncomment for degugging (post-key-event actions)
    // void    OnKeyEvent(wxKeyEvent& event);

    // interface for a Widget
    virtual void    SetContext(void) { x_SetupGLContext(); }
    virtual void    Update(void);
    virtual void    SoftUpdate(void);

    const   TRangeColl&     GetSubjectSelection() const;
    const   TRangeColl&     GetQuerySelection() const;

    /// Record current zoom/pan so that user can undo/redo navigation
    void SaveCurrentView();
    /// Return true if there are previous view positions you can nav back to
    bool CanGoBack() const;
    /// Return true if there are saved view positions you can nav forward to
    bool CanGoForward() const;
    /// move to view position prior to most recent zoom/pan
    void GoBack();
    /// return to view position that you just left through 'GoBack()'
    void GoForward();

    /// @name IMouseZoomHandlerHost implementation
    /// @{
    virtual TModelUnit  MZHH_GetScale(IMouseZoomHandlerHost::EScaleType type);
    virtual void        MZHH_SetScale(TModelUnit scale, const TModelPoint& point);
    virtual void        MZHH_ZoomRect(const TModelRect& rc);
    virtual void        MZHH_ZoomPoint(const TModelPoint& point, TModelUnit factor);
    virtual void        MZHH_Scroll(TModelUnit d_x, TModelUnit d_y);
    virtual void        MZHH_EndOp();
    virtual TVPUnit     MZHH_GetVPPosByY(int y) const;
    /// @}

    /// @name IMinimapHandlerHost implementation
    /// @{
    virtual TModelUnit  MMHH_GetScale(IMinimapHandlerHost::EMMScaleType type);
    virtual void        MMHH_Scroll(TModelUnit d_x, TModelUnit d_y);
    virtual void        MMHH_EndOp();
    virtual TVPUnit     MMHH_GetVPPosByY(int y) const;
    virtual I3DTexture* MMHH_GetTexture(float& xcoord_limit,
                                        float& ycoord_limit);
    /// @}

    virtual void     HMGH_OnChanged(void);
    virtual void     HMGH_OnRefresh(void);
    virtual void     HMGH_UpdateLimits(bool force_redraw = false);
    virtual TVPUnit  HMGH_GetVPPosByY(int y) const;
    
    virtual void     FireCBEvent(void);
    virtual void     FireEditEvent(EPhyloTreeEditCommand ec);

    /// @name ISelHandlerHost implementation
    /// @{
    virtual void        SHH_OnChanged();
    virtual TModelUnit  SHH_GetModelByWindow(int z, EOrientation orient);
    virtual TVPUnit     SHH_GetWindowByModel(TModelUnit z, EOrientation orient);
    virtual void        SHH_SetCursor(const wxCursor& cursor);
    /// @}


    /// @name ITooltipClient implementation
    /// @{
    virtual string  TTHH_NeedTooltip(const wxPoint & pt);
    virtual CTooltipInfo  TTHH_GetTooltip(const wxRect & rect);
    /// @}

protected:
    int         m_CurrRenderer;
    TRenderers  m_Renderers;

    void    x_SetupAxes();

    /// CGlWidgetPane overridables
    virtual void    x_Render(void);
    virtual void    x_OnShowPopup();
    //virtual int     x_HandleMouseRelease();

    virtual bool    x_GrabFocus();

    CPhyloTreeWidget* x_GetWidget()  const;

    void    x_Layout(void);

    // Rendering functions
    void    x_RenderMouseZoomHandler(CGlPane& pane);

    /// adjusts limits and visible rect of the given pane to the master pane
    void    x_AdjustToMasterPane(CGlPane& pane, bool b_model_x, bool b_model_y);

    virtual int     x_GetAreaByVPPos(TVPUnit vp_x, TVPUnit vp_y);

protected:
    enum EHandlerArea   {
        fMatrixArea =   0x1,
        fSubjectArea =  0x2,
        fQueryArea =    0x4,
        fOther = 0x80000000,
        fAllAreas = fMatrixArea | fSubjectArea | fQueryArea | fOther
    };

    CGlPane m_MatrixPane;
  
    /// @name navigation history
    /// @{
    struct TreeNavHistory {
        TreeNavHistory() : m_BackForwardPos(0) {}
        TreeNavHistory(const CGlPane& p) : m_BackForwardPos(0) 
            { m_History.push_back(p); }

        CGlPane Current() const;
        bool Empty() const { return m_History.size() == 0; }
        void Add(const CGlPane& p);
        bool EqualPanes(const CGlPane& lhs, const CGlPane& rhs) const;
        bool CanGoBack() const { return m_BackForwardPos>0; }
        bool CanGoForward() const { return m_BackForwardPos < m_History.size()-1; }
        CGlPane Back(){ return m_History[--m_BackForwardPos]; }
        CGlPane Forward(){ return m_History[++m_BackForwardPos]; }

        void Reset(const CGlPane& p);

        std::vector<CGlPane>  m_History;
        size_t m_BackForwardPos;        ///< position in history
    };
    TreeNavHistory m_NavHistory;
    /// @}

    /// rectangle in OpenGL coordinates

    TVPRect m_rcMatrix;
    TVPRect m_rcLeftSeq;
    TVPRect m_rcBottomSeq;
    TVPRect m_rcAxes;

    CRgbaColor m_BackColor;
    CGlTextureFont   *m_pTextFont;
    CGlTextureFont   *m_pLblFont;

    // event handlers
    CMouseZoomHandler       m_MouseZoomHandler;
    CMinimapHandler         m_MinimapHandler;
    CLinearSelHandler       m_HorzSelHandler;
    CLinearSelHandler       m_VertSelHandler;
    CTooltipHandler         m_TooltipHandler;
    CStickyTooltipHandler   m_TooltipManager;

    // graphical objects
    CRegularGridGen         m_Gen;
    CRegularGridRenderer    m_Grid;

    CRuler      m_BottomRuler;
    CRuler      m_TopRuler;
    CRuler      m_LeftRuler;
    CRuler      m_RightRuler;
};

END_NCBI_SCOPE

#endif  // __GUI_WIDGETS_PHYLO_TREE___PHYLO_TREE_PANE__HPP
