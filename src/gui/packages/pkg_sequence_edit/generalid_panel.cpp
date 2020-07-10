/*  $Id: generalid_panel.cpp 34886 2016-02-25 22:31:14Z asztalos $
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
#include <gui/packages/pkg_sequence_edit/generalid_panel.hpp>
#include <gui/packages/pkg_sequence_edit/miscedit_util.hpp>
#include <gui/widgets/wx/wx_utils.hpp>
#include <wx/sizer.h>
#include <wx/icon.h>
#include <wx/choice.h>
#include <wx/stattext.h>
#include <wx/textctrl.h>


BEGIN_NCBI_SCOPE

/*!
 * CGeneralIDPanel type definition
 */

IMPLEMENT_DYNAMIC_CLASS( CGeneralIDPanel, CFieldNamePanel )


/*!
 * CGeneralIDPanel event table definition
 */

BEGIN_EVENT_TABLE( CGeneralIDPanel, CFieldNamePanel )

////@begin CGeneralIDPanel event table entries
    EVT_CHOICE( ID_CGENERALID_CHOICE, CGeneralIDPanel::OnGeneralIDChoice )
////@end CGeneralIDPanel event table entries

END_EVENT_TABLE()


/*!
 * CGeneralIDPanel constructors
 */

CGeneralIDPanel::CGeneralIDPanel()
{
    Init();
}

CGeneralIDPanel::CGeneralIDPanel( wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style )
{
    Init();
    Create(parent, id, pos, size, style);
}


/*!
 * CGeneralIDPanel creator
 */

bool CGeneralIDPanel::Create( wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style )
{
////@begin CGeneralIDPanel creation
    CFieldNamePanel::Create(parent, id, pos, size, style);

    CreateControls();
    if (GetSizer())
    {
        GetSizer()->SetSizeHints(this);
    }
    Centre();
////@end CGeneralIDPanel creation
    return true;
}


/*!
 * CGeneralIDPanel destructor
 */

CGeneralIDPanel::~CGeneralIDPanel()
{
////@begin CGeneralIDPanel destruction
////@end CGeneralIDPanel destruction
}


/*!
 * Member initialisation
 */

void CGeneralIDPanel::Init()
{
////@begin CGeneralIDPanel member initialisation
    m_GenID = NULL;
    m_Dblabel = NULL;
    m_DbName = NULL;
////@end CGeneralIDPanel member initialisation
}


/*!
 * Control creation for CGeneralIDPanel
 */

void CGeneralIDPanel::CreateControls()
{    
////@begin CGeneralIDPanel content construction
    CGeneralIDPanel* itemPanel1 = this;

    wxBoxSizer* itemBoxSizer2 = new wxBoxSizer(wxVERTICAL);
    itemPanel1->SetSizer(itemBoxSizer2);

    wxBoxSizer* itemBoxSizer3 = new wxBoxSizer(wxHORIZONTAL);
    itemBoxSizer2->Add(itemBoxSizer3, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

    wxArrayString m_GenIDStrings;
    m_GenID = new wxChoice( itemPanel1, ID_CGENERALID_CHOICE, wxDefaultPosition, wxDefaultSize, m_GenIDStrings, 0 );
    itemBoxSizer3->Add(m_GenID, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    m_Dblabel = new wxStaticText( itemPanel1, wxID_STATIC, _("Db:"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer3->Add(m_Dblabel, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    m_DbName = new wxTextCtrl( itemPanel1, ID_CGENERALID_TEXT, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer3->Add(m_DbName, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    vector<string> options = x_GetStrings();
    ToArrayString(options, m_GenIDStrings);
    m_GenID->Append(m_GenIDStrings);
    m_GenID->SetSelection(0);

    m_Dblabel->Enable(false);
    m_DbName->Enable(false);


////@end CGeneralIDPanel content construction
}


vector<string> CGeneralIDPanel::x_GetStrings()
{
    vector<string> options;
    unsigned int part = CGeneralIDField::eGenId;
    for ( ; part <= CGeneralIDField::eGenIdTag; ++part) {
        options.push_back(CGeneralIDField::GetName_GeneralIDPart((CGeneralIDField::EGeneralIDPart)part));
    }

    return options;
}

/*!
 * Should we show tooltips?
 */

bool CGeneralIDPanel::ShowToolTips()
{
    return true;
}

/*!
 * Get bitmap resources
 */

wxBitmap CGeneralIDPanel::GetBitmapResource( const wxString& name )
{
    // Bitmap retrieval
////@begin CGeneralIDPanel bitmap retrieval
    wxUnusedVar(name);
    return wxNullBitmap;
////@end CGeneralIDPanel bitmap retrieval
}

/*!
 * Get icon resources
 */

wxIcon CGeneralIDPanel::GetIconResource( const wxString& name )
{
    // Icon retrieval
////@begin CGeneralIDPanel icon retrieval
    wxUnusedVar(name);
    return wxNullIcon;
////@end CGeneralIDPanel icon retrieval
}


/*!
 * wxEVT_COMMAND_CHOICE_SELECTED event handler for ID_CGENERALID_CHOICE
 */

void CGeneralIDPanel::OnGeneralIDChoice( wxCommandEvent& event )
{
    string selection =ToStdString(m_GenID->GetStringSelection());
    if (NStr::EqualNocase(selection, CGeneralIDField::GetName_GeneralIDPart(CGeneralIDField::eGenIdTag))) {
        m_Dblabel->Enable(true);
        m_DbName->Enable(true);
    } else {
        m_Dblabel->Enable(false);
        m_DbName->Enable(false);
    }
    event.Skip();
}

string CGeneralIDPanel::GetFieldName(const bool subfield)
{
    string field = kEmptyStr;
    int val = m_GenID->GetSelection();
    if (val > -1) {
        field = ToStdString(m_GenID->GetString(val));
        if (m_DbName->IsShown() && wxEmptyString != m_DbName->GetValue()) {
            field = field + " " + ToStdString(m_DbName->GetValue());
        }
    }
    return field;
}

bool CGeneralIDPanel::SetFieldName(const string& field)
{
    return true;
}

END_NCBI_SCOPE

