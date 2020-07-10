/*  $Id: loc_gcode_panel.cpp 36957 2016-11-16 21:57:46Z asztalos $
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
 * Authors:  Vasuki Palanigobu
 */

// For compilers that support precompilation, includes "wx/wx.h".
#include "wx/wxprec.h"

#ifdef __BORLANDC__
#pragma hdrstop
#endif

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

////@begin includes
////@end includes

#include <ncbi_pch.hpp>
#include <objects/seqfeat/Org_ref.hpp>
#include <objects/seqfeat/OrgName.hpp>
#include <objects/seqfeat/Genetic_code_table.hpp>
#include <objects/seqfeat/Genetic_code.hpp>
#include <gui/widgets/wx/wx_utils.hpp>

#include "loc_gcode_panel.hpp"

BEGIN_NCBI_SCOPE
USING_SCOPE(objects);

////@begin XPM images
////@end XPM images


/*
 * CLocAndGCodePanel type definition
 */

IMPLEMENT_DYNAMIC_CLASS( CLocAndGCodePanel, wxPanel )


/*
 * CLocAndGCodePanel event table definition
 */

BEGIN_EVENT_TABLE( CLocAndGCodePanel, wxPanel )

////@begin CLocAndGCodePanel event table entries
////@end CLocAndGCodePanel event table entries

END_EVENT_TABLE()


/*
 * CLocAndGCodePanel constructors
 */
static CBioSource dummy;

 CLocAndGCodePanel::CLocAndGCodePanel():m_Source(dummy)
{
    Init();
}

CLocAndGCodePanel::CLocAndGCodePanel( wxWindow* parent, objects::CBioSource& source, wxWindowID id, const wxPoint& pos, const wxSize& size, long style):
    m_Source(source)
{
    //m_Source = new CBioSource();
    //m_Source->Assign(source);
    Init();
    Create(parent, id, pos, size, style);
}


/*
 * GeneticCodesPanel creator
 */

bool CLocAndGCodePanel::Create( wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style )
{
////@begin CLocAndGCodePanel creation
    wxPanel::Create( parent, id, pos, size, style );

    CreateControls();
    if (GetSizer())
    {
        GetSizer()->SetSizeHints(this);
    }
    Centre();
////@end CLocAndGCodePanel creation
    return true;
}


/*
 * CLocAndGCodePanel destructor
 */

CLocAndGCodePanel::~CLocAndGCodePanel()
{
////@begin CLocAndGCodePanel destruction
////@end CLocAndGCodePanel destruction
}


/*
 * Member initialisation
 */

void CLocAndGCodePanel::Init()
{
////@begin CLocAndGCodePanel member initialisation
    m_LocCtrl = NULL;
    m_OriginCtrl = NULL;
    m_NuclearCodeCtrl = NULL;
    m_MitoCodeCtrl = NULL;
    m_PlastidCodeCtrl = NULL;
    m_BioFocusCtrl = NULL;
////@end CLocAndGCodePanel member initialisation
    const CGenetic_code_table& tbl = CGen_code_table::GetCodeTable();
    ITERATE(CGenetic_code_table::Tdata, it, tbl.Get()) 
    {
        m_CodeToName[(*it)->GetId()] = (*it)->GetName();
        m_NameToCode[(*it)->GetName()] = (*it)->GetId();
    }
}


/*
 * Control creation for GeneticCodesPanel
 */

