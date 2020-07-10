/*  $Id: gbqual_semicontrolled.hpp 37148 2016-12-08 16:17:23Z filippov $
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
#ifndef _GBQUAL_SEMICONTROLLED_H_
#define _GBQUAL_SEMICONTROLLED_H_

#include <corelib/ncbistd.hpp>
#include <corelib/ncbiobj.hpp>
#include <gui/widgets/edit/formattedqual_panel.hpp>

#include <wx/panel.h>
#include <wx/frame.h>
#include <wx/sizer.h>
#include <wx/bitmap.h>
#include <wx/icon.h>
#include <wx/combobox.h>

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
#define SYMBOL_CGBQUALSEMICONTROLLEDTEXTPANEL_STYLE wxCAPTION|wxRESIZE_BORDER|wxSYSTEM_MENU|wxTAB_TRAVERSAL
#define SYMBOL_CGBQUALSEMICONTROLLEDTEXTPANEL_TITLE _("GBQualText Panel")
#define SYMBOL_CGBQUALSEMICONTROLLEDTEXTPANEL_IDNAME ID_CGBQUALTEXTPANEL
#define SYMBOL_CGBQUALSEMICONTROLLEDTEXTPANEL_SIZE wxDefaultSize
#define SYMBOL_CGBQUALSEMICONTROLLEDTEXTPANEL_POSITION wxDefaultPosition
////@end control identifiers


/*!
 * CGBQualSemicontrolledTextPanel class declaration
 */

class CGBQualSemicontrolledTextPanel: public CFormattedQualPanel
{    
    DECLARE_DYNAMIC_CLASS( CGBQualSemicontrolledTextPanel )
    DECLARE_EVENT_TABLE()

public:
    /// Constructors
    CGBQualSemicontrolledTextPanel();
    CGBQualSemicontrolledTextPanel( wxWindow* parent, wxWindowID id = SYMBOL_CGBQUALSEMICONTROLLEDTEXTPANEL_IDNAME, const wxPoint& pos = SYMBOL_CGBQUALSEMICONTROLLEDTEXTPANEL_POSITION, const wxSize& size = SYMBOL_CGBQUALSEMICONTROLLEDTEXTPANEL_SIZE, long style = SYMBOL_CGBQUALSEMICONTROLLEDTEXTPANEL_STYLE );

    /// Creation
    bool Create( wxWindow* parent, wxWindowID id = SYMBOL_CGBQUALSEMICONTROLLEDTEXTPANEL_IDNAME, const wxPoint& pos = SYMBOL_CGBQUALSEMICONTROLLEDTEXTPANEL_POSITION, const wxSize& size = SYMBOL_CGBQUALSEMICONTROLLEDTEXTPANEL_SIZE, long style = SYMBOL_CGBQUALSEMICONTROLLEDTEXTPANEL_STYLE );

    /// Destructor
    ~CGBQualSemicontrolledTextPanel();

    /// Initialises member variables
    void Init();

    /// Creates the controls and sizers
    void CreateControls();

////@begin CGBQualSemicontrolledTextPanel event handler declarations

////@end CGBQualSemicontrolledTextPanel event handler declarations

////@begin CGBQualSemicontrolledTextPanel member function declarations

    /// Retrieves bitmap resources
    wxBitmap GetBitmapResource( const wxString& name );

    /// Retrieves icon resources
    wxIcon GetIconResource( const wxString& name );
////@end CGBQualSemicontrolledTextPanel member function declarations

    /// Should we show tooltips?
    static bool ShowToolTips();

////@begin CGBQualSemicontrolledTextPanel member variables
    wxComboBox* m_theValue;
    /// Control identifiers
    enum {
        ID_CGBQUALTEXTPANEL = 10047,
        ID_TEXTCTRL = 10049
    };
////@end CGBQualSemicontrolledTextPanel member variables

    virtual void SetValue(string val);
    virtual string GetValue();

    void SetControlledList (vector<string> choices);

};

END_NCBI_SCOPE

#endif
    // _GBQUAL_SEMICONTROLLED_H_