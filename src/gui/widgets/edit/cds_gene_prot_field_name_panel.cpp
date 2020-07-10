/*  $Id: cds_gene_prot_field_name_panel.cpp 44691 2020-02-21 19:19:39Z asztalos $
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
#include <objtools/edit/field_handler.hpp>
#include <gui/widgets/wx/wx_utils.hpp>
#include <gui/widgets/edit/cds_gene_prot_field_name_panel.hpp>

#include <wx/sizer.h>

////@begin XPM images
////@end XPM images

BEGIN_NCBI_SCOPE

/*!
 * CCDSGeneProtFieldNamePanel type definition
 */

IMPLEMENT_DYNAMIC_CLASS( CCDSGeneProtFieldNamePanel, CFieldNamePanel )


/*!
 * CCDSGeneProtFieldNamePanel event table definition
 */

BEGIN_EVENT_TABLE( CCDSGeneProtFieldNamePanel, CFieldNamePanel )

////@begin CCDSGeneProtFieldNamePanel event table entries
    EVT_LISTBOX( ID_CCDSGENEPROTFIELDNAME_FIELD, CCDSGeneProtFieldNamePanel::OnFieldSelected)

////@end CCDSGeneProtFieldNamePanel event table entries

END_EVENT_TABLE()


/*!
 * CCDSGeneProtFieldNamePanel constructors
 */

CCDSGeneProtFieldNamePanel::CCDSGeneProtFieldNamePanel()
{
    Init();
}

CCDSGeneProtFieldNamePanel::CCDSGeneProtFieldNamePanel( wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style)
{
    Init();
    Create(parent, id, pos, size, style);
}


/*!
 * CCDSGeneProtFieldNamePanel creator
 */

bool CCDSGeneProtFieldNamePanel::Create( wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style )
{
////@begin CCDSGeneProtFieldNamePanel creation
    CFieldNamePanel::Create( parent, id, pos, size, style );

    CreateControls();
    if (GetSizer())
    {
        GetSizer()->SetSizeHints(this);
    }
    Centre();
////@end CCDSGeneProtFieldNamePanel creation
    return true;
}


/*!
 * CCDSGeneProtFieldNamePanel destructor
 */

CCDSGeneProtFieldNamePanel::~CCDSGeneProtFieldNamePanel()
{
////@begin CCDSGeneProtFieldNamePanel destruction
////@end CCDSGeneProtFieldNamePanel destruction
}


/*!
 * Member initialisation
 */

void CCDSGeneProtFieldNamePanel::Init()
{
////@begin CCDSGeneProtFieldNamePanel member initialisation
    m_Field = NULL;
////@end CCDSGeneProtFieldNamePanel member initialisation
}


/*!
 * Control creation for CCDSGeneProtFieldNamePanel
 */

void CCDSGeneProtFieldNamePanel::CreateControls()
{    
////@begin CCDSGeneProtFieldNamePanel content construction
    CCDSGeneProtFieldNamePanel* itemCFieldNamePanel1 = this;

    wxBoxSizer* itemBoxSizer2 = new wxBoxSizer(wxVERTICAL);
    itemCFieldNamePanel1->SetSizer(itemBoxSizer2);

    vector<string> options = GetStrings();
    wxArrayString m_FieldStrings;
    ITERATE(vector<string>, it, options) {
        m_FieldStrings.Add(ToWxString(*it));
    }
    m_Field = new wxListBox(itemCFieldNamePanel1, ID_CCDSGENEPROTFIELDNAME_FIELD, wxDefaultPosition, wxSize(220, 109), m_FieldStrings, 0);
    itemBoxSizer2->Add(m_Field, 0, wxALIGN_CENTER_HORIZONTAL|wxLEFT|wxRIGHT, 5);

////@end CCDSGeneProtFieldNamePanel content construction
}

void CCDSGeneProtFieldNamePanel::ClearValues()
{
    m_Field->SetSelection(0);
    m_Field->SetFirstItem(0);
}

/*!
 * Should we show tooltips?
 */

bool CCDSGeneProtFieldNamePanel::ShowToolTips()
{
    return true;
}

/*!
 * Get bitmap resources
 */

wxBitmap CCDSGeneProtFieldNamePanel::GetBitmapResource( const wxString& name )
{
    // Bitmap retrieval
////@begin CCDSGeneProtFieldNamePanel bitmap retrieval
    wxUnusedVar(name);
    return wxNullBitmap;
////@end CCDSGeneProtFieldNamePanel bitmap retrieval
}

/*!
 * Get icon resources
 */

wxIcon CCDSGeneProtFieldNamePanel::GetIconResource( const wxString& name )
{
    // Icon retrieval
////@begin CCDSGeneProtFieldNamePanel icon retrieval
    wxUnusedVar(name);
    return wxNullIcon;
////@end CCDSGeneProtFieldNamePanel icon retrieval
}


