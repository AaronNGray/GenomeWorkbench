/*  $Id:  macrofloweditor.h 38070 06/07/2017 15:12:46 17:48:35Z Igor Filippov$
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
 * Authors:  Igor Filippov
 */
#ifndef _MACROFLOWEDITOR_H_
#define _MACROFLOWEDITOR_H_

#include <corelib/ncbistd.hpp>
#include <gui/objutils/macro_engine.hpp>
#include <gui/gui_export.h>
#include <objects/submit/Seq_submit.hpp>
#include <gui/widgets/edit/gui_core_helper.hpp>

/*!
 * Includes
 */


////@begin includes
#include "wx/frame.h"
#include "wx/toolbar.h"
#include "wx/filepicker.h"
#include "wx/treectrl.h"
#include <wx/aui/auibook.h>
#include <wx/gauge.h>
#include <wx/checkbox.h>
#include <wx/collpane.h>
////@end includes

/*!
 * Forward declarations
 */

////@begin forward declarations
class wxFilePickerCtrl;
class wxDirPickerCtrl;
class wxTreeCtrl;
class wxAuiNotebook;
////@end forward declarations

BEGIN_NCBI_SCOPE

class CScriptPanel;
class CMacroEditor;

/*!
 * Control identifiers
 */

////@begin control identifiers
#define ID_CMACROFLOWEDITOR 15000
#define ID_TEXTCTRL 10000
#define ID_TOOLBAR 10002
#define ID_IMPORT_MENU 10005
#define ID_TREECTRL 10006
#define ID_NOTEBOOK 10007
#define ID_FILECTRL 10008
#define ID_DIRPICKERCTRL 10009
#define ID_EXPORT_MENU 10010
#define ID_SKIP_STEP 10011
#define ID_SET_LIB_MENU 10012
#define ID_MACROFLOW_NEW 10013
#define ID_MACROFLOW_OPEN 10014
#define ID_MACROFLOW_SAVE_AS 10015
#define ID_MACROFLOW_CUT 10016
#define ID_MACROFLOW_COPY 10017
#define ID_MACROFLOW_PASTE 10018
#define ID_MACROFLOW_DELETE 10019
#define ID_MACROFLOW_ZOOM_IN 10020
#define ID_MACROFLOW_ZOOM_OUT 10021
#define ID_MACROFLOW_FORWARD 10022
#define ID_MACROFLOW_ADD 10023
#define ID_MACROFLOW_STOP 10024
#define ID_MACROFLOW_FIND 10025
#define ID_MACROFLOW_LOCK_DRAG 10026
#define ID_EDIT_LIB_MENU 10027
#define ID_MACROFLOW_SAVE 10028
#define ID_MACROFLOW_EXPORT_STEPS 10029
#define ID_MACROFLOW_DUPLICATE 10030
#define ID_MACROFLOW_APPEND 10031
#define ID_MACROFLOW_UNDO 10032
#define ID_MACROFLOW_LIB_TO_SCRIPT 10033
#define ID_MACROFLOW_DEL_FROM_LIB 10034
#define ID_MACROFLOW_LIB_EXPAND 10035
#define ID_MACROFLOW_LIB_COLLAPSE 10036
#define ID_COLLAPSIBLE_PANE 10037

#define SYMBOL_CMACROFLOWEDITOR_STYLE wxCAPTION|wxRESIZE_BORDER|wxSYSTEM_MENU|wxMINIMIZE_BOX|wxMAXIMIZE_BOX|wxCLOSE_BOX
#define SYMBOL_CMACROFLOWEDITOR_TITLE _("Macro Flow Editor")
#define SYMBOL_CMACROFLOWEDITOR_IDNAME ID_CMACROFLOWEDITOR
#define SYMBOL_CMACROFLOWEDITOR_SIZE wxSize(1500, 1000)
#define SYMBOL_CMACROFLOWEDITOR_POSITION wxDefaultPosition
////@end control identifiers


/*!
 * CMacroFlowEditor class declaration
 */

