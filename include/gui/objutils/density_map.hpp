#ifndef GUI_UTILS___DENSITY_GRAPH__HPP
#define GUI_UTILS___DENSITY_GRAPH__HPP

/*  $Id: density_map.hpp 43674 2019-08-14 13:57:05Z katargir $
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

#include <corelib/ncbistd.hpp>
#include <objmgr/bioseq_handle.hpp>
#include <objmgr/feat_ci.hpp>
#include <objmgr/align_ci.hpp>
#include <objmgr/annot_ci.hpp>
#include <objtools/alnmgr/alnmap.hpp>
#include <objtools/alnmgr/alnmix.hpp>
#include <objects/seqloc/Seq_loc.hpp>

#include <util/range.hpp>
#include <util/range_coll.hpp>
#include <deque>

/** @addtogroup GUI_UTILS
*
* @{
*/


BEGIN_NCBI_SCOPE

/// TEMPLATE binary functor base struct
template<class Type>
struct binary_functor : public binary_function<Type, Type, Type>
{
    virtual ~binary_functor() {}

    virtual Type operator() (const Type& left, const Type& right) const = 0;
    virtual binary_functor<Type>* clone() const = 0;
};

/// TEMPLATE STRUCT max
template<class Type>
struct plus_func
    : public binary_functor<Type>
{   // functor for addition
    plus_func() {}

    virtual Type operator()(const Type& left, const Type& right) const
    {   // apply operator+ to operands
        return left + right;
    }

    virtual binary_functor<Type>* clone() const
    {
        return new plus_func<Type>;
    }
};

/// TEMPLATE STRUCT max
template<class Type>
struct max_func
    : public binary_functor<Type>
{   // functor for max
    virtual Type operator()(const Type& left, const Type& right) const
    {   // apply operator+ to operands
        return left > right ? left : right;
    }

    virtual binary_functor<Type>* clone() const
    {
        return new max_func<Type>;
    }
};

/// TEMPLATE STRUCT min
template<class Type>
struct min_func
    : public binary_functor<Type>
{   // functor for min
    Type operator()(const Type& left, const Type& right) const
    {   // apply operator+ to operands
        return left < right ? left : right;
    }

    virtual binary_functor<Type>* clone() const
    {
        return new min_func<Type>;
    }
};



/// Run iterator.  iterate through runs of equal values in the bins.

template <typename CntType>
class CDenMapRunIterator {
public:
    typedef vector<CntType> container_type;
    typedef typename container_type::size_type   position_type;

    CDenMapRunIterator(position_type here,
        const container_type& bins,
        TSeqPos start, double window)
        : m_Bins(bins),
        m_Pos(here),
        m_RunLength(x_CalcRunLength()),
        m_Start(start),
        m_Window(window)
    {
    }

    CntType       Advance();

    position_type GetPosition() const { return m_Pos; }
    position_type GetRunLength() const{ return m_RunLength; }
    TSeqPos       GetSeqPosition() const;
    TSeqPos       GetSeqRunEndPosition() const;
    TSeqPos       GetSeqRunLength() const;

    CntType       GetValue() const { return Valid() ? m_Bins[m_Pos]: 0 ; }
    
    bool operator ==(const CDenMapRunIterator<CntType>& rhs) const { return m_Pos == rhs.m_Pos; }

    bool        Valid() const { return  m_Pos >= 0 &&  m_Pos < m_Bins.size() && m_RunLength != 0; }
    DECLARE_OPERATOR_BOOL(Valid());

private:


    position_type   x_CalcRunLength() const;

    const container_type&   m_Bins;
    position_type       m_Pos;
    position_type       m_RunLength;

    const TSeqPos       m_Start;
    const double        m_Window;


};

template <typename CntType>
TSeqPos CDenMapRunIterator<CntType>::GetSeqPosition() const
{
    return TSeqPos(GetPosition() * m_Window + 0.5) + m_Start;
}

template <typename CntType>
TSeqPos CDenMapRunIterator<CntType>::GetSeqRunEndPosition() const
{
    return TSeqPos((GetPosition() + GetRunLength()) * m_Window + 0.5) + m_Start;
}


template <typename CntType>
TSeqPos CDenMapRunIterator<CntType>::GetSeqRunLength() const
{
    return TSeqPos(GetRunLength() * m_Window + 0.5);
}


