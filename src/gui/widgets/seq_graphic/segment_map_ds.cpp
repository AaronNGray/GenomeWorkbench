/*  $Id: segment_map_ds.cpp 44596 2020-01-29 15:41:36Z filippov $
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
 * Authors:  Liangshou Wu
 *
 */

#include <ncbi_pch.hpp>
#include <gui/widgets/seq_graphic/segment_map_ds.hpp>
#include <gui/widgets/seq_graphic/seqgraphic_job.hpp>
#include <gui/widgets/seq_graphic/seqgraphic_utils.hpp>
#include <gui/widgets/seq_graphic/segment_smear_glyph.hpp>
#include <gui/widgets/seq_graphic/seqgraphic_job_result.hpp>
#include <gui/widgets/seq_graphic/segment_glyph.hpp>
#include <gui/widgets/seq_graphic/comment_glyph.hpp>
#include <gui/widgets/seq_graphic/switch_point_glyph.hpp>

#include <objmgr/table_field.hpp>
#include <objmgr/annot_ci.hpp>
#include <objmgr/seq_map_switch.hpp>
#include <objmgr/seqdesc_ci.hpp>
#include <objmgr/util/sequence.hpp>
#include <objects/seq/MolInfo.hpp>
#include <objects/seq/Seq_hist.hpp>
#include <objects/seqtable/Seq_table.hpp>
#include <objects/seqtable/SeqTable_column.hpp>

#include <util/cache/icache.hpp>
#include <corelib/perf_log.hpp>
#include <gui/utils/log_performance.hpp>

BEGIN_NCBI_SCOPE
USING_SCOPE(objects);

class ICache;

///////////////////////////////////////////////////////////////////////////////
/// CSGSegmentMapJob declaration
///
class CSGSegmentMapJob : public CSeqGraphicJob
{
public:
    CSGSegmentMapJob(const string& desc, objects::CBioseq_Handle handle,
        const TSeqRange& range, int depth, int cutoff, TJobToken token);

    static int GetSegmentMapLevels(const objects::CBioseq_Handle& handle,
        const TSeqRange& range);

    static bool HasSegmentMap(const objects::CBioseq_Handle& Handle, int level,
        const TSeqRange& range);

protected:
    virtual EJobState x_Execute();

private:
    bool x_AdaptiveDepth() const
    {   return m_Depth == -1; }

    /// merge equal segment maps into one map.
    IAppJob::EJobState x_MergeSeqSegments(CSegmentGlyph::TSeqMapList& seg_maps,
        CSeqGlyph::TObjects& objs);

private:
    objects::CBioseq_Handle m_Handle;   ///< target sequence
    TSeqRange               m_Range;    ///< target range

    /// Resolve depth for selector.
    /// Valid numbers include: -1: adaptive, 0: first level, 1: second
    /// Depending on the sequence currently shown, first level may be
    /// the scaffolds (chromosome) or the compoments (config)
    int                     m_Depth;

    /// limit of number of segments we want to show.
    /// <=0 means there is no limit.
    int                     m_Cutoff;
};

///////////////////////////////////////////////////////////////////////////////
/// CSGSegmentSmearJob declaration
///
class CSGSegmentSmearJob : public CSGAnnotJob
{
public:
    CSGSegmentSmearJob(const string& desc, objects::CBioseq_Handle handle,
                       const TSeqRange& range, TModelUnit scale, const string& annot, ICache* cache = 0, const CSeqVector* seq_vec = nullptr);

    static void GetAnnotNames(const objects::CBioseq_Handle& handle,
        const TSeqRange& range, objects::SAnnotSelector& sel,
        TAnnotNameTitleMap& annots);

protected:
    virtual EJobState x_Execute();

private:
    IAppJob::EJobState x_LoadFromDeltaSeqs();

    IAppJob::EJobState x_LoadFromNA();

    IAppJob::EJobState x_LoadFromNetCache(const string& key);

    void x_WriteToCache(const string& key, const CSegmentSmearGlyph::CSegMap& density_map);

    string x_GetCacheKey() const;


    bool x_IsGap(vector<char> is_gaps, int row) const
    {
        return is_gaps[row / 8] & 0x01 << (8 - row % 8);
    }
    
    void x_LoadGaps();

private:
    TModelUnit  m_Scale;    ///< bases per pixel
    string      m_Annot;    ///< annotation storing the segments
    ICache*     m_Cache;    ///< cache to store segment ranges
    const CSeqVector* m_SeqVector;
};

///////////////////////////////////////////////////////////////////////////////
/// CSGSegmentsSeqIDJob declaration
///
class CSGSegmentsSeqIDJob : public CSeqGraphicJob
{
public:
    CSGSegmentsSeqIDJob(const string& desc, CSeqGlyph::TObjects& objs,
        objects::CScope& scope, TJobToken token);

protected:
    virtual EJobState x_Execute();

    CSeqGlyph::TObjects     m_Objs;     ///< objects for post-processing
    objects::CScope&        m_Scope;    ///< scope for retrieving the CBioseq
};


