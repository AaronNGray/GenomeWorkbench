#ifndef GUI_WIDGETS_SEQ_GRAPHIC___SEQTABLE_GRAPH_TRACK__HPP
#define GUI_WIDGETS_SEQ_GRAPHIC___SEQTABLE_GRAPH_TRACK__HPP

/*  $Id: seqtable_graph_track.hpp 26487 2012-09-24 18:32:04Z wuliangs $
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

#include <gui/widgets/seq_graphic/graph_track.hpp>

BEGIN_NCBI_SCOPE

///////////////////////////////////////////////////////////////////////////////
///   CSeqTableGraphTrack -- 
///

class NCBI_GUIWIDGETS_SEQGRAPHIC_EXPORT CSeqTableGraphTrack :
    public CGraphTrack
{
    friend class CSeqTableGraphTrackFactory;

public:
    CSeqTableGraphTrack(CSGGraphDS* ds, CRenderingContext* r_cntx);

    /// @name CLayoutTrack public methods.
    /// @{
    virtual const CTrackTypeInfo&  GetTypeInfo() const;
    /// @}

private:
    /// @name prohibited copy constructor and assignment operator.
    /// @{
    CSeqTableGraphTrack(const CSeqTableGraphTrack&);
    CSeqTableGraphTrack& operator=(const CSeqTableGraphTrack&);
    /// @}

private:
    static CTrackTypeInfo   m_TypeInfo;
};

///////////////////////////////////////////////////////////////////////////////
///   CSeqTableGraphTrackFactory
///
class NCBI_GUIWIDGETS_SEQGRAPHIC_EXPORT CSeqTableGraphTrackFactory :
    public CGraphTrackFactory
{
public:
    CSeqTableGraphTrackFactory(){}

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
    { return CSeqTableGraphTrack::m_TypeInfo; }

    /// @name IExtension interface implementation
    /// @{
    virtual string GetExtensionIdentifier() const;
    virtual string GetExtensionLabel() const;
    /// @}
};


END_NCBI_SCOPE

#endif  // GUI_WIDGETS_SEQ_GRAPHIC___SEQTABLE_GRAPH_TRACK__HPP
