#ifndef GUI_WIDGETS_SEQ_GRAPHIC___FEATURE_TRACK__HPP
#define GUI_WIDGETS_SEQ_GRAPHIC___FEATURE_TRACK__HPP

/*  $Id: feature_track.hpp 44952 2020-04-25 01:20:25Z rudnev $
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
#include <gui/widgets/seq_graphic/ifeature_track_base.hpp>
#include <gui/widgets/seq_graphic/data_track.hpp>
#include <gui/widgets/seq_graphic/feature_ds.hpp>
#include <gui/widgets/seq_graphic/feature_enums.hpp>
#include <gui/widgets/seq_graphic/column_layout_policy.hpp>
#include <gui/widgets/seq_graphic/compact_layout_policy.hpp>

#include <objects/seqloc/Seq_interval.hpp>
#include <objmgr/annot_selector.hpp>


BEGIN_NCBI_SCOPE

class CLinkedFeatsGroup;
class CHistogramGlyph;


class CFeatJobResult;

///////////////////////////////////////////////////////////////////////////////
///   CFeatureTrack --
///

class NCBI_GUIWIDGETS_SEQGRAPHIC_EXPORT CFeatureTrack :
    public CDataTrack, public IAsn1Generator, public IFeatureTrackBase
{
    friend class CFeatureTrackFactory;

public:
    enum ELayout {
        eLayout_None,            ///< unassigned layout
        eLayout_Packed,          ///< pack all feature into a histogram
        eLayout_OneLine,         ///< pack all feature into one line
        eLayout_AdaptiveLayered, ///< adaptively switch between packed and layered
        eLayout_AdaptiveInline,  ///< adaptively switch between packed and inline
        eLayout_ExpandedLayered, ///< show all features with layered layout
        eLayout_ExpandedColumn,  ///< show all features with column layout
        eLayout_ExpandedByPos,   ///< show all features, one per row sorted by pos
        eLayout_ExpandedBySize,  ///< one feature per row sorted by size
        eLayout_Default = eLayout_AdaptiveLayered
    };

    enum EAlignIconID {
        eIcon_Score = eIcon_MaxTrackIconID + 1
    };

    static ELayout LayoutStrToValue(const string& layout);
    static const string& LayoutValueToStr(ELayout layout);

    static ELinkedFeatDisplay LinkedFeatDisplayStrToValue(const string& linked_feat);
    static const string& LinkedFeatDisplayValueToStr(ELinkedFeatDisplay linked_feat);

    CFeatureTrack(CSGFeatureDS* ds, CRenderingContext* r_cntx);
    virtual ~CFeatureTrack();

    /// @name CLayoutTrack public methods.
    /// @{
    virtual const CTrackTypeInfo&  GetTypeInfo() const;
    virtual string GetFullTitle() const;
    /// @}

    virtual CRef<CSGGenBankDS>  GetDataSource();
    virtual CConstRef<CSGGenBankDS>  GetDataSource() const;

    // implement IAsn1Generator method
    void GenerateAsn1(objects::CSeq_entry& seq_entry, TSeqRange range);

    // if true, this track should be handled by stage 2
    virtual bool BypassStage1() { return false; }
    
    // estimate number of features in the specified range
    // returns true if range is acceptable for downloads
    // if false, sMsg will contain a message that can be shown to the user
    virtual bool CheckRange(TSeqRange range, string&) { return true; }

    void SetAnnot(const string& annot);
    void ExcludeAnnotName(const string& annot);
    void SetFeatSubtype(int subtype);
    void SetLayout(ELayout layout);

    int GetFeatSubtype() const;
    virtual void GetFeatSubtypes(set<objects::CSeqFeatData::ESubtype> &subtypes) const {subtypes.insert(static_cast<objects::CSeqFeatData::ESubtype>(m_Subtype));}
    virtual string GetAnnot() const {return m_Annot;}

    // enable these methods to put a breakpoint into
//    virtual void SetHeight(TModelUnit h) {
//        CSeqGlyph::SetHeight(h);
//    };
//    virtual TModelUnit GetHeight() const {
//        return CSeqGlyph::GetHeight();
//    };

protected:
    /// @name CLayoutTrack virtual methods overloading.
    /// @{
    virtual void x_LoadSettings(const string& /*preset_style*/,
                                const TKeyValuePairs& settings);

    /// save part of settings to a profile string.
    /// Only the potential settings changed by users will be saved.
    virtual void x_SaveSettings(const string& /*preset_style*/);

    virtual void x_OnIconClicked(TIconID id);
    /// @}

    /// @name CDataTrack pure virtual interfaces
    /// @{
    virtual void x_UpdateData();
    virtual void x_OnJobCompleted(CAppJobNotification& notify);
    /// @}

private:
    /// @name prohibited copy constructor and assignment operator.
    /// @{
    CFeatureTrack(const CFeatureTrack&);
    CFeatureTrack& operator=(const CFeatureTrack&);
    /// @}

    void x_AddFeaturesLayout(const CSGJobResult& result);

    bool x_NeedHighlight(const CSeqGlyph::TObjects& objs) const;
    bool x_NeedHighlight(const CSeqGlyph* glyph) const;

    /// rearrange the child features if necessary.
    /// regrouping may mean removing child features,
    /// removing parent feature or packing all child
    /// features together.
    void x_RegroupFeats(CSeqGlyph::TObjects& objs, ELinkedFeatDisplay link_mode);

    /// initialize all children.
    /// return true if one of the children needs to be highlighted
    bool x_InitGlyphs_Recursive(CSeqGlyph::TObjects& objs,
        int curr_level, bool compact_mode);

    void x_ExtracGlyphs_Recursive(CSeqGlyph::TObjects& objs,
        CLinkedFeatsGroup* feat_group);
    void x_CountFeats_recursive(const CSeqGlyph::TObjects& objs,
        int& count, bool is_overview);

    void x_OnContentIconClicked();
    void x_OnLayoutIconClicked();
    void x_InitLayoutPolicy(CLayoutGroup& group, ELinkedFeatDisplay link_mode);
    void x_OnLayoutPolicyChanged();

    void x_AdjustFeatureLayout();

    bool x_HasLinkedFeats() const;
    void x_SortFeaturesByWeight(CSeqGlyph::TObjects& objs) const;

    void x_AddHistogram(CHistogramGlyph* hist, int layout = -1);

