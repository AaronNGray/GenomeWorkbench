#ifndef GUI_WIDGETS_SEQ_GRAPHIC___HISTOGRAM_GLYPH__HPP
#define GUI_WIDGETS_SEQ_GRAPHIC___HISTOGRAM_GLYPH__HPP

/*  $Id: histogram_glyph.hpp 44290 2019-11-22 20:05:53Z shkeda $
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
 * Authors:  Vlad Lebedev, Liangshou Wu
 *
 * File Description:
 *    CHistogramGlyph -- utility class for having
 *                      feature density histograms his in graphical layouts.
 *
 */


#include <corelib/ncbiobj.hpp>
#include <objects/seqloc/Seq_interval.hpp>
#include <gui/utils/rgba_color.hpp>
#include <gui/objutils/density_map.hpp>
#include <gui/objutils/utils.hpp>
#include <gui/objects/TrackConfig.hpp>
#include <gui/widgets/seq_graphic/seq_glyph.hpp>
#include <gui/widgets/seq_graphic/histogram_conf.hpp>
#include <gui/widgets/seq_graphic/seqgraphic_conf.hpp>
#include <gui/widgets/gl/attrib_menu.hpp>
#include <math.h>

BEGIN_NCBI_SCOPE

class CLayoutGroup;

class NCBI_GUIWIDGETS_SEQGRAPHIC_EXPORT CHistogramData : public CDensityMap < float >
{
public:
    enum EInterpolationMethod
    {
        eInterpolation_None,
        eInterpolation_Piecewise,
        eInterpolation_Linear
    };

    CHistogramData(TSeqPos start = 0, TSeqPos stop = 0, double window = 1,
                accum_functor* func = NULL, float def = 0);
    CHistogramData(const CHistogramData& map);
    CHistogramData& operator=(const CHistogramData& map);

    void SetInterpolationMethod(EInterpolationMethod interpolation_method) {
        m_InterpolationMethod = interpolation_method;
    }

    EInterpolationMethod GetInterpolationMethod() const {
        return m_InterpolationMethod;
    }

    const TDataPoints& GetDataPoints() const { 
        return m_DataPoints; 
    }
    TModelPoint GetFirstPoint() const  {
        return m_FirstPoint;
    }
    TModelPoint GetLastPoint() const {
        return m_LastPoint;
    }

    void InitDataPoints(const TModelPoint& first_point, const TModelPoint& last_point);

    TModelPoint GetDataPoint(size_t idx) const
    {
        TModelPoint p(GetWindow() * m_DataPoints[idx] + GetRange().GetFrom(), (*this)[m_DataPoints[idx]]);
        return p;
    }

protected:
    // m_Bins indexes of data points of interest
    TDataPoints m_DataPoints;
    EInterpolationMethod m_InterpolationMethod;
    TModelPoint m_FirstPoint;
    TModelPoint m_LastPoint;



};

class NCBI_GUIWIDGETS_SEQGRAPHIC_EXPORT CHistogramGlyph : public CSeqGlyph
{
public:
    typedef float TDataType;
    typedef CHistogramData TMap;
    typedef map<string, TMap> TMaps;

    enum ERenderingMode {
        eMode_Single, // typical mode: histogram has a single graph
        eMode_Overlay,
        eMode_Aggregate
    };
    struct max_func
        : public binary_functor<TDataType>
    {   // functor for max
        virtual TDataType operator()(const TDataType& left, const TDataType& right) const
        {
            return fabs(left) > fabs(right) ? left : right;
        }

        virtual binary_functor<TDataType>* clone() const
        {
            return new max_func;
        }
    };


    /// @name ctors
    /// @{
    CHistogramGlyph(const TMap& map, objects::CSeqFeatData::ESubtype subtype,
                    const string& title = "");
    CHistogramGlyph(const TMap& map, const string& title);
    CHistogramGlyph(const TMaps& maps, objects::CSeqFeatData::ESubtype subtype);
    CHistogramGlyph(const TMaps& maps, const string& name);

    ~CHistogramGlyph();
    /// @}

    /// @name CSeqGlyph virtual methods.
    /// @{
    virtual bool OnLeftDblClick(const TModelPoint& /*p*/);
    virtual bool NeedTooltip(const TModelPoint& p, ITooltipFormatter& tt, string& t_title) const;
    virtual void GetTooltip(const TModelPoint& /*p*/, ITooltipFormatter& tt, string& t_title) const;
    virtual void GetHTMLActiveAreas(TAreaVector* p_areas) const;
    virtual bool IsClickable() const;
    ///@}
    virtual bool IsCoverageGraph() const;

