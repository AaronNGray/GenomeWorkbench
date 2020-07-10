/*  $Id: wx_marker_info_panel.cpp 27951 2013-04-30 16:40:07Z wuliangs $
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
#include <ncbi_pch.hpp>////@begin includes
////@end includes

#include <wx/grid.h>
#include <wx/stattext.h>
#include <wx/button.h>
#include <wx/sizer.h>
#include <gui/widgets/wx/wx_utils.hpp>

#include "wx_marker_info_panel.hpp"

////@begin XPM images
////@end XPM images

BEGIN_NCBI_SCOPE

IMPLEMENT_DYNAMIC_CLASS(CSeqMarkerEvent, wxCommandEvent)
DEFINE_EVENT_TYPE(wxEVT_COMMAND_REMOVE_MARKER)

IMPLEMENT_DYNAMIC_CLASS( CwxMarkerInfoPanel, wxPanel )

BEGIN_EVENT_TABLE( CwxMarkerInfoPanel, wxPanel )

////@begin CwxMarkerInfoPanel event table entries
    EVT_BUTTON( RemoveButton, CwxMarkerInfoPanel::OnRemoveButtonClick )

////@end CwxMarkerInfoPanel event table entries

END_EVENT_TABLE()

CwxMarkerInfoPanel::CwxMarkerInfoPanel()
    : m_EventHandler(NULL)
    , m_MarkerPos(0)
{
    Init();
}


CwxMarkerInfoPanel::CwxMarkerInfoPanel(const wxString& marker_id, const wxString& marker_label,
                                       size_t marker_pos, wxWindow* parent,
                                       wxEvtHandler* eventHandler, wxWindowID id,
                                       const wxPoint& pos, const wxSize& size, long style )
                                       : m_EventHandler(eventHandler)
                                       , m_MarkerId(marker_id)
                                       , m_MarkerLabel(marker_label)
                                       , m_MarkerPos(marker_pos)
{
    Init();
    Create(parent, id, pos, size, style);
}

bool CwxMarkerInfoPanel::Create(wxWindow* parent, wxWindowID id,
                                const wxPoint& pos, const wxSize& size, long style )
{
////@begin CwxMarkerInfoPanel creation
    wxPanel::Create( parent, id, pos, size, style );

    CreateControls();
    Centre();
////@end CwxMarkerInfoPanel creation
    return true;
}

CwxMarkerInfoPanel::~CwxMarkerInfoPanel()
{
////@begin CwxMarkerInfoPanel destruction
////@end CwxMarkerInfoPanel destruction
}

void CwxMarkerInfoPanel::Init()
{
////@begin CwxMarkerInfoPanel member initialisation
    m_MarkerName = NULL;
    m_MarkerInfoGrid = NULL;
////@end CwxMarkerInfoPanel member initialisation
}

void CwxMarkerInfoPanel::CreateControls()
{    
////@begin CwxMarkerInfoPanel content construction
    CwxMarkerInfoPanel* itemPanel1 = this;

    wxBoxSizer* itemBoxSizer2 = new wxBoxSizer(wxVERTICAL);
    itemPanel1->SetSizer(itemBoxSizer2);

    wxPanel* itemPanel3 = new wxPanel( itemPanel1, ID_PANEL, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL );
    itemPanel3->SetBackgroundColour(wxColour(193, 193, 193));
    itemBoxSizer2->Add(itemPanel3, 0, wxGROW, 5);

    wxBoxSizer* itemBoxSizer4 = new wxBoxSizer(wxHORIZONTAL);
    itemPanel3->SetSizer(itemBoxSizer4);

    m_MarkerName = new wxStaticText( itemPanel3, wxID_STATIC, _("Marker1"), wxDefaultPosition, wxDefaultSize, wxALIGN_LEFT );
    itemBoxSizer4->Add(m_MarkerName, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    itemBoxSizer4->Add(5, 5, 1, wxGROW, 5);

    wxButton* itemButton7 = new wxButton( itemPanel3, RemoveButton, _("Remove"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer4->Add(itemButton7, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    m_MarkerInfoGrid = new wxGrid( itemPanel1, ID_GRID, wxDefaultPosition, wxDefaultSize, 0 );
    m_MarkerInfoGrid->SetDefaultColSize(120);
    m_MarkerInfoGrid->SetDefaultRowSize(25);
    m_MarkerInfoGrid->SetColLabelSize(25);
    m_MarkerInfoGrid->SetRowLabelSize(50);
    itemBoxSizer2->Add(m_MarkerInfoGrid, 1, wxGROW, 5);

    itemBoxSizer2->Add(5, 10, 0, wxGROW|wxLEFT|wxRIGHT, 5);

////@end CwxMarkerInfoPanel content construction
    m_MarkerInfoGrid->HideRowLabels();
    m_MarkerInfoGrid->CreateGrid(0, 5);
    m_MarkerInfoGrid->SetColLabelValue(0, wxT("Accession/Locus Tag"));
    m_MarkerInfoGrid->SetColLabelValue(1, wxT("Location"));
    m_MarkerInfoGrid->SetColLabelValue(2, wxT("Relative to"));
    m_MarkerInfoGrid->SetColLabelValue(3, wxT("HGVS name"));
    m_MarkerInfoGrid->SetColLabelValue(4, wxT("Sequence"));
    m_MarkerInfoGrid->AutoSize();
    x_UpdateMarkerLabel();
}


void CwxMarkerInfoPanel::UpdateMarker(const SMarkerInfo& marker_info)
{
    m_MarkerLabel = ToWxString(marker_info.m_Label);
    m_MarkerPos = marker_info.m_Pos;
    const SMarkerCoordVec& coord_vec = marker_info.m_CoordVec;
    x_UpdateMarkerLabel();

    wxGridTableBase* table = m_MarkerInfoGrid->GetTable();
    size_t curr_rows = (size_t)table->GetRowsCount();
    int rows_needed = (int)marker_info.m_CoordVec.size();
    if (curr_rows > 0) {
        table->Clear();
    }
    if (curr_rows > rows_needed) {
        table->DeleteRows(rows_needed, curr_rows - rows_needed);
    } else if (curr_rows < rows_needed) {
        table->AppendRows(rows_needed - curr_rows);
    }
    for (size_t row = 0; row < rows_needed; ++row) {
        table->SetValue((int)row, 0, ToWxString(coord_vec[row].m_Acc));
        table->SetValue((int)row, 1, ToWxString(coord_vec[row].m_Loc));
        table->SetValue((int)row, 2, ToWxString(coord_vec[row].m_LocRef));
        table->SetValue((int)row, 3, ToWxString(coord_vec[row].m_Acc) + wxT(":") + ToWxString(coord_vec[row].m_Hgvs));
        string seq = coord_vec[row].m_Seq;
        NStr::ReplaceInPlace(seq, "&nbsp;", " ");
        for (int i = 0; i < 2; ++i) {
            size_t tag_start = seq.find_first_of("<");
            if (tag_start != string::npos) {
                size_t tag_stop = seq.find_first_of(">", tag_start);
                if (tag_stop != string::npos) {
                    seq.replace(tag_start, tag_stop - tag_start + 1, i == 0 ? "[" : "]");
                }
            }
        }
        table->SetValue((int)row, 4, ToWxString(seq));
    }
    m_MarkerInfoGrid->AutoSize();
}


string CwxMarkerInfoPanel::GetMarkerId() const
{
    return ToStdString(m_MarkerId);
}


bool CwxMarkerInfoPanel::ShowToolTips()
{
    return true;
}
wxBitmap CwxMarkerInfoPanel::GetBitmapResource( const wxString& name )
{
    // Bitmap retrieval
////@begin CwxMarkerInfoPanel bitmap retrieval
    wxUnusedVar(name);
    return wxNullBitmap;
////@end CwxMarkerInfoPanel bitmap retrieval
}
wxIcon CwxMarkerInfoPanel::GetIconResource( const wxString& name )
{
    // Icon retrieval
////@begin CwxMarkerInfoPanel icon retrieval
    wxUnusedVar(name);
    return wxNullIcon;
////@end CwxMarkerInfoPanel icon retrieval
}

void CwxMarkerInfoPanel::OnRemoveButtonClick( wxCommandEvent& event )
{
    if (m_EventHandler) {
        CSeqMarkerEvent* dlg_event(new CSeqMarkerEvent(wxEVT_COMMAND_REMOVE_MARKER, GetId()));
        dlg_event->SetMarkerId(GetMarkerId());
        m_EventHandler->QueueEvent(dlg_event);
    }
}


void CwxMarkerInfoPanel::x_UpdateMarkerLabel()
{
    wxString label = m_MarkerLabel + wxT(" (");
    label += ToWxString(NStr::NumericToString(m_MarkerPos));
    label += wxT(")");
    m_MarkerName->SetLabel(label);
}

END_NCBI_SCOPE


