/*  $Id: select_features.cpp 38835 2017-06-23 18:15:53Z katargir $
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

#include <objects/submit/Seq_submit.hpp>
#include <objmgr/seq_entry_handle.hpp>
#include <objmgr/util/sequence.hpp>
#include <objtools/edit/seqid_guesser.hpp>
#include <gui/core/selection_service_impl.hpp>
#include <gui/core/project_service.hpp>
#include <gui/objutils/objects.hpp>
#include <gui/objutils/utils.hpp>
#include <gui/objutils/obj_event.hpp>
#include <gui/widgets/seq/flat_file_ctrl.hpp>
#include <gui/widgets/wx/wx_utils.hpp>
#include <gui/widgets/seq/text_panel.hpp>
#include <gui/widgets/text_widget/text_item_panel.hpp>
#include <gui/packages/pkg_sequence_edit/miscedit_util.hpp>
#include <gui/packages/pkg_sequence_edit/select_features.hpp>

#include <wx/sizer.h>
#include <wx/button.h>

////@begin XPM images
////@end XPM images

BEGIN_NCBI_SCOPE

USING_SCOPE(objects);

/*!
 * CSelectFeatures type definition
 */

IMPLEMENT_DYNAMIC_CLASS( CSelectFeatures, wxDialog )


/*!
 * CSelectFeatures event table definition
 */

BEGIN_EVENT_TABLE( CSelectFeatures, wxDialog )

////@begin CSelectFeatures event table entries
    EVT_BUTTON( ID_SEL_FEAT_ACCEPT, CSelectFeatures::OnAccept )
    EVT_BUTTON( ID_SEL_FEAT_CANCEL, CSelectFeatures::OnCancel )
    EVT_UPDATE_FEATURE_LIST(wxID_ANY, CSelectFeatures::ProcessUpdateFeatEvent )
////@end CSelectFeatures event table entries

END_EVENT_TABLE()


/*!
 * CSelectFeatures constructors
 */

CSelectFeatures::CSelectFeatures()
{
    Init();
}

CSelectFeatures::CSelectFeatures( wxWindow* parent, IWorkbench* wb, wxWindowID id, const wxString& caption, const wxPoint& pos, const wxSize& size, long style )
: m_Workbench(wb)
{
    Init();
    Create(parent, id, caption, pos, size, style);
}


/*!
 * CSelectFeatures creator
 */

bool CSelectFeatures::Create( wxWindow* parent, wxWindowID id, const wxString& caption, const wxPoint& pos, const wxSize& size, long style )
{
////@begin CSelectFeatures creation
    SetExtraStyle(wxWS_EX_BLOCK_EVENTS);
    wxDialog::Create( parent, id, caption, pos, size, style );

    CreateControls();
    if (GetSizer())
    {
        GetSizer()->SetSizeHints(this);
    }
    Centre();
////@end CSelectFeatures creation
    return true;
}


/*!
 * CSelectFeatures destructor
 */

CSelectFeatures::~CSelectFeatures()
{
////@begin CSelectFeatures destruction
////@end CSelectFeatures destruction
}


/*!
 * Member initialisation
 */

void CSelectFeatures::Init()
{
////@begin CSelectFeatures member initialisation
////@end CSelectFeatures member initialisation
    GetMainObject();
    m_FeatureConstraint = NULL;
    m_StringConstraintPanel = NULL;
    m_FeatureType = NULL;
}


/*!
 * Control creation for CSelectFeatures
 */

