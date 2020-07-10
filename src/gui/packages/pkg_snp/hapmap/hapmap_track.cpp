/*  $Id: hapmap_track.cpp 37640 2017-01-31 18:24:42Z shkeda $
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

#include <gui/packages/pkg_snp/hapmap/hapmap_track.hpp>
#include <gui/widgets/seq_graphic/seqgraphic_utils.hpp>
#include <gui/widgets/seq_graphic/simple_layout_policy.hpp>
#include <gui/widgets/seq_graphic/layered_layout_policy.hpp>
#include <gui/widgets/seq_graphic/seqgraphic_ds_manager.hpp>
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
///   CHapmapTrack
///////////////////////////////////////////////////////////////////////////////

CTrackTypeInfo CHapmapTrack::m_TypeInfo("HapMapRR_track",
                                        "A haplotype map of the human genome \
which describes the common patterns of human DNA sequence variation.");

CHapmapTrack::CHapmapTrack(CSGHapmapDS* ds, CRenderingContext* r_cntx)
    : CDataTrack(r_cntx)
    , m_DS(ds)
    , m_IconLayout(eIcon_Layout, "Layout", true, "track_layout")
{

    m_DS->SetJobListener(this);

    // initialize annotation selector
    m_FeatSel = CSeqUtils::GetAnnotSelector();
}


CHapmapTrack::~CHapmapTrack()
{
    if ( !m_DS->AllJobsFinished() ) {
        m_DS->DeleteAllJobs();
    }
}

bool CHapmapTrack::x_IsOverviewMode() const
{
    static double kNPP_Cutoff= 50;  // Nucliotides per pixel cutoff for SNP Overview
    return (m_Context->GetScale() > kNPP_Cutoff);
}

/// @name CLayoutTrack reimplemented methods.
/// @{
const CTrackTypeInfo& CHapmapTrack::GetTypeInfo() const
{
    return m_TypeInfo;
}

void CHapmapTrack::x_OnIconClicked(TIconID id)
{
    bool bIsLayoutOnlyChange = true;

    if (id == eIcon_Layout) {
        wxMenu menu;        

        //menu.AppendRadioItem(eLayout_Default,   wxT(("Default"));
        menu.AppendRadioItem(eLayout_Labels,    wxT("Labels"));

        menu.Check(m_eLayout, true);        

        m_LTHost->LTH_PopupMenu(&menu);
        
        wxMenuItemList& item_list = menu.GetMenuItems();
        ITERATE (wxMenuItemList, iter, item_list) {
            ELayout new_layout = (ELayout)((*iter)->GetId());
            if ((*iter)->IsChecked()  &&  id != m_eLayout) {

                // Reload data if switching back or forth to Labels
                if ( (m_eLayout ==eLayout_Labels) ||
                     (new_layout==eLayout_Labels) )
                {
                    bIsLayoutOnlyChange = false;
                }

                m_eLayout = new_layout;
                break; // stop processing loop
            }
        }
    } else {
        // use default handlers
        CLayoutTrack::x_OnIconClicked(id);
    }

    Update(bIsLayoutOnlyChange);
    x_OnLayoutChanged();
}
/// @}


void CHapmapTrack::x_LoadSettings(const string& /*preset_style*/,
                                  const TKeyValuePairs& /*settings*/)
{
    m_Layered->SetVertSpace(m_gConfig->GetObjectSpace());
}


/// @name CDataTrack pure virtual interfaces
/// @{

void CHapmapTrack::x_UpdateData()
{
    bool bShowLabels = (m_eLayout == eLayout_Labels);
    m_DS->DeleteAllJobs();
    x_SetStartStatus();
    m_DS->LoadData(m_Context->GetVisSeqRange(), 
                   m_Context->GetScale(),
                   bShowLabels,   // fetch features if we show labels.
                   x_IsOverviewMode());    
}


