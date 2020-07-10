#ifndef GUI_WIDGETS_PHYLO_TREE___PHYLO_TREE_WIDGET__HPP
#define GUI_WIDGETS_PHYLO_TREE___PHYLO_TREE_WIDGET__HPP

/*  $Id: phylo_tree_widget.hpp 42391 2019-02-14 18:25:12Z katargir $
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

#include <corelib/ncbiobj.hpp>

#include <gui/widgets/phylo_tree/phylo_tree_ds.hpp>
#include <gui/widgets/phylo_tree/phylo_tree_pane.hpp>

#include <gui/widgets/gl/gl_widget_base.hpp>

#include <gui/utils/command.hpp>
#include <gui/utils/menu_item.hpp>

#include <gui/objutils/query_widget.hpp>
#include <gui/widgets/data/query_parse_panel.hpp>

#include <gui/opengl/gldlist.hpp>
#include <gui/opengl/gltypes.hpp>

#include <objects/seqloc/Seq_loc.hpp>

#include <wx/tglbtn.h>
#include <wx/statline.h>
#include <wx/panel.h>
#include <wx/stattext.h>
#include <wx/statline.h>
#include <wx/statbox.h>
#include <wx/combobox.h>
#include <wx/sizer.h>
#include <wx/button.h>
#include <wx/srchctrl.h>
#include <wx/checkbox.h>
#include <wx/animate.h>
#include <wx/toolbar.h>


class wxFileArtProvider;

BEGIN_NCBI_SCOPE

class CUICommandRegistry;
class CGuiRegistry;
class CMacroQueryExec;


/// Definitions for Phylo Tree Widget commands
enum    EPhyloTreeCommands {
    eCmdChooseSeq = eBaseCmdLast + 555,
    eCmdSetGraphType1,
    eCmdSetGraphType2,
    eCmdSetGraphType3,
    eCmdSetGraphType4,
    eCmdSetGraphType5,
    eCmdUseDistances,
    eCmdUseSplines,
    eCmdRotateLabels,
    eCmdStopLayout,
    eCmdMySettings,
    eCmdRerootTree,
    eCmdRerootTreeAtEdge,
    eCmdMidpointRoot,
    eSelectAll,
    eClearSelection,
    eCmdAddSelectionSet,
    eCmdExportSelection,
    eCmdExportTree,
    eCmdResetNodeSelection,
    eCmdCollapseChildren,
    eCmdExpandChildren,
    eCmdHighlightEdges,
    eCmdCollapseToViewport,
    eCmdCollapseSelected,
    eCmdZoomToSubtree,
    eCmdRelayout,
    eCmdRestoreView,
    eCmdShowAll,
    eCmdEditNode,
    eCmdFilterDistances,
    eCmdLoadAttributes,

    eCmdLabelsForLeavesOnly,
    eCmdLabelsVisible,
    eCmdLabelsHidden,

    eCmdSetTreeLabel,

    eCmdAlNone,
    eCmdAlPhylip,
    eCmdAlPaml,

    eCmdNodeCut,
    eCmdNodePaste,
    eCmdMoveUp,
    eCmdMoveDown,
    eCmdNodeNewChild,
    eCmdNodeNewParent,
    eCmdRemoveSubtree,
    eCmdRemoveSelected,
    eCmdRemoveNode,

    eCmdExportSelected,
	eCmdExportSelectedNew,
    eCmdSubtreeFromSelected,
    eCmdZoomXY,
    eCmdZoomX,
    eCmdZoomY,
    eCmdSort,
    eCmdSortDist,
    eCmdSortLabel,
    eCmdSortLabelRange,
    eCmdSortAscending,
    eCmdClean,
    eCmdFilter,
    eCmdEditLabel
};

////////////////////////////////////////////////////////////////////////////////
/// class CPhyloTreeWidget
class NCBI_GUIWIDGETS_PHYLO_TREE_EXPORT CPhyloTreeWidget 
    : public CGlWidgetBase
    , public IQueryWidget
{
    DECLARE_EVENT_TABLE()
public:

    static  void RegisterCommands(CUICommandRegistry& cmd_reg, wxFileArtProvider& provider);

    CPhyloTreeWidget(wxWindow* parent,
                     wxWindowID id = wxID_ANY,
                     const wxPoint& pos = wxDefaultPosition,
                     const wxSize& size = wxDefaultSize,
                     long style = wxTAB_TRAVERSAL,
                     const wxString& name = wxT("panel"));
        
    virtual ~CPhyloTreeWidget();

    virtual void SetDataSourceNoUpdate(CPhyloTreeDataSource* p_ds);    
    virtual void SetDataSource(CPhyloTreeDataSource* p_ds);
    virtual void RemoveCurrentDataSource();
    /// Force tree to be re-masured and call layout 
    virtual void RedrawDataSource();

    virtual CPhyloTreeDataSource*  GetDS(void);
    virtual CGlPane&    GetPort();
    virtual const CGlPane&    GetPort() const;
    virtual void    OnSetScaleXY(TModelUnit scale_x, const TModelPoint& point);

    /// this will forward events to fix opengl bug (windows) to pane
    virtual void DlgOverlayFix(wxWindow* win);
    
    /// Command handlers

    void    OnContextMenu(wxContextMenuEvent&);

    void    OnPrint(wxCommandEvent & evt);
    void    OnEnablePrintCmdUpdate(wxUpdateUIEvent& evt);
    void    OnSaveImages(wxCommandEvent & evt);
    void    OnSetEqualScale(wxCommandEvent & evt);
    void    OnEnableSaveImagesCmdUpdate(wxUpdateUIEvent& evt);
    void    OnSavePdf(wxCommandEvent & evt);
    void    OnEnableSavePdfCmdUpdate(wxUpdateUIEvent& evt);
    void    OnSetGraphType1(wxCommandEvent & evt);
    void    OnSetGraphType2(wxCommandEvent & evt);
    void    OnSetGraphType3(wxCommandEvent & evt);
    void    OnSetGraphType4(wxCommandEvent & evt);
    void    OnSetGraphType5(wxCommandEvent & evt);
    void    OnRotateLabels(wxCommandEvent & evt);
    void    OnStopLayout(wxCommandEvent & evt);
    void    OnUseDistances(wxCommandEvent & evt);
    void    OnUseSplines(wxCommandEvent & evt);
    void    OnOpenPropertiesDlg(wxCommandEvent & evt);
    void    OnRerootTree(wxCommandEvent & evt);
    void    OnRerootTreeAtEdge(wxCommandEvent & evt);
    void    OnSetMidpointRoot(wxCommandEvent & evt);
    void    OnSelectAll(wxCommandEvent & evt);
    void    OnClearSelection(wxCommandEvent & evt);
    void    OnAddSelectionSet(wxCommandEvent & evt);
    void    OnExportSelection(wxCommandEvent & evt);
    void    OnExportTree(wxCommandEvent & evt);
    void    OnInverseSelection(wxCommandEvent & evt);
    void    OnExpandChildren(wxCommandEvent & evt);
    void    OnHighlightCollapsed(wxCommandEvent & evt);
    void    OnHighlightEdges(wxCommandEvent & evt);
    void    OnCollapseToViewport(wxCommandEvent & evt);
    void    OnCollapseSelected(wxCommandEvent & evt);
    void    OnCollapseChildren(wxCommandEvent & evt);
    void    OnZoomToSubtree(wxCommandEvent & evt);
    void    OnShowAll(wxCommandEvent & evt);
    void    OnEditNode(wxCommandEvent & evt);
    void    OnFilterDistances(wxCommandEvent & evt);
    void    OnZoomXY(wxCommandEvent & evt);
    void    OnZoomX(wxCommandEvent & evt);
    void    OnZoomY(wxCommandEvent & evt);
    void    OnZoomToSelection(wxCommandEvent & evt);
    void    OnGoBack(wxCommandEvent& evt);
    void    OnGoForward(wxCommandEvent& evt);
    void    OnUpdateGoBack(wxUpdateUIEvent& evt);
    void    OnUpdateGoForward(wxUpdateUIEvent& evt);


    void    OnSort(wxCommandEvent & evt);
    void    OnSortAscending(wxCommandEvent & evt);
    void    OnClean(wxCommandEvent & evt);
    void    OnFilter(wxCommandEvent & evt);
    void    OnEditLabel(wxCommandEvent & evt);
    void    OnLoadAttributes(wxCommandEvent & evt);

    void    OnLabels(wxCommandEvent & evt);
    void    OnUpdateLabels(wxUpdateUIEvent& evt);

    void    OnUpdateEditLabel(wxUpdateUIEvent& evt);

    void    OnSetTreeLabel(wxCommandEvent & evt);

    void    OnAl(wxCommandEvent & evt);
    void    OnUpdateAl(wxUpdateUIEvent& evt);

    void    OnEdit(wxCommandEvent & evt);
    void    OnUpdateEdit(wxUpdateUIEvent& evt);     


    // modded zoom handlers
    virtual void    x_ZoomIn(int options);
    virtual void    x_ZoomOut(int options);

    // update handlers
    void    OnUpdateZoomXY(wxUpdateUIEvent& evt);
    void    OnUpdateZoomX(wxUpdateUIEvent& evt);
    void    OnUpdateZoomY(wxUpdateUIEvent& evt);
    void    OnUpdateZoomSel(wxUpdateUIEvent& evt);

    void    OnUpdateEditNode(wxUpdateUIEvent& evt);
    void    OnUpdateRotateLabels(wxUpdateUIEvent& evt);
    void    OnUpdateStopLayout(wxUpdateUIEvent& evt);
    void    OnUpdateUseDistances(wxUpdateUIEvent& evt);
    void    OnUpdateSortAscending(wxUpdateUIEvent& evt);
    void    OnUpdateUseSplines(wxUpdateUIEvent& evt);
    void    OnUpdateSetGraphType1(wxUpdateUIEvent& evt);
    void    OnUpdateSetGraphType2(wxUpdateUIEvent& evt);
    void    OnUpdateSetGraphType3(wxUpdateUIEvent& evt);
    void    OnUpdateSetGraphType4(wxUpdateUIEvent& evt);
    void    OnUpdateSetGraphType5(wxUpdateUIEvent& evt);
    void    OnUpdateRerootTree(wxUpdateUIEvent& evt);
    void    OnUpdateRerootTreeAtEdge(wxUpdateUIEvent& evt);
    void    OnUpdateSetMidpointRoot(wxUpdateUIEvent& evt);
    void    OnUpdateShowAll(wxUpdateUIEvent& evt);

    void    OnUpdateCollapseChildren(wxUpdateUIEvent& evt);
    void    OnUpdateExpandChildren(wxUpdateUIEvent& evt);
    void    OnUpdateCollapseSelected(wxUpdateUIEvent& evt);

    void    OnUpdateZoomToSubtree(wxUpdateUIEvent& evt);

    void    OnSubtreeFromSelection(wxCommandEvent & evt);

    void    OnSearchTip(wxCommandEvent & evt);
    void    OnZoomTip(wxCommandEvent & evt);
    void    OnInfoTip(wxCommandEvent & evt);
    void    OnTipActivated(wxCommandEvent & evt);
    void    OnTipDeactivated(wxCommandEvent & evt);
    void    OnUpdateSubtreeFromSelection(wxUpdateUIEvent& evt);

    void    OnUpdateSelChanged();

    void    UpdateViewingArea();

    void                SetCurrRenderer(int idx);
    int                 GetCurrRenderer() const;
    vector<string>      GetRenderersNames(void);

    virtual void    OnShowPopup() {}

    void    SendSelChangedEvent();
    void    SendEditEvent(EPhyloTreeEditCommand ec=eCmdSomethingEdited);
    void    SetPortLimits(const TModelRect & rect, bool bZoomAll=true);
    void    SetPopupMenuItems(CMenuItem * itm);
    void    SetScheme(CPhyloTreeScheme & sl);
    void    SetRegistryPath(const string& reg_path);
    void    LoadSettings();
    void    SaveSettings() const;
    void    SetUseDistances(bool bDist);
    bool    GetUseDistances() const;
    void    SetRotateLabels(bool rot);
    bool    GetRotateLabels() const;
    void    EditLabel();

    bool HasScheme(void)     {return !m_pScheme.Empty(); }
    CPhyloTreeScheme & GetScheme(void)     {return m_pScheme.GetObject();     }

    /// @name IQueryWidget interface implementation
    /// @{
    virtual string GetSearchHelpAddr() const
    { return string("www.ncbi.nlm.nih.gov/tools/gbench/tutorial3/#Tree_View_Search"); }

    /// Disable any widgets that the user should not use during the query
    virtual void QueryStart();
    /// Re-enable any widgets disabled during the query.
    virtual void QueryEnd(CMacroQueryExec* exec);

    /// Advance to previous/next selected row from query (in current sort order)
    void IterateSelection(int dir);
    /// Set to true to show all rows selected by most recent query as selected
    virtual void SetSelectAll(bool b);  

    /// If true, only rows that were selected by prevous query will be shown
    virtual void SetHideUnselected(bool b);  
    /// @}

    void SetQueryPanel(CQueryParsePanel* queryPanel);

protected:
    // CGlWidgetBase overridables
    virtual void x_CreatePane(void);
    virtual CGlWidgetPane* x_GetPane();

    virtual void    x_SetPortLimits(void);

    virtual void    x_Update();
    virtual void    x_UpdatePane();
    virtual void    x_SoftUpdate();

    virtual void    x_SaveStates();

    virtual void    x_CreateControls();

    void            x_IterateSelection(int dir);

protected:
    CRef<CPhyloTreeDataSource> m_DataSource;
    CRef<CPhyloTreeScheme>     m_pScheme;
    CPhyloTreePane*            m_pPhyloTreePane;
    CMenuItem*                 m_pPopupItems;
    CRef<CGuiRegistry>         m_ConfigCache;
    CGlPane                    m_Port;    
    CQueryParsePanel*          m_QueryPanel;
    bool                       m_SortAscending;

    // This is to allows us to recenter the view after expand/collapse
    EPhyloTreeEditCommand      m_LastCommand;
    CGlPane                    m_PrevPane;  // projection pane before expand/collapse
    CVect2<float>              m_PrevPos; // position of node before expand/collapse
};


END_NCBI_SCOPE

#endif  // GUI_WIDGETS_PHYLO_TREE___PHYLO_TREE_WIDGET__HPP
