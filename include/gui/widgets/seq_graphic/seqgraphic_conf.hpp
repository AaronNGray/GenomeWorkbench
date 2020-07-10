#ifndef GUI_WIDGETS_SEQ_GRAPHIC___SEQGRAPHIC_CONF__HPP
#define GUI_WIDGETS_SEQ_GRAPHIC___SEQGRAPHIC_CONF__HPP

/*  $Id: seqgraphic_conf.hpp 44130 2019-11-02 00:14:51Z rudnev $
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
 *   Configuration file for Sequence Graphical Widget
 *
 */

#include <corelib/ncbiapp.hpp>
#include <corelib/ncbireg.hpp>
#include <gui/opengl/gltexturefont.hpp>
#include <gui/utils/rgba_color.hpp>
#include <gui/objutils/registry.hpp>
#include <gui/widgets/seq_graphic/histogram_conf.hpp>
#include <gui/widgets/seq_graphic/layout_conf.hpp>
#include <gui/widgets/seq_graphic/feature_conf.hpp>

#include <utility>  // for 'pair'

BEGIN_NCBI_SCOPE

class CCommentConfig;

inline string GetDefaultTmsContext() 
{
    static string tmsContext;
    if(tmsContext.empty()) {
        const CNcbiRegistry& reg = CNcbiApplication::Instance()->GetConfig();
        tmsContext = reg.GetString("TMS", "TMS_CONTEXT", "GBench_3-0");
    }
    return tmsContext;
}

class NCBI_GUIWIDGETS_SEQGRAPHIC_EXPORT CSeqGraphicConfig : public CObject
{
public:
    enum EAnnotDepth {
        eDepth_unknown = -2, ///< sequence-specific, up to app to decide
        eDepth_nolimit = -1, ///< adaptive and no depth limitation,
        eDepth_level0 = 0,   ///< exact level 0
        eDepth_level1 = 1,   ///< exact level 1
        eDepth_level2 = 2,   ///< exact level 2
        eDepth_level3 = 3,   ///< exact level 3
        eDepth_level4 = 4    ///< exact level 4
    };

    typedef map<string, CRef<CFeatureParams> > TFeatParams;

    /// we use int instead of CSeqFeatData::ESubtype for two reasons:
    ///  - avoids unnecessary includes
    ///  - gives transparent warning-free compiles with generic serialized
    ///    code (i.e., when we serialize a subtype, it's as an int not as an
    ///    enum)
    typedef int TFeatSubtype;

    CSeqGraphicConfig(bool cgi_mode = false, CGuiRegistry* registry = 0,
                      const string& theme = "", const string& color = "",
                      const string& size = "", const string& deco = "",
                      const string& profile = "", const string& layout = "",
                      const string& label = "");

    ~CSeqGraphicConfig();

    /// update the cached settings.
    void LoadSettings();
    void SaveSettings(bool clear_themes = false);

    static const string& ThemeKey();
    static const string& DefTheme();

    const   string&  GetCurrTheme() const;
    void    SetCurrTheme(const string& theme);

    const string& GetColorTheme() const;
    void    SetColorTheme(const string& theme);

    const string& GetSizeLevel() const;
    void    SetSizeLevel(const string& size);

    const string&  GetDecorateStyle() const;
    void    SetDecorateStyle(const string& style);

    const string& GetTrackProfile() const;
    void    SetTrackProfile(const string& profile);

    const string& GetLayout() const;
    void    SetLayout(const string& layout);

    const string& GetLabelPos() const;
    void    SetLabelPos(const string& label_pos);

    const string& GetGlobalSettingStyle() const;
    void    SetGlobalSettingStyle(const string& style);

    bool    IsAjdustRulerToSelection() const;
    void    SetAjdustRulerToSelection(bool flag);

    bool    GetShowComments() const;
    void    SetShowComments(bool flag);

    bool    GetShowSimpleComments() const;
    void    SetShowSimpleComments(bool flag);

    bool    GetShowMessage() const;
    void    SetShowMessage(bool flag);

    bool    ReserveCmmtSpace() const;
    void    SetReserveCmmtSpace(bool flag);

    bool    GetShowCommentBg() const;
    void    SetShowCommentBg(bool flag);

