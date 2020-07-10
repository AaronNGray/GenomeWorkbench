/*  $Id: about_dlg.cpp 25633 2012-04-13 20:51:47Z katargir $
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
 * Authors:  Andrey Yazhuk
 *
 * File Description:
 */

#include <ncbi_pch.hpp>

////@begin includes
////@end includes

#include <gui/wx_demo/about_dlg.hpp>

#include <wx/sizer.h>
#include <wx/button.h>
#include <wx/icon.h>

BEGIN_NCBI_SCOPE

/*!
 * CwxAboutDlg type definition
 */

IMPLEMENT_DYNAMIC_CLASS( CwxAboutDlg, wxDialog )

/*!
 * CwxAboutDlg event table definition
 */

BEGIN_EVENT_TABLE( CwxAboutDlg, wxDialog )

////@begin CwxAboutDlg event table entries
////@end CwxAboutDlg event table entries

END_EVENT_TABLE()

/*!
 * CwxAboutDlg constructors
 */

CwxAboutDlg::CwxAboutDlg()
{
    Init();
}

CwxAboutDlg::CwxAboutDlg( wxWindow* parent, wxWindowID id, const wxString& caption, const wxPoint& pos, const wxSize& size, long style )
{
    Init();
    Create(parent, id, caption, pos, size, style);
}

/*!
 * CwxAboutDlg creator
 */

bool CwxAboutDlg::Create( wxWindow* parent, wxWindowID id, const wxString& caption, const wxPoint& pos, const wxSize& size, long style )
{
////@begin CwxAboutDlg creation
    SetExtraStyle(wxWS_EX_BLOCK_EVENTS);
    wxDialog::Create( parent, id, caption, pos, size, style );

    CreateControls();
    if (GetSizer())
    {
        GetSizer()->SetSizeHints(this);
    }
    Centre();
////@end CwxAboutDlg creation
    return true;
}

/*!
 * CwxAboutDlg destructor
 */

CwxAboutDlg::~CwxAboutDlg()
{
////@begin CwxAboutDlg destruction
////@end CwxAboutDlg destruction
}

/*!
 * Member initialisation
 */

void CwxAboutDlg::Init()
{
////@begin CwxAboutDlg member initialisation
    m_HTML = NULL;
////@end CwxAboutDlg member initialisation
}

/*!
 * Control creation for CwxAboutDlg
 */

void CwxAboutDlg::CreateControls()
{
////@begin CwxAboutDlg content construction
    CwxAboutDlg* itemDialog1 = this;

    wxBoxSizer* itemBoxSizer2 = new wxBoxSizer(wxVERTICAL);
    itemDialog1->SetSizer(itemBoxSizer2);

    m_HTML = new wxHtmlWindow( itemDialog1, ID_HTMLWINDOW1, wxDefaultPosition, wxSize(300, 180), wxHW_SCROLLBAR_AUTO|wxSUNKEN_BORDER|wxHSCROLL|wxVSCROLL );
    itemBoxSizer2->Add(m_HTML, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

    wxStaticLine* itemStaticLine4 = new wxStaticLine( itemDialog1, ID_STATICLINE, wxDefaultPosition, wxDefaultSize, wxLI_HORIZONTAL );
    itemBoxSizer2->Add(itemStaticLine4, 0, wxGROW|wxALL, 5);

    wxStdDialogButtonSizer* itemStdDialogButtonSizer5 = new wxStdDialogButtonSizer;

    itemBoxSizer2->Add(itemStdDialogButtonSizer5, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);
    wxButton* itemButton6 = new wxButton( itemDialog1, wxID_OK, _("&OK"), wxDefaultPosition, wxDefaultSize, 0 );
    itemStdDialogButtonSizer5->AddButton(itemButton6);

    itemStdDialogButtonSizer5->Realize();

////@end CwxAboutDlg content construction
}

/*!
 * Should we show tooltips?
 */

bool CwxAboutDlg::ShowToolTips()
{
    return true;
}

/*!
 * Get bitmap resources
 */

wxBitmap CwxAboutDlg::GetBitmapResource( const wxString& name )
{
    // Bitmap retrieval
////@begin CwxAboutDlg bitmap retrieval
    wxUnusedVar(name);
    return wxNullBitmap;
////@end CwxAboutDlg bitmap retrieval
}

/*!
 * Get icon resources
 */
const wxChar* s_about =
wxT("<h1>About wx Demo App</h1></br><p>This is the first example of wxWidgets application.</p>\
    <p>wxHTMLView can display <b>bold text</b> as well as <i>italic</i>.</p>\
    It can also show <a href=\"http://www.yahoo.com\">links</a>.\
    <h2>Here is a table</h2>\
    <table><tr><td>1</td><td>2</td></tr><tr><td>3</td><td>4</td></tr></table>");

bool CwxAboutDlg::TransferDataToWindow()
{
    bool ok = wxDialog::TransferDataToWindow();
    if(ok)  {
        m_HTML->SetPage(s_about);
    }
    return ok;
}

wxIcon CwxAboutDlg::GetIconResource( const wxString& name )
{
    // Icon retrieval
////@begin CwxAboutDlg icon retrieval
    wxUnusedVar(name);
    return wxNullIcon;
////@end CwxAboutDlg icon retrieval
}

END_NCBI_SCOPE
