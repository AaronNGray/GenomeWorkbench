/*  $Id: align_mark_handler.cpp 41823 2018-10-17 17:34:58Z katargir $
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
 * CAlignMarkHandler - class implementing support for Marks in alignments.
 * IAlignMarkHandlerHost - context in which CAlignMarkHandler operates.
 */

#include <ncbi_pch.hpp>

#include <gui/widgets/aln_multiple/align_mark_handler.hpp>
#include <gui/opengl/glutils.hpp>
#include <gui/opengl/irender.hpp>

#include <wx/settings.h>

#include <math.h>


BEGIN_NCBI_SCOPE
USING_SCOPE(objects);

BEGIN_EVENT_TABLE(CAlignMarkHandler, wxEvtHandler)
    EVT_LEFT_DOWN(CAlignMarkHandler::OnLeftDown)
    EVT_LEFT_UP(CAlignMarkHandler::OnLeftUp)
    EVT_MOTION(CAlignMarkHandler::OnMotion)
    EVT_MOUSE_CAPTURE_LOST(CAlignMarkHandler::OnMouseCaptureLost)
    EVT_KEY_DOWN(CAlignMarkHandler::OnKeyDown)
    EVT_KEY_UP(CAlignMarkHandler::OnKeyUp)
END_EVENT_TABLE()


CAlignMarkHandler::CAlignMarkHandler()
:   m_Host(NULL),
    m_Pane(NULL),
    m_bHitResizable(false),
    m_State(eIdle),
    m_CursorId(wxCURSOR_DEFAULT),
    m_PrevPos((TSeqPos)-1),
    m_FillColor(0.5f, 0.5f, 1.0f, 0.5f),
    m_FrameColor(0.25f, 0.25f, 0.5f, 1.0f),
    m_DragArea(0)
{
    m_DragArea = wxSystemSettings::GetMetric(wxSYS_DRAG_X);
}


void CAlignMarkHandler::SetHost(IAlignMarkHandlerHost* pHost)
{
    m_Host = pHost;
}


IGenericHandlerHost* CAlignMarkHandler::GetGenericHost()
{
    return dynamic_cast<IGenericHandlerHost*>(m_Host);
}


void CAlignMarkHandler::SetPane(CGlPane* pane)
{
    m_Pane = pane;
}


wxEvtHandler* CAlignMarkHandler::GetEvtHandler()
{
    return this;
}

////////////////////////////////////////////////////////////////////////////////
// opeartions with Mark

// "coll" specifies ranges in sequence coordinates
void CAlignMarkHandler::MarkRowSeq(TNumrow row, const TRangeColl& coll, bool set)
{
    TRowToMarkMap::iterator itM = m_mpRowToMark.find(row);
    if(itM == m_mpRowToMark.end())
        itM = m_mpRowToMark.insert(TRowToMarkMap::value_type(row, TRangeColl())).first;
    TRangeColl& mark = itM->second;

    if(set) {   // mark
        mark.CombineWith(coll);
    } else  {   // unmark
        mark.Subtract(coll);
    }
}


// "coll" specifies ranges in alignment coordinates
void CAlignMarkHandler::MarkRow(TNumrow row, const TRangeColl& coll, bool set)
{
    const IAlnMultiDataSource*  pAlnDS = x_GetHost()->MHH_GetAlnDS();

    TRowToMarkMap::iterator itM = m_mpRowToMark.find(row);
    if(itM == m_mpRowToMark.end())
        itM = m_mpRowToMark.insert(TRowToMarkMap::value_type(row, TRangeColl())).first;
    TRangeColl& mark = itM->second;

    ITERATE(TRangeColl, itR, coll) { // for each Range in C
        // translate Range to sequence coordinates
        TSeqPos from = pAlnDS->GetSeqPosFromAlnPos(row, itR->GetFrom(),
                                                    IAlnExplorer::eRight);
        TSeqPos to = pAlnDS->GetSeqPosFromAlnPos(row, itR->GetTo(),
                                                    IAlnExplorer::eRight);
        TSeqRange  R(from, to);
        if(set)   {
            mark += R;  // mark
        } else  {
            mark -= R;  // unmark
        }
    }
}


void CAlignMarkHandler::UnMarkAll(TNumrow row)
{
    m_mpRowToMark.erase(row);
}


const CAlignMarkHandler::TRangeColl* CAlignMarkHandler::GetMark(TNumrow row) const
{
    TRowToMarkMap::const_iterator itM = m_mpRowToMark.find(row);
    return (itM != m_mpRowToMark.end()) ? &itM->second : NULL;
}


