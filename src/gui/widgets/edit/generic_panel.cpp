/*  $Id: generic_panel.cpp 43609 2019-08-08 16:12:53Z filippov $
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
 * Authors:  Colleen Bollin, Andrea Asztalos
 */


#include <ncbi_pch.hpp>

#include <gui/widgets/wx/wx_utils.hpp>
#include <gui/widgets/seq/text_panel.hpp>
#include <gui/widgets/text_widget/text_item_panel.hpp>
#include <gui/framework/view_manager_service.hpp>
#include <gui/widgets/wx/file_extensions.hpp>
#include <wx/sizer.h>
#include <wx/filedlg.h>
#include <wx/bmpbuttn.h>
#include <wx/hyperlink.h>
#include <gui/widgets/edit/generic_panel.hpp>  

BEGIN_NCBI_SCOPE

/*!
 * CGenericPanel type definition
 */

 IMPLEMENT_DYNAMIC_CLASS(CGenericPanel, wxPanel)


/*!
 * CGenericPanel event table definition
 */

 BEGIN_EVENT_TABLE(CGenericPanel, wxPanel)
    EVT_BUTTON(ID_GENERIC_REPORT_CLOSE, CGenericPanel::OnCloseButton)
    EVT_BUTTON(ID_GEN_RPT_REFRESH, CGenericPanel::OnGenRptRefreshClick)
    EVT_BUTTON(ID_GENERIC_REPORT_FIND, CGenericPanel::OnGenericReportFindClick)
    EVT_BUTTON(ID_GENERIC_REPORT_CLOSE2, CGenericPanel::OnCloseButton)
    EVT_BUTTON(ID_GEN_RPT_REFRESH2, CGenericPanel::OnGenRptRefreshClick2)
    EVT_BUTTON(ID_GENERIC_REPORT_FIND2, CGenericPanel::OnGenericReportFindClick2)
    EVT_BUTTON(ID_GENERIC_REPORT_EXPORT, CGenericPanel::OnGenericReportExportClick)  
    EVT_TEXT_ENTER(ID_GENERIC_REPORT_TEXT_FIND, CGenericPanel::OnGenericReportFindClick)
    EVT_TEXT_ENTER(ID_GENERIC_REPORT_TEXT_FIND2, CGenericPanel::OnGenericReportFindClick2)
END_EVENT_TABLE()


/*!
 * CGenericPanel constructors
 */

 CGenericPanel::CGenericPanel()
    : m_Workbench(0), m_Simple(false)
{
    Init();
}

CGenericPanel::CGenericPanel(wxWindow* parent, IWorkbench* workbench, wxWindowID id, const wxPoint& pos, const wxSize& size, long style)
    : m_Workbench(workbench), m_Simple(false)
{
    Init();
    Create(parent, id, pos, size, style);
}


CGenericPanel::CGenericPanel(wxWindow* parent, bool simple, IWorkbench* workbench, wxWindowID id, const wxPoint& pos, const wxSize& size, long style)
    : m_Workbench(workbench), m_Simple(simple)
{
    Init();
    Create(parent, id, pos, size, style);
}


/*!
 * CGenericPanel creator
 */

bool CGenericPanel::Create(wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style)
{
////@begin CGenericPanel creation
    wxPanel::Create( parent, id, pos, size, style );
    CreateControls();
    if (GetSizer())
    {
        GetSizer()->SetSizeHints(this);
    }
    Centre();
////@end CGenericPanel creation
    return true;
}


/*!
 * CGenericPanel destructor
 */

CGenericPanel::~CGenericPanel()
{
    if (m_Refresh) {
        delete m_Refresh;
    }
}


/*!
 * Member initialisation
 */

void CGenericPanel::Init()
{
    m_RTCtrl = NULL;
    m_FindText = NULL;
    m_RefreshBtn = NULL;
    m_FindText2 = NULL;
    m_RefreshBtn2 = NULL;
    m_Refresh = NULL;
    m_PrevFindPos = 0;
    m_PrevFindText = "";
    m_move_to_top = false;
    m_FindModeCtrl = NULL;
    m_busy = false;
    m_HelpButton = NULL;
}


/*!
 * Control creation for CGenericPanel
 */

