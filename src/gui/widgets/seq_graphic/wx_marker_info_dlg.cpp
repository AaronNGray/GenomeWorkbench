/*  $Id: wx_marker_info_dlg.cpp 27952 2013-04-30 16:42:49Z wuliangs $
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
 * Authors:  Liangshou Wu
 *
 * File Description:
 *
 */
#include <ncbi_pch.hpp>////@begin includes
////@end includes

#include <wx/stattext.h>
#include <wx/button.h>
#include <wx/sizer.h>
#include <wx/frame.h>
#include <wx/scrolwin.h>

#include <gui/widgets/wx/wx_utils.hpp>
#include "wx_marker_info_dlg.hpp"

////@begin XPM images
////@end XPM images

BEGIN_NCBI_SCOPE

IMPLEMENT_DYNAMIC_CLASS(CSeqMarkerDlgEvent, wxCommandEvent)
DEFINE_EVENT_TYPE(wxEVT_COMMAND_REMOVE_ALL_MARKERS)
DEFINE_EVENT_TYPE(wxEVT_COMMAND_MARKER_INFO_CLOSE)


IMPLEMENT_DYNAMIC_CLASS( CwxMarkerInfoDlg, wxDialog )

BEGIN_EVENT_TABLE( CwxMarkerInfoDlg, wxDialog )

////@begin CwxMarkerInfoDlg event table entries
    EVT_CLOSE( CwxMarkerInfoDlg::OnCloseWindow )

    EVT_BUTTON( RemoveAllButton, CwxMarkerInfoDlg::OnRemoveAllButtonClick )

    EVT_BUTTON( wxID_CANCEL, CwxMarkerInfoDlg::OnCancelClick )

////@end CwxMarkerInfoDlg event table entries

END_EVENT_TABLE()

CwxMarkerInfoDlg::CwxMarkerInfoDlg() : m_EventHandler(NULL)
{
    Init();
}

CwxMarkerInfoDlg::CwxMarkerInfoDlg(wxWindow* parent, wxEvtHandler* eventHandler,
                                   wxWindowID id, const wxString& caption,
                                   const wxPoint& pos, const wxSize& size, long style )
                                   : m_EventHandler(eventHandler)
{
    Init();
    Create(parent, id, caption, pos, size, style);
}

bool CwxMarkerInfoDlg::Create( wxWindow* parent, wxWindowID id,
                              const wxString& caption, const wxPoint& pos,
                              const wxSize& size, long style )
{
////@begin CwxMarkerInfoDlg creation
    SetExtraStyle(wxWS_EX_BLOCK_EVENTS);
    wxDialog::Create( parent, id, caption, pos, size, style );

    CreateControls();
    Centre();
////@end CwxMarkerInfoDlg creation
    return true;
}

CwxMarkerInfoDlg::~CwxMarkerInfoDlg()
{
////@begin CwxMarkerInfoDlg destruction
////@end CwxMarkerInfoDlg destruction
}

void CwxMarkerInfoDlg::Init()
{
////@begin CwxMarkerInfoDlg member initialisation
    m_TopPanel = NULL;
    m_PanelSizer = NULL;
////@end CwxMarkerInfoDlg member initialisation
}

