/*  $Id: data_mining_panel.cpp 39666 2017-10-25 16:01:13Z katargir $
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
 *  and reliability of the software and m_Data, the NLM and the U.S.
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
 * Authors:  Andrey Yazhuk, Vladimir Tereshkov
 *
 * File Description:
 *
 */

#include <ncbi_pch.hpp>

#include <gui/core/data_mining_panel.hpp>

#include <gui/core/data_mining_view.hpp>
#include <gui/core/data_mining_service.hpp>
#include <gui/core/view_extensions.hpp>
#include <gui/framework/workbench.hpp>

#include <gui/widgets/wx/wx_utils.hpp>
#include <gui/widgets/wx/fileartprov.hpp>
#include <gui/widgets/wx/message_box.hpp>

#include <gui/utils/view_event.hpp>
#include <gui/widgets/wx/sys_path.hpp>
#include <gui/utils/menu_item.hpp>

//TODO - remove
#include <gui/framework/app_task_service.hpp>
#include <gui/framework/workbench.hpp>
#include <objects/gbproj/ProjectItem.hpp>
#include <gui/objutils/entrez_conversion.hpp>
#include <gui/objutils/utils.hpp>
#include <gui/objutils/label.hpp>
#include <gui/objutils/gencoll_svc.hpp>

#include <objects/entrezgene/Entrezgene.hpp>
#include <objects/seqfeat/Seq_feat.hpp>
#include <objects/seq/Annot_descr.hpp>
#include <objects/seq/Annotdesc.hpp>
#include <objects/variation/Variation.hpp>
#include <objects/variation/VariantPlacement.hpp>

#include <gui/framework/view_manager_service.hpp>
#include <objmgr/object_manager.hpp>

#include <gui/widgets/seq/feature_create_dlg.hpp>
#include <gui/widgets/wx/simple_clipboard.hpp>
#include <gui/core/filter_dialog.hpp>

#include <gui/widgets/wx/ui_command.hpp>
#include <gui/widgets/wx/wx_utils.hpp>
#include <gui/widgets/wx/popup_event.hpp>

#include <objmgr/seq_vector.hpp>
#include <objmgr/util/sequence.hpp>
#include <gui/framework/menu_service.hpp>
//TODO

#include <wx/app.h>
#include <wx/sizer.h>
#include <wx/panel.h>
#include <wx/menu.h>
#include <wx/button.h>

#include <wx/animate.h>
#include <wx/collpane.h>
#include <wx/statline.h>
#include <wx/tglbtn.h>
#include <wx/stattext.h>
#include <wx/toolbar.h>
#include <wx/hyperlink.h>
#include <wx/srchctrl.h>
#include <wx/choice.h>

#include <algorithm>

BEGIN_NCBI_SCOPE
USING_SCOPE(objects);

#define ID_TABLE    10001
#define ID_TOOLBAR  10002
#define ID_COMBOBOX 10003
#define ID_TEXT     11414
#define ID_BUTTON   10005
#define ID_DB       10006
#define ID_ANIMATIONCTRL1   10007
#define ID_COMBOBOX1 10008
#define ID_BUTTON1 10009
#define ID_COMBOBOX_UPDATE 10111

#define ID_HYPERLINKCTRL 10999

#define TIMER_ID    -1



BEGIN_EVENT_TABLE( CDataMiningPanel, wxPanel )
    EVT_CONTEXT_MENU( CDataMiningPanel::OnContextMenu )
    EVT_CHOICE( ID_COMBOBOX1, CDataMiningPanel::OnTargetChanged)
    EVT_CHOICE( ID_COMBOBOX_UPDATE, CDataMiningPanel::OnUpdateNeeded)

    EVT_MENU(eCmdAddToNew, CDataMiningPanel::OnAddToNew)

    EVT_MENU(eCmdCreateFeature, CDataMiningPanel::OnCreateFeature)
    EVT_UPDATE_UI(eCmdCreateFeature, CDataMiningPanel::OnUpdateCreateFeature )

    EVT_MENU( IDMSearchForm::eCmdDMSearch, CDataMiningPanel::OnSearchButton)
    EVT_MENU_RANGE(eCmdCopy1, eCmdCopy10,  CDataMiningPanel::OnCustomCopy)
    EVT_UPDATE_UI_RANGE(eCmdCopy1, eCmdCopy10, CDataMiningPanel::OnEnableCmdRclick )

    EVT_MENU(wxID_ANY, CDataMiningPanel::OnMenuEvent )

    EVT_UPDATE_UI(eCmdAddToNew, CDataMiningPanel::OnEnableCmdRclick )

    EVT_TOOL_RANGE(scm_ModeCmd + eCmdAddToNew, scm_ModeCmd + eCmdNoMoreCommands, CDataMiningPanel::HandleTool)

    EVT_LIST_ITEM_SELECTED(wxID_ANY, CDataMiningPanel::OnSelectionChanged)
    EVT_LIST_ITEM_DESELECTED(wxID_ANY, CDataMiningPanel::OnSelectionChanged)

    EVT_HYPERLINK( ID_HYPERLINKCTRL, CDataMiningPanel::OnHyperlinkctrlClicked )
    EVT_TEXT_ENTER(wxID_ANY, CDataMiningPanel::OnButtonClicked)

    EVT_LIST_COL_END_DRAG(wxID_ANY, CDataMiningPanel::OnColumnEndDrag)
    EVT_LIST_ITEM_ACTIVATED(wxID_ANY, CDataMiningPanel::OnListItemActivate)

    EVT_TIMER(TIMER_ID, CDataMiningPanel::OnAutorun)

