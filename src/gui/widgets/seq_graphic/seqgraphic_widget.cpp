/*  $Id: seqgraphic_widget.cpp 44402 2019-12-12 18:12:05Z filippov $
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
#include <gui/types.hpp>

#include <gui/widgets/seq_graphic/seqgraphic_widget.hpp>
#include <gui/widgets/seq_graphic/seqgraphic_genbank_ds.hpp>
#include <gui/widgets/seq_graphic/seqgraphic_conf.hpp>
#include <gui/widgets/seq_graphic/feature_track.hpp>

#include <gui/widgets/data/data_commands.hpp>
#include <gui/widgets/aln_score/properties_panel_dlg.hpp>
#include <gui/widgets/seq/sequence_goto_dlg.hpp>

#include <gui/widgets/wx/ui_tool.hpp>
#include <gui/widgets/wx/ui_tool_registry.hpp>
#include <gui/widgets/wx/ui_command.hpp>
#include <gui/widgets/wx/commands.hpp>
#include <gui/widgets/wx/sticky_tooltip_wnd.hpp>
#include <gui/widgets/wx/message_box.hpp>
#include <gui/widgets/wx/wx_utils.hpp>
#include <gui/widgets/wx/ieditor_factory.hpp>

#include <misc/hgvs/objcoords.hpp>
#include <gui/objutils/edit_event.hpp>

#include <gui/utils/menu_item.hpp>

#include <gui/utils/extension_impl.hpp>

#include "wx_hairline_dlg.hpp"
#include "wx_seqmarker_set_dlg.hpp"
#include "wx_track_name_dlg.hpp"
#include "wx_track_settings_dlg.hpp"
#include "wx_marker_name_dlg.hpp"
#include "seqgraphic_pane.hpp"
#include "wx_marker_info_dlg.hpp"
#include "wx_export_vector_image_dlg.hpp"

#include <wx/menu.h>


BEGIN_NCBI_SCOPE
USING_SCOPE(objects);

const TVPUnit    kMaxPixelsBase   = 24;

void CSeqGraphicWidget::RegisterCommands(CUICommandRegistry& cmd_reg,
                                         wxFileArtProvider& provider)
{
    WidgetsData_RegisterCommands(cmd_reg, provider);

    static bool initialized = false;
    if( ! initialized)  {
        cmd_reg.RegisterCommand(eCmdPrevSplice,
            "Feature: Previous Splice", "Feature: Previous Splice", "", "");
        cmd_reg.RegisterCommand(eCmdNextSplice,
            "Feature: Next Splice", "Feature: Next Splice", "", "");
        cmd_reg.RegisterCommand(eCmdAdjustRuler,
            "&Adjust Ruler To Follow Selection", "&Adjust Ruler To Follow Selection", "", "", "", "", wxITEM_CHECK );
        cmd_reg.RegisterCommand(eCmdSetSeqStart,
            "Set Sequence Start Here", "Set Sequence Start Here", "", "");
        cmd_reg.RegisterCommand(eCmdSetMarkerDlg,
            "Set New Marker At Position...", "Set New Marker At Position...", "", "");
        cmd_reg.RegisterCommand(eCmdSetMarkerForSelDlg,
            "Set New Marker for Selection...", "Set New Marker for Selection...", "", "");
        cmd_reg.RegisterCommand(eCmdRenameMarkerDlg,
            "Rename Marker...", "Rename This Marker...", "", "");
        cmd_reg.RegisterCommand(eCmdModifyMarkerDlg,
            "Modify Marker Name/Position/Color...", "Modfiy Marker Name/Position/Color...", "", "");
        cmd_reg.RegisterCommand(eCmdMarkerDetailsDlg,
            "Marker Details...", "Show Marker Details...", "", "");
        cmd_reg.RegisterCommand(eCmdZoomAtMarker,
            "Zoom To Sequence At Marker", "Zoome To Sequence Around Marker", "", "");
        cmd_reg.RegisterCommand(eCmdSetSeqStartMarker,
            "Set Sequence Start At Marker", "Set Sequence Start At Marker", "", "");
        cmd_reg.RegisterCommand(eCmdGoToMarker,
            "Center The View At Marker Position", "Center The View At Marker Position", "", "");
        cmd_reg.RegisterCommand(eCmdRemoveMarker,
            "Remove Marker", "Remove Marker", "", "");
        cmd_reg.RegisterCommand(eCmdResetMarker,
            "Remove All Markers", "Remove All Marker", "", "");
        cmd_reg.RegisterCommand(eCmdResetSeqStart,
            "Reset Sequence Start", "Reset Sequence Start", "", "");
        cmd_reg.RegisterCommand(eCmdSetMarker,
            "Set Marker Here", "Set Marker Here", "", "");
        cmd_reg.RegisterCommand(eCmdSetSwitchPoint,
            "Set Segment Switch Point At Marker", "Set Segment Switch Point At Marker", "", "");
        cmd_reg.RegisterCommand(eCmdHairlineOptions,
            "&Interval Rendering Options...", "Interval Rendering Options", "", "");
        cmd_reg.RegisterCommand(eCmdAnnotConfig,
            "&Annotation Rendering Options...", "Annotation Rendering Options", "", "");
        cmd_reg.RegisterCommand(eCmdConfigChanged,
            "Config Changed", "Config Changed", "", "");
        cmd_reg.RegisterCommand(eCmdChangeViewMode,
            "Vertical View Mode", "Vertical View Mode", "", "");
        cmd_reg.RegisterCommand(eCmdFlipStrand,
            "Flip Strands", "Flip Strands", "", "", "", "", wxITEM_CHECK);
        cmd_reg.RegisterCommand(eCmdStrandFlipped,
            "Strands Flipped", "Strands Flipped", "", "");

        cmd_reg.RegisterCommand(eCmdLoadDefaultTracks,
            "&Load Default Tracks ", "Reset Tracks", "", "",
            "Reset Current Tracks to the Original Default Track List");
        cmd_reg.RegisterCommand(eCmdCollapseAllTracks,
            "&Collapse All Tracks", "Collapse All Tracks", "", "");
        cmd_reg.RegisterCommand(eCmdExpandAllTracks,
            "&Expand All Tracks", "Expand All Tracks", "", "");
        cmd_reg.RegisterCommand(eCmdShowAllTracks,
            "&Show All Tracks", "Show All Tracks", "", "");
        cmd_reg.RegisterCommand(eCmdHideAllTracks,
            "&Hide All Tracks", "Hide All Tracks", "", "");
        cmd_reg.RegisterCommand(eCmdCollapseAllSubtracks,
            "Collapse All Subtracks", "Collapse All Subtracks", "", "");
        cmd_reg.RegisterCommand(eCmdExpandAllSubtracks,
            "Expand All Subtracks", "Expand All Subtracks", "", "");
        cmd_reg.RegisterCommand(eCmdMoveTrackToTop,
            "Move to the Top", "Move the Selected Track to the Top", "", "");
        cmd_reg.RegisterCommand(eCmdMoveTrackToBottom,
            "Move to the Bottom", "Move the Selected Track to the Bottom", "", "");
        cmd_reg.RegisterCommand(eCmdCloneTrack,
            "Clone Me", "Clone the Selected Track", "", "");
        cmd_reg.RegisterCommand(eCmdRemoveTrack,
            "Remove Me", "Remove the Selected Track", "", "");
        cmd_reg.RegisterCommand(eCmdAddTrack,
            "Add a New Track", "Add a New Track to the Bottom", "", "");
        cmd_reg.RegisterCommand(eCmdRenameTrack,
            "Rename me", "Rename a Track Using a Different Name", "", "");
        cmd_reg.RegisterCommand(eCmdTrackSettings,
            "Track Settings", "Track Settings", "", "");
        cmd_reg.RegisterCommand(eCmdHideTrack,
            "Hide Track", "Hide Track", "", "");
        cmd_reg.RegisterCommand(eCmdRemoveTrackFromOverlay,
            "Remove Track", "REmove Track from Overlay", "", "");

        initialized = true;
    }
}

static
WX_DEFINE_MENU(sRulerPopupMenuOnMarker)
    WX_MENU_SEPARATOR_L("Top Actions")
    WX_MENU_SEPARATOR_L("Ruler")
    WX_MENU_ITEM(eCmdRenameMarkerDlg)
    WX_MENU_ITEM(eCmdModifyMarkerDlg)
    WX_MENU_ITEM(eCmdMarkerDetailsDlg)
    WX_MENU_ITEM(eCmdRemoveMarker)
    WX_MENU_ITEM(eCmdSetSeqStartMarker)
    WX_MENU_ITEM(eCmdGoToMarker)
    WX_MENU_ITEM(eCmdZoomAtMarker)
    WX_MENU_SEPARATOR()
    WX_MENU_ITEM(eCmdResetMarker)
    WX_MENU_ITEM(eCmdResetSeqStart)
    WX_MENU_SEPARATOR()
    WX_MENU_CHECK_ITEM(eCmdAdjustRuler)
    //WX_MENU_SEPARATOR()
    //WX_MENU_ITEM(eCmdSetSwitchPoint)
WX_END_MENU()

static
WX_DEFINE_MENU(sRulerPopupMenuNotOnMarker)
    WX_MENU_SEPARATOR_L("Top Actions")
    WX_MENU_SEPARATOR_L("Ruler")
    WX_MENU_ITEM(eCmdSetMarker)
    WX_MENU_ITEM(eCmdSetMarkerDlg)
    WX_MENU_ITEM(eCmdSetMarkerForSelDlg)
    WX_MENU_ITEM(eCmdSetSeqStart)
    WX_MENU_SEPARATOR()
    WX_MENU_ITEM(eCmdResetMarker)
    WX_MENU_ITEM(eCmdResetSeqStart)
    WX_MENU_SEPARATOR()
    WX_MENU_CHECK_ITEM(eCmdAdjustRuler)
WX_END_MENU()

static
WX_DEFINE_MENU(sFeatPanePopupMenu)
    WX_MENU_SEPARATOR_L("Top Actions")
    WX_MENU_SEPARATOR_L("Ruler")
    WX_SUBMENU("Ruler")
        WX_MENU_ITEM(eCmdSetMarker)
        WX_MENU_ITEM(eCmdSetMarkerDlg)
        WX_MENU_ITEM(eCmdSetMarkerForSelDlg)
        WX_MENU_ITEM(eCmdSetSeqStart)
        WX_MENU_SEPARATOR()
        WX_MENU_ITEM(eCmdResetMarker)
        WX_MENU_ITEM(eCmdResetSeqStart)
        WX_MENU_SEPARATOR()
        WX_MENU_CHECK_ITEM(eCmdAdjustRuler)
        //WX_MENU_SEPARATOR()
        //WX_MENU_ITEM(eCmdSetSwitchPoint)
    WX_END_SUBMENU()
    WX_MENU_SEPARATOR_L("Zoom")
    WX_SUBMENU("&Zoom")
        WX_MENU_ITEM(eCmdZoomInMouse)
        WX_MENU_ITEM(eCmdZoomOutMouse)
        WX_MENU_ITEM(eCmdZoomAll)
        WX_MENU_ITEM(eCmdZoomSeq)
        WX_MENU_ITEM(eCmdZoomSel)
        WX_MENU_ITEM(eCmdZoomSelObjects)
    WX_END_SUBMENU()
    WX_MENU_SEPARATOR_L("Actions")
    WX_MENU_ITEM(eCmdSeqGoto)
    WX_MENU_SEPARATOR()
    WX_MENU_ITEM(eCmdBack)
    WX_MENU_ITEM(eCmdForward)
    WX_MENU_ITEM(eCmdPrevSplice)
    WX_MENU_ITEM(eCmdNextSplice)
    WX_MENU_SEPARATOR_L("Settings")
    //MENU_ITEM_CHECK(eCmdChangeViewMode, "Vertical View Mode")
    WX_MENU_ITEM(eCmdFlipStrand)
    WX_MENU_ITEM(eCmdHairlineOptions)
    //WX_MENU_ITEM(eCmdAnnotConfig)
    WX_MENU_SEPARATOR_L("Tracks")
    WX_MENU_ITEM(eCmdLoadDefaultTracks)
    WX_MENU_ITEM(eCmdCollapseAllTracks)
    WX_MENU_ITEM(eCmdExpandAllTracks)
    WX_MENU_ITEM(eCmdShowAllTracks)
    WX_MENU_ITEM(eCmdHideAllTracks)
WX_END_MENU()


BEGIN_EVENT_TABLE(CSeqGraphicWidget, CGlWidgetBase)
    EVT_SIZE(CSeqGraphicWidget::OnSize)

    EVT_SCROLL_THUMBRELEASE(CSeqGraphicWidget::OnScrollEnd)

    EVT_CONTEXT_MENU(CSeqGraphicWidget::OnContextMenu)
    EVT_MENU(eCmdSavePdf, CSeqGraphicWidget::OnSavePdf)
    EVT_UPDATE_UI(eCmdSavePdf, CSeqGraphicWidget::OnEnableSavePdfCmdUpdate)
    EVT_MENU(eCmdSaveSvg, CSeqGraphicWidget::OnSaveSvg)
    EVT_UPDATE_UI(eCmdSaveSvg, CSeqGraphicWidget::OnEnableSaveSvgCmdUpdate)
    EVT_MENU(eCmdZoomSeq,    CSeqGraphicWidget::OnZoomSequence)
    EVT_MENU(eCmdZoomSeq,    CSeqGraphicWidget::OnZoomSequence)
    EVT_MENU(eCmdZoomSelObjects, CSeqGraphicWidget::OnZoomObject)
    
    EVT_MENU(eCmdZoomSel,    CSeqGraphicWidget::OnZoomSelection)

    EVT_UPDATE_UI(eCmdZoomSel, CSeqGraphicWidget::OnUpdateZoomSelection)
    EVT_UPDATE_UI(eCmdZoomSelObjects, CSeqGraphicWidget::OnUpdateZoomObject)

    EVT_MENU(eCmdSetMarker, CSeqGraphicWidget::OnSetMarker)
    EVT_MENU(eCmdSetMarkerDlg, CSeqGraphicWidget::OnSetSeqMarkerDlg)
    EVT_MENU(eCmdSetMarkerForSelDlg, CSeqGraphicWidget::OnSetSeqMarkerForSelDlg)
    EVT_UPDATE_UI(eCmdSetMarkerForSelDlg, CSeqGraphicWidget::OnUpdateSetSeqMarkerForSelDlg)

    EVT_MENU(eCmdRenameMarkerDlg, CSeqGraphicWidget::OnRenameMarkerDlg)
    EVT_MENU(eCmdModifyMarkerDlg, CSeqGraphicWidget::OnModifyMarkerDlg)
    EVT_MENU(eCmdMarkerDetailsDlg, CSeqGraphicWidget::OnMarkerDetailsDlg)
    EVT_MENU(eCmdZoomAtMarker, CSeqGraphicWidget::OnZoomAtMarker)
    EVT_MENU(eCmdGoToMarker, CSeqGraphicWidget::OnGoToMarker)
    EVT_MENU(eCmdRemoveMarker, CSeqGraphicWidget::OnRemoveMarker)
    //EVT_MENU(eCmdSetSwitchPoint, CSeqGraphicWidget::OnSetSwitchPoint)

    EVT_MENU(eCmdAdjustRuler, CSeqGraphicWidget::OnAdjustRuler)
    EVT_UPDATE_UI(eCmdAdjustRuler, CSeqGraphicWidget::OnUpdateAdjustRuler)

    EVT_MENU(eCmdSetSeqStart, CSeqGraphicWidget::OnSetSeqStart)
    EVT_MENU(eCmdSetSeqStartMarker, CSeqGraphicWidget::OnSetSeqStartMarker)

    EVT_MENU(eCmdResetMarker, CSeqGraphicWidget::OnRemoveAllMarkersCmd)
    EVT_MENU(eCmdResetSeqStart, CSeqGraphicWidget::OnResetSeqStart)

    EVT_MENU(eCmdHairlineOptions, CSeqGraphicWidget::OnHairlineOptions)

    EVT_MENU(eCmdSeqGoto, CSeqGraphicWidget::OnSeqGotoCmd)
    EVT_MENU(eCmdBack, CSeqGraphicWidget::OnGoBack)
    EVT_MENU(eCmdForward, CSeqGraphicWidget::OnGoForward)
    EVT_UPDATE_UI(eCmdBack, CSeqGraphicWidget::OnUpdateGoBack)
    EVT_UPDATE_UI(eCmdForward, CSeqGraphicWidget::OnUpdateGoForward)

    EVT_MENU(eCmdChangeViewMode, CSeqGraphicWidget::OnVertical)
    EVT_MENU(eCmdFlipStrand, CSeqGraphicWidget::OnFlipStrands)
    EVT_UPDATE_UI(eCmdChangeViewMode, CSeqGraphicWidget::OnUpdateVertical)
    EVT_UPDATE_UI(eCmdFlipStrand, CSeqGraphicWidget::OnUpdateFlipStrands)

    //EVT_MENU(eCmdCut, CSeqGraphicWidget::OnEditCut)
    //EVT_MENU(eCmdCopy, CSeqGraphicWidget::OnEditCopy)
    //EVT_MENU(eCmdPaste, CSeqGraphicWidget::OnEditPaste)

    EVT_MENU(eCmdPrevSplice, CSeqGraphicWidget::OnPrevSplice)
    EVT_MENU(eCmdNextSplice, CSeqGraphicWidget::OnNextSplice)

    EVT_MENU(eCmdLoadDefaultTracks, CSeqGraphicWidget::OnLoadDefaultTracks)
    EVT_MENU(eCmdCollapseAllTracks, CSeqGraphicWidget::OnCollapseAllTracks)
    EVT_MENU(eCmdExpandAllTracks, CSeqGraphicWidget::OnExpandAllTracks)

    EVT_MENU(eCmdShowAllTracks, CSeqGraphicWidget::OnShowAllTracks)
    EVT_MENU(eCmdHideAllTracks, CSeqGraphicWidget::OnHideAllTracks)
    EVT_MENU(eCmdCollapseAllSubtracks, CSeqGraphicWidget::OnCollapseAllSubtracks)
    EVT_MENU(eCmdExpandAllSubtracks, CSeqGraphicWidget::OnExpandAllSubtracks)
    EVT_MENU(eCmdMoveTrackToBottom, CSeqGraphicWidget::OnMoveTrackToBottom)
    EVT_MENU(eCmdMoveTrackToTop, CSeqGraphicWidget::OnMoveTrackToTop)

    EVT_MENU(eCmdCloneTrack, CSeqGraphicWidget::OnCloneTrack)
    EVT_MENU(eCmdRemoveTrack, CSeqGraphicWidget::OnRemoveTrack)
    EVT_MENU(eCmdAddTrack, CSeqGraphicWidget::OnAddTrack)

    EVT_MENU(eCmdRenameTrack, CSeqGraphicWidget::OnRenameTrack)


    //,
    //ON_EVENT(CSeq_featChangedEvent, CEditEvent::eEditEvent_Feature,
    //         CSeqGraphicWidget::OnEditFeature)

    //EVT_UPDATE_UI(eCmdSetSwitchPoint, CSeqGraphicWidget::OnUpdateSetSwitchPoint)

    // Modeless GoTo dialog
    EVT_SEQ_GOTO( wxID_ANY, CSeqGraphicWidget::OnSeqGoto)
    EVT_SEQ_GOTO_CLOSE( wxID_ANY, CSeqGraphicWidget::OnSeqGotoClose)
    EVT_REMOVE_ALL_MARKERS(wxID_ANY, CSeqGraphicWidget::OnRemoveAllMarkers)
    EVT_REMOVE_MARKER(wxID_ANY, CSeqGraphicWidget::OnRemoveMarker_Info)
    EVT_MARKER_INFO_CLOSE(wxID_ANY, CSeqGraphicWidget::OnMarkerInfoDlgClose)
    

    EVT_BUTTON(eCmdSearchTip, CSeqGraphicWidget::OnSearchTip)
    EVT_BUTTON(eCmdZoomTip, CSeqGraphicWidget::OnZoomTip)
    EVT_BUTTON(eCmdInfoTip, CSeqGraphicWidget::OnInfoTip)
    EVT_BUTTON(eCmdTipActive, CSeqGraphicWidget::OnTipActivated)
    EVT_BUTTON(eCmdTipInactive, CSeqGraphicWidget::OnTipDeactivated)
    EVT_BUTTON(eCmdMoveTip, CSeqGraphicWidget::OnMoveTip)
    EVT_BUTTON(eCmdMoveTip, CSeqGraphicWidget::OnMoveTip)
    EVT_BUTTON(eCmdTipAdded, CSeqGraphicWidget::OnTipAdded)
    EVT_BUTTON(eCmdTipRemoved, CSeqGraphicWidget::OnTipRemoved)
    
END_EVENT_TABLE()


/*void  CSeqGraphicWidget::SetViewPopupMenuItems(CMenuItem* itm_view,
                                               CMenuItem* itm_default)
{
    m_pViewPopupItems = itm_view;
    m_pViewDefaultPopupItems = itm_default;
}*/



