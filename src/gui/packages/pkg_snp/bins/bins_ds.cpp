/*  $Id: bins_ds.cpp 42833 2019-04-19 15:47:35Z shkeda $
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
 * Authors:  Melvin Quintos, Dmitry Rudnev
 *
 * Description:  This file defines CBinsJob, CBinsDS, and CBinsDSType
 *
 */
#include <ncbi_pch.hpp>


#include <gui/packages/pkg_snp/bins/bins_ds.hpp>
#include <gui/packages/pkg_snp/bins/bins_glyph.hpp>
#include <gui/packages/pkg_snp/bins/gmark_glyph.hpp>
#include <gui/packages/pkg_snp/bins/scatter_glyph.hpp>
#include <gui/packages/pkg_snp/utils/utils.hpp>

#include <gui/objutils/utils.hpp>
#include <gui/objutils/snp_gui.hpp>
#include <gui/utils/app_job_dispatcher.hpp>
#include <gui/widgets/seq_graphic/histogram_glyph.hpp>
#include <gui/widgets/seq_graphic/seqgraphic_utils.hpp>

#include <objmgr/graph_ci.hpp>
#include <objmgr/seq_loc_mapper.hpp>
#include <objmgr/seq_map_switch.hpp>
#include <objmgr/seqdesc_ci.hpp>
#include <objmgr/table_field.hpp>
#include <objmgr/util/feature.hpp>
#include <objmgr/util/sequence.hpp>
#include <objmgr/util/sequence.hpp>

#include <objects/seq/Annot_descr.hpp>
#include <objects/seq/Annotdesc.hpp>

#include <objtools/snputil/snp_bins.hpp>

#include <objects/seqres/Byte_graph.hpp>
#include <cmath>

BEGIN_NCBI_SCOPE
USING_SCOPE(objects);

///////////////////////////////////////////////////////////////////////////////
///   CBinsJob
///////////////////////////////////////////////////////////////////////////////
class CBinsJob : public CSGAnnotJob
{
public:
    struct SParams {
        bool        isSeqView;
        bool        bAdaptiveSelector;
        TModelUnit  scale;
        int         binWidth;
        int         depth;
        string      name;
    };

    CBinsJob(const CBioseq_Handle& handle, const TSeqRange& range,
        const SParams& params)
        : CSGAnnotJob("SNP", handle, SAnnotSelector(), range)
        , m_Params(params) {}

    virtual EJobState   x_Execute();

private:
    void    x_FetchGraphs(const CSeq_loc &, CSeqGlyph::TObjects*);
    void    x_FetchFtable(const CSeq_loc &, CSeqGlyph::TObjects*);

    // will try to load and create bins and return true;
    // if however the amount of positions would be unreasonably (>500) big, will return
    // false and glyphs will be undefined
    bool x_PreFetchFtable(const SAnnotSelector& sel,
							  const objects::CSeq_loc &loc,
							  CSeqGlyph::TObjects* glyphs);

private:
    SParams                     m_Params;
    SBinsJobResult::EDataType   m_eDataType;
};

IAppJob::EJobState CBinsJob::x_Execute()
{
    EJobState state = eCompleted;

    try {
        CSeqGlyph::TObjects glyphs;

        CRef<CSeq_loc> loc = m_Handle.GetRangeSeq_loc(m_Range.GetFrom(), m_Range.GetTo());

        x_FetchFtable(*loc, &glyphs);

        SBinsJobResult *result = new SBinsJobResult();
        m_Result.Reset(result);

        result->listObjs.swap(glyphs);
        result->eDataType = m_eDataType;
    }
    catch (CException& ex) {
        m_Error.Reset(new CAppJobError(ex.GetMsg()));
        state = eFailed;
    }
    catch (std::exception& ex) {
        m_Error.Reset(new CAppJobError(ex.what()));
        state = eFailed;
    }

    return state;
}


void CBinsJob::x_FetchFtable(const CSeq_loc &loc, CSeqGlyph::TObjects *objs)
{
    SAnnotSelector sel;
    NSnpBins::GetBinSelector(m_Params.name, m_Params.bAdaptiveSelector, m_Params.depth,
                             sel);

    CSeqGlyph::TObjects tmpList;
    bool isBinsLoaded(x_PreFetchFtable(sel, loc, &tmpList));

    if(isBinsLoaded) {
	    objs->swap(tmpList);
        m_eDataType = SBinsJobResult::eBins;
    }
    else {
        x_FetchGraphs(loc, objs);
    }
}

