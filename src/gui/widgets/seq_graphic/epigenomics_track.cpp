/*  $Id: epigenomics_track.cpp 38532 2017-05-23 15:59:45Z katargir $
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

#include <gui/widgets/seq_graphic/epigenomics_track.hpp>
#include <gui/widgets/seq_graphic/seqgraphic_ds_manager.hpp>
#include <gui/widgets/seq_graphic/seqgraphic_job.hpp>
#include <gui/widgets/seq_graphic/seqgraphic_utils.hpp>
#include <gui/widgets/seq_graphic/rendering_ctx.hpp>
#include <gui/widgets/seq_graphic/histogram_glyph.hpp>
#include <gui/widgets/seq_graphic/config_utils.hpp>
#include <gui/widgets/wx/wx_utils.hpp>
#include <gui/utils/rgba_color.hpp>

#include <objmgr/table_field.hpp>
#include <objmgr/annot_ci.hpp>
#include <wx/menu.h>

BEGIN_NCBI_SCOPE
USING_SCOPE(objects);

static const string kTrackTitle = "Epigenomics map";

///////////////////////////////////////////////////////////////////////////////
/// CEpigenomicsJob.
///
class CEpigenomicsJob : public CSGAnnotJob
{
public:
    CEpigenomicsJob(const string& desc, objects::CBioseq_Handle handle,
        const objects::SAnnotSelector& sel, const TSeqRange& range,
        TModelUnit window, bool calc_min, bool fixed_scale)
        : CSGAnnotJob(desc, handle, sel, range)
        , m_Window(window)
        , m_CalcMin(calc_min)
        , m_FixedScale(fixed_scale)
    {}

protected:
    virtual EJobState x_Execute();

private:
    TModelUnit  m_Window;
    bool        m_CalcMin;      ///< add min graph
    bool        m_FixedScale;   ///< use global scale or dynamic scale
};


/// TEMPLATE STRUCT non_zero_min
template<class Type>
struct non_zero_min
    : public binary_functor<Type>
{   // functor for min
    Type operator()(const Type& left, const Type& right) const
    {   // apply operator+ to operands
        return left == 0 ? right : (left < right ? left : right);
    }

    virtual binary_functor<Type>* clone() const
    {
        return new non_zero_min<Type>;
    }
};


IAppJob::EJobState CEpigenomicsJob::x_Execute()
{
    try {
        CSGJobResult* result = new CSGJobResult();
        m_Result.Reset(result);

        CConstRef<CSeq_loc> loc =
            m_Handle.GetRangeSeq_loc(m_Range.GetFrom(), m_Range.GetTo());
        CAnnot_CI it(m_Handle.GetScope(), *loc, m_Sel);

        // we assume there will be one annotation. If not, we only pick up
        // the very first one.
        if (it) {
            typedef CHistogramGlyph::TDataType TDataType;
            CSeq_annot_Handle annot = *it;
            size_t rows = annot.GetSeq_tableNumRows();

            double min_val;
            double val_step;
            vector<char> values;
            CTableFieldHandle<double> col_val_min("value_min");
            CTableFieldHandle<double> col_val_step("value_step");
            CTableFieldHandle<int>   col_val("values");
            if ( !col_val_min.TryGet(annot, 0, min_val)  ||
                !col_val_step.TryGet(annot, 0, val_step) ||
                !col_val.TryGet(annot, 0, values)) {
                    return eFailed;
            }

            TDataType g_max = (TDataType)(255 * val_step + min_val);

            // create a look up table
            float de_quant[256];
            de_quant[0] = (float)min_val;
            for (int i = 1;  i < 256;  ++i) {
                de_quant[i] = de_quant[i - 1] + (float)val_step;
            }

            TSeqPos from = m_Range.GetFrom();
            TSeqPos to = m_Range.GetTo();
            int span;
            int pos;
            CTableFieldHandle<int> col_pos(CSeqTable_column_info::eField_id_location_from);
            CTableFieldHandle<int> col_span("span");

            // find the start row
            int row = 0;
            int r_start = 0;
            int r_end = rows - 1;
            do {
                row = (r_start + r_end) / 2;
                col_pos.TryGet(annot, row, pos);
                if ((TSeqPos)pos < from )   r_start = row;
                else                        r_end = row;
            } while ((TSeqPos)pos != from  &&  r_start < r_end - 1);

            CHistogramGlyph::TMaps maps;
            // Create the max density map
            CHistogramGlyph::TMap* max_data =
                &maps.insert(CHistogramGlyph::TMaps::value_type("Max",
                CHistogramGlyph::TMap(m_Range.GetFrom(), m_Range.GetTo(),
                (float)m_Window, new max_func<TDataType>()))).first->second;

            // Create the min density map
            CHistogramGlyph::TMap* min_data = NULL;
            if (m_CalcMin) {
                min_data = &maps.insert(CHistogramGlyph::TMaps::value_type("Min",
                CHistogramGlyph::TMap(m_Range.GetFrom(), m_Range.GetTo(),
                (float)m_Window, new non_zero_min<TDataType>()))).first->second;
            }

            for (row = (r_start + r_end) / 2;  (size_t)row < rows;  ++row) {
                if ( col_pos.TryGet(annot, row, pos) ) {
                    if ((TSeqPos)pos > to) break;
                    span = 1;
                    col_span.TryGet(annot, row, span);
                    TSeqPos end = pos + span - 1;
                    max_data->AddRange(TSeqRange(pos, end),
                        de_quant[(unsigned char)values[row]]);
                    if (m_CalcMin) {
                        min_data->AddRange(TSeqRange(pos, end),
                            de_quant[(unsigned char)values[row]]);
                    }
                }
            }

            string name = CSeqUtils::GetAnnotName(annot);
            CHistogramGlyph *hist = new CHistogramGlyph(maps, name);
            CRef<CSeqGlyph> fref(hist);
            if (m_FixedScale) {
                hist->SetAxisMaxFixed(g_max);
            }
            result->m_ObjectList.push_back(fref);
        }
    } catch (CException& ex) {
        m_Error.Reset(new CAppJobError(ex.GetMsg()));
        return eFailed;
    } catch (std::exception& ex) {
        m_Error.Reset(new CAppJobError(ex.what()));
        return eFailed;
    }

    return eCompleted;
}


///////////////////////////////////////////////////////////////////////////////
/// CEpigenomicsDS
///
CEpigenomicsDS::CEpigenomicsDS(CScope& scope, const CSeq_id& id)
    : CSGGenBankDS(scope, id)
{}


void CEpigenomicsDS::GetAnnotNames(const TSeqRange& range, TAnnotNameTitleMap& annots,
                                   const vector<string>& annots_allowed)
{
    SAnnotSelector sel = CSeqUtils::GetAnnotSelector(annots_allowed);
    CSeqUtils::SetResolveDepth(sel, GetAdaptive(), GetDepth());
    sel.SetAnnotType(CSeq_annot::C_Data::e_Seq_table);
    sel.SetCollectNames();

    CAnnotTypes_CI annot_it(CSeq_annot::C_Data::e_Seq_table, m_Handle,
        range, eNa_strand_unknown, &sel);
    ITERATE (CAnnotTypes_CI::TAnnotNames, iter, annot_it.GetAnnotNames()) {
        if (iter->IsNamed()) {
            if (iter->GetName().find("@@") == string::npos) {
                annots.insert(TAnnotNameTitleMap::value_type(iter->GetName(), ""));
            }
        } else {
            annots.insert(TAnnotNameTitleMap::value_type(
                CSeqUtils::GetUnnamedAnnot(), ""));
        }
    }
}


void CEpigenomicsDS::LoadData(const TSeqRange& range,
                              TModelUnit scale, bool calc_min,
                              bool fixed_scale)
{
    // create selector
    SAnnotSelector sel =
        CSeqUtils::GetAnnotSelector(CSeq_annot::C_Data::e_Seq_table);
    CSeqUtils::SetAnnot(sel, m_Annot);
    CSeqUtils::SetResolveDepth(sel, GetAdaptive(), GetDepth());
    CRef<CEpigenomicsJob> job( new CEpigenomicsJob("Load data",
        m_Handle, sel, range, scale, calc_min, fixed_scale) );
    x_LaunchJob(*job);
}


///////////////////////////////////////////////////////////////////////////////
/// CEpigenomicsDSType
///
ISGDataSource*
CEpigenomicsDSType::CreateDS(SConstScopedObject& object) const
{
    const CSeq_id& id = dynamic_cast<const CSeq_id&>(object.object.GetObject());
    return new CEpigenomicsDS(object.scope.GetObject(), id);
}


string CEpigenomicsDSType::GetExtensionIdentifier() const
{
    static string sid("epigenomics_ds_type");
    return sid;
}


string CEpigenomicsDSType::GetExtensionLabel() const
{
    static string slabel("Epigenomics Data Source Type");
    return slabel;
}


bool CEpigenomicsDSType::IsSharable() const
{
    return false;
}


///////////////////////////////////////////////////////////////////////////////
///   CEpigenomicsTrack
///

CTrackTypeInfo CEpigenomicsTrack::m_TypeInfo("epigenomic_track",
    "Graphical View Epigenomic Track");


CEpigenomicsTrack::CEpigenomicsTrack(CEpigenomicsDS* ds,
                                     CRenderingContext* r_cntx)
    : CDataTrack(r_cntx)
    , m_DS(ds)
    , m_CalcMin(false)
    , m_FixedScale(false)
{
    m_DS->SetJobListener(this);
    m_Simple->SetVertSpace(0);
    SetLayoutPolicy(m_Simple);

    x_RegisterIcon(SIconInfo(
        eIcon_Settings, "Settings", true, "track_settings"));
}


CEpigenomicsTrack::~CEpigenomicsTrack()
{
    if ( !m_DS->AllJobsFinished() ) {
        m_DS->DeleteAllJobs();
    }
}


const CTrackTypeInfo& CEpigenomicsTrack::GetTypeInfo() const
{
    return m_TypeInfo;
}


string CEpigenomicsTrack::GetFullTitle() const
{
    const string& title = GetTitle();
    if (title.empty()) {
        if (m_AnnotDesc.empty()) {
            return kTrackTitle;
        } else {
            return m_AnnotDesc;
        }
    }

    return title;
}


void CEpigenomicsTrack::x_LoadSettings(const string& /*preset_style*/,
                                       const TKeyValuePairs& settings)
{
    ITERATE (TKeyValuePairs, iter, settings) {
        try {
            if (NStr::EqualNocase(iter->first, "fixed_scale")) {
                m_FixedScale = NStr::StringToBool(iter->second);
            } else if (NStr::EqualNocase(iter->first, "min_graph")) {
                m_CalcMin = NStr::StringToBool(iter->second);
            }
        } catch (CException&) {
            LOG_POST(Error << "CGraphTrack::x_LoadSettings() invalid setting: "
                     << iter->second);
        }
    }
}


