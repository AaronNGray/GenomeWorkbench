/*  $Id: app_options_dlg.cpp 25480 2012-03-27 14:57:20Z kuznets $
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
 * Authors:  Roman Katargin
 *
 * File Description:
 *
 */

#include <ncbi_pch.hpp>

////@begin includes
////@end includes

#include <gui/framework/options_dlg_extension.hpp>
#include <gui/framework/app_options_dlg.hpp>
#include <gui/utils/extension_impl.hpp>
#include <gui/widgets/wx/wx_utils.hpp>

#include <wx/sizer.h>
#include <wx/button.h>
#include <wx/bitmap.h>
#include <wx/icon.h>

BEGIN_NCBI_SCOPE

////@begin XPM images
////@end XPM images

IMPLEMENT_DYNAMIC_CLASS( CAppOptionsDlg, CDialog )

BEGIN_EVENT_TABLE( CAppOptionsDlg, CDialog )

////@begin CAppOptionsDlg event table entries
////@end CAppOptionsDlg event table entries

END_EVENT_TABLE()

CAppOptionsDlg::CAppOptionsDlg()
{
    Init();
}

CAppOptionsDlg::CAppOptionsDlg( wxWindow* parent, wxWindowID id, const wxString& caption, const wxPoint& pos, const wxSize& size, long style )
{
    Init();
    Create(parent, id, caption, pos, size, style);
}

bool CAppOptionsDlg::Create( wxWindow* parent, wxWindowID id, const wxString& caption, const wxPoint& pos, const wxSize& size, long style )
{
////@begin CAppOptionsDlg creation
    SetExtraStyle(wxWS_EX_VALIDATE_RECURSIVELY|wxWS_EX_BLOCK_EVENTS);
    CDialog::Create( parent, id, caption, pos, size, style );

    CreateControls();
    if (GetSizer())
    {
        GetSizer()->SetSizeHints(this);
    }
    Centre();
////@end CAppOptionsDlg creation
    return true;
}

CAppOptionsDlg::~CAppOptionsDlg()
{
////@begin CAppOptionsDlg destruction
////@end CAppOptionsDlg destruction
}

void CAppOptionsDlg::Init()
{
////@begin CAppOptionsDlg member initialisation
////@end CAppOptionsDlg member initialisation
}

static bool ComparePages(IOptionsDlgExtension* page1, IOptionsDlgExtension* page2)
{
    size_t p1 = page1->GetPriority(), p2 = page2->GetPriority();
    if (p1 == p2)
        return page1->GetPageLabel() < page2->GetPageLabel();
    return p1 < p2;
}

void CAppOptionsDlg::CreateControls()
{
////@begin CAppOptionsDlg content construction
    CAppOptionsDlg* itemCDialog1 = this;

    wxBoxSizer* itemBoxSizer2 = new wxBoxSizer(wxVERTICAL);
    itemCDialog1->SetSizer(itemBoxSizer2);

    wxNotebook* itemNotebook3 = new wxNotebook( itemCDialog1, ID_NOTEBOOK1, wxDefaultPosition, wxDefaultSize, wxBK_DEFAULT );

    itemBoxSizer2->Add(itemNotebook3, 1, wxGROW|wxALL, 5);

    wxStaticLine* itemStaticLine4 = new wxStaticLine( itemCDialog1, wxID_STATIC, wxDefaultPosition, wxDefaultSize, wxLI_HORIZONTAL );
    itemBoxSizer2->Add(itemStaticLine4, 0, wxGROW|wxALL, 5);

    wxStdDialogButtonSizer* itemStdDialogButtonSizer5 = new wxStdDialogButtonSizer;

    itemBoxSizer2->Add(itemStdDialogButtonSizer5, 0, wxALIGN_RIGHT|wxALL, 5);
    wxButton* itemButton6 = new wxButton( itemCDialog1, wxID_OK, wxT("&OK"), wxDefaultPosition, wxDefaultSize, 0 );
    itemStdDialogButtonSizer5->AddButton(itemButton6);

    wxButton* itemButton7 = new wxButton( itemCDialog1, wxID_CANCEL, wxT("&Cancel"), wxDefaultPosition, wxDefaultSize, 0 );
    itemStdDialogButtonSizer5->AddButton(itemButton7);

    wxButton* itemButton8 = new wxButton( itemCDialog1, wxID_APPLY, wxT("&Apply"), wxDefaultPosition, wxDefaultSize, 0 );
    itemStdDialogButtonSizer5->AddButton(itemButton8);

    itemStdDialogButtonSizer5->Realize();

////@end CAppOptionsDlg content construction

    vector< IOptionsDlgExtension* > pages;
    GetExtensionAsInterface(EXT_POINT__OPTIONS_DLG_EXTENSION, pages);

    if (pages.size() > 1)
        sort(pages.begin(), pages.end(), ComparePages);

    wxNotebook* notebook = (wxNotebook*)FindWindow(ID_NOTEBOOK1);
    ITERATE(vector< IOptionsDlgExtension* >, it, pages) {
        wxWindow* page = (*it)->CreateSettingsPage(notebook);
        notebook->AddPage(page, ToWxString((*it)->GetPageLabel()));
    }
}

bool CAppOptionsDlg::ShowToolTips()
{
    return true;
}
wxBitmap CAppOptionsDlg::GetBitmapResource( const wxString& name )
{
    // Bitmap retrieval
////@begin CAppOptionsDlg bitmap retrieval
    wxUnusedVar(name);
    return wxNullBitmap;
////@end CAppOptionsDlg bitmap retrieval
}
wxIcon CAppOptionsDlg::GetIconResource( const wxString& name )
{
    // Icon retrieval
////@begin CAppOptionsDlg icon retrieval
    wxUnusedVar(name);
    return wxNullIcon;
////@end CAppOptionsDlg icon retrieval
}

END_NCBI_SCOPE
