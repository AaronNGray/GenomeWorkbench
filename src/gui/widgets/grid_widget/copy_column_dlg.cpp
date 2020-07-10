/*  $Id: copy_column_dlg.cpp 39666 2017-10-25 16:01:13Z katargir $
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
 * Authors:  
 */


#include <ncbi_pch.hpp>

////@begin includes
////@end includes

#include "copy_column_dlg.hpp"
#include <gui/objutils/table_data.hpp>
#include <gui/widgets/data/cmd_table_copy_column.hpp>
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
 * CCopyColumnDlg type definition
 */

IMPLEMENT_DYNAMIC_CLASS( CCopyColumnDlg, CDialog )


/*!
 * CCopyColumnDlg event table definition
 */

BEGIN_EVENT_TABLE( CCopyColumnDlg, CDialog )

////@begin CCopyColumnDlg event table entries
////@end CCopyColumnDlg event table entries

END_EVENT_TABLE()


/*!
 * CCopyColumnDlg constructors
 */

CCopyColumnDlg::CCopyColumnDlg()
: m_Table()
{
    Init();
}

CCopyColumnDlg::CCopyColumnDlg( wxWindow* parent, ITableData& table )
: m_Table(&table)
{
    Init();
    Create(parent,
        SYMBOL_CCOPYCOLUMNDLG_IDNAME,
        SYMBOL_CCOPYCOLUMNDLG_TITLE,
        SYMBOL_CCOPYCOLUMNDLG_POSITION,
        SYMBOL_CCOPYCOLUMNDLG_SIZE,
        SYMBOL_CCOPYCOLUMNDLG_STYLE);
}


/*!
 * CCopyColumnDlg creator
 */

bool CCopyColumnDlg::Create( wxWindow* parent, wxWindowID id, const wxString& caption, const wxPoint& pos, const wxSize& size, long style )
{
////@begin CCopyColumnDlg creation
    SetExtraStyle(wxWS_EX_BLOCK_EVENTS);
    CDialog::Create( parent, id, caption, pos, size, style );

    CreateControls();
    if (GetSizer())
    {
        GetSizer()->SetSizeHints(this);
    }
    Centre();
////@end CCopyColumnDlg creation
    return true;
}


/*!
 * CCopyColumnDlg destructor
 */

CCopyColumnDlg::~CCopyColumnDlg()
{
////@begin CCopyColumnDlg destruction
////@end CCopyColumnDlg destruction
}


/*!
 * Member initialisation
 */

void CCopyColumnDlg::Init()
{
    if (m_Table) {
        m_RegPath = "Dialogs.CCopyColumnDlg." + m_Table->GetTableTypeId();
    }

////@begin CCopyColumnDlg member initialisation
    m_Op = 0;
    m_ColumnFrom = -1;
    m_ColumnTo = -1;
    m_ColumnFromCtrl = NULL;
    m_ColumnToCtrl = NULL;
////@end CCopyColumnDlg member initialisation
}


/*!
 * Control creation for CCopyColumnDlg
 */

