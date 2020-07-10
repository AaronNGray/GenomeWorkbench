/*  $Id: run_tool_dlg.cpp 41347 2018-07-12 18:28:31Z evgeniev $
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
 * Authors:
 *
 * File Description:
 *
 */
#include <ncbi_pch.hpp>

#include <gui/core/run_tool_dlg.hpp>
#include <gui/framework/item_selection_panel.hpp>
#include <gui/framework/menu_service.hpp>
#include <gui/framework/service.hpp>
#include <gui/widgets/wx/wx_utils.hpp>
#include <gui/widgets/wx/message_box.hpp>
#include <gui/objutils/registry.hpp>
#include <gui/objutils/reg_settings.hpp>
#include <gui/objutils/usage_report_job.hpp>

#include <gui/core/quick_launch_list.hpp>
#include <gui/core/recent_tool_list.hpp>
#include "add_quick_launch_dlg.hpp"

#include <wx/sizer.h>
#include <wx/button.h>
#include <wx/icon.h>
#include <wx/msgdlg.h>


BEGIN_NCBI_SCOPE


///////////////////////////////////////////////////////////////////////////////
/// CToolManagerItem
class CToolManagerItem :
    public CObject,
    public IItemWithDescription
{
public:
    CToolManagerItem(IUIAlgoToolManager& manager)
        :   m_Manager(&manager)
    {
    }

    virtual const IUIObject&  GetDescriptor() const
    {
        return m_Manager->GetDescriptor();
    }

    virtual string    GetCategory()
    {
        return m_Manager->GetCategory();
    }

    virtual bool      ShowDefault() const { return false; }

    CIRef<IUIAlgoToolManager>   m_Manager;
};


///////////////////////////////////////////////////////////////////////////////
/// CRecentToolItem
class CToolRecentItem : public CToolManagerItem
{
public:
    CToolRecentItem(IUIAlgoToolManager& manager)
        : CToolManagerItem(manager)
    {
    }

    virtual string GetCategory()
    {
        return "Recent Tools";
    }
};


///////////////////////////////////////////////////////////////////////////////
/// CRunToolDlg

IMPLEMENT_DYNAMIC_CLASS( CRunToolDlg, CDialog )

BEGIN_EVENT_TABLE( CRunToolDlg, CDialog )
////@begin CRunToolDlg event table entries
    EVT_BUTTON( ID_RESTORE_DEFAULTS_BTN, CRunToolDlg::OnRestoreDefaultsBtnClick )
    EVT_BUTTON( ID_BUTTON, CRunToolDlg::OnButtonClick )
    EVT_BUTTON( wxID_BACKWARD, CRunToolDlg::OnBackwardClick )
    EVT_BUTTON( wxID_FORWARD, CRunToolDlg::OnForwardClick )
////@end CRunToolDlg event table entries
    EVT_LISTBOX_DCLICK(wxID_ANY, CRunToolDlg::OnToolSelected)
END_EVENT_TABLE()


CRunToolDlg::CRunToolDlg()
    : m_RecentTools(3)
{
    Init();
}


CRunToolDlg::CRunToolDlg( wxWindow* parent, const vector<TManagerRef>& managers, IServiceLocator* srvLocator )
    : m_SrvLocator(srvLocator), m_Managers(managers), m_RecentTools(3)
{
    Init();
    Create(parent);
}


bool CRunToolDlg::Create( wxWindow* parent, wxWindowID id, const wxString& caption, const wxPoint& pos, const wxSize& size, long style )
{
////@begin CRunToolDlg creation
    SetExtraStyle(wxWS_EX_BLOCK_EVENTS);
    CDialog::Create( parent, id, caption, pos, size, style );

    CreateControls();
    if (GetSizer())
    {
        GetSizer()->SetSizeHints(this);
    }
    Centre();
////@end CRunToolDlg creation
    return true;
}


