/*  $Id: open_dlg.cpp 44287 2019-11-22 16:05:44Z katargir $
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

#include <gui/core/open_dlg.hpp>

#include <gui/core/ui_file_load_manager.hpp>

#include <gui/widgets/wx/splitter.hpp>
#include <gui/widgets/wx/wx_utils.hpp>
#include <gui/widgets/wx/wnd_layout_registry.hpp>

#include <gui/objutils/registry.hpp>
#include <gui/objutils/reg_settings.hpp>

#include <gui/utils/extension.hpp>
#include <gui/objutils/usage_report_job.hpp>


////@begin includes
////@end includes

#include <wx/listctrl.h>
#include <wx/sizer.h>
#include <wx/button.h>
#include <wx/icon.h>
#include <wx/msgdlg.h>
#include <wx/checkbox.h>
#include <wx/listbox.h>

BEGIN_NCBI_SCOPE


IMPLEMENT_DYNAMIC_CLASS( COpenDlg, CDialog )

BEGIN_EVENT_TABLE( COpenDlg, CDialog )
////@begin COpenDlg event table entries
    EVT_LISTBOX( ID_option_list, COpenDlg::OnOptionListSelected )
    EVT_BUTTON( wxID_BACKWARD, COpenDlg::OnBackwardClick )
    EVT_BUTTON( wxID_FORWARD, COpenDlg::OnForwardClick )
    EVT_BUTTON( wxID_CANCEL, COpenDlg::OnCancelClick )
////@end COpenDlg event table entries
    EVT_BUTTON( ID_SKIP, COpenDlg::OnSkipClick )

    EVT_LIST_ITEM_ACTIVATED(wxID_ANY, COpenDlg::OnListItemActivate)

END_EVENT_TABLE()

COpenDlg::COpenDlg()
{
    Init();
}


COpenDlg::COpenDlg( wxWindow* parent, wxWindowID id, const wxString& caption, const wxPoint& pos, const wxSize& size, long style )
{
    Init();
    Create(parent, id, caption, pos, size, style);
}


bool COpenDlg::Create( wxWindow* parent, wxWindowID id, const wxString& caption, const wxPoint& pos, const wxSize& size, long style )
{
////@begin COpenDlg creation
    SetExtraStyle(wxWS_EX_BLOCK_EVENTS);
    CDialog::Create( parent, id, caption, pos, size, style );

    CreateControls();
    Centre();
////@end COpenDlg creation

    SetMinSize(ConvertDialogToPixels(SYMBOL_COPENDLG_SIZE));

    return true;
}


COpenDlg::~COpenDlg()
{
////@begin COpenDlg destruction
////@end COpenDlg destruction
}


void COpenDlg::Init()
{
////@begin COpenDlg member initialisation
    m_BaseTitle = wxT("Open");
    m_Splitter = NULL;
    m_OptionList = NULL;
    m_BackBtn = NULL;
    m_NextBtn = NULL;
////@end COpenDlg member initialisation

    m_OptionPanel = NULL;
    m_SrvLocator = NULL;
    m_RequestedManager = -1;
    m_CurrManager = -1;
    m_CurrPanel = NULL;
    m_OpenExportFolder = nullptr;
}

namespace
{
    class CUnfocusedListBox : public wxListBox
    {
    public:
        CUnfocusedListBox(wxWindow *parent, wxWindowID id,
            const wxPoint& pos, const wxSize& size, const wxArrayString &choices, long style)
        {
            Create(parent, id, pos, size, choices, style);
        }

        virtual bool AcceptsFocus() const { return false; }
    };
}

static const char* kExportOpenFolderTag = "GBENCH.Application.ExportOpenFolder";

void COpenDlg::CreateControls()
{
////@begin COpenDlg content construction
    COpenDlg* itemCDialog1 = this;

    wxBoxSizer* itemBoxSizer2 = new wxBoxSizer(wxVERTICAL);
    itemCDialog1->SetSizer(itemBoxSizer2);

    m_Splitter = new CSplitter( itemCDialog1, ID_MAIN_PANEL, wxDefaultPosition, wxDLG_UNIT(itemCDialog1, wxSize(266, 184)), wxFULL_REPAINT_ON_RESIZE );
    itemBoxSizer2->Add(m_Splitter, 1, wxGROW|wxALL, wxDLG_UNIT(itemCDialog1, wxSize(5, -1)).x);

    wxArrayString m_OptionListStrings;
    m_OptionList = new CUnfocusedListBox( m_Splitter, ID_option_list, wxDefaultPosition, wxDLG_UNIT(m_Splitter, wxSize(120, 195)), m_OptionListStrings, wxLB_SINGLE );

    wxStaticLine* itemStaticLine5 = new wxStaticLine( itemCDialog1, wxID_STATIC, wxDefaultPosition, wxDefaultSize, wxLI_HORIZONTAL );
    itemBoxSizer2->Add(itemStaticLine5, 0, wxGROW|wxALL, wxDLG_UNIT(itemCDialog1, wxSize(5, -1)).x);

    wxBoxSizer* itemBoxSizer6 = new wxBoxSizer(wxHORIZONTAL);
    itemBoxSizer2->Add(itemBoxSizer6, 0, wxGROW|wxLEFT|wxRIGHT, wxDLG_UNIT(itemCDialog1, wxSize(5, -1)).x);

    wxHyperlinkCtrl* itemHyperlinkCtrl7 = new wxHyperlinkCtrl( itemCDialog1, ID_HELP_LINK, _("Help"), wxEmptyString, wxDefaultPosition, wxDefaultSize, wxHL_DEFAULT_STYLE );
    itemHyperlinkCtrl7->SetForegroundColour(wxColour(192, 192, 192));
    itemHyperlinkCtrl7->Enable(false);
    itemBoxSizer6->Add(itemHyperlinkCtrl7, 0, wxALIGN_CENTER_VERTICAL|wxALL, wxDLG_UNIT(itemCDialog1, wxSize(5, -1)).x);

    itemBoxSizer6->Add(wxDLG_UNIT(itemCDialog1, wxSize(2, -1)).x, wxDLG_UNIT(itemCDialog1, wxSize(-1, 2)).y, 1, wxALIGN_CENTER_VERTICAL|wxALL, wxDLG_UNIT(itemCDialog1, wxSize(5, -1)).x);

    wxBoxSizer* itemBoxSizer9 = new wxBoxSizer(wxHORIZONTAL);
    itemBoxSizer6->Add(itemBoxSizer9, 0, wxALIGN_CENTER_VERTICAL, wxDLG_UNIT(itemCDialog1, wxSize(5, -1)).x);

    m_BackBtn = new wxButton( itemCDialog1, wxID_BACKWARD, _("< &Back"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer9->Add(m_BackBtn, 0, wxALIGN_CENTER_VERTICAL|wxTOP|wxBOTTOM, wxDLG_UNIT(itemCDialog1, wxSize(5, -1)).x);

    // in wxCocoa, we get a little button overlap here without the spacer.  I believe
    // this is a bug, but we need to pad for now. The variable is added
    // to create a syntax error if this is overwritten by DialogBlocks
    bool spacer_code_included = true;
#ifdef __WXOSX_COCOA__   
    itemBoxSizer9->AddSpacer(6);
#endif

    m_NextBtn = new wxButton( itemCDialog1, wxID_FORWARD, _("&Next >"), wxDefaultPosition, wxDefaultSize, 0 );
    m_NextBtn->SetDefault();
    itemBoxSizer9->Add(m_NextBtn, 0, wxALIGN_CENTER_VERTICAL|wxRIGHT|wxTOP|wxBOTTOM, wxDLG_UNIT(itemCDialog1, wxSize(5, -1)).x);

    wxButton* itemButton12 = new wxButton( itemCDialog1, wxID_CANCEL, _("&Cancel"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer6->Add(itemButton12, 0, wxALIGN_CENTER_VERTICAL|wxALL, wxDLG_UNIT(itemCDialog1, wxSize(5, -1)).x);

////@end COpenDlg content construction

    if (!spacer_code_included) {
        // If spacer_code_included is not defined, it has been overwritten 
        // so you must add the following code after m_BackBtn is added and
        // before m_NextBtn is added.  We need this since that section of code
        // can be overridden by DialogBlocks.
        /*
    // in wxCocoa, we get a little button overlap here without the spacer.  I believe
    // this is a bug, but we need to pad for now. The variable is added
    // to create a syntax error if this is overwritten by DialogBlocks
    bool spacer_code included = true;
#ifdef __WXOSX_COCOA__   
    itemBoxSizer9->AddSpacer(6);
#endif
        */
    }

    m_OptionList->SetFont(m_OptionList->GetFont().MakeLarger());

    // init Splitter
    int sizes[3] = { 170, 250, -1 };
    m_Splitter->Split(CSplitter::eHorizontal, sizes);

    m_OptionPanel = new wxPanel(m_Splitter, ID_OPTION_PANEL, wxDefaultPosition,
                                wxDefaultSize, wxNO_BORDER|wxTAB_TRAVERSAL );

    wxBoxSizer* itemBoxSizer_op = new wxBoxSizer(wxVERTICAL);
    m_OptionPanel->SetSizer(itemBoxSizer_op);

    m_OpenExportFolder = new wxCheckBox(m_OptionPanel, ID_OPEN_FOLDER_CHECKBOX, _("Open export folder in a file browser"), wxDefaultPosition, wxDefaultSize, 0);
    m_OpenExportFolder->SetValue(CGuiRegistry::GetInstance().GetBool(kExportOpenFolderTag, true));
    m_OpenExportFolder->Show(false);
    itemBoxSizer_op->Add(m_OpenExportFolder, 0, wxALIGN_RIGHT | wxRIGHT, 5);

    m_Splitter->InsertToCell(m_OptionList, 0, 0);
    m_Splitter->InsertToCell(m_OptionPanel, 1, 0);
    //m_ExplainLink->SetVisitedColour(m_ExplainLink->GetNormalColour());
}


