/*  $Id: ref_gene_track_loc_list_panel.hpp 40134 2017-12-22 17:13:45Z filippov $
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
#ifndef _REF_GENE_TRACK_LOC_LIST_PANEL_H_
#define _REF_GENE_TRACK_LOC_LIST_PANEL_H_

#include <corelib/ncbistd.hpp>
#include <objects/general/User_object.hpp>
#include <objects/general/User_field.hpp>
#include <gui/widgets/edit/user_object_util.hpp>

/*!
 * Includes
 */

////@begin includes
////@end includes
#include <wx/sizer.h>
#include <wx/panel.h>
#include <wx/stattext.h>
#include <wx/scrolwin.h>

/*!
 * Forward declarations
 */

////@begin forward declarations
////@end forward declarations

class wxChoice;

BEGIN_NCBI_SCOPE

/*!
 * Control identifiers
 */

////@begin control identifiers
#define SYMBOL_CREFGENETRACKLOCLISTPANEL_STYLE wxTAB_TRAVERSAL
#define SYMBOL_CREFGENETRACKLOCLISTPANEL_TITLE _("RefGeneTrackLocListPanel")
#define SYMBOL_CREFGENETRACKLOCLISTPANEL_IDNAME ID_CREFGENETRACKLOCLISTPANEL
#define SYMBOL_CREFGENETRACKLOCLISTPANEL_SIZE wxSize(400, 300)
#define SYMBOL_CREFGENETRACKLOCLISTPANEL_POSITION wxDefaultPosition
////@end control identifiers


/*!
 * CRefGeneTrackLocListPanel class declaration
 */

class CRefGeneTrackLocListPanel : public CUserFieldListPanel
{    
    DECLARE_DYNAMIC_CLASS( CRefGeneTrackLocListPanel )
    DECLARE_EVENT_TABLE()

public:
    /// Constructors
    CRefGeneTrackLocListPanel();
    CRefGeneTrackLocListPanel( wxWindow* parent, wxWindowID id = SYMBOL_CREFGENETRACKLOCLISTPANEL_IDNAME, const wxPoint& pos = SYMBOL_CREFGENETRACKLOCLISTPANEL_POSITION, const wxSize& size = SYMBOL_CREFGENETRACKLOCLISTPANEL_SIZE, long style = SYMBOL_CREFGENETRACKLOCLISTPANEL_STYLE );

    /// Creation
    bool Create( wxWindow* parent, wxWindowID id = SYMBOL_CREFGENETRACKLOCLISTPANEL_IDNAME, const wxPoint& pos = SYMBOL_CREFGENETRACKLOCLISTPANEL_POSITION, const wxSize& size = SYMBOL_CREFGENETRACKLOCLISTPANEL_SIZE, long style = SYMBOL_CREFGENETRACKLOCLISTPANEL_STYLE );

    /// Destructor
    ~CRefGeneTrackLocListPanel();

    /// Initialises member variables
    void Init();

    /// Creates the controls and sizers
    void CreateControls();

////@begin CRefGeneTrackLocListPanel event handler declarations

    /// wxEVT_COMMAND_CHOICE_SELECTED event handler for ID_TYPE_CHOICE
    void OnTypeChoiceSelected( wxCommandEvent& event );

////@end CRefGeneTrackLocListPanel event handler declarations

////@begin CRefGeneTrackLocListPanel member function declarations

    /// Retrieves bitmap resources
    wxBitmap GetBitmapResource( const wxString& name );

    /// Retrieves icon resources
    wxIcon GetIconResource( const wxString& name );
////@end CRefGeneTrackLocListPanel member function declarations

    /// Should we show tooltips?
    static bool ShowToolTips();

////@begin CRefGeneTrackLocListPanel member variables
    wxChoice* m_Type;
    /// Control identifiers
    enum {
        ID_CREFGENETRACKLOCLISTPANEL = 10156,
        ID_TYPE_CHOICE = 10155
    };
////@end CRefGeneTrackLocListPanel member variables

    virtual bool TransferDataToWindow();
    virtual bool TransferDataFromWindow();

    bool IsEmpty();
};

END_NCBI_SCOPE

/*!
 * CRefGeneTrackLocListPanel class declaration
 */

class CRefGeneTrackLocListPanel: public wxPanel
{    
    DECLARE_DYNAMIC_CLASS( CRefGeneTrackLocListPanel )
    DECLARE_EVENT_TABLE()

public:
    /// Constructors
    CRefGeneTrackLocListPanel();
    CRefGeneTrackLocListPanel( wxWindow* parent, wxWindowID id = SYMBOL_CREFGENETRACKLOCLISTPANEL_IDNAME, const wxPoint& pos = SYMBOL_CREFGENETRACKLOCLISTPANEL_POSITION, const wxSize& size = SYMBOL_CREFGENETRACKLOCLISTPANEL_SIZE, long style = SYMBOL_CREFGENETRACKLOCLISTPANEL_STYLE );

    /// Creation
    bool Create( wxWindow* parent, wxWindowID id = SYMBOL_CREFGENETRACKLOCLISTPANEL_IDNAME, const wxPoint& pos = SYMBOL_CREFGENETRACKLOCLISTPANEL_POSITION, const wxSize& size = SYMBOL_CREFGENETRACKLOCLISTPANEL_SIZE, long style = SYMBOL_CREFGENETRACKLOCLISTPANEL_STYLE );

    /// Destructor
    ~CRefGeneTrackLocListPanel();

    /// Initialises member variables
    void Init();

    /// Creates the controls and sizers
    void CreateControls();

////@begin CRefGeneTrackLocListPanel event handler declarations
////@end CRefGeneTrackLocListPanel event handler declarations

////@begin CRefGeneTrackLocListPanel member function declarations
    /// Retrieves bitmap resources
    wxBitmap GetBitmapResource( const wxString& name );

    /// Retrieves icon resources
    wxIcon GetIconResource( const wxString& name );
////@end CRefGeneTrackLocListPanel member function declarations

    /// Should we show tooltips?
    static bool ShowToolTips();

////@begin CRefGeneTrackLocListPanel member variables
    /// Control identifiers
    enum {
        ID_CREFGENETRACKLOCLISTPANEL = 10156
    };
////@end CRefGeneTrackLocListPanel member variables
};

#endif
// _REF_GENE_TRACK_LOC_LIST_PANEL_H_

