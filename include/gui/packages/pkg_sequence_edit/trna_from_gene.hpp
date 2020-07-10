/*  $Id: trna_from_gene.hpp 39649 2017-10-24 15:22:12Z asztalos $
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
#ifndef _TRNA_FROM_GENE_H_
#define _TRNA_FROM_GENE_H_

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
#define ID_CTRNA_FROM_GENE wxID_ANY
#define SYMBOL_CTRNA_FROM_GENE_STYLE wxCAPTION|wxRESIZE_BORDER|wxSYSTEM_MENU|wxCLOSE_BOX|wxTAB_TRAVERSAL
#define SYMBOL_CTRNA_FROM_GENE_TITLE _("tRNA/rRNA From Gene")
#define SYMBOL_CTRNA_FROM_GENE_IDNAME ID_CTRNA_FROM_GENE
#define SYMBOL_CTRNA_FROM_GENE_SIZE wxSize(900, 500)
#define SYMBOL_CTRNA_FROM_GENE_POSITION wxDefaultPosition
////@end control identifiers



class CtRNAFromGene: public CBulkCmdDlg 
{    
    DECLARE_DYNAMIC_CLASS( CtRNAFromGene )
    DECLARE_EVENT_TABLE()

public:
    /// Constructors
    CtRNAFromGene();
    CtRNAFromGene( wxWindow* parent, IWorkbench* wb, wxWindowID id = SYMBOL_CTRNA_FROM_GENE_IDNAME, const wxString& caption = SYMBOL_CTRNA_FROM_GENE_TITLE, const wxPoint& pos = SYMBOL_CTRNA_FROM_GENE_POSITION, const wxSize& size = SYMBOL_CTRNA_FROM_GENE_SIZE, long style = SYMBOL_CTRNA_FROM_GENE_STYLE );

    /// Creation
    bool Create( wxWindow* parent, wxWindowID id = SYMBOL_CTRNA_FROM_GENE_IDNAME, const wxString& caption = SYMBOL_CTRNA_FROM_GENE_TITLE, const wxPoint& pos = SYMBOL_CTRNA_FROM_GENE_POSITION, const wxSize& size = SYMBOL_CTRNA_FROM_GENE_SIZE, long style = SYMBOL_CTRNA_FROM_GENE_STYLE );

    /// Destructor
    ~CtRNAFromGene();

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
    void OnSelectFeatQual(wxCommandEvent& event);
    void ProcessUpdateFeatEvent( wxCommandEvent& event );
private:
    string GetSuggestedName(CSeq_feat &feat, CRef<CSeq_feat> prot_feat, CSeq_entry_Handle seh);
    void GetNameFromGene(CSeq_feat& gene, const wxString& sel_str, bool remove, string &name);
    void UpdateChildrenFeaturePanels( wxSizer* sizer );

    string m_ErrorMessage;
    wxChoice* m_FeatureType;
    CStringConstraintPanel* m_StringConstraintPanel;
    CFeatureFieldNamePanel* m_FeatureConstraint;
    COkCancelPanel* m_OkCancel;
    wxCheckBox* m_SingleInterval;
    CCapChangePanel *m_CapChangeOptions;
    wxChoice *m_FeatQual;
    wxChoice *m_FeatQual2;
    wxChoice *m_FeatQual3;
    wxStaticText* m_2ndChoice;
    wxStaticText* m_3rdChoice;
    wxTextCtrl *m_StrQual;
    wxCheckBox* m_Remove;
    wxCheckBox* m_Remove2;
    wxCheckBox* m_Remove3;
    bool m_modified;
    bool m_modified_prot;
};

END_NCBI_SCOPE

#endif

