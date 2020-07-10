/*  $Id: macro_applysrc_table.cpp 44649 2020-02-14 14:58:13Z asztalos $
* ===========================================================================
*
*                            PUBLIC DOMAIN NOTICE
*               National Center for Biotechnology Information
*
*  This software / database is a "United States Government Work" under the
*  terms of the United States Copyright Act.It was written as part of
*  the author's official duties as a United States Government employee and
*  thus cannot be copyrighted.This software / database is freely available
*  to the public for use.The National Library of Medicine and the U.S.
*  Government have not placed any restriction on its use or reproduction.
*
*  Although all reasonable efforts have been taken to ensure the accuracy
*  and reliability of the software and data, the NLM and the U.S.
*  Government do not and cannot warrant the performance or results that
*  may be obtained by using this software or data.The NLM and the U.S.
*  Government disclaim all warranties, express or implied, including
*  warranties of performance, merchantability or fitness for any particular
*  purpose.
*
*  Please cite the author in any work or product based on this material.
*
* ===========================================================================
*
* Authors: Andrea Asztalos
*
*/

#include <ncbi_pch.hpp>
#include <util/line_reader.hpp>
#include <gui/objutils/macro_util.hpp>
#include <gui/widgets/wx/wx_utils.hpp>
#include <gui/widgets/wx/unfocused_controls.hpp>
#include <gui/widgets/edit/singlequal_panel.hpp>
#include <gui/widgets/edit/macro_editor_context.hpp>
#include <gui/widgets/edit/macro_applysrc_table.hpp>

#include <wx/sizer.h>
#include <wx/stattext.h>
#include <wx/statbox.h>
#include <wx/scrolwin.h>
#include <wx/textctrl.h>
#include <wx/filedlg.h>
#include <wx/radiobut.h>
#include <wx/listctrl.h>

BEGIN_NCBI_SCOPE
USING_SCOPE(objects);

/*
 * CMacroApplySrcTablePanel type definition
 */

IMPLEMENT_DYNAMIC_CLASS( CMacroApplySrcTablePanel, wxPanel)


/*
 * CMacroApplySrcTablePanel event table definition
 */

BEGIN_EVENT_TABLE( CMacroApplySrcTablePanel, wxPanel)

    EVT_HYPERLINK(wxID_ANY, CMacroApplySrcTablePanel::OnDelete)
    EVT_CHECKBOX(ID_APPLYTBLCB1, CMacroApplySrcTablePanel::OnDelimiterSelected)
    EVT_CHECKBOX(ID_APPLYTBLCB2, CMacroApplySrcTablePanel::OnDelimiterSelected)
    EVT_CHECKBOX(ID_APPLYTBLCB3, CMacroApplySrcTablePanel::OnDelimiterSelected)
    EVT_CHECKBOX(ID_APPLYTBLCB4, CMacroApplySrcTablePanel::OnDelimiterSelected)
    EVT_RADIOBUTTON(wxID_ANY, CMacroApplySrcTablePanel::OnDelimiterSelected)
    EVT_TEXT(ID_APPLYSRCTBLFILE, CMacroApplySrcTablePanel::OnNewFileEntered)
    EVT_TEXT(ID_APPLYTBLTXT1, CMacroApplySrcTablePanel::OnDelimiterSelected)
END_EVENT_TABLE()


/*
 * CMacroApplySrcTablePanel constructors
 */

CMacroApplySrcTablePanel::CMacroApplySrcTablePanel()
{
    Init();
}

CMacroApplySrcTablePanel::CMacroApplySrcTablePanel( wxWindow* parent, EMacroFieldType field_type, wxWindowID id, const wxPoint& pos, const wxSize& size, long style )
    : m_FieldType(field_type)
{
    Init();
    Create(parent, id, pos, size, style);
}


/*
 * CMacroApplySrcTablePanel creator
 */

bool CMacroApplySrcTablePanel::Create( wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style )
{
////@begin CMacroApplySrcTablePanel creation
    SetExtraStyle(wxWS_EX_VALIDATE_RECURSIVELY);
    wxPanel::Create( parent, id, pos, size, style );

    CreateControls();
    if (GetSizer())
    {
        GetSizer()->SetSizeHints(this);
    }
    Centre();
////@end CMacroApplySrcTablePanel creation
    return true;
}


