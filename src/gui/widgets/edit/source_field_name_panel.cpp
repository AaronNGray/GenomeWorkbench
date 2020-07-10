/*  $Id: source_field_name_panel.cpp 44581 2020-01-24 20:57:23Z asztalos $
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
#include <objects/seqfeat/BioSource.hpp>
#include <objects/seqfeat/SubSource.hpp>
#include <objects/seqfeat/OrgMod.hpp>
#include <objtools/edit/field_handler.hpp>

#include <gui/objutils/macro_biodata.hpp>
#include <gui/objutils/macro_fn_where.hpp>
#include <gui/objutils/macro_util.hpp>
#include <gui/widgets/edit/field_type_constants.hpp>
#include <gui/widgets/edit/source_field_name_panel.hpp>
#include <gui/widgets/wx/wx_utils.hpp>
#include <wx/radiobox.h>

BEGIN_NCBI_SCOPE
USING_SCOPE(objects);

/*!
 * CSourceFieldNamePanel type definition
 */

IMPLEMENT_DYNAMIC_CLASS( CSourceFieldNamePanel, CFieldNamePanel )


/*!
 * CSourceFieldNamePanel event table definition
 */

BEGIN_EVENT_TABLE( CSourceFieldNamePanel, CFieldNamePanel )

////@begin CSourceFieldNamePanel event table entries
    EVT_RADIOBUTTON( ID_CSOURCEFIELD_TEXT_QUAL_BTN, CSourceFieldNamePanel::OnTextQualBtnSelected)

    EVT_RADIOBUTTON( ID_CSOURCEFIELD_TAXONOMY_BTN, CSourceFieldNamePanel::OnTaxonomyBtnSelected)

    EVT_RADIOBUTTON( ID_CSOURCEFIELD_LOCATION_BTN, CSourceFieldNamePanel::OnLocationBtnSelected)

    EVT_RADIOBUTTON( ID_CSOURCEFIELD_ORIGIN_BTN, CSourceFieldNamePanel::OnOriginBtnSelected)

    EVT_LISTBOX( ID_CSOURCEFIELD_SRC_SUBLIST, CSourceFieldNamePanel::OnSrcSublistSelected)

////@end CSourceFieldNamePanel event table entries

END_EVENT_TABLE()


/*!
 * CSourceFieldNamePanel constructors
 */

CSourceFieldNamePanel::CSourceFieldNamePanel()
{
    Init();
}

CSourceFieldNamePanel::CSourceFieldNamePanel( wxWindow* parent, bool additional_items, wxWindowID id, const wxPoint& pos, const wxSize& size, long style )
    : m_AdditionalItems(additional_items), m_DoNotShowSubList(false), m_is_taxonomy_panel(false), m_first_run(true)
{
    Init();
    Create(parent, id, pos, size, style);
}


/*!
 * CSourceFieldNamePanel creator
 */

bool CSourceFieldNamePanel::Create( wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style )
{
////@begin CSourceFieldNamePanel creation
    CFieldNamePanel::Create( parent, id, pos, size, style );

    CreateControls();
    if (GetSizer())
    {
        GetSizer()->SetSizeHints(this);
    }
    Centre();
////@end CSourceFieldNamePanel creation
    return true;
}


/*!
 * CSourceFieldNamePanel destructor
 */

CSourceFieldNamePanel::~CSourceFieldNamePanel()
{
////@begin CSourceFieldNamePanel destruction
////@end CSourceFieldNamePanel destruction
}


/*!
 * Member initialisation
 */

void CSourceFieldNamePanel::Init()
{
////@begin CSourceFieldNamePanel member initialisation
    m_TextQualifier = NULL;
    m_Taxonomy = NULL;
    m_Location = NULL;
    m_Origin = NULL;
    m_SourceSubList = NULL;
////@end CSourceFieldNamePanel member initialisation
}


/*!
 * Control creation for CSourceFieldNamePanel
 */

