/*  $Id: bam_ref_sequences_panel.cpp 44710 2020-02-27 18:03:07Z shkeda $
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
 * Authors:  
 */


#include <ncbi_pch.hpp>
#include <wx/sizer.h>
#include <wx/stattext.h>
#include <wx/icon.h>
#include <wx/utils.h>
#include <wx/button.h>

#include <gui/objutils/label.hpp>
#include <objmgr/scope.hpp>
#include <objmgr/object_manager.hpp>
#include <gui/widgets/wx/wx_utils.hpp>

////@begin includes
////@end includes

#include <gui/widgets/loaders/bam_ref_sequences_panel.hpp>

////@begin XPM images
////@end XPM images

BEGIN_NCBI_SCOPE
USING_SCOPE(objects);

/*!
 * CBamRefSequencesPanel type definition
 */

IMPLEMENT_DYNAMIC_CLASS( CBamRefSequencesPanel, wxPanel )


/*!
 * CBamRefSequencesPanel event table definition
 */

BEGIN_EVENT_TABLE( CBamRefSequencesPanel, wxPanel )

////@begin CBamRefSequencesPanel event table entries
    EVT_IDLE( CBamRefSequencesPanel::OnIdle )
    EVT_BUTTON( ID_SELECT_ALL_BTN, CBamRefSequencesPanel::OnSelectAllBtnClick )
    EVT_BUTTON( ID_DESELECT_ALL_BTN, CBamRefSequencesPanel::OnDeselectAllBtnClick )
    EVT_SEARCHCTRL_SEARCH_BTN( ID_SEARCHCTRL, CBamRefSequencesPanel::OnSearchButtonClick )
    EVT_SEARCHCTRL_CANCEL_BTN( ID_SEARCHCTRL, CBamRefSequencesPanel::OnSearchCancelClick )
    EVT_TEXT_ENTER( ID_SEARCHCTRL, CBamRefSequencesPanel::OnSearchEnter )
    EVT_UPDATE_UI( ID_SEARCHCTRL, CBamRefSequencesPanel::OnSearchCtrlUpdate )
////@end CBamRefSequencesPanel event table entries

END_EVENT_TABLE()


/*!
 * CBamRefSequencesPanel constructors
 */

CBamRefSequencesPanel::CBamRefSequencesPanel()
{
    Init();
}

CBamRefSequencesPanel::CBamRefSequencesPanel( wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style )
{
    Init();
    Create(parent, id, pos, size, style);
}


/*!
 * CBAMRefSequencesPanel creator
 */

bool CBamRefSequencesPanel::Create( wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style )
{
////@begin CBamRefSequencesPanel creation
    SetExtraStyle(wxWS_EX_VALIDATE_RECURSIVELY);
    wxPanel::Create( parent, id, pos, size, style );

    CreateControls();
    if (GetSizer())
    {
        GetSizer()->SetSizeHints(this);
    }
    Centre();
////@end CBamRefSequencesPanel creation
    return true;
}


/*!
 * CBamRefSequencesPanel destructor
 */

CBamRefSequencesPanel::~CBamRefSequencesPanel()
{
////@begin CBamRefSequencesPanel destruction
////@end CBamRefSequencesPanel destruction
}


/*!
 * Member initialisation
 */

void CBamRefSequencesPanel::Init()
{
////@begin CBamRefSequencesPanel member initialisation
    m_RefSeqList = NULL;
    m_SearchCtrl = NULL;
////@end CBamRefSequencesPanel member initialisation
}


/*!
 * Control creation for CBAMRefSequencesPanel
 */

