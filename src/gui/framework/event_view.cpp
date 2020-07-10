/*  $Id: event_view.cpp 43619 2019-08-09 16:32:36Z katargir $
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
 *  and reliability of the software and data, the NLM and the U.S.
 *  Government do not and cannot warrant the performance or results that
 *  may be obtained by using this software or data. The NLM and the U.S.
 *  Government disclaim all warranties, express or implied, including
 *  warranties of performance, merchantability or fitness for any particular
 *  purpose.
 *
 *  Please cite the author in any work or product based on this material.
 *
 * ===========================================================================
 *
 * Authors:  Andrey Yazhuk
 *
 * File Description:
 *
 */

#include <ncbi_pch.hpp>


#include <gui/framework/event_view.hpp>

#include <gui/framework/view_manager_service.hpp> //TODO
#include <gui/framework/event_record_details_dlg.hpp>
#include <gui/framework/workbench.hpp>

#include <gui/widgets/wx/wx_utils.hpp>
#include <gui/widgets/wx/fileartprov.hpp>
#include <gui/widgets/data/object_contrib.hpp>

#include <gui/objutils/registry.hpp>

#include <gui/utils/extension_impl.hpp>

#include <wx/sizer.h>
#include <wx/panel.h>
#include <wx/menu.h>
#include <wx/toolbar.h>
#include <wx/statline.h>
#include <wx/button.h>
#include <wx/checkbox.h>


BEGIN_NCBI_SCOPE

bool CEventTablePanel::m_IconsInitialized = false;

/// register the factory
static CExtensionDeclaration
    decl("view_manager_service::view_factory", new CEventViewFactory());


///////////////////////////////////////////////////////////////////////////////
/// CEventView

CEventViewModel::CEventViewModel(IEventLogService* service)
:   m_Service(service)
{
    m_Names[eTitle] = "Title";
    m_Names[eType] = "Type";
    m_Names[eDescription] = "Description";
    m_Names[eTime] = "Time";

    m_TimeFormat.SetFormat("b D, H:m:s p");

    m_Filter.resize(IEventRecord::eLast, true);
}


int CEventViewModel::GetNumRows() const
{
    return (int)m_Records.size();
}


int CEventViewModel::GetNumColumns() const
{
    return eMaxCols;
}


static string sTypeToString(IEventRecord::EType type)
{
    static string sInfo = "Information";
    static string sError = "Error";
    static string sWarning = "Warning";
    static string sInvalid = "Invalid Type";

    switch(type)    {
    case IEventRecord::eInfo:
        return sInfo;
    case IEventRecord::eError:
        return sError;
    case IEventRecord::eWarning:
        return sWarning;
    default:
        return sInvalid;
    }
}


wxVariant CEventViewModel::GetValueAt(int row, int col) const
{
    const IEventRecord* rec = m_Records[row];

    switch (col) {
    case eTitle:
        return wxString::FromUTF8( rec->GetTitle().c_str() );

    case eType: {{
        IEventRecord::EType type = rec->GetType();
        return wxString::FromUTF8( sTypeToString(type).c_str() );
    }}
    case eDescription: {{
        string s = rec->GetDescription();
        return wxString::FromUTF8( s.c_str() );
    }}
    case eTime: {{
        //TODO format properly
        CTime ctime(rec->GetTime(), CTime::eMinute);
        string s = ctime.ToLocalTime().AsString(m_TimeFormat);
        return wxString::FromUTF8( s.c_str() );
    }}
    default:
        _ASSERT(false);
        return wxEmptyString;
    }
}


wxString CEventViewModel::GetColumnType(int col) const
{
    return wxT("string");
}


wxString CEventViewModel::GetColumnName(int col) const
{
    return ToWxString(m_Names[col]);
}


void CEventViewModel::ShowType(IEventRecord::EType type, bool show, bool up)
{
    m_Filter[type] = show;
    if(up)  {
        UpdateTable();
    }
}


bool CEventViewModel::IsTypeShown(IEventRecord::EType type)
{
    return m_Filter[type];
}


void CEventViewModel::UpdateTable()
{
    m_Records.clear();

    if(m_Service)   {
        size_t n = m_Service->GetRecordsCount();
        for( size_t i = 0;  i < n;  i++ )   {
            const IEventRecord* rec = m_Service->GetRecord(i);
            int type = rec->GetType();

            _ASSERT(type >= 0  &&  type < (int) m_Filter.size());

            if(m_Filter[type])  {
                m_Records.push_back(TCRefRec(rec));
            }
        }
    }
    x_FireDataChanged();
}

///////////////////////////////////////////////////////////////////////////////
/// CEventTablePanel
#define ID_TABLE    10001
#define ID_TOOLBAR  10002