////////////////
CGlPane& CSeqGraphicWidget::GetPort(void)
{
    return m_Port;
}


const CGlPane& CSeqGraphicWidget::GetPort(void) const
{
    return m_Port;
}


void CSeqGraphicWidget::OnDataChanging()
{
    m_SeqGraphicPane->OnDataChanging();
}


void CSeqGraphicWidget::OnDataChanged()
{
    CStopWatch sw;
    sw.Start();
    m_SeqGraphicPane->OnDataChanged();
    sw.Stop();

    string info = "Timing: " + sw.AsSmartString(CTimeSpan::eSSP_Millisecond)
        + ". Graphical view OnDataChanged.";
    LOG_POST(Info << info);
}


void CSeqGraphicWidget::OnUpdateSetSwitchPoint(wxUpdateUIEvent& event)
{
    event.Enable(m_SeqGraphicPane->CanSetSwitchPoint());
}


void CSeqGraphicWidget::OnUpdateVertical(wxUpdateUIEvent& event)
{
    event.Check(!IsHorizontal());
}


void CSeqGraphicWidget::OnUpdateFlipStrands(wxUpdateUIEvent& event)
{
    event.Check(IsFlipped());
}


void CSeqGraphicWidget::OnUpdateZoomSelection(wxUpdateUIEvent& event)
{
    event.Enable(m_SeqGraphicPane->CanZoomSelection());
}


