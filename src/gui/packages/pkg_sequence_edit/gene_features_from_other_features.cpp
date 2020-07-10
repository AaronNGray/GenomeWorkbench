/*  $Id: gene_features_from_other_features.cpp 41872 2018-10-31 15:16:50Z asztalos $
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

#include <objmgr/util/seq_loc_util.hpp>
#include <objmgr/seq_vector.hpp>
#include <objmgr/util/sequence.hpp>
#include <objtools/edit/cds_fix.hpp>
#include <objtools/edit/loc_edit.hpp>
#include <gui/objutils/cmd_create_feat.hpp>
#include <gui/objutils/cmd_change_seq_feat.hpp>
#include <gui/objutils/util_cmds.hpp>
#include <gui/core/selection_service_impl.hpp>
#include <gui/widgets/edit/feature_field_name_panel.hpp>
#include <gui/packages/pkg_sequence_edit/gene_features_from_other_features.hpp>

////@begin XPM images
////@end XPM images

BEGIN_NCBI_SCOPE

USING_SCOPE(ncbi::objects);

/*!
 * CGeneFeatFromOtherFeatDlg type definition
 */

IMPLEMENT_DYNAMIC_CLASS( CGeneFeatFromOtherFeatDlg, CBulkCmdDlg )


/*!
 * CGeneFeatFromOtherFeatDlg event table definition
 */

BEGIN_EVENT_TABLE( CGeneFeatFromOtherFeatDlg, CBulkCmdDlg )

////@begin CGeneFeatFromOtherFeatDlg event table entries
   
    EVT_UPDATE_FEATURE_LIST(wxID_ANY, CGeneFeatFromOtherFeatDlg::ProcessUpdateFeatEvent )

////@end CGeneFeatFromOtherFeatDlg event table entries

END_EVENT_TABLE()


/*!
 * CGeneFeatFromOtherFeatDlg constructors
 */

CGeneFeatFromOtherFeatDlg::CGeneFeatFromOtherFeatDlg()
{
    Init();
}

CGeneFeatFromOtherFeatDlg::CGeneFeatFromOtherFeatDlg( wxWindow* parent, IWorkbench* wb, wxWindowID id, const wxString& caption, const wxPoint& pos, const wxSize& size, long style )
  : CBulkCmdDlg(wb)
{
    Init();
    Create(parent, id, caption, pos, size, style);
}


/*!
 * CGeneFeatFromOtherFeatDlg creator
 */

bool CGeneFeatFromOtherFeatDlg::Create( wxWindow* parent, wxWindowID id, const wxString& caption, const wxPoint& pos, const wxSize& size, long style )
{
////@begin CGeneFeatFromOtherFeatDlg creation
    SetExtraStyle(wxWS_EX_BLOCK_EVENTS);
    CBulkCmdDlg::Create( parent, id, caption, pos, size, style );

    CreateControls();
    if (GetSizer())
    {
        GetSizer()->SetSizeHints(this);
    }
    Centre();
////@end CGeneFeatFromOtherFeatDlg creation
    return true;
}


/*!
 * CGeneFeatFromOtherFeatDlg destructor
 */

CGeneFeatFromOtherFeatDlg::~CGeneFeatFromOtherFeatDlg()
{
////@begin CGeneFeatFromOtherFeatDlg destruction
////@end CGeneFeatFromOtherFeatDlg destruction
}


/*!
 * Member initialisation
 */

void CGeneFeatFromOtherFeatDlg::Init()
{
////@begin CGeneFeatFromOtherFeatDlg member initialisation
    m_FeatureType = NULL;
    m_OkCancel = NULL;
////@end CGeneFeatFromOtherFeatDlg member initialisation
    m_ErrorMessage = "";
    m_SingleInterval = NULL;
    m_OnlySelected = NULL;
    m_StringConstraintPanel = NULL;
}


/*!
 * Control creation for CGeneFeatFromOtherFeatDlg
 */

