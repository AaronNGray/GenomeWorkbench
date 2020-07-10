/*  $Id: adjust_features_for_gaps.cpp 43609 2019-08-08 16:12:53Z filippov $
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


// For compilers that support precompilation, includes "wx/wx.h".
#include "wx/wxprec.h"

#ifdef __BORLANDC__
#pragma hdrstop
#endif

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

////@begin includes
#include <wx/textfile.h>
#include <wx/hyperlink.h>
////@end includes

#include <objects/seqfeat/RNA_ref.hpp>
#include <objects/seqfeat/Org_ref.hpp>
#include <objects/seqfeat/Cdregion.hpp>
#include <objects/seqloc/Seq_loc.hpp>
#include <objects/seqfeat/Feat_id.hpp>
#include <objmgr/util/seq_loc_util.hpp>
#include <objmgr/seq_map.hpp>
#include <objmgr/seq_map_ci.hpp>
#include <objmgr/feat_ci.hpp>
#include <objmgr/seq_feat_handle.hpp>
#include <objmgr/util/sequence.hpp>
#include <objtools/cleanup/fix_feature_id.hpp>
#include <gui/objutils/utils.hpp>
#include <gui/widgets/wx/file_extensions.hpp>
#include <gui/objutils/cmd_change_seq_feat.hpp>
#include <gui/objutils/cmd_del_seq_feat.hpp>
#include <gui/objutils/cmd_add_cds.hpp>
#include <gui/objutils/cmd_create_feat.hpp>
#include <gui/objutils/cmd_del_bioseq.hpp>
#include <gui/objutils/cmd_change_bioseq_inst.hpp>
#include <gui/objutils/cmd_composite.hpp>
#include <gui/objutils/convert_raw_to_delta.hpp>
#include <objtools/edit/cds_fix.hpp>
#include <objtools/edit/loc_edit.hpp>
#include <objtools/edit/text_object_description.hpp>
#include <objmgr/util/seq_loc_util.hpp>
#include <objmgr/util/feature.hpp>

#include <gui/packages/pkg_sequence_edit/split_cds_with_too_many_n.hpp>
#include <gui/packages/pkg_sequence_edit/adjust_features_for_gaps.hpp>
#include <gui/widgets/wx/wx_utils.hpp>

////@begin XPM images
////@end XPM images


BEGIN_NCBI_SCOPE
USING_SCOPE(objects);

/*
 * CAdjustFeaturesForGaps type definition
 */

IMPLEMENT_DYNAMIC_CLASS( CAdjustFeaturesForGaps, wxDialog )


/*
 * CAdjustFeaturesForGaps event table definition
 */

BEGIN_EVENT_TABLE( CAdjustFeaturesForGaps, wxDialog )

////@begin CAdjustFeaturesForGaps event table entries
    EVT_CHECKBOX(ID_CADJUSTFEATURES_CHECKBOX, CAdjustFeaturesForGaps::OnKnownUnknownSelected) // known
    EVT_CHECKBOX(ID_CADJUSTFEATURES_CHECKBOX1, CAdjustFeaturesForGaps::OnKnownUnknownSelected) // unknown
    EVT_CHECKBOX(ID_CADJUSTFEATURES_CHECKBOX_NS, CAdjustFeaturesForGaps::OnKnownUnknownSelected) // ns
    EVT_CHECKBOX(ID_CADJUSTFEATURES_CHECKBOX2, CAdjustFeaturesForGaps::OnTrimSelected) // trim
    EVT_CHECKBOX(ID_CADJUSTFEATURES_CHECKBOX5, CAdjustFeaturesForGaps::OnRemoveSelected) // remove
    EVT_CHECKBOX(ID_CADJUSTFEATURES_CHECKBOX3, CAdjustFeaturesForGaps::OnSplitSelected) // split
    EVT_LIST_ITEM_SELECTED(ID_CADJUSTFEATURES_LISTBOX, CAdjustFeaturesForGaps::OnFeatTypeChanged)
    EVT_LIST_ITEM_DESELECTED(ID_CADJUSTFEATURES_LISTBOX, CAdjustFeaturesForGaps::OnFeatTypeChanged)
    EVT_BUTTON(ID_CADJUSTFEATURES_BUTTON2, CAdjustFeaturesForGaps::OnMakeReport)
////@end CAdjustFeaturesForGaps event table entries

END_EVENT_TABLE()


/*
 * CAdjustFeaturesForGaps constructors
 */

CAdjustFeaturesForGaps::CAdjustFeaturesForGaps()
{
    Init();
}

CAdjustFeaturesForGaps::CAdjustFeaturesForGaps( wxWindow* parent, objects::CSeq_entry_Handle tse, wxWindowID id, const wxString& caption, const wxPoint& pos, const wxSize& size, long style )
   : m_TopSeqEntry(tse)
{
    Init();
    Create(parent, id, caption, pos, size, style);
}


/*
 * CAdjustFeaturesForGaps creator
 */

