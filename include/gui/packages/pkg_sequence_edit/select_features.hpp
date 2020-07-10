/*  $Id: select_features.hpp 39649 2017-10-24 15:22:12Z asztalos $
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
 * Authors:  Igor Filippov
 */
#ifndef _SELECT_FEATURES_H_
#define _SELECT_FEATURES_H_

#include <corelib/ncbistd.hpp>
#include <gui/framework/workbench.hpp>
#include <gui/widgets/data/report_dialog.hpp>
#include <gui/packages/pkg_sequence_edit/string_constraint_panel.hpp>
#include <gui/packages/pkg_sequence_edit/field_choice_panel.hpp>
#include <gui/widgets/edit/feature_field_name_panel.hpp>
#include <gui/widgets/edit/feature_type_panel.hpp>
#include <wx/dialog.h>
#include <wx/sizer.h>
#include <wx/button.h>

/*!
 * Includes
 */

////@begin includes
////@end includes

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
#define ID_CSELECTFEATURES 10409
#define ID_SEL_FEAT_ACCEPT 10411
#define ID_SEL_FEAT_CANCEL 10412
#define SYMBOL_CSELECTFEATURES_STYLE wxCAPTION|wxRESIZE_BORDER|wxSYSTEM_MENU|wxCLOSE_BOX|wxTAB_TRAVERSAL
#define SYMBOL_CSELECTFEATURES_TITLE _("Select Features")
#define SYMBOL_CSELECTFEATURES_IDNAME ID_CSELECTFEATURES
#define SYMBOL_CSELECTFEATURES_SIZE wxSize(400, 300)
#define SYMBOL_CSELECTFEATURES_POSITION wxDefaultPosition
////@end control identifiers


/*!
 * CSelectFeatures class declaration
 */

class CSelectFeatures : public CReportEditingDialog
{    
    DECLARE_DYNAMIC_CLASS( CSelectFeatures )
    DECLARE_EVENT_TABLE()

public:
    /// Constructors
    CSelectFeatures();
    CSelectFeatures( wxWindow* parent, IWorkbench* wb, wxWindowID id = SYMBOL_CSELECTFEATURES_IDNAME, const wxString& caption = SYMBOL_CSELECTFEATURES_TITLE, const wxPoint& pos = SYMBOL_CSELECTFEATURES_POSITION, const wxSize& size = SYMBOL_CSELECTFEATURES_SIZE, long style = SYMBOL_CSELECTFEATURES_STYLE );

    /// Creation
    bool Create( wxWindow* parent, wxWindowID id = SYMBOL_CSELECTFEATURES_IDNAME, const wxString& caption = SYMBOL_CSELECTFEATURES_TITLE, const wxPoint& pos = SYMBOL_CSELECTFEATURES_POSITION, const wxSize& size = SYMBOL_CSELECTFEATURES_SIZE, long style = SYMBOL_CSELECTFEATURES_STYLE );

    /// Destructor
    ~CSelectFeatures();

    /// Initialises member variables
    void Init();

    /// Creates the controls and sizers
    void CreateControls();

////@begin CSelectFeatures event handler declarations

    /// wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_SEL_TARGET_ACCEPT
    void OnAccept( wxCommandEvent& event );
   

    /// wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_SEL_TARGET_CANCEL
    void OnCancel( wxCommandEvent& event );

////@end CSelectFeatures event handler declarations

////@begin CSelectFeatures member function declarations

    /// Retrieves bitmap resources
    wxBitmap GetBitmapResource( const wxString& name );

    /// Retrieves icon resources
    wxIcon GetIconResource( const wxString& name );
////@end CSelectFeatures member function declarations

    /// Should we show tooltips?
    static bool ShowToolTips();
   void ProcessUpdateFeatEvent( wxCommandEvent& event );

private:
    void UpdateChildrenFeaturePanels( wxSizer* sizer );
    CConstRef<CObject> GetMainObject();
    IWorkbench*     m_Workbench;
    CFeatureTypePanel* m_FeatureType;
    CStringConstraintPanel *m_StringConstraintPanel;
    CFeatureFieldNamePanel* m_FeatureConstraint;
    CSeq_entry_Handle m_TopSeqEntry;
};

END_NCBI_SCOPE

#endif
    // _SELECT_FEATURES_H_