/*
 * CMacroApplySrcTablePanel destructor
 */

CMacroApplySrcTablePanel::~CMacroApplySrcTablePanel()
{
////@begin CMacroApplySrcTablePanel destruction
////@end CMacroApplySrcTablePanel destruction
}


/*
 * Member initialisation
 */

void CMacroApplySrcTablePanel::Init()
{
    m_ScrolledWindow = nullptr;
    m_Sizer = nullptr;
    m_TablePreview = nullptr;
    m_Filename = nullptr;
    m_QualLabel = nullptr;
    m_Tabdel = nullptr;
    m_Semidel = nullptr;
    m_Commadel = nullptr;
    m_Spacedel = nullptr;
    m_Otherdel = nullptr;
    m_OtherText = nullptr;
    m_MergeDel = nullptr;
    m_SplitFirstCol = nullptr;
    m_ConvertMulti = m_MergeTwoCols = nullptr;
}

/*
 * Control creation for CMacroApplySrcTablePanel
 */

void CMacroApplySrcTablePanel::CreateControls()
{    
////@begin CMacroApplySrcTablePanel content construction
    CMacroApplySrcTablePanel* itemPanel1 = this;

    wxBoxSizer* itemBoxSizer2 = new wxBoxSizer(wxVERTICAL);
    itemPanel1->SetSizer(itemBoxSizer2);

    wxBoxSizer* itemBoxSizer4 = new wxBoxSizer(wxHORIZONTAL);
    itemBoxSizer2->Add(itemBoxSizer4, 0, wxGROW|wxALL, 0);

    wxFlexGridSizer* itemFlexGridSizer5 = new wxFlexGridSizer(0, 1, 0, 0);
    itemBoxSizer4->Add(itemFlexGridSizer5, 0, wxGROW|wxALL, 0);

    wxBoxSizer* itemBoxSizer6 = new wxBoxSizer(wxHORIZONTAL);
    itemFlexGridSizer5->Add(itemBoxSizer6, 0, wxGROW|wxALL, 0);

    wxStaticText* itemStaticText7 = new wxStaticText( itemPanel1, wxID_STATIC, _("Table:"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer6->Add(itemStaticText7, 0, wxALIGN_CENTER_VERTICAL|wxRIGHT|wxBOTTOM, 5);

    m_Filename = new wxTextCtrl( itemPanel1, ID_APPLYSRCTBLFILE, wxEmptyString, wxDefaultPosition, wxSize(261, -1), 0 );
    itemBoxSizer6->Add(m_Filename, 1, wxALIGN_CENTER_VERTICAL|wxBOTTOM, 5);

    wxBitmapButton* itemBitmapButton9 = new wxBitmapButton( itemPanel1, ID_APPLYSRCTBLBTMBTN, wxArtProvider::GetBitmap(wxT("menu::open")), wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW );
    itemBoxSizer6->Add(itemBitmapButton9, 0, wxALIGN_CENTER_VERTICAL|wxLEFT|wxBOTTOM, 5);
    itemBitmapButton9->Bind(wxEVT_BUTTON, &CMacroApplySrcTablePanel::OnOpenFile, this);

    wxBoxSizer* itemBoxSizer1 = new wxBoxSizer(wxHORIZONTAL);
    itemFlexGridSizer5->Add(itemBoxSizer1, 0, wxGROW|wxALIGN_CENTER_VERTICAL|wxALL, 0);

    m_QualLabel = new wxStaticText( itemPanel1, wxID_STATIC, _("Qualifiers"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer1->Add(m_QualLabel, 0, wxALIGN_CENTER_VERTICAL|wxRIGHT|wxTOP|wxBOTTOM, 5);

    itemBoxSizer1->Add(110, 5, 0, wxALIGN_CENTER_VERTICAL|wxLEFT|wxRIGHT|wxTOP, 0);

    wxStaticText* itemStaticText5 = new wxStaticText( itemPanel1, wxID_STATIC, _("Column"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer1->Add(itemStaticText5, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxStaticText* itemStaticText8 = new wxStaticText( itemPanel1, wxID_STATIC, _("Match"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer1->Add(itemStaticText8, 0, wxALIGN_CENTER_VERTICAL|wxRIGHT|wxBOTTOM|wxTOP, 5);

    wxHyperlinkCtrl* itemHyperlinkCtrl10 = new wxHyperlinkCtrl( itemPanel1, ID_APPLYSRCTBLADDQUAL, _("Add qualifier"), wxEmptyString, wxDefaultPosition, wxDefaultSize, wxHL_DEFAULT_STYLE );
    itemBoxSizer1->Add(itemHyperlinkCtrl10, 0, wxALIGN_CENTER_VERTICAL|wxTOP|wxBOTTOM, 5);
    itemHyperlinkCtrl10->SetForegroundColour(wxColour(128, 128, 128));
    itemHyperlinkCtrl10->Bind(wxEVT_HYPERLINK, &CMacroApplySrcTablePanel::OnHyperlinkClicked, this);

    m_ScrolledWindow = new wxScrolledWindow( itemPanel1, ID_APPLYSRCTBLWND, wxDefaultPosition, wxSize(110, 96), wxSUNKEN_BORDER|wxHSCROLL|wxVSCROLL );
    itemFlexGridSizer5->Add(m_ScrolledWindow, 0, wxGROW|wxALIGN_CENTER_VERTICAL|wxALL, 2);
    m_Sizer = new wxFlexGridSizer(0, 2, 0, 0);
    m_ScrolledWindow->SetSizer(m_Sizer);

    x_AddEmptyRow();

    wxStaticBox* itemStaticBoxSizer2Static = new wxStaticBox(itemPanel1, wxID_ANY, _("Delimiter"));
    wxStaticBoxSizer* itemStaticBoxSizer2 = new wxStaticBoxSizer(itemStaticBoxSizer2Static, wxVERTICAL);
    itemBoxSizer4->Add(itemStaticBoxSizer2, 0, wxALIGN_TOP|wxLEFT|wxRIGHT|wxBOTTOM, 5);

    m_Tabdel = new wxRadioButton( itemPanel1, ID_APPLYTBLRBTN1, _("Tab"), wxDefaultPosition, wxDefaultSize, wxRB_GROUP );
    m_Tabdel->SetValue(true);
    itemStaticBoxSizer2->Add(m_Tabdel, 0, wxALIGN_LEFT|wxALL, 5);

    m_Semidel = new wxRadioButton( itemPanel1, ID_APPLYTBLRBTN2, _("Semicolon"), wxDefaultPosition, wxDefaultSize, 0 );
    m_Semidel->SetValue(false);
    itemStaticBoxSizer2->Add(m_Semidel, 0, wxALIGN_LEFT|wxALL, 5);

    m_Commadel = new wxRadioButton( itemPanel1, ID_APPLYTBLRBTN3, _("Comma"), wxDefaultPosition, wxDefaultSize, 0 );
    m_Commadel->SetValue(false);
    itemStaticBoxSizer2->Add(m_Commadel, 0, wxALIGN_LEFT|wxALL, 5);

    m_Spacedel = new wxRadioButton( itemPanel1, ID_APPLYTBLRBTN4, _("Space"), wxDefaultPosition, wxDefaultSize, 0 );
    m_Spacedel->SetValue(false);
    itemStaticBoxSizer2->Add(m_Spacedel, 0, wxALIGN_LEFT|wxALL, 5);

    wxBoxSizer* itemBoxSizer10 = new wxBoxSizer(wxHORIZONTAL);
    itemStaticBoxSizer2->Add(itemBoxSizer10, 0, wxALIGN_LEFT|wxALL, 0);

    m_Otherdel = new wxRadioButton( itemPanel1, ID_APPLYTBLRBTN5, _("Other"), wxDefaultPosition, wxDefaultSize, 0 );
    m_Otherdel->SetValue(false);
    itemBoxSizer10->Add(m_Otherdel, 0, wxALIGN_CENTER_VERTICAL|wxLEFT|wxTOP|wxBOTTOM, 5);

    m_OtherText = new wxTextCtrl( itemPanel1, ID_APPLYTBLTXT1, wxEmptyString, wxDefaultPosition, wxSize(25, -1), 0 );
    itemBoxSizer10->Add(m_OtherText, 0, wxALIGN_CENTER_VERTICAL|wxLEFT|wxRIGHT, 5);

    wxStaticBox* itemStaticBoxSizer1Static = new wxStaticBox(itemPanel1, wxID_ANY, _("Options"));
    wxStaticBoxSizer* itemStaticBoxSizer1 = new wxStaticBoxSizer(itemStaticBoxSizer1Static, wxVERTICAL);
    itemBoxSizer4->Add(itemStaticBoxSizer1, 0, wxALIGN_TOP|wxRIGHT|wxBOTTOM, 5);

    m_MergeDel = new wxCheckBox( itemPanel1, ID_APPLYTBLCB1, _("Merge adjacent delimiters"), wxDefaultPosition, wxDefaultSize, 0 );
    m_MergeDel->SetValue(false);
    itemStaticBoxSizer1->Add(m_MergeDel, 0, wxALIGN_LEFT|wxALL, 5);

    m_SplitFirstCol = new wxCheckBox( itemPanel1, ID_APPLYTBLCB2, _("Split first column at first space"), wxDefaultPosition, wxDefaultSize, 0 );
    m_SplitFirstCol->SetValue(false);
    itemStaticBoxSizer1->Add(m_SplitFirstCol, 0, wxALIGN_LEFT|wxALL, 5);

    m_ConvertMulti = new wxCheckBox( itemPanel1, ID_APPLYTBLCB3, _("Convert multispaces to tabs"), wxDefaultPosition, wxDefaultSize, 0 );
    m_ConvertMulti->SetValue(false);
    itemStaticBoxSizer1->Add(m_ConvertMulti, 0, wxALIGN_LEFT|wxALL, 5);

    m_MergeTwoCols = new wxCheckBox( itemPanel1, ID_APPLYTBLCB4, _("Merge first two columns for ID"), wxDefaultPosition, wxDefaultSize, 0 );
    m_MergeTwoCols->SetValue(false);
    itemStaticBoxSizer1->Add(m_MergeTwoCols, 0, wxALIGN_LEFT|wxALL, 5);

    m_TablePreview = new wxListCtrl( itemPanel1, ID_APPLYSRCTBLPREVIEW, wxDefaultPosition, wxSize(300, 120), wxLC_REPORT |wxLC_HRULES |wxLC_VRULES);
    itemBoxSizer2->Add(m_TablePreview, 0, wxGROW|wxTOP|wxBOTTOM, 5);

////@end CMacroApplySrcTablePanel content constructions
}

CSingleQualPanel* CMacroApplySrcTablePanel::x_AddEmptyRow()
{
    CSingleQualPanel* row = new CSingleQualPanel(m_ScrolledWindow, m_FieldType);
    if (m_RowHeight == 0) {
        int row_width;
        row->GetClientSize(&row_width, &m_RowHeight);
    }
    x_AddRowToWindow(row);
    return row;
}

void CMacroApplySrcTablePanel::x_AddRowToWindow(wxWindow* row)
{
    m_Sizer->Add(row, 1, wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL | wxRIGHT, 5);
    wxHyperlinkCtrl* itemHyperLink = new CNoTabHyperlinkCtrl(m_ScrolledWindow, wxID_ANY, wxT("Delete"), wxT("delete"));
    itemHyperLink->SetVisitedColour(itemHyperLink->GetNormalColour());
    m_Sizer->Add(itemHyperLink, 0, wxALIGN_CENTER_VERTICAL | wxLEFT | wxRIGHT, 0);
    m_ScrolledWindow->SetScrollRate(0, m_RowHeight);
    m_ScrolledWindow->FitInside();
    m_ScrolledWindow->Refresh();
}

string CMacroApplySrcTablePanel::GetFilename() const
{
    return ToStdString(m_Filename->GetValue());
}

string CMacroApplySrcTablePanel::GetDelimiter() const
{
    string delimiter;
    if (m_Tabdel->GetValue()) delimiter = "\t";
    else if (m_Semidel->GetValue()) delimiter = ";";
    else if (m_Commadel->GetValue()) delimiter = ",";
    else if (m_Spacedel->GetValue()) delimiter = " ";
    else if (m_Otherdel->GetValue()) delimiter = ToStdString(m_OtherText->GetValue());
    return delimiter;
}

vector<pair<string, string>> CMacroApplySrcTablePanel::GetValues() const
{
    vector<pair<string, string>> values;
    auto list = m_ScrolledWindow->GetChildren();
    bool found_match = false;
    for (auto it = list.begin(); it != list.end(); ++it) {
        CSingleQualPanel* row = dynamic_cast<CSingleQualPanel*>(*it);
        if (row) {
            if (row->IsMatchingRow()) {
                if (found_match) {
                    // there are at least two columns labeled as a match_field
                    values.clear();
                    break;
                }
                found_match = true;
            }
            else {
                auto field = row->m_FieldFirst->GetStringSelection();
                auto col = row->m_Column->GetValue();
                if (!field.empty() && !col.empty())
                    values.push_back(make_pair(ToStdString(field), ToStdString(col)));
            }
        }
    }
    return values;
}

SFieldFromTable CMacroApplySrcTablePanel::GetMatchField() const
{
    auto list = m_ScrolledWindow->GetChildren();
    for (auto it = list.begin(); it != list.end(); ++it) {
        CSingleQualPanel* row = dynamic_cast<CSingleQualPanel*>(*it);
        if (row && row->IsMatchingRow()) {
            return row->GetField();
        }
    }
    return SFieldFromTable(kEmptyStr, kEmptyStr, kEmptyStr, false);
}

/*
 * Should we show tooltips?
 */

bool CMacroApplySrcTablePanel::ShowToolTips()
{
    return true;
}

/*
 * Get bitmap resources
 */

wxBitmap CMacroApplySrcTablePanel::GetBitmapResource( const wxString& name )
{
    // Bitmap retrieval
////@begin CMacroApplySrcTablePanel bitmap retrieval
    wxUnusedVar(name);
    return wxNullBitmap;
////@end CMacroApplySrcTablePanel bitmap retrieval
}

/*
 * Get icon resources
 */

wxIcon CMacroApplySrcTablePanel::GetIconResource( const wxString& name )
{
    // Icon retrieval
////@begin CMacroApplySrcTablePanel icon retrieval
    wxUnusedVar(name);
    return wxNullIcon;
////@end CMacroApplySrcTablePanel icon retrieval
}

void CMacroApplySrcTablePanel::OnOpenFile(wxCommandEvent& event)
{
    if (event.GetId() != ID_APPLYSRCTBLBTMBTN)
        return;

    wxFileDialog dlg(this, wxT("Select table file"), wxEmptyString, wxEmptyString, _("All files (*.*)|*.*"), wxFD_OPEN | wxFD_FILE_MUST_EXIST);

    if (dlg.ShowModal() == wxID_OK) {
        auto mapped_path = CMacroEditorContext::GetInstance().GetAbsolutePath(dlg.GetPath());
        m_Filename->SetValue(mapped_path);
        m_Filename->SetInsertionPoint(m_Filename->GetLastPosition());
        m_Tabdel->SetValue(true);
        m_MergeDel->SetValue(false);
        m_SplitFirstCol->SetValue(false);
        m_ConvertMulti->SetValue(false);
        m_MergeTwoCols->SetValue(false);
    }
}

void CMacroApplySrcTablePanel::OnNewFileEntered(wxCommandEvent& event)
{
    m_Tabdel->SetValue(true);
    m_MergeDel->SetValue(false);
    m_SplitFirstCol->SetValue(false);
    m_ConvertMulti->SetValue(false);
    m_MergeTwoCols->SetValue(false);
    if (!m_OtherText->IsEmpty() && !m_Otherdel->GetValue())
        m_OtherText->Clear();

    x_ProcessFirstLine();
}

void CMacroApplySrcTablePanel::OnDelimiterSelected(wxCommandEvent& event)
{
    x_ProcessFirstLine();
    if (!m_OtherText->IsEmpty() && !m_Otherdel->GetValue())
        m_OtherText->Clear();

    event.Skip();
}

void CMacroApplySrcTablePanel::x_ProcessFirstLine()
{
    const string filename = ToStdString(m_Filename->GetValue());
    CRef<ILineReader> line_reader;
    try {
        line_reader.Reset(ILineReader::New(filename));
        if (line_reader.Empty()) {
            return;
        }
    }
    catch (const CException&) {
        m_TablePreview->DeleteAllColumns();
        m_TablePreview->DeleteAllItems();
        return;
    }

    string delimiter = GetDelimiter();
    NStr::TSplitFlags flags = (m_MergeDel->IsChecked()) ? NStr::fSplit_MergeDelimiters : 0;

    vector<pair<string, int>> fields;
    unsigned int nr_line = 0;
    
    size_t nr_cols = 0;
    m_TablePreview->DeleteAllColumns();
    m_TablePreview->DeleteAllItems();

    // TODO: handle empty values
    while (!line_reader->AtEOF() && nr_line < 30) {
        line_reader->ReadLine();
        string line = line_reader->GetCurrentLine();
        if (line.empty())
            continue;

        if (m_ConvertMulti->IsChecked()) {
            macro::NMacroUtil::ConvertMultiSpaces(line, delimiter);
        }

        vector<string> cols;
        NStr::SplitByPattern(line, delimiter, cols, flags);
        _ASSERT(!cols.empty());


        if (m_MergeTwoCols->IsChecked()) {
            cols[0] = cols[0] + "/" + cols[1];
            cols.erase(cols.begin() + 1);
        }

        if (m_SplitFirstCol->IsChecked()) {
            string first, second;
            NStr::SplitInTwo(cols[0], " ", first, second);
            cols[0] = first;
            cols.insert(cols.begin() + 1, second);
        }


        if (nr_line == 0) {
             nr_cols = cols.size();

             for (size_t i = 0; i < cols.size(); ++i) {
                 fields.emplace_back(cols[i], i + 1);
                 m_TablePreview->AppendColumn(ToWxString(cols[i]));
             }
         } else {
            auto row = m_TablePreview->InsertItem(nr_line, ToWxString(cols[0]));
            for (size_t index = 1; index < nr_cols; ++index) {
                if (index < cols.size())
                    m_TablePreview->SetItem(row, index, ToWxString(cols[index]));
            }
        }
        ++nr_line;
    }

    for (size_t index = 0; index < m_TablePreview->GetColumnCount(); ++index) {
        m_TablePreview->SetColumnWidth(index, wxLIST_AUTOSIZE_USEHEADER);
    }

    Freeze();
    while (m_Sizer->GetItemCount() > 0) {
        size_t pos = 0;
        m_Sizer->GetItem(pos)->DeleteWindows();
        m_Sizer->Remove(pos);
    }

    for (auto& it : fields) {
        CSingleQualPanel* row = x_AddEmptyRow();
        if (it.first == "organism") it.first = "taxname";
        row->SetField(it.first, it.second);
    }

    Thaw();
}


void CMacroApplySrcTablePanel::OnDelete(wxHyperlinkEvent& event)
{
    if (event.GetURL() != wxT("delete")) {
        event.Skip();
        return;
    }

    wxWindow *win = dynamic_cast<wxWindow*>(event.GetEventObject());
    if (!win)
        return;
    wxWindowList& children = m_ScrolledWindow->GetChildren();
    wxWindow* prev = nullptr;
    for (wxWindowList::iterator child = children.begin(); child != children.end(); ++child) {
        if (*child == win) {
            wxSizer *sizer = win->GetContainingSizer();
            win->Destroy();
            prev->Destroy();
            m_Sizer->Remove(sizer);
            m_Sizer->Layout();
            m_ScrolledWindow->FitInside();
            break;
        }
        prev = *child;
    }
    event.Skip();
}

void CMacroApplySrcTablePanel::OnHyperlinkClicked(wxHyperlinkEvent& event)
{
    Freeze();
    x_AddEmptyRow();
    Thaw();
}


END_NCBI_SCOPE

