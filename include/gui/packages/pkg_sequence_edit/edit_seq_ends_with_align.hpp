/*  $Id: edit_seq_ends_with_align.hpp 38635 2017-06-05 18:51:04Z asztalos $
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
#ifndef _EDIT_SEQ_ENDS_WITH_ALIGN_H_
#define _EDIT_SEQ_ENDS_WITH_ALIGN_H_

#include <corelib/ncbistd.hpp>
#include <objmgr/seq_entry_handle.hpp>
#include <gui/objutils/cmd_composite.hpp>
#include <gui/widgets/data/report_dialog.hpp>
#include <wx/dialog.h>
#include <wx/textctrl.h>

BEGIN_NCBI_SCOPE
USING_SCOPE(objects);

#define ID_EDIT_SEQ_ENDS_WITH_ALIGN wxID_ANY
#define SYMBOL_EDIT_SEQ_ENDS_WITH_ALIGN_STYLE wxCAPTION|wxRESIZE_BORDER|wxSYSTEM_MENU|wxCLOSE_BOX|wxTAB_TRAVERSAL
#define SYMBOL_EDIT_SEQ_ENDS_WITH_ALIGN_TITLE _("Edit Sequence Ends With Alignment")
#define SYMBOL_EDIT_SEQ_ENDS_WITH_ALIGN_IDNAME ID_EDIT_SEQ_ENDS_WITH_ALIGN
#define SYMBOL_EDIT_SEQ_ENDS_WITH_ALIGN_SIZE wxDefaultSize
#define SYMBOL_EDIT_SEQ_ENDS_WITH_ALIGN_POSITION wxDefaultPosition

class CEditSeqEndsWithAlign : public CReportEditingDialog
{    
    DECLARE_DYNAMIC_CLASS( CEditSeqEndsWithAlign )
    DECLARE_EVENT_TABLE()

public:
    /// Constructors
    CEditSeqEndsWithAlign();
    CEditSeqEndsWithAlign( wxWindow* parent, objects::CSeq_entry_Handle seh, wxWindowID id = SYMBOL_EDIT_SEQ_ENDS_WITH_ALIGN_IDNAME, const wxString& caption = SYMBOL_EDIT_SEQ_ENDS_WITH_ALIGN_TITLE, const wxPoint& pos = SYMBOL_EDIT_SEQ_ENDS_WITH_ALIGN_POSITION, const wxSize& size = SYMBOL_EDIT_SEQ_ENDS_WITH_ALIGN_SIZE, long style = SYMBOL_EDIT_SEQ_ENDS_WITH_ALIGN_STYLE );

    /// Creation
    bool Create( wxWindow* parent, wxWindowID id = SYMBOL_EDIT_SEQ_ENDS_WITH_ALIGN_IDNAME, const wxString& caption = SYMBOL_EDIT_SEQ_ENDS_WITH_ALIGN_TITLE, const wxPoint& pos = SYMBOL_EDIT_SEQ_ENDS_WITH_ALIGN_POSITION, const wxSize& size = SYMBOL_EDIT_SEQ_ENDS_WITH_ALIGN_SIZE, long style = SYMBOL_EDIT_SEQ_ENDS_WITH_ALIGN_STYLE );

    /// Destructor
    ~CEditSeqEndsWithAlign();

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

    CRef<CCmdComposite> GetCommand();


    static int FindRow(const CBioseq_Handle& bsh, const CSeq_align& align);
    static int FindPosition(const CSeq_align& align, int row, int after);
    static void TrimBeforeAfter(CBioseq_Handle edited_bsh, int before, int after);
    static void AdjustFeatureLocations(CBioseq_Handle bsh, int before, int after, CRef<CCmdComposite> cmd);
    static CRef<CSeq_loc> AdjustLoc(const CSeq_loc& feat_loc, int before, int after, TSeqPos seq_start, bool &modified, bool &removed);
    static void AdjustFrame(CCdregion &feat, int before, int diff);
    static void TrimQualityScores(CBioseq_Handle bsh, int before, int after, CRef<CCmdComposite> command);
private:
    CSeq_entry_Handle m_TopSeqEntry;
    wxTextCtrl *m_Before;
    wxTextCtrl *m_After;
};

END_NCBI_SCOPE

#endif
    // _EDIT_SEQ_ENDS_WITH_ALIGN_H_
