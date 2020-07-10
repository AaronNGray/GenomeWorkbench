/*  $Id: mixedstrands.cpp 38626 2017-06-05 13:53:11Z asztalos $
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
#include <objmgr/bioseq_handle.hpp>
#include <objmgr/bioseq_ci.hpp>
#include <objmgr/util/sequence.hpp>
#include <objtools/edit/loc_edit.hpp>

#include <algo/blast/api/blast_nucl_options.hpp>
#include <algo/blast/api/bl2seq.hpp>

#include <gui/objutils/cmd_composite.hpp>
#include <gui/objutils/cmd_change_bioseq_inst.hpp>
#include <gui/objutils/cmd_change_seq_feat.hpp>
#include <gui/widgets/wx/wx_utils.hpp>
#include <gui/widgets/wx/message_box.hpp>
#include <gui/widgets/edit/generic_report_dlg.hpp>
#include <gui/packages/pkg_sequence_edit/mixedstrands.hpp>

#include <wx/collpane.h>
#include <wx/choice.h>
#include <wx/button.h>
#include <wx/sizer.h>
#include <wx/checkbox.h>
#include <wx/stattext.h>

BEGIN_NCBI_SCOPE
USING_SCOPE(objects);

/*!
 * CMixedStrands type definition
 */

IMPLEMENT_DYNAMIC_CLASS( CMixedStrands, wxDialog )


/*!
 * CMixedStrands event table definition
 */

BEGIN_EVENT_TABLE( CMixedStrands, wxDialog )
    EVT_CHOICE(ID_CHOICE, CMixedStrands::OnSequenceSelected)
END_EVENT_TABLE()


/*!
 * CMixedStrands constructors
 */

CMixedStrands::CMixedStrands()
{
    Init();
}

CMixedStrands::CMixedStrands(wxWindow* parent, CSeq_entry_Handle seh, wxWindowID id, const wxString& caption, const wxPoint& pos, const wxSize& size, long style)
    : m_TopSeqEntry(seh)
{
    Init();
    Create(parent, id, caption, pos, size, style);
}


/*!
 * CMixedStrands creator
 */

bool CMixedStrands::Create( wxWindow* parent, wxWindowID id, const wxString& caption, const wxPoint& pos, const wxSize& size, long style )
{
////@begin CMixedStrands creation
    SetExtraStyle(wxWS_EX_BLOCK_EVENTS);
    wxDialog::Create( parent, id, caption, pos, size, style );

    CreateControls();
    if (GetSizer())
    {
        GetSizer()->SetSizeHints(this);
    }
    Centre();
////@end CMixedStrands creation
    return true;
}


/*!
 * CMixedStrands destructor
 */

CMixedStrands::~CMixedStrands()
{
    // job is canceled if main window is destroyed
    if (m_JobAdapter) {
        m_JobAdapter->Cancel();
    }
}


/*!
 * Member initialisation
 */

void CMixedStrands::Init()
{
////@begin CMixedStrands member initialisation
    m_Sequences = NULL;
    m_RevFeatChkbox = NULL;
////@end CMixedStrands member initialisation
}


/*!
 * Control creation for CMixedStrands
 */

