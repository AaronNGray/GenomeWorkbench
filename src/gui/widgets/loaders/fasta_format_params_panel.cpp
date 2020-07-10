/*  $Id: fasta_format_params_panel.cpp 44796 2020-03-17 22:37:42Z evgeniev $
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
 *  and reliability of the software and data, the NLM and the U.S.
 *  Government do not and cannot warrant the performance or results that
 *  may be obtained by using this software or data. The NLM and the U.S.
 *  Government disclaim all warranties, express or implied, including
 *  warranties of performance, merchantability or fitness for any particular
 *  purpose.
 *
 *  Please cite the author in any work or product based on this material.
 *
 * ===========================================================================
 *
 * Authors:
 *
 * File Description:
 *
 */

#include <ncbi_pch.hpp>


#include <gui/widgets/loaders/fasta_format_params_panel.hpp>

#include <wx/sizer.h>
#include <wx/radiobox.h>

#include <wx/sizer.h>
#include <wx/radiobox.h>
#include <wx/checkbox.h>
#include <wx/button.h>
#include <wx/icon.h>


////@begin includes
////@end includes

BEGIN_NCBI_SCOPE

///////////////////////////////////////////////////////////////////////////////
/// CFASTAFormatParamsPanel

IMPLEMENT_DYNAMIC_CLASS( CFASTAFormatParamsPanel, wxPanel )

BEGIN_EVENT_TABLE( CFASTAFormatParamsPanel, wxPanel )
////@begin CFASTAFormatParamsPanel event table entries
////@end CFASTAFormatParamsPanel event table entries
END_EVENT_TABLE()


CFASTAFormatParamsPanel::CFASTAFormatParamsPanel()
{
    Init();
}


CFASTAFormatParamsPanel::CFASTAFormatParamsPanel( wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style )
{
    Init();
    Create(parent, id, pos, size, style);
}

bool CFASTAFormatParamsPanel::Create( wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style )
{
////@begin CFASTAFormatParamsPanel creation
    wxPanel::Create( parent, id, pos, size, style );

    CreateControls();
    if (GetSizer())
    {
        GetSizer()->SetSizeHints(this);
    }
    Centre();
////@end CFASTAFormatParamsPanel creation
    return true;
}


CFASTAFormatParamsPanel::~CFASTAFormatParamsPanel()
{
////@begin CFASTAFormatParamsPanel destruction
////@end CFASTAFormatParamsPanel destruction
}


void CFASTAFormatParamsPanel::Init()
{
////@begin CFASTAFormatParamsPanel member initialisation
////@end CFASTAFormatParamsPanel member initialisation
}