END_EVENT_TABLE()



CDataMiningPanel::CDataMiningPanel(CDataMiningView* view)
:   m_View(view),
    m_bInitialized(false),
    m_ParentPanel(NULL),
    m_Timer(this, TIMER_ID),
    mf_Reentry(false),
    m_SearchInProgress(false),
    m_NeedUpdate(true)
{
    Init();
}


CDataMiningPanel::~CDataMiningPanel()
{
}


void CDataMiningPanel::x_SyncTableSettings(bool bWrite) const
{
    if (m_CurrToolName.empty())
        return;

    string toolID = m_CurrToolName;
    replace(toolID.begin(), toolID.end(), ' ', '-');

    vector <string> subsections;
    subsections.push_back(toolID);


    if( ! m_RegPath.empty() && m_ListWidget && m_ListWidget->GetItemCount() > 0) {
        if (bWrite) {
            CRegistryWriteView view = CGuiRegistry::GetInstance().GetWriteView(m_RegPath, subsections);
            m_ListWidget->SaveTableSettings(view);
        }
        else {
            m_ListWidget->LoadTableSettings(CGuiRegistry::GetInstance().GetReadView(m_RegPath, subsections));
        }
    }

}

void CDataMiningPanel::OnColumnEndDrag( wxListEvent &anEvt )
{
    x_SyncTableSettings(true);
}

// will handle row dbl-click and enter
void CDataMiningPanel::OnListItemActivate(wxListEvent &anEvt)
{
    TConstScopedObjects objs;
    GetSelection(objs);
    x_AddToProject(objs);
}

//TODO this function should be here
void CDataMiningPanel::OnHyperlinkctrlClicked( wxHyperlinkEvent& event )
{
    m_Form->PopupADialog(IDMSearchForm::FEATURE_SEL_DIALOG);
    m_Form->Update();
}


void CDataMiningPanel::OnUpdateNeeded(wxCommandEvent& event)
{
    m_Form->Update();
}


void CDataMiningPanel::Create( wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size )
{
    #ifdef __WXMAC__
    //  Without this search view (especially NCBI Net Blast) becomes messed up
    //  when you open the 'open view dialog'.  Also, the area below the search bar
    //  turns black when you un-dock the search view from the application. GB-6515
    SetBackgroundStyle(wxBG_STYLE_COLOUR);
    SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_FRAMEBK));
    #endif

    wxPanel::Create(parent, id, pos, size);
    CreateControls();

    #ifndef __WXMAC__
        PushEventHandler( new CCommandToFocusHandler( this ) );
    #endif
}

bool CDataMiningPanel::Destroy()
{
    #ifndef __WXMAC__
        PopEventHandler( true );
    #endif

    return wxPanel::Destroy();
}

void CDataMiningPanel::Init()
{
    m_Service   = NULL;
    m_Workbench = NULL;

    m_CurrToolName  = "";
    m_MainValue     = "";

    m_TargetChoice  = NULL;
    m_ListWidget    = NULL;
    m_FormSizer     = NULL;
    m_Text          = NULL;

    m_FormSizer     = NULL;
    m_ToolBar       = NULL;

    m_Dict.clear();

    if( ! m_bInitialized ){
        wxFileArtProvider* provider = GetDefaultFileArtProvider();

        provider->RegisterFileAlias(wxT("menu::dm_start"),  wxT("playhs.png"));
        provider->RegisterFileAlias(wxT("menu::dm_stop"),   wxT("stophs.png"));
        provider->RegisterFileAlias(wxT("menu::dm_query"),  wxT("expandspacehs.png"));
        provider->RegisterFileAlias(wxT("menu::dm_range"),  wxT("range.png"));
        provider->RegisterFileAlias(wxT("menu::dm_filter"), wxT("filter.png"));
        provider->RegisterFileAlias(wxT("menu::dm_open"),   wxT("openselecteditemhs.png"));

        m_bInitialized = true;
    }
}


void CDataMiningPanel::OnSelectionChanged(wxListEvent& event)
{
    CEvent evt(CEvent::eEvent_Message, CViewEvent::eWidgetSelectionChanged);
    Send(&evt, ePool_Parent);

    //CEvent range(CEvent::eEvent_Message, CViewEvent::eWidgetRangeChanged);
   // Send(&range, ePool_Parent);
}

