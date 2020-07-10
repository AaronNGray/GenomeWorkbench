/*  $Id: single_choice_panel.cpp 34885 2016-02-25 21:48:08Z asztalos $
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
 * Authors:  Colleen Bollin, Andrea Asztalos
 */


#include <ncbi_pch.hpp>
#include <gui/widgets/wx/wx_utils.hpp>
#include <gui/packages/pkg_sequence_edit/single_choice_panel.hpp>
#include <wx/sizer.h>

BEGIN_NCBI_SCOPE

/*!
 * CSingleChoicePanel type definition
 */

 IMPLEMENT_ABSTRACT_CLASS(CSingleChoicePanel, CFieldNamePanel)


/*!
 * CSingleChoicePanel event table definition
 */

 BEGIN_EVENT_TABLE(CSingleChoicePanel, CFieldNamePanel)

////@begin CSingleChoicePanel event table entries
    EVT_CHOICE( ID_CSINGLECHOICE_FIELD, CSingleChoicePanel::OnFieldChoiceSelected)

////@end CSingleChoicePanel event table entries

END_EVENT_TABLE()


/*!
 * CSingleChoicePanel constructors
 */

 CSingleChoicePanel::CSingleChoicePanel()
{
    Init();
}

CSingleChoicePanel::CSingleChoicePanel(wxWindow* parent, const vector<string>& choices, 
    wxWindowID id, const wxPoint& pos, const wxSize& size, long style)
    : m_Choices(choices)
{
    Init();
    Create(parent, id, pos, size, style);
}


/*!
 * CSingleChoicePanel creator
 */

bool CSingleChoicePanel::Create(wxWindow* parent, wxWindowID id, 
    const wxPoint& pos, const wxSize& size, long style)
{
////@begin CSingleChoicePanel creation
    CFieldNamePanel::Create( parent, id, pos, size, style );

    CreateControls();
    if (GetSizer())
    {
        GetSizer()->SetSizeHints(this);
    }
    Centre();
////@end CSingleChoicePanel creation
    return true;
}


/*!
 * CSingleChoicePanel destructor
 */

CSingleChoicePanel::~CSingleChoicePanel()
{
////@begin CSingleChoicePanel destruction
////@end CSingleChoicePanel destruction
}


/*!
 * Member initialisation
 */

void CSingleChoicePanel::Init()
{
////@begin CSingleChoicePanel member initialisation
    m_FieldChoice = NULL;
////@end CSingleChoicePanel member initialisation
}


/*!
 * Control creation for CSingleChoicePanel
 */

void CSingleChoicePanel::CreateControls()
{    
////@begin CSingleChoicePanel content construction
    CSingleChoicePanel* itemCFieldNamePanel1 = this;

    wxBoxSizer* itemBoxSizer2 = new wxBoxSizer(wxVERTICAL);
    itemCFieldNamePanel1->SetSizer(itemBoxSizer2);

    wxArrayString m_FieldChoiceStrings;
    ToArrayString(m_Choices, m_FieldChoiceStrings);
    m_FieldChoice = new wxChoice(itemCFieldNamePanel1, ID_CSINGLECHOICE_FIELD, wxDefaultPosition, wxDefaultSize, m_FieldChoiceStrings, 0);
    itemBoxSizer2->Add(m_FieldChoice, 0, wxALIGN_CENTER_HORIZONTAL|wxLEFT|wxRIGHT, 5);
////@end CSingleChoicePanel content construction
}


/*!
 * Should we show tooltips?
 */

bool CSingleChoicePanel::ShowToolTips()
{
    return true;
}

/*!
 * Get bitmap resources
 */

wxBitmap CSingleChoicePanel::GetBitmapResource(const wxString& name)
{
    // Bitmap retrieval
////@begin CSingleChoicePanel bitmap retrieval
    wxUnusedVar(name);
    return wxNullBitmap;
////@end CSingleChoicePanel bitmap retrieval
}

/*!
 * Get icon resources
 */

wxIcon CSingleChoicePanel::GetIconResource(const wxString& name)
{
    // Icon retrieval
////@begin CSingleChoicePanel icon retrieval
    wxUnusedVar(name);
    return wxNullIcon;
////@end CSingleChoicePanel icon retrieval
}

/*!
 * wxEVT_COMMAND_CHOICE_SELECTED event handler for ID_CSINGLECHOICE_FIELD
 */

void CSingleChoicePanel::OnFieldChoiceSelected(wxCommandEvent& event)
{
    x_UpdateParent();
    event.Skip();
}

vector<string> CSingleChoicePanel::GetChoices(bool& allow_other)
{
    allow_other = false;
    return m_Choices;
}

END_NCBI_SCOPE

