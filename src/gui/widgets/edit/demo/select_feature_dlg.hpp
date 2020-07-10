#ifndef GUI_DIALOGS_EDIT_DEMO___DELECT_FEATURE__HPP
#define GUI_DIALOGS_EDIT_DEMO___DELECT_FEATURE__HPP

/*  $Id: select_feature_dlg.hpp 19352 2009-06-05 20:28:21Z katargir $
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
 * Authors:  Roman Katargin
 */

/*!
 * Includes
 */

#include <wx/dialog.h>
#include <wx/listbox.h>
#include <wx/button.h>
#include <wx/textctrl.h>
#include <wx/stattext.h>

////@begin includes
#include "wx/statline.h"
////@end includes

#include <objmgr/scope.hpp>
#include <objmgr/object_manager.hpp>

BEGIN_NCBI_SCOPE
USING_SCOPE(ncbi::objects);

/*!
 * Forward declarations
 */

////@begin forward declarations
////@end forward declarations

/*!
 * Control identifiers
 */

////@begin control identifiers
#define ID_CSELECTFEATUREDLG 10015
#define ID_CHOICE5 10016
#define ID_LISTBOX2 10017
#define ID_BUTTON2 10020
#define ID_BUTTON3 10000
#define ID_BUTTON4 10001
#define ID_TEXTCTRL7 10018
#define ID_BUTTON1 10019
#define SYMBOL_CSELECTFEATUREDLG_STYLE wxCAPTION|wxRESIZE_BORDER|wxSYSTEM_MENU|wxCLOSE_BOX|wxTAB_TRAVERSAL
#define SYMBOL_CSELECTFEATUREDLG_TITLE _("Select feature to edit")
#define SYMBOL_CSELECTFEATUREDLG_IDNAME ID_CSELECTFEATUREDLG
#define SYMBOL_CSELECTFEATUREDLG_SIZE wxSize(400, 300)
#define SYMBOL_CSELECTFEATUREDLG_POSITION wxDefaultPosition
////@end control identifiers


/*!
 * CSelectFeatureDlg class declaration
 */

class CSelectFeatureDlg: public wxDialog
{
    DECLARE_DYNAMIC_CLASS( CSelectFeatureDlg )
    DECLARE_EVENT_TABLE()

public:
    /// Constructors
    CSelectFeatureDlg();
    CSelectFeatureDlg( wxWindow* parent, wxWindowID id = SYMBOL_CSELECTFEATUREDLG_IDNAME, const wxString& caption = SYMBOL_CSELECTFEATUREDLG_TITLE, const wxPoint& pos = SYMBOL_CSELECTFEATUREDLG_POSITION, const wxSize& size = SYMBOL_CSELECTFEATUREDLG_SIZE, long style = SYMBOL_CSELECTFEATUREDLG_STYLE );

    /// Creation
    bool Create( wxWindow* parent, wxWindowID id = SYMBOL_CSELECTFEATUREDLG_IDNAME, const wxString& caption = SYMBOL_CSELECTFEATUREDLG_TITLE, const wxPoint& pos = SYMBOL_CSELECTFEATUREDLG_POSITION, const wxSize& size = SYMBOL_CSELECTFEATUREDLG_SIZE, long style = SYMBOL_CSELECTFEATUREDLG_STYLE );

    /// Destructor
    ~CSelectFeatureDlg();

    /// Initialises member variables
    void Init();

    /// Creates the controls and sizers
    void CreateControls();

////@begin CSelectFeatureDlg event handler declarations

    /// wxEVT_COMMAND_CHOICE_SELECTED event handler for ID_CHOICE5
    void OnFeatureTypeSelected( wxCommandEvent& event );

    /// wxEVT_COMMAND_LISTBOX_SELECTED event handler for ID_LISTBOX2
    void OnFeatureSelected( wxCommandEvent& event );

    /// wxEVT_COMMAND_LISTBOX_DOUBLECLICKED event handler for ID_LISTBOX2
    void OnFeatureDoubleClicked( wxCommandEvent& event );

    /// wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_BUTTON2
    void OnFeatureEdit( wxCommandEvent& event );

    /// wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_BUTTON3
    void OnButtonLoadFile( wxCommandEvent& event );

    /// wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_BUTTON4
    void OnButtonSaveFile( wxCommandEvent& event );

    /// wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_BUTTON1
    void OnLoadAccession( wxCommandEvent& event );

////@end CSelectFeatureDlg event handler declarations

////@begin CSelectFeatureDlg member function declarations

    int GetFeatureType() const { return m_FeatureType ; }
    void SetFeatureType(int value) { m_FeatureType = value ; }

    /// Retrieves bitmap resources
    wxBitmap GetBitmapResource( const wxString& name );

    /// Retrieves icon resources
    wxIcon GetIconResource( const wxString& name );
////@end CSelectFeatureDlg member function declarations

    /// Should we show tooltips?
    static bool ShowToolTips();

////@begin CSelectFeatureDlg member variables
    wxListBox* m_FeatureList;
    wxStaticText* m_LoadedStatus;
    wxButton* m_EditFeatureBtn;
    wxButton* m_SaveFileBtn;
    wxTextCtrl* m_AccInput;
private:
    int m_FeatureType;
////@end CSelectFeatureDlg member variables

    CRef<CObjectManager> m_ObjMgr;
    CRef<CScope> m_Scope;
    CBioseq_Handle m_BioseqHandle;

    void x_LoadFeatures();
};

END_NCBI_SCOPE

#endif  // GUI_DIALOGS_EDIT_DEMO___DELECT_FEATURE__HPP
