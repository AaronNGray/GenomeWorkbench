/*  $Id: wx_seqmarker_set_dlg.cpp 38968 2017-07-13 17:22:07Z katargir $
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
 *  and reliability of the software and data, the NLM and the U.S.
 *  Government do not and cannot warrant the performance or results that
 *  may be obtained by using this software or data. The NLM and the U.S.
 *  Government disclaim all warranties, express or implied, including
 *  warranties of performance, merchantability or fitness for any particular
 *  purpose.
 *
 *  Please cite the author in any work or product based on this material.
 *
 * ===========================================================================
 *
 * Authors:
 *
 * File Description:
 *
 */

#include <ncbi_pch.hpp>
#include <gui/widgets/wx/message_box.hpp>
#include <gui/widgets/wx/wx_utils.hpp>
#include <gui/widgets/wx/color_picker_validator.hpp>

#include <wx/bitmap.h>
#include <wx/icon.h>

////@begin includes
////@end includes

#include "wx_seqmarker_set_dlg.hpp"

////@begin XPM images
////@end XPM images

BEGIN_NCBI_SCOPE

IMPLEMENT_DYNAMIC_CLASS( CwxSeqMarkerSetDlg, wxDialog )

BEGIN_EVENT_TABLE( CwxSeqMarkerSetDlg, wxDialog )

////@begin CwxSeqMarkerSetDlg event table entries
    EVT_BUTTON( wxID_OK, CwxSeqMarkerSetDlg::OnOkClick )

////@end CwxSeqMarkerSetDlg event table entries

END_EVENT_TABLE()

CwxSeqMarkerSetDlg::CwxSeqMarkerSetDlg()
{
    Init();
}

CwxSeqMarkerSetDlg::CwxSeqMarkerSetDlg( wxWindow* parent, wxWindowID id, const wxString& caption, const wxPoint& pos, const wxSize& size, long style )
{
    Init();
    CreateX(parent, id, caption, pos, size, style);
}

bool CwxSeqMarkerSetDlg::CreateX( wxWindow* parent, wxWindowID id, const wxString& caption, const wxPoint& pos, const wxSize& size, long style )
{
////@begin CwxSeqMarkerSetDlg creation
    SetExtraStyle(wxWS_EX_BLOCK_EVENTS|wxDIALOG_EX_CONTEXTHELP);
    wxDialog::Create( parent, id, caption, pos, size, style );

    CreateControls();
    if (GetSizer())
    {
        GetSizer()->SetSizeHints(this);
    }
    Centre();
////@end CwxSeqMarkerSetDlg creation
    return true;
}

CwxSeqMarkerSetDlg::~CwxSeqMarkerSetDlg()
{
////@begin CwxSeqMarkerSetDlg destruction
////@end CwxSeqMarkerSetDlg destruction
}

void CwxSeqMarkerSetDlg::Init()
{
////@begin CwxSeqMarkerSetDlg member initialisation
    m_Pos = 0;
    m_ToPos = (unsigned int)-1;
    m_MarkerName = NULL;
    m_MarkerPos = NULL;
////@end CwxSeqMarkerSetDlg member initialisation
}

