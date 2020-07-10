/*  $Id: alignment_smear.cpp 42102 2018-12-19 21:59:38Z shkeda $
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
 * Authors: Robert Smith, Liangshou Wu
 *
 * File Description:
 *
 */

#include <ncbi_pch.hpp>
#include <gui/objutils/alignment_smear.hpp>

#include <gui/objutils/label.hpp>
#include <gui/objutils/utils.hpp>
#include <objtools/alnmgr/aln_converters.hpp>
#include <objtools/alnmgr/sparse_aln.hpp>
#include <objtools/alnmgr/alnmap.hpp>
#include <objtools/alnmgr/aln_generators.hpp>

#include <objmgr/bioseq_handle.hpp>
#include <objmgr/annot_selector.hpp>
#include <objmgr/util/sequence.hpp>
#include <objects/seq/Seq_annot.hpp>
#include <objects/seq/Annot_descr.hpp>
#include <objects/seq/Annotdesc.hpp>
#include <objects/general/User_object.hpp>
#include <objects/general/User_field.hpp>
#include <objects/general/Object_id.hpp>

// need only for debug dumping
#include <serial/objostr.hpp>
#include <serial/serial.hpp>

#include <algorithm>

BEGIN_NCBI_SCOPE

USING_SCOPE(objects);

CAlignmentSmear::CAlignmentSmear(
        const objects::CBioseq_Handle& handle,
        TSeqPos start,
        TSeqPos stop,
        float window,
        EAlignSmearStrand strand_type
    )
    : m_BioseqHandle(handle),
    m_AccumSeg(start, stop, window),
    m_AccumGap(start, stop, window, new max_func<score_type>),
    m_StrandType(strand_type),
    m_CntAligns(0), m_CntMixes(0)
{
}


/// smear all the alignments in this annotation.
void CAlignmentSmear::AddAnnot(const CSeq_annot& seq_annot,
                               ISeqTaskProgressCallback* p_cb)
{
    if (p_cb) p_cb->SetTaskName("Loading alignments...");

    objects::SAnnotSelector sel = CSeqUtils::GetAnnotSelector();
    sel.SetLimitSeqAnnot(m_BioseqHandle.GetScope().GetSeq_annotHandle
                         (seq_annot))
       .SetSortOrder(SAnnotSelector::eSortOrder_None);

    AddAlignments(sel, p_cb);
}



class SAlignSmearSort
{
public:
    typedef CSeq_id_Handle TId;
    SAlignSmearSort(CConstRef<CSparseAln>& aln, const TId& id)
        : m_Id(id)
        , m_Start(aln->GetSeqStart(aln->GetAnchor()))
        , m_End(aln->GetSeqStop(aln->GetAnchor()))
        , m_Alignment(aln)
    {
    }

    SAlignSmearSort(TSeqPos start, TSeqPos end, const TId& id)
        : m_Id(id)
        , m_Start(start)
        , m_End(end)
    {}

    bool operator< (const SAlignSmearSort& rhs) const {
        if (m_Id < rhs.m_Id) return true;
        if (rhs.m_Id < m_Id) return false;
        if (m_Start < rhs.m_Start) return true;
        return false;
    }

    TId  m_Id;
    TSeqPos m_Start;
    TSeqPos m_End;
    CConstRef<CSparseAln>  m_Alignment;
};

void CAlignmentSmear::AddAlignments(const objects::SAnnotSelector& sel,
                                    ISeqTaskProgressCallback* p_cb)
{
    TSeqPos start(m_AccumSeg.GetStart());
    TSeqPos stop(m_AccumSeg.GetStop());

    // grab a alignment iterator for our range
    CAlign_CI align_iter(m_BioseqHandle.GetScope(),
                         *m_BioseqHandle.GetRangeSeq_loc(start, stop),
                         sel);

    AddAlignments(align_iter, p_cb);
}