void CAlignMarkHandler::MarkSelectedRows(const TRangeColl& coll, bool set)
{
    const TSelListModel* pModel = x_GetHost()->MHH_GetSelListModel();
    TSelListModel::TIndexVector vSel;
    pModel->SLM_GetSelectedIndices(vSel);

    //const IAlnMultiDataSource*  pAlnDS = x_GetHost()->MHH_GetAlnDS();

    ITERATE(TSelListModel::TIndexVector, it, vSel)  { // for each Mark
        TNumrow row = pModel->SLM_GetItem(*it);
        MarkRow(row, coll, set);
    }
}


void CAlignMarkHandler::UnMarkAll()
{
    m_mpRowToMark.clear();
}


void CAlignMarkHandler::UnMarkSelected()
{
    // get selected lines
    const TSelListModel* pModel = x_GetHost()->MHH_GetSelListModel();
    TSelListModel::TIndexVector vIndices;
    pModel->SLM_GetSelectedIndices(vIndices);

    // for each selected Line
    ITERATE(TSelListModel::TIndexVector, it, vIndices)  {
        TNumrow row = x_GetHost()->MHH_GetRowByLine(*it);
        m_mpRowToMark.erase(row);
    }
}


const   CAlignMarkHandler::TRowToMarkMap&  CAlignMarkHandler::GetMarks() const
{
    return m_mpRowToMark;
}


////////////////////////////////////////////////////////////////////////////////
// event handlers

bool CAlignMarkHandler::x_MarkState()
{
    return wxGetKeyState(wxKeyCode('M'));
}


void CAlignMarkHandler::OnLeftDown(wxMouseEvent& event)
{
    if(x_MarkState())   {
        wxPoint ms_pos = event.GetPosition();
        x_OnStartSel(ms_pos);
        x_OnSelectCursor(ms_pos);
    } else {
        event.Skip();
    }
}

void CAlignMarkHandler::OnMotion(wxMouseEvent& event)
{
    bool mark = x_MarkState();
    if(event.Dragging())    {
        if(m_State == eResize  &&  mark) {
            x_OnChangeSelRange(event.GetPosition());
        }
        // always handle drags
    } else {
        if(mark)    {
            x_OnSelectCursor(event.GetPosition());
        } else {
            event.Skip();
        }
    }
}


void CAlignMarkHandler::OnLeftUp(wxMouseEvent& event)
{
    if(m_State != eIdle)    {
        if(m_State == eResize)  {
            wxPoint ms_pos = event.GetPosition();
            EState new_state = x_MarkState() ? eReady : eIdle;
            x_OnEndSelRange(new_state, ms_pos);
        }
    } else {
        event.Skip();
    }
}


void CAlignMarkHandler::OnKeyDown(wxKeyEvent& event)
{
    if(x_MarkState())    {
        int k = event.GetKeyCode();
        if(k == WXK_DELETE)  {
            UnMarkSelected();
            GetGenericHost()->GHH_Redraw();
            return;
        }
        x_UpdateState(true, event.GetPosition());
    } else {
        event.Skip();
    }
}


void CAlignMarkHandler::OnKeyUp(wxKeyEvent& event)
{
    if(m_State == eResize)   {
        if(x_MarkState())    {
            x_OnEndSelRange(eIdle, event.GetPosition());
        } else {
            x_UpdateState(true, event.GetPosition());
        }
    } else {
        event.Skip();
    }
}


////////////////////////////////////////////////////////////////////////////////
/// Signal handlers
void CAlignMarkHandler::x_OnStartSel(const wxPoint& ms_pos)
{
    if(x_HitSelectedLine(ms_pos)) {
        m_PrevPos = (TSeqPos) -1;

        x_InitDeltaMap(ms_pos);

        m_State = eResize;
        GetGenericHost()->GHH_Redraw();
        GetGenericHost()->GHH_CaptureMouse();
    }
}


void CAlignMarkHandler::x_OnChangeSelRange(const wxPoint& ms_pos)
{
    TModelUnit mX = x_MouseToSeqPos(ms_pos);
    TSeqPos pos = (TSeqPos) floor(mX);
    pos = x_ClipPosByRange(pos);

    if(pos != m_PrevPos)    {
        x_UpdateSelection(pos);
        GetGenericHost()->GHH_Redraw();
    }

    m_PrevPos = pos;
}


