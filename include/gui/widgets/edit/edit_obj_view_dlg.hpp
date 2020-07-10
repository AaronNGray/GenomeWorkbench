/*  $Id: edit_obj_view_dlg.hpp 43609 2019-08-08 16:12:53Z filippov $
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
 * Authors:  Colleen Bollin
 */
#ifndef _EDIT_OBJ_VIEW_DLG_H_
#define _EDIT_OBJ_VIEW_DLG_H_

#include <corelib/ncbistd.hpp>

#include <gui/widgets/wx/gui_widget_dlg.hpp>
#include <gui/widgets/edit/edit_object.hpp>
#include <gui/widgets/edit/import_export_access.hpp>
#include <gui/widgets/edit/work_dir.hpp>

/*!
 * Includes
 */

////@begin includes
#include "wx/statline.h"
////@end includes
#include <wx/sizer.h>
#include <wx/frame.h>

/*!
 * Forward declarations
 */

////@begin forward declarations
class wxBoxSizer;
class wxPanel;
class wxHyperlinkCtrl;
////@end forward declarations

/*!
 * Control identifiers
 */

////@begin control identifiers
#define SYMBOL_CEDITOBJVIEWDLG_STYLE wxCAPTION|wxRESIZE_BORDER|wxCLOSE_BOX|wxTAB_TRAVERSAL
#define SYMBOL_CEDITOBJVIEWDLG_TITLE _("EditObjViewDlg")
#define SYMBOL_CEDITOBJVIEWDLG_IDNAME ID_CEDITOBJVIEWDLG
#define SYMBOL_CEDITOBJVIEWDLG_SIZE wxSize(400, 300)
#define SYMBOL_CEDITOBJVIEWDLG_POSITION wxDefaultPosition
////@end control identifiers

BEGIN_NCBI_SCOPE

/*!
 * CEditObjViewDlg class declaration
 */


class NCBI_GUIWIDGETS_EDIT_EXPORT CEditObjViewDlg: public wxFrame, public CImportExportContainer, public IWorkDirClient, public IHelpUrlClient
{    
    DECLARE_DYNAMIC_CLASS( CEditObjViewDlg )
    DECLARE_EVENT_TABLE()

public:
    /// Constructors
    CEditObjViewDlg();
    CEditObjViewDlg( wxWindow* parent, bool create, wxWindowID id = SYMBOL_CEDITOBJVIEWDLG_IDNAME, const wxString& caption = SYMBOL_CEDITOBJVIEWDLG_TITLE, const wxPoint& pos = SYMBOL_CEDITOBJVIEWDLG_POSITION, const wxSize& size = SYMBOL_CEDITOBJVIEWDLG_SIZE, long style = SYMBOL_CEDITOBJVIEWDLG_STYLE );

    /// Creation
    bool Create( wxWindow* parent, wxWindowID id = SYMBOL_CEDITOBJVIEWDLG_IDNAME, const wxString& caption = SYMBOL_CEDITOBJVIEWDLG_TITLE, const wxPoint& pos = SYMBOL_CEDITOBJVIEWDLG_POSITION, const wxSize& size = SYMBOL_CEDITOBJVIEWDLG_SIZE, long style = SYMBOL_CEDITOBJVIEWDLG_STYLE );

    /// Destructor
    ~CEditObjViewDlg();

    /// Initialises member variables
    void Init();

    /// Creates the controls and sizers
    void CreateControls();

////@begin CEditObjViewDlg event handler declarations

    /// wxEVT_COMMAND_BUTTON_CLICKED event handler for wxID_OK
    void OnOkClick( wxCommandEvent& event );    
    void OnExport( wxCommandEvent& event );
    void OnImport( wxCommandEvent& event );
    void OnCancelClick( wxCommandEvent& event );

    virtual void EnableImport(bool enable);
    virtual void EnableExport(bool enable);
    CImportExportAccess*  GetImportExportWindow();
////@end CEditObjViewDlg event handler declarations

    void Activate(wxIdleEvent&);

////@begin CEditObjViewDlg member function declarations

    /// Retrieves bitmap resources
    wxBitmap GetBitmapResource( const wxString& name );

    /// Retrieves icon resources
    wxIcon GetIconResource( const wxString& name );
////@end CEditObjViewDlg member function declarations

    /// Should we show tooltips?
    static bool ShowToolTips();

////@begin CEditObjViewDlg member variables
    wxBoxSizer* m_EditWindowSizer;
    /// Control identifiers
    enum {
        ID_CEDITOBJVIEWDLG = 10175,
        CEDITOBJVIEWDLG_BUTTON_IMPORT,
        CEDITOBJVIEWDLG_BUTTON_EXPORT
    };

    // Creates panels corresponding to object
    void SetEditorWindow(wxWindow* editorWindow);

   
    // Data
    virtual void SetUndoManager(ICommandProccessor* cmdProccessor) {
                       m_CmdProccessor = cmdProccessor; }
    ICommandProccessor* GetUndoManager(void) {return m_CmdProccessor;}

    void SetEditor(CIRef<IEditObject> editor) {m_Editor = editor;}

