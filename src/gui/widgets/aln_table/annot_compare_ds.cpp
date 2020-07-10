/*  $Id: annot_compare_ds.cpp 40280 2018-01-19 17:54:35Z katargir $
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
#include <gui/widgets/aln_table/annot_compare_ds.hpp>
#include <gui/objutils/utils.hpp>
#include <gui/objutils/label.hpp>
//#include <gui/widgets/fl/text_report_dlg.hpp>

#include <objects/seqalign/Seq_align.hpp>
#include <objects/seqalign/Seq_align_set.hpp>
#include <objmgr/seq_loc_mapper.hpp>
#include <objmgr/util/sequence.hpp>
#include <objmgr/seq_vector.hpp>
#include <objmgr/feat_ci.hpp>
#include <serial/iterator.hpp>

#include <gui/utils/view_event.hpp>
#include <gui/utils/app_job.hpp>
#include <gui/utils/app_job_impl.hpp>
#include <gui/utils/event_translator.hpp>

#include <corelib/ncbi_system.hpp>
#include <algo/sequence/annot_compare.hpp>
#include <objtools/alnmgr/alnmix.hpp>


BEGIN_NCBI_SCOPE
USING_SCOPE(objects);

/////////////////////////////////////////////////////////////////////////////
///
/// AppJobs for background loading and preparation
///

class CFeatCompare_Result : public CObject
{
public:
    /// row represents a pair of compared features
    CAnnotCompareDS::TRows m_Rows;
};


/////////////////////////////////////////////////////////////////////////////


CAnnotCompareDS::CAnnotCompareDS(CScope& scope)
    : m_Scope(&scope)
    , m_ActiveJob(-1)
    , m_Listener(NULL)
{
}


/// return progress indicator
float CAnnotCompareDS::GetProgress() const
{
    CAppJobDispatcher& disp = CAppJobDispatcher::GetInstance();
    CConstIRef<IAppJobProgress> ref = disp.GetJobProgress(m_ActiveJob);
    return ref->GetNormDone();
}

void CAnnotCompareDS::Clear()
{
    x_DeleteAllJobs();
    m_Rows.clear();
}


void CAnnotCompareDS::RegisterListener(CEventHandler* listener)
{
    m_Listener = listener;
}


void CAnnotCompareDS::DumpTextReport(CNcbiOstream& ostr) const
{
    /// keep sets of statistics

    /// count of identical locations
    size_t identical_locs = 0;

    /// count of identical sequences (genomic)
    size_t identical_seqs = 0;

    /// count of identical products
    size_t identical_prods = 0;

    /// count of identical loc + seq
    size_t identical_locs_seqs = 0;

    /// count of identical loc + prod
    size_t identical_locs_prods = 0;

    /// count of identical seq + prod
    size_t identical_seqs_prods = 0;

    /// count of identical loc + seq + prod
    size_t identical_locs_seqs_prods = 0;

    /// counts of feature subtypes with match counts
    typedef map<CAnnotCompare::TCompareFlags, size_t> TCountMap;
    typedef map<string, TCountMap> TFeatCountMap;
    TFeatCountMap feat_counts;

    typedef map<CSeq_id_Handle, TFeatCountMap> TCountNotFound;
    TCountNotFound count_not_found;

    /// first set: comparison
    ITERATE (TRows, iter, m_Rows) {

        /// register counts per feature type
        string subtype;
        if (iter->feat1) {
            subtype = iter->feat1->GetData().GetKey();
        } else if (iter->feat2) {
            subtype = iter->feat2->GetData().GetKey();
        }

        ///
        /// keep a count per subtype of the features present
        ///
        {{
            TFeatCountMap::iterator it =
                feat_counts.insert(make_pair(subtype, TCountMap())).first;
            TCountMap::value_type elt(iter->loc_state, 0);
            TCountMap::iterator count_it = it->second.insert(elt).first;
            ++count_it->second;
        }}

        CAnnotCompare::TCompareFlags seq_match = iter->loc_state & CAnnotCompare::eSequenceMask;
        if (seq_match == (CAnnotCompare::eSequence_SameSeq | CAnnotCompare::eSequence_SameProduct)) {
            ++identical_seqs_prods;
        }
        if (seq_match & CAnnotCompare::eSequence_SameSeq) {
            ++identical_seqs;
        }
        if (seq_match & CAnnotCompare::eSequence_SameProduct) {
            ++identical_prods;
        }

        CAnnotCompare::TCompareFlags loc_match = iter->loc_state & CAnnotCompare::eLocationMask;
        if (loc_match == CAnnotCompare::eLocation_Same) {
            ++identical_locs;

            if (seq_match == (CAnnotCompare::eSequence_SameSeq | CAnnotCompare::eSequence_SameProduct)) {
                ++identical_locs_seqs_prods;
            }
            if (seq_match & CAnnotCompare::eSequence_SameSeq) {
                ++identical_locs_seqs;
            }
            if (seq_match & CAnnotCompare::eSequence_SameProduct) {
                ++identical_locs_prods;
            }
        }
        if (loc_match == CAnnotCompare::eLocation_Missing) {
            const CSeq_id_Handle& h =
                iter->feat1 ? iter->feat1_seq : iter->feat2_seq;
            const CSeq_feat& feat =
                iter->feat1 ? *iter->feat1 : *iter->feat2;

            _ASSERT(h  &&  h.GetSeqId());

            TCountNotFound::value_type v1(h, TFeatCountMap());
            TCountNotFound::iterator i1 = count_not_found.insert(v1).first;

            TFeatCountMap::value_type v2(feat.GetData().GetKey(), TCountMap());
            TFeatCountMap::iterator i2 = i1->second.insert(v2).first;

            TCountMap::value_type v3(CAnnotCompare::eLocation_Missing, 0);
            TCountMap::iterator i3 = i2->second.insert(v3).first;
            ++i3->second;
        }
    }

    ostr << "Evaluated " << m_Rows.size() << " total comparisons.\n\n";

    ostr << setw(10) << identical_locs <<  "  Identical location\n";
    ostr << setw(10) << identical_seqs <<  "  Identical sequence\n";
    ostr << setw(10) << identical_prods << "  Identical product\n";
    ostr << setw(10) << identical_locs_seqs << "  Identical location + sequence\n";
    ostr << setw(10) << identical_locs_prods << "  Identical location + product\n";
    ostr << setw(10) << identical_seqs_prods << "  Identical sequence + product\n";
    ostr << setw(10) << identical_locs_seqs_prods << "  Identical location + sequence + product\n";

    /// note features not found
    ostr << "\nFeatures Not Found:\n";
    ostr <<   "-------------------\n";
    ITERATE (TCountNotFound, iter, count_not_found) {
        string s;
        CLabel::GetLabel(*iter->first.GetSeqId(), &s, CLabel::eDefault, m_Scope);

        size_t total_feats = 0;
        ITERATE (TFeatCountMap, it, iter->second) {
            ITERATE (TCountMap, i, it->second) {
                total_feats += i->second;
            }
        }
        ostr << setw(10) << total_feats << "  Not found on " << s << "\n";
        ITERATE (TFeatCountMap, it, iter->second) {
            size_t count = 0;
            ITERATE (TCountMap, i, it->second) {
                count += i->second;
            }
            ostr << setw(14) << count << "  " << it->first;
            if (count != 1) {
                ostr << "s";
            }
            ostr << " not found\n";
        }
        ostr << "\n";
    }

    /// note counts per feature subtype, with emphasis on
    /// particular feature subtypes
    ostr << "\nFeature Comparisons by Feature Type:\n";
    ostr <<   "------------------------------------\n";
    ITERATE (TFeatCountMap, iter, feat_counts) {
        /// first, dump a stat on total comparisons for this feature type
        size_t total_feats = 0;
        ITERATE (TCountMap, it, iter->second) {
            total_feats += it->second;
        }

        ostr << setw(10) << total_feats << "  " << iter->first;
        if (total_feats != 1) {
            ostr << "s";
        }
        ostr << "\n";

        ITERATE (TCountMap, it, iter->second) {
            ostr << setw(14) << it->second << "  ";

            /**
            ostr << iter->first;
            if (it->second != 1) {
                ostr << "s";
            }
            ostr << " ";
            **/

            CAnnotCompare::TCompareFlags loc_state =
                it->first & CAnnotCompare::eLocationMask;
            CAnnotCompare::TCompareFlags seq_state =
                it->first & CAnnotCompare::eSequenceMask;
            switch (loc_state) {
            case CAnnotCompare::eLocation_Missing:
                ostr << "not found";
                break;
            case CAnnotCompare::eLocation_Same:
                ostr << "identical location";
                break;
            case CAnnotCompare::eLocation_MissingExon:
                ostr << "missing exons";
                break;
            case CAnnotCompare::eLocation_5PrimeExtension:
                ostr << "5' extension";
                break;
            case CAnnotCompare::eLocation_3PrimeExtension:
                ostr << "3' extension";
                break;
            case CAnnotCompare::eLocation_5PrimeExtraExon:
                ostr << "5' extra exons";
                break;
            case CAnnotCompare::eLocation_3PrimeExtraExon:
                ostr << "3' extra exons";
                break;
            case CAnnotCompare::eLocation_Overlap:
                ostr << "overlap, shared intervals";
                break;
            case CAnnotCompare::eLocation_Complex:
                ostr << "complex location comparison";
                break;
            case CAnnotCompare::eLocation_Subset:
                ostr << "subset";
                break;
            case CAnnotCompare::eLocation_RegionOverlap:
                ostr << "overlap, no shared intervals";
                break;
            case CAnnotCompare::eLocation_DifferentStrand:
                ostr << "different strand";
                break;

            default:
                ostr << "comparison unknown";
                break;
            }

            if (seq_state & CAnnotCompare::eSequence_SameSeq) {
                ostr << ", same sequence";
            }
            if (seq_state & CAnnotCompare::eSequence_SameProduct) {
                ostr << ", same product sequence";
            }

            ostr << "\n";
        }
        ostr << "\n";
    }
}


