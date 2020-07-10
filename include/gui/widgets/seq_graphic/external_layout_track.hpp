#ifndef GUI_WIDGETS_SEQ_GRAPHIC___EXTERNAL_LAYOUT_TRACK__HPP
#define GUI_WIDGETS_SEQ_GRAPHIC___EXTERNAL_LAYOUT_TRACK__HPP

/*  $Id: external_layout_track.hpp 23347 2011-03-10 16:34:59Z wuliangs $
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

#include <gui/widgets/seq_graphic/glyph_container_track.hpp>

#include <objects/seqloc/Seq_interval.hpp>
#include <objmgr/seq_vector.hpp>

/** @addtogroup GUI_LAYOUT_OBJECTS
 *
 * @{
 */

BEGIN_NCBI_SCOPE

///////////////////////////////////////////////////////////////////////////////
///   CExternalLayoutTrack -- the layout track for holding and visualizing the
///   external graphical views' visible ranges.
///
class NCBI_GUIWIDGETS_SEQGRAPHIC_EXPORT CExternalLayoutTrack :
    public CGlyphContainer
{
    friend class CExternalLayoutTrackFactory;

public:
    CExternalLayoutTrack(CRenderingContext* r_cntx);

    /// @name CLayoutTrack public virtual methods.
    /// @{
    virtual const CTrackTypeInfo&  GetTypeInfo() const;
    /// @}

private:
    /// @name prohibited copy constructor and assignment operator.
    /// @{
    CExternalLayoutTrack(const CExternalLayoutTrack&);
    CExternalLayoutTrack& operator=(const CExternalLayoutTrack&);
    /// @}

private:
    static CTrackTypeInfo m_TypeInfo;
};

///////////////////////////////////////////////////////////////////////////////
///   CExternalLayoutTrackFactory
///
class NCBI_GUIWIDGETS_SEQGRAPHIC_EXPORT CExternalLayoutTrackFactory :
    public CObject,
    public ILayoutTrackFactory,
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

    virtual bool UnderstandLevel() const
    { return false; }

    virtual bool NeedBackgroundInit() const
    { return false; }

    virtual const CTrackTypeInfo&  GetThisTypeInfo() const
    { return GetTypeInfo(); }

    static const CTrackTypeInfo&  GetTypeInfo()
    { return CExternalLayoutTrack::m_TypeInfo; }

    /// @name IExtension interface implementation
    /// @{
    virtual string GetExtensionIdentifier() const;
    virtual string GetExtensionLabel() const;
    /// @}
};


END_NCBI_SCOPE

/* @} */

#endif  // GUI_WIDGETS_SEQ_GRAPHIC___EXTERNAL_LAYOUT_TRACK__HPP
