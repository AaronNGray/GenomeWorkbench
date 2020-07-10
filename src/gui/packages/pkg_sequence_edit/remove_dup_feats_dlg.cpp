/*  $Id: remove_dup_feats_dlg.cpp 40679 2018-03-28 21:45:29Z filippov $
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
#include <gui/objutils/util_cmds.hpp>
#include <gui/objutils/cmd_del_seq_annot.hpp>
#include <gui/objutils/cmd_change_seq_feat.hpp>
#include <gui/objutils/utils.hpp>
#include <gui/packages/pkg_sequence_edit/ok_cancel_panel.hpp>
#include <objtools/validator/utilities.hpp>
#include <gui/packages/pkg_sequence_edit/miscedit_util.hpp>
#include <gui/packages/pkg_sequence_edit/remove_dup_feats_dlg.hpp>

#include <util/xregexp/regexp.hpp>

#include <wx/button.h>
#include <wx/statbox.h>
#include <wx/msgdlg.h>

////@begin XPM images
////@end XPM images

BEGIN_NCBI_SCOPE



IMPLEMENT_DYNAMIC_CLASS( CRmDupFeaturesDlg, CBulkCmdDlg )


BEGIN_EVENT_TABLE( CRmDupFeaturesDlg, CBulkCmdDlg )
EVT_UPDATE_FEATURE_LIST(wxID_ANY, CRmDupFeaturesDlg::ProcessUpdateFeatEvent )
END_EVENT_TABLE()

CRmDupFeaturesDlg::CRmDupFeaturesDlg()
{
    Init();
}

CRmDupFeaturesDlg::CRmDupFeaturesDlg( wxWindow* parent, IWorkbench *wb, 
                                        wxWindowID id, const wxString& caption, const wxPoint& pos, const wxSize& size, long style )
    : CBulkCmdDlg(wb)
{
    Init();
    Create(parent, id, caption, pos, size, style);
}


bool CRmDupFeaturesDlg::Create( wxWindow* parent, wxWindowID id, const wxString& caption, const wxPoint& pos, const wxSize& size, long style )
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


CRmDupFeaturesDlg::~CRmDupFeaturesDlg()
{
}


/*!
 * Member initialisation
 */

void CRmDupFeaturesDlg::Init()
{
    m_FeatureType = NULL;
    m_RemoveProteins = NULL;
    m_FeatureConstraint = NULL;
    m_StringConstraintPanel = NULL;
}