void CAlignMarkHandler::x_OnEndSelRange(EState new_state, const wxPoint& ms_pos)
{
    x_OnChangeSelRange(ms_pos);

    x_UpdateMarks();
    m_State = new_state;

    x_OnSelectCursor(ms_pos);
    GetGenericHost()->GHH_Redraw();
    GetGenericHost()->GHH_ReleaseMouse();
}


void CAlignMarkHandler::x_OnResetAll()
{
    m_mpRowToMark.clear();
    m_mpRowToDelta.clear();

    m_State = eIdle;
    m_PrevPos = (TSeqPos) -1;

    GetGenericHost()->GHH_Redraw();
}


void CAlignMarkHandler::x_UpdateState(bool b_key, const wxPoint& ms_pos)
{
    bool bHit = x_HitSelectedLine(ms_pos);

    EState new_state = m_State;
    switch(m_State) {
    case    eIdle:
    case    eReady:
        new_state = (b_key  &&  bHit) ? eReady : eIdle;
        break;
    case    eResize:
        new_state = b_key ? eResize : eIdle;
        break;
    }

    if(new_state != m_State) {
        m_State = new_state;

        x_OnSelectCursor(ms_pos);
        GetGenericHost()->GHH_Redraw();
    }
}


// Select cursor depending on the current state and mouse position
void CAlignMarkHandler::x_OnSelectCursor(const wxPoint& ms_pos)
{
    switch(m_State)    {
    case eIdle:
        m_CursorId = wxCURSOR_DEFAULT;
        break;

    case eReady:    {
        if(x_HitSelectedLine(ms_pos)) {
            m_CursorId = x_HitRangeBorder(ms_pos) ? wxCURSOR_SIZEWE : wxCURSOR_IBEAM;
        } else {
            m_CursorId = wxCURSOR_DEFAULT;
        }
        break;
    }
    case eResize:
        m_CursorId = wxCURSOR_SIZEWE;
        break;
    default:
        break;
    }

    GetGenericHost()->GHH_SetCursor(wxCursor(m_CursorId));
}


////////////////////////////////////////////////////////////////////////////////
/// helper functions

// converts mouse X position to the position in Alignment
TModelUnit CAlignMarkHandler::x_MouseToSeqPos(const wxPoint& ms_pos)
{
    _ASSERT(m_Pane);
    return m_Host->MHH_GetSeqPosByX(ms_pos.x) + 0.5;
}


// functions creates a "delta" map for ranges  that will be resized
void CAlignMarkHandler::x_InitDeltaMap(const wxPoint& ms_pos)
{
    m_mpRowToDelta.clear();

    // get selected lines
    const TSelListModel* pModel = x_GetHost()->MHH_GetSelListModel();
    TSelListModel::TIndexVector vIndices;
    pModel->SLM_GetSelectedIndices(vIndices);

    const IAlnMultiDataSource* pAlnDS = x_GetHost()->MHH_GetAlnDS();

    // for each selected Line
    ITERATE(TSelListModel::TIndexVector, it, vIndices)  {
        TNumrow row = x_GetHost()->MHH_GetRowByLine(*it);
        TSeqRange hit_r;

        // hit testing Mark corresponding to the Row (if it exists)
        bool hit_start = false; // hit start on alignment
        SMarkDelta::EExtState start_state, state;
        bool positive = pAlnDS->IsPositiveStrand(row);

        TRowToMarkMap::iterator itM = m_mpRowToMark.find(row);
        if(itM != m_mpRowToMark.end())   {
            TRangeColl& Mark = itM->second;
            x_HitTest(row, Mark, ms_pos.x, hit_r, hit_start);
        } else { // if Mark does not exist - create it
            m_mpRowToMark[row] = TRangeColl();
        }

        bool seq_start = (hit_start == positive); // hit start on sequnence

        TSeqPos start_pos;
        if(hit_r.NotEmpty()) {   // hit a border of the marked range
            start_state = hit_start ?   SMarkDelta::eExtRangeStart
                                    :   SMarkDelta::eExtRangeEnd;
            state = start_state;
            start_pos = seq_start ? hit_r.GetFrom() : hit_r.GetToOpen();
        } else {
            start_state = SMarkDelta::eNoExt;
            state = positive ? SMarkDelta::eExtRangeEnd : SMarkDelta::eExtRangeStart;

            TModelUnit mX = x_MouseToSeqPos(ms_pos);
            start_pos = (TSeqPos) floor(mX);
            if( ! positive)  {
                start_pos--;
            }
            start_pos = pAlnDS->GetSeqPosFromAlnPos(row, start_pos, IAlnExplorer::eRight);

            hit_r.SetFrom(start_pos);
            hit_r.SetLength(0);
        }

        // initate Map record
        m_mpRowToDelta[row] = SMarkDelta(hit_r, TSeqRange(start_pos, start_pos - 1), start_state, state);
    }
}


