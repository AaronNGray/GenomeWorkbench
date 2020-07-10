/*  $Id: locus_tag_euk_dlg.cpp 43444 2019-07-01 15:47:55Z filippov $
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
#include <gui/packages/pkg_sequence_edit/locus_tag_euk_dlg.hpp>

#include <wx/sizer.h>
#include <wx/stattext.h>
#include <wx/button.h>



BEGIN_NCBI_SCOPE
/*!
 * CLocusTagEukDlg type definition
 */

IMPLEMENT_DYNAMIC_CLASS( CLocusTagEukDlg, wxDialog )


/*!
 * CLocusTagEukDlg event table definition
 */

BEGIN_EVENT_TABLE( CLocusTagEukDlg, wxDialog )

////@begin CLocusTagEukDlg event table entries

////@end CLocusTagEukDlg event table entries

END_EVENT_TABLE()


/*!
 * CLocusTagEukDlg constructors
 */

CLocusTagEukDlg::CLocusTagEukDlg()
{
    Init();
}

CLocusTagEukDlg::CLocusTagEukDlg( wxWindow* parent, const string &locus_tag,
        wxWindowID id, const wxString& caption, const wxPoint& pos, const wxSize& size, long style)
    : m_locus_tag_value(locus_tag)
{
    Init();
    Create(parent, id, caption, pos, size, style);
}


/*!
 * CLocusTagEukDlg creator
 */

bool CLocusTagEukDlg::Create( wxWindow* parent, wxWindowID id, const wxString& caption, const wxPoint& pos, const wxSize& size, long style )
{
////@begin CLocusTagEukDlg creation
    SetExtraStyle(wxWS_EX_BLOCK_EVENTS);
    wxDialog::Create( parent, id, caption, pos, size, style );

    CreateControls();
    if (GetSizer())
    {
        GetSizer()->SetSizeHints(this);
    }
    Centre();
////@end CLocusTagEukDlg creation
    return true;
}


/*!
 * CLocusTagEukDlg destructor
 */

CLocusTagEukDlg::~CLocusTagEukDlg()
{
////@begin CLocusTagEukDlg destruction
////@end CLocusTagEukDlg destruction
}


/*!
 * Member initialisation
 */

void CLocusTagEukDlg::Init()
{
////@begin CLocusTagEukDlg member initialisation
    m_LocusTag = nullptr;
    m_Euk = nullptr;
////@end CLocusTagEukDlg member initialisation
}

/*!
 * Control creation for CLocusTagEukDlg
 */


void CLocusTagEukDlg::CreateControls()
{    
////@begin CLocusTagEukDlg content construction
    CLocusTagEukDlg* itemDialog1 = this;

    wxBoxSizer* itemBoxSizer2 = new wxBoxSizer(wxVERTICAL);
    itemDialog1->SetSizer(itemBoxSizer2);

    wxBoxSizer* itemBoxSizer3 = new wxBoxSizer(wxHORIZONTAL);
    itemBoxSizer2->Add(itemBoxSizer3, 0, wxALIGN_LEFT|wxALL, 2);

    wxStaticText* itemStaticText3 = new wxStaticText( itemDialog1, wxID_STATIC, _("Locus-tag"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer3->Add(itemStaticText3, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    m_LocusTag = new wxTextCtrl( itemDialog1, ID_LOCUS_TAG_EUK_TXT, wxString(m_locus_tag_value), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer3->Add(m_LocusTag, 0, wxALIGN_CENTER_VERTICAL|wxALL, 2);
    if (!m_locus_tag_value.empty())
        m_LocusTag->Disable();

    wxArrayString EukProkTypes;
    EukProkTypes.Add(_("Eukaryotes"));
    EukProkTypes.Add(_("Prokaryotes")); 
    m_Euk = new wxRadioBox(itemDialog1, ID_LOCUS_TAG_EUK_CHK, wxEmptyString, wxDefaultPosition, wxDefaultSize, EukProkTypes, 1, wxRA_SPECIFY_ROWS);
    m_Euk->SetSelection(0);
    itemBoxSizer2->Add(m_Euk, 0, wxALIGN_CENTER_HORIZONTAL | wxLEFT | wxRIGHT, 5);

    wxBoxSizer* itemBoxSizer18 = new wxBoxSizer(wxHORIZONTAL);
    itemBoxSizer2->Add(itemBoxSizer18, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 2);

    wxButton* itemButton19 = new wxButton( itemDialog1, wxID_OK, _("OK"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer18->Add(itemButton19, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxButton* itemButton20 = new wxButton( itemDialog1, wxID_CANCEL, _("Cancel"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer18->Add(itemButton20, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

////@end CLocusTagEukDlg content construction
}


/*!
 * Should we show tooltips?
 */

bool CLocusTagEukDlg::ShowToolTips()
{
    return true;
}

/*!
 * Get bitmap resources
 */

wxBitmap CLocusTagEukDlg::GetBitmapResource( const wxString& name )
{
    // Bitmap retrieval
////@begin CLocusTagEukDlg bitmap retrieval
    wxUnusedVar(name);
    return wxNullBitmap;
////@end CLocusTagEukDlg bitmap retrieval
}

/*!
 * Get icon resources
 */

wxIcon CLocusTagEukDlg::GetIconResource( const wxString& name )
{
    // Icon retrieval
////@begin CLocusTagEukDlg icon retrieval
    wxUnusedVar(name);
    return wxNullIcon;
////@end CLocusTagEukDlg icon retrieval
}

string CLocusTagEukDlg::GetLocusTag()
{
    if (m_LocusTag->IsEnabled())
        return m_LocusTag->GetValue().ToStdString();

    return kEmptyStr;
}

END_NCBI_SCOPE

