/*  $Id: ref_gene_track_loc_list_panel.cpp 40132 2017-12-22 15:45:12Z bollin $
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
#include <objects/general/Object_id.hpp>
#include <gui/widgets/wx/wx_utils.hpp>

////@begin includes
////@end includes

#include <gui/widgets/edit/ref_gene_track_loc_list_panel.hpp>
#include <gui/widgets/edit/single_ref_gene_track_loc_panel.hpp>

////@begin XPM images
////@end XPM images

BEGIN_NCBI_SCOPE
USING_SCOPE(objects);

/*!
 * CRefGeneTrackLocListPanel type definition
 */

IMPLEMENT_DYNAMIC_CLASS( CRefGeneTrackLocListPanel, wxPanel )


/*!
 * CRefGeneTrackLocListPanel event table definition
 */

BEGIN_EVENT_TABLE( CRefGeneTrackLocListPanel, wxPanel )

////@begin CRefGeneTrackLocListPanel event table entries
    EVT_CHOICE( ID_TYPE_CHOICE, CRefGeneTrackLocListPanel::OnTypeChoiceSelected )

////@end CRefGeneTrackLocListPanel event table entries

END_EVENT_TABLE()


/*!
 * CRefGeneTrackLocListPanel constructors
 */

CRefGeneTrackLocListPanel::CRefGeneTrackLocListPanel()
{
    Init();
}

CRefGeneTrackLocListPanel::CRefGeneTrackLocListPanel( wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style )
{
    Init();
    m_UserObj = new CUser_object();
    Create(parent, id, pos, size, style);
    x_UpdateFields();
    if (m_Fields.size() > 0 && m_Fields.front()->IsSetLabel() && m_Fields.front()->GetLabel().IsStr()) {
        m_Type->SetStringSelection(m_Fields.front()->GetLabel().GetStr());
    }
}


/*!
 * CRefGeneTrackLocListPanel creator
 */

bool CRefGeneTrackLocListPanel::Create( wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style )
{
////@begin CRefGeneTrackLocListPanel creation
    wxPanel::Create( parent, id, pos, size, style );

    CreateControls();
    if (GetSizer())
    {
        GetSizer()->SetSizeHints(this);
    }
    Centre();
////@end CRefGeneTrackLocListPanel creation
    return true;
}


/*!
 * CRefGeneTrackLocListPanel destructor
 */

CRefGeneTrackLocListPanel::~CRefGeneTrackLocListPanel()
{
////@begin CRefGeneTrackLocListPanel destruction
////@end CRefGeneTrackLocListPanel destruction
    delete m_FieldManager;
}


/*!
 * Member initialisation
 */

void CRefGeneTrackLocListPanel::Init()
{
////@begin CRefGeneTrackLocListPanel member initialisation
    m_Type = NULL;
////@end CRefGeneTrackLocListPanel member initialisation
    m_ScrolledWindow = NULL;
    m_Sizer = NULL;
    m_FieldManager = new CRefGeneTrackFieldManager();
    SetNeedsEmptyLastRow(false);
}


/*!
 * Control creation for CRefGeneTrackLocListPanel
 */

