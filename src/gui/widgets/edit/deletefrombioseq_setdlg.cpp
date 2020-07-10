/*  $Id: deletefrombioseq_setdlg.cpp 43274 2019-06-06 19:39:53Z asztalos $
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
 * Authors:  Roman Katargin
 */


#include <ncbi_pch.hpp>

////@begin includes
////@end includes

#include <wx/sizer.h>
#include <wx/button.h>
#include <wx/radiobox.h>

#include "deletefrombioseq_setdlg.hpp"

////@begin XPM images
////@end XPM images

BEGIN_NCBI_SCOPE

/*!
 * CDeleteFromBioseq_setDlg type definition
 */

IMPLEMENT_DYNAMIC_CLASS( CDeleteFromBioseq_setDlg, CDialog )


/*!
 * CDeleteFromBioseq_setDlg event table definition
 */

BEGIN_EVENT_TABLE( CDeleteFromBioseq_setDlg, CDialog )

////@begin CDeleteFromBioseq_setDlg event table entries
////@end CDeleteFromBioseq_setDlg event table entries

END_EVENT_TABLE()


/*!
 * CDeleteFromBioseq_setDlg constructors
 */

CDeleteFromBioseq_setDlg::CDeleteFromBioseq_setDlg()
{
    Init();
}

CDeleteFromBioseq_setDlg::CDeleteFromBioseq_setDlg( wxWindow* parent, wxWindowID id, const wxString& caption, const wxPoint& pos, const wxSize& size, long style )
{
    Init();
    Create(parent, id, caption, pos, size, style);
}


/*!
 * CDeleteFromBioseq_setDlg creator
 */

bool CDeleteFromBioseq_setDlg::Create( wxWindow* parent, wxWindowID id, const wxString& caption, const wxPoint& pos, const wxSize& size, long style )
{
////@begin CDeleteFromBioseq_setDlg creation
    SetExtraStyle(wxWS_EX_BLOCK_EVENTS);
    CDialog::Create( parent, id, caption, pos, size, style );

    CreateControls();
    if (GetSizer())
    {
        GetSizer()->SetSizeHints(this);
    }
    Centre();
////@end CDeleteFromBioseq_setDlg creation
    return true;
}


/*!
 * CDeleteFromBioseq_setDlg destructor
 */

CDeleteFromBioseq_setDlg::~CDeleteFromBioseq_setDlg()
{
////@begin CDeleteFromBioseq_setDlg destruction
////@end CDeleteFromBioseq_setDlg destruction
}


/*!
 * Member initialisation
 */

void CDeleteFromBioseq_setDlg::Init()
{
    SetRegistryPath("CDeleteFromBioseq_setDlg");

////@begin CDeleteFromBioseq_setDlg member initialisation
    m_Choice = 0;
////@end CDeleteFromBioseq_setDlg member initialisation
}


/*!
 * Control creation for CDeleteFromBioseq_setDlg
 */

void CDeleteFromBioseq_setDlg::CreateControls()
{    
////@begin CDeleteFromBioseq_setDlg content construction
    CDeleteFromBioseq_setDlg* itemCDialog1 = this;

    wxBoxSizer* itemBoxSizer2 = new wxBoxSizer(wxVERTICAL);
    itemCDialog1->SetSizer(itemBoxSizer2);

    wxArrayString itemRadioBox3Strings;
    itemRadioBox3Strings.Add(_("Remove information from all of the sequences"));
    itemRadioBox3Strings.Add(_("Remove information from just the current sequence"));
    wxRadioBox* itemRadioBox3 = new wxRadioBox( itemCDialog1, ID_RADIOBOX, _("This information applies to multiple sequences"), wxDefaultPosition, wxDefaultSize, itemRadioBox3Strings, 1, wxRA_SPECIFY_COLS );
    itemRadioBox3->SetSelection(0);
    itemBoxSizer2->Add(itemRadioBox3, 0, wxGROW|wxALL, 5);

    wxStdDialogButtonSizer* itemStdDialogButtonSizer4 = new wxStdDialogButtonSizer;

    itemBoxSizer2->Add(itemStdDialogButtonSizer4, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);
    wxButton* itemButton5 = new wxButton( itemCDialog1, wxID_OK, _("&OK"), wxDefaultPosition, wxDefaultSize, 0 );
    itemStdDialogButtonSizer4->AddButton(itemButton5);

    wxButton* itemButton6 = new wxButton( itemCDialog1, wxID_CANCEL, _("&Cancel"), wxDefaultPosition, wxDefaultSize, 0 );
    itemStdDialogButtonSizer4->AddButton(itemButton6);

    itemStdDialogButtonSizer4->Realize();

    // Set validators
    itemRadioBox3->SetValidator( wxGenericValidator(& m_Choice) );
////@end CDeleteFromBioseq_setDlg content construction
}

static const char* kDeleteChoice = "DeleteChoice";

void CDeleteFromBioseq_setDlg::x_LoadSettings(const CRegistryReadView& view)
{
    m_Choice = view.GetInt(kDeleteChoice, m_Choice);
}

void CDeleteFromBioseq_setDlg::x_SaveSettings(CRegistryWriteView view) const
{
    view.Set(kDeleteChoice, m_Choice);
}

/*!
 * Should we show tooltips?
 */

bool CDeleteFromBioseq_setDlg::ShowToolTips()
{
    return true;
}

/*!
 * Get bitmap resources
 */

wxBitmap CDeleteFromBioseq_setDlg::GetBitmapResource( const wxString& name )
{
    // Bitmap retrieval
////@begin CDeleteFromBioseq_setDlg bitmap retrieval
    wxUnusedVar(name);
    return wxNullBitmap;
////@end CDeleteFromBioseq_setDlg bitmap retrieval
}

/*!
 * Get icon resources
 */

wxIcon CDeleteFromBioseq_setDlg::GetIconResource( const wxString& name )
{
    // Icon retrieval
////@begin CDeleteFromBioseq_setDlg icon retrieval
    wxUnusedVar(name);
    return wxNullIcon;
////@end CDeleteFromBioseq_setDlg icon retrieval
}

bool CDeleteFromBioseq_setDlg::TransferDataToWindow()
{
    return CDialog::TransferDataToWindow();
}

END_NCBI_SCOPE
