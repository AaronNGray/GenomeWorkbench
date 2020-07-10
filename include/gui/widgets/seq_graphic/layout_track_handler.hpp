#ifndef GUI_WIDGETS_SEQ_GRAPHIC___LAYOUT_TRACK_HANDLER__HPP
#define GUI_WIDGETS_SEQ_GRAPHIC___LAYOUT_TRACK_HANDLER__HPP

/*  $Id: layout_track_handler.hpp 40803 2018-04-12 15:25:55Z katargir $
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
 * File Description:
 *
 */

#include <corelib/ncbiobj.hpp>
#include <gui/gui_export.h>
#include <gui/widgets/gl/ievent_handler.hpp>
#include <gui/opengl/glpane.hpp>
#include <gui/widgets/seq_graphic/layout_track_impl.hpp>
#include <gui/widgets/seq_graphic/layout_group.hpp>


BEGIN_NCBI_SCOPE

///////////////////////////////////////////////////////////////////////////////
/// ITrackHandlerHost
/// ITrackHandlerHost represents a context for handling the user interactions
/// on a layout track.
///
class ITrackHandlerHost
{
public:
    virtual ~ITrackHandlerHost() {};

    /// Get the layout track under the current mouse position.
    /// @param pos the mouse pos in window coord. sys.
    virtual CRef<CLayoutTrack> THH_GetLayoutTrack() = 0;
    virtual TModelPoint THH_GetModelByWindow(const wxPoint& pt) = 0;
};

////////////////////////////////////////////////////////////////////////////////
/// CLayoutTrackHandler - is the handler for handling the user interactions
/// on layout tracks.  The interactions include repositioning a track, collapsing
/// a track, expanding a track , closing a track and other events resulting from
/// clicking on an icon or button.
///
/// TODO: The declaration is not completed.

class NCBI_GUIWIDGETS_SEQGRAPHIC_EXPORT  CLayoutTrackHandler :
    public wxEvtHandler,
    public IGlEventHandler
{
    DECLARE_EVENT_TABLE()
public:
    enum EState {
        eReadyToMove,
        eMove,
        eIdle
    };

    typedef EState TState;

    CLayoutTrackHandler()
        : m_DropTarget(0)
        , m_Host(NULL)
        , m_Pane(NULL)
        , m_State(eIdle)
    {}

    void    SetHost(ITrackHandlerHost* host);
    IGenericHandlerHost*    GetGenericHost();
    void    Render(CGlPane& pane);

    /// @name IGlEventHandler implementaion
    /// @{
    virtual void    SetPane(CGlPane* pane);
    virtual wxEvtHandler*    GetEvtHandler();
    /// @}

    void    OnKeyEvent(wxKeyEvent& event);
    void    OnOtherMouseEvents(wxMouseEvent& event);
    void    OnLeftDown(wxMouseEvent& event);
    void    OnLeftUp(wxMouseEvent& event);
    void    OnLeftDblClick(wxMouseEvent& event);
    void    OnMotion(wxMouseEvent& event);
    void    OnMouseCaptureLost(wxMouseCaptureLostEvent& event);

    // Drag&drop control
    void    SetDraggable(bool draggable);
private:
    void x_StartMoving();
    void x_StopMoving();
    void x_Move(const TModelPoint& pos);
    void x_MoveTrackUpRecursive(TModelUnit delta_y, TModelUnit& top);
    void x_MoveTrackDownRecursive(TModelUnit delta_y, TModelUnit& top);

private:
    TModelPoint         m_PreMousePos;
    TModelPoint         m_StartPos;
    CRef<CLayoutTrack>  m_LastHitTrack;
    CRef<CLayoutTrack>  m_Delegate;
    IDroppable*         m_DropTarget;
    ITrackHandlerHost*  m_Host;
    CGlPane*            m_Pane;
    TState              m_State;
    bool                m_Draggable = true; // supports darg&drop operation
};

///////////////////////////////////////////////////////////////////////////////
/// CLayoutTrackHandler inline methods
///
inline
void CLayoutTrackHandler::SetHost(ITrackHandlerHost* host)
{ m_Host = host; }

inline /*virutal*/
void CLayoutTrackHandler::SetPane(CGlPane* pane)
{ m_Pane = pane; }

inline
wxEvtHandler* CLayoutTrackHandler::GetEvtHandler()
{ return this; }

inline
IGenericHandlerHost* CLayoutTrackHandler::GetGenericHost()
{ return dynamic_cast<IGenericHandlerHost*>(m_Host); }

inline
void CLayoutTrackHandler::SetDraggable(bool draggable)
{
    m_Draggable = draggable;
}

END_NCBI_SCOPE


#endif	// GUI_WIDGETS_SEQ_GRAPHIC___LAYOUT_TRACK_HANDLER__HPP