    bool    GetRepeatComment() const;
    void    SetRepeatComment(bool flag);

    bool    AutoHideTrack() const;
    void    SetAutoHideTrack(bool flag);

    bool    GetShowHorzGrid() const;
    void    SetShowHorzGrid(bool flag);

    bool    GetShowVertGrid() const;
    void    SetShowVertGrid(bool flag);

    bool    GetShowTrackMinusPlusSign() const;
    void    SetShowTrackMinusPlusSign(bool flag);

    bool    GetBackgroundLoading() const;
    void    SetBackgroundLoading(bool flag);

    bool    GetCgiMode() const;
    void    SetCgiMode(bool flag);

    bool    GetShowDownwardHairlines() const;
    void    SetShowDownwardHairlines(bool flag);

    CRef<CGlTextureFont> GetLabelFont() const;
    void    SetLabelFont(CGlTextureFont::EFontFace font, unsigned int font_size);

    CRef<CGlTextureFont> GetCommentFont() const;
    void    SetCommentFont(CGlTextureFont::EFontFace font, unsigned int font_size);

    CRef<CGlTextureFont> GetTitleFont() const;
    void    SetTitleFont(CGlTextureFont::EFontFace font, unsigned int font_size);

    const CRgbaColor& GetBkColor() const;
    void    SetBkColor(const CRgbaColor& color);

    const CRgbaColor& GetSelHairlineOneColor() const;
    void    SetSelHairlineOneColor(const CRgbaColor& color);

    const CRgbaColor& GetSelHairlineManyColor() const;
    void    SetSelHairlineManyColor(const CRgbaColor& color);

    const CRgbaColor& GetSelHairlineDownColor() const;
    void    SetSelHairlineDownColor(const CRgbaColor& color);

    const CRgbaColor& GetBGCommentColor() const;
    void    SetBGCommentColor(const CRgbaColor& color);

    const CRgbaColor& GetFGCommentColor() const;
    void    SetFGCommentColor(const CRgbaColor& color);

    const CRgbaColor& GetTitleBackground() const;
    void    SetTitleBackground(const CRgbaColor& color);

    const CRgbaColor& GetNonNCBITitleBackground() const;
    void    SetNonNCBITitleBackground(const CRgbaColor& color);

    const CRgbaColor& GetSelLabelColor() const;
    void    SetSelLabelColor(const CRgbaColor& color);

    const CRgbaColor& GetSelectionColor() const;
    void    SetSelectionColor(const CRgbaColor& color);

    const CRgbaColor& GetHorizontalGridColor() const;
    void    SetHorizontalGridColor(const CRgbaColor& color);

    const CRgbaColor& GetVerticalGridColor() const;
    void    SetVerticalGridColor(const CRgbaColor& color);

    int     GetHairlineOption() const;
    void    SetHairlineOption(int option);

    int     GetCommentRepeatDist() const;
    void    SetCommentRepeatDist(int d);

    int     GetOverviewCutoff() const;
    void    SetOverviewCutoff(int cutoff);

    int     GetHistogramCutoff() const;
    void    SetHistogramCutoff(int cutoff);

    float   GetLineWidth() const;
    void    SetLineWidth(float w);

    float   GetZoomFactor() const;
    void    SetZoomFactor(float);

    float   GetBarHeight() const;
    void    SetBarHeight(float h);

    float   GetCommentTopMargin() const;
    void    SetCommentTopMargin(float s);

    float   GetCommentBottomMargin() const;
    void    SetCommentBottomMargin(float s);

    int     GetObjectSpace() const;
    void    SetObjectSpace(int s);

    int     GetIconSize() const;
    void    SetIconSize(int s);

    int     GetIconSignSize() const;
    void    SetIconSignSize(int s);

    EAnnotDepth GetAnnotDepth() const;
    void    SetAnnotDepth(int depth);

    //bool    GetShowFeat(const string& feat) const;
    //void    SetShowFeat(const string& feat, bool show);

    //bool    GetShowFeat(TFeatSubtype subtype) const;
    //void    SetShowFeat(TFeatSubtype subtype, bool show);

    //bool    GetShowTrack(const string& track) const;
    //void    SetShowTrack(const string& track, bool show);