void CMixedStrands::CreateControls()
{    
////@begin CMixedStrands content construction
    CMixedStrands* itemDialog1 = this;

    wxBoxSizer* itemBoxSizer2 = new wxBoxSizer(wxVERTICAL);
    itemDialog1->SetSizer(itemBoxSizer2);

    m_Description = new wxStaticText(itemDialog1, wxID_STATIC, _("Checking for mixed strands within the set..."), wxDefaultPosition, wxDefaultSize, 0);
    itemBoxSizer2->Add(m_Description, 0, wxALIGN_CENTER_HORIZONTAL | wxALL, 5);
#if defined(__WXMSW__)
    m_ColPane = new wxGenericCollapsiblePane(itemDialog1, ID_COLLAPSIBLEPANE, _("Reverse them"), wxDefaultPosition, wxDefaultSize, wxCP_DEFAULT_STYLE);
#else
    m_ColPane = new wxCollapsiblePane(itemDialog1, ID_COLLAPSIBLEPANE, _("Reverse them"), wxDefaultPosition, wxDefaultSize, wxCP_DEFAULT_STYLE);
#endif
    itemBoxSizer2->Add(m_ColPane, 0, wxALIGN_LEFT | wxALL, 5);
    m_ColPane->Collapse(false);

    wxBoxSizer* itemBoxSizer5 = new wxBoxSizer(wxVERTICAL);
    m_ColPane->GetPane()->SetSizer(itemBoxSizer5);

    wxBoxSizer* itemBoxSizer6 = new wxBoxSizer(wxHORIZONTAL);
    itemBoxSizer5->Add(itemBoxSizer6, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 0);

    wxStaticText* itemStaticText7 = new wxStaticText(m_ColPane->GetPane(), wxID_STATIC, _("Sequence with the correct strand"), wxDefaultPosition, wxDefaultSize, 0);
    itemBoxSizer6->Add(itemStaticText7, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxArrayString m_SequencesStrings;
    m_Sequences = new wxChoice(m_ColPane->GetPane(), ID_CHOICE, wxDefaultPosition, wxDefaultSize, m_SequencesStrings, 0);
    itemBoxSizer6->Add(m_Sequences, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    m_RevFeatChkbox = new wxCheckBox(m_ColPane->GetPane(), ID_CHECKBOX1, _("Reverse the strand for the features"), wxDefaultPosition, wxDefaultSize, 0);
    m_RevFeatChkbox->SetValue(true);
    itemBoxSizer5->Add(m_RevFeatChkbox, 0, wxALIGN_LEFT|wxALL, 5);

    wxBoxSizer* itemBoxSizer10 = new wxBoxSizer(wxHORIZONTAL);
    itemBoxSizer2->Add(itemBoxSizer10, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

    wxButton* itemButton11 = new wxButton( itemDialog1, wxID_OK, _("Accept"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer10->Add(itemButton11, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);
    itemButton11->Enable(false);

    wxButton* itemButton12 = new wxButton( itemDialog1, wxID_CANCEL, _("Cancel"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer10->Add(itemButton12, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    x_InitDialog();

////@end CMixedStrands content construction
}


/*!
 * Should we show tooltips?
 */

bool CMixedStrands::ShowToolTips()
{
    return true;
}

/*!
 * Get bitmap resources
 */

wxBitmap CMixedStrands::GetBitmapResource( const wxString& name )
{
    // Bitmap retrieval
////@begin CMixedStrands bitmap retrieval
    wxUnusedVar(name);
    return wxNullBitmap;
////@end CMixedStrands bitmap retrieval
}

/*!
 * Get icon resources
 */

wxIcon CMixedStrands::GetIconResource( const wxString& name )
{
    // Icon retrieval
////@begin CMixedStrands icon retrieval
    wxUnusedVar(name);
    return wxNullIcon;
////@end CMixedStrands icon retrieval
}

CMixedStrands::TAlignVector CMixedStrands::s_GetAlignVector(const blast::TSeqAlignVector& seqaligns)
{
    CMixedStrands::TAlignVector align_vector;
    if (!seqaligns.empty()) {
        ITERATE(blast::TSeqAlignVector, it, seqaligns) {
            if ((*it)->IsSet()) {
                ITERATE(CSeq_align_set::Tdata, al_it, (*it)->Get()) {
                    align_vector.push_back(CConstRef<CSeq_align>((*al_it)));
                }
            }
        }
    }
    return align_vector;
}

CMixedStrands::TAlignVector CMixedStrands::s_Blast2NASeq(const CBioseq_Handle& sh, const CBioseq_Handle& qh)
{
    CMixedStrands::TAlignVector align_vector;

    CRef<CSeq_loc> query_seqloc = qh.GetRangeSeq_loc(0, 0);
    CRef<CSeq_loc> subject_seqloc = sh.GetRangeSeq_loc(0, 0);
    
    blast::SSeqLoc query(query_seqloc.GetPointerOrNull(), &qh.GetScope());
    blast::SSeqLoc subject(subject_seqloc.GetPointerOrNull(), &sh.GetScope());

    // using the BLAST best hits algorithm
    try {
        // blastn -gapextend 5 -gapopen 11 -max_target_seqs 250 -penalty -5 -reward 1 -soft_masking true 
        //        -task megablast -window_size 150 -word_size 12 -dust yes -best_hit_overhang 0.1 -best_hit_score_edge 0.1
        CRef<blast::CBlastNucleotideOptionsHandle> NucOptions(new blast::CBlastNucleotideOptionsHandle);
        CRef<blast::CBlastOptionsHandle> Options(&*NucOptions);
        NucOptions->SetTraditionalMegablastDefaults();  // megablast
        NucOptions->SetGapExtensionCost(5);
        NucOptions->SetGapOpeningCost(11);
        NucOptions->SetMismatchPenalty(-5);
        NucOptions->SetMatchReward(1);
        NucOptions->SetWordSize(12);
        NucOptions->SetWindowSize(150);
        NucOptions->SetDustFiltering(true);
        NucOptions->SetEvalueThreshold(1e-6);

        blast::CBlastOptions& opts = Options->SetOptions();
        opts.SetBestHitOverhang(0.1);  // best_hit_score_edge
        opts.SetBestHitScoreEdge(0.1);  // best_hit_overhang
        opts.SetHitlistSize(250);  // max_target_seqs
        opts.SetMaskLevel(1);   // soft_masking true

        Options->Validate();
        blast::CBl2Seq blaster(query, subject, *NucOptions);
        blast::TSeqAlignVector seqaligns = blaster.Run();

        align_vector = s_GetAlignVector(seqaligns);
        if (align_vector.empty()) {
            blast::CBl2Seq nblaster(query, subject, blast::eBlastn);
            blast::TSeqAlignVector seqaligns = nblaster.Run();
            align_vector = s_GetAlignVector(seqaligns);
        }
    }
    catch (const blast::CBlastException& e) {
        ERR_POST(Error << string(e.what()));
    }

    return align_vector;
}


namespace {
    struct SAlignJobInput
    {
        CSeq_entry_Handle m_Seh;
    };


    bool s_DoesAlignHaveMixedStrands(const CMixedStrands::TAlignVector& align_vector)
    {
        for (auto&& it : align_vector) {
            const CSeq_align& align = it.GetObject();

            CSeq_align::TDim num_row = 0;
            try {
                num_row = align.CheckNumRows();
            }
            catch (const CException&) {
            }

            if (num_row != 2) { // not a pairwise alignment
                continue;
            }
            // TODO: check the relevant indicators: percent identity, coverage

            // check the strands
            if (align.GetSeqStrand(0) != align.GetSeqStrand(1)) {
                return true;
            }
        }
        return false;
    }

    // Returns true if there is at least one pairwise alignment with mixed strand
    bool s_FindMixedStrandAlignment(SAlignJobInput& input, bool& output, string& error, ICanceled& canceled)
    {
        if (canceled.IsCanceled())
            return false;

        LOG_POST(Info << "Start alignment construction... ");

        bool mixed_strands = false;
        for (CBioseq_CI master_iter(input.m_Seh, CSeq_inst::eMol_na); master_iter && !mixed_strands; ++master_iter) {
            if (canceled.IsCanceled())
                return false;

            CBioseq_CI queryh(master_iter);
            ++queryh;
            while (queryh && !mixed_strands) {
                if (canceled.IsCanceled())
                    return false;

                CMixedStrands::TAlignVector align_vector = CMixedStrands::s_Blast2NASeq(*master_iter, *queryh);
                mixed_strands = s_DoesAlignHaveMixedStrands(align_vector);
                ++queryh;
            }
        }

        output = mixed_strands;
        return true;
    }
}

void CMixedStrands::OnJobResult(CObject* result, CJobAdapter&)
{
    CJobAdapterResult<bool>* jobResult = dynamic_cast<CJobAdapterResult<bool>*>(result);
    if (jobResult) {
        LOG_POST(Info << "Finished generating pairwise alignments.");

        const bool mixed_strands  = jobResult->GetData();
        string msg("This set does not have sequences of mixed strands.");
        if (mixed_strands) {
            msg.assign("This set has sequences of mixed strands.");
            x_AddSequenceIds();
        }
        
        m_Description->SetLabel(ToWxString(msg));
        Fit();
        Layout();
        Refresh();
    }
}

void CMixedStrands::OnJobFailed(const string& errMsg, CJobAdapter&)
{
    string err_msg = "Failed: ";
    if (!errMsg.empty()) {
        err_msg += errMsg;
    }
    else {
        err_msg += "Unknown fatal error";
    }

    NcbiErrorBox(err_msg);
}

void CMixedStrands::x_InitDialog()
{
    if (m_JobAdapter) m_JobAdapter->Cancel();

    SAlignJobInput input;
    input.m_Seh = m_TopSeqEntry;

    m_JobAdapter.Reset(LaunchAdapterJob<SAlignJobInput, bool>
        (this, input, s_FindMixedStrandAlignment, "MixedStrandJob", "Find mixed strand pairwise alignment within set"));
}

void CMixedStrands::x_AddSequenceIds()
{
    for (CBioseq_CI b_iter(m_TopSeqEntry, CSeq_inst::eMol_na); b_iter; ++b_iter) {
        string label;
        CSeq_id_Handle best = sequence::GetId(*b_iter, sequence::eGetId_Best);
        best.GetSeqId()->GetLabel(&label, CSeq_id::eContent);
        if (!label.empty()) {
            m_Sequences->Append(label);
            m_BshList.push_back(*b_iter);
        }
    }
}

void CMixedStrands::OnSequenceSelected(wxCommandEvent& event)
{
    bool enable = (m_Sequences->GetCurrentSelection() != -1) ? true : false;
    FindWindowById(wxID_OK)->Enable(enable);
}

CRef<CCmdComposite> CMixedStrands::GetCommand()
{
    CRef<CCmdComposite> cmd(new CCmdComposite("Reverse complement selected sequences"));

    int selected = m_Sequences->GetCurrentSelection();
    if (selected == -1 || selected >= m_BshList.size()) {
        return CRef<CCmdComposite>();
    }
    
    CBioseq_Handle bsh = m_BshList[selected]; // the bioseq that has the correct strand

    // find sequences that are on the other strand than bsh
    CScope& scope = m_TopSeqEntry.GetScope();
    unsigned int num_reversed = 0, count = 0;
    CNcbiOstrstream oss;
    oss << "The following sequences were reversed:\n";
    for (CBioseq_CI b_iter(m_TopSeqEntry, CSeq_inst::eMol_na); b_iter; ++b_iter) {
        count++;
        if (bsh == *b_iter) 
            continue;
        
        bool tobe_reversed = false;
        TAlignVector align_vector = s_Blast2NASeq(bsh, *b_iter);
        tobe_reversed = s_DoesAlignHaveMixedStrands(align_vector);
        if (tobe_reversed) {
            CRef<CSeq_inst> new_inst(new CSeq_inst);
            new_inst->Assign(b_iter->GetInst());
            ReverseComplement(*new_inst, &scope);
            CRef<CCmdChangeBioseqInst> cmd_inst(new CCmdChangeBioseqInst(*b_iter, *new_inst));
            cmd->AddCommand(*cmd_inst);

            num_reversed++;
            string label;
            CSeq_id_Handle best = sequence::GetId(*b_iter, sequence::eGetId_Best);
            best.GetSeqId()->GetLabel(&label, CSeq_id::eContent);
            oss << label << "\n";

            // reverse their features as well is it's required
            if (m_RevFeatChkbox->IsChecked()) {
                for (CFeat_CI feat_it(*b_iter); feat_it; ++feat_it) {
                    CRef<CSeq_feat> new_feat(new CSeq_feat);
                    new_feat->Assign(*feat_it->GetOriginalSeq_feat());
                    edit::ReverseComplementFeature(*new_feat, scope);
                    CRef<CCmdChangeSeq_feat> cmd_feat(new CCmdChangeSeq_feat(*feat_it, *new_feat));
                    cmd->AddCommand(*cmd_feat);
                }
            }
        }
    }
    
    if (num_reversed > 0) {
        oss << num_reversed << " out of " << count << " sequences were reversed.";
        m_Report = string(CNcbiOstrstreamToString(oss));
        return cmd;
    }

    return CRef<CCmdComposite>();
}

void CMixedStrands::Report()
{
    if (m_Report.empty()) 
        return;
    CGenericReportDlg* reportdlg = new CGenericReportDlg(NULL);
    reportdlg->SetTitle("Reverse complement sequences log");
    reportdlg->SetText(ToWxString(m_Report));
    reportdlg->Show(true);
}


END_NCBI_SCOPE

