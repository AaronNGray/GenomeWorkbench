/*  $Id: single_ref_gene_track_loc_panel.cpp 40132 2017-12-22 15:45:12Z bollin $
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

#include <objects/general/Object_id.hpp>
#include <gui/widgets/wx/wx_utils.hpp>
#include <gui/widgets/edit/single_ref_gene_track_loc_panel.hpp>
#include <gui/widgets/edit/ref_gene_track_loc_list_panel.hpp>

////@begin XPM images
////@end XPM images

BEGIN_NCBI_SCOPE
USING_SCOPE(objects);

/*!
 * CSingleRefGeneTrackLocPanel type definition
 */

IMPLEMENT_DYNAMIC_CLASS( CSingleRefGeneTrackLocPanel, wxPanel )


/*!
 * CSingleRefGeneTrackLocPanel event table definition
 */

BEGIN_EVENT_TABLE( CSingleRefGeneTrackLocPanel, wxPanel )

////@begin CSingleRefGeneTrackLocPanel event table entries
    EVT_TEXT( ID_ACCESSION_TXT, CSingleRefGeneTrackLocPanel::OnAccessionTxtTextUpdated )

    EVT_TEXT( ID_GI_TXT, CSingleRefGeneTrackLocPanel::OnGiTxtTextUpdated )

    EVT_TEXT( ID_START_TXT, CSingleRefGeneTrackLocPanel::OnStartTxtTextUpdated )

    EVT_TEXT( ID_STOP_TXT, CSingleRefGeneTrackLocPanel::OnStopTxtTextUpdated )

    EVT_TEXT( ID_COMMENT_TXT, CSingleRefGeneTrackLocPanel::OnCommentTxtTextUpdated )

////@end CSingleRefGeneTrackLocPanel event table entries

END_EVENT_TABLE()


/*!
 * CSingleRefGeneTrackLocPanel constructors
 */

CSingleRefGeneTrackLocPanel::CSingleRefGeneTrackLocPanel()
{
    Init();
}

CSingleRefGeneTrackLocPanel::CSingleRefGeneTrackLocPanel( wxWindow* parent, CUser_field& field, wxWindowID id, const wxPoint& pos, const wxSize& size, long style )
{
    Init();
    m_Field = new CUser_field();
    m_Field->Assign(field);    
    Create(parent, id, pos, size, style);
}


/*!
 * CSingleRefGeneTrackLocPanel creator
 */

bool CSingleRefGeneTrackLocPanel::Create( wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style )
{
////@begin CSingleRefGeneTrackLocPanel creation
    wxPanel::Create( parent, id, pos, size, style );

    CreateControls();
    if (GetSizer())
    {
        GetSizer()->SetSizeHints(this);
    }
    Centre();
////@end CSingleRefGeneTrackLocPanel creation
    return true;
}


/*!
 * CSingleRefGeneTrackLocPanel destructor
 */

CSingleRefGeneTrackLocPanel::~CSingleRefGeneTrackLocPanel()
{
////@begin CSingleRefGeneTrackLocPanel destruction
////@end CSingleRefGeneTrackLocPanel destruction
}


/*!
 * Member initialisation
 */

void CSingleRefGeneTrackLocPanel::Init()
{
////@begin CSingleRefGeneTrackLocPanel member initialisation
    m_Accession = NULL;
    m_GI = NULL;
    m_Start = NULL;
    m_Stop = NULL;
    m_Comment = NULL;
////@end CSingleRefGeneTrackLocPanel member initialisation
}


/*!
 * Control creation for CSingleRefGeneTrackLocPanel
 */

void CSingleRefGeneTrackLocPanel::CreateControls()
{    
////@begin CSingleRefGeneTrackLocPanel content construction
    CSingleRefGeneTrackLocPanel* itemPanel1 = this;

    wxBoxSizer* itemBoxSizer2 = new wxBoxSizer(wxHORIZONTAL);
    itemPanel1->SetSizer(itemBoxSizer2);

    m_Accession = new wxTextCtrl( itemPanel1, ID_ACCESSION_TXT, wxEmptyString, wxDefaultPosition, wxSize(100, -1), 0 );
    itemBoxSizer2->Add(m_Accession, 0, wxALIGN_CENTER_VERTICAL|wxALL, 0);

    m_GI = new wxTextCtrl( itemPanel1, ID_GI_TXT, wxEmptyString, wxDefaultPosition, wxSize(100, -1), 0 );
    itemBoxSizer2->Add(m_GI, 0, wxALIGN_CENTER_VERTICAL|wxALL, 0);

    m_Start = new wxTextCtrl( itemPanel1, ID_START_TXT, wxEmptyString, wxDefaultPosition, wxSize(100, -1), 0 );
    itemBoxSizer2->Add(m_Start, 0, wxALIGN_CENTER_VERTICAL|wxALL, 0);

    m_Stop = new wxTextCtrl( itemPanel1, ID_STOP_TXT, wxEmptyString, wxDefaultPosition, wxSize(100, -1), 0 );
    itemBoxSizer2->Add(m_Stop, 0, wxALIGN_CENTER_VERTICAL|wxALL, 0);

    m_Comment = new wxTextCtrl( itemPanel1, ID_COMMENT_TXT, wxEmptyString, wxDefaultPosition, wxSize(100, -1), 0 );
    itemBoxSizer2->Add(m_Comment, 0, wxALIGN_CENTER_VERTICAL|wxALL, 0);

////@end CSingleRefGeneTrackLocPanel content construction
}


