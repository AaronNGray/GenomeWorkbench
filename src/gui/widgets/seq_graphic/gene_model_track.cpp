/*  $Id: gene_model_track.cpp 44986 2020-05-01 22:45:19Z rudnev $
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

#include <gui/widgets/seq_graphic/gene_model_track.hpp>
#include <gui/widgets/seq_graphic/gene_model_group.hpp>
#include <gui/widgets/seq_graphic/seqgraphic_ds_manager.hpp>
#include <gui/widgets/seq_graphic/seqgraphic_utils.hpp>
#include <gui/widgets/seq_graphic/config_utils.hpp>
#include <gui/widgets/seq_graphic/layout_track_proxy.hpp>
#include <gui/widgets/seq_graphic/rendering_ctx.hpp>
#include <gui/widgets/seq_graphic/histogram_glyph.hpp>
#include <gui/widgets/seq_graphic/feature_track.hpp>
#include <gui/widgets/wx/wx_utils.hpp>

#include <gui/utils/rgba_color.hpp>
#include <gui/objutils/utils.hpp>
#include <gui/objutils/label.hpp>
#include <gui/objutils/registry.hpp>

#include <objmgr/util/feature.hpp>
#include <objmgr/util/sequence.hpp>
#include <util/sequtil/sequtil_manip.hpp>

#include <objects/seq/Seq_descr.hpp>
#include <objects/seq/Seqdesc.hpp>
#include <objects/general/Dbtag.hpp>
#include <objects/general/Object_id.hpp>

#include "search_utils.hpp"

#include <wx/string.h>
#include <wx/menu.h>

BEGIN_NCBI_SCOPE
USING_SCOPE(objects);


///////////////////////////////////////////////////////////////////////////////
///   CGeneModelTrack
///////////////////////////////////////////////////////////////////////////////

// content menu item base id
static const size_t kContentBaseID = 10000;
// potential maximal menu item id range
// assumming there will be less than 100 menu items
static const int kMaxID = 100;
// GeneID's DB name
static const string kGeneIdDb("GeneID");


class CGeneContentEvtHandler : public wxEvtHandler
{
public:
    CGeneContentEvtHandler(CGeneModelTrack* track)
        : m_Track( track )
    {}

    void OnContextMenu( wxContextMenuEvent& anEvent );

private:
    /// @name event handlers.
    /// @{
    /// on toggle a conent menu item.
    void x_OnToggleContent(wxCommandEvent& event)
    {
        int track_order = event.GetId() - kContentBaseID;
        m_Track->OnToggleContent(track_order);
    }

    /// @}

private:
    CGeneModelTrack* m_Track;

    DECLARE_EVENT_TABLE()
};


BEGIN_EVENT_TABLE(CGeneContentEvtHandler, wxEvtHandler)
    EVT_MENU_RANGE(kContentBaseID, kContentBaseID + kMaxID, CGeneContentEvtHandler::x_OnToggleContent)
END_EVENT_TABLE()


static const size_t kCompactThreshold = 500;
static const size_t kMaxGenesOverviewRows = 20;
static const size_t kMaxCompactRows = 100;
static const string kTrackName = "Gene models";
static const string kBaseKey = "GBPlugins.SeqGraphicGeneModel";

CTrackTypeInfo CGeneModelTrack::m_TypeInfo("gene_model_track", 
                                           "A track for showing four main \
features: gene, RNAs, coding regions and Exons. A Gene model is a group of \
linked gene, RNAs, coding regions and Exons.  A gene model may contain one \
or more pairs of linked RNA and coding region.");


CGeneModelTrack::CGeneModelTrack(CSGFeatureDS* ds, CRenderingContext* r_cntx)
    : CDataTrack(r_cntx)
    , m_ContentHandler(new CGeneContentEvtHandler(this))
    , m_DS(ds)
    , m_Compact(new CCompactLayout)
    , m_AnnotNameUpdated(false)
{
    m_DS->SetJobListener(this);

    // initialize annotation selector for land mark features
    m_LandMarkFeatSel = CSeqUtils::GetAnnotSelector();
    m_LandMarkFeatSel.IncludeFeatType(CSeqFeatData::e_Gene);

    // initialize annotation selector for main features
    m_MainSel = CSeqUtils::GetAnnotSelector();

    // initialize annotation selector for CDS product features
    m_cdsFeatSel = CSeqUtils::GetAnnotSelector();
    ITERATE(CFeatList, iter, *CSeqFeatData::GetFeatList()) {
        CSeqFeatData::ESubtype subtype =
            static_cast<CSeqFeatData::ESubtype>(iter->GetSubtype());
        if (subtype == CSeqFeatData::eSubtype_any  ||
            subtype == CSeqFeatData::eSubtype_variation  ||
            subtype == CSeqFeatData::eSubtype_source  ||
            subtype == CSeqFeatData::eSubtype_repeat_region  ||
            subtype == CSeqFeatData::eSubtype_repeat_unit  ||
            subtype == CSeqFeatData::eSubtype_gene  ||
            subtype == CSeqFeatData::eSubtype_prot) {
                continue;
        }
        m_cdsFeatSel.IncludeFeatSubtype(subtype);
    }

    x_RegisterIcon(SIconInfo(eIcon_Content, "Content", true, "track_content"));
    x_RegisterIcon(
        SIconInfo(eIcon_Layout, "Layout style", true, "track_layout"));
}


CGeneModelTrack::~CGeneModelTrack()
{
    if ( !m_DS->AllJobsFinished() ) {
        m_DS->DeleteAllJobs();
    }
}


const CTrackTypeInfo& CGeneModelTrack::GetTypeInfo() const
{
    return m_TypeInfo;
}


string CGeneModelTrack::GetFullTitle() const
{
    string title = GetTitle();
    if (title.empty()) {
        if (CSeqUtils::IsUnnamed(m_AnnotName)) {
            title = "NCBI genes";
        } else {
            title = m_AnnotName;
        }
    } 

    return title;
}


void CGeneModelTrack::SetAnnot(const string& annot)
{
    m_AnnotName = annot;
    m_RealAnnot = annot;
    m_LandMarkFeatSel.ResetAnnotsNames();
    m_MainSel.ResetAnnotsNames();
    if (CSeqUtils::NameTypeStrToValue(m_AnnotName) ==
        CSeqUtils::eAnnot_Unnamed) {
        m_LandMarkFeatSel.AddUnnamedAnnots();
        m_MainSel.AddUnnamedAnnots();
    } else {
        m_MainSel.AddNamedAnnots(m_AnnotName);
        m_MainSel.ExcludeUnnamedAnnots();
        m_LandMarkFeatSel.AddNamedAnnots(m_AnnotName);
        m_LandMarkFeatSel.ExcludeUnnamedAnnots();
        if (NStr::StartsWith(m_AnnotName, "NA0")) {
            if (m_AnnotName.find(".") == string::npos) {
                m_AnnotName += ".1";
            }
            m_MainSel.IncludeNamedAnnotAccession(m_AnnotName);
            m_LandMarkFeatSel.IncludeNamedAnnotAccession(m_AnnotName);
            m_RealAnnot = m_AnnotName;
        }
    }
}

void CGeneModelTrack::GetFeatSubtypes(set<CSeqFeatData::ESubtype> &subtypes) const
{
    for (auto s : m_subtypes)
        subtypes.insert(s);
}

// This method is responsible for selection feature types to be included
// into gene model track. If you modify it, make corresponding changes in
// negative selection counterpart at src/gui/objutils/na_utils.cpp
// CDataTrackUtils::IsGeneModelFeature
void CGeneModelTrack::x_AdjustSelector()
{
    m_MainSel.SetFeatType(CSeqFeatData::e_Gene);
    m_subtypes.clear();
    s_GetSubtypesForType(m_subtypes, CSeqFeatData::e_Gene);

    if (m_Conf->m_ShowRNAs) {
        m_MainSel.IncludeFeatType(CSeqFeatData::e_Rna);
        // RNA subfeature for Imp feature
        m_MainSel.IncludeFeatSubtype(CSeqFeatData::eSubtype_misc_RNA);
        s_GetSubtypesForType(m_subtypes, CSeqFeatData::e_Rna);
        m_subtypes.insert(CSeqFeatData::eSubtype_misc_RNA);
    }

    if (m_Conf->m_ShowCDSs) {
        m_MainSel.IncludeFeatType(CSeqFeatData::e_Cdregion);
        s_GetSubtypesForType(m_subtypes, CSeqFeatData::e_Cdregion);
    }

    if (m_Conf->m_ShowExons) {
        m_MainSel.IncludeFeatSubtype(CSeqFeatData::eSubtype_exon);
        m_subtypes.insert(CSeqFeatData::eSubtype_exon);
    }

    if (m_Conf->m_ShowVDJCs) {
        m_MainSel.IncludeFeatSubtype(CSeqFeatData::eSubtype_C_region);
        m_MainSel.IncludeFeatSubtype(CSeqFeatData::eSubtype_V_segment);
        m_MainSel.IncludeFeatSubtype(CSeqFeatData::eSubtype_D_segment);
        m_MainSel.IncludeFeatSubtype(CSeqFeatData::eSubtype_J_segment);
// Temporary moved back out of gene model track, see SV-2107
//        m_MainSel.IncludeFeatSubtype(CSeqFeatData::eSubtype_enhancer);
        m_subtypes.insert(CSeqFeatData::eSubtype_C_region);
        m_subtypes.insert(CSeqFeatData::eSubtype_V_segment);
        m_subtypes.insert(CSeqFeatData::eSubtype_D_segment);
        m_subtypes.insert(CSeqFeatData::eSubtype_J_segment);
    }
}


void CGeneModelTrack::OnToggleContent(int id)
{
    // the menu item order:
    // 0 - "Hide gene feature bar"
    // 1 - "Show RNAs"
    // 2 - "Show CDSs"
    // 3 - "Show Exons"
    // 4 - "Show VDJ segments"
    // 5 - "Show CDS product features"
    // 6 - "Show Histogram if can't show all genes"
    // 7 - "Show Nt Ruler"
    // 8 - "Show Aa Ruler"



    switch (id) {
        case 0:
            m_Conf->m_ShowGenes = !m_Conf->m_ShowGenes;
            break;
        case 1:
            m_Conf->m_ShowRNAs = !m_Conf->m_ShowRNAs;
            break;
        case 2:
            m_Conf->m_ShowCDSs = !m_Conf->m_ShowCDSs;
            break;
        case 3:
            m_Conf->m_ShowExons = !m_Conf->m_ShowExons;
            break;
        case 4:
            m_Conf->m_ShowVDJCs = !m_Conf->m_ShowVDJCs;
            break;
        case 5:
            m_Conf->m_ShowCDSProductFeats = !m_Conf->m_ShowCDSProductFeats;
            break;
        case 6:
            m_Conf->m_ShowHistogram = !m_Conf->m_ShowHistogram;
            break;
        case 7:
            m_Conf->m_ShowNtRuler= !m_Conf->m_ShowNtRuler;
            break;
        case 8:
            m_Conf->m_ShowAaRuler= !m_Conf->m_ShowAaRuler;
            break;
        default:
            break;
    }

    x_AdjustSelector();
    x_UpdateData();
}


void CGeneModelTrack::x_UpdateData()
{
    CDataTrack::x_UpdateData();
    m_Compact->SetMinDist(max(TSeqPos(1),
        TSeqPos(m_Context->ScreenToSeq(3.0))));

    m_DS->DeleteAllJobs();
    x_SetStartStatus();
    TSeqRange vis_range = x_GetVisRange();
    if (vis_range.Empty()) {
        vis_range = m_Context->GetVisSeqRange();
    }
    m_DS->LoadMainFeatures(m_LandMarkFeatSel, m_MainSel, eJob_LandMarkGenes,
        eJob_MainFeats, m_Conf->m_MergeStyle, 
        m_Context->IsOverviewMode() && GetHighlights().empty() && GetHighlightsByKey().empty(),
        m_Conf->m_OverviewFeatCutoff, vis_range, m_Context->GetScale(),
        m_Conf->m_LandmarkFeatLimit, m_Conf->m_ShowHistogram, m_Conf->m_HighlightMode);
}

 void CGeneModelTrack::GenerateAsn1(CSeq_entry& seq_entry, TSeqRange range)
{
    CSeqUtils::SetResolveDepth(m_MainSel, true, -1);
    CBioseq_Handle& Handle = m_DS->GetBioseqHandle();
    CConstRef<CSeq_loc> loc(
        Handle.GetRangeSeq_loc(range.GetFrom(), range.GetTo()) );

//    cerr << "using loc " << endl << 
//            MSerial_AsnText << *loc << endl;
    CFeat_CI feature_iter(Handle.GetScope(), *loc, m_MainSel);
//    cerr << "got features: " << feature_iter.GetSize() << endl;
    CRef<CSeq_annot> pAnnot(new CSeq_annot);
    CSeq_annot::TData::TFtable& ftable(pAnnot->SetData().SetFtable());

    for ( ;  feature_iter;  ++feature_iter) {
        CRef<CSeq_feat> pFeat(new CSeq_feat);
        const CMappedFeat& feat = *feature_iter;
//        cerr << "Got feature" << endl << 
//          MSerial_AsnText << feat.GetMappedFeature() << endl;
        pFeat->Assign(feat.GetMappedFeature());
        ftable.push_back(pFeat);
    }
    seq_entry.SetAnnot().push_back(pAnnot);
}

void CGeneModelTrack::x_OnJobCompleted(CAppJobNotification& notify)
{
    m_DS->ClearJobID(notify.GetJobID());
    CRef<CObject> res_obj = notify.GetResult();
    CJobResultBase* result = dynamic_cast<CJobResultBase*>(&*res_obj);
    if (!result) {
        LOG_POST(Error << "CGeneModelTrack::x_OnJobCompleted() notification for job \
                does not contain results.");
        return;
    }

    EJobType type = (EJobType)result->m_Token;
    switch (type) {
    case eJob_LandMarkGenes:
        x_AddLandMarkFeaturesLayout(dynamic_cast<CSGJobResult&>(*res_obj));
        break;
    case eJob_MainFeats:
        x_AddMainFeaturesLayout(dynamic_cast<CSGJobResult&>(*res_obj));
        break;
    case eJob_FeatProducts:
        x_AddFeatProductLayout(dynamic_cast<CBatchJobResult&>(*res_obj));
        break;
    default:
        break;
    } // switch
    

}


void CGeneModelTrack::x_LoadSettings(const string& preset_style,
                                     const TKeyValuePairs& settings)
{
    if ( !m_Conf ) {
        m_Conf.Reset(new CGeneModelConfig);
    }

    if (preset_style.empty()) {
        SetProfile("Default");
    } else {
        SetProfile(preset_style);
    }

    m_Conf->LoadSettings(x_GetGlobalConfig(), kBaseKey, GetProfile());
    m_Conf->UpdateSettings(settings);

    x_AdjustSelector();

    m_Layered->SetVertSpace(m_gConfig->GetObjectSpace());
    m_Compact->SetVertSpace(m_gConfig->GetObjectSpace() + 2);
}


void CGeneModelTrack::x_SaveSettings(const string& preset_style)
{
    SetProfile(m_Conf->SaveSettingsAsString(preset_style));
}


void CGeneModelTrack::x_OnIconClicked(TIconID id)
{
    switch (id) {
    case eIcon_Content:
        x_OnContentIconClicked();
        break;
    case eIcon_Layout:
        x_OnLayoutIconClicked();
        break;
    default:
        // use default handlers
        CLayoutTrack::x_OnIconClicked(id);
    }
}


void CGeneModelTrack::x_OnContentIconClicked()
{
    wxMenu menu;
    int curr_id = 0;
    wxMenuItem* item;
    item = menu.AppendCheckItem(kContentBaseID + curr_id++,
        wxT("Hide gene feature bar"));
    if ( !m_Conf->m_ShowGenes ) {
        item->Check();
    }

    item = menu.AppendCheckItem(kContentBaseID + curr_id++, wxT("Show RNAs"));
    if (m_Conf->m_ShowRNAs) {
        item->Check();
    }

    item = menu.AppendCheckItem(kContentBaseID + curr_id++, wxT("Show CDSs"));
    if (m_Conf->m_ShowCDSs) {
        item->Check();
    }

    item = menu.AppendCheckItem(kContentBaseID + curr_id++, wxT("Show Exons"));
    if (m_Conf->m_ShowExons) {
        item->Check();
    }

    item = menu.AppendCheckItem(kContentBaseID + curr_id++, wxT("Show VDJ segments"));
    if (m_Conf->m_ShowVDJCs) {
        item->Check();
    }

    item = menu.AppendCheckItem(
        kContentBaseID + curr_id++, wxT("Show CDS product features"));
    if (m_Conf->m_ShowCDSProductFeats) {
        item->Check();
    }

    item = menu.AppendCheckItem(
        kContentBaseID + curr_id++, wxT("Show Histogram if can't show all genes"));
    if (m_Conf->m_ShowHistogram) {
        item->Check();
    }
    /*
    item = menu.AppendCheckItem(
        kContentBaseID + curr_id++, wxT("Show product ruler"));
    if (m_Conf->m_ShowNtRuler) {
        item->Check();
    }

    item = menu.AppendCheckItem(
        kContentBaseID + curr_id++, wxT("Show protein ruler"));
    if (m_Conf->m_ShowAaRuler) {
        item->Check();
    }
    */
    m_LTHost->LTH_PushEventHandler(m_ContentHandler);
    m_LTHost->LTH_PopupMenu(&menu);
    m_LTHost->LTH_PopEventHandler();
}


