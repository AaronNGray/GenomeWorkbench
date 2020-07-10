/*  $Id: alnvec_multi_ds.cpp 45017 2020-05-07 18:54:19Z shkeda $
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
 * Authors:  Andrey Yazhuk
 *
 * File Description:
 *
 */

#include <ncbi_pch.hpp>

#include <gui/widgets/aln_multiple/alnvec_multi_ds.hpp>

#include <gui/objutils/utils.hpp>
#include <gui/utils/event_translator.hpp>

#include <objects/seqloc/Seq_loc.hpp>
#include <objects/seqloc/Seq_interval.hpp>
#include <objects/seq/Bioseq.hpp>

#include <objmgr/align_ci.hpp>




BEGIN_NCBI_SCOPE
USING_SCOPE(ncbi::objects);


CAlnVecMultiDataSource::CAlnVecMultiDataSource(CScope& scope)
:   m_Scope(&scope), 
    m_ConsRowIndex(-1),
    m_CreateConsensus(false),
    m_isDataReadSync(false)
{
}


CAlnVecMultiDataSource::~CAlnVecMultiDataSource()
{
    x_ClearHandles();

    try {
        if(x_IsJobRunning())   {
            x_DeleteJob();
        }
    }
    catch (CException& e) {
        LOG_POST(Error << "CAlnVecMultiDataSource::~CAlnVecMultiDataSource(): "
            << "failed to delete job: " << e.GetMsg());
    }
}


BEGIN_EVENT_MAP(CAlnVecMultiDataSource, CEventHandler)
    ON_EVENT(CAppJobNotification, CAppJobNotification::eStateChanged,
             &CAlnVecMultiDataSource::OnAppJobNotification)
    ON_EVENT(CAppJobNotification, CAppJobNotification::eProgress,
             &CAlnVecMultiDataSource::OnAppJobNotification)
END_EVENT_MAP()


int CAlnVecMultiDataSource::GetConsensusRow() const
{
    return m_ConsRowIndex;
}


void CAlnVecMultiDataSource::Init(const objects::CSeq_annot& annot, bool sync, bool select_anchor)
{
    vector< CConstRef<CSeq_align> > aligns;

    if(annot.GetData().IsAlign() ) {
        ITERATE (CSeq_annot::TData::TAlign, it, annot.GetData().GetAlign()) {
            aligns.emplace_back(*it);
        }
    }
    Init(aligns, sync, select_anchor);
}


void CAlnVecMultiDataSource::Init(const objects::CBioseq_Handle& handle, bool sync, bool select_anchor)
{
    vector< CConstRef<CSeq_align> > aligns;

    SAnnotSelector sel =
        CSeqUtils::GetAnnotSelector(CSeq_annot::TData::e_Align);
    CAlign_CI it(handle, sel);
    for ( ;  it;  ++it) {
        aligns.emplace_back(&*it);
    }
    Init(aligns, sync, select_anchor);
}


void CAlnVecMultiDataSource::Init(const vector< CConstRef<CSeq_align> >& aligns,
                                  bool sync, bool select_anchor)
{
    _ASSERT( ! m_Job);
    CAppJobDispatcher& disp = CAppJobDispatcher::GetInstance();

    m_Job.Reset(new CBuildAlnVecJob(aligns, *m_Scope, select_anchor));
    m_isDataReadSync = sync;

    if(sync)    {
        // do everything synchronously
        disp.RunSync(*m_Job, m_JobID, *this);
        m_JobID = -1;
        /*
        int type = CDataChangeNotifier::eError;

        if(m_Job->Run())    {
            type = CDataChangeNotifier::eChanged;
            CBuildAlnVecResult* res = dynamic_cast<CBuildAlnVecResult*>(m_Job->GetResult().GetPointer());
            if (!res) {
                NCBI_THROW(CCoreException, eNullPtr, "Cannot get merge results: AlnVec is empty");
            }
            _ASSERT(res);
            x_Assign(*res->m_AlnVec);
        }

        //m_HasScores = x_TransferResults(m_Job->GetResult().GetPointer());
        m_Job.Reset();

        CUpdate up(type, "");
        NotifyListener(up);
        */
    } else {
        // use CAppJobDispatcher to execute background jobs
        m_JobID = disp.StartJob(*m_Job, "ThreadPool", *this, 1, true);
    }
}