void CSourceFieldNamePanel::CreateControls()
{    
////@begin CSourceFieldNamePanel content construction
    CSourceFieldNamePanel* itemCFieldNamePanel1 = this;

    wxBoxSizer* itemBoxSizer2 = new wxBoxSizer(wxVERTICAL);
    itemCFieldNamePanel1->SetSizer(itemBoxSizer2);

    wxBoxSizer* itemBoxSizer3 = new wxBoxSizer(wxHORIZONTAL);
    itemBoxSizer2->Add(itemBoxSizer3, 0, wxALIGN_LEFT|wxLEFT|wxRIGHT, 0);

    wxBoxSizer* itemBoxSizer4 = new wxBoxSizer(wxVERTICAL);
    itemBoxSizer3->Add(itemBoxSizer4, 0, wxALIGN_TOP|wxLEFT|wxRIGHT, 5);

    m_TextQualifier = new wxRadioButton(itemCFieldNamePanel1, ID_CSOURCEFIELD_TEXT_QUAL_BTN, _("Text Qualifier"), wxDefaultPosition, wxDefaultSize, 0);
    m_TextQualifier->SetValue(true);
    itemBoxSizer4->Add(m_TextQualifier, 0, wxALIGN_LEFT|wxALL, 5);

    m_Taxonomy = new wxRadioButton(itemCFieldNamePanel1, ID_CSOURCEFIELD_TAXONOMY_BTN, _("Taxonomy"), wxDefaultPosition, wxDefaultSize, 0);
    m_Taxonomy->SetValue(false);
    itemBoxSizer4->Add(m_Taxonomy, 0, wxALIGN_LEFT|wxALL, 5);

    m_Location = new wxRadioButton(itemCFieldNamePanel1, ID_CSOURCEFIELD_LOCATION_BTN, _("Location"), wxDefaultPosition, wxDefaultSize, 0);
    m_Location->SetValue(false);
    itemBoxSizer4->Add(m_Location, 0, wxALIGN_LEFT|wxALL, 5);

    m_Origin = new wxRadioButton(itemCFieldNamePanel1, ID_CSOURCEFIELD_ORIGIN_BTN, _("Origin"), wxDefaultPosition, wxDefaultSize, 0);
    m_Origin->SetValue(false);
    itemBoxSizer4->Add(m_Origin, 0, wxALIGN_LEFT|wxALL, 5);

    wxArrayString m_SourceSubListStrings;
    m_SourceSubList = new wxListBox(itemCFieldNamePanel1, ID_CSOURCEFIELD_SRC_SUBLIST, wxDefaultPosition, wxSize(220, 109), m_SourceSubListStrings, wxLB_SINGLE);
    itemBoxSizer3->Add(m_SourceSubList, 0, wxGROW|wxLEFT|wxRIGHT, 5);

    wxArrayString m_BioSourceTypes;
    m_BioSourceTypes.Add(_("&All"));
    m_BioSourceTypes.Add(_("&Descriptors"));
    m_BioSourceTypes.Add(_("&Features"));
    m_SourceType = new wxRadioBox(itemCFieldNamePanel1, ID_CSOURCEFIELD_SRCTYPES, wxEmptyString, wxDefaultPosition, wxDefaultSize, m_BioSourceTypes, 1, wxRA_SPECIFY_ROWS);
    m_SourceType->SetSelection(0);
    itemBoxSizer2->Add(m_SourceType, 0, wxGROW | wxLEFT | wxRIGHT, 5);

////@end CSourceFieldNamePanel content construction
    x_ChooseTextQualifier();

#ifdef __WXMAC__
    m_SourceSubList->EnsureVisible(0);
#endif

}

void CSourceFieldNamePanel::ClearValues()
{
    if (m_is_taxonomy_panel)
    {
        m_TextQualifier->SetValue(false);
        m_Taxonomy->SetValue(true);
        m_Location->SetValue(false);
        m_Origin->SetValue(false);
        x_ChooseTaxonomy();
    }
    else
    {
        m_TextQualifier->SetValue(true);
        m_Taxonomy->SetValue(false);
        m_Location->SetValue(false);
        m_Origin->SetValue(false);
        x_ChooseTextQualifier();
    }
    m_SourceSubList->SetSelection(wxNOT_FOUND);
    m_SourceSubList->SetFirstItem(0);
    m_SourceType->SetSelection(0);
}