void CGeneFeatFromOtherFeatDlg::CreateControls()
{    
////@begin CGeneFeatFromOtherFeatDlg content construction
    CGeneFeatFromOtherFeatDlg* itemCBulkCmdDlg1 = this;

    wxBoxSizer* itemBoxSizer2 = new wxBoxSizer(wxVERTICAL);
    itemCBulkCmdDlg1->SetSizer(itemBoxSizer2);

    m_FeatureType = new CFeatureTypePanel( itemCBulkCmdDlg1, wxID_ANY, wxDefaultPosition, wxSize(100, 100), wxSIMPLE_BORDER );
    itemBoxSizer2->Add(m_FeatureType, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);
    GetTopLevelSeqEntryAndProcessor();
    m_FeatureType->ListPresentFeaturesFirst(m_TopSeqEntry); 
    m_FeatureType->m_List->Bind(wxEVT_LISTBOX, &CGeneFeatFromOtherFeatDlg::OnSelectFeature, this);

    wxStaticBox* itemStaticBoxSizer7Static = new wxStaticBox(itemCBulkCmdDlg1, wxID_ANY, _("Select qualifier to use in gene"));
    wxStaticBoxSizer* itemBoxSizer7 = new wxStaticBoxSizer(itemStaticBoxSizer7Static, wxVERTICAL);
    itemBoxSizer2->Add(itemBoxSizer7, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

    wxBoxSizer* itemBoxSizer8 = new wxBoxSizer(wxHORIZONTAL);
    itemBoxSizer7->Add(itemBoxSizer8, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

    wxStaticText* stattext2 = new wxStaticText( this, wxID_STATIC, _("1st Choice"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer8->Add(stattext2, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxALL, 5);
    wxArrayString choice_strings;
    choice_strings.Add(_("None"));
    choice_strings.Add(_("Comment"));
    choice_strings.Add(_("Product"));
    choice_strings.Add(_("Protein descriptor"));
    m_FeatQual = new wxChoice( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, choice_strings, 0 );
    m_FeatQual->SetSelection(0);
    itemBoxSizer8->Add(m_FeatQual, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);
    m_FeatQual->Bind(wxEVT_CHOICE, &CGeneFeatFromOtherFeatDlg::OnSelectFeatQual, this);

    wxBoxSizer* itemBoxSizer9 = new wxBoxSizer(wxHORIZONTAL);
    itemBoxSizer7->Add(itemBoxSizer9, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

    m_2ndChoice = new wxStaticText( this, wxID_STATIC, _("2nd Choice"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer9->Add(m_2ndChoice, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxALL, 5);
    m_FeatQual2 = new wxChoice( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, choice_strings, 0 );
    m_FeatQual2->SetSelection(0);
    itemBoxSizer9->Add(m_FeatQual2, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

    wxBoxSizer* itemBoxSizer10 = new wxBoxSizer(wxHORIZONTAL);
    itemBoxSizer7->Add(itemBoxSizer10, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

    wxStaticText* stattext4 = new wxStaticText( this, wxID_STATIC, _("Use this string:"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer10->Add(stattext4, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxALL, 5);
    m_StrQual = new wxTextCtrl( itemCBulkCmdDlg1, wxID_ANY, wxEmptyString, wxDefaultPosition, wxSize(100, -1), 0 );
    itemBoxSizer10->Add(m_StrQual, 0, wxALIGN_CENTER_HORIZONTAL|wxLEFT|wxRIGHT|wxBOTTOM, 5);
    m_StrQual->Bind(wxEVT_TEXT, &CGeneFeatFromOtherFeatDlg::OnSelectFeatQual, this);

    wxBoxSizer* itemBoxSizer5 = new wxBoxSizer(wxHORIZONTAL);
    itemBoxSizer2->Add(itemBoxSizer5, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);
    wxStaticText* stattext = new wxStaticText( this, wxID_STATIC, _("Select gene qualifier to populate"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer5->Add(stattext, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxALL, 5);
    wxArrayString qual_strings;
    qual_strings.Add(_("locus"));
    qual_strings.Add(_("locus_tag"));
    qual_strings.Add(_("gene description"));
    qual_strings.Add(_("allele"));
    qual_strings.Add(_("gene comment"));
    m_GeneQual = new wxChoice( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, qual_strings, 0 );
    m_GeneQual->SetSelection(0);
    itemBoxSizer5->Add(m_GeneQual, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

    m_CapChangeOptions = new CCapChangePanel( itemCBulkCmdDlg1, wxID_ANY, wxDefaultPosition, wxSize(100, 100), wxSIMPLE_BORDER );
    itemBoxSizer2->Add(m_CapChangeOptions, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

    wxBoxSizer* itemBoxSizer4 = new wxBoxSizer(wxHORIZONTAL);
    itemBoxSizer2->Add(itemBoxSizer4, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

    m_SingleInterval = new wxCheckBox( itemCBulkCmdDlg1, wxID_ANY, _("Create gene with single interval location"), wxDefaultPosition, wxDefaultSize, 0 );
    m_SingleInterval->SetValue(true);
    itemBoxSizer4->Add(m_SingleInterval, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

    m_OnlySelected = new wxCheckBox( itemCBulkCmdDlg1, wxID_ANY, _("Only create genes for selected features"), wxDefaultPosition, wxDefaultSize, 0 );
    m_OnlySelected->SetValue(false);
    itemBoxSizer4->Add(m_OnlySelected, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);
 

    wxPanel *container2 = new wxPanel( itemCBulkCmdDlg1, wxID_ANY, wxDefaultPosition, wxSize(800,90), 0 );
    itemBoxSizer2->Add(container2, 0, wxALIGN_CENTER_HORIZONTAL|wxALL|wxFIXED_MINSIZE, 0);

    wxBoxSizer* itemBoxSizer6 = new wxBoxSizer(wxHORIZONTAL);
    container2->SetSizer(itemBoxSizer6);

    m_StringConstraintPanel = new CStringConstraintPanel( container2, false, wxID_ANY, wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer6->Add(m_StringConstraintPanel, 0, wxALIGN_CENTER_HORIZONTAL|wxALL|wxFIXED_MINSIZE, 0); 

    m_OkCancel = new COkCancelPanel( itemCBulkCmdDlg1, wxID_ANY, wxDefaultPosition, wxSize(100, 100), 0 );
    itemBoxSizer2->Add(m_OkCancel, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

    m_GeneQual->Disable();
    m_CapChangeOptions->Disable();
////@end CGeneFeatFromOtherFeatDlg content construction
}


/*!
 * Should we show tooltips?
 */

bool CGeneFeatFromOtherFeatDlg::ShowToolTips()
{
    return true;
}

/*!
 * Get bitmap resources
 */

wxBitmap CGeneFeatFromOtherFeatDlg::GetBitmapResource( const wxString& name )
{
    // Bitmap retrieval
////@begin CGeneFeatFromOtherFeatDlg bitmap retrieval
    wxUnusedVar(name);
    return wxNullBitmap;
////@end CGeneFeatFromOtherFeatDlg bitmap retrieval
}

/*!
 * Get icon resources
 */

wxIcon CGeneFeatFromOtherFeatDlg::GetIconResource( const wxString& name )
{
    // Icon retrieval
////@begin CGeneFeatFromOtherFeatDlg icon retrieval
    wxUnusedVar(name);
    return wxNullIcon;
////@end CGeneFeatFromOtherFeatDlg icon retrieval
}


void CGeneFeatFromOtherFeatDlg::ProcessUpdateFeatEvent( wxCommandEvent& event )
{
    UpdateChildrenFeaturePanels(this->GetSizer());
    m_FeatureType->ListAnyOption();
}



string CGeneFeatFromOtherFeatDlg::GetErrorMessage()
{
    return m_ErrorMessage;
}


CRef<CCmdComposite> CGeneFeatFromOtherFeatDlg::GetCommand()
{
    CRef<CCmdComposite> cmd(NULL);
    string feat_type = m_FeatureType->GetFieldName();
    if (NStr::EqualNocase(feat_type, "any")) {
        feat_type = "";
    }
    string field_name = "";
    if (NStr::IsBlank(feat_type)) {
        field_name = kPartialStart;
    } else {
        field_name = feat_type + " " + kPartialStart;
    }

    CRef<CMiscSeqTableColumn> col(new CMiscSeqTableColumn(field_name));

    CRef<edit::CStringConstraint> string_constraint(m_StringConstraintPanel->GetStringConstraint());
    string constraint_field = feat_type;

    bool only_selected = m_OnlySelected->GetValue();
   
    vector<CConstRef<CObject> > objs;
    if (only_selected)
        objs = x_GetSelectedFeats();
    else
        objs = col->GetObjects(m_TopSeqEntry, constraint_field, string_constraint);

    if (objs.size() == 0) {
        wxMessageBox(wxT("No features found!"), wxT("Error"),
                     wxOK | wxICON_ERROR, NULL);
        return cmd;
    }

    bool any_change = false;
    CScope &scope =  m_TopSeqEntry.GetScope();
  

    bool single_interval = m_SingleInterval->GetValue();
    cmd.Reset(new CCmdComposite("Gene Features from Other Features"));
    ITERATE(vector<CConstRef<CObject> >, it, objs) 
    {
        const CSeq_feat* f = dynamic_cast<const CSeq_feat* >((*it).GetPointer());
        if (f) 
        {   
            CRef<CSeq_loc> loc(new CSeq_loc());
            loc->Assign(f->GetLocation());
            CBioseq_Handle bsh = scope.GetBioseqHandle(*loc);

            CRef<CSeq_feat> new_feat(new CSeq_feat());
            new_feat->SetData().SetGene();
            new_feat->SetLocation(*loc);
            if (single_interval)
            {
                CRef<CSeq_loc> new_loc = sequence::Seq_loc_Merge(new_feat->GetLocation(), CSeq_loc::fMerge_SingleRange, &scope);
                new_feat->SetLocation(*new_loc);
            }
            new_feat->SetPartial(new_feat->GetLocation().IsPartialStart(eExtreme_Biological) || new_feat->GetLocation().IsPartialStop(eExtreme_Biological));
            CSeq_entry_Handle seh = bsh.GetSeq_entry_Handle();
            x_AddGeneQuals(new_feat, seh, f);

            cmd->AddCommand(*CRef<CCmdCreateFeat>(new CCmdCreateFeat(seh, *new_feat)));
            any_change = true;
        }
    }
                
    
      
    if (!any_change) {
        wxMessageBox(wxT("No effect!"), wxT("Error"),
                     wxOK | wxICON_ERROR, NULL);
        cmd.Reset(NULL);
    }

    return cmd;
}

void CGeneFeatFromOtherFeatDlg::x_AddGeneQuals(CRef<objects::CSeq_feat> gene, CSeq_entry_Handle seh, const CSeq_feat* f)
{
    string str = m_StrQual->GetValue().ToStdString();

    if (m_FeatQual2->IsShown())
    {
        int sel_feat2 = m_FeatQual2->GetSelection();
        if (sel_feat2 != wxNOT_FOUND)
        {
            string sel_feat_name = m_FeatQual2->GetString(sel_feat2).ToStdString();
            if (sel_feat_name == "Comment" && f->IsSetComment() && !f->GetComment().empty())
            {
                str = f->GetComment();
            }
            else if (sel_feat_name == "Product" && f->IsSetProduct())
            {
                CConstRef<CSeq_feat> protein = sequence::GetBestOverlappingFeat(f->GetProduct(),CSeqFeatData::e_Prot, sequence::eOverlap_Contains,seh.GetScope());
                if (protein && protein->GetData().GetProt().IsSetName() && !protein->GetData().GetProt().GetName().empty() && !protein->GetData().GetProt().GetName().front().empty())
                    str = protein->GetData().GetProt().GetName().front();
            }
            else if (sel_feat_name == "Protein descriptor" && f->IsSetProduct())
            {
                CConstRef<CSeq_feat> protein = sequence::GetBestOverlappingFeat(f->GetProduct(),CSeqFeatData::e_Prot, sequence::eOverlap_Contains,seh.GetScope());
                if (protein && protein->GetData().GetProt().IsSetDesc() && !protein->GetData().GetProt().GetDesc().empty())
                    str = protein->GetData().GetProt().GetDesc();
            }
        }
    }

    int sel_feat = m_FeatQual->GetSelection();
    if (sel_feat != wxNOT_FOUND)
    {
        string sel_feat_name = m_FeatQual->GetString(sel_feat).ToStdString();
        if (sel_feat_name == "Comment" && f->IsSetComment() && !f->GetComment().empty())
        {
            str = f->GetComment();
        }
        else if (sel_feat_name == "Product" && f->IsSetProduct())
        {
            CConstRef<CSeq_feat> protein = sequence::GetBestOverlappingFeat(f->GetProduct(),CSeqFeatData::e_Prot, sequence::eOverlap_Contains,seh.GetScope());
            if (protein && protein->GetData().GetProt().IsSetName() && !protein->GetData().GetProt().GetName().empty() && !protein->GetData().GetProt().GetName().front().empty())
                str = protein->GetData().GetProt().GetName().front();
        }
        else if (sel_feat_name == "Protein descriptor" && f->IsSetProduct())
        {
                CConstRef<CSeq_feat> protein = sequence::GetBestOverlappingFeat(f->GetProduct(),CSeqFeatData::e_Prot, sequence::eOverlap_Contains,seh.GetScope());
                if (protein && protein->GetData().GetProt().IsSetDesc() && !protein->GetData().GetProt().GetDesc().empty())
                    str = protein->GetData().GetProt().GetDesc();
            }
    }


    ECapChange cap_change = m_CapChangeOptions->GetCapitalizationRequest();
    FixCapitalizationInString(seh, str, cap_change);

    if (!m_GeneQual) return;
    int sel_int = m_GeneQual->GetSelection();
    if (sel_int == wxNOT_FOUND) return;

    wxString sel_str = m_GeneQual->GetString(sel_int);

    if (sel_str == _("locus"))
    {
        gene->SetData().SetGene().SetLocus(str);
    }

    if (sel_str == _("gene comment"))
    {
        gene->SetComment(str);
    }

    if (sel_str == _("locus_tag"))
    {
        gene->SetData().SetGene().SetLocus_tag(str);
    }

    if (sel_str == _("allele"))
    {
        gene->SetData().SetGene().SetAllele(str);       
    }

    if (sel_str == _("gene description"))
    {
        gene->SetData().SetGene().SetDesc(str);
    }
}

vector<CConstRef<CObject> > CGeneFeatFromOtherFeatDlg::x_GetSelectedFeats()
{
    vector<CConstRef<CObject> > objs;

    TConstScopedObjects objects;
    CIRef<CSelectionService> sel_srv = m_Workbench->GetServiceByType<CSelectionService>();
    if (!sel_srv) 
        return objs;

    sel_srv->GetCurrentSelection(objects);
    //sel_srv->GetActiveObjects(objects);
  
    NON_CONST_ITERATE (TConstScopedObjects, it, objects) {
        const CObject* ptr = it->object.GetPointer();
        const CSeq_feat* seq_feat = dynamic_cast<const CSeq_feat*>(ptr);
        if (seq_feat)
        {
            CConstRef<CObject> o;
            o.Reset(seq_feat);
            objs.push_back(o);
        }
    }
  
    return objs;
}

void CGeneFeatFromOtherFeatDlg::OnSelectFeature(wxCommandEvent& event)
{
    wxArrayString choice_strings;
    m_FeatQual->Clear();
    string feat_type = m_FeatureType->GetFieldName();
    if (feat_type == "CDS" || feat_type == "misc_RNA_imp" || feat_type == "mRNA" || 
        feat_type == "rRNA" || feat_type == "tRNA" )
    {
        choice_strings.Add(_("None"));
        choice_strings.Add(_("Comment"));
        choice_strings.Add(_("Product"));
        choice_strings.Add(_("Protein descriptor"));
        m_FeatQual->Append(choice_strings);
        m_FeatQual->SetSelection(0);
        m_2ndChoice->Show();
        m_FeatQual2->Show();
        m_FeatQual2->SetSelection(0);
    }
    else
    {
        choice_strings.Add(_("None"));
        choice_strings.Add(_("Comment"));
        m_FeatQual->Append(choice_strings);
        m_FeatQual->SetSelection(0);
        m_2ndChoice->Hide();
        m_FeatQual2->Hide();
    }
    m_FeatQual->Refresh();
}


void CGeneFeatFromOtherFeatDlg::OnSelectFeatQual(wxCommandEvent& event)
{
    string sel_feat_name = "None";
    int sel_feat = m_FeatQual->GetSelection();
    if (sel_feat != wxNOT_FOUND)
    {
        sel_feat_name = m_FeatQual->GetString(sel_feat).ToStdString();
    }
    string str = m_StrQual->GetValue().ToStdString();

    if (sel_feat_name != "None" || !str.empty())
    {
        m_GeneQual->Enable();
        m_CapChangeOptions->Enable();
    }
    else
    {
        m_GeneQual->Disable();
        m_CapChangeOptions->Disable();
    }
}

END_NCBI_SCOPE

