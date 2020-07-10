/*  $Id: convert_text_options_panel.cpp 30522 2014-06-06 18:47:41Z asztalos $
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
 * Authors:  Andrea Asztalos
 */


#include <ncbi_pch.hpp>
#include <gui/packages/pkg_sequence_edit/convert_text_options_panel.hpp>
#include <gui/packages/pkg_sequence_edit/cap_change_panel.hpp>
#include <wx/sizer.h>
#include <wx/checkbox.h>
#include <wx/icon.h>

BEGIN_NCBI_SCOPE
USING_SCOPE(ncbi::objects);

/*!
 * CConvertTextOptionsPanel type definition
 */

IMPLEMENT_DYNAMIC_CLASS( CConvertTextOptionsPanel, wxPanel )


/*!
 * CConvertTextOptionsPanel event table definition
 */

BEGIN_EVENT_TABLE( CConvertTextOptionsPanel, wxPanel )

////@begin CConvertTextOptionsPanel event table entries
////@end CConvertTextOptionsPanel event table entries

END_EVENT_TABLE()


/*!
 * CConvertTextOptionsPanel constructors
 */

CConvertTextOptionsPanel::CConvertTextOptionsPanel()
{
    Init();
}

CConvertTextOptionsPanel::CConvertTextOptionsPanel( wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style )
{
    Init();
    Create(parent, id, pos, size, style);
}


/*!
 * CConvertTextOptionsPanel creator
 */

bool CConvertTextOptionsPanel::Create( wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style )
{
////@begin CConvertTextOptionsPanel creation
    wxPanel::Create( parent, id, pos, size, style );

    CreateControls();
    if (GetSizer())
    {
        GetSizer()->SetSizeHints(this);
    }
    Centre();
////@end CConvertTextOptionsPanel creation
    return true;
}


/*!
 * CConvertTextOptionsPanel destructor
 */

CConvertTextOptionsPanel::~CConvertTextOptionsPanel()
{
////@begin CConvertTextOptionsPanel destruction
////@end CConvertTextOptionsPanel destruction
}


/*!
 * Member initialisation
 */

void CConvertTextOptionsPanel::Init()
{
////@begin CConvertTextOptionsPanel member initialisation
    m_LeaveOriginal = NULL;
    m_StripNameFromText = NULL;
    m_CapChangeOptions = NULL;
////@end CConvertTextOptionsPanel member initialisation
}


/*!
 * Control creation for CConvertTextOptionsPanel
 */

void CConvertTextOptionsPanel::CreateControls()
{    
////@begin CConvertTextOptionsPanel content construction
    CConvertTextOptionsPanel* itemPanel1 = this;

    wxBoxSizer* itemBoxSizer2 = new wxBoxSizer(wxVERTICAL);
    itemPanel1->SetSizer(itemBoxSizer2);

    wxBoxSizer* itemBoxSizer3 = new wxBoxSizer(wxHORIZONTAL);
    itemBoxSizer2->Add(itemBoxSizer3, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

    m_LeaveOriginal = new wxCheckBox( itemPanel1, ID_LVORIG_CHKBX, _("Leave on original"), wxDefaultPosition, wxDefaultSize, 0 );
    m_LeaveOriginal->SetValue(false);
    itemBoxSizer3->Add(m_LeaveOriginal, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    m_StripNameFromText = new wxCheckBox( itemPanel1, ID_RMVNAME_CHKBX, _("Strip name from text"), wxDefaultPosition, wxDefaultSize, 0 );
    m_StripNameFromText->SetValue(false);
    itemBoxSizer3->Add(m_StripNameFromText, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    m_CapChangeOptions = new CCapChangePanel( itemPanel1, ID_CAPCHNGPANEL, wxDefaultPosition, wxDefaultSize, wxSUNKEN_BORDER|wxTAB_TRAVERSAL );
    itemBoxSizer2->Add(m_CapChangeOptions, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

////@end CConvertTextOptionsPanel content construction
}


/*!
 * Should we show tooltips?
 */

bool CConvertTextOptionsPanel::ShowToolTips()
{
    return true;
}

/*!
 * Get bitmap resources
 */

wxBitmap CConvertTextOptionsPanel::GetBitmapResource( const wxString& name )
{
    // Bitmap retrieval
////@begin CConvertTextOptionsPanel bitmap retrieval
    wxUnusedVar(name);
    return wxNullBitmap;
////@end CConvertTextOptionsPanel bitmap retrieval
}

/*!
 * Get icon resources
 */

wxIcon CConvertTextOptionsPanel::GetIconResource( const wxString& name )
{
    // Icon retrieval
////@begin CConvertTextOptionsPanel icon retrieval
    wxUnusedVar(name);
    return wxNullIcon;
////@end CConvertTextOptionsPanel icon retrieval
}

END_NCBI_SCOPE