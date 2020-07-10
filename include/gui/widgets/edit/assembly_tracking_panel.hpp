/*  $Id: assembly_tracking_panel.hpp 31212 2014-09-12 12:45:01Z bollin $
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
#ifndef _ASSEMBLY_TRACKING_PANEL_H_
#define _ASSEMBLY_TRACKING_PANEL_H_

#include <corelib/ncbistd.hpp>
#include <objects/seq/Bioseq.hpp>
#include <objects/seqalign/Dense_seg.hpp>
#include <gui/widgets/edit/assembly_tracking_list_panel.hpp>
#include <gui/widgets/edit/utilities.hpp>

/*!
 * Includes
 */

////@begin includes
////@end includes

/*!
 * Forward declarations
 */

////@begin forward declarations
class CAssemblyTrackingListPanel;
////@end forward declarations

BEGIN_NCBI_SCOPE

/*!
 * Control identifiers
 */

////@begin control identifiers
#define SYMBOL_CASSEMBLYTRACKINGPANEL_STYLE wxTAB_TRAVERSAL
#define SYMBOL_CASSEMBLYTRACKINGPANEL_TITLE _("Assembly Tracking")
#define SYMBOL_CASSEMBLYTRACKINGPANEL_IDNAME ID_CASSEMBLYTRACKINGPANEL
#define SYMBOL_CASSEMBLYTRACKINGPANEL_SIZE wxSize(400, 300)
#define SYMBOL_CASSEMBLYTRACKINGPANEL_POSITION wxDefaultPosition
////@end control identifiers


/*!
 * CAssemblyTrackingPanel class declaration
 */

class CAssemblyTrackingPanel: public wxPanel, public IDescEditorPanel
{    
    DECLARE_DYNAMIC_CLASS( CAssemblyTrackingPanel )
    DECLARE_EVENT_TABLE()

public:
    /// Constructors
    CAssemblyTrackingPanel();
    CAssemblyTrackingPanel( wxWindow* parent, wxWindowID id = SYMBOL_CASSEMBLYTRACKINGPANEL_IDNAME, const wxPoint& pos = SYMBOL_CASSEMBLYTRACKINGPANEL_POSITION, const wxSize& size = SYMBOL_CASSEMBLYTRACKINGPANEL_SIZE, long style = SYMBOL_CASSEMBLYTRACKINGPANEL_STYLE );

    /// Creation
    bool Create( wxWindow* parent, wxWindowID id = SYMBOL_CASSEMBLYTRACKINGPANEL_IDNAME, const wxPoint& pos = SYMBOL_CASSEMBLYTRACKINGPANEL_POSITION, const wxSize& size = SYMBOL_CASSEMBLYTRACKINGPANEL_SIZE, long style = SYMBOL_CASSEMBLYTRACKINGPANEL_STYLE );

    /// Destructor
    ~CAssemblyTrackingPanel();

    /// Initialises member variables
    void Init();

    /// Creates the controls and sizers
    void CreateControls();

////@begin CAssemblyTrackingPanel event handler declarations

    /// wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_EXPORT_LIST_BTN
    void OnExportListBtnClick( wxCommandEvent& event );

    /// wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_EXPORT_INTERVALS_BTN
    void OnExportIntervalsBtnClick( wxCommandEvent& event );

    /// wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_IMPORT_INTERVALS_BTN
    void OnImportIntervalsBtnClick( wxCommandEvent& event );

    /// wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_POPULATE_BTN
    void OnPopulateBtnClick( wxCommandEvent& event );

////@end CAssemblyTrackingPanel event handler declarations

////@begin CAssemblyTrackingPanel member function declarations

    /// Retrieves bitmap resources
    wxBitmap GetBitmapResource( const wxString& name );

    /// Retrieves icon resources
    wxIcon GetIconResource( const wxString& name );
////@end CAssemblyTrackingPanel member function declarations

    /// Should we show tooltips?
    static bool ShowToolTips();

////@begin CAssemblyTrackingPanel member variables
    CAssemblyTrackingListPanel* m_AccessionsPanel;
    wxButton* m_PopulateBtn;
    /// Control identifiers
    enum {
        ID_CASSEMBLYTRACKINGPANEL = 10160,
        ID_WINDOW3 = 10161,
        ID_EXPORT_LIST_BTN = 10162,
        ID_EXPORT_INTERVALS_BTN = 10163,
        ID_IMPORT_INTERVALS_BTN = 10164,
        ID_POPULATE_BTN = 10165
    };
////@end CAssemblyTrackingPanel member variables

    void SetUser_object(CRef<objects::CUser_object> user);
    CRef<objects::CUser_object> GetUser_object();

    // this function allows importation of TPA assembly fields from the Seq-hist on the sequence
    void SetBioseq(CConstRef<objects::CBioseq> seq);

    // IDescEditorPanel implementation
    virtual void ChangeSeqdesc(const objects::CSeqdesc& desc);
    virtual void UpdateSeqdesc(objects::CSeqdesc& desc);

private:
    CRef<objects::CUser_object> m_User;
    CConstRef<objects::CBioseq> m_Seq;

};

END_NCBI_SCOPE

#endif
    // _ASSEMBLY_TRACKING_PANEL_H_
