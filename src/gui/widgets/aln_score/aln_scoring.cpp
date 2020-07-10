/*  $Id: aln_scoring.cpp 44842 2020-03-25 17:20:30Z shkeda $
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
 * Authors:  Andrey Yazhuk
 */

#include <ncbi_pch.hpp>

#include <corelib/ncbistd.hpp>
#include <corelib/ncbistl.hpp>
#include <corelib/ncbireg.hpp>
#include <corelib/ncbifile.hpp>

#include <gui/widgets/aln_score/aln_scoring.hpp>

#include <gui/utils/event_translator.hpp>
#include <objects/seqfeat/Genetic_code_table.hpp>
#include <objmgr/util/sequence.hpp>

#include <stdio.h>
#include <math.h>
#include <numeric>

BEGIN_NCBI_SCOPE
USING_SCOPE(objects);

///////////////////////////////////////////////////////////////////////////////
/// CScoringParams

CScoringParams::CScoringParams()
: m_Alignment(NULL),
  m_Method(NULL),
  m_GradNumber(16)
{
}


///////////////////////////////////////////////////////////////////////////////
/// CScoreCache

CScoreCache::CScoreCache()
: m_ScoreColls(NULL),
  m_EnBgProcessing(true),
  m_JobID(-1),
  m_HasScores(false)
{
    m_ScoreColls = new TScoreCollVector();
}


CScoreCache::~CScoreCache()
{
    if(x_IsJobRunning())   {
        x_DeleteJob();
    }

    delete m_ScoreColls;
}


BEGIN_EVENT_MAP(CScoreCache, CEventHandler)
    ON_EVENT(CAppJobNotification, CAppJobNotification::eStateChanged,
             &CScoreCache::OnAJNotification)
    ON_EVENT(CAppJobNotification, CAppJobNotification::eProgress,
             &CScoreCache::OnAJNotification)
END_EVENT_MAP()


void CScoreCache::SetListener(IListener* listener)
{
    m_Listener = listener;
}


void CScoreCache::EnableBackgoundProcessing(bool en)
{
    m_EnBgProcessing = en;
}


void CScoreCache::SetGradNumber(int grad_n)
{
    _ASSERT(grad_n > 1  && grad_n <= 0xFFFF);
    m_Params.m_GradNumber = grad_n;
}


void CScoreCache::SetScoringMethod(IScoringMethod *method)
{
    // changing method requires sopping the running job
    if(x_IsJobRunning())    {
        x_DeleteJob();
    }

    m_Params.m_Method = method;
}


void CScoreCache::ResetScores()
{
    //LOG_POST("CScoreCache::CalculateScores() - Started ");
    CAppJobDispatcher& disp = CAppJobDispatcher::GetInstance();

    if(m_Job)   {
        // job is already running - delete it
        if (m_EnBgProcessing) {
            disp.DeleteJob(m_JobID);
            m_JobID = -1;
        }
        m_Job.Reset();
    }

    m_HasScores = false;
    m_ScoreColls->clear();
}

/// Calculates scores for the given CAlnVec object and saves results in form of
/// TScoreColl objects.
void CScoreCache::CalculateScores()
{
    ResetScores();

    if(m_Params.m_Method)   {
        m_Job.Reset(new CScoringJob(m_Params));

        if(m_EnBgProcessing)    {
            CAppJobDispatcher& disp = CAppJobDispatcher::GetInstance();
            // use CAppJobDispatcher to execute background jobs
            m_JobID = disp.StartJob(*m_Job, "ThreadPool", *this, 1, true);
        } else {
            // do everything synchronously
            m_JobID = -1;
            m_Job->Run();
            m_HasScores = x_TransferResults(m_Job->GetResult().GetPointer());
            m_Job.Reset();
        }
    }
    //LOG_POST("CScoreCache::CalculateScores() - Finished ");
}


void CScoreCache::OnAJNotification(CEvent* evt)
{
    CAppJobNotification* notn =
        dynamic_cast<CAppJobNotification*>(evt);
    _ASSERT(notn);

    if(notn)    {
        int job_id = notn->GetJobID();
        if(m_JobID != job_id) {
            ERR_POST("CScoringCache::OnAJNotification() - unknown Job ID " << job_id);
        } else {
            switch(notn->GetState())    {
            case IAppJob::eCompleted:
                x_OnJobCompleted(*notn);
                break;
            case IAppJob::eFailed:
                x_OnJobFailed(*notn);
                break;
            case IAppJob::eCanceled:
                x_OnJobCanceled(*notn);
                break;
            case IAppJob::eRunning:
                x_OnJobProgress(*notn);
                break;
            default:
                _ASSERT(false);
            }
        }
    }
}