bool CAlnVecMultiDataSource::IsEmpty() const
{
    return m_AlnVec.GetPointer() == NULL;
}


void CAlnVecMultiDataSource::OnAppJobNotification(CEvent* evt)
{
    CAppJobNotification* notn = dynamic_cast<CAppJobNotification*>(evt);
    _ASSERT(notn);

    if(notn)    {
        int job_id = notn->GetJobID();
        if(m_JobID != job_id) {
            ERR_POST("CAlnVecMultiDataSource::OnAppJobNotification() - unknown Job ID " << job_id);
        } else {
            switch(notn->GetState())    {
            case IAppJob::eCompleted:   {
                CBuildAlnVecResult* res = dynamic_cast<CBuildAlnVecResult*>(notn->GetResult().GetPointer());
                _ASSERT(res);
                x_Assign(*res->m_AlnVec);
                CUpdate up(CDataChangeNotifier::eChanged, "");
                NotifyListener(up);
                m_Job.Reset();
                break;
            }
            case IAppJob::eFailed:  {
                CConstIRef<IAppJobError> error = notn->GetError();
                CUpdate up(CDataChangeNotifier::eError, error->GetText());
                NotifyListener(up);
                break;
            }
            case IAppJob::eCanceled:    {
                CUpdate up(CDataChangeNotifier::eError, "Canceled");
                NotifyListener(up);
                break;
            }
            case IAppJob::eRunning: {
                CConstIRef<IAppJobProgress> progressSoFar = notn->GetProgress();
                if (progressSoFar) {
                    CUpdate up( CDataChangeNotifier::eProgressReport,
                        progressSoFar->GetText(), progressSoFar->GetNormDone());
                    NotifyListener(up);
                }
                break;
            }
            default:
                _ASSERT(false);
            }
        }
    }
}


void CAlnVecMultiDataSource::x_Assign(objects::CAlnVec& aln_vec)
{
    x_ClearHandles();

    m_AlnVec.Reset(&aln_vec);
    SetGapChar('-');

    x_CreateHandles();
    for (const auto& row : m_Handles) {
        if (!row->IsConsensus())
            continue;
        m_ConsRowIndex = row->GetRowNum();
        m_CreateConsensus = false;
        break;
    }
    CreateConsensus();
}


bool CAlnVecMultiDataSource::x_IsJobRunning()
{
    return m_Job.GetPointer() != NULL;
}


void CAlnVecMultiDataSource::x_DeleteJob()
{
// This assertion is invalid, see Init with sync == true
//    _ASSERT(m_Job  &&  m_JobID != -1);
    _ASSERT(m_Job);

    if (m_JobID != -1) {
        CAppJobDispatcher& disp = CAppJobDispatcher::GetInstance();
        disp.DeleteJob(m_JobID);

        m_JobID = -1;
    }
    m_Job.Reset();

    //TODO update listener ?
}


void CAlnVecMultiDataSource::x_OnJobProgress(CAppJobNotification& notn)
{
    CConstIRef<IAppJobProgress> prg = notn.GetProgress();
    if(prg) {
        string text = prg->GetText();

        if(x_IsJobRunning())    {
            CUpdate up(eProgressReport, text);
            NotifyListener(up);
        }
    }
}


void CAlnVecMultiDataSource::SetGapChar(TResidue gap_char)
{
    m_AlnVec->SetGapChar(gap_char);
}

IAlnExplorer::EAlignType CAlnVecMultiDataSource::GetAlignType() const
{
    if( ! m_AlnVec) {
        return IAlnExplorer::fInvalid;
    }
    if(m_AlnVec->GetDenseg().IsSetWidths())   {
        return IAlnExplorer::fMixed;
    } else {
        try {
            const CBioseq& bioseq = *GetBioseqHandle(0).GetBioseqCore();
            if (bioseq.GetInst().GetMol() == CSeq_inst::eMol_aa) {
                return IAlnExplorer::fProtein;
            } else {
                return IAlnExplorer::fDNA;
            }
        } catch (exception& e) {
            ERR_POST(Error << e.what());
            return IAlnExplorer::fInvalid;
        }
    }
}