void CGeneModelTrack::x_OnLayoutIconClicked()
{
    typedef map <CGeneModelConfig::EMergeStyle, wxString> TStyleMap;
    TStyleMap styles;
    styles[CGeneModelConfig::eMerge_No] = wxT("Don't merge (Default)");
    styles[CGeneModelConfig::eMerge_Pairs] = wxT("Merge matched RNA + proteins pair");
    styles[CGeneModelConfig::eMerge_All] = wxT("Merge all RNAs and proteins");
    styles[CGeneModelConfig::eMerge_OneLine] = wxT("Merge everything as one line");


    wxMenu menu;
    UseDefaultMarginWidth(menu);

    int id_base = 10000;
    ITERATE (TStyleMap, iter, styles) {
        wxMenuItem* item = menu.AppendRadioItem(id_base + iter->first,
            iter->second);
        item->Check(m_Conf->m_MergeStyle == iter->first);
    }

    m_LTHost->LTH_PopupMenu(&menu);
    wxMenuItemList& item_list = menu.GetMenuItems();
    ITERATE (wxMenuItemList, iter, item_list) {
        CGeneModelConfig::EMergeStyle style =
            (CGeneModelConfig::EMergeStyle)((*iter)->GetId() - id_base);
        if ((*iter)->IsChecked()  &&  style != m_Conf->m_MergeStyle) {
            m_Conf->m_MergeStyle = style;
            x_UpdateData();
        }
    }
}


