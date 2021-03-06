/*  $Id: twopartqual_panel.cpp 38073 2017-03-23 16:34:10Z filippov $
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
#include <gui/widgets/wx/wx_utils.hpp>
////@begin includes
////@end includes

#include "twopartqual_panel.hpp"

////@begin XPM images
////@end XPM images

BEGIN_NCBI_SCOPE

/*!
 * CTwoPartQualPanel type definition
 */

IMPLEMENT_DYNAMIC_CLASS( CTwoPartQualPanel, CFormattedQualPanel )


/*!
 * CTwoPartQualPanel event table definition
 */

BEGIN_EVENT_TABLE( CTwoPartQualPanel, CFormattedQualPanel )

////@begin CTwoPartQualPanel event table entries
    EVT_TEXT( ID_TEXTCTRL28, CTwoPartQualPanel::OnTextctrl28TextUpdated )

////@end CTwoPartQualPanel event table entries

END_EVENT_TABLE()


/*!
 * CTwoPartQualPanel constructors
 */

CTwoPartQualPanel::CTwoPartQualPanel()
{
    Init();
}

CTwoPartQualPanel::CTwoPartQualPanel( wxWindow* parent, wxWindowID id, const wxString& caption, const wxPoint& pos, const wxSize& size, long style )
{
    Init();
    Create(parent, id, caption, pos, size, style);
}


/*!
 * CTwoPartQualPanel creator
 */

bool CTwoPartQualPanel::Create( wxWindow* parent, wxWindowID id, const wxString& caption, const wxPoint& pos, const wxSize& size, long style )
{
////@begin CTwoPartQualPanel creation
    SetExtraStyle(wxWS_EX_BLOCK_EVENTS);
    CFormattedQualPanel::Create( parent, id, pos, size, style );

    CreateControls();
    if (GetSizer())
    {
        GetSizer()->SetSizeHints(this);
    }
    Centre();
////@end CTwoPartQualPanel creation
    return true;
}


/*!
 * CTwoPartQualPanel destructor
 */

CTwoPartQualPanel::~CTwoPartQualPanel()
{
////@begin CTwoPartQualPanel destruction
////@end CTwoPartQualPanel destruction
}


/*!
 * Member initialisation
 */

void CTwoPartQualPanel::Init()
{
////@begin CTwoPartQualPanel member initialisation
    m_ControlledListCtrl = NULL;
    m_FreeTextCtrl = NULL;
////@end CTwoPartQualPanel member initialisation
}


/*!
 * Control creation for CTwoPartQualPanel
 */

void CTwoPartQualPanel::CreateControls()
{    
////@begin CTwoPartQualPanel content construction
    // Generated by DialogBlocks, 15/12/2011 07:59:08 (unregistered)

    CTwoPartQualPanel* itemCFormattedQualPanel1 = this;

    wxBoxSizer* itemBoxSizer2 = new wxBoxSizer(wxVERTICAL);
    itemCFormattedQualPanel1->SetSizer(itemBoxSizer2);

    wxBoxSizer* itemBoxSizer3 = new wxBoxSizer(wxHORIZONTAL);
    itemBoxSizer2->Add(itemBoxSizer3, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 0);

    wxArrayString m_ControlledListCtrlStrings;
    m_ControlledListCtrl = new wxChoice( itemCFormattedQualPanel1, ID_CHOICE12, wxDefaultPosition, wxDefaultSize, m_ControlledListCtrlStrings, 0 );
    itemBoxSizer3->Add(m_ControlledListCtrl, 0, wxALIGN_CENTER_VERTICAL|wxALL, 0);

    m_FreeTextCtrl = new wxTextCtrl( itemCFormattedQualPanel1, ID_TEXTCTRL28, wxEmptyString, wxDefaultPosition, wxSize(150, -1), 0 );
    itemBoxSizer3->Add(m_FreeTextCtrl, 0, wxALIGN_CENTER_VERTICAL|wxALL, 0);

////@end CTwoPartQualPanel content construction
}


void CTwoPartQualPanel::SetValue(string val)
{
    if (NStr::IsBlank (val)) {
        m_ControlledListCtrl->SetStringSelection(wxEmptyString);
        m_FreeTextCtrl->SetValue (wxEmptyString);
    } else {
        int pos = NStr::Find(val, ":");
        if (pos == string::npos) {
            if (m_ControlledListCtrl->SetStringSelection(ToWxString(val))) {
                m_FreeTextCtrl->SetValue (wxEmptyString);
            } else {
                m_ControlledListCtrl->SetStringSelection(wxEmptyString);
                m_FreeTextCtrl->SetValue(ToWxString(val));
            } 
        } else {
            string part1 = val.substr(0, pos);
            string part2 = val.substr(pos + 1);
            if (m_ControlledListCtrl->SetStringSelection(ToWxString(part1))) {
                m_FreeTextCtrl->SetValue(ToWxString(part2));
            } else {
                m_ControlledListCtrl->SetStringSelection(wxEmptyString);
                m_FreeTextCtrl->SetValue(ToWxString(val));
            }
        }
    }
}


string CTwoPartQualPanel::GetValue()
{
    string part1 = ToStdString (m_ControlledListCtrl->GetStringSelection());
    string part2 = ToStdString (m_FreeTextCtrl->GetValue());

    if (NStr::IsBlank (part1)) {
        return part2;
    } else if (NStr::IsBlank (part2)) {
        return part1;
    } else {
      string val = part1 + ":" + part2;
      return val;
    }
}


void CTwoPartQualPanel::GetDimensions(int *width, int *height)
{
	int x1, x2, y1, y2;
	m_ControlledListCtrl->GetSize(&x1, &y1);
	m_FreeTextCtrl->GetSize(&x2, &y2);
	if (height != NULL) {
		if (y1 > y2) {
			*height = y1;
		} else {
			*height = y2;
		}
	}

	if (width != NULL) {
		*width = x1 + x2;
	}
}


void CTwoPartQualPanel::SetControlledList (vector<string> choices)
{
    m_ControlledListCtrl->Clear();
    m_ControlledListCtrl->Append(wxEmptyString);
    ITERATE (vector<string>, it, choices) {
        m_ControlledListCtrl->Append(ToWxString (*it));
    }
}


/*!
 * Should we show tooltips?
 */

bool CTwoPartQualPanel::ShowToolTips()
{
    return true;
}

/*!
 * Get bitmap resources
 */

wxBitmap CTwoPartQualPanel::GetBitmapResource( const wxString& name )
{
    // Bitmap retrieval
////@begin CTwoPartQualPanel bitmap retrieval
    wxUnusedVar(name);
    return wxNullBitmap;
////@end CTwoPartQualPanel bitmap retrieval
}

/*!
 * Get icon resources
 */

wxIcon CTwoPartQualPanel::GetIconResource( const wxString& name )
{
    // Icon retrieval
////@begin CTwoPartQualPanel icon retrieval
    wxUnusedVar(name);
    return wxNullIcon;
////@end CTwoPartQualPanel icon retrieval
}


/*!
 * wxEVT_COMMAND_TEXT_UPDATED event handler for ID_TEXTCTRL28
 */

void CTwoPartQualPanel::OnTextctrl28TextUpdated( wxCommandEvent& event )
{
    wxTextCtrl* item = (wxTextCtrl*)event.GetEventObject();
    if (NStr::IsBlank(ToStdString(item->GetLabel()))) {
        return;
    }
	  NotifyParentOfChange();
}

END_NCBI_SCOPE