BEGIN_EVENT_TABLE(CEventTablePanel, wxPanel)
    EVT_CONTEXT_MENU(CEventTablePanel::OnContextMenu)

    EVT_MENU(eCmdShowErrors, CEventTablePanel::OnShowErrors)
    EVT_MENU(eCmdShowWarnings, CEventTablePanel::OnShowWarnings)
    EVT_MENU(eCmdShowInfo, CEventTablePanel::OnShowInfo)

    // Needed for context menu AND (if not mac) toolbar    
    EVT_MENU(eCmdDetails, CEventTablePanel::OnDetails)

    EVT_UPDATE_UI(eCmdDetails, CEventTablePanel::OnUpdateDetails)
    EVT_UPDATE_UI(eCmdShowErrors, CEventTablePanel::OnUpdateShowErrors)
    EVT_UPDATE_UI(eCmdShowWarnings, CEventTablePanel::OnUpdateShowWarnings)
    EVT_UPDATE_UI(eCmdShowInfo, CEventTablePanel::OnUpdateShowInfo)
    
    EVT_LIST_ITEM_ACTIVATED(ID_TABLE, CEventTablePanel::OnItemActivated)
END_EVENT_TABLE()


CEventTablePanel::CEventTablePanel()
:   wxPanel()
{
    Init();

}


CEventTablePanel::~CEventTablePanel()
{
}


void CEventTablePanel::Create(wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size)
{
#ifdef __WXOSX_COCOA__ // GB-8581
    SetBackgroundStyle(wxBG_STYLE_COLOUR);
    SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_FRAMEBK));
#endif

    wxPanel::Create(parent, id, pos, size);
    CreateControls();
}


void CEventTablePanel::Init()
{
    m_Service = NULL;
    m_Model = NULL;
    m_Table = NULL;

    if( !m_IconsInitialized ){
        wxFileArtProvider* provider = GetDefaultFileArtProvider();

        provider->RegisterFileAlias(wxT("menu::error16x16"), wxT("error16x16.xpm"));
        provider->RegisterFileAlias(wxT("menu::warning16x16"),  wxT("warning16x16.xpm"));
        provider->RegisterFileAlias(wxT("menu::info16x16"), wxT("info16x16.xpm"));
        provider->RegisterFileAlias(wxT("menu::htmpage16x16"), wxT("htmpage16x16.xpm"));
        
        m_IconsInitialized = true;
    }
}


void CEventTablePanel::CreateControls()
{
    wxSizer* sizer = new wxBoxSizer(wxVERTICAL);
    SetSizer(sizer);

    // Create tool bar
    long style = wxTB_FLAT | wxTB_HORIZONTAL | wxTB_TEXT | wxTB_HORZ_LAYOUT;
    wxToolBar* tool_bar = new wxToolBar(this, ID_TOOLBAR, wxDefaultPosition, wxDefaultSize, style);

    wxBitmap bmp_error = wxArtProvider::GetBitmap(wxT("menu::error16x16"));
    wxBitmap bmp_warning = wxArtProvider::GetBitmap(wxT("menu::warning16x16"));
    wxBitmap bmp_info = wxArtProvider::GetBitmap(wxT("menu::info16x16"));
    wxBitmap bmp_details = wxArtProvider::GetBitmap(wxT("menu::htmpage16x16"));
    
    tool_bar->AddCheckTool(eCmdShowErrors, wxT("Errors"), bmp_error, wxNullBitmap, wxT("Show error messages"));
    tool_bar->AddCheckTool(eCmdShowWarnings, wxT("Warnings"), bmp_warning, wxNullBitmap, wxT("Show warning messages"));
    tool_bar->AddCheckTool(eCmdShowInfo, wxT("Info"), bmp_info, wxNullBitmap,  wxT("Show information messages"));  
    tool_bar->AddSeparator();
    tool_bar->AddTool(eCmdDetails, wxT("Details"), bmp_details, wxT("Show event details")); 

    tool_bar->Realize();
    sizer->Add(tool_bar, 0, wxEXPAND);

    // separation line before the table
    wxStaticLine* line = new wxStaticLine(this, wxID_STATIC, wxDefaultPosition, wxDefaultSize, wxLI_HORIZONTAL );
    sizer->Add(line, 0, wxEXPAND);

    // Create Table
    style = wxLC_REPORT | wxLC_VIRTUAL | wxBORDER_NONE;
    m_Table = new CwxTableListCtrl(this, ID_TABLE, wxDefaultPosition, wxDefaultSize, style);
    m_Table->SetSingleStyle(wxLC_SINGLE_SEL);

    sizer->Add(m_Table, 1, wxEXPAND);
}


