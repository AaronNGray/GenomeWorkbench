/*  $Id: snp_ds.cpp 44952 2020-04-25 01:20:25Z rudnev $
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
 * Description:  This file defines CSnpJob, CSGSnpDS, and CSGSnpDSType
 *
 */
#include <ncbi_pch.hpp>


#include <gui/packages/pkg_snp/track/snp_ds.hpp>
#include <gui/widgets/seq_graphic/histogram_glyph.hpp>
#include <gui/widgets/seq_graphic/seqgraphic_utils.hpp>

#include <gui/objutils/utils.hpp>
#include <gui/utils/app_job_dispatcher.hpp>
#include <objmgr/graph_ci.hpp>
#include <objmgr/util/sequence.hpp>
#include <objmgr/seq_loc_mapper.hpp>
#include <objects/seqres/Int_graph.hpp>
#include <objects/seqres/Byte_graph.hpp>


BEGIN_NCBI_SCOPE
USING_SCOPE(objects);

///////////////////////////////////////////////////////////////////////////////
///   CSnpJob
///////////////////////////////////////////////////////////////////////////////
CSnpJob::CSnpJob(const objects::CBioseq_Handle& handle, const TSeqRange& range,
            const SParams& params, SSnpFilter* filter)
        : CSGAnnotJob("SNP", handle, SAnnotSelector(), range)
        , m_Params(params)
        , m_Filter(filter)
{
}

IAppJob::EJobState CSnpJob::x_Execute()
{
    EJobState state = eCompleted;

    try {
        // variation data comes in pairs: graph & feature_table
        // Resolve Graph in overview mode.  Resolve feature table otherwise
        CRef<CSeq_loc> loc = m_Handle.GetRangeSeq_loc(m_Range.GetFrom(), m_Range.GetTo());
        CSeqGlyph::TObjects glyphs;

        if (m_Params.isOverview) {
            x_FetchGraphs(*loc, &glyphs);
        }
        else {
            x_FetchFtable(*loc, &glyphs);
        }

        SSnpJobResult *result = new SSnpJobResult();
        m_Result.Reset(result);

        result->listObjs.swap(glyphs);
        result->eDataType = m_eDataType;
    } catch (CException& ex) {
        m_Error.Reset(new CAppJobError(ex.GetMsg()));
        state = eFailed;
    } catch (std::exception& ex) {
        m_Error.Reset(new CAppJobError(ex.what()));
        state = eFailed;
    }

    return state;
}

//!! this is a temporary function for an interim period when SNP filters are taken from
//!! TMS and recorded as annots
//!! to avoid creating a selector with annotation containing a filter (which will not load
//!! anything as a result), such annotations are converted to "SNP"
//!! this will need to be revisited once TMS/OM bridge will need to be built for SNP_in_VDB
string CSnpJob::s_AdjustAnnotName(const string& sAnnotName)
{
    auto AnnotNameType(CSeqUtils::NameTypeStrToValue(sAnnotName));
    if (AnnotNameType == CSeqUtils::eAnnot_Other) {
        if (!CSeqUtils::IsNAA(sAnnotName)) {
            return "SNP";
        }
    }
    return sAnnotName;
}

