/*  $Id: single_ref_gene_track_loc_panel.hpp 40132 2017-12-22 15:45:12Z bollin $
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
#ifndef _SINGLE_REF_GENE_TRACK_LOC_PANEL_H_
#define _SINGLE_REF_GENE_TRACK_LOC_PANEL_H_

#include <corelib/ncbistd.hpp>
#include <objects/general/User_object.hpp>
#include <objects/general/User_field.hpp>
#include <gui/widgets/edit/user_object_util.hpp>

/*!
 * Includes
 */

////@begin includes
////@end includes

#include <wx/panel.h>
#include <wx/frame.h>
#include <wx/sizer.h>
#include <wx/bitmap.h>
#include <wx/icon.h>
#include <wx/choice.h>
#include <wx/textctrl.h>
#include <wx/stattext.h>

/*!
 * Forward declarations
 */

////@begin forward declarations
////@end forward declarations

BEGIN_NCBI_SCOPE

/*!
 * Control identifiers
 */

////@begin control identifiers
#define SYMBOL_CSINGLEREFGENETRACKLOCPANEL_STYLE wxTAB_TRAVERSAL
#define SYMBOL_CSINGLEREFGENETRACKLOCPANEL_TITLE _("SingleRefGeneTrackLocPanel")
#define SYMBOL_CSINGLEREFGENETRACKLOCPANEL_IDNAME ID_CSINGLEREFGENETRACKLOCPANEL
#define SYMBOL_CSINGLEREFGENETRACKLOCPANEL_SIZE wxSize(400, 300)
#define SYMBOL_CSINGLEREFGENETRACKLOCPANEL_POSITION wxDefaultPosition
////@end control identifiers


/*!
 * CSingleRefGeneTrackLocPanel class declaration
 */

class CSingleRefGeneTrackLocPanel: public CSingleUserFieldPanel
{    
    DECLARE_DYNAMIC_CLASS( CSingleRefGeneTrackLocPanel )
    DECLARE_EVENT_TABLE()

public:
    /// Constructors
    CSingleRefGeneTrackLocPanel();
    CSingleRefGeneTrackLocPanel( wxWindow* parent, objects::CUser_field& field, wxWindowID id = SYMBOL_CSINGLEREFGENETRACKLOCPANEL_IDNAME, const wxPoint& pos = SYMBOL_CSINGLEREFGENETRACKLOCPANEL_POSITION, const wxSize& size = SYMBOL_CSINGLEREFGENETRACKLOCPANEL_SIZE, long style = SYMBOL_CSINGLEREFGENETRACKLOCPANEL_STYLE );

    /// Creation
    bool Create( wxWindow* parent, wxWindowID id = SYMBOL_CSINGLEREFGENETRACKLOCPANEL_IDNAME, const wxPoint& pos = SYMBOL_CSINGLEREFGENETRACKLOCPANEL_POSITION, const wxSize& size = SYMBOL_CSINGLEREFGENETRACKLOCPANEL_SIZE, long style = SYMBOL_CSINGLEREFGENETRACKLOCPANEL_STYLE );

    /// Destructor
    ~CSingleRefGeneTrackLocPanel();

    /// Initialises member variables
    void Init();

    /// Creates the controls and sizers
    void CreateControls();

    virtual bool TransferDataToWindow();
    virtual bool TransferDataFromWindow();
    CRef<objects::CUser_field> GetUser_field();

////@begin CSingleRefGeneTrackLocPanel event handler declarations

    /// wxEVT_COMMAND_TEXT_UPDATED event handler for ID_ACCESSION_TXT
    void OnAccessionTxtTextUpdated( wxCommandEvent& event );

    /// wxEVT_COMMAND_TEXT_UPDATED event handler for ID_GI_TXT
    void OnGiTxtTextUpdated( wxCommandEvent& event );

    /// wxEVT_COMMAND_TEXT_UPDATED event handler for ID_START_TXT
    void OnStartTxtTextUpdated( wxCommandEvent& event );

    /// wxEVT_COMMAND_TEXT_UPDATED event handler for ID_STOP_TXT
    void OnStopTxtTextUpdated( wxCommandEvent& event );

    /// wxEVT_COMMAND_TEXT_UPDATED event handler for ID_COMMENT_TXT
    void OnCommentTxtTextUpdated( wxCommandEvent& event );

////@end CSingleRefGeneTrackLocPanel event handler declarations

////@begin CSingleRefGeneTrackLocPanel member function declarations

    /// Retrieves bitmap resources
    wxBitmap GetBitmapResource( const wxString& name );

    /// Retrieves icon resources
    wxIcon GetIconResource( const wxString& name );
////@end CSingleRefGeneTrackLocPanel member function declarations

    /// Should we show tooltips?
    static bool ShowToolTips();

////@begin CSingleRefGeneTrackLocPanel member variables
    wxTextCtrl* m_Accession;
    wxTextCtrl* m_GI;
    wxTextCtrl* m_Start;
    wxTextCtrl* m_Stop;
    wxTextCtrl* m_Comment;
    /// Control identifiers
    enum {
        ID_CSINGLEREFGENETRACKLOCPANEL = 10149,
        ID_ACCESSION_TXT = 10150,
        ID_GI_TXT = 10151,
        ID_START_TXT = 10152,
        ID_STOP_TXT = 10153,
        ID_COMMENT_TXT = 10154
    };
////@end CSingleRefGeneTrackLocPanel member variables
private:
    CRef<objects::CUser_field> m_Field;

    bool x_IsControlled (const string& field);
};

END_NCBI_SCOPE

#endif
// _SINGLE_REF_GENE_TRACK_LOC_PANEL_H_