void CRmDupFeaturesDlg::CreateControls()
{    
    CRmDupFeaturesDlg* itemDialog1 = this;

    wxBoxSizer* itemBoxSizer2 = new wxBoxSizer(wxVERTICAL);
    itemDialog1->SetSizer(itemBoxSizer2);

    m_FeatureType = new CFeatureTypePanel(itemDialog1, wxID_ANY, wxDefaultPosition, wxDefaultSize, 0);
    itemBoxSizer2->Add(m_FeatureType, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 0);   
    m_FeatureType->ListPresentFeaturesFirst(m_TopSeqEntry);    

    m_CheckPartials = new wxCheckBox( itemDialog1, wxID_ANY, _("Check partials"), wxDefaultPosition, wxDefaultSize, 0 );
    m_CheckPartials->SetValue(false);
    itemBoxSizer2->Add(m_CheckPartials, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 0);   

    m_CaseSensitive = new wxCheckBox( itemDialog1, wxID_ANY, _("Case sensitive"), wxDefaultPosition, wxDefaultSize, 0 );
    m_CaseSensitive->SetValue(false);
    itemBoxSizer2->Add(m_CaseSensitive, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 0);   

    m_RemoveProteins = new wxCheckBox( itemDialog1, wxID_ANY, _("Remove proteins"), wxDefaultPosition, wxDefaultSize, 0 );
    m_RemoveProteins->SetValue(true);
    itemBoxSizer2->Add(m_RemoveProteins, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 0);   

    m_FeatureConstraint = new CFeatureFieldNamePanel(itemDialog1, NULL, wxID_ANY, wxDefaultPosition, wxDefaultSize, 0);
    itemBoxSizer2->Add(m_FeatureConstraint, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 0);   
    m_FeatureConstraint->PopulateFeatureListbox();

    m_StringConstraintPanel = new CStringConstraintPanel( itemDialog1, false, wxID_ANY, wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer2->Add(m_StringConstraintPanel, 0, wxALIGN_LEFT|wxALL, 0); 

    COkCancelPanel *OkCancel = new COkCancelPanel( itemDialog1, ID_RM_DUP_FEATURES_OKCANCEL, wxDefaultPosition, wxSize(100, 100), 0 );
    itemBoxSizer2->Add(OkCancel, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);
}


/*!
 * Should we show tooltips?
 */

bool CRmDupFeaturesDlg::ShowToolTips()
{
    return true;
}

/*!
 * Get bitmap resources
 */

wxBitmap CRmDupFeaturesDlg::GetBitmapResource( const wxString& name )
{
    // Bitmap retrieval
    wxUnusedVar(name);
    return wxNullBitmap;
}

/*!
 * Get icon resources
 */

wxIcon CRmDupFeaturesDlg::GetIconResource( const wxString& name )
{
    // Icon retrieval
    wxUnusedVar(name);
    return wxNullIcon;
}

CRef<CCmdComposite> CRmDupFeaturesDlg::GetCommand()
{
    CRef<CCmdComposite> cmd( new CCmdComposite("Remove Duplicate Features") );
    bool modified = false;
    CRef<CCmdComposite> empty(NULL);
   

    string field_name = m_FeatureType->GetFieldName(true);
    if (field_name.empty()) 
        return empty;
    int itype, isubtype;
    if (!CSeqFeatData::GetFeatList()->GetTypeSubType(field_name, itype, isubtype))
        return empty;
    CSeqFeatData::ESubtype subtype = static_cast<CSeqFeatData::ESubtype>(isubtype);

    vector<CSeq_entry_Handle> entry_handles;
    CRef<edit::CStringConstraint> constraint(m_StringConstraintPanel->GetStringConstraint());
    string constraint_field = m_FeatureConstraint->GetFieldName(false);
    if (constraint && !constraint_field.empty())
    {
        CRef<CMiscSeqTableColumn> col(new CMiscSeqTableColumn(constraint_field));
        vector<CRef<edit::CApplyObject> > objects = col->GetApplyObjects(m_TopSeqEntry, constraint_field, constraint);
        ITERATE(vector<CRef<edit::CApplyObject> >, obj_it, objects)
        {
            const CSeq_feat *feat = dynamic_cast<const CSeq_feat*> ( (*obj_it)->GetOriginalObject() );
            if (feat)
            {
                CBioseq_Handle bsh = m_TopSeqEntry.GetScope().GetBioseqHandle(feat->GetLocation());
                if (bsh)
                {
                    entry_handles.push_back(bsh.GetSeq_entry_Handle());
                }
            }
        }
    }
    else
    {
        entry_handles.push_back(m_TopSeqEntry);
    }

    bool remove_proteins = m_RemoveProteins->GetValue();
    map<objects::CBioseq_Handle, set<objects::CSeq_feat_Handle> > product_to_cds;
    GetProductToCDSMap(m_TopSeqEntry.GetScope(), product_to_cds);
    ITERATE(vector<CSeq_entry_Handle>, seh_it, entry_handles)
    {
        set< CSeq_feat_Handle > deleted_feats;
        for (CFeat_CI feat1(*seh_it, SAnnotSelector(subtype)); feat1; ++feat1)
        {
            for (CFeat_CI feat2(*seh_it, SAnnotSelector(subtype)); feat2; ++feat2)
            {
                if (feat1->GetSeq_feat_Handle() < feat2->GetSeq_feat_Handle() && 
                    deleted_feats.find(feat1->GetSeq_feat_Handle()) == deleted_feats.end() &&
                    deleted_feats.find(feat2->GetSeq_feat_Handle()) == deleted_feats.end() &&
                    validator::IsDuplicate(feat1->GetSeq_feat_Handle(),feat2->GetSeq_feat_Handle(), m_CheckPartials->GetValue(), m_CaseSensitive->GetValue()) != validator::eDuplicate_Not )
                {
                    cmd->AddCommand(*GetDeleteFeatureCommand(feat2->GetSeq_feat_Handle(), remove_proteins, product_to_cds));
                    deleted_feats.insert(feat2->GetSeq_feat_Handle());
                    modified = true;                    
                }
            }
        }
    }
    if (modified)
        return cmd;
    return empty;
}


void CRmDupFeaturesDlg::ProcessUpdateFeatEvent( wxCommandEvent& event )
{
        UpdateChildrenFeaturePanels(this->GetSizer());
}

void CRmDupFeaturesDlg::UpdateChildrenFeaturePanels( wxSizer* sizer )
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