void CSnpJob::x_FetchGraphs(const CSeq_loc& loc, CSeqGlyph::TObjects *glyphs)
{
    // first, try to load a graph track based on the annotation name
    SAnnotSelector sel;

    sel = CSeqUtils::GetAnnotSelector(CSeq_annot::TData::e_Graph,
                                      CSeqUtils::fAnnot_UnsetNamed);
    auto sAnnotName(s_AdjustAnnotName(m_Params.sAnnotName));
    sel.SetExcludeExternal(false);
    CSeqUtils::SetResolveDepth(sel, m_Params.bAdaptiveSelector, m_Params.depth);
    string zoomAnnotName = sAnnotName;
    // special handling for SNP2
    if (CSeqUtils::IsExtendedNAA(sAnnotName)) {
        sel.IncludeNamedAnnotAccession(sAnnotName, -1);
        sel.SetCollectNames();
        CGraph_CI graph_iter(m_Handle.GetScope(), loc, sel);
        int best_zoom_level = kMax_Int;
        int max_zoom_level = loc.GetTotalRange().GetLength()/2000;
        for (auto& name : graph_iter.GetAnnotNames()) {
            if (!name.IsNamed()) {
                continue;
            }
            string base_name;
            int zoom_level;
            if (!ExtractZoomLevel(name.GetName(), &base_name, &zoom_level) ||
                 base_name != sAnnotName) {
                continue;
            }
            if (zoom_level <= max_zoom_level) {
                if (zoom_level > best_zoom_level || best_zoom_level > max_zoom_level) {
                    best_zoom_level = zoom_level;
                }
            }
            else {
                if (zoom_level < best_zoom_level) {
                    best_zoom_level = zoom_level;
                }
            }
        }
        if (best_zoom_level != kMax_Int) {
            zoomAnnotName = CombineWithZoomLevel(sAnnotName, best_zoom_level);
        }
        sel.SetCollectNames(false);
        sel.ResetAnnotsNames();
    }
    sel.IncludeNamedAnnotAccession(zoomAnnotName);
    sel.AddNamedAnnots(zoomAnnotName);

    CGraph_CI graph_iter(m_Handle.GetScope(), loc, sel);

    typedef map<string, CHistogramGlyph::TMap > TDMaps;
    TDMaps dmaps;
    if (graph_iter) {
        for ( ; graph_iter; ++graph_iter) {
            const CMappedGraph& gr = *graph_iter;
            TSeqRange r  = gr.GetLoc().GetTotalRange();
            TSeqPos comp = gr.GetComp();
            TSeqPos pos  = r.GetFrom();
            string title = gr.GetTitle();
            double a = gr.IsSetA()? gr.GetA(): 1.;
            double b = gr.IsSetB()? gr.GetB(): 0.;

            TDMaps::iterator it = dmaps.find(title);
            if ( it == dmaps.end()) {
                it = dmaps.insert(
                    TDMaps::value_type(
                        title,
                        CHistogramGlyph::TMap(
                            m_Range.GetFrom(),
                            m_Range.GetTo(),
                            m_Params.scale))).first;
            }

            CHistogramGlyph::TMap &the_map = (*it).second;
            if (gr.GetGraph().IsByte()) {
                const CByte_graph::TValues& values = gr.GetGraph().GetByte().GetValues();
                ITERATE(CByte_graph::TValues, iter_gr, values) {
                    TSeqRange range = TSeqRange(pos, pos + comp);
                    CHistogramGlyph::TDataType value =
                        CHistogramGlyph::TDataType(*iter_gr * a + b);
                    the_map.AddRange(range, value);
                    pos += comp;
                }
            }
            else {
                const CInt_graph::TValues& values = gr.GetGraph().GetInt().GetValues();
                ITERATE(CInt_graph::TValues, iter_gr, values) {
                    TSeqRange range = TSeqRange(pos, pos + comp);
                    CHistogramGlyph::TDataType value =
                        CHistogramGlyph::TDataType(*iter_gr * a + b);
                    the_map.AddRange(range, value);
                    pos += comp;
                }
            }
        }

        // For each map, create a CHistogramGlyph
        NON_CONST_ITERATE(TDMaps, it, dmaps) {
            const string& title             = it->first;
            CHistogramGlyph::TMap &the_map  = it->second;

            CHistogramGlyph *hist = new CHistogramGlyph(the_map, CSeqFeatData::eSubtype_variation, title);
            CRef<CSeqGlyph> fref(hist);
            hist->SetAnnotName(title);

            glyphs->push_back(fref);
        }
        m_eDataType = SSnpJobResult::eHistFromGraphs;
    }
    // if a graph track is not found, try to make up some data from the features
    else
    {
        sel = CSeqUtils::GetAnnotSelector(CSeqFeatData::eSubtype_variation,
            CSeqUtils::fAnnot_UnsetNamed);

        auto sAnnotName(s_AdjustAnnotName(m_Params.sAnnotName));
        if(CSeqUtils::NameTypeStrToValue(sAnnotName) ==
            CSeqUtils::eAnnot_Unnamed) {
            sel.AddUnnamedAnnots();
        } else {
            sel.IncludeNamedAnnotAccession(sAnnotName);
            sel.AddNamedAnnots(sAnnotName);
        }

        CSeqUtils::SetResolveDepth(sel, m_Params.bAdaptiveSelector, m_Params.depth);
        CSeqGlyph::TObjects tmpList;
        x_PreFetchFtable(sel, loc, tmpList);

        x_MakeHistogramFromFtable(tmpList, *glyphs);
        m_eDataType = SSnpJobResult::eHistFromFeats;
    }
}


static bool s_passes_bitfield(const CSeq_feat& or_feat, const SSnpFilter& filter)
{
    CSnpBitfield bitfield(or_feat);
    return filter.Passes(bitfield);
}

void CSnpJob::x_FetchFtable(const CSeq_loc &loc, CSeqGlyph::TObjects *glyphs)
{
    SAnnotSelector sel;

    sel = CSeqUtils::GetAnnotSelector(CSeqFeatData::eSubtype_variation,
        CSeqUtils::fAnnot_UnsetNamed);

    auto sAnnotName(s_AdjustAnnotName(m_Params.sAnnotName));
    if(CSeqUtils::NameTypeStrToValue(sAnnotName) ==
        CSeqUtils::eAnnot_Unnamed) {
        sel.AddUnnamedAnnots();
    } else {
        sel.IncludeNamedAnnotAccession(sAnnotName);
        sel.AddNamedAnnots(sAnnotName);
    }

    CSeqUtils::SetResolveDepth(sel, m_Params.bAdaptiveSelector, m_Params.depth);

    x_FetchFtableEx(sel, loc, glyphs);
}

