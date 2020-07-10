/*  $Id: altitude_panel.hpp 29060 2013-10-01 19:21:17Z bollin $
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
#ifndef _ALTITUDE_PANEL_H_
#define _ALTITUDE_PANEL_H_

#include <corelib/ncbistd.hpp>
#include <gui/widgets/edit/srcmod_edit_panel.hpp>

#include <wx/textctrl.h>

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
#define ID_CALTITUDEPANEL 10014
#define ID_ALTITUDE_TXT 10023
#define SYMBOL_CALTITUDEPANEL_STYLE wxTAB_TRAVERSAL
#define SYMBOL_CALTITUDEPANEL_TITLE _("AltitudePanel")
#define SYMBOL_CALTITUDEPANEL_IDNAME ID_CALTITUDEPANEL
#define SYMBOL_CALTITUDEPANEL_SIZE wxSize(400, 300)
#define SYMBOL_CALTITUDEPANEL_POSITION wxDefaultPosition
////@end control identifiers


/*!
 * CAltitudePanel class declaration
 */

class CAltitudePanel: public CSrcModEditPanel
{    
    DECLARE_DYNAMIC_CLASS( CAltitudePanel )
    DECLARE_EVENT_TABLE()

public:
    /// Constructors
    CAltitudePanel();
    CAltitudePanel( wxWindow* parent, wxWindowID id = SYMBOL_CALTITUDEPANEL_IDNAME, const wxPoint& pos = SYMBOL_CALTITUDEPANEL_POSITION, const wxSize& size = SYMBOL_CALTITUDEPANEL_SIZE, long style = SYMBOL_CALTITUDEPANEL_STYLE );

    /// Creation
    bool Create( wxWindow* parent, wxWindowID id = SYMBOL_CALTITUDEPANEL_IDNAME, const wxPoint& pos = SYMBOL_CALTITUDEPANEL_POSITION, const wxSize& size = SYMBOL_CALTITUDEPANEL_SIZE, long style = SYMBOL_CALTITUDEPANEL_STYLE );

    /// Destructor
    ~CAltitudePanel();

    /// Initialises member variables
    void Init();

    /// Creates the controls and sizers
    void CreateControls();

////@begin CAltitudePanel event handler declarations

    /// wxEVT_COMMAND_TEXT_UPDATED event handler for ID_ALTITUDE_TXT
    void OnAltitudeTxtTextUpdated( wxCommandEvent& event );

////@end CAltitudePanel event handler declarations

////@begin CAltitudePanel member function declarations

    /// Retrieves bitmap resources
    wxBitmap GetBitmapResource( const wxString& name );

    /// Retrieves icon resources
    wxIcon GetIconResource( const wxString& name );
////@end CAltitudePanel member function declarations

    /// Should we show tooltips?
    static bool ShowToolTips();

////@begin CAltitudePanel member variables
    wxTextCtrl* m_Text;
////@end CAltitudePanel member variables
    virtual string GetValue();
    virtual void SetValue(const string& val);
    virtual bool IsWellFormatted(const string& val);
protected:
    bool x_GetNumber(const string& input, string& number);
};

END_NCBI_SCOPE

#endif
    // _ALTITUDE_PANEL_H_
