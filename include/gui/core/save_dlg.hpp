#ifndef GUI_CORE___SAVE_DLG__HPP
#define GUI_CORE___SAVE_DLG__HPP

/*  $Id: save_dlg.hpp 39528 2017-10-05 15:27:37Z katargir $
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

#include <corelib/ncbistd.hpp>

#include <gui/widgets/wx/dialog.hpp>


////@begin includes
#include "wx/valgen.h"
////@end includes

////@begin forward declarations
class wxBoxSizer;
class wxStdDialogButtonSizer;
////@end forward declarations

class wxCheckBox;
class wxCheckListBox;
class wxStaticText;
class wxButton;

BEGIN_NCBI_SCOPE

BEGIN_SCOPE(objects);
    class CGBWorkspace;
END_SCOPE(objects);

class SSaveProjectsDlgParams {
public:
    string  m_Description; // the explantion shown in the dialog
    bool    m_ShowNoButton;

    vector<int> m_ProjectIds;   // all projects

    /// indices of the projects to save (checked items)
    wxArrayInt  m_ProjectsToSave;

    SSaveProjectsDlgParams();
};


////@begin control identifiers
#define SYMBOL_CSAVEDLG_STYLE wxCAPTION|wxRESIZE_BORDER|wxSYSTEM_MENU|wxCLOSE_BOX|wxTAB_TRAVERSAL
#define SYMBOL_CSAVEDLG_TITLE _("Save")
#define SYMBOL_CSAVEDLG_IDNAME ID_CSAVEDLG
#define SYMBOL_CSAVEDLG_SIZE wxDefaultSize
#define SYMBOL_CSAVEDLG_POSITION wxDefaultPosition
////@end control identifiers

///////////////////////////////////////////////////////////////////////////////
/// CSaveDlg - a dialog for saving workspace and projects.

class CSaveDlg: public CDialog
{
    DECLARE_DYNAMIC_CLASS( CSaveDlg )
    DECLARE_EVENT_TABLE()

public:
    CSaveDlg();
    CSaveDlg( objects::CGBWorkspace& ws, wxWindow* parent, wxWindowID id = SYMBOL_CSAVEDLG_IDNAME, const wxString& caption = SYMBOL_CSAVEDLG_TITLE, const wxPoint& pos = SYMBOL_CSAVEDLG_POSITION, const wxSize& size = SYMBOL_CSAVEDLG_SIZE, long style = SYMBOL_CSAVEDLG_STYLE );

    bool Create( wxWindow* parent, wxWindowID id = SYMBOL_CSAVEDLG_IDNAME, const wxString& caption = SYMBOL_CSAVEDLG_TITLE, const wxPoint& pos = SYMBOL_CSAVEDLG_POSITION, const wxSize& size = SYMBOL_CSAVEDLG_SIZE, long style = SYMBOL_CSAVEDLG_STYLE );

    ~CSaveDlg();

    void Init();

    void CreateControls();

    void SetWorkspace(objects::CGBWorkspace& ws) { m_WS = &ws; }

////@begin CSaveDlg event handler declarations

    void OnInitDialog( wxInitDialogEvent& event );

    void OnPrjListToggled( wxCommandEvent& event );

    void OnSelectChangedClick( wxCommandEvent& event );

    void OnSelectAllClick( wxCommandEvent& event );

    void OnClearAllClick( wxCommandEvent& event );

    void OnSaveClick( wxCommandEvent& event );

    void OnNoClick( wxCommandEvent& event );

////@end CSaveDlg event handler declarations

////@begin CSaveDlg member function declarations

    wxBitmap GetBitmapResource( const wxString& name );

    wxIcon GetIconResource( const wxString& name );
////@end CSaveDlg member function declarations

    static bool ShowToolTips();

////@begin CSaveDlg member variables
    wxBoxSizer* m_RootSizer;
    wxStaticText* m_DescrText;
    wxCheckListBox* m_ProjectList;
    wxStdDialogButtonSizer* m_BtnSizer;
    wxButton* m_SaveBtn;
protected:
    SSaveProjectsDlgParams m_Params;
    enum {
        ID_CSAVEDLG = 10030,
        ID_PRJ_LIST = 10032,
        ID_SELECT_CHANGED = 10014,
        ID_SELECT_ALL = 10041,
        ID_CLEAR_ALL = 10042
    };
////@end CSaveDlg member variables

    void    x_WrapDescription();

public:
    const SSaveProjectsDlgParams& GetParams() const;
    void SetParams(const SSaveProjectsDlgParams& value);

protected:
    bool    m_Wrapped;

    objects::CGBWorkspace* m_WS;

    void    x_UpdateButtons();
};

END_NCBI_SCOPE

#endif // GUI_CORE___SAVE_DLG__HPP