void CGenericPanel::CreateControls()
{    
    CGenericPanel* itemDialog1 = this;

    wxBoxSizer* itemBoxSizer2 = new wxBoxSizer(wxVERTICAL);
    itemDialog1->SetSizer(itemBoxSizer2);

    wxBoxSizer* itemBoxSizer5 = new wxBoxSizer(wxHORIZONTAL);
    itemBoxSizer2->Add(itemBoxSizer5, 0, wxGROW|wxALL, 5);

    wxArrayString strings;
    strings.Add(_("Match case"));
    strings.Add(_("Do not match case"));    
    m_FindModeCtrl = new wxChoice(itemDialog1, wxID_ANY, wxDefaultPosition, wxDefaultSize, strings);
    itemBoxSizer5->Add(m_FindModeCtrl, 0, wxALIGN_CENTER_VERTICAL | wxALL, 5);
    m_FindModeCtrl->SetSelection(1);

    m_FindText2 = new wxTextCtrl(itemDialog1, ID_GENERIC_REPORT_TEXT_FIND2, wxEmptyString, wxDefaultPosition, wxSize(200, -1), wxTE_PROCESS_ENTER);
    itemBoxSizer5->Add(m_FindText2, 0, wxALIGN_CENTER_VERTICAL | wxALL, 5);

    wxBitmapButton* itemButton9 = new wxBitmapButton(itemDialog1, ID_GENERIC_REPORT_FIND2, wxArtProvider::GetBitmap(wxT("text_panel::search")));
    itemBoxSizer5->Add(itemButton9, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    if (!m_Simple) {
        m_RefreshBtn2 = new wxButton(itemDialog1, ID_GEN_RPT_REFRESH2, _("Refresh Top"), wxDefaultPosition, wxDefaultSize, 0);
        itemBoxSizer5->Add(m_RefreshBtn2, 0, wxALIGN_CENTER_VERTICAL | wxALL, 5);

        wxButton* itemButton10 = new wxButton(itemDialog1, ID_GENERIC_REPORT_CLOSE2, _("Close"), wxDefaultPosition, wxDefaultSize, 0);
        itemBoxSizer5->Add(itemButton10, 0, wxALIGN_CENTER_VERTICAL | wxALL, 5);

        itemBoxSizer5->AddStretchSpacer();
        wxButton* itemButton11 = new wxButton(itemDialog1, ID_GENERIC_REPORT_EXPORT, _("Export..."), wxDefaultPosition, wxDefaultSize, 0);
        itemBoxSizer5->Add(itemButton11, 0, wxALIGN_CENTER_VERTICAL | wxALL, 5);
    }

    m_RTCtrl = new CGenTextCtrl(itemDialog1, ID_RICHTEXTCTRL, wxEmptyString, wxDefaultPosition, wxSize(800, 400), wxWANTS_CHARS | wxTE_MULTILINE | wxTE_RICH | wxHSCROLL);
    // m_RTCtrl->SetFont(wxFont(10, wxSWISS, wxNORMAL, wxNORMAL, false, wxT("Consolas")));
    SetFontAsTextPanel();
    SetLineSpacing(5);
    itemBoxSizer2->Add(m_RTCtrl, 1, wxGROW|wxALL, 5);

    if (!m_Simple) {
        wxBoxSizer* itemBoxSizer4 = new wxBoxSizer(wxHORIZONTAL);
        itemBoxSizer2->Add(itemBoxSizer4, 0, wxGROW|wxALL, 5);

        m_FindText = new wxTextCtrl( itemDialog1, ID_GENERIC_REPORT_TEXT_FIND, wxEmptyString, wxDefaultPosition, wxSize(200, -1), wxTE_PROCESS_ENTER);
        itemBoxSizer4->Add(m_FindText, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

        wxBitmapButton* itemButton5 = new wxBitmapButton(itemDialog1, ID_GENERIC_REPORT_FIND, wxArtProvider::GetBitmap(wxT("text_panel::search")));
        itemBoxSizer4->Add(itemButton5, 0, wxALIGN_CENTER_VERTICAL | wxALL, 5);

         m_RefreshBtn = new wxButton(itemDialog1, ID_GEN_RPT_REFRESH, _("Refresh"), wxDefaultPosition, wxDefaultSize, 0);
        itemBoxSizer4->Add(m_RefreshBtn, 0, wxALIGN_CENTER_VERTICAL | wxALL, 5);

        wxButton* itemButton8 = new wxButton(itemDialog1, ID_GENERIC_REPORT_CLOSE, _("Close"), wxDefaultPosition, wxDefaultSize, 0);
        itemBoxSizer4->Add(itemButton8, 0, wxALIGN_CENTER_VERTICAL | wxALL, 5);

        m_HelpButton = new wxHyperlinkCtrl( itemDialog1, wxID_HELP, _("Help"), wxEmptyString, wxDefaultPosition, wxDefaultSize, wxHL_DEFAULT_STYLE );
        m_HelpButton->SetForegroundColour(wxColour(192, 192, 192));
        itemBoxSizer4->Add(m_HelpButton, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

        m_RefreshBtn->Show(false);
        m_RefreshBtn2->Show(false);
        m_HelpButton->Hide();
    }
}

void CGenericPanel::SetHelpUrl(const wxString &url)
{
    m_HelpButton->SetURL(url);
    m_HelpButton->Show(!url.IsEmpty());
}

void CGenericPanel::SetFontAsTextPanel()
{
    wxFont font = CTextPanel::GetFont(CGuiRegistry::GetInstance().GetInt("GBENCH.TextView.FontSize", 1));
    m_RTCtrl->SetInitialSize(wxSize(800*font.GetPointSize()/10,400));
    m_RTCtrl->SetFont(font);
}


/*!
 * Should we show tooltips?
 */

bool CGenericPanel::ShowToolTips()
{
    return true;
}

/*!
 * Get bitmap resources
 */

wxBitmap CGenericPanel::GetBitmapResource(const wxString& name)
{
    wxUnusedVar(name);
    return wxNullBitmap;
}

/*!
 * Get icon resources
 */

wxIcon CGenericPanel::GetIconResource(const wxString& name)
{
    wxUnusedVar(name);
    return wxNullIcon;
}


void CGenericPanel::SetLineSpacing(int spacing)
{
    wxTextAttr attr(m_RTCtrl->GetDefaultStyle());
    attr.SetLineSpacing(spacing);
    m_RTCtrl->SetDefaultStyle(attr);
}


void CGenericPanel::SetText(const wxString& text)
{
    m_RTCtrl->StorePosition();
    Freeze();
    m_RTCtrl->ChangeValue(text);
    if (m_move_to_top)
    {
        m_RTCtrl->ShowPosition(0);
        m_move_to_top = false;
    }
    else
    {
        m_RTCtrl->RestorePosition();
    }
    Thaw();  
    if (m_busy)
    {
        m_busy = false;
        wxEndBusyCursor();
    }
}


void CGenericPanel::SetRefresh(IRefreshCntrl * refresh)
{
    if (m_Refresh) {
        delete m_Refresh;
    }
    m_Refresh = refresh;
    if (refresh) {
        m_RefreshBtn->Show(true);
        m_RefreshBtn2->Show(true);
    } else {
        m_RefreshBtn->Show(false);
        m_RefreshBtn2->Show(false);
    }
}

void CGenericPanel::RefreshData()
{
    if (!m_busy)
    {
        m_busy = true;
        wxBeginBusyCursor();
    }

    if (m_Refresh) {
        m_Refresh->RefreshText(this);
    }
}


void CGenericPanel::OnGenRptRefreshClick(wxCommandEvent& event)
{
    m_move_to_top = false;
    RefreshData();
}

void CGenericPanel::OnGenRptRefreshClick2(wxCommandEvent& event)
{
    m_move_to_top = true;
    RefreshData();
}

void CGenericPanel::OnGenericReportFindClick(wxCommandEvent& event)
{
    string find = ToStdString(m_FindText->GetValue());
    if (NStr::IsBlank(find)) {
        return;
    }
    string text = ToStdString(m_RTCtrl->GetValue());

    if (find != m_PrevFindText) {
        // looking for new text
        m_PrevFindPos = 0;
    }
    
    long val = NStr::Find(text, find, static_cast<NStr::ECase>(m_FindModeCtrl->GetSelection()), NStr::eForwardSearch, m_PrevFindPos);
    if (val == string::npos && m_PrevFindPos > 0) {
        m_PrevFindPos = 0;
        val = NStr::Find(text, find, static_cast<NStr::ECase>(m_FindModeCtrl->GetSelection()), NStr::eForwardSearch, m_PrevFindPos);
    }
    
    if (val != string::npos) {
        m_RTCtrl->SetSelection(val, val + find.length());
        m_RTCtrl->ShowPosition(val);
        m_PrevFindPos++;
    }

    m_PrevFindText = find;
}

void CGenericPanel::OnGenericReportFindClick2(wxCommandEvent& event)
{
    string find = ToStdString(m_FindText2->GetValue());
    if (NStr::IsBlank(find)) {
        return;
    }
    string text = ToStdString(m_RTCtrl->GetValue());

    if (find != m_PrevFindText) {
        // looking for new text
        m_PrevFindPos = 0;
    }

    long val = NStr::Find(text, find, static_cast<NStr::ECase>(m_FindModeCtrl->GetSelection()), NStr::eForwardSearch, m_PrevFindPos);
    if (val == string::npos && m_PrevFindPos > 0) {
        m_PrevFindPos = 0;
        val = NStr::Find(text, find, static_cast<NStr::ECase>(m_FindModeCtrl->GetSelection()), NStr::eForwardSearch, m_PrevFindPos);
    }

    if (val != string::npos) {
        m_RTCtrl->SetSelection(val, val + find.length());
        m_RTCtrl->ShowPosition(val);
        m_PrevFindPos++;
    }

    m_PrevFindText = find;
}


void CGenericPanel::OnCloseButton(wxCommandEvent& event)
{
    GetParent()->Destroy();
}

void CGenericPanel::OnGenericReportExportClick(wxCommandEvent& event)
{
    wxFileDialog dlg(this, wxT("Select a file"), m_WorkDir, wxT(""),
                     CFileExtensions::GetDialogFilter(CFileExtensions::kAllFiles),
                     wxFD_SAVE | wxFD_OVERWRITE_PROMPT);

    if (dlg.ShowModal() != wxID_OK)
        return;

    wxString path = dlg.GetPath();
    if (!path.IsEmpty()) {
        CNcbiOfstream os(path.fn_str(), ios::out);
        os << m_RTCtrl->GetValue();
    }
}

BEGIN_EVENT_TABLE(CGenTextCtrl, wxTextCtrl)
EVT_ACTIVATE(CGenTextCtrl::OnActivate)
EVT_KILL_FOCUS(CGenTextCtrl::OnKillFocus)
EVT_SET_FOCUS(CGenTextCtrl::OnSetFocus)
EVT_CHILD_FOCUS(CGenTextCtrl::OnSetChildFocus)
EVT_SCROLLWIN(CGenTextCtrl::OnScroll)
EVT_SCROLLWIN_TOP(CGenTextCtrl::OnScrollWinTop)
EVT_IDLE(CGenTextCtrl::OnIdle)
END_EVENT_TABLE()


void CGenTextCtrl::OnKillFocus(wxFocusEvent& event)
{  
    StorePosition();
    event.Skip();
}

void CGenTextCtrl::OnSetFocus(wxFocusEvent& event)
{
    event.Skip();
    m_safe_to_store = false;
    CallAfter(&CGenTextCtrl::RestorePosition);
}

void CGenTextCtrl::OnSetChildFocus(wxChildFocusEvent& event)
{
    event.Skip();
    m_safe_to_store = false;
    CallAfter(&CGenTextCtrl::RestorePosition);
}

void CGenTextCtrl::OnActivate(wxActivateEvent& event)
{
    if (event.GetActive())
    {
        event.Skip();
        m_safe_to_store = false;
        CallAfter(&CGenTextCtrl::RestorePosition);
    }
    else
    {
        event.Skip();
    }
}

void CGenTextCtrl::OnScroll(wxScrollWinEvent& event)
{
    event.Skip();
    StorePosition();
}

void CGenTextCtrl::OnIdle(wxIdleEvent&)
{
    StorePosition();
}

void CGenTextCtrl::ShowPosition(long pos)
{
    wxTextCtrl::ShowPosition(pos);
    CallAfter(&CGenTextCtrl::StorePosition);
}

void CGenTextCtrl::OnScrollWinTop(wxScrollWinEvent& event)
{
    event.StopPropagation();
}

void CGenTextCtrl::RestorePosition()
{
    wxTextCtrl::ShowPosition(m_scroll_pos);
    m_safe_to_store = true;
}

void CGenTextCtrl::StorePosition()
{
    if (m_safe_to_store)
        HitTest(GetClientRect().GetTopLeft(), &m_scroll_pos);
}

END_NCBI_SCOPE

