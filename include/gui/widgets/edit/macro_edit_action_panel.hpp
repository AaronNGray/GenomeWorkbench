#ifndef GUI_WIDGETS_EDIT___MACRO_EDIT_ACTION_PANEL__HPP
#define GUI_WIDGETS_EDIT___MACRO_EDIT_ACTION_PANEL__HPP
/*  $Id: macro_edit_action_panel.hpp 44390 2019-12-10 16:13:36Z asztalos $
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
 *  and reliability of the software and data,  the NLM and the U.S.
 *  Government do not and cannot warrant the performance or results that
 *  may be obtained by using this software or data. The NLM and the U.S.
 *  Government disclaim all warranties,  express or implied,  including
 *  warranties of performance,  merchantability or fitness for any particular
 *  purpose.
 *
 *  Please cite the author in any work or product based on this material.
 *
 * ===========================================================================
 *
 * Authors:  Andrea Asztalos
 */

#include <corelib/ncbistd.hpp>
#include <gui/widgets/edit/macro_treeitemdata.hpp>

#include <wx/srchctrl.h>
#include <wx/treectrl.h>
#include <wx/treebase.h>
#include <wx/menu.h>
#include <wx/sizer.h>
#include <wx/timer.h>
#include <wx/hyperlink.h>

BEGIN_NCBI_SCOPE

class CMacroCompoundConstraintPanel;
class CMacroEditor;
class CwxTreeCtrl;

/*!
 * Control identifiers
 */

////@begin control identifiers
#define SYMBOL_CMACROEDITINGACTIONPANEL_STYLE wxTAB_TRAVERSAL
#define SYMBOL_CMACROEDITINGACTIONPANEL_TITLE _("Macro Editing Action Panel")
#define SYMBOL_CMACROEDITINGACTIONPANEL_IDNAME ID_CMACROEDITINGACTIONPANEL
#define SYMBOL_CMACROEDITINGACTIONPANEL_SIZE wxDefaultSize
#define SYMBOL_CMACROEDITINGACTIONPANEL_POSITION wxDefaultPosition
////@end control identifiers


/*!
 * CMacroEditingActionPanel class declaration
 */

class CMacroEditingActionPanel : public wxPanel
{    
    DECLARE_DYNAMIC_CLASS( CMacroEditingActionPanel )
    DECLARE_EVENT_TABLE()

public:
    /// Constructors
    CMacroEditingActionPanel();
    CMacroEditingActionPanel( wxWindow* parent, 
        wxWindowID id = SYMBOL_CMACROEDITINGACTIONPANEL_IDNAME, 
        const wxPoint& pos = SYMBOL_CMACROEDITINGACTIONPANEL_POSITION, 
        const wxSize& size = SYMBOL_CMACROEDITINGACTIONPANEL_SIZE, 
        long style = SYMBOL_CMACROEDITINGACTIONPANEL_STYLE );

    /// Creation
    bool Create( wxWindow* parent, 
        wxWindowID id = SYMBOL_CMACROEDITINGACTIONPANEL_IDNAME, 
        const wxPoint& pos = SYMBOL_CMACROEDITINGACTIONPANEL_POSITION, 
        const wxSize& size = SYMBOL_CMACROEDITINGACTIONPANEL_SIZE, 
        long style = SYMBOL_CMACROEDITINGACTIONPANEL_STYLE );

    /// Destructor
    ~CMacroEditingActionPanel();

    /// Initialises member variables
    void Init();

    /// Creates the controls and sizers
    void CreateControls();

////@begin CMacroEditingActionPanel event handler declarations

    /// wxEVT_TREE_SEL_CHANGED  event handler for ID_MACROEDIT_TREECTRL
    void OnActionTreeItemChanged( wxTreeEvent& event );

    void OnControlInSubPanelChanged(wxCommandEvent& event);
    void OnControlClick(wxCommandEvent& event);
    void OnHyperlinkClicked(wxHyperlinkEvent& event);

