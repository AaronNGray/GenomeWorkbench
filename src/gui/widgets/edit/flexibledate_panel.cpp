/*  $Id: flexibledate_panel.cpp 43219 2019-05-29 19:39:12Z filippov $
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
 * Authors:  Roman Katargin
 */


#include <ncbi_pch.hpp>
#include <objects/general/Date.hpp>
#include <objects/general/Date_std.hpp>
#include <gui/widgets/wx/number_validator.hpp>
#include <gui/widgets/wx/wx_utils.hpp>

#include <gui/widgets/edit/serial_member_primitive_validators.hpp>

////@begin includes
////@end includes
////@begin includes
////@end includes

#include <gui/widgets/edit/flexibledate_panel.hpp>

#include <wx/sizer.h>
#include <wx/bitmap.h>
#include <wx/icon.h>
#include <wx/stattext.h>
#include <wx/textctrl.h>
#include <wx/choice.h>

BEGIN_NCBI_SCOPE
USING_SCOPE(objects);

////@begin XPM images
////@end XPM images


/*
 * CFlexibleDatePanel type definition
 */

IMPLEMENT_DYNAMIC_CLASS( CFlexibleDatePanel, wxPanel )


/*
 * CFlexibleDatePanel event table definition
 */

BEGIN_EVENT_TABLE( CFlexibleDatePanel, wxPanel )

////@begin CFlexibleDatePanel event table entries
////@end CFlexibleDatePanel event table entries

END_EVENT_TABLE()


/*
 * CFlexibleDatePanel constructors
 */

CFlexibleDatePanel::CFlexibleDatePanel()
{
    Init();
}

CFlexibleDatePanel::CFlexibleDatePanel( wxWindow* parent, CRef<CDate> the_date, long min_year, long range,
                                        wxWindowID id, const wxPoint& pos, const wxSize& size, long style ) :
    m_Date(the_date), m_MinYear(min_year), m_YearRange(range)
{
    Init();
    Create(parent, id, pos, size, style);
}



/*
 * CFlexibleDatePanel creator
 */

bool CFlexibleDatePanel::Create( wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style )
{
////@begin CFlexibleDatePanel creation
    SetExtraStyle(wxWS_EX_BLOCK_EVENTS);
    wxPanel::Create( parent, id, pos, size, style );

    CreateControls();
    if (GetSizer())
    {
        GetSizer()->SetSizeHints(this);
    }
    Centre();
////@end CFlexibleDatePanel creation
    return true;
}


/*
 * CFlexibleDatePanel destructor
 */

CFlexibleDatePanel::~CFlexibleDatePanel()
{
////@begin CFlexibleDatePanel destruction
////@end CFlexibleDatePanel destruction
}


/*
 * Member initialisation
 */

void CFlexibleDatePanel::Init()
{
////@begin CFlexibleDatePanel member initialisation
    m_YearCtrl = NULL;
    m_MonthCtrl = NULL;
    m_DayCtrl = NULL;
////@end CFlexibleDatePanel member initialisation
}


/*
 * Control creation for CFlexibleDatePanel
 */