    /// Get feature settings using a feature subtype.
    CConstRef<CFeatureParams> GetFeatParams(TFeatSubtype subtype) const;

    /// Get feature settings using a key name.
    CConstRef<CFeatureParams> GetFeatParams(const string& name) const;

    TFeatParams&    SetFeatSettings();
    
    /// Set show feature strand indicator to false for all features.
    void IgnoreFeatureStrand();

    CConstRef<CClonePlacementParams> GetClonePlacementParams() const;

    /// Get histogram config manager
    CRef<CHistParamsManager> GetHistParamsManager() const;
    CRef<CHistParams> GetHistParams(TFeatSubtype subtype) const;
    CRef<CHistParams> GetHistParams(const string& name) const;

    CConstRef<CCommentConfig> GetCommentConfig() const;

    CConstRef<CCustomFeatureColor> GetCustomFeatColors() const;

    /// if the category configuration is not loaded, load it
    void InitCategoryConfig(const CCategoryConfig::TGroupMap& group_map, 
                                                const CCategoryConfig::TSubgroupMap& sub_group_map);
    // this can be called w/o Init from above, but then an ability to mix in TMS groups is lost forever
    CRef<CCategoryConfig> GetCategoryConfig();

    void SetDirty(bool f);
    bool GetDirty() const;    

    /// Utility method helping initializing various settings styles.
    /// The styles include color, size, feature decoration, track 
    /// profile, and label position.
    /// @reg GUI registry
    /// @theme setting theme. It may be empty. If empty, the default theme will be used
    /// @color color style. If empty, a corresponding style will be set
    /// @size  size  style. If empty, a corresponding style will be set
    /// @deco  deco  style. If empty, a corresponding style will be set
    /// @profile track content style. If empty, a corresponding style will be set
    /// @label label position. If empty, a corresponding style will be set
    static void LoadSettingStyles(const CGuiRegistry& reg, string& theme,
                                  string& color, string& size, string& deco,
                                  string& profile, string& label);

private:
    void    x_VerifySettings() const;

    void    x_LoadFeatSettings();
    void    x_SaveFeatSettings();

    void    x_LoadHistSettings();
    void    x_SaveHistSettings();

    /// @name Forbidden
    /// @{
    CSeqGraphicConfig(const CSeqGraphicConfig&);
    CSeqGraphicConfig& operator=(const CSeqGraphicConfig&);
    /// @}

private:
    CRef<CGuiRegistry>  m_Registry; ///> the registry these views come from.

    /// @name current theme and setting styles
    /// @{
    string  m_CurrTheme;    ///< current theme
    string  m_CurrColor;    ///< color theme
    string  m_CurrSize;     ///< size level
    string  m_CurrDeco;     ///< decoration, mainly for features
    string  m_CurrProfile;  ///< feature panel track profile
    string  m_CurrLayout;   ///< layout style
    string  m_CurrLabelPos; ///< label position style
    string  m_CurrGlobal;   ///< global setting style
    /// @}

    bool    m_AdjustRuler;
    bool    m_ShowComments;
    bool    m_ShowSimpleComments;
    bool    m_ShowMessage;
    bool    m_ReserveCmmtSpace;
    bool    m_ShowCmmtBg;
    bool    m_RepeatComment;
    bool    m_AutoHideTrack;    ///< Automatically hide tracks w/o data
    bool    m_ShowHorzGrid;
    bool    m_ShowVertGrid;
    bool    m_ShowTrackMinusPlusSign;
    bool    m_BackgroundLoading;
    bool    m_CgiMode;      ///< not loaded from configure file
    bool    m_ShowDownwardHairlines;

    /// Cached sizes.
    int     m_HairLineOption;
    int     m_CmmtRepDist;
    int     m_OverviewCutoff;   ///< cut-off between overview and detail mode(base per pixel)
    int     m_HistogramCutoff;  ///< Num of features

    float   m_LineWidth;
    float   m_ZoomFactor;       ///< Per pexel of a mouse movement
    float   m_BarHeight;
    float   m_CmmtTopMargin;
    float   m_CmmtBottomMargin;
    int     m_ObjSpace;
    int     m_IconSize;
    int     m_IconSignSize;
    EAnnotDepth m_AnnotDepth;

