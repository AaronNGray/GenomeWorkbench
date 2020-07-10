/*  $Id: hapmap_ds.cpp 39658 2017-10-24 20:52:55Z katargir $
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
 * Authors:  Melvin Quintos
 *
 * Description:  This file defines CHapmapJob, CSGHapmapDS, and CSGHapmapDSType
 *
 */

#include <ncbi_pch.hpp>
#include <gui/packages/pkg_snp/hapmap/hapmap_ds.hpp>
#include <gui/packages/pkg_snp/utils/utils.hpp>

#include <gui/objutils/utils.hpp>
#include <gui/objutils/snp_gui.hpp>
#include <gui/utils/app_job_dispatcher.hpp>
#include <gui/widgets/seq_graphic/histogram_glyph.hpp>

#include <objmgr/graph_ci.hpp>
#include <objmgr/seq_loc_mapper.hpp>
#include <objmgr/seq_map_switch.hpp>
#include <objmgr/seqdesc_ci.hpp>
#include <objmgr/table_field.hpp>
#include <objmgr/util/feature.hpp>
#include <objmgr/util/sequence.hpp>

#include <objects/seqres/Byte_graph.hpp>
#include <cmath>

BEGIN_NCBI_SCOPE
USING_SCOPE(objects);

///////////////////////////////////////////////////////////////////////////////
///   CHapmapJob
///////////////////////////////////////////////////////////////////////////////
class CHapmapJob : public CSGAnnotJob
{
public:
    struct SParams {
        bool        bPreferFeat;  ///< Only return feat glyphs when not overview
        bool        isOverview;
        bool        bAdaptiveSelector;
        TModelUnit  scale;
        int         binWidth;
        int         depth;
        string      name;
    };

    CHapmapJob(const CBioseq_Handle& handle, const TSeqRange& range,
            const SParams& params)
        : CSGAnnotJob("SNP", handle, SAnnotSelector(), range)
        , m_Params(params) {}

    virtual EJobState   x_Execute();

private:
    EJobState x_Load(CSeqGlyph::TObjects*);

private:
    SParams                     m_Params;
};

IAppJob::EJobState CHapmapJob::x_Execute()
{
    EJobState state = eCompleted;

    try {
        // variation data comes in pairs: graph & feature_table
        // Resolve Graph in overview mode.  Resolve feature table otherwise
        CConstRef<CSeq_loc> loc;
        CSeqGlyph::TObjects glyphs;

        state = x_Load(&glyphs);

        SHapmapJobResult *result = new SHapmapJobResult();
        m_Result.Reset(result);

        result->listObjs.swap(glyphs);
    } catch (CException& ex) {
        m_Error.Reset(new CAppJobError(ex.GetMsg()));
        state = eFailed;
    } catch (std::exception& ex) {
        m_Error.Reset(new CAppJobError(ex.what()));
        state = eFailed;
    }

    return state;
}

IAppJob::EJobState CHapmapJob::x_Load(CSeqGlyph::TObjects *objs)
{
    EJobState state = eCompleted;
    //bool    checkComp   = true;
    bool    fetchTable  = false;
    double  scale       = fabs(m_Params.scale);

    m_Sel.SetOverlapTotalRange().SetResolveAll().SetAdaptiveDepth(true);
    m_Sel.SetAnnotType(CSeq_annot::TData::e_Graph);
    if (CSeqUtils::IsNAA(m_Params.name)) {
        m_Sel.IncludeNamedAnnotAccession(m_Params.name);
    }
    m_Sel.AddNamedAnnots(m_Params.name);
    CSeqUtils::SetResolveDepth(m_Sel, m_Params.bAdaptiveSelector, m_Params.depth);

    CRef<CSeq_loc> loc = m_Handle.GetRangeSeq_loc(m_Range.GetFrom(), m_Range.GetTo());

    /* TODO Fix later
    CGraph_CI graph_iter(m_Handle.GetScope(), *loc, m_Sel);

    for (;  graph_iter ;  ++graph_iter) {
    if (m_StopRequested)
    return eCanceled;

    const CMappedGraph& graph = *graph_iter;

    if (checkComp) {
    checkComp = false; // only check compression on first pass

    if (graph.GetComp() > scale) {
    fetchTable = true;
    break;  // stop processnig graphs
    }
    }

    CRef<CLayoutObject> fref(new CHistogramGlyph(graph, m_Range, (float)scale));
    objs.push_back(fref);
    }
    */
    fetchTable = true;

    if (fetchTable) {
        m_Sel.SetAnnotType(CSeq_annot::TData::e_Seq_table);

        // grab the columns for position and value
        CTableFieldHandle<int>      col_pos("pos");
        CTableFieldHandle<double>   col_val("value");

        // Create a density map
        CHistogramGlyph::TMap data(m_Range.GetFrom(), m_Range.GetTo(),
            (float)scale, new max_func<CHistogramGlyph::TDataType>() );

        for ( CAnnot_CI it(m_Handle.GetScope(), *loc, m_Sel); it;  ++it) {
            CSeq_annot_Handle annot = *it;
            size_t rows = annot.GetSeq_tableNumRows();

            int     pos;
            double  value;

            // try and get position
            for ( size_t row = 0; row < rows; ++row ) {
                if ( col_pos.TryGet(annot, row, pos) &&
                    col_val.TryGet(annot, row, value) ) {
                // CLayoutHistogram works only with integers...
                data.AddRange(TSeqRange(pos,pos),
                    (CHistogramGlyph::TDataType)value);
                    }
            }
        }

        // XXX. We should rewrite the CLayoutHistogram.  The copying here can be avoided.
        CHistogramGlyph *hist = new CHistogramGlyph(data,
            CSeqFeatData::eSubtype_any, "Hapmap recombination rate");
        CRef<CSeqGlyph> fref(hist);
        hist->SetAnnotName(m_Params.name);
        objs->push_back(fref);
    }
    return state;
}