void CHapmapTrack::x_OnJobCompleted(CAppJobNotification& notify)
{
    m_DS->ClearJobID(notify.GetJobID());
    CRef<CObject> res_obj = notify.GetResult();
    SHapmapJobResult* result = dynamic_cast<SHapmapJobResult*>(&*res_obj);
    if (!result) {
        ERR_POST("CHapmapTrack::x_OnJobCompleted() notification for job \
                does not contain results.");
        return;
    }
    
    // Depending on view, setup icon
    if (x_IsOverviewMode())
        x_DeregisterIcon(m_IconLayout.m_Id);
    else
        x_RegisterIcon(m_IconLayout);

    m_Msg = "";
    /*
    switch (result->eDataType) {
    default:
    case SHapmapJobResult::eHistFromFeats:
    m_Msg += ", density (seq-feats)";                
    break;
    case SHapmapJobResult::eHistFromGraphs:
    m_Msg += ", density (seq-graphs)";
    break;
    case SHapmapJobResult::eFeats: 
    {
    unsigned int size = (unsigned int) result->listObjs.size();
    m_Msg += ", " + NStr::UIntToString(size) + " features";
    }
    break;
    }
    */
    x_CreateLayout(*result);
}

/// @}

void CHapmapTrack::x_CreateLayout(const SHapmapJobResult& result)
{
    const CSeqGlyph::TObjects& objs = result.listObjs;    

    if (objs.empty()) {
        Update(true);
        x_OnLayoutChanged();
        return;
    }

    m_Group.Clear();

    ITERATE(CSeqGlyph::TObjects, it, objs) {    
        CSeqGlyph *glyph = const_cast<CSeqGlyph*> (it->GetPointer());
        CHistogramGlyph *hist= dynamic_cast<CHistogramGlyph *>(glyph);        
        hist->SetConfig(*x_GetGlobalConfig());
        Add(glyph);
    }
    

    Update(true);
    x_OnLayoutChanged();
}



///////////////////////////////////////////////////////////////////////////////
/// CHapmapTrackFactory
///////////////////////////////////////////////////////////////////////////////
ILayoutTrackFactory::TTrackMap
CHapmapTrackFactory::CreateTracks(SConstScopedObject& object,
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
        CSGHapmapDS::GetTrackNames(object, annots, r_cntx->GetVisSeqRange(), params);
        
        // collect NA tracks
        if ( !src_annots.empty() ) {
            GetMatchedAnnots(src_annots, params, annots);
        }
    }

    // create feature tracks
    ITERATE (TAnnotNameTitleMap, iter, annots) {
        // Create a new DS object for each track
        CIRef<ISGDataSource> pre_ds = ds_context->GetDS(typeid(CSGHapmapDSType).name(), 
                                                    object);
        CSGHapmapDS* seq_ds = dynamic_cast<CSGHapmapDS*>(pre_ds.GetPointer());
        seq_ds->SetDepth(params.m_Level);
        seq_ds->SetAdaptive(params.m_Adaptive);
        seq_ds->SetName(iter->first);

        CRef<CHapmapTrack> track(new CHapmapTrack(seq_ds, r_cntx));
        if ( !iter->second.empty() ) {
            track->SetTitle(iter->second);
        }
        tracks[iter->first] = track.GetPointer();
    }

    return tracks;
}

void CHapmapTrackFactory::GetMatchedAnnots(
    const TAnnotMetaDataList& src_annots,
    const ILayoutTrackFactory::SExtraParams& params,
    TAnnotNameTitleMap& out_annots) const
{
    ILayoutTrackFactory::GetMatchedAnnots(src_annots, params.m_Annots, "graph", "HapMapRR", out_annots);
}

CRef<CTrackConfigSet>
CHapmapTrackFactory::GetSettings(const string& /*profile*/,
                                 const TKeyValuePairs& settings,
                                 const CTempTrackProxy* /*track_proxy*/) const
{
    CRef<CTrackConfigSet> config_set(new CTrackConfigSet);
    CRef<CTrackConfig> config(new CTrackConfig);
    config_set->Set().push_back(config);
    config->SetHelp() = GetThisTypeInfo().GetDescr();
    return config_set;
}

string CHapmapTrackFactory::GetExtensionIdentifier() const
{
    return CHapmapTrack::m_TypeInfo.GetId();
}

string CHapmapTrackFactory::GetExtensionLabel() const
{
    return CHapmapTrack::m_TypeInfo.GetDescr();
}


END_NCBI_SCOPE