void CSeqGraphicWidget::OnUpdateZoomObject(wxUpdateUIEvent& event)
{
    event.Enable(m_SeqGraphicPane->GetSelectedLayoutObj().NotNull());
}


void CSeqGraphicWidget::OnUpdateGoBack(wxUpdateUIEvent& event)
{
    event.Enable(m_SeqGraphicPane->CanGoBack());
}


void CSeqGraphicWidget::OnUpdateGoForward(wxUpdateUIEvent& event)
{
    event.Enable(m_SeqGraphicPane->CanGoForward());
}



void CSeqGraphicWidget::OnSeqGotoCmd(wxCommandEvent& /*event*/)
{
    wxWindow* tmp = this;
    wxWindow* parent = NULL;
    while (tmp) {
        parent = tmp;
        tmp = tmp->GetParent();
    }

    if (!m_SequenceGotoDlg) {
        m_SequenceGotoDlg = new CSequenceGotoDlg(parent, &m_SequenceGotoData, this);
        m_SequenceGotoDlg->SetFocus();
        m_SequenceGotoDlg->Raise();
        m_SequenceGotoDlg->Show(true);
        m_SequenceGotoDlg->SetFocusToInputBox();
    }
}


void CSeqGraphicWidget::OnSeqGoto( CSequenceGotoEvent& /*event*/ )
{
    long from, to;
    if (m_SequenceGotoData.GetRange(from, to)) {
        TSeqPos len = GetDataSource()->GetSequenceLength();

        if (from < (long)1  ||  to > (long)len) {
            string msg = "Invalid range: Sequence positions should be from 1 to ";
            msg += NStr::NumericToString(len);
            msg += "!";
            NcbiMessageBox(msg);
        } else {
            // convert to 0-based. We assume user inputs are 1-based. 
            if (from > 0) from -= 1;
            if (to > (long)len) to = len;
            to -= 1;
            ZoomOnRange(TSeqRange(from, to), fSaveRange);
            if (from + 1 >= to) {
                m_SeqGraphicPane->SetSeqMarker(TModelPoint(from, 0.0));
            }
        }
    }
}


void CSeqGraphicWidget::OnSeqGotoClose( CSequenceGotoEvent& /*event*/ )
{
    m_SequenceGotoDlg->Destroy();
    m_SequenceGotoDlg = NULL;
}


void CSeqGraphicWidget::OnRemoveAllMarkers(CSeqMarkerDlgEvent& /*event*/)
{
    x_RemoveAllMarkers();
}


void CSeqGraphicWidget::OnRemoveMarker_Info(CSeqMarkerEvent& event)
{
    m_SeqGraphicPane->RemoveMarker(event.GetMarkerId());
    x_RedrawControls();
}


void CSeqGraphicWidget::OnMarkerInfoDlgClose(CSeqMarkerDlgEvent& event)
{
    if (m_MarkerInfoDlg) {
        m_MarkerInfoDlg->Destroy();
        m_MarkerInfoDlg = NULL;
    }
}


void CSeqGraphicWidget::OnSearchTip(wxCommandEvent& evt)
{
    wxEvtHandler* obj = dynamic_cast<wxEvtHandler*>(evt.GetEventObject());
    if (obj != NULL) {
        CTooltipFrame* f = static_cast<CTooltipFrame*>(obj->GetClientData());
        std::string tip_id = f->GetTipInfo().GetTipID();
        m_SeqGraphicPane->OnSearchTip(tip_id, f->GetScreenRect());
    }
}


void CSeqGraphicWidget::OnZoomTip(wxCommandEvent& evt)
{
    wxEvtHandler* obj = dynamic_cast<wxEvtHandler*>(evt.GetEventObject());
    if (obj != NULL) {
        CTooltipFrame* f = static_cast<CTooltipFrame*>(obj->GetClientData());
        std::string tip_id = f->GetTipInfo().GetTipID();
        m_SeqGraphicPane->OnZoomTip(tip_id, f->GetScreenRect());
    }
}


