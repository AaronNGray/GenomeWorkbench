/*  $Id: correct_genes_dialog.hpp 37545 2017-01-19 19:14:39Z filippov $
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
#ifndef _CORRECT_GENES_DIALOG_H_
#define _CORRECT_GENES_DIALOG_H_

#include <corelib/ncbistd.hpp>

#include <gui/packages/pkg_sequence_edit/bulk_cmd_dlg.hpp>
#include <gui/packages/pkg_sequence_edit/ok_cancel_panel.hpp>

/*!
 * Includes
 */

////@begin includes
////@end includes

#include <wx/dialog.h>
#include <wx/sizer.h>
#include <wx/checkbox.h>
#include <wx/radiobox.h>

/*!
 * Forward declarations
 */

////@begin forward declarations
class CConstraintPanel;
class COkCancelPanel;
////@end forward declarations

BEGIN_NCBI_SCOPE

/*!
 * Control identifiers
 */

////@begin control identifiers
#define ID_CCORRECTGENESDIALOG 10416
#define ID_CHECKBOX10 10417
#define ID_CHECKBOX11 10418
#define ID_CHECKBOX12 10419
#define ID_RADIOBOX3 10424
#define ID_CHECKBOX13 10423
#define ID_WINDOW4 10173
#define ID_WINDOW14 10146
#define SYMBOL_CCORRECTGENESDIALOG_STYLE wxCAPTION|wxRESIZE_BORDER|wxSYSTEM_MENU|wxCLOSE_BOX|wxTAB_TRAVERSAL
#define SYMBOL_CCORRECTGENESDIALOG_TITLE _("Correct Genes")
#define SYMBOL_CCORRECTGENESDIALOG_IDNAME ID_CCORRECTGENESDIALOG
#define SYMBOL_CCORRECTGENESDIALOG_SIZE wxSize(400, 300)
#define SYMBOL_CCORRECTGENESDIALOG_POSITION wxDefaultPosition
////@end control identifiers


/*!
 * CCorrectGenesDialog class declaration
 */

class CCorrectGenesDialog: public CBulkCmdDlg
{    
    DECLARE_DYNAMIC_CLASS( CCorrectGenesDialog )
    DECLARE_EVENT_TABLE()

public:
    /// Constructors
    CCorrectGenesDialog();
    CCorrectGenesDialog( wxWindow* parent, IWorkbench* wb, wxWindowID id = SYMBOL_CCORRECTGENESDIALOG_IDNAME, const wxString& caption = SYMBOL_CCORRECTGENESDIALOG_TITLE, const wxPoint& pos = SYMBOL_CCORRECTGENESDIALOG_POSITION, const wxSize& size = SYMBOL_CCORRECTGENESDIALOG_SIZE, long style = SYMBOL_CCORRECTGENESDIALOG_STYLE );

    /// Creation
    bool Create( wxWindow* parent, wxWindowID id = SYMBOL_CCORRECTGENESDIALOG_IDNAME, const wxString& caption = SYMBOL_CCORRECTGENESDIALOG_TITLE, const wxPoint& pos = SYMBOL_CCORRECTGENESDIALOG_POSITION, const wxSize& size = SYMBOL_CCORRECTGENESDIALOG_SIZE, long style = SYMBOL_CCORRECTGENESDIALOG_STYLE );

    /// Destructor
    ~CCorrectGenesDialog();

    /// Initialises member variables
    void Init();

    /// Creates the controls and sizers
    void CreateControls();

////@begin CCorrectGenesDialog event handler declarations

////@end CCorrectGenesDialog event handler declarations

////@begin CCorrectGenesDialog member function declarations

    /// Retrieves bitmap resources
    wxBitmap GetBitmapResource( const wxString& name );

    /// Retrieves icon resources
    wxIcon GetIconResource( const wxString& name );
////@end CCorrectGenesDialog member function declarations

    /// Should we show tooltips?
    static bool ShowToolTips();

////@begin CCorrectGenesDialog member variables
    wxCheckBox* m_MatchInterval;
    wxCheckBox* m_MatchStrand;
    wxCheckBox* m_Log;
    wxRadioBox* m_FeatureChoice;
    wxCheckBox* m_OnlySelected;
    CConstraintPanel* m_Constraint;
    COkCancelPanel* m_OkCancel;
////@end CCorrectGenesDialog member variables

    virtual CRef<CCmdComposite> GetCommand();
    virtual string GetErrorMessage();

protected:
    bool x_CorrectPairInterval(const CSeq_loc& cds, CSeq_feat& gene, CScope* scope);
    bool x_CorrectPairStrand(const CSeq_loc& cds, CSeq_loc& gene);
    string x_CorrectOnePair(const CSeq_feat& cds, CSeq_feat& gene, const string& seq_label, CScope& scope);
    vector<CConstRef<CSeq_feat> > x_GetSelectedPair();
    vector<CConstRef<CSeq_feat> > x_GetConstrainedList();
    bool x_TestGeneForFeature(const CSeq_loc& gene_loc, const CSeq_loc& feat_loc, CScope& scope, bool same_strand, bool& exact, TSeqPos& diff);
    void x_FindExactMatches(const vector<CSeq_feat_Handle> &feats, CScope& scope, bool same_strand, 
                            map<CSeq_feat_Handle, vector<CSeq_feat_Handle> > &feat_to_genes, set<CSeq_feat_Handle> &used_genes);
    void x_FindNonExactMatches(const vector<CSeq_feat_Handle> &feats, CScope& scope, bool same_strand, 
                               map<CSeq_feat_Handle, vector<CSeq_feat_Handle> > &feat_to_genes, set<CSeq_feat_Handle> &used_genes);

    string m_Error;
};

END_NCBI_SCOPE

#endif
    // _CORRECT_GENES_DIALOG_H_
