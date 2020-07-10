/*  $Id: bulk_location_panel.hpp 34894 2016-02-26 19:03:19Z asztalos $
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
#ifndef _BULK_LOCATION_PANEL_H_
#define _BULK_LOCATION_PANEL_H_

#include <corelib/ncbistd.hpp>
#include <objects/seqloc/Seq_loc.hpp>
#include <objects/seqloc/Seq_id.hpp>
#include <objects/seq/Bioseq.hpp>

/*!
 * Includes
 */

////@begin includes
////@end includes
#include <wx/panel.h>
#include <wx/checkbox.h>
#include <wx/radiobut.h>
#include <wx/radiobox.h>
#include <wx/textctrl.h>

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
#define SYMBOL_CBULKLOCATIONPANEL_STYLE wxTAB_TRAVERSAL
#define SYMBOL_CBULKLOCATIONPANEL_TITLE _("BulkLocationPanel")
#define SYMBOL_CBULKLOCATIONPANEL_IDNAME ID_CBULKLOCATIONPANEL
#define SYMBOL_CBULKLOCATIONPANEL_SIZE wxSize(400, 500)
#define SYMBOL_CBULKLOCATIONPANEL_POSITION wxDefaultPosition
////@end control identifiers


/*!
 * CBulkLocationPanel class declaration
 */

class CBulkLocationPanel: public wxPanel
{    
    DECLARE_DYNAMIC_CLASS( CBulkLocationPanel )
    DECLARE_EVENT_TABLE()

public:
    /// Constructors
    CBulkLocationPanel();
    CBulkLocationPanel( wxWindow* parent, wxWindowID id = SYMBOL_CBULKLOCATIONPANEL_IDNAME, const wxPoint& pos = SYMBOL_CBULKLOCATIONPANEL_POSITION, const wxSize& size = SYMBOL_CBULKLOCATIONPANEL_SIZE, long style = SYMBOL_CBULKLOCATIONPANEL_STYLE );

    /// Creation
    bool Create( wxWindow* parent, wxWindowID id = SYMBOL_CBULKLOCATIONPANEL_IDNAME, const wxPoint& pos = SYMBOL_CBULKLOCATIONPANEL_POSITION, const wxSize& size = SYMBOL_CBULKLOCATIONPANEL_SIZE, long style = SYMBOL_CBULKLOCATIONPANEL_STYLE );

    /// Destructor
    ~CBulkLocationPanel();

    /// Initialises member variables
    void Init();

    /// Creates the controls and sizers
    void CreateControls();

////@begin CBulkLocationPanel event handler declarations

    /// wxEVT_COMMAND_RADIOBUTTON_SELECTED event handler for ID_BULKLOC_RBTN1
    void OnBulklocRbtn1Selected( wxCommandEvent& event );

    /// wxEVT_COMMAND_RADIOBUTTON_SELECTED event handler for ID_BULKLOC_RBTN2
    void OnBulklocRbtn2Selected( wxCommandEvent& event );

////@end CBulkLocationPanel event handler declarations

////@begin CBulkLocationPanel member function declarations

    /// Retrieves bitmap resources
    wxBitmap GetBitmapResource( const wxString& name );

    /// Retrieves icon resources
    wxIcon GetIconResource( const wxString& name );
////@end CBulkLocationPanel member function declarations

    /// Should we show tooltips?
    static bool ShowToolTips();

////@begin CBulkLocationPanel member variables
    wxCheckBox* m_Partial5;
    wxCheckBox* m_Partial3;
    wxRadioBox* m_Strand;
    wxRadioButton* m_WholeSeq;
    wxRadioButton* m_Coordinates;
    wxTextCtrl* m_From;
    wxTextCtrl* m_To;
    wxCheckBox* m_AddToExisting;
    /// Control identifiers
    enum {
        ID_CBULKLOCATIONPANEL = 6410,
        ID_BULKLOC_CHCKBOX1 = 6411,
        ID_BULKLOC_CHCKBOX2 = 6412,
        ID_BULKLOC_RBOX = 6413,
        ID_BULKLOC_RBTN1 = 6414,
        ID_BULKLOC_RBTN2 = 6415,
        ID_BULKLOC_TEXTCTRL1 = 6416,
        ID_BULKLOC_TEXTCTRL2 = 6417,
        ID_BULKLOC_CHCKBOX3 = 6418
    };
////@end CBulkLocationPanel member variables

    CRef<objects::CSeq_loc> GetLocForBioseq (const objects::CBioseq& bioseq);
    bool ShouldAddToExisting() {return m_AddToExisting->GetValue();};
};

END_NCBI_SCOPE

#endif
    // _BULK_LOCATION_PANEL_H_
