#ifndef GUI_WIDGETS_SEQ_GRAPHIC___GRAPH_TRACK__HPP
#define GUI_WIDGETS_SEQ_GRAPHIC___GRAPH_TRACK__HPP

/*  $Id: graph_track.hpp 37640 2017-01-31 18:24:42Z shkeda $
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
#include <gui/widgets/seq_graphic/data_track.hpp>
#include <gui/widgets/seq_graphic/seqgraphic_graph_ds.hpp>
#include <objmgr/annot_selector.hpp>
#include <connect/services/neticache_client.hpp>


BEGIN_NCBI_SCOPE

//class CSGGraphDS;

///////////////////////////////////////////////////////////////////////////////
///   CGraphTrack -- 
///

class NCBI_GUIWIDGETS_SEQGRAPHIC_EXPORT CGraphTrack :
    public CDataTrack,
    public ILegendProvider,
    public IDroppable
{
    friend class CGraphTrackFactory;

public:
    enum ELayout
    {
        eLayout_Layered,  // typical mode: all children histograms are stacked 
        eLayout_Overlay,  // Histograms are overlayed
        eLayout_Overlayed // Histograms are overlayed and the whole track is a part of graph overlay
    };

    static const string kOverlayed;

    CGraphTrack(CSGGraphDS* ds, CRenderingContext* r_cntx);
    virtual ~CGraphTrack();

    /// @name CLayoutTrack public methods.
    /// @{
    virtual const CTrackTypeInfo&  GetTypeInfo() const;
    /// @}


    /// @name ILegendProvide methods.
    /// @{
    virtual void GetLegend(const CTempTrackProxy* track_proxy, objects::CTrackConfig::TLegend& legend) const;
    /// @}

    virtual CRef<CSGGenBankDS>  GetDataSource();
    virtual CConstRef<CSGGenBankDS>  GetDataSource() const;

    virtual void GetHTMLActiveAreas(TAreaVector* p_areas) const;
    virtual CHTMLActiveArea* InitHTMLActiveArea(TAreaVector* p_areas) const;

    void SetAnnot(const string& annot);
    const string& GetAnnot() const;

    void SetLayout(ELayout layout);

    /// @name IDroppable virtual methods overloading.
    /// @{
    virtual bool CanDrop(CRef<CLayoutTrack>& track);
    virtual bool Drop(CRef<CLayoutTrack>& track);
    /// @}

protected:
    /// @name CLayoutTrack virtual methods overloading.
    /// @{
    virtual void x_LoadSettings(const string& preset_style,
                                const TKeyValuePairs& settings);

    /// save part of settings to a profile string.
    /// Only the potential settings changed by users will be saved.
    virtual void x_SaveSettings(const string& preset_style);

    virtual void x_OnIconClicked(TIconID id);

    virtual void x_OnLayoutChanged();

    /// @}

    /// @name CDataTrack pure virtual interfaces
    /// @{
    virtual void x_UpdateData();
    virtual void x_OnJobCompleted(CAppJobNotification& notify);

    /// @}

    /// @name CGlyphContainerpure virtual interfaces
    /// @{
    virtual void x_UpdateBoundingBox();
    virtual void x_RenderContent() const;
    /// @}


    objects::SAnnotSelector& x_GetAnnotSelector();
    CSGGraphDS& x_GetGraphDS();

    bool CreateOverlay(vector<CRef<CGraphTrack>>& tracks);

    static CNetICacheClient s_InitICache();
    static CNetICacheClient& s_GetICacheInstance();

private:
    /// @name prohibited copy constructor and assignment operator.
    /// @{
    CGraphTrack(const CGraphTrack&);
    CGraphTrack& operator=(const CGraphTrack&);
    /// @}

    void x_OnSettingsIconClicked();
    void x_InitGrid();
    void x_CollectAllGraphs(vector<CRef<CHistogramGlyph>>& graphs) const;

private:
    string                  m_AnnotName;
    CRef<CSGGraphDS>        m_DS;
    objects::SAnnotSelector m_Sel;

    /// Dynamic or fixed graph scale.
    /// By default, the graph scale is a dynamic value which
    /// is updated within the visible range.
    bool                    m_FixedScale;
    /// Graph y limits stored when FixedScale is set 
    CSGGraphDS::TAxisLimits m_YLimits;

    static CTrackTypeInfo   m_TypeInfo;

    CRef<CHistogramGlyph> m_Grid;
    static ELayout x_LayoutStrToValue(const string& layout);
    static string x_LayoutValueToStr(ELayout layout);

    ELayout m_Layout;
};

///////////////////////////////////////////////////////////////////////////////
///   CGraphTrackFactory
///
class NCBI_GUIWIDGETS_SEQGRAPHIC_EXPORT CGraphTrackFactory :
    public CObject,
    public ILayoutTrackFactory,
    public ITrackConfigurable,
    public IExtension
{
public:
    CGraphTrackFactory(){}

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
    { return CGraphTrack::m_TypeInfo; }

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

inline
const string& CGraphTrack::GetAnnot() const
{
    return m_AnnotName;
}

inline
objects::SAnnotSelector& CGraphTrack::x_GetAnnotSelector()
{
    return m_Sel;
}


inline CSGGraphDS& CGraphTrack::x_GetGraphDS()
{
    return *m_DS;
}


END_NCBI_SCOPE

#endif  // GUI_WIDGETS_SEQ_GRAPHIC___GRAPH_TRACK__HPP