void CSeqGraphicWidget::OnInfoTip(wxCommandEvent& evt)
{
    wxEvtHandler* obj = dynamic_cast<wxEvtHandler*>(evt.GetEventObject());
    if (obj != NULL) {
        CTooltipFrame* f = static_cast<CTooltipFrame*>(obj->GetClientData());
        std::string tip_id = f->GetTipInfo().GetTipID();
        m_SeqGraphicPane->OnInfoTip(tip_id);
    }
}


void CSeqGraphicWidget::OnTipActivated(wxCommandEvent& evt)
{
    wxEvtHandler* obj = dynamic_cast<wxEvtHandler*>(evt.GetEventObject());
    if (obj != NULL) {
        CTooltipFrame* f = static_cast<CTooltipFrame*>(obj->GetClientData());
        string tip_id = f->GetTipInfo().GetTipID();
        m_SeqGraphicPane->SetActivatedTip(tip_id);
        Refresh();
    }
}


void CSeqGraphicWidget::OnTipDeactivated(wxCommandEvent& evt)
{
    wxEvtHandler* obj = dynamic_cast<wxEvtHandler*>(evt.GetEventObject());
    if (obj != NULL) {
        m_SeqGraphicPane->SetActivatedTip("");
        Refresh();
    }
}


void CSeqGraphicWidget::OnTipAdded(wxCommandEvent& evt)
{
    wxEvtHandler* obj = dynamic_cast<wxEvtHandler*>(evt.GetEventObject());
    if (obj != NULL) {
        CTooltipFrame* f = static_cast<CTooltipFrame*>(obj->GetClientData());
        m_SeqGraphicPane->OnTipAdded(f->GetTipInfo().GetTipID());
    }
}


void CSeqGraphicWidget::OnTipRemoved(wxCommandEvent& evt)
{
    wxEvtHandler* obj = dynamic_cast<wxEvtHandler*>(evt.GetEventObject());
    if (obj != NULL) {
        CTooltipFrame* f = static_cast<CTooltipFrame*>(obj->GetClientData());
        m_SeqGraphicPane->OnTipRemoved(f->GetTipInfo().GetTipID());
    }
}


void CSeqGraphicWidget::OnMoveTip(wxCommandEvent& /*evt*/)
{
    Refresh();
}


void CSeqGraphicWidget::OnGoBack(wxCommandEvent& /*event*/)
{
    m_SeqGraphicPane->GoBack();
}


void CSeqGraphicWidget::OnGoForward(wxCommandEvent& /*event*/)
{
    m_SeqGraphicPane->GoForward();
}


void CSeqGraphicWidget::OnVertical(wxCommandEvent& /*event*/)
{
    SetHorizontal(!IsHorizontal(), IsFlipped());
}


void CSeqGraphicWidget::OnFlipStrands(wxCommandEvent& /*event*/)
{
    SetHorizontal(IsHorizontal(), !IsFlipped());

    CEvent evt(eCmdStrandFlipped);
    Send(&evt, CEventHandler::eDispatch_Default, ePool_Parent);
}


void CSeqGraphicWidget::OnHairlineOptions(wxCommandEvent& /*event*/)
{
    CwxHairlineDlg dlg(this);
    dlg.SetConfig( GetConfig() );
    if(dlg.ShowModal() == wxID_OK) {
        x_RedrawControls();
        CRef<CSeqGraphicConfig> config = GetConfig();
        config->SetDirty(true);
    }
}

// OnViewProperties
//
//void CSeqGraphicWidget::OnViewProperties(wxCommandEvent& event)
//{
//    CwxSettingsDlg dlg(this);	
//    dlg.SetConfig( GetConfig() );
//    if(dlg.ShowModal() == wxID_OK) {	
//        UpdateConfig();
//		x_RedrawControls();
//    }
//}


void CSeqGraphicWidget::OnSetSeqMarkerDlg(wxCommandEvent& event)
{
    CwxSeqMarkerSetDlg dlg(this);
    dlg.SetDlgTitle("Add New Marker");
    dlg.SetMarkerName(m_SeqGraphicPane->GetNewMarkerName());
    srand(time(NULL));
    int r = rand() % 256;
    int g = rand() % 256;
    int b = rand() % 256;
    CRgbaColor color(r, g, b);
    color.Darken(0.3f); // make sure it is not too light
    dlg.SetMarkerColor(color);
    dlg.SetMarkerPos((TSeqPos)m_PopupPoint.X());
    if(dlg.ShowModal() == wxID_OK) {
        if (dlg.GetIsRange()) {
            m_SeqGraphicPane->AddRangeMarker(dlg.GetMarkerName(),
                dlg.GetMarkerRange(), dlg.GetMarkerColor());
        }
        else {
            m_SeqGraphicPane->AddPointMarker(dlg.GetMarkerName(),
                dlg.GetMarkerPos(), dlg.GetMarkerColor());
        }
        x_RedrawControls();
    }
}

void CSeqGraphicWidget::OnSetSeqMarkerForSelDlg(wxCommandEvent& event)
{
    // Get the current range from the right click that brought up the dialog
    CSeqGraphicPane::TRangeColl sel =
        m_SeqGraphicPane->GetRangeSelection();

    TSeqPos pos = TSeqPos(m_PopupPoint.X());
    TSeqRange range_pos(pos, pos);
    CSeqGraphicPane::TRangeColl::const_iterator iter = sel.find(pos);

    if (sel.Contains(range_pos)) {        
        if (iter != sel.end())
            range_pos.Set((*iter).GetFrom(), (*iter).GetTo());
    }

    CwxSeqMarkerSetDlg dlg(this);
    dlg.SetDlgTitle("Add New Marker for Selection");
    dlg.SetMarkerName(m_SeqGraphicPane->GetNewMarkerName());
    srand(time(NULL));
    int r = rand() % 256;
    int g = rand() % 256;
    int b = rand() % 256;
    CRgbaColor color(r, g, b);
    color.Darken(0.3f); // make sure it is not too light
    dlg.SetMarkerColor(color);
    dlg.SetMarkerRange(range_pos);
    if(dlg.ShowModal() == wxID_OK) {
        m_SeqGraphicPane->AddRangeMarker(dlg.GetMarkerName(),
            dlg.GetMarkerRange(), dlg.GetMarkerColor());
        // Remove selection range now that it's a marker:
        sel.Subtract(*iter);
        m_SeqGraphicPane->SetRangeSelection(sel);
        x_RedrawControls();
    }
}

void CSeqGraphicWidget::OnUpdateSetSeqMarkerForSelDlg(wxUpdateUIEvent& event)
{
    const CSeqGraphicPane::TRangeColl& sel =
        m_SeqGraphicPane->GetRangeSelection();

    TSeqPos pos = TSeqPos(m_PopupPoint.X());
    TSeqRange range_pos(pos, pos);
    if (sel.Contains(range_pos)) {
        event.Enable(true);
    }
    else {
        event.Enable(false);
    }
}

void CSeqGraphicWidget::OnSetMarker(wxCommandEvent& /*event*/)
{
    m_SeqGraphicPane->SetSeqMarker(m_PopupPoint);
    x_RedrawControls();
}


void CSeqGraphicWidget::OnRenameMarkerDlg(wxCommandEvent& /*event*/)
{
    string marker_id = m_SeqGraphicPane->GetHitMarker();
    CConstRef<CMarker> marker = m_SeqGraphicPane->GetMarker(marker_id);
    if (marker) {
        CwxMarkerNameDlg dlg(this);
        dlg.SetMarkerName(marker->GetLabel());
        if(dlg.ShowModal() == wxID_OK) {
            m_SeqGraphicPane->RenameMarker(marker_id, dlg.GetMarkerName());
            x_RedrawControls();
        }
    }
}


void CSeqGraphicWidget::OnModifyMarkerDlg(wxCommandEvent& /*event*/)
{
    string marker_id = m_SeqGraphicPane->GetHitMarker();
    CConstRef<CMarker> marker = m_SeqGraphicPane->GetMarker(marker_id);
    if (marker) {
        CwxSeqMarkerSetDlg dlg(this);
        dlg.SetDlgTitle("Modify Marker");
        dlg.SetMarkerName(marker->GetLabel());
        dlg.SetMarkerColor(marker->GetColor());

        if (marker->GetMarkerType() == CSeqMarkHandler::ePoint) {
            dlg.SetMarkerPos(marker->GetPos());
        }
        else { // eRange
            TSeqRange range(marker->GetPos(), marker->GetExtendedPos());
            dlg.SetMarkerRange(range);
        }

        if(dlg.ShowModal() == wxID_OK) {
            if (dlg.GetIsRange()) {
                m_SeqGraphicPane->ModifyMarker(marker_id, dlg.GetMarkerName(),
                    dlg.GetMarkerRange(), dlg.GetMarkerColor());
            }
            else {
                m_SeqGraphicPane->ModifyMarker(marker_id, dlg.GetMarkerName(),
                    dlg.GetMarkerPos(), dlg.GetMarkerColor());
            }
            x_RedrawControls();
        }
    }
}


