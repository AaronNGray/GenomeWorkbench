/*  $Id: latlon_panel.cpp 29060 2013-10-01 19:21:17Z bollin $
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

#include <objects/seqfeat/SubSource.hpp>
#include <gui/widgets/edit/latlon_panel.hpp>
#include <gui/widgets/wx/wx_utils.hpp>
#include <wx/sizer.h>

////@begin XPM images
////@end XPM images

BEGIN_NCBI_SCOPE

/*
 * CLatLonPanel type definition
 */

IMPLEMENT_DYNAMIC_CLASS( CLatLonPanel, CSrcModEditPanel )


/*
 * CLatLonPanel event table definition
 */

BEGIN_EVENT_TABLE( CLatLonPanel, CSrcModEditPanel )

////@begin CLatLonPanel event table entries
    EVT_TEXT( ID_LAT_TXT, CLatLonPanel::OnLatTxtTextUpdated )

    EVT_TEXT( ID_LON_TXT, CLatLonPanel::OnLonTxtTextUpdated )

////@end CLatLonPanel event table entries

END_EVENT_TABLE()


/*
 * CLatLonPanel constructors
 */

CLatLonPanel::CLatLonPanel() : CSrcModEditPanel(CSrcModEditPanel::eLatLon)
{
    Init();
}

CLatLonPanel::CLatLonPanel( wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style )
    : CSrcModEditPanel(CSrcModEditPanel::eLatLon)
{
    Init();
    Create(parent, id, pos, size, style);
}


/*
 * CLatLonPanel creator
 */

bool CLatLonPanel::Create( wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style )
{
////@begin CLatLonPanel creation
    CSrcModEditPanel::Create( parent, id, pos, size, style );

    CreateControls();
    if (GetSizer())
    {
        GetSizer()->SetSizeHints(this);
    }
    Centre();
////@end CLatLonPanel creation
    return true;
}


/*
 * CLatLonPanel destructor
 */

CLatLonPanel::~CLatLonPanel()
{
////@begin CLatLonPanel destruction
////@end CLatLonPanel destruction
}


/*
 * Member initialisation
 */

void CLatLonPanel::Init()
{
////@begin CLatLonPanel member initialisation
    m_Lat = NULL;
    m_NS = NULL;
    m_Lon = NULL;
    m_EW = NULL;
////@end CLatLonPanel member initialisation
}


/*
 * Control creation for CLatLonPanel
 */

void CLatLonPanel::CreateControls()
{    
////@begin CLatLonPanel content construction
    CLatLonPanel* itemCSrcModEditPanel1 = this;

    wxBoxSizer* itemBoxSizer2 = new wxBoxSizer(wxHORIZONTAL);
    itemCSrcModEditPanel1->SetSizer(itemBoxSizer2);

    m_Lat = new wxTextCtrl( itemCSrcModEditPanel1, ID_LAT_TXT, wxEmptyString, wxDefaultPosition, wxSize(50, -1), 0 );
    itemBoxSizer2->Add(m_Lat, 0, wxALIGN_CENTER_VERTICAL|wxALL, 0);

    wxArrayString m_NSStrings;
    m_NSStrings.Add(_("N"));
    m_NSStrings.Add(_("S"));
    m_NS = new wxChoice( itemCSrcModEditPanel1, ID_CHOICE, wxDefaultPosition, wxDefaultSize, m_NSStrings, 0 );
    m_NS->SetStringSelection(_("N"));
    itemBoxSizer2->Add(m_NS, 0, wxALIGN_CENTER_VERTICAL|wxALL, 0);

    m_Lon = new wxTextCtrl( itemCSrcModEditPanel1, ID_LON_TXT, wxEmptyString, wxDefaultPosition, wxSize(50, -1), 0 );
    itemBoxSizer2->Add(m_Lon, 0, wxALIGN_CENTER_VERTICAL|wxALL, 0);

    wxArrayString m_EWStrings;
    m_EWStrings.Add(_("E"));
    m_EWStrings.Add(_("W"));
    m_EW = new wxChoice( itemCSrcModEditPanel1, ID_CHOICE1, wxDefaultPosition, wxDefaultSize, m_EWStrings, 0 );
    m_EW->SetStringSelection(_("E"));
    itemBoxSizer2->Add(m_EW, 0, wxALIGN_CENTER_VERTICAL|wxALL, 0);

////@end CLatLonPanel content construction
}


