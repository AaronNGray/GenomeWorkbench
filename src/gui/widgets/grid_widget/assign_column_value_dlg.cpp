/*  $Id: assign_column_value_dlg.cpp 39666 2017-10-25 16:01:13Z katargir $
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
 * Authors:  Roman Katargin
 */


#include <ncbi_pch.hpp>

////@begin includes
////@end includes

#include "assign_column_value_dlg.hpp"
#include <gui/objutils/table_data.hpp>
#include <gui/widgets/data/cmd_table_assign_column.hpp>
#include <gui/widgets/wx/wx_utils.hpp>

#include <wx/sizer.h>
#include <wx/stattext.h>
#include <wx/combobox.h>
#include <wx/button.h>
#include <wx/radiobox.h>


////@begin XPM images
////@end XPM images

BEGIN_NCBI_SCOPE

/*!
 * CAssignColumnValueDlg type definition
 */

IMPLEMENT_DYNAMIC_CLASS( CAssignColumnValueDlg, CDialog )


/*!
 * CAssignColumnValueDlg event table definition
 */

BEGIN_EVENT_TABLE( CAssignColumnValueDlg, CDialog )

////@begin CAssignColumnValueDlg event table entries
    EVT_COMBOBOX( ID_COMBOBOX, CAssignColumnValueDlg::OnComboboxSelected )

////@end CAssignColumnValueDlg event table entries

END_EVENT_TABLE()


/*!
 * CAssignColumnValueDlg constructors
 */

CAssignColumnValueDlg::CAssignColumnValueDlg()
: m_Table()
{
    Init();
}

CAssignColumnValueDlg::CAssignColumnValueDlg( wxWindow* parent, ITableData& table )
: m_Table(&table)
{
    Init();
    Create(parent,
        SYMBOL_CASSIGNCOLUMNVALUEDLG_IDNAME,
        SYMBOL_CASSIGNCOLUMNVALUEDLG_TITLE,
        SYMBOL_CASSIGNCOLUMNVALUEDLG_POSITION,
        SYMBOL_CASSIGNCOLUMNVALUEDLG_SIZE,
        SYMBOL_CASSIGNCOLUMNVALUEDLG_STYLE);
}


/*!
 * CAssignColumnValueDlg creator
 */

bool CAssignColumnValueDlg::Create( wxWindow* parent, wxWindowID id, const wxString& caption, const wxPoint& pos, const wxSize& size, long style )
{
////@begin CAssignColumnValueDlg creation
    SetExtraStyle(wxWS_EX_BLOCK_EVENTS);
    CDialog::Create( parent, id, caption, pos, size, style );

    CreateControls();
    if (GetSizer())
    {
        GetSizer()->SetSizeHints(this);
    }
    Centre();
////@end CAssignColumnValueDlg creation
    return true;
}


/*!
 * CAssignColumnValueDlg destructor
 */

CAssignColumnValueDlg::~CAssignColumnValueDlg()
{
////@begin CAssignColumnValueDlg destruction
////@end CAssignColumnValueDlg destruction
}


/*!
 * Member initialisation
 */

void CAssignColumnValueDlg::Init()
{
    if (m_Table) {
        m_RegPath = "Dialogs.CAssignColumnValueDlg." + m_Table->GetTableTypeId();
    }

////@begin CAssignColumnValueDlg member initialisation
    m_Column = -1;
    m_Op = 0;
    m_Columns = NULL;
    m_Values = NULL;
////@end CAssignColumnValueDlg member initialisation
}


/*!
 * Control creation for CAssignColumnValueDlg
 */