/*!
 * wxEVT_COMMAND_CHOICE_SELECTED event handler for ID_CCDSGENEPROTFIELDNAME_FIELD
 */

void CCDSGeneProtFieldNamePanel::OnFieldSelected( wxCommandEvent& event )
{
    x_UpdateParent();
    event.Skip();
}


string CCDSGeneProtFieldNamePanel::GetFieldName(const bool subfield)
{
    string field = "";
    int val = m_Field->GetSelection();
    if (val < 0) {
        return field;
    }
    field = m_Field->GetString(val);
    return field;
}


bool CCDSGeneProtFieldNamePanel::SetFieldName(const string& field)
{
    bool rval = false;
    for (size_t i = 0; i < m_Field->GetStrings().size(); i++) {
        string cgp_field = ToStdString(m_Field->GetString(i));
        if (objects::edit::CFieldHandler::QualifierNamesAreEquivalent(field, cgp_field)) {
            m_Field->SetSelection(i);
	    m_Field->SetFirstItem(i);
            rval = true;
            break;
        }
    }
    return rval;
}
 
vector<string> CCDSGeneProtFieldNamePanel::GetStrings()
{
    vector<string> options;
    options.push_back("protein name");
    options.push_back("protein description");
    options.push_back("CDS comment");
    options.push_back("CDS inference");
    options.push_back("codon-start");
    options.push_back("gene locus");
    options.push_back("gene description");
    options.push_back("gene comment");
    options.push_back("gene inference");
    options.push_back("gene allele");
    options.push_back("gene maploc");
    options.push_back("gene locus tag");
    options.push_back("gene synonym");
    options.push_back("gene old_locus_tag");
    options.push_back("mRNA product");
    options.push_back("mRNA comment");
    options.push_back("protein name");
    options.push_back("protein description");
    options.push_back("protein EC number");
    options.push_back("protein activity");
    options.push_back("protein comment");
    options.push_back("mat_peptide name");
    options.push_back("mat_peptide description");
    options.push_back("mat_peptide EC number");
    options.push_back("mat_peptide activity");
    options.push_back("mat_peptide comment");
    return options;
}

vector<string> CCDSGeneProtFieldNamePanel::GetChoices(bool& allow_other)
{
    vector<string> choices;

    string field_name = GetFieldName();

    if (field_name == "codon-start")
    {
        choices.push_back("1");
        choices.push_back("2");
        choices.push_back("3");       
        allow_other = false;        
    }

    return choices;
}

string CCDSGeneProtFieldNamePanel::GetMacroFieldName(const string &target, const string& selected_field)
{
    string feature, qualifier;
   
    string field_name = GetFieldName();
    SIZE_TYPE pos = field_name.find(" ");
    if (pos != NPOS) {
        feature = field_name.substr(0, pos);
        qualifier = field_name.substr(pos + 1);
    }  

    if (NStr::EqualNocase(feature, "CDS")) {
        feature = "cdregion";
    }

    string str;
    if (field_name == "codon-start") {
        feature = "cdregion";
        str = "\"data.cdregion.frame\"";
    }
    
    if (qualifier == "name") {
        str = "\"data.prot.name\"";
    }
    else if (qualifier == "description") {
        if (NStr::EqualNocase(feature, "gene")) {
            str = "\"data.gene.desc\"";
        }
        else if (NStr::EqualNocase(feature, "protein") || NStr::EqualNocase(feature, "mat-peptide")) {
            str = "\"data.prot.desc\"";
        }
    }
    else if (qualifier == "comment") {
        str = "\"comment\"";
    }
    else if (qualifier == "locus") {
        str = "\"data.gene.locus\"";
    }
    else if (qualifier == "allele") {
        str = "\"data.gene.allele\"";
    }
    else if (qualifier == "maploc") {
        str = "\"data.gene.maploc\"";
    }
    else if (qualifier == "locus tag") {
        str = "\"data.gene.locus-tag\"";
    }
    else if (qualifier == "synonym") {
        str = "\"data.gene.syn\"";
    }
    else if (qualifier == "EC Number") {
        str = "\"data.prot.ec\"";
    }
    else if (qualifier == "activity") {
        str = "\"data.prot.activity\"";
    }

    if (qualifier == "product" && feature == "mRNA") {
        str = "\"data.rna.ext.name\"";
    }

    if (str.empty()) {
        str = "\"qual\", \"" + qualifier + "\"";
    }

    if (NStr::EqualNocase(target, feature)) {
        return str;
    }

    if (NStr::EqualNocase(feature, "cdregion")) {
        feature = "cds";
    }

    if (CFieldNamePanel::IsFeature(target))
    {
        str = "RELATED_FEATURE(\"" + feature + "\", " + str + ")";
        return str;
    }
    str = "FEATURES(\"" + feature + "\"," + str + ")";
    return str;
}

END_NCBI_SCOPE
