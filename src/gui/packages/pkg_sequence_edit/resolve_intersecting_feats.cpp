/*  $Id: resolve_intersecting_feats.cpp 34540 2016-01-21 21:06:51Z asztalos $
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
#include <gui/packages/pkg_sequence_edit/resolve_intersecting_feats.hpp>

#include <util/xregexp/regexp.hpp>

#include <wx/button.h>
#include <wx/statbox.h>
#include <wx/msgdlg.h>

////@begin XPM images
////@end XPM images

BEGIN_NCBI_SCOPE



IMPLEMENT_DYNAMIC_CLASS( CResolveIntersectingFeats, CBulkCmdDlg )


BEGIN_EVENT_TABLE( CResolveIntersectingFeats, CBulkCmdDlg )
EVT_UPDATE_FEATURE_LIST(wxID_ANY, CResolveIntersectingFeats::ProcessUpdateFeatEvent )
END_EVENT_TABLE()

CResolveIntersectingFeats::CResolveIntersectingFeats()
{
    Init();
}

CResolveIntersectingFeats::CResolveIntersectingFeats( wxWindow* parent, IWorkbench *wb, 
                                        wxWindowID id, const wxString& caption, const wxPoint& pos, const wxSize& size, long style )
    : CBulkCmdDlg(wb)
{
    Init();
    Create(parent, id, caption, pos, size, style);
}


bool CResolveIntersectingFeats::Create( wxWindow* parent, wxWindowID id, const wxString& caption, const wxPoint& pos, const wxSize& size, long style )
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


CResolveIntersectingFeats::~CResolveIntersectingFeats()
{
}


/*!
 * Member initialisation
 */

void CResolveIntersectingFeats::Init()
{
    m_FeatureType1 = NULL;
    m_FeatureConstraint1 = NULL;
    m_StringConstraintPanel1 = NULL;
    m_FeatureType2 = NULL;
    m_FeatureConstraint2 = NULL;
    m_StringConstraintPanel2 = NULL;
}




