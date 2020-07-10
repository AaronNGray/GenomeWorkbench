#ifndef GUI_FRAMEWORK___APP_OPTIONS_DLG__HPP
#define GUI_FRAMEWORK___APP_OPTIONS_DLG__HPP

/*  $Id: app_options_dlg.hpp 25477 2012-03-27 14:53:29Z kuznets $
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
#include "wx/notebook.h"
#include "wx/statline.h"
////@end includes

////@begin forward declarations
////@end forward declarations

BEGIN_NCBI_SCOPE

////@begin control identifiers
#define SYMBOL_CAPPOPTIONSDLG_STYLE wxCAPTION|wxRESIZE_BORDER|wxSYSTEM_MENU|wxCLOSE_BOX|wxTAB_TRAVERSAL
#define SYMBOL_CAPPOPTIONSDLG_TITLE wxT("App Options")
#define SYMBOL_CAPPOPTIONSDLG_IDNAME ID_CAPPOPTIONSDLG
#define SYMBOL_CAPPOPTIONSDLG_SIZE wxSize(400, 300)
#define SYMBOL_CAPPOPTIONSDLG_POSITION wxDefaultPosition
////@end control identifiers

class NCBI_GUIFRAMEWORK_EXPORT CAppOptionsDlg: public CDialog
{
    DECLARE_DYNAMIC_CLASS( CAppOptionsDlg )
    DECLARE_EVENT_TABLE()

public:
    CAppOptionsDlg();
    CAppOptionsDlg( wxWindow* parent,
                    wxWindowID id = SYMBOL_CAPPOPTIONSDLG_IDNAME,
                    const wxString& caption = SYMBOL_CAPPOPTIONSDLG_TITLE,
                    const wxPoint& pos = SYMBOL_CAPPOPTIONSDLG_POSITION,
                    const wxSize& size = SYMBOL_CAPPOPTIONSDLG_SIZE,
                    long style = SYMBOL_CAPPOPTIONSDLG_STYLE );

    bool Create( wxWindow* parent,
                 wxWindowID id = SYMBOL_CAPPOPTIONSDLG_IDNAME,
                 const wxString& caption = SYMBOL_CAPPOPTIONSDLG_TITLE,
                 const wxPoint& pos = SYMBOL_CAPPOPTIONSDLG_POSITION,
                 const wxSize& size = SYMBOL_CAPPOPTIONSDLG_SIZE,
                 long style = SYMBOL_CAPPOPTIONSDLG_STYLE );

    ~CAppOptionsDlg();

    void Init();

    void CreateControls();

////@begin CAppOptionsDlg event handler declarations

////@end CAppOptionsDlg event handler declarations

////@begin CAppOptionsDlg member function declarations

    wxBitmap GetBitmapResource( const wxString& name );

    wxIcon GetIconResource( const wxString& name );
////@end CAppOptionsDlg member function declarations

    static bool ShowToolTips();

////@begin CAppOptionsDlg member variables
    enum {
        ID_CAPPOPTIONSDLG = 10002,
        ID_NOTEBOOK1 = 10003
    };
////@end CAppOptionsDlg member variables
};

END_NCBI_SCOPE

/* @} */

#endif  // GUI_FRAMEWORK___APP_OPTIONS_DLG__HPP