bool CAdjustFeaturesForGaps::Create( wxWindow* parent, wxWindowID id, const wxString& caption, const wxPoint& pos, const wxSize& size, long style )
{
////@begin CAdjustFeaturesForGaps creation
    SetExtraStyle(wxWS_EX_BLOCK_EVENTS);
    wxDialog::Create( parent, id, caption, pos, size, style );

    CreateControls();
    if (GetSizer())
    {
        GetSizer()->SetSizeHints(this);
    }
    Centre();
////@end CAdjustFeaturesForGaps creation
    return true;
}


/*
 * CAdjustFeaturesForGaps destructor
 */

CAdjustFeaturesForGaps::~CAdjustFeaturesForGaps()
{
////@begin CAdjustFeaturesForGaps destruction
////@end CAdjustFeaturesForGaps destruction
}


/*
 * Member initialisation
 */

void CAdjustFeaturesForGaps::Init()
{
////@begin CAdjustFeaturesForGaps member initialisation
    m_Features = NULL;
    m_FeatureType = NULL;
    m_UnknownGap = NULL;
    m_KnownGap = NULL;
    m_MakeTruncatedPartial = NULL;
    m_TrimEnds = NULL;
    m_RemoveFeats = NULL;
    m_SplitForInternal = NULL;
    m_EvenIfIntrons = NULL;
////@end CAdjustFeaturesForGaps member initialisation
    PopulateFeatureList();
    GetGappedFeatures();
}


/*
 * Control creation for CAdjustFeaturesForGaps
 */

