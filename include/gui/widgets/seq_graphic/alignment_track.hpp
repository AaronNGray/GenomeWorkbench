#ifndef GUI_WIDGETS_SEQ_GRAPHIC___ALIGNMENT_TRACK__HPP
#define GUI_WIDGETS_SEQ_GRAPHIC___ALIGNMENT_TRACK__HPP

/*  $Id: alignment_track.hpp 39810 2017-11-07 20:48:32Z shkeda $
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

#include <corelib/ncbiobj.hpp>
#include <gui/utils/extension.hpp>
#include <gui/objutils/reg_settings.hpp>
#include <gui/widgets/seq_graphic/data_track.hpp>
#include <gui/widgets/seq_graphic/alignment_conf.hpp>
#include <gui/widgets/seq_graphic/seqgraphic_job_result.hpp>
#include <gui/widgets/seq_graphic/seqgraphic_alignment_ds.hpp>
#include <gui/widgets/seq_graphic/align_statistics_glyph.hpp>
#include <gui/widgets/seq_graphic/column_layout_policy.hpp>

//#include <objects/seqloc/Seq_interval.hpp>
#include <objmgr/annot_selector.hpp>

class wxEvtHandler;

BEGIN_NCBI_SCOPE

class CGeneModelConfig;

///////////////////////////////////////////////////////////////////////////////
///   CAlignmentTrack -- 
///

class NCBI_GUIWIDGETS_SEQGRAPHIC_EXPORT CAlignmentTrack :
    public CDataTrack
{
    friend class CAlignmentTrackFactory;

public:
    enum EJobType {
        eJob_Align,
        eJob_AlignFeats,
        eJob_AlignScore,
        eJob_AlignStat,
        eJob_CoverageGraph
    };

    enum ELayout {
        eLayout_Adaptive = 0,           ///< adaptive using loading costs to switch between rendering modes
        eLayout_Packed,          ///< smear/pack all alignments into one smear bar
        eLayout_ExpandedByPos,      ///< one alignment per row sorted by pos
        eLayout_Full,      ///< one alignment per row sorted by pos
        eLayout_Default = eLayout_Adaptive
    };

    enum EAlignGlyphType {
        eAlign_Multi,
        eAlign_PW,
        eAlign_MatePair,
        eAlign_Smear,
        eAlign_Graph,
        eAlign_Stat,
        eAlign_Non
    };

    /// @name static methods.
    /// static methods for converting layout str and label position str
    /// layout setting and label position setting.
    /// @{
    static ELayout LayoutStrToValue(const string& layout);
    static const string& LayoutValueToStr(ELayout layout);
    static CAlignmentConfig::ELabelPosition LabelPosStrToValue(const string& pos);
    static const string& LabelPosValueToStr(CAlignmentConfig::ELabelPosition pos); 
    static CAlignmentConfig::EHideSraAlignments HideFlagStrToValue(const string& hideSra);
    static const string& HideFlagValueToStr(CAlignmentConfig::EHideSraAlignments hideSra); 
    static CAlignmentConfig::EUnalignedTailsMode UnalignedTailsModeStrToValue(const string& tailsMode);
    static const string& UnalignedTailsModeValueToStr(CAlignmentConfig::EUnalignedTailsMode tailsMode);
    /// @}


    CAlignmentTrack(CSGAlignmentDS* ds, CRenderingContext* r_cntx);
    virtual ~CAlignmentTrack();

    /// @name CLayoutTrack public methods.
    /// @{
    virtual const CTrackTypeInfo&  GetTypeInfo() const;
    virtual string GetFullTitle() const;
    /// @}

    virtual CRef<CSGGenBankDS>  GetDataSource();
    virtual CConstRef<CSGGenBankDS>  GetDataSource() const;

    void SetAnnot(const string& annot);
    void SetDSContext(ISGDataSourceContext* ds_ctx);

    void SetAlnType(IAlnExplorer::EAlignType type);

    /// @name content menu event handlers.
    /// @{
    void OnToggleContent(int id);
    void OnChangeProjectedFeats();
    /// @}

    void SetDefaultProfile(const string& profile);

protected:
    /// @name CLayoutTrack virtual methods overloading.
    /// @{
    virtual void x_LoadSettings(const string& preset_style,
                                const TKeyValuePairs& settings);

    /// save part of settings to a profile string.
    /// Only the potential settings changed by users will be saved.
    virtual void x_SaveSettings(const string& preset_style);

    virtual void x_OnIconClicked(TIconID id);
    /// @}

    /// @name CDataTrack pure virtual interfaces
    /// @{
    virtual void x_UpdateData();
    virtual void x_OnJobCompleted(CAppJobNotification& notify);
    /// @}

    void x_OnContentIconClicked();
    void x_OnLayoutIconClicked();
    void x_OnScoreIconClicked();
    void x_OnStatIconClicked();
    void x_OnTailsIconClicked();
    void x_OnLayoutPolicyChanged();

    void x_AddAlignmentLayout(const CSGJobResult& result);
    void x_AddAlignFeatLayout(CBatchJobResult& results);
    void x_AddGraphLayout(const CSGJobResult& result);
    void x_AddAlignStatLayout(const CSGJobResult& result);

private:
    enum EAlignIconID {
        eIcon_Score = eIcon_MaxTrackIconID + 1,
        eIcon_Stat = eIcon_MaxTrackIconID + 2,
        eIcon_Tails = eIcon_MaxTrackIconID + 3
    };

private:
    /// @name prohibited copy constructor and assignment operator.
    /// @{
    CAlignmentTrack(const CAlignmentTrack&);
    CAlignmentTrack& operator=(const CAlignmentTrack&);
    /// @}

    void x_AddAlignFeat_Recursive(CLayoutGroup::TObjectList& objs);

    void x_LoadAlignedSeqFeats(vector< CRef<CAlignGlyph> >& aligns,
        CAlignGlyph* aln);

    EAlignGlyphType x_GetAlignGlyphType(const CSeqGlyph* glyph) const;

    /// save all track settings to the configuration file.
    /// this is not used currently.
    void x_SaveConfiguration(const string& preset_style) const;

    /// common function for loading settings for multialign, pairwise aligns,
    /// and align smear.
    void x_LoadAlignmentSettings(const string& key,
        CRef<CAlignmentConfig>& conf);
    
    void x_SaveAlignmentSettings(const string& key,
        CConstRef<CAlignmentConfig> conf) const;

    void x_LoadMultiAlignSettings();
    void x_SaveMultiAlignSettings() const;
    void x_LoadPWAlignSettings();
    void x_SavePWAlignSettings() const;
    void x_LoadAlignSmearSettings();
    void x_SaveAlignSmearSettings() const;
    void x_LoadMPPWAlignSettings();
    void x_LoadMatePairSettings();
    void x_SaveMatePairSettings() const;
    void x_SetMsg();
    CRef<CHistParams> x_GetGraphParams();

    int x_LayoutToCutoff(CAlignmentTrack::ELayout layout);
    int x_LayoutToCutoff_SRA(CAlignmentTrack::ELayout layout);

private:
    /// wx-related event handler.
    wxEvtHandler*           m_ContentHandler;

    string                  m_AnnotName;
    CRef<CSGAlignmentDS>    m_DS;
    CIRef<ISGDataSourceContext> m_DSContext;

    string                  m_DefaultProfile = "Default"; ///> Default Settings profile

    CRef<CAlignmentConfig>  m_MultiAlignConf;
    CRef<CAlignmentConfig>  m_PWAlignConf;
    CRef<CAlignmentConfig>  m_AlignSmearConf;
    CRef<CAlignmentConfig>  m_MPPWAlignConf;    ///< for linked pairwise aligns
    CRef<CMatePairConfig>   m_MatePairConf;
    CRef<CBoundaryParams>   m_GeneGroupConf;
    CRef<CBoundaryParams>   m_FeatGroupConf;
    CRef<CAlnStatConfig>    m_StatConf;
    CRef<CGeneModelConfig>  m_GeneModelConfig;

    ELayout                 m_Layout = eLayout_Adaptive;
    bool                    m_IsDefaultLayout = true;
    bool                    m_ShowAlignedSeqFeats;
    bool                    m_ShowLabel;
    bool                    m_ShowUnalignedTailsForTrans2GenomicAln;
    bool                    m_ShowAlnStat;
    bool                    m_ShowIdenticalBases;

    double m_MinPileUpCost = 3.; /// MinPileUpCost - Show Coverage Graph if cost is higher than MinPileUpCost
    double m_MinAlignCost = 1.5; /// MinAlignCost - Show Alignment Graph if cost is higher than MinAlignCost
    double m_MaxAlignCost = 15; /// MaxAlignCost Show PileUp Graph is cost is higher than MaxAlignCost
    
    int m_MaxAlignShownFull = 250000;
    int m_MaxAlignShownAdaptive = 7500;
    

    // Flag, indicating which SRA alignments to hide
    CAlignmentConfig::EHideSraAlignments m_HideSraAlignments;
    // Flag, indicating how to display unaligned tails
    CAlignmentConfig::EUnalignedTailsMode m_UnalignedTailsMode;

    // Flag indicate if compact display is enabled.
    bool                    m_CompactEnabled;

    /// Alignment number threshold between compact mode and non-compact mode.
    /// Maximal number of alignments shown in the non-compact mode
    size_t                  m_CompactThreshold;
    /// Alignment number threshold between compact mode and extreme compact mode.
    /// Maximal number of alignments shown in the compact mode
    size_t                  m_ExtremeCompactThreshold;

    typedef vector<int>     TSubtypeVec;
    TSubtypeVec             m_ProjectedFeats;

    string                  m_MultiAlignProfile;
    string                  m_PWAlignProfile;
    string                  m_AlignSmearProfile;
    string                  m_MatePairProfile;

    // cached info
    IAlnExplorer::EAlignType m_AlnType;
    size_t                  m_ObjNum;

    CRef<CColumnLayout>     m_Column;
    CRef<CSimpleLayout>     m_GeneModelLayout;
    vector<CRef<CAlnStatGlyph>>     m_StatGlyph;
    
    static CTrackTypeInfo   m_TypeInfo;

};

///////////////////////////////////////////////////////////////////////////////
///   CAlignmentTrackFactory
///
class NCBI_GUIWIDGETS_SEQGRAPHIC_EXPORT CAlignmentTrackFactory :
    public CObject,
    public ILayoutTrackFactory,
    public ITrackConfigurable,
    public IIconProvider,
    public IExtension
{
public:
    CAlignmentTrackFactory(){}

    /// create a layout track based on the input objects and extra parameters.
    virtual TTrackMap CreateTracks(
        SConstScopedObject& object,
        ISGDataSourceContext* ds_context,
        CRenderingContext* r_cntx,
        const SExtraParams& params = SExtraParams(),
        const TAnnotMetaDataList& src_annots = TAnnotMetaDataList()) const;

    virtual void GetMatchedAnnots(
        const TAnnotMetaDataList& src_annots,
        const ILayoutTrackFactory::SExtraParams& params,
        TAnnotNameTitleMap& out_annots) const;

    virtual bool UnderstandLevel() const
    { return true; }

    virtual bool NeedBackgroundInit() const
    { return true; }

    virtual const CTrackTypeInfo&  GetThisTypeInfo() const
    { return GetTypeInfo(); }

    static const CTrackTypeInfo& GetTypeInfo()
    { return CAlignmentTrack::m_TypeInfo; }


    /// @name ITrackConfigurable interface implementation
    /// @{
    virtual CRef<objects::CTrackConfigSet>
        GetSettings(const string& profile,
                    const TKeyValuePairs& settings,
                    const CTempTrackProxy* track_proxy) const;
    /// @}

    /// @name IIconProvider interface implementation
    /// @{
    virtual void RegisterIconImages() const;
    /// @}

    /// @name IExtension interface implementation
    /// @{
    virtual string GetExtensionIdentifier() const;
    virtual string GetExtensionLabel() const;
    /// @}
};


///////////////////////////////////////////////////////////////////////////////
///   CAlignmentTrack inline methods
///////////////////////////////////////////////////////////////////////////////
inline /*virtual*/
CRef<CSGGenBankDS> CAlignmentTrack::GetDataSource()
{
    return CRef<CSGGenBankDS>(m_DS.GetPointer());
}