template <typename CntType>
typename CDenMapRunIterator<CntType>::position_type
CDenMapRunIterator<CntType>::x_CalcRunLength() const
{
    if (m_Pos >= 0 &&  m_Pos < m_Bins.size()) {
        CntType this_value = m_Bins[m_Pos];
        position_type i;
        for (i = m_Pos + 1;
            i < m_Bins.size()  &&  m_Bins[i] == this_value;
            ++i) {}

        return i - m_Pos;
    }
    return 0;
}

template <typename CntType>
CntType
CDenMapRunIterator<CntType>::Advance() {
    m_Pos += m_RunLength;
    m_RunLength = x_CalcRunLength();
    return GetValue();
}


///
/// class CDensityMap generates a low-resolution view of a set of features.
/// The features to be processed are defined by the SAnnotSelector object.
/// The return value is a vector of counts, one for each of the bins
/// defined by the window width and spanning the range from start to stop.
/// If start and stop are both zero, then the entire sequence is evaluated.
///


template <typename CntType>
class CDensityMap
{
public:
    typedef vector<CntType> container_type;
    typedef typename container_type::const_iterator const_iterator;
    typedef typename container_type::iterator       iterator;
    typedef CDenMapRunIterator<CntType>             runlen_iterator;
    typedef binary_functor<CntType>                 accum_functor;
    typedef vector<size_t> TDataPoints;

    CDensityMap(TSeqPos start = 0, TSeqPos stop = 0, double window = 1,
                accum_functor* func = NULL, CntType def = (CntType)0);
    CDensityMap(const objects::CBioseq_Handle& handle, double window = 1,
                accum_functor* func = NULL, CntType def = (CntType)0);

    CDensityMap(const CDensityMap<CntType>& map);
    CDensityMap<CntType>& operator=(const CDensityMap<CntType>& map);
    virtual ~CDensityMap() {}

    virtual void AddRange(TSeqRange range, CntType score = 1, bool expand = false);

    virtual void AddRanges(CRangeCollection<TSeqPos> ranges,
                   CntType score = 1, bool expand = false);

    CntType AddLocation(const objects::CSeq_loc& loc);

    CntType AddFeature(const objects::CSeq_feat& feature);

    /// All features on this bioseq selected by sel in the range of this.
    CntType AddFeatures(const objects::CBioseq_Handle& handle,
        objects::SAnnotSelector sel);

    /// All alignments on this bioseq selected by sel in the range of this.
    CntType AddAlignments(const objects::CBioseq_Handle& handle,
        objects::SAnnotSelector sel);

    /// All alignments in a given annotation on this bioseq within the range of this.
    CntType AddAlignments(const objects::CBioseq_Handle& handle,
        const objects::CSeq_annot& seq_annot);

    void        Clear()
    {
        m_Max = m_DefVal;
        m_Min = m_DefVal;
        fill(m_Bins.begin(), m_Bins.end(), m_DefVal);
    }
    TSeqPos     GetStart() const { return m_Range.GetFrom(); }
    TSeqPos     GetStop() const { return m_Range.GetTo(); }
    TSeqRange   GetRange() const { return m_Range; }
    double       GetWindow() const { return m_Window; }
    size_t      GetBins() const { return m_Bins.size(); }
    CntType     GetMax() const { return m_Max; }
    CntType     GetMin() const { return m_Min; }
    CntType     GetDefVal() const { return m_DefVal; }
    void SetMax(const CntType& max) { m_Max = max; }
    void SetMin(const CntType& min) { m_Min = min; }

    const accum_functor* GetAccum() const { return m_AccumFunc.get(); }

    /// extend our density map to cover the sequence position stop.
    /// can only be used to extend to the right, that is only Stop is affected, not Start.
    void            ExtendTo(TSeqPos stop);

    const_iterator  begin() const { return m_Bins.begin(); }
    const_iterator  end() const { return m_Bins.end(); }
    iterator        begin()  { return m_Bins.begin(); }
    iterator        end()  { return m_Bins.end(); }

    runlen_iterator RunLenBegin() const { return RunLenIterator(0); }
    runlen_iterator RunLenIterator(typename container_type::size_type n) const
    { return runlen_iterator(n, m_Bins, m_Range.GetFrom(), m_Window); }

