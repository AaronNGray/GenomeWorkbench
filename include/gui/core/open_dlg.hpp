#ifndef GUI_CORE___OPEN_DLG__HPP
#define GUI_CORE___OPEN_DLG__HPP

/*  $Id: open_dlg.hpp 44281 2019-11-21 21:59:07Z katargir $
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
 */

#include <corelib/ncbistl.hpp>
#include <corelib/ncbiobj.hpp>

#include <gui/gui_export.h>

#include <gui/core/ui_tool_manager.hpp>

#include <gui/framework/app_task.hpp>

#include <gui/widgets/wx/dialog.hpp>

#include "wx/listbase.h"
////@begin includes
#include "wx/statline.h"
#include "wx/hyperlink.h"
////@end includes

class wxPanel;
class wxButton;
class wxCheckBox;
class wxListBox;
////@begin forward declarations
class CSplitter;
////@end forward declarations


////@begin control identifiers
#define SYMBOL_COPENDLG_STYLE wxCAPTION|wxRESIZE_BORDER|wxSYSTEM_MENU|wxCLOSE_BOX|wxMAXIMIZE_BOX|wxTAB_TRAVERSAL
#define SYMBOL_COPENDLG_TITLE _("Open")
#define SYMBOL_COPENDLG_IDNAME ID_COPENDLG
#define SYMBOL_COPENDLG_SIZE wxSize(450, 275)
#define SYMBOL_COPENDLG_POSITION wxDefaultPosition
////@end control identifiers
#define ID_SKIP 10064

BEGIN_NCBI_SCOPE

class CSplitter;
class CProjectSelectorPanel;

///////////////////////////////////////////////////////////////////////////////
/// COpenDlg
class NCBI_GUICORE_EXPORT COpenDlg: public CDialog
{
    DECLARE_DYNAMIC_CLASS( COpenDlg )
    DECLARE_EVENT_TABLE()

public:
    COpenDlg();
    COpenDlg( wxWindow* parent, wxWindowID id = SYMBOL_COPENDLG_IDNAME, const wxString& caption = SYMBOL_COPENDLG_TITLE, const wxPoint& pos = SYMBOL_COPENDLG_POSITION, const wxSize& size = SYMBOL_COPENDLG_SIZE, long style = SYMBOL_COPENDLG_STYLE );

    bool Create( wxWindow* parent, wxWindowID id = SYMBOL_COPENDLG_IDNAME, const wxString& caption = SYMBOL_COPENDLG_TITLE, const wxPoint& pos = SYMBOL_COPENDLG_POSITION, const wxSize& size = SYMBOL_COPENDLG_SIZE, long style = SYMBOL_COPENDLG_STYLE );

    ~COpenDlg();

    void Init();

    void CreateControls();

////@begin COpenDlg event handler declarations

    void OnOptionListSelected( wxCommandEvent& event );

    void OnBackwardClick( wxCommandEvent& event );

    void OnForwardClick( wxCommandEvent& event );

    void OnCancelClick( wxCommandEvent& event );

////@end COpenDlg event handler declarations
    void OnSkipClick( wxCommandEvent& event );


    void OnListItemActivate( wxListEvent& event );

////@begin COpenDlg member function declarations

    wxString GetBaseTitle() const { return m_BaseTitle ; }
    void SetBaseTitle(wxString value) { m_BaseTitle = value ; }

    wxBitmap GetBitmapResource( const wxString& name );

    wxIcon GetIconResource( const wxString& name );
////@end COpenDlg member function declarations

    static bool ShowToolTips();

////@begin COpenDlg member variables
    CSplitter* m_Splitter;
    wxListBox* m_OptionList;
    wxButton* m_BackBtn;
    wxButton* m_NextBtn;
private:
    wxString m_BaseTitle;
    enum {
        ID_COPENDLG = 10002,
        ID_MAIN_PANEL = 10003,
        ID_option_list = 10004,
        ID_HELP_LINK = 10023
    };
////@end COpenDlg member variables

    enum {
        ID_OPTION_PANEL = 10005,
        ID_OPEN_FOLDER_CHECKBOX = 10006
    };
public:
    typedef CIRef<IUIToolManager>   TManagerRef;

    virtual void    SetManagers(vector<TManagerRef>& managers);
    virtual void    SetServiceLocator(IServiceLocator* srv_locator);
    virtual void    SetFilenames(const vector<wxString>& filenames);

    /// select loading options By Extension ID
    virtual void    SelectOptionByLabel(const string& label);

    virtual void    Update();

    virtual void InitDialog();

    virtual CIRef<IAppTask> GetLoadingTask();

    /// @name IRegSettings interface
    /// @{
    virtual void    SetRegistryPath(const string& path);
    /// @}

    virtual void    EndModal(int ret_code);

protected:
    void    x_AppendOptionForManager(IUIToolManager& manager);
    void    x_SelectManager(int index);
    void    x_SetCurrentPanel(wxPanel* panel, bool firstPage);
    void    x_DoTransition(IUIToolManager::EAction action);
    void    x_UpdateTitle();
    void    x_UpdateButtons();

    virtual void    x_LoadSettings(const CRegistryReadView& view);
    virtual void    x_SaveSettings(CRegistryWriteView view) const;

protected:
    IServiceLocator*    m_SrvLocator;

    /// Load Managers corresponding to Loading Options
    vector<TManagerRef> m_Managers;

    /// if >= 0, than the dialog shall show a particualr option
    int     m_RequestedManager;

    vector<wxString>      m_Filenames;

    int         m_CurrManager;
    wxPanel*    m_CurrPanel;
    wxPanel*    m_OptionPanel;
    wxCheckBox* m_OpenExportFolder;

    CIRef<IAppTask> m_AppTask;
};


END_NCBI_SCOPE

#endif /// GUI_CORE___OPEN_DLG__HPP