/*!
 * Should we show tooltips?
 */

bool CSourceFieldNamePanel::ShowToolTips()
{
    return true;
}

/*!
 * Get bitmap resources
 */

wxBitmap CSourceFieldNamePanel::GetBitmapResource( const wxString& name )
{
    // Bitmap retrieval
////@begin CSourceFieldNamePanel bitmap retrieval
    wxUnusedVar(name);
    return wxNullBitmap;
////@end CSourceFieldNamePanel bitmap retrieval
}

/*!
 * Get icon resources
 */

wxIcon CSourceFieldNamePanel::GetIconResource( const wxString& name )
{
    // Icon retrieval
////@begin CSourceFieldNamePanel icon retrieval
    wxUnusedVar(name);
    return wxNullIcon;
////@end CSourceFieldNamePanel icon retrieval
}

void CSourceFieldNamePanel::HideSourceType()
{
    m_SourceType->Show(false);
}

static const string kInst_suffix("-inst");
static const string kColl_suffix("-coll");
static const string kSpecid_suffix("-specid");

string CSourceFieldNamePanel::GetFieldName(const bool subfield)
{
    string field = "";
    if (m_TextQualifier->GetValue() || m_Taxonomy->GetValue()) {
        int val = m_SourceSubList->GetSelection();
        if (val > -1) {
            field = ToStdString(m_SourceSubList->GetString(val));
        }  
    } else if  ( m_Location->GetValue() ) {
        field = "genome";
    } else if (m_Origin->GetValue()) {
        field = "origin";
    }

    if (m_SourceType->IsShown()) {
        if (m_SourceType->GetSelection() == 1) {
            field.append(" descriptor");
        } else if (m_SourceType->GetSelection() == 2) {
            field.append(" feature");
        }
    }
    return field;
}



bool CSourceFieldNamePanel::SetFieldName(const string& field)
{
    bool rval = false;
    SIZE_TYPE descr_pos = NStr::FindNoCase(field, " descriptor");
    SIZE_TYPE feat_pos = NStr::FindNoCase(field, " feature");
    string field_name(field);

    if ( descr_pos != NPOS) {
        m_SourceType->SetSelection(1);
        field_name = field.substr(0, descr_pos);
    } else if (feat_pos != NPOS) {
        m_SourceType->SetSelection(2);
        field_name = field.substr(0, feat_pos);
    }

    if (NStr::EqualNocase(field_name, "genome")) {
        m_Location->SetValue(true);
        x_ChooseLocation();
        rval = true;
    } else if (NStr::EqualNocase(field_name, "origin")) {
        m_Origin->SetValue(true);
        m_SourceSubList->Show(false);
        rval = true;
    } else {
        bool is_textqual = false;
        // see if it's a SubSource subtype:
        bool issubsrc = CSubSource::IsValidSubtypeName(field_name, CSubSource::eVocabulary_insdc);
        if (issubsrc) {
            CSubSource::TSubtype st = CSubSource::GetSubtypeValue(field_name, CSubSource::eVocabulary_insdc);
            // this line is here to defeat the complaining of the compiler
            st = CSubSource::eSubtype_chromosome;
            is_textqual = true;
        } else {
            // is it an OrgMod subtype?
            bool isorgmod = COrgMod::IsValidSubtypeName(field_name, COrgMod::eVocabulary_insdc);
            if (isorgmod) {
                COrgMod::TSubtype st = COrgMod::GetSubtypeValue(field_name, COrgMod::eVocabulary_insdc);
                // this line is here to defeat the complaining of the compiler
                st = COrgMod::eSubtype_strain;
                is_textqual = true;
            }
        }

        // special cases
        if (edit::CFieldHandler::QualifierNamesAreEquivalent(field_name, kSubSourceNote)
            || edit::CFieldHandler::QualifierNamesAreEquivalent(field_name, kOrgModNote)
            || edit::CFieldHandler::QualifierNamesAreEquivalent(field_name, kHost)
            || edit::CFieldHandler::QualifierNamesAreEquivalent(field_name, kDbXref)
            ) {
            is_textqual = true;
        }

        if (NStr::EndsWith(field_name, kInst_suffix)
            || NStr::EndsWith(field_name, kColl_suffix)
            || NStr::EndsWith(field_name, kSpecid_suffix)) {
            is_textqual = true;
        }

        // primers
        if (NStr::EqualNocase(field_name, kFwdPrimerName) || NStr::EqualNocase(field_name, kFwdPrimerSeq)
            || NStr::EqualNocase(field_name, kRevPrimerName) || NStr::EqualNocase(field_name, kRevPrimerSeq)) {
            is_textqual = true;
        }
            
        if (is_textqual) {
            m_TextQualifier->SetValue(true);
            x_ChooseTextQualifier();
            if (edit::CFieldHandler::QualifierNamesAreEquivalent(field_name, kNatHost)) {
                rval = m_SourceSubList->SetStringSelection(kHost);
		m_SourceSubList->SetFirstItem(kHost);
            } else {
                rval = m_SourceSubList->SetStringSelection(field_name);
		m_SourceSubList->SetFirstItem(field_name);
            }
        } else {
            if (m_first_run)
            {
                m_is_taxonomy_panel = true;
            }
            m_Taxonomy->SetValue(true);
            x_ChooseTaxonomy();
            rval = m_SourceSubList->SetStringSelection(field_name);
	    m_SourceSubList->SetFirstItem(field_name);
        }
    }
    m_first_run = false;
    x_UpdateParent();
    return rval;
}

