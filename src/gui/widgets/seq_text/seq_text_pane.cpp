/*  $Id: seq_text_pane.cpp 44433 2019-12-18 15:53:19Z shkeda $
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

#include <corelib/ncbistl.hpp>
#include <corelib/ncbitime.hpp>

#include <gui/utils/view_event.hpp>
#include <gui/widgets/wx/simple_clipboard.hpp>

#include <gui/widgets/seq_text/seq_text_pane.hpp>
#include <gui/widgets/seq_text/seq_text_widget.hpp>

#include <gui/objutils/label.hpp>
#include <gui/objutils/utils.hpp>
#include <gui/opengl/glutils.hpp>
#include <gui/opengl/gltexturefont.hpp>
#include <gui/opengl/irender.hpp>

#include <objmgr/util/sequence.hpp>
#include <objmgr/seq_loc_mapper.hpp>
#include "seqtextviewoptions.h"

BEGIN_NCBI_SCOPE
USING_SCOPE(objects);


BEGIN_EVENT_TABLE(CSeqTextPane, CGlWidgetPane)
    EVT_SIZE(CSeqTextPane::OnSize)
    EVT_RIGHT_DOWN(CSeqTextPane::x_OnShowPopup )
    EVT_LEFT_DOWN(CSeqTextPane::OnMousePush)
    EVT_LEFT_DCLICK(CSeqTextPane::OnMousePush)
    EVT_MOTION(CSeqTextPane::OnMouseDrag)
    EVT_LEFT_UP(CSeqTextPane::OnMouseRelease)

END_EVENT_TABLE()


CSeqTextPane::CSeqTextPane(CSeqTextWidget* parent)
:   CGlWidgetPane(parent, wxID_ANY),
    m_pParent(parent),
    m_BackColor(1.0f, 1.0f, 1.0f),
    m_pSeqFont(NULL),
    m_BottomRuler(true),
    m_SubjectGraph(NULL),
    m_SequenceTickGraph(NULL),
    m_Config(new CSeqTextConfig()),
    m_SelHandler()
{
    // Explictely show everything in lower case
    m_Config->SetShowFeatAsLower(true);

    if(GetParent())   {
        // TODO this is wrong, parent must establish conections
        CSeqTextWidget* parent = x_GetParent();
        AddListener(parent, ePool_Parent);
    }

    m_OffsetList.clear();


    m_SequencePane.EnableOffset(true);

    m_pSeqFont = x_ChooseFontForSize(m_Config->GetFontSize());

    m_SubjectGraph = new CSequenceTextGraph();
    m_SubjectGraph->SetFont(m_pSeqFont);
    m_SubjectGraph->SetConfig (m_Config);
    m_SubjectGraph->SetHost(&m_SelHandler);
    m_SubjectGraph->SetGeometry(static_cast<ISeqTextGeometry*>(this));

    m_SequenceTickGraph = new CSequenceTickGraph();
    m_SequenceTickGraph->SetFont(m_pSeqFont);
    m_SequenceTickGraph->SetGeometry(static_cast<ISeqTextGeometry*>(this));
    m_SequenceTickGraph->ShowAbsolutePosition (m_Config->GetShowAbsolutePosition());

    //setup Event Handlers
    m_SelHandler.SetHost(static_cast<ISeqTextGeometry*>(this));

    m_TooltipHandler.SetMode(CTooltipHandler::eHideOnMove);
    m_TooltipHandler.SetHost(this);
    x_RegisterHandler(&m_TooltipHandler, fAllAreas, &m_SequencePane);

    Layout();
    x_ConfigureBottomRuler();

}


CSeqTextPane::~CSeqTextPane()
{
    delete m_SubjectGraph;
    delete m_SequenceTickGraph;
}


TVPPoint CSeqTextPane::GetPortSize(void)
{
    return TVPPoint(m_rcSequence.Width(), m_rcSequence.Height());
}



void CSeqTextPane::x_ConfigureBottomRuler()
{
    m_BottomRuler.SetColor(CRuler::eBackground, m_BackColor);

    m_BottomRuler.SetDisplayOptions(CRuler::fShowTextLabel);

    m_BottomRuler.SetLabelOrientation(CGlTextureFont::fFontRotateBase, 0);
}


void CSeqTextPane::x_Render(void)
{
    glClearColor(m_BackColor.GetRed(), m_BackColor.GetGreen(), m_BackColor.GetBlue(), m_BackColor.GetAlpha());
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // render sequence pane
    x_RenderContent();
}


void CSeqTextPane::Update()
{
    _ASSERT(x_GetParent());

    CSeqTextDataSource* pDS = x_GetParent()->GetDS();

    if (pDS) {
        m_SubjectGraph->SetDataSource(pDS);
        m_SequenceTickGraph->SetDataSource(pDS);
    }
    Layout();
    m_SequencePane.SetViewport(m_rcSequence);

    x_GetParent()->AdjustModelForSequenceAndWindowSize();
}

void CSeqTextPane::x_SetRulerText()
{
    string s_label, q_label;
    CSeqTextDataSource* pDS = x_GetParent()->GetDS();
    if(pDS) {
    }

    m_BottomRuler.SetTextLabel(s_label);
}


const CSeqTextPane::TRangeColl&  CSeqTextPane::GetSelection()
{
    TRangeColl source_selection = m_SelHandler.GetSelection();

    CSeqTextDataSource* pDS = x_GetParent()->GetDS();

    m_Selection.clear();

    ITERATE (TRangeColl, iter, source_selection) {
        TSeqRange sel_range(pDS->SourcePosToSequencePos(iter->GetFrom()),
                            pDS->SourcePosToSequencePos(iter->GetTo()));
        m_Selection.CombineWith(sel_range);
    }

    return m_Selection;
}


void  CSeqTextPane::SetRangeSelection(const TRangeColl& coll)
{
    TRangeColl source_selection;
    CSeqTextDataSource* pDS = x_GetParent()->GetDS();

    source_selection.clear();

    ITERATE (TRangeColl, iter, coll) {
        bool from_in_source = false;
        bool to_in_source = false;
        TSeqPos sel_start = iter->GetFrom();
        TSeqPos sel_end = iter->GetTo();
        TSeqPos tmp;
        if (sel_start > sel_end) {
            tmp = sel_end;
            sel_end = sel_start;
            sel_start = tmp;
        }
        TSeqPos source_start = pDS->SequencePosToSourcePos(sel_start, &from_in_source);
        TSeqPos source_end = pDS->SequencePosToSourcePos(sel_end, &to_in_source);
        // need to make sure this selection is actually part of our
        // source
        while ((!from_in_source || !to_in_source) && sel_start < sel_end) {
            if (!from_in_source) {
                sel_start = pDS->SourcePosToSequencePos(source_start);
                source_start = pDS->SequencePosToSourcePos(sel_start, &from_in_source);
            }
            if (!to_in_source) {
                sel_end = pDS->SourcePosToSequencePos(source_end - 1);
                source_end = pDS->SequencePosToSourcePos(sel_end, &to_in_source);
            }
        }
        if ((from_in_source || to_in_source) && source_start < source_end) {
            TSeqRange sel_range(source_start, source_end);
            source_selection.CombineWith(sel_range);
        }
    }

    m_SelHandler.SetRangeSelection(source_selection, true);
}

void CSeqTextPane::ResetRangeSelection()
{
    m_SelHandler.ResetRangeSelection(true);
}


void CSeqTextPane::ResetObjectSelection()
{
    m_SelHandler.ResetObjectSelection(true);
}


void CSeqTextPane::DeSelectObject(const CObject* obj)
{
    m_SelHandler.DeSelectObject(obj, true);
}


void CSeqTextPane::SelectObject(const CObject* obj)
{
    m_SelHandler.SelectObject (obj, true);
}


void CSeqTextPane::ResetSelection()
{
    m_SelHandler.ResetSelection(true);
}


/// recalculates recatngles for all intrnal objects so that they are positioned
/// correctly with regard to the new windows size
bool CSeqTextPane::Layout(void)
{
    TVPPoint sz_left_ruler, sz_bottom_ruler;
    if(x_GetParent())   {
        const CGlPane& VP = x_GetParent()->GetPort();
        const TModelRect& rc_m = VP.GetModelLimitsRect();
        sz_left_ruler = m_SequenceTickGraph->PreferredSize();

        int max_num_x = (int) ceil(rc_m.Right());
        sz_bottom_ruler = m_BottomRuler.GetPreferredSize(max_num_x);

        wxSize sz = GetClientSize();

        // calculate bounds of the sequence text
        int mx_left = sz_left_ruler.X();
        int mx_bottom = sz_bottom_ruler.Y();
        int mx_right = sz.x - 2;
        int mx_top = sz.y - 2;

        int width = abs (mx_right - mx_left);

        TModelPoint character_size = CharacterSize();
        int num_chars_per_line = (int) floor ((width) / character_size.m_X);
        num_chars_per_line = (int) (floor (num_chars_per_line / 10.0) * 10.0);
        mx_right = mx_left + (int)((num_chars_per_line + 1) * character_size.m_X);
        width = mx_right - mx_left;

        m_rcSequence.Init(mx_left, mx_bottom, mx_right, mx_top);
        m_rcAxes.Init(0, 0, width, mx_top);

        // set Rulers rects
        m_rcBottomRuler.Init(mx_left, m_rcAxes.Bottom(), mx_right, sz_bottom_ruler.Y());
        m_rcLeftRuler.Init(m_rcAxes.Left(), mx_bottom, m_rcAxes.Right(), mx_top);
    }
    return true;
}


void CSeqTextPane::x_RenderContent(void)
{
    IRender& gl = GetGl();

    if(x_GetParent() && x_GetParent()->GetDS()) {


        gl.LineWidth(10.0f);

        x_GetParent()->AdjustModelForSequenceAndWindowSize();

        const CGlPane& VP = x_GetParent()->GetPort();

        // prepare CGlPanes
        //TModelRect rc_M = VP.GetModelLimitsRect();
        TModelRect rc_M = GetPreferredModelRect();
        m_SequencePane.SetModelLimitsRect(rc_M);

        TModelRect rc_V = VP.GetVisibleRect();
        m_SequencePane.SetVisibleRect(rc_V);

        TModelUnit scale_x = VP.GetScaleX();
        TModelUnit scale_y = VP.GetScaleY();
        TModelPoint character_size = CharacterSize();
        scale_y = 1.0 / character_size.m_Y;
        scale_x = 1.0 / character_size.m_X;

        m_SequencePane.SetScale (scale_x, scale_y);

        x_RecalculateOffsets();
        // now rendering

        x_RenderSelHandler();

        x_RenderLeftRuler();
        x_RenderBottomRuler();

        m_SubjectGraph->Render(m_SequencePane);

        // render codons if rendering all
        if (m_Config && m_Config->GetCodonDrawChoice() == CSeqTextPaneConfig::eAll) {
            TSeqPos seq_start, seq_stop;
            STG_GetVisibleRange(seq_start, seq_stop);
            CSeqTextDataSource* pDS = x_GetParent()->GetDS();
            if (pDS) {
                pDS->RenderFeatureExtras(static_cast<ISeqTextGeometry*>(this), m_SequencePane, seq_start, seq_stop);
            }
        }

        // render found list
        x_RenderFoundLocations(m_SequencePane);
    }
}


void CSeqTextPane::x_AdjustBottomRuler (CGlPane& pane)
{
    TVPRect rc_VP(m_rcSequence.Left(), m_rcAxes.Bottom(), m_rcSequence.Right(), m_rcSequence.Bottom() - 1);

    TModelUnit scale_x = m_SequencePane.GetScaleX();
    //TModelUnit scale_y = m_SequencePane.GetScaleY();

    TSeqPos chars_in_line, lines_in_seq;
    STG_GetLineInfo(chars_in_line, lines_in_seq);

    rc_VP.SetRight (rc_VP.Left() + (int)(chars_in_line / scale_x));
    pane.SetViewport(rc_VP);

    pane.SetScale (scale_x, 1.0);

    const CGlPane& VP = x_GetParent()->GetPort();

    TModelRect rc_vis = VP.GetVisibleRect();
    TModelRect rc_lim = VP.GetModelLimitsRect();

    int max_x = chars_in_line;
    rc_lim.SetHorz(0, max_x);
    rc_lim.SetVert(0, rc_VP.Top() - rc_VP.Bottom());
    rc_vis.SetHorz(0, max_x);
    rc_vis.SetVert(0, rc_VP.Top() - rc_VP.Bottom());

    pane.SetModelLimitsRect(rc_lim);
    pane.SetVisibleRect(rc_vis);
}


TModelUnit CSeqTextPane::GetBottomRulerModelAdjustmentHeight()
{
    int sequence_pane_height = m_rcSequence.Height();
    TModelUnit adjustment = 0;

    TModelPoint character_size = CharacterSize();
    TSeqPos     chars_in_line, lines_in_seq;
    STG_GetLineInfo(chars_in_line, lines_in_seq);

    if (character_size.m_Y > 0) {
        TSeqPos lines_in_screen = (TSeqPos) floor(sequence_pane_height / character_size.m_Y);
        TSeqPos model_top = lines_in_seq + GetScrollAdjustmentForLastLineVariations();
        if (model_top >= lines_in_screen) {
            adjustment = m_rcBottomRuler.Height() / character_size.m_Y;
        }
    }
    return adjustment;
}

void CSeqTextPane::x_RenderBottomRuler()
{
    CGlPane pane(CGlPane::eAlwaysUpdate);
    pane.EnableOffset(true);
    pane.SetViewport(m_rcAxes);

    // draw bottom ruler
    x_AdjustBottomRuler (pane);

    m_BottomRuler.Render(pane);
}


void CSeqTextPane::x_RenderLeftRuler()
{
    CGlPane pane(CGlPane::eNeverUpdate);
    pane.EnableOffset(true);
    // draw left ruler
    TVPRect rc_VP(m_rcAxes.Left(), m_rcSequence.Bottom(), m_rcSequence.Left() - 1, m_rcSequence.Top());
    pane.SetViewport(rc_VP);

    // set model limits and visible rect for left ruler to match that for sequence pane
    // only the vertical numbers are important - the horizontal numbers will be changed
    // in the Render function
    const CGlPane& VP = x_GetParent()->GetPort();
    TModelRect rc_lim = VP.GetModelLimitsRect();
    pane.SetModelLimitsRect(rc_lim);
    TModelRect sequence_rect = m_SequencePane.GetVisibleRect();
    pane.SetVisibleRect(sequence_rect);

    m_SequenceTickGraph->Render(pane);
}


int CSeqTextPane::x_GetAreaByVPPos(TVPUnit vp_x, TVPUnit vp_y)
{
    if(m_rcSequence.PtInRect(vp_x, vp_y)) {
        return fSequenceArea;
    } else if(m_rcBottomRuler.PtInRect(vp_x, vp_y)) {
        return fSubjectRuler;
    } else if(m_rcLeftRuler.PtInRect(vp_x, vp_y)) {
        return fQueryRuler;
    }
    return fOther;
}


void CSeqTextPane::x_RenderSelHandler()
{
    x_RenderSelHandler(m_SequencePane);
}


void CSeqTextPane::x_RenderSelHandler(CGlPane& pane)
{
    IRender& gl = GetGl();

    gl.Enable(GL_BLEND);
    gl.BlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    m_SelHandler.Render(pane);

    gl.Disable(GL_BLEND);
}


int CSeqTextPane::OnCopy()
{
    CSeqTextDataSource* pDS = x_GetParent()->GetDS();
    if (!pDS)
        return 0;
    CSeqTextPane::GetSelection();
    if (m_Selection.empty())
        return 0;
    objects::SAnnotSelector *sel = m_Config->GetCaseFeature ();
    string text;
    ITERATE(CRangeCollection<TSeqPos>, it_r, m_Selection) { // for each range in mark
        bool    found_in_source;
        TSeqPos src_start = STG_GetSourcePosBySequencePos (it_r->GetFrom(), &found_in_source);
        TSeqPos src_stop = STG_GetSourcePosBySequencePos(it_r->GetTo() + 1, &found_in_source);
        string buffer;
        pDS->GetSeqData(src_start, src_stop, buffer, sel, m_Config->GetShowFeatAsLower());
        text += buffer;
    }
    CSimpleClipboard::Instance().Set(text);
    return 0;
}


int CSeqTextPane::x_OnPaste ()
{
    return 0;
}


void CSeqTextPane::OnSize(wxSizeEvent& event)
{
    Layout();
    m_SequencePane.SetViewport(m_rcSequence);

    x_GetParent()->AdjustModelForSequenceAndWindowSize();

    event.Skip();
}


////////////////////////////////////////////////////////////////////////////////
/// ISeqTextGeometry implementation
void CSeqTextPane::STG_Redraw()
{
    Refresh();
}


void CSeqTextPane::STG_OnChanged()
{
    CEvent evt(CEvent::eEvent_Message, CViewEvent::eWidgetSelectionChanged);
    Send(&evt, ePool_Parent);

    Refresh();
}


unsigned int CSeqTextPane::x_GetOffsetForVariations(TModelUnit y) {
    unsigned int k;
    //unsigned int last_offset = 0;

    if (m_OffsetList.size() == 0 || y <= m_OffsetList[0].m_StartRow) {
        return 0;
    } else if (y > m_OffsetList[m_OffsetList.size() - 1].m_StartRow) {
        return m_OffsetList[m_OffsetList.size() - 1].m_CumulativeExtraRows;
    }
    unsigned int l = 0, r = m_OffsetList.size() - 2;
    while (r >= l) {
        k = (l + r) / 2;
        if (m_OffsetList[k].m_StartRow < y) {
            if (m_OffsetList[k + 1].m_StartRow >= y) {
                return m_OffsetList[k].m_CumulativeExtraRows;
            } else {
                l = k + 1;
            }
        } else if (m_OffsetList[k].m_StartRow > y) {
            r = k - 1;
        } else {
            return m_OffsetList[k-1].m_CumulativeExtraRows;
        }
    }
    return m_OffsetList[l].m_CumulativeExtraRows;
}


TSeqPos CSeqTextPane::x_GetSequenceRowFromScrollLine(TModelUnit scroll_line)
{
    unsigned int k;
    // need x such that x + x_GetOffsetForVariations (x) = scroll_line
    if (m_OffsetList.size() == 0 || scroll_line <= m_OffsetList[0].m_StartRow) {
        return (TSeqPos) scroll_line;
    }
    else if (scroll_line > m_OffsetList[m_OffsetList.size() - 1].m_StartRow + m_OffsetList[m_OffsetList.size() - 1].m_CumulativeExtraRows) {
        return (TSeqPos) scroll_line - m_OffsetList[m_OffsetList.size() - 1].m_CumulativeExtraRows;
    }
    for (k = 0; k < m_OffsetList.size(); ++k) {
        TSeqPos row_k_sum = m_OffsetList[k].m_StartRow + m_OffsetList[k].m_CumulativeExtraRows;
        if (row_k_sum >= scroll_line) {
            return m_OffsetList[k].m_StartRow;
        }
    }
    return (TSeqPos) scroll_line - m_OffsetList[m_OffsetList.size() - 1].m_CumulativeExtraRows;
/*
    unsigned int l = 1, r = m_OffsetList.size() - 1;
    TSeqPos row_k_sum;

    while (r >= l) {
        k = (r + l) / 2;
        row_k_sum = m_OffsetList[k].m_StartRow + m_OffsetList[k].m_CumulativeExtraRows;
        if (row_k_sum > scroll_line) {
            if (m_OffsetList[k - 1].m_StartRow + m_OffsetList[k - 1].m_CumulativeExtraRows <= scroll_line) {
                return (TSeqPos) scroll_line - m_OffsetList[k - 1].m_CumulativeExtraRows;
            } else {
                r = k - 1;
            }
        } else if (row_k_sum == scroll_line) {
            return m_OffsetList[k].m_StartRow;
        } else {
            l = k + 1;
        }
    }
    return (TSeqPos) scroll_line - m_OffsetList[l - 1].m_CumulativeExtraRows;
*/
}


