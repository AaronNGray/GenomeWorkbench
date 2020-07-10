/*  $Id: seqid_fix_dlg.hpp 29402 2013-12-09 19:30:46Z bollin $
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
#ifndef _SEQIDFIXDLG_H_
#define _SEQIDFIXDLG_H_

#include <corelib/ncbistd.hpp>

#include <objects/seqset/Seq_entry.hpp>

#include <gui/packages/pkg_sequence_edit/seq_table_grid.hpp>

/*!
 * Includes
 */

////@begin includes
////@end includes

/*!
 * Forward declarations
 */

////@begin forward declarations
////@end forward declarations

BEGIN_NCBI_SCOPE

/*!
 * Control identifiers
 */

////@begin control identifiers
#define ID_CSEQIDFIXDLG 10104
#define ID_RECHECK_SEQID_PROBLEMS_BTN 10105
#define SYMBOL_CSEQIDFIXDLG_STYLE wxCAPTION|wxRESIZE_BORDER|wxSYSTEM_MENU|wxCLOSE_BOX|wxTAB_TRAVERSAL
#define SYMBOL_CSEQIDFIXDLG_TITLE _("Sequence ID Problems")
#define SYMBOL_CSEQIDFIXDLG_IDNAME ID_CSEQIDFIXDLG
#define SYMBOL_CSEQIDFIXDLG_SIZE wxSize(400, 300)
#define SYMBOL_CSEQIDFIXDLG_POSITION wxDefaultPosition
////@end control identifiers


/*!
 * CSeqIdFixDlg class declaration
 */

class CSeqIdFixDlg: public wxDialog
{    
    DECLARE_DYNAMIC_CLASS( CSeqIdFixDlg )
    DECLARE_EVENT_TABLE()

public:
    /// Constructors
    CSeqIdFixDlg();
    CSeqIdFixDlg( wxWindow* parent, CRef<objects::CSeq_entry> entry_to_add, CConstRef<objects::CSeq_entry> curr_entry, wxWindowID id = SYMBOL_CSEQIDFIXDLG_IDNAME, const wxString& caption = SYMBOL_CSEQIDFIXDLG_TITLE, const wxPoint& pos = SYMBOL_CSEQIDFIXDLG_POSITION, const wxSize& size = SYMBOL_CSEQIDFIXDLG_SIZE, long style = SYMBOL_CSEQIDFIXDLG_STYLE );

    /// Creation
    bool Create( wxWindow* parent, wxWindowID id = SYMBOL_CSEQIDFIXDLG_IDNAME, const wxString& caption = SYMBOL_CSEQIDFIXDLG_TITLE, const wxPoint& pos = SYMBOL_CSEQIDFIXDLG_POSITION, const wxSize& size = SYMBOL_CSEQIDFIXDLG_SIZE, long style = SYMBOL_CSEQIDFIXDLG_STYLE );

    /// Destructor
    ~CSeqIdFixDlg();

    /// Initialises member variables
    void Init();

    /// Creates the controls and sizers
    void CreateControls();

////@begin CSeqIdFixDlg event handler declarations

    /// wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_RECHECK_SEQID_PROBLEMS_BTN
    void OnRecheckSeqidProblemsBtnClick( wxCommandEvent& event );

////@end CSeqIdFixDlg event handler declarations

////@begin CSeqIdFixDlg member function declarations

    /// Retrieves bitmap resources
    wxBitmap GetBitmapResource( const wxString& name );

    /// Retrieves icon resources
    wxIcon GetIconResource( const wxString& name );
////@end CSeqIdFixDlg member function declarations

    /// Should we show tooltips?
    static bool ShowToolTips();

////@begin CSeqIdFixDlg member variables
////@end CSeqIdFixDlg member variables

    CRef<objects::CSeq_table> GetReplacementTable();

private:
    CRef<objects::CSeq_entry> m_EntryToAdd;
    CConstRef<objects::CSeq_entry> m_CurrEntry;

    CSeqTableGridPanel* m_OldGrid;
    CSeqTableGridPanel* m_NewGrid;
    CRef<objects::CSeq_table> m_OldIds;

    void x_RefreshProblems();
};

END_NCBI_SCOPE

#endif
    // _SEQIDFIXDLG_H_
