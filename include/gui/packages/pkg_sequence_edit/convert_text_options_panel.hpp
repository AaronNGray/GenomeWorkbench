/*  $Id: convert_text_options_panel.hpp 30522 2014-06-06 18:47:41Z asztalos $
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
 * Authors:  Andrea Asztalos
 */
#ifndef _CONVERT_TEXT_OPTIONS_PANEL_H_
#define _CONVERT_TEXT_OPTIONS_PANEL_H_

#include <corelib/ncbistd.hpp>
#include <wx/panel.h>
#include <wx/checkbox.h>


BEGIN_NCBI_SCOPE
/*!
 * Forward declarations
 */

////@begin forward declarations
class CCapChangePanel;
////@end forward declarations


/*!
 * Control identifiers
 */

////@begin control identifiers
#define ID_CCONVERTTEXTOPTIONSPANEL 10364
#define ID_LVORIG_CHKBX 10365
#define ID_RMVNAME_CHKBX 10366
#define ID_CAPCHNGPANEL 10367
#define SYMBOL_CCONVERTTEXTOPTIONSPANEL_STYLE wxTAB_TRAVERSAL
#define SYMBOL_CCONVERTTEXTOPTIONSPANEL_TITLE _("ConvertTextOptionsPanel")
#define SYMBOL_CCONVERTTEXTOPTIONSPANEL_IDNAME ID_CCONVERTTEXTOPTIONSPANEL
#define SYMBOL_CCONVERTTEXTOPTIONSPANEL_SIZE wxSize(400, 300)
#define SYMBOL_CCONVERTTEXTOPTIONSPANEL_POSITION wxDefaultPosition
////@end control identifiers


/*!
 * CConvertTextOptionsPanel class declaration
 */

class CConvertTextOptionsPanel: public wxPanel
{    
    DECLARE_DYNAMIC_CLASS( CConvertTextOptionsPanel )
    DECLARE_EVENT_TABLE()

public:
    /// Constructors
    CConvertTextOptionsPanel();
    CConvertTextOptionsPanel( wxWindow* parent, 
        wxWindowID id = SYMBOL_CCONVERTTEXTOPTIONSPANEL_IDNAME, 
        const wxPoint& pos = SYMBOL_CCONVERTTEXTOPTIONSPANEL_POSITION, 
        const wxSize& size = SYMBOL_CCONVERTTEXTOPTIONSPANEL_SIZE, 
        long style = SYMBOL_CCONVERTTEXTOPTIONSPANEL_STYLE );

    /// Creation
    bool Create( wxWindow* parent, 
        wxWindowID id = SYMBOL_CCONVERTTEXTOPTIONSPANEL_IDNAME, 
        const wxPoint& pos = SYMBOL_CCONVERTTEXTOPTIONSPANEL_POSITION, 
        const wxSize& size = SYMBOL_CCONVERTTEXTOPTIONSPANEL_SIZE, 
        long style = SYMBOL_CCONVERTTEXTOPTIONSPANEL_STYLE );

    /// Destructor
    ~CConvertTextOptionsPanel();

    /// Initialises member variables
    void Init();

    /// Creates the controls and sizers
    void CreateControls();

////@begin CConvertTextOptionsPanel event handler declarations

////@end CConvertTextOptionsPanel event handler declarations

////@begin CConvertTextOptionsPanel member function declarations

    /// Retrieves bitmap resources
    wxBitmap GetBitmapResource( const wxString& name );

    /// Retrieves icon resources
    wxIcon GetIconResource( const wxString& name );
////@end CConvertTextOptionsPanel member function declarations

    /// Should we show tooltips?
    static bool ShowToolTips();

////@begin CConvertTextOptionsPanel member variables
    wxCheckBox* m_LeaveOriginal;
    wxCheckBox* m_StripNameFromText;
    CCapChangePanel* m_CapChangeOptions;
////@end CConvertTextOptionsPanel member variables
};

END_NCBI_SCOPE

#endif
    // _CONVERT_TEXT_OPTIONS_PANEL_H_
