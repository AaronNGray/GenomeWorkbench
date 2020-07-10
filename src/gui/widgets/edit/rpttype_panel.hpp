/*  $Id: rpttype_panel.hpp 37148 2016-12-08 16:17:23Z filippov $
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
#ifndef _RPTTYPE_PANEL_H_
#define _RPTTYPE_PANEL_H_

#include <corelib/ncbistd.hpp>
#include <gui/widgets/edit/formattedqual_panel.hpp>

/*!
 * Includes
 */

////@begin includes
////@end includes
#include <wx/checkbox.h>

/*!
 * Forward declarations
 */

////@begin forward declarations
class wxFlexGridSizer;
////@end forward declarations

BEGIN_NCBI_SCOPE

/*!
 * Control identifiers
 */

////@begin control identifiers
#define SYMBOL_CRPTTYPEPANEL_STYLE wxCAPTION|wxRESIZE_BORDER|wxSYSTEM_MENU|wxCLOSE_BOX|wxTAB_TRAVERSAL
#define SYMBOL_CRPTTYPEPANEL_TITLE _("RptTypePanel")
#define SYMBOL_CRPTTYPEPANEL_IDNAME ID_CRPTTYPEPANEL
#define SYMBOL_CRPTTYPEPANEL_SIZE wxSize(400, 300)
#define SYMBOL_CRPTTYPEPANEL_POSITION wxDefaultPosition
////@end control identifiers


/*!
 * CRptTypePanel class declaration
 */

class CRptTypePanel: public CFormattedQualPanel
{    
    DECLARE_DYNAMIC_CLASS( CRptTypePanel )
    DECLARE_EVENT_TABLE()

public:
    /// Constructors
    CRptTypePanel();
    CRptTypePanel( wxWindow* parent, wxWindowID id = SYMBOL_CRPTTYPEPANEL_IDNAME, const wxString& caption = SYMBOL_CRPTTYPEPANEL_TITLE, const wxPoint& pos = SYMBOL_CRPTTYPEPANEL_POSITION, const wxSize& size = SYMBOL_CRPTTYPEPANEL_SIZE, long style = SYMBOL_CRPTTYPEPANEL_STYLE );

    /// Creation
    bool Create( wxWindow* parent, wxWindowID id = SYMBOL_CRPTTYPEPANEL_IDNAME, const wxString& caption = SYMBOL_CRPTTYPEPANEL_TITLE, const wxPoint& pos = SYMBOL_CRPTTYPEPANEL_POSITION, const wxSize& size = SYMBOL_CRPTTYPEPANEL_SIZE, long style = SYMBOL_CRPTTYPEPANEL_STYLE );

    /// Destructor
    ~CRptTypePanel();

    /// Initialises member variables
    void Init();

    /// Creates the controls and sizers
    void CreateControls();

    virtual void SetValue(string val);
    virtual string GetValue();

////@begin CRptTypePanel event handler declarations

////@end CRptTypePanel event handler declarations

////@begin CRptTypePanel member function declarations

    /// Retrieves bitmap resources
    wxBitmap GetBitmapResource( const wxString& name );

    /// Retrieves icon resources
    wxIcon GetIconResource( const wxString& name );
////@end CRptTypePanel member function declarations

    /// Should we show tooltips?
    static bool ShowToolTips();

////@begin CRptTypePanel member variables
    wxFlexGridSizer* m_TheSizer;
    /// Control identifiers
    enum {
        ID_CRPTTYPEPANEL = 10090
    };
////@end CRptTypePanel member variables

    static bool IsParseable( const string& val );

private:
    wxArrayString m_AcceptedValues;
    vector<wxCheckBox*> m_Boxes;

};

END_NCBI_SCOPE

#endif
    // _RPTTYPE_PANEL_H_
