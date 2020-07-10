#ifndef GUI_WIDGET_SEQ_GRAPHICS___FEATURE_PANEL__HPP
#define GUI_WIDGET_SEQ_GRAPHICS___FEATURE_PANEL__HPP

/*  $Id: feature_panel.hpp 44617 2020-02-06 19:35:49Z filippov $
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
 */

#include <gui/gui.hpp>
#include <gui/opengl/glbitmapfont.hpp>
#include <gui/objutils/object_index.hpp>
#include <gui/widgets/seq_graphic/external_layout_track.hpp>
#include <gui/widgets/seq_graphic/track_container_track.hpp>
#include <gui/widgets/seq_graphic/layout_conf.hpp>
#include <gui/widgets/gl/html_active_area.hpp>
#include <gui/graph/regular_grid.hpp>

BEGIN_NCBI_SCOPE

class IServiceLocator;

class NCBI_GUIWIDGETS_SEQGRAPHIC_EXPORT CFeaturePanel :
    public CTrackContainer,
    public ITrackInitObserver
{
    friend class CConfigureTracksDlg;

public:
    /// key-value pair for track settings
    typedef map< string, string > 	TTrackSettings;
    typedef list< TTrackSettings > 	TTrackSettingsSet;
    typedef list< CWeakRef<CSeqGlyph> > TSelectedGlyphs;
    typedef CSeqGraphicImageInfo TImageInfo;

    CFeaturePanel(CRenderingContext* r_cntx, bool cgi_mode);
    virtual ~CFeaturePanel();

    /// @name CSeqGlyph virtual method.
    /// @{
    virtual void Update(bool layout_only);
    virtual CRef<CSeqGlyph> HitTest(const TModelPoint& p);
    virtual bool HitTestHor(TSeqPos x, const CObject *obj);
    /// @}

    void SetDSContext(ISGDataSourceContext* ds_ctx);
    void SetInputObject(SConstScopedObject& obj);
    void ConfigureTracks();
    void OnDataChanging();
    void OnDataChanged();
    void SetSelectedFeats(const TSelectedGlyphs& feats);
    void SetSelectedCDSFeats(const TSelectedGlyphs& cds_feats);
    void SetExternalGlyphs(const CSeqGlyph::TObjects& objs);
    void UpdateGridStep(int step);

    void SetAssembly(const string& assembly);
    void ShowConfigureTracksDlg(const string& category);

    objects::CMappedFeat FindText(const string &text, bool match_case);
    virtual void ResetSearch();
    void SetServiceLocator(IServiceLocator* s) { m_ServiceLocator = s; }

    /// Optional image info structure to return data to (cgi) caller
    void SetImageInfo(TImageInfo* info) { m_ImageInfo = info; }

    /// Reset tracks to the original default track list.
    /// Resetting completely wipes out current track list and
    /// user settings.
    void LoadDefaultTracks();

    /// @name cgi mode-related methods
    /// @{
    /// Reset the track configuration with input settings.
    void SetTrackConfig(const TTrackSettingsSet& settings_set);
    void SetTrackConfig(const TTrackProxies& track_settings);

    /// skip track existence verification.
    /// Assume tracks exist.
    void SkipGenuineCheck();

    /// Turn on an existing track.
    void ShowTrack(const string& track_key);
    /// @}

    void SaveTrackConfig();

    /// @name CLayoutTrack public methods.
    /// @{
    virtual const CTrackTypeInfo&  GetTypeInfo() const;
    /// @}

    /// @name ITrackInitObserver interface implementation.
    /// @{
    virtual void OnTrackInitFinished(const CLayoutTrack* sender);
    /// @}

    void SetConfigKey(const string& config_key);

    void SetDiscoverTracks(bool value = true);

protected:
    virtual void x_Draw() const;

    /// @name data track pure virtual methods.
    /// @{
    virtual bool x_IsJobNeeded(CAppJobDispatcher::TJobID id) const;
    virtual void x_OnJobCompleted(CAppJobNotification& notify);
    /// @}

    /// @name CLayoutTrack virtual methods
    /// @{
    /// make sure we never collapse feature panel
    virtual void x_Expand(bool /*expand*/) {};
    virtual void x_LoadSettings(const string& preset_style,
                                const TKeyValuePairs& settings);
    virtual void x_SaveSettings(const string& preset_style);
    virtual bool x_Empty() const;
    virtual void x_GetTBRect(TModelRect& rect) const;
    virtual void x_RenderTitleBar() const;
    virtual void x_OnIconClicked(TIconID id);
    /// @}

private:
    enum EFeatPanelIconID {
        eIcon_Color = eIcon_MaxTrackIconID + 1, ///< color theme
        eIcon_Size,                ///< size level
        eIcon_Decoration,          ///< feature decoration
        eIcon_Label                ///< feature label position
    };

    void x_OnLayoutIconClicked();
    void x_OnColorIconClicked();
    void x_OnSizeIconClicked();
    void x_OnDecorationIconClicked();
    void x_OnLabelIconClicked();

    /// reset the track status to be ready for reconfiguring layout tracks.
    void x_ConfigureTracks(bool makeContainersVisible = false);

    /// Create all the exsiting layout tracks.
    /// @param group the container into which the created tracks will be added.
    /// @param level the feature retrieving level used for annotation selector.
    /// There are twofold meanings here. If level >=0, then those tracks with
    /// concept of level will be initialized with that exact level, all other
    /// tracks without level concept won't be initialized. If level == -1, all
    /// registered tracks will be initialized.  For those tracks with level
    /// concept will use adpative annotation selector.  if level == -2, only
    /// those tracks without level concept will be initialized.
    void x_ConfigureSubtracks_Recursive(CTrackContainer* parent,
        const TTrackProxies& proxies, int level);

    CTrackContainer* x_ConfigureSubtracks(CTrackContainer* parent,
        const CTrackProxy* param, int level, const set<string>& usedFeatureSubkeys);

    /// Load settings for feature panel.
    void x_LoadSettings();

    /// Save settings for feature panel.
    void x_SaveSettings();

    void x_InitTrackIcons();

    void x_RenderHairLineSelections() const;
    void x_DrawHairLine(int opt, size_t total, const multiset <TSeqPos>& all_pos,
        TSeqPos pos, TModelUnit line_y) const;
    void x_RenderFeatureGrid() const;
    double x_CalcGridStep(double base_step, double min_step, double max_step) const;

    typedef int TLevel;
    typedef vector<TLevel> TDisplayLevels;

    /// Only meaningful if Multilevel is true.
    bool x_GetShowDisplayLevel(TLevel level) const;
    void x_SetShowDisplayLevel(TLevel level, bool show);

    /// create track container for a given annotation level.
    /// This will be used only in multiple level layout mode.
    CRef<CTrackContainer> x_CreateLevelTrack(int level, int id,
        const string& track_name, bool shown);

    /// need show special grid for the selected cds features.
    bool x_NeedSpecialGrid() const;

protected:
    CIRef<ISGDataSourceContext> m_DSContext;
    SConstScopedObject          m_InputObj;

    void x_InitNATracks(bool makeContainersVisible);

private:
    CRef<CExternalLayoutTrack> m_ExtLayoutTrack;

    /// initial track layout settings.
    TTrackProxies m_TrackSettings;

    const TSelectedGlyphs* m_SelFeatures;
    const TSelectedGlyphs* m_SelCDSFeatures;

    /// graphical grid.
    CRegularGridGen      m_Gen;
    CRegularGridRenderer m_Grid;

    bool                 m_IsMultiLevel;
    TDisplayLevels       m_DisplayLevels;

    /// flag indicating whether icon textures have been created.
    mutable bool    m_IconTexInitialized;

    /// special flag indicating if it is in cgi mode.
    /// we need this to avoid rendering some glyphs (like texture/icons)
    bool            m_CgiMode;

    static CTrackTypeInfo m_TypeInfo;

    /// Information about image that can be returned to client
    TImageInfo*  m_ImageInfo;

    /// Base regitry key for panel setting and tracks
    /// default "GBPlugins.SeqGraphicFeatPanel" is for SV 
    /// MSA sets its own "GBPlugins.MSAFeatPanel" to separate MSA and SV tracks
    string m_ConfigKey;

    // if true, DataSource loads NA metadata for tracks discovery
    // not applicable if in CgiMode
    bool m_DiscoverTracks = true;

    TAnnotMetaDataList m_NAData;

    bool    m_AssemblyLoaded;
    vector<CRef<objects::CGC_Assembly> > m_Assemblies;
    string  m_DefaultAssembly;

    void x_StartAssemblyJob();
    void x_OnAssemblyLoaded(const list<CRef<objects::CGC_Assembly> >& assemblies);

public:
    void SetDefaultAssembly(const string& defaultAssembly) { m_DefaultAssembly = defaultAssembly; }
    bool AssemblyLoaded() const { return m_AssemblyLoaded; }
    const vector<CRef<objects::CGC_Assembly> >& GetAssemblies() const { return m_Assemblies; }
    string GetCurrentAssembly() const;

private:
    string m_search_text;
    bool m_search_match_case = false;
    size_t m_next_result = 0;
    vector<objects::CMappedFeat> m_search_results;
    IServiceLocator* m_ServiceLocator = 0;
};

