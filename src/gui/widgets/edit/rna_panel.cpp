/*  $Id: rna_panel.cpp 42593 2019-03-22 19:03:13Z filippov $
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
////@begin includes
#include "wx/imaglist.h"
////@end includes

#include <gui/widgets/edit/serial_member_primitive_validators.hpp>
#include <gui/widgets/wx/wx_utils.hpp>
#include <serial/serialbase.hpp>
#include <serial/typeinfo.hpp>

#include <objects/seqfeat/Seq_feat.hpp>
#include <objects/seqfeat/SeqFeatData.hpp>
#include <objects/seqfeat/RNA_ref.hpp>
#include <objects/seqfeat/RNA_gen.hpp>
#include <objects/seqfeat/Trna_ext.hpp>
#include <objects/seqloc/Seq_loc.hpp>
#include <objects/seqloc/Seq_id.hpp>
#include <objects/seq/Seq_inst.hpp>
#include <objects/general/Object_id.hpp>
#include <objmgr/scope.hpp>
#include <objmgr/bioseq_ci.hpp>
#include <gui/widgets/edit/utilities.hpp>

#include "rna_panel.hpp"
#include "mrna_subpanel.hpp"
#include "rrna_subpanel.hpp"
#include "trna_subpanel.hpp"
#include "ncrna_subpanel.hpp"
#include "tmrna_subpanel.hpp"


#include <wx/sizer.h>
#include <wx/bitmap.h>
#include <wx/icon.h>
#include <wx/radiobox.h>
#include <wx/stattext.h>
#include <wx/combobox.h>
#include <wx/textctrl.h>


BEGIN_NCBI_SCOPE
USING_SCOPE(objects);

////@begin XPM images
////@end XPM images


/*!
 * CRNAPanel type definition
 */

IMPLEMENT_DYNAMIC_CLASS( CRNAPanel, wxPanel )


/*!
 * CRNAPanel event table definition
 */

BEGIN_EVENT_TABLE( CRNAPanel, wxPanel )
////@begin CRNAPanel event table entries
////@end CRNAPanel event table entries
END_EVENT_TABLE()


/*!
 * CRNAPanel constructors
 */

CRNAPanel::CRNAPanel()
{
    Init();
}

CRNAPanel::CRNAPanel( wxWindow* parent, CSerialObject& object, CScope& scope,
                      wxWindowID id, const wxPoint& pos, const wxSize& size, long style )
                      : m_Object(0), m_Scope(&scope), m_trna(new CTrna_ext()), m_ncRNAGen(new CRNA_gen()), m_tmRNAGen(new CRNA_gen())
{
    m_Object = &object;
    _ASSERT(m_Object->GetThisTypeInfo()->GetName() == "Seq-feat");
    Init();
    Create(parent, id, pos, size, style);
}


/*!
 * CRNAPanel creator
 */

bool CRNAPanel::Create( wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style )
{
////@begin CRNAPanel creation
    wxPanel::Create( parent, id, pos, size, style );

    CreateControls();
    if (GetSizer())
    {
        GetSizer()->SetSizeHints(this);
    }
    Centre();
////@end CRNAPanel creation
    return true;
}


/*!
 * CRNAPanel destructor
 */

CRNAPanel::~CRNAPanel()
{
////@begin CRNAPanel destruction
////@end CRNAPanel destruction
}


/*!
 * Member initialisation
 */

void CRNAPanel::Init()
{
////@begin CRNAPanel member initialisation
    m_Choicebook = NULL;
    m_TranscriptID = NULL;
////@end CRNAPanel member initialisation
}


/*!
 * Control creation for CRNAPanel
 */

