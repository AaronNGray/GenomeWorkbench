/*  $Id: gbqualtext_panel.hpp 37148 2016-12-08 16:17:23Z filippov $
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
#ifndef _CGBQUALTEXTPANEL_H_
#define _CGBQUALTEXTPANEL_H_

#include <corelib/ncbistd.hpp>
#include <corelib/ncbiobj.hpp>
#include <gui/widgets/edit/formattedqual_panel.hpp>
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
#define SYMBOL_CGBQUALTEXTPANEL_STYLE wxCAPTION|wxRESIZE_BORDER|wxSYSTEM_MENU|wxCLOSE_BOX|wxTAB_TRAVERSAL
#define SYMBOL_CGBQUALTEXTPANEL_TITLE _("GBQualText Panel")
#define SYMBOL_CGBQUALTEXTPANEL_IDNAME ID_CGBQUALTEXTPANEL
#define SYMBOL_CGBQUALTEXTPANEL_SIZE wxSize(400, -1)
#define SYMBOL_CGBQUALTEXTPANEL_POSITION wxDefaultPosition
////@end control identifiers


/*!
 * CGBQualTextPanel class declaration
 */

class CGBQualTextPanel: public CFormattedQualPanel
{    
    DECLARE_DYNAMIC_CLASS( CGBQualTextPanel )
    DECLARE_EVENT_TABLE()

public:
    /// Constructors
    CGBQualTextPanel();
    CGBQualTextPanel( wxWindow* parent, wxWindowID id = SYMBOL_CGBQUALTEXTPANEL_IDNAME, const wxString& caption = SYMBOL_CGBQUALTEXTPANEL_TITLE, const wxPoint& pos = SYMBOL_CGBQUALTEXTPANEL_POSITION, const wxSize& size = SYMBOL_CGBQUALTEXTPANEL_SIZE, long style = SYMBOL_CGBQUALTEXTPANEL_STYLE );

    /// Creation
    bool Create( wxWindow* parent, wxWindowID id = SYMBOL_CGBQUALTEXTPANEL_IDNAME, const wxString& caption = SYMBOL_CGBQUALTEXTPANEL_TITLE, const wxPoint& pos = SYMBOL_CGBQUALTEXTPANEL_POSITION, const wxSize& size = SYMBOL_CGBQUALTEXTPANEL_SIZE, long style = SYMBOL_CGBQUALTEXTPANEL_STYLE );

    /// Destructor
    ~CGBQualTextPanel();

    /// Initialises member variables
    void Init();

    /// Creates the controls and sizers
    void CreateControls();

    virtual void SetValue(string val);
    virtual string GetValue();

////@begin CGBQualTextPanel event handler declarations

////@end CGBQualTextPanel event handler declarations

////@begin CGBQualTextPanel member function declarations

    /// Retrieves bitmap resources
    wxBitmap GetBitmapResource( const wxString& name );

    /// Retrieves icon resources
    wxIcon GetIconResource( const wxString& name );
////@end CGBQualTextPanel member function declarations

    /// Should we show tooltips?
    static bool ShowToolTips();

////@begin CGBQualTextPanel member variables
    wxTextCtrl* m_theValue;
    /// Control identifiers
    enum {
        ID_CGBQUALTEXTPANEL = 10047,
        ID_TEXTCTRL = 10049
    };
////@end CGBQualTextPanel member variables
};

END_NCBI_SCOPE

#endif
    // _CGBQUALTEXTPANEL_H_
