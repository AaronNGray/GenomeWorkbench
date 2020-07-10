/*  $Id: layout_track_handler.cpp 42358 2019-02-07 17:42:41Z katargir $
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
 * Authors:  Liangshou Wu
 *
 */


#include <ncbi_pch.hpp>
#include <gui/widgets/seq_graphic/layout_track_handler.hpp>
#include <gui/widgets/seq_graphic/rendering_ctx.hpp>
#include <gui/widgets/seq_graphic/track_container_track.hpp>
#include <gui/opengl/irender.hpp>

#include <gui/widgets/gl/attrib_menu.hpp>


BEGIN_NCBI_SCOPE

BEGIN_EVENT_TABLE(CLayoutTrackHandler, wxEvtHandler)
    EVT_KEY_UP(CLayoutTrackHandler::OnKeyEvent)
    EVT_KEY_DOWN(CLayoutTrackHandler::OnKeyEvent)
    EVT_MOUSEWHEEL(CLayoutTrackHandler::OnOtherMouseEvents)
    EVT_RIGHT_DOWN(CLayoutTrackHandler::OnOtherMouseEvents)
    EVT_RIGHT_UP(CLayoutTrackHandler::OnOtherMouseEvents)
    EVT_MIDDLE_DOWN(CLayoutTrackHandler::OnOtherMouseEvents)
    EVT_MIDDLE_UP(CLayoutTrackHandler::OnOtherMouseEvents)

    EVT_LEFT_DOWN(CLayoutTrackHandler::OnLeftDown)
    EVT_LEFT_UP(CLayoutTrackHandler::OnLeftUp)
    EVT_LEFT_DCLICK(CLayoutTrackHandler::OnLeftDblClick)
    EVT_MOTION(CLayoutTrackHandler::OnMotion)
    EVT_MOUSE_CAPTURE_LOST(CLayoutTrackHandler::OnMouseCaptureLost)
END_EVENT_TABLE()


void CLayoutTrackHandler::Render(CGlPane& pane)
{
    if (m_State == eMove  &&  m_LastHitTrack) {
        IRender& gl = GetGl();
        CGlPaneGuard GUARD(pane, CGlPane::eOrtho);
        gl.PushMatrix();

        const CRenderingContext* ctx = m_LastHitTrack->GetRenderingContext();
        gl.Translatef(m_PreMousePos.X() - m_StartPos.X(), -pane.GetOffsetY(), 0.0f);
        TModelRect rc = m_LastHitTrack->GetModelRect();

        gl.Enable(GL_BLEND);
        gl.BlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        if (m_DropTarget)
            gl.Color4f(0.0f, 0.0f, 1.0f, 0.2f);
        else
            gl.Color4f(1.0f, 1.0f, 1.0f, 0.2f);

        ctx->DrawQuad(rc);

#ifndef USE_METAL
        gl.Disable(GL_BLEND);
        gl.ColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_TRUE);
        gl.Color4f(0.0f, 0.0f, 0.0f, 0.5f);

        ctx->DrawQuad(rc);

        gl.ColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_FALSE);
        gl.Enable(GL_BLEND);
        gl.BlendFunc(GL_DST_ALPHA, GL_ONE_MINUS_DST_ALPHA);
#endif

        m_LastHitTrack->Draw();

        gl.ColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
        gl.BlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

        gl.PopMatrix();
    }
}


void CLayoutTrackHandler::OnKeyEvent(wxKeyEvent& event)
{
    if (m_State == eMove) {
        TModelPoint pos = m_Host->THH_GetModelByWindow(event.GetPosition());
        x_Move(pos);
        x_StopMoving();
        GetGenericHost()->GHH_Redraw();
    }
#ifdef ATTRIB_MENU_SUPPORT
    else {
        // Force redraw on key updates so that menu changes
        // in attribute (debug) menu leads to redraw.
        GetGenericHost()->GHH_Redraw();
    }
#endif
    event.Skip();
}


void CLayoutTrackHandler::OnOtherMouseEvents(wxMouseEvent& event)
{
    if (m_State == eMove) {
        TModelPoint pos = m_Host->THH_GetModelByWindow(event.GetPosition());
        x_Move(pos);
        x_StopMoving();
        GetGenericHost()->GHH_Redraw();
    }
    event.Skip();
}