bool CScoreCache::x_IsJobRunning()
{
    return m_Job.GetPointer() != NULL;
}


void CScoreCache::x_DeleteJob()
{
    if (m_EnBgProcessing) {
        _ASSERT(m_Job  &&  m_JobID != -1);

        CAppJobDispatcher& disp = CAppJobDispatcher::GetInstance();
        disp.DeleteJob(m_JobID);

        m_JobID = -1;
    }

    m_Job.Reset();

    if(m_Listener)  {
        m_Listener->OnScoringFinished();
    }
}


void CScoreCache::x_OnJobCompleted(CAppJobNotification& notn)
{
    CRef<CObject> obj = notn.GetResult();
    x_TransferResults(obj.GetPointer());

    m_JobID = -1;
    m_Job.Reset();

    if(m_Listener)  {
        //LOG_POST("Completed - m_Listener->OnScoringFinished()");
        m_Listener->OnScoringFinished();
    }
}


bool CScoreCache::x_TransferResults(CObject* result)
{
    CScoringJobResult* sc_res = dynamic_cast<CScoringJobResult*>(result);
    if(sc_res) {
        delete m_ScoreColls; // delete old data

        // take ownership of the results
        m_ScoreColls = sc_res->m_ScoreColls;
        sc_res->m_ScoreColls = NULL;

        m_HasScores = true;
        _ASSERT(m_ScoreColls );
        return true;
    } else {
        ERR_POST("CScoreCache::x_TransferResults()  - invalid results!");
        _ASSERT(sc_res);
        return false;
    }
}


void CScoreCache::x_OnJobFailed(CAppJobNotification& notn)
{
    CConstIRef<IAppJobError> err = notn.GetError();
    if(err) {
        //TODO
    } else {
        //TODO
    }

    m_JobID = -1;
    m_Job.Reset();

    if(m_Listener)  {
        //LOG_POST("Failed - m_Listener->OnScoringFinished()");
        m_Listener->OnScoringFinished();
    }
}


void CScoreCache::x_OnJobCanceled(CAppJobNotification& /*notn*/)
{
    m_JobID = -1;
    m_Job.Reset();

    if(m_Listener)  {
        //LOG_POST("Canceled - m_Listener->OnScoringFinished()");
        m_Listener->OnScoringFinished();
    }
}


void CScoreCache::x_OnJobProgress(CAppJobNotification& notn)
{
    CConstIRef<IAppJobProgress> prg = notn.GetProgress();
    if(prg) {

        if(m_Listener)  {
            float norm_done = prg->GetNormDone();
            m_Listener->OnScoringProgress(norm_done, "Calculating alignment coloration...");
        }
    }
}


bool    CScoreCache::HasScores() const
{
    return m_HasScores;
}


const CRgbaColor& CScoreCache::GetColorForNoScore(IScoringMethod::EColorType type) const
{
    return m_Params.m_Method->GetColorForNoScore(type);
}


const CScoreCache::TScoreColl&   CScoreCache::GetScores(TNumrow row) const
{
    _ASSERT(m_ScoreColls  &&  row >= 0  && row < (TNumrow) m_ScoreColls->size());

    return (*m_ScoreColls)[row];
}



///////////////////////////////////////////////////////////////////////////////
/// CScoringJob

CScoringJob::CScoringJob(CScoringParams& params)
:   m_ScoreColls(NULL),
    m_BufferStart(0),
    m_RowLength(0)
{
    m_Params.m_Alignment = params.m_Alignment;

    CIRef<IUITool> tool(params.m_Method->Clone()); // make a copy
    IScoringMethod* method = dynamic_cast<IScoringMethod*>(tool.GetPointer());
    _ASSERT(method);
    m_Params.m_Method.Reset(method);

    m_Params.m_GradNumber = params.m_GradNumber;

    m_ScoreColls = new TScoreCollVector();
    _ASSERT(m_Params.m_Method  &&  m_Params.m_Alignment);

    m_Descr = "Alignment Scoring Job, method " + m_Params.m_Method->GetName();
    //LOG_POST("CScoringJob::CScoringJob()  " << m_Descr);
}


CScoringJob::~CScoringJob()
{
    //LOG_POST("CScoringJob::~CScoringJob() Destructor  " << m_Descr);
    delete m_ScoreColls;
}


