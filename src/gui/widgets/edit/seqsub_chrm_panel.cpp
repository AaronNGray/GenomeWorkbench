/*  $Id: seqsub_chrm_panel.cpp 43202 2019-05-28 18:05:59Z filippov $
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
#include <gui/widgets/edit/seqsub_chrm_panel.hpp>
#include <objmgr/bioseq_ci.hpp>
#include <objmgr/seqdesc_ci.hpp>
#include <objmgr/scope.hpp>
#include <objects/seq/Seqdesc.hpp>
#include <objects/seqfeat/BioSource.hpp>
#include <objects/seqfeat/SubSource.hpp>

#include <wx/sizer.h>
#include <wx/stattext.h>
#include <wx/radiobut.h>
#include <wx/scrolwin.h>
#include <wx/icon.h>

BEGIN_NCBI_SCOPE

/*
 * CSeqSubChromoPanel type definition
 */

IMPLEMENT_DYNAMIC_CLASS( CSeqSubChromoPanel, wxPanel )


/*
 * CSeqSubChromoPanel event table definition
 */

BEGIN_EVENT_TABLE( CSeqSubChromoPanel, wxPanel )

////@begin CSeqSubChromoPanel event table entries
    EVT_RADIOBUTTON( ID_MOLCHROMOSOMEYES, CSeqSubChromoPanel::OnChromosomeYesSelected )
    EVT_RADIOBUTTON( ID_MOLCHROMOSOMENO, CSeqSubChromoPanel::OnChromosomeNoSelected )
    EVT_HYPERLINK( ID_MOLADDCHROMOSOME, CSeqSubChromoPanel::OnAddchromosomeClicked )
    EVT_HYPERLINK( ID_MOLDELCHROMOSOME, CSeqSubChromoPanel::OnDeletechromosomeClicked )
    EVT_HYPERLINK(ID_MOLDELONECHROMOSOME, CSeqSubChromoPanel::OnDeleteOneChromosomeClicked )
////@end CSeqSubChromoPanel event table entries

END_EVENT_TABLE()


/*
 * CSeqSubChromoPanel constructors
 */

CSeqSubChromoPanel::CSeqSubChromoPanel()
{
    Init();
}

CSeqSubChromoPanel::CSeqSubChromoPanel( wxWindow* parent, ICommandProccessor* cmdproc, objects::CSeq_entry_Handle seh, wxWindowID id, const wxPoint& pos, const wxSize& size, long style ) :
    m_CmdProcessor(cmdproc), m_Seh(seh)
{
    Init();
    Create(parent, id, pos, size, style);
}


/*
 * CSeqSubChromoPanel creator
 */

bool CSeqSubChromoPanel::Create( wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style )
{
////@begin CSeqSubChromoPanel creation
    SetExtraStyle(wxWS_EX_VALIDATE_RECURSIVELY);
    wxPanel::Create( parent, id, pos, size, style );

    CreateControls();
    if (GetSizer())
    {
        GetSizer()->SetSizeHints(this);
    }
    Centre();
////@end CSeqSubChromoPanel creation
    return true;
}


/*
 * CSeqSubChromoPanel destructor
 */

CSeqSubChromoPanel::~CSeqSubChromoPanel()
{
////@begin CSeqSubChromoPanel destruction
////@end CSeqSubChromoPanel destruction
}


/*
 * Member initialisation
 */

void CSeqSubChromoPanel::Init()
{
////@begin CSeqSubChromoPanel member initialisation
    m_YesChrom = NULL;
    m_NoChrom = NULL;
    m_ScrolledWindow = NULL;
    m_Sizer = NULL;
    m_LabelSizer = NULL;
    m_AddChromosome = NULL;
    m_DelChromosome = NULL;
////@end CSeqSubChromoPanel member initialisation
}


/*
 * Control creation for CSeqSubChromoPanel
 */