void CLayoutTrackHandler::OnLeftDown(wxMouseEvent& event)
{
    m_PreMousePos = m_Host->THH_GetModelByWindow(event.GetPosition());
    CRef<CLayoutTrack> track = m_Host->THH_GetLayoutTrack();
    bool consumed = false;

    if(m_LastHitTrack  &&  m_LastHitTrack.GetPointer() != track) {
        m_LastHitTrack->MouseOut();
        m_LastHitTrack.Reset(NULL);
    }

    if(track) {
        // honor track-specific left-down event handler first
        if (track->OnLeftDown(m_PreMousePos)) {
            consumed = true;
        } else if (m_Draggable && track->IsMovable()  &&  track->SetParent()) {
            CSeqGlyph* grand_p_glyph = track->SetParent()->SetParent();
            CTrackContainer* p_track =
                dynamic_cast<CTrackContainer*>(grand_p_glyph);
            if (p_track  &&  p_track->GetChildren().size() > 1) {
                // prepare to move the track
                consumed = true;
                m_State = eReadyToMove;
            }
        }
    }

    if (consumed) {
        m_LastHitTrack.Reset(track);
    } else {
        m_LastHitTrack.Reset();
    }

    event.Skip(!consumed);
}


void CLayoutTrackHandler::OnLeftUp(wxMouseEvent& event)
{
    bool consumed = false;
    if (m_LastHitTrack) {
        if (m_State == eMove) {
            TModelPoint pos = m_Host->THH_GetModelByWindow(event.GetPosition());
            x_Move(pos);
            x_StopMoving();
            GetGenericHost()->GHH_Redraw();
        } else if (m_State == eReadyToMove) {
            m_State = eIdle;
        }
        consumed = true;
        //m_LastHitTrack.Reset();
    }
    event.Skip(!consumed);
}


void CLayoutTrackHandler::OnLeftDblClick(wxMouseEvent& event)
{
    m_PreMousePos = m_Host->THH_GetModelByWindow(event.GetPosition());
    CRef<CLayoutTrack> track = m_Host->THH_GetLayoutTrack();
    if (track  &&  track->OnLeftDblClick(m_PreMousePos)) {
        GetGenericHost()->GHH_Redraw();
        m_LastHitTrack.Reset(track);
        return;
    }
    m_LastHitTrack.Reset();
    event.Skip();
}


void CLayoutTrackHandler::OnMotion(wxMouseEvent& event)
{
    TModelPoint pos = m_Host->THH_GetModelByWindow(event.GetPosition());
    bool need_redraw = false;
    bool skip = true;
    if (m_State == eMove) {
        x_Move(pos);
        need_redraw = true;
        skip = false;
    } else if (m_State == eReadyToMove) {
        x_StartMoving();
        x_Move(pos);
        skip = false;
        need_redraw = true;
    } else {
        CRef<CLayoutTrack> track = m_Host->THH_GetLayoutTrack();
        // try to limit the refresh call to only when mouse in,
        // mouse out, or mouse over the icons.
        if (m_LastHitTrack != track) {
            if (m_LastHitTrack) m_LastHitTrack->MouseOut();
            m_LastHitTrack.Reset(track);
            need_redraw = true;
        }
        if (track  &&  track->OnMotion(pos)) need_redraw = true;

        //if(m_LastHitTrack  &&  m_LastHitTrack.GetPointer() != track) {
        //    m_LastHitTrack->MouseOut();
        //    m_LastHitTrack.Reset(NULL);
        //    need_redraw = true;
        //}
        //if (track) {
        //    track->OnMotion(pos);
        //    m_LastHitTrack.Reset(track);
        //    need_redraw = true;
        //}
    }
    if (need_redraw) {
        GetGenericHost()->GHH_Redraw();
    }
    event.Skip(skip);
}

void CLayoutTrackHandler::OnMouseCaptureLost(wxMouseCaptureLostEvent& event)
{
    if (m_LastHitTrack  &&  m_State == eMove) {
        x_StopMoving();
    } else {
        event.Skip();
    }
}


/// A dummy layout track for showing an empty area.
class CDummyTrack : public CGlyphContainer
{
public:
    CDummyTrack(CRenderingContext* r_cntx)
        : CGlyphContainer(r_cntx)
    {
        TTrackAttrFlags attr = fShowAlways | fFullTrack;
        SetTrackAttr(attr);

        CRef<CBoundaryParams> params(
            new CBoundaryParams(true, true, CRgbaColor(0.3f, 0.3f, 1.0f),
            CRgbaColor(66, 160, 255, 0.25 * 255),
            //CRgbaColor(0.5f, 0.5f, 1.0f, 0.2f), 
            2.0));
        SetGroup().SetConfig(params);
    }