void CLocAndGCodePanel::CreateControls()
{    
////@begin CLocAndGCodePanel content construction
    CLocAndGCodePanel* itemPanel1 = this;

    wxBoxSizer* itemBoxSizer2 = new wxBoxSizer(wxVERTICAL);
    itemPanel1->SetSizer(itemBoxSizer2);

    wxStaticBox* itemStaticBoxSizer3Static = new wxStaticBox(itemPanel1, wxID_ANY, _("Location and Origin"));
    wxStaticBoxSizer* itemStaticBoxSizer3 = new wxStaticBoxSizer(itemStaticBoxSizer3Static, wxHORIZONTAL);
    itemBoxSizer2->Add(itemStaticBoxSizer3, 0, wxALIGN_LEFT|wxALL, 5);

    wxFlexGridSizer* itemFlexGridSizer4 = new wxFlexGridSizer(0, 2, 0, 0);
    itemStaticBoxSizer3->Add(itemFlexGridSizer4, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxStaticText* itemStaticText5 = new wxStaticText( itemPanel1, wxID_STATIC, _("Location of sequence"), wxDefaultPosition, wxDefaultSize, 0 );
    itemFlexGridSizer4->Add(itemStaticText5, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxArrayString m_LocCtrlStrings;
    m_LocCtrlStrings.Add(_("mitochondrion"));
    m_LocCtrlStrings.Add(_("chloroplast"));
    m_LocCtrlStrings.Add(wxEmptyString);
    m_LocCtrlStrings.Add(_("apicoplast"));
    m_LocCtrlStrings.Add(_("chloroplast"));
    m_LocCtrlStrings.Add(_("chromatophore"));
    m_LocCtrlStrings.Add(_("chromoplast"));
    m_LocCtrlStrings.Add(_("chromosome"));
    m_LocCtrlStrings.Add(_("cyanelle"));
    m_LocCtrlStrings.Add(_("endogenous-virus"));
    m_LocCtrlStrings.Add(_("extrachromosomal"));
    m_LocCtrlStrings.Add(_("genomic"));
    m_LocCtrlStrings.Add(_("hydrogenosome"));
    m_LocCtrlStrings.Add(_("kinetoplast"));
    m_LocCtrlStrings.Add(_("leucoplast"));
    m_LocCtrlStrings.Add(_("macronuclear"));
    m_LocCtrlStrings.Add(_("mitochondrion"));
    m_LocCtrlStrings.Add(_("nucleomorph"));
    m_LocCtrlStrings.Add(_("plasmid"));
    m_LocCtrlStrings.Add(_("plastid"));
    m_LocCtrlStrings.Add(_("proplastid"));
    m_LocCtrlStrings.Add(_("proviral"));
    m_LocCtrl = new wxComboBox( itemPanel1, ID_COMBOBOX5, wxEmptyString, wxDefaultPosition, wxDefaultSize, m_LocCtrlStrings, wxCB_DROPDOWN );
    itemFlexGridSizer4->Add(m_LocCtrl, 0, wxGROW|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxStaticText* itemStaticText7 = new wxStaticText( itemPanel1, wxID_STATIC, _("Origin of Sequence"), wxDefaultPosition, wxDefaultSize, 0 );
    itemFlexGridSizer4->Add(itemStaticText7, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxArrayString m_OriginCtrlStrings;
    m_OriginCtrlStrings.Add(wxEmptyString);
    m_OriginCtrlStrings.Add(_("Natural"));
    m_OriginCtrlStrings.Add(_("Natural mutant"));
    m_OriginCtrlStrings.Add(_("Mutant"));
    m_OriginCtrlStrings.Add(_("Artificial"));
    m_OriginCtrlStrings.Add(_("Synthetic"));
    m_OriginCtrlStrings.Add(_("Other"));
    m_OriginCtrl = new wxComboBox( itemPanel1, ID_COMBOBOX6, wxEmptyString, wxDefaultPosition, wxDefaultSize, m_OriginCtrlStrings, wxCB_DROPDOWN );
    itemFlexGridSizer4->Add(m_OriginCtrl, 0, wxGROW|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxStaticBox* itemStaticBoxSizer9Static = new wxStaticBox(itemPanel1, wxID_ANY, _("Genetic codes"));
    wxStaticBoxSizer* itemStaticBoxSizer9 = new wxStaticBoxSizer(itemStaticBoxSizer9Static, wxHORIZONTAL);
    itemBoxSizer2->Add(itemStaticBoxSizer9, 0, wxALIGN_LEFT|wxALL, 5);

    wxFlexGridSizer* itemFlexGridSizer10 = new wxFlexGridSizer(0, 2, 0, 0);
    itemStaticBoxSizer9->Add(itemFlexGridSizer10, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxStaticText* itemStaticText11 = new wxStaticText( itemPanel1, wxID_STATIC, _("Nuclear"), wxDefaultPosition, wxDefaultSize, 0 );
    itemFlexGridSizer10->Add(itemStaticText11, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxArrayString m_NuclearCodeCtrlStrings;
    m_NuclearCodeCtrlStrings.Add(wxString(m_CodeToName[1]));
    m_NuclearCodeCtrlStrings.Add(wxString(m_CodeToName[6]));
    m_NuclearCodeCtrlStrings.Add(wxString(m_CodeToName[10]));
    m_NuclearCodeCtrlStrings.Add(wxString(m_CodeToName[12]));
    m_NuclearCodeCtrlStrings.Add(wxString(m_CodeToName[15]));
  
    m_NuclearCodeCtrl = new wxComboBox( itemPanel1, ID_COMBOBOX, wxEmptyString, wxDefaultPosition, wxDefaultSize, m_NuclearCodeCtrlStrings, wxCB_DROPDOWN );
    itemFlexGridSizer10->Add(m_NuclearCodeCtrl, 0, wxGROW|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxStaticText* itemStaticText13 = new wxStaticText( itemPanel1, wxID_STATIC, _("Mitochondrial"), wxDefaultPosition, wxDefaultSize, 0 );
    itemFlexGridSizer10->Add(itemStaticText13, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxArrayString m_MitoCodeCtrlStrings;
    m_MitoCodeCtrlStrings.Add(wxString(m_CodeToName[2]));
    m_MitoCodeCtrlStrings.Add(wxString(m_CodeToName[3]));
    m_MitoCodeCtrlStrings.Add(wxString(m_CodeToName[4]));
    m_MitoCodeCtrlStrings.Add(wxString(m_CodeToName[5]));
    m_MitoCodeCtrlStrings.Add(wxString(m_CodeToName[9]));
    m_MitoCodeCtrlStrings.Add(wxString(m_CodeToName[13]));
    m_MitoCodeCtrlStrings.Add(wxString(m_CodeToName[14]));
    m_MitoCodeCtrlStrings.Add(wxString(m_CodeToName[16]));
    m_MitoCodeCtrlStrings.Add(wxString(m_CodeToName[21]));
    m_MitoCodeCtrlStrings.Add(wxString(m_CodeToName[22]));
    m_MitoCodeCtrlStrings.Add(wxString(m_CodeToName[23]));

    m_MitoCodeCtrl = new wxComboBox( itemPanel1, ID_COMBOBOX1, wxEmptyString, wxDefaultPosition, wxDefaultSize, m_MitoCodeCtrlStrings, wxCB_DROPDOWN );
    itemFlexGridSizer10->Add(m_MitoCodeCtrl, 0, wxGROW|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxStaticText* itemStaticText15 = new wxStaticText( itemPanel1, wxID_STATIC, _("Plastid"), wxDefaultPosition, wxDefaultSize, 0 );
    itemFlexGridSizer10->Add(itemStaticText15, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxArrayString m_PlastidCodeCtrlStrings;
    m_PlastidCodeCtrlStrings.Add(wxString(m_CodeToName[1]));
    m_PlastidCodeCtrlStrings.Add(wxString(m_CodeToName[2]));
    m_PlastidCodeCtrlStrings.Add(wxString(m_CodeToName[3]));
    m_PlastidCodeCtrlStrings.Add(wxString(m_CodeToName[4]));
    m_PlastidCodeCtrlStrings.Add(wxString(m_CodeToName[5]));
    m_PlastidCodeCtrlStrings.Add(wxString(m_CodeToName[6]));
    m_PlastidCodeCtrlStrings.Add(wxString(m_CodeToName[9]));
    m_PlastidCodeCtrlStrings.Add(wxString(m_CodeToName[10]));
    m_PlastidCodeCtrlStrings.Add(wxString(m_CodeToName[12]));
    m_PlastidCodeCtrlStrings.Add(wxString(m_CodeToName[13]));
    m_PlastidCodeCtrlStrings.Add(wxString(m_CodeToName[14]));
    m_PlastidCodeCtrlStrings.Add(wxString(m_CodeToName[15]));
    m_PlastidCodeCtrlStrings.Add(wxString(m_CodeToName[16]));
    m_PlastidCodeCtrlStrings.Add(wxString(m_CodeToName[21]));
    m_PlastidCodeCtrlStrings.Add(wxString(m_CodeToName[22]));
    m_PlastidCodeCtrlStrings.Add(wxString(m_CodeToName[23]));
      
    m_PlastidCodeCtrl = new wxComboBox( itemPanel1, ID_COMBOBOX2, wxEmptyString, wxDefaultPosition, wxDefaultSize, m_PlastidCodeCtrlStrings, wxCB_DROPDOWN );
    itemFlexGridSizer10->Add(m_PlastidCodeCtrl, 0, wxGROW|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    m_BioFocusCtrl = new wxCheckBox( itemPanel1, ID_CHECKBOX1, _("Biological focus (if multiple source features)"), wxDefaultPosition, wxDefaultSize, 0 );
    m_BioFocusCtrl->SetValue(false);
    itemBoxSizer2->Add(m_BioFocusCtrl, 0, wxALIGN_LEFT|wxALL, 5);

////@end CLocAndGCodePanel content construction
}

bool CLocAndGCodePanel::TransferDataToWindow()
{
    if (!wxPanel::TransferDataToWindow())
        return false;
    m_LocCtrl->SetValue(ToWxString(x_GetLocation()));
    m_OriginCtrl->SetValue(ToWxString(x_GetOrigin()));
    m_NuclearCodeCtrl->SetValue(ToWxString(x_GetGeneticCode()));
    m_MitoCodeCtrl->SetValue(ToWxString(x_GetMitochondrialCode()));
    m_PlastidCodeCtrl->SetValue(ToWxString(x_GetPlastidCode()));
    if (m_Source.IsSetIs_focus()) {
        m_BioFocusCtrl->SetValue(true);
    }  else {
        m_BioFocusCtrl->SetValue(false);
    }
        
    return true;
}

bool CLocAndGCodePanel::TransferDataFromWindow()
{
    if (!wxPanel::TransferDataFromWindow())
        return false;
    x_SetLocation();
    x_SetOrigin();
    x_SetGeneticCode();
    x_SetMitochondrialCode();
    x_SetPlastidCode();
    if (m_BioFocusCtrl->GetValue()) {
        m_Source.SetIs_focus();
    } else {
        m_Source.ResetIs_focus();
    }
      return true;
}

void CLocAndGCodePanel::x_SetPlastidCode() 
{
    if (m_PlastidCodeCtrl->IsTextEmpty()) {
        m_Source.SetOrg().SetOrgname().ResetPgcode();
        return;
    }

    string pgcode = ToStdString(m_PlastidCodeCtrl->GetValue());
    if (m_NameToCode.find(pgcode) != m_NameToCode.end())
        m_Source.SetOrg().SetOrgname().SetPgcode(m_NameToCode[pgcode]);   
}

void CLocAndGCodePanel::x_SetMitochondrialCode() 
{
    if (m_MitoCodeCtrl->IsTextEmpty()) {
        m_Source.SetOrg().SetOrgname().ResetMgcode();
        return;
    }

    string mgcode = ToStdString(m_MitoCodeCtrl->GetValue());
    if (m_NameToCode.find(mgcode) != m_NameToCode.end())
        m_Source.SetOrg().SetOrgname().SetMgcode(m_NameToCode[mgcode]);      
}

void CLocAndGCodePanel::x_SetGeneticCode()
{
    if (m_NuclearCodeCtrl->IsTextEmpty()) {
        m_Source.SetOrg().SetOrgname().ResetGcode();
        return;
    }

    string gcode = ToStdString(m_NuclearCodeCtrl->GetValue());
    if (m_NameToCode.find(gcode) != m_NameToCode.end())
        m_Source.SetOrg().SetOrgname().SetGcode(m_NameToCode[gcode]);         
}


void CLocAndGCodePanel::x_SetOrigin()
{
    string origin = ToStdString(m_OriginCtrl->GetValue());
    if (NStr::Equal(origin, "Natural")) {
        m_Source.SetOrigin(CBioSource::eOrigin_natural);
    } else if (NStr::Equal(origin, "Natural mutant")) {
        m_Source.SetOrigin(CBioSource::eOrigin_natmut);
    } else if (NStr::Equal(origin, "Mutant")) {
        m_Source.SetOrigin(CBioSource::eOrigin_mut);
    } else if (NStr::Equal(origin, "Artificial")) {
        m_Source.SetOrigin(CBioSource::eOrigin_artificial);
    } else if (NStr::Equal(origin, "Synthetic")) {
        m_Source.SetOrigin(CBioSource::eOrigin_synthetic);
    } else if (NStr::Equal(origin, "Other")) {
        m_Source.SetOrigin(CBioSource::eOrigin_other);
    } else {
        m_Source.SetOrigin(CBioSource::eOrigin_unknown);
    }
}

void CLocAndGCodePanel::x_SetLocation()
{
    string location = ToStdString(m_LocCtrl->GetValue());
    NStr::ReplaceInPlace(location, "-", "_");
    CBioSource::EGenome genome = CBioSource::GetGenomeByOrganelle(location, NStr::eNocase);
    m_Source.SetGenome(genome);
}

string CLocAndGCodePanel::x_GetPlastidCode()
{
    if (m_Source.IsSetOrg() && m_Source.GetOrg().IsSetOrgname() 
        && m_Source.GetOrg().GetOrgname().IsSetPgcode())
    {
        int code = m_Source.GetOrg().GetOrgname().GetPgcode();
        if (m_CodeToName.find(code) != m_CodeToName.end())
            return m_CodeToName[code];      
    }
    return kEmptyStr;
}

string CLocAndGCodePanel::x_GetMitochondrialCode() 
{
    if (m_Source.IsSetOrg() && m_Source.GetOrg().IsSetOrgname() 
        && m_Source.GetOrg().GetOrgname().IsSetMgcode())
    {
        int code = m_Source.GetOrg().GetOrgname().GetMgcode();
        if (m_CodeToName.find(code) != m_CodeToName.end())
            return m_CodeToName[code];           
    }
    return kEmptyStr;
}

string CLocAndGCodePanel::x_GetGeneticCode()
{
    if (m_Source.IsSetOrg() && m_Source.GetOrg().IsSetOrgname() 
        && m_Source.GetOrg().GetOrgname().IsSetGcode())
    {         
        int code = m_Source.GetOrg().GetOrgname().GetGcode();
        if (m_CodeToName.find(code) != m_CodeToName.end())
            return m_CodeToName[code];          
    }
    return kEmptyStr;
}

string CLocAndGCodePanel::x_GetOrigin()
{
    if ( ! m_Source.IsSetOrigin() ) {
        return "";
    }

    switch(m_Source.GetOrigin())
    {
        case CBioSource::eOrigin_natural:
            return "Natural";
        case CBioSource::eOrigin_natmut:
            return "Natural mutant";
        case CBioSource::eOrigin_mut:
            return "Mutant";
        case CBioSource::eOrigin_artificial:
            return "Artificial";
        case CBioSource::eOrigin_synthetic:
            return "Synthetic";
        case CBioSource::eOrigin_other:
            return "Other";
        default:
            return "";
    }
}

string CLocAndGCodePanel::x_GetLocation()
{
    if ( ! m_Source.IsSetGenome() || m_Source.GetGenome() == CBioSource::eGenome_unknown) {
        return "";
    }
    if (m_Source.GetGenome() == CBioSource::eGenome_extrachrom) {
        return "extrachromosomal";
    }
    string rval = CBioSource::GetOrganelleByGenome(m_Source.GetGenome());
    NStr::ReplaceInPlace(rval, "_", "-");
    NStr::ReplaceInPlace(rval, "plastid:", "");
    return rval;
}

void CLocAndGCodePanel::PopulateLocAndGC(CBioSource& source)
{
    if (m_Source.IsSetIs_focus()) {
        source.SetIs_focus();
    } else {
        source.ResetIs_focus();
    }
    
    source.SetGenome(m_Source.GetGenome());
    source.SetOrigin(m_Source.GetOrigin());
    if (m_Source.IsSetOrg() && m_Source.GetOrg().IsSetOrgname()) {
        if (m_Source.GetOrg().GetOrgname().IsSetGcode()) {
            source.SetOrg().SetOrgname().SetGcode(m_Source.GetOrg().GetOrgname().GetGcode());
        }
        if (m_Source.GetOrg().GetOrgname().IsSetMgcode()) {
            source.SetOrg().SetOrgname().SetMgcode(m_Source.GetOrg().GetOrgname().GetMgcode());
        }
        if (m_Source.GetOrg().GetOrgname().IsSetPgcode()) {
            source.SetOrg().SetOrgname().SetPgcode(m_Source.GetOrg().GetOrgname().GetPgcode());
        }
    }
}

void CLocAndGCodePanel::OnChangedTaxname()
{
    m_NuclearCodeCtrl->SetValue(wxEmptyString);
    m_MitoCodeCtrl->SetValue(wxEmptyString);
    m_PlastidCodeCtrl->SetValue(wxEmptyString);
}

/*
 * Should we show tooltips?
 */

bool CLocAndGCodePanel::ShowToolTips()
{
    return true;
}

/*
 * Get bitmap resources
 */

wxBitmap CLocAndGCodePanel::GetBitmapResource( const wxString& name )
{
    // Bitmap retrieval
////@begin CLocAndGCodePanel bitmap retrieval
    wxUnusedVar(name);
    return wxNullBitmap;
////@end CLocAndGCodePanel bitmap retrieval
}

/*
 * Get icon resources
 */

wxIcon CLocAndGCodePanel::GetIconResource( const wxString& name )
{
    // Icon retrieval
////@begin CLocAndGCodePanel icon retrieval
    wxUnusedVar(name);
    return wxNullIcon;
////@end CLocAndGCodePanel icon retrieval
}

END_NCBI_SCOPE
