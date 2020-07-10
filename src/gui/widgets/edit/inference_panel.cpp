/*  $Id: inference_panel.cpp 41357 2018-07-13 15:40:13Z filippov $
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


// Generated by DialogBlocks (unregistered), 08/12/2011 09:51:07

#include <ncbi_pch.hpp>
#include <objects/seqfeat/Gb_qual.hpp>
#include <gui/widgets/wx/wx_utils.hpp>
////@begin includes
////@end includes

#include <wx/statbox.h>
#include <wx/combobox.h>


#include <gui/widgets/edit/inference_panel.hpp>   
#include <gui/widgets/edit/gbqual_panel.hpp>

////@begin XPM images
////@end XPM images

BEGIN_NCBI_SCOPE

/*!
 * CInferencePanel type definition
 */

IMPLEMENT_DYNAMIC_CLASS( CInferencePanel, CFormattedQualPanel )


/*!
 * CInferencePanel event table definition
 */

    BEGIN_EVENT_TABLE( CInferencePanel, CFormattedQualPanel )

////@begin CInferencePanel event table entries
    EVT_CHOICE( ID_CHOICE13, CInferencePanel::OnChoice13Selected )

////@end CInferencePanel event table entries

    END_EVENT_TABLE()


/*!
 * CInferencePanel constructors
 */

    CInferencePanel::CInferencePanel()
{
    Init();
}

CInferencePanel::CInferencePanel( wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style )
{
    Init();
    Create(parent, id, pos, size, style);
}


/*!
 * CInferencePanel creator
 */

bool CInferencePanel::Create( wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style )
{
////@begin CInferencePanel creation
    SetExtraStyle(wxWS_EX_BLOCK_EVENTS);
    CFormattedQualPanel::Create( parent, id, pos, size, style );

    CreateControls();
    if (GetSizer())
    {
        GetSizer()->SetSizeHints(this);
    }
    Centre();
////@end CInferencePanel creation
    return true;
}


/*!
 * CInferencePanel destructor
 */

CInferencePanel::~CInferencePanel()
{
////@begin CInferencePanel destruction
////@end CInferencePanel destruction
}


/*!
 * Member initialisation
 */

void CInferencePanel::Init()
{
////@begin CInferencePanel member initialisation
    m_CategoryCtrl = NULL;
    m_TypeCtrl = NULL;
    m_ExtraCtrlSizer = NULL;
////@end CInferencePanel member initialisation
}


/*!
 * Control creation for CInferencePanel
 */