bool CAlignmentSmear::AddAlignments(CAlign_CI& align_iter,
                                    ISeqTaskProgressCallback* p_cb)
{
    size_t size = align_iter.GetSize();
    if (size == 0) return true;

    typedef vector<const objects::CSeq_align*> TAlnVector;
    typedef CAlnIdMap<TAlnVector, TIdExtract> TAlnIdMap;
    typedef CAlnStats<TAlnIdMap> TAlnStats;

    if (p_cb) {
        p_cb->SetTaskName("Adding alignments...");
        p_cb->SetTaskTotal((int)size);
        p_cb->SetTaskCompleted(0);
    }

    string annot_name = x_GetAnnotName(
        *align_iter.GetSeq_align_Handle().GetAnnot().GetCompleteSeq_annot());

    // setting a threshold for alignment size (20 screen pixel)
    // if the size is smaller than the threshold, we only consider
    // the total range to ignore the unnoticeable detaisl. Doing
    // this can avoid creating alignment manager for a given alignment
    // and improve the smearing performance dramatically, especially 
    // for a large number of alignments.
    TSeqPos threshold = (TSeqPos)(20 * m_AccumSeg.GetWindow());

    // For all the alignments given by sel on this strand (m_StrandType)
    // Group alignments by their accession.
    // Sort the alignments by start location.
    vector<SAlignSmearSort> aligns_to_smear;

    for (int ai = 0; align_iter; ++align_iter, ++ai) {
        if (p_cb  &&  p_cb->StopRequested()) {
            return false;
        }
        const CSeq_align& align = *align_iter;

        try {
            CSeq_align::TDim num_row = align.CheckNumRows();

            if (num_row != 2) { // ignore non-pairwise alignments.
                CRef<CAlnMap> aln_map;
                if (align.GetSegs().IsDenseg()) {
                    aln_map.Reset(new CAlnMap(align.GetSegs().GetDenseg()));
                } else {
                    CRef<CSeq_align> dense_seg_aln = ConvertSeq_align(align, CSeq_align::TSegs::e_Denseg, -1, &m_BioseqHandle.GetScope());
                    _ASSERT(dense_seg_aln && dense_seg_aln->GetSegs().IsDenseg());
                    if (!dense_seg_aln || !dense_seg_aln->GetSegs().IsDenseg())
                        continue; // bummer!
                    aln_map.Reset(new CAlnMap(dense_seg_aln->GetSegs().GetDenseg()));
                }
                for (auto row = 0; row < aln_map->GetNumRows(); ++row) {
                    if ( m_BioseqHandle.IsSynonym(aln_map->GetSeqId(row)) ) {
                        aln_map->SetAnchor(row);
                        break;
                    }
                }
                AddAlignment(*aln_map);
                continue;
            }
            /*
            for (auto row = 0; row < num_row; ++row) {
                CSeq_id_Handle idh = sequence::GetId(align.GetSeq_id(row), m_BioseqHandle.GetScope(), sequence::eGetId_Best);
                if (m_BioseqHandle.IsSynonym(idh)) {
                    anchor = row;
                    break;
                }
            }
            */

            // which row of the alignment is the bioseq on?
            CAlnMap::TNumrow anchor = m_BioseqHandle.IsSynonym(align.GetSeq_id(0))? 0: 1;

            // What strand of our bioseq is this alignment on?
            // if not negative, we would like to think it as positive.
            if (m_StrandType == eSmearStrand_Pos  &&
                align.GetSeqStrand(anchor) == eNa_strand_minus) {
                continue;
            }
            if (m_StrandType == eSmearStrand_Neg  &&
                align.GetSeqStrand(anchor) != eNa_strand_minus) {
                continue;
            }
            CSeq_id_Handle id = CSeq_id_Handle::GetHandle(align.GetSeq_id(1 - anchor));
            TSeqRange range = align.GetSeqRange(anchor);
            if (range.GetLength() < threshold) {
                // optimization here
                // For alignments with small size, we won't create alignment
                // manager for then to speed up the smearing process.
                aligns_to_smear.emplace_back(range.GetFrom(), range.GetTo(), id);
            } else {
                CAlnSeqIdsExtract<CAlnSeqId> id_extract;
                TAlnIdMap aln_id_map(id_extract, 1);
                aln_id_map.push_back(align);

                // convert to an CSparseAln
                // Create align statistics object
                TAlnStats aln_stats(aln_id_map);
                // Create user options
                CAlnUserOptions aln_user_options;
                aln_user_options.m_Direction = CAlnUserOptions::eBothDirections;
                aln_user_options.SetAnchorId(aln_id_map[0][anchor]);
                // Create an anchored align
                CRef<CAnchoredAln> anchored_aln =
                    CreateAnchoredAlnFromAln(aln_stats, 0, aln_user_options);
                // Build a sparse align
                CConstRef<CSparseAln> sparse_aln(
                    new CSparseAln(*anchored_aln, m_BioseqHandle.GetScope()));
                aligns_to_smear.emplace_back(sparse_aln, id);

                /*
                for (int i = 0; i < (int)aln_id_map.size(); i++) {

                    CRef<CAnchoredAln> anchored_aln = CreateAnchoredAlnFromAln(aln_stats,
                                                                               i, aln_user_options);
                    CConstRef<CSparseAln> sparse_aln(
                        new CSparseAln(*anchored_aln, m_BioseqHandle.GetScope()));

                    aligns_to_smear.emplace_back(sparse_aln, id_str);
                }
                */
            }
            ++m_CntAligns;
        } catch (std::exception&) {
            // ignore this alignment
        }

        if (p_cb) {
            p_cb->AddTaskCompleted(1);
        }
    }

    sort(aligns_to_smear.begin(), aligns_to_smear.end());

    if (p_cb) {
        p_cb->SetTaskName("Packing alignments...");
        p_cb->SetTaskTotal((int)size);
        p_cb->SetTaskCompleted(0);
    }

    vector<SAlignSmearSort>::const_iterator aligns_end = aligns_to_smear.end();
    vector<SAlignSmearSort>::const_iterator align_it, last_align = aligns_end;
    for (align_it = aligns_to_smear.begin();
         align_it != aligns_end;
         last_align = align_it, ++align_it) {

        if (p_cb  &&  p_cb->StopRequested()) {
            return false;
        }

        if (last_align != aligns_end  &&
            last_align->m_Id != align_it->m_Id ) {
            ++ m_CntMixes;
        }

        // if the last alignment and this one have the same accession and
        // this alignments start is past the end of the last one
        // then add the space between them as a gap.
        if (last_align != aligns_end  &&
            last_align->m_Id == align_it->m_Id  &&
            last_align->m_End < align_it->m_Start ) {
            m_AccumGap.AddRange(TSeqRange(last_align->m_End , align_it->m_Start));
        }

        // AddAlignment for each
        if ( !align_it->m_Alignment ) {
            m_AccumSeg.AddRange(TSeqRange(align_it->m_Start, align_it->m_End), 1);
        } else {
            AddAlignment(*align_it->m_Alignment);
        }

        if (p_cb) {
            p_cb->AddTaskCompleted(1);
        }
    }

    // So far we only count in the ids that are different from the prevous
    // ones. We need to add one more which is the first one.
    ++m_CntMixes;

    // take out gaps in the middle of blocks.
    MaskGaps();

    // now if there is a chance we would put more than one annotation
    // in a smear we need to join their names together here, or something else?
    SetLabel(annot_name + " (" + NStr::SizetToString(m_CntAligns) +
        " aligns, " + NStr::SizetToString(m_CntMixes) +
        (m_CntMixes > 1 ? " ids)" : " id)"));
        
    if (p_cb) {
        p_cb->SetTaskCompleted((int)size);
    }

    return true;
}