//static const size_t   kLabelsColWidth  = 100;  // width of a column small

struct SFeatureByName
{
    bool operator() (CRef<CSeqGlyph> o1, CRef<CSeqGlyph> o2)
    {
        CFeatGlyph* f1  = dynamic_cast<CFeatGlyph*>(o1.GetPointer());
        CFeatGlyph* f2  = dynamic_cast<CFeatGlyph*>(o2.GetPointer());

        string label1;
        objects::feature::GetLabel(f1->GetFeature(), &label1,
                                   objects::feature::fFGL_Content);

        string label2;
        objects::feature::GetLabel(f2->GetFeature(), &label2,
                                   objects::feature::fFGL_Content);

        return label1 < label2;
    }
};


void CGeneModelTrack::x_AddLandMarkFeaturesLayout(CSGJobResult& result)
{
    //TIME_ME("x_AddLandMarkFeaturesLayout()");
    SetGroup().Clear();

    CSeqGlyph::TObjects objs = result.m_ObjectList;
    size_t count = objs.size();
    
    if (count == 0) {
        // an empty track should be navigable as there might be out of range objects
        m_Attrs |= fNavigable;
        x_UpdateLayout();
        return;
    }

    if ( !m_AnnotNameUpdated ) {
        CFeatGlyph* feat_glyph =
            dynamic_cast<CFeatGlyph*>(objs.front().GetPointer());
        m_AnnotName =
            CSeqUtils::GetAnnotName(feat_glyph->GetMappedFeature().GetAnnot());
        m_AnnotNameUpdated = true;
    }

    CConstRef<CSeqGraphicConfig> g_config = x_GetGlobalConfig();
    CConstRef<CFeatureParams> gene_conf =
        g_config->GetFeatParams(CSeqFeatData::eSubtype_gene);

    CLayoutGroup::TObjectList the_genes;
    NON_CONST_ITERATE (CLayoutGroup::TObjects, iter, objs) {
        CFeatGlyph* feat_glyph = dynamic_cast<CFeatGlyph*>(iter->GetPointer());
        if (!feat_glyph)
            continue;
        feat_glyph->SetConfig(gene_conf);
        if ((!GetHighlights().empty() || !GetHighlightsByKey().empty()) &&  x_NeedHighlight(*feat_glyph)) {
            feat_glyph->SetHighlighted(true);
            if (m_HighlightsColor.get())
                feat_glyph->SetHighlightsColor(*m_HighlightsColor.get());
        } else if ( !feat_glyph->IsHighlighted() ) {
            feat_glyph->SetHideLabel(true);
        }
        the_genes.push_back(*iter);
    }

    TTrackAttrFlags attr = fShowAlways | fFullTrack;
    if (g_config->GetShowComments()) {
        attr |= fShowTitle;
    }

    CGlyphContainer* container = this;
    string gene_track_label;
    if ( g_config->GetShowSimpleComments() ) {
        string count_str = NStr::SizetToString(count);
        if (count >= (size_t)m_Conf->m_LandmarkFeatLimit) {
            gene_track_label += count_str + " landmark genes shown";
        } else {
            gene_track_label += "all " + count_str + " genes shown";
        }
    }

    // Add histogram if avaliable
    if (result.m_ExtraObj.NotEmpty()) {
        SetLayoutPolicy(m_Simple);
        SetMsg(", Overview mode");

        CGlyphContainer* hist_track = new CGlyphContainer(m_Context);
        CRef<CSeqGlyph> track_holder(dynamic_cast<CSeqGlyph*>(hist_track));
        Add(track_holder);
        hist_track->SetConfig(m_gConfig);
        hist_track->SetIndent(m_IndentLevel + 1);

        CHistogramGlyph* hist =
            dynamic_cast<CHistogramGlyph*>(result.m_ExtraObj.GetPointer());
        hist->SetDialogHost(dynamic_cast<IGlyphDialogHost*>(m_LTHost));
        hist->SetConfig(*x_GetGlobalConfig());
        hist_track->SetTitle("Genes Distribution Histogram");
        hist_track->SetTrackAttr(attr);
        hist_track->Add(result.m_ExtraObj);

        container = new CGlyphContainer(m_Context);
        CRef<CSeqGlyph> glyph(dynamic_cast<CSeqGlyph*>(container));
        Add(glyph);
        container->SetConfig(m_gConfig);
        container->SetLayoutPolicy(m_Layered);
        container->SetIndent(m_IndentLevel + 1);
        container->SetTitle(gene_track_label);
        container->SetTrackAttr(attr);
    } else {
        SetLayoutPolicy(m_Layered);
        SetMsg(", Overview mode, " + gene_track_label);
    }
    m_Attrs |= fNavigable;
    container->SetObjects(the_genes);

    // Compress Landmark Genes layout if there are more than
    // kMaxGenesOverviewRows rows in the layout
    m_Layered->SetMaxRow(kMaxGenesOverviewRows);

    if (m_Conf->m_ShowLabel && !result.m_CommentObjs.empty() ) {
        CRef<CCommentConfig> cmt_config(new CCommentConfig);
        cmt_config->m_LineColor.Set(0.5f, 0.5f, 0.5f);
        cmt_config->m_Centered = false;
        cmt_config->m_LabelColor.Set(0.1f, 0.1f, 0.1f);
        SetLabelGlyphs(result.m_CommentObjs, cmt_config);
    }

    // add labels
    /*
    if (m_Conf->m_ShowLMGeneLabel) {
        // sort feature labels alphabetically
        sort(the_genes.begin(), the_genes.end(), SFeatureByName());

        // a number of columns that will fit
        int cols_count = m_Context->GetViewPortWidth() / kLabelsColWidth;
        cols_count = max(1, cols_count);  // always at least 1 column

        // create all row first
        CLayout layout_labels;
        for (size_t i = 0; i < the_genes.size() / cols_count + 1; i++) {
            layout_labels.AddRow();  // add empty row to layout
        }

        // assign columns and repack
        size_t cur_row = 0;
        size_t cur_col = 0;
        NON_CONST_ITERATE (CLayoutGroup::TObjectList, iter, the_genes) {
            CSeqGlyph* tmp = *iter;
            CFeatGlyph* feat  = dynamic_cast<CFeatGlyph*>(tmp);
            CRef<CLayoutFeatLabel> lbl
                (new CLayoutFeatLabel(feat->GetMappedFeature()));

            TVPUnit start = cur_col * kLabelsColWidth + 1;
            TSeqPos from  = m_VisRange.GetFrom() + TSeqPos ( m_SeqScale * start);
            TSeqPos to    = m_VisRange.GetFrom() + TSeqPos ( m_SeqScale * (start + kLabelsColWidth) );

            // set the column index
            lbl->SetColumn(m_OrigScale < 0 ? cols_count - cur_col - 1 : cur_col);

            // set a range in model coordinates
            lbl->SetRange(from, to);

            CLayout::TLayoutRow& the_row = layout_labels.SetRow(cur_row);
            CRef<CSeqGlyph> obj(lbl);
            the_row.push_back(obj);
            cur_row++;

            if (cur_row >= layout_labels.GetLayout().size()) {
                cur_row = 0;
                cur_col++;
            }
        }
    }
    */
    x_UpdateLayout();
}


