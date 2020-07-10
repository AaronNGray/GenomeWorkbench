#ifndef GUI_WIDGETS_EDIT___LOCALE_PANEL__HPP
#define GUI_WIDGETS_EDIT___LOCALE_PANEL__HPP

/*  $Id: locale_panel.hpp 34283 2015-12-08 21:44:29Z filippov $
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

#include <corelib/ncbistd.hpp>

#include <objmgr/scope.hpp>

#include <wx/panel.h>

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

class CSerialObject;

/*!
 * Control identifiers
 */


////@begin control identifiers
#define ID_LOCALE 10065
#define ID_LOCALE_NAME 10066
#define ID_LOCALE_ADDRESS 10067
#define ID_LOCALE_CITY 10068
#define ID_LOCALE_STATE 10069
#define ID_LOCALE_COUNTRY 10070
#define ID_LOCALE_PROCNUM 10071
#define SYMBOL_CLOCALEPANEL_STYLE wxTAB_TRAVERSAL
#define SYMBOL_CLOCALEPANEL_TITLE _("Locale")
#define SYMBOL_CLOCALEPANEL_IDNAME ID_LOCALE
#define SYMBOL_CLOCALEPANEL_SIZE wxSize(400, 300)
#define SYMBOL_CLOCALEPANEL_POSITION wxDefaultPosition
////@end control identifiers


/*!
 * CLocalePanel class declaration
 */

class CLocalePanel: public wxPanel
{    
    DECLARE_DYNAMIC_CLASS( CLocalePanel )
    DECLARE_EVENT_TABLE()

public:
    /// Constructors
    CLocalePanel();
    CLocalePanel( wxWindow* parent, CSerialObject& object, wxWindowID id = SYMBOL_CLOCALEPANEL_IDNAME, const wxPoint& pos = SYMBOL_CLOCALEPANEL_POSITION, const wxSize& size = SYMBOL_CLOCALEPANEL_SIZE, long style = SYMBOL_CLOCALEPANEL_STYLE );

    /// Creation
    bool Create( wxWindow* parent, wxWindowID id = SYMBOL_CLOCALEPANEL_IDNAME, const wxPoint& pos = SYMBOL_CLOCALEPANEL_POSITION, const wxSize& size = SYMBOL_CLOCALEPANEL_SIZE, long style = SYMBOL_CLOCALEPANEL_STYLE );

    /// Destructor
    ~CLocalePanel();

    /// Initialises member variables
    void Init();

    /// Creates the controls and sizers
    void CreateControls();

    CRef<objects::CMeeting> GetMeeting() const;

////@begin CLocalePanel event handler declarations

////@end CLocalePanel event handler declarations

////@begin CLocalePanel member function declarations

    /// Retrieves bitmap resources
    wxBitmap GetBitmapResource( const wxString& name );

    /// Retrieves icon resources
    wxIcon GetIconResource( const wxString& name );
////@end CLocalePanel member function declarations

    /// Should we show tooltips?
    static bool ShowToolTips();

    virtual bool TransferDataToWindow();
    virtual bool TransferDataFromWindow();

////@begin CLocalePanel member variables
////@end CLocalePanel member variables
private:
    CSerialObject* m_Object;
    CRef<CSerialObject> m_EditedMeeting;
    wxWindow *m_PubDate;
};

END_NCBI_SCOPE

#endif  // GUI_WIDGETS_EDIT___LOCALE_PANEL__HPP
