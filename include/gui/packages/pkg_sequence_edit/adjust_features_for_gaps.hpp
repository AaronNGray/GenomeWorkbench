/*  $Id: adjust_features_for_gaps.hpp 43609 2019-08-08 16:12:53Z filippov $
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
#ifndef _CADJUSTFEATURESFORGAPS_H_
#define _CADJUSTFEATURESFORGAPS_H_


/*!
 * Includes
 */
#include <corelib/ncbistd.hpp>
#include <objects/general/Object_id.hpp>
#include <objtools/edit/gap_trim.hpp>

////@begin includes
#include <wx/listctrl.h>
////@end includes

/*!
 * Forward declarations
 */

BEGIN_NCBI_SCOPE
////@begin forward declarations
////@end forward declarations

/*!
 * Control identifiers
 */

////@begin control identifiers
#define ID_CADJUSTFEATURESFORGAPS 10000
#define ID_CADJUSTFEATURES_LISTCTRL 10002
#define ID_CADJUSTFEATURES_LISTBOX 10004
#define ID_CADJUSTFEATURES_CHECKBOX 10005
#define ID_CADJUSTFEATURES_CHECKBOX1 10006
#define ID_CADJUSTFEATURES_RADIOBOX 10007
#define ID_CADJUSTFEATURES_CHECKBOX2 10008
#define ID_CADJUSTFEATURES_CHECKBOX3 10009
#define ID_CADJUSTFEATURES_CHECKBOX4 10010
#define ID_CADJUSTFEATURES_CHECKBOX_NS 10011
#define ID_CADJUSTFEATURES_BUTTON2 10013
#define ID_CADJUSTFEATURES_CHECKBOX5 10014
#define SYMBOL_CADJUSTFEATURESFORGAPS_STYLE wxCAPTION|wxRESIZE_BORDER|wxSYSTEM_MENU|wxCLOSE_BOX|wxTAB_TRAVERSAL
#define SYMBOL_CADJUSTFEATURESFORGAPS_TITLE _("Adjust Features For Gaps")
#define SYMBOL_CADJUSTFEATURESFORGAPS_IDNAME ID_CADJUSTFEATURESFORGAPS
#define SYMBOL_CADJUSTFEATURESFORGAPS_SIZE wxSize(400, 300)
#define SYMBOL_CADJUSTFEATURESFORGAPS_POSITION wxDefaultPosition
////@end control identifiers


/*!
 * CAdjustFeaturesForGaps class declaration
 */

class CAdjustFeaturesForGaps: public wxDialog
{    
    DECLARE_DYNAMIC_CLASS( CAdjustFeaturesForGaps )
    DECLARE_EVENT_TABLE()

public:
    /// Constructors
    CAdjustFeaturesForGaps();
    CAdjustFeaturesForGaps( wxWindow* parent, objects::CSeq_entry_Handle tse, wxWindowID id = SYMBOL_CADJUSTFEATURESFORGAPS_IDNAME, const wxString& caption = SYMBOL_CADJUSTFEATURESFORGAPS_TITLE, const wxPoint& pos = SYMBOL_CADJUSTFEATURESFORGAPS_POSITION, const wxSize& size = SYMBOL_CADJUSTFEATURESFORGAPS_SIZE, long style = SYMBOL_CADJUSTFEATURESFORGAPS_STYLE );

    /// Creation
    bool Create( wxWindow* parent, wxWindowID id = SYMBOL_CADJUSTFEATURESFORGAPS_IDNAME, const wxString& caption = SYMBOL_CADJUSTFEATURESFORGAPS_TITLE, const wxPoint& pos = SYMBOL_CADJUSTFEATURESFORGAPS_POSITION, const wxSize& size = SYMBOL_CADJUSTFEATURESFORGAPS_SIZE, long style = SYMBOL_CADJUSTFEATURESFORGAPS_STYLE );

    /// Destructor
    ~CAdjustFeaturesForGaps();

    /// Initialises member variables
    void Init();

    /// Creates the controls and sizers
    void CreateControls();

////@begin CAdjustFeaturesForGaps event handler declarations

////@end CAdjustFeaturesForGaps event handler declarations

////@begin CAdjustFeaturesForGaps member function declarations