void CEpigenomicsTrack::x_SaveSettings(const string& /*preset_style*/)
{
    CSGConfigUtils::TKeyValuePairs settings;
    settings["min_graph"] = NStr::BoolToString(m_CalcMin);
    settings["fixed_scale"] = NStr::BoolToString(m_FixedScale);
    SetProfile(CSGConfigUtils::ComposeProfileString(settings));

}


void CEpigenomicsTrack::x_OnIconClicked(TIconID id)
{
    switch (id) {
    case eIcon_Settings:
        x_OnSettingsIconClicked();
        break;
    default:
        // use default handlers
        CLayoutTrack::x_OnIconClicked(id);
    }
}


void CEpigenomicsTrack::x_UpdateData()
{
    CDataTrack::x_UpdateData();
    TSeqRange range = m_Context->GetVisSeqRange();
    m_DS->DeleteAllJobs();
    m_DS->LoadData(range, m_Context->GetScale(), m_CalcMin, m_FixedScale);
}


void CEpigenomicsTrack::x_OnJobCompleted(CAppJobNotification& notify)
{
    m_DS->ClearJobID(notify.GetJobID());
    CRef<CObject> res_obj = notify.GetResult();
    CSGJobResult* result = dynamic_cast<CSGJobResult*>(&*res_obj);
    if (result) {
        x_AddGraphs(*result);
    } else {
        LOG_POST(Error << "CEpigenomicsTrack::x_OnJobCompleted() "
            "notification for job does not contain results.");
    }
}


