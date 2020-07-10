/*  $Id: rna_field_name_panel.cpp 44728 2020-02-28 21:45:18Z asztalos $
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
#include <objects/seqfeat/RNA_gen.hpp>
#include <gui/widgets/wx/wx_utils.hpp>
#include <gui/objutils/macro_fn_aecr.hpp>
#include <gui/widgets/edit/macro_editor_context.hpp>
#include <gui/widgets/edit/rna_field_name_panel.hpp>
#include <wx/sizer.h>


BEGIN_NCBI_SCOPE

/*!
 * CRNAFieldNamePanel type definition
 */

IMPLEMENT_DYNAMIC_CLASS( CRNAFieldNamePanel, CFieldNamePanel )


/*!
 * CRNAFieldNamePanel event table definition
 */

BEGIN_EVENT_TABLE( CRNAFieldNamePanel, CFieldNamePanel )

////@begin CRNAFieldNamePanel event table entries
    EVT_CHOICE( ID_CRNAFIELDNAME_RNA_TYPE, CRNAFieldNamePanel::OnRnaTypeSelected)
    EVT_LISTBOX( ID_CRNAFIELDNAME_RNA_FIELD, CRNAFieldNamePanel::OnRnaFieldSelected)

////@end CRNAFieldNamePanel event table entries

END_EVENT_TABLE()


/*!
 * CRNAFieldNamePanel constructors
 */

CRNAFieldNamePanel::CRNAFieldNamePanel() : m_Sibling(NULL)
{
    Init();
}

CRNAFieldNamePanel::CRNAFieldNamePanel( wxWindow* parent, CRNAFieldNamePanel* sibling, wxWindowID id, const wxPoint& pos, const wxSize& size, long style )
  : m_Sibling(sibling)
{
    Init();
    Create(parent, id, pos, size, style);
}


/*!
 * CRNAFieldNamePanel creator
 */

bool CRNAFieldNamePanel::Create( wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style )
{
////@begin CRNAFieldNamePanel creation
    CFieldNamePanel::Create( parent, id, pos, size, style );

    CreateControls();
    if (GetSizer())
    {
        GetSizer()->SetSizeHints(this);
    }
    Centre();
////@end CRNAFieldNamePanel creation
    return true;
}


/*!
 * CRNAFieldNamePanel destructor
 */

CRNAFieldNamePanel::~CRNAFieldNamePanel()
{
////@begin CRNAFieldNamePanel destruction
////@end CRNAFieldNamePanel destruction
}


/*!
 * Member initialisation
 */

void CRNAFieldNamePanel::Init()
{
////@begin CRNAFieldNamePanel member initialisation
    m_RnaTypeLabel = NULL;
    m_RNAType = NULL;
    m_NcrnaClass = NULL;
    m_RnaField = NULL;
////@end CRNAFieldNamePanel member initialisation
}


/*!
 * Control creation for CRNAFieldNamePanel
 */

