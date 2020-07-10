/*  $Id: single_assembly_tracking_panel.hpp 28606 2013-08-06 17:18:45Z bollin $
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
#ifndef _SINGLE_ASSEMBLY_TRACKING_PANEL_H_
#define _SINGLE_ASSEMBLY_TRACKING_PANEL_H_

#include <corelib/ncbistd.hpp>

/*!
 * Includes
 */

////@begin includes
////@end includes
#include <gui/widgets/edit/user_object_util.hpp>
#include <wx/textctrl.h>
#include <wx/panel.h>

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
#define SYMBOL_CSINGLEASSEMBLYTRACKINGPANEL_STYLE wxTAB_TRAVERSAL
#define SYMBOL_CSINGLEASSEMBLYTRACKINGPANEL_TITLE _("SingleAssemblyTrackingPanel")
#define SYMBOL_CSINGLEASSEMBLYTRACKINGPANEL_IDNAME ID_CSINGLEASSEMBLYTRACKINGPANEL
#define SYMBOL_CSINGLEASSEMBLYTRACKINGPANEL_SIZE wxSize(400, 300)
#define SYMBOL_CSINGLEASSEMBLYTRACKINGPANEL_POSITION wxDefaultPosition
////@end control identifiers


/*!
 * CSingleAssemblyTrackingPanel class declaration
 */

class CSingleAssemblyTrackingPanel: public CSingleUserFieldPanel
{    
    DECLARE_DYNAMIC_CLASS( CSingleAssemblyTrackingPanel )
    DECLARE_EVENT_TABLE()

public:
    /// Constructors
    CSingleAssemblyTrackingPanel();
    CSingleAssemblyTrackingPanel( wxWindow* parent, objects::CUser_field& field, wxWindowID id = SYMBOL_CSINGLEASSEMBLYTRACKINGPANEL_IDNAME, const wxPoint& pos = SYMBOL_CSINGLEASSEMBLYTRACKINGPANEL_POSITION, const wxSize& size = SYMBOL_CSINGLEASSEMBLYTRACKINGPANEL_SIZE, long style = SYMBOL_CSINGLEASSEMBLYTRACKINGPANEL_STYLE );

    /// Creation
    bool Create( wxWindow* parent, wxWindowID id = SYMBOL_CSINGLEASSEMBLYTRACKINGPANEL_IDNAME, const wxPoint& pos = SYMBOL_CSINGLEASSEMBLYTRACKINGPANEL_POSITION, const wxSize& size = SYMBOL_CSINGLEASSEMBLYTRACKINGPANEL_SIZE, long style = SYMBOL_CSINGLEASSEMBLYTRACKINGPANEL_STYLE );

    /// Destructor
    ~CSingleAssemblyTrackingPanel();

    /// Initialises member variables
    void Init();

    /// Creates the controls and sizers
    void CreateControls();

    virtual bool TransferDataToWindow();
    virtual bool TransferDataFromWindow();
    virtual CRef<objects::CUser_field> GetUser_field();

////@begin CSingleAssemblyTrackingPanel event handler declarations

    /// wxEVT_COMMAND_TEXT_UPDATED event handler for ID_ACCESSION_TXT
    void OnAccessionTxtTextUpdated( wxCommandEvent& event );

    /// wxEVT_COMMAND_TEXT_UPDATED event handler for ID_START_TXT
    void OnStartTxtTextUpdated( wxCommandEvent& event );

    /// wxEVT_COMMAND_TEXT_UPDATED event handler for ID_STOP_TXT
    void OnStopTxtTextUpdated( wxCommandEvent& event );

////@end CSingleAssemblyTrackingPanel event handler declarations

////@begin CSingleAssemblyTrackingPanel member function declarations

    /// Retrieves bitmap resources
    wxBitmap GetBitmapResource( const wxString& name );

    /// Retrieves icon resources
    wxIcon GetIconResource( const wxString& name );
////@end CSingleAssemblyTrackingPanel member function declarations

    /// Should we show tooltips?
    static bool ShowToolTips();

////@begin CSingleAssemblyTrackingPanel member variables
    wxTextCtrl* m_Accession;
    wxTextCtrl* m_Start;
    wxTextCtrl* m_Stop;
    /// Control identifiers
    enum {
        ID_CSINGLEASSEMBLYTRACKINGPANEL = 10149,
        ID_ACCESSION_TXT = 10150,
        ID_START_TXT = 10152,
        ID_STOP_TXT = 10153
    };
////@end CSingleAssemblyTrackingPanel member variables

    static CRef<objects::CUser_field> s_CreateTPAAssemblyField (const string& accession, int from, int to);
private:
    CRef<objects::CUser_field> m_Field;

    bool x_IsControlled (const string& field);
};

END_NCBI_SCOPE

#endif
    // _SINGLE_ASSEMBLY_TRACKING_PANEL_H_
