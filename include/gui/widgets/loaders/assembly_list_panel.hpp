#ifndef GUI_WIDGETS___LOADERS___ASSEMBLY_LIST_PANEL__HPP
#define GUI_WIDGETS___LOADERS___ASSEMBLY_LIST_PANEL__HPP

/*  $Id: assembly_list_panel.hpp 43933 2019-09-20 20:16:45Z katargir $
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
 *  and reliability of the software and data, the NLM and the U.S.
 *  Government do not and cannot warrant the performance or results that
 *  may be obtained by using this software or data. The NLM and the U.S.
 *  Government disclaim all warranties, express or implied, including
 *  warranties of performance, merchantability or fitness for any particular
 *  purpose.
 *
 *  Please cite the author in any work or product based on this material.
 *
 * ===========================================================================
 *
 * Authors:  Liangshou Wu
 *
 * File Description:
 *
 */

/*!
 * Includes
 */

////@begin includes
////@end includes

#include <corelib/ncbiobj.hpp>
#include <gui/gui_export.h>

#include <gui/utils/job_future.hpp>

#include <gui/objects/DL_AssmInfo.hpp>
// This header must (at least indirectly) precede any wxWidgets headers.
#include <gui/widgets/wx/fixed_base.hpp>
#include <wx/panel.h>

/*!
 * Forward declarations
 */

class wxRadioButton;
class wxStaticBox;
class wxStaticText;
class wxTextCompleter;

BEGIN_NCBI_SCOPE

////@begin forward declarations
class CAutoCompleteTextCtrl;
class CwxTableListCtrl;
////@end forward declarations

class CAutoCompleteTextCtrl;

/*!
 * Control identifiers
 */

////@begin control identifiers
#define SYMBOL_CASSEMBLYLISTPANEL_STYLE wxWANTS_CHARS|wxTAB_TRAVERSAL
#define SYMBOL_CASSEMBLYLISTPANEL_TITLE _("Assembly list")
#define SYMBOL_CASSEMBLYLISTPANEL_IDNAME ID_CASSEMBLYLISTPANEL
#define SYMBOL_CASSEMBLYLISTPANEL_SIZE wxSize(266, 184)
#define SYMBOL_CASSEMBLYLISTPANEL_POSITION wxDefaultPosition
////@end control identifiers


/*!
 * CAssemblyListPanel class declaration
 */

class NCBI_GUIWIDGETS_LOADERS_EXPORT CAssemblyListPanel: public wxPanel
{    
    DECLARE_DYNAMIC_CLASS( CAssemblyListPanel )
    DECLARE_EVENT_TABLE()

public:
    CAssemblyListPanel();
    CAssemblyListPanel( wxWindow* parent, wxWindowID id = SYMBOL_CASSEMBLYLISTPANEL_IDNAME, const wxPoint& pos = SYMBOL_CASSEMBLYLISTPANEL_POSITION, const wxSize& size = SYMBOL_CASSEMBLYLISTPANEL_SIZE, long style = SYMBOL_CASSEMBLYLISTPANEL_STYLE );

    bool Create( wxWindow* parent, wxWindowID id = SYMBOL_CASSEMBLYLISTPANEL_IDNAME, const wxPoint& pos = SYMBOL_CASSEMBLYLISTPANEL_POSITION, const wxSize& size = SYMBOL_CASSEMBLYLISTPANEL_SIZE, long style = SYMBOL_CASSEMBLYLISTPANEL_STYLE );

    ~CAssemblyListPanel();

    void Init();

    void CreateControls();
    void SetMainTitle(const wxString& title);
    void SetDescr(const wxString& descr);

    string GetSearchTerm() const;
    void SetSearchTerm(const string& term);

    void SetMultiSelection(bool flag);

    /// Get accessions of all selected assemblies
    vector<string> GetSelectedAssemblies() const;

    /// Get the name and (if available) description for an accession
    void GetAssmNameAndDesc(const string& accession,
                            string& name,
                            string& description) const;

    bool IsInputValid();

    void SetRegistryPath( const string& path );
    void LoadSettings();
    void SaveSettings() const;

////@begin CAssemblyListPanel event handler declarations

    /// wxEVT_COMMAND_TEXT_ENTER event handler for ID_TEXTCTRL7
    void OnFindEnter( wxCommandEvent& event );

    /// wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_BUTTON4
    void OnFindGenomes( wxCommandEvent& event );

    /// wxEVT_COMMAND_RADIOBUTTON_SELECTED event handler for ID_ALLBUTTON
    void OnAllbuttonSelected( wxCommandEvent& event );

    /// wxEVT_COMMAND_RADIOBUTTON_SELECTED event handler for ID_REFSEQBUTTON
    void OnRefseqbuttonSelected( wxCommandEvent& event );

    /// wxEVT_COMMAND_RADIOBUTTON_SELECTED event handler for ID_GENBANKBUTTON
    void OnGenbankbuttonSelected( wxCommandEvent& event );

////@end CAssemblyListPanel event handler declarations

////@begin CAssemblyListPanel member function declarations

    /// Retrieves bitmap resources
    wxBitmap GetBitmapResource( const wxString& name );

    /// Retrieves icon resources
    wxIcon GetIconResource( const wxString& name );
////@end CAssemblyListPanel member function declarations

    static bool ShowToolTips();

////@begin CAssemblyListPanel member variables
    CAutoCompleteTextCtrl* m_Term;
    CwxTableListCtrl* m_ItemList;
    wxRadioButton* m_ReleaseTypeAll;
    wxRadioButton* m_ReleaseTypeRefSeq;
    wxRadioButton* m_ReleaseTypeGenBank;
    /// Control identifiers
    enum {
        ID_CASSEMBLYLISTPANEL = 10061,
        ID_TEXTCTRL7 = 10119,
        ID_BUTTON4 = 10120,
        ID_WINDOW = 10062,
        ID_ALLBUTTON = 10066,
        ID_REFSEQBUTTON = 10058,
        ID_GENBANKBUTTON = 10065
    };
////@end CAssemblyListPanel member variables

private:
    void x_UpdateListPanel();
    void x_LoadTableSettings();
    void x_UpdateSearchTerm();
    void x_ShowStatus(const string& status);
    void x_OnFindGenomes();

private:
    /// indicates if multiple selections are allowed.
    bool           m_MultiSelection;
    vector<string> m_Assms;
    objects::CDL_AssmInfo::TAssemblies m_OrigItems;
    int            m_ReleaseType;

    string         m_RegPath;

    void x_SetTextCompleter(wxTextCompleter* textCompleter);

    string         m_SearchTerm;
    list<wxString> m_TermHistrory;

    std::unique_ptr<async_job> m_Future;
    string  m_SearchingFor;
};

END_NCBI_SCOPE

#endif // GUI_WIDGETS___LOADERS___ASSEMBLY_LIST_PANEL__HPP
