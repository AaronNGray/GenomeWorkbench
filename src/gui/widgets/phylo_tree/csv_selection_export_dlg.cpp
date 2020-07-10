/*  $Id: csv_selection_export_dlg.cpp 39749 2017-11-01 14:23:15Z katargir $
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
 * Authors: Vladislav Evgeniev
 *
 * File Description:
 *
 */
#include <ncbi_pch.hpp>
////@begin includes
////@end includes

#include <wx/sizer.h>
#include <wx/checkbox.h>
#include <wx/choice.h>
#include <wx/bmpbuttn.h>
#include <wx/filedlg.h>
#include <wx/stattext.h>
#include <wx/textctrl.h>
#include <wx/statbox.h>
#include <wx/artprov.h>

#include <gui/widgets/phylo_tree/csv_selection_export_dlg.hpp>


////@begin XPM images
////@end XPM images

BEGIN_NCBI_SCOPE

IMPLEMENT_DYNAMIC_CLASS( CCSVSelectionExportDlg, CDialog )

BEGIN_EVENT_TABLE( CCSVSelectionExportDlg, CDialog )

////@begin CCSVSelectionExportDlg event table entries
    EVT_BUTTON( ID_BITMAPBUTTON, CCSVSelectionExportDlg::OnSelectFileClick )
    EVT_BUTTON( ID_SELECT_ALL_BTN, CCSVSelectionExportDlg::OnSelectAllBtnClick )
    EVT_BUTTON( ID_DESELECT_ALL_BTN, CCSVSelectionExportDlg::OnDeselectAllBtnClick )
////@end CCSVSelectionExportDlg event table entries

END_EVENT_TABLE()

CCSVSelectionExportDlg::CCSVSelectionExportDlg()
{
    Init();
}

CCSVSelectionExportDlg::CCSVSelectionExportDlg( wxWindow* parent, wxWindowID id, const wxString& caption, const wxPoint& pos, const wxSize& size, long style )
{
    Init();
    Create(parent, id, caption, pos, size, style);
}

bool CCSVSelectionExportDlg::Create( wxWindow* parent, wxWindowID id, const wxString& caption, const wxPoint& pos, const wxSize& size, long style )
{
////@begin CCSVSelectionExportDlg creation
    CDialog::Create( parent, id, caption, pos, size, style );

    CreateControls();
    if (GetSizer())
    {
        GetSizer()->SetSizeHints(this);
    }
    Centre();
////@end CCSVSelectionExportDlg creation
    return true;
}

void CCSVSelectionExportDlg::SetColumnsList(const vector<wxString> &columns)
{
    m_ColumnList->Clear();

    for (size_t i = 0; i < columns.size(); ++i) {
        int newItem = m_ColumnList->Append(columns[i], (void*)(i));
        m_ColumnList->Check(newItem);
    }
    Layout();
    Fit();
}

void CCSVSelectionExportDlg::GetSelectedColumns(vector<wxString> &columns) const
{
    columns.clear();
    wxArrayInt checkedItems;
    m_ColumnList->GetCheckedItems(checkedItems);

    int checked = checkedItems.size();
    columns.reserve(checked);
    for (int i = 0; i < checked; ++i) {
        columns.push_back(m_ColumnList->GetString(checkedItems[i]));
    }
}

CCSVSelectionExportDlg::~CCSVSelectionExportDlg()
{
////@begin CCSVSelectionExportDlg destruction
////@end CCSVSelectionExportDlg destruction
}

void CCSVSelectionExportDlg::Init()
{
////@begin CCSVSelectionExportDlg member initialisation
    m_LeavesOnly = true;
    m_WithHeaders = true;
    m_txtFilename = NULL;
    m_ColumnList = NULL;
////@end CCSVSelectionExportDlg member initialisation
}

static const char* kWithHeaders = "WithHeaders";
static const char* kLeavesOnly = "LeavesOnly";

void CCSVSelectionExportDlg::x_LoadSettings(const CRegistryReadView& view)
{
    m_WithHeaders = view.GetBool(kWithHeaders, m_WithHeaders);
    m_LeavesOnly = view.GetBool(kLeavesOnly, m_LeavesOnly);
}

void CCSVSelectionExportDlg::x_SaveSettings(CRegistryWriteView view) const
{
    view.Set(kWithHeaders, m_WithHeaders);
    view.Set(kLeavesOnly, m_LeavesOnly);
}