TSeqPos  CSeqTextPane::STG_GetSequenceByWindow(int x, int y)
{
    // calculate sequence position based on pixel coordinates
    //TModelRect rc_vis = m_SequencePane.GetVisibleRect();
    //TModelRect rc_model = m_SequencePane.GetModelLimitsRect();
    TModelUnit scale_x = m_SequencePane.GetScaleX();
    TModelUnit scale_y = m_SequencePane.GetScaleY();
    TSeqPos    chars_in_line, lines_in_seq;

    STG_GetLineInfo (chars_in_line, lines_in_seq);

    if (x < m_rcSequence.Left()) {
        x = m_rcSequence.Left();
    }

    TSeqPos seq_x = (TSeqPos) floor((double)(x - m_rcSequence.Left()) * scale_x);
    //There may be a blank margin at the right.  Report space in the margin as
    //being part of the last character on the line.
    if (seq_x >= chars_in_line) seq_x = chars_in_line - 1;
    //TSeqPos scrolled_lines = (TSeqPos) floor(rc_model.Top() - rc_vis.Top());
    TSeqPos scrolled_lines = x_GetParent()->GetPortScrollValue();
    TSeqPos screen_line = (TSeqPos) floor((double) y * scale_y);

    TSeqPos seq_y = x_GetSequenceRowFromScrollLine(screen_line + scrolled_lines);
    if (seq_y >= lines_in_seq) seq_y = lines_in_seq - 1;

    TSeqPos seq_pos = (seq_y * chars_in_line) + seq_x;
    TSeqPos max_pos = x_GetParent()->GetDS()->GetDataLen();
    if (seq_pos > max_pos) {
        seq_pos = max_pos;
    }
    return seq_pos;
}


