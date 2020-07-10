#ifndef GUI_FRAMEWORK___APP_PACKAGES_DLG__HPP
#define GUI_FRAMEWORK___APP_PACKAGES_DLG__HPP

/*  $Id: app_packages_dlg.hpp 43083 2019-05-13 19:26:43Z katargir $
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

/** @addtogroup GUI_FRAMEWORK
*
* @{
*/

#include <corelib/ncbiobj.hpp>
#include <gui/gui_export.h>

#include <gui/widgets/wx/dialog.hpp>

////@begin includes
#include "wx/listctrl.h"
#include "wx/statline.h"
////@end includes

#include "wx/checklst.h"
#include "wx/stattext.h"

#include <set>

////@begin forward declarations
class wxListCtrl;
////@end forward declarations

class wxCheckBox;
class wxTextOutputStream;

BEGIN_NCBI_SCOPE

////@begin control identifiers
#define SYMBOL_CAPPPACKAGESDLG_STYLE wxCAPTION|wxRESIZE_BORDER|wxSYSTEM_MENU|wxCLOSE_BOX|wxTAB_TRAVERSAL
#define SYMBOL_CAPPPACKAGESDLG_TITLE _("Configure Packages")
#define SYMBOL_CAPPPACKAGESDLG_IDNAME ID_CAPPPACKAGESDLG
#define SYMBOL_CAPPPACKAGESDLG_SIZE wxSize(400, 300)
#define SYMBOL_CAPPPACKAGESDLG_POSITION wxDefaultPosition
////@end control identifiers


class CPkgManager;
class CPackageInfo;

class NCBI_GUIFRAMEWORK_EXPORT CAppPackagesDlg: public CDialog
{
    DECLARE_DYNAMIC_CLASS( CAppPackagesDlg )
    DECLARE_EVENT_TABLE()

public:
    CAppPackagesDlg();
    CAppPackagesDlg( wxWindow* parent,
                     CPkgManager* pkgManager,
                     wxWindowID id = SYMBOL_CAPPPACKAGESDLG_IDNAME,
                     const wxString& caption = SYMBOL_CAPPPACKAGESDLG_TITLE,
                     const wxPoint& pos = SYMBOL_CAPPPACKAGESDLG_POSITION,
                     const wxSize& size = SYMBOL_CAPPPACKAGESDLG_SIZE,
                     long style = SYMBOL_CAPPPACKAGESDLG_STYLE );

    bool Create( wxWindow* parent,
                 wxWindowID id = SYMBOL_CAPPPACKAGESDLG_IDNAME,
                 const wxString& caption = SYMBOL_CAPPPACKAGESDLG_TITLE,
                 const wxPoint& pos = SYMBOL_CAPPPACKAGESDLG_POSITION,
                 const wxSize& size = SYMBOL_CAPPPACKAGESDLG_SIZE,
                 long style = SYMBOL_CAPPPACKAGESDLG_STYLE );

    ~CAppPackagesDlg();

    void Init();

    void CreateControls();

////@begin CAppPackagesDlg event handler declarations

    void OnPackageSelected( wxListEvent& event );

    void OnShowInvalidPkgs( wxCommandEvent& event );

    void OnEnablePkg( wxCommandEvent& event );

    void OnOkClick( wxCommandEvent& event );

////@end CAppPackagesDlg event handler declarations

////@begin CAppPackagesDlg member function declarations

    wxBitmap GetBitmapResource( const wxString& name );

    wxIcon GetIconResource( const wxString& name );
////@end CAppPackagesDlg member function declarations

    static bool ShowToolTips();

////@begin CAppPackagesDlg member variables
    wxListCtrl* m_PkgList;
    wxTextCtrl* m_PkgInfo;
    wxCheckBox* m_PkgEnable;
    enum {
        ID_CAPPPACKAGESDLG = 10004,
        ID_LISTCTRL1 = 10006,
        ID_CHECKBOX1 = 10007,
        ID_TEXTCTRL1 = 10005,
        ID_CHECKBOX2 = 10008
    };
////@end CAppPackagesDlg member variables

protected:
    CPkgManager* m_PkgManager;
    set<string> m_EnabledPkgs;
    string m_CurPkg;

    void x_LoadPkgList(bool firstTime = false);
    static void x_DumpPkgInfo(wxTextOutputStream& os, const string& name, const CPackageInfo& pkg_info);
};

END_NCBI_SCOPE

/* @} */

#endif  // GUI_FRAMEWORK___APP_PACKAGES_DLG__HPP
