/*  $Id: bulk_other_add_panel.cpp 42430 2019-02-22 13:19:16Z filippov $
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
#include <objects/seqfeat/Seq_feat.hpp>
#include <objects/seqfeat/SeqFeatData.hpp>
#include <objects/seqfeat/Imp_feat.hpp>
#include <objmgr/bioseq_ci.hpp>

////@begin includes
////@end includes

#include <gui/packages/pkg_sequence_edit/bulk_other_add_panel.hpp>
#include <gui/packages/pkg_sequence_edit/subprep_util.hpp>
//#include <gui/packages/pkg_sequence_edit/featedit_util.hpp>
#include <gui/objutils/cmd_create_feat.hpp>
#include <gui/objutils/utils.hpp>
#include <gui/widgets/edit/feature_type_panel.hpp>
#include <gui/widgets/wx/wx_utils.hpp>
#include <wx/stattext.h>

////@begin XPM images
////@end XPM images

BEGIN_NCBI_SCOPE

USING_SCOPE(ncbi::objects);

/*!
 * CBulkOtherAddPanel type definition
 */

IMPLEMENT_DYNAMIC_CLASS( CBulkOtherAddPanel, CBulkCmdPanel )


/*!
 * CBulkOtherAddPanel event table definition
 */

BEGIN_EVENT_TABLE( CBulkOtherAddPanel, CBulkCmdPanel )

////@begin CBulkOtherAddPanel event table entries
    EVT_LISTBOX( ID_FEATURE_TYPE_LISTBOX, CBulkOtherAddPanel::OnFeatureTypeListboxSelected )

////@end CBulkOtherAddPanel event table entries

END_EVENT_TABLE()


/*!
 * CBulkOtherAddPanel constructors
 */

CBulkOtherAddPanel::CBulkOtherAddPanel()
{
    Init();
}

CBulkOtherAddPanel::CBulkOtherAddPanel( wxWindow* parent, CSeq_entry_Handle seh, string key, bool add_location_panel, wxWindowID id, const wxPoint& pos, const wxSize& size, long style )
    : m_TopSeqEntry(seh), m_DefaultKey(key), CBulkCmdPanel(add_location_panel)
{
    Init();
    Create(parent, id, pos, size, style);
}


/*!
 * CBulkOtherAddPanel creator
 */

bool CBulkOtherAddPanel::Create( wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style )
{
////@begin CBulkOtherAddPanel creation
    CBulkCmdPanel::Create( parent, id, pos, size, style );

    CreateControls();
    if (GetSizer())
    {
        GetSizer()->SetSizeHints(this);
    }
    Centre();
////@end CBulkOtherAddPanel creation
    return true;
}


/*!
 * CBulkOtherAddPanel destructor
 */

CBulkOtherAddPanel::~CBulkOtherAddPanel()
{
////@begin CBulkOtherAddPanel destruction
////@end CBulkOtherAddPanel destruction
}


/*!
 * Member initialisation
 */

void CBulkOtherAddPanel::Init()
{
////@begin CBulkOtherAddPanel member initialisation
    m_FeatureType = NULL;
    m_GBQualSizer = NULL;
    m_Locus = NULL;
    m_GeneDesc = NULL;
    m_Comment = NULL;
    m_Location = NULL;
////@end CBulkOtherAddPanel member initialisation
    m_GBQualPanel = NULL;
    m_QualFeat.Reset(new CSeq_feat());
    m_QualFeat->SetData().SetImp().SetKey("misc_feature");
}


/*!
 * Control creation for CBulkOtherAddPanel
 */

