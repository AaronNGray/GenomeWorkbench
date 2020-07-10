/*  $Id: latlon_panel.hpp 29060 2013-10-01 19:21:17Z bollin $
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
#ifndef _LATLON_PANEL_H_
#define _LATLON_PANEL_H_

#include <corelib/ncbistd.hpp>
#include <gui/widgets/edit/srcmod_edit_panel.hpp>
#include <wx/textctrl.h>
#include <wx/choice.h>

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
#define ID_CLATLONPANEL 10024
#define ID_LAT_TXT 10025
#define ID_CHOICE 10032
#define ID_LON_TXT 10033
#define ID_CHOICE1 10034
#define SYMBOL_CLATLONPANEL_STYLE wxTAB_TRAVERSAL
#define SYMBOL_CLATLONPANEL_TITLE _("LatLonPanel")
#define SYMBOL_CLATLONPANEL_IDNAME ID_CLATLONPANEL
#define SYMBOL_CLATLONPANEL_SIZE wxSize(400, 300)
#define SYMBOL_CLATLONPANEL_POSITION wxDefaultPosition
////@end control identifiers


/*!
 * CLatLonPanel class declaration
 */

class CLatLonPanel: public CSrcModEditPanel
{    
    DECLARE_DYNAMIC_CLASS( CLatLonPanel )
    DECLARE_EVENT_TABLE()

public:
    /// Constructors
    CLatLonPanel();
    CLatLonPanel( wxWindow* parent, wxWindowID id = SYMBOL_CLATLONPANEL_IDNAME, const wxPoint& pos = SYMBOL_CLATLONPANEL_POSITION, const wxSize& size = SYMBOL_CLATLONPANEL_SIZE, long style = SYMBOL_CLATLONPANEL_STYLE );

    /// Creation
    bool Create( wxWindow* parent, wxWindowID id = SYMBOL_CLATLONPANEL_IDNAME, const wxPoint& pos = SYMBOL_CLATLONPANEL_POSITION, const wxSize& size = SYMBOL_CLATLONPANEL_SIZE, long style = SYMBOL_CLATLONPANEL_STYLE );

    /// Destructor
    ~CLatLonPanel();

    /// Initialises member variables
    void Init();

    /// Creates the controls and sizers
    void CreateControls();

////@begin CLatLonPanel event handler declarations

    /// wxEVT_COMMAND_TEXT_UPDATED event handler for ID_LAT_TXT
    void OnLatTxtTextUpdated( wxCommandEvent& event );

    /// wxEVT_COMMAND_TEXT_UPDATED event handler for ID_LON_TXT
    void OnLonTxtTextUpdated( wxCommandEvent& event );

////@end CLatLonPanel event handler declarations

////@begin CLatLonPanel member function declarations

    /// Retrieves bitmap resources
    wxBitmap GetBitmapResource( const wxString& name );

    /// Retrieves icon resources
    wxIcon GetIconResource( const wxString& name );
////@end CLatLonPanel member function declarations

    /// Should we show tooltips?
    static bool ShowToolTips();

////@begin CLatLonPanel member variables
    wxTextCtrl* m_Lat;
    wxChoice* m_NS;
    wxTextCtrl* m_Lon;
    wxChoice* m_EW;
////@end CLatLonPanel member variables
    virtual string GetValue();
    virtual void SetValue(const string& val);
    virtual bool IsWellFormatted(const string& val);
};

END_NCBI_SCOPE

#endif
    // _LATLON_PANEL_H_
