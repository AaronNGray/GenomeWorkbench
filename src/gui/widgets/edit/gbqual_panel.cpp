/*  $Id: gbqual_panel.cpp 44013 2019-10-08 19:30:44Z filippov $
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
////@begin includes
////@end includes

#include <objects/seqfeat/Seq_feat.hpp>

#include <gui/widgets/edit/gbqual_panel.hpp>
#include "singlegbqual_subpanel.hpp"
#include "rpttype_panel.hpp"

#include <wx/bitmap.h>
#include <wx/icon.h>
#include <wx/scrolwin.h>
#include <wx/hyperlink.h>
#include <wx/statline.h>
#include <wx/sizer.h>
#include <wx/stattext.h>

BEGIN_NCBI_SCOPE
USING_SCOPE(objects);

////@begin XPM images
////@end XPM images


const char* CGBQualPanel::stRNA_FMet = "tRNA-fMet";
const char* CGBQualPanel::stRNA_IMet = "tRNA-iMet";
/*!
 * CGBQualPanel type definition
 */

IMPLEMENT_DYNAMIC_CLASS( CGBQualPanel, wxPanel )


/*!
 * CGBQualPanel event table definition
 */

BEGIN_EVENT_TABLE( CGBQualPanel, wxPanel )

////@begin CGBQualPanel event table entries
////@end CGBQualPanel event table entries

END_EVENT_TABLE()


/*!
 * CGBQualPanel constructors
 */

CGBQualPanel::CGBQualPanel()
{
    Init();
}

CGBQualPanel::CGBQualPanel( wxWindow* parent, CSerialObject& object,
                           wxWindowID id, const wxString& caption, const wxPoint& pos, const wxSize& size, long style )
    : m_Object(&object)
{
    Init();
    m_Object = dynamic_cast<CSeq_feat*>(&object);
    const CSeq_feat& seqfeat = dynamic_cast<const CSeq_feat&>(*m_Object);
    m_EditedFeat.Reset((CSerialObject*)CSeq_feat::GetTypeInfo()->Create());
    m_EditedFeat->Assign(seqfeat);
    Create(parent, id, caption, pos, size, style);
}


/*!
 * CGBQualPanel creator
 */

bool CGBQualPanel::Create( wxWindow* parent, wxWindowID id, const wxString& caption, const wxPoint& pos, const wxSize& size, long style )
{
////@begin CGBQualPanel creation
    SetExtraStyle(wxWS_EX_BLOCK_EVENTS);
    wxPanel::Create( parent, id, pos, size, style );

    CreateControls();
    if (GetSizer())
    {
        GetSizer()->SetSizeHints(this);
    }
    Centre();
////@end CGBQualPanel creation
    return true;
}


/*!
 * CGBQualPanel destructor
 */

CGBQualPanel::~CGBQualPanel()
{
////@begin CGBQualPanel destruction
////@end CGBQualPanel destruction
}


/*!
 * Member initialisation
 */

void CGBQualPanel::Init()
{
////@begin CGBQualPanel member initialisation
    m_ScrolledWindow = NULL;
    m_Sizer = NULL;
////@end CGBQualPanel member initialisation
}


/*!
 * Control creation for CGBQualPanel
 */


bool CGBQualPanel::PanelNeeded (CSeq_feat& feat)
{
    bool rval = false;
    
    if (feat.IsSetData() && feat.GetData().IsImp()) {
        rval = false;
    } else if (!feat.IsSetQual()) {
        rval = false;
    } else {
        CSeqFeatData::ESubtype subtype = feat.GetData().GetSubtype();    
        for (CSeq_feat::TQual::const_iterator qual = feat.GetQual().begin();
             qual != feat.GetQual().end() && !rval;
             qual++) {
            if (!AlwaysHandledElsewhere(CSeqFeatData::GetQualifierType((*qual)->GetQual()))) {
                rval = true;
            }
        }
    }
    return rval;	
}


