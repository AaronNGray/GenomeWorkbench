#ifndef GUI_WIDGETS_SEQ_GRAPHIC___TRACK_CONFIG_MANAGER__HPP
#define GUI_WIDGETS_SEQ_GRAPHIC___TRACK_CONFIG_MANAGER__HPP

/*  $Id: track_config_manager.hpp 42531 2019-03-19 18:13:11Z rudnev $
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
* Authors:  Liangshou Wu, Victor Joukov
*
* File Description:
*
*/

#include <corelib/ncbiobj.hpp>
#include <gui/gui_export.h>

#include <gui/objects/DisplayOptions.hpp>
#include <gui/objects/DisplayOptionSet.hpp>

#include <gui/objects/DisplayOptionValue.hpp>
#include <gui/objects/DisplayOption.hpp>
#include <gui/objects/DisplayTrack.hpp>
#include <gui/objects/DisplayOptionsBundle.hpp>

#include <gui/objutils/utils.hpp>
#include <gui/widgets/seq_graphic/layout_track.hpp>
#include <gui/widgets/seq_graphic/layout_track_proxy.hpp>

BEGIN_NCBI_SCOPE

class CSeqGraphicConfig;
struct STrackSettings;

///////////////////////////////////////////////////////////////////////////////
/// CTrackConfigManager a help class to manage layout track configuration.
/// The specific tasks include registering basic tracks, initializing
/// track factory list, loading/saving track settings, merging/aligning/
/// updating any existing track settings based on other track settings.
///
class NCBI_GUIWIDGETS_SEQGRAPHIC_EXPORT CTrackConfigManager : public CObject
{
public:
    typedef map< string, CIRef<ILayoutTrackFactory> >  TFactoryMap;

    typedef vector<string>  TNAAs;    
    typedef ILayoutTrackFactory::TAnnotMetaDataList TAnnotMetaDataList;
    typedef map<int, TAnnotMetaDataList> TKnownAnnotMap;

    typedef map<string, string>  TKeyValuePairs;
    typedef CTempTrackProxy::TTrackProxies TTrackProxies;
    typedef map< string, string > 	TTrackSettings;
    typedef list< TTrackSettings > 	TTrackSettingsSet;

    enum EAnnotMerge {
        eAnnot_Replace,
        eAnnot_Merge,
        eAnnot_NoChange
    };

    CTrackConfigManager();
    virtual ~CTrackConfigManager() {};

    static void RegisterSGTracks();

    const ILayoutTrackFactory* GetTrackFactory(const string& key) const;
    const TFactoryMap& GetTrackFactories() const;

    list<string> GetTrackTypes() const;
    CConstRef<objects::CDisplayOptionsBundle>
        GetTrackDisplayOptions(const list<string>& keys) const;

    /// Get known annotation based on level number.
    /// @param level annotation level
    /// @return return the corresponding annot list.
    ///         If not available, an empty list will be returned.
    ///         Special case, level=-1 will be treated the same as level 0.
    const TAnnotMetaDataList& GetKnownAnnots(int level) const;

    void SetKnownAnnots(const TAnnotMetaDataList& annots, int level);

    bool OnShownList(const CTempTrackProxy* p_proxy,
                     const CTempTrackProxy* t_proxy,
                     const string& track_name) const;

    static void AdjustTrackOrder(CTempTrackProxy* t_proxy);

    /// @name cgi mode-related methods
    /// @{
    /// Align the existing track configuration to the input settings set.
    /// Update all the settings in the original tracks to the ones in the
    /// input track list.  This is very close to completely replacing the 
    /// the original track settings. The differences include: If a track 
    /// presents only in the original track list, it won't be removed from
    /// the track configuration, but it will be turned off regardless of
    /// its oringal on/off state. The potential updated settings include
    /// annotation list, track on/off state, track order, and track-specific
    /// settings. Track name will not be updated since it is considered as
    /// part of the track identity along with the track key and subkey.
    void AlignTrackConfig(TTrackProxies& t_settings,
        const TTrackSettingsSet& in_settings);