void CSourceFieldNamePanel::SetSourceSubpanelSelection(const string &sel)
{
    if (!m_SourceSubList)
        return;
    m_SourceSubList->SetStringSelection(sel);
    m_SourceSubList->SetFirstItem(sel);
}


vector<string> CSourceFieldNamePanel::s_GetSourceOriginOptions()
{
    return {
        "unknown",
        "natural",
        "natmut",
        "mut",
        "artificial",
        "synthetic",
        "other"
    };
}

vector<string> CSourceFieldNamePanel::s_GetSourceLocationOptions()
{
    vector<string> choices;
    choices.push_back("mitochondrion");
    choices.push_back("chloroplast");
    CEnumeratedTypeValues::TValues loc_values = CBioSource::ENUM_METHOD_NAME(EGenome)()->GetValues();
    for (auto& it : loc_values) {
        if (it.first != "mitochondrion" && it.first != "chloroplast" && it.first != "unknown") {
            choices.push_back(it.first);
        }
    }
    return choices;
}

vector<string> s_GetTrueFalseList()
{
    vector<string> tf_strings;
    tf_strings.push_back("true");
    tf_strings.push_back("");
    return tf_strings;
}

vector<string> CSourceFieldNamePanel::GetChoices(bool &allow_other)
{
    vector<string> choices;

    if (m_Location->GetValue()) {
        choices = s_GetSourceLocationOptions();
        allow_other = false;
    } else if (m_Origin->GetValue()) {
        choices = s_GetSourceOriginOptions();
        allow_other = false;
    } else if (m_TextQualifier->GetValue()) {
        bool is_true_false = false;
        int val = m_SourceSubList->GetSelection();
        if (val > -1) {
            string field = ToStdString(m_SourceSubList->GetString(val));
            if (!NStr::IsBlank(field)) {
                // see if it's a SubSource subtype:
                bool issubsrc = CSubSource::IsValidSubtypeName(field, CSubSource::eVocabulary_insdc);
                if (issubsrc) {
                    CSubSource::TSubtype st = CSubSource::GetSubtypeValue (field, CSubSource::eVocabulary_insdc);
                    is_true_false = CSubSource::NeedsNoText(st);
                }
                // no orgmod values are true/false
            }
            if (is_true_false) {
                choices = s_GetTrueFalseList();
                allow_other = false;
            }
        }
    }

    return choices;
}

