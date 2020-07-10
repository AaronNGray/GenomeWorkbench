/*  $Id: histogram_glyph.cpp 44362 2019-12-05 16:29:20Z shkeda $
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
* Authors:  Vlad Lebedev, Liangshou Wu, Andrei Shkeda
*
* File Description:
*    CHistogramGlyph -- utility class for having
*                      feature density histograms his in graphical layouts.
*
*/

#include <ncbi_pch.hpp>
#include <gui/widgets/seq_graphic/histogram_glyph.hpp>
#include <gui/widgets/seq_graphic/rendering_ctx.hpp>
#include <gui/objutils/tooltip.hpp>
#include <gui/objutils/obj_fingerprint.hpp>
#include <gui/objutils/utils.hpp>
#include <gui/opengl/irender.hpp>
#include <gui/opengl/glutils.hpp>
#include <gui/opengl/gltexturefont.hpp>
#include "wx_histogram_config_dlg.hpp"
#include <gui/widgets/wx/message_box.hpp>
#include <gui/utils/splines/Splines.hh>

#include <gui/opengl/glpoint.hpp>
#include <gui/widgets/gl/attrib_menu.hpp>
#include <gui/widgets/seq_graphic/legend_item_glyph.hpp>
#include <gui/widgets/seq_graphic/layered_layout_policy.hpp>
#include <gui/widgets/seq_graphic/layout_group.hpp>
#include <gui/objects/LegendItem.hpp>
#include <gui/widgets/seq_graphic/layout_track_impl.hpp>

BEGIN_NCBI_SCOPE
USING_SCOPE(objects);

#ifdef ATTRIB_MENU_SUPPORT
    int CHistogramGlyph::m_SDMult = 5;
    float CHistogramGlyph::m_SD = 0.0f;
#endif

#define M_NCBI_E 2.71828182845904523536
static const float kDataPointRadius = 3.;

static const int kRulerLebelsMinDistance = 200;

static const int kLegendBarWidth = 30;
static const int kLegendLabelGap = 5;
static const int kLegenPadding = 5;

static const string kCoverageLabel = "Coverage";



CHistogramData::CHistogramData(TSeqPos start, TSeqPos stop, double window, accum_functor* funct, float def)
    : CDensityMap<float>(start, stop, window, funct, def)
    , m_InterpolationMethod(eInterpolation_None)
{
}


CHistogramData::CHistogramData(const CHistogramData& map)
    : CDensityMap<float>(map)
    , m_DataPoints(map.m_DataPoints)
    , m_InterpolationMethod(map.m_InterpolationMethod)
    , m_FirstPoint(map.m_FirstPoint)
    , m_LastPoint(map.m_LastPoint)
{
}

CHistogramData& CHistogramData::operator=(const CHistogramData& map)
{
    // check self assignment
    if (this == &map)
        return *this;
    CDensityMap<float>::operator=(map);
    m_DataPoints = map.m_DataPoints;
    m_InterpolationMethod = map.m_InterpolationMethod;
    m_FirstPoint = map.m_FirstPoint;
    m_LastPoint = map.m_LastPoint;
    return *this;
}

void CHistogramData::InitDataPoints(const TModelPoint& first_point, const TModelPoint& last_point)
{
    m_FirstPoint = first_point;
    m_LastPoint = last_point;
    double min_v = GetMin();
    double max_v = GetMax();
    min_v = min(min_v, min(m_FirstPoint.Y(), m_LastPoint.Y()));
    max_v = max(max_v, max(m_FirstPoint.Y(), m_LastPoint.Y()));
    double last_val = 0;
    int last_x = -1;

    for (int x = 0; x < GetBins(); ++x) {
        if ((*this)[x] != GetDefVal()) {
            if (last_x == -1) {
                last_x = x;
                last_val = (*this)[x];
            } else if (last_val != (*this)[x]) {
                last_x += (x - last_x) / 2;
                m_DataPoints.push_back(last_x);
                last_val = (*this)[x];
                last_val < min_v ? min_v = last_val : max_v = max(max_v, last_val);
                last_x = -1;
            }
        } else if (last_x != -1) {
            last_x += (x - last_x) / 2;
            m_DataPoints.push_back(last_x);
            last_val < min_v ? min_v = last_val : max_v = max(max_v, last_val);
            last_x = -1;
        }
    }
    if (last_x != -1) {
        last_x += (GetBins() - last_x) / 2;
        m_DataPoints.push_back(last_x);
        last_val < min_v ? min_v = last_val : max_v = max(max_v, last_val);
    }
    SetMin(min_v);
    SetMax(max_v);
    if (GetMax() == GetMin()) {
        (GetMax() > 0) ? SetMin(0) : SetMax(0);
    }
}

static bool IsOutlierTooltips(CRenderingContext& ctx)
{
    return ctx.GetScale() <= 1.0f/2.0f;
}


struct SLogScaleData
{
    double top_denominator;
    double bottom_denominator;
    double m_AxisMax;
    double m_AxisMin;
    int m_Height;
    CHistParams::EScale m_Scale;
    SLogScaleData(double AxisMax, double AxisMin, int height, CHistParams::EScale scale)
        : top_denominator(0), bottom_denominator(0)
        , m_AxisMax(AxisMax), m_AxisMin(AxisMin), m_Height(height), m_Scale(scale)
    {
        if (m_Scale == CHistParams::eLinear)
            return;
        x_InitDenominator(fabs(m_AxisMax), top_denominator);
        x_InitDenominator(fabs(m_AxisMin), bottom_denominator);
    }

    void x_InitDenominator(double AxisMax, double& denom)
    {
        double tick_n = 0;
        double log_remainder = 0;
        if (AxisMax <= 0)
            return;
        double log_max = 0;
        if (AxisMax > FLT_EPSILON) {
            if (m_Scale == CHistParams::eLog10)
                log_max = log10((double)AxisMax);
            else if (m_Scale == CHistParams::eLog2)
                log_max = log((double)AxisMax) / log(2.0);  //log2 not supported on windows
            else
                log_max = log((double)AxisMax);
        }
        log_remainder = log_max - floor(log_max);
        log_max = floor(log_max);
        tick_n = (int)log_max;
        if (tick_n == 0) {  // if AxisMax is <= 1
            tick_n = 1;
            log_remainder = 0.0;
        }
        denom = tick_n + log_remainder;
    }

    // If the desired display format is log, convert value to log

    double MapValue(double val)
    {
        if (m_Scale == CHistParams::eLinear) {
            return val;
        }

        int dir = val > 0 ? 1 : -1;
        TModelUnit y_ratio = (m_AxisMin < 0 && m_AxisMax > 0) ? 0.5f : 1.0;
        TModelUnit height = y_ratio * m_Height;
        if (dir < 0 && y_ratio != 1.)
            height = m_Height - height;
        val = fabs(val);
        // Interpolate position based on log scale markings.  Values below 1 are linear since
        // log(0) is undefined and log(0+)..log(0.99.) are negative.
        // We somewhat arbitrarily place the value 1 at 0.3*(the distance between ticks)
        //TModelUnit dist_y = GetHeight() / (TModelUnit(tick_n) + log_remainder);
        TModelUnit dist_denom = dir > 0 ? top_denominator : bottom_denominator;
        if (dist_denom == 0)
            dist_denom = 1.0;
        TModelUnit dist_y = height / dist_denom;
        if (fabs(val) <= TModelUnit(1.0)) {
            val = val * 0.3 * dist_y;
        } else {
            TModelUnit log_val;
            if (m_Scale == CHistParams::eLog10)
                log_val = log10(val);
            else if (m_Scale == CHistParams::eLog2)
                log_val = log(val) / log(2.0);  //log2 not supported on windows
            else // log e
                log_val = log(val);

            // interpolate between the lower and upper tick for this value
            TModelUnit lower_tick_log = floor(log_val);
            TModelUnit dist_between_ticks = log_val - lower_tick_log;

            // Special case for values between 1 and log base since we use 0 as a tick mark rather than 1 (since
            // this graph does not show values between 0 and 1).  Probably a proper graph would have evenly spaced
            // 0, 1, log_base^1, log_base^2 etc. But here we combine 0-log-base^1 into one tick mark since data
            // from 0..1 isn't very interesting since this is used with integers (no fractional numbers).
            if (lower_tick_log == TModelUnit(0))
                val = TModelUnit(0.3) * dist_y + dist_between_ticks * (dist_y - TModelUnit(0.3) * dist_y);
            else
                val = lower_tick_log * dist_y + dist_between_ticks * dist_y;
        }
        if (dir < 0)
            val = -val;
        return val;
    }
};




//
// CHistogramGlyph::CHistogramGlyph()
//
CHistogramGlyph::CHistogramGlyph(const TMap& map,
                                 CSeqFeatData::ESubtype subtype, const string& title)
    : m_DlgHost(NULL)
    , m_FixedScale(false)
    , m_AnnotName(CSeqUtils::GetUnnamedAnnot())
    , m_Subtype(subtype)
    , m_ShowTitle(false)
    , m_Mode(eMode_Single)
{
    if(CSeqUtils::isRmtAnnotName(title)) {
        m_RmtAnnotName = title;
    }
    string desc = title;
    if (desc.empty()) {
        const CFeatList& feat_list = *CSeqFeatData::GetFeatList();
        desc = feat_list.GetDescription(
            CSeqFeatData::GetTypeFromSubtype(subtype), subtype);
    }
    m_Maps.insert(TMaps::value_type(desc, map));
    SetAxisRange();
}


CHistogramGlyph::CHistogramGlyph(const TMap& map, const string& title)
    : m_DlgHost(NULL)
    , m_FixedScale(false)
    , m_AnnotName(CSeqUtils::GetUnnamedAnnot())
    , m_Subtype(CSeqFeatData::eSubtype_any)
    , m_ShowTitle(false)
    , m_Mode(eMode_Single)
{
    if(CSeqUtils::isRmtAnnotName(title)) {
        m_RmtAnnotName = title;
    }
    m_Maps.insert(TMaps::value_type(title, map));
    SetAxisRange();
}


CHistogramGlyph::CHistogramGlyph(const TMaps& maps,
                                 CSeqFeatData::ESubtype subtype)
    : m_DlgHost(NULL)
    , m_FixedScale(false)
    , m_AnnotName(CSeqUtils::GetUnnamedAnnot())
    , m_Subtype(subtype)
    , m_ShowTitle(false)
    , m_Mode(eMode_Single)
{
    m_Maps = maps;
    SetAxisRange();
}


CHistogramGlyph::CHistogramGlyph(const TMaps& maps, const string& name)
    : m_DlgHost(NULL)
    , m_FixedScale(false)
    , m_Subtype(CSeqFeatData::eSubtype_any)
    , m_ShowTitle(false)
    , m_Mode(eMode_Single)
{
    SetAnnotName(name);
    m_Maps = maps;
    SetAxisRange();
}

