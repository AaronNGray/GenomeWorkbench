/*  $Id: text_align_params_panel.cpp 38672 2017-06-07 21:13:40Z katargir $
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
 * Authors: Roman Katargin
 *
 * File Description:
 *
 */

#include <ncbi_pch.hpp>

#include <wx/sizer.h>
#include <wx/choice.h>
#include <wx/textctrl.h>
#include <wx/valtext.h>
#include <wx/valgen.h>
#include <wx/stattext.h>
#include <wx/bitmap.h>
#include <wx/icon.h>

////@begin includes
////@end includes

#include <gui/widgets/loaders/text_align_params_panel.hpp>

BEGIN_NCBI_SCOPE

////@begin XPM images
////@end XPM images

IMPLEMENT_DYNAMIC_CLASS( CTextAlignParamsPanel, wxPanel )

BEGIN_EVENT_TABLE( CTextAlignParamsPanel, wxPanel )

////@begin CTextAlignParamsPanel event table entries
////@end CTextAlignParamsPanel event table entries

END_EVENT_TABLE()

CTextAlignParamsPanel::CTextAlignParamsPanel()
{
    Init();
}

CTextAlignParamsPanel::CTextAlignParamsPanel( wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style )
{
    Init();
    Create(parent, id, pos, size, style);
}

bool CTextAlignParamsPanel::Create( wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style )
{
////@begin CTextAlignParamsPanel creation
    wxPanel::Create( parent, id, pos, size, style );

    CreateControls();
    if (GetSizer())
    {
        GetSizer()->SetSizeHints(this);
    }
    Centre();
////@end CTextAlignParamsPanel creation
    return true;
}

CTextAlignParamsPanel::~CTextAlignParamsPanel()
{
////@begin CTextAlignParamsPanel destruction
////@end CTextAlignParamsPanel destruction
}

void CTextAlignParamsPanel::Init()
{
////@begin CTextAlignParamsPanel member initialisation
////@end CTextAlignParamsPanel member initialisation
}

