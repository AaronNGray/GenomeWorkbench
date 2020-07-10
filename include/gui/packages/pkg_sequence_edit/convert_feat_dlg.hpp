/*  $Id: convert_feat_dlg.hpp 44316 2019-11-27 16:08:06Z filippov $
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
#ifndef _CONVERT_FEAT_DLG_H_
#define _CONVERT_FEAT_DLG_H_

#include <corelib/ncbistd.hpp>
#include <gui/objutils/convert_feat.hpp>
#include <objtools/edit/string_constraint.hpp>
#include <gui/packages/pkg_sequence_edit/bulk_cmd_dlg.hpp>
#include <gui/widgets/edit/feature_type_panel.hpp>
#include <gui/packages/pkg_sequence_edit/string_constraint_panel.hpp>
#include <gui/packages/pkg_sequence_edit/ok_cancel_panel.hpp>

/*!
 * Includes
 */

////@begin includes
////@end includes

#include <wx/sizer.h>
#include <wx/checkbox.h>

/*!
 * Forward declarations
 */

////@begin forward declarations
class wxFlexGridSizer;
class CFeatureTypePanel;
class CFieldConstraintPanel;
class COkCancelPanel;
////@end forward declarations

BEGIN_NCBI_SCOPE

/*!
 * Control identifiers
 */

////@begin control identifiers
#define ID_CCONVERTFEATDLG 10147
#define ID_CF_FEATURETYPEFROM 10278
#define ID_CF_FEATURETYPETO 10279
#define ID_CF_FUNCTION_DESC 10280
#define ID_CF_LEAVE_ORIGINAL 10283
#define ID_CF_ADD_UNVERIFIED 10284
#define ID_CF_CONSTRAINT 10281
#define ID_CF_OKC 10282
#define SYMBOL_CCONVERTFEATDLG_STYLE wxCAPTION|wxRESIZE_BORDER|wxSYSTEM_MENU|wxCLOSE_BOX|wxTAB_TRAVERSAL
#define SYMBOL_CCONVERTFEATDLG_TITLE _("Convert Features")
#define SYMBOL_CCONVERTFEATDLG_IDNAME ID_CCONVERTFEATDLG
#define SYMBOL_CCONVERTFEATDLG_SIZE wxSize(400, 300)
#define SYMBOL_CCONVERTFEATDLG_POSITION wxDefaultPosition
////@end control identifiers


/*!
 * CConvertFeatDlg class declaration
 */

class CConvertFeatDlg: public CBulkCmdDlg
              , public CFieldNamePanelParent
{    
    DECLARE_DYNAMIC_CLASS( CConvertFeatDlg )
    DECLARE_EVENT_TABLE()

public:
    /// Constructors
    CConvertFeatDlg();
    CConvertFeatDlg( wxWindow* parent, IWorkbench* wb, wxWindowID id = SYMBOL_CCONVERTFEATDLG_IDNAME, const wxString& caption = SYMBOL_CCONVERTFEATDLG_TITLE, const wxPoint& pos = SYMBOL_CCONVERTFEATDLG_POSITION, const wxSize& size = SYMBOL_CCONVERTFEATDLG_SIZE, long style = SYMBOL_CCONVERTFEATDLG_STYLE );

    /// Creation
    bool Create( wxWindow* parent, wxWindowID id = SYMBOL_CCONVERTFEATDLG_IDNAME, const wxString& caption = SYMBOL_CCONVERTFEATDLG_TITLE, const wxPoint& pos = SYMBOL_CCONVERTFEATDLG_POSITION, const wxSize& size = SYMBOL_CCONVERTFEATDLG_SIZE, long style = SYMBOL_CCONVERTFEATDLG_STYLE );

    /// Destructor
    ~CConvertFeatDlg();

    /// Initialises member variables
    void Init();

    /// Creates the controls and sizers
    void CreateControls();

////@begin CConvertFeatDlg event handler declarations

    void ProcessUpdateFeatEvent( wxCommandEvent& event );
////@end CConvertFeatDlg event handler declarations

////@begin CConvertFeatDlg member function declarations

    /// Retrieves bitmap resources
    wxBitmap GetBitmapResource( const wxString& name );

    /// Retrieves icon resources
    wxIcon GetIconResource( const wxString& name );
////@end CConvertFeatDlg member function declarations

    /// Should we show tooltips?
    static bool ShowToolTips();

////@begin CConvertFeatDlg member variables
    wxFlexGridSizer* m_FeatureChoiceSizer;
    CFeatureTypePanel* m_FeatureTypeFrom;
    CFeatureTypePanel* m_FeatureTypeTo;
    wxTextCtrl* m_FunctionDescriptionTxt;
    wxFlexGridSizer* m_OptionsSizer;
    wxCheckBox* m_LeaveOriginal;
    wxCheckBox* m_AddUnverified;
    CStringConstraintPanel* m_Constraint;
    COkCancelPanel* m_OkCancel;
////@end CConvertFeatDlg member variables

    // for CBulkCmdDlg
    virtual CRef<CCmdComposite> GetCommand();
    virtual string GetErrorMessage();

    // for CFieldNamePanelParent
    virtual void UpdateEditor();

protected:
    CRef<CConvertFeatureBase> m_Converter;

    void x_UpdateOptions();
    void x_SetOptions();
    void x_FixLayout();

private:
    void x_AddUnverified(objects::CBioseq_Handle bsh, CRef<CCmdComposite> cmd,  set<objects::CSeq_entry_Handle> &already_done);
    bool DoesObjectAllQualsMatchFieldConstraint (const CObject& object, const set<string>& qual_list, CRef<edit::CStringConstraint> string_constraint, CRef<CScope> scope);
    set<string> GetQualList(void);
    string m_old_feature_from;

};

END_NCBI_SCOPE

#endif
    // _CONVERT_FEAT_DLG_H_
