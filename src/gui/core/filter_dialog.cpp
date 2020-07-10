/*  $Id: filter_dialog.cpp 25794 2012-05-10 14:53:54Z katargir $
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

////@begin includes
////@end includes


#include <wx/dialog.h>
#include <wx/textctrl.h>
#include <wx/sizer.h>
#include <wx/checkbox.h>
#include <wx/stattext.h>
#include <wx/statbox.h>
#include <wx/bitmap.h>
#include <wx/icon.h>
#include <wx/checklst.h>
#include <wx/button.h>

#include <gui/core/filter_dialog.hpp>
#include <gui/widgets/wx/wx_utils.hpp>


////@begin XPM images
////@end XPM images

BEGIN_NCBI_SCOPE

IMPLEMENT_DYNAMIC_CLASS( CFilterDialog, wxDialog )

BEGIN_EVENT_TABLE( CFilterDialog, wxDialog )

////@begin CFilterDialog event table entries
    EVT_LISTBOX( ID_CHECKLISTBOX1, CFilterDialog::OnChecklistbox1Selected )

    EVT_BUTTON( ID_BUTTON2, CFilterDialog::OnClearClicked )

////@end CFilterDialog event table entries

END_EVENT_TABLE()

CFilterDialog::CFilterDialog()
{
    Init();
}

CFilterDialog::CFilterDialog( wxWindow* parent, wxWindowID id, const wxString& caption, const wxPoint& pos, const wxSize& size, long style )
{
    Init();
    Create(parent, id, caption, pos, size, style);
}

bool CFilterDialog::Create( wxWindow* parent, wxWindowID id, const wxString& caption, const wxPoint& pos, const wxSize& size, long style )
{
////@begin CFilterDialog creation
    SetExtraStyle(wxWS_EX_BLOCK_EVENTS);
    wxDialog::Create( parent, id, caption, pos, size, style );

    CreateControls();
    if (GetSizer())
    {
        GetSizer()->SetSizeHints(this);
    }
    Centre();
////@end CFilterDialog creation
    return true;
}

CFilterDialog::~CFilterDialog()
{
////@begin CFilterDialog destruction
////@end CFilterDialog destruction
}

void CFilterDialog::Init()
{
////@begin CFilterDialog member initialisation
    m_ListBox = NULL;
    m_Name = NULL;
    m_Expression = NULL;
////@end CFilterDialog member initialisation
}

void CFilterDialog::CreateControls()
{    
////@begin CFilterDialog content construction
    CFilterDialog* itemDialog1 = this;

    wxBoxSizer* itemBoxSizer2 = new wxBoxSizer(wxVERTICAL);
    itemDialog1->SetSizer(itemBoxSizer2);

    wxArrayString m_ListBoxStrings;
    m_ListBox = new wxCheckListBox( itemDialog1, ID_CHECKLISTBOX1, wxDefaultPosition, wxDefaultSize, m_ListBoxStrings, wxLB_SINGLE );
    itemBoxSizer2->Add(m_ListBox, 0, wxGROW|wxALL, 5);

    wxFlexGridSizer* itemFlexGridSizer4 = new wxFlexGridSizer(0, 2, 0, 0);
    itemBoxSizer2->Add(itemFlexGridSizer4, 0, wxGROW|wxALL, 5);

    wxStaticText* itemStaticText5 = new wxStaticText( itemDialog1, wxID_STATIC, _("Filter Name"), wxDefaultPosition, wxDefaultSize, 0 );
    itemFlexGridSizer4->Add(itemStaticText5, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    m_Name = new wxTextCtrl( itemDialog1, ID_TEXTCTRL4, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
    m_Name->Enable(false);
    itemFlexGridSizer4->Add(m_Name, 0, wxGROW|wxALIGN_TOP|wxALL, 5);

    wxStaticText* itemStaticText7 = new wxStaticText( itemDialog1, wxID_STATIC, _("Expression"), wxDefaultPosition, wxDefaultSize, 0 );
    itemFlexGridSizer4->Add(itemStaticText7, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    m_Expression = new wxTextCtrl( itemDialog1, ID_TEXTCTRL5, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_MULTILINE );
    m_Expression->Enable(false);
    itemFlexGridSizer4->Add(m_Expression, 0, wxGROW|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    itemFlexGridSizer4->AddGrowableCol(1);

    wxBoxSizer* itemBoxSizer9 = new wxBoxSizer(wxHORIZONTAL);
    itemBoxSizer2->Add(itemBoxSizer9, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

    wxButton* itemButton10 = new wxButton( itemDialog1, ID_BUTTON2, _("Clear"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer9->Add(itemButton10, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxButton* itemButton11 = new wxButton( itemDialog1, wxID_CANCEL, _("Cancel"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer9->Add(itemButton11, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxButton* itemButton12 = new wxButton( itemDialog1, wxID_OK, _("OK"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer9->Add(itemButton12, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

////@end CFilterDialog content construction
}

bool CFilterDialog::ShowToolTips()
{
    return true;
}
wxBitmap CFilterDialog::GetBitmapResource( const wxString& name )
{
    // Bitmap retrieval
////@begin CFilterDialog bitmap retrieval
    wxUnusedVar(name);
    return wxNullBitmap;
////@end CFilterDialog bitmap retrieval
}
wxIcon CFilterDialog::GetIconResource( const wxString& name )
{
    // Icon retrieval
////@begin CFilterDialog icon retrieval
    wxUnusedVar(name);
    return wxNullIcon;
////@end CFilterDialog icon retrieval
}

bool CFilterDialog::TransferDataToWindow()
{   
    ITERATE (vector<IDMSearchTool::TFilter>, flt, m_Filters.filters) {
        int item = m_ListBox->Append(ToWxString(flt->first));
        m_ListBox->Check(item, 
            find(m_Filters.selected.begin(), m_Filters.selected.end(), item)!=m_Filters.selected.end());    
    }  

    if (m_ListBox->GetCount()) {
        m_ListBox->Select(0);
        x_SelectionUpdated();
    }

    return wxDialog::TransferDataToWindow();
}


bool CFilterDialog::TransferDataFromWindow()
{
    m_Filters.selected.clear();
    for (size_t i=0; i < m_ListBox->GetCount(); i++) {
        if (m_ListBox->IsChecked((unsigned)i)) {
            m_Filters.selected.push_back((int)i);
        }    
    }
    return wxDialog::TransferDataFromWindow();    
}

void CFilterDialog::OnClearClicked( wxCommandEvent& event )
{
    m_Filters.selected.clear();
    EndModal(wxID_OK);
}


void CFilterDialog::OnChecklistbox1Selected( wxCommandEvent& event )
{
    x_SelectionUpdated();
    event.Skip();
}

void CFilterDialog::x_SelectionUpdated()
{
    int sel = m_ListBox->GetSelection();
    m_Name->SetValue(ToWxString(m_Filters.filters[sel].first));
    m_Expression->SetValue(ToWxString(m_Filters.filters[sel].second));
}

END_NCBI_SCOPE