void CGBQualPanel::CreateControls()
{    
////@begin CGBQualPanel content construction
    CGBQualPanel* itemPanel1 = this;

    wxBoxSizer* itemBoxSizer2 = new wxBoxSizer(wxVERTICAL);
    itemPanel1->SetSizer(itemBoxSizer2);
	int height = 450;
#ifdef NCBI_OS_MSWIN
	height = 300;
#endif
    m_ScrolledWindow = new wxScrolledWindow( itemPanel1, ID_SCROLLEDWINDOW, wxDefaultPosition, wxSize(-1,height), wxTAB_TRAVERSAL );
    itemBoxSizer2->Add(m_ScrolledWindow, 1, wxGROW|wxALL, 5);
    m_ScrolledWindow->SetScrollbars(1, 1, 0, 0);
    m_Sizer = new wxBoxSizer(wxVERTICAL);
    m_ScrolledWindow->SetSizer(m_Sizer);

    //itemBoxSizer2->Add(450, 0, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 0);

////@end CGBQualPanel content construction
    CSeq_feat& edited_feat = dynamic_cast<CSeq_feat&>(*m_EditedFeat);
    m_TotalHeight = 0;
    m_TotalWidth = 0;
    m_NumRows = 0;
    m_ScrollRate = 0;
    int row_height = 0;
    int row_width = 0;

    CSeqFeatData::ESubtype subtype = edited_feat.GetData().GetSubtype();

    // need to collect rpt_type qualifiers, place quals
    string parseable_rpt_types = "";

    vector<bool> placed;
    vector<CSeqFeatData::EQualifier> subtypes;
    if (edited_feat.IsSetQual()) {
        CSeq_feat::TQual::iterator qual = edited_feat.SetQual().begin();
        while (qual != edited_feat.SetQual().end()) {
            if ((*qual)->IsSetQual() && NStr::EqualNocase((*qual)->GetQual(), "rpt_type")
                && (*qual)->IsSetVal() && CRptTypePanel::IsParseable((*qual)->GetVal())) {
                if (!NStr::IsBlank(parseable_rpt_types)) {
                    parseable_rpt_types = parseable_rpt_types + ";";
                }
                parseable_rpt_types = parseable_rpt_types + (*qual)->GetVal();
                qual = edited_feat.SetQual().erase(qual);
            } else {
                placed.push_back(false);
                subtypes.push_back(CSeqFeatData::GetQualifierType((*qual)->GetQual()));
                qual++;
            }
        }
    }
    if (!NStr::IsBlank(parseable_rpt_types)) {
        CRef<CGb_qual> new_qual(new CGb_qual());
        new_qual->SetQual("rpt_type");
        new_qual->SetVal(parseable_rpt_types);
        edited_feat.SetQual().push_back(new_qual);
        placed.push_back(false);
        subtypes.push_back(CSeqFeatData::eQual_rpt_type);
    }        

    CSeqFeatData::TQualifiers mandatory = CSeqFeatData::GetMandatoryQualifiers(subtype);
    // list mandatory qualifiers first
    ITERATE (CSeqFeatData::TQualifiers, required, mandatory) {  
        if (AlwaysHandledElsewhere(*required)) {
            continue;
        }
        bool have_any = false;
        // do we have any of this type?
        if (edited_feat.IsSetQual()) {
            int pos = 0;
            for (CSeq_feat::TQual::iterator qual = edited_feat.SetQual().begin();
                 qual != edited_feat.SetQual().end();
                 qual++, pos++) {
                if (!placed[pos] && subtypes[pos] == *required) {
                    x_AddRow (*qual);
                    placed[pos] = true;
                    have_any = true;
                }
            }
        }
        if (!have_any && !PreferredHandledElsewhere(subtype, *required)) {
            CRef<CGb_qual> new_qual(new CGb_qual());
            new_qual->SetQual(CSeqFeatData::GetQualifierAsString(*required));
            edited_feat.SetQual().push_back(new_qual);
            placed.push_back(true);
            subtypes.push_back(*required);
            x_AddRow (new_qual);
        }
    }

    // list optional qualifiers next
    for (auto required : CSeqFeatData::GetLegalQualifiers(subtype)) {
        if (AlwaysHandledElsewhere(required)) {
			continue;
		}
        // if mandatory, was already listed
        bool is_mandatory = false;
        ITERATE(CSeqFeatData::TQualifiers, m, mandatory) {
            if (*m == required) {
                is_mandatory = true;
                break;
            }
        }
        if (is_mandatory) {
            continue;
        }
        bool have_any = false;
        // do we have any of this type?
        if (edited_feat.IsSetQual()) {
            int pos = 0;
            for (CSeq_feat::TQual::iterator qual = edited_feat.SetQual().begin();
                 qual != edited_feat.SetQual().end();
                 qual++, pos++) {
                if (!placed[pos] && subtypes[pos] == required) {
                    x_AddRow (*qual);
                    placed[pos] = true;
                    have_any = true;
                }
            }
        }
        if (!have_any && !PreferredHandledElsewhere(subtype, required)) {
            string key = CSeqFeatData::GetQualifierAsString(required);
            if (!NStr::IsBlank(key)) {
                CRef<CGb_qual> new_qual(new CGb_qual());
                new_qual->SetQual(CSeqFeatData::GetQualifierAsString(required));
                edited_feat.SetQual().push_back(new_qual);
                placed.push_back(true);
                subtypes.push_back(required);
                x_AddRow (new_qual);
            }
        }
    }

    bool any_unplaced = false;
    int i = 0;
    for (CSeq_feat::TQual::iterator qual = edited_feat.SetQual().begin();
         qual != edited_feat.SetQual().end();
         qual++, i++) {
		 if (!placed[i] && !AlwaysHandledElsewhere(CSeqFeatData::GetQualifierType((*qual)->GetQual())) ) {
            any_unplaced = true;
            break;
        }
    }

    if (any_unplaced) {
        wxStaticLine* itemStaticLine = new wxStaticLine( m_ScrolledWindow, wxID_STATIC, wxDefaultPosition, wxDefaultSize, wxLI_HORIZONTAL );
        m_Sizer->Add(itemStaticLine, 0, wxGROW|wxALL, 5);
        itemStaticLine->GetClientSize(&row_width, &row_height);
        m_TotalHeight += row_height;

        // then list unplaced qualifiers
        int pos = 0;
        for (CSeq_feat::TQual::iterator qual = edited_feat.SetQual().begin();
             qual != edited_feat.SetQual().end();
             qual++, pos++) {
            if (!placed[pos] && !AlwaysHandledElsewhere(CSeqFeatData::GetQualifierType((*qual)->GetQual()))) {
                x_AddRow (*qual);
                placed[pos] = true;
            }
        }
    }

    bool any_illegal = false;
    i = 0;
    for (CSeq_feat::TQual::iterator qual = edited_feat.SetQual().begin();
         qual != edited_feat.SetQual().end();
         qual++, i++) {
        if (!placed[i]) {
            any_illegal = true;
            break;
        }
    }

    if (any_illegal) {
        wxStaticLine* itemStaticLine = new wxStaticLine( m_ScrolledWindow, wxID_STATIC, wxDefaultPosition, wxDefaultSize, wxLI_HORIZONTAL );
        m_Sizer->Add(itemStaticLine, 0, wxGROW|wxALL, 5);
        itemStaticLine->GetClientSize(&row_width, &row_height);
        m_TotalHeight += row_height;

        // then list illegal qualifiers
        int pos = 0;
        for (CSeq_feat::TQual::iterator qual = edited_feat.SetQual().begin();
             qual != edited_feat.SetQual().end();
             qual++, pos++) {
            if (!placed[pos]) {
                x_AddRow (*qual);
                placed[pos] = true;
            }
        }
    }

    //m_ScrolledWindow->SetVirtualSize(m_TotalWidth + 10, m_TotalHeight);
    m_ScrolledWindow->FitInside();
    m_ScrolledWindow->SetScrollRate(0, m_ScrollRate);   
}


