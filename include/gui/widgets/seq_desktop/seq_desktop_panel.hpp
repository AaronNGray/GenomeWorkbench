#ifndef GUI_WIDGETS_SEQ_DESKTOP___SEQ_DESKTOP_PANEL__HPP
#define GUI_WIDGETS_SEQ_DESKTOP___SEQ_DESKTOP_PANEL__HPP

/*  $Id: seq_desktop_panel.hpp 37865 2017-02-23 21:52:38Z asztalos $
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
 * Authors:  Roman Katargin, Andrea Asztalos
 *
 * File Description:
 *
 */

#include <corelib/ncbiobj.hpp>
#include <gui/gui_export.h>
#include <gui/utils/event_handler.hpp>
#include <gui/widgets/wx/gui_widget.hpp>
#include <gui/widgets/wx/ibioseq_editor.hpp>
#include <gui/widgets/seq_desktop/desktop_view_context.hpp>

#include <wx/panel.h>
#include <wx/aui/aui.h>

class wxSearchCtrl;

BEGIN_NCBI_SCOPE

class CDesktopCanvas;

////@begin control identifiers
#define SYMBOL_CSEQDESKTOPPANEL_STYLE wxTAB_TRAVERSAL
#define SYMBOL_CSEQDESKTOPPANEL_IDNAME ID_CSEQDESKTOPPANEL
#define SYMBOL_CSEQDESKTOPPANEL_SIZE wxSize(400, 300)
#define SYMBOL_CSEQDESKTOPPANEL_POSITION wxDefaultPosition
////@end control identifiers

class NCBI_GUIWIDGETS_DESKTOP_EXPORT CSeqDesktopPanel : public wxPanel, public IGuiWidget, public CEventHandler
{
    DECLARE_DYNAMIC_CLASS(CSeqDesktopPanel)
    DECLARE_EVENT_TABLE()

public:
    /// Constructors
    CSeqDesktopPanel();
    CSeqDesktopPanel(wxWindow* parent, wxWindowID id = SYMBOL_CSEQDESKTOPPANEL_IDNAME,
        const wxPoint& pos = SYMBOL_CSEQDESKTOPPANEL_POSITION,
        const wxSize& size = SYMBOL_CSEQDESKTOPPANEL_SIZE,
        long style = SYMBOL_CSEQDESKTOPPANEL_STYLE);

    /// Destructor
    virtual ~CSeqDesktopPanel();

    /// Initialises member variables
    void Init();

    /// Creates the controls and sizers
    void CreateControls();


    /// @name IGuiWidget implementation
    /// @{
    virtual void SetHost(IGuiWidgetHost* host);
    virtual bool InitWidget(TConstScopedObjects& objects);
    virtual const CObject* GetOrigObject() const;

    // Selection
    virtual void GetSelectedObjects(TConstObjects& objects) const;
    virtual void SetSelectedObjects(const TConstObjects& objects);

    // Data
    virtual void SetUndoManager(ICommandProccessor* cmdProccessor);
    virtual void DataChanging();
    virtual void DataChanged();

    // Objects for action
    virtual void GetActiveObjects(vector<TConstScopedObjects>& objects) {}
    /// @}

    void OnZoomInClick(wxCommandEvent& event);
    void OnZoomOutClick(wxCommandEvent& event);
    void UpdateZoomIn(wxUpdateUIEvent& event);
    void UpdateZoomOut(wxUpdateUIEvent& event);

    void OnSearchCtrlClick(wxCommandEvent& event);
    //void OnSearchCtrlEnter(wxCommandEvent& event);

    // a hack, the function above should be used, 
    // but it does not work
    void OnSearchCtrlEnter(wxKeyEvent& event);  

    // handlers for propagated/sent events
    void OnCutSelection(wxCommandEvent& event);
    void OnCopySelection(wxCommandEvent& event);
    void OnMouseClick(wxMouseEvent& event);

    void GetMainObjects(TConstScopedObjects& objs) const;

    CDesktopCanvas* GetWidget() const { return m_TextWindow; }
    /// Calls the builder (CDesktopDataBuilder) that creates all desktop items.
    /// Passes the root item to the canvas and sets up the context
    ///@param[in] state
    ///  Corresponds to the expansion level
    void Load(int state = 0);

private:
    /// Sets m_MaxExpansion for the main object. The minimum expansion level is 2,
    /// and the maximum expansion level is 4 (when features are also present).
    void x_SetMaxExpansionLevel(void);
    CDesktopViewContext* x_CreateDesktopViewContext();
    /// Creates the IBioseqEditor event handler. This is responsible for the Edit & Delete context menu items.
    CIRef<IBioseqEditor> x_CreateEditor();
    void x_CreateToolbar();
    /// Obtains the saved object and its type from the context
    /// for a possible, future Paste operation.
    void x_SaveSelection();
    void x_FindText(const string& search);

    wxAuiManager    m_AuiManager;
    wxAuiToolBar*   m_Toolbar;
    IGuiWidgetHost* m_WidgetHost;

    wxSearchCtrl* m_SearchCtrl;
    /// The canvas where all drawing takes place
    CDesktopCanvas* m_TextWindow;

    enum {
        ID_CSEQDESKTOPPANEL = 10550,
        ID_WIDGET,
        ID_SEARCHCTRL,

        ID_TB_ZOOMIN,
        ID_TB_ZOOMOUT
    };

    ICommandProccessor* m_CmdProccessor;

    /// Main object to be displayed
    CConstRef<CSerialObject> m_SO;
    /// Scope of the main object
    CRef<objects::CScope> m_Scope;
    /// Maximum level of expansion to be shown for the main object.
    /// It is set in x_SetMaxExpansionLevel();
    int m_MaxExpansion;
    // Members used to save the coordinates of the starting point of the view during changing data event
    int m_SavedStartX, m_SavedStartY;
    // Store expansion level during changing data event
    int m_SavedExpansion;

    // Members used to store information about the saved object and its type.
    // Used for cut/copy/paste operations.
    CConstRef<CSerialObject> m_SavedObject;
    CDesktopViewContext::ESavedObjectType m_SavedType;
    
};

END_NCBI_SCOPE

#endif  
    // GUI_WIDGETS_SEQ_DESKTOP___SEQ_DESKTOP_PANEL__HPP
