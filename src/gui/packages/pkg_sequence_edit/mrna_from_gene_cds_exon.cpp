/*  $Id: mrna_from_gene_cds_exon.cpp 42061 2018-12-11 17:28:43Z filippov $
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
 * Authors:  Igor Filippov
 */


#include <ncbi_pch.hpp>

////@begin includes
////@end includes

#include <wx/stattext.h>
#include <wx/msgdlg.h>
#include <wx/statbox.h>

#include <objects/seqfeat/RNA_ref.hpp>
#include <objects/seqfeat/Imp_feat.hpp>
#include <objmgr/util/seq_loc_util.hpp>
#include <objmgr/seq_vector.hpp>
#include <objmgr/util/sequence.hpp>
#include <objtools/edit/cds_fix.hpp>
#include <objtools/edit/loc_edit.hpp>
#include <gui/objutils/cmd_change_seq_feat.hpp>
#include <gui/objutils/cmd_create_feat.hpp>
#include <gui/core/selection_service_impl.hpp>
#include <gui/widgets/edit/feature_field_name_panel.hpp>
#include <gui/packages/pkg_sequence_edit/mrna_from_gene_cds_exon.hpp>

////@begin XPM images
////@end XPM images

BEGIN_NCBI_SCOPE

USING_SCOPE(ncbi::objects);

/*!
 * CmRNAFromGeneCdsExon type definition
 */

IMPLEMENT_DYNAMIC_CLASS( CmRNAFromGeneCdsExon, CBulkCmdDlg )


/*!
 * CmRNAFromGeneCdsExon event table definition
 */

BEGIN_EVENT_TABLE( CmRNAFromGeneCdsExon, CBulkCmdDlg )

////@begin CmRNAFromGeneCdsExon event table entries
   
////@end CmRNAFromGeneCdsExon event table entries
EVT_UPDATE_FEATURE_LIST(wxID_ANY, CmRNAFromGeneCdsExon::ProcessUpdateFeatEvent )
END_EVENT_TABLE()


/*!
 * CmRNAFromGeneCdsExon constructors
 */

CmRNAFromGeneCdsExon::CmRNAFromGeneCdsExon()
{
    Init();
}

CmRNAFromGeneCdsExon::CmRNAFromGeneCdsExon( wxWindow* parent, IWorkbench* wb, wxWindowID id, const wxString& caption, const wxPoint& pos, const wxSize& size, long style )
  : CBulkCmdDlg(wb)
{
    Init();
    Create(parent, id, caption, pos, size, style);
}


/*!
 * CmRNAFromGeneCdsExon creator
 */

bool CmRNAFromGeneCdsExon::Create( wxWindow* parent, wxWindowID id, const wxString& caption, const wxPoint& pos, const wxSize& size, long style )
{
////@begin CmRNAFromGeneCdsExon creation
    SetExtraStyle(wxWS_EX_BLOCK_EVENTS);
    CBulkCmdDlg::Create( parent, id, caption, pos, size, style );

    CreateControls();
    if (GetSizer())
    {
        GetSizer()->SetSizeHints(this);
    }
    Centre();
////@end CmRNAFromGeneCdsExon creation
    return true;
}


/*!
 * CmRNAFromGeneCdsExon destructor
 */

CmRNAFromGeneCdsExon::~CmRNAFromGeneCdsExon()
{
////@begin CmRNAFromGeneCdsExon destruction
////@end CmRNAFromGeneCdsExon destruction
}


/*!
 * Member initialisation
 */

void CmRNAFromGeneCdsExon::Init()
{
////@begin CmRNAFromGeneCdsExon member initialisation
    m_FeatureType = NULL;
    m_OkCancel = NULL;
////@end CmRNAFromGeneCdsExon member initialisation
    m_ErrorMessage = "";
    m_SingleInterval = NULL;
    m_StringConstraintPanel = NULL;
}


/*!
 * Control creation for CmRNAFromGeneCdsExon
 */