IAppJob::EJobState CScoringJob::Run()
{
/*
    LOG_POST(Info << "CScoringJob::Run()  Started  " << m_Descr
                  << " BufferStart=" << m_BufferStart
                  << " RowLength="   << m_RowLength
                  << " m_vRows.size()= " << m_vRows.size()
        );
*/
    m_Result.Reset();
    m_Error.Reset();

    if(Calculate()) {
        // trnasfer collection onwership to the result
        m_Result.Reset(new CScoringJobResult(m_ScoreColls));
        m_ScoreColls = NULL;
        return eCompleted;
    }
    //LOG_POST("CScoringJob::Run()  Finished  " << m_Descr);
    return eFailed;
}



CConstIRef<IAppJobProgress> CScoringJob::GetProgress()
{
    CAppJobProgress* prg = new CAppJobProgress(m_NormDone, "");
    return CConstIRef<IAppJobProgress>(prg);
}


CRef<CObject> CScoringJob::GetResult()
{
    return CRef<CObject>(m_Result.GetPointer());
}


CConstIRef<IAppJobError> CScoringJob::GetError()
{
    return CConstIRef<IAppJobError>(m_Error.GetPointer());
}


string CScoringJob::GetDescr() const
{
    return m_Descr;
}


bool CScoringJob::Calculate()
{
    _ASSERT(m_ScoreColls);

    if(m_Params.m_Method  &&  GetAlignment())   {
        CStopWatch sw;
        sw.Start();

        TNumrow row_n = GetAlignment()->GetNumRows();
        m_ScoreColls->resize(row_n);

        IRowScoringMethod* row_method =
            dynamic_cast<IRowScoringMethod*>(m_Params.m_Method.GetPointer());
        if(row_method)  {
            x_CalculateSequenceScores(*row_method);
        } else {
            IColumnScoringMethod* col_method =
                dynamic_cast<IColumnScoringMethod*>(m_Params.m_Method.GetPointer());

            if(col_method)  {
                if(col_method->CanCalculateScores(*GetAlignment())) {
                    col_method->UpdateSettings(*GetAlignment());
                    x_CalculateAlignmentScores(*col_method);
                } else {
                    return false;
                }
            } else  {
                    _ASSERT(false); // invalid method
            }
        }
        IWindowScoringMethod * win_method
            = dynamic_cast<IWindowScoringMethod*>(m_Params.m_Method.GetPointer());
        if (win_method  &&  win_method->GetWindowSize() > 0) {
            x_WindowAvgScores(win_method->GetWindowSize());
        }
    }
    return true;
}


CScoringJob::TScoreCollVector* CScoringJob::GetResults()
{
    TScoreCollVector* res = m_ScoreColls;
    m_ScoreColls = NULL;
    return res;
}


///////////////////////////////////////////////////////////////////////////////
/// Sequence buffer management routins

inline char CScoringJob::x_BufferGetSeq(TSeqPos pos,  TNumrow row) const
{
    _ASSERT(pos >= m_BufferStart  &&  pos < m_BufferStart + m_RowLength);
    _ASSERT(row >= 0  &&  row < (TNumrow) m_vRows.size());

    return m_vRows[row][pos - m_BufferStart];
}


void CScoringJob::x_AllocBuffer(TSeqPos row_len)
{
    _ASSERT(GetAlignment());

    int rows_n = GetAlignment()->GetNumRows();
    if(rows_n != (TNumrow) m_vRows.size()  ||  m_RowLength != row_len)    {
        m_RowLength = row_len;
        m_vRows.resize(rows_n);
        for (auto& itR : m_vRows) 
            itR.resize(m_RowLength);
    }
}


void CScoringJob::x_FreeBuffer()
{
    m_vRows.clear();
}