void CHistogramGlyph::SetAxisRange()
{
    CRef<CHistParams> params = GetHistParams();
    //if (params)
        //params->m_NumBins = 4;
    bool has_bins = params && (params->m_Type == CHistParams::eSmearBar || params->m_Type == CHistParams::eMergedBar) && params->m_NumBins > 0;
    if (has_bins)
        m_FixedScale = true;

    m_AxisMin = m_FixedScale ? m_AxisMin : GetMin();
    m_AxisMax = m_FixedScale ? m_AxisMax : GetMax();
    if (m_AxisMin == m_AxisMax)
        (m_AxisMax > 0) ? m_AxisMin = 0 : m_AxisMax = 0;

    if (params.IsNull())
        return;
    if (has_bins) {
        m_NumBins = params->m_NumBins;
        float max_val = (m_AxisMax - m_AxisMin) + 1;
        float denom = pow(10, floor(log10(max_val)));
        max_val = round(max_val/denom) * denom;
        denom /= 100;
        m_BinSize = floor(max_val / m_NumBins);
        m_BinSize = round((float(m_BinSize)/denom)) * denom;
    }


    // If we are clipping outliers, compute the standard deviation then set the
    // maximum possible value to:
    // mean + (max. standard deviation)*standard-deviation-multiplier OR the current
    // max value, whichever is lower.
    if (params->m_ClipOutliers) {
        TDataType mean;
        TDataType sdev_max = TDataType(0);

        ITERATE (TMaps, map_iter, m_Maps) {
            TDataType standard_deviation = x_ComputeDeviation(map_iter->second, mean);
            sdev_max = std::max(TDataType(mean + params->m_SDeviationThreshold*standard_deviation), sdev_max);
        }

        // If the standard deviation threshold is below the current maximum value,
        // set the new max value to the highest value below any value in the current
        // data set that is getting clipped (so outliers are clipped down to the
        // next lower non-outlier value)
        if (sdev_max < m_AxisMax) {
            m_AxisMax = sdev_max;
            TDataType lowest_max = TDataType(0);
            ITERATE (TMaps, map_iter, m_Maps) {
                TDataType low_max = x_HighestValueBelowMax(map_iter->second);
                lowest_max = std::max(low_max, lowest_max);
            }
            if (lowest_max > 0)
                m_AxisMax = std::min(m_AxisMax, lowest_max);
        }
        if (m_AxisMin < 0 && sdev_max < fabs(m_AxisMin)) {
            m_AxisMin = -sdev_max;
            TDataType lowest_min = TDataType(0);
            ITERATE (TMaps, map_iter, m_Maps) {
                TDataType low_min = x_LowesetValueBelowMin(map_iter->second);
                lowest_min = std::min(low_min, lowest_min);
            }
            m_AxisMin = std::max(m_AxisMin, lowest_min);
        }
    }
    if (params->m_Scale == CHistParams::eLinear && params->m_RangeAutoscale == false) {
        if (params->m_ValueRange.GetFrom() != params->m_ValueRange.GetEmptyFrom()) {
            m_AxisMin = params->m_ValueRange.GetFrom();
            params->m_ClipOutliers = true;
        }
        if (params->m_ValueRange.GetTo() != params->m_ValueRange.GetEmptyTo()) {
            m_AxisMax = params->m_ValueRange.GetTo();
            params->m_ClipOutliers = true;
        }
    }

    if (!m_FixedScale) {
        double sign_max = m_AxisMax >= 0 ? 1.0f : -1.0f;
        double sign_min = m_AxisMin >= 0 ? 1.0f : -1.0f;

        // Convert axis_max/min to linear based on underlying scale:
        if (params->m_StoredScale == CHistParams::eLog10) {
            m_AxisMax = pow(10.0, (double)fabs(m_AxisMax)) * sign_max;
            m_AxisMin = pow(10.0, (double)fabs(m_AxisMin)) * sign_min;
        }
        else if (params->m_StoredScale == CHistParams::eLoge) {
            m_AxisMax = pow(M_NCBI_E, (double)fabs(m_AxisMax))  * sign_max;
            m_AxisMin = pow(M_NCBI_E, (double)fabs(m_AxisMin)) * sign_min;
        }
        else if (params->m_StoredScale == CHistParams::eLog2) {
            m_AxisMax = pow(2.0, (double)fabs(m_AxisMax)) * sign_max;
            m_AxisMin = pow(2.0, (double)fabs(m_AxisMin)) * sign_min;
        }
    }
//    m_AxisMin = floor(m_AxisMin);
//    m_AxisMax = ceil(m_AxisMax);
    if (params->m_ClipOutliers == false && m_AxisMin > 0) {
        m_AxisMin = 0;
    }


#ifdef ATTRIB_MENU_SUPPORT
    m_SDMult = params->m_SDeviationThreshold;

    CAttribMenu& m = CAttribMenuInstance::GetInstance();
    if (!m.FindMenu("Graph Glyph")) {
        CAttribMenu* sub_menu = m.AddSubMenu("Graph Glyph");
        sub_menu->AddInt("SD Multiplier", &m_SDMult, 3.0f, 1, 40, 1);
        sub_menu->AddFloatReadOnly("Standard Deviation: ", &m_SD);
    }

#endif

}

CHistogramGlyph::~CHistogramGlyph()
{
#ifdef ATTRIB_MENU_SUPPORT
#endif
}

void CHistogramGlyph::SetConfig(const CSeqGraphicConfig& config)
{
    m_gConfig.Reset(&config);
    // Update range to handle clip outliers
    SetAxisRange();
}


bool CHistogramGlyph::OnLeftDblClick(const TModelPoint& /*p*/)
{
    if (m_Mode != eMode_Single)
        return true;
    RunSettingsDialog();
    return true;
}

void CHistogramGlyph::RunSettingsDialog()
{
    _ASSERT(m_gConfig);
    if (!m_gConfig)
        return;
    string name;
    if (GetFeatSubtype() != CSeqFeatData::eSubtype_any) {
        const CFeatList& feats(*CSeqFeatData::GetFeatList());
        name = feats.GetStoragekey(GetFeatSubtype());
    } else {
        name = GetAnnotName();
    }


    if (!m_gConfig->GetHistParamsManager()->IsTempSettings(name)) {
        CHistConfigDlg dlg;
        dlg.SetConfig(GetHistParams());
        dlg.SetConfigName(name);
        dlg.SetRegistryPath("Dialogs.GraphRenderingOptions");
        dlg.CreateX(NULL);
        if (m_DlgHost) {
            m_DlgHost->PreDialogShow();
        }
        if (!m_Desc.empty())
            dlg.SetHistName(m_Desc);
        if (dlg.ShowModal() == wxID_OK) {
            m_gConfig->GetHistParamsManager()->AddSettings(name, dlg.GetConfig());
            SetAxisRange();
            x_OnLayoutChanged();
        }
        if (m_DlgHost) {
            m_DlgHost->PostDialogShow();
        }
    }

}


bool CHistogramGlyph::NeedTooltip(const TModelPoint& p, ITooltipFormatter& tt, string& t_title) const
{
    if (m_Mode != eMode_Single)
        return false;
    GetTooltip(p, tt, t_title);
    return true;
}

void CHistogramGlyph::GetTooltip(const TModelPoint& p, ITooltipFormatter& tt, string& /*t_title*/) const
{
    if (m_Mode != eMode_Single)
        return;

    string value;

    if (GetFeatSubtype() != CSeqFeatData::eSubtype_any) {
        value = CSeqFeatData::GetFeatList()->GetDescription(GetFeatType(), GetFeatSubtype());
        value += " heatmap";
        value[0] = toupper(value[0]);
        if (!CSeqUtils::IsUnnamed(GetTitle())) {
            value += " (" + GetTitle() + ")";
        }
        tt.AddRow(value);
    } else { // it is graph

        if (!GetDesc().empty()) {
            value = GetDesc();
            /*NStr::ReplaceInPlace(tt, "&#xA;", "\n");
            NStr::ReplaceInPlace(tt, "&#xa;", "\n");*/
            tt.AddRow(value);
        } else {
            tt.AddRow("Graph:", GetTitle());
        }

        bool check_data_points = false;
        ITERATE(TMaps, map_iter, m_Maps)  {
            const TMap& densityMap = map_iter->second;
            if (!densityMap.GetDataPoints().empty()) {
                check_data_points = true;
                break;
            }
        }

        if (check_data_points) {

            CRef<CHistParams> params = GetHistParams();
            TModelUnit x = p.X();
            TModelUnit y = p.Y();
            CGlRect<TModelUnit> rect(x - kDataPointRadius, y + kDataPointRadius, x + kDataPointRadius, y - kDataPointRadius);
            SLogScaleData logscale(m_AxisMax, m_AxisMin, GetHeight(), params->m_Scale);

            ITERATE(TMaps, map_iter, m_Maps) {
                const TMap& densityMap = map_iter->second;
                for (size_t idx = 0; idx < densityMap.GetDataPoints().size(); ++idx) {
                    TModelPoint p = densityMap.GetDataPoint(idx);
                    double value = p.Y();
                    p.m_Y = x_MapValue(p.m_Y);
                    if (params->m_Scale != CHistParams::eLinear)
                        p.m_Y = logscale.MapValue(p.m_Y);
                    p = x_PointToWorld(p);
                    if (rect.PtInRect(p.X(), p.Y())) {
                        tt.AddRow("Value:", NStr::DoubleToString(value, 2));
                        return;
                    }
                }
            }
        }
    }

    // assume IsBackgroundJob returns false in Cgi mode 
    if (CSGDataSource::IsBackgroundJob()) {
        tt.AddRow();
        tt.AddRow("Double click on the histogram to change settings");
    }
}

TModelPoint CHistogramGlyph::x_PointToWorld(TModelPoint p) const
{

    TModelUnit scale_y = (m_AxisMin < 0 && m_AxisMax <= 0) ? m_AxisMin - m_AxisMax : m_AxisMax - m_AxisMin;
    if (scale_y == 0)
        return TModelPoint(p);

    TModelUnit x = p.X();
    TModelUnit y = p.Y();

    CRef<CHistParams> params = GetHistParams();
    if (params->m_Scale == CHistParams::eLinear) {

        TModelUnit y_ratio = min(1.0, fabs(m_AxisMax) / scale_y);
        if (y_ratio > 0 && y_ratio < 1.) {
            TModelUnit pos_height = y_ratio * GetHeight();
            TModelUnit neg_height = GetHeight() - pos_height;
            if (y >= neg_height) {
                scale_y = GetAxisMax() / pos_height;
                y /= scale_y;
                y += neg_height;
            } else {
                scale_y = GetAxisMin() / neg_height;
                y /= scale_y;
                y += pos_height;
            }
        } else {
            scale_y /= (TModelUnit)GetHeight();
            y /= scale_y;
        }
    } else {
        TModelUnit y_ratio = (m_AxisMin < 0 && m_AxisMax > 0) ? 0.5f : 1.0f;
        if (y_ratio != 1.) {
            TModelUnit h = y_ratio * GetHeight();
            h = GetHeight() - h;
            scale_y = (y >= h) ? 1 : -1;
            y /= scale_y;
            y += h;

        } else {
            scale_y = (m_AxisMin < 0) ? -1. : 1;
            y /= scale_y;
        }
    }
    if (scale_y > 0)
        y = GetHeight() - y;

    TModelUnit t_x = x;
    TModelUnit t_y = y;
    x_Local2World(t_x, t_y);
    return TModelPoint(t_x, t_y);
}