    /// cached fonts.
    CRef<CGlTextureFont> m_LabelFont;
    CRef<CGlTextureFont> m_CommentFont;
    CRef<CGlTextureFont> m_TitleFont;

    /// Cached colors.
    CRgbaColor  m_Background;
    CRgbaColor  m_SelHairlineOne;
    CRgbaColor  m_SelHairlineMany;
    CRgbaColor  m_SelHairlineDown;
    CRgbaColor  m_BGComment;
    CRgbaColor  m_FGComment;
    CRgbaColor  m_TitleBackground;
    CRgbaColor  m_NonNCBITitleBackground;
    CRgbaColor  m_SelLabel;
    CRgbaColor  m_Selection;
    CRgbaColor  m_GridHorizontal;
    CRgbaColor  m_GridVertical;

    /// cached histogram rendering parameters.
    TFeatParams m_FeatSettings;

    CRef<CHistParamsManager> m_HistParamsManager;

    /// cached clone placement settings.
    CRef<CClonePlacementParams> m_ClonePlacementSettings;
    
    CRef<CCommentConfig> m_CommentConfig;

    CRef<CCustomFeatureColor>  m_CustomFeatColors;

    CRef<CCategoryConfig> m_CategoryConfig;

    mutable bool m_Dirty;
};

///////////////////////////////////////////////////////////////////////////////
/// CSeqGraphicConfig inline methods
///
inline
const string& CSeqGraphicConfig::ThemeKey()
{
    static string key = "Theme";
    return key;
}

inline
const string& CSeqGraphicConfig::DefTheme()
{
    static string key = "Default";
    return key;
}

inline
const string& CSeqGraphicConfig::GetCurrTheme() const
{
    return m_CurrTheme;
}

inline
void CSeqGraphicConfig::SetCurrTheme(const string& theme)
{
    m_CurrTheme = theme;
}

inline
const string& CSeqGraphicConfig::GetColorTheme() const
{
    return m_CurrColor;
}

inline
void CSeqGraphicConfig::SetColorTheme(const string& theme)
{
    m_CurrColor = theme;
}

inline
const string& CSeqGraphicConfig::GetSizeLevel() const
{
    return m_CurrSize;
}

inline
void CSeqGraphicConfig::SetSizeLevel(const string& size)
{
    m_CurrSize = size;
}

inline
const string& CSeqGraphicConfig::GetDecorateStyle() const
{
    return m_CurrDeco;
}

inline
void CSeqGraphicConfig::SetDecorateStyle(const string& style)
{
    m_CurrDeco = style;
}

inline
const string& CSeqGraphicConfig::GetTrackProfile() const
{
    return m_CurrProfile;
}

inline
void CSeqGraphicConfig::SetTrackProfile(const string& profile)
{
    m_CurrProfile = profile;
}

inline
const string& CSeqGraphicConfig::GetLayout() const
{
    return m_CurrLayout;
}

inline
void CSeqGraphicConfig::SetLayout(const string& layout)
{
    m_CurrLayout = layout;
}

inline
const string& CSeqGraphicConfig::GetLabelPos() const
{
    return m_CurrLabelPos;
}

inline
void CSeqGraphicConfig::SetLabelPos(const string& label_pos)
{
    m_CurrLabelPos = label_pos;
}

inline
const string& CSeqGraphicConfig::GetGlobalSettingStyle() const
{
    return m_CurrGlobal;
}

inline
void CSeqGraphicConfig::SetGlobalSettingStyle(const string& style)
{
    m_CurrGlobal = style;
}

inline
bool CSeqGraphicConfig::IsAjdustRulerToSelection() const
{
    return m_AdjustRuler;
}

inline
void CSeqGraphicConfig::SetAjdustRulerToSelection(bool flag)
{
    m_AdjustRuler = flag;
}

inline
bool CSeqGraphicConfig::GetShowComments() const
{
    return m_ShowComments;
}

inline
void CSeqGraphicConfig::SetShowComments(bool flag)
{
    m_ShowComments = flag;
}

inline
bool CSeqGraphicConfig::GetShowSimpleComments() const
{
    return m_ShowSimpleComments;
}

