/*  $Id: cds_to_mat_peptide.hpp 39649 2017-10-24 15:22:12Z asztalos $
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
#ifndef _CDS_TO_MAT_PEPTIDE_H_
#define _CDS_TO_MAT_PEPTIDE_H_

#include <corelib/ncbistd.hpp>
#include <objmgr/seq_entry_handle.hpp>
#include <gui/packages/pkg_sequence_edit/bulk_cmd_dlg.hpp>
#include <gui/widgets/edit/feature_type_panel.hpp>
#include <gui/packages/pkg_sequence_edit/edit_feat_loc_panel.hpp>
#include <gui/packages/pkg_sequence_edit/ok_cancel_panel.hpp>
#include <objtools/edit/loc_edit.hpp>
#include <gui/packages/pkg_sequence_edit/miscedit_util.hpp>

#include <gui/packages/pkg_sequence_edit/string_constraint_panel.hpp>
#include <gui/packages/pkg_sequence_edit/field_choice_panel.hpp>
#include <gui/widgets/edit/feature_field_name_panel.hpp>
#include <gui/packages/pkg_sequence_edit/cap_change_panel.hpp>

/*!
 * Includes
 */

////@begin includes
////@end includes

#include <wx/sizer.h>
#include <wx/radiobut.h>
#include <wx/choice.h>

/*!
 * Forward declarations
 */

////@begin forward declarations
class CFeatureTypePanel;
class wxBoxSizer;
class CFieldConstraintPanel;
class COkCancelPanel;
////@end forward declarations

BEGIN_NCBI_SCOPE

/*!
 * Control identifiers
 */

////@begin control identifiers
#define ID_CDSTOMATPEPTIDE 10568
#define SYMBOL_CDSTOMATPEPTIDE_STYLE wxCAPTION|wxRESIZE_BORDER|wxSYSTEM_MENU|wxCLOSE_BOX|wxTAB_TRAVERSAL
#define SYMBOL_CDSTOMATPEPTIDE_TITLE _("Convert CDS to Mat-peptide")
#define SYMBOL_CDSTOMATPEPTIDE_IDNAME ID_CDSTOMATPEPTIDE
#define SYMBOL_CDSTOMATPEPTIDE_SIZE wxSize(900, 500)
#define SYMBOL_CDSTOMATPEPTIDE_POSITION wxDefaultPosition
////@end control identifiers



class CCdsToMatPeptide: public CBulkCmdDlg 
{    
    DECLARE_DYNAMIC_CLASS( CCdsToMatPeptide )
    DECLARE_EVENT_TABLE()

public:
    /// Constructors
    CCdsToMatPeptide();
    CCdsToMatPeptide( wxWindow* parent, IWorkbench* wb, wxWindowID id = SYMBOL_CDSTOMATPEPTIDE_IDNAME, const wxString& caption = SYMBOL_CDSTOMATPEPTIDE_TITLE, const wxPoint& pos = SYMBOL_CDSTOMATPEPTIDE_POSITION, const wxSize& size = SYMBOL_CDSTOMATPEPTIDE_SIZE, long style = SYMBOL_CDSTOMATPEPTIDE_STYLE );

    /// Creation
    bool Create( wxWindow* parent, wxWindowID id = SYMBOL_CDSTOMATPEPTIDE_IDNAME, const wxString& caption = SYMBOL_CDSTOMATPEPTIDE_TITLE, const wxPoint& pos = SYMBOL_CDSTOMATPEPTIDE_POSITION, const wxSize& size = SYMBOL_CDSTOMATPEPTIDE_SIZE, long style = SYMBOL_CDSTOMATPEPTIDE_STYLE );

    /// Destructor
    ~CCdsToMatPeptide();

    /// Initialises member variables
    void Init();

    /// Creates the controls and sizers
    void CreateControls();

    /// Retrieves bitmap resources
    wxBitmap GetBitmapResource( const wxString& name );

    /// Retrieves icon resources
    wxIcon GetIconResource( const wxString& name );

    /// Should we show tooltips?
    static bool ShowToolTips();

    virtual CRef<CCmdComposite> GetCommand();
    virtual string GetErrorMessage();
    void OnRadioButton(wxCommandEvent& event);
    void ProcessUpdateFeatEvent( wxCommandEvent& event );

private:
    void UpdateChildrenFeaturePanels( wxSizer* sizer );
    void ConvertEach(CRef<CCmdComposite> cmd);
    void MergeConvert(CRef<CCmdComposite> cmd);
    void ConvertInner(CRef<CCmdComposite> cmd);
    void RemoveFeature(CRef<CSeq_entry> entry, const CSeq_feat* feat);
    void RemoveProteins(CRef<CSeq_entry> entry, const CSeq_feat* feat, CSeq_entry_Handle seh);
    void FindAnnot(CRef<CSeq_entry> entry, const CSeq_feat* feat, CRef<CSeq_annot> &annot);
    string m_ErrorMessage;
    COkCancelPanel* m_OkCancel;
    wxRadioButton *m_Inner;
    wxRadioButton *m_Merge;
    wxRadioButton *m_Each;
    CRef<CConvertFeatureBase> m_converter;
    wxCheckBox *m_EntireRange;
    wxRadioButton *m_ProductFirst;
    wxRadioButton *m_ProductThis;
    CRef<CMiscSeqTableColumn> m_col;
    wxTextCtrl* m_ProductName;
    CStringConstraintPanel *m_StringConstraintPanel;
    CFeatureFieldNamePanel* m_FeatureConstraint;
};


class CExplicit_Mapper_Sequence_Info : public IMapper_Sequence_Info
{
public:
    void AddSeq(const CSeq_id_Handle& idh, TSeqType seq_type, TSeqPos seq_len);
    void AddSynonym(const CSeq_id_Handle& idh1, const CSeq_id_Handle& idh2);
    virtual TSeqType GetSequenceType(const CSeq_id_Handle& idh);
    virtual TSeqPos GetSequenceLength(const CSeq_id_Handle& idh);
    virtual void CollectSynonyms(const CSeq_id_Handle& id, TSynonyms& synonyms);
private:
    map<CSeq_id_Handle, TSeqType> m_SeqType;
    map<CSeq_id_Handle, TSeqPos> m_SeqLen;
    map<CSeq_id_Handle, set<CSeq_id_Handle> > m_Synonyms;
};

END_NCBI_SCOPE

#endif