void CAnnotCompareDS::x_BackgroundJob(IAppJob* job)
{
    CAppJobDispatcher& disp = CAppJobDispatcher::GetInstance();
    try {
        m_ActiveJob = disp.StartJob(*job, "ObjManagerEngine",
                                    *this, -1, true);
    } catch(CAppJobException& e)  {
        LOG_POST(Error << "CAnnotCompareDS::x_BackgroundJob(): "
                          "Failed to start job: " << e.GetMsg());
        LOG_POST(e.ReportAll());
    }
}


void CAnnotCompareDS::x_DeleteAllJobs()
{
    CAppJobDispatcher& disp = CAppJobDispatcher::GetInstance();
    try {
        disp.DeleteJob(m_ActiveJob);
    }
    catch (CAppJobException& e)  {
        switch (e.GetErrCode())  {
        case CAppJobException::eUnknownJob:
        case CAppJobException::eEngine_UnknownJob:
            /// this is fine - job probably already finished
            break;

        default:
            // something wrong
            LOG_POST(Error << "CAnnotCompareDS::x_DeleteAllJobs(): "
                              "Error canceling job: " << e);
            LOG_POST(e.ReportAll());
        }
    }

    m_ActiveJob = -1;
}


BEGIN_EVENT_MAP(CAnnotCompareDS, CEventHandler)
    ON_EVENT(CAppJobNotification, CAppJobNotification::eStateChanged,
             &CAnnotCompareDS::OnAJNotification)
