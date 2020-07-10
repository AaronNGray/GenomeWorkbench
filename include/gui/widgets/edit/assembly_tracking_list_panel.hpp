/*  $Id: assembly_tracking_list_panel.hpp 28606 2013-08-06 17:18:45Z bollin $
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
#ifndef _ASSEMBLY_TRACKING_LIST_PANEL_H_
#define _ASSEMBLY_TRACKING_LIST_PANEL_H_

#include <corelib/ncbistd.hpp>
#include <gui/widgets/edit/user_object_util.hpp>

#include <wx/scrolwin.h>

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
#define SYMBOL_CASSEMBLYTRACKINGLISTPANEL_STYLE wxTAB_TRAVERSAL
#define SYMBOL_CASSEMBLYTRACKINGLISTPANEL_TITLE _("AssemblyTrackingListPanel")
#define SYMBOL_CASSEMBLYTRACKINGLISTPANEL_IDNAME ID_CASSEMBLYTRACKINGLISTPANEL
#define SYMBOL_CASSEMBLYTRACKINGLISTPANEL_SIZE wxSize(400, 300)
#define SYMBOL_CASSEMBLYTRACKINGLISTPANEL_POSITION wxDefaultPosition
////@end control identifiers


/*!
 * CAssemblyTrackingListPanel class declaration
 */

class CAssemblyTrackingListPanel: public CUserFieldListPanel
{    
    DECLARE_DYNAMIC_CLASS( CAssemblyTrackingListPanel )
    DECLARE_EVENT_TABLE()

public:
    /// Constructors
    CAssemblyTrackingListPanel();
    CAssemblyTrackingListPanel( wxWindow* parent, wxWindowID id = SYMBOL_CASSEMBLYTRACKINGLISTPANEL_IDNAME, const wxPoint& pos = SYMBOL_CASSEMBLYTRACKINGLISTPANEL_POSITION, const wxSize& size = SYMBOL_CASSEMBLYTRACKINGLISTPANEL_SIZE, long style = SYMBOL_CASSEMBLYTRACKINGLISTPANEL_STYLE );

    /// Creation
    bool Create( wxWindow* parent, wxWindowID id = SYMBOL_CASSEMBLYTRACKINGLISTPANEL_IDNAME, const wxPoint& pos = SYMBOL_CASSEMBLYTRACKINGLISTPANEL_POSITION, const wxSize& size = SYMBOL_CASSEMBLYTRACKINGLISTPANEL_SIZE, long style = SYMBOL_CASSEMBLYTRACKINGLISTPANEL_STYLE );

    /// Destructor
    ~CAssemblyTrackingListPanel();

    /// Initialises member variables
    void Init();

    /// Creates the controls and sizers
    void CreateControls();

////@begin CAssemblyTrackingListPanel event handler declarations

////@end CAssemblyTrackingListPanel event handler declarations

////@begin CAssemblyTrackingListPanel member function declarations

    /// Retrieves bitmap resources
    wxBitmap GetBitmapResource( const wxString& name );

    /// Retrieves icon resources
    wxIcon GetIconResource( const wxString& name );
////@end CAssemblyTrackingListPanel member function declarations

    /// Should we show tooltips?
    static bool ShowToolTips();

////@begin CAssemblyTrackingListPanel member variables
    /// Control identifiers
    enum {
        ID_CASSEMBLYTRACKINGLISTPANEL = 10156
    };
////@end CAssemblyTrackingListPanel member variables
};

END_NCBI_SCOPE

#endif
    // _ASSEMBLY_TRACKING_LIST_PANEL_H_
