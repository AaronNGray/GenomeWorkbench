#ifndef GUI_WIDGETS_SEQ_GRAPHIC___SEQGRAPHIC_PANE__HPP
#define GUI_WIDGETS_SEQ_GRAPHIC___SEQGRAPHIC_PANE__HPP

/*  $Id: seqgraphic_pane.hpp 44930 2020-04-21 17:07:30Z evgeniev $
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
 * Authors:  Vlad Lebedev, Liangshou Wu
 *
 * File Description:
 *   Pane for the Graphic Sequence Widget
 *   (see <gui/widgets/seq_graphic/seqgraphic_widget.hpp>)
 *
 */

#include <gui/widgets/seq_graphic/seqgraphic_render.hpp>
#include <gui/widgets/seq_graphic/seqgraphic_conf.hpp>
#include <gui/widgets/seq_graphic/layout_track_handler.hpp>

#include <gui/gui.hpp>
#include <gui/opengl.h>

#include <gui/widgets/gl/gl_widget_base.hpp>
#include <gui/widgets/gl/linear_sel_handler.hpp>
#include <gui/widgets/gl/mouse_zoom_handler.hpp>
#include <gui/widgets/gl/seqmark_handler.hpp>
//#include <gui/widgets/gl/tooltip_handler.hpp>
#include <gui/widgets/gl/gl_widget_base.hpp>
#include <gui/widgets/gl/attrib_menu.hpp>
#include <gui/utils/iclipboard_handler.hpp>
#include <gui/widgets/wx/image_grabber.hpp>

#include <objmgr/seq_vector.hpp>

BEGIN_NCBI_SCOPE


class CSeqGraphicWidget;

class CCachedTipHandle : public CObject
{
public:
    CCachedTipHandle (const string& t_name, objects::CScope& scope)
        : m_ParentTrackName(t_name)
    {
        m_ObjIndex.Clear(&scope);
    }

    void Add(CObject& obj) {
        m_ObjIndex.Add(NULL, obj);
    }

    const string& GetParentTrackName() const { return m_ParentTrackName; }
    bool HasMatches(const CObject& object, objects::CScope& scope) const
    { return m_ObjIndex.HasMatches(object, scope); }

private:
    CObjectIndex m_ObjIndex;
    string       m_ParentTrackName;
};


/// Help class to manage markers.
/// It is intended to manage (rename/remove) marker easier.
class CMarker : public CObject
{
public:
    CMarker() : m_Deleted(false) {}

    void    Render(CGlPane& pane, CSeqMarkHandler::ERenderingOption option)
    { m_Handler.Render(pane, option); }

    void  SetLabel(const string& label)
    { m_Handler.SetLabel(label); }

    const string& GetLabel() const
    { return m_Handler.GetLabel(); }

    void  SetPos(TModelUnit pos)
    { m_Handler.SetPos(pos); }

    TSeqPos  GetPos() const
    { return m_Handler.GetPos(); }

    void  SetExtendedPos(TModelUnit pos)
    { m_Handler.SetExtendedPos(pos); }

    void SetRange(TSeqRange range)
    { m_Handler.SetRange(range); }

    TSeqPos  GetExtendedPos() const
    { return m_Handler.GetExtendedPos(); }

    void  SetColor(const CRgbaColor& color)
    { m_Handler.SetColor(color); }

    const CRgbaColor& GetColor() const
    { return m_Handler.GetColor(); }

    void SetRemoved(bool f)
    { m_Deleted = f; }

    bool IsRemoved() const
    { return m_Deleted; }

    CSeqMarkHandler::EMarkerType GetMarkerType() const 
    { return m_Handler.GetMarkerType(); }

    void SetMarkerType(CSeqMarkHandler::EMarkerType t) 
    { m_Handler.SetMarkerType(t); }

    void SetFlipped(bool f)
    { m_Handler.SetFlipped(f); }

    bool GetFlipped() const
    { return  m_Handler.GetFlipped(); }

public:
    bool            m_Deleted;
    CSeqMarkHandler m_Handler;
};

