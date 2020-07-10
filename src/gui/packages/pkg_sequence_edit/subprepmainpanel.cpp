/*  $Id: subprepmainpanel.cpp 43676 2019-08-14 14:28:05Z asztalos $
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
 * Authors:  Colleen Bollin
 */


#include <ncbi_pch.hpp>

////@begin includes
////@end includes

#include <gui/widgets/edit/field_type_constants.hpp>
#include <gui/packages/pkg_sequence_edit/subprep_util.hpp>
#include <gui/packages/pkg_sequence_edit/subprep_panel.hpp>
#include <gui/packages/pkg_sequence_edit/molinfoedit_util.hpp>

////@begin XPM images
////@end XPM images

BEGIN_NCBI_SCOPE

/*!
 * CSubmissionPrepMainPanel type definition
 */

IMPLEMENT_DYNAMIC_CLASS( CSubmissionPrepMainPanel, wxPanel )


/*!
 * CSubmissionPrepMainPanel event table definition
 */

BEGIN_EVENT_TABLE( CSubmissionPrepMainPanel, wxPanel )

////@begin CSubmissionPrepMainPanel event table entries
    EVT_HYPERLINK( ID_HYPERLINKCTRL1, CSubmissionPrepMainPanel::OnEditSequencingTechnologyClicked )

    EVT_CHOICE( ID_SOURCETYPE, CSubmissionPrepMainPanel::OnSourceTypeSelected )

    EVT_HYPERLINK( ID_HYPERLINKCTRL2, CSubmissionPrepMainPanel::OnEditSourceClicked )

    EVT_HYPERLINK( ID_HYPERLINKCTRL3, CSubmissionPrepMainPanel::OnAddFeaturesClicked )

    EVT_HYPERLINK( ID_HYPERLINKCTRL4, CSubmissionPrepMainPanel::OnImportFeatureTableFile )

    EVT_HYPERLINK( ID_REMOVE_FEAT, CSubmissionPrepMainPanel::OnRemoveFeatHyperlinkClicked )

////@end CSubmissionPrepMainPanel event table entries

END_EVENT_TABLE()


/*!
 * CSubmissionPrepMainPanel constructors
 */

CSubmissionPrepMainPanel::CSubmissionPrepMainPanel()
{
    Init();
}

CSubmissionPrepMainPanel::CSubmissionPrepMainPanel( wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style )
{
    Init();
    Create(parent, id, pos, size, style);
}


/*!
 * CSubmissionPrepMainPanel creator
 */

bool CSubmissionPrepMainPanel::Create( wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style )
{
////@begin CSubmissionPrepMainPanel creation
    wxPanel::Create( parent, id, pos, size, style );

    CreateControls();
    if (GetSizer())
    {
        GetSizer()->SetSizeHints(this);
    }
    Centre();
////@end CSubmissionPrepMainPanel creation
    return true;
}


/*!
 * CSubmissionPrepMainPanel destructor
 */

CSubmissionPrepMainPanel::~CSubmissionPrepMainPanel()
{
////@begin CSubmissionPrepMainPanel destruction
////@end CSubmissionPrepMainPanel destruction
}


static CSubmissionPrepMainPanel::TWizardSrcTypeName s_srctype_names[] = {
  CSubmissionPrepMainPanel::TWizardSrcTypeName(CSourceRequirements::eWizardSrcType_any, "Not listed above or mixed set of different viruses"),
  CSubmissionPrepMainPanel::TWizardSrcTypeName(CSourceRequirements::eWizardSrcType_any, "Any"),
  CSubmissionPrepMainPanel::TWizardSrcTypeName(CSourceRequirements::eWizardSrcType_any, "Some other organism"),
  CSubmissionPrepMainPanel::TWizardSrcTypeName(CSourceRequirements::eWizardSrcType_any, "Something else"),
  CSubmissionPrepMainPanel::TWizardSrcTypeName(CSourceRequirements::eWizardSrcType_virus_norovirus, "Norovirus, Sapovirus (Caliciviridae)"),
  CSubmissionPrepMainPanel::TWizardSrcTypeName(CSourceRequirements::eWizardSrcType_virus_foot_and_mouth, "Foot-and-mouth disease virus"),
  CSubmissionPrepMainPanel::TWizardSrcTypeName(CSourceRequirements::eWizardSrcType_virus_influenza, "Influenza virus"),
  CSubmissionPrepMainPanel::TWizardSrcTypeName(CSourceRequirements::eWizardSrcType_virus_rotavirus, "Rotavirus"),
  CSubmissionPrepMainPanel::TWizardSrcTypeName(CSourceRequirements::eWizardSrcType_bacteria_or_archaea, "Bacteria or Archaea"),
  CSubmissionPrepMainPanel::TWizardSrcTypeName(CSourceRequirements::eWizardSrcType_bacteria_or_archaea, "Cultured Bacteria or Archaea"),
  CSubmissionPrepMainPanel::TWizardSrcTypeName(CSourceRequirements::eWizardSrcType_uncultured_fungus, "Fungi"),
  CSubmissionPrepMainPanel::TWizardSrcTypeName(CSourceRequirements::eWizardSrcType_cultured_fungus, "Cultured Fungus"),  
  CSubmissionPrepMainPanel::TWizardSrcTypeName(CSourceRequirements::eWizardSrcType_cultured_fungus, "Cultured fungal samples"),  
  CSubmissionPrepMainPanel::TWizardSrcTypeName(CSourceRequirements::eWizardSrcType_vouchered_fungus, "Vouchered Fungus"),
  CSubmissionPrepMainPanel::TWizardSrcTypeName(CSourceRequirements::eWizardSrcType_vouchered_fungus, "Vouchered fungal samples"),
  CSubmissionPrepMainPanel::TWizardSrcTypeName(CSourceRequirements::eWizardSrcType_plant, "Plant"),
  CSubmissionPrepMainPanel::TWizardSrcTypeName(CSourceRequirements::eWizardSrcType_animal, "Animal")
};
static int num_srctype_names = sizeof (s_srctype_names) / sizeof (CSubmissionPrepMainPanel::TWizardSrcTypeName);