void CRefGeneTrackLocListPanel::CreateControls()
{    
////@begin CRefGeneTrackLocListPanel content construction
    CRefGeneTrackLocListPanel* itemPanel1 = this;

    wxBoxSizer* itemBoxSizer2 = new wxBoxSizer(wxVERTICAL);
    itemPanel1->SetSizer(itemBoxSizer2);

    wxBoxSizer* itemBoxSizer3 = new wxBoxSizer(wxHORIZONTAL);
    itemBoxSizer2->Add(itemBoxSizer3, 0, wxALIGN_LEFT|wxALL, 5);

    wxStaticText* itemStaticText4 = new wxStaticText( itemPanel1, wxID_STATIC, _("Type"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer3->Add(itemStaticText4, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxArrayString m_TypeStrings;
    m_TypeStrings.Add(_("Assembly"));
    m_TypeStrings.Add(_("IdenticalTo"));
    m_Type = new wxChoice( itemPanel1, ID_TYPE_CHOICE, wxDefaultPosition, wxSize(100, -1), m_TypeStrings, 0 );
    m_Type->SetStringSelection(_("Assembly"));
    m_Type->SetName(_T("Type"));
    itemBoxSizer3->Add(m_Type, 0, wxALIGN_CENTER_VERTICAL|wxALL, 0);

    wxBoxSizer* itemBoxSizer6 = new wxBoxSizer(wxHORIZONTAL);
    itemBoxSizer2->Add(itemBoxSizer6, 0, wxALIGN_LEFT|wxALL, 5);

    wxStaticText* itemStaticText7 = new wxStaticText( itemPanel1, wxID_STATIC, _("Accession"), wxDefaultPosition, wxSize(100, -1), 0 );
    itemBoxSizer6->Add(itemStaticText7, 0, wxALIGN_CENTER_VERTICAL|wxALL, 0);

    wxStaticText* itemStaticText8 = new wxStaticText( itemPanel1, wxID_STATIC, _("GI"), wxDefaultPosition, wxSize(100, -1), 0 );
    itemBoxSizer6->Add(itemStaticText8, 0, wxALIGN_CENTER_VERTICAL|wxALL, 0);

    wxStaticText* itemStaticText9 = new wxStaticText( itemPanel1, wxID_STATIC, _("Start"), wxDefaultPosition, wxSize(100, -1), 0 );
    itemBoxSizer6->Add(itemStaticText9, 0, wxALIGN_CENTER_VERTICAL|wxALL, 0);

    wxStaticText* itemStaticText10 = new wxStaticText( itemPanel1, wxID_STATIC, _("Stop"), wxDefaultPosition, wxSize(100, -1), 0 );
    itemBoxSizer6->Add(itemStaticText10, 0, wxALIGN_CENTER_VERTICAL|wxALL, 0);

    wxStaticText* itemStaticText11 = new wxStaticText( itemPanel1, wxID_STATIC, _("Comment"), wxDefaultPosition, wxSize(100, -1), 0 );
    itemBoxSizer6->Add(itemStaticText11, 0, wxALIGN_CENTER_VERTICAL|wxALL, 0);

////@end CRefGeneTrackLocListPanel content construction
    m_ScrolledWindow = new wxScrolledWindow( itemPanel1, wxID_ANY, wxDefaultPosition, wxSize(610, 100), wxSUNKEN_BORDER|wxHSCROLL|wxVSCROLL );
    itemBoxSizer2->Add(m_ScrolledWindow, 1, wxALIGN_CENTER_HORIZONTAL|wxALL, 0);
    m_ScrolledWindow->SetScrollbars(1, 1, 0, 0);
    m_Sizer = new wxBoxSizer(wxVERTICAL);
    m_ScrolledWindow->SetSizer(m_Sizer);

    m_ScrolledWindow->FitInside();

}


/*!
 * Should we show tooltips?
 */

bool CRefGeneTrackLocListPanel::ShowToolTips()
{
    return true;
}

/*!
 * Get bitmap resources
 */

wxBitmap CRefGeneTrackLocListPanel::GetBitmapResource( const wxString& name )
{
    // Bitmap retrieval
////@begin CRefGeneTrackLocListPanel bitmap retrieval
    wxUnusedVar(name);
    return wxNullBitmap;
////@end CRefGeneTrackLocListPanel bitmap retrieval
}

/*!
 * Get icon resources
 */

wxIcon CRefGeneTrackLocListPanel::GetIconResource( const wxString& name )
{
    // Icon retrieval
////@begin CRefGeneTrackLocListPanel icon retrieval
    wxUnusedVar(name);
    return wxNullIcon;
////@end CRefGeneTrackLocListPanel icon retrieval
}


bool CRefGeneTrackLocListPanel::TransferDataToWindow()
{
    bool add_last_row = false;
    if (m_Fields.size() > 0 && m_Fields.front()->IsSetLabel() && m_Fields.front()->GetLabel().IsStr()) {
        const string& field_type = m_Fields.front()->GetLabel().GetStr();
        m_Type->SetStringSelection(field_type);
        if (NStr::Equal(field_type, "IdenticalTo")) {
            add_last_row = false;
        } else {
            add_last_row = true;
        }
    }
    SetNeedsEmptyLastRow(add_last_row);
    return CUserFieldListPanel::TransferDataToWindow();
}


bool CRefGeneTrackLocListPanel::TransferDataFromWindow()
{
    if (!wxPanel::TransferDataFromWindow())
        return false;

    m_Fields.clear();
    CRef<CUser_field> new_field(new CUser_field());
    new_field->SetLabel().SetStr(ToStdString(m_Type->GetStringSelection()));
    new_field->ResetData();

    wxSizerItemList::iterator node = m_Sizer->GetChildren().begin();
    for (; node != m_Sizer->GetChildren().end(); ++node) {
        wxWindow *w = (*node)->GetWindow();
        if (!w) {
            continue;
        }
        w->TransferDataFromWindow();
        CRef<CUser_field> edited_field = m_FieldManager->GetUserField(w);
        if (edited_field && !m_FieldManager->IsEmpty(*edited_field)) {
            new_field->SetData().SetFields().push_back(edited_field);
        }
    }
    if (new_field->IsSetData() && new_field->GetData().IsFields()) {
        m_Fields.push_back(new_field);
    }
    return true;
}


/*!
 * wxEVT_COMMAND_CHOICE_SELECTED event handler for ID_TYPE_CHOICE
 */

void CRefGeneTrackLocListPanel::OnTypeChoiceSelected(wxCommandEvent& event)
{
    TransferDataFromWindow();
    const string field_type = ToStdString(m_Type->GetStringSelection());
    if (NStr::Equal(field_type, "IdenticalTo")) {
        SetNeedsEmptyLastRow(false);
    } else {
        SetNeedsEmptyLastRow(true);
    }
    TransferDataToWindow();
}


bool CRefGeneTrackLocListPanel::IsEmpty()
{
    TransferDataFromWindow();
    return m_Fields.empty();
}

END_NCBI_SCOPE