void CRNAFieldNamePanel::CreateControls()
{    
////@begin CRNAFieldNamePanel content construction
    CRNAFieldNamePanel* itemCFieldNamePanel1 = this;

    wxBoxSizer* itemBoxSizer1 = new wxBoxSizer(wxVERTICAL);
    itemCFieldNamePanel1->SetSizer(itemBoxSizer1);

    wxBoxSizer* itemBoxSizer2 = new wxBoxSizer(wxHORIZONTAL);
    itemBoxSizer1->Add(itemBoxSizer2, 0, wxALIGN_CENTER_HORIZONTAL|wxTOP|wxBOTTOM,0);

    m_RnaTypeLabel = new wxStaticText( itemCFieldNamePanel1, wxID_STATIC, _("RNA Type"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer2->Add(m_RnaTypeLabel, 0, wxALIGN_CENTER_VERTICAL|wxRIGHT, 5);

    wxArrayString m_RNATypeStrings;
    m_RNAType = new wxChoice(itemCFieldNamePanel1, ID_CRNAFIELDNAME_RNA_TYPE, wxDefaultPosition, wxDefaultSize, m_RNATypeStrings, 0);
    itemBoxSizer2->Add(m_RNAType, 0, wxALIGN_CENTER_VERTICAL|wxRESERVE_SPACE_EVEN_IF_HIDDEN| wxLEFT|wxRIGHT|wxBOTTOM, 5);

    wxArrayString m_NcrnaClassStrings;
    m_NcrnaClassStrings.Add(_("any"));
    m_NcrnaClass = new wxComboBox(itemCFieldNamePanel1, ID_CRNAFIELDNAME_NCRNA_CLASS, _("any"), wxDefaultPosition, wxDefaultSize, m_NcrnaClassStrings, wxCB_DROPDOWN);
    m_NcrnaClass->SetStringSelection(_("any"));
    m_NcrnaClass->Enable(false);
    itemBoxSizer2->Add(m_NcrnaClass, 0, wxALIGN_CENTER_VERTICAL|wxLEFT|wxBOTTOM, 5);

    wxArrayString m_RnaFieldStrings;
    m_RnaField = new wxListBox(itemCFieldNamePanel1, ID_CRNAFIELDNAME_RNA_FIELD, wxDefaultPosition, wxSize(220, 109), m_RnaFieldStrings, wxLB_SINGLE);
    itemBoxSizer1->Add(m_RnaField, 0, wxALIGN_CENTER_HORIZONTAL|wxLEFT, 5);

    vector<string> rna_types = GetRNATypes();
    ITERATE(vector<string>, it, rna_types) {
        m_RNAType->AppendString(ToWxString(*it));
    }
    m_RNAType->SetStringSelection(rna_types[0]);

    vector<string> rna_fields = GetRNAFields();
    ITERATE(vector<string>, it, rna_fields) {
        m_RnaField->AppendString(ToWxString(*it));
    }

    vector<string> class_vals = objects::CRNA_gen::GetncRNAClassList();
    ITERATE(vector<string>, it, class_vals) {
        m_NcrnaClass->AppendString(ToWxString(*it));
    }

    if (m_Sibling) {
        m_RnaTypeLabel->Show(false);
        m_RNAType->Show(false);
        m_NcrnaClass->Show(false);
    }
    x_EnableNcRnaClass();
////@end CRNAFieldNamePanel content construction
}

void CRNAFieldNamePanel::ClearValues()
{
    m_RNAType->SetSelection(0);
    m_NcrnaClass->SetStringSelection(_("any"));
    m_NcrnaClass->Enable(false);
    m_RnaField->SetSelection(0);
    m_RnaField->SetFirstItem(0);
}


/*!
 * Should we show tooltips?
 */

bool CRNAFieldNamePanel::ShowToolTips()
{
    return true;
}

/*!
 * Get bitmap resources
 */

wxBitmap CRNAFieldNamePanel::GetBitmapResource( const wxString& name )
{
    // Bitmap retrieval
////@begin CRNAFieldNamePanel bitmap retrieval
    wxUnusedVar(name);
    return wxNullBitmap;
////@end CRNAFieldNamePanel bitmap retrieval
}

/*!
 * Get icon resources
 */

wxIcon CRNAFieldNamePanel::GetIconResource( const wxString& name )
{
    // Icon retrieval
////@begin CRNAFieldNamePanel icon retrieval
    wxUnusedVar(name);
    return wxNullIcon;
////@end CRNAFieldNamePanel icon retrieval
}


string CRNAFieldNamePanel::GetRnaType()
{
    if (m_Sibling) {
        return m_Sibling->GetRnaType();
    }
    string rna_type = "";
    int val = m_RNAType->GetSelection();
    if (val > -1) {
        rna_type = m_RNAType->GetString(val);
        if (NStr::EqualNocase(rna_type, "any")) {
            rna_type = "";
        }  else if (NStr::StartsWith(rna_type, "misc", NStr::eNocase)) {
            rna_type.insert(4, "_");
        } else if (NStr::StartsWith(rna_type, "pre", NStr::eNocase)) {
            rna_type.insert(3, "_");
        } else if (NStr::EqualNocase(rna_type, "ncRNA")) {
            string ncrna_class = ToStdString(m_NcrnaClass->GetValue());
            if (!NStr::EqualNocase (ncrna_class, "any")) {
                rna_type += " " + ncrna_class;
            }
        }
    }
    return rna_type;
}

string CRNAFieldNamePanel::GetRnaTypeOnly(void)
{
    if (m_Sibling) 
    {
        return m_Sibling->GetRnaTypeOnly();
    }
    int i = m_RNAType->GetSelection();
    if (i != wxNOT_FOUND)
    {
        return m_RNAType->GetString(i).ToStdString();
    }
    return kEmptyStr;
}

string CRNAFieldNamePanel::GetNcrnaType(void)
{
    return m_NcrnaClass->GetValue().ToStdString();
}

string CRNAFieldNamePanel::GetRnaField(void)
{
    return GetFieldName(true);
}

string CRNAFieldNamePanel::GetFieldName(const bool subfield)
{
    string field = "";
    int val = m_RnaField->GetSelection();
    if (val < 0) {
        return field;
    }
    field = m_RnaField->GetString(val);
    if (subfield) {
        return field;
    }
    string rna_type = GetRnaType();
    if (!NStr::IsBlank(rna_type)) {
        field = rna_type + " " + field;
    }
        
    return field;
}


bool CRNAFieldNamePanel::SetRnaType (const string& rna_type)
{
    if (m_Sibling) {
        return m_Sibling->SetRnaType(rna_type);
    }
    bool rval = false;
    if (NStr::IsBlank (rna_type)) {
        // choose any
        m_RNAType->SetSelection(0);
    } else {
        string remainder = rna_type;
        if (NStr::EqualNocase(rna_type, "misc_RNA")) {
            m_RNAType->SetStringSelection(ToWxString("miscRNA"));
        } else if (NStr::EqualNocase(rna_type, "pre_RNA")) {
            m_RNAType->SetStringSelection(ToWxString("preRNA"));
        } else {
            for (size_t i = 0; i < m_RNAType->GetStrings().size(); i++) {
                string major_type = ToStdString(m_RNAType->GetString(i));
                if (NStr::StartsWith (rna_type, major_type)) {
                    m_RNAType->SetSelection(i);
                    string remainder = rna_type.substr(major_type.length());
                    NStr::TruncateSpacesInPlace(remainder);
                    if (NStr::EqualNocase (major_type, "ncRNA")) {
                        m_NcrnaClass->SetValue(remainder);
                        remainder = "";
                    } 
                    break;
                }
            }
            if (!NStr::IsBlank(remainder)) {
                rval = false;
            }
        }
    }
    x_EnableNcRnaClass();
    return rval;
}


bool CRNAFieldNamePanel::SetFieldName(const string& field)
{
    bool rval = false;
    m_NcrnaClass->Enable(false);
    string rna_type = field;
    for (size_t i = 0; i < m_RnaField->GetCount(); i++) {
        string rna_field = ToStdString(m_RnaField->GetString(i));
        if (NStr::EndsWith (field, rna_field)) {
            m_RnaField->SetSelection(i);
            m_RnaField->SetFirstItem(i);
            if (rna_field.length() < field.length()) {
                rna_type = field.substr(0, field.length() - rna_field.length());
                NStr::TruncateSpacesInPlace(rna_type);
            }
            rval = true;
            break;
        }
    }

    if (rval) {
        rval = SetRnaType(rna_type);
    }
    x_UpdateParent();
    return rval;
}


void CRNAFieldNamePanel::x_EnableNcRnaClass()
{
    string rna_type = ToStdString(m_RNAType->GetStringSelection());
    if (NStr::EqualNocase(rna_type, "ncRNA")) {
        m_NcrnaClass->Enable(true);
    } else {
        m_NcrnaClass->Enable(false);
    }
}


/*!
 * wxEVT_COMMAND_CHOICE_SELECTED event handler for ID_RNA_TYPE
 */

void CRNAFieldNamePanel::OnRnaTypeSelected( wxCommandEvent& event )
{
    x_EnableNcRnaClass();
    x_UpdateParent();
    event.Skip();
}

void CRNAFieldNamePanel::OnRnaFieldSelected( wxCommandEvent& event )
{
    x_UpdateParent();
    event.Skip();
}

vector<string> CRNAFieldNamePanel::GetRNATypes()
{
    vector<string> options;
    options.push_back("any");
    options.push_back("preRNA");
    options.push_back("mRNA");
    options.push_back("tRNA");
    options.push_back("rRNA");
    options.push_back("ncRNA");
    options.push_back("tmRNA");
    options.push_back("miscRNA");
    return options;
}


vector<string> CRNAFieldNamePanel::GetRNAFields()
{
    vector<string> options;
    options.push_back("product");
    options.push_back("comment");
    options.push_back("ncRNA class");
    options.push_back("codons recognized");
    options.push_back("tag-peptide");
    options.push_back("anticodon");
    options.push_back("gene locus");
    options.push_back("gene description");
    options.push_back("gene maploc");
    options.push_back("gene locus tag");
    options.push_back("gene synonym");
    options.push_back("gene comment");
    return options;
}

string CRNAFieldNamePanel::GetMacroFieldName(const string &target, const string& selected_field)
{
    string qual_field;
    int val = m_RnaField->GetSelection();
    if (val >= 0) {
        qual_field = m_RnaField->GetString(val);
    }
    string rna_type = ToStdString(m_RNAType->GetStringSelection());
    string str;
    if (NStr::StartsWith(qual_field, "gene")) {
        str = CMacroEditorContext::GetInstance().GetAsnPathToFieldName(qual_field, EMacroFieldType::eGene);
        str = "\"" + str + "\"";
        if (CFieldNamePanel::IsFeature(target)) {
            str = "RELATED_FEATURE(\"gene\", " + str + ")";
        }
        else {
            str = "FEATURES(\"gene\"," + str + ")";
        }
    }
    else {
        str = CMacroEditorContext::GetInstance().GetAsnPathToFieldName(rna_type + " " + qual_field, EMacroFieldType::eRNA);
        str = "\"" + str + "\"";
        if (str.find("::product")) {
            str = macro::CMacroFunction_GetRnaProduct::GetFuncName() + "()";
        }
        if (rna_type != "any") {
            if (rna_type != target && rna_type != selected_field) {
                if (CFieldNamePanel::IsFeature(target)) {
                    str = "RELATED_FEATURE(\"" + rna_type + "\", " + str + ")";
                }
                else {
                    str = "FEATURES(\"" + rna_type + "\"," + str + ")";
                }
            }
            //
        }
    }
    return str;
}




/// Dual panel
/*!
 * CDualRNAFieldNamePanel type definition
 */

IMPLEMENT_DYNAMIC_CLASS( CDualRNAFieldNamePanel, CFieldNamePanel )


/*!
 * CDualRNAFieldNamePanel event table definition
 */

BEGIN_EVENT_TABLE( CDualRNAFieldNamePanel, CFieldNamePanel )

////@begin CDualRNAFieldNamePanel event table entries
    EVT_CHOICE( ID_CRNAFIELDNAME_RNA_TYPE, CDualRNAFieldNamePanel::OnRnaTypeSelected)
    EVT_LISTBOX( ID_CRNAFIELDNAME_RNA_FIELD, CDualRNAFieldNamePanel::OnRnaFieldSelected)
    EVT_LISTBOX( ID_CRNAFIELDNAME_RNA_FIELD2, CDualRNAFieldNamePanel::OnRnaFieldSelected)
////@end CDualRNAFieldNamePanel event table entries

END_EVENT_TABLE()


/*!
 * CDualRNAFieldNamePanel constructors
 */

CDualRNAFieldNamePanel::CDualRNAFieldNamePanel() 
{
    Init();
}

CDualRNAFieldNamePanel::CDualRNAFieldNamePanel( wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style )
{
    Init();
    Create(parent, id, pos, size, style);
}


/*!
 * CDualRNAFieldNamePanel creator
 */

bool CDualRNAFieldNamePanel::Create( wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style )
{
////@begin CDualRNAFieldNamePanel creation
    CFieldNamePanel::Create( parent, id, pos, size, style );

    CreateControls();
    if (GetSizer())
    {
        GetSizer()->SetSizeHints(this);
    }
    Centre();
////@end CDualRNAFieldNamePanel creation
    return true;
}


/*!
 * CDualRNAFieldNamePanel destructor
 */

CDualRNAFieldNamePanel::~CDualRNAFieldNamePanel()
{
////@begin CDualRNAFieldNamePanel destruction
////@end CDualRNAFieldNamePanel destruction
}


/*!
 * Member initialisation
 */

void CDualRNAFieldNamePanel::Init()
{
////@begin CDualRNAFieldNamePanel member initialisation
    m_RnaTypeLabel = NULL;
    m_RNAType = NULL;
    m_NcrnaClass = NULL;
    m_RnaField = NULL;
    m_RnaField2 = NULL;
////@end CDualRNAFieldNamePanel member initialisation
}


/*!
 * Control creation for CDualRNAFieldNamePanel
 */

void CDualRNAFieldNamePanel::CreateControls()
{    
////@begin CDualRNAFieldNamePanel content construction
    CDualRNAFieldNamePanel* itemCFieldNamePanel1 = this;

    wxBoxSizer* itemBoxSizer1 = new wxBoxSizer(wxVERTICAL);
    itemCFieldNamePanel1->SetSizer(itemBoxSizer1);

    wxBoxSizer* itemBoxSizer2 = new wxBoxSizer(wxHORIZONTAL);
    itemBoxSizer1->Add(itemBoxSizer2, 0, wxALIGN_CENTER_HORIZONTAL|wxTOP,5);

    m_RnaTypeLabel = new wxStaticText( itemCFieldNamePanel1, wxID_STATIC, _("RNA Type"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer2->Add(m_RnaTypeLabel, 0, wxALIGN_CENTER_VERTICAL|wxRIGHT|wxTOP, 5);

    wxArrayString m_RNATypeStrings;
    m_RNAType = new wxChoice(itemCFieldNamePanel1, ID_CRNAFIELDNAME_RNA_TYPE, wxDefaultPosition, wxDefaultSize, m_RNATypeStrings, 0);
    itemBoxSizer2->Add(m_RNAType, 0, wxALIGN_CENTER_VERTICAL|wxRESERVE_SPACE_EVEN_IF_HIDDEN| wxALL, 5);

    wxArrayString m_NcrnaClassStrings;
    m_NcrnaClassStrings.Add(_("any"));
    m_NcrnaClass = new wxComboBox(itemCFieldNamePanel1, ID_CRNAFIELDNAME_NCRNA_CLASS, _("any"), wxDefaultPosition, wxDefaultSize, m_NcrnaClassStrings, wxCB_DROPDOWN);
    m_NcrnaClass->SetStringSelection(_("any"));
    m_NcrnaClass->Enable(false);
    itemBoxSizer2->Add(m_NcrnaClass, 0, wxALIGN_CENTER_VERTICAL|wxLEFT|wxBOTTOM|wxTOP, 5);

    wxBoxSizer* itemBoxSizer3 = new wxBoxSizer(wxHORIZONTAL);
    itemBoxSizer1->Add(itemBoxSizer3, 0, wxALIGN_CENTER_HORIZONTAL|wxTOP|wxBOTTOM,0);

    wxBoxSizer* itemBoxSizer4 = new wxBoxSizer(wxVERTICAL);
    itemBoxSizer3->Add(itemBoxSizer4, 0, wxALIGN_TOP|wxTOP|wxBOTTOM,0);

    wxStaticText* fromLabel = new wxStaticText( itemCFieldNamePanel1, wxID_STATIC, _("From"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer4->Add(fromLabel, 0, wxALIGN_CENTER_HORIZONTAL|wxTOP, 5);

    wxArrayString m_RnaFieldStrings;
    m_RnaField = new wxListBox(itemCFieldNamePanel1, ID_CRNAFIELDNAME_RNA_FIELD, wxDefaultPosition, wxSize(220, 109), m_RnaFieldStrings, wxLB_SINGLE);
    itemBoxSizer4->Add(m_RnaField, 0, wxALIGN_CENTER_HORIZONTAL|wxTOP, 5);

    wxBoxSizer* itemBoxSizer5 = new wxBoxSizer(wxVERTICAL);
    itemBoxSizer3->Add(itemBoxSizer5, 0, wxALIGN_TOP|wxTOP|wxBOTTOM,0);

    wxStaticText* toLabel = new wxStaticText( itemCFieldNamePanel1, wxID_STATIC, _("To"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer5->Add(toLabel, 0, wxALIGN_CENTER_HORIZONTAL|wxTOP, 5);

    m_RnaField2 = new wxListBox(itemCFieldNamePanel1, ID_CRNAFIELDNAME_RNA_FIELD2, wxDefaultPosition, wxSize(220, 109), m_RnaFieldStrings, wxLB_SINGLE);
    itemBoxSizer5->Add(m_RnaField2, 0, wxALIGN_CENTER_HORIZONTAL|wxTOP, 5);

////@end CDualRNAFieldNamePanel content construction
    vector<string> rna_types = CRNAFieldNamePanel::GetRNATypes();
    ITERATE(vector<string>, it, rna_types) {
        m_RNAType->AppendString(ToWxString(*it));
    }
    m_RNAType->SetStringSelection(rna_types[0]);

    vector<string> rna_fields = CRNAFieldNamePanel::GetRNAFields();
    ITERATE(vector<string>, it, rna_fields) {
        m_RnaField->AppendString(ToWxString(*it));
        m_RnaField2->AppendString(ToWxString(*it));
    }

    vector<string> class_vals = objects::CRNA_gen::GetncRNAClassList();
    ITERATE(vector<string>, it, class_vals) {
        m_NcrnaClass->AppendString(ToWxString(*it));
    }

    x_EnableNcRnaClass();
}

void CDualRNAFieldNamePanel::ClearValues()
{
    m_RNAType->SetSelection(0);
    m_NcrnaClass->SetStringSelection(_("any"));
    m_NcrnaClass->Enable(false);
    m_RnaField->SetSelection(0);
    m_RnaField2->SetSelection(0);
    m_RnaField->SetFirstItem(0);
    m_RnaField2->SetFirstItem(0);
}


/*!
 * Should we show tooltips?
 */

bool CDualRNAFieldNamePanel::ShowToolTips()
{
    return true;
}

/*!
 * Get bitmap resources
 */

wxBitmap CDualRNAFieldNamePanel::GetBitmapResource( const wxString& name )
{
    // Bitmap retrieval
////@begin CDualRNAFieldNamePanel bitmap retrieval
    wxUnusedVar(name);
    return wxNullBitmap;
////@end CDualRNAFieldNamePanel bitmap retrieval
}

/*!
 * Get icon resources
 */

wxIcon CDualRNAFieldNamePanel::GetIconResource( const wxString& name )
{
    // Icon retrieval
////@begin CDualRNAFieldNamePanel icon retrieval
    wxUnusedVar(name);
    return wxNullIcon;
////@end CDualRNAFieldNamePanel icon retrieval
}


string CDualRNAFieldNamePanel::GetRnaType()
{
    string rna_type = "";
    int val = m_RNAType->GetSelection();
    if (val > -1) {
        rna_type = m_RNAType->GetString(val);
        if (NStr::EqualNocase(rna_type, "any")) {
            rna_type = "";
        }  else if (NStr::StartsWith(rna_type, "misc", NStr::eNocase)) {
            rna_type.insert(4, "_");
        } else if (NStr::StartsWith(rna_type, "pre", NStr::eNocase)) {
            rna_type.insert(3, "_");
        } else if (NStr::EqualNocase(rna_type, "ncRNA")) {
            string ncrna_class = ToStdString(m_NcrnaClass->GetValue());
            if (!NStr::EqualNocase (ncrna_class, "any")) {
                rna_type += " " + ncrna_class;
            }
        }
    }
    return rna_type;
}

string CDualRNAFieldNamePanel::GetRnaTypeOnly(void)
{
    int i = m_RNAType->GetSelection();
    if (i != wxNOT_FOUND)
    {
        return m_RNAType->GetString(i).ToStdString();
    }
    return kEmptyStr;
}

string CDualRNAFieldNamePanel::GetNcrnaType(void)
{
    return m_NcrnaClass->GetValue().ToStdString();
}

string CDualRNAFieldNamePanel::GetRnaField(void)
{
    return GetFieldName(true);
}

string CDualRNAFieldNamePanel::GetRnaField2(void)
{
    return GetFieldName2(true);
}


string CDualRNAFieldNamePanel::GetFieldName(const bool subfield)
{
    string field = "";
    int val = m_RnaField->GetSelection();
    if (val < 0) {
        return field;
    }
    field = m_RnaField->GetString(val);
    if (subfield) {
        return field;
    }
    string rna_type = GetRnaType();
    if (!NStr::IsBlank(rna_type)) {
        field = rna_type + " " + field;
    }
        
    return field;
}

string CDualRNAFieldNamePanel::GetFieldName2(const bool subfield)
{
    string field = "";
    int val = m_RnaField2->GetSelection();
    if (val < 0) {
        return field;
    }
    field = m_RnaField2->GetString(val);
    if (subfield) {
        return field;
    }
    string rna_type = GetRnaType();
    if (!NStr::IsBlank(rna_type)) {
        field = rna_type + " " + field;
    }
        
    return field;
}


bool CDualRNAFieldNamePanel::SetRnaType (const string& rna_type)
{
    bool rval = false;
    if (NStr::IsBlank (rna_type)) {
        // choose any
        m_RNAType->SetSelection(0);
    } else {
        string remainder = rna_type;
        if (NStr::EqualNocase(rna_type, "misc_RNA")) {
            m_RNAType->SetStringSelection(ToWxString("miscRNA"));
        } else if (NStr::EqualNocase(rna_type, "pre_RNA")) {
            m_RNAType->SetStringSelection(ToWxString("preRNA"));
        } else {
            for (size_t i = 0; i < m_RNAType->GetStrings().size(); i++) {
                string major_type = ToStdString(m_RNAType->GetString(i));
                if (NStr::StartsWith (rna_type, major_type)) {
                    m_RNAType->SetSelection(i);
                    string remainder = rna_type.substr(major_type.length());
                    NStr::TruncateSpacesInPlace(remainder);
                    if (NStr::EqualNocase (major_type, "ncRNA")) {
                        m_NcrnaClass->SetValue(remainder);
                        remainder = "";
                    } 
                    break;
                }
            }
            if (!NStr::IsBlank(remainder)) {
                rval = false;
            }
        }
    }
    x_EnableNcRnaClass();
    return rval;
}


bool CDualRNAFieldNamePanel::SetFieldName(const string& field)
{
    bool rval = false;
    m_NcrnaClass->Enable(false);
    string rna_type = field;
    for (size_t i = 0; i < m_RnaField->GetCount(); i++) {
        string rna_field = ToStdString(m_RnaField->GetString(i));
        if (NStr::EndsWith (field, rna_field)) {
            m_RnaField->SetSelection(i);
	    m_RnaField->SetFirstItem(i);
            if (rna_field.length() < field.length()) {
                rna_type = field.substr(0, field.length() - rna_field.length());
                NStr::TruncateSpacesInPlace(rna_type);
            }
            rval = true;
            break;
        }
    }

    if (rval) {
        rval = SetRnaType(rna_type);
    }
    x_UpdateParent();
    return rval;
}

bool CDualRNAFieldNamePanel::SetFieldName2(const string& field)
{
    bool rval = false;
    m_NcrnaClass->Enable(false);
    string rna_type = field;
    for (size_t i = 0; i < m_RnaField2->GetCount(); i++) {
        string rna_field = ToStdString(m_RnaField2->GetString(i));
        if (NStr::EndsWith (field, rna_field)) {
            m_RnaField2->SetSelection(i);
	    m_RnaField2->SetFirstItem(i);
            if (rna_field.length() < field.length()) {
                rna_type = field.substr(0, field.length() - rna_field.length());
                NStr::TruncateSpacesInPlace(rna_type);
            }
            rval = true;
            break;
        }
    }

    if (rval) {
        rval = SetRnaType(rna_type);
    }
    x_UpdateParent();
    return rval;
}


void CDualRNAFieldNamePanel::x_EnableNcRnaClass()
{
    string rna_type = ToStdString(m_RNAType->GetStringSelection());
    if (NStr::EqualNocase(rna_type, "ncRNA")) {
        m_NcrnaClass->Enable(true);
    } else {
        m_NcrnaClass->Enable(false);
    }
}


/*!
 * wxEVT_COMMAND_CHOICE_SELECTED event handler for ID_RNA_TYPE
 */

void CDualRNAFieldNamePanel::OnRnaTypeSelected( wxCommandEvent& event )
{
    x_EnableNcRnaClass();
    x_UpdateParent();
    event.Skip();
}

void CDualRNAFieldNamePanel::OnRnaFieldSelected( wxCommandEvent& event )
{
    x_UpdateParent();
    event.Skip();
}


END_NCBI_SCOPE
