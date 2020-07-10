/*  $Id: seqsub_plasmid_panel.cpp 43202 2019-05-28 18:05:59Z filippov $
* ===========================================================================
*
*                            PUBLIC DOMAIN NOTICE
*               National Center for Biotechnology Information
*
*  This software / database is a "United States Government Work" under the
*  terms of the United States Copyright Act.It was written as part of
*  the author's official duties as a United States Government employee and
*  thus cannot be copyrighted.This software / database is freely available
*  to the public for use.The National Library of Medicine and the U.S.
*  Government have not placed any restriction on its use or reproduction.
*
*  Although all reasonable efforts have been taken to ensure the accuracy
*  and reliability of the software and data, the NLM and the U.S.
*  Government do not and cannot warrant the performance or results that
*  may be obtained by using this software or data.The NLM and the U.S.
*  Government disclaim all warranties, express or implied, including
*  warranties of performance, merchantability or fitness for any particular
*  purpose.
*
*  Please cite the author in any work or product based on this material.
*
* ===========================================================================
*
* Authors: Andrea Asztalos
*
*/

#include <ncbi_pch.hpp>
#include <gui/widgets/wx/unfocused_controls.hpp>
#include <gui/widgets/edit/single_sequence_panel.hpp>
#include <gui/widgets/edit/seqsub_plasmid_panel.hpp>
#include <objmgr/bioseq_ci.hpp>
#include <objmgr/seqdesc_ci.hpp>
#include <objmgr/scope.hpp>
#include <objects/seq/Seqdesc.hpp>
#include <objects/seq/MolInfo.hpp>
#include <objects/seqfeat/BioSource.hpp>
#include <objects/seqfeat/SubSource.hpp>
#include <gui/objutils/cmd_composite.hpp>
#include <gui/core/project_service.hpp>
#include <gui/core/project_task.hpp>
#include <gui/core/undo_manager.hpp>
#include <gui/objects/GBWorkspace.hpp>
#include <gui/objects/WorkspaceFolder.hpp>

#include <wx/sizer.h>
#include <wx/stattext.h>
#include <wx/radiobut.h>
#include <wx/scrolwin.h>
#include <wx/icon.h>

BEGIN_NCBI_SCOPE


/*
 * CSeqSubPlasmidPanel type definition
 */

IMPLEMENT_DYNAMIC_CLASS( CSeqSubPlasmidPanel, wxPanel )


/*
 * CSeqSubPlasmidPanel event table definition
 */

BEGIN_EVENT_TABLE( CSeqSubPlasmidPanel, wxPanel )

////@begin CSeqSubPlasmidPanel event table entries
    EVT_RADIOBUTTON( ID_MOLPLASMIDYES, CSeqSubPlasmidPanel::OnPlasmidYesSelected )
    EVT_RADIOBUTTON( ID_MOLPLASMIDNO, CSeqSubPlasmidPanel::OnPlasmidNoSelected )
    EVT_HYPERLINK( ID_MOLADDPLASMID, CSeqSubPlasmidPanel::OnAddplasmidClicked )
    EVT_HYPERLINK( ID_MOLDELPLASMID, CSeqSubPlasmidPanel::OnDeleteplasmidClicked )
    EVT_HYPERLINK( ID_MOLDELONEPLASMID, CSeqSubPlasmidPanel::OnDeleteOnePlasmidClicked )
////@end CSeqSubPlasmidPanel event table entries

END_EVENT_TABLE()


/*
 * CSeqSubPlasmidPanel constructors
 */

CSeqSubPlasmidPanel::CSeqSubPlasmidPanel()
{
    Init();
}

CSeqSubPlasmidPanel::CSeqSubPlasmidPanel( wxWindow* parent, ICommandProccessor* cmdproc, objects::CSeq_entry_Handle seh, wxWindowID id, const wxPoint& pos, const wxSize& size, long style ) :
    m_CmdProcessor(cmdproc), m_Seh(seh)
{
    Init();
    Create(parent, id, pos, size, style);
}


/*
 * CSeqSubPlasmidPanel creator
 */

bool CSeqSubPlasmidPanel::Create( wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style )
{
////@begin CSeqSubPlasmidPanel creation
    SetExtraStyle(wxWS_EX_VALIDATE_RECURSIVELY);
    wxPanel::Create( parent, id, pos, size, style );

    CreateControls();
    if (GetSizer())
    {
        GetSizer()->SetSizeHints(this);
    }
    Centre();
////@end CSeqSubPlasmidPanel creation
    return true;
}


