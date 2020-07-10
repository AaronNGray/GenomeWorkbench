/*  $Id: ld_filter_dlg.cpp 25491 2012-03-27 17:52:21Z kuznets $
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
 * Authors:  Melvin Quintos
 */


#include <ncbi_pch.hpp>

////@begin includes
////@end includes

#include <gui/packages/pkg_snp/ld/ld_filter_dlg.hpp>

////@begin XPM images
////@end XPM images

BEGIN_NCBI_SCOPE

/*!
 * CLDFilterDialog type definition
 */

IMPLEMENT_DYNAMIC_CLASS( CLDFilterDialog, wxDialog )


/*!
 * CLDFilterDialog event table definition
 */

BEGIN_EVENT_TABLE( CLDFilterDialog, wxDialog )

////@begin CLDFilterDialog event table entries
    EVT_SLIDER( ID_SLIDER, CLDFilterDialog::OnScoreSliderUpdated )

    EVT_SLIDER( ID_SLIDER1, CLDFilterDialog::OnLengthSliderUpdated )

////@end CLDFilterDialog event table entries

END_EVENT_TABLE()


/*!
 * CLDFilterDialog constructors
 */

CLDFilterDialog::CLDFilterDialog()
{
    Init();
}

CLDFilterDialog::CLDFilterDialog( wxWindow* parent, wxWindowID id, const wxString& caption, const wxPoint& pos, const wxSize& size, long style )
{
    Init();
    CreateX(parent, id, caption, pos, size, style);
}


/*!
 * CLDFilterDialog creator
 */

bool CLDFilterDialog::CreateX( wxWindow* parent, wxWindowID id, const wxString& caption, const wxPoint& pos, const wxSize& size, long style )
{
////@begin CLDFilterDialog creation
    SetExtraStyle(wxWS_EX_BLOCK_EVENTS);
    wxDialog::Create( parent, id, caption, pos, size, style );

    CreateControls();
    if (GetSizer())
    {
        GetSizer()->SetSizeHints(this);
    }
    Centre();
////@end CLDFilterDialog creation
    return true;
}


/*!
 * CLDFilterDialog destructor
 */

CLDFilterDialog::~CLDFilterDialog()
{
////@begin CLDFilterDialog destruction
////@end CLDFilterDialog destruction
}


/*!
 * Member initialisation
 */

void CLDFilterDialog::Init()
{
////@begin CLDFilterDialog member initialisation
    m_ctrlScore = NULL;
    m_txtScore = NULL;
    m_ctrlLength = NULL;
    m_txtLength = NULL;
////@end CLDFilterDialog member initialisation
}


/*!
 * Control creation for CLDFilterDialog
 */

