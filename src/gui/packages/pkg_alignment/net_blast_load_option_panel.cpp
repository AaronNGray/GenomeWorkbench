/*  $Id: net_blast_load_option_panel.cpp 39613 2017-10-13 21:57:04Z katargir $
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

#include <gui/packages/pkg_alignment/net_blast_load_option_panel.hpp>
#include <gui/widgets/wx/wx_utils.hpp>

#include <wx/sizer.h>
#include <wx/image.h>
#include <wx/icon.h>
#include <wx/stattext.h>
#include <wx/statbox.h>
#include <wx/menu.h>

////@begin XPM images
////@end XPM images

BEGIN_NCBI_SCOPE

IMPLEMENT_DYNAMIC_CLASS( CNetBLASTLoadOptionPanel, wxPanel )

BEGIN_EVENT_TABLE( CNetBLASTLoadOptionPanel, wxPanel )
////@begin CNetBLASTLoadOptionPanel event table entries
////@end CNetBLASTLoadOptionPanel event table entries
END_EVENT_TABLE()


CNetBLASTLoadOptionPanel::CNetBLASTLoadOptionPanel()
{
    Init();
}


CNetBLASTLoadOptionPanel::CNetBLASTLoadOptionPanel( wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style )
{
    Init();
    Create(parent, id, pos, size, style);
}


bool CNetBLASTLoadOptionPanel::Create( wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style )
{
////@begin CNetBLASTLoadOptionPanel creation
    wxPanel::Create( parent, id, pos, size, style );

    CreateControls();
    if (GetSizer())
    {
        GetSizer()->SetSizeHints(this);
    }
    Centre();
////@end CNetBLASTLoadOptionPanel creation
    return true;
}


CNetBLASTLoadOptionPanel::~CNetBLASTLoadOptionPanel()
{
////@begin CNetBLASTLoadOptionPanel destruction
////@end CNetBLASTLoadOptionPanel destruction
}


void CNetBLASTLoadOptionPanel::Init()
{
////@begin CNetBLASTLoadOptionPanel member initialisation
////@end CNetBLASTLoadOptionPanel member initialisation
}

namespace
{
    class CTextCtrl : public wxTextCtrl
    {
    public:
        CTextCtrl(wxWindow* parent, wxWindowID id, const wxString& value = "",
            const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize,
            long style = 0, const wxValidator& validator = wxDefaultValidator,
            const wxString& name = wxTextCtrlNameStr)
            : wxTextCtrl(parent, id, value, pos, size, style, validator, name) {}
    protected:
        void OnContextMenu(wxContextMenuEvent& event);

        DECLARE_EVENT_TABLE()
    };
}

BEGIN_EVENT_TABLE(CTextCtrl, wxTextCtrl)
    EVT_CONTEXT_MENU(CTextCtrl::OnContextMenu)
END_EVENT_TABLE()

void CTextCtrl::OnContextMenu(wxContextMenuEvent& event)
{
    wxMenu* menu = new wxMenu;
    menu->Append(wxID_CUT, _("Cu&t"));
    menu->Append(wxID_COPY, _("&Copy"));
    menu->Append(wxID_PASTE, _("&Paste"));
    menu->Append(wxID_CLEAR, _("&Delete"));
    menu->AppendSeparator();
    menu->Append(wxID_SELECTALL, _("Select &All"));
    PopupMenu(menu);
}

void CNetBLASTLoadOptionPanel::CreateControls()
{
////@begin CNetBLASTLoadOptionPanel content construction
    CNetBLASTLoadOptionPanel* itemPanel1 = this;

    wxBoxSizer* itemBoxSizer2 = new wxBoxSizer(wxVERTICAL);
    itemPanel1->SetSizer(itemBoxSizer2);

    wxStaticBox* itemStaticBoxSizer3Static = new wxStaticBox(itemPanel1, wxID_ANY, _("BLAST RIDs to load"));
    wxStaticBoxSizer* itemStaticBoxSizer3 = new wxStaticBoxSizer(itemStaticBoxSizer3Static, wxVERTICAL);
    itemBoxSizer2->Add(itemStaticBoxSizer3, 1, wxGROW|wxALL, 5);

    wxStaticText* itemStaticText4 = new wxStaticText( itemPanel1, wxID_STATIC, _("Separate BLAST RIDs with spaces, commas or semicolons"), wxDefaultPosition, wxDefaultSize, 0 );
    itemStaticBoxSizer3->Add(itemStaticText4, 0, wxGROW|wxALL, 5);

    CTextCtrl* itemTextCtrl5 = new CTextCtrl( itemPanel1, ID_TEXTCTRL1, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_MULTILINE );
    itemStaticBoxSizer3->Add(itemTextCtrl5, 1, wxGROW|wxALL, 5);

////@end CNetBLASTLoadOptionPanel content construction

    wxTextCtrl* ridInput = dynamic_cast<wxTextCtrl*>(FindWindow(ID_TEXTCTRL1));
    _ASSERT(ridInput);

    wxAcceleratorEntry entries[6];

    entries[0].Set(wxACCEL_CMD, (int) 'C', wxID_COPY);
    entries[1].Set(wxACCEL_CMD, (int) 'X', wxID_CUT);
    entries[2].Set(wxACCEL_CMD, (int) 'V', wxID_PASTE);
    entries[3].Set(wxACCEL_CMD, (int) 'A', wxID_SELECTALL);
    entries[4].Set(wxACCEL_CMD, 384, wxID_COPY);
    entries[5].Set(wxACCEL_SHIFT, 384, wxID_PASTE);

    wxAcceleratorTable accel(6, entries);
    ridInput->SetAcceleratorTable(accel);
}


string CNetBLASTLoadOptionPanel::GetInput() const
{
    wxTextCtrl* ridInput = dynamic_cast<wxTextCtrl*>(FindWindow(ID_TEXTCTRL1));
    _ASSERT(ridInput);
    return string(ridInput->GetValue().ToUTF8());
}


void CNetBLASTLoadOptionPanel::SetInput(const string& input)
{
    wxTextCtrl* ridInput = dynamic_cast<wxTextCtrl*>(FindWindow(ID_TEXTCTRL1));
    _ASSERT(ridInput);
    ridInput->SetValue(wxString::FromUTF8(input.c_str()));
}


bool CNetBLASTLoadOptionPanel::IsInputValid()
{
    return true;
}

static string sDelim(" \t\n,;");

void CNetBLASTLoadOptionPanel::GetRIDs(vector<string>& rids)
{
    wxTextCtrl* ridInput = dynamic_cast<wxTextCtrl*>(FindWindow(ID_TEXTCTRL1));
    _ASSERT(ridInput);
    string text = ToStdString(ridInput->GetValue());

    vector<string> tokens;
    NStr::Split(text, sDelim, tokens, NStr::fSplit_Tokenize);

    rids = tokens;
}


bool CNetBLASTLoadOptionPanel::ShowToolTips()
{
    return true;
}


wxBitmap CNetBLASTLoadOptionPanel::GetBitmapResource( const wxString& name )
{
    // Bitmap retrieval
////@begin CNetBLASTLoadOptionPanel bitmap retrieval
    wxUnusedVar(name);
    return wxNullBitmap;
////@end CNetBLASTLoadOptionPanel bitmap retrieval
}

wxIcon CNetBLASTLoadOptionPanel::GetIconResource( const wxString& name )
{
    // Icon retrieval
////@begin CNetBLASTLoadOptionPanel icon retrieval
    wxUnusedVar(name);
    return wxNullIcon;
////@end CNetBLASTLoadOptionPanel icon retrieval
}


END_NCBI_SCOPE