END_EVENT_MAP()


void CAnnotCompareDS::OnAJNotification(CEvent* evt)
{
    x_OnAppJobNotification(evt);
}

void CAnnotCompareDS::x_OnAppJobNotification(CEvent* evt)
{
    CAppJobNotification* notn = dynamic_cast<CAppJobNotification*>(evt);
    _ASSERT(notn);

    if (notn) {
        int job_id = notn->GetJobID();
        if (job_id != -1  &&  job_id != m_ActiveJob) {
            return;
        }

        switch (notn->GetState())    {
        case IAppJob::eCompleted:
            {{
                m_ActiveJob = -1;
                CRef<CObject> res_obj = notn->GetResult();
                CFeatCompare_Result* result =
                    dynamic_cast<CFeatCompare_Result*>(&*res_obj);
                if (result) {
                    m_Rows.swap(result->m_Rows);

                    CEvent evt(CViewEvent::eWidgetDataChanged);
                    Send(&evt, ePool_Parent);
                }
            }}
            break;

        case IAppJob::eFailed:
        case IAppJob::eCanceled:
            m_ActiveJob = -1;
            break;

        default:
            break;
        }
    }
}


CScope& CAnnotCompareDS::GetScope() const
{
    return *m_Scope;
}


const CAnnotCompareDS::TRows& CAnnotCompareDS::GetData() const
{
    return m_Rows;
}