void CDataMiningPanel::CreateControls()
{
    // main sizer
    m_ParentPanel = new wxPanel(this);

    wxBoxSizer* main_sizer = new wxBoxSizer(wxVERTICAL);
    m_ParentPanel->SetSizer(main_sizer);

    // toolbar
    long styles = wxTB_FLAT | wxTB_HORIZONTAL | wxTB_TEXT | wxTB_HORZ_LAYOUT;
    m_ToolBar = new wxToolBar(m_ParentPanel, ID_TOOLBAR,
                              wxDefaultPosition, wxSize(-1, -1), styles);

    // label
    wxStaticText* tool_text = new wxStaticText( m_ToolBar, wxID_STATIC,
                                                wxT("   Search Tool: "));
    m_ToolBar->AddControl(tool_text);

    // combo
    m_TargetChoice = new  wxChoice(m_ToolBar, ID_COMBOBOX1,
                                     wxDefaultPosition, wxSize(180,-1),
                                     0, (const wxString*)NULL);
    m_ToolBar->AddControl(m_TargetChoice);


    // buttons
    wxBitmap bmp_start = wxArtProvider::GetBitmap(wxT("menu::dm_start"));
    m_ToolBar->AddTool(scm_ModeCmd + eCmdStart, wxT("Start"), bmp_start, wxT("Start Search"));

    wxBitmap bmp_stop = wxArtProvider::GetBitmap(wxT("menu::dm_stop"));
    m_ToolBar->AddTool(scm_ModeCmd + eCmdStop, wxT("Stop"), bmp_stop, wxT("Stop Search"));
    m_ToolBar->EnableTool(scm_ModeCmd + eCmdStop, false);

    m_Animation = new wxAnimationCtrl(m_ToolBar, ID_ANIMATIONCTRL1, wxAnimation(),
                                      wxDefaultPosition, wxSize(22, 22));
//    m_Animation->SetInactiveBitmap(wxNullBitmap);

    wxString path = CSysPath::ResolvePath(wxT("<res>\\status_anim.gif"));
    m_Animation->LoadFile(path);
    m_ToolBar->AddControl(m_Animation);

    m_ToolBar->AddSeparator();

    wxBitmap bmp_range = wxArtProvider::GetBitmap(wxT("menu::dm_range"));
    m_ToolBar->AddCheckTool(scm_ModeCmd + eCmdRangeLimit, wxT("Range"), bmp_range,
                          wxNullBitmap, wxT("Limit searches by selected range when possible"));
    m_ToolBar->ToggleTool(scm_ModeCmd + eCmdRangeLimit, false);

    wxBitmap bmp_filter = wxArtProvider::GetBitmap(wxT("menu::dm_filter"));
    m_ToolBar->AddCheckTool(scm_ModeCmd + eCmdFilter, wxT("Filter"), bmp_filter,
                          wxNullBitmap, wxT("Filter search results"));
    m_ToolBar->ToggleTool(scm_ModeCmd + eCmdFilter, false);

    m_ToolBar->AddSeparator();

    wxBitmap bmp_query = wxArtProvider::GetBitmap(wxT("menu::dm_query"));
    m_ToolBar->AddCheckTool(scm_ModeCmd + eCmdShowPanel, wxT("Form"), bmp_query,
                          wxNullBitmap, wxT("Show / Hide query pane"));
    m_ToolBar->ToggleTool(scm_ModeCmd + eCmdShowPanel, true);

    m_ToolBar->Realize();
    main_sizer->Add( m_ToolBar, 0, wxEXPAND );

    // form pane
    m_FormSizer = new wxBoxSizer(wxVERTICAL);
    main_sizer->Add(m_FormSizer, 0, wxGROW|wxALL, 0);


    // wxStatusBar
    m_Status = new wxStatusBar(m_ParentPanel, wxID_ANY, 0);
    main_sizer->Add(m_Status, 0, wxEXPAND);


    // separation line before the table
    wxStaticLine* line = new wxStaticLine(m_ParentPanel, wxID_STATIC, wxDefaultPosition, wxDefaultSize, wxLI_HORIZONTAL );
    main_sizer->Add(line, 0, wxEXPAND);


    m_ListWidget = new CObjectListWidget(m_ParentPanel, ID_TABLE, wxDefaultPosition, wxDefaultSize, wxBORDER_NONE );
    m_ListWidget->SetContextMenuEventPropagation( true );
    m_ListWidget->AssignModel( new CObjectListTableModel() );

    main_sizer->Add(m_ListWidget, 1, wxEXPAND);

    wxBoxSizer* frame_sizer = new wxBoxSizer(wxHORIZONTAL);
    frame_sizer->Add(m_ParentPanel, 1, wxALL|wxGROW, 0);
    SetSizerAndFit(frame_sizer);

    m_FixGenericListCtrl.ConnectToControl(*m_ListWidget);
}


void CDataMiningPanel::OnButtonClicked(wxCommandEvent& event)
{
    if (m_ToolBar->GetToolEnabled(scm_ModeCmd + eCmdStart)) {
        x_ExecuteSearch();
        x_SearchInProgress(true);
    }
    event.Skip();
}


void CDataMiningPanel::x_ExecuteSearch()
{
    if (m_Service) {
        x_SyncTableSettings(true);

        string tool_name = ToStdString(m_TargetChoice->GetStringSelection());
        CIRef<IDMSearchTool> tool = m_Service->GetToolByName(tool_name);

        CIRef<IDMSearchQuery> query = m_Form->ConstructQuery();

        string query_str = query->ToString();
        if (query_str.empty()) {
            LOG_POST(Info << "DataMining Filter for " << tool_name);
        } else {
            LOG_POST(Info << "DataMining Search Execute for " << tool_name
                          << "\n QUERY: " << query_str );
        }

        tool->StartSearch(*query, *this);
        m_Form->Push();
    }
}


void CDataMiningPanel::x_CancelSearch()
{
    if (m_Service) {
        string tool_name = ToStdString(m_TargetChoice->GetStringSelection());
        CIRef<IDMSearchTool> tool = m_Service->GetToolByName(tool_name);
        if( tool ){
            tool->CancelSearch(*this);
        }
    }
}


