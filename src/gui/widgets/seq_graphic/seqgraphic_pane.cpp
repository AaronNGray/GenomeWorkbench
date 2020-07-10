/*  $Id: seqgraphic_pane.cpp 44617 2020-02-06 19:35:49Z filippov $
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
 */


#include <ncbi_pch.hpp>

#include "seqgraphic_pane.hpp"

#include <wx/msgdlg.h>
#include <util/xregexp/regexp.hpp>
#include <objmgr/util/feature.hpp>
#include <objmgr/util/sequence.hpp>
#include <objects/seqfeat/SeqFeatData.hpp>
#include <objects/general/Object_id.hpp>
#include <corelib/ncbitime.hpp>

#include <gui/opengl/glhelpers.hpp>
#include <gui/opengl/glresmgr.hpp>
#include <gui/objutils/tooltip.hpp>
#include <gui/objutils/obj_fingerprint.hpp>
#include <gui/utils/clipboard.hpp>
#include <gui/utils/view_event.hpp>
#include <gui/widgets/wx/async_call.hpp>
#include <gui/widgets/wx/message_box.hpp>
#include <gui/widgets/seq_graphic/switch_point_glyph.hpp>
#include <gui/widgets/seq_graphic/seqgraphic_conf.hpp>
#include <gui/widgets/seq_graphic/seqgraphic_widget.hpp>
#include <gui/widgets/seq_graphic/feature_glyph.hpp>
#include <gui/widgets/seq_graphic/alignment_glyph.hpp>
#include <gui/widgets/seq_graphic/url_tooltip_handler.hpp>
#include <gui/utils/track_info.hpp>

#include <algorithm>
#include <math.h>

#include <gui/widgets/gl/attrib_menu.hpp>

#include "unaligned_region_dlg.hpp"
#include "search_utils.hpp"

BEGIN_NCBI_SCOPE
USING_SCOPE(objects);

const static string kCommonTipId = "----";
const static string kDefaultMarker = "Default";
const static string kDefaultMarkerLabel = "M0";
const static string kNamedMarker = "M";

BEGIN_EVENT_TABLE(CSeqGraphicPane, CGlWidgetPane)
    //EVT_SIZE(CSeqGraphicPane::OnSize)
    EVT_CONTEXT_MENU(CSeqGraphicPane::OnContextMenu)
    EVT_KEY_UP(CSeqGraphicPane::OnKeyUp)
    EVT_KEY_DOWN(CSeqGraphicPane::OnKeyDown)
    EVT_LEFT_DOWN(CSeqGraphicPane::OnLeftDown)
    EVT_MIDDLE_DOWN(CSeqGraphicPane::OnMiddleDown)
    EVT_LEFT_UP(CSeqGraphicPane::OnLeftUp)
    EVT_LEFT_DCLICK(CSeqGraphicPane::OnLeftDblClick)
    EVT_MOTION(CSeqGraphicPane::OnMotion)
    EVT_MOUSEWHEEL(CSeqGraphicPane::OnMouseWheel)
    EVT_MOUSE_CAPTURE_LOST(CSeqGraphicPane::OnMouseCaptureLost)

    EVT_KILL_FOCUS(CSeqGraphicPane::OnKillFocus)
END_EVENT_TABLE()

class CSeqGraphicPaneEvent : public CEvent
{
public:
    enum EEventType {
        ePurgeEvent
    };

    CSeqGraphicPaneEvent() : CEvent(ePurgeEvent) {}
};

BEGIN_EVENT_MAP(CSeqGraphicPane, CEventHandler)
    ON_EVENT(CSeqGraphicPaneEvent, CSeqGraphicPaneEvent::ePurgeEvent, &CSeqGraphicPane::x_OnPurgeMarkers)
END_EVENT_MAP()

template <class T>
class CObjectTypeIs : public unary_function<T, bool>
{
public:
    CObjectTypeIs(){}
    bool operator()(const CObject* arg) const
    {
        return (dynamic_cast<const T*>(arg)) != NULL;
    }
};


static string s_GetTrackName(const string& tip_id)
{
    size_t pos = NStr::Find(tip_id, kCommonTipId, NStr::eCase, NStr::eReverseSearch);
    if (pos != string::npos) {
        return tip_id.substr(pos + kCommonTipId.length());
    }

    return "";
}


CSeqGraphicPane::CSeqGraphicPane(CSeqGraphicWidget* parent)
    : CGlWidgetPane(parent, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxWANTS_CHARS)
    , m_Renderer(new CSeqGraphicRenderer(false))
    , m_DSContext(new CSGDataSourceContext)
    , m_BackForwardPos(0)
    , m_StartPoint(0, 0)
    , m_DragPoint(0, 0)
    , m_CurrMouse(0, 0)
    , m_MouseMode(eMouse_Idle)
    , m_Flipped(false)
    , m_Horz(true)
    , m_MarkerHandlerIndex(0)
    , m_MarkerId(1)
    , m_SeqStart(0)
    , m_Title(false)
    , m_VectorPane(false)
#ifdef ATTRIB_MENU_SUPPORT
    , m_RenderMs(0.0f)
#endif
{
    if (x_GetParent()) {
        CSeqGraphicWidget* parent_widget = x_GetParent();
        AddListener(parent_widget, ePool_Parent);
        //m_MatrixPane = parent->GetPort();
    }

    m_Renderer->SetDSContext(m_DSContext);
    m_Renderer->SetHost(this);

    // setup Event Handlers
    m_TrackHandler.SetHost(this);
    m_TrackHandler.SetPane(&m_Renderer->GetFeatGlPane());
    x_RegisterHandler(&m_TrackHandler, fArea_Object,
        &m_Renderer->GetFeatGlPane(), m_MarkerHandlerIndex++);

    m_TooltipManager.SetHost(this);
    x_RegisterHandler(&m_TooltipManager, fArea_Object,
        &m_Renderer->GetFeatGlPane(), m_MarkerHandlerIndex++);

    // NOTE: we want to put all sequence markers after tooltip handler and before
    // selection handler. If there is any other handler needs to be registered
    // before sequence marker, please increase m_MarkerHandlerIndex to make sure
    // sequence markers added during run-time will be registered to the right
    // order.
    //m_SeqMarkHandler.SetHost(this);
    //x_RegisterHandler(&m_SeqMarkHandler, fArea_Ruler,
    //   &m_Renderer->GetRulerGlPane(), m_MarkerHandlerIndex);

    m_SelHandler.SetOrientation(eHorz);
    m_SelHandler.SetHost(this);
    x_RegisterHandler(&m_SelHandler, fArea_Ruler, &m_Renderer->GetRulerGlPane());

    m_MouseZoomHandler.SetHost(this);
    m_MouseZoomHandler.SetMode(CMouseZoomHandler::eHorz);
    x_RegisterHandler(&m_MouseZoomHandler, fArea_All, &m_Renderer->GetFeatGlPane());

#ifdef ATTRIB_MENU_SUPPORT
    //CAttribMenu& m = CAttribMenuInstance::GetInstance();
    //CAttribMenu* m1 = m.AddSubMenuUnique("SeqSubmenu", this);
    //m1->AddFloat("Pre-popup Move Max", & (m_TooltipManager.GetMoveThreshold()), 6.0f, 0.0f, 20.0f, 0.1f);
    //m1->AddInt("Post-popup Clear Delay", & (m_TooltipManager.GetClearUnpinnedDelay()), 200, 10, 1000, 10);
    //m1->AddFloat("Tip Dist", &(m_TooltipManager.GetPopupDistance()), 0.333f, 0.0f, 2.0f, 0.01f);
    //m1->AddInt("Tip Popup Delay", &(m_TooltipManager.GetTipPopupDelay()), 200, 10, 1000, 10);

    CAttribMenu& m = CAttribMenuInstance::GetInstance();
    CAttribMenu* sub_menu = m.AddSubMenuUnique("Seq Render", this);
    sub_menu->AddFloatReadOnly("Render MS: ", &m_RenderMs);
    CAttribStringsMenuItem* strings = sub_menu->AddStrings("Renderer", &m_GlRenderer);
    strings->AddString("GL Default");
    strings->AddString("Debug");
    strings->SetValue(0);
    m_GlRenderer = "GL Default";
    // support <, > for closing and opening menus
    m.SetOpenCloseKeys(int('.'), int(','));
#endif
}


CSeqGraphicPane::~CSeqGraphicPane()
{
    if (m_ConfigSettings) {
        m_ConfigSettings->SaveSettings();
    }

    //CAttribMenu& m = CAttribMenuInstance::GetInstance();
    //m.RemoveMenuR("SeqSubmenu", this);

#ifdef ATTRIB_MENU_SUPPORT
    CAttribMenuInstance::GetInstance().RemoveMenuR("Seq Render", this);
#endif
}


void CSeqGraphicPane::SetInputObject(SConstScopedObject& obj)
{
    if ( !m_DS ) {
        InitDataSource(obj);
    }

    m_Renderer->SetInputObject(obj);
    m_Renderer->ConfigureTracks();

    int w, h;
    GetClientSize(&w, &h);
    TVPRect rcVP(0, 0, w, h);
    m_Renderer->SetHorizontal(m_Horz, m_Flipped, rcVP, true);

    m_SelHandler.ResetSelection(false);
    m_BackForwardHistory.clear();
    m_BackForwardPos = 0;
    m_BackForwardHistory.push_back(TSeqRange(0, m_DS->GetSequenceLength()));
    x_ClearMarkers();
    //CEvent evt(CEvent::eEvent_Message, CViewEvent::eWidgetDataChanged);
    //Send(&evt, CEventHandler::eDispatch_Default, ePool_Parent);
}


void CSeqGraphicPane::InitDataSource(SConstScopedObject& obj)
{
    m_DSContext->ClearCache();
    CIRef<ISGDataSource> ds = m_DSContext->GetDS(
        typeid(CSGSequenceDSType).name(), obj);
    m_DS.Reset(dynamic_cast<CSGSequenceDS*>(ds.GetPointer()));
}


CSGSequenceDS* CSeqGraphicPane::GetDataSource(void)
{
    return m_DS.GetPointer();
}


CFeaturePanel* CSeqGraphicPane::GetFeaturePanel()
{
    return m_Renderer->GetFeaturePanel();
}


void CSeqGraphicPane::OnKeyUp(wxKeyEvent& event)
{
    //if (m_LenseZoom) {
    //    m_Renderer->CancelLensZoom();
    //    m_LenseZoom = false;
    //    Refresh();
    //}

    event.Skip();
}


void CSeqGraphicPane::OnKeyDown(wxKeyEvent& event)
{
    if (!m_DS) {
        event.Skip();
        return;
    }

    //bool b_ctrl = wxGetKeyState(WXK_CONTROL);
    bool b_shift = wxGetKeyState(WXK_SHIFT);

    switch (event.GetKeyCode()) {
    case 'd':
    case 'D':
        {{
            // Lense Zoom (D key)
            //m_LenseZoom = true;
            //m_Renderer->SetLensZoom(m_CurrMouse.x,
            //MZHH_GetVPPosByY(m_CurrMouse.y));
            //Refresh();
        }}
        break;
    case 'm':
    case 'M':
        {{
            // set marker
            TModelUnit x = SHH_GetModelByWindow(m_CurrMouse.x, eHorz);
            TModelUnit y = SHH_GetModelByWindow(m_CurrMouse.y, eVert);
            SetSeqMarker(TModelPoint(x, y));
            Refresh();
        }}
        break;
    case '/':
        // wuliangs: move this to switch point track
        x_ChangeSwitchPoint();
        Refresh();
        break;
    case WXK_LEFT:
        NextPrevSplice(CSeqGraphicPane::eDir_Left);
        break;
    case WXK_RIGHT:
        NextPrevSplice(CSeqGraphicPane::eDir_Right);
        break;
    case WXK_TAB:
        if (b_shift) {
            NextPrevSplice(CSeqGraphicPane::eDir_Prev);
        } else {
            NextPrevSplice(CSeqGraphicPane::eDir_Next);
        }
        break;
    case WXK_UP: // Prevent gtk navigation via arrow keys
    case WXK_DOWN:
        break;
    default:
        event.Skip();
        break;
    }
}


