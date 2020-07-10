/*  $Id: pubauthor_panel.cpp 42811 2019-04-18 10:15:32Z bollin $
* ========================================================================== =
*
*                            PUBLIC DOMAIN NOTICE
*               National Center for Biotechnology Information
*
*  This software / database is a "United States Government Work" under the
*  terms of the United States Copyright Act.It was written as part of
*  the author's official duties as a United States Government employee and
*  thus cannot be copyrighted.This software / database is freely available
*  to the public for use.The National Library of Medicine and the U.S.
*  Government have not placed any restriction on its use or reproduction.
*
*  Although all reasonable efforts have been taken to ensure the accuracy
*  and reliability of the software and data, the NLM and the U.S.
*  Government do not and cannot warrant the performance or results that
*  may be obtained by using this software or data.The NLM and the U.S.
*  Government disclaim all warranties, express or implied, including
*  warranties of performance, merchantability or fitness for any particular
*  purpose.
*
*  Please cite the author in any work or product based on this material.
*
* ========================================================================== =
*
* Authors: Andrea Asztalos
*
*/

#include <ncbi_pch.hpp>
#include <objects/biblio/Auth_list.hpp>
#include <objects/biblio/Author.hpp>
#include <objects/general/Person_id.hpp>
#include <objects/general/Name_std.hpp>
#include <gui/widgets/wx/unfocused_controls.hpp>
#include <gui/widgets/edit/singleauthor_panel.hpp>
#include <gui/widgets/edit/consortium_panel.hpp>
#include <gui/widgets/edit/pubauthor_panel.hpp>

#include <wx/sizer.h>
#include <wx/scrolwin.h>
#include <wx/stattext.h>
#include <wx/icon.h>

BEGIN_NCBI_SCOPE
USING_SCOPE(objects);

/*
 * CPubAuthorPanel type definition
 */

IMPLEMENT_DYNAMIC_CLASS( CPubAuthorPanel, wxPanel )


/*
 * CPubAuthorPanel event table definition
 */

BEGIN_EVENT_TABLE( CPubAuthorPanel, wxPanel )

////@begin CPubAuthorPanel event table entries
    EVT_HYPERLINK( ID_MOLADDCHROMOSOME, CPubAuthorPanel::OnAddAuthorClicked )
    EVT_HYPERLINK( ID_MOLDELCHROMOSOME, CPubAuthorPanel::OnAddConsortiumClicked )
////@end CPubAuthorPanel event table entries

END_EVENT_TABLE()


/*
 * CPubAuthorPanel constructors
 */

CPubAuthorPanel::CPubAuthorPanel()
{
    Init();
}

CPubAuthorPanel::CPubAuthorPanel( wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style )
{
    Init();
    Create(parent, id, pos, size, style);
}


/*
 * CPubAuthorPanel creator
 */

bool CPubAuthorPanel::Create( wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style )
{
////@begin CPubAuthorPanel creation
    SetExtraStyle(wxWS_EX_VALIDATE_RECURSIVELY);
    wxPanel::Create( parent, id, pos, size, style );

    CreateControls();
    Centre();
////@end CPubAuthorPanel creation
    return true;
}


/*
 * CPubAuthorPanel destructor
 */

CPubAuthorPanel::~CPubAuthorPanel()
{
////@begin CPubAuthorPanel destruction
////@end CPubAuthorPanel destruction
}


/*
 * Member initialisation
 */

void CPubAuthorPanel::Init()
{
////@begin CPubAuthorPanel member initialisation
    m_ScrolledWindow = NULL;
    m_Sizer = NULL;
////@end CPubAuthorPanel member initialisation
}


/*
 * Control creation for CPubAuthorPanel
 */

