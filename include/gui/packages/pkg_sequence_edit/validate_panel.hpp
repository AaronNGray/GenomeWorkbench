/*  $Id: validate_panel.hpp 44126 2019-11-01 16:58:03Z filippov $
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

#ifndef _VALIDATE_PANEL_H_
#define _VALIDATE_PANEL_H_

#include <corelib/ncbistd.hpp>
#include <gui/objects/GBProjectHandle.hpp>
#include <gui/framework/workbench.hpp>
#include <gui/core/selection_client.hpp>
#include <gui/utils/event_handler.hpp>
#include <gui/utils/job_adapter.hpp>
#include <objects/valerr/ValidError.hpp>
#include <objects/valerr/ValidErrItem.hpp>
#include <objects/general/User_object.hpp>
#include <objmgr/scope.hpp>
#include <objmgr/seq_entry_handle.hpp>
#include <objmgr/bioseq_handle.hpp>
#include <gui/widgets/seq/flat_file_ctrl.hpp>
#include <gui/widgets/edit/table_data_validate.hpp>
#include <gui/widgets/edit/table_data_validate_params.hpp>
#include <gui/core/project_view.hpp>
/*!
 * Includes
 */

////@begin includes
#include "wx/frame.h"
#include <wx/listctrl.h>
#include <wx/stattext.h>
////@end includes

class wxChoice;

BEGIN_NCBI_SCOPE


/*!
 * Forward declarations
 */

////@begin forward declarations
class CSeqGraphicWidget;
class IProjectView;
////@end forward declarations

/*!
 * Control identifiers
 */

////@begin control identifiers
#define ID_VALIDATE_FRAME_PANEL 17001
#define ID_VALIDATE_FRAME_SEVERITY 17002
#define ID_VALIDATE_FRAME_TEXTCTRL 17003
#define ID_VALIDATE_FRAME_ERRCODE 17005
#define SYMBOL_VALIDATE_PANEL_STYLE wxSUNKEN_BORDER|wxTAB_TRAVERSAL
#define SYMBOL_VALIDATE_PANEL_IDNAME ID_VALIDATE_FRAME_PANEL
#define SYMBOL_VALIDATE_PANEL_SIZE wxDefaultSize
#define SYMBOL_VALIDATE_PANEL_POSITION wxDefaultPosition
////@end control identifiers


/*!
 * CValidatePanel class declaration
 */

class CValidatePanel : public wxPanel
    , public CEventHandler
    , public CJobAdapter::IJobCallback
{    
    DECLARE_CLASS( CValidatePanel )
    DECLARE_EVENT_TABLE()

    DECLARE_EVENT_MAP();

public:
    /// Constructors
    CValidatePanel();
    CValidatePanel( wxWindow* parent, IWorkbench* wb, CRef<objects::CUser_object> params, wxWindowID id = SYMBOL_VALIDATE_PANEL_IDNAME, const wxPoint& pos = SYMBOL_VALIDATE_PANEL_POSITION, const wxSize& size = SYMBOL_VALIDATE_PANEL_SIZE, long style = SYMBOL_VALIDATE_PANEL_STYLE );

    bool Create( wxWindow* parent, wxWindowID id = SYMBOL_VALIDATE_PANEL_IDNAME, const wxPoint& pos = SYMBOL_VALIDATE_PANEL_POSITION, const wxSize& size = SYMBOL_VALIDATE_PANEL_SIZE, long style = SYMBOL_VALIDATE_PANEL_STYLE );

    /// Destructor
    virtual ~CValidatePanel();

    /// Initialises member variables
    void Init();

    /// Creates the controls and sizers
    void CreateControls();

    /// @name CJobAdapter::IJobCallback interface implementation
    /// @{
    virtual void OnJobResult(CObject* result, CJobAdapter& adapter);
    virtual void OnJobFailed(const string&, CJobAdapter& adapter);
    /// @}

////@begin CValidatePanel event handler declarations
    void OnListItemSelected(wxListEvent& event);
    void OnListItemActivated(wxListEvent& event);
    void OnListItemRightClick(wxListEvent& event);
    void OnValidatorSeveritySelected( wxCommandEvent& event );
    void OnErrcodeChoiceSelected( wxCommandEvent& event );
    void OnColumnClicked(wxListEvent& event);
    void OnMouseMove( wxMouseEvent &event );
////@end CValidatePanel event handler declarations

    void OnChildFocus(wxChildFocusEvent& evt);

////@begin CValidatePanel member function declarations

    /// Retrieves bitmap resources
    wxBitmap GetBitmapResource( const wxString& name );

    /// Retrieves icon resources
    wxIcon GetIconResource( const wxString& name );
////@end CValidatePanel member function declarations

    /// Should we show tooltips?
    static bool ShowToolTips();

////@begin CValidatePanel member variables
////@end CValidatePanel member variables
    bool GetAscending(void);
    string GetItemText(long item);

    void GetSelected(set<objects::CBioseq_Handle> &selected);
    void GetTopSeqAndSeqSubmit(objects::CSeq_entry_Handle &top_seq_entry, CConstRef<objects::CSeq_submit> &seq_submit);
    CRef<CObjectFor<objects::CValidError::TErrs> > GetErrors() {return m_errs;}
    TConstScopedObjects GetObjects() {return m_objects;}
    wxString GetWorkDir() {return m_WorkDir;}
    CRef<objects::CScope> GetScope() {return m_Scope;}
    void LoadData(void);
    void SetProjectHandleAndItem(objects::CGBProjectHandle* ph, objects::CProjectItem* pi) {m_ProjectHandle = ph; m_ProjectItem = pi;}
    
private:
    void UpdateList(const string &statusText);
    void DisplayList(void);
    bool x_MatchFilter(const objects::CValidErrItem& valitem) const;
    bool x_MatchSeverity(const objects::CValidErrItem& valitem) const;
    void OnDataChanged(CEvent* evt);
    void ConnectListener(void);
    void DisconnectListener(void);
    bool IsSelectable(const CSerialObject *obj);
    void LaunchEditor(CRef<objects::CValidErrItem> error);
    void GetObjectsFromProjectItem();
    IProjectView* FindGraphicalSequenceView(CRef<objects::CSeq_id> id);

    IWorkbench*     m_Workbench;
    CRef<CJobAdapter> m_JobAdapter;
    wxListCtrl* m_ListCtrl;
    CRef<objects::CScope> m_Scope;
    CIRef<IFlatFileCtrl> m_FlatFileCtrl;
    wxStaticText* m_SummaryText;
    wxStaticText* m_Modified;
    wxChoice* m_Severity;
    wxChoice* m_Filter;
    CRef<CObjectFor<objects::CValidError::TErrs> > m_errs;
    vector<size_t> m_visible;
    TConstScopedObjects m_objects;
    CRef<objects::CUser_object> m_params;
    int m_SortedColumn;
    bool m_Ascending;
    vector< vector<string> > m_item_text;
    wxImageList* m_imageListSmall;

    CRef<CSelectionClient> m_SelectionClient;
    bool m_SelectionClientAttached;
    bool m_IsBusy;
    string m_RegPath;

    wxString m_WorkDir;
    objects::CGBProjectHandle* m_ProjectHandle;
    objects::CProjectItem* m_ProjectItem;
};


END_NCBI_SCOPE

#endif
    // _VALIDATE_PANEL_H_
