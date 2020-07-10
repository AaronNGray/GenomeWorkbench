/*  $Id: phylo_tree_widget.cpp 43126 2019-05-17 18:16:36Z katargir $
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
 * Authors:  Vladimir Tereshkov
 *
 * File Description:
 *
 */

#include <ncbi_pch.hpp>
#include <gui/widgets/phylo_tree/phylo_tree_widget.hpp>

#include <corelib/ncbitime.hpp>
#include <gui/widgets/wx/sys_path.hpp>

#include <gui/opengl/glfont.hpp>
#include <gui/types.hpp>
#include <gui/utils/view_event.hpp>
#include <gui/widgets/wx/print_handler_wx.hpp>
#include <gui/widgets/phylo_tree/phylo_image_grabber.hpp>
#include <gui/widgets/phylo_tree/phylo_pdf_grabber.hpp>
#include <gui/widgets/wx/save_images_setup_dlg.hpp>
#include "save_tree_pdf_dlg.hpp"
#include "phylo_save_selection_dlg.hpp"

#include <gui/widgets/phylo_tree/phylo_tree_rect_cladogram.hpp>
#include <gui/widgets/phylo_tree/phylo_tree_slanted_cladogram.hpp>
#include <gui/widgets/phylo_tree/phylo_tree_force.hpp>
#include <gui/widgets/phylo_tree/phylo_tree_radial.hpp>
#include <gui/widgets/phylo_tree/phylo_circular_cladogram.hpp>

#include <gui/widgets/phylo_tree/wx_phylo_settings_dlg.hpp>
#include <gui/widgets/phylo_tree/wx_phylo_edit_dlg.hpp>
#include <gui/widgets/phylo_tree/wx_label_edit_dlg.hpp>
#include <gui/widgets/phylo_tree/wx_tree_label_edit.hpp>
#include <gui/widgets/phylo_tree/csv_selection_export_dlg.hpp>
#include <gui/widgets/phylo_tree/tree_model.hpp>

#include <gui/opengl/glresmgr.hpp>
#include <gui/opengl/irender.hpp>
#include <gui/widgets/wx/message_box.hpp>
#include <gui/widgets/wx/wx_utils.hpp>
#include <gui/widgets/wx/ui_command.hpp>
#include <gui/widgets/wx/sticky_tooltip_wnd.hpp>
#include <gui/widgets/wx/fileartprov.hpp>
#include <gui/objutils/biotree_attr_reader.hpp>
#include <gui/objutils/macro_query_exec.hpp>

#include <gui/widgets/wx/message_box.hpp>
#include <gui/widgets/wx/unfocused_controls.hpp>
#include <gui/widgets/wx/fileartprov.hpp>
#include <gui/widgets/wx/csv_exporter.hpp>
#include <gui/utils/command_processor.hpp>

#include <objects/biotree/BioTreeContainer.hpp>
#include <objects/biotree/FeatureDescr.hpp>
#include <objects/biotree/FeatureDictSet.hpp>
#include <objects/biotree/NodeFeatureSet.hpp>
#include <objects/biotree/NodeFeature.hpp>
#include <objects/biotree/NodeSet.hpp>
#include <objects/biotree/Node.hpp>

#include <util/image/image.hpp>
#include <util/image/image_io.hpp>

#include <wx/settings.h>
#include <wx/msgdlg.h>
#include <wx/filedlg.h>
#include <wx/textctrl.h>
#include <wx/tglbtn.h>
#include <wx/printdlg.h>
#include <wx/cmndata.h>

#include <list>

#include <wx/menu.h>

BEGIN_NCBI_SCOPE


//static const char* kLayout = "Layout";
//static const char* kScrollMode = "Scroll Mode";
//static const char* kPhyloTree = ".PhyloTree";


////////////////////////////////////////////////////////////////////////////////
/// class CPhyloTreeWidget
void CPhyloTreeWidget::RegisterCommands(CUICommandRegistry& cmd_reg,
                                         wxFileArtProvider& /* provider */)
{
    static bool initialized = false;
    if (initialized)
        return;
    initialized = true;

    cmd_reg.RegisterCommand(eCmdEditLabel, "Edit Label", "Edit Label", "", "");
    cmd_reg.RegisterCommand(eCmdEditNode, "Properties", "Properties", "", "");

    cmd_reg.RegisterCommand(eCmdNodeCut, "Cut", "Cut", "", "");
    cmd_reg.RegisterCommand(eCmdNodePaste, "Paste", "Paste", "", "");
    cmd_reg.RegisterCommand(eCmdMoveUp, "Move Up", "Move Up", "", "");
    cmd_reg.RegisterCommand(eCmdMoveDown, "Move Down", "Move Down", "", "");

    cmd_reg.RegisterCommand(eCmdNodeNewChild, "New Child", "New Child", "", "");
    cmd_reg.RegisterCommand(eCmdNodeNewParent, "New Parent", "New Parent", "", "");

    cmd_reg.RegisterCommand(eCmdRemoveSubtree, "Subtree", "Subtree", "", "");
    cmd_reg.RegisterCommand(eCmdRemoveSelected, "Selected", "Selected", "", "");
    cmd_reg.RegisterCommand(eCmdRemoveNode, "Node", "Node", "", "");

    cmd_reg.RegisterCommand(eCmdExportSelected, "Save Selected to Newick file...", "Save Selected to Newick file...", "", "");
    cmd_reg.RegisterCommand(eCmdFilter, "Display the subtree", "Display the subtree", "", "");
    cmd_reg.RegisterCommand(eCmdSubtreeFromSelected, "Create Subtree from Selected Nodes", "Create Subtree from Selected Nodes", "", "");
    cmd_reg.RegisterCommand(eCmdSort, "Sort by Number of Children", "Sort by Number of Children", "", "");
    cmd_reg.RegisterCommand(eCmdSortDist, "Sort by Distance", "Sort by Distance", "", "");
    cmd_reg.RegisterCommand(eCmdSortLabel, "Sort by Label", "Sort by Label", "", "");
    cmd_reg.RegisterCommand(eCmdSortLabelRange, "Sort by Subtree Labels", "Sort by Subtree Labels", "", "");
    cmd_reg.RegisterCommand(eCmdSortAscending, "Sort in Ascending Order", "Sort in Ascending Order", "", "", "", "", wxITEM_CHECK);

    cmd_reg.RegisterCommand(eCmdSetGraphType1,
        "Rectangle Cladogram", "Rectangle Cladogram", "", "", "", "", wxITEM_RADIO);
    cmd_reg.RegisterCommand(eCmdSetGraphType2,
        "Slanted Cladogram", "Slanted Cladogram", "", "", "", "", wxITEM_RADIO);
    cmd_reg.RegisterCommand(eCmdSetGraphType3,
        "Radial Tree", "Radial Tree", "", "", "", "", wxITEM_RADIO);
    cmd_reg.RegisterCommand(eCmdSetGraphType4,
        "Force Layout", "Force Layout", "", "", "", "", wxITEM_RADIO);
    cmd_reg.RegisterCommand(eCmdSetGraphType5,
        "Circular Layout", "Circular Layout", "", "", "", "", wxITEM_RADIO);
    cmd_reg.RegisterCommand(eCmdRotateLabels,
        "Rotate Labels", "Rotate Labels", "", "", "", "", wxITEM_CHECK);
    cmd_reg.RegisterCommand(eCmdStopLayout,
        "Stop Layout", "Stop Layout", "", "", "", "", wxITEM_NORMAL);
    cmd_reg.RegisterCommand(eCmdUseDistances,
        "Use Distances", "Use Distances", "", "", "", "", wxITEM_CHECK);

    cmd_reg.RegisterCommand(eCmdZoomXY,
        "Proportional", "Proportional", "", "", "", "", wxITEM_RADIO);
    cmd_reg.RegisterCommand(eCmdZoomY,
        "Vertical", "Vertical", "", "", "", "", wxITEM_RADIO);
    cmd_reg.RegisterCommand(eCmdZoomX,
        "Horizontal", "Horizontal", "", "", "", "", wxITEM_RADIO);

    cmd_reg.RegisterCommand(eCmdRerootTree, "Make this Root", "Make this Root", "", "");
    cmd_reg.RegisterCommand(eCmdRerootTreeAtEdge, "Place Root at Middle of Branch", "Place Root at Middle of Branch", "", "");
    cmd_reg.RegisterCommand(eCmdMidpointRoot, "Set Midpoint Root", "Set Midpoint Root", "", "");
    cmd_reg.RegisterCommand(eCmdZoomToSubtree,
        "Re-root and show only child nodes", "Re-root and show only child nodes", "menu::zoom_all", "");


    cmd_reg.RegisterCommand(eCmdLabelsForLeavesOnly,
        "For Leaves Only", "For Leaves Only", "", "", "", "", wxITEM_RADIO);
    cmd_reg.RegisterCommand(eCmdLabelsVisible,
        "Show All", "Show All", "", "", "", "", wxITEM_RADIO);
    cmd_reg.RegisterCommand(eCmdLabelsHidden,
        "Hide All", "Hide All", "", "", "", "", wxITEM_RADIO);

    cmd_reg.RegisterCommand(eCmdSetTreeLabel, "Set Tree Label", "Set Tree Label", "", "");

    cmd_reg.RegisterCommand(eCmdAlNone, "None", "None", "", "", "", "", wxITEM_RADIO);
    cmd_reg.RegisterCommand(eCmdAlPhylip, "Phylip-Style", "Phylip-Style", "", "", "", "", wxITEM_RADIO);
    cmd_reg.RegisterCommand(eCmdAlPaml, "PAML-Style", "PAML-Style", "", "", "", "", wxITEM_RADIO);

    cmd_reg.RegisterCommand(eCmdShowAll, "Expand All", "Expand All", "", "");
    cmd_reg.RegisterCommand(eCmdCollapseChildren, "Collapse", "Collapse", "", "");
    cmd_reg.RegisterCommand(eCmdCollapseSelected, "Collapse Selected", "Collapse Selected", "", "");
    cmd_reg.RegisterCommand(eCmdExpandChildren, "Expand", "Expand", "", "");
    cmd_reg.RegisterCommand(eCmdHighlightEdges, "Highlight Longer Edges", "Highlight Longer Edges", "", "", "", "", wxITEM_CHECK);
    cmd_reg.RegisterCommand(eCmdCollapseToViewport, "Collapse Tree to fit Viewport", "Collapse Tree to fit Viewport", "", "", "", "", wxITEM_CHECK);

    cmd_reg.RegisterCommand(eSelectAll, "Select All", "Select All", "", "");
    cmd_reg.RegisterCommand(eClearSelection, "Clear Selection", "Clear Selection", "", "");
    cmd_reg.RegisterCommand(eCmdAddSelectionSet, "Update Selection Sets", "Update Selection Sets", "", "");
    cmd_reg.RegisterCommand(eCmdExportSelection, "Export Selection to CSV...", "Export Selection to CSV...", "", "");
    cmd_reg.RegisterCommand(eCmdExportTree, "Export Tree...", "Export Tree to Newick/Nexus...", "", "");

    cmd_reg.RegisterCommand(eCmdClean, "Delete Single-Child Nodes", "Delete Single-Child Nodes", "", "");
    cmd_reg.RegisterCommand(eCmdFilterDistances, "Distance Filtering...", "Distance Filtering...", "", "");

    cmd_reg.RegisterCommand(eCmdUseSplines, "Use Splines", "Use Splines", "", "", "", "", wxITEM_CHECK);

    cmd_reg.RegisterCommand(eCmdMySettings, "Settings...", "Settings...", "", "");

    cmd_reg.RegisterCommand(eCmdLoadAttributes, "Load Attributes...", "Attributes...", "", "");
}

CPhyloTreeWidget::CPhyloTreeWidget(wxWindow* parent, wxWindowID id,
                                   const wxPoint& pos, const wxSize& size, long style,
                                   const wxString& name)
: CGlWidgetBase(parent, id, pos, size, style, name)
, m_pPhyloTreePane(NULL)
, m_QueryPanel(NULL)
, m_SortAscending(false)
, m_LastCommand(EPhyloTreeEditCommand::eEditCmdNone)
, m_PrevPos(0.0f, 0.0f)
{
    // setup Port
    m_Port.SetAdjustmentPolicy(CGlPane::fAdjustAll, CGlPane::fAdjustAll);
    m_Port.SetMinScaleX(1 / 1000.0);
    m_Port.SetMinScaleY(1 / 1000.0);    
    m_Port.SetOriginType(CGlPane::eOriginLeft, CGlPane::eOriginBottom);
    m_Port.EnableZoom(true, true);
}

CPhyloTreeWidget::~CPhyloTreeWidget()
{
    if (m_QueryPanel) {
        m_QueryPanel->RemoveListener(this);
        delete m_QueryPanel;
    }
}