void CInferencePanel::CreateControls()
{    
////@begin CInferencePanel content construction
    // Generated by DialogBlocks, 05/01/2012 08:48:22 (unregistered)

    CInferencePanel* itemCFormattedQualPanel1 = this;

    wxBoxSizer* itemBoxSizer2 = new wxBoxSizer(wxHORIZONTAL);
    itemCFormattedQualPanel1->SetSizer(itemBoxSizer2);

    wxArrayString m_CategoryCtrlStrings;
    m_CategoryCtrlStrings.Add(wxEmptyString);
    m_CategoryCtrlStrings.Add(_("COORDINATES"));
    m_CategoryCtrlStrings.Add(_("DESCRIPTION"));
    m_CategoryCtrlStrings.Add(_("EXISTENCE"));
    m_CategoryCtrl = new wxChoice( itemCFormattedQualPanel1, ID_CHOICE12, wxDefaultPosition, wxDefaultSize, m_CategoryCtrlStrings, 0 );
    itemBoxSizer2->Add(m_CategoryCtrl, 0, wxALIGN_CENTER_VERTICAL|wxALL, 0);

    wxArrayString m_TypeCtrlStrings;
    m_TypeCtrlStrings.Add(wxEmptyString);
    m_TypeCtrlStrings.Add(_("similar to sequence"));
    m_TypeCtrlStrings.Add(_("similar to protein"));
    m_TypeCtrlStrings.Add(_("similar to DNA"));
    m_TypeCtrlStrings.Add(_("similar to RNA"));
    m_TypeCtrlStrings.Add(_("similar to mRNA"));
    m_TypeCtrlStrings.Add(_("similar to EST"));
    m_TypeCtrlStrings.Add(_("similar to other RNA"));
    m_TypeCtrlStrings.Add(_("profile"));
    m_TypeCtrlStrings.Add(_("nucleotide motif"));
    m_TypeCtrlStrings.Add(_("protein motif"));
    m_TypeCtrlStrings.Add(_("ab initio prediction"));
    m_TypeCtrlStrings.Add(_("alignment"));
    m_TypeCtrl = new wxChoice( itemCFormattedQualPanel1, ID_CHOICE13, wxDefaultPosition, wxDefaultSize, m_TypeCtrlStrings, 0 );
    itemBoxSizer2->Add(m_TypeCtrl, 0, wxALIGN_CENTER_VERTICAL|wxALL, 0);

    m_ExtraCtrlSizer = new wxBoxSizer(wxHORIZONTAL);
    itemBoxSizer2->Add(m_ExtraCtrlSizer, 0, wxALIGN_CENTER_VERTICAL|wxALL, 0);

////@end CInferencePanel content construction

    m_TopWidth = 0;
    m_TotalHeight = 0;
    int x = 0, y = 0;
    wxSizerItemList& itemList = itemBoxSizer2->GetChildren();
    for(wxSizerItemList::iterator it = itemList.begin(); it != itemList.end(); ++it) {
        if ((*it)->GetWindow()) {
	    (*it)->GetWindow()->GetSize(&x, &y);
	    // 10 is for border
	    m_TopWidth += x + 10;

            if ( y + 10 > m_TotalHeight) {
                m_TotalHeight = y + 10;
            }
        }
    }
    m_TotalWidth = m_TopWidth;

}

typedef SStaticPair<const char*, const char*>  TInferenceTypeSynonymPairElem;
static const TInferenceTypeSynonymPairElem k_inference_type_synonym_pair_map[] = {
    { "similar to AA sequence", "similar to protein" },
    { "similar to DNA sequence", "similar to DNA" },
    { "similar to RNA sequence", "similar to RNA" },
    { "similar to RNA sequence, EST", "similar to EST" },
    { "similar to RNA sequence, mRNA", "similar to mRNA" },
    { "similar to RNA sequence, other", "similar to other RNA" },
};


typedef CStaticArrayMap<const char*, const char*, PNocase_CStr> TInferenceTypeSynonymPairMap;
DEFINE_STATIC_ARRAY_MAP(TInferenceTypeSynonymPairMap, sc_InferenceTypeSynonymPairMap, k_inference_type_synonym_pair_map);