void CPubAuthorPanel::CreateControls()
{    
////@begin CPubAuthorPanel content construction
    CPubAuthorPanel* itemPanel1 = this;

    wxBoxSizer* itemBoxSizer2 = new wxBoxSizer(wxVERTICAL);
    itemPanel1->SetSizer(itemBoxSizer2);

    itemBoxSizer2->Add(5, 5, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

    wxFlexGridSizer* itemFlexGridSizer3 = new wxFlexGridSizer(0, 1, 0, 0);
    itemBoxSizer2->Add(itemFlexGridSizer3, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 0);

    wxBoxSizer* itemBoxSizer4 = new wxBoxSizer(wxHORIZONTAL);
    itemFlexGridSizer3->Add(itemBoxSizer4, 0, wxGROW|wxALIGN_CENTER_VERTICAL|wxALL, 0);

    wxStaticText* itemStaticText5 = new wxStaticText( itemPanel1, wxID_STATIC, _("First (given) name*"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer4->Add(itemStaticText5, 0, wxALIGN_CENTER_VERTICAL|wxRIGHT|wxTOP|wxBOTTOM, 5);

    itemBoxSizer4->Add(1, 5, 0, wxALIGN_CENTER_VERTICAL | wxALL, 5);

    wxStaticText* itemStaticText6 = new wxStaticText( itemPanel1, wxID_STATIC, _("Middle initial(s)"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer4->Add(itemStaticText6, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    itemBoxSizer4->Add(1, 5, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxStaticText* itemStaticText8 = new wxStaticText( itemPanel1, wxID_STATIC, _("Last (family) name*"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer4->Add(itemStaticText8, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    itemBoxSizer4->Add(5, 5, 0, wxALIGN_CENTER_VERTICAL | wxALL, 0);

    wxStaticText* itemStaticText1 = new wxStaticText( itemPanel1, wxID_STATIC, _("Suffix"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer4->Add(itemStaticText1, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    m_ScrolledWindow = new wxScrolledWindow( itemPanel1, ID_REFAUTHORSCROLLEDWND, wxDefaultPosition, wxSize(440, 92), wxNO_BORDER|wxHSCROLL|wxVSCROLL );
    itemFlexGridSizer3->Add(m_ScrolledWindow, 0, wxGROW|wxALIGN_CENTER_VERTICAL|wxALL, 0);
    m_ScrolledWindow->SetScrollbars(1, 1, 0, 0);
    
    m_Sizer = new wxFlexGridSizer(0, 2, 0, 0);
    m_ScrolledWindow->SetSizer(m_Sizer);

    for (size_t index = 0; index < m_Rows; ++index) {
        x_AddAuthorRow();
    }

    wxBoxSizer* itemBoxSizer1 = new wxBoxSizer(wxHORIZONTAL);
    itemFlexGridSizer3->Add(itemBoxSizer1, 0, wxGROW|wxALIGN_CENTER_VERTICAL|wxTOP|wxBOTTOM|wxRESERVE_SPACE_EVEN_IF_HIDDEN, 5);

    wxHyperlinkCtrl* itemHyperlinkCtrl2 = new wxHyperlinkCtrl( itemPanel1, ID_MOLADDCHROMOSOME, _("Add another author"), wxEmptyString, wxDefaultPosition, wxDefaultSize, wxHL_DEFAULT_STYLE );
    itemBoxSizer1->Add(itemHyperlinkCtrl2, 0, wxALIGN_CENTER_VERTICAL|wxALL|wxRESERVE_SPACE_EVEN_IF_HIDDEN, 5);

    itemBoxSizer1->Add(5, 5, 1, wxALIGN_CENTER_VERTICAL|wxALL|wxRESERVE_SPACE_EVEN_IF_HIDDEN, 5);

    wxHyperlinkCtrl* itemHyperlinkCtrl4 = new wxHyperlinkCtrl( itemPanel1, ID_MOLDELCHROMOSOME, _("Add consortium"), wxEmptyString, wxDefaultPosition, wxDefaultSize, wxHL_DEFAULT_STYLE );
    itemBoxSizer1->Add(itemHyperlinkCtrl4, 0, wxALIGN_CENTER_VERTICAL|wxALL|wxRESERVE_SPACE_EVEN_IF_HIDDEN, 5);



////@end CPubAuthorPanel content construction
}

void CPubAuthorPanel::x_AddAuthorRow()
{
    CRef<CAuthor> auth(new CAuthor());
    CSingleAuthorPanel* row = new CSingleAuthorPanel(m_ScrolledWindow, *auth);
    row->m_FirstNameCtrl->SetMinSize(wxSize(120, -1));
    row->m_LastNameCtrl->SetMinSize(wxSize(120, -1));
    row->HideNonTextCtrls();
    if (m_RowHeight == 0) {
        int row_width;
        row->GetClientSize(&row_width, &m_RowHeight);
    }
    x_AddRowToWindow(row);
}

void CPubAuthorPanel::x_AddConsortiumRow()
{
    CRef<CAuthor> auth(new CAuthor());
    CConsortiumPanel* row = new CConsortiumPanel(m_ScrolledWindow, *auth);
    if (m_RowHeight == 0) {
        int row_width;
        row->GetClientSize(&row_width, &m_RowHeight);
    }
    x_AddRowToWindow(row);
}

void CPubAuthorPanel::x_AddRowToWindow(wxWindow* row)
{
    m_Sizer->Add(row, 0, wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL | wxALL, 0);
    wxHyperlinkCtrl* itemHyperLink = new CNoTabHyperlinkCtrl(m_ScrolledWindow, wxID_ANY, wxT("Delete"), wxT("delete"));
    itemHyperLink->SetVisitedColour(itemHyperLink->GetNormalColour());
    m_Sizer->Add(itemHyperLink, 0, wxALIGN_BOTTOM | wxBOTTOM | wxRIGHT, 0);
    m_ScrolledWindow->SetScrollRate(0, m_RowHeight);
    m_ScrolledWindow->FitInside();
    m_ScrolledWindow->Refresh();
}



/*
 * Should we show tooltips?
 */

bool CPubAuthorPanel::ShowToolTips()
{
    return true;
}

/*
 * Get bitmap resources
 */

wxBitmap CPubAuthorPanel::GetBitmapResource( const wxString& name )
{
    // Bitmap retrieval
////@begin CPubAuthorPanel bitmap retrieval
    wxUnusedVar(name);
    return wxNullBitmap;
////@end CPubAuthorPanel bitmap retrieval
}

/*
 * Get icon resources
 */

wxIcon CPubAuthorPanel::GetIconResource( const wxString& name )
{
    // Icon retrieval
////@begin CPubAuthorPanel icon retrieval
    wxUnusedVar(name);
    return wxNullIcon;
////@end CPubAuthorPanel icon retrieval
}


/*
 * wxEVT_COMMAND_HYPERLINK event handler for ID_MOLADDCHROMOSOME
 */

void CPubAuthorPanel::OnAddAuthorClicked( wxHyperlinkEvent& event )
{
    x_AddAuthorRow();
}


/*
 * wxEVT_COMMAND_HYPERLINK event handler for ID_MOLDELCHROMOSOME
 */

void CPubAuthorPanel::OnAddConsortiumClicked( wxHyperlinkEvent& event )
{
    x_AddConsortiumRow();
}


bool CPubAuthorPanel::TransferDataToWindow() 
{
    // TODO
    return true;
}

bool CPubAuthorPanel::TransferDataFromWindow()
{
    // TODO
    // dummy for now
    m_Sub->SetAuthors().SetNames().SetStr().push_back("?");
    return true;
}

void CPubAuthorPanel::ApplyCitSub(objects::CCit_sub& sub)
{
    m_Sub.Reset(&sub);
    TransferDataToWindow();
}


END_NCBI_SCOPE
