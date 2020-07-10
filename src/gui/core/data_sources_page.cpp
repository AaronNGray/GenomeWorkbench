/*  $Id: data_sources_page.cpp 23723 2011-05-23 14:55:17Z wuliangs $
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

#include <gui/utils/extension_impl.hpp>
#include <gui/utils/ui_object.hpp>
#include <gui/framework/options_dlg_extension.hpp>
#include <gui/core/ui_data_source_service.hpp>

#include <wx/sizer.h>
#include <wx/choice.h>
#include <wx/bitmap.h>
#include <wx/icon.h>

////@begin includes
////@end includes

#include <gui/core/data_sources_page.hpp>

BEGIN_NCBI_SCOPE

////@begin XPM images
////@end XPM images

IMPLEMENT_DYNAMIC_CLASS( CDataSourcesPage, wxPanel )

BEGIN_EVENT_TABLE( CDataSourcesPage, wxPanel )

////@begin CDataSourcesPage event table entries
////@end CDataSourcesPage event table entries

END_EVENT_TABLE()

CDataSourcesPage::CDataSourcesPage() : m_Service()
{
    Init();
}

CDataSourcesPage::CDataSourcesPage( wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style )
    : m_Service()
{
    Init();
    Create(parent, id, pos, size, style);
}

bool CDataSourcesPage::Create( wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style )
{
////@begin CDataSourcesPage creation
    wxPanel::Create( parent, id, pos, size, style );

    CreateControls();
    if (GetSizer())
    {
        GetSizer()->SetSizeHints(this);
    }
    Centre();
////@end CDataSourcesPage creation
    return true;
}

CDataSourcesPage::~CDataSourcesPage()
{
////@begin CDataSourcesPage destruction
////@end CDataSourcesPage destruction
}

void CDataSourcesPage::Init()
{
////@begin CDataSourcesPage member initialisation
////@end CDataSourcesPage member initialisation
}

void CDataSourcesPage::CreateControls()
{
////@begin CDataSourcesPage content construction
    CDataSourcesPage* itemPanel1 = this;

    wxBoxSizer* itemBoxSizer2 = new wxBoxSizer(wxVERTICAL);
    itemPanel1->SetSizer(itemBoxSizer2);

    wxArrayString itemChoice3Strings;
    wxChoice* itemChoice3 = new wxChoice( itemPanel1, ID_CHOICE2, wxDefaultPosition, wxDefaultSize, itemChoice3Strings, 0 );
    itemBoxSizer2->Add(itemChoice3, 0, wxGROW|wxALL, 5);

////@end CDataSourcesPage content construction

    if (m_Service) {
        wxChoice* choice = (wxChoice*)FindWindow(ID_CHOICE2);
        CUIDataSourceService::TDataSourceTypeVec types;
        m_Service->GetDataSourceTypes(types);
        NON_CONST_ITERATE(CUIDataSourceService::TDataSourceTypeVec, it, types) {
            const IUIObject& descr = (*it)->GetDescr();
            choice->Append(ToWxString(descr.GetLabel()));
        }

        // This setting doesn't affect anything currently, disable it for now.
        // The choice is just for display purpose for now.  It should be loaded
        // from the saved settings if it gets reenabled in the future.
        if (choice->GetCount() > 1) {
            choice->SetSelection(1);
        } else if (choice->GetCount() > 0) {
            choice->SetSelection(0);
        }

        choice->Disable();
    }
}

bool CDataSourcesPage::ShowToolTips()
{
    return true;
}
wxBitmap CDataSourcesPage::GetBitmapResource( const wxString& name )
{
    // Bitmap retrieval
////@begin CDataSourcesPage bitmap retrieval
    wxUnusedVar(name);
    return wxNullBitmap;
////@end CDataSourcesPage bitmap retrieval
}
wxIcon CDataSourcesPage::GetIconResource( const wxString& name )
{
    // Icon retrieval
////@begin CDataSourcesPage icon retrieval
    wxUnusedVar(name);
    return wxNullIcon;
////@end CDataSourcesPage icon retrieval
}

END_NCBI_SCOPE