static
WX_DEFINE_MENU(sPopupMenu)
    WX_MENU_ITEM(eCmdEditLabel)
    WX_MENU_ITEM(eCmdEditNode)
    WX_SUBMENU("Edit")
        WX_SUBMENU("Move")
            WX_MENU_ITEM(eCmdNodeCut)
            WX_MENU_ITEM(eCmdNodePaste)
            WX_MENU_SEPARATOR()
            WX_MENU_ITEM(eCmdMoveUp)
            WX_MENU_ITEM(eCmdMoveDown)
        WX_END_SUBMENU()
        WX_SUBMENU("Add")
            WX_MENU_ITEM(eCmdNodeNewChild)
            WX_MENU_ITEM(eCmdNodeNewParent)
        WX_END_SUBMENU()
        WX_SUBMENU("Remove")
            WX_MENU_ITEM(eCmdRemoveSubtree)
            WX_MENU_ITEM(eCmdRemoveSelected)
            WX_MENU_ITEM(eCmdRemoveNode)
        WX_END_SUBMENU()
        WX_MENU_ITEM(eCmdClean)
    WX_END_SUBMENU()

    WX_MENU_ITEM(eCmdSubtreeFromSelected)
    WX_SUBMENU("Sort")
        WX_MENU_ITEM(eCmdSort)
        WX_MENU_ITEM(eCmdSortDist)
        WX_MENU_ITEM(eCmdSortLabel)
        WX_MENU_ITEM(eCmdSortLabelRange)
        WX_MENU_SEPARATOR()
        WX_MENU_ITEM(eCmdSortAscending)
    WX_END_SUBMENU()

    WX_MENU_SEPARATOR()
    WX_SUBMENU("Layout")
        WX_MENU_ITEM(eCmdSetGraphType1)
        WX_MENU_ITEM(eCmdSetGraphType2)
        WX_MENU_ITEM(eCmdSetGraphType3)
        WX_MENU_ITEM(eCmdSetGraphType5)
        //WX_MENU_ITEM(eCmdSetGraphType4)
        WX_MENU_SEPARATOR()
        WX_MENU_ITEM(eCmdStopLayout)
        WX_MENU_SEPARATOR()
        WX_MENU_ITEM(eCmdRotateLabels)       
        WX_MENU_ITEM(eCmdUseDistances)
        WX_MENU_ITEM(eCmdUseSplines)
    WX_END_SUBMENU()

    WX_SUBMENU("Zoom")
        WX_MENU_ITEM(eCmdZoomIn)
        WX_MENU_ITEM(eCmdZoomOut)
        WX_MENU_ITEM(eCmdZoomAll)
        WX_MENU_ITEM(eCmdZoomSel)
    WX_END_SUBMENU()

    WX_SUBMENU("Zoom Behavior")
        WX_MENU_ITEM(eCmdZoomXY)
        WX_MENU_ITEM(eCmdZoomY)
        WX_MENU_ITEM(eCmdZoomX)
    WX_END_SUBMENU()

    WX_SUBMENU("Re-root")
        WX_MENU_ITEM(eCmdRerootTree)
        WX_MENU_ITEM(eCmdRerootTreeAtEdge)
        WX_MENU_ITEM(eCmdMidpointRoot)       
        WX_MENU_ITEM(eCmdZoomToSubtree)
    WX_END_SUBMENU()

    WX_SUBMENU("Label Visibility")
        WX_MENU_ITEM(eCmdLabelsForLeavesOnly)
        WX_MENU_ITEM(eCmdLabelsVisible)
        WX_MENU_ITEM(eCmdLabelsHidden)
        WX_MENU_SEPARATOR()
        WX_SUBMENU("Auto-Labeling")
            WX_MENU_ITEM(eCmdAlNone)
            WX_MENU_ITEM(eCmdAlPhylip)
            WX_MENU_ITEM(eCmdAlPaml)
        WX_END_SUBMENU()
    WX_END_SUBMENU()

    WX_MENU_ITEM(eCmdSetTreeLabel)  

    WX_MENU_SEPARATOR()

    WX_MENU_ITEM(eCmdShowAll)
    WX_MENU_ITEM(eCmdCollapseChildren)
    WX_MENU_ITEM(eCmdCollapseSelected)
    WX_MENU_ITEM(eCmdExpandChildren)
    WX_MENU_ITEM(eCmdHighlightEdges)
    WX_MENU_ITEM(eCmdCollapseToViewport)

    WX_MENU_SEPARATOR()

    WX_MENU_ITEM(eSelectAll)
    WX_MENU_ITEM(eClearSelection)
    WX_MENU_ITEM(eCmdAddSelectionSet)
    WX_MENU_ITEM(eCmdExportSelection)
    WX_MENU_ITEM(eCmdExportTree)

    WX_MENU_SEPARATOR()

    WX_MENU_ITEM(eCmdMySettings)
    WX_MENU_SEPARATOR()
    WX_MENU_ITEM(eCmdLoadAttributes)

WX_END_MENU()

BEGIN_EVENT_TABLE(CPhyloTreeWidget, CGlWidgetBase)
    EVT_MENU(wxID_PRINT, CPhyloTreeWidget::OnPrint)
    EVT_UPDATE_UI(wxID_PRINT, CPhyloTreeWidget::OnEnablePrintCmdUpdate)
    EVT_MENU(eCmdSaveImages, CPhyloTreeWidget::OnSaveImages)
    EVT_UPDATE_UI(eCmdSaveImages, CPhyloTreeWidget::OnEnableSaveImagesCmdUpdate)
    EVT_MENU(eCmdSavePdf, CPhyloTreeWidget::OnSavePdf)
    EVT_UPDATE_UI(eCmdSavePdf, CPhyloTreeWidget::OnEnableSavePdfCmdUpdate)

    EVT_CONTEXT_MENU(CPhyloTreeWidget::OnContextMenu)

    EVT_MENU(eCmdSetGraphType1, CPhyloTreeWidget::OnSetGraphType1)
    EVT_MENU(eCmdSetGraphType2, CPhyloTreeWidget::OnSetGraphType2)
    EVT_MENU(eCmdSetGraphType3, CPhyloTreeWidget::OnSetGraphType3)
    EVT_MENU(eCmdSetGraphType4, CPhyloTreeWidget::OnSetGraphType4)
    EVT_MENU(eCmdSetGraphType5, CPhyloTreeWidget::OnSetGraphType5)
    EVT_MENU(eCmdUseDistances,  CPhyloTreeWidget::OnUseDistances)
    EVT_MENU(eCmdRotateLabels,  CPhyloTreeWidget::OnRotateLabels)
    EVT_MENU(eCmdStopLayout,    CPhyloTreeWidget::OnStopLayout)
    EVT_MENU(eCmdUseSplines,    CPhyloTreeWidget::OnUseSplines)
    EVT_MENU(eCmdMySettings,    CPhyloTreeWidget::OnOpenPropertiesDlg)
    EVT_MENU(eCmdRerootTree,    CPhyloTreeWidget::OnRerootTree)
    EVT_MENU(eCmdRerootTreeAtEdge, CPhyloTreeWidget::OnRerootTreeAtEdge)
    EVT_MENU(eCmdMidpointRoot, CPhyloTreeWidget::OnSetMidpointRoot)
    EVT_MENU(eCmdLoadAttributes,CPhyloTreeWidget::OnLoadAttributes)

    EVT_MENU(eCmdEditNode,      CPhyloTreeWidget::OnEditNode)
    EVT_UPDATE_UI(eCmdEditNode, CPhyloTreeWidget::OnUpdateEditNode)

    EVT_MENU(eCmdFilterDistances,   CPhyloTreeWidget::OnFilterDistances)

    EVT_MENU(eSelectAll,        CPhyloTreeWidget::OnSelectAll)
    EVT_MENU(eClearSelection,   CPhyloTreeWidget::OnClearSelection)
    EVT_MENU(eCmdAddSelectionSet,   CPhyloTreeWidget::OnAddSelectionSet)
    EVT_MENU(eCmdExportSelection, CPhyloTreeWidget::OnExportSelection)
    EVT_MENU(eCmdExportTree, CPhyloTreeWidget::OnExportTree)

    EVT_MENU(eCmdCollapseChildren, CPhyloTreeWidget::OnCollapseChildren)
    EVT_MENU(eCmdExpandChildren, CPhyloTreeWidget::OnExpandChildren)
    EVT_MENU(eCmdCollapseSelected, CPhyloTreeWidget::OnCollapseSelected)
    EVT_MENU(eCmdHighlightEdges, CPhyloTreeWidget::OnHighlightEdges)
    EVT_MENU(eCmdCollapseToViewport, CPhyloTreeWidget::OnCollapseToViewport)

    EVT_MENU(eCmdZoomToSubtree,    CPhyloTreeWidget::OnZoomToSubtree)
    EVT_UPDATE_UI(eCmdZoomToSubtree,    CPhyloTreeWidget::OnUpdateZoomToSubtree)

    EVT_MENU(eCmdShowAll,    CPhyloTreeWidget::OnShowAll)
    EVT_UPDATE_UI(eCmdShowAll,   CPhyloTreeWidget::OnUpdateShowAll)

    EVT_UPDATE_UI(eCmdCollapseChildren, CPhyloTreeWidget::OnUpdateCollapseChildren)
    EVT_UPDATE_UI(eCmdExpandChildren, CPhyloTreeWidget::OnUpdateExpandChildren)
    EVT_UPDATE_UI(eCmdCollapseSelected, CPhyloTreeWidget::OnUpdateCollapseSelected)

    EVT_UPDATE_UI(eCmdRerootTree,    CPhyloTreeWidget::OnUpdateRerootTree)
    EVT_UPDATE_UI(eCmdRerootTreeAtEdge, CPhyloTreeWidget::OnUpdateRerootTreeAtEdge)    
    EVT_UPDATE_UI(eCmdMidpointRoot, CPhyloTreeWidget::OnUpdateSetMidpointRoot)
    EVT_UPDATE_UI(eCmdRotateLabels,  CPhyloTreeWidget::OnUpdateRotateLabels)
    EVT_UPDATE_UI(eCmdStopLayout,  CPhyloTreeWidget::OnUpdateStopLayout)
    EVT_UPDATE_UI(eCmdUseDistances,  CPhyloTreeWidget::OnUpdateUseDistances)
    EVT_UPDATE_UI(eCmdSortAscending,  CPhyloTreeWidget::OnUpdateSortAscending)
    EVT_UPDATE_UI(eCmdUseSplines,    CPhyloTreeWidget::OnUpdateUseSplines)
    EVT_UPDATE_UI(eCmdSetGraphType1, CPhyloTreeWidget::OnUpdateSetGraphType1)
    EVT_UPDATE_UI(eCmdSetGraphType2, CPhyloTreeWidget::OnUpdateSetGraphType2)
    EVT_UPDATE_UI(eCmdSetGraphType3, CPhyloTreeWidget::OnUpdateSetGraphType3)
    EVT_UPDATE_UI(eCmdSetGraphType4, CPhyloTreeWidget::OnUpdateSetGraphType4)
    EVT_UPDATE_UI(eCmdSetGraphType5, CPhyloTreeWidget::OnUpdateSetGraphType5)

    EVT_MENU(eCmdZoomXY,       CPhyloTreeWidget::OnZoomXY)
    EVT_UPDATE_UI(eCmdZoomXY,  CPhyloTreeWidget::OnUpdateZoomXY)
    EVT_MENU(eCmdZoomX,        CPhyloTreeWidget::OnZoomX)
    EVT_UPDATE_UI(eCmdZoomX,   CPhyloTreeWidget::OnUpdateZoomX)
    EVT_MENU(eCmdZoomY,        CPhyloTreeWidget::OnZoomY)
    EVT_UPDATE_UI(eCmdZoomY,   CPhyloTreeWidget::OnUpdateZoomY)
    EVT_MENU(eCmdZoomSel,      CPhyloTreeWidget::OnZoomToSelection)
    EVT_MENU(eCmdBack,         CPhyloTreeWidget::OnGoBack)
    EVT_MENU(eCmdForward,      CPhyloTreeWidget::OnGoForward)
    EVT_UPDATE_UI(eCmdBack,    CPhyloTreeWidget::OnUpdateGoBack)
    EVT_UPDATE_UI(eCmdForward, CPhyloTreeWidget::OnUpdateGoForward)
    EVT_UPDATE_UI(eCmdZoomSel, CPhyloTreeWidget::OnUpdateZoomSel)

    EVT_MENU(eCmdSort,      CPhyloTreeWidget::OnSort)
    EVT_MENU(eCmdSortDist,  CPhyloTreeWidget::OnSort)
    EVT_MENU(eCmdSortLabel, CPhyloTreeWidget::OnSort)
    EVT_MENU(eCmdSortLabelRange, CPhyloTreeWidget::OnSort)
    EVT_MENU(eCmdSortAscending, CPhyloTreeWidget::OnSortAscending)

    EVT_MENU(eCmdClean,     CPhyloTreeWidget::OnClean)
    EVT_MENU(eCmdFilter,    CPhyloTreeWidget::OnFilter)

    EVT_MENU_RANGE(eCmdLabelsForLeavesOnly, eCmdLabelsHidden,      CPhyloTreeWidget::OnLabels)
    EVT_UPDATE_UI_RANGE(eCmdLabelsForLeavesOnly, eCmdLabelsHidden, CPhyloTreeWidget::OnUpdateLabels)

    EVT_MENU(eCmdSetTreeLabel, CPhyloTreeWidget::OnSetTreeLabel)    

    EVT_MENU_RANGE(eCmdAlNone, eCmdAlPaml,      CPhyloTreeWidget::OnAl)
    EVT_UPDATE_UI_RANGE(eCmdAlNone, eCmdAlPaml, CPhyloTreeWidget::OnUpdateAl)

    EVT_MENU_RANGE(eCmdNodeCut, eCmdRemoveNode,      CPhyloTreeWidget::OnEdit)
    EVT_UPDATE_UI_RANGE(eCmdNodeCut, eCmdRemoveNode, CPhyloTreeWidget::OnUpdateEdit)

    EVT_MENU(eCmdEditLabel,      CPhyloTreeWidget::OnEditLabel)
    EVT_UPDATE_UI(eCmdEditLabel, CPhyloTreeWidget::OnUpdateEditLabel)

    EVT_MENU(eCmdSubtreeFromSelected, CPhyloTreeWidget::OnSubtreeFromSelection)
    EVT_UPDATE_UI(eCmdSubtreeFromSelected, CPhyloTreeWidget::OnUpdateSubtreeFromSelection)

    EVT_BUTTON(eCmdSearchTip, CPhyloTreeWidget::OnSearchTip)
    EVT_BUTTON(eCmdZoomTip, CPhyloTreeWidget::OnZoomTip)
    EVT_BUTTON(eCmdInfoTip, CPhyloTreeWidget::OnInfoTip)
    EVT_BUTTON(eCmdTipActive, CPhyloTreeWidget::OnTipActivated)
    EVT_BUTTON(eCmdTipInactive, CPhyloTreeWidget::OnTipDeactivated)

