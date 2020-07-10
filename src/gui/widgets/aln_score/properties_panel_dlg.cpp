/*  $Id: properties_panel_dlg.cpp 37492 2017-01-13 21:51:26Z shkeda $
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
 * Authors:  Andrey Yazhuk
 *
 * File Description:
 */

#include <ncbi_pch.hpp>

#include <gui/widgets/aln_score/properties_panel_dlg.hpp>
#include <gui/widgets/wx/ui_tool.hpp>

#include <wx/sizer.h>
#include <wx/statline.h>
#include <wx/button.h>

BEGIN_NCBI_SCOPE

BEGIN_EVENT_TABLE( CPropertiesPanelDlg, wxDialog )
    EVT_BUTTON( wxID_OK, CPropertiesPanelDlg::OnOkClick)
END_EVENT_TABLE()

CPropertiesPanelDlg::CPropertiesPanelDlg(wxWindow* parent, IUIToolWithGUI* metod)
    : m_PropPanel()
{
    Create(parent, wxID_ANY, _("Coloring Method Properties"),
           wxDefaultPosition, wxDefaultSize,
           wxCAPTION|wxSYSTEM_MENU|wxCLOSE_BOX);

    SetExtraStyle(wxWS_EX_VALIDATE_RECURSIVELY|GetExtraStyle());

    CreateControls(metod);
    if (GetSizer())
    {
        GetSizer()->SetSizeHints(this);
    }
    Centre();
}

void CPropertiesPanelDlg::CreateControls(IUIToolWithGUI* metod)
{
    wxBoxSizer* itemBoxSizer1 = new wxBoxSizer(wxVERTICAL);
    this->SetSizer(itemBoxSizer1);

    m_PropPanel = metod->CreatePropertiesPanel(this);
    itemBoxSizer1->Add(m_PropPanel, 1, wxGROW|wxALL, 5);
    itemBoxSizer1->Add(new wxStaticLine(this), 0, wxGROW|wxALL, 5);

    wxBoxSizer* itemBoxSizer2 = new wxBoxSizer(wxHORIZONTAL);
    itemBoxSizer1->Add(itemBoxSizer2, 0, wxALIGN_RIGHT|wxALL, 0);

    itemBoxSizer2->Add(new wxButton(this, wxID_OK, _("&OK")), 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);
    itemBoxSizer2->Add(new wxButton(this, wxID_CANCEL, _("&Cancel")), 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);
}

void CPropertiesPanelDlg::OnOkClick ( wxCommandEvent& WXUNUSED(event) )
{
    if (Validate() && TransferDataFromWindow()) {
        wxCommandEvent cmdApply(wxEVT_COMMAND_BUTTON_CLICKED, wxID_APPLY);
        m_PropPanel->GetEventHandler()->ProcessEvent(cmdApply);
        EndModal(wxID_OK);
    }
}

END_NCBI_SCOPE
