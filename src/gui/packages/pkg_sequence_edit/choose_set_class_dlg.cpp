/*  $Id: choose_set_class_dlg.cpp 34945 2016-03-03 20:31:51Z asztalos $
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

#include <gui/packages/pkg_sequence_edit/choose_set_class_dlg.hpp>

#include <wx/sizer.h>
#include <wx/button.h>

////@begin XPM images
////@end XPM images


/*!
 * CChooseSetClassDlg type definition
 */

IMPLEMENT_DYNAMIC_CLASS( CChooseSetClassDlg, wxDialog )


/*!
 * CChooseSetClassDlg event table definition
 */

BEGIN_EVENT_TABLE( CChooseSetClassDlg, wxDialog )

////@begin CChooseSetClassDlg event table entries
////@end CChooseSetClassDlg event table entries

END_EVENT_TABLE()


/*!
 * CChooseSetClassDlg constructors
 */

CChooseSetClassDlg::CChooseSetClassDlg()
{
    Init();
}

CChooseSetClassDlg::CChooseSetClassDlg( wxWindow* parent, wxWindowID id, const wxString& caption, const wxPoint& pos, const wxSize& size, long style )
{
    Init();
    Create(parent, id, caption, pos, size, style);
}


/*!
 * CChooseSetClassDlg creator
 */

bool CChooseSetClassDlg::Create( wxWindow* parent, wxWindowID id, const wxString& caption, const wxPoint& pos, const wxSize& size, long style )
{
////@begin CChooseSetClassDlg creation
    SetExtraStyle(wxWS_EX_BLOCK_EVENTS);
    wxDialog::Create( parent, id, caption, pos, size, style );

    CreateControls();
    if (GetSizer())
    {
        GetSizer()->SetSizeHints(this);
    }
    Centre();
////@end CChooseSetClassDlg creation
    return true;
}


/*!
 * CChooseSetClassDlg destructor
 */

CChooseSetClassDlg::~CChooseSetClassDlg()
{
////@begin CChooseSetClassDlg destruction
////@end CChooseSetClassDlg destruction
}


/*!
 * Member initialisation
 */

void CChooseSetClassDlg::Init()
{
////@begin CChooseSetClassDlg member initialisation
    m_SetClassChoice = NULL;
////@end CChooseSetClassDlg member initialisation
}


/*!
 * Control creation for CChooseSetClassDlg
 */

void CChooseSetClassDlg::CreateControls()
{    
////@begin CChooseSetClassDlg content construction
    CChooseSetClassDlg* itemDialog1 = this;

    wxBoxSizer* itemBoxSizer2 = new wxBoxSizer(wxVERTICAL);
    itemDialog1->SetSizer(itemBoxSizer2);

    wxArrayString m_SetClassChoiceStrings;
    m_SetClassChoiceStrings.Add(_("Genbank"));
    m_SetClassChoiceStrings.Add(_("Mutation Set"));
    m_SetClassChoiceStrings.Add(_("Population Study"));
    m_SetClassChoiceStrings.Add(_("Phylogenetic Study"));
    m_SetClassChoiceStrings.Add(_("Ecological Sample Study"));
    m_SetClassChoiceStrings.Add(_("WGS Set"));
    m_SetClassChoiceStrings.Add(_("Small Genome Set"));
    m_SetClassChoice = new wxChoice( itemDialog1, ID_SET_CLASS_CHOICE, wxDefaultPosition, wxDefaultSize, m_SetClassChoiceStrings, 0 );
    m_SetClassChoice->SetStringSelection(_("Phylogenetic Study"));
    itemBoxSizer2->Add(m_SetClassChoice, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

    wxBoxSizer* itemBoxSizer4 = new wxBoxSizer(wxHORIZONTAL);
    itemBoxSizer2->Add(itemBoxSizer4, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

    wxButton* itemButton5 = new wxButton( itemDialog1, wxID_OK, _("Accept"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer4->Add(itemButton5, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxButton* itemButton6 = new wxButton( itemDialog1, wxID_CANCEL, _("Cancel"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer4->Add(itemButton6, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

////@end CChooseSetClassDlg content construction
}


/*!
 * Should we show tooltips?
 */

bool CChooseSetClassDlg::ShowToolTips()
{
    return true;
}

/*!
 * Get bitmap resources
 */

wxBitmap CChooseSetClassDlg::GetBitmapResource( const wxString& name )
{
    // Bitmap retrieval
////@begin CChooseSetClassDlg bitmap retrieval
    wxUnusedVar(name);
    return wxNullBitmap;
////@end CChooseSetClassDlg bitmap retrieval
}

/*!
 * Get icon resources
 */

wxIcon CChooseSetClassDlg::GetIconResource( const wxString& name )
{
    // Icon retrieval
////@begin CChooseSetClassDlg icon retrieval
    wxUnusedVar(name);
    return wxNullIcon;
////@end CChooseSetClassDlg icon retrieval
}


ncbi::objects::CBioseq_set::EClass CChooseSetClassDlg::GetClass()
{
    ncbi::objects::CBioseq_set::EClass rval = ncbi::objects::CBioseq_set::eClass_other;

    switch (m_SetClassChoice->GetSelection()) {
        case 0:
            rval = ncbi::objects::CBioseq_set::eClass_genbank;
            break;
        case 1:
            rval = ncbi::objects::CBioseq_set::eClass_mut_set;
            break;
        case 2:
            rval = ncbi::objects::CBioseq_set::eClass_pop_set;
            break;
        case 3:
            rval = ncbi::objects::CBioseq_set::eClass_phy_set;
            break;
        case 4:
            rval = ncbi::objects::CBioseq_set::eClass_eco_set;
            break;
        case 5:
            rval = ncbi::objects::CBioseq_set::eClass_wgs_set;
            break;
        case 6:
            rval = ncbi::objects::CBioseq_set::eClass_small_genome_set;
            break;
    }
    return rval;
}
