/*  $Id: alnmulti_header_handler.cpp 42648 2019-03-28 14:36:06Z katargir $
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

#include <ncbi_pch.hpp>

#include <gui/widgets/aln_multiple/alnmulti_header_handler.hpp>

#include <gui/opengl/glhelpers.hpp>
#include <gui/opengl/irender.hpp>

#include <wx/settings.h>

BEGIN_NCBI_SCOPE

////////////////////////////////////////////////////////////////////////////////
/// CAlnMultiHeaderHandler

BEGIN_EVENT_TABLE(CAlnMultiHeaderHandler, wxEvtHandler)
    EVT_LEFT_DOWN(CAlnMultiHeaderHandler::OnLeftDown)
    EVT_LEFT_UP(CAlnMultiHeaderHandler::OnLeftUp)
    EVT_MOTION(CAlnMultiHeaderHandler::OnMotion)
    EVT_KEY_DOWN(CAlnMultiHeaderHandler::OnKeyDown)
END_EVENT_TABLE()

static const int kGrabZone = 4;

// It probably is good idea to have a minumal column width
// to allow users adjust the width easier. 
static const int kMinWidth = 10;

CAlnMultiHeaderHandler::CAlnMultiHeaderHandler()
:   m_Host(NULL),
    m_Pane(NULL),
    m_State(eIdle),
    m_HitResult(eNone),
    m_DragIndex(-1), m_ResizableIndex(-1),
    m_CurrInsIndex(-1)
{
    m_DragArea = wxSystemSettings::GetMetric(wxSYS_DRAG_X);
}


void CAlnMultiHeaderHandler::SetHost(IAMHeaderHandlerHost* host)
{
    m_Host = host;
}


IGenericHandlerHost* CAlnMultiHeaderHandler::GetGenericHost()
{
    return dynamic_cast<IGenericHandlerHost*>(m_Host);
}


bool CAlnMultiHeaderHandler::IsActive() const
{
    return m_State != eIdle;
}


void CAlnMultiHeaderHandler::SetPane(CGlPane* pane)
{
    m_Pane = pane;
}


wxEvtHandler* CAlnMultiHeaderHandler::GetEvtHandler()
{
    return this;
}


void CAlnMultiHeaderHandler::OnLeftDown(wxMouseEvent& event)
{
    _ASSERT(m_Host  &&  m_Host->HHH_GetContext());

    wxPoint ms_pos = event.GetPosition();
    m_VPPushPos = m_VPPrevMousePos = m_VPMousePos = m_Host->HHH_GetVPPosByWindowPos(ms_pos);
    m_State = ePushed;

    x_HitTest(m_VPPushPos, m_DragIndex);

    x_OnSelectCursor();
}


void CAlnMultiHeaderHandler::OnMotion(wxMouseEvent& event)
{
    _ASSERT(m_Host);
    wxPoint ms_pos = event.GetPosition();

    if(event.Dragging())    {
        m_VPPrevMousePos = m_VPMousePos;
        m_VPMousePos = m_Host->HHH_GetVPPosByWindowPos(ms_pos);

        if(m_State == ePushed)  {
            m_HitResult = x_HitTest(m_VPPushPos, m_DragIndex);

            int d_x = m_VPMousePos.X() - m_VPPushPos.X();
            int d_y = m_VPMousePos.Y() - m_VPPushPos.Y();
            int dd = d_x * d_x + d_y * d_y;

            if(dd > m_DragArea * m_DragArea)    {       // dragged far enough
                switch(m_HitResult) {
                case eBorder:   {
                    x_SwitchToResizeMode();
                    break;
                }
                case eColumn:   {
                    x_SwitchToMoveMode();
                    break;
                }
                default: break;
                }
            }
        } else {
            switch(m_State) {
            case eResize:   {
                int d_x = m_VPMousePos.X() - m_VPPushPos.X();
                x_MoveColumnSeparator(d_x);
                break;
            };
            case eMove:
                x_MoveColumn();
                break;
            default: break;
            };
        }

        x_OnSelectCursor();
    } else {
        TVPPoint pt = m_Host->HHH_GetVPPosByWindowPos(ms_pos);
        m_HitResult = x_HitTest(pt, m_DragIndex);
        event.Skip();
    }
    x_OnSelectCursor();
}


void CAlnMultiHeaderHandler::OnLeftUp(wxMouseEvent& event)
{
    if(m_State != eIdle)    {
        x_SwitchToIdleState(true);
    } else {
        event.Skip();
    }
}


void CAlnMultiHeaderHandler::OnKeyDown(wxKeyEvent& event)
{
    if(m_State != eIdle)    {
        int key = event.GetKeyCode();
        if(key == WXK_ESCAPE)    {
            x_SwitchToIdleState(false);
        }
    } else {
        event.Skip();
    }
}


CAlnMultiHeaderHandler::EHitResult
    CAlnMultiHeaderHandler::x_HitTest(const TVPPoint& point, int& index)
{
    _ASSERT(m_Host  &&  m_Host->HHH_GetContext());

    IAlnMultiHeaderContext* context = m_Host->HHH_GetContext();
    // get index of the column that physically contains the mouse pointer
    index = context->GetColumnIndexByX(point.X());

    int col_n = context->GetColumnsCount();
    if(index >= 0  &&  index < col_n)    {
        const TColumn& column = context->GetColumn(index);
        int right = column.m_Pos + column.m_Width - 1;
        int x = point.X();

        if(x >= right - kGrabZone  &&  index < col_n - 1)  {   // right border
            return eBorder;
        } else if(x < column.m_Pos + kGrabZone)    {   // left border
            for( index--; index >= 0; index--) {
                const TColumn& col = context->GetColumn(index);
                if(col.m_Visible)   {
                    return eBorder;
                }
            }
            return eNone;
        } else {
            return eColumn;
        }
    } else {
        index = -1;
        return eNone;
    }
}


// initiates "resising by dragging" session
void CAlnMultiHeaderHandler::x_SwitchToResizeMode()
{
    _ASSERT(m_Host  &&  m_Host->HHH_GetContext());

    IAlnMultiHeaderContext* context = m_Host->HHH_GetContext();
    int col_n = context->GetColumnsCount();

    if(m_DragIndex < col_n - 1)  {
        m_State = eResize;

        // initializing dragging state
        m_Columns.clear();
        m_Columns.resize(col_n);
        for( int i = 0;  i < col_n;  i++ )  {
            m_Columns[i] = context->GetColumn(i);
        }

        //TODO clean-up later this dead old code
        //int right_i = x_NextVisibleColumn(m_DragIndex, true);
        //m_OrigPos = m_Columns[right_i].m_Pos;

        m_OrigPos =  m_Columns[m_DragIndex].Right();
        m_ResizableIndex = context->GetResizableColumnIndex();

        // calculate limits for separator dragging
        if(x_HasResizableColumn())  {
            int resizable_width = m_Columns[m_ResizableIndex].m_Width;

            if(m_DragIndex < m_ResizableIndex)    {
                // we can move to the left untill the resized column width is >= 0
                m_LeftLimit = (m_DragIndex == 0 ? 0 : m_Columns[m_DragIndex].m_Pos) + kMinWidth;
                // we can move to the right until we consume all space of the
                // resiable column
                m_RightLimit = m_OrigPos + resizable_width - kMinWidth;
            } else {
                // we can move to the left until we consume all space of the
                // resiable column
                m_LeftLimit = m_OrigPos - resizable_width + kMinWidth;
                // we can move to the right until the resized column width is >= 0
                //TVPRect rc = m_Host->HHH_GetHeaderRect();
                //m_RightLimit = rc.Width();
                m_RightLimit = m_OrigPos + m_Columns[m_DragIndex + 1].m_Width - kMinWidth;
            }
        } else {
            m_LeftLimit = m_Columns[m_DragIndex].m_Pos;
            TVPRect rc = m_Host->HHH_GetHeaderRect();
            m_RightLimit = rc.Width();
        }
        GetGenericHost()->GHH_Redraw();
    }
}


// find the next visible column to the right (left)
int CAlnMultiHeaderHandler::x_NextVisibleColumn(int index, bool right)
{
    int col_n = (int) m_Columns.size();
    _ASSERT(index >= 0  &&  index < col_n);

    if(right)   {
        for( int i = index + 1;  i < col_n;  i++ )   {
            if(m_Columns[i].m_Visible) {
                return i;
            }
        }
    } else {
        for( int  i = index - 1;  i >= 0;  i--) {
            if(m_Columns[i].m_Visible) {
                return i;
            }
        }
    }
    return -1;
}

void CAlnMultiHeaderHandler::x_SwitchToMoveMode()
{
    _ASSERT(m_Host  &&  m_Host->HHH_GetContext());

    IAlnMultiHeaderContext* context = m_Host->HHH_GetContext();
    int col_n = context->GetColumnsCount();

    _ASSERT(m_DragIndex < col_n);

    m_State = eMove;
    m_CurrInsIndex = m_DragIndex;

    // initializing dragging state
    m_Columns.clear();
    m_Columns.reserve(col_n);
    for( int i = 0;  i < col_n;  i++ )  {
        m_Columns.push_back(context->GetColumn(i));
    }
    m_ResizableIndex = context->GetResizableColumnIndex();
    m_Hide = false;

    m_OrigPos = -1;
    m_LeftLimit = m_RightLimit = -1;

    GetGenericHost()->GHH_Redraw();
}


void CAlnMultiHeaderHandler::x_SwitchToIdleState(bool apply)
{
    if(apply)   {
        switch(m_State) {
        case eMove:
            if(x_DoMoveColumn())    {
                m_Host->HHH_SetColumns(m_Columns, m_ResizableIndex);
            }
            break;
        case eResize:
            m_Host->HHH_SetColumns(m_Columns, m_ResizableIndex);
            break;
        case ePushed:
            m_Host->HHH_SortByColumn(m_DragIndex);
            break;
        default: break;
        }
    }

    m_State = eIdle;
    m_HitResult = eNone;
    m_Columns.clear();

    x_OnSelectCursor();
    GetGenericHost()->GHH_Redraw();
}


bool CAlnMultiHeaderHandler::x_DoMoveColumn()
{
    if(m_Hide)  {
        TColumn& col = m_Columns[m_DragIndex];
        col.m_Visible = false;
        return true;
    } else if(m_CurrInsIndex != m_DragIndex  &&  m_CurrInsIndex != m_DragIndex + 1)   {
        TColumn col = m_Columns[m_DragIndex];
        int resizable_index = m_ResizableIndex;

        // move the column to the new position
        m_Columns.erase(m_Columns.begin() + m_DragIndex);
        int ins_index = m_CurrInsIndex;
        if(m_DragIndex < m_CurrInsIndex)    {
            ins_index--;
        }

        int col_n = (int) m_Columns.size();
        if(m_CurrInsIndex < col_n) {
            m_Columns.insert(m_Columns.begin() + ins_index, col);
        } else {
            m_Columns.push_back(col);
        }

        // update Resizable Column Index
        if(m_DragIndex < m_ResizableIndex  &&  m_CurrInsIndex > m_ResizableIndex )  {
            resizable_index--;
        } else if(m_DragIndex > m_ResizableIndex  &&  m_CurrInsIndex <= m_ResizableIndex )  {
            resizable_index++;
        } else if(m_DragIndex == m_ResizableIndex)  {
            resizable_index = m_CurrInsIndex;
        }
        resizable_index = min(resizable_index, col_n);

        m_ResizableIndex = resizable_index;
        return true;
    }
    return false;
}


bool CAlnMultiHeaderHandler::x_HasResizableColumn()
{
    if(m_ResizableIndex >= 0  &&  m_ResizableIndex < (int) m_Columns.size())  {
        return m_Columns[m_ResizableIndex].m_Visible;
    }
    return false;
}


// d_x - global shift
void CAlnMultiHeaderHandler::x_MoveColumnSeparator(int d_x)
{
    _ASSERT(m_Host);

    // calculate new separator position
    int new_value = m_OrigPos + d_x;
    new_value = max(new_value, m_LeftLimit);
    new_value = min(new_value, m_RightLimit);

    if(x_HasResizableColumn())  {
        if(m_DragIndex < m_ResizableIndex)    {
            TColumn& column = m_Columns[m_DragIndex];
            int new_width = new_value - column.m_Pos;
            int d_width = new_width - column.m_Width;

            if(d_width != 0)    {
                column.m_Width = new_width;

                // update columns to the left
                for( int i = m_DragIndex + 1; i <= m_ResizableIndex;  i++ )   {
                    m_Columns[i].m_Pos += d_width;
                }
                m_Columns[m_ResizableIndex].m_Width -= d_width;
                GetGenericHost()->GHH_Redraw();
            }
        } else {
            int right_i = x_NextVisibleColumn(m_DragIndex, true);
            TColumn& column = m_Columns[right_i];
            int d_pos = new_value - column.m_Pos;

            if(d_pos != 0)  {
                column.m_Width -= d_pos;

                // update columns to the right
                for( int i = right_i;  i > m_ResizableIndex; i--)  {
                    m_Columns[i].m_Pos += d_pos;
                }
                m_Columns[m_ResizableIndex].m_Width += d_pos;
                GetGenericHost()->GHH_Redraw();
            }
        }
    } else {        // Resizable column is hidden
        TColumn& column = m_Columns[m_DragIndex];
        int new_width = new_value - column.m_Pos;
        int d_width = new_width - column.m_Width;

        if(d_width != 0)    {
            column.m_Width = new_width;

            // update columns to the right
            for( int i = m_DragIndex + 1; i <= m_ResizableIndex;  i++ )   {
                m_Columns[i].m_Pos += d_width;
            }
            m_Columns[m_ResizableIndex].m_Width -= d_width;
            GetGenericHost()->GHH_Redraw();
        }
    }
}


void CAlnMultiHeaderHandler::x_MoveColumn()
{
    _ASSERT(m_Host);

    TVPRect rc_header = m_Host->HHH_GetHeaderRect();
    int d_y = m_VPMousePos.Y() - m_VPPushPos.Y();
    m_Hide = -d_y > rc_header.Height();

    if( ! m_Hide)   {
        // locate insertion point
        int col_n = (int) m_Columns.size();
        int ins_pos = col_n;
        for( int i = 0;  i < col_n;  i++ ) {
            const TColumn& col = m_Columns[i];
            if(col.m_Visible) {
                if(m_VPMousePos.X() < (col.m_Pos + col.m_Width / 2)) {
                    ins_pos = i;
                    break;
                }
            }
        }

        _ASSERT(ins_pos >=0  && ins_pos <= col_n);

        if(ins_pos != m_CurrInsIndex) {
            m_CurrInsIndex = ins_pos;
        }
    }

    if(m_VPMousePos.X() != m_VPPrevMousePos.X())   {
        GetGenericHost()->GHH_Redraw();
    }
}


void CAlnMultiHeaderHandler::x_OnSelectCursor()
{
    m_CursorId = wxCURSOR_DEFAULT;

    switch(m_State) {
    case eIdle:
    case ePushed:
        if(m_HitResult == eBorder)  {
            m_CursorId = wxCURSOR_SIZEWE;
        }
        break;
    case eResize:
        m_CursorId = wxCURSOR_SIZEWE;
        break;
    case eMove:
        m_CursorId = wxCURSOR_SIZING;
        break;
    default:
        break;
    }

    GetGenericHost()->GHH_SetCursor(wxCursor(m_CursorId));
}


void CAlnMultiHeaderHandler::Render(CGlPane& pane)
{
    if(m_State != eIdle)  {
        CGlAttrGuard guard(GL_LINE_BIT |  GL_ENABLE_BIT  | GL_COLOR_BUFFER_BIT );
        x_SetupContext();

        pane.OpenPixels();

        switch(m_State) {
        case eResize:
            x_RenderResizeMode(pane);
            break;
        case eMove:
            x_RenderMoveMode(pane);
            break;
        default:
            break;
        }

        pane.Close();
    }
}


void CAlnMultiHeaderHandler::x_SetupContext()
{
    IRender& gl = GetGl();

    gl.Enable(GL_BLEND);
    gl.BlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    gl.Enable(GL_LINE_SMOOTH);
    glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);
}


void CAlnMultiHeaderHandler::x_RenderResizeMode(CGlPane& pane)
{
    IRender& gl = GetGl();

    // draw separation lines
    const TVPRect& rc_vp = pane.GetViewport();
    int y1 = rc_vp.Bottom();
    int y2 = rc_vp.Top();

    gl.Color4d(0.0, 0.0, 1.0, 0.25);
    gl.LineWidth(3.0);

    gl.Begin(GL_LINES);
    for( size_t i = 0;  i < m_Columns.size(); i++)  {
        TColumn& col = m_Columns[i];
        if(col.m_Visible) {
            int x = col.m_Pos + col.m_Width - 1;
            gl.Vertex2d(x, y1);
            gl.Vertex2d(x, y2);
        }
    }
    gl.End();

    // highligh the columns being resized
    TVPRect rc_col= m_Host->HHH_GetHeaderRect();

    if(x_HasResizableColumn())  {
        // render the signle column
        int i_col = (m_DragIndex < m_ResizableIndex) ? m_DragIndex
                                    : x_NextVisibleColumn(m_DragIndex, true);
        const TColumn& col = m_Columns[i_col];
        rc_col.SetHorz(col.m_Pos, col.Right());
    } else {
        // render a rectangle covering two columns (both are affected by resiaing in this mode)
        int x1 = m_Columns[m_DragIndex].m_Pos;
        int right_i = x_NextVisibleColumn(m_DragIndex, true);
        int x2 = m_Columns[right_i].Right();
        rc_col.SetHorz(x1, x2);
    }

    gl.Begin(GL_LINES);
        gl.Vertex2d(rc_col.Left(),  rc_col.Bottom());
        gl.Vertex2d(rc_col.Right(), rc_col.Bottom());
    gl.End();

    gl.LineWidth(1.0);
}



// renders handler in eMove mode
void CAlnMultiHeaderHandler::x_RenderMoveMode(CGlPane& /*pane*/)
{
    _ASSERT(m_Host  &&  m_State == eMove);
    _ASSERT(m_CurrInsIndex >=0  &&  m_CurrInsIndex <= (int) m_Columns.size());

    IRender& gl = GetGl();

    const TVPRect rc_h = m_Host->HHH_GetHeaderRect();

    gl.Color4d(0.0, 0.0, 1.0, 0.25);
    if( ! m_Hide)   {
        if(m_CurrInsIndex != m_DragIndex  &&  m_CurrInsIndex != m_DragIndex + 1)    {
            // highlight insertion point
            TModelUnit x = 0;
            if(m_CurrInsIndex < (int) m_Columns.size())  {
                x = m_Columns[m_CurrInsIndex].m_Pos;
            } else {
                x = m_Columns[m_CurrInsIndex - 1].Right();
            }
            x -= 0.5;

            // render insertion point
            gl.LineWidth(6);
            gl.Begin(GL_LINES);
                gl.Vertex2d(x, rc_h.Bottom());
                gl.Vertex2d(x, rc_h.Top());
            gl.End();
        } else {
            // highlight its current position
            TVPRect rc_col(rc_h);
            const TColumn& col = m_Columns[m_DragIndex];
            rc_col.SetHorz(col.m_Pos, col.Right());

            gl.LineWidth(3);
            gl.RectC(rc_col);
        }
    }
    gl.LineWidth(1);

    // render column header
    TVPRect rc_col(rc_h);
    int left = m_Columns[m_DragIndex].m_Pos;
    int right = m_Columns[m_DragIndex].Right();
    rc_col.SetHorz(left, right);

    rc_col.Offset(m_VPMousePos.X() - m_VPPushPos.X(), m_VPMousePos.Y() - m_VPPushPos.Y());

    m_Host->HHH_RenderColumnHeader(m_DragIndex, rc_col);

    // render the cross
    if(m_Hide)  {
        int x = m_VPMousePos.X();
        int y = m_VPMousePos.Y();
        int w = 16;

        gl.Color4d(1.0, 0.0, 0.0, 0.5);
        gl.LineWidth(7);
        gl.Begin(GL_LINES);
            gl.Vertex2d(x - w, y - w);
            gl.Vertex2d(x + w, y + w);
            gl.Vertex2d(x + w, y - w);
            gl.Vertex2d(x - w, y + w);
        gl.End();
        gl.LineWidth(1.0);
    }
}


END_NCBI_SCOPE