void CDataMiningPanel::HandleTool(wxCommandEvent& event)
{
    switch(event.GetId() - scm_ModeCmd){
    case eCmdShowPanel:
        if ( ! m_FormSizer->GetChildren().empty()) {
            if (event.IsChecked()) {
                m_FormSizer->Show(size_t(0));
            } else {
                m_FormSizer->Hide(size_t(0));
            }
            m_ParentPanel->Layout();
            m_ListWidget->Refresh();
        }
        break;
    case eCmdRangeLimit:
        if (m_Form) {
            m_Form->SetRangeLimit(event.IsChecked());
        }
        break;
   case eCmdFilter:
        if (m_Form) {
             string tool_name = ToStdString(m_TargetChoice->GetStringSelection());
             CIRef<IDMSearchTool> tool = m_Service->GetToolByName(tool_name);

             CFilterDialog dlg(NULL);
             dlg.GetFilters() = tool->GetFilters();

             m_ToolBar->ToggleTool(scm_ModeCmd + eCmdFilter, !event.IsChecked());

             if (dlg.ShowModal() == wxID_OK) {
                tool->SetFilters() = dlg.GetFilters();
                m_ToolBar->ToggleTool(scm_ModeCmd + eCmdFilter, !dlg.GetFilters().selected.empty());

                // if restart possible, do it
                if (m_ToolBar->GetToolEnabled(scm_ModeCmd + eCmdStart)) {
                    x_RestartSearch();
                }
             }
        }
        break;
    case eCmdStart:
        x_CancelSearch();
        x_ExecuteSearch();
        x_SearchInProgress(true);
        break;

    case eCmdStop:
        x_CancelSearch();
        x_SearchInProgress(false);
        break;
    };
}

void CDataMiningPanel::OnCustomCopy(wxCommandEvent& event)
{
    int column = event.GetId() - eCmdCopy1;
    wxString clip_str;

    if (m_ListWidget->GetItemCount() && m_ListWidget->GetSelectedItemCount()) {
        long sel = m_ListWidget->GetFirstSelected();
        if (sel!=-1) {
            clip_str = m_ListWidget->GetValueVisibleAt(sel, column).GetString();
        }
    }

    if( !clip_str.IsEmpty() ){
        CSimpleClipboard::Instance().Set( clip_str );
    }
}


CDataMiningService* CDataMiningPanel::GetDataMiningService()
{
    return m_Service;
}


void CDataMiningPanel::OnSearchFinished(CDMSearchResult& result)
{
    x_SearchInProgress(false);

    string strMsg = "Search is completed at ";

    try {
        CObjectListTableModel* oltm = result.DetachOLTModel();
        CObjectList * ol = result.GetObjectList();

		if (ol) {
			if (oltm == NULL){
				m_ListWidget->Init(*ol);
			} else {
				m_ListWidget->Init(*ol, oltm);
			}

			x_SyncTableSettings(false);

			strMsg +=  (NStr::IntToString(ol->GetNumRows()) + " items.");
            if (result.IsIncomplete()) {
                strMsg += " Total found: " + NStr::NumericToString(result.GetMaxSearchResult());
                strMsg += ".";
            }
		}
    }
    catch (CException& e) {
        LOG_POST(Error << "CDataMiningPanel::OnSearchFinished(): OLW Error: " << e.GetMsg());
    }
    catch (std::exception& e) {
        LOG_POST(Error << "CDataMiningPanel::OnSearchFinished(): Unexpected OLW Error" << e.what());
    }

    m_Status->SetStatusText(ToWxString(strMsg));
}

void CDataMiningPanel::OnSearchProgress(CDMSearchResult& result, const string& prg_text)
{
    m_Status->SetStatusText(ToWxString(prg_text));
    m_Workbench->GetMainWindow()->SendSizeEvent();
}


void CDataMiningPanel::OnSearchCanceled()
{
    m_Status->SetStatusText(wxT("Search has been cancelled"));
    x_SearchInProgress(false);
}


void CDataMiningPanel::OnSearchFailed(const string& error)
{
    m_Status->SetStatusText(ToWxString(error));
    x_SearchInProgress(false);
}


void CDataMiningPanel::OnSearchEnabled(bool benable)
{
    if (!m_SearchInProgress)
        m_ToolBar->EnableTool(scm_ModeCmd + eCmdStart, benable);
}


void CDataMiningPanel::x_SearchInProgress(bool bYes)
{
    if (bYes) {
        m_Animation->Play();
        m_ToolBar->EnableTool(scm_ModeCmd + eCmdStart, false);
        m_ToolBar->EnableTool(scm_ModeCmd + eCmdStop, true);
        m_Status->SetStatusText(wxT("Please wait, search is in progress..."));

    } else {
        m_Animation->Stop();
        m_ToolBar->EnableTool(scm_ModeCmd + eCmdStart, true);
        m_ToolBar->EnableTool(scm_ModeCmd + eCmdStop, false);
        m_Status->SetStatusText(wxT("Ready"));
    }

    m_SearchInProgress = bYes;

    // This forces a window update (otherwise widgets look bad on mac)
    if (m_Workbench)
        m_Workbench->GetMainWindow()->SendSizeEvent();
}


void CDataMiningPanel::SetService(CDataMiningService* service)
{
    if (service == NULL) {
        // this is part of PreDetach, at this point we should cancel ongoing background search job
        x_CancelSearch();
    }

    m_Service = service;

    if(m_CurrToolName.empty())  {
        // select the first tool
        vector<string> names;
        m_Service->GetToolNames(names);
        if( ! names.empty())    {
            m_CurrToolName = names[0];
        }
    }
}


void CDataMiningPanel::SetRegistryPath(const string& reg_path)
{
    m_RegPath = reg_path; // store for later use
}


static const char *  kSelectedTool = "SelectedTool";
static const char *  kAutocomplete = "Autocomplete";