void CSeqGraphicWidget::OnMarkerDetailsDlg(wxCommandEvent& /*event*/)
{
    if (m_MarkerInfoDlg  ||  m_SeqGraphicPane->GetMarkerNum() == 0) {
        return;
    }

    wxWindow* tmp = this;
    wxWindow* parent = NULL;
    while (tmp) {
        parent = tmp;
        tmp = tmp->GetParent();
    }
    m_MarkerInfoDlg = new CwxMarkerInfoDlg(parent, this);
    m_MarkerInfoDlg->Show(true);
    x_UpdateMarkerInfo();
}


void CSeqGraphicWidget::OnZoomAtMarker(wxCommandEvent& /*event*/)
{
    string marker_id = m_SeqGraphicPane->GetHitMarker();
    CConstRef<CMarker> marker = m_SeqGraphicPane->GetMarker(marker_id);
    if (marker) {
        TModelPoint center;
        if (marker->GetMarkerType() == CSeqMarkHandler::eRange) {
            center.Init((marker->GetPos() + marker->GetExtendedPos())/2.0, 1.0);
        }
        else {
            center.Init((TModelUnit)marker->GetPos(), 1.0);
        }
        m_Port.SetScale(IsHorizontal() ? m_Port.GetMinScaleX() : 1.0,
            IsHorizontal() ? 1.0 : m_Port.GetMinScaleY(), center);
        x_UpdateOnZoom();
        if (m_SeqGraphicPane.get()) {
            x_SaveStates();
        }

    }
}


void CSeqGraphicWidget::OnGoToMarker(wxCommandEvent& /*event*/)
{
    string marker_id = m_SeqGraphicPane->GetHitMarker();
    CConstRef<CMarker> marker = m_SeqGraphicPane->GetMarker(marker_id);
    if (marker) {
        TSeqPos mark_pos = marker->GetPos();
        TSeqPos half_r = (TSeqPos)(GetVisibleSeqRange().GetLength() * 0.5);
        if (half_r > mark_pos) {
            half_r = mark_pos;
        }
        TSeqPos max_r = GetDataSource()->GetSequenceLength() - 1;
        if (half_r > max_r - mark_pos) {
            half_r = max_r - mark_pos;
        }
        ZoomOnRange(TSeqRange(mark_pos - half_r, mark_pos + half_r), fSaveRange);
    }
}


void CSeqGraphicWidget::OnRemoveMarker(wxCommandEvent& /*event*/)
{
    string marker_id = m_SeqGraphicPane->GetHitMarker();
    if ( !marker_id.empty() ) {
        m_SeqGraphicPane->RemoveMarker(marker_id);
        x_RedrawControls();
    }
}


void CSeqGraphicWidget::OnSetSwitchPoint(wxCommandEvent& /*event*/)
{
    m_SeqGraphicPane->SetSwitchPoint();
    x_RedrawControls();
}


// set (reset) sequence start
void CSeqGraphicWidget::OnSetSeqStartMarker(wxCommandEvent& /*event*/)
{
    m_SeqGraphicPane->SetSeqStartMarker(m_SeqGraphicPane->GetHitMarker());
    x_RedrawControls();
}


void CSeqGraphicWidget::OnAdjustRuler(wxCommandEvent& /*event*/)
{
    CRef<CSeqGraphicConfig> config = GetConfig();
    bool flag = config->IsAjdustRulerToSelection();
    config->SetAjdustRulerToSelection( !flag );
    x_RedrawControls();
    config->SetDirty(true);
}


void CSeqGraphicWidget::OnUpdateAdjustRuler(wxUpdateUIEvent& event)
{
    CRef<CSeqGraphicConfig> config = GetConfig();
    bool flag = config->IsAjdustRulerToSelection();
    event.Check(flag);
}


void CSeqGraphicWidget::OnSetSeqStart(wxCommandEvent& /*event*/)
{
    m_SeqGraphicPane->SetSeqStartPoint(m_PopupPoint);
    x_RedrawControls();
}


void CSeqGraphicWidget::OnRemoveAllMarkersCmd(wxCommandEvent& /*event*/)
{
    x_RemoveAllMarkers();
}


void CSeqGraphicWidget::OnMarkerChanged()
{
    x_UpdateMarkerInfo();
    SetDirty(true);
}


void CSeqGraphicWidget::OnResetSeqStart(wxCommandEvent& /*event*/)
{
    m_SeqGraphicPane->ResetSeqStart();
    x_RedrawControls();
}


void CSeqGraphicWidget::OnZoomSequence(wxCommandEvent& /*event*/)
{
    m_Port.SetScale(IsHorizontal() ? m_Port.GetMinScaleX() : 1.0,
        IsHorizontal() ? 1.0 : m_Port.GetMinScaleY(), m_PopupPoint);

    x_UpdateOnZoom();
    if (m_SeqGraphicPane.get()) {
        x_SaveStates();
    }
}


void CSeqGraphicWidget::OnZoomSelection(wxCommandEvent& /*event*/)
{
    const CSeqGraphicPane::TRangeColl& sel =
        m_SeqGraphicPane->GetRangeSelection();
    TSeqRange range = TSeqRange(sel.GetFrom(), sel.GetTo());
    ZoomOnRange(range, fAddMargins | fSaveRange);
}

void CSeqGraphicWidget::OnSavePdf(wxCommandEvent& /* evt */)
{
    x_SaveVectorImage(CPrintOptions::ePdf);
}

void CSeqGraphicWidget::OnEnableSavePdfCmdUpdate(wxUpdateUIEvent& event)
{
    event.Enable(true);
}

void CSeqGraphicWidget::OnSaveSvg(wxCommandEvent& /* evt */)
{
    x_SaveVectorImage(CPrintOptions::eSvg);
}

void CSeqGraphicWidget::OnEnableSaveSvgCmdUpdate(wxUpdateUIEvent& event)
{
    event.Enable(true);
}

void CSeqGraphicWidget::OnZoomObject(wxCommandEvent& /*event*/)
{
    m_SeqGraphicPane->ZoomObject();
}


void CSeqGraphicWidget::OnPrevSplice(wxCommandEvent& /*event*/)
{
    m_SeqGraphicPane->NextPrevSplice(CSeqGraphicPane::eDir_Prev);
}


void CSeqGraphicWidget::OnNextSplice(wxCommandEvent& /*event*/)
{
    m_SeqGraphicPane->NextPrevSplice(CSeqGraphicPane::eDir_Next);
}


void CSeqGraphicWidget::OnLoadDefaultTracks(wxCommandEvent& /*event*/)
{
    m_SeqGraphicPane->LoadDefaultTracks();
}


void CSeqGraphicWidget::OnCollapseAllTracks(wxCommandEvent& /*event*/)
{
    m_SeqGraphicPane->ExpandAllTracks(false);
}


void CSeqGraphicWidget::OnExpandAllTracks(wxCommandEvent& /*event*/)
{
    m_SeqGraphicPane->ExpandAllTracks(true);
}


void CSeqGraphicWidget::OnShowAllTracks(wxCommandEvent& /*event*/)
{
    m_SeqGraphicPane->ShowAllTracks(true);
}

void CSeqGraphicWidget::OnHideAllTracks(wxCommandEvent& /*event*/)
{
    m_SeqGraphicPane->ShowAllTracks(false);
}


void CSeqGraphicWidget::OnCollapseAllSubtracks(wxCommandEvent& /*event*/)
{
    CTrackContainer* track =
        dynamic_cast<CTrackContainer*>(x_GetCurrentHitTrack());
    if (track) track->ExpandAll(false);
}


void CSeqGraphicWidget::OnExpandAllSubtracks(wxCommandEvent& /*event*/)
{
    CTrackContainer* track =
        dynamic_cast<CTrackContainer*>(x_GetCurrentHitTrack());
    if (track) track->ExpandAll(true);
}


void CSeqGraphicWidget::OnMoveTrackToTop(wxCommandEvent& /*event*/)
{
    CLayoutTrack* track = x_GetCurrentHitTrack();
    CTrackContainer* p_track =
        dynamic_cast<CTrackContainer*>(track->GetParentTrack());
    if (p_track) {
        p_track->MoveTrackToTop(track->GetOrder());
    }
}


void CSeqGraphicWidget::OnMoveTrackToBottom(wxCommandEvent& /*event*/)
{
    CLayoutTrack* track = x_GetCurrentHitTrack();
    CTrackContainer* p_track =
        dynamic_cast<CTrackContainer*>(track->GetParentTrack());
    if (p_track) {
        p_track->MoveTrackToBottom(track->GetOrder());
    }
}