    /// Access the first density map.
    const TMap& GetDensityMap(void) const;

    /// Access all the maps.
    const TMaps& GetDensityMaps(void) const;

    /// Access the number of maps.
    const int   GetMapNum() const;

    // access the position of this object.
    virtual TSeqRange GetRange(void) const;

    objects::CSeqFeatData::E_Choice GetFeatType() const;
    objects::CSeqFeatData::ESubtype GetFeatSubtype() const;

    const string& GetTitle() const; 

    void SetAnnotName(const string& name);
    const string& GetAnnotName() const;

    void SetSubtype(objects::CSeqFeatData::ESubtype subtype);

    void SetDesc(const string& desc);
    const string& GetDesc() const;

    void SetConfig(const CSeqGraphicConfig& config);

    /// Get the actually density max/min values.
    TDataType GetMax() const;
    TDataType GetMin() const;
    /// Get the axis limits (max/min).
    TDataType GetAxisMax() const;
    TDataType GetAxisMin() const;

    // set vertical axis min-max values based on density map min-max values
    void SetAxisRange();

    void SetAxisMax(TDataType max);
    void SetAxisMin(TDataType min);

    void SetAxisMaxFixed(TDataType max);
    void SetAxisMinFixed(TDataType min);

    void SetFixedScale(bool value);

    void SetShowTitle(bool f);
    void SetDialogHost(IGlyphDialogHost* host);
    CRef<CHistParams> GetHistParams() const;

    void DrawGrid(bool include_background) const;

    void SetRenderingMode(ERenderingMode mode);

    void RunSettingsDialog();

    void GetLegend(objects::CTrackConfig::TLegend& legend) const;

    // true if the feature is based on a remote file derived  annotation
    bool isRmtBased() const;

protected:
    virtual void x_Draw() const;
    virtual void x_UpdateBoundingBox();
    
    TDataType x_ComputeDeviation(const TMap& densityMap, TDataType& mean) const;
    TDataType x_HighestValueBelowMax(const TMap& densityMap) const;
    TDataType x_LowesetValueBelowMin(const TMap& densityMap) const;

    void x_DrawLogRuler(bool negative) const;
    void x_DrawLinearRuler(bool negative) const;
    TDataType x_MapValue(TDataType val, CRgbaColor** color = 0, bool* outlier = 0) const;

    void x_DrawGraph(TMaps::const_iterator& map_iter) const;
    void x_DrawDataPoints(const TMap& densityMap) const;

    void x_DrawHeatMap(TMaps::const_iterator& map_iter, const CHistParams& config) const;
    void x_DrawGraphMap(TMaps::const_iterator& map_iter, const CHistParams& config) const;
    
    void x_DrawSmoothCurve(vector<TModelPoint>& points) const;
    
    void x_UpdateLegend();
    bool x_HasLegend() const;

    TModelPoint x_PointToWorld(TModelPoint p) const;

    void x_GetLabel(string& label) const;

protected:
    IGlyphDialogHost*               m_DlgHost;
    TMaps                           m_Maps;

    // Interactive menu debugging support
#ifdef ATTRIB_MENU_SUPPORT
    static int m_SDMult;
    static float m_SD;

    mutable TDataType               m_AxisMax;
    mutable TDataType               m_AxisMin; 
#else
    TDataType               m_AxisMax;
    TDataType               m_AxisMin; 
#endif

    bool m_FixedScale;
    /// Key for accessing histogram settings.
    /// It is used only when m_Subtype is eSubtype_any.
    string                          m_AnnotName;

    // if the glyph is based on a temporary annotation created by a remote pipeline, it is stored here
    string                          m_RmtAnnotName;
    string                          m_Desc;
    CConstRef<CSeqGraphicConfig>    m_gConfig;
    objects::CSeqFeatData::ESubtype m_Subtype;
    bool                            m_ShowTitle;
    ERenderingMode                  m_Mode;
    mutable CRgbaColor              m_fgColor;
    mutable CRgbaColor              m_negColor;

    CRef<CLayoutGroup> m_Legend;
    mutable int m_NumBins = 0;
    mutable int m_BinSize = 0;