void CwxMarkerInfoDlg::CreateControls()
{    
////@begin CwxMarkerInfoDlg content construction
    CwxMarkerInfoDlg* itemDialog1 = this;

    wxBoxSizer* itemBoxSizer2 = new wxBoxSizer(wxVERTICAL);
    itemDialog1->SetSizer(itemBoxSizer2);

    wxBoxSizer* itemBoxSizer3 = new wxBoxSizer(wxVERTICAL);
    itemBoxSizer2->Add(itemBoxSizer3, 0, wxGROW|wxALL, 5);

    wxBoxSizer* itemBoxSizer4 = new wxBoxSizer(wxHORIZONTAL);
    itemBoxSizer3->Add(itemBoxSizer4, 0, wxGROW, 5);

    itemBoxSizer4->Add(5, 5, 1, wxGROW|wxALL, 5);

    wxButton* itemButton6 = new wxButton( itemDialog1, RemoveAllButton, _("Remove all markers"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer4->Add(itemButton6, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxStaticLine* itemStaticLine7 = new wxStaticLine( itemDialog1, wxID_STATIC, wxDefaultPosition, wxSize(600, -1), wxLI_HORIZONTAL );
    itemBoxSizer3->Add(itemStaticLine7, 0, wxGROW|wxLEFT|wxRIGHT|wxBOTTOM, 5);

    m_TopPanel = new wxScrolledWindow( itemDialog1, ID_SCROLLEDWINDOW, wxDefaultPosition, wxSize(100, 100), wxHSCROLL|wxVSCROLL );
    itemBoxSizer2->Add(m_TopPanel, 1, wxGROW|wxALL, 5);
    m_TopPanel->SetScrollbars(1, 1, 0, 0);
    m_PanelSizer = new wxBoxSizer(wxVERTICAL);
    m_TopPanel->SetSizer(m_PanelSizer);

    m_TopPanel->FitInside();

    wxBoxSizer* itemBoxSizer10 = new wxBoxSizer(wxHORIZONTAL);
    itemBoxSizer2->Add(itemBoxSizer10, 0, wxALIGN_RIGHT|wxALL, 5);

    wxButton* itemButton11 = new wxButton( itemDialog1, wxID_CANCEL, _("Close"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer10->Add(itemButton11, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

////@end CwxMarkerInfoDlg content construction
}


void CwxMarkerInfoDlg::UpdateMarker(const SMarkerInfoVec& info_vec)
{
    if (info_vec.empty()) {
        RemoveAllMarkers();
        return;
    }

    list<CwxMarkerInfoPanel*>::iterator panel_iter = m_MarkerPanels.begin();
    ITERATE (SMarkerInfoVec, iter, info_vec) {
        const SMarkerInfo& marker_info = *iter;
        CwxMarkerInfoPanel* panel = NULL;
        while (panel_iter != m_MarkerPanels.end()) {
            if ((*panel_iter)->GetMarkerId() == marker_info.m_Id) {
                panel = *panel_iter;
                break;
            }
            m_PanelSizer->GetItem(*panel_iter)->DeleteWindows();
            panel_iter = m_MarkerPanels.erase(panel_iter);
        }

        if (panel == NULL) {
            panel = new CwxMarkerInfoPanel(ToWxString(marker_info.m_Id),
                ToWxString(marker_info.m_Label),
                marker_info.m_Pos, m_TopPanel, m_EventHandler);
            m_PanelSizer->Add(panel, 0, wxLEFT|wxRIGHT|wxEXPAND);
            panel_iter = m_MarkerPanels.insert(panel_iter, panel);
        }
        panel->UpdateMarker(marker_info);
        ++panel_iter;
    }

    // remove the reest markers
    while (panel_iter != m_MarkerPanels.end()) {
        m_PanelSizer->GetItem(*panel_iter)->DeleteWindows();
        panel_iter = m_MarkerPanels.erase(panel_iter);
    }
    Layout();
}


void CwxMarkerInfoDlg::RemoveAllMarkers()
{
    m_PanelSizer->Clear(true);
    m_MarkerPanels.clear();
}


bool CwxMarkerInfoDlg::Show(bool show)
{
#ifdef __WXOSX_COCOA__
    wxFrame* f = NULL;
    wxWindow* pp = GetParent();
    while (f==NULL  &&  pp !=NULL) {
        f = dynamic_cast<wxFrame*>(pp);
        pp = pp->GetParent();
    }

    if (f != NULL) {
        if (show) {
            ncbi::AddChildWindowAbove(f, this);
        } else {
            ncbi::RemoveChildWindow(f, this);
        }
    }
#endif
    return wxWindow::Show(show);
}


bool CwxMarkerInfoDlg::ShowToolTips()
{
    return true;
}
wxBitmap CwxMarkerInfoDlg::GetBitmapResource( const wxString& name )
{
    // Bitmap retrieval
////@begin CwxMarkerInfoDlg bitmap retrieval
    wxUnusedVar(name);
    return wxNullBitmap;
////@end CwxMarkerInfoDlg bitmap retrieval
}
wxIcon CwxMarkerInfoDlg::GetIconResource( const wxString& name )
{
    // Icon retrieval
////@begin CwxMarkerInfoDlg icon retrieval
    wxUnusedVar(name);
    return wxNullIcon;
////@end CwxMarkerInfoDlg icon retrieval
}

void CwxMarkerInfoDlg::OnRemoveAllButtonClick( wxCommandEvent& event )
{
    //RemoveAllMarkers();
    CSeqMarkerEvent dlg_event(wxEVT_COMMAND_REMOVE_ALL_MARKERS, GetId());
    x_Send(dlg_event);
}

void CwxMarkerInfoDlg::OnCloseWindow( wxCloseEvent& event )
{
    CSeqMarkerEvent dlg_event(wxEVT_COMMAND_MARKER_INFO_CLOSE, GetId());
    x_Send(dlg_event);
}

void CwxMarkerInfoDlg::OnCancelClick( wxCommandEvent& event )
{
    CSeqMarkerEvent dlg_event(wxEVT_COMMAND_MARKER_INFO_CLOSE, GetId());
    x_Send(dlg_event);
}


void CwxMarkerInfoDlg::x_Send(CSeqMarkerEvent& event)
{
    if (m_EventHandler) {
        m_EventHandler->ProcessEvent(event);
    }
}

END_NCBI_SCOPE



