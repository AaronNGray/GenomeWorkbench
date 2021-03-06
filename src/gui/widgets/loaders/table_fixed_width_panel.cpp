/*  $Id: table_fixed_width_panel.cpp 43699 2019-08-14 20:17:05Z katargir $
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
 * Authors: Bob Falk
 *
 * File Description:
 *
 */

#include <ncbi_pch.hpp>////@begin includes
#include "wx/imaglist.h"
////@end includes

#include "table_fixed_width_panel.hpp"
#include <gui/objutils/registry.hpp>

#include <wx/sizer.h>
#include <wx/stattext.h>
#include <wx/statbox.h>
#include <wx/icon.h>
#include <wx/bitmap.h>
#include <wx/msgdlg.h>
#include <wx/listctrl.h>
#include <wx/textctrl.h>

BEGIN_NCBI_SCOPE
USING_SCOPE(objects);

////@begin XPM images
////@end XPM images
IMPLEMENT_DYNAMIC_CLASS( CTableFixedWidthPanel, wxPanel )

BEGIN_EVENT_TABLE( CTableFixedWidthPanel, wxPanel )

////@begin CTableFixedWidthPanel event table entries
    EVT_IDLE( CTableFixedWidthPanel::OnIdle )
    EVT_TOGGLEBUTTON( ID_INSERTCOLUMNBTN, CTableFixedWidthPanel::OnInsertColumnBtnClick )
    EVT_TOGGLEBUTTON( ID_DELETECOLUMNBTN, CTableFixedWidthPanel::OnDeleteColumnBtnClick )
////@end CTableFixedWidthPanel event table entries

    EVT_LIST_COL_CLICK( ID_CTABLEIMPORTLISTCTRL1, CTableFixedWidthPanel::OnCtableImportListctrlColLeftClick )
    EVT_LIST_COL_BEGIN_DRAG( ID_CTABLEIMPORTLISTCTRL1, CTableFixedWidthPanel::OnCtableImportListctrlColBeginDrag )
    EVT_LIST_COL_DRAGGING( ID_CTABLEIMPORTLISTCTRL1, CTableFixedWidthPanel::OnCTableImportListctrlColDragging )
    EVT_LIST_COL_END_DRAG( ID_CTABLEIMPORTLISTCTRL1, CTableFixedWidthPanel::OnCtableImportListctrlColEndDrag )

END_EVENT_TABLE()

CTableFixedWidthPanel::CTableFixedWidthPanel()
{
    Init();
}

CTableFixedWidthPanel::CTableFixedWidthPanel( wxWindow* parent, wxWindowID id, const wxString& caption, const wxPoint& pos, const wxSize& size, long style )
{
    Init();
    Create(parent, id, caption, pos, size, style);
}

bool CTableFixedWidthPanel::Create( wxWindow* parent, wxWindowID id, const wxString& caption, const wxPoint& pos, const wxSize& size, long style )
{
////@begin CTableFixedWidthPanel creation
    SetExtraStyle(wxWS_EX_BLOCK_EVENTS);
    wxPanel::Create( parent, id, pos, size, style );

    CreateControls();
    if (GetSizer())
    {
        GetSizer()->SetSizeHints(this);
    }
    Centre();
////@end CTableFixedWidthPanel creation
    return true;
}

CTableFixedWidthPanel::~CTableFixedWidthPanel()
{
////@begin CTableFixedWidthPanel destruction
////@end CTableFixedWidthPanel destruction
}

void CTableFixedWidthPanel::Init()
{
////@begin CTableFixedWidthPanel member initialisation
    m_ListCtrlPanel = NULL;
    m_ClickToInsert = NULL;
    m_ClickToDelete = NULL;
    m_InfoTxt = NULL;
    m_FixedWidthListCtrl = NULL;
////@end CTableFixedWidthPanel member initialisation

    m_ResizeColumnEventCheck = false;
    m_IsDragging = false;
}

