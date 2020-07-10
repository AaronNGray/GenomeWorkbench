/*  $Id: csuc_frame.hpp 43609 2019-08-08 16:12:53Z filippov $
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
 *
 */

#ifndef _CSUC_FRAME_H_
#define _CSUC_FRAME_H_

#include <corelib/ncbistd.hpp>

#include <gui/framework/workbench.hpp>
#include <gui/utils/event_handler.hpp>
#include <gui/widgets/edit/suc_data.hpp>
#include <gui/core/selection_client.hpp>
#include <gui/utils/job_adapter.hpp>
#include <gui/widgets/seq/flat_file_ctrl.hpp>

/*!
 * Includes
 */

////@begin includes
#include "wx/frame.h"
#include "wx/treectrl.h"
#include <wx/listctrl.h>
////@end includes


BEGIN_NCBI_SCOPE


/*!
 * Forward declarations
 */

////@begin forward declarations
////@end forward declarations

/*!
 * Control identifiers
 */

////@begin control identifiers
#define ID_CCLICKABLESORTUNIQUECOUNT 17000
#define ID_CCLICKABLESORTUNIQUECOUNT_PANEL 17001
#define ID_CCLICKABLESORTUNIQUECOUNT_TREECTRL 17002
#define ID_CCLICKABLESORTUNIQUECOUNT_TEXTCTRL 17003
#define ID_CCLICKABLESORTUNIQUECOUNT_REFRESH 17004
#define ID_CCLICKABLESORTUNIQUECOUNT_EXPAND 17005
#define ID_CCLICKABLESORTUNIQUECOUNT_COLLAPSE 17006
#define ID_CCLICKABLESORTUNIQUECOUNT_DISMISS 17007
#define SYMBOL_CCLICKABLESORTUNIQUECOUNT_STYLE wxCAPTION|wxRESIZE_BORDER|wxSYSTEM_MENU|wxCLOSE_BOX
#define SYMBOL_CCLICKABLESORTUNIQUECOUNT_TITLE _("Clickable Sort Unique Count")
#define SYMBOL_CCLICKABLESORTUNIQUECOUNT_IDNAME ID_CCLICKABLESORTUNIQUECOUNT
#define SYMBOL_CCLICKABLESORTUNIQUECOUNT_SIZE wxSize(800, 600)
#define SYMBOL_CCLICKABLESORTUNIQUECOUNT_POSITION wxDefaultPosition
////@end control identifiers


/*!
 * CClickableSortUniqueCount class declaration
 */

class CClickableSortUniqueCount: public wxFrame,  public CEventHandler, public CJobAdapter::IJobCallback
{    
    DECLARE_CLASS( CClickableSortUniqueCount )
    DECLARE_EVENT_TABLE()

    DECLARE_EVENT_MAP();

public:
    /// Constructors
    CClickableSortUniqueCount();
    CClickableSortUniqueCount( wxWindow* parent, IWorkbench* wb, wxWindowID id = SYMBOL_CCLICKABLESORTUNIQUECOUNT_IDNAME, const wxString& caption = SYMBOL_CCLICKABLESORTUNIQUECOUNT_TITLE, const wxPoint& pos = SYMBOL_CCLICKABLESORTUNIQUECOUNT_POSITION, const wxSize& size = SYMBOL_CCLICKABLESORTUNIQUECOUNT_SIZE, long style = SYMBOL_CCLICKABLESORTUNIQUECOUNT_STYLE );

    bool Create( wxWindow* parent, wxWindowID id = SYMBOL_CCLICKABLESORTUNIQUECOUNT_IDNAME, const wxString& caption = SYMBOL_CCLICKABLESORTUNIQUECOUNT_TITLE, const wxPoint& pos = SYMBOL_CCLICKABLESORTUNIQUECOUNT_POSITION, const wxSize& size = SYMBOL_CCLICKABLESORTUNIQUECOUNT_SIZE, long style = SYMBOL_CCLICKABLESORTUNIQUECOUNT_STYLE );

    /// Destructor
    virtual ~CClickableSortUniqueCount();

    /// Initialises member variables
    void Init();

    /// Creates the controls and sizers
    void CreateControls();

 /// @name CJobAdapter::IJobCallback interface implementation
    /// @{
    virtual void OnJobResult(CObject* result, CJobAdapter& adapter);
    virtual void OnJobFailed(const string&, CJobAdapter& adapter);
    /// @}

   void OnChildFocus(wxChildFocusEvent& evt);

////@begin CClickableSortUniqueCount event handler declarations
    void OnRefresh(wxCommandEvent& event );
    void OnDismiss(wxCommandEvent& event );
    void OnExpand(wxCommandEvent& event );
    void OnCollapse(wxCommandEvent& event );
    void OnTreeSelectionChanged( wxTreeEvent& event );
    void OnTreeItemActivated( wxTreeEvent& event );
    void OnListItemSelected(wxListEvent& event);
    void OnListItemActivated(wxListEvent& event);
    void OnDataChanged(CEvent* evt);
////@end CClickableSortUniqueCount event handler declarations

////@begin CClickableSortUniqueCount member function declarations

    /// Retrieves bitmap resources
    wxBitmap GetBitmapResource( const wxString& name );

    /// Retrieves icon resources
    wxIcon GetIconResource( const wxString& name );
////@end CClickableSortUniqueCount member function declarations

    /// Should we show tooltips?
    static bool ShowToolTips();

////@begin CClickableSortUniqueCount member variables
////@end CClickableSortUniqueCount member variables

private:
    void LoadData(void);
    void UpdateTree(CRef<CSUCResults> res);
    void AddToListCtrl(wxTreeItemId& id);
    void AddToBulkEditList(wxTreeItemId& id, vector<CSeq_feat_Handle> &cds, vector<CSeq_feat_Handle> &gene, vector<CSeq_feat_Handle> &rna,
                           vector<CBioseq_Handle> &biosrc);
    void ClearListCtrl(void);
    void GetExpandedNodes(wxTreeItemId id, set<string> &expanded_nodes);
    void SetExpandedNodes(wxTreeItemId id, const set<string> &expanded_nodes);
    void ConnectListener();
    void DisconnectListener();

    IWorkbench*     m_Workbench;
    wxListCtrl* m_ListCtrl;
    wxTreeCtrl* m_TreeCtrl;
    map<wxTreeItemId, CRef<CSUCLine> > m_item_to_line;
    CRef<CScope> m_Scope;
    CIRef<IFlatFileCtrl> m_FlatFileCtrl;
    vector<pair<CRef<CSUCLine>, size_t> > m_CurLines;
    CRef<CJobAdapter> m_JobAdapter;
    CRef<CSelectionClient> m_SelectionClient;
    wxStaticText* m_StatusText;
    bool m_SelectionClientAttached;
    bool m_IsBusy;
    bool m_TreeDeleteAllItemsFlag;
};


END_NCBI_SCOPE

#endif
    // _CSUC_FRAME_H_
