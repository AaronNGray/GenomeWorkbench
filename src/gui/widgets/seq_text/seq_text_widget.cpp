/*  $Id: seq_text_widget.cpp 35180 2016-04-05 14:49:47Z asztalos $
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


#include <gui/opengl/glfont.hpp>
#include <gui/graph/igraph_utils.hpp>
#include <gui/types.hpp>
//#include <gui/widgets/wx/message_box.hpp>
#include <gui/widgets/wx/wx_utils.hpp>
#include <gui/widgets/wx/ui_command.hpp>

#include <gui/utils/menu_item.hpp>

#include <gui/widgets/seq_text/seq_text_widget.hpp>
#include <gui/widgets/seq_text/seq_text_pane.hpp>

#include <objmgr/util/sequence.hpp>
#include <objmgr/util/feature.hpp>
#include <objmgr/impl/handle_range_map.hpp>

#include <list>

#include <wx/msgdlg.h>
#include <wx/scrolbar.h>
#include <wx/menu.h>

BEGIN_NCBI_SCOPE
USING_SCOPE(ncbi::objects);

////////////////////////////////////////////////////////////////////////////////
/// class CSeqTextWidget

CSeqTextWidget::CSeqTextWidget(wxWindow* parent, wxWindowID id,
                               const wxPoint& pos, const wxSize& size,
                               long style)
    : CGlWidgetBase(parent, id, pos, size, style),
      m_DataSource(NULL),
      m_SequencePane(NULL),
      m_PopupMenuSourcePos(0),
      m_pHost(NULL),
      m_LastSourcePos (0)
{
    // setup Port
    m_Port.SetAdjustmentPolicy(CGlPane::fAdjustAll, CGlPane::fAdjustAll);
    m_Port.SetMinScaleX(1 / 30.0);
    m_Port.SetOriginType(CGlPane::eOriginLeft, CGlPane::eOriginBottom);

    m_Port.EnableZoom (false, false);
}


CSeqTextWidget::~CSeqTextWidget()
{

}


CSeqTextDataSource*  CSeqTextWidget::GetDS(void)
{
    return m_DataSource;
}


void CSeqTextWidget::SetDataSource(CSeqTextDataSource& ds)
{
    CRef<CSeqTextDataSource> guard = m_DataSource; // keep it alive until update is completed
    TModelRect model_rect;

    m_DataSource.Reset(&ds);

    m_LastSourcePos = 0;
    if (m_SequencePane.get() == NULL) {
        x_CreatePane();
    }
    x_Update();
}


static
WX_DEFINE_MENU(sPopupMenu)
    WX_MENU_ITEM(wxID_COPY)
    WX_MENU_ITEM(eCmdZoomSel)
    WX_MENU_ITEM(eCmdSettings)
WX_END_MENU()



BEGIN_EVENT_TABLE(CSeqTextWidget, CGlWidgetBase)
    EVT_MENU(eCmdSettings, CSeqTextWidget::Configure )
    EVT_MENU(eCmdZoomSel, CSeqTextWidget::OnScrollToSelection)
    EVT_MENU(wxID_COPY, CSeqTextWidget::OnCopySelection)
    EVT_UPDATE_UI(wxID_COPY, CSeqTextWidget::OnHasSelection)

    EVT_MENU(wxID_CLEAR, CSeqTextWidget::OnClearSelection)
    EVT_UPDATE_UI(wxID_CLEAR, CSeqTextWidget::OnHasSelection)

    EVT_UPDATE_UI(wxID_PASTE, CSeqTextWidget::OnDisableCommands)
    EVT_UPDATE_UI(wxID_CUT, CSeqTextWidget::OnDisableCommands)
    EVT_UPDATE_UI(wxID_DELETE, CSeqTextWidget::OnDisableCommands)
    EVT_UPDATE_UI(wxID_SELECTALL, CSeqTextWidget::OnDisableCommands)


    EVT_UPDATE_UI(eCmdSettings, CSeqTextWidget::OnUpdateSettings)
END_EVENT_TABLE()

void CSeqTextWidget::OnUpdateSettings(wxUpdateUIEvent& event)
{
    event.Enable(true);
}

void CSeqTextWidget::x_CreatePane()
{
    m_SequencePane.reset(new CSeqTextPane(this));
}


CGlWidgetPane* CSeqTextWidget::x_GetPane()
{
    return static_cast<CGlWidgetPane*>(m_SequencePane.get());
}


void CSeqTextWidget::x_SetPortLimits()
{
    TModelPoint character_size = m_SequencePane->CharacterSize();
    TModelRect rc_model = m_SequencePane->GetPreferredModelRect();
    if (character_size.m_Y > 0 && character_size.m_X > 0)
    {
        rc_model.SetTop (rc_model.Top() + m_SequencePane->GetBottomRulerModelAdjustmentHeight());
        m_Port.SetModelLimitsRect(rc_model);
        TModelUnit scale_y = 1.0 / character_size.m_Y;
        TModelUnit scale_x = 1.0 / character_size.m_X;
        m_Port.SetScale(scale_x, scale_y);
    }
}


void CSeqTextWidget::x_Update()
{
    x_SetPortLimits();

    x_UpdatePane();
    x_UpdateScrollbars();

    x_RedrawControls();
}

void CSeqTextWidget::x_UpdatePane()
{
    m_SequencePane->Update();
}


CGlPane& CSeqTextWidget::GetPort()
{
    return m_Port;
}


const CGlPane& CSeqTextWidget::GetPort() const
{
    return m_Port;
}


void CSeqTextWidget::SetScaleX(TModelUnit scale_x, const TModelPoint& point)
{
}


void CSeqTextWidget::AdjustModelForSequenceAndWindowSize ()
{
    TModelPoint character_size = m_SequencePane->CharacterSize();
    if (character_size.m_Y > 0 && character_size.m_X > 0)
    {
        TModelRect old_limits = m_Port.GetModelLimitsRect ();
        x_SetPortLimits();
        TModelRect new_limits = m_Port.GetModelLimitsRect ();
        if (old_limits.Left() != new_limits.Left()
            || old_limits.Right() != new_limits.Right()
            || old_limits.Top() != new_limits.Top()
            || old_limits.Bottom() != new_limits.Bottom()) {
            x_UpdateScrollbars();
            x_RedrawControls();
        } else {
            x_MakePortMatchScrollValue (x_GetScrollValueForSourcePos(m_LastSourcePos));
        }
    }
}


void CSeqTextWidget::ReportMouseOverPos(TSeqPos pos)
{
    // use host interface to pass information to view
    if (m_pHost != NULL) {
        m_pHost->STWH_ReportMouseOverPos(pos);
    }
    m_PopupMenuSourcePos = pos;
}


int CSeqTextWidget::x_GetVScrollMax ()
{
    int max_val = 0;

    TSeqPos chars_in_line, lines_in_seq;
    m_SequencePane->STG_GetLineInfo(chars_in_line, lines_in_seq);
    max_val = lines_in_seq + m_SequencePane->GetScrollAdjustmentForLastLineVariations();
    return    max_val;
}


TSeqPos CSeqTextWidget::x_GetScrollValueForSourcePos (TSeqPos source_pos)
{
    TSeqPos curr_pos = 0, chars_in_line, lines_in_seq;

    m_SequencePane->STG_GetLineInfo(chars_in_line, lines_in_seq);
    if (chars_in_line > 0) {
        curr_pos = source_pos / chars_in_line;
    }
    return curr_pos;
}


void CSeqTextWidget::x_UpdateScrollbars()
{
    wxScrollBar* scroll_bar_v = (wxScrollBar*)FindWindow(ID_VSCROPLLBAR);
    wxScrollBar* scroll_bar_h = (wxScrollBar*)FindWindow(ID_HSCROPLLBAR);

    if (scroll_bar_v)   {
        TModelRect rcVisible = m_Port.GetVisibleRect();
        TSeqPos curr_pos, max_val;

        curr_pos = x_GetScrollValueForSourcePos(m_LastSourcePos);
        max_val = x_GetVScrollMax ();
        if (curr_pos > max_val) curr_pos = max_val;

        int page_size = (int) (rcVisible.Height() - m_SequencePane->GetBottomRulerModelAdjustmentHeight());
        if (page_size < 0) page_size = 0;
        int thumb_size = page_size;
        if (max_val <= (TSeqPos) page_size) {
//          scroll_bar_v->Hide();
            scroll_bar_v->SetScrollbar (0, 0, 0, 0, true);
        } else {
//          scroll_bar_v->Show();
            scroll_bar_v->SetScrollbar (curr_pos, thumb_size, max_val, page_size, true);
            x_MakePortMatchScrollValue (curr_pos);
        }
    }
    if (scroll_bar_h) {
        scroll_bar_h->Hide();
    }
}


int CSeqTextWidget::GetPortScrollValue ()
{
    int curr_pos = 0;
    TModelRect rcVisible = m_Port.GetVisibleRect();
    TModelRect rcAll = m_Port.GetModelLimitsRect();

    //wxScrollBar* scroll_bar_v = (wxScrollBar*)FindWindow(ID_VSCROPLLBAR);

    curr_pos = (int) ceil(rcAll.Top() - rcVisible.Top());
    return curr_pos;
}


TSeqPos CSeqTextWidget::x_GetSourcePosForPortScrollValue()
{
    int curr_pos = GetPortScrollValue();

    TSeqPos chars_in_line, lines_in_seq;
    m_SequencePane->STG_GetLineInfo(chars_in_line, lines_in_seq);

    return curr_pos * chars_in_line;
}

void CSeqTextWidget::x_MakePortMatchScrollValue (int pos)
{
    TModelRect rcAll = m_Port.GetModelLimitsRect();
    TModelRect rcVisible = m_Port.GetVisibleRect();
    double desired_pos = rcAll.Top() - pos;
    double dY = desired_pos - rcVisible.Top();
    m_Port.Scroll(0, dY);
}


void CSeqTextWidget::x_OnScrollY(int pos)
{
    //wxScrollBar* scroll_bar_v = (wxScrollBar*)FindWindow(ID_VSCROPLLBAR);

    x_MakePortMatchScrollValue (pos);
    x_RedrawControls();

    m_LastSourcePos = x_GetSourcePosForPortScrollValue();

    // translate the notification to the standard message
    CEvent evt(CEvent::eEvent_Message, CViewEvent::eWidgetRangeChanged);
    Send(&evt, ePool_Parent);
}


void CSeqTextWidget::x_OnScrollX(int pos)
{
    // do nothing.  no horizontal scrolling.
}


void CSeqTextWidget::ScrollToSourcePosition(TSeqPos pos, bool notify)
{
    m_SequencePane->ScrollToPosition (pos, notify);
    int scroll_pos = x_GetScrollValueForSourcePos (pos);
    x_SetScrollPosition (scroll_pos);
    m_LastSourcePos = pos;
}


void CSeqTextWidget::ScrollToSequencePosition(TSeqPos pos, bool notify)
{
    bool found_in_source;
    pos = m_SequencePane->STG_GetSourcePosBySequencePos(pos, &found_in_source);

    m_SequencePane->ScrollToPosition (pos, notify);
    int scroll_pos = x_GetScrollValueForSourcePos (pos);
    x_SetScrollPosition (scroll_pos);
    m_LastSourcePos = pos;
}


void CSeqTextWidget::ScrollToPosition(TSeqPos pos, bool notify)
{
    if (m_SequencePane->GetShowAbsolutePosition()) {
        ScrollToSequencePosition (pos, notify);
    } else {
        ScrollToSourcePosition(pos, notify);
    }
}


void CSeqTextWidget::ScrollToText(const string& fragment, TSeqPos start_search)
{
    if (m_DataSource) {
        int source_pos = m_DataSource->FindSequenceFragment(fragment, start_search);
        if (source_pos >= 0) {
            ScrollToSourcePosition (source_pos);
            TSeqRange range;
            range.Set(source_pos, source_pos + fragment.length() - 1);
            TRangeColl segs;
            segs.clear();
            segs.CombineWith(range);
            SetRangeSelection(segs);
        }
    }
}

void CSeqTextWidget::UpdateLastSourcePos()
{
    m_LastSourcePos = x_GetSourcePosForPortScrollValue();
}

void CSeqTextWidget::x_SetScrollPosition (int scroll_pos)
{
    wxScrollBar* scroll_bar_v = (wxScrollBar*)FindWindow(ID_VSCROPLLBAR);

    if (scroll_bar_v) {
        // set scroll position
        int max_val = x_GetVScrollMax ();
        TModelRect rcVisible = m_Port.GetVisibleRect();

        int page_size = (int) (rcVisible.Height() - m_SequencePane->GetBottomRulerModelAdjustmentHeight());
        if (page_size < 0) page_size = 0;
        int thumb_size = page_size;
        if (max_val <= page_size) {
//          scroll_bar_v->Hide();
            scroll_bar_v->SetScrollbar (0, 0, 0, 0, true);
        } else {
//          scroll_bar_v->Show();
            scroll_bar_v->SetScrollbar (scroll_pos, thumb_size, max_val, page_size, true);
            x_MakePortMatchScrollValue (scroll_pos);
        }
    }
}


void CSeqTextWidget::OnScrollToSelection (wxCommandEvent& event)
{
    const CSeqTextWidget::TRangeColl& s_coll = GetSelection();
    TSeqPos first_selection_start = 0;
    TSeqPos translated_start;

    if(! s_coll.empty())  {
        ITERATE(CRangeCollection<TSeqPos>, it_r, s_coll) { // for each range in mark
            first_selection_start = it_r->GetFrom();
            translated_start = m_DataSource->SequencePosToSourcePos (first_selection_start);
            if (first_selection_start == m_DataSource->SourcePosToSequencePos (translated_start)) {
                ScrollToSourcePosition (translated_start);
                return;
            }
        }
    }
}

void CSeqTextWidget::OnCopySelection(wxCommandEvent& event)
{
    _ASSERT(m_DataSource);
    if (!m_DataSource)
        return;
    m_SequencePane->OnCopy();
}

void CSeqTextWidget::OnClearSelection(wxCommandEvent& event)
{
    const CSeqTextWidget::TRangeColl& s_coll = GetSelection();
    if (s_coll.empty())
        return;
    m_SequencePane->ResetRangeSelection();
}

void CSeqTextWidget::OnHasSelection(wxUpdateUIEvent& event)
{
    event.Enable(m_DataSource && !GetSelection().empty());
}

void CSeqTextWidget::OnDisableCommands(wxUpdateUIEvent& event)
{
    event.Enable(false);
}


const CSeqTextWidget::TRangeColl&
    CSeqTextWidget::GetSelection() const
{
    return m_SequencePane->GetSelection();
}


void   CSeqTextWidget::SetRangeSelection(const TRangeColl& segs)
{
    m_SequencePane->SetRangeSelection(segs);
}


void    CSeqTextWidget::DeSelectObject(const CObject* obj)
{
    m_SequencePane->DeSelectObject (obj);
}


void    CSeqTextWidget::SelectObject(const CObject* obj)
{
    m_SequencePane->SelectObject (obj);
}


void    CSeqTextWidget::ResetObjectSelection()
{
    m_SequencePane->ResetObjectSelection();
}


CSeq_loc* CSeqTextWidget::GetVisibleRange()
{
    if (m_DataSource) {
        TSeqPos seq_start, seq_stop;
        m_SequencePane->STG_GetVisibleRange (seq_start, seq_stop);

        CSeq_loc* visible_range = new CSeq_loc();

        visible_range->SetInt().SetFrom(m_DataSource->SourcePosToSequencePos (seq_start));
        visible_range->SetInt().SetTo(m_DataSource->SourcePosToSequencePos (seq_stop));
        visible_range->SetId(*(m_DataSource->GetId()));
        return visible_range;
    } else {
        return NULL;
    }
}


// Functions for configuring the widget

// configure from menu
void CSeqTextWidget::Configure(wxCommandEvent& event)
{
    m_SequencePane->Configure();
}


void CSeqTextWidget::ChooseCaseFeature(CSeqFeatData::ESubtype subtype)
{
    m_SequencePane->ChooseCaseFeature(subtype);
}


int CSeqTextWidget::GetCaseFeatureSubtype()
{
    return m_SequencePane->GetCaseFeatureSubtype();
}


void CSeqTextWidget::SetFontSize(int font_size)
{
    m_SequencePane->SetFontSize(font_size);
    AdjustModelForSequenceAndWindowSize();
}


int CSeqTextWidget::GetFontSize()
{
    return m_SequencePane->GetFontSize();
}


void CSeqTextWidget::SetHost(ISeqTextWidgetHost* pHost)
{
    m_pHost = pHost;
}


void CSeqTextWidget::ChangeConfig()
{
    // use host interface to pass information to view
    if (m_pHost != NULL) {
        m_pHost->STWH_ChangeConfig();
    }
}


void CSeqTextWidget::ShowFeaturesInLowerCase(bool do_lower)
{
   if (do_lower) {
       m_SequencePane->ShowFeaturesInLowerCase();
   } else {
       m_SequencePane->ShowFeaturesInUpperCase();
   }
}

bool CSeqTextWidget::GetShowFeaturesInLowerCase()
{
    return m_SequencePane->GetShowFeaturesInLowerCase();
}


// Popup menu when right-clicking widget
void CSeqTextWidget::OnShowPopup()
{
    CUICommandRegistry& cmd_reg = CUICommandRegistry::GetInstance();
    auto_ptr<wxMenu> root(cmd_reg.CreateMenu(sPopupMenu));
    PopupMenu(root.get());
}





END_NCBI_SCOPE