void CSeqGraphicWidget::OnCloneTrack(wxCommandEvent& /*event*/)
{
    CLayoutTrack* track = x_GetCurrentHitTrack();
    _ASSERT(track);
    CTrackContainer* p_track =
        dynamic_cast<CTrackContainer*>(track->GetParentTrack());
    _ASSERT(p_track);

    CwxTrackNameDlg dlg(this);
    dlg.SetTitle(wxT("Duplicate the Track"));
    dlg.SetTrackName(ToWxString(track->GetFullTitle() + " clone"));
    while(dlg.ShowModal() == wxID_OK ) {
        string track_title = ToStdString(dlg.GetTrackName());
        NStr::TruncateSpacesInPlace(track_title);
        if (track_title.empty()) {
            dlg.SetMsg(wxT("The given track is title empty. Please input a valid track title!")); 
        } else {
            p_track->CloneTrack(track->GetOrder(), track_title);
            break;
        }
    }
}


void CSeqGraphicWidget::OnRemoveTrack(wxCommandEvent& /*event*/)
{
    CLayoutTrack* track = x_GetCurrentHitTrack();
    _ASSERT(track);
    string msg = "Do you really want to permanently remove this track: '";
    msg += track->GetFullTitle() + "'?";
    if (NcbiMessageBox(msg, eDialog_YesNo|eDialog_Modal, eIcon_Question,
        "Remove Track", eWrap)==eYes) {
        CTrackContainer* p_track =
            dynamic_cast<CTrackContainer*>(track->GetParentTrack());
        _ASSERT(p_track);
        p_track->RemoveTrack(track->GetOrder(), false);
    }
}


void CSeqGraphicWidget::OnAddTrack(wxCommandEvent& /*event*/)
{
    CTrackContainer* p_track =
        dynamic_cast<CTrackContainer*>(x_GetCurrentHitTrack());
    _ASSERT(p_track);

    CwxTrackSettingsDlg dlg(this);
    dlg.InitTrackTypes(p_track->GetConfigMgr()->GetTrackFactories());
    while(dlg.ShowModal() == wxID_OK ) {
        string track_title = ToStdString(dlg.GetTrackName());
        NStr::TruncateSpacesInPlace(track_title);
        if (track_title.empty()) {
            NcbiMessageBox("The given track title is empty. Please input a valid track title!");
        } else {
            string track_key = ToStdString(dlg.GetTrackType());
            vector<string> annots;
            string annot_str = ToStdString(dlg.GetAnnots());
            NStr::TruncateSpacesInPlace(annot_str);
            NStr::Split(annot_str, ",;|", annots);
            string sub_key = kEmptyStr;
            if (track_key == CFeatureTrackFactory::GetTypeInfo().GetId()) {
                sub_key = ToStdString(dlg.GetSubtype());
            }
            string filter = ToStdString(dlg.GetTrackFilter());
            NStr::TruncateSpacesInPlace(filter);
            p_track->AddNewTrack(track_key, annots, track_title, "", filter, sub_key);
            break;
        }
    }
}


void CSeqGraphicWidget::OnRenameTrack(wxCommandEvent& /*event*/)
{
    CLayoutTrack* track = x_GetCurrentHitTrack();
    _ASSERT(track);
    CTrackContainer* p_track =
        dynamic_cast<CTrackContainer*>(track->GetParentTrack());
    _ASSERT(p_track);

    CwxTrackNameDlg dlg(this);
    while(dlg.ShowModal() == wxID_OK ) {
        string track_name = ToStdString(dlg.GetTrackName());
        NStr::TruncateSpacesInPlace(track_name);
        if ( !track_name.empty()  ||
            NcbiMessageBox("The given track title is empty. A default track "
            "title will be used. Do you want to proceed with the default track title?",
            eDialog_OkCancel, eIcon_Exclamation, "Attention!" ) == eOK ) {
            p_track->RenameChildTrack(track->GetOrder(), track_name);
            break;
        }
    }
}

/// editing interfaces
void CSeqGraphicWidget::OnEditCut(wxCommandEvent& /*event*/)
{
    //m_SeqGraphicPane->OnEditCut();
}


void CSeqGraphicWidget::OnEditCopy(wxCommandEvent& /*event*/)
{
    //m_SeqGraphicPane->OnEditCopy();
}


void CSeqGraphicWidget::OnEditPaste(wxCommandEvent& /*event*/)
{
    //m_SeqGraphicPane->OnEditPaste();
}


void CSeqGraphicWidget::OnObjToRangeSelection()
{
}


void CSeqGraphicWidget::OnRangeToObjSelection()
{
}


void CSeqGraphicWidget::OnSize(wxSizeEvent& event)
{
    CGlWidgetBase::OnSize(event);
    //TVPRect vp = m_Port.GetViewport();
    //cout << "CSeqGraphicWidget::OnSize: " << vp.Width() << endl;
    if (m_SeqGraphicPane.get()) {
        m_SeqGraphicPane->SetUpdateContent();
        m_SeqGraphicPane->AdjustViewPort();
    }
}

void CSeqGraphicWidget::OnScrollEnd(wxScrollEvent& event)
{
    if (m_SeqGraphicPane.get()) {
        x_SaveStates();
    }
    event.Skip();
}

void CSeqGraphicWidget::UpdateHeight(TModelUnit height)
{
    TModelRect rcM = m_Port.GetModelLimitsRect();
    TModelRect rcV = m_Port.GetVisibleRect();

    //rcM.SetVert(0, height);
    rcM.SetBottom(height);
    rcV.SetBottom(m_Port.GetViewport().Height() + rcV.Top());

    //cout << "Layout Height: " << height << endl;
    //cout << "Port Height: " << m_Port.GetViewport().Height() << endl;
    //cout << "Visible TOP: " << rcV.Top() << endl;
    //cout << "Visible Bottom: " << rcV.Bottom() << endl;

    m_Port.SetModelLimitsRect(rcM);
    m_Port.SetVisibleRect(rcV);

    //x_UpdateOnZoom();
    x_UpdateScrollbars();
    //x_RedrawControls();
}


void CSeqGraphicWidget::SetScaleX(TModelUnit scale, const TModelPoint& point)
{
    m_Port.SetScaleRefPoint(scale, m_Port.GetScaleY(), point);
    x_UpdateOnZoom();
}


void CSeqGraphicWidget::OnContextMenu(wxContextMenuEvent& event)
{
    wxPoint pos = ScreenToClient(event.GetPosition());
    // If mouse is hovering over a selectable object that is not curently
    // being selected, we will select this object, and deselect any
    // other objects that are being selected.
    int x  = pos.x;
    int y = m_SeqGraphicPane->MZHH_GetVPPosByY(pos.y);
    m_SeqGraphicPane->SelectLastHitObject(x, y);
    m_context_x = x;
    m_context_y = y;

    m_PopupPoint.m_X = m_SeqGraphicPane->SHH_GetModelByWindow(pos.x, eHorz);
    m_PopupPoint.m_Y = m_SeqGraphicPane->SHH_GetModelByWindow(pos.y, eVert);
    int area = m_SeqGraphicPane->GetPopupArea(pos);

    Update();

    wxMenu* menu = x_CreatePopupMenu(area);
    x_ShowDecoratedPopupMenu(menu);
}


CSeqGraphicWidget::CSeqGraphicWidget(wxWindow* parent, wxWindowID id,
                                     const wxPoint& pos, const wxSize& size,
                                     long style)
    : CGlWidgetBase(parent, id, pos, size, style)
    , m_SequenceGotoDlg(NULL)
    , m_MarkerInfoDlg(NULL)
    , m_Dirty(false)
    , m_context_x(0)
    , m_context_y(0)
{
    // setup Port
    m_Port.SetAdjustmentPolicy(CGlPane::fAdjustAll, CGlPane::fAdjustAll);
    m_Port.SetMinScaleX(1.0 / kMaxPixelsBase);
    m_Port.SetOriginType(CGlPane::eOriginLeft, CGlPane::eOriginTop);
    m_Port.EnableZoom(true, true);
}


CSeqGraphicWidget::~CSeqGraphicWidget()
{ 
    if (m_BioseqEditor)
        RemoveEventHandler(m_BioseqEditor);

    if (m_SequenceGotoDlg) {
        m_SequenceGotoDlg->Destroy();
        m_SequenceGotoDlg = NULL;
    }
    if (m_MarkerInfoDlg) {
        m_MarkerInfoDlg->Destroy();
        m_MarkerInfoDlg = NULL;
    }

    //m_ScoreToParams.clear();
    /*if (m_SeqGraphicPane) {
        m_SeqGraphicPane->RemoveListener(this);
        RemoveListener(m_SeqGraphicPane);
    }*/
}


void CSeqGraphicWidget::SetExternalGlyphs(const CSeqGlyph::TObjects& objs)
{
    m_SeqGraphicPane->SetExternalGlyphs(objs);
}


//void CSeqGraphicWidget::OnScroll(CScrollbar* /*scrollbar*/)
//{
//    m_SeqGraphicPane->Scroll(TSeqPos(m_ScrollX->GetValue()),
//                             TSeqPos(m_ScrollY->GetValue()) );
//}


