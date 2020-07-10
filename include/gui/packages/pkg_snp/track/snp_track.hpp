#ifndef GUI_PACKAGES_SNP_TRACK__SNP_TRACK__HPP
#define GUI_PACKAGES_SNP_TRACK__SNP_TRACK__HPP

/*  $Id: snp_track.hpp 44952 2020-04-25 01:20:25Z rudnev $
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
 * Authors:  Melvin Quintos, Dmitry Rudnev
 *
 * File Description:
 *    This file contains class declarations for CSnpTrack and CSnpTrackFactory
 *
 */

#include <corelib/ncbiobj.hpp>
#include <gui/utils/extension.hpp>
#include <gui/widgets/seq_graphic/ifeature_track_base.hpp>
#include <gui/widgets/seq_graphic/data_track.hpp>
#include <gui/widgets/seq_graphic/seqgraphic_job_result.hpp>
#include <gui/packages/pkg_snp/track/snp_ds.hpp>
#include <gui/widgets/snp/filter/snp_filter.hpp>
#include <gui/utils/rgba_color.hpp>

#include <objects/seqloc/Seq_interval.hpp>
#include <objmgr/annot_selector.hpp>
#include <objects/seqfeat/Seq_feat.hpp>
#include <objmgr/graph_ci.hpp>

BEGIN_NCBI_SCOPE

///////////////////////////////////////////////////////////////////////////////
//   CSnpTrack
///////////////////////////////////////////////////////////////////////////////

class CSNPFeatureParams;

class CSnpTrack :
    public CDataTrack, public IAsn1Generator, public IFeatureTrackBase
{
    friend class CSnpTrackFactory;

public:
    enum ELayout {
        eLayout_Adaptive = 260,  ///< All SNPs drawn in one line
        eLayout_Density,         ///< Always show density
        eLayout_Features,        ///< Show features whenever possible
        eLayout_Labels           ///< Show labels and cascade features (CLayeredLayoutPolicy)
    };

public:
    CSnpTrack(CSGSnpDS* ds, CRenderingContext* r_cntx, const string& sFilter, const string& sTitle, const string& sLayout = "");
    virtual ~CSnpTrack();

    /// @name CDataTrack reimplemented methods.
    /// @{
    virtual CRef<CSGGenBankDS>  GetDataSource();
    virtual CConstRef<CSGGenBankDS>  GetDataSource() const;
    /// @}

    /// @name CLayoutTrack reimplemented methods.
    /// @{
    virtual const CTrackTypeInfo&  GetTypeInfo() const;
    virtual string GetFullTitle() const;
    /// @}

    void SetAnnot(const string& name);

    /// @name IFeatureTrackBase reimplemented methods.
    /// @{
    virtual void GetFeatSubtypes(set<objects::CSeqFeatData::ESubtype> &subtypes) const {}
    virtual string GetAnnot() const {return m_AnnotName;}
    virtual bool IsSnpTrack() const {return true;}
    /// @}

    // implement IAsn1Generator method
    void GenerateAsn1(objects::CSeq_entry& seq_entry, TSeqRange range);

    // if true, this track should be handled by stage 2
    virtual bool BypassStage1() { return true; }
    
    // estimate number of features in the specified range
    // returns true if range is acceptable for downloads
    // if false, sMsg will contain a message that can be shown to the user
    virtual bool CheckRange(TSeqRange range, string& sMsg);


protected:
    virtual void x_OnIconClicked(TIconID id);

    virtual void x_SaveSettings(const string& preset_style);
    virtual void x_LoadSettings(const string& preset_style,
                                const TKeyValuePairs& settings);

    /// @name CDataTrack pure virtual interfaces
    /// @{
    virtual void x_UpdateData();
    virtual void x_OnJobCompleted(CAppJobNotification& notify);
    /// @}

    void x_ProcessJobResult();
    void x_AddSnpsLayout(const SSnpJobResult& result);
    void x_UpdateLayout();

private:
    /// @name prohibited copy constructor and assignment operator.
    /// @{
    CSnpTrack(const CSnpTrack&);
    CSnpTrack& operator=(const CSnpTrack&);
    /// @}

    bool x_IsOverviewMode() const;
    
    objects::CFeat_CI x_GetFeat_CI(const TSeqRange& range);
    objects::CGraph_CI x_GetGraph_CI(const TSeqRange& range);

private:

    int                     m_eLayout;
    string                  m_AnnotName;

    CRef<CSGSnpDS>          m_DS;
    CRef<CSNPFeatureParams> m_Params;

    SIconInfo               m_IconLayout;
    CRef<SSnpJobResult>     m_JobResult;
    CRef<SSnpFilter>        m_Filter;
    string                  m_PreferredTitle;

    static CTrackTypeInfo   m_TypeInfo;


};