void CGeneModelTrack::x_AddMainFeaturesLayout(const CSGJobResult& result)
{
    //TIME_ME("x_AddMainFeaturesLayout()");
    if (m_Conf->m_HighlightMode != 0) {
        ResetLabelGlyph();
    }

    SetGroup().Clear();
    SetMsg("");
    SetLayoutPolicy(m_Compact);
    if (result.m_ObjectList.empty()) {
        // an empty track should be navigable as there might be out of range objects
        m_Attrs |= fNavigable;
        x_UpdateLayout();
        return;
    }

    // make sure we remove the constraint on maximal number of row set
    // in overview mode.
    m_Layered->SetMaxRow(kMaxCompactRows);

    SetObjects(result.m_ObjectList);

    //  Turning off label for mRNA and CDS if they are not the first ones,
    //  not side labeling, and not forced to show all.
    //      a) First
    //      b) Longest (always first, actually)
    //      c) Most exons (turns our this is also first one)
    //    LinkFeatures works that way
    
    //  set rendering context and corresponding configs if any

    // prepare feature configs
    TFeatConfigMap feat_confs;
    feat_confs[CSeqFeatData::eSubtype_gene] =
        m_gConfig->GetFeatParams(CSeqFeatData::eSubtype_gene);
    feat_confs[CSeqFeatData::eSubtype_mRNA] =
        m_gConfig->GetFeatParams(CSeqFeatData::eSubtype_mRNA);
    feat_confs[CSeqFeatData::eSubtype_cdregion] =
        m_gConfig->GetFeatParams(CSeqFeatData::eSubtype_cdregion);
    feat_confs[CSeqFeatData::eSubtype_exon] =
        m_gConfig->GetFeatParams(CSeqFeatData::eSubtype_exon);
    feat_confs[CSeqFeatData::eSubtype_mat_peptide_aa] =
        m_gConfig->GetFeatParams(CSeqFeatData::eSubtype_mat_peptide_aa);

    size_t size = GetChildren().size();
    bool compact_mode = size > kCompactThreshold && GetHighlights().empty() && GetHighlightsByKey().empty();
    if (compact_mode) {
        // for large number of features, we use
        //  - less expensive layout style
        //  - faster layout in terms of rendering
        SetLayoutPolicy(m_Layered);
    }

    TFeatBatchJobRequests requests;
    x_InitFeature_Recursive(SetChildren(), feat_confs, NULL, requests, compact_mode);

    SetMsg(", " + NStr::SizetToString(size) +
        " gene model" + (size > 1 ? "s shown" : " shown"));
    if ( !requests.empty() ) {
        m_DS->LoadFeatures(m_Context->GetVisSeqRange(),
            m_Context->GetScale(), eJob_FeatProducts, requests);
    }

    x_UpdateLayout();
}


void CGeneModelTrack::x_AddCDSProduct_FeatsLayout(CSGJobResult& result)
{
    //TIME_ME("x_AddCDSProduct_FeatsLayout()");

    SetMsg("");
    const CSeqGlyph::TObjects& objs = result.m_ObjectList;
    size_t obj_num = objs.size();
    if (obj_num == 0  ||  !result.m_Owner) {
        m_Attrs |= fNavigable;
        return;
    }

    CSeqGlyph* p_feat = result.m_Owner;
    CLayoutGroup* parent_group =
        dynamic_cast<CLayoutGroup*>(p_feat->SetParent());
    _ASSERT(parent_group);

    // need a group to hold it
    CRef<CLayoutGroup> group(new CLayoutGroup());
    group->SetLayoutPolicy(m_Layered);
    parent_group->InsertAft(p_feat, group);
    group->Set(objs);

    NON_CONST_ITERATE (CSeqGlyph::TObjects, iter, group->SetChildren()) {
        CFeatGlyph* feat = dynamic_cast<CFeatGlyph*>(iter->GetPointer());
        feat->SetConfig(m_gConfig->GetFeatParams(
            feat->GetFeature().GetData().GetSubtype()));
    }
    if (m_DS->IsBackgroundJob()) {
        // udpate children's layout
        group->Update(true);
        // update parent's layout
        group->UpdateLayout();
    }

}


