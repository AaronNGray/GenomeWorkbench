/*  $Id: diag_console_list.cpp 39934 2017-11-28 16:20:01Z katargir $
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
#include <wx/imaglist.h>
#include <wx/textbuf.h>

#include <gui/framework/diag_console_list.hpp>

#include <gui/widgets/wx/log_gbench.hpp>
#include <gui/widgets/wx/simple_clipboard.hpp>
#include <gui/widgets/wx/ui_command.hpp>

#include <gui/objutils/registry.hpp>

#include <wx/utils.h>

BEGIN_NCBI_SCOPE

BEGIN_EVENT_MAP( CDiagConsoleList, CEventHandler )
    ON_EVENT( CwxLogDiagHandler::CDiagEvent, CwxLogDiagHandler::CDiagEvent::eNewItem, &CDiagConsoleList::x_OnNewItem )
END_EVENT_MAP()

BEGIN_EVENT_TABLE( CDiagConsoleList, wxListCtrl )    
    EVT_CONTEXT_MENU( CDiagConsoleList::OnContextMenu ) 
END_EVENT_TABLE()

CDiagConsoleList::CDiagConsoleList() : m_Filter(0), m_Cache(500)
{
}

CDiagConsoleList::CDiagConsoleList(
                 wxWindow* parent,
                 wxWindowID id,
                 const wxPoint& pos,
                 const wxSize& size,
                 long style)
: m_Filter(eShowAll), m_Cache(500)
{
    Create(parent, id, pos, size, style);
}

CDiagConsoleList::~CDiagConsoleList()
{
    CwxLogDiagHandler::GetInstance()->RemoveListener(this);
}

/// Creation
bool CDiagConsoleList::Create(
            wxWindow* parent,
            wxWindowID id,
            const wxPoint& pos,
            const wxSize& size,
            long style)
{
    if (!wxListCtrl::Create(parent, id, pos, size, style|wxLC_REPORT|wxLC_VIRTUAL))
        return false;

    this->InsertColumn(0, wxT("Message"));
    this->InsertColumn(1, wxT("Time"));
    this->InsertColumn(2, wxT("Source"));

    this->SetColumnWidth(0, 320);
    this->SetColumnWidth(1, 64);
    this->SetColumnWidth(2, 400);

    CwxLogDiagHandler::GetInstance()->AddListener(this);

    m_FixGenericListCtrl.ConnectToControl(*this);

    return true;
}

void CDiagConsoleList::SetFilter(size_t filter)
{
    m_Filter = filter;
    x_ReloadMessages();
}

static size_t s_CountLines(const wxString& msg)
{
    wxString tmp = msg;
    tmp.Trim();
    return 1 + std::count(tmp.begin(), tmp.end(), wxChar('\n'));
}

static wxString s_GetLine(const wxString& msg, size_t line)
{
    wxString str;
    for (size_t pos = 0, pos2;; pos = pos2 + 1) {
        pos2 = msg.find_first_of(wxChar('\n'), pos);
        if (line-- == 0) {
            return msg.substr(pos, pos2 - pos);
            break;
        }
        if (pos2 == wxString::npos)
            break;
    }

    size_t i, indent = 0;
    for (i = 0; i < str.Length(); ++i) {
        if (str[i] == wxChar('\t'))
            indent += 8;
        else if (str[i] == wxChar(' '))
            ++indent;
        else
            break;
    }

    if (i > 0) {
        str = str.Mid(i);
        str.Pad(indent, ' ', false);
    }

    return str;
}

wxString CDiagConsoleList::GetItemRawMsg(long item) const
{
    if (item < 0 || (size_t)item >= m_Items.size())
        return wxEmptyString;

    auto index = x_GetIndex(item);

    wxString str = s_GetLine(m_Cache.GetItemMessage(get<0>(index)), get<1>(index));
    if (get<1>(index) > 0)
        return str;

    int image = m_Cache.GetItemImage(get<0>(index));

    wxString retVal;
    switch(image) {
    case 0:
        retVal = wxT("Error: ") + str;
        break;
    case 1:
        retVal = wxT("Warning: ") + str;
        break;
    case 2:
        retVal = wxT("Info: ") + str;
        break;
    default:
        retVal = wxT("Other: ") + str;
        break;
    }

    return retVal;
}

void CDiagConsoleList::OnContextMenu( wxContextMenuEvent& anEvt )
{
    void* data = anEvt.GetClientData();
    unique_ptr<wxMenu> menu(data ? (wxMenu*)data : new wxMenu());

    AppendMenuItems(*menu);

    if (ShouldPropagateContextMenu()){
        anEvt.SetClientData(menu.get());
        anEvt.Skip();
        return;
    }

    anEvt.SetClientData(NULL);

    CleanupSeparators(*menu);
    PopupMenu(menu.get());
}

void CDiagConsoleList::AppendMenuItems( wxMenu& aMenu )
{
    aMenu.Append( wxID_SEPARATOR, wxT("Edit") );
    CUICommandRegistry::GetInstance().AppendMenuItem( aMenu, wxID_COPY );
}


int CDiagConsoleList::OnGetItemImage(long item) const
{
    if (item < 0 || (size_t)item >= m_Items.size())
        return -1;

    auto index = x_GetIndex(item);
    return (get<1>(index) > 0) ? 4 : m_Cache.GetItemImage(get<0>(index));
}

wxString CDiagConsoleList::OnGetItemText(long item, long column) const
{
    if (item < 0 || (size_t)item >= m_Items.size())
        return wxEmptyString;

    auto index = x_GetIndex(item);

    if (column == 1)
        return (get<1>(index) > 0) ? wxString() : m_Cache.GetItemTime(get<0>(index));
    else if (column == 2)
        return (get<1>(index) > 0) ? wxString() : m_Cache.GetItemSource(get<0>(index));

    if (column != 0)
        return wxEmptyString;

    return s_GetLine(m_Cache.GetItemMessage(get<0>(index)), get<1>(index));
}

void CDiagConsoleList::x_ReloadMessages()
{
    wxBusyCursor wait;
    CwxLogDiagHandler& log_handler = *CwxLogDiagHandler::GetInstance();
    size_t i, total, buffered;
    log_handler.GetMsgCount(total, buffered);

    m_Items.clear();

    for (i = total - buffered; i < total; ++i) {
        CwxLogDiagHandler::SMessage msg;
        if (!log_handler.GetMessage(i, msg, total)) {
            m_Items.clear();
            continue;
        }

        int flag = 0;
        switch(msg.severity) {
            case eDiag_Error:
                flag = eShowErrors;
                break;
            case eDiag_Warning:
                flag = eShowWarnings;
                break;
            case eDiag_Info:
                flag = eShowInfo;
                break;
            default:
                flag = eShowOther;
                break;
        }
        flag |= msg.wxMsg ? eShowWxWdidgets : eShowNCBI;

        if ((flag&m_Filter&eShowFlags) && (flag&m_Filter&eShowNCBIWxWdidgets)) {
            size_t lines = s_CountLines(wxString::FromUTF8(msg.message.c_str()));
            while (lines--)
                m_Items.push_back(i);
        }
    }

    SetItemCount((long)m_Items.size());
    SetColumnWidth(1, wxLIST_AUTOSIZE);
    if (m_Items.size() > 0)
        EnsureVisible((long)(m_Items.size()-1));
    Refresh();
}

void CDiagConsoleList::x_OnNewItem(CEvent*)
{
    CwxLogDiagHandler& log_handler = *CwxLogDiagHandler::GetInstance();
    size_t i, total, buffered;
    log_handler.GetMsgCount(total, buffered);

    bool update = false;
    if (m_Items.size() > 0) {
        for (i = 0; i < m_Items.size() && m_Items[i] < total - buffered; ++i);
        if (i > 0) {
            m_Items.erase(m_Items.begin(), m_Items.begin() + i);
            update = true;
        }
    }

    i = (m_Items.size() > 0) ? m_Items.back() + 1 : total - buffered;

    for (; i < total; ++i) {
        CwxLogDiagHandler::SMessage msg;
        if (!log_handler.GetMessage(i, msg, total)) {
            x_ReloadMessages();
            return;
        }

        int flag = 0;
        switch(msg.severity) {
            case eDiag_Error:
                flag = eShowErrors;
                break;
            case eDiag_Warning:
                flag = eShowWarnings;
                break;
            case eDiag_Info:
                flag = eShowInfo;
                break;
            default:
                flag = eShowOther;
                break;
        }
        flag |= msg.wxMsg ? eShowWxWdidgets : eShowNCBI;

        if ((flag&m_Filter&eShowFlags) && (flag&m_Filter&eShowNCBIWxWdidgets)) {
            size_t lines = s_CountLines(wxString::FromUTF8(msg.message.c_str()));
            while (lines--)
                m_Items.push_back(i);
            update = true;
        }
    }

    if (update) {
        SetItemCount((long)m_Items.size());
        SetColumnWidth(1, wxLIST_AUTOSIZE);
        if (m_Items.size() > 0)
            EnsureVisible((long)(m_Items.size()-1));
        Refresh();
    }
}

static const char* kFilterTag = "Filter";

void CDiagConsoleList::LoadSettings()
{
    if( ! m_RegPath.empty())   {
        CGuiRegistry& gui_reg = CGuiRegistry::GetInstance();
        CRegistryReadView view = gui_reg.GetReadView(m_RegPath);

        m_Filter = view.GetInt(kFilterTag, (int)m_Filter);
        x_ReloadMessages();
    }
}

void CDiagConsoleList::SaveSettings() const
{
    if( ! m_RegPath.empty())   {
        CGuiRegistry& gui_reg = CGuiRegistry::GetInstance();
        CRegistryWriteView view = gui_reg.GetWriteView(m_RegPath);

        view.Set(kFilterTag, (int)m_Filter);
    }
}

///////////////////////////////////////////////////////////////////////////////
/// CDiagConsoleCache

int CDiagConsoleCache::GetItemImage(size_t index)
{
    return x_GetItem(index).m_Image;
}

wxString CDiagConsoleCache::GetItemMessage(size_t index)
{
    return x_GetItem(index).m_Message;
}

wxString CDiagConsoleCache::GetItemSource(size_t index)
{
    return x_GetItem(index).m_Source;
}

wxString CDiagConsoleCache::GetItemTime(size_t index)
{
    return x_GetItem(index).m_Time;
}

const CDiagConsoleCache::CItem& CDiagConsoleCache::x_GetItem(size_t index)
{
    if (++m_AccessTime == numeric_limits<unsigned long>::max()) {
        m_Data.clear();
        m_AccessTime = 0;
    }

    TData::iterator it = m_Data.find(index);
    if (it != m_Data.end()) {
        it->second.m_AccessTime = m_AccessTime;
        return it->second;
    }

    if (m_Size == m_Data.size()) {
        TData::iterator it_erase = it = m_Data.begin();
        for (++it;it != m_Data.end(); ++it) {
            if (it->second.m_AccessTime < it_erase->second.m_AccessTime)
                it_erase = it;
        }
        m_Data.erase(it_erase);
    }

    CwxLogDiagHandler& log_handler = *CwxLogDiagHandler::GetInstance();
    CwxLogDiagHandler::SMessage msg;
    size_t total;
    CItem item;

    if (log_handler.GetMessage(index, msg, total)) {
        wxString text = wxString::FromUTF8(msg.message.c_str());

        item.m_Message = text;
        item.m_Time = ToWxString(msg.time.AsString("h:m:s"));
        item.m_Source = ToWxString(msg.source); 

        switch(msg.severity) {
        case eDiag_Error:
            item.m_Image = 0;
            break;
        case eDiag_Warning:
            item.m_Image = 1;
            break;
        case eDiag_Info:
            item.m_Image = 2;
            break;
        default:
            item.m_Image = 3;
            break;
        }
    }
    else {
        item.m_Message = wxT("Wrong item requested (problem with the console view).");
        item.m_Image = -1;
    }

    return m_Data[index] = item;
}

END_NCBI_SCOPE

