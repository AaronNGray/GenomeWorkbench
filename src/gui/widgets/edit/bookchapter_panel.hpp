#ifndef GUI_WIDGETS_EDIT___BOOKCHAPTER_PANEL__HPP
#define GUI_WIDGETS_EDIT___BOOKCHAPTER_PANEL__HPP

/*  $Id: bookchapter_panel.hpp 24388 2011-09-19 21:24:04Z katargir $
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
#define ID_BOOKCHAPTER 10055
#define ID_BOOKCHAPTER_CHAPTER_TITLE 10062
#define ID_BOOKCHAPTER_BOOK_TITLE 10056
#define ID_BOOKCHAPTER_PAGES 10057
#define ID_BOOKCHAPTER_PUB_YEAR 10063
#define ID_BOOKCHAPTER_COPY_YEAR 10064
#define SYMBOL_CBOOKCHAPTERPANEL_STYLE wxTAB_TRAVERSAL
#define SYMBOL_CBOOKCHAPTERPANEL_TITLE _("BookChapter")
#define SYMBOL_CBOOKCHAPTERPANEL_IDNAME ID_BOOKCHAPTER
#define SYMBOL_CBOOKCHAPTERPANEL_SIZE wxSize(400, 300)
#define SYMBOL_CBOOKCHAPTERPANEL_POSITION wxDefaultPosition
////@end control identifiers


/*!
 * CBookChapterPanel class declaration
 */

class CBookChapterPanel: public wxPanel
{    
    DECLARE_DYNAMIC_CLASS( CBookChapterPanel )
    DECLARE_EVENT_TABLE()

public:
    /// Constructors
    CBookChapterPanel();
    CBookChapterPanel( wxWindow* parent, CSerialObject& object, objects::CScope& scope, wxWindowID id = SYMBOL_CBOOKCHAPTERPANEL_IDNAME, const wxPoint& pos = SYMBOL_CBOOKCHAPTERPANEL_POSITION, const wxSize& size = SYMBOL_CBOOKCHAPTERPANEL_SIZE, long style = SYMBOL_CBOOKCHAPTERPANEL_STYLE );

    /// Creation
    bool Create( wxWindow* parent, wxWindowID id = SYMBOL_CBOOKCHAPTERPANEL_IDNAME, const wxPoint& pos = SYMBOL_CBOOKCHAPTERPANEL_POSITION, const wxSize& size = SYMBOL_CBOOKCHAPTERPANEL_SIZE, long style = SYMBOL_CBOOKCHAPTERPANEL_STYLE );

    /// Destructor
    ~CBookChapterPanel();

    /// Initialises member variables
    void Init();

    /// Creates the controls and sizers
    void CreateControls();

////@begin CBookChapterPanel event handler declarations

////@end CBookChapterPanel event handler declarations

////@begin CBookChapterPanel member function declarations

    /// Retrieves bitmap resources
    wxBitmap GetBitmapResource( const wxString& name );

    /// Retrieves icon resources
    wxIcon GetIconResource( const wxString& name );
////@end CBookChapterPanel member function declarations

    /// Should we show tooltips?
    static bool ShowToolTips();

////@begin CBookChapterPanel member variables
////@end CBookChapterPanel member variables
private:
    CSerialObject* m_Object;
    CRef<objects::CScope> m_Scope;

};

END_NCBI_SCOPE

#endif  // GUI_WIDGETS_EDIT___BOOKCHAPTER_PANEL__HPP
