/*  $Id: cds_to_mat_peptide.cpp 42167 2019-01-08 17:17:20Z filippov $
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
#include <objects/seq/Seq_descr.hpp>
#include <objects/seqset/seqset_macros.hpp>
#include <objects/seq/seq_macros.hpp>
#include <objmgr/util/seq_loc_util.hpp>
#include <objmgr/seq_vector.hpp>
#include <objmgr/util/sequence.hpp>
#include <objtools/edit/cds_fix.hpp>
#include <objtools/edit/loc_edit.hpp>
#include <gui/core/selection_service_impl.hpp>
#include <gui/objutils/cmd_add_cds.hpp>
#include <gui/objutils/cmd_change_seq_feat.hpp>
#include <gui/objutils/cmd_del_seq_feat.hpp>
#include <gui/objutils/cmd_create_feat.hpp>
#include <gui/objutils/cmd_change_seq_entry.hpp>
#include <gui/objutils/cmd_add_seqentry.hpp>
#include <gui/objutils/util_cmds.hpp>
#include <gui/objutils/convert_feat.hpp>
#include <objtools/validator/utilities.hpp>
#include <gui/packages/pkg_sequence_edit/cds_to_mat_peptide.hpp>
#include <gui/widgets/edit/feature_field_name_panel.hpp>
////@begin XPM images
////@end XPM images

BEGIN_NCBI_SCOPE

USING_SCOPE(ncbi::objects);

/*!
 * CCdsToMatPeptide type definition
 */

IMPLEMENT_DYNAMIC_CLASS( CCdsToMatPeptide, CBulkCmdDlg )


/*!
 * CCdsToMatPeptide event table definition
 */

BEGIN_EVENT_TABLE( CCdsToMatPeptide, CBulkCmdDlg )

////@begin CCdsToMatPeptide event table entries
EVT_RADIOBUTTON(wxID_ANY, CCdsToMatPeptide::OnRadioButton)   
EVT_UPDATE_FEATURE_LIST(wxID_ANY, CCdsToMatPeptide::ProcessUpdateFeatEvent )
////@end CCdsToMatPeptide event table entries

END_EVENT_TABLE()


/*!
 * CCdsToMatPeptide constructors
 */

CCdsToMatPeptide::CCdsToMatPeptide()
{
    Init();
}

CCdsToMatPeptide::CCdsToMatPeptide( wxWindow* parent, IWorkbench* wb, wxWindowID id, const wxString& caption, const wxPoint& pos, const wxSize& size, long style )
  : CBulkCmdDlg(wb)
{
    Init();
    Create(parent, id, caption, pos, size, style);
}


/*!
 * CCdsToMatPeptide creator
 */

bool CCdsToMatPeptide::Create( wxWindow* parent, wxWindowID id, const wxString& caption, const wxPoint& pos, const wxSize& size, long style )
{
////@begin CCdsToMatPeptide creation
    SetExtraStyle(wxWS_EX_BLOCK_EVENTS);
    CBulkCmdDlg::Create( parent, id, caption, pos, size, style );

    CreateControls();
    if (GetSizer())
    {
        GetSizer()->SetSizeHints(this);
    }
    Centre();
////@end CCdsToMatPeptide creation
    return true;
}


/*!
 * CCdsToMatPeptide destructor
 */

CCdsToMatPeptide::~CCdsToMatPeptide()
{
////@begin CCdsToMatPeptide destruction
////@end CCdsToMatPeptide destruction
}


/*!
 * Member initialisation
 */

void CCdsToMatPeptide::Init()
{
////@begin CCdsToMatPeptide member initialisation
    m_OkCancel = NULL;
////@end CCdsToMatPeptide member initialisation
    m_ErrorMessage = "";
    m_converter = CConvertFeatureBaseFactory::Create(CSeqFeatData::eSubtype_cdregion, CSeqFeatData::eSubtype_mat_peptide_aa );
    string field_name = "CDS " + kPartialStart;
    m_col.Reset(new CMiscSeqTableColumn(field_name));
}


/*!
 * Control creation for CCdsToMatPeptide
 */

