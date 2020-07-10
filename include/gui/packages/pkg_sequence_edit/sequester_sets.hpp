/*  $Id: sequester_sets.hpp 42600 2019-03-25 17:32:27Z filippov $
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
#ifndef _SEQUESTER_SETS_H_
#define _SEQUESTER_SETS_H_

#include <corelib/ncbistd.hpp>
#include <objmgr/seq_entry_handle.hpp>
#include <objmgr/blob_id.hpp>
#include <gui/objutils/cmd_composite.hpp>
#include <gui/framework/workbench_impl.hpp>
#include <gui/packages/pkg_sequence_edit/string_constraint_panel.hpp>
#include <gui/packages/pkg_sequence_edit/field_choice_panel.hpp>
#include <gui/widgets/edit/feature_field_name_panel.hpp>
#include <gui/packages/pkg_sequence_edit/segregate_sets.hpp>

#include <wx/dialog.h>
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
#define ID_CSEQUESTER_SETS wxID_ANY
#define SYMBOL_CSEQUESTER_SETS_STYLE wxCAPTION|wxRESIZE_BORDER|wxSYSTEM_MENU|wxCLOSE_BOX|wxTAB_TRAVERSAL
#define SYMBOL_CSEQUESTER_SETS_TITLE _("Sequester Sets")
#define SYMBOL_CSEQUESTER_SETS_IDNAME ID_CSEQUESTER_SETS
#define SYMBOL_CSEQUESTER_SETS_SIZE wxDefaultSize
#define SYMBOL_CSEQUESTER_SETS_POSITION wxDefaultPosition
////@end control identifiers
#define ID_SEQUESTER_BUTTON 11202
#define ID_CANCEL_BUTTON 11203
#define ID_ACCEPT_BUTTON 11204


class CSequesterSets : public CSegregateSetsBase,   public wxFrame
{    
    DECLARE_DYNAMIC_CLASS( CSequesterSets )
    DECLARE_EVENT_TABLE()

public:
    /// Constructors
    CSequesterSets();
    CSequesterSets( wxWindow* parent, objects::CSeq_entry_Handle seh, CConstRef<CSeq_submit> seq_submit, IWorkbench* wb, wxWindowID id = SYMBOL_CSEQUESTER_SETS_IDNAME, const wxString& caption = SYMBOL_CSEQUESTER_SETS_TITLE, const wxPoint& pos = SYMBOL_CSEQUESTER_SETS_POSITION, const wxSize& size = SYMBOL_CSEQUESTER_SETS_SIZE, long style = SYMBOL_CSEQUESTER_SETS_STYLE );

    /// Creation
    bool Create( wxWindow* parent, wxWindowID id = SYMBOL_CSEQUESTER_SETS_IDNAME, const wxString& caption = SYMBOL_CSEQUESTER_SETS_TITLE, const wxPoint& pos = SYMBOL_CSEQUESTER_SETS_POSITION, const wxSize& size = SYMBOL_CSEQUESTER_SETS_SIZE, long style = SYMBOL_CSEQUESTER_SETS_STYLE );

    /// Destructor
    virtual ~CSequesterSets();

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

    vector< vector<int> > & GetSubsets();
    void SetSubsets(const set<CBioseq_Handle> &selected);
    void OnSequester( wxCommandEvent& event );
    void OnCancel( wxCommandEvent& event );
    void OnAccept( wxCommandEvent& event );
    CRef<CUser_object> GetParams();
    void RestoreTextView();

private:
    void RecursiveCollectSets(const CBioseq_set_Handle &bssh, vector<CBioseq_set_Handle> &set_vec, set<CBioseq_set_Handle> &unique_set);
    void RecursivePropagateDown(void);
    void RecursivePropagateUp(void);
    void BioseqSetDescriptorPropagateUp(CBioseq_set_Handle set);
    void BioseqSetDescriptorPropagateDown( const CBioseq_set_Handle & bioseq_set_h);
    void EnableSmartPackageDone(bool enable);
    bool ShoudStayInPlace(const CSeqdesc& desc);

    wxNotebook *m_Notebook;
    IWorkbench*     m_Workbench;
    CIRef<IProjectView> m_TextView;
    CRef<CSeq_entry> m_SavedEntry;

    CConstRef<CSeq_submit> m_SeqSubmit;

    wxButton* m_SequesterButton;
    wxButton* m_AcceptButton;
};

END_NCBI_SCOPE

#endif
    // _SEQUESTER_SETS_H_
