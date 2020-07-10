#ifndef GUI_CORE___SAVE_SMART_OBJECTS_DLG_HPP
#define GUI_CORE___SAVE_SMART_OBJECTS_DLG_HPP

/*  $Id: save_smart_objects_dlg.hpp 28638 2013-08-08 19:42:37Z katargir $
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

#include <gui/widgets/wx/dialog.hpp>

#include <wx/listbox.h>

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
#define SYMBOL_CSAVESMARTOBJECTSDLG_STYLE wxCAPTION|wxRESIZE_BORDER|wxSYSTEM_MENU|wxCLOSE_BOX|wxTAB_TRAVERSAL
#define SYMBOL_CSAVESMARTOBJECTSDLG_TITLE _("Save Smart objects")
#define SYMBOL_CSAVESMARTOBJECTSDLG_IDNAME ID_CSAVESMARTOBJECTSDLG
#define SYMBOL_CSAVESMARTOBJECTSDLG_SIZE wxSize(400, 300)
#define SYMBOL_CSAVESMARTOBJECTSDLG_POSITION wxDefaultPosition
////@end control identifiers


/*!
 * CSaveSmartObjectsDlg class declaration
 */

class CSaveSmartObjectsDlg: public CDialog
{    
    DECLARE_DYNAMIC_CLASS( CSaveSmartObjectsDlg )
    DECLARE_EVENT_TABLE()

public:
    CSaveSmartObjectsDlg();
    CSaveSmartObjectsDlg( wxWindow* parent, wxWindowID id = SYMBOL_CSAVESMARTOBJECTSDLG_IDNAME, const wxString& caption = SYMBOL_CSAVESMARTOBJECTSDLG_TITLE, const wxPoint& pos = SYMBOL_CSAVESMARTOBJECTSDLG_POSITION, const wxSize& size = SYMBOL_CSAVESMARTOBJECTSDLG_SIZE, long style = SYMBOL_CSAVESMARTOBJECTSDLG_STYLE );

    bool Create( wxWindow* parent, wxWindowID id = SYMBOL_CSAVESMARTOBJECTSDLG_IDNAME, const wxString& caption = SYMBOL_CSAVESMARTOBJECTSDLG_TITLE, const wxPoint& pos = SYMBOL_CSAVESMARTOBJECTSDLG_POSITION, const wxSize& size = SYMBOL_CSAVESMARTOBJECTSDLG_SIZE, long style = SYMBOL_CSAVESMARTOBJECTSDLG_STYLE );

    ~CSaveSmartObjectsDlg();

    void Init();

    void CreateControls();

////@begin CSaveSmartObjectsDlg event handler declarations

////@end CSaveSmartObjectsDlg event handler declarations

////@begin CSaveSmartObjectsDlg member function declarations

    wxBitmap GetBitmapResource( const wxString& name );

    wxIcon GetIconResource( const wxString& name );
////@end CSaveSmartObjectsDlg member function declarations

    static bool ShowToolTips();

////@begin CSaveSmartObjectsDlg member variables
    wxListBox* m_ProjectListrCtrl;
    enum {
        ID_CSAVESMARTOBJECTSDLG = 10038,
        ID_LISTBOX = 10049
    };
////@end CSaveSmartObjectsDlg member variables
};

END_NCBI_SCOPE


#endif  // GUI_CORE___SAVE_SMART_OBJECTS_DLG_HPP
