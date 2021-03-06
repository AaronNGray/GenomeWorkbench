/*  $Id: igsflankpanel.cpp 28329 2013-06-19 11:58:38Z bollin $
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
#include <gui/widgets/wx/wx_utils.hpp>

////@begin includes
////@end includes

#include <wx/stattext.h>
#include <gui/packages/pkg_sequence_edit/igsflankpanel.hpp>

////@begin XPM images
////@end XPM images

BEGIN_NCBI_SCOPE

/*!
 * CIGSFlankPanel type definition
 */

IMPLEMENT_DYNAMIC_CLASS( CIGSFlankPanel, wxPanel )


/*!
 * CIGSFlankPanel event table definition
 */

BEGIN_EVENT_TABLE( CIGSFlankPanel, wxPanel )

////@begin CIGSFlankPanel event table entries
    EVT_RADIOBUTTON( ID_RADIOBUTTON, CIGSFlankPanel::OntrnabuttonSelected )

    EVT_RADIOBUTTON( ID_RADIOBUTTON1, CIGSFlankPanel::OnProteinSelected )

////@end CIGSFlankPanel event table entries

END_EVENT_TABLE()


/*!
 * CIGSFlankPanel constructors
 */

CIGSFlankPanel::CIGSFlankPanel()
{
    Init();
}

CIGSFlankPanel::CIGSFlankPanel( wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style )
{
    Init();
    Create(parent, id, pos, size, style);
}


/*!
 * CIGSFlankPanel creator
 */

bool CIGSFlankPanel::Create( wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style )
{
////@begin CIGSFlankPanel creation
    wxPanel::Create( parent, id, pos, size, style );

    CreateControls();
    if (GetSizer())
    {
        GetSizer()->SetSizeHints(this);
    }
    Centre();
////@end CIGSFlankPanel creation
    return true;
}


/*!
 * CIGSFlankPanel destructor
 */

CIGSFlankPanel::~CIGSFlankPanel()
{
////@begin CIGSFlankPanel destruction
////@end CIGSFlankPanel destruction
}


/*!
 * Member initialisation
 */

void CIGSFlankPanel::Init()
{
////@begin CIGSFlankPanel member initialisation
    m_IstRNA = NULL;
    m_tRNA = NULL;
    m_IsProtein = NULL;
    m_ProteinName = NULL;
    m_GeneSymbol = NULL;
    m_IncludeFeat = NULL;
////@end CIGSFlankPanel member initialisation
}


/*!
 * Control creation for CIGSFlankPanel
 */

