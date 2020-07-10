#ifndef GUI_WIDGETS_EDIT___TITLE_PANEL__HPP
#define GUI_WIDGETS_EDIT___TITLE_PANEL__HPP

/*  $Id: titlepanel.hpp 37381 2017-01-03 15:22:09Z filippov $
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
#include <gui/widgets/wx/richtextctrl.hpp>

#include <wx/panel.h>
#include <wx/frame.h>
#include <wx/sizer.h>
#include <wx/bitmap.h>
#include <wx/icon.h>
#include <wx/choice.h>
#include <wx/textctrl.h>
#include <wx/stattext.h>
#include "wx/valtext.h"

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

class CTitle;

/*!
 * Control identifiers
 */

////@begin control identifiers
#define ID_TITLEPANEL 10015
#define ID_TEXTCTRL1 10016
#define ID_PUB_TITLE_SEARCH 10017
#define SYMBOL_CTITLEPANEL_STYLE wxCAPTION|wxRESIZE_BORDER|wxSYSTEM_MENU|wxCLOSE_BOX|wxTAB_TRAVERSAL
#define SYMBOL_CTITLEPANEL_TITLE _("TitlePanel")
#define SYMBOL_CTITLEPANEL_IDNAME ID_TITLEPANEL
#define SYMBOL_CTITLEPANEL_SIZE wxSize(400, 300)
#define SYMBOL_CTITLEPANEL_POSITION wxDefaultPosition
////@end control identifiers


/*!
 * CTitlePanel class declaration
 */

class CTitlePanel: public wxPanel
{    
    DECLARE_DYNAMIC_CLASS( CTitlePanel )
    DECLARE_EVENT_TABLE()

public:
    /// Constructors
    CTitlePanel();
    CTitlePanel( wxWindow* parent, CRef<objects::CTitle::C_E> title, wxWindowID id = SYMBOL_CTITLEPANEL_IDNAME, const wxString& caption = SYMBOL_CTITLEPANEL_TITLE, const wxPoint& pos = SYMBOL_CTITLEPANEL_POSITION, const wxSize& size = SYMBOL_CTITLEPANEL_SIZE, long style = SYMBOL_CTITLEPANEL_STYLE );

    /// Creation
    bool Create( wxWindow* parent, wxWindowID id = SYMBOL_CTITLEPANEL_IDNAME, const wxString& caption = SYMBOL_CTITLEPANEL_TITLE, const wxPoint& pos = SYMBOL_CTITLEPANEL_POSITION, const wxSize& size = SYMBOL_CTITLEPANEL_SIZE, long style = SYMBOL_CTITLEPANEL_STYLE );

    /// Destructor
    ~CTitlePanel();

    /// Initialises member variables
    void Init();

    /// Creates the controls and sizers
    void CreateControls();

    virtual bool TransferDataFromWindow();
    virtual bool TransferDataToWindow();

////@begin CTitlePanel event handler declarations
    void OnSearchTitle( wxCommandEvent& event );
////@end CTitlePanel event handler declarations

////@begin CTitlePanel member function declarations

    /// Retrieves bitmap resources
    wxBitmap GetBitmapResource( const wxString& name );

    /// Retrieves icon resources
    wxIcon GetIconResource( const wxString& name );
////@end CTitlePanel member function declarations

    /// Should we show tooltips?
    static bool ShowToolTips();

////@begin CTitlePanel member variables
    CRichTextCtrl* m_TitleCtrl;
////@end CTitlePanel member variables

private:
    CRef<objects::CTitle::C_E> m_Title;

};

END_NCBI_SCOPE

#endif  // GUI_WIDGETS_EDIT___TITLE_PANEL__HPP