void CCopyColumnDlg::CreateControls()
{    
////@begin CCopyColumnDlg content construction
    CCopyColumnDlg* itemCDialog1 = this;

    wxBoxSizer* itemBoxSizer2 = new wxBoxSizer(wxVERTICAL);
    itemCDialog1->SetSizer(itemBoxSizer2);

    wxFlexGridSizer* itemFlexGridSizer3 = new wxFlexGridSizer(0, 2, 0, 0);
    itemBoxSizer2->Add(itemFlexGridSizer3, 0, wxGROW|wxALL, 5);

    wxStaticText* itemStaticText4 = new wxStaticText( itemCDialog1, wxID_STATIC, _("From Column:"), wxDefaultPosition, wxDefaultSize, 0 );
    itemFlexGridSizer3->Add(itemStaticText4, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxArrayString m_ColumnFromCtrlStrings;
    m_ColumnFromCtrl = new wxComboBox( itemCDialog1, ID_COMBOBOX2, wxEmptyString, wxDefaultPosition, wxDefaultSize, m_ColumnFromCtrlStrings, wxCB_READONLY );
    itemFlexGridSizer3->Add(m_ColumnFromCtrl, 0, wxGROW|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxStaticText* itemStaticText6 = new wxStaticText( itemCDialog1, wxID_STATIC, _("To Column:"), wxDefaultPosition, wxDefaultSize, 0 );
    itemFlexGridSizer3->Add(itemStaticText6, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxArrayString m_ColumnToCtrlStrings;
    m_ColumnToCtrl = new wxComboBox( itemCDialog1, ID_COMBOBOX3, wxEmptyString, wxDefaultPosition, wxDefaultSize, m_ColumnToCtrlStrings, wxCB_READONLY );
    itemFlexGridSizer3->Add(m_ColumnToCtrl, 0, wxGROW|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    itemFlexGridSizer3->AddGrowableCol(1);

    wxArrayString itemRadioBox8Strings;
    itemRadioBox8Strings.Add(_("&Copy"));
    itemRadioBox8Strings.Add(_("&Append"));
    itemRadioBox8Strings.Add(_("&Prepend"));
    wxRadioBox* itemRadioBox8 = new wxRadioBox( itemCDialog1, ID_RADIOBOX, _("What to Do"), wxDefaultPosition, wxDefaultSize, itemRadioBox8Strings, 1, wxRA_SPECIFY_ROWS );
    itemRadioBox8->SetSelection(0);
    itemBoxSizer2->Add(itemRadioBox8, 0, wxALIGN_LEFT|wxALL, 5);

    itemBoxSizer2->Add(5, 5, 1, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

    wxStdDialogButtonSizer* itemStdDialogButtonSizer10 = new wxStdDialogButtonSizer;

    itemBoxSizer2->Add(itemStdDialogButtonSizer10, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);
    wxButton* itemButton11 = new wxButton( itemCDialog1, wxID_OK, _("&OK"), wxDefaultPosition, wxDefaultSize, 0 );
    itemStdDialogButtonSizer10->AddButton(itemButton11);

    wxButton* itemButton12 = new wxButton( itemCDialog1, wxID_CANCEL, _("&Cancel"), wxDefaultPosition, wxDefaultSize, 0 );
    itemStdDialogButtonSizer10->AddButton(itemButton12);

    itemStdDialogButtonSizer10->Realize();

    // Set validators
    itemRadioBox8->SetValidator( wxGenericValidator(& m_Op) );
////@end CCopyColumnDlg content construction

    if (m_Table) {
        size_t cols = m_Table->GetColsCount();
        for (size_t i = 0 ; i < cols; ++i) {
            string label = m_Table->GetColumnLabel(i);
            if (m_Table->GetColumnType(i) == ITableData::kString && m_Table->AllowEdit(i)) {
                m_ColumnToCtrl->Append(wxString::FromUTF8(label.c_str()), (void*)i);
            }
            m_ColumnFromCtrl->Append(wxString::FromUTF8(label.c_str()), (void*)i);
        }
        if (m_ColumnToCtrl->GetCount() > 0) {
            m_ColumnToCtrl->Select(0);
        }
        if (m_ColumnFromCtrl->GetCount() > 0) {
            m_ColumnFromCtrl->Select(0);
        }
    }
}

bool CCopyColumnDlg::TransferDataFromWindow()
{
    if (!CDialog::TransferDataFromWindow())
        return false;

    int sel = m_ColumnFromCtrl->GetSelection();
    if (sel != wxNOT_FOUND) {
        m_ColumnFrom = (int)(intptr_t)m_ColumnFromCtrl->GetClientData(sel);
    }
    else
        m_ColumnFrom = -1;

    sel = m_ColumnToCtrl->GetSelection();
    if (sel != wxNOT_FOUND) {
        m_ColumnTo = (int)(intptr_t)m_ColumnToCtrl->GetClientData(sel);
    }
    else
        m_ColumnTo = -1;

    return true;
}

bool CCopyColumnDlg::CanCopyColumn(ITableData& table)
{
    size_t cols = table.GetColsCount();
    for (size_t i = 0 ; i < cols; ++i) {
        if (table.GetColumnType(i) == ITableData::kString && table.AllowEdit(i)) {
            return true;
        }
    }
    return false;
}

IEditCommand* CCopyColumnDlg::GetEditCommand()
{
    if (m_ColumnFrom < 0 || m_ColumnTo < 0)
        return 0;

    CCmdTableCopyColumn::EOp op = CCmdTableCopyColumn::eCopy;
    if (m_Op == 1)
        op = CCmdTableCopyColumn::eAppend;
    else if (m_Op == 2)
        op = CCmdTableCopyColumn::ePrepend;

    return new CCmdTableCopyColumn(*m_Table, m_ColumnFrom, m_ColumnTo, op);
}


/*!
 * Should we show tooltips?
 */

bool CCopyColumnDlg::ShowToolTips()
{
    return true;
}

/*!
 * Get bitmap resources
 */

wxBitmap CCopyColumnDlg::GetBitmapResource( const wxString& name )
{
    // Bitmap retrieval
////@begin CCopyColumnDlg bitmap retrieval
    wxUnusedVar(name);
    return wxNullBitmap;
////@end CCopyColumnDlg bitmap retrieval
}

/*!
 * Get icon resources
 */

wxIcon CCopyColumnDlg::GetIconResource( const wxString& name )
{
    // Icon retrieval
////@begin CCopyColumnDlg icon retrieval
    wxUnusedVar(name);
    return wxNullIcon;
////@end CCopyColumnDlg icon retrieval
}

static const char* kColumnFrom = "ColumnFrom";
static const char* kColumnTo = "ColumnTo";
static const char* kOp  = "Op";


void CCopyColumnDlg::LoadSettings()
{
    CDialog::LoadSettings();

    if (m_Table && !m_RegPath.empty()) {
        CRegistryReadView view = CGuiRegistry::GetInstance().GetReadView(m_RegPath);

        m_ColumnFromCtrl->SetValue(ToWxString(view.GetString(kColumnFrom)));
        m_ColumnToCtrl->SetValue(ToWxString(view.GetString(kColumnTo)));

        m_Op = view.GetInt(kOp, m_Op);
        if (m_Op < 0 || m_Op > 2)
            m_Op = 0;
    }
}

void CCopyColumnDlg::SaveSettings() const
{
    CDialog::SaveSettings();

    if (m_Table && !m_RegPath.empty()) {
        CRegistryWriteView view = CGuiRegistry::GetInstance().GetWriteView(m_RegPath);
        view.Set(kColumnFrom, ToStdString(m_ColumnFromCtrl->GetValue()));
        view.Set(kColumnTo, ToStdString(m_ColumnToCtrl->GetValue()));
        view.Set(kOp, m_Op);
    }
}

END_NCBI_SCOPE