///////////////////////////////////////////////////////////////////////////////
//   CSnpTrackFactory
///////////////////////////////////////////////////////////////////////////////
class NCBI_GUIWIDGETS_SNP_EXPORT CSnpTrackFactory :
    public CObject,
    public ILayoutTrackFactory,
    public ITrackConfigurable,
    public IExtension
{
public:
    CSnpTrackFactory(){}

    static const CTrackTypeInfo& GetTypeInfo()
        { return CSnpTrack::m_TypeInfo; }

    /// @name ILayoutTrackFactory interface implementation
    /// @{
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
    /// @}

    /// @name ITrackConfigurable reimplemented methods.
    /// @{
    CRef<objects::CTrackConfigSet>
    GetSettings(const string& profile,
                const TKeyValuePairs& settings,
                const CTempTrackProxy* track_proxy) const;
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
CRef<CSGGenBankDS> CSnpTrack::GetDataSource()
{
    return CRef<CSGGenBankDS>(m_DS.GetPointer());
}

inline /*virtual*/
CConstRef<CSGGenBankDS> CSnpTrack::GetDataSource() const
{
    return CConstRef<CSGGenBankDS>(m_DS.GetPointer());
}

class CSNPColorPicker
{
public:
	// check that the feature is a known SNP class, return a color that corresponds to it
	// if not a SNP feature or class is unknown, returns a default color
    static CRgbaColor GetSNPColor(const objects::CSeq_feat& feat, const CRegistryReadView& ColorView);
};


struct SSNPFeatParamDefiner
{
public:
	SSNPFeatParamDefiner(CSnpBitfield::EVariationClass VariationClass = CSnpBitfield::eUnknownVariation, CFeatureParams::EBoxStyle BoxStyle = CFeatureParams::eBox_Filled)
		: m_VariationClass(VariationClass), m_BoxStyle(BoxStyle) {}
	CSnpBitfield::EVariationClass m_VariationClass;
	CFeatureParams::EBoxStyle m_BoxStyle;
};

inline bool operator<(const SSNPFeatParamDefiner& left, const SSNPFeatParamDefiner& right)
{
    return (left.m_VariationClass != right.m_VariationClass)
			? ((int)left.m_VariationClass < (int)right.m_VariationClass)
            : ((int)left.m_BoxStyle < (int)right.m_BoxStyle);
}


// class that tries to create the least amount of diverse CRef<CFeatureParams>
class NCBI_GUIWIDGETS_SNP_EXPORT CSNPFeatureParams : public CObject
{
public:

    // create all future params based on the OrigParams
    static CRef<CSNPFeatureParams> CreateSNPFeatureParams();
    static CRef<CSNPFeatureParams> CreateSNPFeatureParams(CConstRef<CFeatureParams> OrigParams);
    void Reset(CConstRef<CFeatureParams> OrigParams);

    // access to the stored params template
    CRef<CFeatureParams> SetParamsTemplate() { return m_ParamsTemplate; }
    void UpdateLabelPos();

    // get parameters for a given SNP feature, they are based on m_ParamsTemplate, but modified according to
    // the feature contents
    CConstRef<CFeatureParams> GetFeatureParams(const objects::CSeq_feat& feat, const CRegistryReadView& ColorView) const;

private:
    typedef map<SSNPFeatParamDefiner, CConstRef<CFeatureParams> > TParamsCache;

    CSNPFeatureParams()
        : m_ParamsTemplate(new CFeatureParams()) {}
    CSNPFeatureParams(CConstRef<CFeatureParams> OrigParams)
        : m_ParamsTemplate(new CFeatureParams()) { *m_ParamsTemplate = *OrigParams; }

    CRef<CFeatureParams> m_ParamsTemplate;
    mutable TParamsCache m_ParamsCache;
};

END_NCBI_SCOPE

#endif  // GUI_PACKAGES_SNP_TRACK__SNP_TRACK__HPP