void CInferencePanel::SetValue(string val)
{
    int win_height, win_width;
    GetSize(&win_width, &win_height);
    string category, type_str, database, accession, program, version, acc_list;
    bool is_same_species;
  
    objects::CGb_qual::ParseInferenceString(val, category, type_str, is_same_species, database, 
					    accession, program, version, acc_list);
    bool found = false;

    m_CategoryCtrl->SetStringSelection(wxEmptyString);
    wxArrayString categories = m_CategoryCtrl->GetStrings();
    // start with 1 - first item is blank
    if (!category.empty())
        for (unsigned int i = 1; i < categories.size(); i++) {
            if (NStr::EqualNocase(category, ToStdString (categories[i]))) {
                m_CategoryCtrl->SetStringSelection(categories[i]);
                found = true;
                break;
            }
        }
    string display_type_str = type_str;
    auto display_type_str_it = sc_InferenceTypeSynonymPairMap.find(type_str.c_str());
    if ( display_type_str_it != sc_InferenceTypeSynonymPairMap.end())
	display_type_str = display_type_str_it->second;
    m_TypeCtrl->SetStringSelection(wxEmptyString);
    wxArrayString types = m_TypeCtrl->GetStrings();
    // start with 1 - first item is blank
    if (!display_type_str.empty())
        for (unsigned int i = 1; i < types.size(); i++) {
            if (NStr::EqualNocase(display_type_str, ToStdString (types[i]))) {
                m_TypeCtrl->SetStringSelection(types[i]);
                found = true;
                break;
            }
        }

    // add type-dependent extra data

    m_ExtraCtrlSizer->Clear(true);

    win_width = m_TopWidth;
    if (NStr::StartsWith(type_str, "similar to ")) {
        wxCheckBox* same_species = new wxCheckBox( this, wxID_ANY, _("(same species)"), wxDefaultPosition, wxDefaultSize, 0 );
        same_species->SetValue(is_same_species);
        m_ExtraCtrlSizer->Add(same_species, 0, wxALIGN_CENTER_VERTICAL|wxALL, 0);

        wxArrayString choices;
        choices.push_back(_("GenBank"));
        choices.push_back(_("EMBL"));
        choices.push_back(_("DDBJ"));
        choices.push_back(_("INSD"));
        choices.push_back(_("RefSeq"));
        choices.push_back(_("UniProt"));
        choices.push_back(_("PDB"));
        choices.push_back(_("UniProtKB"));
        choices.push_back(_("Other"));
        wxChoice* database_ctrl = new wxChoice( this, wxID_ANY, wxDefaultPosition, wxSize(70, -1), choices, 0 );
        m_ExtraCtrlSizer->Add(database_ctrl, 0, wxALIGN_CENTER_VERTICAL|wxALL, 0);
        database_ctrl->SetToolTip(_("Database"));

        wxStaticText* colon = new wxStaticText( this, wxID_ANY, _(":"), wxDefaultPosition );
        m_ExtraCtrlSizer->Add(colon, 0, wxALIGN_CENTER_VERTICAL|wxALL, 0);

        wxTextCtrl* accession_ctrl = new wxTextCtrl( this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxSize(90, -1), 0 );
        m_ExtraCtrlSizer->Add(accession_ctrl, 0, wxALIGN_CENTER_VERTICAL|wxALL, 0);
        accession_ctrl->SetToolTip(_("Accession (must include version)"));

        database_ctrl->SetStringSelection(ToWxString(database));
        accession_ctrl->SetValue(ToWxString(accession));
        win_width += 240;
    } else if (NStr::EqualNocase(type_str, "profile") 
	       || NStr::EqualNocase(type_str, "nucleotide motif")
	       || NStr::EqualNocase(type_str, "protein motif")) {
        wxTextCtrl* program_ctrl = new wxTextCtrl( this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxSize(60, -1), 0 );
        m_ExtraCtrlSizer->Add(program_ctrl, 0, wxALIGN_CENTER_VERTICAL|wxALL, 0);
        program_ctrl->SetToolTip(_("Program or Database"));
        
        wxStaticText* colon = new wxStaticText( this, wxID_ANY, _(":"), wxDefaultPosition );
        m_ExtraCtrlSizer->Add(colon, 0, wxALIGN_CENTER_VERTICAL|wxALL, 0);

        wxTextCtrl* version_ctrl = new wxTextCtrl( this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxSize(60, -1), 0 );
        m_ExtraCtrlSizer->Add(version_ctrl, 0, wxALIGN_CENTER_VERTICAL|wxALL, 0);
        version_ctrl->SetToolTip(_("Version or Accession"));

        program_ctrl->SetValue(ToWxString (program));
        version_ctrl->SetValue (ToWxString (version));
        win_width += 110;
    } else if (NStr::EqualNocase(type_str, "ab initio prediction")) {
        wxArrayString ProgramCtrlStrings;
        ProgramCtrlStrings.Add(wxEmptyString);
        ProgramCtrlStrings.Add(_("tRNAScan"));
        ProgramCtrlStrings.Add(_("Genscan"));
        ProgramCtrlStrings.Add(_("SignalP"));
        wxComboBox* program_ctrl = new wxComboBox( this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxSize(60, -1), ProgramCtrlStrings, wxCB_DROPDOWN );
        m_ExtraCtrlSizer->Add(program_ctrl, 0, wxALIGN_CENTER_VERTICAL|wxALL, 0);
        program_ctrl->SetToolTip(_("Program"));

        wxStaticText* colon = new wxStaticText( this, wxID_ANY, _(":"), wxDefaultPosition );
        m_ExtraCtrlSizer->Add(colon, 0, wxALIGN_CENTER_VERTICAL|wxALL, 0);

        wxTextCtrl* version_ctrl = new wxTextCtrl( this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxSize(60, -1), 0 );
        m_ExtraCtrlSizer->Add(version_ctrl, 0, wxALIGN_CENTER_VERTICAL|wxALL, 0);
        version_ctrl->SetToolTip(_("Version"));

        program_ctrl->SetValue(ToWxString (program));
        version_ctrl->SetValue (ToWxString (version));
        win_width += 110;
    } else if (NStr::EqualNocase(type_str, "alignment")) {
        wxTextCtrl* program_ctrl = new wxTextCtrl( this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxSize(60, -1), 0 );
        m_ExtraCtrlSizer->Add(program_ctrl, 0, wxALIGN_CENTER_VERTICAL|wxALL, 0);
        program_ctrl->SetToolTip(_("Program"));
        
        wxStaticText* colon = new wxStaticText( this, wxID_ANY, _(":"), wxDefaultPosition );
        m_ExtraCtrlSizer->Add(colon, 0, wxALIGN_CENTER_VERTICAL|wxALL, 0);

        wxTextCtrl* version_ctrl = new wxTextCtrl( this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxSize(60, -1), 0 );
        m_ExtraCtrlSizer->Add(version_ctrl, 0, wxALIGN_CENTER_VERTICAL|wxALL, 0);
        version_ctrl->SetToolTip(_("Version"));

        colon = new wxStaticText( this, wxID_ANY, _(":"), wxDefaultPosition );
        m_ExtraCtrlSizer->Add(colon, 0, wxALIGN_CENTER_VERTICAL|wxALL, 0);

        wxTextCtrl* acc_list_ctrl = new wxTextCtrl( this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxSize(120, 50), wxTE_MULTILINE );
        m_ExtraCtrlSizer->Add(acc_list_ctrl, 0, wxALIGN_CENTER_VERTICAL|wxALL, 0);
        acc_list_ctrl->SetToolTip(_("Accession List"));
        m_TotalHeight = 60;

        program_ctrl->SetValue(ToWxString (program));
        version_ctrl->SetValue (ToWxString (version));
        acc_list_ctrl->SetValue(ToWxString(acc_list));

        win_width += 240;
    }

    if (!found && !val.empty())
    {
        wxTextCtrl* illegal_value = new wxTextCtrl( this, wxID_ANY, wxString(val), wxDefaultPosition, wxSize(100, -1), 0 );
        m_ExtraCtrlSizer->Add(illegal_value, 0, wxALIGN_CENTER_VERTICAL|wxALL, 0);
        win_width += 100;
    }

    wxWindow* parent = this->GetParent();
    if (parent) {
        parent->Layout();
    }
    Layout();
    m_TotalWidth = win_width;
    //SetSize(m_TotalWidth, m_TotalHeight);
    SetMinSize(wxSize(m_TotalWidth, m_TotalHeight));
}