void COpenDlg::SetServiceLocator(IServiceLocator* srv_locator)
{
    m_SrvLocator = srv_locator;
}


void COpenDlg::SetManagers(vector<TManagerRef>& managers)
{
    _ASSERT(m_SrvLocator);

    /// disconnect existing managers
    for( size_t i = 0;  i < m_Managers.size();  i++ ) {
        TManagerRef manager = m_Managers[i];
        manager->CleanUI();
        manager->SetServiceLocator(NULL);
        manager->SetParentWindow(NULL);
    }

    m_Managers = managers;

    // initialize the new managers
    for( size_t i = 0;  i < m_Managers.size();  i++ ) {
        TManagerRef manager = m_Managers[i];
        manager->SetServiceLocator(m_SrvLocator);
        manager->SetParentWindow(this);
        manager->InitUI();
        x_AppendOptionForManager(*manager);
    }

    //if(m_Managers.size())   {
     //   x_SelectManager(0);
    //}
}


void COpenDlg::SetFilenames(const vector<wxString>& filenames)
{
    m_Filenames = filenames;
}


void COpenDlg::SelectOptionByLabel(const string& label)
{
    for( size_t i = 0;  i < m_Managers.size();  i++ ) {
        TManagerRef& manager = m_Managers[i];
        string s = manager->GetDescriptor().GetLabel();
        if(s == label)    {
            m_RequestedManager = (int)i;
            break;
        }
    }
}