void CAlignmentSmear::AddAlignment(const CSparseAln& aln)
{
    CAlnMap::TSignedRange range(aln.GetAlnRange());

    // which row of the alignment is the bioseq on.
    CAlnMap::TNumrow bs_row = aln.GetAnchor();

    // process all the other rows in the alignment
    for (CAlnMap::TNumrow row = 0; row < aln.GetNumRows(); ++row) {
        // ignore the row on the alignment representing the bioseq.
        if (bs_row == row)
            continue;

        auto_ptr<IAlnSegmentIterator> p_it(aln.CreateSegmentIterator(
            row, range, IAlnSegmentIterator::eSkipGaps));

        TSeqPos pre_stop = (*p_it)->GetAlnRange().GetTo();
        for (IAlnSegmentIterator& it(*p_it);  it;  ++it) {
            const IAlnSegment& seg = *it;
            TSeqPos start = seg.GetAlnRange().GetFrom();
            TSeqPos stop  = seg.GetAlnRange().GetTo();

            m_AccumGap.AddRange(TSeqRange(pre_stop, start));
            m_AccumSeg.AddRange(TSeqRange(start, stop), 1);
            pre_stop = stop;
        }
    }
}


void CAlignmentSmear::AddAlignment(const CAlnMap& aln_map)
{

    CAlnMap::TSignedRange range(aln_map.GetAlnStart(),
                                aln_map.GetAlnStop());

    // which row of the alignment is the bioseq on.
    CAlnMap::TNumrow bs_row = aln_map.GetAnchor();

    // process all the other rows in the alignment
    for (CAlnMap::TNumrow row = 0; row < aln_map.GetNumRows(); ++row) {
        // ignore the row on the alignment representing the bioseq.
        if (bs_row == row)
            continue;

        CRef<CAlnMap::CAlnChunkVec> aln_chunks
                (aln_map.GetAlnChunks(row, range,
                           CAlnMap::fSeqOnly | CAlnMap::fChunkSameAsSeg ));

        // Smear all the segments.
        for (int i = 0;  i < aln_chunks->size();  ++i) {
            CConstRef<CAlnMap::CAlnChunk> chunk((*aln_chunks)[i]);

            TSeqPos start = chunk->GetRange().GetFrom();
            start = aln_map.GetSeqPosFromSeqPos(bs_row, row, start);

            TSeqPos stop = chunk->GetRange().GetTo();
            stop = aln_map.GetSeqPosFromSeqPos(bs_row, row, stop);

            m_AccumSeg.AddRange(TSeqRange(start, stop), 1);
            ++m_CntMixes;
        }

        // Smear the gaps.
        for (int i = 1;  i < aln_chunks->size();  ++i) {
            CConstRef<CAlnMap::CAlnChunk> prev_chunk((*aln_chunks)[i-1]);
            CConstRef<CAlnMap::CAlnChunk> chunk((*aln_chunks)[i]);

            TSeqPos start = prev_chunk->GetRange().GetTo();
            start = aln_map.GetSeqPosFromSeqPos(bs_row, row, start);

            TSeqPos stop = chunk->GetRange().GetFrom();
            stop = aln_map.GetSeqPosFromSeqPos(bs_row, row, stop);

            m_AccumGap.AddRange(TSeqRange(start, stop));
        }
    }
}