    using SOutlier = struct {
        TSeqRange range;
        TModelUnit y;
        TModelUnit value;
    };

    mutable vector<SOutlier> m_Outliers;


};

///////////////////////////////////////////////////////////////////////////////
/// CHistogramGlyph inline method implementation.

inline
const CHistogramGlyph::TMap& CHistogramGlyph::GetDensityMap(void) const
{
    TMaps::const_iterator iter = m_Maps.begin();
    return iter->second;
}

inline
bool CHistogramGlyph::isRmtBased() const
{
    return CSeqUtils::isRmtAnnotName(m_RmtAnnotName);
}

inline
const CHistogramGlyph::TMaps& CHistogramGlyph::GetDensityMaps(void) const
{
    return m_Maps;
}

inline
const int CHistogramGlyph::GetMapNum() const
{
    return (int)m_Maps.size();
}

inline
TSeqRange CHistogramGlyph::GetRange(void) const
{
    TSeqRange range;
    ITERATE (TMaps, iter, m_Maps) {
        range.CombineWith(iter->second.GetRange());
    }
    return range;
}

inline
objects::CSeqFeatData::ESubtype CHistogramGlyph::GetFeatSubtype() const
{ return m_Subtype; }

inline
const string& CHistogramGlyph::GetTitle() const
{ return m_Maps.begin()->first; }

inline
const string& CHistogramGlyph::GetAnnotName() const
{ return m_AnnotName; }

inline
void CHistogramGlyph::SetSubtype(objects::CSeqFeatData::ESubtype subtype)
{ m_Subtype = subtype; }

inline
void CHistogramGlyph::SetDesc(const string& desc)
{ m_Desc = desc; }

inline
const string& CHistogramGlyph::GetDesc() const
{ return m_Desc; }

inline
CHistogramGlyph::TDataType CHistogramGlyph::GetMax() const
{
    double max_val = DBL_MIN;
    ITERATE (TMaps, map_iter, m_Maps) {
        max_val = max(max_val, (double)map_iter->second.GetMax());
    }
    return TDataType(max_val);
}

inline
CHistogramGlyph::TDataType CHistogramGlyph::GetMin() const
{
    double min_val = DBL_MAX;
    ITERATE (TMaps, map_iter, m_Maps) {
        min_val = min(min_val, (double)map_iter->second.GetMin());
    }
    return TDataType(min_val);
}

inline
CHistogramGlyph::TDataType CHistogramGlyph::GetAxisMax() const
{ return m_AxisMax; }

inline
CHistogramGlyph::TDataType CHistogramGlyph::GetAxisMin() const
{ return m_AxisMin; }

inline
void CHistogramGlyph::SetAxisMaxFixed(TDataType max_a)
{ 
    m_AxisMax = ceil(max(max_a, m_AxisMax)); 
}

inline
void CHistogramGlyph::SetAxisMinFixed(TDataType min_a)
{ 
    m_AxisMin = floor(min(min_a, m_AxisMin)); 
}

inline
void CHistogramGlyph::SetAxisMax(TDataType max_a)
{ 
    m_AxisMax = ceil(max_a); 
}

inline
void CHistogramGlyph::SetAxisMin(TDataType min_a)
{ 
    m_AxisMin = floor(min_a); 
}


inline
void CHistogramGlyph::SetShowTitle(bool f)
{ m_ShowTitle = f; }

inline
void CHistogramGlyph::SetDialogHost(IGlyphDialogHost* host)
{ m_DlgHost = host; }

inline
CRef<CHistParams> CHistogramGlyph::GetHistParams() const
{
    if (m_gConfig.IsNull()) {
        return CRef<CHistParams>();
    }

    if (m_Subtype != objects::CSeqFeatData::eSubtype_any && m_Subtype != objects::CSeqFeatData::eSubtype_bad) {
        return m_gConfig->GetHistParamsManager()->GetHistParams(m_Subtype);
    }
    return m_gConfig->GetHistParamsManager()->GetHistParams(GetAnnotName());
}

inline
void CHistogramGlyph::SetFixedScale(bool value)
{
    m_FixedScale = value;
}

inline
void CHistogramGlyph::SetRenderingMode(ERenderingMode mode)
{
    m_Mode = mode;
}

END_NCBI_SCOPE

/* @} */

#endif  // GUI_WIDGETS_SEQ_GRAPHIC___HISTOGRAM_GLYPH__HPP
