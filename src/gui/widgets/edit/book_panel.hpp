#ifndef GUI_WIDGETS_EDIT___BOOK_PANEL__HPP
#define GUI_WIDGETS_EDIT___BOOK_PANEL__HPP

/*  $Id: book_panel.hpp 24374 2011-09-19 20:45:11Z katargir $
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

BEGIN_NCBI_SCOPE

/*!
 * Forward declarations
 */

////@begin forward declarations
////@end forward declarations

class CSerialObject;


/*!
 * Control identifiers
 */


////@begin control identifiers
#define ID_BOOK 10055
#define ID_BOOK_TITLE 10056
#define ID_BOOK_PUB_YEAR 10063
#define ID_BOOK_COPY_YEAR 10064
#define SYMBOL_CBOOKPANEL_STYLE wxTAB_TRAVERSAL
#define SYMBOL_CBOOKPANEL_TITLE _("Book")
#define SYMBOL_CBOOKPANEL_IDNAME ID_BOOK
#define SYMBOL_CBOOKPANEL_SIZE wxSize(400, 300)
#define SYMBOL_CBOOKPANEL_POSITION wxDefaultPosition
////@end control identifiers


/*!
 * CBookPanel class declaration
 */

class CBookPanel: public wxPanel
{    
    DECLARE_DYNAMIC_CLASS( CBookPanel )
    DECLARE_EVENT_TABLE()

public:
    /// Constructors
    CBookPanel();
    CBookPanel( wxWindow* parent, CSerialObject& object, objects::CScope& scope, wxWindowID id = SYMBOL_CBOOKPANEL_IDNAME, const wxPoint& pos = SYMBOL_CBOOKPANEL_POSITION, const wxSize& size = SYMBOL_CBOOKPANEL_SIZE, long style = SYMBOL_CBOOKPANEL_STYLE );

    /// Creation
    bool Create( wxWindow* parent, wxWindowID id = SYMBOL_CBOOKPANEL_IDNAME, const wxPoint& pos = SYMBOL_CBOOKPANEL_POSITION, const wxSize& size = SYMBOL_CBOOKPANEL_SIZE, long style = SYMBOL_CBOOKPANEL_STYLE );

    /// Destructor
    ~CBookPanel();

    /// Initialises member variables
    void Init();

    /// Creates the controls and sizers
    void CreateControls();

////@begin CBookPanel event handler declarations

////@end CBookPanel event handler declarations

////@begin CBookPanel member function declarations

    /// Retrieves bitmap resources
    wxBitmap GetBitmapResource( const wxString& name );

    /// Retrieves icon resources
    wxIcon GetIconResource( const wxString& name );
////@end CBookPanel member function declarations

    /// Should we show tooltips?
    static bool ShowToolTips();

////@begin CBookPanel member variables
////@end CBookPanel member variables
private:
    CSerialObject* m_Object;
    CRef<objects::CScope> m_Scope;

};

END_NCBI_SCOPE

#endif  // GUI_WIDGETS_EDIT___BOOK_PANEL__HPP