void CSnpJob::x_FetchFtableEx(const SAnnotSelector& sel,
							  const ncbi::objects::CSeq_loc &loc,
							  CSeqGlyph::TObjects *glyphs)
{
    CSeqGlyph::TObjects tmpList;
    x_PreFetchFtable(sel, loc, tmpList);

    if (m_Params.bPreferFeat || (m_Params.bAdaptive && (tmpList.size() <= 50)) ) {
		if(glyphs->empty())
			glyphs->swap(tmpList);
		else
			glyphs->insert(glyphs->end(), tmpList.begin(), tmpList.end());
        m_eDataType = SSnpJobResult::eFeats;
    }
    else {
        x_MakeHistogramFromFtable(tmpList, *glyphs);
        m_eDataType = SSnpJobResult::eHistFromFeats;
    }
}

void CSnpJob::x_PreFetchFtable(const SAnnotSelector& sel,
							  const CSeq_loc &loc,
							  CSeqGlyph::TObjects& glyphs)
{
	CFeat_CI feat_it(m_Handle.GetScope(), loc, sel);
    if (m_Filter) {
        for( ; feat_it; ++feat_it) {
            CFeatGlyph *feat = new CFeatGlyph(*feat_it);
            CRef<CSeqGlyph> glyph(feat);

            if (s_passes_bitfield(feat_it->GetOriginalFeature(), *m_Filter)) {
                CFeatGlyph::TIntervals& intervals = feat->SetIntervals();
                intervals.push_back(feat_it->GetTotalRange());
                glyphs.push_back(glyph);
            }
        }
    }
    else {
        for( ; feat_it; ++feat_it) {
            CFeatGlyph *feat = new CFeatGlyph(*feat_it);
            CRef<CSeqGlyph> glyph(feat);
            CFeatGlyph::TIntervals& intervals = feat->SetIntervals();
            intervals.push_back(feat_it->GetTotalRange());
            glyphs.push_back(glyph);
        }
    }
}

void CSnpJob::x_MakeHistogramFromFtable(const CSeqGlyph::TObjects& srcFtable,
							  CSeqGlyph::TObjects& glyphs)
{
    CHistogramGlyph::TMap dmap(m_Range.GetFrom(), m_Range.GetTo(), m_Params.scale);

    ITERATE(CSeqGlyph::TObjects, iter, srcFtable) {
        dmap.AddRange( (*iter)->GetRange() );
    }

    auto sAnnotName(s_AdjustAnnotName(m_Params.sAnnotName));
    CHistogramGlyph* hist = new CHistogramGlyph(
        dmap, CSeqFeatData::eSubtype_variation, sAnnotName);
    glyphs.push_back(CRef<CSeqGlyph>(hist));
    hist->SetAnnotName(sAnnotName);
}

///////////////////////////////////////////////////////////////////////////////
/// CSGSnpDS
///////////////////////////////////////////////////////////////////////////////
CSGSnpDS::CSGSnpDS(CScope& scope, const CSeq_id& id)
    : CSGGenBankDS(scope, id)
{}

void CSGSnpDS::GetAnnotNames(SConstScopedObject& object,
                             map<string, string>& annots,
                             const TSeqRange& range,
                             const ILayoutTrackFactory::SExtraParams& params)
{
    const CSeq_id& id = dynamic_cast<const CSeq_id&>(object.object.GetObject());
    CBioseq_Handle handle = object.scope->GetBioseqHandle(id);

    SAnnotSelector sel(CSeqUtils::GetAnnotSelector(params.m_Annots));
    if(find(params.m_Annots.begin(), params.m_Annots.end(), "SNP") != params.m_Annots.end()) {
        sel.IncludeNamedAnnotAccession("SNP");
    }
    sel.SetFeatSubtype(CSeqFeatData::eSubtype_variation);
    sel.SetCollectNames();
    CSeqUtils::SetResolveDepth(sel, params.m_Adaptive, params.m_Level);

    CFeat_CI feat_iter(handle, range, sel);
    CFeat_CI::TAnnotNames::const_iterator iter = feat_iter.GetAnnotNames().begin();
    if (iter != feat_iter.GetAnnotNames().end()) {
        if (iter->IsNamed())
            annots[params.m_Filter] = iter->GetName();
        else
            annots[params.m_Filter] = "Unnamed";
    }
}

void CSGSnpDS::LoadData(const TSeqRange& range,
                        CSnpJob::SParams& p,
                        SSnpFilter* filter)
{
    CRef<CSnpJob> job;
    p.bAdaptiveSelector = m_Adaptive;
    p.depth = m_Depth;
    job.Reset(new CSnpJob(m_Handle, range, p, filter));
    x_LaunchJob(*job);
}

///////////////////////////////////////////////////////////////////////////////
/// CSGSnpDSType
///////////////////////////////////////////////////////////////////////////////

ISGDataSource*
CSGSnpDSType::CreateDS(SConstScopedObject& object) const
{
    const CSeq_id& id = dynamic_cast<const CSeq_id&>(object.object.GetObject());
    return new CSGSnpDS(object.scope.GetObject(), id);
}


string CSGSnpDSType::GetExtensionIdentifier() const
{
    static string sid("seqgraphic_snp_ds_type");
    return sid;
}

string CSGSnpDSType::GetExtensionLabel() const
{
    static string slabel("Graphical View SNP Data Source Type");
    return slabel;
}

bool CSGSnpDSType::IsSharable() const
{
    return false;
}


END_NCBI_SCOPE