bool CSingleRefGeneTrackLocPanel::TransferDataToWindow()
{
    if (!wxPanel::TransferDataToWindow())
        return false;

    string accession = "";
    int gi  = 0;
    int from = 0;
    int to = 0;
    string comment  = "";

    if (m_Field->GetData().IsFields()) {
        ITERATE(CUser_field::TData::TFields, it, m_Field->GetData().GetFields()) {
            if ((*it)->IsSetLabel() && (*it)->GetLabel().IsStr()) {
                const string& label = (*it)->GetLabel().GetStr();
                if (NStr::EqualNocase(label, "accession")) {
                    if ((*it)->GetData().IsStr()) {
                        accession = (*it)->GetData().GetStr();
                    }
                } else if (NStr::EqualNocase(label, "name")) {
                    if ((*it)->GetData().IsStr()) {
                        comment = (*it)->GetData().GetStr();
                    }
                } else if (NStr::EqualNocase(label, "gi")) {
                    if ((*it)->GetData().IsInt()) {
                        gi = (*it)->GetData().GetInt();
                    }
                } else if (NStr::EqualNocase(label, "from")) {
                    if ((*it)->GetData().IsInt()) {
                        from = (*it)->GetData().GetInt();
                    }
                } else if (NStr::EqualNocase(label, "to")) {
                    if ((*it)->GetData().IsInt()) {
                        to = (*it)->GetData().GetInt();
                    }
                }
            }
        }
    }

    m_Accession->SetValue(ToWxString(accession));
    if (gi > 0) {
        m_GI->SetValue(ToWxString(NStr::NumericToString(gi)));
    } else {
        m_GI->SetValue(wxEmptyString);
    }
    if (from > 0) {
        m_Start->SetValue(ToWxString(NStr::NumericToString(from)));
    } else {
        m_Start->SetValue(wxEmptyString);
    }
    if (to > 0) {
        m_Stop->SetValue(ToWxString(NStr::NumericToString(to)));
    } else {
        m_Stop->SetValue(wxEmptyString);
    }

    m_Comment->SetValue(ToWxString(comment));

    return true;
}


bool CSingleRefGeneTrackLocPanel::x_IsControlled (const string& field)
{
    if (NStr::EqualNocase(field, "accession")
        || NStr::EqualNocase(field, "name")
        || NStr::EqualNocase(field, "to")
        || NStr::EqualNocase(field, "from")
        || NStr::EqualNocase(field, "gi")) {
        return true;
    } else {
        return false;
    }
}


bool CSingleRefGeneTrackLocPanel::TransferDataFromWindow()
{
    if (!wxPanel::TransferDataFromWindow())
        return false;

    string accession = ToStdString(m_Accession->GetValue());
    string comment = ToStdString(m_Comment->GetValue());
    int gi = 0;
    int from = 0;
    int to = 0;
    try {
        gi = NStr::StringToInt (ToStdString(m_GI->GetValue()));
    } catch(CException &e) {} catch (exception &e) {}


    try {
        from = NStr::StringToInt(ToStdString(m_Start->GetValue()));
    } catch(CException &e) {} catch (exception &e) {}

    try {
        to = NStr::StringToInt(ToStdString(m_Stop->GetValue()));
    } catch(CException &e) {} catch (exception &e) {}

    m_Field->SetLabel().SetId(0);
    
    CUser_field::TData::TFields::iterator it = m_Field->SetData().SetFields().begin();
    while (it != m_Field->SetData().SetFields().end()) {
        if ((*it)->IsSetLabel() && (*it)->GetLabel().IsStr() && x_IsControlled((*it)->GetLabel().GetStr())) {
            it = m_Field->SetData().SetFields().erase(it);
        } else {
            ++it;
        }
    }

    if (!NStr::IsBlank(accession)) {
        CRef<CUser_field> acc_field(new CUser_field());
        acc_field->SetLabel().SetStr("accession");
        acc_field->SetData().SetStr(accession);
        m_Field->SetData().SetFields().push_back(acc_field);
    }
    if (!NStr::IsBlank(comment)) {
        CRef<CUser_field> comment_field(new CUser_field());
        comment_field->SetLabel().SetStr("name");
        comment_field->SetData().SetStr(comment);
        m_Field->SetData().SetFields().push_back(comment_field);
    }
    if (to > 0 && from > 0) {
        CRef<CUser_field> from_field(new CUser_field());
        from_field->SetLabel().SetStr("from");
        from_field->SetData().SetInt(from);
        m_Field->SetData().SetFields().push_back(from_field);
        CRef<CUser_field> to_field(new CUser_field());
        to_field->SetLabel().SetStr("to");
        to_field->SetData().SetInt(to);
        m_Field->SetData().SetFields().push_back(to_field);
    }
    if (gi > 0) {
        CRef<CUser_field> gi_field(new CUser_field());
        gi_field->SetLabel().SetStr("gi");
        gi_field->SetData().SetInt(gi);
        m_Field->SetData().SetFields().push_back(gi_field);
    }

    return true;
}


