/*  $Id: text_sel_handler.cpp 41823 2018-10-17 17:34:58Z katargir $
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
 * Authors:  Colleen Bollin (adapted from a file by Andrey Yazhuk)
 *
 * File Description:
 *
 */

#include <ncbi_pch.hpp>
#include <corelib/ncbistd.hpp>

#include <gui/widgets/seq_text/text_sel_handler.hpp>
#include <gui/opengl/glhelpers.hpp>
#include <gui/opengl/glresmgr.hpp>
#include <gui/opengl/irender.hpp>

#include <objmgr/seq_loc_mapper.hpp>

#include <math.h>


BEGIN_NCBI_SCOPE
USING_SCOPE(objects);


static int kDragThreshold = 4; // make this configurable
BEGIN_EVENT_TABLE(CTextSelHandler, wxEvtHandler)
    EVT_LEFT_DOWN(CTextSelHandler::OnMousePush)
    EVT_MOTION(CTextSelHandler::OnMouseDrag)
    EVT_LEFT_UP(CTextSelHandler::OnMouseRelease)
#if 0
//    EVT_LEFT_DOWN(CHitMatrixGraph::OnLeftDown)
//    EVT_LEFT_UP(CHitMatrixGraph::OnLeftUp)
//    EVT_MOTION(CHitMatrixGraph::OnMotion)
//    case FL_PUSH: res =  x_OnMousePush(event, pane); break;
//    case FL_DRAG: res =  x_OnMouseDrag(event, pane); break;
//    case FL_RELEASE: res = x_OnMouseRelease(event, pane); break;
//    case FL_MOVE: res = x_OnMouseMove(event, pane); break;

    EVT_KEY_DOWN(CTextSelHandler::OnKeyDown)
    EVT_KEY_UP(CTextSelHandler::OnKeyUp)
#endif
END_EVENT_TABLE()

////////////////////////////////////////////////////////////////////////////////
/// class CTextSelHandler

CTextSelHandler::CTextSelHandler()
:   m_ExtState(eNoExt),
    m_OpType(eNoOp),
    m_bResizeCursor(false),
    m_pGeometry(NULL),
    m_SelColor(0.5f, 0.5f, 0.5f, 0.25f),
    m_BorderColor(0.25f, 0.25f, 0.25f, 0.5f),
    m_PassiveSelColor(0.6f, 0.6f, 0.6f, 0.25f),
    m_SymbolColor(0.0f, 0.0f, 1.0f, 1.0f),
    m_Font(CGlTextureFont::eFontFace_Helvetica, 24),
    m_HoverPos (kInvalidSeqPos)
{
    m_Selection.clear();
}


CTextSelHandler::~CTextSelHandler()
{
}


void CTextSelHandler::SetHost(ISeqTextGeometry* pGeometry)
{
    m_pGeometry = pGeometry;
}


TSeqRange  CTextSelHandler::GetSelectionLimits()    const
{
    return m_Selection.GetLimits();
}


const   CTextSelHandler::TRangeColl&   CTextSelHandler::GetSelection() const
{
    return m_Selection;
}


void    CTextSelHandler::SetRangeSelection(const TRangeColl& C, bool b_redraw)
{
    m_Selection = C;
    if(b_redraw)    {
        m_pGeometry->STG_OnChanged();
    }
}


void CTextSelHandler::ResetObjectSelection(bool b_redraw)
{
    CScope& scope = m_pGeometry->STG_GetScope();
    m_SelectedObjects.Clear(&scope);

    if (b_redraw) {
        m_pGeometry->STG_OnChanged();
    }
}


void CTextSelHandler::DeSelectObject(const CObject* obj, bool b_redraw)
{
    if(obj) {
        m_SelectedObjects.Remove(*obj);
        if (b_redraw) {
            m_pGeometry->STG_OnChanged();
        }
    }
}


void CTextSelHandler::SelectObject(const CObject* obj, bool b_redraw)
{
    m_SelectedObjects.Add(NULL, *const_cast<CObject*>(obj));
    if (b_redraw) {
        m_pGeometry->STG_OnChanged();
    }
}


void    CTextSelHandler::ResetRangeSelection(bool b_redraw)
{
    m_Selection.clear();
    if(b_redraw) {
        m_pGeometry->STG_OnChanged();
    }
}

