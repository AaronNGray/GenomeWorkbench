/*  $Id: molinfo_panel.cpp 38676 2017-06-08 14:41:06Z filippov $
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
 * Authors:  Roman Katargin
 */


#include <ncbi_pch.hpp>
#include <objects/seq/Seq_inst.hpp>
#include <serial/enumvalues.hpp>

////@begin includes
////@end includes

#include <wx/panel.h>
#include <wx/sizer.h>
#include <wx/bitmap.h>
#include <wx/icon.h>
#include <wx/choice.h>
#include <wx/textctrl.h>
#include <wx/stattext.h>
#include <wx/valtext.h>
#include <wx/toplevel.h>

#include "molinfo_panel.hpp"
#include <gui/widgets/wx/wx_utils.hpp>

////@begin XPM images
////@end XPM images

BEGIN_NCBI_SCOPE
USING_SCOPE(objects);


/*!
 * CMolInfoPanel type definition
 */

IMPLEMENT_DYNAMIC_CLASS( CMolInfoPanel, wxPanel )


/*!
 * CMolInfoPanel event table definition
 */

BEGIN_EVENT_TABLE( CMolInfoPanel, wxPanel )

////@begin CMolInfoPanel event table entries
    EVT_CHOICE( ID_CHOICE8, CMolInfoPanel::OnChoice8Selected )

////@end CMolInfoPanel event table entries

END_EVENT_TABLE()


/*!
 * CMolInfoPanel constructors
 */

CMolInfoPanel::CMolInfoPanel()
{
    Init();
}

CMolInfoPanel::CMolInfoPanel( wxWindow* parent, CRef<CMolInfo> molinfo, CRef<CBioseq> bioseq, wxWindowID id, const wxString& caption, const wxPoint& pos, const wxSize& size, long style )
{
    Init();
    m_Molinfo = new objects::CMolInfo();
    if (molinfo) {
        m_Molinfo->Assign(*molinfo);
    }
    m_Bioseq = new objects::CBioseq();
    if (bioseq) {
        m_Bioseq->Assign(*bioseq);
    }

    Create(parent, id, caption, pos, size, style);
}


/*!
 * CMolInfoPanel creator
 */

bool CMolInfoPanel::Create( wxWindow* parent, wxWindowID id, const wxString& caption, const wxPoint& pos, const wxSize& size, long style )
{
////@begin CMolInfoPanel creation
    SetExtraStyle(wxWS_EX_BLOCK_EVENTS);
    wxPanel::Create( parent, id, pos, size, style );

    CreateControls();
    if (GetSizer())
    {
        GetSizer()->SetSizeHints(this);
    }
    Centre();
////@end CMolInfoPanel creation
    return true;
}


/*!
 * CMolInfoPanel destructor
 */

CMolInfoPanel::~CMolInfoPanel()
{
////@begin CMolInfoPanel destruction
////@end CMolInfoPanel destruction
}


/*!
 * Member initialisation
 */

void CMolInfoPanel::Init()
{
////@begin CMolInfoPanel member initialisation
    m_MoleculeCtrl = NULL;
    m_CompletednessCtrl = NULL;
    m_TechniqueCtrl = NULL;
    m_TechExpCtrl = NULL;
    m_TopologyCtrl = NULL;
    m_StrandednessCtrl = NULL;
////@end CMolInfoPanel member initialisation
}


/*!
 * Control creation for CMolInfoPanel
 */