void CEpigenomicsTrack::x_AddGraphs(const CSGJobResult& result)
{
    SetGroup().Clear();
    SetMsg("");
    SetObjects(result.m_ObjectList);

    CSeqGlyph::TObjects& objs = SetChildren();
    size_t size = objs.size();
    if ( size > 0 ) {
        CRef<CHistParamsManager> conf_mgr = m_gConfig->GetHistParamsManager();
        NON_CONST_ITERATE (CSeqGlyph::TObjects, iter, objs) {
            CHistogramGlyph* hist =
                dynamic_cast<CHistogramGlyph*>(iter->GetPointer());
            hist->SetDialogHost(dynamic_cast<IGlyphDialogHost*>(m_LTHost));
            const string& hist_name = hist->GetAnnotName();
            CRef<CHistParams> hist_conf = conf_mgr->GetHistParams(hist_name);
            if ( !conf_mgr->HasSettings(hist_name) ) {
                hist_conf.Reset(new CHistParams(*hist_conf));
                conf_mgr->AddSettings(hist_name, hist_conf);
            }
            hist_conf->m_Colors.clear();
            hist_conf->m_Colors["Max"] = CRgbaColor(0.2f, 0.8f, 0.2f, 0.8f);
            hist_conf->m_Colors["Min"] = CRgbaColor(0.0f, 0.0f, 1.0f, 1.0f);
            hist_conf->m_Colors["Avg"] = CRgbaColor(0.4f, 0.4f, 0.4f, 0.8f);
            hist->SetConfig(*x_GetGlobalConfig());
            if (size == 1) {
                m_AnnotDesc = hist->GetTitle();
            }
        }
    }
    x_UpdateLayout();
}