void COpenDlg::Update()
{
    x_UpdateButtons();
    x_UpdateTitle();
}


void COpenDlg::InitDialog()
{
    wxDialog::InitDialog();

    LoadSettings();

    for(  size_t i = 0;  i < m_Managers.size();  i++ )  {
        TManagerRef manager = m_Managers[i];

        CFileLoadManager* file_mgr = dynamic_cast<CFileLoadManager*>(manager.GetPointer());
        if(file_mgr)   {
            file_mgr->SetDialog(this);

            if( ! m_Filenames.empty())  {
                // D&D happened - activate the Manager for "File" option
                m_CurrManager = -1; // to force update
                x_SelectManager((int)i); // make it active
                file_mgr->SetFilenames(m_Filenames);
                return;
            }
        }
    }

    // select the default manager
    int index = max(m_CurrManager, 0); // do not allow -1
    if(m_RequestedManager == -1)    {
        /// select the manager based on saved settings
        m_CurrManager = -1;     // to force update
        x_SelectManager(index);
    } else {
        /// select the requested option (ignore settings)
        m_CurrManager = -1;     // to force update
        x_SelectManager(m_RequestedManager);
    }
}


CIRef<IAppTask> COpenDlg::GetLoadingTask()
{
    return m_AppTask;
}


bool COpenDlg::ShowToolTips()
{
    return true;
}


wxBitmap COpenDlg::GetBitmapResource( const wxString& name )
{
    // Bitmap retrieval
////@begin COpenDlg bitmap retrieval
    wxUnusedVar(name);
    return wxNullBitmap;
////@end COpenDlg bitmap retrieval
}

wxIcon COpenDlg::GetIconResource( const wxString& name )
{
    // Icon retrieval
////@begin COpenDlg icon retrieval
    wxUnusedVar(name);
    return wxNullIcon;
////@end COpenDlg icon retrieval
}