/*void CSeqGraphicWidget::OnEndScroll(CScrollbar*)
{
    m_SeqGraphicPane->OnEndScroll();
}*/



void CSeqGraphicWidget::ZoomOnRange(TSeqRange range, TZoomFlag flag)
{
    if (flag & fAddMargins) {
        // add a total of 6% margin around two ends
        TSeqPos margin = TSeqPos(range.GetLength() * 0.15f);
        CSGSequenceDS* pDS = GetDataSource();
        if (pDS) {
            TSeqPos len = pDS->GetSequenceLength();
            range.SetFrom(range.GetFrom() > margin ? range.GetFrom() - margin : 0);
            range.SetTo(range.GetTo() < len - margin ? range.GetTo() + margin : len - 1);
        }
    }

    TModelRect rcV = m_Port.GetModelLimitsRect();
    rcV.SetHorz(range.GetFrom(), range.GetToOpen());
    m_Port.ZoomRect(rcV);
    x_UpdateOnZoom();
    if (flag & fSaveRange) {
        x_SaveStates();
    }
}


bool CSeqGraphicWidget::VisibleRangeChanged() const
{
    return false; //return m_SeqGraphicPane->VisibleRangeChanged();
}


void CSeqGraphicWidget::ResetVisibleRangeChanged()
{
    //m_SeqGraphicPane->ResetVisibleRangeChanged();
}


void CSeqGraphicWidget::SetInputObject(SConstScopedObject& obj)
{
    m_SeqGraphicPane->SetInputObject(obj);
    x_Update();
}

CFeaturePanel* CSeqGraphicWidget::GetFeaturePanel()
{
	if (!m_SeqGraphicPane.get())
		return nullptr;

	return m_SeqGraphicPane->GetFeaturePanel();
}

void CSeqGraphicWidget::ConfigureTracksDlg()
{
    if (m_SeqGraphicPane.get()) {
        string category;
        m_SeqGraphicPane->LTH_ConfigureTracksDlg(category);
    }
}

void CSeqGraphicWidget::ResetSearch()
{
    if (m_SeqGraphicPane.get()) 
        m_SeqGraphicPane->ResetSearch();
}

bool CSeqGraphicWidget::FindText(const string &text, bool match_case)
{
    if (m_SeqGraphicPane.get()) 
    {
        TSeqRange range = m_SeqGraphicPane->FindText(text, match_case);
        if (range.NotEmpty())
        {
            if (range.GetTo() == range.GetFrom())
            {
                TSeqPos mark_pos = range.GetTo();
                TSeqPos half_r = (TSeqPos)(GetVisibleSeqRange().GetLength() * 0.5);
                if (half_r > mark_pos) 
                    half_r = mark_pos;
                TSeqPos max_r = GetDataSource()->GetSequenceLength() - 1;
                if (half_r > max_r - mark_pos) 
                    half_r = max_r - mark_pos;
                ZoomOnRange(TSeqRange(mark_pos - half_r, mark_pos + half_r), fSaveRange);
            }
            else
                ZoomOnRange(range, fSaveRange);
            return true;
        }
    }
    return false;
}

void CSeqGraphicWidget::InitDataSource(SConstScopedObject& obj)
{
    m_SeqGraphicPane->InitDataSource(obj);
}


CSGSequenceDS* CSeqGraphicWidget::GetDataSource(void) const
{
    return m_SeqGraphicPane->GetDataSource();
}

void CSeqGraphicWidget::SetHorizontal(bool b_horz, bool b_flip)
{
    m_SeqGraphicPane->SetHorizontal(b_horz, b_flip);
    m_SeqGraphicPane->Refresh();
}


bool CSeqGraphicWidget::IsHorizontal() const
{
    return m_SeqGraphicPane->IsHorizontal();
}


CRef<CSeqGraphicConfig> CSeqGraphicWidget::GetConfig()
{
    return m_SeqGraphicPane->GetConfig();
}


bool CSeqGraphicWidget::IsFlipped() const
{
    return m_SeqGraphicPane->IsFlipped();
}


// Set/Clear selection
void CSeqGraphicWidget::SelectObject(const CObject* obj, bool verified)
{
    m_SeqGraphicPane->SelectObject(obj, verified);
}


void CSeqGraphicWidget::SelectSeqLoc(const CSeq_loc* loc)
{
    m_SeqGraphicPane->SelectSeqLoc(loc);
}


void CSeqGraphicWidget::ResetSelection()
{
    m_SeqGraphicPane->ResetSelection();
}


void CSeqGraphicWidget::ResetRangeSelection()
{
    m_SeqGraphicPane->ResetRangeSelection();
}


void CSeqGraphicWidget::ResetObjectSelection()
{
    m_SeqGraphicPane->ResetObjectSelection();
}


// retrieve selected objects from this widget
void CSeqGraphicWidget::GetObjectSelection(TConstObjects& objs) const
{
    m_SeqGraphicPane->GetObjectSelection(objs);
}


const CSeqGraphicWidget::TRangeColl&
     CSeqGraphicWidget::GetRangeSelection(void) const
{
    return m_SeqGraphicPane->GetRangeSelection();
}


void CSeqGraphicWidget::SetRangeSelection(const TRangeColl& ranges)
{
    m_SeqGraphicPane->SetRangeSelection(ranges);
}

void CSeqGraphicWidget::GetRangeOrObjectSelection(TRangeColl& ranges) const
{
    // Try to get range
    const CSeqGraphicWidget::TRangeColl& coll = m_SeqGraphicPane->GetRangeSelection();

    if (coll.size()) {
        ranges = coll;
    } else {
        // if there is no range selection, try selected layout object
        CConstRef<CSeqGlyph> obj = m_SeqGraphicPane->GetSelectedLayoutObj();
        if (obj) {
            TSeqRange range = obj->GetRange();
            ranges.CombineWith(range);
        }
    }

}

void CSeqGraphicWidget::UpdateConfig()
{
    m_SeqGraphicPane->UpdateConfig();
}

//void CSeqGraphicWidget::OnEditFeature(ncbi::CEvent* /*evt*/)
//{
//    UpdateConfig();
//}


string CSeqGraphicWidget::GetMarkers() const
{
    return m_SeqGraphicPane->GetMarkers();
}


void CSeqGraphicWidget::SetMarkers(const string& markers)
{
    m_SeqGraphicPane->SetMarkers(markers);
}


void CSeqGraphicWidget::SaveViewSettings()
{
    GetConfig()->SaveSettings();
    m_SeqGraphicPane->SaveTrackConfig();
}


bool CSeqGraphicWidget::IsDirty() const
{
    return m_Dirty;
}


void CSeqGraphicWidget::SetDirty(bool flag)
{
    m_Dirty = flag;
}


void CSeqGraphicWidget::x_CreatePane()
{
    m_SeqGraphicPane.reset(new CSeqGraphicPane(this));    
}


CGlWidgetPane* CSeqGraphicWidget::x_GetPane()
{
    return static_cast<CGlWidgetPane*>(m_SeqGraphicPane.get());
}


void CSeqGraphicWidget::x_Update()
{
    //cout << "CSeqGraphicWidget::x_Update" << endl;
    x_SetPortLimits();

    /*if(m_DataSource)   {
        x_ZoomToHits();
    } else {
        m_Port.ZoomAll();
    }*/

    //x_UpdatePane();
    x_UpdateScrollbars();
    x_RedrawControls();
}


void CSeqGraphicWidget::x_UpdatePane()
{
    m_SeqGraphicPane->Update();
}


void CSeqGraphicWidget::x_SaveStates()
{
    m_SeqGraphicPane->SaveCurrentRange();
}


void CSeqGraphicWidget::x_SetPortLimits()
{
    TModelRect rc_m(0, 0, 1000, 1000);

    CSGSequenceDS* pDS = GetDataSource();
    if (pDS) {
        rc_m.SetHorz(0, pDS->GetSequenceLength());
        rc_m.SetVert(m_Port.GetViewport().Height(), 0);
        //rc_m = TModelRect(0, m_Port.GetViewport().Height(), pDS->GetSequenceLength(), 0);
    }

    m_Port.SetModelLimitsRect(rc_m);
    m_Port.SetVisibleRect(rc_m);
}

void CSeqGraphicWidget::x_UpdateOnVScroll()
{
    x_UpdateScrollbars();
    x_RedrawControls();
}