size_t CAnnotCompareDS::GetRows(void) const
{
    return m_Rows.size();
}


const CAnnotCompareDS::SRow& CAnnotCompareDS::GetRow(size_t row_idx) const
{
    _ASSERT(row_idx < m_Rows.size());
    return m_Rows[row_idx];
}


/////////////////////////////////////////////////////////////////////////////
///
///  Alignment-based data source
///

class CFeatCompareJob : public CJobCancelable
{
public:
    CFeatCompareJob(CScope& scope);


    /// @name IAppJob implementation
    /// @{
    virtual CConstIRef<IAppJobProgress> GetProgress();
    virtual CRef<CObject>               GetResult();
    virtual CConstIRef<IAppJobError>    GetError();
    virtual string                      GetDescr() const;
    /// @}

protected:

    CRef<CScope> m_Scope;

    /// for status reporting
    size_t m_TotalComparisons;
    size_t m_CompletedComparisons;

    // for job results/status
    CRef<CAppJobError>        m_Error;
    CRef<CFeatCompare_Result> m_Result;
};


/////////////////////////////////////////////////////////////////////////////

CFeatCompareJob::CFeatCompareJob(CScope& scope)
    : m_Scope(&scope)
{
}


CConstIRef<IAppJobProgress> CFeatCompareJob::GetProgress()
{
    CRef<CAppJobProgress> p(new CAppJobProgress);
    p->SetNormDone(float(m_CompletedComparisons) / float(m_TotalComparisons));
    return CConstIRef<IAppJobProgress>(p.GetPointer());
}


CRef<CObject> CFeatCompareJob::GetResult()
{
    return CRef<CObject>(m_Result.GetPointer());
}


CConstIRef<IAppJobError> CFeatCompareJob::GetError()
{
    return CConstIRef<IAppJobError>(m_Error.GetPointer());
}


string CFeatCompareJob::GetDescr() const
{
    return string("CFeatCompareJob");
}

/////////////////////////////////////////////////////////////////////////////

class CFeatCompareJob_Align : public CFeatCompareJob
{
public:
    CFeatCompareJob_Align(CScope& scope, const CSeq_align& alignment);

    /// @name IAppJob implementation
    /// @{
    virtual EJobState   Run();
    /// @}

protected:
    CConstRef<CSeq_align> m_Align;
};


CFeatCompareJob_Align::CFeatCompareJob_Align(CScope& scope,
                                             const CSeq_align& alignment)
    : CFeatCompareJob(scope)
    , m_Align(&alignment)
{
}


