/*  $Id: gbqualdirection_panel.cpp 25028 2012-01-12 17:26:13Z katargir $
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
#include <gui/widgets/wx/wx_utils.hpp>
////@begin includes
////@end includes

#include "gbqualdirection_panel.hpp"

////@begin XPM images
////@end XPM images

BEGIN_NCBI_SCOPE

/*!
 * CGBQualDirectionPanel type definition
 */

IMPLEMENT_DYNAMIC_CLASS( CGBQualDirectionPanel, CFormattedQualPanel )


/*!
 * CGBQualDirectionPanel event table definition
 */

BEGIN_EVENT_TABLE( CGBQualDirectionPanel, CFormattedQualPanel )

////@begin CGBQualDirectionPanel event table entries
////@end CGBQualDirectionPanel event table entries

END_EVENT_TABLE()


/*!
 * CGBQualDirectionPanel constructors
 */

CGBQualDirectionPanel::CGBQualDirectionPanel()
{
    Init();
}

CGBQualDirectionPanel::CGBQualDirectionPanel( wxWindow* parent, wxWindowID id, const wxString& caption, const wxPoint& pos, const wxSize& size, long style )
{
    Init();
    Create(parent, id, caption, pos, size, style);
}


/*!
 * CGBQualDirectionPanel creator
 */

bool CGBQualDirectionPanel::Create( wxWindow* parent, wxWindowID id, const wxString& caption, const wxPoint& pos, const wxSize& size, long style )
{
////@begin CGBQualDirectionPanel creation
    SetExtraStyle(wxWS_EX_BLOCK_EVENTS);
    CFormattedQualPanel::Create( parent, id, pos, size, style );

    CreateControls();
    if (GetSizer())
    {
        GetSizer()->SetSizeHints(this);
    }
    Centre();
////@end CGBQualDirectionPanel creation
    return true;
}


/*!
 * CGBQualDirectionPanel destructor
 */

CGBQualDirectionPanel::~CGBQualDirectionPanel()
{
////@begin CGBQualDirectionPanel destruction
////@end CGBQualDirectionPanel destruction
}


/*!
 * Member initialisation
 */

void CGBQualDirectionPanel::Init()
{
////@begin CGBQualDirectionPanel member initialisation
    m_DirectionCtrl = NULL;
////@end CGBQualDirectionPanel member initialisation
}


/*!
 * Control creation for CGBQualDirectionPanel
 */

void CGBQualDirectionPanel::CreateControls()
{    
////@begin CGBQualDirectionPanel content construction
    CGBQualDirectionPanel* itemCFormattedQualPanel1 = this;

    wxBoxSizer* itemBoxSizer2 = new wxBoxSizer(wxVERTICAL);
    itemCFormattedQualPanel1->SetSizer(itemBoxSizer2);

    wxArrayString m_DirectionCtrlStrings;
    m_DirectionCtrlStrings.Add(wxEmptyString);
    m_DirectionCtrlStrings.Add(_("LEFT"));
    m_DirectionCtrlStrings.Add(_("RIGHT"));
    m_DirectionCtrlStrings.Add(_("BOTH"));
    m_DirectionCtrl = new wxChoice( itemCFormattedQualPanel1, ID_CHOICE4, wxDefaultPosition, wxDefaultSize, m_DirectionCtrlStrings, 0 );
    itemBoxSizer2->Add(m_DirectionCtrl, 0, wxALIGN_LEFT|wxALL, 5);

////@end CGBQualDirectionPanel content construction
}


void CGBQualDirectionPanel::SetValue(string val)
{
    string tmp = NStr::ToUpper(val);
    if (!m_DirectionCtrl->SetStringSelection (ToWxString(tmp))) {
        m_DirectionCtrl->Append(ToWxString(val));
        m_DirectionCtrl->SetStringSelection (ToWxString(val));        
    }
}


string CGBQualDirectionPanel::GetValue()
{
    string val = ToStdString(m_DirectionCtrl->GetStringSelection());
    return val;
}


/*!
 * Should we show tooltips?
 */

bool CGBQualDirectionPanel::ShowToolTips()
{
    return true;
}

/*!
 * Get bitmap resources
 */

wxBitmap CGBQualDirectionPanel::GetBitmapResource( const wxString& name )
{
    // Bitmap retrieval
////@begin CGBQualDirectionPanel bitmap retrieval
    wxUnusedVar(name);
    return wxNullBitmap;
////@end CGBQualDirectionPanel bitmap retrieval
}

/*!
 * Get icon resources
 */

wxIcon CGBQualDirectionPanel::GetIconResource( const wxString& name )
{
    // Icon retrieval
////@begin CGBQualDirectionPanel icon retrieval
    wxUnusedVar(name);
    return wxNullIcon;
////@end CGBQualDirectionPanel icon retrieval
}

END_NCBI_SCOPE
