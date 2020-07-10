/*  $Id: fix_for_trans_splicing.hpp 42192 2019-01-10 16:52:15Z filippov $
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
#ifndef _FIX_FOR_TRANS_SPLICING_H_
#define _FIX_FOR_TRANS_SPLICING_H_

#include <corelib/ncbistd.hpp>
#include <objmgr/seq_entry_handle.hpp>
#include <objmgr/bioseq_set_handle.hpp>
#include <gui/objutils/cmd_composite.hpp>
#include <gui/objutils/cmd_del_seq_feat.hpp>
#include <gui/widgets/seq/flat_file_ctrl.hpp>
#include <gui/core/selection_client.hpp>
#include <gui/packages/pkg_sequence_edit/bulk_cmd_dlg.hpp>

/*!
 * Includes
 */

////@begin includes
////@end includes
#include <wx/sizer.h>
#include <wx/choice.h>
#include <wx/listbox.h>
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
#define ID_FIX_FOR_TRANS_SPLICING wxID_ANY
#define SYMBOL_FIX_FOR_TRANS_SPLICING_STYLE wxCAPTION|wxRESIZE_BORDER|wxSYSTEM_MENU|wxCLOSE_BOX|wxTAB_TRAVERSAL
#define SYMBOL_FIX_FOR_TRANS_SPLICING_TITLE _("Fix For Trans-splicing")
#define SYMBOL_FIX_FOR_TRANS_SPLICING_IDNAME ID_FIX_FOR_TRANS_SPLICING
#define SYMBOL_FIX_FOR_TRANS_SPLICING_SIZE wxDefaultSize
#define SYMBOL_FIX_FOR_TRANS_SPLICING_POSITION wxDefaultPosition
////@end control identifiers

class CFixForTransSplicing: public CBulkCmdDlg
{    
    DECLARE_DYNAMIC_CLASS( CFixForTransSplicing )
    DECLARE_EVENT_TABLE()

public:
    /// Constructors
    CFixForTransSplicing();
    CFixForTransSplicing( wxWindow* parent, IWorkbench *wb, wxWindowID id = SYMBOL_FIX_FOR_TRANS_SPLICING_IDNAME, const wxString& caption = SYMBOL_FIX_FOR_TRANS_SPLICING_TITLE, const wxPoint& pos = SYMBOL_FIX_FOR_TRANS_SPLICING_POSITION, const wxSize& size = SYMBOL_FIX_FOR_TRANS_SPLICING_SIZE, long style = SYMBOL_FIX_FOR_TRANS_SPLICING_STYLE );

    /// Creation
    bool Create( wxWindow* parent, wxWindowID id = SYMBOL_FIX_FOR_TRANS_SPLICING_IDNAME, const wxString& caption = SYMBOL_FIX_FOR_TRANS_SPLICING_TITLE, const wxPoint& pos = SYMBOL_FIX_FOR_TRANS_SPLICING_POSITION, const wxSize& size = SYMBOL_FIX_FOR_TRANS_SPLICING_SIZE, long style = SYMBOL_FIX_FOR_TRANS_SPLICING_STYLE );

    /// Destructor
    ~CFixForTransSplicing();

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

    virtual CRef<CCmdComposite> GetCommand();
    virtual void ExecuteCmd(CRef<CCmdComposite> cmd);
    virtual string GetErrorMessage(){ return kEmptyStr;}
    void OnProteinNameSelected( wxCommandEvent& event );
    void OnPageLeft( wxCommandEvent& event );
    void OnPageRight( wxCommandEvent& event );
    void OnListItemSelected(wxCommandEvent& event);
    void OnListItemActivated(wxCommandEvent& event);
private:
    void CollectProteinNames();
    void CombineFeatures(const  vector<CSeq_feat_Handle> &feats, CRef<CCmdComposite> cmd, CScope &scope, int &offset, CSeq_loc::TOpFlags merge_flag, bool second_pass, bool create_general_only);
    void PopulateListBox();
    void ReloadData();

    enum EControlIds
    {
        ID_PROTEIN_NAME = 23001,
        ID_CDS_LIST,
        ID_PAGE_LEFT,
        ID_PAGE_RIGHT
    };
    wxChoice *m_Protein;
    wxArrayString m_ProteinNames;
    wxListBox *m_ListBox;
    map<string, map<CBioseq_Handle, wxArrayString> > m_name_to_cds;
    map<string, map<CBioseq_Handle, vector<CSeq_feat_Handle> > > m_name_to_obj;
    vector<CBioseq_Handle> m_bioseqs;
    int m_page;
    wxButton *m_PageLeft;
    wxButton *m_PageRight;
    CIRef<IFlatFileCtrl> m_FlatFileCtrl;
    CRef<CSelectionClient> m_SelectionClient;
    bool m_SelectionClientAttached;
    wxStaticText *m_BioseqLabel;
};

END_NCBI_SCOPE

#endif
    // _FIX_FOR_TRANS_SPLICING_H_