void CDataMiningPanel::SaveSettings() const
{
    if( ! m_RegPath.empty() ){
        CGuiRegistry& gui_reg = CGuiRegistry::GetInstance();
        CRegistryWriteView view = gui_reg.GetWriteView(m_RegPath);

        view.Set(kSelectedTool, m_CurrToolName);
        view.Set(kAutocomplete, m_Dict);

        if (m_Form) {
            m_Form->SaveSettings();
        }

        x_SyncTableSettings(true);
    }
}


void CDataMiningPanel::LoadSettings()
{
    if( ! m_RegPath.empty() ){
        CGuiRegistry &          gui_reg = CGuiRegistry::GetInstance();
        CRegistryReadView view = gui_reg.GetReadView(m_RegPath);

        m_CurrToolName = view.GetString(kSelectedTool, m_CurrToolName);
        view.GetStringList(kAutocomplete, m_Dict);

        if (m_Form) {
            m_Form->LoadSettings();
        }

        x_SyncTableSettings(false);
    }
}


void CDataMiningPanel::GetSelection(TConstScopedObjects& buf) const
{
    m_ListWidget->GetSelection( buf );

    /*
    wxArrayInt objIxs = m_ListWidget->GetDataRowsSelected();

    for( int i = 0; i < (int)objIxs.size(); ++i ){
        int ix = objIxs[i];
        CObject* obj = ((CObjectListTableModel*)m_ListWidget->GetModel())->GetObject(ix);
        CScope* sc = ((CObjectListTableModel*)m_ListWidget->GetModel())->GetScope(ix);
        buf.push_back( SConstScopedObject(*obj, *sc) );
    }
    */
}

void CDataMiningPanel::AppendMenuItems( wxMenu& aMenu )
{
    aMenu.Append( wxID_SEPARATOR, wxT("Actions") );

    aMenu.Append(eCmdAddToNew, wxT("Add to Project..."));
    aMenu.Append(eCmdCreateFeature, wxT("Create Feature..."));
}



WX_DEFINE_MENU(kContextMenu)
    WX_MENU_SEPARATOR_L("Top Actions")
    WX_MENU_SEPARATOR_L("Actions")
WX_END_MENU()


void CDataMiningPanel::OnContextMenu( wxContextMenuEvent& anEvent )
{
    SetFocus();

    CUICommandRegistry& cmd_reg = m_Workbench->GetUICommandRegistry();

    //wxMenu* menu;

    // custom copy submenu
    wxMenu* ccMenu = NULL;


    //void* data = anEvent.GetClientData();
    //menu = data ? (wxMenu*)data : new wxMenu();
    wxMenu* menu = cmd_reg.CreateMenu(kContextMenu);

    // adding custom copy menu
    if (m_ListWidget->GetItemCount() && m_ListWidget->GetSelectedItemCount()) {
        ccMenu = new wxMenu();

        int cols = m_ListWidget->GetColumnCount();
        wxListItem col;
        for (int i=0; i<cols && i<10; i++) {
            const int mod_ix = m_ListWidget->ColumnIndexToModel(i);
            ccMenu->Append(eCmdCopy1 + mod_ix, m_ListWidget->GetColumnNameByModel(mod_ix));
        }
        menu->Append( wxID_ANY, wxT("Copy Column"), ccMenu);
    }


    AppendMenuItems( *menu );

    if( /*ShouldPropagate()*/ false ){
        anEvent.SetClientData( menu );
        anEvent.Skip();
        return;
    }

    anEvent.SetClientData( NULL );


    auto_ptr<wxMenu> main_menu( CreateContextMenuBackbone() );
    Merge( *main_menu.get(), *menu );

    // check for contributors menues
    CDataMiningService::TContributors contrib =
        m_Service->GetContributors();

    if (!contrib.empty()) {
        for( size_t i = 0;  i < contrib.size();  i++ )   {
            IDMContextMenuContributor& contributor = *contrib[i];
            auto_ptr<wxMenu> c_menu(contributor.GetContextMenu());
            Merge(*main_menu.get(), *c_menu);
        }
    }

    CleanupSeparators( *main_menu );
    PopupMenu( main_menu.get() );


    if (menu) {
        delete menu;
    }
}

void CDataMiningPanel::OnMenuEvent(wxCommandEvent& event)
{
    if( mf_Reentry ){
        event.Skip();
        return;
    }
    CBoolGuard _guard( mf_Reentry );

    if (!GetEventHandler()->ProcessEvent(event)) {
        // forward contributors if necessary
        CDataMiningService::TContributors contrib =
            m_Service->GetContributors();

            if (!contrib.empty()) {
                for( size_t i = 0;  i < contrib.size();  i++ )   {
                    IDMContextMenuContributor* contributor = contrib[i];
                    wxMenuItem * menu_item =
                        contributor->GetContextMenu()->FindItem(event.GetId());

                    if (menu_item) {
                        // check whether contributor is a handler
                        wxEvtHandler * contrib_handler =
                            dynamic_cast<wxEvtHandler *>(contributor);

                        IWMClient * contrib_win =
                            dynamic_cast<IWMClient *>(contributor);

                        bool bHandled = false;

                        if (contrib_handler) {
                            bHandled = contrib_handler->ProcessEvent(event);
                        }
                        else if (contrib_win) {
                            wxWindow* window = contrib_win->GetWindow();
                            _ASSERT(window);
                            bHandled = window->GetEventHandler()->ProcessEvent(event);
                        }
                        else {
                            bHandled = wxTheApp->GetTopWindow()->GetEventHandler()->ProcessEvent(event);
                        }

                        if (!bHandled) {
                            event.Skip();
                        }
                    }
                }
            }
    }
}


