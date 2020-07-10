/*  $Id: loadentrez_seq_dlg.cpp 35961 2016-07-19 20:41:20Z asztalos $
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
#include <objects/seqloc/Seq_id.hpp>
#include <gui/widgets/wx/wx_utils.hpp>
#include <gui/packages/pkg_sequence_edit/loadentrez_seq_dlg.hpp>

#include <wx/sizer.h>
#include <wx/stattext.h>
#include <wx/textctrl.h>
#include <wx/button.h>


BEGIN_NCBI_SCOPE
USING_SCOPE(objects);
/*!
 * CLoadSeqFromEntrez_dlg type definition
 */

IMPLEMENT_DYNAMIC_CLASS( CLoadSeqFromEntrez_dlg, wxDialog )


/*!
 * CLoadSeqFromEntrez_dlg event table definition
 */

BEGIN_EVENT_TABLE( CLoadSeqFromEntrez_dlg, wxDialog )

////@begin CLoadSeqFromEntrez_dlg event table entries
    EVT_UPDATE_UI( wxID_OK, CLoadSeqFromEntrez_dlg::OnTextChange )
    EVT_TEXT_ENTER(ID_LDTEXT, CLoadSeqFromEntrez_dlg::OnTextCtrlEnter)
////@end CLoadSeqFromEntrez_dlg event table entries

END_EVENT_TABLE()


/*!
 * CLoadSeqFromEntrez_dlg constructors
 */

CLoadSeqFromEntrez_dlg::CLoadSeqFromEntrez_dlg()
{
    Init();
}

CLoadSeqFromEntrez_dlg::CLoadSeqFromEntrez_dlg( wxWindow* parent, wxWindowID id, const wxString& caption, const wxPoint& pos, const wxSize& size, long style )
{
    Init();
    Create(parent, id, caption, pos, size, style);
}


/*!
 * CLoadSeqFromEntrez_dlg creator
 */

bool CLoadSeqFromEntrez_dlg::Create( wxWindow* parent, wxWindowID id, const wxString& caption, const wxPoint& pos, const wxSize& size, long style )
{
////@begin CLoadSeqFromEntrez_dlg creation
    SetExtraStyle(wxWS_EX_BLOCK_EVENTS);
    wxDialog::Create( parent, id, caption, pos, size, style );

    CreateControls();
    if (GetSizer())
    {
        GetSizer()->SetSizeHints(this);
    }
    Centre();
////@end CLoadSeqFromEntrez_dlg creation
    return true;
}


/*!
 * CLoadSeqFromEntrez_dlg destructor
 */

CLoadSeqFromEntrez_dlg::~CLoadSeqFromEntrez_dlg()
{
////@begin CLoadSeqFromEntrez_dlg destruction
////@end CLoadSeqFromEntrez_dlg destruction
}


/*!
 * Member initialisation
 */

void CLoadSeqFromEntrez_dlg::Init()
{
////@begin CLoadSeqFromEntrez_dlg member initialisation
    m_Text = NULL;
////@end CLoadSeqFromEntrez_dlg member initialisation
}


/*!
 * Control creation for CLoadSeqFromEntrez_dlg
 */

void CLoadSeqFromEntrez_dlg::CreateControls()
{    
////@begin CLoadSeqFromEntrez_dlg content construction
    CLoadSeqFromEntrez_dlg* itemDialog1 = this;

    wxBoxSizer* itemBoxSizer2 = new wxBoxSizer(wxVERTICAL);
    itemDialog1->SetSizer(itemBoxSizer2);

    wxBoxSizer* itemBoxSizer3 = new wxBoxSizer(wxHORIZONTAL);
    itemBoxSizer2->Add(itemBoxSizer3, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

    wxStaticText* itemStaticText4 = new wxStaticText( itemDialog1, wxID_STATIC, _("Accession/GI:"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer3->Add(itemStaticText4, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    m_Text = new wxTextCtrl(itemDialog1, ID_LDTEXT, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_PROCESS_ENTER);
    itemBoxSizer3->Add(m_Text, 0, wxALIGN_CENTER_VERTICAL | wxALL, 5);

    wxBoxSizer* itemBoxSizer6 = new wxBoxSizer(wxHORIZONTAL);
    itemBoxSizer2->Add(itemBoxSizer6, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

    wxButton* itemButton7 = new wxButton( itemDialog1, wxID_OK, _("Retrieve"), wxDefaultPosition, wxDefaultSize, 0 );
    itemButton7->Enable(false);
    itemBoxSizer6->Add(itemButton7, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxButton* itemButton8 = new wxButton( itemDialog1, wxID_CANCEL, _("Cancel"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer6->Add(itemButton8, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

////@end CLoadSeqFromEntrez_dlg content construction
}

string CLoadSeqFromEntrez_dlg::GetData() const
{
    return ToStdString(m_Text->GetValue());
}

void CLoadSeqFromEntrez_dlg::OnTextChange(wxUpdateUIEvent& event)
{
    bool enable = (m_Text->IsEmpty()) ? false : true;
    event.Enable(enable);
}

void CLoadSeqFromEntrez_dlg::OnTextCtrlEnter(wxCommandEvent& event)
{
    EndModal(wxID_OK);
}

/*!
 * Should we show tooltips?
 */

bool CLoadSeqFromEntrez_dlg::ShowToolTips()
{
    return true;
}

/*!
 * Get bitmap resources
 */

wxBitmap CLoadSeqFromEntrez_dlg::GetBitmapResource( const wxString& name )
{
    // Bitmap retrieval
////@begin CLoadSeqFromEntrez_dlg bitmap retrieval
    wxUnusedVar(name);
    return wxNullBitmap;
////@end CLoadSeqFromEntrez_dlg bitmap retrieval
}

/*!
 * Get icon resources
 */

wxIcon CLoadSeqFromEntrez_dlg::GetIconResource( const wxString& name )
{
    // Icon retrieval
////@begin CLoadSeqFromEntrez_dlg icon retrieval
    wxUnusedVar(name);
    return wxNullIcon;
////@end CLoadSeqFromEntrez_dlg icon retrieval
}


END_NCBI_SCOPE