    CntType operator[](typename container_type::size_type n) const { return m_Bins[n]; }
    CntType& operator[](typename container_type::size_type n) { return m_Bins[n]; }



    /// OLD static method. Use AddFeatures method instead.
    /// retrieve a density map.  The return value is the maximum value
    /// in the density graph.
    static TSeqPos GetDensityMap(const objects::CBioseq_Handle& handle,
        TSeqPos start, TSeqPos stop,
        TSeqPos window,
        objects::SAnnotSelector sel,
        vector<TSeqPos>& density);


protected:
    /// given the range and window size, how many bins should there be?
    size_t  x_CalcNbins()
    {
        return TSeqPos((m_Range.GetToOpen() - m_Range.GetFrom()) / m_Window);
    }

    /// convert from sequence coords to a bin number.
    size_t  x_BinN(TSeqPos p)
    {
        return size_t((p - m_Range.GetFrom())/m_Window);
    }

    /// closed range on a sequence this covers.
    TSeqRange       m_Range;

    /// coordinates per bin.
    double           m_Window;

    /// Default value.
    CntType         m_DefVal;

    /// maximum Count accumulated in the bins so far.
    CntType         m_Max;

    /// Smallest count in a bin.
    CntType         m_Min;

    /// Where we actually keep the accumulated counts/scores/whatever.
    container_type  m_Bins;

    auto_ptr<accum_functor> m_AccumFunc;

};



///
///
/////////////////////////////////////////////////////////////////////////////
///
///

template <typename CntType>
CDensityMap<CntType>::CDensityMap(TSeqPos start,
                                  TSeqPos stop,
                                  double window /* = 1 */,
                                  accum_functor* func /*= NULL */,
                                  CntType def /*= (CntType)0*/)
    : m_Range(start, stop)
    , m_Window(window)
    , m_DefVal(def)
    , m_Max(def)
    , m_Min(def)
    , m_Bins(x_CalcNbins(), def)
    , m_AccumFunc(func ? func : new plus_func<CntType>)
{
}


template <typename CntType>
CDensityMap<CntType>::CDensityMap(const objects::CBioseq_Handle& handle,
                                  double window /* = 1 */,
                                  accum_functor* func /*= new plus_func<CntType> */,
                                  CntType def /*= (CntType)0*/)
    : m_Range(0, handle.GetSeqVector().size())
    , m_Window(window)
    , m_DefVal(def)
    , m_Max(def)
    , m_Min(def)
    , m_Bins(x_CalcNbins(), def)
    , m_AccumFunc(func ? func : new plus_func<CntType>)
{
}


template <typename CntType>
CDensityMap<CntType>::CDensityMap(const CDensityMap<CntType>& map)
    : m_Range(map.m_Range)
    , m_Window(map.m_Window)
    , m_DefVal(map.m_DefVal)
    , m_Max(map.m_Max)
    , m_Min(map.m_Min)
    , m_Bins(map.m_Bins)
    , m_AccumFunc(map.m_AccumFunc->clone())
{
}


template <typename CntType>
CDensityMap<CntType>& CDensityMap<CntType>::operator=(const CDensityMap<CntType>& map)
{
    // check self assignment
    if (this == &map)
        return *this;

    m_Range = map.m_Range;
    m_Window = map.m_Window;
    m_DefVal = map.m_DefVal;
    m_Max = map.m_Max;
    m_Min = map.m_Min;
    m_Bins = map.m_Bins;
    m_AccumFunc.reset(map.m_AccumFunc->clone());
    return *this;
}


template <typename CntType>
void  CDensityMap<CntType>::ExtendTo(TSeqPos stop)
{
    if (stop > GetStop()) {
        m_Range.SetTo(stop);
        m_Bins.resize(x_CalcNbins(), m_DefVal);
    }
}