void CRNAPanel::CreateControls()
{
////@begin CRNAPanel content construction
    CRNAPanel* itemPanel1 = this;

    wxBoxSizer* itemBoxSizer2 = new wxBoxSizer(wxVERTICAL);
    itemPanel1->SetSizer(itemBoxSizer2);

    m_Choicebook = new wxChoicebook( itemPanel1, ID_CHOICEBOOK1, wxDefaultPosition, wxDefaultSize, wxBK_DEFAULT );

    itemBoxSizer2->Add(m_Choicebook, 0, wxGROW|wxALL, 5);

    wxBoxSizer* itemBoxSizer4 = new wxBoxSizer(wxHORIZONTAL);
    itemBoxSizer2->Add(itemBoxSizer4, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 0);

    wxStaticText* itemStaticText5 = new wxStaticText( itemPanel1, wxID_STATIC, _("Transcript Sequence ID"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer4->Add(itemStaticText5, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxArrayString m_TranscriptIDStrings;
    m_TranscriptID = new wxComboBox( itemPanel1, ID_COMBOBOX1, wxEmptyString, wxDefaultPosition, wxDefaultSize, m_TranscriptIDStrings, wxCB_DROPDOWN );
    if (CRNAPanel::ShowToolTips())
        m_TranscriptID->SetToolTip(_("Please use accession and version to specify transcript sequence."));
    itemBoxSizer4->Add(m_TranscriptID, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

////@end CRNAPanel content construction
    CSeq_feat& seq_feat = dynamic_cast<CSeq_feat&>(*m_Object);
    CRNA_ref& rna = seq_feat.SetData().SetRna();

    m_Choicebook->AddPage(new wxWindow(m_Choicebook, wxID_ANY), wxT("unknown"));

    string name;
    if (rna.IsSetExt()) {
        const CRNA_ref::C_Ext& ext = rna.GetExt();
        if (ext.IsName()) {
            name = ext.GetName();
        } else if (ext.IsTRNA()) {
            m_trna->Assign(ext.GetTRNA());
        } else if (ext.IsGen()) {
            m_ncRNAGen->Assign(ext.GetGen());
            m_tmRNAGen->Assign(ext.GetGen());
            if (ext.GetGen().IsSetProduct()) {
                name = ext.GetGen().GetProduct();
            }
        }
    }

    CmRNASubPanel* namePanel = new CmRNASubPanel(m_Choicebook, wxID_ANY);
    namePanel->SetRnaName(ToWxString(name));
    m_Choicebook->AddPage(namePanel, wxT("preRNA"));

    namePanel = new CmRNASubPanel(m_Choicebook, wxID_ANY);
    namePanel->SetRnaName(ToWxString(name));
    m_Choicebook->AddPage(namePanel, wxT("mRNA"));

    CtRNASubPanel* trnaPanel = new CtRNASubPanel(m_Choicebook, m_trna, ConstRef(&seq_feat), *m_Scope, wxID_ANY);
    m_Choicebook->AddPage(trnaPanel, wxT("tRNA"));

    CrRNASubPanel* namePanel2 = new CrRNASubPanel(m_Choicebook, wxID_ANY);
    namePanel2->SetRnaName(ToWxString(name));
    m_Choicebook->AddPage(namePanel2, wxT("rRNA"));

    CncRNASubPanel* ncrnaPanel = new CncRNASubPanel(m_Choicebook, m_ncRNAGen, wxID_ANY);
    m_Choicebook->AddPage(ncrnaPanel, wxT("ncRNA"));

    CtmRNASubPanel* tmrnaPanel = new CtmRNASubPanel(m_Choicebook, m_tmRNAGen, wxID_ANY);
    m_Choicebook->AddPage(tmrnaPanel, wxT("tmRNA"));

    namePanel2 = new CrRNASubPanel(m_Choicebook, wxID_ANY);
    namePanel2->SetRnaName(ToWxString(name));
    m_Choicebook->AddPage(namePanel2, wxT("misc_RNA"));

    CRNA_ref::TType type = rna.GetType();

    size_t page_index = 0;

    switch (type) {
    case CRNA_ref::eType_unknown: page_index =  0; break;
    case CRNA_ref::eType_premsg:  page_index =  1; break;
    case CRNA_ref::eType_mRNA:    page_index =  2; break;
    case CRNA_ref::eType_tRNA:    page_index =  3; break;
    case CRNA_ref::eType_rRNA:    page_index =  4; break;
    case CRNA_ref::eType_ncRNA:   page_index =  5; break;
    case CRNA_ref::eType_snRNA:   page_index =  5; break;
    case CRNA_ref::eType_scRNA:   page_index =  5; break;
    case CRNA_ref::eType_snoRNA:  page_index =  5; break;
    case CRNA_ref::eType_tmRNA:   page_index =  6; break;
    case CRNA_ref::eType_miscRNA: page_index =  7; break;
    case CRNA_ref::eType_other:   page_index =  7; break;
    default:                      page_index =  0; break;
    }

    m_Choicebook->SetSelection(page_index);
    x_PopulateTranscriptID();
}

void CRNAPanel::SetGBQualPanel(CGBQualPanel* gbqual_panel)
{
    CtRNASubPanel* trna_panel = 0;
    for (size_t i = 0; i < m_Choicebook->GetPageCount(); ++i) {
        if ((trna_panel = dynamic_cast<CtRNASubPanel*>(m_Choicebook->GetPage(i))) != 0) {
            trna_panel->SetGBQualPanel(gbqual_panel);
        }
    }
}


void CRNAPanel::x_PopulateTranscriptID()
{
    _ASSERT(m_Scope);

    CSeq_feat& seq_feat = dynamic_cast<CSeq_feat&>(*m_Object);
    CRNA_ref& rna = seq_feat.SetData().SetRna();

    CConstRef<CSeq_id> transcript_id;
    if (seq_feat.IsSetProduct()) {
        transcript_id.Reset(seq_feat.GetProduct().GetId());
    }

    int match = -1, pos = 0;
    wxArrayString TranscriptIDs;
    CScope::TTSE_Handles tses;
    m_Scope->GetAllTSEs(tses, CScope::eAllTSEs);
    ITERATE (CScope::TTSE_Handles, handle, tses) {
        for (CBioseq_CI b_iter(*handle, CSeq_inst::eMol_rna);  b_iter;  ++b_iter) {
            CConstRef<CSeq_id> id = b_iter->GetId().front().GetSeqId();
            if (id) {
                string id_label = kEmptyStr;
                x_GetLabelForSeqID(*id, id_label);
                if (transcript_id) {
                    ITERATE (CBioseq_Handle::TId, id_it, b_iter->GetId()) {
                        if (id_it->GetSeqId()->Compare(*transcript_id) == CSeq_id::e_YES) {
                            match = pos;
                            x_GetLabelForSeqID(*transcript_id, id_label);
                        }
                    }
                }
                TranscriptIDs.Add(ToWxString(id_label));
                ++pos;
            }
        }
    }

    m_TranscriptID->Append (TranscriptIDs);
    if (seq_feat.IsSetProduct() || 
        (rna.IsSetType() && rna.GetType() == CRNA_ref::eType_mRNA)) {
        if (match > -1) {
            m_TranscriptID->SetSelection(match);
        } else if (transcript_id) {
            string id_label = kEmptyStr;
            x_GetLabelForSeqID(*transcript_id, id_label);
            m_TranscriptID->SetValue(ToWxString(id_label));
        }
    } else {
        m_TranscriptID->Enable(false);
    }
}

void CRNAPanel::x_GetLabelForSeqID(const CSeq_id& id, string& id_label)
{
    id_label.clear();
    id.GetLabel(&id_label);
}

static bool s_EndsWithDigitsAndVersion (string str, int num_digits_expected)
{
    int pos = 0;

    while (pos < num_digits_expected && pos < str.length() && isdigit(str.c_str()[pos])) {
        pos++;
    }
    if (pos < num_digits_expected) {
        return false;
    } else if (pos == str.length()) {
        return false;
    } else if (str.c_str()[pos] != '.') {
        return false;
    }
    pos++;
    if (pos == str.length()) {
        return false;
    }
    while (pos < str.length()) {
        if (!isdigit(str.c_str()[pos])) {
            return false;
        }
        pos++;
    }
    return true;
}


static CRef<CSeq_id> s_GuessId (const string& id_str, CScope* scope)
{
    // is this local?
    CRef<CSeq_id> tid(new CSeq_id("lcl|" + id_str));
    CBioseq_Handle bsh = scope->GetBioseqHandle(*tid);
    if (!bsh) {
        try {
            // not local - is RefSeq accession?
            if (id_str.length() > 8
                && NStr::StartsWith(id_str, "N") && isalpha (id_str.c_str()[1])
                && isupper (id_str.c_str()[1])
                && id_str.c_str()[2] == '_'
                && s_EndsWithDigitsAndVersion(id_str.substr(3), 6)) {
                tid = new CSeq_id("ref|" + id_str);
            } else if (id_str.length() >= 8
                       && isalpha (id_str.c_str()[0])
                       && isupper (id_str.c_str()[0])
                       && isalpha (id_str.c_str()[1])
                       && isupper (id_str.c_str()[1])
                       && s_EndsWithDigitsAndVersion(id_str.substr(2), 6)) {
                tid = new CSeq_id("gb|" + id_str);
            }
        } catch (CException&) {
            tid = new CSeq_id("lcl|" + id_str);
        }
    }
    return tid;
}


void x_SetNameFromName(const wxString& input_name, CRNA_ref& rna)
{
    string name = GetAdjustedRnaName(ToStdString(input_name));
    if (NStr::IsBlank(name)) {
        rna.ResetExt();
    } else {
        rna.SetExt().SetName(name);
    }
}


bool CRNAPanel::TransferDataFromWindow()
{
    bool result = wxPanel::TransferDataFromWindow();

    for (size_t i = 0; i < m_Choicebook->GetPageCount(); ++i) {
        m_Choicebook->GetPage(i)->TransferDataFromWindow();
    }

    CSeq_feat& seq_feat = dynamic_cast<CSeq_feat&>(*m_Object);
    CRNA_ref& rna = seq_feat.SetData().SetRna();
    rna.SetExt().Reset();

    size_t page_index = m_Choicebook->GetSelection();
    wxWindow* page = m_Choicebook->GetPage(page_index);
    switch(page_index) {
    case 0:
        rna.SetType(CRNA_ref::eType_unknown);
        break;
    case 1:
        rna.SetType(CRNA_ref::eType_premsg);
        {{
            CmRNASubPanel* namePanel = (CmRNASubPanel*)page;
            x_SetNameFromName(namePanel->GetRnaName(), rna);
        }}
        break;
    case 2:
        rna.SetType(CRNA_ref::eType_mRNA);
        {{
            CmRNASubPanel* namePanel = (CmRNASubPanel*)page;
            x_SetNameFromName(namePanel->GetRnaName(), rna);
        }}
        break;
    case 3:
        rna.SetType(CRNA_ref::eType_tRNA);
        rna.SetExt().SetTRNA(*m_trna);
        break;
    case 4:
        rna.SetType(CRNA_ref::eType_rRNA);
        {{
            CrRNASubPanel* namePanel = (CrRNASubPanel*)page;
            x_SetNameFromName(namePanel->GetRnaName(), rna);
        }}
        break;
    case 5:
        rna.SetType(CRNA_ref::eType_ncRNA);
        rna.SetExt().SetGen(*m_ncRNAGen);
        break;
    case 6:
        rna.SetType(CRNA_ref::eType_tmRNA);
        rna.SetExt().SetGen(*m_tmRNAGen);
        break;
    case 7:
        rna.SetType(CRNA_ref::eType_miscRNA);
        {{
            CrRNASubPanel* namePanel = (CrRNASubPanel*)page;
            x_SetNameFromName(namePanel->GetRnaName(), rna);
        }}
        break;

    }

    string transcript_id = ToStdString(m_TranscriptID->GetValue());
    if (NStr::IsBlank(transcript_id)) {
        seq_feat.ResetProduct();
    } else {
        CRef<CSeq_id> tid;
        if (NStr::Find(transcript_id, "|") == string::npos) {
            tid = s_GuessId(transcript_id, m_Scope);
        } else {
            try {
                tid = (new CSeq_id(transcript_id));
            } catch (CException&) {
                tid = new CSeq_id();
                tid->SetLocal().SetStr(transcript_id);
            }
        }
        seq_feat.SetProduct().SetWhole().Assign(*tid);
    }

    return result;
}


bool CRNAPanel::TransferDataToWindow()
{
    bool result = wxPanel::TransferDataToWindow();

    for (size_t i = 0; i < m_Choicebook->GetPageCount(); ++i) {
        result &= m_Choicebook->GetPage(i)->TransferDataToWindow();
    }
    return result;
}


/*!
 * Should we show tooltips?
 */

bool CRNAPanel::ShowToolTips()
{
    return true;
}

/*!
 * Get bitmap resources
 */

wxBitmap CRNAPanel::GetBitmapResource( const wxString& name )
{
    // Bitmap retrieval
////@begin CRNAPanel bitmap retrieval
    wxUnusedVar(name);
    return wxNullBitmap;
////@end CRNAPanel bitmap retrieval
}

/*!
 * Get icon resources
 */

wxIcon CRNAPanel::GetIconResource( const wxString& name )
{
    // Icon retrieval
////@begin CRNAPanel icon retrieval
    wxUnusedVar(name);
    return wxNullIcon;
////@end CRNAPanel icon retrieval
}

END_NCBI_SCOPE
