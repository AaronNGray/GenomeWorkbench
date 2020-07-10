#ifndef GUI_PKG_SEQ_EDIT___INPRESSREF_PANEL__HPP
#define GUI_PKG_SEQ_EDIT___INPRESSREF_PANEL__HPP
/*  $Id: inpressref_panel.hpp 42780 2019-04-11 16:35:17Z asztalos $
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
 * Authors:  Andrea Asztalos
 */

#include <corelib/ncbistd.hpp>
#include <wx/panel.h>

class wxTextCtrl;

BEGIN_NCBI_SCOPE


/*!
 * Control identifiers
 */

////@begin control identifiers
#define SYMBOL_CINPRESSREFPANEL_STYLE wxTAB_TRAVERSAL
#define SYMBOL_CINPRESSREFPANEL_TITLE _("InPress Reference")
#define SYMBOL_CINPRESSREFPANEL_IDNAME ID_CINPRESSREFPANEL
#define SYMBOL_CINPRESSREFPANEL_SIZE wxSize(400, 300)
#define SYMBOL_CINPRESSREFPANEL_POSITION wxDefaultPosition
////@end control identifiers


/*!
 * CInPressRefPanel class declaration
 */

class CInPressRefPanel: public wxPanel
{    
    DECLARE_DYNAMIC_CLASS( CInPressRefPanel )
    DECLARE_EVENT_TABLE()

public:
    /// Constructors
    CInPressRefPanel();
    CInPressRefPanel( wxWindow* parent, 
        wxWindowID id = SYMBOL_CINPRESSREFPANEL_IDNAME, 
        const wxPoint& pos = SYMBOL_CINPRESSREFPANEL_POSITION, 
        const wxSize& size = SYMBOL_CINPRESSREFPANEL_SIZE, 
        long style = SYMBOL_CINPRESSREFPANEL_STYLE );

    /// Creation
    bool Create( wxWindow* parent, 
        wxWindowID id = SYMBOL_CINPRESSREFPANEL_IDNAME, 
        const wxPoint& pos = SYMBOL_CINPRESSREFPANEL_POSITION, 
        const wxSize& size = SYMBOL_CINPRESSREFPANEL_SIZE, 
        long style = SYMBOL_CINPRESSREFPANEL_STYLE );

    /// Destructor
    ~CInPressRefPanel();

    /// Initialises member variables
    void Init();

    /// Creates the controls and sizers
    void CreateControls();

////@begin CInPressRefPanel event handler declarations

    /// wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_PMIDLOOKBTN
    void OnLookupClick( wxCommandEvent& event );

////@end CInPressRefPanel event handler declarations

////@begin CInPressRefPanel member function declarations

    /// Retrieves bitmap resources
    wxBitmap GetBitmapResource( const wxString& name );

    /// Retrieves icon resources
    wxIcon GetIconResource( const wxString& name );
////@end CInPressRefPanel member function declarations

    /// Should we show tooltips?
    static bool ShowToolTips();

private:
////@begin CInPressRefPanel member variables
    wxTextCtrl* m_PubMedID;
    wxTextCtrl* m_ReferenceTitle;
    wxTextCtrl* m_JournalTitle;
    wxTextCtrl* m_YearCtrl;
    wxTextCtrl* m_VolCtrl;
    wxTextCtrl* m_IssueCtrl;
    wxTextCtrl* m_PagesFromCtrl;
    wxTextCtrl* m_PagesToCtrl;
public:
    /// Control identifiers
    enum {
        ID_CINPRESSREFPANEL = 6665,
        ID_PMIDTEXTCTRL = 6666,
        ID_PMIDLOOKBTN = 6667,
        ID_INPREFTITLE = 6668,
        ID_INPJOURNAL = 6669,
        ID_INPYEAR = 6670,
        ID_INPVOL = 6671,
        ID_INPISSUE = 6672,
        ID_INPPAGESFROM = 6673,
        ID_INPPAGESTO = 6674
    };
////@end CInPressRefPanel member variables
};

END_NCBI_SCOPE

#endif
    // GUI_PKG_SEQ_EDIT___INPRESSREF_PANEL__HPP
