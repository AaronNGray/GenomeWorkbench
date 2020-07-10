/*  $Id: srcmod_text_panel.cpp 29061 2013-10-01 19:31:45Z bollin $
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

#include <gui/widgets/edit/srcmod_text_panel.hpp>
#include <gui/widgets/wx/wx_utils.hpp>
#include <wx/sizer.h>

////@begin XPM images
////@end XPM images

BEGIN_NCBI_SCOPE

/*
 * CSrcModTextPanel type definition
 */

IMPLEMENT_DYNAMIC_CLASS( CSrcModTextPanel, CSrcModEditPanel )


/*
 * CSrcModTextPanel event table definition
 */

BEGIN_EVENT_TABLE( CSrcModTextPanel, CSrcModEditPanel )

////@begin CSrcModTextPanel event table entries
    EVT_TEXT( ID_TEXTCTRL5, CSrcModTextPanel::OnTextctrl5TextUpdated )

////@end CSrcModTextPanel event table entries

END_EVENT_TABLE()


/*
 * CSrcModTextPanel constructors
 */

CSrcModTextPanel::CSrcModTextPanel() : CSrcModEditPanel(CSrcModEditPanel::eText)
{
    Init();
}

CSrcModTextPanel::CSrcModTextPanel( wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style )
    : CSrcModEditPanel(CSrcModEditPanel::eText)
{
    Init();
    Create(parent, id, pos, size, style);
}


/*
 * CSrcModTextPanel creator
 */

bool CSrcModTextPanel::Create( wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style )
{
////@begin CSrcModTextPanel creation
    CSrcModEditPanel::Create( parent, id, pos, size, style );

    CreateControls();
    if (GetSizer())
    {
        GetSizer()->SetSizeHints(this);
    }
    Centre();
////@end CSrcModTextPanel creation
    return true;
}


/*
 * CSrcModTextPanel destructor
 */

CSrcModTextPanel::~CSrcModTextPanel()
{
////@begin CSrcModTextPanel destruction
////@end CSrcModTextPanel destruction
}


/*
 * Member initialisation
 */

void CSrcModTextPanel::Init()
{
////@begin CSrcModTextPanel member initialisation
    m_Text = NULL;
////@end CSrcModTextPanel member initialisation
}


/*
 * Control creation for CSrcModTextPanel
 */

void CSrcModTextPanel::CreateControls()
{    
////@begin CSrcModTextPanel content construction
    CSrcModTextPanel* itemCSrcModEditPanel1 = this;

    wxBoxSizer* itemBoxSizer2 = new wxBoxSizer(wxVERTICAL);
    itemCSrcModEditPanel1->SetSizer(itemBoxSizer2);

    m_Text = new wxTextCtrl( itemCSrcModEditPanel1, ID_TEXTCTRL5, wxEmptyString, wxDefaultPosition, wxSize(320, -1), 0 );
    itemBoxSizer2->Add(m_Text, 0, wxGROW|wxALL, 0);

////@end CSrcModTextPanel content construction
}


/*
 * Should we show tooltips?
 */

bool CSrcModTextPanel::ShowToolTips()
{
    return true;
}

/*
 * Get bitmap resources
 */

wxBitmap CSrcModTextPanel::GetBitmapResource( const wxString& name )
{
    // Bitmap retrieval
////@begin CSrcModTextPanel bitmap retrieval
    wxUnusedVar(name);
    return wxNullBitmap;
////@end CSrcModTextPanel bitmap retrieval
}

/*
 * Get icon resources
 */

wxIcon CSrcModTextPanel::GetIconResource( const wxString& name )
{
    // Icon retrieval
////@begin CSrcModTextPanel icon retrieval
    wxUnusedVar(name);
    return wxNullIcon;
////@end CSrcModTextPanel icon retrieval
}


string CSrcModTextPanel::GetValue()
{
    return ToStdString(m_Text->GetValue());
}


void CSrcModTextPanel::SetValue(const string& val)
{
    m_Text->SetValue(ToWxString(val));
}


bool CSrcModTextPanel::IsWellFormatted(const string& val)
{
    return true;
}


/*
 * wxEVT_COMMAND_TEXT_UPDATED event handler for ID_TEXTCTRL5
 */

void CSrcModTextPanel::OnTextctrl5TextUpdated( wxCommandEvent& event )
{
    x_NotifyParent();
}


END_NCBI_SCOPE