// http://www.insdc.org/files/feature_table.html
// "[CATEGORY:]TYPE[ (same species)][:EVIDENCE_BASIS]"
// "EVIDENCE_BASIS"   -   "[ALGORITHM][:EVIDENCE_DBREF[,EVIDENCE_DBREF]*[,...]]"
string CInferencePanel::GetValue()
{
    string val = "";
    string category = ToStdString(m_CategoryCtrl->GetStringSelection());
    if (!NStr::IsBlank(category)) {
	val += category + ": ";
    }

    string i_type = ToStdString(m_TypeCtrl->GetStringSelection());
    for (auto synonyms : sc_InferenceTypeSynonymPairMap)
    {
	if (NStr::EqualNocase(synonyms.second, i_type))
	    i_type = synonyms.first;
    }
    if (!NStr::IsBlank(i_type)) {
	val += i_type;

    }

    wxSizerItemList& itemList = m_ExtraCtrlSizer->GetChildren();
    if (itemList.size() == 0) {
        return val;
    }
    size_t pos = 0;
    if (NStr::StartsWith(i_type, "similar to ")) {
        wxCheckBox* same_species = dynamic_cast<wxCheckBox*>(itemList[0]->GetWindow());
        if (same_species) {
            pos++;
            if (same_species->GetValue()) {
		val += " (same species)";
            }
        }

    }
   
    string evidence_basis;
    bool preceeding_colon = false;
    while (pos < itemList.size()) {
        string add_text = "";
        wxTextCtrl* text = dynamic_cast<wxTextCtrl*>(itemList[pos]->GetWindow());
        if (text) {
            add_text = ToStdString(text->GetValue());
        } else {
            wxComboBox* combo = dynamic_cast<wxComboBox*>(itemList[pos]->GetWindow());
            if (combo) {
                add_text = ToStdString (combo->GetValue());
            } else {
                wxChoice* chc = dynamic_cast<wxChoice*>(itemList[pos]->GetWindow());
                if (chc) {
                    add_text = ToStdString(chc->GetStringSelection());
                } else {
                    wxStaticText* ctrl = dynamic_cast<wxStaticText*>(itemList[pos]->GetWindow());
                    if (ctrl) {
                        add_text = ToStdString(ctrl->GetLabel());
                    }
                }
            }
        }
	if (add_text == ":")
	{
	    preceeding_colon = true;
	    pos++;
	    continue;
	}
        if (NStr::Equal(i_type, "alignment") && pos == itemList.size() - 1) {
            NStr::ReplaceInPlace(add_text, "\n", ",");
            vector<string> tokens;
            NStr::Split(add_text, ",", tokens);
            add_text = "";
            ITERATE (vector<string>, it, tokens) {
                string acc = *it;
                if (!NStr::IsBlank(acc)) {
                    NStr::ReplaceInPlace(acc, ":", "|");
                    size_t pos = NStr::Find(acc, "|");
                    if (pos == string::npos) {
                        acc = "INSD|" + acc;
                    }
                    pos = NStr::Find(acc, ".");
                    if (pos == string::npos) {
                        acc += ".1";
                    }
                    if (!NStr::IsBlank(add_text)) {
                        add_text += ",";
                    }
                    add_text += acc;
                }
            }
        }        
        if (!add_text.empty())
	{
	    if (preceeding_colon)
		evidence_basis += ":";
            evidence_basis += add_text;
	}
	preceeding_colon = false;
        pos++;
    }

    if (!evidence_basis.empty())
	val += ":" + evidence_basis;

    string tmp = val;
    NStr::ReplaceInPlace(tmp, ":", "");
    NStr::ReplaceInPlace(tmp, "|", "");
    if (NStr::IsBlank(tmp)) {
        val = "";
    }

    return val;
}