///////////////////////////////////////////////////////////////////////////////
/// CSGSwitchPointJob declaration
///
class CSGSwitchPointJob : public CSeqGraphicJob
{
public:
    CSGSwitchPointJob(const string& desc, objects::CBioseq_Handle handle);

protected:
    virtual EJobState x_Execute();

private:
    objects::CBioseq_Handle m_Handle;   ///< target sequence
};


///////////////////////////////////////////////////////////////////////////////
/// CSGSegmentMapDS implementation
///

ICache* CSGSegmentMapDS::m_Cache = 0;
bool    CSGSegmentMapDS::m_Enabled = true;

CSGSegmentMapDS::CSGSegmentMapDS(CScope& scope, const CSeq_id& id)
    : CSGGenBankDS(scope, id)
    , m_IsChromosome(true)
{}


void CSGSegmentMapDS::LoadSegmentMap(const TSeqRange& range,
                                  int cutoff, TJobToken token)
{
    CRef<CSGSegmentMapJob> job(
        new CSGSegmentMapJob("SegmentMap", m_Handle,
        range, m_Depth, cutoff, token));
    x_LaunchJob(*job);
}


void CSGSegmentMapDS::LoadSegmentSmear(const TSeqRange& range,
                                       TModelUnit scale, const CSeqVector* seq_vec)
{
    CRef<CSGSegmentSmearJob> job(
        new CSGSegmentSmearJob("SegmentSmear",
                               m_Handle, range, scale, m_Annot, m_Cache, seq_vec));
    x_LaunchJob(*job);
}


void CSGSegmentMapDS::LoadSegmentMapSeqIDs(CSeqGlyph::TObjects& objs,
                                        TJobToken token)
{
    CRef<CSGSegmentsSeqIDJob> job(
        new CSGSegmentsSeqIDJob("Resolving component IDs",
        objs, GetScope(), token) );
    x_LaunchJob(*job);
}


void CSGSegmentMapDS::LoadSwitchPoints()
{
    CRef<CSGSwitchPointJob> job(
        new CSGSwitchPointJob("Switch points", m_Handle) );
    x_LaunchJob(*job);
}


int CSGSegmentMapDS::GetSegmentMapLevels(const TSeqRange& range) const
{
    return CSGSegmentMapJob::GetSegmentMapLevels(m_Handle, range);
}


bool CSGSegmentMapDS::HasSegmentMap(int level, const TSeqRange& range) const
{
    return CSGSegmentMapJob::HasSegmentMap(m_Handle, level, range);
}


bool CSGSegmentMapDS::HasComponent(const TSeqRange& range) const
{
    int level = IsChromosome() ? 1 : 0;
    return HasSegmentMap(level, range);
}


bool CSGSegmentMapDS::HasScaffold(const TSeqRange& range) const
{
    if (IsChromosome()) {
        return HasSegmentMap(0, range);
    }

    return false;
}


void CSGSegmentMapDS::GetAnnotNames(objects::SAnnotSelector& sel,
                                 const TSeqRange& range,
                                 TAnnotNameTitleMap& names) const
{
    CSeqUtils::SetResolveDepth(sel, true, -1);
    CSGSegmentSmearJob::GetAnnotNames(m_Handle, range, sel, names);
}


void CSGSegmentMapDS::SetSegmentLevel(ESegmentLevel level)
{
    int depth = x_ConceptualToRealDepth(level);
    SetDepth(depth);
}

void CSGSegmentMapDS::SetICacheClient(ICache* pCache)
{
    m_Cache = pCache;
}

void CSGSegmentMapDS::SetEnabled(bool enabled)
{
    m_Enabled = enabled;
}

bool CSGSegmentMapDS::GetEnabled()
{
    return m_Enabled;
}

int CSGSegmentMapDS::x_ConceptualToRealDepth(ESegmentLevel level) const
{
    int depth = level;
    if ( !m_IsChromosome ) {
        if (level == eComponent) {
            depth = 0;
        } else if (level == eContig) {
            depth = -2; /// invalid
        }
    }
    return depth;
}

///////////////////////////////////////////////////////////////////////////////
/// CSGSegmentMapDSType implementation
///
ISGDataSource*
CSGSegmentMapDSType::CreateDS(SConstScopedObject& object) const
{
    const CSeq_id& id = dynamic_cast<const CSeq_id&>(object.object.GetObject());
    return new CSGSegmentMapDS(object.scope.GetObject(), id);
}


string CSGSegmentMapDSType::GetExtensionIdentifier() const
{
    static string sid("seqgraphic_segment_map_ds_type");
    return sid;
}


string CSGSegmentMapDSType::GetExtensionLabel() const
{
    static string slabel("Graphical View Segment Map Data Source Type");
    return slabel;
}


bool CSGSegmentMapDSType::IsSharable() const
{
    return false;
}