void CwxSeqMarkerSetDlg::CreateControls()
{
////@begin CwxSeqMarkerSetDlg content construction
    CwxSeqMarkerSetDlg* itemDialog1 = this;

    wxBoxSizer* itemBoxSizer2 = new wxBoxSizer(wxVERTICAL);
    itemDialog1->SetSizer(itemBoxSizer2);

    wxFlexGridSizer* itemFlexGridSizer3 = new wxFlexGridSizer(0, 2, 0, 0);
    itemBoxSizer2->Add(itemFlexGridSizer3, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

    wxStaticText* itemStaticText4 = new wxStaticText( itemDialog1, wxID_STATIC, _("Name:"), wxDefaultPosition, wxDefaultSize, 0 );
    itemFlexGridSizer3->Add(itemStaticText4, 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    m_MarkerName = new wxTextCtrl( itemDialog1, ID_TEXTCTRL4, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
    itemFlexGridSizer3->Add(m_MarkerName, 1, wxGROW|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxStaticText* itemStaticText6 = new wxStaticText( itemDialog1, wxID_STATIC, _("Position:"), wxDefaultPosition, wxDefaultSize, 0 );
    itemFlexGridSizer3->Add(itemStaticText6, 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    m_MarkerPos = new wxTextCtrl( itemDialog1, ID_TEXTCTRL1, wxEmptyString, wxDefaultPosition, wxSize(200, -1), 0 );
    itemFlexGridSizer3->Add(m_MarkerPos, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxStaticText* itemStaticText8 = new wxStaticText( itemDialog1, wxID_STATIC, _("Color:"), wxDefaultPosition, wxDefaultSize, 0 );
    itemFlexGridSizer3->Add(itemStaticText8, 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxColourPickerCtrl* itemColourPickerCtrl9 = new wxColourPickerCtrl( itemDialog1, ID_COLOURCTRL );
    itemFlexGridSizer3->Add(itemColourPickerCtrl9, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxStdDialogButtonSizer* itemStdDialogButtonSizer10 = new wxStdDialogButtonSizer;

    itemBoxSizer2->Add(itemStdDialogButtonSizer10, 0, wxALIGN_RIGHT|wxALL, 5);
    wxButton* itemButton11 = new wxButton( itemDialog1, wxID_OK, _("&OK"), wxDefaultPosition, wxDefaultSize, 0 );
    itemButton11->SetDefault();
    itemStdDialogButtonSizer10->AddButton(itemButton11);

    wxButton* itemButton12 = new wxButton( itemDialog1, wxID_CANCEL, _("&Cancel"), wxDefaultPosition, wxDefaultSize, 0 );
    itemStdDialogButtonSizer10->AddButton(itemButton12);

    itemStdDialogButtonSizer10->Realize();

    // Set validators
    itemColourPickerCtrl9->SetValidator( CColorPickerValidator(& m_Color) );
////@end CwxSeqMarkerSetDlg content construction
    string value;
    NStr::IntToString(value, m_Pos);
    m_MarkerPos->SetValue(ToWxString(value));
}


void CwxSeqMarkerSetDlg::SetDlgTitle(const string& title)
{
    SetTitle(ToWxString(title));
}


void CwxSeqMarkerSetDlg::SetMarkerName(const string& name)
{
    m_MarkerName->SetValue(ToWxString(name));
}

void CwxSeqMarkerSetDlg::SetMarkerPos(TSeqPos pos)
{
    m_Pos = pos + 1;
    m_ToPos = (unsigned int)-1;
    m_MarkerPos->SetValue(ToWxString(NStr::NumericToString(m_Pos)));
}

void CwxSeqMarkerSetDlg::SetMarkerRange(TSeqRange range)
{
    m_Pos = range.GetFrom() + 1;
    m_ToPos = range.GetToOpen();
    m_MarkerPos->SetValue(ToWxString(NStr::NumericToString(m_Pos)) + ToWxString(":") +
                          ToWxString(NStr::NumericToString(m_ToPos)));
}

void CwxSeqMarkerSetDlg::SetMarkerColor(const CRgbaColor& color)
{
    m_Color = color;
}


string CwxSeqMarkerSetDlg::GetMarkerName() const
{
    return ToStdString(m_MarkerName->GetValue());
}


TSeqPos CwxSeqMarkerSetDlg::GetMarkerPos() const
{
    return m_Pos - 1;
}

bool CwxSeqMarkerSetDlg::GetIsRange() const
{
    return (m_ToPos != (unsigned int)-1);
}

TSeqRange CwxSeqMarkerSetDlg::GetMarkerRange() const
{
    TSeqRange r(m_Pos-1, m_ToPos-1);
    return r;
}

const CRgbaColor CwxSeqMarkerSetDlg::GetMarkerColor() const
{
    return m_Color;
}

void CwxSeqMarkerSetDlg::OnOkClick( wxCommandEvent& event )
{
    string orig = ToStdString(m_MarkerPos->GetValue());
    string goto_from, goto_to;

    bool k_from = false, m_from = false;
    bool k_to = false, m_to = false;
    bool pos_e = false;
    bool bad_input = false;

    ITERATE (string, it, orig) {
        char ch = *it;
        switch(ch) {
            case '0':  case '1':  case '2':  case '3':
            case '4':  case '5':  case '6':  case '7':
            case '8':  case '9':
                pos_e ? goto_to += ch : goto_from += ch;
                break;
            case 'k':  case 'K':
                pos_e ? k_to = true : k_from = true;
                break;
            case 'm':  case 'M':
                pos_e ? m_to = true : m_from = true;
                break;
            case ':':  case '-': case '.':
                pos_e = true;
                break;
            case ',':
            case ' ':
                break;
            default:
                bad_input = true;
                break;
        }
    }

    if (bad_input || goto_from=="") {
        NcbiErrorBox("Not a valid sequence position", "Set Marker To Position");
        return;  // invalid input
    }

    // add thousands (or millions)
    if (k_from) goto_from += "000";
    else if (m_from) goto_from += "000000";

    if (k_to) goto_to += "000";
    else if (m_to) goto_to += "000000";

    try {
        m_Pos = NStr::StringToInt(goto_from);
        if (m_Pos < 1) m_Pos = 1;

        if (goto_to != "") {
            m_ToPos =  NStr::StringToInt(goto_to);
            if (m_ToPos < 1) m_ToPos = 1;
        }
        else {
            m_ToPos = (unsigned int)-1; // single position marker
        }

        event.Skip();
    }
    catch (const CStringException&) {
        NcbiErrorBox("Not a valid sequence position", "Set Marker To Position");
        return;  // invalid input
    }
}


bool CwxSeqMarkerSetDlg::ShowToolTips()
{
    return true;
}


wxBitmap CwxSeqMarkerSetDlg::GetBitmapResource( const wxString& name )
{
    // Bitmap retrieval
////@begin CwxSeqMarkerSetDlg bitmap retrieval
    wxUnusedVar(name);
    return wxNullBitmap;
////@end CwxSeqMarkerSetDlg bitmap retrieval
}
wxIcon CwxSeqMarkerSetDlg::GetIconResource( const wxString& name )
{
    // Icon retrieval
////@begin CwxSeqMarkerSetDlg icon retrieval
    wxUnusedVar(name);
    return wxNullIcon;
////@end CwxSeqMarkerSetDlg icon retrieval
}


END_NCBI_SCOPE