void    CTextSelHandler::ResetSelection(bool b_redraw)
{
    ResetRangeSelection(b_redraw);
    ResetObjectSelection(b_redraw);
}

void    CTextSelHandler::SetColor(EColorType type, const CRgbaColor& color)
{
    switch(type)    {
    case eSelection: m_SelColor = color; break;
    case ePasssiveSelection: m_PassiveSelColor = color; break;
    case eSymbol: m_SymbolColor = color; break;
    default: _ASSERT(false);
    }
}


////////////////////////////////////////////////////////////////////////////////
// event handlers

void CTextSelHandler::OnMousePush(wxMouseEvent &event)
{
    if (event.ButtonDClick(1)) {
        x_OnResetAll();
    } else {
        x_OnStartSel(event);
        x_OnSelectCursor(event);
    }
}


void CTextSelHandler::OnMouseDrag(wxMouseEvent &event)
{
    if (event.Dragging()) {
        if (m_OpType != eNoOp) {
            x_OnChangeSelRange(event);
        }
    } else {
        TSeqPos pos = x_MouseToSeqPos(event);
        x_SetHoverPos (pos);
        x_OnSelectCursor(event);
    }
}


void CTextSelHandler::OnMouseRelease(wxMouseEvent &event)
{
    m_OpType = x_GetOpTypeByEvent(event);
    x_OnChangeSelRange(event);
    x_OnEndSelRange();
    x_OnSelectCursor(event);
}


void CTextSelHandler::x_SetHoverPos(TSeqPos pos)
{
    if (m_pGeometry != NULL) {
        m_pGeometry->STG_ReportMouseOverPos(pos);
        if (m_HoverPos != pos) {
            m_HoverPos = pos;
        }
        m_pGeometry->STG_OnChanged();
    }
}


#if 0
int CTextSelHandler::x_OnMouseMove(CGUIEvent& event, CGlPane& pane)
{
    CGUIEvent::EGUIState state = event.GetGUIState();
    x_OnSelectCursor();

    TSeqPos pos = x_MouseToSeqPos();
    x_SetHoverPos (pos);

    if(state == CGUIEvent::eSelectState || state == CGUIEvent::eSelectAltState) {
           return 1;
    } else return 0;
}


void CTextSelHandler::OnKeyDown(wxKeyEvent& event)
{
    x_OnOpChange(event);
    return 0;
}


void CTextSelHandler::OnKeyUp(wxKeyEvent& event)
{
    x_OnOpChange(event);



    return 0;
}
#endif

////////////////////////////////////////////////////////////////////////////////
/// Signal handlers
void    CTextSelHandler::x_OnStartSel(wxMouseEvent &event)
{
    TSeqRange HitR;
    bool b_hit_start = false;
    x_HitTest(HitR, b_hit_start, event);

    if(HitR.NotEmpty()) {   // hit a border of the selected range
        m_OpType = eChange;
        m_ExtState = b_hit_start ? eExtRangeStart : eExtRangeEnd;
        m_CurrRange = HitR;

        x_RemoveFromSelection(HitR);
    } else {
        m_ExtState = eExtRangeEnd;
        m_OpType = x_GetOpTypeByEvent(event);

        TSeqPos pos = x_MouseToSeqPos(event);

        m_CurrRange.SetFrom(pos);
        m_CurrRange.SetToOpen(pos);
    }

    m_pGeometry->STG_Redraw();
}