void CGeneModelTrack::x_AddFeatProductLayout(CBatchJobResult& results)
{
    //TIME_ME("x_AddFeatProductLayout()");

    SetMsg("");
    if (results.m_Results.empty()) {
        m_Attrs |= fNavigable;
        return;
    }

    NON_CONST_ITERATE (CBatchJobResult::TResults, iter, results.m_Results) {
        CSGJobResult& result = **iter;
        if ((result.m_ObjectList.empty()  &&  !result.m_ExtraObj)  ||
            !result.m_Owner) {
            continue;
        }

        CSeqGlyph* p_feat = result.m_Owner;
        CLayoutGroup* parent_group =
            dynamic_cast<CLayoutGroup*>(p_feat->SetParent());
        _ASSERT(parent_group);

        if ( !result.m_ObjectList.empty() ) {
            // need a group to hold it
            CRef<CLayoutGroup> group(new CLayoutGroup());
            parent_group->InsertAft(p_feat, group);
            group->Set(result.m_ObjectList);

            NON_CONST_ITERATE (CSeqGlyph::TObjects, c_iter, group->SetChildren()) {
                CFeatGlyph* feat = dynamic_cast<CFeatGlyph*>(c_iter->GetPointer());
                feat->SetConfig(m_gConfig->GetFeatParams(feat->GetFeature().GetData().GetSubtype()));
                feat->SetProjectedFeat(true);
                if (m_Conf->m_ShowLabel == false) 
                    feat->SetHideLabel(true);
            }

            group->SetLayoutPolicy(m_Layered);

            parent_group = group;
        } else {
            CHistogramGlyph* hist =
                dynamic_cast<CHistogramGlyph*>(result.m_ExtraObj.GetPointer());
            hist->SetDialogHost(dynamic_cast<IGlyphDialogHost*>(m_LTHost));
            hist->SetConfig(*m_gConfig);
            parent_group->InsertAft(p_feat, hist);
        }

        if (m_DS->IsBackgroundJob()) {
            // udpate children's layout
            parent_group->Update(true);
        }
    }

    if (m_DS->IsBackgroundJob()) {
        // update parent's layout
        x_UpdateLayout();
    }
}


bool CGeneModelTrack::x_NeedHighlight(const CFeatGlyph &feat_glyph) const
{
    string label;
    feat_glyph.GetLabel(label, CLabel::eContent);
    NStr::ToLower(label);
    
    if (GetHighlights().count(label) > 0) 
        return true;

    do
    {
        if (GetHighlightsByKey().empty())
            break;
            
        const CSeq_feat &feat = feat_glyph.GetFeature();
        if (CSeqFeatData::eSubtype_gene != feat.GetData().GetSubtype())
            break;

        const CGlyphContainer::TKeyHighlights &dbxrefHighlights = GetHighlightsByKey();
        string geneIdKey(kGeneIdDb);
        NStr::ToLower(geneIdKey);
        const CGlyphContainer::TKeyHighlights::const_iterator it_geneid = dbxrefHighlights.find(geneIdKey);
        if (dbxrefHighlights.end() == it_geneid)
            break;

        CConstRef< CDbtag > id_tag = feat.GetNamedDbxref(kGeneIdDb);
        if (id_tag.Empty() || !id_tag->IsSetTag())
            break;

        CNcbiOstrstream ss_tag;
        id_tag->GetTag().AsString(ss_tag);
        if (it_geneid->second.find((string)CNcbiOstrstreamToString(ss_tag)) != it_geneid->second.end())
            return true;
    }
    while (false);

    return false;
}
 
bool CGeneModelTrack::x_InitFeature_Recursive(CLayoutGroup::TObjectList& objs,
                                              TFeatConfigMap& feat_confs,
                                              const CFeatGlyph* parent_gene,
                                              TFeatBatchJobRequests& requests,
                                              bool compact_mode)
{
    bool need_highlight = false;
    CLayoutGroup::TObjectList::iterator iter = objs.begin();
    while (iter != objs.end()) {
        /// it is possible that the objs size will be increased within the 
        /// iteration when the job is executed in the foreground thread. In
        /// order to keep the iterator point to the right element, we need
        /// to know exactly how many new elements are added between two
        /// iterations.
        size_t obj_cnt = objs.size();

        CSeqGlyph* glyph = iter->GetPointer();
        glyph->SetRenderingContext(m_Context);
        if (CGeneGroup* group = dynamic_cast<CGeneGroup*>(glyph)) {
            group->SetConfig(m_Conf->m_GeneBoxConfig);
            group->SetLayoutPolicy(m_Simple);
            if (m_Conf->m_MergeStyle == CGeneModelConfig::eMerge_OneLine) {
                group->HideGeneAndLabel();
            } else if ( !compact_mode ) {
                // follow the setting to hide gene feature bar 
                // only not in compact mode
                if (!m_Conf->m_ShowLabel || ((CFeatureParams::ePos_NoLabel == feat_confs[CSeqFeatData::eSubtype_gene]->m_LabelPos) && !m_Conf->m_ShowGenes))
                   group->HideGeneAndLabel();
                else
                   group->SetShowGene(m_Conf->m_ShowGenes);
            }
            const CSeqGlyph* first_glyph = group->SetChildren().front();
            const CFeatGlyph* feat = dynamic_cast<const CFeatGlyph*>(first_glyph);
            _ASSERT(feat);
            if (x_InitFeature_Recursive(group->SetChildren(),
                feat_confs, feat, requests, compact_mode)) {
                need_highlight = true;
                group->SetHighlighted(true);
                if (m_HighlightsColor.get())
                    group->SetHighlightsColor(*m_HighlightsColor.get());
            }

        } else if (CLinkedFeatsGroup* group = dynamic_cast<CLinkedFeatsGroup*>(glyph)) {
            group->SetParentFeat(parent_gene);
            group->SetLayoutPolicy(m_Simple);
            group->SetConfig(m_gConfig->GetFeatParams("RnaCds"));
            group->SetGroup().SetRenderingContext(m_Context);
            group->SetShowLabel(m_Conf->m_ShowLabel);
            if (x_InitFeature_Recursive(group->SetChildren(),
                feat_confs, NULL, requests, compact_mode)) {
                need_highlight = true;
            }
            if (need_highlight) {
                group->SetHighlighted(true);
                if (m_HighlightsColor.get())
                    group->SetHighlightsColor(*m_HighlightsColor.get());
            }
        } else if (CLayoutGroup* group = dynamic_cast<CLayoutGroup*>(glyph)) {
            // can be a group containing mRNA and CDS or only exons.
            CRef<CSeqGlyph> child0 = group->GetChild(0);
            const CFeatGlyph* feat =
                dynamic_cast<const CFeatGlyph*>(child0.GetPointer());
            _ASSERT(feat);
            if (feat->GetFeature().GetData().GetSubtype() ==
                CSeqFeatData::eSubtype_exon) {
                if (CSGUtils::IsmRNA(m_DS->GetBioseqHandle(), m_DS->GetScope())) {
                    // exons from mRNA very likely abut upon each other, use
                    // layered layout
                    group->SetLayoutPolicy(m_Layered);
                } else {
                    // Otherwise, we apply inline layout for to form compact layout
                    group->SetLayoutPolicy(m_Inline);
                }
            } else {
                // otherwise, we apply simple layout policy
                group->SetLayoutPolicy(m_Simple);
            }            
            if (x_InitFeature_Recursive(group->SetChildren(),
                feat_confs, NULL, requests, compact_mode)) {
                need_highlight = true;
                group->SetHighlighted(true);
                if (m_HighlightsColor.get())
                    group->SetHighlightsColor(*m_HighlightsColor.get());
            }

        } else if (CFeatGlyph* feat = dynamic_cast<CFeatGlyph*>(glyph)){
            m_Attrs |= fNavigable;
            if (compact_mode || m_Conf->m_ShowLabel == false) {
                feat->SetHideLabel(true);
            }
            if ( !m_AnnotNameUpdated) {
                m_AnnotName =
                    CSeqUtils::GetAnnotName(feat->GetMappedFeature().GetAnnot());
                m_AnnotNameUpdated = true;
            }
            if ((!GetHighlights().empty() || !GetHighlightsByKey().empty()) &&  x_NeedHighlight(*feat)) {
                feat->SetHighlighted(true);
                if (m_HighlightsColor.get())
                    feat->SetHighlightsColor(*m_HighlightsColor.get());
                need_highlight = true;
            }
            CSeqFeatData::ESubtype subtype =
                feat->GetFeature().GetData().GetSubtype();
            bool show_product = feat->GetFeature().IsSetProduct()  &&
                m_Conf->m_MergeStyle == CGeneModelConfig::eMerge_No;

            switch (subtype) {
                case CSeqFeatData::eSubtype_exon:
                    feat->SetConfig(feat_confs[CSeqFeatData::eSubtype_exon]);
                    break;
                case CSeqFeatData::eSubtype_cdregion:
                    {{
                        feat->SetConfig(feat_confs[CSeqFeatData::eSubtype_cdregion]);
                        feat->SetRulerType((m_Conf->m_ShowNtRuler ? CFeatGlyph::eNtRuler : CFeatGlyph::eNoRuler) 
                                           | (m_Conf->m_ShowAaRuler ? CFeatGlyph::eAaRuler : CFeatGlyph::eNoRuler));
                        CCdsGlyph* cds_glyph = dynamic_cast<CCdsGlyph*>(feat);
                        cds_glyph->SetCdsConfig(m_Conf->m_CdsConfig);
                        if (show_product) {
                            if (m_Conf->m_ShowCDSProductFeats) {
                                requests.push_back(SFeatBatchJobRequest(feat, m_cdsFeatSel));
                            }
                        }
                    }}
                    break;
                case CSeqFeatData::eSubtype_gene:
                    feat->SetConfig(feat_confs[CSeqFeatData::eSubtype_gene]);
                    break;
                case CSeqFeatData::eSubtype_mRNA:
                default:
                    feat->SetConfig(feat_confs[(subtype == CSeqFeatData::eSubtype_mat_peptide_aa) ? CSeqFeatData::eSubtype_mat_peptide_aa : CSeqFeatData::eSubtype_mRNA]);
                    feat->SetRulerType(m_Conf->m_ShowNtRuler ? CFeatGlyph::eNtRuler : CFeatGlyph::eNoRuler); 
                    break;
            }
            
        }

        for (size_t i = obj_cnt; i <= objs.size(); ++i) {
            ++iter;
        }
    } // while loop

    return need_highlight;
}


