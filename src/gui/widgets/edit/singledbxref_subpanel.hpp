/*  $Id: singledbxref_subpanel.hpp 25028 2012-01-12 17:26:13Z katargir $
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
 * Authors:  Colleen Bollin
 */
#ifndef _SINGLEDBXREF_SUBPANEL_H_
#define _SINGLEDBXREF_SUBPANEL_H_

#include <corelib/ncbistd.hpp>
#include <objects/general/Dbtag.hpp>

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
#define SYMBOL_CSINGLEDBXREF_SUBPANEL_STYLE wxCAPTION|wxRESIZE_BORDER|wxSYSTEM_MENU|wxCLOSE_BOX|wxTAB_TRAVERSAL
#define SYMBOL_CSINGLEDBXREF_SUBPANEL_TITLE _("SingleDbxrefSubPanel")
#define SYMBOL_CSINGLEDBXREF_SUBPANEL_IDNAME ID_CSINGLEDBXREF_SUBPANEL
#define SYMBOL_CSINGLEDBXREF_SUBPANEL_SIZE wxSize(400, 300)
#define SYMBOL_CSINGLEDBXREF_SUBPANEL_POSITION wxDefaultPosition
////@end control identifiers


/*!
 * CSingleDbxref_SubPanel class declaration
 */

class CSingleDbxref_SubPanel: public wxPanel
{    
    DECLARE_DYNAMIC_CLASS( CSingleDbxref_SubPanel )
    DECLARE_EVENT_TABLE()

public:
    /// Constructors
    CSingleDbxref_SubPanel();
    CSingleDbxref_SubPanel( wxWindow* parent, objects::CDbtag& tag, wxWindowID id = SYMBOL_CSINGLEDBXREF_SUBPANEL_IDNAME, const wxPoint& pos = SYMBOL_CSINGLEDBXREF_SUBPANEL_POSITION, const wxSize& size = SYMBOL_CSINGLEDBXREF_SUBPANEL_SIZE, long style = SYMBOL_CSINGLEDBXREF_SUBPANEL_STYLE );

    /// Creation
    bool Create( wxWindow* parent, wxWindowID id = SYMBOL_CSINGLEDBXREF_SUBPANEL_IDNAME, const wxPoint& pos = SYMBOL_CSINGLEDBXREF_SUBPANEL_POSITION, const wxSize& size = SYMBOL_CSINGLEDBXREF_SUBPANEL_SIZE, long style = SYMBOL_CSINGLEDBXREF_SUBPANEL_STYLE );

    /// Destructor
    ~CSingleDbxref_SubPanel();

    /// Initialises member variables
    void Init();

    /// Creates the controls and sizers
    void CreateControls();

    virtual bool TransferDataToWindow();
    virtual bool TransferDataFromWindow();
    CRef<objects::CDbtag> GetDbtag();

////@begin CSingleDbxref_SubPanel event handler declarations

    /// wxEVT_COMMAND_TEXT_UPDATED event handler for ID_TEXTCTRL32
    void OnTextctrl32TextUpdated( wxCommandEvent& event );

////@end CSingleDbxref_SubPanel event handler declarations

////@begin CSingleDbxref_SubPanel member function declarations

    /// Retrieves bitmap resources
    wxBitmap GetBitmapResource( const wxString& name );

    /// Retrieves icon resources
    wxIcon GetIconResource( const wxString& name );
////@end CSingleDbxref_SubPanel member function declarations

    /// Should we show tooltips?
    static bool ShowToolTips();

////@begin CSingleDbxref_SubPanel member variables
    wxTextCtrl* m_DbCtrl;
    wxTextCtrl* m_ObjectIdCtrl;
    /// Control identifiers
    enum {
        ID_CSINGLEDBXREF_SUBPANEL = 10093,
        ID_TEXTCTRL31 = 10094,
        ID_TEXTCTRL32 = 10095
    };
////@end CSingleDbxref_SubPanel member variables
private:
    CRef<objects::CDbtag> m_Tag;

};

END_NCBI_SCOPE

#endif
    // _SINGLEDBXREF_SUBPANEL_H_