void CScoringJob::x_UpdateBuffer(TSeqPos start, TSeqPos stop, int& state, bool translate)
{
    _ASSERT(GetAlignment());
    _ASSERT( (stop - start + 1) <= m_RowLength);

    m_BufferStart = start;
    m_RowLength = stop - start + 1;

    TNumrow row_n = (TNumrow) m_vRows.size();
    vector<size_t> row_size(row_n);
    IAlnExplorer::TSignedRange range(start, stop);
    bool is_mixed = false;
    int base_width = -1;
    for (TNumrow r = 0; r < row_n; r++) {
        GetAlignment()->GetAlnSeqString(r, m_vRows[r],range);
        if (r == 0)
            base_width = GetAlignment()->GetBaseWidth(r);
        else if (!is_mixed && base_width != GetAlignment()->GetBaseWidth(r)) {
            is_mixed = true;
        }
    }
    if (translate && !is_mixed && base_width == 1) {
        m_BufferStart /= 3;
    }

    for( TNumrow r = 0;  r < row_n; r++ )  {
        size_t seq_len = m_vRows[r].size();
        if (is_mixed) {
            string new_seq;
            if (GetAlignment()->GetBaseWidth(r) == 1) {
                TranslateNAToAA(m_vRows[r], new_seq, state, GetAlignment()->GetGenCode(r));
                seq_len = new_seq.size();
                m_vRows[r] = move(new_seq);
            } 

            // The protein sequence is in native protein coordinate,
            // we need to convert it to alignment coordinate which 
            // is 3 times the protein coordinate
            new_seq.resize(m_RowLength);
            for (size_t i = 0; i < seq_len; ++i) {
                size_t n_i = 3 * i;
                _ASSERT(n_i + 2 < new_seq.size());
                if (n_i > new_seq.size() - 2)
                    break;
                new_seq[n_i] = new_seq[n_i + 1] = new_seq[n_i + 2] = m_vRows[r][i];
            }

            m_vRows[r] = move(new_seq);
            seq_len = seq_len * 3;
        }
        else if (translate && base_width == 1) {
            string new_seq;
            TranslateNAToAA(m_vRows[r], new_seq, state, GetAlignment()->GetGenCode(r));
            seq_len = new_seq.size();
            m_vRows[r] = move(new_seq);
        }

        if (seq_len < m_RowLength) {
            // padding the short row with gap char '-'
            m_vRows[r].append(m_RowLength - seq_len, '-');
        }
    }
}


void CScoringJob::TranslateNAToAA(const string& na, string& aa,
                                 int& state, int gencode) const
{
    const objects::CTrans_table& tbl =
        objects::CGen_code_table::GetTransTable(gencode);

    size_t na_remainder = na.size() % 3;
    size_t na_size = na.size() - na_remainder;

    if (&aa != &na) {
        aa.resize(na_size / 3 + (na_remainder ? 1 : 0));
    }

    if ( na.empty() ) return;

    size_t aa_i = 0;
    for (size_t na_i = 0;  na_i < na_size; ) {
        for (size_t i = 0;  i < 3;  ++i, ++na_i) {
            state = tbl.NextCodonState(state, na[na_i]);
        }
        aa[aa_i++] = tbl.GetCodonResidue(state);
    }
    if (na_remainder) {
        aa[aa_i++] = '\\';
    }

    if (&aa == &na) {
        aa.resize(aa_i);
    }
}


void CScoringJob::x_BufferGetColumn(TSeqPos pos, string& column) const
{
    _ASSERT(pos >= m_BufferStart  && pos < m_BufferStart + m_RowLength);

    size_t col = pos - m_BufferStart;
    for(  size_t row = 0;  row < m_vRows.size();  row++ )   {
       column[row] = m_vRows[row][col];
    }
}


void CScoringJob::x_CalculateSequenceScores(IRowScoringMethod& method)
{
    TNumrow row_n = GetAlignment()->GetNumRows();
    for(TNumrow r = 0;  r < row_n  &&  ! IsCanceled();  r++ )  {
        m_NormDone = float(r) / row_n;
        method.CalculateScores(r,  *GetAlignment(),  (*m_ScoreColls)[r]);
        //Sleep(1000);
    }
}

