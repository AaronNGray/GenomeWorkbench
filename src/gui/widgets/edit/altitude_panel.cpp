/*  $Id: altitude_panel.cpp 33304 2015-07-06 13:41:18Z asztalos $
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

#include <gui/widgets/edit/altitude_panel.hpp>
#include <gui/widgets/wx/wx_utils.hpp>

#include <wx/stattext.h>
#include <wx/sizer.h>

////@begin XPM images
////@end XPM images

BEGIN_NCBI_SCOPE

/*
 * CAltitudePanel type definition
 */

IMPLEMENT_DYNAMIC_CLASS( CAltitudePanel, CSrcModEditPanel )


/*
 * CAltitudePanel event table definition
 */

BEGIN_EVENT_TABLE( CAltitudePanel, CSrcModEditPanel )

////@begin CAltitudePanel event table entries
    EVT_TEXT( ID_ALTITUDE_TXT, CAltitudePanel::OnAltitudeTxtTextUpdated )

////@end CAltitudePanel event table entries

END_EVENT_TABLE()


/*
 * CAltitudePanel constructors
 */

CAltitudePanel::CAltitudePanel() : CSrcModEditPanel(CSrcModEditPanel::eAltitude)
{
    Init();
}

CAltitudePanel::CAltitudePanel( wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style )
: CSrcModEditPanel(CSrcModEditPanel::eAltitude)
{
    Init();
    Create(parent, id, pos, size, style);
}


/*
 * CAltitudePanel creator
 */

bool CAltitudePanel::Create( wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style )
{
////@begin CAltitudePanel creation
    CSrcModEditPanel::Create( parent, id, pos, size, style );

    CreateControls();
    if (GetSizer())
    {
        GetSizer()->SetSizeHints(this);
    }
    Centre();
////@end CAltitudePanel creation
    return true;
}


/*
 * CAltitudePanel destructor
 */

CAltitudePanel::~CAltitudePanel()
{
////@begin CAltitudePanel destruction
////@end CAltitudePanel destruction
}


/*
 * Member initialisation
 */

void CAltitudePanel::Init()
{
////@begin CAltitudePanel member initialisation
    m_Text = NULL;
////@end CAltitudePanel member initialisation
}


/*
 * Control creation for CAltitudePanel
 */

void CAltitudePanel::CreateControls()
{    
////@begin CAltitudePanel content construction
    CAltitudePanel* itemCSrcModEditPanel1 = this;

    wxBoxSizer* itemBoxSizer2 = new wxBoxSizer(wxHORIZONTAL);
    itemCSrcModEditPanel1->SetSizer(itemBoxSizer2);

    m_Text = new wxTextCtrl( itemCSrcModEditPanel1, ID_ALTITUDE_TXT, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer2->Add(m_Text, 0, wxALIGN_CENTER_VERTICAL|wxALL, 0);

    wxStaticText* itemStaticText4 = new wxStaticText( itemCSrcModEditPanel1, wxID_STATIC, _("meters"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer2->Add(itemStaticText4, 0, wxALIGN_CENTER_VERTICAL|wxALL, 0);

////@end CAltitudePanel content construction
}


/*
 * Should we show tooltips?
 */

bool CAltitudePanel::ShowToolTips()
{
    return true;
}

/*
 * Get bitmap resources
 */

wxBitmap CAltitudePanel::GetBitmapResource( const wxString& name )
{
    // Bitmap retrieval
////@begin CAltitudePanel bitmap retrieval
    wxUnusedVar(name);
    return wxNullBitmap;
////@end CAltitudePanel bitmap retrieval
}

/*
 * Get icon resources
 */

wxIcon CAltitudePanel::GetIconResource( const wxString& name )
{
    // Icon retrieval
////@begin CAltitudePanel icon retrieval
    wxUnusedVar(name);
    return wxNullIcon;
////@end CAltitudePanel icon retrieval
}


string CAltitudePanel::GetValue()
{
    string val = ToStdString(m_Text->GetValue());
    if (NStr::IsBlank(val)) {
        return val;
    }
    string check = val + " m.";
    string number = "";
    if (x_GetNumber(check, number)) {
        return check;
    } else if (x_GetNumber(val, number)) {
        check = number + " m.";
        return check;
    } else {
        return val;
    }
}


void CAltitudePanel::SetValue(const string& val)
{
    string number = "";
    if (x_GetNumber(val, number)) {
        m_Text->SetValue(number);
    } else {
        m_Text->SetValue(val);
    }

}


bool CAltitudePanel::IsWellFormatted(const string& val)
{
    string number = "";
    return x_GetNumber (val, number);
}


/*
 * wxEVT_COMMAND_TEXT_UPDATED event handler for ID_ALTITUDE_TXT
 */

void CAltitudePanel::OnAltitudeTxtTextUpdated( wxCommandEvent& event )
{
    x_NotifyParent();    
}


bool CAltitudePanel::x_GetNumber(const string& input, string& number)
{
    number = "";
    if (NStr::IsBlank(input)) {
        return true;
    }
    
    string cpy = input;
    size_t pos = 0;
    if (NStr::StartsWith(input, "-")) {
        cpy = input.substr(1);
        pos++;
    }
    bool any_digit = false;
    string::iterator sit = cpy.begin();
    while (sit != cpy.end() && isdigit(*sit)) {
        any_digit = true;
        ++sit;
        pos++;
    }

    if (sit != cpy.end() && *sit == '.') {
        ++sit;
        pos++;
        while (sit != cpy.end() && isdigit(*sit)) {
            any_digit = true;
            ++sit;
            pos++;
        }
    }

    if (!any_digit) {
        return false;
    }
    while (sit != cpy.end() && isspace(*sit)) {
        ++sit;
        pos++;
    }

    if (sit == cpy.end()) {
        return false;
    }
    string units = input.substr(pos);
    if (NStr::Equal(units, "m")
        || NStr::Equal(units, "m.")
        || NStr::Equal(units, "meters")
        || NStr::Equal(units, "metres")) {
        number = input.substr(0, pos - 1);
        return true;
    } else {
        return false;
    }
}


END_NCBI_SCOPE