void CGBQualPanel::AdjustSize()
{
	m_TotalHeight = 0;
	m_TotalWidth = 0;
	m_ScrollRate = 0;
	m_NumRows = 0;
	wxSizerItemList::iterator node = m_Sizer->GetChildren().begin();
    for (; node != m_Sizer->GetChildren().end(); ++node) {
        wxWindow *row = (*node)->GetWindow();
        if (!row) {
            continue;
        }
		int row_width;
		int row_height;
		row->GetClientSize(&row_width, &row_height);
		if (row_width > m_TotalWidth) {
			m_TotalWidth = row_width;
		}
		if (row_height < m_ScrollRate || m_ScrollRate == 0) {
			m_ScrollRate = row_height;
		}
		m_TotalHeight += row_height;
		m_NumRows++;
    }

    //m_ScrolledWindow->SetVirtualSize(m_TotalWidth + 10, m_TotalHeight);
    m_ScrolledWindow->FitInside();
    m_ScrolledWindow->SetScrollRate(0, m_ScrollRate);
}


void CGBQualPanel::x_AddRow(CRef<CGb_qual> qual)
{
    CSingleGbQualSubPanel* row = new CSingleGbQualSubPanel(m_ScrolledWindow, *qual);
    row->TransferDataToWindow();
    m_Sizer->Add(row, 0, wxALIGN_LEFT|wxALL, 0);
    int row_width;
    int row_height;
    row->GetClientSize(&row_width, &row_height);
    if (row_width > m_TotalWidth) {
        m_TotalWidth = row_width;
    }
    m_TotalHeight += row_height;
    m_ScrollRate = row_height;
    m_NumRows++;
}

