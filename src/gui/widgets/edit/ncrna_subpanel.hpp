/*  $Id: ncrna_subpanel.hpp 25028 2012-01-12 17:26:13Z katargir $
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
 * Authors:  Roman Katargin
 */
#ifndef _NCRNA_SUBPANEL_H_
#define _NCRNA_SUBPANEL_H_

#include <corelib/ncbistd.hpp>

/*!
 * Includes
 */

////@begin includes
////@end includes

#include <wx/panel.h>
#include <wx/frame.h>
#include <wx/sizer.h>
#include <wx/bitmap.h>
#include <wx/icon.h>
#include <wx/choice.h>
#include <wx/textctrl.h>
#include <wx/stattext.h>
#include <wx/valtext.h>

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
#define SYMBOL_CNCRNASUBPANEL_STYLE wxCAPTION|wxRESIZE_BORDER|wxSYSTEM_MENU|wxCLOSE_BOX|wxTAB_TRAVERSAL
#define SYMBOL_CNCRNASUBPANEL_TITLE _("ncRNA SubPanel")
#define SYMBOL_CNCRNASUBPANEL_IDNAME ID_CNCRNASUBPANEL
#define SYMBOL_CNCRNASUBPANEL_SIZE wxSize(400, 300)
#define SYMBOL_CNCRNASUBPANEL_POSITION wxDefaultPosition
////@end control identifiers


/*!
 * CncRNASubPanel class declaration
 */

class CncRNASubPanel: public wxPanel
{    
    DECLARE_DYNAMIC_CLASS( CncRNASubPanel )
    DECLARE_EVENT_TABLE()

public:
    /// Constructors
    CncRNASubPanel();
    CncRNASubPanel( wxWindow* parent, CRef<objects::CRNA_gen> gen, wxWindowID id = SYMBOL_CNCRNASUBPANEL_IDNAME, const wxString& caption = SYMBOL_CNCRNASUBPANEL_TITLE, const wxPoint& pos = SYMBOL_CNCRNASUBPANEL_POSITION, const wxSize& size = SYMBOL_CNCRNASUBPANEL_SIZE, long style = SYMBOL_CNCRNASUBPANEL_STYLE );

    /// Creation
    bool Create( wxWindow* parent, wxWindowID id = SYMBOL_CNCRNASUBPANEL_IDNAME, const wxString& caption = SYMBOL_CNCRNASUBPANEL_TITLE, const wxPoint& pos = SYMBOL_CNCRNASUBPANEL_POSITION, const wxSize& size = SYMBOL_CNCRNASUBPANEL_SIZE, long style = SYMBOL_CNCRNASUBPANEL_STYLE );

    /// Destructor
    ~CncRNASubPanel();

    /// Initialises member variables
    void Init();

    /// Creates the controls and sizers
    void CreateControls();

    virtual bool TransferDataToWindow();
    virtual bool TransferDataFromWindow();

////@begin CncRNASubPanel event handler declarations

    /// wxEVT_COMMAND_CHOICE_SELECTED event handler for ID_CHOICE5
    void OnChoice5Selected( wxCommandEvent& event );

////@end CncRNASubPanel event handler declarations

////@begin CncRNASubPanel member function declarations

    /// Retrieves bitmap resources
    wxBitmap GetBitmapResource( const wxString& name );

    /// Retrieves icon resources
    wxIcon GetIconResource( const wxString& name );
////@end CncRNASubPanel member function declarations

    /// Should we show tooltips?
    static bool ShowToolTips();

////@begin CncRNASubPanel member variables
    wxChoice* m_ClassCtrl;
    wxTextCtrl* m_OtherClass;
    wxTextCtrl* m_ProductCtrl;
    /// Control identifiers
    enum {
        ID_CNCRNASUBPANEL = 10050,
        ID_CHOICE5 = 10052,
        ID_TEXTCTRL11 = 10051,
        ID_TEXTCTRL13 = 10053
    };
////@end CncRNASubPanel member variables

private:
    CRef<objects::CRNA_gen> m_Gen;

};

END_NCBI_SCOPE

#endif
    // _NCRNA_SUBPANEL_H_
