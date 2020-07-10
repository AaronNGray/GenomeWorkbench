#ifndef __GUI_WIDGETS_ALNMULTI___ALNMULTI_PANE__HPP
#define __GUI_WIDGETS_ALNMULTI___ALNMULTI_PANE__HPP

/*  $Id: alnmulti_pane.hpp 44930 2020-04-21 17:07:30Z evgeniev $
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



#include <gui/widgets/aln_multiple/alnmulti_renderer.hpp>
#include <gui/widgets/aln_multiple/alnmulti_header_handler.hpp>
#include <gui/widgets/aln_multiple/alnmulti_base_pane.hpp>
#include <gui/widgets/aln_multiple/list_mvc.hpp>
#include <gui/widgets/aln_multiple/sel_list_controller.hpp>
#include <gui/widgets/aln_multiple/align_mark_handler.hpp>

#include <gui/widgets/gl/gl_widget_base.hpp>
#include <gui/widgets/gl/ruler.hpp>
#include <gui/widgets/gl/mouse_zoom_handler.hpp>
#include <gui/widgets/gl/linear_sel_handler.hpp>
#include <gui/widgets/gl/progress_panel.hpp>
#include <gui/widgets/wx/image_grabber.hpp>


BEGIN_NCBI_SCOPE

typedef ISelListView<IAlnMultiDataSource::TNumrow>  TNumrowSelListView;
typedef CSelListController<IAlnMultiDataSource::TNumrow>    TNumrowSelListController;

BEGIN_SCOPE(objects)
    class CScope;
END_SCOPE(objects)

////////////////////////////////////////////////////////////////////////////////
/// class CAlnMultiPane
class NCBI_GUIWIDGETS_ALNMULTIPLE_EXPORT CAlnMultiPane
    : public CGlWidgetPane,
      public CEventHandler,
      public TNumrowSelListController,
      public TNumrowSelListView,
      public ISelHandlerHost,
      public IAlignMarkHandlerHost,
      public IMouseZoomHandlerHost,
      public IAlignRowHost,
      public IAMHeaderHandlerHost,
      public IVectorGraphicsRenderer
{
    DECLARE_EVENT_TABLE()
public:
    typedef CLinearSelHandler::TRangeColl TRangeColl;
    typedef IAlignRow::TColumnType      TColumnType;
    typedef map<TNumrow, TRangeColl>    TRowToMarkMap;
    typedef ISelListModel<TNumrow>      TSelListModel;
    typedef CAlnMultiRenderer           TRenderer;

    enum EEventType {
        eUnknown,
        eSelRowChanged,
        eLinearSelChanged
    };

    class CEvent : public ncbi::CEvent
    {
    public:
        CEvent(EEventType type)  : ncbi::CEvent(eEvent_Message, type)    {}
    };

public:
    CAlnMultiPane(wxWindow* parent,
                  wxWindowID id,
                  const wxPoint& pos = wxDefaultPosition,
                  const wxSize& size = wxDefaultSize,
                  long style = wxWANTS_CHARS);
    ~CAlnMultiPane();

    virtual TVPPoint GetPortSize(void) override;

    void    Init(objects::CScope* scope);
    void    SetWidget(IAlnMultiPaneParent* p_arent);
    void    SetContext(IAlnMultiRendererContext* p_context);

    void    SetBackColor(const CRgbaColor& color);
    void    SetMasterBackColor(const CRgbaColor& color);

    void    OnSize(wxSizeEvent& event);

    /// @name Interface for a Widget
    /// @{
    int     GetAlignVPHeight() const;
    int     GetAlignVPWidth() const;

    virtual void    Update() override; // updates GUI assuming that data did not change
    virtual void    UpdateOnDataChanged(); // full update

    CRuler&    GetRuler();

    CAlnMultiRenderer&  GetRenderer();

    CRange<TSeqPos>     GetSelectionLimits();
    const TRangeColl&   GetSelection() const;
    void    SetSelection(const TRangeColl& C, bool redraw);
    void    ResetSelection(bool redraw);

    void    MarkSelectedRows(const TRangeColl& C, bool bMark);
    void    UnMarkAll();
    const   TRowToMarkMap&  GetMarks() const;

    void    SortByColumn(int index);
    void    UpdateHeaderSortState();

    void    ShowProgressPanel(float progress, const string& msg);
    void    HideProgressPanel();
    /// @}

    /// @name CSelListController extension
    /// @{
    virtual void    SetPane(CGlPane* pane) override;
    virtual wxEvtHandler*    GetEvtHandler() override;

    virtual TSelListModel* SLC_GetModel() override;

    virtual int     SLC_GetHeight() override;
    virtual int     SLC_GetLineByWindowY(int WinY, bool b_clip = false) override;
    virtual void    SLC_VertScrollToMakeVisible(int Index) override;
    /// @}

    /// @name TNumrowSelListView implementation
    /// @{
    virtual void    SLV_SetModel(TSelListModel* pModel) override;
    virtual void    SLV_UpdateItems(const TIndexVector& vIndices) override;
    virtual void    SLV_UpdateRange(int iFirstItem, int iLastItem) override;
    /// @}

    /// @name ISelHandlerHost implementation
    /// @{
    virtual void    SHH_OnChanged() override;
    virtual TModelUnit  SHH_GetModelByWindow(int z, EOrientation orient) override;
    virtual TVPUnit     SHH_GetWindowByModel(TModelUnit z, EOrientation orient) override;
    /// @}

    /// @name IAlnMarkHandlerHost implementation
    /// @{
    virtual const IAlnMultiDataSource*      MHH_GetAlnDS() const override;
    virtual const TSelListModel*    MHH_GetSelListModel() const override;

    virtual TNumrow    MHH_GetRowByLine(int Index) const override;
    virtual int MHH_GetLineByRowNum(TNumrow Row) const override;

    virtual int MHH_GetLineByWindowY(int Y) const override;
    virtual int MHH_GetLinePosY(int Index) const override;
    virtual int MHH_GetLineHeight(int Index) const override;

    virtual TModelUnit  MHH_GetSeqPosByX(int X) const override;
    /// @}

    /// @name IMouseZoomHandlerHost implementation
    /// @{
    virtual TModelUnit  MZHH_GetScale(EScaleType type) override;
    virtual void        MZHH_SetScale(TModelUnit scale, const TModelPoint& point) override;
    virtual void        MZHH_ZoomRect(const TModelRect& rc) override;
    virtual void        MZHH_ZoomPoint(const TModelPoint& point, TModelUnit factor) override;
    virtual void        MZHH_Scroll(TModelUnit d_x, TModelUnit d_y) override;
    virtual void        MZHH_EndOp() override;
    virtual TVPUnit     MZHH_GetVPPosByY(int y) const override;
    /// @}

    /// @name IAlignRowHost implementation
    /// @{
    virtual void               ARH_OnRowChanged(IAlignRow* p_row) override;
    virtual TVPPoint           ARH_GetVPPosByWindowPos(const wxPoint& pos) override;
    virtual void               ARH_OnSelectionChanged() override;
    virtual CSelectionVisitor* ARH_GetSelectionVisitor() override;
    virtual int                ARH_GetRowTop(IAlignRow* p_row) override;

    /// @}

    /// @name IAMHeaderHandlerHost implementation
    /// @{
    virtual IAlnMultiHeaderContext* HHH_GetContext() override;
    virtual TVPRect HHH_GetHeaderRect() override;
    virtual TVPPoint    HHH_GetVPPosByWindowPos(const wxPoint& pos) override;
    virtual void    HHH_SetColumns(const TColumns& columns, int resizable_index) override;
    virtual void    HHH_SortByColumn(int index) override;
    virtual void    HHH_RenderColumnHeader(int index, const TVPRect& rc) override;
    /// @}

    /// @name IGraphContainerHost implementation
    /// @{
    virtual TVPPoint GCH_GetVPPosByWindowPos(const wxPoint& pos);
    /// @}

    /// @name ITooltipClient implementation
    /// @{
    //virtual bool    TC_NeedTooltip(const wxPoint &);
    //virtual string  TC_GetTooltip(const wxRect &);
    /// @}

    /// @name IStickyTooltipHandlerHost implementation
    /// @{
    virtual string  TTHH_NeedTooltip(const wxPoint& pt) override;
    virtual CTooltipInfo  TTHH_GetTooltip(const wxRect& rect) override;
    virtual bool TTHH_ProcessURL(const wxString &href) override;
    /// @}

    /// @name IVectorGraphicsRenderer implementations
    /// @{
    virtual const TVPRect& GetViewportRect() const;
    /// Render for vector output.
    virtual void RenderVectorGraphics(int vp_width, int vp_height);
    virtual void UpdateVectorLayout();
    virtual void AddTitle(bool b) { m_Title = b; }
    /// @)

    int     GetListAreaHeight() const   {   return m_Renderer.GetListAreaHeight();    }

    void    NotifyParent(EEventType type);

    CAlignMarkHandler& GetMarkHandler() {   return m_MarkHandler;   }

    bool    HasSelectedObjects() const;
    void    GetObjectSelection(TConstObjects &objs) const;

    void    OnLeftDown(wxMouseEvent& event);
    void    OnLeftUp(wxMouseEvent& event);
    void    OnMotion(wxMouseEvent& event);
    void    OnLeftDoubleClick(wxMouseEvent& event);
    void    OnMouseWheel(wxMouseEvent& event);
    void    OnKeyDown(wxKeyEvent& event);

    //virtual bool    Layout();
protected:
    /// @name CGlWidgetChild functions
    /// @{
    virtual void    x_Render() override;
    int     x_GetAreaByWindowPos(const wxPoint& pos) override;
    /// @}

    virtual void    x_RenderSelection();
    virtual void    x_RenderMark();
    virtual void    x_RenderZoomHandler();
    virtual void    x_RenderHeaderHandler();
    virtual void    x_RenderProgressPanel();

    virtual void    x_OnRowMouseEvent(wxMouseEvent& event);
    /// @}

    TVPPoint    x_GetVPPosByWindowPos(int x, int y);
    int     x_GetLineByWindowY(int WinY) const;
    bool    x_IsMasterLineByWindowY(int WinY);
    TVPRect x_GetLineRect(int Index) const;

    virtual int     x_GetAreaByVPPos(TVPUnit vp_x, TVPUnit vp_y) override;

    IAlnMultiPaneParent* x_GetParent()    {   return m_Parent;   }
    const IAlnMultiPaneParent* x_GetParent()  const    {   return m_Parent;   }

    IAlnMultiRendererContext* x_GetContext()    {   return m_Context;   }
    const IAlnMultiRendererContext* x_GetContext()  const    {   return m_Context;   }

    inline TSelListModel*  x_GetModel()   {   return m_Model;    }
    inline const TSelListModel*  x_GetModel()   const {   return m_Model;    }

protected:
    enum EHandlerArea   {
        fDescrArea  = 0x01,
        fIconsArea  = 0x02,
        fAlignArea  = 0x04,
        fHeaderArea = 0x08,
        fRulerArea  = 0x10,
        fOtherAreas = 0x20,
        fAllAreas   = fDescrArea | fIconsArea | fAlignArea | fHeaderArea | fRulerArea | fOtherAreas
    };
    typedef list<SHandlerRec>    THandlerRecList;

    IAlnMultiPaneParent*         m_Parent;
    IAlnMultiRendererContext*    m_Context;
    TSelListModel*  m_Model;

    CGlPane     m_HandlerPane;

    CLinearSelHandler   m_SelHandler;
    CAlignMarkHandler   m_MarkHandler;
    CMouseZoomHandler   m_MouseZoomHandler;

    CAlnMultiRenderer       m_Renderer;
    CAlnMultiHeaderHandler  m_HeaderHandler;
    //CTooltipHandler     m_TooltipHandler;
    CStickyTooltipHandler   m_TooltipManager;
    CSelectionVisitor m_SelectionVisitor;
    CProgressPanel  m_ProgressPanel;

    /// When rendering vector image, add a title
    bool        m_Title;
};


END_NCBI_SCOPE

#endif  // __GUI_WIDGETS_ALNMULTI___ALNMULTI_PANE__HPP
