/*  $Id: filter_columns_dlg.cpp 36594 2016-10-12 20:17:36Z evgeniev $
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
 * Authors:   Vladislav Evgeniev
 */


#include <ncbi_pch.hpp>

////@begin includes
////@end includes

#include <wx/checklst.h>
#include <wx/button.h>
#include <wx/textctrl.h>
#include <wx/stattext.h>
#include <wx/statbox.h>
#include <wx/sizer.h>

#include <corelib/ncbistr.hpp>
#include <gui/widgets/grid_widget/filter_columns_dlg.hpp>

#include <util/xregexp/regexp.hpp>

////@begin XPM images
////@end XPM images

BEGIN_NCBI_SCOPE

/*!
 * CFilterColumnsDlg type definition
 */

IMPLEMENT_DYNAMIC_CLASS( CFilterColumnsDlg, wxDialog )


/*!
 * CFilterColumnsDlg event table definition
 */

BEGIN_EVENT_TABLE( CFilterColumnsDlg, wxDialog )

////@begin CFilterColumnsDlg event table entries
    EVT_BUTTON( ID_SELECT_ALL_BTN, CFilterColumnsDlg::OnSelectAllBtnClick )
    EVT_BUTTON( ID_DESELECT_BTN, CFilterColumnsDlg::OnDeselectBtnClick )
    EVT_TEXT( ID_RANGE_TEXTCTRL, CFilterColumnsDlg::OnRangeTextctrlTextUpdated )
    EVT_BUTTON( ID_SELECT_BTN, CFilterColumnsDlg::OnSelectBtnClick )
////@end CFilterColumnsDlg event table entries

END_EVENT_TABLE()


/*!
 * CFilterColumnsDlg constructors
 */

 CFilterColumnsDlg::CFilterColumnsDlg()
{
    Init();
}

CFilterColumnsDlg::CFilterColumnsDlg(wxWindow* parent, wxWindowID id, const wxString& caption, const wxPoint& pos, const wxSize& size, long style)
{
    Init();
    Create(parent, id, caption, pos, size, style);
}

void CFilterColumnsDlg::SetGridAdapter(IGridTableAdapter* gridAdapter)
{
    _ASSERT(gridAdapter);
    _ASSERT(m_ColumnsLstBox);
    m_GridAdapter = gridAdapter;

    m_ColumnsLstBox->Clear();

    vector<size_t> vis_columns = m_GridAdapter->GetVisibleColumns();
    set<size_t> vis_set;
    vis_set.insert(vis_columns.begin(), vis_columns.end());

    for (size_t col = 0; col < m_GridAdapter->GetTable().GetColsCount(); col++) {
        string label = NStr::NumericToString(col + 1);
        label += ". ";
        label += m_GridAdapter->GetTable().GetColumnLabel(col);
        m_ColumnsLstBox->AppendString(label);
        m_ColumnsLstBox->Check(col, vis_set.find(col) != vis_set.end());
    }
}

/*!
 * CFilterColumnsDlg creator
 */

bool CFilterColumnsDlg::Create( wxWindow* parent, wxWindowID id, const wxString& caption, const wxPoint& pos, const wxSize& size, long style )
{
////@begin CFilterColumnsDlg creation
    SetExtraStyle(wxWS_EX_VALIDATE_RECURSIVELY|wxWS_EX_BLOCK_EVENTS);
    wxDialog::Create( parent, id, caption, pos, size, style );

    CreateControls();
    if (GetSizer())
    {
        GetSizer()->SetSizeHints(this);
    }
    Centre();
////@end CFilterColumnsDlg creation
    return true;
}


/*!
 * CFilterColumnsDlg destructor
 */

CFilterColumnsDlg::~CFilterColumnsDlg()
{
////@begin CFilterColumnsDlg destruction
////@end CFilterColumnsDlg destruction
}


/*!
 * Member initialisation
 */

void CFilterColumnsDlg::Init()
{
////@begin CFilterColumnsDlg member initialisation
    m_ColumnsLstBox = NULL;
    m_RangeText = NULL;
    m_SelectBtn = NULL;
////@end CFilterColumnsDlg member initialisation
}


/*!
 * Control creation for CFilterColumnsDlg
 */

