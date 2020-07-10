/*  $Id: attrib_table_column_id_panel.cpp 45101 2020-05-29 20:53:24Z asztalos $
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
#include <objects/seq/Annot_descr.hpp>
#include <objtools/edit/mail_report.hpp>
#include <gui/widgets/edit/feature_field_name_panel.hpp>

#include <gui/widgets/edit/field_type_constants.hpp>
#include <gui/widgets/edit/struct_comm_field_panel.hpp>
#include <gui/widgets/edit/struct_fieldvalue_panel.hpp>
#include <gui/widgets/edit/generic_report_dlg.hpp>
#include <gui/widgets/wx/async_call.hpp>
#include <gui/widgets/wx/sys_path.hpp>
#include <gui/widgets/loaders/table_annot_data_source.hpp>
#include <gui/widgets/loaders/table_object_loader.hpp>
#include <gui/widgets/wx/wnd_layout_registry.hpp>
#include <gui/utils/execute_unit.hpp>
#include <gui/utils/app_popup.hpp>
#include <gui/utils/event_handler.hpp>
#include <gui/objutils/util_cmds.hpp>

#include <gui/packages/pkg_sequence_edit/field_choice_panel.hpp>
#include <gui/packages/pkg_sequence_edit/miscedit_util.hpp>
#include <gui/packages/pkg_sequence_edit/table_reader.hpp>
#include <gui/packages/pkg_sequence_edit/featedit_util.hpp>
#include <gui/packages/pkg_sequence_edit/constraint_panel.hpp>
#include <gui/packages/pkg_sequence_edit/editing_actions.hpp>
#include <gui/packages/pkg_sequence_edit/editing_action_constraint.hpp>
#include "attrib_table_column_id_panel.hpp"

#include <wx/dialog.h>
#include <wx/imaglist.h>
#include <wx/richtext/richtextctrl.h>


BEGIN_NCBI_SCOPE

/*!
 * CAttribTableColumnIdPanel type definition
 */

IMPLEMENT_DYNAMIC_CLASS( CAttribTableColumnIdPanel, wxPanel )


/*!
 * CAttribTableColumnIdPanel event table definition
 */

BEGIN_EVENT_TABLE( CAttribTableColumnIdPanel, wxPanel )

////@begin CAttribTableColumnIdPanel event table entries
    EVT_TEXT( ID_COLUMNNAMETXT, CAttribTableColumnIdPanel::OnColumnNameTxtTextUpdated )

    EVT_CHECKBOX( ID_SKIPBTN, CAttribTableColumnIdPanel::OnSkipBtnSelected )

    EVT_CHECKBOX( ID_MATCH_COLUMN, CAttribTableColumnIdPanel::OnMatchColumnClick )

    EVT_RADIOBOX( ID_MATCHTYPE, CAttribTableColumnIdPanel::OnMatchTypeSelected )

    EVT_RADIOBUTTON( ID_REPLACE_BTN, CAttribTableColumnIdPanel::OnReplaceBtnSelected )

    EVT_RADIOBUTTON( ID_APPEND_BTN, CAttribTableColumnIdPanel::OnAppendBtnSelected )

    EVT_RADIOBUTTON( ID_PREFIX_BTN, CAttribTableColumnIdPanel::OnPrefixBtnSelected )

    EVT_RADIOBUTTON( ID_IGNORE_BTN, CAttribTableColumnIdPanel::OnIgnoreBtnSelected )

    EVT_RADIOBUTTON( ID_ADD_QUAL_BTN, CAttribTableColumnIdPanel::OnAddQualBtnSelected )

    EVT_RADIOBUTTON( ID_SEMICOLON_BTN, CAttribTableColumnIdPanel::OnSemicolonBtnSelected )

    EVT_RADIOBUTTON( ID_SPACE_BTN, CAttribTableColumnIdPanel::OnSpaceBtnSelected )

    EVT_RADIOBUTTON( ID_COLON_BTN, CAttribTableColumnIdPanel::OnColonBtnSelected )

    EVT_RADIOBUTTON( ID_COMMA_BTN, CAttribTableColumnIdPanel::OnCommaBtnSelected )

    EVT_RADIOBUTTON( ID_NODELIMITER_BTN, CAttribTableColumnIdPanel::OnNodelimiterBtnSelected )

    EVT_RADIOBUTTON( ID_IGNORE_BLANKS_BTN, CAttribTableColumnIdPanel::OnIgnoreBlanksBtnSelected )

    EVT_RADIOBUTTON( ID_ERASE_EXISTING_VALUE_BTN, CAttribTableColumnIdPanel::OnEraseExistingValueBtnSelected )

    EVT_LIST_COL_CLICK( ID_CTABLEIMPORTLISTCTRL3, CAttribTableColumnIdPanel::OnCtableImportListctrlColLeftClick )
    
    EVT_LIST_COL_DRAGGING( ID_CTABLEIMPORTLISTCTRL3, CAttribTableColumnIdPanel::OnCtableImportListCtrlColDragging )

    EVT_BUTTON( ID_SPLIT_FIRST_COL, CAttribTableColumnIdPanel::OnSplitFirstColClick )

    EVT_BUTTON( ID_MULTISPACE_TO_TAB_BTN, CAttribTableColumnIdPanel::OnMultispaceToTabBtnClick )

    EVT_BUTTON( ID_MERGEIDCOLS, CAttribTableColumnIdPanel::OnMergeIDColsClick )

    EVT_CHECKBOX( ID_MAIL_REPORT_CHECKBOX, CAttribTableColumnIdPanel::OnMailReportCheckboxClick )

    EVT_BUTTON( ID_APPLY_NO_CLOSE, CAttribTableColumnIdPanel::OnApplyNoCloseClick )

    EVT_UPDATE_FEATURE_LIST(wxID_ANY, CAttribTableColumnIdPanel::ProcessUpdateFeatEvent )
    
    EVT_COMMAND(wxID_ANY, wxEVT_COMMAND_UPDATE_STRUCTCOMMENT_FIELD, CAttribTableColumnIdPanel::ProcessUpdateStructCommentEvent)

    EVT_CHOICEBOOK_PAGE_CHANGED(wxID_ANY, CAttribTableColumnIdPanel::OnPageChanged)
////@end CAttribTableColumnIdPanel event table entries
END_EVENT_TABLE()


const string kSequenceIdAbbrevColLabel = "Seq-ID";

/*!
 * CAttribTableColumnIdPanel constructors
 */

CAttribTableColumnIdPanel::CAttribTableColumnIdPanel()
{
    Init();
}

CAttribTableColumnIdPanel::CAttribTableColumnIdPanel( wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style )
{
    Init();
    Create(parent, id, pos, size, style);
}


/*!
 * CAttribTableColumnIdPanel creator
 */

bool CAttribTableColumnIdPanel::Create( wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style )
{
////@begin CAttribTableColumnIdPanel creation
    SetExtraStyle(wxWS_EX_BLOCK_EVENTS);
    wxPanel::Create( parent, id, pos, size, style );

    CreateControls();
    if (GetSizer())
    {
        GetSizer()->SetSizeHints(this);
    }
    Centre();
////@end CAttribTableColumnIdPanel creation
    return true;
}


/*!
 * CAttribTableColumnIdPanel destructor
 */

CAttribTableColumnIdPanel::~CAttribTableColumnIdPanel()
{
////@begin CAttribTableColumnIdPanel destruction
////@end CAttribTableColumnIdPanel destruction
}


/*!
 * Member initialisation
 */

void CAttribTableColumnIdPanel::Init()
{
////@begin CAttribTableColumnIdPanel member initialisation
    m_ColumnPropertiesSizer = NULL;
    m_ColumnNameStaticTxt = NULL;
    m_ColumnNameTxtCtrl = NULL;
    m_SkipFormatBtn = NULL;
    m_MatchColumn = NULL;
    m_MatchType = NULL;
    m_Replace = NULL;
    m_Append = NULL;
    m_Prefix = NULL;
    m_LeaveOld = NULL;
    m_AddQual = NULL;
    m_Semicolon = NULL;
    m_Space = NULL;
    m_Colon = NULL;
    m_Comma = NULL;
    m_NoDelimiter = NULL;
    m_IgnoreBlanks = NULL;
    m_EraseValue = NULL;
    m_ColumnIdList = NULL;
    m_MergeIDCols = NULL;
    m_MakeMailReport = NULL;

    m_Fields = NULL;
    m_FieldsSize.Set(0,0);
    m_CurrentColumnIdx = -1;
    m_Constraint = NULL;
////@end CAttribTableColumnIdPanel member initialisation
}


/*!
 * Control creation for CAttribTableColumnIdPanel
 */

