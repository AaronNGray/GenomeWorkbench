/*  $Id: update_seq_dlg.cpp 44068 2019-10-18 18:57:05Z asztalos $
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


// Generated by DialogBlocks (unregistered), 26/01/2015 09:02:45

#include <ncbi_pch.hpp>

#include <gui/widgets/wx/wx_utils.hpp>
#include <gui/widgets/wx/message_box.hpp>
#include <gui/widgets/data/report_dialog.hpp>
#include <gui/packages/pkg_sequence_edit/update_seq.hpp>
#include "update_seq_panel.hpp"
#include <gui/packages/pkg_sequence_edit/update_seq_dlg.hpp>
#include <gui/packages/pkg_sequence_edit/sequpd.hpp>
#include <gui/packages/pkg_sequence_edit/sequpd_except.hpp>

#include <wx/sizer.h>
#include <wx/button.h>
#include <wx/msgdlg.h>
#include <wx/utils.h>

BEGIN_NCBI_SCOPE
USING_SCOPE(objects);

/*!
 * CUpdateSeq_Dlg type definition
 */

IMPLEMENT_DYNAMIC_CLASS( CUpdateSeq_Dlg, CDialog )


/*!
 * CUpdateSeq_Dlg event table definition
 */

BEGIN_EVENT_TABLE( CUpdateSeq_Dlg, CDialog )
    EVT_IDLE(CUpdateSeq_Dlg::OnIdle)
    EVT_BUTTON(wxID_OK, CUpdateSeq_Dlg::OnUpdateSequence)
END_EVENT_TABLE()


/*!
 * CUpdateSeq_Dlg constructors
 */

 CUpdateSeq_Dlg::CUpdateSeq_Dlg()
    : m_UpdSeqInput(0)
{
    Init();
}

CUpdateSeq_Dlg::CUpdateSeq_Dlg( wxWindow* parent, 
        CUpdateSeq_Input& updseq_in,
        wxWindowID id, const wxString& caption, 
        const wxPoint& pos, const wxSize& size, long style )
    : m_UpdSeqInput(&updseq_in)
{
    if (!m_UpdSeqInput->IsReadyForUpdate()) {
        NCBI_THROW(CSeqUpdateException, eInternal, 
        "Sequence can not be updated: check sequences");
    }
    
    if (m_UpdSeqInput->HaveIdenticalResidues()) {
        string msg("Sequence is identical to update sequence");
        wxMessageBox(ToWxString(msg), wxT("Info"), wxOK | wxICON_INFORMATION);
    }

    Init();
    Create(parent, id, caption, pos, size, style);
    NEditingStats::ReportUsage(caption);
}


/*!
 * CUpdateSeq_Dlg creator
 */

bool CUpdateSeq_Dlg::Create( wxWindow* parent, wxWindowID id, const wxString& caption, const wxPoint& pos, const wxSize& size, long style )
{
////@begin CUpdateSeq_Dlg creation
    SetExtraStyle(wxWS_EX_BLOCK_EVENTS | wxWS_EX_VALIDATE_RECURSIVELY);
    CDialog::Create( parent, id, caption, pos, size, style );
    CreateControls();
    if (GetSizer())
    {
        GetSizer()->SetSizeHints(this);
    }
    Centre();
////@end CUpdateSeq_Dlg creation
    return true;
}


/*!
 * CUpdateSeq_Dlg destructor
 */

CUpdateSeq_Dlg::~CUpdateSeq_Dlg()
{
}


/*!
 * Member initialisation
 */

void CUpdateSeq_Dlg::Init()
{
////@begin CUpdateSeq_Dlg member initialisation
    m_UpdSeqPanel = NULL;
////@end CUpdateSeq_Dlg member initialisation
}

/*!
 * Control creation for CUpdateSeq_Dlg
 */

