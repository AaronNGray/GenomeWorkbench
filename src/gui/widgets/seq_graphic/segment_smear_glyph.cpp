/*  $Id: segment_smear_glyph.cpp 43684 2019-08-14 16:45:11Z katargir $
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

#include <ncbi_pch.hpp>
#include <corelib/rwstream.hpp>
#include <gui/widgets/seq_graphic/segment_smear_glyph.hpp>
#include <gui/widgets/seq_graphic/segment_config.hpp>
#include <gui/widgets/seq_graphic/rendering_ctx.hpp>
#include <gui/opengl/irender.hpp>
#include <connect/services/netservice_api_expt.hpp>
#include <gui/opengl/rasterizer.hpp>

BEGIN_NCBI_SCOPE
USING_SCOPE(objects);

const size_t resizeStep = 500;

CSegmentSmearGlyph::CSegmentSmearGlyph(auto_ptr<CSegMap> map)
    : m_SegMap(map)
{}


bool CSegmentSmearGlyph::NeedTooltip(const TModelPoint& /*p*/, ITooltipFormatter& /*tt*/, string& /*t_title*/) const
{
    return false;
}

void CSegmentSmearGlyph::GetTooltip(const TModelPoint& /*p*/, ITooltipFormatter& /*tt*/, string& /*t_title*/) const
{
}


bool CSegmentSmearGlyph::IsClickable() const
{
    return true;
}


void CSegmentSmearGlyph::x_Draw() const
{
    if ( !m_SegMap.get() ) return;

    IRender& gl = GetGl();

    TModelUnit top = GetTop();
    TModelUnit bottom = top + (GetHeight() - 1);
//    top += 1;
//    bottom -= 1;
    CSegMap::runlen_iterator seg_it = m_SegMap->RunLenBegin();
    for (; seg_it; seg_it.Advance()) {
        TValueType value = seg_it.GetValue();
        _ASSERT(value < m_Colors.size());
        TSeqPos f = seg_it.GetSeqPosition();
        TSeqPos t = seg_it.GetSeqRunEndPosition();
        if (value == (TValueType)CSegmentConfig::eGap) {
            gl.ColorC(m_Colors[value]);
            m_Context->DrawQuad(f, top, t, bottom);
        } else {
            m_Context->Draw3DQuad(f, top, t, bottom, m_Colors[value], false);
        }
    }
    if (m_Context->GetScale() > 1.)
        x_DrawGaps();
}

void CSegmentSmearGlyph::x_DrawGaps() const
{

    IRender& gl = GetGl();

    vector<TSeqRange> gaps;
    m_SegMap->GetGaps(m_Context->GetVisibleRange(), gaps);
    if (gaps.empty())
        return;

    CGlPane* pane = m_Context->GetGlPane();
    TVPRect rc_vp = pane->GetViewport();

    gl.PushAttrib(GL_ALL_ATTRIB_BITS);
    gl.PushMatrix();
    gl.Scalef(m_Context->GetScale(), 1.0f, 1.0f);
    TModelUnit seq_from = m_Context->GetVisibleFrom();
    TModelUnit seq_to = m_Context->GetVisibleTo();
    
    TVPUnit screen_from = rc_vp.Left();
    TVPUnit screen_to = rc_vp.Right();
    double shift_x = m_Context->SeqToScreenX(seq_from);

    CRasterizer<float>  rasterizer(screen_from, screen_to, seq_from, seq_to);
    for (auto&& gap : gaps) {
        rasterizer.AddInterval(gap.GetFrom(), gap.GetToOpen(), 1., [](float& dst, const float& src, float fraction, bool) {
            dst = min<float>(1., dst + fraction);
        });
    }
    gl.Enable(GL_BLEND);
    gl.BlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    gl.PolygonMode(GL_FRONT_AND_BACK, GL_FILL);

    auto& raster = rasterizer.GetRaster();

    static const float kGaps_Threshold = 0.025f;
    TModelUnit top = GetTop();
    TModelUnit bottom = top + (GetHeight() - 1);

    for (size_t x1 = 0; x1 < raster.size();) {
        if (raster[x1] < kGaps_Threshold) {
            ++x1;
            continue;
        }
        size_t x2 = x1;
        float val = max<float>(0.25, raster[x1]);
        while (++x2 < raster.size()) {
            const float& new_val = raster[x2];
            if (new_val < kGaps_Threshold || max<float>(0.25, new_val) != val)
                break;
        }
        size_t next_pos = x2;
        x2 -= 1;
        CRgbaColor gap_color = m_Colors[CSegmentConfig::eGap];
        if (val < 0.5)
            gap_color.SetAlpha(val);
        gl.ColorC(gap_color);

        double start = x1 + shift_x;
        double stop = x2 + shift_x;
        if (start == stop) {
            gl.Begin(GL_LINES);
            gl.Vertex2d(start, top);
            gl.Vertex2d(stop, bottom);
            gl.End();
        } else {
            gl.Rectd(start, top, stop, bottom);
        }

        x1 = next_pos;
    }
    gl.PopMatrix();
    gl.PopAttrib();

}