///////////////////////////////////////////////////////////////////////////////
/// CSGSegmentMapJob implementation
///
CSGSegmentMapJob::CSGSegmentMapJob(const string& desc, CBioseq_Handle handle,
                                   const TSeqRange& range, int depth,
                                   int cutoff, TJobToken token)
    : CSeqGraphicJob(desc)
    , m_Handle(handle)
    , m_Range(range)
    , m_Depth(depth)
    , m_Cutoff(cutoff)
{
    SetToken(token);
}


int CSGSegmentMapJob::GetSegmentMapLevels(const objects::CBioseq_Handle& handle,
                                          const TSeqRange& range)
{
    SSeqMapSelector sel(CSeqMap::fFindAnyLeaf | CSeqMap::fFindExactLevel);
    CRegistryReadView view = CSeqUtils::GetSelectorRegistry();
    int max_search_segs = CSeqUtils::GetMaxSearchSegments(view);
    SAnnotSelector::EMaxSearchSegmentsAction action{CSeqUtils::GetMaxSearchSegmentsAction(view)};
    int total_levels = 0;
    for (int level = 0;  level < 2;  ++level) {
        int nSegs{0};
        sel.SetResolveCount(level);
        TSeqPos start = range.GetFrom();
        TSeqPos stop = range.GetTo();
        TSeqPos step = 1000000;
        bool isMaxSegmentsReached{false};
        while (start <= stop) {
            TSeqRange r(start, min(start + step - 1, stop));
            start += step;
            CSeqMap::const_iterator seg(handle, sel, r);
            while (seg  &&  seg.GetType() != CSeqMap::eSeqRef) {
                ++nSegs;
                ++seg;
                isMaxSegmentsReached = CSeqUtils::CheckMaxSearchSegments(nSegs, max_search_segs, action);
                if(isMaxSegmentsReached) {
                    break;
                }
            }
            if(isMaxSegmentsReached) {
                break;
            }
            if(seg) {
                ++total_levels;
                break;
            }
        }
//        LOG_POST(Trace << "Scanned: " << nSegs << " segments at level: " << level);
    }
    return total_levels;
}


bool CSGSegmentMapJob::HasSegmentMap(const objects::CBioseq_Handle& handle,
                                     int level, const TSeqRange& range)
{
    int max_level = 3;
    SSeqMapSelector sel(CSeqMap::fFindAnyLeaf | CSeqMap::fFindExactLevel);
    level = min(level, max_level);

    int start_l = level;
    int end_l = level;
    if (level < 0) {
        start_l = 0;
        end_l = max_level;
    }

    for (int l = start_l;  l <= end_l;  ++l) {
        sel.SetResolveCount(l);
        CSeqMap::const_iterator seg(handle, sel, range);
        while (seg  &&  seg.GetType() != CSeqMap::eSeqRef) {
            ++seg;
        }
        if (seg) return true;
    }
    return false;
}