bool CBinsJob::x_PreFetchFtable(const SAnnotSelector& sel,
                                const CSeq_loc &loc,
                                CSeqGlyph::TObjects *objs)
{
    const CTableFieldHandle<int>      col_type("trackType");
    const CTableFieldHandle<string>   col_sub_type("trackSubType");
    int type;
    string subType;
    int     pos_start, pos_end;
    string  title, comment;

    bool    hasResult   = false; // data has at least 1 non-zero bin
    double  scale       = fabs(m_Params.scale);
    const int binWidth  = m_Params.binWidth;
    // data has at least one non-0 pvalue
    bool hasPvalue(false);

    double window = scale*binWidth;

    if (window < 1 || m_Params.isSeqView) {
        window = 1;
    }

    CBinsGlyph::TDensityMap   dmap(
        m_Range.GetFrom(),
        m_Range.GetTo(),
        window,
        new CBinsGlyph::accum_study());

    CGeneMarkerGlyph::TGeneMarkers   gene_markers;

    // Grab annotations and start processing data
    CSeq_annot_Handle annot;
    if(!NSnpBins::GetBinHandle(m_Handle.GetScope(), sel, loc, annot)) {
        return false;
    }
    NSnpBins::ReadAnnotDesc(annot, title, comment);
    NSnpBins::FindPosIndexRange(annot, (int)m_Range.GetFrom(), (int)m_Range.GetTo(), pos_start, pos_end);

    // Read the type information
    // if none, assume eGAP
    if(!col_type.TryGet(annot, 0, type)) {
        type = NSnpBins::eGAP;
    }
    // get the potentially existing subtype for GAP tracks
    if(type == NSnpBins::eGAP) {
        col_sub_type.TryGet(annot, 0, subType);
    }
    bool isGeneMarker(NSnpBins::isGeneMarker(subType));

    // if there are too many rows, abort the load and indicate that a graph should be loaded instead
    if(pos_end - pos_start > 500) {
        return false;
    }
    // Read all the rows and columns
    // the least common denominator is having the following column:
    // pos (if absent, the line will be skipped entirely)
    // if trackType is missing, it is assumed to be eGAP
    // all others are allowed to be missing
    // (although the data provider should strive to have all correct columns expected
    // for a given trackType)
    for(int row = pos_start; row < pos_end; ++row) {
        CRef<NSnpBins::SBinEntry> BinEntry(NSnpBins::GetEntry(annot, row));
        if(!BinEntry.IsNull()) {
            if(!isGeneMarker) {
                TSeqRange    range  = TSeqRange(BinEntry->pos, BinEntry->pos);
                if(BinEntry->pvalue != 0) {
                    hasPvalue = true;
                }
                CBinsGlyph::SBinMapped bin;
                bin.obj.Reset(new NSnpBins::SBin);
                bin.obj->count = 1;
                bin.obj->type = type;
                bin.obj->range = range;
                bin.obj->m_EntryList.push_back(BinEntry);
                bin.obj->m_SigEntry = BinEntry;

                dmap.AddRange(range, bin);
            } else {
                TSeqRange    range = TSeqRange(BinEntry->pos, BinEntry->pos_end);
                CRef<NSnpBins::SBin> obj(new NSnpBins::SBin);
                obj->count = 1;
                obj->type = type;
                obj->range = range;
                obj->m_SigEntry = BinEntry;

                gene_markers.push_back(obj);
            }
            hasResult = true;
        }
    }
    if (hasResult) {
        if(!isGeneMarker) {
            size_t binCount(0);
            NON_CONST_ITERATE(CBinsGlyph::TDensityMap, iBin, dmap) {
                iBin->SequentialNumber = binCount++;
            }
            if(hasPvalue && (type == NSnpBins::eGAP || type == NSnpBins::eGCAT)) {
                CRef<CScatterPlotGlyph>  scatter_plot(new CScatterPlotGlyph(dmap, loc));
                scatter_plot->SetTitle(title);
                scatter_plot->SetAnnotName(m_Params.name);
                CRef<CSeqGlyph> sref(scatter_plot.GetPointer());
                objs->push_back(sref);
            }
            CRef<CBinsGlyph>  bins;
            bins.Reset(new CBinsGlyph(dmap, loc, &m_Handle.GetScope()));
            bins->SetTitle(title);
            bins->SetAnnotName(m_Params.name);
            bins->SetType(type);
            CRef<CSeqGlyph> fref(bins.GetPointer());
            objs->push_back(fref);
        } else {
            CRef<CGeneMarkerGlyph>  gene_marker_glyph(new CGeneMarkerGlyph(gene_markers, loc, &m_Handle.GetScope()));
            gene_marker_glyph->SetTitle(title);
            gene_marker_glyph->SetAnnotName(m_Params.name);
            CRef<CSeqGlyph> fref(gene_marker_glyph.GetPointer());
            objs->push_back(fref);
        }
    }
    return true;
}

