/*  $Id: twopartqual_panel.hpp 37148 2016-12-08 16:17:23Z filippov $
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
#ifndef _TWOPARTQUAL_PANEL_H_
#define _TWOPARTQUAL_PANEL_H_

#include <corelib/ncbistd.hpp>
#include <gui/widgets/edit/formattedqual_panel.hpp>

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
#define SYMBOL_CTWOPARTQUALPANEL_STYLE wxCAPTION|wxRESIZE_BORDER|wxSYSTEM_MENU|wxCLOSE_BOX|wxTAB_TRAVERSAL
#define SYMBOL_CTWOPARTQUALPANEL_TITLE _("TwoPartQualPanel")
#define SYMBOL_CTWOPARTQUALPANEL_IDNAME ID_CTWOPARTQUALPANEL
#define SYMBOL_CTWOPARTQUALPANEL_SIZE wxDefaultSize
#define SYMBOL_CTWOPARTQUALPANEL_POSITION wxDefaultPosition
////@end control identifiers


/*!
 * CTwoPartQualPanel class declaration
 */

class CTwoPartQualPanel: public CFormattedQualPanel
{    
    DECLARE_DYNAMIC_CLASS( CTwoPartQualPanel )
    DECLARE_EVENT_TABLE()

public:
    /// Constructors
    CTwoPartQualPanel();
    CTwoPartQualPanel( wxWindow* parent, wxWindowID id = SYMBOL_CTWOPARTQUALPANEL_IDNAME, const wxString& caption = SYMBOL_CTWOPARTQUALPANEL_TITLE, const wxPoint& pos = SYMBOL_CTWOPARTQUALPANEL_POSITION, const wxSize& size = SYMBOL_CTWOPARTQUALPANEL_SIZE, long style = SYMBOL_CTWOPARTQUALPANEL_STYLE );

    /// Creation
    bool Create( wxWindow* parent, wxWindowID id = SYMBOL_CTWOPARTQUALPANEL_IDNAME, const wxString& caption = SYMBOL_CTWOPARTQUALPANEL_TITLE, const wxPoint& pos = SYMBOL_CTWOPARTQUALPANEL_POSITION, const wxSize& size = SYMBOL_CTWOPARTQUALPANEL_SIZE, long style = SYMBOL_CTWOPARTQUALPANEL_STYLE );

    /// Destructor
    ~CTwoPartQualPanel();

    /// Initialises member variables
    void Init();

    /// Creates the controls and sizers
    void CreateControls();

    virtual void SetValue(string val);
    virtual string GetValue();
	virtual void GetDimensions(int *width, int *height);

    void SetControlledList (vector<string> choices);

////@begin CTwoPartQualPanel event handler declarations

    /// wxEVT_COMMAND_TEXT_UPDATED event handler for ID_TEXTCTRL28
    void OnTextctrl28TextUpdated( wxCommandEvent& event );

////@end CTwoPartQualPanel event handler declarations

////@begin CTwoPartQualPanel member function declarations

    /// Retrieves bitmap resources
    wxBitmap GetBitmapResource( const wxString& name );

    /// Retrieves icon resources
    wxIcon GetIconResource( const wxString& name );
////@end CTwoPartQualPanel member function declarations

    /// Should we show tooltips?
    static bool ShowToolTips();

////@begin CTwoPartQualPanel member variables
    wxChoice* m_ControlledListCtrl;
    wxTextCtrl* m_FreeTextCtrl;
    /// Control identifiers
    enum {
        ID_CTWOPARTQUALPANEL = 10083,
        ID_CHOICE12 = 10084,
        ID_TEXTCTRL28 = 10085
    };
////@end CTwoPartQualPanel member variables
};

END_NCBI_SCOPE

#endif
    // _TWOPARTQUAL_PANEL_H_
