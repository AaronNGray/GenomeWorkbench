#ifndef GUI_CORE___PROJECT_TREE_PANEL__HPP
#define GUI_CORE___PROJECT_TREE_PANEL__HPP

/*  $Id: project_tree_panel.hpp 41875 2018-10-31 17:19:44Z filippov $
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
 *      Project Tree Panel class.
 */

#include <corelib/ncbistd.hpp>

#include <gui/core/pt_item.hpp>
#include <gui/core/app_explorer_data_object.hpp>

#include <gui/objects/GBWorkspace.hpp>

#include <gui/utils/event_handler.hpp>

#include <wx/panel.h>
#include <wx/treectrl.h>
#include <wx/dnd.h>

BEGIN_NCBI_SCOPE

class CAppExplorerDataObject;
class IWorkbench;
class CGBDocument;
class IProjectView;

namespace PT
{
class CRoot;
class CWorkspace;
}

//////////////////////////////////////////////////////////////////////////////
/// CProjectTreeCtrl - subclass of wxTreeCtrl to allow functions to be
///                    overridden for application-specific behavior.

class CProjectTreeCtrl: public wxTreeCtrl
{
    DECLARE_EVENT_TABLE()
public:
    CProjectTreeCtrl() : wxTreeCtrl() {}
    CProjectTreeCtrl(wxWindow *parent, 
                     wxWindowID id = wxID_ANY,
                     const wxPoint& pos = wxDefaultPosition,
                     const wxSize& size = wxDefaultSize,
                     long style = wxTR_DEFAULT_STYLE,
                     const wxValidator &validator = wxDefaultValidator,
                     const wxString& name = wxTreeCtrlNameStr) 
    : wxTreeCtrl(parent, id, pos, size, style, validator, name) {}

    virtual ~CProjectTreeCtrl() {};

    virtual wxTextCtrl *EditLabel(const wxTreeItemId& item,
                                  wxClassInfo* textCtrlClass = CLASSINFO(wxTextCtrl));

    void    OnLeftDown(wxMouseEvent& event);
    void    OnKeyUp(wxKeyEvent& event);
    void    OnMove(wxMouseEvent& event);
};

///////////////////////////////////////////////////////////////////////////////
/// CProjectTreePanel - a window that represents Project View.

