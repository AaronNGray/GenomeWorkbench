/*  $Id: restriction_sites_glyph.cpp 41823 2018-10-17 17:34:58Z katargir $
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
#include <gui/widgets/seq_graphic/restriction_sites_glyph.hpp>
#include <gui/widgets/seq_graphic/rendering_ctx.hpp>
#include <gui/objutils/obj_fingerprint.hpp>
#include <gui/objutils/tooltip.hpp>
#include <gui/objutils/utils.hpp>
#include <objects/seqfeat/Rsite_ref.hpp>
#include <objects/general/Dbtag.hpp>


BEGIN_NCBI_SCOPE
USING_SCOPE(objects);

CRsitesGlyph::CRsitesGlyph(const CSeq_feat& feat)
    : m_Feature(&feat)
    , m_SiteW(8)
    , m_SiteH(6)
    , m_Color(0.2f, 1.0f, 0.2f, 0.7f)
{
}


bool CRsitesGlyph::NeedTooltip(const TModelPoint& p, ITooltipFormatter& tt, string& t_title) const
{
    _ASSERT(m_Context);

    TModelUnit m_p = p.X() - 0.5;
    const CPacked_seqpnt::TPoints& pnts =
        m_Feature->GetLocation().GetPacked_pnt().GetPoints();

    // Assuming the sites are sorted by position
    CPacked_seqpnt::TPoints::const_iterator iter =
        std::lower_bound(pnts.begin(), pnts.end(), m_p);
    TModelUnit half_w = m_Context->ScreenToSeq(m_SiteW) * 0.5;

    int site_p = -1;
    if (iter == pnts.end()) {
        if ((m_p - pnts.back()) < half_w) {
            site_p = pnts.back();
        }
    } else {
        if (iter != pnts.begin()  &&  m_p - *(iter - 1) < *iter - m_p) {
            if (m_p - *(iter - 1) < half_w) site_p = *(iter - 1);
        } else if (*iter - m_p < half_w) {
            site_p = *iter;
        }
    }

    if (site_p > 0) {
        t_title = "Restriction site";
        tt.AddRow("Enzyme:", GetLabel());      
        tt.AddRow("Site:", NStr::NumericToString(site_p + 1, NStr::fWithCommas));
        return true;
    }

    return false;
}


void CRsitesGlyph::GetHTMLActiveAreas(TAreaVector* p_areas) const
{
    CHTMLActiveArea base_area;
    CSeqGlyph::x_InitHTMLActiveArea(base_area);
    base_area.m_PositiveStrand = true; // this doesn't really matter

    // For every (visible) site, create a separate HTML active area
    TSeqPos from = m_Context->GetVisSeqFrom();
    TSeqPos to = m_Context->GetVisSeqTo();
    const CPacked_seqpnt::TPoints& pnts =
        m_Feature->GetLocation().GetPacked_pnt().GetPoints();

    // Assuming the sites are sorted by position
    CPacked_seqpnt::TPoints::const_iterator iter =
        std::lower_bound(pnts.begin(), pnts.end(), from);

    TSeqPos half_w = m_SiteW / 2;
    while (iter != pnts.end()  &&  *iter <= to) {
        CHTMLActiveArea area = base_area;
        TVPUnit x = m_Context->SeqToScreenXClipped(*iter + 0.5);
        area.m_Bounds.SetLeft(x < half_w ? 0 : x - half_w);
        area.m_Bounds.SetRight(x + half_w);
        area.m_SeqRange.Set(*iter, *iter);
        area.m_Signature = CObjFingerprint::GetRsiteSignature(
            *m_Feature, *iter, &m_Context->GetScope());
        p_areas->push_back(area);

        ++iter;
    }
}


TSeqRange CRsitesGlyph::GetRange(void) const
{
    TSeqRange range;
    try { // watch out for mix loc with multiple seq-ids
        range = m_Feature->GetLocation().GetTotalRange();
    } catch (CException&) {
        // For restrication sites, we don't expect this
        LOG_POST(Error << "Can't get the total range for restriction site: "
            << GetLabel());
    }

    return range;
}


string CRsitesGlyph::GetLabel() const
{
    string label;
    if (m_Feature->GetData().GetRsite().IsStr()) {
        label = m_Feature->GetData().GetRsite().GetStr();
    } else if (m_Feature->GetData().GetRsite().IsDb()) { // must be a db
        m_Feature->GetData().GetRsite().GetDb().GetLabel(&label);
    }

    return label;
}


bool CRsitesGlyph::HasVisibleRsite() const
{
    TSeqPos from = m_Context->GetVisSeqFrom();
    TSeqPos to = m_Context->GetVisSeqTo();
    const CPacked_seqpnt::TPoints& pnts =
        m_Feature->GetLocation().GetPacked_pnt().GetPoints();

    // Assuming the sites are sorted by position
    CPacked_seqpnt::TPoints::const_iterator low =
        std::lower_bound(pnts.begin(), pnts.end(), from);
    CPacked_seqpnt::TPoints::const_iterator up =
        std::upper_bound(pnts.begin(), pnts.end(), to);

    return low != up  ||  *up == to;
}


void CRsitesGlyph::x_Draw() const
{
    _ASSERT(m_Context);

    TSeqPos from = m_Context->GetVisSeqFrom();
    TSeqPos to = m_Context->GetVisSeqTo();
    const CPacked_seqpnt::TPoints& pnts =
        m_Feature->GetLocation().GetPacked_pnt().GetPoints();

    // Assuming the sites are sorted by position
    CPacked_seqpnt::TPoints::const_iterator iter =
        std::lower_bound(pnts.begin(), pnts.end(), from);

    TModelUnit base = GetTop();
    IRender& gl = GetGl();
    TModelUnit half_w = m_Context->ScreenToSeq(m_SiteW) * 0.5;
    while (iter != pnts.end()  &&  *iter <= to) {
        gl.ColorC(m_Color);
        m_Context->DrawTriangle(*iter + 0.5, base, *iter - half_w + 0.5,
            base + m_SiteH, *iter + half_w + 0.5, base + m_SiteH, true);
        ++iter;
    }
}


void CRsitesGlyph::x_UpdateBoundingBox()
{
    _ASSERT(m_Context);

    TSeqRange range = GetRange();
    SetHeight(m_SiteH);
    SetWidth(range.GetLength());
    SetLeft(range.GetFrom());
}



END_NCBI_SCOPE
