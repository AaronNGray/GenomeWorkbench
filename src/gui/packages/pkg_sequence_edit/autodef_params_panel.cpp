/*  $Id: autodef_params_panel.cpp 39666 2017-10-25 16:01:13Z katargir $
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

#include <gui/packages/pkg_sequence_edit/autodef_params_panel.hpp>
#include <gui/widgets/object_list/object_list_widget.hpp>
#include <gui/widgets/seq/feature_check_dialog.hpp>
#include <gui/widgets/wx/wx_utils.hpp>

#include <objects/seqloc/Seq_id.hpp>
#include <objects/general/Object_id.hpp>

#include <objtools/edit/autodef_with_tax.hpp>

#include <wx/sizer.h>
#include <wx/stattext.h>
#include <wx/statbox.h>
#include <wx/textctrl.h>
#include <wx/button.h>
#include <wx/scrolwin.h>
#include <wx/bitmap.h>
#include <wx/checkbox.h>
#include <wx/icon.h>
#include <wx/choice.h>
#include <wx/radiobox.h>

BEGIN_NCBI_SCOPE
USING_SCOPE (objects);
////@begin XPM images
////@end XPM images


/*!
 * CAutodefParamsPanel type definition
 */

IMPLEMENT_DYNAMIC_CLASS( CAutodefParamsPanel, CAlgoToolManagerParamsPanel )


/*!
 * CAutodefParamsPanel event table definition
 */

BEGIN_EVENT_TABLE( CAutodefParamsPanel, wxPanel )

////@begin CAutodefParamsPanel event table entries
EVT_CHOICE (ID_MODIFIER_LIST, CAutodefParamsPanel::OnChangeModifierListChoice )
EVT_CHECKBOX (ID_MODIFIER_CHECKBOX, CAutodefParamsPanel::OnChangeModifierSelection )
EVT_CHECKBOX (ID_INCLUDE_MISC_FEATS, CAutodefParamsPanel::OnChangeMiscFeat )
EVT_BUTTON (ID_SUPPRESS_FEATURES, CAutodefParamsPanel::OnSuppressFeatures )
////@end CAutodefParamsPanel event table entries

END_EVENT_TABLE()


/*!
 * CAutodefParamsPanel constructors
 */

CAutodefParamsPanel::CAutodefParamsPanel()
{
    Init();
}

CAutodefParamsPanel::CAutodefParamsPanel( wxWindow* parent, wxWindowID id, const wxString& caption, const wxPoint& pos, const wxSize& size, long style )
{
    Init();
    Create(parent, id, caption, pos, size, style);
}


/*!
 * CAutodefParamsPanel creator
 */

bool CAutodefParamsPanel::Create( wxWindow* parent, wxWindowID id, const wxString& caption, const wxPoint& pos, const wxSize& size, long style )
{
////@begin CAutodefParamsPanel creation
    CAlgoToolManagerParamsPanel::Create( parent, id, pos, size, style );

    CreateControls();
    if (GetSizer())
    {
        GetSizer()->SetSizeHints(this);
    }
    Centre();
////@end CAutodefParamsPanel creation
    return true;
}


/*!
 * CAutodefParamsPanel destructor
 */

CAutodefParamsPanel::~CAutodefParamsPanel()
{
////@begin CAutodefParamsPanel destruction
////@end CAutodefParamsPanel destruction
}


/*!
 * Member initialisation
 */

void CAutodefParamsPanel::Init()
{
////@begin CAutodefParamsPanel member initialisation
    m_ObjectListCtrl = NULL;
    m_ModifierListChoice = NULL;
    m_MaxModifiers = NULL;
    m_ModifierButtons.clear();
    m_UseLabels = NULL;
    m_LeaveParen = NULL;
    m_NoSp = NULL;
    m_NoNr = NULL;
    m_NoCf = NULL;
    m_NoAff = NULL;
    m_HIVRule = NULL;
    m_IncludeCountry = NULL;
    m_FeatureListChoice = NULL;
    m_SuppressAltSplice = NULL;
    m_SuppressTransposon = NULL;
    m_SupressGeneCluster = NULL;
    m_SuppressLocusTags = NULL;
    m_KeepExons = NULL;
    m_Keep5UTRs = NULL;
    m_KeepIntrons = NULL;
    m_Keep3UTRs = NULL;
    m_KeepRegulatoryFeatures = NULL;
    m_UseFakePromoters = NULL;
    m_KeepMiscFeats = NULL;
    m_MiscFeatParseRule = NULL;
    m_ProductClauseChoice = NULL;
    m_AppendAltSplice = NULL;
    m_UseNcRNAComment = NULL;

    m_Params = NULL;
    m_InputListDirty = true;
    m_SuppressedFeatures.clear();

////@end CAutodefParamsPanel member initialisation
}


static string s_GetModifierStatus (bool all_present, bool all_unique, bool is_unique)
{
    string status = "";

    if (all_present && all_unique)
    {
      status = "All present, all unique";
    }
    else if (all_present && is_unique)
    {
      status = "All present, one unique";
    }
    else if (all_present && ! is_unique)
    {
      status = "All present, mixed";
    }
    else if (! all_present && all_unique)
    {
      status = "Some missing, all unique";
    }
    else if (! all_present && is_unique)
    {
      status = "Some missing, one unique";
    }
    else if (! all_present && ! is_unique)
    {
      status = "Some missing, mixed";
    }
    return status;
}


/*!
 * Control creation for CAutodefParamsPanel
 */