IAppJob::EJobState CSGSegmentMapJob::x_Execute()
{
    try {
        CSGJobResult* result = new CSGJobResult();
        m_Result.Reset(result);
        result->m_Token = m_Token;

        // -2 means we have already known there is no data for
        // the given level.
        if (m_Depth == -2){
            return eCompleted;
        }

        bool over_limit = false;
        CSegmentGlyph::TSeqMapList seg_maps;

        CSeq_id_Handle target_sih = m_Handle.GetSeq_id_Handle();
        /*
        LOG_POST(Info << "CSGSegmentMapJob::x_Execute() started "
                      << " SeqId=" << target_sih.AsString()
                      << " Range.GetFrom()=" << m_Range.GetFrom()
                      << " Range.GetLength()=" << m_Range.GetLength()
                      << " Depth=" << m_Depth
                 );
        */

        for (size_t levels = 0;  levels < 4;  ++levels) {
            SSeqMapSelector sel(CSeqMap::fFindAnyLeaf | CSeqMap::fFindExactLevel,
                                x_AdaptiveDepth() ? levels : m_Depth);

            seg_maps.clear();

            if (IsCanceled()) {
                return eCanceled;
            }

            /// determine how many segments we have here
            /// this determines if we should try to retrieve the skeletons
            size_t seg_count = 0;

            try {
                for (CSeqMap_CI seg(m_Handle, sel, m_Range);
                     seg  &&  seg.GetPosition() < m_Range.GetTo();
                     ++seg)
                {
                    if (IsCanceled()) {
                        return eCanceled;
                    }

                    if (seg.GetType() == CSeqMap::eSeqRef) {
                        ++seg_count;
                    }
                    if (seg.GetEndPosition() > m_Range.GetTo()) {
                        break;
                    }
                    if (m_Cutoff > 0  &&  seg_count > (size_t)m_Cutoff) {
                        over_limit = true;
                        break;
                    }
                }

                if (over_limit) break;

                // We set a limit on the number of components to retrieve. If more
                // than kSegmentLoadLimit (see segment_map_ds.hpp) components are
                // visible, the component overlaps will not be displayed.
                // If the component sequence already been in scope, it will!
                bool fetch_sequences = (seg_count < kSegmentLoadLimit);

                seg_count = 0;
                for (CSeqMap_CI seg(m_Handle, sel, m_Range);
                     seg  &&  seg.GetPosition() < m_Range.GetTo();
                     ++seg)
                {
                    if (IsCanceled()) {
                        return eCanceled;
                    }

                    if ( seg.GetType() == CSeqMap::eSeqRef ) {
                        ++seg_count;
                        CSeq_id_Handle sih = seg.GetRefSeqid();

                        /// these are the mapped coordinates - i.e., the chunk of
                        /// the main sequence (in main sequence coordinates)
                        /// represented by this piece
                        TSeqPos seg_from = seg.GetPosition();
                        TSeqPos seg_to   = seg.GetEndPosition() - 1;

                        /// these are the coordinates on the actual chunk
                        TSeqPos ref_from = seg.GetRefPosition();
                        TSeqPos ref_to   = seg.GetRefEndPosition() - 1;



                        CScope::EGetBioseqFlag flag = fetch_sequences ?
                                        CScope::eGetBioseq_All :
                                        CScope::eGetBioseq_Loaded;

                        CRef<CSegmentGlyph> seqmap;
                        CBioseq_Handle bsh = m_Handle.GetScope().GetBioseqHandle(sih, flag);
                        TSeqRange ref_r = TSeqRange(ref_from, ref_to);
                        TSeqRange seg_r = TSeqRange(seg_from, seg_to);
                        bool neg = seg.GetRefMinusStrand();

                        if (bsh) {
                            TSeqPos length = bsh.GetBioseqLength();
                            seqmap.Reset(new CSegmentGlyph(sih, ref_r, target_sih,
                                                           seg_r, neg, length));
                        } else {
                            seqmap.Reset(new CSegmentGlyph(sih, ref_r, target_sih,
                                                           seg_r, neg ));
                        }

                        //CRef<CSeqGlyph> ref;
                        //ref.Reset(seqmap.GetPointer());
                        seg_maps.push_back(seqmap);
                    }
                    if (seg.GetEndPosition() > m_Range.GetTo()) {
                        break;
                    }
                }
            }
            catch (CException& e) {
                LOG_POST(Info << "CSGSegmentMapJob::GetSeqMaps(): "
                         "exception in retrieval of seq-maps: " << e.GetMsg());
            }

            /// make sure we preserve the last state - it is possible we will
            /// get through our iteration and leave maps empty
            if (seg_maps.size()) {
                CSeqGlyph::TObjects tmp;
                EJobState state = x_MergeSeqSegments(seg_maps, tmp);
                if (state != eCompleted) {
                    return state;
                }
                result->m_ObjectList.swap(tmp);
            }

            if (result->Size() > 1  ||
                (m_Range.GetLength() >= 1000000  &&  result->Size() > 0)) {
                break;
            }

            if (!x_AdaptiveDepth()) {
                break;
            }
        }

        if (over_limit  &&  result->m_ObjectList.empty()) {
            string msg = "There are more than ";
            msg += NStr::IntToString(m_Cutoff);
            msg += " segments for the given range. Segment map is not shown at this zooming level!";
            TModelUnit x = (m_Range.GetFrom() + m_Range.GetTo() ) * 0.5;
            CRef<CSeqGlyph> message_glyph(
                new CCommentGlyph(msg, TModelPoint(x, 0.0), true));
            result->m_ObjectList.push_back(message_glyph);
        }
    } catch (CException& ex) {
        m_Error.Reset(new CAppJobError(ex.GetMsg()));
        return eFailed;
    } catch (std::exception& ex) {
        m_Error.Reset(new CAppJobError(ex.what()));
        return eFailed;
    }

    return eCompleted;
}


IAppJob::EJobState
CSGSegmentMapJob::x_MergeSeqSegments(CSegmentGlyph::TSeqMapList& seg_maps,
                                     CSeqGlyph::TObjects& objs)
{
    CSeqGlyph::TObjects merged;
    typedef pair<const TSeqRange, CRef<CSegmentGlyph> > CSegMapPair;
    typedef multimap<const TSeqRange, CRef<CSegmentGlyph> > CSegMapMap;
    CSegMapMap merge_map;
    set <TSeqRange> map_keys;

    // first, build out structure
    ITERATE(CSegmentGlyph::TSeqMapList, iter, seg_maps) {
        if (IsCanceled()) {
            return eCanceled;
        }

        CRef<CSegmentGlyph> segmap = *iter;
        const TSeqRange& range = segmap->GetLocation().GetTotalRange();

        merge_map.insert( CSegMapPair(range, segmap) );  // values
        map_keys.insert( range );  // keys (ranges)
    }

    // do the merger
    ITERATE(set <TSeqRange>, iter, map_keys) {
        if (IsCanceled()) {
            return eCanceled;
        }

        pair<CSegMapMap::iterator, CSegMapMap::iterator> the_bounds;
        the_bounds = merge_map.equal_range(*iter);

        CRef<CSegmentGlyph> ref;  // resulting map
        set<TSeqRange> comp_range;  // accumulated ranges
        set<TSeqRange> mapped_range;
        for (CSegMapMap::iterator it = the_bounds.first;
             it != the_bounds.second; it++)
        {
            CSegMapPair pair = *it;
            CRef<CSegmentGlyph> segmap = pair.second;

            ITERATE (set<TSeqRange>, cr_it, segmap->GetComponentRanges())
                comp_range.insert(*cr_it);

            ITERATE (set<TSeqRange>, mr_it, segmap->GetMappedRanges())
                mapped_range.insert(*mr_it);

            if (ref.Empty())
                ref.Reset(pair.second.GetPointer());
        }

        ref->SetComponentRanges(comp_range);
        ref->SetMappedRanges(mapped_range);

        CRef<CSeqGlyph> obj_ref;
        obj_ref.Reset(ref.GetPointer());
        objs.push_back(obj_ref);
    }
    return eCompleted;
}