void CFASTAFormatParamsPanel::CreateControls()
{
////@begin CFASTAFormatParamsPanel content construction
    CFASTAFormatParamsPanel* itemPanel1 = this;

    wxBoxSizer* itemBoxSizer2 = new wxBoxSizer(wxVERTICAL);
    itemPanel1->SetSizer(itemBoxSizer2);

    wxBoxSizer* itemBoxSizer3 = new wxBoxSizer(wxHORIZONTAL);
    itemBoxSizer2->Add(itemBoxSizer3, 0, wxGROW, 5);

    wxArrayString itemRadioBox4Strings;
    itemRadioBox4Strings.Add(_("&Autodetect"));
    itemRadioBox4Strings.Add(_("&Nucleotide"));
    itemRadioBox4Strings.Add(_("&Protein"));
    wxRadioBox* itemRadioBox4 = new wxRadioBox( itemPanel1, ID_SEQ_TYPE, _("Sequence Type"), wxDefaultPosition, wxSize(itemPanel1->ConvertDialogToPixels(wxSize(150, -1)).x, -1), itemRadioBox4Strings, 1, wxRA_SPECIFY_COLS );
    itemRadioBox4->SetSelection(0);
    itemBoxSizer3->Add(itemRadioBox4, 1, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxArrayString itemRadioBox5Strings;
    itemRadioBox5Strings.Add(_("&Region Features"));
    itemRadioBox5Strings.Add(_("R&epeat Features"));
    itemRadioBox5Strings.Add(_("&Locations"));
    wxRadioBox* itemRadioBox5 = new wxRadioBox( itemPanel1, ID_LOWERCASE, _("Turn Lowercase Regions into"), wxDefaultPosition, wxSize(itemPanel1->ConvertDialogToPixels(wxSize(150, -1)).x, -1), itemRadioBox5Strings, 1, wxRA_SPECIFY_COLS );
    itemRadioBox5->SetSelection(0);
    itemBoxSizer3->Add(itemRadioBox5, 1, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxCheckBox* itemCheckBox6 = new wxCheckBox( itemPanel1, ID_CHECKBOX1, _("Force local sequence IDs"), wxDefaultPosition, wxDefaultSize, 0 );
    itemCheckBox6->SetValue(false);
    itemBoxSizer2->Add(itemCheckBox6, 0, wxALIGN_LEFT|wxALL, 5);

    wxCheckBox* itemCheckBox7 = new wxCheckBox( itemPanel1, ID_CHECKBOX2, _("Make a delta sequence if gaps are found"), wxDefaultPosition, wxDefaultSize, 0 );
    itemCheckBox7->SetValue(false);
    itemBoxSizer2->Add(itemCheckBox7, 0, wxALIGN_LEFT|wxALL, 5);

    wxCheckBox* itemCheckBox8 = new wxCheckBox( itemPanel1, ID_CHECKBOX8, _("Ignore hyphens"), wxDefaultPosition, wxDefaultSize, 0 );
    itemCheckBox8->SetValue(false);
    itemBoxSizer2->Add(itemCheckBox8, 0, wxALIGN_LEFT|wxALL, 5);

    wxCheckBox* itemCheckBox9 = new wxCheckBox( itemPanel1, ID_CHECKBOX4, _("Only read the first sequence"), wxDefaultPosition, wxDefaultSize, 0 );
    itemCheckBox9->SetValue(false);
    itemBoxSizer2->Add(itemCheckBox9, 0, wxALIGN_LEFT|wxALL, 5);

    wxCheckBox* itemCheckBox11 = new wxCheckBox( itemPanel1, ID_CHECKBOX5, _("Skip sequences with invalid defines"), wxDefaultPosition, wxDefaultSize, 0 );
    itemCheckBox11->SetValue(false);
    itemBoxSizer2->Add(itemCheckBox11, 0, wxALIGN_LEFT|wxALL, 5);

    wxCheckBox* itemCheckBox12 = new wxCheckBox( itemPanel1, ID_CHECKBOX, _("Don't split out ambiguous sequence regions"), wxDefaultPosition, wxDefaultSize, 0 );
    itemCheckBox12->SetValue(false);
    itemBoxSizer2->Add(itemCheckBox12, 0, wxALIGN_LEFT|wxALL, 5);

    // Set validators
    itemRadioBox4->SetValidator( wxGenericValidator(& GetData().m_SeqType) );
    itemRadioBox5->SetValidator( wxGenericValidator(& GetData().m_LowercaseOption) );
    itemCheckBox6->SetValidator( wxGenericValidator(& GetData().m_ForceLocalIDs) );
    itemCheckBox7->SetValidator( wxGenericValidator(& GetData().m_MakeDelta) );
    itemCheckBox8->SetValidator( wxGenericValidator(& GetData().m_IgnoreGaps) );
    itemCheckBox9->SetValidator( wxGenericValidator(& GetData().m_ReadFirst) );
    itemCheckBox11->SetValidator( wxGenericValidator(& GetData().m_SkipInvalid) );
    itemCheckBox12->SetValidator( wxGenericValidator(& GetData().m_NoSplit) );
////@end CFASTAFormatParamsPanel content construction
}


bool CFASTAFormatParamsPanel::ShowToolTips()
{
    return true;
}


wxBitmap CFASTAFormatParamsPanel::GetBitmapResource( const wxString& name )
{
    // Bitmap retrieval
////@begin CFASTAFormatParamsPanel bitmap retrieval
    wxUnusedVar(name);
    return wxNullBitmap;
////@end CFASTAFormatParamsPanel bitmap retrieval
}


wxIcon CFASTAFormatParamsPanel::GetIconResource( const wxString& name )
{
    // Icon retrieval
////@begin CFASTAFormatParamsPanel icon retrieval
    wxUnusedVar(name);
    return wxNullIcon;
////@end CFASTAFormatParamsPanel icon retrieval
}

/*!
 * Transfer data to the window
 */

bool CFASTAFormatParamsPanel::TransferDataToWindow()
{
    return wxPanel::TransferDataToWindow();
}

/*!
 * Transfer data from the window
 */

bool CFASTAFormatParamsPanel::TransferDataFromWindow()
{
    return wxPanel::TransferDataFromWindow();
}

END_NCBI_SCOPE
