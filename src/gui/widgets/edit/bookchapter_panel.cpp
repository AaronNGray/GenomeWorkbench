/*  $Id: bookchapter_panel.cpp 24375 2011-09-19 20:47:12Z katargir $
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
#include <objects/seqfeat/Seq_feat.hpp>

#include <gui/widgets/edit/serial_member_primitive_validators.hpp>

#include <wx/sizer.h>
#include <wx/bitmap.h>
#include <wx/icon.h>
#include <wx/stattext.h>
#include <wx/textctrl.h>

#include "bookchapter_panel.hpp"

BEGIN_NCBI_SCOPE
USING_SCOPE(objects);

////@begin XPM images
////@end XPM images


/*
 * CBookChapterPanel type definition
 */

IMPLEMENT_DYNAMIC_CLASS( CBookChapterPanel, wxPanel )


/*
 * CBookChapterPanel event table definition
 */

BEGIN_EVENT_TABLE( CBookChapterPanel, wxPanel )

////@begin CBookChapterPanel event table entries
////@end CBookChapterPanel event table entries

END_EVENT_TABLE()


/*
 * CBookChapterPanel constructors
 */

CBookChapterPanel::CBookChapterPanel()
{
    Init();
}

CBookChapterPanel::CBookChapterPanel( wxWindow* parent, CSerialObject& object, objects::CScope& scope,
         wxWindowID id, const wxPoint& pos, const wxSize& size, long style ) :
     m_Object(0)
{
    Init();
    m_Object = dynamic_cast<CSeq_feat*>(&object);
    Create(parent, id, pos, size, style);
}


/*
 * CBookChapterPanel creator
 */

bool CBookChapterPanel::Create( wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style )
{
////@begin CBookChapterPanel creation
    SetExtraStyle(wxWS_EX_BLOCK_EVENTS);
    wxPanel::Create( parent, id, pos, size, style );

    CreateControls();
    if (GetSizer())
    {
        GetSizer()->SetSizeHints(this);
    }
    Centre();
////@end CBookChapterPanel creation
    return true;
}


/*
 * CBookChapterPanel destructor
 */

CBookChapterPanel::~CBookChapterPanel()
{
////@begin CBookChapterPanel destruction
////@end CBookChapterPanel destruction
}


/*
 * Member initialisation
 */

void CBookChapterPanel::Init()
{
////@begin CBookChapterPanel member initialisation
////@end CBookChapterPanel member initialisation
}


/*
 * Control creation for CBookChapterPanel
 */

void CBookChapterPanel::CreateControls()
{    
////@begin CBookChapterPanel content construction
    CBookChapterPanel* itemPanel1 = this;

    wxBoxSizer* itemBoxSizer2 = new wxBoxSizer(wxVERTICAL);
    itemPanel1->SetSizer(itemBoxSizer2);

    wxFlexGridSizer* itemFlexGridSizer3 = new wxFlexGridSizer(0, 2, 0, 0);
    itemBoxSizer2->Add(itemFlexGridSizer3, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

    wxStaticText* itemStaticText4 = new wxStaticText( itemPanel1, wxID_STATIC, _("Chapter Title"), wxDefaultPosition, wxDefaultSize, 0 );
    itemFlexGridSizer3->Add(itemStaticText4, 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxTextCtrl* itemTextCtrl5 = new wxTextCtrl( itemPanel1, ID_BOOKCHAPTER_CHAPTER_TITLE, wxEmptyString, wxDefaultPosition, wxSize(400, -1), wxTE_MULTILINE );
    itemFlexGridSizer3->Add(itemTextCtrl5, 0, wxGROW|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxStaticText* itemStaticText6 = new wxStaticText( itemPanel1, wxID_STATIC, _("Book Title"), wxDefaultPosition, wxDefaultSize, 0 );
    itemFlexGridSizer3->Add(itemStaticText6, 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxTextCtrl* itemTextCtrl7 = new wxTextCtrl( itemPanel1, ID_BOOKCHAPTER_BOOK_TITLE, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_MULTILINE );
    itemFlexGridSizer3->Add(itemTextCtrl7, 0, wxGROW|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxStaticText* itemStaticText8 = new wxStaticText( itemPanel1, wxID_STATIC, _("Pages"), wxDefaultPosition, wxDefaultSize, 0 );
    itemFlexGridSizer3->Add(itemStaticText8, 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxTextCtrl* itemTextCtrl9 = new wxTextCtrl( itemPanel1, ID_BOOKCHAPTER_PAGES, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
    itemFlexGridSizer3->Add(itemTextCtrl9, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxStaticText* itemStaticText10 = new wxStaticText( itemPanel1, wxID_STATIC, _("Publication Year"), wxDefaultPosition, wxDefaultSize, 0 );
    itemFlexGridSizer3->Add(itemStaticText10, 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxTextCtrl* itemTextCtrl11 = new wxTextCtrl( itemPanel1, ID_BOOKCHAPTER_PUB_YEAR, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
    itemFlexGridSizer3->Add(itemTextCtrl11, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxStaticText* itemStaticText12 = new wxStaticText( itemPanel1, wxID_STATIC, _("Copyright Year"), wxDefaultPosition, wxDefaultSize, 0 );
    itemFlexGridSizer3->Add(itemStaticText12, 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxTextCtrl* itemTextCtrl13 = new wxTextCtrl( itemPanel1, ID_BOOKCHAPTER_COPY_YEAR, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
    itemFlexGridSizer3->Add(itemTextCtrl13, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxALL, 5);

////@end CBookChapterPanel content construction
}


/*
 * Should we show tooltips?
 */

bool CBookChapterPanel::ShowToolTips()
{
    return true;
}

/*
 * Get bitmap resources
 */

wxBitmap CBookChapterPanel::GetBitmapResource( const wxString& name )
{
    // Bitmap retrieval
////@begin CBookChapterPanel bitmap retrieval
    wxUnusedVar(name);
    return wxNullBitmap;
////@end CBookChapterPanel bitmap retrieval
}

/*
 * Get icon resources
 */

wxIcon CBookChapterPanel::GetIconResource( const wxString& name )
{
    // Icon retrieval
////@begin CBookChapterPanel icon retrieval
    wxUnusedVar(name);
    return wxNullIcon;
////@end CBookChapterPanel icon retrieval
}
END_NCBI_SCOPE
