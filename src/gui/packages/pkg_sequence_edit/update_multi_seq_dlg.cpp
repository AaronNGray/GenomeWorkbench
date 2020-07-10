/*  $Id: update_multi_seq_dlg.cpp 43773 2019-08-29 17:37:48Z katargir $
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
 * Authors:  Andrea Asztalos
 */

#include <ncbi_pch.hpp>
#include <gui/widgets/wx/wx_utils.hpp>
#include <gui/widgets/edit/generic_report_dlg.hpp>
#include <gui/widgets/wx/message_box.hpp>
#include <gui/utils/app_job_impl.hpp>
#include <util/line_reader.hpp>
#include <objtools/edit/cds_fix.hpp>
#include <gui/widgets/data/report_dialog.hpp>
#include <gui/packages/pkg_sequence_edit/update_seq.hpp>
#include "update_seq_panel.hpp"
#include <gui/packages/pkg_sequence_edit/update_multi_seq_input.hpp>
#include <gui/packages/pkg_sequence_edit/update_multi_seq_dlg.hpp>
#include <gui/packages/pkg_sequence_edit/sequpd.hpp>
#include <gui/packages/pkg_sequence_edit/sequpd_except.hpp>

#include <wx/sizer.h>
#include <wx/button.h>
#include <wx/listbox.h>
#include <wx/notebook.h>
#include <wx/msgdlg.h>
#include <wx/filedlg.h>
#include <wx/wupdlock.h>

BEGIN_NCBI_SCOPE
USING_SCOPE(objects);

/*!
 * CUpdateMultiSeq_Dlg type definition
 */

IMPLEMENT_DYNAMIC_CLASS( CUpdateMultiSeq_Dlg, CDialog )


/*!
 * CUpdateMultiSeq_Dlg event table definition
 */

BEGIN_EVENT_TABLE( CUpdateMultiSeq_Dlg, CDialog )
////@begin CUpdateMultiSeq_Dlg event table entries
    EVT_CLOSE( CUpdateMultiSeq_Dlg::OnCloseWindow )
    EVT_LISTBOX( ID_UPDMULTSEQ_PNL1_LISTBOX1, CUpdateMultiSeq_Dlg::OnNonIdentSeqSelected )
    EVT_LISTBOX( ID_UPDMULTSEQ_PNL1_LISTBOX2, CUpdateMultiSeq_Dlg::OnIdentSeqSelected )
    EVT_BUTTON( ID_UPDMULTSEQ_PNL1_BTN, CUpdateMultiSeq_Dlg::OnRemoveIdenticalsClick )
    EVT_BUTTON( ID_UPDMULTSEQ_PNL2_BTN1, CUpdateMultiSeq_Dlg::OnLoadMapClick )
    EVT_BUTTON( ID_UPDMULTSEQ_PNL2_BTN2, CUpdateMultiSeq_Dlg::OnMapSelectionClick )

    EVT_BUTTON( ID_UPDMULTSEQ_OKTHIS, CUpdateMultiSeq_Dlg::OnUpdateThisSequenceClick )
    EVT_BUTTON( ID_UPDMULTSEQ_SKIP, CUpdateMultiSeq_Dlg::OnSkipThisSequenceClick )
    EVT_BUTTON( ID_UPDMULTSEQ_OKALL, CUpdateMultiSeq_Dlg::OnUpdateAllSequenceClick )
    EVT_BUTTON( ID_UPDMULTSEQ_STOP, CUpdateMultiSeq_Dlg::OnStopUpdatingClick )
    EVT_BUTTON( wxID_CANCEL, CUpdateMultiSeq_Dlg::OnCancelClick )

    EVT_UPDATE_UI(ID_UPDMULTSEQ_PNL1_BTN, CUpdateMultiSeq_Dlg::OnRemoveIdenticalUpdate)
    EVT_UPDATE_UI(ID_UPDMULTSEQ_PNL2_BTN2, CUpdateMultiSeq_Dlg::OnMapSelectionUpdate)
    
////@end CUpdateMultiSeq_Dlg event table entries

    EVT_IDLE(CUpdateMultiSeq_Dlg::OnIdle)
END_EVENT_TABLE()


/*!
 * CUpdateMultiSeq_Dlg constructors
 */

CUpdateMultiSeq_Dlg::CUpdateMultiSeq_Dlg()
    : m_UpdMultiSeqInput(0), m_Updated(0), m_Skipped(0)
{
    Init();
}

CUpdateMultiSeq_Dlg::CUpdateMultiSeq_Dlg(wxWindow* parent, CUpdateMultipleSeq_Input& updmultiseq_in, CRef<CCmdComposite> cmd, 
    wxWindowID id, const wxString& caption, const wxPoint& pos, const wxSize& size, long style)
    : m_UpdMultiSeqInput(&updmultiseq_in), m_UpdateCmd(cmd), m_Updated(0), m_Skipped(0)
{
    Init();
    Create(parent, id, caption, pos, size, style);
    NEditingStats::ReportUsage(caption);
}

void CUpdateMultiSeq_Dlg::s_ReportIdenticalPairs(const CUpdateMultipleSeq_Input& input)
{
    size_t identNum = input.GetIdenticalUpdates().size();
    if (identNum == 0) {
        return;
    }

    size_t nonIdentNum = input.GetNonIdenticalUpdates().size();
    CNcbiOstrstream oss;
    if (identNum == 1) {
        oss << "One sequence out of " << identNum + nonIdentNum << " is identical to its update sequence.";
    }
    else if (identNum > 1) {
        oss << identNum << " sequences out of ";
        oss << identNum + nonIdentNum << " are identical to their update sequences.";
    }

    string msg;
    for (auto&& it : input.GetIdenticalUpdates()) {
        msg.append(s_GetSeqIDLabel(it.first));
        msg.append("\n");
    }
    
    wxMessageBox(ToWxString(CNcbiOstrstreamToString(oss)), wxT("Info"), wxOK | wxICON_INFORMATION);
}