    virtual void Update(bool /*layout_only*/)
    {
        // always shown
        SetExpanded(true);
        SetShow(true);
        x_UpdateBoundingBox();
    }

protected:
    virtual void x_UpdateBoundingBox()
    {
        SetGroup().SetTop(0.0);
        SetGroup().SetLeft(GetLeft());
        SetGroup().SetWidth(GetWidth());
        SetGroup().SetHeight(GetHeight());
    }
};


void CLayoutTrackHandler::x_StartMoving()
{
    _ASSERT(m_LastHitTrack);
    CLayoutTrack* track = m_LastHitTrack.GetPointer();
    CTrackContainer* p_track =
        dynamic_cast<CTrackContainer*>(track->SetParent()->SetParent());
    _ASSERT(p_track);

    m_State = eMove;
    int order = track->GetOrder();
    /*
    CTrackContainer* cont = dynamic_cast<CTrackContainer*>(track);
    if (cont && cont->GetChildren().size() > 1) {
        track = dynamic_cast<CLayoutTrack*>(cont->SetChildren().back().GetPointer());
        m_LastHitTrack.Reset(track);
        m_LastHitTrack->SetOrder(++order);
        int ord = order + 1;
        CTrackContainer::TTrackProxies::iterator iter = p_track->GetSubtrackProxies().begin();
        while (iter != p_track->GetSubtrackProxies().end() && order > (*iter)->GetOrder()) {
            (*iter)->SetOrder(ord);
            ++ord;
            ++iter;
        }
    }
    */
    if (!m_Delegate) {
        CRenderingContext* context =
            const_cast<CRenderingContext*>(m_LastHitTrack->GetRenderingContext());
        m_Delegate.Reset(new CDummyTrack(context));
    }
    m_Delegate->SetHeight(track->GetHeight());
    m_Delegate->SetWidth(track->GetWidth() * 0.99);
    m_Delegate->SetLeft(track->GetLeft() + track->GetWidth() * 0.005);

    // get pos in the world coord before remove it
    TModelPoint pos_world;
    track->GetPosInWorld(pos_world); 
    
    // save track state
    // make sure we don't lost track states such as expand/collapse
    //p_track->SaveTrackStates(track->GetOrder());
    //p_track->SetTrack(m_Delegate, track->GetOrder());

    p_track->SaveTrackStates(order);
    p_track->SetTrack(m_Delegate, order);

    track->SetParent(NULL);
    track->SetParentTrack(NULL);
    m_Delegate->Update(true);
    p_track->SetGroup().UpdateLayout();

    track->SetPos(pos_world);
    track->SetParent(NULL);
    m_StartPos = m_PreMousePos;
    GetGenericHost()->GHH_CaptureMouse();
}


void CLayoutTrackHandler::x_StopMoving()
{
    CRef<CTrackContainer> p_track(dynamic_cast<CTrackContainer*>(m_Delegate->SetParent()->SetParent()));
    _ASSERT(p_track);
    int order = m_Delegate->GetOrder();

    m_Delegate->SetParent(NULL);
    m_Delegate->SetParentTrack(NULL);
    m_Delegate.Reset(0);
    bool set_track = true;
    if (m_DropTarget) {
        try {
            m_LastHitTrack->SetOrder(order);
            set_track = !m_DropTarget->Drop(m_LastHitTrack);
        } catch (exception& e) {
            ERR_POST(Error << e.what());
        }
        m_DropTarget = 0;
    } 
    if (set_track)
        p_track->SetTrack(m_LastHitTrack, order);
    else
        p_track->RemoveTrack(order);
    p_track->SetGroup().UpdateLayout();
    m_State = eIdle;
    GetGenericHost()->GHH_ReleaseMouse();
}


void CLayoutTrackHandler::x_Move(const TModelPoint& pos)
{
    TModelUnit delta_y = pos.Y() - m_PreMousePos.Y();
    if (delta_y == 0) {
        return;
    }
    m_PreMousePos = pos;
    m_DropTarget = 0;
    TModelUnit top = m_LastHitTrack->GetTop();
    if (delta_y < 0) {
        x_MoveTrackUpRecursive(-delta_y, top);
    } else {
        x_MoveTrackDownRecursive(delta_y, top);
    }
    m_LastHitTrack->SetTop(m_LastHitTrack->GetTop() + delta_y);
}


