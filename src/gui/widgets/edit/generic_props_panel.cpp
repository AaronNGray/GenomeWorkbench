/*  $Id: generic_props_panel.cpp 42778 2019-04-11 16:12:41Z filippov $
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
#include <gui/widgets/wx/wx_utils.hpp>
#include <gui/objutils/utils.hpp>
#include <objects/seqfeat/Seq_feat.hpp>
#include <objects/seqfeat/Gb_qual.hpp>
#include <objmgr/util/seq_loc_util.hpp>
#include <objmgr/util/sequence.hpp>
#include <objtools/cleanup/cleanup.hpp>

////@begin includes
////@end includes

#include <gui/widgets/edit/serial_member_primitive_validators.hpp>
#include <gui/objutils/reg_settings.hpp>

#include "generic_props_panel.hpp"
#include "gene_panel.hpp"
#include "cds_translation_panel.hpp"

#include <wx/sizer.h>
#include <wx/stattext.h>
#include <wx/textctrl.h>
#include <wx/choice.h>
#include <wx/bitmap.h>
#include <wx/icon.h>
#include <wx/treebook.h>
#include <wx/msgdlg.h> 

BEGIN_NCBI_SCOPE
USING_SCOPE(objects);

////@begin XPM images
////@end XPM images


/*!
 * CGenericPropsPanel type definition
 */

IMPLEMENT_DYNAMIC_CLASS( CGenericPropsPanel, wxPanel )


/*!
 * CGenericPropsPanel event table definition
 */

BEGIN_EVENT_TABLE( CGenericPropsPanel, wxPanel )

////@begin CGenericPropsPanel event table entries
    EVT_COMBOBOX( ID_EXPLANATION, CGenericPropsPanel::OnExplanationSelected )
    EVT_TEXT( ID_EXPLANATION, CGenericPropsPanel::OnExplanationUpdated )
    EVT_CHOICE(ID_PSEUDOGENE_CHOICE, CGenericPropsPanel::OnPseudoChanged)
////@end CGenericPropsPanel event table entries

END_EVENT_TABLE()


/*!
 * CGenericPropsPanel constructors
 */

 CGenericPropsPanel::CGenericPropsPanel() : m_Object(0)
{
    Init();
}

CGenericPropsPanel::CGenericPropsPanel( wxWindow* parent, CObject& object, CScope& scope,
                                        wxWindowID id, const wxPoint& pos, const wxSize& size, long style ) :
    m_Object(0), m_Scope(&scope)
{
    m_Object = dynamic_cast<CSeq_feat*>(&object);
    Init();
    Create(parent, id, pos, size, style);
}


/*!
 * CGenePropsPanel creator
 */

bool CGenericPropsPanel::Create( wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style )
{
////@begin CGenericPropsPanel creation
    wxPanel::Create( parent, id, pos, size, style );

    CreateControls();
    if (GetSizer())
    {
        GetSizer()->SetSizeHints(this);
    }
    Centre();
////@end CGenericPropsPanel creation
    return true;
}


/*!
 * CGenericPropsPanel destructor
 */

CGenericPropsPanel::~CGenericPropsPanel()
{
////@begin CGenericPropsPanel destruction
////@end CGenericPropsPanel destruction
}


/*!
 * Member initialisation
 */

void CGenericPropsPanel::Init()
{
////@begin CGenericPropsPanel member initialisation
    m_PseudogeneChoice = NULL;
    m_Explanation = NULL;
    m_Evidence = NULL;
    m_Partial = NULL;
////@end CGenericPropsPanel member initialisation
}


static const string kPseudogene = "pseudogene";

/*!
 * Control creation for CGenePropsPanel
 */