CAlnVecMultiDataSource::TNumrow  CAlnVecMultiDataSource::GetNumRows (void) const
{
    return (TNumrow) m_AlnVec->GetNumRows();
}


TSeqPos  CAlnVecMultiDataSource::GetAlnStart (void) const
{
    return m_AlnVec->GetAlnStart();
}


TSeqPos  CAlnVecMultiDataSource::GetAlnStop (void) const
{
    return m_AlnVec->GetAlnStop();
}


TSeqPos  CAlnVecMultiDataSource::GetSeqStart(TNumrow row) const
{
    return m_AlnVec->GetSeqStart(row);
}


TSeqPos  CAlnVecMultiDataSource::GetSeqStop(TNumrow row) const
{
    return m_AlnVec->GetSeqStop(row);
}


bool  CAlnVecMultiDataSource::IsSetAnchor (void) const
{
    return m_AlnVec->IsSetAnchor();
}


CAlnVecMultiDataSource::TNumrow CAlnVecMultiDataSource::GetAnchor(void) const
{
    return m_AlnVec->GetAnchor();
}


bool CAlnVecMultiDataSource::CanChangeAnchor(void) const
{
    return true;
}


bool CAlnVecMultiDataSource::SetAnchor(TNumrow anchor)
{
    m_AlnVec->SetAnchor(anchor);
    return true;
}


bool CAlnVecMultiDataSource::UnsetAnchor (void)
{
    m_AlnVec->UnsetAnchor();
    return true;
}


bool CAlnVecMultiDataSource::CanGetId(TNumrow row) const
{
    return (row >= 0  &&  row < m_AlnVec->GetNumRows());
}


const CSeq_id&  CAlnVecMultiDataSource::GetSeqId(TNumrow row) const
{
    return m_AlnVec->GetSeqId(row);
}


const CBioseq_Handle &  CAlnVecMultiDataSource::GetBioseqHandle(TNumrow row) const
{
    return m_AlnVec->GetBioseqHandle(row);
}

TSeqPos CAlnVecMultiDataSource::GetBaseWidth(TNumrow row) const
{
    return m_AlnVec->GetWidth(row);
}

bool CAlnVecMultiDataSource::IsPositiveStrand(TNumrow row) const
{
    return m_AlnVec->IsPositiveStrand(row);
}


bool CAlnVecMultiDataSource::IsNegativeStrand(TNumrow row) const
{
    return m_AlnVec->IsNegativeStrand(row);
}


TSignedSeqPos  CAlnVecMultiDataSource::GetAlnPosFromSeqPos(TNumrow for_row, TSeqPos aln_pos,
                                                        IAlnExplorer::ESearchDirection dir,
                                                        bool try_reverse_dir) const
{
    return m_AlnVec->GetAlnPosFromSeqPos(for_row, aln_pos,
                (CAlnVec::ESearchDirection) dir, try_reverse_dir);
}


TSignedSeqPos  CAlnVecMultiDataSource::GetSeqPosFromAlnPos(TNumrow row, TSeqPos seq_pos,
                                            IAlnExplorer::ESearchDirection dir,
                                            bool try_reverse_dir) const
{
    return m_AlnVec->GetSeqPosFromAlnPos(row, seq_pos, (CAlnVec::ESearchDirection) dir, try_reverse_dir);
}



string& CAlnVecMultiDataSource::GetAlnSeqString(TNumrow row, string &buffer,
                                                const IAlnExplorer::TSignedRange& aln_range) const
{
    return m_AlnVec->GetAlnSeqString(buffer, row, aln_range);
}


void  CAlnVecMultiDataSource::GetAlnFromSeq(TNumrow row,
                                            const TRangeColl& seq_coll,
                                            TRangeColl& aln_coll) const
{
    aln_coll.empty();

    TSeqPos seq_start = m_AlnVec->GetSeqStart(row);
    TSeqPos seq_stop = m_AlnVec->GetSeqStop(row);
    if(seq_start > seq_stop)    {
        swap(seq_start, seq_stop);
    }

    ITERATE(TRangeColl, it, seq_coll)  {
        // clip collection by alignment
        TSeqPos from = max(seq_start, it->GetFrom());
        TSeqPos to = min(seq_stop, it->GetTo());
        // translate
        TSeqPos aln_from = m_AlnVec->GetAlnPosFromSeqPos(row, from, CAlnVec::eRight);
        TSeqPos aln_to = m_AlnVec->GetAlnPosFromSeqPos(row, to, CAlnVec::eLeft);
        if(aln_from > aln_to)    {
            swap(aln_from, aln_to);
        }
        aln_coll.CombineWith(TSeqRange(aln_from, aln_to));
    }
}