bool CSeqTextPane::STG_IsPointInRightMargin(int x, int y)
{
    TSeqPos    chars_in_line, lines_in_seq;
    STG_GetLineInfo(chars_in_line, lines_in_seq);
    TModelUnit scale_x = m_SequencePane.GetScaleX();
    TSeqPos seq_x = (TSeqPos) floor((double)(x - m_rcSequence.Left()) * scale_x);
    if (seq_x >= chars_in_line) {
        return true;
    } else {
        return false;
    }

}


TModelUnit CSeqTextPane::x_TopBlank()
{
    TModelRect rc_vis = m_SequencePane.GetVisibleRect();
    TModelUnit top_blank = rc_vis.Top() - rc_vis.Bottom() - floor (rc_vis.Top() - rc_vis.Bottom());
    return top_blank;
}


TModelPoint CSeqTextPane::STG_GetModelPointBySourcePos(TSeqPos z)
{
    // calculate pixel coordinate base on sequence position
    TModelPoint window_pos (0.0, 0.0);
    TModelRect rc_vis = m_SequencePane.GetVisibleRect();
    TModelUnit scale_x = m_SequencePane.GetScaleX();
    TModelUnit scale_y = m_SequencePane.GetScaleY();
    TModelUnit scrolled_lines = x_GetParent()->GetPortScrollValue();
    TSeqPos chars_in_line, lines_in_pane;

    TSeqPos seq_start, seq_stop;
    STG_GetVisibleRange(seq_start, seq_stop);

    STG_GetLineInfo (chars_in_line, lines_in_pane);
    int num_lines_in_pane = (int) floor (rc_vis.Top() - rc_vis.Bottom());
    TModelUnit top_blank = x_TopBlank();

    if (chars_in_line > 0 && scale_y > 0 && scale_x > 0) {
        TSeqPos row_y = z / chars_in_line;
        TSeqPos pos_skipped_lines = x_GetOffsetForVariations (row_y);
        window_pos.m_Y = num_lines_in_pane - (row_y - scrolled_lines + pos_skipped_lines) + top_blank - 1;
        window_pos.m_X = ((int) z % chars_in_line);
    }

    return window_pos;
}