void CLayoutTrackHandler::x_MoveTrackUpRecursive(TModelUnit delta_y, TModelUnit& top)
{
    if (delta_y == 0)
        return;
    CTrackContainer* p_track = dynamic_cast<CTrackContainer*>(
        m_Delegate->SetParent()->SetParent());
    _ASSERT(p_track);
    CLayoutGroup& group = p_track->SetGroup();

    TModelPoint pos_w;
    m_Delegate->GetPosInWorld(pos_w);
    int idx_curr = group.GetIndex(m_Delegate.GetPointer());
    int min_movable_track = idx_curr;
    for (; min_movable_track >= 0; --min_movable_track) {
        CConstRef<CSeqGlyph> child_glyph = group.GetChild(min_movable_track - 1);
        const CLayoutTrack* track = dynamic_cast<const CLayoutTrack*>(child_glyph.GetPointer());
        if ( !track  ||  !track->IsMovable()) {
            break;
        }
    }
    
    if (idx_curr != min_movable_track) {
        CRef<CSeqGlyph> glyph_above = group.GetChild(idx_curr - 1);
        TModelPoint pos_w_above;
        glyph_above->GetPosInWorld(pos_w_above);
        TModelUnit will_be_y = top - delta_y;
        TModelUnit critical_y = (pos_w.Y() + pos_w_above.Y()) * 0.5;
        IDroppable* droppable = dynamic_cast<IDroppable*>(glyph_above.GetPointer());
        m_DropTarget = 0;
        if (droppable && droppable->CanDrop(m_LastHitTrack)) {
            if (will_be_y < pos_w.Y() + pos_w_above.Y()) {
                m_DropTarget = droppable;
                critical_y = pos_w_above.Y();
            }
        } 

        if (will_be_y < critical_y) {
            m_DropTarget = 0;
            top = critical_y;
            if ((int)(critical_y - will_be_y) == delta_y)
                return;
            delta_y = (int)(critical_y - will_be_y);
            p_track->MoveUp(m_Delegate->GetOrder());
            group.UpdateLayout();
            x_MoveTrackUpRecursive(delta_y, top);
        } 
    }
}


void CLayoutTrackHandler::x_MoveTrackDownRecursive(TModelUnit delta_y, TModelUnit& top)
{
    CTrackContainer* p_track = dynamic_cast<CTrackContainer*>(
        m_Delegate->SetParent()->SetParent());
    _ASSERT(p_track);
    CLayoutGroup& group = p_track->SetGroup();

    TModelPoint pos_w;
    m_Delegate->GetPosInWorld(pos_w);
    int idx_curr = group.GetIndex(m_Delegate.GetPointer());
    int max_movable_track = idx_curr;
    for (; (unsigned)max_movable_track < group.GetChildrenNum(); ++max_movable_track) {
        CConstRef<CSeqGlyph> child_glyph = group.GetChild(max_movable_track + 1);
        const CLayoutTrack* track = dynamic_cast<const CLayoutTrack*>(child_glyph.GetPointer());
        if ( !track  ||  !track->IsMovable()) {
            break;
        }
    }

    if (idx_curr != max_movable_track) {
        CRef<CSeqGlyph> glyph_below = group.GetChild(idx_curr + 1);
        TModelPoint pos_w_below;
        glyph_below->GetPosInWorld(pos_w_below);
        TModelUnit will_be_y = top + delta_y;
        TModelUnit critical_y = pos_w_below.Y() + glyph_below->GetHeight() * 0.5 - m_LastHitTrack->GetHeight();
        m_DropTarget = 0;
        IDroppable* droppable = dynamic_cast<IDroppable*>(glyph_below.GetPointer());
        if (droppable && droppable->CanDrop(m_LastHitTrack)) {
//            if (will_be_y > pos_w_below.Y()) {
                m_DropTarget = droppable;
                critical_y = pos_w_below.Y() + glyph_below->GetHeight();
//            } 
        } 
        if (will_be_y > critical_y) {
            m_DropTarget = 0;
            if ((int)(will_be_y - critical_y) == delta_y)
                return;
            top = critical_y;
            p_track->MoveDown(m_Delegate->GetOrder());
            delta_y = (int)(will_be_y - critical_y);
            group.UpdateLayout();
            x_MoveTrackDownRecursive(delta_y, top);
        }
    }
}

END_NCBI_SCOPE
