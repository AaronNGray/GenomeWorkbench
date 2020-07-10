#ifndef GUI_WIDGETS_EDIT___UNPUBLISHED_PANEL__HPP
#define GUI_WIDGETS_EDIT___UNPUBLISHED_PANEL__HPP

/*  $Id: unpublished_panel.hpp 37431 2017-01-10 16:55:15Z filippov $
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

#include <gui/gui_export.h>
#include <objmgr/scope.hpp>
#include <gui/widgets/wx/richtextctrl.hpp>

#include <wx/panel.h>
#include <wx/textctrl.h>

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
#define ID_CUNPUBLISHEDPANEL 10011
#define ID_UNPUB_TITLE 10033
#define ID_UNPUB_TITLE_SEARCH 10034
#define SYMBOL_CUNPUBLISHEDPANEL_STYLE wxTAB_TRAVERSAL
#define SYMBOL_CUNPUBLISHEDPANEL_TITLE _("UnpublishedPanel")
#define SYMBOL_CUNPUBLISHEDPANEL_IDNAME ID_CUNPUBLISHEDPANEL
#define SYMBOL_CUNPUBLISHEDPANEL_SIZE wxSize(400, 300)
#define SYMBOL_CUNPUBLISHEDPANEL_POSITION wxDefaultPosition
////@end control identifiers


/*!
 * CUnpublishedPanel class declaration
 */

class NCBI_GUIWIDGETS_EDIT_EXPORT CUnpublishedPanel: public wxPanel
{    
public:
    /// Constructors
    CUnpublishedPanel();
    CUnpublishedPanel( wxWindow* parent, const string &title, wxWindowID id = SYMBOL_CUNPUBLISHEDPANEL_IDNAME, const wxPoint& pos = SYMBOL_CUNPUBLISHEDPANEL_POSITION, const wxSize& size = SYMBOL_CUNPUBLISHEDPANEL_SIZE, long style = SYMBOL_CUNPUBLISHEDPANEL_STYLE );

    /// Creation
    bool Create( wxWindow* parent, wxWindowID id = SYMBOL_CUNPUBLISHEDPANEL_IDNAME, const wxPoint& pos = SYMBOL_CUNPUBLISHEDPANEL_POSITION, const wxSize& size = SYMBOL_CUNPUBLISHEDPANEL_SIZE, long style = SYMBOL_CUNPUBLISHEDPANEL_STYLE );

    /// Destructor
    ~CUnpublishedPanel();

    /// Initialises member variables
    void Init();

    /// Creates the controls and sizers
    void CreateControls();

    void SetTitle(const string &title);
    string GetTitle(void);

////@begin CUnpublishedPanel event handler declarations
    void OnSearchTitle( wxCommandEvent& event );
////@end CUnpublishedPanel event handler declarations

////@begin CUnpublishedPanel member function declarations

    /// Retrieves bitmap resources
    wxBitmap GetBitmapResource( const wxString& name );

    /// Retrieves icon resources
    wxIcon GetIconResource( const wxString& name );
////@end CUnpublishedPanel member function declarations

    /// Should we show tooltips?
    static bool ShowToolTips();

////@begin CUnpublishedPanel member variables
////@end CUnpublishedPanel member variables
private:
    CRichTextCtrl* m_TextCtrl;
    string m_title;
    DECLARE_DYNAMIC_CLASS( CUnpublishedPanel )
    DECLARE_EVENT_TABLE()
};

END_NCBI_SCOPE

#endif  // GUI_WIDGETS_EDIT___UNPUBLISHED_PANEL__HPP
