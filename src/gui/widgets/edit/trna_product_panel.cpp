/*  $Id: trna_product_panel.cpp 43077 2019-05-13 12:30:05Z bollin $
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
#include <objects/seqfeat/RNA_ref.hpp>
#include <objects/seqfeat/Trna_ext.hpp>
#include <objects/seqfeat/seqfeat_macros.hpp>

#include <util/sequtil/sequtil_convert.hpp>
#include <gui/widgets/wx/wx_utils.hpp>
#include <gui/widgets/edit/gbqual_panel.hpp>
#include "trna_product_panel.hpp"

#include <wx/sizer.h>
#include <wx/choice.h>
#include <wx/stattext.h>
#include <wx/radiobox.h>
#include <wx/treebook.h>


BEGIN_NCBI_SCOPE
USING_SCOPE(objects);


/*!
 * CtRNAProductPanel type definition
 */

IMPLEMENT_DYNAMIC_CLASS( CtRNAProductPanel, wxPanel )


/*!
 * CtRNAProductPanel event table definition
 */

BEGIN_EVENT_TABLE( CtRNAProductPanel, wxPanel )

////@begin CtRNAProductPanel event table entries
    EVT_CHOICE( ID_TRNAPRODUCT_CHOICE, CtRNAProductPanel::OnAminoAcidSelected )
    EVT_RADIOBOX( ID_TRNAPRODUCT_RDBX, CtRNAProductPanel::OnInitiatorSelected )
////@end CtRNAProductPanel event table entries

END_EVENT_TABLE()


/*!
 * CtRNAProductPanel constructors
 */

CtRNAProductPanel::CtRNAProductPanel()
    : m_GbQualPanel(0)
{
    Init();
}

CtRNAProductPanel::CtRNAProductPanel(wxWindow* parent, CRef<CTrna_ext> trna, CConstRef<CSeq_feat> feat, wxWindowID id, const wxPoint& pos, const wxSize& size, long style)
    : m_trna(trna), m_Feat(feat), m_GbQualPanel(0)
{
    Init();
    Create(parent, id, pos, size, style);
}


/*!
 * CtRNAProductPanel creator
 */

bool CtRNAProductPanel::Create( wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style )
{
////@begin CtRNAProductPanel creation
    wxPanel::Create( parent, id, pos, size, style );

    CreateControls();
    if (GetSizer())
    {
        GetSizer()->SetSizeHints(this);
    }
    Centre();
////@end CtRNAProductPanel creation
    return true;
}


/*!
 * CtRNAProductPanel destructor
 */

CtRNAProductPanel::~CtRNAProductPanel()
{
////@begin CtRNAProductPanel destruction
////@end CtRNAProductPanel destruction
}


/*!
 * Member initialisation
 */

void CtRNAProductPanel::Init()
{
////@begin CtRNAProductPanel member initialisation
    m_AminoAcidCtrl = NULL;
    m_InitiatorCtrl = NULL;
////@end CtRNAProductPanel member initialisation
}


/*!
 * Control creation for CtRNAProductPanel
 */

