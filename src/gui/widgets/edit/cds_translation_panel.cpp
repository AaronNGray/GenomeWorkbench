/*  $Id: cds_translation_panel.cpp 44382 2019-12-09 15:53:43Z filippov $
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
#include <objects/general/Dbtag.hpp>
#include <objtools/edit/cds_fix.hpp>
#include <gui/widgets/wx/wx_utils.hpp>
#include <gui/objutils/cmd_add_seqentry.hpp>
#include <objects/seqloc/Seq_id.hpp>
#include <objects/seq/Seq_inst.hpp>
#include <objects/seq/Seq_data.hpp>
#include <objects/seqloc/Seq_loc.hpp>
#include <objects/seqfeat/Genetic_code.hpp>
#include <objects/seqfeat/Genetic_code_table.hpp>
#include <objmgr/bioseq_ci.hpp>
#include <objmgr/scope.hpp>
#include <objmgr/seq_vector.hpp>
#include <objmgr/seq_vector_ci.hpp>
#include <objmgr/util/sequence.hpp>
#include <objtools/cleanup/cleanup.hpp>
#include <algo/align/prosplign/prosplign.hpp>
#include <gui/objutils/cmd_composite.hpp>
#include <gui/objutils/cmd_change_seq_feat.hpp>

#include <gui/widgets/loaders/open_objects_dlg.hpp>
#include <gui/widgets/loaders/file_load_wizard.hpp>
#include <gui/widgets/wx/message_box.hpp>

#include <gui/utils/object_loader.hpp>
#include <gui/utils/extension.hpp>
#include <gui/utils/extension_impl.hpp>
#include <gui/utils/execute_unit.hpp>
#include <gui/widgets/wx/async_call.hpp>

#include "generic_props_panel.hpp"

////@begin includes
////@end includes

#include <wx/treebook.h>
#include <wx/panel.h>
#include <wx/sizer.h>
#include <wx/choice.h>
#include <wx/textctrl.h>
#include <wx/stattext.h>
#include <wx/hyperlink.h>
#include <wx/checkbox.h>
#include <wx/msgdlg.h>


#include "cds_translation_panel.hpp"
#include "location_panel.hpp"
#include "cds_exception_panel.hpp"

////@begin XPM images
////@end XPM images

BEGIN_NCBI_SCOPE
USING_SCOPE(objects);


static const char* const s_ValidExceptions[] = {
    "annotated by transcript or proteomic data",
    "rearrangement required for product",
    "reasons given in citation",
    "RNA editing"
};

typedef CStaticArraySet<const char*, PNocase_CStr> TLegalExceptions;
DEFINE_STATIC_ARRAY_MAP(TLegalExceptions, sc_LegalExceptions, s_ValidExceptions);

bool CCDSTranslationPanel::IsValidException(const string& text)
{
    vector<string> exceptions;
    NStr::Split(text, ",", exceptions, 0);
    NON_CONST_ITERATE(vector<string>, it, exceptions) {
        NStr::TruncateSpacesInPlace(*it);
        if (sc_LegalExceptions.find((*it).c_str()) != sc_LegalExceptions.end()) {
            return true;
        }
    }
    return false;
}

/*!
 * CCDSTranslationPanel type definition
 */

IMPLEMENT_DYNAMIC_CLASS( CCDSTranslationPanel, wxPanel )


/*!
 * CCDSTranslationPanel event table definition
 */

BEGIN_EVENT_TABLE( CCDSTranslationPanel, wxPanel )

////@begin CCDSTranslationPanel event table entries
    EVT_HYPERLINK( ID_PREDICT_INTERVAL, CCDSTranslationPanel::OnPredictIntervalHyperlinkClicked )

    EVT_HYPERLINK( ID_RETRANSLATE_LINK, CCDSTranslationPanel::OnRetranslateLinkHyperlinkClicked )

    EVT_HYPERLINK( ID_HYPERLINKCTRL2, CCDSTranslationPanel::AdjustForStopCodonHyperlinkClicked )

    EVT_HYPERLINK( ID_IMPORT_CDS_PROTEIN, CCDSTranslationPanel::OnImportCdsProteinHyperlinkClicked )

    EVT_COMBOBOX(ID_COMBOBOX, CCDSTranslationPanel::OnProductChanged)
    EVT_TEXT(ID_COMBOBOX, CCDSTranslationPanel::OnProductChanged)

    EVT_CHECKBOX(ID_RETRANSLATE_ON_OK_BTN, CCDSTranslationPanel::OnRetranslateOnOkChanged)
////@end CCDSTranslationPanel event table entries

END_EVENT_TABLE()


/*!
 * CCDSTranslationPanel constructors
 */

CCDSTranslationPanel::CCDSTranslationPanel()
{
    Init();
}

CCDSTranslationPanel::CCDSTranslationPanel( wxWindow* parent, CSeq_feat& feat, CScope& scope, wxWindowID id, 
                                           const wxString& caption, const wxPoint& pos, const wxSize& size, long style ) :
    m_EditedFeat(new CSeq_feat()), m_Scope(&scope), m_EditedBioseq(0), m_add_stop_codon(false)
{
    Init();
    m_EditedFeat->Assign (feat);
    if (feat.IsSetProduct()) {
        const CSeq_id *id = feat.GetProduct().GetId();
        if (id) {
            CBioseq_Handle bsh = m_Scope->GetBioseqHandle(*id);
            if (bsh && bsh.IsProtein()) {
                m_EditedBioseq = new CBioseq();
                m_EditedBioseq->Assign(*(bsh.GetCompleteBioseq()));
            }
        }
    }
    Create(parent, id, caption, pos, size, style);
    SetRegistryPath("Dialogs.Edit.CDSTranslationPanel");
    LoadSettings();
}

static const char* kUpdateGeneSpan = "UpdateGeneSpan";

void CCDSTranslationPanel::SetRegistryPath(const string& reg_path)
{
    m_RegPath = reg_path;
}

void CCDSTranslationPanel::SaveSettings() const
{
    if (!m_RegPath.empty() && m_UpdateGeneSpan) 
    {
        CGuiRegistry& gui_reg = CGuiRegistry::GetInstance();
        CRegistryWriteView view = gui_reg.GetWriteView(m_RegPath);
        view.Set(kUpdateGeneSpan, m_UpdateGeneSpan->GetValue());
    }
}


void CCDSTranslationPanel::LoadSettings()
{
    if (!m_RegPath.empty() && m_UpdateGeneSpan) 
    {
        CGuiRegistry& gui_reg = CGuiRegistry::GetInstance();
        CRegistryReadView view = gui_reg.GetReadView(m_RegPath);
        m_UpdateGeneSpan->SetValue(view.GetBool(kUpdateGeneSpan, true));
    }
}


/*!
 * CCDSTranslationPanel creator
 */

bool CCDSTranslationPanel::Create( wxWindow* parent, wxWindowID id, const wxString& caption, const wxPoint& pos, const wxSize& size, long style )
{
////@begin CCDSTranslationPanel creation
    SetExtraStyle(wxWS_EX_BLOCK_EVENTS);
    wxPanel::Create( parent, id, pos, size, style );

    CreateControls();
    bool is_empty = m_ProductCtrl->IsListEmpty();
    if (is_empty)
        m_ProductCtrl->Append(_("ABCDE"));
    if (GetSizer())
    {
        GetSizer()->SetSizeHints(this);
    }
    Centre();
    if (is_empty)
        m_ProductCtrl->Clear();
////@end CCDSTranslationPanel creation
    return true;
}


