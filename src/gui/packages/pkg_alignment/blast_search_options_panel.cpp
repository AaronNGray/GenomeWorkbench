/*  $Id: blast_search_options_panel.cpp 39666 2017-10-25 16:01:13Z katargir $
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
 * Authors: Andrey Yazhuk
 *
 * File Description:
 *
 */

#include <ncbi_pch.hpp>

#include <gui/packages/pkg_alignment/blast_search_options_panel.hpp>

#include <gui/packages/pkg_alignment/blast_search_params.hpp>
#include <gui/packages/pkg_alignment/blast_db_dialog.hpp>
#include <gui/packages/pkg_alignment/net_blast_ui_data_source.hpp>

#include <gui/widgets/object_list/object_list_widget.hpp>
#include <gui/widgets/loaders/lblast_params_panel.hpp>

#include <gui/widgets/wx/wx_utils.hpp>
#include <gui/widgets/wx/ind_progress_bar.hpp>
#include <gui/widgets/wx/sys_path.hpp>
#include <gui/objutils/blast_databases.hpp>

#include <wx/sizer.h>
#include <wx/image.h>
#include <wx/stattext.h>
#include <wx/button.h>
#include <wx/combobox.h>
#include <wx/choice.h>
#include <wx/radiobut.h>
#include <wx/statbox.h>
#include <wx/msgdlg.h>

////@begin XPM images
////@end XPM images


BEGIN_NCBI_SCOPE


IMPLEMENT_DYNAMIC_CLASS( CBLASTSearchOptionsPanel, CAlgoToolManagerParamsPanel )

BEGIN_EVENT_TABLE( CBLASTSearchOptionsPanel, CAlgoToolManagerParamsPanel )

////@begin CBLASTSearchOptionsPanel event table entries
    EVT_RADIOBUTTON( ID_NUC_RADIO, CBLASTSearchOptionsPanel::OnNucRadioSelected )

    EVT_RADIOBUTTON( ID_PROT_RADIO, CBLASTSearchOptionsPanel::OnProtRadioSelected )

    EVT_CHOICE( ID_QUERY_SEQ_SET, CBLASTSearchOptionsPanel::OnQuerySequenceSetSelected )

    EVT_COMBOBOX( ID_PROGRAM_COMBO, CBLASTSearchOptionsPanel::OnProgramComboSelected )

    EVT_RADIOBUTTON( ID_RADIOBUTTON, CBLASTSearchOptionsPanel::OnSubjectSequencesSelected )

    EVT_RADIOBUTTON( ID_RADIOBUTTON2, CBLASTSearchOptionsPanel::OnSubjectNCBIDBSelected )

    EVT_RADIOBUTTON( ID_RADIOBUTTON1, CBLASTSearchOptionsPanel::OnSubjectLocalDBSelected )

    EVT_CHOICE( ID_SUBJECT_SEQ_SET, CBLASTSearchOptionsPanel::OnSubjectSequenceSetSelected )

    EVT_COMBOBOX( ID_DB_COMBO, CBLASTSearchOptionsPanel::OnDatabaseComboSelected )

    EVT_BUTTON( ID_SELECT_NCBI_DB, CBLASTSearchOptionsPanel::OnBrowseDatabases )

    EVT_BUTTON( ID_SELECT_LOCAL_DB_BTN, CBLASTSearchOptionsPanel::OnSelectLocalDB )

    EVT_CHECKBOX( ID_CHECKBOX, CBLASTSearchOptionsPanel::OnLocalDBLoaderClick )

////@end CBLASTSearchOptionsPanel event table entries

    EVT_BUTTON( ID_DBS_TRY_AGAIN, CBLASTSearchOptionsPanel::OnTryAgain )
    EVT_TIMER(-1, CBLASTSearchOptionsPanel::OnTimer)

END_EVENT_TABLE()


CBLASTSearchOptionsPanel::CBLASTSearchOptionsPanel()
    : m_DataSource(NULL)
    , m_NucObjects(NULL)
    , m_ProtObjects(NULL)
    , m_Params(NULL)
    , m_Timer(this)
    , m_DBLoadState(CBLASTDatabases::eInitial)
{
    Init();
}


CBLASTSearchOptionsPanel::CBLASTSearchOptionsPanel( wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style )
    : m_Timer(this)
    , m_DBLoadState(CBLASTDatabases::eInitial)
{
    Init();
    Create(parent, id, pos, size, style);
}


bool CBLASTSearchOptionsPanel::Create( wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style )
{
    wxPanel::Create( parent, id, pos, size, style );

    Hide();

    CreateControls();
    if (GetSizer())
    {
        GetSizer()->SetSizeHints(this);
    }

    return true;
}


CBLASTSearchOptionsPanel::~CBLASTSearchOptionsPanel()
{
////@begin CBLASTSearchOptionsPanel destruction
////@end CBLASTSearchOptionsPanel destruction
}

void CBLASTSearchOptionsPanel::Setup(CNetBLASTUIDataSource& ds,
const string& tool_name)
{
    m_DataSource = &ds;
    m_ToolName = tool_name;
}

void CBLASTSearchOptionsPanel::SetParams(CBLASTParams* params,
                                         map<string, TConstScopedObjects>* nuc_objects,
                                         map<string, TConstScopedObjects>* prot_objects)
{
    m_Params = params;

    _ASSERT(nuc_objects  &&  prot_objects);

    m_NucObjects = nuc_objects;
    m_ProtObjects = prot_objects;

}