/*
 * Should we show tooltips?
 */

bool CLatLonPanel::ShowToolTips()
{
    return true;
}

/*
 * Get bitmap resources
 */

wxBitmap CLatLonPanel::GetBitmapResource( const wxString& name )
{
    // Bitmap retrieval
////@begin CLatLonPanel bitmap retrieval
    wxUnusedVar(name);
    return wxNullBitmap;
////@end CLatLonPanel bitmap retrieval
}

/*
 * Get icon resources
 */

wxIcon CLatLonPanel::GetIconResource( const wxString& name )
{
    // Icon retrieval
////@begin CLatLonPanel icon retrieval
    wxUnusedVar(name);
    return wxNullIcon;
////@end CLatLonPanel icon retrieval
}


/*
 * wxEVT_COMMAND_TEXT_UPDATED event handler for ID_LAT_TXT
 */

void CLatLonPanel::OnLatTxtTextUpdated( wxCommandEvent& event )
{
    x_NotifyParent();
}


/*
 * wxEVT_COMMAND_TEXT_UPDATED event handler for ID_LON_TXT
 */

void CLatLonPanel::OnLonTxtTextUpdated( wxCommandEvent& event )
{
    x_NotifyParent();
}


string CLatLonPanel::GetValue()
{
    string lat = ToStdString(m_Lat->GetValue());
    string lon = ToStdString(m_Lon->GetValue());
    if (NStr::IsBlank(lat) && NStr::IsBlank(lon)) {
        return "";
    }
    
    string ns = ToStdString(m_NS->GetStringSelection());
    string ew = ToStdString(m_EW->GetStringSelection());
    
    string val = lat + " " + ns + " " + lon + " " + ew;
    
    return val;
}


void CLatLonPanel::SetValue(const string& val)
{
    if (NStr::IsBlank(val)) {
        m_Lat->SetValue(wxEmptyString);
        m_Lon->SetValue(wxEmptyString);
        m_NS->SetStringSelection(wxT("N"));
        m_EW->SetStringSelection(wxT("E"));
        return;
    }
    bool format_correct = false;
    bool precision_correct = false;
    bool lat_in_range = false;
    bool lon_in_range = false;
    double lat_value = 0.0;
    double lon_value = 0.0;

    objects::CSubSource::IsCorrectLatLonFormat (val, format_correct, precision_correct,
                                     lat_in_range, lon_in_range,
                                     lat_value, lon_value);

    if (lat_value < 0.0) {
        m_NS->SetStringSelection(wxT("S"));
        lat_value = 0.0 - lat_value;
    } else {
        m_NS->SetStringSelection(wxT("N"));
    }
    string number = "";
    NStr::NumericToString(number, lat_value);
    m_Lat->SetValue(ToWxString(number));

    if (lon_value < 0.0) {
        m_EW->SetStringSelection(wxT("W"));
        lon_value = 0.0 - lon_value;
    } else {
        m_EW->SetStringSelection(wxT("E"));
    }
    number = "";
    NStr::NumericToString(number, lon_value);
    m_Lon->SetValue(ToWxString(number));
}


bool CLatLonPanel::IsWellFormatted(const string& val)
{
    if (NStr::IsBlank(val)) {
        return true;
    }
    bool format_correct = false;
    bool precision_correct = false;
    bool lat_in_range = false;
    bool lon_in_range = false;
    double lat_value = 0.0;
    double lon_value = 0.0;

    objects::CSubSource::IsCorrectLatLonFormat (val, format_correct, precision_correct,
                                     lat_in_range, lon_in_range,
                                     lat_value, lon_value);

    return format_correct;
}



END_NCBI_SCOPE

