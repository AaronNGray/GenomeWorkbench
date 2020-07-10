/*  $Id: multipleassemblyprogrampanel.cpp 36594 2016-10-12 20:17:36Z evgeniev $
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
#include <gui/widgets/edit/multipleassemblyprogrampanel.hpp>
#include <wx/textctrl.h>
#include <wx/grid.h>

////@begin XPM images
////@end XPM images

BEGIN_NCBI_SCOPE

/*!
 * CMultipleAssemblyProgramPanel type definition
 */

IMPLEMENT_DYNAMIC_CLASS( CMultipleAssemblyProgramPanel, CAssemblyProgramPanel )


/*!
 * CMultipleAssemblyProgramPanel event table definition
 */

BEGIN_EVENT_TABLE( CMultipleAssemblyProgramPanel, CAssemblyProgramPanel )

////@begin CMultipleAssemblyProgramPanel event table entries
////@end CMultipleAssemblyProgramPanel event table entries
    EVT_TEXT( ID_LASTVERSION, CMultipleAssemblyProgramPanel::OnVersionEntered )

END_EVENT_TABLE()


/*!
 * CMultipleAssemblyProgramPanel constructors
 */

CMultipleAssemblyProgramPanel::CMultipleAssemblyProgramPanel()
{
    Init();
}

CMultipleAssemblyProgramPanel::CMultipleAssemblyProgramPanel( wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style )
{
    Init();
    Create(parent, id, pos, size, style);
}


/*!
 * CMultipleAssemblyProgramPanel creator
 */

bool CMultipleAssemblyProgramPanel::Create( wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style )
{
////@begin CMultipleAssemblyProgramPanel creation
    CAssemblyProgramPanel::Create( parent, id, pos, size, style );

    CreateControls();
    if (GetSizer())
    {
        GetSizer()->SetSizeHints(this);
    }
    Centre();
////@end CMultipleAssemblyProgramPanel creation
    return true;
}


/*!
 * CMultipleAssemblyProgramPanel destructor
 */

CMultipleAssemblyProgramPanel::~CMultipleAssemblyProgramPanel()
{
////@begin CMultipleAssemblyProgramPanel destruction
////@end CMultipleAssemblyProgramPanel destruction
}


/*!
 * Member initialisation
 */

void CMultipleAssemblyProgramPanel::Init()
{
////@begin CMultipleAssemblyProgramPanel member initialisation
    m_ScrolledWindow = NULL;
    m_Sizer = NULL;
////@end CMultipleAssemblyProgramPanel member initialisation
}


/*!
 * Control creation for CMultipleAssemblyProgramPanel
 */

void CMultipleAssemblyProgramPanel::CreateControls()
{    
////@begin CMultipleAssemblyProgramPanel content construction
    CMultipleAssemblyProgramPanel* itemCAssemblyProgramPanel1 = this;

    wxBoxSizer* itemBoxSizer2 = new wxBoxSizer(wxVERTICAL);
    itemCAssemblyProgramPanel1->SetSizer(itemBoxSizer2);

    wxBoxSizer* itemBoxSizer3 = new wxBoxSizer(wxHORIZONTAL);
    itemBoxSizer2->Add(itemBoxSizer3, 0, wxALIGN_LEFT|wxALL, 5);

    wxStaticText* itemStaticText4 = new wxStaticText( itemCAssemblyProgramPanel1, wxID_STATIC, _("Assembly Program (required)"), wxDefaultPosition, wxSize(150, -1), 0 );
    itemBoxSizer3->Add(itemStaticText4, 0, wxALIGN_CENTER_VERTICAL|wxALL, 0);

    wxStaticText* itemStaticText5 = new wxStaticText( itemCAssemblyProgramPanel1, wxID_STATIC, _("Version or date (required)"), wxDefaultPosition, wxSize(150, -1), 0 );
    itemBoxSizer3->Add(itemStaticText5, 0, wxALIGN_CENTER_VERTICAL|wxALL, 0);

    m_ScrolledWindow = new wxScrolledWindow( itemCAssemblyProgramPanel1, ID_SCROLLEDWINDOW, wxDefaultPosition, wxSize(360, 65), wxSUNKEN_BORDER|wxVSCROLL|wxTAB_TRAVERSAL );
    itemBoxSizer2->Add(m_ScrolledWindow, 0, wxALIGN_LEFT|wxALL|wxFIXED_MINSIZE, 0);
    m_ScrolledWindow->SetScrollbars(1, 1, 0, 0);
    m_Sizer = new wxFlexGridSizer(0, 2, 0, 0);
    m_ScrolledWindow->SetSizer(m_Sizer);

    m_ScrolledWindow->FitInside();

////@end CMultipleAssemblyProgramPanel content construction
    m_TotalHeight = 0;
    m_RowHeight = 20;
    m_TextWidth = 150;
}