void CAssignColumnValueDlg::CreateControls()
{    
////@begin CAssignColumnValueDlg content construction
    CAssignColumnValueDlg* itemCDialog1 = this;

    wxBoxSizer* itemBoxSizer2 = new wxBoxSizer(wxVERTICAL);
    itemCDialog1->SetSizer(itemBoxSizer2);

    wxBoxSizer* itemBoxSizer3 = new wxBoxSizer(wxHORIZONTAL);
    itemBoxSizer2->Add(itemBoxSizer3, 0, wxGROW|wxALL, 5);

    wxStaticText* itemStaticText4 = new wxStaticText( itemCDialog1, wxID_STATIC, _("Select Column:"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer3->Add(itemStaticText4, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxArrayString m_ColumnsStrings;
    m_Columns = new wxComboBox( itemCDialog1, ID_COMBOBOX, wxEmptyString, wxDefaultPosition, wxDefaultSize, m_ColumnsStrings, wxCB_READONLY );
    itemBoxSizer3->Add(m_Columns, 1, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxBoxSizer* itemBoxSizer6 = new wxBoxSizer(wxHORIZONTAL);
    itemBoxSizer2->Add(itemBoxSizer6, 0, wxGROW|wxALL, 5);

    wxStaticText* itemStaticText7 = new wxStaticText( itemCDialog1, wxID_STATIC, _("Select (enter) value:"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer6->Add(itemStaticText7, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxArrayString m_ValuesStrings;
    m_Values = new wxComboBox( itemCDialog1, ID_COMBOBOX1, wxEmptyString, wxDefaultPosition, wxDefaultSize, m_ValuesStrings, wxCB_DROPDOWN );
    itemBoxSizer6->Add(m_Values, 1, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxArrayString itemRadioBox9Strings;
    itemRadioBox9Strings.Add(_("&Assign"));
    itemRadioBox9Strings.Add(_("&Append"));
    itemRadioBox9Strings.Add(_("&Prepend"));
    wxRadioBox* itemRadioBox9 = new wxRadioBox( itemCDialog1, ID_RADIOBOX1, _("What to Do"), wxDefaultPosition, wxDefaultSize, itemRadioBox9Strings, 1, wxRA_SPECIFY_ROWS );
    itemRadioBox9->SetSelection(0);
    itemBoxSizer2->Add(itemRadioBox9, 0, wxALIGN_LEFT|wxALL, 5);

    itemBoxSizer2->Add(5, 5, 1, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

    wxStdDialogButtonSizer* itemStdDialogButtonSizer11 = new wxStdDialogButtonSizer;

    itemBoxSizer2->Add(itemStdDialogButtonSizer11, 0, wxGROW|wxALL, 5);
    wxButton* itemButton12 = new wxButton( itemCDialog1, wxID_OK, _("&OK"), wxDefaultPosition, wxDefaultSize, 0 );
    itemStdDialogButtonSizer11->AddButton(itemButton12);

    wxButton* itemButton13 = new wxButton( itemCDialog1, wxID_CANCEL, _("&Cancel"), wxDefaultPosition, wxDefaultSize, 0 );
    itemStdDialogButtonSizer11->AddButton(itemButton13);

    itemStdDialogButtonSizer11->Realize();

    // Set validators
    m_Values->SetValidator( wxTextValidator(wxFILTER_NONE, & m_Value) );
    itemRadioBox9->SetValidator( wxGenericValidator(& m_Op) );
////@end CAssignColumnValueDlg content construction

    if (m_Table) {
        size_t cols = m_Table->GetColsCount();
        for (size_t i = 0 ; i < cols; ++i) {
            if (m_Table->GetColumnType(i) == ITableData::kString && m_Table->AllowEdit(i)) {
                string label = m_Table->GetColumnLabel(i);
                m_Columns->Append(wxString::FromUTF8(label.c_str()), (void*)i);
            }
        }
        if (m_Columns->GetCount() > 0) {
            m_Columns->Select(0);
            m_Column = (int)(intptr_t)m_Columns->GetClientData(0);
            x_UpdateValues();
        }
    }
}

bool CAssignColumnValueDlg::TransferDataFromWindow()
{
    if (!CDialog::TransferDataFromWindow())
        return false;

    int sel = m_Columns->GetSelection();
    if (sel != wxNOT_FOUND) {
        m_Column = (int)(intptr_t)m_Columns->GetClientData(sel);
    }
    else
        m_Column = -1;

    return true;
}

void CAssignColumnValueDlg::x_UpdateValues()
{
    if (!m_Table)
        return;

    m_Value = m_Values->GetValue();
    m_Values->Clear();
    int sel = m_Columns->GetSelection();
    if (sel != wxNOT_FOUND) {
        m_Column = (int)(intptr_t)m_Columns->GetClientData(sel);
        size_t rows = m_Table->GetRowsCount();
        set<string> values;
        string s;
        for (size_t i = 0; i < rows; ++i) {
            m_Table->GetStringValue(i, (size_t)m_Column, s);
            values.insert(s);
            if (values.size() >= 10)
                break;
        }
        ITERATE(set<string>, it, values) {
            m_Values->Append(*it);
        }
    }
    else {
        m_Column = -1;
    }
    m_Values->SetValue(m_Value);
}

bool CAssignColumnValueDlg::CanAssignColumn(ITableData& table)
{
    size_t cols = table.GetColsCount();
    for (size_t i = 0 ; i < cols; ++i) {
        if (table.GetColumnType(i) == ITableData::kString && table.AllowEdit(i)) {
            return true;
        }
    }
    return false;
}


/*!
 * Should we show tooltips?
 */

bool CAssignColumnValueDlg::ShowToolTips()
{
    return true;
}

/*!
 * Get bitmap resources
 */

wxBitmap CAssignColumnValueDlg::GetBitmapResource( const wxString& name )
{
    // Bitmap retrieval
////@begin CAssignColumnValueDlg bitmap retrieval
    wxUnusedVar(name);
    return wxNullBitmap;
////@end CAssignColumnValueDlg bitmap retrieval
}

/*!
 * Get icon resources
 */

wxIcon CAssignColumnValueDlg::GetIconResource( const wxString& name )
{
    // Icon retrieval
////@begin CAssignColumnValueDlg icon retrieval
    wxUnusedVar(name);
    return wxNullIcon;
////@end CAssignColumnValueDlg icon retrieval
}

/*!
 * wxEVT_COMMAND_COMBOBOX_SELECTED event handler for ID_COMBOBOX
 */

void CAssignColumnValueDlg::OnComboboxSelected( wxCommandEvent& event )
{
    x_UpdateValues();
}

IEditCommand* CAssignColumnValueDlg::GetEditCommand()
{
    if (m_Column < 0)
        return 0;

    CCmdTableAssignColumn::EOp op = CCmdTableAssignColumn::eAssign;
    if (m_Op == 1)
        op = CCmdTableAssignColumn::eAppend;
    else if (m_Op == 2)
        op = CCmdTableAssignColumn::ePrepend;

    return new CCmdTableAssignColumn(*m_Table, m_Column, m_Value, op);
}

static const char* kColumn = "Column";
static const char* kValue  = "Value";
static const char* kOp  = "Op";


void CAssignColumnValueDlg::LoadSettings()
{
    CDialog::LoadSettings();

    if (m_Table && !m_RegPath.empty()) {
        CRegistryReadView view = CGuiRegistry::GetInstance().GetReadView(m_RegPath);

        m_Value = ToWxString(view.GetString(kValue, ToStdString(m_Value)));
        m_Columns->SetValue(ToWxString(view.GetString(kColumn)));
        m_Op = view.GetInt(kOp, m_Op);
        if (m_Op < 0 || m_Op > 2)
            m_Op = 0;
    }
}

void CAssignColumnValueDlg::SaveSettings() const
{
    CDialog::SaveSettings();

    if (m_Table && !m_RegPath.empty()) {
        CRegistryWriteView view = CGuiRegistry::GetInstance().GetWriteView(m_RegPath);
        view.Set(kValue, ToStdString(m_Value));
        view.Set(kColumn, ToStdString(m_Columns->GetValue()));
        view.Set(kOp, m_Op);
    }
}

END_NCBI_SCOPE