void CBLASTSearchOptionsPanel::Init()
{
////@begin CBLASTSearchOptionsPanel member initialisation
    m_NucRadio = NULL;
    m_ProtRadio = NULL;
    m_QuerySeqSetStatic = NULL;
    m_QuerySeqSetCombo = NULL;
    m_QueryListCtrl = NULL;
    m_ProgramCombo = NULL;
    m_LocalSizer = NULL;
    m_SubjectTypeSeqsCtrl = NULL;
    m_SubjectTypeNCBICtrl = NULL;
    m_SubjectTypeLDBCtrl = NULL;
    m_SubjectSeqSetStatic = NULL;
    m_SubjectSeqSetCombo = NULL;
    m_LocalSeqsSizer = NULL;
    m_SubjectListCtrl = NULL;
    m_NCBIDBSizer = NULL;
    m_DBSizer = NULL;
    m_DatabaseCombo = NULL;
    m_BrowseDBBtn = NULL;
    m_QueryCombo = NULL;
    m_LocalDBSizer = NULL;
    m_LocalDBCtrl = NULL;
    m_LocalDBLoaderCtrl = NULL;
////@end CBLASTSearchOptionsPanel member initialisation

    m_DataSource = NULL;
    m_ToolName.clear();

    m_NucObjects = NULL;
    m_ProtObjects = NULL;

    m_Params = NULL;

    m_DBLoadState = CBLASTDatabases::eInitial;
}