void CTextSelHandler::x_OnChangeSelRange(wxMouseEvent &event)
{
    if (m_ExtState == eNoExt && m_CurrRange.Empty())   
        return;
    TSeqPos pos = x_MouseToSeqPos(event);
    pos = x_ClipPosByRange(pos);
    TSeqRange old_r = m_CurrRange;
    bool in_right_margin = x_MouseInRightMargin(event);

    if (in_right_margin && m_ExtState == eExtRangeEnd) {
        pos++;
    }

    if(m_ExtState == eNoExt  &&  pos != m_CurrRange.GetFrom())   {
        if(pos > m_CurrRange.GetFrom()) {
            m_CurrRange.SetToOpen(pos);
            m_ExtState = eExtRangeEnd;
        } else  {
            m_CurrRange.SetToOpen(m_CurrRange.GetFrom());
            m_CurrRange.SetFrom(pos);
            m_ExtState = eExtRangeStart;
        }
    } else if(m_ExtState == eExtRangeEnd &&  pos != m_CurrRange.GetToOpen()) {
        if(pos > m_CurrRange.GetFrom())  {
            m_CurrRange.SetToOpen(pos);
        } else { //flip
            m_CurrRange.SetToOpen(m_CurrRange.GetFrom());
            m_CurrRange.SetFrom(pos);
            m_ExtState = eExtRangeStart;
        }
    } else if(m_ExtState == eExtRangeStart  &&  pos != m_CurrRange.GetFrom())    {
        if(pos <= m_CurrRange.GetToOpen())  {
            m_CurrRange.SetFrom(pos);
        } else {
            m_CurrRange.SetFrom(m_CurrRange.GetToOpen());
            m_CurrRange.SetToOpen(pos);
            m_ExtState = eExtRangeEnd;
        }
    };

    //Report end of range
    if (pos == m_CurrRange.GetToOpen() && pos > 0) {
        x_SetHoverPos (pos - 1);
    } else {
        x_SetHoverPos (pos);
    }

    if(m_CurrRange != old_r) {
        m_pGeometry->STG_Redraw();
    }
}


void    CTextSelHandler::x_OnEndSelRange()
{
    if(! m_CurrRange.Empty())   {
        switch(m_OpType)   {
        case eAdd:
        case eChange:   x_AddToSelection(m_CurrRange); break;
        case eRemove:   x_RemoveFromSelection(m_CurrRange); break;
        case eNoOp: break;
        }
    }
    m_CurrRange.SetLength(0);

    m_ExtState = eNoExt;
    m_OpType = eNoOp;

    m_pGeometry->STG_OnChanged();
}

void    CTextSelHandler::x_OnResetAll()
{
    bool b_update = ! m_Selection.empty();
    m_Selection.clear();

    m_ExtState = eNoExt;
    m_OpType = eNoOp;

    if(b_update) {
        m_pGeometry->STG_OnChanged();
    }
}


void    CTextSelHandler::x_OnOpChange(wxMouseEvent &event)
{
   if(m_ExtState != eNoExt)   {
        EOpType NewType = x_GetOpTypeByEvent(event);

        if(NewType != m_OpType) {
            m_OpType = NewType;
            x_OnSelectCursor(event);

            m_pGeometry->STG_Redraw();
        }
    }
}


void    CTextSelHandler::x_OnSelectCursor(wxMouseEvent &event)
{
    switch(m_OpType)    {
    case eNoOp:   {
        TSeqRange HitR;
        bool b_hit_start = false;
        x_HitTest(HitR, b_hit_start, event);

        m_bResizeCursor = HitR.NotEmpty();
    }; break;
    case eAdd:
    case eRemove:
    case eChange:   m_bResizeCursor = true; break;
    }
    x_SetCursor();
}


void    CTextSelHandler::x_SetCursor()
{
    if (m_pGeometry) {
        if (m_bResizeCursor) {
            m_pGeometry->STG_SetResizeCursor();
        } else {
            m_pGeometry->STG_SetDefaultCursor();
        }
    }
}

////////////////////////////////////////////////////////////////////////////////
/// helper functions

// translate modificators to operation type
CTextSelHandler::EOpType CTextSelHandler::x_GetOpTypeByEvent(wxKeyEvent& event)    const
{
//    switch(event.GetGUIState())    {
//    case CGUIEvent::eSelectState: return eAdd;
//    case CGUIEvent::eSelectAltState:    return eRemove;
//    default:    return eNoOp;
//    }
    return eNoOp;
}


CTextSelHandler::EOpType CTextSelHandler::x_GetOpTypeByEvent(wxMouseEvent& event)    const
{
//    switch(event.GetGUIState())    {
//    case CGUIEvent::eSelectState: return eAdd;
//    case CGUIEvent::eSelectAltState:    return eRemove;
//    default:    return eNoOp;
//    }
    if (event.AltDown()) {
        return eRemove;
    } else {
        return eAdd;
    }
}


TSeqPos CTextSelHandler::x_MouseToSeqPos(wxMouseEvent& event)
{
    //_ASSERT(m_pPane);
    wxPoint pt = event.GetPosition();
    return m_pGeometry->STG_GetSequenceByWindow(pt.x, pt.y);
}


