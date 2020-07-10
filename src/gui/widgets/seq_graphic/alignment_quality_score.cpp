/*  $Id: alignment_quality_score.cpp 43687 2019-08-14 17:03:01Z katargir $
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
 * File Description:
 *
 */

#include <ncbi_pch.hpp>
#include <objmgr/graph_ci.hpp>
#include <objmgr/seq_loc_mapper.hpp>
#include <objects/seqres/Byte_graph.hpp>
#include <objects/seqres/Int_graph.hpp>
#include <objects/seqres/Real_graph.hpp>

#include <gui/widgets/seq_graphic/alignment_quality_score.hpp>
#include <gui/objutils/utils.hpp>

BEGIN_NCBI_SCOPE
USING_SCOPE(objects);

///////////////////////////////////////////////////////////////////////////////
/// CAlignOhterScore
///

void CSGAlnQualityScore::CalculateScores()
{
    if (m_AlnInfo) {
        // try aligned sequence first
        x_RetrieveQualityMap(1 - m_AlnInfo->m_Anchor, true);

        if ( !HasScores() ) {
            // maybe the anchor is the short read sequence
            x_RetrieveQualityMap(m_AlnInfo->m_Anchor, false);
        }

        // We don't need it anymore, release it.
        m_AlnInfo.Reset();
    }
}


bool CSGAlnQualityScore::HasScores() const
{
    return !m_ScoreColl.empty();
}


IAlnScoreIterator* CSGAlnQualityScore::GetScoreIterator(int row) const
{
    return new CQualityScoreIterator(m_ScoreColl);
}


CRgbaColor CSGAlnQualityScore::GetColorForScore(float score,
                                                IScoringMethod::EColorType type) const
{
    if(type == IScoringMethod::fBackground) {
        int idx = (int)(score / 5);
        switch(idx) {
            case 0:
            case 1:
                return CRgbaColor(0.855f, 0.482f, 0.141f);
            case 2:
            case 3:
                return CRgbaColor(0.855f, 0.659f, 0.141f);
            case 4:
                return CRgbaColor(0.855f, 0.835f, 0.141f);
            case 5:
                return CRgbaColor(0.514f, 0.855f, 0.141f);
            case 6:
                return CRgbaColor(0.16f,  0.855f, 0.141f);
            case 7:
                return CRgbaColor(0.141f, 0.855f, 0.482f);
            default:
                // score >= 40
                return CRgbaColor(0.141f, 0.855f, 0.788f);
        }
    }

    _ASSERT(false); // shouldn't get to here
    return CRgbaColor(0.0f, 0.0f, 0.0f);
}


bool CSGAlnQualityScore::IsAverageable() const
{
    return true;
}


int CSGAlnQualityScore::GetSupportedColorTypes() const
{
    return IScoringMethod::fBackground;
}


bool CSGAlnQualityScore::IsCacheable() const
{
    return true;
}


const string& CSGAlnQualityScore::GetScoringMethod() const
{
    return GetScoreName();
}


const string& CSGAlnQualityScore::GetScoreName()
{
    static const string kScoreName = "Quality score";
    return kScoreName;
}



template <typename ValType>
void s_AddGraphToMap(CSGAlnQualityScore::TScoreColl& score,
                     const TSeqRange& range,
                     const CSeq_graph& gr,
                     const vector<ValType>& vec)
{
    if (vec.empty()) return;

    float a = (float)(gr.IsSetA() ? gr.GetA() : 1.0);
    float b = (float)(gr.IsSetB() ? gr.GetB() : 0.0);

    TSeqPos comp = gr.IsSetComp() ? gr.GetComp() : 1;
    if (comp == 0) {
        return;
    }

    CRangeCollection<TSeqPos> ranges;
    for (CSeq_loc_CI iter(gr.GetLoc());  iter;  ++iter) {
        ranges += iter.GetRange();
    }

    typename vector<ValType>::const_iterator iter_gr = vec.begin();
    TSeqPos pre_pos = -1;
    score.SetFrom(ranges.GetFrom());
    ITERATE (CRangeCollection<TSeqPos>, r_iter, ranges) {
        const TSeqRange& r  = *r_iter;
        TSeqPos pos  = r.GetFrom();
        if (pos > pre_pos) {
            // fill the gap
            score.push_back(0, pos - pre_pos);
        }

        // if the data type is char, we need to convert them into unsigned char
        bool unsigned_char = false;
        if (sizeof(ValType) == 1) unsigned_char = true;
        unsigned char val;
        while (iter_gr != vec.end()  &&  pos < r.GetToOpen()) {
            if (unsigned_char) {
                val = (unsigned char)((*iter_gr) * a + b);
            } else {
                int val_i = (int)((*iter_gr) * a + b);
                if (val_i > 255) val = 255;
                else val = (unsigned char)val_i;
            }
            score.push_back(val, comp);
            pos += comp;
            ++iter_gr;
        }
        pre_pos = pos;
    }
}


