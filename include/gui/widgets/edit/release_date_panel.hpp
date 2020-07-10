#ifndef GUI_WIDGETS_EDIT___RELEASE_DATE_PANEL__HPP
#define GUI_WIDGETS_EDIT___RELEASE_DATE_PANEL__HPP

/*  $Id: release_date_panel.hpp 27349 2013-01-31 14:19:51Z bollin $
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

#include <corelib/ncbiobj.hpp>
#include <serial/serialbase.hpp>
#include <wx/panel.h>
#include <wx/radiobut.h>
#include <wx/stattext.h>
#include <gui/gui_export.h>
#include <gui/widgets/edit/flexibledate_panel.hpp>


/*!
 * Includes
 */

////@begin includes
////@end includes

/*!
 * Forward declarations
 */

////@begin forward declarations
class wxBoxSizer;
////@end forward declarations

BEGIN_NCBI_SCOPE

/*!
 * Control identifiers
 */

////@begin control identifiers
#define ID_CRELEASEDATEPANEL 10085
#define ID_RADIOBUTTON 10082
#define ID_RADIOBUTTON1 10084
#define SYMBOL_CRELEASEDATEPANEL_STYLE wxTAB_TRAVERSAL
#define SYMBOL_CRELEASEDATEPANEL_TITLE _("ReleaseDatePanel")
#define SYMBOL_CRELEASEDATEPANEL_IDNAME ID_CRELEASEDATEPANEL
#define SYMBOL_CRELEASEDATEPANEL_SIZE wxSize(400, 300)
#define SYMBOL_CRELEASEDATEPANEL_POSITION wxDefaultPosition
////@end control identifiers


/*!
 * CReleaseDatePanel class declaration
 */

class NCBI_GUIWIDGETS_EDIT_EXPORT CReleaseDatePanel: public wxPanel
{    
    DECLARE_DYNAMIC_CLASS( CReleaseDatePanel )
    DECLARE_EVENT_TABLE()

public:
    /// Constructors
    CReleaseDatePanel();
    CReleaseDatePanel( wxWindow* parent, CSerialObject& object, wxWindowID id = SYMBOL_CRELEASEDATEPANEL_IDNAME, const wxPoint& pos = SYMBOL_CRELEASEDATEPANEL_POSITION, const wxSize& size = SYMBOL_CRELEASEDATEPANEL_SIZE, long style = SYMBOL_CRELEASEDATEPANEL_STYLE );

    /// Creation
    bool Create( wxWindow* parent, wxWindowID id = SYMBOL_CRELEASEDATEPANEL_IDNAME, const wxPoint& pos = SYMBOL_CRELEASEDATEPANEL_POSITION, const wxSize& size = SYMBOL_CRELEASEDATEPANEL_SIZE, long style = SYMBOL_CRELEASEDATEPANEL_STYLE );

    /// Destructor
    ~CReleaseDatePanel();

    /// Initialises member variables
    void Init();

    /// Creates the controls and sizers
    void CreateControls();

    virtual bool TransferDataToWindow();
    virtual bool TransferDataFromWindow();
    string GetProblems();
    void UnselectReleaseDateChoice();

////@begin CReleaseDatePanel event handler declarations

    /// wxEVT_COMMAND_RADIOBUTTON_SELECTED event handler for ID_RADIOBUTTON
    void OnImmediateReleaseSelected( wxCommandEvent& event );

    /// wxEVT_COMMAND_RADIOBUTTON_SELECTED event handler for ID_RADIOBUTTON1
    void OnHUPSelected( wxCommandEvent& event );

////@end CReleaseDatePanel event handler declarations

////@begin CReleaseDatePanel member function declarations

    /// Retrieves bitmap resources
    wxBitmap GetBitmapResource( const wxString& name );

    /// Retrieves icon resources
    wxIcon GetIconResource( const wxString& name );
////@end CReleaseDatePanel member function declarations

    /// Should we show tooltips?
    static bool ShowToolTips();

////@begin CReleaseDatePanel member variables
    wxRadioButton* m_ImmediateRelease;
    wxRadioButton* m_HUP;
    wxBoxSizer* m_DateSizer;
////@end CReleaseDatePanel member variables
    CFlexibleDatePanel* m_Reldate;
private:
    CSerialObject* m_Object;
};

END_NCBI_SCOPE

#endif
    // GUI_WIDGETS_EDIT___RELEASE_DATE_PANEL__HPP
