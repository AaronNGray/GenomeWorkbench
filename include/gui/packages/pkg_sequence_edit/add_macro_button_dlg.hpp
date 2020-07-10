/*  $Id: add_macro_button_dlg.hpp 40432 2018-02-12 21:50:50Z filippov $
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
 * Authors:  Igor Filippov
 */

#ifndef _CADDMACROBUTTON_H_
#define _CADDMACROBUTTON_H_
#include <corelib/ncbistd.hpp>
#include <gui/gui_export.h>
/*!
 * Includes
 */

////@begin includes
#include "wx/filepicker.h"
////@end includes

/*!
 * Forward declarations
 */

////@begin forward declarations
class wxFilePickerCtrl;
////@end forward declarations

BEGIN_NCBI_SCOPE
/*!
 * Control identifiers
 */

////@begin control identifiers
#define ID_CADDMACROBUTTON 10000
#define SYMBOL_CADDMACROBUTTON_STYLE wxCAPTION|wxRESIZE_BORDER|wxSYSTEM_MENU|wxCLOSE_BOX|wxTAB_TRAVERSAL
#define SYMBOL_CADDMACROBUTTON_TITLE _("Add Macro Button")
#define SYMBOL_CADDMACROBUTTON_IDNAME ID_CADDMACROBUTTON
#define SYMBOL_CADDMACROBUTTON_SIZE wxSize(400, 300)
#define SYMBOL_CADDMACROBUTTON_POSITION wxDefaultPosition
////@end control identifiers


/*!
 * CAddMacroButton class declaration
 */

class NCBI_GUIPKG_SEQUENCE_EDIT_EXPORT CAddMacroButton: public wxDialog
{    
    DECLARE_DYNAMIC_CLASS( CAddMacroButton )
    DECLARE_EVENT_TABLE()

    enum EControlIds
    {
	ID_TEXTCTRL = 22001,
	ID_FILECTRL,
	ID_CHECKBOX,
	ID_TEXTCTRL1 
    };
public:
    /// Constructors
    CAddMacroButton();
    CAddMacroButton( wxWindow* parent, wxWindowID id = SYMBOL_CADDMACROBUTTON_IDNAME, const wxString& caption = SYMBOL_CADDMACROBUTTON_TITLE, const wxPoint& pos = SYMBOL_CADDMACROBUTTON_POSITION, const wxSize& size = SYMBOL_CADDMACROBUTTON_SIZE, long style = SYMBOL_CADDMACROBUTTON_STYLE );

    /// Creation
    bool Create( wxWindow* parent, wxWindowID id = SYMBOL_CADDMACROBUTTON_IDNAME, const wxString& caption = SYMBOL_CADDMACROBUTTON_TITLE, const wxPoint& pos = SYMBOL_CADDMACROBUTTON_POSITION, const wxSize& size = SYMBOL_CADDMACROBUTTON_SIZE, long style = SYMBOL_CADDMACROBUTTON_STYLE );

    /// Destructor
    ~CAddMacroButton();

    /// Initialises member variables
    void Init();

    /// Creates the controls and sizers
    void CreateControls();

////@begin CAddMacroButton event handler declarations
    void OnReportRequireCheck(wxCommandEvent&);
    void OnFileSelected(wxFileDirPickerEvent& event);
////@end CAddMacroButton event handler declarations

////@begin CAddMacroButton member function declarations

    /// Retrieves bitmap resources
    wxBitmap GetBitmapResource( const wxString& name );

    /// Retrieves icon resources
    wxIcon GetIconResource( const wxString& name );
////@end CAddMacroButton member function declarations

    /// Should we show tooltips?
    static bool ShowToolTips();

    wxString GetName() {return m_ButtonName->GetValue();}
    string GetFileName() {return m_FileName->GetPath().ToStdString();}
    bool GetReportRequred() {return m_ReportRequired->GetValue();}
    string GetTitle() {return m_Title->GetValue().ToStdString();}

private:
////@begin CAddMacroButton member variables
    wxTextCtrl* m_ButtonName;
    wxFilePickerCtrl* m_FileName;
    wxCheckBox* m_ReportRequired;
    wxTextCtrl* m_Title;
////@end CAddMacroButton member variables
};

END_NCBI_SCOPE

#endif
    // _CADDMACROBUTTON_H_