void CBulkOtherAddPanel::CreateControls()
{    
////@begin CBulkOtherAddPanel content construction
    CBulkOtherAddPanel* itemCBulkCmdPanel1 = this;

    wxBoxSizer* itemBoxSizer2 = new wxBoxSizer(wxVERTICAL);
    itemCBulkCmdPanel1->SetSizer(itemBoxSizer2);

    wxBoxSizer* itemBoxSizer3 = new wxBoxSizer(wxHORIZONTAL);
    itemBoxSizer2->Add(itemBoxSizer3, 0, wxALIGN_LEFT|wxALL, 5);

    wxBoxSizer* itemBoxSizer4 = new wxBoxSizer(wxVERTICAL);
    itemBoxSizer3->Add(itemBoxSizer4, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxStaticText* itemStaticText5 = new wxStaticText( itemCBulkCmdPanel1, wxID_STATIC, _("Feature Type"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer4->Add(itemStaticText5, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

    wxArrayString m_FeatureTypeStrings;
    m_FeatureType = new wxListBox( itemCBulkCmdPanel1, ID_FEATURE_TYPE_LISTBOX, wxDefaultPosition, wxDefaultSize, m_FeatureTypeStrings, wxLB_SINGLE );
    itemBoxSizer4->Add(m_FeatureType, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

    m_GBQualSizer = new wxBoxSizer(wxVERTICAL);
    itemBoxSizer3->Add(m_GBQualSizer, 0, wxGROW|wxALL, 5);

    wxFlexGridSizer* itemFlexGridSizer8 = new wxFlexGridSizer(0, 2, 0, 0);
    itemBoxSizer2->Add(itemFlexGridSizer8, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

    wxStaticText* itemStaticText9 = new wxStaticText( itemCBulkCmdPanel1, wxID_STATIC, _("Gene Symbol"), wxDefaultPosition, wxDefaultSize, 0 );
    itemFlexGridSizer8->Add(itemStaticText9, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    m_Locus = new wxTextCtrl( itemCBulkCmdPanel1, ID_TEXTCTRL11, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
    itemFlexGridSizer8->Add(m_Locus, 0, wxGROW|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxStaticText* itemStaticText11 = new wxStaticText( itemCBulkCmdPanel1, wxID_STATIC, _("Gene Description"), wxDefaultPosition, wxDefaultSize, 0 );
    itemFlexGridSizer8->Add(itemStaticText11, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    m_GeneDesc = new wxTextCtrl( itemCBulkCmdPanel1, ID_TEXTCTRL12, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
    itemFlexGridSizer8->Add(m_GeneDesc, 0, wxGROW|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxStaticText* itemStaticText13 = new wxStaticText( itemCBulkCmdPanel1, wxID_STATIC, _("Comment"), wxDefaultPosition, wxDefaultSize, 0 );
    itemFlexGridSizer8->Add(itemStaticText13, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    m_Comment = new wxTextCtrl( itemCBulkCmdPanel1, ID_TEXTCTRL14, wxEmptyString, wxDefaultPosition, wxSize(300, 70), wxTE_MULTILINE );
    itemFlexGridSizer8->Add(m_Comment, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    if (m_add_location_panel)
    {
        m_Location = new CBulkLocationPanel( itemCBulkCmdPanel1, ID_WINDOW1, wxDefaultPosition, wxSize(100, 100), wxSIMPLE_BORDER );
        itemBoxSizer2->Add(m_Location, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);
    }

    itemBoxSizer2->Add(500, 0, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 0);

////@end CBulkOtherAddPanel content construction

    x_ListFeatures();

    m_GBQualPanel = new CGBQualPanel (this, *m_QualFeat);
    m_GBQualSizer->Add(m_GBQualPanel, 1, wxGROW|wxALL, 0);
    x_ChangeFeatureType("Gene");

}

static bool s_IsRNAFeature(int feat_subtype)
{
    if (feat_subtype == CSeqFeatData::eSubtype_preRNA 
        || feat_subtype == CSeqFeatData::eSubtype_mRNA
        || feat_subtype == CSeqFeatData::eSubtype_tRNA
        || feat_subtype == CSeqFeatData::eSubtype_rRNA
        || feat_subtype == CSeqFeatData::eSubtype_otherRNA
        || feat_subtype == CSeqFeatData::eSubtype_misc_RNA
        || feat_subtype == CSeqFeatData::eSubtype_precursor_RNA
        || feat_subtype == CSeqFeatData::eSubtype_ncRNA
        || feat_subtype == CSeqFeatData::eSubtype_tmRNA) {
        return true;
    }
    return false;
}

void CBulkOtherAddPanel::x_ListFeatures()
{
    vector<string> listed_feat;
    set<string> existing;
    map<pair<int,int>,string> types;
    
    listed_feat.push_back("Gene");
    listed_feat.push_back("misc_feature");
    existing.insert(listed_feat[0]);
    existing.insert(listed_feat[1]);
    
    vector<const CFeatListItem *> featlist = GetSortedFeatList(m_TopSeqEntry);
    ITERATE(vector<const CFeatListItem *>, feat_it, featlist) {
        const CFeatListItem& item = **feat_it;
        string desc = item.GetDescription();
        int feat_type = item.GetType();
        CSeqFeatData::ESubtype feat_subtype = (CSeqFeatData::ESubtype)item.GetSubtype();
        if (feat_type == CSeqFeatData::e_Imp
            && !CFeatureTypePanel::s_IsRarelyUsedOrDiscouragedFeatureType(feat_subtype)
            && !CSeqFeatData::IsRegulatory(feat_subtype)) {
            types[pair<int,int>(feat_type,feat_subtype)] = desc;
            if (existing.find(desc) == existing.end()) {
                existing.insert(desc);
                listed_feat.push_back(desc);
            }
        }
    }

    m_FeatureType->Clear();
    for (size_t i = 0; i < listed_feat.size(); ++i) {
        m_FeatureType->Append(ToWxString(listed_feat[i]));
    }

    if (find(listed_feat.begin(), listed_feat.end(), m_DefaultKey) != listed_feat.end()) {
        m_FeatureType->SetStringSelection(ToWxString(m_DefaultKey));
        m_QualFeat->SetData().SetImp().SetKey(m_DefaultKey);
    }

}

/*!
 * Should we show tooltips?
 */

bool CBulkOtherAddPanel::ShowToolTips()
{
    return true;
}

/*!
 * Get bitmap resources
 */

wxBitmap CBulkOtherAddPanel::GetBitmapResource( const wxString& name )
{
    // Bitmap retrieval
////@begin CBulkOtherAddPanel bitmap retrieval
    wxUnusedVar(name);
    return wxNullBitmap;
////@end CBulkOtherAddPanel bitmap retrieval
}

/*!
 * Get icon resources
 */

wxIcon CBulkOtherAddPanel::GetIconResource( const wxString& name )
{
    // Icon retrieval
////@begin CBulkOtherAddPanel icon retrieval
    wxUnusedVar(name);
    return wxNullIcon;
////@end CBulkOtherAddPanel icon retrieval
}

CRef<CCmdComposite> CBulkOtherAddPanel::GetCommand()
{
    string key = ToStdString(m_FeatureType->GetStringSelection());
   
    if (NStr::IsBlank(key)) {
        CRef<CCmdComposite> empty;
        return empty;
    }
    CRef<CCmdComposite> cmd (new CCmdComposite("Bulk Add Other Feature"));

    objects::CBioseq_CI b_iter(m_TopSeqEntry, objects::CSeq_inst::eMol_na);
    for ( ; b_iter ; ++b_iter ) {
        AddOneCommand(*b_iter, cmd);
    }

    return cmd;
}

void CBulkOtherAddPanel::AddOneCommand(const CBioseq_Handle& bsh, CRef<CCmdComposite> cmd)
{

    string key = ToStdString(m_FeatureType->GetStringSelection());
    string comment = ToStdString(m_Comment->GetValue());
    string gene_symbol = ToStdString(m_Locus->GetValue());
    string gene_desc = ToStdString(m_GeneDesc->GetValue());
    m_GBQualPanel->TransferDataFromWindow();
    m_GBQualPanel->PopulateGBQuals(*m_QualFeat);

    objects::CSeq_entry_Handle seh = bsh.GetSeq_entry_Handle();
    if (!ShouldAddToExisting() && AlreadyHasFeature(bsh, key, comment)) {
        return;
    }
    CRef<objects::CSeq_loc> loc = GetLocForBioseq(*(bsh.GetCompleteBioseq()));
    if (!loc) {
        return;
    }
    if (!NStr::EqualNocase(key, "Gene")) {
        CRef<objects::CSeq_feat> feat(new objects::CSeq_feat());
        feat->SetData().SetImp().SetKey(key);
        if (!NStr::IsBlank(comment)) {
            feat->SetComment(comment);
        }
        feat->SetLocation().Assign(*loc);
        if (feat->GetLocation().IsPartialStart(objects::eExtreme_Biological)
            || feat->GetLocation().IsPartialStop(objects::eExtreme_Biological)) {
            feat->SetPartial(true);
        }
        x_AddQuals(*feat);
        cmd->AddCommand(*CRef<CCmdCreateFeat>(new CCmdCreateFeat(seh, *feat)));
    }
    if (!NStr::IsBlank(gene_symbol) || !NStr::IsBlank(gene_desc) || NStr::EqualNocase(key, "Gene")) {
        // create gene feature
        CRef<objects::CSeq_feat> new_gene(new objects::CSeq_feat());
        new_gene->SetData().SetGene().SetLocus(gene_symbol);
        if (!NStr::IsBlank(gene_desc)) {
            new_gene->SetData().SetGene().SetDesc(gene_desc);
        }
        if (NStr::EqualNocase(key, "Gene") && !NStr::IsBlank(comment)) {
            new_gene->SetComment(comment);
            }
        new_gene->SetLocation().Assign(*loc);
        if (new_gene->GetLocation().IsPartialStart(objects::eExtreme_Biological)
            || new_gene->GetLocation().IsPartialStop(objects::eExtreme_Biological)) {
            new_gene->SetPartial(true);
        }
        x_AddGeneQuals(*new_gene);
        cmd->AddCommand(*CRef<CCmdCreateFeat>(new CCmdCreateFeat(seh, *new_gene)));
    }
}

string CBulkOtherAddPanel::GetErrorMessage()
{
    string key = ToStdString(m_FeatureType->GetStringSelection());
    if (NStr::IsBlank(key)) {
        return "You must select feature type!";
    }
    objects::CBioseq_CI b_iter(m_TopSeqEntry, objects::CSeq_inst::eMol_na);
    for ( ; b_iter ; ++b_iter ) {
        objects::CSeq_entry_Handle seh = b_iter->GetSeq_entry_Handle();
        CRef<objects::CSeq_loc> loc = GetLocForBioseq(*(b_iter->GetCompleteBioseq()));
        if (!loc) {
            return "You must specify valid location values!";
        }
    }
    return "Unknown error";
}


void CBulkOtherAddPanel::x_ChangeFeatureType(const string& key)
{
    m_GBQualPanel->TransferDataFromWindow();
    m_GBQualPanel->PopulateGBQuals(*m_QualFeat);
    if (NStr::EqualNocase(key, "Gene")) {
        m_QualFeat->SetData().SetGene();
        m_QualFeat->ResetQual();
    } else {
        m_QualFeat->SetData().SetImp().SetKey(key);
    }
    while (m_GBQualSizer->GetItemCount() > 0) {
        size_t pos = 0;
        m_GBQualSizer->GetItem(pos)->DeleteWindows();
        m_GBQualSizer->Remove(pos);
    }
    m_GBQualPanel = new CGBQualPanel (this, *m_QualFeat);
    m_GBQualSizer->Add(m_GBQualPanel, 1, wxGROW|wxALL, 0);
    m_GBQualPanel->TransferDataToWindow();
//    if (NStr::EqualNocase(key, "Gene")) {
//        m_GBQualPanel->Show(false);
//    }
    Layout();
}


/*!
 * wxEVT_COMMAND_LISTBOX_SELECTED event handler for ID_FEATURE_TYPE_LISTBOX
 */

void CBulkOtherAddPanel::OnFeatureTypeListboxSelected( wxCommandEvent& event )
{
    string key = ToStdString(m_FeatureType->GetStringSelection());
    x_ChangeFeatureType(key);
}


void CBulkOtherAddPanel::x_AddQuals(objects::CSeq_feat& feat)
{
    if (m_QualFeat && m_QualFeat->IsSetQual()) {
        ITERATE(CSeq_feat::TQual, it, m_QualFeat->GetQual()) {
            CRef<CGb_qual> q(new CGb_qual());
            q->Assign(**it);
            feat.SetQual().push_back(q);

            if ( NStr::EqualNocase(q->GetQual(), "mobile_element_type") && feat.IsSetData() && feat.GetData().IsImp()
                       && feat.GetData().GetImp().IsSetKey() && feat.GetData().GetImp().GetKey() == "repeat_region" )
                {
                    feat.SetData().SetImp().SetKey("mobile_element");
                }
        }
    }
}

void CBulkOtherAddPanel::x_AddGeneQuals(objects::CSeq_feat& gene)
{
    if (m_QualFeat && m_QualFeat->IsSetQual()) 
    {
        ITERATE(CSeq_feat::TQual, it, m_QualFeat->GetQual()) 
        {
            string val = (*it)->GetVal();
            string qual = (*it)->GetQual();
            if (qual == "locus")
                gene.SetData().SetGene().SetLocus(val);
            if (qual == "locus_tag")
                gene.SetData().SetGene().SetLocus_tag(val);
            if (qual == "allele")
                gene.SetData().SetGene().SetAllele(val);
            if (qual == "map")
                gene.SetData().SetGene().SetMaploc(val);
            if (qual == "gene_synonym")
                gene.SetData().SetGene().SetSyn().push_back(val);
        }
    }
}


END_NCBI_SCOPE