// erase gaps where there are segments.
void CAlignmentSmear::MaskGaps()
{
    TSegMap::iterator seg_it = m_AccumSeg.begin();
    TGapMap::iterator gap_it = m_AccumGap.begin();
    for (; gap_it != m_AccumGap.end(); ++gap_it, ++seg_it  ) {
        if (*gap_it > 0   &&  *seg_it > 0) {
            *gap_it = 0;
        }
    }

}


string CAlignmentSmear::GetLabel() const
{
    return m_Label;
}


void CAlignmentSmear::SetLabel(const string& label)
{
    m_Label = label;
}



bool CAlignmentSmear::SeparateStrands(const objects::CSeq_annot& seq_annot)
{
    string name = x_GetAnnotName(seq_annot);

    if (name == "BLASTX - swissprot"  ||
        name == "BLASTN - mrna" ) {
        return true;
    }
    return false;
}



static const string s_BlastFieldKey("Blast Type");
static const string s_HistFieldKey("Hist Seqalign");


string CAlignmentSmear::x_GetAnnotName(const objects::CSeq_annot& seq_annot)
{
    string label;
    string annot_name;
    string blast_type;
    string hist_type;

    if (seq_annot.IsSetDesc()) {
        ITERATE ( CSeq_annot::TDesc::Tdata, it,
                  seq_annot.GetDesc().Get() ) {
            const CAnnotdesc& desc = **it;
            if (desc.Which() == CAnnotdesc::e_Name ) {
                annot_name = desc.GetName();
            }
            if (desc.Which() == CAnnotdesc::e_User) {
                const CUser_object& user_obj = desc.GetUser();

                if (user_obj.CanGetType()  &&
                    user_obj.GetType().Which() == CObject_id::e_Str ) {

                    if (user_obj.GetType().GetStr()  ==  s_BlastFieldKey) {
                        const CUser_field& user_field = * user_obj.GetData().front();

                        if (user_field.CanGetData()  &&
                            user_field.GetData().Which() == CUser_field::TData::e_Int &&
                            user_field.CanGetLabel()) {
                            const CObject_id& id = user_field.GetLabel();

                            if (id.Which() == CObject_id::e_Str ) {
                                blast_type = id.GetStr();
                            }
                        }
                    }

                    if (user_obj.GetType().GetStr()  ==  s_HistFieldKey  ) {
                        const CUser_field& user_field = * user_obj.GetData().front();
                        if (user_field.CanGetData()  &&
                            user_field.GetData().Which() == CUser_field::TData::e_Bool &&
                            user_field.GetData().GetBool()  &&
                            user_field.CanGetLabel()) {
                            const CObject_id& id = user_field.GetLabel();
                            if (id.Which() == CObject_id::e_Str ) {
                                hist_type = id.GetStr();
                            }
                        }
                    }
                }
            }
        }
        if ( ! annot_name.empty()) {
            label = annot_name;
        } else if ( ! blast_type.empty() ) {
            label = blast_type;
        } else if ( ! hist_type.empty() ) {
            label = hist_type;
        }
    }
    return label;
}


END_NCBI_SCOPE
