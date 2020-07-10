/*  $Id: diag_panel.cpp 43619 2019-08-09 16:32:36Z katargir $
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

#include <wx/artprov.h>
#include <wx/sizer.h>
#include <wx/panel.h>
#include <wx/choice.h>
#include <wx/menu.h>

#include <wx/sstream.h>
#include <wx/txtstrm.h>
#include <wx/clipbrd.h>
#include <wx/dataobj.h>

#include <gui/framework/diag_panel.hpp>
#include <gui/framework/view_manager_service.hpp> //TODO

#include <gui/widgets/wx/log_gbench.hpp>

#include <gui/objutils/registry.hpp>

#include <gui/utils/extension_impl.hpp>

#include <gui/framework/diag_console_list.hpp>

// TODO
#include "../res/share/gbench/error16x16.xpm"
#include "../res/share/gbench/warning16x16.xpm"
#include "../res/share/gbench/info16x16.xpm"
#include "../res/share/gbench/htmpage16x16.xpm"
#include "../res/share/gbench/transp16x16.xpm"

#include <wx/statline.h>
#include <wx/settings.h>


BEGIN_NCBI_SCOPE

/// register the factory
static CExtensionDeclaration
    decl("view_manager_service::view_factory", new CDiagConsoleViewFactory());

///////////////////////////////////////////////////////////////////////////////
/// CDiagConsolePanel
BEGIN_EVENT_TABLE( CDiagConsolePanel, wxPanel )
    EVT_MENU(ID_TOOL_ERRORS, CDiagConsolePanel::OnToolErrorsClick )
    EVT_MENU(ID_TOOL_WARNINGS, CDiagConsolePanel::OnToolWarningsClick )
    EVT_MENU(ID_TOOL_INFO, CDiagConsolePanel::OnToolInfoClick )
    EVT_MENU(ID_TOOL_OTHER, CDiagConsolePanel::OnToolOtherClick )
    EVT_CHOICE(ID_CHOICE3, CDiagConsolePanel::OnChoice3Selected )
    EVT_MENU(wxID_COPY, CDiagConsolePanel::OnCopy)
    EVT_UPDATE_UI(wxID_COPY, CDiagConsolePanel::OnCopyUpdateUI )
END_EVENT_TABLE()


CDiagConsolePanel::CDiagConsolePanel() : m_ImageList(16, 16)
{
    Init();
}

bool CDiagConsolePanel::Create( wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style )
{
#ifdef __WXOSX_COCOA__ // GB-8581
    SetBackgroundStyle(wxBG_STYLE_COLOUR);
    SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_FRAMEBK));
#endif

    wxPanel::Create( parent, id, pos, wxSize(0,0), style );
    CreateControls();

    x_LoadMessages();

    return true;
}


CDiagConsolePanel::~CDiagConsolePanel()
{
}


void CDiagConsolePanel::Init()
{
    m_LogBufferIndex = 0;
    m_MsgList = NULL;
}


void CDiagConsolePanel::CreateControls()
{
    CDiagConsolePanel* itemPanel1 = this;

    wxBoxSizer* main_sizer = new wxBoxSizer(wxVERTICAL);
    itemPanel1->SetSizer(main_sizer);

    wxBoxSizer* itemBoxSizer3 = new wxBoxSizer(wxVERTICAL);
    main_sizer->Add(itemBoxSizer3, 0, wxGROW|wxALL, 0);

    wxToolBar* tool_bar = new wxToolBar( itemPanel1, ID_TOOLBAR2, wxDefaultPosition, wxDefaultSize, wxTB_FLAT|wxTB_HORIZONTAL|wxTB_TEXT|wxTB_HORZ_LAYOUT );
    tool_bar->SetToolBitmapSize(wxSize(16, 16));
    wxBitmap errorBitmap(error16x16_xpm);
    wxBitmap errorBitmapDisabled;
    tool_bar->AddTool(ID_TOOL_ERRORS, wxT("Errors"), errorBitmap, errorBitmapDisabled, wxITEM_CHECK, wxT(""), wxEmptyString);
    wxBitmap warningBitmap(warning16x16_xpm);
    wxBitmap warningBitmapDisabled;
    tool_bar->AddTool(ID_TOOL_WARNINGS, wxT("Warnings"), warningBitmap, warningBitmapDisabled, wxITEM_CHECK, wxT(""), wxEmptyString);
    wxBitmap infoBitmap(info16x16_xpm);
    wxBitmap infoBitmapDisabled;
    tool_bar->AddTool(ID_TOOL_INFO, wxT("Info"), infoBitmap, infoBitmapDisabled, wxITEM_CHECK, wxT(""), wxEmptyString);
    wxBitmap otherBitmap(htmpage_xpm);
    wxBitmap otherBitmapDisabled;
    tool_bar->AddTool(ID_TOOL_OTHER, wxT("Other"), otherBitmap, otherBitmapDisabled, wxITEM_CHECK, wxT(""), wxEmptyString);
    tool_bar->AddSeparator();

    wxArrayString itemChoice10Strings;
    itemChoice10Strings.Add(wxT("All"));
    itemChoice10Strings.Add(wxT("NCBI"));
    itemChoice10Strings.Add(wxT("wxWidgets"));
    wxChoice* itemChoice10 = new wxChoice( tool_bar, ID_CHOICE3, wxDefaultPosition, wxDefaultSize, itemChoice10Strings, 0 );
    itemChoice10->SetStringSelection(wxT("All"));
    tool_bar->AddControl(itemChoice10);
    tool_bar->Realize();
    itemBoxSizer3->Add(tool_bar, 0, wxGROW|wxALL, 0);

    // separation line before the table
    wxStaticLine* line = new wxStaticLine(this, wxID_STATIC, wxDefaultPosition, wxDefaultSize, wxLI_HORIZONTAL );
    main_sizer->Add(line, 0, wxEXPAND);

    // create Table
    m_MsgList = new CDiagConsoleList( itemPanel1, ID_LISTCTRL1, wxDefaultPosition, wxSize(100, 100), wxBORDER_NONE);
    main_sizer->Add(m_MsgList, 1, wxGROW | wxALL, 0);

    m_ImageList.Add(errorBitmap);
    m_ImageList.Add(warningBitmap);
    m_ImageList.Add(infoBitmap);
    m_ImageList.Add(otherBitmap);
    m_ImageList.Add(wxBitmap(transp16x16_xpm));

    m_MsgList->SetImageList(&m_ImageList, wxIMAGE_LIST_SMALL);

    x_UpdateUI();
}


wxBitmap CDiagConsolePanel::GetBitmapResource( const wxString& name )
{
    // Bitmap retrieval
    ////@begin CDiagConsolePanel bitmap retrieval
    wxUnusedVar(name);
    return wxNullBitmap;
    ////@end CDiagConsolePanel bitmap retrieval
}

void CDiagConsolePanel::SetRegistryPath(const string& path)
{
    m_RegPath = path; // store for later use
    m_MsgList->SetRegistryPath(m_RegPath + ".MsgList");
}

void CDiagConsolePanel::SaveSettings() const
{
    m_MsgList->SaveSettings();
}

void CDiagConsolePanel::LoadSettings()
{
    m_MsgList->LoadSettings();
    x_UpdateUI();
}

void CDiagConsolePanel::x_LoadMessages()
{
    wxListCtrl* listCtrl = (wxListCtrl*)FindWindow(ID_LISTCTRL1);
    listCtrl->DeleteAllItems();
    m_LogBufferIndex = 0;
}

void CDiagConsolePanel::x_UpdateUI()
{
    wxToolBar* itemToolBar12 = (wxToolBar*)FindWindow(ID_TOOLBAR2);
    wxChoice* itemChoice18 = (wxChoice*)itemToolBar12->FindControl(ID_CHOICE3);

    size_t filter = m_MsgList->GetFilter();

    itemToolBar12->ToggleTool(ID_TOOL_ERRORS, (filter&CDiagConsoleList::eShowErrors) != 0);
    itemToolBar12->ToggleTool(ID_TOOL_WARNINGS, (filter&CDiagConsoleList::eShowWarnings) != 0);
    itemToolBar12->ToggleTool(ID_TOOL_INFO, (filter&CDiagConsoleList::eShowInfo) != 0);
    itemToolBar12->ToggleTool(ID_TOOL_OTHER, (filter&CDiagConsoleList::eShowOther) != 0);

    switch (filter&CDiagConsoleList::eShowNCBIWxWdidgets)
    {
    case CDiagConsoleList::eShowNCBI:
        itemChoice18->Select(1);
        break;
    case CDiagConsoleList::eShowWxWdidgets:
        itemChoice18->Select(2);
        break;
    default:
        itemChoice18->Select(0);
        break;
    }
}

void CDiagConsolePanel::OnToolErrorsClick(wxCommandEvent& event)
{
    size_t filter = m_MsgList->GetFilter();
    if (event.IsChecked())
        filter |= CDiagConsoleList::eShowErrors;
    else
        filter &= ~CDiagConsoleList::eShowErrors;
    m_MsgList->SetFilter(filter);
}

void CDiagConsolePanel::OnToolWarningsClick(wxCommandEvent& event)
{
    size_t filter = m_MsgList->GetFilter();
    if (event.IsChecked())
        filter |= CDiagConsoleList::eShowWarnings;
    else
        filter &= ~CDiagConsoleList::eShowWarnings;
    m_MsgList->SetFilter(filter);
}

void CDiagConsolePanel::OnToolInfoClick(wxCommandEvent& event)
{
    size_t filter = m_MsgList->GetFilter();
    if (event.IsChecked())
        filter |= CDiagConsoleList::eShowInfo;
    else
        filter &= ~CDiagConsoleList::eShowInfo;
    m_MsgList->SetFilter(filter);
}

void CDiagConsolePanel::OnToolOtherClick(wxCommandEvent& event)
{
    size_t filter = m_MsgList->GetFilter();
    if (event.IsChecked())
        filter |= CDiagConsoleList::eShowOther;
    else
        filter &= ~CDiagConsoleList::eShowOther;
    m_MsgList->SetFilter(filter);
}

void CDiagConsolePanel::OnChoice3Selected(wxCommandEvent& event)
{
    size_t filter = (m_MsgList->GetFilter()& ~CDiagConsoleList::eShowNCBIWxWdidgets);
    switch(event.GetSelection()) {
        case 1:
            filter |= CDiagConsoleList::eShowNCBI;
            break;
        case 2:
            filter |= CDiagConsoleList::eShowWxWdidgets;
            break;
        default:
            filter |= CDiagConsoleList::eShowNCBIWxWdidgets;
            break;
    }
    m_MsgList->SetFilter(filter);
}

void CDiagConsolePanel::OnCopy(wxCommandEvent& event)
{
    wxStringOutputStream stream;
    wxTextOutputStream os(stream);

    long item = m_MsgList->GetNextItem(-1, wxLIST_NEXT_ALL,
                                       wxLIST_STATE_SELECTED);
    while (item != -1)
    {
        os << m_MsgList->GetItemRawMsg(item) << endl;
        item = m_MsgList->GetNextItem(item, wxLIST_NEXT_ALL,
                                      wxLIST_STATE_SELECTED);
    }

    wxClipboardLocker locker;
    if ( !locker ) return;
    wxTheClipboard->SetData( new wxTextDataObject(stream.GetString()));
}

void CDiagConsolePanel::OnCopyUpdateUI(wxUpdateUIEvent& event)
{
    event.Enable(m_MsgList->GetSelectedItemCount() > 0);
}

///////////////////////////////////////////////////////////////////////////////
/// CDiagConsoleView

CViewTypeDescriptor CDiagConsoleView::m_TypeDescr(
    "Diagnostic Console", // type name
    "diag_console_view", // icon alias TODO
    "Diagnostic Console displays technical information that can be used for troubleshooting and debugging.",
    "Diagnostic Console displays technical information that can be used for troubleshooting and debugging.",
    "DIAG_CONSOLE_VIEW", // help ID
    "System",     // category
    true);       // singleton


CDiagConsoleView::CDiagConsoleView()
:   m_Panel(NULL)
{
}


CDiagConsoleView::~CDiagConsoleView()
{
}


const CViewTypeDescriptor& CDiagConsoleView::GetTypeDescriptor() const
{
    return m_TypeDescr;
}


void CDiagConsoleView::SetWorkbench(IWorkbench* workbench)
{
    if(workbench)   {
        // connect to services
    } else {
        // disconnect from services
    }
}

void CDiagConsoleView::CreateViewWindow(wxWindow* parent)
{
    m_Panel = new CDiagConsolePanel();
    m_Panel->Create(parent, SYMBOL_CWXDIAGPANEL_IDNAME);
}


void CDiagConsoleView::DestroyViewWindow()
{
    if(m_Panel) {
        m_Panel->Destroy();
        m_Panel = NULL;
    }
}


wxWindow* CDiagConsoleView::GetWindow()
{
    return m_Panel;
}


string CDiagConsoleView::GetClientLabel(IWMClient::ELabel) const
{
    return m_TypeDescr.GetLabel();
}


string CDiagConsoleView::GetIconAlias() const
{
    return m_TypeDescr.GetIconAlias();
}


const CRgbaColor* CDiagConsoleView::GetColor() const
{
    return NULL;
}


void CDiagConsoleView::SetColor(const CRgbaColor& color)
{
    // do nothing
}


const wxMenu* CDiagConsoleView::GetMenu()
{
    return NULL;
}


void CDiagConsoleView::UpdateMenu(wxMenu& root_menu)
{
}


IWMClient::CFingerprint CDiagConsoleView::GetFingerprint() const
{
    return CFingerprint(m_TypeDescr.GetLabel(), true);

}

void CDiagConsoleView::SetRegistryPath(const string& path)
{
    m_RegPath = path; // store for later use
    m_Panel->SetRegistryPath(m_RegPath + ".Table");
}



void CDiagConsoleView::SaveSettings() const
{
    m_Panel->SaveSettings();
}


void CDiagConsoleView::LoadSettings()
{
    m_Panel->LoadSettings();
}



///////////////////////////////////////////////////////////////////////////////
/// CDiagConsoleViewFactory

void CDiagConsoleViewFactory::RegisterIconAliases(wxFileArtProvider& provider)
{
    //TODO
}


const CViewTypeDescriptor& CDiagConsoleViewFactory::GetViewTypeDescriptor() const
{
    return CDiagConsoleView::m_TypeDescr;
}


IView* CDiagConsoleViewFactory::CreateInstance() const
{
    return new CDiagConsoleView();
}


IView* CDiagConsoleViewFactory::CreateInstanceByFingerprint(const TFingerprint& fingerprint) const
{
    TFingerprint print(CDiagConsoleView::m_TypeDescr.GetLabel(), true);
    if(print == fingerprint)   {
        return new CDiagConsoleView();
    }
    return NULL;
}


string CDiagConsoleViewFactory::GetExtensionIdentifier() const
{
    static string sid("diag_console_view_factory");
    return sid;
}


string CDiagConsoleViewFactory::GetExtensionLabel() const
{
    static string slabel("Diagnostic Console Factory");
    return slabel;
}


END_NCBI_SCOPE
