/*  $Id: seqsub_organelle_panel.cpp 43202 2019-05-28 18:05:59Z filippov $
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
#include <gui/widgets/edit/seqsub_organelle_panel.hpp>
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
 * CSeqSubOrganellePanel type definition
 */

IMPLEMENT_DYNAMIC_CLASS( CSeqSubOrganellePanel, wxPanel )


/*
 * CSeqSubOrganellePanel event table definition
 */

BEGIN_EVENT_TABLE( CSeqSubOrganellePanel, wxPanel )

////@begin CSeqSubOrganellePanel event table entries
    EVT_RADIOBUTTON( ID_MOLORGANELLEYES, CSeqSubOrganellePanel::OnOrganelleYesSelected )
    EVT_RADIOBUTTON( ID_MOLORGANELLENO, CSeqSubOrganellePanel::OnOrganelleNoSelected )
    EVT_HYPERLINK( ID_MOLADDORGANELLE, CSeqSubOrganellePanel::OnAddOrganelleClicked )
    EVT_HYPERLINK( ID_MOLDELORGANELLE, CSeqSubOrganellePanel::OnDeleteOrganelleClicked )
    EVT_HYPERLINK( ID_MOLDELONEORGANELLE, CSeqSubOrganellePanel::OnDeleteOneOrganelleClicked )
////@end CSeqSubOrganellePanel event table entries

END_EVENT_TABLE()


/*
 * CSeqSubOrganellePanel constructors
 */

CSeqSubOrganellePanel::CSeqSubOrganellePanel()
{
    Init();
}

CSeqSubOrganellePanel::CSeqSubOrganellePanel( wxWindow* parent, ICommandProccessor* cmdproc, objects::CSeq_entry_Handle seh, wxWindowID id, const wxPoint& pos, const wxSize& size, long style ) :
    m_CmdProcessor(cmdproc), m_Seh(seh)
{
    Init();
    Create(parent, id, pos, size, style);
}


/*
 * CSeqSubOrganellePanel creator
 */

bool CSeqSubOrganellePanel::Create( wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style )
{
////@begin CSeqSubOrganellePanel creation
    SetExtraStyle(wxWS_EX_VALIDATE_RECURSIVELY);
    wxPanel::Create( parent, id, pos, size, style );

    CreateControls();
    if (GetSizer())
    {
        GetSizer()->SetSizeHints(this);
    }
    Centre();
////@end CSeqSubOrganellePanel creation
    return true;
}


/*
 * CSeqSubOrganellePanel destructor
 */

CSeqSubOrganellePanel::~CSeqSubOrganellePanel()
{
////@begin CSeqSubOrganellePanel destruction
////@end CSeqSubOrganellePanel destruction
}


/*
 * Member initialisation
 */

void CSeqSubOrganellePanel::Init()
{
////@begin CSeqSubOrganellePanel member initialisation
    m_YesOrganelle = NULL;
    m_NoOrganelle = NULL;
    m_ScrolledWindow = NULL;
    m_Sizer = NULL;
    m_LabelSizer = NULL;
    m_AddOrganelle = NULL;
    m_DelOrganelle = NULL;
    x_GetOrganelleTypes();

////@end CSeqSubOrganellePanel member initialisation
}


/*
 * Control creation for CSeqSubOrganellePanel
 */

