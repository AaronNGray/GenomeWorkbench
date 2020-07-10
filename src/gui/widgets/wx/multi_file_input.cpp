/*  $Id: multi_file_input.cpp 40970 2018-05-04 17:11:56Z katargir $
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
 * Authors:  Andrey Yazhuk
 *
 * File Description:
 *
 */

#include <ncbi_pch.hpp>

#include <gui/widgets/wx/multi_file_input.hpp>
#include <gui/widgets/wx/advanced_file_input.hpp>

#include <wx/sizer.h>
#include <gui/widgets/wx/hyperlink.hpp>
#include <wx/scrolwin.h>
#include <wx/bmpbuttn.h>
#include <wx/filedlg.h>
#include <wx/stattext.h>
#include <wx/statbox.h>
#include <wx/artprov.h>


BEGIN_NCBI_SCOPE

CMultiFileInput::CDropTarget::CDropTarget(CMultiFileInput& input)
: m_Input(input)
{
    SetDataObject(new wxFileDataObject);
}

wxDragResult CMultiFileInput::CDropTarget::OnDragOver(wxCoord WXUNUSED(x), wxCoord WXUNUSED(y),
                                                      wxDragResult def)
{
    return wxDragCopy;
}

wxDragResult CMultiFileInput::CDropTarget::OnData(wxCoord x, wxCoord y, wxDragResult def)
{
    if (!GetData())
        return wxDragNone;

    wxFileDataObject *dobj = (wxFileDataObject *)m_dataObject;
    wxArrayString filenames = dobj->GetFilenames();

    vector<wxString> names;

    size_t n = filenames.GetCount();
    for (size_t i = 0; i < n; i++)
        names.push_back(filenames[i]);

    // On windows, dropping the file comes as a wxDragMove, and on Mac (wxCocoa)
    // the same operation causes a wxDragLink
    if (def == wxDragMove || def == wxDragLink || def == wxDragCopy) {
        m_Input.AddFilenames(names);
        return wxDragCopy;
    }
    return wxDragError;
}


BEGIN_EVENT_TABLE(CMultiFileInput, wxPanel)
    EVT_HYPERLINK(wxID_ANY, CMultiFileInput::OnLink)
    EVT_BUTTON(wxID_ANY, CMultiFileInput::OnBrowseBtn)
END_EVENT_TABLE()


CMultiFileInput::CMultiFileInput() : m_SingleMode(false)
{
    Init();
}


CMultiFileInput::CMultiFileInput(wxWindow* parent,
                                 wxWindowID id,
                                 const wxPoint& pos,
                                 const wxSize& size,
                                 long style,
                                 const wxString& name)
: m_SingleMode(false)
{
    Init();
    Create(parent, id, pos, size, style, name);
}


CMultiFileInput::~CMultiFileInput()
{
}


void CMultiFileInput::Init()
{
    m_ScrollWnd = NULL;
    m_FileSizer = NULL;
    m_LinkSizer = NULL;

    m_DlgTitle = wxT("Select Files");
    m_DlgParent = this;

    m_LastChildID = eFirstControl;
}


void CMultiFileInput::Create(wxWindow* parent,
                             wxWindowID id,
                             const wxPoint& pos,
                             const wxSize& size,
                             long style,
                             const wxString& name)
{
    wxPanel::Create(parent, id, pos, size, style, name);

    wxBoxSizer* sizer = new wxBoxSizer(wxVERTICAL);

    // Create scrolled window
    m_ScrollWnd = new wxScrolledWindow(this);
    //m_ScrollWnd->SetWindowStyle(wxBORDER_STATIC);
    m_ScrollWnd->SetScrollRate(4, 4);
    sizer->Add(m_ScrollWnd, 1, wxEXPAND);

    // Setup Flex Sizer that will hold filename table
    m_FileSizer = new wxFlexGridSizer(0, 3, 0, 0);
    m_FileSizer->AddGrowableCol(0);
    m_FileSizer->SetFlexibleDirection(wxHORIZONTAL);
    m_ScrollWnd->SetSizer(m_FileSizer);

    // add one empty row to the file table
    x_AddFilenameRow(wxEmptyString);
    m_FileSizer->SetRows(1);

    // Create links at the bottom
    m_LinkSizer = new wxBoxSizer(wxHORIZONTAL);
    sizer->Add(m_LinkSizer, 0, wxEXPAND, 0);    

    wxHyperlinkCtrl* delete_link =
        new wxHyperlinkCtrl(this, eDeleteAllFilesLink, wxT("Remove all files from list"), wxT("delete_files"));
    delete_link->SetVisitedColour(delete_link->GetNormalColour());
    m_LinkSizer->Add(delete_link, 0, wxALL, 4);

    m_LinkSizer->AddStretchSpacer(1);

    wxStaticText* drop_text = new wxStaticText(this, wxID_ANY, wxT("Drop files here"));
    m_LinkSizer->Add(drop_text, 0, wxALL, 4);

    //m_LinkSizer->Add(new wxButton(this, eLoadBtn, "Load"), 0, wxTOP, 4);

    SetSizer(sizer);

    SetDropTarget(new CMultiFileInput::CDropTarget(*this));
}



