/*  $Id: rptunitrange_panel.hpp 37148 2016-12-08 16:17:23Z filippov $
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
#ifndef _RPTUNITRANGE_PANEL_H_
#define _RPTUNITRANGE_PANEL_H_

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
#define SYMBOL_CRPTUNITRANGEPANEL_STYLE wxCAPTION|wxRESIZE_BORDER|wxSYSTEM_MENU|wxCLOSE_BOX|wxTAB_TRAVERSAL
#define SYMBOL_CRPTUNITRANGEPANEL_TITLE _("RptUnitRangePanel")
#define SYMBOL_CRPTUNITRANGEPANEL_IDNAME ID_CRPTUNITRANGEPANEL
#define SYMBOL_CRPTUNITRANGEPANEL_SIZE wxSize(400, 300)
#define SYMBOL_CRPTUNITRANGEPANEL_POSITION wxDefaultPosition
////@end control identifiers


/*!
 * CRptUnitRangePanel class declaration
 */

class CRptUnitRangePanel: public CFormattedQualPanel
{    
    DECLARE_DYNAMIC_CLASS( CRptUnitRangePanel )
    DECLARE_EVENT_TABLE()

public:
    /// Constructors
    CRptUnitRangePanel();
    CRptUnitRangePanel( wxWindow* parent, wxWindowID id = SYMBOL_CRPTUNITRANGEPANEL_IDNAME, const wxString& caption = SYMBOL_CRPTUNITRANGEPANEL_TITLE, const wxPoint& pos = SYMBOL_CRPTUNITRANGEPANEL_POSITION, const wxSize& size = SYMBOL_CRPTUNITRANGEPANEL_SIZE, long style = SYMBOL_CRPTUNITRANGEPANEL_STYLE );

    /// Creation
    bool Create( wxWindow* parent, wxWindowID id = SYMBOL_CRPTUNITRANGEPANEL_IDNAME, const wxString& caption = SYMBOL_CRPTUNITRANGEPANEL_TITLE, const wxPoint& pos = SYMBOL_CRPTUNITRANGEPANEL_POSITION, const wxSize& size = SYMBOL_CRPTUNITRANGEPANEL_SIZE, long style = SYMBOL_CRPTUNITRANGEPANEL_STYLE );

    /// Destructor
    ~CRptUnitRangePanel();

    /// Initialises member variables
    void Init();

    /// Creates the controls and sizers
    void CreateControls();

    virtual void SetValue(string val);
    virtual string GetValue();

////@begin CRptUnitRangePanel event handler declarations

////@end CRptUnitRangePanel event handler declarations

////@begin CRptUnitRangePanel member function declarations

    /// Retrieves bitmap resources
    wxBitmap GetBitmapResource( const wxString& name );

    /// Retrieves icon resources
    wxIcon GetIconResource( const wxString& name );
////@end CRptUnitRangePanel member function declarations

    /// Should we show tooltips?
    static bool ShowToolTips();

////@begin CRptUnitRangePanel member variables
    wxTextCtrl* m_StartCtrl;
    wxTextCtrl* m_StopCtrl;
    /// Control identifiers
    enum {
        ID_CRPTUNITRANGEPANEL = 10048,
        ID_TEXTCTRL29 = 10086,
        ID_TEXTCTRL30 = 10087
    };
////@end CRptUnitRangePanel member variables
};

END_NCBI_SCOPE

#endif
    // _RPTUNITRANGE_PANEL_H_