void CCSVSelectionExportDlg::CreateControls()
{    
////@begin CCSVSelectionExportDlg content construction
    CCSVSelectionExportDlg* itemCDialog1 = this;

    wxBoxSizer* itemBoxSizer2 = new wxBoxSizer(wxVERTICAL);
    itemCDialog1->SetSizer(itemBoxSizer2);

    wxBoxSizer* itemBoxSizer3 = new wxBoxSizer(wxHORIZONTAL);
    itemBoxSizer2->Add(itemBoxSizer3, 0, wxGROW|wxALL, 5);

    wxStaticText* itemStaticText4 = new wxStaticText( itemCDialog1, wxID_STATIC, _("File Name:"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer3->Add(itemStaticText4, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    m_txtFilename = new wxTextCtrl( itemCDialog1, ID_TEXTCTRL, wxEmptyString, wxDefaultPosition, wxSize(200, -1), 0 );
    itemBoxSizer3->Add(m_txtFilename, 1, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxBitmapButton* itemBitmapButton6 = new wxBitmapButton( itemCDialog1, ID_BITMAPBUTTON, itemCDialog1->GetBitmapResource(wxT("menu::open")), wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW );
    if (CCSVSelectionExportDlg::ShowToolTips())
        itemBitmapButton6->SetToolTip(_("Select File"));
    itemBoxSizer3->Add(itemBitmapButton6, 0, wxGROW|wxALL, 5);

    wxCheckBox* itemCheckBox7 = new wxCheckBox( itemCDialog1, ID_HEADER_CHECKBOX, _("First row to contain headers"), wxDefaultPosition, wxDefaultSize, 0 );
    itemCheckBox7->SetValue(false);
    itemBoxSizer2->Add(itemCheckBox7, 0, wxALIGN_LEFT|wxALL, 5);

    wxCheckBox* itemCheckBox8 = new wxCheckBox( itemCDialog1, ID_LEAVES_ONLY_CHECKBOX, _("Export leaves only"), wxDefaultPosition, wxDefaultSize, 0 );
    itemCheckBox8->SetValue(true);
    itemBoxSizer2->Add(itemCheckBox8, 0, wxALIGN_LEFT|wxALL, 5);

    wxStaticBox* itemStaticBoxSizer9Static = new wxStaticBox(itemCDialog1, wxID_ANY, _("Select fields to export"));
    wxStaticBoxSizer* itemStaticBoxSizer9 = new wxStaticBoxSizer(itemStaticBoxSizer9Static, wxVERTICAL);
    itemBoxSizer2->Add(itemStaticBoxSizer9, 0, wxGROW|wxALL, 5);

    wxArrayString m_ColumnListStrings;
    m_ColumnList = new wxCheckListBox( itemStaticBoxSizer9->GetStaticBox(), ID_CHECKLISTBOX, wxDefaultPosition, wxDefaultSize, m_ColumnListStrings, wxLB_SINGLE );
    itemStaticBoxSizer9->Add(m_ColumnList, 0, wxGROW|wxALL, 5);

    wxBoxSizer* itemBoxSizer11 = new wxBoxSizer(wxHORIZONTAL);
    itemStaticBoxSizer9->Add(itemBoxSizer11, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

    wxButton* itemButton12 = new wxButton( itemStaticBoxSizer9->GetStaticBox(), ID_SELECT_ALL_BTN, _("Select All"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer11->Add(itemButton12, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxButton* itemButton13 = new wxButton( itemStaticBoxSizer9->GetStaticBox(), ID_DESELECT_ALL_BTN, _("Deselect All"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer11->Add(itemButton13, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxStdDialogButtonSizer* itemStdDialogButtonSizer14 = new wxStdDialogButtonSizer;

    itemBoxSizer2->Add(itemStdDialogButtonSizer14, 0, wxALIGN_RIGHT|wxALL, 5);
    wxButton* itemButton15 = new wxButton( itemCDialog1, wxID_OK, _("&OK"), wxDefaultPosition, wxDefaultSize, 0 );
    itemStdDialogButtonSizer14->AddButton(itemButton15);

    wxButton* itemButton16 = new wxButton( itemCDialog1, wxID_CANCEL, _("&Cancel"), wxDefaultPosition, wxDefaultSize, 0 );
    itemStdDialogButtonSizer14->AddButton(itemButton16);

    itemStdDialogButtonSizer14->Realize();

    // Set validators
    m_txtFilename->SetValidator( wxTextValidator(wxFILTER_NONE, & m_FileName) );
    itemCheckBox7->SetValidator( wxGenericValidator(& m_WithHeaders) );
    itemCheckBox8->SetValidator( wxGenericValidator(& m_LeavesOnly) );
////@end CCSVSelectionExportDlg content construction
}

void CCSVSelectionExportDlg::OnSelectFileClick( wxCommandEvent& event )
{
    wxFileDialog fileDlg(this, wxT("Select a file"), wxT(""), wxT(""),
        wxString(wxT("CSV Files (*.csv)|*.csv|")) + wxALL_FILES, wxFD_SAVE | wxFD_OVERWRITE_PROMPT);

    if (fileDlg.ShowModal() == wxID_OK) {
        wxString path = fileDlg.GetPath();
        m_txtFilename->SetValue(path);
    }
}

void CCSVSelectionExportDlg::OnSelectAllBtnClick( wxCommandEvent& event )
{
    for (unsigned i = 0; i < m_ColumnList->GetCount(); ++i) {
        m_ColumnList->Check(i);
    }
}

void CCSVSelectionExportDlg::OnDeselectAllBtnClick( wxCommandEvent& event )
{
    wxArrayInt checkedItems;
    m_ColumnList->GetCheckedItems(checkedItems);

    int checked = checkedItems.size();
    for (int i = 0; i < checked; ++i) {
        m_ColumnList->Check(checkedItems[i], false);
    }
}

bool CCSVSelectionExportDlg::ShowToolTips()
{
    return true;
}
wxBitmap CCSVSelectionExportDlg::GetBitmapResource( const wxString& name )
{
    return wxArtProvider::GetBitmap(name);
}
wxIcon CCSVSelectionExportDlg::GetIconResource( const wxString& name )
{
    // Icon retrieval
////@begin CCSVSelectionExportDlg icon retrieval
    wxUnusedVar(name);
    return wxNullIcon;
////@end CCSVSelectionExportDlg icon retrieval
}

END_NCBI_SCOPE
