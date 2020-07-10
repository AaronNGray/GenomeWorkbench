/*  $Id: open_objects_panel.cpp 39666 2017-10-25 16:01:13Z katargir $
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
 * Authors: Roman Katargin
 *
 * File Description:
 *
 */
#include <ncbi_pch.hpp>

#include <gui/widgets/loaders/open_objects_panel.hpp>
#include <gui/widgets/loaders/tool_wizard.hpp>

#include <gui/utils/object_loader.hpp>
#include <gui/utils/execute_unit.hpp>

#include <gui/widgets/wx/wx_utils.hpp>

#include <wx/sizer.h>
#include <wx/htmllbox.h>
#include <wx/msgdlg.h>

#include <gui/widgets/wx/wizard_dlg.hpp>

BEGIN_NCBI_SCOPE


IMPLEMENT_DYNAMIC_CLASS( COpenObjectsPanel, CSplitter )

BEGIN_EVENT_TABLE(COpenObjectsPanel, CSplitter)
    EVT_LISTBOX( ID_LOADER_LIST, COpenObjectsPanel::OnLoaderSelected )
END_EVENT_TABLE()

COpenObjectsPanel::COpenObjectsPanel()
 : m_WizardPage(*this)
{
    Init();
}


COpenObjectsPanel
::COpenObjectsPanel( wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style )
 : m_WizardPage(*this)
{
    Init();
    Create(parent, id, pos, size, style);
}


bool COpenObjectsPanel::Create( wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style )
{
    CSplitter::Create( parent, id, pos, size, style );

    int sizes[3] = { 150, 250, -1 };
    Split(CSplitter::eHorizontal, sizes);

    CreateControls();
    if (GetSizer())
    {
        GetSizer()->SetSizeHints(this);
    }
    Centre();

    return true;
}


COpenObjectsPanel::~COpenObjectsPanel()
{
}

void COpenObjectsPanel::Init()
{
    m_LoadersListBox = NULL;
    m_OptionPanel = NULL;
    m_CurrManager = -1;
}

void COpenObjectsPanel::CreateControls()
{
    wxArrayString m_OptionListStrings;
    m_LoadersListBox = new wxSimpleHtmlListBox( this, ID_LOADER_LIST, wxDefaultPosition, wxSize(160, 320), m_OptionListStrings, wxHLB_DEFAULT_STYLE );
    InsertToCell(m_LoadersListBox, 0, 0);
}

void COpenObjectsPanel::SetManagers(vector<CIRef<IOpenObjectsPanelClient> >& managers)
{
    _ASSERT(m_Managers.size() == 0);

    m_Managers = managers;

    if (m_Managers.size() == 0)
        return;

    if (m_Managers.size() == 1) {
        this->Hide();
        m_OptionPanel = new wxPanel( GetParent(), ID_OPTION_PANEL, wxDefaultPosition, wxDefaultSize, wxNO_BORDER|wxTAB_TRAVERSAL );
        m_OptionPanel->SetSizer(new wxBoxSizer(wxVERTICAL));
    }
    else {
        m_OptionPanel = new wxPanel( this, ID_OPTION_PANEL, wxDefaultPosition, wxDefaultSize, wxNO_BORDER|wxTAB_TRAVERSAL );
        m_OptionPanel->SetSizer(new wxBoxSizer(wxVERTICAL));
        InsertToCell(m_OptionPanel, 1, 0);
    }

    // initialize the new managers
    for( size_t i = 0;  i < m_Managers.size();  i++ ) {
		IOpenObjectsPanelClient& manager = *m_Managers[i];
		IToolWizard& wizard = dynamic_cast<IToolWizard&>(manager);
        wizard.SetParentWindow(m_OptionPanel);
        wizard.SetPrevPage(&m_WizardPage);
        x_AppendOptionForManager(manager);
    }
}

IObjectLoader* COpenObjectsPanel::GetObjectLoader()
{
    IObjectLoader* object_loader = 0;
    if (m_CurrManager >= 0) {
        object_loader = dynamic_cast<IObjectLoader*>(m_Managers[m_CurrManager]->GetExecuteUnit());
    }
    return object_loader;
}

bool COpenObjectsPanel::x_CanLeavePage(bool forward)
{
    if (!forward)
        return false;

    if (m_CurrManager < 0) {
        wxMessageBox(wxT("Please, select loader from the list on the left side"), wxT("Open Objects"),
            wxOK|wxICON_EXCLAMATION, GetParent());
        return false;
    }

	IToolWizard& wizard = dynamic_cast<IToolWizard&>(*m_Managers[m_CurrManager]);

    IWizardPage* managerPage = wizard.GetFirstPage();
    return managerPage->CanLeavePage(forward);
}

wxPanel* COpenObjectsPanel::x_GetPanel()
{
    if (m_Managers.size() == 1)
        return m_OptionPanel;

    return this;
}

IWizardPage* COpenObjectsPanel::x_GetNextPage()
{
	if (m_CurrManager < 0)
		return 0;

	IToolWizard& wizard = dynamic_cast<IToolWizard&>(*m_Managers[m_CurrManager]);
	return wizard.GetFirstPage()->GetNextPage();
}

IWizardPage* COpenObjectsPanel::x_GetOptionsPage()
{
	if (m_CurrManager < 0)
		return 0;

	IToolWizard& wizard = dynamic_cast<IToolWizard&>(*m_Managers[m_CurrManager]);
	return wizard.GetFirstPage()->GetOptionsPage();
}