TModelPoint CSeqTextPane::STG_GetModelPointBySequencePos(TSeqPos z, bool *found_in_source)
{
    TSeqPos mapped_z = x_GetParent()->GetDS()->SequencePosToSourcePos (z, found_in_source);
    return STG_GetModelPointBySourcePos (mapped_z);
}


TSeqPos CSeqTextPane::STG_GetSourcePosBySequencePos (TSeqPos z, bool *found_in_source)
{
    return x_GetParent()->GetDS()->SequencePosToSourcePos (z, found_in_source);
}

int CSeqTextPane::STG_GetLinesInPane ()
{
    TModelPoint character_size = CharacterSize();
    int num_lines_in_pane = 0;
    if (character_size.m_Y > 0) {
        int sequence_pane_height = m_SequencePane.GetViewport().Height();
        num_lines_in_pane = (TSeqPos) ceil(sequence_pane_height / character_size.m_Y);
    }
    return num_lines_in_pane;
}


void CSeqTextPane::STG_GetVisibleRange (TSeqPos& seq_start, TSeqPos &seq_stop)
{
    const TModelRect& rc_model = m_SequencePane.GetVisibleRect();
    int num_lines_in_pane = (int) floor (rc_model.Top() - rc_model.Bottom());
    int num_chars_per_row = (int) floor ((rc_model.Right() - rc_model.Left()) / 10.0) * 10;
#if 0
    const TModelRect& rc_limits = m_SequencePane.GetModelLimitsRect ();
    TModelUnit top_blank = x_TopBlank();


    TModelUnit scrolled_rows = floor (rc_limits.Top() - rc_model.Top());
#else
    int scrolled_rows = x_GetParent()->GetPortScrollValue();
#endif
    TSeqPos row_y = x_GetSequenceRowFromScrollLine(scrolled_rows);

    seq_start = (TSeqPos) (num_chars_per_row * row_y);

    row_y = x_GetSequenceRowFromScrollLine (row_y + num_lines_in_pane);
    seq_stop = (TSeqPos)(num_chars_per_row) * row_y;
}