void CHistogramGlyph::x_GetLabel(string& label) const
{
    label.clear();
    if (IsCoverageGraph())
        label = kCoverageLabel;
    CRef<CHistParams> params = GetHistParams();
    if (params->m_Scale != CHistParams::eLinear) {
        if (!label.empty())
            label += ", ";
        label += CHistParams::ScaleValueToName(params->m_Scale);
        label += " scaled";
    }
}

void CHistogramGlyph::GetHTMLActiveAreas(TAreaVector* p_areas) const
{

    _ASSERT(p_areas);
    // Legen should go first
    if (m_Legend) {
        m_Legend->GetHTMLActiveAreas(p_areas);
    }

    bool check_data_points = false;
    ITERATE(TMaps, map_iter, m_Maps)  {
        const TMap& densityMap = map_iter->second;
        if (!densityMap.GetDataPoints().empty()) {
            break;
        }
    }
    bool area_added = false;
    if (check_data_points) {

        CRef<CHistParams> params = GetHistParams();
        SLogScaleData logscale(m_AxisMax, m_AxisMin, GetHeight(), params->m_Scale);

        ITERATE(TMaps, map_iter, m_Maps)  {
            const TMap& densityMap = map_iter->second;
            for (size_t idx = 0; idx < densityMap.GetDataPoints().size(); ++idx) {
                TModelPoint p = densityMap.GetDataPoint(idx);
                TModelUnit y = x_MapValue(p.Y());
                if (params->m_Scale != CHistParams::eLinear)
                    y = logscale.MapValue(y);

                TModelUnit x = p.X();
                TModelPoint pt = x_PointToWorld(TModelPoint(x, y));
                CHTMLActiveArea area;
                area.m_SeqRange = TSeqRange(p.X(), p.X());
                x = m_Context->SeqToScreenX(pt.X());
                //                area.m_ID = GetAnnotName() + ":" + NStr::NumericToString(p.X());
                area.m_Bounds.Init(x - kDataPointRadius, pt.Y() + kDataPointRadius,
                                   x + kDataPointRadius, pt.Y() - kDataPointRadius);
                area.m_Flags = CHTMLActiveArea::fNoPin | CHTMLActiveArea::fZoomOnDblClick ;
                area.m_Signature = "sig";
                area.m_Descr = NStr::DoubleToString(p.Y(), 2);
                p_areas->push_back(area);
                area_added = true;
            }
        }
    }
    if (IsOutlierTooltips(*m_Context)) {

        for (const auto& outlier : m_Outliers) {
            TModelPoint p(outlier.range.GetFrom(), outlier.y);
            TModelPoint pt = x_PointToWorld(p);
            CHTMLActiveArea area;
            area.m_SeqRange = outlier.range;
            auto x1 = m_Context->SeqToScreenX(outlier.range.GetFrom());
            auto x2 = m_Context->SeqToScreenX(outlier.range.GetTo());
            if (outlier.value > 0) 
                area.m_Bounds.Init(x1, pt.Y() + 5, x2, pt.Y() + 1);
            else                
                area.m_Bounds.Init(x1, pt.Y() + 2, x2, pt.Y() - 2);
            area.m_Flags = CHTMLActiveArea::fNoPin 
                | CHTMLActiveArea::fZoomOnDblClick
                | CHTMLActiveArea::fTooltipEmbedded;
            area.m_Signature = "otl";
            area.m_Descr = "Outlier value: " + NStr::DoubleToString(outlier.value, 2);
            p_areas->push_back(area);
        }
    }
    
    if (!area_added && m_Mode != eMode_Overlay) {
        CHTMLActiveArea area;
        CSeqGlyph::x_InitHTMLActiveArea(area);
        area.m_PositiveStrand = true;

        CSeq_loc loc;
        CSeq_interval& ival = loc.SetInt();
        CRef<CSGSequenceDS> seq_ds = m_Context->GetSeqDS();
        CConstRef<CSeq_id> seq_id = seq_ds->GetBioseqHandle().GetSeqId();
        ival.SetId().Assign(*seq_id);

        TMaps::const_iterator first_map = m_Maps.begin();
        int subtype = 0;
        CObjFingerprint::EObjectType type = CObjFingerprint::eUnknown;
        CRef<CHistParams> params = GetHistParams();
        // All HistogramGlyphs tooltips should be embedded
        // They still have to have correct signature as seqgraphic uses it to parse out title for remote tracks 
        // Histogram glyphs for remote tracks are expected to have empty m_Descr 
        // so that its tooltip can be created form GetTitle()

        area.m_Flags = CHTMLActiveArea::fZoomOnDblClick
            | CHTMLActiveArea::fTooltipEmbedded
            | CHTMLActiveArea::fNoSelection
            | CHTMLActiveArea::fNoHighlight
            | CHTMLActiveArea::fNoPin;

        if (GetFeatSubtype() != CSeqFeatData::eSubtype_any) {
            // This is track feature pile-up which can be rendered as histogram or heatmap
            area.m_Descr =
                CSeqFeatData::GetFeatList()->GetDescription(GetFeatType(), GetFeatSubtype());

            if (params->m_Type == CHistParams::eSmearBar || params->m_Type == CHistParams::eMergedBar) 
                area.m_Descr += " heatmap";
            else
                area.m_Descr += " distribution histogram";
            area.m_Descr[0] = toupper(area.m_Descr[0]);

            type = CObjFingerprint::eFeature;
            subtype = GetFeatSubtype();

            ival.SetFrom(first_map->second.GetStart());
            ival.SetTo(first_map->second.GetStop());
        } else {
            // This is track graph track or it can be single line graph create for feaures (dbavr track only???)
            type = CObjFingerprint::eGraph;
            ival.SetFrom(0);
            ival.SetTo(seq_ds->GetSequenceLength() - 1);
            if (IsCoverageGraph()) 
                x_GetLabel(area.m_Descr);
            else {
                area.m_Descr = GetDesc();
            }
        }  
        area.m_Signature =
            CObjFingerprint::GetHistogramSignature(loc, type, &m_Context->GetScope(),
                                                GetTitle(), 
                                                isRmtBased() ? m_RmtAnnotName : GetAnnotName(), 
                                                subtype);
        if (type == CObjFingerprint::eFeature) {
            area.m_Signature = "fake|" + area.m_Signature;
        }
        //}
        // a tooltip should be generated for histograms created by a remote file pipeline to avoid an additional roundtrip
        if(isRmtBased()) {
            string s;
            string title;
            CIRef<ITooltipFormatter> tooltip = ITooltipFormatter::CreateTooltipFormatter(ITooltipFormatter::eTooltipFormatter_CSSTable);
            GetTooltip(TModelPoint(-1, -1), *tooltip, title);
            s = tooltip->Render();
            string text = NStr::Replace(s, "\n", "<br/>");
            area.m_Descr = text;
        }
        p_areas->push_back(area);
        
        if (IsCoverageGraph()) {
            string label;
            x_GetLabel(label);
            
            CGlTextureFont font(CGlTextureFont::eFontFace_Courier, 11);
            IRender& gl = GetGl();

            TModelRange vis_r = m_Context->IntersectVisible(this);
            int view_width = m_Context->GetViewWidth();
            if (view_width == 0)
                view_width = m_Context->SeqToScreen(vis_r.GetLength());
            view_width -= 10;
            
            auto right = view_width;
            right -= (gl.TextWidth(&font, label.c_str()) + 4);
            
            TModelUnit left = 0;
            TModelUnit top = 0;
            x_Local2World(left, top);
            CHTMLActiveArea label_area;
            
            CSeqGlyph::x_InitHTMLActiveArea(label_area);
            label_area.m_Bounds.SetTop(top);
            label_area.m_Bounds.SetBottom(top + gl.TextHeight(&font) + 2);
            label_area.m_Bounds.SetLeft(-1);
            label_area.m_Bounds.SetRight(right);
            label_area.m_Flags = CHTMLActiveArea::fComment
                | CHTMLActiveArea::fNoSelection 
                | CHTMLActiveArea::fNoTooltip 
                | CHTMLActiveArea::fNoPin;
            label_area.m_ID = label;
            // required, but nonsense fields
            label_area.m_PositiveStrand = true;
            label_area.m_SeqRange.SetFrom(0);
            label_area.m_SeqRange.SetTo(0);
            
            p_areas->emplace_back(label_area);
        }


    }
}


bool CHistogramGlyph::IsClickable() const
{
    _ASSERT(m_gConfig);
    if (!m_gConfig)
        return false;

    string name;
    if (GetFeatSubtype() != CSeqFeatData::eSubtype_any) {
        const CFeatList& feats(*CSeqFeatData::GetFeatList());
        name = feats.GetStoragekey(GetFeatSubtype());
    } else {
        name = GetAnnotName();
    }
    
    if (m_gConfig->GetHistParamsManager()->IsTempSettings(name)) {
        return false;
    }

    return true;
}

bool CHistogramGlyph::IsCoverageGraph() const
{
    return CSeqUtils::IsCoverageAnnotName(m_AnnotName);
}


objects::CSeqFeatData::E_Choice CHistogramGlyph::GetFeatType() const
{
    return objects::CSeqFeatData::GetTypeFromSubtype(m_Subtype);
}


void CHistogramGlyph::SetAnnotName(const string& name)
{
    m_AnnotName = name;
    if (NStr::StartsWith(m_AnnotName, "NA")) {
        // In case it is a high level coverage graph, the
        // annotation name has level number tagged at the
        // end of the base annotation name, such as
        // NA000008860.1@100 for NA000008860.1
        // What we need here is the base annotation name
        int level;
        string base_name;
        if (ExtractZoomLevel(name, &base_name, &level) ) {
            m_AnnotName = base_name;
        }
    }
    if(CSeqUtils::isRmtAnnotName(name)) {
        m_RmtAnnotName = name;
    }
}