inline
void CSeqGraphicConfig::SetShowSimpleComments(bool flag)
{
    m_ShowSimpleComments = flag;
}

inline
bool CSeqGraphicConfig::GetShowMessage() const
{
    return m_ShowMessage;
}

inline
void CSeqGraphicConfig::SetShowMessage(bool flag)
{
    m_ShowMessage = flag;
}

inline
bool CSeqGraphicConfig::ReserveCmmtSpace() const
{
    return m_ReserveCmmtSpace;
}

inline
void CSeqGraphicConfig::SetReserveCmmtSpace(bool flag)
{
    m_ReserveCmmtSpace = flag;
}

inline
bool CSeqGraphicConfig::GetShowCommentBg() const
{
    return m_ShowCmmtBg;
}

inline
void CSeqGraphicConfig::SetShowCommentBg(bool flag)
{
    m_ShowCmmtBg = flag;
}

inline
bool CSeqGraphicConfig::GetRepeatComment() const
{
    return m_RepeatComment;
}

inline
void CSeqGraphicConfig::SetRepeatComment(bool flag)
{
    m_RepeatComment = flag;
}

inline
bool CSeqGraphicConfig::AutoHideTrack() const
{
    return m_AutoHideTrack;
}

inline
void CSeqGraphicConfig::SetAutoHideTrack(bool flag)
{ 
    m_AutoHideTrack = flag;
}

inline
bool CSeqGraphicConfig::GetShowHorzGrid() const
{
    return m_ShowHorzGrid;
}

inline
void CSeqGraphicConfig::SetShowHorzGrid(bool flag)
{
    m_ShowHorzGrid = flag;
}

inline
bool CSeqGraphicConfig::GetShowVertGrid() const
{
    return m_ShowVertGrid;
}

inline
void CSeqGraphicConfig::SetShowVertGrid(bool flag)
{
    m_ShowVertGrid = flag;
}

inline
bool CSeqGraphicConfig::GetShowTrackMinusPlusSign() const
{
    return m_ShowTrackMinusPlusSign;
}

inline
void CSeqGraphicConfig::SetShowTrackMinusPlusSign(bool flag)
{
    m_ShowTrackMinusPlusSign = flag;
}

inline
bool CSeqGraphicConfig::GetBackgroundLoading() const
{
    return m_BackgroundLoading;
}

inline
void CSeqGraphicConfig::SetBackgroundLoading(bool flag)
{
    m_BackgroundLoading = flag;
}

inline
bool CSeqGraphicConfig::GetCgiMode() const
{
    return m_CgiMode;
}

inline
void CSeqGraphicConfig::SetCgiMode(bool flag)
{
    m_CgiMode = flag;
}

inline
bool CSeqGraphicConfig::GetShowDownwardHairlines() const
{
    return m_ShowDownwardHairlines;
}

inline
void CSeqGraphicConfig::SetShowDownwardHairlines(bool flag)
{
    m_ShowDownwardHairlines = flag;
}

inline
CRef<CGlTextureFont> CSeqGraphicConfig::GetLabelFont() const
{
    return m_LabelFont;
}

inline
void CSeqGraphicConfig::SetLabelFont(CGlTextureFont::EFontFace font,
                                     unsigned int font_size)
{
    m_LabelFont->SetFontFace(font);
    m_LabelFont->SetFontSize(font_size);
}

inline
CRef<CGlTextureFont> CSeqGraphicConfig::GetCommentFont() const
{
    return m_CommentFont;
}

inline
void CSeqGraphicConfig::SetCommentFont(CGlTextureFont::EFontFace font, 
                                       unsigned int font_size)
{
    m_CommentFont->SetFontFace(font);
    m_CommentFont->SetFontSize(font_size);
}

inline
CRef<CGlTextureFont> CSeqGraphicConfig::GetTitleFont() const
{
    return m_TitleFont;
}

inline
void CSeqGraphicConfig::SetTitleFont(CGlTextureFont::EFontFace font, 
                                       unsigned int font_size)
{
    m_TitleFont->SetFontFace(font);
    m_TitleFont->SetFontSize(font_size);
}


inline
const CRgbaColor& CSeqGraphicConfig::GetBkColor() const
{
    return m_Background;
}

