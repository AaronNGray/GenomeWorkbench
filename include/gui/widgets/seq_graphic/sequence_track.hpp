#ifndef GUI_WIDGETS_SEQ_GRAPHIC___SEQUENCE_TRACK__HPP
#define GUI_WIDGETS_SEQ_GRAPHIC___SEQUENCE_TRACK__HPP

/*  $Id: sequence_track.hpp 44647 2020-02-13 21:28:46Z evgeniev $
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
#include <gui/widgets/seq_graphic/segment_smear_glyph.hpp>
#include <gui/widgets/seq_graphic/segment_map_ds.hpp>
#include <gui/widgets/seq_graphic/restriction_sites_glyph.hpp>

#include <objects/seqloc/Seq_interval.hpp>
#include <objmgr/seq_vector.hpp>

BEGIN_NCBI_SCOPE

class CSGJobResult;

///////////////////////////////////////////////////////////////////////////////
///   CSequenceTrack -- the layout track for holding and visualizing a
///   molecule sequence superimposed by component map.
///
class NCBI_GUIWIDGETS_SEQGRAPHIC_EXPORT CSequenceTrack :
    public CDataTrack,
    public IObjectBasedGlyph
{
    friend class CSequenceTrackFactory;

public:
    CSequenceTrack(CSGSequenceDS* seq_ds, CRenderingContext* r_cntx,
        CSGSegmentMapDS* seg_map_ds = NULL);

    const objects::CSeqVector& GetSeqVector() const;

    /// @name CSeqGlyph virtual method implementation
    /// @{
    virtual CRef<CSeqGlyph> HitTest(const TModelPoint& p);
    virtual bool OnLeftDblClick(const TModelPoint& p);
    virtual bool NeedTooltip(const TModelPoint& p, ITooltipFormatter& tt, string& t_title) const;
    virtual void GetTooltip(const TModelPoint& p, ITooltipFormatter& tt, string& t_title) const;
    /// @}

    /// @name CLayoutTrack public virtual methods.
    /// @{
    virtual CHTMLActiveArea* InitHTMLActiveArea(TAreaVector* p_areas) const;
    virtual const CTrackTypeInfo&  GetTypeInfo() const;
    /// @}

    /// @name CDataTrack interface implementation
    /// @{
    virtual CRef<CSGGenBankDS>  GetDataSource();
    virtual CConstRef<CSGGenBankDS>  GetDataSource() const;
    /// @}

    /// @name IObjectBasedGlyph interface implement
    /// For sticky tooltip purpose
    /// @{
    virtual const objects::CSeq_loc& GetLocation(void) const;
    virtual CConstRef<CObject> GetObject(TSeqPos pos) const;
    virtual void  GetObjects(vector<CConstRef<CObject> >& objs) const;
    virtual bool HasObject(CConstRef<CObject> obj) const;
    virtual string GetSignature() const;
    virtual const IObjectBasedGlyph::TIntervals& GetIntervals(void) const;
    /// @}


protected:
    virtual void x_UpdateBoundingBox();

    /// @name CLayoutTrack virtual methods.
    /// @{
    virtual void x_RenderContent() const;
    virtual bool x_Empty() const;

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

private:
    /// @name prohibited copy constructor and assignment operator.
    /// @{
    CSequenceTrack(const CSequenceTrack&);
    CSequenceTrack& operator=(const CSequenceTrack&);
    /// @}

    /// save all track settings to the configuration file.
    /// this is not used currently.
    void x_SaveConfiguration(const string& preset_style) const;

    /// Render a sequence bar and the sequence if fits.
    void x_RenderSequence(const TModelRect& rcm,
        bool seq_fit, bool direct, bool show_strand) const;

    void x_RenderGaps(const TModelRect& rcm, TSeqPos from, TSeqPos to, bool show3d) const;

    /// Get the sequence bar height.
    TModelUnit x_GetBarHeight() const;

    /// Get the height of the gaps bar, drown on top of the segment map
    TModelUnit x_GetGapsBarHeight() const;

    bool x_ShowSegMap() const;
    bool x_HasVisibleRsite() const;

private:
    CRef<CSGSequenceDS>     m_SeqDS;
    CRef<CSGSegmentMapDS>   m_SegMapDS;
    CRef<objects::CSeq_loc> m_Location;

    /// list of restriction sites
    typedef vector< CRef<CRsitesGlyph> > TRsites;
    TRsites m_Rsites;

    /// @name track settings
    /// @{
    bool                m_ShowLabel;    ///< show sequence label.
    bool                m_ColorGapsKey; // color gaps by type
    bool                m_ShowSegMap;  ///< superimpose segment map.
    bool m_SegMapJobCompleted;
    CRef<CGlTextureFont> m_SeqFont;      ///< font
    CRgbaColor          m_SeqColor;     ///< sequence text color.
    CRgbaColor          m_SeqBarColor;  ///< sequence bar color.
    CRgbaColor          m_StrandColor;  ///< sequence strand color.
    /// color codes for segment map.
    CSegmentSmearGlyph::TColorCode  m_SegMapColors;
    /// @}

    static CTrackTypeInfo m_TypeInfo;
};

///////////////////////////////////////////////////////////////////////////////
///   CSequenceTrackFactory
///
class NCBI_GUIWIDGETS_SEQGRAPHIC_EXPORT CSequenceTrackFactory :
    public CObject,
    public ILayoutTrackFactory,
    public ITrackConfigurable,
    public IExtension
{
public:
    /// create a layout track based on input object and extra parameters.
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
    { return false; }

    virtual bool NeedBackgroundInit() const
    { return true; }

    virtual const CTrackTypeInfo&  GetThisTypeInfo() const
    { return GetTypeInfo(); }

    static const CTrackTypeInfo&  GetTypeInfo()
    { return CSequenceTrack::m_TypeInfo; }

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
};


END_NCBI_SCOPE


#endif  // GUI_WIDGETS_SEQ_GRAPHIC___SEQUENCE_TRACK__HPP
