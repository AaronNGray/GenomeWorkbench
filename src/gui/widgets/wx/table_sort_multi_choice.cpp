/*  $Id: table_sort_multi_choice.cpp 40969 2018-05-04 17:00:04Z katargir $
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
 * Authors: Yury Voronov
 *
 * File Description:
 *
 */

#include <ncbi_pch.hpp>

#include <gui/widgets/wx/table_sort_multi_choice.hpp>
#include <gui/widgets/wx/wx_utils.hpp>

#include <wx/sizer.h>
#include <gui/widgets/wx/hyperlink.hpp>
#include <wx/scrolwin.h>
#include <wx/button.h>
#include <wx/stattext.h>



BEGIN_NCBI_SCOPE


BEGIN_EVENT_TABLE(CTableSortMultiChoice, wxPanel)
    EVT_HYPERLINK(wxID_ANY, CTableSortMultiChoice::OnLink)
    EVT_BUTTON(wxID_ANY, CTableSortMultiChoice::OnBrowseBtn)
END_EVENT_TABLE()


CTableSortMultiChoice::CTableSortMultiChoice()
{
    Init();
}


CTableSortMultiChoice::CTableSortMultiChoice(
    wxWindow* parent,
    wxWindowID id,
    const wxPoint& pos,
    const wxSize& size,
    long style,
    const wxString& name
){
    Init();
    Create( parent, id, pos, size, style, name );
}


CTableSortMultiChoice::~CTableSortMultiChoice()
{
}


void CTableSortMultiChoice::Init()
{
    m_ScrollWnd = NULL;
    m_LevelSizer = NULL;

    m_LastChildID = eFirstControl;
}


void CTableSortMultiChoice::Create(
    wxWindow* parent,
    wxWindowID id,
    const wxPoint& pos,
    const wxSize& size,
    long style,
    const wxString& name
){
    wxPanel::Create( parent, id, pos, size, style, name );

    wxBoxSizer* sizer = new wxBoxSizer( wxVERTICAL );

    // Create scrolled window
    m_ScrollWnd = new wxScrolledWindow(this);
    m_ScrollWnd->SetWindowStyle(wxBORDER_STATIC);
    m_ScrollWnd->SetScrollRate(4, 4);
    sizer->Add(m_ScrollWnd, 1, wxEXPAND);

    // Setup Flex Sizer that will hold levels table
    m_LevelSizer = new wxFlexGridSizer(0, 3);
    m_LevelSizer->AddGrowableCol(0);
    m_LevelSizer->SetFlexibleDirection(wxHORIZONTAL);
    m_ScrollWnd->SetSizer(m_LevelSizer);

    // add one empty row to the level table
    x_AddLevelRow("");
    m_LevelSizer->SetRows(1);

    // Create links at the bottom
    wxBoxSizer* link_sizer = new wxBoxSizer(wxHORIZONTAL);
    sizer->Add(link_sizer, 0, wxEXPAND, 0);

    // Create Add Levels link
    wxHyperlinkCtrl* add_link =
        new wxHyperlinkCtrl(this, eAddFilesLink, wxT("Add levels"), wxT("add_levels"));
    add_link->SetVisitedColour(add_link->GetNormalColour());
    link_sizer->Add(add_link, 0, wxALIGN_LEFT | wxALL, 4);

    wxHyperlinkCtrl* delete_link =
        new wxHyperlinkCtrl(this, eDeleteAllLevelsLink, wxT("Remove all levels from list"), wxT("delete_levels"));
    delete_link->SetVisitedColour(delete_link->GetNormalColour());
    link_sizer->Add(delete_link, 0, wxALIGN_LEFT | wxALL, 4);


    link_sizer->AddStretchSpacer(1);

    //link_sizer->Add(new wxButton(this, eLoadBtn, "Load"), 0, wxTOP, 4);

    SetSizer(sizer);
}


void CTableSortMultiChoice::GetLevels( vector< pair<int,bool> >& levels ) const
{
    for( size_t i = 0;  i < m_Inputs.size();  i++ ) {
        const wxComboBox* input = m_Inputs[i];
        string level = ToStdString(input->GetValue());
        if( ! level.empty()) {
            levels.push_back(level);
        }
    }
}


void CTableSortMultiChoice::SetLevels( const vector<string>& levels )
{
    x_DeleteAllFilenames();

    AddFilenames(levels);
}


void CTableSortMultiChoice::OnLink( wxHyperlinkEvent& event )
{
    int id = event.GetId();
    if( id == eAddFilesLink ){
        x_OnAddFilenames();
    } else if(id == eDeleteAllFilesLink)    {
        x_DeleteAllFilenames();
    } else {
        x_DeleteFilename(id);
    }
}