void CTableFixedWidthPanel::CreateControls()
{    
////@begin CTableFixedWidthPanel content construction
    // Generated by DialogBlocks, 14/08/2019 16:14:41 (unregistered)

    CTableFixedWidthPanel* itemPanel1 = this;

    wxBoxSizer* itemBoxSizer2 = new wxBoxSizer(wxVERTICAL);
    itemPanel1->SetSizer(itemBoxSizer2);

    wxStaticText* itemStaticText3 = new wxStaticText( itemPanel1, wxID_STATIC, _("Divide Table Data into Fixed-Width Columns"), wxDefaultPosition, wxDefaultSize, 0 );
    itemStaticText3->SetFont(wxFont(10, wxFONTFAMILY_SWISS, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL, false, wxT("Tahoma")));
    itemBoxSizer2->Add(itemStaticText3, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

    wxStaticText* itemStaticText4 = new wxStaticText( itemPanel1, wxID_STATIC, _("                \"To add columns, click on 'Insert Columns' and then click on a data column to divide it into two columns.\n                To remove columns, click on 'Remove Columns'  and then click on a data column to remove it.\"\n              "), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer2->Add(itemStaticText4, 0, wxALIGN_LEFT|wxALL, 5);

    wxStaticBox* itemStaticBoxSizer5Static = new wxStaticBox(itemPanel1, wxID_ANY, _("Table Data"));
    wxStaticBoxSizer* itemStaticBoxSizer5 = new wxStaticBoxSizer(itemStaticBoxSizer5Static, wxHORIZONTAL);
    itemBoxSizer2->Add(itemStaticBoxSizer5, 1, wxGROW|wxALL, 5);

    m_ListCtrlPanel = new wxPanel( itemStaticBoxSizer5->GetStaticBox(), ID_FIXEDWIDTHTABLEPANEL, wxDefaultPosition, wxDefaultSize, wxNO_BORDER|wxTAB_TRAVERSAL );
    itemStaticBoxSizer5->Add(m_ListCtrlPanel, 1, wxGROW|wxTOP, 5);

    wxBoxSizer* itemBoxSizer7 = new wxBoxSizer(wxVERTICAL);
    m_ListCtrlPanel->SetSizer(itemBoxSizer7);

    wxBoxSizer* itemBoxSizer8 = new wxBoxSizer(wxHORIZONTAL);
    itemBoxSizer7->Add(itemBoxSizer8, 0, wxALIGN_LEFT|wxALL, 5);

    m_ClickToInsert = new wxToggleButton( m_ListCtrlPanel, ID_INSERTCOLUMNBTN, _("Insert Column"), wxDefaultPosition, wxDefaultSize, 0 );
    m_ClickToInsert->SetValue(false);
    if (CTableFixedWidthPanel::ShowToolTips())
        m_ClickToInsert->SetToolTip(_("The next column you click on will be split into two columns"));
    itemBoxSizer8->Add(m_ClickToInsert, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    m_ClickToDelete = new wxToggleButton( m_ListCtrlPanel, ID_DELETECOLUMNBTN, _("Remove Column"), wxDefaultPosition, wxDefaultSize, 0 );
    m_ClickToDelete->SetValue(false);
    if (CTableFixedWidthPanel::ShowToolTips())
        m_ClickToDelete->SetToolTip(_("The next column you click on will be deleted"));
    itemBoxSizer8->Add(m_ClickToDelete, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    itemBoxSizer8->Add(5, 5, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    m_InfoTxt = new wxStaticText( m_ListCtrlPanel, wxID_FIXEDWIDTHINSTRUCTIONS, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
    m_InfoTxt->SetForegroundColour(wxColour(0, 0, 255));
    m_InfoTxt->SetFont(wxFont(10, wxFONTFAMILY_SWISS, wxFONTSTYLE_ITALIC, wxFONTWEIGHT_NORMAL, false, wxT("Tahoma")));
    itemBoxSizer8->Add(m_InfoTxt, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxBoxSizer* itemBoxSizer13 = new wxBoxSizer(wxHORIZONTAL);
    itemBoxSizer7->Add(itemBoxSizer13, 1, wxGROW|wxALL, 5);

    m_FixedWidthListCtrl = new CTableImportListCtrl( m_ListCtrlPanel, ID_CTABLEIMPORTLISTCTRL1, wxDefaultPosition, wxSize(100, 100), 0 );
    itemBoxSizer13->Add(m_FixedWidthListCtrl, 1, wxGROW|wxALL, 5);

////@end CTableFixedWidthPanel content construction
}

bool CTableFixedWidthPanel::Show(bool show)
{
    if (show) {
        if (!m_ImportedTableData.IsNull()) {
            m_FixedWidthListCtrl->SetViewType(CTableImportListCtrl::eMultiColumn);

            // Delete any column headers that may have been already added
            m_FixedWidthListCtrl->SetDataSource(m_ImportedTableData);
        }
    }
    
    return wxPanel::Show(show);
}

void CTableFixedWidthPanel::SetMainTitle(const wxString& title)
{
}

bool CTableFixedWidthPanel::IsInputValid()
{
    // Save the (fixed) position and length info to the datasource 
    // table records
     if (!m_ImportedTableData.IsNull()) {
        m_ImportedTableData->ExtractFixedFields();

        m_ImportedTableData->LogFixedFieldWidths();
     }

    return true;
}


void CTableFixedWidthPanel::SetRegistryPath( const string& path )
{
    m_RegPath = path;
}


void CTableFixedWidthPanel::LoadSettings()
{
    m_FixedWidthListCtrl->SetViewType(CTableImportListCtrl::eSingleColumn);
}


void CTableFixedWidthPanel::SaveSettings() const
{
}


bool CTableFixedWidthPanel::ShowToolTips()
{
    return true;
}
wxBitmap CTableFixedWidthPanel::GetBitmapResource( const wxString& name )
{
    // Bitmap retrieval
////@begin CTableFixedWidthPanel bitmap retrieval
    wxUnusedVar(name);
    return wxNullBitmap;
////@end CTableFixedWidthPanel bitmap retrieval
}
wxIcon CTableFixedWidthPanel::GetIconResource( const wxString& name )
{
    // Icon retrieval
////@begin CTableFixedWidthPanel icon retrieval
    wxUnusedVar(name);
    return wxNullIcon;
////@end CTableFixedWidthPanel icon retrieval
}

void CTableFixedWidthPanel::OnInsertColumnBtnClick( wxCommandEvent& event )
{
    if (event.IsChecked()) {
        m_ClickToDelete->SetValue(false);
        m_InfoTxt->SetLabelText(wxT("Click on a column to split it into two columns"));
    }
    else {
        m_InfoTxt->SetLabelText(wxT(""));
    }
}

void CTableFixedWidthPanel::OnDeleteColumnBtnClick( wxCommandEvent& event )
{
    if (event.IsChecked()) {
        m_ClickToInsert->SetValue(false);
        m_InfoTxt->SetLabelText(wxT("Click on a column to remove the column (but not the data)"));
    }
    else {
        m_InfoTxt->SetLabelText(wxT(""));
    }
}

void CTableFixedWidthPanel::OnCtableImportListctrlColLeftClick( wxListEvent& event )
{
    int col = event.GetColumn();

    if (m_ImportedTableData.IsNull())
        return;
    
    if (m_ClickToDelete->GetValue()) {
        m_FixedWidthListCtrl->MergeColumns(col);
        m_ClickToDelete->SetValue(false);
        m_InfoTxt->SetLabelText(wxT(""));
    }
    else if (m_ClickToInsert->GetValue()) {
        m_FixedWidthListCtrl->DivideColumn(col);
        m_ClickToInsert->SetValue(false);
        m_InfoTxt->SetLabelText(wxT(""));
    }

    m_FixedWidthListCtrl->UpdateColumnWidths();
    m_ListWidgetColPrevWidths = m_FixedWidthListCtrl->GetColumnWidths();
}


void CTableFixedWidthPanel::OnCtableImportListctrlColBeginDrag( wxListEvent& event )
{
    event.Skip(); 

#ifdef __WXMSW__
    m_IsDragging = true;
    m_DragEvent.m_col = event.GetColumn();
#endif
}


void CTableFixedWidthPanel::OnCTableImportListctrlColDragging( wxListEvent& event )
{
    // This event handler may resize other columns based on changes to this one. This
    // check prevents us from handling those events too and going into a loop
    if (m_FixedWidthListCtrl->IgnoreWidthEvent()) {
        event.Skip();
        return;
    }

    // No need to update if there are no changes to any of the widths (Needed
    // especially for windows which is using idle callback since this event
    // isn't being called in windows (wx 2.9.3 only?).
    if (!m_FixedWidthListCtrl->ColumnWidthsUpdated())
        return;

    // If this is the first time we are called, just updat the widths
    vector<int> widths = m_FixedWidthListCtrl->GetColumnWidths();

    if (m_ListWidgetColPrevWidths.size() != widths.size()) {
        m_ListWidgetColPrevWidths = widths;
        event.Skip();
        return;
    }

    // Get change in column width of the column on the left-hand side of the column
    // divider being dragged
    int delta = widths[event.GetColumn()] - m_ListWidgetColPrevWidths[event.GetColumn()];

    //_TRACE("Column dragging " << event.GetColumn() << " Delta: " << delta);
    if ((size_t)event.GetColumn() < widths.size()-1) {
        widths[event.GetColumn() + 1] -= delta;

        int col_width = widths[event.GetColumn() + 1];
        int char_width = col_width/m_FixedWidthListCtrl->GetFontWidth();

        // Don't shrink the column to the right to a size it can't show characters:
        if (char_width >= 3 ) {
            m_FixedWidthListCtrl->SetColumnWidthIgnoreEvent(event.GetColumn() + 1, 
                widths[event.GetColumn() + 1]);
            m_FixedWidthListCtrl->SynchDataSourceColumnWidths();
        }
    }

    m_ImportedTableData->RecomputeHeaders();
    m_FixedWidthListCtrl->UpdateColumnNames();

    m_FixedWidthListCtrl->Refresh();

    // Update previous widths to current. The pevious widths let us track how much 
    // the column width changed each time this is called
    m_ListWidgetColPrevWidths = widths;
    m_FixedWidthListCtrl->UpdateColumnWidths();

    event.Skip();
}

void CTableFixedWidthPanel::OnCtableImportListctrlColEndDrag( wxListEvent& event )
{   
    m_IsDragging = false;

    if (event.GetColumn() == m_FixedWidthListCtrl->GetColumnCount()-1) {
        // Get width of all columns combined and make sure that exceeds the
        // width of the longest line 
        if (m_FixedWidthListCtrl->GetColumnsCombinedCharWidth() < 
            (int)m_ImportedTableData->GetMaxRowLength()) {
                // User has resized the last column to a size that
                // does not show all the text. Vetoing the event in
                // OnCTableImportListctrlColDragging does not work since
                // after 1 veto all dragging seems to stop. (at least on windows)

                // We also can't do it here on enddrag, but we can set up a flag
                // so it is handled in the next call to the idle handler.
                m_ResizeColumnEventCheck = true;
        }
    }
}

void CTableFixedWidthPanel::OnIdle( wxIdleEvent& event )
{
    // The flag to do this is set after a column dragging (resize) event
    // completes.
    if (!m_ResizeColumnEventCheck) {

#ifdef __WXMSW__
        if (m_IsDragging) {
            if (m_DragEvent.GetColumn() < (int)m_FixedWidthListCtrl->GetColumnWidths().size() &&
                m_DragEvent.GetColumn() >= 0)
                    OnCTableImportListctrlColDragging(m_DragEvent);
        }
#endif
        return;
    }

    m_ResizeColumnEventCheck = false;

    int col_idx = m_FixedWidthListCtrl->GetColumnCount()-1;

    // Compute the column width in characters (after draggin)
    int col_width1 = m_FixedWidthListCtrl->GetColumnWidth(col_idx);

    // Get width of all columns combined and make sure that exceeds the
    // width of the longest line 
    if (m_FixedWidthListCtrl->GetColumnsCombinedCharWidth() +2 < 
        (int)m_ImportedTableData->GetMaxRowLength()) {
            // User has resized the last column to a size that
            // does not show all the text. Update the last column here
            // to make sure all characters fit.
            int update = (m_ImportedTableData->GetMaxRowLength() + 2) -
                m_FixedWidthListCtrl->GetColumnsCombinedCharWidth();
            update *= m_FixedWidthListCtrl->GetFontWidth();

            // Update the actual column width, update the column information for
            // the columns to reflect the length, and record the new column widths.
            m_FixedWidthListCtrl->SetColumnWidthIgnoreEvent(col_idx, col_width1 + update);
            m_FixedWidthListCtrl->SynchDataSourceColumnWidths();     
            m_FixedWidthListCtrl->UpdateColumnWidths();
            m_ListWidgetColPrevWidths = m_FixedWidthListCtrl->GetColumnWidths();
    }
}

END_NCBI_SCOPE
