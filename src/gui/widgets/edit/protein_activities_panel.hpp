/*  $Id: protein_activities_panel.hpp 40212 2018-01-09 17:18:14Z filippov $
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
#ifndef _PROTEIN_ACTIVITIES_PANEL_H_
#define _PROTEIN_ACTIVITIES_PANEL_H_

#include <corelib/ncbistd.hpp>
#include <corelib/ncbiobj.hpp>

/*!
 * Includes
 */

////@begin includes
////@end includes
#include <wx/panel.h>
#include <wx/sizer.h>
#include <wx/icon.h>
#include <wx/bitmap.h>

/*!
 * Forward declarations
 */

////@begin forward declarations
////@end forward declarations

BEGIN_NCBI_SCOPE

BEGIN_SCOPE(objects)
    class CScope;
END_SCOPE(objects)

/*!
 * Control identifiers
 */

////@begin control identifiers
#define SYMBOL_CPROTEINACTIVITIESPANEL_STYLE wxTAB_TRAVERSAL
#define SYMBOL_CPROTEINACTIVITIESPANEL_TITLE _("Protein Activities Panel")
#define SYMBOL_CPROTEINACTIVITIESPANEL_IDNAME ID_CSYNONYMSPANEL
#define SYMBOL_CPROTEINACTIVITIESPANEL_SIZE wxDefaultSize
#define SYMBOL_CPROTEINACTIVITIESPANEL_POSITION wxDefaultPosition
////@end control identifiers


/*!
 * CProteinActivitiesPanel class declaration
 */

class CProteinActivitiesPanel: public wxPanel
{    
    DECLARE_DYNAMIC_CLASS( CProteinActivitiesPanel )
    DECLARE_EVENT_TABLE()

public:
    /// Constructors
    CProteinActivitiesPanel();
    CProteinActivitiesPanel( wxWindow* parent, CSerialObject& object, wxWindowID id = SYMBOL_CPROTEINACTIVITIESPANEL_IDNAME, const wxPoint& pos = SYMBOL_CPROTEINACTIVITIESPANEL_POSITION, const wxSize& size = SYMBOL_CPROTEINACTIVITIESPANEL_SIZE, long style = SYMBOL_CPROTEINACTIVITIESPANEL_STYLE );

    /// Creation
    bool Create( wxWindow* parent, wxWindowID id = SYMBOL_CPROTEINACTIVITIESPANEL_IDNAME, const wxPoint& pos = SYMBOL_CPROTEINACTIVITIESPANEL_POSITION, const wxSize& size = SYMBOL_CPROTEINACTIVITIESPANEL_SIZE, long style = SYMBOL_CPROTEINACTIVITIESPANEL_STYLE );

    /// Destructor
    ~CProteinActivitiesPanel();

    /// Initialises member variables
    void Init();

    /// Creates the controls and sizers
    void CreateControls();

////@begin CProteinActivitiesPanel event handler declarations

////@end CProteinActivitiesPanel event handler declarations

////@begin CProteinActivitiesPanel member function declarations

    /// Retrieves bitmap resources
    wxBitmap GetBitmapResource( const wxString& name );

    /// Retrieves icon resources
    wxIcon GetIconResource( const wxString& name );
////@end CProteinActivitiesPanel member function declarations

    /// Should we show tooltips?
    static bool ShowToolTips();

////@begin CProteinActivitiesPanel member variables
    /// Control identifiers
    enum {
        ID_CSYNONYMSPANEL = 10019,
        ID_WINDOW = 10020
    };
////@end CProteinActivitiesPanel member variables
private:
    CSerialObject* m_Object;
};

END_NCBI_SCOPE

#endif
    // _PROTEIN_ACTIVITIES_PANEL_H_
