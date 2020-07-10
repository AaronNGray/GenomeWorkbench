/*  $Id: create_needleman_wunsch_tool_manager.cpp 42821 2019-04-18 19:32:56Z joukovv $
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
 * Authors:  Mike DiCuccio
 *
 * File Description:
 *
 */

#include <ncbi_pch.hpp>
#include "create_needleman_wunsch_tool_manager.hpp"

#include <gui/objutils/label.hpp>
#include <objects/gbproj/ProjectItem.hpp>

#include <objects/general/Date.hpp>
#include <objects/seq/Annot_descr.hpp>
#include <objects/seq/Annotdesc.hpp>
#include <objects/seqalign/Seq_align.hpp>
#include <objects/seqalign/Seq_align_set.hpp>
#include <objects/seqalign/Score.hpp>
#include <objects/seqalign/Dense_seg.hpp>

#include <algo/sequence/align_cleanup.hpp>

#include <algo/align/util/blast_tabular.hpp>
#include <algo/align/util/hit_filter.hpp>
#include <algo/align/util/score_builder.hpp>

#include <objmgr/seq_vector.hpp>
#include <objmgr/util/sequence.hpp>
#include <objmgr/seq_loc_mapper.hpp>

#include <util/tables/raw_scoremat.h>

#include <algo/align/nw/nw_aligner.hpp>
#include <algo/align/nw/mm_aligner.hpp>
#include <algo/align/nw/nw_band_aligner.hpp>
#include <algo/align/nw/nw_formatter.hpp>


BEGIN_NCBI_SCOPE
USING_SCOPE(objects);


CCreateNeedlemanWunschToolManager::CCreateNeedlemanWunschToolManager()
:   CAlgoToolManagerBase("Needleman-Wunsch Alignment",
                         "",
                         "Create a Needleman-Wunsch Alignment",
                         "Generate a global alignment using the "
                         "Needleman-Wunsch algorithm",
                         "NEEDLEMAN_WUNSCH",
                         "Alignment Creation"),
    m_Panel(NULL)
{
}


string CCreateNeedlemanWunschToolManager::GetExtensionIdentifier() const
{
    return "create_needleman_wunsch_tool_manager";
}


string CCreateNeedlemanWunschToolManager::GetExtensionLabel() const
{
    return "Needleman-Wunsch Tool";
}


void CCreateNeedlemanWunschToolManager::InitUI()
{
    CAlgoToolManagerBase::InitUI();

    m_Panel = NULL;
}


void CCreateNeedlemanWunschToolManager::CleanUI()
{
    m_Panel = NULL;

    CAlgoToolManagerBase::CleanUI();
}


void CCreateNeedlemanWunschToolManager::x_CreateParamsPanelIfNeeded()
{
    if(m_Panel == NULL)   {
        x_SelectCompatibleInputObjects();

        m_Panel = new CAlignNeedlemanWunschPanel();
        m_Panel->Hide(); // to reduce flicker
        m_Panel->Create(m_ParentWindow);

        m_Panel->SetData(m_Params);
        m_Panel->SetObjects(&m_Objects);

        m_Panel->SetRegistryPath(m_RegPath + ".ParamsPanel");
        m_Panel->LoadSettings();
    }
}


/// select only Seq-aligns
void CCreateNeedlemanWunschToolManager::x_SelectCompatibleInputObjects()
{
    x_ConvertInputObjects(CSeq_loc::GetTypeInfo(), m_Objects);
}


CAlgoToolManagerParamsPanel* CCreateNeedlemanWunschToolManager::x_GetParamsPanel()
{
    return m_Panel;
}


IRegSettings* CCreateNeedlemanWunschToolManager::x_GetParamsAsRegSetting()
{
    return &m_Params;
}


CDataLoadingAppJob* CCreateNeedlemanWunschToolManager::x_CreateLoadingJob()
{
    if (m_Panel) m_Params = m_Panel->GetData();
    CCreateNeedlemanWunschJob* job = new CCreateNeedlemanWunschJob(m_Params);
    return job;
}




///////////////////////////////////////////////////////////////////////////////
/// CCreateNeedlemanWunschJob

CCreateNeedlemanWunschJob::CCreateNeedlemanWunschJob(const CNeedlemanWunschToolParams& params)
:   m_Params(params), m_Done(0)
{
    m_Descr = "Creating alignments"; //TODO
}