void CSeqGraphicPane::OnLeftUp(wxMouseEvent& event)
{
    if (m_DS  &&  m_MouseMode != eMouse_Idle) {
        m_CurrMouse = event.GetPosition();
        int x = m_CurrMouse.x;
        int y = MZHH_GetVPPosByY(m_CurrMouse.y);
        CRef<CSeqGlyph> last_hit_glyph = m_Renderer->HitTest(x, y);
        m_LastHitGlyph.Reset(last_hit_glyph.GetPointer());

        if (m_MouseMode == eMouse_RectSelect  ||
            m_MouseMode == eMouse_IncRectSelect) {
            x_OnEndRectSelect();
        } else if (m_MouseMode == eMouse_Pan) {
            x_OnEndPan();
        } else if (m_MouseMode == eMouse_IncSelect) {
            const IObjectBasedGlyph* obj =
                dynamic_cast<const IObjectBasedGlyph*>(last_hit_glyph.GetPointer());
            if (obj) {
                CConstRef<CObject> sel_obj =
                    obj->GetObject(m_Renderer->Screen2Seq(x, y));
                if (m_Renderer->IsObjectSelected(sel_obj)) {
                    DeSelectObject(sel_obj);
                } else {
                    SelectObject(sel_obj, true);
                    // reset Ruler to selected object
                    if (m_ConfigSettings->IsAjdustRulerToSelection()) {
                        TSeqRange range = last_hit_glyph->GetRange();
                        const CFeatGlyph* feat = dynamic_cast<const CFeatGlyph*>(
                            last_hit_glyph.GetPointer());
                        if (feat) {
                            bool neg_strand = (sequence::GetStrand(feat->GetLocation())
                                == eNa_strand_minus);
                            // respect strand
                            TSeqPos r_pos = neg_strand ? range.GetTo() : range.GetFrom();
                            SetSeqStart(r_pos);
                        } else {
                            SetSeqStart(range.GetFrom());
                        }
                    }
                }
            }
            m_MouseMode = eMouse_Idle;
            x_NotifyWidgetSelChanged();
            GHH_SetCursor(wxCursor(wxCURSOR_ARROW));
            GHH_ReleaseMouse();
            Refresh();
        } else if (m_MouseMode == eMouse_Down) {
            // Select last clicked object if it is selectable
            SelectOnlyThisObject(last_hit_glyph.GetPointer(), x, y);
            m_MouseMode = eMouse_Idle;
            GHH_SetCursor(wxCursor(wxCURSOR_ARROW));
            GHH_ReleaseMouse();
            Refresh();
        }
    } else {
        event.Skip();
    }
}


void CSeqGraphicPane::OnLeftDown(wxMouseEvent& event)
{
    CheckOverlayTimer();

    if (!m_DS) {
        event.Skip();
        return;
    }

    m_LastHitGlyph.Reset();

    if (wxGetKeyState(wxKeyCode('Z'))  ||
        wxGetKeyState(wxKeyCode('R'))  ||
        wxGetKeyState(wxKeyCode('P')) )
    {
        event.Skip();
        return;
    }

    m_StartPoint = m_DragPoint = event.GetPosition();

    int x = m_StartPoint.x;
    int y = MZHH_GetVPPosByY(m_StartPoint.y);
    int area = x_GetAreaByVPPos(x, y);
    if (area != fArea_Object) {
        event.Skip();
        return;
    }

    SetFocus();

    CRef<CSeqGlyph> last_hit_glyph = m_Renderer->HitTest(x, y);
    m_LastHitGlyph.Reset(last_hit_glyph.GetPointer());

    bool rect_select = event.ShiftDown();
    bool inc_select = event.CmdDown();

    // do track-specific interactions if it is not extension
    // selection (shift key down) or incremental selection (ctrl key down)
    if (last_hit_glyph  &&  !inc_select  &&  !rect_select ) {
        CLayoutTrack* track =
            dynamic_cast<CLayoutTrack*>(last_hit_glyph.GetPointer());
        if (track) {
            event.Skip();
            return;
        }
    }

    if (rect_select){
        if (inc_select) {
            m_MouseMode = eMouse_IncRectSelect;
        } else {
            m_MouseMode = eMouse_RectSelect;
        }
    } else if (inc_select) {
        m_MouseMode = eMouse_IncSelect;
    } else {
        const CGlPane& pane = m_Renderer->GetFeatGlPane();
        TModelPoint pos = pane.UnProject(x, y);
        // call glyph-specific left down event handler
        if (last_hit_glyph  &&  last_hit_glyph->OnLeftDown(pos)) {
            return;
        }

        m_MouseMode = eMouse_Down;
    }
    GHH_CaptureMouse();
}


void CSeqGraphicPane::OnMiddleDown(wxMouseEvent& event)
{
    CheckOverlayTimer();

    if ( !HasCapture() ) {
        event.Skip();
    }
}

void CSeqGraphicPane::OnMotion(wxMouseEvent& event)
{
    if (!m_DS) {
        event.Skip();
        return;
    }

    if (wxGetKeyState(wxKeyCode('Z'))  ||
        wxGetKeyState(wxKeyCode('R'))  ||
        wxGetKeyState(wxKeyCode('P')) )
    {
        event.Skip();
        return;
    }

    m_CurrMouse = event.GetPosition();
    CRef<CSeqGlyph> last_hit_glyph =
        m_Renderer->HitTest(m_CurrMouse.x, MZHH_GetVPPosByY(m_CurrMouse.y));
    m_LastHitGlyph.Reset(last_hit_glyph.GetPointer());

    if (m_MouseMode == eMouse_RectSelect  ||
        m_MouseMode == eMouse_IncRectSelect) {
        wxPoint pos = event.GetPosition();
        if (pos.x != m_DragPoint.x  ||  pos.y != m_DragPoint.y) {
            m_DragPoint = pos;
            GHH_SetCursor(wxCursor(wxCURSOR_CROSS));
            Refresh();
        }
    } else if (m_MouseMode == eMouse_Pan  ||
               m_MouseMode == eMouse_Down) {
        m_MouseMode = eMouse_Pan;
        m_CurrMouse = event.GetPosition();
        x_OnPan();
        m_StartPoint = m_CurrMouse;
        Refresh();
    } else if (last_hit_glyph  &&  last_hit_glyph->IsClickable()) {
        GHH_SetCursor(wxCursor(wxCURSOR_HAND));
    } else {
        GHH_SetCursor(wxCursor(wxCURSOR_ARROW));
    }

    if (m_MouseMode == eMouse_Idle) {
        event.Skip();
    }
}


void CSeqGraphicPane::OnMouseWheel(wxMouseEvent& event)
{
    CheckOverlayTimer();
    if (wxGetKeyState(wxKeyCode('Z'))  ||
        wxGetKeyState(wxKeyCode('R'))  ||
        wxGetKeyState(wxKeyCode('P')) )
    {
        event.Skip();
        return;
    }

    int shift = event.GetWheelRotation()/4;
    if (m_Horz) {
        MZHH_Scroll(0, -shift);
    } else {
        MZHH_Scroll(-shift, 0);
    }
}


void CSeqGraphicPane::OnLeftDblClick(wxMouseEvent& event)
{
    CheckOverlayTimer();

    if (!m_DS) {
        event.Skip();
        return;
    }

    wxPoint pos = event.GetPosition();
    pos.y = MZHH_GetVPPosByY(pos.y);

    int area = x_GetAreaByVPPos(pos.x, pos.y);
    if (area != fArea_Object) {
        event.Skip();
        return;
    }

    CRef<CSeqGlyph> obj = m_Renderer->HitTest(pos.x, pos.y);
    const CGlPane& pane = m_Renderer->GetFeatGlPane();
    TModelPoint pnt = pane.UnProject(pos.x, pos.y);

    if (obj  &&  ! obj->OnLeftDblClick(pnt)) {
        TSeqRange range = obj->GetRange();

        // TODO: do we need to take care of any special cases?
        x_GetParent()->ZoomOnRange(range,
            CSeqGraphicWidget::fAddMargins | CSeqGraphicWidget::fSaveRange);
    } else if ( !obj ) {
        if ( m_Renderer->HasSelectedObjects() ) {
            m_Renderer->ResetObjectSelection();
            x_NotifyWidgetSelChanged();
            Refresh();
        }
    }
}


void CSeqGraphicPane::OnMouseCaptureLost(wxMouseCaptureLostEvent& /*event*/)
{
    switch (m_MouseMode) {
        case eMouse_RectSelect:
        case eMouse_IncRectSelect:
            x_OnEndRectSelect();
            break;
        case eMouse_Pan:
            x_OnEndPan();
            break;
        default:
            m_MouseMode = eMouse_Idle;
            break;
    }
}


void CSeqGraphicPane::x_OnEndRectSelect()
{
    if (m_MouseMode == eMouse_RectSelect  &&
        m_Renderer->HasSelectedObjects()) {
        m_Renderer->ResetObjectSelection();
    }

    x_SelectByRect();
    m_MouseMode = eMouse_Idle;

    x_NotifyWidgetSelChanged();
    GHH_SetCursor(wxCursor(wxCURSOR_ARROW));
    GHH_ReleaseMouse();
    Refresh();
}


void CSeqGraphicPane::x_OnEndPan()
{
    m_MouseMode = eMouse_Idle;
    x_OnPan();
    GHH_SetCursor(wxCursor(wxCURSOR_ARROW));
    GHH_ReleaseMouse();
    Refresh();
    SaveCurrentRange();
}


void CSeqGraphicPane::x_OnPan()
{
    const CGlPane& pane = m_Renderer->GetFeatGlPane();
    TModelUnit m_x1 = pane.UnProjectX(m_StartPoint.x);
    TModelUnit m_x2 = pane.UnProjectX(m_CurrMouse.x);

    int y1 = MZHH_GetVPPosByY(m_StartPoint.y);
    int y2 = MZHH_GetVPPosByY(m_CurrMouse.y);
    TModelUnit m_y1 = pane.UnProjectY(y1);
    TModelUnit m_y2 = pane.UnProjectY(y2);

    MZHH_Scroll(m_x1 - m_x2, m_y1 - m_y2);
}


string CSeqGraphicPane::x_GetCachedTipId(const CSeqGlyph* glyph) const
{
    string tip_id = "";
    const IObjectBasedGlyph* obj_glyph =
        dynamic_cast<const IObjectBasedGlyph*>(glyph);
    if (obj_glyph) {
        CConstRef<CObject> obj = obj_glyph->GetObject(0);
        string track_name = "";

        const CSeqGlyph* p = glyph->GetParent();
        while (p) {
            const CLayoutTrack* p_track = dynamic_cast<const CLayoutTrack*>(p);
            if (p_track) {
                track_name = p_track->GetFullTitle();
                break;
            }
            p = p->GetParent();
        }

        ITERATE (TPinnedTips, iter, m_PinnedTips) {
            const CCachedTipHandle& handle = *iter->second;
            if (handle.HasMatches(const_cast<CObject&>(*obj), m_DS->GetScope())  &&
                handle.GetParentTrackName() == track_name) {
                    return iter->first;
            }
        }

    }
    return tip_id;
}


class CGlyphSearchVisitor : public IGlyphVisitor
{
public:
    typedef list< CWeakRef<CSeqGlyph> > TSelectedGlyphs;

    CGlyphSearchVisitor(const CCachedTipHandle& handle,
        CScope& scope)
        : m_TipHandle(handle)
        , m_Scope(&scope)
    {}

    /// @name IGlyphVisitor interface implementation
    /// @{
    virtual bool Visit(CSeqGlyph* glyph);
    /// @}

    TSelectedGlyphs Search(CSeqGlyph* glyph)
    {
        m_Glyphs.clear();
        glyph->Accept(this);
        return m_Glyphs;
    }

private:
    const CCachedTipHandle& m_TipHandle;
    CRef<CScope>             m_Scope;
    TSelectedGlyphs          m_Glyphs;
};


bool CGlyphSearchVisitor::Visit(CSeqGlyph* glyph)
{
    bool cont = true;
    IObjectBasedGlyph* obj_glyph = dynamic_cast<IObjectBasedGlyph*>(glyph);
    if (obj_glyph) {
        CConstRef<CObject> glyph_o(obj_glyph->GetObject(0));
        if (!glyph_o.Empty()) {
            const CObject& obj = *glyph_o;
            if (m_TipHandle.HasMatches(obj, *m_Scope)) {
                string track_name = "";
                const CSeqGlyph* p = glyph->GetParent();
                while (p) {
                    const CLayoutTrack* p_track = dynamic_cast<const CLayoutTrack*>(p);
                    if (p_track) {
                        track_name = p_track->GetFullTitle();
                        break;
                    }
                    p = p->GetParent();
                }
                if (m_TipHandle.GetParentTrackName() == track_name) {
                    m_Glyphs.push_back(CWeakRef<CSeqGlyph>(glyph));
                    cont = false;
                }
            }
        }
    }
    return cont;
}


