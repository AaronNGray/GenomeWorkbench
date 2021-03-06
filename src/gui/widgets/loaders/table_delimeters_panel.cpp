/*  $Id: table_delimeters_panel.cpp 43700 2019-08-14 20:31:35Z katargir $
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
 * Authors: Bob Falk
 *
 * File Description:
 *
 */

#include <ncbi_pch.hpp>////@begin includes
#include "wx/imaglist.h"
////@end includes

#include "table_delimeters_panel.hpp"
#include <gui/objutils/registry.hpp>

#include <wx/sizer.h>
#include <wx/stattext.h>
#include <wx/statbox.h>
#include <wx/icon.h>
#include <wx/bitmap.h>
#include <wx/msgdlg.h>
#include <wx/radiobut.h>
#include <wx/listctrl.h>
#include <wx/checkbox.h>
#include <wx/textctrl.h>

////@begin XPM images
////@end XPM images

BEGIN_NCBI_SCOPE
USING_SCOPE(objects);

IMPLEMENT_DYNAMIC_CLASS( CTableDelimitersPanel, wxPanel )

BEGIN_EVENT_TABLE( CTableDelimitersPanel, wxPanel )

////@begin CTableDelimitersPanel event table entries
    EVT_CHECKBOX( ID_TABDELIMCHECK, CTableDelimitersPanel::OnTabDelimCheckClick )
    EVT_CHECKBOX( ID_SEMICOLONDELIMCHECK, CTableDelimitersPanel::OnSemiColonDelimCheckClick )
    EVT_CHECKBOX( ID_COMMADELIMCHECK, CTableDelimitersPanel::OnCommaDelimCheckClick )
    EVT_CHECKBOX( ID_SPACEDELIMCHECK, CTableDelimitersPanel::OnSpaceDelimCheckClick )
    EVT_CHECKBOX( ID_OTHERDELIMCHECK, CTableDelimitersPanel::OnOtherDelimCheckClick )
    EVT_TEXT( ID_TABLEDELIMITERTEXTCTRL, CTableDelimitersPanel::OnTableDelimiterTextCtrlTextUpdated )
    EVT_CHECKBOX( ID_ADJACENTDELIMITERSCHECK, CTableDelimitersPanel::OnAdjacentDelimitersCheckClick )
    EVT_RADIOBOX( ID_RADIOBOX, CTableDelimitersPanel::OnRadioBoxSelected )
////@end CTableDelimitersPanel event table entries

    EVT_LIST_COL_DRAGGING( ID_CTABLEIMPORTLISTCTRL2, CTableDelimitersPanel::OnCtableImportListctrlColDragging )

END_EVENT_TABLE()

CTableDelimitersPanel::CTableDelimitersPanel()
{
    Init();
}

CTableDelimitersPanel::CTableDelimitersPanel( wxWindow* parent, wxWindowID id, const wxString& caption, const wxPoint& pos, const wxSize& size, long style )
{
    Init();
    Create(parent, id, caption, pos, size, style);
}

bool CTableDelimitersPanel::Create( wxWindow* parent, wxWindowID id, const wxString& caption, const wxPoint& pos, const wxSize& size, long style )
{
////@begin CTableDelimitersPanel creation
    SetExtraStyle(wxWS_EX_BLOCK_EVENTS);
    wxPanel::Create( parent, id, pos, size, style );

    CreateControls();
    if (GetSizer())
    {
        GetSizer()->SetSizeHints(this);
    }
    Centre();
////@end CTableDelimitersPanel creation
    return true;
}

CTableDelimitersPanel::~CTableDelimitersPanel()
{
////@begin CTableDelimitersPanel destruction
////@end CTableDelimitersPanel destruction
}

void CTableDelimitersPanel::Init()
{
////@begin CTableDelimitersPanel member initialisation
    m_TabDelimCheck = NULL;
    m_SemicolonDelimCheck = NULL;
    m_CommaDelimCheck = NULL;
    m_SpaceDelimCheck = NULL;
    m_OtherDelimCheck = NULL;
    m_DelimTextCtrl = NULL;
    m_MergeAdjacentDelimiters = NULL;
    m_TextQualifier = NULL;
    m_ListCtrlPanel = NULL;
    m_DelimiterListCtrl = NULL;
////@end CTableDelimitersPanel member initialisation

    m_DelimsInitialized = false;
}