END_EVENT_TABLE()

void CPhyloTreeWidget::x_CreateControls()
{
#ifdef CRASH_DEBUG
    wxMessageBox(wxT("CPhyloTreeWidget::x_CreateControls()"), wxT(""), wxICON_WARNING | wxOK);
#endif
    // Make sure we have loaded needed bitmaps:
    static bool first_tree_window = false;
    if( !first_tree_window ){
        wxFileArtProvider* provider = GetDefaultFileArtProvider();
        first_tree_window = true;

        provider->RegisterFileAlias(wxT("menu::dm_start"), wxT("playhs.png"));
        provider->RegisterFileAlias(wxT("menu::dm_stop"),  wxT("stophs.png"));
        provider->RegisterFileAlias(wxT("menu::back"), wxT("back.png"));
        provider->RegisterFileAlias(wxT("menu::forward"), wxT("forward.png"));
        provider->RegisterFileAlias(wxT("menu::help"), wxT("help.png"));
    }

    CGlWidgetBase::x_CreateControls();
}

void CPhyloTreeWidget::SetQueryPanel(CQueryParsePanel* queryPanel)
{
    m_QueryPanel = queryPanel;
    m_QueryPanel->AddListener(this, ePool_Parent);
}

void CPhyloTreeWidget::OnContextMenu(wxContextMenuEvent& event)
{
    wxPoint ms_pos = ScreenToClient(event.GetPosition());

    m_PopupPoint.m_X = m_pPhyloTreePane->SHH_GetModelByWindow(ms_pos.x, eHorz);
    m_PopupPoint.m_Y = m_pPhyloTreePane->SHH_GetModelByWindow(ms_pos.y, eVert);

    CUICommandRegistry& cmd_reg = CUICommandRegistry::GetInstance();
    auto_ptr<wxMenu> menu(cmd_reg.CreateMenu(sPopupMenu));
    PopupMenu(menu.get());
}


void  CPhyloTreeWidget::SetPopupMenuItems(CMenuItem * itm)
{
    m_pPopupItems = itm;
}

void CPhyloTreeWidget::x_CreatePane()
{
    m_pPhyloTreePane = new CPhyloTreePane(this);

    // defauld model space for renderer
    double w = 2000.;
    double h = 2000.;
    
    m_pPhyloTreePane->AddRenderer(CRef<IPhyloTreeRender>(new CPhyloRectCladogram(w, h)));
    m_pPhyloTreePane->AddRenderer(CRef<IPhyloTreeRender>(new CPhyloSlantedCladogram(w, h)));
    m_pPhyloTreePane->AddRenderer(CRef<IPhyloTreeRender>(new CPhyloRadial(w, h)));
    m_pPhyloTreePane->AddRenderer(CRef<IPhyloTreeRender>(new CPhyloForce(w, h)));
    m_pPhyloTreePane->AddRenderer(CRef<IPhyloTreeRender>(new CPhyloCircularCladogram(w, h)));
}


CGlWidgetPane*    CPhyloTreeWidget::x_GetPane()
{
    return static_cast<CGlWidgetPane*>(m_pPhyloTreePane);
}


void CPhyloTreeWidget::x_SetPortLimits()
{
    if (m_pPhyloTreePane) {
        double DimX = m_pPhyloTreePane->GetCurrRenderer()->GetDimX();
        double DimY = m_pPhyloTreePane->GetCurrRenderer()->GetDimY();
        m_Port.SetModelLimitsRect(TModelRect(0, 0, DimX, DimY));
    }
}

void   CPhyloTreeWidget::SetPortLimits(const TModelRect & rect,  bool bZoomAll)
{
    const TModelRect mr = m_Port.GetModelLimitsRect();
    m_Port.SetModelLimitsRect(rect);

    double scale_factor = std::max(50.0, (double)(m_DataSource->GetNumNodes()/8));

    m_Port.SetMinScaleX(m_Port.GetZoomAllScaleX() / scale_factor);
    m_Port.SetMinScaleY(m_Port.GetZoomAllScaleY() / scale_factor);

    if (bZoomAll) {
        m_Port.ZoomAll();
    }
    else {
        TModelUnit sx = (rect.Width()  / mr.Width());
        TModelUnit sy = (rect.Height() / mr.Height());

        m_Port.SetScale(m_Port.GetScaleX() * sx, m_Port.GetScaleY() * sy);
        x_UpdateOnZoom();
    }
}

void CPhyloTreeWidget::SetDataSourceNoUpdate(CPhyloTreeDataSource* p_ds)
{
    m_DataSource = p_ds;

    m_pPhyloTreePane->UpdateDataSource();

    if (m_QueryPanel != NULL) {
        m_QueryPanel->SetDataSource(p_ds);
    }

    double scale_factor = std::max(50.0, (double)(p_ds->GetNumNodes()/8));
    m_Port.SetMinScaleX(m_Port.GetZoomAllScaleX() / scale_factor);
    m_Port.SetMinScaleY(m_Port.GetZoomAllScaleY() / scale_factor);   
}

void CPhyloTreeWidget::SetDataSource(CPhyloTreeDataSource* p_ds)
{
    if (m_QueryPanel != NULL) {
        m_QueryPanel->SetDataSource(p_ds);
    }

    double scale_factor = std::max(50.0, (double)(p_ds->GetNumNodes()/8));
    m_Port.SetMinScaleX(m_Port.GetZoomAllScaleX() / scale_factor);
    m_Port.SetMinScaleY(m_Port.GetZoomAllScaleY() / scale_factor);   

    if (!m_DataSource){   
        m_DataSource = p_ds;        
        x_Update();
    }
    else {    
        m_DataSource = p_ds;
        x_SoftUpdate();
    }
}

void CPhyloTreeWidget::RemoveCurrentDataSource()
{
    if (!m_DataSource.IsNull())
        m_DataSource->Clear();

    if (m_pPhyloTreePane) {
        m_pPhyloTreePane->RemoveCurrentDataSource();
    }

    if (m_QueryPanel != NULL) {
        m_QueryPanel->SetDataSource(NULL);
    }
}

void CPhyloTreeWidget::RedrawDataSource()
{
    // re-compute tree characteristics (such as children per-node)
    if (m_DataSource != NULL) {
        m_DataSource->MeasureTree();
        m_DataSource->Clusterize(m_pScheme);

        // layout tree
        x_SoftUpdate();
        
		// Redraw tree
        m_pPhyloTreePane->Refresh();
        // Recenter view on expand/colllapse
        if (m_LastCommand == eCmdNodeExpandCollapse) {

            // After expand/collapse, scroll view so that expanded/collapsed node remains
            // at it's same position (otherwise can lose node after expand/collapse)
            CPhyloTree::TTreeIdx idx = m_DataSource->GetTree()->GetCurrentNodeIdx();
            if (idx != CPhyloTree::Null()) {
                CPhyloTree::TNodeType& current =
                    m_DataSource->GetTree()->GetNode(m_DataSource->GetTree()->GetCurrentNodeIdx());
              
                TVPPoint node_screen_pos = m_PrevPane.Project(TModelUnit(m_PrevPos.X()), TModelUnit(m_PrevPos.Y()));
                float pctx = (node_screen_pos.X() - float(m_PrevPane.GetViewport().Left())) / float(m_PrevPane.GetViewport().Width());
                float pcty = (node_screen_pos.Y() - float(m_PrevPane.GetViewport().Bottom())) / float(m_PrevPane.GetViewport().Height());

                TModelRect vr = m_PrevPane.GetVisibleRect();
                TModelRect mr = m_PrevPane.GetModelLimitsRect();

                // Keep visible rect in same relative proportions after collapsing
                CGlPane& pane = GetPort(); //There is also m_pPhyloTreePane->GetPane(); but local pane is used for visible rect...
                pane.SetProportional(vr, mr);

                // Scroll viewport to keep newly collapsed/expanded node in same relative position
                CVect2<float> pos = current.GetValue().XY();
                pane.ScrollTo(CVect2<TModelUnit>(pos.X(), pos.Y()), pctx, pcty);
            }

            m_LastCommand = EPhyloTreeEditCommand::eEditCmdNone;
        }
    }
}

CPhyloTreeDataSource*  CPhyloTreeWidget::GetDS(void)
{
    return m_DataSource;
}


CGlPane& CPhyloTreeWidget::GetPort(void)
{
    return m_Port;
}


const CGlPane& CPhyloTreeWidget::GetPort(void) const
{
    return m_Port;
}

void    CPhyloTreeWidget::OnPrint(wxCommandEvent& /* evt */)
{
    m_pPhyloTreePane->SetContext();

    size_t w = m_pPhyloTreePane->GetPane().GetViewport().Width();
    size_t h = m_pPhyloTreePane->GetPane().GetViewport().Height();
    
    // create the CImage
    CRef<CImage> img(new CImage(w, h, 3));

    //_TRACE("Size (w,h,buf): (" << w << ", " << h << ", " << image_size << ")");

    // Read using single-byte alignment
    GLint alignment;
    glGetIntegerv(GL_PACK_ALIGNMENT, &alignment);
    glPixelStorei(GL_PACK_ALIGNMENT, 1);
    glReadPixels(0, 0, w, h, GL_RGB, GL_UNSIGNED_BYTE, img->SetData());
    glPixelStorei(GL_PACK_ALIGNMENT, alignment);
    img->Flip();
    
    //wxImage wximg(img->GetWidth(),
    //              img->GetHeight(), 
    //             (unsigned char*)img->GetData(), true);
    //wximg.SaveFile(wxT("c:\\temp\\image_save1.bmp"),wxBITMAP_TYPE_BMP);

    CPrintHanderWx  print_handler(img,
                                  1 /*1 page */,
                                  wxT("Print Test"),
                                  30 /* units per centimeter*/);
   
    print_handler.SetImage(img);


    //QuickPrint*  myprint = new QuickPrint( 5 /* 5 pages */,
    //                                       wxT("wxPrint test"),
    //                                       30 /* 30 units per centimeter */ );
    if (!print_handler.performPageSetup(true))
    {
        // user cancelled
        return;
    }
    
    wxPrintDialogData data(print_handler.getPrintData());
    wxPrinter printer(&data);
    const bool success = printer.Print(NULL, &print_handler, true /* show dialog */);
  
    
    if (!success)
    {
        std::cerr << "Failed!!\n";
        return;
    }
}

void CPhyloTreeWidget::OnEnablePrintCmdUpdate(wxUpdateUIEvent& event)
{
    // disable until print works correctly.
    event.Enable(false);
}

void CPhyloTreeWidget::OnSaveImages(wxCommandEvent& /* evt */)
{
    m_pPhyloTreePane->SetContext(); 
    float xsize, ysize;
    I3DTexture* pTexture = m_pPhyloTreePane->MMHH_GetTexture(xsize, ysize);
    CRef<CImage> img;
    if (pTexture) {
        img.Reset(pTexture->GenerateImage());
        if (img && (xsize != 1.0f || ysize != 1.0f))
            img.Reset(img->GetSubImage(0, 0, (size_t)(xsize*img->GetWidth()), (size_t)(ysize*img->GetHeight())));
    }

    if (!img) {
        size_t w = m_pPhyloTreePane->GetPane().GetViewport().Width();
        size_t h = m_pPhyloTreePane->GetPane().GetViewport().Height();
        img.Reset(new CImage(w, h, 3));
    }

    m_pPhyloTreePane->GetPane().SetModelLimitsRect(m_Port.GetModelLimitsRect());
    CPhyloImageGrabber grb(*m_pPhyloTreePane);     
    CSaveImagesSetupDlg dlg(img, &grb, this);
    CPhyloTreeScheme::TZoomBehavior zb =
        m_pScheme->GetZoomBehavior(m_pPhyloTreePane->GetCurrRenderer()->GetDescription());
    dlg.SetZoomBehavior(zb == CPhyloTreeScheme::eZoomX ||zb == CPhyloTreeScheme::eZoomXY,
                        zb == CPhyloTreeScheme::eZoomY || zb == CPhyloTreeScheme::eZoomXY);

    dlg.ShowModal();
    DlgOverlayFix(this);
}

void CPhyloTreeWidget::OnEnableSaveImagesCmdUpdate(wxUpdateUIEvent& event)
{
#ifdef USE_METAL
    event.Enable(false);
#else
    event.Enable(true);
#endif
}

void    CPhyloTreeWidget::OnSavePdf(wxCommandEvent & evt)
{
    CSaveTreePdfDlg  tree_pdf_dlg(m_pPhyloTreePane, this);
    tree_pdf_dlg.ShowModal();
    DlgOverlayFix(this);
}

void    CPhyloTreeWidget::OnEnableSavePdfCmdUpdate(wxUpdateUIEvent& evt)
{
    evt.Enable(true);
}

void    CPhyloTreeWidget::OnSetEqualScale(wxCommandEvent& /* evt */)
{
    TModelUnit scale = min(m_Port.GetScaleX(), m_Port.GetScaleY());
    m_Port.SetScale(scale, scale);
    x_UpdateOnZoom();
}

// zoom behavior
void CPhyloTreeWidget::OnZoomXY(wxCommandEvent& /* evt */) 
{
    // Set layout-specific zoom behavior
    m_pScheme->SetZoomBehavior(m_pPhyloTreePane->GetCurrRenderer()->GetDescription(), CPhyloTreeScheme::eZoomXY);        
    m_pScheme->SaveCurrentSettings();
    
    CGlPane& port = GetPort();

    /// With proportional zoom, visible region must be in 
    /// same proportion of limits rect, so if it isn't, update
    /// the scale to be equal be reducing the larger dimension.
    TModelRect v = port.GetVisibleRect();
    TModelRect l = port.GetModelLimitsRect();
    TModelRect new_rect = v;

    double limits_ratio = l.Width()/l.Height();
    double visible_ratio = v.Width()/v.Height();
    
    if (visible_ratio == limits_ratio) {
        return; 
    }
    else if (visible_ratio < limits_ratio) {
        double target_width = l.Width()*(v.Height()/l.Height());
        double center = (v.Left()+v.Right())/2.0;
        new_rect.SetLeft(center - target_width/2.0);
        new_rect.SetRight(center + target_width/2.0);       
    }
    else if (visible_ratio > limits_ratio) {
        double target_height = l.Height()*(v.Width()/l.Width());
        double center = (v.Top()+v.Bottom())/2.0;
        new_rect.SetBottom(center - target_height/2.0);
        new_rect.SetTop(center + target_height/2.0);
    }
    
    port.ZoomRect(new_rect);
    m_pPhyloTreePane->GetPane() = port;
    x_UpdateOnZoom();

    x_SoftUpdate();
    Refresh();
}

