/*  $Id: reorder_sequences_dlg.hpp 33127 2015-06-01 21:14:42Z filippov $
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
#ifndef _REORDER_SEQUENCES_DLG_H_
#define _REORDER_SEQUENCES_DLG_H_

#include <corelib/ncbistd.hpp>
#include <objmgr/seq_entry_handle.hpp>
#include <gui/objutils/cmd_composite.hpp>


#include <wx/dialog.h>

/*!
 * Includes
 */

////@begin includes
////@end includes
#include <wx/sizer.h>
#include <wx/listctrl.h>
#include <wx/bmpbuttn.h>
#include <gui/widgets/wx/wx_utils.hpp>

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
#define ID_CREORDER_SEQUENCES_DLG 11100
#define SYMBOL_CREORDER_SEQUENCES_DLG_STYLE wxCAPTION|wxRESIZE_BORDER|wxSYSTEM_MENU|wxCLOSE_BOX|wxTAB_TRAVERSAL
#define SYMBOL_CREORDER_SEQUENCES_DLG_TITLE _("Reorder Sequences Dialog")
#define SYMBOL_CREORDER_SEQUENCES_DLG_IDNAME ID_CREORDER_SEQUENCES_DLG
#define SYMBOL_CREORDER_SEQUENCES_DLG_SIZE wxDefaultSize
#define SYMBOL_CREORDER_SEQUENCES_DLG_POSITION wxDefaultPosition
////@end control identifiers
#define ID_LISTCTRL_REORDER 11101
#define ID_BUTTON_UP 11102
#define ID_BUTTON_DOWN 11103
#define ID_BUTTON_SORT 11104



class CReorderSequencesDlg: public wxDialog
{    
    DECLARE_DYNAMIC_CLASS( CReorderSequencesDlg )
    DECLARE_EVENT_TABLE()

public:
    /// Constructors
    CReorderSequencesDlg();
    CReorderSequencesDlg( wxWindow* parent, objects::CSeq_entry_Handle seh, wxWindowID id = SYMBOL_CREORDER_SEQUENCES_DLG_IDNAME, const wxString& caption = SYMBOL_CREORDER_SEQUENCES_DLG_TITLE, const wxPoint& pos = SYMBOL_CREORDER_SEQUENCES_DLG_POSITION, const wxSize& size = SYMBOL_CREORDER_SEQUENCES_DLG_SIZE, long style = SYMBOL_CREORDER_SEQUENCES_DLG_STYLE );

    /// Creation
    bool Create( wxWindow* parent, wxWindowID id = SYMBOL_CREORDER_SEQUENCES_DLG_IDNAME, const wxString& caption = SYMBOL_CREORDER_SEQUENCES_DLG_TITLE, const wxPoint& pos = SYMBOL_CREORDER_SEQUENCES_DLG_POSITION, const wxSize& size = SYMBOL_CREORDER_SEQUENCES_DLG_SIZE, long style = SYMBOL_CREORDER_SEQUENCES_DLG_STYLE );

    /// Destructor
    ~CReorderSequencesDlg();

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
    static CRef<CCmdComposite>  ReorderById(objects::CSeq_entry_Handle seh);

    void OnButtonUp( wxCommandEvent& event );
    void OnButtonDown( wxCommandEvent& event );
    void OnButtonSort( wxCommandEvent& event );
private:
    objects::CSeq_entry_Handle m_TopSeqEntry;
    wxListCtrl* m_ListCtrl;
    wxButton* m_SortButton;
    bool m_ReverseSort;
    wxBitmap m_down_bitmap, m_up_bitmap;
    vector < pair<CBioseq_set_Handle,CSeq_entry_Handle> > m_SetSeq;
    static void PopulateSet(CBioseq_set_Handle bssh, CBioseq_set& new_set, const vector<int> &list_index, const vector < pair<CBioseq_set_Handle,CSeq_entry_Handle> > &set_seq);
    static void ReadBioseq(const CSeq_entry& se,vector < pair<CBioseq_set_Handle,CSeq_entry_Handle> > &setseq, CSeq_entry_Handle seh);
    static string GetIdLabel(CBioseq_Handle bsh, bool &numeric);
    void PrepareList(vector<int> &list_index);
};

END_NCBI_SCOPE

#endif
    // _REORDER_SEQUENCES_DLG_H_