void CTableDelimitersPanel::CreateControls()
{    
////@begin CTableDelimitersPanel content construction
    // Generated by DialogBlocks, 14/08/2019 16:30:17 (unregistered)

    CTableDelimitersPanel* itemPanel1 = this;

    wxBoxSizer* itemBoxSizer2 = new wxBoxSizer(wxVERTICAL);
    itemPanel1->SetSizer(itemBoxSizer2);

    wxStaticText* itemStaticText3 = new wxStaticText( itemPanel1, wxID_STATIC, _("Set Delimiters to Separate Data into Columns"), wxDefaultPosition, wxDefaultSize, 0 );
    itemStaticText3->SetFont(wxFont(10, wxFONTFAMILY_SWISS, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL, false, wxT("Tahoma")));
    itemBoxSizer2->Add(itemStaticText3, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

    wxBoxSizer* itemBoxSizer4 = new wxBoxSizer(wxHORIZONTAL);
    itemBoxSizer2->Add(itemBoxSizer4, 0, wxALIGN_LEFT|wxALL, 5);

    wxStaticBox* itemStaticBoxSizer5Static = new wxStaticBox(itemPanel1, wxID_ANY, _("Set Column Delimiters"));
    wxStaticBoxSizer* itemStaticBoxSizer5 = new wxStaticBoxSizer(itemStaticBoxSizer5Static, wxVERTICAL);
    itemBoxSizer4->Add(itemStaticBoxSizer5, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    m_TabDelimCheck = new wxCheckBox( itemStaticBoxSizer5->GetStaticBox(), ID_TABDELIMCHECK, _("Tab"), wxDefaultPosition, wxDefaultSize, 0 );
    m_TabDelimCheck->SetValue(false);
    if (CTableDelimitersPanel::ShowToolTips())
        m_TabDelimCheck->SetToolTip(_("Check if table columns are separated by tabs"));
    itemStaticBoxSizer5->Add(m_TabDelimCheck, 0, wxALIGN_LEFT|wxALL, 5);

    m_SemicolonDelimCheck = new wxCheckBox( itemStaticBoxSizer5->GetStaticBox(), ID_SEMICOLONDELIMCHECK, _("Semicolon"), wxDefaultPosition, wxDefaultSize, 0 );
    m_SemicolonDelimCheck->SetValue(false);
    if (CTableDelimitersPanel::ShowToolTips())
        m_SemicolonDelimCheck->SetToolTip(_("Check if table columns are separated by semicolons"));
    itemStaticBoxSizer5->Add(m_SemicolonDelimCheck, 0, wxALIGN_LEFT|wxALL, 5);

    m_CommaDelimCheck = new wxCheckBox( itemStaticBoxSizer5->GetStaticBox(), ID_COMMADELIMCHECK, _("Comma"), wxDefaultPosition, wxDefaultSize, 0 );
    m_CommaDelimCheck->SetValue(false);
    itemStaticBoxSizer5->Add(m_CommaDelimCheck, 0, wxALIGN_LEFT|wxALL, 5);

    m_SpaceDelimCheck = new wxCheckBox( itemStaticBoxSizer5->GetStaticBox(), ID_SPACEDELIMCHECK, _("Space"), wxDefaultPosition, wxDefaultSize, 0 );
    m_SpaceDelimCheck->SetValue(false);
    itemStaticBoxSizer5->Add(m_SpaceDelimCheck, 0, wxALIGN_LEFT|wxALL, 5);

    wxBoxSizer* itemBoxSizer10 = new wxBoxSizer(wxHORIZONTAL);
    itemStaticBoxSizer5->Add(itemBoxSizer10, 0, wxALIGN_LEFT|wxRIGHT, 5);

    m_OtherDelimCheck = new wxCheckBox( itemStaticBoxSizer5->GetStaticBox(), ID_OTHERDELIMCHECK, _("Other"), wxDefaultPosition, wxDefaultSize, 0 );
    m_OtherDelimCheck->SetValue(false);
    if (CTableDelimitersPanel::ShowToolTips())
        m_OtherDelimCheck->SetToolTip(_("For a separators not listed above"));
    itemBoxSizer10->Add(m_OtherDelimCheck, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    m_DelimTextCtrl = new wxTextCtrl( itemStaticBoxSizer5->GetStaticBox(), ID_TABLEDELIMITERTEXTCTRL, wxEmptyString, wxDefaultPosition, wxSize(20, -1), 0 );
    m_DelimTextCtrl->SetMaxLength(1);
    if (CTableDelimitersPanel::ShowToolTips())
        m_DelimTextCtrl->SetToolTip(_("Type single character tabel delimiter"));
    itemBoxSizer10->Add(m_DelimTextCtrl, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxBoxSizer* itemBoxSizer13 = new wxBoxSizer(wxVERTICAL);
    itemBoxSizer4->Add(itemBoxSizer13, 0, wxALIGN_TOP|wxALL, 5);

    m_MergeAdjacentDelimiters = new wxCheckBox( itemPanel1, ID_ADJACENTDELIMITERSCHECK, _("Treat adjacent delimiters as a single delimiter"), wxDefaultPosition, wxDefaultSize, 0 );
    m_MergeAdjacentDelimiters->SetValue(false);
    itemBoxSizer13->Add(m_MergeAdjacentDelimiters, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

    wxArrayString m_TextQualifierStrings;
    m_TextQualifierStrings.Add(_("&Double Quote: \""));
    m_TextQualifierStrings.Add(_("&Single Quote: '"));
    m_TextQualifierStrings.Add(_("&No Text Qualifier"));
    m_TextQualifier = new wxRadioBox( itemPanel1, ID_RADIOBOX, _("Text Qualifier Selection"), wxDefaultPosition, wxDefaultSize, m_TextQualifierStrings, 3, wxRA_SPECIFY_ROWS );
    m_TextQualifier->SetSelection(2);
    itemBoxSizer13->Add(m_TextQualifier, 0, wxALIGN_LEFT|wxALL, 5);

    wxStaticBox* itemStaticBoxSizer16Static = new wxStaticBox(itemPanel1, wxID_ANY, _("Table Data"));
    wxStaticBoxSizer* itemStaticBoxSizer16 = new wxStaticBoxSizer(itemStaticBoxSizer16Static, wxHORIZONTAL);
    itemBoxSizer2->Add(itemStaticBoxSizer16, 1, wxGROW|wxALL, 5);

    m_ListCtrlPanel = new wxPanel( itemStaticBoxSizer16->GetStaticBox(), ID_PANEL3, wxDefaultPosition, wxDefaultSize, wxNO_BORDER|wxTAB_TRAVERSAL );
    itemStaticBoxSizer16->Add(m_ListCtrlPanel, 1, wxGROW|wxTOP, 5);

    wxBoxSizer* itemBoxSizer18 = new wxBoxSizer(wxHORIZONTAL);
    m_ListCtrlPanel->SetSizer(itemBoxSizer18);

    m_DelimiterListCtrl = new CTableImportListCtrl( m_ListCtrlPanel, ID_CTABLEIMPORTLISTCTRL2, wxDefaultPosition, wxSize(100, 100), 0 );
    itemBoxSizer18->Add(m_DelimiterListCtrl, 1, wxGROW|wxALL, 5);

////@end CTableDelimitersPanel content construction
}

bool CTableDelimitersPanel::Show(bool show)
{
    if (show) {
        m_TabDelimCheck->SetValue(false);
        m_SemicolonDelimCheck->SetValue(false);
        m_CommaDelimCheck->SetValue(false);
        m_SpaceDelimCheck->SetValue(false);
        m_OtherDelimCheck->SetValue(false);
        m_DelimTextCtrl->SetValue(wxT(""));

        if (!m_ImportedTableData.IsNull()) {
            m_DelimiterListCtrl->SetViewType(CTableImportListCtrl::eMultiColumn);
            m_DelimiterListCtrl->SetDataSource(m_ImportedTableData);

            for (size_t i=0; i<m_ImportedTableData->GetDelimiters().size(); ++i) {
                if (m_ImportedTableData->GetDelimiters()[i] == '\t') {
                    m_TabDelimCheck->SetValue(true);
                }
                else if (m_ImportedTableData->GetDelimiters()[i] == ';') {
                    m_SemicolonDelimCheck->SetValue(true);
                }
                else if (m_ImportedTableData->GetDelimiters()[i] == ',') {
                    m_CommaDelimCheck->SetValue(true);
                }
                else if (m_ImportedTableData->GetDelimiters()[i] == ' ') {
                    m_SpaceDelimCheck->SetValue(true);
                }
                else {
                    m_OtherDelimCheck->SetValue(true);
                    char delim = m_ImportedTableData->GetDelimiters()[i];
                    string delim_str(1, delim);
                    m_DelimTextCtrl->SetValue(ToWxString(delim_str));
                }
            }

            m_MergeAdjacentDelimiters->SetValue(
                m_ImportedTableData->GetDelimiterRules().GetMergeDelimiters());

            char quote_char = m_ImportedTableData->GetDelimiterRules().GetQuoteChar();

            if (quote_char == '\"')
                m_TextQualifier->SetSelection(0);
            if (quote_char == '\'')
                m_TextQualifier->SetSelection(1);
            if (quote_char == ' ')
                m_TextQualifier->SetSelection(2);
        }

        m_DelimsInitialized = true;
    }
    else {
        m_DelimsInitialized = false;
    }

    
    return wxPanel::Show(show);
}

void CTableDelimitersPanel::SetMainTitle(const wxString& title)
{
}

bool CTableDelimitersPanel::IsInputValid()
{
    return true;
}

void CTableDelimitersPanel::UpdateDelimiters()
{
    // Don't do this before panel is displayed (in that case we use
    // the delimiter(s) already in the m_ImportedTableData)
    if (!m_DelimsInitialized)
        return;

    vector<char> new_delims;

    if (m_TabDelimCheck->GetValue()) 
        new_delims.push_back('\t');
    if (m_SemicolonDelimCheck->GetValue()) 
        new_delims.push_back(';');
    if (m_CommaDelimCheck->GetValue()) 
        new_delims.push_back(',');
    if (m_SpaceDelimCheck->GetValue()) 
        new_delims.push_back(' ');
    if (m_OtherDelimCheck->GetValue()) {
        string sval = ToStdString(m_DelimTextCtrl->GetValue());
        if (sval.length() == 1) {
            char schar = sval.c_str()[0];
            if (schar != '\t' && 
                schar != ';' && 
                schar != ',' &&
                schar != ' ')
                    new_delims.push_back(schar);
        }
    }

    bool merge_delimiters = m_MergeAdjacentDelimiters->GetValue();

    // Get text qualifier based on selection number in corresponding
    // radio box.
    int selected_qualifier_idx = m_TextQualifier->GetSelection();    
    char text_qualifier = ' ';    
    if (selected_qualifier_idx == 0)
        text_qualifier = '\"';
    else if (selected_qualifier_idx == 1)
        text_qualifier ='\'';

    if (!m_ImportedTableData.IsNull()) {
        bool update = false;

        if (m_ImportedTableData->GetDelimiterRules().GetMergeDelimiters() != 
            merge_delimiters) {
                m_ImportedTableData->GetDelimiterRules().SetMergeDelimiters(merge_delimiters);
                update = true;
        }

        if (m_ImportedTableData->GetDelimiterRules().GetQuoteChar() != 
            text_qualifier) {
                m_ImportedTableData->GetDelimiterRules().SetQuoteChar(text_qualifier);
                update = true;
        }

        if (!m_ImportedTableData->GetDelimiterRules().MatchingDelimiters(new_delims)) {
            m_ImportedTableData->GetDelimiterRules().SetDelimiters(new_delims);
            update = true;
        }

        if (update) {
            m_ImportedTableData->RecomputeFields(true);
            m_DelimiterListCtrl->SetDataSource(m_ImportedTableData);
        }

        ERR_POST(Info << "Import Table User Updated Delimiters");
        m_ImportedTableData->GetDelimiterRules().LogDelims();
    }
}


void CTableDelimitersPanel::SetRegistryPath( const string& path )
{
    m_RegPath = path;
}


void CTableDelimitersPanel::LoadSettings()
{
}


void CTableDelimitersPanel::SaveSettings() const
{
}

bool CTableDelimitersPanel::ShowToolTips()
{
    return true;
}
wxBitmap CTableDelimitersPanel::GetBitmapResource( const wxString& name )
{
    // Bitmap retrieval
////@begin CTableDelimitersPanel bitmap retrieval
    wxUnusedVar(name);
    return wxNullBitmap;
////@end CTableDelimitersPanel bitmap retrieval
}
wxIcon CTableDelimitersPanel::GetIconResource( const wxString& name )
{
    // Icon retrieval
////@begin CTableDelimitersPanel icon retrieval
    wxUnusedVar(name);
    return wxNullIcon;
////@end CTableDelimitersPanel icon retrieval
}

void CTableDelimitersPanel::OnTabDelimCheckClick( wxCommandEvent& event )
{
    UpdateDelimiters();
}

void CTableDelimitersPanel::OnSemiColonDelimCheckClick( wxCommandEvent& event )
{
    UpdateDelimiters(); 
}

void CTableDelimitersPanel::OnCommaDelimCheckClick( wxCommandEvent& event )
{
    UpdateDelimiters(); 
}

void CTableDelimitersPanel::OnSpaceDelimCheckClick( wxCommandEvent& event )
{
    UpdateDelimiters(); 
}

void CTableDelimitersPanel::OnOtherDelimCheckClick( wxCommandEvent& event )
{
    UpdateDelimiters(); 
}

void CTableDelimitersPanel::OnTableDelimiterTextCtrlTextUpdated( wxCommandEvent& event )
{
    UpdateDelimiters(); 
}

void CTableDelimitersPanel::OnAdjacentDelimitersCheckClick( wxCommandEvent& event )
{
    UpdateDelimiters(); 
}

void CTableDelimitersPanel::OnRadioBoxSelected( wxCommandEvent& event )
{
    UpdateDelimiters(); 
}

void CTableDelimitersPanel::OnCtableImportListctrlColDragging( wxListEvent& event )
{
    // Update the whole control when dragging.  This is needed because there
    // may be header rows where distrubtion across columns is based on width
    // of each column (basically they are left-justified into row irrespective
    // of column boundaries)
    m_DelimiterListCtrl->Refresh();
}

END_NCBI_SCOPE