void CHistogramGlyph::x_DrawLinearRuler(bool negative) const
{
    IRender& gl = GetGl();
    CRef<CHistParams> params = GetHistParams();
    TModelUnit top = GetTop();

    // Compute number of vertical ticks based on font height
    CGlTextureFont font(CGlTextureFont::eFontFace_Helvetica, 8);
    TModelUnit font_height = font.GetMetric(IGlFont::eMetric_FullCharHeight);

    TDataType AxisMax = negative ? fabs(m_AxisMin) : fabs(m_AxisMax);
    TModelUnit y_ratio = fabs(m_AxisMax - m_AxisMin) != 0 ?
        fabs(m_AxisMax)/fabs(m_AxisMax - m_AxisMin) : 1.0f;
    if (y_ratio == 0.0f || y_ratio > 1.0f)
        y_ratio = 1.0f;

    TModelUnit height = ceil(y_ratio * GetHeight());
    if (negative && y_ratio != 1.0f) {
        top += height;
        height = GetHeight() - height;
    }
    TModelUnit bottom = top + height;

    bool forced_min_ticks = false;
    int tick_n = floor(double(TModelUnit(height)/(TModelUnit(3)*font_height)));
    if (tick_n < 2) {
        forced_min_ticks = true;
        tick_n = 2;
    }

    TModelUnit dist_y = height / tick_n;
    double axis_max = AxisMax;
    double axis_min = y_ratio == 1.0f ? (negative ? fabs(m_AxisMax) : fabs(m_AxisMin)) : 0;
    int precision = 0;

    // suppress precision (even it tick marks divide with a remainder) if distance between ticks
    // is large enough that it wouldn't be noticable.
    TModelUnit height_per_tick = (axis_max-axis_min)/TModelUnit(tick_n);
    if (height_per_tick < 10)
        precision = 1;


    const CRgbaColor& r_c = params->m_RulerColor;
    gl.Color4f(r_c.GetRed(), r_c.GetGreen(), r_c.GetBlue(), 0.4f);

    for (size_t i=0; i<=(size_t)tick_n; ++i) {
        m_Context->DrawLine(GetLeft(), top + dist_y*i, GetRight(), top + dist_y*i);
    }

    int ruler_n = 6;
    TModelUnit dist_x = GetWidth() / ruler_n;
    TModelUnit min_dist = m_Context->ScreenToSeq(kRulerLebelsMinDistance);
    if (dist_x < min_dist) {
        dist_x = min_dist;
        ruler_n = (int)(GetWidth() / dist_x);
    }
    TModelUnit label_off = m_Context->ScreenToSeq(1.0);

    TModelUnit x = GetLeft() + dist_x * 0.1;
    for (;  x < GetRight();  x += dist_x) {

        if (params->m_NeedRulerLabels || !m_gConfig->GetCgiMode()) {

            gl.Color4f(r_c.GetRed(), r_c.GetGreen(), r_c.GetBlue(), 0.5f);
            m_Context->DrawLine(x, top, x, bottom);
            
            gl.ColorC(params->m_LabelColor);
            if (negative == false) {
                if (axis_min != 0) {
                    m_Context->TextOut(&font, NStr::DoubleToString(axis_min).c_str(),
                                       x + label_off,
                                       bottom - 1, false);
                }
            } else {
                if (axis_max != 0) {
                    m_Context->TextOut(&font, NStr::DoubleToString(-axis_max).c_str(),
                                       x + label_off,
                                       bottom - 1, false);
                }
            }
            
            // Draw values for ticks between min and max, unless the space is too narrow to fit (forced_min_ticks)
            if (forced_min_ticks)
                tick_n = 1;
            for (size_t i=0; i<(size_t)tick_n; ++i) {
                TModelUnit height = negative ?
                    TModelUnit(axis_max-axis_min)*(TModelUnit(i)/TModelUnit(tick_n))
                    : TModelUnit(axis_max) - TModelUnit(axis_max-axis_min)*(TModelUnit(i)/TModelUnit(tick_n));
                if (height == 0)
                    continue;
                // skip zero if it's already printed on the positive side
                if (height == 0 && negative && y_ratio != 1.0)
                    continue;
                // only show decimals if delta between ticks is low and delta >= 0.1
                int current_precision = 0;
                if (precision > 0 && height-floor(height) >= 0.1f)
                    current_precision = precision;
                if (negative && height > 0)
                    height = -height;
                m_Context->TextOut(&font, NStr::DoubleToString(height, current_precision).c_str(),
                                   x + label_off, top + dist_y*i + gl.TextHeight(&font) + 1, false);
            }
        }
    }
}


void CHistogramGlyph::x_DrawLogRuler(bool negative) const
{
    IRender& gl = GetGl();
    CRef<CHistParams> params = GetHistParams();
    TModelUnit top = GetTop();

    // to display in any of the log scales (regardless of what
    // the underlying graph scale is)
    // Compute number of vertical ticks based on font height
    CGlTextureFont font(CGlTextureFont::eFontFace_Helvetica, 8);
    TModelUnit font_height = font.GetMetric(IGlFont::eMetric_FullCharHeight);
    TModelUnit vertical_text_space_min = TModelUnit(2)*font_height;

    TDataType AxisMax = negative ? fabs(m_AxisMin) : fabs(m_AxisMax);
    TModelUnit y_ratio = (m_AxisMin < 0 && m_AxisMax > 0) ? 0.5f : 1.0f;
    TModelUnit height = y_ratio * GetHeight();
    if (negative && y_ratio != 1.0f) {
        top += height;
        height = GetHeight() - height;
    }
    TModelUnit bottom = top + height;

    double log_base = 10.0;
    double log_max = 0.0;
    double log_min = 0.0;
    double log_remainder = 0.0;
    double axis_max = AxisMax;
    double axis_min = y_ratio == 1.0f ? (negative ? fabs(m_AxisMax) : fabs(m_AxisMin)) : 0;
    int precision = 0;

    if (axis_max > FLT_EPSILON) {
        if (params->m_Scale == CHistParams::eLog10) {
            log_base = 10.0;
            log_max = log10((double)axis_max);
            log_min = log10((double)axis_min);
        }
        else if (params->m_Scale == CHistParams::eLog2) {
            log_base = 2.0;
            log_max = log((double)axis_max )/log( 2.0 );  //log2 not supported on windows
            log_min = log((double)axis_min )/log( 2.0 );
        }
        else { // eLoge
            log_base = M_NCBI_E;
            log_max = log((double)axis_max );
            log_min = log((double)axis_min );
            precision = 2;
        }
    }

    //log_max = ceil(log_max);
    log_remainder = log_max-floor(log_max);
    log_max = floor(log_max);
    log_min = max(0.0, ceil(log_min));

    int tick_n = (int)log_max;
    if (tick_n == 0) {  // if value is <= 1
        tick_n = 1;
        log_remainder = 0.0;
    }

    // Ticks are in increments of the log scale (e.g. for 10: 10, 100, 1000)
    // but the top number is always axis_max which is unlikely to be a power
    // of the log value so top_y is the distance from axis_max down to the
    // first log value (log_max)
    TModelUnit top_y = height * (log_remainder/(log_max+log_remainder));
    TModelUnit dist_y = (height - top_y) / double(tick_n);

    const CRgbaColor& r_c = params->m_RulerColor;

    int ruler_n = 4;
    TModelUnit dist_x = GetWidth() / ruler_n;
    TModelUnit min_dist = m_Context->ScreenToSeq(kRulerLebelsMinDistance);
    if (dist_x < min_dist) {
        dist_x = min_dist;
        ruler_n = (int)(GetWidth() / dist_x);
    }
    TModelUnit label_off = m_Context->ScreenToSeq(1.0);

    TModelUnit x = GetLeft() + dist_x * 0.1;
    bool draw_horizontal_lines = true;
    // outside loop: iterate from left to right
    for (;  x < GetRight();  x += dist_x) {

        TModelUnit yval = TModelUnit(0);
        if (params->m_NeedRulerLabels) {

            gl.Color4f(r_c.GetRed(), r_c.GetGreen(), r_c.GetBlue(), 0.5f);
            m_Context->DrawLine(x, top, x, bottom);

            gl.ColorC(params->m_LabelColor);
            if (negative == false) {
                if (axis_max != 0) {
                    m_Context->TextOut(&font, NStr::DoubleToString(axis_max, precision).c_str(),
                                       x + label_off, top + gl.TextHeight(&font) + 1, false);
                }
                if (yval != 0) {
                    m_Context->TextOut(&font, NStr::DoubleToString(yval).c_str(),
                                       x + label_off,
                                       bottom - 1, false);
                }
            } else {
                if (axis_max != 0) {
                    m_Context->TextOut(&font, NStr::DoubleToString(-axis_max, precision).c_str(),
                                       x + label_off, bottom - 1, false);
                }
            }
        }
            
        if (draw_horizontal_lines) {
            // inner loop: iterate from top to bottom
            gl.Color4f(r_c.GetRed(), r_c.GetGreen(), r_c.GetBlue(), 0.4f);
            m_Context->DrawLine(GetLeft(), top, GetRight(), top);
            if (y_ratio != 1.0f)
                m_Context->DrawLine(GetLeft(), bottom, GetRight(), bottom);
        }

        gl.ColorC(params->m_LabelColor);
        TModelUnit vertical_text_space = dist_y;
        TModelUnit vertical_line_space = dist_y;
        bool draw_unlabled_lines = (tick_n*dist_y-vertical_text_space_min*0.5) < vertical_text_space_min;

        // iterate from bottom to the top
        yval = log_base;

        // first line
        gl.Color4f(r_c.GetRed(), r_c.GetGreen(), r_c.GetBlue(), 0.4f);
        m_Context->DrawLine(GetLeft(), bottom, GetRight(), bottom);

        for (int i=1; i<=tick_n; ++i) {
            // Don't write text on vertical ticks that are too close together. (or
            // too close to the bottom -0- bar).  Allow a little less space (50%)
            // between very top (axis_max) and top tick
            TModelUnit y = negative ?
                top + dist_y*(TModelUnit)(i) : top + top_y + dist_y*(TModelUnit)(tick_n-i);
            TModelUnit bottom_gap = negative ? bottom - y : y - top ;
            if (vertical_text_space >= vertical_text_space_min &&
                bottom_gap >= vertical_text_space_min*0.5) {

                // Draw horizontal lines on on first pass through outer loop since
                // these to all the way across.
                if (draw_horizontal_lines) {
                    gl.Color4f(r_c.GetRed(), r_c.GetGreen(), r_c.GetBlue(), 0.4f);
                    m_Context->DrawLine(GetLeft(), y, GetRight(), y);
                }
                if (yval != 0 && params->m_NeedRulerLabels) {

                    gl.ColorC(params->m_LabelColor);
                    if (negative) {
                        m_Context->TextOut(&font, NStr::DoubleToString(-yval, precision).c_str(),
                                           x + label_off, y - 1, false);
                    } else {
                        m_Context->TextOut(&font, NStr::DoubleToString(yval, precision).c_str(),
                                           x + label_off, y + gl.TextHeight(&font) + 1, false);
                    }
                }
                vertical_text_space = TModelUnit(0);
                vertical_line_space = TModelUnit(0);
            }
            // only draw some lines sometimes even if no room for text.  This is only turned on
            // if there is no room for lables on intermediate lines.
            else if (draw_horizontal_lines && draw_unlabled_lines &&
                        vertical_line_space > (vertical_text_space_min*0.5) &&
                        bottom_gap >= vertical_text_space_min*0.5) {
                // Draw horizontal lines on on first pass through outer loop since
                // these to all the way across.
                gl.Color4f(r_c.GetRed(), r_c.GetGreen(), r_c.GetBlue(), 0.4f);
                m_Context->DrawLine(GetLeft(), y, GetRight(), y);
                vertical_line_space = TModelUnit(0);
            }

            vertical_text_space += dist_y;
            vertical_line_space += dist_y;
            yval *= log_base;
        }
        draw_horizontal_lines = false;
    }
}


