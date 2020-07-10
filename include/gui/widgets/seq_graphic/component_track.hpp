#ifndef GUI_WIDGETS_SEQ_GRAPHIC___COMPONENT_TRACK__HPP
#define GUI_WIDGETS_SEQ_GRAPHIC___COMPONENT_TRACK__HPP

/*  $Id: component_track.hpp 33142 2015-06-03 19:06:10Z joukovv $
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

#include <gui/widgets/seq_graphic/segment_map_track.hpp>


BEGIN_NCBI_SCOPE

///////////////////////////////////////////////////////////////////////////////
///   CComponentTrack -- 
///

class NCBI_GUIWIDGETS_SEQGRAPHIC_EXPORT CComponentTrack :
    public CSegmentMapTrack
{
    friend class CComponentTrackFactory;

public:
    CComponentTrack(CSGSegmentMapDS* ds, CRenderingContext* r_cntx);
    virtual ~CComponentTrack();

    /// @name CLayoutTrack public methods.
    /// @{
    virtual const CTrackTypeInfo&  GetTypeInfo() const;
    virtual string GetFullTitle() const;
    /// @}

private:
    /// @name prohibited copy constructor and assignment operator.
    /// @{
    CComponentTrack(const CComponentTrack&);
    CComponentTrack& operator=(const CComponentTrack&);
    /// @}

private:
    static CTrackTypeInfo   m_TypeInfo;
};

///////////////////////////////////////////////////////////////////////////////
///   CComponentTrackFactory
///
class NCBI_GUIWIDGETS_SEQGRAPHIC_EXPORT CComponentTrackFactory :
    public CSegmentMapTrackFactory
{
public:
    CComponentTrackFactory(){}

    /// create a layout track based on the input objects and extra parameters.
    virtual TTrackMap CreateTracks(
        SConstScopedObject& object,
        ISGDataSourceContext* ds_context,
        CRenderingContext* r_cntx,
        const SExtraParams& params = SExtraParams(),
        const TAnnotMetaDataList& src_annots = TAnnotMetaDataList()) const;

    virtual const CTrackTypeInfo&  GetThisTypeInfo() const
    { return GetTypeInfo(); }

    static const CTrackTypeInfo& GetTypeInfo()
    { return CComponentTrack::m_TypeInfo; }

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
///   CComponentTrack inline methods
///


END_NCBI_SCOPE

#endif  // GUI_WIDGETS_SEQ_GRAPHIC___SCAFFOLD_TRACK__HPP