void CAttribTableColumnIdPanel::CreateControls()
{
    ////@begin CAttribTableColumnIdPanel content construction
    CAttribTableColumnIdPanel* itemPanel1 = this;

    wxBoxSizer* itemBoxSizer2 = new wxBoxSizer(wxVERTICAL);
    itemPanel1->SetSizer(itemBoxSizer2);

    wxStaticText* itemStaticText3 = new wxStaticText(itemPanel1, wxID_STATIC, _("Set or Verify Type Information for each Table Column"), wxDefaultPosition, wxDefaultSize, 0);
    itemStaticText3->SetFont(wxFont(10, wxFONTFAMILY_SWISS, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL, false, wxT("Tahoma")));
    itemBoxSizer2->Add(itemStaticText3, 0, wxALIGN_CENTER_HORIZONTAL | wxLEFT | wxRIGHT | wxTOP, 5);

    m_ColumnPropertiesSizer = new wxStaticBox(itemPanel1, wxID_ANY, _("Column Properties"));
    wxStaticBoxSizer* itemStaticBoxSizer5 = new wxStaticBoxSizer(m_ColumnPropertiesSizer, wxVERTICAL);
    itemBoxSizer2->Add(itemStaticBoxSizer5, 1, wxEXPAND | wxLEFT |wxRIGHT |wxBOTTOM, 5);

    wxBoxSizer* itemBoxSizer7 = new wxBoxSizer(wxHORIZONTAL);
    itemStaticBoxSizer5->Add(itemBoxSizer7, 0, wxEXPAND | wxALL, 0);

    m_ColumnNameStaticTxt = new wxStaticText(itemStaticBoxSizer5->GetStaticBox(), wxID_STATIC, _("Name:"), wxDefaultPosition, wxDefaultSize, 0);
    m_ColumnNameStaticTxt->Enable(false);
    itemBoxSizer7->Add(m_ColumnNameStaticTxt, 0, wxALIGN_CENTER_VERTICAL | wxALL, 5);

    m_ColumnNameTxtCtrl = new wxTextCtrl(itemStaticBoxSizer5->GetStaticBox(), ID_COLUMNNAMETXT, wxEmptyString, wxDefaultPosition, wxSize(200, -1), 0);
    m_ColumnNameTxtCtrl->Enable(false);
    itemBoxSizer7->Add(m_ColumnNameTxtCtrl, 1, wxALIGN_CENTER_VERTICAL| wxALL, 5);

    m_SkipFormatBtn = new wxCheckBox(itemStaticBoxSizer5->GetStaticBox(), ID_SKIPBTN, _("Skip Column"), wxDefaultPosition, wxDefaultSize, 0);
    m_SkipFormatBtn->SetValue(false);
    m_SkipFormatBtn->Enable(false);
    itemBoxSizer7->Add(m_SkipFormatBtn, 0,  wxALIGN_CENTER_VERTICAL| wxALL, 5);

    m_MatchColumn = new wxCheckBox(itemStaticBoxSizer5->GetStaticBox(), ID_MATCH_COLUMN, _("Match Column"), wxDefaultPosition, wxDefaultSize, 0);
    m_MatchColumn->Enable(false);
    itemBoxSizer7->Add(m_MatchColumn, 0, wxALIGN_CENTER_VERTICAL | wxTOP|wxBOTTOM|wxRIGHT, 5);

    wxArrayString m_MatchTypeStrings;
    m_MatchTypeStrings.Add(_("&SeqID"));
    m_MatchTypeStrings.Add(_("&Taxname"));
    m_MatchTypeStrings.Add(_("&Other"));
    m_MatchType = new wxRadioBox(itemStaticBoxSizer5->GetStaticBox(), ID_MATCHTYPE, wxEmptyString, wxDefaultPosition, wxDefaultSize, m_MatchTypeStrings, 1, wxRA_SPECIFY_ROWS);
    m_MatchType->SetSelection(0);
    itemBoxSizer7->Add(m_MatchType, 0, wxALIGN_CENTER_VERTICAL | wxRIGHT, 5);

    wxStaticBox* qual_box = new wxStaticBox(itemStaticBoxSizer5->GetStaticBox(), wxID_ANY, wxEmptyString);
    wxStaticBoxSizer* itemStaticBoxSizer14 = new wxStaticBoxSizer(qual_box, wxHORIZONTAL);
    itemStaticBoxSizer5->Add(itemStaticBoxSizer14, 1, wxEXPAND | wxLEFT|wxRIGHT|wxBOTTOM, 5);

    vector<CFieldNamePanel::EFieldType> field_types;
    field_types.push_back(CFieldNamePanel::eFieldType_Taxname);
    field_types.push_back(CFieldNamePanel::eFieldType_Source);
    field_types.push_back(CFieldNamePanel::eFieldType_Feature);
    field_types.push_back(CFieldNamePanel::eFieldType_CDSGeneProt);
    field_types.push_back(CFieldNamePanel::eFieldType_RNA);
    field_types.push_back(CFieldNamePanel::eFieldType_MolInfo);
    field_types.push_back(CFieldNamePanel::eFieldType_Misc);
    field_types.push_back(CFieldNamePanel::eFieldType_Pub);
    field_types.push_back(CFieldNamePanel::eFieldType_StructuredComment);
    field_types.push_back(CFieldNamePanel::eFieldType_DBLink);

    m_Fields = new CFieldChoicePanel(itemStaticBoxSizer14->GetStaticBox(), field_types, false, false, wxID_ANY, wxDefaultPosition, wxDefaultSize, 0);
    itemStaticBoxSizer14->Add(m_Fields, 0, wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL | wxTOP, 2);
    m_Fields->SetShowLegalOnly(true);

    wxBoxSizer* itemBoxSizerRules = new wxBoxSizer(wxVERTICAL);
    itemStaticBoxSizer14->Add(itemBoxSizerRules, 0, wxALIGN_LEFT | wxALL, 0);

    wxStaticBox* itemStaticBoxSizer17Static = new wxStaticBox(itemStaticBoxSizer14->GetStaticBox(), wxID_ANY, _("Text Update Rules"));
    wxStaticBoxSizer* itemStaticBoxSizer17 = new wxStaticBoxSizer(itemStaticBoxSizer17Static, wxVERTICAL);
    itemBoxSizerRules->Add(itemStaticBoxSizer17, 1, wxEXPAND | wxTOP| wxRIGHT, 2);

    wxBoxSizer* itemBoxSizer20 = new wxBoxSizer(wxHORIZONTAL);
    itemStaticBoxSizer17->Add(itemBoxSizer20, 0, wxALIGN_LEFT | wxALL, 0);

    m_Replace = new wxRadioButton(itemStaticBoxSizer17->GetStaticBox(), ID_REPLACE_BTN, _("Replace"), wxDefaultPosition, wxDefaultSize, wxRB_GROUP);
    m_Replace->SetValue(true);
    itemBoxSizer20->Add(m_Replace, 0, wxALIGN_LEFT | wxALL, 2);

    m_Append = new wxRadioButton(itemStaticBoxSizer17->GetStaticBox(), ID_APPEND_BTN, _("Append"), wxDefaultPosition, wxDefaultSize, 0);
    m_Append->SetValue(false);
    itemBoxSizer20->Add(m_Append, 0, wxALIGN_CENTER_VERTICAL | wxALL, 2);

    m_Prefix = new wxRadioButton(itemStaticBoxSizer17->GetStaticBox(), ID_PREFIX_BTN, _("Prefix"), wxDefaultPosition, wxDefaultSize, 0);
    m_Prefix->SetValue(false);
    itemBoxSizer20->Add(m_Prefix, 0, wxALIGN_CENTER_VERTICAL | wxALL, 2);

    wxBoxSizer* itemBoxSizer21 = new wxBoxSizer(wxHORIZONTAL);
    itemStaticBoxSizer17->Add(itemBoxSizer21, 0, wxALIGN_LEFT | wxALL, 0);

    m_LeaveOld = new wxRadioButton(itemStaticBoxSizer17->GetStaticBox(), ID_IGNORE_BTN, _("Ignore new text"), wxDefaultPosition, wxDefaultSize, 0);
    m_LeaveOld->SetValue(false);
    itemBoxSizer21->Add(m_LeaveOld, 0, wxALIGN_LEFT | wxLEFT | wxALL, 2);

    m_AddQual = new wxRadioButton(itemStaticBoxSizer17->GetStaticBox(), ID_ADD_QUAL_BTN, _("Add new qual"), wxDefaultPosition, wxDefaultSize, 0);
    m_AddQual->SetValue(false);
    itemBoxSizer21->Add(m_AddQual, 0, wxALIGN_LEFT | wxLEFT | wxALL, 2);

    wxStaticBox* itemStaticBoxSizer18Static = new wxStaticBox(itemStaticBoxSizer14->GetStaticBox(), wxID_ANY, _("Separate New and Old Text with"));
    wxStaticBoxSizer* itemStaticBoxSizer18 = new wxStaticBoxSizer(itemStaticBoxSizer18Static, wxVERTICAL);
    itemBoxSizerRules->Add(itemStaticBoxSizer18, 1, wxEXPAND | wxTOP | wxRIGHT, 2);

    wxBoxSizer* itemBoxSizer27 = new wxBoxSizer(wxHORIZONTAL);
    itemStaticBoxSizer18->Add(itemBoxSizer27, 0, wxALIGN_LEFT|wxALL, 0);

    m_Semicolon = new wxRadioButton( itemStaticBoxSizer18->GetStaticBox(), ID_SEMICOLON_BTN, _("Semicolon"), wxDefaultPosition, wxDefaultSize, wxRB_GROUP );
    m_Semicolon->SetValue(true);
    itemBoxSizer27->Add(m_Semicolon, 0, wxALIGN_CENTER_VERTICAL|wxALL, 2);

    m_Space = new wxRadioButton( itemStaticBoxSizer18->GetStaticBox(), ID_SPACE_BTN, _("Space"), wxDefaultPosition, wxDefaultSize, 0 );
    m_Space->SetValue(false);
    itemBoxSizer27->Add(m_Space, 0, wxALIGN_CENTER_VERTICAL|wxALL, 2);

    m_Colon = new wxRadioButton( itemStaticBoxSizer18->GetStaticBox(), ID_COLON_BTN, _("Colon"), wxDefaultPosition, wxDefaultSize, 0 );
    m_Colon->SetValue(false);
    itemBoxSizer27->Add(m_Colon, 0, wxALIGN_CENTER_VERTICAL|wxALL, 2);

    m_Comma = new wxRadioButton( itemStaticBoxSizer18->GetStaticBox(), ID_COMMA_BTN, _("Comma"), wxDefaultPosition, wxDefaultSize, 0 );
    m_Comma->SetValue(false);
    itemBoxSizer27->Add(m_Comma, 0, wxALIGN_CENTER_VERTICAL|wxALL, 2);

    m_NoDelimiter = new wxRadioButton( itemStaticBoxSizer18->GetStaticBox(), ID_NODELIMITER_BTN, _("Do not separate"), wxDefaultPosition, wxDefaultSize, 0 );
    m_NoDelimiter->SetValue(false);
    itemStaticBoxSizer18->Add(m_NoDelimiter, 0, wxALIGN_LEFT|wxALL, 2);

    wxStaticBox* itemStaticBoxSizer19Static = new wxStaticBox(itemStaticBoxSizer14->GetStaticBox(), wxID_ANY, _("Treat Blanks in Column"));
    wxStaticBoxSizer* itemStaticBoxSizer19 = new wxStaticBoxSizer(itemStaticBoxSizer19Static, wxVERTICAL);
    itemBoxSizerRules->Add(itemStaticBoxSizer19, 1, wxEXPAND | wxTOP | wxRIGHT|wxBOTTOM, 2);

    m_IgnoreBlanks = new wxRadioButton( itemStaticBoxSizer19->GetStaticBox(), ID_IGNORE_BLANKS_BTN, _("Ignore Blanks"), wxDefaultPosition, wxDefaultSize, wxRB_GROUP );
    m_IgnoreBlanks->SetValue(true);
    itemStaticBoxSizer19->Add(m_IgnoreBlanks, 0, wxALIGN_LEFT|wxALL, 2);

    m_EraseValue = new wxRadioButton( itemStaticBoxSizer19->GetStaticBox(), ID_ERASE_EXISTING_VALUE_BTN, _("Erase Existing Value in Record"), wxDefaultPosition, wxDefaultSize, 0 );
    m_EraseValue->SetValue(false);
    itemStaticBoxSizer19->Add(m_EraseValue, 0, wxALIGN_LEFT|wxALL, 2);

    wxStaticBox* itemStaticBoxSizer38Static = new wxStaticBox(itemPanel1, wxID_ANY, _("Click on Table Columns to Set Properties"));
    wxStaticBoxSizer* itemStaticBoxSizer38 = new wxStaticBoxSizer(itemStaticBoxSizer38Static, wxHORIZONTAL);
    itemBoxSizer2->Add(itemStaticBoxSizer38, 1, wxGROW|wxLEFT|wxRIGHT, 5);

    m_ColumnIdList = new CTableImportListCtrl(itemStaticBoxSizer38->GetStaticBox(), ID_CTABLEIMPORTLISTCTRL3, wxDefaultPosition, wxSize(-1, 300), wxLC_REPORT|wxLC_VIRTUAL|wxLC_ALIGN_LEFT|wxLC_HRULES|wxLC_VRULES );
    itemStaticBoxSizer38->Add(m_ColumnIdList, 1, wxGROW|wxALL, 5);

    m_Constraint = new CConstraintPanel( itemPanel1, m_TopSeqEntry);
    itemBoxSizer2->Add(m_Constraint, 0, wxEXPAND|wxALL, 5);

    wxBoxSizer* itemBoxSizer42 = new wxBoxSizer(wxHORIZONTAL);
    itemBoxSizer2->Add(itemBoxSizer42, 0, wxALIGN_LEFT|wxALL, 0);

    wxButton* itemButton43 = new wxButton( itemPanel1, ID_SPLIT_FIRST_COL, _("Split First Col."), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer42->Add(itemButton43, 0, wxALIGN_LEFT| wxALIGN_CENTER_VERTICAL | wxLEFT | wxRIGHT|wxTOP, 5);

    wxButton* itemButton44 = new wxButton( itemPanel1, ID_MULTISPACE_TO_TAB_BTN, _("Multispaces to Tabs"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer42->Add(itemButton44, 0, wxALIGN_CENTER|wxLEFT |wxRIGHT| wxTOP, 5);

    m_MergeIDCols = new wxButton( itemPanel1, ID_MERGEIDCOLS, _("Merge Cols For ID"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer42->Add(m_MergeIDCols, 0, wxALIGN_CENTER| wxLEFT | wxRIGHT | wxTOP, 5);

    m_MakeMailReport = new wxCheckBox( itemPanel1, ID_MAIL_REPORT_CHECKBOX, _("Make Mail Report"), wxDefaultPosition, wxDefaultSize, 0 );
    m_MakeMailReport->SetValue(false);
    itemBoxSizer42->Add(m_MakeMailReport, 0, wxALIGN_CENTER| wxLEFT | wxRIGHT | wxTOP, 5);

    wxButton* itemButton47 = new wxButton( itemPanel1, ID_APPLY_NO_CLOSE, _("Apply (No Close)"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer42->Add(itemButton47, 0, wxALIGN_CENTER| wxLEFT | wxRIGHT | wxTOP, 5);

////@end CAttribTableColumnIdPanel content construction
}

void CAttribTableColumnIdPanel::SetTopLevelEntry(CSeq_entry_Handle seq_entry) 
{ 
    wxBusyCursor wait;
    m_TopSeqEntry = seq_entry; 
    if (m_Constraint)
        m_Constraint->ListPresentFeaturesFirst(seq_entry);
    m_id_guesser = make_shared<edit::CSeqIdGuesser>(m_TopSeqEntry);
}

bool CAttribTableColumnIdPanel::Show(bool show)
{    
    if (show) {
        x_EnableColumnEditControls(false);

        // Get initial size of fields and when it changes, update
        // the dialog size accordingly
        m_FieldsSize = m_Fields->GetSize();

        x_GuessQualifiers();
    }    

    return wxPanel::Show(show);
}

void CAttribTableColumnIdPanel::x_GuessQualifiers()
{
    if (!m_ImportedTableData.IsNull()) {
        m_ColumnIdList->SetViewType(CTableImportListCtrl::eMultiColumn);
        m_ColumnIdList->SetDataSource(m_ImportedTableData); 


        // Set the initial column to the first one that does not have
        // qualifier automatched or selected yet. If all are matched,
        // set initial focus column to the first one
        int focus_col_idx = -1;

        // Iterate over columns and guess types/qualifiers
        for (size_t col=1; col<m_ImportedTableData->GetColumns().size(); ++col) {

            m_ImportedTableData->GetColumn(col).SetIsCurrent(false);

            // Set default type for columns in qualifier table - all text except for the first one
            // which should be an ID (but user can change)
            if (col == 1) {
                // If not already set:
                bool has_match = false;
                for (size_t mcol=1; mcol<m_ImportedTableData->GetColumns().size(); ++mcol) {
                    if (m_ImportedTableData->GetColumn(mcol).GetMatchColumn()) {
                        has_match = true;
                        break;
                    }
                }

                if (m_ImportedTableData->GetColumn(col).GetQualifier() == "" && !has_match) {
                    m_PrevColName = m_ImportedTableData->GetColumn(col).GetName();

                    m_ImportedTableData->GetColumn(col).SetType(CTableImportColumn::eTextColumn);
                    m_ImportedTableData->GetColumn(col).SetQualifierType(kFieldTypeSeqId);                    
                    m_ImportedTableData->GetColumn(col).SetQualifier(kFieldTypeSeqId); 
                    m_ImportedTableData->GetColumn(col).SetMatchColumn(true);
                    m_ImportedTableData->GetColumn(col).SetName("match " + kSequenceIdAbbrevColLabel);
                    int width = m_ImportedTableData->GetColumn(col).GetWidth();
                    m_ImportedTableData->GetColumn(col).SetWidth((int)m_ImportedTableData->GetColumn(col).GetName().length());
                }
                else {
                    focus_col_idx = 1;
                }
            }
            else {
                // If this is not the first time we visited the panel, column may already be skipped
                // If so, leave it as such.
                if (!m_ImportedTableData->GetColumn(col).GetSkipped()) {                
                    m_ImportedTableData->GetColumn(col).SetType(CTableImportColumn::eTextColumn);
                }

                // If a qualifier has not yet been selected for this column, try automatch to find
                // one (this may not be the first time init is called, so some may already be set).
                // If the user did not specify a header row, (no column names) we will not get any.                    
                if (m_ImportedTableData->GetColumn(col).GetQualifier() == "") {
                    bool matched = false;
                    string col_name = m_ImportedTableData->GetColumnName(col);
                    if (col_name != "") {

                        string qual = m_Fields->AutoMatch(col_name);
                        if (qual != "") {
                            CFieldNamePanel::EFieldType ft = m_Fields->GetFieldTypeFromName(qual);

                            if (ft != CFieldNamePanel::eFieldType_Unknown) {
                                string ft_str = CFieldNamePanel::GetFieldTypeName(ft);
                                string qual_name = qual.substr(ft_str.length(), qual.length()-ft_str.length());
                                NStr::TruncateSpacesInPlace(qual_name);
                                m_ImportedTableData->GetColumn(col).SetQualifierType(ft_str);                                    
                                m_ImportedTableData->GetColumn(col).SetQualifier(qual_name); 
                                m_ImportedTableData->GetColumn(col).SetName(qual_name);
                                int width = m_ImportedTableData->GetColumn(col).GetWidth();
                                m_ImportedTableData->GetColumn(col).SetWidth((int)m_ImportedTableData->GetColumn(col).GetName().length());
                                matched = true;
                            }
                        }
                    }
                    if (m_ImportedTableData->GetColumn(col).GetQualifier() == "") {
                        m_ImportedTableData->GetColumn(col).SetType(
                            CTableImportColumn::eSkippedColumn);
                    }

                    if (!matched && focus_col_idx==-1)
                        focus_col_idx = col;
                }
            }
        }

        m_ColumnIdList->UpdateColumnImages();

        // Should only be one selection, but loop anyway and unselect
        // any selected row
        long item = -1;
        for ( ;; )
        {
            item = m_ColumnIdList->GetNextItem(item,
                wxLIST_NEXT_ALL,
                wxLIST_STATE_SELECTED);
            if ( item == -1 )
                break;

            // deselect
            m_ColumnIdList->SetItemState(item, 0, wxLIST_STATE_SELECTED);
        }


        // Is there a currently selected column?  If so make sure the fields
        // for editing column info are up-to-date. (This only happens if the
        // users was previously on this page and is returning)
        if (m_CurrentColumnIdx >= 0 && 
            m_CurrentColumnIdx < (int)m_ImportedTableData->GetColumns().size()) {

                wxListEvent dummy_event;
                dummy_event.m_col = m_CurrentColumnIdx;

                OnCtableImportListctrlColLeftClick(dummy_event);
        }
        // Set the column being edited to the first one that has not been matched
        // since that is (probably) the first one the user needs to work on
        else {
            wxListEvent dummy_event;

            if (focus_col_idx==-1)
                dummy_event.m_col = 1;
            else
                dummy_event.m_col = focus_col_idx;

            OnCtableImportListctrlColLeftClick(dummy_event);
        }
    }
}

void CAttribTableColumnIdPanel::SetMainTitle(const wxString& title)
{
}

void CAttribTableColumnIdPanel::UpdateCol()
{
    if (!m_Fields || ! m_ImportedTableData)
        return;

    string field_name = m_Fields->GetFieldName();
    string prev_qual_name = m_ImportedTableData->
        GetColumn(m_CurrentColumnIdx).GetQualifier();

    // Do not update the field name to "" if there is already a column name there
    // and the user has not chosen a field name
    if (field_name != "" || 
        (prev_qual_name != "" && prev_qual_name != kSequenceIdAbbrevColLabel && prev_qual_name != kFieldTypeTaxname)) {

            // No updates to the ID/Match column here (it gets updated when
            // user clicks on match button)
            if (m_ImportedTableData->GetColumn(m_CurrentColumnIdx).GetQualifierType() != kFieldTypeSeqId) {       
                string qual_type_str = m_Fields->GetFieldTypeName(m_Fields->GetFieldType());

                m_ImportedTableData->GetColumn(m_CurrentColumnIdx).
                    SetQualifierType(qual_type_str);  
                m_ImportedTableData->GetColumn(m_CurrentColumnIdx).
                    SetQualifier(m_Fields->GetFieldName());  

                if (field_name != "") {
                    m_SkipFormatBtn->SetValue(false);
                    m_ImportedTableData->GetColumn(m_CurrentColumnIdx).SetType(
                        CTableImportColumn::eTextColumn);
                }

                if (m_ImportedTableData->GetColumn(m_CurrentColumnIdx).GetMatchColumn() ) {
                    if (field_name != "") {
                        field_name = "match " + field_name;
                    }
                    else {
                        field_name = "match " + 
                            m_ImportedTableData->GetColumn(m_CurrentColumnIdx).GetName(); 
                    }
                }
                m_ImportedTableData->GetColumn(m_CurrentColumnIdx).
                    SetName(field_name);
                m_ImportedTableData->GetColumn(m_CurrentColumnIdx).
                    SetWidth(field_name.length());
            }
    }
}


bool CAttribTableColumnIdPanel::IsInputValid()
{
    // When we leave the page, check if we need to update field value of
    // current column (we otherwise update these not when field choice updates
    // but rather when user clicks on a new column)
    // If its the match column we do not update it since it has already been set
    // (and we have to keep the 'match' string in the title)
    if (m_CurrentColumnIdx != -1  &&
        (size_t)m_CurrentColumnIdx < m_ImportedTableData->GetColumns().size()) {
            UpdateCol();
    }

    size_t col;
    for (col=1; col<m_ImportedTableData->GetColumns().size(); ++col) {
        if (m_ImportedTableData->GetColumn(col).GetMatchColumn() &&
            m_ImportedTableData->GetColumn(col).GetQualifierType() == kFieldTypeSeqId) {
                break;
        }
    }


    // first column is a row number field and is not in the data
    --col;

    if (col != m_ImportedTableData->GetColumns().size()-1) {

        string msg;
        map<string, set<CSeq_entry_Handle> > value_to_sehs;
        CMiscSeqTable::s_GetValueToSehMap(m_TopSeqEntry, value_to_sehs, "Local ID", CFieldNamePanel::eFieldType_LocalId, -1);

        for (size_t row=m_ImportedTableData->GetFirstImportRow(); 
             row < m_ImportedTableData->GetNumRows(); ++row) {

                // Get field from table (if this row is messed up and it doesn't have
                // enough fields blank will be returned
                string id_val = m_ImportedTableData->GetField(row, col);
                NStr::TruncateSpacesInPlace(id_val);
                string seqid_str;

                CRef<CSeq_id> guess = m_id_guesser->Guess(id_val);
                
                if (!guess.IsNull()) {
                    guess->GetLabel(&seqid_str, CSeq_id::eBoth);
                }
                else if (value_to_sehs.find(id_val) != value_to_sehs.end()) {
                    seqid_str = id_val;
                }
                

                if (seqid_str.empty()) {
                    if (!msg.empty())
                        msg += "\n";

                    msg += "\"" + id_val + "\" does not appear to be a valid ID";
                    continue;
                }


                // Don't try to set a field that's not there (if this row is incomplete 
                // i.e. did not have a seq-id column because of too few delims)
                if (col < m_ImportedTableData->GetRow(row).GetNumFields()) {
                    // replace the substring in the row that currently represents the ID
                    // with the updated ID.
                    string row_text = m_ImportedTableData->GetRow(row).GetValue();
                    pair<size_t,size_t> field_pos = m_ImportedTableData->GetRow(row).GetFields()[col];
           
                    string updated_row = row_text.substr(0, field_pos.first);
                    updated_row += seqid_str;
                    updated_row += row_text.substr(field_pos.first + field_pos.second, 
                                   row_text.length()-(field_pos.first + field_pos.second));

                    m_ImportedTableData->GetRow(row).GetValue() = updated_row;
                    m_ImportedTableData->RecomputeRowFields(row);              
                }
        }

        if (msg != "") {            
            CLongMessageDialog dlg(this, wxString(msg + "\nDo you want to continue?"));
            int res = dlg.ShowModal(); 
            //int res = wxMessageBox(ToWxString(msg) + "\nDo you want to continue?", wxT("ID Errors"), wxYES_NO | wxICON_QUESTION);
            if (res != wxYES)
                return false;
        }
    }
    else {
        /// error not to have a match column
        size_t match_col = -1;
        for (match_col=1; match_col<m_ImportedTableData->GetColumns().size(); ++match_col) {
            if (m_ImportedTableData->GetColumn(match_col).GetMatchColumn()) {
                break;
            }
        }

        if (match_col == m_ImportedTableData->GetColumns().size()) {
            wxMessageBox(wxT("Table must have an ID column"), wxT("Error"), wxICON_WARNING | wxOK);
            return false;
        }
    }

    set<string> unique_names;
    for (col=1; col<m_ImportedTableData->GetColumns().size(); ++col) {
        string name = m_ImportedTableData->GetColumn(col).GetQualifier(); 

        if (!NStr::IsBlank(name)) {
            if (!m_ImportedTableData->GetColumn(col).GetMatchColumn())
            {
                auto inserted_it = unique_names.insert(name);
                if (!m_ImportedTableData->GetColumn(col).HasProperty("text_separator"))
                    m_ImportedTableData->GetColumn(col).SetProperty("text_separator", ";");
                
                if (!m_ImportedTableData->GetColumn(col).HasProperty("update_rule")) {
                    if (inserted_it.second) {
                        m_ImportedTableData->GetColumn(col).SetProperty("update_rule", "replace");
                    }
                    else {
                        m_ImportedTableData->GetColumn(col).SetProperty("update_rule", "add_new_qual");
                    }
                }
            }

            if (!m_ImportedTableData->GetColumn(col).HasProperty("blanks"))
                m_ImportedTableData->GetColumn(col).SetProperty("blanks", "ignore");

            CMiscSeqTableColumn seq_table_col(name); 

            vector<string> col_values;

            for (size_t row=m_ImportedTableData->GetFirstImportRow(); 
                row < m_ImportedTableData->GetNumRows(); ++row) {

                    // Get field from table (if this row is messed up and it doesn't have
                    // enough fields blank will be returned).  Subtract one here
                    // since the data columns are 1 less than the GetColumns() array (because
                    // line number not in data)
                    string field_val = m_ImportedTableData->GetField(row, col-1);
                    col_values.push_back(field_val);
            }

            vector<string> results = seq_table_col.IsValid(col_values);

            if (results.size() > 0) {
                string msg;
                for (size_t i=0; i<results.size(); ++i) {
                    if (results[i].length() > 0) {
                        if (msg != "")
                            msg += "\n";

                        msg += results[i];
                    }
                }

                int res = wxMessageBox(ToWxString(msg) + "\nDo you want to continue?", wxT("Validation Errors"), wxYES_NO | wxICON_QUESTION);
                if (res != wxYES)
                    return false;
            }
        }
    }

    return true;
}


void CAttribTableColumnIdPanel::SetRegistryPath( const string& path )
{
    m_RegPath = path;
}

void CAttribTableColumnIdPanel::LoadSettings()
{
    if ( !m_RegPath.empty() ) { 
    }
}


void CAttribTableColumnIdPanel::SaveSettings() const
{
    if ( !m_RegPath.empty() ) {
        CGuiRegistry& gui_reg = CGuiRegistry::GetInstance();
        CRegistryWriteView view = gui_reg.GetWriteView(m_RegPath);
    }
}

void CAttribTableColumnIdPanel::x_EnableColumnEditControls(bool b)
{
    m_SkipFormatBtn->Enable(b);  
    if (m_MatchColumn) m_MatchColumn->Enable(b);
    
    x_EnableNonFormatEditControls(b);
}

void CAttribTableColumnIdPanel::x_EnableNonFormatEditControls(bool b)
{
    m_ColumnNameTxtCtrl->Enable(b);
    m_ColumnPropertiesSizer->Enable(b);
    m_ColumnNameStaticTxt->Enable(b);
    m_Fields->Enable(b);
}

void CAttribTableColumnIdPanel::x_EnableMachColumnFields(bool match_column)
{
    m_MatchType->Enable(match_column);
    if (!match_column) {
        m_Fields->Enable();
    }

    bool allow_options = !match_column;

    m_Semicolon->Enable(allow_options);
    m_Space->Enable(allow_options);
    m_Comma->Enable(allow_options);
    m_Colon->Enable(allow_options);
    m_NoDelimiter->Enable(allow_options);

    m_IgnoreBlanks->Enable(allow_options);
    m_EraseValue->Enable(allow_options);

    m_Replace->Enable(allow_options);
    m_Append->Enable(allow_options);
    m_Prefix->Enable(allow_options);
    m_LeaveOld->Enable(allow_options);
    m_AddQual->Enable(allow_options);
}

void CAttribTableColumnIdPanel::ProcessUpdateStructCommentEvent( wxCommandEvent& event )
{
    set<string> fields;
    CStructCommentFieldPanel::GetStructCommentFields(m_TopSeqEntry,fields);
    UpdateStructuredCommentFields(this, fields);
}

void CAttribTableColumnIdPanel::UpdateStructuredCommentFields( wxWindow* win, const set<string> &fields )
{
    wxWindowList &slist = win->GetChildren();
    for (wxWindowList::iterator iter = slist.begin(); iter != slist.end(); ++iter) 
    {
        wxWindow* child = *iter;
        CStructFieldValuePanel* fieldvalue_panel = dynamic_cast<CStructFieldValuePanel*>(child);
        CStructCommentFieldPanel* strcomm_fields_panel = dynamic_cast<CStructCommentFieldPanel*>(child);
        if (fieldvalue_panel || strcomm_fields_panel) 
        {
            if (fieldvalue_panel) 
            {
                fieldvalue_panel->ListStructCommentFields(fields);
            } 
            else if (strcomm_fields_panel) 
            {
                strcomm_fields_panel->CompleteStructCommentFields(fields);
            }
        } 
        UpdateStructuredCommentFields(child, fields);
    }
}

void CAttribTableColumnIdPanel::UpdateChildrenFeaturePanels( wxWindow* win )
{
    wxWindowList &slist = win->GetChildren();
    for (wxWindowList::iterator iter = slist.begin(); iter != slist.end(); ++iter) 
    {
        wxWindow* child = *iter;
        CFeatureTypePanel* panel = dynamic_cast<CFeatureTypePanel*>(child);
        if (panel) 
        {
            panel->ListPresentFeaturesFirst(m_TopSeqEntry);
        } 
        else 
        {
            UpdateChildrenFeaturePanels(child);
        }
    }     
}

/*!
 * Should we show tooltips?
 */

bool CAttribTableColumnIdPanel::ShowToolTips()
{
    return true;
}

/*!
 * Get bitmap resources
 */

wxBitmap CAttribTableColumnIdPanel::GetBitmapResource( const wxString& name )
{
    // Bitmap retrieval
////@begin CAttribTableColumnIdPanel bitmap retrieval
    wxUnusedVar(name);
    return wxNullBitmap;
////@end CAttribTableColumnIdPanel bitmap retrieval
}

/*!
 * Get icon resources
 */

wxIcon CAttribTableColumnIdPanel::GetIconResource( const wxString& name )
{
    // Icon retrieval
////@begin CAttribTableColumnIdPanel icon retrieval
    wxUnusedVar(name);
    return wxNullIcon;
////@end CAttribTableColumnIdPanel icon retrieval
}


void CAttribTableColumnIdPanel::OnCtableImportListctrlColLeftClick( wxListEvent& event )
{
    if (!m_ImportedTableData.IsNull()) {
        int col = event.GetColumn();
        
        // First column is the row number column and has no attributes.
        if (col == 0) {
            event.Veto();
            return;
        }

        if ((size_t)col < m_ImportedTableData->GetColumns().size()) {
    
            /// if there is another current column, remove the checkmark from it:
            if (m_CurrentColumnIdx != -1  &&
                m_CurrentColumnIdx != col &&        
                (size_t)m_CurrentColumnIdx < m_ImportedTableData->GetColumns().size()) {
                    UpdateCol();

                    m_ImportedTableData->GetColumn(m_CurrentColumnIdx).SetIsCurrent(false);
            }           

            m_CurrentColumnIdx = col;

            m_SkipFormatBtn->SetValue(false);

            CTableImportColumn::eColumnType t = 
                m_ImportedTableData->GetColumn(m_CurrentColumnIdx).GetType();

            if (t == CTableImportColumn::eSkippedColumn) {
                m_SkipFormatBtn->SetValue(true);
            }

            // Controls initially default to not being enabled, since no column is
            // selected.  Enable all controls for identifying column info here.
            x_EnableColumnEditControls(true);

            // If blank, this just sets type to unknown
            string fieldtype = m_ImportedTableData->GetColumn(m_CurrentColumnIdx).GetQualifierType();

            if ( m_ImportedTableData->GetColumn(m_CurrentColumnIdx).GetMatchColumn()) {
                if (m_ImportedTableData->GetColumn(m_CurrentColumnIdx).GetQualifierType() == 
                    kFieldTypeSeqId) {
                        m_MatchType->SetSelection(0);

                        m_Fields->SetFieldType(CFieldNamePanel::eFieldType_Unknown);
                        m_Fields->Disable();
                }
                else if (m_ImportedTableData->GetColumn(m_CurrentColumnIdx).GetQualifierType() == 
                         kFieldTypeTaxname) {
                        m_MatchType->SetSelection(1);

                        m_Fields->SetFieldType(CFieldNamePanel::eFieldType_Unknown);
                        m_Fields->Disable();
                }
                else {
                    // m_SeqidCol->SetValue(false);
                    m_MatchType->SetSelection(2); // other, although if not the match column, this is irrelevant
                    m_Fields->Enable();

                    // There is also SetFieldType, but SetFieldName only works for features if the
                    // type is also appended.
                    string fieldname = m_ImportedTableData->GetColumn(m_CurrentColumnIdx).GetQualifier();       
                    if (fieldname != "") {
                        m_Fields->SetFieldName(fieldtype + " " + fieldname);
                    }
                    else {
                        // Default to a source field
                        m_Fields->SetFieldType(CFieldNamePanel::eFieldType_Source);
                    }
                }
                if (m_MatchColumn) {
                    m_MatchColumn->SetValue(true);
                }
                x_EnableMachColumnFields(true);
            }
            else {
                if (m_MatchColumn) {
                    m_MatchColumn->SetValue(false);
                }
                x_EnableMachColumnFields(false);

                // There is also SetFieldType, but SetFieldName only works for features if the
                // type is also appended.
                string fieldname = m_ImportedTableData->GetColumn(m_CurrentColumnIdx).GetQualifier();       
                if (fieldname != "") {
                    m_Fields->SetFieldName(fieldtype + " " + fieldname);
                }
                else {
                    // Default to a source field
                    m_Fields->SetFieldType(CFieldNamePanel::eFieldType_Source);
                }
            }

            /// Update controls to represent current column choice:
            m_ColumnNameTxtCtrl->SetValue(
                m_ImportedTableData->GetColumnName((size_t)m_CurrentColumnIdx));

            // Set radio buttons for text handling:
            string prop = m_ImportedTableData->GetColumn(m_CurrentColumnIdx).GetProperty("text_separator");
            if (prop == ";") 
                m_Semicolon->SetValue(true);
            else if (prop == " ")
                m_Space->SetValue(true);
            else if (prop == ",")
                m_Comma->SetValue(true);
            else if (prop == ":")
                m_Colon->SetValue(true);
            // special case - a blank value is returned if that's the value OR there is no such property
            else if (prop=="" && m_ImportedTableData->GetColumn(m_CurrentColumnIdx).HasProperty("text_separator"))
                m_NoDelimiter->SetValue(true);
            else
                m_Semicolon->SetValue(true);

            prop = m_ImportedTableData->GetColumn(m_CurrentColumnIdx).GetProperty("blanks");
            if (prop == "ignore") 
                m_IgnoreBlanks->SetValue(true);
            else if (prop == "erase")
                m_EraseValue->SetValue(true);
            else 
                m_IgnoreBlanks->SetValue(true);

            prop = m_ImportedTableData->GetColumn(m_CurrentColumnIdx).GetProperty("update_rule");
            if (prop == "replace") 
                m_Replace->SetValue(true);
            if (prop == "append") 
                m_Append->SetValue(true);
            else if (prop == "prefix")
                m_Prefix->SetValue(true);
            else if (prop == "ignore")
                m_LeaveOld->SetValue(true);
            else if (prop=="add_new_qual")
                m_AddQual->SetValue(true);
            else
                m_Replace->SetValue(true);

            m_Fields->Refresh();

            /// Set column to be 'current'
            m_ImportedTableData->GetColumn(m_CurrentColumnIdx).SetIsCurrent(true);
        }
    }


    m_ColumnIdList->UpdateColumnImages();
    Refresh();

}

void CAttribTableColumnIdPanel::OnColumnNameTxtTextUpdated( wxCommandEvent& event )
{
    wxString colname = m_ColumnNameTxtCtrl->GetValue();

    event.Skip();
}


void CAttribTableColumnIdPanel::OnSkipBtnSelected( wxCommandEvent& event )
{
   if (m_CurrentColumnIdx != -1 &&
        !m_ImportedTableData.IsNull() &&
        (size_t)m_CurrentColumnIdx < m_ImportedTableData->GetColumns().size()) {

        /// Set column type based on user selection
        if (event.IsChecked()) {
            m_ImportedTableData->GetColumn(m_CurrentColumnIdx).SetType(
                CTableImportColumn::eSkippedColumn);
        
            // Disable column controls other than format (which allows skipped/not skipped
            // to be set)
            //x_EnableNonFormatEditControls(false);
            m_Fields->SetFieldType(CFieldNamePanel::eFieldType_Unknown);
            m_Fields->SetFieldName("");
            UpdateCol();
        }
        else {
            m_ImportedTableData->GetColumn(m_CurrentColumnIdx).SetType(
                CTableImportColumn::eTextColumn);

            // Disable column controls other than format (which allows skipped/not skipped
            // to be set)
            //x_EnableNonFormatEditControls(true);
        }

        m_ColumnIdList->UpdateColumnImages();
       
        // Update first row which holds type information
        //m_ColumnIdList->RefreshItem(0);
        m_ColumnIdList->Refresh();
    }
}

void CAttribTableColumnIdPanel::OnCtableImportListCtrlColDragging( wxListEvent& event )
{
    // Update the whole control when dragging.  This is needed because there
    // may be header rows where distrubtion across columns is based on width
    // of each column (basically they are left-justified into row irrespective
    // of column boundaries)
    m_ColumnIdList->Refresh();
    event.Skip();
}

void CAttribTableColumnIdPanel::ProcessUpdateFeatEvent( wxCommandEvent& event )
{
        UpdateChildrenFeaturePanels(this);
}

void CAttribTableColumnIdPanel::UpdateEditor()
{
    UpdateCol();
    m_ColumnIdList->UpdateColumnImages();
    m_ColumnIdList->Refresh();
    x_RefreshParentWnd();
}

void CAttribTableColumnIdPanel::x_RefreshParentWnd()
{
    wxDialog* parent = nullptr;
    wxWindow* w = this->GetParent();

    while (w != NULL) {
        parent = dynamic_cast<wxDialog*>(w);
        if (parent) {
            break;
        }
        else {
            w = w->GetParent();
        }
    }
    if (!parent)
        return;

    parent->Freeze();
    parent->Layout();
    parent->Refresh();
    parent->Thaw();
}

void CAttribTableColumnIdPanel::OnMatchColumnClick( wxCommandEvent& event )
{
    string field_name = m_Fields->GetFieldName();
    event.Skip();

    if (event.IsChecked()) {
        x_EnableMachColumnFields(true);

        // If any other column is marked as the match column, unset it
        for (size_t col=1; col<m_ImportedTableData->GetColumns().size(); ++col) {
            if (m_ImportedTableData->GetColumn(col).GetMatchColumn() &&
                col != m_CurrentColumnIdx) {
                    m_ImportedTableData->GetColumn(col).SetMatchColumn(false);
                    string name = m_ImportedTableData->GetColumn(col).GetName();

                    if (m_ImportedTableData->GetColumn(col).GetQualifierType() == kFieldTypeSeqId ||
                        m_ImportedTableData->GetColumn(col).GetQualifierType() == kFieldTypeTaxname) {
                        // setting to seqid loses previous values. Since this unsets seqid, set them back
                        m_ImportedTableData->GetColumn(col).SetName(m_PrevColName);
                        m_ImportedTableData->GetColumn(col).SetWidth(m_PrevColName.length());
                        m_ImportedTableData->GetColumn(col).SetQualifierType(m_PrevQualType);  
                        m_ImportedTableData->GetColumn(col).SetQualifier(m_PrevQual);                               
                    }
                    else if (!NStr::Compare(name, 0, 6, "match ")) {
                        name = name.substr(6, name.length()-6);
                        m_ImportedTableData->GetColumn(col).SetName(name);
                        m_ImportedTableData->GetColumn(col).SetWidth(name.length());
                    }

                    // If this is not the first time we visited the panel, column may already be skipped
                    // If so, leave it as such.
                    if (!m_ImportedTableData->GetColumn(col).GetSkipped()) {                
                        m_ImportedTableData->GetColumn(col).SetType(CTableImportColumn::eTextColumn);
                    }

                    // If a qualifier has not yet been selected for this column, try automatch to find
                    // one (this may not be the first time init is called, so some may already be set).
                    // If the user did not specify a header row, (no column names) we will not get any.                    
                    if (m_ImportedTableData->GetColumn(col).GetQualifier() == "") {
                        bool matched = false;
                        string col_name = m_ImportedTableData->GetColumnName(col);
                        if (col_name != "") {
                            
                            string qual = m_Fields->AutoMatch(col_name);
                            if (qual != "") {
                                CFieldNamePanel::EFieldType ft = m_Fields->GetFieldTypeFromName(qual);
                                
                                if (ft != CFieldNamePanel::eFieldType_Unknown) {
                                    string ft_str = CFieldNamePanel::GetFieldTypeName(ft);
                                    string qual_name = qual.substr(ft_str.length(), qual.length()-ft_str.length());
                                    NStr::TruncateSpacesInPlace(qual_name);
                                    m_ImportedTableData->GetColumn(col).SetQualifierType(ft_str);                                    
                                    m_ImportedTableData->GetColumn(col).SetQualifier(qual_name); 
                                    m_ImportedTableData->GetColumn(col).SetName(qual_name);
                                    int width = m_ImportedTableData->GetColumn(col).GetWidth();
                                    m_ImportedTableData->GetColumn(col).SetWidth((int)m_ImportedTableData->GetColumn(col).GetName().length());
                                    matched = true;
                                }
                            }
                        }
                        if (m_ImportedTableData->GetColumn(col).GetQualifier() == "") {
                            m_ImportedTableData->GetColumn(col).SetType(
                                CTableImportColumn::eSkippedColumn);
                        }
                        
                    }
            }
        } 

        // Get the current qualifier name and type and update the table, if needed
        string prev_qual_name = m_ImportedTableData->
            GetColumn(m_CurrentColumnIdx).GetQualifier();

        m_PrevColName = m_ImportedTableData->GetColumn(m_CurrentColumnIdx).GetName();

        if (field_name == "taxname") {
            m_Fields->Disable();
            m_MatchType->SetSelection(1); // taxname
        }
        else if (prev_qual_name == kFieldTypeSeqId) {
            m_Fields->Disable();
            m_MatchType->SetSelection(0); // seq-id
        }
        else {
            m_Fields->Enable();
            m_MatchType->SetSelection(2); // set selection to 'other'
            m_Fields->SetFieldType(CFieldNamePanel::eFieldType_Source);
        }

        string qual_type_str = m_Fields->GetFieldTypeName(m_Fields->GetFieldType());

        //
        if ((field_name != "" || prev_qual_name != "")) {
            m_ImportedTableData->GetColumn(m_CurrentColumnIdx).
                SetQualifierType(qual_type_str);  
            m_ImportedTableData->GetColumn(m_CurrentColumnIdx).
                SetQualifier(field_name);  
        }

        if (field_name != "") {
            m_ImportedTableData->GetColumn(m_CurrentColumnIdx).
                SetName("match " + field_name); 
        }
        else {
            m_ImportedTableData->GetColumn(m_CurrentColumnIdx).
                SetName("match " + m_ImportedTableData->GetColumn(m_CurrentColumnIdx).GetName()); 
        }
        m_ImportedTableData->GetColumn(m_CurrentColumnIdx).
            SetWidth( m_ImportedTableData->GetColumn(m_CurrentColumnIdx).GetName().length());

        m_ImportedTableData->GetColumn(m_CurrentColumnIdx).SetMatchColumn(true);         
        m_SkipFormatBtn->SetValue(false);
        m_ImportedTableData->GetColumn(m_CurrentColumnIdx).SetType(CTableImportColumn::eTextColumn);
    }
    else {
        m_ImportedTableData->GetColumn(m_CurrentColumnIdx).SetMatchColumn(false);

        // if type of match was not other (so it was taxname or seqid)
        if (m_MatchType->GetSelection() != 2) {
            // setting to seqid or taxname loses previous values. Since this unsets seqid, set them back
            m_ImportedTableData->GetColumn(m_CurrentColumnIdx).SetName(m_PrevColName);
            m_ImportedTableData->GetColumn(m_CurrentColumnIdx).SetWidth(m_PrevColName.length());
            m_ImportedTableData->GetColumn(m_CurrentColumnIdx).SetQualifierType(m_PrevQualType);  
            m_ImportedTableData->GetColumn(m_CurrentColumnIdx).SetQualifier(m_PrevQual); 
            m_MatchType->SetSelection(2); // set selection to 'other'
        }
        else {                   
            m_ImportedTableData->GetColumn(m_CurrentColumnIdx).SetName(field_name);
            m_ImportedTableData->GetColumn(m_CurrentColumnIdx).SetWidth(field_name.length());
        }

        x_EnableMachColumnFields(false);
    }

    m_ColumnNameTxtCtrl->SetValue(
        m_ImportedTableData->GetColumnName((size_t)m_CurrentColumnIdx));

    m_ColumnIdList->UpdateColumnImages();
    Refresh();
}

void CAttribTableColumnIdPanel::OnMailReportCheckboxClick( wxCommandEvent& event )
{
    event.Skip();
}

void CAttribTableColumnIdPanel::OnMergeIDColsClick( wxCommandEvent& event )
{
    // Merge first and second columns, and put a colon in between. Does not
    // make sense to do this more than once, so disable after
    vector<size_t> merge_cols;
    merge_cols.push_back(1);
    merge_cols.push_back(2);

    vector<CTableImportColumn> cols = m_ImportedTableData->GetColumns();

    // Merge the first two data columns
    m_ImportedTableData->MergeColumns(merge_cols, '/');
    m_ImportedTableData->GetColumn(1).SetMatchColumn(true);
    m_ImportedTableData->GetColumn(1).SetType(CTableImportColumn::eTextColumn);
    m_ImportedTableData->GetColumn(1).SetQualifierType(kFieldTypeSeqId);
    m_ImportedTableData->GetColumn(1).SetQualifier(kFieldTypeSeqId);
    m_ImportedTableData->GetColumn(1).SetName("match " + kSequenceIdAbbrevColLabel);
    m_ImportedTableData->GetColumn(1).SetWidth(
        m_ImportedTableData->GetColumn(1).GetName().length());


    // After merging columns, table will re-create columns based on headers, but we prefer to
    // use the previous column meta-info which could included qualifiers from both auto-match
    // and user-interaction.
    for (int i=3; i<cols.size() && i<m_ImportedTableData->GetColumns().size()+1; ++i) {
        m_ImportedTableData->GetColumn(i-1) = cols[i];
    }    

    // This recomputes (and resizes) columns
    m_ColumnIdList->SetDataSource(m_ImportedTableData);

    m_MergeIDCols->Disable();
    m_ColumnIdList->UpdateColumnImages();
    
    // Set the column being edited to the first one following the merged
    // id column (or the id column if there are no other columns)       
    wxListEvent dummy_event;

    dummy_event.m_col = std::min(2, (int)(m_ImportedTableData->GetColumns().size()-1));
    OnCtableImportListctrlColLeftClick(dummy_event);

    Refresh();
}

string CAttribTableColumnIdPanel::x_GetConstraintField()
{
    CFieldNamePanel* constraint_field_panel = m_Constraint->GetFieldNamePanel();
    string constraint_field;
    string constraint_field_type = m_Constraint->GetFieldType();
    if (constraint_field_panel)
        constraint_field = constraint_field_panel->GetFieldName();

    if (NStr::IsBlank(constraint_field)) {
        constraint_field = constraint_field_type;
    } else {
        if (NStr::StartsWith(constraint_field_type, "RNA") && NStr::Find(constraint_field, "RNA") == NPOS ) {
            constraint_field = "RNA " + constraint_field;
        }
    }
    return constraint_field;
}


void CAttribTableColumnIdPanel::x_TableReaderCommon(CIRef<IObjectLoader> object_loader, const wxString& msg, bool modal)
{
    if (!object_loader) {
        wxMessageBox(wxT("Failed to get object loader"), wxT("Error"),
                        wxOK | wxICON_ERROR);
        return;
    }
    else {
        IExecuteUnit* execute_unit = dynamic_cast<IExecuteUnit*>(object_loader.GetPointer());
        if (execute_unit) {
            if (!execute_unit->PreExecute())
                return;

            if (!GUI_AsyncExecUnit(*execute_unit, msg))
                return; // Canceled

            if (!execute_unit->PostExecute())
                return;
        }
    }
    bool make_mail_report = m_MakeMailReport->GetValue();
    const IObjectLoader::TObjects& objects = object_loader->GetObjects();
    ITERATE(IObjectLoader::TObjects, obj_it, objects) {
        const CObject& ptr = obj_it->GetObject();
        const objects::CSeq_annot* annot = dynamic_cast<const objects::CSeq_annot*>(&ptr);
        if (annot && annot->IsSeq_table()) {
            try {
                CRef<objects::CSeq_table> input_table (new objects::CSeq_table());
                input_table->Assign(annot->GetData().GetSeq_table());
                CRef<objects::CUser_object> column_properties(new objects::CUser_object);
                
                // Get user data from seqannot
                const CUser_object* ud = NULL;
                if (annot->CanGetDesc()) {    
                    const CSeq_annot::TDesc& annot_desc = annot->GetDesc();
                    if (annot_desc.CanGet()) {      
                        const CAnnot_descr_Base::Tdata& desc_list_data = annot_desc.Get();
                        CAnnot_descr_Base::Tdata::const_iterator tditer = desc_list_data.begin();

                        for (; tditer!=desc_list_data.end(); ++tditer) {
                        if ( (*tditer)->IsUser() )
                            break;
                        }

                        if (tditer != desc_list_data.end()) {
                            // Get User data from Annot Descriptor this is the Properties data 
                            // structure, where each entry is a combination of column number
                            // and field properties, e.g.
                            // Column.1  &xtype=StartPosition &one_based=true
                            ud = &(*tditer)->GetUser();
                        }
                    }
                }

                if (ud != NULL)
                    column_properties->Assign(*ud);

                CRef<CSeq_table> mail_report(NULL);
                if (make_mail_report) {
                    mail_report = edit::MakeMailReportPreReport(m_TopSeqEntry);
                }

                string constraint_field = x_GetConstraintField();
                CRef<edit::CStringConstraint> string_constraint = m_Constraint->GetStringConstraint();

                CMiscSeqTable table_applier(input_table, m_TopSeqEntry, *m_id_guesser, column_properties, constraint_field, string_constraint, m_CmdProccessor);
                table_applier.ApplyTableToEntry();

                if (mail_report) {
                    edit::MakeMailReportPostReport(*mail_report, m_TopSeqEntry.GetScope());    
                    if (modal)
                    {
                        CGenericReportDlgModal* report = new CGenericReportDlgModal(NULL);
                        report->SetTitle(wxT("Organism Change Report"));
                        report->SetText(ToWxString(edit::GetReportFromMailReportTable(*mail_report, &m_TopSeqEntry.GetScope())));
                        report->ShowModal();
                    }
                    else
                    {
                        CGenericReportDlg* report = new CGenericReportDlg(NULL);
                        report->SetTitle(wxT("Organism Change Report"));
                        report->SetText(ToWxString(edit::GetReportFromMailReportTable(*mail_report, &m_TopSeqEntry.GetScope())));
                        report->Show(true);
                    }
                }
            }
            catch (CException& e) {
                wxMessageBox(e.GetMsg(), wxT("Error"),
                                wxOK | wxICON_ERROR, NULL);                        
            }                   
        }
    }
}

void CAttribTableColumnIdPanel::OnApplyNoCloseClick( wxCommandEvent& event )
{   
    bool valid = IsInputValid();
    if (!valid)
        return;
   
    CRef<CTableAnnotDataSource> annot_table_data;
    annot_table_data.Reset(new CTableAnnotDataSource());  

    m_ImportedTableData->ConvertToSeqAnnot(annot_table_data->GetContainer());
    CIRef<IObjectLoader> object_loader(new CTableObjectLoader(*annot_table_data, m_FileNames));

    wxString msg;
    x_TableReaderCommon(object_loader, msg, true);
}

void CAttribTableColumnIdPanel::OnMatchTypeSelected( wxCommandEvent& event )
{
    // No need to do this is the column has already been unset as the match column
    if (!m_ImportedTableData->GetColumn(m_CurrentColumnIdx).GetMatchColumn())
        return;

    // Must already be the match column (otherwise radio-box would be disabled)

    // If match type is seq-id:
    if (m_MatchType->GetSelection() == 0) {       
        m_PrevColName = m_ImportedTableData->GetColumn(m_CurrentColumnIdx).GetName();
        if (m_PrevColName.length() >= 6 && m_PrevColName.substr(0,6)== "match ") {
            if (m_PrevColName.length() == 6)
                m_PrevColName = "";
            else
                m_PrevColName = m_PrevColName.substr(6, m_PrevColName.length()-6);        
        }

        m_PrevQualType = m_ImportedTableData->GetColumn(m_CurrentColumnIdx).GetQualifierType();
        m_PrevQual = m_ImportedTableData->GetColumn(m_CurrentColumnIdx).GetQualifier(); 

        m_ImportedTableData->GetColumn(m_CurrentColumnIdx).SetQualifierType(kFieldTypeSeqId);  
        m_ImportedTableData->GetColumn(m_CurrentColumnIdx).SetQualifier(kFieldTypeSeqId); 
        m_ImportedTableData->GetColumn(m_CurrentColumnIdx).SetName("match " + kSequenceIdAbbrevColLabel);
        m_Fields->Disable();
    }
    // If match type is taxname:
    else if (m_MatchType->GetSelection() == 1) {
        m_PrevColName = m_ImportedTableData->GetColumn(m_CurrentColumnIdx).GetName();
        if (m_PrevColName.length() >= 6 && m_PrevColName.substr(0,6)== "match ") {
            if (m_PrevColName.length() == 6)
                m_PrevColName = "";
            else
                m_PrevColName = m_PrevColName.substr(6, m_PrevColName.length()-6);        
        }

        m_PrevQualType = m_ImportedTableData->GetColumn(m_CurrentColumnIdx).GetQualifierType();
        m_PrevQual = m_ImportedTableData->GetColumn(m_CurrentColumnIdx).GetQualifier(); 

        m_ImportedTableData->GetColumn(m_CurrentColumnIdx).SetQualifierType(kFieldTypeTaxname);  
        m_ImportedTableData->GetColumn(m_CurrentColumnIdx).SetQualifier(kFieldTypeTaxname); 
        m_ImportedTableData->GetColumn(m_CurrentColumnIdx).SetName("match " + CTempString(kFieldTypeTaxname));
        m_Fields->Disable();
    }
    // match type is other (string):
    else {      
        m_ImportedTableData->GetColumn(m_CurrentColumnIdx).SetQualifierType(m_PrevQualType);  
        m_ImportedTableData->GetColumn(m_CurrentColumnIdx).SetQualifier(m_PrevQual); 

        if (m_PrevQual != "")
            m_ImportedTableData->GetColumn(m_CurrentColumnIdx).SetName("match " + m_PrevQual);
        else
            m_ImportedTableData->GetColumn(m_CurrentColumnIdx).SetName("match " + m_PrevColName);

        m_ImportedTableData->GetColumn(m_CurrentColumnIdx).SetQualifierType(m_Fields->GetFieldTypeName(m_Fields->GetFieldType()));  
        m_ImportedTableData->GetColumn(m_CurrentColumnIdx).SetQualifier(m_Fields->GetFieldName()); 
        m_Fields->Enable();
        m_Fields->SetFieldType(CFieldNamePanel::eFieldType_Source);
    }


    m_ImportedTableData->GetColumn(m_CurrentColumnIdx).SetWidth(
        m_ImportedTableData->GetColumn(m_CurrentColumnIdx).GetName().length());
    m_SkipFormatBtn->SetValue(false);
    m_ImportedTableData->GetColumn(m_CurrentColumnIdx).SetType(CTableImportColumn::eTextColumn);
    m_ColumnNameTxtCtrl->SetValue(m_ImportedTableData->GetColumnName((size_t)m_CurrentColumnIdx));
    m_ColumnIdList->UpdateColumnImages();
    Refresh();
}


void CAttribTableColumnIdPanel::OnSplitFirstColClick( wxCommandEvent& event )
{
    // Split first column on first blank in that column.  Make the (new) first
    // field the seq-id match column.
    vector<CTableImportColumn> cols = m_ImportedTableData->GetColumns();

    // Merge the first two data columns
    m_ImportedTableData->SplitColumn(1, ' ', false);
    m_ImportedTableData->GetColumn(1).SetMatchColumn(true);
    m_ImportedTableData->GetColumn(1).SetType(CTableImportColumn::eTextColumn);
    m_ImportedTableData->GetColumn(1).SetQualifierType(kFieldTypeSeqId);
    m_ImportedTableData->GetColumn(1).SetQualifier(kFieldTypeSeqId);
    m_ImportedTableData->GetColumn(1).SetName("match " + kSequenceIdAbbrevColLabel);
    m_ImportedTableData->GetColumn(1).SetWidth(
        m_ImportedTableData->GetColumn(1).GetName().length());


    // After merging columns, table will re-create columns based on headers, but we prefer to
    // use the previous column meta-info which could included qualifiers from both auto-match
    // and user-interaction.
    for (int i=3; i<cols.size() && i<m_ImportedTableData->GetColumns().size()-1; ++i) {
        m_ImportedTableData->GetColumn(i+1) = cols[i];
    }

    // This recomputes (and resizes) columns
    m_ColumnIdList->SetDataSource(m_ImportedTableData);

    m_ColumnIdList->UpdateColumnImages();
    Refresh();
}

void CAttribTableColumnIdPanel::OnMultispaceToTabBtnClick( wxCommandEvent& event )
{
    m_ImportedTableData->ReplaceSpaces();
    x_GuessQualifiers();
}

///
/// Append field text delimiter options
///
void CAttribTableColumnIdPanel::OnSemicolonBtnSelected( wxCommandEvent& event )
{
    if (event.IsChecked())
        m_ImportedTableData->GetColumn(m_CurrentColumnIdx).SetProperty("text_separator", ";");
}

void CAttribTableColumnIdPanel::OnSpaceBtnSelected( wxCommandEvent& event )
{
    if (event.IsChecked())
        m_ImportedTableData->GetColumn(m_CurrentColumnIdx).SetProperty("text_separator", " ");
}

void CAttribTableColumnIdPanel::OnColonBtnSelected( wxCommandEvent& event )
{
    if (event.IsChecked())
        m_ImportedTableData->GetColumn(m_CurrentColumnIdx).SetProperty("text_separator", ":");
}

void CAttribTableColumnIdPanel::OnCommaBtnSelected( wxCommandEvent& event )
{
    if (event.IsChecked())
        m_ImportedTableData->GetColumn(m_CurrentColumnIdx).SetProperty("text_separator", ",");
}

void CAttribTableColumnIdPanel::OnNodelimiterBtnSelected( wxCommandEvent& event )
{
    if (event.IsChecked())
        m_ImportedTableData->GetColumn(m_CurrentColumnIdx).SetProperty("text_separator", "");
}


///
/// Options for handling blank fields
///
void CAttribTableColumnIdPanel::OnIgnoreBlanksBtnSelected( wxCommandEvent& event )
{
    if (event.IsChecked())
        m_ImportedTableData->GetColumn(m_CurrentColumnIdx).SetProperty("blanks", "ignore");
}

void CAttribTableColumnIdPanel::OnEraseExistingValueBtnSelected( wxCommandEvent& event )
{
    if (event.IsChecked())
        m_ImportedTableData->GetColumn(m_CurrentColumnIdx).SetProperty("blanks", "erase");
}



///
/// Text Update rules (append, prefix, add or ignore)
///
void CAttribTableColumnIdPanel::OnReplaceBtnSelected( wxCommandEvent& event )
{
    if (event.IsChecked())
        m_ImportedTableData->GetColumn(m_CurrentColumnIdx).SetProperty("update_rule", "replace");
}
void CAttribTableColumnIdPanel::OnAppendBtnSelected( wxCommandEvent& event )
{
    if (event.IsChecked())
        m_ImportedTableData->GetColumn(m_CurrentColumnIdx).SetProperty("update_rule", "append");
}

void CAttribTableColumnIdPanel::OnPrefixBtnSelected( wxCommandEvent& event )
{
    if (event.IsChecked())
        m_ImportedTableData->GetColumn(m_CurrentColumnIdx).SetProperty("update_rule", "prefix");
}

void CAttribTableColumnIdPanel::OnIgnoreBtnSelected( wxCommandEvent& event )
{
    if (event.IsChecked())
        m_ImportedTableData->GetColumn(m_CurrentColumnIdx).SetProperty("update_rule", "ignore");
}

void CAttribTableColumnIdPanel::OnAddQualBtnSelected( wxCommandEvent& event )
{
    if (event.IsChecked())
        m_ImportedTableData->GetColumn(m_CurrentColumnIdx).SetProperty("update_rule", "add_new_qual");
}

void CAttribTableColumnIdPanel::OnPageChanged(wxBookCtrlEvent& event)
{
    x_RefreshParentWnd();
}


////////////////////////////////////////////////////////////////////////////

IMPLEMENT_DYNAMIC_CLASS( CLongMessageDialog, wxDialog )


/*!
 * CLongMessageDialog event table definition
 */

BEGIN_EVENT_TABLE( CLongMessageDialog, wxDialog )
  EVT_BUTTON( wxID_YES, CLongMessageDialog::OnYesClick )
  EVT_BUTTON( wxID_NO, CLongMessageDialog::OnNoClick )
  EVT_CLOSE( CLongMessageDialog::OnClose )
END_EVENT_TABLE()


/*!
 * CLongMessageDialog constructors
 */

CLongMessageDialog::CLongMessageDialog()
{
    Init();
}

CLongMessageDialog::CLongMessageDialog( wxWindow* parent, wxString msg,
    wxWindowID id, const wxString& caption, const wxPoint& pos, const wxSize& size, long style )
    : m_Text(msg)
{
    Init();
    Create(parent, id, caption, pos, size, style);
}


/*!
 * CLongMessageDialog creator
 */

bool CLongMessageDialog::Create( wxWindow* parent, wxWindowID id, const wxString& caption, const wxPoint& pos, const wxSize& size, long style )
{
////@begin CLongMessageDialog creation
    SetExtraStyle(wxWS_EX_BLOCK_EVENTS);
    wxDialog::Create( parent, id, caption, pos, size, style );
    CreateControls();
    if (GetSizer())
    {
        GetSizer()->SetSizeHints(this);
    }
    Centre();
////@end CLongMessageDialog creation
    return true;
}


/*!
 * CLongMessageDialog destructor
 */

CLongMessageDialog::~CLongMessageDialog()
{
}


/*!
 * Member initialisation
 */

void CLongMessageDialog::Init()
{
}


/*!
 * Control creation for CLongMessageDialog
 */

void CLongMessageDialog::CreateControls()
{    
    CLongMessageDialog* itemDialog1 = this;

    wxBoxSizer* itemBoxSizer2 = new wxBoxSizer(wxVERTICAL);
    itemDialog1->SetSizer(itemBoxSizer2);

    wxRichTextCtrl *RTCtrl = new wxRichTextCtrl( itemDialog1, wxID_ANY, m_Text, wxDefaultPosition, wxSize(360, 400), wxRE_MULTILINE | wxRE_READONLY );
    //RTCtrl->SetFont(wxFont(10, wxSWISS, wxNORMAL, wxNORMAL, false, wxT("Consolas")));
    itemBoxSizer2->Add(RTCtrl, 1, wxGROW|wxALL, 5);

    wxSizer* itemBoxSizer4 = itemDialog1->CreateButtonSizer(wxYES | wxNO);
    itemBoxSizer2->Add(itemBoxSizer4, 0, wxGROW|wxALL, 5);
}

/*!
 * Should we show tooltips?
 */

bool CLongMessageDialog::ShowToolTips()
{
    return true;
}

/*!
 * Get bitmap resources
 */

wxBitmap CLongMessageDialog::GetBitmapResource( const wxString& name )
{
    // Bitmap retrieval
////@begin CLongMessageDialog bitmap retrieval
    wxUnusedVar(name);
    return wxNullBitmap;
////@end CLongMessageDialog bitmap retrieval
}

/*!
 * Get icon resources
 */

wxIcon CLongMessageDialog::GetIconResource( const wxString& name )
{
    // Icon retrieval
////@begin CLongMessageDialog icon retrieval
    wxUnusedVar(name);
    return wxNullIcon;
////@end CLongMessageDialog icon retrieval
}

void CLongMessageDialog::OnClose(wxCloseEvent& event)
{
    if (IsModal())
    {
        EndModal(wxNO);
    }
    else
    {
        event.Skip();
    }
}

void CLongMessageDialog::OnYesClick( wxCommandEvent& event )
{
    if (IsModal())
    {
        EndModal(wxYES);
    }
    else
    {
        Destroy();
    }
}

void CLongMessageDialog::OnNoClick( wxCommandEvent& event )
{
    if (IsModal())
    {
        EndModal(wxNO);
    }
    else
    {
        Destroy();
    }
}


END_NCBI_SCOPE