void CCdsToMatPeptide::CreateControls()
{    
////@begin CCdsToMatPeptide content construction
    CCdsToMatPeptide* itemCBulkCmdDlg1 = this;

    wxBoxSizer* itemBoxSizer2 = new wxBoxSizer(wxVERTICAL);
    itemCBulkCmdDlg1->SetSizer(itemBoxSizer2);


    GetTopLevelSeqEntryAndProcessor();

    m_Inner = new wxRadioButton( itemCBulkCmdDlg1, wxID_ANY, _("Convert inner CDSs to mat_peptides"), wxDefaultPosition, wxDefaultSize,  wxRB_GROUP );
    m_Inner->SetValue(true);
    itemBoxSizer2->Add(m_Inner, 0, wxALIGN_LEFT|wxALL, 5);

    m_Merge = new wxRadioButton( itemCBulkCmdDlg1, wxID_ANY, _("Merge multiple CDSs to one CDS and convert inner CDSs to mat_peptides"), wxDefaultPosition, wxDefaultSize,  0 );
    m_Merge->SetValue(false);
    itemBoxSizer2->Add(m_Merge, 0, wxALIGN_LEFT|wxALL, 5);

    m_Each = new wxRadioButton( itemCBulkCmdDlg1, wxID_ANY, _("Create mat_peptide from protein on each CDS"), wxDefaultPosition, wxDefaultSize,  0 );
    m_Each->SetValue(false);
    itemBoxSizer2->Add(m_Each, 0, wxALIGN_LEFT|wxALL, 5);

    m_EntireRange = new wxCheckBox( itemCBulkCmdDlg1, wxID_ANY, _("New CDS should cover the entire sequence"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer2->Add(m_EntireRange, 0, wxALIGN_LEFT|wxALL, 5);

    m_ProductFirst = new wxRadioButton( itemCBulkCmdDlg1, wxID_ANY, _("Use product name from first CDS"), wxDefaultPosition, wxDefaultSize,  wxRB_GROUP );
    m_ProductFirst->SetValue(true);
    itemBoxSizer2->Add(m_ProductFirst, 0, wxALIGN_LEFT|wxALL, 5);

    wxBoxSizer* itemBoxSizer3 = new wxBoxSizer(wxHORIZONTAL);
    itemBoxSizer2->Add(itemBoxSizer3, 0, wxALIGN_LEFT, 0);
    m_ProductThis = new wxRadioButton( itemCBulkCmdDlg1, wxID_ANY, _("Use this product name"), wxDefaultPosition, wxDefaultSize,  0 );
    m_ProductThis->SetValue(false);
    itemBoxSizer3->Add(m_ProductThis, 0, wxALIGN_LEFT|wxALL, 5);
    m_ProductName = new wxTextCtrl( itemCBulkCmdDlg1, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer3->Add(m_ProductName, 0, wxALIGN_LEFT|wxALL, 5);

    m_FeatureConstraint = new CFeatureFieldNamePanel(itemCBulkCmdDlg1, NULL, wxID_ANY, wxDefaultPosition, wxDefaultSize, 0);
    itemBoxSizer2->Add(m_FeatureConstraint, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 0);   
    m_FeatureConstraint->PopulateFeatureListbox();

    m_StringConstraintPanel = new CStringConstraintPanel( itemCBulkCmdDlg1, false, wxID_ANY, wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer2->Add(m_StringConstraintPanel, 0, wxALIGN_CENTER_HORIZONTAL|wxALL|wxFIXED_MINSIZE, 0); 

    m_OkCancel = new COkCancelPanel( itemCBulkCmdDlg1, wxID_ANY, wxDefaultPosition, wxSize(100, 100), 0 );
    itemBoxSizer2->Add(m_OkCancel, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

////@end CCdsToMatPeptide content construction
    m_EntireRange->Disable();
    m_ProductFirst->Disable();
    m_ProductThis->Disable();
    m_ProductName->Disable();
    m_StringConstraintPanel->Disable();
    m_FeatureConstraint->Disable();
}


/*!
 * Should we show tooltips?
 */

bool CCdsToMatPeptide::ShowToolTips()
{
    return true;
}

/*!
 * Get bitmap resources
 */

wxBitmap CCdsToMatPeptide::GetBitmapResource( const wxString& name )
{
    // Bitmap retrieval
////@begin CCdsToMatPeptide bitmap retrieval
    wxUnusedVar(name);
    return wxNullBitmap;
////@end CCdsToMatPeptide bitmap retrieval
}

/*!
 * Get icon resources
 */

wxIcon CCdsToMatPeptide::GetIconResource( const wxString& name )
{
    // Icon retrieval
////@begin CCdsToMatPeptide icon retrieval
    wxUnusedVar(name);
    return wxNullIcon;
////@end CCdsToMatPeptide icon retrieval
}



string CCdsToMatPeptide::GetErrorMessage()
{
    return m_ErrorMessage;
}


CRef<CCmdComposite> CCdsToMatPeptide::GetCommand()
{
    
    CRef<CCmdComposite> cmd(new CCmdComposite("Convert Features"));

    if (m_Each->GetValue())
    {
        ConvertEach(cmd);
    }
    else if (m_Merge->GetValue())
    {
        MergeConvert(cmd);
    }
    else if (m_Inner->GetValue())
    {
        ConvertInner(cmd);
    }

    return cmd;
}

void CCdsToMatPeptide::ConvertEach(CRef<CCmdComposite> cmd)
{
    CRef<edit::CStringConstraint> string_constraint(NULL);
    vector<CConstRef<CObject> > objs;
    objs = m_col->GetObjects(m_TopSeqEntry, kEmptyStr, string_constraint);

    bool leave_original = true;
    ITERATE(vector<CConstRef<CObject> >, it, objs) {
        const CSeq_feat* f = dynamic_cast<const CSeq_feat* >((*it).GetPointer());
        CRef<CCmdComposite> subcmd = m_converter->Convert(*f, leave_original, m_TopSeqEntry.GetScope());
        if (subcmd) {
            cmd->AddCommand(*subcmd);
        }
    }
}

void CCdsToMatPeptide::MergeConvert(CRef<CCmdComposite> cmd)
{   
    CScope &scope = m_TopSeqEntry.GetScope();
    CRef<edit::CStringConstraint> string_constraint(m_StringConstraintPanel->GetStringConstraint());
    string constraint_field = m_FeatureConstraint->GetFieldName(false);
    bool create_general_only = objects::edit::IsGeneralIdProtPresent(m_TopSeqEntry);
    for (CBioseq_CI bi(m_TopSeqEntry, objects::CSeq_inst::eMol_na); bi; ++bi) 
    {
        CBioseq_Handle bsh = *bi;
        CSeq_entry_Handle seh = bsh.GetSeq_entry_Handle();
        if (!seh.IsSet())
        {
            CBioseq_set_Handle bss = seh.GetParentBioseq_set();
            if (bss)
                seh = bss.GetParentEntry();
        }
        vector<CConstRef<CObject> > objs;
        objs = m_col->GetObjects(seh, constraint_field, string_constraint);
        if (objs.empty())
            continue;
        CRef<objects::CSeq_id> prot_id(new objects::CSeq_id());
        int offset = 1;
        string id_label;
        prot_id->Assign(*objects::edit::GetNewProtId(bsh, offset, id_label, create_general_only));

        CRef<CSeq_feat> new_cds(new CSeq_feat);
        CRef<CSeq_loc> new_loc(new CSeq_loc);
        CRef<CSeq_annot> annot;
        CRef<CSeq_entry> new_entry(new CSeq_entry);
        new_entry->Assign(*seh.GetCompleteSeq_entry());
        string new_product;
        vector<string> products;
        bool first = true;
        ITERATE(vector<CConstRef<CObject> >, it, objs) 
        {
            const CSeq_feat* f = dynamic_cast<const CSeq_feat* >((*it).GetPointer());
            string product = m_converter->x_GetCDSProduct(*f, scope);
            products.push_back(product);
            if (first)
            {
                new_cds->Assign(*f);
                new_loc->Assign(f->GetLocation());
                new_product = product;
                FindAnnot(new_entry, f, annot);
                first = false;
            }
            else
            {
                CRef<CSeq_loc> add_loc(new CSeq_loc);
                add_loc->Assign(f->GetLocation());
                new_loc->Assign(*(sequence::Seq_loc_Add(*new_loc, *add_loc, CSeq_loc::fMerge_All|CSeq_loc::fSort, &scope)));
            }
            
        }
        
        if (m_EntireRange->GetValue())
        {
            new_loc = bsh.GetRangeSeq_loc(0,0);
        }
        new_cds->SetLocation(*new_loc);         
        new_cds->SetProduct().SetWhole().Assign(*prot_id);       
        annot->SetData().SetFtable().push_back(new_cds);

        CRef<CSeq_annot> prot_annot(new CSeq_annot);     
        CRef<objects::CBioseq> protein = CSeqTranslator::TranslateToProtein (*new_cds, scope);       
        protein->SetId().push_back(prot_id);
        CRef<objects::CSeqdesc> pdesc(new objects::CSeqdesc());
        pdesc->SetMolinfo().SetBiomol(objects::CMolInfo::eBiomol_peptide);
        pdesc->SetMolinfo().SetCompleteness(objects::CMolInfo::eCompleteness_complete);
        if (new_cds->GetLocation().IsPartialStart(objects::eExtreme_Biological)) 
        {
            pdesc->SetMolinfo().SetCompleteness(objects::CMolInfo::eCompleteness_no_left);
        }
        if (new_cds->GetLocation().IsPartialStop(objects::eExtreme_Biological)) 
        {
            if (pdesc->GetMolinfo().GetCompleteness() == objects::CMolInfo::eCompleteness_complete) {
                pdesc->SetMolinfo().SetCompleteness(objects::CMolInfo::eCompleteness_no_right);
            } else {
                pdesc->SetMolinfo().SetCompleteness(objects::CMolInfo::eCompleteness_no_ends);
            }
        }
        protein->SetDescr().Set().push_back(pdesc);
        CRef<objects::CSeq_feat> prot_feat(new CSeq_feat);
        prot_feat->SetData().SetProt();
        if (m_ProductThis->GetValue())
        {
            new_product = m_ProductName->GetValue().ToStdString();
        }
        prot_feat->SetData().SetProt().SetName().push_back(new_product); 
        prot_feat->SetLocation().SetInt().SetId(*prot_id);
        prot_feat->SetLocation().SetInt().SetFrom(0);
        prot_feat->SetLocation().SetInt().SetTo(protein->GetLength() - 1);
        prot_feat->SetLocation().SetPartialStart(new_cds->GetLocation().IsPartialStart(objects::eExtreme_Biological), objects::eExtreme_Biological);
        prot_feat->SetLocation().SetPartialStop(new_cds->GetLocation().IsPartialStop(objects::eExtreme_Biological), objects::eExtreme_Biological);
        prot_feat->SetPartial(new_cds->GetLocation().IsPartialStart(objects::eExtreme_Biological) || new_cds->GetLocation().IsPartialStop(objects::eExtreme_Biological));

        prot_annot->SetData().SetFtable().push_back(prot_feat);
        protein->SetAnnot().push_back(prot_annot);
        CRef<CSeq_entry> prot_entry(new CSeq_entry);
        prot_entry->SetSeq(*protein);
        new_entry->SetSet().SetSeq_set().push_back(prot_entry);                

        CRef<CExplicit_Mapper_Sequence_Info> explicit_seq_info(new CExplicit_Mapper_Sequence_Info);
        explicit_seq_info->AddSeq(CSeq_id_Handle::GetHandle(*new_cds->GetProduct().GetId()), CSeq_loc_Mapper::eSeq_prot, protein->GetLength());
        explicit_seq_info->AddSynonym(CSeq_id_Handle::GetHandle(*new_cds->GetProduct().GetId()),CSeq_id_Handle::GetHandle(*new_cds->GetProduct().GetId()));
        for (size_t i = 0; i < bsh.GetId().size(); i++)
        {
            explicit_seq_info->AddSeq(bsh.GetId()[i], CSeq_loc_Mapper::eSeq_nuc, bsh.GetBioseqLength());
            explicit_seq_info->AddSynonym(CSeq_id_Handle::GetHandle(*new_cds->GetLocation().GetId()),bsh.GetId()[i]);
        }
        

        size_t i = 0;
        ITERATE(vector<CConstRef<CObject> >, it, objs) 
        {
            const CSeq_feat* f = dynamic_cast<const CSeq_feat* >((*it).GetPointer());
            CSeq_loc_Mapper_Base mapper(*new_cds, CSeq_loc_Mapper::eLocationToProduct,  CSeq_loc_Mapper_Options(explicit_seq_info));
            mapper.SetMergeAbutting();
			// change made for GB-7810
            CRef<CSeq_loc> loc = mapper.MapTotalRange(f->GetLocation());
            loc->SetId(*prot_id);
            if (loc->GetStop(objects::eExtreme_Positional) >= protein->GetLength())
            {
                bool partial_stop =  loc->IsPartialStop(objects::eExtreme_Positional);
                CSeq_loc sub(*prot_id, protein->GetLength(), loc->GetStop(objects::eExtreme_Positional), loc->GetStrand());
                loc = sequence::Seq_loc_Subtract(*loc, sub, CSeq_loc::fMerge_All|CSeq_loc::fSort, &scope);
                loc->SetPartialStop(partial_stop, objects::eExtreme_Positional);
            }
            CRef<CSeq_feat> mat_peptide(new CSeq_feat);
            mat_peptide->Assign(*f);
            mat_peptide->SetLocation(*loc);
            mat_peptide->SetPartial(loc->IsPartialStart(eExtreme_Biological) || loc->IsPartialStop(eExtreme_Biological));
            if (i < products.size())
                mat_peptide->SetData().SetProt().SetName().push_back(products[i++]); 
            mat_peptide->SetData().SetProt().SetProcessed(CProt_ref::eProcessed_mature);
            mat_peptide->ResetProduct();
            prot_annot->SetData().SetFtable().push_back(mat_peptide);           
        }

        ITERATE(vector<CConstRef<CObject> >, it, objs) 
        {
            const CSeq_feat* f = dynamic_cast<const CSeq_feat* >((*it).GetPointer());
            CConstRef<CSeq_feat> gene = sequence::GetOverlappingGene(f->GetLocation(), scope);
            if (gene) 
            {              
                RemoveFeature(new_entry, gene.GetPointer()); 
            }
            CConstRef<CSeq_feat> mrna = sequence::GetOverlappingmRNA(f->GetLocation(), scope);
            if (mrna) 
            {
                RemoveFeature(new_entry, mrna.GetPointer());                
            }   
            RemoveProteins(new_entry, f, seh);
            RemoveFeature(new_entry, f);           
        }
        cmd->AddCommand(*CRef<CCmdChangeSeqEntry>(new CCmdChangeSeqEntry(seh, new_entry)));
    }    
}

void CCdsToMatPeptide::ConvertInner(CRef<CCmdComposite> cmd)
{
    CScope &scope = m_TopSeqEntry.GetScope();
    map<objects::CBioseq_Handle, set<objects::CSeq_feat_Handle> > product_to_cds;
    GetProductToCDSMap(scope, product_to_cds);
    CRef<edit::CStringConstraint> string_constraint(NULL);
    vector<CConstRef<CObject> > objs;
    objs = m_col->GetObjects(m_TopSeqEntry, kEmptyStr, string_constraint);
    bool leave_original = false;
    set<CSeq_feat_Handle> already_converted;
    ITERATE(vector<CConstRef<CObject> >, it, objs) {
        const CSeq_feat* f = dynamic_cast<const CSeq_feat* >((*it).GetPointer());
        if (f->IsSetData() && f->GetData().GetSubtype() == CSeqFeatData::eSubtype_cdregion)
        {
            CSeq_feat_Handle fh = scope.GetSeq_featHandle(*f);
            sequence::TFeatScores feats;
            sequence::GetOverlappingFeatures(f->GetLocation(), CSeqFeatData::e_Cdregion, CSeqFeatData::eSubtype_cdregion, sequence::eOverlap_Subset, feats, scope);
            for (size_t i = 0; i < feats.size(); i++)
            {
                CConstRef<CSeq_feat> inner = feats[i].second;
                CSeq_feat_Handle inner_fh = scope.GetSeq_featHandle(*inner);
                if (!inner->Equals(*f) && already_converted.find(inner_fh) == already_converted.end())   //&& feature::IsLocationInFrame(fh,inner->GetLocation()) == feature::eLocationInFrame_InFrame)
                {
                    already_converted.insert(inner_fh);
                    CSeq_loc_Mapper mapper(*f, CSeq_loc_Mapper::eLocationToProduct, &scope);
                    mapper.SetMergeAbutting();
					// for GB-7810, need to use MapTotalRange to get correct location mapping when ribosomal slippage is present
                    //CRef<CSeq_loc> loc = mapper.Map(inner->GetLocation()); 
					CRef<CSeq_loc> loc = mapper.MapTotalRange(inner->GetLocation()); 
                    CRef<CSeq_feat> mat_peptide(new CSeq_feat);
                    mat_peptide->Assign(*inner);
                    mat_peptide->SetLocation(*loc);
                    string product = m_converter->x_GetCDSProduct(*inner, scope);
                    mat_peptide->SetData().SetProt().SetName().push_back(product); 
                    mat_peptide->SetData().SetProt().SetProcessed(CProt_ref::eProcessed_mature);
                    mat_peptide->ResetProduct();
                    mat_peptide->ResetComment();
                    CBioseq_Handle prot_bsh2 = scope.GetBioseqHandle(inner->GetProduct());
                    if (prot_bsh2 && prot_bsh2.IsProtein())   
                    {
                        CFeat_CI prot_feat_ci(prot_bsh2, CSeqFeatData::e_Prot);
                        if (prot_feat_ci && prot_feat_ci->GetOriginalFeature().IsSetData() && prot_feat_ci->GetOriginalFeature().GetData().IsProt() 
                            && prot_feat_ci->GetOriginalFeature().GetData().GetProt().IsSetDesc())
                        {
                            mat_peptide->SetData().SetProt().SetDesc(prot_feat_ci->GetOriginalFeature().GetData().GetProt().GetDesc());
                        }
                    }
                    CBioseq_Handle prot_bsh = scope.GetBioseqHandle(f->GetProduct());
                    if (mat_peptide->GetLocation().GetStop(eExtreme_Biological) > prot_bsh.GetInst_Length() - 1) {
                        // need to truncate
                        if (mat_peptide->GetLocation().IsInt()) {
                            mat_peptide->SetLocation().SetInt().SetTo(prot_bsh.GetInst_Length() - 1);
                        }
                    }

                    CSeq_entry_Handle seh = prot_bsh.GetSeq_entry_Handle();
                    cmd->AddCommand(*CRef<CCmdCreateFeat>(new CCmdCreateFeat(seh, *mat_peptide)));
                    cmd->AddCommand(*GetDeleteFeatureCommand(inner_fh, true, product_to_cds)); 
                    // cmd->AddCommand(*CRef<CCmdDelSeq_feat>(new CCmdDelSeq_feat(inner_fh)));
                }
            }
        }       
    }
}

void CCdsToMatPeptide::RemoveFeature(CRef<CSeq_entry> entry, const CSeq_feat* feat)
{
    CSeq_entry::TAnnot::iterator annot_it = entry->SetAnnot().begin();
    while ( annot_it != entry->SetAnnot().end() )
    {
        if ((*annot_it)->IsSetData() && (*annot_it)->GetData().IsFtable())
        {
            EDIT_EACH_SEQFEAT_ON_SEQANNOT(feat_it, **annot_it)
            {
                if ((*feat_it)->IsSetData() && (*feat_it)->GetData().GetSubtype() == feat->GetData().GetSubtype() && feat->Equals(**feat_it))
                {
                    ERASE_SEQFEAT_ON_SEQANNOT(feat_it, **annot_it);
                }
            }
            if ((*annot_it)->GetData().GetFtable().empty())
            {
                annot_it = entry->SetAnnot().erase(annot_it);
            }
            else
                ++annot_it;
        }
        else
            ++annot_it;
    }
    if (entry->GetAnnot().empty())
    {
        if (entry->IsSet())
            entry->SetSet().ResetAnnot();
        else if (entry->IsSeq())
            entry->SetSeq().ResetAnnot();
    }

    if (entry->IsSet())
    {
        EDIT_EACH_SEQENTRY_ON_SEQSET(entry_it, entry->SetSet())
        {
            RemoveFeature(*entry_it, feat);
        }
    }
}


void CCdsToMatPeptide::RemoveProteins(CRef<CSeq_entry> entry, const CSeq_feat* feat, CSeq_entry_Handle seh)
{
    if (feat->IsSetProduct() && !validator::IsFarLocation(feat->GetProduct(), seh) && entry->IsSet()) 
    {
        EDIT_EACH_SEQENTRY_ON_SEQSET(entry_it, entry->SetSet())
        {
            if ((*entry_it)->IsSeq() && (*entry_it)->GetSeq().IsAa() && (*entry_it)->GetSeq().GetFirstId()->Match(*feat->GetProduct().GetId()))
            {
                ERASE_SEQENTRY_ON_SEQSET(entry_it, entry->SetSet());
            }
        }        
    }
}

void CCdsToMatPeptide::OnRadioButton(wxCommandEvent& event)
{
    if ( m_Merge->GetValue())
    {
        m_EntireRange->Enable();
        m_ProductFirst->Enable();
        m_ProductThis->Enable();
        m_ProductName->Enable(); 
        m_StringConstraintPanel->Enable();
        m_FeatureConstraint->Enable();
    }
    else
    {
        m_EntireRange->Disable();
        m_ProductFirst->Disable();
        m_ProductThis->Disable();
        m_ProductName->Disable();        
        m_StringConstraintPanel->Disable();
        m_FeatureConstraint->Disable();
    }
    event.Skip();
}

void CCdsToMatPeptide::FindAnnot(CRef<CSeq_entry> entry, const CSeq_feat* feat, CRef<CSeq_annot> &annot)
{
    CSeq_entry::TAnnot::iterator annot_it = entry->SetAnnot().begin();
    while ( annot_it != entry->SetAnnot().end() )
    {
        if ((*annot_it)->IsSetData() && (*annot_it)->GetData().IsFtable())
        {
            FOR_EACH_SEQFEAT_ON_SEQANNOT(feat_it, **annot_it)
            {
                if ((*feat_it)->IsSetData() && (*feat_it)->GetData().GetSubtype() == feat->GetData().GetSubtype() && feat->Compare(**feat_it) == 0)
                {
                    annot = *annot_it;
                    break;
                }
            }
       
        }
        if (annot)
            break;
        ++annot_it;
    }
  
    if (!annot && entry->IsSet())
    {
        FOR_EACH_SEQENTRY_ON_SEQSET(entry_it, entry->SetSet())
        {
            FindAnnot(*entry_it, feat, annot);
        }
    }
}

void CCdsToMatPeptide::ProcessUpdateFeatEvent( wxCommandEvent& event )
{
        UpdateChildrenFeaturePanels(this->GetSizer());
}

void CCdsToMatPeptide::UpdateChildrenFeaturePanels( wxSizer* sizer )
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


void CExplicit_Mapper_Sequence_Info::AddSeq(const CSeq_id_Handle& idh, TSeqType seq_type, TSeqPos seq_len)
{ 
    m_SeqType[idh] = seq_type; 
    m_SeqLen[idh] = seq_len;

    CConstRef<CSeq_id> id = idh.GetSeqId();
    CSeq_id::TSeqIdHandles matches;
    id->GetMatchingIds(matches);
    ITERATE(CSeq_id::TSeqIdHandles, mit, matches) 
    {
        m_SeqType[*mit] = seq_type; 
        m_SeqLen[*mit] = seq_len;
    }
}

IMapper_Sequence_Info::TSeqType CExplicit_Mapper_Sequence_Info::GetSequenceType(const CSeq_id_Handle& idh)
{ 
    return m_SeqType[idh]; 
}

TSeqPos CExplicit_Mapper_Sequence_Info::GetSequenceLength(const CSeq_id_Handle& idh)
{ 
    return m_SeqLen[idh]; 
}

void CExplicit_Mapper_Sequence_Info::CollectSynonyms(const CSeq_id_Handle& idh, TSynonyms&  synonyms)
{ 
    synonyms.insert(m_Synonyms[idh].begin(), m_Synonyms[idh].end());    
}

void CExplicit_Mapper_Sequence_Info::AddSynonym(const CSeq_id_Handle& idh1, const CSeq_id_Handle& idh2)
{
    m_Synonyms[idh1].insert(idh1);
    m_Synonyms[idh1].insert(idh2);
    for (set<CSeq_id_Handle>::iterator it = m_Synonyms[idh1].begin(); it != m_Synonyms[idh1].end(); ++it)
    {
        m_Synonyms[*it].insert(m_Synonyms[idh1].begin(), m_Synonyms[idh1].end());
    }
}
 
END_NCBI_SCOPE




