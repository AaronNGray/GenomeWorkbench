#ifndef GUI_WIDGETS_WX__WINDOWS_DLG__HPP
#define GUI_WIDGETS_WX__WINDOWS_DLG__HPP

/*  $Id: windows_dlg.hpp 26253 2012-08-10 18:19:02Z katargir $
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
 * Authors:  Andrey Yazhuk
 */

#include <corelib/ncbistd.hpp>


#include <gui/widgets/wx/dialog.hpp>

#include <gui/widgets/wx/table_model.hpp>

#include <wx/listctrl.h>

////@begin control identifiers
#define SYMBOL_CWINDOWSDLG_STYLE wxCAPTION|wxRESIZE_BORDER|wxSYSTEM_MENU|wxCLOSE_BOX|wxTAB_TRAVERSAL
#define SYMBOL_CWINDOWSDLG_TITLE _("Windows")
#define SYMBOL_CWINDOWSDLG_IDNAME ID_WINDOWS_DLG
#define SYMBOL_CWINDOWSDLG_SIZE wxSize(400, 300)
#define SYMBOL_CWINDOWSDLG_POSITION wxDefaultPosition
////@end control identifiers


class wxButton;


BEGIN_NCBI_SCOPE;

class CwxTableListCtrl;
class CWindowManager;
class IWMClient;

///////////////////////////////////////////////////////////////////////////////
/// CWindowsDlg

class CWindowsDlg: public CDialog
{
    DECLARE_DYNAMIC_CLASS( CWindowsDlg )
    DECLARE_EVENT_TABLE()

public:
    /// Constructors
    CWindowsDlg();
    CWindowsDlg( wxWindow* parent, wxWindowID id = SYMBOL_CWINDOWSDLG_IDNAME, const wxString& caption = SYMBOL_CWINDOWSDLG_TITLE, const wxPoint& pos = SYMBOL_CWINDOWSDLG_POSITION, const wxSize& size = SYMBOL_CWINDOWSDLG_SIZE, long style = SYMBOL_CWINDOWSDLG_STYLE );

    /// Creation
    bool Create( wxWindow* parent, wxWindowID id = SYMBOL_CWINDOWSDLG_IDNAME, const wxString& caption = SYMBOL_CWINDOWSDLG_TITLE, const wxPoint& pos = SYMBOL_CWINDOWSDLG_POSITION, const wxSize& size = SYMBOL_CWINDOWSDLG_SIZE, long style = SYMBOL_CWINDOWSDLG_STYLE );

    /// Destructor
    ~CWindowsDlg();

    /// Initialises member variables
    void Init();

    /// Creates the controls and sizers
    void CreateControls();

////@begin CWindowsDlg event handler declarations

    /// wxEVT_COMMAND_LIST_ITEM_SELECTED event handler for ID_TABLE
    void OnTableSelected( wxListEvent& event );

    /// wxEVT_COMMAND_LIST_ITEM_DESELECTED event handler for ID_TABLE
    void OnTableDeselected( wxListEvent& event );

    /// wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_ACTIVATE
    void OnActivateClick( wxCommandEvent& event );

    /// wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_FLOAT
    void OnFloatClick( wxCommandEvent& event );

    /// wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_RECOVER
    void OnRecoverClick( wxCommandEvent& event );

    /// wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_MINIMIZE
    void OnMinimizeClick( wxCommandEvent& event );

    /// wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_RESTORE
    void OnRestoreClick( wxCommandEvent& event );

    /// wxEVT_COMMAND_BUTTON_CLICKED event handler for wxID_CLOSE
    void OnCloseClick( wxCommandEvent& event );

////@end CWindowsDlg event handler declarations

////@begin CWindowsDlg member function declarations

    /// Retrieves bitmap resources
    wxBitmap GetBitmapResource( const wxString& name );

    /// Retrieves icon resources
    wxIcon GetIconResource( const wxString& name );
////@end CWindowsDlg member function declarations

    /// Should we show tooltips?
    static bool ShowToolTips();

////@begin CWindowsDlg member variables
    CwxTableListCtrl* m_Table;
    wxButton* m_FloatBtn;
    wxButton* m_RecoverBtn;
    wxButton* m_MinimizeBtn;
    wxButton* m_RestoreBtn;
    wxButton* m_CloseBtn;
    /// Control identifiers
    enum {
        ID_WINDOWS_DLG = 10002,
        ID_TABLE = 10003,
        ID_ACTIVATE = 10004,
        ID_FLOAT = 10000,
        ID_RECOVER = 10005,
        ID_MINIMIZE = 10001,
        ID_RESTORE = 10006
    };
////@end CWindowsDlg member variables

public:
    void    SetWindowManager(CWindowManager* manager);

protected:
    void    x_UpdateTable(const vector<IWMClient*>& clients);
    void    x_GetSelectedClients(vector<IWMClient*>& clients);
    void    x_UpdateButtons();
    void    x_SelectClients(vector<IWMClient*>& clients);

protected:
    CWindowManager*   m_WindowManager;
    CTextTableModel     m_Model;
};

END_NCBI_SCOPE


#endif //GUI_WIDGETS_WX__WINDOWS_DLG__HPP
