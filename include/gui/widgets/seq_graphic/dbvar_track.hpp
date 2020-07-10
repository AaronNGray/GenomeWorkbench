#ifndef GUI_WIDGETS_SEQ_GRAPHIC___DBVAR_TRACK__HPP
#define GUI_WIDGETS_SEQ_GRAPHIC___DBVAR_TRACK__HPP

/*  $Id: dbvar_track.hpp 44299 2019-11-25 18:37:34Z filippov $
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

#include <gui/widgets/seq_graphic/feature_track.hpp>


BEGIN_NCBI_SCOPE

///////////////////////////////////////////////////////////////////////////////
///   CVarTrack -- 
///

class NCBI_GUIWIDGETS_SEQGRAPHIC_EXPORT CVarTrack :
    public CFeatureTrack
{
    friend class CVarTrackFactory;

public:
    CVarTrack(CSGFeatureDS* ds, CRenderingContext* r_cntx);
    virtual ~CVarTrack();

    /// @name CLayoutTrack public methods.
    /// @{
    virtual const CTrackTypeInfo&  GetTypeInfo() const override;
    virtual CHTMLActiveArea* InitHTMLActiveArea(TAreaVector* p_areas) const override;
    /// @}

    /// @name IFeatureTrackBase reimplemented methods.
    /// @{
    virtual bool IsVarTrack() const {return true;}
    /// @}
protected:
    /// @name CFeatureTrack method override
    /// @{
    virtual void x_UpdateData () override;
    virtual void x_LoadSettings(const string&, const TKeyValuePairs &settings) override;

    /// @}
    
private:
    /// @name prohibited copy constructor and assignment operator.
    /// @{
    CVarTrack(const CVarTrack&);
    CVarTrack& operator=(const CVarTrack&);
    /// @}

private:
    static CTrackTypeInfo   m_TypeInfo;
    bool m_ZoomToSee = false;
};

///////////////////////////////////////////////////////////////////////////////
///   CVarTrackFactory
///
class NCBI_GUIWIDGETS_SEQGRAPHIC_EXPORT CVarTrackFactory :
    public CFeatureTrackFactory
{
public:
    CVarTrackFactory(){}

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

    virtual const CTrackTypeInfo&  GetThisTypeInfo() const
    { return GetTypeInfo(); }

    static const CTrackTypeInfo& GetTypeInfo()
    { return CVarTrack::m_TypeInfo; }

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
///   CVarTrack inline methods
///


END_NCBI_SCOPE

#endif  // GUI_WIDGETS_SEQ_GRAPHIC___DBVAR_TRACK__HPP