/*!
 * CCDSTranslationPanel destructor
 */

CCDSTranslationPanel::~CCDSTranslationPanel()
{
////@begin CCDSTranslationPanel destruction
    SaveSettings();
////@end CCDSTranslationPanel destruction
}


/*!
 * Member initialisation
 */

void CCDSTranslationPanel::Init()
{
////@begin CCDSTranslationPanel member initialisation
    m_GeneticCode = NULL;
    m_Frame = NULL;
    m_ConflictCtrl = NULL;
    m_ProteinLengthTxt = NULL;
    m_ProductCtrl = NULL;
    m_ForceNear = NULL;
    m_TranslationCtrl = NULL;
    m_RetranslateLink = NULL;
    m_RetranslateOnOkBtn = NULL;
    m_UpdatemRNASpan = NULL;
    m_UpdateGeneSpan = NULL;
////@end CCDSTranslationPanel member initialisation
}


/*!
 * Control creation for CCDSTranslationPanel
 */

void CCDSTranslationPanel::CreateControls()
{    
////@begin CCDSTranslationPanel content construction
    CCDSTranslationPanel* itemPanel1 = this;

    wxBoxSizer* itemBoxSizer2 = new wxBoxSizer(wxVERTICAL);
    itemPanel1->SetSizer(itemBoxSizer2);

    wxFlexGridSizer* itemFlexGridSizer3 = new wxFlexGridSizer(0, 2, 0, 0);
    itemBoxSizer2->Add(itemFlexGridSizer3, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

    wxStaticText* itemStaticText4 = new wxStaticText( itemPanel1, wxID_STATIC, _("Genetic Code"), wxDefaultPosition, wxDefaultSize, 0 );
    itemFlexGridSizer3->Add(itemStaticText4, 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxArrayString m_GeneticCodeStrings;
    m_GeneticCode = new wxChoice( itemPanel1, ID_CHOICE1, wxDefaultPosition, wxDefaultSize, m_GeneticCodeStrings, 0 );
    if (CCDSTranslationPanel::ShowToolTips())
        m_GeneticCode->SetToolTip(_("Choose the genetic code that should be used to translate the nucleotide sequence.  For more information, and for the translation tables themselves, see the NCBI Taxonomy page.  If the genetic code is already populated from the taxonomy database, do not change this selection."));
    itemFlexGridSizer3->Add(m_GeneticCode, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxStaticText* itemStaticText6 = new wxStaticText( itemPanel1, wxID_STATIC, _("Reading Frame"), wxDefaultPosition, wxDefaultSize, 0 );
    itemFlexGridSizer3->Add(itemStaticText6, 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxArrayString m_FrameStrings;
    m_FrameStrings.Add(_("One"));
    m_FrameStrings.Add(_("Two"));
    m_FrameStrings.Add(_("Three"));
    m_Frame = new wxChoice( itemPanel1, ID_CHOICE2, wxDefaultPosition, wxDefaultSize, m_FrameStrings, 0 );
    if (CCDSTranslationPanel::ShowToolTips())
        m_Frame->SetToolTip(_("Choose the reading frame in which to translate the sequence."));
    itemFlexGridSizer3->Add(m_Frame, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxStaticText* itemStaticText8 = new wxStaticText( itemPanel1, wxID_STATIC, _("Conflict"), wxDefaultPosition, wxDefaultSize, 0 );
    itemFlexGridSizer3->Add(itemStaticText8, 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    m_ConflictCtrl = new wxCheckBox( itemPanel1, ID_CHECKBOX, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
    m_ConflictCtrl->SetValue(false);
    itemFlexGridSizer3->Add(m_ConflictCtrl, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxStaticText* itemStaticText10 = new wxStaticText( itemPanel1, wxID_STATIC, _("Protein Length"), wxDefaultPosition, wxDefaultSize, 0 );
    itemFlexGridSizer3->Add(itemStaticText10, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    m_ProteinLengthTxt = new wxStaticText( itemPanel1, wxID_STATIC, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
    itemFlexGridSizer3->Add(m_ProteinLengthTxt, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxStaticLine* itemStaticLine12 = new wxStaticLine( itemPanel1, wxID_STATIC, wxDefaultPosition, wxDefaultSize, wxLI_HORIZONTAL );
    itemBoxSizer2->Add(itemStaticLine12, 0, wxGROW|wxALL, 5);

    wxBoxSizer* itemBoxSizer13 = new wxBoxSizer(wxHORIZONTAL);
    itemBoxSizer2->Add(itemBoxSizer13, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

    wxStaticText* itemStaticText14 = new wxStaticText( itemPanel1, wxID_STATIC, _("Protein Product"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer13->Add(itemStaticText14, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxArrayString m_ProductCtrlStrings;
    CScope::TTSE_Handles tses;
    m_Scope->GetAllTSEs(tses, CScope::eAllTSEs);
    ITERATE (CScope::TTSE_Handles, handle, tses) {
        for (CBioseq_CI bioseq_it(*handle, CSeq_inst::eMol_aa);  bioseq_it;  ++bioseq_it) {
            string label;
            CConstRef<CSeq_id> seqid = sequence::GetId(*bioseq_it, sequence::eGetId_Best).GetSeqId();
            seqid->GetLabel(&label, CSeq_id::eFasta);
            m_ProductCtrlStrings.Add(ToWxString(label));          
            m_SeqIds[label] = seqid;
        }
    }
    m_ProductCtrl = new wxOwnerDrawnComboBox( itemPanel1, ID_COMBOBOX, wxEmptyString, wxDefaultPosition, wxDefaultSize, m_ProductCtrlStrings, wxCB_SIMPLE|wxTE_PROCESS_ENTER );
    itemBoxSizer13->Add(m_ProductCtrl, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    m_ForceNear = new wxCheckBox( itemPanel1, ID_FORCE_NEAR_BTN, _("Force Near, Create Product"), wxDefaultPosition, wxDefaultSize, 0 );
    m_ForceNear->SetValue(false);
    itemBoxSizer13->Add(m_ForceNear, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    m_TranslationCtrl = new wxTextCtrl( itemPanel1, ID_TEXTCTRL27, wxEmptyString, wxDefaultPosition, wxSize(400, 100), wxTE_MULTILINE|wxTE_READONLY );
    m_TranslationCtrl->SetBackgroundColour(wxColour(255, 255, 255));
    m_TranslationCtrl->Enable(false);
    itemBoxSizer2->Add(m_TranslationCtrl, 0, wxGROW|wxALL, 5);

    wxBoxSizer* itemBoxSizer18 = new wxBoxSizer(wxHORIZONTAL);
    itemBoxSizer2->Add(itemBoxSizer18, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

    wxHyperlinkCtrl* itemHyperlinkCtrl19 = new wxHyperlinkCtrl( itemPanel1, ID_PREDICT_INTERVAL, _("Predict Interval"), wxEmptyString, wxDefaultPosition, wxDefaultSize, wxHL_DEFAULT_STYLE );
    itemBoxSizer18->Add(itemHyperlinkCtrl19, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    m_RetranslateLink = new wxHyperlinkCtrl( itemPanel1, ID_RETRANSLATE_LINK, _("Retranslate"), wxEmptyString, wxDefaultPosition, wxDefaultSize, wxHL_DEFAULT_STYLE );
    m_RetranslateLink->SetNormalColour(wxColour(0, 0, 255));
    m_RetranslateLink->SetVisitedColour(wxColour(0, 0, 255));
    itemBoxSizer18->Add(m_RetranslateLink, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxHyperlinkCtrl* itemHyperlinkCtrl21 = new wxHyperlinkCtrl( itemPanel1, ID_HYPERLINKCTRL2, _("Adjust for Stop Codon"), wxEmptyString, wxDefaultPosition, wxDefaultSize, wxHL_DEFAULT_STYLE );
    itemHyperlinkCtrl21->SetNormalColour(wxColour(0, 0, 255));
    itemHyperlinkCtrl21->SetVisitedColour(wxColour(0, 0, 255));
    itemBoxSizer18->Add(itemHyperlinkCtrl21, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxHyperlinkCtrl* itemHyperlinkCtrl22 = new wxHyperlinkCtrl( itemPanel1, ID_IMPORT_CDS_PROTEIN, _("Import Protein Sequence"), wxEmptyString, wxDefaultPosition, wxDefaultSize, wxHL_DEFAULT_STYLE );
    itemBoxSizer18->Add(itemHyperlinkCtrl22, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxBoxSizer* itemBoxSizer23 = new wxBoxSizer(wxHORIZONTAL);
    itemBoxSizer2->Add(itemBoxSizer23, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

    m_RetranslateOnOkBtn = new wxCheckBox( itemPanel1, ID_RETRANSLATE_ON_OK_BTN, _("Retranslate on OK"), wxDefaultPosition, wxDefaultSize, 0 );
    m_RetranslateOnOkBtn->SetValue(true);
    DisableRetranslateOnOk((m_EditedFeat->IsSetExcept_text() && IsValidException(m_EditedFeat->GetExcept_text())), x_IsPseudo());
    itemBoxSizer23->Add(m_RetranslateOnOkBtn, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    m_UpdatemRNASpan = new wxCheckBox( itemPanel1, ID_CHECKBOX3, _("Update mRNA span"), wxDefaultPosition, wxDefaultSize, 0 );
    m_UpdatemRNASpan->SetValue(false);
    itemBoxSizer23->Add(m_UpdatemRNASpan, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    m_UpdateGeneSpan = new wxCheckBox( itemPanel1, ID_UPDATE_GENE_SPAN, _("Update Gene span"), wxDefaultPosition, wxDefaultSize, 0 );
    m_UpdateGeneSpan->SetValue(true);
    itemBoxSizer23->Add(m_UpdateGeneSpan, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

////@end CCDSTranslationPanel content construction   

    const CGenetic_code_table& code_table = CGen_code_table::GetCodeTable();
    const CGenetic_code_table::Tdata& codes = code_table.Get();

    ITERATE (CGenetic_code_table::Tdata, it, codes) {
        string str = (*it)->GetName();
        int id = (*it)->GetId();
        x_NormalizeGeneticCodeName(str);
        m_GeneticCode->Append(ToWxString(str), (void*)id);
    }

}


void CCDSTranslationPanel::DisableRetranslateOnOk(bool exception, bool pseudo)
{
    if (exception)        
        m_RetranslateOnOkBtn->SetValue(false);
    else 
        m_RetranslateOnOkBtn->SetValue(true);
    m_RetranslateOnOkBtn->Enable(!(exception || pseudo));
}


void CCDSTranslationPanel::x_NormalizeGeneticCodeName(string& code_name)
{
    string::size_type pos = code_name.find(';');
    code_name = (pos != string::npos) ? code_name.substr(0, pos) + ", etc." : code_name;
}


bool CCDSTranslationPanel::TransferDataToWindow()
{
    if (!wxPanel::TransferDataToWindow()) {
        return false;
    }
    string label;
    if (m_EditedFeat->IsSetProduct()) {
        const CSeq_id *id = m_EditedFeat->GetProduct().GetId();
        if (id) {
            CBioseq_Handle bsh = m_Scope->GetBioseqHandle(*id);
            if (bsh) {
                CConstRef<CSeq_id> seqid = sequence::GetId(bsh, sequence::eGetId_Best).GetSeqId();
                seqid->GetLabel(&label, CSeq_id::eFasta);

                // warn user if this label is different from the CDS product_id (GB-6513)
                string label_feat;
                auto feat_seqid_handle = sequence::GetId(*m_EditedFeat->GetProduct().GetId(), *m_Scope, sequence::eGetId_Best);
                if (feat_seqid_handle)
                    feat_seqid_handle.GetSeqId()->GetLabel(&label_feat, CSeq_id::eFasta);
                else
                    m_EditedFeat->GetProduct().GetId()->GetLabel(&label_feat, CSeq_id::eFasta);
                if (!NStr::EqualCase(label, label_feat)) {
                    NcbiWarningBox("Protein seq-id (" + label + ") differs from CDS product-id (" + label_feat + ")");
                }

            } else {
                auto feat_seqid_handle = sequence::GetId(*m_EditedFeat->GetProduct().GetId(), *m_Scope, sequence::eGetId_Best);
                if (feat_seqid_handle)
                    feat_seqid_handle.GetSeqId()->GetLabel(&label, CSeq_id::eFasta);
                else
                    m_EditedFeat->GetProduct().GetId()->GetLabel(&label, CSeq_id::eFasta);
            }
        }
    }
    m_ProductCtrl->SetValue (ToWxString(label));
    x_UpdateForceNear(label);
    if (m_EditedBioseq && m_EditedBioseq->IsAa()) {
        string prot_seq;
        CSeqVector prot_vec(*m_EditedBioseq, m_Scope);
        prot_vec.SetCoding(CSeq_data::e_Ncbieaa);
        prot_vec.GetSeqData(0, prot_vec.size(), prot_seq);
        if (m_add_stop_codon)
            prot_seq += "*";
        m_TranslationCtrl->SetValue(ToWxString(prot_seq));
        m_TranslationCtrl->Enable(true);
        m_ProteinLengthTxt->SetLabel(ToWxString(NStr::NumericToString(m_EditedBioseq->GetLength())));
    } else {
        m_TranslationCtrl->SetValue(wxEmptyString);
        m_ProteinLengthTxt->SetLabel(wxEmptyString);
    }

    CCdregion& cds = m_EditedFeat->SetData().SetCdregion();
    string codeName = "";
    int codeId = 1;

    if (cds.IsSetCode()) {
        const CCdregion::TCode& code = cds.GetCode();
        codeId = code.GetId();
        codeName = code.GetName();
    }

    if (NStr::IsBlank(codeName)) {
        const CGenetic_code_table& code_table = CGen_code_table::GetCodeTable();
        const CGenetic_code_table::Tdata& codes = code_table.Get();

        ITERATE (CGenetic_code_table::Tdata, it, codes) {
            string str = (*it)->GetName();
            int id = (*it)->GetId();
            if (id == codeId) {
                x_NormalizeGeneticCodeName(str);
                codeName = str;
                break;
            }
        }
    }            
    m_GeneticCode->SetStringSelection(ToWxString (codeName));

    if (cds.IsSetConflict() && cds.GetConflict()) {
        m_ConflictCtrl->SetValue (true);
    }

    if (cds.IsSetFrame()) {
        switch (cds.GetFrame()) {
            case CCdregion::eFrame_two:
                m_Frame->SetSelection(1);
                break;
            case CCdregion::eFrame_three:
                m_Frame->SetSelection(2);
                break;
            default:
                m_Frame->SetSelection(0);
                break;
        }
    } else {
        m_Frame->SetSelection(0);
    }
    return true;
}


wxTreebook* CCDSTranslationPanel::x_GetTree()
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


bool CCDSTranslationPanel::x_IsPseudo()
{
    wxTreebook* parent = x_GetTree();
    if (!parent) {
        return sequence::IsPseudo(*m_EditedFeat, *m_Scope);
    }
    CGenericPropsPanel* gen_panel = 0;
    for (size_t i = 0; i < parent->GetPageCount(); ++i) {
        gen_panel = dynamic_cast<CGenericPropsPanel*>(parent->GetPage(i));
        if (gen_panel)
            break;
    }
    if (!gen_panel) {
        return sequence::IsPseudo(*m_EditedFeat, *m_Scope);
    }
    return gen_panel->IsPseudo();
}


bool CCDSTranslationPanel::x_CollectData()
{
    if (!wxPanel::TransferDataFromWindow()) {
        return false;
    }

    // change product ID
    string id_string = ToStdString (m_ProductCtrl->GetValue());
    if (NStr::IsBlank(id_string)) {
        m_EditedFeat->ResetProduct();
    } else {
            CRef<CSeq_id> id =  GetProductId();
            if (id)
            {
                m_EditedFeat->SetProduct().SetWhole().Assign(*id);    
                if (m_EditedBioseq) {
                    m_EditedBioseq->SetId().clear();
                    m_EditedBioseq->SetId().push_back(id);
                }
            }
    }

    int index = m_GeneticCode->GetSelection();
    int id = 1;
    if (index != wxNOT_FOUND)
        id = (long)m_GeneticCode->GetClientData(index);

    CRef< CGenetic_code::C_E > ce(new CGenetic_code::C_E);
    ce->SetId(id);
    CRef<CGenetic_code> code(new CGenetic_code());
    code->Set().push_back(ce);

    CCdregion& cds = m_EditedFeat->SetData().SetCdregion();
    cds.SetCode(*code);

    if (m_ConflictCtrl->GetValue ()) {
        cds.SetConflict(true);
    } else {
        cds.ResetConflict();
    }

    switch (m_Frame->GetSelection ()) {
        case 0:
            cds.SetFrame(CCdregion::eFrame_one);
            break;
        case 1:
            cds.SetFrame(CCdregion::eFrame_two);
            break;
        case 2:
            cds.SetFrame(CCdregion::eFrame_three);
            break;
        default:
            cds.ResetFrame();
            break;
    }
    return true;
}


bool CCDSTranslationPanel::TransferDataFromWindow()
{
    if (!x_CollectData()) {
        return false;
    }
   
    return true;
}


void CCDSTranslationPanel::SetProduct(CSeq_feat& feat)
{
    if (m_EditedFeat->IsSetProduct()) {
        CRef<CSeq_loc> product(new CSeq_loc());
        product->Assign(m_EditedFeat->GetProduct());
        feat.SetProduct(*product);
    } else {
        feat.ResetProduct();
    }

    const CCdregion& cds = m_EditedFeat->GetData().GetCdregion();
    CCdregion& edit_cds = feat.SetData().SetCdregion();
    if (cds.IsSetCode()) {
        CRef<CGenetic_code> new_code(new CGenetic_code());
        new_code->Assign(cds.GetCode());
        edit_cds.SetCode(*new_code);
    } else {
        edit_cds.ResetCode();
    }

    if (cds.IsSetFrame()) {
        edit_cds.SetFrame(cds.GetFrame());
    } else {
        edit_cds.ResetFrame();
    }
    
    if (cds.IsSetConflict() && cds.GetConflict()) {
        edit_cds.SetConflict(true);
    } else {
        edit_cds.ResetConflict();
    }
        
        
}


void CCDSTranslationPanel::SetProtein(CBioseq& bioseq)
{
    TransferDataFromWindow();
    if (m_EditedBioseq && m_EditedBioseq->IsAa()) {
        // only copy data and length and ID
        CRef<CSeq_inst> inst(new CSeq_inst());
        inst->Assign(m_EditedBioseq->GetInst());
        bioseq.SetInst(*inst);
        if (m_EditedBioseq->IsSetId()) {
            CRef<CSeq_id> new_id(new CSeq_id());
            auto seqid_handle = sequence::GetId(*(m_EditedBioseq->GetId().front()), *m_Scope, sequence::eGetId_Best);
            if (seqid_handle)
                new_id->Assign(*seqid_handle.GetSeqId());
            else
                new_id->Assign(*(m_EditedBioseq->GetId().front()));
            bioseq.SetId().push_back(new_id);        
        }
    } else {
        bioseq.ResetInst();
    }
}


/*!
 * Should we show tooltips?
 */

bool CCDSTranslationPanel::ShowToolTips()
{
    return true;
}

/*!
 * Get bitmap resources
 */

wxBitmap CCDSTranslationPanel::GetBitmapResource( const wxString& name )
{
    // Bitmap retrieval
////@begin CCDSTranslationPanel bitmap retrieval
    wxUnusedVar(name);
    return wxNullBitmap;
////@end CCDSTranslationPanel bitmap retrieval
}

/*!
 * Get icon resources
 */

wxIcon CCDSTranslationPanel::GetIconResource( const wxString& name )
{
    // Icon retrieval
////@begin CCDSTranslationPanel icon retrieval
    wxUnusedVar(name);
    return wxNullIcon;
////@end CCDSTranslationPanel icon retrieval
}


void CCDSTranslationPanel::Retranslate()
{
    x_CollectData();
    // get new location and new frame/code, if it has been edited
    wxWindow* parent = this->GetParent();
    wxTreebook* tbook = dynamic_cast<wxTreebook*>(parent);
    if (tbook) {
        CLocationPanel* locPanel = 0;
        CCDSExceptionPanel* exceptionPanel = 0;

        for (size_t i = 0; i < tbook->GetPageCount(); ++i) {
            if (!locPanel) {
                locPanel = dynamic_cast<CLocationPanel*>(tbook->GetPage(i));
            }
            if (!exceptionPanel) {
                exceptionPanel = dynamic_cast<CCDSExceptionPanel*>(tbook->GetPage(i));
            }

            if (locPanel && exceptionPanel) {
                break;
            }
        }
        if (locPanel) {
            locPanel->TransferDataFromWindow();
            CRef<objects::CSeq_loc> loc = locPanel->GetSeq_loc();
            m_EditedFeat->SetLocation(*loc);
        }

        if (exceptionPanel) {
            exceptionPanel->TransferDataFromWindow();
            exceptionPanel->SetExceptions(m_EditedFeat->SetData().SetCdregion());
        }

    }        
    x_Translate();
    TransferDataToWindow();
}

void CCDSTranslationPanel::x_Translate()
{
    m_add_stop_codon = false;

    if (x_IsPseudo())
    {
        m_EditedBioseq.Reset();
        m_EditedFeat->ResetProduct();      
        return;
    }

    string prot;
    try
    {
        CSeqTranslator::Translate(*m_EditedFeat, *m_Scope, prot);
    }
    catch(CSeqMapException&) {}
    if (!prot.empty())
    {
        if (!m_EditedBioseq) 
        {
            m_EditedBioseq.Reset(new CBioseq());          
        }
        if (NStr::EndsWith(prot, "*")) 
        {
            prot = prot.substr(0, prot.length() - 1);
            m_add_stop_codon = true;
        }
        m_EditedBioseq->SetInst().ResetExt();
        m_EditedBioseq->SetInst().SetRepr(objects::CSeq_inst::eRepr_raw); 
        m_EditedBioseq->SetInst().SetSeq_data().SetNcbieaa().Set(prot);
        m_EditedBioseq->SetInst().SetLength(TSeqPos(prot.length()));
        m_EditedBioseq->SetInst().SetMol(CSeq_inst::eMol_aa);
    }
    else
    {
        m_EditedBioseq.Reset();
    }
}

bool CCDSTranslationPanel::ShouldCreateNewProductSeq()
{
    return m_ForceNear->GetValue();
}


void CCDSTranslationPanel::SetProductId(const string& val)
{
    m_ProductCtrl->SetValue(ToWxString(val));
    x_UpdateForceNear(val);
    CRef<CSeq_id> id = GetProductId();
    if (!id) {
        m_EditedFeat->ResetProduct();
        m_EditedBioseq->SetId().clear();
    } else {
        m_EditedFeat->SetProduct().SetWhole().Assign(*id);
        m_EditedBioseq->SetId().clear();
        m_EditedBioseq->SetId().push_back(id);
    }
}


CRef<CSeq_id> CCDSTranslationPanel::GetProductId()
{
    string id_val = ToStdString(m_ProductCtrl->GetValue());
    if (m_SeqIds.find(id_val) != m_SeqIds.end())
    {
        CRef<CSeq_id> id(new CSeq_id);
        id->Assign(*m_SeqIds[id_val]);
        return id;
    }
    if (!NStr::IsBlank(id_val)) {
        CRef<CSeq_id> id;
        try {        
            id.Reset(new CSeq_id(id_val, CSeq_id::fParse_Default));
        } catch (const CSeqIdException&) {
            id.Reset(new CSeq_id);
            if (NStr::StartsWith(id_val, "gnl|"))
            {
                NStr::TrimPrefixInPlace(id_val, "gnl|");
                string db, tag;
                NStr::SplitInTwo(id_val, ":", db, tag);
                id->SetGeneral().SetDb(db);
                id->SetGeneral().SetTag().SetStr(tag);
            }
            else
            {
                id->SetLocal().SetStr(id_val);
            }
        }
        return id;
    }
    return CRef<CSeq_id>(NULL);
}


/*!
 * wxEVT_COMMAND_HYPERLINK event handler for ID_HYPERLINKCTRL1
 */

void CCDSTranslationPanel::OnRetranslateLinkHyperlinkClicked( wxHyperlinkEvent& event )
{
    if (x_IsPseudo() &&
        wxYES == wxMessageBox(_("Cannot retranslate with pseudo set, remove pseudo flag?"), wxT("Remove pseudo"), wxYES_NO | wxICON_QUESTION))
    {
        wxTreebook* parent = x_GetTree();
        if (!parent) {
            return;
        }
        CGenericPropsPanel* gen_panel = 0;
        for (size_t i = 0; i < parent->GetPageCount(); ++i) {
            gen_panel = dynamic_cast<CGenericPropsPanel*>(parent->GetPage(i));
            if (gen_panel)
                break;
        }
        if (!gen_panel) {
            return;
        }
        gen_panel->ResetPseudo();
    }
    Retranslate();
}


static CRef<CSeq_loc> TruncateSeqLoc (CRef<CSeq_loc> orig_loc, size_t new_len)
{
    CRef<CSeq_loc> new_loc;
    if (!orig_loc) {
        return new_loc;
    }
    size_t len = 0;
    for (CSeq_loc_CI it(*orig_loc); it && len < new_len; ++it) {
        size_t this_len = it.GetRange().GetLength();
        CConstRef<CSeq_loc> this_loc = it.GetRangeAsSeq_loc();
        if (len + this_len <= new_len) {
            if (new_loc) {
                new_loc->Add(*this_loc);
            } else {
                new_loc.Reset(new CSeq_loc());
                new_loc->Assign(*this_loc);
            }
            len += this_len;
        } else {
            CRef<CSeq_loc> partial_loc(new CSeq_loc());
            size_t len_wanted = new_len - len;
            size_t start = this_loc->GetStart(eExtreme_Biological);
            if (len_wanted == 1) {
                // make a point
                partial_loc->SetPnt().SetPoint(start);
            } else {
                // make an interval
                if (this_loc->IsSetStrand() && this_loc->GetStrand() == eNa_strand_minus) {
                    partial_loc->SetInt().SetFrom(start - len_wanted + 1);
                    partial_loc->SetInt().SetTo(start);
                } else {
                    partial_loc->SetInt().SetFrom(start);
                    partial_loc->SetInt().SetTo(start + len_wanted - 1);
                }
            }
            partial_loc->SetId(*this_loc->GetId());
            if (this_loc->IsSetStrand()) {
                partial_loc->SetStrand(this_loc->GetStrand());
            }
            partial_loc->SetPartialStart(this_loc->IsPartialStart(eExtreme_Biological),eExtreme_Biological);
            partial_loc->SetPartialStop(this_loc->IsPartialStop(eExtreme_Biological),eExtreme_Biological);
            if (new_loc) {
                new_loc->Add(*partial_loc);
            } else {
                new_loc.Reset(new CSeq_loc());
                new_loc->Assign(*partial_loc);
            }
            len += len_wanted;  
        }
    }

    return new_loc;
}


CRef<CSeq_loc> SeqLocExtend(const CSeq_loc& loc, size_t pos, CScope* scope)
{
    size_t loc_start = loc.GetStart(eExtreme_Positional);
    size_t loc_stop = loc.GetStop(eExtreme_Positional);
    bool partial_start = loc.IsPartialStart(eExtreme_Positional);
    bool partial_stop = loc.IsPartialStop(eExtreme_Positional);
    ENa_strand strand = loc.GetStrand();
    CRef<CSeq_loc> new_loc(NULL);

    if (pos < loc_start) {
        CRef<CSeq_id> id(new CSeq_id());
        id->Assign(*(loc.GetId()));
        CRef<CSeq_loc> add(new CSeq_loc(*id, pos, loc_start - 1, strand));
        add->SetPartialStart(partial_start, eExtreme_Positional);
        new_loc = sequence::Seq_loc_Add(loc, *add, CSeq_loc::fSort | CSeq_loc::fMerge_AbuttingOnly, scope);
    } else if (pos > loc_stop) {
        CRef<CSeq_id> id(new CSeq_id());
        id->Assign(*(loc.GetId()));
        CRef<CSeq_loc> add(new CSeq_loc(*id, loc_stop + 1, pos, strand));
        add->SetPartialStop(partial_stop, eExtreme_Positional);
        new_loc = sequence::Seq_loc_Add(loc, *add, CSeq_loc::fSort | CSeq_loc::fMerge_AbuttingOnly, scope);
    }
    return new_loc;
}


ENa_strand GetStrandForLastInterval(const CSeq_loc& loc)
{
    CSeq_loc_CI it(loc);
    ENa_strand strand = eNa_strand_unknown;
    while (it) {
        strand = it.GetStrand();
        ++it;
    }
    return strand;
}

static CRef<CSeq_loc> ExtendToStopCodon (CRef<CSeq_feat> feat, CScope* scope)
{
    CRef<CSeq_loc> new_loc;

    if (!feat || !feat->IsSetLocation()) {
        return new_loc;
    }
    const CSeq_loc& loc = feat->GetLocation();

    CBioseq_Handle bsh;
    try
    {
        bsh = scope->GetBioseqHandle(*(loc.GetId()));
    } catch (const CException &e) {}
    if (!bsh) {
        return new_loc;
    }

    const CGenetic_code* code = NULL;
    if (feat->IsSetData() && feat->GetData().IsCdregion() && feat->GetData().GetCdregion().IsSetCode()) {
        code = &(feat->GetData().GetCdregion().GetCode());
    }

    size_t stop = loc.GetStop(eExtreme_Biological);
    // figure out if we have a partial codon at the end
    size_t orig_len = sequence::GetLength(loc, scope);
    size_t len = orig_len;
    if (feat->IsSetData() && feat->GetData().IsCdregion() && feat->GetData().GetCdregion().IsSetFrame()) {
        CCdregion::EFrame frame = feat->GetData().GetCdregion().GetFrame();
        if (frame == CCdregion::eFrame_two) {
            len -= 1;
        } else if (frame == CCdregion::eFrame_three) {
            len -= 2;
        }
    }
    size_t mod = len % 3;
    CRef<CSeq_loc> vector_loc(new CSeq_loc());
    vector_loc->SetInt().SetId().Assign(*(loc.GetId()));

    if (GetStrandForLastInterval(loc) == eNa_strand_minus) {
        vector_loc->SetInt().SetFrom(0);
        vector_loc->SetInt().SetTo(stop + mod - 1);
        vector_loc->SetStrand(eNa_strand_minus);
    } else {
        vector_loc->SetInt().SetFrom(stop - mod + 1);
        vector_loc->SetInt().SetTo(bsh.GetInst_Length() - 1);
    }

    CSeqVector seq(*vector_loc, *scope, CBioseq_Handle::eCoding_Iupac);
    // reserve our space
    const size_t usable_size = seq.size();

    // get appropriate translation table
    const CTrans_table & tbl =
        (code ? CGen_code_table::GetTransTable(*code) :
                CGen_code_table::GetTransTable(1));

    // main loop through bases
    CSeqVector::const_iterator start = seq.begin();

    size_t i;
    size_t k;
    size_t state = 0;
    size_t length = usable_size / 3;

    for (i = 0;  i < length;  ++i) {
        // loop through one codon at a time
        for (k = 0;  k < 3;  ++k, ++start) {
            state = tbl.NextCodonState(state, *start);
        }

        if (tbl.GetCodonResidue (state) == '*') {
            CSeq_loc_CI it(loc);
            CSeq_loc_CI it_next = it;
            ++it_next;
            while (it_next) {
                CConstRef<CSeq_loc> this_loc = it.GetRangeAsSeq_loc();
                if (new_loc) {
                    new_loc->Add(*this_loc);
                } else {
                    new_loc.Reset(new CSeq_loc());
                    new_loc->Assign(*this_loc);
                }
                it = it_next;
                ++it_next;
            }
            CRef<CSeq_loc> last_interval(new CSeq_loc());
            CConstRef<CSeq_loc> this_loc = it.GetRangeAsSeq_loc();
            size_t this_start = this_loc->GetStart(eExtreme_Positional);
            size_t this_stop = this_loc->GetStop(eExtreme_Positional);
            size_t extension = ((i + 1) * 3) - mod;
            last_interval->SetInt().SetId().Assign(*(this_loc->GetId()));
            if (this_loc->IsSetStrand() && this_loc->GetStrand() == eNa_strand_minus) {
                last_interval->SetStrand(eNa_strand_minus);
                last_interval->SetInt().SetFrom(this_start - extension);
                last_interval->SetInt().SetTo(this_stop);
            } else {
                last_interval->SetInt().SetFrom(this_start);
                last_interval->SetInt().SetTo(this_stop + extension);
            }
                
            if (new_loc) {
                new_loc->Add(*last_interval);
            } else {
                new_loc.Reset(new CSeq_loc());
                new_loc->Assign(*last_interval);
            }
            new_loc->SetPartialStart(loc.IsPartialStart(eExtreme_Biological), eExtreme_Biological);
            new_loc->SetPartialStop(false, eExtreme_Biological);

            return new_loc;
        }
    }

    if (usable_size < 3 && !new_loc) {
        if (loc.GetStrand() == eNa_strand_minus) {
            new_loc = SeqLocExtend(loc, 0, scope);
        } else {
            new_loc = SeqLocExtend(loc, bsh.GetInst_Length() - 1, scope);
        }
        if (new_loc) {
            new_loc->SetPartialStop(true, eExtreme_Biological);
        }
    }

    return new_loc;
}


/*!
 * wxEVT_COMMAND_HYPERLINK event handler for ID_HYPERLINKCTRL2
 */

void CCDSTranslationPanel::AdjustForStopCodonHyperlinkClicked( wxHyperlinkEvent& event )
{
    TransferDataFromWindow();

    // get new location and new frame/code, if it has been edited
    wxWindow* parent = this->GetParent();
    wxTreebook* tbook = dynamic_cast<wxTreebook*>(parent);
    if (!tbook) {
        return;
    }

    CLocationPanel* locPanel = 0;
    CCDSExceptionPanel* exceptionPanel = 0;

    for (size_t i = 0; i < tbook->GetPageCount(); ++i) {
        if (!locPanel) {
            locPanel = dynamic_cast<CLocationPanel*>(tbook->GetPage(i));
        }
        if (!exceptionPanel) {
            exceptionPanel = dynamic_cast<CCDSExceptionPanel*>(tbook->GetPage(i));
        }

        if (locPanel && exceptionPanel) {
            break;
        }
    }
    if (!locPanel) {
        return;
    }

    locPanel->TransferDataFromWindow();
    CRef<objects::CSeq_loc> loc = locPanel->GetSeq_loc();
    m_EditedFeat->SetLocation(*loc);

    if (exceptionPanel) {
        exceptionPanel->TransferDataFromWindow();
        exceptionPanel->SetExceptions(m_EditedFeat->SetData().SetCdregion());
    }

    string prot_str ;
    CSeqTranslator::Translate(*m_EditedFeat, *m_Scope,  prot_str);
    if (!prot_str.empty()) {
        CRef<CSeq_loc> new_loc(NULL);
  
        size_t pos = NStr::Find(prot_str, "*");
        if (pos != string::npos) {
            // want to truncate the location and retranslate
            size_t len_wanted =  3 * (pos + 1);
            if (m_EditedFeat->GetData().GetCdregion().IsSetFrame()) {
                CCdregion::EFrame frame = m_EditedFeat->GetData().GetCdregion().GetFrame();
                if (frame == CCdregion::eFrame_two) {
                    len_wanted += 1;
                } else if (frame == CCdregion::eFrame_three) {
                    len_wanted += 2;
                }
            }
            if (len_wanted > 0) {
                new_loc = TruncateSeqLoc (loc, len_wanted);
                if (new_loc) {
                    new_loc->SetPartialStop(false, eExtreme_Biological);
                }
            }
        } else {
            // want to extend the location and retranslate
            new_loc = ExtendToStopCodon(m_EditedFeat, m_Scope);

            if (!new_loc) {
                int choice = wxMessageBox(wxT("No stop codon found - extend to full length of sequence (and make partial)?"), wxT("Error"),
                     wxYES_NO | wxICON_ERROR, NULL);
                if (choice == wxYES) {
                    if (loc->GetStrand() == eNa_strand_minus) {
                        new_loc = SeqLocExtend(*loc, 0, m_Scope);
                    } else {
                        CBioseq_Handle bsh;
                        try
                        {
                            bsh = m_Scope->GetBioseqHandle(*loc);
                        } catch(const CException &e) {}
                        new_loc = SeqLocExtend(*loc, bsh.GetInst_Length() - 1, m_Scope);
                    }
                    if (!new_loc) {
                        new_loc.Reset(new CSeq_loc());
                        new_loc->Assign(*loc);
                    }
                    new_loc->SetPartialStop(true, eExtreme_Biological); 
                }
            }
        }

        if (new_loc) {            
            m_EditedFeat->SetLocation(*new_loc);
            locPanel->SetSeq_loc(m_EditedFeat->SetLocation());
            x_Translate();
            TransferDataToWindow();
        }
    }
}


static void s_ExtendIntervalToEnd (objects::CSeq_interval& ival, size_t len)
{
    if (ival.IsSetStrand() && ival.GetStrand() == objects::eNa_strand_minus) {
        if (ival.GetFrom() > 3) {
            ival.SetFrom(ival.GetFrom() - 3);
        } else {
            ival.SetFrom(0);
        }
    } else {
        if (ival.GetTo() < len - 4) {
            ival.SetTo(ival.GetTo() + 3);
        } else {
            ival.SetTo(len - 1);
        }
    }
}


void CCDSTranslationPanel::x_SetLocationForProtein(CRef<objects::CSeq_feat> cds, objects::CSeq_entry_Handle seh)
{
    if (!m_EditedBioseq || !m_EditedBioseq->IsAa()) {
        return;
    }

    objects::CSeq_entry_Handle protein_h;
    if (!seh.GetScope().GetBioseqHandle(*m_EditedBioseq, CScope::eMissing_Null))
    {
        CRef<objects::CSeq_entry> protein_entry(new objects::CSeq_entry());
        protein_entry->SetSeq().Assign(*m_EditedBioseq);
        protein_h = seh.GetScope().AddTopLevelSeqEntry(*protein_entry);
    }

    CProSplign prosplign(CProSplignScoring(), false, true, false, false);

    CBioseq_Handle bsh;
    try
    {
        bsh = seh.GetScope().GetBioseqHandle(cds->GetLocation());
    } catch (const CException &e) {return;}
            
    CRef<objects::CSeq_id> seq_id(new objects::CSeq_id());
    seq_id->Assign(*(bsh.GetSeqId()));
    CRef<objects::CSeq_loc> match_loc(new objects::CSeq_loc(*seq_id, 0, bsh.GetBioseqLength() - 1));

    CRef<objects::CSeq_align> alignment;
    try
    {
        alignment = prosplign.FindAlignment(seh.GetScope(), *(m_EditedBioseq->GetId().front()), *match_loc,
                                            CProSplignOutputOptions(CProSplignOutputOptions::ePassThrough));
    }
    catch(const CException& e) {
        LOG_POST(Error << "Could not find alignment: " << e.GetMsg());
        alignment.Reset();
    }
    catch (const exception& e) {
        LOG_POST(Error << "Could not find alignment: " << e.what());
        alignment.Reset();
    }

    if (protein_h)
        seh.GetScope().RemoveTopLevelSeqEntry(protein_h);

    CRef<objects::CSeq_loc> cds_loc(new objects::CSeq_loc());
    bool found_start_codon = false;
    bool found_stop_codon = false;
    if (alignment && alignment->IsSetSegs() && alignment->GetSegs().IsSpliced()) {
        CRef<objects::CSeq_id> seq_id (new objects::CSeq_id());
        seq_id->Assign(*match_loc->GetId());

        ITERATE (objects::CSpliced_seg::TExons, exon_it, alignment->GetSegs().GetSpliced().GetExons()) {

            TSeqPos from = (*exon_it)->GetGenomic_start();
            CRangeCollection<TSeqPos> insertions = (*exon_it)->GetRowSeq_insertions(1, alignment->GetSegs().GetSpliced());
            for (auto &ins : insertions)
            {
                CRef<objects::CSeq_loc> exon2(new objects::CSeq_loc(*seq_id, from, ins.GetFrom() - 1));
                from = ins.GetTo() + 1;
                if ((*exon_it)->IsSetGenomic_strand()) {
                    exon2->SetStrand((*exon_it)->GetGenomic_strand());
                } else if (alignment->GetSegs().GetSpliced().IsSetGenomic_strand()) {
                    exon2->SetStrand(alignment->GetSegs().GetSpliced().GetGenomic_strand());
                } 
                cds_loc->SetMix().Set().push_back(exon2);
            }

            CRef<objects::CSeq_loc> exon(new objects::CSeq_loc(*seq_id, from, (*exon_it)->GetGenomic_end()));                
            if ((*exon_it)->IsSetGenomic_strand()) {
                exon->SetStrand((*exon_it)->GetGenomic_strand());
            } else if (alignment->GetSegs().GetSpliced().IsSetGenomic_strand()) {
                exon->SetStrand(alignment->GetSegs().GetSpliced().GetGenomic_strand());
            } 
          
            cds_loc->SetMix().Set().push_back(exon);
        }


        ITERATE (objects::CSpliced_seg::TModifiers, mod_it,
                 alignment->GetSegs().GetSpliced().GetModifiers()) {
            if ((*mod_it)->IsStart_codon_found()) {
                found_start_codon = (*mod_it)->GetStart_codon_found();
            }
            if ((*mod_it)->IsStop_codon_found()) {
                found_stop_codon = (*mod_it)->GetStop_codon_found();
            }
        }
        
    }

    if (!cds_loc->IsMix()) {
        //no exons, no match
        string label;        
        m_EditedBioseq->GetId().front()->GetLabel(&label, CSeq_id::eFasta);
        string error = "Unable to find coding region location for protein sequence " + label + ".  Import failed.";
        wxMessageBox(ToWxString(error), wxT("Error"),
                     wxOK | wxICON_ERROR, NULL);
        return;
    } else {
        if (cds_loc->GetMix().Get().size() == 1) {
            CRef<objects::CSeq_loc> exon = cds_loc->SetMix().Set().front();
            cds_loc->Assign(*exon);
        }
    }
    if (!found_start_codon) {
        cds_loc->SetPartialStart(true, objects::eExtreme_Biological);
    }
    if (found_stop_codon) {
        // extend to cover stop codon        
        size_t len = bsh.GetInst_Length();
        if (cds_loc->IsMix()) {
            s_ExtendIntervalToEnd(cds_loc->SetMix().Set().back()->SetInt(), len);
        } else {
            s_ExtendIntervalToEnd(cds_loc->SetInt(), len);
        }        
    } else {
        cds_loc->SetPartialStop(true, objects::eExtreme_Biological);
    }


    // set new location and frame
    wxWindow* parent = this->GetParent();
    wxTreebook* tbook = dynamic_cast<wxTreebook*>(parent);
    if (!tbook) {
        return;
    }

    CLocationPanel* locPanel = 0;
    CCDSExceptionPanel* exceptionPanel = 0;

    for (size_t i = 0; i < tbook->GetPageCount(); ++i) {
        if (!locPanel) {
            locPanel = dynamic_cast<CLocationPanel*>(tbook->GetPage(i));
        }
        if (!exceptionPanel) {
            exceptionPanel = dynamic_cast<CCDSExceptionPanel*>(tbook->GetPage(i));
        }

        if (locPanel && exceptionPanel) {
            break;
        }
    }
    if (!locPanel) {
        return;
    }

    m_EditedFeat->SetLocation(*cds_loc);
    locPanel->SetSeq_loc(m_EditedFeat->SetLocation());    
}

CBioseq_Handle GetBioseqHandleForMultihomeLocation(CScope &scope, const CSeq_loc &loc)
{
    CBioseq_Handle bsh;
    CSeq_loc_CI it(loc);
    while (it) 
    {
        try
        {
            bsh = scope.GetBioseqHandle(it.GetSeq_id());
        }
        catch (const CException& e) {}
        if (bsh)
            break;
        ++it;
    }
    return bsh;
}

void CCDSTranslationPanel::x_SetProtein(const CBioseq& protein, CRef<CSeq_feat> cds, CSeq_entry_Handle seh)
{
    CRef<objects::CSeq_entry> protein_entry(new objects::CSeq_entry());
    protein_entry->SetSeq().Assign(protein);
    CBioseq_Handle bsh = GetBioseqHandleForMultihomeLocation(seh.GetScope(), cds->GetLocation());
    int offset = 1;
    string id_label;
    bool create_general_only = objects::edit::IsGeneralIdProtPresent(seh);
    CRef<objects::CSeq_id> product_id = objects::edit::GetNewProtId(bsh, offset, id_label, create_general_only); 
    protein_entry->SetSeq().ResetId();
    protein_entry->SetSeq().SetId().push_back(product_id);

    if (!m_EditedBioseq) {
        m_EditedBioseq.Reset(new CBioseq());
    }
    m_EditedBioseq->Assign(protein_entry->GetSeq());
    m_add_stop_codon = false;

    if (cds->IsSetExcept_text() && IsValidException(cds->GetExcept_text())) {
        m_RetranslateOnOkBtn->SetValue(false);
        TransferDataToWindow();
        return;
    }
    // x_SetLocationForProtein(cds, seh); // GB-6528
    TransferDataToWindow();
}


/*!
 * wxEVT_COMMAND_HYPERLINK event handler for ID_IMPORT_CDS_PROTEIN
 */

void CCDSTranslationPanel::OnImportCdsProteinHyperlinkClicked( wxHyperlinkEvent& event )
{
    CRef<CFileLoadWizard> fileManager(new CFileLoadWizard());
    vector<string> format_ids;
    format_ids.push_back("file_loader_fasta");
    // format_ids.push_back("file_loader_asn");
    fileManager->LoadFormats(format_ids);

    vector<CIRef<IOpenObjectsPanelClient> > loadManagers;
    loadManagers.push_back(CIRef<IOpenObjectsPanelClient>(fileManager.GetPointer()));

    COpenObjectsDlg dlg(NULL);
    dlg.SetSize(710, 480);

    dlg.SetRegistryPath("Dialogs.Edit.OpenTable");
//    dlg.SetServiceLocator(workbench);
    dlg.SetManagers(loadManagers);

    if (dlg.ShowModal() == wxID_OK) {
        CIRef<IObjectLoader> object_loader(dlg.GetObjectLoader());
        if (!object_loader) {
            wxMessageBox(wxT("Failed to get object loader"), wxT("Error"),
                         wxOK | wxICON_ERROR);
        }
        else {
            IExecuteUnit* execute_unit = dynamic_cast<IExecuteUnit*>(object_loader.GetPointer());
            if (execute_unit) {
                if (!execute_unit->PreExecute())
                    return;

                if (!GUI_AsyncExecUnit(*execute_unit, wxT("Reading file(s)...")))
                    return; // Canceled

                if (!execute_unit->PostExecute())
                    return;
            }
            const objects::CBioseq* seq = NULL;
            const IObjectLoader::TObjects& objects = object_loader->GetObjects();
            ITERATE(IObjectLoader::TObjects, obj_it, objects) {
                const CObject& ptr = obj_it->GetObject();
                seq = dynamic_cast<const objects::CBioseq*>(&ptr);
                if (seq) {
                    break;
                } else {
                    const objects::CSeq_entry* entry = dynamic_cast<const objects::CSeq_entry*>(&ptr);
                    if (entry && entry->IsSeq()) {
                        seq = &(entry->GetSeq());
                        break;
                    }
                }
            }
            if (seq && seq->IsAa()) {
                x_SetProtein(*seq, m_EditedFeat, GetBioseqHandleForMultihomeLocation(*m_Scope, m_EditedFeat->GetLocation()).GetTSE_Handle().GetTopLevelEntry());
            } else {
                 wxMessageBox(wxT("Please import a protein sequence file"), wxT("Error"),
                         wxOK | wxICON_ERROR);
            }
        }
    }

}


/*!
 * wxEVT_COMMAND_HYPERLINK event handler for ID_PREDICT_INTERVAL
 */

void CCDSTranslationPanel::OnPredictIntervalHyperlinkClicked( wxHyperlinkEvent& event )
{
    wxBusyCursor wait;
   
    x_SetLocationForProtein(m_EditedFeat, GetBioseqHandleForMultihomeLocation(*m_Scope, m_EditedFeat->GetLocation()).GetTSE_Handle().GetTopLevelEntry());
    TransferDataToWindow();
}

void CCDSTranslationPanel::OnProductChanged(wxCommandEvent& event)
{
    UpdateForceNear();
}

void CCDSTranslationPanel::UpdateForceNear()
{
    string val = ToStdString(m_ProductCtrl->GetValue());
    x_UpdateForceNear(val);
}

void CCDSTranslationPanel::x_UpdateForceNear(const string &val)
{
    if (x_IsPseudo())
    {
        m_ForceNear->SetValue(false);
        m_ForceNear->Disable();
    }
    else if (NStr::IsBlank(val))
    {
        m_ForceNear->SetValue(true);
        m_ForceNear->Disable();
    }
    else if (m_SeqIds.find(val) == m_SeqIds.end())
    {
        m_ForceNear->SetValue(true);
        m_ForceNear->Enable();
    }
    else
    {
        m_ForceNear->SetValue(false);
        m_ForceNear->Enable();
    }
}

void CCDSTranslationPanel::OnRetranslateOnOkChanged(wxCommandEvent& event)
{
    if (!x_IsPseudo() && NStr::IsBlank(ToStdString(m_TranslationCtrl->GetValue())) && !m_RetranslateOnOkBtn->GetValue()) {
        wxMessageBox(wxT("A coding region usually has to have a protein translation unless it is pseudo"), wxT("Warning"), wxOK | wxICON_ERROR, this);
    }
}

END_NCBI_SCOPE