void  CAlnVecMultiDataSource::GetSeqFromAln(TNumrow row,
                                            const TRangeColl& aln_coll,
                                            TRangeColl& seq_coll) const
{
    seq_coll.empty();

    TSeqPos aln_start = m_AlnVec->GetSeqAlnStart(row);
    TSeqPos aln_stop = m_AlnVec->GetSeqAlnStop(row);

    ITERATE(TRangeColl, it, aln_coll)  {
        // clip collection by alignment
        TSeqPos aln_from = max(aln_start, it->GetFrom());
        TSeqPos aln_to = min(aln_stop, it->GetTo());

        // translate
        TSeqPos from = m_AlnVec->GetSeqPosFromAlnPos(row, aln_from, CAlnVec::eRight);
        TSeqPos to = m_AlnVec->GetSeqPosFromAlnPos(row, aln_to, CAlnVec::eLeft);

        if(from > to)    {
            swap(from, to);
        }
        seq_coll.CombineWith(TSeqRange(from, to));
    }
}


const IAlignRowHandle*  CAlnVecMultiDataSource::GetRowHandle(TNumrow row) const
{
    return static_cast<const IAlignRowHandle*>(m_Handles[row]);
}


IAlnSegmentIterator*
    CAlnVecMultiDataSource::CreateSegmentIterator(TNumrow row, const IAlnExplorer::TSignedRange& range,
                                     IAlnSegmentIterator::EFlags flags) const
{
    const IAlignRowHandle* handle = GetRowHandle(row);
    return handle->CreateSegmentIterator(range, flags);
}


void CAlnVecMultiDataSource::x_ClearHandles()
{
    NON_CONST_ITERATE(THandleVector, it, m_Handles) {
        delete *it;
    }
    m_Handles.clear();
}


void CAlnVecMultiDataSource::x_CreateHandles()
{
    TNumrow row_n = m_AlnVec->GetNumRows();
    m_Handles.resize(row_n);
    for( TNumrow r = 0; r < row_n;  r++ )   {
        m_Handles[r] = new CAlnVecRowHandle(m_AlnVec, r);
    }
}


bool CAlnVecMultiDataSource::CanCreateConsensus()
{
    return true;
}

bool s_IsGap(char ch)
{
    return  ch == '-';
}

void CollectNucleotideFrequences(const vector<string>& rows, int col, int base_count[], int numBases)
{
    // first, we record which bases occur and how often
    // this is computed in NCBI4na notation
    fill_n(base_count, numBases, 0);

    unsigned char c;
    for (size_t i = 0; i < rows.size(); ++i) {
        if (col >= rows[i].size())
            continue;
        c = rows[i][col];
        switch (c) {
        case 'A':
            ++base_count[0];
            break;
        case 'C':
            ++base_count[1];
            break;
        case 'M':
            ++base_count[1];
            ++base_count[0];
            break;
        case 'G':
            ++base_count[2];
            break;
        case'R':
            ++base_count[2];
            ++base_count[0];
            break;
        case 'S':
            ++base_count[2];
            ++base_count[1];
            break;
        case 'V':
            ++base_count[2];
            ++base_count[1];
            ++base_count[0];
            break;
        case 'T':
            ++base_count[3];
            break;
        case 'W':
            ++base_count[3];
            ++base_count[0];
            break;
        case 'Y':
            ++base_count[3];
            ++base_count[1];
            break;
        case 'H':
            ++base_count[3];
            ++base_count[1];
            ++base_count[0];
            break;
        case 'K':
            ++base_count[3];
            ++base_count[2];
            break;
        case 'D':
            ++base_count[3];
            ++base_count[2];
            ++base_count[0];
            break;
        case 'B':
            ++base_count[3];
            ++base_count[2];
            ++base_count[1];
            break;
        case 'N':
            ++base_count[3];
            ++base_count[2];
            ++base_count[1];
            ++base_count[0];
            break;
        default:
            break;
        }
    }
}

