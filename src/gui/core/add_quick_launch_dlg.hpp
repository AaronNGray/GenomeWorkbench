#ifndef GUI_CORE___ADD_QUICK_LAUNCH_DLG__HPP
#define GUI_CORE___ADD_QUICK_LAUNCH_DLG__HPP

/*  $Id: add_quick_launch_dlg.hpp 26396 2012-09-11 19:41:55Z katargir $
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


/*!
 * Includes
 */

#include <corelib/ncbistd.hpp>
#include <gui/gui.hpp>

#include <gui/widgets/wx/dialog.hpp>

////@begin includes
////@end includes

/*!
 * Forward declarations
 */

////@begin forward declarations
////@end forward declarations

class wxTextCtrl;

BEGIN_NCBI_SCOPE

/*!
 * Control identifiers
 */

////@begin control identifiers
#define SYMBOL_CADDQUICKLAUNCHDLG_STYLE wxCAPTION|wxRESIZE_BORDER|wxSYSTEM_MENU|wxCLOSE_BOX|wxTAB_TRAVERSAL
#define SYMBOL_CADDQUICKLAUNCHDLG_TITLE _("Add Quick Launch Entry")
#define SYMBOL_CADDQUICKLAUNCHDLG_IDNAME ID_CADDQUICKLAUNCHDLG
#define SYMBOL_CADDQUICKLAUNCHDLG_SIZE wxSize(400, 300)
#define SYMBOL_CADDQUICKLAUNCHDLG_POSITION wxDefaultPosition
////@end control identifiers


/*!
 * CAddQuickLaunchDlg class declaration
 */

class CAddQuickLaunchDlg: public CDialog
{    
    DECLARE_DYNAMIC_CLASS( CAddQuickLaunchDlg )
    DECLARE_EVENT_TABLE()

public:
    CAddQuickLaunchDlg();
    CAddQuickLaunchDlg( wxWindow* parent, wxWindowID id = SYMBOL_CADDQUICKLAUNCHDLG_IDNAME, const wxString& caption = SYMBOL_CADDQUICKLAUNCHDLG_TITLE, const wxPoint& pos = SYMBOL_CADDQUICKLAUNCHDLG_POSITION, const wxSize& size = SYMBOL_CADDQUICKLAUNCHDLG_SIZE, long style = SYMBOL_CADDQUICKLAUNCHDLG_STYLE );

    bool Create( wxWindow* parent, wxWindowID id = SYMBOL_CADDQUICKLAUNCHDLG_IDNAME, const wxString& caption = SYMBOL_CADDQUICKLAUNCHDLG_TITLE, const wxPoint& pos = SYMBOL_CADDQUICKLAUNCHDLG_POSITION, const wxSize& size = SYMBOL_CADDQUICKLAUNCHDLG_SIZE, long style = SYMBOL_CADDQUICKLAUNCHDLG_STYLE );

    ~CAddQuickLaunchDlg();

    void Init();

    void CreateControls();

    virtual bool TransferDataFromWindow();

////@begin CAddQuickLaunchDlg event handler declarations

////@end CAddQuickLaunchDlg event handler declarations

////@begin CAddQuickLaunchDlg member function declarations

    string GetDescr() const { return m_Descr ; }
    void SetDescr(string value) { m_Descr = value ; }

    wxBitmap GetBitmapResource( const wxString& name );

    wxIcon GetIconResource( const wxString& name );
////@end CAddQuickLaunchDlg member function declarations

    static bool ShowToolTips();

////@begin CAddQuickLaunchDlg member variables
    wxTextCtrl* m_DescrCtrl;
private:
    string m_Descr;
    enum {
        ID_CADDQUICKLAUNCHDLG = 10113,
        ID_TEXTCTRL3 = 10114
    };
////@end CAddQuickLaunchDlg member variables
};

END_NCBI_SCOPE

#endif  // GUI_CORE___ADD_QUICK_LAUNCH_DLG__HPP