wxMenu* CSeqGraphicWidget::x_CreatePopupMenu(int area)
{
    CUICommandRegistry& cmd_reg = CUICommandRegistry::GetInstance();

    if (area == CSeqGraphicPane::fArea_Ruler) {
        const string& hit_marker = m_SeqGraphicPane->GetHitMarker();
        if (hit_marker.empty()) {
            return cmd_reg.CreateMenu(sRulerPopupMenuNotOnMarker);
        }

        return cmd_reg.CreateMenu(sRulerPopupMenuOnMarker);
    }

    wxMenu* root = cmd_reg.CreateMenu(sFeatPanePopupMenu);
    if (m_BioseqEditor) {              
        unique_ptr<wxMenu> editorMenu(m_BioseqEditor->CreateContextMenu(true));
        if (editorMenu.get()) {
            Merge(*root, *editorMenu);
        }
    }

    // track-specific context menu
    wxMenu* track_menu = new wxMenu;
    CRef<CLayoutTrack> track = m_SeqGraphicPane->THH_GetLayoutTrack();
    if (track) {
        cmd_reg.AppendMenuItem(*track_menu, eCmdMoveTrackToTop);
        cmd_reg.AppendMenuItem(*track_menu, eCmdMoveTrackToBottom);
        CTrackContainer* cont = dynamic_cast<CTrackContainer*>(track.GetPointer());
        if (cont) {
            cmd_reg.AppendMenuItem(*track_menu, eCmdCollapseAllSubtracks);
            cmd_reg.AppendMenuItem(*track_menu, eCmdExpandAllSubtracks);
        }
        cmd_reg.AppendMenuItem(*track_menu, eCmdCloneTrack);

        CTrackContainer* p_track =
            dynamic_cast<CTrackContainer*>(track->GetParentTrack());
        if (p_track) {
            const CTrackProxy::TTrackProxies& proxies =
                p_track->GetSubtrackProxies();
            ITERATE (CTrackProxy::TTrackProxies, iter, proxies) {
                const CTempTrackProxy* temp_proxy = *iter;
                if (temp_proxy->GetOrder() == track->GetOrder()) {
                    const CTrackProxy* proxy =
                        dynamic_cast<const CTrackProxy*>(temp_proxy);
                    if (proxy) {
                        cmd_reg.AppendMenuItem(*track_menu, eCmdRemoveTrack);
                        if (proxy->GetKey() ==
                            CTrackContainerFactory::GetTypeInfo().GetId()) {
                            cmd_reg.AppendMenuItem(*track_menu, eCmdAddTrack);
                        }
                    }
                    break;
                }
            }
        }
        cmd_reg.AppendMenuItem(*track_menu, eCmdRenameTrack);
    } else {
        track = m_SeqGraphicPane->GetFeaturePanel();
        cmd_reg.AppendMenuItem(*track_menu, eCmdAddTrack);
    }

    string label = track->GetFullTitle();
    if (label.length() > 50) {
        label = label.substr(0, 50);
        label += "...";
    }
    root->AppendSubMenu(track_menu, ToWxString(label));

    root->Append( wxID_SEPARATOR, wxT("VRBroadcast") );

    //root->AppendSeparator();
    //root->Append(eCmdSettings, wxT("Settings..."));

    return root;
}


CLayoutTrack* CSeqGraphicWidget::x_GetCurrentHitTrack()
{
    CRef<CLayoutTrack> track = m_SeqGraphicPane->THH_GetLayoutTrack();
    if ( !track ) track = m_SeqGraphicPane->GetFeaturePanel();
    return track;
}


void CSeqGraphicWidget::x_RemoveAllMarkers()
{
    if (m_SeqGraphicPane->GetMarkerNum() > 0) {
        if (NcbiMessageBox("Do you really want to remove all markers?",
            eDialog_YesNo|eDialog_Modal, eIcon_Question,
            "Remove All Markers", eWrap)==eYes) {
            m_SeqGraphicPane->RemoveAllMarkers();
            x_RedrawControls();
        }
    }
}


void CSeqGraphicWidget::x_UpdateMarkerInfo()
{
    if ( !m_MarkerInfoDlg ) return;

    CObjectCoords oc(GetDataSource()->GetScope());
    CwxMarkerInfoDlg::SMarkerInfoVec info_vec;
    ITERATE (CSeqGraphicPane::TSeqMarkers, m_iter, m_SeqGraphicPane->GetAllMarkers()) {
        SMarkerInfo marker_info;
        marker_info.m_Id = m_iter->first;
        marker_info.m_Label = m_iter->second->GetLabel();
        marker_info.m_Pos = m_iter->second->GetPos() + 1;
        CHGVS_Coordinate_Set coords;
        coords.Set();
        oc.GetCoordinates(coords, *GetDataSource()->GetBioseqHandle().GetSeqId(),
            m_iter->second->GetPos());

        if ( !coords.Get().empty() ) {
            SMarkerCoordVec& vec = marker_info.m_CoordVec;
            ITERATE(CHGVS_Coordinate_Set::Tdata, iter, coords.Get()) {
                SMarkerCoord info;
                info.m_Acc = (*iter)->GetTitle();
                if ((*iter)->CanGetPos_mapped()) {
                    info.m_Loc = NStr::NumericToString((*iter)->GetPos_mapped() + 1);
                }
                info.m_Hgvs = (*iter)->GetHgvs_position();
                if (info.m_Hgvs.find("c") == 0) {
                    info.m_LocRef = "CDS start";
                } else {
                    info.m_LocRef = "Seq start";
                }
                info.m_Seq = (*iter)->GetSequence();
                vec.push_back(info);
            }
            info_vec.push_back(marker_info);                
        }
    }

    m_MarkerInfoDlg->UpdateMarker(info_vec);
}

void CSeqGraphicWidget::x_SaveVectorImage(CPrintOptions::EOutputFormat format)
{
    CSGSequenceDS* pDS = GetDataSource();
    if (!pDS) return;

    SaveViewSettings();

    CSeqGraphicWidget widget(this->GetParent());
    widget.Create();
    widget.UpdateConfig();
    CConstRef<CSeq_id> id = GetDataSource()->GetBioseqHandle().GetSeqId();
    SConstScopedObject obj(id, &GetDataSource()->GetScope());
    widget.SetInputObject(obj);
    // set markers
    widget.SetMarkers(GetMarkers());
    // set selections
    TConstObjects sel_objs;
    GetObjectSelection(sel_objs);
    ITERATE(TConstObjects, it, sel_objs) {
        widget.SelectObject(*it, true);
    }
    // set new origin
    widget.m_SeqGraphicPane->SetSeqStart(m_SeqGraphicPane->GetSeqStart());

    // Disable normal (non-vector) rendering in widget being used for vector graphics
    widget.m_SeqGraphicPane->SetVectorMode(true);

    TVPRect vp = GetPort().GetViewport();
    widget.GetPort().SetViewport(vp);
    widget.SetHorizontal(true, IsFlipped());
    // Use the whole sequence range as the initial range
    // to guarantee all valid tracks will be created. If use
    // the current range, some of the tracks may not be displayed
    // in the final PDF if there is no data in current range.
    TSeqRange range(0, pDS->GetSequenceLength() - 1);
    widget.ZoomOnRange(range, 0);

    range = GetVisibleSeqRange();
    CwxExportVectorImageDlg dlg(format, widget.m_SeqGraphicPane.get(), this);
    // store the original seq range, and viewport size
    dlg.SetSeqRange(range.GetFrom(), range.GetTo());
    dlg.SetViewport(vp);

    string assembly = m_SeqGraphicPane->GetFeaturePanel()->GetDS()->GetAssembly();
    if (!assembly.empty())
        widget.GetFeaturePanel()->SetAssembly(assembly);

    dlg.ShowModal();
}

void  CSeqGraphicWidget::InitBioseqEditor(ICommandProccessor& cmdProcessor)
{   
    if (m_BioseqEditor)
        RemoveEventHandler(m_BioseqEditor);

    vector<IEditorFactory*> factories;
    GetExtensionAsInterface(EXT_POINT__EDITOR_FACTORY, factories);
    
    ITERATE(vector<IEditorFactory*>, it, factories) {
        m_BioseqEditor.Reset(dynamic_cast<IBioseqEditor*>((*it)->CreateEditor(typeid(IBioseqEditor).name(), cmdProcessor, nullptr)));        
        if (m_BioseqEditor)
            break;
    }
    if (m_BioseqEditor) {
        m_BioseqEditor->SetCallBack(this);
        PushEventHandler(m_BioseqEditor);
    }
}
    
objects::CBioseq_Handle CSeqGraphicWidget::GetCurrentBioseq()
{
     return GetDataSource()->GetBioseqHandle();
}

void CSeqGraphicWidget::GetCurrentSelection(TEditObjectList& objs)
{
    objects::CBioseq_Handle bsh = GetDataSource()->GetBioseqHandle();
    TConstObjects sel_objs;
    m_SeqGraphicPane->GetCurrentObject(m_context_x, m_context_y, sel_objs);
    if (sel_objs.empty())
        GetObjectSelection(sel_objs);
    for (auto &item : sel_objs)
    {
        const CSeq_feat* ofeat = dynamic_cast<const CSeq_feat*>(item.GetPointer());
        if (ofeat)
            objs.emplace_back(*ofeat, bsh.GetSeq_entry_Handle());
    }
}

END_NCBI_SCOPE
