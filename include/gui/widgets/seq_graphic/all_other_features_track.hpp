#ifndef GUI_WIDGETS_SEQ_GRAPHIC___ALL_OTHER_FEATURES_TRACK__HPP
#define GUI_WIDGETS_SEQ_GRAPHIC___ALL_OTHER_FEATURES_TRACK__HPP

/* $Id: all_other_features_track.hpp 42431 2019-02-22 16:04:19Z katargir $
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
 * Author:  Liangshou Wu
 *
 */

 /**
 * File Description:
 */


#include <gui/widgets/seq_graphic/track_container_track.hpp>

BEGIN_NCBI_SCOPE

///////////////////////////////////////////////////////////////////////////////
/// CAllOtherFeaturesTrack - a track container in the form of layout track.
/// Compared to CGlyphContainer, this class assume its children must be one 
/// type of CLayoutTracks.  So it has knowledge about the child tracks
/// to turn them on or off. The purpose of CAllOtherFeaturesTrack is that it forms a
/// group of tracks to make them behave like a single track: movable,
/// collapsible, and closible. The track container may be resposible of
/// creating/initializing the contained tracks. The reason of inhereting
/// from CDataTrack is because a container track may need to initialize 
/// its child tracks asynchronously. Potentially, a container track 
/// may delegate its child tracks to load data for performance purpose.

class NCBI_GUIWIDGETS_SEQGRAPHIC_EXPORT CAllOtherFeaturesTrack
    : public CTrackContainer
{
    friend class CAllOtherFeaturesTrackFactory;

public:
    typedef CSeqGraphicConfig::TFeatSubtype TFeatSubtype;
    typedef list<TFeatSubtype>  TSubtypeList;

    CAllOtherFeaturesTrack(CRenderingContext* r_cntx, CFeaturePanelDS* ds, const set<string>& usedFeatureSubkeys);
    virtual ~CAllOtherFeaturesTrack();

    /// override method in CTrackContainer.
    virtual bool NoSubtrackEver() const;

    /// @name CLayoutTrack public methods.
    /// @{
    virtual const CTrackTypeInfo&  GetTypeInfo() const;
    virtual string GetFullTitle() const;
    /// @}

    void SetProxy(CTrackProxy* proxy);

    virtual void ReinitNASubtracks(TAnnotMetaDataList& annots, bool makeContainersVisible);

protected:
    virtual void x_OnAllJobsFinished();
    virtual void x_ReinitSubtracks();

private:
    /// prohibited copy constructor and assignment operator.
    CAllOtherFeaturesTrack(const CAllOtherFeaturesTrack&);
    CAllOtherFeaturesTrack& operator=(const CAllOtherFeaturesTrack&);

    void x_ReinitFeatureTrack(CTempTrackProxy* t_proxy);

private:
    /// track proxy for all other features track.
    CWeakRef<CTrackProxy>  m_ThisProxy;
    set<string> m_UsedFeatureSubkeys;

    /// CLayoutTrack required track type info.
    static CTrackTypeInfo m_TypeInfo;
};

///////////////////////////////////////////////////////////////////////////////
///   CAllOtherFeaturesTrackFactory
///
class NCBI_GUIWIDGETS_SEQGRAPHIC_EXPORT CAllOtherFeaturesTrackFactory :
    public CObject,
    public ILayoutTrackFactory,
    public ITrackConfigurable,
    public IExtension
{
public:
    CAllOtherFeaturesTrackFactory(){}

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
    { return CAllOtherFeaturesTrack::m_TypeInfo; }

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


///////////////////////////////////////////////////////////////////////////////
///   CAllOtherFeaturesTrack inline methods

inline
void CAllOtherFeaturesTrack::SetProxy(CTrackProxy* proxy)
{ m_ThisProxy.Reset(proxy); }


END_NCBI_SCOPE

#endif // GUI_WIDGETS_SEQ_GRAPHIC___ALL_OTHER_FEATURES_TRACK__HPP
