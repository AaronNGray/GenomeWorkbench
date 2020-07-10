/*  $Id: taxtree_testdlg.hpp 25629 2012-04-13 18:05:00Z katargir $
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
 * Authors:
 */
#ifndef _TAXTREE_TESTDLG_H_
#define _TAXTREE_TESTDLG_H_

#include <corelib/ncbistd.hpp>
#include <wx/dialog.h>

#include <objects/entrez2/entrez2_client.hpp>
#include <objmgr/object_manager.hpp>
#include <objmgr/scope.hpp>

/*!
 * Includes
 */

////@begin includes
#include "wx/treectrl.h"
#include "wx/statline.h"
////@end includes

/*!
 * Forward declarations
 */

////@begin forward declarations
class CTaxTreeBrowser;
////@end forward declarations

/*!
 * Control identifiers
 */

////@begin control identifiers
#define SYMBOL_CTAXTREETESTDLG_STYLE wxCAPTION|wxRESIZE_BORDER|wxSYSTEM_MENU|wxCLOSE_BOX|wxTAB_TRAVERSAL
#define SYMBOL_CTAXTREETESTDLG_TITLE _("Taxonomy Tree Test Dialog")
#define SYMBOL_CTAXTREETESTDLG_IDNAME ID_CTAXTREETESTDLG
#define SYMBOL_CTAXTREETESTDLG_SIZE wxSize(400, 300)
#define SYMBOL_CTAXTREETESTDLG_POSITION wxDefaultPosition
////@end control identifiers


BEGIN_NCBI_SCOPE
/*!
 * CTaxTreeTestDlg class declaration
 */

class CTaxTreeTestDlg: public wxDialog
{
    DECLARE_DYNAMIC_CLASS( CTaxTreeTestDlg )
    DECLARE_EVENT_TABLE()

public:
    /// Constructors
    CTaxTreeTestDlg();
    CTaxTreeTestDlg( wxWindow* parent, wxWindowID id = SYMBOL_CTAXTREETESTDLG_IDNAME, const wxString& caption = SYMBOL_CTAXTREETESTDLG_TITLE, const wxPoint& pos = SYMBOL_CTAXTREETESTDLG_POSITION, const wxSize& size = SYMBOL_CTAXTREETESTDLG_SIZE, long style = SYMBOL_CTAXTREETESTDLG_STYLE );

    /// Creation
    bool Create( wxWindow* parent, wxWindowID id = SYMBOL_CTAXTREETESTDLG_IDNAME, const wxString& caption = SYMBOL_CTAXTREETESTDLG_TITLE, const wxPoint& pos = SYMBOL_CTAXTREETESTDLG_POSITION, const wxSize& size = SYMBOL_CTAXTREETESTDLG_SIZE, long style = SYMBOL_CTAXTREETESTDLG_STYLE );

    /// Destructor
    ~CTaxTreeTestDlg();

    /// Initialises member variables
    void Init();

    /// Creates the controls and sizers
    void CreateControls();

////@begin CTaxTreeTestDlg event handler declarations

    /// wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_BUTTON1
    void OnSubmitClick( wxCommandEvent& event );

    /// wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_BUTTON2
    void OnRelatedClick( wxCommandEvent& event );

////@end CTaxTreeTestDlg event handler declarations

////@begin CTaxTreeTestDlg member function declarations

    /// Retrieves bitmap resources
    wxBitmap GetBitmapResource( const wxString& name );

    /// Retrieves icon resources
    wxIcon GetIconResource( const wxString& name );
////@end CTaxTreeTestDlg member function declarations

    /// Should we show tooltips?
    static bool ShowToolTips();

////@begin CTaxTreeTestDlg member variables
    wxTextCtrl* m_Input;
    CTaxTreeBrowser* m_Tree;
    /// Control identifiers
    enum {
        ID_CTAXTREETESTDLG = 10000,
        ID_TEXTCTRL1 = 10001,
        ID_BUTTON1 = 10002,
        ID_BUTTON2 = 10003,
        ID_TREECTRL = 10005
    };
////@end CTaxTreeTestDlg member variables
    CRef<objects::CEntrez2Client> m_Client;
    CRef<objects::CObjectManager> m_ObjMgr;
    CRef<objects::CScope> m_Scope;
    CRef<CTaxTreeDataSource> m_DataSource;
    objects::CEntrez2Client& x_GetClient();
};

END_NCBI_SCOPE

#endif
    // _TAXTREE_TESTDLG_H_