CRunToolDlg::~CRunToolDlg()
{
////@begin CRunToolDlg destruction
////@end CRunToolDlg destruction
}


void CRunToolDlg::Init()
{
////@begin CRunToolDlg member initialisation
    m_Panel = NULL;
    m_HelpLink = NULL;
    m_RestoreDefaultsBtn = NULL;
    m_QuickLaunch = NULL;
    m_BackBtn = NULL;
    m_NextBtn = NULL;
////@end CRunToolDlg member initialisation
    m_CurrPanel = NULL;
    m_ItemPanel = NULL;
}


void CRunToolDlg::CreateControls()
{
////@begin CRunToolDlg content construction
    CRunToolDlg* itemCDialog1 = this;

    wxBoxSizer* itemBoxSizer2 = new wxBoxSizer(wxVERTICAL);
    itemCDialog1->SetSizer(itemBoxSizer2);

    m_Panel = new wxPanel( itemCDialog1, ID_PANEL, wxDefaultPosition, wxSize(400, 300), wxFULL_REPAINT_ON_RESIZE|wxTAB_TRAVERSAL );
    itemBoxSizer2->Add(m_Panel, 1, wxGROW, 5);

    wxBoxSizer* itemBoxSizer4 = new wxBoxSizer(wxVERTICAL);
    m_Panel->SetSizer(itemBoxSizer4);

    wxStaticLine* itemStaticLine5 = new wxStaticLine( itemCDialog1, wxID_STATIC, wxDefaultPosition, wxDefaultSize, wxLI_HORIZONTAL );
    itemBoxSizer2->Add(itemStaticLine5, 0, wxGROW|wxALL, 5);

    wxBoxSizer* itemBoxSizer6 = new wxBoxSizer(wxHORIZONTAL);
    itemBoxSizer2->Add(itemBoxSizer6, 0, wxGROW|wxLEFT|wxRIGHT, 5);

    m_HelpLink = new wxHyperlinkCtrl( itemCDialog1, ID_HYPERLINKCTRL, _("Help"), wxEmptyString, wxDefaultPosition, wxDefaultSize, wxHL_DEFAULT_STYLE );
    m_HelpLink->SetForegroundColour(wxColour(128, 128, 128));
    itemBoxSizer6->Add(m_HelpLink, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    m_RestoreDefaultsBtn = new wxButton( itemCDialog1, ID_RESTORE_DEFAULTS_BTN, _("Defaults"), wxDefaultPosition, wxDefaultSize, 0 );
    if (CRunToolDlg::ShowToolTips())
        m_RestoreDefaultsBtn->SetToolTip(_("Restore default options"));
    itemBoxSizer6->Add(m_RestoreDefaultsBtn, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    itemBoxSizer6->Add(5, 5, 1, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxBoxSizer* itemBoxSizer10 = new wxBoxSizer(wxHORIZONTAL);
    itemBoxSizer6->Add(itemBoxSizer10, 0, wxALIGN_CENTER_VERTICAL, 5);

    m_QuickLaunch = new wxButton( itemCDialog1, ID_BUTTON, _("Add Quick Launch..."), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer10->Add(m_QuickLaunch, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    m_BackBtn = new wxButton( itemCDialog1, wxID_BACKWARD, _("< &Back"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer10->Add(m_BackBtn, 0, wxALIGN_CENTER_VERTICAL|wxLEFT|wxTOP|wxBOTTOM, 5);

    // in wxCocoa, we get a little button overlap here without the spacer.  I believe
    // this is a bug, but we need to pad for now. The variable is added
    // to create a syntax error if this is overwritten by DialogBlocks
    bool spacer_code_included = true;
#ifdef __WXOSX_COCOA__   
    itemBoxSizer10->AddSpacer(6);
#endif

    m_NextBtn = new wxButton( itemCDialog1, wxID_FORWARD, _("&Next >"), wxDefaultPosition, wxDefaultSize, 0 );
    m_NextBtn->SetDefault();
    itemBoxSizer10->Add(m_NextBtn, 0, wxALIGN_CENTER_VERTICAL|wxRIGHT|wxTOP|wxBOTTOM, 5);

    wxButton* itemButton14 = new wxButton( itemCDialog1, wxID_CANCEL, _("&Cancel"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer6->Add(itemButton14, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

////@end CRunToolDlg content construction

    if (!spacer_code_included) {
        // If spacer_code_included is not defined, it has been overwritten 
        // so you must add the following code after m_BackBtn is added and
        // before m_NextBtn is added.  We need this since that section of code
        // can be overridden by DialogBlocks.
        /*
    // in wxCocoa, we get a little button overlap here without the spacer.  I believe
    // this is a bug, but we need to pad for now. The variable is added
    // to create a syntax error if this is overwritten by DialogBlocks
    bool spacer_code_included = true;
#ifdef __WXOSX_COCOA__   
    itemBoxSizer10->AddSpacer(6);
#endif
        */
    }

    for( size_t i =0; i < m_Managers.size(); i++ ){
        TManagerRef manager = m_Managers[i];
        manager->SetServiceLocator(m_SrvLocator);
        manager->SetParentWindow(m_Panel);
    }

    if (m_Managers.size() == 1) {
        m_CurrManager = m_Managers[0];
    } else {
        m_ItemPanel = new CItemSelectionPanel(m_Panel, wxID_ANY);
        m_ItemPanel->SetItemTypeLabel("tool");

        m_ItemPanel->m_MapWidget->GetMapControl()->GetProperties().m_SingleColumn = true;
    }

    x_CalcMinPanelSize();

    if (m_ItemPanel)
        x_SetCurrentPanel(m_ItemPanel);

    x_UpdateButtons();
}


typedef CGroupMapWidget::SGroupDescr TGroupDescr;

class ToolCmp
{
public:
    ToolCmp(const string& label) : m_ToolLabel(label) {}

    bool operator()(CConstIRef<IUIAlgoToolManager> tool)
    {
        return (tool->GetDescriptor().GetLabel() == m_ToolLabel);
    }

private:
    string m_ToolLabel;
};

void CRunToolDlg::x_CreateToolItems()
{
    typedef CIRef<IItemWithDescription> TItemRef;
    vector<TItemRef> items;

    bool have_recent_tool = false;
    // Recent tools group
    ITERATE(CMRUList<string>::TItems, it, m_RecentTools.GetItems()) {
        vector<TManagerRef>::iterator it_tool =
            find_if(m_Managers.begin(), m_Managers.end(), ToolCmp(*it));

        if (it_tool != m_Managers.end()) {
            have_recent_tool = true;
            items.push_back(TItemRef(new CToolRecentItem(**it_tool)));
        }
    }

    for( size_t i =0; i < m_Managers.size(); i++ ){
        TManagerRef manager = m_Managers[i];
        manager->InitUI();
        TItemRef item(new CToolManagerItem(*manager));
        items.push_back(item);
    }

    m_ItemPanel->SetItems(items);

    if (have_recent_tool) {
        CSelectionControl* control = m_ItemPanel->m_MapWidget->GetMapControl();
        int selected = control->GetSelectedIndex();
        if (selected != -1)
            control->SelectItem(selected, false);
        control->SelectItem(1);
    }

    x_UpdateButtons();
}


void CRunToolDlg::SetInputObjects(const vector<TConstScopedObjects>& objects)
{
    m_InputObjects = objects;
}

int CRunToolDlg::ShowModal()
{
    if (m_CurrManager) {
        string err_str = m_CurrManager->SetInputObjects(m_InputObjects);

        if( !err_str.empty() ){
            wxMessageBox(
                ToWxString( err_str ), wxT("Error"), 
                wxOK | wxICON_ERROR, this
            );
            return wxID_CANCEL;
        }

        if( x_DoTransition(IUIToolManager::eNext) ){
            return wxID_OK;
        }
    }
    return CDialog::ShowModal();
}

CIRef<IAppTask> CRunToolDlg::GetToolTask()
{
    return m_AppTask;
}


void CRunToolDlg::OnToolSelected(wxCommandEvent& event)
{
    x_OnToolSelected();
}

void CRunToolDlg::x_CalcMinPanelSize()
{
    wxSizer* sizer = m_Panel->GetSizer();
    wxSize minSize(0, 0);

    if (m_CurrManager) {
        wxPanel* panel = m_CurrManager->GetMaxPanel();
        if (panel) {
            panel->Show();
            sizer->Add(panel, 1, wxEXPAND | wxALL, 5);
            minSize.IncTo(sizer->GetMinSize());
            sizer->Detach(panel);
            panel->Destroy();
        }
    }
    else {
        for( size_t i =0; i < m_Managers.size(); i++ ){
            TManagerRef manager = m_Managers[i];
            wxPanel* panel = manager->GetMaxPanel();
            if (!panel) continue;
            panel->Show();
            sizer->Add(panel, 1, wxEXPAND | wxALL, 5);
            minSize.IncTo(sizer->GetMinSize());
            sizer->Detach(panel);
            panel->Destroy();
        }

        m_ItemPanel->Show();
        sizer->Add(m_ItemPanel, 1, wxEXPAND | wxALL, 5);
        minSize.IncTo(sizer->GetMinSize());
        m_ItemPanel->Hide();
        sizer->Detach(m_ItemPanel);
        //sizer->SetMinSize(minSize);
    }

    m_Panel->SetClientSize(minSize);
    m_Panel->SetMinSize(m_Panel->GetSize());

    GetSizer()->SetSizeHints(this);
}

// set ne pluggable panel
void CRunToolDlg::x_SetCurrentPanel(wxPanel* panel, const wxString& caption)
{
    if(panel == m_CurrPanel)  {
        return;
    }

    if(m_CurrPanel) {
        m_CurrPanel->Hide();
        m_CurrPanel = NULL;
        SetTitle(wxT(""));
    }
    if(panel)   {
        if( panel->GetContainingSizer() != m_Panel->GetSizer()) {
            m_Panel->GetSizer()->Add(panel, 1, wxEXPAND | wxALL, 5);
        }
        m_CurrPanel = panel;
        SetTitle(caption);

        m_CurrPanel->Show();
        m_Panel->Layout();
        m_CurrPanel->SetFocus();
    }
}


bool CRunToolDlg::x_DoTransition(IUIToolManager::EAction action)
{
	try {
		if(m_CurrManager->DoTransition(action))   {
			if(m_CurrManager->IsCompletedState()) {
				// we are done
				m_AppTask.Reset(m_CurrManager->GetTask());

                // Report usage
                const IUIObject& descriptor = m_CurrManager->GetDescriptor();
                const string& log_event = descriptor.GetLogEvent();
                if (!log_event.empty()) {
                    REPORT_USAGE(log_event, .Add("tool_name", descriptor.GetLabel()));
                }

                CRecentToolList& recentTool = CRecentToolList::GetInstance();
                string toolLabel = m_CurrManager->GetDescriptor().GetLabel();
                recentTool.AddItem(toolLabel);

                CIRef<IMenuService> menu_srv = m_SrvLocator->GetServiceByType<IMenuService>();
                if (menu_srv)
                    menu_srv->ResetMenuBar();

				LOG_POST(Info << "CRunToolDlg: Task created: " 
							  << toolLabel
							  << " task descr=" 
							  << m_AppTask->GetDescr()
							  );
                return true;
			} else {
				/// show the panel for the next step
				wxPanel* new_panel = m_CurrManager->GetCurrentPanel();

				if(new_panel == NULL)   {
					x_SetCurrentPanel(m_ItemPanel);

				} else {
					wxString title = SYMBOL_CRUNTOOLDLG_TITLE;
					title += wxT(" - ");
					title += ToWxString(m_CurrManager->GetDescriptor().GetLabel());

					x_SetCurrentPanel(new_panel, title);
				}
			}
		    x_UpdateButtons();
		}
	}
	catch(CException& ex)
	{
		ERR_POST(ex.ReportAll());
		NcbiErrorBox(ex.GetMsg());
        return true;
	}
	catch(std::exception& ex)
	{
        ERR_POST(ex.what());
		NcbiErrorBox(ex.what());
        return true;
	}
    return false;
}


/// handles the first step of the dialog - selecting the tool
void CRunToolDlg::x_OnToolSelected()
{
    CIRef<IItemWithDescription> item = m_ItemPanel->GetSelectedItemRef();
    CToolManagerItem* tool_item = dynamic_cast<CToolManagerItem*>(item.GetPointer());
    if(tool_item != NULL) {

        const IUIObject &ui_obj = item->GetDescriptor();
        wxString help_url(wxT("https://www.ncbi.nlm.nih.gov/tools/gbench/manual5/#"));
        help_url += ui_obj.GetHelpId();
        m_HelpLink->SetURL(help_url);

        m_CurrManager = tool_item->m_Manager;
        m_RecentTools.AddItem(m_CurrManager->GetDescriptor().GetLabel());

        if( m_CurrManager ){
            string err_str = m_CurrManager->SetInputObjects(m_InputObjects);

            if( !err_str.empty() ){
                wxMessageBox(
                    ToWxString( err_str ), wxT("Error"), 
                    wxOK | wxICON_ERROR, this
                );
                return;
            }

            if( x_DoTransition(IUIToolManager::eNext) ){
                EndModal(wxID_OK);
            }
        }

        return;
    }

    wxMessageBox(wxT("Please select a tool!"), wxT("Run Tool"),
                 wxOK | wxICON_EXCLAMATION, this);
}


void CRunToolDlg::x_UpdateButtons()
{
    bool can_back = false;
    bool can_next = true;
    bool final = false, quickLaunch = false;;


    if(m_CurrManager) {
        can_back = m_CurrManager->CanDo(IUIToolManager::eBack) || 
            (m_ItemPanel && m_CurrPanel != m_ItemPanel);
        can_next = m_CurrManager->CanDo(IUIToolManager::eNext);
        final = m_CurrManager->IsFinalState();
        quickLaunch = m_CurrManager->CanQuickLaunch();
    }

    m_RestoreDefaultsBtn->Show(can_back);
    m_BackBtn->Show(can_back);
    m_NextBtn->Show(can_next);
    m_NextBtn->SetLabel(final? wxT("Finish") : wxT("Next >"));

    m_QuickLaunch->Show(final);
    if (final)
        m_QuickLaunch->Enable(quickLaunch);

    Layout();
}


void CRunToolDlg::OnBackwardClick(wxCommandEvent& event)
{
    if (x_DoTransition(IUIToolManager::eBack)) {
        EndModal(wxID_OK);
    }
}


void CRunToolDlg::OnForwardClick(wxCommandEvent& event)
{
    if(m_ItemPanel && m_CurrPanel == m_ItemPanel)   {
        // this is the first step
        x_OnToolSelected();
    } else {
        if (x_DoTransition(IUIToolManager::eNext)) {
            EndModal(wxID_OK);
        }
    }
}


void CRunToolDlg::SetRegistryPath(const string& path)
{
    CDialog::SetRegistryPath(path);

    if (m_ItemPanel)
        m_ItemPanel->SetRegistryPath(path + "ItemsPanel");

    /// set Registry paths for all Managers
    /// each Manager will get a section located under the dialog's section
    for(  size_t i = 0;  i < m_Managers.size();  i++ )  {
        TManagerRef manager = m_Managers[i];
        IRegSettings* rgs = dynamic_cast<IRegSettings*>(manager.GetPointer());
        if(rgs) {
            string sub_section = manager->GetDescriptor().GetLabel();
            string mgr_path = m_RegPath + "." + sub_section;
            rgs->SetRegistryPath(mgr_path);
        }
    }
}


void CRunToolDlg::EndModal(int ret_code)
{
    CDialog::EndModal(ret_code);

    for (auto& m : m_Managers)
        m->CleanUI();
}


//static const char* kSelOptionTag = "SelectedOption";
static const char* kRecentTools = "Recent";


void CRunToolDlg::x_SaveSettings(CRegistryWriteView view) const
{
    if( ! m_RegPath.empty())   {
        // save Items Panel
        if (m_ItemPanel)
            m_ItemPanel->SaveSettings();

        view.Set(kRecentTools, m_RecentTools.GetItems());

        // save Managers' settings
        for( size_t i = 0;  i < m_Managers.size();  i++ )  {
            TManagerRef manager = m_Managers[i];
            IRegSettings* rgs = dynamic_cast<IRegSettings*>(manager.GetPointer());
            if(rgs) {
                rgs->SaveSettings();
            }
        }
    }
}


void CRunToolDlg::x_LoadSettings(const CRegistryReadView& view)
{
    if( ! m_RegPath.empty())   {
        // load Items panel settings
        if (m_ItemPanel)
            m_ItemPanel->LoadSettings();

        view.GetStringList(kRecentTools, m_RecentTools.SetItems());

        // load Managers' settigns and find the selected manager
        for(  size_t i = 0;  i < m_Managers.size();  i++ )  {
            TManagerRef manager = m_Managers[i];
            // load settings (if supported)
            IRegSettings* rgs = dynamic_cast<IRegSettings*>(manager.GetPointer());
            if(rgs) {
                rgs->LoadSettings();
            }
        }
    }

    if (m_Managers.size() > 1)
        x_CreateToolItems();
}


bool CRunToolDlg::ShowToolTips()
{
    return true;
}


wxBitmap CRunToolDlg::GetBitmapResource( const wxString& name )
{
    // Bitmap retrieval
////@begin CRunToolDlg bitmap retrieval
    wxUnusedVar(name);
    return wxNullBitmap;
////@end CRunToolDlg bitmap retrieval
}


wxIcon CRunToolDlg::GetIconResource( const wxString& name )
{
    // Icon retrieval
////@begin CRunToolDlg icon retrieval
    wxUnusedVar(name);
    return wxNullIcon;
////@end CRunToolDlg icon retrieval
}


void CRunToolDlg::OnButtonClick( wxCommandEvent& event )
{
    if (!m_CurrManager)
        return;

    CAddQuickLaunchDlg dlg(this);
    if (dlg.ShowModal() != wxID_OK)
        return;

    string toolLabel = m_CurrManager->GetDescriptor().GetLabel();
    string entry = toolLabel + " (" + dlg.GetDescr() + ")";

    CQuickLaunchList& quickLaunch = CQuickLaunchList::GetInstance();
    quickLaunch.AddItem(entry);

    string regPath = quickLaunch.GetItemPath(entry);
    CGuiRegistry::GetInstance().Set(regPath + ".Tool", toolLabel);

    IRegSettings* rgs = dynamic_cast<IRegSettings*>(m_CurrManager.GetPointer());
    if (rgs) {
        rgs->SetRegistryPath(regPath + ".Params");
        rgs->SaveSettings();
        rgs->SetRegistryPath(m_RegPath + "." + toolLabel);
    }

    CIRef<IMenuService> menu_srv = m_SrvLocator->GetServiceByType<IMenuService>();
    if (menu_srv)
        menu_srv->ResetMenuBar();
}

void CRunToolDlg::OnRestoreDefaultsBtnClick( wxCommandEvent& event )
{
    if (!m_CurrManager)
        return;

    m_CurrManager->RestoreDefaults();
}

END_NCBI_SCOPE