objects::CScope& CSeqTextPane::STG_GetScope()
{
    return x_GetParent()->GetDS()->GetScope();
}


const CSeq_loc* CSeqTextPane::STG_GetDataSourceLoc()
{
    return x_GetParent()->GetDS()->GetLoc();
}


TModelUnit  CSeqTextPane::STG_GetTextHeight()
{
    return m_SubjectGraph->GetTextHeight();
}


void
CSeqTextPane::STG_SetSubtypesForFeature
(CSeqTextDefs::TSubtypeVector &subtypes,
 const CSeq_loc& loc,
 int   subtype,
 TSeqPos start_offset,
 TSeqPos stop_offset)
{
    TSeqPos i;

    for (CSeq_loc_CI feat_loc_it (loc);
         feat_loc_it;
         ++ feat_loc_it) {

        CSeq_loc_CI::TRange feat_range = feat_loc_it.GetRange();
        TSeqPos feat_start = feat_range.GetFrom();
        TSeqPos feat_stop = feat_range.GetTo();
        for (i = feat_start;
             i <= feat_stop;
             i++) {
            bool pos_in_source = false;
            TSeqPos k = STG_GetSourcePosBySequencePos(i, &pos_in_source);
            if (pos_in_source && k >= start_offset && k <= stop_offset)
            {
                subtypes[k - start_offset] = CSeqTextDataSource::ChooseBetterSubtype(subtype,
                                   subtypes[k - start_offset]);
            }
            else if (k > stop_offset)
            {
              break;
            }
        }
    }
}


void CSeqTextPane::x_RenderCodonTick(TSeqPos cds_start, ENa_strand strand)
{
    IRender& gl = GetGl();

    bool    found_in_source;
    CRgbaColor CodonColor (0.0f, 0.0f, 0.0f);
    TModelPoint codon_start = STG_GetModelPointBySequencePos (cds_start, &found_in_source);
    TModelUnit tick_len = STG_GetTextHeight() / 4.0;

    if (found_in_source) {
        /* use a fine line to indicate codon location */
        gl.LineWidth (0.5);
        gl.ColorC(CodonColor);
        gl.Begin (GL_LINES);
        gl.Vertex2d(codon_start.m_X, codon_start.m_Y);
        gl.Vertex2d(codon_start.m_X, codon_start.m_Y + tick_len);

        gl.Vertex2d(codon_start.m_X, codon_start.m_Y);
        if (strand == eNa_strand_minus) {
            gl.Vertex2d(codon_start.m_X - tick_len, codon_start.m_Y);
        } else {
            gl.Vertex2d(codon_start.m_X + tick_len, codon_start.m_Y);
        }
        gl.End();
    }
}


void CSeqTextPane::x_RenderCodons (const CSeq_loc& loc, CCdregion_Base::EFrame frame)
{
    int offset = 0;
    CRgbaColor CodonColor (0.0f, 0.0f, 0.0f);

    if (frame == CCdregion::eFrame_two) {
        offset = 1;
    } else if (frame == CCdregion::eFrame_three) {
        offset = 2;
    }

    for (CSeq_loc_CI seq_loc_it (loc); seq_loc_it; ++ seq_loc_it) {
        CSeq_loc_CI::TRange cds_range = seq_loc_it.GetRange();
        TSeqPos cds_start = cds_range.GetFrom();
        TSeqPos cds_stop = cds_range.GetTo() + 1;

        if (seq_loc_it.GetStrand() == eNa_strand_minus) {
            cds_stop -= offset;
            while (cds_stop > cds_start) {
                x_RenderCodonTick(cds_stop, eNa_strand_minus);
                cds_stop -= 3;
            }
        } else {
            cds_start += offset;
            while (cds_start < cds_stop) {
                x_RenderCodonTick(cds_start, eNa_strand_plus);
                cds_start += 3;
            }
        }
        offset = cds_start - cds_stop;
    }
}


void CSeqTextPane::x_RenderFeatureExtras (const CSeq_feat& feat, const CSeq_loc& mapped_loc)
{
    if (feat.GetData().Which() == CSeqFeatData::e_Cdregion) {
        const CCdregion& cdr = feat.GetData().GetCdregion();
        if ( cdr.CanGetFrame()) {
            x_RenderCodons(mapped_loc, cdr.GetFrame());
        }
    }
}


void CSeqTextPane::x_RenderFeatureExtras (const CSeq_feat& feat)
{
    if (feat.GetData().Which() == CSeqFeatData::e_Cdregion) {
        const CCdregion& cdr = feat.GetData().GetCdregion();
        if ( cdr.CanGetFrame()) {
            x_RenderCodons(feat.GetLocation(), cdr.GetFrame());
        }
    }
}


