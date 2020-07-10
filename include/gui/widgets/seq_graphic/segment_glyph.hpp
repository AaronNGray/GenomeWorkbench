#ifndef GUI_WIDGETS_SEQ_GRAPHIC___SEGMENT_GLYPH__HPP
#define GUI_WIDGETS_SEQ_GRAPHIC___SEGMENT_GLYPH__HPP

/*  $Id: segment_glyph.hpp 35413 2016-05-04 17:51:26Z evgeniev $
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
 * Authors:  Vlad Lebedev, Liangshou Wu, Victor Joukov
 *
 * File Description:
 *   CSegmentGlyph -- utility class to layout sequence segments and
 *                    hold CSeq_id_Handle objects
 */


#include <corelib/ncbiobj.hpp>
#include <gui/widgets/seq_graphic/seq_glyph.hpp>
#include <gui/widgets/seq_graphic/object_based_glyph.hpp>
#include <gui/widgets/seq_graphic/segment_config.hpp>
#include <gui/objutils/label.hpp>

#include <objects/seqloc/Seq_interval.hpp>
#include <objects/seq/seq_id_handle.hpp>


BEGIN_NCBI_SCOPE

class NCBI_GUIWIDGETS_SEQGRAPHIC_EXPORT CSegmentGlyph :
    public CSeqGlyph,
    public IObjectBasedGlyph
{
public:
    typedef vector< CRef<CSegmentGlyph> > TSeqMapList;

    /// @name ctors
    /// @{
    CSegmentGlyph(//objects::CScope& scope,
                  const objects::CSeq_id_Handle& component_id,
                  const TSeqRange& range,
                  const objects::CSeq_id_Handle& mapped_id,
                  const TSeqRange& mapped_range,
                  bool component_negative_strand,
                  TSeqPos length = 0);
    /// @}

    /// @name CSeqGlyph virtual methods.
    /// @{
    virtual bool NeedTooltip(const TModelPoint& p, ITooltipFormatter& tt, string& t_title) const;
    virtual void GetTooltip(const TModelPoint& p, ITooltipFormatter& tt, string& t_title) const;
    virtual void GetHTMLActiveAreas(TAreaVector* p_areas) const;
    virtual TSeqRange GetRange(void) const;
    virtual void SetHideLabel(bool b);
    virtual bool HasSideLabel() const;
    virtual bool IsClickable() const;
    /// @}

    /// @name IObjectBasedGlyph interface implement
    /// @{
    /// Access the remapped location.
    const objects::CSeq_loc&  GetLocation(void) const;
    /// Access the data as a CObject.
    virtual CConstRef<CObject> GetObject(TSeqPos pos) const;
    virtual void GetObjects(vector<CConstRef<CObject> >& objs) const;
    virtual bool HasObject(CConstRef<CObject> obj) const;
    virtual string GetSignature() const;
    virtual const TIntervals& GetIntervals(void) const;
    /// @}

    CConstRef<objects::CSeq_id>  GetSeqID(void) const;
    TSeqPos GetComponentLength() const;
    const set<TSeqRange>& GetComponentRanges() const;
    const set<TSeqRange>& GetMappedRanges() const;

    void SetComponentRanges(const set<TSeqRange>& range);
    void SetMappedRanges(const set<TSeqRange>& range);

    CSegmentConfig::TSegMapQuality GetSeqQuality() const;
    void SetSeqQuality(CSegmentConfig::TSegMapQuality quality);

    void SetConfig(CRef<CSegmentConfig> config);

    void GetLabel(string& label, CLabel::ELabelType type) const;

protected:
    virtual void x_Draw() const;
    virtual void x_UpdateBoundingBox();

private:
    bool x_ShowLabel() const;

protected:
    /// our mapped location
    CRef<objects::CSeq_loc>     m_Location;

    /// the seq-id we cover.  this is the component seq-id
    objects::CSeq_id_Handle     m_IdHandle;
    /// range on component sequence
    TSeqRange                   m_ComponentRange;
    /// the seq-id we cover.  this is the master seq-id
    objects::CSeq_id_Handle     m_MappedIdHandle;
    /// range on master (mapped) sequence
    TSeqRange                   m_MappedRange;
    bool                        m_CompNegative;

    /// length of the component sequence, independent of the chunk we use
    TSeqPos m_Length;



    /// the range on the component we represent
    set<TSeqRange> m_SeqRanges;

    /// the range on the main sequence we represent
    set<TSeqRange> m_MappedSeqRanges;

    CSegmentConfig::TSegMapQuality    m_SegMapQuality;

    /// intervals (like for features or alignments).
    TIntervals  m_Intervals;

    /// rendering configuration for a segment.
    CConstRef<CSegmentConfig>   m_Config;

    /// Force to hide label.
    /// Flag to force hiding the label regardless of the settings
    /// from CSegmentConfig
    bool        m_HideLabel;
};


///////////////////////////////////////////////////////////////////////////////
///   CSegmentGlyph inline methods
///
inline
CConstRef<objects::CSeq_id> CSegmentGlyph::GetSeqID(void) const
{
    return m_IdHandle.GetSeqId();
}

inline
TSeqPos CSegmentGlyph::GetComponentLength() const
{
    return m_Length;
}

inline
const set<TSeqRange>& CSegmentGlyph::GetComponentRanges() const
{
    return m_SeqRanges;
}

inline
const set<TSeqRange>& CSegmentGlyph::GetMappedRanges() const
{
    return m_MappedSeqRanges;
}

inline
void CSegmentGlyph::SetComponentRanges(const set<TSeqRange>& range)
{
    m_SeqRanges = range;
}

inline
void CSegmentGlyph::SetMappedRanges(const set<TSeqRange>& range)
{
    m_MappedSeqRanges = range;
}

inline
CSegmentConfig::TSegMapQuality CSegmentGlyph::GetSeqQuality() const
{
    return m_SegMapQuality;
}

inline
void CSegmentGlyph::SetSeqQuality(CSegmentConfig::TSegMapQuality quality)
{
    m_SegMapQuality = quality;
}

inline
void CSegmentGlyph::SetConfig(CRef<CSegmentConfig> config)
{
    m_Config = config;
}


END_NCBI_SCOPE

/* @} */

#endif  // GUI_WIDGETS_SEQ_GRAPHIC___SEGMENT_GLYPH__HPP
