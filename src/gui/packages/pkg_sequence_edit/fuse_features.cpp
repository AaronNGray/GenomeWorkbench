/*  $Id: fuse_features.cpp 42192 2019-01-10 16:52:15Z filippov $
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
#include <sstream>
#include <objmgr/scope.hpp>
#include <objects/misc/sequence_macros.hpp>
#include <objmgr/feat_ci.hpp>
#include <objmgr/seq_annot_ci.hpp>
#include <objmgr/bioseq_ci.hpp>
#include <objmgr/seq_feat_handle.hpp>
#include <objmgr/util/feature.hpp>
#include <objtools/edit/cds_fix.hpp>
#include <gui/objutils/util_cmds.hpp>
#include <gui/objutils/cmd_del_seq_annot.hpp>
#include <gui/objutils/cmd_change_seq_feat.hpp>
#include <gui/objutils/cmd_del_bioseq.hpp>
#include <gui/objutils/utils.hpp>
#include <gui/packages/pkg_sequence_edit/ok_cancel_panel.hpp>
#include <objtools/validator/utilities.hpp>
#include <gui/packages/pkg_sequence_edit/miscedit_util.hpp>
#include <gui/packages/pkg_sequence_edit/fuse_features.hpp>

#include <util/xregexp/regexp.hpp>

#include <wx/button.h>
#include <wx/statbox.h>
#include <wx/msgdlg.h>

////@begin XPM images
////@end XPM images

BEGIN_NCBI_SCOPE



IMPLEMENT_DYNAMIC_CLASS( CFuseFeaturesDlg, CBulkCmdDlg )


BEGIN_EVENT_TABLE( CFuseFeaturesDlg, CBulkCmdDlg )
EVT_UPDATE_FEATURE_LIST(wxID_ANY, CFuseFeaturesDlg::ProcessUpdateFeatEvent )
END_EVENT_TABLE()

CFuseFeaturesDlg::CFuseFeaturesDlg()
{
    Init();
}

CFuseFeaturesDlg::CFuseFeaturesDlg( wxWindow* parent, IWorkbench *wb, 
                                        wxWindowID id, const wxString& caption, const wxPoint& pos, const wxSize& size, long style )
    : CBulkCmdDlg(wb)
{
    Init();
    Create(parent, id, caption, pos, size, style);
}


bool CFuseFeaturesDlg::Create( wxWindow* parent, wxWindowID id, const wxString& caption, const wxPoint& pos, const wxSize& size, long style )
{
    SetExtraStyle(wxWS_EX_BLOCK_EVENTS);
    CBulkCmdDlg::Create( parent, id, caption, pos, size, style );

    CreateControls();
    if (GetSizer())
    {
        GetSizer()->SetSizeHints(this);
    }
    Centre();

  
    return true;
}


CFuseFeaturesDlg::~CFuseFeaturesDlg()
{
}


/*!
 * Member initialisation
 */

void CFuseFeaturesDlg::Init()
{
  
}




