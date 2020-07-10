/*  $Id: sparse_multi_ds.cpp 40279 2018-01-19 17:48:49Z katargir $
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

#include <gui/widgets/aln_multiple/sparse_multi_ds.hpp>
#include <gui/widgets/aln_multiple/sparse_row_handle.hpp>
#include <gui/objutils/sparse_functions.hpp>
#include <gui/objutils/sparse_iterator.hpp>
#include <gui/utils/event_translator.hpp>


BEGIN_NCBI_SCOPE
USING_SCOPE(ncbi::objects);

///////////////////////////////////////////////////////////////////////////////
///  CSparseMultiDataSource

BEGIN_EVENT_MAP(CSparseMultiDataSource, CEventHandler)
    ON_EVENT(CAppJobNotification, CAppJobNotification::eStateChanged,
             &CSparseMultiDataSource::OnAppJobNotification)
    ON_EVENT(CAppJobNotification, CAppJobNotification::eProgress,
             &CSparseMultiDataSource::OnAppJobNotification)
END_EVENT_MAP()


CSparseMultiDataSource::CSparseMultiDataSource(objects::CScope& scope) : m_Scope(&scope)
{
}


CSparseMultiDataSource::~CSparseMultiDataSource()
{
    x_ClearHandles();
    try {
        if(x_IsJobRunning())   {
            x_DeleteJob();
        }
    }
    catch (CException& e) {
        LOG_POST(Error << "CSparseMultiDataSource::~CSparseMultiDataSource(): "
            << "failed to delete job: " << e.GetMsg());
    }

}


bool CSparseMultiDataSource::IsEmpty() const
{
    return m_Alignment == NULL;
}


void CSparseMultiDataSource::OnAppJobNotification(CEvent* evt)
{
    CAppJobNotification* notn = dynamic_cast<CAppJobNotification*>(evt);
    _ASSERT(notn);

    if(notn)    {
        int job_id = notn->GetJobID();
        if(m_JobID != job_id) {
            ERR_POST("CSparseMultiDataSource::OnAppJobNotification() - unknown Job ID " << job_id);
        } else {
            switch(notn->GetState())    {
            case IAppJob::eCompleted:   {
                CBuildSparseAlnResult* res = dynamic_cast<CBuildSparseAlnResult*>(notn->GetResult().GetPointer());
                _ASSERT(res);
                m_Alignment = res->m_SparseAln;
                x_CreateHandles();
                CUpdate up(CDataChangeNotifier::eChanged, "");
                NotifyListener(up);
                m_Job.Reset();
                break;
            }
            case IAppJob::eFailed:  {
                CConstIRef<IAppJobError> error = notn->GetError();
                CUpdate up(CDataChangeNotifier::eError, error->GetText());
                NotifyListener(up);
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



void CSparseMultiDataSource::SetGapChar(TResidue gap_char)
{
    m_Alignment->SetGapChar(gap_char);
}


void CSparseMultiDataSource::Init(CSparseAln& align)
{
    m_Alignment.Reset(&align);
    x_CreateHandles();
}


void CSparseMultiDataSource::Init(vector<CRef<CAnchoredAln> >& aligns,
                                  const CAlnUserOptions& options, 
                                  bool sync)
{
    _ASSERT( ! m_Job);
    CAppJobDispatcher& disp = CAppJobDispatcher::GetInstance();

    m_Job.Reset(new CBuildSparseAlnJob(aligns, options, *m_Scope));

    if(sync)    {
        // do everything synchronously
        m_JobID = -1;
        int type = CDataChangeNotifier::eError;

        if(m_Job->Run())    {
            type = CDataChangeNotifier::eChanged;
            CBuildSparseAlnResult* res = dynamic_cast<CBuildSparseAlnResult*>(m_Job->GetResult().GetPointer());
            if (!res) {
                NCBI_THROW(CCoreException, eNullPtr, "Cannot get merge results: AlnVec is empty");
            }
            _ASSERT(res);
            m_Alignment = res->m_SparseAln;
            x_CreateHandles();
        }

        //m_HasScores = x_TransferResults(m_Job->GetResult().GetPointer());
        m_Job.Reset();

        CUpdate up(type, "");
        NotifyListener(up);
    } else {
        // use CAppJobDispatcher to execute background jobs
        m_JobID = disp.StartJob(*m_Job, "ThreadPool", *this, 1, true);
    }
}


int CSparseMultiDataSource::GetConsensusRow() const
{
    return -1;//x_GetConsensusRow();
}


bool CSparseMultiDataSource::CanCreateConsensus()
{
    return false;
}


void CSparseMultiDataSource::CreateConsensus()
{
    _ASSERT(false); // not supported
}


IAlnExplorer::EAlignType CSparseMultiDataSource::GetAlignType() const
{
    IAlnExplorer::EAlignType type = IAlnExplorer::fInvalid;
    if( ! m_Alignment) {
        return type;
    }

    CRef<CScope> scope = m_Alignment->GetScope();
    CSparseAln::TNumrow rows = m_Alignment->GetNumRows();
    for (CSparseAln::TNumrow i = 0;  i < rows;  ++i) {
        CBioseq_Handle bsh = scope->GetBioseqHandle(m_Alignment->GetSeqId(i));
        IAlnExplorer::EAlignType this_type = IAlnExplorer::fInvalid;
        if (bsh) {
            if (bsh.IsNucleotide()) {
                this_type = IAlnExplorer::fDNA;
            } else {
                this_type = IAlnExplorer::fProtein;
            }
        }

        switch (this_type) {
        case IAlnExplorer::fInvalid:
            return this_type;

        default:
            if (type == IAlnExplorer::fInvalid) {
                type = this_type;
            } else if (type != this_type) {
                return IAlnExplorer::fMixed;
            }
        }
    }

    return type;
}


CSparseMultiDataSource::TNumrow  CSparseMultiDataSource::GetNumRows (void) const
{
    return m_Alignment->GetNumRows();
}


TSeqPos  CSparseMultiDataSource::GetAlnStart (void) const
{
    return m_Alignment->GetAlnRange().GetFrom();
}


TSeqPos  CSparseMultiDataSource::GetAlnStop (void) const
{
    return m_Alignment->GetAlnRange().GetTo();
}


TSeqPos  CSparseMultiDataSource::GetSeqStart(TNumrow row) const
{
    return m_Alignment->GetSeqStart(row);
}


TSeqPos  CSparseMultiDataSource::GetSeqStop(TNumrow row) const
{
    return m_Alignment->GetSeqStop(row);
}


bool  CSparseMultiDataSource::IsSetAnchor(void) const
{
    return m_Alignment->IsSetAnchor();
}


CSparseMultiDataSource::TNumrow  CSparseMultiDataSource::GetAnchor(void) const
{
    return m_Alignment->GetAnchor();
}


bool CSparseMultiDataSource::CanChangeAnchor(void) const
{
    return false;
}


bool CSparseMultiDataSource::SetAnchor(TNumrow /*anchor*/)
{
    _ASSERT(false); // not supported
    return false;
}


