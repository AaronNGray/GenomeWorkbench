/*  $Id: file_load_option_panel.cpp 44286 2019-11-22 15:51:34Z katargir $
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

#include <gui/core/file_load_option_panel.hpp>
#include <gui/core/ui_file_load_manager.hpp>
#include <gui/widgets/wx/multi_file_input.hpp>
#include <gui/widgets/wx/htmlwin.hpp>
#include <gui/widgets/wx/wx_utils.hpp>
#include <gui/widgets/wx/compressed_file.hpp>

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
#include <wx/checkbox.h>
#include <wx/settings.h>


BEGIN_NCBI_SCOPE


IMPLEMENT_DYNAMIC_CLASS( CFileLoadOptionPanel, wxPanel )

BEGIN_EVENT_TABLE(CFileLoadOptionPanel, wxPanel)
////@begin CFileLoadOptionPanel event table entries
    EVT_CHOICE( ID_CHOICE, CFileLoadOptionPanel::OnFormatListSelected )

    EVT_HTML_LINK_CLICKED( ID_HTMLWINDOW, CFileLoadOptionPanel::OnMRUWindowLinkClicked )

////@end CFileLoadOptionPanel event table entries
    EVT_BUTTON(CMultiFileInput::eLoadBtn, CFileLoadOptionPanel::OnLoadFiles)
END_EVENT_TABLE()


CFileLoadOptionPanel::CFileLoadOptionPanel()
    : m_Manager(), m_FileMRU(), m_MRUFileClicked(false), m_MRUFormat(-1)
{
    Init();
}


CFileLoadOptionPanel::CFileLoadOptionPanel( wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style )
    : m_Manager(), m_FileMRU(), m_MRUFileClicked(false), m_MRUFormat(-1)
{
    Init();
    Create(parent, id, pos, size, style);
}


bool CFileLoadOptionPanel::Create( wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style )
{
////@begin CFileLoadOptionPanel creation
    wxPanel::Create( parent, id, pos, size, style );

    CreateControls();
    if (GetSizer())
    {
        GetSizer()->SetSizeHints(this);
    }
    Centre();
////@end CFileLoadOptionPanel creation
    return true;
}


CFileLoadOptionPanel::~CFileLoadOptionPanel()
{
////@begin CFileLoadOptionPanel destruction
////@end CFileLoadOptionPanel destruction
}


void CFileLoadOptionPanel::Init()
{
////@begin CFileLoadOptionPanel member initialisation
    m_FormatList = NULL;
    m_CheckFormat = NULL;
    m_FileInput = NULL;
    m_MRUWindow = NULL;
////@end CFileLoadOptionPanel member initialisation
    m_CurrFormat = -1;
}


void CFileLoadOptionPanel::CreateControls()
{
////@begin CFileLoadOptionPanel content construction
    CFileLoadOptionPanel* itemPanel1 = this;

    wxBoxSizer* itemBoxSizer2 = new wxBoxSizer(wxVERTICAL);
    itemPanel1->SetSizer(itemBoxSizer2);

    wxBoxSizer* itemBoxSizer3 = new wxBoxSizer(wxHORIZONTAL);
    itemBoxSizer2->Add(itemBoxSizer3, 0, wxGROW|wxALL, 0);

    wxStaticText* itemStaticText4 = new wxStaticText( itemPanel1, wxID_STATIC, _("File Format:"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer3->Add(itemStaticText4, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxArrayString m_FormatListStrings;
    m_FormatList = new wxChoice( itemPanel1, ID_CHOICE, wxDefaultPosition, wxDefaultSize, m_FormatListStrings, 0 );
    itemBoxSizer3->Add(m_FormatList, 1, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    m_CheckFormat = new wxCheckBox( itemPanel1, ID_CHECKBOX6, _("Verify that the file data match the selected file format"), wxDefaultPosition, wxDefaultSize, 0 );
    m_CheckFormat->SetValue(false);
    itemBoxSizer2->Add(m_CheckFormat, 0, wxALIGN_LEFT|wxLEFT|wxRIGHT, 5);

    wxStaticText* itemStaticText7 = new wxStaticText( itemPanel1, wxID_STATIC, _("Filenames or URLs:"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer2->Add(itemStaticText7, 0, wxALIGN_LEFT|wxLEFT|wxRIGHT|wxTOP, 5);

    m_FileInput = new CMultiFileInput( itemPanel1, ID_FILE_INPUT, wxDefaultPosition, wxSize(300, 100), wxSUNKEN_BORDER|wxTAB_TRAVERSAL );
    itemBoxSizer2->Add(m_FileInput, 1, wxGROW|wxALL, 5);

    wxStaticText* itemStaticText9 = new wxStaticText( itemPanel1, wxID_STATIC, _("Recently used Files:"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer2->Add(itemStaticText9, 0, wxALIGN_LEFT|wxLEFT|wxRIGHT|wxTOP, 5);

    m_MRUWindow = new CwxHtmlWindow( itemPanel1, ID_HTMLWINDOW, wxDefaultPosition, wxSize(200, 150), wxHW_SCROLLBAR_AUTO|wxSUNKEN_BORDER|wxHSCROLL|wxVSCROLL );
    itemBoxSizer2->Add(m_MRUWindow, 1, wxGROW|wxALL, 5);

////@end CFileLoadOptionPanel content construction

    m_MRUWindow->SetBorders(2);
    m_MRUWindow->SetStandardFonts();
}


void CFileLoadOptionPanel::SetManager(CFileLoadManager* manager)
{
    m_Manager = manager;
}

void CFileLoadOptionPanel::OnActivatePanel()
{
    m_MRUFileClicked = false;
    m_MRUFormat = -1;
    m_MRUFile.empty();
}

void CFileLoadOptionPanel::SetMRU(const TFileMRU& file_mru)
{
    m_FileMRU = &file_mru;
    m_MRUFilenames.clear();

    x_FillMRUList();
}

static string sFormatTimePeriod(CTime& t_now, CTime& t_before)
{
    CTimeFormat format("b D, H:m p");
    return t_before.ToLocalTime().AsString(format);
}

void CFileLoadOptionPanel::x_FillMRUList()
{
    _ASSERT(m_FileMRU);

    if (m_FileMRU) {
        CTime now(CTime::eCurrent);

        typedef TFileMRU::TTimeToTMap TMap;
        const TMap& map =  m_FileMRU->GetMap();

        wxStringOutputStream strstrm;
        wxTextOutputStream os(strstrm);

        os << wxT("<html><body>");

        for( TMap::const_reverse_iterator it = map.rbegin();  it != map.rend();  ++it) {
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

    m_MRUWindow->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_WINDOW));
}

void CFileLoadOptionPanel::x_LoadMRUFile(int index)
{
    if (index == wxNOT_FOUND)
        return;

    wxString filename      = m_MRUFilenames[index].GetFileName();
    string manager_id      = m_MRUFilenames[index].GetFileLoaderId();
    wxString manager_label = m_MRUFilenames[index].GetFileLoaderLabel();

    if (!CCompressedFile::FileExists(filename)) {
        wxString err_msg = wxT("The file: \"") + filename + wxT("\" doesn't exist.");
        wxMessageBox(err_msg, wxT("Error"), wxOK | wxICON_ERROR, this);
        return;
    }

    int format_index = -1;
    for (size_t i = 0; i < m_Formats.size(); ++i) {
        if (m_Formats[i].m_Id == manager_id) {
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

    wxCommandEvent event(wxEVT_COMMAND_BUTTON_CLICKED, wxID_FORWARD);
    event.SetEventObject(this);
    AddPendingEvent(event);
}

void CFileLoadOptionPanel::SetFormats(const vector<CFormatDescriptor>& formats)
{
    m_Formats = formats;

    size_t n = m_Formats.size();

    for(size_t i = 0;  i < n;  i++) {
        if (!m_Formats[i].m_Hidden) {
            m_FormatList->Append(m_Formats[i].m_Label, (void*)i);
        }
    }

    if (m_FormatList->GetCount() > 0) {
        m_FormatList->SetSelection(0);
        int format = (int)(intptr_t)m_FormatList->GetClientData(0);
        x_SetCurrentFormat(format);
    }
}


int CFileLoadOptionPanel::GetSelectedFormat()
{
    return m_MRUFileClicked ? m_MRUFormat : m_CurrFormat;
}

void CFileLoadOptionPanel::GetSelectedFormat(wxString& selected_format)
{
    selected_format = m_FormatList->GetString(m_FormatList->GetSelection());
}


void CFileLoadOptionPanel::SelectFormat(int format)
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

void CFileLoadOptionPanel::SetCheckFormat(bool value)
{
    m_CheckFormat->SetValue(value);
}


bool CFileLoadOptionPanel::GetCheckFormat()
{
    return m_CheckFormat->IsChecked();
}


void CFileLoadOptionPanel::GetFilenames(vector<wxString>& filenames) const
{
    if (m_MRUFileClicked)
        filenames.push_back(m_MRUFile);
    else {
        m_FileInput->GetFilenames(filenames);
    }
}


void CFileLoadOptionPanel::SetFilenames(const vector<wxString>& filenames)
{
    m_FileInput->SetFilenames(filenames);
}


bool CFileLoadOptionPanel::ShowToolTips()
{
    return true;
}


void CFileLoadOptionPanel::OnLoadFiles(wxCommandEvent& event)
{
    bool ok = true;
    if(ok)  {
        wxCommandEvent event(wxEVT_COMMAND_BUTTON_CLICKED, wxID_FORWARD);
        event.SetEventObject(this);
        AddPendingEvent(event);
    }
}


void CFileLoadOptionPanel::OnFormatListSelected( wxCommandEvent& event )
{
    int index = m_FormatList->GetSelection();
    if (index >= 0) {
        int format = (int)(intptr_t)m_FormatList->GetClientData((unsigned int)index);
        x_SetCurrentFormat(format);
        m_Manager->OnFormatChanged();
    }
}


void CFileLoadOptionPanel::x_SetCurrentFormat(int index)
{
    m_CurrFormat = index;
    m_FileInput->SetWildcard(m_Formats[index].m_WildCards);
    m_FileInput->SetSingleMode(m_Formats[index].m_SingleFile);
}

void CFileLoadOptionPanel::OnMRUWindowLinkClicked( wxHtmlLinkEvent& event )
{
    const wxHtmlLinkInfo info = event.GetLinkInfo();
    long index;
    if (info.GetHref().ToLong(&index))
        x_LoadMRUFile((int)index);
}

wxBitmap CFileLoadOptionPanel::GetBitmapResource( const wxString& name )
{
////@begin CFileLoadOptionPanel bitmap retrieval
    wxUnusedVar(name);
    return wxNullBitmap;
////@end CFileLoadOptionPanel bitmap retrieval
}


wxIcon CFileLoadOptionPanel::GetIconResource( const wxString& name )
{
////@begin CFileLoadOptionPanel icon retrieval
    wxUnusedVar(name);
    return wxNullIcon;
////@end CFileLoadOptionPanel icon retrieval
}

END_NCBI_SCOPE
