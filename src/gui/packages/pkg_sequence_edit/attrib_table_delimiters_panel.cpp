/*  $Id: attrib_table_delimiters_panel.cpp 43763 2019-08-28 17:32:50Z katargir $
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
 * Authors:  Bob Falk
 */


#include <ncbi_pch.hpp>

#include <corelib/ncbifile.hpp>

////@begin includes
#include "wx/imaglist.h"
////@end includes

#include "attrib_table_delimiters_panel.hpp"

#include <gui/packages/pkg_sequence_edit/field_choice_panel.hpp>

#include <gui/widgets/wx/async_call.hpp>
#include <gui/widgets/wx/wx_utils.hpp>

#include <gui/core/ui_tool_manager.hpp>

#include <objects/seqloc/Seq_id.hpp>

#include <wx/msgdlg.h>
#include <wx/utils.h>



BEGIN_NCBI_SCOPE
USING_SCOPE (objects);
////@begin XPM images
////@end XPM images

IMPLEMENT_DYNAMIC_CLASS( CAttribTableDelimitersPanel, wxPanel )


/*!
 * CAttribTableDelimitersPanel event table definition
 */

BEGIN_EVENT_TABLE( CAttribTableDelimitersPanel, wxPanel )

////@begin CAttribTableDelimitersPanel event table entries
    EVT_CHECKBOX( ID_TABDELIMCHECK, CAttribTableDelimitersPanel::OnTabDelimCheckClick )

    EVT_CHECKBOX( ID_SEMICOLONDELIMCHECK, CAttribTableDelimitersPanel::OnSemiColonDelimCheckClick )

    EVT_CHECKBOX( ID_COMMADELIMCHECK, CAttribTableDelimitersPanel::OnCommaDelimCheckClick )

    EVT_CHECKBOX( ID_SPACEDELIMCHECK, CAttribTableDelimitersPanel::OnSpaceDelimCheckClick )

    EVT_CHECKBOX( ID_MULTSPANCEONLYCHECK, CAttribTableDelimitersPanel::OnMultSpanceOnlyCheckClick )

    EVT_CHECKBOX( ID_OTHERDELIMCHECK, CAttribTableDelimitersPanel::OnOtherDelimCheckClick )

    EVT_TEXT( ID_TABLEDELIMITERTEXTCTRL, CAttribTableDelimitersPanel::OnTableDelimiterTextCtrlTextUpdated )

    EVT_CHECKBOX( ID_ADJACENTDELIMITERSCHECK, CAttribTableDelimitersPanel::OnAdjacentDelimitersCheckClick )

    EVT_RADIOBOX( ID_TEXTQUALIFIER, CAttribTableDelimitersPanel::OnTextQualifierSelected )

    EVT_CHECKBOX( ID_PARSECOLUMNNAMESBTN, CAttribTableDelimitersPanel::OnParseColumnNamesBtnClick )

    EVT_BUTTON( ID_SPLITCOLBTN, CAttribTableDelimitersPanel::OnSplitColBtnClick )

    EVT_BUTTON( ID_MERGECOLSBTN, CAttribTableDelimitersPanel::OnMergeColsBtnClick )

    EVT_BUTTON( ID_EXPORTBTN, CAttribTableDelimitersPanel::OnExportBtnClick )

    EVT_TEXT( ID_TEXTMSDELIMITER, CAttribTableDelimitersPanel::OnTextMSDelimiterTextUpdated )

    EVT_LIST_ITEM_SELECTED( ID_LISTCTRL, CAttribTableDelimitersPanel::OnCTableImportListctrl2Selected )
    EVT_LIST_COL_CLICK( ID_LISTCTRL, CAttribTableDelimitersPanel::OnListctrlColLeftClick )
    EVT_LIST_COL_DRAGGING( ID_LISTCTRL, CAttribTableDelimitersPanel::OnCtableImportListctrl2ColDragging )

    EVT_TEXT( ID_DELIMROWTEXT, CAttribTableDelimitersPanel::OnDelimRowTextUpdated )

    EVT_BUTTON( ID_REVERTUPDATEBUTTON, CAttribTableDelimitersPanel::OnRevertUpdateButtonClick )

////@end CAttribTableDelimitersPanel event table entries

END_EVENT_TABLE()


/*!
 * CAttribTableDelimitersPanel constructors
 */

CAttribTableDelimitersPanel::CAttribTableDelimitersPanel()
{
    Init();
}

CAttribTableDelimitersPanel::CAttribTableDelimitersPanel( wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style )
{
    Init();
    Create(parent, id, pos, size, style);
}


/*!
 * CAttribTableDelimitersPanel creator
 */

bool CAttribTableDelimitersPanel::Create( wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style )
{
////@begin CAttribTableDelimitersPanel creation
    SetExtraStyle(wxWS_EX_BLOCK_EVENTS);
    wxPanel::Create( parent, id, pos, size, style );

    CreateControls();
    if (GetSizer())
    {
        GetSizer()->SetSizeHints(this);
    }
    Centre();
////@end CAttribTableDelimitersPanel creation
    return true;
}


/*!
 * CAttribTableDelimitersPanel destructor
 */

CAttribTableDelimitersPanel::~CAttribTableDelimitersPanel()
{
////@begin CAttribTableDelimitersPanel destruction
////@end CAttribTableDelimitersPanel destruction
}


/*!
 * Member initialisation
 */

void CAttribTableDelimitersPanel::Init()
{
////@begin CAttribTableDelimitersPanel member initialisation
    m_TabDelimCheck = NULL;
    m_SemicolonDelimCheck = NULL;
    m_CommaDelimCheck = NULL;
    m_SpaceDelimCheck = NULL;
    m_MultipleSpacesOnly = NULL;
    m_OtherDelimCheck = NULL;
    m_DelimTextCtrl = NULL;
    m_MergeAdjacentDelimiters = NULL;
    m_TextQualifier = NULL;
    m_ParseColumnNamesBtn = NULL;
    m_SplitColBtn = NULL;
    m_MergeColsBtn = NULL;
    m_MergeSplitStaticText = NULL;
    m_TabMSBtn = NULL;
    m_SpaceMSBtn = NULL;
    m_WhitespaceMSBtn = NULL;
    m_ColonMSBtn = NULL;
    m_SemicolonMSBtn = NULL;
    m_CommaMSBtn = NULL;
    m_NoneMSBtn = NULL;
    m_OtherMSBtn = NULL;
    m_DelimMSText = NULL;
    m_DelimiterListCtrl = NULL;
    m_DelimRowText = NULL;
    m_RevertRowUpdate = NULL;
////@end CAttribTableDelimitersPanel member initialisation

    m_DelimsInitialized = false;
    m_EditRowIdx = -1;
    m_ToolManager = NULL;
    m_FileUpdated = false;
}