    /// Retrieves bitmap resources
    wxBitmap GetBitmapResource( const wxString& name );

    /// Retrieves icon resources
    wxIcon GetIconResource( const wxString& name );
////@end CAdjustFeaturesForGaps member function declarations

    /// Should we show tooltips?
    static bool ShowToolTips();

    void OnKnownUnknownSelected(wxCommandEvent& event);
    void OnTrimSelected(wxCommandEvent& event);
    void OnRemoveSelected(wxCommandEvent& event);
    void OnSplitSelected(wxCommandEvent& event);
    void OnFeatTypeChanged(wxListEvent& event);
    void OnMakeReport( wxCommandEvent& event);

    CRef<CCmdComposite> GetCommand();
    bool LeaveUp() { return m_LeaveUp->GetValue(); }
    void Refresh();

private:
    void PopulateFeatureList(void);
    void GetGappedFeatures(void);
    void DisplayGappedFeatures(void);
    void EnableDisableAcceptButton(void);
    void GetTextForFeature(objects::CSeq_feat_Handle fh, string &name, string &desc, string &location);
    void RecalculateGapIntervals();
    bool x_IsFeatureTypeSelected(objects::CSeqFeatData::ESubtype subtype);
    bool x_Adjustable(objects::edit::CFeatGapInfo& fgap);
    string x_Action(objects::edit::CFeatGapInfo& fgap);
    objects::edit::TGappedFeatList x_GetSelectedFeatures();

    typedef pair<CRef<objects::edit::CFeatGapInfo>, vector<CRef<objects::CSeq_feat> > > TFeatUpdatePair;
    typedef vector<TFeatUpdatePair> TFeatUpdatePairVector;

    TFeatUpdatePairVector x_PullRelatedGroup(objects::edit::TGappedFeatList &to_edit, bool always, bool unless_pseudo, bool do_trim, bool do_split, bool split_intron, bool create_general_only);
    bool x_GroupHasFeature(objects::CSeq_feat_Handle feat, const TFeatUpdatePairVector& grp) const;
    void x_DoGroup(TFeatUpdatePairVector grp, objects::CObject_id::TId& next_id, CCmdComposite& cmd, map<objects::CObject_id::TId, objects::CObject_id::TId> &old_to_new, bool create_xref_map);
    void x_DoOne(TFeatUpdatePair& p, objects::CObject_id::TId& next_id, CCmdComposite& cmd, map<objects::CObject_id::TId, objects::CObject_id::TId> &old_to_new, bool create_xref_map);
    objects::edit::TGappedFeatList x_GetGenesForFeatures(const objects::edit::TGappedFeatList& to_do);
    CRef<CCmdComposite> GetCommand_impl(map<objects::CObject_id::TId, objects::CObject_id::TId> &old_to_new, bool create_xref_map);

////@begin CAdjustFeaturesForGaps member variables
    wxListCtrl* m_Features;
    wxStaticText* m_FeatureCountText;
    wxListCtrl* m_FeatureType;
    wxCheckBox* m_UnknownGap;
    wxCheckBox* m_KnownGap;
    wxCheckBox* m_Ns;
    wxRadioBox* m_MakeTruncatedPartial;
    wxCheckBox* m_TrimEnds;
    wxCheckBox* m_RemoveFeats;
    wxCheckBox* m_SplitForInternal;
    wxCheckBox* m_EvenIfIntrons;
    wxButton* m_AcceptButton;
    wxCheckBox *m_LeaveUp;
////@end CAdjustFeaturesForGaps member variables
    objects::CSeq_entry_Handle m_TopSeqEntry;
    wxArrayString m_FeatureTypeStrings;
    vector<int> m_feat_types, m_feat_subtypes;

    objects::edit::TGappedFeatList m_GappedFeatureList;
    CRef<objects::feature::CFeatTree> m_FeatTree;
    objects::CObject_id::TId m_TopFeatureId;
};

END_NCBI_SCOPE
#endif
    // _CADJUSTFEATURESFORGAPS_H_