void CResolveIntersectingFeats::CreateControls()
{    
    CResolveIntersectingFeats* itemDialog1 = this;

    wxBoxSizer* itemBoxSizer2 = new wxBoxSizer(wxVERTICAL);
    itemDialog1->SetSizer(itemBoxSizer2);

    wxBoxSizer* itemBoxSizer7 = new wxBoxSizer(wxHORIZONTAL);
    itemBoxSizer2->Add(itemBoxSizer7, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 0);   

    wxBoxSizer* itemBoxSizer8 = new wxBoxSizer(wxVERTICAL);
    itemBoxSizer7->Add(itemBoxSizer8, 0, wxALL|wxGROW, 0);   

    wxStaticText* itemStaticText1 = new wxStaticText( itemDialog1, wxID_STATIC, _("Trim features of type"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer8->Add(itemStaticText1, 1, wxALIGN_TOP|wxALL, 5);  

    wxStaticText* itemStaticText2 = new wxStaticText( itemDialog1, wxID_STATIC, _("Where they overlap features of type"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer8->Add(itemStaticText2, 1, wxALIGN_TOP|wxALL, 5); 

    wxBoxSizer* itemBoxSizer9 = new wxBoxSizer(wxVERTICAL);
    itemBoxSizer7->Add(itemBoxSizer9, 0, wxALIGN_TOP|wxALL, 0);    

    wxBoxSizer* itemBoxSizer3 = new wxBoxSizer(wxHORIZONTAL);
    itemBoxSizer9->Add(itemBoxSizer3, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 0);   

    m_FeatureType1 = new CFeatureTypePanel(itemDialog1, wxID_ANY, wxDefaultPosition, wxDefaultSize, 0);
    itemBoxSizer3->Add(m_FeatureType1, 0, wxALIGN_TOP|wxALL, 5);   
    m_FeatureType1->ListPresentFeaturesFirst(m_TopSeqEntry);    

    wxStaticBox* itemStaticBoxSizer4Static = new wxStaticBox(itemDialog1, wxID_ANY, _("Constraints"));
    wxStaticBoxSizer* itemStaticBoxSizer4 = new wxStaticBoxSizer(itemStaticBoxSizer4Static, wxHORIZONTAL);
    itemBoxSizer3->Add(itemStaticBoxSizer4, 0, wxALIGN_TOP|wxALL, 5);

    m_FeatureConstraint1 = new CFeatureFieldNamePanel(itemDialog1, NULL, wxID_ANY, wxDefaultPosition, wxDefaultSize, 0);
    itemStaticBoxSizer4->Add(m_FeatureConstraint1, 0, wxALIGN_TOP|wxALL, 5);   
    m_FeatureConstraint1->PopulateFeatureListbox();

    m_StringConstraintPanel1 = new CStringConstraintPanel( itemDialog1, false, wxID_ANY, wxDefaultPosition, wxDefaultSize, 0 );
    itemStaticBoxSizer4->Add(m_StringConstraintPanel1, 0, wxALIGN_TOP|wxALL, 5); 

    wxBoxSizer* itemBoxSizer5 = new wxBoxSizer(wxHORIZONTAL);
    itemBoxSizer9->Add(itemBoxSizer5, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 0);   

    m_FeatureType2 = new CFeatureTypePanel(itemDialog1, wxID_ANY, wxDefaultPosition, wxDefaultSize, 0);
    itemBoxSizer5->Add(m_FeatureType2, 0, wxALIGN_TOP|wxALL, 5);   
    m_FeatureType2->ListPresentFeaturesFirst(m_TopSeqEntry);    

    wxStaticBox* itemStaticBoxSizer6Static = new wxStaticBox(itemDialog1, wxID_ANY, _("Constraints"));
    wxStaticBoxSizer* itemStaticBoxSizer6 = new wxStaticBoxSizer(itemStaticBoxSizer6Static, wxHORIZONTAL);
    itemBoxSizer5->Add(itemStaticBoxSizer6, 0, wxALIGN_TOP|wxALL, 5);

    m_FeatureConstraint2 = new CFeatureFieldNamePanel(itemDialog1, NULL, wxID_ANY, wxDefaultPosition, wxDefaultSize, 0);
    itemStaticBoxSizer6->Add(m_FeatureConstraint2, 0, wxALIGN_TOP|wxALL, 5);   
    m_FeatureConstraint2->PopulateFeatureListbox();

    m_StringConstraintPanel2 = new CStringConstraintPanel( itemDialog1, false, wxID_ANY, wxDefaultPosition, wxDefaultSize, 0 );
    itemStaticBoxSizer6->Add(m_StringConstraintPanel2, 0, wxALIGN_TOP|wxALL, 5); 

    COkCancelPanel *OkCancel = new COkCancelPanel( itemDialog1, ID_RESOLVE_INTERSECTING_FEATS_OKCANCEL, wxDefaultPosition, wxSize(100, 100), 0 );
    itemBoxSizer2->Add(OkCancel, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);
}


/*!
 * Should we show tooltips?
 */

bool CResolveIntersectingFeats::ShowToolTips()
{
    return true;
}

/*!
 * Get bitmap resources
 */

wxBitmap CResolveIntersectingFeats::GetBitmapResource( const wxString& name )
{
    // Bitmap retrieval
    wxUnusedVar(name);
    return wxNullBitmap;
}

/*!
 * Get icon resources
 */

wxIcon CResolveIntersectingFeats::GetIconResource( const wxString& name )
{
    // Icon retrieval
    wxUnusedVar(name);
    return wxNullIcon;
}

CRef<CCmdComposite> CResolveIntersectingFeats::GetCommand()
{
    CRef<CCmdComposite> cmd( new CCmdComposite("Resolve Feature Overlap") );

    string field_name1 = m_FeatureType1->GetFieldName(true);
    if (field_name1.empty()) 
        return cmd;
    field_name1  += " " + kPartialStart;

    CRef<edit::CStringConstraint> constraint1(m_StringConstraintPanel1->GetStringConstraint());
    string constraint_field1 = m_FeatureConstraint1->GetFieldName(false);
    
    CRef<CMiscSeqTableColumn> col1(new CMiscSeqTableColumn(field_name1));
    vector<CRef<edit::CApplyObject> > objects1 = col1->GetApplyObjects(m_TopSeqEntry, constraint_field1, constraint1);

    string field_name2 = m_FeatureType2->GetFieldName(true);
    if (field_name2.empty()) 
        return cmd;
    field_name2  += " " + kPartialStart;

    CRef<edit::CStringConstraint> constraint2(m_StringConstraintPanel2->GetStringConstraint());
    string constraint_field2 = m_FeatureConstraint2->GetFieldName(false);

    CRef<CMiscSeqTableColumn> col2(new CMiscSeqTableColumn(field_name2));
    vector<CRef<edit::CApplyObject> > objects2 = col2->GetApplyObjects(m_TopSeqEntry, constraint_field2, constraint2);

    CScope &scope = m_TopSeqEntry.GetScope();
    
    ITERATE(vector<CRef<edit::CApplyObject> >, obj_it1, objects1)
    {
        const CSeq_feat *feat1 = dynamic_cast<const CSeq_feat*> ( (*obj_it1)->GetOriginalObject() );
        if (feat1)
        {
            ITERATE(vector<CRef<edit::CApplyObject> >, obj_it2, objects2)
            {
                const CSeq_feat *feat2 = dynamic_cast<const CSeq_feat*> ( (*obj_it2)->GetOriginalObject() );
                if (feat2)
                {
                    if (feat1->IsSetLocation() && feat2->IsSetLocation() && sequence::TestForOverlap(feat1->GetLocation(), feat2->GetLocation(),sequence::eOverlap_Interval, kInvalidSeqPos, &scope) != -1)
                    {
                        CRef<CSeq_feat> new_feat(new CSeq_feat);
                        new_feat->Assign(*feat1);
                        CRef<CSeq_loc> new_loc = sequence::Seq_loc_Subtract(feat1->GetLocation(), feat2->GetLocation(), 0, &scope);
                        if (new_loc)
                        {
                            new_feat->SetLocation(*new_loc);
                            new_feat->SetPartial(new_feat->GetLocation().IsPartialStart(eExtreme_Biological) || new_feat->GetLocation().IsPartialStop(eExtreme_Biological));
                            CSeq_feat_Handle fh = scope.GetSeq_featHandle(*feat1);
                            cmd->AddCommand(*CRef<CCmdChangeSeq_feat>(new CCmdChangeSeq_feat(fh, *new_feat)));
                        }
                    }                    
                }
            }
        }
    }
  

    return cmd;
}


void CResolveIntersectingFeats::ProcessUpdateFeatEvent( wxCommandEvent& event )
{
        UpdateChildrenFeaturePanels(this->GetSizer());
}

void CResolveIntersectingFeats::UpdateChildrenFeaturePanels( wxSizer* sizer )
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
