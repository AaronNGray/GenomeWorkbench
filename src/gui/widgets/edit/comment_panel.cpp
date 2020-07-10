/*  $Id: comment_panel.cpp 43573 2019-08-01 16:28:35Z filippov $
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

////@begin includes
////@end includes

#include "comment_panel.hpp"

#include <gui/widgets/edit/serial_member_primitive_validators.hpp>
#include <gui/widgets/wx/richtextctrl.hpp>

#include <wx/sizer.h>
#include <wx/textctrl.h>
#include <wx/bitmap.h>
#include <wx/icon.h>
#include <wx/button.h>

BEGIN_NCBI_SCOPE

////@begin XPM images
////@end XPM images


/*!
 * CCommentPanel type definition
 */

IMPLEMENT_DYNAMIC_CLASS( CCommentPanel, wxPanel )


/*!
 * CCommentPanel event table definition
 */

BEGIN_EVENT_TABLE( CCommentPanel, wxPanel )

////@begin CCommentPanel event table entries
    EVT_BUTTON( ID_CLEAR_COMMENT_BTN, CCommentPanel::OnClearCommentBtnClick )

////@end CCommentPanel event table entries

END_EVENT_TABLE()


/*!
 * CCommentPanel constructors
 */

CCommentPanel::CCommentPanel() : m_Object(0)
{
    Init();
}

CCommentPanel::CCommentPanel( wxWindow* parent, CSerialObject& object, 
                              wxWindowID id, const wxPoint& pos, const wxSize& size, long style )
    : m_Object(&object)
{
    Init();
    Create(parent, id, pos, size, style);
}


/*!
 * Commentpanel creator
 */

bool CCommentPanel::Create( wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style )
{
////@begin CCommentPanel creation
    wxPanel::Create( parent, id, pos, size, style );

    CreateControls();
    if (GetSizer())
    {
        GetSizer()->SetSizeHints(this);
    }
    Centre();
////@end CCommentPanel creation
    return true;
}


/*!
 * CCommentPanel destructor
 */

CCommentPanel::~CCommentPanel()
{
////@begin CCommentPanel destruction
////@end CCommentPanel destruction
}


/*!
 * Member initialisation
 */

void CCommentPanel::Init()
{
////@begin CCommentPanel member initialisation
    m_CommentCtrl = NULL;
////@end CCommentPanel member initialisation
}


/*!
 * Control creation for Commentpanel
 */

void CCommentPanel::CreateControls()
{
////@begin CCommentPanel content construction
    CCommentPanel* itemPanel1 = this;

    wxBoxSizer* itemBoxSizer2 = new wxBoxSizer(wxVERTICAL);
    itemPanel1->SetSizer(itemBoxSizer2);

    wxBoxSizer* itemBoxSizer3 = new wxBoxSizer(wxHORIZONTAL);
    itemBoxSizer2->Add(itemBoxSizer3, 1, wxGROW|wxALL, 0);

    m_CommentCtrl = new CRichTextCtrl( itemPanel1, ID_TEXTCTRL7, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_MULTILINE );
    itemBoxSizer3->Add(m_CommentCtrl, 1, wxGROW|wxALL, 0);

    wxButton* itemButton5 = new wxButton( itemPanel1, ID_CLEAR_COMMENT_BTN, _("Clear"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer2->Add(itemButton5, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

    // Set validators
    m_CommentCtrl->SetValidator( CSerialTextValidator(*m_Object, "comment") );
////@end CCommentPanel content construction
}


/*!
 * Should we show tooltips?
 */

bool CCommentPanel::ShowToolTips()
{
    return true;
}

/*!
 * Get bitmap resources
 */

wxBitmap CCommentPanel::GetBitmapResource( const wxString& name )
{
    // Bitmap retrieval
////@begin CCommentPanel bitmap retrieval
    wxUnusedVar(name);
    return wxNullBitmap;
////@end CCommentPanel bitmap retrieval
}

/*!
 * Get icon resources
 */

wxIcon CCommentPanel::GetIconResource( const wxString& name )
{
    // Icon retrieval
////@begin CCommentPanel icon retrieval
    wxUnusedVar(name);
    return wxNullIcon;
////@end CCommentPanel icon retrieval
}


/*!
 * wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_CLEAR_COMMENT_BTN
 */

void CCommentPanel::OnClearCommentBtnClick( wxCommandEvent& event )
{
    m_CommentCtrl->SetValue(wxEmptyString);
}


END_NCBI_SCOPE