////////////////////////////////////////////////////////////////////////////////
/// class CSeqGraphicPane
class CSeqGraphicPane : public CGlWidgetPane,
                        public IMouseZoomHandlerHost,   ///< for zooming with mouse
                        public ISelHandlerHost,         ///< for linear selection
                        public ISeqMarkHandlerHost,     ///< for seq bar hairpin
                        public ITrackHandlerHost,       ///< for layout track interaction
                        public CEventHandler,
                        public ILayoutTrackHost,
                        public IGlyphDialogHost,
                        public IVectorGraphicsRenderer
                        //public IClipboardHandler,
{
    DECLARE_EVENT_TABLE()
public:
    enum EHandlerArea   {
        fArea_Ruler     = 1 << 0,
        fArea_Object    = 1 << 1,
        fArea_Alignment = 1 << 2,
        fArea_Feature   = 1 << 3,
        fArea_Temp_Track =1 << 4,
        fArea_Track     = 1 << 5,
        fArea_Container = 1 << 6,
        fArea_Other     = 1 << 7,

        fArea_All = fArea_Ruler | fArea_Object | fArea_Other |
                    fArea_Alignment | fArea_Feature
    };

    typedef CLinearSelHandler::TRangeColl TRangeColl;
    typedef map< string, CRef<CMarker> > TSeqMarkers;

    // Feature navigation
    enum EDirection {
        eDir_Next,
        eDir_Prev,
        eDir_Left,
        eDir_Right
    };

    enum EMouseMode {
        eMouse_RectSelect,     ///< with SHIFT key down
        eMouse_IncRectSelect,  ///< with SHIFT and CTRL key down
        eMouse_IncSelect,      ///< with CTRL key down
        eMouse_Pan,  ///< left click and drag 
        eMouse_Down, ///< left button down, not sure what to do yet
        eMouse_Idle  ///< no button down
    };

    CSeqGraphicPane(CSeqGraphicWidget* parent);
    //CSeqGraphicPane(int x, int y, int w, int h);
    virtual ~CSeqGraphicPane();

    void SetInputObject(SConstScopedObject& obj);
    void InitDataSource(SConstScopedObject& obj);
    CSGSequenceDS* GetDataSource(void);
    CFeaturePanel* GetFeaturePanel();

    TModelUnit GetHeight();

    //void SetWidget(CSeqGraphicWidget* pParent);
    //void Resize(const TVPRect& rc);

    void SetExternalGlyphs(const CSeqGlyph::TObjects& objs);

    void GetFeatureNames(vector<string>& names);

    /* TODO: there is no implementation for the following functions
    void ShowFeature(objects::CSeqFeatData::E_Choice t,
                     objects::CSeqFeatData::ESubtype s, bool flag);
    bool IsFeatureShown(objects::CSeqFeatData::E_Choice t,
                        objects::CSeqFeatData::ESubtype s);

    TModelUnit GetZoomX(void) const;
    */

    bool CanSetSwitchPoint();

    void OnKeyUp(wxKeyEvent& event);
    void OnKeyDown(wxKeyEvent& event);
    void OnMotion(wxMouseEvent& event);
    void OnLeftUp(wxMouseEvent& event);
    void OnLeftDown(wxMouseEvent& event);
    void OnMiddleDown(wxMouseEvent& event);
    void OnLeftDblClick(wxMouseEvent& event);
    void OnMouseWheel(wxMouseEvent& event);
    void OnContextMenu(wxContextMenuEvent& event);

    void OnScrollEnd(wxScrollEvent& event);
    void OnMouseCaptureLost(wxMouseCaptureLostEvent& /*event*/);

    void OnKillFocus(wxFocusEvent& event);

    int GetPopupArea(wxPoint pos);

    CRef<CSeqGraphicConfig> GetConfig();

    /// Potential actions include loading data and generating layouts.
    void AdjustViewPort();
    void SetUpdateContent();
    void UpdateData(TSeqPos from, TSeqPos to);
    void AdjustViewPortHeightToImage();
    void SetViewportWidth(TSeqPos from, TSeqPos to, TVPUnit vp_width);

    bool AllJobsFinished() const; 
    bool AllTracksCreated() const;

    void SetHorizontal(bool b_horz, bool b_flip);
    bool IsHorizontal() const;
    bool IsFlipped() const;

    void SetSeqMarker(TModelPoint point);
    void RemoveMarker(const string& marker_id);
    size_t GetMarkerNum() const;
    const TSeqMarkers& GetAllMarkers() const;
    void AddPointMarker(const string& name, TSeqPos pos, const CRgbaColor color);
    void AddRangeMarker(const string& name, TSeqRange range, const CRgbaColor color);

    void RemoveAllMarkers();
    string GetNewMarkerName();
    void RenameMarker(const string& id, const string& label);
    void ModifyMarker(const string& id, const string& label,
        TSeqPos pos, const CRgbaColor color);
    void ModifyMarker(const string& id, const string& label,
        TSeqRange range, const CRgbaColor color);

    void SetSeqStartPoint(TModelPoint point);
    void SetSeqStart(TSeqPos pos);
    void SetSeqStartMarker(const string& id);
    void ResetSeqStart();
    void SetSwitchPoint();

    string GetHitMarker() const;
    CConstRef<CMarker> GetMarker(const string& id);

    /// @name interface from wxWindows.
    /// @{
    virtual void Update(void);
    /// @}

    /// the config object just changed. Do what is necessary.
    void UpdateConfig();
    void OnDataChanging() { m_Renderer->OnDataChanging(); }
    void OnDataChanged();

    /// @name selection operations
    /// selection-related methods.
    /// @{
    void DeSelectObject(const CObject* obj);
    void SelectObject(const CObject* obj, bool verified);
    void SelectLastHitObject(int mouse_x, int mouse_y);
    void SelectOnlyThisObject(const CSeqGlyph* glyph, int mouse_x, int mouse_y);
    void SelectSeqLoc(const objects::CSeq_loc* loc);
    void ResetSelection();
    void ResetRangeSelection();
    void ResetObjectSelection();
    /// @}

    /// retrieve the selections from our renderer.
    void GetObjectSelection(TConstObjects& objs) const;
    const TRangeColl&    GetRangeSelection(void) const;
    void  SetRangeSelection(const TRangeColl& ranges);
    CConstRef<CSeqGlyph> GetSelectedLayoutObj();

    /// @name IMouseZoomHandlerHost implementation
    /// @{
    virtual TModelUnit  MZHH_GetScale(EScaleType type);
    virtual void        MZHH_SetScale(TModelUnit scale, const TModelPoint& point);
    virtual void        MZHH_ZoomPoint(const TModelPoint& point, TModelUnit factor);
    virtual void        MZHH_ZoomRect(const TModelRect& rc);
    virtual void        MZHH_Scroll(TModelUnit d_x, TModelUnit d_y);
    virtual TVPUnit     MZHH_GetVPPosByY(int y) const;
    virtual void        MZHH_EndOp();
    /// @}

    /// @name ISelHandlerHost implementation
    /// @{
    virtual void        SHH_OnChanged();
    virtual TModelUnit  SHH_GetModelByWindow(int z, EOrientation orient);
    virtual TVPUnit     SHH_GetWindowByModel(TModelUnit z, EOrientation orient);
    /// @}

    /// @name ISeqMarkHandlerHost implementation
    /// @{
    virtual TModelUnit  SMHH_GetModelByWindow(int z, EOrientation orient);
    virtual TSeqPos     SMHH_GetSeqMark() const;
    virtual void        SMHH_SetSeqMark(TSeqPos mark);
    virtual void        SMHH_OnReset(const string& id);
    /// @}

    /// @name class ITrackHandlerHost implementation
    /// @{
    virtual CRef<CLayoutTrack> THH_GetLayoutTrack();
    virtual TModelPoint THH_GetModelByWindow(const wxPoint& pt);
    /// @}

    /// @name ITooltipClient implementation
    /// @{
    //virtual bool    TC_NeedTooltip(const wxPoint&);
    //virtual string  TC_GetTooltip(const wxRect&);
    /// @}

    /// @name IStickyTooltipHandlerHost implementation
    /// @{
    virtual string  TTHH_NeedTooltip(const wxPoint& pt);
    virtual CTooltipInfo  TTHH_GetTooltip(const wxRect& rect);
    virtual bool TTHH_ProcessURL(const wxString &href);
    /// @}


    /// @name ILayoutTrackHost implementation
    /// @{
    //virtual TVPPoint    LTH_GetVPPosByWindowPos(const wxPoint& pos);
    virtual void LTH_OnLayoutChanged();
    virtual void LTH_ZoomOnRange(const TSeqRange& range);
    virtual void LTH_PopupMenu(wxMenu* menu);
    virtual void LTH_PushEventHandler(wxEvtHandler* handler);
    virtual void LTH_PopEventHandler();
    virtual void LTH_ConfigureTracksDlg(const string& category);
    /// @}


    /// @name IGlyphDialogHost implementation
    /// @{
    virtual void PreDialogShow();
    virtual void PostDialogShow();
    /// @}


    TSeqRange FindText(const string &text, bool match_case);
    void ResetSearch();

public:

    /// @name Command handlers
    /// @{
    void ZoomObject(void);
    bool CanZoomSelection(void);

    //void OnEditCut(void);
    void OnEditCopy(void);
    //void OnEditPaste(void);
    void NextPrevSplice(EDirection dir);
    
    void LoadDefaultTracks();
    /// @param expand true if expand, false if collapse
    void ExpandAllTracks(bool expand);
    void ShowAllTracks(bool shown) { m_Renderer->ShowAllTracks(shown); }

    bool CanGoBack(void) const;
    bool CanGoForward(void) const;
    void GoBack(void);
    void GoForward(void);
    /// @}

    void SaveCurrentRange();  ///< For Back or Forward  operations.
    void SaveTrackConfig();

    /// @name IVectorGraphicsRenderer implementations
    /// @{
    virtual const TVPRect& GetViewportRect() const;
    /// Render for vector output.
    virtual void RenderVectorGraphics(int vp_width, int vp_height);
    virtual void UpdateVectorLayout();
    virtual void AddTitle(bool b) { m_Title = b; }
    /// @)

    /// Indicate pane is only for creating vector graphics - ignore other draw events
    void SetVectorMode(bool b) { m_VectorPane = b; }

    void OnSearchTip(const string& tip_id, const wxRect& tip_rect);
    void OnZoomTip(const string& tip_id, const wxRect& /*tip_rect*/);
    void OnInfoTip(const string& /*tip_id*/);
    void SetActivatedTip(const string& id);
    void OnTipAdded(const string& tip_id);
    void OnTipRemoved(const string& tip_id);

    /// get/set markers stored as a string.
    /// Syntax: <position>|<marker name>|<color in RGB hex>
    /// Multiple markers can be set by separating the triples with a comma,
    /// and the name and color parameters are optional.
    /// Examples: 
    ///   - 1000
    ///   - 1000|Marker1|00ff00,
    ///   - 1000|Marker1|00ff00,3000|Marker2|80ff10
    string GetMarkers() const;
    void SetMarkers(const string& markers);
    TSeqPos GetSeqStart() const;

    void GetCurrentObject(int mouse_x, int mouse_y, TConstObjects& objs);
protected:
    DECLARE_EVENT_MAP();

    void x_OnPurgeMarkers(CEvent*);

    /// @name Methods from CGLWidgetPane
    /// @{
    virtual int  x_GetAreaByVPPos(int vp_x, int vp_y);

    /// Get GL view port size.
    /// It is required by CGlWidgetPane.  There is no actuall meaning.
    virtual TVPPoint GetPortSize(void);
    /// @}

    void x_NotifyWidgetSelChanged();
    void x_NotifyWidgetRangeChanged();

    /// Draw the GL window.
    /// It is declared in CwxGLCanvas.
    virtual void x_Render();

    CSeqGraphicWidget* x_GetParent()
    { return (CSeqGraphicWidget*) GetParent(); }

    const CSeqGraphicWidget* x_GetParent() const
    { return (CSeqGraphicWidget*) GetParent(); }

private:
    void x_SelectByRect();
    void x_RenderTooltipConnector();
    void x_RenderRectSelHandler(CGlPane& pane); ///< Rectangular selection
    void x_ChangeSwitchPoint();
    void x_OnEndRectSelect();
    void x_OnEndPan();
    void x_OnPan();

    CConstRef<CSeqGlyph> x_GetOnlyOneSelectedGlyph();
    string x_GetCachedTipId(const CSeqGlyph* glyph) const;
    CWeakRef<CSeqGlyph> x_GetGlyphByTipId(const string& tip_id);

    /// @name marker-related methods
    /// @ {
    CSeqMarkHandler& x_GetDefaultMarker();
    void x_AddNewMarker(CRef<CMarker> marker, 
                        const string& label, 
                        const CRgbaColor& color);
    void x_RemoveMarker(const string& id);
    bool x_HasDefaultMarker() const;
    void x_ClearMarkers();
    bool x_ModifyMarker(CRef<CMarker>& marker, 
                        const string& id, 
                        const string& label, 
                        const CRgbaColor color);
    /// @ }
private:
    typedef vector <TSeqRange> TRangeVec;

    CRef<CSeqGraphicRenderer>   m_Renderer;
    CRef<CSeqGraphicConfig>     m_ConfigSettings;
    CIRef<ISGDataSourceContext> m_DSContext;
    CRef<CSGSequenceDS>         m_DS;

    /// last hit glyph.
    /// not necessarilly selected glyph, and not limited to selectable glyphs.
    CWeakRef<CSeqGlyph>         m_LastHitGlyph;

    /// @name Handlers
    /// @{
    CLayoutTrackHandler m_TrackHandler;
    CLinearSelHandler  m_SelHandler;
    CMouseZoomHandler  m_MouseZoomHandler;
    //CTooltipHandler    m_TooltipHandler;
    CStickyTooltipHandler   m_TooltipManager;
    TSeqMarkers        m_SeqMarkers;
    /// @}

    /// @name navigation history
    /// @{
    TRangeVec   m_BackForwardHistory;
    size_t      m_BackForwardPos;        ///< position in m_BackForwardHistory
    /// @}

    //string        m_TooltipText;
    CTooltipInfo  m_TooltipInfo;
    string        m_ActivatedTipId;
    typedef map< string, CRef<CCachedTipHandle> > TPinnedTips;
    TPinnedTips   m_PinnedTips;

    /// @name Rectangular selection
    /// @{
    wxPoint     m_StartPoint;
    wxPoint     m_DragPoint;
    wxPoint     m_CurrMouse;
    /// @}

    EMouseMode  m_MouseMode;
    bool        m_Flipped;
    bool        m_Horz;

    /// Marker handler index among all registered handlers.
    int         m_MarkerHandlerIndex;
    size_t      m_MarkerId;
    TSeqPos     m_SeqStart;

    /// When rendering vector image, add a title
    bool        m_Title;

    /// If true pane is only for creating vector graphics - ignore other draw events
    bool        m_VectorPane;

#ifdef ATTRIB_MENU_SUPPORT
    float       m_RenderMs;
    string      m_GlRenderer;
#endif
};