/*!
 * Control creation for CAttribTableDelimitersPanel
 */

void CAttribTableDelimitersPanel::CreateControls()
{    
////@begin CAttribTableDelimitersPanel content construction
    CAttribTableDelimitersPanel* itemPanel1 = this;

    wxBoxSizer* itemBoxSizer2 = new wxBoxSizer(wxVERTICAL);
    itemPanel1->SetSizer(itemBoxSizer2);

    wxStaticText* itemStaticText3 = new wxStaticText( itemPanel1, wxID_STATIC, _("Set Delimiters to Separate Data into Columns"), wxDefaultPosition, wxDefaultSize, 0 );
    itemStaticText3->SetFont(wxFont(10, wxSWISS, wxNORMAL, wxNORMAL, false, wxT("Tahoma")));
    itemBoxSizer2->Add(itemStaticText3, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

    wxBoxSizer* itemBoxSizer4 = new wxBoxSizer(wxHORIZONTAL);
    itemBoxSizer2->Add(itemBoxSizer4, 0, wxALIGN_LEFT|wxALL, 5);

    wxStaticBox* itemStaticBoxSizer5Static = new wxStaticBox(itemPanel1, wxID_ANY, _("Set Column Delimiters"));
    wxStaticBoxSizer* itemStaticBoxSizer5 = new wxStaticBoxSizer(itemStaticBoxSizer5Static, wxVERTICAL);
    itemBoxSizer4->Add(itemStaticBoxSizer5, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    m_TabDelimCheck = new wxCheckBox( itemStaticBoxSizer5->GetStaticBox(), ID_TABDELIMCHECK, _("Tab"), wxDefaultPosition, wxDefaultSize, 0 );
    m_TabDelimCheck->SetValue(false);
    if (CAttribTableDelimitersPanel::ShowToolTips())
        m_TabDelimCheck->SetToolTip(_("Check if table columns are separated by tabs"));
    itemStaticBoxSizer5->Add(m_TabDelimCheck, 0, wxALIGN_LEFT|wxALL, 5);

    m_SemicolonDelimCheck = new wxCheckBox( itemStaticBoxSizer5->GetStaticBox(), ID_SEMICOLONDELIMCHECK, _("Semicolon"), wxDefaultPosition, wxDefaultSize, 0 );
    m_SemicolonDelimCheck->SetValue(false);
    if (CAttribTableDelimitersPanel::ShowToolTips())
        m_SemicolonDelimCheck->SetToolTip(_("Check if table columns are separated by semicolons"));
    itemStaticBoxSizer5->Add(m_SemicolonDelimCheck, 0, wxALIGN_LEFT|wxALL, 5);

    m_CommaDelimCheck = new wxCheckBox( itemStaticBoxSizer5->GetStaticBox(), ID_COMMADELIMCHECK, _("Comma"), wxDefaultPosition, wxDefaultSize, 0 );
    m_CommaDelimCheck->SetValue(false);
    itemStaticBoxSizer5->Add(m_CommaDelimCheck, 0, wxALIGN_LEFT|wxALL, 5);

    wxBoxSizer* itemBoxSizer9 = new wxBoxSizer(wxHORIZONTAL);
    itemStaticBoxSizer5->Add(itemBoxSizer9, 0, wxALIGN_LEFT|wxRIGHT, 5);

    m_SpaceDelimCheck = new wxCheckBox( itemStaticBoxSizer5->GetStaticBox(), ID_SPACEDELIMCHECK, _("Space"), wxDefaultPosition, wxDefaultSize, 0 );
    m_SpaceDelimCheck->SetValue(false);
    itemBoxSizer9->Add(m_SpaceDelimCheck, 0, wxALIGN_TOP|wxALL, 5);

    m_MultipleSpacesOnly = new wxCheckBox( itemStaticBoxSizer5->GetStaticBox(), ID_MULTSPANCEONLYCHECK, _("Multiple Spaces Only"), wxDefaultPosition, wxDefaultSize, 0 );
    m_MultipleSpacesOnly->SetValue(false);
    m_MultipleSpacesOnly->Enable(false);
    itemBoxSizer9->Add(m_MultipleSpacesOnly, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxBoxSizer* itemBoxSizer12 = new wxBoxSizer(wxHORIZONTAL);
    itemStaticBoxSizer5->Add(itemBoxSizer12, 0, wxALIGN_LEFT|wxRIGHT, 5);

    m_OtherDelimCheck = new wxCheckBox( itemStaticBoxSizer5->GetStaticBox(), ID_OTHERDELIMCHECK, _("Other"), wxDefaultPosition, wxDefaultSize, 0 );
    m_OtherDelimCheck->SetValue(false);
    if (CAttribTableDelimitersPanel::ShowToolTips())
        m_OtherDelimCheck->SetToolTip(_("For a separators not listed above"));
    itemBoxSizer12->Add(m_OtherDelimCheck, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    m_DelimTextCtrl = new wxTextCtrl( itemStaticBoxSizer5->GetStaticBox(), ID_TABLEDELIMITERTEXTCTRL, wxEmptyString, wxDefaultPosition, wxSize(20, -1), 0 );
    m_DelimTextCtrl->SetMaxLength(1);
    if (CAttribTableDelimitersPanel::ShowToolTips())
        m_DelimTextCtrl->SetToolTip(_("Type single character tabel delimiter"));
    itemBoxSizer12->Add(m_DelimTextCtrl, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxBoxSizer* itemBoxSizer15 = new wxBoxSizer(wxVERTICAL);
    itemBoxSizer4->Add(itemBoxSizer15, 1, wxGROW|wxALL, 5);

    m_MergeAdjacentDelimiters = new wxCheckBox( itemPanel1, ID_ADJACENTDELIMITERSCHECK, _("Treat adjacent delimiters as a single delimiter"), wxDefaultPosition, wxDefaultSize, 0 );
    m_MergeAdjacentDelimiters->SetValue(false);
    itemBoxSizer15->Add(m_MergeAdjacentDelimiters, 0, wxALIGN_LEFT|wxALL, 5);

    wxArrayString m_TextQualifierStrings;
    m_TextQualifierStrings.Add(_("&Double Quote: \""));
    m_TextQualifierStrings.Add(_("&Single Quote: '"));
    m_TextQualifierStrings.Add(_("&No Text Qualifier"));
    m_TextQualifier = new wxRadioBox( itemPanel1, ID_TEXTQUALIFIER, _("Text Qualifier Selection"), wxDefaultPosition, wxDefaultSize, m_TextQualifierStrings, 1, 0 );
    m_TextQualifier->SetSelection(2);
    itemBoxSizer15->Add(m_TextQualifier, 0, wxALIGN_LEFT|wxALL, 5);

    m_ParseColumnNamesBtn = new wxCheckBox( itemPanel1, ID_PARSECOLUMNNAMESBTN, _("Has Header Row"), wxDefaultPosition, wxDefaultSize, 0 );
    m_ParseColumnNamesBtn->SetValue(true);
    itemBoxSizer15->Add(m_ParseColumnNamesBtn, 0, wxALIGN_LEFT|wxALL, 5);

    wxStaticBox* itemStaticBoxSizer19Static = new wxStaticBox(itemPanel1, wxID_ANY, _("Table Data"));
    wxStaticBoxSizer* itemStaticBoxSizer19 = new wxStaticBoxSizer(itemStaticBoxSizer19Static, wxHORIZONTAL);
    itemBoxSizer2->Add(itemStaticBoxSizer19, 1, wxGROW|wxALL, 5);

    wxPanel* itemPanel20 = new wxPanel( itemStaticBoxSizer19->GetStaticBox(), ID_PANEL3, wxDefaultPosition, wxDefaultSize, wxNO_BORDER|wxTAB_TRAVERSAL );
    itemStaticBoxSizer19->Add(itemPanel20, 1, wxGROW|wxTOP, 5);

    wxBoxSizer* itemBoxSizer21 = new wxBoxSizer(wxVERTICAL);
    itemPanel20->SetSizer(itemBoxSizer21);

    wxBoxSizer* itemBoxSizer22 = new wxBoxSizer(wxHORIZONTAL);
    itemBoxSizer21->Add(itemBoxSizer22, 0, wxGROW|wxLEFT|wxRIGHT, 5);

    m_SplitColBtn = new wxButton( itemPanel20, ID_SPLITCOLBTN, _("Split"), wxDefaultPosition, wxDefaultSize, 0 );
    if (CAttribTableDelimitersPanel::ShowToolTips())
        m_SplitColBtn->SetToolTip(_("Split A Column Into Two"));
    m_SplitColBtn->Enable(false);
    itemBoxSizer22->Add(m_SplitColBtn, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    m_MergeColsBtn = new wxButton( itemPanel20, ID_MERGECOLSBTN, _("Merge"), wxDefaultPosition, wxDefaultSize, 0 );
    if (CAttribTableDelimitersPanel::ShowToolTips())
        m_MergeColsBtn->SetToolTip(_("Merge Adjacent Columns"));
    m_MergeColsBtn->Enable(false);
    itemBoxSizer22->Add(m_MergeColsBtn, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxButton* itemButton25 = new wxButton( itemPanel20, ID_EXPORTBTN, _("Export..."), wxDefaultPosition, wxDefaultSize, 0 );
    if (CAttribTableDelimitersPanel::ShowToolTips())
        itemButton25->SetToolTip(_("Save Updated File"));
    itemBoxSizer22->Add(itemButton25, 0, wxALIGN_TOP|wxALL, 5);

    wxStaticBox* itemStaticBoxSizer26Static = new wxStaticBox(itemPanel20, wxID_ANY, wxEmptyString);
    wxStaticBoxSizer* itemStaticBoxSizer26 = new wxStaticBoxSizer(itemStaticBoxSizer26Static, wxHORIZONTAL);
    itemBoxSizer21->Add(itemStaticBoxSizer26, 0, wxALIGN_LEFT|wxLEFT|wxRIGHT, 5);

    m_MergeSplitStaticText = new wxStaticText( itemStaticBoxSizer26->GetStaticBox(), wxID_STATIC, _("Merge/Split Char:"), wxDefaultPosition, wxDefaultSize, 0 );
    m_MergeSplitStaticText->Enable(false);
    itemStaticBoxSizer26->Add(m_MergeSplitStaticText, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    m_TabMSBtn = new wxRadioButton( itemStaticBoxSizer26->GetStaticBox(), ID_TABMSBTN, _("Tab"), wxDefaultPosition, wxDefaultSize, 0 );
    m_TabMSBtn->SetValue(false);
    m_TabMSBtn->Enable(false);
    itemStaticBoxSizer26->Add(m_TabMSBtn, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    m_SpaceMSBtn = new wxRadioButton( itemStaticBoxSizer26->GetStaticBox(), ID_SPACEMSBTN, _("Space"), wxDefaultPosition, wxDefaultSize, 0 );
    m_SpaceMSBtn->SetValue(false);
    m_SpaceMSBtn->Enable(false);
    itemStaticBoxSizer26->Add(m_SpaceMSBtn, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    m_WhitespaceMSBtn = new wxRadioButton( itemStaticBoxSizer26->GetStaticBox(), ID_WHITESPACEMSBTN, _("Whitespace"), wxDefaultPosition, wxDefaultSize, 0 );
    m_WhitespaceMSBtn->SetValue(false);
    itemStaticBoxSizer26->Add(m_WhitespaceMSBtn, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    m_ColonMSBtn = new wxRadioButton( itemStaticBoxSizer26->GetStaticBox(), ID_COLONMSBTN, _("Colon"), wxDefaultPosition, wxDefaultSize, 0 );
    m_ColonMSBtn->SetValue(false);
    m_ColonMSBtn->Enable(false);
    itemStaticBoxSizer26->Add(m_ColonMSBtn, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    m_SemicolonMSBtn = new wxRadioButton( itemStaticBoxSizer26->GetStaticBox(), ID_SEMICOLONMSBTN, _("Semicolon"), wxDefaultPosition, wxDefaultSize, 0 );
    m_SemicolonMSBtn->SetValue(false);
    itemStaticBoxSizer26->Add(m_SemicolonMSBtn, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    m_CommaMSBtn = new wxRadioButton( itemStaticBoxSizer26->GetStaticBox(), ID_COMMAMSBTN, _("Comma"), wxDefaultPosition, wxDefaultSize, 0 );
    m_CommaMSBtn->SetValue(false);
    itemStaticBoxSizer26->Add(m_CommaMSBtn, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    m_NoneMSBtn = new wxRadioButton( itemStaticBoxSizer26->GetStaticBox(), ID_NONEMSBTN, _("None"), wxDefaultPosition, wxDefaultSize, 0 );
    m_NoneMSBtn->SetValue(false);
    itemStaticBoxSizer26->Add(m_NoneMSBtn, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    m_OtherMSBtn = new wxRadioButton( itemStaticBoxSizer26->GetStaticBox(), ID_OTHERMSBTN, _("Other"), wxDefaultPosition, wxDefaultSize, 0 );
    m_OtherMSBtn->SetValue(false);
    m_OtherMSBtn->Enable(false);
    itemStaticBoxSizer26->Add(m_OtherMSBtn, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    m_DelimMSText = new wxTextCtrl( itemStaticBoxSizer26->GetStaticBox(), ID_TEXTMSDELIMITER, wxEmptyString, wxDefaultPosition, wxSize(20, -1), 0 );
    m_DelimMSText->SetMaxLength(1);
    m_DelimMSText->Enable(false);
    itemStaticBoxSizer26->Add(m_DelimMSText, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxBoxSizer* itemBoxSizer37 = new wxBoxSizer(wxHORIZONTAL);
    itemBoxSizer21->Add(itemBoxSizer37, 1, wxGROW|wxALL, 5);

    m_DelimiterListCtrl = new CTableImportListCtrl( itemPanel20, ID_LISTCTRL, wxDefaultPosition, wxDefaultSize, wxLC_REPORT|wxLC_VIRTUAL|wxLC_ALIGN_LEFT|wxLC_HRULES|wxLC_VRULES );
    itemBoxSizer37->Add(m_DelimiterListCtrl, 1, wxGROW|wxALL, 5);

    wxBoxSizer* itemBoxSizer39 = new wxBoxSizer(wxHORIZONTAL);
    itemBoxSizer21->Add(itemBoxSizer39, 0, wxGROW|wxALL, 5);

    m_DelimRowText = new wxTextCtrl( itemPanel20, ID_DELIMROWTEXT, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_PROCESS_TAB );
    itemBoxSizer39->Add(m_DelimRowText, 1, wxALIGN_CENTER_VERTICAL|wxLEFT|wxRIGHT, 5);

    m_RevertRowUpdate = new wxButton( itemPanel20, ID_REVERTUPDATEBUTTON, _("Revert"), wxDefaultPosition, wxDefaultSize, 0 );
    m_RevertRowUpdate->Enable(false);
    itemBoxSizer39->Add(m_RevertRowUpdate, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

////@end CAttribTableDelimitersPanel content construction
}

bool CAttribTableDelimitersPanel::Show(bool show)
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
                    m_MultipleSpacesOnly->Enable(true);
                }
                else {
                    m_OtherDelimCheck->SetValue(true);
                    char delim = m_ImportedTableData->GetDelimiters()[i];
                    string delim_str(1, delim);
                    m_DelimTextCtrl->SetValue(ToWxString(delim_str));
                }
            }

            if (m_ImportedTableData->GetDelimiters().size() == 0)
                x_EnableMergeSplitWidgets(false);
            else
                x_EnableMergeSplitWidgets(true);

            m_MergeAdjacentDelimiters->SetValue(
                m_ImportedTableData->GetDelimiterRules().GetMergeDelimiters());

            m_MultipleSpacesOnly->SetValue(
                m_ImportedTableData->GetDelimiterRules().GetMultipleSpacesOnly()); 

            char quote_char = m_ImportedTableData->GetDelimiterRules().GetQuoteChar();

            if (quote_char == '\"')
                m_TextQualifier->SetSelection(0);
            if (quote_char == '\'')
                m_TextQualifier->SetSelection(1);
            if (quote_char == ' ')
                m_TextQualifier->SetSelection(2);
        }

        // Set x mark for any skipped columns
        for (size_t col=1; col<m_ImportedTableData->GetColumns().size(); ++col) {
            // If we arrived here fromm second panel, there may be a current column and skipped
            // columns. we should unset current column
            m_ImportedTableData->GetColumn(col).SetIsCurrent(false);
        }

        // update column images (current, seq-id, skipped)
        m_DelimiterListCtrl->UpdateColumnImages(true);

        m_DelimsInitialized = true;
    }
    else {
        m_DelimsInitialized = false;
    }
    
    bool result = wxPanel::Show(show);

    if (show &&& m_ToolManager && m_FileUpdated) {
        // This is needed so that if the user goes back to this pane from the second panel,
        // we don't want to then jump straigt back to second panel
        m_FileUpdated = false;

        // Jump to second page if the delimiter is tab (as normally expected) IF
        // this is the first time the manager has shown this page.        
        if (m_ImportedTableData->GetDelimiters().size() == 1 &&
            m_ImportedTableData->GetDelimiters()[0] == '\t') {  
                wxCommandEvent ev(wxEVT_COMMAND_BUTTON_CLICKED, wxID_FORWARD);
                this->GetParent()->GetEventHandler()->ProcessEvent(ev);
        }
    }

    return result;
}

void CAttribTableDelimitersPanel::SetMainTitle(const wxString& title)
{
}

bool CAttribTableDelimitersPanel::IsInputValid()
{
    return true;
}

void CAttribTableDelimitersPanel::UpdateDelimiters()
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

    if (m_SpaceDelimCheck->GetValue()) {
        m_MultipleSpacesOnly->Enable(true);
    }
    else {
        m_MultipleSpacesOnly->SetValue(false);
        m_MultipleSpacesOnly->Enable(false);    
    }

    bool merge_delimiters = m_MergeAdjacentDelimiters->GetValue();
    bool multi_space_only = m_MultipleSpacesOnly->GetValue();

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

        if (m_ImportedTableData->GetDelimiterRules().GetMultipleSpacesOnly() != 
            multi_space_only) {
                m_ImportedTableData->GetDelimiterRules().SetMultipleSpacesOnly(multi_space_only);
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
            // This recomputes (and resizes) columns
            m_DelimiterListCtrl->SetDataSource(m_ImportedTableData);
        }

        if (new_delims.size() == 0)
            x_EnableMergeSplitWidgets(false);
        else
            x_EnableMergeSplitWidgets(true);

        ERR_POST(Info << "Import Table User Updated Delimiters");
        m_ImportedTableData->GetDelimiterRules().LogDelims();
    }
}


void CAttribTableDelimitersPanel::SetRegistryPath( const string& path )
{
    m_RegPath = path;
}


void CAttribTableDelimitersPanel::LoadSettings()
{
    if ( !m_RegPath.empty()) {
        CGuiRegistry& gui_reg = CGuiRegistry::GetInstance();
        CRegistryWriteView view = gui_reg.GetWriteView(m_RegPath);
    }
}


void CAttribTableDelimitersPanel::SaveSettings() const
{
    if ( !m_RegPath.empty()) {
        CGuiRegistry& gui_reg = CGuiRegistry::GetInstance();
        CRegistryWriteView view = gui_reg.GetWriteView(m_RegPath);
    }
}

bool CAttribTableDelimitersPanel::PreviewData()
{
    wxString s = m_PreviewFname;
    m_FileUpdated = true;

    try {

        Int8 filesize = -1;
        {
            CFile tstfile(s.c_str().AsChar());
            
            if (!tstfile.IsFile()) {            
                wxString msg = wxT("Error opening file: ") + s;
                wxMessageBox(msg, wxT("Error Opening or Loading File"),             
                    wxOK | wxICON_ERROR, this);
                return false;
            }

            filesize = tstfile.GetLength();
        }
        
        // Set listcontrol to an empty table first so that any ui events don't try to access
        // the loading table.
        CRef<CTableImportDataSource> empty_table;
        m_DelimiterListCtrl->SetDataSource(empty_table);

        if (!GUI_AsyncExec(
            [this, filesize](ICanceled& canceled)
            {
                return m_ImportedTableData->LoadTable(m_PreviewFname, filesize, &canceled) || canceled.IsCanceled();
            }, wxT("Loading Table..."))) {
            wxString msg = wxT("Error loading table from file: ") + m_PreviewFname;
            wxMessageBox(msg, wxT("Error Opening or Loading File"),             
                wxOK | wxICON_ERROR);
            return false;
        }     

        // If there was an error or user canceled the load there will be no rows
        // so just refresh and return.
        if (m_ImportedTableData->GetNumRows() == 0) {
            m_DelimiterListCtrl->SetDataSource(m_ImportedTableData);
            Refresh();
        }

        // Do not allow fixed-width tables here:
        if (m_ImportedTableData->GetTableType() != 
            CTableImportDataSource::eDelimitedTable) {
                m_ImportedTableData->SetTableType(CTableImportDataSource::eDelimitedTable);
        }

        // force tabs as delimiterfor attribute tables unless there are definitely not
        // enough tabs in the file
        CTableDelimiterRules r = m_ImportedTableData->GetDelimiterRules();
        if (r.GetDelimiters().size() != 1 || r.GetDelimiters()[0] != '\t') {
            int rows = m_ImportedTableData->GetNumRows();

            // could tabs in first 100 rows. If << 1/row, don't set as separator.
            int num_tabs = 0;
            for (int i=0; i<std::min(rows,100); ++i) {
                const string& row = m_ImportedTableData->GetRow(i).GetValue();
                num_tabs += std::count(row.begin(), row.end(), '\t');
            }

            if (num_tabs >= std::min(rows,100)) {
                m_TabDelimCheck->SetValue(true);
                vector<char>  delims;
                delims.push_back('\t');
                m_ImportedTableData->GetDelimiterRules().SetDelimiters(delims);                
            }
            else {
                // This prevents us from skipping the delimiters panel in this case - since the tab
                // does not work as a separator.
                m_FileUpdated = false;
            }
        }
        
        if (m_FileUpdated)
            m_ImportedTableData->GetDelimiterRules().SetMergeDelimiters(false);

        m_ImportedTableData->SetHeaderAndFirstRow(0, 1);
        m_ImportedTableData->RecomputeFields(true);

        // Was it really a header? Get names and check. Ignore first column
        // since it is used as a rowcounter.
        vector<string> field_names;
        for (size_t col=1; col<m_ImportedTableData->GetColumns().size(); ++col) {
            field_names.push_back(m_ImportedTableData->GetColumn(col).GetName());
        }
            
        bool has_header = CFieldChoicePanel::LooksLikeHeader(field_names);
        
        // We have already set the header.  Remove it if needed:
        if (!has_header) {
            m_ImportedTableData->SetHeaderAndFirstRow(-1, 0);
            m_ImportedTableData->RecomputeFields(true);
            m_ParseColumnNamesBtn->SetValue(false);
        }


        m_DelimiterListCtrl->SetDataSource(m_ImportedTableData);
        m_DelimiterListCtrl->SetViewType(CTableImportListCtrl::eMultiColumn);

        UpdateDelimiters();

        Refresh();
    }
    catch(const CException&) {  
        wxString msg = wxT("Unable to open file: ") + s + wxT(" for reading");
        wxMessageBox(msg, wxT("Error Opening File"),             
            wxOK | wxICON_ERROR, this);
        return false;
    }
    catch (const exception&) 
    {
        wxString msg = wxT("Unable to open file: ") + s + wxT(" for reading");
        wxMessageBox(msg, wxT("Error Opening File"),             
                     wxOK | wxICON_ERROR, this);
        return false;
    }    
    return true;
}



/*!
 * Should we show tooltips?
 */

bool CAttribTableDelimitersPanel::ShowToolTips()
{
    return true;
}

/*!
 * Get bitmap resources
 */

wxBitmap CAttribTableDelimitersPanel::GetBitmapResource( const wxString& name )
{
    // Bitmap retrieval
////@begin CAttribTableDelimitersPanel bitmap retrieval
    wxUnusedVar(name);
    return wxNullBitmap;
////@end CAttribTableDelimitersPanel bitmap retrieval
}

/*!
 * Get icon resources
 */

wxIcon CAttribTableDelimitersPanel::GetIconResource( const wxString& name )
{
    // Icon retrieval
////@begin CAttribTableDelimitersPanel icon retrieval
    wxUnusedVar(name);
    return wxNullIcon;
////@end CAttribTableDelimitersPanel icon retrieval
}

void CAttribTableDelimitersPanel::OnTabDelimCheckClick( wxCommandEvent& event )
{
    UpdateDelimiters(); 
}

void CAttribTableDelimitersPanel::OnSemiColonDelimCheckClick( wxCommandEvent& event )
{
    UpdateDelimiters(); 
}

void CAttribTableDelimitersPanel::OnCommaDelimCheckClick( wxCommandEvent& event )
{
    UpdateDelimiters(); 
}

void CAttribTableDelimitersPanel::OnSpaceDelimCheckClick( wxCommandEvent& event )
{
    UpdateDelimiters(); 
}

void CAttribTableDelimitersPanel::OnOtherDelimCheckClick( wxCommandEvent& event )
{
    UpdateDelimiters(); 
}

void CAttribTableDelimitersPanel::OnTableDelimiterTextCtrlTextUpdated( wxCommandEvent& event )
{
    UpdateDelimiters(); 
}

void CAttribTableDelimitersPanel::OnAdjacentDelimitersCheckClick( wxCommandEvent& event )
{
    UpdateDelimiters(); 
}

void CAttribTableDelimitersPanel::OnTextQualifierSelected( wxCommandEvent& event )
{
    UpdateDelimiters(); 
}

void CAttribTableDelimitersPanel::OnMultSpanceOnlyCheckClick( wxCommandEvent& event )
{
    UpdateDelimiters(); 
}

void CAttribTableDelimitersPanel::OnParseColumnNamesBtnClick( wxCommandEvent& event )
{        
    if (m_ParseColumnNamesBtn->GetValue()) {        
        m_ImportedTableData->SetHeaderAndFirstRow(0, 1);        
    }
    else {
        m_ImportedTableData->SetHeaderAndFirstRow(-1, 0);       
    } 

    // This can change the current row, so clear the row-edit info
    m_EditRowIdx = -1;
    m_EditRow = "";
    m_DelimRowText->SetValue("");

    // reset data source which changes the records in the display to include/
    // not include the header row
    m_DelimiterListCtrl->SetDataSource(m_ImportedTableData);
    m_DelimiterListCtrl->Refresh();
}

void CAttribTableDelimitersPanel::OnCtableImportListctrl2ColDragging( wxListEvent& event )
{
    // Update the whole control when dragging.  This is needed because there
    // may be header rows where distrubtion across columns is based on width
    // of each column (basically they are left-justified into row irrespective
    // of column boundaries)
    m_DelimiterListCtrl->Refresh();
}

void CAttribTableDelimitersPanel::OnCTableImportListctrl2Selected( wxListEvent& event )
{
    int row_idx = event.GetIndex();
    row_idx += m_ImportedTableData->GetFirstImportRow();

    if (row_idx != m_EditRowIdx) {
        m_EditRowIdx = row_idx;
        m_EditRow = m_ImportedTableData->GetRow(row_idx).GetValue();
    }

    // This calls OnDelimRowTextUpdated which is not needed here, but also not
    // a problem
    m_DelimRowText->SetValue(m_ImportedTableData->GetRow(row_idx).GetValue());
    // disable until there is a user-update to the row
    m_RevertRowUpdate->Enable(false);
    
    int count = m_DelimiterListCtrl->GetSelectedItemCount();

}

void CAttribTableDelimitersPanel::OnDelimRowTextUpdated( wxCommandEvent& event )
{
    if (m_EditRowIdx != -1) {
        m_ImportedTableData->GetRow(m_EditRowIdx).GetValue() = m_DelimRowText->GetValue();
        m_ImportedTableData->RecomputeRowFields(m_EditRowIdx);
        m_DelimiterListCtrl->Refresh();
        m_RevertRowUpdate->Enable(true);
    }
}


void CAttribTableDelimitersPanel::OnRevertUpdateButtonClick( wxCommandEvent& event )
{
    if (m_EditRowIdx != -1) {
        // this will cause OnDelimRowTextUpdated to be called, updating the listctrl
        m_DelimRowText->SetValue(m_EditRow);

        // disable until there is another update
        m_RevertRowUpdate->Enable(false);
    }
}

void CAttribTableDelimitersPanel::OnSplitColBtnClick( wxCommandEvent& event )
{    
    if (m_NoneMSBtn->GetValue()) {
        wxMessageBox("'None' is not a valid selection for splitting a column.", "Select another Character");
    }

    // This button should not be enabled unless only one column is selected
    int col = -1;
    for (size_t c=0; c<m_ImportedTableData->GetColumns().size(); ++c) {
        if (m_ImportedTableData->GetColumns()[c].GetIsCurrent()) {
            if (col != -1)
                _TRACE("Error - more than one column selected for split operation");
            else
                col = (int)c;
        }
    }
    
    bool duplicate = false;
    char ch;
    bool char_selected = x_MergeSplitCharSelected(ch, duplicate);
    bool split_on_whitespace = m_WhitespaceMSBtn->GetValue();

    if (col == 0) {
        wxMessageBox("The first column and can't be split", "Select Different Column");
        return;
    }

    if (duplicate) {
        wxMessageBox("The character to split the column must not be one of the characters for splitting the table", "Select Unique Character");
        return;
    }
    if (!char_selected && !split_on_whitespace) {
        wxMessageBox("Please select a character on which to split the column", "Select Character First");
        return;
    }

    // Do actual split       
    m_ImportedTableData->SplitColumn(col, ch, split_on_whitespace);
    // This recomputes (and resizes) columns
    m_DelimiterListCtrl->SetDataSource(m_ImportedTableData);

    // Move focus to listctrl before disabling current button.
    // If we disable the control we are on, we will jump to the 
    // next control (tab radio button) AND due to a 2.9.4 wxWidgets
    // bug we will select that control (not cool, wx)
    m_DelimiterListCtrl->SetFocus();

    m_SplitColBtn->Enable(false);
    m_MergeColsBtn->Enable(false);

    // set all columns to un-selected
    for (size_t c=0; c<m_ImportedTableData->GetColumns().size(); ++c) {
        if (m_ImportedTableData->GetColumns()[c].GetIsCurrent()) {
            m_ImportedTableData->GetColumns()[c].SetIsCurrent(false);
            wxListItem item;
            m_DelimiterListCtrl->GetColumn(c, item);
            m_ImportedTableData->GetColumns()[c].SetIsCurrent(false);

            // Reset the image from the (previous selected) column:
            int img_idx = m_DelimiterListCtrl->GetColumnImageID(c, true);
            item.SetImage(img_idx);
            m_DelimiterListCtrl->SetColumn(c, item);
        }
    }
}

void CAttribTableDelimitersPanel::OnMergeColsBtnClick( wxCommandEvent& event )
{
    if (m_WhitespaceMSBtn->GetValue()) {
        wxMessageBox("'None' is not a valid selection for splitting a column.", "Select another Character");
    }

    // This button should not be enabled unless only one column is selected
    int col1 = -1;
    int col2 = -1;
    for (size_t c=0; c<m_ImportedTableData->GetColumns().size(); ++c) {
        if (m_ImportedTableData->GetColumns()[c].GetIsCurrent()) {
            if (col1 == -1)
                col1 = c;
            else
                col2 = c;                
        }
    }

    vector<size_t> merge_cols;
    for (size_t c=0; c<m_ImportedTableData->GetColumns().size(); ++c) {
        if (m_ImportedTableData->GetColumns()[c].GetIsCurrent()) {
            merge_cols.push_back(c);
        }
    }
    
    bool duplicate = false;
    char ch;
    bool char_selected = x_MergeSplitCharSelected(ch, duplicate);
    bool merge_char_is_none = m_NoneMSBtn->GetValue();

    // Should not happen - column selection enables the button
    if (col2 == -1) {
        wxMessageBox("First select a range of columns to merge", "Select Columns");
        return;
    }

    if (col1 == 0) {
        wxMessageBox("The first column and can't be merged", "Select Different Column");
        return;
    }

    if (duplicate) {
        wxMessageBox("The character to merge columns must not be one of the characters for splitting the table", "Select Unique Character");
        return;
    }
    if (!char_selected && !merge_char_is_none) {
        wxMessageBox("Please select a character to replace delimiters when merging the columns", "Select Character First");
        return;
    }

    // Do actual merge
    //m_ImportedTableData->MergeColumns(col1, col2, ch, merge_char_is_none);
    m_ImportedTableData->MergeColumns(merge_cols, ch, merge_char_is_none);
    
    // This recomputes (and resizes) columns
    m_DelimiterListCtrl->SetDataSource(m_ImportedTableData);

    // Move focus to listctrl before disabling current button.
    // If we disable the control we are on, we will jump to the 
    // next control (tab radio button) AND due to a 2.9.4 wxWidgets
    // bug we will select that control (not cool, wx)
    m_DelimiterListCtrl->SetFocus();

    // disable buttons
    m_MergeColsBtn->Enable(false);
    m_SplitColBtn->Enable(false);

    // set all columns to un-selected
    for (size_t c=0; c<m_ImportedTableData->GetColumns().size(); ++c) {
        if (m_ImportedTableData->GetColumns()[c].GetIsCurrent()) {
            m_ImportedTableData->GetColumns()[c].SetIsCurrent(false);
            wxListItem item;
            m_DelimiterListCtrl->GetColumn(c, item);
            m_ImportedTableData->GetColumns()[c].SetIsCurrent(false);

            // Reset the image from the (previous selected) column:
            int img_idx = m_DelimiterListCtrl->GetColumnImageID(c, true);
            item.SetImage(img_idx);
            m_DelimiterListCtrl->SetColumn(c, item);
        }
    }
}

void CAttribTableDelimitersPanel::OnListctrlColLeftClick( wxListEvent& event )
{
    int col = event.GetColumn();

    m_SplitColBtn->Enable(false);
    m_MergeColsBtn->Enable(false);

    m_WhitespaceMSBtn->Enable(true);
    m_NoneMSBtn->Enable(true);

    // If shift is not down remove any previous selection
    if (!wxGetKeyState(WXK_SHIFT) && !wxGetKeyState(WXK_CONTROL)) {
        for (size_t c=0; c<m_ImportedTableData->GetColumns().size(); ++c) {
            if (m_ImportedTableData->GetColumns()[c].GetIsCurrent()) {
                m_ImportedTableData->GetColumns()[c].SetIsCurrent(false);
                
                wxListItem item;
                m_DelimiterListCtrl->GetColumn(c, item);                

                // Reset the image from the (previous selected) column:
                int img_idx = m_DelimiterListCtrl->GetColumnImageID(c, true);
                item.SetImage(img_idx);
                m_DelimiterListCtrl->SetColumn(c, item);
            }
        }
    }

    // See if a column is already selected
    int col1 = -1;    
    int sel_count = 0;
    for (size_t c=0; c<m_ImportedTableData->GetColumns().size(); ++c) {
        if (m_ImportedTableData->GetColumns()[c].GetIsCurrent()) {
            if (col1==-1)
                col1 = c;
            sel_count++;
        }
    }
    // Need to select a second for a merge to be valid
    if (!wxGetKeyState(WXK_SHIFT) && !wxGetKeyState(WXK_CONTROL)) {        
        m_ImportedTableData->GetColumns()[col].SetIsCurrent(true);

        // Put the icon in the column to show it is current:
        wxListItem colinfo;
        m_DelimiterListCtrl->GetColumn(col, colinfo);                      
        colinfo.SetText(m_ImportedTableData->GetColumns()[col].GetName());
        int img_idx = m_DelimiterListCtrl->GetColumnImageID(col, true);
        colinfo.SetImage(img_idx);
        m_DelimiterListCtrl->SetColumn(col, colinfo);
        ++sel_count;
    }
    // ctrl select and at least 1 column already selected
    else if (wxGetKeyState(WXK_CONTROL)) {
        // If selected column already selected, unselect
        if (m_ImportedTableData->GetColumns()[col].GetIsCurrent()) {
            m_ImportedTableData->GetColumns()[col].SetIsCurrent(false);
            wxListItem item;
            m_DelimiterListCtrl->GetColumn(col, item);
            m_ImportedTableData->GetColumns()[col].SetIsCurrent(false);

            // Reset the image from the (previous selected) column:
            int img_idx = m_DelimiterListCtrl->GetColumnImageID(col, true);
            item.SetImage(img_idx);
            m_DelimiterListCtrl->SetColumn(col, item);
            --sel_count;
        }
        // If selected column not currently selected, select it
        else {
            m_ImportedTableData->GetColumns()[col].SetIsCurrent(true);

            // Put the icon in the column to show it is current:
            wxListItem colinfo;
            m_DelimiterListCtrl->GetColumn(col, colinfo);                      
            colinfo.SetText(m_ImportedTableData->GetColumns()[col].GetName());
            int img_idx = m_DelimiterListCtrl->GetColumnImageID(col, true);
            colinfo.SetImage(img_idx);
            m_DelimiterListCtrl->SetColumn(col, colinfo);
            ++sel_count;
        }
    }
    // shift select and column is not already selected
    else if (wxGetKeyState(WXK_SHIFT) &&
             !m_ImportedTableData->GetColumns()[col].GetIsCurrent()) {
            
            // If there was not a current column previously, this will just select
            // column just clicked on.
            if (col1 == -1)
                col1 = col;
            int from_col = std::min(col1, col);
            int to_col = std::max(col1, col);

            for (int i=from_col; i<=to_col; ++i) {
                size_t col_idx = (size_t)i;
                if (!m_ImportedTableData->GetColumns()[col_idx].GetIsCurrent()) {
                    ++sel_count;
                    m_ImportedTableData->GetColumns()[col_idx].SetIsCurrent(true);

                    // Put the icon in the column to show it is current:
                    wxListItem colinfo;
                    m_DelimiterListCtrl->GetColumn(col_idx, colinfo);                      
                    colinfo.SetText(m_ImportedTableData->GetColumns()[col_idx].GetName());
                    int img_idx = m_DelimiterListCtrl->GetColumnImageID(col_idx, true);
                    colinfo.SetImage(img_idx);
                    m_DelimiterListCtrl->SetColumn(col_idx, colinfo);
                }
            }
    }

    if (sel_count == 1) {
        m_SplitColBtn->Enable(true);
        // None is not a valid delimeter for a split, so disable and
        // it if is checked, pick a different value by default       
        if (m_NoneMSBtn->GetValue()) {
            m_TabMSBtn->SetValue(true);
        }
        m_NoneMSBtn->Enable(false);
    }
    else if (sel_count > 1) {
        m_MergeColsBtn->Enable(true);

        // Whitespace is not valid for merging
        if (m_WhitespaceMSBtn->GetValue()) {
            m_TabMSBtn->SetValue(true);          
        }
        m_WhitespaceMSBtn->Enable(false);
    }


}

void CAttribTableDelimitersPanel::OnTextMSDelimiterTextUpdated( wxCommandEvent& event )
{
    // If user enters data in 'other' they must mean to use it.
    if (!m_OtherMSBtn->GetValue() && m_DelimMSText->GetValue().length()>0)
        m_OtherMSBtn->SetValue(true);
}

void CAttribTableDelimitersPanel::x_EnableMergeSplitWidgets(bool b)
{
    // If table delimiter changes, we disable these until (new) columns
    // are selected
    m_SplitColBtn->Enable(false);
    m_MergeColsBtn->Enable(false);

    m_MergeSplitStaticText->Enable(b);
    m_TabMSBtn->Enable(b);
    m_SpaceMSBtn->Enable(b);
    m_WhitespaceMSBtn->Enable(b);
    m_ColonMSBtn->Enable(b);
    m_SemicolonMSBtn->Enable(b);
    m_CommaMSBtn->Enable(b);
    m_NoneMSBtn->Enable(b);
    m_OtherMSBtn->Enable(b);
    m_DelimMSText->Enable(b);
}

bool CAttribTableDelimitersPanel::x_MergeSplitCharSelected(char& ch, bool& duplicate)
{
    bool char_selected = false;
    duplicate = false;

    vector<char> delims = m_ImportedTableData->GetDelimiters();

    if (m_TabMSBtn->GetValue()) {
        if (std::find(delims.begin(), delims.end(), '\t') == delims.end()) {
            ch = '\t';
            char_selected = true;
        }
        else {
            duplicate = true;
        }
    }
    else if (m_SpaceMSBtn->GetValue()) {
        if (std::find(delims.begin(), delims.end(), ' ') == delims.end()) {
            ch = ' ';
            char_selected = true;
        }
        else {
            duplicate = true;
        }
    }
    else if (m_ColonMSBtn->GetValue()) {
        if (std::find(delims.begin(), delims.end(), ':') == delims.end()) {
            ch = ':';
            char_selected = true;
        }
        else {
            duplicate = true;
        }
    }
    else if (m_SemicolonMSBtn->GetValue()) {
        if (std::find(delims.begin(), delims.end(), ';') == delims.end()) {
            ch = ';';
            char_selected = true;
        }
        else {
            duplicate = true;
        }
    }
    else if (m_CommaMSBtn->GetValue()) {
        if (std::find(delims.begin(), delims.end(), ',') == delims.end()) {
            ch = ',';
            char_selected = true;
        }
        else {
            duplicate = true;
        }
    }
    else if (m_OtherMSBtn->GetValue()) {   
        wxString txt = m_DelimMSText->GetValue();

        // length could also be 0 in which case char_selected will remain false
        if (txt.length() == 1) {
            ch = txt[0];
            if (std::find(delims.begin(), delims.end(), ch) == delims.end()) {                  
                char_selected = true;
            }
            else {
                duplicate = true;
            }
        }   
    }

    return char_selected;
}

void CAttribTableDelimitersPanel::OnExportBtnClick( wxCommandEvent& event )
{
    static wxString filePath;
    wxFileDialog dlg(this, _("Select a file"), _(""), filePath,
                     _("All files (*.*)|*.*"),
                     wxFD_SAVE|wxFD_OVERWRITE_PROMPT);

    if (dlg.ShowModal() != wxID_OK)
        return;

    filePath = dlg.GetPath();

    try {
        CNcbiOfstream ofile(filePath.c_str().AsChar());
        if( !ofile ){
            wxMessageBox( "Error opening selected file: " + dlg.GetPath(), "File Error" );
            return;
        }

        m_ImportedTableData->SaveTable(ofile);


    } catch( CException& ex ){
        wxMessageBox( ToWxString( ex.GetMsg() ), "Error while saving table" );
        return;
    }
}

END_NCBI_SCOPE 