void CPhyloTreeWidget::OnZoomX(wxCommandEvent& /* evt */)  
{
    if (m_pPhyloTreePane->GetCurrRenderer() == NULL)
        return;

    m_pScheme->SetZoomBehavior(m_pPhyloTreePane->GetCurrRenderer()->GetDescription(), CPhyloTreeScheme::eZoomX);
    m_pScheme->SaveCurrentSettings();

    x_UpdateOnZoom();
    x_SoftUpdate();
    Refresh();
}

void CPhyloTreeWidget::OnZoomY(wxCommandEvent& /* evt */)  
{
    if (m_pPhyloTreePane->GetCurrRenderer() == NULL)
        return;

    m_pScheme->SetZoomBehavior(m_pPhyloTreePane->GetCurrRenderer()->GetDescription(), CPhyloTreeScheme::eZoomY);
    m_pScheme->SaveCurrentSettings();

    x_SoftUpdate();
    Refresh();
}

void CPhyloTreeWidget::OnZoomToSelection(wxCommandEvent& /* evt */)
{        
    CVect2<float> min_pos;
    CVect2<float> max_pos;

    if (m_DataSource->GetTree()->GetSelectedBoundary(min_pos, max_pos)) {
        TModelRect sel_rect;       
      
        sel_rect.Init(min_pos.X(), min_pos.Y(), max_pos.X(), max_pos.Y());

        TModelUnit def_node_size = (TModelUnit)
                m_pPhyloTreePane->GetCurrRenderer()->DefaultNodeSize();

        sel_rect.Inflate(def_node_size, def_node_size);
        // We ignore labels here, so selected labals may fall ouside of visible area

        CPhyloTreeScheme::TZoomBehavior zb =
            m_pScheme->GetZoomBehavior(m_pPhyloTreePane->GetCurrRenderer()->GetDescription());

        if  (zb == CPhyloTreeScheme::eZoomX) {
            // Only update X:
            TModelRect vis_rect = GetPort().GetVisibleRect();
            sel_rect.SetTop(vis_rect.Top());
            sel_rect.SetBottom(vis_rect.Bottom());
        }
        else if (zb == CPhyloTreeScheme::eZoomY) {
            // Only update Y:
            TModelRect vis_rect = GetPort().GetVisibleRect();
            sel_rect.SetLeft(vis_rect.Left());
            sel_rect.SetRight(vis_rect.Right());
        }

        ZoomRect(sel_rect);

        x_UpdateOnZoom();
    }
}

void CPhyloTreeWidget::OnGoBack(wxCommandEvent& /*evt*/)
{
    m_pPhyloTreePane->GoBack();
    x_UpdateOnZoom();
}

void CPhyloTreeWidget::OnGoForward(wxCommandEvent& /*evt*/)
{
    m_pPhyloTreePane->GoForward();
    x_UpdateOnZoom();
}

void CPhyloTreeWidget::OnUpdateGoBack(wxUpdateUIEvent& event)
{
    event.Enable(m_pPhyloTreePane->CanGoBack());
}

void CPhyloTreeWidget::OnUpdateGoForward(wxUpdateUIEvent& event)
{
    event.Enable(m_pPhyloTreePane->CanGoForward());
}

void CPhyloTreeWidget::OnUpdateZoomXY(wxUpdateUIEvent& evt)
{
    if (m_pPhyloTreePane->GetCurrRenderer() != NULL)
        evt.Check(m_pScheme->GetZoomBehavior(m_pPhyloTreePane->GetCurrRenderer()->GetDescription())==CPhyloTreeScheme::eZoomXY);
}

void CPhyloTreeWidget::OnUpdateZoomX(wxUpdateUIEvent& evt)
{
    if (m_pPhyloTreePane->GetCurrRenderer() != NULL)
        evt.Check(m_pScheme->GetZoomBehavior(m_pPhyloTreePane->GetCurrRenderer()->GetDescription())==CPhyloTreeScheme::eZoomX);
}

void CPhyloTreeWidget::OnUpdateZoomY(wxUpdateUIEvent& evt)
{
    if (m_pPhyloTreePane->GetCurrRenderer() != NULL)
        evt.Check(m_pScheme->GetZoomBehavior(m_pPhyloTreePane->GetCurrRenderer()->GetDescription())==CPhyloTreeScheme::eZoomY);
}

void CPhyloTreeWidget::OnUpdateZoomSel(wxUpdateUIEvent& evt)
{
    // only allow selection zoom if one or more nodes are selected    
    if ( m_DataSource->GetTree()->HasSelection() )
        evt.Enable(true);
    else
        evt.Enable(false);

}

void CPhyloTreeWidget::OnSetGraphType1(wxCommandEvent& /* evt */)
{
    SetCurrRenderer(0);
}


void CPhyloTreeWidget::OnSetGraphType2(wxCommandEvent& /* evt */)
{
    SetCurrRenderer(1);
}

void CPhyloTreeWidget::OnSetGraphType3(wxCommandEvent& /* evt */)
{
    SetCurrRenderer(2);
}

void CPhyloTreeWidget::OnSetGraphType4(wxCommandEvent& /* evt */)
{
    SetCurrRenderer(3);
}

void CPhyloTreeWidget::OnSetGraphType5(wxCommandEvent& /* evt */)
{
    SetCurrRenderer(4);
}

void CPhyloTreeWidget::OnUpdateSetGraphType1(wxUpdateUIEvent& evt)
{
    evt.Check(m_pPhyloTreePane->GetCurrRendererIdx()==0);
}


void CPhyloTreeWidget::OnUpdateSetGraphType2(wxUpdateUIEvent& evt)
{
    evt.Check(m_pPhyloTreePane->GetCurrRendererIdx()==1);
}


void CPhyloTreeWidget::OnUpdateSetGraphType3(wxUpdateUIEvent& evt)
{
    evt.Check(m_pPhyloTreePane->GetCurrRendererIdx()==2);
}


void CPhyloTreeWidget::OnUpdateSetGraphType4(wxUpdateUIEvent& evt)
{
    evt.Check(m_pPhyloTreePane->GetCurrRendererIdx()==3);
}

void CPhyloTreeWidget::OnUpdateSetGraphType5(wxUpdateUIEvent& evt)
{
    evt.Check(m_pPhyloTreePane->GetCurrRendererIdx()==4);
}

void CPhyloTreeWidget::OnRotateLabels(wxCommandEvent& /* evt */)
{
	CPhyloTreePane::TRenderers &vRend = m_pPhyloTreePane->GetRenderers();
    for (auto &renderer : vRend) {
		if (!renderer->SupportsRotatedLabels())
			continue;
		renderer->SetRotatedLabels(!renderer->GetRotatedLabels());
    }

    // This event saves the rendering options for the tree in the biotreecontainer.
    // only called for options that we want to save with the tree itself.
    CEvent evt(CEvent::eEvent_Message, eCmdRenderingOptionsChanged);
    Send(&evt, ePool_Parent);

    x_SoftUpdate();
}


void CPhyloTreeWidget::OnUpdateRotateLabels(wxUpdateUIEvent& evt)
{
    evt.Enable(false); 
    if (m_pPhyloTreePane->GetCurrRenderer() == NULL) {
        return;
    }

    IPhyloTreeRender* r = m_pPhyloTreePane->GetCurrRenderer();
    evt.Enable(r->SupportsRotatedLabels());
    evt.Check(r->GetRotatedLabels());
}

void CPhyloTreeWidget::SetRotateLabels(bool rot)
{
    bool update_layout = false;

    if (m_pPhyloTreePane->GetCurrRenderer() != NULL &&
        m_pPhyloTreePane->GetCurrRenderer()->GetRotatedLabels() != rot) {
            update_layout = true;
    }

	CPhyloTreePane::TRenderers &vRend = m_pPhyloTreePane->GetRenderers();
	for (auto &renderer : vRend) {
        if (renderer->SupportsRotatedLabels() && renderer->GetRotatedLabels() != rot) {
			renderer->SetRotatedLabels(rot);
        }
    }
    
    // If we changed the option for the current layout, update:
    if (update_layout)
        x_SoftUpdate();
}

bool CPhyloTreeWidget::GetRotateLabels() const
{
    // Get value from first renderer that supports it (the are currently
    // all set on/off together)
	CPhyloTreePane::TRenderers &vRend = m_pPhyloTreePane->GetRenderers();
	for (auto &renderer : vRend) {
        if (renderer->SupportsRotatedLabels())
            return renderer->GetRotatedLabels();
    }

    return false;
}

void CPhyloTreeWidget::OnStopLayout(wxCommandEvent& /* evt */)
{
    if (m_pPhyloTreePane->GetCurrRenderer() == NULL)
        return;

    IPhyloTreeRender* r = m_pPhyloTreePane->GetCurrRenderer();

    CPhyloForce* rforce = dynamic_cast<CPhyloForce*>(r);
    if (rforce) {
        rforce->StopLayout();
    }
}


void CPhyloTreeWidget::OnUpdateStopLayout(wxUpdateUIEvent& evt)
{
    evt.Enable(false);

    if (m_pPhyloTreePane->GetCurrRenderer() == NULL) {
        return;
    }

    IPhyloTreeRender* r = m_pPhyloTreePane->GetCurrRenderer();

    CPhyloForce* rforce = dynamic_cast<CPhyloForce*>(r);
    if (rforce) {
        evt.Enable(true);
    }
}

void CPhyloTreeWidget::OnUseDistances(wxCommandEvent& /* evt */)
{
	CPhyloTreePane::TRenderers &vRend = m_pPhyloTreePane->GetRenderers();
	if (vRend.empty())
		return;

    SetUseDistances(!(*vRend.begin())->GetDistRendering());

	// This event saves the rendering options for the tree in the biotreecontainer.
	// only called for options that we want to save with the tree itself.
	CEvent evt(CEvent::eEvent_Message, eCmdRenderingOptionsChanged);
	Send(&evt, ePool_Parent);

	x_SoftUpdate();
}

void CPhyloTreeWidget::SetUseDistances(bool bDist)
{
	CPhyloTreePane::TRenderers &vRend = m_pPhyloTreePane->GetRenderers();
	for (auto &renderer : vRend) {
		renderer->SetDistRendering(bDist && renderer->SupportsDistanceRendering());
	}
}

bool CPhyloTreeWidget::GetUseDistances() const
{
    // The first renderer (rect cladogram) supports distance rendering so
    // get value from there. (for renderers that don't support it, value
    // is always false).
	CPhyloTreePane::TRenderers &vRend = m_pPhyloTreePane->GetRenderers();
	if (vRend.empty())
		return true;
       
	return (*vRend.begin())->GetDistRendering();
}

void CPhyloTreeWidget::OnUpdateUseDistances(wxUpdateUIEvent& evt)
{
    if (m_pPhyloTreePane->GetCurrRenderer() != NULL) {
        evt.Enable(m_pPhyloTreePane->GetCurrRenderer()->SupportsDistanceRendering());
        evt.Check(m_pPhyloTreePane->GetCurrRenderer()->GetDistRendering());
        return;
    }

	CPhyloTreePane::TRenderers &vRend = m_pPhyloTreePane->GetRenderers();
	if (vRend.empty())
		return;

    evt.Check((*vRend.begin())->GetDistRendering());
}

void CPhyloTreeWidget::OnUseSplines(wxCommandEvent& /* evt */)
{
	CPhyloTreePane::TRenderers &vRend = m_pPhyloTreePane->GetRenderers();
	for (auto &renderer : vRend) {
		renderer->SetSplinesRendering(!renderer->GetSplinesRendering());
	}
	x_SoftUpdate();
}


void CPhyloTreeWidget::OnUpdateUseSplines(wxUpdateUIEvent& evt)
{
    evt.Check(m_pPhyloTreePane->GetCurrRenderer()->GetSplinesRendering());
}

void CPhyloTreeWidget::UpdateViewingArea()
{
    x_UpdateScrollbars();

    /// notify our parent that we've changed
    CEvent evt(CEvent::eEvent_Message, CViewEvent::eWidgetRangeChanged);
    Send(&evt, ePool_Parent);
}

// zoom preserving update
void CPhyloTreeWidget::x_SoftUpdate()
{
     m_pPhyloTreePane->SoftUpdate();
}

void CPhyloTreeWidget::x_SaveStates()
{
    if (m_pPhyloTreePane) {
        m_pPhyloTreePane->SaveCurrentView();
    }
}

void CPhyloTreeWidget::x_Update()
{
    x_SetPortLimits();    
    x_UpdatePane();
    x_UpdateScrollbars();
    x_RedrawControls();
}


void CPhyloTreeWidget::x_UpdatePane()
{
    if (m_pPhyloTreePane) {
        m_pPhyloTreePane->Update();
    }
}