/// utility function
CSegmentConfig::ESequenceQuality SGetSeqQuality(CMolInfo_Base::TTech tech)
{
    switch (tech) {
        case CMolInfo::eTech_htgs_1:
        case CMolInfo::eTech_htgs_2:
            return CSegmentConfig::eDraft;
        case CMolInfo::eTech_htgs_3:
            return CSegmentConfig::eFinished;
        case CMolInfo::eTech_wgs:
            return CSegmentConfig::eWgs;
        default:
            return CSegmentConfig::eOther;
    }
}

///////////////////////////////////////////////////////////////////////////////
/// CSGSegmentSmearJob implementation
///
CSGSegmentSmearJob::CSGSegmentSmearJob(const string& desc,
                                       objects::CBioseq_Handle handle,
                                       const TSeqRange& range,
                                       TModelUnit scale,
                                       const string& annot,
                                       ICache* cache,
                                       const CSeqVector* seq_vec)
    : CSGAnnotJob(desc, handle,
        CSeqUtils::GetAnnotSelector(CSeq_annot::C_Data::e_Seq_table), range)
    , m_Scale(scale)
    , m_Annot(annot)
    , m_Cache(cache)
    , m_SeqVector(seq_vec)
{
    SetTaskName("Loading segment map...");
    if ( !m_Annot.empty() ) {
        m_Sel.AddNamedAnnots(m_Annot);
        m_Sel.IncludeNamedAnnotAccession(m_Annot);
    }
}


void CSGSegmentSmearJob::GetAnnotNames(const objects::CBioseq_Handle& handle,
                                       const TSeqRange& /*range*/,
                                       objects::SAnnotSelector& sel,
                                       TAnnotNameTitleMap& annots)
{
    sel.SetCollectNames();
    // LOG_POST(Error << "Before CAnnotTypes_CI()");
    CAnnotTypes_CI annot_it(CSeq_annot::C_Data::e_Seq_table, handle,
        TSeqRange::GetWhole(), eNa_strand_unknown, &sel);
    // LOG_POST(Error << "After CAnnotTypes_CI()");
    ITERATE (CAnnotTypes_CI::TAnnotNames, iter, annot_it.GetAnnotNames()) {
        if (iter->IsNamed()) {
            if (iter->GetName().find("@@") == string::npos) {
                annots.insert(TAnnotNameTitleMap::value_type(iter->GetName(), ""));
            }
        } else {
            annots.insert(TAnnotNameTitleMap::value_type(
                CSeqUtils::GetUnnamedAnnot(), ""));
        }
    }

    // In case there are more than one NA accesssions with the same accession,
    // but different versions, we pick the most recent one.
    TAnnotNameTitleMap::iterator pre = annots.begin();
    while (pre != annots.end()) {
        TAnnotNameTitleMap::iterator iter = pre;
        ++iter;
        if (iter != annots.end()
            &&  CSeqUtils::IsNAA(pre->first, true)
            &&  CSeqUtils::IsNAA(iter->first, true)
            &&  pre->first.substr(0, 11) == iter->first.substr(0, 11)) {
                annots.erase(pre);
        }
        pre = iter;
    }
}


IAppJob::EJobState CSGSegmentSmearJob::x_Execute()
{
    if (m_Cache) {
        CConstRef<CSeq_id> id = m_Handle.GetInitialSeqIdOrNull();
        if (!id.Empty() && id->IsLocal()) {
            return eFailed;
        }
        string key = x_GetCacheKey();
        try {
            if (m_Cache->HasBlobs(key,"")) 
                return x_LoadFromNetCache(key);
        }
        catch (const CException& e) {
            LOG_POST(Warning << "CSGSegmentSmearJob::x_Execute: "
                "failed to retrieve map from cache: " << e.GetMsg());
        }
    }
    x_LoadGaps();
    if (m_Annot.empty()) {
        // brute force, load data from delta seqs
        return x_LoadFromDeltaSeqs();
    }
    return x_LoadFromNA();
}

