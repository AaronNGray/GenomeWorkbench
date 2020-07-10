/*  $Id: ref_gene_tracking_panel.hpp 40132 2017-12-22 15:45:12Z bollin $
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
 * Authors:  Roman Katargin
 */
#ifndef _REF_GENE_TRACKING_PANEL_H_
#define _REF_GENE_TRACKING_PANEL_H_

#include <corelib/ncbistd.hpp>
#include <gui/widgets/edit/ref_gene_track_loc_list_panel.hpp>
#include <objects/general/User_object.hpp>
#include <objects/general/User_field.hpp>
#include <gui/widgets/edit/utilities.hpp>

/*!
 * Includes
 */

////@begin includes
////@end includes

#include <wx/panel.h>
#include <wx/sizer.h>
#include <wx/radiobut.h>
#include <wx/textctrl.h>
#include <wx/checkbox.h>

/*!
 * Forward declarations
 */

////@begin forward declarations
class wxBoxSizer;
class CRefGeneTrackLocListPanel;
////@end forward declarations

BEGIN_NCBI_SCOPE

/*!
 * Control identifiers
 */

////@begin control identifiers
#define SYMBOL_CREFGENETRACKINGPANEL_STYLE wxTAB_TRAVERSAL
#define SYMBOL_CREFGENETRACKINGPANEL_TITLE _("RefGeneTrackingPanel")
#define SYMBOL_CREFGENETRACKINGPANEL_IDNAME ID_CREFGENETRACKINGPANEL
#define SYMBOL_CREFGENETRACKINGPANEL_SIZE wxSize(400, 300)
#define SYMBOL_CREFGENETRACKINGPANEL_POSITION wxDefaultPosition
////@end control identifiers


/*!
 * CRefGeneTrackingPanel class declaration
 */

class CRefGeneTrackingPanel: public wxPanel, public IDescEditorPanel
{    
    DECLARE_DYNAMIC_CLASS( CRefGeneTrackingPanel )
    DECLARE_EVENT_TABLE()

public:
    /// Constructors
    CRefGeneTrackingPanel();
    CRefGeneTrackingPanel( wxWindow* parent, wxWindowID id = SYMBOL_CREFGENETRACKINGPANEL_IDNAME, const wxPoint& pos = SYMBOL_CREFGENETRACKINGPANEL_POSITION, const wxSize& size = SYMBOL_CREFGENETRACKINGPANEL_SIZE, long style = SYMBOL_CREFGENETRACKINGPANEL_STYLE );

    /// Creation
    bool Create( wxWindow* parent, wxWindowID id = SYMBOL_CREFGENETRACKINGPANEL_IDNAME, const wxPoint& pos = SYMBOL_CREFGENETRACKINGPANEL_POSITION, const wxSize& size = SYMBOL_CREFGENETRACKINGPANEL_SIZE, long style = SYMBOL_CREFGENETRACKINGPANEL_STYLE );

    /// Destructor
    ~CRefGeneTrackingPanel();

    /// Initialises member variables
    void Init();

    /// Creates the controls and sizers
    void CreateControls();

////@begin CRefGeneTrackingPanel event handler declarations

    /// wxEVT_COMMAND_TEXT_UPDATED event handler for ID_TEXTCTRL9
    void OnGenomicSourceTextUpdated( wxCommandEvent& event );

////@end CRefGeneTrackingPanel event handler declarations

////@begin CRefGeneTrackingPanel member function declarations

    /// Retrieves bitmap resources
    wxBitmap GetBitmapResource( const wxString& name );

    /// Retrieves icon resources
    wxIcon GetIconResource( const wxString& name );
////@end CRefGeneTrackingPanel member function declarations

    /// Should we show tooltips?
    static bool ShowToolTips();

////@begin CRefGeneTrackingPanel member variables
    wxBoxSizer* m_StatusSizer;
    wxRadioButton* m_InferredBtn;
    wxRadioButton* m_PredictedBtn;
    wxRadioButton* m_ProvisionalBtn;
    wxRadioButton* m_ValidatedBtn;
    wxRadioButton* m_ReviewedBtn;
    wxRadioButton* m_ModelBtn;
    wxRadioButton* m_WgsBtn;
    wxRadioButton* m_PipelineBtn;
    wxCheckBox* m_GeneratedBtn;
    wxTextCtrl* m_CuratorTxt;
    wxTextCtrl* m_GenomicSourceTxt;
    wxTextCtrl* m_URLTxt;
    CRefGeneTrackLocListPanel* m_AccessionsPanel;
    /// Control identifiers
    enum {
        ID_CREFGENETRACKINGPANEL = 10136,
        ID_RADIOBUTTON = 10137,
        ID_RADIOBUTTON1 = 10138,
        ID_RADIOBUTTON2 = 10139,
        ID_RADIOBUTTON3 = 10140,
        ID_RADIOBUTTON4 = 10141,
        ID_RADIOBUTTON5 = 10142,
        ID_RADIOBUTTON6 = 10143,
        ID_RADIOBUTTON7 = 10144,
        ID_CHECKBOX16 = 10145,
        ID_TEXTCTRL4 = 10146,
        ID_TEXTCTRL9 = 10147,
        ID_TEXTCTRL21 = 10148,
        ID_FOREIGN = 10157
    };
////@end CRefGeneTrackingPanel member variables
    void SetUser_object(CRef<objects::CUser_object> user);
    CRef<objects::CUser_object> GetUser_object();

    // IDescEditorPanel implementation
    virtual void ChangeSeqdesc(const objects::CSeqdesc& desc);
    virtual void UpdateSeqdesc(objects::CSeqdesc& desc);

private:
    CRef<objects::CUser_object> m_User;

    void x_SetStatus(const string& val);
    string x_GetStatus();
    bool x_IsControlledField(const string& field);
    
};

END_NCBI_SCOPE

#endif
// _REF_GENE_TRACKING_PANEL_H_
//
