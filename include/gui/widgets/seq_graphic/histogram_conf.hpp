#ifndef GUI_OBJUTILS___HISTOGRAM_CONF_PARAMS__HPP
#define GUI_OBJUTILS___HISTOGRAM_CONF_PARAMS__HPP

/* $Id: histogram_conf.hpp 42475 2019-03-05 20:47:36Z shkeda $
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
 * Author:  Liangshou Wu
 *
 */

 /**
 * File Description:
  */


#include <gui/gui.hpp>
#include <gui/opengl/glbitmapfont.hpp>
#include <gui/utils/rgba_color.hpp>
#include <gui/objects/Choice.hpp>

BEGIN_NCBI_SCOPE

///////////////////////////////////////////////////////////////////////////////
/// CHistParams: the data structure holding feature rendering parameters.
///

class NCBI_GUIWIDGETS_SEQGRAPHIC_EXPORT CHistParams : public CObject
{
public:
    typedef map<string, CRgbaColor>  TColorSet;

    enum EType {
        eSmearBar,
        eHistogram,
        eMergedBar,
        eLineGraph,
        eDefaultType = eHistogram
    };

    enum EScale {
        eLinear,
        eLog10,
        eLog2,
        eLoge
    };


public:
    CHistParams()
        : m_Height(10)
        , m_Type(eHistogram)
        , m_Scale(eLinear)
        , m_StoredScale(eLinear)
        , m_NeedRuler(false)
        , m_DrawBg(false)
        , m_ClipOutliers(false)
        , m_SDeviationThreshold(5)
        , m_Dirty(false)
    {}

    static EType TypeStrToValue(const string& type);
    static const string& TypeValueToStr(CHistParams::EType type);

    static EScale ScaleStrToValue(const string& scale);
    static const string& ScaleValueToStr(CHistParams::EScale scale);
    static const string& ScaleValueToName(CHistParams::EScale scale);

    /// converts values returned in TMS "stored_scale" attribute (e.g. "log2 scaled") to
    /// values understood by ScaleStrToValue() and JS code (e.g. "log2")
    static string ScaleTMSToStr(const string& sScaleTMS);

    static CRef<objects::CChoice> CreateScaleOptions(const string& option_name, CHistParams::EScale option_value);


    void SetDirty(bool f) { m_Dirty = f; }
    bool GetDirty() const { return m_Dirty; }

public:
    /// @name Colors.
    /// @{
    CRgbaColor  m_fgColor;
    CRgbaColor  m_fgNegColor;
    CRgbaColor  m_bgColor;
    CRgbaColor  m_LabelColor;
    CRgbaColor  m_SmearColorMin;
    CRgbaColor  m_SmearColorMax;
    CRgbaColor  m_RulerColor;
    CRgbaColor  m_OutlierColor{"red"};
    TColorSet   m_Colors;
    /// @}

    /// @name Sizes.
    /// @{
    TModelUnit  m_Height = 10;
    /// @}

    EType       m_Type = eHistogram;
    /// requested scale
    EScale      m_Scale = eLinear;

    /// stored scale of the underlying data (e.g. as reported by TMS)
    EScale        m_StoredScale = eLinear;
    bool          m_NeedRuler = false;
    bool          m_NeedRulerLabels = true;  // not optional, set to false for alignment coverage graphs
    bool          m_DrawBg = false;
    bool          m_ClipOutliers = false;
    int           m_SDeviationThreshold = 5;
    int           m_NumBins = 0; // HeatMap bin number
    bool          m_RangeAutoscale = true;
    bool          m_SmoothCurve = false;
    int           m_FitStep = 4;
    CRange<float> m_ValueRange;

private:
    mutable bool  m_Dirty = false;
};


///////////////////////////////////////////////////////////////////////////////
/// CHistParamsManager: class for manage histogram/graph settings for
/// all feature subtype and various graphs.
///

class NCBI_GUIWIDGETS_SEQGRAPHIC_EXPORT CHistParamsManager : public CObject
{
public:
    typedef map<string, CRef<CHistParams> > THistParams;
    typedef int TFeatSubtype;

    void LoadSettings(const string& curr_color, const string& curr_size);

    void SaveSettings(const string& curr_color, const string& curr_size) const;

    CRef<CHistParams> GetDefHistParams() const;

    /// Get histogram settings using feature subtype.
    /// If there is no settings for the given feature subtype,
    /// the default one (Master) will be returned.
    CRef<CHistParams> GetHistParams(TFeatSubtype subtype) const;

    /// Get histogram settings using a key name.
    /// If there is no settings for the given name,
    /// the default one will be returned.
    CRef<CHistParams> GetHistParams(const string& name) const;

    /// Check if there is settings for a given name.
    bool IsTempSettings(const string& name) const;

    /// Check if there is settings for a given name.
    bool HasSettings(const string& name) const;

    /// Check if there is settings for a given feature subtype.
    bool HasSettings(TFeatSubtype subtype) const;


    /// Add a new settings for a given name.
    /// If there exists settings for the given name, then the existing one
    /// will get modified.
    void AddSettings(const string& name, CRef<CHistParams> hist_params);

    /// Add a temporary settings for a given name.
    void AddTempSettings(const string& name, CRef<CHistParams> hist_params);

private:
    void x_AddSettings(const string& name, CRef<CHistParams> hist_params,
        THistParams& settings);

private:
    /// cached histogram rendering parameters.
    THistParams m_HistSettings;

    /// temporary histogram settings.
    /// no need to save.
    THistParams m_TempHistSettings;

};


///////////////////////////////////////////////////////////////////////////////
/// CHistParamsManager inline methods
///

inline
void CHistParamsManager::AddSettings(const string& name,
                                     CRef<CHistParams> hist_params)
{
    hist_params->SetDirty(true);
    x_AddSettings(name, hist_params, m_HistSettings);
}

inline
void CHistParamsManager::AddTempSettings(const string& name,
                                         CRef<CHistParams> hist_params)
{
    x_AddSettings(name, hist_params, m_TempHistSettings);
}



END_NCBI_SCOPE

#endif // GUI_OBJUTILS___HISTOGRAM_CONF_PARAMS__HPP