static bool s_ProgressCallback(CNWAligner::SProgressInfo* info)
{
    CCreateNeedlemanWunschJob* job =
        reinterpret_cast<CCreateNeedlemanWunschJob*>(info->m_data);
    if ( !job ) {
        return false;
    }

    char buf[128];
    float pct_done = (100.0f / info->m_iter_total) * info->m_iter_done;
    sprintf( buf, "%2.0lf %% completed", pct_done);

    return job->ProgressCallback(buf, pct_done);
}

bool CCreateNeedlemanWunschJob::ProgressCallback(const string& status, float done)
{
    if (IsCanceled())
        return true;

    CFastMutexGuard lock(m_Mutex);

    m_Status = status;
    m_Done = done;
    return false;
}

CConstIRef<IAppJobProgress> CCreateNeedlemanWunschJob::GetProgress()
{
    CFastMutexGuard lock(m_Mutex);
    CConstIRef<IAppJobProgress> pr(new CAppJobProgress(m_Done, m_Status));
    return pr;
}

void CCreateNeedlemanWunschJob::x_CreateProjectItems()
{
    TConstScopedObjects& locs = m_Params.SetObjects();
    if (locs.size() != 2) {
                NCBI_THROW(CException, eUnknown,
                    "Algorithm acepts exactly two sequences");
    }

    // make sure that the sequences are of a known type; fetch the sequences
    CRef<CScope> new_scope;
    vector<string> seqs;
    vector<string> seq_labels;

    typedef vector< CConstRef<CSeq_id> > TIds;
    TIds seq_ids;
    const SNCBIPackedScoreMatrix* scoremat = 0;

    NON_CONST_ITERATE (TConstScopedObjects, loc_iter, locs) {
        const CSeq_loc& loc = dynamic_cast<const CSeq_loc&>(loc_iter->object.GetObject());
        CScope& scope = loc_iter->scope.GetObject();
        if ( !new_scope ) {
            new_scope.Reset(&scope);
        }

        if ( !sequence::IsOneBioseq(loc, &scope) ) {
            string str;
            CLabel::GetLabel(loc, &str, CLabel::eDefault, &scope);
            LOG_POST(Info << "CAlgoPlugin_NeedlemanWunsch: "
                     "location on multiple bioseqs ignored: " << str);
            continue;
        }

        CBioseq_Handle handle =
            scope.GetBioseqHandle(sequence::GetId(loc, &scope));

        CSeqVector vec(loc, scope, CBioseq_Handle::eCoding_Iupac);

        // save our sequence
        seqs.push_back(string());
        vec.GetSeqData(0, vec.size(), seqs.back());
        NStr::ToUpper(seqs.back());

        // save a label for this sequence
        string loc_str;
        CLabel::GetLabel(loc, &loc_str, CLabel::eDefault, &scope);
        seq_labels.push_back(loc_str);

        // save the gi for this sequence
        seq_ids.push_back(CConstRef<CSeq_id>(handle.GetSeqId()));

        scoremat =  vec.IsNucleotide() ? 0: &NCBISM_Blosum62;
    }

    const size_t band = m_Params.GetBandSize();
    bool use_myers_miller = false;
    if(band == 0) {
        const size_t nw_limit = 200*1024*1024;
        vector<string>::const_iterator iter_seqs = seqs.begin();
        double dim_square = (iter_seqs++)->length();
        dim_square *= iter_seqs->length();
        use_myers_miller = dim_square > nw_limit;
    }

    //
    // main algorithm
    //
    string output;
    CNWAligner::TScore score = 0;

    const char* seq1 = seqs[0].c_str(), * seq2 = seqs[1].c_str();
    size_t dim1 = seqs[0].size(), dim2 = seqs[1].size();

    auto_ptr<CNWAligner> aligner;
    if(band > 0) {

        aligner.reset(new CBandAligner (seq1, dim1, seq2, dim2,
                                      scoremat, band));
    }
    else {

        if(use_myers_miller) {

            aligner.reset(new CMMAligner (seq1, dim1, seq2, dim2,
                                          scoremat));

            LOG_POST( Info << "CAlgoPlugin_NeedlemanWunsch: "
                      "Using Myers-Miller method");

        }
        else {

            aligner.reset(new CNWAligner (seq1, dim1, seq2, dim2,
                                          scoremat));
        }
    }

    if(scoremat == 0) {
        aligner->SetWm (m_Params.GetMatchCost());
        aligner->SetWms(m_Params.GetMismatchCost());
        aligner->SetScoreMatrix(NULL);
    }

    aligner->SetWg (m_Params.GetGapOpenCost());
    aligner->SetWs (m_Params.GetGapExtendCost());

    // end-space free alignment setup
    bool  left1  = (m_Params.GetFirstSeqFreeEnds() == CNeedlemanWunschToolParams::eLeft) ||  
                   (m_Params.GetFirstSeqFreeEnds() == CNeedlemanWunschToolParams::eBoth);
    bool  right1 = (m_Params.GetFirstSeqFreeEnds() == CNeedlemanWunschToolParams::eRight) ||  
                   (m_Params.GetFirstSeqFreeEnds() == CNeedlemanWunschToolParams::eBoth);
    bool  left2  = (m_Params.GetSecondSeqFreeEnds() == CNeedlemanWunschToolParams::eLeft) ||  
                   (m_Params.GetSecondSeqFreeEnds() == CNeedlemanWunschToolParams::eBoth);
    bool  right2 = (m_Params.GetSecondSeqFreeEnds() == CNeedlemanWunschToolParams::eRight) ||  
                   (m_Params.GetSecondSeqFreeEnds() == CNeedlemanWunschToolParams::eBoth);
    aligner->SetEndSpaceFree(left1, right1, left2, right2);

    aligner->SetProgressCallback(s_ProgressCallback, this);
    score = aligner->Run();

    if (IsCanceled())
        return;

    // create a seq-align structure for our alignment
    CNWFormatter formatter (*aligner);
    CRef<CSeq_align> align =
        formatter.AsSeqAlign(0, eNa_strand_plus, 0, eNa_strand_plus);

    // we need to set the IDs correctly, and remap our alignment
    list< CRef<CSeq_loc> > from_locs;
    ITERATE (TConstScopedObjects, loc_iter, locs) {
        const CSeq_loc& loc = dynamic_cast<const CSeq_loc&>(loc_iter->object.GetObject());

        CRef<CSeq_loc> ref(new CSeq_loc());
        switch (loc.Which()) {
        case CSeq_loc::e_Whole:
            ref->Assign(loc);
            break;

        default:
            ref->SetInt().SetFrom(0);
            ref->SetInt().SetTo(loc.GetTotalRange().GetLength());
            ref->SetId(sequence::GetId(loc, NULL));
            break;
        }
        from_locs.push_back(ref);
    }

    TIds::iterator seq_id_iter = seq_ids.begin();
    NON_CONST_ITERATE(CDense_seg::TIds, iter,
                      align->SetSegs().SetDenseg().SetIds()) {
        (*iter)->Assign(**seq_id_iter++);

    }

    /// remap the alignment
    list< CRef<CSeq_loc> >::const_iterator iter1 = from_locs.begin();
    NON_CONST_ITERATE (TConstScopedObjects, loc_iter, locs) {
        const CSeq_loc& loc = dynamic_cast<const CSeq_loc&>(loc_iter->object.GetObject());
        CScope& scope = loc_iter->scope.GetObject();

        CRef<CSeq_loc> ref = *iter1++;
        if (loc.IsWhole()) {
            continue;
        }

        CSeq_loc_Mapper mapper(*ref, loc, &scope);
        align = mapper.Map(*align);
    }

    // pack the alignment in a Seq-annot and label it appropriately
    CRef<CSeq_annot> annot(new CSeq_annot());
    annot->SetData().SetAlign().push_back(align);

    // prepare a title
    string title;
    ITERATE (vector<string>, iter, seq_labels) {
        if ( !title.empty() ) {
            title += " + ";
        }
        title += *iter;
    }

    title = "Global alignment of " + title;
    CTime time(CTime::eCurrent);
    annot->SetNameDesc(title);
    annot->SetCreateDate(time);

    string str;
    str = "This alignment was produced on ";
    str += time.AsString();
    str += " using the Needleman-Wunsch alignment algorithm";
    annot->AddComment(str);

    CRef<CAnnotdesc> desc(new CAnnotdesc());
    desc->SetCreate_date().SetStr(time.AsString());
    annot->SetDesc().Set().push_back(desc);

    CRef<CProjectItem> item(new CProjectItem());
    item->SetItem().SetAnnot(*annot);
    item->SetLabel(title);
    AddProjectItem(*item);
}

END_NCBI_SCOPE

