/*  $Id: validate_panel.cpp 44126 2019-11-01 16:58:03Z filippov $
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


#include <ncbi_pch.hpp>
#include <objects/submit/Seq_submit.hpp>
#include <gui/core/selection_service_impl.hpp>
#include <gui/objutils/project_item_extra.hpp>
#include <gui/core/project_task.hpp>
#include <gui/objutils/objects.hpp>
#include <gui/widgets/edit/table_data_validate_job.hpp>
#include <gui/widgets/edit/table_data_validate_params.hpp>
#include <gui/widgets/data/report_dialog.hpp>
#include <gui/objutils/utils.hpp>
#include <gui/objutils/table_data_status.hpp>
#include <objmgr/util/sequence.hpp>
#include <gui/packages/pkg_sequence_edit/miscedit_util.hpp>
#include <gui/objutils/execute_lock_guard.hpp>
#include <gui/objects/GBWorkspace.hpp>
#include <gui/core/project_service.hpp>
#include <gui/core/simple_project_view.hpp>
#include <gui/widgets/edit/bioseq_editor.hpp>
#include <gui/packages/pkg_sequence_edit/segregate_sets.hpp>
#include <gui/packages/pkg_sequence_edit/sequester_sets.hpp>
#include <gui/widgets/edit/smart_save_to_client.hpp>
#include <gui/widgets/edit/sequence_editing_cmds.hpp>
#include <objtools/validator/validerror_format.hpp>
#include <objmgr/seqdesc_ci.hpp>
#include <gui/widgets/wx/wx_utils.hpp>
#include <gui/objutils/cmd_create_desc.hpp>
#include <gui/objutils/descriptor_change.hpp>
#include <gui/packages/pkg_sequence_edit/validate_panel.hpp>
#include <objects/seq/Pubdesc.hpp>
#include <objects/pub/Pub_equiv.hpp>
#include <objects/pub/Pub.hpp>
#include <gui/widgets/wx/fileartprov.hpp>
// for validator response commands
#include <gui/packages/pkg_sequence_edit/common_commands.hpp>
#include <gui/objutils/seqdesc_title_edit.hpp>
#include <gui/objutils/gap_edit.hpp>
#include <gui/objutils/primary_edit.hpp>
#include <gui/widgets/seq_graphic/seqgraphic_widget.hpp>
#include <gui/widgets/seq_graphic/graphic_panel.hpp>

#include "validator_report_cfg_dlg.hpp"

// For compilers that support precompilation, includes "wx/wx.h".
#include "wx/wxprec.h"

#ifdef __BORLANDC__
#pragma hdrstop
#endif

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

////@begin includes
#include "wx/imaglist.h"
#include <wx/display.h>
////@end includes

#include <wx/filename.h>

////@begin XPM images
////@end XPM images

BEGIN_NCBI_SCOPE

static const char * empty_xpm[] = {
"16 16 2 1",
"   c None",
"X  c Gray25",
"                ",
"                ",
"                ",
"                ",
"                ",
"                ",
"                ",
"                ",
"                ",
"                ",
"                ",
"                ",
"                ",
"                ",
"                ",
"                "};

static const char * up_xpm[] = {
"16 16 2 1",
"   c None",
"X  c Gray25",
"                ",
"                ",
"                ",
"                ",
"                ",
"                ",
"        X       ",
"       XXX      ",
"      XXXXX     ",
"     XXXXXXX    ",
"                ",
"                ",
"                ",
"                ",
"                ",
"                "};


static const char * down_xpm[] = {
"16 16 2 1",
"   c None",
"X  c Gray25",
"                ",
"                ",
"                ",
"                ",
"                ",
"                ",
"     XXXXXXX    ",
"      XXXXX     ",
"       XXX      ",
"        X       ",
"                ",
"                ",
"                ",
"                ",
"                ",
"                "};

/*
 * CValidatePanel type definition
 */

IMPLEMENT_CLASS( CValidatePanel, wxPanel )


/*
 * CValidatePanel event table definition
 */

BEGIN_EVENT_TABLE( CValidatePanel, wxPanel )

////@begin CValidatePanel event table entries
    EVT_LIST_ITEM_SELECTED(ID_VALIDATE_FRAME_TEXTCTRL, CValidatePanel::OnListItemSelected)
    EVT_LIST_ITEM_ACTIVATED(ID_VALIDATE_FRAME_TEXTCTRL, CValidatePanel::OnListItemActivated)
    EVT_CHOICE( ID_VALIDATE_FRAME_SEVERITY, CValidatePanel::OnValidatorSeveritySelected )
    EVT_CHOICE( ID_VALIDATE_FRAME_ERRCODE, CValidatePanel::OnErrcodeChoiceSelected )
    EVT_LIST_COL_CLICK(ID_VALIDATE_FRAME_TEXTCTRL, CValidatePanel::OnColumnClicked)
    EVT_LIST_ITEM_RIGHT_CLICK(ID_VALIDATE_FRAME_TEXTCTRL, CValidatePanel::OnListItemRightClick)
////@end CValidatePanel event table entries

    EVT_CHILD_FOCUS(CValidatePanel::OnChildFocus)

END_EVENT_TABLE()

BEGIN_EVENT_MAP( CValidatePanel, CEventHandler )
    ON_EVENT(CProjectViewEvent, CViewEvent::eProjectChanged, &CValidatePanel::OnDataChanged)
