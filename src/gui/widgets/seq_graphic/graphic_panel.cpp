/*  $Id: graphic_panel.cpp 44402 2019-12-12 18:12:05Z filippov $
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
* Authors:  Roman Katargin
*
* File Description:
*
*/

#include <ncbi_pch.hpp>

#include <wx/choice.h>
#include <wx/time.h>
#include <wx/menuitem.h>

#include <gui/widgets/seq_graphic/graphic_panel.hpp>
#include <gui/widgets/seq_graphic/seqgraphic_widget.hpp>
#include <gui/widgets/seq_graphic/feature_panel.hpp>
#include <gui/widgets/wx/ui_command.hpp>
#include <gui/widgets/wx/ind_progress_bar.hpp>
#include <gui/widgets/wx/fileartprov.hpp>
#include <gui/widgets/wx/wx_utils.hpp>
#include <gui/widgets/wx/richtextctrl.hpp>

BEGIN_NCBI_SCOPE
USING_SCOPE(objects);

BEGIN_EVENT_TABLE(CGraphicPanel, wxPanel)
    EVT_CHOICE(ID_TB_AssemblyList, CGraphicPanel::OnAssemblySelected)
    EVT_MENU(eCmdConfigureTracks, CGraphicPanel::OnConfigureTracks)
    EVT_MENU(ID_TB_Help, CGraphicPanel::OnHelpClick)
    EVT_TIMER(-1, CGraphicPanel::OnTimer)
    EVT_TEXT_ENTER( ID_TB_SearchTerm, CGraphicPanel::OnSearchTermCtrlEnter )
    EVT_MENU( ID_TB_SearchButton, CGraphicPanel::OnFindNextClick )
END_EVENT_TABLE()

static const wxChar* pszMainToolbar = wxT("Main Toolbar");

CGraphicPanel::CGraphicPanel(wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style)
    : m_Timer(this), m_AssemblyLoaded(false)
{
    Init();
    Create(parent, id, pos, size, style);
    CreateControls();
}

CGraphicPanel::~CGraphicPanel()
{
    m_AuiManager.UnInit();
}

void CGraphicPanel::Init()
{
    m_SeqWidget = 0;
    m_FindTextCtrl = nullptr;
    m_FindModeCtrl = nullptr;
}

void CGraphicPanel::CreateControls()
{
    Hide();

    m_AuiManager.SetManagedWindow(this);

    m_SeqWidget = new CSeqGraphicWidget(this, ID_WIDGET);
    m_SeqWidget->Create();
    m_AuiManager.AddPane(m_SeqWidget, wxAuiPaneInfo().CenterPane());
}

void CGraphicPanel::CreateToolbar()
{
    if (!x_CheckLoaded())
        m_Timer.Start(500);

    x_CreateToolbar();
}

bool CGraphicPanel::ProcessEvent(wxEvent &event)
{
    if (event.IsCommandEvent()) {
        wxEventType type = event.GetEventType();
        if (type == wxEVT_UPDATE_UI || type == wxEVT_COMMAND_MENU_SELECTED) {
            wxEvtHandler* evtHandler = 0;
            if (m_SeqWidget) evtHandler = m_SeqWidget->GetEventHandler();
            if (evtHandler && evtHandler->ProcessEventLocally(event))
                return true;
        }
    }
    return wxPanel::ProcessEvent(event);
}

void CGraphicPanel::OnAssemblySelected(wxCommandEvent& event)
{
    int selection = event.GetSelection();
    if (selection < 0 || selection >= (int)m_Assemblies.size())
        return;

    CFeaturePanel* fp = m_SeqWidget->GetFeaturePanel();
    if (fp)
        fp->SetAssembly(m_Assemblies[selection]->GetAccession());
}

void CGraphicPanel::OnTimer(wxTimerEvent& event)
{
    if (x_CheckLoaded()) {
        m_Timer.Stop();
        x_CreateToolbar();
    }
}

bool CGraphicPanel::x_CheckLoaded()
{
    CFeaturePanel* fp = m_SeqWidget->GetFeaturePanel();
    if (!fp) return false;

    m_AssemblyLoaded = fp->AssemblyLoaded();
    if (m_AssemblyLoaded) {
        m_Assemblies = fp->GetAssemblies();
        m_CurrentAssembly = fp->GetCurrentAssembly();
    }
    return m_AssemblyLoaded;
}

