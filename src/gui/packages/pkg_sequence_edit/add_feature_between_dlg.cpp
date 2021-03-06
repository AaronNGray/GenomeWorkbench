/*  $Id: add_feature_between_dlg.cpp 45101 2020-05-29 20:53:24Z asztalos $
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
#include <objects/seqfeat/Imp_feat.hpp>
#include <objects/seqfeat/Seq_feat.hpp>
#include <objmgr/bioseq_ci.hpp>
#include <objmgr/feat_ci.hpp>
#include <objmgr/bioseq_handle.hpp>

////@begin includes
////@end includes
#include <gui/core/selection_service_impl.hpp>
#include <gui/packages/pkg_sequence_edit/add_feature_between_dlg.hpp>
#include <gui/packages/pkg_sequence_edit/igspanel.hpp>
#include <gui/objutils/cmd_create_feat.hpp>
#include <gui/widgets/edit/feature_type_panel.hpp>
#include <gui/packages/pkg_sequence_edit/bulk_other_add_panel.hpp>
#include <gui/packages/pkg_sequence_edit/bulk_rna_add_panel.hpp>
#include <gui/packages/pkg_sequence_edit/bulkcdsaddpanel.hpp>
#include <gui/packages/pkg_sequence_edit/miscedit_util.hpp>
#include <gui/packages/pkg_sequence_edit/subprep_util.hpp>
#include <gui/packages/pkg_sequence_edit/bulk_location_panel.hpp>
#include <gui/objutils/utils.hpp>

#include <wx/msgdlg.h>

#include <gui/packages/pkg_sequence_edit/editing_actions.hpp>
#include <gui/packages/pkg_sequence_edit/editing_action_constraint.hpp>

////@begin XPM images
////@end XPM images

BEGIN_NCBI_SCOPE

/*!
 * CAddFeatureBetweenDlg type definition
 */

IMPLEMENT_DYNAMIC_CLASS( CAddFeatureBetweenDlg, CBulkCmdDlg )


/*!
 * CAddFeatureBetweenDlg event table definition
 */

BEGIN_EVENT_TABLE( CAddFeatureBetweenDlg, CBulkCmdDlg )

////@begin CAddFeatureBetweenDlg event table entries
EVT_BUTTON( ID_APPLY_BUTTON, CAddFeatureBetweenDlg::OnApplyButton )
////@end CAddFeatureBetweenDlg event table entries

END_EVENT_TABLE()


/*!
 * CAddFeatureBetweenDlg constructors
 */

CAddFeatureBetweenDlg::CAddFeatureBetweenDlg()
{
    Init();
}

CAddFeatureBetweenDlg::CAddFeatureBetweenDlg( wxWindow* parent, objects::CSeq_entry_Handle seh, IWorkbench* workbench, 
                             wxWindowID id, const wxString& caption, const wxPoint& pos, const wxSize& size, long style )
    : m_TopSeqEntry(seh), m_Workbench(workbench)
{
    Init();
    Create(parent, id, caption, pos, size, style);
}


/*!
 * CAddFeatureBetweenDlg creator
 */

bool CAddFeatureBetweenDlg::Create( wxWindow* parent, wxWindowID id, const wxString& caption, const wxPoint& pos, const wxSize& size, long style )
{
////@begin CAddFeatureBetweenDlg creation
    SetExtraStyle(wxWS_EX_BLOCK_EVENTS);
    CBulkCmdDlg::Create( parent, id, caption, pos, size, style );

    CreateControls();
    if (GetSizer())
    {
        GetSizer()->SetSizeHints(this);
    }
    Centre();
////@end CAddFeatureBetweenDlg creation

    return true;
}


/*!
 * CAddFeatureBetweenDlg destructor
 */

CAddFeatureBetweenDlg::~CAddFeatureBetweenDlg()
{
////@begin CAddFeatureBetweenDlg destruction
////@end CAddFeatureBetweenDlg destruction
}


/*!
 * Member initialisation
 */

