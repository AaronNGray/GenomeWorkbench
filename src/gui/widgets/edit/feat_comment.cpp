/*  $Id: feat_comment.cpp 39032 2017-07-21 16:04:03Z filippov $
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
 * Authors:  Igor Filippov
 */


#include <ncbi_pch.hpp>

////@begin includes
////@end includes

#include "feat_comment.hpp"

#include <wx/sizer.h>
#include <wx/stattext.h>
#include <wx/bitmap.h>
#include <wx/icon.h>

BEGIN_NCBI_SCOPE

////@begin XPM images
////@end XPM images


/*!
 * CFeatCommentPanel type definition
 */

IMPLEMENT_DYNAMIC_CLASS( CFeatCommentPanel, wxPanel )


/*!
 * CFeatCommentPanel event table definition
 */

BEGIN_EVENT_TABLE( CFeatCommentPanel, wxPanel )

////@begin CFeatCommentPanel event table entries
////@end CFeatCommentPanel event table entries

END_EVENT_TABLE()


/*!
 * CFeatCommentPanel constructors
 */

CFeatCommentPanel::CFeatCommentPanel() 
{
    Init();
}

CFeatCommentPanel::CFeatCommentPanel( wxWindow* parent, const wxString& comment,
                              wxWindowID id, const wxPoint& pos, const wxSize& size, long style )
    : m_comment(comment)
{
    Init();
    Create(parent, id, pos, size, style);
}


/*!
 * Commentpanel creator
 */

bool CFeatCommentPanel::Create( wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style )
{
////@begin CFeatCommentPanel creation
    wxPanel::Create( parent, id, pos, size, style );

    CreateControls();
    if (GetSizer())
    {
        GetSizer()->SetSizeHints(this);
    }
    Centre();
////@end CFeatCommentPanel creation
    return true;
}


/*!
 * CFeatCommentPanel destructor
 */

CFeatCommentPanel::~CFeatCommentPanel()
{
////@begin CFeatCommentPanel destruction
////@end CFeatCommentPanel destruction
}


/*!
 * Member initialisation
 */

void CFeatCommentPanel::Init()
{
////@begin CFeatCommentPanel member initialisation
    m_ConvertToMiscFeat = NULL;
////@end CFeatCommentPanel member initialisation
}


/*!
 * Control creation for Commentpanel
 */

void CFeatCommentPanel::CreateControls()
{
////@begin CFeatCommentPanel content construction
    CFeatCommentPanel* itemPanel1 = this;

    wxBoxSizer* itemBoxSizer2 = new wxBoxSizer(wxVERTICAL);
    itemPanel1->SetSizer(itemBoxSizer2);

    wxStaticText* itemStaticText1 = new wxStaticText( itemPanel1, wxID_STATIC, m_comment, wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer2->Add(itemStaticText1, 0, wxALIGN_LEFT|wxALL, 5);

    m_ConvertToMiscFeat = new wxCheckBox( itemPanel1, wxID_ANY, _("Convert to misc-feat"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer2->Add(m_ConvertToMiscFeat, 0, wxALIGN_LEFT|wxALL, 5);

////@end CFeatCommentPanel content construction
}


/*!
 * Should we show tooltips?
 */

bool CFeatCommentPanel::ShowToolTips()
{
    return true;
}

/*!
 * Get bitmap resources
 */

wxBitmap CFeatCommentPanel::GetBitmapResource( const wxString& name )
{
    // Bitmap retrieval
////@begin CFeatCommentPanel bitmap retrieval
    wxUnusedVar(name);
    return wxNullBitmap;
////@end CFeatCommentPanel bitmap retrieval
}

/*!
 * Get icon resources
 */

wxIcon CFeatCommentPanel::GetIconResource( const wxString& name )
{
    // Icon retrieval
////@begin CFeatCommentPanel icon retrieval
    wxUnusedVar(name);
    return wxNullIcon;
////@end CFeatCommentPanel icon retrieval
}


END_NCBI_SCOPE

