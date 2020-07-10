/*  $Id: ld_ds.cpp 39658 2017-10-24 20:52:55Z katargir $
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
 * Description:  This file defines CLDBlockJob, CLDBlockDS, and CLDBlockDSType
 *
 */

#include <ncbi_pch.hpp>
#include <gui/packages/pkg_snp/ld/ld_ds.hpp>
#include <gui/packages/pkg_snp/ld/ld_glyph.hpp>
#include <gui/packages/pkg_snp/ld/ld_line.hpp>
#include <gui/packages/pkg_snp/utils/utils.hpp>

#include <gui/objutils/utils.hpp>
#include <gui/utils/app_job_dispatcher.hpp>
#include <gui/widgets/seq_graphic/histogram_glyph.hpp>

#include <objmgr/graph_ci.hpp>
#include <objmgr/seq_loc_mapper.hpp>
#include <objmgr/seq_map_switch.hpp>
#include <objmgr/seqdesc_ci.hpp>
#include <objmgr/table_field.hpp>
#include <objmgr/util/feature.hpp>
#include <objmgr/util/sequence.hpp>
#include <objmgr/util/sequence.hpp>

#include <objects/seqres/Byte_graph.hpp>
#include <cmath>

BEGIN_NCBI_SCOPE
USING_SCOPE(objects);

///////////////////////////////////////////////////////////////////////////////
///   Static Helper functions
///////////////////////////////////////////////////////////////////////////////
static bool s_PassesFilter(const CMappedFeat& feat, CLDBlockDS::SParams& params)
{
    const CSeq_feat& or_feat = feat.GetOriginalFeature();
    const CUser_object &user = or_feat.GetData().GetUser();

    TSeqRange range = feat.GetTotalRange();
    float     score = user.GetField("score").GetData().GetReal();

    TSeqPos dist = range.GetLength();

    return (  ((int)dist >= params.filterLength) && (score >= params.filterScore) );
}

///////////////////////////////////////////////////////////////////////////////
///   CLDBlockJob
///////////////////////////////////////////////////////////////////////////////
class CLDBlockJob : public CSGAnnotJob
{
public:
    CLDBlockJob(const CBioseq_Handle& handle, const CLDBlockDS::SParams& params)
        : CSGAnnotJob("SNP", handle, SAnnotSelector(), params.range)
        , m_Params(params) {}

    virtual EJobState   x_Execute();

private:
    //bool    x_IsOverview() const;
    void x_Load(CSeqGlyph::TObjects*);
    void x_LoadFtable(CFeat_CI& feats, CSeqGlyph::TObjects* glyphs);

private:
    CLDBlockDS::SParams m_Params;
};

IAppJob::EJobState CLDBlockJob::x_Execute()
{
    EJobState state = eCompleted;

    try {
        // variation data comes in pairs: graph & feature_table
        // Resolve Graph in overview mode.  Resolve feature table otherwise
        CConstRef<CSeq_loc> loc;
        CSeqGlyph::TObjects glyphs;

        x_Load(&glyphs);

        CSGJobResult *result = new CSGJobResult();
        m_Result.Reset(result);

        result->m_ObjectList.swap(glyphs);
    } catch (CException& ex) {
        m_Error.Reset(new CAppJobError(ex.GetMsg()));
        state = eFailed;
    } catch (std::exception& ex) {
        m_Error.Reset(new CAppJobError(ex.what()));
        state = eFailed;
    }

    return state;
}

void CLDBlockJob::x_Load(CSeqGlyph::TObjects *objs)
{
    SAnnotSelector sel =
    CSeqUtils::GetAnnotSelector(CSeq_annot::TData::e_Ftable);
    sel.AddNamedAnnots(m_Params.name);
    CSeqUtils::SetResolveDepth(sel, m_Params.bAdaptiveSelector, m_Params.depth);

    CRef<CSeq_loc> loc = m_Handle.GetRangeSeq_loc(m_Range.GetFrom(), m_Range.GetTo());

    CFeat_CI iter(m_Handle.GetScope(), *loc, sel);
    x_LoadFtable(iter, objs);
}