void CMolInfoPanel::CreateControls()
{    
////@begin CMolInfoPanel content construction
    CMolInfoPanel* itemPanel1 = this;

    wxBoxSizer* itemBoxSizer2 = new wxBoxSizer(wxVERTICAL);
    itemPanel1->SetSizer(itemBoxSizer2);

    wxFlexGridSizer* itemFlexGridSizer3 = new wxFlexGridSizer(0, 3, 0, 0);
    itemBoxSizer2->Add(itemFlexGridSizer3, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

    wxStaticText* itemStaticText4 = new wxStaticText( itemPanel1, wxID_STATIC, _("Molecule"), wxDefaultPosition, wxDefaultSize, 0 );
    itemFlexGridSizer3->Add(itemStaticText4, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxArrayString m_MoleculeCtrlStrings;
    m_MoleculeCtrlStrings.Add(wxEmptyString);
    m_MoleculeCtrlStrings.Add(_("Genomic DNA"));
    m_MoleculeCtrlStrings.Add(_("Genomic RNA"));
    m_MoleculeCtrlStrings.Add(_("Precursor RNA"));
    m_MoleculeCtrlStrings.Add(_("mRNA [cDNA]"));
    m_MoleculeCtrlStrings.Add(_("Ribosomal RNA"));
    m_MoleculeCtrlStrings.Add(_("Transfer RNA"));
    m_MoleculeCtrlStrings.Add(_("Peptide"));
    m_MoleculeCtrlStrings.Add(_("Other-Genetic DNA"));
    m_MoleculeCtrlStrings.Add(_("Other-Genetic RNA"));
    m_MoleculeCtrlStrings.Add(_("Genomic-mRNA"));
    m_MoleculeCtrlStrings.Add(_("cRNA"));
    m_MoleculeCtrlStrings.Add(_("Transcribed RNA"));
    m_MoleculeCtrlStrings.Add(_("Non-coding RNA"));
    m_MoleculeCtrlStrings.Add(_("Other"));
    m_MoleculeCtrl = new wxChoice( itemPanel1, ID_CHOICE6, wxDefaultPosition, wxDefaultSize, m_MoleculeCtrlStrings, 0 );
    itemFlexGridSizer3->Add(m_MoleculeCtrl, 0, wxGROW|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    itemFlexGridSizer3->Add(5, 5, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxStaticText* itemStaticText7 = new wxStaticText( itemPanel1, wxID_STATIC, _("Completedness"), wxDefaultPosition, wxDefaultSize, 0 );
    itemFlexGridSizer3->Add(itemStaticText7, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxArrayString m_CompletednessCtrlStrings;
    m_CompletednessCtrlStrings.Add(wxEmptyString);
    m_CompletednessCtrlStrings.Add(_("Complete"));
    m_CompletednessCtrlStrings.Add(_("Partial"));
    m_CompletednessCtrlStrings.Add(_("No Left"));
    m_CompletednessCtrlStrings.Add(_("No Right"));
    m_CompletednessCtrlStrings.Add(_("No Ends"));
    m_CompletednessCtrlStrings.Add(_("Has Left"));
    m_CompletednessCtrlStrings.Add(_("Has Right"));
    m_CompletednessCtrlStrings.Add(_("Other"));
    m_CompletednessCtrl = new wxChoice( itemPanel1, ID_CHOICE7, wxDefaultPosition, wxDefaultSize, m_CompletednessCtrlStrings, 0 );
    itemFlexGridSizer3->Add(m_CompletednessCtrl, 0, wxGROW|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    itemFlexGridSizer3->Add(5, 5, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxStaticText* itemStaticText10 = new wxStaticText( itemPanel1, wxID_STATIC, _("Technique"), wxDefaultPosition, wxDefaultSize, 0 );
    itemFlexGridSizer3->Add(itemStaticText10, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxArrayString m_TechniqueCtrlStrings;
    CEnumeratedTypeValues::TValues tech_values = objects::CMolInfo::ENUM_METHOD_NAME(ETech)()->GetValues();
    for (CEnumeratedTypeValues::TValues::const_iterator i = tech_values.begin(); i != tech_values.end(); ++i)
    {
        m_TechniqueCtrlStrings.Add(wxString(i->first));
    }
    m_TechOther = tech_values.size() - 1;

    m_TechniqueCtrl = new wxChoice( itemPanel1, ID_CHOICE8, wxDefaultPosition, wxDefaultSize, m_TechniqueCtrlStrings, 0 );
    itemFlexGridSizer3->Add(m_TechniqueCtrl, 0, wxGROW|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    m_TechExpCtrl = new wxTextCtrl( itemPanel1, ID_TEXTCTRL24, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
    itemFlexGridSizer3->Add(m_TechExpCtrl, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxStaticText* itemStaticText13 = new wxStaticText( itemPanel1, wxID_STATIC, _("Topology"), wxDefaultPosition, wxDefaultSize, 0 );
    itemFlexGridSizer3->Add(itemStaticText13, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxArrayString m_TopologyCtrlStrings;
    m_TopologyCtrlStrings.Add(wxEmptyString);
    m_TopologyCtrlStrings.Add(_("Linear"));
    m_TopologyCtrlStrings.Add(_("Circular"));
    m_TopologyCtrlStrings.Add(_("Tandem"));
    m_TopologyCtrlStrings.Add(_("Other"));
    m_TopologyCtrl = new wxChoice( itemPanel1, ID_CHOICE9, wxDefaultPosition, wxDefaultSize, m_TopologyCtrlStrings, 0 );
    itemFlexGridSizer3->Add(m_TopologyCtrl, 0, wxGROW|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    itemFlexGridSizer3->Add(5, 5, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxStaticText* itemStaticText16 = new wxStaticText( itemPanel1, wxID_STATIC, _("Strandedness"), wxDefaultPosition, wxDefaultSize, 0 );
    itemFlexGridSizer3->Add(itemStaticText16, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxArrayString m_StrandednessCtrlStrings;
    m_StrandednessCtrlStrings.Add(wxEmptyString);
    m_StrandednessCtrlStrings.Add(_("Single"));
    m_StrandednessCtrlStrings.Add(_("Double"));
    m_StrandednessCtrlStrings.Add(_("Mixed"));
//    m_StrandednessCtrlStrings.Add(_("Mixed Rev")); 
    m_StrandednessCtrlStrings.Add(_("Other"));
    m_StrandednessCtrl = new wxChoice( itemPanel1, ID_CHOICE10, wxDefaultPosition, wxDefaultSize, m_StrandednessCtrlStrings, 0 );
    itemFlexGridSizer3->Add(m_StrandednessCtrl, 0, wxGROW|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    itemFlexGridSizer3->Add(5, 5, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxALL, 5);

////@end CMolInfoPanel content construction
    if (!m_Bioseq) {
        itemStaticText13->Show(false);
        m_TopologyCtrl->Show(false);
        itemStaticText16->Show(false);
        m_StrandednessCtrl->Show(false);
    }
}


void CMolInfoPanel::x_SetMolecule()
{
    if (m_Molinfo->IsSetBiomol()) {
        switch (m_Molinfo->GetBiomol()) {
            case CMolInfo::eBiomol_genomic:
                if (m_Bioseq && m_Bioseq->IsSetInst() 
                    && m_Bioseq->GetInst().IsSetMol() 
                    && m_Bioseq->GetInst().GetMol() == CSeq_inst::eMol_rna) {
                    m_MoleculeCtrl->SetStringSelection (_("Genomic RNA"));
                } else {
                    m_MoleculeCtrl->SetStringSelection (_("Genomic DNA"));
                }
                break;
            case CMolInfo::eBiomol_pre_RNA:
                m_MoleculeCtrl->SetStringSelection (_("Precursor RNA"));
                break;
            case CMolInfo::eBiomol_mRNA:
                m_MoleculeCtrl->SetStringSelection (_("mRNA [cDNA]"));
                break;
            case CMolInfo::eBiomol_rRNA:
                m_MoleculeCtrl->SetStringSelection (_("Ribosomal RNA"));
                break;
            case CMolInfo::eBiomol_tRNA:
                m_MoleculeCtrl->SetStringSelection (_("Transfer RNA"));
                break;
            case CMolInfo::eBiomol_peptide:
                m_MoleculeCtrl->SetStringSelection (_("Peptide"));
                break;
            case CMolInfo::eBiomol_other_genetic:
                if (m_Bioseq && m_Bioseq->IsSetInst() 
                    && m_Bioseq->GetInst().IsSetMol() 
                    && m_Bioseq->GetInst().GetMol() == CSeq_inst::eMol_rna) {
                    m_MoleculeCtrl->SetStringSelection (_("Other-Genetic RNA"));
                } else {
                    m_MoleculeCtrl->SetStringSelection (_("Other-Genetic DNA"));
                }
                break;
            case CMolInfo::eBiomol_genomic_mRNA:
                m_MoleculeCtrl->SetStringSelection (_("Genomic-mRNA"));
                break;
            case CMolInfo::eBiomol_cRNA:
                m_MoleculeCtrl->SetStringSelection (_("cRNA"));
                break;
            case CMolInfo::eBiomol_transcribed_RNA:
                m_MoleculeCtrl->SetStringSelection (_("Transcribed RNA"));
                break;
            case CMolInfo::eBiomol_ncRNA:
            case CMolInfo::eBiomol_scRNA:
            case CMolInfo::eBiomol_snoRNA:
            case CMolInfo::eBiomol_snRNA:
                m_MoleculeCtrl->SetStringSelection (_("Non-coding RNA"));
                break;
            case CMolInfo::eBiomol_other:
                m_MoleculeCtrl->SetStringSelection (_("Other"));
                break;
            case CMolInfo::eBiomol_unknown:
                m_MoleculeCtrl->SetStringSelection (wxEmptyString);
                break;
        }
    } else {
        m_MoleculeCtrl->SetStringSelection (wxEmptyString);
    }
}


void CMolInfoPanel::x_GetMolecule()
{
    int biomol = m_MoleculeCtrl->GetSelection();
    switch (biomol) {
        case 0:
            m_Molinfo->ResetBiomol();
            break;
        case 1:
            m_Molinfo->SetBiomol(CMolInfo::eBiomol_genomic);
            if (m_Bioseq) {
                m_Bioseq->SetInst().SetMol(CSeq_inst::eMol_dna);
            }
            break;
        case 2:
            m_Molinfo->SetBiomol(CMolInfo::eBiomol_genomic);
            if (m_Bioseq) {
                m_Bioseq->SetInst().SetMol(CSeq_inst::eMol_rna);
            }
            break;
        case 3:
            m_Molinfo->SetBiomol(CMolInfo::eBiomol_pre_RNA);
            if (m_Bioseq) {
                m_Bioseq->SetInst().SetMol(CSeq_inst::eMol_rna);
            }
            break;
        case 4:
            m_Molinfo->SetBiomol(CMolInfo::eBiomol_mRNA);
            if (m_Bioseq) {
                m_Bioseq->SetInst().SetMol(CSeq_inst::eMol_rna);
            }
            break;
        case 5:
            m_Molinfo->SetBiomol(CMolInfo::eBiomol_rRNA);
            if (m_Bioseq) {
                m_Bioseq->SetInst().SetMol(CSeq_inst::eMol_rna);
            }
            break;
        case 6:
            m_Molinfo->SetBiomol(CMolInfo::eBiomol_tRNA);
            if (m_Bioseq) {
                m_Bioseq->SetInst().SetMol(CSeq_inst::eMol_rna);
            }
            break;
        case 7:
            m_Molinfo->SetBiomol(CMolInfo::eBiomol_peptide);
            if (m_Bioseq) {
                m_Bioseq->SetInst().SetMol(CSeq_inst::eMol_aa);
            }
            break;
        case 8:
            m_Molinfo->SetBiomol(CMolInfo::eBiomol_other_genetic);
            if (m_Bioseq) {
                m_Bioseq->SetInst().SetMol(CSeq_inst::eMol_dna);
            }
            break;
        case 9:
            m_Molinfo->SetBiomol(CMolInfo::eBiomol_other_genetic);
            if (m_Bioseq) {
                m_Bioseq->SetInst().SetMol(CSeq_inst::eMol_rna);
            }
            break;
        case 10:
            m_Molinfo->SetBiomol(CMolInfo::eBiomol_genomic_mRNA);
            if (m_Bioseq) {
                m_Bioseq->SetInst().SetMol(CSeq_inst::eMol_rna);
            }
            break;
        case 11:
            m_Molinfo->SetBiomol(CMolInfo::eBiomol_cRNA);
            if (m_Bioseq) {
                m_Bioseq->SetInst().SetMol(CSeq_inst::eMol_rna);
            }
            break;
        case 12:
            m_Molinfo->SetBiomol(CMolInfo::eBiomol_transcribed_RNA);
            if (m_Bioseq) {
                m_Bioseq->SetInst().SetMol(CSeq_inst::eMol_rna);
            }
            break;
        case 13:
            m_Molinfo->SetBiomol(CMolInfo::eBiomol_ncRNA);
            if (m_Bioseq) {
                m_Bioseq->SetInst().SetMol(CSeq_inst::eMol_rna);
            }
            break;
        case 14:
            m_Molinfo->SetBiomol(CMolInfo::eBiomol_other);
            if (m_Bioseq) {
                m_Bioseq->SetInst().SetMol(CSeq_inst::eMol_rna);
            }
            break;
    }
}


const int kCompletenessOther = 8;

void CMolInfoPanel::x_SetCompletedness()
{
    int val = 0;
    if (m_Molinfo->IsSetCompleteness()) {
        val = m_Molinfo->GetCompleteness();
    }
    if (val == CMolInfo::eCompleteness_other) {
        m_CompletednessCtrl->SetSelection(kCompletenessOther);
    } else {
        m_CompletednessCtrl->SetSelection(val);
    }
}


void CMolInfoPanel::x_GetCompletedness()
{
    int val = m_CompletednessCtrl->GetSelection();
    if (val == kCompletenessOther) {
        m_Molinfo->SetCompleteness(CMolInfo::eCompleteness_other);
    } else {
        m_Molinfo->SetCompleteness(val);
    }
}



void CMolInfoPanel::x_SetTechnique()
{
    int val = 0;
    if (m_Molinfo->IsSetTech()) {
        val = m_Molinfo->GetTech();
    }

    if (val == CMolInfo::eTech_other) {
        m_TechniqueCtrl->SetSelection(m_TechOther);
        m_TechExpCtrl->Show(true);
        if (m_Molinfo->IsSetTechexp()) {
            m_TechExpCtrl->SetValue(ToWxString(m_Molinfo->GetTechexp()));
        } else {
            m_TechExpCtrl->SetValue(wxEmptyString);
        }
    } else {
        m_TechniqueCtrl->SetSelection(val);
        m_TechExpCtrl->Show(false);
    }    
}


void CMolInfoPanel::x_GetTechnique()
{
    int val = m_TechniqueCtrl->GetSelection();
    if (val == m_TechOther) {
        m_Molinfo->SetTech(CMolInfo::eTech_other);
        if (m_TechExpCtrl->IsShown())
        {
            string exp =  m_TechExpCtrl->GetValue().ToStdString();
            if (!exp.empty())
                m_Molinfo->SetTechexp(exp);
            else
                m_Molinfo->ResetTechexp();
        }
    } else {
        m_Molinfo->SetTech (val);
    }
}


const int kTopologyOther = 4;


void CMolInfoPanel::x_SetTopology()
{
    if (!m_Bioseq) {
        return;
    }
    int val = 0;
    if (m_Bioseq->IsSetInst() && m_Bioseq->GetInst().IsSetTopology()) {
        val = m_Bioseq->GetInst().GetTopology();
    }
    if (val == CSeq_inst::eTopology_other) {
        m_TopologyCtrl->SetSelection(kTopologyOther);
    } else {
        m_TopologyCtrl->SetSelection(val);
    }    
}


void CMolInfoPanel::x_GetTopology()
{
    if (!m_Bioseq) {
        return;
    }
    int val = m_TopologyCtrl->GetSelection();
    if (val == kTopologyOther) {
        m_Bioseq->SetInst().SetTopology(CSeq_inst::eTopology_other);
    } else {
        CSeq_inst::TTopology topology = (CSeq_inst::TTopology) val;
        m_Bioseq->SetInst().SetTopology (topology);
    }
}


const int kStrandednessOther = 4;


void CMolInfoPanel::x_SetStrandedness()
{
    if (!m_Bioseq) {
        return;
    }
    int val = 0;
    if (m_Bioseq->IsSetInst() && m_Bioseq->GetInst().IsSetStrand()) {
        val = m_Bioseq->GetInst().GetStrand();
    }
    if (val == CSeq_inst::eStrand_other) {
        m_StrandednessCtrl->SetSelection(kStrandednessOther);
    } else {
        m_StrandednessCtrl->SetSelection(val);
    }    
}


void CMolInfoPanel::x_GetStrandedness()
{
    if (!m_Bioseq) {
        return;
    }
    int val = m_StrandednessCtrl->GetSelection();
    if (val == kStrandednessOther) {
        m_Bioseq->SetInst().SetStrand(CSeq_inst::eStrand_other);
    } else {
        CSeq_inst::TStrand strand = (CSeq_inst::TStrand) val;
        m_Bioseq->SetInst().SetStrand (strand);
    }
}


bool CMolInfoPanel::TransferDataToWindow()
{
    if (!wxPanel::TransferDataToWindow())
        return false;

    x_SetMolecule();
    x_SetCompletedness();
    x_SetTechnique();
    x_SetTopology();
    x_SetStrandedness();

    return true;
}


bool CMolInfoPanel::TransferDataFromWindow()
{
    if (!wxPanel::TransferDataFromWindow())
        return false;

    x_GetMolecule();
    x_GetCompletedness();
    x_GetTechnique();
    x_GetTopology();
    x_GetStrandedness();

    return true;
}


CRef<objects::CMolInfo> CMolInfoPanel::GetMolinfo() const
{
    if (m_Molinfo)
        return m_Molinfo;

    return CRef<objects::CMolInfo>();
}


void CMolInfoPanel::SetBioseqValues(CRef<CBioseq> bioseq)
{
    if (!bioseq || !m_Bioseq) {
        return;
    }

    if (!m_Bioseq->IsSetInst() || !m_Bioseq->GetInst().IsSetTopology()) {
        bioseq->SetInst().ResetTopology();
    } else {
        bioseq->SetInst().SetTopology(m_Bioseq->GetInst().GetTopology());
    }
    if (!m_Bioseq->IsSetInst() || !m_Bioseq->GetInst().IsSetStrand()) {
        bioseq->SetInst().ResetStrand();
    } else {
        bioseq->SetInst().SetStrand(m_Bioseq->GetInst().GetStrand());
    }
    if (!m_Bioseq->IsSetInst() || !m_Bioseq->GetInst().IsSetMol()) {
        bioseq->SetInst().ResetMol();
    } else {
        bioseq->SetInst().SetMol(m_Bioseq->GetInst().GetMol());
    }
}


void CMolInfoPanel::ChangeBioseq(CConstRef<objects::CBioseq> bioseq)
{
    m_Bioseq = new objects::CBioseq();
    if (bioseq) {
        m_Bioseq->Assign(*bioseq);
    }
    TransferDataToWindow();
}


/*!
 * Should we show tooltips?
 */

bool CMolInfoPanel::ShowToolTips()
{
    return true;
}

/*!
 * Get bitmap resources
 */

wxBitmap CMolInfoPanel::GetBitmapResource( const wxString& name )
{
    // Bitmap retrieval
////@begin CMolInfoPanel bitmap retrieval
    wxUnusedVar(name);
    return wxNullBitmap;
////@end CMolInfoPanel bitmap retrieval
}

/*!
 * Get icon resources
 */

wxIcon CMolInfoPanel::GetIconResource( const wxString& name )
{
    // Icon retrieval
////@begin CMolInfoPanel icon retrieval
    wxUnusedVar(name);
    return wxNullIcon;
////@end CMolInfoPanel icon retrieval
}


/*!
 * wxEVT_COMMAND_CHOICE_SELECTED event handler for ID_CHOICE8
 */

void CMolInfoPanel::OnChoice8Selected( wxCommandEvent& event )
{
    if (m_TechniqueCtrl->GetSelection() == m_TechOther) {
        m_TechExpCtrl->Show(true);
    } else {
        m_TechExpCtrl->Show(false);
    }
}

void CMolInfoPanel::ChangeSeqdesc(const objects::CSeqdesc& desc)
{
    m_Molinfo.Reset(new CMolInfo());
    m_Molinfo->Assign(desc.GetMolinfo());
    TransferDataToWindow();
}


void CMolInfoPanel::UpdateSeqdesc(objects::CSeqdesc& desc)
{
    TransferDataFromWindow();
    desc.SetMolinfo(*GetMolinfo());
}


END_NCBI_SCOPE

