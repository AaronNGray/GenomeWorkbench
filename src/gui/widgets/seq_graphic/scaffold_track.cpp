/*  $Id: scaffold_track.cpp 34291 2015-12-08 23:38:49Z rudnev $
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

#include <gui/widgets/seq_graphic/scaffold_track.hpp>
#include <gui/widgets/seq_graphic/seqgraphic_ds_manager.hpp>
#include <gui/widgets/seq_graphic/seqgraphic_utils.hpp>
#include <gui/widgets/seq_graphic/rendering_ctx.hpp>

BEGIN_NCBI_SCOPE
USING_SCOPE(objects);


///////////////////////////////////////////////////////////////////////////////
///   CScaffoldTrack
///////////////////////////////////////////////////////////////////////////////

CTrackTypeInfo CScaffoldTrack::m_TypeInfo("scaffold_track",
    "Scaffold/config track");

static string kDefTrackTitle = "Scaffold Map";

CScaffoldTrack::CScaffoldTrack(CSGSegmentMapDS* ds, CRenderingContext* r_cntx)
    : CSegmentMapTrack(ds, r_cntx)
{}


CScaffoldTrack::~CScaffoldTrack()
{}


const CTrackTypeInfo& CScaffoldTrack::GetTypeInfo() const
{
    return m_TypeInfo;
}


string CScaffoldTrack::GetFullTitle() const
{
    if (GetTitle().empty()) {
        return kDefTrackTitle;
    }
    return GetTitle();
}


///////////////////////////////////////////////////////////////////////////////
/// CScaffoldTrackFactory
///////////////////////////////////////////////////////////////////////////////
ILayoutTrackFactory::TTrackMap
CScaffoldTrackFactory::CreateTracks(SConstScopedObject& object,
                               ISGDataSourceContext* ds_context,
                               CRenderingContext* r_cntx,
                               const SExtraParams& params,
                               const TAnnotMetaDataList& /* src_annots */) const
{
    TTrackMap tracks;
    CIRef<ISGDataSource> ds = ds_context->GetDS(
        typeid(CSGSegmentMapDSType).name(), object);
    CSGSegmentMapDS* segment_ds = dynamic_cast<CSGSegmentMapDS*>(ds.GetPointer());

    bool is_chromosome = x_IsChromosome(*segment_ds);
    segment_ds->SetChromosome(is_chromosome);

    if ( !params.m_Adaptive ) {
        // In Multi-level layout mode, we need to check if this is
        // the right level to show the scaffold map.

        // For multi-level layout, we move segment map one level down.
        // So we will show level 0 segments (scaffolds) at level 1, and
        // show leve 1 segments (scaffolds) at level 2, and there is
        // no segment map shown at level 0.
        if ( !is_chromosome  ||
            (is_chromosome  &&  params.m_Level != 1)) {
            return tracks;
        }
    }

    segment_ds->SetSegmentLevel(CSGSegmentMapDS::eContig);

    TSeqRange range = params.m_Range;
    if (range.Empty()) {
        range = r_cntx->GetVisSeqRange();
    }
    if (range.IsWhole()) {
        CRef<CSGSequenceDS> seq_ds = r_cntx->GetSeqDS();
        range.Set(0, seq_ds->GetSequenceLength() - 1);
    }

    if (params.m_SkipGenuineCheck  ||  segment_ds->HasScaffold(range)) {
        CRef<CScaffoldTrack> track(new CScaffoldTrack(segment_ds, r_cntx));
        tracks[kDefTrackTitle] = track.GetPointer();
    }
    return tracks;
}


string CScaffoldTrackFactory::GetExtensionIdentifier() const
{
    return CScaffoldTrack::m_TypeInfo.GetId();
}


string CScaffoldTrackFactory::GetExtensionLabel() const
{
    return CScaffoldTrack::m_TypeInfo.GetDescr();
}


CRef<CTrackConfigSet>
CScaffoldTrackFactory::GetSettings(const string& /*profile*/,
                                   const TKeyValuePairs& settings,
                                   const CTempTrackProxy* track_proxy) const
{
    /// no setting to report
    CRef<CTrackConfigSet> config_set(new CTrackConfigSet);
    CRef<CTrackConfig> config(new CTrackConfig);
    config_set->Set().push_back(config);
    config->SetHelp() = "Coordinated, mapped set of overlapping DNA segments \
derived from the genetic source and representing a complete molecule segment.";
    config->SetLegend_text("anchor_9");

    return config_set;
}


END_NCBI_SCOPE