void CFlexibleDatePanel::CreateControls()
{    
////@begin CFlexibleDatePanel content construction
    CFlexibleDatePanel* itemPanel1 = this;

    wxFlexGridSizer* itemFlexGridSizer2 = new wxFlexGridSizer(0, 3, 0, 0);
    itemPanel1->SetSizer(itemFlexGridSizer2);

    wxStaticText* itemStaticText3 = new wxStaticText( itemPanel1, wxID_STATIC, _("Year"), wxDefaultPosition, wxDefaultSize, 0 );
    itemFlexGridSizer2->Add(itemStaticText3, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxStaticText* itemStaticText4 = new wxStaticText( itemPanel1, wxID_STATIC, _("Month"), wxDefaultPosition, wxDefaultSize, 0 );
    itemFlexGridSizer2->Add(itemStaticText4, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxStaticText* itemStaticText5 = new wxStaticText( itemPanel1, wxID_STATIC, _("Day"), wxDefaultPosition, wxDefaultSize, 0 );
    itemFlexGridSizer2->Add(itemStaticText5, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    m_YearCtrl = new wxTextCtrl( itemPanel1, ID_FLEXIBLEDATE_YEAR, wxEmptyString, wxDefaultPosition, wxSize(60, -1), 0 );
    itemFlexGridSizer2->Add(m_YearCtrl, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxArrayString m_MonthCtrlStrings;
    m_MonthCtrlStrings.Add(wxEmptyString);
    m_MonthCtrlStrings.Add(_("Jan"));
    m_MonthCtrlStrings.Add(_("Feb"));
    m_MonthCtrlStrings.Add(_("Mar"));
    m_MonthCtrlStrings.Add(_("Apr"));
    m_MonthCtrlStrings.Add(_("May"));
    m_MonthCtrlStrings.Add(_("Jun"));
    m_MonthCtrlStrings.Add(_("Jul"));
    m_MonthCtrlStrings.Add(_("Aug"));
    m_MonthCtrlStrings.Add(_("Sep"));
    m_MonthCtrlStrings.Add(_("Oct"));
    m_MonthCtrlStrings.Add(_("Nov"));
    m_MonthCtrlStrings.Add(_("Dec"));
    m_MonthCtrl = new wxChoice( itemPanel1, ID_FLEXIBLEDATE_MONTH, wxDefaultPosition, wxDefaultSize, m_MonthCtrlStrings, 0 );
    itemFlexGridSizer2->Add(m_MonthCtrl, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    m_DayCtrl = new wxTextCtrl( itemPanel1, ID_FLEXIBLEDATE_DAY, wxEmptyString, wxDefaultPosition, wxSize(40, -1), 0 );
    itemFlexGridSizer2->Add(m_DayCtrl, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxALL, 5);

////@end CFlexibleDatePanel content construction

    m_Year = 0;
    m_Day = 0;
    m_Month = 0;
    if (m_Date->SetStd().IsSetYear()) {
        m_Year = m_Date->GetStd().GetYear();
    }
    if (m_Date->SetStd().IsSetMonth()) {
        m_Month = m_Date->GetStd().GetMonth();
        if (m_Month > 12 || m_Month < 0) {
            m_Month = 0;
        }
    }
    if (m_Date->SetStd().IsSetDay()) {
        m_Day = m_Date->GetStd().GetDay();
    }
    m_MonthCtrl->SetSelection(m_Month);
    m_DayCtrl->SetValidator(CNumberOrBlankValidator(&m_Day));
    m_YearCtrl->SetValidator(CNumberOrBlankValidator(&m_Year, m_MinYear, m_MinYear + m_YearRange));
}

void CFlexibleDatePanel::SetToolTip(const wxString &tipString)
{
	m_YearCtrl->SetToolTip(tipString);
	m_MonthCtrl->SetToolTip(tipString);
	m_DayCtrl->SetToolTip(tipString);	
	wxPanel::SetToolTip(tipString);
}

bool CFlexibleDatePanel::TransferDataToWindow()
{
    m_Year = 0;
    m_Day = 0;
    m_Month = 0;
    if (m_Date->SetStd().IsSetYear()) {
        m_Year = m_Date->GetStd().GetYear();
    }
    if (m_Date->SetStd().IsSetMonth()) {
        m_Month = m_Date->GetStd().GetMonth();
        if (m_Month > 12 || m_Month < 0) {
            m_Month = 0;
        }
    }
    if (m_Date->SetStd().IsSetDay()) {
        m_Day = m_Date->GetStd().GetDay();
    }
    if (!wxPanel::TransferDataToWindow())
        return false;

    m_MonthCtrl->SetSelection(m_Month);
    return true;
}


bool CFlexibleDatePanel::TransferDataFromWindow()
{
    if (!wxPanel::TransferDataFromWindow())
        return false;

    if (m_Year < m_MinYear) {
        m_Date->SetStr("?");
        return true;
    }

    m_Month = m_MonthCtrl->GetSelection();

    m_Date->SetStd().SetYear(m_Year);
    if (m_Month > 0) {
        m_Date->SetStd().SetMonth(m_Month);
        if (m_Day > 0) {
            m_Date->SetStd().SetDay(m_Day);
        } else {
            m_Date->SetStd().ResetDay();
        }
    } else {
        m_Date->SetStd().ResetMonth();
        m_Date->SetStd().ResetDay();
    }
    return true;
}


static bool s_IsDateEmpty (CRef<CDate> a_date) 
{
    if (!a_date || !a_date->IsStd() || !a_date->GetStd().IsSetYear() || !a_date->GetStd().IsSetMonth() || !a_date->GetStd().IsSetDay()) {
        return true;
    } else {
        return false;
    }
}


CRef<CDate> CFlexibleDatePanel::GetDate() const
{
    if (s_IsDateEmpty (m_Date)) {
        CRef<CDate> no_date;
        return no_date;
    }

    CRef<CDate> a_date(new CDate());
    a_date->Assign(*m_Date);

    return a_date;
}

void CFlexibleDatePanel::SetDate( const CDate& a_date ) 
{
    m_Date->Assign(a_date);

    TransferDataToWindow();
}


void CFlexibleDatePanel::Clear()
{
    m_Date->Reset();
    TransferDataToWindow();
}


/*
 * Should we show tooltips?
 */

bool CFlexibleDatePanel::ShowToolTips()
{
    return true;
}

/*
 * Get bitmap resources
 */

wxBitmap CFlexibleDatePanel::GetBitmapResource( const wxString& name )
{
    // Bitmap retrieval
////@begin CFlexibleDatePanel bitmap retrieval
    wxUnusedVar(name);
    return wxNullBitmap;
////@end CFlexibleDatePanel bitmap retrieval
}

/*
 * Get icon resources
 */

wxIcon CFlexibleDatePanel::GetIconResource( const wxString& name )
{
    // Icon retrieval
////@begin CFlexibleDatePanel icon retrieval
    wxUnusedVar(name);
    return wxNullIcon;
////@end CFlexibleDatePanel icon retrieval
}


void CFlexibleDatePanel::SetDate(const string &date)
{
    vector<string> dmy;
    NStr::Split(date, "-", dmy);    
    m_Year = 0;
    m_Day = 0;
    m_Month = 0;
    if (dmy.size() == 1)
    {
        m_Year = NStr::StringToInt(dmy[0]);
    }
    else if (dmy.size() == 2)
    {
        m_Month = m_MonthCtrl->FindString(wxString(dmy[0]), false);
        m_Year = NStr::StringToInt(dmy[1]);
    }
    else if (dmy.size() == 3)
    {
        m_Day = NStr::StringToInt(dmy[0]);
        m_Month = m_MonthCtrl->FindString(wxString(dmy[1]), false);
        m_Year = NStr::StringToInt(dmy[2]);
    }
    wxPanel::TransferDataToWindow();
    m_MonthCtrl->SetSelection(m_Month);
    TransferDataFromWindow(); // This populates m_Date
}

END_NCBI_SCOPE