//returns true if mouse pointer is over selected line
bool    CAlignMarkHandler::x_HitSelectedLine(const wxPoint& ms_pos)
{
    const TSelListModel* pModel = x_GetHost()->MHH_GetSelListModel();
    int Index = x_GetHost()->MHH_GetLineByWindowY(ms_pos.y);

    return Index >=0  &&  pModel->SLM_IsItemSelected(Index);
}


// returns true if mouse pointer is located over the end of one of the marked segments
bool    CAlignMarkHandler::x_HitRangeBorder(const wxPoint& ms_pos) const
{
    int Index = x_GetHost()->MHH_GetLineByWindowY(ms_pos.y);

    if(Index >= 0)  {
        const TSelListModel* pModel = x_GetHost()->MHH_GetSelListModel();

        if(pModel->SLM_IsItemSelected(Index))   {
            TNumrow row = x_GetHost()->MHH_GetRowByLine(Index);
            TSeqRange hit_r;
            bool hit_start = false;
            // test the Mark corresponding to the row
            TRowToMarkMap::const_iterator it = m_mpRowToMark.find(row);
            if(it != m_mpRowToMark.end())   {
                const TRangeColl& Mark = it->second;
                x_HitTest(row, Mark, ms_pos.x, hit_r, hit_start);
                if(hit_r.NotEmpty())
                    return true;
            }
        }
    }
    return false;
}


// perfroms a hit test of a given position "X" agains given TRangeColl C
// if positions hits a range in the C, this range is returned in "Range"
// (in "sequence" coordinates), overwise "Range" is set empty. If position
// hits the start of a range then "hit_start" is assigned "true"
void CAlignMarkHandler::x_HitTest(TNumrow row, const TRangeColl& C, int X, TSeqRange& Range, bool& hit_start) const
{
    _ASSERT(m_Pane);

    int MinD = -1;
    bool bMinStart = false;
    TRangeColl::const_iterator itMinRange = C.end();
    const IAlnMultiDataSource *pAlnDS = x_GetHost()->MHH_GetAlnDS();

    ITERATE(TRangeColl, itR, C)  { // iterating by Ranges in C
        TSeqPos from = pAlnDS->GetAlnPosFromSeqPos(row, itR->GetFrom());
        TSeqPos to = pAlnDS->GetAlnPosFromSeqPos(row, itR->GetTo());
        TSeqRange  aln_r(from, to);

        int FromX = m_Pane->ProjectX(aln_r.GetFrom());
        int ToX = m_Pane->ProjectX(aln_r.GetToOpen());

        int D = abs(X - FromX); // distance from the start of the range
        if(MinD < 0 || MinD > D)    {
            MinD = D;
            bMinStart = true;
            itMinRange = itR;
        }

        D = abs(X - ToX); // distance from the stop of the range
        if(MinD > D)    {
            MinD = D;
            bMinStart = false;
            itMinRange = itR;
        }
    }

    if(MinD > -1  &&  MinD <= m_DragArea) {
        hit_start = bMinStart;
        _ASSERT(itMinRange != C.end());
        Range  = *itMinRange;
    } else Range.SetLength(0);
}


TSeqPos CAlignMarkHandler::x_ClipPosByRange(TSeqPos pos)
{
    TModelRect rcL = m_Pane->GetModelLimitsRect();
    pos = min(pos, (TSeqPos) rcL.Right() - 1);
    pos = max(pos, (TSeqPos) rcL.Left());
    return pos;
}


// updates Ranges being dragged accordingly to the given position
void CAlignMarkHandler::x_UpdateSelection(TSeqPos pos)
{
    const IAlnMultiDataSource* pAlnDS = x_GetHost()->MHH_GetAlnDS();

    NON_CONST_ITERATE(TRowToDeltaMap, it, m_mpRowToDelta)    {
        SMarkDelta& delta = it->second;
        TNumrow row = it->first;
        bool negative = pAlnDS->IsNegativeStrand(row);
        TSeqPos trans_pos = pos + (negative ? -1 : 0);
        IAlnExplorer::ESearchDirection dir = negative    ?   IAlnExplorer::eLeft
                                                        :   IAlnExplorer::eRight;

        TSeqPos seq_pos = pAlnDS->GetSeqPosFromAlnPos(row, trans_pos, dir);
        x_UpdateDelta(delta.m_Range, delta.m_State, seq_pos);
    }
}