void CSelectFeatures::CreateControls()
{    
////@begin CSelectFeatures content construction
    CSelectFeatures* itemDialog1 = this;

    wxBoxSizer* itemBoxSizer2 = new wxBoxSizer(wxVERTICAL);
    itemDialog1->SetSizer(itemBoxSizer2);

    m_FeatureType = new CFeatureTypePanel(itemDialog1, wxID_ANY, wxDefaultPosition, wxSize(100, 100), 0 );
    itemBoxSizer2->Add(m_FeatureType, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxALL, 5);
    m_FeatureType->ListPresentFeaturesFirst(m_TopSeqEntry);

    m_FeatureConstraint = new CFeatureFieldNamePanel(itemDialog1, NULL, wxID_ANY, wxDefaultPosition, wxDefaultSize, 0);
    itemBoxSizer2->Add(m_FeatureConstraint, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 0);   
    m_FeatureConstraint->PopulateFeatureListbox();

    m_StringConstraintPanel = new CStringConstraintPanel(itemDialog1, false, wxID_ANY, wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer2->Add(m_StringConstraintPanel, 0, wxALIGN_CENTER_HORIZONTAL|wxALL|wxFIXED_MINSIZE, 0); 

    wxBoxSizer* itemBoxSizer4 = new wxBoxSizer(wxHORIZONTAL);
    itemBoxSizer2->Add(itemBoxSizer4, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

    wxButton* itemButton5 = new wxButton( itemDialog1, ID_SEL_FEAT_ACCEPT, _("Accept"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer4->Add(itemButton5, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxButton* itemButton7 = new wxButton( itemDialog1, ID_SEL_FEAT_CANCEL, _("Cancel"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer4->Add(itemButton7, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

////@end CSelectFeatures content construction
}


/*!
 * Should we show tooltips?
 */

bool CSelectFeatures::ShowToolTips()
{
    return true;
}

/*!
 * Get bitmap resources
 */

wxBitmap CSelectFeatures::GetBitmapResource( const wxString& name )
{
    // Bitmap retrieval
////@begin CSelectFeatures bitmap retrieval
    wxUnusedVar(name);
    return wxNullBitmap;
////@end CSelectFeatures bitmap retrieval
}

/*!
 * Get icon resources
 */

wxIcon CSelectFeatures::GetIconResource( const wxString& name )
{
    // Icon retrieval
////@begin CSelectFeatures icon retrieval
    wxUnusedVar(name);
    return wxNullIcon;
////@end CSelectFeatures icon retrieval
}


/*!
 * wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_SEL_TARGET_ACCEPT
 */


CConstRef<CObject> CSelectFeatures::GetMainObject()
{
    m_TopSeqEntry.Reset();
    CConstRef<CObject> main_object(NULL);
    if (!m_Workbench)
        return main_object;
    TConstScopedObjects objects;
    CIRef<CSelectionService> sel_srv = m_Workbench->GetServiceByType<CSelectionService>();
    if (!sel_srv) 
        return main_object;
    sel_srv->GetActiveObjects(objects);
    if (objects.empty()) 
    {
        GetViewObjects(m_Workbench, objects);
    }
    

    // look for "real" main object
    NON_CONST_ITERATE (TConstScopedObjects, it, objects) {
        const objects::CSeq_submit* seqsubmit = dynamic_cast<const objects::CSeq_submit*>(it->object.GetPointer());
        const objects::CSeq_entry* seqEntry = dynamic_cast<const objects::CSeq_entry*>(it->object.GetPointer());
        if (seqsubmit) {
            m_TopSeqEntry = GetTopSeqEntryFromScopedObject(*it);
            main_object = (*it).object;
            break;
        } else if (seqEntry) {
            m_TopSeqEntry = GetTopSeqEntryFromScopedObject(*it);
            main_object = (*it).object;
        }
    }
    
    return main_object;
}

void CSelectFeatures::OnAccept( wxCommandEvent& event )
{   
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

    string constraint_field = m_FeatureConstraint->GetFieldName();
    CRef<edit::CStringConstraint> string_constraint = m_StringConstraintPanel->GetStringConstraint();

    vector<CConstRef<CObject> > objs = col->GetObjects(m_TopSeqEntry, constraint_field, string_constraint);
    CSelectionEvent evt(m_TopSeqEntry.GetScope()); 
    CSelectionService* service = m_Workbench->GetServiceByType<CSelectionService>();
    evt.AddObjectSelection(objs);
    service->Broadcast(evt, NULL);
}


/*!
 * wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_SEL_TARGET_CANCEL
 */

void CSelectFeatures::OnCancel( wxCommandEvent& event )
{
    Close();
}

void CSelectFeatures::ProcessUpdateFeatEvent( wxCommandEvent& event )
{
        UpdateChildrenFeaturePanels(this->GetSizer());
}

void CSelectFeatures::UpdateChildrenFeaturePanels( wxSizer* sizer )
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

