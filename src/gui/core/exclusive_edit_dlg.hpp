#ifndef GUI_CORE___EXCLUSIVE_EDIT_DLG_HPP
#define GUI_CORE___EXCLUSIVE_EDIT_DLG_HPP

/*  $Id: exclusive_edit_dlg.hpp 34316 2015-12-11 19:55:46Z katargir $
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

class wxStaticText;

BEGIN_NCBI_SCOPE

/*!
 * Control identifiers
 */

////@begin control identifiers
#define SYMBOL_CEXCLUSIVEEDITDLG_STYLE wxCAPTION|wxRESIZE_BORDER|wxSYSTEM_MENU|wxCLOSE_BOX|wxTAB_TRAVERSAL
#define SYMBOL_CEXCLUSIVEEDITDLG_TITLE _("Exclusive Edit Access")
#define SYMBOL_CEXCLUSIVEEDITDLG_IDNAME ID_CEXCLUSIVEEDITDLG
#define SYMBOL_CEXCLUSIVEEDITDLG_SIZE wxSize(400, 300)
#define SYMBOL_CEXCLUSIVEEDITDLG_POSITION wxDefaultPosition
////@end control identifiers


/*!
 * CExclusiveEditDlg class declaration
 */

class CExclusiveEditDlg: public wxDialog
{    
    DECLARE_DYNAMIC_CLASS( CExclusiveEditDlg )
    DECLARE_EVENT_TABLE()

public:
    CExclusiveEditDlg();
    CExclusiveEditDlg( wxWindow* parent, wxWindowID id = SYMBOL_CEXCLUSIVEEDITDLG_IDNAME, const wxString& caption = SYMBOL_CEXCLUSIVEEDITDLG_TITLE, const wxPoint& pos = SYMBOL_CEXCLUSIVEEDITDLG_POSITION, const wxSize& size = SYMBOL_CEXCLUSIVEEDITDLG_SIZE, long style = SYMBOL_CEXCLUSIVEEDITDLG_STYLE );

    bool Create( wxWindow* parent, wxWindowID id = SYMBOL_CEXCLUSIVEEDITDLG_IDNAME, const wxString& caption = SYMBOL_CEXCLUSIVEEDITDLG_TITLE, const wxPoint& pos = SYMBOL_CEXCLUSIVEEDITDLG_POSITION, const wxSize& size = SYMBOL_CEXCLUSIVEEDITDLG_SIZE, long style = SYMBOL_CEXCLUSIVEEDITDLG_STYLE );

    ~CExclusiveEditDlg();

    void Init();

    void InitData(wxWindow* exclusiveEdit, const string& exclusiveDescr)
    {
        m_ExclusiveEdit = exclusiveEdit;
        m_ExclusiveDescr = exclusiveDescr;
    }

    void CreateControls();

////@begin CExclusiveEditDlg event handler declarations

    void OnActivateWindowClick( wxCommandEvent& event );

////@end CExclusiveEditDlg event handler declarations

////@begin CExclusiveEditDlg member function declarations

    wxBitmap GetBitmapResource( const wxString& name );

    wxIcon GetIconResource( const wxString& name );
////@end CExclusiveEditDlg member function declarations

    static bool ShowToolTips();

////@begin CExclusiveEditDlg member variables
    wxStaticText* m_TextCtrl;
    enum {
        ID_CEXCLUSIVEEDITDLG = 10058,
        wxID_WINDOW_DESCRIPTION = 10060,
        ID_ACTIVATE_WINDOW = 10059
    };
////@end CExclusiveEditDlg member variables

    wxWindow* m_ExclusiveEdit;
    string    m_ExclusiveDescr;
};

END_NCBI_SCOPE

#endif  // GUI_CORE___EXCLUSIVE_EDIT_DLG_HPP