// For protein-to-genomic alignements, we need to do sequence translation.
// So we need to take care of codon boundary to make sure we consider the
// frame shift caused by various reasons such as product insertion. So we
// need to get a list of alignment ranges.
static 
void s_SetRangesForMixedAlignments(const IScoringAlignment* aln_mgr, vector<TSeqRange>& ranges)
{
    _ASSERT(aln_mgr->IsMixedAlign() && aln_mgr->GetAnchor() >= 0);
    TSeqPos aln_start = aln_mgr->GetAlnStart();
    TSeqPos aln_stop = aln_mgr->GetAlnStop();

    //&& aln_mgr->GetNumRows() == 2) {
    // Currently, only pairwise protein-to-genomic alignments with anchor
    // row set are handled. It is too complicated to deal with multiple 
    // protein-to-genomic alignments.
    IAlnExplorer::TNumrow anchor = aln_mgr->GetAnchor();
    for (IAlnExplorer::TNumrow aligned_row = 0; aligned_row < aln_mgr->GetNumRows(); ++aligned_row) {
        if (aligned_row == anchor)
            continue;
        IAlnExplorer::TSignedRange aln_range((TSignedSeqPos)aln_start,
                (TSignedSeqPos)aln_stop);
        auto_ptr<IAlnSegmentIterator> seg_it(aln_mgr->CreateSegmentIterator(
            aligned_row, aln_range, IAlnSegmentIterator::eSkipGaps));
        if (!seg_it.get())
            continue;
        TSeqPos anchor_width = aln_mgr->GetBaseWidth(anchor);
        TSeqPos aligned_width = aln_mgr->GetBaseWidth(aligned_row);
        for (IAlnSegmentIterator& it = *seg_it; it; ++it) {
            const IAlnSegment& seg = *it;
            IAlnSegment::TSignedRange curr_aln_r = seg.GetAlnRange();
            TSignedSeqPos prot_from = curr_aln_r.GetFrom();
            TSignedSeqPos prot_to = curr_aln_r.GetTo();
            if (anchor_width == 3) {
                prot_from = aln_mgr->GetSeqPosFromAlnPos(anchor, prot_from);
                prot_to = aln_mgr->GetSeqPosFromAlnPos(anchor, prot_to);
            } else if (aligned_width == 3) {
                prot_from = aln_mgr->GetSeqPosFromAlnPos(aligned_row, prot_from);
                prot_to = aln_mgr->GetSeqPosFromAlnPos(aligned_row, prot_to);
            } else {
                // It's a mixed alignment and anchor's type (na or aa) is equal to row's type
                // Is is possible?
                _ASSERT(true);
                ranges.emplace_back(curr_aln_r.GetFrom(), curr_aln_r.GetTo());
                continue;
            }

            bool negative = prot_from > prot_to;
            if (negative) 
                swap(prot_from, prot_to);
            TSignedSeqPos f_off = prot_from % 3;
            TSignedSeqPos t_off = 2 - prot_to % 3;
            if (negative) 
                swap(f_off, t_off);
            ranges.emplace_back(curr_aln_r.GetFrom() - f_off, curr_aln_r.GetTo() + t_off);
        }
    }
}

void CScoringJob::x_CalculateAlignmentScores(IColumnScoringMethod& method)
{
    _ASSERT(m_ScoreColls);

    const IScoringAlignment* aln_mgr = GetAlignment();
    TSeqPos aln_start = aln_mgr->GetAlnStart();
    TSeqPos aln_stop = aln_mgr->GetAlnStop();
    TNumrow row_n = aln_mgr->GetNumRows();
    if (row_n == 0)
        return;

    // preparing score collections
    NON_CONST_ITERATE(TScoreCollVector,  itC,  *m_ScoreColls)  {
        itC->SetFrom(aln_start); // clear and initialize
    }

    string column(row_n, '\0');
    TScore col_score = 0;
    TScoreVector v_col_scores(row_n,  0.0);

    // make it a multiple of 3 for protein-to-genomic alignments
    // such that we have exact 3-to-1 match between DNA codon and 
    // protein sequence.

    // max buffer for all rows is ~50k
    // if num_rows >16K the buffer grows unconditionally to num_rows x 300 
    // however, num_rows is expected to be equal to two in most(?) cases
    const TSeqPos kPageSize = max(100, (16 * 1024) / row_n) * 3;
    TScore grad_n = (TScore) m_Params.m_GradNumber;
    TNumrow cons_row = aln_mgr->GetAnchor();

    vector<TSeqRange> ranges;
    auto mixed = aln_mgr->IsMixedAlign();
    if (mixed && aln_mgr->GetAnchor() >= 0)
        s_SetRangesForMixedAlignments(aln_mgr, ranges);
    if (ranges.empty()) 
        ranges.emplace_back(aln_start, aln_stop);
    
    int col_width = 1;
    bool need_translation = false;
    if (method.NeedsTranslation() && !mixed && aln_mgr->GetBaseWidth(0) == 1) {
        need_translation = true;
        col_width = 3;
    }
    int state = 0;
    TSeqPos pre_pos = aln_start;
    ITERATE (vector<TSeqRange>, r_iter, ranges) {
        TSeqPos start = r_iter->GetFrom();
        TSeqPos stop = r_iter->GetTo();

        if (pre_pos < start) {
            // fill unaligned range with dummy values
            // to account for possible inserts in the beggining of the alignment
            // See example in GB-3561
            for (TNumrow r = 0; r < row_n; r++) {
                (*m_ScoreColls)[r].push_back(0, start - pre_pos);
            }
            pre_pos = start;
        }

        x_AllocBuffer(kPageSize);
        // iterate from "start" to "stop" using "sliding buffer"
        for (TSeqPos pos = pre_pos;  pos < stop  &&  ! IsCanceled();) {
            m_NormDone = float(pos - start) / (stop - start); // update progress

            TSeqPos pos_stop = min(pos + kPageSize -1,  stop);
            x_UpdateBuffer(pos, pos_stop, state, need_translation); // fetch next page in Seq Buffer
            TSeqPos p = pos;
            while (p <= pos_stop) { // for each column
                if (IsCanceled())
                    break;
                x_BufferGetColumn(p/col_width, column);
                char cons = (cons_row > -1) ? column[cons_row] : 0;
                method.CalculateScores(cons,  column,  col_score,  v_col_scores);
                // append scores to collections
                for(TNumrow r = 0;  r < row_n;  r++ )  {
                    TScore sc = v_col_scores[r];
                    sc = ((int) (sc * grad_n)) / grad_n;
                    (*m_ScoreColls)[r].push_back(sc, col_width);
                }
                p += col_width;
            }
            pos = pos_stop + 1;
        }
        pre_pos = stop + 1;
    }
    /*
    for (TNumrow r = 0; r < row_n; r++) {
        cout << "============================================================" << endl;
        for (int i = 0; i < (*m_ScoreColls)[r].size(); ++i) {
            cout << (*m_ScoreColls)[r][i].GetFrom() << ".." << (*m_ScoreColls)[r][i].GetTo() << "\tsc:" << (*m_ScoreColls)[r][i].GetAttr() << endl;
        }
        cout << "============================================================" << endl;
    }
    */
}