void CGraphicPanel::x_CreateToolbar()
{
    static bool sImagesRegistered = false;
    if (!sImagesRegistered) {
        wxFileArtProvider* provider = GetDefaultFileArtProvider();
        provider->RegisterFileAlias(wxT("graphic_panel::search"), wxT("search.png"));
        sImagesRegistered = true;
    }

    wxAuiPaneInfo& pane = m_AuiManager.GetPane(pszMainToolbar);
    wxAuiToolBar* toolbar = nullptr;
    wxString searchTerm;
    if (pane.IsOk()) {
        toolbar = (wxAuiToolBar*)pane.window;

        wxControl* ctrl = toolbar->FindControl(ID_TB_AssemblyLabel);
        if (ctrl) ctrl->Destroy();

        ctrl = toolbar->FindControl(ID_TB_AssemblyList);
        if (ctrl) ctrl->Destroy();

        ctrl = toolbar->FindControl(ID_TB_SearchModeCtrl);
        if (ctrl) ctrl->Destroy();
        m_FindModeCtrl = 0;
        
        ctrl = toolbar->FindControl(ID_TB_SearchTerm);
        if (ctrl) {
            searchTerm = ((CRichTextCtrl*)ctrl)->GetValue();
            ctrl->Destroy();
        }
        m_FindTextCtrl = 0;

        toolbar->ClearTools();
    }
    else {
        toolbar = new wxAuiToolBar(this, wxID_ANY, wxDefaultPosition,
                                   wxDefaultSize, wxAUI_TB_HORIZONTAL | wxBORDER_NONE);
    }

    CUICommandRegistry& cmd_reg = CUICommandRegistry::GetInstance();

    cmd_reg.AppendTool(*toolbar, eCmdBack);
    cmd_reg.AppendTool(*toolbar, eCmdForward);
    toolbar->AddSeparator();
    cmd_reg.AppendTool(*toolbar, eCmdZoomOut);
    cmd_reg.AppendTool(*toolbar, eCmdZoomIn);
    cmd_reg.AppendTool(*toolbar, eCmdZoomAll);
    cmd_reg.AppendTool(*toolbar, eCmdZoomSeq);
    cmd_reg.AppendTool(*toolbar, eCmdZoomSel);
    toolbar->AddSeparator();
    cmd_reg.AppendTool(*toolbar, eCmdConfigureTracks);
    toolbar->AddSeparator();

//    if (RunningInsideNCBI()) 
    {
        toolbar->AddLabel(ID_TB_SearchModeLabel, wxT("Find mode:"));
        wxArrayString strings;
        strings.Add(_("Match case"));
        strings.Add(_("Do not match case"));
        m_FindModeCtrl = new wxChoice(toolbar, ID_TB_SearchModeCtrl, wxDefaultPosition, wxDefaultSize, strings);
        m_FindModeCtrl->SetSelection(0);
        toolbar->AddControl(m_FindModeCtrl);

        m_FindTextCtrl = new CRichTextCtrl(toolbar, ID_TB_SearchTerm, searchTerm, wxDefaultPosition, wxDefaultSize, wxTE_PROCESS_ENTER);
        m_FindTextCtrl->SetMaxLength(60);

        wxMenu* menu = new wxMenu();
        menu->Append(wxID_COPY, _("&Copy"));
        menu->Append(wxID_PASTE, _("&Paste"));
        m_FindTextCtrl->SetContextMenu(menu);

        toolbar->AddControl(m_FindTextCtrl, wxT("Enter search term"));

        toolbar->AddTool(ID_TB_SearchButton, wxT("Search"), wxArtProvider::GetBitmap(wxT("graphic_panel::search")), wxT("Search"));
        toolbar->AddSeparator();
    }

    if (m_AssemblyLoaded) {
        toolbar->AddLabel(ID_TB_AssemblyLabel, wxT("Assembly:"));
        if (m_Assemblies.empty())
            toolbar->AddLabel(ID_TB_AssemblyList, wxT("N/A"));
        else {
            int selection = -1;
            wxChoice* assListCtrl = new wxChoice(toolbar, ID_TB_AssemblyList);
            for (auto i : m_Assemblies) {
                string accession = i->GetAccession();
                string name = i->GetName();
                int index = assListCtrl->Append(ToWxString(name + " (") + accession + ")");
                if (!m_CurrentAssembly.empty() &&
                    m_CurrentAssembly == accession)
                    selection = index;
                //objects::CGC_Assembly::TFullAssemblies ass = i->GetFullAssemblies();
                //for (auto a : ass)
                //assListCtrl->Append(ToWxString(a->GetName()));
            }
            if (selection >= 0)
                assListCtrl->SetSelection(selection);
            toolbar->AddControl(assListCtrl);
        }
    }
    else {
        toolbar->AddLabel(ID_TB_AssemblyLabel, wxT("Querying Assembly..."));
        toolbar->AddControl(new CIndProgressBar(toolbar, ID_TB_AssemblyList, wxDefaultPosition, 80),
            wxT("Querying..."));
    }

    toolbar->AddStretchSpacer();
    toolbar->AddTool(ID_TB_Help, wxEmptyString,
        wxArtProvider::GetBitmap(wxART_HELP, wxART_OTHER, wxSize(16, 16)),
                                 wxT("Graphical View Help"));

    // As part of auto resize tollbar is resized in vertical position for a moment
    // this is visible on MS windows screen - so hide it
    toolbar->Hide();
    toolbar->Realize();
    toolbar->Show();

    if (!pane.IsOk())
        m_AuiManager.AddPane(toolbar, wxAuiPaneInfo()
        .Name(pszMainToolbar)
        .Caption(pszMainToolbar)
        .ToolbarPane()
        .Top()
        .TopDockable(true)
        .LeftDockable(false)
        .RightDockable(false)
        .BottomDockable(false)
        .Floatable(false)
        .Resizable(true)
        .DockFixed()
        .Gripper(false)
        .DestroyOnClose(true)
        .Position(0));
    else
        pane.BestSize(toolbar->GetClientSize());

    m_AuiManager.Update();
    Update();
}


