#ifndef GUI_WIDGETS_EDIT___PROCEEDINGSCHAPTER_PANEL__HPP
#define GUI_WIDGETS_EDIT___PROCEEDINGSCHAPTER_PANEL__HPP

/*  $Id: proceedingschapter_panel.hpp 24602 2011-10-25 17:59:16Z katargir $
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
#define ID_PROCEEDINGSCHAPTER 10072
#define ID_PROCEEDINGSCHAPTER_ABSTRACT_TITLE 10073
#define ID_PROCEEDINGSCHAPTER_PROCEEDINGS_TITLE 10074
#define ID_PROCEEDINGSCHAPTER_PAGES 10058
#define ID_PROCEEDINGSCHAPTER_PUB_YEAR 10078
#define ID_PROCEEDINGSCHAPTER_COPY_YEAR 10079
#define SYMBOL_CPROCEEDINGSCHAPTERPANEL_STYLE wxTAB_TRAVERSAL
#define SYMBOL_CPROCEEDINGSCHAPTERPANEL_TITLE _("ProceedingsChapter")
#define SYMBOL_CPROCEEDINGSCHAPTERPANEL_IDNAME ID_PROCEEDINGSCHAPTER
#define SYMBOL_CPROCEEDINGSCHAPTERPANEL_SIZE wxSize(400, 300)
#define SYMBOL_CPROCEEDINGSCHAPTERPANEL_POSITION wxDefaultPosition
////@end control identifiers


/*!
 * CProceedingsChapterPanel class declaration
 */

class CProceedingsChapterPanel: public wxPanel
{    
    DECLARE_DYNAMIC_CLASS( CProceedingsChapterPanel )
    DECLARE_EVENT_TABLE()

public:
    /// Constructors
    CProceedingsChapterPanel();
    CProceedingsChapterPanel( wxWindow* parent, CSerialObject& object, wxWindowID id = SYMBOL_CPROCEEDINGSCHAPTERPANEL_IDNAME, const wxPoint& pos = SYMBOL_CPROCEEDINGSCHAPTERPANEL_POSITION, const wxSize& size = SYMBOL_CPROCEEDINGSCHAPTERPANEL_SIZE, long style = SYMBOL_CPROCEEDINGSCHAPTERPANEL_STYLE );

    /// Creation
    bool Create( wxWindow* parent, wxWindowID id = SYMBOL_CPROCEEDINGSCHAPTERPANEL_IDNAME, const wxPoint& pos = SYMBOL_CPROCEEDINGSCHAPTERPANEL_POSITION, const wxSize& size = SYMBOL_CPROCEEDINGSCHAPTERPANEL_SIZE, long style = SYMBOL_CPROCEEDINGSCHAPTERPANEL_STYLE );

    /// Destructor
    ~CProceedingsChapterPanel();

    /// Initialises member variables
    void Init();

    /// Creates the controls and sizers
    void CreateControls();

    virtual bool TransferDataToWindow();
    virtual bool TransferDataFromWindow();

    ////@begin CProceedingsChapterPanel event handler declarations

////@end CProceedingsChapterPanel event handler declarations

////@begin CProceedingsChapterPanel member function declarations

    /// Retrieves bitmap resources
    wxBitmap GetBitmapResource( const wxString& name );

    /// Retrieves icon resources
    wxIcon GetIconResource( const wxString& name );
////@end CProceedingsChapterPanel member function declarations

    /// Should we show tooltips?
    static bool ShowToolTips();

////@begin CProceedingsChapterPanel member variables
////@end CProceedingsChapterPanel member variables
private:
    CSerialObject* m_Object;
    CRef<CSerialObject> m_EditedArt;
    long m_PubYear;
    long m_CopyrightYear;
};

END_NCBI_SCOPE

#endif  // GUI_WIDGETS_EDIT___PROCEEDINGSCHAPTER_PANEL__HPP