void CPhyloTreeWidget::OnSetScaleXY(TModelUnit scale_x, const TModelPoint& point)
{
    double ratio = m_Port.GetScaleY()/m_Port.GetScaleX();

    CPhyloTreeScheme::TZoomBehavior zb =
        m_pScheme->GetZoomBehavior(m_pPhyloTreePane->GetCurrRenderer()->GetDescription());

    TModelUnit sx = scale_x;
    TModelUnit sy = scale_x;

    // handle cases of only one axes zoom
    if      (zb == CPhyloTreeScheme::eZoomX)    sy = m_Port.GetScaleY();
    else if (zb == CPhyloTreeScheme::eZoomY)    sx = m_Port.GetScaleX();
    else if (zb == CPhyloTreeScheme::eZoomXY)   sy *= ratio;

    m_Port.SetScaleRefPoint(sx, sy, point);

    x_UpdateOnZoom();
}

void CPhyloTreeWidget::DlgOverlayFix(wxWindow* /*win*/)
{
    m_pPhyloTreePane->DlgOverlayFix();
}

void CPhyloTreeWidget::OnUpdateSelChanged()
{    
    m_pPhyloTreePane->BufferedRender();
    m_pPhyloTreePane->Refresh();

    // Update active selection set:
    if (m_DataSource->GetTree()->HasSelection()) {
        vector<CPhyloTree::TTreeIdx> sel;        
        m_DataSource->GetTree()->GetExplicitlySelectedAndNotCollapsed(sel);
        CPhyloTree::TTreeIdx cur_node = m_DataSource->GetCurrentSearchNode();
        if (std::find(sel.begin(), sel.end(), cur_node) == sel.end())
            m_DataSource->ClearQueryResults();
    }
}

void CPhyloTreeWidget::SetCurrRenderer(int idx)
{
    m_pPhyloTreePane->SetFocus();
    m_pPhyloTreePane->SetCurrRendererIdx(idx);
    m_pScheme->SaveCurrentSettings();

    m_pScheme->SetLayoutIdx(idx);
    m_pScheme->SaveCurrentSettings();

    // This event saves the rendering options for the tree in the biotreecontainer.
    // only called for options that we want to save with the tree itself.
    CEvent evt(CEvent::eEvent_Message, eCmdRenderingOptionsChanged);
    Send(&evt, ePool_Parent);

    x_Update();
}

int CPhyloTreeWidget::GetCurrRenderer() const
{
    return m_pPhyloTreePane->GetCurrRendererIdx();
}

vector<string>  CPhyloTreeWidget::GetRenderersNames(void)
{
    vector<string> names;
    int size = m_pPhyloTreePane->GetRenderers().size();
    for (Int4 i=0; i<size; i++) {
        names.push_back(m_pPhyloTreePane->GetRenderers()[i]->GetDescription());
    }
    return names;
}


void  CPhyloTreeWidget::SendSelChangedEvent()
{
    // notify parents
    CEvent evt(CEvent::eEvent_Message, CViewEvent::eWidgetSelectionChanged);
    Send(&evt, ePool_Parent);

    OnUpdateSelChanged();
}

void CPhyloTreeWidget::OnLoadAttributes(wxCommandEvent& /* evt */)
{
    SWFileDlgData data(wxT("Open attributes file"), wxALL_FILES_PATTERN, wxFD_OPEN);
    string error = "";
    if ( NcbiFileBrowser(data) == wxID_OK) {
        wxString filename = data.GetFilename();
        try {
            this->SetCursor(*wxHOURGLASS_CURSOR);        
            CBioTreeAttrReader::TAttrTable table;
            {{
                CBioTreeAttrReader reader;
                CNcbiIfstream is(filename.fn_str());
                reader.Read(is, table);
            }}

            m_DataSource->ApplyAttributes(table, m_pScheme);
            x_Update(); 
            SendEditEvent();          
        } catch(CException& e)  {
            error = e.GetMsg();
        } catch(std::exception& e)  {
            error = e.what();
        } 
        this->SetCursor(*wxSTANDARD_CURSOR);
    }
    if( ! error.empty())    {
        wxString s = ToWxString(error);
        wxMessageBox(s, wxT("Error loading file."), wxOK | wxICON_ERROR);
    }
}

void CPhyloTreeWidget::OnOpenPropertiesDlg(wxCommandEvent& /* evt */)
{
    // temporary scheme - load from disk in case some other view has updated
    // the scheme since this view was created.
    CRef <CPhyloTreeScheme> tmp_scheme(new CPhyloTreeScheme());
    tmp_scheme->LoadCurrentSettings();
    
    // Label visibility is not saved to registry (should it be?) so set it here
    if (!m_pScheme.IsNull()) {
        tmp_scheme->SetLabelVisibility(m_pScheme->GetLabelVisibility());
    }

    // show the dialog
    string label_format = tmp_scheme->SetLabelFormat();
    CwxPhyloSettingsDlg dlg(this);
    dlg.SetParams(m_DataSource.GetPointer(), tmp_scheme.GetPointer());

    int result = dlg.ShowModal();
    DlgOverlayFix(this);

    if(result == wxID_OK) {  
        SetScheme(tmp_scheme.GetObject());
        m_DataSource->Relabel(tmp_scheme, tmp_scheme->SetLabelFormat());
        
        // The label format is also saved in the biotreecontainer so that it can be
        // tree-specific (scheme default used when it is not in the tree). 
        if (tmp_scheme->SetLabelFormat() != label_format) {
            CEvent evt(CEvent::eEvent_Message, eCmdLabelFormatChanged);
            Send(&evt, ePool_Parent);
        }

        x_SoftUpdate();
    }
}

void CPhyloTreeWidget::OnRerootTree(wxCommandEvent& /* evt */)
{
    CPhyloTree::TTreeIdx idx = m_DataSource->GetTree()->GetCurrentNodeIdx();
    if (idx != CPhyloTree::Null()) {      
        m_DataSource->ReRoot(idx);
        m_DataSource->GetTree()->ClearSelection();

        // Distances can change, so that means labels could change too
        m_DataSource->Relabel(m_pScheme, m_pScheme->SetLabelFormat());

        m_pPhyloTreePane->BufferedRender();
        m_pPhyloTreePane->Refresh();

        SendEditEvent();
    }        
}

void CPhyloTreeWidget::OnRerootTreeAtEdge(wxCommandEvent& /* evt */)
{
    if (m_DataSource->GetTree()->HasCurrentEdge()) {
        CPhyloTree::TTreeIdx child_idx, parent_idx;

        m_DataSource->GetTree()->GetCurrentEdge(child_idx, parent_idx);
        m_DataSource->ReRootEdge(child_idx);
        m_DataSource->GetTree()->ClearSelection();

        // Distances can change, so that means labels could change too
        m_DataSource->Relabel(m_pScheme, m_pScheme->SetLabelFormat());

        m_pPhyloTreePane->BufferedRender();
        m_pPhyloTreePane->Refresh();

        SendEditEvent();
    }
}

void CPhyloTreeWidget::OnSetMidpointRoot(wxCommandEvent& /* evt */)
{
    m_DataSource->ReRootMidpoint();
    m_DataSource->GetTree()->ClearSelection();

    // Distances can change, so that means labels could change too
    m_DataSource->Relabel(m_pScheme, m_pScheme->SetLabelFormat());

    m_pPhyloTreePane->BufferedRender();
    m_pPhyloTreePane->Refresh();

    SendEditEvent();
}

void CPhyloTreeWidget::OnUpdateSubtreeFromSelection(wxUpdateUIEvent& evt)
{
    bool has_current_node = 
        m_DataSource->GetTree()->GetCurrentNodeIdx() != CPhyloTree::Null();

    evt.Enable(m_DataSource->GetTree()->GetNumSelected() > 0 || has_current_node);
}

void CPhyloTreeWidget::OnUpdateRerootTree(wxUpdateUIEvent& evt)
{
    CPhyloTree::TTreeIdx idx = m_DataSource->GetTree()->GetCurrentNodeIdx();
    if (idx != CPhyloTree::Null() && 
        idx != m_DataSource->GetTree()->GetRootIdx()) {
            evt.Enable(true);
    }
    else {
            evt.Enable(false);
    }
}

void CPhyloTreeWidget::OnUpdateRerootTreeAtEdge(wxUpdateUIEvent& evt)
{
    if (m_DataSource->GetTree()->HasCurrentEdge()) {
        evt.Enable(true);
    }
    else {
        evt.Enable(false);
    }
}

void CPhyloTreeWidget::OnUpdateSetMidpointRoot(wxUpdateUIEvent& evt)
{
    evt.Enable(m_DataSource->GetTree()->GetNumNodes() > 1);
}

void CPhyloTreeWidget::OnUpdateCollapseChildren(wxUpdateUIEvent& evt)
{
    CPhyloTree::TTreeIdx idx = m_DataSource->GetTree()->GetCurrentNodeIdx();
    bool collapsable = false;
    if (idx != CPhyloTree::Null()) {
        CPhyloTree::TNodeType& current = 
            m_DataSource->GetTree()->GetNode(m_DataSource->GetTree()->GetCurrentNodeIdx());
        
        if (current.CanExpandCollapse(CPhyloNodeData::eHideChildren)) {     
            collapsable = true;
        }
    }

    evt.Enable(collapsable);
}

void CPhyloTreeWidget::OnUpdateExpandChildren(wxUpdateUIEvent& evt)
{
    CPhyloTree::TTreeIdx idx = m_DataSource->GetTree()->GetCurrentNodeIdx();
    bool expandable = false;
    if (idx != CPhyloTree::Null()) {
        CPhyloTree::TNodeType& current = 
            m_DataSource->GetTree()->GetNode(m_DataSource->GetTree()->GetCurrentNodeIdx());
        
        if (current.CanExpandCollapse(CPhyloNodeData::eShowChildren)) {     
            expandable = true;
        }
    }

    evt.Enable(expandable);
}

void CPhyloTreeWidget::OnUpdateCollapseSelected(wxUpdateUIEvent& evt)
{
    bool collapsable = false;

    vector<TTreeIdx> sel;
    m_DataSource->GetTree()->GetSelected(sel);
    TTreeIdx root_idx = m_DataSource->GetTree()->GetRootIdx();

    // Make sure at least one selected node is eligable to be collapsed
    for (size_t i = 0; i < sel.size(); ++i)
    {
        CPhyloTree::TTreeIdx node_idx = sel[i];

        // A node can be collapsed if it is not already collapsed and none of its parent nodes are collapsed.
        // The root node also cannot be collapsed.
        if (m_DataSource->GetTree()->GetNode(node_idx).CanExpandCollapse(CPhyloNodeData::eHideChildren)) {
            TTreeIdx parent_node_idx = m_DataSource->GetTree()->GetNode(node_idx).GetParent();
            while (parent_node_idx != root_idx) {
                if (!m_DataSource->GetTree()->GetNode(parent_node_idx).CanExpandCollapse(CPhyloNodeData::eHideChildren))
                    break;
                parent_node_idx = m_DataSource->GetTree()->GetNode(parent_node_idx).GetParent();
            }

            // no parent nodes were already collapsed, so this node can be collapsed:
            if (parent_node_idx == root_idx) {
                collapsable = true;
                break;
            }
        }        
    }

    evt.Enable(collapsable);
}


void CPhyloTreeWidget::SetScheme(CPhyloTreeScheme & sl)
{
    m_pScheme.Reset(&sl);
    m_pScheme->SaveCurrentSettings();
	CPhyloTreePane::TRenderers &vRend = m_pPhyloTreePane->GetRenderers();
	for (auto &renderer : vRend) {
        renderer->SetScheme(sl);
    }

    if (m_DataSource.NotNull()) 
        m_DataSource->GetModel().SetScheme(sl);
}

void CPhyloTreeWidget::SetRegistryPath(const string& reg_path)
{
    m_RegPath = reg_path;
}

void CPhyloTreeWidget::LoadSettings()
{
    CPhyloTreeScheme * style = new CPhyloTreeScheme();

    if (!style->LoadCurrentSettings())
        return;
    
    SetScheme(*style);

    if (m_QueryPanel != NULL) {
        m_QueryPanel->SetRegistryPath(m_RegPath);
        m_QueryPanel->LoadSettings();
    }
}

void CPhyloTreeWidget::SaveSettings() const
{
    if (m_QueryPanel != NULL)
        m_QueryPanel->SaveSettings();

    if (!m_pScheme.IsNull())
        m_pScheme->SaveCurrentSettings();
}

void CPhyloTreeWidget::OnSelectAll(wxCommandEvent& /* evt */)
{ 
    m_DataSource->GetTree()->SetSelection(m_DataSource->GetTree()->GetRootIdx(),
                                           true, true, false);
    x_SoftUpdate();
}