void CSourceFieldNamePanel::x_ChooseTextQualifier()
{
    m_SourceSubList->Show(true);
    m_SourceSubList->Clear();

    wxArrayString choices;
    // subsource modifiers
    for (size_t i = 0; i < CSubSource::eSubtype_other; i++) {
        try {
            string qual_name = CSubSource::GetSubtypeName (i);
            if (!NStr::IsBlank(qual_name) && !CSubSource::IsDiscouraged(i)) {
                choices.push_back(ToWxString(qual_name));
            }
            
        } catch (const exception&) {
        }
    }
    choices.push_back(kSubSourceNote);

    // orgmod modifiers
    for (size_t i = 0; i < COrgMod::eSubtype_other; i++) {
        try {
            string qual_name = COrgMod::GetSubtypeName (i);
            if (!NStr::IsBlank(qual_name)) {
                // special case for nat-host
                if (NStr::EqualNocase(qual_name, kNatHost)) {
                    choices.push_back(kHost);
                } else {
                    choices.push_back(ToWxString(qual_name));
                    if ( i == COrgMod::eSubtype_bio_material ||
                         i == COrgMod::eSubtype_culture_collection ||
                         i == COrgMod::eSubtype_specimen_voucher) {
                        choices.push_back(ToWxString(qual_name + kColl_suffix));
                        choices.push_back(ToWxString(qual_name + kInst_suffix));
                        choices.push_back(ToWxString(qual_name + kSpecid_suffix));
                    }
                }
            }
            
        } catch (const exception&) {
        }
    }
    choices.push_back(kOrgModNote);

    // primers
    choices.push_back(kFwdPrimerName);
    choices.push_back(kFwdPrimerSeq);
    choices.push_back(kRevPrimerName);
    choices.push_back(kRevPrimerSeq);

    choices.push_back(kDbXref);

    choices.Sort();
    choices.insert(choices.begin(), wxT("taxname"));

    if (m_AdditionalItems) {
        m_SourceSubList->Insert(ToWxString(kAllNotes), 0);
        m_SourceSubList->Insert(ToWxString(kAllPrimers), 1);
    }

    ITERATE (wxArrayString, it, choices) {
        m_SourceSubList->Append (*it);
    }
}

// BioSource
string CSourceFieldNamePanel::GetMacroFieldName(const string &target, const string& selected_field)
{
    string str;
    if (m_TextQualifier->GetValue() || m_Taxonomy->GetValue()) 
    {
        int val = m_SourceSubList->GetSelection();
        if (val > -1) 
        {
            string field = ToStdString(m_SourceSubList->GetString(val));
            if (  CSubSource::IsValidSubtypeName(field, CSubSource::eVocabulary_insdc) )
                str = "\"subtype\", \"" + field + "\"";
            if (COrgMod::IsValidSubtypeName(field, COrgMod::eVocabulary_insdc))
                str = "\"org.orgname.mod\", \"" + field + "\"";
            if (edit::CFieldHandler::QualifierNamesAreEquivalent(field, kHost) ||
                edit::CFieldHandler::QualifierNamesAreEquivalent(field, kNatHost))
                str = "\"org.orgname.mod\", \"nat-host\"";
            if (edit::CFieldHandler::QualifierNamesAreEquivalent(field, kDbXref))
                str = "\"org.db.db\"";
            if (field == "taxname")
                str = "\"org.taxname\"";
            if (field == "lineage")
                str = "\"org.orgname.lineage\"";
            if (field == "division")
                str = "\"org.orgname.div\"";
            if (edit::CFieldHandler::QualifierNamesAreEquivalent(field, kOrgModNote))
                str = "\"org.orgname.mod\", \"other\"";
            if (edit::CFieldHandler::QualifierNamesAreEquivalent(field, kSubSourceNote))
                str = "\"subtype\", \"other\"";
            if ( NStr::EqualNocase(field, kFwdPrimerSeq))
                str = "\"pcr-primers..forward..seq\"";
            if (NStr::EqualNocase(field, kRevPrimerSeq))
                str = "\"pcr-primers..reverse..seq\"";
            if ( NStr::EqualNocase(field, kFwdPrimerName))
                str = "\"pcr-primers..forward..name\"";
            if (NStr::EqualNocase(field, kRevPrimerName))
                str = "\"pcr-primers..reverse..name\"";
            if (field == "common name")
                str = "\"org.common\"";
            if (macro::NMacroUtil::IsStructVoucherPart(field)) {
                auto pos = field.find_last_of('-');
                str = macro::CMacroFunction_StructVoucherPart::GetFuncName();
                str += "(\"" + field.substr(0, pos) + "\", \"" + field.substr(pos + 1) + "\")";
            }
        }  
    } 

    if  ( m_Location->GetValue() ) 
    {
       str = "\"genome\"";
    } 

    if (m_Origin->GetValue()) 
    {
        str = "\"origin\"";
    }
    
    if (!str.empty())
    {
        if (target == macro::CMacroBioData::sm_BioSource) {

        } 
        else if (target == macro::CMacroBioData::sm_MolInfo) {
            str = macro::CMacroFunction_GetSeqdesc::sm_BsrcForMolinfo + string("(") + str + ")";
        }
        else if (target == macro::CMacroBioData::sm_StrComm) {
            str = macro::CMacroFunction_GetSeqdesc::sm_BsrcForSeqdesc + string("(") + str + ")";
        } 
        else if (target == macro::CMacroBioData::sm_Seq || target == macro::CMacroBioData::sm_SeqNa) {
            str = macro::CMacroFunction_GetSeqdesc::sm_BsrcForSeq + string("(") + str + ")";
        }
        else if (CFieldNamePanel::IsFeature(target)) {
            str = macro::CMacroFunction_GetSeqdesc::sm_BsrcForFeat + string("(") + str + ")";
        }
        else {
            str.clear();
        }
    }

    return str;
}