void CGraphicPanel::OnConfigureTracks(wxCommandEvent&)
{
    m_SeqWidget->ConfigureTracksDlg();
}

void CGraphicPanel::OnHelpClick(wxCommandEvent& evt)
{
    wxAuiPaneInfo& pane = m_AuiManager.GetPane(pszMainToolbar);
    if (!pane.IsOk()) return;

    wxAuiToolBar* tb = static_cast<wxAuiToolBar*>(pane.window);
    CUICommandRegistry& cmd_reg = CUICommandRegistry::GetInstance();

    wxMenu menuPopup;
    cmd_reg.AppendMenuItem(menuPopup, eCmdWebHelpGSVNavigation);
    cmd_reg.AppendMenuItem(menuPopup, eCmdWebHelpGSVInteractionGuide);
    cmd_reg.AppendMenuItem(menuPopup, eCmdWebHelpGSVLegends);
    cmd_reg.AppendMenuItem(menuPopup, eCmdWebHelpApp);

    // line up our menu with the button
    wxRect rect = tb->GetToolRect(evt.GetId());
    wxPoint pt = tb->ClientToScreen(rect.GetBottomLeft());
    pt = ScreenToClient(pt);

    PopupMenu(&menuPopup, pt);
}

void CGraphicPanel::x_FindText()
{
    if (!m_SeqWidget)
        return;

    const wxString caption = wxT("Find Text");

    wxString text = m_FindTextCtrl->GetValue();
    if (text.empty()) {
        m_SeqWidget->ResetSearch();
        wxMessageBox(wxT("Search string is empty."), caption, wxOK | wxICON_EXCLAMATION);
        return;
    }

    bool found = false;
    {
        wxBusyCursor wait;
        found =  m_SeqWidget->FindText(text.ToStdString(), (m_FindModeCtrl->GetSelection() == 0)); 
    }

    if (!found)
        wxMessageBox(wxT("Search string not found."), caption, wxOK | wxICON_EXCLAMATION);
}


void CGraphicPanel::OnFindNextClick( wxCommandEvent& event )
{
    x_FindText();
}

void CGraphicPanel::OnSearchTermCtrlEnter( wxCommandEvent& event )
{
    x_FindText();
}

END_NCBI_SCOPE