bool CTextSelHandler::x_MouseInRightMargin(wxMouseEvent& event)
{
    //_ASSERT(m_pPane);
    wxPoint pt = event.GetPosition();
    return m_pGeometry->STG_IsPointInRightMargin (pt.x, pt.y);
}


void    CTextSelHandler::x_HitTest(TSeqRange& range, bool& b_hit_start, wxMouseEvent &event)
{
    //_ASSERT(m_pPane);
    wxPoint pt = event.GetPosition();

    TSeqPos z = m_pGeometry->STG_GetSequenceByWindow(pt.x, pt.y);

    int min_D = -1;
    bool b_min_start = false;
    const TSeqRange* p_min_range = 0;

    const TRangeColl& C = m_Selection;
    ITERATE(TRangeColl, it, C)  {
        const TSeqRange& R = *it;

        int D = (int)abs((int)(z - R.GetFrom()));

        if(min_D < 0 || min_D > D)    {
            min_D = D;
            b_min_start = true;
            p_min_range = &R;
        }
        D = (int)abs((int)(z - R.GetToOpen()));
        if(min_D > D)    {
            min_D = D;
            b_min_start = false;
            p_min_range = &R;
        }
    }

    if(min_D > -1  && min_D <= kDragThreshold) {
        b_hit_start = b_min_start;
        _ASSERT(p_min_range);
        range  = *p_min_range;
    } else range.SetLength(0);
}


TSeqPos CTextSelHandler::x_ClipPosByRange(TSeqPos pos)
{
    _ASSERT (m_pGeometry);

    TSeqPos chars_in_line, lines_in_seq;
    m_pGeometry->STG_GetLineInfo(chars_in_line, lines_in_seq);

    pos = max (pos, (TSeqPos) 0);
    pos = min (pos, (TSeqPos) (chars_in_line * lines_in_seq));
    return pos;
}


void    CTextSelHandler::x_AddToSelection(const TSeqRange& range)
{
    m_Selection.CombineWith(range);
}


void    CTextSelHandler::x_RemoveFromSelection(const TSeqRange& range)
{
    m_Selection.Subtract(range);
}

inline void RenderRect(const TModelRect& rc, TModelUnit offset_x, TModelUnit offset_y,
                       CRgbaColor& color, GLenum type)
{
    IRender& gl = GetGl();

    gl.ColorC(color);
    gl.PolygonMode(GL_FRONT_AND_BACK, type);
    gl.Rectd(rc.Left() - offset_x, rc.Top() - offset_y,
             rc.Right() - offset_x, rc.Bottom() - offset_y);
}


void    CTextSelHandler::x_RenderRange (TSeqRange r,
                                        CRgbaColor& border_color)
{
    IRender& gl = GetGl();

    TSeqPos seq_start = min (r.GetFrom(), r.GetTo());
    TSeqPos seq_end = max (r.GetFrom(), r.GetTo());

    _ASSERT (m_pGeometry);
    TSeqPos vis_start, vis_stop;
    m_pGeometry->STG_GetVisibleRange (vis_start, vis_stop);

    if (seq_start > vis_stop || seq_end < vis_start) {
        // range is completely offscreen, no need to draw
        return;
    }

    TSeqPos  chars_in_line, lines_in_seq;
    m_pGeometry->STG_GetLineInfo(chars_in_line, lines_in_seq);
    seq_start = max ((TSeqPos) 0, seq_start);
    seq_end = min (seq_end, lines_in_seq * chars_in_line);

    TModelPoint start_point = m_pGeometry->STG_GetModelPointBySourcePos(seq_start);
    TModelPoint end_point = m_pGeometry->STG_GetModelPointBySourcePos(seq_end);
    TSeqPos     row_end = ((seq_start / chars_in_line) * chars_in_line) + chars_in_line - 1;
    while (row_end < vis_start) {
        TSeqPos next_row_start = row_end + 1;
        row_end = min (next_row_start + chars_in_line - 1, seq_end);
    }
    TModelPoint interim_point = m_pGeometry->STG_GetModelPointBySourcePos (row_end);

    if (start_point.m_Y == end_point.m_Y) {
        interim_point.m_X = end_point.m_X;
    }

    TModelRect rc (start_point.m_X, start_point.m_Y - 0.2,
                   interim_point.m_X + 1.0,
                   interim_point.m_Y + 0.8);

    gl.LineWidth (0.5);

    RenderRect(rc, .1, 0, m_PassiveSelColor, GL_FILL);
    RenderRect(rc, 0, 0, border_color, GL_LINE);

    while (row_end < vis_stop && row_end < seq_end)
    {
        TSeqPos next_row_start = row_end + 1;
        start_point = m_pGeometry->STG_GetModelPointBySourcePos(next_row_start);
        row_end = min (next_row_start + chars_in_line - 1, seq_end);
        interim_point = m_pGeometry->STG_GetModelPointBySourcePos(row_end);
        rc.Init (start_point.m_X, start_point.m_Y - 0.2, interim_point.m_X + 1.0, interim_point.m_Y + 0.8);

        RenderRect(rc, 0, 0, m_PassiveSelColor, GL_FILL);
        RenderRect(rc, 0, 0, border_color, GL_LINE);
    }
}