void CEpigenomicsTrack::x_OnSettingsIconClicked()
{
    wxMenu menu;
    UseDefaultMarginWidth(menu);
    int id_base = 10000;
    wxMenuItem* item_min = menu.AppendCheckItem(id_base, wxT("Shown min graph"));
    if (m_CalcMin) {
        item_min->Check();
    }
    wxMenuItem* item_scale = menu.AppendCheckItem(id_base + 1, wxT("Fixed scale"));
    if (m_FixedScale) {
        item_scale->Check();
    }
    m_LTHost->LTH_PopupMenu(&menu);

    if (m_CalcMin != item_min->IsChecked()) {
        m_CalcMin = !m_CalcMin;
        x_UpdateData();
    } else if (m_FixedScale != item_scale->IsChecked()) {
        m_FixedScale = !m_FixedScale;
        x_UpdateData();
    }

}


///////////////////////////////////////////////////////////////////////////////
/// CEpigenomicsTrackFactory
///////////////////////////////////////////////////////////////////////////////
ILayoutTrackFactory::TTrackMap
CEpigenomicsTrackFactory::CreateTracks(SConstScopedObject& object,
                                       ISGDataSourceContext* ds_context,
                                       CRenderingContext* r_cntx,
                                       const SExtraParams& params,
                                       const TAnnotMetaDataList& src_annots) const
{
    TAnnotNameTitleMap annots;

    if (params.m_SkipGenuineCheck  &&  !params.m_Annots.empty()) {
        ITERATE (SExtraParams::TAnnots, iter, params.m_Annots) {
            annots.insert(TAnnotNameTitleMap::value_type(*iter, ""));
        }
    } else {

        // collect non-NA tracks
        CIRef<ISGDataSource> ds =
            ds_context->GetDS(typeid(CEpigenomicsDSType).name(), object);
        CEpigenomicsDS* ds_pointer =
            dynamic_cast<CEpigenomicsDS*>(ds.GetPointer());
        ds_pointer->SetAdaptive(params.m_Adaptive);
        ds_pointer->SetDepth(params.m_Level);
        ds_pointer->GetAnnotNames(r_cntx->GetVisSeqRange(), annots, params.m_Annots);

        // collect NA tracks
        if ( !src_annots.empty() ) {
            GetMatchedAnnots(src_annots, params, annots);
        }
    }

    // create feature tracks
    TTrackMap tracks;
    ITERATE (TAnnotNameTitleMap, iter, annots) {
        CIRef<ISGDataSource> ds =
            ds_context->GetDS(typeid(CEpigenomicsDSType).name(), object);
        CEpigenomicsDS* ds_pointer =
            dynamic_cast<CEpigenomicsDS*>(ds.GetPointer());
        ds_pointer->SetDepth(params.m_Level);
        ds_pointer->SetAdaptive(params.m_Adaptive);
        ds_pointer->SetAnnot(iter->first);

        CRef<CEpigenomicsTrack> track(
            new CEpigenomicsTrack(ds_pointer, r_cntx));
        if ( !iter->second.empty() ) {
            track->SetTitle(iter->second);
        }

        tracks[iter->first] = track.GetPointer();
    }

    return tracks;
}

void CEpigenomicsTrackFactory::GetMatchedAnnots(
    const TAnnotMetaDataList& src_annots,
    const ILayoutTrackFactory::SExtraParams& params,
    TAnnotNameTitleMap& out_annots) const
{
    ILayoutTrackFactory::GetMatchedAnnots(src_annots, params.m_Annots, "seqtable", "epignomics", out_annots); //!! check that there is not typo in the word "epignomics"
}


string CEpigenomicsTrackFactory::GetExtensionIdentifier() const
{
    return CEpigenomicsTrack::m_TypeInfo.GetId();
}


string CEpigenomicsTrackFactory::GetExtensionLabel() const
{
    return CEpigenomicsTrack::m_TypeInfo.GetDescr();
}


END_NCBI_SCOPE