template <typename CntType>
void CDensityMap<CntType>::AddRange(TSeqRange range, CntType score, bool expand)
{
    //_ASSERT(range.GetFrom() <= range.GetTo());
    if (range.GetFrom() > range.GetTo()) {
        range = TSeqRange(range.GetTo(), range.GetFrom());
    }

    if ( expand ) {
        ExtendTo( range.GetTo());
    }

    TSeqRange usable_range(m_Range.IntersectionWith(range));
    if (usable_range.Empty()) {
        return;
    }

    m_Max = max(m_Max, score);
    m_Min = min(m_Min, score);

    size_t begin_bin = x_BinN(usable_range.GetFrom());
    size_t end_bin   = begin_bin;
    if (m_Window > 1.0) {
        end_bin = x_BinN(usable_range.GetTo()) + 1;
    } else {
        end_bin = x_BinN(usable_range.GetToOpen());
    }
    end_bin = min(end_bin, m_Bins.size());

    for (size_t i = begin_bin;  i < end_bin ;  ++i) {
        CntType& new_val = m_Bins[i];
        new_val = (*m_AccumFunc)(new_val, score);
        m_Max = max(m_Max, new_val);
        m_Min = min(m_Min, new_val);
    }

}

template <typename CntType>
void CDensityMap<CntType>::AddRanges(CRangeCollection<TSeqPos> ranges,
                                     CntType score, bool expand)
{
    if ( expand ) {
        ExtendTo( ranges.GetTo());
    }

    ranges.IntersectWith(m_Range);
    if (ranges.Empty()) {
        return;
    }
    m_Max = max(m_Max, score);
    m_Min = min(m_Min, score);

    size_t previous_end_bin = 0;
    ITERATE (CRangeCollection<TSeqPos>, range_it, ranges) {
        size_t begin_bin = x_BinN(range_it->GetFrom());
        size_t end_bin   = begin_bin;
        if (m_Window > 1.0) {
            end_bin = x_BinN(range_it->GetTo()) + 1;
        } else {
            end_bin = x_BinN(range_it->GetToOpen());
        }

        begin_bin = max(begin_bin, previous_end_bin);
        end_bin = min(end_bin, m_Bins.size());
        previous_end_bin = end_bin;
        for (size_t i = begin_bin;  i < end_bin ;  ++i) {
            CntType& new_val = m_Bins[i];
            new_val = (*m_AccumFunc)(new_val, score);
            m_Max = max(m_Max, new_val);
            m_Min = min(m_Min, new_val);
        }
    }
}


template <typename CntType>
CntType CDensityMap<CntType>::AddLocation(const objects::CSeq_loc& loc)
{
    CRangeCollection<TSeqPos> ranges;
    for( objects::CSeq_loc_CI it(loc); it; ++it) {
        ranges += it.GetRange();
    }
    AddRanges(ranges, 1, false);
    return GetMax();
}


template <typename CntType>
CntType CDensityMap<CntType>::AddFeature(const objects::CSeq_feat& feature)
{
    return AddLocation(feature.GetLocation());
}


template <typename CntType>
CntType CDensityMap<CntType>::AddFeatures(
    const objects::CBioseq_Handle& handle,
    objects::SAnnotSelector sel)
{
    TSeqPos start(GetStart());
    TSeqPos stop(GetStop());

    /// eliminate some expensive functions
    sel.SetSortOrder(objects::SAnnotSelector::eSortOrder_None);

    // grab a feature iterator for our range
    CConstRef<objects::CSeq_loc> loc(handle.GetRangeSeq_loc(start, stop));
    objects::CFeat_CI feat_iter(handle.GetScope(), *loc, sel);
    if (feat_iter.GetSize() == 0) {
        return GetMax();
    }

    for (;  feat_iter;  ++feat_iter) {
        objects::CMappedFeat feat = *feat_iter;
        //TSeqRange range = feat.GetLocation().GetTotalRange();
        //AddRange(range, 1, false);
        AddLocation(feat.GetLocation());
    }
    return GetMax();
}