    /// Update the existing track configuration.
    /// The input settings will be applied on the existing tracks.
    /// The settings for the tracks that only present in the input settings_set
    /// will be ginored. The potential updated settings include track
    /// annotation list and track-specific settings.  Track on/off state,
    /// track name and track order will not be updated.
    /// @param align_annot indicate if annots in the input settings_set, if any
    /// will be used.
    void UpdateTrackConfig(TTrackProxies& t_settings,
        const TTrackSettingsSet& in_settings);

    /// Merge with the existing track configuration.
    /// If a track presents in the input settings_set, but not in the existing
    /// track list, that track will be added.  When merging settings on the
    /// tracks presented in both the input track list and original tracks, 
    /// the settings in the input track list will be honored. The potential
    /// merged settings include track on/off state, annots, and track-specific
    /// settigns.  Track order in the original track list won't be affected.
    /// @param merge_annots indicate if annots in the input settings_set will be
    /// merged with the ones in the original track settings.
    void MergeTrackConfig(TTrackProxies& t_settings,
        const TTrackSettingsSet& settings_set, EAnnotMerge merge);

    /// Reset the track configuration with input settings.
    static void SetTrackConfig(TTrackProxies& t_settings,
        const TTrackSettingsSet& in_settings,
        int order_offset = 0);
    
    /// Create configuration for a give track proxy.
    /// @param title_base is recommended title base name. It may be
    ///        ignored.
    
    CRef<objects::CTrackConfig> CreateTrackConfig(CTempTrackProxy* temp_proxy,
                                         CSeqGraphicConfig& g_config,
                                         const string& title_base) const;

    // creates dispaly name from Track proxy
    void CreateDisplayName(const CAnnotMetaData* meta_data, const CTempTrackProxy& temp_proxy, string& display_name) const;

    static void GetUsedFeatureKeys(set<string>& subkeys, const TTrackProxies& proxies);
    static bool ValidFeatureTrack(const objects::CFeatListItem& item);

    /// helper class to faciliatet subtrack processing
    /// build pseudo compound track index 
    /// and provides find method
    class CTrackIndex {
    public:
        CTrackIndex(TTrackProxies& t_settings);
        CRef<CTrackProxy> FindTrackById(const string& track_id);
    private:
        typedef map<string, CTrackProxy*> TTrackIndex;
        TTrackIndex m_IdIndex;
        TTrackIndex m_AnnotIndex;
        TTrackIndex m_UidIndex;
    };
private:
    void x_InitTrackManager();

    void x_AppendKeyDisplayOptions(const string& key,
        objects::CDisplayOptionsBundle& bundle) const;

private:
    TFactoryMap     m_Factories;

    /// Complete list of known annotations with meta-data.
    /// Different annotations may annotated at different levels for
    /// a given sequence, e.g. Chromosome, Scaffold, and Component.
    /// So each level will have a separate list. The level number is
    /// a relative number. level = 0 means current sequence level.
    /// level = 1 means one level down from the current sequence level.
    /// Currently, only named annotations (NAAs) from seq-annot database are
    /// covered which live at level 0. The NAA meta-data are retrieved
    /// using eutils instead of object manager to avoid performance and
    /// memory consumption issues.
    TKnownAnnotMap  m_KnownAnnots;

};


///////////////////////////////////////////////////////////////////////////////
/// CTrackConfigManager inline methods

inline
const CTrackConfigManager::TFactoryMap&
CTrackConfigManager::GetTrackFactories() const
{
    return m_Factories;
}

inline
void CTrackConfigManager::SetKnownAnnots(
    const CTrackConfigManager::TAnnotMetaDataList& annots, int level)
{
    m_KnownAnnots[level] = annots;
}


END_NCBI_SCOPE


#endif	// GUI_WIDGETS_SEQ_GRAPHIC___TRACK_CONFIG_MANAGER__HPP