IAppJob::EJobState CFeatCompareJob_Align::Run()
{
    m_Error.Reset(NULL);
    m_Result.Reset(new CFeatCompare_Result);

    ///
    /// don't merge our alignments!
    /// rather, we use them as-is and rely on the user supplying us with a
    /// curated alignment
    ///

    SAnnotSelector sel = CSeqUtils::GetAnnotSelector();
    sel.ExcludeNamedAnnots("SNP");

    /// first, we collect all features in all sequences represented in
    /// the alignment.  Each feature must be accounted for.
    typedef map<CConstRef<CSeq_feat>, CMappedFeat> TFeats;
    typedef map<CSeq_id_Handle, TFeats> TFeatMap;

    m_TotalComparisons     = 0;
    m_CompletedComparisons = 0;

    TFeatMap fmap;
    list<CSeq_id_Handle> ids;

    size_t rows = m_Align->CheckNumRows();
    for (size_t i = 0;  i < rows;  ++i) {
        const CSeq_id& id  = m_Align->GetSeq_id((int)i);
        CSeq_id_Handle sih = CSeq_id_Handle::GetHandle(id);
        ids.push_back(sih);

        CBioseq_Handle handle = m_Scope->GetBioseqHandle(sih);
        CFeat_CI feat_iter(handle, m_Align->GetSeqRange((int)i), m_Align->GetSeqStrand((int)i));
        TFeats& feats = fmap[sih];
        for ( ;  feat_iter;  ++feat_iter) {
            CConstRef<CSeq_feat> ref(&feat_iter->GetOriginalFeature());
            feats[ref] = *feat_iter;
        }

        m_TotalComparisons += feats.size();
    }

    /// next, we scan for the best matches across all sequences
    /// the goal is to represent each feature in each sequence, and report
    /// if the feature is missing

    /// for each sequence
    ITERATE (list<CSeq_id_Handle>, idh_iter1, ids) {

        /// find the best matching feature in each additional sequence
        ITERATE (list<CSeq_id_Handle>, idh_iter2, ids) {
            if (idh_iter1 == idh_iter2) {
                continue;
            }

            bool do_swap = (*idh_iter2 < *idh_iter1);

            CSeq_id_Handle sih1 = (do_swap ? *idh_iter2 : *idh_iter1);
            CSeq_id_Handle sih2 = (do_swap ? *idh_iter1 : *idh_iter2);

            /// we plan to delete out of set 2 if we find an exact match
            const TFeats& feat_set1 = (do_swap ? fmap[sih2] : fmap[sih1]);
            TFeats&       feat_set2 = (do_swap ? fmap[sih1] : fmap[sih2]);
            CBioseq_Handle bsh  = m_Scope->GetBioseqHandle(sih2);

            /// build or seq-loc mapper
            /// NB: optimize this out of this loop!
            CSeq_loc_Mapper mapper(*m_Align, *sih2.GetSeqId(), m_Scope);
            mapper.SetMergeAbutting();

            /// for each feature in feature set 1
            ITERATE (TFeats, fiter1, feat_set1) {
                if (IsCanceled()) {
                    NCBI_THROW(CException, eUnknown, "Job cancelled");
                }
                CMappedFeat f1 = fiter1->second;
                sel.SetFeatSubtype(f1.GetData().GetSubtype());

                CRef<CSeq_loc> mapped_loc = mapper.Map(f1.GetLocation());

                /// find all possible overlaps
                typedef multimap<CAnnotCompare::TCompareFlags, CMappedFeat> TCompMap;
                TCompMap comp_map;
                CFeat_CI iter(*m_Scope, *mapped_loc, sel);
                if (iter.GetSize()) {
                    for ( ;  iter;  ++iter) {
                        if (IsCanceled()) {
                            NCBI_THROW(CException, eUnknown, "Job cancelled");
                        }
                        CMappedFeat f2 = *iter;

                        CAnnotCompare annot_comp;
                        CAnnotCompare::TCompareFlags comp =
                            annot_comp.CompareFeats(f1.GetOriginalFeature(),
                                                    *mapped_loc, *m_Scope,
                                                    f2.GetOriginalFeature(),
                                                    f2.GetLocation(), *m_Scope,
                                                    NULL, NULL);
                        if ((comp & CAnnotCompare::eLocationMask) == CAnnotCompare::eLocation_Same) {
                            /// exact match
                            comp_map.clear();
                            comp_map.insert(TCompMap::value_type(comp, f2));

                            /// we also strip the feature from feature set 2
                            CConstRef<CSeq_feat> ref2(&f2.GetOriginalFeature());
                            TFeats::iterator fi2 = feat_set2.find(ref2);
                            if (fi2 != feat_set2.end()) {
                                feat_set2.erase(fi2);
                                ++m_CompletedComparisons;
                            }
                            break;
                        } else if ((comp & CAnnotCompare::eLocationMask) != CAnnotCompare::eLocation_Missing) {
                            comp_map.insert(TCompMap::value_type(comp, f2));
                        }
                    }

                    /// save any categorizations we've got
                    ITERATE (TCompMap, cmiter, comp_map) {
                        CAnnotCompareDS::SRow row;
                        row.scope = m_Scope;

                        row.feat1.Reset(&f1.GetOriginalFeature());
                        row.feat1_loc.Reset(&f1.GetLocation());
                        row.feat2.Reset(&cmiter->second.GetOriginalFeature());
                        row.feat2_loc.Reset(&cmiter->second.GetLocation());

                        /**
                        if (do_swap) {
                            swap(row.feat1,     row.feat2);
                            swap(row.feat1_loc, row.feat2_loc);
                        }
                        **/

                        row.loc_state = cmiter->first;
                        m_Result->m_Rows.push_back(row);
                    }
                } else {
                    /// missing!
                    /// we always save at least the mapped location
                    CAnnotCompareDS::SRow row;
                    row.scope = m_Scope;

                    row.feat1.Reset(&f1.GetOriginalFeature());
                    row.feat1_loc.Reset(&f1.GetLocation());

                    if (mapped_loc->Which() != CSeq_loc::e_Null) {
                        row.feat2_loc = mapped_loc;
                    }

                    /**
                    if (do_swap) {
                        swap(row.feat1,     row.feat2);
                        swap(row.feat1_loc, row.feat2_loc);
                    }
                    **/

                    row.loc_state = CAnnotCompare::eLocation_Missing;
                    m_Result->m_Rows.push_back(row);
                }

                ++m_CompletedComparisons;
            }
        }
    }

    /// finalize our data
    size_t count = 0;
    NON_CONST_ITERATE (CAnnotCompareDS::TRows, iter, m_Result->m_Rows) {
        CAnnotCompareDS::SRow& data = *iter;
        data.row_idx = count++;
        if ( data.feat1  &&  !data.feat1_loc ) {
            data.feat1_loc.Reset(&data.feat1->GetLocation());
        }
        if ( data.feat2  &&  !data.feat2_loc ) {
            data.feat2_loc.Reset(&data.feat2->GetLocation());
        }

        if (data.feat1_loc) {
            data.feat1_seq = sequence::GetIdHandle(*data.feat1_loc, m_Scope);
        }
        if (data.feat2_loc) {
            data.feat2_seq = sequence::GetIdHandle(*data.feat2_loc, m_Scope);
        }
    }
    return eCompleted;
}