void CScoringJob::x_WindowAvgScores(size_t window_size)
{
    typedef TScoreColl::const_pos_iterator TPosIt;

    for( size_t i = 0;  i < m_ScoreColls->size()  &&  ! IsCanceled();  i++) {
        m_NormDone = float(i) / m_ScoreColls->size();

        TScoreColl&  score_row = (*m_ScoreColls)[i];
        TScoreColl avgd_score_row;
        TPosIt itC = score_row.begin_pos();
        TPosIt itC_end = score_row.end_pos();

        // Min/Max before and after averaging to
        // renormalise values afterwards.
        TScore old_min = numeric_limits<TScore>::max();
        TScore old_max = numeric_limits<TScore>::min();
        ITERATE(TScoreColl, it_sc, score_row) {
            TScore this_score = it_sc->GetAttr();
            if (this_score < old_min) old_min = this_score;
            if (this_score > old_max) old_max = this_score;
        }
        TScore new_min = numeric_limits<TScore>::max();
        TScore new_max = numeric_limits<TScore>::min();

        // calculate our first windowed average.
        TPosIt win_begin = itC;
        TPosIt win_end = itC_end;
        if (window_size < score_row.GetLength()) {
            win_end = itC + 1 + int(window_size);
        }

        TScore win_sum = (TScore) accumulate(win_begin, win_end, (TScore)0);
        TScore win_score = win_sum / (win_end - win_begin);
        avgd_score_row.push_back(win_score);
        if (win_score < new_min) {
            new_min = win_score;
        }
        if (win_score > new_max) {
            new_max = win_score;
        }

        if(IsCanceled()) {
            return;
        }

        for(  ++itC;  itC != itC_end;  ++itC  ) {
            // Move the window & update the average.
            if ((itC - win_begin) > (int) window_size) {
                // move the beginning of the window, the trailing edge.
                win_sum -= *win_begin;
                ++win_begin;
            }
            if (win_end < itC_end) {
                // move the end of the window, the leading edge.
                win_sum += *win_end;
                ++win_end;
            }
            win_score = win_sum/(win_end - win_begin);
            avgd_score_row.push_back(win_score);
            if (win_score < new_min) new_min = win_score;
            if (win_score > new_max) new_max = win_score;
        }

        if(IsCanceled()) {
            return;
        }

        // renormalize scores back to the dynamic range they had
        // before we averaged them.
        TScoreColl norm_score_row;
        TScore norm_slope = (old_max - old_min) / (new_max - new_min);
        ITERATE(TScoreColl, it_sc, avgd_score_row) {
            norm_score_row.push_back(
                    (it_sc->GetAttr() - new_min) * norm_slope + old_min,
                    it_sc->GetLength() );
        }
        score_row.swap(norm_score_row);
    }
}


END_NCBI_SCOPE
