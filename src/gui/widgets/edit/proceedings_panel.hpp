#ifndef GUI_WIDGETS_EDIT___PROCEEDINGS_PANEL__HPP
#define GUI_WIDGETS_EDIT___PROCEEDINGS_PANEL__HPP

/*  $Id: proceedings_panel.hpp 24602 2011-10-25 17:59:16Z katargir $
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

#include <objects/biblio/Cit_book.hpp>
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
#define ID_PROCEEDINGS 10072
#define ID_PROCEEDINGS_TITLE 10074
#define ID_PROCEEDINGS_PUB_YEAR 10078
#define ID_PROCEEDINGS_COPY_YEAR 10079
#define SYMBOL_CPROCEEDINGSPANEL_STYLE wxTAB_TRAVERSAL
#define SYMBOL_CPROCEEDINGSPANEL_TITLE _("Proceedings")
#define SYMBOL_CPROCEEDINGSPANEL_IDNAME ID_PROCEEDINGS
#define SYMBOL_CPROCEEDINGSPANEL_SIZE wxSize(400, 300)
#define SYMBOL_CPROCEEDINGSPANEL_POSITION wxDefaultPosition
////@end control identifiers


/*!
 * CProceedingsPanel class declaration
 */

class CProceedingsPanel: public wxPanel
{    
    DECLARE_DYNAMIC_CLASS( CProceedingsPanel )
    DECLARE_EVENT_TABLE()

public:
    /// Constructors
    CProceedingsPanel();
    CProceedingsPanel( wxWindow* parent, CRef<objects::CCit_book> book, wxWindowID id = SYMBOL_CPROCEEDINGSPANEL_IDNAME, const wxPoint& pos = SYMBOL_CPROCEEDINGSPANEL_POSITION, const wxSize& size = SYMBOL_CPROCEEDINGSPANEL_SIZE, long style = SYMBOL_CPROCEEDINGSPANEL_STYLE );

    /// Creation
    bool Create( wxWindow* parent, wxWindowID id = SYMBOL_CPROCEEDINGSPANEL_IDNAME, const wxPoint& pos = SYMBOL_CPROCEEDINGSPANEL_POSITION, const wxSize& size = SYMBOL_CPROCEEDINGSPANEL_SIZE, long style = SYMBOL_CPROCEEDINGSPANEL_STYLE );

    /// Destructor
    ~CProceedingsPanel();

    /// Initialises member variables
    void Init();

    /// Creates the controls and sizers
    void CreateControls();

    virtual bool TransferDataToWindow();
    virtual bool TransferDataFromWindow();

////@begin CProceedingsPanel event handler declarations

////@end CProceedingsPanel event handler declarations

////@begin CProceedingsPanel member function declarations

    /// Retrieves bitmap resources
    wxBitmap GetBitmapResource( const wxString& name );

    /// Retrieves icon resources
    wxIcon GetIconResource( const wxString& name );
////@end CProceedingsPanel member function declarations

    /// Should we show tooltips?
    static bool ShowToolTips();

////@begin CProceedingsPanel member variables
////@end CProceedingsPanel member variables
private:
    CRef<objects::CCit_book> m_Book;
    long m_PubYear;
    long m_CopyrightYear;

};

END_NCBI_SCOPE

#endif  // GUI_WIDGETS_EDIT___PROCEEDINGS_PANEL__HPP