/////////////////////////////////////////////////////////////////////////////


CAnnotCompare_AlignDS::CAnnotCompare_AlignDS(CScope& scope,
                                             const CSeq_annot& annot)
    : CAnnotCompareDS(scope)
{
    _ASSERT(annot.GetData().Which() == CSeq_annot::TData::e_Align);
    std::copy(annot.GetData().GetAlign().begin(),
              annot.GetData().GetAlign().end(),
              back_inserter(m_Alignments));
    x_Init();
}


CAnnotCompare_AlignDS::CAnnotCompare_AlignDS(CScope& scope,
                                             const CSeq_align& align)
    : CAnnotCompareDS(scope)
{
    m_Alignments.push_back(CConstRef<CSeq_align>(&align));
    x_Init();
}


CAnnotCompare_AlignDS::CAnnotCompare_AlignDS(CScope& scope,
                                             const CSeq_align_set& align_set)
    : CAnnotCompareDS(scope)
{
    /// save the original alignments as well
    std::copy(align_set.Get().begin(),
              align_set.Get().end(),
              back_inserter(m_Alignments));

    x_Init();
}


CAnnotCompare_AlignDS::CAnnotCompare_AlignDS(CScope& scope,
                                             const list< CConstRef<CSeq_align> >& aligns)
    : CAnnotCompareDS(scope),
      m_Alignments(aligns)
{
    x_Init();
}


CAnnotCompare_AlignDS::~CAnnotCompare_AlignDS()
{
}


void CAnnotCompare_AlignDS::x_Init()
{
    CAlnMix mix(*m_Scope);
    ITERATE (list< CConstRef<CSeq_align> >, iter, m_Alignments) {
        mix.Add(**iter);
    }
    mix.Merge(CAlnMix::fGapJoin);

    m_Align.Reset(&mix.GetSeqAlign());
}


void CAnnotCompare_AlignDS::Update()
{
    Clear();

    ///
    /// launch a backgroun job for our data
    ///
    CRef<CFeatCompareJob_Align> job(new CFeatCompareJob_Align(*m_Scope, *m_Align));
    x_BackgroundJob(job);
}


/////////////////////////////////////////////////////////////////////////////
///
/// Annotation Comparison by location alone
/// This is done by checking a few constraints
///

/////////////////////////////////////////////////////////////////////////////

class CFeatCompareJob_Location : public CFeatCompareJob
{
public:
    CFeatCompareJob_Location(CScope& scope, const CSeq_loc& loc);

    /// @name IAppJob implementation
    /// @{
    virtual EJobState Run();
    /// @}

protected:

    CConstRef<CSeq_loc> m_Loc;
};


CFeatCompareJob_Location::CFeatCompareJob_Location(CScope& scope,
                                                   const CSeq_loc& loc)
    : CFeatCompareJob(scope)
    , m_Loc(&loc)
{
}