inline
void CSeqGraphicConfig::SetBkColor(const CRgbaColor& color)
{
    m_Background = color;
}

inline
const CRgbaColor& CSeqGraphicConfig::GetSelHairlineOneColor() const
{
    return m_SelHairlineOne;
}

inline
void CSeqGraphicConfig::SetSelHairlineOneColor(const CRgbaColor& color)
{
    m_SelHairlineOne = color;
}

inline
const CRgbaColor& CSeqGraphicConfig::GetSelHairlineManyColor() const
{
    return m_SelHairlineMany;
}

inline
void CSeqGraphicConfig::SetSelHairlineManyColor(const CRgbaColor& color)
{
    m_SelHairlineMany = color;
}

inline
const CRgbaColor& CSeqGraphicConfig::GetSelHairlineDownColor() const
{
    return m_SelHairlineDown;
}

inline
void CSeqGraphicConfig::SetSelHairlineDownColor(const CRgbaColor& color)
{
    m_SelHairlineDown = color;
}

inline
const CRgbaColor& CSeqGraphicConfig::GetBGCommentColor() const
{
    return m_BGComment;
}

inline
void CSeqGraphicConfig::SetBGCommentColor(const CRgbaColor& color)
{
    m_BGComment = color;
}

inline
const CRgbaColor& CSeqGraphicConfig::GetFGCommentColor() const
{
    return m_FGComment;
}

inline
void CSeqGraphicConfig::SetFGCommentColor(const CRgbaColor& color)
{
    m_FGComment = color;
}

inline
const CRgbaColor& CSeqGraphicConfig::GetTitleBackground() const
{
    return m_TitleBackground;
}

inline
void CSeqGraphicConfig::SetTitleBackground(const CRgbaColor& color)
{
    m_TitleBackground = color;
}

inline
const CRgbaColor& CSeqGraphicConfig::GetNonNCBITitleBackground() const
{
    return m_NonNCBITitleBackground;
}

inline
void CSeqGraphicConfig::SetNonNCBITitleBackground(const CRgbaColor& color)
{
    m_NonNCBITitleBackground = color;
}

inline
const CRgbaColor& CSeqGraphicConfig::GetSelLabelColor() const
{
    return m_SelLabel;
}

inline
void CSeqGraphicConfig::SetSelLabelColor(const CRgbaColor& color)
{
    m_SelLabel = color;
}

inline
const CRgbaColor& CSeqGraphicConfig::GetSelectionColor() const
{
    return m_Selection;
}

inline
void CSeqGraphicConfig::SetSelectionColor(const CRgbaColor& color)
{
    m_Selection = color;
}

inline
const CRgbaColor& CSeqGraphicConfig::GetHorizontalGridColor() const
{
    return m_GridHorizontal;
}

inline
void CSeqGraphicConfig::SetHorizontalGridColor(const CRgbaColor& color)
{
    m_GridHorizontal = color;
}

inline
const CRgbaColor& CSeqGraphicConfig::GetVerticalGridColor() const
{
    return m_GridVertical;
}

inline
void CSeqGraphicConfig::SetVerticalGridColor(const CRgbaColor& color)
{
    m_GridVertical = color;
}

inline
int CSeqGraphicConfig::GetHairlineOption() const
{
    return m_HairLineOption;
}

inline
void CSeqGraphicConfig::SetHairlineOption(int option)
{
   m_HairLineOption = option;
}

inline
int CSeqGraphicConfig::GetCommentRepeatDist() const
{
    return m_CmmtRepDist;
}

inline
void CSeqGraphicConfig::SetCommentRepeatDist(int d)
{
    m_CmmtRepDist = d;
}

inline
int CSeqGraphicConfig::GetOverviewCutoff() const
{
    return m_OverviewCutoff;
}

inline
void CSeqGraphicConfig::SetOverviewCutoff(int cutoff)
{
   m_OverviewCutoff = cutoff;
}

inline
int CSeqGraphicConfig::GetHistogramCutoff() const
{
    // defines the number of features to be in historgam in detailed mode
    // default is 50 features in current visible reange
    return m_HistogramCutoff;
}

inline
void CSeqGraphicConfig::SetHistogramCutoff(int cutoff)
{
   m_HistogramCutoff = cutoff;
}