void CPhyloTreeWidget::OnClearSelection(wxCommandEvent& /* evt */)
{
    m_DataSource->GetTree()->ClearSelection();
    x_SoftUpdate();
}

 void CPhyloTreeWidget::OnAddSelectionSet(wxCommandEvent & evt)
 {
    string sel_name = m_QueryPanel->GetLastQuery();

    CSelectionSetEdit* edit = new CSelectionSetEdit();
    edit->GetPrevSet() = m_DataSource->GetSelectionSets();

    CPhyloSaveSelectiondlg dlg(this);
    dlg.SetSelections(m_DataSource, sel_name);
    dlg.ShowModal();

    m_DataSource->UpdateSelectionSets(m_pScheme);
    m_pPhyloTreePane->BufferedRender();
    m_pPhyloTreePane->Refresh();

    // This event saves the rendering options for the tree in the biotreecontainer.
    // This way, the updated selection sets will be saved with the tree and 
    // used when we re-initialize from the biotreecontainer on updates.
    edit->GetUpdatedSet() = m_DataSource->GetSelectionSets();
    if (edit->Updated()) {
        m_DataSource->GetTree()->GetSelectionSets() = edit->GetPrevSet();
        CEvent selection_set_change(CEvent::eEvent_Message, eCmdRenderingOptionsChanged, edit, CEvent::eRelease, this);
        Send(&selection_set_change, ePool_Parent);
    }
 }

 void CPhyloTreeWidget::OnExportSelection(wxCommandEvent & evt)
 {
     vector< CPhyloTree::TTreeIdx > selection;
     const CPhyloTree *tree = m_DataSource->GetTree();
     tree->GetSelected(selection);
     if (selection.empty())
         return;

     vector<wxString> columns;
     const CBioTreeFeatureDictionary::TFeatureDict &dict = m_DataSource->GetDictionary().GetFeatureDict();
     ITERATE(CBioTreeFeatureDictionary::TFeatureDict, itFeature, dict) {
         columns.push_back(itFeature->second);
     }

     CCSVSelectionExportDlg dlgExport(this);
     dlgExport.SetRegistryPath("Dialogs.ExportTreeViewToCSV");
     dlgExport.SetColumnsList(columns);

     if (dlgExport.ShowModal() != wxID_OK)
         return;

     wxString fileName = dlgExport.GetFileName();
     if (fileName.empty())
         return;

     auto_ptr<CNcbiOstream> os;
     os.reset(new CNcbiOfstream(fileName.fn_str(), IOS_BASE::out));

     if (os.get() == NULL){
         NCBI_THROW(
             CException, eUnknown,
             "File is not accessible"
             );
     }

     CCSVExporter exporter(*os, ',', '"');

     CBioTreeFeatureDictionary::TFeatureDict featuresToExport;
     vector<wxString> selected;
     dlgExport.GetSelectedColumns(selected);
     ITERATE(vector<wxString>, field, selected) {
         ITERATE(CBioTreeFeatureDictionary::TFeatureDict, itFeature, dict) {
             if (itFeature->second == *field)
                 featuresToExport.insert(*itFeature);
         }
     }

     if (dlgExport.GetWithHeaders()) {
         ITERATE(CBioTreeFeatureDictionary::TFeatureDict, itFeature, featuresToExport) {
             exporter.Field(itFeature->second);
         }
         exporter.NewRow();
     }
     
     bool leavesOnly = dlgExport.GetLeavesOnly();

     ITERATE(vector< CPhyloTree::TTreeIdx >, itIdx, selection) {
         const CPhyloTreeNode &node = tree->GetNode(*itIdx);
         if (leavesOnly && !node.IsLeaf())
             continue;
         
         const CBioTreeFeatureList &features = node.GetValue().GetBioTreeFeatureList();

         ITERATE(CBioTreeFeatureDictionary::TFeatureDict, itFeature, featuresToExport) {
             exporter.Field(features.GetFeatureValue(itFeature->first));
         }
         exporter.NewRow();
     }
 }
 
 void CPhyloTreeWidget::OnExportTree(wxCommandEvent & evt)
 {
     CEvent fwd_evt(CEvent::eEvent_Message, eCmdExportTree);
     Send(&fwd_evt, ePool_Parent);
 }

 void CPhyloTreeWidget::OnExpandChildren(wxCommandEvent& /* evt */)
{
    CPhyloTree::TTreeIdx idx = m_DataSource->GetTree()->GetCurrentNodeIdx();
    if (idx != CPhyloTree::Null()) {
        CPhyloTree::TNodeType& current = 
            m_DataSource->GetTree()->GetNode(m_DataSource->GetTree()->GetCurrentNodeIdx());
        
        if (current.CanExpandCollapse(CPhyloNodeData::eShowChildren)) {     
            SendEditEvent(eCmdNodeExpandCollapse);
        }
    }
}

void CPhyloTreeWidget::OnCollapseSelected(wxCommandEvent& /* evt */)
{
    m_DataSource->CollapseSelected();
    SendEditEvent();
}

void CPhyloTreeWidget::OnHighlightEdges(wxCommandEvent& /* evt */)
{
    // Enable for current renderer (only).
    m_pPhyloTreePane->GetCurrRenderer()->SetHighlightEdges(!m_pPhyloTreePane->GetCurrRenderer()->GetHighlightEdges());

    x_SoftUpdate();
}

void CPhyloTreeWidget::OnCollapseToViewport(wxCommandEvent& /* evt */)
{
    set<CPhyloNodeData::TID> collapsed_nodes;

    // Collapse nodes in tree from most to least distant until tree fits in viewport
    if (m_pPhyloTreePane->GetCurrRenderer() != NULL) {
        int target_leaves = m_pPhyloTreePane->GetCurrRenderer()->GetMaxLeavesVisible();

        CRef<SPriorityCollapse> c(new SPriorityCollapse(*m_DataSource->GetTree()));

        collapsed_nodes = m_DataSource->CollapseByDistance(target_leaves, c.GetPointer());

        // Event collapses nodes (and supports undo of collapse)
        if (collapsed_nodes.size() > 0) {
            CExpandCollapseNodes* ec = new CExpandCollapseNodes();
            ec->SetExpanded(CPhyloNodeData::eHideChildren);           
            ec->GetIds().insert(ec->GetIds().begin(), collapsed_nodes.begin(), collapsed_nodes.end());
            m_DataSource->SetCollapsedLabels(ec->GetIds());

            CEvent evt(CEvent::eEvent_Message, eCmdGrpExpandCollapse, ec, CEvent::eRelease, this);
            Send(&evt, ePool_Parent);
        }
    }
}

void CPhyloTreeWidget::OnCollapseChildren(wxCommandEvent& /* evt */)
{
    CPhyloTree::TTreeIdx idx = m_DataSource->GetTree()->GetCurrentNodeIdx();
    if (idx != CPhyloTree::Null()) {
        CPhyloTree::TNodeType& current = m_DataSource->GetTree()->GetNode(idx);
        
        if (current.CanExpandCollapse(CPhyloNodeData::eHideChildren)) {
            m_DataSource->SetCollapsedLabel(idx);
            SendEditEvent(eCmdNodeExpandCollapse);
        }
    }
}

void CPhyloTreeWidget::OnZoomToSubtree(wxCommandEvent& /* evt */)
{
    CPhyloTree::TTreeIdx idx = m_DataSource->GetTree()->GetCurrentNodeIdx();
    if (idx != CPhyloTree::Null()) {
        CPhyloTree::TTreeIdx parent_idx = m_DataSource->GetTree()->GetNode(idx).GetParent();

        // Re-root the tree to currently selected node
        m_DataSource->ReRoot(idx);

        // invalidate selections (would need to reselect otherwise since topology changed)
        m_DataSource->GetTree()->ClearSelection();

        // if possible, hide other nodes
        if (parent_idx != CPhyloTree::Null()) {
            CPhyloTree::TNodeType& node = 
                m_DataSource->GetTree()->GetNode(parent_idx);
        
            // Don't use SendEditEvent(eCmdNodeExpandCollapse) for 
            // expand/collapse here (undo for this operation will 
            // undo both reroot and collapse)
            if (node.CanExpandCollapse(CPhyloNodeData::eHideChildren)) {     
                node.ExpandCollapse(m_DataSource->GetTree()->GetFeatureDict(),
                                    CPhyloNodeData::eHideChildren);
            }
        }
    }    

    SendEditEvent();
}

void CPhyloTreeWidget::OnUpdateZoomToSubtree(wxUpdateUIEvent& evt)
{
    evt.Enable(false); 
    
    CPhyloTree::TTreeIdx idx = m_DataSource->GetTree()->GetCurrentNodeIdx();

    if (idx != CPhyloTree::Null() && 
        idx != m_DataSource->GetTree()->GetRootIdx()) {
            CPhyloTreeNode& current = m_DataSource->GetTree()->GetNode(idx);
            if (current.Expanded() && !current.IsLeafEx()) 
                evt.Enable(true);
    }
}


void CPhyloTreeWidget::OnSearchTip(wxCommandEvent & evt)
{
    // Need the tool tip window for the event - we stored its pointer in
    // the button's (event objects) client data when the button was created
    wxEvtHandler* obj = dynamic_cast<wxEvtHandler*>(evt.GetEventObject());
    if (obj != NULL) {
        CTooltipFrame* f = static_cast<CTooltipFrame*>(obj->GetClientData());
        std::string tip_id = f->GetTipInfo().GetTipID();

        int id = NStr::StringToInt(tip_id);
        CPhyloTree::TTreeIdx idx = 
            m_DataSource->GetTree()->FindNodeById(CPhyloTree::TID(id));
        
        if (idx != CPhyloTreeNode::Null()){  
            CVect2<float> pos = m_DataSource->GetTree()->GetNode(idx)->XY();
            
            wxPoint win_pos = m_pPhyloTreePane->GetScreenPosition();

            // move to center of screen (if current zoom level allows)
            TModelUnit center_x = m_pPhyloTreePane->GetScreenRect().x +
                                  m_pPhyloTreePane->GetScreenRect().GetWidth()/2 - 
                                  win_pos.x;
            TModelUnit center_y = m_pPhyloTreePane->GetScreenRect().y - win_pos.y;
            center_y = m_pPhyloTreePane->GetRect().GetHeight()/2-center_y;

            // Get rectangle for tool tip within the window using standard
            // cartesian (and opengl) coords. (Lower left is (0,0)).
            wxRect tip_rect = f->GetScreenRect();
            tip_rect.x = tip_rect.x - win_pos.x;
            tip_rect.y = tip_rect.y - win_pos.y;
            tip_rect.y = m_pPhyloTreePane->GetRect().GetHeight() -
                         (tip_rect.y + tip_rect.GetHeight());

            // Could potentially add code to make sure glyph doesn't wind up
            // under this, or some other, tooltip here.  But that's not easy
            // since we don't have pointers to other tips (here) and we don't
            // know how much the actual scroll will be since it is constrained
            // when we are zoomed out.
           
            TModelPoint center_model_coord = 
                m_pPhyloTreePane->GetPane().UnProject((int)center_x, (int)center_y);

            double dx = center_model_coord.X()-pos.X();
            double dy = center_model_coord.Y()-pos.Y();         
                        
            Scroll(-dx, -dy);

            std::string tip_id = f->GetTipInfo().GetTipID();
            int id = NStr::StringToInt(tip_id);          
            m_pPhyloTreePane->GetCurrRenderer()->PointToNode(id, tip_rect, 0.5);
            m_pPhyloTreePane->Refresh();
        }
    }
}

void CPhyloTreeWidget::OnZoomTip(wxCommandEvent & evt)
{
    // Need the tool tip window for the event - we stored its pointer in
    // the button's (event objects) client data when the button was created
    wxEvtHandler* obj = dynamic_cast<wxEvtHandler*>(evt.GetEventObject());
    if (obj != NULL) {
        CTooltipFrame* f = static_cast<CTooltipFrame*>(obj->GetClientData());
        std::string tip_id = f->GetTipInfo().GetTipID();

        int id = NStr::StringToInt(tip_id);
        TTreeIdx idx = 
            m_DataSource->GetTree()->FindNodeById(CPhyloTree::TID(id));

        if (idx != NULL_TREE_IDX) {
            CPhyloTreeNode& tip_node  = m_DataSource->GetTree()->GetNode(idx);
            TModelPoint  pt((tip_node)->X(), (tip_node)->Y());      

            // Scroll to the glyph prior to zooming:
            OnSearchTip(evt);
            
            float def_node_size = (float)
                m_pPhyloTreePane->GetCurrRenderer()->GetDefaultNodeSize(&tip_node);
            
            // Compute distance between nodes here since not all renderers
            // do it the same (and they return int, but we can't round)
            float dimy = (float)m_pPhyloTreePane->GetCurrRenderer()->GetDimY();
            float size = (float)m_DataSource->GetSize();
            float scale_y = (float)m_pPhyloTreePane->GetPane().GetScaleY();
            float dist_between_nodes =  dimy/(size * scale_y);

            // should not happen, but just in case:
            if (dist_between_nodes <= 0.0f) {              
                m_pPhyloTreePane->Refresh();
                return;
            }                           
           
            // Normally labels are displayed if the distance between nodes 
            // is > 2*node size.  We set the zoom factor here to be twice that
            // zoom level. We do not zoom in more if view is already zoomed.
            float zfact = 4.0f*def_node_size/dist_between_nodes;

            if (zfact > 1.0f) {

                // Zoom based on current zoom behavior.  If user has selected
                // horizonal zoom, zooming in generally won't provide much
                // if any benefit.  With veritcal or proportional zoom, this
                // should guarantee label visibility.
                CPhyloTreeScheme::TZoomBehavior zb = m_pScheme->GetZoomBehavior(m_pPhyloTreePane->GetCurrRenderer()->GetDescription());
                CGlPane::EZoomOptions options = CGlPane::fZoomXY;
                if (zb == CPhyloTreeScheme::eZoomX) 
                    options = CGlPane::fZoomX;
                else if (zb == CPhyloTreeScheme::eZoomY) 
                    options = CGlPane::fZoomY;

                ZoomPoint(pt, zfact, options);
            }
            m_pPhyloTreePane->Refresh();
        }
    }
}

void CPhyloTreeWidget::OnInfoTip(wxCommandEvent & evt)
{
    // Need the tool tip window for the event - we stored its pointer in
    // the button's (event objects) client data when the button was created
    wxEvtHandler* obj = dynamic_cast<wxEvtHandler*>(evt.GetEventObject());
    if (obj != NULL) {
        CTooltipFrame* f = static_cast<CTooltipFrame*>(obj->GetClientData());
        std::string tip_id = f->GetTipInfo().GetTipID();
      
        int id = NStr::StringToInt(tip_id);
        CPhyloTree::TTreeIdx tip_node_idx = 
            m_DataSource->GetTree()->FindNodeById(CPhyloTree::TID(id));

        if (tip_node_idx != CPhyloTree::Null()) {
            CPhyloTreeNode& n = m_DataSource->GetTree()->GetNode(tip_node_idx);

            CFeatureEdit* feat_edit(new CFeatureEdit());
            feat_edit->GetDictionary() = m_DataSource->GetDictionary();
            feat_edit->GetUpdated().push_back(CUpdatedFeature());
            feat_edit->GetUpdated()[0].SetNode(n->GetId(), tip_node_idx);          
            feat_edit->GetUpdated()[0].GetPrevFeatures() = n->GetBioTreeFeatureList();

            CwxPhyloEditDlg dlg(this);
            dlg.SetParams(m_DataSource->GetTree(), tip_node_idx, feat_edit);

            int result = dlg.ShowModal();
            DlgOverlayFix(this);

            if(result == wxID_OK) {
                m_pScheme->SetTooltipFormat() = m_DataSource->GenerateTooltipFormat();
                CEvent evt(CEvent::eEvent_Message, eCmdFeaturesEdited, feat_edit, CEvent::eRelease, this);
                Send(&evt, ePool_Parent);
            }  
            else {
                delete feat_edit;
            }
        }        
    }
}