//void CollectProteinFrequences(const string& col, int base_count[], int numBases)
void CollectProteinFrequences(const vector<string>& rows, int col, int base_count[], int numBases)
{
    // first, we record which bases occur and how often
    // this is computed in NCBI4na notation
    fill_n(base_count, numBases, 0);

    char c;
    for (size_t i = 0; i < rows.size(); ++i) {
        if (rows[i].empty())
            continue;
        c = rows[i][col];
        int pos = c - 'A';
        if (0 <= pos && pos < numBases)
            ++base_count[pos];
    }
}

void CAlnVecMultiDataSource::CreateConsensus(vector<string>& consens) const
{

    bool isNucleotide = m_AlnVec->GetBioseqHandle(0).IsNucleotide();

    const int numBases = isNucleotide ? 4 : 26;
    const int num_rows = m_AlnVec->GetNumRows();
    const int num_segs = m_AlnVec->GetNumSegs();

    int base_count[26]; // must be a compile-time constant for some compilers

    // determine what the number of segments required for a gapped consensus
    // segment is.  this must be rounded to be at least 50%.
    int gap_seg_thresh = num_rows - num_rows / 2;

    for (size_t j = 0; j < (size_t)num_segs; ++j) {
        // evaluate for gap / no gap
        int gap_count = 0;
        size_t seg_len = m_AlnVec->GetLen(j, 0);
        for (size_t i = 0; i < (size_t)num_rows; ++i) {
            if (m_AlnVec->GetStart(i, j, 0) == -1) {
                //auto seg_type = m_AlnVec->GetSegType(i, j, 0);
                //if (seg_type != 0)
                    ++gap_count;
            }
        }

        // check to make sure that this seg is not a consensus
        // gap seg
        if (gap_count > gap_seg_thresh) {
            consens[j].resize(seg_len);
            for (size_t i = 0; i < seg_len; ++i)
                consens[j][i] = m_AlnVec->GetGapChar(0);
            continue;
        }
        

        // the base threshold for being considered unique is at least
        // 70% of the available sequences
        int base_thresh =
            ((num_rows - gap_count) * 7 + 5) / 10;

        {
            // we will build a segment with enough bases to match
            consens[j].resize(seg_len);

            // retrieve all sequences for this segment
            vector<string> segs(num_rows);
            m_AlnVec->RetrieveSegmentSequences(j, segs);

            typedef multimap<int, unsigned char, greater<int> > TRevMap;            
            // 
            // evaluate for a consensus
            //   
            for (size_t i = 0; i < seg_len; ++i) {
                if (isNucleotide) {                   
                    CollectNucleotideFrequences(segs, i, base_count, numBases);
                }
                else {                    
                    CollectProteinFrequences(segs, i, base_count, numBases);
                }               


                // we create a sorted list (in descending order) of
                // frequencies of appearance to base
                // the frequency is "global" for this position: that is,
                // if 40% of the sequences are gapped, the highest frequency
                // any base can have is 0.6
                TRevMap rev_map;               

                for (int k = 0; k < numBases; ++k) {
                    // this gets around a potentially tricky idiosyncrasy
                    // in some implementations of multimap.  depending on
                    // the library, the key may be const (or not)
                    TRevMap::value_type p(base_count[k], isNucleotide ? (1 << k) : k);
                    rev_map.insert(p);
                }

                // now, the first element here contains the best frequency
                // we scan for the appropriate bases
                if (rev_map.count(rev_map.begin()->first) == 1 &&
                    rev_map.begin()->first >= base_thresh) {
                    consens[j][i] = isNucleotide ?
                        m_AlnVec->ToIupac(rev_map.begin()->second) :
                        (rev_map.begin()->second + 'A');
                }
                else {
                    // now we need to make some guesses based on IUPACna
                    // notation
                    int               count;
                    unsigned char     c = 0x00;
                    int               freq = 0;
                    TRevMap::iterator curr = rev_map.begin();
                    TRevMap::iterator prev = rev_map.begin();
                    for (count = 0;
                        curr != rev_map.end() &&
                        (freq < base_thresh || prev->first == curr->first);
                    ++curr, ++count) {
                        prev = curr;
                        freq += curr->first;
                        if (isNucleotide) {
                            c |= curr->second;
                        }
                        else {
                            unsigned char cur_char = curr->second + 'A';
                            switch (c) {
                            case 0x00:
                                c = cur_char;
                                break;
                            case 'N': case 'D':
                                c = (cur_char == 'N' || cur_char == 'D') ? 'B' : 'X';
                                break;
                            case 'Q': case 'E':
                                c = (cur_char == 'Q' || cur_char == 'E') ? 'Z' : 'X';
                                break;
                            case 'I': case 'L':
                                c = (cur_char == 'I' || cur_char == 'L') ? 'J' : 'X';
                                break;
                            default:
                                c = 'X';
                            }
                        }
                    }

                    //
                    // catchall
                    //                    
                    if (count > 2) {
                        consens[j][i] = isNucleotide ? 'N' : 'X';
                    }
                    else {
                        consens[j][i] = isNucleotide ? m_AlnVec->ToIupac(c) : c;
                    }
                }
            }
        }
    }
}