CHistogramGlyph::TDataType CHistogramGlyph::x_MapValue(TDataType val, CRgbaColor** color, bool* outlier) const
{
    CRef<CHistParams> params = GetHistParams();
    int dir = (val < 0) ? -1 : 1;
    if (color != 0)
        *color = val >= 0 && !(m_AxisMin < 0 && m_AxisMax <= 0) ? &m_fgColor : &m_negColor;
    if (outlier != 0)
        *outlier = false;

    TDataType AxisMax = dir < 0 ? m_AxisMin : m_AxisMax;
    if (params->m_Scale == CHistParams::eLinear && params->m_ClipOutliers) {
        if ((dir < 0 && val < AxisMax) || (dir > 0 && val > AxisMax)) {
            if (outlier != 0)
                *outlier = true;
            val = fabs(AxisMax);
        }
    }
    // Convert to linear initially (even if display target is log)
    if (params->m_StoredScale == CHistParams::eLog10)
        val = pow(10.0, fabs(val));
    else if (params->m_StoredScale == CHistParams::eLog2)
        val = pow(2.0, fabs(val));
    else if (params->m_StoredScale == CHistParams::eLoge)
        val = pow(M_NCBI_E, fabs(val));

    // Clip outliers (only supported for linear graphs)
    TModelUnit y_curr = fabs(val);
    if (params->m_Scale == CHistParams::eLinear) {
        if (m_AxisMin >= 0 && m_AxisMax > 0) {
            y_curr -= m_AxisMin;
            y_curr = max(0., y_curr);
        } else if (m_AxisMin < 0 && m_AxisMax <= 0) {
            y_curr += m_AxisMax;
        }
    }

    if (dir < 0)
        y_curr = -y_curr;
    y_curr = min<TModelUnit>(y_curr, fabs(AxisMax));
    return y_curr;
}

void CHistogramGlyph::x_DrawDataPoints(const TMap& densityMap) const
{
    IRender& gl = GetGl();
    CRef<CHistParams> params = GetHistParams();
    SLogScaleData logscale(m_AxisMax, m_AxisMin, GetHeight(), params->m_Scale);

    gl.PolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    gl.Enable(GL_BLEND);
    gl.BlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    gl.Enable(GL_LINE_SMOOTH);
    glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);
    gl.LineWidth(0.5);

    const CRenderingContext* ctx = GetRenderingContext();
    bool all_pos = !(m_AxisMin < 0 && m_AxisMax <= 0); // all data are positive

    double null_val = x_MapValue(0);
    if (params->m_Scale != CHistParams::eLinear)
        null_val = logscale.MapValue(null_val);

    vector<TModelPoint> points;
    points.reserve(densityMap.GetDataPoints().size() + 2);
    vector<const CRgbaColor*> colors;
    colors.reserve(densityMap.GetDataPoints().size() + 2);

    TModelPoint first_point = densityMap.GetFirstPoint();
    if (params->m_SmoothCurve) 
        points.push_back(TModelPoint(first_point.X(), x_MapValue(first_point.Y())));
    else 
        points.push_back(TModelPoint(first_point.X() - ctx->GetOffset(), x_MapValue(first_point.Y())));       
    colors.push_back(first_point.Y() >= 0 && all_pos ? &m_fgColor : &m_negColor);

    for (size_t idx = 0; idx < densityMap.GetDataPoints().size(); ++idx) {
        TModelPoint p = densityMap.GetDataPoint(idx);
        colors.push_back(p.Y() >= 0 && all_pos ? &m_fgColor : &m_negColor);
        if (params->m_SmoothCurve == false) {
            p.m_X -= ctx->GetOffset();
        }
        p.m_Y = x_MapValue(p.m_Y);
        points.push_back(p);
    }
    TModelPoint last_point = densityMap.GetLastPoint();
    if (params->m_SmoothCurve) 
        points.push_back(TModelPoint(last_point.X(), x_MapValue(last_point.Y())));
    else    
        points.push_back(TModelPoint(last_point.X() - ctx->GetOffset(), x_MapValue(last_point.Y())));
    colors.push_back(last_point.Y() >= 0 && all_pos ? &m_fgColor : &m_negColor);

    gl.Begin(params->m_Type == CHistParams::eLineGraph ? GL_LINE_STRIP : GL_TRIANGLE_STRIP);


    for (size_t i = 0; i < points.size(); ++i) {
        if (i == 0 || colors[i - 1] != colors[i])
            gl.ColorC(*colors[i]);
        if (params->m_Scale != CHistParams::eLinear)
            points[i].m_Y = logscale.MapValue(points[i].Y());
        if (params->m_SmoothCurve == false)  {
            if (densityMap.GetInterpolationMethod() == CHistogramData::eInterpolation_Linear) {

                if (i > 0) {
                    TModelUnit prev_y = points[i - 1].Y();
                    TModelUnit curr_y = points[i].Y();
                    if ((prev_y > 0 && curr_y < 0) || (prev_y < 0 && curr_y > 0)) {
                        double m = (curr_y - prev_y) / (points[i].X() - points[i - 1].X());
                        TModelUnit x = (-prev_y / m) + points[i - 1].X();
                        prev_y > 0 ? gl.ColorC(m_fgColor) : gl.ColorC(m_negColor);
                        if (params->m_Type == CHistParams::eHistogram)
                            gl.Vertex2d(x, null_val);
                        gl.Vertex2d(x, null_val);
                        gl.ColorC(*colors[i]);
                    }
                }

                if (params->m_Type == CHistParams::eHistogram)
                    gl.Vertex2d(points[i].X(), null_val);
                gl.Vertex2d(points[i].X(), points[i].Y());
            } else if (densityMap.GetInterpolationMethod() == CHistogramData::eInterpolation_Piecewise) {
                if (i > 0) {

                    TModelUnit prev_y = points[i - 1].Y();
                    TModelUnit curr_y = points[i].Y();

                    TModelUnit x = points[i - 1].X() + (points[i].X() - points[i - 1].X()) / 2;

                    gl.ColorC(*colors[i - 1]);
                    if (params->m_Type == CHistParams::eHistogram)
                        gl.Vertex2d(points[i - 1].X(), null_val);
                    gl.Vertex2d(points[i - 1].X(), prev_y);

                    if (params->m_Type == CHistParams::eHistogram)
                        gl.Vertex2d(x, null_val);
                    gl.Vertex2d(x, prev_y);

                    if ((prev_y > 0 && curr_y < 0) || (prev_y < 0 && curr_y > 0)) {

                        if (params->m_Type == CHistParams::eHistogram)
                            gl.Vertex2d(x, null_val);
                        gl.Vertex2d(x, prev_y);

                        if (params->m_Type == CHistParams::eHistogram)
                            gl.Vertex2d(x, null_val);
                        gl.Vertex2d(x, null_val);
                    }
                    gl.ColorC(*colors[i]);

                    if (params->m_Type == CHistParams::eHistogram)
                        gl.Vertex2d(x, null_val);
                    gl.Vertex2d(x, curr_y);

                    if (params->m_Type == CHistParams::eHistogram)
                        gl.Vertex2d(points[i].X(), null_val);
                    gl.Vertex2d(points[i].X(), curr_y);

                } else {
                    TModelUnit x = max(0., points[0].X());
                    if (params->m_Type == CHistParams::eHistogram)
                        gl.Vertex2d(x, null_val);
                    gl.Vertex2d(x, points[i].Y());
                }
            }
        }
    }
    gl.End();

    if (params->m_SmoothCurve)  
        x_DrawSmoothCurve(points);

    // draw values
    TModelUnit radius_x = ctx->ScreenToSeq(kDataPointRadius);
    TModelUnit radius_y = kDataPointRadius;
    //    TModelUnit radius_x_inner = ctx->ScreenToSeq(kDataPointRadius - 1.);
    //    TModelUnit radius_y_inner = kDataPointRadius - 1.;

    if (params->m_Scale == CHistParams::eLinear) {
        TModelUnit h = fabs(GetAxisMax() - GetAxisMin()) + 1;
        radius_y *= h / (TModelUnit)GetHeight();
        //  radius_y_inner *= h / (TModelUnit)GetHeight();
    }
    TModelUnit t = 2 * 3.1415926;
    CRgbaColor* color = 0;
    bool outlier = false;
    for (size_t idx = 0; idx < densityMap.GetDataPoints().size(); ++idx) {
        TModelPoint p = densityMap.GetDataPoint(idx);
        p.m_Y = x_MapValue(p.m_Y, &color, &outlier);
        if (outlier)
            color = &params->m_OutlierColor;
        if (params->m_Scale != CHistParams::eLinear)
            p.m_Y = logscale.MapValue(p.m_Y);
        p.m_X -= ctx->GetOffset();

        {
            gl.ColorC(*color);
            TModelUnit f = 0;
            gl.Begin(GL_TRIANGLE_FAN);
            gl.Vertex2d(p.X(), p.Y());
            TModelUnit step = (t - f) * 0.02;
            for (; f < t + step * 0.02; f += step) {
                gl.Vertex2d(p.X() - radius_x * cos(f), p.Y() - radius_y * sin(f));
            }
            gl.End();
        }

        //if (params->m_DrawBg)
        /*
        {

            gl.ColorC(params->m_bgColor);
            TModelUnit f = 0;
            gl.Begin(GL_TRIANGLE_FAN);
            gl.Vertex2d(p.X(), p.Y());
            TModelUnit step = (t - f) * 0.02;
            for (; f < t + step * 0.02; f += step) {
                gl.Vertex2d(p.X() - radius_x_inner * cos(f), p.Y() - radius_y_inner * sin(f));
            }
            gl.End();
        }
        */

        if (outlier) {
            gl.LineWidth(1.0);
            gl.Begin(GL_LINES);
            gl.Vertex2d(p.X(), p.Y());
            gl.Vertex2d(p.X(), null_val);
            gl.End();
        }
    }
    gl.Disable(GL_BLEND);
    gl.Disable(GL_LINE_SMOOTH);
    gl.LineWidth(1.0);
}