void CGenericPropsPanel::CreateControls()
{
////@begin CGenericPropsPanel content construction
    CGenericPropsPanel* itemPanel1 = this;

    wxBoxSizer* itemBoxSizer2 = new wxBoxSizer(wxVERTICAL);
    itemPanel1->SetSizer(itemBoxSizer2);

    wxBoxSizer* itemBoxSizer3 = new wxBoxSizer(wxHORIZONTAL);
    itemBoxSizer2->Add(itemBoxSizer3, 0, wxALIGN_LEFT|wxALL, 5);

    wxStaticText* itemStaticText4 = new wxStaticText( itemPanel1, wxID_STATIC, _("Pseudogene"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer3->Add(itemStaticText4, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxArrayString m_PseudogeneChoiceStrings;
    m_PseudogeneChoice = new wxChoice( itemPanel1, ID_PSEUDOGENE_CHOICE, wxDefaultPosition, wxDefaultSize, m_PseudogeneChoiceStrings, 0 );
    itemBoxSizer3->Add(m_PseudogeneChoice, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxBoxSizer* itemBoxSizer6 = new wxBoxSizer(wxHORIZONTAL);
    itemBoxSizer2->Add(itemBoxSizer6, 0, wxALIGN_LEFT|wxALL, 0);

    wxStaticText* itemStaticText7 = new wxStaticText( itemPanel1, wxID_STATIC, _("Exception"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer6->Add(itemStaticText7, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxArrayString m_ExplanationStrings;
    m_Explanation = new wxComboBox( itemPanel1, ID_EXPLANATION, wxEmptyString, wxDefaultPosition, wxDefaultSize, m_ExplanationStrings, wxCB_DROPDOWN );
    itemBoxSizer6->Add(m_Explanation, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxBoxSizer* itemBoxSizer9 = new wxBoxSizer(wxHORIZONTAL);
    itemBoxSizer2->Add(itemBoxSizer9, 0, wxALIGN_LEFT|wxALL, 0);

    wxArrayString m_EvidenceStrings;
    m_EvidenceStrings.Add(_("N/A"));
    m_EvidenceStrings.Add(_("Experimental"));
    m_EvidenceStrings.Add(_("Non-Experimental"));
    m_Evidence = new wxRadioBox( itemPanel1, ID_RADIOBOX3, _("Evidence"), wxDefaultPosition, wxDefaultSize, m_EvidenceStrings, 1, wxRA_SPECIFY_ROWS );
    m_Evidence->SetSelection(0);
    if (CGenericPropsPanel::ShowToolTips())
        m_Evidence->SetToolTip(_("The use of the Evidence field is discouraged - if you wish to list experimental evidence, please use the Experiments tab.  If you wish to describe how similar sequences, alignments, or gene prediction software were used to infer the presence of the feature, please use the Inferences tab."));
    itemBoxSizer9->Add(m_Evidence, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    m_Partial = new wxCheckBox( itemPanel1, ID_RADIOBOX2, _("Partial"), wxDefaultPosition, wxDefaultSize, 0 );
    m_Partial->SetValue(false);
    m_Partial->Enable(false);
    itemBoxSizer2->Add(m_Partial, 0, wxALIGN_LEFT|wxALL, 5);

    // Set validators
    m_Evidence->SetValidator( CSerialEnumValidator(*m_Object, "exp-ev") );
////@end CGenericPropsPanel content construction

    m_ExplanationStrings.clear();
    vector<string> explanation_strings = CSeq_feat::GetListOfLegalExceptions(x_IsRefSeq());
    for (vector<string>::const_iterator s = explanation_strings.begin(); s != explanation_strings.end(); ++s)
        m_ExplanationStrings.Add(wxString(*s));
    m_Explanation->Append(m_ExplanationStrings);

    CSeq_feat& feat = dynamic_cast<CSeq_feat&>(*m_Object);
    if (feat.IsSetPartial() && feat.GetPartial()) {
        m_Partial->SetValue (true);
        unsigned int partial_loc  = sequence::SeqLocPartialCheck(feat.GetLocation(), m_Scope );
        if (!( partial_loc & sequence::eSeqlocPartial_Start )
            && !( partial_loc & sequence::eSeqlocPartial_Stop )
            && !( partial_loc & sequence::eSeqlocPartial_Internal )
            && !( partial_loc & sequence::eSeqlocPartial_Other )
            && !( partial_loc & sequence::eSeqlocPartial_Nostart )
            && !( partial_loc & sequence::eSeqlocPartial_Nostop )
            && !( partial_loc & sequence::eSeqlocPartial_Nointernal)) {
            m_Partial->Enable(true);
        }
    }

    string pseudo_choice = "";
    if (sequence::IsPseudo(feat, *m_Scope)) {
        pseudo_choice = "Unqualified";
    }

    if (feat.IsSetQual()) {
        ITERATE(CSeq_feat::TQual, it, feat.GetQual()) {
            if ((*it)->IsSetQual() 
                && NStr::EqualNocase((*it)->GetQual(), kPseudogene)
                && (*it)->IsSetVal()) {
                pseudo_choice = (*it)->GetVal();
            }
        }
    }
    // add blank, to allow user to remove pseudogene value
    m_PseudogeneChoice->Append(wxT(""));
    auto pseudogene_values = CGb_qual::GetSetOfLegalPseudogenes();
    for (auto it : pseudogene_values) {
        m_PseudogeneChoice->Append(ToWxString(it));
    }
    // add unqualified, to allow the gene.pseudo = true option
    m_PseudogeneChoice->Append(wxT("unqualified"));

    if (NStr::IsBlank(pseudo_choice)) {
        if (!(x_IsGene() || x_IsCds())) {
            m_PseudogeneChoice->Enable(false);
        }
    } else {
        if (!m_PseudogeneChoice->SetStringSelection(ToWxString(pseudo_choice))) {
            bool found = false;
            for (auto it : pseudogene_values) {
                if (NStr::EqualNocase(it, pseudo_choice)) {
                    m_PseudogeneChoice->SetStringSelection(ToWxString(it));
                    found = true;
                    break;
                }
            }
            if (!found) {
                m_PseudogeneChoice->Append(ToWxString(pseudo_choice));
                m_PseudogeneChoice->SetStringSelection(ToWxString(pseudo_choice));
            }
        }
    }

    m_OriginalPseudoSelection = m_PseudogeneChoice->GetStringSelection();

    if (feat.IsSetExcept_text()) {
        m_Explanation->SetValue (ToWxString(feat.GetExcept_text()));
    }
    if (!feat.IsSetExp_ev()) {
        m_Evidence->Enable(false);
    }
}

bool CGenericPropsPanel::PseudoSelectionChanged() const
{
    return (m_OriginalPseudoSelection != m_PseudogeneChoice->GetStringSelection() && !x_IsGene()); 
}

bool CGenericPropsPanel::TransferDataFromWindow()
{
    if (!wxPanel::TransferDataFromWindow()) {
        return false;
    }
    CSeq_feat& feat = dynamic_cast<CSeq_feat&>(*m_Object);
    if (m_Partial->IsEnabled() && m_Partial->GetValue()) {
        feat.SetPartial(true);
    } else {
        feat.ResetPartial();
    }

   
    ModifyPseudo(feat, GetPseudoChoice());

    string exception_text = ToStdString(m_Explanation->GetValue());
    if (NStr::IsBlank(exception_text)) {
        feat.ResetExcept();
        feat.ResetExcept_text();
    } else {
        feat.SetExcept(true);
        feat.SetExcept_text(exception_text);
    }
    return true;
}

string CGenericPropsPanel::GetPseudoChoice()
{
    string pseudo_choice = ToStdString(m_PseudogeneChoice->GetStringSelection());
    NStr::ToLower(pseudo_choice);
    return pseudo_choice;
}

void CGenericPropsPanel::ModifyPseudo(CSeq_feat &feat, const string &pseudo_choice)
{
    if (feat.IsSetQual()) {
        CSeq_feat::TQual::iterator it = feat.SetQual().begin();
        while (it != feat.SetQual().end()) {
            if ((*it)->IsSetQual() && NStr::EqualNocase((*it)->GetQual(), kPseudogene)) {
                it = feat.SetQual().erase(it);
            } else {
                ++it;
            }
        }
        if (feat.SetQual().empty()) {
            feat.ResetQual();
        }
    } 
    if (!NStr::IsBlank(pseudo_choice)) {
        if (pseudo_choice != "unqualified")
        {
            CRef<CGb_qual> qual(new CGb_qual(kPseudogene, pseudo_choice));
            feat.SetQual().push_back(qual);
        }
        feat.SetPseudo(true);
    } else {
        feat.ResetPseudo();
    }
}

bool CGenericPropsPanel::IsPseudo()
{
    string pseudo_choice = ToStdString(m_PseudogeneChoice->GetStringSelection());
    NStr::ToLower(pseudo_choice);    
    return !NStr::IsBlank(pseudo_choice);
}

void CGenericPropsPanel::ResetPseudo()
{
    if (m_PseudogeneChoice && m_PseudogeneChoice->IsEnabled())
    {
        m_PseudogeneChoice->SetStringSelection(wxEmptyString);
        x_UpdateDisableOnRetranslate();
    }
}

/*!
 * Should we show tooltips?
 */

bool CGenericPropsPanel::ShowToolTips()
{
    return true;
}

/*!
 * Get bitmap resources
 */

wxBitmap CGenericPropsPanel::GetBitmapResource( const wxString& name )
{
    // Bitmap retrieval
////@begin CGenericPropsPanel bitmap retrieval
    wxUnusedVar(name);
    return wxNullBitmap;
////@end CGenericPropsPanel bitmap retrieval
}

/*!
 * Get icon resources
 */

wxIcon CGenericPropsPanel::GetIconResource( const wxString& name )
{
    // Icon retrieval
////@begin CGenericPropsPanel icon retrieval
    wxUnusedVar(name);
    return wxNullIcon;
////@end CGenericPropsPanel icon retrieval
}


bool CGenericPropsPanel::x_IsGene() const
{
    bool rval = false;
    wxWindow* parent = this->GetParent();
    wxTreebook* tbook = dynamic_cast<wxTreebook*>(parent);
    if (tbook) {
        CGenePanel* genePanel = 0;
        for (size_t i = 0; i < tbook->GetPageCount() && !genePanel; ++i) {
            genePanel = dynamic_cast<CGenePanel*>(tbook->GetPage(i));            
        }
        if (genePanel) {
            rval = true;
        }
    }
    return rval;
}

bool CGenericPropsPanel::x_IsCds() const
{
    CSeq_feat& feat = dynamic_cast<CSeq_feat&>(*m_Object);
    if (feat.IsSetData() && feat.GetData().IsCdregion())
        return true;
    return false;  
}

bool CGenericPropsPanel::x_IsRefSeq()
{
    bool refseq = false;
    CSeq_feat& feat = dynamic_cast<CSeq_feat&>(*m_Object);
    CBioseq_Handle bsh = GetBioseqForSeqFeat(feat, *m_Scope);
    if (bsh && bsh.IsSetId())
    {
        for (CBioseq_Handle::TId::const_iterator idh = bsh.GetId().begin(); idh != bsh.GetId().end(); ++idh)
        {
            CConstRef<CSeq_id> id = idh->GetSeqId();
            if (id->IsOther())
            {
                refseq = true;
                break;
            }
        }
    }
    return refseq;
}

/*!
 * wxEVT_COMMAND_COMBOBOX_SELECTED event handler for ID_EXPLANATION
 */

wxTreebook* CGenericPropsPanel::x_GetTree()
{
    wxWindow* w = this->GetParent();
    while (w != NULL) {
        wxTreebook* parent = dynamic_cast<wxTreebook*>(w);
        if (parent) {
            return parent;
        } else {
            w = w->GetParent();
        }
    }
    return NULL;
}


void CGenericPropsPanel::OnExplanationSelected( wxCommandEvent& event )
{
    x_UpdateDisableOnRetranslate();
}

void CGenericPropsPanel::x_UpdateDisableOnRetranslate()
{
    // need to enable/disable RetranslateOnOk based on selection
    wxTreebook* parent = x_GetTree();

    CCDSTranslationPanel* translation_panel = 0;
    for (size_t i = 0; i < parent->GetPageCount(); ++i) {
        translation_panel = dynamic_cast<CCDSTranslationPanel*>(parent->GetPage(i));
        if (translation_panel)
            break;
    }
    if (translation_panel) {
        bool valid_exception = CCDSTranslationPanel::IsValidException(ToStdString(m_Explanation->GetValue()));
        bool is_pseudo = IsPseudo();
        translation_panel->DisableRetranslateOnOk(valid_exception, is_pseudo);
        translation_panel->UpdateForceNear();
    }
}


/*!
 * wxEVT_COMMAND_TEXT_UPDATED event handler for ID_EXPLANATION
 */

void CGenericPropsPanel::OnExplanationUpdated( wxCommandEvent& event )
{
    x_UpdateDisableOnRetranslate();
}

void CGenericPropsPanel::OnPseudoChanged( wxCommandEvent& event )
{
    x_UpdateDisableOnRetranslate();
}

END_NCBI_SCOPE