END_EVENT_MAP()

static int wxCALLBACK s_CompareFunction(wxIntPtr item1, wxIntPtr item2, wxIntPtr sortData)
{
    CValidatePanel *frame = (CValidatePanel*) sortData;
    bool asc = frame->GetAscending();
    string str1 = frame->GetItemText(item1);
    string str2 = frame->GetItemText(item2);
    if (str1 == str2)
        return 0;
    int r = str1 < str2 ? -1 : 1;
    if (!asc)
        r = -r;
    return r;  
}

/*
 * CValidatePanel constructors
 */

CValidatePanel::CValidatePanel()
    : m_Workbench(NULL)
{
    Init();
}

CValidatePanel::CValidatePanel( wxWindow* parent, IWorkbench*  wb, CRef<CUser_object> params, wxWindowID id, const wxPoint& pos, const wxSize& size, long style )
    : m_Workbench(wb), m_params(params), m_SelectionClient(new CSelectionClient("Validate Errors"))
{
    Init();
    Create( parent, id, pos, size, style );

    CIRef<CSelectionService> sel_srv = m_Workbench->GetServiceByType<CSelectionService>();
    if (sel_srv) sel_srv->AttachClient(m_SelectionClient);
    m_SelectionClientAttached = true;
}


/*
 * CValidatePanel creator
 */

bool CValidatePanel::Create( wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style )
{
////@begin CValidatePanel creation
    wxPanel::Create( parent, id, pos, size, style );

    CreateControls();    
    Centre();
////@end CValidatePanel creation
    return true;
}


/*
 * CValidatePanel destructor
 */

CValidatePanel::~CValidatePanel()
{
////@begin CValidatePanel destruction

////@end CValidatePanel destruction

    if (m_Workbench && m_SelectionClient && m_SelectionClientAttached) {
        CIRef<CSelectionService> sel_srv = m_Workbench->GetServiceByType<CSelectionService>();
        if (sel_srv) sel_srv->DetachClient(m_SelectionClient);
        m_SelectionClientAttached = false;
    }

    if (m_JobAdapter) m_JobAdapter->Cancel();

    DisconnectListener();
    SetCursor(wxCursor(wxNullCursor));
    wxSetCursor(wxNullCursor);
}

void CValidatePanel::OnChildFocus(wxChildFocusEvent& evt)
{
    if (m_Workbench && m_SelectionClient && m_SelectionClientAttached) {
        CIRef<CSelectionService> sel_srv = m_Workbench->GetServiceByType<CSelectionService>();
        if (sel_srv) sel_srv->OnActiveClientChanged(m_SelectionClient);
    }
}

/*
 * Member initialisation
 */

void CValidatePanel::Init()
{
////@begin CValidatePanel member initialisation
    m_ListCtrl = NULL;
    m_SortedColumn = 2;
    m_Ascending = true;
    m_IsBusy = false;
	m_ProjectHandle = nullptr;
	m_ProjectItem = nullptr;
    wxInitAllImageHandlers();
    static bool bitmap_registered = false;
    if (!bitmap_registered) 
    {
        wxFileArtProvider* provider = GetDefaultFileArtProvider();
        provider->RegisterFileAlias(wxT("validate::edit"), wxT("pen.png"));
        bitmap_registered = true;
    }

    m_imageListSmall = new wxImageList(16,16);

    m_imageListSmall->Add( wxBitmap( empty_xpm ) );
    m_imageListSmall->Add( wxBitmap( down_xpm ) );
    m_imageListSmall->Add( wxBitmap( up_xpm ) );
    m_imageListSmall->Add( wxArtProvider::GetBitmap(wxT("validate::edit")) );
////@end CValidatePanel member initialisation
}


/*
 * Control creation for CValidatePanel
 */