void CAutodefParamsPanel::CreateControls()
{
////@begin CAutodefParamsPanel content construction
    CAutodefParamsPanel* itemDialog1 = this;

    wxBoxSizer* itemBoxSizer2 = new wxBoxSizer(wxVERTICAL);
    itemDialog1->SetSizer(itemBoxSizer2);

    m_ObjectListCtrl = new CObjectListWidget( itemDialog1, ID_LISTCTRL, wxDefaultPosition, itemDialog1->ConvertDialogToPixels(wxSize(100, 100)), wxLC_REPORT );
    itemBoxSizer2->Add(m_ObjectListCtrl, 0, wxGROW|wxALL, 5);

    wxStaticBox* itemStaticBoxSizer4Static = new wxStaticBox(itemDialog1, wxID_ANY, _("Organism Description"));
    wxStaticBoxSizer* itemStaticBoxSizer4 = new wxStaticBoxSizer(itemStaticBoxSizer4Static, wxVERTICAL);
    itemBoxSizer2->Add(itemStaticBoxSizer4, 0, wxGROW|wxALL, 5);

    wxBoxSizer* modifierLimitsSizer = new wxBoxSizer (wxHORIZONTAL);
    itemStaticBoxSizer4->Add (modifierLimitsSizer, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

    wxStaticText* itemStaticText7 = new wxStaticText( itemDialog1, wxID_STATIC, _("Modifier List"), wxDefaultPosition, wxDefaultSize, 0 );
    modifierLimitsSizer->Add(itemStaticText7, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxArrayString m_ModifierListChoiceStrings;
    m_ModifierListChoiceStrings.Add(_("Custom"));
    m_ModifierListChoiceStrings.Add(_("All"));
    m_ModifierListChoice = new wxChoice( itemDialog1, ID_MODIFIER_LIST, wxDefaultPosition, wxDefaultSize, m_ModifierListChoiceStrings, 0 );
    m_ModifierListChoice->SetStringSelection(_("Custom"));
    modifierLimitsSizer->Add(m_ModifierListChoice, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxStaticText* itemStaticText10 = new wxStaticText( itemDialog1, wxID_STATIC, _("Maximum modifiers per line"), wxDefaultPosition, wxDefaultSize, 0 );
    modifierLimitsSizer->Add(itemStaticText10, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxArrayString m_MaxModifiersStrings;

    m_MaxModifiersStrings.Add(_("no limit"));
    /* count available modifiers */
    int num_total_mods = 0;
    for (size_t n = 0; n < m_Params->m_ModifierList.size(); n++) {
        if (m_Params->m_ModifierList[n].AnyPresent()) {
            num_total_mods++;
            m_MaxModifiersStrings.Add
                (ToWxString(NStr::IntToString(num_total_mods)));
        }
    }

    m_MaxModifiers = new wxChoice( itemDialog1, ID_MODIFIERS_PER_LINE,
                                   wxDefaultPosition, wxDefaultSize,
                                   m_MaxModifiersStrings, 0 );
    modifierLimitsSizer->Add(m_MaxModifiers, 0,
                             wxALIGN_CENTER_VERTICAL|wxALL, 5);
    m_MaxModifiers->SetSelection (0);


    m_UseLabels = new wxCheckBox( itemDialog1, ID_USE_LABELS,
                                  _("Use labels (e.g., 'strain BALB')"),
                                  wxDefaultPosition, wxDefaultSize, 0 );
    m_UseLabels->SetValue(true);
    itemStaticBoxSizer4->Add(m_UseLabels, 0,
                             wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

    wxStaticBox* modifierGroupLabelBox =
        new wxStaticBox( itemDialog1, wxID_STATIC, _("Available Modifiers"),
                         wxDefaultPosition, wxDefaultSize, 0 );
    wxStaticBoxSizer* modifierGroup =
        new wxStaticBoxSizer(modifierGroupLabelBox, wxHORIZONTAL);
    itemStaticBoxSizer4->Add(modifierGroup, 0, wxGROW|wxALL, 5);

    wxBoxSizer* modifierColumn = new wxBoxSizer (wxVERTICAL);
    modifierGroup->Add(modifierColumn, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);
    int num_in_column = 0;
    for (size_t n = 0; n < m_Params->m_ModifierList.size(); n++) {
        if (m_Params->m_ModifierList[n].AnyPresent()) {
            if (num_in_column == 4) {
                modifierColumn = new wxBoxSizer (wxVERTICAL);
                modifierGroup->Add(modifierColumn, 0,
                                   wxALIGN_CENTER_HORIZONTAL|wxALL, 5);
                num_in_column = 0;
            }
            wxBoxSizer *row = new wxBoxSizer (wxHORIZONTAL);
            modifierColumn->Add (row, 0, wxALIGN_LEFT|wxALL, 5);
            m_ModifierButtons.push_back
                (new wxCheckBox( itemDialog1, ID_MODIFIER_CHECKBOX,
                                 ToWxString(m_Params->m_ModifierList[n].Label()),
                                 wxDefaultPosition, wxDefaultSize, 0 ));
            row->Add(m_ModifierButtons[n], 0, wxALIGN_LEFT|wxALL, 5);
            if (m_Params->m_ModifierList[n].IsRequested()) {
                m_ModifierButtons[n]->SetValue (true);
            }
            string label = s_GetModifierStatus (m_Params->m_ModifierList[n].AllPresent(), m_Params->m_ModifierList[n].AllUnique(), m_Params->m_ModifierList[n].IsUnique());
            string first_value;
            m_Params->m_ModifierList[n].FirstValue (first_value);
            if (first_value.length() > 25) {
                first_value = first_value.substr(0, 22) + "...";
            }
            label += " '" + first_value + "'";
            wxStaticText* textLabel =
                new wxStaticText( itemDialog1, wxID_STATIC,
                                  ToWxString(label),
                                  wxDefaultPosition, wxDefaultSize, 0 );
            row->Add(textLabel, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

            num_in_column++;
        } else {
            m_ModifierButtons.push_back(NULL);
        }
    }

    if (m_Params->m_NeedHIVRule) {
        wxArrayString HIVStrings;
        HIVStrings.Add(_("Prefer Clone"));
        HIVStrings.Add(_("Prefer Isolate"));
        HIVStrings.Add(_("Want Both Isolate and Clone"));

        m_HIVRule = new wxRadioBox (itemDialog1, wxID_ANY, _("HIV Rule"), wxDefaultPosition, wxDefaultSize,
                                    HIVStrings, 1, wxRA_SPECIFY_ROWS);
        itemStaticBoxSizer4->Add (m_HIVRule, 0, wxALIGN_LEFT|wxALL, 5);
    }

    wxStaticBox* otherOptionsGroupLabelBox = new wxStaticBox( itemDialog1, wxID_STATIC, _("Other Options"), wxDefaultPosition, wxDefaultSize, 0 );
    wxStaticBoxSizer* otherOptionsGroup = new wxStaticBoxSizer(otherOptionsGroupLabelBox, wxHORIZONTAL);
    itemStaticBoxSizer4->Add(otherOptionsGroup, 0, wxGROW|wxALL, 5);

    wxBoxSizer* otherOptionsColumn = new wxBoxSizer (wxVERTICAL);
    otherOptionsGroup->Add(otherOptionsColumn, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

    m_LeaveParen = new wxCheckBox( itemDialog1, ID_LEAVE_PARENTHETICAL, _("Leave in parenthetical organism info"), wxDefaultPosition, wxDefaultSize, 0 );
    m_LeaveParen->SetValue(true);
    otherOptionsColumn->Add(m_LeaveParen, 0, wxALIGN_LEFT|wxALL, 5);

    m_NoSp = new wxCheckBox( itemDialog1, ID_DO_NOT_APPLY_TO_SP, _("Do not apply modifiers to 'sp.' organisms"), wxDefaultPosition, wxDefaultSize, 0 );
    m_NoSp->SetValue(true);
    otherOptionsColumn->Add(m_NoSp, 0, wxALIGN_LEFT|wxALL, 5);

    m_NoNr = new wxCheckBox( itemDialog1, ID_DO_NOT_APPLY_TO_NR, _("Do not apply modifiers to 'nr.' organisms"), wxDefaultPosition, wxDefaultSize, 0 );
    m_NoNr->SetValue(true);
    otherOptionsColumn->Add(m_NoNr, 0, wxALIGN_LEFT|wxALL, 5);

    otherOptionsColumn = new wxBoxSizer (wxVERTICAL);
    otherOptionsGroup->Add(otherOptionsColumn, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

    m_IncludeCountry = new wxCheckBox( itemDialog1, ID_INCLUDE_COUNTRY_TEXT, _("Include text after colon in country"), wxDefaultPosition, wxDefaultSize, 0 );
    m_IncludeCountry->SetValue(false);
    otherOptionsColumn->Add(m_IncludeCountry, 0, wxALIGN_LEFT|wxALL, 5);

    m_NoCf = new wxCheckBox( itemDialog1, ID_DO_NOT_APPLY_TO_CF, _("Do not apply modifiers to 'cf.' organisms"), wxDefaultPosition, wxDefaultSize, 0 );
    m_NoCf->SetValue(true);
    otherOptionsColumn->Add(m_NoCf, 0, wxALIGN_LEFT|wxALL, 5);

    m_NoAff = new wxCheckBox( itemDialog1, ID_DO_NOT_APPLY_TO_AFF, _("Do not apply modifiers to 'aff.' organisms"), wxDefaultPosition, wxDefaultSize, 0 );
    m_NoAff->SetValue(true);
    otherOptionsColumn->Add(m_NoAff, 0, wxALIGN_LEFT|wxALL, 5);

    otherOptionsColumn = new wxBoxSizer (wxVERTICAL);
    otherOptionsGroup->Add(otherOptionsColumn, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

    m_KeepAfterSemicolon = new wxCheckBox (itemDialog1, ID_KEEP_AFTER_SEMICOLON, _("Include text after semicolon in modifiers"), wxDefaultPosition, wxDefaultSize, 0 );
    m_KeepAfterSemicolon->SetValue (false);
    otherOptionsColumn->Add(m_KeepAfterSemicolon, 0, wxALIGN_LEFT|wxALL, 5);

    wxStaticBox* itemStaticBoxSizer17Static = new wxStaticBox(itemDialog1, wxID_ANY, _("Feature Clauses"));
    wxStaticBoxSizer* featureBox = new wxStaticBoxSizer(itemStaticBoxSizer17Static, wxVERTICAL);
    itemBoxSizer2->Add(featureBox, 0, wxGROW|wxALL, 5);

    wxBoxSizer* featureRow1 = new wxBoxSizer (wxHORIZONTAL);
    featureBox->Add (featureRow1, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

    wxBoxSizer* featureListGroup = new wxBoxSizer (wxVERTICAL);
    featureRow1->Add (featureListGroup, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

    wxBoxSizer* itemBoxSizer18 = new wxBoxSizer(wxHORIZONTAL);
    featureListGroup->Add(itemBoxSizer18, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

    wxStaticText* itemStaticText19 = new wxStaticText( itemDialog1, wxID_STATIC, _("Features or Complete"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer18->Add(itemStaticText19, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxArrayString m_FeatureListChoiceStrings;
    m_FeatureListChoiceStrings.Add(_("List Features"));
    m_FeatureListChoiceStrings.Add(_("Complete Sequence"));
    m_FeatureListChoiceStrings.Add(_("Complete Genome"));
    m_FeatureListChoice = new wxChoice( itemDialog1, ID_LIST_FEATURES, wxDefaultPosition, wxDefaultSize, m_FeatureListChoiceStrings, 0 );
    m_FeatureListChoice->SetStringSelection(_("List Features"));
    itemBoxSizer18->Add(m_FeatureListChoice, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxArrayString m_ProductClauseChoiceStrings;
    m_ProductClauseChoiceStrings.Add(_("No mitochondrial or chloroplast suffix"));
    m_ProductClauseChoiceStrings.Add(_("Nuclear gene(s) for mitochondrial product(s)"));
    m_ProductClauseChoiceStrings.Add(_("Nuclear gene(s) for chloroplast product(s)"));
    m_ProductClauseChoiceStrings.Add(_("Nuclear gene(s) for kinetoplast product(s)"));
    m_ProductClauseChoiceStrings.Add(_("Nuclear gene(s) for plastid product(s)"));
    m_ProductClauseChoiceStrings.Add(_("Nuclear gene(s) for chromoplast product(s)"));
    m_ProductClauseChoiceStrings.Add(_("Nuclear gene(s) for cyanelle product(s)"));
    m_ProductClauseChoiceStrings.Add(_("Nuclear gene(s) for apicoplast product(s)"));
    m_ProductClauseChoiceStrings.Add(_("Nuclear gene(s) for leucoplast product(s)"));
    m_ProductClauseChoiceStrings.Add(_("Nuclear gene(s) for proplastid product(s)"));
    m_ProductClauseChoiceStrings.Add(_("Nuclear genes based on CDS products"));
    m_ProductClauseChoiceStrings.Add(_("Nuclear copy of mitochondrial gene"));
    m_ProductClauseChoiceStrings.Add(_("Nuclear copy of chloroplast gene"));
    m_ProductClauseChoiceStrings.Add(_("Nuclear copy of kinetoplast gene"));
    m_ProductClauseChoiceStrings.Add(_("Nuclear copy of plastid gene"));
    m_ProductClauseChoiceStrings.Add(_("Nuclear copy of chromoplast gene"));
    m_ProductClauseChoiceStrings.Add(_("Nuclear copy of cyanelle gene"));
    m_ProductClauseChoiceStrings.Add(_("Nuclear copy of apicoplast gene"));
    m_ProductClauseChoiceStrings.Add(_("Nuclear copy of leucoplast gene"));
    m_ProductClauseChoiceStrings.Add(_("Nuclear copy of proplastid gene"));
    m_ProductClauseChoice = new wxChoice( itemDialog1, ID_PRODUCT_NAME, wxDefaultPosition, wxDefaultSize, m_ProductClauseChoiceStrings, 0 );
    m_ProductClauseChoice->SetSelection(10);
    featureListGroup->Add(m_ProductClauseChoice, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    m_AppendAltSplice = new wxCheckBox( itemDialog1, ID_ADD_ALT_SPLICE, _("Append \"alternatively spliced\" to definition line"), wxDefaultPosition, wxDefaultSize, 0 );
    m_AppendAltSplice->SetValue(false);
    featureListGroup->Add(m_AppendAltSplice, 0, wxALIGN_LEFT|wxALL, 5);

    m_UseNcRNAComment = new wxCheckBox( itemDialog1, ID_USE_NCRNA_COMMENT, _("Use ncRNA note if no class or product"), wxDefaultPosition, wxDefaultSize, 0 );
    m_UseNcRNAComment->SetValue(false);
    featureListGroup->Add(m_UseNcRNAComment, 0, wxALIGN_LEFT|wxALL, 5);

    wxStaticBox* optionalFeaturesBoxLabel = new wxStaticBox(itemDialog1, wxID_ANY, _("Optional Features"));
    wxStaticBoxSizer* optionalFeaturesBox = new wxStaticBoxSizer(optionalFeaturesBoxLabel, wxHORIZONTAL);
    featureRow1->Add(optionalFeaturesBox, 0, wxGROW|wxALL, 5);

    wxBoxSizer* optionalFeaturesColumn = new wxBoxSizer (wxVERTICAL);
    optionalFeaturesBox->Add (optionalFeaturesColumn, 0, wxALIGN_LEFT|wxALL, 5);

    m_KeepExons = new wxCheckBox( itemDialog1, ID_INCLUDE_EXONS, _("Exons"), wxDefaultPosition, wxDefaultSize, 0 );
    m_KeepExons->SetValue(false);
    optionalFeaturesColumn->Add(m_KeepExons, 0, wxALIGN_LEFT|wxALL, 5);

    m_KeepIntrons = new wxCheckBox( itemDialog1, ID_INCLUDE_INTRONS, _("Introns"), wxDefaultPosition, wxDefaultSize, 0 );
    m_KeepIntrons->SetValue(false);
    optionalFeaturesColumn->Add(m_KeepIntrons, 0, wxALIGN_LEFT|wxALL, 5);

    m_Keep5UTRs = new wxCheckBox( itemDialog1, ID_INCLUDE_5UTRS, _("5' UTRs"), wxDefaultPosition, wxDefaultSize, 0 );
    m_Keep5UTRs->SetValue(false);
    optionalFeaturesColumn->Add(m_Keep5UTRs, 0, wxALIGN_LEFT|wxALL, 5);

    m_Keep3UTRs = new wxCheckBox( itemDialog1, ID_INCLUDE_3UTRS, _("3' UTRs"), wxDefaultPosition, wxDefaultSize, 0 );
    m_Keep3UTRs->SetValue(false);
    optionalFeaturesColumn->Add(m_Keep3UTRs, 0, wxALIGN_LEFT|wxALL, 5);

    optionalFeaturesColumn = new wxBoxSizer (wxVERTICAL);
    optionalFeaturesBox->Add (optionalFeaturesColumn, 0, wxALIGN_LEFT|wxALL, 5);

    m_KeepRegulatoryFeatures = new wxCheckBox(itemDialog1, ID_INCLUDE_REGULATORY_FEATURES, _("Regulatory Features"), wxDefaultPosition, wxDefaultSize, 0);
    m_KeepRegulatoryFeatures->SetValue(false);
    optionalFeaturesColumn->Add(m_KeepRegulatoryFeatures, 0, wxALIGN_LEFT | wxALL, 5);
    m_UseFakePromoters = new wxCheckBox(itemDialog1, ID_USE_FAKE_PROMOTERS, _("Add Promoters"), wxDefaultPosition, wxDefaultSize, 0);
    m_UseFakePromoters->SetValue(false);
    optionalFeaturesColumn->Add(m_UseFakePromoters, 0, wxALIGN_LEFT | wxALL, 5);

    optionalFeaturesColumn = new wxBoxSizer (wxVERTICAL);
    optionalFeaturesBox->Add (optionalFeaturesColumn, 0, wxALIGN_LEFT|wxALL, 5);

    m_KeepMiscFeats = new wxCheckBox( itemDialog1, ID_INCLUDE_MISC_FEATS, _("Misc_feats with comments"), wxDefaultPosition, wxDefaultSize, 0 );
    m_KeepMiscFeats->SetValue(false);
    optionalFeaturesColumn->Add(m_KeepMiscFeats, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxArrayString m_MiscFeatParseRuleStrings;
    m_MiscFeatParseRuleStrings.Add(_("Look for Noncoding Products"));
    m_MiscFeatParseRuleStrings.Add(_("Use comment before first semicolon"));
    m_MiscFeatParseRule = new wxChoice( itemDialog1, ID_MISC_FEAT_RULE, wxDefaultPosition, wxDefaultSize, m_MiscFeatParseRuleStrings, 0 );
    m_MiscFeatParseRule->SetSelection(0);
    optionalFeaturesColumn->Add(m_MiscFeatParseRule, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);
    m_MiscFeatParseRule->Disable();

    wxBoxSizer* featureRow2 = new wxBoxSizer (wxHORIZONTAL);
    featureBox->Add (featureRow2, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

    wxStaticBox* suppressFeaturesBoxLabel = new wxStaticBox(itemDialog1, wxID_ANY, _("Suppress Features"));
    wxStaticBoxSizer* suppressFeaturesBox = new wxStaticBoxSizer(suppressFeaturesBoxLabel, wxHORIZONTAL);
    featureRow2->Add(suppressFeaturesBox, 0, wxGROW|wxALL, 5);

    wxButton* itemButton39 = new wxButton( itemDialog1, ID_SUPPRESS_FEATURES, _("Suppress Features"), wxDefaultPosition, wxDefaultSize, 0 );
    suppressFeaturesBox->Add(itemButton39, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

    wxStaticBox* suppressOptionsBoxLabel = new wxStaticBox(itemDialog1, wxID_ANY, _("Suppress"));
    wxStaticBoxSizer* suppressOptionsBox = new wxStaticBoxSizer(suppressOptionsBoxLabel, wxVERTICAL);
    featureRow2->Add(suppressOptionsBox, 0, wxGROW|wxALL, 5);

    m_SuppressTransposon = new wxCheckBox( itemDialog1, ID_SUPPRESS_TRANSPOSON, _("Mobile element subfeatures"), wxDefaultPosition, wxDefaultSize, 0 );
    m_SuppressTransposon->SetValue(false);
    suppressOptionsBox->Add(m_SuppressTransposon, 0, wxALIGN_LEFT|wxALL, 5);

    m_SupressGeneCluster = new wxCheckBox( itemDialog1, ID_SUPPRESS_GENE_CLUSTER, _("Gene cluster/locus subfeatures (both strands)"), wxDefaultPosition, wxDefaultSize, 0 );
    m_SupressGeneCluster->SetValue(false);
    suppressOptionsBox->Add(m_SupressGeneCluster, 0, wxALIGN_LEFT|wxALL, 5);

    m_SuppressLocusTags = new wxCheckBox( itemDialog1, ID_SUPPRESS_LOCUS_TAGS, _("Locus tags"), wxDefaultPosition, wxDefaultSize, 0 );
    m_SuppressLocusTags->SetValue(false);
    suppressOptionsBox->Add(m_SuppressLocusTags, 0, wxALIGN_LEFT|wxALL, 5);

    m_SuppressAltSplice = new wxCheckBox( itemDialog1, ID_SUPPRESS_ALTERNATIVE_SPLICE, _("Alternative splice phrase"), wxDefaultPosition, wxDefaultSize, 0 );
    m_SuppressAltSplice->SetValue(false);
    suppressOptionsBox->Add(m_SuppressAltSplice, 0, wxALIGN_LEFT|wxALL, 5);


////@end CAutodefParamsPanel content construction
}


/*!
 * Should we show tooltips?
 */

bool CAutodefParamsPanel::ShowToolTips()
{
    return true;
}

/*!
 * Get bitmap resources
 */

wxBitmap CAutodefParamsPanel::GetBitmapResource( const wxString& name )
{
    // Bitmap retrieval
////@begin CAutodefParamsPanel bitmap retrieval
    wxUnusedVar(name);
    return wxNullBitmap;
////@end CAutodefParamsPanel bitmap retrieval
}

/*!
 * Get icon resources
 */

wxIcon CAutodefParamsPanel::GetIconResource( const wxString& name )
{
    // Icon retrieval
////@begin CAutodefParamsPanel icon retrieval
    wxUnusedVar(name);
    return wxNullIcon;
////@end CAutodefParamsPanel icon retrieval
}


void CAutodefParamsPanel::SetParams(SAutodefParams* params,
                                            TConstScopedObjects* objects)
{
    m_Params = params;
    m_InputObjects = objects;
}


bool CAutodefParamsPanel::TransferDataToWindow()
{
    if(m_InputListDirty)    {
        m_ObjectListCtrl->SetObjects(*m_InputObjects);
        m_ObjectListCtrl->SelectAll();
        m_InputListDirty = false;
    }

    m_UseLabels->SetValue (m_Params->m_UseLabels);

    for (size_t n = 0; n < m_Params->m_ModifierList.size(); n++) {
        if (m_ModifierButtons[n] != NULL) {
            m_ModifierButtons[n]->SetValue (m_Params->m_ModifierList[n].IsRequested());
        }
    }

    m_NoSp->SetValue (m_Params->m_DoNotApplyToSp);
    m_NoNr->SetValue (m_Params->m_DoNotApplyToNr);
    m_NoCf->SetValue (m_Params->m_DoNotApplyToCf);
    m_NoAff->SetValue (m_Params->m_DoNotApplyToAff);

    if (m_HIVRule) {
        m_HIVRule->SetSelection (m_Params->m_HIVRule);
    }

    m_MaxModifiers->SetSelection (m_Params->m_MaxMods);
    m_LeaveParen->SetValue (m_Params->m_LeaveParenthetical);
    m_IncludeCountry->SetValue (m_Params->m_IncludeCountryText);
    m_KeepAfterSemicolon->SetValue (m_Params->m_KeepAfterSemicolon);
    m_SuppressAltSplice->SetValue (m_Params->m_SuppressFeatureAltSplice);
    m_UseNcRNAComment->SetValue (m_Params->m_UseNcRNAComment);
    m_SuppressTransposon->SetValue (m_Params->m_SuppressMobileElementSubfeatures);
    m_SupressGeneCluster->SetValue (m_Params->m_GeneClusterOppStrand);
    m_SuppressLocusTags->SetValue (m_Params->m_SuppressLocusTags);
    m_KeepExons->SetValue (m_Params->m_KeepExons);
    m_Keep5UTRs->SetValue (m_Params->m_Keep5UTRs);
    m_KeepIntrons->SetValue (m_Params->m_KeepIntrons);
    m_Keep3UTRs->SetValue (m_Params->m_Keep3UTRs);
    m_KeepRegulatoryFeatures->SetValue(m_Params->m_KeepRegulatoryFeatures);
    m_UseFakePromoters->SetValue(m_Params->m_UseFakePromoters);

    switch (m_Params->m_MiscFeatRule) {
        case CAutoDefOptions::eNoncodingProductFeat:
            m_KeepMiscFeats->SetValue (true);
            m_MiscFeatParseRule->SetSelection (0);
            break;
        case CAutoDefOptions::eCommentFeat:
            m_KeepMiscFeats->SetValue (true);
            m_MiscFeatParseRule->SetSelection (1);
            break;
        case CAutoDefOptions::eDelete:
        default:
            m_KeepMiscFeats->SetValue (false);
            m_MiscFeatParseRule->SetSelection (0);
            break;
    }

    if (m_Params->m_SpecifyNuclearProduct) {
        m_ProductClauseChoice->SetSelection (10);
    } else {
        m_ProductClauseChoice->SetSelection(0);
        if (m_Params->m_ProductFlag > 0) {
            switch (m_Params->m_ProductFlag) {
            case CBioSource::eGenome_mitochondrion:
                m_ProductClauseChoice->SetSelection(1);
                break;
            case CBioSource::eGenome_chloroplast:
                m_ProductClauseChoice->SetSelection(2);
                break;
            case CBioSource::eGenome_kinetoplast:
                m_ProductClauseChoice->SetSelection(3);
                break;
            case CBioSource::eGenome_plastid:
                m_ProductClauseChoice->SetSelection(4);
                break;
            case CBioSource::eGenome_chromoplast:
                m_ProductClauseChoice->SetSelection(5);
                break;
            case CBioSource::eGenome_cyanelle:
                m_ProductClauseChoice->SetSelection(6);
                break;
            case CBioSource::eGenome_apicoplast:
                m_ProductClauseChoice->SetSelection(7);
                break;
            case CBioSource::eGenome_leucoplast:
                m_ProductClauseChoice->SetSelection(8);
                break;
            case CBioSource::eGenome_proplastid:
                m_ProductClauseChoice->SetSelection(9);
                break;
            }
        }
        else if (m_Params->m_NuclearCopyFlag > 0) {
            switch (m_Params->m_NuclearCopyFlag) {
            case CBioSource::eGenome_mitochondrion:
                m_ProductClauseChoice->SetSelection(11);
                break;
            case CBioSource::eGenome_chloroplast:
                m_ProductClauseChoice->SetSelection(12);
                break;
            case CBioSource::eGenome_kinetoplast:
                m_ProductClauseChoice->SetSelection(13);
                break;
            case CBioSource::eGenome_plastid:
                m_ProductClauseChoice->SetSelection(14);
                break;
            case CBioSource::eGenome_chromoplast:
                m_ProductClauseChoice->SetSelection(15);
                break;
            case CBioSource::eGenome_cyanelle:
                m_ProductClauseChoice->SetSelection(16);
                break;
            case CBioSource::eGenome_apicoplast:
                m_ProductClauseChoice->SetSelection(17);
                break;
            case CBioSource::eGenome_leucoplast:
                m_ProductClauseChoice->SetSelection(18);
                break;
            case CBioSource::eGenome_proplastid:
                m_ProductClauseChoice->SetSelection(19);
                break;
            }
        }
    }
    m_AppendAltSplice->SetValue (m_Params->m_AltSpliceFlag);
    m_FeatureListChoice->SetSelection (m_Params->m_FeatureListType);

    // suppressed features
    m_SuppressedFeatures = m_Params->m_SuppressedFeatures;
//TODO - the rest of the values
    return wxPanel::TransferDataToWindow();
}


bool CAutodefParamsPanel::TransferDataFromWindow()
{
    if(wxPanel::TransferDataFromWindow())   {
        m_ObjectListCtrl->GetSelection(m_Params->m_SeqEntries);

        m_Params->m_UseLabels = m_UseLabels->GetValue();

        for (size_t n = 0; n < m_Params->m_ModifierList.size(); n++) {
            if (m_ModifierButtons[n] == NULL) {
                m_Params->m_ModifierList[n].SetRequested(false);
            } else {
                m_Params->m_ModifierList[n].SetRequested (m_ModifierButtons[n]->GetValue ());
            }
        }

        m_Params->m_DoNotApplyToSp = m_NoSp->GetValue ();
        m_Params->m_DoNotApplyToNr = m_NoNr->GetValue ();
        m_Params->m_DoNotApplyToCf = m_NoCf->GetValue ();
        m_Params->m_DoNotApplyToAff = m_NoAff->GetValue ();

        if (m_HIVRule) {
            m_Params->m_HIVRule = m_HIVRule->GetSelection ();
        }

        m_Params->m_MaxMods = m_MaxModifiers->GetSelection ();

        m_Params->m_LeaveParenthetical = m_LeaveParen->GetValue();
        m_Params->m_IncludeCountryText = m_IncludeCountry->GetValue();
        m_Params->m_KeepAfterSemicolon = m_KeepAfterSemicolon->GetValue();

        m_Params->m_SuppressFeatureAltSplice = m_SuppressAltSplice->GetValue();
        m_Params->m_SuppressMobileElementSubfeatures = m_SuppressTransposon->GetValue ();
        m_Params->m_GeneClusterOppStrand = m_SupressGeneCluster->GetValue ();
        m_Params->m_SuppressLocusTags = m_SuppressLocusTags->GetValue ();
        m_Params->m_KeepExons = m_KeepExons->GetValue ();
        m_Params->m_Keep5UTRs = m_Keep5UTRs->GetValue ();
        m_Params->m_KeepIntrons = m_KeepIntrons->GetValue ();
        m_Params->m_Keep3UTRs = m_Keep3UTRs->GetValue ();
        m_Params->m_UseFakePromoters = m_UseFakePromoters->GetValue();
        m_Params->m_KeepRegulatoryFeatures = m_KeepRegulatoryFeatures->GetValue();

        if (m_KeepMiscFeats->GetValue ()) {
            if (m_MiscFeatParseRule->GetSelection() == 0) {
                m_Params->m_MiscFeatRule = CAutoDefOptions::eNoncodingProductFeat;
            } else {
                m_Params->m_MiscFeatRule = CAutoDefOptions::eCommentFeat;
            }
        } else {
            m_Params->m_MiscFeatRule = CAutoDefOptions::eDelete;
        }

        int selected = m_ProductClauseChoice->GetSelection();

        if (selected == 10) {
            m_Params->m_SpecifyNuclearProduct = true;
        }
        else {
            m_Params->m_SpecifyNuclearProduct = false;
            m_Params->m_ProductFlag = objects::CBioSource::eGenome_unknown;
            m_Params->m_NuclearCopyFlag = objects::CBioSource::eGenome_unknown;
            if (selected < 10) {
                switch (selected) {
                case 1:
                    m_Params->m_ProductFlag = CBioSource::eGenome_mitochondrion;
                    break;
                case 2:
                    m_Params->m_ProductFlag = CBioSource::eGenome_chloroplast;
                    break;
                case 3:
                    m_Params->m_ProductFlag = CBioSource::eGenome_kinetoplast;
                    break;
                case 4:
                    m_Params->m_ProductFlag = CBioSource::eGenome_plastid;
                    break;
                case 5:
                    m_Params->m_ProductFlag = CBioSource::eGenome_chromoplast;
                    break;
                case 6:
                    m_Params->m_ProductFlag = CBioSource::eGenome_cyanelle;
                    break;
                case 7:
                    m_Params->m_ProductFlag = CBioSource::eGenome_apicoplast;
                    break;
                case 8:
                    m_Params->m_ProductFlag = CBioSource::eGenome_leucoplast;
                    break;
                case 9:
                    m_Params->m_ProductFlag = CBioSource::eGenome_proplastid;
                    break;
                }
            }
            else if (selected > 10) {
                switch (selected) {
                case 10:
                    m_Params->m_NuclearCopyFlag = objects::CBioSource::eGenome_mitochondrion;
                    break;
                case 11:
                    m_Params->m_NuclearCopyFlag = objects::CBioSource::eGenome_chloroplast;
                    break;
                case 12:
                    m_Params->m_NuclearCopyFlag = objects::CBioSource::eGenome_kinetoplast;
                    break;
                case 13:
                    m_Params->m_NuclearCopyFlag = objects::CBioSource::eGenome_plastid;
                    break;
                case 14:
                    m_Params->m_NuclearCopyFlag = objects::CBioSource::eGenome_chromoplast;
                    break;
                case 15:
                    m_Params->m_NuclearCopyFlag = objects::CBioSource::eGenome_cyanelle;
                    break;
                case 16:
                    m_Params->m_NuclearCopyFlag = objects::CBioSource::eGenome_apicoplast;
                    break;
                case 17:
                    m_Params->m_NuclearCopyFlag = objects::CBioSource::eGenome_leucoplast;
                    break;
                case 18:
                    m_Params->m_NuclearCopyFlag = objects::CBioSource::eGenome_proplastid;
                    break;
                }
            }
        }

        m_Params->m_AltSpliceFlag = m_AppendAltSplice->GetValue ();
        m_Params->m_UseNcRNAComment = m_UseNcRNAComment->GetValue();
        m_Params->m_FeatureListType = m_FeatureListChoice->GetSelection ();

        m_Params->m_SuppressedFeatures = m_SuppressedFeatures;

//TODO - the rest of  the values
        return true;
    }
    return false;
}


void CAutodefParamsPanel::SetRegistryPath(const string& path)
{
    m_RegPath = path; // store for later use
}


static const char* kTableTag = "Table";

void CAutodefParamsPanel::SaveSettings() const
{
    if( ! m_RegPath.empty())   {
        CGuiRegistry& gui_reg = CGuiRegistry::GetInstance();
        string reg_path = CGuiRegistryUtil::MakeKey(m_RegPath, kTableTag);

        CRegistryWriteView table_view =
            gui_reg.GetWriteView(reg_path);
    }
}


void CAutodefParamsPanel::LoadSettings()
{
    if( ! m_RegPath.empty())   {
        CGuiRegistry& gui_reg = CGuiRegistry::GetInstance();
        string reg_path = CGuiRegistryUtil::MakeKey(m_RegPath, kTableTag);

        CRegistryReadView table_view = gui_reg.GetReadView(reg_path);
    }
}


void CAutodefParamsPanel::OnChangeMiscFeat(wxCommandEvent& event)
{
    if (m_MiscFeatParseRule) {
        if (m_KeepMiscFeats && m_KeepMiscFeats->GetValue ()) {
            m_MiscFeatParseRule->Enable();
        } else {
            m_MiscFeatParseRule->Disable();
        }
    }
}


void CAutodefParamsPanel::OnSuppressFeatures(wxCommandEvent& event)
{
    CFeatureCheckDialog dlg(NULL);
    dlg.SetSelected (m_SuppressedFeatures);
    if (dlg.ShowModal() == wxID_OK) {
        m_SuppressedFeatures.clear();
        dlg.GetSelected(m_SuppressedFeatures);
    }
}


void CAutodefParamsPanel::OnChangeModifierListChoice(wxCommandEvent& event)
{
    if (m_ModifierListChoice->GetSelection() == 1) {
        for (size_t n = 0; n < m_ModifierButtons.size(); n++) {
            if (m_ModifierButtons[n] != NULL) {
                m_ModifierButtons[n]->Disable();
            }
        }
    } else {
        for (size_t n = 0; n < m_ModifierButtons.size(); n++) {
            if (m_ModifierButtons[n] != NULL) {
                m_ModifierButtons[n]->Enable();
            }
        }
    }
}


bool CAutodefParamsPanel::x_AllowHIVRule()
{
    bool allow_hiv_rule = true;

    for (size_t n = 0;
         n < m_ModifierButtons.size() && n < m_Params->m_ModifierList.size() && allow_hiv_rule;
         n++) {
        if (m_ModifierButtons[n] != NULL
            && m_ModifierButtons[n]->GetValue()
            && ((m_Params->m_ModifierList[n].IsOrgMod()
                 && m_Params->m_ModifierList[n].GetOrgModType() == COrgMod::eSubtype_isolate)
                || (!m_Params->m_ModifierList[n].IsOrgMod()
                    && m_Params->m_ModifierList[n].GetSubSourceType() == CSubSource::eSubtype_clone))) {
            allow_hiv_rule = false;
        }
    }
    return allow_hiv_rule;
}


void CAutodefParamsPanel::OnChangeModifierSelection(wxCommandEvent& event)
{
    if (m_HIVRule != NULL) {
        if (x_AllowHIVRule()) {
            m_HIVRule->Enable();
        } else {
            m_HIVRule->Disable();
        }
    }
}

void CAutodefParamsPanel::RestoreDefaults()
{
    if (!m_Params)
        return;
    m_Params->Init();
    TransferDataToWindow();
}

END_NCBI_SCOPE