void CSeqSubChromoPanel::CreateControls()
{    
////@begin CSeqSubChromoPanel content construction
    CSeqSubChromoPanel* itemPanel1 = this;

    wxBoxSizer* itemBoxSizer2 = new wxBoxSizer(wxVERTICAL);
    itemPanel1->SetSizer(itemBoxSizer2);

    itemBoxSizer2->Add(5, 5, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

    wxFlexGridSizer* itemFlexGridSizer1 = new wxFlexGridSizer(0, 1, 0, 0);
    itemBoxSizer2->Add(itemFlexGridSizer1, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 0);

    wxStaticText* itemStaticText20 = new wxStaticText( itemPanel1, wxID_STATIC, _("Does the organism have only one chromosome?*"), wxDefaultPosition, wxDefaultSize, 0 );
    itemFlexGridSizer1->Add(itemStaticText20, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxBoxSizer* itemBoxSizer21 = new wxBoxSizer(wxHORIZONTAL);
    itemFlexGridSizer1->Add(itemBoxSizer21, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxRIGHT|wxTOP|wxBOTTOM, 5);

    m_YesChrom = new wxRadioButton( itemPanel1, ID_MOLCHROMOSOMEYES, _("Yes"), wxDefaultPosition, wxDefaultSize, wxRB_GROUP );
    m_YesChrom->SetValue(true);
    itemBoxSizer21->Add(m_YesChrom, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    m_NoChrom = new wxRadioButton( itemPanel1, ID_MOLCHROMOSOMENO, _("No"), wxDefaultPosition, wxDefaultSize, 0 );
    m_NoChrom->SetValue(false);
    itemBoxSizer21->Add(m_NoChrom, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    m_LabelSizer = new wxBoxSizer(wxHORIZONTAL);
    itemFlexGridSizer1->Add(m_LabelSizer, 0, wxGROW|wxALIGN_CENTER_VERTICAL|wxRIGHT|wxTOP|wxBOTTOM, 5);

    wxStaticText* itemStaticText25 = new wxStaticText( itemPanel1, wxID_STATIC, _("Sequence ID*"), wxDefaultPosition, wxDefaultSize, 0 );
    m_LabelSizer->Add(itemStaticText25, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);
#ifdef __WXMSW__
    m_LabelSizer->Add(60, 5, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);
#else
    m_LabelSizer->Add(40, 5, 0, wxALIGN_CENTER_VERTICAL | wxALL, 5);
#endif
    wxStaticText* itemStaticText27 = new wxStaticText( itemPanel1, wxID_STATIC, _("Length"), wxDefaultPosition, wxDefaultSize, 0 );
    m_LabelSizer->Add(itemStaticText27, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

#ifdef __WXMSW__
    m_LabelSizer->Add(10, 5, 0, wxALIGN_CENTER_VERTICAL | wxALL, 0);
#else
    m_LabelSizer->Add(5, 5, 0, wxALIGN_CENTER_VERTICAL | wxALL, 0);
#endif
    m_ChromoLabel = new wxStaticText( itemPanel1, wxID_STATIC, _("Chromosome name*"), wxDefaultPosition, wxDefaultSize, 0 );
    m_LabelSizer->Add(m_ChromoLabel, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxStaticText* itemStaticText30 = new wxStaticText( itemPanel1, wxID_STATIC, _("Is the\nchromo-\nsome"), wxDefaultPosition, wxDefaultSize, 0 );
    m_LabelSizer->Add(itemStaticText30, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxStaticText* itemStaticText31 = new wxStaticText( itemPanel1, wxID_STATIC, _("Circular"), wxDefaultPosition, wxDefaultSize, 0 );
    m_LabelSizer->Add(itemStaticText31, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    m_ScrolledWindow = new wxScrolledWindow( itemPanel1, ID_MOLCHROMOSCROLLEDWND, wxDefaultPosition, wxSize(496, 92), wxTAB_TRAVERSAL);
    itemFlexGridSizer1->Add(m_ScrolledWindow, 0, wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL|wxALL, 5);
    m_ScrolledWindow->SetScrollbars(1, 1, 0, 0);

    m_Sizer = new wxFlexGridSizer(0, 2, 0, 0);
    m_ScrolledWindow->SetSizer(m_Sizer);
    x_AddFirstRow();

    wxBoxSizer* itemBoxSizer34 = new wxBoxSizer(wxHORIZONTAL);
    itemFlexGridSizer1->Add(itemBoxSizer34, 0, wxGROW|wxALIGN_CENTER_VERTICAL|wxRIGHT|wxTOP|wxBOTTOM|wxRESERVE_SPACE_EVEN_IF_HIDDEN, 5);

    m_AddChromosome = new wxHyperlinkCtrl( itemPanel1, ID_MOLADDCHROMOSOME, _("Add another chromosome"), wxEmptyString, wxDefaultPosition, wxDefaultSize, wxHL_DEFAULT_STYLE );
    itemBoxSizer34->Add(m_AddChromosome, 0, wxALIGN_CENTER_VERTICAL|wxALL|wxRESERVE_SPACE_EVEN_IF_HIDDEN, 5);

    itemBoxSizer34->Add(5, 5, 1, wxALIGN_CENTER_VERTICAL|wxALL|wxRESERVE_SPACE_EVEN_IF_HIDDEN, 5);

    m_DelChromosome = new wxHyperlinkCtrl( itemPanel1, ID_MOLDELCHROMOSOME, _("Delete all chromosomes"), wxEmptyString, wxDefaultPosition, wxDefaultSize, wxHL_DEFAULT_STYLE );
    itemBoxSizer34->Add(m_DelChromosome, 0, wxALIGN_CENTER_VERTICAL|wxALL|wxRESERVE_SPACE_EVEN_IF_HIDDEN, 5);

    x_EnableControls(false);

////@end CSeqSubChromoPanel content construction
}

void CSeqSubChromoPanel::x_EnableControls(bool value)
{
    m_ChromoLabel->Enable(value);
    m_AddChromosome->Show(value);
    m_DelChromosome->Show(value);

    wxSizerItemList& itemList = m_Sizer->GetChildren();
    wxSizerItemList::iterator node = itemList.begin();
    while (node != itemList.end()) {
        wxWindow* child = (**node).GetWindow();
        if (child) {
            if (CSingleSequencePanel* panel = dynamic_cast<CSingleSequencePanel*> (child)) {
                panel->EnableTextCtrl(value);
            }
            else if (wxHyperlinkCtrl* link = dynamic_cast<wxHyperlinkCtrl*>(child)) {
                link->Show(value);
            }
        }
        ++node;
    }
}

void CSeqSubChromoPanel::x_AddFirstRow()
{
    x_AddEmptyRow(true);
}

void CSeqSubChromoPanel::x_AddEmptyRow(bool hide_link)
{
    CSingleSequencePanel* row = new CSingleSequencePanel(m_ScrolledWindow, m_Seh, objects::CSubSource::eSubtype_chromosome);
    if (m_RowHeight == 0) {
        int row_width;
        row->GetClientSize(&row_width, &m_RowHeight);
    }
    x_AddRowToWindow(row, hide_link);
}

void CSeqSubChromoPanel::x_AddRowToWindow(wxWindow* row, bool hide_link)
{
    m_Sizer->Add(row, 0, wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL | wxALL, 0);
    wxHyperlinkCtrl* itemHyperLink = new CNoTabHyperlinkCtrl(m_ScrolledWindow, ID_MOLDELONECHROMOSOME, wxT("Delete"), wxT("delete"));
    itemHyperLink->SetVisitedColour(itemHyperLink->GetNormalColour());
    m_Sizer->Add(itemHyperLink, 0, wxALIGN_CENTER_VERTICAL | wxRESERVE_SPACE_EVEN_IF_HIDDEN | wxLEFT | wxRIGHT, 0);
    if (hide_link) {
        itemHyperLink->Hide();
    }
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

bool CSeqSubChromoPanel::ShowToolTips()
{
    return true;
}

/*
 * Get bitmap resources
 */

wxBitmap CSeqSubChromoPanel::GetBitmapResource( const wxString& name )
{
    // Bitmap retrieval
////@begin CSeqSubChromoPanel bitmap retrieval
    wxUnusedVar(name);
    return wxNullBitmap;
////@end CSeqSubChromoPanel bitmap retrieval
}

/*
 * Get icon resources
 */

wxIcon CSeqSubChromoPanel::GetIconResource( const wxString& name )
{
    // Icon retrieval
////@begin CSeqSubChromoPanel icon retrieval
    wxUnusedVar(name);
    return wxNullIcon;
////@end CSeqSubChromoPanel icon retrieval
}


/*
 * wxEVT_COMMAND_RADIOBUTTON_SELECTED event handler for ID_MOLCHROMOSOMEYES
 */

void CSeqSubChromoPanel::OnChromosomeYesSelected( wxCommandEvent& event )
{
    x_EnableControls(false);
    if (m_Sizer->IsEmpty()) {
        x_AddFirstRow();
    }
}


/*
 * wxEVT_COMMAND_RADIOBUTTON_SELECTED event handler for ID_MOLCHROMOSOMENO
 */

void CSeqSubChromoPanel::OnChromosomeNoSelected( wxCommandEvent& event )
{
    x_EnableControls(true);
    if (m_Sizer->GetItemCount() == 1) {
        m_Sizer->ShowItems(true);
    }
}


/*
 * wxEVT_COMMAND_HYPERLINK event handler for ID_MOLADDCHROMOSOME
 */

void CSeqSubChromoPanel::OnAddchromosomeClicked( wxHyperlinkEvent& event )
{
    x_AddEmptyRow();
    int x, y;
    m_ScrolledWindow->GetVirtualSize(&x, &y);

    int xUnit, yUnit;
    m_ScrolledWindow->GetScrollPixelsPerUnit(&xUnit, &yUnit);
    m_ScrolledWindow->Scroll(0, y / yUnit);
}


/*
 * wxEVT_COMMAND_HYPERLINK event handler for ID_MOLDELCHROMOSOME
 */

void CSeqSubChromoPanel::OnDeletechromosomeClicked( wxHyperlinkEvent& event )
{
    x_DeleteAllSubWindows();
}

void CSeqSubChromoPanel::x_DeleteAllSubWindows()
{
    while (m_Sizer->GetItemCount() > 0) {
        size_t pos = 0;
        m_Sizer->GetItem(pos)->DeleteWindows();
        m_Sizer->Remove(pos);
    }
}


void CSeqSubChromoPanel::OnDeleteOneChromosomeClicked(wxHyperlinkEvent& event)
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

void CSeqSubChromoPanel::SetSeqEntryHandle(objects::CSeq_entry_Handle seh)
{ 
    m_Seh = seh; 
    x_DeleteAllSubWindows();
    if (m_YesChrom->GetValue()) {
        x_AddEmptyRow();
    }
    TransferDataToWindow();
}

bool CSeqSubChromoPanel::TransferDataToWindow()
{
    if (!m_Seh) {
        return false;
    }

    // examine sequences looking for sources with chromosome modifiers
    size_t num_rows_needed = 0;
    for (objects::CBioseq_CI bi(m_Seh, objects::CSeq_inst::eMol_na); bi; ++bi) {
        if (CSingleSequencePanel::NeedsRow(*bi, objects::CSubSource::eSubtype_chromosome)) {
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

    if (num_rows_needed > 1) {
        m_YesChrom->SetValue(false);
        m_NoChrom->SetValue(true);
        x_EnableControls(true);
    } else {
        m_YesChrom->SetValue(true);
        m_NoChrom->SetValue(false);
        x_EnableControls(false);
    }

    auto rit = m_ScrolledWindow->GetChildren().begin();
    for (objects::CBioseq_CI bi(m_Seh, objects::CSeq_inst::eMol_na); bi; ++bi) {
        if (CSingleSequencePanel::NeedsRow(*bi, objects::CSubSource::eSubtype_chromosome)) {
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


void CSeqSubChromoPanel::ApplyCommand()
{
    if (!m_Seh) {
        return;
    }

    bool any_changes = false;
    CRef<CCmdComposite> cmd(new CCmdComposite("Update chromosomes"));
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
        if (CSingleSequencePanel::NeedsRow(*bi, objects::CSubSource::eSubtype_chromosome) && !updated[*bi]) {
            // take chromosome name off
            any_changes |= CSingleSequencePanel::RemoveNamedValue(*cmd, objects::CSubSource::eSubtype_chromosome, *bi);
        }
    }


    if (any_changes) {
        m_CmdProcessor->Execute(cmd);
    }
}

void CSeqSubChromoPanel::ReportMissingFields(string &text)
{
    if (m_YesChrom->GetValue())
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
        text += "Chromosome Name\n";
}

END_NCBI_SCOPE