bool CSparseMultiDataSource::UnsetAnchor(void)
{
    _ASSERT(CanChangeAnchor());
    return false;
}


bool CSparseMultiDataSource::CanGetId(TNumrow /*row*/) const
{
    return true;
}


const CSeq_id&  CSparseMultiDataSource::GetSeqId(TNumrow row) const
{
    return m_Alignment->GetSeqId(row);
}


bool CSparseMultiDataSource::IsPositiveStrand(TNumrow row) const
{
    return m_Alignment->IsPositiveStrand(row);
}


bool CSparseMultiDataSource::IsNegativeStrand(TNumrow row) const
{
    return m_Alignment->IsNegativeStrand(row);
}


TSignedSeqPos CSparseMultiDataSource::GetAlnPosFromSeqPos(TNumrow row, TSeqPos aln_pos,
                                                        IAlnExplorer::ESearchDirection dir,
                                                        bool try_reverse_dir) const
{
    return m_Alignment->GetAlnPosFromSeqPos(row, aln_pos, dir, try_reverse_dir);
}


TSignedSeqPos CSparseMultiDataSource::GetSeqPosFromAlnPos(TNumrow row, TSeqPos seq_pos,
                                            IAlnExplorer::ESearchDirection dir,
                                            bool try_reverse_dir) const
{
    return m_Alignment->GetSeqPosFromAlnPos(row, seq_pos, dir, try_reverse_dir);
}


IAlnSegmentIterator*
    CSparseMultiDataSource::CreateSegmentIterator(TNumrow row,
                                        const IAlnExplorer::TSignedRange& range,
                                        IAlnSegmentIterator::EFlags flag) const
{
    return m_Alignment->CreateSegmentIterator(row, range, flag);
}


bool CSparseMultiDataSource::CanGetSeqString(TNumrow /*row*/) const
{
    return true;
}