void CEventTablePanel::CreateModel(IEventLogService* service)
{
    m_Service = service;

    m_Model = new CEventViewModel(service);
    m_Table->SetModel(m_Model);

    m_Model->UpdateTable();

    m_Table->SetColumnWidth(CEventViewModel::eTitle, 400);
    m_Table->SetColumnWidth(CEventViewModel::eDescription, 200);
    m_Table->SetColumnWidth(CEventViewModel::eTime, 120);
    m_Table->SetColumnWidth(CEventViewModel::eType, 90);
}


void CEventTablePanel::DestroyModel()
{
    m_Table->RemoveModel();

    delete m_Model;
    m_Model = NULL;

    m_Service = NULL;
}


void CEventTablePanel::UpdateTable()
{
    m_Model->UpdateTable();
}


void CEventTablePanel::SetRegistryPath(const string& path)
{
    m_RegPath = path; // store for later use
}


//static const char* kDisplayModeTag = "DisplayMode";
static const char* kShowErrorsTag = "Show Errors";
static const char* kShowWarningsTag = "Show Warnings";
static const char* kShowInfoTag = "Show Info";

void CEventTablePanel::SaveSettings() const
{
    if( ! m_RegPath.empty())   {
        CGuiRegistry& gui_reg = CGuiRegistry::GetInstance();
        CRegistryWriteView view = gui_reg.GetWriteView(m_RegPath);

        _ASSERT(m_Table  &&  m_Model);

        view.Set(kShowErrorsTag, m_Model->IsTypeShown(IEventRecord::eError));
        view.Set(kShowWarningsTag, m_Model->IsTypeShown(IEventRecord::eWarning));
        view.Set(kShowInfoTag, m_Model->IsTypeShown(IEventRecord::eInfo));

        m_Table->SaveTableSettings(view);
    }
}


void CEventTablePanel::LoadSettings()
{
    if( ! m_RegPath.empty())   {
        CGuiRegistry& gui_reg = CGuiRegistry::GetInstance();
        CRegistryReadView view = gui_reg.GetReadView(m_RegPath);

        _ASSERT(m_Table  &&  m_Model);

        m_Model->ShowType(IEventRecord::eError, view.GetBool(kShowErrorsTag, true));
        m_Model->ShowType(IEventRecord::eWarning, view.GetBool(kShowWarningsTag, true));
        m_Model->ShowType(IEventRecord::eInfo, view.GetBool(kShowInfoTag, true));

        m_Table->LoadTableSettings(view);

        m_Model->UpdateTable();
    }
}


void CEventTablePanel::AppendMenuItems( wxMenu& aMenu )
{
    aMenu.Append( wxID_SEPARATOR, wxT("Settings") );
    aMenu.Append( eCmdDetails, wxT("Details") );
}

void CEventTablePanel::OnContextMenu( wxContextMenuEvent& anEvent )
{
    wxMenu* menu;

    void* data = anEvent.GetClientData();
    menu = data ? (wxMenu*)data : new wxMenu();

    AppendMenuItems( *menu );

    if( /*ShouldPropagate()*/ false ){
        anEvent.SetClientData( menu );
        anEvent.Skip();
        return;
    }

    anEvent.SetClientData( NULL );


    auto_ptr<wxMenu> main_menu( CreateContextMenuBackbone() );
    Merge( *main_menu.get(), *menu );

    CleanupSeparators( *main_menu );
    PopupMenu( main_menu.get() );

    delete menu;
}


void CEventTablePanel::OnDetails(wxCommandEvent& event)
{
    _ASSERT(m_Table  &&  m_Service);

    long sel_index = m_Table->GetFirstSelected();

    _ASSERT(sel_index != -1);

    OnShowDetailsDialog(sel_index);
}


void CEventTablePanel::OnItemActivated(wxListEvent &event)
{
    int index = event.GetIndex();
    OnShowDetailsDialog(index);
}


void CEventTablePanel::OnShowDetailsDialog(int vis_index)
{
    int data_index = m_Table->RowVisibleToData(vis_index);
    const IEventRecord* record = m_Service->GetRecord(data_index);

    CEventRecordDetailsDlg dlg(this);
    dlg.SetRecord(*record);
    dlg.ShowModal();

    SaveSettings();
}


void CEventTablePanel::OnUpdateDetails(wxUpdateUIEvent& event)
{
    event.Enable(m_Table  &&  m_Table->GetSelectedItemCount() == 1);
}


void CEventTablePanel::OnShowErrors(wxCommandEvent& event)
{
    m_Model->ShowType(IEventRecord::eError, event.IsChecked());
}


void CEventTablePanel::OnUpdateShowErrors(wxUpdateUIEvent& event)
{
    event.Check(m_Model  &&  m_Model->IsTypeShown(IEventRecord::eError));
}