class NCBI_GUIWIDGETS_EDIT_EXPORT CMacroFlowEditor: public wxFrame
{    
    DECLARE_CLASS( CMacroFlowEditor )
    DECLARE_EVENT_TABLE()

public:
    /// Constructors
    CMacroFlowEditor();
    CMacroFlowEditor( wxWindow* parent, wxWindowID id = SYMBOL_CMACROFLOWEDITOR_IDNAME, const wxString& caption = SYMBOL_CMACROFLOWEDITOR_TITLE, const wxPoint& pos = SYMBOL_CMACROFLOWEDITOR_POSITION, const wxSize& size = SYMBOL_CMACROFLOWEDITOR_SIZE, long style = SYMBOL_CMACROFLOWEDITOR_STYLE );

    CMacroFlowEditor( wxWindow* parent, CRef<IGuiCoreHelper> gui_core_helper,
                      wxWindowID id = SYMBOL_CMACROFLOWEDITOR_IDNAME, const wxString& caption = SYMBOL_CMACROFLOWEDITOR_TITLE, const wxPoint& pos = SYMBOL_CMACROFLOWEDITOR_POSITION, const wxSize& size = SYMBOL_CMACROFLOWEDITOR_SIZE, long style = SYMBOL_CMACROFLOWEDITOR_STYLE );

    static CMacroFlowEditor* GetInstance( wxWindow* parent, CRef<IGuiCoreHelper> gui_core_helper,
					  wxWindowID id = SYMBOL_CMACROFLOWEDITOR_IDNAME, const wxString& caption = SYMBOL_CMACROFLOWEDITOR_TITLE, const wxPoint& pos = SYMBOL_CMACROFLOWEDITOR_POSITION, 
					  const wxSize& size = SYMBOL_CMACROFLOWEDITOR_SIZE, long style = SYMBOL_CMACROFLOWEDITOR_STYLE );

    bool Create( wxWindow* parent, wxWindowID id = SYMBOL_CMACROFLOWEDITOR_IDNAME, const wxString& caption = SYMBOL_CMACROFLOWEDITOR_TITLE, const wxPoint& pos = SYMBOL_CMACROFLOWEDITOR_POSITION, const wxSize& size = SYMBOL_CMACROFLOWEDITOR_SIZE, long style = SYMBOL_CMACROFLOWEDITOR_STYLE );

    /// Destructor
    ~CMacroFlowEditor();

    /// Initialises member variables
    void Init();

    /// Creates the controls and sizers
    void CreateControls();

////@begin CMacroFlowEditor event handler declarations
   /// wxEVT_COMMAND_MENU_SELECTED event handler for wxID_NEW
    void OnNewClick( wxCommandEvent& event );

    /// wxEVT_COMMAND_MENU_SELECTED event handler for wxID_OPEN
    void OnOpenClick( wxCommandEvent& event );

    void OnRecent( wxCommandEvent& event);

    /// wxEVT_COMMAND_MENU_SELECTED event handler for wxID_SAVE
    void OnSaveClick( wxCommandEvent& event );

    /// wxEVT_UPDATE_UI event handler for wxID_SAVE
    void OnSaveUpdate( wxUpdateUIEvent& event );

    void OnSaveAsClick( wxCommandEvent& event );
    void OnSaveAsUpdate( wxUpdateUIEvent& event );

    void OnExportStepsClick( wxCommandEvent& event );

    /// wxEVT_COMMAND_MENU_SELECTED event handler for ID_IMPORT_MENU
    void OnImportMenuClick( wxCommandEvent& event );

    /// wxEVT_COMMAND_MENU_SELECTED event handler for ID_EXPORT_MENU
    void OnExportMenuClick( wxCommandEvent& event );
    void OnExportUpdate( wxUpdateUIEvent& event );

    void OnEditLibrary( wxCommandEvent& event );
    void OnEditLibraryUpdate( wxUpdateUIEvent& event );

    /// wxEVT_COMMAND_MENU_SELECTED event handler for wxID_EXIT
    void OnExitClick( wxCommandEvent& event );

    /// wxEVT_COMMAND_MENU_SELECTED event handler for wxID_CUT
    void OnCutClick( wxCommandEvent& event );

    /// wxEVT_UPDATE_UI event handler for wxID_CUT
    void OnEditUpdate( wxUpdateUIEvent& event );
    void OnPasteUpdate( wxUpdateUIEvent& event );