void CBinsJob::x_FetchGraphs(const CSeq_loc& loc, CSeqGlyph::TObjects *glyphs)
{
    SAnnotSelector sel;
    sel = CSeqUtils::GetAnnotSelector(CSeq_annot::TData::e_Seq_table);
    sel.IncludeNamedAnnotAccession(m_Params.name);
    sel.AddNamedAnnots(m_Params.name);
    CSeqUtils::SetResolveDepth(sel, m_Params.bAdaptiveSelector, m_Params.depth);
    CSeqGlyph::TObjects tmpList;

    CTableFieldHandle<int>      col_pos("pos");

    int     pos;
    int     pos_start, pos_end;

    CHistogramGlyph::TMap dmap(m_Range.GetFrom(), m_Range.GetTo(), m_Params.scale);

    // Grab annotations and start processing data
    CAnnot_CI iter(m_Handle.GetScope(), loc, sel);

    // internal error!
    if (iter.size()!=1) {
        NCBI_THROW(CException, eUnknown, "Incorrect bins data!");
    }

    CSeq_annot_Handle annot = *iter;
    string  title, comment;
    NSnpBins::ReadAnnotDesc(annot, title, comment);
    NSnpBins::FindPosIndexRange(annot, (int)m_Range.GetFrom(), (int)m_Range.GetTo(), pos_start, pos_end);

    // Read all the rows and columns
    for ( int row = pos_start; row < pos_end; ++row ) {
        if ( col_pos.TryGet(annot, row, pos)) {

            TSeqRange    range  = TSeqRange(pos, pos);
            dmap.AddRange(range);
        }
    }
    CHistogramGlyph* hist = new CHistogramGlyph(dmap, CSeqFeatData::eSubtype_variation, title);
    hist->SetAnnotName(m_Params.name);
    glyphs->push_back(CRef<CSeqGlyph>(hist));

    m_eDataType = SBinsJobResult::eHistFromFeats;
}



///////////////////////////////////////////////////////////////////////////////
/// CBinsDS
///////////////////////////////////////////////////////////////////////////////
CBinsDS::CBinsDS(CScope& scope, const CSeq_id& id)
    : CSGGenBankDS(scope, id)
{}

void CBinsDS::GetTrackNames(SConstScopedObject& object,
                            TAnnotNameTitleMap& names,
                            const TSeqRange& range,
                            const ILayoutTrackFactory::SExtraParams& params)
{
    const CSeq_id& id = dynamic_cast<const CSeq_id&>(object.object.GetObject());
    CBioseq_Handle handle = object.scope->GetBioseqHandle(id);

    // Find data
    SAnnotSelector sel = CSeqUtils::GetAnnotSelector(params.m_Annots);
    CSeqUtils::SetResolveDepth(sel, params.m_Adaptive, params.m_Level);
    sel.SetAnnotType(CSeq_annot::TData::e_Seq_table);
    sel.SetCollectNames();

    CAnnotTypes_CI annot_it(CSeq_annot::C_Data::e_Seq_table, handle,
        range, eNa_strand_unknown, &sel);
    ITERATE (CAnnotTypes_CI::TAnnotNames, iter, annot_it.GetAnnotNames()) {
        if (iter->IsNamed()) {
            names.insert(TAnnotNameTitleMap::value_type(iter->GetName(), ""));
        } else {
            names.insert(TAnnotNameTitleMap::value_type("Unnamed", ""));
        }
    }
}


void CBinsDS::SetName(const string& name)
{
    m_Name = name;
}

void CBinsDS::LoadData(const TSeqRange& range,
                        TModelUnit scale,
                        bool isSeqView)
{
    CRef<CBinsJob> job;

    CBinsJob::SParams p;
    p.isSeqView   = isSeqView;
    p.bAdaptiveSelector = m_Adaptive;
    p.scale       = scale;
    p.binWidth    = NSnpGui::c_BinWidth;
    p.depth       = m_Depth;
    p.name        = m_Name;

    job.Reset(new CBinsJob(m_Handle, range, p));

    x_LaunchJob(*job);
}

///////////////////////////////////////////////////////////////////////////////
/// CBinsDSType
///////////////////////////////////////////////////////////////////////////////

ISGDataSource*
CBinsDSType::CreateDS(SConstScopedObject& object) const
{
    const CSeq_id& id = dynamic_cast<const CSeq_id&>(object.object.GetObject());
    return new CBinsDS(object.scope.GetObject(), id);
}


string CBinsDSType::GetExtensionIdentifier() const
{
    static string sid("snp_bins_ds_type");
    return sid;
}

string CBinsDSType::GetExtensionLabel() const
{
    static string slabel("SNP Bin Track Data Source Type");
    return slabel;
}

bool CBinsDSType::IsSharable() const
{
    return false;
}


END_NCBI_SCOPE