void CmRNAFromGeneCdsExon::CreateControls()
{    
////@begin CmRNAFromGeneCdsExon content construction
    CmRNAFromGeneCdsExon* itemCBulkCmdDlg1 = this;

    wxBoxSizer* itemBoxSizer2 = new wxBoxSizer(wxVERTICAL);
    itemCBulkCmdDlg1->SetSizer(itemBoxSizer2);


    GetTopLevelSeqEntryAndProcessor();

    wxArrayString feat_strings;
    feat_strings.Add(_("Gene"));
    feat_strings.Add(_("CDS"));
    feat_strings.Add(_("exon"));
    feat_strings.Add(_("5'UTR"));
    feat_strings.Add(_("3'UTR"));
    m_FeatureType = new wxChoice( itemCBulkCmdDlg1, wxID_ANY, wxDefaultPosition, wxDefaultSize, feat_strings, 0 );
    m_FeatureType->SetSelection(0);
    itemBoxSizer2->Add(m_FeatureType, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);
    m_FeatureType->Bind(wxEVT_CHOICE, &CmRNAFromGeneCdsExon::OnSelectFeature, this);

    wxStaticBox* itemStaticBoxSizer7Static = new wxStaticBox(itemCBulkCmdDlg1, wxID_ANY, _("Source for new mRNA name"));
    wxStaticBoxSizer* itemBoxSizer7 = new wxStaticBoxSizer(itemStaticBoxSizer7Static, wxVERTICAL);
    itemBoxSizer2->Add(itemBoxSizer7, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

    wxBoxSizer* itemBoxSizer8 = new wxBoxSizer(wxHORIZONTAL);
    itemBoxSizer7->Add(itemBoxSizer8, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

    wxStaticText* stattext2 = new wxStaticText( this, wxID_STATIC, _("1st Choice"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer8->Add(stattext2, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);
    wxArrayString choice_strings;
    choice_strings.Add(_("None"));
    choice_strings.Add(_("locus"));
    choice_strings.Add(_("description"));
    choice_strings.Add(_("comment"));
    choice_strings.Add(_("allele"));
    choice_strings.Add(_("maploc"));
    choice_strings.Add(_("locus_tag"));
    choice_strings.Add(_("synonym"));
    choice_strings.Add(_("old_locus_tag"));


    m_FeatQual = new wxChoice( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, choice_strings, 0 );
    m_FeatQual->SetSelection(0);
    itemBoxSizer8->Add(m_FeatQual, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);
    m_FeatQual->Bind(wxEVT_CHOICE, &CmRNAFromGeneCdsExon::OnSelectFeatQual, this);
    m_Remove = new wxCheckBox( itemCBulkCmdDlg1, wxID_ANY, _("Remove if used"), wxDefaultPosition, wxDefaultSize, 0 );
    m_Remove->SetValue(false);
    itemBoxSizer8->Add(m_Remove, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);


    wxBoxSizer* itemBoxSizer9 = new wxBoxSizer(wxHORIZONTAL);
    itemBoxSizer7->Add(itemBoxSizer9, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);
    m_2ndChoice = new wxStaticText( this, wxID_STATIC, _("2nd Choice"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer9->Add(m_2ndChoice, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);
    m_FeatQual2 = new wxChoice( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, choice_strings, 0 );
    m_FeatQual2->SetSelection(0);
    itemBoxSizer9->Add(m_FeatQual2, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);
    m_FeatQual2->Bind(wxEVT_CHOICE, &CmRNAFromGeneCdsExon::OnSelectFeatQual, this);
    m_FeatQual2->Disable();
    m_Remove2 = new wxCheckBox( itemCBulkCmdDlg1, wxID_ANY, _("Remove if used"), wxDefaultPosition, wxDefaultSize, 0 );
    m_Remove2->SetValue(false);
    itemBoxSizer9->Add(m_Remove2, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);
    m_Remove2->Disable();

    wxBoxSizer* itemBoxSizer10 = new wxBoxSizer(wxHORIZONTAL);
    itemBoxSizer7->Add(itemBoxSizer10, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);
    m_3rdChoice = new wxStaticText( this, wxID_STATIC, _("3rd Choice"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer10->Add(m_3rdChoice, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);
    m_FeatQual3 = new wxChoice( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, choice_strings, 0 );
    m_FeatQual3->SetSelection(0);
    itemBoxSizer10->Add(m_FeatQual3, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);
    m_FeatQual3->Disable();
    m_Remove3 = new wxCheckBox( itemCBulkCmdDlg1, wxID_ANY, _("Remove if used"), wxDefaultPosition, wxDefaultSize, 0 );
    m_Remove3->SetValue(false);
    itemBoxSizer10->Add(m_Remove3, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);
    m_Remove3->Disable();

    m_CapChangeOptions = new CCapChangePanel( itemCBulkCmdDlg1, wxID_ANY, wxDefaultPosition, wxSize(100, 100), wxSIMPLE_BORDER );
    itemBoxSizer2->Add(m_CapChangeOptions, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);
    
    wxStaticText* stattext4 = new wxStaticText( this, wxID_STATIC, _("Append text to name"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer2->Add(stattext4, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxALL, 5);
    m_StrQual = new wxTextCtrl( itemCBulkCmdDlg1, wxID_ANY, wxEmptyString, wxDefaultPosition, wxSize(100, -1), 0 );
    itemBoxSizer2->Add(m_StrQual, 0, wxALIGN_CENTER_HORIZONTAL|wxLEFT|wxRIGHT|wxBOTTOM, 5);


    m_SingleInterval = new wxCheckBox( itemCBulkCmdDlg1, wxID_ANY, _("Fuse multiple intervals for new mRNA"), wxDefaultPosition, wxDefaultSize, 0 );
    m_SingleInterval->SetValue(false);
    itemBoxSizer2->Add(m_SingleInterval, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

    m_IncludeUTR = new wxCheckBox( itemCBulkCmdDlg1, wxID_ANY, _("Include UTR regions in mRNA location"), wxDefaultPosition, wxDefaultSize, 0 );
    m_IncludeUTR->SetValue(false);
    itemBoxSizer2->Add(m_IncludeUTR, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);


    m_FeatureConstraint = new CFeatureFieldNamePanel(itemCBulkCmdDlg1, NULL, wxID_ANY, wxDefaultPosition, wxDefaultSize, 0);
    itemBoxSizer2->Add(m_FeatureConstraint, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 0);   
    m_FeatureConstraint->PopulateFeatureListbox();

    m_StringConstraintPanel = new CStringConstraintPanel( itemCBulkCmdDlg1, false, wxID_ANY, wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer2->Add(m_StringConstraintPanel, 0, wxALIGN_CENTER_HORIZONTAL|wxALL|wxFIXED_MINSIZE, 0); 

    m_OkCancel = new COkCancelPanel( itemCBulkCmdDlg1, wxID_ANY, wxDefaultPosition, wxSize(100, 100), 0 );
    itemBoxSizer2->Add(m_OkCancel, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

    m_CapChangeOptions->Disable();
////@end CmRNAFromGeneCdsExon content construction
}


/*!
 * Should we show tooltips?
 */

bool CmRNAFromGeneCdsExon::ShowToolTips()
{
    return true;
}

/*!
 * Get bitmap resources
 */

wxBitmap CmRNAFromGeneCdsExon::GetBitmapResource( const wxString& name )
{
    // Bitmap retrieval
////@begin CmRNAFromGeneCdsExon bitmap retrieval
    wxUnusedVar(name);
    return wxNullBitmap;
////@end CmRNAFromGeneCdsExon bitmap retrieval
}

/*!
 * Get icon resources
 */

wxIcon CmRNAFromGeneCdsExon::GetIconResource( const wxString& name )
{
    // Icon retrieval
////@begin CmRNAFromGeneCdsExon icon retrieval
    wxUnusedVar(name);
    return wxNullIcon;
////@end CmRNAFromGeneCdsExon icon retrieval
}



string CmRNAFromGeneCdsExon::GetErrorMessage()
{
    return m_ErrorMessage;
}


CRef<CCmdComposite> CmRNAFromGeneCdsExon::GetCommand()
{
    CRef<CCmdComposite> cmd(NULL);

    string feat_type;
    int sel_feat = m_FeatureType->GetSelection();
    if (sel_feat != wxNOT_FOUND)
    {
        feat_type = m_FeatureType->GetString(sel_feat).ToStdString();
    }

    string field_name;
    if (NStr::IsBlank(feat_type)) {
        field_name = kPartialStart;
    } else {
        field_name = feat_type + " " + kPartialStart;
    }

    CRef<CMiscSeqTableColumn> col(new CMiscSeqTableColumn(field_name));

    CRef<edit::CStringConstraint> string_constraint(m_StringConstraintPanel->GetStringConstraint());
    string constraint_field = m_FeatureConstraint->GetFieldName(false);

   
    vector<CConstRef<CObject> > objs;
    objs = col->GetObjects(m_TopSeqEntry, constraint_field, string_constraint);

    if (objs.size() == 0) {
        wxMessageBox(wxT("No features found!"), wxT("Error"),
                     wxOK | wxICON_ERROR, NULL);
        return cmd;
    }

    bool any_change = false;
    CScope &scope =  m_TopSeqEntry.GetScope();
  

    bool single_interval = m_SingleInterval->GetValue();
    bool include_utr_locs = m_IncludeUTR->GetValue();

    map<CBioseq_Handle, vector<const CSeq_feat*> > features;
    cmd.Reset(new CCmdComposite("mRNA from Gene, CDS, exon"));
    ITERATE(vector<CConstRef<CObject> >, it, objs) 
    {
        const CSeq_feat* f = dynamic_cast<const CSeq_feat* >((*it).GetPointer());
        if (f) 
        {   
            CBioseq_Handle bsh = scope.GetBioseqHandle(f->GetLocation());
            features[bsh].push_back(f);
        }
    }

    for (auto &bsh_pair : features)
    {
        CBioseq_Handle bsh = bsh_pair.first;
        for (size_t i = 0; i < bsh_pair.second.size(); i++)
        {
            const CSeq_feat* f = bsh_pair.second[i];
            CRef<CSeq_loc> loc(new CSeq_loc());
            loc->Assign(f->GetLocation());

            if (single_interval)
            {
                bool partial_start = loc->IsPartialStart(eExtreme_Positional);
                bool partial_stop = loc->IsPartialStop(eExtreme_Positional);
                loc->SetPartialStart(false,eExtreme_Positional);
                loc->SetPartialStop(false,eExtreme_Positional);
                for (size_t j = 1; j < bsh_pair.second.size(); j++)
                {
                    const CSeq_feat* f2 = bsh_pair.second[j];
                    CRef<CSeq_loc> add_loc(new CSeq_loc);
                    add_loc->Assign(f2->GetLocation());
                    partial_stop = add_loc->IsPartialStop(eExtreme_Positional);
                    add_loc->SetPartialStart(false,eExtreme_Positional);
                    add_loc->SetPartialStop(false,eExtreme_Positional);
                    loc->Assign(*(sequence::Seq_loc_Add(*loc, *add_loc, CSeq_loc::fMerge_All|CSeq_loc::fSort, &scope)));
                }
                loc->SetPartialStart(partial_start,eExtreme_Positional);
                loc->SetPartialStop(partial_stop,eExtreme_Positional);
            }

            if (include_utr_locs)
            {
                for (CFeat_CI utr(bsh, loc->IsReverseStrand() ? CSeqFeatData::eSubtype_5UTR : CSeqFeatData::eSubtype_3UTR); utr; ++utr)
                {
                    if (utr->GetLocation().GetStart(eExtreme_Positional) == loc->GetStop(eExtreme_Positional) + 1)
                    {
                        loc = sequence::Seq_loc_Add(*loc, utr->GetLocation(), CSeq_loc::fMerge_All|CSeq_loc::fSort, &scope);
                        break;
                    }
                }
                for (CFeat_CI utr(bsh, loc->IsReverseStrand() ? CSeqFeatData::eSubtype_3UTR : CSeqFeatData::eSubtype_5UTR); utr; ++utr)
                {
                    if (utr->GetLocation().GetStop(eExtreme_Positional) + 1 == loc->GetStart(eExtreme_Positional) )
                    {
                        loc = sequence::Seq_loc_Add(*loc, utr->GetLocation(), CSeq_loc::fMerge_All|CSeq_loc::fSort, &scope);
                        break;
                    }
                }
            }

            CRef<CSeq_feat> new_feat(new CSeq_feat());
            new_feat->SetData().SetRna().SetType(CRNA_ref::eType_mRNA);
            new_feat->SetLocation(*loc);            
            new_feat->SetPartial(new_feat->GetLocation().IsPartialStart(eExtreme_Biological) | new_feat->GetLocation().IsPartialStop(eExtreme_Biological));

            CSeq_entry_Handle seh = bsh.GetSeq_entry_Handle();
            m_modified = false;
            m_modified_prot = false;
            CRef<CSeq_feat> mod_feat(new CSeq_feat());
            mod_feat->Assign(*f);

            CRef<CSeq_feat> prot_feat;
            CSeq_feat_Handle prot_fh;
            if (f->IsSetProduct())
            {
                CBioseq_Handle prot_bsh = scope.GetBioseqHandle(f->GetProduct());
                if (prot_bsh)
                {
                    CFeat_CI prot_feat_ci(prot_bsh, CSeqFeatData::e_Prot);
                    if (prot_feat_ci)
                    {
                        prot_feat.Reset(new CSeq_feat);
                        prot_feat->Assign(prot_feat_ci->GetOriginalFeature());
                        prot_fh = prot_feat_ci->GetSeq_feat_Handle();
                    }
                }
            }

            string name = GetSuggestedName(*mod_feat, prot_feat, seh);
            if (m_modified)
            {
                CSeq_feat_Handle fh = scope.GetSeq_featHandle(*f);
                cmd->AddCommand(*CRef<CCmdChangeSeq_feat>(new CCmdChangeSeq_feat(fh,*mod_feat)));
            }
            if (m_modified_prot && prot_fh)
            {
                cmd->AddCommand(*CRef<CCmdChangeSeq_feat>(new CCmdChangeSeq_feat(prot_fh, *prot_feat)));
            }
           
            new_feat->SetData().SetRna().SetExt().SetName(name);
            cmd->AddCommand(*CRef<CCmdCreateFeat>(new CCmdCreateFeat(seh, *new_feat)));
            any_change = true;
            if (single_interval)
                break;
        }
    }
                
    
      
    if (!any_change) {
        wxMessageBox(wxT("No effect!"), wxT("Error"),
                     wxOK | wxICON_ERROR, NULL);
        cmd.Reset(NULL);
    }

    return cmd;
}

void CmRNAFromGeneCdsExon::OnSelectFeature(wxCommandEvent& event)
{
    string feat_type;
    int sel_feat = m_FeatureType->GetSelection();
    if (sel_feat != wxNOT_FOUND)
    {
        feat_type = m_FeatureType->GetString(sel_feat).ToStdString();
    }
    if (feat_type.empty())
        return;

    wxArrayString choice_strings;
    m_FeatQual->Clear();
    m_FeatQual2->Clear();
    m_FeatQual3->Clear();

    if ( feat_type == "CDS" )
    {
        choice_strings.Add(_("None"));
        choice_strings.Add(_("Comment"));
        choice_strings.Add(_("Product"));
        m_FeatQual->Append(choice_strings);
        m_FeatQual2->Append(choice_strings);
        m_FeatQual3->Append(choice_strings);
    }
    else if (feat_type == "Gene")
    {    
        choice_strings.Add(_("None"));
        choice_strings.Add(_("locus"));
        choice_strings.Add(_("description"));
        choice_strings.Add(_("comment"));
        choice_strings.Add(_("allele"));
        choice_strings.Add(_("maploc"));
        choice_strings.Add(_("locus_tag"));
        choice_strings.Add(_("synonym"));
        choice_strings.Add(_("old_locus_tag"));
        m_FeatQual->Append(choice_strings);      
        m_FeatQual2->Append(choice_strings);
        m_FeatQual3->Append(choice_strings);
    }
    else if (feat_type == "exon")
    {
        choice_strings.Add(_("None"));
        choice_strings.Add(_("allele"));
        choice_strings.Add(_("comment"));
        choice_strings.Add(_("EC_number"));
        choice_strings.Add(_("function"));
        choice_strings.Add(_("old_locus_tag"));
        choice_strings.Add(_("number"));
        choice_strings.Add(_("product"));
        m_FeatQual->Append(choice_strings);      
        m_FeatQual2->Append(choice_strings);
        m_FeatQual3->Append(choice_strings);
    }
    m_FeatQual->SetSelection(0); 
    if (feat_type == "5' UTR" || feat_type == "3' UTR")
    {     
        m_FeatQual->Disable();
        m_Remove->Disable();
    }
    m_FeatQual->Refresh();
    m_FeatQual2->SetSelection(0);
    m_FeatQual2->Disable(); 
    m_Remove2->Disable();
    m_FeatQual2->Refresh();
    m_FeatQual3->SetSelection(0);
    m_FeatQual3->Disable();
    m_Remove3->Disable(); 
    m_FeatQual3->Refresh();
   
}


void CmRNAFromGeneCdsExon::OnSelectFeatQual(wxCommandEvent& event)
{
    int sel1 = m_FeatQual->GetSelection();
    if (sel1 != wxNOT_FOUND && sel1 != 0)
    {
        m_FeatQual2->Enable();
        m_Remove2->Enable();
        m_CapChangeOptions->Enable();
    }
    else
    {
        m_FeatQual2->Disable();
        m_Remove2->Disable();
        m_CapChangeOptions->Disable();
    }

    int sel2 = m_FeatQual2->GetSelection();
    if (sel2 != wxNOT_FOUND && sel2 != 0)
    {
        m_FeatQual3->Enable();
        m_Remove3->Enable();
    }
    else
    {
        m_FeatQual3->Disable();
        m_Remove3->Disable();
    }
}

string CmRNAFromGeneCdsExon::GetSuggestedName(CSeq_feat &feat, CRef<CSeq_feat> prot_feat, CSeq_entry_Handle seh)
{
    string name;
    string feat_type;
    int sel_feat = m_FeatureType->GetSelection();
    if (sel_feat != wxNOT_FOUND)
    {
        feat_type = m_FeatureType->GetString(sel_feat).ToStdString();
    }
    if (feat_type == "Gene" && feat.IsSetData() && feat.GetData().IsGene())
    {
        if (m_FeatQual->GetSelection() !=  wxNOT_FOUND)
            GetNameFromGene(feat, m_FeatQual->GetString(m_FeatQual->GetSelection()), m_Remove->GetValue(), name);     

        if (name.empty() && m_FeatQual2->IsEnabled() && m_FeatQual2->GetSelection() !=  wxNOT_FOUND)
            GetNameFromGene(feat, m_FeatQual2->GetString(m_FeatQual2->GetSelection()), m_Remove2->GetValue(), name);

        if (name.empty() && m_FeatQual3->IsEnabled() && m_FeatQual3->GetSelection() !=  wxNOT_FOUND)
            GetNameFromGene(feat, m_FeatQual3->GetString(m_FeatQual3->GetSelection()), m_Remove3->GetValue(), name);    
                   
    }
    if (feat_type == "CDS" && feat.IsSetData() && feat.GetData().IsCdregion())
    {
        if (m_FeatQual->GetSelection() !=  wxNOT_FOUND)
            GetNameFromCDS(feat, m_FeatQual->GetString(m_FeatQual->GetSelection()), m_Remove->GetValue(), name, prot_feat);       

        if (name.empty() && m_FeatQual2->IsEnabled() && m_FeatQual2->GetSelection() !=  wxNOT_FOUND)
            GetNameFromCDS(feat, m_FeatQual2->GetString(m_FeatQual2->GetSelection()), m_Remove2->GetValue(), name, prot_feat);

        if (name.empty() && m_FeatQual3->IsEnabled() && m_FeatQual3->GetSelection() !=  wxNOT_FOUND)
            GetNameFromCDS(feat, m_FeatQual3->GetString(m_FeatQual3->GetSelection()), m_Remove3->GetValue(), name, prot_feat);
    }
    if (feat_type == "exon" && feat.IsSetData() && feat.GetData().IsImp() && feat.GetData().GetImp().IsSetKey() &&
        feat.GetData().GetImp().GetKey() == "exon" )
    {
        if (m_FeatQual->GetSelection() !=  wxNOT_FOUND)
            GetNameFromExon(feat, m_FeatQual->GetString(m_FeatQual->GetSelection()), m_Remove->GetValue(), name);       

        if (name.empty() && m_FeatQual2->IsEnabled() && m_FeatQual2->GetSelection() !=  wxNOT_FOUND)
            GetNameFromExon(feat, m_FeatQual2->GetString(m_FeatQual2->GetSelection()), m_Remove2->GetValue(), name);

        if (name.empty() && m_FeatQual3->IsEnabled() && m_FeatQual3->GetSelection() !=  wxNOT_FOUND)
            GetNameFromExon(feat, m_FeatQual3->GetString(m_FeatQual3->GetSelection()), m_Remove3->GetValue(), name);
    }


    name += m_StrQual->GetValue().ToStdString();
    if (m_CapChangeOptions->IsEnabled())
    {
        ECapChange cap_change = m_CapChangeOptions->GetCapitalizationRequest();
        FixCapitalizationInString(seh, name, cap_change);
    }
    return name;
}

void  CmRNAFromGeneCdsExon::GetNameFromGene(CSeq_feat& gene, const wxString& sel_str, bool remove, string &name)
{
    string str;
    if (sel_str == _("locus") && gene.GetData().GetGene().IsSetLocus())
    {
        str = gene.GetData().GetGene().GetLocus();
        if (remove)
        {
            gene.SetData().SetGene().ResetLocus();
            m_modified = true;
        }
    }

    if (sel_str == _("comment") && gene.IsSetComment())
    {
        str = gene.GetComment();
        if (remove)
        {
            gene.ResetComment();
            m_modified = true;
        }
    }

    if (sel_str == _("locus_tag") && gene.GetData().GetGene().IsSetLocus_tag())
    {
        str = gene.GetData().GetGene().GetLocus_tag();
        if (remove)
        {
            gene.SetData().SetGene().ResetLocus_tag();
            m_modified = true;
        }
    }

    if (sel_str == _("allele") && gene.GetData().GetGene().IsSetAllele())
    {
        str = gene.GetData().GetGene().GetAllele();       
        if (remove)
        {
            gene.SetData().SetGene().ResetAllele();
            m_modified = true;
        }
    }

    if (sel_str == _("description") && gene.GetData().GetGene().IsSetDesc())
    {
        str = gene.GetData().GetGene().GetDesc();
        if (remove)
        {
            gene.SetData().SetGene().ResetDesc();
            m_modified = true;
        }
    }

    if (sel_str == _("maploc") && gene.GetData().GetGene().IsSetMaploc())
    {
        str = gene.GetData().GetGene().GetMaploc();
        if (remove)
        {
            gene.SetData().SetGene().ResetMaploc();
            m_modified = true;
        }
    }

    if (sel_str == _("synonym") && gene.GetData().GetGene().IsSetSyn() && !gene.GetData().GetGene().GetSyn().empty())
    {
        str = gene.GetData().GetGene().GetSyn().front();
        if (remove)
        {
            gene.SetData().SetGene().SetSyn().erase(gene.SetData().SetGene().SetSyn().begin());
            if (gene.GetData().GetGene().GetSyn().empty())
                gene.SetData().SetGene().ResetSyn();
            m_modified = true;
        }
    }

    if (sel_str == _("old_locus_tag"))
    {
        str = gene.GetNamedQual("old_locus_tag");
        if (remove)
        {
            gene.RemoveQualifier("old_locus_tag");
            m_modified = true;
        }
    }
    if (!str.empty())
        name = str;
}

void CmRNAFromGeneCdsExon::GetNameFromCDS(CSeq_feat& feat, const wxString& sel_str, bool remove, string &name, CRef<CSeq_feat> prot_feat)
{
    string str;
    if (sel_str == _("Comment") && feat.IsSetComment() && !feat.GetComment().empty())
    {
        str = feat.GetComment();
        if (remove)
        {
            feat.ResetComment();
            m_modified = true;
        }
    }

    if (sel_str == _("Product") && prot_feat)
    {
        CProt_ref &prot = prot_feat->SetData().SetProt();
        if (prot.IsSetName() && !prot.GetName().empty())
        {
            str = prot.GetName().front();
            if (remove)
            {
                prot.SetName().erase(prot.SetName().begin());
                if (prot.GetName().empty())
                    prot.ResetName();
                m_modified_prot = true;
            }
        }
    }

    if (!str.empty())
        name = str;
}


void  CmRNAFromGeneCdsExon::GetNameFromExon(CSeq_feat& feat, const wxString& sel_str, bool remove, string &name)
{
    string str;
    if (sel_str == _("comment") && feat.IsSetComment() && !feat.GetComment().empty())
    {
        str =  feat.GetComment();
        if (remove)
        {
            feat.ResetComment();
            m_modified = true;
        }
    }

    if (!sel_str.IsEmpty() && sel_str != _("None"))
    {
        str = feat.GetNamedQual(sel_str.ToStdString());
        if (remove)
        {
            feat.RemoveQualifier(sel_str.ToStdString());
            m_modified = true;
        }
    }

    if (!str.empty())
        name = str;
}

void CmRNAFromGeneCdsExon::ProcessUpdateFeatEvent( wxCommandEvent& event )
{
        UpdateChildrenFeaturePanels(this->GetSizer());
}

void CmRNAFromGeneCdsExon::UpdateChildrenFeaturePanels( wxSizer* sizer )
{
    wxSizerItemList& slist = sizer->GetChildren();
    int n =0;
    for (wxSizerItemList::iterator iter = slist.begin(); iter != slist.end(); ++iter, ++n) {
        if ((*iter)->IsSizer()) {
            UpdateChildrenFeaturePanels((*iter)->GetSizer());
        } else if ((*iter)->IsWindow()) {
            wxWindow* child = (*iter)->GetWindow();
            if (child) {
                CFeatureTypePanel* panel = dynamic_cast<CFeatureTypePanel*>(child);
                if (panel) {
                    panel->ListPresentFeaturesFirst(m_TopSeqEntry);
                } else {
                    wxSizer* subsizer = child->GetSizer();
                    if (subsizer) {
                        UpdateChildrenFeaturePanels(subsizer);
                    } 
                }
            }
        } 
    }    
}

END_NCBI_SCOPE




