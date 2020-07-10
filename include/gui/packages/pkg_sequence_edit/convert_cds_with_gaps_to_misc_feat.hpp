/*  $Id: convert_cds_with_gaps_to_misc_feat.hpp 37460 2017-01-12 15:49:41Z filippov $
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
#ifndef _CONVERT_CDS_WITH_GAPS_TO_MISC_FEAT_H_
#define _CONVERT_CDS_WITH_GAPS_TO_MISC_FEAT_H_

#include <corelib/ncbistd.hpp>
#include <gui/objutils/convert_feat.hpp>
#include <gui/packages/pkg_sequence_edit/bulk_cmd_dlg.hpp>
#include <gui/packages/pkg_sequence_edit/field_constraint_panel.hpp>
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
class CFeatureTypePanel;
class CFieldConstraintPanel;
class COkCancelPanel;
////@end forward declarations

BEGIN_NCBI_SCOPE

/*!
 * Control identifiers
 */

////@begin control identifiers
#define ID_CONVERT_CDS_WITH_GAPS_TO_MISC_FEAT wxID_ANY
#define SYMBOL_CONVERT_CDS_WITH_GAPS_TO_MISC_FEAT_STYLE wxCAPTION|wxRESIZE_BORDER|wxSYSTEM_MENU|wxCLOSE_BOX|wxTAB_TRAVERSAL
#define SYMBOL_CONVERT_CDS_WITH_GAPS_TO_MISC_FEAT_TITLE _("Convert Coding Regions With Gaps to Misc Features")
#define SYMBOL_CONVERT_CDS_WITH_GAPS_TO_MISC_FEAT_IDNAME ID_CONVERT_CDS_WITH_GAPS_TO_MISC_FEAT
#define SYMBOL_CONVERT_CDS_WITH_GAPS_TO_MISC_FEAT_SIZE wxSize(400, 300)
#define SYMBOL_CONVERT_CDS_WITH_GAPS_TO_MISC_FEAT_POSITION wxDefaultPosition
////@end control identifiers


/*!
 * CConvertFeatDlg class declaration
 */

class CConvertCdsWithGapsToMiscFeat: public CBulkCmdDlg
{    
    DECLARE_DYNAMIC_CLASS( CConvertCdsWithGapsToMiscFeat )
    DECLARE_EVENT_TABLE()

public:
    /// Constructors
    CConvertCdsWithGapsToMiscFeat();
    CConvertCdsWithGapsToMiscFeat( wxWindow* parent, IWorkbench* wb, wxWindowID id = SYMBOL_CONVERT_CDS_WITH_GAPS_TO_MISC_FEAT_IDNAME, const wxString& caption = SYMBOL_CONVERT_CDS_WITH_GAPS_TO_MISC_FEAT_TITLE, const wxPoint& pos = SYMBOL_CONVERT_CDS_WITH_GAPS_TO_MISC_FEAT_POSITION, const wxSize& size = SYMBOL_CONVERT_CDS_WITH_GAPS_TO_MISC_FEAT_SIZE, long style = SYMBOL_CONVERT_CDS_WITH_GAPS_TO_MISC_FEAT_STYLE );

    /// Creation
    bool Create( wxWindow* parent, wxWindowID id = SYMBOL_CONVERT_CDS_WITH_GAPS_TO_MISC_FEAT_IDNAME, const wxString& caption = SYMBOL_CONVERT_CDS_WITH_GAPS_TO_MISC_FEAT_TITLE, const wxPoint& pos = SYMBOL_CONVERT_CDS_WITH_GAPS_TO_MISC_FEAT_POSITION, const wxSize& size = SYMBOL_CONVERT_CDS_WITH_GAPS_TO_MISC_FEAT_SIZE, long style = SYMBOL_CONVERT_CDS_WITH_GAPS_TO_MISC_FEAT_STYLE );

    /// Destructor
    ~CConvertCdsWithGapsToMiscFeat();

    /// Initialises member variables
    void Init();

    /// Creates the controls and sizers
    void CreateControls();

////@begin CConvertFeatDlg event handler declarations

////@end CConvertFeatDlg event handler declarations

////@begin CConvertFeatDlg member function declarations

    /// Retrieves bitmap resources
    wxBitmap GetBitmapResource( const wxString& name );

    /// Retrieves icon resources
    wxIcon GetIconResource( const wxString& name );
////@end CConvertFeatDlg member function declarations

    /// Should we show tooltips?
    static bool ShowToolTips();

    // for CBulkCmdDlg
    virtual CRef<CCmdComposite> GetCommand();
    virtual string GetErrorMessage();

private:
////@begin CConvertFeatDlg member variables
    CRef<CConvertCDSToMiscFeat> m_Converter;

    CStringConstraintPanel* m_StringConstraintPanel;
    COkCancelPanel* m_OkCancel;
    wxRadioButton *m_RadioButton;
////@end CConvertFeatDlg member variables
};

class CConvertBadCdsAndRnaToMiscFeat
{
public:
    static CRef<CCmdComposite> apply(wxWindow *parent, objects::CSeq_entry_Handle seh);
private:
    static bool IsExtendableLeft(TSeqPos left, const CBioseq& seq, CScope* scope, TSeqPos& extend_len);
    static bool IsExtendableRight(TSeqPos right, const CBioseq& seq, CScope* scope, TSeqPos& extend_len);
    static bool IsNonExtendable(const CSeq_loc& loc, const CBioseq& seq, CScope* scope);
    static bool getBacterialPartialNonextendable(const CSeq_feat &feat, CBioseq_Handle bsh);
    static bool IsShortrRNA(const CSeq_feat& f, CScope* scope);
    static bool getShortRRNA(const CSeq_feat &feat, CScope &scope);
    static bool getCdsOverlap(const CSeq_feat &cds, CBioseq_Handle bsh);
    static bool getRnaOverlap(const CSeq_feat &rna, CBioseq_Handle bsh);
    static bool getCdsRnaOverlap(const CSeq_feat &feat, CBioseq_Handle bsh);
};

END_NCBI_SCOPE

#endif
    // _CONVERT_CDS_WITH_GAPS_TO_MISC_FEAT_H_