    /// wxEVT_COMMAND_MENU_SELECTED event handler for wxID_COPY
    void OnCopyClick( wxCommandEvent& event );

    /// wxEVT_COMMAND_MENU_SELECTED event handler for wxID_PASTE
    void OnPasteClick( wxCommandEvent& event );

    /// wxEVT_COMMAND_MENU_SELECTED event handler for wxID_DELETE
    void OnDeleteClick( wxCommandEvent& event );

    void OnDuplicateClick( wxCommandEvent& event );
    void OnDuplicateUpdate( wxUpdateUIEvent& event );

    void OnAppendClick( wxCommandEvent& event );

    void OnIncreaseFont( wxCommandEvent& event );
    void OnDecreaseFont( wxCommandEvent& event );

    /// wxEVT_COMMAND_MENU_SELECTED event handler for wxID_FIND
    void OnFindClick( wxCommandEvent& event );

    /// wxEVT_COMMAND_MENU_SELECTED event handler for wxID_ADD
    void OnAddClick( wxCommandEvent& event );

    /// wxEVT_COMMAND_MENU_SELECTED event handler for wxID_FORWARD
    void OnForwardClick( wxCommandEvent& event );

    /// wxEVT_UPDATE_UI event handler for wxID_FORWARD
    void OnForwardUpdate( wxUpdateUIEvent& event );

    void OnUndo( wxCommandEvent& event );
    void OnUndoUpdate( wxUpdateUIEvent& event );

    /// wxEVT_COMMAND_MENU_SELECTED event handler for wxID_STOP
    void OnStopClick( wxCommandEvent& event );

    /// wxEVT_UPDATE_UI event handler for wxID_STOP
    void OnStopUpdate( wxUpdateUIEvent& event );

    /// wxEVT_UPDATE_UI event handler for wxID_FIND
    void OnFindUpdate( wxUpdateUIEvent& event );

    /// wxEVT_UPDATE_UI event handler for wxID_ADD
    void OnAddUpdate( wxUpdateUIEvent& event );

    /// wxEVT_COMMAND_TREE_ITEM_ACTIVATED event handler for ID_TREECTRL
    void OnTreectrlItemActivated( wxTreeEvent& event );

    void OnTreectrlItemDrag( wxTreeEvent& event );
    void OnTreectrlItemDrop( wxTreeEvent& event );
    void OnTreectrlMenu( wxTreeEvent& event );

  /// wxEVT_COMMAND_MENU_SELECTED event handler for ID_SKIP_STEP
    void OnSkipStepClick( wxCommandEvent& event );

    /// wxEVT_UPDATE_UI event handler for ID_SKIP_STEP
    void OnSkipStepUpdate( wxUpdateUIEvent& event );

    void OnInputFile( wxCommandEvent& event );

    void OnSetDefaultLibrary( wxCommandEvent& event );

    void OnLockDrag( wxCommandEvent& event );

    void OnClose(wxCloseEvent& event);
    void OnPageClose(wxAuiNotebookEvent& event);
    void OnPageChanged(wxAuiNotebookEvent& event);
    void OnPageChanging(wxAuiNotebookEvent& event);

    void OnLibToScript( wxCommandEvent& event );
    void OnLibToScriptUpdate( wxUpdateUIEvent& event );

    void OnDeleteFromLibrary( wxCommandEvent& event );
    void OnDeleteFromLibraryUpdate( wxUpdateUIEvent& event );

    void OnLibExpand( wxCommandEvent& event );
    void OnLibCollapse( wxCommandEvent& event );
    void OnLibraryExpandUpdate( wxUpdateUIEvent& event );

	void OnCollapsiblePane(wxCollapsiblePaneEvent& event);
////@end CMacroFlowEditor event handler declarations

////@begin CMacroFlowEditor member function declarations

    /// Retrieves bitmap resources
    wxBitmap GetBitmapResource( const wxString& name );

    /// Retrieves icon resources
    wxIcon GetIconResource( const wxString& name );
////@end CMacroFlowEditor member function declarations

    /// Should we show tooltips?
    static bool ShowToolTips();

    virtual void SetRegistryPath(const string& reg_path);  
    virtual void LoadSettings();
    virtual void SaveSettings() const;

