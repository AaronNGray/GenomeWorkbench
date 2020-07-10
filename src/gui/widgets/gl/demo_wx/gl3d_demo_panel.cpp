/*  $Id: gl3d_demo_panel.cpp 25560 2012-04-09 14:28:39Z katargir $
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
#include <wx/textctrl.h>
#include <wx/valtext.h>
#include <wx/choice.h>

#include "gl3d_demo_panel.hpp"
#include "gl3d_window.hpp"

BEGIN_NCBI_SCOPE

#define ID_GL3D_WINDOW  wxID_HIGHEST + 1
#define ID_X_ROTATION   wxID_HIGHEST + 2
#define ID_Y_ROTATION   wxID_HIGHEST + 3
#define ID_Z_ROTATION   wxID_HIGHEST + 4
#define ID_CHOICE_PROJ  wxID_HIGHEST + 5

BEGIN_EVENT_TABLE(CGl3dDemoPanel, wxPanel)
    EVT_CHOICE( ID_CHOICE_PROJ, CGl3dDemoPanel::OnProjSelected )
    EVT_TEXT_ENTER( ID_X_ROTATION, CGl3dDemoPanel::OnXRotationEnter )
    EVT_TEXT_ENTER( ID_Y_ROTATION, CGl3dDemoPanel::OnYRotationEnter )
    EVT_TEXT_ENTER( ID_Z_ROTATION, CGl3dDemoPanel::OnZRotationEnter )
END_EVENT_TABLE()

CGl3dDemoPanel::CGl3dDemoPanel()
{
}

CGl3dDemoPanel::CGl3dDemoPanel(wxWindow* parent, wxWindowID id)
{
    Create(parent, id);
}

bool CGl3dDemoPanel::Create(wxWindow* parent, wxWindowID id)
{
    wxPanel::Create(parent, id, wxDefaultPosition, wxSize(0,0), 0);
    CreateControls();
    return true;
}

void CGl3dDemoPanel::CreateControls()
{
    wxBoxSizer *topSizer = new wxBoxSizer(wxVERTICAL);
    wxBoxSizer *barSizer = new wxBoxSizer(wxHORIZONTAL);

    wxArrayString projChoiceStrings;
    projChoiceStrings.Add(wxT("Perspective"));
    projChoiceStrings.Add(wxT("Orthographic"));
    wxChoice *projChoice = new wxChoice(this, ID_CHOICE_PROJ,
                                        wxDefaultPosition, wxDefaultSize,
                                        projChoiceStrings, 0);
    projChoice->SetSelection(0);
    barSizer->Add(projChoice, 0, wxALL|wxALIGN_CENTER_VERTICAL, 5);

    wxTextValidator validator(wxFILTER_NUMERIC);

    barSizer->Add(new wxStaticText(this, wxID_ANY, wxT("X Rot")), 0, wxALL|wxALIGN_CENTER_VERTICAL, 1);
    wxTextCtrl* itemTextXRot = new wxTextCtrl(this, ID_X_ROTATION, wxT("0"), wxDefaultPosition, wxSize(-1, -1), wxTE_PROCESS_ENTER, validator);
    itemTextXRot->SetMaxLength(7);
    barSizer->Add(itemTextXRot, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    barSizer->Add(new wxStaticText(this, wxID_ANY, wxT("Y Rot")), 0, wxALL|wxALIGN_CENTER_VERTICAL, 1);
    wxTextCtrl* itemTextYRot = new wxTextCtrl(this, ID_Y_ROTATION, wxT("0"), wxDefaultPosition, wxSize(-1, -1), wxTE_PROCESS_ENTER, validator);
    itemTextYRot->SetMaxLength(7);
    barSizer->Add(itemTextYRot, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    barSizer->Add(new wxStaticText(this, wxID_ANY, wxT("Z Rot")), 0, wxALL|wxALIGN_CENTER_VERTICAL, 1);
    wxTextCtrl* itemTextZRot = new wxTextCtrl(this, ID_Z_ROTATION, wxT("0"), wxDefaultPosition, wxSize(-1, -1), wxTE_PROCESS_ENTER, validator);
    itemTextZRot->SetMaxLength(7);
    barSizer->Add(itemTextZRot, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    topSizer->Add(barSizer, 0, wxALIGN_RIGHT);

    CGl3dWindow *gl3dWindow = new CGl3dWindow(this, ID_GL3D_WINDOW);
    topSizer->Add(gl3dWindow, 1, wxEXPAND);

    wxCoord w, h;
    itemTextXRot->GetTextExtent(wxT("WWWWWW"), &w, &h);
    wxSize textSize(w, wxDefaultCoord);
    itemTextXRot->SetMinSize(textSize);
    itemTextYRot->SetMinSize(textSize);
    itemTextZRot->SetMinSize(textSize);

    SetSizer(topSizer);
}

void CGl3dDemoPanel::OnXRotationEnter(wxCommandEvent& WXUNUSED(event))
{
    wxTextCtrl* itemText = (wxTextCtrl*)FindWindow(ID_X_ROTATION);
    wxString str = itemText->GetValue();
    double value;
    if (str.ToDouble(&value)) {
        CGl3dWindow *gl3dWindow = (CGl3dWindow*)FindWindow(ID_GL3D_WINDOW);
        gl3dWindow->SetXRot((float)value);
    }
}

void CGl3dDemoPanel::OnYRotationEnter(wxCommandEvent& WXUNUSED(event))
{
    wxTextCtrl* itemText = (wxTextCtrl*)FindWindow(ID_Y_ROTATION);
    wxString str = itemText->GetValue();
    double value;
    if (str.ToDouble(&value)) {
        CGl3dWindow *gl3dWindow = (CGl3dWindow*)FindWindow(ID_GL3D_WINDOW);
        gl3dWindow->SetYRot((float)value);
    }
}

void CGl3dDemoPanel::OnZRotationEnter(wxCommandEvent& WXUNUSED(event))
{
    wxTextCtrl* itemText = (wxTextCtrl*)FindWindow(ID_Z_ROTATION);
    wxString str = itemText->GetValue();
    double value;
    if (str.ToDouble(&value)) {
        CGl3dWindow *gl3dWindow = (CGl3dWindow*)FindWindow(ID_GL3D_WINDOW);
        gl3dWindow->SetZRot((float)value);
    }
}

void CGl3dDemoPanel::OnProjSelected(wxCommandEvent& event)
{
    CGl3dWindow *gl3dWindow = (CGl3dWindow*)FindWindow(ID_GL3D_WINDOW);
    int value = event.GetSelection();
    if (value == 0)
        gl3dWindow->SetPerspective();
    else
        gl3dWindow->SetOrtho();
}

END_NCBI_SCOPE

