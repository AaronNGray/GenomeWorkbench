#ifndef GUI_PKG_SEQ_EDIT___UNPUBLISHEDREF_PANEL__HPP
#define GUI_PKG_SEQ_EDIT___UNPUBLISHEDREF_PANEL__HPP
/*  $Id: unpublishedref_panel.hpp 43202 2019-05-28 18:05:59Z filippov $
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
#include <objects/biblio/Cit_gen.hpp>

class wxTextCtrl;

BEGIN_NCBI_SCOPE


/*!
 * Control identifiers
 */

////@begin control identifiers
#define SYMBOL_CUNPUBLISHEDREFPANEL_STYLE wxTAB_TRAVERSAL
#define SYMBOL_CUNPUBLISHEDREFPANEL_TITLE _("Unpublished Reference")
#define SYMBOL_CUNPUBLISHEDREFPANEL_IDNAME ID_CUNPUBLISHEDREFPANEL
#define SYMBOL_CUNPUBLISHEDREFPANEL_SIZE wxSize(400, 300)
#define SYMBOL_CUNPUBLISHEDREFPANEL_POSITION wxDefaultPosition
////@end control identifiers


/*!
 * CUnpublishedRefPanel class declaration
 */

class CUnpublishedRefPanel: public wxPanel
{    
    DECLARE_DYNAMIC_CLASS( CUnpublishedRefPanel )
    DECLARE_EVENT_TABLE()

public:
    /// Constructors
    CUnpublishedRefPanel();
    CUnpublishedRefPanel( wxWindow* parent, 
        wxWindowID id = SYMBOL_CUNPUBLISHEDREFPANEL_IDNAME, 
        const wxPoint& pos = SYMBOL_CUNPUBLISHEDREFPANEL_POSITION, 
        const wxSize& size = SYMBOL_CUNPUBLISHEDREFPANEL_SIZE, 
        long style = SYMBOL_CUNPUBLISHEDREFPANEL_STYLE );

    /// Creation
    bool Create( wxWindow* parent, 
        wxWindowID id = SYMBOL_CUNPUBLISHEDREFPANEL_IDNAME, 
        const wxPoint& pos = SYMBOL_CUNPUBLISHEDREFPANEL_POSITION, 
        const wxSize& size = SYMBOL_CUNPUBLISHEDREFPANEL_SIZE, 
        long style = SYMBOL_CUNPUBLISHEDREFPANEL_STYLE );

    /// Destructor
    ~CUnpublishedRefPanel();

    /// Initialises member variables
    void Init();

    /// Creates the controls and sizers
    void CreateControls();

////@begin CUnpublishedRefPanel event handler declarations

////@end CUnpublishedRefPanel event handler declarations

////@begin CUnpublishedRefPanel member function declarations

    /// Retrieves bitmap resources
    wxBitmap GetBitmapResource( const wxString& name );

    /// Retrieves icon resources
    wxIcon GetIconResource( const wxString& name );
////@end CUnpublishedRefPanel member function declarations

    /// Should we show tooltips?
    static bool ShowToolTips();

    void SetCitGen(const objects::CCit_gen& gen);
    void UpdateCitGen(objects::CCit_gen& gen);
    void ReportMissingFields(string &text);

private:
////@begin CUnpublishedRefPanel member variables
    wxTextCtrl* m_ReferenceTitle;
public:
    /// Control identifiers
    enum {
        ID_CUNPUBLISHEDREFPANEL = 6660,
        ID_REFTITLETXTCTRL = 6661
    };
////@end CUnpublishedRefPanel member variables
};

END_NCBI_SCOPE

#endif
    // GUI_PKG_SEQ_EDIT___UNPUBLISHEDREF_PANEL__HPP