void CMultiFileInput::SetDlgTitle(const wxString& title)
{
    m_DlgTitle = title;
}


void CMultiFileInput::SetDefaultDir(const wxString& dir)
{
    m_DefaultDir = dir;
}


wxString CMultiFileInput::GetDefaultDir() const
{
    return m_DefaultDir;
}


void CMultiFileInput::SetWildcard(const wxString& wildcard)
{
    m_Wildcard = wildcard;
}


wxString CMultiFileInput::GetWildcard() const
{
    return m_Wildcard;
}


void CMultiFileInput::SetDlgParent(wxWindow* parent)
{
    m_DlgParent = parent;
}


void CMultiFileInput::SetDlgAttrs(const wxString& title, const wxString& dir,
                                  const wxString& wildcard, wxWindow* parent)
{
    m_DlgTitle = title;
    m_DefaultDir = dir;
    m_Wildcard = wildcard;
    m_DlgParent = parent;
}

void CMultiFileInput::SetSingleMode (bool single)
{
    if (single) {
        x_SetSingleMode();
    }
    else {
        x_SetMultiMode();
    }
}

void CMultiFileInput::x_SetSingleMode()
{
    if (m_SingleMode)
        return;

    m_SingleMode = true;

    GetFilenames(m_SaveFilenames);
    x_DeleteAllFilenames();

    if (!m_SaveFilenames.empty()) {
        // Don't need text edit events needed by SetValue in versions 2.9+
#if (wxMAJOR_VERSION == 2 && wxMINOR_VERSION < 9)
        m_Inputs[0]->SetValue(m_SaveFilenames[0]);
#else
        m_Inputs[0]->ChangeValue(m_SaveFilenames[0]);
#endif
        x_SendFileTxtChangedEvt(m_Inputs[0], m_SaveFilenames[0]);
    }

    wxWindow* wnd = FindWindow(eDeleteAllFilesLink);
    if (wnd) wnd->Show(false);

    m_LinkSizer->Layout();
}

void CMultiFileInput::x_SetMultiMode()
{
    if (!m_SingleMode)
        return;

    m_SingleMode = false;

    if (!m_SaveFilenames.empty()) { // when the control is created it is empty
        if (!m_Inputs[0]->GetValue().empty())
            m_SaveFilenames.erase(m_SaveFilenames.begin());

        AddFilenames(m_SaveFilenames);
        m_SaveFilenames.clear();
    }

    wxWindow* wnd = FindWindow(eDeleteAllFilesLink);
    if (wnd) wnd->Show(true);

    m_LinkSizer->Layout();
}

void CMultiFileInput::GetFilenames(vector<wxString>& filenames) const
{
    for( size_t i = 0;  i < m_Inputs.size();  i++ ) {
        const CAdvancedFileInput* input = m_Inputs[i];
        wxString file = input->GetValue();
        if (!file.empty())
            filenames.push_back(file);
    }
}


void CMultiFileInput::SetFilenames(const vector<wxString>& filenames)
{
    x_DeleteAllFilenames();

    AddFilenames(filenames);
}


void CMultiFileInput::OnLink(wxHyperlinkEvent& event)
{
    int id = event.GetId();
    if (event.GetId() == eDeleteAllFilesLink) {
        x_DeleteAllFilenames();
    }
    else {
        x_DeleteFilename(id);
    }
}


void CMultiFileInput::x_AddFilenameRow(const wxString& filename)
{
    int row_n = m_FileSizer->GetRows();

    // add a row to Flex sizer (if needed)
    m_FileSizer->SetRows(row_n + 1);

    // Create File Combo
    CAdvancedFileInput* input = new CAdvancedFileInput(m_ScrollWnd, m_LastChildID++);

// Don't need text edit events needed by SetValue in versions 2.9+
#if (wxMAJOR_VERSION == 2 && wxMINOR_VERSION < 9)
    input->SetValue(filename);
#else
    input->ChangeValue(filename);
#endif
    x_SendFileTxtChangedEvt(input, filename);

    m_FileSizer->Add(input, 1, wxEXPAND | wxALL, 2);
    m_Inputs.push_back(input);

    // Create "Choose File" button
    wxButton* btn = new wxBitmapButton(m_ScrollWnd, m_LastChildID++, wxArtProvider::GetBitmap(wxT("menu::open")), wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW);
    btn->SetToolTip(wxT("Choose a file..."));
    m_FileSizer->Add(btn, 0, wxEXPAND | wxALL, 2);

    // create "Delete" link
    wxHyperlinkCtrl* link = new CHyperlink(m_ScrollWnd, m_LastChildID++, wxT("Delete"), wxT("delete"));
    link->SetVisitedColour(link->GetNormalColour());
    m_FileSizer->Add(link, 0, wxALIGN_BOTTOM | wxALL, 5);
}