void CAdjustFeaturesForGaps::CreateControls()
{    
////@begin CAdjustFeaturesForGaps content construction
    // Generated by DialogBlocks, 25/02/2016 16:13:18 (unregistered)

    CAdjustFeaturesForGaps* itemDialog1 = this;

    wxBoxSizer* itemBoxSizer2 = new wxBoxSizer(wxVERTICAL);
    itemDialog1->SetSizer(itemBoxSizer2);

    wxBoxSizer* itemBoxSizer3 = new wxBoxSizer(wxHORIZONTAL);
    itemBoxSizer2->Add(itemBoxSizer3, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

    m_Features = new wxListCtrl( itemDialog1, ID_CADJUSTFEATURES_LISTCTRL, wxDefaultPosition, wxDefaultSize, wxLC_REPORT );
    itemBoxSizer3->Add(m_Features, 1, wxGROW|wxALL, 5);

    wxListItem col0;
    col0.SetId(0);
    col0.SetText( _("Feature") );
    col0.SetWidth(80);
    m_Features->InsertColumn(0, col0);
 
    wxListItem col1;
    col1.SetId(1);
    col1.SetText( wxEmptyString );
    col1.SetWidth(250);
    m_Features->InsertColumn(1, col1);

    wxListItem col2;
    col2.SetId(2);
    col2.SetText(_("Action"));
    col2.SetWidth(60);
    m_Features->InsertColumn(2, col2);
 
    wxListItem col3;
    col3.SetId(3);
    col3.SetText( _("Location") );
    col3.SetWidth(250);
    m_Features->InsertColumn(3, col3);   

    m_FeatureCountText = new wxStaticText( itemDialog1, wxID_STATIC, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer2->Add(m_FeatureCountText, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

    m_FeatureType = new wxListCtrl(itemDialog1, ID_CADJUSTFEATURES_LISTBOX, wxDefaultPosition, wxSize(250, 200), wxLC_REPORT);
    itemBoxSizer2->Add(m_FeatureType, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

    wxBoxSizer* itemBoxSizer7 = new wxBoxSizer(wxHORIZONTAL);
    itemBoxSizer2->Add(itemBoxSizer7, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

    m_UnknownGap = new wxCheckBox( itemDialog1, ID_CADJUSTFEATURES_CHECKBOX, _("Unknown length gaps"), wxDefaultPosition, wxDefaultSize, 0 );
    m_UnknownGap->SetValue(false);
    itemBoxSizer7->Add(m_UnknownGap, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    m_KnownGap = new wxCheckBox( itemDialog1, ID_CADJUSTFEATURES_CHECKBOX1, _("Known length gaps"), wxDefaultPosition, wxDefaultSize, 0 );
    m_KnownGap->SetValue(false);
    itemBoxSizer7->Add(m_KnownGap, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    m_Ns = new wxCheckBox(itemDialog1, ID_CADJUSTFEATURES_CHECKBOX_NS, _("Ns"), wxDefaultPosition, wxDefaultSize, 0);
    m_Ns->SetValue(false);
    itemBoxSizer7->Add(m_Ns, 0, wxALIGN_CENTER_VERTICAL | wxALL, 5);

    wxArrayString m_MakeTruncatedPartialStrings;
    m_MakeTruncatedPartialStrings.Add(_("Always"));
    m_MakeTruncatedPartialStrings.Add(_("Unless pseudo"));
    m_MakeTruncatedPartialStrings.Add(_("Never"));
    m_MakeTruncatedPartial = new wxRadioBox( itemDialog1, ID_CADJUSTFEATURES_RADIOBOX, _("Make truncated ends partial"), wxDefaultPosition, wxDefaultSize, m_MakeTruncatedPartialStrings, 1, wxRA_SPECIFY_ROWS );
    m_MakeTruncatedPartial->SetSelection(0);
    itemBoxSizer2->Add(m_MakeTruncatedPartial, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

    wxBoxSizer* itemBoxSizer11 = new wxBoxSizer(wxHORIZONTAL);
    itemBoxSizer2->Add(itemBoxSizer11, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

    m_TrimEnds = new wxCheckBox( itemDialog1, ID_CADJUSTFEATURES_CHECKBOX2, _("Trim ends in gaps"), wxDefaultPosition, wxDefaultSize, 0 );
    m_TrimEnds->SetValue(false);
    itemBoxSizer11->Add(m_TrimEnds, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    m_RemoveFeats = new wxCheckBox(itemDialog1, ID_CADJUSTFEATURES_CHECKBOX5, _("Remove features entirely in gaps"), wxDefaultPosition, wxDefaultSize, 0);
    m_RemoveFeats->SetValue(false);
    itemBoxSizer11->Add(m_RemoveFeats, 0, wxALIGN_CENTER_VERTICAL | wxALL, 5);

    m_SplitForInternal = new wxCheckBox( itemDialog1, ID_CADJUSTFEATURES_CHECKBOX3, _("Split for internal gaps"), wxDefaultPosition, wxDefaultSize, 0 );
    m_SplitForInternal->SetValue(false);
    itemBoxSizer11->Add(m_SplitForInternal, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    m_EvenIfIntrons = new wxCheckBox( itemDialog1, ID_CADJUSTFEATURES_CHECKBOX4, _("[Even when gaps are in introns]"), wxDefaultPosition, wxDefaultSize, 0 );
    m_EvenIfIntrons->SetValue(false);
    m_EvenIfIntrons->Enable(false);
    itemBoxSizer11->Add(m_EvenIfIntrons, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxBoxSizer* itemBoxSizer15 = new wxBoxSizer(wxHORIZONTAL);
    itemBoxSizer2->Add(itemBoxSizer15, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

    m_AcceptButton = new wxButton( itemDialog1, wxID_OK, _("Accept"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer15->Add(m_AcceptButton, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);
    m_AcceptButton->Disable();

    wxButton* itemButton17 = new wxButton( itemDialog1, wxID_CANCEL, _("Close"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer15->Add(itemButton17, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxButton* itemButton18 = new wxButton( itemDialog1, ID_CADJUSTFEATURES_BUTTON2, _("Make Report"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer15->Add(itemButton18, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxHyperlinkCtrl* itemHyperlinkCtrl = new wxHyperlinkCtrl( itemDialog1, wxID_HELP, _("Help"), wxT("https://www.ncbi.nlm.nih.gov/tools/gbench/manual15/#adjust-features"), wxDefaultPosition, wxDefaultSize, wxHL_DEFAULT_STYLE );
    itemHyperlinkCtrl->SetForegroundColour(wxColour(192, 192, 192));
    itemBoxSizer15->Add(itemHyperlinkCtrl, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    m_LeaveUp = new wxCheckBox(itemDialog1, wxID_ANY, _("Leave dialog up"));
    itemBoxSizer15->Add(m_LeaveUp, 0, wxALIGN_LEFT | wxALL, 5);

////@end CAdjustFeaturesForGaps content construction
    DisplayGappedFeatures();

    m_FeatureType->InsertColumn(0, "    Feature", wxLIST_FORMAT_LEFT, 237);
    for (unsigned int i = 0; i < m_FeatureTypeStrings.size(); i++) {
        m_FeatureType->InsertItem(i, m_FeatureTypeStrings[i]);
    }
}

void CAdjustFeaturesForGaps::PopulateFeatureList()
{
    m_FeatureTypeStrings.Clear();
    if (!m_TopSeqEntry)
        return;
    set<wxString> existing;
    vector<const objects::CFeatListItem *> feat_list = GetSortedFeatList(m_TopSeqEntry);
    ITERATE(vector<const objects::CFeatListItem *>, ft_it, feat_list) 
    {
        const objects::CFeatListItem& item = **ft_it;
        wxString desc = wxString(item.GetDescription());
        int feat_type = item.GetType();
        int feat_subtype = item.GetSubtype();
        if (existing.find(desc) == existing.end())
        {
            existing.insert(desc);
            m_FeatureTypeStrings.Add(desc);
            m_feat_types.push_back(feat_type);
            m_feat_subtypes.push_back(feat_subtype);
        }
    }
}

void CAdjustFeaturesForGaps::GetGappedFeatures()
{
    if (!m_TopSeqEntry)
        return;

    CFeat_CI f(m_TopSeqEntry);
    m_FeatTree = new feature::CFeatTree(f);

    m_GappedFeatureList = edit::ListGappedFeatures(f, m_TopSeqEntry.GetScope());
    RecalculateGapIntervals();
    m_TopFeatureId = CFixFeatureId::s_FindHighestFeatureId(m_TopSeqEntry);
}


bool CAdjustFeaturesForGaps::x_IsFeatureTypeSelected(CSeqFeatData::ESubtype subtype)
{
    long item = -1;
    for (;;)
    {
        item = m_FeatureType->GetNextItem(item, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);
        if (item == -1)  break;
        string name = ToStdString(m_FeatureType->GetItemText(item));
        if (NStr::EqualNocase(name, "All")) {
            return true;
        } else {
            CSeqFeatData::ESubtype match = CSeqFeatData::SubtypeNameToValue(name);
            if (match == subtype) {
                return true;
            }
        }
    }
    return false;
}

string CAdjustFeaturesForGaps::x_Action(edit::CFeatGapInfo& fgap)
{
    if ((fgap.HasKnown() && m_KnownGap->GetValue()) ||
        (fgap.HasUnknown() && m_UnknownGap->GetValue()) ||
        (fgap.HasNs() && m_Ns->GetValue())) {
        if (fgap.ShouldRemove() && m_RemoveFeats->GetValue()) {
            return "remove";
        } else {
            string rval = kEmptyStr;
            if (fgap.Trimmable() && m_TrimEnds->GetValue()) {
                rval = "trim";
            }
            if (fgap.Splittable() && m_SplitForInternal->GetValue()) {
                if (!rval.empty()) {
                    rval += ";";
                }
                rval += "split";
            }
            return rval;
        }
    } else {
        return kEmptyStr;
    }
}


bool CAdjustFeaturesForGaps::x_Adjustable(edit::CFeatGapInfo& fgap)
{
    if (((fgap.HasKnown() && m_KnownGap->GetValue()) ||
        (fgap.HasUnknown() && m_UnknownGap->GetValue()) ||
        (fgap.HasNs() && m_Ns->GetValue())) &&
        ((fgap.Trimmable() && m_TrimEnds->GetValue()) ||
        (fgap.Splittable() && m_SplitForInternal->GetValue())))
    {

        if (x_IsFeatureTypeSelected(fgap.GetFeature().GetData().GetSubtype())) {
            return true;
        } else {
            return false;
        }
    } else if (fgap.ShouldRemove() && m_RemoveFeats->GetValue()) {
        if (x_IsFeatureTypeSelected(fgap.GetFeature().GetData().GetSubtype())) {
            return true;
        } else {
            return false;
        }
    } else {
        return false;
    }
}


void CAdjustFeaturesForGaps::DisplayGappedFeatures()
{
    if (!m_Features || ! m_KnownGap || !m_UnknownGap || !m_Ns || !m_FeatureType || !m_TopSeqEntry || !m_FeatureCountText)
        return;
    m_FeatureCountText->SetLabelText(wxEmptyString);
    m_Features->DeleteAllItems();
    size_t internal_gap = 0;
    for (size_t i = 0; i < m_GappedFeatureList.size(); i++)
    {
        if (m_GappedFeatureList[i]->Splittable()) {
            internal_gap++;
        }
        if (x_Adjustable(*(m_GappedFeatureList[i]))) {
            objects::CSeq_feat_Handle fh = m_GappedFeatureList[i]->GetFeature();

            string name, desc, location;
            GetTextForFeature(fh, name, desc, location);
            string action = x_Action(*(m_GappedFeatureList[i]));

            long itemIndex = m_Features->InsertItem(0, wxString(name));
            m_Features->SetItem(itemIndex, 1, wxString(desc));
            m_Features->SetItem(itemIndex, 2, wxString(action));
            m_Features->SetItem(itemIndex, 3, wxString(location));
        }
    }
    size_t num_items = m_Features->GetItemCount();
    if (num_items > 0 || internal_gap > 0)
    {
        wxString feat_count;
        if (num_items > 0) {
            feat_count << num_items;
            if (num_items > 1)
                feat_count << " features will be adjusted";
            else
                feat_count << " feature will be adjusted";
        }
        m_FeatureCountText->SetLabelText(feat_count);
        Layout();
    }
}

void CAdjustFeaturesForGaps::GetTextForFeature(objects::CSeq_feat_Handle fh, string &name, string &desc, string &location)
{
    string locus_tag;
    edit::GetTextObjectDescription(*fh.GetOriginalSeq_feat(), fh.GetScope(), name, desc, location, locus_tag);
}

/*
 * Should we show tooltips?
 */

bool CAdjustFeaturesForGaps::ShowToolTips()
{
    return true;
}

/*
 * Get bitmap resources
 */

wxBitmap CAdjustFeaturesForGaps::GetBitmapResource( const wxString& name )
{
    // Bitmap retrieval
////@begin CAdjustFeaturesForGaps bitmap retrieval
    wxUnusedVar(name);
    return wxNullBitmap;
////@end CAdjustFeaturesForGaps bitmap retrieval
}

/*
 * Get icon resources
 */

wxIcon CAdjustFeaturesForGaps::GetIconResource( const wxString& name )
{
    // Icon retrieval
////@begin CAdjustFeaturesForGaps icon retrieval
    wxUnusedVar(name);
    return wxNullIcon;
////@end CAdjustFeaturesForGaps icon retrieval
}

void CAdjustFeaturesForGaps::OnFeatTypeChanged(wxListEvent& event)
{
    DisplayGappedFeatures();
    EnableDisableAcceptButton();
}


void CAdjustFeaturesForGaps::RecalculateGapIntervals()
{
    if (!m_KnownGap || !m_UnknownGap || !m_Ns) {
        return;
    }
    bool show_known = m_KnownGap->GetValue();
    bool show_unknown = m_UnknownGap->GetValue();
    bool show_ns = m_Ns->GetValue();

    NON_CONST_ITERATE(edit::TGappedFeatList, it, m_GappedFeatureList) {
        (*it)->CalculateRelevantIntervals(show_unknown, show_known, show_ns);
    }
}


void CAdjustFeaturesForGaps::OnKnownUnknownSelected(wxCommandEvent& event)
{
    RecalculateGapIntervals();
    DisplayGappedFeatures();
    EnableDisableAcceptButton();
}

void CAdjustFeaturesForGaps::OnTrimSelected(wxCommandEvent& event)
{  
    DisplayGappedFeatures();
    EnableDisableAcceptButton();
}


void CAdjustFeaturesForGaps::OnRemoveSelected(wxCommandEvent& event)
{
    DisplayGappedFeatures();
    EnableDisableAcceptButton();
}


void CAdjustFeaturesForGaps::OnSplitSelected(wxCommandEvent& event)
{
    if (m_SplitForInternal->GetValue())
    {
        m_EvenIfIntrons->Enable();
    }
    else
    {
        m_EvenIfIntrons->Disable();
    }
    DisplayGappedFeatures();
    EnableDisableAcceptButton();
}

void CAdjustFeaturesForGaps::EnableDisableAcceptButton()
{
    if ((m_SplitForInternal->GetValue() || m_TrimEnds->GetValue() || m_RemoveFeats->GetValue()) &&
        m_Features->GetItemCount() > 0)
    {
        m_AcceptButton->Enable();
    }
    else
    {
        m_AcceptButton->Disable();
    }
}

void CAdjustFeaturesForGaps::OnMakeReport( wxCommandEvent& event) 
{
    size_t num_adjustable = 0, num_internal = 0;
    for (size_t i = 0; i < m_GappedFeatureList.size(); i++)
    {
        if (x_Adjustable(*(m_GappedFeatureList[i]))) {
            num_adjustable++;
        }
        if (m_GappedFeatureList[i]->Splittable()) {
            num_internal++;
        }
    }
    if (num_adjustable == 0 && num_internal == 0) {
        wxMessageBox(_("No features found"), wxT("Error"), wxOK|wxICON_ERROR);
        return;
    }

    wxFileDialog save_file(this, wxT("Make report"), wxEmptyString, wxEmptyString,
                           CFileExtensions::GetDialogFilter(CFileExtensions::kAllFiles),
                           wxFD_SAVE|wxFD_OVERWRITE_PROMPT);
    
    if (save_file.ShowModal() == wxID_OK)
    {
        wxString path = save_file.GetPath();
        if( !path.IsEmpty())
        {
            string lb(wxString(wxTextFile::GetEOL()).ToStdString());
            ios::openmode mode = ios::out;
            CNcbiOfstream os(path.fn_str(), mode);
            if (num_adjustable > 0)
            {
                os << num_adjustable;
                if (num_adjustable > 1)
                    os << "  features will be adjusted" << lb;
                else
                    os <<" feature will be adjusted" << lb;

                for (size_t i = 0; i < m_GappedFeatureList.size(); i++)
                {
                    if (x_Adjustable(*(m_GappedFeatureList[i]))) {
                        string name, desc, location;
                        GetTextForFeature(m_GappedFeatureList[i]->GetFeature(), name, desc, location);
                        os << name << "\t" << desc << "\t" << location << lb;
                    }
                }
                if (num_internal > 0)
                    os << lb;
            }
            if (num_internal > 0)
            {
                os << num_internal;
                if (num_internal > 1)
                    os << " features contain internal gaps" << lb;
                else
                    os << " feature contains internal gaps" << lb;
                for (size_t i = 0; i < m_GappedFeatureList.size(); i++)
                {
                    if (m_GappedFeatureList[i]->Splittable()) {
                        string name, desc, location;
                        GetTextForFeature(m_GappedFeatureList[i]->GetFeature(), name, desc, location);
                        os << name << "\t" << desc << "\t" << location << lb;
                    }
                }
            }
        }
    }
}


CAdjustFeaturesForGaps::TFeatUpdatePairVector CAdjustFeaturesForGaps::x_PullRelatedGroup(edit::TGappedFeatList &to_edit, bool always, bool unless_pseudo, bool do_trim, bool do_split, bool split_intron, bool create_general_only)
{
    CRef<edit::CFeatGapInfo> top = to_edit[0];
    TFeatUpdatePairVector grp;
    bool make_partial = (always || (unless_pseudo && !(top->GetFeature().IsSetPseudo() && top->GetFeature().GetPseudo())));
    grp.push_back(TFeatUpdatePair(top, top->AdjustForRelevantGapIntervals(make_partial, do_trim, do_split, split_intron, create_general_only)));
    to_edit.erase(to_edit.begin());
    size_t check_index = 0;
    while (check_index < grp.size()) {
        CRef<edit::CFeatGapInfo> top = grp[check_index].first;
        edit::TGappedFeatList::iterator cand = to_edit.begin();
        while (cand != to_edit.end()) {
            if (top->IsRelatedByCrossRef(**cand)) {
                bool make_partial = (always || (unless_pseudo && !((*cand)->GetFeature().IsSetPseudo() && (*cand)->GetFeature().GetPseudo())));
                grp.push_back(TFeatUpdatePair(*cand, (*cand)->AdjustForRelevantGapIntervals(make_partial, do_trim, do_split, split_intron, create_general_only)));
                cand = to_edit.erase(cand);
            } else {
                ++cand;
            }
        }
        check_index++;
    }

    return grp;
}


void CAdjustFeaturesForGaps::x_DoGroup(TFeatUpdatePairVector grp, CObject_id::TId& next_id, CCmdComposite& cmd, 
                                       map<CObject_id::TId, CObject_id::TId> &old_to_new, bool create_xref_map)
{
    NON_CONST_ITERATE(TFeatUpdatePairVector, it, grp) {
        if (it->second.size() > 1) {
            edit::FixFeatureIdsForUpdates(it->second, next_id);
        }
    }
    NON_CONST_ITERATE(TFeatUpdatePairVector, it1, grp) {
        TFeatUpdatePairVector::iterator it2 = it1;
        ++it2;
        while (it2 != grp.end()) {
            edit::FixFeatureIdsForUpdatePair(it1->second, it2->second);
            ++it2;
        }
        x_DoOne(*it1, next_id, cmd, old_to_new, create_xref_map);
    }

}

void CAdjustFeaturesForGaps::x_DoOne(TFeatUpdatePair& p, CObject_id::TId& next_id, CCmdComposite& cmd, map<CObject_id::TId, CObject_id::TId> &old_to_new, bool create_xref_map)
{
    CSeq_feat_Handle fh = p.first->GetFeature();
    CScope& scope = fh.GetScope();
    vector<CRef<CSeq_feat> > updates = p.second;
    if (updates.empty() || (p.first->ShouldRemove() && m_RemoveFeats->GetValue())) {
        CRef<CCmdDelSeq_feat> cmd_del(new CCmdDelSeq_feat(fh));
        if (cmd_del)
        {
            cmd.AddCommand(*cmd_del);
        }
    } else if (fh.GetData().IsCdregion()) {
        // safer just to add all new coding regions 
        const objects::CSeq_annot_Handle& feat_annot_handle = fh.GetAnnot();
        objects::CSeq_entry_Handle feat_seh = feat_annot_handle.GetParentEntry();
        if (!feat_seh.IsSet()) {
            CBioseq_set_Handle bssh = feat_seh.GetParentBioseq_set();
            if (bssh && bssh.IsSetClass() && bssh.GetClass() == CBioseq_set::eClass_nuc_prot) {
                feat_seh = bssh.GetParentEntry();
            }
        }

        CRef< CCmdChangeSeq_feat > change_feat(new CCmdChangeSeq_feat(fh, *(updates.front())));
        if (change_feat)
        {
            cmd.AddCommand(*change_feat);
        }
        if (fh.IsSetProduct())
            {
                const CSeq_id *id = fh.GetProduct().GetId();
                if (id)
                {
                    CBioseq_Handle prot_bsh = scope.GetBioseqHandle(*id);
                    if (prot_bsh)
                    {
                        string prot;
                        try
                        {
                            CSeqTranslator::Translate(*(updates.front()), scope, prot);
                        }
                        catch (const CSeqVectorException&) {}

                        if (!prot.empty())
                        {
                            if (NStr::EndsWith(prot, "*"))
                            {
                                prot = prot.substr(0, prot.length() - 1);
                            }
                            CRef<CBioseq> prot_seq(new CBioseq);
                            prot_seq->Assign(*(prot_bsh.GetCompleteBioseq()));
                            prot_seq->SetInst().ResetExt();
                            prot_seq->SetInst().SetRepr(objects::CSeq_inst::eRepr_raw);
                            prot_seq->SetInst().SetSeq_data().SetIupacaa().Set(prot);
                            prot_seq->SetInst().SetLength(TSeqPos(prot.length()));
                            prot_seq->SetInst().SetMol(CSeq_inst::eMol_aa);
                            CRef<CCmdChangeBioseqInst> chgInst(new CCmdChangeBioseqInst(prot_bsh, prot_seq->SetInst()));
                            cmd.AddCommand(*chgInst);
                            objects::CFeat_CI prot_feat_ci(prot_bsh, objects::SAnnotSelector(objects::CSeqFeatData::eSubtype_prot));
                            if (prot_feat_ci)
                            {
                                CRef<objects::CSeq_feat> prot_feat(new objects::CSeq_feat());
                                prot_feat->Assign(*(prot_feat_ci->GetSeq_feat()));
                                prot_feat->ResetLocation();
                                prot_feat->SetLocation().SetInt().SetId().Assign(*(prot_seq->GetId().front()));
                                prot_feat->SetLocation().SetInt().SetFrom(0);
                                prot_feat->SetLocation().SetInt().SetTo(prot_seq->GetLength() - 1);
                                edit::AdjustProteinFeaturePartialsToMatchCDS(*prot_feat, *(updates.front()));
                                CIRef<IEditCommand> chgFeat(new CCmdChangeSeq_feat(prot_feat_ci->GetSeq_feat_Handle(), *prot_feat));
                                cmd.AddCommand(*chgFeat);
                            }
                            bool modified;
                            CRef<CCmdComposite> composite(&cmd);
                            NRawToDeltaSeq::RemapOtherProtFeats(*fh.GetOriginalSeq_feat(), *(updates.front()), prot_bsh, composite, modified);
                        }
                        else
                        {
                            CRef<CCmdDelBioseqInst> delInst(new CCmdDelBioseqInst(prot_bsh));
                            cmd.AddCommand(*delInst);
                        }
                    }
                }
            }

        for (size_t offset = 1; offset < updates.size(); offset++) {
            CRef<CSeq_feat> new_feat = updates[offset];
            CRef<objects::CSeq_feat> prot_feat;
            objects::CBioseq_Handle product = scope.GetBioseqHandle(fh.GetProduct());
            vector<CRef<CSeq_feat> > other_prot_feats;
            vector<CRef<objects::CSeq_id> > new_prot_id;
            CRef<objects::CSeq_id> new_id(new objects::CSeq_id);
            new_id->Assign(new_feat->GetProduct().GetWhole());
            new_prot_id.push_back(new_id);

            if (product)
            {
                for (objects::CFeat_CI prot_feat_ci(product); prot_feat_ci; ++prot_feat_ci)
                {
                    if (prot_feat_ci->GetSeq_feat_Handle().GetFeatSubtype() == objects::CSeqFeatData::eSubtype_prot)
                    {
                        prot_feat.Reset(new objects::CSeq_feat());
                        prot_feat->Assign(*(prot_feat_ci->GetSeq_feat()));
                        if (offset > 0) 
                        {
                            edit::FixFeatureIdsForUpdates(*prot_feat, next_id);
                        }
                    }
                    else
                    {
                        --next_id;
                        NRawToDeltaSeq::RemapOtherProtFeats(*fh.GetOriginalSeq_feat(), *new_feat, prot_feat_ci->GetSeq_feat_Handle(), other_prot_feats, next_id, old_to_new, create_xref_map);
                        ++next_id;
                    }
                }
            }             
            cmd.AddCommand(*CRef<CCmdCreateCDS>(new CCmdCreateCDS(feat_seh, *new_feat, prot_feat, new_prot_id, other_prot_feats)));
        }              
    } else {
        CRef< CCmdChangeSeq_feat > change_feat(new CCmdChangeSeq_feat(fh, *(updates.front())));
        if (change_feat)
        {
            cmd.AddCommand(*change_feat);
        }
        const objects::CSeq_annot_Handle& feat_annot_handle = fh.GetAnnot();
        objects::CSeq_entry_Handle feat_seh = feat_annot_handle.GetParentEntry();
        for (size_t offset = 1; offset < updates.size(); offset++) {
            CRef<CSeq_feat> new_feat = updates[offset];
            CRef<CCmdCreateFeat> cmd_add_feat(new CCmdCreateFeat(feat_seh, *new_feat));
            cmd.AddCommand(*cmd_add_feat);
        }
    }
}


CRef<edit::CFeatGapInfo> s_ListHasFeature(CSeq_feat_Handle feat, const edit::TGappedFeatList &to_do)
{
    ITERATE(edit::TGappedFeatList, it, to_do) {
        if ((*it)->GetFeature() == feat) {
            return *it;
        }
    }
    return CRef<edit::CFeatGapInfo>(NULL);
}


// if trimming, need to adjust genes even if not selected
edit::TGappedFeatList CAdjustFeaturesForGaps::x_GetGenesForFeatures(const edit::TGappedFeatList& to_do)
{
    edit::TGappedFeatList genes_to_trim;
    ITERATE(edit::TGappedFeatList, it, to_do) {
        CSeq_feat_Handle fh = (*it)->GetFeature();
        if (!fh.GetData().IsGene()) {
            CSeq_feat_Handle gene = m_FeatTree->GetBestGene(fh);
            if (gene && !s_ListHasFeature(gene, to_do)) {
                CRef<edit::CFeatGapInfo> add = s_ListHasFeature(gene, m_GappedFeatureList);
                if (add) {
                    genes_to_trim.push_back(add);
                }
            }
        }
    }
    return genes_to_trim;
}


edit::TGappedFeatList CAdjustFeaturesForGaps::x_GetSelectedFeatures()
{
    edit::TGappedFeatList rval;

    edit::TGappedFeatList::reverse_iterator rit = m_GappedFeatureList.rbegin();
    while (rit != m_GappedFeatureList.rend() &&
           !x_Adjustable(**rit)) {
        rit++;
    }
    size_t shown_feat_offset = 0;
    long item = -1;
    for (;;)
    {
        item = m_Features->GetNextItem(item, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);
        if (item == -1)  break;
        while (shown_feat_offset < item) {
            rit++;
            while (rit != m_GappedFeatureList.rend() && !x_Adjustable(**rit)) {
                rit++;
            }
            if (rit == m_GappedFeatureList.rend()) {
                break;
            }
            shown_feat_offset++;
        }
        if (item == shown_feat_offset) {
            rval.push_back(*rit);
        }
    }
    return rval;
}

CRef<CCmdComposite> CAdjustFeaturesForGaps::GetCommand()
{
    map<CObject_id::TId, CObject_id::TId> old_to_new;
    bool create_xref_map = true;
    GetCommand_impl(old_to_new, create_xref_map);
    create_xref_map = false;
    return GetCommand_impl(old_to_new, create_xref_map);
}

CRef<CCmdComposite> CAdjustFeaturesForGaps::GetCommand_impl(map<CObject_id::TId, CObject_id::TId> &old_to_new, bool create_xref_map)
{
    CRef<CCmdComposite> cmd(new CCmdComposite("Adjust Features for Gaps"));
    objects::CScope &scope = m_TopSeqEntry.GetScope();
    bool create_general_only = objects::edit::IsGeneralIdProtPresent(m_TopSeqEntry);
    int partial_sel = m_MakeTruncatedPartial->GetSelection();
    if (partial_sel == wxNOT_FOUND)
        return cmd;
    string partial_sel_str =  m_MakeTruncatedPartial->GetString(partial_sel).ToStdString();
    bool always = false;
    bool unless_pseudo = false;
    bool never = false;
    if (partial_sel_str == "Always")
        always = true;
    else if (partial_sel_str == "Unless pseudo")
        unless_pseudo = true;
    else
        never = true;   

    bool do_trim = m_TrimEnds->GetValue();
    bool do_remove = m_RemoveFeats->GetValue();
    bool do_split = m_SplitForInternal->GetValue();
    bool split_intron = false;
    if (do_split) {
        split_intron = m_EvenIfIntrons->GetValue();
    }

    edit::TGappedFeatList to_do;
    if (m_Features->GetSelectedItemCount() == 0) {
        NON_CONST_ITERATE(edit::TGappedFeatList, it, m_GappedFeatureList) {
            if (x_Adjustable(**it)) {
                to_do.push_back(*it);
            }
        }
    } else {
        to_do = x_GetSelectedFeatures();
    }
    
    CObject_id::TId next_id = m_TopFeatureId + 1;

    if (do_trim) {
        edit::TGappedFeatList genes_to_trim = x_GetGenesForFeatures(to_do);
        NON_CONST_ITERATE(edit::TGappedFeatList, it, genes_to_trim) {
            TFeatUpdatePairVector grp;
            bool make_partial = (always || (unless_pseudo && !((*it)->GetFeature().IsSetPseudo() && (*it)->GetFeature().GetPseudo())));
            grp.push_back(TFeatUpdatePair(*it, (*it)->AdjustForRelevantGapIntervals(make_partial, do_trim, false, false, create_general_only)));
            x_DoGroup(grp, next_id, *cmd, old_to_new, create_xref_map);
        }
    }

    while (!to_do.empty()) {
        TFeatUpdatePairVector grp = x_PullRelatedGroup(to_do, always, unless_pseudo, do_trim, do_split, split_intron, create_general_only);
        x_DoGroup(grp, next_id, *cmd, old_to_new, create_xref_map);
    }

    return cmd;
}


void CAdjustFeaturesForGaps::Refresh()
{
    GetGappedFeatures();
    DisplayGappedFeatures();
    EnableDisableAcceptButton();
}


END_NCBI_SCOPE