void CUpdateSeq_Dlg::CreateControls()
{    
////@begin CUpdateSeq_Dlg content construction
    // Generated by DialogBlocks, 04/03/2015 08:35:51 (unregistered)

    CUpdateSeq_Dlg* itemDialog1 = this;

    wxBoxSizer* itemBoxSizer2 = new wxBoxSizer(wxVERTICAL);
    itemDialog1->SetSizer(itemBoxSizer2);

    m_UpdSeqPanel = new CUpdateSeqPanel(itemDialog1, m_UpdSeqInput, wxID_ANY, wxDefaultPosition, wxDefaultSize, 0);
    itemBoxSizer2->Add(m_UpdSeqPanel, 1, wxGROW | wxALL, 5);
    x_CalculateAlignment();

    wxBoxSizer* itemBoxSizer3 = new wxBoxSizer(wxHORIZONTAL);
    itemBoxSizer2->Add(itemBoxSizer3, 0, wxGROW | wxALL, 2);

    wxHyperlinkCtrl* itemHyperlinkCtrl = new wxHyperlinkCtrl(itemDialog1, wxID_HELP, _("Help"), 
        wxT("https://www.ncbi.nlm.nih.gov/tools/gbench/manual9/#update-sequence"), wxDefaultPosition, wxDefaultSize, wxHL_DEFAULT_STYLE);
    itemHyperlinkCtrl->SetForegroundColour(wxColour(192, 192, 192));
    itemBoxSizer3->Add(itemHyperlinkCtrl, 0, wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL | wxALL, 5);

    itemBoxSizer3->AddStretchSpacer();

    wxButton* itemButton1 = new wxButton( itemDialog1, wxID_OK, _("Update Sequence"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer3->Add(itemButton1, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);
    itemButton1->Disable();

    wxButton* itemButton2 = new wxButton( itemDialog1, wxID_CANCEL, _("Cancel"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer3->Add(itemButton2, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

////@end CUpdateSeq_Dlg content construction

    SetRegistryPath("Dialogs.CUpdateSeq_Dlg");
}

void CUpdateSeq_Dlg::SetData(const SUpdateSeqParams& params)
{
    _ASSERT(m_UpdSeqPanel);
    if (m_UpdSeqPanel) {
        m_UpdSeqPanel->SetData(params);
    }
}

SUpdateSeqParams CUpdateSeq_Dlg::GetData() const
{
    _ASSERT(m_UpdSeqPanel);
    if (m_UpdSeqPanel) {
        return m_UpdSeqPanel->GetData();
    }

    SUpdateSeqParams params;
    return params;
}

static CConstRef<CSeq_align> s_CreateAlign(CBioseq_Handle subject, CBioseq_Handle query, ICanceled& canceled)
{
    CConstRef<CSeq_align> result;

    bool accept_atleast_one = (subject.GetBioseqLength() < query.GetBioseqLength());

    if (canceled.IsCanceled())
        return result;

    auto align_vector = sequpd::RunBlast2Seq(subject, query, accept_atleast_one, &canceled);

    if (align_vector.size() > 1) {
        LOG_POST(Info << "More than one alignment was found");
    }

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
        result = align_vector[best_align];
    }

    return result;
}

void CUpdateSeq_Dlg::x_CalculateAlignment()
{
    m_Future = job_async(bind(s_CreateAlign, m_UpdSeqInput->GetOldBioseq(), m_UpdSeqInput->GetUpdateBioseq(), placeholders::_1),
                         "Generate alignment for sequence update");
}

void CUpdateSeq_Dlg::OnIdle(wxIdleEvent& event)
{
    if (m_Future.IsComplete()) {
        string errMsg = "Failed: ";
        try {
            auto result = m_Future();
            m_UpdSeqInput->SetAlignment(result.GetPointerOrNull());
            m_UpdSeqPanel->UpdatePanel(false);
            FindWindow(wxID_OK)->Enable();
            errMsg.clear();
        }
        catch (const CException& e) {
            errMsg += e.GetMsg();
        }
        catch (const exception& e) {
            errMsg += e.what();
        }

        m_Future.reset();

        if (!errMsg.empty()) {
            NcbiMessageBox(errMsg);
            EndModal(wxID_CANCEL);
        }
    }
    else {
        event.Skip();
    }
}

void CUpdateSeq_Dlg::OnUpdateSequence(wxCommandEvent& event)
{
    if (RunningInsideNCBI() && m_UpdSeqInput->OldSeqHasProteinFeats() && !m_UpdSeqPanel->GetUpdateProteins()) {
        string msg = "The protein sequence associated with this nucleotide sequence has protein features."
            "The protein sequence and features will be updated when the nucleotide sequence is updated.";
        auto ret = NcbiMessageBox(msg, eDialog_OkCancel, eIcon_Exclamation, "Warning");
        if (ret == eOK) {
            m_UpdSeqPanel->SetUpdateProteins(true);
            event.Skip();
        }
    }
    else {
        event.Skip();
    }
}

void CUpdateSeq_Dlg::x_LoadSettings(const CRegistryReadView& view)
{
        m_UpdSeqPanel->LoadSettings(view.GetReadView("Panel"));
}

void CUpdateSeq_Dlg::x_SaveSettings(CRegistryWriteView view) const
{
    if (m_UpdSeqPanel)
        m_UpdSeqPanel->SaveSettings(view.GetWriteView("Panel"));
}

/*!
 * Should we show tooltips?
 */

bool CUpdateSeq_Dlg::ShowToolTips()
{
    return true;
}

/*!
 * Get bitmap resources
 */

wxBitmap CUpdateSeq_Dlg::GetBitmapResource( const wxString& name )
{
    // Bitmap retrieval
////@begin CUpdateSeq_Dlg bitmap retrieval
    wxUnusedVar(name);
    return wxNullBitmap;
////@end CUpdateSeq_Dlg bitmap retrieval
}

/*!
 * Get icon resources
 */

wxIcon CUpdateSeq_Dlg::GetIconResource( const wxString& name )
{
    // Icon retrieval
////@begin CUpdateSeq_Dlg icon retrieval
    wxUnusedVar(name);
    return wxNullIcon;
////@end CUpdateSeq_Dlg icon retrieval
}

END_NCBI_SCOPE
