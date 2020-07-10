#ifndef GUI_WIDGETS_SEQ_GRAPHIC___SEGMENT_SMEAR_GLYPH__HPP
#define GUI_WIDGETS_SEQ_GRAPHIC___SEGMENT_SMEAR_GLYPH__HPP

/*  $Id: segment_smear_glyph.hpp 36548 2016-10-06 18:53:00Z shkeda $
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
 *   CSegmentSmearGlyph -- a smear bar to represent a segments map
 */


#include <corelib/ncbiobj.hpp>
#include <gui/widgets/seq_graphic/seq_glyph.hpp>
#include <gui/objutils/density_map.hpp>

BEGIN_NCBI_SCOPE

class IReader;
class IWriter;

class NCBI_GUIWIDGETS_SEQGRAPHIC_EXPORT CSegmentSmearGlyph : public CSeqGlyph
{
public:
    typedef unsigned int            TValueType;
    typedef vector<CRgbaColor>  TColorCode;

    class CSegMap :
        public CDensityMap<TValueType>
    {
    public:
        CSegMap(TSeqPos start = 0, TSeqPos stop = 0, double window = 1, accum_functor* func = NULL, TValueType def = 0) :
            CDensityMap(start, stop, window, func, def),
            m_ReservedCount(1000)
        {}
        CSegMap(const objects::CBioseq_Handle& handle, double window = 1, accum_functor* func = NULL, TValueType def = 0) :
            CDensityMap(handle, window, func, def),
            m_ReservedCount(1000)
        {}

        void AddRange(TSeqRange range, TValueType score = 1, bool expand = false);
        void AddRanges(CRangeCollection<TSeqPos> ranges, TValueType score = 1, bool expand = false);

        void Serialize(IWriter& writer) const;
        void Deserialize(IReader& reader);
        void SetRange(const TSeqRange& range);
        void GetGaps(const TModelRange& range, vector<TSeqRange>& gaps);
    private:
        struct SRangeItem
        {
            TSeqPos     from;
            TSeqPos     to;
            TValueType  score;
        };
        size_t                  m_ReservedCount;
        std::vector<SRangeItem> m_RangesCache;
    };

    CSegmentSmearGlyph(auto_ptr<CSegMap> map);

    /// @name CSeqGlyph virtual methods.
    /// @{
    virtual bool NeedTooltip(const TModelPoint& p, ITooltipFormatter& tt, string& t_title) const;
    virtual void GetTooltip(const TModelPoint& p, ITooltipFormatter& tt, string& t_title) const;
    virtual bool IsClickable() const;
    /// @}

    void SetColorCode(const TColorCode& colors);

protected:
    virtual void x_Draw() const;
    virtual void x_UpdateBoundingBox();

    void x_DrawGaps() const;

private:
    auto_ptr<CSegMap>   m_SegMap;

    /// color code for segment map.
    TColorCode          m_Colors;
};


///////////////////////////////////////////////////////////////////////////////
///   CSegmentSmearGlyph inline methods
///
inline
void CSegmentSmearGlyph::SetColorCode(const TColorCode& colors)
{
    m_Colors = colors;
}


END_NCBI_SCOPE


#endif  // GUI_WIDGETS_SEQ_GRAPHIC___SEGMENT_SMEAR_GLYPH__HPP
