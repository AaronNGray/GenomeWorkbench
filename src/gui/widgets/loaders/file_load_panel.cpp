/*  $Id: file_load_panel.cpp 41045 2018-05-15 19:36:32Z katargir $
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

#include "file_load_panel.hpp"
#include <gui/widgets/loaders/file_load_wizard.hpp>

#include <gui/widgets/wx/multi_file_input.hpp>
#include <gui/widgets/wx/htmlwin.hpp>
#include <gui/widgets/wx/wx_utils.hpp>

////@begin includes
////@end includes


#include <wx/sizer.h>
#include <wx/choice.h>
#include <wx/statbox.h>
#include <wx/stattext.h>
#include <wx/msgdlg.h>
#include <wx/bitmap.h>
#include <wx/icon.h>
#include <wx/filename.h>
#include <wx/txtstrm.h>
#include <wx/sstream.h>


BEGIN_NCBI_SCOPE


IMPLEMENT_DYNAMIC_CLASS( CFileLoadPanel, wxPanel )

BEGIN_EVENT_TABLE(CFileLoadPanel, wxPanel)
////@begin CFileLoadPanel event table entries
    EVT_CHOICE( ID_CHOICE, CFileLoadPanel::OnFormatListSelected )

    EVT_HTML_LINK_CLICKED( ID_HTMLWINDOW, CFileLoadPanel::OnMRUWindowLinkClicked )

////@end CFileLoadPanel event table entries
    EVT_BUTTON(CMultiFileInput::eLoadBtn, CFileLoadPanel::OnLoadFiles)
END_EVENT_TABLE()


CFileLoadPanel::CFileLoadPanel()
    : m_Manager(), m_CurrFormat(0), m_MRUFileClicked(false), m_MRUFormat(-1)
{
    Init();
}


CFileLoadPanel::CFileLoadPanel( wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style )
    : m_Manager(), m_CurrFormat(0), m_MRUFileClicked(false), m_MRUFormat(-1)
{
    Init();
    Create(parent, id, pos, size, style);
}


bool CFileLoadPanel::Create( wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style )
{
////@begin CFileLoadPanel creation
    wxPanel::Create( parent, id, pos, size, style );

    CreateControls();
    if (GetSizer())
    {
        GetSizer()->SetSizeHints(this);
    }
    Centre();
////@end CFileLoadPanel creation
    return true;
}


CFileLoadPanel::~CFileLoadPanel()
{
////@begin CFileLoadPanel destruction
////@end CFileLoadPanel destruction
}


void CFileLoadPanel::Init()
{
////@begin CFileLoadPanel member initialisation
    m_FormatList = NULL;
    m_FileInput = NULL;
    m_MRUWindow = NULL;
////@end CFileLoadPanel member initialisation
    m_CurrFormat = -1;
}


void CFileLoadPanel::CreateControls()
{
////@begin CFileLoadPanel content construction
    CFileLoadPanel* itemPanel1 = this;

    wxBoxSizer* itemBoxSizer2 = new wxBoxSizer(wxVERTICAL);
    itemPanel1->SetSizer(itemBoxSizer2);

    wxBoxSizer* itemBoxSizer3 = new wxBoxSizer(wxHORIZONTAL);
    itemBoxSizer2->Add(itemBoxSizer3, 0, wxGROW|wxALL, 5);

    wxStaticText* itemStaticText4 = new wxStaticText( itemPanel1, wxID_STATIC, _("File Format:"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer3->Add(itemStaticText4, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxArrayString m_FormatListStrings;
    m_FormatList = new wxChoice( itemPanel1, ID_CHOICE, wxDefaultPosition, wxDefaultSize, m_FormatListStrings, 0 );
    itemBoxSizer3->Add(m_FormatList, 1, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxStaticText* itemStaticText6 = new wxStaticText( itemPanel1, wxID_STATIC, _("Filenames:"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer2->Add(itemStaticText6, 0, wxALIGN_LEFT|wxLEFT|wxRIGHT|wxTOP, 5);

    m_FileInput = new CMultiFileInput( itemPanel1, ID_FILE_INPUT, wxDefaultPosition, wxSize(300, 100), wxSUNKEN_BORDER|wxTAB_TRAVERSAL );
    itemBoxSizer2->Add(m_FileInput, 1, wxGROW|wxALL, 5);

    wxStaticText* itemStaticText8 = new wxStaticText( itemPanel1, wxID_STATIC, _("Recently used Files:"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer2->Add(itemStaticText8, 0, wxALIGN_LEFT|wxLEFT|wxRIGHT|wxTOP, 5);

    m_MRUWindow = new CwxHtmlWindow( itemPanel1, ID_HTMLWINDOW, wxDefaultPosition, wxSize(200, 150), wxHW_SCROLLBAR_AUTO|wxSUNKEN_BORDER|wxHSCROLL|wxVSCROLL );
    itemBoxSizer2->Add(m_MRUWindow, 1, wxGROW|wxALL, 5);

////@end CFileLoadPanel content construction

    m_MRUWindow->SetBorders(2);
    m_MRUWindow->SetStandardFonts();
}

void CFileLoadPanel::SetWorkDir(const wxString& workDir)
{
    if (m_FileInput)
        m_FileInput->SetDefaultDir(workDir);
}

void CFileLoadPanel::SetManager(CFileLoadWizard* manager)
{
	m_Manager = manager;

	const vector<CIRef<IFileLoadPanelClient> >& managers = m_Manager->GetFormatManagers();

    for(size_t i = 0;  i < managers.size();  i++) {
		const IFileLoadPanelClient& manager = *managers[i];
		string id = manager.GetFileLoaderId();
        // We don't want to overcrowd file format list
        // User can explicitly use "Project or Workspace" loader on the left
		if (id == "file_loader_gbench_project")
			continue;

        m_FormatList->Append(manager.GetLabel(), (void*)i);
    }

    if (m_FormatList->GetCount() > 0) {
        m_FormatList->SetSelection(0);
        int format = (int)(intptr_t)m_FormatList->GetClientData(0);
        x_SetCurrentFormat(format);
    }

	x_FillMRUList();
}

void CFileLoadPanel::OnActivatePanel()
{
    m_MRUFileClicked = false;
    m_MRUFormat = -1;
    m_MRUFile.empty();
}

static string sFormatTimePeriod(CTime& t_now, CTime& t_before)
{
    CTimeFormat format("b D, H:m p");
    return t_before.ToLocalTime().AsString(format);
}

void CFileLoadPanel::x_FillMRUList()
{
	_ASSERT(m_Manager);

	CFileLoadMRUList& mru = m_Manager->GetFileMRUList();

    CTime now(CTime::eCurrent);

    const CFileLoadMRUList::TTimeToTMap& map =  mru.GetMap();

    wxStringOutputStream strstrm;
    wxTextOutputStream os(strstrm);

    os << wxT("<html><body>");

    for(CFileLoadMRUList::TTimeToTMap::const_reverse_iterator it = map.rbegin();
													it != map.rend(); ++it) {
        time_t t = it->first;
        CTime tm(t);
        wxString name, ext, filename = it->second.GetFileName();

        wxFileName::SplitPath(filename, 0, &name, &ext);
        if( ! ext.empty())  {
            name += wxT(".") + ext;
        }

        os  << wxT("<b><a href=\"") << (wxInt32)m_MRUFilenames.size() << wxT("\">") << name
            << wxT("</a></b>&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;")
            << wxT("<font color=#808080>last&nbsp;used&nbsp;&nbsp;")
            << ToWxString(sFormatTimePeriod(now, tm)) << wxT("</font><br />")
            << wxT("<small>") << filename << wxT("</small><br />");

        m_MRUFilenames.push_back(it->second);
    }

    os << wxT("</body></html>");

    m_MRUWindow->SetPage(strstrm.GetString());
}

void CFileLoadPanel::x_LoadMRUFile(int index)
{
    if (index == wxNOT_FOUND)
        return;

    wxString filename      = m_MRUFilenames[index].GetFileName();
    string manager_id      = m_MRUFilenames[index].GetFileLoaderId();
    wxString manager_label = m_MRUFilenames[index].GetFileLoaderLabel();

    if (!wxFileName::FileExists(filename)) {
        wxString err_msg = wxT("The file: \"") + filename + wxT("\" doesn't exist.");
        wxMessageBox(err_msg, wxT("Error"), wxOK | wxICON_ERROR, this);
        return;
    }

	const vector<CIRef<IFileLoadPanelClient> >& managers = m_Manager->GetFormatManagers();

    int format_index = -1;
    for (size_t i = 0; i < managers.size(); ++i) {
		const IFileLoadPanelClient& manager = *managers[i];
        if (manager.GetFileLoaderId() == manager_id) {
            format_index = (int)i;
            break;
        }
    }

    if (format_index < 0) {
        wxString err_msg = wxT("File loader \"") + manager_label + wxT("\" not found.");
        wxMessageBox(err_msg, wxT("Error"), wxOK | wxICON_ERROR, this);
        return;
    }

    m_MRUFileClicked = true;
    m_MRUFormat = format_index;
    m_MRUFile = filename;

	m_Manager->OnFormatChanged(m_MRUFormat);

    wxCommandEvent event(wxEVT_COMMAND_BUTTON_CLICKED, wxID_FORWARD);
    event.SetEventObject(this);
    AddPendingEvent(event);
}

void CFileLoadPanel::SelectFormat(int format)
{
    x_SetCurrentFormat(format);

    for (unsigned int i = 0; i < m_FormatList->GetCount(); ++i) {
        int entryFormat = (int)(intptr_t)m_FormatList->GetClientData(i);
        if (entryFormat == format) {
            m_FormatList->SetSelection((int)i);
            break;
        }
    }
}

int CFileLoadPanel::GetSelectedFormat()
{
    return m_MRUFileClicked ? m_MRUFormat : m_CurrFormat;
}

void CFileLoadPanel::GetFilenames(vector<wxString>& filenames) const
{
    if (m_MRUFileClicked)
        filenames.push_back(m_MRUFile);
    else {
        m_FileInput->GetFilenames(filenames);
    }
}


void CFileLoadPanel::SetFilenames(const vector<wxString>& filenames)
{
    m_FileInput->SetFilenames(filenames);
}


bool CFileLoadPanel::ShowToolTips()
{
    return true;
}


void CFileLoadPanel::OnLoadFiles(wxCommandEvent& event)
{
    bool ok = true;
    if(ok)  {
        wxCommandEvent event(wxEVT_COMMAND_BUTTON_CLICKED, wxID_FORWARD);
        event.SetEventObject(this);
        AddPendingEvent(event);
    }
}


void CFileLoadPanel::OnFormatListSelected( wxCommandEvent& event )
{
    int index = m_FormatList->GetSelection();
    if (index >= 0) {
        int format = (int)(intptr_t)m_FormatList->GetClientData((unsigned int)index);
        x_SetCurrentFormat(format);
		m_Manager->OnFormatChanged(format);
    }
}

void CFileLoadPanel::x_SetCurrentFormat(int index)
{
    m_CurrFormat = index;

	const IFileLoadPanelClient& manager = *m_Manager->GetFormatManagers()[m_CurrFormat];
    m_FileInput->SetWildcard(manager.GetFormatWildcard());
    m_FileInput->SetSingleMode(manager.SingleFileLoader());
}

void CFileLoadPanel::OnMRUWindowLinkClicked( wxHtmlLinkEvent& event )
{
    const wxHtmlLinkInfo info = event.GetLinkInfo();
    long index;
    if (info.GetHref().ToLong(&index))
        x_LoadMRUFile((int)index);
}

wxBitmap CFileLoadPanel::GetBitmapResource( const wxString& name )
{
////@begin CFileLoadPanel bitmap retrieval
    wxUnusedVar(name);
    return wxNullBitmap;
////@end CFileLoadPanel bitmap retrieval
}


wxIcon CFileLoadPanel::GetIconResource( const wxString& name )
{
////@begin CFileLoadPanel icon retrieval
    wxUnusedVar(name);
    return wxNullIcon;
////@end CFileLoadPanel icon retrieval
}

END_NCBI_SCOPE