void CBamRefSequencesPanel::CreateControls()
{    
////@begin CBamRefSequencesPanel content construction
    CBamRefSequencesPanel* itemPanel1 = this;

    wxBoxSizer* itemBoxSizer2 = new wxBoxSizer(wxVERTICAL);
    itemPanel1->SetSizer(itemBoxSizer2);

    wxStaticText* itemStaticText3 = new wxStaticText( itemPanel1, wxID_STATIC, _("The following sequences are referenced in the file. Please choose the ones to add to the project:"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer2->Add(itemStaticText3, 0, wxALIGN_LEFT|wxALL, 5);

    wxArrayString m_RefSeqListStrings;
    m_RefSeqList = new wxCheckListBox( itemPanel1, ID_CHECKLISTBOX, wxDefaultPosition, wxDefaultSize, m_RefSeqListStrings, wxLB_SINGLE );
    itemBoxSizer2->Add(m_RefSeqList, 1, wxGROW|wxALL, 5);

    wxBoxSizer* itemBoxSizer5 = new wxBoxSizer(wxHORIZONTAL);
    itemBoxSizer2->Add(itemBoxSizer5, 0, wxGROW|wxTOP, 5);

    wxButton* itemButton6 = new wxButton( itemPanel1, ID_SELECT_ALL_BTN, _("Select All"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer5->Add(itemButton6, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxButton* itemButton7 = new wxButton( itemPanel1, ID_DESELECT_ALL_BTN, _("Deselect All"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer5->Add(itemButton7, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    itemBoxSizer5->Add(5, 5, 1, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxStaticText* itemStaticText9 = new wxStaticText( itemPanel1, wxID_STATIC, _("Filter:"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer5->Add(itemStaticText9, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    m_SearchCtrl = new wxSearchCtrl( itemPanel1, ID_SEARCHCTRL, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_PROCESS_ENTER );
    itemBoxSizer5->Add(m_SearchCtrl, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

////@end CBamRefSequencesPanel content construction
}


/*!
 * Should we show tooltips?
 */

bool CBamRefSequencesPanel::ShowToolTips()
{
    return true;
}

/*!
 * Get bitmap resources
 */

wxBitmap CBamRefSequencesPanel::GetBitmapResource( const wxString& name )
{
    // Bitmap retrieval
////@begin CBamRefSequencesPanel bitmap retrieval
    wxUnusedVar(name);
    return wxNullBitmap;
////@end CBamRefSequencesPanel bitmap retrieval
}

/*!
 * Get icon resources
 */

wxIcon CBamRefSequencesPanel::GetIconResource( const wxString& name )
{
    // Icon retrieval
////@begin CBamRefSequencesPanel icon retrieval
    wxUnusedVar(name);
    return wxNullIcon;
////@end CBamRefSequencesPanel icon retrieval
}

void CBamRefSequencesPanel::SetReferenceSeqIds(const vector<CConstRef<CSeq_id> >& refSeqIds)
{
    m_RefSeqIds = refSeqIds;
    m_RefSeqList->Clear();

    if (!m_Scope) {
        CRef<CObjectManager> obj_mgr = CObjectManager::GetInstance();
        m_Scope.Reset(new CScope(*obj_mgr));
        m_Scope->AddDefaults();
    }

    x_FillList(NcbiEmptyString);
}

void CBamRefSequencesPanel::GetSelectedReferenceSeqIds(vector<CConstRef<CSeq_id> >& selSeqIds) const
{
    selSeqIds.clear();
    wxArrayInt checkedItems;
    m_RefSeqList->GetCheckedItems(checkedItems);
    
    int checked = checkedItems.size();
    for (int i=0; i < checked; ++i) {
        CConstRef<CSeq_id> seqId((CSeq_id*)m_RefSeqList->GetClientData(checkedItems[i]));
        selSeqIds.push_back(seqId);
    }
}

void CBamRefSequencesPanel::OnSelectAllBtnClick( wxCommandEvent& event )
{
    for (unsigned i=0; i < m_RefSeqList->GetCount(); ++i)
        m_RefSeqList->Check(i);
}

void CBamRefSequencesPanel::OnDeselectAllBtnClick( wxCommandEvent& event )
{
    for (unsigned i = 0; i < m_RefSeqList->GetCount(); ++i)
        m_RefSeqList->Check(i, false);
}

void CBamRefSequencesPanel::OnSearchButtonClick( wxCommandEvent& event )
{
    x_FillList(string(event.GetString().ToUTF8()));
}

void CBamRefSequencesPanel::OnSearchCancelClick( wxCommandEvent& event )
{
    x_FillList(NcbiEmptyString);
}

void CBamRefSequencesPanel::OnSearchEnter( wxCommandEvent& event )
{
    x_FillList(string(m_SearchCtrl->GetValue().ToUTF8()));
}

void CBamRefSequencesPanel::x_FillList(const string& filter)
{

    m_RefSeqList->Clear();
    m_IdsWithNoDescriptions.clear();

    m_CurrentFilter = filter;
    NStr::TruncateSpacesInPlace(m_CurrentFilter);

    sort(m_RefSeqIds.begin(), m_RefSeqIds.end(), [](const CConstRef<CSeq_id>& a, const CConstRef<CSeq_id>& b) {
        int a_local = a->IsLocal() ? 1 : 0;
        int b_local = b->IsLocal() ? 1 : 0;
        if (a_local != b_local)
            return a_local < b_local;
        return a->GetSeqIdString(true) < b->GetSeqIdString(true);
    });

    for (const auto& id : m_RefSeqIds) {
        string seqIdLabel(id->GetSeqIdString(true));
        if (!filter.empty() && NStr::FindNoCase(seqIdLabel, m_CurrentFilter) == NPOS)
            continue;

        int newItem = m_RefSeqList->Append(seqIdLabel, (void*)id.GetPointerOrNull());
        if (!id->IsLocal()) {
            m_IdsWithNoDescriptions.push_back(newItem);
            m_RefSeqList->Check(newItem);
        }
    }
}

void CBamRefSequencesPanel::OnSearchCtrlUpdate( wxUpdateUIEvent& event )
{
    m_SearchCtrl->ShowCancelButton(!m_CurrentFilter.empty());
}


/*!
 * wxEVT_IDLE event handler for ID_CBAMREFSEQUENCESPANEL
 */

void CBamRefSequencesPanel::OnIdle(wxIdleEvent& event) 
{
    if (!m_IdsWithNoDescriptions.empty()) {
        auto item_id = m_IdsWithNoDescriptions.front();
        try {
            CConstRef<CSeq_id> id((CSeq_id*)m_RefSeqList->GetClientData(item_id));
            string seqIdDesc;
            CLabel::GetLabel(*id, &seqIdDesc, CLabel::eDescription, m_Scope);
            string seqIdLabel(id->GetSeqIdString(true));
            seqIdLabel += " (";
            seqIdLabel += seqIdDesc;
            seqIdLabel += ")";
            m_RefSeqList->SetString(item_id, ToWxString(seqIdLabel));
        } catch (exception& e) {
            ERR_POST(Error << e.what());
        }
        m_IdsWithNoDescriptions.pop_front();
    }
    event.Skip();
}


END_NCBI_SCOPE



