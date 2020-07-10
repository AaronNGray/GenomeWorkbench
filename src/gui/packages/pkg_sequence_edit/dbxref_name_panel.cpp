/*  $Id: dbxref_name_panel.cpp 33539 2015-08-11 16:07:13Z asztalos $
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
#include <gui/packages/pkg_sequence_edit/dbxref_name_panel.hpp>
#include <gui/widgets/wx/wx_utils.hpp>
#include <wx/sizer.h>
#include <wx/stattext.h>
#include <wx/textctrl.h>

BEGIN_NCBI_SCOPE

/*!
 * CDbxrefNamePanel type definition
 */

IMPLEMENT_DYNAMIC_CLASS( CDbxrefNamePanel, CFieldNamePanel )


/*!
 * CDbxrefNamePanel event table definition
 */

BEGIN_EVENT_TABLE( CDbxrefNamePanel, CFieldNamePanel )

////@begin CDbxrefNamePanel event table entries
////@end CDbxrefNamePanel event table entries

END_EVENT_TABLE()


/*!
 * CDbxrefNamePanel constructors
 */

CDbxrefNamePanel::CDbxrefNamePanel()
{
    Init();
}

CDbxrefNamePanel::CDbxrefNamePanel( wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style )
{
    Init();
    Create(parent, id, pos, size, style);
}


/*!
 * CDbxrefNamePanel creator
 */

bool CDbxrefNamePanel::Create( wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style )
{
////@begin CDbxrefNamePanel creation
    CFieldNamePanel::Create( parent, id, pos, size, style );

    CreateControls();
    if (GetSizer())
    {
        GetSizer()->SetSizeHints(this);
    }
    Centre();
////@end CDbxrefNamePanel creation
    return true;
}


/*!
 * CDbxrefNamePanel destructor
 */

CDbxrefNamePanel::~CDbxrefNamePanel()
{
////@begin CDbxrefNamePanel destruction
////@end CDbxrefNamePanel destruction
}


/*!
 * Member initialisation
 */

void CDbxrefNamePanel::Init()
{
////@begin CDbxrefNamePanel member initialisation
    m_DbName = NULL;
////@end CDbxrefNamePanel member initialisation
}


/*!
 * Control creation for CDbxrefNamePanel
 */

void CDbxrefNamePanel::CreateControls()
{    
////@begin CDbxrefNamePanel content construction
    CDbxrefNamePanel* itemCFieldNamePanel1 = this;

    wxBoxSizer* itemBoxSizer2 = new wxBoxSizer(wxVERTICAL);
    itemCFieldNamePanel1->SetSizer(itemBoxSizer2);

    wxBoxSizer* itemBoxSizer3 = new wxBoxSizer(wxHORIZONTAL);
    itemBoxSizer2->Add(itemBoxSizer3, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

    wxStaticText* itemStaticText4 = new wxStaticText( itemCFieldNamePanel1, wxID_STATIC, _("Db:"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer3->Add(itemStaticText4, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    m_DbName = new wxTextCtrl( itemCFieldNamePanel1, ID_TXTCTRl_DBXREF, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer3->Add(m_DbName, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

////@end CDbxrefNamePanel content construction
}


/*!
 * Should we show tooltips?
 */

bool CDbxrefNamePanel::ShowToolTips()
{
    return true;
}

/*!
 * Get bitmap resources
 */

wxBitmap CDbxrefNamePanel::GetBitmapResource( const wxString& name )
{
    // Bitmap retrieval
////@begin CDbxrefNamePanel bitmap retrieval
    wxUnusedVar(name);
    return wxNullBitmap;
////@end CDbxrefNamePanel bitmap retrieval
}

/*!
 * Get icon resources
 */

wxIcon CDbxrefNamePanel::GetIconResource( const wxString& name )
{
    // Icon retrieval
////@begin CDbxrefNamePanel icon retrieval
    wxUnusedVar(name);
    return wxNullIcon;
////@end CDbxrefNamePanel icon retrieval
}

string CDbxrefNamePanel::GetFieldName(const bool subfield)
{
    return "Dbxref " + ToStdString(m_DbName->GetValue());
}


bool CDbxrefNamePanel::SetFieldName(const string& field)
{
    return true;
}


END_NCBI_SCOPE