inline
void CFeaturePanel::SetDSContext(ISGDataSourceContext* ds_ctx)
{ m_DSContext.Reset(ds_ctx); }

inline
void CFeaturePanel::SetSelectedFeats(const TSelectedGlyphs& feats)
{ m_SelFeatures = &feats; }

inline
void CFeaturePanel::SetSelectedCDSFeats(const TSelectedGlyphs& cds_feats)
{ m_SelCDSFeatures = &cds_feats; }

inline
void CFeaturePanel::UpdateGridStep(int step)
{ m_Gen.SetStep((double)step); }

inline
void CFeaturePanel::SkipGenuineCheck()
{ SetSkipGenuineCheck(true); }

inline
bool CFeaturePanel::x_NeedSpecialGrid() const
{ return m_SelCDSFeatures  &&  !m_SelCDSFeatures->empty(); }

inline
void CFeaturePanel::SetConfigKey(const string& config_key)
{
    _ASSERT(!config_key.empty());
    m_ConfigKey = config_key;
}

inline
void CFeaturePanel::SetTrackConfig(const TTrackProxies& track_settings)
{
    m_TrackSettings = track_settings;
}

inline
void CFeaturePanel::SetDiscoverTracks(bool value)
{
    m_DiscoverTracks = value;
}

END_NCBI_SCOPE


#endif  // GUI_WIDGET_SEQ_GRAPHICS___FEATURE_PANEL__HPP
