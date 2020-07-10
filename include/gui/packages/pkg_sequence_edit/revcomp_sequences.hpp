
/*  $Id: revcomp_sequences.hpp 38635 2017-06-05 18:51:04Z asztalos $
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

#ifndef _REVCOMP_SEQUENCES_H_
#define _REVCOMP_SEQUENCES_H_

#include <corelib/ncbistd.hpp>
//#include <gui/packages/pkg_sequence_edit/bulk_cmd_dlg.hpp>
//#include <gui/packages/pkg_sequence_edit/seq_table_grid.hpp>
//#include <gui/packages/pkg_sequence_edit/tbl_edit_dlg.hpp>

#include <gui/utils/command_processor.hpp>
#include <objects/seq/Seq_inst.hpp>
#include <objmgr/bioseq_handle.hpp>
#include <gui/widgets/data/report_dialog.hpp>
#include <gui/packages/pkg_sequence_edit/string_constraint_panel.hpp>

#include <wx/dialog.h>
#include <wx/listctrl.h>
#include <wx/msgdlg.h> 

BEGIN_NCBI_SCOPE
/*!
 * Control identifiers
 */

////@begin control identifiers
#define ID_REVCOMP_DLG 11300
#define SYMBOL_REVCOMP_DLG_STYLE wxCAPTION|wxRESIZE_BORDER|wxSYSTEM_MENU|wxCLOSE_BOX|wxTAB_TRAVERSAL
#define SYMBOL_REVCOMP_DLG_TITLE _("Reverse Complement")
#define SYMBOL_REVCOMP_DLG_IDNAME ID_REVCOMP_DLG
#define SYMBOL_REVCOMP_DLG_SIZE wxDefaultSize
#define SYMBOL_REVCOMP_DLG_POSITION wxDefaultPosition
////@end control identifiers
#define ID_SELECT 11302
#define ID_SELECTALL 11303
#define ID_UNSELECTALL 11304


class CRevCompSequencesDlg : public CReportEditingDialog
{    
    DECLARE_DYNAMIC_CLASS( CRevCompSequencesDlg )
    DECLARE_EVENT_TABLE()

public:
    /// Constructors
    CRevCompSequencesDlg();
    CRevCompSequencesDlg( wxWindow* parent, objects::CSeq_entry_Handle seh, wxWindowID id = SYMBOL_REVCOMP_DLG_IDNAME, const wxString& caption = SYMBOL_REVCOMP_DLG_TITLE, const wxPoint& pos = SYMBOL_REVCOMP_DLG_POSITION, const wxSize& size = SYMBOL_REVCOMP_DLG_SIZE, long style = SYMBOL_REVCOMP_DLG_STYLE );

    /// Creation
    bool Create( wxWindow* parent, wxWindowID id = SYMBOL_REVCOMP_DLG_IDNAME, const wxString& caption = SYMBOL_REVCOMP_DLG_TITLE, const wxPoint& pos = SYMBOL_REVCOMP_DLG_POSITION, const wxSize& size = SYMBOL_REVCOMP_DLG_SIZE, long style = SYMBOL_REVCOMP_DLG_STYLE );

    /// Destructor
    ~CRevCompSequencesDlg();

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

    void OnSelect( wxCommandEvent& event );
    void OnSelectAll( wxCommandEvent& event );
    void OnUnselectAll( wxCommandEvent& event );

    void apply(CSeq_entry_Handle tse, ICommandProccessor* cmdProcessor, string title, bool update_seq, bool update_feat, bool update_graph);
private:
    bool RevCompBioSeq(CBioseq_Handle bsh, CRef<CCmdComposite> cmd, CSeq_entry_Handle tse, bool update_seq, bool update_feat, bool update_graph);

    objects::CSeq_entry_Handle m_TopSeqEntry;
    wxListCtrl* m_ListCtrl;
    vector < CBioseq_Handle > m_Entries;
    CStringConstraintPanel *m_StringConstraintPanel;
    wxCheckBox * m_CheckSequence;
    wxCheckBox * m_CheckFeat;
};


END_NCBI_SCOPE

#endif
    // _REVCOMP_SEQUENCES_H_
