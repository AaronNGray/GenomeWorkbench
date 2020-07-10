#ifndef GUI_PKG_SEQ_EDIT___PUBAUTHOR_PANEL__HPP
#define GUI_PKG_SEQ_EDIT___PUBAUTHOR_PANEL__HPP
/*  $Id: pubauthor_panel.hpp 42811 2019-04-18 10:15:32Z bollin $
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
#include <wx/hyperlink.h>
#include <objects/biblio/Cit_sub.hpp>

class wxScrolledWindow;
class wxFlexGridSizer;


BEGIN_NCBI_SCOPE

/*!
 * Control identifiers
 */

////@begin control identifiers
#define SYMBOL_CPUBAUTHORPANEL_STYLE wxTAB_TRAVERSAL
#define SYMBOL_CPUBAUTHORPANEL_TITLE _("Reference Author Panel")
#define SYMBOL_CPUBAUTHORPANEL_IDNAME ID_CPUBAUTHORPANEL
#define SYMBOL_CPUBAUTHORPANEL_SIZE wxDefaultSize
#define SYMBOL_CPUBAUTHORPANEL_POSITION wxDefaultPosition
////@end control identifiers


/*!
 * CPubAuthorPanel class declaration
 */

class CPubAuthorPanel: public wxPanel
{    
    DECLARE_DYNAMIC_CLASS( CPubAuthorPanel )
    DECLARE_EVENT_TABLE()

public:
    /// Constructors
    CPubAuthorPanel();
    CPubAuthorPanel( wxWindow* parent, 
        wxWindowID id = SYMBOL_CPUBAUTHORPANEL_IDNAME, 
        const wxPoint& pos = SYMBOL_CPUBAUTHORPANEL_POSITION, 
        const wxSize& size = SYMBOL_CPUBAUTHORPANEL_SIZE, 
        long style = SYMBOL_CPUBAUTHORPANEL_STYLE );

    /// Creation
    bool Create( wxWindow* parent, 
        wxWindowID id = SYMBOL_CPUBAUTHORPANEL_IDNAME, 
        const wxPoint& pos = SYMBOL_CPUBAUTHORPANEL_POSITION, 
        const wxSize& size = SYMBOL_CPUBAUTHORPANEL_SIZE, 
        long style = SYMBOL_CPUBAUTHORPANEL_STYLE );

    /// Destructor
    ~CPubAuthorPanel();

    /// Initialises member variables
    void Init();

    /// Creates the controls and sizers
    void CreateControls();

    virtual bool TransferDataToWindow();
    virtual bool TransferDataFromWindow();

    void ApplyCitSub(objects::CCit_sub& sub);


////@begin CPubAuthorPanel event handler declarations

    /// wxEVT_COMMAND_HYPERLINK event handler for ID_MOLADDCHROMOSOME
    void OnAddAuthorClicked( wxHyperlinkEvent& event );

    /// wxEVT_COMMAND_HYPERLINK event handler for ID_MOLDELCHROMOSOME
    void OnAddConsortiumClicked( wxHyperlinkEvent& event );

////@end CPubAuthorPanel event handler declarations

////@begin CPubAuthorPanel member function declarations

    /// Retrieves bitmap resources
    wxBitmap GetBitmapResource( const wxString& name );

    /// Retrieves icon resources
    wxIcon GetIconResource( const wxString& name );
////@end CPubAuthorPanel member function declarations

    /// Should we show tooltips?
    static bool ShowToolTips();
private:
    void x_AddAuthorRow();
    void x_AddConsortiumRow();
    void x_AddRowToWindow(wxWindow* row);

    size_t m_Rows{ 3 };
    int m_RowHeight{ 0 };

    CRef<objects::CCit_sub> m_Sub;

////@begin CPubAuthorPanel member variables
    wxScrolledWindow* m_ScrolledWindow;
    wxFlexGridSizer* m_Sizer;
public:
    /// Control identifiers
    enum {
        ID_CPUBAUTHORPANEL = 6645,
        ID_REFAUTHORSCROLLEDWND = 6646,
        ID_MOLADDCHROMOSOME = 6647,
        ID_MOLDELCHROMOSOME = 6648
    };
////@end CPubAuthorPanel member variables
};

END_NCBI_SCOPE

#endif
    // GUI_PKG_SEQ_EDIT___PUBAUTHOR_PANEL__HPP
