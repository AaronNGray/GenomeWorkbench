#ifndef GUI_PKG_SEQ_EDIT___PUBLISHEDREF_PANEL__HPP
#define GUI_PKG_SEQ_EDIT___PUBLISHEDREF_PANEL__HPP
/*  $Id: publishedref_panel.hpp 43202 2019-05-28 18:05:59Z filippov $
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
#include <objects/biblio/Cit_art.hpp>

class wxTextCtrl;
class wxChoice;

BEGIN_NCBI_SCOPE

/*!
 * Control identifiers
 */

////@begin control identifiers
#define SYMBOL_CPUBLISHEDREFPANEL_STYLE wxTAB_TRAVERSAL
#define SYMBOL_CPUBLISHEDREFPANEL_TITLE _("Published Reference")
#define SYMBOL_CPUBLISHEDREFPANEL_IDNAME ID_CPUBLISHEDREFPANEL
#define SYMBOL_CPUBLISHEDREFPANEL_SIZE wxSize(400, 300)
#define SYMBOL_CPUBLISHEDREFPANEL_POSITION wxDefaultPosition
////@end control identifiers


/*!
 * CPublishedRefPanel class declaration
 */

class CPublishedRefPanel: public wxPanel
{    
    DECLARE_DYNAMIC_CLASS( CPublishedRefPanel )
    DECLARE_EVENT_TABLE()

public:
    /// Constructors
    CPublishedRefPanel();
    CPublishedRefPanel( wxWindow* parent, bool is_in_press,
        wxWindowID id = SYMBOL_CPUBLISHEDREFPANEL_IDNAME, 
        const wxPoint& pos = SYMBOL_CPUBLISHEDREFPANEL_POSITION, 
        const wxSize& size = SYMBOL_CPUBLISHEDREFPANEL_SIZE, 
        long style = SYMBOL_CPUBLISHEDREFPANEL_STYLE );

    /// Creation
    bool Create( wxWindow* parent, 
        wxWindowID id = SYMBOL_CPUBLISHEDREFPANEL_IDNAME, 
        const wxPoint& pos = SYMBOL_CPUBLISHEDREFPANEL_POSITION, 
        const wxSize& size = SYMBOL_CPUBLISHEDREFPANEL_SIZE, 
        long style = SYMBOL_CPUBLISHEDREFPANEL_STYLE );

    /// Destructor
    ~CPublishedRefPanel();

    /// Initialises member variables
    void Init();

    /// Creates the controls and sizers
    void CreateControls();

////@begin CPublishedRefPanel event handler declarations

    /// wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_PUBPMIDLOOKBTN
    void OnLookupClick( wxCommandEvent& event );

////@end CPublishedRefPanel event handler declarations

////@begin CPublishedRefPanel member function declarations

    /// Retrieves bitmap resources
    wxBitmap GetBitmapResource( const wxString& name );

    /// Retrieves icon resources
    wxIcon GetIconResource( const wxString& name );
////@end CPublishedRefPanel member function declarations

    /// Should we show tooltips?
    static bool ShowToolTips();

    void SetCitArt(objects::CCit_art& art);
    void SetPmid(int pmid);
    int GetPmid() { return m_Pmid; }
    virtual bool TransferDataToWindow();
    virtual bool TransferDataFromWindow();
    void ReportMissingFields(string &text);

private:
    CRef<objects::CCit_art> m_Art;
    int m_Pmid;
    bool m_is_inpress;

    void x_Reset();
    void x_UpdateDOIPmidControls();
////@begin CPublishedRefPanel member variables
    wxChoice* m_ChoiceCtrl;
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
        ID_CPUBLISHEDREFPANEL = 6680,
        ID_LOOKUPCHOICE = 6681,
        ID_PUBPMIDTEXTCTRL = 6682,
        ID_PUBPMIDLOOKBTN = 6683,
        ID_PUBREFTITLE = 6684,
        ID_PUBJOURNAL = 6685,
        ID_PUBYEAR = 6686,
        ID_PUBVOL = 6687,
        ID_PUBISSUE = 6688,
        ID_PUBPAGESFROM = 6689,
        ID_PUBPAGESTO = 6690
    };
////@end CPublishedRefPanel member variables
};

END_NCBI_SCOPE

#endif
    // GUI_PKG_SEQ_EDIT___PUBLISHEDREF_PANEL__HPP