void CTableSortMultiChoice::x_AddLevelRow(const string& level)
{
    int row_n = m_LevelSizer->GetRows();

    // Create Level Combo
    wxComboBox* input = new wxComboBox(m_ScrollWnd, m_LastChildID++);
    input->SetValue(ToWxString(level));
    m_LevelSizer->Add(input, 1, wxEXPAND | wxALL, 2);
    m_Inputs.push_back(input);

    // Create "Choose Level" button
    wxButton* btn = new wxButton(m_ScrollWnd, m_LastChildID++, wxT("..."), wxDefaultPosition, wxSize(30, 20));
    btn->SetToolTip(wxT("Choose a level..."));
    m_LevelSizer->Add(btn, 0, wxEXPAND | wxALL, 2);

    // create "Delete" link
    wxHyperlinkCtrl* link = new CHyperlink(m_ScrollWnd, m_LastChildID++, wxT("Delete"), wxT("delete"));
    link->SetVisitedColour(link->GetNormalColour());
    m_LevelSizer->Add(link, 0, wxALIGN_BOTTOM | wxALL, 5);

    // add a row to Flex sizer (if needed)
    m_LevelSizer->SetRows(row_n + 1);
}


void CTableSortMultiChoice::x_DeleteLevel(int link_id)
{
    int row_n = m_LevelSizer->GetRows();

    if(row_n > 1)   {
        // delete a row
        wxWindow* input = FindWindowById(link_id - 2, m_ScrollWnd);
        vector<wxCombobox*>::iterator it = std::find(m_Inputs.begin(), m_Inputs.end(), input);
        m_Inputs.erase(it);
        input->Destroy();

        wxWindow* btn = FindWindowById(link_id - 1, m_ScrollWnd);
        btn->Destroy();

        wxWindow* link = FindWindowById(link_id, m_ScrollWnd);
        link->Destroy();

        m_LevelSizer->SetRows(row_n - 1);
        m_ScrollWnd->FitInside();
    } else {
        // there is only one row left - keep it but remove the value
        m_Inputs[0]->SetValue(wxT(""));
    }
}


void CTableSortMultiChoice::x_OnAddLevels()
{
    _ASSERT(m_DlgParent != NULL  &&  ! m_DlgTitle.empty());

    /*
    long style = wxFD_OPEN | wxFD_FILE_MUST_EXIST | wxFD_MULTIPLE;
    wxFileDialog dlg(m_DlgParent,
                     ToWxString(m_DlgTitle),
                     ToWxString(m_DefaultDir),
                     wxT(""),
                     ToWxString(m_Wildcard),
                     style);

    if(dlg.ShowModal() == wxID_OK)  {
        wxArrayString paths;
        dlg.GetPaths(paths);

        // remember the directory
        m_DefaultDir = ToStdString(dlg.GetDirectory());

        vector<string> filenames;
        FromArrayString(paths, filenames);

        AddFilenames(filenames);
    }
    */
}


void CTableSortMultiChoice::AddLevels(const vector<string>& paths)
{
    // add new files, if we have empty row in the end - use them
    int i = m_Inputs.size() - 1;
    while(i >= 0)   {
        wxCombobox* input = m_Inputs[i];
        string s = ToStdString(input->GetValue());
        if( ! s.empty())    {
            break;
        } else  {
            i--;
        }
    }
    // copy levels to existing rows
    int i_p = 0; // index of the path
    int paths_n = paths.size();
    for ( int j = i + 1;  j < (int) m_Inputs.size()  &&  i_p < paths_n;  j++, i_p++ )    {
        m_Inputs[j]->SetValue(ToWxString(paths[i_p]));
    }
    // add more rows
    for( ;  i_p < paths_n;  i_p++ )   {
        x_AddFilenameRow(paths[i_p].c_str());
    }
    m_ScrollWnd->FitInside();
}


void CTableSortMultiChoice::x_DeleteAllFilenames()
{
    m_Inputs.clear();
    m_ScrollWnd->DestroyChildren();
    m_LevelSizer->SetRows(0);

    x_AddLevelRow(""); // add one empty line

    m_ScrollWnd->FitInside();
}


void CTableSortMultiChoice::OnBrowseBtn(wxCommandEvent& event)
{
    int id = event.GetId();
    if(id == eLoadBtn)  {
        event.Skip(); // let parents handl it
        return;
    }

    int input_id = id - 1;
    for( size_t i = 0;  i < m_Inputs.size();  i++ ) {
        wxCombobox* input = m_Inputs[i];

        if(input->GetId() == input_id)  {
            // that was a browser button - show File Open dialog
            string path = ToStdString(input->GetValue());

            _ASSERT(m_DlgParent != NULL  &&  ! m_DlgTitle.empty());

            long style = wxFD_OPEN | wxFD_FILE_MUST_EXIST;
            wxFileDialog dlg(m_DlgParent,
                             ToWxString(m_DlgTitle),
                             ToWxString(m_DefaultDir),
                             wxT(""),
                             ToWxString(m_Wildcard),
                             style);
            if(path.empty()) {
                path = m_DefaultDir;
            }
            dlg.SetPath(ToWxString(path));

            if(dlg.ShowModal() == wxID_OK)  {
                input->SetValue(dlg.GetPath());

                m_DefaultDir = ToStdString(dlg.GetDirectory());
            }
            return;
        }
    }
}

END_NCBI_SCOPE