void CSeqTextPane::STG_ReportMouseOverPos(TSeqPos pos)
{
    CSeqTextDataSource* pDS = x_GetParent()->GetDS();
    if (pDS == NULL) return;

    TSeqPos max_pos = pDS->GetDataLen() - 1;
    if (pos > max_pos) pos = max_pos;
    if (m_Config && m_Config->GetShowAbsolutePosition()) {
        pos = pDS->SourcePosToSequencePos(pos) + 1;
    }
    x_GetParent()->ReportMouseOverPos(pos);
}


void CSeqTextPane::STG_RenderSelectedFeature(const CSeq_feat& feat, const CSeq_loc& mapped_loc)
{
    if (m_Config && m_Config->GetCodonDrawChoice() == CSeqTextPaneConfig::eSelected) {
        x_RenderFeatureExtras (feat, mapped_loc);
    }
}


void CSeqTextPane::STG_RenderMouseOverFeature(const CSeq_feat& feat)
{
    if (m_Config && m_Config->GetCodonDrawChoice() == CSeqTextPaneConfig::eMouseOver) {
        x_RenderFeatureExtras (feat);
    }
}


void CSeqTextPane::STG_RenderFeatureExtras(const CMappedFeat& feat)
{
    if (m_Config && m_Config->GetCodonDrawChoice() == CSeqTextPaneConfig::eAll) {
        x_RenderFeatureExtras (*(feat.GetSeq_feat()));
    }
}


vector<CConstRef<CSeq_feat> > CSeqTextPane::STG_GetFeaturesAtPosition(TSeqPos pos)
{
    return x_GetParent()->GetDS()->GetFeaturesAtPosition(pos);
}


void CSeqTextPane::x_OnShowPopup(wxMouseEvent &event)
{
    x_GetParent()->OnShowPopup();
}


void CSeqTextPane::x_UnderlineInterval (TSeqPos start, TSeqPos stop)
{
    IRender& gl = GetGl();

    TSeqPos  seq_start = min (start, stop);
    TSeqPos  seq_end = max (start, stop);

    TSeqPos vis_start, vis_stop;
    STG_GetVisibleRange (vis_start, vis_stop);

    if (seq_start > vis_stop || seq_end < vis_start) {
        // range is completely offscreen, no need to draw
        return;
    }

    TSeqPos chars_in_line, lines_in_seq;
    STG_GetLineInfo(chars_in_line, lines_in_seq);

    TModelPoint start_point = STG_GetModelPointBySourcePos(seq_start);
    TModelPoint end_point = STG_GetModelPointBySourcePos(seq_end);
    TSeqPos     row_end = ((seq_start / chars_in_line) * chars_in_line) + chars_in_line - 1;
    TModelPoint interim_point = STG_GetModelPointBySourcePos (row_end);

    if (start_point.m_Y == end_point.m_Y) {
        interim_point.m_X = end_point.m_X;
    }

    gl.Vertex2d(start_point.m_X, start_point.m_Y - m_SubjectGraph->GetTextOffset());
    gl.Vertex2d(interim_point.m_X + 1.0, start_point.m_Y - m_SubjectGraph->GetTextOffset());

    while (row_end < seq_end)
    {
        TSeqPos next_row_start = row_end + 1;
        start_point = STG_GetModelPointBySourcePos(next_row_start);
        row_end = min ((int)next_row_start + chars_in_line - 1, seq_end);
        interim_point = STG_GetModelPointBySourcePos(row_end);
        gl.Vertex2d(start_point.m_X, start_point.m_Y - m_SubjectGraph->GetTextOffset());
        gl.Vertex2d(interim_point.m_X + 1.0, start_point.m_Y - m_SubjectGraph->GetTextOffset());
    }
}


void CSeqTextPane::x_RenderFoundLocations(CGlPane& pane)
{
   
    IRender& gl = GetGl();

    CRgbaColor FindColor(0.0f, 0.0f, 0.0f);

    TSeqPos findLen = m_SearchObj.m_LastSearch.length();
    const CSeqTextDefs::TSeqPosVector& foundList = m_SearchObj.GetFoundList();

    if (findLen == 0 || foundList.size() == 0) return;

    CGlPaneGuard GUARD(pane, CGlPane::eOrtho);

    gl.LineWidth(1);
    gl.ColorC(FindColor);
    gl.Begin(GL_LINES);
            
    for (unsigned int i = 0; i < foundList.size(); i++) {
        x_UnderlineInterval(foundList[i], foundList[i] + findLen - 1);

    }
    gl.End();
}

TModelPoint CSeqTextPane::CharacterSize()
{
    TModelPoint size(0.0, 0.0);

    if (m_pSeqFont) {
        size.m_Y = m_pSeqFont->GetMetric(CGlTextureFont::eMetric_FullCharHeight);
        size.m_X = m_pSeqFont->GetMetric(CGlTextureFont::eMetric_MaxCharWidth);
    }
    return size;
}


void CSeqTextPane::STG_GetLineInfo(TSeqPos &chars_in_line, TSeqPos &lines_in_seq)
{
    chars_in_line = 0;
    lines_in_seq = 0;

    CSeqTextDataSource* pDS = x_GetParent()->GetDS();

    if (pDS && m_pSeqFont) {
#if 0
        int sequence_pane_width = m_SequencePane.GetViewport().Width() - 1 - m_SequenceTickGraph->PreferredSize().m_X;
#else
        int sequence_pane_width = m_rcSequence.Width();
#endif

        TModelPoint character_size = CharacterSize();

        TModelUnit num_chars_per_line = 0;
        TModelUnit num_lines = 0;


        if (character_size.m_X > 0) {
            num_chars_per_line = floor (sequence_pane_width / character_size.m_X);
            num_chars_per_line = floor (num_chars_per_line / 10.0) * 10.0;
            chars_in_line = (TSeqPos) num_chars_per_line;
        }
        if (num_chars_per_line > 0 && character_size.m_Y > 0) {
            num_lines = ceil (pDS->GetDataLen () / num_chars_per_line) + 1.0;
            lines_in_seq = (TSeqPos) num_lines;
        }
    }
}