void CPhyloTreeWidget::OnTipActivated(wxCommandEvent & evt)
{
    // Need the tool tip window for the event - we stored its pointer in
    // the button's (event objects) client data when the button was created
    wxEvtHandler* obj = dynamic_cast<wxEvtHandler*>(evt.GetEventObject());
    if (obj != NULL) {
        CTooltipFrame* f = static_cast<CTooltipFrame*>(obj->GetClientData());
        std::string tip_id = f->GetTipInfo().GetTipID();
        int id = NStr::StringToInt(tip_id);
        m_pPhyloTreePane->GetCurrRenderer()->SetActiveTooltipNode(id);
        m_pPhyloTreePane->Refresh();
    }
}

void CPhyloTreeWidget::OnTipDeactivated(wxCommandEvent& /* evt */)
{
    m_pPhyloTreePane->GetCurrRenderer()->SetActiveTooltipNode(-1);
    m_pPhyloTreePane->Refresh();
}

void CPhyloTreeWidget::OnShowAll(wxCommandEvent& /* evt */)
{
    vector<CPhyloTree::TTreeIdx> collapsed = 
        m_DataSource->GetTree()->GetAllCollapsed();

    if (collapsed.size() == 0)
        return;

    CExpandCollapseNodes* ec = new CExpandCollapseNodes();
    ec->SetExpanded(CPhyloNodeData::eShowChildren);

    for (size_t i=0; i<collapsed.size(); ++i) {
        CPhyloTreeNode& current= (*m_DataSource->GetTree())[collapsed[i]];
        ec->AddNode(current->GetId());
    }
    
    CEvent evt(CEvent::eEvent_Message, eCmdGrpExpandCollapse, ec, CEvent::eRelease, this);
    Send(&evt, ePool_Parent);
}

void CPhyloTreeWidget::OnUpdateShowAll(wxUpdateUIEvent& evt)
{    
    evt.Enable(true);
}

void CPhyloTreeWidget::OnEditNode(wxCommandEvent& /* evt */)
{       
    if (m_DataSource->GetTree()->HasCurrentNode()){ 
        CPhyloTreeNode& hover = m_DataSource->GetTree()->GetCurrentNode();

        CwxPhyloEditDlg dlg(this);

        CFeatureEdit* feat_edit(new CFeatureEdit());
        feat_edit->GetDictionary() = m_DataSource->GetDictionary();
        feat_edit->GetUpdated().push_back(CUpdatedFeature());
        feat_edit->GetUpdated()[0].SetNode(hover->GetId(), m_DataSource->GetTree()->GetCurrentNodeIdx());
        feat_edit->GetUpdated()[0].GetPrevFeatures() = hover->GetBioTreeFeatureList();
        dlg.SetParams(m_DataSource->GetTree(),  
                      m_DataSource->GetTree()->GetCurrentNodeIdx(), 
                      feat_edit);

        int result = dlg.ShowModal();
        DlgOverlayFix(this);

        if(result == wxID_OK) {
            m_pScheme->SetTooltipFormat() = m_DataSource->GenerateTooltipFormat();
            CEvent evt(CEvent::eEvent_Message, eCmdFeaturesEdited, feat_edit, CEvent::eRelease, this);
            Send(&evt, ePool_Parent);
        }  
        else {
            delete feat_edit;
        }
    }
}

void CPhyloTreeWidget::OnUpdateEditNode(wxUpdateUIEvent& evt)
{      
    evt.Enable(m_DataSource->GetTree()->HasCurrentNode());
}


void CPhyloTreeWidget::OnFilterDistances(wxCommandEvent& /* evt */)
{
    /*
    auto_ptr <CPhyloTreeFilterDlg> filterDialog(new CPhyloTreeFilterDlg(m_pDataSource->GetMinDistance(),
                                                                        m_pDataSource->GetNormDistance()));
    int result = dlg.ShowModal();
    DlgOverlayFix(this);

    if(result == wxID_OK) {
        m_pDataSource->FilterDistances(filterDialog->GetDistance());
        m_pDataSource->Refresh();
        x_SoftUpdate();
        SendEditEvent();
    }
    */
}

void    CPhyloTreeWidget::x_ZoomIn(int /* options */)
{   
    CPhyloTreeScheme::TZoomBehavior zb =
        m_pScheme->GetZoomBehavior(m_pPhyloTreePane->GetCurrRenderer()->GetDescription());

    if      (zb == CPhyloTreeScheme::eZoomX) CGlWidgetBase::x_ZoomIn(CGlPane::fZoomX);
    else if (zb == CPhyloTreeScheme::eZoomY) CGlWidgetBase::x_ZoomIn(CGlPane::fZoomY);
    else                                     CGlWidgetBase::x_ZoomIn(CGlPane::fZoomXY);
}


void CPhyloTreeWidget::x_ZoomOut(int /* options */)
{   
    CPhyloTreeScheme::TZoomBehavior zb =
        m_pScheme->GetZoomBehavior(m_pPhyloTreePane->GetCurrRenderer()->GetDescription());

    if      (zb == CPhyloTreeScheme::eZoomX) CGlWidgetBase::x_ZoomOut(CGlPane::fZoomX);
    else if (zb == CPhyloTreeScheme::eZoomY) CGlWidgetBase::x_ZoomOut(CGlPane::fZoomY);
    else                                     CGlWidgetBase::x_ZoomOut(CGlPane::fZoomXY);
}

void CPhyloTreeWidget::OnSort(wxCommandEvent& evt)
{
    switch (evt.GetId()){
        case eCmdSort:
            m_DataSource->Sort(m_SortAscending);
            break;
        case eCmdSortDist:
            m_DataSource->SortDist(m_SortAscending);
            break;
        case eCmdSortLabel:
            m_DataSource->SortLabel(m_SortAscending);
            break;
        case eCmdSortLabelRange:
            m_DataSource->SortLabelRange(m_SortAscending);
            break;
        default:
            return;
    }

	SendEditEvent(eCmdTreeSorted);
}

void CPhyloTreeWidget::OnUpdateSortAscending(wxUpdateUIEvent& evt)
{
    evt.Check(m_SortAscending);
}

void CPhyloTreeWidget::OnSortAscending(wxCommandEvent & evt)
{
    m_SortAscending = !m_SortAscending;
}

void CPhyloTreeWidget::OnClean(wxCommandEvent& /* evt */)
{
    m_DataSource->Clean();
    m_DataSource->Relabel(m_pScheme, m_pScheme->SetLabelFormat());
    SendEditEvent();
}

void CPhyloTreeWidget::OnFilter(wxCommandEvent& /* evt */)
{
    m_DataSource->Filter();
    SendEditEvent();
}

void CPhyloTreeWidget::OnLabels(wxCommandEvent & evt)
{
    switch (evt.GetId()){
    case eCmdLabelsForLeavesOnly:
        m_pScheme->SetLabelVisibility(CPhyloTreeScheme::eLabelsForLeavesOnly);
        break;
    case eCmdLabelsVisible:
        m_pScheme->SetLabelVisibility(CPhyloTreeScheme::eLabelsVisible);
        break;
    case eCmdLabelsHidden:
        m_pScheme->SetLabelVisibility(CPhyloTreeScheme::eLabelsHidden);
        break;
    default:
        break;
    }
    x_SoftUpdate();
}

void CPhyloTreeWidget::OnUpdateLabels(wxUpdateUIEvent& evt)
{
    switch (evt.GetId()){
    case eCmdLabelsForLeavesOnly:
        evt.Check(m_pScheme->GetLabelVisibility()==CPhyloTreeScheme::eLabelsForLeavesOnly);
        break;
    case eCmdLabelsVisible:
        evt.Check(m_pScheme->GetLabelVisibility()==CPhyloTreeScheme::eLabelsVisible);
        break;
    case eCmdLabelsHidden:
        evt.Check(m_pScheme->GetLabelVisibility()==CPhyloTreeScheme::eLabelsHidden);
        break;
    default:
        break;
    }
}

void CPhyloTreeWidget::OnAl(wxCommandEvent & evt)
{
    switch (evt.GetId()){
    case eCmdAlNone:
        m_pScheme->SetAutoLabels(CPhyloTreeScheme::eAlNone);
        break;
    case eCmdAlPhylip:
        m_pScheme->SetAutoLabels(CPhyloTreeScheme::eAlPhylip);
        break;
    case eCmdAlPaml:
        m_pScheme->SetAutoLabels(CPhyloTreeScheme::eAlPaml);
        break;
    default:
        break;
    }

    x_SoftUpdate();
}

void CPhyloTreeWidget::OnUpdateAl(wxUpdateUIEvent& evt)
{
    switch (evt.GetId()){
    case eCmdAlNone:
        evt.Check(m_pScheme->GetAutoLabels()==CPhyloTreeScheme::eAlNone);
        break;
    case eCmdAlPhylip:
        evt.Check(m_pScheme->GetAutoLabels()==CPhyloTreeScheme::eAlPhylip);
        break;
    case eCmdAlPaml:
        evt.Check(m_pScheme->GetAutoLabels()==CPhyloTreeScheme::eAlPaml);
        break;
    default:
        break;
    }
}

void CPhyloTreeWidget::OnSetTreeLabel(wxCommandEvent& evt)
{
    CwxTreeLabelEdit dlg(this);
    dlg.SetParams(m_DataSource.GetPointer(), m_pScheme);

    int result = dlg.ShowModal();
    DlgOverlayFix(this);

    if(result == wxID_OK) {
        CEvent evt(CEvent::eEvent_Message, eCmdTreeLabelSet);
        Send(&evt, ePool_Parent);

        x_SoftUpdate();
    }
}

void CPhyloTreeWidget::OnEdit(wxCommandEvent & evt)
{  
    switch (evt.GetId()){
    case eCmdNodeCut:        m_DataSource->Cut();            break;
    case eCmdNodePaste:      m_DataSource->Paste();          break;
    case eCmdNodeNewChild:   m_DataSource->NewNode();        break;
    case eCmdNodeNewParent:  m_DataSource->NewNode(false);   break;
    case eCmdRemoveNode:     m_DataSource->Remove(false);    break;
    case eCmdRemoveSelected: m_DataSource->RemoveSelected(); break;
    case eCmdRemoveSubtree:  m_DataSource->Remove();         break;
    case eCmdMoveUp:         m_DataSource->MoveNode(true);   break;
    case eCmdMoveDown:       m_DataSource->MoveNode(false);  break;
    default: break;
    }

    //x_SoftUpdate();

    SendEditEvent();
}

void CPhyloTreeWidget::OnUpdateEdit(wxUpdateUIEvent& evt)
{
    switch (evt.GetId()){
    case eCmdNodeCut:
        evt.Enable((m_DataSource->GetTree()->HasCurrentNode()) &&
                   (m_DataSource->GetTree()->GetCurrentNode().HasParent()));
        break;
    case eCmdNodePaste:
        evt.Enable((m_DataSource->GetTree()->HasCurrentNode()) &&
                   !(m_DataSource->ClipboardEmpty()));
        break;
    case eCmdNodeNewChild:
        evt.Enable(m_DataSource->GetTree()->HasCurrentNode());
        break;
    case eCmdNodeNewParent:
    case eCmdMoveUp:
    case eCmdMoveDown:
        evt.Enable((m_DataSource->GetTree()->HasCurrentNode()) &&
                   (m_DataSource->GetTree()->GetCurrentNode().HasParent()));
        break;
    case eCmdRemoveNode:
    case eCmdRemoveSubtree:
        evt.Enable(m_DataSource->GetTree()->HasCurrentNode());
        break;
    case eCmdRemoveSelected:
        evt.Enable(m_DataSource->GetTree()->HasSelection());
        break;
    default:
        break;
    }
}

void CPhyloTreeWidget::SendEditEvent(EPhyloTreeEditCommand ec)
{
    m_LastCommand = ec;

    if (ec == eCmdNodeExpandCollapse) {


        if (m_DataSource->GetTree()->HasCurrentNode()) { 
            CPhyloTree::TNodeType& current = 
                m_DataSource->GetTree()->GetCurrentNode();

            // Saving these values allows us to recenter viewport after expand/collapse completed
            m_PrevPane = m_pPhyloTreePane->GetPane();
            m_PrevPos = current.GetValue().XY();

            CFeatureEdit* feat_edit(new CFeatureEdit());
            feat_edit->GetDictionary() = m_DataSource->GetDictionary();

            CUpdatedFeature f; 

            f.GetPrevFeatures() = (*current).GetBioTreeFeatureList();
            f.GetFeatures() = (*current).GetBioTreeFeatureList();
            TBioTreeFeatureId feat_id = feat_edit->m_Dictionary.Register("$NODE_COLLAPSED");

            bool update = false;

            // Update features (but not prevfeatures)
            switch ((*current).GetDisplayChildren()){
            case CPhyloNodeData::eHideChildren:
                if (current.CanExpandCollapse(CPhyloNodeData::eShowChildren)) {
                    f.GetFeatures().SetFeature(feat_id, "0"); //showdhilds
                    update = true;
                }
                break;
            case CPhyloNodeData::eShowChildren:
                if (current.CanExpandCollapse(CPhyloNodeData::eHideChildren)){
                    f.GetFeatures().SetFeature(feat_id, "1"); //hidechildren
                    update = true;
                }
                break;
            }

            // Caller should have already updated feature in node in this case so we
            // just grab that and the (possibly) updated dictionary.

            if (!update) {
                delete feat_edit;
                return;
            }

            f.SetNode((*current).GetId(), m_DataSource->GetTree()->GetCurrentNodeIdx());          

            feat_edit->GetUpdated().push_back(f);
            CEvent evt(CEvent::eEvent_Message, ec, feat_edit, CEvent::eRelease, this);
            Send(&evt, ePool_Parent);
        }
    }
    else {
        CEvent evt(CEvent::eEvent_Message, ec);
        Send(&evt, ePool_Parent);
    }
}