CWeakRef<CSeqGlyph> CSeqGraphicPane::x_GetGlyphByTipId(const string& tip_id)
{
    CWeakRef<CSeqGlyph> glyph;
    TPinnedTips::const_iterator iter = m_PinnedTips.find(tip_id);
    if (iter == m_PinnedTips.end()) return glyph;

    const CCachedTipHandle& handle = *iter->second;
    CGlyphSearchVisitor glyph_searcher(handle, m_DS->GetScope());
    CGlyphSearchVisitor::TSelectedGlyphs glyphs =
        glyph_searcher.Search(GetFeaturePanel());
    if ( !glyphs.empty() ) {
        glyph = glyphs.front();
    }
    return glyph;
}


void CSeqGraphicPane::AdjustViewPort()
{
    m_Renderer->SetHorizontal(m_Horz, m_Flipped,
        x_GetParent()->GetPort().GetViewport());
    Refresh();
}


void CSeqGraphicPane::SetHorizontal(bool b_horz, bool b_flip)
{
    m_Horz = b_horz;
    m_Flipped = b_flip; 
    m_Renderer->SetHorizontal(b_horz, b_flip,
        x_GetParent()->GetPort().GetViewport());

    // Any range-based seqmarkers need to be flipped.
    NON_CONST_ITERATE (TSeqMarkers, iter, m_SeqMarkers) {
        if (iter->second->IsRemoved()) continue;

        iter->second->SetFlipped(m_Flipped);
    }

}


void CSeqGraphicPane::UpdateData(TSeqPos from, TSeqPos to)
{
    x_GetParent()->ZoomOnRange(TSeqRange(from, to), 0);
    m_Renderer->ZoomOnRange(TModelRange(from, to));
    m_Renderer->UpdateData();
}


void CSeqGraphicPane::AdjustViewPortHeightToImage()
{
    TVPRect& vp = x_GetParent()->GetPort().GetViewport();
    TVPUnit img_height = (TVPUnit)GetHeight();
    img_height = max(220, img_height);
    vp.SetTop(img_height);
    m_Renderer->SetHorizontal(m_Horz, m_Flipped,
        x_GetParent()->GetPort().GetViewport());
}


void CSeqGraphicPane::SetViewportWidth(TSeqPos from, TSeqPos to, TVPUnit vp_width)
{
    TVPRect& vp = x_GetParent()->GetPort().GetViewport();
    TModelRect& rect = x_GetParent()->GetPort().GetVisibleRect();
    vp.SetRight(vp_width);
    rect.SetLeft(from);
    rect.SetRight(to);
}


bool CSeqGraphicPane::AllJobsFinished() const
{
    return m_Renderer->AllJobsFinished();
}

bool CSeqGraphicPane::AllTracksCreated() const
{
    return m_Renderer->AllTracksCreated();
}

void CSeqGraphicPane::SetExternalGlyphs(const CSeqGlyph::TObjects& objs)
{
    m_Renderer->SetExternalGlyphs(objs);
}


// Set/Clear selection
void CSeqGraphicPane::DeSelectObject(const CObject* obj)
{
    if (!obj) {
        return;
    }

    m_Renderer->DeSelectObject(obj);
	Refresh();
}


void CSeqGraphicPane::SelectObject(const CObject* obj, bool verified)
{
    if (!obj) {
        return;
    }

    m_Renderer->SelectObject(obj, verified);
	Refresh();
}


void CSeqGraphicPane::SelectLastHitObject(int mouse_x, int mouse_y)
{
    CWeakRef<CSeqGlyph>::TRefType last_hit_glyph = m_LastHitGlyph.Lock();
    if (last_hit_glyph  &&  !last_hit_glyph->IsSelected()) {
        SelectOnlyThisObject(last_hit_glyph.GetPointer(), mouse_x, mouse_y);
    }
}


void CSeqGraphicPane::SelectOnlyThisObject(const CSeqGlyph* glyph,
    int mouse_x, int mouse_y)
{
    const IObjectBasedGlyph* obj = dynamic_cast<const IObjectBasedGlyph*>(glyph);
    if (obj) {
        // deselect all of the originally selected objects
        if (m_Renderer->HasSelectedObjects()) {
            m_Renderer->ResetObjectSelection();
        }
        CConstRef<CObject> sel_obj
            = obj->GetObject(m_Renderer->Screen2Seq(mouse_x, mouse_y));
        SelectObject(sel_obj, true);

        // reset Ruler to selected object
        if (m_ConfigSettings->IsAjdustRulerToSelection()) {
            TSeqRange range = glyph->GetRange();
            const CFeatGlyph* feat = dynamic_cast<const CFeatGlyph*>(glyph);
            if (feat) {
                bool neg_strand = (sequence::GetStrand(feat->GetLocation())
                    == eNa_strand_minus);
                // respect strand
                TSeqPos r_pos = neg_strand ? range.GetTo() : range.GetFrom();
                SetSeqStart(r_pos);
            } else {
                SetSeqStart(range.GetFrom());
            }
        }
        x_NotifyWidgetSelChanged();
    }
}

void CSeqGraphicPane::GetCurrentObject(int mouse_x, int mouse_y, TConstObjects& objs)
{
    CRef<CSeqGlyph> glyph = m_Renderer->HitTest(mouse_x, mouse_y);
    const IObjectBasedGlyph* obj = dynamic_cast<const IObjectBasedGlyph*>(glyph.GetPointer());
    if (obj) {      
        CConstRef<CObject> sel_obj = obj->GetObject(m_Renderer->Screen2Seq(mouse_x, mouse_y));
        objs.push_back(sel_obj);
    }
}

void CSeqGraphicPane::SelectSeqLoc(const CSeq_loc* loc)
{
    try {
        TRangeColl r = TRangeColl( loc->GetTotalRange() );
        m_SelHandler.SetSelection(r, false);
        Refresh();
    } catch (CException& e) {
        LOG_POST(Warning << "CSeqGraphicPane::SelectSeqLoc "
                 << e.GetMsg());
    }
}


void CSeqGraphicPane::ResetSelection()
{
    ResetRangeSelection();
    ResetObjectSelection();
}


void CSeqGraphicPane::ResetRangeSelection()
{
    m_SelHandler.ResetSelection(false);
	Refresh();
}


void CSeqGraphicPane::ResetObjectSelection()
{
    m_Renderer->ResetObjectSelection();
	Refresh();
}


// retrieve the selections from our renderer
void CSeqGraphicPane::GetObjectSelection(TConstObjects& objs) const
{
    m_Renderer->GetObjectSelection(objs);
}


const CSeqGraphicPane::TRangeColl&
     CSeqGraphicPane::GetRangeSelection(void) const
{
    return m_SelHandler.GetSelection();
}


void CSeqGraphicPane::SetRangeSelection(const TRangeColl& ranges)
{
    m_SelHandler.SetSelection(ranges, true);
	Refresh();
}