CRef<CUser_field> CSingleRefGeneTrackLocPanel::GetUser_field()
{
    CRef<CUser_field> new_field;
    if (m_Field) {
        new_field.Reset(new CUser_field());
        new_field->Assign(*m_Field);
    }

    return new_field;
}


/*!
 * Should we show tooltips?
 */

bool CSingleRefGeneTrackLocPanel::ShowToolTips()
{
    return true;
}

/*!
 * Get bitmap resources
 */

wxBitmap CSingleRefGeneTrackLocPanel::GetBitmapResource( const wxString& name )
{
    // Bitmap retrieval
////@begin CSingleRefGeneTrackLocPanel bitmap retrieval
    wxUnusedVar(name);
    return wxNullBitmap;
////@end CSingleRefGeneTrackLocPanel bitmap retrieval
}

/*!
 * Get icon resources
 */

wxIcon CSingleRefGeneTrackLocPanel::GetIconResource( const wxString& name )
{
    // Icon retrieval
////@begin CSingleRefGeneTrackLocPanel icon retrieval
    wxUnusedVar(name);
    return wxNullIcon;
////@end CSingleRefGeneTrackLocPanel icon retrieval
}


/*!
 * wxEVT_COMMAND_TEXT_UPDATED event handler for ID_ACCESSION_TXT
 */

void CSingleRefGeneTrackLocPanel::OnAccessionTxtTextUpdated( wxCommandEvent& event )
{
    wxTextCtrl* item = (wxTextCtrl*)event.GetEventObject();
    if (NStr::IsBlank(ToStdString(item->GetValue()))) {
        return;
    }
    x_OnChange();
}


/*!
 * wxEVT_COMMAND_TEXT_UPDATED event handler for ID_GI_TXT
 */

void CSingleRefGeneTrackLocPanel::OnGiTxtTextUpdated( wxCommandEvent& event )
{
    wxTextCtrl* item = (wxTextCtrl*)event.GetEventObject();
    if (NStr::IsBlank(ToStdString(item->GetValue()))) {
        return;
    }
    x_OnChange();
}


/*!
 * wxEVT_COMMAND_TEXT_UPDATED event handler for ID_START_TXT
 */

void CSingleRefGeneTrackLocPanel::OnStartTxtTextUpdated( wxCommandEvent& event )
{
    wxTextCtrl* item = (wxTextCtrl*)event.GetEventObject();
    if (NStr::IsBlank(ToStdString(item->GetValue()))) {
        return;
    }
    x_OnChange();
}


/*!
 * wxEVT_COMMAND_TEXT_UPDATED event handler for ID_STOP_TXT
 */

void CSingleRefGeneTrackLocPanel::OnStopTxtTextUpdated( wxCommandEvent& event )
{
    wxTextCtrl* item = (wxTextCtrl*)event.GetEventObject();
    if (NStr::IsBlank(ToStdString(item->GetValue()))) {
        return;
    }
    x_OnChange();
}


/*!
 * wxEVT_COMMAND_TEXT_UPDATED event handler for ID_COMMENT_TXT
 */

void CSingleRefGeneTrackLocPanel::OnCommentTxtTextUpdated( wxCommandEvent& event )
{
    wxTextCtrl* item = (wxTextCtrl*)event.GetEventObject();
    if (NStr::IsBlank(ToStdString(item->GetValue()))) {
        return;
    }
    x_OnChange();
}


END_NCBI_SCOPE
