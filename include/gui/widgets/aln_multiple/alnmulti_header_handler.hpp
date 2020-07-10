#ifndef __GUI_WIDGETS_ALNMULTI___ALNMULTI_HEADER_HANDLER__HPP
#define __GUI_WIDGETS_ALNMULTI___ALNMULTI_HEADER_HANDLER__HPP

/*  $Id: alnmulti_header_handler.hpp 36919 2016-11-14 21:21:27Z katargir $
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

#include <gui/opengl/glpane.hpp>
#include <gui/opengl/glbitmapfont.hpp>

#include <gui/widgets/gl/ievent_handler.hpp>

#include <gui/widgets/aln_multiple/alnmulti_header.hpp>


BEGIN_NCBI_SCOPE


////////////////////////////////////////////////////////////////////////////////
/// IAMHeaderHandlerHost
class NCBI_GUIWIDGETS_ALNMULTIPLE_EXPORT IAMHeaderHandlerHost
{
public:
    typedef IAlnMultiHeaderContext::SColumn TColumn;
    typedef vector<TColumn> TColumns;

    virtual ~IAMHeaderHandlerHost() {};

    virtual IAlnMultiHeaderContext* HHH_GetContext() = 0;
    virtual TVPRect HHH_GetHeaderRect() = 0;  /// returns bounds of the Header
    virtual TVPPoint    HHH_GetVPPosByWindowPos(const wxPoint& pos) = 0;

    /// change order of columns, their width and posisitions
    virtual void    HHH_SetColumns(const TColumns& columns, int resizable_index) = 0;
    virtual void    HHH_SortByColumn(int index) = 0;

    virtual void    HHH_RenderColumnHeader(int index, const TVPRect& rc) = 0;
};


////////////////////////////////////////////////////////////////////////////////
/// CAlnMultiHeaderHandler
class NCBI_GUIWIDGETS_ALNMULTIPLE_EXPORT CAlnMultiHeaderHandler :
        public wxEvtHandler,
        public IGlEventHandler
{
    DECLARE_EVENT_TABLE()
public:
    typedef IAMHeaderHandlerHost::TColumn TColumn;
    typedef IAMHeaderHandlerHost::TColumns TColumns;

    CAlnMultiHeaderHandler();

    virtual void    SetHost(IAMHeaderHandlerHost* host);
    virtual IGenericHandlerHost*    GetGenericHost();
    virtual bool    IsActive() const;
    virtual void    Render(CGlPane& pane);

    /// @name IGlEventHandler implementaion
    /// @{
    virtual void    SetPane(CGlPane* pane);
    virtual wxEvtHandler*   GetEvtHandler();
    /// @}

    void    OnLeftDown(wxMouseEvent& event);
    void    OnLeftUp(wxMouseEvent& event);
    void    OnMotion(wxMouseEvent& event);
    void    OnKeyDown(wxKeyEvent& event);

protected:

    void    x_OnSelectCursor();
    void    x_SwitchToResizeMode();
    void    x_SwitchToMoveMode();
    void    x_MoveColumnSeparator(int d_x);
    void    x_MoveColumn();
    void    x_SwitchToIdleState(bool apply);

    void    x_SetupContext();
    void    x_RenderResizeMode(CGlPane& pane);
    void    x_RenderMoveMode(CGlPane& pane);

    bool    x_DoMoveColumn();
    bool    x_HasResizableColumn();
    int     x_NextVisibleColumn(int index, bool right);

    enum    EState  {
        eIdle,
        ePushed,
        eResize,
        eMove
    };

    enum    EHitResult  {
        eNone,
        eBorder,
        eColumn
    };

    EHitResult x_HitTest(const TVPPoint& point, int& index);

protected:
    IAMHeaderHandlerHost* m_Host;
    CGlPane*     m_Pane;
    int m_DragArea;

    EState  m_State;
    EHitResult m_HitResult;

    TVPPoint m_VPPushPos;   // point where mouse has been pushed
    TVPPoint m_VPMousePos;  // current mouse pos
    TVPPoint m_VPPrevMousePos; // previous mouse pos

    int m_DragIndex;  // index of the column  or separator being moved
    TColumns m_Columns;  // reflects columns state during dragging
    int m_ResizableIndex;   // index of the resizable column

    int m_OrigPos; // original position of the column separator
    int m_LeftLimit, m_RightLimit;  // right and left limits for the column separator

    /// insertion point for the column being moved (new index for this column)
    int m_CurrInsIndex;
    bool    m_Hide;

    wxStockCursor m_CursorId;
};


END_NCBI_SCOPE

#endif  // __GUI_WIDGETS_ALNMULTI___ALNMULTI_HEADER_HANDLER__HPP