void CAddFeatureBetweenDlg::Init()
{
////@begin CAddFeatureBetweenDlg member initialisation
    m_Notebook = NULL;
    m_FeatLeftConstraint = NULL;
    m_FeatRightConstraint = NULL;
    m_StringConstraintPanelLeft = NULL;
    m_StringConstraintPanelRight = NULL;
////@end CAddFeatureBetweenDlg member initialisation
    x_ListFeatures();
}


/*!
 * Control creation for CAddFeatureBetweenDlg
 */

void CAddFeatureBetweenDlg::CreateControls()
{    
////@begin CAddFeatureBetweenDlg content construction
    // Generated by DialogBlocks, 10/06/2013 17:47:57 (unregistered)

    CAddFeatureBetweenDlg* itemCBulkCmdDlg1 = this;

    wxBoxSizer* itemBoxSizer2 = new wxBoxSizer(wxVERTICAL);
    itemCBulkCmdDlg1->SetSizer(itemBoxSizer2);

    wxBoxSizer* itemBoxSizer3 = new wxBoxSizer(wxHORIZONTAL);
    itemBoxSizer2->Add(itemBoxSizer3, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

    wxBoxSizer* LeftOptions = new wxBoxSizer(wxVERTICAL);
    itemBoxSizer3->Add(LeftOptions, 0, wxALIGN_TOP|wxALL, 5);

    wxStaticText* itemStaticText5 = new wxStaticText( itemCBulkCmdDlg1, wxID_STATIC, _("Left Feature"), wxDefaultPosition, wxDefaultSize, 0 );
    LeftOptions->Add(itemStaticText5, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

    m_NoFeatPartialLeft = new wxRadioButton( itemCBulkCmdDlg1, wxID_ANY, _("if selected feature is missing, extend to partial end"), wxDefaultPosition, wxDefaultSize, wxRB_GROUP);
    m_NoFeatPartialLeft->SetValue(true);
    LeftOptions->Add(m_NoFeatPartialLeft, 0, wxALIGN_LEFT|wxALL, 5);

    m_NoFeatNoFeatLeft = new wxRadioButton( itemCBulkCmdDlg1, wxID_ANY, _("if selected feature is missing, do not create feature"));
    m_NoFeatNoFeatLeft->SetValue(false);
    LeftOptions->Add(m_NoFeatNoFeatLeft, 0, wxALIGN_LEFT|wxALL, 5);

    m_ExtendCompleteLeft = new wxRadioButton( itemCBulkCmdDlg1, wxID_ANY, _("extend to complete end"));
    m_ExtendCompleteLeft->SetValue(false);
    LeftOptions->Add(m_ExtendCompleteLeft, 0, wxALIGN_LEFT|wxALL, 5);

    m_ExtendPartialLeft = new wxRadioButton( itemCBulkCmdDlg1, wxID_ANY, _("extend to partial end"));
    m_ExtendPartialLeft->SetValue(false);
    LeftOptions->Add(m_ExtendPartialLeft, 0, wxALIGN_LEFT|wxALL, 5);

    m_FeatureTypeLeft = new wxListBox( itemCBulkCmdDlg1, wxID_ANY, wxDefaultPosition, wxDefaultSize, m_FeatureTypeStrings, wxLB_SINGLE );
    LeftOptions->Add(m_FeatureTypeLeft, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);
    m_FeatureTypeLeft->Bind(wxEVT_LISTBOX, &CAddFeatureBetweenDlg::OnFeatLeftChanged, this);

    m_FeatLeftConstraint =  new CFeatureFieldNamePanel(itemCBulkCmdDlg1, NULL, wxID_ANY, wxDefaultPosition, wxDefaultSize, 0);
    LeftOptions->Add(m_FeatLeftConstraint, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);
    m_FeatLeftConstraint->ListPresentFeaturesFirst(m_TopSeqEntry);

    m_StringConstraintPanelLeft = new CStringConstraintPanel(itemCBulkCmdDlg1, false, wxID_ANY, wxDefaultPosition, wxDefaultSize, 0);
    LeftOptions->Add(m_StringConstraintPanelLeft, 0, wxALIGN_CENTER_HORIZONTAL|wxFIXED_MINSIZE|wxALL, 0);

    wxStaticLine *line1 = new wxStaticLine (itemCBulkCmdDlg1, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxLI_VERTICAL);
    itemBoxSizer3->Add(line1, 0, wxALIGN_CENTER_VERTICAL|wxALL|wxSHAPED, 5);

    wxBoxSizer* FeatureOptions = new wxBoxSizer(wxVERTICAL);
    itemBoxSizer3->Add(FeatureOptions, 0, wxALIGN_TOP|wxALL, 5);

    wxStaticText* itemStaticText6 = new wxStaticText( itemCBulkCmdDlg1, wxID_STATIC, _("Feature To Add"), wxDefaultPosition, wxDefaultSize, 0 );
    FeatureOptions->Add(itemStaticText6, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

    m_Notebook = new wxNotebook(itemCBulkCmdDlg1, wxID_ANY,wxDefaultPosition,wxDefaultSize);

    FeatureOptions->Add(m_Notebook, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

    CBulkRNAAddPanel *rnaPanel = new CBulkRNAAddPanel( m_Notebook, m_TopSeqEntry, false);
    m_Notebook->AddPage(rnaPanel,_("RNA"));

    CBulkCDSAddPanel *cdsPanel = new CBulkCDSAddPanel( m_Notebook, m_TopSeqEntry, false);
    m_Notebook->AddPage(cdsPanel,_("CDS"));

    CBulkOtherAddPanel *otherPanel = new CBulkOtherAddPanel( m_Notebook, m_TopSeqEntry, "", false);
    m_Notebook->AddPage(otherPanel,_("Other"));
     

    wxStaticLine *line2 = new wxStaticLine (itemCBulkCmdDlg1, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxLI_VERTICAL);
    itemBoxSizer3->Add(line2, 0, wxALIGN_CENTER_VERTICAL|wxALL|wxSHAPED, 5);

    wxBoxSizer* RightOptions = new wxBoxSizer(wxVERTICAL);
    itemBoxSizer3->Add(RightOptions, 0, wxALIGN_TOP|wxALL, 5);

    wxStaticText* itemStaticText7 = new wxStaticText( itemCBulkCmdDlg1, wxID_STATIC, _("Right Feature"), wxDefaultPosition, wxDefaultSize, 0 );
    RightOptions->Add(itemStaticText7, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

    m_NoFeatPartialRight = new wxRadioButton( itemCBulkCmdDlg1, wxID_ANY, _("if selected feature is missing, extend to partial end"), wxDefaultPosition, wxDefaultSize, wxRB_GROUP);
    m_NoFeatPartialRight->SetValue(true);
    RightOptions->Add(m_NoFeatPartialRight, 0, wxALIGN_LEFT|wxALL, 5);

    m_NoFeatNoFeatRight = new wxRadioButton( itemCBulkCmdDlg1, wxID_ANY, _("if selected feature is missing, do not create feature"));
    m_NoFeatNoFeatRight->SetValue(false);
    RightOptions->Add(m_NoFeatNoFeatRight, 0, wxALIGN_LEFT|wxALL, 5);

    m_ExtendCompleteRight = new wxRadioButton( itemCBulkCmdDlg1, wxID_ANY, _("extend to complete end"));
    m_ExtendCompleteRight->SetValue(false);
    RightOptions->Add(m_ExtendCompleteRight, 0, wxALIGN_LEFT|wxALL, 5);

    m_ExtendPartialRight = new wxRadioButton( itemCBulkCmdDlg1, wxID_ANY, _("extend to partial end"));
    m_ExtendPartialRight->SetValue(false);
    RightOptions->Add(m_ExtendPartialRight, 0, wxALIGN_LEFT|wxALL, 5);

    m_FeatureTypeRight = new wxListBox( itemCBulkCmdDlg1, wxID_ANY, wxDefaultPosition, wxDefaultSize, m_FeatureTypeStrings, wxLB_SINGLE );
    RightOptions->Add(m_FeatureTypeRight, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);
    m_FeatureTypeRight->Bind(wxEVT_LISTBOX, &CAddFeatureBetweenDlg::OnFeatRightChanged, this);

    m_FeatRightConstraint =  new CFeatureFieldNamePanel(itemCBulkCmdDlg1, NULL, wxID_ANY, wxDefaultPosition, wxDefaultSize, 0);
    RightOptions->Add(m_FeatRightConstraint, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);
    m_FeatRightConstraint->ListPresentFeaturesFirst(m_TopSeqEntry);

    m_StringConstraintPanelRight = new CStringConstraintPanel(itemCBulkCmdDlg1, false, wxID_ANY, wxDefaultPosition, wxDefaultSize, 0);
    RightOptions->Add(m_StringConstraintPanelRight, 0, wxALIGN_CENTER_HORIZONTAL|wxFIXED_MINSIZE|wxALL, 0);

    wxBoxSizer* itemBoxSizer5 = new wxBoxSizer(wxHORIZONTAL);
    itemBoxSizer2->Add(itemBoxSizer5, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

    wxButton* itemButton6 = new wxButton( itemCBulkCmdDlg1, ID_APPLY_BUTTON, _("Accept"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer5->Add(itemButton6, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxButton* itemButton7 = new wxButton( itemCBulkCmdDlg1, wxID_CANCEL, _("Cancel"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer5->Add(itemButton7, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    m_LeaveUp = new wxCheckBox (itemCBulkCmdDlg1, wxID_ANY, _("Leave dialog up"));
    itemBoxSizer5->Add(m_LeaveUp, 0, wxALIGN_LEFT|wxALL, 5);   
////@end CAddFeatureBetweenDlg content construction
}

void CAddFeatureBetweenDlg::x_ListFeatures()
{
    if (!m_TopSeqEntry)
        return;

    set<string> existing;
    m_Subtype.clear();
    m_FeatureTypeStrings.Clear();

    existing.insert("All");

    vector<const CFeatListItem *> featlist = GetSortedFeatList(m_TopSeqEntry);
    ITERATE(vector<const CFeatListItem *>, feat_it, featlist) {
        const CFeatListItem& item = **feat_it;
        string desc = item.GetDescription();
        int feat_type = item.GetType();
        CSeqFeatData::ESubtype feat_subtype = (CSeqFeatData::ESubtype)item.GetSubtype();
        if (!CFeatureTypePanel::s_IsRarelyUsedOrDiscouragedFeatureType(feat_subtype) && !CSeqFeatData::IsRegulatory(feat_subtype)) {
            if (existing.find(desc) == existing.end()) {
                existing.insert(desc);
                m_Subtype.push_back(feat_subtype);
                m_FeatureTypeStrings.Add(ToWxString(desc));
            }
        }
    }
}

void CAddFeatureBetweenDlg::GetFeatureLocations(vector<CRef<objects::CSeq_loc> > &locs)
{
    locs.clear();
    int  s1 = m_FeatureTypeLeft->GetSelection();
    int  s2 = m_FeatureTypeRight->GetSelection();
    
    objects::CSeqFeatData::ESubtype subtype1 = objects::CSeqFeatData::eSubtype_bad;
    if ( (m_NoFeatPartialLeft->GetValue() || m_NoFeatNoFeatLeft->GetValue())
         && s1 != wxNOT_FOUND)
    {
        subtype1 = m_Subtype[s1];
    }
    objects::CSeqFeatData::ESubtype subtype2 = objects::CSeqFeatData::eSubtype_bad;
    if ( (m_NoFeatPartialRight->GetValue() || m_NoFeatNoFeatRight->GetValue())
         && s2 != wxNOT_FOUND)
    {
        subtype2 = m_Subtype[s2];
    } 

    CRef<CEditingActionConstraint> constraint_left;
    if (m_StringConstraintPanelLeft->GetStringConstraint())
    {
        constraint_left = CreateEditingActionConstraint( m_FeatureTypeLeft->GetStringSelection().ToStdString(), 
                                                         CFieldNamePanel::eFieldType_Feature, 
                                                         subtype1, 
                                                         m_FeatLeftConstraint->GetFieldName(true), 
                                                         CFieldNamePanel::eFieldType_Feature, 
                                                         GetSubtype(m_FeatLeftConstraint),
                                                         CRef<CConstraintMatcher>(new CSimpleConstraintMatcher(m_StringConstraintPanelLeft->GetStringConstraint())));
    }

    CRef<CEditingActionConstraint> constraint_right; 
    if (m_StringConstraintPanelRight->GetStringConstraint())
    {
        constraint_right = CreateEditingActionConstraint( m_FeatureTypeRight->GetStringSelection().ToStdString(), 
                                                          CFieldNamePanel::eFieldType_Feature, 
                                                          subtype2, 
                                                          m_FeatRightConstraint->GetFieldName(true), 
                                                          CFieldNamePanel::eFieldType_Feature, 
                                                          GetSubtype(m_FeatRightConstraint),
                                                          CRef<CConstraintMatcher>(new CSimpleConstraintMatcher(m_StringConstraintPanelRight->GetStringConstraint())));
    }

    for (CBioseq_CI bsi(m_TopSeqEntry, CSeq_inst::eMol_na); bsi; ++bsi)
    {

        bool partial_left = true;
        bool partial_right = true;
        bool found = false;

        TSignedSeqPos length = bsi->GetBioseqLength();
        if (length == 0)
            continue;

        if (subtype1 != objects::CSeqFeatData::eSubtype_bad)
        {
            for (objects::CFeat_CI feat1(*bsi, objects::SAnnotSelector(subtype1)); feat1; ++feat1)                
            {
                if (constraint_left && !constraint_left->Match(*feat1))
                    continue;
                const CSeq_loc &loc1 = feat1->GetLocation();
                TSignedSeqPos start1 = feat1->GetLocationTotalRange().GetFrom();
                TSignedSeqPos stop1 = feat1->GetLocationTotalRange().GetTo();
                partial_left = false;
                TSignedSeqPos feat_left = stop1+1;
                TSignedSeqPos feat_right = length - 1;
            
                if (subtype2 != objects::CSeqFeatData::eSubtype_bad)
                    for (objects::CFeat_CI feat2(*bsi, objects::SAnnotSelector(subtype2)); feat2 && feat_right == length-1; ++feat2)
                    {
                        if (constraint_right && !constraint_right->Match(*feat2))
                            continue;
                        const CSeq_loc &loc2 = feat2->GetLocation();
                        TSignedSeqPos start2 = feat2->GetLocationTotalRange().GetFrom();
                        TSignedSeqPos stop2 = feat2->GetLocationTotalRange().GetTo();
                        if (start2 >= feat_left)
                        {
                            partial_right = false;
                            feat_right = start2-1;
                        }
                    }

                if ( feat_right >= feat_left  && (feat_right < length - 1 || !m_NoFeatNoFeatRight->GetValue()))
                {
                    if (m_ExtendCompleteRight->GetValue())
                        partial_right = false;
                    AddOneFeatureBetween(locs,feat_left,feat_right,partial_left,partial_right,loc1.GetStrand(),loc1.GetId());
                }
                found = true;
            }
            if (!found && m_NoFeatPartialLeft->GetValue())
            {
                TSignedSeqPos feat_left = 0;
                TSignedSeqPos feat_right = length - 1;
                if (subtype2 != objects::CSeqFeatData::eSubtype_bad)
                    for (objects::CFeat_CI feat2(*bsi, objects::SAnnotSelector(subtype2)); feat2 && feat_right == length-1; ++feat2)
                    {
                        if (constraint_right && !constraint_right->Match(*feat2))
                            continue;
                        const CSeq_loc &loc2 = feat2->GetLocation();
                        TSignedSeqPos start2 = feat2->GetLocationTotalRange().GetFrom();
                        TSignedSeqPos stop2 = feat2->GetLocationTotalRange().GetTo();
                        if (start2 > feat_left)
                        {
                            partial_right = false;
                            feat_right = start2 - 1;
                            found = true;
                            AddOneFeatureBetween(locs,feat_left,feat_right,partial_left,partial_right,loc2.GetStrand(),loc2.GetId());
                        }
                    }
                
                if (!found && !m_NoFeatNoFeatRight->GetValue())
                {
                    if (m_ExtendCompleteRight->GetValue()) 
                        partial_right = false;
        
                    if (feat_right >= feat_left) 
                        AddOneFeatureBetween (locs, feat_left, feat_right, partial_left, partial_right, objects::eNa_strand_plus, bsi->GetSeqId().GetPointer());
            
                }
            }
        }
        else if (subtype2 != objects::CSeqFeatData::eSubtype_bad) 
        {
            TSignedSeqPos feat_left = 0;
            if (m_ExtendCompleteLeft->GetValue()) 
                partial_left = false;
            for (objects::CFeat_CI feat2(*bsi, objects::SAnnotSelector(subtype2)); feat2 && !found; ++feat2)
            {
                if (constraint_right && !constraint_right->Match(*feat2))
                    continue;
                partial_right = false;
                const CSeq_loc &loc2 = feat2->GetLocation();
                TSignedSeqPos start2 = feat2->GetLocationTotalRange().GetFrom();
                TSignedSeqPos stop2 = feat2->GetLocationTotalRange().GetTo();
                TSignedSeqPos feat_right = start2 - 1;
                if (feat_right >= feat_left) 
                    AddOneFeatureBetween(locs,feat_left,feat_right,partial_left,partial_right,loc2.GetStrand(),loc2.GetId());
                found = true;
            }
            
            if (!found  && !m_NoFeatNoFeatRight->GetValue())
                AddOneFeatureBetween (locs, feat_left, length-1, partial_left, partial_right, objects::eNa_strand_plus, bsi->GetSeqId().GetPointer());
        } 
        else 
        {
            TSignedSeqPos feat_left = 0;
            TSignedSeqPos feat_right = length - 1;
            if (m_ExtendCompleteLeft->GetValue()) 
                partial_left = false;
            if (m_ExtendCompleteRight->GetValue()) 
                partial_right = false;
            if (feat_right >= feat_left) 
                AddOneFeatureBetween (locs, feat_left, feat_right, partial_left, partial_right, objects::eNa_strand_plus, bsi->GetSeqId().GetPointer());
        }       
    }
}

void CAddFeatureBetweenDlg::AddOneFeatureBetween(vector<CRef<objects::CSeq_loc> > &locs, TSignedSeqPos feat_left, TSignedSeqPos feat_right, bool partial_left, bool partial_right, objects::ENa_strand strand, const CSeq_id* id)
{
    if (feat_left < 0 || feat_right < 0)
        return;
    CRef<objects::CSeq_loc> loc(new CSeq_loc);
    loc->SetInt().SetId().Assign(*id);
    loc->SetInt().SetFrom(feat_left); 
    loc->SetInt().SetTo(feat_right);
    loc->SetStrand(strand);
    loc->SetPartialStart(partial_left,eExtreme_Positional); // loc1.IsPartialStop(eExtreme_Positional)
    loc->SetPartialStop(partial_right,eExtreme_Positional); // loc2.IsPartialStart(eExtreme_Positional)
    locs.push_back(loc);
}

/*!
 * Should we show tooltips?
 */

bool CAddFeatureBetweenDlg::ShowToolTips()
{
    return true;
}

/*!
 * Get bitmap resources
 */

wxBitmap CAddFeatureBetweenDlg::GetBitmapResource( const wxString& name )
{
    // Bitmap retrieval
////@begin CAddFeatureBetweenDlg bitmap retrieval
    wxUnusedVar(name);
    return wxNullBitmap;
////@end CAddFeatureBetweenDlg bitmap retrieval
}

/*!
 * Get icon resources
 */

wxIcon CAddFeatureBetweenDlg::GetIconResource( const wxString& name )
{
    // Icon retrieval
////@begin CAddFeatureBetweenDlg icon retrieval
    wxUnusedVar(name);
    return wxNullIcon;
////@end CAddFeatureBetweenDlg icon retrieval
}

CRef<CCmdComposite> CAddFeatureBetweenDlg::GetCommand()
{
    CRef<CCmdComposite> cmd(new CCmdComposite("Add Feature Between"));
    vector<CRef<objects::CSeq_loc> > locs;
    GetFeatureLocations(locs);
    for (unsigned int i=0; i<locs.size(); i++)
    {
        if (locs[i] && locs[i]->IsInt() && locs[i]->GetInt().IsSetId())
        {
            CBioseq_Handle bsh = m_TopSeqEntry.GetScope().GetBioseqHandle(locs[i]->GetInt().GetId());
            if (m_Notebook)
            {
                int sel = m_Notebook->GetSelection();
                if (sel != wxNOT_FOUND)
                {
                    wxWindow *win = m_Notebook->GetPage(sel);
                    CBulkCmdPanel * panel = dynamic_cast<CBulkCmdPanel*>(win);
                    if (panel)
                    {
                        panel->SetLoc(locs[i]);
                        panel->AddOneCommand(bsh, cmd);
                    }
                }
            }
        }
    }
    return cmd;
}


string CAddFeatureBetweenDlg::GetErrorMessage ()
{
    if (m_Notebook)
    {
        int sel = m_Notebook->GetSelection();
        if (sel != wxNOT_FOUND)
        {
            wxWindow *win = m_Notebook->GetPage(sel);
            CBulkCmdPanel * panel = dynamic_cast<CBulkCmdPanel*>(win);
            if (panel)
            {
                return panel->GetErrorMessage();
            }
        }
    }
    return kEmptyStr;
}

void CAddFeatureBetweenDlg::OnApplyButton(wxCommandEvent& event )
{

    m_TopSeqEntry.Reset();
    if (!m_Workbench) return;
    
    TConstScopedObjects objects;
    CIRef<CSelectionService> sel_srv = m_Workbench->GetServiceByType<CSelectionService>();
    if (sel_srv)
    {
        sel_srv->GetActiveObjects(objects);
    }
    if (objects.empty())
    {
        GetViewObjects(m_Workbench,objects);
    }

    NON_CONST_ITERATE (TConstScopedObjects, it, objects) {
        CSeq_entry_Handle seh = GetTopSeqEntryFromScopedObject(*it);
        if (seh) {
            m_TopSeqEntry = seh;
        }
    }

    if (objects.size() == 2)
    {
        const CObject* ptr = objects.front().object.GetPointer();
        if (ptr)
        {
            const objects::CSeq_loc* loc = dynamic_cast<const objects::CSeq_loc*>(ptr);
            if (loc)
            {
                CBioseq_Handle bsh = objects.front().scope->GetBioseqHandle(*loc);
                if (bsh) 
                {
                    CSeq_entry_Handle seh = bsh.GetSeq_entry_Handle();
                    if (seh)
                        m_TopSeqEntry = seh;
                }
            }
        }
    }

    if (!m_TopSeqEntry)
        return;
    ICommandProccessor* cmdProcessor = NULL;

    CIRef<CProjectService> srv = m_Workbench->GetServiceByType<CProjectService>();
    if (srv)
    {
        CRef<CGBWorkspace> ws = srv->GetGBWorkspace();
        if (!ws) return;

        CGBDocument* doc = dynamic_cast<CGBDocument*>(ws->GetProjectFromScope(m_TopSeqEntry.GetScope()));
        if (doc)
            cmdProcessor = &doc->GetUndoManager(); 
    }
    if (!cmdProcessor)
        return;

    CRef<CCmdComposite> cmd = GetCommand();
    if (cmd)
        cmdProcessor->Execute(cmd);         
    if (!m_LeaveUp->GetValue())
    {
        Close();
    }
}

void CAddFeatureBetweenDlg::OnFeatLeftChanged(wxCommandEvent& event)
{    
    string str = m_FeatureTypeLeft->GetStringSelection().ToStdString();
    if (str.empty() || !m_FeatLeftConstraint || !m_StringConstraintPanelLeft)
        return;
    m_FeatLeftConstraint->SetFieldName(str);
    bool allow_other = true;
    vector<string> choices = m_FeatLeftConstraint->GetChoices(allow_other); 
    m_StringConstraintPanelLeft->SetChoices(choices);      
}

void CAddFeatureBetweenDlg::OnFeatRightChanged(wxCommandEvent& event)
{    
    string str = m_FeatureTypeRight->GetStringSelection().ToStdString();
    if (str.empty() || !m_FeatRightConstraint || !m_StringConstraintPanelRight)
        return;
    m_FeatRightConstraint->SetFieldName(str);
    bool allow_other = true;
    vector<string> choices = m_FeatRightConstraint->GetChoices(allow_other); 
    m_StringConstraintPanelRight->SetChoices(choices);      
}

END_NCBI_SCOPE