/*!
 * CUpdateMultiSeq_Dlg creator
 */

bool CUpdateMultiSeq_Dlg::Create( wxWindow* parent, wxWindowID id, const wxString& caption, 
    const wxPoint& pos, const wxSize& size, long style )
{
////@begin CUpdateMultiSeq_Dlg creation
    SetExtraStyle(wxWS_EX_BLOCK_EVENTS | wxWS_EX_VALIDATE_RECURSIVELY);
    CDialog::Create( parent, id, caption, pos, size, style );

    CreateControls();
    if (GetSizer())
    {
        GetSizer()->SetSizeHints(this);
    }
    Centre();

    s_ReportIdenticalPairs(*m_UpdMultiSeqInput);
////@end CUpdateMultiSeq_Dlg creation
    return true;
}


/*!
 * CUpdateMultiSeq_Dlg destructor
 */

CUpdateMultiSeq_Dlg::~CUpdateMultiSeq_Dlg()
{
}


/*!
 * Member initialisation
 */

void CUpdateMultiSeq_Dlg::Init()
{
////@begin CUpdateMultiSeq_Dlg member initialisation
    m_Notebook = NULL;
    m_NonIdenticalList = NULL;
    m_IdenticalList = NULL;
    m_IdenticalUpdBtn = NULL;
    m_NoUpdateList = NULL;
    m_UnmatchedList = NULL;
    m_LoadMap = NULL;
    m_MapSelection = NULL;
    m_UpdSeqPanel = NULL;
////@end CUpdateMultiSeq_Dlg member initialisation
}


/*!
 * Control creation for CUpdateMultiSeq_Dlg
 */

