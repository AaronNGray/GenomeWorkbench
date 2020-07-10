#ifndef GUI_WIDGETS_SEQ_GRAPHIC___GRAPH_OVERLAY__HPP
#define GUI_WIDGETS_SEQ_GRAPHIC___GRAPH_OVERLAY__HPP

/* 
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
 * Author:  Andrei Shkeda
 *
 */

 /**
 * File Description:
 */


#include <gui/widgets/seq_graphic/data_track.hpp>
#include <gui/widgets/seq_graphic/feature_panel_ds.hpp>
#include <gui/widgets/seq_graphic/track_config_manager.hpp>
#include <gui/widgets/seq_graphic/track_container_track.hpp>
#include <gui/widgets/seq_graphic/legend_item_glyph.hpp>
#include <gui/widgets/seq_graphic/histogram_glyph.hpp>


class wxEvtHandler;

BEGIN_NCBI_SCOPE

///////////////////////////////////////////////////////////////////////////////
/// CGraphOverlay container of the graph_tracks that are dispalyed in overlay mode

class NCBI_GUIWIDGETS_SEQGRAPHIC_EXPORT CGraphOverlay :
    public CTrackContainer,
    public ILegendHost,
    public IDroppable
{
    friend class CGraphOverlayFactory;

public:
    enum ELayout {
        eOverlay,
        eStacked
    };
    typedef float TDataType;
    CGraphOverlay(CRenderingContext* r_cntx, CFeaturePanelDS* ds = NULL, const string& source = NcbiEmptyString);

    virtual void OnTrackSettings(int order);
    virtual void GetHTMLActiveAreas(TAreaVector* p_areas) const;
    /// Close a track.
    /// Hide the given track, but track is not removed.
    //virtual void CloseTrack(int order);
    //virtual void x_ShowTrack(CLayoutTrack* track, bool on);
    
    void UpdateTrackParams(const string& annot_name);

    /// @name TrackContainer public methods.
    /// @{
    virtual const CTrackTypeInfo&  GetTypeInfo() const;
    /// @}

    virtual CRef<CSeqGlyph> HitTest(const TModelPoint& p);

    /// @name data ILegendHost pure virtual methods.
    /// @{
    virtual void LH_OnItemDblClick(const string& source_name);
    virtual void LH_OnItemRightClick(const string& source_name);
    /// @}

    virtual bool IsOverlay() const;

    /// @name IDroppable virtual methods overloading.
    /// @{
    virtual bool CanDrop(CRef<CLayoutTrack>& track);
    virtual bool Drop(CRef<CLayoutTrack>& track);
    /// @}
    
    void UpdateSource(CTempTrackProxy* self_proxy = 0);
    void MoveTrackOut(int track_id);

protected:

    virtual void x_LoadSettings(const string& /*preset_style*/,
                        const TKeyValuePairs& settings);
    virtual void x_SaveSettings(const string& preset_style);

    void x_UpdateContainerParams(CHistParams& params);

    virtual void x_RenderContent() const;
    virtual void x_UpdateBoundingBox();
    virtual void x_OnLayoutChanged();
    virtual void x_OnAllJobsFinished();

    void x_UpdateTrackSettings(const string& source_name);

    /// The unique name - combination of subtracks annots - 
    /// is used to access the track setting in registry
    string m_Source;

    void x_UpdateLegend();
private:
    CLayoutGroup m_Legend;
    CRef<CHistogramGlyph> m_Grid;
    ELayout m_Layout = eOverlay;
    CRef<COverlayLayout> m_OverlayLayout;
    CRef<CSimpleLayout>  m_StackedLayout;

    /// CLayoutTrack required track type info.
    static CTrackTypeInfo m_TypeInfo;
};

///////////////////////////////////////////////////////////////////////////////
///   CTrackContainerFactory
///
class NCBI_GUIWIDGETS_SEQGRAPHIC_EXPORT CGraphOverlayFactory :
    public CObject,
    public ILayoutTrackFactory,
    public ITrackConfigurable,
    public IExtension
{
public:
    CGraphOverlayFactory();

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
    { return false; }

    virtual const CTrackTypeInfo&  GetThisTypeInfo() const
    { return GetTypeInfo(); }

    static const CTrackTypeInfo& GetTypeInfo()
    { return CGraphOverlay::m_TypeInfo; }

    /// @name ITrackConfigurable public methods.
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


inline
bool CGraphOverlay::IsOverlay() const
{
    return true;
}


END_NCBI_SCOPE

#endif // GUI_WIDGETS_SEQ_GRAPHIC___GRAPH_OVERLAY__HPP