void CIGSFlankPanel::CreateControls()
{    
////@begin CIGSFlankPanel content construction
    // Generated by DialogBlocks, 10/06/2013 17:55:51 (unregistered)

    CIGSFlankPanel* itemPanel1 = this;

    wxBoxSizer* itemBoxSizer2 = new wxBoxSizer(wxVERTICAL);
    itemPanel1->SetSizer(itemBoxSizer2);

    wxFlexGridSizer* itemFlexGridSizer3 = new wxFlexGridSizer(0, 2, 0, 0);
    itemBoxSizer2->Add(itemFlexGridSizer3, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

    m_IstRNA = new wxRadioButton( itemPanel1, ID_RADIOBUTTON, _("tRNA"), wxDefaultPosition, wxDefaultSize, 0 );
    m_IstRNA->SetValue(true);
    itemFlexGridSizer3->Add(m_IstRNA, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxBoxSizer* itemBoxSizer5 = new wxBoxSizer(wxHORIZONTAL);
    itemFlexGridSizer3->Add(itemBoxSizer5, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxStaticText* itemStaticText6 = new wxStaticText( itemPanel1, wxID_STATIC, _("Select tRNA"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer5->Add(itemStaticText6, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxArrayString m_tRNAStrings;
    m_tRNAStrings.Add(_("A Alanine"));
    m_tRNAStrings.Add(_("B Asp or Asn"));
    m_tRNAStrings.Add(_("C Cysteine"));
    m_tRNAStrings.Add(_("D Aspartic Acid"));
    m_tRNAStrings.Add(_("E Glutamic Acid"));
    m_tRNAStrings.Add(_("F Phenylalanine"));
    m_tRNAStrings.Add(_("G Glycine"));
    m_tRNAStrings.Add(_("H Histidine"));
    m_tRNAStrings.Add(_("I Isoleucine"));
    m_tRNAStrings.Add(_("J Leu or Ile"));
    m_tRNAStrings.Add(_("K Lysine"));
    m_tRNAStrings.Add(_("L Leucine"));
    m_tRNAStrings.Add(_("M Methionine"));
    m_tRNAStrings.Add(_("N Asparagine"));
    m_tRNAStrings.Add(_("O Pyrrolysine"));
    m_tRNAStrings.Add(_("P Proline"));
    m_tRNAStrings.Add(_("Q Glutamine"));
    m_tRNAStrings.Add(_("R Arginine"));
    m_tRNAStrings.Add(_("S Serine"));
    m_tRNAStrings.Add(_("T Threonine"));
    m_tRNAStrings.Add(_("U Selenocysteine"));
    m_tRNAStrings.Add(_("V Valine"));
    m_tRNAStrings.Add(_("W Tryptophan"));
    m_tRNAStrings.Add(_("X Undetermined or atypical"));
    m_tRNAStrings.Add(_("Y Tyrosine"));
    m_tRNAStrings.Add(_("Z Glu or Gln"));
    m_tRNA = new wxChoice( itemPanel1, ID_CHOICE, wxDefaultPosition, wxDefaultSize, m_tRNAStrings, 0 );
    itemBoxSizer5->Add(m_tRNA, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    m_IsProtein = new wxRadioButton( itemPanel1, ID_RADIOBUTTON1, _("protein coding gene"), wxDefaultPosition, wxDefaultSize, 0 );
    m_IsProtein->SetValue(false);
    itemFlexGridSizer3->Add(m_IsProtein, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxFlexGridSizer* itemFlexGridSizer9 = new wxFlexGridSizer(0, 2, 0, 0);
    itemFlexGridSizer3->Add(itemFlexGridSizer9, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxStaticText* itemStaticText10 = new wxStaticText( itemPanel1, wxID_STATIC, _("Protein Name"), wxDefaultPosition, wxDefaultSize, 0 );
    itemFlexGridSizer9->Add(itemStaticText10, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    m_ProteinName = new wxTextCtrl( itemPanel1, ID_TEXTCTRL3, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
    m_ProteinName->Enable(false);
    itemFlexGridSizer9->Add(m_ProteinName, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxStaticText* itemStaticText12 = new wxStaticText( itemPanel1, wxID_STATIC, _("Gene Symbol"), wxDefaultPosition, wxDefaultSize, 0 );
    itemFlexGridSizer9->Add(itemStaticText12, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    m_GeneSymbol = new wxTextCtrl( itemPanel1, ID_TEXTCTRL4, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
    m_GeneSymbol->Enable(false);
    itemFlexGridSizer9->Add(m_GeneSymbol, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxArrayString m_IncludeFeatStrings;
    m_IncludeFeatStrings.Add(_("&Yes"));
    m_IncludeFeatStrings.Add(_("Yes, &but only in some of the sequences"));
    m_IncludeFeatStrings.Add(_("&No, it is only the intergenic spacer on this end"));
    m_IncludeFeat = new wxRadioBox( itemPanel1, ID_RADIOBOX1, _("Do your sequences contain part of the feature described above?"), wxDefaultPosition, wxDefaultSize, m_IncludeFeatStrings, 1, wxRA_SPECIFY_COLS );
    m_IncludeFeat->SetSelection(0);
    itemBoxSizer2->Add(m_IncludeFeat, 0, wxGROW|wxALL, 5);

////@end CIGSFlankPanel content construction
}


/*!
 * Should we show tooltips?
 */

bool CIGSFlankPanel::ShowToolTips()
{
    return true;
}

/*!
 * Get bitmap resources
 */

wxBitmap CIGSFlankPanel::GetBitmapResource( const wxString& name )
{
    // Bitmap retrieval
////@begin CIGSFlankPanel bitmap retrieval
    wxUnusedVar(name);
    return wxNullBitmap;
////@end CIGSFlankPanel bitmap retrieval
}

/*!
 * Get icon resources
 */

wxIcon CIGSFlankPanel::GetIconResource( const wxString& name )
{
    // Icon retrieval
////@begin CIGSFlankPanel icon retrieval
    wxUnusedVar(name);
    return wxNullIcon;
////@end CIGSFlankPanel icon retrieval
}

string CIGSFlankPanel::GetIGSLabel()
{
    string name = "";
    if (m_IstRNA->GetValue()) {
        string trna = ToStdString(m_tRNA->GetStringSelection());
        if (!NStr::IsBlank(trna)) {            
            name = "trn" + trna.substr(0, 1);
        }
    } else if (m_IsProtein->GetValue()) {
        string prot = ToStdString(m_ProteinName->GetValue());
        string gene = ToStdString(m_GeneSymbol->GetValue());
        if (NStr::IsBlank(gene)) {
            name = prot;
        } else {
            name = gene;
        }
    }
    return name;
}


string s_GetTRNAAbbrev (string trna_label)
{
    string abbrev = "";

    // temporary
    if (NStr::Find(trna_label, " or ")) {
        abbrev = trna_label.substr(0, 2) + "x";
    } else {
        abbrev = trna_label.substr(0, 3);
    }
    return abbrev;
}

string CIGSFlankPanel::GetFlankLabel()
{
    string name = "";
    if (m_IncludeFeat->GetSelection() == 0 || m_IncludeFeat->GetSelection() == 1) {
        if (m_IstRNA->GetValue()) {
            string trna = ToStdString(m_tRNA->GetStringSelection());
            if (!NStr::IsBlank(trna)) {            
                name = "tRNA-" + s_GetTRNAAbbrev(trna.substr(2)) + " (trn" + trna.substr(0, 1) + ")";
            }
        } else if (m_IsProtein->GetValue()) {
            string prot = ToStdString(m_ProteinName->GetValue());
            string gene = ToStdString(m_GeneSymbol->GetValue());
            if (NStr::IsBlank(prot)) {
                if (!NStr::IsBlank(gene)) {
                    name = gene;
                }
            } else {
                if (NStr::IsBlank(gene)) {
                    name = prot;
                } else {
                    name = prot + " (" + gene + ")";
                }
            }
        }
    }
    return name;
}


/*!
 * wxEVT_COMMAND_RADIOBUTTON_SELECTED event handler for ID_RADIOBUTTON
 */

void CIGSFlankPanel::OntrnabuttonSelected( wxCommandEvent& event )
{
    m_tRNA->Enable(true);
    m_ProteinName->Enable(false);
    m_GeneSymbol->Enable(false);
}


/*!
 * wxEVT_COMMAND_RADIOBUTTON_SELECTED event handler for ID_RADIOBUTTON1
 */

void CIGSFlankPanel::OnProteinSelected( wxCommandEvent& event )
{
    m_tRNA->Enable(false);
    m_ProteinName->Enable(true);
    m_GeneSymbol->Enable(true);
}


END_NCBI_SCOPE
