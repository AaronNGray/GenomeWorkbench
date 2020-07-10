/*  $Id: rnanamepanel.cpp 27547 2013-03-04 15:03:09Z bollin $
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
#include <objmgr/bioseq_ci.hpp>
#include <objects/seqfeat/RNA_ref.hpp>
#include <objects/seqfeat/RNA_gen.hpp>
#include <objects/seqloc/Seq_id.hpp>
#include <gui/objutils/cmd_create_feat.hpp>
#include <gui/widgets/wx/wx_utils.hpp>

////@begin includes
////@end includes

#include <wx/checkbox.h>
#include <gui/packages/pkg_sequence_edit/rnanamepanel.hpp>
#include <gui/packages/pkg_sequence_edit/subprep_util.hpp>

////@begin XPM images
////@end XPM images

BEGIN_NCBI_SCOPE

/*!
 * CRNANamePanel type definition
 */

IMPLEMENT_DYNAMIC_CLASS( CRNANamePanel, CBulkCmdPanel )


/*!
 * CRNANamePanel event table definition
 */

BEGIN_EVENT_TABLE( CRNANamePanel, CBulkCmdPanel )

////@begin CRNANamePanel event table entries
////@end CRNANamePanel event table entries
    EVT_RADIOBUTTON( wxID_ANY, CRNANamePanel::OnRNATypeSelected )
    EVT_CHECKBOX( wxID_ANY, CRNANamePanel::OnRNATypeSelected )

END_EVENT_TABLE()


/*!
 * CRNANamePanel constructors
 */

CRNANamePanel::CRNANamePanel()
{
    Init();
}

CRNANamePanel::CRNANamePanel( wxWindow* parent, objects::CSeq_entry_Handle seh, 
                             CSourceRequirements::EWizardType wizard_type,
                             CSourceRequirements::EWizardSrcType src_type,
                             bool multi,
                             wxWindowID id, const wxPoint& pos, const wxSize& size, long style )
: m_TopSeqEntry(seh),
  m_WizardType (wizard_type),
  m_SrcType (src_type),
  m_IsMulti (multi)
{
    Init();
    Create(parent, id, pos, size, style);
}


/*!
 * CRNANamePanel creator
 */

bool CRNANamePanel::Create( wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style )
{
////@begin CRNANamePanel creation
    CBulkCmdPanel::Create( parent, id, pos, size, style );

    CreateControls();
    if (GetSizer())
    {
        GetSizer()->SetSizeHints(this);
    }
    Centre();
////@end CRNANamePanel creation
    return true;
}


/*!
 * CRNANamePanel destructor
 */

CRNANamePanel::~CRNANamePanel()
{
////@begin CRNANamePanel destruction
////@end CRNANamePanel destruction
}


/*!
 * Member initialisation
 */

void CRNANamePanel::Init()
{
////@begin CRNANamePanel member initialisation
    m_RNANameSizer = NULL;
////@end CRNANamePanel member initialisation
}


static string s_RNABacteriaArchaea[] = {
  "16S ribosomal RNA",
  "16S-23S ribosomal RNA intergenic spacer",
  "23S ribosomal RNA"
};

const int s_NumRNABacteriaArchaea = sizeof (s_RNABacteriaArchaea) / sizeof (string);


static string s_RNAOrganelle[] = {
  "small subunit ribosomal RNA",
  "large subunit ribosomal RNA"
};

const int s_NumRNAOrganelle = sizeof (s_RNAOrganelle) / sizeof (string);


static string s_RNAFungal[] = {
  "18S ribosomal RNA",
  "small subunit ribosomal RNA",
  "internal transcribed spacer 1",
  "5.8S ribosomal RNA",
  "internal transcribed spacer 2",
  "28S ribosomal RNA",
  "26S ribosomal RNA",
  "large subunit ribosomal RNA"
};

const int s_NumRNAFungal = sizeof (s_RNAFungal) / sizeof (string);


/*!
 * Control creation for CRNANamePanel
 */