void CHistogramGlyph::x_DrawSmoothCurve(vector<TModelPoint>& points) const
{
    if (points.empty())
        return;

    vector<double> x_points;
    x_points.reserve(points.size());
    vector<double> y_points;
    y_points.reserve(points.size());
    CGlPane* orig_pane = m_Context->GetGlPane();
    auto from = orig_pane->ProjectX(points.front().X());
    auto to = orig_pane->ProjectX(points.back().X());
    if (from > to) {
        swap(from, to);
    }
    double y_min = numeric_limits<double>::max(), y_max = numeric_limits<double>::min();
    size_t idx = 0;
    for (const auto& p : points) {
        auto x = orig_pane->ProjectX(p.X());
        const auto& y = p.Y();
        y_max = max<double>(y, y_max);
        y_min = min<double>(y, y_min);
        bool new_value = (idx == 0) || (m_Context->IsFlippedStrand() && x_points[idx - 1] > x) || (!m_Context->IsFlippedStrand() && x_points[idx - 1] < x);        
        if (new_value) {
            x_points.push_back(x);
            y_points.push_back(y);
            ++idx;
        } else {
            if (fabs(y) > fabs(y_points[idx - 1]))
                y_points[idx - 1] = y;
        }
    }
    if (m_Context->IsFlippedStrand()) {
        reverse(x_points.begin(), x_points.end());
        reverse(y_points.begin(), y_points.end());
    }
    CRef<CHistParams> params = GetHistParams();
    Splines::PchipSpline spline;
    spline.build(x_points, y_points);
    
    IRender& gl = GetGl();
    gl.PushMatrix();
    gl.LoadIdentity();
    orig_pane->Close();
    CGlPane pane(*orig_pane);
    auto rcVP = pane.GetViewport();
    //pane.Close();
    TModelUnit t = 0;
    TModelUnit t_x = 0;
    x_Local2World(t_x, t);

    t = (rcVP.Top() - t);
    TVPRect vp_rect(rcVP.Left(), t - GetHeight(), rcVP.Right(), t + GetTop()); 
    pane.SetViewport(vp_rect);
    pane.OpenPixels();
    float height = vp_rect.Height();
    double scale = 1.0;
    if (params->m_Scale == CHistParams::eLinear) {
        double h = fabs(GetAxisMax() - GetAxisMin());
        scale = GetHeight();
        scale /= h;
    }
    auto bottom = vp_rect.Bottom() + 1;
    gl.Translatef(0.0f, bottom, 0.0f);
    gl.Scalef(1.0f, scale, 1.0f);

    if (params->m_Scale == CHistParams::eLinear) {
        if (m_AxisMax < 0)
            gl.Translatef(0.0, m_AxisMax, 0.0);
        else if (m_AxisMin < 0)
            gl.Translatef(0.0, -m_AxisMin, 0.0);
    } else {
        if (m_AxisMax <= 0)
            gl.Translatef(0.0, height, 0.0);
        else if (m_AxisMin < 0 && m_AxisMax > 0)
            gl.Translatef(0.0, height / 2., 0.0);
    }
    if (params->m_Type == CHistParams::eHistogram) {
        CRgbaColor top_colors[2] = { m_fgColor, m_negColor };
        CRgbaColor bottom_colors[2] = { m_fgColor, m_negColor };
        bottom_colors[0].Lighten(0.55f);
        bottom_colors[1].Lighten(0.55f);
        CRgbaColor middle_colors[2] = {
            CRgbaColor::Interpolate(top_colors[0], bottom_colors[0], 0.6f),
            CRgbaColor::Interpolate(top_colors[1], bottom_colors[1], 0.6f)
        };

        TModelUnit model_heights[2];
        if (params->m_Scale == CHistParams::eLinear) {
            if (m_AxisMin < 0) {
                model_heights[0] = GetAxisMax();
                model_heights[1] = fabs(GetAxisMin());
            }  else {
                model_heights[0] = model_heights[1] = fabs(GetAxisMax() - GetAxisMin());
            }
        } else {
            model_heights[0] = model_heights[1] = m_AxisMin < 0 ? height * 0.5f : height;
        }
        gl.ShadeModel(GL_SMOOTH);
        gl.Begin(GL_LINES);

        for (TSignedSeqPos pos = from; pos <= to; ++pos) {
            //auto y = alglib::spline1dcalc(s, pos);
            float y = spline(pos);
            //y = (y <= y_min) ? y_min : (y > y_max) ? y_max : y;
            if (y <= y_min) 
               y = y_min;
            else if (y >= y_max)
               y = y_max;

            size_t idx = y < 0 ? 1 : 0;
            int dir = y < 0 ? -1 : 1;
            CRgbaColor middle_c = middle_colors[idx];
            auto model_hh = model_heights[idx] * 0.3;
            if (fabs(y) > model_hh) {
                auto middle_y = model_hh * dir;

                gl.ColorC(middle_c);
                gl.Vertex2f(pos, middle_y);

                float alpha = (fabs(y) - model_hh) / (model_heights[idx] - model_hh);
                gl.ColorC(CRgbaColor::Interpolate(top_colors[idx], middle_c, alpha));
                gl.Vertex2f(pos, y);
                y = middle_y;
            } else {
                middle_c = CRgbaColor::Interpolate(middle_c, bottom_colors[idx], fabs(y) / model_hh);
            }
            gl.ColorC(bottom_colors[idx]);
            gl.Vertex2f(pos, 0);
            gl.ColorC(middle_c);
            gl.Vertex2f(pos, y);
        }
        gl.End();
        gl.ShadeModel(GL_FLAT);

    } else {
        gl.Enable(GL_LINE_SMOOTH);
        glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);
        if (m_gConfig->GetCgiMode()) {
            if (m_Context->GetScale() > 3)
                gl.LineWidth(0.5);
            else if (m_Context->GetScale() > 1. / 4.)
                gl.LineWidth(1.5);
            else
                gl.LineWidth(2);
        }
        gl.Begin(GL_LINE_STRIP);
        TModelUnit prev_y = 0;
        for (TSignedSeqPos pos = from; pos <= to; ++pos) {
            float y = spline(pos);
            if (y <= y_min)
                y = y_min;
            else if (y >= y_max)
                y = y_max;

            if (pos > 0) {
                if ((y < 0 && prev_y > 0) || (prev_y < 0 && y > 0)) {
                    gl.Vertex2d(pos, 0);
                    gl.ColorC(y < 0 ? m_negColor : m_fgColor);
                }
            } else {
                gl.ColorC(y < 0 ? m_negColor : m_fgColor);
            }
            gl.Vertex2d(pos, y);
            prev_y = y;
        }
        gl.End();
        gl.Disable(GL_LINE_SMOOTH);
    }
    gl.PopMatrix();
    orig_pane->OpenOrtho();
}


void CHistogramGlyph::x_DrawGraph(TMaps::const_iterator& map_iter) const
{
    const TMap& densityMap = map_iter->second;
    TMap::runlen_iterator seg_it = densityMap.RunLenBegin();
    if (!seg_it)
        return;

    if (!densityMap.GetDataPoints().empty()) {
        x_DrawDataPoints(densityMap);
        return;
    }

    IRender& gl = GetGl();
    gl.PolygonMode(GL_FRONT_AND_BACK, GL_FILL);

    CRef<CHistParams> params = GetHistParams();
    SLogScaleData logscale(m_AxisMax, m_AxisMin, GetHeight(), params->m_Scale);

    TSeqPos prev_x = 0;
    TModelUnit prev_y = 0;
    m_Outliers.clear();
    
    if (params->m_Type == CHistParams::eLineGraph) {

        gl.Enable(GL_LINE_SMOOTH);
        glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);
        if (m_gConfig->GetCgiMode()) {
            if (m_Context->GetScale() > 3)
                gl.LineWidth(0.5);
            else if (m_Context->GetScale() > 1. / 4.)
                gl.LineWidth(1.5);
            else
                gl.LineWidth(2);
        }
        gl.Begin(GL_LINE_STRIP);
    } else if (params->m_Type == CHistParams::eHistogram) {
        gl.ShadeModel(GL_SMOOTH);
        gl.Begin(GL_QUADS);
    }
   //    int pix_per_base = 1. / m_Context->GetScale();
   //    int fit_step = max<int>(params->m_FitStep, pix_per_base);
    int fit_step = params->m_FitStep;
    vector<TModelPoint> points;

    // pre-calculated variables for Histogram rendering
    CRgbaColor top_colors[2] = { m_fgColor, m_negColor };
    CRgbaColor bottom_colors[2] = { m_fgColor, m_negColor };
    CRgbaColor middle_colors[2];
    TModelUnit model_heights[2];
    TModelUnit model_heights3[2];

    if (params->m_Scale == CHistParams::eLinear) {
        if (m_AxisMin < 0 && m_AxisMax > 0) {
            model_heights[0] = GetAxisMax();
            model_heights[1] = fabs(GetAxisMin());
        } else {
            model_heights[0] = model_heights[1] = fabs(GetAxisMax() - GetAxisMin());
        }
    } else {
        model_heights[0] = model_heights[1] = m_AxisMin < 0 ? GetHeight() * 0.5f : GetHeight();
    }
    if (params->m_Type == CHistParams::eHistogram) {
        bottom_colors[0].Lighten(0.55f);
        bottom_colors[1].Lighten(0.55f);
        middle_colors[0] = CRgbaColor::Interpolate(top_colors[0], bottom_colors[0], 0.6f);
        middle_colors[1] = CRgbaColor::Interpolate(top_colors[1], bottom_colors[1], 0.6f);
        model_heights3[0] = model_heights[0] * 0.3f;
        model_heights3[1] = model_heights[1] * 0.3f;
    }

    for (; seg_it; seg_it.Advance()) {
        TSeqPos f = seg_it.GetSeqPosition();
        TSeqPos t = seg_it.GetSeqRunEndPosition();
        if (f == t)
            continue;
        if (prev_x >= t) {
            t = prev_x + 1;
        }

        TModelUnit val = seg_it.GetValue();
        // zero all the way
        if (val == TModelUnit(0)
            && seg_it.GetPosition() == 0
            && seg_it.GetRunLength() == densityMap.GetBins())
            continue;
        if (val == densityMap.GetDefVal())
            val = 0;
        //continue;

        int dir = (val < 0) ? -1 : 1;
        size_t idx = (val < 0) ? 1 : 0;
        CRgbaColor* curr_color = 0;
        bool outlier = false;
        TModelUnit y_curr = x_MapValue(val, &curr_color, &outlier);
        gl.ColorC(*curr_color);
        if (params->m_Scale != CHistParams::eLinear) {
            y_curr = logscale.MapValue(y_curr);
        }
        //m_Context->DrawQuad(f, y_curr, t, 0.0);
        // If x1 and x2 are projected onto the same pixel on screen.
        // We simply draw aline instead of a quad.
        TModelUnit x1 = f;
        TModelUnit x2 = t;
        if (fabs(x2 - x1) < m_Context->GetScale()) {
            TModelUnit mid = (x2 + x1) / TModelUnit(2.0);
            x1 = mid - m_Context->GetScale() / (TModelUnit)2.0;
            x2 = mid + m_Context->GetScale() / (TModelUnit)2.0;
        }
        if (outlier) {
            TModelUnit y = model_heights[idx];
            if (dir < 0)
                y = GetAxisMin() >= 0 ? 0 : -model_heights[idx];
            // TODO
            if (false && !m_Outliers.empty()
                && m_Outliers.back().range.GetTo() == x1
                && m_Outliers.back().y == y
                && (!IsOutlierTooltips(*m_Context) || m_Outliers.back().value == val)) {
                m_Outliers.back().range.SetTo(x2);
            }
            else {
                SOutlier s = { TSeqRange(x1, x2), y, val };
                m_Outliers.push_back(s);
            }
        }
        if (params->m_SmoothCurve) {
            points.emplace_back(x1, y_curr);
            if (x2 > x1) {
                auto pix_len = (x2 - x1);
                double step_len = fit_step * m_Context->GetScale();
                if (step_len == 0)
                    step_len = pix_len;
                int steps = pix_len / step_len;
                for (size_t i = 1; i < steps; ++i) {
                    points.emplace_back(x1 + i * step_len, y_curr);
                }
                points.emplace_back(x2, y_curr);
            }
        } else {
            x1 -= m_Context->GetOffset();
            x2 -= m_Context->GetOffset();

            if (params->m_Type == CHistParams::eHistogram) {
                CRgbaColor middle_c = middle_colors[idx];
                auto y = fabs(y_curr);
                if (y > model_heights3[idx]) {
                    auto middle_y = model_heights3[idx] * dir;
                    gl.ColorC(middle_c);
                    gl.Vertex2f(x1, middle_y);
                    gl.Vertex2f(x2, middle_y);
                    float alpha = (y - model_heights3[idx]) / (model_heights[idx] - model_heights3[idx]);
                    gl.ColorC(CRgbaColor::Interpolate(top_colors[idx], middle_c, alpha));
                    gl.Vertex2f(x2, y_curr);
                    gl.Vertex2f(x1, y_curr);
                    y_curr = middle_y;
                }
                else {
                    middle_c = CRgbaColor::Interpolate(middle_c, bottom_colors[idx], y / model_heights3[idx]);
                }
                gl.ColorC(bottom_colors[idx]);
                gl.Vertex2f(x1, 0);
                gl.Vertex2f(x2, 0);
                gl.ColorC(middle_c);
                gl.Vertex2f(x2, y_curr);
                gl.Vertex2f(x1, y_curr);
            } else {
                if ((prev_y > 0 && y_curr < 0) || (prev_y < 0 && y_curr > 0)) {
                    double m = (y_curr - prev_y) / (f - prev_x);
                    f = (-prev_y / m) + prev_x;
                    prev_y > 0 ? gl.ColorC(m_fgColor) : gl.ColorC(m_negColor);
                    gl.Vertex2d(f - m_Context->GetOffset(), 0);
                    gl.ColorC(*curr_color);
                }
                gl.Vertex2d(f - m_Context->GetOffset(), y_curr);
                if (t > f)
                    gl.Vertex2d(t - m_Context->GetOffset(), y_curr);
            }
        }

        prev_y = y_curr;
        prev_x = t;
    }
    if (params->m_Type == CHistParams::eLineGraph) {
        gl.End();
        gl.Disable(GL_LINE_SMOOTH);
    } else if (params->m_Type == CHistParams::eHistogram) {
        gl.End();
        gl.ShadeModel(GL_FLAT);
    }
    gl.LineWidth(1.0);

    if (params->m_SmoothCurve)
        x_DrawSmoothCurve(points);

    if (!m_Outliers.empty()) {
        float o_min = numeric_limits<float>::max();
        float o_max = numeric_limits<float>::min();
        for (const auto& o : m_Outliers) {
            if (o.value > o_max)
                o_max = o.value;
            if (o.value < o_min)
                o_min = o.value;
        }

        TModelUnit one_pix_x = m_Context->ScreenToSeq(1.);

        float h = fabs(GetAxisMax() - GetAxisMin());
        TModelUnit three_pix_y = (3. * h) / GetHeight();

        gl.ColorC(params->m_OutlierColor);
        gl.Begin(GL_QUADS);
        for (const auto& o : m_Outliers) {
            TModelUnit three_pix = three_pix_y;
            if (o.value < 0)
                three_pix = -three_pix;
            auto x1 = o.range.GetFrom() - m_Context->GetOffset();
            auto x2 = o.range.GetTo() - m_Context->GetOffset();
            if (m_Context->SeqToScreen(o.range.GetLength()) < 2) {
                x1 -= one_pix_x;
                x2 += one_pix_x;
            }
            float factor = o.value / (o.value >= 0 ? o_max : o_min);
            if (IsOutlierTooltips(*m_Context)) {
                CRgbaColor cl(params->m_OutlierColor);
                cl.Lighten(1. - factor);
                gl.ColorC(cl);
            }
            gl.Vertex2d(x1, o.y);
            gl.Vertex2d(x2, o.y);
            gl.Vertex2d(x2, o.y - three_pix);
            gl.Vertex2d(x1, o.y - three_pix);
        }
        gl.End();
    }
}