// update given range so that the range end specified by "state" becomes equal
// to "pos".
void CAlignMarkHandler::x_UpdateDelta(TSeqRange& R,
                                    SMarkDelta::EExtState& state, TSeqPos pos)
{
    if(state == SMarkDelta::eNoExt  &&  pos != R.GetFrom())   {
        if(pos > R.GetFrom()) {
            R.SetToOpen(pos);
            state = SMarkDelta::eExtRangeEnd;
        } else  {
            R.SetToOpen(R.GetFrom());
            R.SetFrom(pos);
            state = SMarkDelta::eExtRangeStart;
        }
    } else if(state == SMarkDelta::eExtRangeEnd  &&  pos != R.GetToOpen()) {
        if(pos > R.GetFrom())  {
            R.SetToOpen(pos);
        } else { //flip
            R.SetToOpen(R.GetFrom());
            R.SetFrom(pos);
            state = SMarkDelta::eExtRangeStart;
        }
    } else if(state == SMarkDelta::eExtRangeStart  &&  pos != R.GetFrom())    {
        if(pos <= R.GetToOpen())  {
            R.SetFrom(pos);
        } else {
            R.SetFrom(R.GetToOpen());
            R.SetToOpen(pos);
            state = SMarkDelta::eExtRangeEnd;
        }
    }

}


/// applies changes in SMarkDelta to mark collection
void CAlignMarkHandler::x_UpdateMarks()
{
    ITERATE(TRowToDeltaMap, it, m_mpRowToDelta)    { // for every row
        TNumrow row = it->first;
        TRowToMarkMap::iterator itMark = m_mpRowToMark.find(row);
        if(itMark == m_mpRowToMark.end())   {
            itMark = m_mpRowToMark.insert(
                        TRowToMarkMap::value_type(row, TRangeColl())).first;
        }
        TRangeColl& C = itMark->second;

        const SMarkDelta& delta = it->second;
        const TSeqRange& R = delta.m_Range;
        const TSeqRange& hit_r = delta.m_HitRange;

        switch(delta.m_StartState)  {
        case SMarkDelta::eNoExt:    {
            if(R.NotEmpty())
                C.CombineWith(R);
        }; break;
        case SMarkDelta::eExtRangeEnd:  {
            if(R.GetTo() > hit_r.GetTo())  {
                // extend the end
                C.CombineWith(R);
            } else {
                // cut the end
                TSeqPos P = max(hit_r.GetFrom(), R.GetFrom());
                C.Subtract(TSeqRange(P, hit_r.GetTo()));
                if(R.GetFrom() < hit_r.GetFrom())   {
                    C.CombineWith(TSeqRange(R.GetFrom(), hit_r.GetFrom() - 1));
                }
            }
        }; break;
        case SMarkDelta::eExtRangeStart:  {
            if(R.GetFrom() < hit_r.GetFrom())  {
                // extend the start
                C.CombineWith(R);
            } else {
                // cut the start
                TSeqPos P = min(hit_r.GetTo(), R.GetTo());
                C.Subtract(TSeqRange(hit_r.GetFrom(), P));
                if(R.GetTo() > hit_r.GetTo())   {
                    C.CombineWith(TSeqRange(hit_r.GetToOpen(), R.GetTo()));
                }
            }
        }; break;
        };
    }
    m_mpRowToDelta.clear();
}