TSeqPos CSeqTextPane::GetScrollAdjustmentForLastLineVariations ()
{
    CSeqTextDataSource* pDS = x_GetParent()->GetDS();
    TSeqPos extra_lines = 0;

    if (pDS && m_pSeqFont) {
        TModelPoint character_size = CharacterSize();
        int sequence_pane_height = m_SequencePane.GetViewport().Height() - 1 - m_BottomRuler.GetPreferredSize(100).Y();
        TSeqPos     chars_in_line, lines_in_seq;
        STG_GetLineInfo(chars_in_line, lines_in_seq);

        if (character_size.m_Y > 0 && lines_in_seq > 1) {
            TSeqPos num_lines_in_page =
                (int)ceil(sequence_pane_height / character_size.m_Y);

            // need to calculate number of lines needed for variations for last page
            CSeqTextDefs::TVariationGraphVector variations;
            TOffsetStart offset_list;
            TSeqPos last_page_first_line;
            if (lines_in_seq > num_lines_in_page - 1) {
                last_page_first_line = lines_in_seq - num_lines_in_page - 1;
            } else {
                last_page_first_line = 0;
            }
            TSeqPos last_page_start = last_page_first_line * chars_in_line;
            pDS->GetVariations(last_page_start, pDS->GetDataLen(), variations);
            x_CalculateOffsetList(variations, offset_list);
            if (offset_list.size() > 0) {
                // calculate which variations aren't seen because they've scrolled off the page
                int i = offset_list.size() - 1;
                while (i > -1 && offset_list[i].m_StartRow > last_page_first_line + extra_lines) {
                    if (i > 0) {
                        extra_lines += offset_list[i].m_CumulativeExtraRows - offset_list[i - 1].m_CumulativeExtraRows;
                    } else {
                        extra_lines += offset_list[i].m_CumulativeExtraRows;
                    }
                    i--;
                }
            }
        }
    }
    return extra_lines;
}


TModelRect CSeqTextPane::GetPreferredModelRect ()
{
    TModelRect size(0.0, 0.0, 1.0, 1.0);
    CSeqTextDataSource* pDS = NULL;

    if (x_GetParent()) {
        pDS = x_GetParent()->GetDS();
    }

    if (pDS && m_pSeqFont) {
        int sequence_pane_width = m_rcSequence.Width();
        int sequence_pane_height = m_rcSequence.Height();

        TModelPoint character_size = CharacterSize();
        TSeqPos     chars_in_line, lines_in_seq;
        STG_GetLineInfo(chars_in_line, lines_in_seq);

        if (character_size.m_X > 0) {
            size.SetRight(sequence_pane_width / character_size.m_X);
        }
        if (character_size.m_Y > 0) {
            TSeqPos lines_in_screen = (TSeqPos) floor(sequence_pane_height / character_size.m_Y);
            TModelUnit top_blank = (sequence_pane_height / character_size.m_Y) - floor (sequence_pane_height / character_size.m_Y);
            TSeqPos model_top = lines_in_seq + GetScrollAdjustmentForLastLineVariations();
            if (model_top < lines_in_screen) {
                model_top = lines_in_screen;
            }
            size.SetTop (model_top + top_blank);
        }
    }
    return size;
}


void CSeqTextPane::ScrollToPosition(TSeqPos pos, bool notify)
{
    TModelRect rc_vis, rc_model;
    TModelUnit new_top;
    TModelUnit scroll_y;
    TSeqPos    chars_in_line, lines_in_seq;

    STG_GetLineInfo(chars_in_line, lines_in_seq);

    pos = max(pos, (TSeqPos)0);
    pos = min(pos, lines_in_seq * chars_in_line);

    rc_vis = m_SequencePane.GetVisibleRect();
    rc_model = m_SequencePane.GetModelLimitsRect();

    scroll_y = floor ((double) pos / (double) chars_in_line);

    new_top = floor(rc_model.Top() - scroll_y);

    TModelUnit d_y = new_top - rc_vis.Top();

    x_GetParent()->Scroll(0, d_y);

    if (notify) {
        // translate the notification to the standard message
        x_GetParent()->NotifyVisibleRangeChanged();
    }
}


void CSeqTextPane::ShowFeaturesInUpperCase()
{
    m_Config->SetShowFeatAsLower(false);
    Refresh();
}


void CSeqTextPane::ShowFeaturesInLowerCase()
{
    m_Config->SetShowFeatAsLower(true);
    Refresh();
}


bool CSeqTextPane::GetShowFeaturesInLowerCase()
{
    return m_Config->GetShowFeatAsLower();
}

void CSeqTextPane::ChooseCaseFeature(CSeqFeatData::ESubtype subtype)
{
    m_Config->SetCaseFeature (subtype);
    Refresh();
 }


int CSeqTextPane::GetCaseFeatureSubtype()
{
    return m_Config->GetCaseFeatureSubtype();
}


void CSeqTextPane::ReloadSettings()
{
    m_Config->Reload();
    Refresh();
}


CGlTextureFont *CSeqTextPane::x_ChooseFontForSize(int font_size)
{
    CGlTextureFont *new_SeqFont = NULL;

    switch (font_size) {
        case 6:
            new_SeqFont = new CGlTextureFont(CGlTextureFont::eFontFace_Helvetica6, 6);
            break;
        case 8:
            new_SeqFont = new CGlTextureFont(CGlTextureFont::eFontFace_BPMono_Bold, 8);
            break;
        case 10:
            new_SeqFont = new CGlTextureFont(CGlTextureFont::eFontFace_BPMono_Bold, 10);
            break;
        case 12:
            new_SeqFont = new CGlTextureFont(CGlTextureFont::eFontFace_BPMono_Bold, 12);
            break;
        case 14:
            new_SeqFont = new CGlTextureFont(CGlTextureFont::eFontFace_BPMono_Bold, 14);
            break;
        case 18:
            new_SeqFont = new CGlTextureFont(CGlTextureFont::eFontFace_BPMono_Bold, 18);
            break;
        case 20:
            new_SeqFont = new CGlTextureFont(CGlTextureFont::eFontFace_BPMono_Bold, 20);
            break;
        case 24:
            new_SeqFont = new CGlTextureFont(CGlTextureFont::eFontFace_BPMono_Bold, 24);
            break;
        default:
            break;
    }
    return new_SeqFont;
}