void CHistogramGlyph::DrawGrid(bool include_background) const
{
    IRender& gl = GetGl();
    CRef<CHistParams> params = GetHistParams();
    if (params->m_DrawBg) {
        gl.ColorC(params->m_bgColor);
        const TModelRect& rcm = GetModelRect();
        m_Context->DrawQuad(rcm);
    }
    if (params->m_NeedRuler  &&
        (params->m_Type == CHistParams::eHistogram  ||
        params->m_Type == CHistParams::eLineGraph) ) {

        CGlAttrGuard guard(GL_LINE_BIT);
        gl.Disable(GL_LINE_SMOOTH);
        gl.LineWidth(1.);
        if (params->m_Scale ==  CHistParams::eLinear) {
            if (m_AxisMin < 0)
                x_DrawLinearRuler(true);
            if (m_AxisMax > 0)
                x_DrawLinearRuler(false);
        } else {
            if (m_AxisMin < 0)
                x_DrawLogRuler(true);
            if (m_AxisMax > 0)
                x_DrawLogRuler(false);
        }
        gl.Enable(GL_LINE_SMOOTH);
    }
}

void CHistogramGlyph::x_DrawHeatMap(TMaps::const_iterator& map_iter, const CHistParams& config) const
{
    _ASSERT(config.m_Type == CHistParams::eSmearBar || config.m_Type == CHistParams::eMergedBar);

    IRender& gl = GetGl();
    // draw smear bar
    CRgbaColor color_max = config.m_SmearColorMax;
    CRgbaColor color_min = config.m_SmearColorMin;

    const TMap& densityMap = map_iter->second;
    float l_scale = (densityMap.GetMax() - densityMap.GetMin()) + 1;;
    if (l_scale > 0.0f) {
        // do pre-division
        l_scale = 1.0f / l_scale;
    }
    TModelUnit top = GetTop();
    TModelUnit bottom = top + GetHeight();
    if (m_Legend)
        bottom -= (m_Legend->GetHeight() + kLegenPadding);
    const CHistParams::TColorSet::const_iterator c_iter =
        config.m_Colors.find(map_iter->first);
    if (c_iter != config.m_Colors.end()) {
        color_max = c_iter->second;
        color_max.SetAlpha(0.9f);
        color_min = color_max;
        color_min.Lighten(0.5f);
        color_min.SetAlpha(0.9f);
    }
    TMap::runlen_iterator seg_it = densityMap.RunLenBegin();
    for (; seg_it; seg_it.Advance()) {
        TModelUnit  value = seg_it.GetValue();
        if (value == 0 || value == densityMap.GetDefVal())
            continue;

        TSeqPos f = seg_it.GetSeqPosition();
        TSeqPos t = seg_it.GetSeqRunEndPosition();

        //                float score = (value - GetAxisMin()) * l_scale;
        float score = 0;
        if (GetAxisMin() < 0) {
            score = fabs(value) * l_scale * 0.5;
            if (value > 0)
                score += 0.5;
            else
                score = 0.5 - score;
        } else {
            if (m_BinSize > 0) {
                score = int(value / m_BinSize);
                score /= m_NumBins - 1;

            } else {
                score = (value - GetAxisMin()) * l_scale;
            }

        }
        score = max(0.0f, score);
        score = min(1.0f, score);
        CRgbaColor color(CRgbaColor::Interpolate(color_max, color_min, score));
        gl.ColorC(color);
        // draw our segment as quad
        m_Context->DrawQuad(f, top, t, bottom);
    }

    if (m_Legend && m_gConfig && !m_gConfig->GetCgiMode())
        m_Legend->Draw();
}

void CHistogramGlyph::x_DrawGraphMap(TMaps::const_iterator& map_iter, const CHistParams& config) const
{
    IRender& gl = GetGl();

    if (m_Maps.size() > 1 && config.m_Colors.count(map_iter->first) > 0) {
        CHistParams::TColorSet::const_iterator c_iter = config.m_Colors.find(map_iter->first);
        m_fgColor = c_iter->second;
    } else {
        m_fgColor = config.m_fgColor;
    }
    if (m_Mode == eMode_Overlay && config.m_Type == CHistParams::eHistogram && m_fgColor.GetAlpha() == 1.f)
        m_fgColor.SetAlpha(0.75f);
    float scale = 1.0;
    if (config.m_Scale == CHistParams::eLinear) {
        float h = fabs(GetAxisMax() - GetAxisMin());
        if (h > 0.0)
            h = 1.0f / h;
        scale = h * GetHeight();
    }
    gl.PushAttrib(GL_ALL_ATTRIB_BITS);
    gl.PushMatrix();
    gl.LineWidth(1.0);

    TModelUnit bottom = GetTop() + GetHeight();

    if (m_AxisMin < 0 && m_AxisMax <= 0) {
        //gl.PushMatrix();
        //gl.Translatef(0.0f, top, 0.0f);
        //gl.ColorC(m_negColor);
        // m_Context->DrawLine(rcm.Left(), 0.0, rcm.Right(), 0.0);
        //gl.PopMatrix();
        gl.Translatef(0.0f, bottom, 0.0f);
    } else {
        gl.Translatef(0.0f, bottom, 0.0f);
        //gl.ColorC(m_fgColor);
        //m_Context->DrawLine(rcm.Left(), 0.0, rcm.Right(), 0.0);
    }

    gl.Scalef(1.0f, -scale, 1.0f);

    if (config.m_Scale == CHistParams::eLinear) {
        if (m_AxisMax < 0)
            gl.Translatef(0.0, m_AxisMax, 0.0);
        else if (m_AxisMin < 0)
            gl.Translatef(0.0, -m_AxisMin, 0.0);
    } else {
        if (m_AxisMax <= 0)
            gl.Translatef(0.0, GetHeight(), 0.0);
        else if (m_AxisMin < 0 && m_AxisMax > 0)
            gl.Translatef(0.0, GetHeight() / 2, 0.0);
    }

    x_DrawGraph(map_iter);
    gl.PopMatrix();
    gl.PopAttrib();
}



