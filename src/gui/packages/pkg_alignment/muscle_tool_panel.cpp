/*  $Id: muscle_tool_panel.cpp 43792 2019-08-30 18:31:36Z katargir $
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

#include <gui/widgets/object_list/object_list_widget_sel.hpp>
#include <gui/widgets/wx/exe_file_validator.hpp>

#include "muscle_tool_panel.hpp"

#include <wx/sizer.h>
#include <wx/stattext.h>
#include <wx/choice.h>
#include <wx/bmpbuttn.h>
#include <wx/checkbox.h>
#include "wx/valgen.h"
#include "wx/valtext.h"
#include <wx/msgdlg.h>
#include <wx/filedlg.h>
#include <wx/filename.h>
#include <wx/artprov.h>

#include "wx/bitmap.h"
#include "wx/icon.h"

////@begin includes
////@end includes

BEGIN_NCBI_SCOPE
USING_SCOPE(ncbi::objects);

IMPLEMENT_DYNAMIC_CLASS( CMuscleToolPanel, CAlgoToolManagerParamsPanel )

BEGIN_EVENT_TABLE( CMuscleToolPanel, CAlgoToolManagerParamsPanel )

////@begin CMuscleToolPanel event table entries
    EVT_BUTTON( ID_BITMAPBUTTON1, CMuscleToolPanel::OnMusclePathClick )
////@end CMuscleToolPanel event table entries

END_EVENT_TABLE()

CMuscleToolPanel::CMuscleToolPanel()
{
    Init();
}

CMuscleToolPanel::CMuscleToolPanel( wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style, bool visible )
{
    Init();
    Create(parent, id, pos, size, style, visible);
}

bool CMuscleToolPanel::Create( wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style, bool visible )
{
    SetExtraStyle(wxWS_EX_VALIDATE_RECURSIVELY);

    CAlgoToolManagerParamsPanel::Create( parent, id, pos, size, style );

    if (!visible) {
        Hide();
    }

    CreateControls();
    if (GetSizer())
    {
        GetSizer()->SetSizeHints(this);
    }
    Centre();
    return true;
}

CMuscleToolPanel::~CMuscleToolPanel()
{
////@begin CMuscleToolPanel destruction
////@end CMuscleToolPanel destruction
}

void CMuscleToolPanel::Init()
{
////@begin CMuscleToolPanel member initialisation
    m_LocationSel = NULL;
////@end CMuscleToolPanel member initialisation
}

void CMuscleToolPanel::CreateControls()
{    
////@begin CMuscleToolPanel content construction
    // Generated by DialogBlocks, 30/08/2019 14:25:53 (unregistered)

    CMuscleToolPanel* itemCAlgoToolManagerParamsPanel1 = this;

    wxBoxSizer* itemBoxSizer2 = new wxBoxSizer(wxVERTICAL);
    itemCAlgoToolManagerParamsPanel1->SetSizer(itemBoxSizer2);

    m_LocationSel = new CObjectListWidgetSel( itemCAlgoToolManagerParamsPanel1, ID_PANEL, wxDefaultPosition, wxDefaultSize, wxNO_BORDER|wxTAB_TRAVERSAL );
    itemBoxSizer2->Add(m_LocationSel, 1, wxGROW|wxALL, 0);

    wxBoxSizer* itemBoxSizer4 = new wxBoxSizer(wxHORIZONTAL);
    itemBoxSizer2->Add(itemBoxSizer4, 0, wxALIGN_RIGHT|wxALL, 0);

    wxStaticText* itemStaticText5 = new wxStaticText( itemCAlgoToolManagerParamsPanel1, wxID_STATIC, _("Scoring method"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer4->Add(itemStaticText5, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxArrayString itemChoice6Strings;
    itemChoice6Strings.Add(_("log-expectation score (default)"));
    itemChoice6Strings.Add(_("PAM200"));
    itemChoice6Strings.Add(_("VTML240"));
    wxChoice* itemChoice6 = new wxChoice( itemCAlgoToolManagerParamsPanel1, ID_CHOICE2, wxDefaultPosition, wxDefaultSize, itemChoice6Strings, 0 );
    itemChoice6->SetStringSelection(_("log-expectation score (default)"));
    itemBoxSizer4->Add(itemChoice6, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxCheckBox* itemCheckBox7 = new wxCheckBox( itemCAlgoToolManagerParamsPanel1, ID_CHECKBOX12, _("Generate reconstructed tree from alignment"), wxDefaultPosition, wxDefaultSize, 0 );
    itemCheckBox7->SetValue(false);
    itemBoxSizer2->Add(itemCheckBox7, 0, wxALIGN_LEFT|wxALL, 5);

    wxBoxSizer* itemBoxSizer8 = new wxBoxSizer(wxHORIZONTAL);
    itemBoxSizer2->Add(itemBoxSizer8, 0, wxGROW|wxALL, 0);

    wxStaticText* itemStaticText9 = new wxStaticText( itemCAlgoToolManagerParamsPanel1, wxID_STATIC, _("Command line options"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer8->Add(itemStaticText9, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxTextCtrl* itemTextCtrl10 = new wxTextCtrl( itemCAlgoToolManagerParamsPanel1, ID_TEXTCTRL11, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer8->Add(itemTextCtrl10, 1, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxBoxSizer* itemBoxSizer11 = new wxBoxSizer(wxHORIZONTAL);
    itemBoxSizer2->Add(itemBoxSizer11, 0, wxGROW|wxALL, 0);

    wxStaticText* itemStaticText12 = new wxStaticText( itemCAlgoToolManagerParamsPanel1, wxID_STATIC, _("Path to executable"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer11->Add(itemStaticText12, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxTextCtrl* itemTextCtrl13 = new wxTextCtrl( itemCAlgoToolManagerParamsPanel1, ID_TEXTCTRL12, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer11->Add(itemTextCtrl13, 1, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxBitmapButton* itemBitmapButton14 = new wxBitmapButton( itemCAlgoToolManagerParamsPanel1, ID_BITMAPBUTTON1, itemCAlgoToolManagerParamsPanel1->GetBitmapResource(wxT("menu::open")), wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW );
    itemBitmapButton14->SetHelpText(_("Select MUSCLE executable"));
    if (CMuscleToolPanel::ShowToolTips())
        itemBitmapButton14->SetToolTip(_("Select MUSCLE executable"));
    itemBoxSizer11->Add(itemBitmapButton14, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    // Set validators
    itemChoice6->SetValidator( wxGenericValidator(& GetData().m_ScoringMethod) );
    itemCheckBox7->SetValidator( wxGenericValidator(& GetData().m_GenerateTree) );
    itemTextCtrl10->SetValidator( wxTextValidator(wxFILTER_NONE, & GetData().m_CommandLine) );
    itemTextCtrl13->SetValidator( CExeFileValidator(& GetData().m_MusclePath, wxT("MUSCLE")) );
////@end CMuscleToolPanel content construction

    m_LocationSel->SetTitle(wxT("Select Locations"));
    m_LocationSel->SetDoSelectAll(true);
    m_LocationSel->ShowNuclProtButtons();
    m_LocationSel->EnableGroups(false);
    m_LocationSel->SetDefaultSelection("Genomic Location (total range)");
}

void CMuscleToolPanel::SetObjects(map<string, TConstScopedObjects>* objects)
{
    m_LocationSel->SetObjects(objects);
}

bool CMuscleToolPanel::ShowToolTips()
{
    return true;
}


wxBitmap CMuscleToolPanel::GetBitmapResource( const wxString& name )
{
    return wxArtProvider::GetBitmap(name);
}

wxIcon CMuscleToolPanel::GetIconResource( const wxString& name )
{
    // Icon retrieval
////@begin CMuscleToolPanel icon retrieval
    wxUnusedVar(name);
    return wxNullIcon;
////@end CMuscleToolPanel icon retrieval
}

bool CMuscleToolPanel::TransferDataFromWindow()
{
    if (!CAlgoToolManagerParamsPanel::TransferDataFromWindow())
        return false;

    GetData().SetObjects() = m_LocationSel->GetSelection();

    return true;
}

static const char* kLocationSel = ".LocationSel";

void CMuscleToolPanel::SetRegistryPath(const string& path)
{
    m_RegPath = path;
    m_LocationSel->SetRegistryPath(m_RegPath + kLocationSel);
}

void CMuscleToolPanel::SaveSettings() const
{
    m_LocationSel->SaveSettings();
}

void CMuscleToolPanel::LoadSettings()
{
    m_LocationSel->LoadSettings();
}

void CMuscleToolPanel::OnMusclePathClick( wxCommandEvent& WXUNUSED(event) )
{
    wxTextCtrl* textCtrl = dynamic_cast<wxTextCtrl*>(FindWindow(ID_TEXTCTRL12));
    _ASSERT(nullptr != textCtrl);

    wxString path = textCtrl->GetValue();

    wxFileDialog dlg(this, wxT("Select a file"), wxT(""), wxT(""), wxT("*"),
        wxFD_OPEN|wxFD_FILE_MUST_EXIST);

    dlg.SetPath(path);

    if (dlg.ShowModal() != wxID_OK)
        return;

    path = dlg.GetPath();
    textCtrl->SetValue(path);
}

void CMuscleToolPanel::RestoreDefaults()
{
    GetData().Init();
    TransferDataToWindow();
}

END_NCBI_SCOPE