class NCBI_GUICORE_EXPORT CProjectTreePanel :
    public wxPanel,
    public CEventHandler
{
    DECLARE_EVENT_TABLE()

public:
    friend class CProjectTreeViewDropTarget;
    friend class CProjectTreeView;

    typedef int TProjectId;
    typedef vector<TProjectId> TProjectIdVector;

public:
    CProjectTreePanel();
    virtual ~CProjectTreePanel();

    /// Initializes member variables
    void    Init();

    /// Creates the controls and sizers
    void    CreateControls();

    void    Create(wxWindow* parent,
                   wxWindowID id = wxID_ANY,
                   const wxPoint& pos = wxDefaultPosition,
                   const wxSize& size = wxDefaultSize);

    void    SetWorkbench(IWorkbench* workbench);

    void    GetSelectedItems(PT::TItems& items);
    void    GetAllItems(PT::TItems& items);
    void    GetSelectedProjectIds(TProjectIdVector& ids);

    /// @name Event handlers
    /// @{
    void    OnContextMenu(wxContextMenuEvent& event);

    void    OnItemExpandedCollapsed(wxTreeEvent& event);
    void    OnItemActivated(wxTreeEvent& event);
    void    OnSelectionChanged(wxTreeEvent& event);
    void    OnBeginDrag(wxTreeEvent& event);
    void    OnEndDrag(wxTreeEvent& event);
    void    OnTreeKeyDown(wxTreeEvent& event);
    void    OnBeginLabelEdit(wxTreeEvent& event);
    void    OnEndLabelEdit(wxTreeEvent& event);

    void    OnLeftDown(wxMouseEvent& event);

    void    OnCut(wxCommandEvent& event);
    void    OnUpdateCut(wxUpdateUIEvent& event);
    void    OnCopy(wxCommandEvent& event);
    void    OnUpdateCopy(wxUpdateUIEvent& event);
    void    OnPaste(wxCommandEvent& event);
    void    OnUpdatePaste(wxUpdateUIEvent& event);

    void    OnRemove(wxCommandEvent& event);
    void    OnUpdateRemove(wxUpdateUIEvent& event);

    void    OnShowHideDisabledItems(wxCommandEvent& event);
    void    OnUpdateShowHideDisabledItems(wxUpdateUIEvent& event);

    void    OnEnableDisable(wxCommandEvent& event);
    void    OnUpdateEnableDisable(wxUpdateUIEvent& event);
    void    OnNewFolder(wxCommandEvent& event);
    void    OnUpdateNewFolder(wxUpdateUIEvent& event);

    void    OnCloseView(wxCommandEvent& event);
    void    OnUpdateCloseView(wxUpdateUIEvent& event);
    void    OnActivateClients(wxCommandEvent& event);
    void    OnUpdateActivateClients(wxUpdateUIEvent& event);
    void    OnProperties(wxCommandEvent& event);
    void    OnUpdateProperties(wxUpdateUIEvent& event);

    void    OnUpdateUnLoadProject(wxUpdateUIEvent& event);
    void    OnUnLoadProject(wxCommandEvent& event);

    void    OnLoadProject(wxCommandEvent& event);
    void    OnUpdateLoadProject(wxUpdateUIEvent& event);

    void    OnRemoveProject(wxCommandEvent& event);
    void    OnUpdateRemoveProject(wxUpdateUIEvent& event);

    void    OnProjectTableView(wxCommandEvent& event);
    void    OnUpdateProjectTableView(wxUpdateUIEvent& event);
    /// @}

    void    ProjectStateChanged(CGBDocument& doc);
    void    ProjectViewsChanged(CGBDocument& doc);
    void    ProjectAdded(CGBDocument& doc);
    void    ProjectUpdateItems(CGBDocument& doc);
    void    ProjectUpdateLabel(CGBDocument& doc);
    void    UpdateViewLabel(CGBDocument& doc, IProjectView& view);
    void    ProjectRemoved(size_t id);
    void    UpdateDisabledItems();
    void    UpdateWorkspaceLabel();

    /// @name Command handlers
    /// @{
    /// @}

    /// @name D&D support - interface for Drop Target
    /// @{
    wxDragResult OnDragOver(wxCoord x, wxCoord y, wxDragResult def);
    wxDragResult OnDrop(wxCoord x, wxCoord y, wxDragResult def, CAppExplorerDataObject& data);
    /// @}

    /// Drag and drop support for OSX Cocoa (generic dnd path not working in OSX cocoa 2.9.3
    /// so we use dnd specific to the tree in this case). 
    void SetCopyMode(bool b) { m_CopyMode = b; }

    void    ReloadProjectTree();

protected:
    void    x_SetSelections(wxArrayTreeItemIds& ids);
    void    x_GetItemsFromIds(const wxArrayTreeItemIds& ids, PT::TItems& items);
    void    x_CollectItemIds( wxTreeItemId root, wxArrayTreeItemIds &ids );

    PT::CItem*  x_GetExplorerItem(const wxTreeItemId& id);
    PT::CItem*  x_GetSingleSelectedItem();

    bool    x_AllViewsItemSelected(PT::TItems& items);
    bool    x_AtLeastOneProjectLoaded();

    void    x_LoadUnloadProject(bool load);

    void    x_RefreshProjectTables(int projectId);

    CAppExplorerDataObject* x_CreateDataObject(bool cut);
    void    x_CutOrCopyToClipboard(bool cut);
    void    x_ClearClipboardData();

    IExplorerItemCmdContributor::TContribution x_GetContextMenu(PT::TItems& items);

    PT::CRoot*      x_GetRootItem();
    PT::CWorkspace* x_GetWorkspaceItem();

protected:
    /// Data object used for drag and drop (this is the data being dragged)
    CAppExplorerDataObject*  m_DataObject;
    string m_RegPath;

    IWorkbench*     m_Workbench;
    CRef<objects::CGBWorkspace>  m_WS;

    CProjectTreeCtrl* m_Tree;

    vector<wxEvtHandler*> m_ContributedHandlers;

    /// D&D state
    wxTreeItemId    m_DropItemId; // current D&D target
    CStopWatch      m_DropItemTimer;
    bool    m_DropTimerActive;

    // D&D copy (vs move) state for D&D implemented with tree-functions (osx cocoa)
    bool m_CopyMode; 
};


///////////////////////////////////////////////////////////////////////////////
/// CProjectTreeViewDropTarget - a drop target associated with CProjectTreeView.
class CProjectTreeViewDropTarget : public wxDropTarget
{
public:
    CProjectTreeViewDropTarget(CProjectTreePanel* panel);

    /// @name wxDropTarget virtual functions
    /// @{
    virtual wxDragResult OnEnter(wxCoord x, wxCoord y, wxDragResult def);
    virtual wxDragResult OnDragOver(wxCoord x, wxCoord y, wxDragResult def);
    virtual wxDragResult OnData(wxCoord x, wxCoord y, wxDragResult def);
    /// @}

protected:
    CProjectTreePanel*  m_Panel;
    bool    m_AcceptableData;
};


END_NCBI_SCOPE;

#endif // GUI_CORE___PROJECT_TREE_PANEL__HPP