void CHistogramGlyph::x_Draw() const
{
    if (m_Maps.empty()) 
        return;
    _ASSERT(m_Context);
    IRender& gl = GetGl();
    TModelRect rcm = GetModelRect();
    CRef<CHistParams> params = GetHistParams();
    if (m_Mode == eMode_Single && params->m_DrawBg) {
        gl.ColorC(params->m_bgColor);
        m_Context->DrawQuad(rcm);
    }

#ifdef ATTRIB_MENU_SUPPORT
    // For debugging - allows the standard-deviation threshold for
    // clipping to be updated dynamically
    params->m_SDeviationThreshold = m_SDMult;
    const_cast<CHistogramGlyph*>(this)->SetAxisRange();
#endif

    TSeqRange range = GetRange();
    TModelUnit two_pix_size = m_Context->ScreenToSeq(2.0);
    if (range.GetLength() < two_pix_size) {
        TModelUnit top = GetTop();
        TModelUnit bottom = top + GetHeight();

        // bar is less then 2 pixel. Just draw a background
        gl.ColorC(params->m_fgColor);
        m_Context->DrawQuad(range.GetFrom(), top,
            range.GetFrom() + two_pix_size, bottom);
        return;
    }
    m_negColor = params->m_fgNegColor;

    CGlAttrGuard guard(GL_LINE_BIT | GL_LIGHTING_BIT);
    gl.Disable(GL_LINE_SMOOTH);

    {
        CGlAttrGuard guard(GL_ENABLE_BIT | GL_COLOR_BUFFER_BIT);
        gl.Enable(GL_BLEND);
        gl.BlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

        ITERATE(TMaps, map_iter, m_Maps) {
            if (params->m_Type == CHistParams::eSmearBar || params->m_Type == CHistParams::eMergedBar) {
                x_DrawHeatMap(map_iter, *params);
            } else { // draw line graph or histogram
                x_DrawGraphMap(map_iter, *params);
            }
        }
    }

    if (m_Mode == eMode_Single)
        DrawGrid(false);

    // Draw selection
    if (IsSelected()) {
        m_Context->DrawSelection(rcm);
    }

    if (!m_gConfig->GetCgiMode() && IsCoverageGraph()) {
        string label;
        x_GetLabel(label);
        _ASSERT(!label.empty());
        auto pane = m_Context->GetGlPane();
        CGlTextureFont font(CGlTextureFont::eFontFace_Helvetica, 11);
        auto w = gl.TextWidth(&font, label.c_str());
        auto h = gl.TextHeight(&font);
        const TModelRect& vr = pane->GetVisibleRect();
        auto one_px = m_Context->ScreenToSeq(1);
        auto x = vr.Right() - ((w + 16) * one_px);
        auto y = vr.Top() + h + 2;
        gl.Color3f(0.f, 0.f, 0.f);
        m_Context->TextOut(&font, label.c_str(), x, y, false);
    }

    gl.Enable(GL_LINE_SMOOTH);
}

void CHistogramGlyph::x_UpdateLegend()
{
    if (!x_HasLegend()) {
        if (m_Legend)
            m_Legend.Reset();
       return;
    }
    struct SLegendData
    {
        string annot;
        string label;
        CRgbaColor color;
        CHistParams* params;
    };
    CRef<CHistParams> params = GetHistParams();
    _ASSERT(params);

    CGlTextureFont font(CGlTextureFont::eFontFace_Helvetica, 8);
    IRender& gl = GetGl();

    TModelRange vis_r = m_Context->IntersectVisible(this);
    
    CRgbaColor color_max = params->m_SmearColorMax;
    CRgbaColor color_min = params->m_SmearColorMin;
    
    TModelUnit legend_item_span = 0;
    vector<SLegendData> legend_fields;
    for (int bin = 0; bin < m_NumBins; ++bin) {
        SLegendData data;
        data.annot = m_AnnotName;
        data.params = params.GetPointer();
        data.label = NStr::NumericToString(bin * m_BinSize + 1);
        data.label += "..";
        if (bin + 1 == m_NumBins)
            data.label += NStr::NumericToString(m_AxisMax);
        else
            data.label += NStr::NumericToString((bin + 1) * m_BinSize);
        data.color = CRgbaColor::Interpolate(color_max, color_min, float(bin) / (m_NumBins - 1));
        
        legend_item_span = max((TModelUnit)gl.TextWidth(&font, data.label.c_str()), legend_item_span);
        legend_fields.push_back(data);
    }
    if (legend_fields.empty())
        return;
    legend_item_span += kLegendBarWidth + kLegendLabelGap + kLegenPadding;

    int view_width = m_Context->GetViewWidth();
    if (view_width == 0)
        view_width = m_Context->SeqToScreen(vis_r.GetLength());
    view_width -= 10;
    
    int num_cols = min((int)(view_width / legend_item_span), (int)legend_fields.size());
    if (num_cols == 0)
        return;
    
    if (!m_Legend) {
        CRef<CLayeredLayout> layered_layout(new CLayeredLayout);
        m_Legend.Reset(new CLayoutGroup);
        m_Legend->SetLayoutPolicy(&*layered_layout);
        m_Legend->SetRenderingContext(&*m_Context);
        m_Legend->SetParent(this);
    } else {
        m_Legend->Clear();
    }

    TModelUnit center = m_Context->SeqToScreen(vis_r.GetFrom() + vis_r.GetLength() / 2);
    int num_rows = ceil((float)legend_fields.size() / num_cols);
    TModelUnit row_height = gl.TextHeight(&font) + 2;
    for (int row = 0; row < num_rows; ++row) {
        size_t base_index = row * num_cols;
        // last row may have less columns
        if (base_index + num_cols >= legend_fields.size())
            num_cols = legend_fields.size() - base_index;
        int c = num_cols / 2;
        TModelUnit left = center;
        if (num_cols % 2 != 0)
            left -= legend_item_span / 2;
        for (int col = 0; col < c; ++col) {
            left -= 4;
            left -= legend_item_span;
        }

        for (int col = 0; col < num_cols; ++col) {
            CRef<CLegendItemGlyph> g
                (new CLegendItemGlyph(legend_fields[base_index + col].annot,
                                      legend_fields[base_index + col].label,
                                      NcbiEmptyString,
                                      legend_fields[base_index + col].color,
                                      legend_fields[base_index + col].params->m_LabelColor));
            g->SetTop(row * row_height + row * 4 + 2);
            g->SetHeight(row_height);
            g->SetWidth(m_Context->ScreenToSeq(legend_item_span));
            g->SetLeft(m_Context->ScreenToSeq(left));
            g->SetConfig(*m_gConfig);
            //  g->SetHost(this);
            m_Legend->PushBack(&*g);
            
            left += 4;
            left += legend_item_span;
        }
    }

    m_Legend->SetTop(GetHeight() + kLegenPadding);
    m_Legend->SetLeft(GetLeft());
    m_Legend->SetWidth(GetWidth());
    m_Legend->SetHeight(row_height * num_rows + num_rows * 4);
    SetHeight(GetHeight() + m_Legend->GetHeight() + kLegenPadding);

}


void CHistogramGlyph::x_UpdateBoundingBox()
{
    CRef<CHistParams> params = GetHistParams();
    SetHeight(params->m_Height);
    TSeqRange range = GetRange();
    SetWidth(range.GetLength());
    SetLeft(range.GetFrom());
    x_UpdateLegend();
}

CHistogramGlyph::TDataType CHistogramGlyph::x_ComputeDeviation(const TMap& densityMap, TDataType& mean) const
{
    TMap::runlen_iterator seg_it = densityMap.RunLenBegin();

    mean = TDataType(0);
    TDataType variance = TDataType(0);
    TDataType samples = (TDataType)(densityMap.GetStop()-densityMap.GetStart()) + 1;

    // First find all the 0 entries since we do not include them in our calculations of mean and
    // standard deviation (Of course there are different datasources so in some cases 0 may
    // be a legitamite value, but I think that's not always the case)
    for (; seg_it; seg_it.Advance()) {
        TDataType  value = seg_it.GetValue();
        if (value == TDataType(0) || value == densityMap.GetDefVal()) {
            TSeqPos f   = seg_it.GetSeqPosition();
            TSeqPos t   = seg_it.GetSeqRunEndPosition();

            samples -= TDataType(t-f);
            continue;
        }
    }

    // Compute the mean, ignoring 0-values.
    TMap::runlen_iterator seg_it2 = densityMap.RunLenBegin();
    for (; seg_it2; seg_it2.Advance()) {
        TDataType  value = seg_it2.GetValue();
        if (value == TDataType(0) || value == densityMap.GetDefVal()) {
            continue;
        }

        TSeqPos f   = seg_it2.GetSeqPosition();
        TSeqPos t   = seg_it2.GetSeqRunEndPosition();
        mean += (TDataType(t-f)*value)/samples;
    }

    // Compute the variance (avg. distance squared from the mean)
    TMap::runlen_iterator seg_it3 = densityMap.RunLenBegin();
    for (; seg_it3; seg_it3.Advance()) {
        TDataType  value = seg_it3.GetValue();
        if (value == TDataType(0) || value == densityMap.GetDefVal())
            continue;
        TSeqPos f   = seg_it3.GetSeqPosition();
        TSeqPos t   = seg_it3.GetSeqRunEndPosition();
        TDataType delta = value-mean;
        variance += TModelUnit(t-f)*(delta*delta)/samples;
    }

    // Compute standard deviation as square root of the variance
    variance = sqrt(variance);

#ifdef ATTRIB_MENU_SUPPORT
    m_SD = variance;
#endif

    return variance;
}


CHistogramGlyph::TDataType CHistogramGlyph::x_HighestValueBelowMax(const TMap& densityMap) const
{
    TMap::runlen_iterator seg_it = densityMap.RunLenBegin();

    TDataType  max_value = TDataType(0);
    bool outlier=false;

    // Find the largest value below m_AxisMax IF there is at least one value above m_AxisMax.  This
    // lets us cut the visible range of the graph to the level of the highest value that is not an outlier.
    for (; seg_it; seg_it.Advance()) {
        TDataType  value = seg_it.GetValue();
        if (value == densityMap.GetDefVal())
            continue;
        if (value <= m_AxisMax)
            max_value = std::max(max_value, value);
        else
            outlier = true;
    }

    if (outlier)
        return max_value;

    return m_AxisMax;
}

CHistogramGlyph::TDataType CHistogramGlyph::x_LowesetValueBelowMin(const TMap& densityMap) const
{
    TMap::runlen_iterator seg_it = densityMap.RunLenBegin();

    TDataType  min_value = m_AxisMin; //TDataType(0);
    bool outlier=false;

    // Find the largest value below m_AxisMax IF there is at least one value above m_AxisMax.  This
    // lets us cut the visible range of the graph to the level of the highest value that is not an outlier.
    for (; seg_it; seg_it.Advance()) {
        TDataType  value = seg_it.GetValue();
        if (value == densityMap.GetDefVal())
            continue;
        if (value <= m_AxisMin)
            min_value = std::min(min_value, value);
        else
            outlier = true;
    }

    if (outlier)
        return min_value;

    return m_AxisMin;
}

void CHistogramGlyph::GetLegend(CTrackConfig::TLegend& legend) const
{
    if (!x_HasLegend())
        return;

    // draw smear bar
    CRef<CHistParams> params = GetHistParams();
    CRgbaColor color_max = params->m_SmearColorMax;
    CRgbaColor color_min = params->m_SmearColorMin;

    for (int bin = 0; bin < m_NumBins; ++bin) {
        CRef<CLegendItem> legend_item(new CLegendItem);
        legend_item->SetId("");

        string label = NStr::NumericToString(bin * m_BinSize + 1);
        label += "..";
        if (bin + 1 == m_NumBins)
            label += NStr::NumericToString(m_AxisMax);
        else
            label += NStr::NumericToString((bin + 1) * m_BinSize);
        legend_item->SetLabel(label);
        auto color = CRgbaColor::Interpolate(color_max, color_min, float(bin) / (m_NumBins - 1));
        legend_item->SetColor(color.ToString());
        legend.push_back(legend_item);
    }
}

bool CHistogramGlyph::x_HasLegend() const
{
    return m_NumBins > 1; 
}

END_NCBI_SCOPE