void CAlignMarkHandler::Render(CGlPane& pane)
{
    IRender& gl = GetGl();
    TModelRect rcV = pane.GetVisibleRect();

    const IAlnMultiDataSource* pAlnDS = x_GetHost()->MHH_GetAlnDS();
    const TSelListModel* pModel = x_GetHost()->MHH_GetSelListModel();

    if(! rcV.IsEmpty()  && pAlnDS  && pModel) {

        pane.OpenOrtho();

        TModelUnit offset_x = pane.GetOffsetX();
        TModelUnit offset_y = pane.GetOffsetY();

        ITERATE(TRowToMarkMap, itM, m_mpRowToMark)  { // for each Mark
            TNumrow row = itM->first;
            int Index = x_GetHost()->MHH_GetLineByRowNum(row);

            if(Index >= 0)  { // row exists
                TModelUnit top_y = x_GetHost()->MHH_GetLinePosY(Index);
                TModelUnit bottom_y = top_y + x_GetHost()->MHH_GetLineHeight(Index) - 1;
                bool bVisible = ! (bottom_y < rcV.Top()  ||  top_y > rcV.Bottom());

                if(bVisible) {
                    bool bSelected = pModel->SLM_IsItemSelected(Index);
                    const TRangeColl& Mark = itM->second;

                    // draw existing mark
                    ITERATE(TRangeColl, itR, Mark)  {   // for each Range in Mark
                        TSeqRange aln_r = x_AlnRangeFromSeqRange(pAlnDS, row, *itR);
                        TModelUnit x1 = aln_r.GetFrom() - offset_x;
                        TModelUnit x2 = aln_r.Empty() ? x1 : (aln_r.GetToOpen() - offset_x);

                        gl.ColorC(m_FillColor);
                        gl.PolygonMode(GL_FRONT_AND_BACK, GL_FILL);
                        gl.Rectd(x1, bottom_y - offset_y, x2, top_y - offset_y);

                        if(bSelected)   {
                            gl.ColorC(m_FrameColor);
                            gl.PolygonMode(GL_FRONT_AND_BACK, GL_LINE);
                            gl.Rectd(x1, bottom_y - offset_y, x2, top_y - offset_y);
							gl.PolygonMode(GL_FRONT_AND_BACK, GL_FILL);
                        }
                    }

                    // draw delta - range being edited
                    TRowToDeltaMap::const_iterator itD = m_mpRowToDelta.find(row);
                    if(itD != m_mpRowToDelta.end())  {
                        const SMarkDelta& delta = itD->second;
                        const TSeqRange& delta_r = delta.m_Range;

                        TSeqRange aln_r = x_AlnRangeFromSeqRange(pAlnDS, row, delta_r);

                        TModelUnit x1 = aln_r.GetFrom() - offset_x;
                        TModelUnit x2 = aln_r.Empty() ? x1 : (aln_r.GetToOpen() - offset_x);

                        gl.Color4d(0.25, 0.25, 1.0, 1.0);
                        gl.PolygonMode(GL_FRONT_AND_BACK, GL_LINE);
                        gl.Rectd(x1, bottom_y - offset_y,
                                x2, top_y - offset_y);
						gl.PolygonMode(GL_FRONT_AND_BACK, GL_FILL);
                    }
                }
            }
        }
        pane.Close();
    }
}


TSeqRange CAlignMarkHandler::x_AlnRangeFromSeqRange(const IAlnMultiDataSource* pAlnDS ,
                                                    TNumrow row,
                                                    const TSeqRange& seq_r)
{
    TSeqRange r;
    bool negative = pAlnDS->IsNegativeStrand(row);

    if(seq_r.Empty()) {
        TSeqPos seq_from = seq_r.GetFrom() + (negative ? -1 : 0);
        TSeqPos pos_1 = pAlnDS->GetAlnPosFromSeqPos(row, seq_from);//,
                        //IAlnMultiDataSource::TSearchDirection::eRight);

        r.SetFrom(pos_1);
        r.SetLength(0);
    } else {
        TSeqPos pos_1, pos_2;

        if(negative)   {
            pos_2 = pAlnDS->GetAlnPosFromSeqPos(row, seq_r.GetFrom(),
                                IAlnExplorer::eLeft);
            pos_1 = pAlnDS->GetAlnPosFromSeqPos(row, seq_r.GetTo(),
                                IAlnExplorer::eRight);
        } else {
            pos_1 = pAlnDS->GetAlnPosFromSeqPos(row, seq_r.GetFrom(),
                                IAlnExplorer::eRight);
            pos_2 = pAlnDS->GetAlnPosFromSeqPos(row, seq_r.GetTo(),
                                IAlnExplorer::eLeft);

        }
        r.SetFrom(pos_1);
        r.SetTo(pos_2);
    }
    return r;
}


void CAlignMarkHandler::OnMouseCaptureLost(wxMouseCaptureLostEvent& evt)
{
    m_State = eIdle;

    x_OnSelectCursor(wxPoint()); // position does not matter for eIdle
    GetGenericHost()->GHH_Redraw();
}


END_NCBI_SCOPE
