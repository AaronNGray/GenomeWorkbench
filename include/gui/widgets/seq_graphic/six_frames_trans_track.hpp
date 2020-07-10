#ifndef GUI_WIDGETS_SEQ_GRAPHIC___SIXFRAMES_TRANSLATION_TRACK__HPP
#define GUI_WIDGETS_SEQ_GRAPHIC___SIXFRAMES_TRANSLATION_TRACK__HPP

/*  $Id: six_frames_trans_track.hpp 43682 2019-08-14 16:00:25Z katargir $
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
#include <gui/widgets/seq_graphic/seqgraphic_genbank_ds.hpp>
#include <gui/widgets/seq_graphic/seqgraphic_job_result.hpp>
//#include <objects\seqloc\Na_strand.hpp>

BEGIN_NCBI_SCOPE

class CTranslationConfig;

///////////////////////////////////////////////////////////////////////////////
/// CSFTransDS - Six-frames translation data source
///
class NCBI_GUIWIDGETS_SEQGRAPHIC_EXPORT CSFTransDS : public CSGGenBankDS
{
public:
    CSFTransDS(objects::CScope& scope, const objects::CSeq_id& id);

    /// do Six-frames translation.
    /// @param range is the sequence range within which we want to do
    ///        Six-frames translation.
    void DoTranslation(const TSeqRange& range, objects::ENa_strand strand = objects::eNa_strand_both);
    
    void SetGeneticCode(int id);
    int  GetGeneticCode() const;
    void SetAltStart(bool f) { m_AltStart = f; }
    bool GetAltStart() const { return m_AltStart; }

private:
    int  m_GeneticCode;
    bool m_AltStart;    ///< allow alternative start codon
};


///////////////////////////////////////////////////////////////////////////////
/// CSFTransDSType - Six-frames translation data source type
///
class NCBI_GUIWIDGETS_SEQGRAPHIC_EXPORT CSFTransDSType :
    public CObject,
    public ISGDataSourceType,
    public IExtension
{
public:
    /// create an instance of the layout track type using default settings.
    virtual ISGDataSource* CreateDS(SConstScopedObject& object) const;

    /// @name IExtension interface implementation
    /// @{
    virtual string GetExtensionIdentifier() const;
    virtual string GetExtensionLabel() const;
    /// @}

    /// check if the data source can be shared.
    virtual bool IsSharable() const;
};


///////////////////////////////////////////////////////////////////////////////
///   CSixFramesTransTrack -- A special track for doing six-frames translation
///   of the current sequence. Three frame shifts on one direction, and
///   another three frame shifts on the reverse direction.
///

class NCBI_GUIWIDGETS_SEQGRAPHIC_EXPORT CSixFramesTransTrack :
    public CDataTrack
{
    friend class CSixFramesTransTrackFactory;

public:
    enum ETranslation {
        eTrans_Always,  ///< do translation for all resolution
        eTrans_Adaptive ///< do translation only for under certain zoom level
    };

    enum EStrandOption {
        eOpt_Forward = objects::eNa_strand_plus,  ///< show forward translations
        eOpt_Reverse = objects::eNa_strand_minus, ///< show reverse translations
        eOpt_All = objects::eNa_strand_both,       ///< show translation for both directions
        eOpt_LeftToRight = objects::eNa_strand_other ///< Choose what to show based on orientation 
    };

    static ETranslation TransStrToValue(const string& trans);
    static const string& TransValueToStr(ETranslation trans);
    static EStrandOption OptionStrToValue(const string& opt);
    static const string& OptionValueToStr(EStrandOption opt);

    CSixFramesTransTrack(CSFTransDS* ds, CRenderingContext* r_cntx);
    virtual ~CSixFramesTransTrack();


    /// @name CLayoutTrack public methods.
    /// @{
    virtual const CTrackTypeInfo&  GetTypeInfo() const override;
    virtual string GetFullTitle() const override;
    /// @}

    virtual CRef<CSGGenBankDS>  GetDataSource() override;
    virtual CConstRef<CSGGenBankDS>  GetDataSource() const override;

    void SetGCName(const string& gc_name);

protected:
    /// @name CLayoutTrack virtual methods overloading.
    /// @{
    virtual void x_LoadSettings(const string& preset_style,
                                const TKeyValuePairs& settings) override;

    /// save part of settings to a profile string.
    /// Only the potential settings changed by users will be saved.
    virtual void x_SaveSettings(const string& preset_style) override;
    virtual void x_OnIconClicked(TIconID id) override;
    /// @}

    /// @name CDataTrack pure virtual interfaces
    /// @{
    virtual void Update(bool layout_only) override;
    virtual void x_UpdateData() override;
    virtual void x_OnJobCompleted(CAppJobNotification& notify) override;
    /// @}

    void x_AddTranslations(const CSGJobResult& result);

private:
    enum ESixFrameIconID {
        eIcon_Genetic = eIcon_MaxTrackIconID + 1
    };


    /// @name prohibited copy constructor and assignment operator.
    /// @{
    CSixFramesTransTrack(const CSixFramesTransTrack&);
    CSixFramesTransTrack& operator=(const CSixFramesTransTrack&);
    /// @}

    void x_OnContentIconClicked();
    void x_OnSettingIconClicked();
    void x_OnGeneticIconClicked();
    void x_UpdateTitle();
    void x_InitGeneticCodeName(int gc_id);
    TSeqRange x_GetCurrentTransRange() const;

private:
    CRef<CSFTransDS>      m_DS;
    ETranslation          m_Trans;
    EStrandOption         m_Option;

    /// genetic code name.
    string                m_GCName;

    /// the sequence range current translation is done.
    TSeqRange             m_TransRange;
    /// the sequence strand for the current translation
    objects::ENa_strand   m_TransStrand;
    CRef<CTranslationConfig> m_Config;
    CRef<CBoundaryParams> m_GroupConf;

    static CTrackTypeInfo   m_TypeInfo;
};

///////////////////////////////////////////////////////////////////////////////
///   CSixFramesTransTrackFactory
///
class NCBI_GUIWIDGETS_SEQGRAPHIC_EXPORT CSixFramesTransTrackFactory :
    public CObject,
    public ILayoutTrackFactory,
    public ITrackConfigurable,
    public IIconProvider,
    public IExtension
{
public:
    CSixFramesTransTrackFactory(){}

    /// create a layout track based on the input objects and extra parameters.
    virtual TTrackMap CreateTracks(
        SConstScopedObject& object,
        ISGDataSourceContext* ds_context,
        CRenderingContext* r_cntx,
        const SExtraParams& params = SExtraParams(),
        const TAnnotMetaDataList& src_annots = TAnnotMetaDataList()) const;

    virtual CRef<objects::CTrackConfigSet>
        GetSettings(const string& profile,
                    const TKeyValuePairs& settings,
                    const CTempTrackProxy* track_proxy) const;

    virtual bool UnderstandLevel() const
    { return false; }

    virtual bool NeedBackgroundInit() const
    { return true; }

    virtual const CTrackTypeInfo&  GetThisTypeInfo() const
    { return GetTypeInfo(); }

    static const CTrackTypeInfo& GetTypeInfo()
    { return CSixFramesTransTrack::m_TypeInfo; }

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

//
//class CSixFramesTransTrackConfig : public CObject
//{
//public:
//    CSixFramesTransTrackConfig() {}
//
//    void LoadSettings(CConstRef<CSeqGraphicConfig> g_conf,
//        const string& reg_path);
//    void SaveSettings(CConstRef<CSeqGraphicConfig> g_conf,
//        const string& reg_path) const;
//};

///////////////////////////////////////////////////////////////////////////////
///   CSFTransDS inline methods
inline
void CSFTransDS::SetGeneticCode(int id)
{ m_GeneticCode = id; }

inline
int  CSFTransDS::GetGeneticCode() const
{ return m_GeneticCode; }

///////////////////////////////////////////////////////////////////////////////
///   CSixFramesTransTrack inline methods
inline /*virtual*/
CRef<CSGGenBankDS> CSixFramesTransTrack::GetDataSource()
{
    return CRef<CSGGenBankDS>(m_DS.GetPointer());
}

inline /*virtual*/
CConstRef<CSGGenBankDS> CSixFramesTransTrack::GetDataSource() const
{
    return CConstRef<CSGGenBankDS>(m_DS.GetPointer());
}

inline
void CSixFramesTransTrack::SetGCName(const string& gc_name)
{
    m_GCName = gc_name;
}

END_NCBI_SCOPE

#endif  // GUI_WIDGETS_SEQ_GRAPHIC___SIXFRAMES_TRANSLATION_TRACK__HPP