void CLDBlockJob::x_LoadFtable(CFeat_CI& feat, CSeqGlyph::TObjects* glyphs)
{
    // Place feats in separate lists based on popid
    // For each list, create a new CLDBlockLine

    typedef std::map<string, CLDBlockLine::TListFeats> TMapPopNames;
    TMapPopNames  mapPopName;

    for ( ; feat; ++feat) {
        if (s_PassesFilter(*feat, m_Params)) {
            const CSeq_feat& or_feat = feat->GetOriginalFeature();
            const CUser_object &user = or_feat.GetData().GetUser();

            string pop = user.GetField("pop_name").GetData().GetStr();

            mapPopName[pop].push_back(*feat);
        }
    }

    // Check if we are showing histograms or real features
    if (m_Params.scale > 10000 && feat.GetSize() > 20) {
        // Show histogram

        ITERATE(TMapPopNames, mapIter, mapPopName) {
            const string& title = mapIter->first;
            CHistogramGlyph::TMap dmap(m_Range.GetFrom(),
                m_Range.GetTo(), m_Params.scale);

            ITERATE(CLDBlockLine::TListFeats, feat_iter, mapIter->second) {
                dmap.AddRange(feat_iter->GetTotalRange());
            }

            CHistogramGlyph* hist = new CHistogramGlyph(dmap, title);
            glyphs->push_back(CRef<CSeqGlyph>(hist));
            hist->SetAnnotName("LDBlock");
        }
    }
    else {
        // Show features
        ITERATE(TMapPopNames, mapIter, mapPopName) {
            CLayoutGroup::TObjectList the_feats;

            ITERATE(CLDBlockLine::TListFeats, iter, mapIter->second) {
                CRef<CLDBlockGlyph>  ldblock;
                ldblock.Reset(new CLDBlockGlyph(*iter) );

                CRef<CSeqGlyph> fref(ldblock.GetPointer());
                the_feats.push_back(fref);
            }

            string title = mapIter->first;

            CGlyphContainer* ld_line = new CLDBlockLine(m_Params.r_cntx);
            CRef<CSeqGlyph> ref_line(dynamic_cast<CSeqGlyph*>(ld_line));
            glyphs->push_back(ref_line);

            ld_line->SetObjects(the_feats);
            ld_line->SetTitle(title);
        }
    }
}


///////////////////////////////////////////////////////////////////////////////
/// CLDBlockDS
///////////////////////////////////////////////////////////////////////////////
CLDBlockDS::CLDBlockDS(CScope& scope, const CSeq_id& id)
    : CSGGenBankDS(scope, id)
{}

void CLDBlockDS::GetTrackNames(SConstScopedObject& object,
                             TAnnotNameTitleMap& names,
                             const TSeqRange& range,
                             const ILayoutTrackFactory::SExtraParams& params)
{
    const CSeq_id& id = dynamic_cast<const CSeq_id&>(object.object.GetObject());
    CBioseq_Handle handle = object.scope->GetBioseqHandle(id);

    // Find NHGRI Catalog
    SAnnotSelector sel = CSeqUtils::GetAnnotSelector(params.m_Annots);
    CSeqUtils::SetResolveDepth(sel, params.m_Adaptive, params.m_Level);
    sel.SetCollectNames();

    CFeat_CI feat_iter(handle, range, sel);
    ITERATE (CFeat_CI::TAnnotNames, iter, feat_iter.GetAnnotNames()) {
        if (iter->IsNamed()) {
            names.insert(TAnnotNameTitleMap::value_type(iter->GetName(), ""));
        } else {
            names.insert(TAnnotNameTitleMap::value_type("Unnamed", ""));
        }
    }
}


void CLDBlockDS::SetName(const string& name)
{
    m_Name = name;
}

void CLDBlockDS::LoadData(const SParams& param)
{
    CRef<CLDBlockJob> job;

    SParams p(param);
    p.name = m_Name;
    p.bAdaptiveSelector = m_Adaptive;
    p.depth = m_Depth;
    job.Reset(new CLDBlockJob(m_Handle, p));

    x_LaunchJob(*job);
}

///////////////////////////////////////////////////////////////////////////////
/// CLDBlockDSType
///////////////////////////////////////////////////////////////////////////////

ISGDataSource*
CLDBlockDSType::CreateDS(SConstScopedObject& object) const
{
    const CSeq_id& id = dynamic_cast<const CSeq_id&>(object.object.GetObject());
    return new CLDBlockDS(object.scope.GetObject(), id);
}


string CLDBlockDSType::GetExtensionIdentifier() const
{
    static string sid("seqgraphic_ldblock_ds_type");
    return sid;
}

string CLDBlockDSType::GetExtensionLabel() const
{
    static string slabel("Graphical View SNP Data Source Type");
    return slabel;
}

bool CLDBlockDSType::IsSharable() const
{
    return false;
}


END_NCBI_SCOPE