/*
 * CSeqSubPlasmidPanel destructor
 */

CSeqSubPlasmidPanel::~CSeqSubPlasmidPanel()
{
////@begin CSeqSubPlasmidPanel destruction
////@end CSeqSubPlasmidPanel destruction
}


/*
 * Member initialisation
 */

void CSeqSubPlasmidPanel::Init()
{
////@begin CSeqSubPlasmidPanel member initialisation
    m_YesPlasmid = NULL;
    m_NoPlasmid = NULL;
    m_ScrolledWindow = NULL;
    m_Sizer = NULL;
    m_LabelSizer = NULL;
    m_AddPlasmid = NULL;
    m_DelPlasmid = NULL;
////@end CSeqSubPlasmidPanel member initialisation
}


/*
 * Control creation for CSeqSubPlasmidPanel
 */

void CSeqSubPlasmidPanel::CreateControls()
{    
////@begin CSeqSubPlasmidPanel content construction
    CSeqSubPlasmidPanel* itemPanel1 = this;

    wxBoxSizer* itemBoxSizer2 = new wxBoxSizer(wxVERTICAL);
    itemPanel1->SetSizer(itemBoxSizer2);

    itemBoxSizer2->Add(5, 5, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

    wxFlexGridSizer* itemFlexGridSizer1 = new wxFlexGridSizer(0, 1, 0, 0);
    itemBoxSizer2->Add(itemFlexGridSizer1, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 0);

    wxStaticText* itemStaticText2 = new wxStaticText( itemPanel1, wxID_STATIC, _("Does any sequence belong to a plasmid?*"), wxDefaultPosition, wxDefaultSize, 0 );
    itemFlexGridSizer1->Add(itemStaticText2, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxBoxSizer* itemBoxSizer3 = new wxBoxSizer(wxHORIZONTAL);
    itemFlexGridSizer1->Add(itemBoxSizer3, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxRIGHT|wxTOP|wxBOTTOM, 5);

    m_YesPlasmid = new wxRadioButton( itemPanel1, ID_MOLPLASMIDYES, _("Yes"), wxDefaultPosition, wxDefaultSize, wxRB_GROUP );
    m_YesPlasmid->SetValue(false);
    itemBoxSizer3->Add(m_YesPlasmid, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    m_NoPlasmid = new wxRadioButton( itemPanel1, ID_MOLPLASMIDNO, _("No"), wxDefaultPosition, wxDefaultSize, 0 );
    m_NoPlasmid->SetValue(true);
    itemBoxSizer3->Add(m_NoPlasmid, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    m_LabelSizer = new wxBoxSizer(wxHORIZONTAL);
    itemFlexGridSizer1->Add(m_LabelSizer, 0, wxGROW|wxALIGN_CENTER_VERTICAL|wxRIGHT|wxTOP|wxBOTTOM|wxRESERVE_SPACE_EVEN_IF_HIDDEN, 5);

    wxStaticText* itemStaticText7 = new wxStaticText( itemPanel1, wxID_STATIC, _("Sequence ID*"), wxDefaultPosition, wxDefaultSize, 0 );
    m_LabelSizer->Add(itemStaticText7, 0, wxALIGN_CENTER_VERTICAL|wxALL| wxRESERVE_SPACE_EVEN_IF_HIDDEN, 5);
#ifdef __WXMSW__
    m_LabelSizer->Add(60, 5, 0, wxALIGN_CENTER_VERTICAL | wxALL, 5);
#else
    m_LabelSizer->Add(40, 5, 0, wxALIGN_CENTER_VERTICAL | wxALL, 5);
#endif
    wxStaticText* itemStaticText9 = new wxStaticText( itemPanel1, wxID_STATIC, _("Length"), wxDefaultPosition, wxDefaultSize, 0 );
    m_LabelSizer->Add(itemStaticText9, 0, wxALIGN_CENTER_VERTICAL|wxALL| wxRESERVE_SPACE_EVEN_IF_HIDDEN, 5);

    m_LabelSizer->Add(5, 5, 0, wxALIGN_CENTER_VERTICAL | wxALL, 5);

    wxStaticText* itemStaticText10 = new wxStaticText( itemPanel1, wxID_STATIC, _("Plasmid name*"), wxDefaultPosition, wxDefaultSize, 0 );
    m_LabelSizer->Add(itemStaticText10, 0, wxALIGN_CENTER_VERTICAL|wxALL| wxRESERVE_SPACE_EVEN_IF_HIDDEN, 5);
#ifdef __WXMSW__
    m_LabelSizer->Add(15, 5, 0, wxALIGN_CENTER_VERTICAL | wxALL, 5);
#else
    m_LabelSizer->Add(10, 5, 0, wxALIGN_CENTER_VERTICAL | wxALL, 5);
#endif
    wxStaticText* itemStaticText12 = new wxStaticText( itemPanel1, wxID_STATIC, _("Complete"), wxDefaultPosition, wxDefaultSize, 0 );
    m_LabelSizer->Add(itemStaticText12, 0, wxALIGN_CENTER_VERTICAL|wxALL| wxRESERVE_SPACE_EVEN_IF_HIDDEN, 5);

    wxStaticText* itemStaticText13 = new wxStaticText( itemPanel1, wxID_STATIC, _("Circular"), wxDefaultPosition, wxDefaultSize, 0 );
    m_LabelSizer->Add(itemStaticText13, 0, wxALIGN_CENTER_VERTICAL|wxALL| wxRESERVE_SPACE_EVEN_IF_HIDDEN, 5);

    m_ScrolledWindow = new wxScrolledWindow( itemPanel1, ID_MOLPLASMIDSCROLLEDWND, wxDefaultPosition, wxSize(496, 92), wxTAB_TRAVERSAL);
    itemFlexGridSizer1->Add(m_ScrolledWindow, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxALL|wxRESERVE_SPACE_EVEN_IF_HIDDEN, 5);
    m_ScrolledWindow->SetScrollbars(1, 1, 0, 0);

    m_Sizer = new wxFlexGridSizer(0, 2, 0, 0);
    m_ScrolledWindow->SetSizer(m_Sizer);

    for (size_t index = 0; index < m_Rows; ++index) {
        x_AddEmptyRow();
    }

    wxBoxSizer* itemBoxSizer16 = new wxBoxSizer(wxHORIZONTAL);
    itemFlexGridSizer1->Add(itemBoxSizer16, 0, wxGROW|wxALIGN_CENTER_VERTICAL|wxRIGHT|wxTOP|wxBOTTOM, 5);

    m_AddPlasmid = new wxHyperlinkCtrl( itemPanel1, ID_MOLADDPLASMID, _("Add another plasmid"), wxEmptyString, wxDefaultPosition, wxDefaultSize, wxHL_DEFAULT_STYLE );
    itemBoxSizer16->Add(m_AddPlasmid, 0, wxALIGN_CENTER_VERTICAL|wxALL|wxRESERVE_SPACE_EVEN_IF_HIDDEN, 5);

    itemBoxSizer16->Add(5, 5, 1, wxGROW|wxALL, 5);

    m_DelPlasmid = new wxHyperlinkCtrl( itemPanel1, ID_MOLDELPLASMID, _("Delete all plasmids"), wxEmptyString, wxDefaultPosition, wxDefaultSize, wxHL_DEFAULT_STYLE );
    itemBoxSizer16->Add(m_DelPlasmid, 0, wxALIGN_CENTER_VERTICAL|wxALL|wxRESERVE_SPACE_EVEN_IF_HIDDEN, 5);

    x_EnableControls(false);

////@end CSeqSubPlasmidPanel content construction
}

void CSeqSubPlasmidPanel::x_EnableControls(bool value)
{
    for (size_t i = 0; i < m_LabelSizer->GetItemCount(); ++i) {
        wxSizerItem* item = m_LabelSizer->GetItem(i);
        if (item->IsWindow()) {
            item->GetWindow()->Show(value);
        }
    }
    m_ScrolledWindow->Show(value);
    m_AddPlasmid->Show(value);
    m_DelPlasmid->Show(value);

    wxSizerItemList& itemList = m_Sizer->GetChildren();
    wxSizerItemList::iterator node = itemList.begin();
    while (node != itemList.end()) {
        wxWindow* child = (**node).GetWindow();
        if (child) {
            if (wxHyperlinkCtrl* link = dynamic_cast<wxHyperlinkCtrl*>(child)) {
                link->Show(value);
            }
        }
        ++node;
    }
}

void CSeqSubPlasmidPanel::x_AddEmptyRow()
{
    CSingleSequencePanel* row = new CSingleSequencePanel(m_ScrolledWindow, m_Seh, objects::CSubSource::eSubtype_plasmid_name);
    if (m_RowHeight == 0) {
        int row_width;
        row->GetClientSize(&row_width, &m_RowHeight);
    }
    x_AddRowToWindow(row);
}

void CSeqSubPlasmidPanel::x_AddRowToWindow(wxWindow* row)
{
    m_Sizer->Add(row, 0, wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL | wxALL, 0);
    wxHyperlinkCtrl* itemHyperLink = new CNoTabHyperlinkCtrl(m_ScrolledWindow, ID_MOLDELONEPLASMID, wxT("Delete"), wxT("delete"));
    itemHyperLink->SetVisitedColour(itemHyperLink->GetNormalColour());
    m_Sizer->Add(itemHyperLink, 0, wxALIGN_CENTER_VERTICAL | wxLEFT | wxRIGHT, 0);
#ifdef __WXMSW__
    m_ScrolledWindow->SetScrollRate(0, m_RowHeight);
    m_ScrolledWindow->FitInside();
    m_ScrolledWindow->Refresh();
#else 
    int row_width, row_height;
    row->GetClientSize(&row_width, &row_height);

    int link_width, link_height;
    itemHyperLink->GetClientSize(&link_width, &link_height);
    if (link_height > row_height) {
        row_height = link_height;
    }
    row_width += link_width;

    if (row_width > m_TotalWidth) {
        m_TotalWidth = row_width;
    }

    m_TotalHeight += row_height;
    m_ScrollRate = row_height;

    m_ScrolledWindow->SetVirtualSize(m_TotalWidth, m_TotalHeight);
    m_ScrolledWindow->SetScrollRate(0, m_ScrollRate);
    m_ScrolledWindow->FitInside();
    m_ScrolledWindow->SetMinSize(wxSize(m_TotalWidth + 20, m_MaxRows * m_ScrollRate));
    m_ScrolledWindow->Refresh();
#endif
}



/*
 * Should we show tooltips?
 */

bool CSeqSubPlasmidPanel::ShowToolTips()
{
    return true;
}

/*
 * Get bitmap resources
 */

wxBitmap CSeqSubPlasmidPanel::GetBitmapResource( const wxString& name )
{
    // Bitmap retrieval
////@begin CSeqSubPlasmidPanel bitmap retrieval
    wxUnusedVar(name);
    return wxNullBitmap;
////@end CSeqSubPlasmidPanel bitmap retrieval
}

/*
 * Get icon resources
 */

wxIcon CSeqSubPlasmidPanel::GetIconResource( const wxString& name )
{
    // Icon retrieval
////@begin CSeqSubPlasmidPanel icon retrieval
    wxUnusedVar(name);
    return wxNullIcon;
////@end CSeqSubPlasmidPanel icon retrieval
}


/*
 * wxEVT_COMMAND_RADIOBUTTON_SELECTED event handler for ID_MOLPLASMIDYES
 */

void CSeqSubPlasmidPanel::OnPlasmidYesSelected( wxCommandEvent& event )
{
    x_EnableControls(true);
    if (m_Sizer->IsEmpty()) {
        x_AddEmptyRow();
    }
}


/*
 * wxEVT_COMMAND_RADIOBUTTON_SELECTED event handler for ID_MOLPLASMIDNO
 */

void CSeqSubPlasmidPanel::OnPlasmidNoSelected( wxCommandEvent& event )
{
    x_EnableControls(false);
}


/*
 * wxEVT_COMMAND_HYPERLINK event handler for ID_MOLADDPLASMID
 */

void CSeqSubPlasmidPanel::OnAddplasmidClicked( wxHyperlinkEvent& event )
{
    x_AddEmptyRow();
    int x, y;
    m_ScrolledWindow->GetVirtualSize(&x, &y);

    int xUnit, yUnit;
    m_ScrolledWindow->GetScrollPixelsPerUnit(&xUnit, &yUnit);
    m_ScrolledWindow->Scroll(0, y / yUnit);
}


/*
 * wxEVT_COMMAND_HYPERLINK event handler for ID_MOLDELPLASMID
 */

void CSeqSubPlasmidPanel::OnDeleteplasmidClicked( wxHyperlinkEvent& event )
{
    x_DeleteAllSubWindows();
}

void CSeqSubPlasmidPanel::x_DeleteAllSubWindows()
{
    while (m_Sizer->GetItemCount() > 0) {
        size_t pos = 0;
        m_Sizer->GetItem(pos)->DeleteWindows();
        m_Sizer->Remove(pos);
    }
}


void CSeqSubPlasmidPanel::OnDeleteOnePlasmidClicked(wxHyperlinkEvent& event)
{
    wxWindow *win = dynamic_cast<wxWindow*>(event.GetEventObject());
    if (!win)
        return;
    wxWindowList &children = m_ScrolledWindow->GetChildren();
    wxWindow *prev = NULL;
    for (wxWindowList::iterator child = children.begin(); child != children.end(); ++child)
    {
        if (*child == win)
        {
            wxSizer *sizer = win->GetContainingSizer();
            win->Destroy();
            prev->Destroy();
            m_Sizer->Remove(sizer);
            m_Sizer->Layout();
            m_ScrolledWindow->FitInside();
            break;
        }
        prev = *child;
    }
}


bool CSeqSubPlasmidPanel::TransferDataToWindow()
{
    if (!m_Seh) {
        return false;
    }

    // examine sequences looking for sources with plasmid modifiers
    size_t num_rows_needed = 0;
    for (objects::CBioseq_CI bi(m_Seh, objects::CSeq_inst::eMol_na); bi; ++bi) {
        if (CSingleSequencePanel::NeedsRow(*bi, objects::CSubSource::eSubtype_plasmid_name)) {
            num_rows_needed++;
        }
    }

    // count existing rows
    size_t num_rows_now = 0;
    for (auto item : m_ScrolledWindow->GetChildren()) {
        CSingleSequencePanel* row = dynamic_cast<CSingleSequencePanel *>(item);
        if (row) {
            num_rows_now++;
        }
    }
    while (num_rows_now < num_rows_needed) {
        x_AddEmptyRow();
        num_rows_now++;
    }

    if (num_rows_needed > 0) {
        m_YesPlasmid->SetValue(true);
        m_NoPlasmid->SetValue(false);
        x_EnableControls(true);
    } else {
        m_YesPlasmid->SetValue(false);
        m_NoPlasmid->SetValue(true);
        x_EnableControls(false);
    }

    auto rit = m_ScrolledWindow->GetChildren().begin();
    for (objects::CBioseq_CI bi(m_Seh, objects::CSeq_inst::eMol_na); bi; ++bi) {
        if (CSingleSequencePanel::NeedsRow(*bi, objects::CSubSource::eSubtype_plasmid_name)) {
            CSingleSequencePanel* row = dynamic_cast<CSingleSequencePanel *>(*rit);
            while (!row) {
                rit++;
                row = dynamic_cast<CSingleSequencePanel *>(*rit);
            }
            row->SetValue(*bi);
            rit++;
        }
    }
    while (rit != m_ScrolledWindow->GetChildren().end()) {
        CSingleSequencePanel* row = dynamic_cast<CSingleSequencePanel *>(*rit);
        if (row) {
            row->ClearValue();
        }
        rit++;
    }

    return true;
}


void CSeqSubPlasmidPanel::ApplyCommand()
{
    if (!m_Seh) {
        return;
    }

    bool any_changes = false;
    CRef<CCmdComposite> cmd(new CCmdComposite("Update plasmids"));
    typedef map<objects::CBioseq_Handle, bool> TUpdateMap;
    TUpdateMap updated;

    for (auto rit : m_ScrolledWindow->GetChildren()) {
        CSingleSequencePanel* row = dynamic_cast<CSingleSequencePanel *>(rit);
        if (row) {
            string seqid = row->GetSeqId();
            if (!seqid.empty()) {
                CRef<objects::CSeq_id> id(new objects::CSeq_id(seqid));
                objects::CBioseq_Handle bh = m_Seh.GetScope().GetBioseqHandle(*id);
                updated[bh] = true;

                any_changes |= row->AddToUpdateCommand(*cmd);
            }
        }
    }

    for (objects::CBioseq_CI bi(m_Seh, objects::CSeq_inst::eMol_na); bi; ++bi) {
        if (CSingleSequencePanel::NeedsRow(*bi, objects::CSubSource::eSubtype_plasmid_name) && !updated[*bi]) {
            // take plasmid name off
            any_changes |= CSingleSequencePanel::RemoveNamedValue(*cmd, objects::CSubSource::eSubtype_plasmid_name, *bi);
        }
    }


    if (any_changes) {
        m_CmdProcessor->Execute(cmd);
    }

}

void CSeqSubPlasmidPanel::ReportMissingFields(string &text)
{
    if (m_NoPlasmid->GetValue())
        return;
    
    bool found = false;
    for (auto rit : m_ScrolledWindow->GetChildren()) 
    {
        CSingleSequencePanel* row = dynamic_cast<CSingleSequencePanel *>(rit);
        if (row) 
        {
            string seqid = row->GetSeqId();
            string name = row->GetModifierValue();
            if (!seqid.empty() && !name.empty()) 
            {
                found = true;
            }
        }
    }
    if (!found)
        text += "Plasmid Name\n";
}

END_NCBI_SCOPE