    void OnSearchCtrlClick(wxCommandEvent& event);
    void OnSearchCtrlEnter(wxCommandEvent& event);

////@end CMacroEditingActionPanel event handler declarations

////@begin CMacroEditingActionPanel member function declarations

    /// Retrieves bitmap resources
    wxBitmap GetBitmapResource( const wxString& name );

    /// Retrieves icon resources
    wxIcon GetIconResource( const wxString& name );
////@end CMacroEditingActionPanel member function declarations

    /// Should we show tooltips?
    static bool ShowToolTips();

    enum {
        ID_CMACROEDITINGACTIONPANEL = 10320,
        ID_MACROEDIT_SEARCHCTRL = 10321,
        ID_MACROEDIT_TREECTRL = 10322,
        ID_MACROEDIT_CONSTRAINTPNL = 10323
    };

    string GetMacro();
    void SetParentFrame(CMacroEditor* parent) { m_ParentFrame = parent; }
    // returns pair<FOR_EACH_SELECTOR, selected_field_in_panel>
    pair<string, string> GetForTarget();
    void UpdateParentFrame();
    const string& GetWarning() const;
private:
    void x_PopulateTreeCtrl();
    void x_Build(const CMActionNode& root, wxTreeItemId root_id, const CMTreeItemDataBuilder& builder);

    void x_FindText(const string& search);
    void x_FindItem(const string& search, const wxTreeItemId& root);
    void x_FocusFoundItem();

////@begin CMacroEditingActionPanel member variables
    wxSearchCtrl* m_SearchCtrl;
    wxTreeCtrl* m_ActionTree;
    wxStaticBoxSizer* m_ActionSizer;
    wxStaticBoxSizer* m_ConstraintSizer;
    CMacroCompoundConstraintPanel* m_ConstraintPanel;
    IMacroActionItemData* m_SelectedItem;
    
    wxTreeItemId m_TopId;
    // for searching
    string m_RecentSearch;
    vector<wxTreeItemId> m_RecentItemIds;
    vector<wxTreeItemId>::const_iterator m_RecentIt;

    CMacroEditor* m_ParentFrame;
};

class CwxTreeCtrl : public wxTreeCtrl
{
    DECLARE_EVENT_TABLE()
    DECLARE_DYNAMIC_CLASS(CwxTreeCtrl)
    DECLARE_NO_COPY_CLASS(CwxTreeCtrl)
public:
    CwxTreeCtrl() : wxTreeCtrl() {}
    CwxTreeCtrl(wxWindow *parent,
        wxWindowID id = wxID_ANY,
        const wxPoint& pos = wxDefaultPosition,
        const wxSize& size = wxDefaultSize,
        long style = wxTR_DEFAULT_STYLE,
        const wxValidator &validator = wxDefaultValidator,
        const wxString& name = wxTreeCtrlNameStr)
        : wxTreeCtrl(parent, id, pos, size, style, validator, name) {}

    virtual ~CwxTreeCtrl();

    void OnMouseMove(wxMouseEvent& event);
    void OnTimer(wxTimerEvent& event);
    void OnLeftDown(wxMouseEvent& event);

    void OnContextMenu(wxContextMenuEvent& event);

    void OnExpandAllMenuItem(wxCommandEvent& event);
    void OnCollapseAllMenuItem(wxCommandEvent& event);

    enum {
        ID_MACROEDIT_POPUPEXPAND = 10324,
        ID_MACROEDIT_POPUPCOLLAPSE = 10325
    };
private:
    void x_ShowTooltip(bool show);

    wxTimer m_Timer{ this };
    wxString m_ToolTipText{ wxEmptyString };
    wxTreeItemId m_SelItem;
};

END_NCBI_SCOPE

#endif
    // GUI_WIDGETS_EDIT___MACRO_EDIT_ACTION_PANEL__HPP
