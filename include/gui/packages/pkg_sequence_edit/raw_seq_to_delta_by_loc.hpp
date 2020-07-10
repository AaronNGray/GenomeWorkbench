/*  $Id: raw_seq_to_delta_by_loc.hpp 42755 2019-04-10 16:32:15Z filippov $
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
 * Authors: Igor Filippov
 */
#ifndef _RAW_SEQ_TO_DELTA_BY_LOC_H_
#define _RAW_SEQ_TO_DELTA_BY_LOC_H_

#include <corelib/ncbistd.hpp>
#include <wx/dialog.h>
#include <objects/seq/Seq_gap.hpp>
#include <objects/general/Object_id.hpp>
#include <objmgr/bioseq_handle.hpp>
#include <gui/objutils/cmd_composite.hpp>
#include <gui/objutils/cmd_change_bioseq_inst.hpp>
#include <gui/objutils/convert_raw_to_delta.hpp>
#include <gui/widgets/data/report_dialog.hpp>
#include <objmgr/bioseq_ci.hpp>
#include <objmgr/seq_align_handle.hpp>

/*!
 * Includes
 */

////@begin includes
////@end includes
#include <wx/sizer.h>

/*!
 * Forward declarations
 */

////@begin forward declarations
class wxBoxSizer;
class wxRadioButton;
class wxCheckBox;
class wxTextCtrl;
////@end forward declarations

BEGIN_NCBI_SCOPE

/*!
 * Control identifiers
 */

////@begin control identifiers
#define ID_CRAWSEQTODELTASEQBYLOC 10000
#define ID_CRAWTODELTA_SCROLLEDWINDOW 10001
#define ID_CRAWTODELTA_TEXTCTRL 10002
#define ID_CRAWTODELTA_CHOICE 10003
#define ID_CRAWTODELTA_TEXTCTRL1 10004
#define ID_CRAWTODELTA_CHOICE1 10005
#define ID_CRAWTODELTA_TEXTCTRL2 10011
#define ID_CRAWTODELTA_CHOICE2 10012
#define ID_CRAWTODELTA_TEXTCTRL3 10013
#define ID_CRAWTODELTA_CHOICE3 10014
#define ID_CRAWTODELTA_TEXTCTRL4 10015
#define ID_CRAWTODELTA_CHOICE4 10016
#define ID_CRAWTODELTA_TEXTCTRL5 10017
#define ID_CRAWTODELTA_CHOICE5 10018
#define ID_CRAWTODELTA_TEXTCTRL6 10019
#define ID_CRAWTODELTA_CHOICE6 10020
#define ID_CRAWTODELTA_TEXTCTRL7 10021
#define ID_CRAWTODELTA_CHOICE7 10022
#define ID_CRAWTODELTA_RADIOBUTTON 10006
#define ID_CRAWTODELTA_RADIOBUTTON1 10007
#define ID_CRAWTODELTA_CHECKBOX 10008
#define SYMBOL_CRAWSEQTODELTASEQBYLOC_STYLE wxCAPTION|wxRESIZE_BORDER|wxSYSTEM_MENU|wxCLOSE_BOX|wxTAB_TRAVERSAL
#define SYMBOL_CRAWSEQTODELTASEQBYLOC_TITLE _("Convert Raw Sequence to Delta Sequence")
#define SYMBOL_CRAWSEQTODELTASEQBYLOC_IDNAME ID_CRAWSEQTODELTASEQBYLOC
#define SYMBOL_CRAWSEQTODELTASEQBYLOC_SIZE wxSize(400, 300)
#define SYMBOL_CRAWSEQTODELTASEQBYLOC_POSITION wxDefaultPosition
////@end control identifiers


/*!
 * CRawSeqToDeltaSeqByLoc class declaration
 */

class CRawSeqToDeltaSeqByLoc : public CReportEditingDialog
{    
    DECLARE_DYNAMIC_CLASS( CRawSeqToDeltaSeqByLoc )
    DECLARE_EVENT_TABLE()

public:
    /// Constructors
    CRawSeqToDeltaSeqByLoc();
    CRawSeqToDeltaSeqByLoc( wxWindow* parent, objects::CSeq_entry_Handle tse, wxWindowID id = SYMBOL_CRAWSEQTODELTASEQBYLOC_IDNAME, const wxString& caption = SYMBOL_CRAWSEQTODELTASEQBYLOC_TITLE, const wxPoint& pos = SYMBOL_CRAWSEQTODELTASEQBYLOC_POSITION, const wxSize& size = SYMBOL_CRAWSEQTODELTASEQBYLOC_SIZE, long style = SYMBOL_CRAWSEQTODELTASEQBYLOC_STYLE );