//TODO this a hack, this operation needs to be redesigned
void CDataMiningPanel::OnAddToNew(wxCommandEvent& event)
{
    if (m_Workbench) {
        TConstScopedObjects objs;
        GetSelection(objs);
        x_AddToProject(objs);
    }
}

void CDataMiningPanel::OnCreateFeature(wxCommandEvent& event)
{
    if (m_Workbench) {
        x_CreateFeature();
    }
}

void CDataMiningPanel::OnUpdateCreateFeature(wxUpdateUIEvent& event)
{
    if (!m_ListWidget || m_ListWidget->GetItemCount() == 0 ||
        m_ListWidget->GetSelectedItemCount() == 0) {
        event.Enable(false);
        return;
    }

    CIRef<IDMSearchTool> tool = m_Service->GetToolByName(m_CurrToolName);
    IDMSearchTool::TConversions& conv = tool->GetConversions();
    event.Enable(!conv.options.empty());
}


void CDataMiningPanel::OnEnableCmdRclick(wxUpdateUIEvent& event)
{
    event.Enable(m_ListWidget &&
        m_ListWidget->GetItemCount() && m_ListWidget->GetSelectedItemCount());
}



void CDataMiningPanel::x_CreateFeature()
{
    CFeatureCreateDialog dlg(NULL);
    TConstScopedObjects  in, out;
    GetSelection(in);

    CIRef<IDMSearchTool> tool = m_Service->GetToolByName(m_CurrToolName);
    IDMSearchTool::TConversions & conv = tool->GetConversions();
    string label    = tool->GetName() + " tool results";
    string comments = "";

    dlg.SetIO(in, out);
    dlg.SetConstraints(conv.options, conv.selected);
    dlg.SetLbl(ToWxString(label));
    if (dlg.ShowModal() == wxID_OK) {
        x_AddToProject(out,
                       ToStdString(dlg.GetLbl()),
                       ToStdString(dlg.GetComments()));
    }
}


void CDataMiningPanel::x_AddToProject(TConstScopedObjects & objs, string label, string comment)
{
    CIRef<CProjectService> prjService = m_Workbench->GetServiceByType<CProjectService>();

    CGBankLoadingJob* job = new CGBankLoadingJob(objs, label, comment);

    CSelectProjectOptions options;
    options.Set_CreateNewProject("Search Results");
    options.Set_DecideLater();

    CDataLoadingAppTask* task = new CDataLoadingAppTask(prjService, options, *job, "Add to Project");
    m_Workbench->GetAppTaskService()->AddTask(*task);
}


void CDataMiningPanel::OnSearchButton(wxCommandEvent& event)
{
    x_RestartSearch();
}


void CDataMiningPanel::OnTargetChanged(wxCommandEvent& event)
{
    x_TargetChanged();
}

void CDataMiningPanel::x_TargetChanged()
{
    x_SyncTableSettings(true);
    x_UpdateWidgetAndTool();
}

void CDataMiningPanel::SetWorkbench(IWorkbench* workbench)
{
    m_Workbench = workbench;
}


void CDataMiningPanel::SelectToolByName(const string& tool_name)
{
    vector<string> names;
    m_Service->GetToolNames(names);

    for(  size_t i = 0;  i < names.size();  i++ )   {
        if(names[i] == tool_name)   {
            m_TargetChoice->SetStringSelection(ToWxString(tool_name));
            x_TargetChanged();
            return;
        }
    }
    LOG_POST(Error << "CDataMiningPanel::SelectTool() - invalid tool name " << tool_name);
}


CDataMiningPanel::EDisplayMode CDataMiningPanel::x_GetModeByCommand(TCmdID cmd)
{
    return EDisplayMode(cmd - scm_ModeCmd);
}


void CDataMiningPanel::x_Init()
{
    // register icons only once
    /*
    if( ! m_bInitialized ){
        wxFileArtProvider* provider = GetDefaultFileArtProvider();

        provider->RegisterFileAlias("menu::insp_table_mode", "insp_table_mode.png");
        provider->RegisterFileAlias("menu::insp_brief_text_mode", "insp_brief_text_mode.png");
        provider->RegisterFileAlias("menu::insp_text_mode", "insp_text_mode.png");

        m_bInitialized = true;
    }
    */
}

void CDataMiningPanel::x_InitCurrentWidget()
{
    //if( m_DisplayMode == eTable ){
      //  m_ListWidget->Init( m_SelObjects );

    //} else {
    //    m_TextWidget->Init(m_SelObjects, m_DisplayMode == eBriefText);
    //}
    LoadSettings();
}


void CDataMiningPanel::Update()
{
    m_NeedUpdate = true;
    if (!m_Timer.IsRunning())
        m_Timer.Start(500, wxTIMER_ONE_SHOT);
}


void CDataMiningPanel::OnAutorun(wxTimerEvent& event)
{
    if (m_NeedUpdate) {
        m_NeedUpdate = false;

        m_TargetChoice->Clear();
        if(m_Service)   {
            // fill the list
            vector<string> names;
            m_Service->GetToolNames(names);

            ITERATE(vector<string>, name, names){
                m_TargetChoice->Append(ToWxString(*name));
            }
            m_TargetChoice->SetStringSelection(ToWxString(m_CurrToolName));
        }
        x_UpdateWidgetAndTool();
    }
    else
        x_ExecuteSearch();
}