void CSeqTextPane::SetFontSize (int font_size)
{
    CGlTextureFont *new_SeqFont = x_ChooseFontForSize(font_size);
    if (new_SeqFont == NULL) return;

    m_Config->SetFontSize(font_size);
    m_pSeqFont = new_SeqFont;
    m_SubjectGraph->SetFont(m_pSeqFont);
    m_SequenceTickGraph->SetFont(m_pSeqFont);

    Layout();
    m_SequencePane.SetViewport(m_rcSequence);
    Refresh();
}

int CSeqTextPane::GetFontSize ()
{
    return m_Config->GetFontSize();
}


bool CSeqTextPane::GetShowAbsolutePosition()
{
    return m_Config->GetShowAbsolutePosition();
}


void CSeqTextPane::Configure()
{
    // create and show the Dialog
    CwxSeqTextViewOptions dlg(this, m_Config);

    if(dlg.ShowModal() == ID_SEQTEXTDLG_OK) {
        int font_size = m_Config->GetFontSize();
        m_SequenceTickGraph->ShowAbsolutePosition (m_Config->GetShowAbsolutePosition());
        /* Setting the font size will redraw.  Everything but the font size and absolute position is
         * automatically picked up from the config.
         */
        x_GetParent()->SetFontSize (font_size);
    }
}


void CSeqTextPane::x_CalculateOffsetList(CSeqTextDefs::TVariationGraphVector& variations, TOffsetStart& offset_list)
{
    unsigned int i, k;

    TSeqPos chars_in_line, lines_in_seq;
    STG_GetLineInfo(chars_in_line, lines_in_seq);

    offset_list.clear();

    if (lines_in_seq < 1) {
        return;
    }

    if (variations.size() == 0) {
        return;
    }

    // calculate display lines
    for (i = 0; i < variations.size() - 1; i++) {
        for (k = i + 1;
             k < variations.size() && variations[k].GetFeatLeft() < variations[i].GetFeatLeft() + variations[i].GetDrawWidth();
             k++) {
            variations[k].SetDisplayLine(max (variations[i].GetDisplayLine() + 1, variations[k].GetDisplayLine()));
        }
    }

    // get display rows for each line
    for (i = 0; i < variations.size(); i++) {
        TSeqPos start_pos = variations[i].GetFeatLeft();
        TSeqPos stop_pos = variations[i].GetFeatRight();
        //TSeqPos row_start = start_pos - start_pos % chars_in_line;
        //TSeqPos row_stop = stop_pos - stop_pos % chars_in_line;
        TSeqPos start_row_y = start_pos / chars_in_line;
        TSeqPos stop_row_y = stop_pos / chars_in_line;
        TSeqPos row_index;

        if (offset_list.size() < 1) {
            k = 0;
        } else {
            k = offset_list.size() - 1;
            while (k > 0 && offset_list[k].m_StartRow > start_row_y) {
                k--;
            }
            if (offset_list[k].m_StartRow < start_row_y) {
                k++;
            }
        }
        for (row_index = start_row_y; row_index <= stop_row_y; row_index++) {
            if (k >= offset_list.size()) {
                offset_list.push_back(COffsetStart(row_index, variations[i].GetDisplayLine()));
            } else {
                offset_list[k].m_CumulativeExtraRows = max (offset_list[k].m_CumulativeExtraRows, variations[i].GetDisplayLine());
            }
            k++;
        }
    }

    // now make each offset a cumulative total
    for (i = 1; i < offset_list.size(); i++) {
        offset_list[i].m_CumulativeExtraRows += offset_list[i - 1].m_CumulativeExtraRows;
    }
}


void CSeqTextPane::x_CalculateOffsetList(CSeqTextDefs::TVariationGraphVector& variations)
{
    x_CalculateOffsetList (variations, m_OffsetList);
}


void CSeqTextPane::x_RecalculateOffsets (void)
{
    CSeqTextDefs::TVariationGraphVector variations;

    TSeqPos chars_in_line, lines_in_seq;

    STG_GetLineInfo(chars_in_line, lines_in_seq);

    m_OffsetList.clear();

    TSeqPos seq_start, seq_stop;
    STG_GetVisibleRange (seq_start, seq_stop);

    CSeqTextDataSource* pDS = x_GetParent()->GetDS();
    if (pDS) {
        pDS->GetVariations(seq_start, seq_stop, variations);
        x_CalculateOffsetList (variations);
    }

}


bool CSeqTextPane::TC_NeedTooltip(const wxPoint &pt)
{
    CSeqTextDataSource* pDS = x_GetParent()->GetDS();
    if (pDS) {
        TSeqPos source_pos = STG_GetSequenceByWindow (pt.x, pt.y);
        string tip_text = pDS->GetToolTipForSourcePos (source_pos);
        if (tip_text.empty()) {
            return false;
        } else {
            return true;
        }
    } else {
        return false;
    }
}


string CSeqTextPane::TC_GetTooltip(const wxRect &rc)
{
    CSeqTextDataSource* pDS = x_GetParent()->GetDS();
    if (pDS) {
        TSeqPos source_pos = STG_GetSequenceByWindow (rc.x, rc.y);
        string tip_text = pDS->GetToolTipForSourcePos (source_pos);
        return tip_text;
    } else {
        return "No data source!";
    }
}

// For selection handling
////////////////////////////////////////////////////////////////////////////////
// event handlers

void CSeqTextPane::OnMousePush(wxMouseEvent &event)
{
    m_SelHandler.OnMousePush(event);
    event.Skip();
}


void CSeqTextPane::OnMouseDrag(wxMouseEvent &event)
{
    m_SelHandler.OnMouseDrag(event);
    event.Skip();
}


void CSeqTextPane::OnMouseRelease(wxMouseEvent &event)
{
    m_SelHandler.OnMouseRelease (event);
    event.Skip();
}

void CSeqTextPane::STG_SetDefaultCursor()
{
    SetCursor (wxCURSOR_ARROW);
}


void CSeqTextPane::STG_SetResizeCursor()
{
    SetCursor (wxCURSOR_SIZEWE);
}


END_NCBI_SCOPE
