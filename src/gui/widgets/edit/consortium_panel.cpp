/*  $Id: consortium_panel.cpp 42706 2019-04-04 17:21:31Z asztalos $
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
#include <gui/utils/command_processor.hpp>
#include <gui/widgets/edit/edit_object.hpp>
#include <objects/biblio/Author.hpp>
#include <objects/general/Person_id.hpp>
#include <objects/general/Name_std.hpp>

#include <gui/widgets/edit/serial_member_primitive_validators.hpp>
#include <gui/widgets/wx/wx_utils.hpp>

////@begin includes
////@end includes

#include <wx/sizer.h>
#include <wx/bitmap.h>
#include <wx/icon.h>
#include <wx/listbook.h>
#include <wx/stattext.h>


#include <gui/widgets/edit/consortium_panel.hpp>
#include <gui/widgets/edit/authornames_panel.hpp>


BEGIN_NCBI_SCOPE
USING_SCOPE(objects);

////@begin XPM images
////@end XPM images


/*
 * CConsortiumPanel type definition
 */

IMPLEMENT_DYNAMIC_CLASS( CConsortiumPanel, wxPanel )


/*
 * CConsortiumPanel event table definition
 */

BEGIN_EVENT_TABLE( CConsortiumPanel, wxPanel )

////@begin CConsortiumPanel event table entries

////@end CConsortiumPanel event table entries

END_EVENT_TABLE()


/*
 * CConsortiumPanel constructors
 */

CConsortiumPanel::CConsortiumPanel()
{
    Init();
}

CConsortiumPanel::CConsortiumPanel( wxWindow* parent, CAuthor& author,
         wxWindowID id, const wxPoint& pos, const wxSize& size, long style ) :
     m_Author(0)
{
    Init();
    m_Author = new CAuthor();
    m_Author->Assign(author);
    m_Author->SetName().SetConsortium();
    Create(parent, id, pos, size, style);
}


/*
 * CConsortiumPanel creator
 */

bool CConsortiumPanel::Create( wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style )
{
////@begin CConsortiumPanel creation
    SetExtraStyle(wxWS_EX_BLOCK_EVENTS);
    wxPanel::Create( parent, id, pos, size, style );

    CreateControls();
    if (GetSizer())
    {
        GetSizer()->SetSizeHints(this);
    }
    Centre();
////@end CConsortiumPanel creation
    return true;
}


/*
 * CConsortiumPanel destructor
 */

CConsortiumPanel::~CConsortiumPanel()
{
////@begin CConsortiumPanel destruction
////@end CConsortiumPanel destruction
}


/*
 * Member initialisation
 */

void CConsortiumPanel::Init()
{
////@begin CConsortiumPanel member initialisation
    m_Consortium = NULL;
////@end CConsortiumPanel member initialisation
}



/*
 * Control creation for CConsortiumPanel
 */

void CConsortiumPanel::CreateControls()
{    
////@begin CConsortiumPanel content construction
    CConsortiumPanel* itemPanel1 = this;

    wxBoxSizer* itemBoxSizer2 = new wxBoxSizer(wxHORIZONTAL);
    itemPanel1->SetSizer(itemBoxSizer2);

    wxStaticText* itemStaticText3 = new wxStaticText( itemPanel1, wxID_STATIC, _("Consortium"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer2->Add(itemStaticText3, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    m_Consortium = new wxTextCtrl( itemPanel1, ID_TEXTCTRL, wxEmptyString, wxDefaultPosition, wxSize(200, -1), 0 );
    itemBoxSizer2->Add(m_Consortium, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);


////@end CConsortiumPanel content construction
}


bool CConsortiumPanel::TransferDataToWindow()
{
    if (!wxPanel::TransferDataToWindow())
        return false;

    if (m_Author->IsSetName() && m_Author->GetName().IsConsortium()) {
        m_Consortium->SetValue(ToWxString(m_Author->GetName().GetConsortium()));
    } else {
        m_Consortium->SetValue(wxEmptyString);
    }
    return true;
}


bool CConsortiumPanel::TransferDataFromWindow()
{
    if (!wxPanel::TransferDataFromWindow())
        return false;

    string consortium = ToAsciiStdString(m_Consortium->GetValue());
    m_Author->SetName().SetConsortium(consortium);
    return true;
}


CRef<CAuthor> CConsortiumPanel::GetAuthor() const
{
    if (m_Author)
        return m_Author;

    return CRef<CAuthor>();
}



/*
 * Should we show tooltips?
 */

bool CConsortiumPanel::ShowToolTips()
{
    return true;
}

/*
 * Get bitmap resources
 */

wxBitmap CConsortiumPanel::GetBitmapResource( const wxString& name )
{
    // Bitmap retrieval
////@begin CConsortiumPanel bitmap retrieval
    wxUnusedVar(name);
    return wxNullBitmap;
////@end CConsortiumPanel bitmap retrieval
}

/*
 * Get icon resources
 */

wxIcon CConsortiumPanel::GetIconResource( const wxString& name )
{
    // Icon retrieval
////@begin CConsortiumPanel icon retrieval
    wxUnusedVar(name);
    return wxNullIcon;
////@end CConsortiumPanel icon retrieval
}




END_NCBI_SCOPE