///////////////////////////////////////////////////////////////////////////////
/// CSGHapmapDS
///////////////////////////////////////////////////////////////////////////////
CSGHapmapDS::CSGHapmapDS(CScope& scope, const CSeq_id& id)
    : CSGGenBankDS(scope, id)
{}

void CSGHapmapDS::GetTrackNames(SConstScopedObject& object,
                                TAnnotNameTitleMap& names,
                                const TSeqRange& range,
                                const ILayoutTrackFactory::SExtraParams& params)
{
    const CSeq_id& id = dynamic_cast<const CSeq_id&>(object.object.GetObject());
    CBioseq_Handle handle = object.scope->GetBioseqHandle(id);

    // Find NHGRI Catalog
    SAnnotSelector sel(CSeqUtils::GetAnnotSelector(params.m_Annots));
    sel.SetAnnotType(CSeq_annot::TData::e_Seq_table);
    sel.SetCollectNames();
    CSeqUtils::SetResolveDepth(sel, params.m_Adaptive, params.m_Level);
    CAnnotTypes_CI a_iter(CSeq_annot::C_Data::e_Seq_table, handle,
        range, eNa_strand_unknown, &sel);
    ITERATE (CAnnotTypes_CI::TAnnotNames, iter, a_iter.GetAnnotNames()) {
        if (iter->IsNamed()) {
            names.insert(TAnnotNameTitleMap::value_type(iter->GetName(), ""));
        } else {
            names.insert(TAnnotNameTitleMap::value_type("Unnamed", ""));
        }
    }
}

void CSGHapmapDS::SetName(const string& name)
{
    m_Name = name;
}

void CSGHapmapDS::LoadData(const TSeqRange& range,
                        TModelUnit scale,
                        bool preferFeats,
                        bool isOverview)
{
    CRef<CHapmapJob> job;

    CHapmapJob::SParams p;

    p.bPreferFeat = preferFeats;
    p.isOverview  = isOverview;
    p.bAdaptiveSelector = m_Adaptive;
    p.scale       = scale;
    p.binWidth    = NSnpGui::c_BinWidth;
    p.depth       = m_Depth;
    p.name        = m_Name;

    job.Reset(new CHapmapJob(m_Handle, range, p));

    x_LaunchJob(*job);
}

///////////////////////////////////////////////////////////////////////////////
/// CSGHapmapDSType
///////////////////////////////////////////////////////////////////////////////

ISGDataSource*
CSGHapmapDSType::CreateDS(SConstScopedObject& object) const
{
    const CSeq_id& id = dynamic_cast<const CSeq_id&>(object.object.GetObject());
    return new CSGHapmapDS(object.scope.GetObject(), id);
}


string CSGHapmapDSType::GetExtensionIdentifier() const
{
    static string sid("seqgraphic_hapmap_ds_type");
    return sid;
}

string CSGHapmapDSType::GetExtensionLabel() const
{
    static string slabel("Graphical View SNP Data Source Type");
    return slabel;
}

bool CSGHapmapDSType::IsSharable() const
{
    return false;
}


END_NCBI_SCOPE