void    CTextSelHandler::x_RenderOpSymbol (TSeqRange r)
{
    TSeqPos seq_start = min (r.GetFrom(), r.GetTo());
    TSeqPos seq_end = max (r.GetFrom(), r.GetTo());
    int     sym_len = 0;

    // draw Operation symbol in center of range
    const char* s = 0;
    switch(m_OpType)    {
        case eAdd:
            s = "+";
            sym_len = 1;
            break;
        case eRemove:
            s = "-";
            sym_len = 1;
            break;
        case eChange:
            s = "<->";
            sym_len = 3;
            break;
        case eNoOp:
            break;
    }
    if (sym_len < 1) {
        return;
    }

    _ASSERT (m_pGeometry);
    TSeqPos vis_start, vis_stop;
    m_pGeometry->STG_GetVisibleRange (vis_start, vis_stop);

    if (seq_start > vis_stop || seq_end < vis_start) {
        // range is completely offscreen, no need to draw
        return;
    }

    seq_start = max (seq_start, vis_start);
    seq_end = min (seq_end, vis_stop);

    TModelPoint start_point = m_pGeometry->STG_GetModelPointBySourcePos(seq_start);
    TModelPoint end_point = m_pGeometry->STG_GetModelPointBySourcePos(seq_end);
    TModelRect  range_rect(min (start_point.m_X, end_point.m_X), min (start_point.m_Y, end_point.m_Y),
                           max (start_point.m_X, end_point.m_X), max (start_point.m_Y, end_point.m_Y));

    // if there is enougth space - draw operation symbol
    TModelUnit x = range_rect.Left() + range_rect.Width() / 2 - sym_len / 2;
    TModelUnit y = range_rect.Bottom() + range_rect.Height() / 2 + .5;
    m_Font.TextOut(x, y, s);
}


void    CTextSelHandler::x_RenderSelectedFeatures ()
{
    TConstObjects objs;
    m_SelectedObjects.GetObjects(objs);

    if (objs.size() == 0) 
        return;
    if (!m_Mapper) {
        // need to map locations
        CBioseq_Handle top_handle = m_pGeometry->STG_GetScope().GetBioseqHandle(*(m_pGeometry->STG_GetDataSourceLoc()->GetId()));
        m_Mapper.Reset(new CSeq_loc_Mapper(top_handle, CSeq_loc_Mapper::eSeqMap_Up));
    }
    set <CSerialObject*> objsToDraw;
    ITERATE(TConstObjects, obj, objs) {
        const CSerialObject* cso =
            dynamic_cast<const CSerialObject*>(obj->GetPointer());

        if (cso) {
            //const CTypeInfo* this_info = cso->GetThisTypeInfo();
            if (cso->GetThisTypeInfo() == CSeq_feat::GetTypeInfo()) {
                const CSeq_feat& feat = dynamic_cast<const CSeq_feat&>(*cso);
                CRef<CSeq_loc> feat_loc = m_Mapper->Map(feat.GetLocation());

                if (m_pGeometry != NULL) {
                    m_pGeometry->STG_RenderSelectedFeature(feat, *feat_loc);
                }
            }
        }
    }
}