void CtRNAProductPanel::CreateControls()
{    
////@begin CtRNAProductPanel content construction
    CtRNAProductPanel* itemPanel1 = this;

    wxBoxSizer* itemBoxSizer2 = new wxBoxSizer(wxVERTICAL);
    itemPanel1->SetSizer(itemBoxSizer2);

    wxBoxSizer* itemBoxSizer3 = new wxBoxSizer(wxHORIZONTAL);
    itemBoxSizer2->Add(itemBoxSizer3, 0, wxALIGN_CENTER, 0);

    wxStaticText* itemStaticText4 = new wxStaticText( itemPanel1, wxID_STATIC, _("Amino Acid"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer3->Add(itemStaticText4, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxArrayString m_AminoAcidCtrlStrings;
    m_AminoAcidCtrlStrings.Add(wxEmptyString);
    m_AminoAcidCtrlStrings.Add(_("A Alanine"));
    m_AminoAcidCtrlStrings.Add(_("B Asp or Asn"));
    m_AminoAcidCtrlStrings.Add(_("C Cysteine"));
    m_AminoAcidCtrlStrings.Add(_("D Aspartic Acid"));
    m_AminoAcidCtrlStrings.Add(_("E Glutamic Acid"));
    m_AminoAcidCtrlStrings.Add(_("F Phenylalanine"));
    m_AminoAcidCtrlStrings.Add(_("G Glycine"));
    m_AminoAcidCtrlStrings.Add(_("H Histidine"));
    m_AminoAcidCtrlStrings.Add(_("I Isoleucine"));
    m_AminoAcidCtrlStrings.Add(_("J Leu or Ile"));
    m_AminoAcidCtrlStrings.Add(_("K Lysine"));
    m_AminoAcidCtrlStrings.Add(_("L Leucine"));
    m_AminoAcidCtrlStrings.Add(_("M Methionine"));
    m_AminoAcidCtrlStrings.Add(_("N Asparagine"));
    m_AminoAcidCtrlStrings.Add(_("O Pyrrolysine"));
    m_AminoAcidCtrlStrings.Add(_("P Proline"));
    m_AminoAcidCtrlStrings.Add(_("Q Glutamine"));
    m_AminoAcidCtrlStrings.Add(_("R Arginine"));
    m_AminoAcidCtrlStrings.Add(_("S Serine"));
    m_AminoAcidCtrlStrings.Add(_("T Threonine"));
    m_AminoAcidCtrlStrings.Add(_("U Selenocysteine"));
    m_AminoAcidCtrlStrings.Add(_("V Valine"));
    m_AminoAcidCtrlStrings.Add(_("W Tryptophan"));
    m_AminoAcidCtrlStrings.Add(_("X Undetermined"));
    m_AminoAcidCtrlStrings.Add(_("Y Tyrosine"));
    m_AminoAcidCtrlStrings.Add(_("Z Glu or Gln"));
    m_AminoAcidCtrlStrings.Add(_("* Stop Codon"));
    m_AminoAcidCtrl = new wxChoice( itemPanel1, ID_TRNAPRODUCT_CHOICE, wxDefaultPosition, wxDefaultSize, m_AminoAcidCtrlStrings, 0 );
    itemBoxSizer3->Add(m_AminoAcidCtrl, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxArrayString m_InitiatorCtrlStrings;
    m_InitiatorCtrlStrings.Add(_("&No"));
    m_InitiatorCtrlStrings.Add(_("&fMet"));
    m_InitiatorCtrlStrings.Add(_("&iMet"));
    m_InitiatorCtrl = new wxRadioBox( itemPanel1, ID_TRNAPRODUCT_RDBX, _("Initiator"), wxDefaultPosition, wxDefaultSize, m_InitiatorCtrlStrings, 1, wxRA_SPECIFY_ROWS );
    m_InitiatorCtrl->SetSelection(0);
    m_InitiatorCtrl->Enable(false);
    itemBoxSizer2->Add(m_InitiatorCtrl, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

////@end CtRNAProductPanel content construction
}


void CtRNAProductPanel::x_SetAminoAcid ()
{
    unsigned char aa = 0;
    vector<char> seqData;
    string str = "";

    if (!m_trna || !m_trna->IsSetAa()) {
        m_AminoAcidCtrl->SetSelection(0);
    } else {
    
        switch (m_trna->GetAa().Which()) {
            case CTrna_ext::C_Aa::e_Iupacaa:
                str = m_trna->GetAa().GetIupacaa();
                CSeqConvert::Convert(str, CSeqUtil::e_Iupacaa, 0, str.size(), seqData, CSeqUtil::e_Ncbieaa);
                aa = seqData[0];
                break;
            case CTrna_ext::C_Aa::e_Ncbi8aa:
                str = m_trna->GetAa().GetNcbi8aa();
                CSeqConvert::Convert(str, CSeqUtil::e_Ncbi8aa, 0, str.size(), seqData, CSeqUtil::e_Ncbieaa);
                aa = seqData[0];
                break;
            case CTrna_ext::C_Aa::e_Ncbistdaa:
                str = m_trna->GetAa().GetNcbi8aa();
                CSeqConvert::Convert(str, CSeqUtil::e_Ncbistdaa, 0, str.size(), seqData, CSeqUtil::e_Ncbieaa);
                aa = seqData[0];
                break;
            case CTrna_ext::C_Aa::e_Ncbieaa:
                seqData.push_back(m_trna->GetAa().GetNcbieaa());
                aa = seqData[0];
                break;
            default:
                break;
        }

        switch (aa) {
            case 'A':
                m_AminoAcidCtrl->SetSelection(1);
                break;
            case 'B':
                m_AminoAcidCtrl->SetSelection(2);
                break;
            case 'C':
                m_AminoAcidCtrl->SetSelection(3);
                break;
            case 'D':
                m_AminoAcidCtrl->SetSelection(4);
                break;
            case 'E':
                m_AminoAcidCtrl->SetSelection(5);
                break;
            case 'F':
                m_AminoAcidCtrl->SetSelection(6);
                break;
            case 'G':
                m_AminoAcidCtrl->SetSelection(7);
                break;
            case 'H':
                m_AminoAcidCtrl->SetSelection(8);
                break;
            case 'I':
                m_AminoAcidCtrl->SetSelection(9);
                break;
            case 'J':
                m_AminoAcidCtrl->SetSelection(10);
                break;
            case 'K':
                m_AminoAcidCtrl->SetSelection(11);
                break;
            case 'L':
                m_AminoAcidCtrl->SetSelection(12);
                break;
            case 'M':
                m_AminoAcidCtrl->SetSelection(13);
                break;
            case 'N':
                m_AminoAcidCtrl->SetSelection(14);
                break;
            case 'O':
                m_AminoAcidCtrl->SetSelection(15);
                break;
            case 'P':
                m_AminoAcidCtrl->SetSelection(16);
                break;
            case 'Q':
                m_AminoAcidCtrl->SetSelection(17);
                break;
            case 'R':
                m_AminoAcidCtrl->SetSelection(18);
                break;
            case 'S':
                m_AminoAcidCtrl->SetSelection(19);
                break;
            case 'T':
                m_AminoAcidCtrl->SetSelection(20);
                break;
            case 'U':
                m_AminoAcidCtrl->SetSelection(21);
                break;
            case 'V':
                m_AminoAcidCtrl->SetSelection(22);
                break;
            case 'W':
                m_AminoAcidCtrl->SetSelection(23);
                break;
            case 'X':
                m_AminoAcidCtrl->SetSelection(24);
                break;
            case 'Y':
                m_AminoAcidCtrl->SetSelection(25);
                break;
            case 'Z':
                m_AminoAcidCtrl->SetSelection(26);
                break;
            case '*':
                m_AminoAcidCtrl->SetSelection(27);
                break;
            default:
                m_AminoAcidCtrl->SetSelection(0);
                break;
        }
    }
}


bool CtRNAProductPanel::TransferDataToWindow()
{
    if (!wxPanel::TransferDataToWindow())
        return false;

    // amino acid
    x_SetAminoAcid();
    x_EnableInitiatorSelection();

    // set the initiator radiobutton
    
    if (m_InitiatorCtrl->IsEnabled()) {
        FOR_EACH_GBQUAL_ON_FEATURE(it, *m_Feat) {
            const CGb_qual& qual = **it;
            if (NStr::EqualCase(qual.GetQual(), "product") && qual.IsSetVal()) {
                if (NStr::EqualCase(qual.GetVal(), CGBQualPanel::stRNA_FMet)) {
                    m_InitiatorCtrl->SetSelection(1);
                }
                else if (NStr::EqualCase(qual.GetVal(), CGBQualPanel::stRNA_IMet)) {
                    m_InitiatorCtrl->SetSelection(2);
                }
                break;
            }
        }
    }

    return true;
}


bool CtRNAProductPanel::TransferDataFromWindow()
{
    if (!wxPanel::TransferDataFromWindow())
        return false;

    // amino acid
    string aa = ToStdString( m_AminoAcidCtrl->GetStringSelection());
    if (NStr::IsBlank(aa)) {
        m_trna->ResetAa();
    } else {
        m_trna->SetAa().SetNcbieaa(aa.c_str()[0]);
    }

    return true;
}


/*!
 * Should we show tooltips?
 */

bool CtRNAProductPanel::ShowToolTips()
{
    return true;
}

/*!
 * Get bitmap resources
 */

wxBitmap CtRNAProductPanel::GetBitmapResource( const wxString& name )
{
    // Bitmap retrieval
////@begin CtRNAProductPanel bitmap retrieval
    wxUnusedVar(name);
    return wxNullBitmap;
////@end CtRNAProductPanel bitmap retrieval
}

/*!
 * Get icon resources
 */

wxIcon CtRNAProductPanel::GetIconResource( const wxString& name )
{
    // Icon retrieval
////@begin CtRNAProductPanel icon retrieval
    wxUnusedVar(name);
    return wxNullIcon;
////@end CtRNAProductPanel icon retrieval
}

void CtRNAProductPanel::OnAminoAcidSelected( wxCommandEvent& event )
{
    x_EnableInitiatorSelection();
}

void CtRNAProductPanel::x_EnableInitiatorSelection()
{
    string aa = ToStdString(m_AminoAcidCtrl->GetStringSelection());
    m_InitiatorCtrl->Enable(NStr::EqualNocase(aa, "M Methionine"));
}

void CtRNAProductPanel::OnInitiatorSelected(wxCommandEvent& event)
{
    _ASSERT(m_GbQualPanel);
    switch (m_InitiatorCtrl->GetSelection()) {
    case 0: // no
        m_GbQualPanel->Update_tRNA_ProductQual(kEmptyStr);
        break;
    case 1: // fMet
        m_GbQualPanel->Update_tRNA_ProductQual(CGBQualPanel::stRNA_FMet);
        break;
    case 2: // iMet
        m_GbQualPanel->Update_tRNA_ProductQual(CGBQualPanel::stRNA_IMet);
        break;
    }
}

END_NCBI_SCOPE