IAppJob::EJobState CFeatCompareJob_Location::Run()
{
    m_Error.Reset(NULL);
    m_Result.Reset(new CFeatCompare_Result);

    SAnnotSelector sel = CSeqUtils::GetAnnotSelector();

    /// iterate all features on our sequence
    CFeat_CI feat_iter(*m_Scope, *m_Loc, sel);
    m_TotalComparisons = feat_iter.GetSize();
    for ( ;  feat_iter;  ++feat_iter) {
        if (IsCanceled()) {
            NCBI_THROW(CException, eUnknown, "Job cancelled");
        }
        CMappedFeat f1 = *feat_iter;
        string annot_name;
        if (f1.GetSeq_feat_Handle().GetAnnot().IsNamed()) {
            annot_name = f1.GetSeq_feat_Handle().GetAnnot().GetName();
        }

        /// scan for additional features not on the same annotation
        SAnnotSelector sel2(sel);
        sel2.SetFeatSubtype(f1.GetData().GetSubtype());

        typedef multimap<CAnnotCompare::TCompareFlags, CMappedFeat> TCompMap;
        TCompMap comp_map;
        CFeat_CI feat_iter2(*m_Scope, f1.GetLocation(), sel2);
        for ( ;  feat_iter2;  ++feat_iter2) {
            if (IsCanceled()) {
                NCBI_THROW(CException, eUnknown, "Job cancelled");
            }
            CMappedFeat f2 = *feat_iter2;
            if (f2.GetSeq_feat_Handle().GetAnnot().IsNamed()) {
                if (f2.GetSeq_feat_Handle().GetAnnot().GetName() == annot_name) {
                    continue;
                }
            } else if (annot_name.empty()) {
                continue;
            }

            CAnnotCompare annot_comp;
            CAnnotCompare::TCompareFlags comp =
                annot_comp.CompareFeats(f1, *m_Scope, f2, *m_Scope,
                                        NULL, NULL);
            if ((comp & CAnnotCompare::eLocationMask) == CAnnotCompare::eLocation_Same) {
                comp_map.clear();
                comp_map.insert(TCompMap::value_type(comp, f2));
                break;
            } else if ((comp & CAnnotCompare::eLocationMask) != CAnnotCompare::eLocation_Missing) {
                comp_map.insert(TCompMap::value_type(comp, f2));
            }
        }

        if (comp_map.size()) {
            /// save any additional categorizations we've got
            ITERATE (TCompMap, cmiter, comp_map) {
                CAnnotCompareDS::SRow row;
                row.scope = m_Scope;

                row.feat1.Reset(&f1.GetOriginalFeature());
                row.feat1_loc.Reset(&f1.GetLocation());
                row.feat1_seq = sequence::GetIdHandle(*row.feat1_loc, m_Scope);
                row.feat2.Reset(&cmiter->second.GetOriginalFeature());
                row.feat2_loc.Reset(&cmiter->second.GetLocation());
                row.feat2_seq = sequence::GetIdHandle(*row.feat2_loc, m_Scope);

                row.loc_state = cmiter->first;
                m_Result->m_Rows.push_back(row);
            }
        } else {
            /// missing!
            CAnnotCompareDS::SRow row;
            row.scope = m_Scope;

            row.feat1.Reset(&f1.GetOriginalFeature());
            row.feat1_loc.Reset(&f1.GetLocation());
            row.feat1_seq = sequence::GetIdHandle(*row.feat1_loc, m_Scope);

            row.loc_state = CAnnotCompare::eLocation_Missing;
            m_Result->m_Rows.push_back(row);
        }

        ++m_CompletedComparisons;
    }
    return eCompleted;
}

/////////////////////////////////////////////////////////////////////////////

CAnnotCompare_LocationDS::CAnnotCompare_LocationDS(CScope& scope,
                                                   const CSeq_loc& loc)
    : CAnnotCompareDS(scope)
    , m_Loc(&loc)
{
}


CAnnotCompare_LocationDS::~CAnnotCompare_LocationDS()
{
}


void CAnnotCompare_LocationDS::Update()
{
    Clear();

    ///
    /// launch a backgroun job for our data
    ///
    CRef<CFeatCompareJob_Location> job
        (new CFeatCompareJob_Location(*m_Scope, *m_Loc));
    x_BackgroundJob(job);
}


END_NCBI_SCOPE