void    CTextSelHandler::Render(CGlPane& pane, ERenderingOption option)
{
    CGlAttrGuard AttrGuard(GL_ENABLE_BIT | GL_POLYGON_BIT | GL_HINT_BIT);

    TModelRect rc_vis = pane.GetVisibleRect();

    if(! rc_vis.IsEmpty()) {
        CGlPaneGuard GUARD(pane, CGlPane::eOrtho);

        // draw exisiting selection

        const TRangeColl& C = m_Selection;
        ITERATE(TRangeColl, it, C)  { // for every segment
            const TSeqRange& r = *it;

            x_RenderRange (r, m_BorderColor);
        }

        if(m_OpType != eNoOp)   { // draw current range

            x_RenderRange (m_CurrRange, m_SymbolColor);

            if(option == eActiveState)  {
                x_RenderOpSymbol (m_CurrRange);
            }
        }

        x_RenderSelectedFeatures();

        if (m_pGeometry != NULL) {
            vector<CConstRef<CSeq_feat> > feat_list = m_pGeometry->STG_GetFeaturesAtPosition(m_HoverPos);
            ITERATE(vector<CConstRef<CSeq_feat> >, feat, feat_list) {
                m_pGeometry->STG_RenderMouseOverFeature(**feat);
            }
        }
    }
}


void CTextSelHandler::STGH_GetSelectedFeatureSubtypes(CSeqTextDefs::TSubtypeVector &subtypes)
{
    unsigned int i;
    TConstObjects objs;
    TSeqPos start_offset, stop_offset;

    subtypes.clear();

    // no point in trying to return feature subtypes without a host
    if (m_pGeometry == NULL) {
        return;
    }

    m_pGeometry->STG_GetVisibleRange(start_offset, stop_offset);

    for (i = 0; i < stop_offset - start_offset + 2; i++) {
        subtypes.push_back (CSeqFeatData::eSubtype_bad);
    }

    m_SelectedObjects.GetObjects(objs);

    // no selected features, no selected feature subtypes
    if (objs.size() == 0) {
        return;
    }

    if (!m_Mapper) {
        // need to map locations
        CBioseq_Handle top_handle = m_pGeometry->STG_GetScope().GetBioseqHandle(*(m_pGeometry->STG_GetDataSourceLoc()->GetId()));
        m_Mapper.Reset(new CSeq_loc_Mapper(top_handle, CSeq_loc_Mapper::eSeqMap_Up));
    }
    set <CSerialObject*> objsToDraw;
    ITERATE(TConstObjects, obj, objs) {
        const CSerialObject* cso =
            dynamic_cast<const CSerialObject*>(obj->GetPointer());

        if (cso) {
            //const CTypeInfo* this_info = cso->GetThisTypeInfo();
            if (cso->GetThisTypeInfo() == CSeq_feat::GetTypeInfo()) {
                const CSeq_feat& feat = dynamic_cast<const CSeq_feat&>(*cso);
                CRef<CSeq_loc> feat_loc = m_Mapper->Map(feat.GetLocation());
                m_pGeometry->STG_SetSubtypesForFeature (subtypes, *feat_loc, feat.GetData().GetSubtype(), start_offset, stop_offset);
            }
        }
    }
}


void CTextSelHandler::STGH_GetMouseOverFeatureSubtypes(CSeqTextDefs::TSubtypeVector &subtypes)
{
    unsigned int i;
    TConstObjects objs;
    TSeqPos start_offset, stop_offset;

    subtypes.clear();

    // no point in trying to return feature subtypes without a host
    if (m_pGeometry == NULL) {
        return;
    }

    m_pGeometry->STG_GetVisibleRange(start_offset, stop_offset);

    for (i = 0; i < stop_offset - start_offset + 2; i++) {
        subtypes.push_back (CSeqFeatData::eSubtype_bad);
    }

    vector<CConstRef<CSeq_feat> > feat_list = m_pGeometry->STG_GetFeaturesAtPosition(m_HoverPos);
    ITERATE(vector<CConstRef<CSeq_feat> >, feat, feat_list) {
        m_pGeometry->STG_SetSubtypesForFeature (subtypes, (*feat)->GetLocation(), (*feat)->GetData().GetSubtype(), start_offset, stop_offset);
    }

}


END_NCBI_SCOPE
