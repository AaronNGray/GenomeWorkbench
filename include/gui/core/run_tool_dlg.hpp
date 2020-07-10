#ifndef GUI_CORE___RUN_TOOL_DLG__HPP
#define GUI_CORE___RUN_TOOL_DLG__HPP

/*  $Id: run_tool_dlg.hpp 39737 2017-10-31 17:03:07Z katargir $
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

#include <corelib/ncbiobj.hpp>

#include <gui/gui_export.h>

#include <gui/widgets/wx/dialog.hpp>

#include <gui/core/ui_tool_manager.hpp>
#include <gui/utils/mru_list.hpp>

#include <gui/framework/app_task.hpp>


#include <wx/statline.h>
#include <wx/hyperlink.h>

////@begin control identifiers
#define SYMBOL_CRUNTOOLDLG_STYLE wxCAPTION|wxRESIZE_BORDER|wxSYSTEM_MENU|wxCLOSE_BOX|wxTAB_TRAVERSAL
#define SYMBOL_CRUNTOOLDLG_TITLE _("Run Tool")
#define SYMBOL_CRUNTOOLDLG_IDNAME ID_CRUNTOOLDLG
#define SYMBOL_CRUNTOOLDLG_SIZE wxSize(400, 300)
#define SYMBOL_CRUNTOOLDLG_POSITION wxDefaultPosition
////@end control identifiers

////@begin forward declarations
class wxHyperlinkCtrl;
////@end forward declarations

class wxButton;


BEGIN_NCBI_SCOPE

class CItemSelectionPanel;

class NCBI_GUICORE_EXPORT CRunToolDlg: public CDialog
{
    DECLARE_DYNAMIC_CLASS( CRunToolDlg )
    DECLARE_EVENT_TABLE()
public:
    typedef CIRef<IUIAlgoToolManager>   TManagerRef;

    CRunToolDlg();
    CRunToolDlg( wxWindow* parent, const vector<TManagerRef>& managers, IServiceLocator* srvLocator );

    bool Create( wxWindow* parent, wxWindowID id = SYMBOL_CRUNTOOLDLG_IDNAME, const wxString& caption = SYMBOL_CRUNTOOLDLG_TITLE, const wxPoint& pos = SYMBOL_CRUNTOOLDLG_POSITION, const wxSize& size = SYMBOL_CRUNTOOLDLG_SIZE, long style = SYMBOL_CRUNTOOLDLG_STYLE );

    ~CRunToolDlg();

    void Init();

    void CreateControls();

////@begin CRunToolDlg event handler declarations

    void OnRestoreDefaultsBtnClick( wxCommandEvent& event );

    void OnButtonClick( wxCommandEvent& event );

    void OnBackwardClick( wxCommandEvent& event );

    void OnForwardClick( wxCommandEvent& event );

////@end CRunToolDlg event handler declarations

////@begin CRunToolDlg member function declarations

    wxBitmap GetBitmapResource( const wxString& name );

    wxIcon GetIconResource( const wxString& name );
////@end CRunToolDlg member function declarations

    static bool ShowToolTips();

////@begin CRunToolDlg member variables
    wxPanel* m_Panel;
    wxHyperlinkCtrl* m_HelpLink;
    wxButton* m_RestoreDefaultsBtn;
    wxButton* m_QuickLaunch;
    wxButton* m_BackBtn;
    wxButton* m_NextBtn;
    enum {
        ID_CRUNTOOLDLG = 10025,
        ID_PANEL = 10000,
        ID_HYPERLINKCTRL = 10005,
        ID_RESTORE_DEFAULTS_BTN = 10002,
        ID_BUTTON = 10112
    };
////@end CRunToolDlg member variables

public:
    void    SetInputObjects(const vector<TConstScopedObjects>& input_objects);

    virtual CIRef<IAppTask> GetToolTask();

    void    OnToolSelected(wxCommandEvent& event);

    /// @name IRegSettings interface
    /// @{
    virtual void    SetRegistryPath(const string& path);
    /// @}

public:
    virtual int ShowModal();

    // overriding CDialog::EndModal()
    virtual void EndModal(int ret_code);

protected:
    void    x_EndModal(int res);

    virtual void    x_LoadSettings(const CRegistryReadView& view);
    virtual void    x_SaveSettings(CRegistryWriteView view) const;

    void    x_SetCurrentPanel(wxPanel* panel, const wxString& caption = SYMBOL_CRUNTOOLDLG_TITLE);
    void    x_CalcMinPanelSize();
    void    x_UpdateButtons();
    void    x_OnToolSelected();
    bool    x_DoTransition(IUIToolManager::EAction action);

    void    x_CreateToolItems();

protected:
    IServiceLocator*    m_SrvLocator;

    vector<TManagerRef> m_Managers; // tool managers

    CMRUList<string>    m_RecentTools;

    /// objects that serve as input arguments for the tools
    vector<TConstScopedObjects> m_InputObjects;

    CItemSelectionPanel*    m_ItemPanel;
    TManagerRef m_CurrManager;
    wxPanel*    m_CurrPanel;

    CIRef<IAppTask> m_AppTask; // a task for launching the selected tool
};

END_NCBI_SCOPE


#endif
    // GUI_CORE___RUN_TOOL_DLG__HPP