void CSGAlnQualityScore::x_RetrieveQualityMap(IAlnExplorer::TNumrow row, bool remap)
{
    CScope& scope = *m_AlnInfo->m_Scope;
    const CSeq_align& aln = *m_AlnInfo->m_Align;
    CBioseq_Handle handle = scope.GetBioseqHandle(aln.GetSeq_id(row));
    if ( !handle ) {
        return;
    }

    CConstRef<CSeq_loc> loc(
        handle.GetRangeSeq_loc(aln.GetSeqStart(row), aln.GetSeqStop(row)));
    SAnnotSelector sel = CSeqUtils::GetAnnotSelector(CSeq_annot::C_Data::e_Graph);
    sel.AddUnnamedAnnots();
    CGraph_CI graph_iter(scope, *loc, sel);
    if (graph_iter  &&  graph_iter.GetSize() == 1) {
        const CMappedGraph& mapped_gr = *graph_iter;
        CConstRef<CSeq_graph> seq_graph(&mapped_gr.GetMappedGraph());

        // remap the graph if needed
        bool reverse = false;
        if (remap) {
            try {
                CSeq_loc_Mapper mapper(aln, aln.GetSeq_id(1 - row));
                reverse = aln.GetSeqStrand(0) != aln.GetSeqStrand(1);
                seq_graph = mapper.Map(*seq_graph);
            } catch (CAnnotMapperException&) {
                return ;
            }
        }
        
        if (seq_graph) {
            TSeqRange range = aln.GetSeqRange(m_AlnInfo->m_Anchor);
            const CSeq_graph::TGraph& graph = seq_graph->GetGraph();
            switch (graph.Which())
            {
            case CSeq_graph::TGraph::e_Byte:
                {{
                    const CByte_graph& b_graph = graph.GetByte();
                    if (reverse) {
                        CByte_graph::TValues values = b_graph.GetValues();
                        std::reverse(values.begin(), values.end());
                        s_AddGraphToMap(m_ScoreColl, range, *seq_graph, values);
                    } else {
                        s_AddGraphToMap(m_ScoreColl, range, *seq_graph, b_graph.GetValues());
                    }
                    break;
                }}
            case CSeq_graph::TGraph::e_Int:
                {{
                    const CInt_graph& i_graph = graph.GetInt();
                    if (reverse) {
                        CInt_graph::TValues values = i_graph.GetValues();
                        std::reverse(values.begin(), values.end());
                        s_AddGraphToMap(m_ScoreColl, range, *seq_graph, values);
                    } else {
                        s_AddGraphToMap(m_ScoreColl, range, *seq_graph, i_graph.GetValues());
                    }
                    break;
                }}
            case CSeq_graph::TGraph::e_Real:
                {{
                    const CReal_graph& r_graph = graph.GetReal();
                    if (reverse) {
                        CReal_graph::TValues values = r_graph.GetValues();
                        std::reverse(values.begin(), values.end());
                        s_AddGraphToMap(m_ScoreColl, range, *seq_graph, values);
                    } else {
                        s_AddGraphToMap(m_ScoreColl, range, *seq_graph, r_graph.GetValues());
                    }
                    break;
                }}
            default:
                break;
            }
        }
    }
}

END_NCBI_SCOPE