void CDataMiningPanel::x_UpdateWidgetAndTool()
{
	if (m_Service) {
        string tmp = ToStdString(m_TargetChoice->GetStringSelection());
		if (m_CurrToolName != tmp) {

            if( !tmp.empty() ){
			    m_CurrToolName = tmp;
            }

            CIRef<IDMSearchTool> tool = m_Service->GetToolByName(m_CurrToolName);

            m_ListWidget->Clear();

            // check whether try looking into cache
            if( !tool.IsNull() ){
                x_UpdateWidget();

                if( tool->GetFlags() & IDMSearchTool::eCache ){
                    // autorun and cache are not compatible
                    if( tool->GetFlags() & IDMSearchTool::eAutorun ){
                        LOG_POST(Error << "CDataMiningPanel - Autorun ignored for tool: " << m_CurrToolName);
                    }
                    else {
                        try {
                            CIRef<IDMSearchQuery> query = m_Form->ConstructQuery();
                            tool->PullSearch( *query, *this );
                        }
                        catch( std::exception& e ){
                            LOG_POST(Error << "CDataMiningPanel - Attempt to retreive cached results for a tool failed." << e.what());
                            x_CancelSearch();
                        }
                    }
                }

                // check for autorun flag and launch tool as needed
                if ((tool->GetFlags() & IDMSearchTool::eAutorun) &&
                    !m_Timer.IsRunning()) {
                    m_Timer.Start(1000, wxTIMER_ONE_SHOT);
                }
            }
		}

    }
    x_UpdateWidget();
}


void CDataMiningPanel::x_UpdateWidget()
{
#ifndef __WXMAC__
    Freeze();
#endif

    bool bShown = true;

    if ( ! m_FormSizer->GetChildren().empty()) {
        bShown = m_FormSizer->IsShown(size_t(0));
    }

    if (m_Form) {
        m_Form->SaveSettings();
        string mvalue = m_Form->GetMainValue();
        if (mvalue!="#empty#") {
            m_MainValue = mvalue;
        }
    }

    m_FormSizer->Clear(true);

    CIRef<IDMSearchTool> tool = m_Service->GetToolByName(m_CurrToolName);
    m_Form = tool->CreateSearchForm();
    m_Form->SetController(this);
    m_Form->SetRegistryPath(m_RegPath);
    m_Form->Init();

    // reset buttons
    if ( ! m_Animation->IsPlaying())    {
        x_SearchInProgress(false);
    }

    // refresh contexts
    m_Form->UpdateContexts();

    m_Form->LoadSettings();

    wxSizer * dynamicForm = m_Form->GetWidget(m_ParentPanel);
    if (m_Form && dynamicForm) {
        m_FormSizer->Add(dynamicForm, 0, wxGROW|wxLEFT|wxRIGHT|wxBOTTOM, 5);
        if ( ! bShown) {
            m_FormSizer->Hide(size_t(0));
        }
    }

    // enable auto-complete
    m_Form->SetDictionary(&m_Dict);

    // post-process form
    m_Form->Update();

    // preserve main search value
    m_Form->SetMainValue(m_MainValue);

    // enable/disable filter
    m_ToolBar->EnableTool(scm_ModeCmd + eCmdFilter,
        (tool->GetFlags() & IDMSearchTool::eFilter));

    // layout
    m_ParentPanel->Layout();

#ifndef __WXMAC__
    Thaw();
#endif
}


void CDataMiningPanel::x_RestartSearch()
{
    x_CancelSearch();
    x_ExecuteSearch();
    x_SearchInProgress(true);
}

void CDataMiningPanel::SetRange(objects::CSeq_loc::TRange & range)
{
    m_Range = range;
    //m_ToolBar->GetToolState(scm_ModeCmd + eCmdRangeLimit)

    string status_text = "Ready";
    if (m_Range.GetLength()) {
        status_text = "Visible range selected: [" + NStr::IntToString(m_Range.GetFrom()) +
                             " - " + NStr::IntToString(m_Range.GetTo()) + "]";
    }
    m_Status->SetStatusText(ToWxString(status_text));
}


///////////////////////////////////////////////////////////////////////////////
/// CGBankLoadingJob
CGBankLoadingJob::CGBankLoadingJob(TConstScopedObjects& ids,
                                   string label, string comment)
:   m_Objs(ids),
    m_Label(label),
    m_Comment(comment)
{
}