void CGBQualPanel::Update_tRNA_ProductQual(const string& value)
{
    bool found_product = false;
    wxSizerItemList::iterator node = m_Sizer->GetChildren().begin();
    for (; node != m_Sizer->GetChildren().end(); ++node) {
        wxWindow *w = (*node)->GetWindow();
        if (!w) {
            continue;
        }
        CSingleGbQualSubPanel* singlequal_panel = dynamic_cast<CSingleGbQualSubPanel*> (w);
        if (singlequal_panel) {
            CRef<CGb_qual> qual = singlequal_panel->GetGbQual();
            if (!qual->IsSetQual() || !NStr::EqualNocase(qual->GetQual(), "product")) {
                continue;
            }

            found_product = true;
            if ((NStr::EqualNocase(qual->GetVal(), stRNA_FMet) && (NStr::EqualNocase(value, stRNA_IMet) || value.empty()))
                || (NStr::EqualNocase(qual->GetVal(), stRNA_IMet) && (NStr::EqualNocase(value, stRNA_FMet) || value.empty()))) {
                qual->SetVal(value);
                singlequal_panel->SetGbQual(qual);
                break;
            }

        }
    }

    if (!found_product && !value.empty()) {
        // another row is needed
        CRef<CGb_qual> new_qual(new CGb_qual("product", value));
        x_AddRow(new_qual);
    }
}


bool CGBQualPanel::TransferDataToWindow()
{
    if (!wxPanel::TransferDataToWindow())
        return false;

    wxSizerItemList::iterator node = m_Sizer->GetChildren().begin();
    for (; node != m_Sizer->GetChildren().end(); ++node) {
        wxWindow *w = (*node)->GetWindow();
        if (!w) {
            continue;
        }
        CSingleGbQualSubPanel* singlequal_panel = dynamic_cast<CSingleGbQualSubPanel*> (w);
        if (singlequal_panel) {
            singlequal_panel->TransferDataToWindow();
        }
    }

    return true;
}


