/*  $Id: singleassemblyprogrampanel.cpp 26367 2012-09-05 12:56:41Z bollin $
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

#include <wx/stattext.h>
#include <gui/widgets/edit/singleassemblyprogrampanel.hpp>

////@begin XPM images
////@end XPM images

BEGIN_NCBI_SCOPE
USING_SCOPE(objects);

/*!
 * CSingleAssemblyProgramPanel type definition
 */

IMPLEMENT_DYNAMIC_CLASS( CSingleAssemblyProgramPanel, CAssemblyProgramPanel )


/*!
 * CSingleAssemblyProgramPanel event table definition
 */

BEGIN_EVENT_TABLE( CSingleAssemblyProgramPanel, CAssemblyProgramPanel )

////@begin CSingleAssemblyProgramPanel event table entries
////@end CSingleAssemblyProgramPanel event table entries

END_EVENT_TABLE()


/*!
 * CSingleAssemblyProgramPanel constructors
 */

CSingleAssemblyProgramPanel::CSingleAssemblyProgramPanel()
{
    Init();
}

CSingleAssemblyProgramPanel::CSingleAssemblyProgramPanel( wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style )
{
    Init();
    Create(parent, id, pos, size, style);
}


/*!
 * CSingleAssemblyProgramPanel creator
 */

bool CSingleAssemblyProgramPanel::Create( wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style )
{
////@begin CSingleAssemblyProgramPanel creation
    CAssemblyProgramPanel::Create( parent, id, pos, size, style );

    CreateControls();
    if (GetSizer())
    {
        GetSizer()->SetSizeHints(this);
    }
    Centre();
////@end CSingleAssemblyProgramPanel creation
    return true;
}


/*!
 * CSingleAssemblyProgramPanel destructor
 */

CSingleAssemblyProgramPanel::~CSingleAssemblyProgramPanel()
{
////@begin CSingleAssemblyProgramPanel destruction
////@end CSingleAssemblyProgramPanel destruction
}


/*!
 * Member initialisation
 */

void CSingleAssemblyProgramPanel::Init()
{
////@begin CSingleAssemblyProgramPanel member initialisation
    m_Program = NULL;
    m_Version = NULL;
////@end CSingleAssemblyProgramPanel member initialisation
}


/*!
 * Control creation for CSingleAssemblyProgramPanel
 */

void CSingleAssemblyProgramPanel::CreateControls()
{    
////@begin CSingleAssemblyProgramPanel content construction
    CSingleAssemblyProgramPanel* itemCAssemblyProgramPanel1 = this;

    wxFlexGridSizer* itemFlexGridSizer2 = new wxFlexGridSizer(2, 2, 0, 0);
    itemCAssemblyProgramPanel1->SetSizer(itemFlexGridSizer2);

    wxStaticText* itemStaticText3 = new wxStaticText( itemCAssemblyProgramPanel1, wxID_STATIC, _("Assembly program (required):"), wxDefaultPosition, wxDefaultSize, 0 );
    itemFlexGridSizer2->Add(itemStaticText3, 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    m_Program = new wxTextCtrl( itemCAssemblyProgramPanel1, ID_TEXTCTRL17, wxEmptyString, wxDefaultPosition, wxSize(150, -1), 0 );
    itemFlexGridSizer2->Add(m_Program, 0, wxGROW|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxStaticText* itemStaticText5 = new wxStaticText( itemCAssemblyProgramPanel1, wxID_STATIC, _("Version or date (required):"), wxDefaultPosition, wxDefaultSize, 0 );
    itemFlexGridSizer2->Add(itemStaticText5, 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    m_Version = new wxTextCtrl( itemCAssemblyProgramPanel1, ID_TEXTCTRL18, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
    itemFlexGridSizer2->Add(m_Version, 0, wxGROW|wxALIGN_CENTER_VERTICAL|wxALL, 5);

////@end CSingleAssemblyProgramPanel content construction
}


void CSingleAssemblyProgramPanel::SetValue( const string& val)
{
    size_t pos = NStr::Find (val, " v.");
    
    if (pos == string::npos) {
        m_Program->SetValue (ToWxString (val));
        m_Version->SetValue (wxEmptyString);
    } else {
        string program = val.substr (0, pos);
        string version = val.substr (pos + 3);
        NStr::TruncateSpacesInPlace (program);
        NStr::TruncateSpacesInPlace (version);
        m_Program->SetValue (program);
        m_Version->SetValue (version);
    }
}


string CSingleAssemblyProgramPanel::GetValue ()
{
    string program = ToStdString (m_Program->GetValue());
    string version = ToStdString (m_Version->GetValue());
    NStr::TruncateSpacesInPlace (program);
    NStr::TruncateSpacesInPlace (version);

    if (!NStr::IsBlank (version)) {
        program += " v. ";
        program += version;
    }

    return program;
}


/*!
 * Should we show tooltips?
 */

bool CSingleAssemblyProgramPanel::ShowToolTips()
{
    return true;
}

/*!
 * Get bitmap resources
 */

wxBitmap CSingleAssemblyProgramPanel::GetBitmapResource( const wxString& name )
{
    // Bitmap retrieval
////@begin CSingleAssemblyProgramPanel bitmap retrieval
    wxUnusedVar(name);
    return wxNullBitmap;
////@end CSingleAssemblyProgramPanel bitmap retrieval
}

/*!
 * Get icon resources
 */

wxIcon CSingleAssemblyProgramPanel::GetIconResource( const wxString& name )
{
    // Icon retrieval
////@begin CSingleAssemblyProgramPanel icon retrieval
    wxUnusedVar(name);
    return wxNullIcon;
////@end CSingleAssemblyProgramPanel icon retrieval
}

END_NCBI_SCOPE