inline /*virtual*/
CConstRef<CSGGenBankDS> CAlignmentTrack::GetDataSource() const
{
    return CConstRef<CSGGenBankDS>(m_DS.GetPointer());
}

inline
void CAlignmentTrack::SetDSContext(ISGDataSourceContext* ds_ctx)
{ m_DSContext.Reset(ds_ctx); }

inline
void CAlignmentTrack::SetAlnType(IAlnExplorer::EAlignType type)
{ m_AlnType = type; }

inline
void CAlignmentTrack::x_LoadMultiAlignSettings()
{
    x_LoadAlignmentSettings(m_MultiAlignProfile, m_MultiAlignConf);
    m_MultiAlignConf->m_UnalignedTailsMode = CAlignmentConfig::eTails_ShowGlyph;
    m_MultiAlignConf->m_ShowUnalignedTailsForTrans2GenomicAln = false;
}

inline
void CAlignmentTrack::x_SaveMultiAlignSettings() const
{
    x_SaveAlignmentSettings(m_MultiAlignProfile,
        CConstRef<CAlignmentConfig>(m_MultiAlignConf));
}

inline
void CAlignmentTrack::x_LoadPWAlignSettings()
{
    x_LoadAlignmentSettings(m_PWAlignProfile, m_PWAlignConf);
}