void CSeqSubOrganellePanel::CreateControls()
{    
////@begin CSeqSubOrganellePanel content construction
    CSeqSubOrganellePanel* itemPanel1 = this;

    wxBoxSizer* itemBoxSizer2 = new wxBoxSizer(wxVERTICAL);
    itemPanel1->SetSizer(itemBoxSizer2);

    itemBoxSizer2->Add(5, 5, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

    wxFlexGridSizer* itemFlexGridSizer1 = new wxFlexGridSizer(0, 1, 0, 0);
    itemBoxSizer2->Add(itemFlexGridSizer1, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 0);

    wxStaticText* itemStaticText2 = new wxStaticText( itemPanel1, wxID_STATIC, _("Does any sequence belong to an organelle?*"), wxDefaultPosition, wxDefaultSize, 0 );
    itemFlexGridSizer1->Add(itemStaticText2, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxBoxSizer* itemBoxSizer3 = new wxBoxSizer(wxHORIZONTAL);
    itemFlexGridSizer1->Add(itemBoxSizer3, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxRIGHT|wxTOP|wxBOTTOM, 5);

    m_YesOrganelle = new wxRadioButton( itemPanel1, ID_MOLORGANELLEYES, _("Yes"), wxDefaultPosition, wxDefaultSize, wxRB_GROUP );
    m_YesOrganelle->SetValue(false);
    itemBoxSizer3->Add(m_YesOrganelle, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    m_NoOrganelle = new wxRadioButton( itemPanel1, ID_MOLORGANELLENO, _("No"), wxDefaultPosition, wxDefaultSize, 0 );
    m_NoOrganelle->SetValue(true);
    itemBoxSizer3->Add(m_NoOrganelle, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

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
#ifdef __WXMSW__
    m_LabelSizer->Add(5, 5, 0, wxALIGN_CENTER_VERTICAL | wxALL, 5);
#else
    m_LabelSizer->Add(2, 5, 0, wxALIGN_CENTER_VERTICAL | wxALL, 5);
#endif
    
    wxStaticText* itemStaticText10 = new wxStaticText( itemPanel1, wxID_STATIC, _("Organelle name*"), wxDefaultPosition, wxDefaultSize, 0 );
    m_LabelSizer->Add(itemStaticText10, 0, wxALIGN_CENTER_VERTICAL|wxALL| wxRESERVE_SPACE_EVEN_IF_HIDDEN, 5);
#if defined(__WXMSW__) || defined(__WXOSX_COCOA__)
    m_LabelSizer->Add(75, 5, 0, wxALIGN_CENTER_VERTICAL | wxALL, 5);
#else
    m_LabelSizer->Add(90, 5, 0, wxALIGN_CENTER_VERTICAL | wxALL, 5);
#endif
    
    wxStaticText* itemStaticText12 = new wxStaticText( itemPanel1, wxID_STATIC, _("Complete"), wxDefaultPosition, wxDefaultSize, 0 );
    m_LabelSizer->Add(itemStaticText12, 0, wxALIGN_CENTER_VERTICAL|wxALL| wxRESERVE_SPACE_EVEN_IF_HIDDEN, 5);

    wxStaticText* itemStaticText13 = new wxStaticText( itemPanel1, wxID_STATIC, _("Circular"), wxDefaultPosition, wxDefaultSize, 0 );
    m_LabelSizer->Add(itemStaticText13, 0, wxALIGN_CENTER_VERTICAL|wxALL| wxRESERVE_SPACE_EVEN_IF_HIDDEN, 5);

    m_ScrolledWindow = new wxScrolledWindow( itemPanel1, ID_MOLORGANELLESCROLLEDWND, wxDefaultPosition, wxSize(566, 92), wxTAB_TRAVERSAL );
    itemFlexGridSizer1->Add(m_ScrolledWindow, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxALL|wxRESERVE_SPACE_EVEN_IF_HIDDEN, 5);
    m_ScrolledWindow->SetScrollbars(1, 1, 0, 0);

    m_Sizer = new wxFlexGridSizer(0, 2, 0, 0);
    m_ScrolledWindow->SetSizer(m_Sizer);

    for (size_t index = 0; index < m_Rows; ++index) {
        x_AddEmptyRow();
    }

    wxBoxSizer* itemBoxSizer16 = new wxBoxSizer(wxHORIZONTAL);
    itemFlexGridSizer1->Add(itemBoxSizer16, 0, wxGROW|wxALIGN_CENTER_VERTICAL|wxRIGHT|wxTOP|wxBOTTOM, 5);

    m_AddOrganelle = new wxHyperlinkCtrl( itemPanel1, ID_MOLADDORGANELLE, _("Add another organelle"), wxEmptyString, wxDefaultPosition, wxDefaultSize, wxHL_DEFAULT_STYLE );
    itemBoxSizer16->Add(m_AddOrganelle, 0, wxALIGN_CENTER_VERTICAL|wxALL|wxRESERVE_SPACE_EVEN_IF_HIDDEN, 5);

    itemBoxSizer16->Add(5, 5, 1, wxGROW|wxALL, 5);

    m_DelOrganelle = new wxHyperlinkCtrl( itemPanel1, ID_MOLDELORGANELLE, _("Delete all organelles"), wxEmptyString, wxDefaultPosition, wxDefaultSize, wxHL_DEFAULT_STYLE );
    itemBoxSizer16->Add(m_DelOrganelle, 0, wxALIGN_CENTER_VERTICAL|wxALL|wxRESERVE_SPACE_EVEN_IF_HIDDEN, 5);

    x_EnableControls(false);

////@end CSeqSubOrganellePanel content construction
}

void CSeqSubOrganellePanel::x_GetOrganelleTypes()
{
    m_organelle_types.clear();
    CEnumeratedTypeValues::TValues type_values = objects::CBioSource::ENUM_METHOD_NAME(EGenome)()->GetValues();
    for (CEnumeratedTypeValues::TValues::const_iterator i = type_values.begin(); i != type_values.end(); ++i)
    {
        objects::CBioSource::EGenome val = static_cast<objects::CBioSource::EGenome>(i->second);
        if (val == objects::CBioSource::eGenome_unknown || 
            val == objects::CBioSource::eGenome_genomic || 
            val == objects::CBioSource::eGenome_plasmid || 
            val == objects::CBioSource::eGenome_chromosome || 
            val == objects::CBioSource::eGenome_endogenous_virus || 
            val == objects::CBioSource::eGenome_extrachrom || 
            val == objects::CBioSource::eGenome_macronuclear || 
            val == objects::CBioSource::eGenome_proviral)
            continue;
        m_organelle_types.push_back(val);
    }        
}

void CSeqSubOrganellePanel::x_EnableControls(bool value)
{
    for (size_t i = 0; i < m_LabelSizer->GetItemCount(); ++i) {
        wxSizerItem* item = m_LabelSizer->GetItem(i);
        if (item->IsWindow()) {
            item->GetWindow()->Show(value);
        }
    }
    m_ScrolledWindow->Show(value);
    m_AddOrganelle->Show(value);
    m_DelOrganelle->Show(value);

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

void CSeqSubOrganellePanel::x_AddEmptyRow()
{
    CSingleSequencePanel* row = new CSingleSequencePanel(m_ScrolledWindow, m_Seh, objects::CSubSource::eSubtype_other, true, m_organelle_types); 
    if (m_RowHeight == 0) {
        int row_width;
        row->GetClientSize(&row_width, &m_RowHeight);
    }
    x_AddRowToWindow(row);
}

void CSeqSubOrganellePanel::x_AddRowToWindow(wxWindow* row)
{
    m_Sizer->Add(row, 0, wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL | wxALL, 0);
    wxHyperlinkCtrl* itemHyperLink = new CNoTabHyperlinkCtrl(m_ScrolledWindow, ID_MOLDELONEORGANELLE, wxT("Delete"), wxT("delete"));
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

bool CSeqSubOrganellePanel::ShowToolTips()
{
    return true;
}

/*
 * Get bitmap resources
 */

wxBitmap CSeqSubOrganellePanel::GetBitmapResource( const wxString& name )
{
    // Bitmap retrieval
////@begin CSeqSubOrganellePanel bitmap retrieval
    wxUnusedVar(name);
    return wxNullBitmap;
////@end CSeqSubOrganellePanel bitmap retrieval
}

/*
 * Get icon resources
 */

wxIcon CSeqSubOrganellePanel::GetIconResource( const wxString& name )
{
    // Icon retrieval
////@begin CSeqSubOrganellePanel icon retrieval
    wxUnusedVar(name);
    return wxNullIcon;
////@end CSeqSubOrganellePanel icon retrieval
}


/*
 * wxEVT_COMMAND_RADIOBUTTON_SELECTED event handler for ID_MOLORGANELLEYES
 */

void CSeqSubOrganellePanel::OnOrganelleYesSelected( wxCommandEvent& event )
{
    x_EnableControls(true);
    if (m_Sizer->IsEmpty()) {
        x_AddEmptyRow();
    }
}


/*
 * wxEVT_COMMAND_RADIOBUTTON_SELECTED event handler for ID_MOLORGANELLENO
 */

void CSeqSubOrganellePanel::OnOrganelleNoSelected( wxCommandEvent& event )
{
    x_EnableControls(false);
}


/*
 * wxEVT_COMMAND_HYPERLINK event handler for ID_MOLADDORGANELLE
 */

void CSeqSubOrganellePanel::OnAddOrganelleClicked( wxHyperlinkEvent& event )
{
    x_AddEmptyRow();
    int x, y;
    m_ScrolledWindow->GetVirtualSize(&x, &y);

    int xUnit, yUnit;
    m_ScrolledWindow->GetScrollPixelsPerUnit(&xUnit, &yUnit);
    m_ScrolledWindow->Scroll(0, y / yUnit);
}


/*
 * wxEVT_COMMAND_HYPERLINK event handler for ID_MOLDELORGANELLE
 */

void CSeqSubOrganellePanel::OnDeleteOrganelleClicked( wxHyperlinkEvent& event )
{
    x_DeleteAllSubWindows();
}

void CSeqSubOrganellePanel::x_DeleteAllSubWindows()
{
    while (m_Sizer->GetItemCount() > 0) {
        size_t pos = 0;
        m_Sizer->GetItem(pos)->DeleteWindows();
        m_Sizer->Remove(pos);
    }
}


void CSeqSubOrganellePanel::OnDeleteOneOrganelleClicked(wxHyperlinkEvent& event)
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


bool CSeqSubOrganellePanel::TransferDataToWindow()
{
    if (!m_Seh) {
        return false;
    }

    // examine sequences looking for sources with organelle modifiers
    size_t num_rows_needed = 0;
    for (objects::CBioseq_CI bi(m_Seh, objects::CSeq_inst::eMol_na); bi; ++bi) {
        if (CSingleSequencePanel::NeedsOrganelleRow(*bi, m_organelle_types)) {
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
        m_YesOrganelle->SetValue(true);
        m_NoOrganelle->SetValue(false);
        x_EnableControls(true);
    } else {
        m_YesOrganelle->SetValue(false);
        m_NoOrganelle->SetValue(true);
        x_EnableControls(false);
    }

    auto rit = m_ScrolledWindow->GetChildren().begin();
    for (objects::CBioseq_CI bi(m_Seh, objects::CSeq_inst::eMol_na); bi; ++bi) {
        if (CSingleSequencePanel::NeedsOrganelleRow(*bi, m_organelle_types)) {
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


void CSeqSubOrganellePanel::ApplyCommand()
{
    if (!m_Seh) {
        return;
    }

    bool any_changes = false;
    CRef<CCmdComposite> cmd(new CCmdComposite("Update organelles"));
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


    if (any_changes) {
        m_CmdProcessor->Execute(cmd);
    }

}

void CSeqSubOrganellePanel::ReportMissingFields(string &text)
{
    if (m_NoOrganelle->GetValue())
        return;
    
    bool found = false;
    for (auto rit : m_ScrolledWindow->GetChildren()) 
    {
        CSingleSequencePanel* row = dynamic_cast<CSingleSequencePanel *>(rit);
        if (row) 
        {
            string seqid = row->GetSeqId();
            string name = row->GetOrganelleValue();
            if (!seqid.empty() && !name.empty()) 
            {
                found = true;
            }
        }
    }
    if (!found)
        text += "Organelle Name\n";
}

END_NCBI_SCOPE