void CUpdateMultiSeq_Dlg::CreateControls()
{    
////@begin CUpdateMultiSeq_Dlg content construction
    CUpdateMultiSeq_Dlg* itemDialog1 = this;

    wxBoxSizer* itemBoxSizer2 = new wxBoxSizer(wxVERTICAL);
    itemDialog1->SetSizer(itemBoxSizer2);

    wxBoxSizer* itemBoxSizer3 = new wxBoxSizer(wxHORIZONTAL);
    itemBoxSizer2->Add(itemBoxSizer3, 1, wxEXPAND | wxALL, 5);

    m_Notebook = new wxNotebook( itemDialog1, ID_NOTEBOOK, wxDefaultPosition, wxDefaultSize, wxBK_TOP );

    wxPanel* itemPanel5 = new wxPanel( m_Notebook, ID_PANEL, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL );
    wxBoxSizer* itemBoxSizer6 = new wxBoxSizer(wxVERTICAL);
    itemPanel5->SetSizer(itemBoxSizer6);
    x_CreatePairwiseAlignments();

    wxStaticText* itemStaticText7 = new wxStaticText( itemPanel5, wxID_STATIC, _("Nonidentical updates"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer6->Add(itemStaticText7, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

    wxArrayString m_NonIdenticalListStrings;
    m_NonIdenticalList = new wxListBox( itemPanel5, ID_UPDMULTSEQ_PNL1_LISTBOX1, wxDefaultPosition, wxDefaultSize, m_NonIdenticalListStrings, wxLB_SINGLE | wxLB_SORT);
    itemBoxSizer6->Add(m_NonIdenticalList, 1, wxEXPAND | wxALL, 5);
    x_FillNonIdenticalList();
    if (!m_NonIdenticalList->IsEmpty()) {
        m_NonIdenticalList->SetSelection(0);
    }

    wxStaticText* itemStaticText9 = new wxStaticText( itemPanel5, wxID_STATIC, _("Identical updates"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer6->Add(itemStaticText9, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

    wxArrayString m_IdenticalListStrings;
    m_IdenticalList = new wxListBox(itemPanel5, ID_UPDMULTSEQ_PNL1_LISTBOX2, wxDefaultPosition, wxDefaultSize, m_IdenticalListStrings, wxLB_SINGLE | wxLB_SORT);
    itemBoxSizer6->Add(m_IdenticalList, 1, wxEXPAND | wxALL, 5);
    x_FillIdenticalList();
    if (m_NonIdenticalList->IsEmpty() && !m_IdenticalList->IsEmpty()) {
        m_IdenticalList->SetSelection(0);
    }

    m_IdenticalUpdBtn = new wxButton( itemPanel5, ID_UPDMULTSEQ_PNL1_BTN, _("Remove Identical Updates"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer6->Add(m_IdenticalUpdBtn, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

    m_Notebook->AddPage(itemPanel5, _("Update"));

    wxPanel* itemPanel12 = new wxPanel( m_Notebook, ID_PANEL1, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL );
    wxBoxSizer* itemBoxSizer13 = new wxBoxSizer(wxVERTICAL);
    itemPanel12->SetSizer(itemBoxSizer13);

    wxStaticText* itemStaticText14 = new wxStaticText( itemPanel12, wxID_STATIC, _("No updates"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer13->Add(itemStaticText14, 0, wxALIGN_CENTER_HORIZONTAL | wxALL, 5);

    wxArrayString m_NoUpdateListStrings;
    m_NoUpdateList = new wxListBox(itemPanel12, ID_UPDMULTSEQ_PNL2_LISTBOX1, wxDefaultPosition, wxDefaultSize, m_NoUpdateListStrings, wxLB_SINGLE | wxLB_SORT);
    itemBoxSizer13->Add(m_NoUpdateList, 1, wxEXPAND| wxALL, 5);
    x_FillNoUpdateList();

    wxStaticText* itemStaticText16 = new wxStaticText( itemPanel12, wxID_STATIC, _("Unmatched sequences"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer13->Add(itemStaticText16, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

    wxArrayString m_UnmatchedListStrings;
    m_UnmatchedList = new wxListBox(itemPanel12, ID_UPDMULTSEQ_PNL2_LISTBOX2, wxDefaultPosition, wxDefaultSize, m_UnmatchedListStrings, wxLB_SINGLE | wxLB_SORT);
    itemBoxSizer13->Add(m_UnmatchedList, 1, wxEXPAND | wxALL, 5);
    x_FillUnmatchedList();

    wxBoxSizer* itemBoxSizer18 = new wxBoxSizer(wxHORIZONTAL);
    itemBoxSizer13->Add(itemBoxSizer18, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 0);
    m_LoadMap = new wxButton( itemPanel12, ID_UPDMULTSEQ_PNL2_BTN1, _("Load Map"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer18->Add(m_LoadMap, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    m_MapSelection = new wxButton( itemPanel12, ID_UPDMULTSEQ_PNL2_BTN2, _("Map Selection"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer18->Add(m_MapSelection, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    m_Notebook->AddPage(itemPanel12, _("Map"));

    itemBoxSizer3->Add(m_Notebook, 0, wxGROW|wxALL, 2);

    // for the first update pair, form the panel
    string seq_name;
    if (m_NonIdenticalList && !m_NonIdenticalList->IsEmpty()) {
        seq_name = ToStdString(m_NonIdenticalList->GetString(0));
    }
    else if (m_IdenticalList && !m_IdenticalList->IsEmpty()) {
        seq_name = ToStdString(m_IdenticalList->GetString(0));
    }
    auto idh_it = m_SeqList.find(seq_name);
    if (idh_it != m_SeqList.end()) {
        CConstRef<CUpdateSeq_Input> upd_input = m_UpdMultiSeqInput->GetUpdateInput(idh_it->second);
        m_UpdSeqPanel = new CUpdateSeqPanel(itemDialog1, upd_input.GetPointer(), wxID_ANY, wxDefaultPosition, wxDefaultSize, 0);
    }
    else { // when the pairs need to be manually selected
        m_UpdSeqPanel = new CUpdateSeqPanel(itemDialog1, NULL, wxID_ANY, wxDefaultPosition, wxDefaultSize, 0);
    }
    itemBoxSizer3->Add(m_UpdSeqPanel, 1, wxGROW|wxALL, 5);

    wxBoxSizer* itemBoxSizer22 = new wxBoxSizer(wxHORIZONTAL);
    itemBoxSizer2->Add(itemBoxSizer22, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

    wxButton* itemButton23 = new wxButton( itemDialog1, ID_UPDMULTSEQ_OKTHIS, _("Update This Sequence"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer22->Add(itemButton23, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxButton* itemButton24 = new wxButton( itemDialog1, ID_UPDMULTSEQ_SKIP, _("Skip This Sequence"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer22->Add(itemButton24, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxButton* itemButton25 = new wxButton( itemDialog1, ID_UPDMULTSEQ_OKALL, _("Update All Sequences"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer22->Add(itemButton25, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxButton* itemButton26 = new wxButton( itemDialog1, ID_UPDMULTSEQ_STOP, _("Stop Updating"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer22->Add(itemButton26, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxButton* itemButton27 = new wxButton( itemDialog1, wxID_CANCEL, _("Cancel"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer22->Add(itemButton27, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

////@end CUpdateMultiSeq_Dlg content construction

    if (!m_Futures.empty()) {
        // disabel the update/skip buttons only if there are alignments to be generated in the background
        itemButton23->Disable();
        itemButton24->Disable();
        itemButton25->Disable();
    }
    x_SetUp();

}


void CUpdateMultiSeq_Dlg::x_FillNonIdenticalList()
{
    // fills the m_UpdateList listbox with sequence IDs
    _ASSERT(m_UpdMultiSeqInput);
    if (!m_NonIdenticalList) {
        return;
    }
    const CUpdateMultipleSeq_Input::TIDToUpdInputMap& updates = m_UpdMultiSeqInput->GetNonIdenticalUpdates();

    for (auto&& it : updates) {
        string label = s_GetSeqIDLabel(it.first);
        m_SeqList.emplace(label, it.first);
        m_NonIdenticalList->Append(ToWxString(label));
    }
}

void CUpdateMultiSeq_Dlg::x_FillIdenticalList()
{
    if (!m_IdenticalList) {
        return;
    }

    const CUpdateMultipleSeq_Input::TIDToUpdInputMap& identicals = m_UpdMultiSeqInput->GetIdenticalUpdates();
    for (auto&& it : identicals) {
        string label = s_GetSeqIDLabel(it.first);
        m_SeqList.emplace(label, it.first);
        m_IdenticalList->Append(ToWxString(label));
    }
}

// list of old sequences without updates
void CUpdateMultiSeq_Dlg::x_FillNoUpdateList()
{
    _ASSERT(m_UpdMultiSeqInput);
    const CUpdateMultipleSeq_Input::TSeqIDHVector& noUpdates = m_UpdMultiSeqInput->GetSeqsWithoutUpdates();
    for (auto&& it : noUpdates) {
        string label = s_GetSeqIDLabel(it);
        m_UnmatchedOldSeqs.emplace(label, it);
        m_NoUpdateList->Append(ToWxString(label));
    }
}

// list of update sequences that were not matched to any old sequences
void CUpdateMultiSeq_Dlg::x_FillUnmatchedList()
{
    const CUpdateMultipleSeq_Input::TSeqIDHVector& noUpdates = m_UpdMultiSeqInput->GetUnmatchedUpdateSeqs();
    for (auto&& it : noUpdates) {
        string label = s_GetSeqIDLabel(it);
        m_UnmatchedUpdSeqs.emplace(label, it);
        m_UnmatchedList->Append(ToWxString(label));
    }
}

void CUpdateMultiSeq_Dlg::x_SetUp()
{
    SetRegistryPath("Dialogs.CUpdateMultiSeq_Dlg");

    if (m_NonIdenticalList->IsEmpty() && (!m_NoUpdateList->IsEmpty() || !m_UnmatchedList->IsEmpty())) {
        m_Notebook->ChangeSelection(1);
    }

    SUpdateSeqParams params(SUpdateSeqParams::eSeqUpdateReplace);
    if (m_UpdSeqPanel) {
        m_UpdSeqPanel->SetData(params);
    }
}

string CUpdateMultiSeq_Dlg::s_GetSeqIDLabel(const CSeq_id_Handle& idh)
{
    const CSeq_id& id = *(idh.GetSeqId());
    string label;
    id.GetLabel(&label, CSeq_id::eContent);
    return label;
}

static tuple<objects::CSeq_id_Handle, CConstRef<objects::CSeq_align> > s_CreateAlign(
    CSeq_id_Handle idH,
    CBioseq_Handle subject,
    CBioseq_Handle query,
    ICanceled& canceled)
{
    tuple<objects::CSeq_id_Handle, CConstRef<objects::CSeq_align> > result(idH, CConstRef<CSeq_align>());

    if (canceled.IsCanceled())
        return result;

    try {
        bool accept_atleast_one = (subject.GetBioseqLength() < query.GetBioseqLength());

        auto align_vector = sequpd::RunBlast2Seq(subject, query, accept_atleast_one, &canceled);

        if (align_vector.size() > 1)
            LOG_POST(Info << "More than one alignment was found");

        if (align_vector.empty())
            LOG_POST(Info << "Could not form alignment between old and update sequence");
        else {
            // choose the 'better' alignment
            size_t best_align = 0;
            for (size_t i = 0; i < align_vector.size(); ++i) {
                for (size_t j = i + 1; j < align_vector.size(); ++j) {
                    best_align = sequpd::CompareAlignments(*align_vector[i], *align_vector[j]) ? i : j;
                }
            }
            _ASSERT(best_align < align_vector.size());
            std::get<1>(result) = align_vector[best_align];
        }
            
    } NCBI_CATCH("CUpdateMultiSeq_Dlg::s_CreateAlign()");

    return result;
}

void CUpdateMultiSeq_Dlg::OnIdle(wxIdleEvent& event)
{
    if (m_Futures.empty()) {
        event.Skip();
        return;
    }

    bool skip = true;
    auto it = m_Futures.begin();
    while (it != m_Futures.end()) {
        if (it->IsComplete()) {
            skip = false;

            auto& res = (*it)();
            const CSeq_id_Handle& idH = std::get<0>(res);
            m_Running.erase(idH);

            CRef<CUpdateSeq_Input> upd_input = m_UpdMultiSeqInput->GetUpdateInput(idH);
            upd_input->SetAlignment(std::get<1>(res).GetPointerOrNull());

            // update the panel for the one that's been selected
            const string seq_name = x_GetCurrentSelection();
            if (!seq_name.empty()) {
                auto idh_it = m_SeqList.find(seq_name);
                if (idh_it != m_SeqList.end() && idh_it->second == idH)
                    x_UpdateAlignmentPanel(seq_name);
            }

            it = m_Futures.erase(it);
        }
        else
            ++it;
    }

    if (m_Futures.empty()) {
        LOG_POST(Info << "Alignments for sequence update have been generated in: " << m_StopWatch.Elapsed() << "s");
        FindWindow(ID_UPDMULTSEQ_OKALL)->Enable();
        FindWindow(ID_UPDMULTSEQ_OKTHIS)->Enable();
        FindWindow(ID_UPDMULTSEQ_SKIP)->Enable();
    }

    if (skip)
        event.Skip();
}

void CUpdateMultiSeq_Dlg::x_CreatePairwiseAlignments()
{
    m_UpdMultiSeqInput->PrepareMultipleSequenceInputs();
    m_StopWatch.Restart();
    x_LaunchJobs(m_UpdMultiSeqInput->GetNonIdenticalUpdates());
    x_CreateAlignmentForIdenticalUpdates();
}

void CUpdateMultiSeq_Dlg::x_CreateAlignmentForIdenticalUpdates()
{
    const CUpdateMultipleSeq_Input::TIDToUpdInputMap& ident_updates = m_UpdMultiSeqInput->GetIdenticalUpdates();
    for (auto&& it : ident_updates) {
        CRef<CSeq_annot> annot = CUpdateSeq_Input::s_Align2IdenticalSeq(it.second->GetOldBioseq(), it.second->GetUpdateBioseq());
        if (annot && annot->IsAlign()) {
            CRef<CUpdateSeq_Input> upd_input = m_UpdMultiSeqInput->GetUpdateInput(it.first);
            upd_input->SetAlignment(annot->GetData().GetAlign().front());
        }
    }
}

void CUpdateMultiSeq_Dlg::x_LaunchJobs(const CUpdateMultipleSeq_Input::TIDToUpdInputMap& updates)
{
    for (const auto& it : updates) {
        m_Futures.push_back(job_async(
            [idH = it.first, subject = it.second->GetOldBioseq(), query = it.second->GetUpdateBioseq()](ICanceled& canceled)
        {
            return s_CreateAlign(idH, subject, query, canceled);
        }, "Update Sequence Align"));
        m_Running.insert(it.first);
    }
}

void CUpdateMultiSeq_Dlg::x_GenerateAlignments(const CUpdateMultipleSeq_Input::TOldToUpdateMap& update_pairs)
{
    for (auto&& it : update_pairs) {
        CRef<CUpdateSeq_Input> seq_input = m_UpdMultiSeqInput->GetUpdateInput(it.first);
        _ASSERT(seq_input);
        if (!seq_input) {
            NcbiMessageBox("Error in processing mapped sequence pairs.");
            return;
        }

        if (m_UpdMultiSeqInput->HaveIdenticalResidues(it.first, it.second)) {
            CRef<CSeq_annot> annot = CUpdateSeq_Input::s_Align2IdenticalSeq(seq_input->GetOldBioseq(), seq_input->GetUpdateBioseq());
            if (annot && annot->IsAlign()) {
                seq_input->SetAlignment(annot->GetData().GetAlign().front());
            }
        }
        else {
            m_Futures.push_back(job_async(
                [idH = it.first, subject = seq_input->GetOldBioseq(), query = seq_input->GetUpdateBioseq()](ICanceled& canceled)
            {
                return s_CreateAlign(idH, subject, query, canceled);
            }, "Update Sequence Align"));
            m_Running.insert(it.first);
        }
    }
}

/*!
 * Should we show tooltips?
 */

bool CUpdateMultiSeq_Dlg::ShowToolTips()
{
    return true;
}

/*!
 * Get bitmap resources
 */

wxBitmap CUpdateMultiSeq_Dlg::GetBitmapResource( const wxString& name )
{
    // Bitmap retrieval
////@begin CUpdateMultiSeq_Dlg bitmap retrieval
    wxUnusedVar(name);
    return wxNullBitmap;
////@end CUpdateMultiSeq_Dlg bitmap retrieval
}

/*!
 * Get icon resources
 */

wxIcon CUpdateMultiSeq_Dlg::GetIconResource( const wxString& name )
{
    // Icon retrieval
////@begin CUpdateMultiSeq_Dlg icon retrieval
    wxUnusedVar(name);
    return wxNullIcon;
////@end CUpdateMultiSeq_Dlg icon retrieval
}

void CUpdateMultiSeq_Dlg::x_LoadSettings(const CRegistryReadView& view)
{
    if (m_UpdSeqPanel)
        m_UpdSeqPanel->LoadSettings(view.GetReadView("Panel"));
}

void CUpdateMultiSeq_Dlg::x_SaveSettings(CRegistryWriteView view) const
{
    if (m_UpdSeqPanel)
        m_UpdSeqPanel->SaveSettings(view.GetWriteView("Panel"));
}

string CUpdateMultiSeq_Dlg::x_GetCurrentSelection()
{
    int selected = wxNOT_FOUND;
    string seq_name;
    if (m_NonIdenticalList && !m_NonIdenticalList->IsEmpty()) {
        selected = m_NonIdenticalList->GetSelection();
        if (selected != wxNOT_FOUND) {
            seq_name = ToStdString(m_NonIdenticalList->GetString(selected));
        }
    }

    if (seq_name.empty()) {
        if (m_IdenticalList && !m_IdenticalList->IsEmpty()) {
            selected = m_IdenticalList->GetSelection();
            if (selected != wxNOT_FOUND) {
                seq_name = ToStdString(m_IdenticalList->GetString(selected));
            }
        }
    }
    return seq_name;
}

void CUpdateMultiSeq_Dlg::x_UpdateAlignmentPanel(const string& seq_name)
{
    if (seq_name.empty())
        return;

    auto idh_it = m_SeqList.find(seq_name);
    if (idh_it == m_SeqList.end()) {
        wxMessageBox(wxT("This sequence could not be updated!"), wxT("Info"), wxOK | wxICON_INFORMATION);
        return;
    }

    CRef<CUpdateSeq_Input> upd_input = m_UpdMultiSeqInput->GetUpdateInput(idh_it->second);
    if (upd_input.IsNull()) {
        wxMessageBox(wxT("This sequence could not be updated!"), wxT("Info"), wxOK | wxICON_INFORMATION);
        return;
    }

    m_UpdSeqPanel->UpdatePanel(m_Running.find(idh_it->second) != m_Running.end(), upd_input);
    Layout();
}


void CUpdateMultiSeq_Dlg::OnRemoveIdenticalUpdate(wxUpdateUIEvent& event)
{
    if (m_IdenticalList) {
        event.Enable(!m_IdenticalList->IsEmpty());
        return;
    }
    event.Enable(false);
}

void CUpdateMultiSeq_Dlg::OnMapSelectionUpdate(wxUpdateUIEvent& event)
{
    if (m_NoUpdateList->IsEmpty() && m_UnmatchedList->IsEmpty()) {
        event.Enable(false);
        return;
    }
    event.Enable(true);
}

void CUpdateMultiSeq_Dlg::OnNonIdentSeqSelected(wxCommandEvent& event)
{
    int selected = m_NonIdenticalList->GetSelection();
    if (selected != wxNOT_FOUND) {
        const string seq_name = ToStdString(m_NonIdenticalList->GetString(selected));
        x_UpdateAlignmentPanel(seq_name);

        if (m_IdenticalList && m_IdenticalList->GetSelection() != wxNOT_FOUND) {
            m_IdenticalList->Deselect(m_IdenticalList->GetSelection());
        }
    }
}

void CUpdateMultiSeq_Dlg::OnIdentSeqSelected(wxCommandEvent& event)
{
    int selected = m_IdenticalList->GetSelection();
    if (selected != wxNOT_FOUND) {
        const string seq_name = ToStdString(m_IdenticalList->GetString(selected));
        x_UpdateAlignmentPanel(seq_name);

        if (m_NonIdenticalList && m_NonIdenticalList->GetSelection() != wxNOT_FOUND) {
            m_NonIdenticalList->Deselect(m_NonIdenticalList->GetSelection());
        }
    }
}

void CUpdateMultiSeq_Dlg::OnRemoveIdenticalsClick(wxCommandEvent& event)
{
    // remove sequences from the update list whose updates are identical with the sequences
    m_IdenticalList->Clear();
    
    const CUpdateMultipleSeq_Input::TIDToUpdInputMap& identicals = m_UpdMultiSeqInput->GetIdenticalUpdates();
    for (auto&& it : identicals) {
        string label = s_GetSeqIDLabel(it.first);
        m_SeqList.erase(label);
    }
}

void CUpdateMultiSeq_Dlg::OnLoadMapClick(wxCommandEvent& event)
{
    wxString ext = wxT("Text Documents (*.txt)|*.txt|All Files (*.*)|*.*");
    wxFileDialog open_dlg(this, wxT("Select a file"), wxEmptyString, wxEmptyString, ext, wxFD_OPEN | wxFD_FILE_MUST_EXIST);
    if (open_dlg.ShowModal() == wxID_CANCEL) {
        return;
    }

    wxString filename = open_dlg.GetPath();
    CNcbiIfstream fstream(filename.fn_str(), ios_base::binary);
    CRef<ILineReader> line_reader(ILineReader::New(fstream));
    if (line_reader.Empty()) {
        NcbiMessageBox("Could not open file.");
        return;
    }

    auto unmatchedOldSeqs = m_UnmatchedOldSeqs;
    for (const auto& s : m_UnmatchedOldSeqs) {
        auto pos = s.first.find('.');
        if (string::npos != pos)
            unmatchedOldSeqs.emplace(s.first.substr(0, pos), s.second);
    }

    vector<string> ignored_lines;
    string line;
    CUpdateMultipleSeq_Input::TOldToUpdateMap update_pairs;
    while (!line_reader->AtEOF()) {
        line_reader->ReadLine();
        line = line_reader->GetCurrentLine();

        vector<string> ids;
        NStr::Split(line, "\t", ids, NStr::fSplit_MergeDelimiters);
        for (auto&& it : ids) {
            NStr::TruncateSpacesInPlace(it);
        }

        if (ids.size() != 2) {
            ignored_lines.push_back(line);
            continue;
        }

        // first assume that the first column corresponds to old IDs
        auto oldidh_it = unmatchedOldSeqs.find(ids[0]);
        auto updidh_it = m_UnmatchedUpdSeqs.find(ids[1]);
        if (oldidh_it == unmatchedOldSeqs.end()) {
            // the second column corrresponds to old IDs
            oldidh_it = unmatchedOldSeqs.find(ids[1]);
            updidh_it = m_UnmatchedUpdSeqs.find(ids[0]);
        }

        if (updidh_it == m_UnmatchedUpdSeqs.end()) {
            // ignore that ID and report it
            ignored_lines.push_back(line);
            continue;
        }

        if (oldidh_it == unmatchedOldSeqs.end()) {
            NcbiMessageBox("Old sequence id " + ids[1] + " was not found.");
            ignored_lines.push_back(line);
            continue;
        }

        update_pairs.emplace(oldidh_it->second, updidh_it->second);
    }

    if (m_UpdMultiSeqInput->AppendOld_UpdateSeqPairs(update_pairs)) {
        // the pairs don't have their corresponding alignments up till here
        x_GenerateAlignments(update_pairs);
        
        wxWindowUpdateLocker locker(m_Notebook);
        for (auto&& it : update_pairs) {
            // add the old_id to either the nonidentical or to the identical list

            CSeq_id_Handle old_idh = it.first;
            string old_id_label = s_GetSeqIDLabel(old_idh);
            const CUpdateMultipleSeq_Input::TIDToUpdInputMap& nonident_updates = m_UpdMultiSeqInput->GetNonIdenticalUpdates();
            if (nonident_updates.find(old_idh) != nonident_updates.end()) {
                m_SeqList.emplace(old_id_label, old_idh);
                m_NonIdenticalList->Append(ToWxString(old_id_label));
            }
            else {
                const CUpdateMultipleSeq_Input::TIDToUpdInputMap& identicals = m_UpdMultiSeqInput->GetIdenticalUpdates();
                if (identicals.find(old_idh) != identicals.end()) {
                    m_SeqList.emplace(old_id_label, old_idh);
                    m_IdenticalList->Append(ToWxString(old_id_label));
                }
            }

            auto oldidh_it = m_UnmatchedOldSeqs.find(old_id_label);

            string upd_id_label = s_GetSeqIDLabel(it.second);
            auto updidh_it = m_UnmatchedUpdSeqs.find(upd_id_label);

            m_UnmatchedOldSeqs.erase(oldidh_it);
            m_UnmatchedUpdSeqs.erase(updidh_it);
            m_NoUpdateList->Delete(m_NoUpdateList->FindString(old_id_label));
            m_UnmatchedList->Delete(m_UnmatchedList->FindString(upd_id_label));
        }
    }

    x_UpdateNotebookAfterMapping();

    if (!ignored_lines.empty()) {
        string msg = "These lines were ignored:";
        for (auto&& it : ignored_lines) {
            msg.append("\n");
            msg.push_back('"');
            msg.append(it);
            msg.push_back('"');
        }
        NcbiMessageBox(msg, eDialog_Ok, eIcon_Exclamation, "Info");
    }
}

void CUpdateMultiSeq_Dlg::OnMapSelectionClick(wxCommandEvent& event)
{
    int noupdate_sel = m_NoUpdateList->GetSelection();   // from the list of old sequences
    int unmatched_sel = m_UnmatchedList->GetSelection();  // from the list of update sequences

    if (noupdate_sel == wxNOT_FOUND || unmatched_sel == wxNOT_FOUND) {
        NcbiMessageBox("Please select one ID from each of the above two lists.");
        return;
    }

    const string old_seq = ToStdString(m_NoUpdateList->GetString(noupdate_sel));
    const string upd_seq = ToStdString(m_UnmatchedList->GetString(unmatched_sel));
    
    auto oldidh_it = m_UnmatchedOldSeqs.find(old_seq);
    auto updidh_it = m_UnmatchedUpdSeqs.find(upd_seq);
    if (oldidh_it == m_UnmatchedOldSeqs.end() || updidh_it == m_UnmatchedUpdSeqs.end()) {
        NcbiMessageBox("This pair could not be selected.");
        return;
    }

    // update the m_UpdMultiSeqInput with this pair

    CUpdateMultipleSeq_Input::TOldToUpdateMap pair;
    pair.emplace(oldidh_it->second, updidh_it->second);
    if (m_UpdMultiSeqInput->AppendOld_UpdateSeqPairs(pair)) {
        x_GenerateAlignments(pair);
        
        wxWindowUpdateLocker locker(m_Notebook);
        // add the pair to either the nonidentical or to the identical list
        CSeq_id_Handle old_idh = oldidh_it->second;
        string label = s_GetSeqIDLabel(old_idh);

        const CUpdateMultipleSeq_Input::TIDToUpdInputMap& nonident_updates = m_UpdMultiSeqInput->GetNonIdenticalUpdates();
        if (nonident_updates.find(old_idh) != nonident_updates.end()) {
            m_SeqList.emplace(label, old_idh);
            m_NonIdenticalList->Append(ToWxString(label));
        }
        else {
            const CUpdateMultipleSeq_Input::TIDToUpdInputMap& identicals = m_UpdMultiSeqInput->GetIdenticalUpdates();
            if (identicals.find(old_idh) != identicals.end()) {
                m_SeqList.emplace(label, old_idh);
                m_IdenticalList->Append(ToWxString(label));
            }
        }

        m_UnmatchedOldSeqs.erase(oldidh_it);
        m_UnmatchedUpdSeqs.erase(updidh_it);
        m_NoUpdateList->Delete(noupdate_sel);
        m_UnmatchedList->Delete(unmatched_sel);
    }

    x_UpdateNotebookAfterMapping();
}

void CUpdateMultiSeq_Dlg::x_UpdateNotebookAfterMapping(void)
{
    if (m_NoUpdateList->IsEmpty() && m_UnmatchedList->IsEmpty()) {
        m_Notebook->ChangeSelection(0);
        string seq_name;
        if (!m_NonIdenticalList->IsEmpty()) {
            seq_name = ToStdString(m_NonIdenticalList->GetString(0));
            m_NonIdenticalList->SetSelection(0);
        }
        else if (!m_IdenticalList->IsEmpty()) {
            seq_name = ToStdString(m_IdenticalList->GetString(0));
            m_IdenticalList->SetSelection(0);
        }

        x_UpdateAlignmentPanel(seq_name);
    }
}

void CUpdateMultiSeq_Dlg::OnUpdateThisSequenceClick(wxCommandEvent& event)
{
    int selectNonIdent = (m_NonIdenticalList) ? m_NonIdenticalList->GetSelection() : wxNOT_FOUND;
    int selectIdent = (m_IdenticalList) ? m_IdenticalList->GetSelection() : wxNOT_FOUND;
    if (selectNonIdent == wxNOT_FOUND && selectIdent == wxNOT_FOUND) {
        wxMessageBox(wxT("No sequence was selected!"), wxT("Info"), wxOK | wxICON_INFORMATION);
        return;
    }
    bool create_general_only = objects::edit::IsGeneralIdProtPresent(m_UpdMultiSeqInput->GetOldEntry().GetTopLevelEntry());
    CRef<CCmdComposite> cmd;
    if (selectNonIdent != wxNOT_FOUND) {
        const string seq_name = ToStdString(m_NonIdenticalList->GetString(selectNonIdent));
        cmd.Reset(x_UpdateSelected(seq_name, create_general_only));
        x_SkipSelected(*m_NonIdenticalList, (unsigned int)selectNonIdent, m_IdenticalList);
    }
    else if (selectIdent != wxNOT_FOUND) {
        const string seq_name = ToStdString(m_IdenticalList->GetString(selectIdent));
        cmd.Reset(x_UpdateSelected(seq_name, create_general_only));
        x_SkipSelected(*m_IdenticalList, (unsigned int)selectIdent, m_NonIdenticalList);
    }

    if (cmd) {
        m_UpdateCmd->AddCommand(*cmd);
        m_Updated++;
    }

    if (((m_NonIdenticalList && m_NonIdenticalList->IsEmpty()) || !m_NonIdenticalList) &&
        ((m_IdenticalList && m_IdenticalList->IsEmpty()) || !m_IdenticalList)) {
        x_CloseDialog();
    }
}

CRef<CCmdComposite> CUpdateMultiSeq_Dlg::x_UpdateSelected(const string& seq_name, bool create_general_only)
{
    if (seq_name.empty())
        return CRef<CCmdComposite>();

    auto idh_it = m_SeqList.find(seq_name);
    if (idh_it == m_SeqList.end()) {
        wxMessageBox(wxT("This sequence could not be updated!"), wxT("Error"), wxOK | wxICON_INFORMATION);
        return CRef<CCmdComposite>();
    }

    CConstRef<CUpdateSeq_Input> upd_input = m_UpdMultiSeqInput->GetUpdateInput(idh_it->second);
    if (!upd_input) {
        return CRef<CCmdComposite>();
    }

    try {
        SUpdateSeqParams params = m_UpdSeqPanel->GetData();
        CSequenceUpdater updater(*upd_input, params);
        CRef<CCmdComposite> cmd = updater.Update(create_general_only);
        if (updater.IsReverseComplemented()) {
            m_RevCompReport.append("Reverse complemented ");
            m_RevCompReport.append(seq_name);
            m_RevCompReport.append("\n");
        }
        CNcbiOstrstream oss;
        updater.GetNotImportedFeatsReport(oss);
        if (!IsOssEmpty(oss)) {
            NcbiInfoBox(string(CNcbiOstrstreamToString(oss)));
        }
        return cmd;
    }
    catch (const CSeqUpdateException& e) {
        NcbiMessageBox(seq_name + " could not be updated.\n" + e.GetMsg());
    }

    return CRef<CCmdComposite>();
}

void CUpdateMultiSeq_Dlg::OnSkipThisSequenceClick(wxCommandEvent& event)
{
    int selectNonIdent = (m_NonIdenticalList) ? m_NonIdenticalList->GetSelection() : wxNOT_FOUND;
    int selectIdent = (m_IdenticalList) ? m_IdenticalList->GetSelection() : wxNOT_FOUND;
    if (selectNonIdent == wxNOT_FOUND && selectIdent == wxNOT_FOUND) {
        wxMessageBox(wxT("No sequence was selected!"), wxT("Info"), wxOK | wxICON_INFORMATION);
        return;
    }

    if (selectNonIdent != wxNOT_FOUND) {
        x_SkipSelected(*m_NonIdenticalList, selectNonIdent, m_IdenticalList);
        m_Skipped++;
    }
    else if (selectIdent != wxNOT_FOUND) {
        x_SkipSelected(*m_IdenticalList, selectIdent, m_NonIdenticalList);
        m_Skipped++;
    }

    if (((m_NonIdenticalList && m_NonIdenticalList->IsEmpty()) || !m_NonIdenticalList) &&
        ((m_IdenticalList && m_IdenticalList->IsEmpty()) || !m_IdenticalList)) {
        x_CloseDialog();
    }
}


void CUpdateMultiSeq_Dlg::x_SkipSelected(wxListBox& sel_list, unsigned int selected, wxListBox* other_list)
{
    string seq_name = ToStdString(sel_list.GetString(selected));
    auto idh_it = m_SeqList.find(seq_name);
    if (idh_it == m_SeqList.end()) {
        wxMessageBox(wxT("This sequence could not be skipped!"), wxT("Info"), wxOK | wxICON_INFORMATION);
        return;
    }

    m_SeqList.erase(idh_it);
    sel_list.Delete(selected);
    if ( ! sel_list.IsEmpty()) {
        if (selected > sel_list.GetCount() - 1) {
            selected = sel_list.GetCount() - 1;
        }
        sel_list.SetSelection(selected);
        seq_name = ToStdString(sel_list.GetString(selected));
        x_UpdateAlignmentPanel(seq_name);
    }
    else if (other_list && !other_list->IsEmpty()) {
        other_list->SetSelection(0);
        seq_name = ToStdString(other_list->GetString(selected));
        x_UpdateAlignmentPanel(seq_name);
    }
}


void CUpdateMultiSeq_Dlg::OnUpdateAllSequenceClick(wxCommandEvent& event)
{
    bool create_general_only = objects::edit::IsGeneralIdProtPresent(m_UpdMultiSeqInput->GetOldEntry().GetTopLevelEntry());
    // update nonidentical as well as identical update-seq pairs
    if (m_NonIdenticalList) {
        for (unsigned int selected = 0; selected < m_NonIdenticalList->GetCount(); ++selected) {
            const string seq_name = ToStdString(m_NonIdenticalList->GetString(selected));
            CRef<CCmdComposite> cmd = x_UpdateSelected(seq_name, create_general_only);
            if (cmd) {
                m_UpdateCmd->AddCommand(*cmd);
                m_Updated++;
            }
            auto idh_it = m_SeqList.find(seq_name);
            if (idh_it != m_SeqList.end()) {
                m_SeqList.erase(idh_it);
            }
        }
    }

    if (m_IdenticalList) {
        for (unsigned int selected = 0; selected < m_IdenticalList->GetCount(); ++selected) {
            const string seq_name = ToStdString(m_IdenticalList->GetString(selected));
            CRef<CCmdComposite> cmd = x_UpdateSelected(seq_name, create_general_only);
            if (cmd) {
                m_UpdateCmd->AddCommand(*cmd);
                m_Updated++;
            }
            auto idh_it = m_SeqList.find(seq_name);
            if (idh_it != m_SeqList.end()) {
                m_SeqList.erase(idh_it);
            }
        }
    }

    x_CloseDialog();
}

void CUpdateMultiSeq_Dlg::OnCancelClick(wxCommandEvent& event)
{
    EndModal(wxCANCEL);
}

void CUpdateMultiSeq_Dlg::OnStopUpdatingClick(wxCommandEvent& event)
{
    // has the role of CLOSE
    x_CloseDialog();
}

void CUpdateMultiSeq_Dlg::OnCloseWindow(wxCloseEvent& event)
{
    x_CloseDialog();
}

void CUpdateMultiSeq_Dlg::ReportStats() const
{
    CNcbiOstrstream oss;
    if (m_Updated > 0) {
        oss << m_Updated << " updated";
        if (m_Skipped > 0) {
            oss << ", " << m_Skipped << " skipped";
        }
    }
    else if (m_Skipped > 0) {
        oss << m_Skipped << " skipped";
    }
    string msg = CNcbiOstrstreamToString(oss);
    if (!msg.empty()) {
        wxMessageBox(ToWxString(msg), wxT("Info"), wxOK | wxICON_INFORMATION);
    }
}

void CUpdateMultiSeq_Dlg::x_CloseDialog()
{
    int retcode = (m_Updated > 0) ? wxOK : wxCLOSE;
    EndModal(retcode);
}

END_NCBI_SCOPE


