/*  $Id: csv_export_dlg.cpp 39749 2017-11-01 14:23:15Z katargir $
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
 * Authors:  Melvin Quintos
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

#include <gui/widgets/wx/csv_export_dlg.hpp>
#include <gui/widgets/wx/wx_utils.hpp>

////@begin XPM images
////@end XPM images

BEGIN_NCBI_SCOPE

/*!
 * CwxCSVExportDlg type definition
 */

IMPLEMENT_DYNAMIC_CLASS( CwxCSVExportDlg, CDialog )


/*!
 * CwxCSVExportDlg event table definition
 */

BEGIN_EVENT_TABLE( CwxCSVExportDlg, CDialog )

////@begin CwxCSVExportDlg event table entries
    EVT_BUTTON( ID_BITMAPBUTTON, CwxCSVExportDlg::OnSelectFileClick )

    EVT_CHECKBOX( ID_SELECTED_CELLS_CHECKBOX, CwxCSVExportDlg::OnSelectedCellsCheckboxClick )

    EVT_BUTTON( ID_SELECT_ALL_BTN, CwxCSVExportDlg::OnSelectAllBtnClick )

    EVT_BUTTON( ID_DESELECT_ALL_BTN, CwxCSVExportDlg::OnDeselectAllBtnClick )

////@end CwxCSVExportDlg event table entries

END_EVENT_TABLE()


/*!
 * CwxCSVExportDlg constructors
 */

CwxCSVExportDlg::CwxCSVExportDlg()
{
    Init();
}

CwxCSVExportDlg::CwxCSVExportDlg( wxWindow* parent, wxWindowID id, const wxString& caption, const wxPoint& pos, const wxSize& size, long style )
{
    Init();
    Create(parent, id, caption, pos, size, style);
}


/*!
 * CwxCSVExportDlg creator
 */

bool CwxCSVExportDlg::Create( wxWindow* parent, wxWindowID id, const wxString& caption, const wxPoint& pos, const wxSize& size, long style )
{
////@begin CwxCSVExportDlg creation
    CDialog::Create( parent, id, caption, pos, size, style );

    CreateControls();
////@end CwxCSVExportDlg creation
    return true;
}

void CwxCSVExportDlg::SetColumnsList(const vector<wxString> &columns)
{
    m_ColumnList->Clear();

    for(size_t i = 0; i < columns.size(); ++i) {
        int newItem = m_ColumnList->Append(columns[i], (void*)(i));
        m_ColumnList->Check(newItem);
    }
}

void CwxCSVExportDlg::GetSelectedColumns(vector<int> &columns) const
{
    columns.clear();
    wxArrayInt checkedItems;
    m_ColumnList->GetCheckedItems(checkedItems);

    int checked = checkedItems.size();
    columns.reserve(checked);
    for (int i = 0; i < checked; ++i) {
        int col = (int)(size_t)(m_ColumnList->GetClientData(checkedItems[i]));
        columns.push_back(col);
    }
}

/*!
 * CwxCSVExportDlg destructor
 */

CwxCSVExportDlg::~CwxCSVExportDlg()
{
////@begin CwxCSVExportDlg destruction
////@end CwxCSVExportDlg destruction
}


/*!
 * Member initialisation
 */

void CwxCSVExportDlg::Init()
{
////@begin CwxCSVExportDlg member initialisation
    m_SelectedOnly = false;
    m_WithHeaders = true;
    m_txtFilename = NULL;
    m_ColumnList = NULL;
////@end CwxCSVExportDlg member initialisation
}


/*!
 * Control creation for CwxCSVExportDlg
 */