    void AddNewMacro(CRef<macro::CMacroRep> macro_rep);
    bool GetLockDrag() {return m_Lock;} //{return m_LockDrag->GetValue();}
    void RemoveEditor();

    wxBitmap GetLockBitmap() {return m_LockBitmap;}
    wxBitmap GetPlusBitmap() {return m_PlusBitmap;}
    void Shake();
    void OpenScript(const wxString &path, bool recent_scripts, bool allow_empty);

    void SaveAddMacroSizeAndPosition(int w, int h, int x, int y);
 private:
    void ImportLibrary(const wxString &path, bool report);
    wxString GetSettingsPath() const;
    void Pulse(int c);
    bool RunScript( const vector<pair<CRef<macro::CMacroRep>, bool> >& script, CScriptPanel *page, wxString &log, size_t num_files, size_t num_entries, objects::CSeq_entry_Handle seh, 
                    CConstRef<objects::CSeq_submit> submit = CConstRef<objects::CSeq_submit>(NULL), CRef<CCmdComposite> cmd = CRef<CCmdComposite>(NULL));
    void RunScriptInWidget( const vector<pair<CRef<macro::CMacroRep>, bool> >& script, CScriptPanel *page);
    void RunScriptStandalone( const vector<pair<CRef<macro::CMacroRep>, bool> >& script, CScriptPanel *page);
    void UpdateCounter(CScriptPanel *page, size_t i, size_t counter);
    void LibraryModified();
    void SaveScript(const wxString &path, const vector<pair<CRef<macro::CMacroRep>, bool> >& script);
    void PopulateRecentMenu();
    void CopyDefaultLibrary();
    void DeleteNode(wxTreeItemId &id);
    void GetExpandedNodes(wxTreeItemId id, set<string> &expanded_nodes, string label = kEmptyStr) ;
    void SetExpandedNodes(wxTreeItemId id, const set<string> &expanded_nodes, string label = kEmptyStr);
    void LoadScrollPos();
    void SetPositionAndSize();

////@begin CMacroFlowEditor member variables
    CRef<IGuiCoreHelper> m_gui_core_helper;
    bool m_standalone;
    wxCollapsiblePane *m_CollapsiblePanel;
    wxWindow *m_CollapsibleWindow;
    wxTreeCtrl* m_TreeCtrl;
    wxAuiNotebook* m_Notebook;
    wxTextCtrl* m_InputFileText;
    wxArrayString m_InputFiles;
    wxDirPickerCtrl* m_OutputFolder;
    //    wxCheckBox* m_LockDrag;
    bool m_Lock;
    wxBitmap m_LockBitmap;
    wxBitmap m_UnlockBitmap;
    wxBitmap m_PlusBitmap;
    wxToolBar* m_toolbar;
    wxGauge* m_Progress;
    wxTextCtrl* m_Search;
    size_t m_script_count;
    wxString m_DefaultDir;
    map<wxTreeItemId, CRef<macro::CMacroRep> > m_id_to_macro;
    vector<wxTreeItemId> m_all_ids;
    string m_RegPath;
    list<string> m_opened_scripts;
    wxMenu *m_recent_submenu;
    map<int, string> m_id_to_path;
    bool m_stop;
    bool m_running;
    vector<CRef<macro::CMacroRep> > m_clipboard;
    vector<bool> m_clipboard_skipped;
    // used for logging copy/paste events
    int m_LastEventId{ 0 };
    string m_prev_search;
    size_t m_found;
    string m_DefaultLibrary;
    string m_CurrentLibrary;
    bool m_loading_script;
    bool m_loading_library;
    CMacroEditor* m_MacroEditor;
    wxTreeItemId m_drag_id;
    IUndoManager* m_UndoManager;
    objects::CSeq_entry_Handle m_undo_tse;
    static CMacroFlowEditor *m_Instance;
    int m_colpane_width;
    bool  m_HideLibrary;
    int m_width;
    int m_height;
    int m_pos_x;
    int m_pos_y;
    int m_width_add_macro;
    int m_height_add_macro;
    int m_pos_x_add_macro;
    int m_pos_y_add_macro;
////@end CMacroFlowEditor member variables
};


class CStopWorkException : public CException
{
};

END_NCBI_SCOPE
#endif
    // _MACROFLOWEDITOR_H_
