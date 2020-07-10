/*  $Id: desc_placement_panel.cpp 43255 2019-06-06 14:38:10Z filippov $
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
 * Authors:  Colleen Bollin
 */


#include <ncbi_pch.hpp>
////@begin includes
////@end includes

#include <wx/sizer.h>
#include <wx/stattext.h>

#include <objmgr/util/sequence.hpp>
#include <gui/widgets/wx/wx_utils.hpp>
#include <gui/widgets/edit/desc_placement_panel.hpp>

////@begin XPM images
////@end XPM images

BEGIN_NCBI_SCOPE
USING_SCOPE(objects);

/*!
 * CDescPlacementPanel type definition
 */

IMPLEMENT_DYNAMIC_CLASS( CDescPlacementPanel, wxPanel )


/*!
 * CDescPlacementPanel event table definition
 */

BEGIN_EVENT_TABLE( CDescPlacementPanel, wxPanel )

////@begin CDescPlacementPanel event table entries
    EVT_CHOICE( ID_DESC_PLACEMENT_CTRL, CDescPlacementPanel::OnDescPlacementCtrlSelected )

////@end CDescPlacementPanel event table entries

END_EVENT_TABLE()


/*!
 * CDescPlacementPanel constructors
 */

CDescPlacementPanel::CDescPlacementPanel()
{
    Init();
}

CDescPlacementPanel::CDescPlacementPanel( wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style )
{
    Init();
    Create(parent, id, pos, size, style);
}


/*!
 * CDescPlacementPanel creator
 */

bool CDescPlacementPanel::Create( wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style )
{
////@begin CDescPlacementPanel creation
    wxPanel::Create( parent, id, pos, size, style );

    CreateControls();
    if (GetSizer())
    {
        GetSizer()->SetSizeHints(this);
    }
    Centre();
////@end CDescPlacementPanel creation
    return true;
}


/*!
 * CDescPlacementPanel destructor
 */

CDescPlacementPanel::~CDescPlacementPanel()
{
////@begin CDescPlacementPanel destruction
////@end CDescPlacementPanel destruction
}


/*!
 * Member initialisation
 */

void CDescPlacementPanel::Init()
{
////@begin CDescPlacementPanel member initialisation
    m_Location = NULL;
////@end CDescPlacementPanel member initialisation
}


/*!
 * Control creation for CDescPlacementPanel
 */

void CDescPlacementPanel::CreateControls()
{    
////@begin CDescPlacementPanel content construction
    CDescPlacementPanel* itemPanel1 = this;

    wxBoxSizer* itemBoxSizer2 = new wxBoxSizer(wxVERTICAL);
    itemPanel1->SetSizer(itemBoxSizer2);

    wxStaticText* itemStaticText3 = new wxStaticText( itemPanel1, wxID_STATIC, _("Descriptors may apply to a single sequence or to a set of sequences. Please choose the location for the new descriptor."), wxDefaultPosition, wxDefaultSize, 0 );
    itemStaticText3->Wrap(200);
    itemBoxSizer2->Add(itemStaticText3, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

    wxArrayString m_LocationStrings;
    m_Location = new wxChoice( itemPanel1, ID_DESC_PLACEMENT_CTRL, wxDefaultPosition, wxDefaultSize, m_LocationStrings, 0 );
    itemBoxSizer2->Add(m_Location, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

////@end CDescPlacementPanel content construction
}


/*!
 * Should we show tooltips?
 */

bool CDescPlacementPanel::ShowToolTips()
{
    return true;
}

/*!
 * Get bitmap resources
 */

wxBitmap CDescPlacementPanel::GetBitmapResource( const wxString& name )
{
    // Bitmap retrieval
////@begin CDescPlacementPanel bitmap retrieval
    wxUnusedVar(name);
    return wxNullBitmap;
////@end CDescPlacementPanel bitmap retrieval
}

/*!
 * Get icon resources
 */

wxIcon CDescPlacementPanel::GetIconResource( const wxString& name )
{
    // Icon retrieval
////@begin CDescPlacementPanel icon retrieval
    wxUnusedVar(name);
    return wxNullIcon;
////@end CDescPlacementPanel icon retrieval
}


void CDescPlacementPanel::x_AddToStrings(const CSeq_entry& se, CScope& scope)
{
    string label;

    if (se.IsSeq()) {
        CSeq_entry_Handle seh = scope.GetSeq_entryHandle(se);
        CConstRef<CSeq_id> seqid = sequence::GetId(seh.GetSeq(), sequence::eGetId_Best).GetSeqId();
        label = seqid->GetSeqIdString();
        m_List.push_back(TNameSeqEntry(label, seh));
    } else if (se.IsSet()) {
        // TODO: Add label for set
        se.GetSet().GetLabel(&label, CBioseq_set::eBoth);            
        m_List.push_back(TNameSeqEntry(label, scope.GetSeq_entryHandle(se)));
        ITERATE(CBioseq_set::TSeq_set, it, se.GetSet().GetSeq_set()) {
            x_AddToStrings(**it, scope);
        }
    }
}


void CDescPlacementPanel::PopulateLocationChoices (CSeq_entry_Handle seh)
{
    m_Location->Clear();
    x_AddToStrings(*(seh.GetCompleteSeq_entry()), seh.GetScope());
    wxArrayString choices;
    ITERATE(TNameSeqEntryList, it, m_List) {
        choices.push_back(ToWxString(it->first));
    }
    m_Location->Append(choices);
    m_Location->SetSelection(0);
    m_Location->Enable(true);
    m_Location->Update();
}


CSeq_entry_Handle CDescPlacementPanel::GetLocationChoice ()
{
    int pos = m_Location->GetSelection();
    return m_List[pos].second;
}


/*!
 * wxEVT_COMMAND_CHOICE_SELECTED event handler for ID_DESC_PLACEMENT_CTRL
 */

void CDescPlacementPanel::OnDescPlacementCtrlSelected( wxCommandEvent& event )
{
    if (m_Notify) {
        int pos = m_Location->GetSelection();
        wxWindow *win = GetParent();
        wxTopLevelWindow* gui_widget = nullptr;
        while (win && !gui_widget)
        {
            wxTopLevelWindow* gui_widget = dynamic_cast<wxTopLevelWindow*>(win);
            win = win->GetParent();
        }
        m_Notify->LocationChange(m_List[pos].second, gui_widget);
    }
}


END_NCBI_SCOPE