inline
void CAlignmentTrack::x_SavePWAlignSettings() const
{
    x_SaveAlignmentSettings(m_PWAlignProfile,
        CConstRef<CAlignmentConfig>(m_PWAlignConf));
}

inline
void CAlignmentTrack::x_LoadAlignSmearSettings()
{
    x_LoadAlignmentSettings(m_AlignSmearProfile, m_AlignSmearConf);
}

inline
void CAlignmentTrack::x_SaveAlignSmearSettings() const
{
    x_SaveAlignmentSettings(m_AlignSmearProfile,
        CConstRef<CAlignmentConfig>(m_AlignSmearConf));
}

inline
void CAlignmentTrack::x_LoadMPPWAlignSettings()
{
    x_LoadAlignmentSettings(m_PWAlignProfile, m_MPPWAlignConf);
    m_MPPWAlignConf->m_ShowLabel = false;
    m_MPPWAlignConf->m_UnalignedTailsMode = CAlignmentConfig::eTails_ShowGlyph;
    m_MPPWAlignConf->m_ShowUnalignedTailsForTrans2GenomicAln = false;
}

inline
void CAlignmentTrack::SetDefaultProfile(const string& profile)
{
    m_DefaultProfile = profile;
}


END_NCBI_SCOPE

#endif  // GUI_WIDGETS_SEQ_GRAPHIC___ALIGNMENT_TRACK__HPP