/// add a Loading Option (displayed in the list on the left) for the given Manager
void COpenObjectsPanel::x_AppendOptionForManager(IOpenObjectsPanelClient& manager)
{
    string s = manager.GetLabel();

    // generating an HTML label
    string s_html;
    size_t prev_pos = 0;
    size_t pos = s.find(" ", prev_pos);
    while(pos != string::npos)  {
        // found a space - copy the text before space
        s_html += s.substr(prev_pos, pos - prev_pos);
        size_t pos_end = s.find_last_of(" ", pos);
        for(  ; pos <= pos_end; pos++ ) {
            s_html += "&nbsp;";
        }
        prev_pos = pos_end + 1;
        pos = s.find(" ", prev_pos);
    }
    s_html += s.substr(prev_pos, s.size() - prev_pos);

    m_LoadersListBox->Append(ToWxString(s_html));
}

void COpenObjectsPanel::OnLoaderSelected( wxCommandEvent& event )
{
    x_SelectManager(m_LoadersListBox->GetSelection());
}

void COpenObjectsPanel::x_SelectManager(int index)
{
    if (m_CurrManager != index)  {
        if (m_CurrManager >= 0) {
			IToolWizard& wizard = dynamic_cast<IToolWizard&>(*m_Managers[m_CurrManager]);
            IWizardPage* managerPage = wizard.GetFirstPage();
            wxPanel* panel = managerPage->GetPanel();
            if (panel) {
                panel->Hide();
            }
        }

        m_CurrManager = index;
        m_LoadersListBox->SetSelection(m_CurrManager);
        m_LoadersListBox->Update();

        if (m_CurrManager != -1) {
            wxBusyCursor wait;

			IToolWizard& wizard = dynamic_cast<IToolWizard&>(*m_Managers[m_CurrManager]);
            IWizardPage* managerPage = wizard.GetFirstPage();
            wxPanel* panel = managerPage->GetPanel();
            if (panel->GetParent() != m_OptionPanel) {
                panel->Reparent(m_OptionPanel);
            }

            if(panel->GetContainingSizer() != m_OptionPanel->GetSizer())   {
                m_OptionPanel->GetSizer()->Add(panel, 1, wxEXPAND | wxALL, 5);
            }

            panel->Show();
            panel->SetFocus();
            m_OptionPanel->Layout();
        }
    }

    wxCommandEvent evt(wxEVT_COMMAND_MENU_SELECTED, CWizardDlg::eCmdUpdateButtons);
    GetParent()->ProcessWindowEvent(evt);
}

void COpenObjectsPanel::SetRegistryPath(const string& path)
{
    m_RegPath = path;
}

static const char* kSplitterWidthsTag = "SplitterWidths";
static const char* kSelOptionTag = "SelectedOption";

void COpenObjectsPanel::LoadSettings()
{
    if (m_RegPath.empty())
        return;

    for(  size_t i = 0;  i < m_Managers.size();  i++ )  {
		IOpenObjectsPanelClient& manager = *m_Managers[i];
        IRegSettings* rgs = dynamic_cast<IRegSettings*>(&manager);
        if(rgs) {
            string sub_section = manager.GetLabel();
            string mgr_path = m_RegPath + "." + sub_section;
            rgs->SetRegistryPath(mgr_path);
            rgs->LoadSettings();
        }
    }

    CGuiRegistry& gui_reg = CGuiRegistry::GetInstance();
    CRegistryReadView view = gui_reg.GetReadView(m_RegPath);

    // load splitter configuration
    vector<int> widths;
    view.GetIntVec(kSplitterWidthsTag, widths);
    if(widths.size() == 2)  {
        SetWidths(widths);
    }

    // load the default option
    string sel_mgr_label = view.GetString(kSelOptionTag, "empty");

    // load Managers' settings and find the selected manager
    for(  size_t i = 0;  i < m_Managers.size();  i++ )  {
		IOpenObjectsPanelClient& manager = *m_Managers[i];
        // check whether this is the selected manager
        string mgr_label = manager.GetLabel();
        if(mgr_label == sel_mgr_label)  {
            m_CurrManager = (int)i;
        }
    }

    int manager = max(m_CurrManager, 0);
    m_CurrManager = -1;
    x_SelectManager(manager);
}

void COpenObjectsPanel::SaveSettings() const
{
    if (m_RegPath.empty())
        return;

    for(  size_t i = 0;  i < m_Managers.size();  i++ )  {
		IOpenObjectsPanelClient& manager = const_cast<IOpenObjectsPanelClient&>(*m_Managers[i]);
        IRegSettings* rgs = dynamic_cast<IRegSettings*>(&manager);
        if(rgs) {
            string sub_section = manager.GetLabel();
            string mgr_path = m_RegPath + "." + sub_section;
            rgs->SetRegistryPath(mgr_path);
            rgs->SaveSettings();
        }
    }

    CGuiRegistry& gui_reg = CGuiRegistry::GetInstance();
    CRegistryWriteView view = gui_reg.GetWriteView(m_RegPath);

    // save splitter configuration
    vector<int> widths;
    GetWidths(widths);
    view.Set(kSplitterWidthsTag, widths);

    // remember the selected option
    if(m_CurrManager != -1) {
        const IOpenObjectsPanelClient& manager = *m_Managers[m_CurrManager];
        string mgr_label = manager.GetLabel();
        view.Set(kSelOptionTag, mgr_label);
    }
}

END_NCBI_SCOPE