/*
// Create consensus using CAlnVec function
void CAlnVecMultiDataSource::CreateConsensus()
{
    if (m_CreateConsensus) {
        if (m_AlnVec.NotEmpty() && m_ConsRowIndex == -1)    {
            x_ClearHandles();

            CRef<CDense_seg> ds = m_AlnVec->CreateConsensus(m_ConsRowIndex);
            m_AlnVec.Reset(new CAlnVec(*ds, m_AlnVec->GetScope()));

            x_CreateHandles();
        }
    }
}
*/

// Create alignment using local (CAlnVecMultiDataSource) function
void CAlnVecMultiDataSource::CreateConsensus()
{
    if (m_CreateConsensus) {
        if (m_AlnVec.NotEmpty() && m_ConsRowIndex == -1)    {
            x_ClearHandles();
            
            unique_ptr<vector<string>> consens(new vector<string>);
            consens->resize(m_AlnVec->GetNumSegs());
            CreateConsensus(*consens);

            CSeq_id consensus_id("lcl|consensus");
            CRef<CBioseq> bioseq(new CBioseq);
            CRef<CDense_seg> ds = m_AlnVec->CreateConsensus(m_ConsRowIndex,
                *bioseq, consensus_id, consens.get());

            // add bioseq to the scope
            CRef<CSeq_entry> entry(new CSeq_entry());
            entry->SetSeq(*bioseq);
            m_AlnVec->GetScope().AddTopLevelSeqEntry(*entry);

            m_AlnVec.Reset(new CAlnVec(*ds, m_AlnVec->GetScope()));
            x_CreateHandles();
        }
    }
}

static void s_TranslateAndExpand(string& seq, int gen_code)
{
    if (seq.empty())
        return;
    string new_seq;
    CAlnVec::TranslateNAToAA(seq, new_seq);
    auto seq_len = new_seq.size();
    seq.resize(seq_len * 3);
   
    for (size_t i = 0; i < seq_len; ++i) {
        size_t n_i = 3 * i;
        seq[n_i] = seq[n_i + 1] = seq[n_i + 2] = new_seq[i];
    }
}

#ifdef _SHOW_CONSENSUS_IN_PANORAMA_

static set<char> ss_NonAmbiguousAA = {
    'A','C','D','E','F',
    'G','H','I','K','L',
    'M','N','O','P','Q',
    'R','S','T','U','V',
    'W','Y','X', ' ', '-'
};

static set<char> ss_NonAmbiguousDNA = {
    'A','C','G','T', 'N', ' ', '-'
};
#endif

#define ADD_BASE_TO_GRAPH(b) \
 { if (graphs[b].size() == 0) graphs[b].resize(ref_len, 0); graphs[b][curr_pos] += 1; }


