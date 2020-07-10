/*  $Id: add_feature_between_dlg.hpp 39649 2017-10-24 15:22:12Z asztalos $
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
#ifndef _ADD_FEATURE_BETWEEN_DLG_H_
#define _ADD_FEATURE_BETWEEN_DLG_H_

#include <corelib/ncbistd.hpp>
#include <objects/seq/Seq_annot.hpp>
#include <objects/seqfeat/SeqFeatData.hpp>
#include <objmgr/seq_entry_handle.hpp>
#include <gui/packages/pkg_sequence_edit/srcedit_util.hpp>
#include <gui/packages/pkg_sequence_edit/bulk_cmd_panel.hpp>
#include <gui/packages/pkg_sequence_edit/bulk_other_add_panel.hpp>
#include <gui/packages/pkg_sequence_edit/bulk_location_panel.hpp>
#include <gui/packages/pkg_sequence_edit/bulk_cmd_dlg.hpp>
#include <gui/packages/pkg_sequence_edit/string_constraint_panel.hpp>
#include <gui/widgets/edit/feature_field_name_panel.hpp>
#include <gui/objutils/cmd_composite.hpp>
#include <gui/framework/workbench.hpp>
#include <gui/utils/command_processor.hpp>

/*!
 * Includes
 */

////@begin includes
////@end includes

#include <wx/radiobox.h>
#include <wx/dialog.h>
#include <wx/radiobut.h>
#include <wx/sizer.h>
#include <wx/statbox.h>
#include <wx/button.h>
#include <wx/notebook.h>


/*!
 * Forward declarations
 */

////@begin forward declarations
class wxBoxSizer;
////@end forward declarations

BEGIN_NCBI_SCOPE

/*!
 * Control identifiers
 */

////@begin control identifiers
#define ID_ADD_FEATURE_BETWEEN_DLG 10029
#define SYMBOL_ADD_FEATURE_BETWEEN_DLG_STYLE wxCAPTION|wxRESIZE_BORDER|wxSYSTEM_MENU|wxCLOSE_BOX|wxTAB_TRAVERSAL
#define SYMBOL_ADD_FEATURE_BETWEEN_DLG_TITLE _("Add Features")
#define SYMBOL_ADD_FEATURE_BETWEEN_DLG_IDNAME ID_ADD_FEATURE_BETWEEN_DLG
#define SYMBOL_ADD_FEATURE_BETWEEN_DLG_SIZE wxSize(400, 300)
#define SYMBOL_ADD_FEATURE_BETWEEN_DLG_POSITION wxDefaultPosition
////@end control identifiers
#define ID_FEATURE_TABLE_TEXT 10030
#define ID_APPLY_BUTTON 10031

/*!
 * CAddFeatureBetweenDlg class declaration
 */

class CAddFeatureBetweenDlg: public CBulkCmdDlg
{    
    DECLARE_DYNAMIC_CLASS( CAddFeatureBetweenDlg )
    DECLARE_EVENT_TABLE()

public:
    /// Constructors
    CAddFeatureBetweenDlg();
    CAddFeatureBetweenDlg( wxWindow* parent, objects::CSeq_entry_Handle seh, IWorkbench* workbench,
                   wxWindowID id = SYMBOL_ADD_FEATURE_BETWEEN_DLG_IDNAME, const wxString& caption = SYMBOL_ADD_FEATURE_BETWEEN_DLG_TITLE, const wxPoint& pos = SYMBOL_ADD_FEATURE_BETWEEN_DLG_POSITION, const wxSize& size = SYMBOL_ADD_FEATURE_BETWEEN_DLG_SIZE, long style = SYMBOL_ADD_FEATURE_BETWEEN_DLG_STYLE );

    /// Creation
    bool Create( wxWindow* parent, wxWindowID id = SYMBOL_ADD_FEATURE_BETWEEN_DLG_IDNAME, const wxString& caption = SYMBOL_ADD_FEATURE_BETWEEN_DLG_TITLE, const wxPoint& pos = SYMBOL_ADD_FEATURE_BETWEEN_DLG_POSITION, const wxSize& size = SYMBOL_ADD_FEATURE_BETWEEN_DLG_SIZE, long style = SYMBOL_ADD_FEATURE_BETWEEN_DLG_STYLE );

    /// Destructor
    ~CAddFeatureBetweenDlg();

    /// Initialises member variables
    void Init();

    /// Creates the controls and sizers
    void CreateControls();

////@begin CAddFeatureBetweenDlg event handler declarations

////@end CAddFeatureBetweenDlg event handler declarations
 
////@begin CAddFeatureBetweenDlg member function declarations

    /// Retrieves bitmap resources
    wxBitmap GetBitmapResource( const wxString& name );

    /// Retrieves icon resources
    wxIcon GetIconResource( const wxString& name );
////@end CAddFeatureBetweenDlg member function declarations

    /// Should we show tooltips?
    static bool ShowToolTips();

////@begin CAddFeatureBetweenDlg member variables

////@end CAddFeatureBetweenDlg member variables

    virtual CRef<CCmdComposite> GetCommand();
    virtual string GetErrorMessage();
    void OnApplyButton(wxCommandEvent& event );
    void OnFeatLeftChanged(wxCommandEvent& event);
    void OnFeatRightChanged(wxCommandEvent& event);

private:
    void x_ListFeatures();
    void GetFeatureLocations(vector<CRef<objects::CSeq_loc> > &locs);
    void AddOneFeatureBetween(vector<CRef<objects::CSeq_loc> > &locs, TSignedSeqPos feat_left, TSignedSeqPos feat_right, bool partial_left, bool partial_right, objects::ENa_strand strand, const CSeq_id* id);

    objects::CSeq_entry_Handle m_TopSeqEntry;
    IWorkbench*     m_Workbench;
    wxNotebook *m_Notebook;
    wxListBox *m_FeatureTypeLeft;
    wxListBox *m_FeatureTypeRight;
    wxArrayString m_FeatureTypeStrings;
    wxRadioButton *m_NoFeatPartialLeft, *m_NoFeatNoFeatLeft, *m_ExtendCompleteLeft, *m_ExtendPartialLeft;
    wxRadioButton *m_NoFeatPartialRight, *m_NoFeatNoFeatRight, *m_ExtendCompleteRight, *m_ExtendPartialRight;
    vector<objects::CSeqFeatData::ESubtype> m_Subtype;
    wxCheckBox *m_LeaveUp;
    CFeatureFieldNamePanel* m_FeatLeftConstraint;
    CFeatureFieldNamePanel* m_FeatRightConstraint;
    CStringConstraintPanel* m_StringConstraintPanelLeft;
    CStringConstraintPanel* m_StringConstraintPanelRight;
};


END_NCBI_SCOPE

#endif
    // _ADD_FEATURE_BETWEEN_DLG_H_