void CTextAlignParamsPanel::CreateControls()
{    
////@begin CTextAlignParamsPanel content construction
    CTextAlignParamsPanel* itemPanel1 = this;

    wxBoxSizer* itemBoxSizer2 = new wxBoxSizer(wxVERTICAL);
    itemPanel1->SetSizer(itemBoxSizer2);

    wxStaticText* itemStaticText3 = new wxStaticText( itemPanel1, wxID_STATIC, _("Text Alignment Load Parameters"), wxDefaultPosition, wxDefaultSize, 0 );
    itemStaticText3->SetFont(wxFont(wxNORMAL_FONT->GetPointSize(), wxNORMAL_FONT->GetFamily(), wxNORMAL_FONT->GetStyle(), wxFONTWEIGHT_BOLD, wxNORMAL_FONT->GetUnderlined(), wxNORMAL_FONT->GetFaceName()));
    itemBoxSizer2->Add(itemStaticText3, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

    wxFlexGridSizer* itemFlexGridSizer4 = new wxFlexGridSizer(0, 2, 0, 0);
    itemBoxSizer2->Add(itemFlexGridSizer4, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

    wxStaticText* itemStaticText5 = new wxStaticText( itemPanel1, wxID_STATIC, _("Ambiguous/Unknown"), wxDefaultPosition, wxDefaultSize, 0 );
    itemFlexGridSizer4->Add(itemStaticText5, 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL|wxALL, 2);

    wxTextCtrl* itemTextCtrl6 = new wxTextCtrl( itemPanel1, ID_TEXTCTRL14, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
    itemFlexGridSizer4->Add(itemTextCtrl6, 0, wxGROW|wxALIGN_CENTER_VERTICAL|wxALL, 2);

    wxStaticText* itemStaticText7 = new wxStaticText( itemPanel1, wxID_STATIC, _("Match"), wxDefaultPosition, wxDefaultSize, 0 );
    itemFlexGridSizer4->Add(itemStaticText7, 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL|wxALL, 2);

    wxTextCtrl* itemTextCtrl8 = new wxTextCtrl( itemPanel1, ID_TEXTCTRL15, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
    itemFlexGridSizer4->Add(itemTextCtrl8, 0, wxGROW|wxALIGN_CENTER_VERTICAL|wxALL, 2);

    wxStaticText* itemStaticText9 = new wxStaticText( itemPanel1, wxID_STATIC, _("Begining Gap"), wxDefaultPosition, wxDefaultSize, 0 );
    itemFlexGridSizer4->Add(itemStaticText9, 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL|wxALL, 2);

    wxTextCtrl* itemTextCtrl10 = new wxTextCtrl( itemPanel1, ID_TEXTCTRL16, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
    itemFlexGridSizer4->Add(itemTextCtrl10, 0, wxGROW|wxALIGN_CENTER_VERTICAL|wxALL, 2);

    wxStaticText* itemStaticText11 = new wxStaticText( itemPanel1, wxID_STATIC, _("Middle Gap"), wxDefaultPosition, wxDefaultSize, 0 );
    itemFlexGridSizer4->Add(itemStaticText11, 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL|wxALL, 2);

    wxTextCtrl* itemTextCtrl12 = new wxTextCtrl( itemPanel1, ID_TEXTCTRL17, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
    itemFlexGridSizer4->Add(itemTextCtrl12, 0, wxGROW|wxALIGN_CENTER_VERTICAL|wxALL, 2);

    wxStaticText* itemStaticText13 = new wxStaticText( itemPanel1, wxID_STATIC, _("End Gap"), wxDefaultPosition, wxDefaultSize, 0 );
    itemFlexGridSizer4->Add(itemStaticText13, 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL|wxALL, 2);

    wxTextCtrl* itemTextCtrl14 = new wxTextCtrl( itemPanel1, ID_TEXTCTRL18, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
    itemFlexGridSizer4->Add(itemTextCtrl14, 0, wxGROW|wxALIGN_CENTER_VERTICAL|wxALL, 2);

    wxStaticText* itemStaticText15 = new wxStaticText( itemPanel1, wxID_STATIC, _("Sequence Type"), wxDefaultPosition, wxDefaultSize, 0 );
    itemFlexGridSizer4->Add(itemStaticText15, 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL|wxALL, 2);

    wxArrayString itemChoice16Strings;
    itemChoice16Strings.Add(_("Automatically determined"));
    itemChoice16Strings.Add(_("Nucleotide"));
    itemChoice16Strings.Add(_("Protein"));
    wxChoice* itemChoice16 = new wxChoice( itemPanel1, ID_CHOICE7, wxDefaultPosition, wxDefaultSize, itemChoice16Strings, 0 );
    itemChoice16->SetStringSelection(_("Automatically determined"));
    itemFlexGridSizer4->Add(itemChoice16, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    // Set validators
    itemTextCtrl6->SetValidator( wxTextValidator(wxFILTER_ASCII, & GetData().m_Unknown) );
    itemTextCtrl8->SetValidator( wxTextValidator(wxFILTER_ASCII, & GetData().m_Match) );
    itemTextCtrl10->SetValidator( wxTextValidator(wxFILTER_ASCII, & GetData().m_Begin) );
    itemTextCtrl12->SetValidator( wxTextValidator(wxFILTER_ASCII, & GetData().m_Middle) );
    itemTextCtrl14->SetValidator( wxTextValidator(wxFILTER_ASCII, & GetData().m_End) );
    itemChoice16->SetValidator( wxGenericValidator(& GetData().m_SeqType) );
////@end CTextAlignParamsPanel content construction
}

bool CTextAlignParamsPanel::ShowToolTips()
{
    return true;
}
wxBitmap CTextAlignParamsPanel::GetBitmapResource( const wxString& name )
{
    // Bitmap retrieval
////@begin CTextAlignParamsPanel bitmap retrieval
    wxUnusedVar(name);
    return wxNullBitmap;
////@end CTextAlignParamsPanel bitmap retrieval
}
wxIcon CTextAlignParamsPanel::GetIconResource( const wxString& name )
{
    // Icon retrieval
////@begin CTextAlignParamsPanel icon retrieval
    wxUnusedVar(name);
    return wxNullIcon;
////@end CTextAlignParamsPanel icon retrieval
}
bool CTextAlignParamsPanel::TransferDataToWindow()
{
    return wxPanel::TransferDataToWindow();
}
bool CTextAlignParamsPanel::TransferDataFromWindow()
{
    return wxPanel::TransferDataFromWindow();
}

END_NCBI_SCOPE