string& CSparseMultiDataSource::GetSeqString(TNumrow row,
                                             string &buffer,
                                             const IAlnExplorer::TRange &seq_range) const
{
    m_Alignment->GetSeqString(row, buffer, seq_range);
    return buffer;
}


string& CSparseMultiDataSource::GetAlnSeqString(TNumrow row,
                                                string &buffer,
                                                const IAlnExplorer::TSignedRange& aln_range) const
{
    m_Alignment->GetAlnSeqString(row, buffer, aln_range);
    return buffer;
}


bool CSparseMultiDataSource::CanGetBioseqHandle(TNumrow row) const
{
    return true;
}


const CBioseq_Handle& CSparseMultiDataSource::GetBioseqHandle(TNumrow row) const
{
    return m_Alignment->GetBioseqHandle(row);
}


const IAlignRowHandle* CSparseMultiDataSource::GetRowHandle(TNumrow row) const
{
    return m_Handles[row];
}

TSeqPos CSparseMultiDataSource::GetBaseWidth(TNumrow row) const
{
    return m_Alignment->GetBaseWidth(row);
}


void CSparseMultiDataSource::GetAlnFromSeq(
    TNumrow row, const TRangeColl& seq_coll, TRangeColl& aln_coll
) const {
    //ERR_POST("CSparseMultiDataSource::GetAlnFromSeq() - not implemented yet");

    aln_coll.empty();

    TSeqPos seq_start = m_Alignment->GetSeqStart( row );
    TSeqPos seq_stop = m_Alignment->GetSeqStop( row );
    if( seq_start > seq_stop ){
        swap( seq_start, seq_stop );
    }

    ITERATE( TRangeColl, it, seq_coll ){
        // clip collection by alignment
        TSeqPos from = max( seq_start, it->GetFrom() );
        TSeqPos to = min( seq_stop, it->GetTo() );
        // translate
        TSeqPos aln_from = m_Alignment->GetAlnPosFromSeqPos( row, from, IAlnExplorer::eRight );
        TSeqPos aln_to = m_Alignment->GetAlnPosFromSeqPos( row, to, IAlnExplorer::eLeft );
        if( aln_from > aln_to ){
            swap( aln_from, aln_to );
        }

        aln_coll.CombineWith( TSeqRange( aln_from, aln_to ) );
    }

}


void CSparseMultiDataSource::GetSeqFromAln(TNumrow row, const TRangeColl& aln_coll,
                                           TRangeColl& seq_coll) const
{
    seq_coll.empty();

    TSeqPos aln_start = m_Alignment->GetSeqAlnStart( row );
    TSeqPos aln_stop = m_Alignment->GetSeqAlnStop( row );

    ITERATE( TRangeColl, it, aln_coll ){
        // clip collection by alignment
        TSeqPos aln_from = max( aln_start, it->GetFrom() );
        TSeqPos aln_to = min( aln_stop, it->GetTo() );

        // translate
        TSeqPos from = m_Alignment->GetSeqPosFromAlnPos( row, aln_from, IAlnExplorer::eRight );
        TSeqPos to = m_Alignment->GetSeqPosFromAlnPos(row, aln_to, IAlnExplorer::eLeft );

        if( from > to ){
            swap( from, to );
        }

        seq_coll.CombineWith( TSeqRange( from, to ) );
    }
}

void CSparseMultiDataSource::x_ClearHandles()
{
    NON_CONST_ITERATE(THandleVector, it, m_Handles) {
        delete *it;
    }
    m_Handles.clear();
}


void CSparseMultiDataSource::x_CreateHandles()
{
    TNumrow row_n = m_Alignment->GetNumRows();
    m_Handles.resize(row_n);

    for( TNumrow r = 0; r < row_n;  r++ )   {
        m_Handles[r] = new CSparseRowHandle(*m_Alignment, r);
    }
}

bool CSparseMultiDataSource::x_IsJobRunning()
{
    return m_Job.GetPointer() != NULL;
}


void CSparseMultiDataSource::x_DeleteJob()
{
    _ASSERT(m_Job  &&  m_JobID != -1);

    CAppJobDispatcher& disp = CAppJobDispatcher::GetInstance();
    disp.DeleteJob(m_JobID);

    m_JobID = -1;
    m_Job.Reset();

    //TODO update listener ?
}


void CSparseMultiDataSource::x_OnJobProgress(CAppJobNotification& notn)
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

END_NCBI_SCOPE