void CSourceFieldNamePanel::x_ChooseTaxonomy()
{
    m_SourceSubList->Show(true);
    m_SourceSubList->Clear();

    m_SourceSubList->Append(_("taxname"));
    m_SourceSubList->Append(_("common name"));
    m_SourceSubList->Append(_("division"));
    m_SourceSubList->Append(_("lineage"));
    m_SourceSubList->SetStringSelection(_("taxname"));
    m_SourceSubList->SetFirstItem(_("taxname"));
}

void CSourceFieldNamePanel::x_ChooseLocation()
{
    m_SourceSubList->Clear();
    m_SourceSubList->Show(false);   
}


/*!
 * wxEVT_COMMAND_RADIOBUTTON_SELECTED event handler for ID_TEXT_QUAL_BTN
 */

void CSourceFieldNamePanel::OnTextQualBtnSelected( wxCommandEvent& event )
{
    x_ChooseTextQualifier();
    x_UpdateParent();
    event.Skip();
}


/*!
 * wxEVT_COMMAND_RADIOBUTTON_SELECTED event handler for ID_TAXONOMY_BTN
 */

void CSourceFieldNamePanel::OnTaxonomyBtnSelected( wxCommandEvent& event )
{
    x_ChooseTaxonomy();
    x_UpdateParent();
    event.Skip();
}


/*!
 * wxEVT_COMMAND_RADIOBUTTON_SELECTED event handler for ID_LOCATION_BTN
 */

void CSourceFieldNamePanel::OnLocationBtnSelected( wxCommandEvent& event )
{
    x_ChooseLocation();
    x_UpdateParent();
    event.Skip();
}


/*!
 * wxEVT_COMMAND_RADIOBUTTON_SELECTED event handler for ID_ORIGIN_BTN
 */

void CSourceFieldNamePanel::OnOriginBtnSelected( wxCommandEvent& event )
{
    m_SourceSubList->Show(false);
    x_UpdateParent();
    event.Skip();
}


/*!
 * wxEVT_COMMAND_LISTBOX_SELECTED event handler for ID_SRC_SUBLIST
 */

void CSourceFieldNamePanel::OnSrcSublistSelected( wxCommandEvent& event )
{
    x_UpdateParent();
    event.Skip();
}


END_NCBI_SCOPE