void  CGeneModelTrack::x_SaveConfiguration() const
{
    if (m_Conf) {
        m_Conf->SaveSettings(x_GetGlobalConfig(), kBaseKey, GetProfile());
    }
}


///////////////////////////////////////////////////////////////////////////////
/// CGeneModelFactory
///////////////////////////////////////////////////////////////////////////////
ILayoutTrackFactory::TTrackMap
CGeneModelFactory::CreateTracks(SConstScopedObject& object,
                                ISGDataSourceContext* ds_context,
                                CRenderingContext* r_cntx,
                                const SExtraParams& params,
                                const TAnnotMetaDataList& src_annots) const
{
    TSeqRange range = params.m_Range;
    if (range.Empty()) {
        range = r_cntx->GetVisSeqRange();
    }

    const CFeatList& feats(*CSeqFeatData::GetFeatList());
    CIRef<ISGDataSource> pre_ds = ds_context->GetDS(
        typeid(CSGFeatureDSType).name(), object);
    CSGFeatureDS* seq_ds = dynamic_cast<CSGFeatureDS*>(pre_ds.GetPointer());
    seq_ds->SetDepth(params.m_Level);
    seq_ds->SetAdaptive(params.m_Adaptive);

    // check if there is data available
    // gene
    SAnnotSelector sel(CSeqUtils::GetAnnotSelector(params.m_Annots));

    TAnnotNameTitleMap gene_annots;

    if ((params.m_SkipGenuineCheck  ||  !params.m_DataTypeCheck)
        &&  !params.m_Annots.empty())
    {
        ITERATE (SExtraParams::TAnnots, iter, params.m_Annots) {
            gene_annots.insert(TAnnotNameTitleMap::value_type(*iter, ""));
        }
    } else {
        sel.SetFeatType(CSeqFeatData::e_Gene);
        seq_ds->GetAnnotNames(sel, range, gene_annots);
        // collect NA tracks
        if ( !src_annots.empty() ) {
            ILayoutTrackFactory::GetMatchedAnnots(src_annots, params.m_Annots, "ftable",
                feats.GetStoragekey(CSeqFeatData::eSubtype_gene), gene_annots);
        }
    }

    // do something differently if there is any filters set
    if ( !params.m_Filter.empty() ) {
        return x_CreateTracksWithFilters(object, ds_context,
            r_cntx, params, gene_annots);
    }

    TTrackMap tracks;
    if (!params.m_SkipGenuineCheck) {

        const string unnamed_a = CSeqUtils::GetUnnamedAnnot();
        bool need_unnamed_track = false;
        bool has_unnamed_track = gene_annots.count(unnamed_a) > 0;
        
        // rna, there might be more annots for RNA than gene
        list<int> rna_subtypes;
        ITERATE (CFeatList, iter, feats) {
            if (iter->GetType() == CSeqFeatData::e_Rna  &&
                iter->GetSubtype() != CSeqFeatData::eSubtype_any) {
                rna_subtypes.push_back(iter->GetSubtype());
            }
        }
        
        TAnnotNameTitleMap rna_annots;
        sel.SetFeatType(CSeqFeatData::e_Rna);
        seq_ds->GetAnnotNames(sel, range, rna_annots);
        // collect NA tracks
        if ( !src_annots.empty() ) {
            ITERATE (list<int>, st_iter, rna_subtypes) {
                ILayoutTrackFactory::GetMatchedAnnots(src_annots, params.m_Annots, "ftable",
                    feats.GetStoragekey(*st_iter), rna_annots);
            }
        }
        
        if ( !has_unnamed_track  &&  rna_annots.count(unnamed_a) > 0 ) {
            TAnnotNameTitleMap::iterator rna_i = rna_annots.find(unnamed_a);
            rna_annots.erase(rna_i);
            need_unnamed_track = true;
        }
        
        // see if we need to create RNA feature tracks
        TAnnotNameTitleMap::iterator gene_i = gene_annots.begin();
        while ( !rna_annots.empty()  &&  gene_i != gene_annots.end()) {
            TAnnotNameTitleMap::iterator rna_i = rna_annots.find(gene_i->first);
            if (rna_i != rna_annots.end()) {
                rna_annots.erase(rna_i);
            }
            ++gene_i;
        }
        
        
        // cds, there might be more annots for cds than gene
        TAnnotNameTitleMap cds_annots;
        sel.SetFeatSubtype(CSeqFeatData::eSubtype_cdregion);
        seq_ds->GetAnnotNames(sel, range, cds_annots);
        // collect NA tracks
        if ( !src_annots.empty() ) {
            ILayoutTrackFactory::GetMatchedAnnots(src_annots, params.m_Annots, "ftable",
                             feats.GetStoragekey(CSeqFeatData::eSubtype_cdregion), rna_annots);
        }
        
        if ( !has_unnamed_track  &&  cds_annots.count(unnamed_a) > 0 ) {
            TAnnotNameTitleMap::iterator rna_i = cds_annots.find(unnamed_a);
            cds_annots.erase(rna_i);
            need_unnamed_track = true;
        }
        
        // see if we need to create exon feature track
        gene_i = gene_annots.begin();
        while ( !cds_annots.empty()  &&  gene_i != gene_annots.end()) {
            TAnnotNameTitleMap::iterator cds_i = cds_annots.find(gene_i->first);
            if (cds_i != cds_annots.end()) {
                cds_annots.erase(cds_i);
            }
            ++gene_i;
        }
        
        
        // exon, there might be more annots for exon than gene
        sel.SetFeatSubtype(CSeqFeatData::eSubtype_exon);
        TAnnotNameTitleMap exon_annots;
        seq_ds->GetAnnotNames(sel, range, exon_annots);
        // collect NA tracks
        if ( !src_annots.empty() ) {
            ILayoutTrackFactory::GetMatchedAnnots(src_annots, params.m_Annots, "ftable",
                feats.GetStoragekey(CSeqFeatData::eSubtype_exon), exon_annots);
        }
        
        // see if we need to create exon feature track
        gene_i = gene_annots.begin();
        while ( !exon_annots.empty()  &&  gene_i != gene_annots.end()) {
            TAnnotNameTitleMap::iterator exon_i = exon_annots.find(gene_i->first);
            if (exon_i != exon_annots.end()) {
                exon_annots.erase(exon_i);
            }
            ++gene_i;
        }
      
        // check to see if any two of rna, cds and exon share the same
        // annotatation name. If yes, we need to create a Gene track
        // to group them together.
        set<string> unique_annots;
        TAnnotNameTitleMap rna_cds_exon_annots;
        ITERATE (TAnnotNameTitleMap, iter, rna_annots) {
            unique_annots.insert(iter->first);
        }
        ITERATE (TAnnotNameTitleMap, iter, cds_annots) {
            if ( !unique_annots.insert(iter->first).second ) {
                rna_cds_exon_annots.insert(*iter);
            }
        }
        ITERATE (TAnnotNameTitleMap, iter, exon_annots) {
            if ( !unique_annots.insert(iter->first).second ) {
                rna_cds_exon_annots.insert(*iter);
            }
        }

        // remove any annotaiton shared by any two of rna, cds,
        // and exons from the corresponding list
        ITERATE (TAnnotNameTitleMap, iter, rna_cds_exon_annots) {
            rna_annots.erase(iter->first);
            cds_annots.erase(iter->first);
            exon_annots.erase(iter->first);
            gene_annots.insert(*iter);
        }        
       
        // create rna feature tracks
        if ( !rna_annots.empty() ) {
            ITERATE (TAnnotNameTitleMap, iter, rna_annots) {
                SExtraParams feat_params(params);
                feat_params.m_Annots.clear();
                feat_params.m_Annots.push_back(iter->first);
                ITERATE (list<int>, st_iter, rna_subtypes) {
                    feat_params.m_Subkey = feats.GetStoragekey(*st_iter);
                    CFeatureTrackFactory feature_factory;
                    TTrackMap rna_tracks = feature_factory.CreateTracks(
                        object, ds_context, r_cntx, feat_params, src_annots);
                    if ( !rna_tracks.empty()) {
                        tracks[iter->first] = rna_tracks.begin()->second;
                    }
                }
            }
        }
        
        // create cdregion feature tracks
        ITERATE (TAnnotNameTitleMap, iter, cds_annots) {
            CFeatureTrackFactory feature_factory;
            SExtraParams feat_params(params);
            feat_params.m_Annots.clear();
            feat_params.m_Annots.push_back(iter->first);
            feat_params.m_Subkey = feats.GetStoragekey(CSeqFeatData::eSubtype_cdregion);
            TTrackMap cds_tracks = feature_factory.CreateTracks(
                object, ds_context, r_cntx, feat_params, src_annots);
            if (!cds_tracks.empty()) {
                tracks[iter->first] = cds_tracks.begin()->second;
            }
        }

        // create exon feature tracks
        ITERATE (TAnnotNameTitleMap, iter, exon_annots) {
            CFeatureTrackFactory feature_factory;
            SExtraParams feat_params(params);
            feat_params.m_Annots.clear();
            feat_params.m_Annots.push_back(iter->first);
            feat_params.m_Subkey = feats.GetStoragekey(CSeqFeatData::eSubtype_exon);
            TTrackMap exon_tracks = feature_factory.CreateTracks(
                object, ds_context, r_cntx, feat_params, src_annots);
            if (!exon_tracks.empty()) {
                tracks[iter->first] = exon_tracks.begin()->second;
            }
        }

        if (need_unnamed_track) {
            gene_annots.insert(TAnnotNameTitleMap::value_type(unnamed_a, ""));
        }
    }

    // create gene model tracks
    ITERATE (TAnnotNameTitleMap, iter, gene_annots) {
        CIRef<ISGDataSource> ds = ds_context->GetDS(
            typeid(CSGFeatureDSType).name(), object);
        seq_ds = dynamic_cast<CSGFeatureDS*>(ds.GetPointer());
        seq_ds->SetDepth(params.m_Level);
        seq_ds->SetAdaptive(params.m_Adaptive);
        //seq_ds->SetSortBy(params.m_SortBy);
        CRef<CGeneModelTrack> gene_model(new CGeneModelTrack(seq_ds, r_cntx));
        gene_model->SetAnnot(iter->first);
        if ( !iter->second.empty() ) {
            gene_model->SetTitle(iter->second);
        }
        tracks[iter->first] = gene_model.GetPointer();
    }

    return tracks;
}