void CFilterColumnsDlg::CreateControls()
{    
////@begin CFilterColumnsDlg content construction
    CFilterColumnsDlg* itemDialog1 = this;

    wxBoxSizer* itemBoxSizer2 = new wxBoxSizer(wxVERTICAL);
    itemDialog1->SetSizer(itemBoxSizer2);

    wxStaticBox* itemStaticBoxSizer3Static = new wxStaticBox(itemDialog1, wxID_ANY, _("Columns"));
    wxStaticBoxSizer* itemStaticBoxSizer3 = new wxStaticBoxSizer(itemStaticBoxSizer3Static, wxVERTICAL);
    itemBoxSizer2->Add(itemStaticBoxSizer3, 0, wxGROW|wxALL, 5);

    wxArrayString m_ColumnsLstBoxStrings;
    m_ColumnsLstBox = new wxCheckListBox( itemDialog1, ID_COLUMNS_CHECKLISTBOX, wxDefaultPosition, wxSize(250, 250), m_ColumnsLstBoxStrings, wxLB_SINGLE );
    itemStaticBoxSizer3->Add(m_ColumnsLstBox, 0, wxGROW|wxALL, 5);

    wxStaticBox* itemStaticBoxSizer5Static = new wxStaticBox(itemDialog1, wxID_ANY, _("Select/Deselect All"));
    wxStaticBoxSizer* itemStaticBoxSizer5 = new wxStaticBoxSizer(itemStaticBoxSizer5Static, wxVERTICAL);
    itemStaticBoxSizer3->Add(itemStaticBoxSizer5, 0, wxGROW|wxALL, 5);

    wxBoxSizer* itemBoxSizer6 = new wxBoxSizer(wxHORIZONTAL);
    itemStaticBoxSizer5->Add(itemBoxSizer6, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

    wxButton* itemButton7 = new wxButton( itemDialog1, ID_SELECT_ALL_BTN, _("Select All"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer6->Add(itemButton7, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxButton* itemButton8 = new wxButton( itemDialog1, ID_DESELECT_BTN, _("Deselect All"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer6->Add(itemButton8, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxStaticBox* itemStaticBoxSizer9Static = new wxStaticBox(itemDialog1, wxID_ANY, _("Select Range"));
    wxStaticBoxSizer* itemStaticBoxSizer9 = new wxStaticBoxSizer(itemStaticBoxSizer9Static, wxVERTICAL);
    itemStaticBoxSizer3->Add(itemStaticBoxSizer9, 0, wxGROW|wxALL, 5);

    wxStaticText* itemStaticText10 = new wxStaticText( itemDialog1, wxID_STATIC, _("Enter selection range (e.g. 1,3-5):"), wxDefaultPosition, wxDefaultSize, 0 );
    itemStaticBoxSizer9->Add(itemStaticText10, 0, wxALIGN_LEFT|wxALL, 5);

    m_RangeText = new wxTextCtrl( itemDialog1, ID_RANGE_TEXTCTRL, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
    itemStaticBoxSizer9->Add(m_RangeText, 0, wxGROW|wxALL, 5);

    wxBoxSizer* itemBoxSizer12 = new wxBoxSizer(wxHORIZONTAL);
    itemBoxSizer2->Add(itemBoxSizer12, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

    m_SelectBtn = new wxButton( itemDialog1, ID_SELECT_BTN, _("Select"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer12->Add(m_SelectBtn, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxButton* itemButton14 = new wxButton( itemDialog1, wxID_CANCEL, _("Cancel"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer12->Add(itemButton14, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

////@end CFilterColumnsDlg content construction
}


/*!
 * Should we show tooltips?
 */

bool CFilterColumnsDlg::ShowToolTips()
{
    return true;
}

/*!
 * Get bitmap resources
 */

wxBitmap CFilterColumnsDlg::GetBitmapResource( const wxString& name )
{
    // Bitmap retrieval
////@begin CFilterColumnsDlg bitmap retrieval
    wxUnusedVar(name);
    return wxNullBitmap;
////@end CFilterColumnsDlg bitmap retrieval
}

/*!
 * Get icon resources
 */

wxIcon CFilterColumnsDlg::GetIconResource( const wxString& name )
{
    // Icon retrieval
////@begin CFilterColumnsDlg icon retrieval
    wxUnusedVar(name);
    return wxNullIcon;
////@end CFilterColumnsDlg icon retrieval
}


/*!
 * wxEVT_COMMAND_TEXT_UPDATED event handler for ID_TEXTCTRL
 */

void CFilterColumnsDlg::OnRangeTextctrlTextUpdated( wxCommandEvent& event )
{
    CRegexp regex("^\\d+(?:-\\d+)?(?:,\\d+(?:-\\d+)?)*$");
    if (regex.IsMatch(m_RangeText->GetLineText(0).ToStdString())) {
        x_ProcessSelectionPattern();
    }
}


/*!
 * wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_SELECT_BTN
 */

void CFilterColumnsDlg::OnSelectBtnClick( wxCommandEvent& event )
{
    _ASSERT(m_GridAdapter);
    wxArrayInt checkedItems;
    m_ColumnsLstBox->GetCheckedItems(checkedItems);
    set<size_t> vis_set;
    for (size_t i = 0; i < checkedItems.Count(); ++i) {
        vis_set.insert(checkedItems[i]);
    }

    for (size_t col = 0; col < m_GridAdapter->GetTable().GetColsCount(); col++) {
        bool shown = (vis_set.end() != vis_set.find(col));
        m_GridAdapter->ShowColumn(col, shown);
    }

    EndModal(wxID_OK);
}

void CFilterColumnsDlg::x_ProcessSelectionPattern()
{
    set<size_t> vis_set;
    vector< string > tokens;
    NStr::Split(m_RangeText->GetLineText(0).ToStdString(), ",", tokens);
    ITERATE(vector< string >, itToken, tokens) {
        vector< string > range;
        NStr::Split(*itToken, "-", range);
        if (1 == range.size()) {
            vis_set.insert(NStr::StringToUInt(range[0]) - 1);
        }
        else if (2 == range.size()) {
            size_t first = NStr::StringToUInt(range[0]);
            size_t last = NStr::StringToUInt(range[1]);
            for (size_t i = first; i <= last; ++i) {
                vis_set.insert(i - 1);
            }
        }
    }
    for (size_t i = 0; i < m_ColumnsLstBox->GetCount(); ++i) {
        m_ColumnsLstBox->Check(i, vis_set.find(i) != vis_set.end());
    }
}


/*!
 * wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_SELECT_ALL_BTN
 */

void CFilterColumnsDlg::OnSelectAllBtnClick( wxCommandEvent& event )
{
    for (size_t i = 0; i < m_ColumnsLstBox->GetCount(); ++i) {
        m_ColumnsLstBox->Check(i, true);
    }
}


/*!
 * wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_DESELECT_BTN
 */

void CFilterColumnsDlg::OnDeselectBtnClick( wxCommandEvent& event )
{
    for (size_t i = 0; i < m_ColumnsLstBox->GetCount(); ++i) {
        m_ColumnsLstBox->Check(i, false);
    }
}


END_NCBI_SCOPE
