/*  $Id: ld_track.cpp 37640 2017-01-31 18:24:42Z shkeda $
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
 * File Description:
 *
 */

#include <ncbi_pch.hpp>

#include <gui/packages/pkg_snp/ld/ld_track.hpp>
#include <gui/packages/pkg_snp/ld/ld_glyph.hpp>
#include <gui/packages/pkg_snp/ld/ld_filter_dlg.hpp>
#include <gui/widgets/seq_graphic/seqgraphic_utils.hpp>
#include <gui/widgets/seq_graphic/seqgraphic_job_result.hpp>
#include <gui/widgets/seq_graphic/seqgraphic_ds_manager.hpp>
#include <gui/widgets/seq_graphic/simple_layout_policy.hpp>
#include <gui/widgets/seq_graphic/layered_layout_policy.hpp>
#include <gui/widgets/seq_graphic/rendering_ctx.hpp>
#include <gui/widgets/seq_graphic/histogram_glyph.hpp>

#include <gui/utils/rgba_color.hpp>
#include <gui/objutils/utils.hpp>
#include <gui/objutils/label.hpp>

#include <objmgr/util/feature.hpp>
#include <objmgr/util/sequence.hpp>
#include <objmgr/annot_ci.hpp>

#include <objects/seqfeat/SeqFeatData.hpp>

#include <wx/menu.h>

BEGIN_NCBI_SCOPE
USING_SCOPE(objects);
    
///////////////////////////////////////////////////////////////////////////////
///   CLDBlockTrack
///////////////////////////////////////////////////////////////////////////////

CTrackTypeInfo CLDBlockTrack::m_TypeInfo("SNP_LDBlock_track",
                                     "Graphical View LD Block Track");

CLDBlockTrack::CLDBlockTrack(CLDBlockDS* ds, CRenderingContext* r_cntx)
    : CDataTrack(r_cntx)
    , m_DS(ds)
    , m_IconFilter(eIcon_Settings, "Filters", true, "track_settings")
{
    m_DS->SetJobListener(this);
    m_Params.filterScore = 0.0;
    m_Params.filterLength = 0;
    x_RegisterIcon(m_IconFilter);
    m_Simple->SetVertSpace(1);
}


CLDBlockTrack::~CLDBlockTrack()
{
    if ( !m_DS->AllJobsFinished() ) {
        m_DS->DeleteAllJobs();
    }
}

bool CLDBlockTrack::x_IsOverviewMode() const
{
    static double kNPP_Cutoff= 50;  // Nucliotides per pixel cutoff for SNP Overview
    return (m_Context->GetScale() > kNPP_Cutoff);
}

/// @name CLayoutTrack reimplemented methods.
/// @{
const CTrackTypeInfo& CLDBlockTrack::GetTypeInfo() const
{
    return m_TypeInfo;
}

void CLDBlockTrack::x_OnIconClicked(TIconID id)
{
    bool bIsLayoutOnlyChange = true;

    if (id == eIcon_Settings) {
        // create structure
        CLDFilterParams param;
        int lenExp = m_Params.filterLength;
        lenExp = (lenExp<1) ? 0 : (int)log10((float)lenExp);

        param.SetScore((int)(m_Params.filterScore*100));
        param.SetLengthExp(lenExp);

        // load current settings
        CLDFilterDialog dlg(NULL);
        dlg.SetData(param);
        
        // do dialog window        
        if (dlg.ShowModal() == wxID_OK) {
            // save settings
            const CLDFilterParams &p = dlg.GetData();                        
            float lenPow = pow(10.0f, p.GetLengthExp());
            m_Params.filterLength = (int)lenPow;
            m_Params.filterScore  = p.GetScore()/100.0f;
            bIsLayoutOnlyChange = false;
        }
    } else {
        // use default handlers
        CLayoutTrack::x_OnIconClicked(id);
    }

    Update(bIsLayoutOnlyChange);
    x_OnLayoutChanged();
}
/// @}


/// @name CDataTrack pure virtual interfaces
/// @{

void CLDBlockTrack::x_UpdateData()
{
    CDataTrack::x_UpdateData();
    bool bShowLabels = (m_eLayout == eLayout_Labels);
    m_DS->DeleteAllJobs();
    x_SetStartStatus();
    
    m_Params.isOverview    = x_IsOverviewMode();
    m_Params.r_cntx        = const_cast<CRenderingContext*> (GetRenderingContext());
    m_Params.range         = m_Context->GetVisSeqRange();
    m_Params.scale         = m_Context->GetScale();
    m_Params.preferFeats   = bShowLabels; // fetch features if we show labels.

    m_DS->LoadData(m_Params);
}


