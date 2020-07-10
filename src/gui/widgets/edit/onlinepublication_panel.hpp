#ifndef GUI_WIDGETS_EDIT___ONLINEPUBLICATION_PANEL__HPP
#define GUI_WIDGETS_EDIT___ONLINEPUBLICATION_PANEL__HPP

/*  $Id: onlinepublication_panel.hpp 24385 2011-09-19 21:07:55Z katargir $
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
#define ID_ONLINEPUBLICATION 10085
#define ID_ONLINEPUBLICATION_TITLE 10086
#define ID_ONLINEPUBLICATION_SERIAL_NUMBER 10087
#define SYMBOL_CONLINEPUBLICATIONPANEL_STYLE wxTAB_TRAVERSAL
#define SYMBOL_CONLINEPUBLICATIONPANEL_TITLE _("OnlinePublication")
#define SYMBOL_CONLINEPUBLICATIONPANEL_IDNAME ID_ONLINEPUBLICATION
#define SYMBOL_CONLINEPUBLICATIONPANEL_SIZE wxSize(400, 300)
#define SYMBOL_CONLINEPUBLICATIONPANEL_POSITION wxDefaultPosition
////@end control identifiers


/*!
 * COnlinePublicationPanel class declaration
 */

class COnlinePublicationPanel: public wxPanel
{    
    DECLARE_DYNAMIC_CLASS( COnlinePublicationPanel )
    DECLARE_EVENT_TABLE()

public:
    /// Constructors
    COnlinePublicationPanel();
    COnlinePublicationPanel( wxWindow* parent, CSerialObject& object, wxWindowID id = SYMBOL_CONLINEPUBLICATIONPANEL_IDNAME, const wxPoint& pos = SYMBOL_CONLINEPUBLICATIONPANEL_POSITION, const wxSize& size = SYMBOL_CONLINEPUBLICATIONPANEL_SIZE, long style = SYMBOL_CONLINEPUBLICATIONPANEL_STYLE );

    /// Creation
    bool Create( wxWindow* parent, wxWindowID id = SYMBOL_CONLINEPUBLICATIONPANEL_IDNAME, const wxPoint& pos = SYMBOL_CONLINEPUBLICATIONPANEL_POSITION, const wxSize& size = SYMBOL_CONLINEPUBLICATIONPANEL_SIZE, long style = SYMBOL_CONLINEPUBLICATIONPANEL_STYLE );

    /// Destructor
    ~COnlinePublicationPanel();

    /// Initialises member variables
    void Init();

    /// Creates the controls and sizers
    void CreateControls();

    virtual bool TransferDataFromWindow();
////@begin COnlinePublicationPanel event handler declarations

////@end COnlinePublicationPanel event handler declarations

////@begin COnlinePublicationPanel member function declarations

    /// Retrieves bitmap resources
    wxBitmap GetBitmapResource( const wxString& name );

    /// Retrieves icon resources
    wxIcon GetIconResource( const wxString& name );
////@end COnlinePublicationPanel member function declarations

    /// Should we show tooltips?
    static bool ShowToolTips();

////@begin COnlinePublicationPanel member variables
////@end COnlinePublicationPanel member variables
private:
    CSerialObject* m_Object;
    CRef<CSerialObject> m_EditedCit;
    CRef<objects::CScope> m_Scope;
    long m_SerialNumber;

};

END_NCBI_SCOPE

#endif  // GUI_WIDGETS_EDIT___ONLINEPUBLICATION_PANEL__HPP