template <typename CntType>
CntType CDensityMap<CntType>::AddAlignments(
    const objects::CBioseq_Handle& handle,
    objects::SAnnotSelector sel)
{
    CRange<TSeqPos> range(GetStart(), GetStop());


    // grab a feature iterator for our range
    objects::CAlign_CI align_iter(handle, range, sel);

    for (size_t ai = 0;  align_iter;  ++align_iter, ++ai) {
        const objects::CSeq_align& align = *align_iter;

        if (! align.CanGetDim()) {
            _TRACE("Dimension not set. " << ai);
            continue;
        }
        int dim = align.GetDim();
        if (dim != 2) {
            _TRACE("Dimension not 2. " << ai);
            continue;
        }

        /*
        CAlnMix mix(handle.GetScope());
        mix.Add(align);
        mix.Merge(CAlnMix::fGapJoin);
        CRef<CAlnMap> aln_map
        (new CAlnMap(mix.GetDenseg()));
        */

        CRef< objects::CAlnMap> aln_map;
        if (align.GetSegs().IsStd()) {
            _TRACE(ai << ": Std seg");
            CRef<objects::CSeq_align> ds_align
                = align.CreateDensegFromStdseg();
            aln_map =  new objects::CAlnMap( ds_align->GetSegs().GetDenseg());
            continue;
        } else if (align.GetSegs().IsDenseg()) {
            aln_map = new objects::CAlnMap(align.GetSegs().GetDenseg());
            _TRACE(ai << ": Dense seg");
        }


        {{
            string all_ids = ": ";
            for (objects::CAlnMap::TDim di = 0;  di < aln_map->GetNumRows();
                ++di) {
                    all_ids += aln_map->GetSeqId(di).AsFastaString() + ", ";
            }
            _TRACE(all_ids);;
        }}

        TSeqRange range_final(aln_map->GetSeqStart(0), aln_map->GetSeqStop(0));
        AddRange(range_final, 1, false);
    }

    return GetMax();

}



template <typename CntType>
CntType CDensityMap<CntType>::AddAlignments(
    const objects::CBioseq_Handle& handle,
    const objects::CSeq_annot& seq_annot)
{
    CRange<TSeqPos> range(GetStart(), GetStop());

    objects::SAnnotSelector sel;
    objects::CSeq_annot_Handle sah
        = handle.GetScope().GetSeq_annotHandle(seq_annot);
    sel.SetLimitSeqAnnot(sah)
        .SetSortOrder(objects::SAnnotSelector::eSortOrder_None);

    // grab an alignment iterator for our range
    objects::CAlign_CI align_iter(handle, range, sel);

    objects::CAlnMix mix(handle.GetScope());
    for (int ai = 0; align_iter; ++align_iter, ++ai) {
        const objects::CSeq_align& align = *align_iter;

        // check number of dimensions
        if (! align.CanGetDim()  ||  align.GetDim() != 2) {
            // _TRACE("Dimension not 2. " << ai);
            continue;
        }

        // conver to a AlnMap.
        CRef<objects::CAlnMap> aln_map;
        if (align.GetSegs().IsStd()) {
            // _TRACE(ai << ": Std seg" );
            continue; // IGNORE std segs for now.
            try {
                CRef<objects::CSeq_align> ds_align =
                    align.CreateDensegFromStdseg();
                aln_map =
                    new objects::CAlnMap(ds_align->GetSegs().GetDenseg());
                {{
                    string all_ids = ": ";
                    for (objects::CAlnMap::TDim di = 0;
                        di < aln_map->GetNumRows();  ++di) {
                            all_ids += aln_map->GetSeqId(di).AsFastaString() + ", ";
                    }
                    _TRACE(all_ids);
                }}
            } catch (const objects::CSeqalignException& /*e*/) {
                // _TRACE(" : FAILED! " << e.what());
                continue;
            }
        } else if (align.GetSegs().IsDenseg()) {
            mix.Add(align);//, CAlnMix::fDontUseObjMgr);
            mix.Merge();
            aln_map = new objects::CAlnMap(mix.GetDenseg());
            //aln_map = new CAlnMap(align.GetSegs().GetDenseg());
            if (aln_map->GetNumSegs() < 2)
                continue;
            _TRACE(ai << ": Dense seg");
        }


        // find out what row our bioseq is on.
        objects::CAlnMap::TNumrow row = 0;
        objects::CAlnMap::TNumrow anchor = 0;
        for (row = 0;  row != aln_map->GetNumRows();  ++row) {
            if ( handle.IsSynonym(aln_map->GetSeqId(row)) ) {
                aln_map->SetAnchor(row);
                anchor = row;
                break;
            }
        }
        if (row == aln_map->GetNumRows()) {
            continue;
        }
        // works since dimension always 2.
        //objects::CAlnMap::TNumrow other_row = 1 - row;

        /**
        // total range
        _TRACE(" (" << aln_map->GetSeqStart(row) << "," << aln_map->GetSeqStop(row) << ")");
        // IDs
        {{
        _TRACE(" Segs: " << aln_map->GetNumSegs());
        string all_ids = ": ";
        for (CAlnMap::TDim di = 0; di < aln_map->GetNumRows(); ++di) {
        all_ids += aln_map->GetSeqId(di).AsFastaString() + ", ";
        }
        _TRACE(all_ids);
        }}
        // Segments.
        _TRACE("            ");
        for (CAlnMap::TNumseg s = 0; s < aln_map->GetNumSegs(); ++s) {
        // if (aln_map->GetSegType(row, s) && CAlnMap:: )
        _TRACE(" (" << aln_map->GetStart(row, s) << "," << aln_map->GetStop(row, s) << ")");
        }
        // Chunks
        _TRACE("     Chunks: "  );
        CRef<CAlnMap::CAlnChunkVec> chunks =
        aln_map->GetSeqChunks(row, CAlnMap::TSignedRange(start, stop), CAlnMap::fAlnSegsOnly);
        for (CAlnMap::TNumchunk c = 0; c < chunks->size(); ++c) {
        CConstRef<CAlnMap::CAlnChunk> chunk = (*chunks)[c];
        _TRACE("(" << chunk->GetRange().GetFrom() << "," << chunk->GetRange().GetTo() << ") ");
        }
        **/

        /*
        TSeqRange range(aln_map->GetSeqRange(row));
        TSeqRange other_range(aln_map->GetSeqRange(other_row));
        if (GetRange().IntersectingWith(range)) {
        AddRange(range, 1, false);
        }
        */
    }
    /*
    mix.Merge();
    CRef< CAlnMap> aln_map;
    aln_map = new CAlnMap(mix.GetDenseg());
    */
    return GetMax();
}