void IAlnMultiDataSource::CollectAlignStats(const TSignedSeqRange& aln_range, TStatGraphs& graphs, bool translate_sequence) const
{
    _ASSERT(graphs.empty());
    graphs.clear();
    auto align_type = GetAlignType();

    if (align_type == IAlnExplorer::fMixed)
        NCBI_THROW(CException, eUnknown, "Alignment of mixed types are not supported");
    if (GetNumRows() == 0)
        return;
    char ambiguous_residue = ' ';    
    if (align_type == IAlnExplorer::EAlignType::fDNA)
        ambiguous_residue = 'N';
    else if (align_type == IAlnExplorer::EAlignType::fProtein) 
        ambiguous_residue = 'X';

    // initialize the top sequence to generate statistics for every base
    string ref_str;
    int anchor = -1;
    auto consensus_idx = GetConsensusRow();
    if (IsSetAnchor()) {
        anchor = GetAnchor();
        GetAlnSeqString(anchor, ref_str, aln_range);
        if (translate_sequence)
            s_TranslateAndExpand(ref_str, GetGenCode(anchor));
    }
    bool ref_is_consensus = anchor != -1 && anchor == consensus_idx;
    
#ifdef _SHOW_CONSENSUS_IN_PANORAMA_
    const set<char>* non_ambiguous_set = nullptr;
    if (consensus_idx >= 0) {
        if (translate_sequence || GetAlignType() == IAlnExplorer::fProtein)
            non_ambiguous_set = &ss_NonAmbiguousAA;
        else
            non_ambiguous_set = &ss_NonAmbiguousDNA;
    }
#endif

    TSeqPos ref_len = aln_range.GetLength();
    for (TNumrow row = 0; row < GetNumRows(); ++row) {
        if (row == consensus_idx) // skip consensus
            continue;

        auto_ptr<IAlnSegmentIterator> p_it
            (CreateSegmentIterator(row, aln_range, IAlnSegmentIterator::eAllSegments));

        for (IAlnSegmentIterator& it = *p_it; it; ++it) {
            const IAlnSegment& seg = *it;
            
            if ((seg.GetType() & IAlnSegment::fIndel  &&
                 !seg.GetRange().Empty()) ||
                //                seg.GetType() & IAlnSegment::fUnaligned ||
                seg.GetType() & IAlnSegment::fGap) {
                // ignore the inserts
                continue;
            }
            //            auto row_str = GetSeqId(row).GetSeqIdString();
            const IAlnSegment::TSignedRange& curr_aln_r = seg.GetAlnRange();
            TSeqPos seg_len = (TSeqPos)curr_aln_r.GetLength();
            if (seg_len == 0)
                continue;
            
            TSignedSeqPos curr_aln_start = curr_aln_r.GetFrom();
            
            TSeqPos off = 0;
            size_t pos = 0;
            if (curr_aln_start < aln_range.GetFrom()) {
                pos = aln_range.GetFrom() - curr_aln_start;
            } else {
                off = curr_aln_start - aln_range.GetFrom();
            }
            size_t curr_pos = off;
            string aln_seq;
            if (seg.GetType() & IAlnSegment::fAligned) {
                GetAlnSeqString(row, aln_seq, curr_aln_r);
                if (translate_sequence)
                    s_TranslateAndExpand(aln_seq, GetGenCode(row));
            }
            
            while (pos < seg_len && curr_pos < ref_len) {
                if (seg.GetType() & IAlnSegment::fAligned && pos < aln_seq.size()) {
                    char base = aln_seq[pos];
                    if (s_IsGap(base)) {
                        base = '-';
                    } else if (anchor >=0 && curr_pos < ref_len) {
#ifdef _SHOW_CONSENSUS_IN_PANORAMA_
                        
                        if (non_ambiguous_set && non_ambiguous_set->count(ref_str[curr_pos]) == 0) 
                            ADD_BASE_TO_GRAPH('w')
                            else
#endif                  
                        if (base != ref_str[curr_pos] && base != ambiguous_residue) {
                            if (ref_str[curr_pos] != ambiguous_residue || ref_is_consensus) {
                                ADD_BASE_TO_GRAPH('m')
                            }
                        }
                              
                        /*            
                        if (base != ref_str[curr_pos] 
                            && base != ambiguous_residue 
                            && ref_str[curr_pos] != ambiguous_residue) 
                            ADD_BASE_TO_GRAPH('m')
                        */    
                    }
                    ADD_BASE_TO_GRAPH(base)
                    
                } else {
                    ADD_BASE_TO_GRAPH('-')
                }
                ++pos;
                ++curr_pos;
            }
        }
    }
}


END_NCBI_SCOPE