void CFuseFeaturesDlg::CreateControls()
{    
    CFuseFeaturesDlg* itemDialog1 = this;

    wxBoxSizer* itemBoxSizer2 = new wxBoxSizer(wxVERTICAL);
    itemDialog1->SetSizer(itemBoxSizer2);

    m_FeatureType = new CFeatureTypePanel(itemDialog1, wxID_ANY, wxDefaultPosition, wxDefaultSize, 0);
    itemBoxSizer2->Add(m_FeatureType, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 0);   
    m_FeatureType->ListPresentFeaturesFirst(m_TopSeqEntry);    

    m_FeatureConstraint = new CFeatureFieldNamePanel(itemDialog1, NULL, wxID_ANY, wxDefaultPosition, wxDefaultSize, 0);
    itemBoxSizer2->Add(m_FeatureConstraint, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 0);   
    m_FeatureConstraint->PopulateFeatureListbox();

    m_StringConstraintPanel = new CStringConstraintPanel( itemDialog1, false, wxID_ANY, wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer2->Add(m_StringConstraintPanel, 0, wxALIGN_LEFT|wxALL, 0); 

    COkCancelPanel *OkCancel = new COkCancelPanel( itemDialog1, ID_FUSE_FEATURES_OKCANCEL, wxDefaultPosition, wxSize(100, 100), 0 );
    itemBoxSizer2->Add(OkCancel, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);
}


/*!
 * Should we show tooltips?
 */

bool CFuseFeaturesDlg::ShowToolTips()
{
    return true;
}

/*!
 * Get bitmap resources
 */

wxBitmap CFuseFeaturesDlg::GetBitmapResource( const wxString& name )
{
    // Bitmap retrieval
    wxUnusedVar(name);
    return wxNullBitmap;
}

/*!
 * Get icon resources
 */

wxIcon CFuseFeaturesDlg::GetIconResource( const wxString& name )
{
    // Icon retrieval
    wxUnusedVar(name);
    return wxNullIcon;
}

CRef<CCmdComposite> CFuseFeaturesDlg::GetCommand()
{
    CRef<CCmdComposite> cmd( new CCmdComposite("Fuse Features") );
    bool modified = false;
    CRef<CCmdComposite> empty(NULL);
    CScope &scope = m_TopSeqEntry.GetScope();

    string field_name = m_FeatureType->GetFieldName(true);
    if (field_name.empty()) 
        return empty;
    int itype, isubtype;
    if (!CSeqFeatData::GetFeatList()->GetTypeSubType(field_name, itype, isubtype))
        return empty;
    CSeqFeatData::ESubtype subtype = static_cast<CSeqFeatData::ESubtype>(isubtype);

    if (NStr::IsBlank(field_name)) {   
        field_name = kPartialStart;
    } else {
        field_name += " " + kPartialStart;
    }

    vector<CSeq_feat_Handle> entry_handles;
    CRef<edit::CStringConstraint> constraint(m_StringConstraintPanel->GetStringConstraint());
    string constraint_field = m_FeatureConstraint->GetFieldName(false);
    if (constraint && !constraint_field.empty())
    {
        CRef<CMiscSeqTableColumn> col(new CMiscSeqTableColumn(field_name));
        vector<CRef<edit::CApplyObject> > objects = col->GetApplyObjects(m_TopSeqEntry, constraint_field, constraint);
        ITERATE(vector<CRef<edit::CApplyObject> >, obj_it, objects)
        {
            const CSeq_feat *feat = dynamic_cast<const CSeq_feat*> ( (*obj_it)->GetOriginalObject() );

            if (feat)
            {
                if (feat->IsSetData() && feat->GetData().IsCdregion() && 
                    feat->IsSetExcept_text() && NStr::Find(feat->GetExcept_text(), "RNA editing") != string::npos) 
                {
                    continue;
                }
                CSeq_feat_Handle fh = scope.GetSeq_featHandle(*feat,CScope::eMissing_Null);
                if (fh)
                {
                    entry_handles.push_back(fh);
                }
            }
        }
    }
    else
    {
        for (CFeat_CI feat(m_TopSeqEntry, SAnnotSelector(subtype)); feat; ++feat)
        {
            const CSeq_feat& cds = feat->GetOriginalFeature();
            if (cds.IsSetData() && cds.GetData().IsCdregion() && 
                cds.IsSetExcept_text() && NStr::Find(cds.GetExcept_text(), "RNA editing") != string::npos) 
            {
                continue;
            }
            entry_handles.push_back(feat->GetSeq_feat_Handle());
        }
    }

 
    set< CSeq_feat_Handle > deleted_feats;
    int offset = 1;
    bool create_general_only = objects::edit::IsGeneralIdProtPresent(m_TopSeqEntry);
    for (size_t i = 0; i < entry_handles.size(); i++)
    {
        CSeq_feat_Handle feat1 = entry_handles[i];
        if (deleted_feats.find(feat1) == deleted_feats.end())
        {
            CRef<CSeq_loc> new_loc(new CSeq_loc);
            new_loc->Assign(feat1.GetLocation());
            bool partial_start = new_loc->IsPartialStart(eExtreme_Positional);
            bool partial_stop = new_loc->IsPartialStop(eExtreme_Positional);
            new_loc->SetPartialStart(false,eExtreme_Positional);
            new_loc->SetPartialStop(false,eExtreme_Positional);
            bool changed = false;
            CRef<CSeq_id> new_product_id(NULL);
            if (feat1.IsSetProduct()) {
                new_product_id.Reset(new CSeq_id());
                new_product_id->Assign(*(feat1.GetProduct().GetId()));
            }
            for (size_t j = i + 1; j < entry_handles.size(); j++)
            {
                CSeq_feat_Handle feat2 = entry_handles[j];
                if ( deleted_feats.find(feat2) == deleted_feats.end() &&
                     sequence::IsSameBioseq(*feat1.GetLocation().GetId(),*feat2.GetLocation().GetId(),&scope))
                {                   
                    CRef<CSeq_loc> add_loc(new CSeq_loc);
                    add_loc->Assign(feat2.GetLocation());
                    partial_stop = add_loc->IsPartialStop(eExtreme_Positional);
                    add_loc->SetPartialStart(false,eExtreme_Positional);
                    add_loc->SetPartialStop(false,eExtreme_Positional);
                    new_loc->Assign(*(sequence::Seq_loc_Add(*new_loc, *add_loc, CSeq_loc::fMerge_All|CSeq_loc::fSort, &scope)));
                    cmd->AddCommand(*CRef<CCmdDelSeq_feat>(new CCmdDelSeq_feat(feat2)));
                    if (feat2.IsSetProduct()) {
                        if (!new_product_id) {
                            new_product_id.Reset(new CSeq_id());
                            new_product_id->Assign(*(feat2.GetProduct().GetId()));
                        } else {
                            CBioseq_Handle product = scope.GetBioseqHandle(feat2.GetProduct());
                            if (product) {
                                cmd->AddCommand(*CRef<CCmdDelBioseqInst>(new CCmdDelBioseqInst(product)));
                            }
                        }
                    }
                    deleted_feats.insert(feat2);
                    changed = true;
                }
            }
            if (changed)
            {
                CRef<CSeq_feat> new_feat(new CSeq_feat);
                new_feat->Assign(*feat1.GetOriginalSeq_feat());
                new_loc->SetPartialStart(partial_start,eExtreme_Positional);
                new_loc->SetPartialStop(partial_stop,eExtreme_Positional);
                new_feat->SetLocation(*new_loc);                
                new_feat->SetPartial(partial_start || partial_stop);
                if (!new_feat->IsSetProduct() && new_product_id) {
                    new_feat->SetProduct().SetWhole().Assign(*new_product_id);
                }
                cmd->AddCommand(*CRef< CCmdChangeSeq_feat > (new CCmdChangeSeq_feat(feat1,*new_feat)));
                if (subtype == CSeqFeatData::eSubtype_cdregion && new_feat->IsSetProduct())
                {
                    CRef<CCmdComposite> retranslate_cmd = GetRetranslateCDSCommand(scope, *new_feat, offset, create_general_only);
                    cmd->AddCommand(*retranslate_cmd);
                }
                modified = true;
            }
        }
    }
    
    if (modified)
        return cmd;
    return empty;
}


void CFuseFeaturesDlg::ProcessUpdateFeatEvent( wxCommandEvent& event )
{
        UpdateChildrenFeaturePanels(this->GetSizer());
}

void CFuseFeaturesDlg::UpdateChildrenFeaturePanels( wxSizer* sizer )
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


CRef<CCmdComposite> CFuseJoinsInLocs::apply(CSeq_entry_Handle seh)
{
    CRef<CCmdComposite> composite(new CCmdComposite("Fuse Joins in Feature Locations"));
    bool modified = false;
    for (CFeat_CI feat_it(seh); feat_it; ++feat_it)
    {
        if (feat_it->GetOriginalFeature().IsSetData())
        {
            CRef<CSeq_feat> new_feat(new CSeq_feat);
            new_feat->Assign(feat_it->GetOriginalFeature());
            const CSeq_loc& feat_loc = feat_it->GetLocation();
            CRef<CSeq_loc> new_loc(new CSeq_loc);
            new_loc->Assign(feat_loc);
            vector<pair<int,int> > ranges;
            for (CSeq_loc_CI loc_ci(feat_loc, CSeq_loc_CI::eEmpty_Skip, CSeq_loc_CI::eOrder_Positional); loc_ci;  ++loc_ci)
            {
                CSeq_loc_CI::TRange feat_range = loc_ci.GetRange();
                TSeqPos feat_start = feat_range.GetFrom();
                TSeqPos feat_stop = feat_range.GetTo();
                ranges.push_back(pair<int, int>(feat_start, feat_stop));
            }
            size_t i = 0;
            for (size_t j = 1; j < ranges.size(); j++)
            {
                if (ranges[j].first == ranges[i].second + 1)
                {
                    ranges[i].second = ranges[j].second;
                    ranges[j].first  = -1;
                    ranges[j].second = -1;
                    continue;
                }
                i = j;
            }

            CSeq_loc_I loc_it(*new_loc); // , CSeq_loc_CI::eEmpty_Skip, CSeq_loc_CI::eOrder_Positional
            i = 0;
            while(loc_it)      
            {
                if (loc_it.IsEmpty())
                {
                    ++loc_it;
                    continue;
                }
                CSeq_loc_CI::TRange feat_range = loc_it.GetRange();
                TSeqPos feat_start = feat_range.GetFrom();
                TSeqPos feat_stop = feat_range.GetTo();
                int new_start = ranges[i].first;
                int new_stop = ranges[i].second;
                if (new_start < 0 || new_stop < 0)
                {
                    loc_it.Delete();
                    ++i;               
                    continue;
                }
                if (feat_start != new_start)
                {
                    loc_it.SetFrom(new_start);
                }
                if (feat_stop != new_stop)
                {
                    loc_it.SetTo(new_stop);
                }
                ++loc_it;
                ++i;
            }
            CRef<CSeq_loc> loc = loc_it.MakeSeq_loc();
            if (loc_it.HasChanges())
            {
                new_feat->SetLocation(*loc);                
                CRef< CCmdChangeSeq_feat > cmd(new CCmdChangeSeq_feat(feat_it->GetSeq_feat_Handle(),*new_feat));
                if (cmd) 
                {
                    composite->AddCommand(*cmd);
                    modified = true;
                }
                
            }
        }
    }
    if (!modified)
        composite.Reset();
    return composite;
}


END_NCBI_SCOPE