void CValidatePanel::CreateControls()
{    
////@begin CValidatePanel content construction
    // Generated by DialogBlocks, 18/10/2016 11:02:51 (unregistered)

    wxPanel* itemPanel2 = this;

    wxBoxSizer* itemBoxSizer3 = new wxBoxSizer(wxVERTICAL);
    itemPanel2->SetSizer(itemBoxSizer3);
   
    wxBoxSizer* itemBoxSizer4 = new wxBoxSizer(wxHORIZONTAL);
    itemBoxSizer3->Add(itemBoxSizer4, 0, wxALIGN_LEFT|wxALL, 5);

    wxStaticBox* itemStaticBoxSizer7Static = new wxStaticBox(itemPanel2, wxID_ANY, _("Status"));
    wxStaticBoxSizer* itemStaticBoxSizer7 = new wxStaticBoxSizer(itemStaticBoxSizer7Static, wxVERTICAL);
    itemBoxSizer4->Add(itemStaticBoxSizer7, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    m_SummaryText = new wxStaticText( itemPanel2, wxID_STATIC, wxEmptyString, wxDefaultPosition, wxSize(450, -1), 0 );
    itemStaticBoxSizer7->Add(m_SummaryText, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

    m_Modified = new wxStaticText( itemPanel2, wxID_STATIC, wxEmptyString, wxDefaultPosition, wxSize(450, -1), 0 );
    itemStaticBoxSizer7->Add(m_Modified, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

    wxStaticText* itemStaticText5 = new wxStaticText( itemPanel2, wxID_STATIC, _("Severity"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer4->Add(itemStaticText5, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxArrayString m_SeverityStrings;
    m_SeverityStrings.Add(_("INFO"));
    m_SeverityStrings.Add(_("WARN"));
    m_SeverityStrings.Add(_("ERROR"));
    m_SeverityStrings.Add(_("REJECT"));
    m_Severity = new wxChoice( itemPanel2, ID_VALIDATE_FRAME_SEVERITY, wxDefaultPosition, wxDefaultSize, m_SeverityStrings, 0 );
    m_Severity->SetStringSelection(_("INFO"));
    itemBoxSizer4->Add(m_Severity, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxStaticText* itemStaticText7 = new wxStaticText( itemPanel2, wxID_STATIC, _("Filter"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer4->Add(itemStaticText7, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxArrayString m_FilterStrings;
    m_FilterStrings.Add(_("ALL"));
    m_Filter = new wxChoice( itemPanel2, ID_VALIDATE_FRAME_ERRCODE, wxDefaultPosition, wxSize(200, -1), m_FilterStrings, 0 );
    m_Filter->SetStringSelection(_("ALL"));
    itemBoxSizer4->Add(m_Filter, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);


    m_ListCtrl = new wxListCtrl( itemPanel2, ID_VALIDATE_FRAME_TEXTCTRL, wxDefaultPosition, wxDefaultSize, wxLC_REPORT | wxLC_SINGLE_SEL | wxLC_HRULES | wxLC_VRULES );
    itemBoxSizer3->Add(m_ListCtrl, 1, wxGROW|wxALL, 5);
    //m_ListCtrl->EnableCheckboxes(true);
    m_ListCtrl->InsertColumn(0,_(" "),wxLIST_FORMAT_LEFT, 16);
    m_ListCtrl->InsertColumn(1,_("   Severity"),wxLIST_FORMAT_LEFT);
    m_ListCtrl->InsertColumn(2,_("Sequence"),wxLIST_FORMAT_LEFT);
    m_ListCtrl->InsertColumn(3,_("Error title"),wxLIST_FORMAT_LEFT);
    m_ListCtrl->InsertColumn(4,_("Message & Object Description"),wxLIST_FORMAT_LEFT, 750);
    m_ListCtrl->SetImageList(m_imageListSmall,wxIMAGE_LIST_SMALL);  
    if (!RunningInsideNCBI())
    {
        m_ListCtrl->Bind(wxEVT_MOTION, &CValidatePanel::OnMouseMove, this);
    }   

////@end CValidatePanel content construction
}


/*
 * Should we show tooltips?
 */

bool CValidatePanel::ShowToolTips()
{
    return true;
}

/*
 * Get bitmap resources
 */

wxBitmap CValidatePanel::GetBitmapResource( const wxString& name )
{
    // Bitmap retrieval
////@begin CValidatePanel bitmap retrieval
    wxUnusedVar(name);
    return wxNullBitmap;
////@end CValidatePanel bitmap retrieval
}

/*
 * Get icon resources
 */

wxIcon CValidatePanel::GetIconResource( const wxString& name )
{
    // Icon retrieval
////@begin CValidatePanel icon retrieval
    wxUnusedVar(name);
    return wxNullIcon;
////@end CValidatePanel icon retrieval
}

void CValidatePanel::ConnectListener()
{
    if (!m_Workbench || !m_Scope)
        return;

    CIRef<CProjectService> srv = m_Workbench->GetServiceByType<CProjectService>();
    if (!srv)
        return;
    CRef<CGBWorkspace> ws = srv->GetGBWorkspace();
    if (!ws)
        return;
    CGBDocument* doc = dynamic_cast<CGBDocument*>(ws->GetProjectFromScope(*m_Scope));        
    if (!doc)
        return;
    doc->AddListener(this);
}

void CValidatePanel::DisconnectListener()
{
    if (!m_Workbench || !m_Scope)
        return;
    
    CIRef<CProjectService> srv = m_Workbench->GetServiceByType<CProjectService>();
    if (!srv)
        return;
    CRef<CGBWorkspace> ws = srv->GetGBWorkspace();
    if (!ws)
        return;
    CGBDocument* doc = dynamic_cast<CGBDocument*>(ws->GetProjectFromScope(*m_Scope));        
    if (!doc)
        return;
    doc->RemoveListener(this);
}

void CValidatePanel::OnJobResult(CObject* result, CJobAdapter&)
{
    string statusText;
    m_errs.Reset();

    m_errs.Reset(dynamic_cast<CObjectFor<CValidError::TErrs>*>(result));
    if (!m_errs) {
        statusText = "Validation failed!";
    }
    else if (m_errs->GetData().size() == 0) {
        m_errs.Reset();
        statusText = "Validation complete, no errors found!";
    }

    UpdateList(statusText);
    ConnectListener();
    if (!m_SelectionClientAttached)
    {
        CIRef<CSelectionService> sel_srv = m_Workbench->GetServiceByType<CSelectionService>();
        if (sel_srv) sel_srv->AttachClient(m_SelectionClient);
        m_SelectionClientAttached = true;
    }

//#ifdef NCBI_OS_MSWIN
    if (m_IsBusy)
    {
        wxEndBusyCursor();
        m_IsBusy = false;
    }
//#endif
}

void CValidatePanel::OnJobFailed(const string& errMsg, CJobAdapter&)
{
    string statusText = "Failed: ";
    m_errs.Reset();

    if (!errMsg.empty())
        statusText += errMsg;
    else
        statusText += "Unknown fatal error";

    UpdateList(statusText);
    ConnectListener();
    if (!m_SelectionClientAttached)
    {
        CIRef<CSelectionService> sel_srv = m_Workbench->GetServiceByType<CSelectionService>();
        if (sel_srv) sel_srv->AttachClient(m_SelectionClient);
        m_SelectionClientAttached = true;
    }

//#ifdef NCBI_OS_MSWIN
    if (m_IsBusy)
    {
        wxEndBusyCursor();
        m_IsBusy = false;
    }
//#endif
}

void CValidatePanel::LoadData()
{
//#ifdef NCBI_OS_MSWIN
    if (!m_IsBusy)
    {
        wxBeginBusyCursor();
        m_IsBusy = true;
    }
//#endif
    if (m_Workbench && m_SelectionClient && m_SelectionClientAttached) 
    {
        CIRef<CSelectionService> sel_srv = m_Workbench->GetServiceByType<CSelectionService>();
        if (sel_srv) sel_srv->DetachClient(m_SelectionClient);
        TConstScopedObjects objs;
        m_SelectionClient->SetSelectedObjects(objs);
        m_SelectionClientAttached = false;
    }
    if (m_JobAdapter) m_JobAdapter->Cancel();

    m_errs.Reset();
    DisconnectListener();
    m_Modified->SetLabel(wxEmptyString);
  
    GetObjectsFromProjectItem();

    if (m_objects.empty()){
        UpdateList("No objects");
//#ifdef NCBI_OS_MSWIN
        if (m_IsBusy)
        {
            wxEndBusyCursor();
            m_IsBusy = false;
        }
//#endif
        return;
    }
    m_Scope = m_objects.front().scope;

    TConstScopedObjects main_objects;
    main_objects.push_back(m_objects.back());

    CIRef<CProjectService> prjSrv = m_Workbench->GetServiceByType<CProjectService>();
    CIRef<IProjectView> pTextView(prjSrv->FindView(*(m_objects.back().object), "Text View"));
    if (pTextView) 
    {
        m_FlatFileCtrl.Reset(dynamic_cast<IFlatFileCtrl*>(pTextView.GetPointerOrNull()));
    }
   

    string statusText = "Validating...";

    try {
        CIRef<CProjectService> srv = m_Workbench->GetServiceByType<CProjectService>();
        _ASSERT(srv);
        CRef<CGBWorkspace> ws = srv->GetGBWorkspace();
        CGBDocument* doc = 0;
        if (ws) 
            doc = dynamic_cast<CGBDocument*>(ws->GetProjectFromScope(*m_objects.front().scope));

        if (doc)
            m_WorkDir = doc->GetWorkDir();
   
        CRef<CTableDataValidateJob> job(new CTableDataValidateJob(main_objects, m_params.GetPointer()));
        if (doc) 
        {
            job->SetDataLocker(new CWeakExecuteGuard(doc->GetUndoManager()));
        }

        m_JobAdapter.Reset(new CJobAdapter(*this));
        m_JobAdapter->Start(*job);
    } catch( CAppJobException& e ){
        if (m_JobAdapter) m_JobAdapter->Cancel();
        statusText = "Validation failed!";
        LOG_POST( Error
            << "CTableDataValidate failed to start job: "
            << e.GetMsg()
        );
        LOG_POST(e.ReportAll());
    }

    if (!statusText.empty())
        UpdateList(statusText);
}

bool CValidatePanel::x_MatchSeverity(const objects::CValidErrItem& valitem) const
{
    EDiagSev sev = eDiag_Info;
    switch (m_Severity->GetSelection()) 
    {
    case 0:
        sev = eDiag_Info;
        break;
    case 1:
        sev = eDiag_Warning;
        break;
    case 2:
        sev = eDiag_Error;
        break;
    case 3:
        sev = eDiag_Critical;
        break;
    default:
        break;
    }

    return valitem.GetSeverity() >= sev;
}

bool CValidatePanel::x_MatchFilter(const objects::CValidErrItem& valitem) const
{
    string errcode = m_Filter->GetStringSelection().ToStdString();
    return errcode == "ALL" || NStr::EqualNocase(errcode, valitem.GetErrCode());
}

void  CValidatePanel::UpdateList(const string &statusText)
{
    if (!m_errs)
    {
        m_SummaryText->SetLabel(statusText);
        m_Filter->Clear();
        m_Filter->AppendString(wxT("ALL"));
        m_Filter->SetSelection(0);
        DisplayList();
        return;
    }
  
    set<string> err_codes;
    for (size_t row = 0; row < m_errs->GetData().size(); ++row)
    {
        CRef<CValidErrItem> error = m_errs->GetData()[row];
        if (!error)
            continue;
        err_codes.insert(error->GetErrCode());       
    }    
  
    m_Filter->Clear();
    m_Filter->AppendString(wxT("ALL"));   
    ITERATE(set<string>, it, err_codes) 
    {
        m_Filter->AppendString(ToWxString(*it));
    }
    m_Filter->SetSelection(0);
    DisplayList();
}

void  CValidatePanel::DisplayList()
{
    m_ListCtrl->DeleteAllItems();
    m_visible.clear();
    m_item_text.clear();
  
    if (!m_errs)
    {  
        return;
    }
    CSeq_entry_Handle seh;
    CScope::TTSE_Handles tses;
    m_Scope->GetAllTSEs(tses, CScope::eAllTSEs);
    if (!tses.empty())
        seh = tses.front();
    m_item_text.resize(5);
    map<EDiagSev, size_t> err_counts;
    for (size_t row = 0; row < m_errs->GetData().size(); ++row)
    {
        CRef<CValidErrItem> error = m_errs->GetData()[row];
        if (!error)
            continue;
        if (!x_MatchFilter(*error))
            continue;
        err_counts[error->GetSeverity()]++;
        if (!x_MatchSeverity(*error))
            continue;
        string severity = CValidErrItem::ConvertSeverity(error->GetSeverity());
        if (error->GetSeverity() == eDiag_Critical)
            severity = "Reject"; // per GB-6508
        string accession = error->CanGetAccnver() ? error->GetAccnver() : kEmptyStr;
        string title = (error->CanGetErrorGroup() ? error->GetErrorGroup() + " " : kEmptyStr) + (error->CanGetErrorName() ? error->GetErrorName() : kEmptyStr);
        string msg = error->CanGetMsg() ? error->GetMsg() + "; " : kEmptyStr;
        msg += error->CanGetObjDesc() ? error->GetObjDesc() : kEmptyStr;
        long item = m_ListCtrl->GetItemCount();

        const CSerialObject&  obj = error->GetObject();   
        unsigned int err_code = error->GetErrIndex();

        if (!RunningInsideNCBI() && valedit::IsEditable(err_code, obj, seh))
        {
            item = m_ListCtrl->InsertItem(item, 3);
        }
        else
        {
            item = m_ListCtrl->InsertItem(item, 0);
        }
        m_ListCtrl->SetItem(item, 1, wxString(severity));
        m_ListCtrl->SetItem(item, 2, wxString(accession));
        m_ListCtrl->SetItem(item, 3, wxString(title));
        m_ListCtrl->SetItem(item, 4, wxString(msg));
        m_ListCtrl->SetItemData(item, item);

        if (!RunningInsideNCBI() && IsSelectable(&obj))
        {
            m_ListCtrl->SetItemTextColour(item, *wxBLUE);
            wxFont font = m_ListCtrl->GetItemFont(item);
            font.SetUnderlined(true);
            m_ListCtrl->SetItemFont(item, font);
        }
        
        m_visible.push_back(row);
        m_item_text[0].push_back(kEmptyStr);
        m_item_text[1].push_back(severity);
        m_item_text[2].push_back(accession);
        m_item_text[3].push_back(title);
        m_item_text[4].push_back(msg);
    }    
    if (m_ListCtrl->GetItemCount() > 0)
    {
        m_ListCtrl->SetColumnWidth(0, wxLIST_AUTOSIZE_USEHEADER);
        m_ListCtrl->SetColumnWidth(1, wxLIST_AUTOSIZE_USEHEADER);
        m_ListCtrl->SetColumnWidth(2, wxLIST_AUTOSIZE);
        m_ListCtrl->SetColumnWidth(3, wxLIST_AUTOSIZE);
        m_ListCtrl->SetColumnWidth(4, wxLIST_AUTOSIZE);   
    }
    else
    {
        m_ListCtrl->SetColumnWidth(0, wxLIST_AUTOSIZE_USEHEADER);
        m_ListCtrl->SetColumnWidth(1, wxLIST_AUTOSIZE_USEHEADER);
        m_ListCtrl->SetColumnWidth(2, wxLIST_AUTOSIZE_USEHEADER);
        m_ListCtrl->SetColumnWidth(3, wxLIST_AUTOSIZE_USEHEADER);
        m_ListCtrl->SetColumnWidth(4, wxLIST_AUTOSIZE_USEHEADER);   
    }
    string msg = "INFO: " + NStr::NumericToString(err_counts[eDiag_Info]) + "   " + "WARNING: " + NStr::NumericToString(err_counts[eDiag_Warning]) +
        "   " + "ERROR: " + NStr::NumericToString(err_counts[eDiag_Error]) + "   " + "REJECT: " + NStr::NumericToString(err_counts[eDiag_Critical]);
    m_SummaryText->SetLabel(ToWxString(msg));

    if (m_SortedColumn > 0)
    {
        wxListItem item;
        item.SetMask(wxLIST_MASK_IMAGE);
        item.SetImage(m_Ascending ? 1 : 2);       
        m_ListCtrl->SetColumn(m_SortedColumn, item);
        m_ListCtrl->SortItems(s_CompareFunction, (wxIntPtr)this);
    }
}

bool CValidatePanel::IsSelectable(const CSerialObject *obj)
{
    const CSeqdesc *seqdesc = dynamic_cast<const CSeqdesc *>(obj);
    if (seqdesc)
        return true;
    const CSeq_feat *seqfeat = dynamic_cast<const CSeq_feat *>(obj);
    if (seqfeat)
        return true;
    const CGapEdit *gap = dynamic_cast<const CGapEdit *>(obj);
    if (gap)
        return true;
    const CPrimaryEdit *primary = dynamic_cast<const CPrimaryEdit *>(obj);
    if (primary)
        return true;
    const CSeqdescTitleEdit *defline = dynamic_cast<const CSeqdescTitleEdit *>(obj);
    if (defline)
        return true;
    return false;
}

void CValidatePanel::OnValidatorSeveritySelected( wxCommandEvent& event )
{
    DisplayList();
}

void CValidatePanel::OnErrcodeChoiceSelected( wxCommandEvent& event )
{
    DisplayList();
}

IProjectView* CValidatePanel::FindGraphicalSequenceView(CRef<CSeq_id> id)
{ 
    if (!id)
        return nullptr;

    CIRef<CProjectService> prjSrv = m_Workbench->GetServiceByType<CProjectService>();

    CRef<CGBWorkspace> ws = prjSrv->GetGBWorkspace();
    if (!ws)
        return nullptr;

    CGBDocument* doc = dynamic_cast<CGBDocument*>(ws->GetProjectFromScope(*m_Scope));        
    if (!doc)
        return nullptr;

    CIRef<IProjectView> prj_view;
    const vector< CIRef<IProjectView> >& views =  doc->GetViews();

    for (const auto & view : views)
    {
        if (view->GetLabel(IProjectView::eType) == "Graphical Sequence View")
        {
            prj_view = view;
            break;
        }
    }
    if (!prj_view)
        return nullptr;

    TConstScopedObjects objects;
    prj_view->GetMainObject(objects);
    CConstRef<CSeq_id> other_id(dynamic_cast<const CSeq_id*>(objects.front().object.GetPointer()));
    if (!other_id || !sequence::IsSameBioseq(*id, *other_id, m_Scope))
    {
        SConstScopedObject obj(ConstRef(id.GetPointer()), m_Scope);
        prj_view->SetOrigObject(obj);
    }
    return prj_view.GetPointer();
}

void CValidatePanel::OnListItemSelected(wxListEvent& event)
{
    long index = m_ListCtrl->GetItemData(event.GetIndex());
    if (index >= m_visible.size() || !m_errs)
        return;   
    CRef<CValidErrItem> error = m_errs->GetData()[m_visible[index]];
    const CSerialObject&  obj = error->GetObject();   
    wxPoint p = m_ListCtrl->ScreenToClient(wxGetMousePosition());
    if (p.x < m_ListCtrl->GetColumnWidth(0) && !RunningInsideNCBI())
    {
        CSeq_entry_Handle seh;
        CScope::TTSE_Handles tses;
        m_Scope->GetAllTSEs(tses, CScope::eAllTSEs);
        if (!tses.empty())
            seh = tses.front();
        unsigned int err_code = error->GetErrIndex();
        if (valedit::IsEditable(err_code, obj, seh))
        {
            OnListItemActivated(event);
            return;
        }
    }

    string accession = error->GetAccession();
    CBioseq_Handle bsh;
    CRef<CSeq_id> id;
    try
    {
        id.Reset(new CSeq_id(accession, CSeq_id::fParse_Default));
        bsh = m_Scope->GetBioseqHandle(*id);
    }
    catch (CException& ex) {}

    if (m_FlatFileCtrl && bsh)
    {        
        try
        {
            m_FlatFileCtrl->SetPosition(bsh, &obj);
        }
        catch (CException& ex) {}
    }

    IProjectView* gs_view = FindGraphicalSequenceView(id);
    if (gs_view) 
    {
        CGraphicPanel* panel = dynamic_cast<CGraphicPanel*>(gs_view->GetWindow());
        if (panel)
        {
            CSeqGraphicWidget* widget = panel->GetWidget();
            if (widget && bsh && bsh == widget->GetCurrentBioseq())
            {
                const CSeq_feat* feat = dynamic_cast<const CSeq_feat*>(&obj);
                if (feat && feat->IsSetLocation())
                    widget->ZoomOnRange(feat->GetLocation().GetTotalRange(), CSeqGraphicWidget::fAddMargins);
            }
        }
    }

    if (m_SelectionClient && m_SelectionClientAttached) 
    {
        TConstScopedObjects objs;
        objs.push_back(SConstScopedObject(&obj, m_Scope));
        m_SelectionClient->SetSelectedObjects(objs);
    }
}


void CValidatePanel::OnListItemActivated(wxListEvent& event)
{
    long index = m_ListCtrl->GetItemData(event.GetIndex());
    if (index >= m_visible.size() || !m_errs)
        return;   

    CRef<CValidErrItem> error = m_errs->GetData()[m_visible[index]];
    CallAfter(&CValidatePanel::LaunchEditor, error);
}

void CValidatePanel::LaunchEditor(CRef<CValidErrItem> error)
{
    const CSerialObject&  obj = error->GetObject();
    unsigned int err_code = error->GetErrIndex();

    const CBioseq *bioseq = dynamic_cast<const CBioseq *>(&obj);
    const CBioseq_set* bioseq_set = dynamic_cast<const CBioseq_set *>(&obj);
    const CSeq_feat* f = dynamic_cast<const CSeq_feat *>(&obj);
    const CSeqdesc* d = dynamic_cast<const CSeqdesc *>(&obj);
    const CSeq_align *align = dynamic_cast<const CSeq_align *>(&obj);

    CSeq_entry_Handle seh;

    if (bioseq) {
        CBioseq_Handle bsh = m_Scope->GetBioseqHandle(*bioseq);
        seh = bsh.GetParentEntry();
    }
    if (bioseq_set) {
        CBioseq_set_Handle bsh = m_Scope->GetBioseq_setHandle(*bioseq_set);
        seh = bsh.GetParentEntry();
    }
    if (f && f->IsSetLocation())
    {
        CBioseq_Handle bsh = m_Scope->GetBioseqHandle(f->GetLocation());
        if (bsh)
            seh = bsh.GetSeq_entry_Handle();
    }
    if (d) {
        seh = edit::GetSeqEntryForSeqdesc(m_Scope, *d);
    }
    if (align) {
        int num_rows = align->CheckNumRows();
        for (size_t row = 0; row < num_rows; row++)
        {
            const CSeq_id&  id = align->GetSeq_id(row);
            CBioseq_Handle bsh = m_Scope->GetBioseqHandle(id);
            if (bsh)
            {
                seh = bsh.GetTopLevelEntry();
                if (seh)
                    break;
            }
        }
    }

    if (!seh)
    {
        CScope::TTSE_Handles tses;
        m_Scope->GetAllTSEs(tses, CScope::eAllTSEs);
        if (!tses.empty())
            seh = tses.front();
    }

    if (seh) {
        valedit::LaunchEditor(err_code, obj, seh, m_Workbench);
    }
}

void CValidatePanel::OnListItemRightClick(wxListEvent& event)
{
    if (!RunningInsideNCBI())
        return;

    long index = m_ListCtrl->GetItemData(event.GetIndex());
    if (index >= m_visible.size() || !m_errs)
        return;   

    CSeq_entry_Handle seh;
    CRef<CValidErrItem> error = m_errs->GetData()[m_visible[index]];
    CValidErrItem::TErrIndex err_index = error->GetErrIndex();
    string description = error->GetErrCode();
    CScope::TTSE_Handles tses;
    m_Scope->GetAllTSEs(tses, CScope::eAllTSEs);
    if (!tses.empty())
        seh = tses.front();
   
    if (!seh)
        return;

    wxString msg;
    msg << "Suppress " << description << "?";
    int answer = wxMessageBox(msg, wxT("Supress validator test"), wxYES_NO | wxICON_QUESTION, this);
    if (answer != wxYES)
        return;


    CIRef<IEditCommand> cmd;
    CRef<CSeqdesc> suppress(new CSeqdesc());
    for (CSeqdesc_CI desc_it(seh, CSeqdesc::e_User); desc_it; ++desc_it)
    {
        if (desc_it->GetUser().GetObjectType() == CUser_object::eObjectType_ValidationSuppression) 
        {
            suppress->Assign(*desc_it);
            validator::CValidErrorFormat::AddSuppression(suppress->SetUser(), err_index);
            cmd.Reset(new CCmdChangeSeqdesc(seh, *desc_it, *suppress));
            break;
        }
    }

    if (!cmd)
    {
        suppress->SetUser().SetObjectType(CUser_object::eObjectType_ValidationSuppression);
        validator::CValidErrorFormat::AddSuppression(suppress->SetUser(), err_index);
        cmd.Reset(new CCmdCreateDesc(seh, *suppress));
    }


    CIRef<CProjectService> srv = m_Workbench->GetServiceByType<CProjectService>();
    if (!srv)
        return;
    
    CRef<CGBWorkspace> ws = srv->GetGBWorkspace();
    if (!ws) 
        return;
    
    CGBDocument* doc = dynamic_cast<CGBDocument*>(ws->GetProjectFromScope(*m_Scope));
    if (!doc)
        return;
    ICommandProccessor* cmdProcessor = &doc->GetUndoManager();   
    cmdProcessor->Execute(cmd);   
}

void CValidatePanel::GetSelected(set<CBioseq_Handle> &selected)
{
    if (m_visible.empty() || !m_errs)
        return;
   
    for ( long item = 0; item < m_ListCtrl->GetItemCount(); ++item)
    {
        //if (!m_ListCtrl->IsItemChecked(item))
        //    continue;
        CRef<CValidErrItem> error = m_errs->GetData()[m_visible[item]];
        if (error) 
        {
            string accession = error->GetAccession();

            CSeq_id acc_id(accession, CSeq_id::fParse_Default);

            CBioseq_Handle bsh;
            try
            {
                bsh = m_Scope->GetBioseqHandle(acc_id);
            }
            catch(exception &)
            {}
            if (bsh)
                selected.insert(bsh);
        }
    }  
}

void CValidatePanel::GetTopSeqAndSeqSubmit(CSeq_entry_Handle &top_seq_entry, CConstRef<objects::CSeq_submit> &seq_submit)
{   
    if (m_objects.empty()) return;
    NON_CONST_ITERATE (TConstScopedObjects, it, m_objects) {
        const CSeq_submit* sub = dynamic_cast<const CSeq_submit*>((*it).object.GetPointer());
        if (sub) {
            seq_submit.Reset(sub);
        }
        if (!top_seq_entry) {
            CSeq_entry_Handle seh;
            try
            {
                seh = GetTopSeqEntryFromScopedObject(*it);
            } catch(exception &) {}
            if (seh) {
                top_seq_entry = seh;
                if (seq_submit) {
                    break;
                }
            }
        }
    }
}

string CValidatePanel::GetItemText(long item)
{
    if (m_SortedColumn <= 0)
        return kEmptyStr;
    return m_item_text[m_SortedColumn][item];
}

bool CValidatePanel::GetAscending()
{
    return m_Ascending;
}

void CValidatePanel::OnColumnClicked(wxListEvent& event)
{
    if (!m_ListCtrl)
        return;
    int col = event.GetColumn();
    if (col <= 0)
        return;
    if (m_SortedColumn == col)
    {
        m_Ascending = !m_Ascending;
    }
    else
    {
        if (m_SortedColumn > 0)
        {
            wxListItem item;
            item.SetMask(wxLIST_MASK_IMAGE);
            item.SetImage(0);       
            m_ListCtrl->SetColumn(m_SortedColumn, item);
        }
        m_SortedColumn = col;
        m_Ascending = true;
    }

    // http://www.codeprogress.com/cpp/libraries/wxwidgets/showWxExample.php?key=wxListCtrlColumnHeaderTextIcon&index=54
    wxListItem item;
    item.SetMask(wxLIST_MASK_IMAGE);
    item.SetImage(m_Ascending ? 1 : 2);       
    m_ListCtrl->SetColumn(m_SortedColumn, item);

    m_ListCtrl->SortItems(s_CompareFunction, (wxIntPtr)this);
}


void CValidatePanel::OnDataChanged(CEvent* evt)
{
    if (!evt)
        return;
    CProjectViewEvent *prj_evt = dynamic_cast<CProjectViewEvent*>(evt);
    if (!prj_evt)
        return;
    switch (prj_evt->GetSubtype()) 
    {
        case CProjectViewEvent::eData:
            m_Modified->SetLabel(_("Data has been modified, please refresh"));
            Refresh();
            break;
        case CProjectViewEvent::eUnloadProject:
            //LoadData();
            m_Modified->SetLabel(_("Current project has been removed."));
            Refresh();
            break;
        default:
           break;
    } 
}

void CValidatePanel::OnMouseMove( wxMouseEvent &event )
{
    wxPoint p = event.GetPosition();
    if (m_ListCtrl->GetItemCount() < 1)
    {       
        event.Skip();
        return;
    }
    wxRect rect0;
    m_ListCtrl->GetItemRect(0, rect0);
    long item = p.y / rect0.GetHeight();
#if defined(__WXMSW__) 
    item--;
#endif
    item += m_ListCtrl->GetTopItem();

    if (item < 0 || item >= m_ListCtrl->GetItemCount())
    {
        SetCursor(wxCursor(wxNullCursor));
        wxSetCursor(wxNullCursor);
        event.Skip();
        return;
    }
    long index = m_ListCtrl->GetItemData(item);

    if (index >= m_visible.size() || !m_errs)
    {
        SetCursor(wxCursor(wxNullCursor));
        wxSetCursor(wxNullCursor);
        event.Skip();
        return;
    }
   
    CRef<CValidErrItem> error = m_errs->GetData()[m_visible[index]];
    const CSerialObject&  obj = error->GetObject();   

    bool hand_cursor = false;
    if (!RunningInsideNCBI())
    {
        if (p.x < m_ListCtrl->GetColumnWidth(0))
        {
            CSeq_entry_Handle seh;
            CScope::TTSE_Handles tses;
            m_Scope->GetAllTSEs(tses, CScope::eAllTSEs);
            if (!tses.empty())
                seh = tses.front();
            unsigned int err_code = error->GetErrIndex();
            if (valedit::IsEditable(err_code, obj, seh))
            {
                hand_cursor = true;
            }
        }
        else if ( IsSelectable(&obj) )
        {
            hand_cursor = true;
        }
    }
    if (hand_cursor)
    {
        SetCursor(wxCursor(wxCURSOR_HAND));
        wxSetCursor(wxCursor(wxCURSOR_HAND));
    }
    else 
    {
        SetCursor(wxCursor(wxNullCursor));
        wxSetCursor(wxNullCursor);
    }

    event.Skip();
}

void CValidatePanel::GetObjectsFromProjectItem()
{
    m_objects.clear();
    if (m_ProjectItem && m_ProjectHandle)
    {
        CConstRef<CSeq_submit> submit;
        CRef<CScope> scope(m_ProjectHandle->GetScope());

        if (m_ProjectItem->IsSetItem() && m_ProjectItem->GetItem().IsSubmit())
            submit.Reset(&m_ProjectItem->GetItem().GetSubmit());


        if (submit)
        {
            if (submit->IsSetData() && submit->GetData().IsEntrys())
            {
                for (auto entry : submit->GetData().GetEntrys())
                {
                    if (entry)
                    {
                        m_objects.push_back(SConstScopedObject(entry, scope));
                    }
                }        
            }   
            m_objects.push_back(SConstScopedObject(submit, scope));
        }
        else if (m_ProjectItem->IsSetItem() && m_ProjectItem->GetItem().IsEntry())
        {
            m_objects.push_back(SConstScopedObject(&m_ProjectItem->GetItem().GetEntry(), scope));
        }
    }
    else
    {
        CIRef<CSelectionService> sel_srv = m_Workbench->GetServiceByType<CSelectionService>();
        if (!sel_srv) 
            return;
        sel_srv->GetActiveObjects(m_objects);
        if (m_objects.empty()) 
        {
            GetViewObjects(m_Workbench, m_objects);
        }
    }
}

END_NCBI_SCOPE