inline
float CSeqGraphicConfig::GetLineWidth() const
{
    return m_LineWidth;
}

inline
void CSeqGraphicConfig::SetLineWidth(float w)
{
    m_LineWidth = w;
}

inline
float CSeqGraphicConfig::GetZoomFactor() const
{
    return m_ZoomFactor;
}

inline
void CSeqGraphicConfig::SetZoomFactor(float f)
{
    m_ZoomFactor = f;
}

inline
void CSeqGraphicConfig::SetBarHeight(float h)
{
    m_BarHeight = h;
}

inline
float CSeqGraphicConfig::GetBarHeight() const
{
    return m_BarHeight;
}

inline
float CSeqGraphicConfig::GetCommentTopMargin() const
{
    return m_CmmtTopMargin;
}

inline
void  CSeqGraphicConfig::SetCommentTopMargin(float s)
{
    m_CmmtTopMargin = s;
}

inline
float CSeqGraphicConfig::GetCommentBottomMargin() const
{
    return m_CmmtBottomMargin;
}

inline
void  CSeqGraphicConfig::SetCommentBottomMargin(float s)
{
    m_CmmtBottomMargin = s;
}

inline
int CSeqGraphicConfig::GetObjectSpace() const
{
    return m_ObjSpace;
}

inline
void CSeqGraphicConfig::SetObjectSpace(int s)
{
    m_ObjSpace = s;
}

inline
int CSeqGraphicConfig::GetIconSize() const
{
    return m_IconSize;
}

inline
void CSeqGraphicConfig::SetIconSize(int s)
{
    m_IconSize = s;
}

inline
int CSeqGraphicConfig::GetIconSignSize() const
{
    return m_IconSignSize;
}

inline
void CSeqGraphicConfig::SetIconSignSize(int s)
{
    m_IconSignSize = s;
}

inline
CSeqGraphicConfig::EAnnotDepth CSeqGraphicConfig::GetAnnotDepth() const
{
    return m_AnnotDepth;
}

inline
void CSeqGraphicConfig::SetAnnotDepth(int depth)
{
    m_AnnotDepth = (EAnnotDepth)depth;
}

inline CSeqGraphicConfig::TFeatParams&
CSeqGraphicConfig::SetFeatSettings()
{
    return m_FeatSettings;
}

inline
CRef<CHistParamsManager> CSeqGraphicConfig::GetHistParamsManager() const
{
    return m_HistParamsManager;
}

inline CRef<CHistParams>
CSeqGraphicConfig::GetHistParams(TFeatSubtype subtype) const
{
    return m_HistParamsManager->GetHistParams(subtype);
}

inline CRef<CHistParams>
CSeqGraphicConfig::GetHistParams(const string& name) const
{
    return m_HistParamsManager->GetHistParams(name);
}

inline
void CSeqGraphicConfig::IgnoreFeatureStrand()
{
    NON_CONST_ITERATE(TFeatParams, iter, m_FeatSettings) {
        iter->second->m_ShowStrandIndicator = false;
    }
}

inline
CConstRef<CClonePlacementParams>
CSeqGraphicConfig::GetClonePlacementParams() const
{
    return CConstRef<CClonePlacementParams>(m_ClonePlacementSettings.GetPointer());
}

inline
CConstRef<CCustomFeatureColor> CSeqGraphicConfig::GetCustomFeatColors() const
{
    return CConstRef<CCustomFeatureColor>(m_CustomFeatColors.GetPointer());
}

inline
void CSeqGraphicConfig::SetDirty(bool f)
{ m_Dirty = f; }

inline
bool CSeqGraphicConfig::GetDirty() const
{ return m_Dirty; }
    
inline
void CSeqGraphicConfig::x_LoadHistSettings()
{
    m_HistParamsManager->LoadSettings(m_CurrColor, m_CurrSize);
}

inline
void CSeqGraphicConfig::x_SaveHistSettings()
{
    m_HistParamsManager->SaveSettings(m_CurrColor, m_CurrSize);
}


END_NCBI_SCOPE

#endif  /* GUI_WIDGETS_SEQ_GRAPHIC___SEQGRAPHIC_CONF__HPP */