/*!
 * Member initialisation
 */

void CSubmissionPrepMainPanel::Init()
{
////@begin CSubmissionPrepMainPanel member initialisation
    m_StarTech = NULL;
    m_SequencingTechLabel = NULL;
    m_SeqTechEdit = NULL;
    m_StarSourceType = NULL;
    m_SourceType = NULL;
    m_StarSource = NULL;
    m_SourceLabel = NULL;
    m_EditSourceLink = NULL;
    m_StarFeatures = NULL;
    m_FeaturesLabel = NULL;
    m_FeatureAddBtn = NULL;
    m_FeatureImportBtn = NULL;
    m_RemoveFeaturesBtn = NULL;
    m_WizardExtras = NULL;
    m_ExtraGrid = NULL;
    m_ChimeraSizer = NULL;
    m_VirusMolSizer = NULL;
////@end CSubmissionPrepMainPanel member initialisation

    m_SrcTypeNames.clear();
    for (int i = 0; i < num_srctype_names; i++) {
        m_SrcTypeNames.push_back(s_srctype_names[i]);
    }
    m_ChimeraPanel = NULL;
    m_VirusMolQuestions = NULL;
}


/*!
 * Control creation for CSubmissionPrepMainPanel
 */

void CSubmissionPrepMainPanel::CreateControls()
{    
////@begin CSubmissionPrepMainPanel content construction
    // Generated by DialogBlocks, 10/06/2013 17:53:07 (unregistered)

    CSubmissionPrepMainPanel* itemPanel1 = this;

    wxBoxSizer* itemBoxSizer2 = new wxBoxSizer(wxVERTICAL);
    itemPanel1->SetSizer(itemBoxSizer2);

    wxFlexGridSizer* itemFlexGridSizer3 = new wxFlexGridSizer(0, 4, 0, 0);
    itemBoxSizer2->Add(itemFlexGridSizer3, 0, wxALIGN_LEFT|wxALL, 5);

    wxStaticText* itemStaticText4 = new wxStaticText( itemPanel1, wxID_STATIC, _("Sequencing Technology"), wxDefaultPosition, wxSize(60, -1), 0 );
    itemStaticText4->Wrap(20);
    itemFlexGridSizer3->Add(itemStaticText4, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    m_StarTech = new wxStaticText( itemPanel1, wxID_STATIC, _("*"), wxDefaultPosition, wxDefaultSize, 0 );
    m_StarTech->SetForegroundColour(wxColour(255, 0, 0));
    m_StarTech->SetFont(wxFont(12, wxSWISS, wxNORMAL, wxNORMAL, false, wxT("Tahoma")));
    itemFlexGridSizer3->Add(m_StarTech, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    m_SequencingTechLabel = new wxTextCtrl( itemPanel1, ID_TEXTCTRL, wxEmptyString, wxDefaultPosition, wxSize(400, -1), wxTE_MULTILINE|wxTE_READONLY );
    itemFlexGridSizer3->Add(m_SequencingTechLabel, 0, wxGROW|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    m_SeqTechEdit = new wxHyperlinkCtrl( itemPanel1, ID_HYPERLINKCTRL1, _("Edit"), wxEmptyString, wxDefaultPosition, wxDefaultSize, wxHL_DEFAULT_STYLE );
    itemFlexGridSizer3->Add(m_SeqTechEdit, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxStaticText* itemStaticText8 = new wxStaticText( itemPanel1, wxID_STATIC, _("Source Type"), wxDefaultPosition, wxDefaultSize, 0 );
    itemFlexGridSizer3->Add(itemStaticText8, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    m_StarSourceType = new wxStaticText( itemPanel1, wxID_STATIC, _("*"), wxDefaultPosition, wxDefaultSize, 0 );
    m_StarSourceType->SetForegroundColour(wxColour(255, 0, 0));
    m_StarSourceType->SetFont(wxFont(12, wxSWISS, wxNORMAL, wxNORMAL, false, wxT("Tahoma")));
    itemFlexGridSizer3->Add(m_StarSourceType, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxArrayString m_SourceTypeStrings;
    m_SourceType = new wxChoice( itemPanel1, ID_SOURCETYPE, wxDefaultPosition, wxDefaultSize, m_SourceTypeStrings, 0 );
    if (CSubmissionPrepMainPanel::ShowToolTips())
        m_SourceType->SetToolTip(_("If you provide correct information about the type of organism the sample was isolated, this tool will suggest the required and recommended information needed to help the submission be processed more quickly by GenBank staff.\n"));
    itemFlexGridSizer3->Add(m_SourceType, 0, wxGROW|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    itemFlexGridSizer3->Add(5, 5, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxStaticText* itemStaticText12 = new wxStaticText( itemPanel1, wxID_STATIC, _("Source"), wxDefaultPosition, wxDefaultSize, 0 );
    itemFlexGridSizer3->Add(itemStaticText12, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    m_StarSource = new wxStaticText( itemPanel1, wxID_STATIC, _("*"), wxDefaultPosition, wxDefaultSize, 0 );
    m_StarSource->SetForegroundColour(wxColour(255, 0, 0));
    m_StarSource->SetFont(wxFont(12, wxSWISS, wxNORMAL, wxNORMAL, false, wxT("Tahoma")));
    itemFlexGridSizer3->Add(m_StarSource, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    m_SourceLabel = new wxTextCtrl( itemPanel1, ID_TEXTCTRL2, wxEmptyString, wxDefaultPosition, wxSize(-1, 50), wxTE_MULTILINE|wxTE_READONLY );
    itemFlexGridSizer3->Add(m_SourceLabel, 0, wxGROW|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    m_EditSourceLink = new wxHyperlinkCtrl( itemPanel1, ID_HYPERLINKCTRL2, _("Edit"), wxEmptyString, wxDefaultPosition, wxDefaultSize, wxHL_DEFAULT_STYLE );
    itemFlexGridSizer3->Add(m_EditSourceLink, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxStaticText* itemStaticText16 = new wxStaticText( itemPanel1, wxID_STATIC, _("Features"), wxDefaultPosition, wxDefaultSize, 0 );
    itemFlexGridSizer3->Add(itemStaticText16, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    m_StarFeatures = new wxStaticText( itemPanel1, wxID_STATIC, _("*"), wxDefaultPosition, wxDefaultSize, 0 );
    m_StarFeatures->SetForegroundColour(wxColour(255, 0, 0));
    m_StarFeatures->SetFont(wxFont(12, wxSWISS, wxNORMAL, wxNORMAL, false, wxT("Tahoma")));
    itemFlexGridSizer3->Add(m_StarFeatures, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    m_FeaturesLabel = new wxTextCtrl( itemPanel1, ID_TEXTCTRL1, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_MULTILINE|wxTE_READONLY );
    itemFlexGridSizer3->Add(m_FeaturesLabel, 0, wxGROW|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxBoxSizer* itemBoxSizer19 = new wxBoxSizer(wxVERTICAL);
    itemFlexGridSizer3->Add(itemBoxSizer19, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    m_FeatureAddBtn = new wxHyperlinkCtrl( itemPanel1, ID_HYPERLINKCTRL3, _("Add"), wxEmptyString, wxDefaultPosition, wxDefaultSize, wxHL_DEFAULT_STYLE );
    itemBoxSizer19->Add(m_FeatureAddBtn, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

    m_FeatureImportBtn = new wxHyperlinkCtrl( itemPanel1, ID_HYPERLINKCTRL4, _("Import File"), wxEmptyString, wxDefaultPosition, wxDefaultSize, wxHL_DEFAULT_STYLE );
    itemBoxSizer19->Add(m_FeatureImportBtn, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

    m_RemoveFeaturesBtn = new wxHyperlinkCtrl( itemPanel1, ID_REMOVE_FEAT, _("Remove All Features"), wxEmptyString, wxDefaultPosition, wxDefaultSize, wxHL_DEFAULT_STYLE );
    itemBoxSizer19->Add(m_RemoveFeaturesBtn, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

    m_WizardExtras = new wxBoxSizer(wxVERTICAL);
    itemBoxSizer2->Add(m_WizardExtras, 0, wxALIGN_LEFT|wxALL, 0);

    m_ExtraGrid = new wxFlexGridSizer(0, 4, 0, 0);
    m_WizardExtras->Add(m_ExtraGrid, 0, wxALIGN_LEFT|wxALL, 0);

    m_ChimeraSizer = new wxBoxSizer(wxVERTICAL);
    itemBoxSizer2->Add(m_ChimeraSizer, 0, wxALIGN_LEFT|wxALL, 0);

    m_VirusMolSizer = new wxBoxSizer(wxVERTICAL);
    itemBoxSizer2->Add(m_VirusMolSizer, 0, wxALIGN_LEFT|wxALL, 5);

    // Connect events and objects
    m_SequencingTechLabel->Connect(ID_TEXTCTRL, wxEVT_LEFT_DOWN, wxMouseEventHandler(CSubmissionPrepMainPanel::OnClickSeqTechSummary), NULL, this);
    m_SourceLabel->Connect(ID_TEXTCTRL2, wxEVT_LEFT_DOWN, wxMouseEventHandler(CSubmissionPrepMainPanel::OnClickSrcSummary), NULL, this);
    m_FeaturesLabel->Connect(ID_TEXTCTRL1, wxEVT_LEFT_DOWN, wxMouseEventHandler(CSubmissionPrepMainPanel::OnClickFeatureSummary), NULL, this);
////@end CSubmissionPrepMainPanel content construction
}


/*!
 * Should we show tooltips?
 */

bool CSubmissionPrepMainPanel::ShowToolTips()
{
    return true;
}

/*!
 * Get bitmap resources
 */

wxBitmap CSubmissionPrepMainPanel::GetBitmapResource( const wxString& name )
{
    // Bitmap retrieval
////@begin CSubmissionPrepMainPanel bitmap retrieval
    wxUnusedVar(name);
    return wxNullBitmap;
////@end CSubmissionPrepMainPanel bitmap retrieval
}

/*!
 * Get icon resources
 */

wxIcon CSubmissionPrepMainPanel::GetIconResource( const wxString& name )
{
    // Icon retrieval
////@begin CSubmissionPrepMainPanel icon retrieval
    wxUnusedVar(name);
    return wxNullIcon;
////@end CSubmissionPrepMainPanel icon retrieval
}


CSubPrep_panel* CSubmissionPrepMainPanel::x_GetParent()
{
    wxWindow* w = this->GetParent();
    while (w != NULL) {
        CSubPrep_panel* parent = dynamic_cast<CSubPrep_panel*>(w);
        if (parent) {
            return parent;
        } else {
            w = w->GetParent();
        }
    }
    return NULL;
}


static void s_ShowStar(wxStaticText* star, bool show)
{
    if (show) {
        star->SetLabel(wxT("*"));
    } else {
        star->SetLabel(wxT(" "));
    }
}


void CSubmissionPrepMainPanel::UpdateSeqTechCtrls (CSourceRequirements::EWizardType wizard_type)
{
    CSubPrep_panel* parent = x_GetParent();
    if (!parent) {
        return;
    }

    bool required = parent->IsSequencingTechnologyRequired (wizard_type);

    bool ok_to_bulk_edit = true;
    string message = "";

    bool missing_required = false;

    CRef<objects::CSeq_table> table = parent->GetSeqTechTable();

    if (table) {
        string mixed = "";
        string missing = "";
        string all_summary = "";
        bool any_data = false;
        bool missing_required_col = false;
        for (size_t i = 1; i < table->GetColumns().size(); i++) {
            string title = table->GetColumns()[i]->GetHeader().GetTitle();
            if (AreAllColumnValuesTheSame(table->SetColumns()[i], "")) {
                string first_val = table->GetColumns()[i]->GetData().GetString()[0];
                if (!NStr::IsBlank(first_val)) {
                    any_data = true;
                    if (!NStr::IsBlank(all_summary)) {
                        all_summary += "\n";
                    }
                    all_summary += title + ":" + first_val;
                }
            } else {
                any_data = true;
                ok_to_bulk_edit = false;
                if (NStr::IsBlank(mixed)) {
                    mixed = "Record contains mixed values for ";
                } else {
                    mixed += ", ";
                }
                mixed += title;
            }         
            if (AreAnyColumnValuesMissing(table->SetColumns()[i])) {
                if (NStr::Equal(title, "Sequencing Technology")) {
                    missing += "Missing required Sequencing Technology information. ";
                    missing_required_col = true;
                }
            }
        }
        if (!NStr::IsBlank(mixed)) {
            mixed += ".";
        }
        if ((required || any_data) && missing_required_col) {
            missing_required = true;
        }
        if (any_data) {
            message = missing + mixed;
        } else if (required) {
            message = "Missing required sequencing technology information";
            missing_required = true;
        }
        if (NStr::IsBlank(message)) {
            message = all_summary;
        }
    } else {
        if (required) {
            message = "Missing required sequencing technology information";
            missing_required = true;
        }
    }

    // update controls
    m_SequencingTechLabel->SetValue(ToWxString (message));
    s_ShowStar (m_StarTech, missing_required);
    if (missing_required) {
        parent->SetSubmissionNotReady(message);
    }
    if (ok_to_bulk_edit) {
        m_SeqTechEdit->SetLabel(wxT("Edit"));
    } else {
        m_SeqTechEdit->SetLabel(wxT("Clear"));
    }
}


/*!
 * wxEVT_LEFT_DOWN event handler for ID_TEXTCTRL2
 */

void CSubmissionPrepMainPanel::OnClickSrcSummary( wxMouseEvent& event )
{
    CSubPrep_panel* parent = x_GetParent();
    if (!parent) {
        return;
    }
    parent->LaunchSourceEditor();
}


/*!
 * wxEVT_LEFT_DOWN event handler for ID_TEXTCTRL
 */

void CSubmissionPrepMainPanel::OnClickSeqTechSummary( wxMouseEvent& event )
{
    CSubPrep_panel* parent = x_GetParent();
    if (!parent) {
        return;
    }
    if (parent->OkToBulkEditSeqTech()) {
        parent->LaunchSeqTechEditor();
    }
}


/*!
 * wxEVT_COMMAND_HYPERLINK event handler for ID_HYPERLINKCTRL1
 */

void CSubmissionPrepMainPanel::OnEditSequencingTechnologyClicked( wxHyperlinkEvent& event )
{
    CSubPrep_panel* parent = x_GetParent();
    if (!parent) {
        return;
    }
    wxHyperlinkCtrl* link_ctrl = dynamic_cast<wxHyperlinkCtrl*>(event.GetEventObject());
    if (link_ctrl) {
        if (NStr::Equal("Edit", ToStdString(link_ctrl->GetLabel()))) {
            parent->LaunchSeqTechEditor();
        } else {
            parent->ClearSeqTechValues();
        }
    }
}


void CSubmissionPrepMainPanel::SetSourceTypeOptions (CSourceRequirements::EWizardType wizard_type)
{
    m_SourceType->Clear();
    switch (wizard_type) {
        case CSourceRequirements::eWizardType_viruses:
            m_SourceType->Append (wxT("Norovirus, Sapovirus (Caliciviridae)"));
            m_SourceType->Append (wxT("Foot-and-mouth disease virus"));
            m_SourceType->Append (wxT("Influenza virus"));
            m_SourceType->Append (wxT("Rotavirus"));
            m_SourceType->Append (wxT("Not listed above or mixed set of different viruses"));
            m_SourceType->Enable (true);
            break;
        case CSourceRequirements::eWizardType_uncultured_samples:
            m_SourceType->Append (wxT("Bacteria or Archaea"));
            m_SourceType->Append (wxT("Fungi"));
            m_SourceType->Append (wxT("Some other organism"));
            m_SourceType->Enable (true);
            break;
        case CSourceRequirements::eWizardType_rrna_its_igs:
            m_SourceType->Append (wxT("Cultured Bacteria or Archaea"));
            m_SourceType->Append (wxT("Cultured Fungus"));
            m_SourceType->Append (wxT("Vouchered Fungus"));
            m_SourceType->Append (wxT("Something else"));
            m_SourceType->Enable (true);
            break;
        case CSourceRequirements::eWizardType_igs:
            m_SourceType->Append (wxT("Culutured fungal samples"));
            m_SourceType->Append (wxT("Vouchered fungal samples"));
            m_SourceType->Append (wxT("Plant"));
            m_SourceType->Append (wxT("Animal"));
            m_SourceType->Enable (true);
            break;
        case CSourceRequirements::eWizardType_standard:
        case CSourceRequirements::eWizardType_tsa:
        case CSourceRequirements::eWizardType_microsatellite:
        case CSourceRequirements::eWizardType_d_loop:
        default:
            m_SourceType->Append (wxT("Any"));
            m_SourceType->SetSelection(0);
            m_SourceType->Enable (false);
            break;
    }
}


void CSubmissionPrepMainPanel::SetSourceType (CSourceRequirements::EWizardSrcType src_type)
{
    CSubPrep_panel* parent = x_GetParent();
    if (!parent) {
        return;
    }
    CSourceRequirements::EWizardType wizard_type = parent->GetWizardType();

    bool is_unchosen = false;
    if (src_type == CSourceRequirements::eWizardSrcType_any) {
        CSubPrep_panel* parent = x_GetParent();
        if (parent && NStr::IsBlank(parent->GetWizardSrcTypeField())) {
            CSourceRequirements::EWizardType wizard_type = parent->GetWizardType();
            if (wizard_type == CSourceRequirements::eWizardType_standard
                || wizard_type == CSourceRequirements::eWizardType_tsa
                || wizard_type == CSourceRequirements::eWizardType_microsatellite 
                || wizard_type == CSourceRequirements::eWizardType_d_loop) {
            } else {
                is_unchosen = true;
            }
        }        
    }

    wxArrayString choices = m_SourceType->GetStrings();
    if (!is_unchosen) {
        // if the last option is blank, get rid of it
        if (choices.size() > 0 && choices.Last() == wxEmptyString) {
            SetSourceTypeOptions(wizard_type);
        }
        is_unchosen = true;
        ITERATE (TWizardSrcTypeNameList, kit, m_SrcTypeNames) {
            if (kit->first == src_type) {
                wxArrayString::iterator it = choices.begin();
                size_t pos = 0;
                while (it != choices.end() && !NStr::EqualNocase(kit->second, ToStdString (*it))) {
                    pos++;
                    it++;
                }
                if (it != choices.end()) {
                    m_SourceType->SetSelection(pos);
                    is_unchosen = false;
                    break;
                }
            }
        }
    }

    s_ShowStar (m_StarSourceType, is_unchosen && choices.size() > 1);

    if (is_unchosen && choices.size() > 1) {
        m_SourceType->AppendString(wxEmptyString);
        m_SourceType->SetSelection(choices.size());
        parent->SetSubmissionNotReady("Must choose source type");
        // disable controls below
        m_SourceLabel->Enable(false);
        m_EditSourceLink->Enable(false);
        m_FeaturesLabel->Enable(false);
        m_FeatureAddBtn->Enable(false);
        m_FeatureImportBtn->Enable(false);
        m_RemoveFeaturesBtn->Enable(false);
    } else {
        // enable controls below
        m_SourceLabel->Enable(true);
        m_EditSourceLink->Enable(true);
        m_FeaturesLabel->Enable(true);
        m_FeatureAddBtn->Enable(true);
        m_FeatureImportBtn->Enable(true);
        m_RemoveFeaturesBtn->Enable(true);
    }
}


CSourceRequirements::EWizardSrcType CSubmissionPrepMainPanel::GetWizardSrcTypeFromCtrls (CSourceRequirements::EWizardType wizard_type)
{
    int val = m_SourceType->GetSelection();
    CSourceRequirements::EWizardSrcType src_type = CSourceRequirements::eWizardSrcType_any;

    switch (wizard_type) {
        case CSourceRequirements::eWizardType_viruses:
            switch (val) {
                case 0: 
                    src_type = CSourceRequirements::eWizardSrcType_virus_norovirus;
                    break;
                case 1:
                    src_type = CSourceRequirements::eWizardSrcType_virus_foot_and_mouth;
                    break;
                case 2:
                    src_type = CSourceRequirements::eWizardSrcType_virus_influenza;
                    break;
                case 3:
                    src_type = CSourceRequirements::eWizardSrcType_virus_rotavirus;
                    break;
                default:
                    src_type = CSourceRequirements::eWizardSrcType_any;
                    break;
            }
            break;
        case CSourceRequirements::eWizardType_uncultured_samples:
            switch (val) {
                case 0:
                    src_type = CSourceRequirements::eWizardSrcType_bacteria_or_archaea;
                    break;
                case 1:
                    src_type = CSourceRequirements::eWizardSrcType_uncultured_fungus;
                    break;
                default:
                    src_type = CSourceRequirements::eWizardSrcType_any;
                    break;
            }
            break;
        case CSourceRequirements::eWizardType_rrna_its_igs:
            switch (val) {
                case 0:
                    src_type = CSourceRequirements::eWizardSrcType_bacteria_or_archaea;
                    break;
                case 1:
                    src_type = CSourceRequirements::eWizardSrcType_cultured_fungus;
                    break;
                case 2:
                    src_type = CSourceRequirements::eWizardSrcType_vouchered_fungus;
                    break;
                default:
                    src_type = CSourceRequirements::eWizardSrcType_any;
                    break;
            }
            break;
        case CSourceRequirements::eWizardType_igs:
            switch (val) {
                case 0:
                    src_type = CSourceRequirements::eWizardSrcType_cultured_fungus;
                    break;
                case 1:
                    src_type = CSourceRequirements::eWizardSrcType_vouchered_fungus;
                    break;
                case 2:
                    src_type = CSourceRequirements::eWizardSrcType_cultured_fungus;
                    break;
                case 3:
                    src_type = CSourceRequirements::eWizardSrcType_plant;
                    break;
                default:
                    src_type = CSourceRequirements::eWizardSrcType_animal;
                    break;
            }
            break;
        default:
            src_type = CSourceRequirements::eWizardSrcType_any;
            break;
    }
            
    return src_type;
}


CSourceRequirements::EWizardSrcType CSubmissionPrepMainPanel::GetWizardSrcTypeFromName(string name)
{
    CSourceRequirements::EWizardSrcType src_type = CSourceRequirements::eWizardSrcType_any;
    ITERATE (TWizardSrcTypeNameList, kit, m_SrcTypeNames) {
        if (NStr::EqualNocase (kit->second, name)) {
            src_type = kit->first;
            break;
        }
    }
    return src_type;
}


/*!
 * wxEVT_COMMAND_CHOICE_SELECTED event handler for ID_SOURCETYPE
 */

void CSubmissionPrepMainPanel::OnSourceTypeSelected( wxCommandEvent& event )
{
    CSubPrep_panel* parent = x_GetParent();
    if (!parent) {
        return;
    }
    parent->SetSourceType(ToStdString(m_SourceType->GetStringSelection()));
    s_ShowStar (m_StarSourceType, false);
}


void CSubmissionPrepMainPanel::SetSourceLabel (string label)
{
    m_SourceLabel->SetValue(ToWxString(label));
}


void CSubmissionPrepMainPanel::ShowSourceLabelStar (bool val)
{
    s_ShowStar (m_StarSource, val);
}


void CSubmissionPrepMainPanel::ResetFieldsInUse()
{
    NON_CONST_ITERATE(vector<bool>, it, m_FieldsInUse) {
        *it = false;
    }
}


void CSubmissionPrepMainPanel::ClearUnusedFields()
{
    int num_keep = 0;
    for (int pos = m_FieldsInUse.size() - 1; pos > -1; pos--) {
        if (m_FieldsInUse[pos]) {
            num_keep ++;
        } else {
            // note - delete item pos + 1, because m_ExtraGrid is the first element in the sizer
            m_WizardExtras->GetItem(pos + 1)->DeleteWindows();
            m_WizardExtras->Remove(pos + 1);
            if (pos < m_ExtraFields.size()) {
                m_ExtraFields[pos] = NULL;
            }
        }
    }

    m_FieldsInUse.clear();
    for (int pos = 0; pos < num_keep; pos++) {
        m_FieldsInUse.push_back(true);
    }    
    vector<CSubPrepFieldEditor * >::iterator it = m_ExtraFields.begin();
    while (it != m_ExtraFields.end()) {
        if ((*it) == NULL) {
            it = m_ExtraFields.erase(it);
        } else {
            it++;
        }
    }
}


void CSubmissionPrepMainPanel::AddField(objects::CSeq_entry_Handle seh, 
                                        ICommandProccessor* processor,
                                        IWorkbench* workbench,
                                        string label, string column_name, 
                                        CTableFieldCommandConverter * converter, 
                                        bool must_choose, bool required, 
                                        string default_val, string url,
                                        bool allow_mix)
{
    CSubPrepFieldEditor* new_field = NULL;
    // first, see if we already have one, and if so, mark it as in use
    size_t pos = 0;
    ITERATE(vector<CSubPrepFieldEditor* >, it, m_ExtraFields) {
        if (NStr::Equal((*it)->GetColumnName(), column_name)) {
            new_field = *it;
            break;
        }
        pos++;
    }

    if (new_field == NULL) {
        new_field = new CSubPrepFieldEditor(this, seh, processor, workbench);
        m_WizardExtras->Add(new_field, 0, wxGROW|wxALIGN_CENTER_VERTICAL|wxALL, 0);
        m_ExtraFields.push_back(new_field);
        m_FieldsInUse.push_back(true);
    } else {
        m_FieldsInUse[pos] = true;
    }
    
    new_field->SetConverter(label, column_name, converter, must_choose, required, default_val, url, allow_mix);
    // update
    Layout();
    Refresh();
}


/*!
 * wxEVT_COMMAND_HYPERLINK event handler for ID_HYPERLINKCTRL2
 */

void CSubmissionPrepMainPanel::OnEditSourceClicked( wxHyperlinkEvent& event )
{
    CSubPrep_panel* parent = x_GetParent();
    if (!parent) {
        return;
    }
    parent->LaunchSourceEditor();
}


/*!
 * wxEVT_COMMAND_HYPERLINK event handler for ID_HYPERLINKCTRL3
 */

void CSubmissionPrepMainPanel::OnAddFeaturesClicked( wxHyperlinkEvent& event )
{
    CSubPrep_panel* parent = x_GetParent();
    if (!parent) {
        return;
    }
    parent->LaunchFeatureAdder();
}


/*!
 * wxEVT_COMMAND_HYPERLINK event handler for ID_HYPERLINKCTRL4
 */

void CSubmissionPrepMainPanel::OnImportFeatureTableFile( wxHyperlinkEvent& event )
{
    CSubPrep_panel* parent = x_GetParent();
    if (!parent) {
        return;
    }
    parent->ImportFeatureTableFile();
}


/*!
 * wxEVT_LEFT_DOWN event handler for ID_TEXTCTRL1
 */

void CSubmissionPrepMainPanel::OnClickFeatureSummary( wxMouseEvent& event )
{
    CSubPrep_panel* parent = x_GetParent();
    if (!parent) {
        return;
    }
    parent->LaunchFeatureAdder();
}


void CSubmissionPrepMainPanel::ShowFeaturesStar (bool val)
{
    s_ShowStar(m_StarFeatures, val);
}


void CSubmissionPrepMainPanel::SetFeaturesLabel(string label)
{
    m_FeaturesLabel->SetValue(ToWxString(label));
}


void CSubmissionPrepMainPanel::ShowAddFeatureBtn(bool val)
{
    m_FeatureAddBtn->Show(val);
}


wxStaticText* CSubmissionPrepMainPanel::x_AddStar(wxFlexGridSizer* sizer)
{
    wxStaticText* star = new wxStaticText( this, wxID_STATIC, _("*"), wxDefaultPosition, wxDefaultSize, 0 );
    star->SetForegroundColour(wxColour(255, 0, 0));
    sizer->Add(star, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxALL, 5);
    star->SetFont(wxFont(12, wxSWISS, wxNORMAL, wxNORMAL, false, wxT("Tahoma")));
    return star;
}


void CSubmissionPrepMainPanel::CreateSetChoice (CSourceRequirements::EWizardType wizard_type, objects::CSeq_entry_Handle seh)
{
    // clear previous set choice control if it exists
    while (m_ExtraGrid->GetItemCount() > 0) {
        size_t pos = 0;
        m_ExtraGrid->GetItem(pos)->DeleteWindows();
        m_ExtraGrid->Remove(pos);
    }

    CSubPrep_panel* parent = x_GetParent();
    if (!parent) {
        return;
    }

    if (!seh || !seh.IsSet()) {
        // don't need a set type if not a set
        return;
    }
    int set_class = objects::CBioseq_set::eClass_not_set;
    if (seh.GetSet().IsSetClass()) {
        set_class = seh.GetSet().GetClass();
    }

    wxArrayString set_strings;
    wxString tooltip_text = wxEmptyString;

    wxString sel = wxEmptyString;
    switch (set_class) {
        case objects::CBioseq_set::eClass_pop_set:
            sel = "Population Study";
            break;
        case objects::CBioseq_set::eClass_phy_set:
            sel = "Phylogenetic Study";
            break;
        case objects::CBioseq_set::eClass_mut_set:
            sel = "Mutation Study";
            break;
        case objects::CBioseq_set::eClass_eco_set:
            sel = "Environmental set";
            break;
        case objects::CBioseq_set::eClass_genbank:
            sel = "Batch";
            break;
        default:
            // do nothing
            break;
    }

    switch (wizard_type) {
        case CSourceRequirements::eWizardType_standard:
            tooltip_text.append(wxT("Batch: Do not process as a set.\n"));
            tooltip_text.append(wxT("Population Study: a set of sequences that were derived by sequencing the same gene from different isolates of the same organism.\n"));
            tooltip_text.append(wxT("Phylogenetic Study: a set of sequences that were derived by sequencing the same gene from different organisms.\n"));
            tooltip_text.append(wxT("Mutation Study: a set of sequences that were derived by sequencing multiple mutations of a single gene.\n"));
            tooltip_text.append(wxT("Environmental set: a set of sequences that were derived by sequencing the same gene from a population of unclassified or unknown organisms."));

            set_strings.push_back(wxT("Batch"));
            set_strings.push_back(wxT("Population Study"));
            set_strings.push_back(wxT("Phylogenetic Study"));
            set_strings.push_back(wxT("Mutation Study"));
            set_strings.push_back(wxT("Environmental set"));
            break;
        case CSourceRequirements::eWizardType_uncultured_samples:
            tooltip_text.append(wxT("Batch: Do not process as a set.\n"));
            tooltip_text.append(wxT("Environmental set: a set of sequences that were derived by sequencing the same gene from a population of unclassified or unknown organisms."));
            set_strings.push_back(wxT("Batch"));
            set_strings.push_back(wxT("Environmental set"));
            break;
        case CSourceRequirements::eWizardType_viruses:
        case CSourceRequirements::eWizardType_rrna_its_igs:
        case CSourceRequirements::eWizardType_igs:
        case CSourceRequirements::eWizardType_d_loop:
            tooltip_text.append(wxT("Batch: Do not process as a set.\n"));
            tooltip_text.append(wxT("Population Study: a set of sequences that were derived by sequencing the same gene from different isolates of the same organism.\n"));
            tooltip_text.append(wxT("Phylogenetic Study: a set of sequences that were derived by sequencing the same gene from different organisms.\n"));
            tooltip_text.append(wxT("Mutation Study: a set of sequences that were derived by sequencing multiple mutations of a single gene.\n"));
            tooltip_text.append(wxT("Environmental set: a set of sequences that were derived by sequencing the same gene from a population of unclassified or unknown organisms."));

            set_strings.push_back(wxT("Batch"));
            set_strings.push_back(wxT("Population Study"));
            set_strings.push_back(wxT("Phylogenetic Study"));
            set_strings.push_back(wxT("Mutation Study"));
            set_strings.push_back(wxT("Environmental set"));
            break;
        default:
            if (set_class == objects::CBioseq_set::eClass_genbank) {
                // do nothing
            } else {
                set_strings.push_back(wxT("Batch"));
            }
            break;
    } 
    if (set_strings.empty()) {
        // nothing to choose
        return;
    } else {
        wxStaticText* itemStaticText = new wxStaticText( this, wxID_STATIC, wxT("Set Type"),
                                                         wxDefaultPosition, wxSize(60, -1), wxALIGN_CENTRE );
        m_ExtraGrid->Add(itemStaticText, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxALL, 5);

        wxStaticText* star = x_AddStar(m_ExtraGrid);

        wxChoice *choice = new wxChoice( this, ID_SET_CHOICE, wxDefaultPosition, wxSize(400, -1), set_strings, 0 );
        m_ExtraGrid->Add(choice, 0, wxGROW|wxALIGN_CENTER_VERTICAL|wxALL, 5);
        wxString please_choose = wxT("Please choose a set type");
        if (sel.empty() || !choice->SetStringSelection(sel)) {
            choice->AppendString(please_choose);
            choice->SetStringSelection(please_choose);
            parent->SetSubmissionNotReady("Must choose set type");
            s_ShowStar (star, true);
        } else {
            s_ShowStar (star, false);
        }

        if (CSubmissionPrepMainPanel::ShowToolTips() && !tooltip_text.IsEmpty())
            choice->SetToolTip(tooltip_text);

        // add spacer as place holder
        m_ExtraGrid->Add(5, 5, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxALL, 5);

        Layout();
        Refresh();
    }
}


void CSubmissionPrepMainPanel::ShowChimera (objects::CSeq_entry_Handle seh, 
                                            ICommandProccessor* processor,
                                            IWorkbench* workbench, bool show)
{
    if (!m_ChimeraPanel) {
        m_ChimeraPanel = new CChimeraPanel(this, seh, processor, workbench);
        m_ChimeraSizer->Add(m_ChimeraPanel);
    }
    m_ChimeraPanel->UpdateChimera();
    Fit();
    Layout();
    Refresh();
}


void CSubmissionPrepMainPanel::CreateVirusMoleculeOptions (objects::CSeq_entry_Handle seh, ICommandProccessor* processor)
{
    CRef<objects::CSeq_table> values_table = BuildMolInfoValuesTableFromSeqEntry(seh, CSourceRequirements::eWizardType_viruses);
    CRef<objects::CSeqTable_column> moltype_col = FindSeqTableColumnByName (values_table, kMoleculeType);
    if (moltype_col && DoesColumnHaveValue (moltype_col, "mRNA")) {
        if (m_VirusMolQuestions) {
            m_VirusMolQuestions->Show(true);
        } else {
            m_VirusMolQuestions = new CVirusMoleculeQuestionsPanel(this, seh, processor);
            m_VirusMolSizer->Add(m_VirusMolQuestions, 0, wxGROW|wxALIGN_CENTER_VERTICAL|wxALL, 5);
        }
        m_VirusMolQuestions->OnDataChange();
    } else {
        if (m_VirusMolQuestions) {
            m_VirusMolQuestions->Show(false);
        }
    }
    Fit();
    Layout();
    Refresh();
}


/*!
 * wxEVT_COMMAND_HYPERLINK event handler for ID_REMOVE_FEAT
 */

void CSubmissionPrepMainPanel::OnRemoveFeatHyperlinkClicked( wxHyperlinkEvent& event )
{
    CSubPrep_panel* parent = x_GetParent();
    if (!parent) {
        return;
    }
    parent->RemoveAllFeatures();
}


END_NCBI_SCOPE


