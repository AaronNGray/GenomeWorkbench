/*  $Id: ok_cancel_panel.cpp 40191 2018-01-04 21:03:28Z asztalos $
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

#include <wx/button.h>
#include <wx/sizer.h>
#include <wx/msgdlg.h>
#include <gui/widgets/data/report_dialog.hpp>
#include <gui/packages/pkg_sequence_edit/ok_cancel_panel.hpp>
#include <gui/packages/pkg_sequence_edit/bulk_cmd_dlg.hpp>

////@begin XPM images
////@end XPM images

BEGIN_NCBI_SCOPE

USING_SCOPE(ncbi::objects);

/*!
 * COkCancelPanel type definition
 */

IMPLEMENT_DYNAMIC_CLASS( COkCancelPanel, wxPanel )


/*!
 * COkCancelPanel event table definition
 */

BEGIN_EVENT_TABLE( COkCancelPanel, wxPanel )

////@begin COkCancelPanel event table entries
    EVT_BUTTON( ID_ACCEPT_BTN, COkCancelPanel::OnAcceptBtnClick )
    EVT_BUTTON( wxID_CLOSE, COkCancelPanel::OnClickCancel )
////@end COkCancelPanel event table entries

END_EVENT_TABLE()


/*!
 * COkCancelPanel constructors
 */

COkCancelPanel::COkCancelPanel()
{
    Init();
}

COkCancelPanel::COkCancelPanel( wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style )
{
    Init();
    Create(parent, id, pos, size, style);
}


/*!
 * COkCancelPanel creator
 */

bool COkCancelPanel::Create( wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style )
{
////@begin COkCancelPanel creation
    wxPanel::Create( parent, id, pos, size, style );

    CreateControls();
    if (GetSizer())
    {
        GetSizer()->SetSizeHints(this);
    }
    Centre();
////@end COkCancelPanel creation
    return true;
}


/*!
 * COkCancelPanel destructor
 */

COkCancelPanel::~COkCancelPanel()
{
////@begin COkCancelPanel destruction
////@end COkCancelPanel destruction
}


/*!
 * Member initialisation
 */

void COkCancelPanel::Init()
{
////@begin COkCancelPanel member initialisation
    m_LeaveUp = NULL;
////@end COkCancelPanel member initialisation
}


/*!
 * Control creation for COkCancelPanel
 */

void COkCancelPanel::CreateControls()
{    
////@begin COkCancelPanel content construction
    COkCancelPanel* itemPanel1 = this;

    wxBoxSizer* itemBoxSizer2 = new wxBoxSizer(wxVERTICAL);
    itemPanel1->SetSizer(itemBoxSizer2);

    wxBoxSizer* itemBoxSizer3 = new wxBoxSizer(wxHORIZONTAL);
    itemBoxSizer2->Add(itemBoxSizer3, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

    wxButton* itemButton4 = new wxButton( itemPanel1, ID_ACCEPT_BTN, _("Accept"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer3->Add(itemButton4, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxButton* itemButton5 = new wxButton( itemPanel1, wxID_CLOSE, _("&Close"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer3->Add(itemButton5, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    m_LeaveUp = new wxCheckBox( itemPanel1, ID_LEAVE_DIALOG_UP_CHKBOX, _("Leave Dialog Up"), wxDefaultPosition, wxDefaultSize, 0 );
    m_LeaveUp->SetValue(false);
    itemBoxSizer3->Add(m_LeaveUp, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

////@end COkCancelPanel content construction
}


/*!
 * Should we show tooltips?
 */

bool COkCancelPanel::ShowToolTips()
{
    return true;
}

/*!
 * Get bitmap resources
 */

wxBitmap COkCancelPanel::GetBitmapResource( const wxString& name )
{
    // Bitmap retrieval
////@begin COkCancelPanel bitmap retrieval
    wxUnusedVar(name);
    return wxNullBitmap;
////@end COkCancelPanel bitmap retrieval
}

/*!
 * Get icon resources
 */

wxIcon COkCancelPanel::GetIconResource( const wxString& name )
{
    // Icon retrieval
////@begin COkCancelPanel icon retrieval
    wxUnusedVar(name);
    return wxNullIcon;
////@end COkCancelPanel icon retrieval
}


/*!
 * wxEVT_COMMAND_BUTTON_CLICKED event handler for wxID_OK
 */

void COkCancelPanel::OnAcceptBtnClick( wxCommandEvent& event )
{
    x_ReportUsage();
    wxWindow* w = this->GetParent();
    while (w != NULL) {
        CBulkCmdDlg* parent = dynamic_cast<CBulkCmdDlg*>(w);
        if (parent) {
            if (parent->GetTopLevelSeqEntryAndProcessor()) {
                CRef<CCmdComposite> cmd = parent->GetCommand();
                if (cmd) {
                    parent->ExecuteCmd(cmd);
                    if (!m_LeaveUp->GetValue()) {
                        parent->Destroy();
                    }
                } else {
                    string error = parent->GetErrorMessage();
                    if (!NStr::IsBlank(error)) {
                        wxMessageBox(ToWxString(error), wxT("Error"),
                                     wxOK | wxICON_ERROR, NULL);
                    }
                }
            } else {
                wxMessageBox(ToWxString("No data was selected"), 
                    wxT("Error"), wxOK | wxICON_ERROR, NULL);
            }
            return;
        } else {
            w = w->GetParent();
        }
    }
}


/*!
 * wxEVT_COMMAND_BUTTON_CLICKED event handler for wxID_CLOSE
 */

void COkCancelPanel::OnClickCancel( wxCommandEvent& event )
{
    wxWindow* w = this->GetParent();
    while (w != NULL) {
        CBulkCmdDlg* parent = dynamic_cast<CBulkCmdDlg*>(w);
        if (parent) {
            parent->Destroy();
            return;
        } else {
            w = w->GetParent();
        }
    }
}

void COkCancelPanel::x_ReportUsage()
{
    wxWindow* win = this->GetParent();
    while (win != nullptr) {
        wxDialog* dlg_parent = dynamic_cast<wxDialog*>(win);
        if (dlg_parent) {
            NEditingStats::ReportUsage(dlg_parent->GetLabel());
            break;
        }
        else {
            wxFrame* fr_parent = dynamic_cast<wxFrame*>(win);
            if (fr_parent) {
                NEditingStats::ReportUsage(fr_parent->GetLabel());
                break;
            }
            else {
                win = win->GetParent();
            }
        }
    }
}

END_NCBI_SCOPE