////////////////////////////////////////////////////////////////////////////////
/// class CSeqGraphicPane
////////////////////////////////////////////////////////////////////////////////
inline
TModelUnit CSeqGraphicPane::GetHeight()
{
    return m_Renderer->GetHeight();
}

inline
void CSeqGraphicPane::SetUpdateContent()
{
    m_Renderer->SetUpdateContent();
}

inline
bool CSeqGraphicPane::IsHorizontal() const
{
    return m_Horz;
}

inline
bool CSeqGraphicPane::IsFlipped() const
{
    return m_Flipped;
}

inline
void CSeqGraphicPane::LoadDefaultTracks()
{
    m_Renderer->LoadDefaultTracks();
}

inline
void CSeqGraphicPane::ExpandAllTracks(bool expand)
{
    m_Renderer->ExpandAllTracks(expand);
}

inline
void CSeqGraphicPane::OnDataChanged()
{
    m_Renderer->OnDataChanged();
}

inline void CSeqGraphicPane::SetActivatedTip(const string& id)
{
    m_ActivatedTipId = id;
}

inline 
const CSeqGraphicPane::TSeqMarkers& CSeqGraphicPane::GetAllMarkers() const
{
    return m_SeqMarkers;
}

inline
void CSeqGraphicPane::SaveTrackConfig()
{
    m_Renderer->SaveTrackConfig();
}

inline
TSeqPos CSeqGraphicPane::GetSeqStart() const
{
    return m_SeqStart;
}


END_NCBI_SCOPE

#endif  /* GUI_WIDGETS_SEQ_GRAPHIC___SEQGRAPHIC_PANE__HPP */