void COpenDlg::x_SelectManager(int index)
{
    if(m_CurrManager != index)  {
        x_SetCurrentPanel(NULL, true);

        m_CurrManager = index;
        m_OptionList->SetSelection(m_CurrManager);
        m_OptionList->Update();

        if(m_CurrManager != -1) {
            wxBusyCursor wait;
            wxPanel* panel = m_Managers[m_CurrManager]->GetCurrentPanel();
            x_SetCurrentPanel(panel, true);
        }

        x_UpdateTitle();
        x_UpdateButtons();
    }
}


// changes the panel displayed in the dialog
void COpenDlg::x_SetCurrentPanel(wxPanel* panel, bool firstPage)
{
    // remove the old panel
    if(m_CurrPanel) {
        m_CurrPanel->Hide();
        m_CurrPanel = NULL;
    }

    m_Splitter->Show(firstPage);

    if (!panel)
        return;
    // insert the new panel

    if (firstPage) {
        if (panel->GetParent() != m_OptionPanel) {
            panel->Reparent(m_OptionPanel);
        }

        if(panel->GetContainingSizer() != m_OptionPanel->GetSizer())   {
            m_OptionPanel->GetSizer()->Insert(0, panel, 1, wxEXPAND | wxALL, 5);
        }

        m_OpenExportFolder->Show(m_BaseTitle == "Export");
    }
    else {
        if (panel->GetParent() != this) {
            panel->Reparent(this);
        }

        if (panel->GetContainingSizer() != GetSizer()) {
            GetSizer()->Insert(0, panel, 1, wxEXPAND | wxALL, 5);
        }
        m_OpenExportFolder->Show(false);
    }

    m_CurrPanel = panel;
    m_CurrPanel->Show();
    m_CurrPanel->SetFocus();

    firstPage ? m_OptionPanel->Layout() : this->Layout();
}


/// add a Loading Option (displayed in the list on the left) for the given Manager
void COpenDlg::x_AppendOptionForManager(IUIToolManager& manager)
{
    const IUIObject& descr = manager.GetDescriptor();
    m_OptionList->Append(wxString::FromUTF8(descr.GetLabel().c_str()));
}


void COpenDlg::OnOptionListSelected( wxCommandEvent& event )
{
    int index = m_OptionList->GetSelection();
    x_SelectManager(index);
}


void COpenDlg::OnSkipClick( wxCommandEvent& event )
{
    x_DoTransition(IUIToolManager::eSkip);
}


void COpenDlg::OnBackwardClick( wxCommandEvent& event )
{
    x_DoTransition(IUIToolManager::eBack);
}


void COpenDlg::OnForwardClick( wxCommandEvent& event )
{
    x_DoTransition(IUIToolManager::eNext);
}


void COpenDlg::OnListItemActivate( wxListEvent& event )
{
    x_DoTransition(IUIToolManager::eNext);
}


void COpenDlg::x_DoTransition(IUIToolManager::EAction action)
{
    TManagerRef manager = m_Managers[m_CurrManager];

    bool success = false;
    string errMsg;

    try {
        success = manager->DoTransition(action);
    }
    catch (const CException& e) {
        errMsg = e.GetMsg();
    }
    catch (const exception& e) {
        errMsg = e.what();
    }

    if (!errMsg.empty()) {
        wxMessageBox (ToWxString(errMsg), wxT("Error"), wxOK | wxICON_ERROR);
        return;
    }

    // if its file manager and table, hack in change??
    if (success) {
        if(manager->IsCompletedState()) {
            if (m_BaseTitle == "Export")
                CGuiRegistry::GetInstance()
                    .Set(kExportOpenFolderTag, m_OpenExportFolder->GetValue());

            // launch the task
            m_AppTask.Reset(manager->GetTask());

            // Report usage
            const IUIObject& descriptor = manager->GetDescriptor();
            const string& log_event = descriptor.GetLogEvent();
            if (!log_event.empty()) {
                REPORT_USAGE(log_event, .Add("tool_name", descriptor.GetLabel()));
            }
            
            EndModal(wxID_OK);
        } else {
            /// show the panel for the next step
            wxPanel* new_panel = manager->GetCurrentPanel();
            if(new_panel != m_CurrPanel)    {
                x_SetCurrentPanel(new_panel, !manager->CanDo(IUIToolManager::eBack));
            }
            x_UpdateButtons();
        }
    }
    else {        
        // For one of the manager - file loader - it may recognize a file type
        // (bam or table) but not be able to load it with one of its file loading
        // wizards.  In that case, it sets an alternate tool manager from the 
        // set of open dialog managers in m_Managers and returns that as 
        // the 'algernate tool manager'  This code switches to the alternate tool
        // manager and sets filenames so the user doesn't have to type them
        // in again.
        IUIToolManager*  alt_mgr = manager->GetAlternateToolManager();
        if (alt_mgr != NULL && alt_mgr != manager.GetPointer()) {
            for (size_t i=0; i<m_Managers.size(); ++i) {
                if (m_Managers[i].GetPointer() == alt_mgr) {
                    CFileLoadManager* flm = dynamic_cast<CFileLoadManager*>(
                        m_Managers[m_CurrManager].GetPointer());
                    
                    if (flm != NULL) {
                        vector<wxString> fnames = flm->GetFilenames();

                        //CBamUIDataSource* blm = dynamic_cast<CBamUIDataSource*>(alt_mgr);
                        //if (blm != NULL) {
                            // Initialize bam files here
                        //}
                    } 
                    x_SelectManager(i);

                    break;
                }
            }
        }
    }
}


