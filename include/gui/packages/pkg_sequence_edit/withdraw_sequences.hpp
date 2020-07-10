/*  $Id: withdraw_sequences.hpp 40701 2018-04-03 20:43:31Z filippov $
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
#ifndef _WITHDRAW_SEQUENCES_H_
#define _WITHDRAW_SEQUENCES_H_

#include <corelib/ncbistd.hpp>
#include <objmgr/seq_entry_handle.hpp>
#include <gui/objutils/cmd_composite.hpp>
#include <gui/widgets/data/report_dialog.hpp>
#include <gui/packages/pkg_sequence_edit/string_constraint_panel.hpp>
#include <gui/packages/pkg_sequence_edit/field_choice_panel.hpp>
#include <gui/widgets/edit/feature_field_name_panel.hpp>
#include <gui/packages/pkg_sequence_edit/segregate_sets.hpp>
#include <wx/notebook.h>

/*!
 * Includes
 */

////@begin includes
////@end includes
#include <wx/sizer.h>
#include <wx/listctrl.h>
#include <wx/treectrl.h>
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
#define ID_CWITHDRAW_SEQUENCES 11300
#define SYMBOL_CWITHDRAW_SEQUENCES_STYLE wxCAPTION|wxRESIZE_BORDER|wxSYSTEM_MENU|wxCLOSE_BOX|wxTAB_TRAVERSAL
#define SYMBOL_CWITHDRAW_SEQUENCES_TITLE _("Withdraw Sequences")
#define SYMBOL_CWITHDRAW_SEQUENCES_IDNAME ID_CWITHDRAW_SEQUENCES
#define SYMBOL_CWITHDRAW_SEQUENCES_SIZE wxDefaultSize
#define SYMBOL_CWITHDRAW_SEQUENCES_POSITION wxDefaultPosition
////@end control identifiers


class CWithdrawSequences : public CSegregateSetsBase, public CReportEditingDialog
{    
    DECLARE_DYNAMIC_CLASS( CWithdrawSequences )
    DECLARE_EVENT_TABLE()

public:
    /// Constructors
    CWithdrawSequences();
    CWithdrawSequences( wxWindow* parent, objects::CSeq_entry_Handle seh, wxWindowID id = SYMBOL_CWITHDRAW_SEQUENCES_IDNAME, const wxString& caption = SYMBOL_CWITHDRAW_SEQUENCES_TITLE, const wxPoint& pos = SYMBOL_CWITHDRAW_SEQUENCES_POSITION, const wxSize& size = SYMBOL_CWITHDRAW_SEQUENCES_SIZE, long style = SYMBOL_CWITHDRAW_SEQUENCES_STYLE );

    /// Creation
    bool Create( wxWindow* parent, wxWindowID id = SYMBOL_CWITHDRAW_SEQUENCES_IDNAME, const wxString& caption = SYMBOL_CWITHDRAW_SEQUENCES_TITLE, const wxPoint& pos = SYMBOL_CWITHDRAW_SEQUENCES_POSITION, const wxSize& size = SYMBOL_CWITHDRAW_SEQUENCES_SIZE, long style = SYMBOL_CWITHDRAW_SEQUENCES_STYLE );

    /// Destructor
    ~CWithdrawSequences();

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
    void PopulateSet(CBioseq_set& new_set, const vector<int> &subset);
    vector< vector<int> > & GetSubsets() {return m_IDPanel->GetSubsets();}
    void CopyDescrToEntry(const CBioseq_set& source, CSeq_entry& target);
    vector<string> GetNames();

private:
    CIDSubpanel *m_IDPanel;
};



END_NCBI_SCOPE

#endif
    // _WITHDRAW_SEQUENCES_H_