void CPhyloTreeWidget::OnUpdateEditLabel(wxUpdateUIEvent& evt)
{
    evt.Enable(m_DataSource->GetTree()->HasCurrentNode());
}

void CPhyloTreeWidget::OnEditLabel(wxCommandEvent& /* evt */)
{
    EditLabel();
}

void CPhyloTreeWidget::EditLabel()
{       
    if (m_DataSource->GetTree()->HasCurrentNode()) { 
        CPhyloTreeNode& hover = m_DataSource->GetTree()->GetCurrentNode();
        CwxLabelEditDlg dlg(this);
        dlg.SetParams( m_DataSource->GetTree(), m_DataSource->GetTree()->GetCurrentNodeIdx());

        int result = dlg.ShowModal();
        DlgOverlayFix(this);

        if(result == wxID_OK) {
            // Get updated label and set it in out updated feature list
            string new_label = dlg.GetUpdatedLabel();
            TBioTreeFeatureId label_id = m_DataSource->GetTree()->GetFeatureDict().GetId("label");

            CFeatureEdit* feat_edit(new CFeatureEdit());
            feat_edit->GetDictionary() = m_DataSource->GetDictionary();
            feat_edit->GetUpdated().push_back(CUpdatedFeature());
            feat_edit->GetUpdated()[0].SetNode(hover->GetId(), m_DataSource->GetTree()->GetCurrentNodeIdx());
            feat_edit->GetUpdated()[0].GetPrevFeatures() = hover->GetBioTreeFeatureList();
            feat_edit->GetUpdated()[0].GetFeatures() = hover->GetBioTreeFeatureList();
            feat_edit->GetUpdated()[0].GetFeatures().SetFeature(label_id, new_label);          

            CEvent evt(CEvent::eEvent_Message, eCmdFeaturesEdited, feat_edit, CEvent::eRelease, this);
            Send(&evt, ePool_Parent);
        }       
    }
}

void CPhyloTreeWidget::OnSubtreeFromSelection(wxCommandEvent& /* evt */)
{
    CEvent eee(CEvent::eEvent_Message, eCmdSubtreeFromSelected);
    Send(&eee, ePool_Parent);
}

void CPhyloTreeWidget::QueryStart()
{
    this->SetCursor(*wxHOURGLASS_CURSOR);        
    m_pPhyloTreePane->Disable(); 
    m_QueryPanel->SetCursor(*wxSTANDARD_CURSOR);
}
   
void CPhyloTreeWidget::QueryEnd(CMacroQueryExec* exec)
{
    this->SetCursor(*wxSTANDARD_CURSOR);
    m_pPhyloTreePane->Enable();

    /// Set the current node to the first node in the selection set
    m_DataSource->IterateOverSelNodes(0, m_QueryPanel->IsSelectAll());

    SendSelChangedEvent();

    // If query was a macro, also set undo/redo event here
    if (exec != NULL) {
        CTreeQueryExec *e = dynamic_cast<CTreeQueryExec*>(exec);

        if (e != NULL) {
            CFeatureEdit* feat_edit = e->GetFeatureEdit();
            CSelectionSetEdit* sel_edit = e->GetSelectionEdit();

            // If user changed topology (deleted node(s)), we need to clear selection and send an
            // edit event which updates the project and saves a copy of previous tree for undo/redo
            if (e->GetTopologyChange()) {
                m_DataSource->GetTree()->ClearSelection();

                // Distances can change, so that means labels could change too
                m_DataSource->Relabel(m_pScheme, m_pScheme->SetLabelFormat());

                SendEditEvent();
            }
            else if (feat_edit != NULL) {
                feat_edit->GetDictionary() = m_DataSource->GetDictionary();

                if (feat_edit->m_Updated.size() > 0) {
                    CEvent evt(CEvent::eEvent_Message, eCmdFeaturesEdited, feat_edit, CEvent::eRelease, this);
                    Send(&evt, ePool_Parent);
                }

                // If selection set was updated, update the current selection set with the value
                // it had before macros were invoked, then use the eCmdRenderingOptionsChanged to
                // perform the update/edit using undo-redo mechanism.
                sel_edit->GetUpdatedSet() = m_DataSource->GetTree()->GetSelectionSets();
                if (sel_edit->Updated()) {
                    m_DataSource->GetTree()->GetSelectionSets() = sel_edit->GetPrevSet();
                    CEvent evt2(CEvent::eEvent_Message, eCmdRenderingOptionsChanged, sel_edit, CEvent::eRelease, this);
                    Send(&evt2, ePool_Parent);
                }
            }
        }
    }
}

void CPhyloTreeWidget::IterateSelection(int dir)
{
    CPhyloTree::TTreeIdx node_idx;
    
    // Go to the next/previous node in the selection set, and return it
    node_idx = m_DataSource->IterateOverSelNodes(dir, m_QueryPanel->IsSelectAll());

    // If we have a node, check to see if it is currently on screen.  If
    // it isn't, scroll the window so that it is visible. Do not scroll it
    // to the screen center, but rather half way toward the center. That
    // gives a better impression to the user of which way the screen is
    // moving.
    if (node_idx != CPhyloTree::Null()) {
        CPhyloTree::TNodeType& node = m_DataSource->GetTree()->GetNode(node_idx);

        TModelPoint  pt((node)->X(), (node)->Y());

        // Get extent of label in ll,ur (lower-left and upper-right) if labels
        // are visible
        CVect2<float> ll(node->XY()), ur(node->XY());
        bool rotated = false;        
        bool label_visible = false;

        IPhyloTreeRender* r = m_pPhyloTreePane->GetCurrRenderer();
        if (r != NULL) {
            label_visible = r->LabelsVisible();
            rotated = r->GetRotatedLabels();
        }

        if (label_visible) {
            // Get current rotation angle for zoom level (varies from default rotation angle
            // which is node.GetAngle()
            float rotation_angle =
                m_DataSource->GetModel().GetCurrentRotationAngle(m_pPhyloTreePane->GetPane(), node, label_visible);

            if (label_visible) {
                CVect2<float> scalef(float(m_pPhyloTreePane->GetPane().GetScale().X()),
                    float(m_pPhyloTreePane->GetPane().GetScale().Y()));

                // Remember current angle 'a'
                float a = node->GetAngle();
                node->SetAngle(rotation_angle);
                node.GetValue().GetLabelRect(scalef, ll, ur, rotated);
                node->SetAngle(a);
            }
        }
        
        // label_end_pos holds the labels x and y coordinates that are most
        // distant from the node itself (so when we can scroll we can
        // try to get the entire label into viewport)
        CVect2<float> label_end_pos;
        if (std::abs(ll.X() - node->X()) > std::abs(ur.X() - node->X()))
            label_end_pos.X() = ll.X();
        else
            label_end_pos.X() = ur.X();

        if (std::abs(ll.Y() - node->Y()) > std::abs(ur.Y() - node->Y()))
            label_end_pos.Y() = ll.Y();
        else
            label_end_pos.Y() = ur.Y();

        // Get viewport
        TVPRect vp = m_pPhyloTreePane->GetPane().GetViewport();

        // Get viewport coordinates of node position and (furthest) label position
        TVPPoint ppt = m_pPhyloTreePane->GetPane().Project(pt.X(), pt.Y());
        TVPPoint label_ppt = m_pPhyloTreePane->GetPane().Project(label_end_pos.X(), label_end_pos.Y());

        // Labels could be wider or (if rotated) taller than the viewport which
        // would make it impossible to scroll the whole label into view. 
        // restrict label width/height to 70% of the viewport size for
        // scrolling purposes.     
        float label_xpct = float(label_ppt.X() - ppt.X()) / float(vp.Width());
        float label_ypct = float(label_ppt.Y() - ppt.Y()) / float(vp.Height());

        m_pPhyloTreePane->GetPane().OpenOrtho();
        if (label_xpct > 0.7f) {          
            label_xpct = 0.7f;
            label_end_pos.X() = pt.X() + label_xpct*m_pPhyloTreePane->GetPane().UnProjectWidth(vp.Width());
        }
        else if (label_xpct < -0.7f) {
            label_xpct = -0.7f;
            label_end_pos.X() = pt.X() + label_xpct*m_pPhyloTreePane->GetPane().UnProjectWidth(vp.Width());
        }

        if (label_ypct > 0.7f) {
            label_ypct = 0.7f;
            label_end_pos.Y() = pt.Y() + label_xpct*m_pPhyloTreePane->GetPane().UnProjectHeight(vp.Height());
        }
        else if (label_ypct < -0.7f) {
            label_ypct = -0.7f;
            label_end_pos.Y() = pt.Y() + label_xpct*m_pPhyloTreePane->GetPane().UnProjectHeight(vp.Height());
        }
        m_pPhyloTreePane->GetPane().Close();

        // Reset (distant) label projected position to match label_x/ypct
        label_ppt.Init(ppt.X() + int(label_xpct*vp.Width()),
                       ppt.Y() + int(label_ypct*vp.Height()));
        
        // If either the node position or the (possibly truncated) label end
        // are not within the viewport, set up to scroll them into view
        if (!m_pPhyloTreePane->GetPane().GetViewport().PtInRect(ppt) ||
            !m_pPhyloTreePane->GetPane().GetViewport().PtInRect(label_ppt)) {
            wxPoint win_pos = m_pPhyloTreePane->GetScreenPosition();

            //Get center of screen in model coords
            TModelUnit center_x = m_pPhyloTreePane->GetScreenRect().x +
                                  m_pPhyloTreePane->GetScreenRect().GetWidth()/2 - 
                                  win_pos.x;
            TModelUnit center_y = m_pPhyloTreePane->GetScreenRect().y - win_pos.y;
            center_y = m_pPhyloTreePane->GetRect().GetHeight()/2-center_y;

            TModelPoint center_model_coord = 
                m_pPhyloTreePane->GetPane().UnProject((int)center_x, (int)center_y);

            double dx = 0.0;
            double dy = 0.0;

            // The node and/or label end are outside of the viewport.
            // determine for both x and y which one is further outside
            // the viewport and set scroll amounts (in model coords) 
            // to bring it back in.
            if (ppt.X() > vp.Width() || label_ppt.X() > vp.Width()) {
                // If node or (end of) label are to the right of the viewport                
                if (ppt.X() > label_ppt.X()) {
                    double off = ppt.X() - vp.Width();

                    // In each of these, 'dx' is the amount to scroll the node
                    // or label position to the center of the viewport
                    dx = (center_model_coord.X() - pt.X());

                    // We then scale that amount to bring the node to just
                    // 25% inside the viewport (or for end of label just 10%)
                    dx *= (off + vp.Width()*0.25)/(off + vp.Width()*0.5);
                }
                else {
                    double off = label_ppt.X() - vp.Width();
                    dx = (center_model_coord.X() - label_end_pos.X());
                    dx *= (off + vp.Width()*0.10)/(off + vp.Width()*0.5);
                }                
            }
            else if (ppt.X() < 0 || label_ppt.X() < 0) {
                // If node or (end of) label are to the left of the viewport               
                if (ppt.X() < label_ppt.X()) {
                    double off = -ppt.X();
                    dx = (center_model_coord.X() - pt.X());
                    dx *= (off + vp.Width()*0.25) / (off + vp.Width()*0.5);
                }
                else {
                    double off = -label_ppt.X();
                    dx = (center_model_coord.X() - label_end_pos.X());
                    dx *= (off + vp.Width()*0.10) / (off + vp.Width()*0.5);
                }
            }

            if (ppt.Y() > vp.Height() || label_ppt.Y() > vp.Height()) {               
                if (ppt.Y() > label_ppt.Y()) {
                    double off = ppt.Y() - vp.Height();
                    dy = (center_model_coord.Y() - pt.Y());
                    dy *= (off + vp.Height()*0.25) / (off + vp.Height()*0.5);
                }
                else {
                    double off = label_ppt.Y() - vp.Height();
                    dy = (center_model_coord.Y() - label_end_pos.Y());
                    dy *= (off + vp.Height()*0.10) / (off + vp.Height()*0.5);
                }                
            }
            else if (ppt.Y() < 0 || label_ppt.Y() < 0) {                
                if (ppt.Y() < label_ppt.Y()) {
                    double off = -ppt.Y();
                    dy = (center_model_coord.Y() - pt.Y());
                    dy *= (off + vp.Height()*0.25) / (off + vp.Height()*0.5);
                }
                else {
                    double off = -label_ppt.Y();
                    dy = (center_model_coord.Y() - label_end_pos.Y());
                    dy *= (off + vp.Height()*0.10) / (off + vp.Height()*0.5);
                }                
            }    

            Scroll(-dx, -dy);
        }
    }

    SendSelChangedEvent();  
}

void CPhyloTreeWidget::SetSelectAll(bool b)
{     
    if (m_DataSource != NULL) {
        m_pScheme->SetShowAllSelected(b);
        m_DataSource->IterateOverSelNodes(0, b);
        SendSelChangedEvent();    
    }
}

void CPhyloTreeWidget::SetHideUnselected(bool b)
{
    m_pScheme->SetSelectionVisibility(b ?
                                      CPhyloTreeScheme::eHighlightSelection :
                                      CPhyloTreeScheme::eNormalSelection); 
    m_pPhyloTreePane->BufferedRender();
    m_pPhyloTreePane->Refresh();
}


END_NCBI_SCOPE