void CRNANamePanel::CreateControls()
{    
////@begin CRNANamePanel content construction
    CRNANamePanel* itemCBulkCmdPanel1 = this;

    wxBoxSizer* itemBoxSizer2 = new wxBoxSizer(wxVERTICAL);
    itemCBulkCmdPanel1->SetSizer(itemBoxSizer2);

    wxStaticBox* itemStaticBoxSizer3Static = new wxStaticBox(itemCBulkCmdPanel1, wxID_ANY, _("RNA Name"));
    m_RNANameSizer = new wxStaticBoxSizer(itemStaticBoxSizer3Static, wxVERTICAL);
    itemBoxSizer2->Add(m_RNANameSizer, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

////@end CRNANamePanel content construction

    size_t i = 0;
    wxRadioButton* btn;
    wxCheckBox* box;

    switch (m_SrcType) {
        case CSourceRequirements::eWizardSrcType_bacteria_or_archaea:
            for (i = 0; i < s_NumRNABacteriaArchaea; i++) {
                if (m_IsMulti) {
                    box = new wxCheckBox ( itemCBulkCmdPanel1, wxID_ANY, ToWxString (s_RNABacteriaArchaea[i]));
                    m_RNANameSizer->Add (box, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5); 
                } else {
                    btn = new wxRadioButton ( itemCBulkCmdPanel1, wxID_ANY, ToWxString (s_RNABacteriaArchaea[i]));
                    m_RNANameSizer->Add (btn, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5); 
                }
            }
            break;
        case CSourceRequirements::eWizardSrcType_cultured_fungus:
        case CSourceRequirements::eWizardSrcType_uncultured_fungus:
        case CSourceRequirements::eWizardSrcType_vouchered_fungus:
            for (i = 0; i < s_NumRNAFungal; i++) {
                if (m_IsMulti) {
                    box = new wxCheckBox ( itemCBulkCmdPanel1, wxID_ANY, ToWxString (s_RNAFungal[i]));
                    m_RNANameSizer->Add (box, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5); 
                } else {
                    btn = new wxRadioButton ( itemCBulkCmdPanel1, wxID_ANY, ToWxString (s_RNAFungal[i]));
                    m_RNANameSizer->Add (btn, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5); 
                }
            }
            break;
        default:
            for (i = 0; i < s_NumRNABacteriaArchaea; i++) {
                if (m_IsMulti) {
                    box = new wxCheckBox ( itemCBulkCmdPanel1, wxID_ANY, ToWxString (s_RNABacteriaArchaea[i]));
                    m_RNANameSizer->Add (box, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5); 
                } else {
                    btn = new wxRadioButton ( itemCBulkCmdPanel1, wxID_ANY, ToWxString (s_RNABacteriaArchaea[i]));
                    m_RNANameSizer->Add (btn, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5); 
                }
            }
            for (i = 0; i < s_NumRNAFungal; i++) {
                if (m_IsMulti) {
                    box = new wxCheckBox ( itemCBulkCmdPanel1, wxID_ANY, ToWxString (s_RNAFungal[i]));
                    m_RNANameSizer->Add (box, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5); 
                } else {
                    btn = new wxRadioButton ( itemCBulkCmdPanel1, wxID_ANY, ToWxString (s_RNAFungal[i]));
                    m_RNANameSizer->Add (btn, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5); 
                }
            }            
            break;
    }
    bool enable = true;
    if (i > 0) {
        if (m_IsMulti) {
            box = new wxCheckBox ( itemCBulkCmdPanel1, wxID_ANY, _("Something else"));
            m_RNANameSizer->Add (box, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5); 
        } else {
            btn = new wxRadioButton ( itemCBulkCmdPanel1, wxID_ANY, _("Something else"));
            m_RNANameSizer->Add (btn, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);
        }
        enable = false;
    }

    m_OtherLabel = new wxTextCtrl( itemCBulkCmdPanel1, wxID_ANY, wxEmptyString, wxDefaultPosition, wxSize(200, -1), 0 );
    m_RNANameSizer->Add(m_OtherLabel, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxALL, 5);
    m_OtherLabel->Enable(enable);

}


/*!
 * Should we show tooltips?
 */

bool CRNANamePanel::ShowToolTips()
{
    return true;
}

/*!
 * Get bitmap resources
 */

wxBitmap CRNANamePanel::GetBitmapResource( const wxString& name )
{
    // Bitmap retrieval
////@begin CRNANamePanel bitmap retrieval
    wxUnusedVar(name);
    return wxNullBitmap;
////@end CRNANamePanel bitmap retrieval
}

/*!
 * Get icon resources
 */

wxIcon CRNANamePanel::GetIconResource( const wxString& name )
{
    // Icon retrieval
////@begin CRNANamePanel icon retrieval
    wxUnusedVar(name);
    return wxNullIcon;
////@end CRNANamePanel icon retrieval
}


static bool s_IsrRNA (string rna_name)
{
    bool rval = false;

    if (NStr::IsBlank(rna_name)) {
        rval = false;
    } else if ((NStr::StartsWith(rna_name, "large ") || NStr::StartsWith(rna_name, "small "))
               && NStr::Equal(rna_name.substr(5), " subunit ribosomal RNA")) {
        rval = true;
    } else {
        string::iterator cp = rna_name.begin();
        size_t pos = 0;
        if (isdigit (*cp)) {
            cp++;
            pos++;
            while (cp != rna_name.end() && isdigit (*cp)) {
                cp++;
                pos++;
            }
            if (NStr::Equal(rna_name.substr(pos), "S ribosomal RNA")) {
                rval = true;
            }
        }
    }
    return rval;
}


static bool s_IsRNASpacer (string rna_name)
{
    bool rval = false;

    if (NStr::IsBlank(rna_name)) {
        rval = false;
    } else {
        string::iterator cp = rna_name.begin();
        size_t pos = 0;
        if (isdigit (*cp)) {
            cp++;
            pos++;
            while (cp != rna_name.end() && isdigit (*cp)) {
                cp++;
                pos++;
            }
            if (NStr::Equal(rna_name.substr(pos, 2), "S-")) {
                cp++;
                cp++;
                pos +=2;
                if (isdigit (*cp)) {
                    cp++;
                    pos++;
                    while (cp != rna_name.end() && isdigit (*cp)) {
                        cp++;
                        pos++;
                    }
                    if (NStr::Equal(rna_name.substr(pos), "S ribosomal RNA intergenic spacer")) {
                        rval = true;
                    }
                }
            }
        }
    }
    return rval;
}


string CRNANamePanel::x_GetMultiElementName()
{
    string rna_name = "";
    wxSizerItemList& itemList = m_RNANameSizer->GetChildren();
    size_t pos = 0;
    vector<string> labels;
    labels.clear();
    while (pos < itemList.size()) {
        wxCheckBox* c_btn = dynamic_cast<wxCheckBox*>(itemList[pos]->GetWindow());
        if (c_btn) {
            if (c_btn->GetValue()) {
                string label = ToStdString(c_btn->GetLabel());
                if (NStr::Equal(label, "Something else")) {
                    label = ToStdString(m_OtherLabel->GetValue());
                }
                labels.push_back(label);
            }
        }
        pos++;
    }
    if (labels.size() == 1) {
        rna_name = labels[0];
    } else if (labels.size() == 2) {
        rna_name = "contains " + labels[0] + " and " + labels[1];
    } else if (labels.size() > 2) {
        rna_name = "contains ";
        for (size_t i = 0; i < labels.size() - 1; i++) {
            rna_name += labels[i] + ", ";
        }
        rna_name += "and " + labels[labels.size() - 1];
    }
    return rna_name;
}


string CRNANamePanel::x_GetSingleElementName()
{
    string rna_name = "";
    wxSizerItemList& itemList = m_RNANameSizer->GetChildren();
    size_t pos = 0;
    while (pos < itemList.size()) {
        wxRadioButton* r_btn = dynamic_cast<wxRadioButton*>(itemList[pos]->GetWindow());
        if (r_btn) {
            if (r_btn->GetValue()) {
                string label = ToStdString(r_btn->GetLabel());
                if (NStr::Equal(label, "Something else")) {
                    rna_name = ToStdString(m_OtherLabel->GetValue());
                } else {
                    rna_name = label;
                }
            }
        }        
        pos++;
    }
    return rna_name;
}


CRef<CCmdComposite> CRNANamePanel::GetCommand()
{
    string rna_name = "";
    if (m_IsMulti) {
        rna_name = x_GetMultiElementName();
    } else {
        rna_name = x_GetSingleElementName();
    }

    if (NStr::IsBlank(rna_name)) {
        CRef<CCmdComposite> empty;
        return empty;
    }

    bool is_rRNA = s_IsrRNA(rna_name);
    bool is_RNASpacer = s_IsRNASpacer (rna_name);

    CRef<CCmdComposite> cmd (new CCmdComposite("Bulk Add RNA Features"));
    
    objects::CBioseq_CI b_iter(m_TopSeqEntry, objects::CSeq_inst::eMol_na);
    for ( ; b_iter ; ++b_iter ) {
        CRef<objects::CSeq_feat> new_feat(new objects::CSeq_feat());
        if (is_rRNA) {
            new_feat->SetData().SetRna().SetType(objects::CRNA_ref::eType_rRNA);
            new_feat->SetData().SetRna().SetExt().SetName(rna_name);
        } else {
            new_feat->SetData().SetRna().SetType(objects::CRNA_ref::eType_other);
            if (is_RNASpacer) {
                new_feat->SetData().SetRna().SetExt().SetGen().SetProduct(rna_name);
            } else {
                new_feat->SetComment(rna_name);
            }
        }
        if (!AlreadyHasFeature(*b_iter, new_feat->GetData().GetSubtype())) {
            CRef<objects::CSeq_id> new_feat_id(new objects::CSeq_id());
            new_feat_id->Assign(*(b_iter->GetCompleteBioseq()->GetId().front()));
            new_feat->SetLocation().SetInt().SetId(*new_feat_id);
            new_feat->SetLocation().SetInt().SetFrom(0);
            new_feat->SetLocation().SetInt().SetTo(b_iter->GetBioseqLength() - 1);
            new_feat->SetLocation().SetPartialStart(true, objects::eExtreme_Biological);
            new_feat->SetLocation().SetPartialStop(true, objects::eExtreme_Biological);
            objects::CSeq_entry_Handle seh = b_iter->GetSeq_entry_Handle();
            cmd->AddCommand(*CRef<CCmdCreateFeat>(new CCmdCreateFeat(seh, *new_feat)));
        }
    }
    return cmd;
}


string CRNANamePanel::GetErrorMessage()
{
    return "You must specify the name for the RNA feature to be created.";
}


void CRNANamePanel::OnRNATypeSelected( wxCommandEvent& event )
{
    bool enable = false;

    wxSizerItemList& itemList = m_RNANameSizer->GetChildren();
    size_t pos = 0;
    while (pos < itemList.size() && !enable) {
        wxRadioButton* r_btn = dynamic_cast<wxRadioButton*>(itemList[pos]->GetWindow());
        if (r_btn) {
            if (r_btn->GetValue() && NStr::Equal(ToStdString(r_btn->GetLabel()), "Something else")) {
                enable = true;
            }
        } else {
            wxCheckBox* c_btn = dynamic_cast<wxCheckBox*>(itemList[pos]->GetWindow());
            if (c_btn) {
                if (c_btn->GetValue() && NStr::Equal(ToStdString(c_btn->GetLabel()), "Something else")) {
                    enable = true;
                }
            }
        }
        pos++;
    }
    m_OtherLabel->Enable(enable);
}


END_NCBI_SCOPE
