#ifndef GUI_CORE___PROJECT_LOCKED_DLG_HPP
#define GUI_CORE___PROJECT_LOCKED_DLG_HPP

/*  $Id: project_locked_dlg.hpp 31530 2014-10-20 19:53:58Z katargir $
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
 * Authors:  Roman Katargin
 *
 * File Description:
 *
 */

#include <corelib/ncbistd.hpp>

#include <wx/dialog.h>

/*!
 * Includes
 */

////@begin includes
////@end includes

/*!
 * Forward declarations
 */

////@begin forward declarations
////@end forward declarations

BEGIN_NCBI_SCOPE

/*!
 * Control identifiers
 */

////@begin control identifiers
#define SYMBOL_CPROJECTLOCKEDDLG_STYLE wxCAPTION|wxTAB_TRAVERSAL
#define SYMBOL_CPROJECTLOCKEDDLG_TITLE _("Project Access Error")
#define SYMBOL_CPROJECTLOCKEDDLG_IDNAME ID_CPROJECTLOCKEDDLG
#define SYMBOL_CPROJECTLOCKEDDLG_SIZE wxSize(400, 300)
#define SYMBOL_CPROJECTLOCKEDDLG_POSITION wxDefaultPosition
////@end control identifiers


/*!
 * CProjectLockedDlg class declaration
 */

class CProjectLockedDlg: public wxDialog
{    
    DECLARE_DYNAMIC_CLASS( CProjectLockedDlg )
    DECLARE_EVENT_TABLE()

public:
    CProjectLockedDlg();
    CProjectLockedDlg( wxWindow* parent, wxWindowID id = SYMBOL_CPROJECTLOCKEDDLG_IDNAME, const wxString& caption = SYMBOL_CPROJECTLOCKEDDLG_TITLE, const wxPoint& pos = SYMBOL_CPROJECTLOCKEDDLG_POSITION, const wxSize& size = SYMBOL_CPROJECTLOCKEDDLG_SIZE, long style = SYMBOL_CPROJECTLOCKEDDLG_STYLE );

    bool Create( wxWindow* parent, wxWindowID id = SYMBOL_CPROJECTLOCKEDDLG_IDNAME, const wxString& caption = SYMBOL_CPROJECTLOCKEDDLG_TITLE, const wxPoint& pos = SYMBOL_CPROJECTLOCKEDDLG_POSITION, const wxSize& size = SYMBOL_CPROJECTLOCKEDDLG_SIZE, long style = SYMBOL_CPROJECTLOCKEDDLG_STYLE );

    ~CProjectLockedDlg();

    void Init();

    void CreateControls();

////@begin CProjectLockedDlg event handler declarations

////@end CProjectLockedDlg event handler declarations

////@begin CProjectLockedDlg member function declarations

    wxBitmap GetBitmapResource( const wxString& name );

    wxIcon GetIconResource( const wxString& name );
////@end CProjectLockedDlg member function declarations

    static bool ShowToolTips();

////@begin CProjectLockedDlg member variables
    enum {
        ID_CPROJECTLOCKEDDLG = 10057
    };
////@end CProjectLockedDlg member variables
};

END_NCBI_SCOPE

#endif  // GUI_CORE___PROJECT_LOCKED_DLG_HPP
