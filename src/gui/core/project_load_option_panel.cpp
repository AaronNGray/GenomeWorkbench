/*  $Id: project_load_option_panel.cpp 39528 2017-10-05 15:27:37Z katargir $
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

#include <corelib/ncbitime.hpp>

////@begin includes
////@end includes

#include <gui/core/project_load_option_panel.hpp>

#include <gui/widgets/wx/multi_file_input.hpp>
#include <gui/widgets/wx/wx_utils.hpp>
#include <gui/widgets/wx/compressed_file.hpp>

#include <wx/sizer.h>
#include <wx/stattext.h>
#include <wx/statbox.h>
#include <wx/button.h>
#include <wx/msgdlg.h>
#include <wx/icon.h>
#include <wx/filename.h>
#include <wx/txtstrm.h>
#include <wx/sstream.h>


BEGIN_NCBI_SCOPE


IMPLEMENT_DYNAMIC_CLASS( CProjectLoadOptionPanel, wxPanel )

BEGIN_EVENT_TABLE( CProjectLoadOptionPanel, wxPanel )
////@begin CProjectLoadOptionPanel event table entries
    EVT_LISTBOX_DCLICK( ID_RECENT_LIST, CProjectLoadOptionPanel::OnRecentListDoubleClicked )
    EVT_HTML_LINK_CLICKED( ID_RECENT_LIST, CProjectLoadOptionPanel::OnRecentListLinkClicked )

////@end CProjectLoadOptionPanel event table entries
END_EVENT_TABLE()


CProjectLoadOptionPanel::CProjectLoadOptionPanel()
{
    Init();
}


CProjectLoadOptionPanel::CProjectLoadOptionPanel( wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style )
{
    Init();
    Create(parent, id, pos, size, style);
}


bool CProjectLoadOptionPanel::Create( wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style )
{
////@begin CProjectLoadOptionPanel creation
    wxPanel::Create( parent, id, pos, size, style );

    CreateControls();
    if (GetSizer())
    {
        GetSizer()->SetSizeHints(this);
    }
    Centre();
////@end CProjectLoadOptionPanel creation
    return true;
}

CProjectLoadOptionPanel::~CProjectLoadOptionPanel()
{
////@begin CProjectLoadOptionPanel destruction
////@end CProjectLoadOptionPanel destruction
}


void CProjectLoadOptionPanel::Init()
{
////@begin CProjectLoadOptionPanel member initialisation
    m_FilePanel = NULL;
    m_MRUListBox = NULL;
////@end CProjectLoadOptionPanel member initialisation

    m_FileMRU = NULL;
    m_SelectedFilenames.clear();
}


void CProjectLoadOptionPanel::CreateControls()
{
////@begin CProjectLoadOptionPanel content construction
    CProjectLoadOptionPanel* itemPanel1 = this;

    wxBoxSizer* itemBoxSizer2 = new wxBoxSizer(wxVERTICAL);
    itemPanel1->SetSizer(itemBoxSizer2);

    wxBoxSizer* itemBoxSizer3 = new wxBoxSizer(wxHORIZONTAL);
    itemBoxSizer2->Add(itemBoxSizer3, 0, wxGROW, 5);

    wxStaticText* itemStaticText4 = new wxStaticText( itemPanel1, wxID_STATIC, _("Please select Projects."), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer3->Add(itemStaticText4, 0, wxGROW|wxALL, 5);

    m_FilePanel = new CMultiFileInput( itemPanel1, ID_FILE_PANEL, wxDefaultPosition, wxSize(200, 100), wxSUNKEN_BORDER|wxTAB_TRAVERSAL );
    itemBoxSizer2->Add(m_FilePanel, 1, wxGROW|wxALL, 5);

    wxStaticText* itemStaticText6 = new wxStaticText( itemPanel1, wxID_STATIC, _("Recently used Projects:"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer2->Add(itemStaticText6, 0, wxALIGN_LEFT|wxLEFT|wxRIGHT|wxTOP, 5);

    wxArrayString m_MRUListBoxStrings;
    m_MRUListBox = new wxSimpleHtmlListBox( itemPanel1, ID_RECENT_LIST, wxDefaultPosition, wxSize(200, 100), m_MRUListBoxStrings, wxHLB_MULTIPLE|wxSUNKEN_BORDER );
    itemBoxSizer2->Add(m_MRUListBox, 1, wxGROW|wxALL, 5);

////@end CProjectLoadOptionPanel content construction

    m_FilePanel->SetFocus();

    static wxString wildcard(wxT("Projects (*.gbp)|*.gbp|Projects(*.gbp)|*.gbp"));

    m_FilePanel->SetDlgAttrs(wxT("Open Project"), wxEmptyString, wildcard, this);
}


string sFormatTimePeriod(CTime& t_now, CTime& t_before)
{
    CTimeFormat format("b D, H:m p");
    return t_before.ToLocalTime().AsString(format);
}


void CProjectLoadOptionPanel::x_FileMRUList()
{
    _ASSERT(m_FileMRU);

    if(m_FileMRU)   {
        CTime now(CTime::eCurrent);

        typedef TFileMRU::TTimeToTMap TMap;
        const TMap& map =  m_FileMRU->GetMap();

        for( TMap::const_reverse_iterator it = map.rbegin();  it != map.rend();  ++it) {
            time_t t = it->first;
            CTime tm(t);

            wxString name, ext, filename = it->second;
            if (m_InvalidFiles.find(filename) != m_InvalidFiles.end())
                continue;

            wxFileName::SplitPath(filename, 0, &name, &ext);
            if( ! ext.empty())  {
                name += wxT(".") + ext;
            }

            wxStringOutputStream strstrm;
            wxTextOutputStream os(strstrm);

            os << wxT("<b><a href=\"") << (wxInt32)m_MRUListBox->GetCount() << wxT("\">")
               << name << wxT("</a></b>&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;")
               << wxT("<font color=#808080>last&nbsp;used&nbsp;&nbsp;") << ToWxString(sFormatTimePeriod(now, tm)) << wxT("</font><br />")
               << wxT("<small>") << filename << wxT("</small>");

            m_MRUListBox->Append(strstrm.GetString());
            m_MRUFilenames.push_back(filename);
        }
    }
}


void CProjectLoadOptionPanel::SetMRU(const TFileMRU& file_mru)
{
    m_FileMRU = &file_mru;

    m_MRUListBox->Clear();
    m_MRUFilenames.clear();

    x_FileMRUList();
}


void CProjectLoadOptionPanel::OnRecentListDoubleClicked( wxCommandEvent& event )
{
    wxCommandEvent new_event(wxEVT_COMMAND_BUTTON_CLICKED, wxID_FORWARD);
    new_event.SetEventObject(this);
    AddPendingEvent(new_event);
}


void CProjectLoadOptionPanel::OnRecentListLinkClicked( wxHtmlLinkEvent& event )
{
    const wxHtmlLinkInfo info = event.GetLinkInfo();
    long index;
    if (info.GetHref().ToLong(&index))
        m_MRUListBox->SetSelection(index);

    wxCommandEvent new_event(wxEVT_COMMAND_BUTTON_CLICKED, wxID_FORWARD);
    new_event.SetEventObject(this);
    AddPendingEvent(new_event);
}



void CProjectLoadOptionPanel::x_GetSelectedMRUFilenames(vector<wxString>& filenames)
{
    unsigned long cookie;
    int index = m_MRUListBox->GetFirstSelected(cookie);

    while(index != wxNOT_FOUND) {
        filenames.push_back(m_MRUFilenames[index]);
        index = m_MRUListBox->GetNextSelected(cookie);
    }
}


inline wxString sFilenameSetToString(const set<wxString>& filenames)
{
    wxString s;
    ITERATE(set<wxString>, it, filenames)    {
        s += *it;
    }
    return s;
}


bool CProjectLoadOptionPanel::OnFinish()
{
    vector<wxString> filenames;
    m_FilePanel->GetFilenames(filenames);
    x_GetSelectedMRUFilenames(filenames);
    return x_Finish(filenames);
}

// finish this page
bool CProjectLoadOptionPanel::x_Finish(const vector<wxString>& filenames)
{
    wxString err_msg;
    set<wxString> invalid_files;

    if( ! filenames.empty())    {
        // verify that there is no duplication
        // verify that files exist

        set<wxString> processed;
        set<wxString> duplicates;

        int ws_count = 0;
        int prj_count = 0;

        // gather statistics
        for( size_t i = 0;  i < filenames.size();  i++ )    {
            wxString filename = filenames[i];
            if(processed.find(filename) != processed.end())    {
                duplicates.insert(filename);
            } else {
                processed.insert(filename);

                if (CCompressedFile::FileExists(filename)) {
                    wxString ext;
                    wxFileName::SplitPath(filename, 0, 0, &ext);
                    if (ext == wxT("gbw"))   {
                        ws_count++;
                    } else if (ext == wxT("gbp")) {
                        prj_count++;
                    } else {
                        invalid_files.insert(filename);
                    }
                } else {
                    invalid_files.insert(filename);
                }
            }
            //LOG_POST("  " << filename);
        }

        // analyze statistics
        if( ! duplicates.empty()) {
            err_msg = wxT("The following files are duplicated :\n\n");
            err_msg += sFilenameSetToString(duplicates);
        } else if( ! invalid_files.empty()) {
            err_msg = wxT("The following files are invalid or do not exist:\n\n");
            err_msg += sFilenameSetToString(invalid_files);
        } else {
            if(ws_count >= 1)    {
                err_msg = wxT("This version of Genome Workbench can't handle workspace files.");
            } else {
                _ASSERT((prj_count > 0));
            }
        }
    } else {
        err_msg = wxT("Please select a Project to load!");
    }
    if( ! err_msg.empty())  {
        wxMessageBox(err_msg, wxT("Errors in input"),
                     wxOK | wxICON_ERROR, this);

        if( ! invalid_files.empty()) {
            m_InvalidFiles.insert(invalid_files.begin(), invalid_files.end());
            m_MRUListBox->DeselectAll();
            m_MRUListBox->Clear();
            m_MRUFilenames.clear();
            x_FileMRUList();
        }
    } else {
        /// everything is Ok, go to the next step
        m_SelectedFilenames = filenames;
        return true;
        //wxCommandEvent event(wxEVT_COMMAND_BUTTON_CLICKED, wxID_FORWARD);
        //event.SetEventObject(this);
        //AddPendingEvent(event);
    }
    return false;
}


void CProjectLoadOptionPanel::GetProjectFilenames(vector<wxString>& filenames)
{
    filenames = m_SelectedFilenames;
    return;
}


bool CProjectLoadOptionPanel::ShowToolTips()
{
    return true;
}


wxBitmap CProjectLoadOptionPanel::GetBitmapResource( const wxString& name )
{
    // Bitmap retrieval
////@begin CProjectLoadOptionPanel bitmap retrieval
    wxUnusedVar(name);
    return wxNullBitmap;
////@end CProjectLoadOptionPanel bitmap retrieval
}


wxIcon CProjectLoadOptionPanel::GetIconResource( const wxString& name )
{
    // Icon retrieval
////@begin CProjectLoadOptionPanel icon retrieval
    wxUnusedVar(name);
    return wxNullIcon;
////@end CProjectLoadOptionPanel icon retrieval
}


END_NCBI_SCOPE