void CBLASTSearchOptionsPanel::CreateControls()
{
////@begin CBLASTSearchOptionsPanel content construction
    CBLASTSearchOptionsPanel* itemCAlgoToolManagerParamsPanel1 = this;

    wxBoxSizer* itemBoxSizer2 = new wxBoxSizer(wxVERTICAL);
    itemCAlgoToolManagerParamsPanel1->SetSizer(itemBoxSizer2);

    wxStaticBox* itemStaticBoxSizer3Static = new wxStaticBox(itemCAlgoToolManagerParamsPanel1, wxID_ANY, _("Query Sequences"));
    wxStaticBoxSizer* itemStaticBoxSizer3 = new wxStaticBoxSizer(itemStaticBoxSizer3Static, wxVERTICAL);
    itemBoxSizer2->Add(itemStaticBoxSizer3, 1, wxGROW|wxALL, 5);

    wxBoxSizer* itemBoxSizer4 = new wxBoxSizer(wxHORIZONTAL);
    itemStaticBoxSizer3->Add(itemBoxSizer4, 0, wxALIGN_RIGHT|wxALL, 5);

    wxBoxSizer* itemBoxSizer5 = new wxBoxSizer(wxHORIZONTAL);
    itemStaticBoxSizer3->Add(itemBoxSizer5, 0, wxGROW, 5);

    m_NucRadio = new wxRadioButton( itemCAlgoToolManagerParamsPanel1, ID_NUC_RADIO, _("Nucleotides"), wxDefaultPosition, wxDefaultSize, wxRB_GROUP );
    m_NucRadio->SetValue(true);
    itemBoxSizer5->Add(m_NucRadio, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    m_ProtRadio = new wxRadioButton( itemCAlgoToolManagerParamsPanel1, ID_PROT_RADIO, _("Proteins"), wxDefaultPosition, wxDefaultSize, 0 );
    m_ProtRadio->SetValue(false);
    itemBoxSizer5->Add(m_ProtRadio, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    itemBoxSizer5->Add(7, 8, 1, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    m_QuerySeqSetStatic = new wxStaticText( itemCAlgoToolManagerParamsPanel1, wxID_STATIC, _("Sequence set:"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer5->Add(m_QuerySeqSetStatic, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxArrayString m_QuerySeqSetComboStrings;
    m_QuerySeqSetCombo = new wxChoice( itemCAlgoToolManagerParamsPanel1, ID_QUERY_SEQ_SET, wxDefaultPosition, wxDefaultSize, m_QuerySeqSetComboStrings, 0 );
    itemBoxSizer5->Add(m_QuerySeqSetCombo, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    m_QueryListCtrl = new CObjectListWidget( itemCAlgoToolManagerParamsPanel1, ID_LIST, wxDefaultPosition, itemCAlgoToolManagerParamsPanel1->ConvertDialogToPixels(wxSize(350, 75)), wxLC_REPORT );
    itemStaticBoxSizer3->Add(m_QueryListCtrl, 1, wxGROW|wxALL, 5);

    wxBoxSizer* itemBoxSizer12 = new wxBoxSizer(wxHORIZONTAL);
    itemBoxSizer2->Add(itemBoxSizer12, 0, wxGROW|wxALL, 5);

    wxStaticText* itemStaticText13 = new wxStaticText( itemCAlgoToolManagerParamsPanel1, wxID_STATIC, _("Program:"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer12->Add(itemStaticText13, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxArrayString m_ProgramComboStrings;
    m_ProgramCombo = new wxComboBox( itemCAlgoToolManagerParamsPanel1, ID_PROGRAM_COMBO, wxEmptyString, wxDefaultPosition, wxDefaultSize, m_ProgramComboStrings, wxCB_READONLY );
    itemBoxSizer12->Add(m_ProgramCombo, 1, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxStaticBox* itemStaticBoxSizer15Static = new wxStaticBox(itemCAlgoToolManagerParamsPanel1, wxID_ANY, _("Subject"));
    m_LocalSizer = new wxStaticBoxSizer(itemStaticBoxSizer15Static, wxVERTICAL);
    itemBoxSizer2->Add(m_LocalSizer, 1, wxGROW|wxALL, 0);

    wxBoxSizer* itemBoxSizer16 = new wxBoxSizer(wxHORIZONTAL);
    m_LocalSizer->Add(itemBoxSizer16, 0, wxGROW|wxALL, 0);

    m_SubjectTypeSeqsCtrl = new wxRadioButton( itemCAlgoToolManagerParamsPanel1, ID_RADIOBUTTON, _("Sequences"), wxDefaultPosition, wxDefaultSize, wxRB_GROUP );
    m_SubjectTypeSeqsCtrl->SetValue(true);
    itemBoxSizer16->Add(m_SubjectTypeSeqsCtrl, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    m_SubjectTypeNCBICtrl = new wxRadioButton( itemCAlgoToolManagerParamsPanel1, ID_RADIOBUTTON2, _("NCBI Database"), wxDefaultPosition, wxDefaultSize, 0 );
    m_SubjectTypeNCBICtrl->SetValue(false);
    itemBoxSizer16->Add(m_SubjectTypeNCBICtrl, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    m_SubjectTypeLDBCtrl = new wxRadioButton( itemCAlgoToolManagerParamsPanel1, ID_RADIOBUTTON1, _("Local Database"), wxDefaultPosition, wxDefaultSize, 0 );
    m_SubjectTypeLDBCtrl->SetValue(false);
    itemBoxSizer16->Add(m_SubjectTypeLDBCtrl, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    itemBoxSizer16->Add(7, 8, 1, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    m_SubjectSeqSetStatic = new wxStaticText( itemCAlgoToolManagerParamsPanel1, wxID_STATIC, _("Sequence set:"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer16->Add(m_SubjectSeqSetStatic, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxArrayString m_SubjectSeqSetComboStrings;
    m_SubjectSeqSetCombo = new wxChoice( itemCAlgoToolManagerParamsPanel1, ID_SUBJECT_SEQ_SET, wxDefaultPosition, wxDefaultSize, m_SubjectSeqSetComboStrings, 0 );
    itemBoxSizer16->Add(m_SubjectSeqSetCombo, 0, wxALIGN_CENTER_VERTICAL|wxLEFT|wxRIGHT, 5);

    m_LocalSeqsSizer = new wxBoxSizer(wxVERTICAL);
    m_LocalSizer->Add(m_LocalSeqsSizer, 1, wxGROW, 0);

    m_SubjectListCtrl = new CObjectListWidget( itemCAlgoToolManagerParamsPanel1, ID_SUBJ_LIST, wxDefaultPosition, itemCAlgoToolManagerParamsPanel1->ConvertDialogToPixels(wxSize(350, 75)), wxLC_REPORT );
    m_LocalSeqsSizer->Add(m_SubjectListCtrl, 1, wxGROW|wxALL, 5);

    m_NCBIDBSizer = new wxFlexGridSizer(0, 2, 0, 0);
    m_LocalSizer->Add(m_NCBIDBSizer, 0, wxGROW|wxALL, 5);

    wxStaticText* itemStaticText26 = new wxStaticText( itemCAlgoToolManagerParamsPanel1, wxID_STATIC, _("Database:"), wxDefaultPosition, wxDefaultSize, 0 );
    m_NCBIDBSizer->Add(itemStaticText26, 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    m_DBSizer = new wxBoxSizer(wxHORIZONTAL);
    m_NCBIDBSizer->Add(m_DBSizer, 0, wxGROW|wxALIGN_CENTER_VERTICAL, 5);

    wxArrayString m_DatabaseComboStrings;
    m_DatabaseCombo = new wxComboBox( itemCAlgoToolManagerParamsPanel1, ID_DB_COMBO, wxEmptyString, wxDefaultPosition, wxDefaultSize, m_DatabaseComboStrings, wxCB_READONLY );
    m_DBSizer->Add(m_DatabaseCombo, 1, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    m_BrowseDBBtn = new wxBitmapButton( itemCAlgoToolManagerParamsPanel1, ID_SELECT_NCBI_DB, itemCAlgoToolManagerParamsPanel1->GetBitmapResource(wxT("menu::open")), wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW );
    m_BrowseDBBtn->SetHelpText(_("Select NCBI DB"));
    if (CBLASTSearchOptionsPanel::ShowToolTips())
        m_BrowseDBBtn->SetToolTip(_("Select NCBI DB"));
    m_DBSizer->Add(m_BrowseDBBtn, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxStaticText* itemStaticText30 = new wxStaticText( itemCAlgoToolManagerParamsPanel1, wxID_STATIC, _("Entrez Query:"), wxDefaultPosition, wxDefaultSize, 0 );
    m_NCBIDBSizer->Add(itemStaticText30, 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxArrayString m_QueryComboStrings;
    m_QueryCombo = new wxComboBox( itemCAlgoToolManagerParamsPanel1, ID_ENTREZ_COMBO, wxEmptyString, wxDefaultPosition, wxDefaultSize, m_QueryComboStrings, wxCB_DROPDOWN );
    m_NCBIDBSizer->Add(m_QueryCombo, 0, wxGROW|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    m_NCBIDBSizer->AddGrowableCol(1);

    m_LocalDBSizer = new wxBoxSizer(wxVERTICAL);
    m_LocalSizer->Add(m_LocalDBSizer, 0, wxGROW, 5);

    wxBoxSizer* itemBoxSizer33 = new wxBoxSizer(wxHORIZONTAL);
    m_LocalDBSizer->Add(itemBoxSizer33, 0, wxGROW|wxALL, 0);

    wxStaticText* itemStaticText34 = new wxStaticText( itemCAlgoToolManagerParamsPanel1, wxID_STATIC, _("Database:"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer33->Add(itemStaticText34, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    m_LocalDBCtrl = new wxTextCtrl( itemCAlgoToolManagerParamsPanel1, ID_TEXTCTRL, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer33->Add(m_LocalDBCtrl, 1, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxBitmapButton* itemBitmapButton36 = new wxBitmapButton( itemCAlgoToolManagerParamsPanel1, ID_SELECT_LOCAL_DB_BTN, itemCAlgoToolManagerParamsPanel1->GetBitmapResource(wxT("menu::open")), wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW );
    itemBitmapButton36->SetHelpText(_("Select Local DB"));
    if (CBLASTSearchOptionsPanel::ShowToolTips())
        itemBitmapButton36->SetToolTip(_("Select Local DB"));
    itemBoxSizer33->Add(itemBitmapButton36, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    m_LocalDBLoaderCtrl = new wxCheckBox( itemCAlgoToolManagerParamsPanel1, ID_CHECKBOX, _("Add Local BLAST DB data loader to the project"), wxDefaultPosition, wxDefaultSize, 0 );
    m_LocalDBLoaderCtrl->SetValue(false);
    m_LocalDBSizer->Add(m_LocalDBLoaderCtrl, 0, wxALIGN_LEFT|wxALL, 5);

////@end CBLASTSearchOptionsPanel content construction

    wxStaticText* failedText = new wxStaticText( this, ID_DBS_FAILED, wxT("Failed to load..."), wxDefaultPosition, wxDefaultSize, 0 );

    wxFont bold(wxNORMAL_FONT->GetPointSize(), wxNORMAL_FONT->GetFamily(), wxNORMAL_FONT->GetStyle(), wxFONTWEIGHT_BOLD, wxNORMAL_FONT->GetUnderlined(), wxNORMAL_FONT->GetFaceName());
    failedText->SetFont(bold);

    failedText->SetForegroundColour(*wxRED);
    m_DBSizer->Add(failedText, 1, wxALIGN_CENTER_VERTICAL|wxALL, 5);
    m_DBSizer->Hide(failedText);

    wxButton* tryAgainBtn = new wxButton( this, ID_DBS_TRY_AGAIN, _("Try Again"), wxDefaultPosition, wxDefaultSize, 0 );
    m_DBSizer->Add(tryAgainBtn, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);
    m_DBSizer->Hide(tryAgainBtn);

    wxStaticText* loadingText = new wxStaticText( this, ID_DBS_LOADING_TEXT, wxT("Loading..."), wxDefaultPosition, wxDefaultSize, 0 );
    loadingText->SetFont(bold);
    loadingText->SetForegroundColour(*wxBLACK);
    m_DBSizer->Add(loadingText, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);
    m_DBSizer->Hide(loadingText);

    CIndProgressBar* progress = new CIndProgressBar(this, ID_DBS_LOADING_PROGRESS, wxDefaultPosition, 100);
    m_DBSizer->Add(progress, 1, wxALIGN_CENTER_VERTICAL|wxALL, 5);
    m_DBSizer->Hide(progress);

    m_LocalSizer->Show(m_LocalSeqsSizer, true);
    m_LocalSizer->Hide(m_LocalDBSizer, true);
    m_LocalSizer->Hide(m_NCBIDBSizer, true);
}


bool CBLASTSearchOptionsPanel::ShowToolTips()
{
    return true;
}


wxBitmap CBLASTSearchOptionsPanel::GetBitmapResource( const wxString& name )
{
    return wxArtProvider::GetBitmap(name);
}


wxIcon CBLASTSearchOptionsPanel::GetIconResource( const wxString& name )
{
    // Icon retrieval
////@begin CBLASTSearchOptionsPanel icon retrieval
    wxUnusedVar(name);
    return wxNullIcon;
////@end CBLASTSearchOptionsPanel icon retrieval
}

static const char* kTableTag = "Table";

void CBLASTSearchOptionsPanel::SaveSettings() const
{
    if( ! m_RegPath.empty())   {
        CGuiRegistry& gui_reg = CGuiRegistry::GetInstance();
        string reg_path = CGuiRegistryUtil::MakeKey(m_RegPath, kTableTag);

        CRegistryWriteView query_table_view = gui_reg.GetWriteView(
            reg_path + ".QueryTable" 
        );
        m_QueryListCtrl->SaveTableSettings(query_table_view);

        CRegistryWriteView subject_table_view = gui_reg.GetWriteView(
            reg_path + ".SubjectTable" 
        );
        m_SubjectListCtrl->SaveTableSettings(subject_table_view);
    }
}


void CBLASTSearchOptionsPanel::LoadSettings()
{
    if( ! m_RegPath.empty())   {
        CGuiRegistry& gui_reg = CGuiRegistry::GetInstance();
        string reg_path = CGuiRegistryUtil::MakeKey(m_RegPath, kTableTag);

        CRegistryReadView query_table_view = gui_reg.GetReadView(
            reg_path + ".QueryTable" 
        );
        m_QueryListCtrl->LoadTableSettings(query_table_view);

        CRegistryReadView subject_table_view = gui_reg.GetReadView(
            reg_path + ".SubjectTable" 
        );
        m_SubjectListCtrl->LoadTableSettings(subject_table_view);
    }
}

bool CBLASTSearchOptionsPanel::TransferDataToWindow()
{
    bool nuc_subj = m_Params->IsNucInput();
    m_NucRadio->SetValue(nuc_subj);
    m_ProtRadio->SetValue(!nuc_subj);

    CBLASTParams::ESubjectType subjectType = m_Params->GetSubjectType();
    m_SubjectTypeSeqsCtrl->SetValue(subjectType == CBLASTParams::eSequences);
    m_SubjectTypeNCBICtrl->SetValue(subjectType == CBLASTParams::eNCBIDB);
    m_SubjectTypeLDBCtrl->SetValue(subjectType == CBLASTParams::eLocalDB);

    x_UpdateSubjectType();

    m_LocalDBLoaderCtrl->SetValue(m_Params->GetLocalDBLoader());

    x_CheckDBLoaded();

    x_OnInputTypeChanged(nuc_subj);

    wxString query = ToWxString(m_Params->GetEntrezQuery());
    m_QueryCombo->SetValue(query);

    return wxPanel::TransferDataToWindow();
}

bool CBLASTSearchOptionsPanel::TransferDataFromWindow()
{
    if (!wxPanel::TransferDataFromWindow())
        return false;

    m_Params->GetSeqLocs().clear();
    m_QueryListCtrl->GetSelection(m_Params->GetSeqLocs());

    if (m_Params->GetSeqLocs().empty())  {
        wxMessageBox(wxT("Please select at least one query sequence!"),
            wxT("BALST Tool Error"),
            wxOK | wxICON_ERROR);
        m_QueryListCtrl->SetFocus();
        return false;
    }

    CBLASTParams::ESubjectType subjectType = m_Params->GetSubjectType();

    m_Params->GetSubjSeqLocs().clear();
    if (subjectType == CBLASTParams::eNCBIDB) {
        if (!CBLASTDatabases::GetInstance().Loaded()) {
            wxMessageBox(wxT("BLAST database must be selected.\n")
                wxT("Please, wait till databases are loaded."),
                wxT("BALST Tool Error"),
                wxOK | wxICON_ERROR);
            return false;
        }

        m_Params->GetEntrezQuery() = ToStdString(m_QueryCombo->GetValue());
        m_Params->GetCurrDatabase() = ToStdString(m_DatabaseCombo->GetStringSelection());
        if (m_Params->GetCurrDatabase().empty()) {
            wxMessageBox(wxT("Please select a database!"),
                wxT("BALST Tool Error"),
                wxOK | wxICON_ERROR);
            return false;
        }
    }
    else if (subjectType == CBLASTParams::eLocalDB) {
        wxString dbText = m_LocalDBCtrl->GetValue();
        if (dbText.empty()) {
            wxMessageBox(wxT("Please, select a local database."), wxT("Error"),
                wxOK | wxICON_ERROR, this);
            m_LocalDBCtrl->SetFocus();
            return false;
        }

        bool subjProteins = !m_Params->IsSubjNucInput();
        if (!CLBLASTParamsPanel::ValidateLocalDatabase(dbText, subjProteins)) {
            wxMessageBox(wxT("Invalid local database: \"") + dbText + wxT("\"."), wxT("Error"),
                wxOK | wxICON_ERROR, this);
            m_LocalDBCtrl->SetFocus();
            return false;
        }

        string db(dbText.ToUTF8());

        if (subjProteins)
            m_Params->SetLocalProtDB(db);
        else
            m_Params->SetLocalNucDB(db);
    }
    else {
        m_SubjectListCtrl->GetSelection(m_Params->GetSubjSeqLocs());
        if (m_Params->GetSubjSeqLocs().empty()) {
            wxMessageBox(wxT("No subject sequences selected."), wxT("Error"),
                wxOK | wxICON_ERROR, this);
            return false;
        }
    }

    wxString program = m_ProgramCombo->GetStringSelection();
    m_Params->GetCurrProgram() =
        CBLASTParams::GetProgramByLabel(ToStdString(program));

    m_Params->IsNucInput() = m_NucRadio->GetValue();
    return true;
}


void CBLASTSearchOptionsPanel::OnProtRadioSelected(wxCommandEvent& event)
{
    x_OnInputTypeChanged(false);
}


void CBLASTSearchOptionsPanel::OnNucRadioSelected(wxCommandEvent& event)
{
    x_OnInputTypeChanged(true);
}

static const char* kSelectedSequences = "Selected sequences";

void CBLASTSearchOptionsPanel::x_OnInputTypeChanged(bool nuc_input)
{
    m_Params->IsNucInput() = nuc_input;

    wxArrayString programLabels;

    vector<string> labels;
    CBLASTParams::GetProgramLabels(nuc_input, labels);
    ToArrayString(labels, programLabels);

    m_ProgramCombo->Clear();
    m_ProgramCombo->Append(programLabels);

    wxString program = ToWxString(CBLASTParams::GetProgramLabel
                           (m_Params->GetCurrProgram()));
    m_ProgramCombo->SetValue(program);

    x_UpdateQuerySeqSet();
    x_FillQueryList();

    x_UpdateSubjects();
}


static bool s_UseSets(map<string, TConstScopedObjects>& objects)
{
    if (objects.size() <= 1)
        return false;

    size_t count = 0;
    for (auto& i : objects)
        count += i.second.size();
    return (count > 10);
}

void CBLASTSearchOptionsPanel::x_UpdateQuerySeqSet()
{
    map<string, TConstScopedObjects>& objects = m_Params->IsNucInput() ? *m_NucObjects : *m_ProtObjects;
    if (s_UseSets(objects)) {
        m_QuerySeqSetStatic->Show(true);
        m_QuerySeqSetCombo->Show(true);
        m_QuerySeqSetCombo->Clear();
        for (const auto& i : objects) {
            if (i.first == kSelectedSequences)
                m_QuerySeqSetCombo->Insert(ToWxString(kSelectedSequences), 0);
            else
                m_QuerySeqSetCombo->Append(ToWxString(i.first));
        }

        int sel = 0;
        if (!m_CurrentQuerySet.empty()) {
            sel = m_QuerySeqSetCombo->FindString(ToWxString(m_CurrentQuerySet));
            if (sel == wxNOT_FOUND) sel = 0;
        }

        m_QuerySeqSetCombo->SetSelection(sel);
        m_CurrentQuerySet = ToStdString(m_QuerySeqSetCombo->GetString(sel));
    }
    else {
        m_QuerySeqSetStatic->Show(false);
        m_QuerySeqSetCombo->Show(false);
        m_QuerySeqSetCombo->Clear();
    }
}


void CBLASTSearchOptionsPanel::x_FillQueryList()
{
    map<string, TConstScopedObjects>& objects = m_Params->IsNucInput() ? *m_NucObjects : *m_ProtObjects;

    if (s_UseSets(objects)) {
        map<string, TConstScopedObjects>::iterator it = objects.find(m_CurrentQuerySet);

        if (it != objects.end())
            m_QueryListCtrl->SetObjects(it->second);
        else {
            TConstScopedObjects empty;
            m_QueryListCtrl->SetObjects(empty);
        }

        if (m_QueryListCtrl->GetItemCount() <= 100) {
            m_QueryListCtrl->SelectAll();
        }
        else if (m_SubjectListCtrl->GetItemCount() > 0) {
            m_QueryListCtrl->Select(0);
        }
    }
    else {
        TConstScopedObjects allObjects;
        for (auto& i : objects)
            for (auto& o : i.second)
                allObjects.push_back(o);
        m_QueryListCtrl->SetObjects(allObjects);
        m_QueryListCtrl->SelectAll();
    }
}

void CBLASTSearchOptionsPanel::OnQuerySequenceSetSelected(wxCommandEvent& event)
{
    string sequenceSet = ToStdString(event.GetString());
    if (sequenceSet != m_CurrentQuerySet) {
        m_CurrentQuerySet = sequenceSet;
        x_FillQueryList();
    }
}

void CBLASTSearchOptionsPanel::OnProgramComboSelected( wxCommandEvent& event )
{
    bool nuc_subj = m_Params->IsSubjNucInput();

    wxString program = m_ProgramCombo->GetStringSelection();
    m_Params->GetCurrProgram() =
        CBLASTParams::GetProgramByLabel(ToStdString(program));

    if (m_Params->IsSubjNucInput() != nuc_subj)
        x_UpdateSubjects();
}

void CBLASTSearchOptionsPanel::OnDatabaseComboSelected( wxCommandEvent& event )
{
    string database = ToStdString(m_DatabaseCombo->GetStringSelection());
    m_Params->GetCurrDatabase() = database;
    m_Params->GetCurrMRUDatabases().AddItem(database);
}

void CBLASTSearchOptionsPanel::x_UpdateSubjects()
{
    CBLASTParams::ESubjectType subjectType = m_Params->GetSubjectType();
    if (subjectType == CBLASTParams::eNCBIDB) {
        x_UpdateDBCombo();
    }
    else if (subjectType == CBLASTParams::eLocalDB) {
        wxString db = m_Params->IsSubjNucInput() ?
            wxString::FromUTF8(m_Params->GetLocalNucDB().c_str()) :
            wxString::FromUTF8(m_Params->GetLocalProtDB().c_str());
        m_LocalDBCtrl->SetValue(db);
    } else {
        x_UpdateSubjectSeqSet();
        x_FillSubjectList();
    }
}

void CBLASTSearchOptionsPanel::x_UpdateSubjectSeqSet()
{
    map<string, TConstScopedObjects>& objects = m_Params->IsSubjNucInput() ? *m_NucObjects : *m_ProtObjects;
    if (s_UseSets(objects)) {
        m_SubjectSeqSetStatic->Show(true);
        m_SubjectSeqSetCombo->Show(true);
        m_SubjectSeqSetCombo->Clear();

        for (const auto& i : objects) {
            if (i.first == kSelectedSequences)
                m_SubjectSeqSetCombo->Insert(ToWxString(kSelectedSequences), 0);
            else
                m_SubjectSeqSetCombo->Append(ToWxString(i.first));
        }

        int sel = 0;
        if (!m_CurrentSubjectSet.empty()) {
            sel = m_SubjectSeqSetCombo->FindString(ToWxString(m_CurrentSubjectSet));
            if (sel == wxNOT_FOUND) sel = 0;
        }

        m_SubjectSeqSetCombo->SetSelection(sel);
        m_CurrentSubjectSet = ToStdString(m_SubjectSeqSetCombo->GetString(sel));

    } else {
        m_SubjectSeqSetStatic->Show(false);
        m_SubjectSeqSetCombo->Show(false);
        m_SubjectSeqSetCombo->Clear();
    }
}


void CBLASTSearchOptionsPanel::x_FillSubjectList()
{
    map<string, TConstScopedObjects>& objects = m_Params->IsSubjNucInput() ? *m_NucObjects : *m_ProtObjects;

    if (s_UseSets(objects)) {
        map<string, TConstScopedObjects>::iterator it = objects.find(m_CurrentSubjectSet);

        if (it != objects.end())
            m_SubjectListCtrl->SetObjects(it->second);
        else {
            TConstScopedObjects empty;
            m_SubjectListCtrl->SetObjects(empty);
        }

        if (m_SubjectListCtrl->GetItemCount() <= 100) {
            m_SubjectListCtrl->SelectAll();
        }
        else if (m_SubjectListCtrl->GetItemCount() > 0) {
            m_SubjectListCtrl->Select(0);
        }
    }
    else {
        TConstScopedObjects allObjects;
        for (auto& i : objects)
            for (auto& o : i.second)
                allObjects.push_back(o);
        m_SubjectListCtrl->SetObjects(allObjects);
        m_SubjectListCtrl->SelectAll();
    }
}

void CBLASTSearchOptionsPanel::OnSubjectSequenceSetSelected(wxCommandEvent& event)
{
    string sequenceSet = ToStdString(event.GetString());
    if (sequenceSet != m_CurrentSubjectSet) {
        m_CurrentSubjectSet = sequenceSet;
        x_FillSubjectList();
    }
}

static const char* kDBBrowserSection = ".DB Browser";

void CBLASTSearchOptionsPanel::OnBrowseDatabases( wxCommandEvent& event )
{
    CBLASTDatabases& dbs = CBLASTDatabases::GetInstance();
    if (!dbs.Loaded())
        return;

    wxString database = m_DatabaseCombo->GetStringSelection();
    vector<string> databases;
    databases.push_back(ToStdString(database));

    CBLAST_DB_Dialog dlg(this);
    dlg.SetRegistryPath(m_RegPath + kDBBrowserSection);

    dlg.SetToolName(m_ToolName);
    dlg.SelectDatabases(databases);

    bool nuc_db = m_Params->IsSubjNucInput();
    const CBLASTDatabases::TDbMap& map = dbs.GetDbMap(nuc_db);
    dlg.SetDBMap(map);

    if(dlg.ShowModal() == wxID_OK)   {
        // User selected a database
        databases.clear();
        dlg.GetSelectedDatabases(databases);

        _ASSERT(databases.size() == 1);

        string db = databases[0];
        m_Params->GetCurrDatabase() = db;
        m_Params->GetCurrMRUDatabases().AddItem(db);

        x_UpdateDBCombo();
    }
}

void CBLASTSearchOptionsPanel::x_CheckDBLoaded()
{
    if (m_Params->GetSubjectType() != CBLASTParams::eNCBIDB)
        return;

    wxWindow* tryAgainBtn = FindWindow(ID_DBS_TRY_AGAIN);
    wxWindow* progress = FindWindow(ID_DBS_LOADING_PROGRESS);

    CBLASTDatabases& dbs = CBLASTDatabases::GetInstance();
    switch(dbs.GetState()) {
    case CBLASTDatabases::eInitial :
    case CBLASTDatabases::eFailed :
        if (m_DBLoadState != CBLASTDatabases::eFailed) {
            m_Timer.Stop();

            m_DBSizer->Show(FindWindow(ID_DBS_FAILED));
            m_DBSizer->Show(tryAgainBtn);

            m_DBSizer->Hide(FindWindow(ID_DBS_LOADING_TEXT));
            m_DBSizer->Hide(progress);

            m_DBSizer->Hide(m_DatabaseCombo);
            m_DBSizer->Hide(m_BrowseDBBtn);
            Layout();
            m_DBLoadState = CBLASTDatabases::eFailed;
        }
        break;
    case CBLASTDatabases::eLoading :
        if (m_DBLoadState != CBLASTDatabases::eLoading) {
            m_Timer.Start(300);

            int height = tryAgainBtn->GetSize().GetHeight();
            progress->SetMinSize(wxSize(1, height));

            m_DBSizer->Hide(FindWindow(ID_DBS_FAILED));
            m_DBSizer->Hide(tryAgainBtn);

            m_DBSizer->Show(FindWindow(ID_DBS_LOADING_TEXT));
            m_DBSizer->Show(progress);

            m_DBSizer->Hide(m_DatabaseCombo);
            m_DBSizer->Hide(m_BrowseDBBtn);
            Layout();
            m_DBLoadState = CBLASTDatabases::eLoading;
        }
        break;
    case CBLASTDatabases::eLoaded :
        if (m_DBLoadState != CBLASTDatabases::eLoaded) {
            m_Timer.Stop();

            m_DBSizer->Hide(FindWindow(ID_DBS_FAILED));
            m_DBSizer->Hide(tryAgainBtn);

            m_DBSizer->Hide(FindWindow(ID_DBS_LOADING_TEXT));
            m_DBSizer->Hide(progress);

            x_UpdateDBCombo();

            m_DBSizer->Show(m_DatabaseCombo);
            m_DBSizer->Show(m_BrowseDBBtn);
            Layout();
            m_DBLoadState = CBLASTDatabases::eLoaded;
        }
        break;
    }
}

void CBLASTSearchOptionsPanel::x_UpdateDBCombo()
{
    CBLASTDatabases& dbs = CBLASTDatabases::GetInstance();
    if (!dbs.Loaded())
        return;

    wxBusyCursor wait;

    blast::EProgram program = m_Params->GetCurrProgram();
    bool nuc_db = CBLASTParams::IsNucDatabase(program);

    const CBLASTDatabases::TDbMap& db_map = dbs.GetDbMap(nuc_db);

    CBLASTParams::TMRUDatabases& mru_dbs = m_Params->GetMRUDatabases(nuc_db);
    const CBLASTParams::TMRUDatabases::TItems& mruItems = mru_dbs.GetItems();

    list<string> newItems;
    ITERATE(CBLASTParams::TMRUDatabases::TItems, it, mruItems) {
        if (db_map.find(*it) != db_map.end())
            newItems.push_back(*it);
    }

    if (!newItems.empty())  {
        mru_dbs.SetItems(newItems);
    } else {
        const vector<string>& mru = dbs.GetDefaultMRU_DBs(nuc_db);
        newItems.assign(mru.begin(), mru.end());
        mru_dbs.SetItems(newItems);
    }

    string& db = m_Params->GetCurrDatabase();

    if (std::find(newItems.begin(), newItems.end(), db) == newItems.end()) {
        if (db_map.find(db) != db_map.end()) {
            mru_dbs.AddItem(db); // valid DB name - add to the MRU
        } else if (!mru_dbs.GetItems().empty()) {
            m_Params->GetDatabase(nuc_db) = *mru_dbs.GetItems().begin();
        }
    }

    wxArrayString arr;
    ITERATE(CMRUList<string>::TItems, it, mru_dbs.GetItems()) {
        const string& db_name = *it;
        arr.Add(ToWxString(db_name));
        LOG_POST(Info << "\t" << db_name);
    }
    m_DatabaseCombo->Clear();
    m_DatabaseCombo->Append(arr);

    m_DatabaseCombo->SetValue(ToWxString(m_Params->GetCurrDatabase()));
}

void CBLASTSearchOptionsPanel::OnTryAgain( wxCommandEvent& event )
{
    wxString homePath = CSysPath::ResolvePath(wxT("<home>"));
    wxString dbTreePath = CSysPath::ResolvePath(wxT("<std>/etc/blastdb-spec/blast-db-tree.asn"));
    CBLASTDatabases::GetInstance().Load(homePath.ToUTF8(), dbTreePath.ToUTF8());
    x_CheckDBLoaded();
}

void CBLASTSearchOptionsPanel::OnTimer(wxTimerEvent& event)
{
    x_CheckDBLoaded();
}

bool CBLASTSearchOptionsPanel::CanDoNext()
{
    return CBLASTDatabases::GetInstance().Loaded();
}

void CBLASTSearchOptionsPanel::RestoreDefaults()
{
    if (!m_Params)
        return;
    m_Params->ResetCurrParams();
    wxBusyCursor wait;
    TransferDataToWindow();
}

void CBLASTSearchOptionsPanel::x_UpdateSubjectType()
{
    CBLASTParams::ESubjectType subjectType = m_Params->GetSubjectType();
    if (subjectType == CBLASTParams::eNCBIDB) {
        m_SubjectSeqSetStatic->Show(false);
        m_SubjectSeqSetCombo->Show(false);
        m_SubjectSeqSetCombo->Clear();

        m_LocalSizer->Hide(m_LocalSeqsSizer, true);
        m_LocalSizer->Show(m_NCBIDBSizer, true);
        m_LocalSizer->Hide(m_LocalDBSizer, true);
        m_DBLoadState = CBLASTDatabases::eInitial;
        x_CheckDBLoaded();
    }
    else if (subjectType == CBLASTParams::eLocalDB) {
        m_SubjectSeqSetStatic->Show(false);
        m_SubjectSeqSetCombo->Show(false);
        m_SubjectSeqSetCombo->Clear();

        m_LocalSizer->Hide(m_LocalSeqsSizer, true);
        m_LocalSizer->Hide(m_NCBIDBSizer, true);
        m_LocalSizer->Show(m_LocalDBSizer, true);
    }
    else {
        x_UpdateSubjects();
        m_LocalSizer->Show(m_LocalSeqsSizer, true);
        m_LocalSizer->Hide(m_NCBIDBSizer, true);
        m_LocalSizer->Hide(m_LocalDBSizer, true);
    }
}

void CBLASTSearchOptionsPanel::OnSubjectSequencesSelected( wxCommandEvent& event )
{
    m_Params->SetSubjectType(CBLASTParams::eSequences);
    x_UpdateSubjectType();
    Layout();
}

void CBLASTSearchOptionsPanel::OnSubjectNCBIDBSelected(wxCommandEvent& event)
{
    m_Params->SetSubjectType(CBLASTParams::eNCBIDB);
    x_UpdateSubjectType();
    Layout();
}

void CBLASTSearchOptionsPanel::OnSubjectLocalDBSelected( wxCommandEvent& event )
{
    m_Params->SetSubjectType(CBLASTParams::eLocalDB);
    x_UpdateSubjectType();
    Layout();
}

void CBLASTSearchOptionsPanel::OnLocalDBLoaderClick(wxCommandEvent& event)
{
    m_Params->SetLocalDBLoader(event.IsChecked());
}

void CBLASTSearchOptionsPanel::OnSelectLocalDB( wxCommandEvent& event )
{
    wxString text = m_LocalDBCtrl->GetValue();
    bool subjProteins = !m_Params->IsSubjNucInput();

    if (CLBLASTParamsPanel::SelectDatabase(this, text, subjProteins)) {
        string db(text.ToUTF8());
        if (subjProteins)
            m_Params->SetLocalProtDB(db);
        else
            m_Params->SetLocalNucDB(db);

        m_LocalDBCtrl->SetValue(text);
    }
}

END_NCBI_SCOPE
