/*  $Id: font_demo_panel.cpp 25560 2012-04-09 14:28:39Z katargir $
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
 * Authors:  Roman Katargin
 *
 * File Description:
 */

#include <ncbi_pch.hpp>

#include <wx/sizer.h>
#include <wx/stattext.h>
#include <wx/choice.h>

#include "font_demo_panel.hpp"
#include "font_window.hpp"

BEGIN_NCBI_SCOPE

#define ID_CHOICE_FONT wxID_HIGHEST + 1
#define ID_CHOICE_SIZE wxID_HIGHEST + 2
#define ID_FONT_WINDOW wxID_HIGHEST + 3


BEGIN_EVENT_TABLE(CFontDemoPanel, wxPanel)
    EVT_CHOICE( ID_CHOICE_FONT, CFontDemoPanel::OnFontSelected )
    EVT_CHOICE( ID_CHOICE_SIZE, CFontDemoPanel::OnFontSelected )
END_EVENT_TABLE()

CFontDemoPanel::CFontDemoPanel()
{
}

CFontDemoPanel::CFontDemoPanel(wxWindow* parent, wxWindowID id)
{
    Create(parent, id);
}

bool CFontDemoPanel::Create(wxWindow* parent, wxWindowID id)
{
    wxPanel::Create(parent, id, wxDefaultPosition, wxSize(0,0), 0);
    CreateControls();
    return true;
}

void CFontDemoPanel::CreateControls()
{
    wxBoxSizer *topSizer = new wxBoxSizer(wxVERTICAL);
    wxBoxSizer *barSizer = new wxBoxSizer(wxHORIZONTAL);
    barSizer->Add(new wxStaticText(this, wxID_ANY, wxT("Font")), 0, wxALL|wxALIGN_CENTER_VERTICAL, 5);
    topSizer->Add(barSizer, 0, wxALIGN_RIGHT);

    wxArrayString fontChoiceStrings;
    fontChoiceStrings.Add(wxT("Courier"));
    fontChoiceStrings.Add(wxT("Fixed"));
    fontChoiceStrings.Add(wxT("Helvetica"));
    fontChoiceStrings.Add(wxT("Lucida"));
    fontChoiceStrings.Add(wxT("Times"));

    wxChoice *fontChoice = new wxChoice(this, ID_CHOICE_FONT,
                                        wxDefaultPosition, wxDefaultSize,
                                        fontChoiceStrings, 0);
    fontChoice->SetStringSelection(wxT("Helvetica"));
    barSizer->Add(fontChoice, 0, wxALL|wxALIGN_CENTER_VERTICAL, 5);

    wxArrayString sizeChoiceStrings;
    sizeChoiceStrings.Add(wxT("8"));
    sizeChoiceStrings.Add(wxT("10"));
    sizeChoiceStrings.Add(wxT("12"));
    sizeChoiceStrings.Add(wxT("14"));
    sizeChoiceStrings.Add(wxT("18"));
    sizeChoiceStrings.Add(wxT("24"));

    wxChoice *sizeChoice = new wxChoice(this, ID_CHOICE_SIZE,
                                        wxDefaultPosition, wxDefaultSize,
                                        sizeChoiceStrings, 0);
    sizeChoice->SetStringSelection(wxT("10"));
    barSizer->Add(sizeChoice, 0, wxALL|wxALIGN_CENTER_VERTICAL, 5);

    barSizer->Add(new wxStaticText(this, wxID_ANY, wxT("Size")), 0, wxALL|wxALIGN_CENTER_VERTICAL, 5);

    CFontWindow *fontWindow = new CFontWindow(this, ID_FONT_WINDOW);
    topSizer->Add(fontWindow, 1, wxEXPAND);

    SetSizer(topSizer);
}

void CFontDemoPanel::OnFontSelected(wxCommandEvent& WXUNUSED(event))
{
    wxChoice *fontChoice = (wxChoice*)FindWindow(ID_CHOICE_FONT);
    wxChoice *sizeChoice = (wxChoice*)FindWindow(ID_CHOICE_SIZE);
    CFontWindow *fontWindow = (CFontWindow*)FindWindow(ID_FONT_WINDOW);

    wxString name = fontChoice->GetStringSelection();
    wxString size = sizeChoice->GetStringSelection();

    int fontFace = CGlBitmapFont::eFontFace_Helvetica;
    if (wxT("Courier") == name)
        fontFace = CGlBitmapFont::eFontFace_Courier;
    else if (wxT("Fixed") == name)
        fontFace = CGlBitmapFont::eFontFace_Bitmap;
    else if (wxT("Lucida") == name)
        fontFace = CGlBitmapFont::eFontFace_Lucida;
    else if (wxT("Times") == name)
        fontFace = CGlBitmapFont::eFontFace_TimesRoman;

    int fontSize = CGlBitmapFont::eFontSize_10;
    if (wxT("8") == size)
        fontSize = CGlBitmapFont::eFontSize_8;
    else if (wxT("12") == size)
        fontSize = CGlBitmapFont::eFontSize_12;
    else if (wxT("14") == size)
        fontSize = CGlBitmapFont::eFontSize_14;
    else if (wxT("18") == size)
        fontSize = CGlBitmapFont::eFontSize_18;
    else if (wxT("24") == size)
        fontSize = CGlBitmapFont::eFontSize_24;

    if (fontFace == CGlBitmapFont::eFontFace_Bitmap &&
        fontSize == CGlBitmapFont::eFontSize_10) fontSize = CGlBitmapFont::eFontSize_12;

    fontWindow->SetGlFont((CGlBitmapFont::EFont)(fontFace|fontSize));
}

END_NCBI_SCOPE