    /// @name IWorkDirClient implementation
    /// @{
    virtual void SetWorkDir(const wxString& workDir) { m_WorkDir = workDir; }
    /// @}

    virtual void SetHelpUrl(const wxString &url);
protected:
    TConstScopedObjects m_Orig_Objects;
    CIRef<IEditObject> m_Editor;
    wxWindow* m_EditorWindow;
    wxPanel* m_ContainerPanel;
    bool m_Create;
    wxButton *m_ButtonImport, *m_ButtonExport;
    ICommandProccessor* m_CmdProccessor;
    wxString m_WorkDir;
    wxHyperlinkCtrl* m_ButtonHelp;
};

class NCBI_GUIWIDGETS_EDIT_EXPORT CEditObjViewDlgModal: public CGuiWidgetDlg, public CImportExportContainer, public IHelpUrlClient
{    
    DECLARE_DYNAMIC_CLASS( CEditObjViewDlgModal )
    DECLARE_EVENT_TABLE()

public:
    /// Constructors
    CEditObjViewDlgModal();
    CEditObjViewDlgModal( wxWindow* parent, bool create, wxWindowID id = SYMBOL_CEDITOBJVIEWDLG_IDNAME, const wxString& caption = SYMBOL_CEDITOBJVIEWDLG_TITLE, const wxPoint& pos = SYMBOL_CEDITOBJVIEWDLG_POSITION, const wxSize& size = SYMBOL_CEDITOBJVIEWDLG_SIZE, long style = SYMBOL_CEDITOBJVIEWDLG_STYLE );

    /// Creation
    bool Create( wxWindow* parent, wxWindowID id = SYMBOL_CEDITOBJVIEWDLG_IDNAME, const wxString& caption = SYMBOL_CEDITOBJVIEWDLG_TITLE, const wxPoint& pos = SYMBOL_CEDITOBJVIEWDLG_POSITION, const wxSize& size = SYMBOL_CEDITOBJVIEWDLG_SIZE, long style = SYMBOL_CEDITOBJVIEWDLG_STYLE );

    /// Destructor
    ~CEditObjViewDlgModal();

    /// Initialises member variables
    void Init();

    /// Creates the controls and sizers
    void CreateControls();

////@begin CEditObjViewDlg event handler declarations

    /// wxEVT_COMMAND_BUTTON_CLICKED event handler for wxID_OK
    void OnOkClick( wxCommandEvent& event );
    void OnExport( wxCommandEvent& event );
    void OnImport( wxCommandEvent& event );
    void OnClose(wxCloseEvent& event);
    virtual bool Show(bool show = true);

    virtual void EnableImport(bool enable);
    virtual void EnableExport(bool enable);
    CImportExportAccess*  GetImportExportWindow();
////@end CEditObjViewDlg event handler declarations

////@begin CEditObjViewDlg member function declarations

    /// Retrieves bitmap resources
    wxBitmap GetBitmapResource( const wxString& name );

    /// Retrieves icon resources
    wxIcon GetIconResource( const wxString& name );
////@end CEditObjViewDlg member function declarations

    /// Should we show tooltips?
    static bool ShowToolTips();

////@begin CEditObjViewDlg member variables
    wxBoxSizer* m_EditWindowSizer;
    /// Control identifiers
    enum {
        ID_CEDITOBJVIEWDLG = 10175,
        CEDITOBJVIEWDLG_BUTTON_IMPORT,
        CEDITOBJVIEWDLG_BUTTON_EXPORT
    };
////@end CEditObjViewDlg member variables
    static wxWindow* CreateDialogNew(wxWindow* parent);
    static wxWindow* CreateDialogEdit(wxWindow* parent);

    // Creates panels corresponding to object
    void SetEditorWindow(wxWindow* editorWindow);

    /// @name IGuiWidget implementation
    /// @{
    virtual void SetHost(IGuiWidgetHost* host);
    virtual bool InitWidget(TConstScopedObjects& objects);
    virtual const CObject* GetOrigObject() const;

    // Selection
    virtual void GetSelectedObjects (TConstObjects& objects) const;
    virtual void SetSelectedObjects (const TConstObjects& objects);

    // Data
    virtual void SetUndoManager(ICommandProccessor* cmdProccessor) {
                       m_CmdProccessor = cmdProccessor; }
    ICommandProccessor* GetUndoManager(void) {return m_CmdProccessor;}
    virtual void DataChanging();
    virtual void DataChanged();

    // Objects for action
    virtual void GetActiveObjects(vector<TConstScopedObjects>& objects);
    /// @}
    void SetEditor(CIRef<IEditObject> editor) {m_Editor = editor;}
    virtual void SetHelpUrl(const wxString &url);
protected:
    TConstScopedObjects m_Orig_Objects;
    CIRef<IEditObject> m_Editor;
    wxWindow* m_EditorWindow;
    bool m_Create;
    wxButton *m_ButtonImport, *m_ButtonExport;
    wxHyperlinkCtrl* m_ButtonHelp;
};

END_NCBI_SCOPE

#endif
    // _EDIT_OBJ_VIEW_DLG_H_
