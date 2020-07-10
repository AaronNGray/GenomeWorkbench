#ifndef GUI_WIDGETS_SEQ_GRAPHIC___SEGMENT_MAP_TRACK__HPP
#define GUI_WIDGETS_SEQ_GRAPHIC___SEGMENT_MAP_TRACK__HPP

/*  $Id: segment_map_track.hpp 33142 2015-06-03 19:06:10Z joukovv $
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
#include <objmgr/annot_selector.hpp>
#include <objects/seqloc/Seq_interval.hpp>
#include <gui/utils/extension.hpp>

#include <gui/widgets/seq_graphic/data_track.hpp>
#include <gui/widgets/seq_graphic/segment_map_ds.hpp>
#include <gui/widgets/seq_graphic/segment_glyph.hpp>




BEGIN_NCBI_SCOPE

class CSGJobResult;

///////////////////////////////////////////////////////////////////////////////
///   CSegmentMapTrack -- 
///
class NCBI_GUIWIDGETS_SEQGRAPHIC_EXPORT CSegmentMapTrack :
    public CDataTrack
{
    friend class CSegmentMapTrackFactory;

public:
    typedef CSGSegmentMapDS::ESegmentLevel TSegmentLevel;

    enum EJobType {
        eSegmentMap,
        eSegmentMapSeqID
    };

    /// @name static methods.
    /// static methods for converting level str and label position str
    /// level setting and label position setting.
    /// @{
    static TSegmentLevel LevelStrToValue(const string& level);
    static const string& LevelValueToStr(TSegmentLevel level);
    static CSegmentConfig::ELabelPosition LabelPosStrToValue(const string& pos);
    static const string& LabelPosValueToStr(CSegmentConfig::ELabelPosition pos);    
    /// @}

    CSegmentMapTrack(CSGSegmentMapDS* ds, CRenderingContext* r_cntx);
    virtual ~CSegmentMapTrack();

    /// @name CLayoutTrack public methods.
    /// @{
    virtual const CTrackTypeInfo&  GetTypeInfo() const;
    virtual string GetFullTitle() const;
    /// @}

    virtual CRef<CSGGenBankDS>  GetDataSource();
    virtual CConstRef<CSGGenBankDS>  GetDataSource() const;

protected:
    /// @name CLayoutTrack virtual methods overloading.
    /// @{
    virtual void x_LoadSettings(const string& preset_style,
                                const TKeyValuePairs& settings);

    /// save part of settings to a profile string.
    /// Only the potential settings changed by users will be saved.
    virtual void x_SaveSettings(const string& preset_style);
    /// @}

    /// @name CDataTrack pure virtual interfaces
    /// @{
    virtual void x_UpdateData();
    virtual void x_OnJobCompleted(CAppJobNotification& notify);
    /// @}

    void x_AddSegmentMapLayout(const CSGJobResult& result);

    bool x_NeedHighlight(const CSegmentGlyph* seg) const;

private:
    /// @name prohibited copy constructor and assignment operator.
    /// @{
    CSegmentMapTrack(const CSegmentMapTrack&);
    CSegmentMapTrack& operator=(const CSegmentMapTrack&);
    /// @}

    void x_SaveConfiguration(const string& preset_style) const;


private:
    CRef<CSGSegmentMapDS>   m_DS;
    CRef<CSegmentConfig>    m_Config;

    bool            m_ResolveSequences;
    bool            m_UseCutoff;

    static CTrackTypeInfo   m_TypeInfo;
};

///////////////////////////////////////////////////////////////////////////////
///   CSegmentMapTrackFactory
///
class NCBI_GUIWIDGETS_SEQGRAPHIC_EXPORT CSegmentMapTrackFactory :
    public CObject,
    public ILayoutTrackFactory,
    public ITrackConfigurable,
    public IExtension
{
public:
    CSegmentMapTrackFactory(){}

    /// create a layout track based on the input objects and extra parameters.
    virtual TTrackMap CreateTracks(
        SConstScopedObject& object,
        ISGDataSourceContext* ds_context,
        CRenderingContext* r_cntx,
        const SExtraParams& params = SExtraParams(),
        const TAnnotMetaDataList& src_annots = TAnnotMetaDataList()) const;

    virtual bool UnderstandLevel() const
    { return true; }

    virtual bool NeedBackgroundInit() const
    { return true; }

    virtual const CTrackTypeInfo&  GetThisTypeInfo() const
    { return GetTypeInfo(); }

    static const CTrackTypeInfo& GetTypeInfo()
    { return CSegmentMapTrack::m_TypeInfo; }

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
    bool x_IsChromosome(CSGSegmentMapDS& ds) const;
};


///////////////////////////////////////////////////////////////////////////////
///   CSegmentMapTrack inline methods
///
inline /*virtual*/
CRef<CSGGenBankDS> CSegmentMapTrack::GetDataSource()
{
    return CRef<CSGGenBankDS>(m_DS.GetPointer());
}

inline /*virtual*/
CConstRef<CSGGenBankDS> CSegmentMapTrack::GetDataSource() const
{
    return CConstRef<CSGGenBankDS>(m_DS.GetPointer());
}

END_NCBI_SCOPE

#endif  // GUI_WIDGETS_SEQ_GRAPHIC___SEGMENT_MAP_TRACK__HPP
