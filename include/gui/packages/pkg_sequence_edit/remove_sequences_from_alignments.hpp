/*  $Id: remove_sequences_from_alignments.hpp 40701 2018-04-03 20:43:31Z filippov $
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
#ifndef _REMOVE_SEQ_FROM_ALIGN_DLG_H_
#define _REMOVE_SEQ_FROM_ALIGN_DLG_H_

#include <corelib/ncbistd.hpp>
#include <objmgr/seq_entry_handle.hpp>
#include <gui/objutils/cmd_composite.hpp>
#include <objects/seqalign/Std_seg.hpp>

#include <wx/dialog.h>

/*!
 * Includes
 */

////@begin includes
////@end includes
#include <wx/sizer.h>
#include <wx/listctrl.h>
#include <wx/radiobut.h>
#include <wx/checkbox.h>
#include <wx/textctrl.h>
#include <gui/widgets/wx/wx_utils.hpp>
#include <gui/widgets/data/report_dialog.hpp>

/*!
 * Forward declarations
 */

////@begin forward declarations
class wxBoxSizer;
////@end forward declarations

BEGIN_NCBI_SCOPE
using namespace objects;

/*!
 * Control identifiers
 */

////@begin control identifiers
#define ID_CREMOVE_SEQ_FROM_ALIGN_DLG 12000
#define SYMBOL_CREMOVE_SEQ_FROM_ALIGN_DLG_STYLE wxCAPTION|wxRESIZE_BORDER|wxSYSTEM_MENU|wxCLOSE_BOX|wxTAB_TRAVERSAL
#define SYMBOL_CREMOVE_SEQ_FROM_ALIGN_DLG_TITLE _("Remove Sequences From Alignments")
#define SYMBOL_CREMOVE_SEQ_FROM_ALIGN_DLG_IDNAME ID_CREMOVE_SEQ_FROM_ALIGN_DLG
#define SYMBOL_CREMOVE_SEQ_FROM_ALIGN_DLG_SIZE wxDefaultSize
#define SYMBOL_CREMOVE_SEQ_FROM_ALIGN_DLG_POSITION wxDefaultPosition
////@end control identifiers
#define ID_LISTCTRL_SEQ_ALIGN 12001

class CRemoveSeqFromAlignDlg : public CReportEditingDialog
{    
    DECLARE_DYNAMIC_CLASS( CRemoveSeqFromAlignDlg )
    DECLARE_EVENT_TABLE()

public:
    /// Constructors
    CRemoveSeqFromAlignDlg();
    CRemoveSeqFromAlignDlg( wxWindow* parent, objects::CSeq_entry_Handle seh, wxWindowID id = SYMBOL_CREMOVE_SEQ_FROM_ALIGN_DLG_IDNAME, const wxString& caption = SYMBOL_CREMOVE_SEQ_FROM_ALIGN_DLG_TITLE, const wxPoint& pos = SYMBOL_CREMOVE_SEQ_FROM_ALIGN_DLG_POSITION, const wxSize& size = SYMBOL_CREMOVE_SEQ_FROM_ALIGN_DLG_SIZE, long style = SYMBOL_CREMOVE_SEQ_FROM_ALIGN_DLG_STYLE );

    /// Creation
    bool Create( wxWindow* parent, wxWindowID id = SYMBOL_CREMOVE_SEQ_FROM_ALIGN_DLG_IDNAME, const wxString& caption = SYMBOL_CREMOVE_SEQ_FROM_ALIGN_DLG_TITLE, const wxPoint& pos = SYMBOL_CREMOVE_SEQ_FROM_ALIGN_DLG_POSITION, const wxSize& size = SYMBOL_CREMOVE_SEQ_FROM_ALIGN_DLG_SIZE, long style = SYMBOL_CREMOVE_SEQ_FROM_ALIGN_DLG_STYLE );

    /// Destructor
    ~CRemoveSeqFromAlignDlg();

    /// Initialises member variables
    void Init();
    void Init(objects::CSeq_entry_Handle seh);

    /// Creates the controls and sizers
    void CreateControls();

    /// Retrieves bitmap resources
    wxBitmap GetBitmapResource( const wxString& name );

    /// Retrieves icon resources
    wxIcon GetIconResource( const wxString& name );

    /// Should we show tooltips?
    static bool ShowToolTips();

    CRef<CCmdComposite> GetCommand();
    static void ApplyToCSeq_entry (objects::CSeq_entry_Handle tse, CCmdComposite* composite, const vector<string> &names);

private:
    objects::CSeq_entry_Handle m_TopSeqEntry;
    set<string> m_Label;
    wxListCtrl* m_ListCtrl;
    static bool RemoveIdLabel(const CSeq_align& align, string name, bool& modified);
    static void ModifyAlign(CRef<CSeq_align> align, const string name);
    void GetAlignLabel(const CSeq_align& align);
    void SearchSeq_entry (objects::CSeq_entry_Handle tse, const CSeq_entry& se);
    static bool GetIdsFromLocs(const CStd_seg::TLoc& locs,  set<string> &labels, const string name="", bool detect=false, bool found=false);
    static bool GetIdsFromLoc(const CSeq_loc& loc,  set<string> &labels, const string name="", bool detect=false, bool found=false);
};



END_NCBI_SCOPE

#endif
    // _REMOVE_SEQ_FROM_ALIGN_DLG_H_