void CLDBlockTrack::x_OnJobCompleted(CAppJobNotification& notify)
{
    m_DS->ClearJobID(notify.GetJobID());
    CRef<CObject> res_obj = notify.GetResult();
    CSGJobResult* result = dynamic_cast<CSGJobResult*>(&*res_obj);
    if (!result) {
        ERR_POST("CLDBlockTrack::x_OnJobCompleted() notification for job \
                does not contain results.");
        return;
    }

    SetMsg("");
    x_CreateLayout(*result);
}

/// @}

void CLDBlockTrack::x_CreateLayout(const CSGJobResult& result)
{
    const CSeqGlyph::TObjects& objs = result.m_ObjectList;    

    m_Group.Clear();

    if (objs.empty()) {
        Update(true);
        x_OnLayoutChanged();
        return;
    }
        
    CLayoutTrack::TTrackAttrFlags attrib = CLayoutTrack::fMovable 
                                         | CLayoutTrack::fFullTrack;                                         

    ITERATE(CSeqGlyph::TObjects, it, objs) {    
        CSeqGlyph*          glyph = const_cast<CSeqGlyph*> (it->GetPointer());        

        if (typeid(*glyph) == typeid(CHistogramGlyph)) {
            CHistogramGlyph *hist = dynamic_cast<CHistogramGlyph*> (glyph);
            hist->SetConfig(*x_GetGlobalConfig());
        }
        else {
            CGlyphContainer*    track = dynamic_cast<CGlyphContainer*> (glyph);
            track->SetConfig(m_gConfig);
            track->SetLayoutPolicy(m_Inline);
            //track->SetIndent(m_IndentLevel + 1);
            track->SetTrackAttr(attrib);
        }

        Add(glyph);
    }    

    Update(true);
    x_OnLayoutChanged();
}



///////////////////////////////////////////////////////////////////////////////
/// CLDBlockTrackFactory
///////////////////////////////////////////////////////////////////////////////
ILayoutTrackFactory::TTrackMap
CLDBlockTrackFactory::CreateTracks(SConstScopedObject& object,
                                  ISGDataSourceContext* ds_context,
                                  CRenderingContext* r_cntx,
                                  const SExtraParams& params,
                                  const TAnnotMetaDataList& src_annots) const
{
    TTrackMap tracks;
    
    TAnnotNameTitleMap annots;
    if (params.m_SkipGenuineCheck  &&  !params.m_Annots.empty()) {
        ITERATE (SExtraParams::TAnnots, iter, params.m_Annots) {
            annots.insert(TAnnotNameTitleMap::value_type(*iter, ""));
        }
    } else {

        CLDBlockDS::GetTrackNames(object, annots, r_cntx->GetVisSeqRange(), params);
        if ( !src_annots.empty() ) {
            GetMatchedAnnots(src_annots, params, annots);
        }
    }

    // create feature tracks
    ITERATE (TAnnotNameTitleMap, iter, annots) {
        // Create a new DS object for each track
        CIRef<ISGDataSource> pre_ds = ds_context->GetDS(typeid(CLDBlockDSType).name(), 
                                                    object);
        CLDBlockDS* seq_ds = dynamic_cast<CLDBlockDS*>(pre_ds.GetPointer());
        seq_ds->SetDepth(params.m_Level);
        seq_ds->SetAdaptive(params.m_Adaptive);
        seq_ds->SetName(iter->first);
        CRef<CLDBlockTrack> track(new CLDBlockTrack(seq_ds, r_cntx));
        if ( !iter->second.empty() ) {
            track->SetTitle(iter->second);
        }
        tracks[iter->first] = track.GetPointer();
    }

    return tracks;
}

void CLDBlockTrackFactory::GetMatchedAnnots(
    const TAnnotMetaDataList& src_annots,
    const ILayoutTrackFactory::SExtraParams& params,
    TAnnotNameTitleMap& out_annots) const
{
    ILayoutTrackFactory::GetMatchedAnnots(src_annots, params.m_Annots, "ftable", "HapMapLDBlock", out_annots);
}


string CLDBlockTrackFactory::GetExtensionIdentifier() const
{
    return CLDBlockTrack::m_TypeInfo.GetId();
}

string CLDBlockTrackFactory::GetExtensionLabel() const
{
    return CLDBlockTrack::m_TypeInfo.GetDescr();
}


END_NCBI_SCOPE