void CGBankLoadingJob::x_CreateProjectItems()
{
    CRef<CSeq_annot>     annot;
    CRef<CProjectItem>   item;
    CRef<CProjectItem>   sl_item;


    // possible case
//  CRef<CSeq_annot> annot(new CSeq_annot());

    // iterate by ids and create items
    for ( size_t i = 0;  i < m_Objs.size();  i++  )   {
        // create a Project Item
        //CRef<CProjectItem> item(new CProjectItem());
        // TODO: do we need support for multiple scopes here?
        if (!m_Scope) {
            m_Scope = m_Objs[i].scope;
        }

        const CXmlNodeObject * docsum =
            dynamic_cast<const CXmlNodeObject*>(m_Objs[i].object.GetPointer());

        const objects::CSeq_loc * seq_loc =
            dynamic_cast<const objects::CSeq_loc*>(m_Objs[i].object.GetPointer());

        const objects::CSeq_feat * seq_feat =
            dynamic_cast<const objects::CSeq_feat*>(m_Objs[i].object.GetPointer());

		const objects::CVariation * snp_search =
			dynamic_cast<const objects::CVariation*>(m_Objs[i].object.GetPointer());

		if (docsum) {
            // creating project item for each docsum
            string label;
            item.Reset(new CProjectItem());
            CRef<CSerialObject> obj(x_Convert(docsum->GetNode(),docsum->GetDBName()));

            CScope scope(*CObjectManager::GetInstance());
            scope.AddDefaults();
            CLabel::GetLabel(*obj, &label, CLabel::eDefault, &scope);
            item->SetLabel(label);
            item->SetObject(*obj);

            // add item to a list
            AddProjectItem(*item);
        }
        else if (snp_search) {
            item.Reset(new CProjectItem());

			// CVariation used as a search result can have one and only one placement
			NCBI_ASSERT(snp_search->CanGetPlacements(), "Unexpected absence of placements in SNP Search Result!");
			const objects::CVariation::TPlacements& Placements(snp_search->GetPlacements());
			NCBI_ASSERT(Placements.size(), "Unexpected number of placements in SNP Search Result!");
			CRef<CVariantPlacement> pPlacement(Placements.front());

			item->SetObject(pPlacement->SetLoc());
			CLabel::SetLabelByData(*item, m_Scope.GetPointer());

            // add item to a list
            AddProjectItem(*item);
		}
        else if (seq_loc) {
             if (sl_item.Empty()) {
                 annot.Reset(new CSeq_annot());
                 annot->SetName(m_Label);

                 CRef<CAnnotdesc>    descr(new CAnnotdesc());
                 descr->SetName(m_Label);
                 descr->SetComment(m_Comment);
                 annot->SetDesc().Set().push_back(descr);

                 sl_item.Reset(new CProjectItem());
             }

             CRef<CSeq_feat> feat(new CSeq_feat());
            /*
             CSeq_loc& floc = feat->SetLocation();
             floc.SetInt().SetFrom(seq_loc->GetStart(eExtreme_Positional));
             floc.SetInt().SetTo(seq_loc->GetStop(eExtreme_Positional));
             // assume plus strand for now
             floc.SetInt().SetStrand(eNa_strand_plus);
             floc.SetId(sequence::GetId(*seq_loc, m_Objs[i].scope));

             CRef<CSeq_loc> new_loc =
                 CSeqUtils::RemapChildToParent(*seq_loc, floc);

             feat->SetLocation(*new_loc);
             */

            feat->SetLocation(*const_cast<CSeq_loc*>(seq_loc));


             // set feature data
             feat->SetData().SetRegion() = "region";

             // save in annot
             annot->SetData().SetFtable().push_back(feat);

        }
        else if (seq_feat) {
             if (sl_item.Empty()) {
                 annot.Reset(new CSeq_annot());
                 annot->SetName(m_Label);

                 CRef<CAnnotdesc>    descr(new CAnnotdesc());
                 descr->SetName(m_Label);
                 descr->SetComment(m_Comment);
                 annot->SetDesc().Set().push_back(descr);

                 sl_item.Reset(new CProjectItem());
             }
             CRef<CSeq_feat> feat(new CSeq_feat());
             feat->Assign(*seq_feat);
             annot->SetData().SetFtable().push_back(feat);
        }
    }

    // if we created a an special item with seq annot, add it
    if (!sl_item.Empty()) {
        string label;
        CLabel::GetLabel(*annot, &label, CLabel::eContent, m_Scope);
        sl_item->SetLabel(label);
        //sl_item->SetLabel("CSeq_annot");

        sl_item->SetObject(*annot);
        AddProjectItem(*sl_item);
    }
}


CRef<CSerialObject> CGBankLoadingJob::x_Convert(const xml::node &doc_sum, const string& db_name)
{
    CRef<CSerialObject> obj;

    if(db_name == "nucleotide"  ||  db_name == "protein")   {
        /// Nucleotide and Protein - load as Seq-ids
        CRef<CSeq_id> id = CEntrezDB::CreateId_Nuc_Prot(doc_sum);
        obj.Reset(id.GetPointer());
    } else if(db_name == "assembly") {
        xml::node::const_iterator a_acc = doc_sum.find("AssemblyAccession");
        string id_str;
        if (a_acc != doc_sum.end())
            id_str = a_acc->get_content();

        if (!id_str.empty()) {
            //!! used when an assembly found in the search view is put into the project in its serialized form
            //!! consequence in requested information reduction: less sequences are suggested when clicked upon
            //!! from project view (maybe less alignments too). suggestion: use the reduced assembly by default,
            //!! but give the user an option to request a full one (giving a warning that it will take much more
            //!! time and space to handle)
            CRef<CGC_Assembly> gc_assm = CGencollSvc::GetInstance()->GetGCAssembly(id_str, true, "Gbench");
            if (!gc_assm) {
                string msg = "Failed to retrieve gencoll assembly for ";
                msg += id_str;
                NCBI_THROW(CException, eUnknown, msg);
            }
            obj.Reset(gc_assm.GetPointer());
        } else {
            string msg = "The object doesn't contain assembly information.";
            NCBI_THROW(CException, eUnknown, msg);
        }
    } else if(db_name == "gene") {
        /// Entrez Gene
        CRef<CEntrezgene> gene = CEntrezDB::CreateGene_Gene(doc_sum);
        obj.Reset(gene.GetPointer());
    }

    return obj;
}

string CGBankLoadingJob::GetDescr() const
{
    return "Add search results to project...";
}


END_NCBI_SCOPE