void CSeqGraphicPane::x_Render(void)
{
    // Don't do non-vector rendering from a pane that was created
    // just to write vector graphics - It can cause problems with the primary
    // pane user is looking at...
    if (m_VectorPane) {
        return;
    }

    IRender& gl = GetGl();

    /// TODO: we clear the frame buffers twice, one here
    /// and another one in m_Renderer.  Do we need this?
    glClearColor(1, 1, 1, 0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    if (!x_GetParent()  ||  !m_DS)
        return;

    CIRef<IRender>  mgr = CGlResMgr::Instance().
        GetRenderer( CGlResMgr::Instance().GetApiLevel());
    if (mgr.IsNull()) {
        LOG_POST(Error << "IRender object not available.");
        return;
    }

    CGlResMgr::Instance().SetCurrentRenderer(mgr);

#ifdef ATTRIB_MENU_SUPPORT
    if (m_GlRenderer != "GL Default") {  // "Debug"
        CRef<IRender>  mgr = CGlResMgr::Instance().GetRenderer(eRenderDebug);
        CGlRenderDebug* rdebug = NULL;
        if (mgr.IsNull()) {
            rdebug = new CGlRenderDebug();
            mgr.Reset(rdebug);
            CGlResMgr::Instance().AddRenderer(mgr);
        }
        else {
            rdebug = dynamic_cast<CGlRenderDebug*>(mgr.GetPointerOrNull());
            if (rdebug == NULL) {
                LOG_POST(Error << "CGlRenderDebug: unable to cast render manager to CGlRenderDebug");
                return;
            }
        }
        CGlResMgr::Instance().SetCurrentRenderer(mgr);
    }

    CStopWatch  t;
    t.Start();
#endif

    //m_Renderer->ClearDisplayList();
    TModelRange mrc = x_GetParent()->GetVisibleRange();
    m_Renderer->ZoomOnRange(mrc);
    m_Renderer->SetVertScroll(x_GetParent()->GetPort().GetVisibleRect().Top());
    m_Renderer->Render();

    CGlPane& ruler_pane = m_Renderer->GetRulerGlPane();
    CGlPane& feat_pane  = m_Renderer->GetFeatGlPane();

    m_TrackHandler.Render(feat_pane);

    // ZoomHandler indicator, Linear selection, seqmark (hairpin),
    // rectangular selection and bookmarks
    gl.Enable(GL_BLEND);
    gl.BlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // rendering highlighted tooltip
    x_RenderTooltipConnector();

    // Sequence markers (hairpin) on the ruler bar
    NON_CONST_ITERATE (TSeqMarkers, iter, m_SeqMarkers) {
        if (iter->second->IsRemoved()) continue;

        iter->second->Render(ruler_pane, CSeqMarkHandler::eActiveState);
        iter->second->Render(feat_pane,  CSeqMarkHandler::ePassiveState);
    }

    // Zoom
    m_MouseZoomHandler.Render(ruler_pane);
    m_MouseZoomHandler.Render(feat_pane);

    // draw linear selection
    m_SelHandler.Render(ruler_pane);
    m_SelHandler.Render(feat_pane, CLinearSelHandler::ePassiveState);

    x_RenderRectSelHandler(feat_pane);

    gl.Disable(GL_BLEND);

    x_GetParent()->UpdateHeight( m_Renderer->GetLayoutHeight() );

    CHECK_GLERROR();

#ifdef ATTRIB_MENU_SUPPORT
    t.Stop();
    m_RenderMs = t.Elapsed();

    CAttribMenuInstance::GetInstance().DrawMenu();
#endif

}


const TVPRect& CSeqGraphicPane::GetViewportRect() const
{
    return x_GetParent()->GetPort().GetViewport();
}


void CSeqGraphicPane::RenderVectorGraphics(int vp_width, int vp_height)
{
    if (!x_GetParent()  ||  !m_DS)
        return;

    IRender& gl = GetGl();

    // main rendering...
    m_Renderer->RenderVectorGraphics();

    CGlPane& ruler_pane = m_Renderer->GetRulerGlPane();
    CGlPane& feat_pane  = m_Renderer->GetFeatGlPane();

    m_TrackHandler.Render(feat_pane);

    // ZoomHandler indicator, Linear selection, seqmark (hairpin),
    // rectangular selection and bookmarks
    gl.Enable(GL_BLEND);
    gl.BlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // rendering highlighted tooltip
    //x_RenderTooltipConnector();

    // Sequence markers (hairpin) on the ruler bar
    NON_CONST_ITERATE (TSeqMarkers, iter, m_SeqMarkers) {
        if (iter->second->IsRemoved()) continue;

        iter->second->Render(ruler_pane, CSeqMarkHandler::eActiveState);
        iter->second->Render(feat_pane,  CSeqMarkHandler::ePassiveState);
    }

    // draw linear selection

    m_SelHandler.Render(ruler_pane);
    m_SelHandler.Render(feat_pane, CLinearSelHandler::ePassiveState);
    x_RenderRectSelHandler(feat_pane);

    gl.Disable(GL_BLEND);


    ///
    /// Draw Title if requested.  The dialog should have made room
    /// for the title be creating the viewing window (CMedia) to be larger
    ///

    if (m_Title) {

        CRef<CGlTextureFont> font = m_ConfigSettings->GetTitleFont();
        float title_height = font->TextHeight() - font->GetFontDescender();
        float vert_offset = title_height + 2.0f; 

        string title = m_DS->GetAcc_Best();
        if (title.size() > 0) 
            title += ":";

        TSeqRange range = m_Renderer->GetVisibleRange();
        title += NStr::NumericToString((unsigned long)range.GetFrom());
        title += ".." + NStr::NumericToString((unsigned long)range.GetTo());
        title += " " + m_DS->GetTitle();

        gl.Viewport(0, vp_height-vert_offset, vp_width, vert_offset);
        gl.MatrixMode(GL_PROJECTION);
        gl.LoadIdentity();
        gl.Ortho(0, vp_width, 0, vert_offset, -1.0, 1.0);
        gl.MatrixMode(GL_MODELVIEW);
        gl.LoadIdentity();

        gl.Color3f(0.9f, 0.9f, 0.9f);
        gl.Rectf(0.0f, 0.0f, float(vp_width), float(vert_offset));
        gl.Color3f(0.0f, 0.0f, 0.0f);
        gl.BeginText(font);
        TModelUnit yoffset = -font->GetFontDescender();
        gl.WriteText(0.0f, yoffset, float(vp_width), float(vert_offset-yoffset), title.c_str(), IGlFont::eAlign_Left);
        gl.EndText();
    }
}


void CSeqGraphicPane::UpdateVectorLayout()
{
    if (!x_GetParent()  ||  !m_DS)
        return;

    // The layout in PDF model may be different from the one
    // regular rendering mode. Update the layout and adjust
    // the image size
    m_Renderer->UpdateVectorLayout();
    AdjustViewPortHeightToImage();
}


CConstRef<CSeqGlyph> CSeqGraphicPane::GetSelectedLayoutObj()
{
    return x_GetOnlyOneSelectedGlyph();
}

// wuliangs: move this to Switch Point Track
void CSeqGraphicPane::x_ChangeSwitchPoint()
{
    CConstRef<CSeqGlyph> sel_glyph = x_GetOnlyOneSelectedGlyph();

    const CSwitchPointGlyph* sp =
        dynamic_cast<const CSwitchPointGlyph*>(sel_glyph.GetPointer());

    if ( !sp )  return;

    if (x_HasDefaultMarker()) {
        TSeqPos marker_pos = x_GetDefaultMarker().GetPos();
        // marker is on this alignment?
        TSeqRange range = sp->GetRange();
        if (!range.IntersectingWith(TSeqRange(marker_pos, marker_pos)))
            return;

        CRef<objects::CSeqMapSwitchPoint> the_point = sp->GetSwitchPoints();
        if (!the_point)
            return;

        try {
            the_point->ChangeSwitchPoint(marker_pos, 0);
        } catch (std::exception&) {
            LOG_POST(Error << "x_ChangeSwitchPoint(): error chaning switch point");
        }
    }
}


void CSeqGraphicPane::x_SelectByRect()
{
    int x1 = m_StartPoint.x;
    int y1 = MZHH_GetVPPosByY(m_StartPoint.y);
    int x2 = m_DragPoint.x;
    int y2 = MZHH_GetVPPosByY(m_DragPoint.y);
    TVPRect rc( min(x1, x2), min(y1, y2), max(x1, x2), max(y1, y2));
    if (!rc.IsEmpty()) {
        m_Renderer->SelectObjByRect(rc);
    }
}


void CSeqGraphicPane::x_RenderTooltipConnector()
{
    if (m_ActivatedTipId.empty()  ||
        NStr::StartsWith(m_ActivatedTipId, kCommonTipId)) {
        return;
    }

    IRender& gl = GetGl();

    // get the glyph for the given tip_id
    CWeakRef<CSeqGlyph> glyph_wk = x_GetGlyphByTipId(m_ActivatedTipId);
    CWeakRef<CSeqGlyph>::TRefType glyph = glyph_wk.Lock();
    // get tooltip frame rect
    vector<TipLocation> tip_rects = GetDisplayedTips();
    vector<TipLocation>::const_iterator curr_tip = tip_rects.begin();
    while (curr_tip != tip_rects.end()  &&  curr_tip->TipID != m_ActivatedTipId) {
        ++curr_tip;
    }

    if (glyph  &&  curr_tip != tip_rects.end()) {
        const CGlRect<float>& tip_rect = curr_tip->TipRect;
        const CRenderingContext* ctx = glyph->GetRenderingContext();

        CGlPane& pane = m_Renderer->GetFeatGlPane();

        // calculate tooltip rect in feature pane space
        TModelRect tip_rect_m;
        tip_rect_m.SetLeft(ctx->GetOffset() + ctx->ScreenToSeq(m_Flipped ? -tip_rect.Right() : tip_rect.Left()));
        tip_rect_m.SetRight(ctx->GetOffset() + ctx->ScreenToSeq(m_Flipped ? -tip_rect.Left() : tip_rect.Right()));
        tip_rect_m.SetTop(pane.GetOffsetY() - tip_rect.Top());
        tip_rect_m.SetBottom(tip_rect_m.Top() + tip_rect.Height());

        // calculate object rect in world corrd.
        TModelPoint glyph_pos;
        glyph->GetPosInWorld(glyph_pos);
        TModelRect glyph_rect;
        glyph_rect.SetLeft(glyph_pos.X());
        glyph_rect.SetTop(glyph_pos.Y());
        glyph_rect.SetRight(glyph_pos.X() + glyph->GetWidth());
        glyph_rect.SetBottom(glyph_pos.Y() + glyph->GetHeight());

        CGlPaneGuard GUARD(pane, CGlPane::eOrtho);
        gl.PushMatrix();

        // highlight glyph
        gl.Translatef(0.0, -pane.GetOffsetY(), 0.0f);
        gl.Color4f(1.0f, 0.0f, 0.0f, 0.4f);
        TModelUnit band_w = 6.0;
        TModelUnit band_w_x = ctx->ScreenToSeq(band_w);
        TModelUnit x_left = glyph_rect.Left() - ctx->GetOffset();
        TModelUnit x_right = glyph_rect.Right() - ctx->GetOffset();

        CRgbaColor color1(1.0f, 0.0f, 0.0f, 0.5f);
        CRgbaColor color2(1.0f, 0.0f, 0.0f, 0.0f);
        gl.ShadeModel(GL_SMOOTH);
        gl.Begin(GL_QUADS);
        gl.ColorC(color1);
        gl.Vertex3d(x_left, glyph_rect.Top(), 0.0);
        gl.Vertex3d(x_right, glyph_rect.Top(), 0.0);
        gl.ColorC(color2);
        gl.Vertex3d(x_right, glyph_rect.Top() - band_w, 0.0);
        gl.Vertex3d(x_left, glyph_rect.Top() - band_w, 0.0);
        gl.ColorC(color1);
        gl.Vertex3d(x_right, glyph_rect.Bottom(), 0.0);
        gl.Vertex3d(x_left, glyph_rect.Bottom(), 0.0);
        gl.ColorC(color2);
        gl.Vertex3d(x_left, glyph_rect.Bottom() + band_w, 0.0);
        gl.Vertex3d(x_right, glyph_rect.Bottom() + band_w, 0.0);

        gl.ColorC(color1);
        gl.Vertex3d(x_left, glyph_rect.Bottom(), 0.0);
        gl.Vertex3d(x_left, glyph_rect.Top(), 0.0);
        gl.ColorC(color2);
        gl.Vertex3d(x_left - band_w_x, glyph_rect.Top(), 0.0);
        gl.Vertex3d(x_left - band_w_x, glyph_rect.Bottom(), 0.0);
        gl.ColorC(color1);
        gl.Vertex3d(x_right, glyph_rect.Bottom(), 0.0);
        gl.Vertex3d(x_right, glyph_rect.Top(), 0.0);
        gl.ColorC(color2);
        gl.Vertex3d(x_right + band_w_x, glyph_rect.Top(), 0.0);
        gl.Vertex3d(x_right + band_w_x, glyph_rect.Bottom(), 0.0);

        gl.ColorC(color1);
        gl.Vertex3d(x_left, glyph_rect.Top(), 0.0);
        gl.ColorC(color2);
        gl.Vertex3d(x_left, glyph_rect.Top() - band_w, 0.0);
        gl.Vertex3d(x_left - band_w_x, glyph_rect.Top() - band_w, 0.0);
        gl.Vertex3d(x_left - band_w_x, glyph_rect.Top(), 0.0);
        gl.ColorC(color1);
        gl.Vertex3d(x_right, glyph_rect.Top(), 0.0);
        gl.ColorC(color2);
        gl.Vertex3d(x_right + band_w_x, glyph_rect.Top(), 0.0);
        gl.Vertex3d(x_right + band_w_x, glyph_rect.Top() - band_w, 0.0);
        gl.Vertex3d(x_right, glyph_rect.Top() - band_w, 0.0);

        gl.ColorC(color1);
        gl.Vertex3d(x_left, glyph_rect.Bottom(), 0.0);
        gl.ColorC(color2);
        gl.Vertex3d(x_left, glyph_rect.Bottom() + band_w, 0.0);
        gl.Vertex3d(x_left - band_w_x, glyph_rect.Bottom() + band_w, 0.0);
        gl.Vertex3d(x_left - band_w_x, glyph_rect.Bottom(), 0.0);
        gl.ColorC(color1);
        gl.Vertex3d(x_right, glyph_rect.Bottom(), 0.0);
        gl.ColorC(color2);
        gl.Vertex3d(x_right + band_w_x, glyph_rect.Bottom(), 0.0);
        gl.Vertex3d(x_right + band_w_x, glyph_rect.Bottom() + band_w, 0.0);
        gl.Vertex3d(x_right, glyph_rect.Bottom() + band_w, 0.0);

        gl.End();

        gl.ShadeModel(GL_FLAT);

        // draw connector between glyph and tooltip
        TModelPoint start_pnt = tip_rect_m.CenterPoint();
        TModelPoint end_pnt = glyph_rect.CenterPoint();
        TModelRange t_h(tip_rect_m.Left(), tip_rect_m.Right());
        TModelRange g_h(glyph_rect.Left(), glyph_rect.Right());
        TModelRange t_v(tip_rect_m.Top(), tip_rect_m.Bottom());
        TModelRange g_v(glyph_rect.Top(), glyph_rect.Bottom());
        TModelRange temp;
        bool no_connector = false;
        if (tip_rect_m.Bottom() < glyph_rect.Top()) {
            // tip obove object
            start_pnt.m_Y = t_v.GetTo();
            end_pnt.m_Y = g_v.GetFrom();
            temp = t_h.IntersectionWith(g_h);
            temp.SetTo(temp.GetTo() - 1.0);
            if (temp.Empty()) {
                start_pnt.m_X = t_h.GetFrom() < g_h.GetFrom() ? t_h.GetTo() : t_h.GetFrom();
                end_pnt.m_X = t_h.GetFrom() < g_h.GetFrom() ? g_h.GetFrom() : g_h.GetTo();
            } else {
                start_pnt.m_X = end_pnt.m_X = temp.GetFrom();
            }
        } else if (tip_rect_m.Top() > glyph_rect.Bottom()) {
            // tip under object
            start_pnt.m_Y = t_v.GetFrom();
            end_pnt.m_Y = g_v.GetTo();
            temp = t_h.IntersectionWith(g_h);
            temp.SetTo(temp.GetTo() - 1.0);
            if (temp.Empty()) {
                start_pnt.m_X = t_h.GetFrom() < g_h.GetFrom() ? t_h.GetTo() : t_h.GetFrom();
                end_pnt.m_X = t_h.GetFrom() < g_h.GetFrom() ? g_h.GetFrom() : g_h.GetTo();
            } else {
                start_pnt.m_X = end_pnt.m_X = temp.GetFrom();
            }
        } else if (tip_rect_m.Left() > glyph_rect.Right()) {
            // tip on the right side
            start_pnt.m_X = t_h.GetFrom();
            end_pnt.m_X = g_h.GetTo();
            temp = t_v.IntersectionWith(g_v);
            temp.SetTo(temp.GetTo() - 1.0);
            if (temp.Empty()) {
                start_pnt.m_Y = t_v.GetFrom() < g_v.GetFrom() ? t_v.GetTo() : t_v.GetFrom();
                end_pnt.m_Y = t_v.GetFrom() < g_v.GetFrom() ? g_v.GetFrom() : g_v.GetTo();
            } else {
                start_pnt.m_Y = end_pnt.m_Y = temp.GetFrom();
            }
        } else if (tip_rect_m.Right() < glyph_rect.Left()) {
            // tip on the left side
            start_pnt.m_X = t_h.GetTo();
            end_pnt.m_X = g_h.GetFrom();
            temp = t_v.IntersectionWith(g_v);
            temp.SetTo(temp.GetTo() - 1.0);
            if (temp.Empty()) {
                start_pnt.m_Y = t_v.GetFrom() < g_v.GetFrom() ? t_v.GetTo() : t_v.GetFrom();
                end_pnt.m_Y = t_v.GetFrom() < g_v.GetFrom() ? g_v.GetFrom() : g_v.GetTo();
            } else {
                start_pnt.m_Y = end_pnt.m_Y = temp.GetFrom();
            }
        } else {
            // overlapping
            no_connector = true;
        }

        if ( !no_connector ) {
            const TModelUnit small_num = 0.000001;
            TModelUnit len_pix = min(TModelUnit(40.0), t_v.GetLength());
            TModelPoint start_pnt1 = start_pnt;
            TModelPoint start_pnt2 = start_pnt;
            TModelUnit dx = ctx->SeqToScreen(start_pnt.m_X - end_pnt.m_X);
            TModelUnit dy = start_pnt.m_Y -  end_pnt.m_Y;
            TModelUnit dist = sqrt(dx*dx + dy*dy);
            TModelUnit off_x = dist < small_num ? len_pix : dy * len_pix / dist;
            TModelUnit off_y = dist < small_num ? 0.0 : dx * len_pix / dist;
            if (fabs(off_x) < small_num) {
                start_pnt1.m_X = start_pnt2.m_X = start_pnt.m_X;
                start_pnt1.m_Y = start_pnt.m_Y - len_pix * 0.5;
                start_pnt2.m_Y = start_pnt.m_Y + len_pix * 0.5;
                TModelUnit shift = 0.0;
                if (start_pnt1.m_Y < t_v.GetFrom()) {
                    shift = t_v.GetFrom() - start_pnt1.m_Y;
                } else if (start_pnt2.m_Y > t_v.GetTo()) {
                    shift = t_v.GetTo() - start_pnt2.m_Y;
                }
                start_pnt1.m_Y += shift;
                start_pnt2.m_Y += shift;
            } else if (fabs(off_y) < small_num) {
                start_pnt1.m_Y = start_pnt2.m_Y = start_pnt.m_Y;
                start_pnt1.m_X = start_pnt.m_X - ctx->ScreenToSeq(len_pix) * 0.5;
                start_pnt2.m_X = start_pnt.m_X + ctx->ScreenToSeq(len_pix) * 0.5;
                TModelUnit shift = 0.0;
                if (start_pnt1.m_X < t_h.GetFrom()) {
                    shift = t_h.GetFrom() - start_pnt1.m_X;
                } else if (start_pnt2.m_X > t_h.GetTo()) {
                    shift = t_h.GetTo() - start_pnt2.m_X;
                }
                start_pnt1.m_X += shift;
                start_pnt2.m_X += shift;
            } else {
                if (dx * dy < 0) {
                    off_y = -off_y;
                    off_x = -off_x;
                }
                start_pnt1.m_X = start_pnt.m_X;
                start_pnt2.m_Y = start_pnt.m_Y;
                start_pnt1.m_Y = start_pnt.m_Y + off_y;
                start_pnt2.m_X = start_pnt.m_X + ctx->ScreenToSeq(off_x);
            }

            gl.Color4f(1.0f, 1.0f, 0.65f, 0.4f);
            ctx->DrawTriangle(start_pnt1.m_X, start_pnt1.m_Y,
                start_pnt2.m_X, start_pnt2.m_Y,
                end_pnt.m_X, end_pnt.m_Y);

            CGlAttrGuard LineGuard(GL_LINE_BIT);
            gl.Enable(GL_LINE_SMOOTH);

            gl.Color4f(0.0f, 0.0f, 0.0f, 0.9f);
            ctx->DrawLine(start_pnt1.m_X, start_pnt1.m_Y, end_pnt.m_X, end_pnt.m_Y);
            ctx->DrawLine(start_pnt2.m_X, start_pnt2.m_Y, end_pnt.m_X, end_pnt.m_Y);
        }

        gl.PopMatrix();

    }
}


void CSeqGraphicPane::x_RenderRectSelHandler(CGlPane& pane)
{
    if (m_MouseMode == eMouse_RectSelect  ||
        m_MouseMode == eMouse_IncRectSelect) {

        IRender& gl = GetGl();

        CGlAttrGuard LineGuard(GL_LINE_BIT);
        CGlPaneGuard GUARD(pane, CGlPane::ePixels);

        gl.LineWidth(1.0f);
        gl.Color4f(0.0f, 0.0f, 0.0f, 1.0f);

        gl.Enable(GL_LINE_STIPPLE);
        gl.LineStipple(1, 0x0F0F);

        int x1 = m_StartPoint.x;
        int y1 = MZHH_GetVPPosByY(m_StartPoint.y);
        int x2 = m_DragPoint.x;
        int y2 = MZHH_GetVPPosByY(m_DragPoint.y);

        if (x2 < x1)
            swap(x1, x2);
        if (y2 < y1)
            swap(y1, y2);

        gl.Begin(GL_LINES);
            gl.Vertex2d(x1, y2);
            gl.Vertex2d(x2, y2);

            gl.Vertex2d(x2, y2);
            gl.Vertex2d(x2, y1);

            gl.Vertex2d(x1, y2);
            gl.Vertex2d(x1, y1);

            gl.Vertex2d(x1, y1);
            gl.Vertex2d(x2, y1);
        gl.End();

        gl.Disable(GL_LINE_STIPPLE);
    }
}


////////////////////////////////////////////////////////////////////////////
/*int CSeqGraphicPane::x_HandleKeyEvent()
{
    IGlEventHandler* pH = NULL; //dummy
    int res = x_Handlers_handle(fArea_All, pH, false);

    if (res == 0  &&  m_Event.GetFLTKEvent() == FL_KEYDOWN) {
        CGlPane& ref_pane = m_Renderer->GetFeatGlPane();
        TModelPoint ref_point = ref_pane.UnProject(Fl::event_x(),
                                MZHH_GetVPPosByY(h() - Fl::event_y()));
        switch(m_Event.GetGUISignal()) {
        case CGUIEvent::eZoomInSignal:  ZoomInPoint(ref_point); break;
        case CGUIEvent::eZoomOutSignal: ZoomOutPoint(ref_point); break;
        case CGUIEvent::eZoomAllSignal: ZoomAll(); break;
        case CGUIEvent::eNextItem:      NextPrevSplice(eDir_Next); break;
        case CGUIEvent::ePrevItem:      NextPrevSplice(eDir_Prev); break;
        default: break;
        }
    }
    return res;
}*/


// Place selected objects into clipboard
void CSeqGraphicPane::OnEditCopy()
{
    /*CClipboard::Instance().Clear();

    // retrieve and process selections
    CScope& scope = m_DS->GetBioseqHandle().GetScope();

    TConstObjects sel_objs;
    GetObjectSelection(sel_objs);

    ITERATE (TConstObjects, it, sel_objs) {
        CClipboard::Instance().Add(SConstScopedObject(**it, scope));
    }

    IClipboardHandler::x_OnCopy();*/
}


bool CSeqGraphicPane::CanGoBack(void) const
{
    return m_BackForwardPos > 0;
}


bool CSeqGraphicPane::CanGoForward(void) const
{
    return m_BackForwardPos < m_BackForwardHistory.size() - 1;
}


void CSeqGraphicPane::GoBack(void)
{
    if (CanGoBack()) {
        TSeqRange range = m_BackForwardHistory[--m_BackForwardPos];
        x_GetParent()->ZoomOnRange(range, 0);
    }
}


void CSeqGraphicPane::GoForward(void)
{
    if (CanGoForward()) {
        TSeqRange range = m_BackForwardHistory[++m_BackForwardPos];
        x_GetParent()->ZoomOnRange(range, 0);
    }
}


void CSeqGraphicPane::SaveCurrentRange()
{
    TSeqRange r_cur = x_GetParent()->GetVisibleSeqRange();
    TSeqRange r_old = m_BackForwardHistory.empty() ? TSeqRange() :
        m_BackForwardHistory[m_BackForwardPos];

    if (r_cur != r_old) {
        // first, empty history from current and up
        if (m_BackForwardPos < m_BackForwardHistory.size()) {
            m_BackForwardHistory.erase
                (m_BackForwardHistory.begin() + m_BackForwardPos + 1,
                 m_BackForwardHistory.end());
        }

        if ( !m_BackForwardHistory.empty() ) {
            // do not save position if it's already there
            if (r_cur != m_BackForwardHistory.back()) {
                m_BackForwardHistory.push_back(r_cur);
            }
        } else {
            m_BackForwardHistory.push_back(r_cur);
        }

        m_BackForwardPos = m_BackForwardHistory.size() - 1;
        x_GetParent()->SetDirty(true);
    }
}


void CSeqGraphicPane::OnSearchTip(const string& tip_id, const wxRect& tip_rect)
{
    // parse the signature to get range
    CSeq_id_Handle idh;
    TSeqPos from = 0;
    TSeqPos to   = 0;
    CObjFingerprint::EObjectType type = CObjFingerprint::eUnknown;
    int subtype = 0;
    Uint4 fingerprint = 0;
    Uint4 ds_fingerprint = 0;
    string ds_name;
    try {
        CObjFingerprint::ParseSignature(tip_id, idh, from, to, type,
            subtype, fingerprint, ds_fingerprint, ds_name, &m_DS->GetScope());
    } catch (CException&) {
    }

    if (idh) {
        TSeqRange obj_r(from, to);
        TSeqRange vis_r = m_Renderer->GetVisibleRange();

        const CGlPane& pane = m_Renderer->GetFeatGlPane();
        const TModelRect& vis_rect = pane.GetVisibleRect();
        const TVPRect& vp_rect = pane.GetViewport();

        // calculate tooltip rect in feature pane space
        wxRect win_rect = GetScreenRect();
        TModelRect tip_rect_m;
        tip_rect_m.SetLeft(m_Renderer->Screen2Seq(tip_rect.x - win_rect.x, 0));
        TModelUnit offset_y = pane.GetOffsetY() - vp_rect.Height();
        tip_rect_m.SetTop(tip_rect.y - win_rect.y -
            (win_rect.GetHeight() - vp_rect.Height()) + offset_y);
        tip_rect_m.SetRight(tip_rect_m.Left() + m_Renderer->Screen2SeqWidth(tip_rect.GetWidth()));
        tip_rect_m.SetBottom(tip_rect_m.Top() + tip_rect.GetHeight());

        TSeqPos min_dist = m_Renderer->Screen2SeqWidth(60);
        bool need_shift = true;
        if (vis_r.IntersectingWith(obj_r)) {
            // get the glyph for the given tip_id
            CWeakRef<CSeqGlyph> glyph_wk = x_GetGlyphByTipId(tip_id);
            CWeakRef<CSeqGlyph>::TRefType glyph = glyph_wk.Lock();

            if (glyph) {
                TModelPoint glyph_pos;
                glyph->GetPosInWorld(glyph_pos);
                TModelRect glyph_rect;
                glyph_rect.SetLeft(glyph_pos.X());
                glyph_rect.SetTop(glyph_pos.Y());
                glyph_rect.SetRight(glyph_pos.X() + glyph->GetWidth());
                glyph_rect.SetBottom(glyph_pos.Y() + glyph->GetHeight());
                if ( !tip_rect_m.Intersects(glyph_rect) ) {
                    need_shift = false;
                }
            } else {
                // we know the target object is located in the visible range,
                // but it is not showing.  There are many possibilities:
                // the track gets collapsed or closed, or the object is
                // packed into a histogram.  We may need to use OnZoomTip
                // in order to show the object if it is in packed form.
                need_shift = false;
                x_GetParent()->ZoomOnRange(obj_r, CSeqGraphicWidget::fAddMargins);
            }
        }

        if (need_shift) {
            // make sure the target object is not completely hidden under
            int shift = 0;
            if (tip_rect_m.Left() + tip_rect_m.Right() < vis_rect.Left() + vis_rect.Right()) {
                TSeqPos target_pos =
                    TSeqPos(min(vis_rect.Right(), tip_rect_m.Right() + min_dist));
                shift = obj_r.GetFrom() - target_pos;
            } else {
                TSeqPos target_pos = TSeqPos(max(vis_rect.Left(), tip_rect_m.Left() - min_dist));
                shift = obj_r.GetTo() - target_pos;
            }
            from = (TSeqPos)(max(0, (int)vis_r.GetFrom() + shift));
            to = (TSeqPos)(max(0, (int)vis_r.GetTo() + shift));
            x_GetParent()->ZoomOnRange(TSeqRange(from, to), CSeqGraphicWidget::fSaveRange);
        }
    }
}


void CSeqGraphicPane::OnZoomTip(const string& tip_id, const wxRect& /*tip_rect*/)
{
    // parse the signature to get range
    CSeq_id_Handle idh;
    TSeqPos from = 0;
    TSeqPos to   = 0;
    CObjFingerprint::EObjectType type = CObjFingerprint::eUnknown;
    int subtype = 0;
    Uint4 fingerprint = 0;
    Uint4 ds_fingerprint = 0;
    string ds_name;
    try {
        CObjFingerprint::ParseSignature(tip_id, idh, from, to, type,
            subtype, fingerprint, ds_fingerprint, ds_name, &m_DS->GetScope());
    } catch (CException&) {
    }

    if (idh) {
        // zoom to range with a margin
        x_GetParent()->ZoomOnRange(TSeqRange(from, to),
            CSeqGraphicWidget::fAddMargins | CSeqGraphicWidget::fSaveRange);
    }
}


void CSeqGraphicPane::OnInfoTip(const string& /*tip_id*/)
{
}


void CSeqGraphicPane::OnTipAdded(const string& tip_id)
{
    CWeakRef<CSeqGlyph>::TRefType last_hit_glyph = m_LastHitGlyph.Lock();
    if (last_hit_glyph) {
        const IObjectBasedGlyph* obj_glyph =
            dynamic_cast<const IObjectBasedGlyph*>(last_hit_glyph.GetPointer());
        if (obj_glyph  &&  m_PinnedTips.count(tip_id) == 0  &&  obj_glyph->GetObject(0)) {
            string track_name = s_GetTrackName(tip_id);
            CRef<CCachedTipHandle> tip_handle(new CCachedTipHandle(track_name, m_DS->GetScope()));
            tip_handle->Add(const_cast<CObject&>(*obj_glyph->GetObject(0)));
            m_PinnedTips[tip_id] = tip_handle;
        }
    }
}


void CSeqGraphicPane::OnTipRemoved(const string& tip_id)
{
    m_PinnedTips.erase(tip_id);
}


// IMouseZoomHandlerHost implementation
TModelUnit CSeqGraphicPane::MZHH_GetScale(EScaleType type)
{
    const CGlPane& VP = x_GetParent()->GetPort();
    switch (type) {
    case eCurrent:   return VP.GetScaleX();
    case eMin: return VP.GetMinScaleX();
    case eMax: return VP.GetZoomAllScaleX();
    default: _ASSERT(false); return -1;
    }
}


void CSeqGraphicPane::MZHH_SetScale(TModelUnit scale, const TModelPoint& point)
{
    x_GetParent()->SetScaleX(scale, point);
}


void CSeqGraphicPane::MZHH_ZoomPoint(const TModelPoint& point, TModelUnit factor)
{
    x_GetParent()->ZoomPoint(point, factor);
}


void CSeqGraphicPane::MZHH_ZoomRect(const TModelRect& rc)
{
    TModelRect vert_bar(rc);
    const TModelRect& rec = m_Renderer->GetFeatGlPane().GetVisibleRect();
    vert_bar.SetTop(rec.Top());
    vert_bar.SetBottom(rec.Bottom());
    x_GetParent()->ZoomRect(vert_bar);
}


void CSeqGraphicPane::MZHH_Scroll(TModelUnit d_x, TModelUnit d_y)
{
    x_GetParent()->Scroll(d_x, d_y);
}


void CSeqGraphicPane::MZHH_EndOp()
{
    // translate the notification to the standard message
    SaveCurrentRange();
    x_GetParent()->NotifyVisibleRangeChanged();
}


TVPUnit CSeqGraphicPane::MZHH_GetVPPosByY(int y) const
{
    return GetClientSize().y - 1  - y;
}


//// ISeqMarkHandlerHost implementation
TModelUnit CSeqGraphicPane::SMHH_GetModelByWindow(int z, EOrientation orient)
{
    return SHH_GetModelByWindow(z, orient);
}


TSeqPos CSeqGraphicPane::SMHH_GetSeqMark() const
{
    // We want the marker handler to track and maintain the marker position
    // since there may be multiple sequence markers.
    return TSeqPos(-1);
}


void CSeqGraphicPane::SMHH_SetSeqMark(TSeqPos mark)
{
    // New sequence marker position changed, update other
    // widgets if necessary
    // max(TSeqPos(0), mark);
    // min(mark, m_DS->GetSequenceLength() - 1);
    CEvent evt(CEvent::eEvent_Message, CViewEvent::eWidgetDataChanged);
    Send(&evt, CEventHandler::eDispatch_Default, ePool_Parent);
}


void CSeqGraphicPane::SMHH_OnReset(const string& id)
{
    x_RemoveMarker(id);
    CEvent evt(CEvent::eEvent_Message, CViewEvent::eWidgetDataChanged);
    Send(&evt, CEventHandler::eDispatch_Default, ePool_Parent);
}


/// ISelHandlerHost implementation
void CSeqGraphicPane::SHH_OnChanged()
{
    x_NotifyWidgetSelChanged();
    Refresh();
}


TModelUnit CSeqGraphicPane::SHH_GetModelByWindow(int z, EOrientation orient)
{
    switch(orient) {
    case eHorz: return m_Renderer->GetRulerGlPane().UnProjectX(z);
    case eVert: return m_Renderer->GetRulerGlPane().UnProjectY(GetClientSize().y - z);
    default:    _ASSERT(false); return -1;
    }
}


TVPUnit CSeqGraphicPane::SHH_GetWindowByModel(TModelUnit z, EOrientation orient)
{
    switch(orient) {
    case eHorz: return m_Renderer->GetRulerGlPane().ProjectX(z);
    case eVert: return GetClientSize().y - m_Renderer->GetRulerGlPane().ProjectY(z);
    default:    _ASSERT(false); return -1;
    }
}


void CSeqGraphicPane::LTH_OnLayoutChanged()
{
    m_Renderer->SetNeedUpdateSelection();
    Refresh();
}


void CSeqGraphicPane::LTH_ZoomOnRange(const TSeqRange& range)
{
    x_GetParent()->ZoomOnRange(range, CSeqGraphicWidget::fSaveRange);
}


void CSeqGraphicPane::LTH_PopupMenu(wxMenu* menu)
{
    SetPopupMenuDisplayed(true);
    PopupMenu(menu);
    SetPopupMenuDisplayed(false);
}


void CSeqGraphicPane::LTH_ConfigureTracksDlg(const string& category)
{
    GetFeaturePanel()->ShowConfigureTracksDlg(category);
}

void CSeqGraphicPane::ResetSearch()
{
    GetFeaturePanel()->ResetSearch();
}

TSeqRange CSeqGraphicPane::FindText(const string &text, bool match_case)
{
    TSeqRange range = s_splitPosOrRange(text);
    if (range.NotEmpty())
        return range;

    auto fm =  GetFeaturePanel()->FindText(text, match_case);
    if (!fm)
        return range;
    
    const CSeq_feat& feat = fm.GetOriginalFeature();

    if (m_Renderer->HasSelectedObjects()) 
    {
        m_Renderer->ResetObjectSelection();
    }
    SelectObject(&feat, true);
    x_NotifyWidgetSelChanged();
    range =  fm.GetTotalRange();
    if (range.GetTo() == range.GetFrom())
        range.SetTo(range.GetTo() + 1);

    TSeqPos margin = TSeqPos(range.GetLength() * 0.15f);
    CSGSequenceDS* pDS = GetDataSource();
    if (pDS) {
        TSeqPos len = pDS->GetSequenceLength();
        range.SetFrom(range.GetFrom() > margin ? range.GetFrom() - margin : 0);
        range.SetTo(range.GetTo() < len - margin ? range.GetTo() + margin : len - 1);
    }

    return range;
}

void CSeqGraphicPane::LTH_PushEventHandler(wxEvtHandler* handler)
{
    PushEventHandler(handler);
}


void CSeqGraphicPane::LTH_PopEventHandler()
{
    PopEventHandler();
}


void CSeqGraphicPane::PreDialogShow()
{
    // temprorarily disable sticky tooltip popup
    SetPopupMenuDisplayed(true);
}


void CSeqGraphicPane::PostDialogShow()
{
    // enable sticky tooltip popup
    SetPopupMenuDisplayed(false);
    DlgOverlayFix();
}

// send to parents an event notifing that selection has changed
void CSeqGraphicPane::x_NotifyWidgetSelChanged()
{
    CEvent evt(CEvent::eEvent_Message, CViewEvent::eWidgetSelectionChanged);
    Send(&evt, ePool_Parent);
}


void CSeqGraphicPane::x_NotifyWidgetRangeChanged()
{
    CEvent evt(CEvent::eEvent_Message, CViewEvent::eWidgetRangeChanged);
    Send(&evt, ePool_Parent);
}


CRef<CLayoutTrack> CSeqGraphicPane::THH_GetLayoutTrack()
{
    CRef<CLayoutTrack> track;
    CWeakRef<CSeqGlyph>::TRefType last_hit_glyph = m_LastHitGlyph.Lock();
    if (last_hit_glyph) {
        track.Reset(dynamic_cast<CLayoutTrack*>(last_hit_glyph.GetPointer()));
    }

    return track;
}


TModelPoint CSeqGraphicPane::THH_GetModelByWindow(const wxPoint& pt)
{
    const CGlPane& pane = m_Renderer->GetFeatGlPane();
    return pane.UnProject(pt.x, MZHH_GetVPPosByY(pt.y));
}


///////////////////////////////////////////////////////////////////////////////
/// ITooltip Implementation
/// TC_NeedTooltip() and TC_GetTooltip() is evrything needed to show toolitps
//bool CSeqGraphicPane::TC_NeedTooltip(const wxPoint& pt)
//{
//    if (!m_DS) return false;
//
//    m_TooltipText = "";
//    CWeakRef<CSeqGlyph>::TRefType last_hit_glyph = m_LastHitGlyph.Lock();
//    if (last_hit_glyph) {
//        const CGlPane& pane = m_Renderer->GetFeatGlPane();
//        TModelPoint pos = pane.UnProject(pt.x, MZHH_GetVPPosByY(pt.y));
//        last_hit_glyph->GetTooltip(pos, m_TooltipText);
//    }
//
//    return m_TooltipText.length() > 0;
//}
//
//
//string CSeqGraphicPane::TC_GetTooltip(const wxRect&)
//{
//    return m_TooltipText;
//}


static string s_GetRandomTipID()
{
    srand(time(NULL));
    return kCommonTipId + NStr::IntToString(rand());
}

string CSeqGraphicPane::TTHH_NeedTooltip(const wxPoint& pt)
{
    m_TooltipInfo.SetTipID("");
    m_TooltipInfo.SetTipText("");
    m_TooltipInfo.SetTitleText("");

    if (m_DS) {
        CWeakRef<CSeqGlyph>::TRefType last_hit_glyph = m_LastHitGlyph.Lock();
        if (last_hit_glyph) {
            string title;
            const CGlPane& pane = m_Renderer->GetFeatGlPane();
            TModelPoint pos = pane.UnProject(pt.x, MZHH_GetVPPosByY(pt.y));
            CIRef<ITooltipFormatter> tooltip = ITooltipFormatter::CreateTooltipFormatter(ITooltipFormatter::eTooltipFormatter_Html);
            if (last_hit_glyph->NeedTooltip(pos, *tooltip, title)) {
                m_TooltipInfo.SetTipText(tooltip->Render());
                m_TooltipInfo.SetTitleText(title);

                // check if any cached ids
                string tip_id = x_GetCachedTipId(last_hit_glyph.GetPointer());
                if (tip_id.empty()) {
                    tip_id = s_GetRandomTipID();
                    m_TooltipInfo.SetToolTipOptions(0);
                }
                m_TooltipInfo.SetTipID(tip_id);
            }
        }
    }

    return m_TooltipInfo.GetTipID();
}


CTooltipInfo CSeqGraphicPane::TTHH_GetTooltip(const wxRect& rect)
{
    if (m_TooltipInfo.GetTipText().empty()) {
        CRef<CSeqGlyph> last_hit_glyph(m_LastHitGlyph.Lock());
        if (last_hit_glyph) {
            const CGlPane& pane = m_Renderer->GetFeatGlPane();
            TModelPoint pos = pane.UnProject(rect.x, MZHH_GetVPPosByY(rect.y));

            try {
                CSeqGlyph* glyph = last_hit_glyph.GetPointer();
                m_TooltipInfo = GUI_AsyncExec(
                    [glyph, pos](ICanceled&)
                    {
                        CTooltipInfo info;
                        string tip_title;
                        CIRef<ITooltipFormatter> tooltip = ITooltipFormatter::CreateTooltipFormatter(ITooltipFormatter::eTooltipFormatter_Html);
                        glyph->GetTooltip(pos, *tooltip, tip_title);
                        info.SetTipText(tooltip->Render());
                        info.SetTitleText(tip_title);

                        const IObjectBasedGlyph* obj_glyph =
                            dynamic_cast<const IObjectBasedGlyph*>(glyph);
                        if (obj_glyph) {
                            string tip_id = obj_glyph->GetSignature();
                            // get parent track title
                            const CSeqGlyph* p = glyph->GetParent();
                            while (p) {
                                const CLayoutTrack* p_track = dynamic_cast<const CLayoutTrack*>(p);
                                if (p_track) {
                                    tip_id += kCommonTipId + p_track->GetFullTitle();
                                    break;
                                }
                                p = p->GetParent();
                            }
                            info.SetTipID(tip_id);
                            info.SetToolTipOptions(CTooltipInfo::ePin |
                                CTooltipInfo::eSearch | CTooltipInfo::eMagnify);
                        }
                        else {
                            info.SetTipID(s_GetRandomTipID());
                            info.SetToolTipOptions(0);
                        }

                        return info;
                    },
                    wxT("Retrieving tooltip..."));
            } NCBI_CATCH("Retrieving tooltip.");
        }
    }
    return m_TooltipInfo;
}


bool CSeqGraphicPane::TTHH_ProcessURL(const wxString & href)
{
    CURLTooltipHandler url_handler(m_DS->GetScope(), *this);
    return url_handler.ProcessURL(href.ToStdString());
}

// ITooltip end


void CSeqGraphicPane::GetFeatureNames(vector<string>& names)
{
    names.clear();
    const CFeatList& feat_list = *CSeqFeatData::GetFeatList();
    ITERATE (CFeatList, iter, feat_list) {
        names.push_back(iter->GetDescription());
    }
}


CRef<CSeqGraphicConfig> CSeqGraphicPane::GetConfig()
{
    return m_ConfigSettings;
}


void CSeqGraphicPane::Update()
{
    Layout();
}


void CSeqGraphicPane::UpdateConfig()
{
    if ( !m_ConfigSettings ) {
        m_ConfigSettings.Reset(new CSeqGraphicConfig(false, 0, "Default"));
    } else {
        m_ConfigSettings->LoadSettings();
    }
    m_Renderer->UpdateConfig(m_ConfigSettings);
    m_Renderer->ConfigureTracks();

    //x_Redraw(true);
    //CEvent evt(eCmdConfigChanged);
    //Send(&evt, CEventHandler::eDispatch_Default, ePool_Parent);
}


TVPPoint CSeqGraphicPane::GetPortSize(void)
{
    const TVPRect& rect = GetViewportRect();
    return TVPPoint(rect.Width(), rect.Height());
}


void CSeqGraphicPane::SetSeqMarker(TModelPoint point)
{
    TModelUnit pos = IsHorizontal() ? point.X() : point.Y();
    x_GetDefaultMarker().SetPos(pos);
    CEvent evt(CEvent::eEvent_Message, CViewEvent::eWidgetDataChanged);
    Send(&evt, CEventHandler::eDispatch_Default, ePool_Parent);
}


void CSeqGraphicPane::RemoveMarker(const string& marker_id)
{
    x_RemoveMarker(marker_id);
    CEvent evt(CEvent::eEvent_Message, CViewEvent::eWidgetDataChanged);
    Send(&evt, CEventHandler::eDispatch_Default, ePool_Parent);
}


void CSeqGraphicPane::AddPointMarker(const string& name,
                                     TSeqPos pos,
                                     const CRgbaColor color)
{
    CRef<CMarker> marker;
    marker.Reset(new CMarker);
    marker->SetMarkerType(CSeqMarkHandler::ePoint);
    marker->SetPos((TModelUnit)pos);

    x_AddNewMarker(marker, name, color);
    CEvent evt(CEvent::eEvent_Message, CViewEvent::eWidgetDataChanged);
    Send(&evt, CEventHandler::eDispatch_Default, ePool_Parent);
}

void CSeqGraphicPane::AddRangeMarker(const string& name, TSeqRange range, const CRgbaColor color)
{
    CRef<CMarker> marker;
    marker.Reset(new CMarker);
    marker->SetMarkerType(CSeqMarkHandler::eRange);
    marker->SetPos((TModelUnit)range.GetFrom());
    marker->SetExtendedPos((TModelUnit)range.GetTo());

    x_AddNewMarker(marker, name, color);
    CEvent evt(CEvent::eEvent_Message, CViewEvent::eWidgetDataChanged);
    Send(&evt, CEventHandler::eDispatch_Default, ePool_Parent);
}

void CSeqGraphicPane::RemoveAllMarkers()
{
    x_ClearMarkers();
    CEvent evt(CEvent::eEvent_Message, CViewEvent::eWidgetDataChanged);
    Send(&evt, CEventHandler::eDispatch_Default, ePool_Parent);
}


size_t CSeqGraphicPane::GetMarkerNum() const
{
    size_t num = 0;
    ITERATE (TSeqMarkers, iter, m_SeqMarkers) {
        if ( !iter->second->IsRemoved() ) {
            ++num;
        }
    }
    return num;
}


string CSeqGraphicPane::GetNewMarkerName()
{
    return kNamedMarker + NStr::NumericToString(m_MarkerId);
}


void CSeqGraphicPane::RenameMarker(const string& id, const string& label)
{
    TSeqMarkers::iterator iter = m_SeqMarkers.find(id);
    if (iter != m_SeqMarkers.end()  &&  !iter->second->IsRemoved()) {
        CRef<CMarker> marker = iter->second;

        // Reset new label
        marker->SetLabel(label);
        if (id == kDefaultMarker  && label != kDefaultMarkerLabel) {
            // Label change on the default marker will make it a non-default marker
            // First remove from the list
            m_SeqMarkers.erase(iter);

            // Set new id
            string marker_id = kNamedMarker + NStr::NumericToString(m_MarkerId++);
            marker->m_Handler.SetId(marker_id);

            // Add it back with new id
            m_SeqMarkers.insert(TSeqMarkers::value_type(marker_id, marker));
        }
        CEvent evt(CEvent::eEvent_Message, CViewEvent::eWidgetDataChanged);
        Send(&evt, CEventHandler::eDispatch_Default, ePool_Parent);
    }
}


void CSeqGraphicPane::ModifyMarker(const string& id, const string& label,
                                   TSeqPos pos, const CRgbaColor color)
{
    CRef<CMarker> marker;
    if (x_ModifyMarker(marker, id, label, color)) {
        marker->SetLabel(label);
        marker->SetPos(pos);
        marker->SetMarkerType(CSeqMarkHandler::ePoint);
 
        CEvent evt(CEvent::eEvent_Message, CViewEvent::eWidgetDataChanged);
        Send(&evt, CEventHandler::eDispatch_Default, ePool_Parent);
    }
}

void CSeqGraphicPane::ModifyMarker(const string& id, const string& label,
                                   TSeqRange range, const CRgbaColor color)
{
    CRef<CMarker> marker;
    if (x_ModifyMarker(marker, id, label, color)) {       
        marker->SetRange(range);      
        marker->SetMarkerType(CSeqMarkHandler::eRange);
 
        CEvent evt(CEvent::eEvent_Message, CViewEvent::eWidgetDataChanged);
        Send(&evt, CEventHandler::eDispatch_Default, ePool_Parent);
    }
}

bool CSeqGraphicPane::x_ModifyMarker(CRef<CMarker>& marker,
                                     const string& id, 
                                     const string& label, 
                                     const CRgbaColor color)
{
    TSeqMarkers::iterator iter = m_SeqMarkers.find(id);
    if (iter != m_SeqMarkers.end()  &&  !iter->second->IsRemoved()) {
        marker = iter->second;
        marker->SetLabel(label);
        marker->SetColor(color);
        if (id == kDefaultMarker  &&  label != kDefaultMarkerLabel) {
            // Label change on the default marker will make it a non-default marker
            // First remove from the list
            m_SeqMarkers.erase(iter);

            // Set the id
            string marker_id = kNamedMarker + NStr::NumericToString(m_MarkerId++);
            marker->m_Handler.SetId(marker_id);

            // Add it back with the new id
            m_SeqMarkers.insert(TSeqMarkers::value_type(marker_id, marker));
        }
        return true;
    }
    return false;
}


void CSeqGraphicPane::SetSeqStartPoint(TModelPoint point)
{
    TModelUnit pos = IsHorizontal() ? point.X() : point.Y();
    SetSeqStart(TSeqPos(pos));
}


void CSeqGraphicPane::SetSeqStart(TSeqPos pos)
{
    m_SeqStart = pos;
    m_Renderer->SetRulerSeqStart(pos);
}


void CSeqGraphicPane::SetSeqStartMarker(const string& id)
{
    CConstRef<CMarker> marker = GetMarker(id);
    if (marker) {
        TSeqPos pos = marker->GetPos();
        SetSeqStart(pos);
    }
}


void CSeqGraphicPane::ResetSeqStart()
{
    SetSeqStart(0);
}


// wuliangs: move this to switch point track
void CSeqGraphicPane::SetSwitchPoint()
{
    x_ChangeSwitchPoint();
}


string CSeqGraphicPane::GetHitMarker() const
{
    ITERATE(TSeqMarkers, iter, m_SeqMarkers) {
        if (iter->second->IsRemoved()) continue;

        if (iter->second->m_Handler.HitMe()) {
            return iter->first;
        }
    }
    return kEmptyStr;
}


CConstRef<CMarker> CSeqGraphicPane::GetMarker(const string& id)
{
    CConstRef<CMarker> marker;
    TSeqMarkers::iterator iter = m_SeqMarkers.find(id);
    if (iter != m_SeqMarkers.end()  &&  !iter->second->IsRemoved()) {
        marker.Reset(iter->second.GetPointer());
    }
    return marker;
}


bool CSeqGraphicPane::CanZoomSelection(void)
{
    return ! m_SelHandler.GetSelection().empty();
}


void CSeqGraphicPane::ZoomObject(void)
{
    CConstRef<CSeqGlyph> sel_glyph = x_GetOnlyOneSelectedGlyph();
    if (sel_glyph) {
        x_GetParent()->ZoomOnRange(sel_glyph->GetRange(),
            CSeqGraphicWidget::fAddMargins | CSeqGraphicWidget::fSaveRange);
    }
}


void CSeqGraphicPane::NextPrevSplice(EDirection dir)
{
    CConstRef<CSeqGlyph> sel_glyph = x_GetOnlyOneSelectedGlyph();
    const IObjectBasedGlyph* obj =
        dynamic_cast<const IObjectBasedGlyph*>(sel_glyph.GetPointer());

    if ( !obj ) {
        return;
    }

    const IObjectBasedGlyph::TIntervals& intervals = obj->GetIntervals();
    if (intervals.size() == 0)
        return;

    // Get the default marker
    CSeqMarkHandler& seq_marker = x_GetDefaultMarker();
    TSeqPos marker_pos = seq_marker.GetPos();

    // (for a minus strand object, next is to the left, not to the right)
    bool neg_strand = sequence::GetStrand(obj->GetLocation()) == eNa_strand_minus;
    bool reverse = neg_strand ^ m_Flipped;
    bool is_next = dir == eDir_Next  ||
        (dir == eDir_Left  &&  reverse)  ||  (dir == eDir_Right  &&  !reverse);

    // first, find the nearest interval end following the direction.
    TSeqPos pre = INT_MAX;
    TSeqPos next = 0;
    TSeqPos near_dist = INT_MAX;
    TSeqPos near_val = 0;
    ITERATE (IObjectBasedGlyph::TIntervals, iter, intervals) {
        TSeqPos f = iter->GetFrom();
        TSeqPos t = iter->GetTo();
        TSeqPos t_dist = marker_pos > t ? marker_pos - t : t - marker_pos;
        if (near_dist > t_dist) {
            near_val = t;
            near_dist = t_dist;
        }
        TSeqPos f_dist = marker_pos > f ? marker_pos - f : f - marker_pos;
        if (near_dist > f_dist) {
            near_val = f;
            near_dist = f_dist;
        }

        if (marker_pos >= f  &&  marker_pos <= t) {
            pre = f;
            next = t;
            break;
        }

        if (marker_pos < f) {
            next = f;
            if (!neg_strand) break;
        }

        if (marker_pos > t) {
            pre = t;
            if (neg_strand) break;
        }
    }
    if (pre == INT_MAX) pre = near_val;
    if (next == 0) next = near_val;

    // now do the next/prev thing
    if (marker_pos == near_val) {  // already at the interval end
        ITERATE (IObjectBasedGlyph::TIntervals, iter, intervals) {
            TSeqPos f = iter->GetFrom();
            TSeqPos t = iter->GetTo();
            bool have_next = (iter + 1) != intervals.end();
            bool have_prev = iter != intervals.begin();

            if (is_next) { // Next exon
                if (marker_pos == f  &&  f == t  &&  have_next) {
                    marker_pos = (iter + 1)->GetFrom();
                    break;
                } else if (marker_pos == f) {
                    if (!neg_strand) {
                        marker_pos = t;
                    } else if (have_next) {
                        marker_pos = (iter + 1)->GetTo();
                    }
                    break;
                } else if (marker_pos == t) {
                    if (neg_strand) {
                        marker_pos = f;
                    } else if (have_next) {
                        marker_pos = (iter + 1)->GetFrom();
                    }
                    break;
                }
            } else {// Prev. exon
                if (marker_pos == t  &&  f == t  &&  have_prev) {
                    marker_pos = (iter - 1)->GetTo();
                } else if (marker_pos == t) {
                    if (!neg_strand) {
                        marker_pos = f;
                    } else if (have_prev) {
                        marker_pos = (iter - 1)->GetFrom();
                    }
                    break;
                } else if (marker_pos == f) {
                    if (neg_strand) {
                        marker_pos = t;
                    } else if (have_prev) {
                        marker_pos = (iter - 1)->GetTo();
                    }
                    break;
                }
            }
        }  // ITERATE
    } else {  // go to interval end
        if (is_next) {
            if (neg_strand) marker_pos = pre;
            else marker_pos = next;
        } else {
            if (neg_strand) marker_pos = next;
            else marker_pos = pre;
        }
    }

    // Move the visible center only when the current marker is outside the visible range
    TSeqRange vis_r = x_GetParent()->GetVisibleSeqRange();
    if (vis_r.GetFrom() > marker_pos  ||  vis_r.GetTo() < marker_pos) {
        TSeqPos center_x = (TSeqPos)((vis_r.GetTo() + vis_r.GetFrom()) * 0.5f);
        TSeqRange range(vis_r.GetFrom() + (marker_pos - center_x),
            vis_r.GetTo() + (marker_pos - center_x));
        x_GetParent()->ZoomOnRange(range, CSeqGraphicWidget::fSaveRange);
    }

    seq_marker.SetPos((TModelUnit)marker_pos);

    CEvent evt(CEvent::eEvent_Message, CViewEvent::eWidgetDataChanged);
    Send(&evt, CEventHandler::eDispatch_Default, ePool_Parent);
    Refresh();
}


int CSeqGraphicPane::GetPopupArea(wxPoint pos)
{
    int x = pos.x;
    int y = MZHH_GetVPPosByY(pos.y);

    int area = x_GetAreaByVPPos(x, y);

    // do we have context menu specific to feaature or alignment?
    /*
    if (area == fArea_Object) {
        CConstRef<CSeqGlyph> glyph = x_GetOnlyOneSelectedGlyph();
        if (glyph) {
            if (dynamic_cast<const CAlignGlyph*>(glyph.GetPointer())) {
                area = fArea_Alignment;
            } else if (dynamic_cast<const CFeatGlyph*>(glyph.GetPointer())) {
                area = fArea_Feature;
            }
        }
    }
    */

    return area;
}


bool CSeqGraphicPane::CanSetSwitchPoint()
{
    CConstRef<CSeqGlyph> sel_glyph = x_GetOnlyOneSelectedGlyph();
    const CSwitchPointGlyph* sp =
        dynamic_cast<const CSwitchPointGlyph*>(sel_glyph.GetPointer());
    if ( !sp || !x_HasDefaultMarker())  return false;

    TSeqPos marker_pos = x_GetDefaultMarker().GetPos();
    TSeqRange range = sp->GetLocation().GetTotalRange();
    return ( marker_pos >= range.GetFrom()  &&  marker_pos <= range.GetTo() );
}


static string s_EscapeCommaAndPipe(const string& str)
{
    string out = kEmptyStr;
    const size_t str_l = str.size();
    out.reserve(str_l + 2);
    for (size_t i = 0; i < str_l; ++i) {
        const char& curr_char = str[i];
        if (curr_char == ',' || curr_char == '|') {
            out += "\\";
        }
        out.append(1, curr_char);
    }
    return out;
}


string CSeqGraphicPane::GetMarkers() const
{
    string str;
    bool first = true;
    ITERATE(TSeqMarkers, iter, m_SeqMarkers) {
        if (iter->second->IsRemoved()) continue;
        if ( !first ) {
            str += ",";
        } else {
            first = false;
        }
        const CSeqMarkHandler& marker = iter->second->m_Handler;
        str += NStr::NumericToString(marker.GetPos());
        if (marker.GetMarkerType() == CSeqMarkHandler::eRange) {            
            str += ":" + NStr::NumericToString(marker.GetExtendedPos());
        }
        str += "|";
        str += s_EscapeCommaAndPipe(marker.GetLabel() == "" ? " " : marker.GetLabel()) + "|";
        str += marker.GetColor().ToString(false);
    }
    return str;
}


void CSeqGraphicPane::SetMarkers(const string& markers)
{
    typedef vector<string> TTokens;
    TTokens m_tokens;
    CTrackUtils::TokenizeWithEscape(markers, ",", m_tokens, true);
    ITERATE(TTokens, m_iter, m_tokens) {
        TTokens e_tokens;
        CTrackUtils::TokenizeWithEscape(*m_iter, "|", e_tokens, true);
        size_t e_size = e_tokens.size();
        if (e_size == 3) {
            try {
                CRgbaColor color(e_tokens[2]);

                // Could be a point marker (single number) or range (#:#)
                vector<string> arr;
                NStr::Split(e_tokens[0], ":", arr);
                if (arr.size() == 1) {
                    TSeqPos pos = NStr::StringToSizet(arr[0]);
                    AddPointMarker(NStr::TruncateSpaces(e_tokens[1]), pos, color);
                }
                else if (arr.size() == 2) {
                    TSeqRange range(NStr::StringToSizet(arr[0]), 
                                    NStr::StringToSizet(arr[1]));
                    AddRangeMarker(NStr::TruncateSpaces(e_tokens[1]), range, color);
                }
            } catch (CException&) {
                // invalid marker position, ignore it
            }
        }
    }
    CEvent evt(CEvent::eEvent_Message, CViewEvent::eWidgetDataChanged);
    Send(&evt, CEventHandler::eDispatch_Default, ePool_Parent);
}


void CSeqGraphicPane::x_OnPurgeMarkers(CEvent*)
{
    TSeqMarkers::iterator iter = m_SeqMarkers.begin();
    while (iter != m_SeqMarkers.end()) {
        if (iter->second->IsRemoved()) {
            x_UnregisterHandler(&iter->second->m_Handler);
            m_SeqMarkers.erase(iter++);
        }
        else {
            ++iter;
        }
    }
}


int CSeqGraphicPane::x_GetAreaByVPPos(int vp_x, int vp_y)
{
    const TVPRect& rc_feat  = m_Renderer->GetFeatGlPane().GetViewport();
    const TVPRect& rc_ruler = m_Renderer->GetRulerGlPane().GetViewport();

    // Deside where the mouse is:
    if (rc_ruler.PtInRect(vp_x, vp_y)) {
        return fArea_Ruler;
    } else if (rc_feat.PtInRect(vp_x, vp_y)) {
        return fArea_Object;
    }

    return fArea_Other;
}


CConstRef<CSeqGlyph> CSeqGraphicPane::x_GetOnlyOneSelectedGlyph()
{
    CConstRef<CSeqGlyph> glyph;
    const CSeqGlyph::TConstObjects& sel_glyphs =
        m_Renderer->GetSelectedLayoutObjects();

    if (sel_glyphs.size() == 1) {
        glyph = sel_glyphs.front();
    }

    return glyph;
}


CSeqMarkHandler& CSeqGraphicPane::x_GetDefaultMarker()
{
    TSeqMarkers::iterator iter = m_SeqMarkers.find(kDefaultMarker);
    if (iter == m_SeqMarkers.end()) {
        CRef<CMarker> marker(new CMarker);
        iter = m_SeqMarkers.insert(TSeqMarkers::value_type(kDefaultMarker, marker)).first;
        marker->m_Handler.SetHost(this);
        marker->m_Handler.SetId(kDefaultMarker);
        marker->m_Handler.SetLabel(kDefaultMarkerLabel);
        x_RegisterHandler(&marker->m_Handler, fArea_Ruler,
            &m_Renderer->GetRulerGlPane(), m_MarkerHandlerIndex);
    } else if (iter->second->IsRemoved()) {
        iter->second->SetPos(0);
        iter->second->SetRemoved(false);
        x_RegisterHandler(&iter->second->m_Handler, fArea_Ruler,
            &m_Renderer->GetRulerGlPane(), m_MarkerHandlerIndex);
    }
    return iter->second->m_Handler;
}

void CSeqGraphicPane::x_AddNewMarker(CRef<CMarker> marker,
                                     const string& label,
                                     const CRgbaColor& color)
{
    //x_PurgeMarkers();
    marker->m_Handler.SetHost(this);
    marker->SetColor(color);
    marker->SetLabel(label);
    marker->SetFlipped(m_Flipped);
    string marker_id;
    if (label == kDefaultMarkerLabel  &&  !x_HasDefaultMarker()) {
        marker_id = kDefaultMarker;
    } else {
        marker_id = kNamedMarker + NStr::NumericToString(m_MarkerId);
        m_MarkerId++;
    }
    marker->m_Handler.SetId(marker_id);
    m_SeqMarkers.insert(TSeqMarkers::value_type(marker_id, marker));
    x_RegisterHandler(&marker->m_Handler, fArea_Ruler,
        &m_Renderer->GetRulerGlPane(), m_MarkerHandlerIndex);
}

void CSeqGraphicPane::x_RemoveMarker(const string& id)
{
    TSeqMarkers::iterator iter = m_SeqMarkers.find(id);
    if (iter != m_SeqMarkers.end()) {
        // maker is marked as removed, but not ready to be removed
        // from the m_SeqMarkers list yet
        iter->second->SetRemoved(true);
        Post(CRef<CEvent>(new CSeqGraphicPaneEvent()));
    }
}


bool CSeqGraphicPane::x_HasDefaultMarker() const
{
    TSeqMarkers::const_iterator iter = m_SeqMarkers.find(kDefaultMarker);
    return iter != m_SeqMarkers.end()  &&  !iter->second->IsRemoved();
}


void CSeqGraphicPane::x_ClearMarkers()
{
    NON_CONST_ITERATE (TSeqMarkers, iter, m_SeqMarkers) {
        x_UnregisterHandler(&iter->second->m_Handler);
    }
    m_SeqMarkers.clear();
}

void CSeqGraphicPane::OnContextMenu(wxContextMenuEvent& event)
{
    CheckOverlayTimer();

    while (true) {
        if (!m_DS)
            break;
        wxPoint pos = ScreenToClient(event.GetPosition());
        pos.y = MZHH_GetVPPosByY(pos.y);
        int area = x_GetAreaByVPPos(pos.x, pos.y);
        if (area != fArea_Object)
            break;
        CRef<CSeqGlyph> obj = m_Renderer->HitTest(pos.x, pos.y);
        if (!obj)
            break;
        if (!obj->OnContextMenu(event))
            break;
        return;
    }
    event.Skip();
    return;

}

void CSeqGraphicPane::OnKillFocus(wxFocusEvent& event)
{
    CFeaturePanel* featurePanel = GetFeaturePanel();
    if (featurePanel)
        featurePanel->SaveTrackConfig();
}

END_NCBI_SCOPE