void CInferencePanel::OnChildChange (wxWindow* wnd)
{
    NotifyParentOfChange();
}


void CInferencePanel::GetDimensions(int *width, int *height)
{
    if (height != NULL) {
        *height = m_TotalHeight;
    }
    if (width != NULL) {
        *width = m_TotalWidth;
    }
}


/*!
 * Should we show tooltips?
 */

bool CInferencePanel::ShowToolTips()
{
    return true;
}

/*!
 * Get bitmap resources
 */

wxBitmap CInferencePanel::GetBitmapResource( const wxString& name )
{
    // Bitmap retrieval
////@begin CInferencePanel bitmap retrieval
    wxUnusedVar(name);
    return wxNullBitmap;
////@end CInferencePanel bitmap retrieval
}

/*!
 * Get icon resources
 */

wxIcon CInferencePanel::GetIconResource( const wxString& name )
{
    // Icon retrieval
////@begin CInferencePanel icon retrieval
    wxUnusedVar(name);
    return wxNullIcon;
////@end CInferencePanel icon retrieval
}


/*!
 * wxEVT_COMMAND_CHOICE_SELECTED event handler for ID_CHOICE13
 */

void CInferencePanel::OnChoice13Selected( wxCommandEvent& event )
{
    string val = GetValue();
    SetValue (val);
    NotifyParentOfChange();
}


END_NCBI_SCOPE