void CGeneModelFactory::GetMatchedAnnots(
    const TAnnotMetaDataList& src_annots,
    const ILayoutTrackFactory::SExtraParams& params,
    TAnnotNameTitleMap& out_annots) const
{
    const CFeatList& feats(*CSeqFeatData::GetFeatList());
    ILayoutTrackFactory::GetMatchedAnnots(src_annots, params.m_Annots, "ftable",
                                          feats.GetStoragekey(CSeqFeatData::eSubtype_gene), out_annots);
    if (!params.m_Filter.empty()) return;

    // rna, there might be more annots for RNA than gene
    list<int> rna_subtypes;
    ITERATE(CFeatList, iter, feats) {
        if (iter->GetType() == CSeqFeatData::e_Rna  &&
            iter->GetSubtype() != CSeqFeatData::eSubtype_any) {
            rna_subtypes.push_back(iter->GetSubtype());
        }
    }

    ITERATE(list<int>, st_iter, rna_subtypes) {
        ILayoutTrackFactory::GetMatchedAnnots(src_annots, params.m_Annots, "ftable",
            feats.GetStoragekey(*st_iter), out_annots);
    }

    ILayoutTrackFactory::GetMatchedAnnots(src_annots, params.m_Annots, "ftable",
        feats.GetStoragekey(CSeqFeatData::eSubtype_cdregion), out_annots);

    ILayoutTrackFactory::GetMatchedAnnots(src_annots, params.m_Annots, "ftable",
        feats.GetStoragekey(CSeqFeatData::eSubtype_exon), out_annots);
}



string CGeneModelFactory::GetExtensionIdentifier() const
{
    return CGeneModelTrack::m_TypeInfo.GetId();
}


string CGeneModelFactory::GetExtensionLabel() const
{
    return CGeneModelTrack::m_TypeInfo.GetDescr();
}