void CEventTablePanel::OnShowWarnings(wxCommandEvent& event)
{
    m_Model->ShowType(IEventRecord::eWarning, event.IsChecked());
}


void CEventTablePanel::OnUpdateShowWarnings(wxUpdateUIEvent& event)
{
    event.Check(m_Model  &&  m_Model->IsTypeShown(IEventRecord::eWarning));
}


void CEventTablePanel::OnShowInfo(wxCommandEvent& event)
{
    m_Model->ShowType(IEventRecord::eInfo, event.IsChecked());
}


void CEventTablePanel::OnUpdateShowInfo(wxUpdateUIEvent& event)
{
    event.Check(m_Model  &&  m_Model->IsTypeShown(IEventRecord::eInfo));
}

///////////////////////////////////////////////////////////////////////////////
/// CEventView

BEGIN_EVENT_MAP(CEventView, CEventHandler)
    ON_EVENT(CLogServiceEvent, CLogServiceEvent::eNewRecord, &CEventView::x_OnNewRecord)
END_EVENT_MAP()

CViewTypeDescriptor CEventView::m_TypeDescr(
    "Event View", // type name
    "event_view", // icon alias TODO
    "Event View displays a list of application events.",
    "Event View displays a list of application events.",
    "EVENT_VIEW", // help ID
    "System",     // category
    true);       // singleton


CEventView::CEventView()
:   m_Service(NULL),
    m_Panel(NULL)
{
}


CEventView::~CEventView()
{
}

void CEventView::x_OnNewRecord(CEvent*)
{
    m_Panel->UpdateTable();
}

wxWindow* CEventView::GetWindow()
{
    return m_Panel;
}

wxEvtHandler* CEventView::GetCommandHandler()
{
    _ASSERT(m_Panel);
    CwxTableListCtrl* table = m_Panel->GetTable();
    return table->GetEventHandler();
}

string CEventView::GetClientLabel(IWMClient::ELabel ltype) const
{
    return m_TypeDescr.GetLabel();
}


string CEventView::GetIconAlias() const
{
    return m_TypeDescr.GetIconAlias();
}


const CRgbaColor* CEventView::GetColor() const
{
    return NULL;
}


void CEventView::SetColor(const CRgbaColor& color)
{
    // do nothing
}


const wxMenu* CEventView::GetMenu()
{
    return NULL;
}


void CEventView::UpdateMenu(wxMenu& root_menu)
{
}


IWMClient::CFingerprint CEventView::GetFingerprint() const
{
    return CFingerprint(m_TypeDescr.GetLabel(), true);
}


const CViewTypeDescriptor& CEventView::GetTypeDescriptor() const
{
    return m_TypeDescr;
}


void CEventView::SetWorkbench(IWorkbench* workbench)
{
    if(workbench)   {
        // connect to services
        m_Service  = workbench->GetServiceByType<CEventLogService>();
        m_Service->AddListener(this);
        m_Panel->CreateModel(m_Service);
    } else {
        if(m_Service)   {
            m_Service->RemoveListener(this);
            m_Panel->DestroyModel();
            m_Service = NULL;
        }
    }
}


void CEventView::CreateViewWindow(wxWindow* parent)
{
    m_Panel = new CEventTablePanel();
    m_Panel->Create(parent, wxID_ANY);
}


void CEventView::DestroyViewWindow()
{
    if(m_Panel) {
        m_Panel->Destroy();
        m_Panel = NULL;
    }
}


const char* kTableSection = ".Table";

void CEventView::SetRegistryPath(const string& path)
{
    m_RegPath = path; // store for later use
    m_Panel->SetRegistryPath(m_RegPath + kTableSection);
}


void CEventView::SaveSettings() const
{
    m_Panel->SaveSettings();
}


void CEventView::LoadSettings()
{
    m_Panel->LoadSettings();
}


///////////////////////////////////////////////////////////////////////////////
/// CEventViewFactory

void CEventViewFactory::RegisterIconAliases(wxFileArtProvider& provider)
{
    //TODO
}


const CViewTypeDescriptor& CEventViewFactory::GetViewTypeDescriptor() const
{
    return CEventView::m_TypeDescr;
}


IView* CEventViewFactory::CreateInstance() const
{
    return new CEventView();
}


IView* CEventViewFactory::CreateInstanceByFingerprint(const TFingerprint& fingerprint) const
{
    TFingerprint print(CEventView::m_TypeDescr.GetLabel(), true);
    if(print == fingerprint)   {
        return new CEventView();
    }
    return NULL;
}


string CEventViewFactory::GetExtensionIdentifier() const
{
    static string sid("event_view_factory");
    return sid;
}


string CEventViewFactory::GetExtensionLabel() const
{
    static string slabel("Event View Factory");
    return slabel;
}


END_NCBI_SCOPE