void CMultipleAssemblyProgramPanel::x_AddBlankRow ()
{
    wxTextCtrl* p = new wxTextCtrl( m_ScrolledWindow, wxID_ANY, wxEmptyString, wxDefaultPosition, wxSize(m_TextWidth, m_RowHeight), 0 );
    m_Sizer->Add(p, 0, wxGROW|wxALIGN_CENTER_VERTICAL|wxALL, 0);
    wxTextCtrl* v = new wxTextCtrl( m_ScrolledWindow, ID_LASTVERSION, wxEmptyString, wxDefaultPosition, wxSize(m_TextWidth, m_RowHeight), 0 );
    m_Sizer->Add(v, 0, wxGROW|wxALIGN_CENTER_VERTICAL|wxALL, 0);
    m_TotalHeight += m_RowHeight;
}


void CMultipleAssemblyProgramPanel::SetValue( const string& val)
{
    m_Sizer->Clear(true);
    m_TotalHeight = 0;
    vector<string> programs;
    NStr::Split(val, ";", programs);
    string last_program = "";
    string last_version = "";
    for (size_t i = 0; i < programs.size(); i++) {
        size_t pos = NStr::Find(programs[i], " v.");
        string prog = "";
        string ver = "";
        if (pos == string::npos) {
            prog = programs[i];
        } else {
            prog = programs[i].substr(0, pos);
            ver = programs[i].substr(pos + 3);
        }
        NStr::TruncateSpacesInPlace (prog);
        NStr::TruncateSpacesInPlace (ver);

        wxTextCtrl* p = new wxTextCtrl( m_ScrolledWindow, wxID_ANY, ToWxString (prog), wxDefaultPosition, wxSize(m_TextWidth, m_RowHeight), 0 );
        m_Sizer->Add(p, 0, wxGROW|wxALIGN_CENTER_VERTICAL|wxALL, 0);
        wxTextCtrl* v = new wxTextCtrl( m_ScrolledWindow, ID_LASTVERSION, ToWxString (ver), wxDefaultPosition, wxSize(m_TextWidth, m_RowHeight), 0 );
        m_Sizer->Add(v, 0, wxGROW|wxALIGN_CENTER_VERTICAL|wxALL, 0);
        last_program = prog;
        last_version = ver;
          m_TotalHeight += m_RowHeight;
    }
 
    x_AddBlankRow ();

    int win_height, win_width;
    m_ScrolledWindow->GetSize(&win_width, &win_height);
    m_ScrolledWindow->SetVirtualSize(m_TotalHeight, win_width);
    m_ScrolledWindow->SetScrollRate(0, m_RowHeight);

}


string CMultipleAssemblyProgramPanel::GetValue ()
{
    string program = "";

    wxSizerItemList& itemList = m_Sizer->GetChildren();
    size_t pos = 0;
    while (pos < itemList.size() - 1) {   
        wxTextCtrl* p_ctrl = dynamic_cast<wxTextCtrl*>(itemList[pos]->GetWindow());
        wxTextCtrl* v_ctrl = dynamic_cast<wxTextCtrl*>(itemList[pos + 1]->GetWindow());
        if (p_ctrl && v_ctrl) {
            string p = ToStdString (p_ctrl->GetValue());
            string v = ToStdString (v_ctrl->GetValue());
            NStr::TruncateSpacesInPlace (p);
            NStr::TruncateSpacesInPlace (v);
            if (!NStr::IsBlank(p) || !NStr::IsBlank(v)) {
                if (!NStr::IsBlank(program)) {
                    program += "; ";
                }
                program += p;
                if (!NStr::IsBlank(v)) {
                    program += " v. ";
                    program += v;
                }
            }
        }
        pos+= 2;
    }
    
    return program;
}


/*!
 * Should we show tooltips?
 */

bool CMultipleAssemblyProgramPanel::ShowToolTips()
{
    return true;
}

/*!
 * Get bitmap resources
 */

wxBitmap CMultipleAssemblyProgramPanel::GetBitmapResource( const wxString& name )
{
    // Bitmap retrieval
////@begin CMultipleAssemblyProgramPanel bitmap retrieval
    wxUnusedVar(name);
    return wxNullBitmap;
////@end CMultipleAssemblyProgramPanel bitmap retrieval
}

/*!
 * Get icon resources
 */

wxIcon CMultipleAssemblyProgramPanel::GetIconResource( const wxString& name )
{
    // Icon retrieval
////@begin CMultipleAssemblyProgramPanel icon retrieval
    wxUnusedVar(name);
    return wxNullIcon;
////@end CMultipleAssemblyProgramPanel icon retrieval
}


void CMultipleAssemblyProgramPanel::OnVersionEntered ( wxCommandEvent& event )
{
    wxTextCtrl* item = (wxTextCtrl*)event.GetEventObject();
    // is it blank?
    if (NStr::IsBlank(ToStdString(item->GetValue()))) {
        return;
    }
    // is this the last version?
    wxSizerItemList& itemList = m_Sizer->GetChildren();
    wxTextCtrl* last = dynamic_cast<wxTextCtrl*>(itemList[itemList.size() - 1]->GetWindow());
    if (last == item) {
        x_AddBlankRow ();
        int win_height, win_width;
        m_ScrolledWindow->GetSize(&win_width, &win_height);
        m_ScrolledWindow->SetVirtualSize(m_TotalHeight, win_width);
        m_ScrolledWindow->SetScrollRate(0, m_RowHeight);
    }              


}

END_NCBI_SCOPE
