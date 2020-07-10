/*  $Id: glpanewidget_demo_panel.cpp 41269 2018-06-26 19:32:42Z katargir $
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

#include "glpanewidget_demo_panel.hpp"
#include "glpanewidget_demo.hpp"

BEGIN_NCBI_SCOPE

#define ID_GLPANEWIDGET_WINDOW  wxID_HIGHEST + 1

//BEGIN_EVENT_TABLE(CGlPaneWidgetDemoPanel, wxPanel)
//END_EVENT_TABLE()


CGlPaneWidgetDemoPanel::CGlPaneWidgetDemoPanel(wxWindow* parent, wxWindowID id) : m_Widget()
{
    Create(parent, id);
}

bool CGlPaneWidgetDemoPanel::Create(wxWindow* parent, wxWindowID id)
{
    wxPanel::Create(parent, id, wxDefaultPosition, wxSize(0,0), 0);
    CreateControls();
    return true;
}

void CGlPaneWidgetDemoPanel::CreateControls()
{
    wxBoxSizer *top_sizer = new wxBoxSizer(wxVERTICAL);
    wxBoxSizer *bar_sizer = new wxBoxSizer(wxHORIZONTAL);

    wxStaticText* text = new wxStaticText(this, wxID_ANY,
                                          wxT("Placeholder for controls"));
    bar_sizer->Add(text, 0, wxALL|wxALIGN_CENTER_VERTICAL, 5);
    top_sizer->Add(bar_sizer, 0, wxALIGN_RIGHT);

    m_Widget = new CGlWidgetDemo(this, ID_GLPANEWIDGET_WINDOW);

    top_sizer->Add(m_Widget, 1, wxEXPAND);

    SetSizer(top_sizer);
    Bind(wxEVT_IDLE, &CGlPaneWidgetDemoPanel::x_Activate, this);
}

void CGlPaneWidgetDemoPanel::x_Activate(wxIdleEvent& event)
{
    if (!m_Widget || !IsShown()) {
        event.Skip();
        return;
    }

    Unbind(wxEVT_IDLE, &CGlPaneWidgetDemoPanel::x_Activate, this);

    m_Widget->Create();
    m_Widget->ZoomRect(TModelRect(0.0, 0.0, 200.0, 200.0));

    Layout();
}

END_NCBI_SCOPE

