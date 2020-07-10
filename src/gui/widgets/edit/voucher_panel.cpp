/*  $Id: voucher_panel.cpp 29060 2013-10-01 19:21:17Z bollin $
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

#include <gui/widgets/edit/voucher_panel.hpp>
#include <gui/widgets/wx/wx_utils.hpp>
#include <wx/stattext.h>
#include <wx/sizer.h>

////@begin XPM images
////@end XPM images

BEGIN_NCBI_SCOPE

/*
 * CVoucherPanel type definition
 */

IMPLEMENT_DYNAMIC_CLASS( CVoucherPanel, CSrcModEditPanel )


/*
 * CVoucherPanel event table definition
 */

BEGIN_EVENT_TABLE( CVoucherPanel, CSrcModEditPanel )

////@begin CVoucherPanel event table entries
    EVT_TEXT( ID_VOUCHER_INST, CVoucherPanel::OnVoucherInstTextUpdated )

    EVT_TEXT( ID_VOUCHER_COLL, CVoucherPanel::OnVoucherCollTextUpdated )

    EVT_TEXT( ID_VOUCHER_TEXT, CVoucherPanel::OnVoucherTextTextUpdated )

////@end CVoucherPanel event table entries

END_EVENT_TABLE()


/*
 * CVoucherPanel constructors
 */

CVoucherPanel::CVoucherPanel() : CSrcModEditPanel(CSrcModEditPanel::eVoucher)
{
    Init();
}

CVoucherPanel::CVoucherPanel( wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style )
    : CSrcModEditPanel(CSrcModEditPanel::eVoucher)
{
    Init();
    Create(parent, id, pos, size, style);
}


/*
 * CVoucherPanel creator
 */

bool CVoucherPanel::Create( wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style )
{
////@begin CVoucherPanel creation
    CSrcModEditPanel::Create( parent, id, pos, size, style );

    CreateControls();
    if (GetSizer())
    {
        GetSizer()->SetSizeHints(this);
    }
    Centre();
////@end CVoucherPanel creation
    return true;
}


/*
 * CVoucherPanel destructor
 */

CVoucherPanel::~CVoucherPanel()
{
////@begin CVoucherPanel destruction
////@end CVoucherPanel destruction
}


/*
 * Member initialisation
 */

void CVoucherPanel::Init()
{
////@begin CVoucherPanel member initialisation
    m_Inst = NULL;
    m_Coll = NULL;
    m_ID = NULL;
////@end CVoucherPanel member initialisation
}


/*
 * Control creation for CVoucherPanel
 */

void CVoucherPanel::CreateControls()
{    
////@begin CVoucherPanel content construction
    CVoucherPanel* itemCSrcModEditPanel1 = this;

    wxBoxSizer* itemBoxSizer2 = new wxBoxSizer(wxHORIZONTAL);
    itemCSrcModEditPanel1->SetSizer(itemBoxSizer2);

    wxStaticText* itemStaticText3 = new wxStaticText( itemCSrcModEditPanel1, wxID_STATIC, _("Inst"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer2->Add(itemStaticText3, 0, wxALIGN_CENTER_VERTICAL|wxALL, 0);

    m_Inst = new wxTextCtrl( itemCSrcModEditPanel1, ID_VOUCHER_INST, wxEmptyString, wxDefaultPosition, wxSize(60, -1), 0 );
    itemBoxSizer2->Add(m_Inst, 0, wxALIGN_CENTER_VERTICAL|wxALL, 0);

    wxStaticText* itemStaticText5 = new wxStaticText( itemCSrcModEditPanel1, wxID_STATIC, _("Coll"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer2->Add(itemStaticText5, 0, wxALIGN_CENTER_VERTICAL|wxALL, 0);

    m_Coll = new wxTextCtrl( itemCSrcModEditPanel1, ID_VOUCHER_COLL, wxEmptyString, wxDefaultPosition, wxSize(60, -1), 0 );
    itemBoxSizer2->Add(m_Coll, 0, wxALIGN_CENTER_VERTICAL|wxALL, 0);

    wxStaticText* itemStaticText7 = new wxStaticText( itemCSrcModEditPanel1, wxID_STATIC, _("SpecID/Text"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer2->Add(itemStaticText7, 0, wxALIGN_CENTER_VERTICAL|wxALL, 0);

    m_ID = new wxTextCtrl( itemCSrcModEditPanel1, ID_VOUCHER_TEXT, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer2->Add(m_ID, 0, wxALIGN_CENTER_VERTICAL|wxALL, 0);

////@end CVoucherPanel content construction
}


/*
 * Should we show tooltips?
 */

bool CVoucherPanel::ShowToolTips()
{
    return true;
}

/*
 * Get bitmap resources
 */

wxBitmap CVoucherPanel::GetBitmapResource( const wxString& name )
{
    // Bitmap retrieval
////@begin CVoucherPanel bitmap retrieval
    wxUnusedVar(name);
    return wxNullBitmap;
////@end CVoucherPanel bitmap retrieval
}

/*
 * Get icon resources
 */

wxIcon CVoucherPanel::GetIconResource( const wxString& name )
{
    // Icon retrieval
////@begin CVoucherPanel icon retrieval
    wxUnusedVar(name);
    return wxNullIcon;
////@end CVoucherPanel icon retrieval
}


string CVoucherPanel::GetValue()
{
    string inst = ToStdString(m_Inst->GetValue());
    string coll = ToStdString(m_Coll->GetValue());
    string id = ToStdString(m_ID->GetValue());

    if (NStr::IsBlank (inst) && NStr::IsBlank (coll) && NStr::IsBlank(id)) {
        return "";
    } else if (NStr::IsBlank (inst) && NStr::IsBlank(coll)) {
        return id;
    } else if (NStr::IsBlank (coll)) {
        return inst + ":" + id;
    } else {
        return inst + ":" + coll + ":" + id;
    }
}


void CVoucherPanel::SetValue(const string& val)
{
    string inst = "";
    string coll = "";
    string id = "";

    size_t pos = NStr::Find(val, ":");
    if (pos == string::npos) {
        id = val;
    } else {
        inst = val.substr(0, pos);
        string remainder = val.substr(pos + 1);
        size_t pos = NStr::Find (remainder, ":");
        if (pos == string::npos) {
            id = remainder;
        } else {
            coll = remainder.substr(0, pos);
            id = remainder.substr(pos + 1);
        }
    }
    m_Inst->SetValue(ToWxString(inst));
    m_Coll->SetValue(ToWxString(coll));
    m_ID->SetValue(ToWxString(id));
}


bool CVoucherPanel::IsWellFormatted(const string& val)
{
    return true;
}


/*
 * wxEVT_COMMAND_TEXT_UPDATED event handler for ID_VOUCHER_INST
 */

void CVoucherPanel::OnVoucherInstTextUpdated( wxCommandEvent& event )
{
    x_NotifyParent();
}


/*
 * wxEVT_COMMAND_TEXT_UPDATED event handler for ID_VOUCHER_COLL
 */

void CVoucherPanel::OnVoucherCollTextUpdated( wxCommandEvent& event )
{
    x_NotifyParent();
}


/*
 * wxEVT_COMMAND_TEXT_UPDATED event handler for ID_VOUCHER_TEXT
 */

void CVoucherPanel::OnVoucherTextTextUpdated( wxCommandEvent& event )
{
    x_NotifyParent();
}


END_NCBI_SCOPE


