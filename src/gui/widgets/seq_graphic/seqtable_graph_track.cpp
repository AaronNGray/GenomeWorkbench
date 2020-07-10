/*  $Id: seqtable_graph_track.cpp 42211 2019-01-11 19:53:22Z rudnev $
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

#include <gui/widgets/seq_graphic/seqtable_graph_track.hpp>
#include <gui/widgets/seq_graphic/seqgraphic_graph_ds.hpp>
#include <gui/widgets/seq_graphic/seqgraphic_job_result.hpp>
#include <gui/widgets/seq_graphic/seqgraphic_ds_manager.hpp>
#include <gui/widgets/seq_graphic/layout_track_proxy.hpp>
#include <gui/widgets/seq_graphic/comment_glyph.hpp>
#include <gui/widgets/seq_graphic/histogram_glyph.hpp>
#include <gui/widgets/seq_graphic/rendering_ctx.hpp>


BEGIN_NCBI_SCOPE
USING_SCOPE(objects);


///////////////////////////////////////////////////////////////////////////////
///   CSeqTableGraphTrack

CTrackTypeInfo CSeqTableGraphTrack::m_TypeInfo("seqtable_graph_track",
                                       "Sequence table graph track");

CSeqTableGraphTrack::CSeqTableGraphTrack(CSGGraphDS* ds,
                                         CRenderingContext* r_cntx)
    : CGraphTrack(ds, r_cntx)
{
    // initialize annotation selector
    x_GetAnnotSelector().SetAnnotType(CSeq_annot::C_Data::e_Seq_table);
}


const CTrackTypeInfo& CSeqTableGraphTrack::GetTypeInfo() const
{
    return m_TypeInfo;
}


///////////////////////////////////////////////////////////////////////////////
/// CSeqTableGraphTrackFactory methods

ILayoutTrackFactory::TTrackMap
CSeqTableGraphTrackFactory::CreateTracks(SConstScopedObject& object,
                                 ISGDataSourceContext* ds_context,
                                 CRenderingContext* r_cntx,
                                 const SExtraParams& params,
                                 const TAnnotMetaDataList& /*src_annots*/) const
{
    TTrackMap tracks;
    TAnnotNameTitleMap annots;

    if (params.m_SkipGenuineCheck  &&  !params.m_Annots.empty()) {
        ITERATE (SExtraParams::TAnnots, iter, params.m_Annots) {
            annots.insert(TAnnotNameTitleMap::value_type(*iter, ""));
        }
    } else {
        // collect non-NA tracks
        CIRef<ISGDataSource> ds = ds_context->GetDS(
            typeid(CSGGraphDSType).name(), object);
        CSGGraphDS* graph_ds = dynamic_cast<CSGGraphDS*>(ds.GetPointer());
        graph_ds->SetDepth(params.m_Level);
        graph_ds->SetAdaptive(params.m_Adaptive);
        
        SAnnotSelector sel;
        if (params.m_Annots.empty()) {
            sel = CSeqUtils::GetAnnotSelector(CSeq_annot::C_Data::e_Seq_table);
            sel.ExcludeNamedAnnots("SNP");
            graph_ds->GetSeqtableAnnots(sel, r_cntx->GetVisSeqRange(), annots);
        } else {
            sel = CSeqUtils::GetAnnotSelector(params.m_Annots);
            sel.ExcludeNamedAnnots("SNP");
            graph_ds->GetAnnotNames(sel, r_cntx->GetVisSeqRange(), annots, true);
        }        
    }
    
    // create feature tracks
    ITERATE (TAnnotNameTitleMap, iter, annots) {
        CIRef<ISGDataSource> ds =
            ds_context->GetDS(typeid(CSGGraphDSType).name(), object);
        CSGGraphDS* graph_ds = dynamic_cast<CSGGraphDS*>(ds.GetPointer());
        graph_ds->SetDepth(params.m_Level);
        graph_ds->SetAdaptive(params.m_Adaptive);
        graph_ds->SetSeqTable(true);
        graph_ds->SetGraphLevels(iter->first);

        CRef<CSeqTableGraphTrack> track(new CSeqTableGraphTrack(graph_ds, r_cntx));
        track->SetAnnot(iter->first);
        if ( !iter->second.empty() ) {
            track->SetTitle(iter->second);
        }
        tracks[iter->first] = track.GetPointer();
    }

   return tracks;
}


string CSeqTableGraphTrackFactory::GetExtensionIdentifier() const
{
    return CSeqTableGraphTrack::m_TypeInfo.GetId();
}


string CSeqTableGraphTrackFactory::GetExtensionLabel() const
{
    return CSeqTableGraphTrack::m_TypeInfo.GetDescr();
}


END_NCBI_SCOPE
