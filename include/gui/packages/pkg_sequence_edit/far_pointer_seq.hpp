/*  $Id: far_pointer_seq.hpp 38636 2017-06-05 19:12:02Z asztalos $
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


#ifndef _CFARPOINTERSEQ_H_
#define _CFARPOINTERSEQ_H_


/*!
 * Includes
 */

////@begin includes
#include <corelib/ncbistd.hpp>
#include <objects/seqalign/Seq_align.hpp>
#include <objmgr/seq_entry_handle.hpp>
#include <gui/objutils/cmd_composite.hpp>
#include <gui/widgets/data/report_dialog.hpp>
////@end includes

#include <wx/graphics.h>

BEGIN_NCBI_SCOPE
USING_SCOPE(objects);
/*!
 * Forward declarations
 */

////@begin forward declarations
////@end forward declarations

/*!
 * Control identifiers
 */

////@begin control identifiers
#define ID_CFARPOINTERSEQ wxID_ANY
#define SYMBOL_CFARPOINTERSEQ_STYLE wxCAPTION|wxRESIZE_BORDER|wxSYSTEM_MENU|wxCLOSE_BOX|wxTAB_TRAVERSAL
#define SYMBOL_CFARPOINTERSEQ_TITLE _("Far Pointer Sequences")
#define SYMBOL_CFARPOINTERSEQ_IDNAME ID_CFARPOINTERSEQ
#define SYMBOL_CFARPOINTERSEQ_SIZE wxSize(400, 300)
#define SYMBOL_CFARPOINTERSEQ_POSITION wxDefaultPosition
#define ID_FARPOINTERSEQ_CHECKLISTBOX 11501
#define ID_FARPOINTERSEQ_EXPORT 11502
////@end control identifiers


/*!
 * CFarPointerSeq class declaration
 */

class CFarPointerSeq : public CReportEditingDialog
{    
    DECLARE_DYNAMIC_CLASS( CFarPointerSeq )
    DECLARE_EVENT_TABLE()

public:
    /// Constructors
    CFarPointerSeq();
    CFarPointerSeq( wxWindow* parent, CSeq_entry_Handle seh, wxWindowID id = SYMBOL_CFARPOINTERSEQ_IDNAME, const wxString& caption = SYMBOL_CFARPOINTERSEQ_TITLE, const wxPoint& pos = SYMBOL_CFARPOINTERSEQ_POSITION, const wxSize& size = SYMBOL_CFARPOINTERSEQ_SIZE, long style = SYMBOL_CFARPOINTERSEQ_STYLE );

    /// Creation
    bool Create( wxWindow* parent, wxWindowID id = SYMBOL_CFARPOINTERSEQ_IDNAME, const wxString& caption = SYMBOL_CFARPOINTERSEQ_TITLE, const wxPoint& pos = SYMBOL_CFARPOINTERSEQ_POSITION, const wxSize& size = SYMBOL_CFARPOINTERSEQ_SIZE, long style = SYMBOL_CFARPOINTERSEQ_STYLE );

    /// Destructor
    ~CFarPointerSeq();

    /// Initialises member variables
    void Init();

    /// Creates the controls and sizers
    void CreateControls();

////@begin CFarPointerSeq event handler declarations
    void OnCheck(wxCommandEvent& event);
    void OnExport(wxCommandEvent& event);
////@end CFarPointerSeq event handler declarations

////@begin CFarPointerSeq member function declarations

    /// Retrieves bitmap resources
    wxBitmap GetBitmapResource( const wxString& name );

    /// Retrieves icon resources
    wxIcon GetIconResource( const wxString& name );
////@end CFarPointerSeq member function declarations

    /// Should we show tooltips?
    static bool ShowToolTips();

    CRef<CCmdComposite> GetCommand();
private:
    string GetMessageForId(const CSeq_id &id, string label, bool &check, bool &disable);
    CRef<objects::CSeq_align> RunBlast(const CBioseq_Handle& sh, const CBioseq_Handle& qh);
    string ValidateAlignment(CRef<objects::CSeq_align> align, CBioseq_Handle bsh1, CBioseq_Handle bsh2, bool& check);
    void AddSpaces(string &str, const string& label_max, wxGraphicsContext *gc);
    void ReverseRow(CDense_seg &dense_seg, int offset, size_t row, CSeq_align::TDim num_rows);

////@begin CFarPointerSeq member variables
    wxCheckListBox* m_CheckListBox;
////@end CFarPointerSeq member variables
    CSeq_entry_Handle m_TopSeqEntry;
    vector<bool> m_disabled;
    map<string, string> m_errors;
    vector<string> m_labels;
    map<string, CBioseq_Handle> m_label_to_bsh1;
    map<string, CBioseq_Handle> m_label_to_bsh2;
    map<CBioseq_Handle, int> m_offset;
    map<CBioseq_Handle, bool> m_flip_strand;
};

END_NCBI_SCOPE
#endif
    // _CFARPOINTERSEQ_H_