protected:
    string                  m_AnnotName;
    string                  m_Annot;
    string                  m_Subtitle;
    CRef<CSGFeatureDS>      m_DS;
    objects::SAnnotSelector m_FeatSel;
    CSeqGraphicConfig::TFeatSubtype m_Subtype;

    /// maximal number of features shown in on track.
    /// -1 means no limitation
    int                     m_FeatLimits;
    /// Feature number threshold between compact mode and non-compact mode.
    /// Maximal number of features shown in the non-compact mode
    int                     m_CompactThreshold;
    /// Maximal number of feature rows in one feature track.
    int                     m_MaxRow;
    /// Maximal feature group pixel height in adaptive mode
    int                     m_MaxAdaptiveHeight = 400;

    ELayout                 m_Layout;
    ELinkedFeatDisplay      m_LinkedFeat;

    /// An independent histogram threshold used by SViewer.
    /// It differs from m_FeatLimits which tries to capture all
    /// possible rendering layouts.
    /// 0 means this value has no any effect.
    size_t                  m_HistThreshold;
    CRef<CColumnLayout>     m_Column;
    CRef<CSimpleLayout>     m_GroupSimple;
    CRef<CLayeredLayout>    m_GroupLayered;
    CRef<CLayeredLayout>    m_LinkedGroup;
    CRef<CCompactLayout>    m_Compact;
    CRef<CBoundaryParams>   m_GroupConfig;
    bool                    m_HasLinkedFeats;

    static CTrackTypeInfo   m_TypeInfo;
};

///////////////////////////////////////////////////////////////////////////////
/// 
/// 
///
class NCBI_GUIWIDGETS_SEQGRAPHIC_EXPORT CFeatJobResult : public CJobResultBase
{
public:
    CFeatureTrack::ELayout m_Layout = CFeatureTrack::eLayout_None;
};


///////////////////////////////////////////////////////////////////////////////
///   CFeatureTrackFactory
///
class NCBI_GUIWIDGETS_SEQGRAPHIC_EXPORT CFeatureTrackFactory :
    public CObject,
    public ILayoutTrackFactory,
    public ITrackConfigurable,
    public IExtension
{
public:
    CFeatureTrackFactory() : 
        m_DefaultLayout(CFeatureTrack::eLayout_AdaptiveLayered),
        m_DefaultLinkedFeat(ELinkedFeatDisplay::eLFD_Packed) {}

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

    virtual void CloneTrack(const CLayoutTrack* track,
        CTempTrackProxy* track_proxy) const;

    virtual bool UnderstandLevel() const
    { return true; }

    // this track supports export to ASN1
    // this means that it is  able to accept a NULL as
    // CRenderingContext in a call to CreateTracks()
    // the track constructor is also able to accept a NULL as well
    // and the track implements IAsn1Generator interface
    virtual bool SupportsExportToAsn1() const { return true; }

    virtual bool NeedBackgroundInit() const
    { return true; }

    virtual const CTrackTypeInfo&  GetThisTypeInfo() const
    { return GetTypeInfo(); }

    static const CTrackTypeInfo& GetTypeInfo()
    { return CFeatureTrack::m_TypeInfo; }

    /// @name ITrackConfigurable interface implementation
    /// @{
    virtual CRef<objects::CTrackConfigSet>
        GetSettings(const string& profile,
                    const TKeyValuePairs& settings,
                    const CTempTrackProxy* track_proxy) const;
    /// @}

    /// @name IExtension interface implementation
    /// @{
    virtual string GetExtensionIdentifier() const;
    virtual string GetExtensionLabel() const;
    /// @}
protected:
    CFeatureTrack::ELayout  m_DefaultLayout;
    ELinkedFeatDisplay      m_DefaultLinkedFeat;
};

//
//class CFeatureTrackConfig : public CObject
//{
//public:
//    CFeatureTrackConfig() {}
//
//    void LoadSettings(CConstRef<CSeqGraphicConfig> g_conf,
//        const string& reg_path);
//    void SaveSettings(CConstRef<CSeqGraphicConfig> g_conf,
//        const string& reg_path) const;
//};

///////////////////////////////////////////////////////////////////////////////
///   CFeatureTrack inline methods
///
inline /*virtual*/
CRef<CSGGenBankDS> CFeatureTrack::GetDataSource()
{
    return CRef<CSGGenBankDS>(m_DS.GetPointer());
}

inline /*virtual*/
CConstRef<CSGGenBankDS> CFeatureTrack::GetDataSource() const
{
    return CConstRef<CSGGenBankDS>(m_DS.GetPointer());
}

inline
void CFeatureTrack::SetLayout(ELayout layout)
{
    m_Layout = layout;
}

inline
int CFeatureTrack::GetFeatSubtype() const
{
    return m_Subtype;
}

inline
bool CFeatureTrack::x_HasLinkedFeats() const
{
    return m_HasLinkedFeats;
}


END_NCBI_SCOPE

#endif  // GUI_WIDGETS_SEQ_GRAPHIC___FEATURE_TRACK__HPP
