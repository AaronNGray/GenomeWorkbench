/*  $Id: rename_column_dlg.hpp 27947 2013-04-29 20:11:18Z filippov $
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
 * Authors: Igor Filippov
 */
#ifndef _RENAME_COLUMN_DLG_H_
#define _RENAME_COLUMN_DLG_H_

#include <corelib/ncbistd.hpp>
#include <wx/dialog.h>
#include <wx/choice.h>

/*!
 * Includes
 */

////@begin includes
////@end includes
#include <wx/sizer.h>

/*!
 * Forward declarations
 */

////@begin forward declarations
class wxBoxSizer;
////@end forward declarations

BEGIN_NCBI_SCOPE

/*!
 * Control identifiers
 */

////@begin control identifiers
#define ID_RENAME_COLUMN_DLG wxID_ANY
#define SYMBOL_CRENAME_COLUMN_DLG_STYLE wxCAPTION|wxRESIZE_BORDER|wxSYSTEM_MENU|wxCLOSE_BOX|wxTAB_TRAVERSAL
#define SYMBOL_CRENAME_COLUMN_DLG_TITLE _("Rename Column Dialog")
#define SYMBOL_CRENAME_COLUMN_DLG_IDNAME ID_RENAME_COLUMN_DLG
#define SYMBOL_CRENAME_COLUMN_DLG_SIZE wxSize(200,100)
#define SYMBOL_CRENAME_COLUMN_DLG_POSITION wxDefaultPosition
////@end control identifiers




class CRenameColumnDlg: public wxDialog
{    
    DECLARE_DYNAMIC_CLASS( CRenameColumnDlg )
    DECLARE_EVENT_TABLE()

public:
    /// Constructors
    CRenameColumnDlg();
    CRenameColumnDlg( wxWindow* parent, 
                      wxArrayString& srcModNameStrings,
                      wxWindowID id = SYMBOL_CRENAME_COLUMN_DLG_IDNAME, 
                      const wxString& caption = SYMBOL_CRENAME_COLUMN_DLG_TITLE, 
                      const wxPoint& pos = SYMBOL_CRENAME_COLUMN_DLG_POSITION, 
                      const wxSize& size = SYMBOL_CRENAME_COLUMN_DLG_SIZE, 
                      long style = SYMBOL_CRENAME_COLUMN_DLG_STYLE );

    /// Creation
    bool Create( wxWindow* parent, 
		 wxWindowID id = SYMBOL_CRENAME_COLUMN_DLG_IDNAME, 
		 const wxString& caption = SYMBOL_CRENAME_COLUMN_DLG_TITLE, 
		 const wxPoint& pos = SYMBOL_CRENAME_COLUMN_DLG_POSITION, 
		 const wxSize& size = SYMBOL_CRENAME_COLUMN_DLG_SIZE, 
		 long style = SYMBOL_CRENAME_COLUMN_DLG_STYLE );

    /// Destructor
    ~CRenameColumnDlg();

    /// Initialises member variables
    void Init();

    /// Creates the controls and sizers
    void CreateControls();



    /// Retrieves bitmap resources
    wxBitmap GetBitmapResource( const wxString& name );

    /// Retrieves icon resources
    wxIcon GetIconResource( const wxString& name );

    /// Should we show tooltips?
    static bool ShowToolTips();

    wxString GetSelection();

private:
    wxArrayString m_srcModNameStrings;
    wxChoice* m_QualList;
};

END_NCBI_SCOPE

#endif
    // _RENAME_COLUMN_DLG_H_