template <typename CntType>
TSeqPos CDensityMap<CntType>::GetDensityMap(const objects::CBioseq_Handle& handle,
                                            TSeqPos start, TSeqPos stop,
                                            TSeqPos window,
                                            objects::SAnnotSelector sel,
                                            vector<TSeqPos>& density)
{
    // set up our bins to handle pooled feature counts
    if (stop == start  &&  stop == 0) {
        objects::CSeqVector vec = handle.GetSeqVector();
        stop = vec.size();
    }

    // grab a feature iterator for our range
    objects::CFeat_CI feat_iter(handle.GetScope(),
        *handle.GetRangeSeq_loc(start, stop),
        sel);
    if (feat_iter.GetSize() == 0) {
        return 0;
    }

    size_t bins = (stop - start) / window;
    if (bins * window < stop - start) {
        ++bins;
    }

    density.resize(bins, 0);

    TSeqPos max_val = 0;

    // we keep a temporary list of mapped features - we use these
    // the screen what's in our visible range
    deque<objects::CMappedFeat> feats;
    TSeqPos bin_count = 0;
    NON_CONST_ITERATE(vector<TSeqPos>, bin_iter, density) {
        TSeqPos bin_start = start + bin_count * window;
        TSeqPos bin_stop = bin_start + window;

        // accumulate features for this bin
        if (feat_iter) {
            objects::CMappedFeat feat = *feat_iter;
            TSeqRange range = feat.GetLocation().GetTotalRange();

            while (range.GetFrom() < bin_stop) {
                feats.push_back(feat);

                ++feat_iter;
                if ( !feat_iter ) {
                    break;
                }
                feat = *feat_iter;
                range = feat.GetLocation().GetTotalRange();
            }
        }

        // remove features outside this bin
        while (feats.size()) {
            objects::CMappedFeat& feat = feats.front();
            TSeqRange range = feat.GetLocation().GetTotalRange();

            if (range.GetTo() < bin_start) {
                feats.pop_front();
            } else {
                break;
            }
        }

        // store our count and go to the next bin
        *bin_iter = feats.size();
        max_val = max(*bin_iter, max_val);
        ++bin_count;
    }

    return max_val;
}


END_NCBI_SCOPE

/* @} */

#endif  // GUI_UTILS___DENSITY_GRAPH__HPP