void CSegmentSmearGlyph::x_UpdateBoundingBox()
{
    CRef<CSGSequenceDS> seq_ds = m_Context->GetSeqDS();
    SetWidth(seq_ds->GetSequenceLength());
    SetLeft(0.0);
}

void CSegmentSmearGlyph::CSegMap::AddRange(TSeqRange range, TValueType score, bool expand)
{
    if (m_RangesCache.capacity() <= m_ReservedCount)
        m_RangesCache.reserve(m_ReservedCount);
    if (m_RangesCache.capacity() == m_RangesCache.size()) {
        m_ReservedCount += resizeStep;
        m_RangesCache.reserve(m_ReservedCount);
    }
    SRangeItem item;
    item.from = range.GetFrom();
    item.to = range.GetTo();
    item.score = score;
    m_RangesCache.push_back(item);
    CDensityMap<TValueType>::AddRange(range, score, expand);
}

void CSegmentSmearGlyph::CSegMap::AddRanges(CRangeCollection<TSeqPos> ranges, TValueType score, bool expand)
{
    if (m_RangesCache.capacity() <= m_ReservedCount)
        m_RangesCache.reserve(m_ReservedCount);
    if (m_RangesCache.capacity() == m_RangesCache.size()) {
        m_ReservedCount += resizeStep;
        m_RangesCache.reserve(m_ReservedCount);
    }
    SRangeItem item;
    ITERATE(CRangeCollection<TSeqPos>, range_it, ranges) {
        item.from = range_it->GetFrom();
        item.to = range_it->GetTo();
        item.score = score;
        m_RangesCache.push_back(item);
    }
    CDensityMap<TValueType>::AddRanges(ranges, score, expand);
}

void CSegmentSmearGlyph::CSegMap::Serialize(IWriter& writer) const
{
    size_t sz = m_RangesCache.size();
    writer.Write(reinterpret_cast<const char*>(&sz), sizeof(sz));
    writer.Write(reinterpret_cast<const char*>(&m_RangesCache[0]), m_RangesCache.size() * sizeof(m_RangesCache[0]));
    writer.Flush();
}

void CSegmentSmearGlyph::CSegMap::Deserialize(IReader& reader)
{
    size_t sz(0);
    reader.Read(reinterpret_cast<char*>(&sz), sizeof(sz));
    m_RangesCache.resize(sz);
    size_t bytes_read;
    size_t total_bytes_read = 0;
    size_t buf_size = sz * sizeof(m_RangesCache[0]);
    char* buf_ptr = reinterpret_cast<char*>(&m_RangesCache[0]);
    while (buf_size > 0) {
        ERW_Result rw_res = reader.Read(buf_ptr, buf_size, &bytes_read);
        if (rw_res == eRW_Success) {
            total_bytes_read += bytes_read;
            buf_ptr += bytes_read;
            buf_size -= bytes_read;
        } else if (rw_res == eRW_Eof) {
            break;
        } else {
            NCBI_THROW(CNetServiceException, eCommunicationError, "Error while reading BLOB");
        }
    }

    for (size_t i = 0; i < m_RangesCache.size(); ++i) {
        const SRangeItem &item = m_RangesCache[i];
        CDensityMap<TValueType>::AddRange(TSeqRange(item.from,item.to), item.score, false);
    }
}

void CSegmentSmearGlyph::CSegMap::SetRange(const TSeqRange& range)
{
    m_Bins.clear();
    m_Max = m_DefVal;
    m_Min = m_DefVal;
    m_Bins.resize(x_CalcNbins(), m_DefVal);
    m_Range = range;
    for (size_t i = 0; i < m_RangesCache.size(); ++i) {
        const SRangeItem &item = m_RangesCache[i];
        CDensityMap<TValueType>::AddRange(TSeqRange(item.from, item.to), item.score, false);
    }
}

void CSegmentSmearGlyph::CSegMap::GetGaps(const TModelRange& range, vector<TSeqRange>& gaps)
{
    gaps.clear();
    for (size_t i = 0; i < m_RangesCache.size(); ++i) {
        const SRangeItem &item = m_RangesCache[i];
        if (item.to < range.GetFrom())
            continue;
        if (item.from > range.GetTo())
            break;
        if (item.score == CSegmentConfig::eGap) {
            gaps.push_back(TSeqRange(item.from, item.to));
        }
    }
}

END_NCBI_SCOPE