void CMultiFileInput::x_DeleteFilename(int link_id)
{
    int row_n = m_FileSizer->GetRows();

    if(row_n > 1)   {
        // delete a row
        wxWindow* input = FindWindowById(link_id - 2, m_ScrollWnd);
        vector<CAdvancedFileInput*>::iterator it = std::find(m_Inputs.begin(), m_Inputs.end(), input);
        m_Inputs.erase(it);
        input->Destroy();

        wxWindow* btn = FindWindowById(link_id - 1, m_ScrollWnd);
        btn->Destroy();

        wxWindow* link = FindWindowById(link_id, m_ScrollWnd);
        link->Destroy();

        m_FileSizer->SetRows(row_n - 1);
        m_ScrollWnd->FitInside();
    } else {
        // there is only one row left - keep it but remove the value
      
        // Don't need text edit events needed by SetValue in versions 2.9+
#if (wxMAJOR_VERSION == 2 && wxMINOR_VERSION < 9)
        m_Inputs[0]->SetValue(wxEmptyString);
#else
        m_Inputs[0]->ChangeValue(wxEmptyString);
#endif
        x_SendFileTxtChangedEvt(m_Inputs[0], wxEmptyString);
    }
}


void CMultiFileInput::x_OnAddFilenames()
{
    _ASSERT(m_DlgParent != NULL  &&  ! m_DlgTitle.empty());

    long style = wxFD_OPEN | wxFD_FILE_MUST_EXIST;
    if (!m_SingleMode) style |= wxFD_MULTIPLE;
    wxFileDialog dlg(m_DlgParent, m_DlgTitle, m_DefaultDir, wxEmptyString, m_Wildcard, style);

    if (dlg.ShowModal() == wxID_OK)  {
        wxArrayString paths;
        dlg.GetPaths(paths);

        // remember the directory
        m_DefaultDir = dlg.GetDirectory();

        vector<wxString> filenames;
        size_t n = paths.GetCount();
        for (size_t i = 0; i < n; i++)
            filenames.push_back (paths[i]);

        AddFilenames(filenames);
    }
}


void CMultiFileInput::AddFilenames(const vector<wxString>& paths)
{
    if (m_SingleMode && !paths.empty()) {
        
        // Don't need text edit events needed by SetValue in versions 2.9+
#if (wxMAJOR_VERSION == 2 && wxMINOR_VERSION < 9)
        m_Inputs[0]->SetValue(paths[0]);
#else
        m_Inputs[0]->ChangeValue(paths[0]);
#endif
        x_SendFileTxtChangedEvt(m_Inputs[0], paths[0]);

        return;
    }

    // add new files, if we have empty row in the end - use them
    int i = (int)(m_Inputs.size() - 1);
    while(i >= 0)   {
        CAdvancedFileInput* input = m_Inputs[i];
        wxString s = input->GetValue();
        if( ! s.empty())    {
            break;
        } else  {
            i--;
        }
    }
    // copy filenames to existing rows
    int i_p = 0; // index of the path
    int paths_n = (int)paths.size();

    for ( int j = i + 1;  j < (int) m_Inputs.size()  &&  i_p < paths_n;  j++, i_p++ )    {
        // Don't need text edit events needed by SetValue in versions 2.9+
#if (wxMAJOR_VERSION == 2 && wxMINOR_VERSION < 9)
        m_Inputs[j]->SetValue(paths[i_p]);
#else
        m_Inputs[j]->ChangeValue(paths[i_p]);
#endif
        x_SendFileTxtChangedEvt(m_Inputs[j], paths[i_p]);
    }
    // add more rows
    for( ;  i_p < paths_n;  i_p++ )   {
        x_AddFilenameRow(paths[i_p]);
    }
    m_ScrollWnd->FitInside();
}


void CMultiFileInput::x_DeleteAllFilenames()
{
    m_Inputs.clear();
    m_ScrollWnd->DestroyChildren();
    m_FileSizer->SetRows(0);

    x_AddFilenameRow(wxEmptyString); // add one empty line

    m_ScrollWnd->FitInside();
}


void CMultiFileInput::OnBrowseBtn(wxCommandEvent& event)
{

    int id = event.GetId();
    if(id == eLoadBtn)  {
        event.Skip(); // let parents handl it
        return;
    }

    int input_id = id - 1;
    for( size_t i = 0;  i < m_Inputs.size();  i++ ) {
        if (m_Inputs[i]->GetId() == input_id)  {
            x_OnAddFilenames();
            break;
        }
    }
}

// Send a change event for any objects that want to see filename changes
// as they happen.
void CMultiFileInput::x_SendFileTxtChangedEvt(CAdvancedFileInput* input, 
                                              const wxString& txt)
{
    wxCommandEvent txt_change_evt(CAdvancedFileInput::s_InputTxtChangedEvt);
    txt_change_evt.SetString(txt);
    txt_change_evt.SetClientData(input);
    GetEventHandler()->ProcessEvent(txt_change_evt);
}

END_NCBI_SCOPE