void COpenDlg::x_UpdateButtons()
{
    TManagerRef manager = m_Managers[m_CurrManager];

    bool can_back = manager->CanDo(IUIToolManager::eBack);
    //m_OptionList->Show( ! can_back);

    m_BackBtn->Show(can_back);
    m_NextBtn->Show(manager->CanDo(IUIToolManager::eNext));
    m_NextBtn->SetLabel(manager->IsFinalState() ? wxT("Finish") : wxT("Next >"));
    Layout();
}


void COpenDlg::x_UpdateTitle()
{
    wxString title = m_BaseTitle;
    if(m_CurrManager > -1)  {
        TManagerRef manager = m_Managers[m_CurrManager];
        wxString s = ToWxString(manager->GetDescriptor().GetLabel());
        if( ! s.empty())   {
            title += wxT(" - ") + s;
        }
    }
    SetTitle(title);
}


void COpenDlg::EndModal(int ret_code)
{
    for (auto &manager: m_Managers) {
        manager->ResetState();
    }

    SaveSettings();

    // reset Managers
    for (auto &manager : m_Managers) {
        manager->CleanUI();
    }

    wxDialog::EndModal(ret_code);
}


void COpenDlg::SetRegistryPath(const string& path)
{
    CDialog::SetRegistryPath(path);

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


static const char* kSplitterWidthsTag = "SplitterWidths";
static const char* kSelOptionTag = "SelectedOption";

void COpenDlg::x_LoadSettings(const CRegistryReadView& view)
{
    // load splitter configuration
    vector<int> widths;
    view.GetIntVec(kSplitterWidthsTag, widths);
    if (widths.size() == 2)  {
        m_Splitter->SetWidths(widths);
    }

    // load the default option
    string sel_mgr_label = view.GetString(kSelOptionTag, "empty");

    // load Managers' settings and find the selected manager
    for (size_t i = 0; i < m_Managers.size(); i++)  {
        TManagerRef manager = m_Managers[i];
        // load settings (if supported)
        IRegSettings* rgs = dynamic_cast<IRegSettings*>(manager.GetPointer());
        if (rgs) {
            rgs->LoadSettings();
        }
        // check whether this is the selected manager
        string mgr_label = manager->GetDescriptor().GetLabel();
        if (mgr_label == sel_mgr_label)  {
            m_CurrManager = (int)i; // found the selected manager
        }
    }
}

void COpenDlg::x_SaveSettings(CRegistryWriteView view) const
{
    // save splitter configuration
    vector<int> widths;
    m_Splitter->GetWidths(widths);
    view.Set(kSplitterWidthsTag, widths);

    // remember the selected option
    if (m_CurrManager != -1) {
        TManagerRef manager = m_Managers[m_CurrManager];
        string mgr_label = manager->GetDescriptor().GetLabel();
        view.Set(kSelOptionTag, mgr_label);
    }

    // save Managers' settigns
    for (size_t i = 0; i < m_Managers.size(); i++)  {
        TManagerRef manager = m_Managers[i];
        IRegSettings* rgs = dynamic_cast<IRegSettings*>(manager.GetPointer());
        if (rgs) {
            rgs->SaveSettings();
        }
    }
}

void COpenDlg::OnCancelClick( wxCommandEvent& event )
{
////@begin wxEVT_COMMAND_BUTTON_CLICKED event handler for wxID_CANCEL in COpenDlg.
    // Before editing this code, remove the block markers.
    event.Skip();
////@end wxEVT_COMMAND_BUTTON_CLICKED event handler for wxID_CANCEL in COpenDlg.
}


END_NCBI_SCOPE


