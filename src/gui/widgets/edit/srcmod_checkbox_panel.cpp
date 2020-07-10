/*  $Id: srcmod_checkbox_panel.cpp 29794 2014-02-12 20:13:29Z bollin $
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
 * Authors:  Colleen Bollin
 */



#include <ncbi_pch.hpp>



////@begin includes
////@end includes

#include <gui/widgets/edit/srcmod_checkbox_panel.hpp>
#include <wx/sizer.h>
#include <wx/icon.h>

////@begin XPM images
////@end XPM images

BEGIN_NCBI_SCOPE

/*
 * CSrcModCheckboxPanel type definition
 */

IMPLEMENT_DYNAMIC_CLASS( CSrcModCheckboxPanel, CSrcModEditPanel )


/*
 * CSrcModCheckboxPanel event table definition
 */

BEGIN_EVENT_TABLE( CSrcModCheckboxPanel, CSrcModEditPanel )

////@begin CSrcModCheckboxPanel event table entries
    EVT_CHECKBOX( ID_CHECKBOX, CSrcModCheckboxPanel::OnCheckboxClick )

////@end CSrcModCheckboxPanel event table entries

END_EVENT_TABLE()


/*
 * CSrcModCheckboxPanel constructors
 */

CSrcModCheckboxPanel::CSrcModCheckboxPanel() : CSrcModEditPanel(CSrcModEditPanel::eCheckbox)
{
    Init();
}

CSrcModCheckboxPanel::CSrcModCheckboxPanel( wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style )
: CSrcModEditPanel(CSrcModEditPanel::eCheckbox)
{
    Init();
    Create(parent, id, pos, size, style);
}


/*
 * CSrcModCheckboxPanel creator
 */

bool CSrcModCheckboxPanel::Create( wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style )
{
////@begin CSrcModCheckboxPanel creation
    CSrcModEditPanel::Create( parent, id, pos, size, style );

    CreateControls();
    if (GetSizer())
    {
        GetSizer()->SetSizeHints(this);
    }
    Centre();
////@end CSrcModCheckboxPanel creation
    return true;
}


/*
 * CSrcModCheckboxPanel destructor
 */

CSrcModCheckboxPanel::~CSrcModCheckboxPanel()
{
////@begin CSrcModCheckboxPanel destruction
////@end CSrcModCheckboxPanel destruction
}


/*
 * Member initialisation
 */

void CSrcModCheckboxPanel::Init()
{
////@begin CSrcModCheckboxPanel member initialisation
    m_Checkbox = NULL;
////@end CSrcModCheckboxPanel member initialisation
}


/*
 * Control creation for CSrcModCheckboxPanel
 */

void CSrcModCheckboxPanel::CreateControls()
{    
////@begin CSrcModCheckboxPanel content construction
    CSrcModCheckboxPanel* itemCSrcModEditPanel1 = this;

    wxBoxSizer* itemBoxSizer2 = new wxBoxSizer(wxVERTICAL);
    itemCSrcModEditPanel1->SetSizer(itemBoxSizer2);

    m_Checkbox = new wxCheckBox( itemCSrcModEditPanel1, ID_CHECKBOX, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
    m_Checkbox->SetValue(true);
    itemBoxSizer2->Add(m_Checkbox, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 0);

////@end CSrcModCheckboxPanel content construction
}


/*
 * Should we show tooltips?
 */

bool CSrcModCheckboxPanel::ShowToolTips()
{
    return true;
}

/*
 * Get bitmap resources
 */

wxBitmap CSrcModCheckboxPanel::GetBitmapResource( const wxString& name )
{
    // Bitmap retrieval
////@begin CSrcModCheckboxPanel bitmap retrieval
    wxUnusedVar(name);
    return wxNullBitmap;
////@end CSrcModCheckboxPanel bitmap retrieval
}

/*
 * Get icon resources
 */

wxIcon CSrcModCheckboxPanel::GetIconResource( const wxString& name )
{
    // Icon retrieval
////@begin CSrcModCheckboxPanel icon retrieval
    wxUnusedVar(name);
    return wxNullIcon;
////@end CSrcModCheckboxPanel icon retrieval
}


string CSrcModCheckboxPanel::GetValue()
{
    if (m_Checkbox->GetValue()) {
        return "TRUE";
    } else {
        return "";
    }
}


void CSrcModCheckboxPanel::SetValue(const string& val)
{
    if (NStr::EqualNocase(val, "TRUE")) {
        m_Checkbox->SetValue(true);
    } else {
        m_Checkbox->SetValue(false);
    }
}


bool CSrcModCheckboxPanel::IsWellFormatted(const string& val)
{
    if (NStr::IsBlank(val) || NStr::EqualNocase(val, "TRUE") || NStr::EqualNocase(val, "FALSE")) {
        return true;
    } else {
        return false;
    }
    return true;
}


/*
 * wxEVT_COMMAND_CHECKBOX_CLICKED event handler for ID_CHECKBOX
 */

void CSrcModCheckboxPanel::OnCheckboxClick( wxCommandEvent& event )
{
    x_NotifyParent();    
}


END_NCBI_SCOPE