    /// Creation
    bool Create( wxWindow* parent, wxWindowID id = SYMBOL_CRAWSEQTODELTASEQBYLOC_IDNAME, const wxString& caption = SYMBOL_CRAWSEQTODELTASEQBYLOC_TITLE, const wxPoint& pos = SYMBOL_CRAWSEQTODELTASEQBYLOC_POSITION, const wxSize& size = SYMBOL_CRAWSEQTODELTASEQBYLOC_SIZE, long style = SYMBOL_CRAWSEQTODELTASEQBYLOC_STYLE );

    /// Destructor
    ~CRawSeqToDeltaSeqByLoc();

    /// Initialises member variables
    void Init();

    /// Creates the controls and sizers
    void CreateControls();

////@begin CRawSeqToDeltaSeqByLoc event handler declarations

////@end CRawSeqToDeltaSeqByLoc event handler declarations

////@begin CRawSeqToDeltaSeqByLoc member function declarations

    /// Retrieves bitmap resources
    wxBitmap GetBitmapResource( const wxString& name );

    /// Retrieves icon resources
    wxIcon GetIconResource( const wxString& name );
////@end CRawSeqToDeltaSeqByLoc member function declarations

    /// Should we show tooltips?
    static bool ShowToolTips();
    void OnNewText(wxCommandEvent& event);

    CRef<CCmdComposite> apply(string title);

    static void UpdateDensegAlignment(const objects::CSeq_entry_Handle& tse, CRef<CCmdComposite> composite,
                                      const vector<NRawToDeltaSeq::SGap> &gaps, const bool use_align_coords);
    static CRef<CCmdComposite> ByAssemblyGapFeatures(objects::CSeq_entry_Handle seh);
    void UseAlignCoords(objects::CBioseq_Handle bsh, vector<NRawToDeltaSeq::SGap> &gaps);
    void ComputeAlignCoords(objects::CBioseq_Handle bsh, int before, int after, int &real_before, int &real_after);
private:
    static void x_UpdateRows(size_t num_rows, const vector<NRawToDeltaSeq::SGap> &gaps, const vector< TSignedSeqPos > &orig_starts, const vector< TSeqPos > &lens,
                             const vector< objects::ENa_strand > &strands, const vector< CRef< objects::CScore > > &scores,
                             vector< vector< vector<TSignedSeqPos> > > &all_new_starts,  vector< vector<TSeqPos> > &all_new_lens,
                             vector< vector< vector< objects::ENa_strand > > > &all_new_strands, vector< vector< CRef<objects::CScore> > > &all_new_scores,
                             const bool use_align_coords, const objects::CSeq_align& align);

    static CRef<objects::CSeq_align> x_GetNewAlign(const size_t block, const objects::CSeq_align& align, const size_t num_rows, 
                                                   const vector < vector< vector<TSignedSeqPos> > > &all_new_starts,  const vector < vector<TSeqPos> > &all_new_lens,
                                                   const vector< vector< vector< objects::ENa_strand > > > &all_new_strands, const vector< vector< CRef<objects::CScore> > > &all_new_scores);

    CRef<CCmdComposite> apply_impl(string title, map<objects::CObject_id::TId, objects::CObject_id::TId> &old_to_new, bool create_xref_map);
    static CRef<CCmdComposite> ByAssemblyGapFeatures_impl(objects::CSeq_entry_Handle seh, map<objects::CObject_id::TId, objects::CObject_id::TId> &old_to_new, bool create_xref_map);

////@begin CRawSeqToDeltaSeqByLoc member variables
    wxBoxSizer* m_ScrollSizer;
    wxRadioButton* m_CoordSeq;
    wxCheckBox* m_AdjustCDS;
    wxTextCtrl* m_LastStart;
    wxTextCtrl* m_LastLength;
////@end CRawSeqToDeltaSeqByLoc member variables
    objects::CSeq_entry_Handle m_TopSeqEntry;
};


END_NCBI_SCOPE

#endif
    // _RAW_SEQ_TO_DELTA_BY_LOC_H_