void CSGSegmentSmearJob::x_LoadGaps() // pre-warming cache for later faster loads
{
    if (!m_SeqVector)
        return;

    TSeqPos from = m_Cache ? 0 : m_Range.GetFrom();
    TSeqPos to = m_Cache ? m_Handle.GetBioseqLength() - 1 : m_Range.GetTo();

    CSeqVector_CI seq_vec_it(*m_SeqVector, from);
    CSeqMap_CI it = seq_vec_it.GetCurrentSeqMap_CI();
    
    while(it)
    {    
        const TSeqPos start = it.GetPosition();
        if (start > to)
            break;  
        auto t = it.GetType();
        ++it;
    }
}

IAppJob::EJobState CSGSegmentSmearJob::x_LoadFromDeltaSeqs()
{
    CLogPerformance perfLog("CSGSegmentSmearJob::x_LoadFromDeltaSeqs");
    perfLog.AddParameter ("description", "Load segment smear map from delta seqs");
    CSGJobResult* result = new CSGJobResult();
    m_Result.Reset(result);
    typedef CSegmentSmearGlyph::TValueType TValType;

    bool is_chromosome =
        CSGUtils::IsChromosome(m_Handle, m_Handle.GetScope());
    int depth = is_chromosome ? 1 : 0;
    CSeq_id_Handle target_sih = m_Handle.GetSeq_id_Handle();

    TSeqPos     r_from = m_Cache ? 0 : m_Range.GetFrom();
    TSeqPos     r_to = m_Cache ? m_Handle.GetBioseqLength() - 1 : m_Range.GetTo();
    TSeqRange   range(r_from, r_to);

    auto_ptr<CSegmentSmearGlyph::CSegMap>
        the_map(new CSegmentSmearGlyph::CSegMap(
            r_from, r_to,
            (float)m_Scale, new max_func<TValType>));

    TSeqPos pre_to = r_from;
    TValType gap_val = (TValType)CSegmentConfig::eGap;
    SSeqMapSelector sel(CSeqMap::fFindAnyLeaf | CSeqMap::fFindExactLevel, depth);
    using TSegData = struct {
        TSeqRange range;
        TValType val;
        size_t index;
    };
    vector<TSegData> data;
    CScope::TIds ids;
    size_t index = 0;
    // collect gaps and for non-gaps collect list of seq_id for bulk retrieval
    // store index, the index of seq_id in the list 
    for (CSeqMap_CI seg(m_Handle, sel, range); seg  &&  seg.GetPosition() < r_to; ++seg) {
        if (IsCanceled()) 
            return eCanceled;
        if ( seg.GetType() == CSeqMap::eSeqRef ) {
            /// these are the mapped coordinates
            TSeqPos seg_from = seg.GetPosition();
            TSeqPos seg_to   = seg.GetEndPosition() - 1;
            if (pre_to < seg_from) {
                data.push_back({TSeqRange(pre_to, seg_from - 1), gap_val, 0});
            }
            ids.push_back(seg.GetRefSeqid());
            // val will be resolved after bulk seq_id retrieval
            data.push_back({TSeqRange(seg_from, seg_to), (TValType)(CSegmentConfig::eOther), index});
            ++index;
            pre_to = seg_to + 1;
        }
        if (seg.GetEndPosition() > r_to) {
            break;
        }
    }
    if (pre_to < r_to) 
        data.push_back({TSeqRange(pre_to, r_to), gap_val, 0});

    // do bulk retireval 
    auto handles = m_Handle.GetScope().GetBioseqHandles(ids);
    // update val for the retrieved seq_ids
    // and fill the density map
    for (auto& d : data) {
        if (d.val != gap_val) {
            CBioseq_Handle& bsh = handles[d.index];
            if (bsh && bsh.CanGetInst_Mol()) {
                CSeqdesc_CI di (bsh, CSeqdesc::e_Molinfo);
                if (di && di->GetMolinfo().IsSetTech()) {
                    d.val = SGetSeqQuality(di->GetMolinfo().GetTech());
                }
            }
        }
        the_map->AddRange(d.range, d.val, false);
    }
    // no need for data
    data.clear();
    if (m_Cache) {
        x_WriteToCache(x_GetCacheKey(), *the_map);
        if (m_Range != range)
            the_map->SetRange(m_Range);
    }
    CRef<CSeqGlyph> glyph(new CSegmentSmearGlyph(the_map));
    result->m_ObjectList.push_back(glyph);

    perfLog.Post(CRequestStatus::e200_Ok);

    return eCompleted;
}