void CLDFilterDialog::CreateControls()
{    
////@begin CLDFilterDialog content construction
    CLDFilterDialog* itemDialog1 = this;

    wxBoxSizer* itemBoxSizer2 = new wxBoxSizer(wxVERTICAL);
    itemDialog1->SetSizer(itemBoxSizer2);

    wxBoxSizer* itemBoxSizer3 = new wxBoxSizer(wxHORIZONTAL);
    itemBoxSizer2->Add(itemBoxSizer3, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

    wxBoxSizer* itemBoxSizer4 = new wxBoxSizer(wxVERTICAL);
    itemBoxSizer3->Add(itemBoxSizer4, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxStaticText* itemStaticText5 = new wxStaticText( itemDialog1, wxID_STATIC, _("Score"), wxDefaultPosition, wxSize(-1, 20), 0 );
    itemBoxSizer4->Add(itemStaticText5, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

    wxStaticText* itemStaticText6 = new wxStaticText( itemDialog1, wxID_STATIC, _("Length"), wxDefaultPosition, wxSize(-1, 20), 0 );
    itemBoxSizer4->Add(itemStaticText6, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

    wxBoxSizer* itemBoxSizer7 = new wxBoxSizer(wxVERTICAL);
    itemBoxSizer3->Add(itemBoxSizer7, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxBoxSizer* itemBoxSizer8 = new wxBoxSizer(wxHORIZONTAL);
    itemBoxSizer7->Add(itemBoxSizer8, 0, wxALIGN_LEFT|wxALL, 0);

    m_ctrlScore = new wxSlider( itemDialog1, ID_SLIDER, 0, 0, 100, wxDefaultPosition, wxSize(-1, 20), wxSL_HORIZONTAL );
    itemBoxSizer8->Add(m_ctrlScore, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    m_txtScore = new wxStaticText( itemDialog1, wxID_STATIC, _("0.0"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer8->Add(m_txtScore, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxBoxSizer* itemBoxSizer11 = new wxBoxSizer(wxHORIZONTAL);
    itemBoxSizer7->Add(itemBoxSizer11, 0, wxALIGN_LEFT|wxALL, 0);

    m_ctrlLength = new wxSlider( itemDialog1, ID_SLIDER1, 0, 0, 7, wxDefaultPosition, wxSize(-1, 20), wxSL_HORIZONTAL );
    itemBoxSizer11->Add(m_ctrlLength, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    m_txtLength = new wxStaticText( itemDialog1, wxID_STATIC, _("10^0 b"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer11->Add(m_txtLength, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxStdDialogButtonSizer* itemStdDialogButtonSizer14 = new wxStdDialogButtonSizer;

    itemBoxSizer2->Add(itemStdDialogButtonSizer14, 0, wxALIGN_RIGHT|wxALL, 5);
    wxButton* itemButton15 = new wxButton( itemDialog1, wxID_OK, _("&OK"), wxDefaultPosition, wxDefaultSize, 0 );
    itemStdDialogButtonSizer14->AddButton(itemButton15);

    wxButton* itemButton16 = new wxButton( itemDialog1, wxID_CANCEL, _("&Cancel"), wxDefaultPosition, wxDefaultSize, 0 );
    itemStdDialogButtonSizer14->AddButton(itemButton16);

    itemStdDialogButtonSizer14->Realize();

    // Set validators
    m_ctrlScore->SetValidator( wxGenericValidator(& GetData().m_nScore) );
    m_ctrlLength->SetValidator( wxGenericValidator(& GetData().m_nLengthExp) );
////@end CLDFilterDialog content construction
}


/*!
 * Should we show tooltips?
 */

bool CLDFilterDialog::ShowToolTips()
{
    return true;
}

/*!
 * Get bitmap resources
 */

wxBitmap CLDFilterDialog::GetBitmapResource( const wxString& name )
{
    // Bitmap retrieval
////@begin CLDFilterDialog bitmap retrieval
    wxUnusedVar(name);
    return wxNullBitmap;
////@end CLDFilterDialog bitmap retrieval
}

/*!
 * Get icon resources
 */

wxIcon CLDFilterDialog::GetIconResource( const wxString& name )
{
    // Icon retrieval
////@begin CLDFilterDialog icon retrieval
    wxUnusedVar(name);
    return wxNullIcon;
////@end CLDFilterDialog icon retrieval
}


/*!
 * wxEVT_COMMAND_SLIDER_UPDATED event handler for ID_SLIDER
 */

void CLDFilterDialog::OnScoreSliderUpdated( wxCommandEvent& event )
{
    int v = m_ctrlScore->GetValue();
    float score = (float)(v)/100;
    wxString label = wxString::Format(wxT("%3.2f"), score);
    m_txtScore->SetLabel(label);

    event.Skip();
}


/*!
 * wxEVT_COMMAND_SLIDER_UPDATED event handler for ID_SLIDER1
 */

void CLDFilterDialog::OnLengthSliderUpdated( wxCommandEvent& event )
{
    int v = m_ctrlLength->GetValue();

    wxString base = wxT("bp");
    if (v < 3) 
        base = wxT(" bp");
    else if (v < 6) 
        base = wxT(" Kbp");
    else if (v < 9)
        base = wxT(" Mbp");

    int lenPow = (int)pow(10.0f, v%3);
    wxString label = wxString::Format(wxT("%d"), lenPow) + base;
    m_txtLength->SetLabel(label);

    event.Skip();
}


bool CLDFilterDialog::TransferDataToWindow()
{    
    bool res = wxDialog::TransferDataToWindow();

    wxCommandEvent dummy;
    OnScoreSliderUpdated(dummy);
    OnLengthSliderUpdated(dummy);

    return res;
}

END_NCBI_SCOPE