CRef<CTrackConfigSet>
CGeneModelFactory::GetSettings(const string& profile,
                               const TKeyValuePairs& settings,
                               const CTempTrackProxy* /*track_proxy*/) const
{
    CRef<CTrackConfigSet> config_set(new CTrackConfigSet);
    // create a track configure
    CRef<CTrackConfig> config(new CTrackConfig);
    config_set->Set().push_back(config);
    config->SetHelp() = GetThisTypeInfo().GetDescr();
    config->SetLegend_text("anchor_1");

    CGeneModelConfig gm_settings;
    gm_settings.LoadSettingsPartial(kBaseKey, profile);
    gm_settings.UpdateSettings(settings);

    CGeneModelConfig::ERenderingOption option = CGeneModelConfig::eOption_MergeAll;
    if (gm_settings.m_ShowGenes) {
        if (gm_settings.m_ShowRNAs) {
            if (gm_settings.m_MergeStyle == CGeneModelConfig::eMerge_No) {
                option = CGeneModelConfig::eOption_ShowAll;
            }
        } else {
            option = CGeneModelConfig::eOption_GeneOnly;
        }
    } else {
        if (gm_settings.m_MergeStyle == CGeneModelConfig::eMerge_Pairs) {
            option = CGeneModelConfig::eOption_MergePairs;
        } else if (gm_settings.m_MergeStyle == CGeneModelConfig::eMerge_No) {
            option = CGeneModelConfig::eOption_ShowAllButGenes;
        } else if (gm_settings.m_MergeStyle == CGeneModelConfig::eMerge_OneLine) {
            option = CGeneModelConfig::eOption_SingleLine;
        }
    }

    CRef<CChoice> choice = CTrackConfigUtils::CreateChoice(
        "Options", "Rendering options", CGeneModelConfig::OptionValueToStr(option),
        "Rendering options on how gene features are rendered.");

    choice->SetValues().push_back(
        CTrackConfigUtils::CreateChoiceItem(
            CGeneModelConfig::OptionValueToStr(CGeneModelConfig::eOption_GeneOnly),
            "Gene bar only",
            "Only the 'green' gene bar is shown",
            "Each bar represents the extent of the gene as it was last annotated, without showing exons."));
    choice->SetValues().push_back(
        CTrackConfigUtils::CreateChoiceItem(
            CGeneModelConfig::OptionValueToStr(CGeneModelConfig::eOption_MergeAll),
            "Merge all transcripts and CDSs, no gene bar",
            "Merge all transcripts and CDSs into a single line, no gene bar shown.",
            "Vertical lines represent exon placements.  Arrows indicate orientation of the gene, 5' to 3'. Coding regions (CDS) are blue"));
    choice->SetValues().push_back(
        CTrackConfigUtils::CreateChoiceItem(
            CGeneModelConfig::OptionValueToStr(CGeneModelConfig::eOption_MergePairs),
            "Merge transcript and CDS pairs, no gene bar",
            "Merge the display of the transcript and its coding region as appropritate. Coding reginos are represented by different color. No gene bar is shown.",
            "Each transcript variant is rendered on a separate line.  Verticqal lines represent exon features.  Arrows indicate orientation of the gene, 5' to 3'."));
    choice->SetValues().push_back(
        CTrackConfigUtils::CreateChoiceItem(
            CGeneModelConfig::OptionValueToStr(CGeneModelConfig::eOption_ShowAllButGenes),
            "Show all transcripts and CDSs, no gene bar",
            "Show all transcripts and CDSs separately but with no gene bar.",
            "Blue lines represent transcripts while red lines represent CDSs. Vertical lines represent exon features. Arrows indicate orientation of the gene, 5' to 3'."));
    choice->SetValues().push_back(
        CTrackConfigUtils::CreateChoiceItem(
            CGeneModelConfig::OptionValueToStr(CGeneModelConfig::eOption_ShowAll),
            "Show all",
            "Show all transcripts and CDSs separately with gene bar.",
            "Blue lines represent transcripts while red lines represent CDSs. Vertical lines represent exon features. Arrows indicate orientation of the gene, 5' to 3'."));
    choice->SetValues().push_back(
        CTrackConfigUtils::CreateChoiceItem(
            CGeneModelConfig::OptionValueToStr(CGeneModelConfig::eOption_SingleLine),
            "Show on single line with exon structure, no gene bar",
            "No gene bar, but merge exon/CDS features and only show on a single line.",
            "The vertical lines represent exon features. Arrows indicate orientation of the gene, 5' to 3'."));
    config->SetChoice_list().push_back(choice);

    config->SetCheck_boxes().push_back(
        CTrackConfigUtils::CreateCheckBox(
            "CDSProductFeats", "Product Features",
            "Show product features projected from protein sequence", "",
            gm_settings.m_ShowCDSProductFeats));
    /*
    config->SetCheck_boxes().push_back(
        CTrackConfigUtils::CreateCheckBox(
            "NtRuler", "Show Product Ruler",
            "Show Product Ruler", "",
            gm_settings.m_ShowNtRuler));
    config->SetCheck_boxes().push_back(
        CTrackConfigUtils::CreateCheckBox(
            "AaRuler", "Show Protein Ruler",
            "Show Protein Ruler", "",
            gm_settings.m_ShowAaRuler));
    */
    config->SetHidden_settings().push_back(
        CTrackConfigUtils::CreateHiddenSetting(
        "NtRuler", NStr::BoolToString(gm_settings.m_ShowNtRuler)));
    config->SetHidden_settings().push_back(
        CTrackConfigUtils::CreateHiddenSetting(
        "AaRuler", NStr::BoolToString(gm_settings.m_ShowAaRuler)));
    config->SetHidden_settings().push_back(
        CTrackConfigUtils::CreateHiddenSetting(
        "HighlightMode", NStr::IntToString(gm_settings.m_HighlightMode)));
    config->SetHidden_settings().push_back(
        CTrackConfigUtils::CreateHiddenSetting(
        "ShowLabel", NStr::BoolToString(gm_settings.m_ShowLabel)));

    return config_set;
}


ILayoutTrackFactory::TTrackMap
CGeneModelFactory::x_CreateTracksWithFilters(SConstScopedObject& object,
        ISGDataSourceContext* ds_context, CRenderingContext* r_cntx,
        const SExtraParams& params, const TAnnotNameTitleMap& annots) const
{
    TTrackMap tracks;

    TSeqRange range = params.m_Range;
    if (range.Empty()) {
        range = r_cntx->GetVisSeqRange();
    }

    CIRef<ISGDataSource> pre_ds = ds_context->GetDS(
        typeid(CSGFeatureDSType).name(), object);
    CSGFeatureDS* seq_ds = dynamic_cast<CSGFeatureDS*>(pre_ds.GetPointer());
    seq_ds->SetDepth(params.m_Level);
    seq_ds->SetAdaptive(params.m_Adaptive);
    seq_ds->SetFilter(params.m_Filter);
    seq_ds->SetSortBy(params.m_SortBy);
    // create gene model tracks with specified filters
    ITERATE (TAnnotNameTitleMap, iter, annots) {
        vector<string> filters;
        if (params.m_SkipGenuineCheck) {
            filters.push_back(params.m_Filter);
        } else {
            filters = seq_ds->GetFilters(iter->first, range);
        }
        ITERATE (vector<string>, f_iter, filters) {
            CIRef<ISGDataSource> ds = ds_context->GetDS(
                typeid(CSGFeatureDSType).name(), object);
            CSGFeatureDS* feat_ds = dynamic_cast<CSGFeatureDS*>(ds.GetPointer());
            feat_ds->SetDepth(params.m_Level);
            feat_ds->SetAdaptive(params.m_Adaptive);
            CRef<CGeneModelTrack> gene_model(new CGeneModelTrack(feat_ds, r_cntx));
            gene_model->SetAnnot(iter->first);
            if ( !iter->second.empty() ) {
                gene_model->SetTitle(iter->second);
            }
            gene_model->SetFilter(*f_iter);
            tracks[MergeAnnotAndFilter(iter->first, *f_iter)] = gene_model.GetPointer();
        }
    }

    return tracks;

}


END_NCBI_SCOPE