void CwxCSVExportDlg::CreateControls()
{    
////@begin CwxCSVExportDlg content construction
    CwxCSVExportDlg* itemCDialog1 = this;

    wxBoxSizer* itemBoxSizer2 = new wxBoxSizer(wxVERTICAL);
    itemCDialog1->SetSizer(itemBoxSizer2);

    wxBoxSizer* itemBoxSizer3 = new wxBoxSizer(wxHORIZONTAL);
    itemBoxSizer2->Add(itemBoxSizer3, 0, wxGROW|wxALL, 5);

    wxStaticText* itemStaticText4 = new wxStaticText( itemCDialog1, wxID_STATIC, _("File Name:"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer3->Add(itemStaticText4, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    m_txtFilename = new wxTextCtrl( itemCDialog1, ID_TEXTCTRL, wxEmptyString, wxDefaultPosition, wxSize(200, -1), 0 );
    itemBoxSizer3->Add(m_txtFilename, 1, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxBitmapButton* itemBitmapButton6 = new wxBitmapButton( itemCDialog1, ID_BITMAPBUTTON, itemCDialog1->GetBitmapResource(wxT("menu::open")), wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW );
    if (CwxCSVExportDlg::ShowToolTips())
        itemBitmapButton6->SetToolTip(_("Select File"));
    itemBoxSizer3->Add(itemBitmapButton6, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxCheckBox* itemCheckBox7 = new wxCheckBox( itemCDialog1, ID_HEADER_CHECKBOX, _("First row to contain headers"), wxDefaultPosition, wxDefaultSize, 0 );
    itemCheckBox7->SetValue(false);
    itemBoxSizer2->Add(itemCheckBox7, 0, wxALIGN_LEFT|wxALL, 5);

    wxCheckBox* itemCheckBox8 = new wxCheckBox( itemCDialog1, ID_SELECTED_CELLS_CHECKBOX, _("Selected cells only"), wxDefaultPosition, wxDefaultSize, 0 );
    itemCheckBox8->SetValue(false);
    itemBoxSizer2->Add(itemCheckBox8, 0, wxALIGN_LEFT|wxALL, 5);

    wxStaticBox* itemStaticBoxSizer9Static = new wxStaticBox(itemCDialog1, wxID_ANY, _("Select columns to export"));
    wxStaticBoxSizer* itemStaticBoxSizer9 = new wxStaticBoxSizer(itemStaticBoxSizer9Static, wxVERTICAL);
    itemBoxSizer2->Add(itemStaticBoxSizer9, 0, wxGROW|wxALL, 5);

    wxArrayString m_ColumnListStrings;
    m_ColumnList = new wxCheckListBox( itemCDialog1, ID_CHECKLISTBOX, wxDefaultPosition, wxDefaultSize, m_ColumnListStrings, wxLB_SINGLE );
    itemStaticBoxSizer9->Add(m_ColumnList, 0, wxGROW|wxALL, 5);

    wxBoxSizer* itemBoxSizer11 = new wxBoxSizer(wxHORIZONTAL);
    itemStaticBoxSizer9->Add(itemBoxSizer11, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

    wxButton* itemButton12 = new wxButton( itemCDialog1, ID_SELECT_ALL_BTN, _("Select All"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer11->Add(itemButton12, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxButton* itemButton13 = new wxButton( itemCDialog1, ID_DESELECT_ALL_BTN, _("Deselect All"), wxDefaultPosition, wxDefaultSize, 0 );
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
    itemCheckBox8->SetValidator( wxGenericValidator(& m_SelectedOnly) );
////@end CwxCSVExportDlg content construction
}


/*!
 * Should we show tooltips?
 */

bool CwxCSVExportDlg::ShowToolTips()
{
    return true;
}

/*!
 * Get bitmap resources
 */

wxBitmap CwxCSVExportDlg::GetBitmapResource( const wxString& name )
{
    return wxArtProvider::GetBitmap(name);
}

/*!
 * Get icon resources
 */

wxIcon CwxCSVExportDlg::GetIconResource( const wxString& name )
{
    // Icon retrieval
////@begin CwxCSVExportDlg icon retrieval
    wxUnusedVar(name);
    return wxNullIcon;
////@end CwxCSVExportDlg icon retrieval
}

/*!
 * wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_BUTTON
 */

void CwxCSVExportDlg::OnSelectFileClick( wxCommandEvent& event )
{
    wxFileDialog fileDlg(this, wxT("Select a file"), wxT(""), wxT(""),
        wxString(wxT("CSV Files (*.csv)|*.csv|")) + wxALL_FILES, wxFD_SAVE|wxFD_OVERWRITE_PROMPT);

    if (fileDlg.ShowModal() == wxID_OK) {
        wxString path = fileDlg.GetPath();
        m_txtFilename->SetValue(path);
    }
}

static const char* kFileName = "FileName";
static const char* kWithHeaders = "WithHeaders";
static const char* kSelectedOnly = "SelectedOnly";

void CwxCSVExportDlg::x_LoadSettings(const CRegistryReadView& view)
{
    m_FileName = FnToWxString(view.GetString(kFileName, FnToStdString(m_FileName)));
    m_WithHeaders = view.GetBool(kWithHeaders, m_WithHeaders);
    m_SelectedOnly = view.GetBool(kSelectedOnly, m_SelectedOnly);
}

void CwxCSVExportDlg::x_SaveSettings(CRegistryWriteView view) const
{
    view.Set(kFileName, FnToStdString(m_FileName));
    view.Set(kWithHeaders, m_WithHeaders);
    view.Set(kSelectedOnly, m_SelectedOnly);
}

/*!
 * wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_SELECT_ALL_BTN
 */

void CwxCSVExportDlg::OnSelectAllBtnClick( wxCommandEvent& event )
{
    for (unsigned i = 0; i < m_ColumnList->GetCount(); ++i) {
        m_ColumnList->Check(i);
    }
}

/*!
 * wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_DESELECT_ALL_BTN
 */

void CwxCSVExportDlg::OnDeselectAllBtnClick( wxCommandEvent& event )
{
    wxArrayInt checkedItems;
    m_ColumnList->GetCheckedItems(checkedItems);

    int checked = checkedItems.size();
    for (int i = 0; i < checked; ++i) {
        m_ColumnList->Check(checkedItems[i], false);
    }
}

/*!
 * wxEVT_COMMAND_CHECKBOX_CLICKED event handler for ID_SELECTED_CELLS_CHECKBOX
 */

void CwxCSVExportDlg::OnSelectedCellsCheckboxClick( wxCommandEvent& event )
{
    TransferDataFromWindow();
    if (m_SelectedOnly)
        m_ColumnList->Enable(false);
    else
        m_ColumnList->Enable(true);
}

END_NCBI_SCOPE
