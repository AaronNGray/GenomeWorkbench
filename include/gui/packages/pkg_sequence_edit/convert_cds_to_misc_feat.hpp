/*  $Id: convert_cds_to_misc_feat.hpp 39649 2017-10-24 15:22:12Z asztalos $
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
#ifndef _CONVERT_CDS_TO_MISC_FEAT_H_
#define _CONVERT_CDS_TO_MISC_FEAT_H_

#include <corelib/ncbistd.hpp>
#include <gui/objutils/convert_feat.hpp>
#include <gui/packages/pkg_sequence_edit/bulk_cmd_dlg.hpp>
#include <gui/widgets/edit/feature_type_panel.hpp>
#include <gui/packages/pkg_sequence_edit/ok_cancel_panel.hpp>

/*!
 * Includes
 */

////@begin includes
////@end includes

#include <wx/sizer.h>
#include <wx/checkbox.h>
#include <wx/radiobut.h>

/*!
 * Forward declarations
 */

////@begin forward declarations
class wxFlexGridSizer;
////@end forward declarations

BEGIN_NCBI_SCOPE

/*!
 * Control identifiers
 */
class CFeatureTypePanel;
class CConstraintPanel;
class COkCancelPanel;

////@begin control identifiers
#define ID_CONVERT_CDS_TO_MISC_FEAT 10147
#define ID_CDS_TO_MISCFEAT_RADIOBUTTON 10148
#define ID_CDS_TO_MISCFEAT_RADIOBUTTON1 10149
#define ID_CDS_TO_MISCFEAT_RADIO_ALL 10150
#define ID_CDS_TO_MISCFEAT_RADIO_WHERE 10151
#define SYMBOL_CONVERT_CDS_TO_MISC_FEAT_STYLE wxCAPTION|wxRESIZE_BORDER|wxSYSTEM_MENU|wxCLOSE_BOX|wxTAB_TRAVERSAL
#define SYMBOL_CONVERT_CDS_TO_MISC_FEAT_TITLE _("Convert CDS to Misc Features")
#define SYMBOL_CONVERT_CDS_TO_MISC_FEAT_IDNAME ID_CONVERT_CDS_TO_MISC_FEAT
#define SYMBOL_CONVERT_CDS_TO_MISC_FEAT_SIZE wxSize(400, 300)
#define SYMBOL_CONVERT_CDS_TO_MISC_FEAT_POSITION wxDefaultPosition
////@end control identifiers


/*!
 * CConvertFeatDlg class declaration
 */

class CConvertCdsToMiscFeat: public CBulkCmdDlg
{    
    DECLARE_DYNAMIC_CLASS( CConvertCdsToMiscFeat )
    DECLARE_EVENT_TABLE()

public:
    /// Constructors
    CConvertCdsToMiscFeat();
    CConvertCdsToMiscFeat( wxWindow* parent, IWorkbench* wb, wxWindowID id = SYMBOL_CONVERT_CDS_TO_MISC_FEAT_IDNAME, const wxString& caption = SYMBOL_CONVERT_CDS_TO_MISC_FEAT_TITLE, const wxPoint& pos = SYMBOL_CONVERT_CDS_TO_MISC_FEAT_POSITION, const wxSize& size = SYMBOL_CONVERT_CDS_TO_MISC_FEAT_SIZE, long style = SYMBOL_CONVERT_CDS_TO_MISC_FEAT_STYLE );

    /// Creation
    bool Create( wxWindow* parent, wxWindowID id = SYMBOL_CONVERT_CDS_TO_MISC_FEAT_IDNAME, const wxString& caption = SYMBOL_CONVERT_CDS_TO_MISC_FEAT_TITLE, const wxPoint& pos = SYMBOL_CONVERT_CDS_TO_MISC_FEAT_POSITION, const wxSize& size = SYMBOL_CONVERT_CDS_TO_MISC_FEAT_SIZE, long style = SYMBOL_CONVERT_CDS_TO_MISC_FEAT_STYLE );

    /// Destructor
    ~CConvertCdsToMiscFeat();

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
    void OnChoiceChanged( wxCommandEvent& event );
    void OnConditionChanged( wxCommandEvent& event );

    // for CBulkCmdDlg
    virtual CRef<CCmdComposite> GetCommand();
    virtual string GetErrorMessage();

    void ApplyToAllAndDie(bool similar, bool nonfunctional, bool unverified);

protected:
    CRef<CConvertFeatureBase> m_Converter;

    void x_SetOptions();
    void DetectStartStopCodons(const CSeq_feat* f, bool &stop_codon_present, bool &bad_start_codon, bool &bad_stop_codon);

private:
////@begin CConvertFeatDlg member variables
    wxFlexGridSizer* m_OptionsSizer;
    wxCheckBox* m_LeaveOriginal;
    wxCheckBox*  m_AddUnverified;
    wxCheckBox* m_InternalStop;
    wxCheckBox* m_BadStart;
    wxCheckBox* m_BadStop;
    wxCheckBox* m_IsPseudo;
    CConstraintPanel* m_Constraint;
    COkCancelPanel* m_OkCancel;
    wxRadioButton *m_RadioButton;
    wxRadioButton *m_RadioWhere;
    wxRadioButton *m_RadioAnd;
    wxRadioButton *m_RadioOr;
    wxRadioButton *m_RadioSimilar;
    wxRadioButton *m_RadioNonfunctional;
////@end CConvertFeatDlg member variables
};

END_NCBI_SCOPE

#endif
    // _CONVERT_CDS_TO_MISC_FEAT_H_