IAppJob::EJobState CSGSegmentSmearJob::x_LoadFromNA()
{
    CLogPerformance perfLog("CSGSegmentSmearJob::x_LoadFromNA");
    perfLog.AddParameter ("description", "Loading segment smear map from NA");
    CSGJobResult* result = new CSGJobResult();
    m_Result.Reset(result);

    typedef CSegmentSmearGlyph::TValueType TValType;
    CConstRef<CSeq_loc> loc =
        m_Handle.GetRangeSeq_loc(m_Range.GetFrom(), m_Range.GetTo());
    CAnnot_CI it(m_Handle.GetScope(), *loc, m_Sel);

    if (it) {
        TSeqPos     r_from = m_Cache ? 0 : m_Range.GetFrom();
        TSeqPos     r_to = m_Cache ? m_Handle.GetBioseqLength() - 1 : m_Range.GetTo();
        TSeqRange   range(r_from, r_to);

        auto_ptr<CSegmentSmearGlyph::CSegMap> the_map(
            new CSegmentSmearGlyph::CSegMap(r_from, r_to, (float)m_Scale,
                                            new max_func<TValType>));
        CTableFieldHandle<int> col_from(CSeqTable_column_info::eField_id_location_from);
        CTableFieldHandle<int> col_to(CSeqTable_column_info::eField_id_location_to);
        CTableFieldHandle<int> col_tech("data");
        for (; it; ++it) {
            CSeq_annot_Handle annot = *it;
            size_t rows = annot.GetSeq_tableNumRows();

            int from = 0;
            int to = 0;

            col_to.TryGet(annot, rows - 1, to);
            if ((TSeqPos)to < r_from) {
                continue;
            }

            col_from.TryGet(annot, 0, from);
            if (r_to < (TSeqPos)from) break;

            // for case that 'is_gap' column is stored as bit
            const CSeq_table::TColumns& cols =
                annot.GetCompleteSeq_annot()->GetData().GetSeq_table().GetColumns();
            const CSeqTable_column* col_gap = NULL;
            ITERATE (CSeq_table::TColumns, iter, cols) {
                if ((*iter)->GetHeader().CanGetField_name()  &&
                    (*iter)->GetHeader().GetField_name() == "is_gap") {
                    col_gap = iter->GetPointer();
                    break;
                }
            }
            if ( !col_gap ) return eFailed;

            // for case that 'is_gap' column is stored as types
            //vector<char> is_gaps;
            //CTableFieldHandle<int> col_gap("is_gap");
            //if ( !col_gap.TryGet(annot, 0, is_gaps) ) return eFailed;

            // find the start row
            int row = 0;
            int value = 0;
            int r_start = 0;
            int r_end = rows - 1;
            bool is_gap = true;
            do {
                row = (r_start + r_end) / 2;
                col_from.TryGet(annot, row, from);
                if ((TSeqPos)from < r_from )  r_start = row;
                else                          r_end = row;
            } while ((TSeqPos)from != r_from  &&  r_start < r_end - 1);


            for (row = (r_start + r_end) / 2;  (size_t)row < rows;  ++row) {
                if (col_from.TryGet(annot, row, from)  &&
                    col_to.TryGet(annot, row, to)  &&
                    col_tech.TryGet(annot, row, value) &&
                    col_gap->TryGetBool(row, is_gap)) {
                        if ((TSeqPos)from > r_to) break;
                        CSegmentConfig::ESequenceQuality qual = CSegmentConfig::eGap;
                        //if ( !is_gaps[row] > 0 ) {
                        if ( !is_gap ) {
                            qual = SGetSeqQuality((CMolInfo_Base::TTech)value);
                        }
                        the_map->AddRange(TSeqRange(from, to), (TValType)qual, false);
                }
            }
        }

        if (m_Cache) {
            x_WriteToCache(x_GetCacheKey(),*the_map);
            if (m_Range != range)
                the_map->SetRange(m_Range);
        }
        CRef<CSeqGlyph> hist(new CSegmentSmearGlyph(the_map));
        result->m_ObjectList.push_back(hist);
    }

    perfLog.Post(CRequestStatus::e200_Ok);

    return eCompleted;
}

IAppJob::EJobState CSGSegmentSmearJob::x_LoadFromNetCache(const string& key)
{
    _ASSERT(m_Cache);

    CLogPerformance perfLog("CSGSegmentSmearJob::x_LoadFromNetCache");
    perfLog.AddParameter ("description", "Loading segment smear map from NetCache");

    TSeqPos r_from = m_Range.GetFrom();
    TSeqPos r_to = m_Range.GetTo();

    auto_ptr<CSegmentSmearGlyph::CSegMap> the_map(
        new CSegmentSmearGlyph::CSegMap(r_from, r_to, (float)m_Scale,
        new max_func<CSegmentSmearGlyph::TValueType>));

    auto_ptr<IReader> reader(m_Cache->GetReadStream(key, 0, ""));
    the_map->Deserialize(*reader);

    CRef<CSeqGlyph> hist(new CSegmentSmearGlyph(the_map));
    CSGJobResult* result = new CSGJobResult();
    m_Result.Reset(result);
    result->m_ObjectList.push_back(hist);

    perfLog.Post(CRequestStatus::e200_Ok);

    return eCompleted;
}