bool CGBQualPanel::TransferDataFromWindow()
{
    if (!wxPanel::TransferDataFromWindow())
        return false;

    CSeq_feat& edited_feat = dynamic_cast<CSeq_feat&>(*m_EditedFeat);
    edited_feat.ResetQual();

    wxSizerItemList::iterator node = m_Sizer->GetChildren().begin();
    for (; node != m_Sizer->GetChildren().end(); ++node) {
        wxWindow *w = (*node)->GetWindow();
        if (!w) {
            continue;
        }
        CRef<CGb_qual> edited_qual;
        CSingleGbQualSubPanel* singlequal_panel = dynamic_cast<CSingleGbQualSubPanel*> (w);
        if (singlequal_panel) {
            edited_qual = singlequal_panel->GetGbQual();
            if (edited_qual && edited_qual->IsSetVal() && (!NStr::IsBlank(edited_qual->GetVal()) || NStr::Equal(edited_qual->GetQual(), "replace")))
            {
                edited_feat.SetQual().push_back(edited_qual);
            }
        }
    }
    return true;
}


void CGBQualPanel::PopulateGBQuals(objects::CSeq_feat& seq_feat) 
{
    TransferDataFromWindow();
    CSeq_feat& edited_feat = dynamic_cast<CSeq_feat&>(*m_EditedFeat);
    seq_feat.ResetQual();

    for (CSeq_feat::TQual::iterator qual = edited_feat.SetQual().begin();
         qual != edited_feat.SetQual().end();
         qual++) {
        if ((*qual)->IsSetQual() && !NStr::IsBlank((*qual)->GetQual())
            && !AlwaysHandledElsewhere(CSeqFeatData::GetQualifierType((*qual)->GetQual()))
            && (*qual)->IsSetVal() && 
            (!NStr::IsBlank((*qual)->GetVal()) || NStr::Equal((*qual)->GetQual(), "replace"))) {
            if (NStr::EqualNocase((*qual)->GetQual(), "rpt_type")
                && CRptTypePanel::IsParseable((*qual)->GetVal())) {
                // add rpt_types back individually
                vector<string> tokens;
                NStr::Split((*qual)->GetVal(), ";", tokens);
                ITERATE (vector<string>, tok, tokens) {
                    CRef<CGb_qual> new_qual(new CGb_qual());
                    new_qual->SetQual("rpt_type");
                    new_qual->SetVal(*tok);
                    seq_feat.SetQual().push_back(new_qual);
                }                    
            } else {                
                CRef<CGb_qual> new_qual(new CGb_qual());
                new_qual->Assign(**qual);
                seq_feat.SetQual().push_back(new_qual);
            }
        }
    }
}


/*!
 * Should we show tooltips?
 */

bool CGBQualPanel::ShowToolTips()
{
    return true;
}

/*!
 * Get bitmap resources
 */

wxBitmap CGBQualPanel::GetBitmapResource( const wxString& name )
{
    // Bitmap retrieval
////@begin CGBQualPanel bitmap retrieval
    wxUnusedVar(name);
    return wxNullBitmap;
////@end CGBQualPanel bitmap retrieval
}

/*!
 * Get icon resources
 */

wxIcon CGBQualPanel::GetIconResource( const wxString& name )
{
    // Icon retrieval
////@begin CGBQualPanel icon retrieval
    wxUnusedVar(name);
    return wxNullIcon;
////@end CGBQualPanel icon retrieval
}


bool AlwaysHandledElsewhere(CSeqFeatData::EQualifier qual_type)
{
    // experiment and inference get their own panels
    if (qual_type == CSeqFeatData::eQual_experiment || qual_type == CSeqFeatData::eQual_inference) {
	return true;
    }
    // pseudo and pseudogene are handled separately
    if (qual_type == CSeqFeatData::eQual_pseudogene || qual_type == CSeqFeatData::eQual_pseudo) {
        return true;
    }
    return false;
}

