#ifndef GUI_WIDGETS_SEQ_GRAPHIC___GENE_MODEL_TRACK__HPP
#define GUI_WIDGETS_SEQ_GRAPHIC___GENE_MODEL_TRACK__HPP

/*  $Id: gene_model_track.hpp 44952 2020-04-25 01:20:25Z rudnev $
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
#include <gui/widgets/seq_graphic/feature_ds.hpp>
#include <gui/widgets/seq_graphic/gene_model_config.hpp>
#include <gui/widgets/seq_graphic/compact_layout_policy.hpp>
#include <gui/widgets/seq_graphic/ifeature_track_base.hpp>

#include <objects/seqfeat/Seq_feat.hpp>
#include <objects/seqloc/Seq_interval.hpp>
#include <objmgr/seq_vector.hpp>
#include <objmgr/annot_selector.hpp>


class wxEvtHandler;

BEGIN_NCBI_SCOPE

///////////////////////////////////////////////////////////////////////////////
///   CGeneModelTrack -- 
///

class NCBI_GUIWIDGETS_SEQGRAPHIC_EXPORT CGeneModelTrack :
    public CDataTrack, public IAsn1Generator, public IFeatureTrackBase
{
    friend class CGeneModelFactory;

public:
    enum EJobType {
        eJob_LandMarkGenes,
        eJob_MainFeats,
        eJob_GeneProducts,
        eJob_CDSProduct_Feats,
        eJob_FeatProducts
    };

    CGeneModelTrack(CSGFeatureDS* ds, CRenderingContext* r_cntx);
    virtual ~CGeneModelTrack();

    /// @name CLayoutTrack public methods.
    /// @{
    virtual const CTrackTypeInfo&  GetTypeInfo() const;
    virtual string GetFullTitle() const;
    /// @}

    /// Method for getting data source.
    virtual CRef<CSGGenBankDS>  GetDataSource();
    virtual CConstRef<CSGGenBankDS>  GetDataSource() const;

    //void SetConfig(CGeneModelConfig* config);
    void SetAnnot(const string& annot);

    void SetFilter(const string& filter);

    void OnToggleContent(int id);

    // implement IAsn1Generator method
    void GenerateAsn1(objects::CSeq_entry& seq_entry, TSeqRange range);

    // if true, this track should be handled by stage 2
    virtual bool BypassStage1() { return false; }
    
    // estimate number of features in the specified range
    // returns true if range is acceptable for downloads
    // if false, sMsg will contain a message that can be shown to the user
    virtual bool CheckRange(TSeqRange range, string&) { return true; }

    virtual string GetAnnot() const {return m_RealAnnot;}
    virtual void GetFeatSubtypes(set<objects::CSeqFeatData::ESubtype> &subtypes) const;
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

private:
    /// @name prohibited copy constructor and assignment operator.
    /// @{
    CGeneModelTrack(const CGeneModelTrack&);
    CGeneModelTrack& operator=(const CGeneModelTrack&);
    /// @}

    void x_AdjustSelector();

    void x_OnContentIconClicked();
    void x_OnLayoutIconClicked();

    void x_AddLandMarkFeaturesLayout(CSGJobResult& result);
    void x_AddMainFeaturesLayout(const CSGJobResult& result);
    void x_AddCDSProduct_FeatsLayout(CSGJobResult& result);
    void x_AddFeatProductLayout(CBatchJobResult& results);

    bool x_NeedHighlight(const CFeatGlyph &feat_glyph) const;

    typedef map< objects::CSeqFeatData::ESubtype,
        CConstRef<CFeatureParams> > TFeatConfigMap;

    bool x_InitFeature_Recursive(CLayoutGroup::TObjectList& objs,
        TFeatConfigMap& feat_confs, const CFeatGlyph* parent_gene,
        TFeatBatchJobRequests& requests, bool compact_mode);

    /// save all track settings to the configuration file.
    /// this is not used currently.
    void  x_SaveConfiguration() const;

    //void x_AddFeatProductsLayout(const CSeqJobResult& result);
    //void x_AddGeneHistogramLayout(CSeqJobResult& result);
    //void x_AddFeatCDSmRNA_SNPLayout(CSeqJobResult& result);
    //void x_AddFeatCDSProduct_FeatsLayout(CSeqJobResult& result);

private:
    /// wx-related event handler.
    wxEvtHandler*           m_ContentHandler;

    CRef<CGeneModelConfig>  m_Conf;
    string                  m_AnnotName;
    string                  m_RealAnnot;

    CRef<CSGFeatureDS>      m_DS;
    objects::SAnnotSelector m_LandMarkFeatSel;
    objects::SAnnotSelector m_MainSel;
    objects::SAnnotSelector m_cdsFeatSel;

    CRef<CCompactLayout>    m_Compact;

    /// flag for indicating if we have used a more meaningful annotation name.
    /// This needs to be done only once.
    bool                    m_AnnotNameUpdated;

    static CTrackTypeInfo   m_TypeInfo;
    set<objects::CSeqFeatData::ESubtype> m_subtypes;
};

///////////////////////////////////////////////////////////////////////////////
///   CGeneModelFactory
///
class NCBI_GUIWIDGETS_SEQGRAPHIC_EXPORT CGeneModelFactory :
    public CObject,
    public ILayoutTrackFactory,
    public ITrackConfigurable,
    public IExtension
{
public:
    CGeneModelFactory(){}

    /// create a layout track based on the input object and extra parameters.
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
    { return CGeneModelTrack::m_TypeInfo; }


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

private:
    TTrackMap x_CreateTracksWithFilters(SConstScopedObject& object,
        ISGDataSourceContext* ds_context, CRenderingContext* r_cntx,
        const SExtraParams& params, const TAnnotNameTitleMap& annots) const;
};

///////////////////////////////////////////////////////////////////////////////
///   CGeneModelTrack inline methods
///
inline /*virtual*/
CRef<CSGGenBankDS> CGeneModelTrack::GetDataSource()
{
    return CRef<CSGGenBankDS>(m_DS.GetPointer());
}

inline /*virtual*/
CConstRef<CSGGenBankDS> CGeneModelTrack::GetDataSource() const
{
    return CConstRef<CSGGenBankDS>(m_DS.GetPointer());
}

inline
void CGeneModelTrack::SetFilter(const string& filter)
{
    if (m_DS) m_DS->SetFilter(filter);
}

END_NCBI_SCOPE

#endif  // GUI_WIDGETS_SEQ_GRAPHIC___GENE_MODEL_TRACK__HPP