void CSGSegmentSmearJob::x_WriteToCache(const string& key, const CSegmentSmearGlyph::CSegMap& density_map)
{
    _ASSERT(m_Cache);

    try {
        auto_ptr<IWriter> writer (m_Cache->GetWriteStream(key, 0, ""));
        density_map.Serialize(*writer);
    }
    catch (const CException& e) {
        LOG_POST(Warning << "CSGSegmentSmearJob::x_WriteToCache: "
            "failed to store map to cache: " << e.GetMsg());
    }
}

string CSGSegmentSmearJob::x_GetCacheKey() const
{
    CNcbiOstrstream os;

    CSeq_id_Handle best_idh = sequence::GetId(m_Handle, sequence::eGetId_Best);
    string name;
    best_idh.GetSeqId()->GetLabel(&name, CSeq_id::eContent);
    os << name;
    os << "_SM";

    return CNcbiOstrstreamToString(os);
}

///////////////////////////////////////////////////////////////////////////////
/// CSGSegmentsSeqIDJob implementation
///
CSGSegmentsSeqIDJob::CSGSegmentsSeqIDJob(const string& desc,
                                         CSeqGlyph::TObjects& objs,
                                         objects::CScope& scope,
                                         TJobToken token)
    : CSeqGraphicJob(desc)
    , m_Objs(objs)
    , m_Scope(scope)
{
    SetToken(token);
    SetTaskName("Resolving component seq-ids...");
}


IAppJob::EJobState CSGSegmentsSeqIDJob::x_Execute()
{
    CJobResultBase* result = new CJobResultBase();
    m_Result.Reset(result);
    result->m_Token = m_Token;
    SetTaskTotal(m_Objs.size());
    if (!m_Objs.empty()) {
        SetTaskCompleted(0);
        CScope::TIds ids;
        ids.reserve(m_Objs.size());
        for (auto&& iter : m_Objs) {
            if (IsCanceled())
                return eCanceled;
            CSeqGlyph* obj = const_cast<CSeqGlyph*>(iter.GetPointer());
            CSegmentGlyph* seq_map = dynamic_cast<CSegmentGlyph*>(obj);
            if (!seq_map)
                return eFailed;
            const CSeq_id* seq_id = seq_map->GetLocation().GetId();
            ids.push_back(CSeq_id_Handle::GetHandle(*seq_id));
        }
        SetTaskCompleted(ceil(0.25 * m_Objs.size()));
        _ASSERT(m_Objs.size() == ids.size());
        CScope::TBioseqHandles bshs = m_Scope.GetBioseqHandles(ids);
        _ASSERT(bshs.size() == ids.size());
        if (bshs.size() != ids.size())
            return eFailed;
        size_t i = 0;
        for (auto&& iter : m_Objs) {
            if (IsCanceled())
                return eCanceled;
            if (bshs[i] && bshs[i].CanGetInst_Mol()) {
                CSeqdesc_CI di(bshs[i], CSeqdesc::e_Molinfo);
                if (di  &&  di->GetMolinfo().IsSetTech()) {
                    CSeqGlyph* obj = const_cast<CSeqGlyph*>(iter.GetPointer());
                    CSegmentGlyph* seq_map = dynamic_cast<CSegmentGlyph*>(obj);
                    seq_map->SetSeqQuality(SGetSeqQuality(di->GetMolinfo().GetTech()));
                }
            }
            ++i;
        }
    }
    SetTaskCompleted(m_Objs.size());
    return eCompleted;
}


///////////////////////////////////////////////////////////////////////////////
/// CSGSwitchPointJob implementation
///

CSGSwitchPointJob::CSGSwitchPointJob(const string& desc, CBioseq_Handle handle)
    : CSeqGraphicJob(desc)
    , m_Handle(handle)
{}

IAppJob::EJobState CSGSwitchPointJob::x_Execute()
{
    CSGJobResult* result = new CSGJobResult();
    m_Result.Reset(result);

    CSeqGlyph::TObjects all_objs;

    CConstRef<CBioseq> bioseq = m_Handle.GetCompleteBioseq();
    if ( !bioseq->IsSetInst() )
        return eFailed;

    const CBioseq::TInst& inst = bioseq->GetInst();
    if ( !inst.IsSetHist() )
        return eFailed;

    const CSeq_hist& hist = inst.GetHist();
    if ( !hist.IsSetAssembly() )
        return eFailed;

    const CSeq_hist::TAssembly& assm = hist.GetAssembly();
    if (assm.empty()) // do nothing if no assembly at all
        return eFailed;

    TSeqMapSwitchPoints spoints = GetAllSwitchPoints(m_Handle);
    ITERATE(TSeqMapSwitchPoints, iter, spoints) {
        if (IsCanceled()) {
            return eCanceled;
        }

        CRef<CSeqMapSwitchPoint> one_point = *iter;

        CRef<CSeqGlyph> sp(new CSwitchPointGlyph(one_point));
        all_objs.push_back(sp);
    }  // ITERATE

    result->m_ObjectList.swap(all_objs);
    result->m_Token = m_Token;
    return eCompleted;
}



END_NCBI_SCOPE