bool PreferredHandledElsewhere(CSeqFeatData::ESubtype feat_subtype, CSeqFeatData::EQualifier qual_type) 
{
    if (qual_type == CSeqFeatData::eQual_note) {
        return true;
    }
    if (qual_type == CSeqFeatData::eQual_citation
        || qual_type == CSeqFeatData::eQual_db_xref
        || qual_type == CSeqFeatData::eQual_exception
        || qual_type == CSeqFeatData::eQual_gene
        || qual_type == CSeqFeatData::eQual_gene_synonym
        || qual_type == CSeqFeatData::eQual_insertion_seq
        || qual_type == CSeqFeatData::eQual_label
        || qual_type == CSeqFeatData::eQual_locus_tag
        || qual_type == CSeqFeatData::eQual_note
        || qual_type == CSeqFeatData::eQual_partial
        || qual_type == CSeqFeatData::eQual_product
        || qual_type == CSeqFeatData::eQual_pseudo
        || qual_type == CSeqFeatData::eQual_rpt_unit
        || qual_type == CSeqFeatData::eQual_transposon
        || qual_type == CSeqFeatData::eQual_trans_splicing
        || qual_type == CSeqFeatData::eQual_ribosomal_slippage
        || qual_type == CSeqFeatData::eQual_standard_name
        || qual_type == CSeqFeatData::eQual_usedin) {
        return true;
    }

    if (qual_type == CSeqFeatData::eQual_product) {
        if (feat_subtype == CSeqFeatData::eSubtype_mat_peptide
            || feat_subtype == CSeqFeatData::eSubtype_sig_peptide
            || feat_subtype == CSeqFeatData::eSubtype_transit_peptide
            || feat_subtype == CSeqFeatData::eSubtype_C_region
            || feat_subtype == CSeqFeatData::eSubtype_D_segment
            || feat_subtype == CSeqFeatData::eSubtype_exon
            || feat_subtype == CSeqFeatData::eSubtype_J_segment
            || feat_subtype == CSeqFeatData::eSubtype_misc_feature
            || feat_subtype == CSeqFeatData::eSubtype_N_region
            || feat_subtype == CSeqFeatData::eSubtype_S_region
            || feat_subtype == CSeqFeatData::eSubtype_V_region
            || feat_subtype == CSeqFeatData::eSubtype_V_segment
            || feat_subtype == CSeqFeatData::eSubtype_variation) {
            return false;
        } else {
            return true;
        }
    }

    if (feat_subtype == CSeqFeatData::eSubtype_gene)
    {
        if (  qual_type == CSeqFeatData::eQual_allele ||
              qual_type == CSeqFeatData::eQual_gene_synonym ||
              qual_type == CSeqFeatData::eQual_locus_tag ||
              qual_type == CSeqFeatData::eQual_map
            )
            return true;
        else
            return false;
    }


    if (feat_subtype == CSeqFeatData::eSubtype_cdregion) {
        if (qual_type == CSeqFeatData::eQual_codon_start
            || qual_type == CSeqFeatData::eQual_codon
            || qual_type == CSeqFeatData::eQual_EC_number
            || qual_type == CSeqFeatData::eQual_gdb_xref
            || qual_type == CSeqFeatData::eQual_number
            || qual_type == CSeqFeatData::eQual_protein_id
            || qual_type == CSeqFeatData::eQual_transl_except
            || qual_type == CSeqFeatData::eQual_transl_table
            || qual_type == CSeqFeatData::eQual_translation
            || qual_type == CSeqFeatData::eQual_allele
            || qual_type == CSeqFeatData::eQual_translation
            || qual_type == CSeqFeatData::eQual_function
            || qual_type == CSeqFeatData::eQual_old_locus_tag) {
            return true;
        }
    }

   

    if (qual_type == CSeqFeatData::eQual_map 
        && feat_subtype != CSeqFeatData::eSubtype_repeat_region 
        && feat_subtype != CSeqFeatData::eSubtype_gap) {
      return true;
    }
    if (qual_type == CSeqFeatData::eQual_operon
        && feat_subtype != CSeqFeatData::eSubtype_operon) {
        return true;
    }
    return false;
}


END_NCBI_SCOPE
