/*  $Id: segment_glyph.cpp 42079 2018-12-13 20:37:10Z evgeniev $
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
 *   CSegmentGlyph -- utility class to layout sequence segments
 *                    and hold CSeq_id_Handle objects
 */

#include <ncbi_pch.hpp>
#include <gui/widgets/seq_graphic/segment_glyph.hpp>
#include <gui/widgets/seq_graphic/rendering_ctx.hpp>
#include <gui/objutils/tooltip.hpp>
#include <gui/objutils/obj_fingerprint.hpp>
#include <gui/objutils/gui_object_info_component.hpp>

#include <gui/opengl/irender.hpp>

#include <objects/seq/seq_id_handle.hpp>
#include <objects/seqloc/Seq_loc.hpp>
#include <objects/seqfeat/Org_ref.hpp>
#include <objects/seqfeat/BioSource.hpp>
#include <objects/seqfeat/SubSource.hpp>

#include <objmgr/seqdesc_ci.hpp>
#include <objmgr/util/sequence.hpp>


BEGIN_NCBI_SCOPE
USING_SCOPE(objects);

/// extra space for side labeling.
static const int kSideLabelSpace = 5;

/// vertical space between label and object.
static const int kVertLabelSpace = 2;

CSegmentGlyph::CSegmentGlyph(const CSeq_id_Handle& component_id,
                             const TSeqRange& range,
                             const CSeq_id_Handle& mapped_id,
                             const TSeqRange& mapped_range,
                             bool component_negative_strand,
                             TSeqPos length)
    : m_IdHandle(component_id)
    , m_ComponentRange(range)
    , m_MappedIdHandle(mapped_id)
    , m_CompNegative(component_negative_strand)
    , m_SegMapQuality(CSegmentConfig::eDefault)
    , m_HideLabel(false)
{
    m_SeqRanges.insert(range);
    m_MappedSeqRanges.insert(mapped_range);

    TSeqPos int_start = mapped_range.GetFrom();
    TSeqPos int_end   = mapped_range.GetTo();

    if (length) {
        // We know true length.
        m_Length = length;

        /// 5' overhang
        TSeqPos over_5p = range.GetFrom();
        TSeqPos over_3p = m_Length - range.GetToOpen();
        if (component_negative_strand) {
            swap(over_5p, over_3p);
        }

        if (int_start > over_5p) {
            int_start -= over_5p;
        } else {
            int_start = 0;
        }

        /// 3' overhang
        int_end += over_3p;
    } else {
        // We don't know length, so no 3' overhang.
        m_Length = range.GetTo();
        // TODO: Shouldn't we adjust int_start here anyway?
    }
    m_MappedRange = TSeqRange(int_start, int_end);

    // This location is schizophrenic - it has id of component
    // and range of master sequence. We'll leave it alone for
    // a while and better make 2 true locations.
    m_Location.Reset(new CSeq_loc());
    m_Location->SetInt().SetFrom(int_start);
    m_Location->SetInt().SetTo  (int_end);
    CConstRef<CSeq_id> c_id = component_id.GetSeqId();
    m_Location->SetInt().SetId().Assign(*c_id);
    if (component_negative_strand) {
        m_Location->SetInt().SetStrand(eNa_strand_minus);
    }

    m_Intervals.push_back(m_Location->GetTotalRange());

    /**
    LOG_POST(Info
        << "CSegmentGlyph(): [" << mapped_range.GetFrom() << " - " << mapped_range.GetTo() << "]: "
        << " source range: [" << range.GetFrom() << " - " << range.GetTo() << "]: "
        << " seq len = " << m_Length << "  over 5' = " << over_5p << "  over 3' = " << over_3p
        << "  interval: " << m_Location->GetInt().GetFrom() << " - " << m_Location->GetInt().GetTo());
        **/
}


bool CSegmentGlyph::NeedTooltip(const TModelPoint& /*p*/, ITooltipFormatter& /*tt*/, string& /*t_title*/) const
{
    return true;
}


void CSegmentGlyph::GetTooltip(const TModelPoint& /*p*/, ITooltipFormatter& tt, string& t_title) const
{
    CScope& scope = m_Context->GetScope();
    SConstScopedObject scoped_obj(m_Location, &scope);
    CIRef<IGuiObjectInfo> gui_info(
        CGuiObjectInfoComponent::CreateObject(scoped_obj, NULL));

    if (!gui_info) return;

    bool isTooltipGeneratedBySvc(false);
    gui_info->GetToolTip(tt, t_title, (TSeqPos)-1, &isTooltipGeneratedBySvc);
    if (!isTooltipGeneratedBySvc) {
        gui_info->GetLinks(tt, false);
    }
}


void CSegmentGlyph::GetHTMLActiveAreas(TAreaVector* p_areas) const
{
    CHTMLActiveArea area;
    CSeqGlyph::x_InitHTMLActiveArea(area);
    area.m_PositiveStrand = (sequence::GetStrand(GetLocation()) != eNa_strand_minus);
    area.m_DB_Name = "nucleotide";
    GetSeqID()->GetLabel(&area.m_DB_ID, CSeq_id::eContent);
    area.m_Signature = GetSignature();
    p_areas->push_back(area);
}


TSeqRange CSegmentGlyph::GetRange(void) const
{
    return m_Location->GetTotalRange();
}


void CSegmentGlyph::SetHideLabel(bool b)
{
    m_HideLabel = b;
}


bool CSegmentGlyph::HasSideLabel() const
{
    return !m_HideLabel  &&
        m_Config->m_LabelPos == CSegmentConfig::ePos_Side;
}


bool CSegmentGlyph::IsClickable() const
{
    return true;
}


// CConstRef
CConstRef<CObject> CSegmentGlyph::GetObject(TSeqPos /*pos*/) const
{
    return CConstRef<CObject>(m_IdHandle.GetSeqId());
}


void CSegmentGlyph::GetObjects(vector<CConstRef<CObject> >& objs) const
{
    objs.push_back( CConstRef<CObject>(m_IdHandle.GetSeqId()) );
}


bool CSegmentGlyph::HasObject(CConstRef<CObject> obj) const
{
    return m_IdHandle.GetSeqId().GetPointer() == obj.GetPointer();
}


string CSegmentGlyph::GetSignature() const
{
    objects::CSeq_loc comp_loc;
    comp_loc.SetInt().SetFrom(m_ComponentRange.GetFrom());
    comp_loc.SetInt().SetTo(m_ComponentRange.GetTo());
    CConstRef<CSeq_id> id = m_IdHandle.GetSeqId();
    comp_loc.SetInt().SetId().Assign(*id);

    objects::CSeq_loc mapped_loc;
    mapped_loc.SetInt().SetFrom(m_MappedRange.GetFrom());
    mapped_loc.SetInt().SetTo(m_MappedRange.GetTo());
    CConstRef<CSeq_id> mapped_id = m_MappedIdHandle.GetSeqId();
    mapped_loc.SetInt().SetId().Assign(*mapped_id);

    if (m_CompNegative) {
        mapped_loc.SetInt().SetStrand(eNa_strand_minus);
    }

    return  CObjFingerprint::GetComponentSignature(
        comp_loc, mapped_loc, &m_Context->GetScope());
}


const CSegmentGlyph::TIntervals& CSegmentGlyph::GetIntervals(void) const
{
    return m_Intervals;
}


const objects::CSeq_loc& CSegmentGlyph::GetLocation(void) const
{
    return *m_Location;
}


void CSegmentGlyph::GetLabel(string& label, CLabel::ELabelType type) const
{
    CLabel::GetLabel(*GetSeqID(), &label, type, &m_Context->GetScope());
}


void CSegmentGlyph::x_Draw() const
{
    IRender& gl = GetGl();

    bool show_label = x_ShowLabel();
    TSeqRange range = GetRange();
    TModelRect rcm  = GetModelRect();
    TModelUnit base = rcm.Top();
    TModelUnit label_h =  gl.TextHeight(&(m_Config->m_LabelFont));

    TModelUnit line_y1 = base + 1;
    if (show_label  &&  m_Config->m_LabelPos == CSegmentConfig::ePos_Above) {
        line_y1 += kVertLabelSpace + label_h;
    }
    TModelUnit line_y2 = line_y1 + m_Config->m_BarHeight;

    TSeqPos from = range.GetFrom();
    TSeqPos to   = range.GetTo();

    // draw segments as quads
    CRgbaColor color1 = m_Config->m_FGExc;
    CRgbaColor color2 = m_Config->m_FGInc_U;

    switch (GetSeqQuality()) {
        case CSegmentConfig::eFinished:
            color2 = m_Config->m_FGInc_F;
            break;
        case CSegmentConfig::eDraft:
            color2 = m_Config->m_FGInc_D;
            break;
        case CSegmentConfig::eWgs:
            color2 = m_Config->m_FGInc_W;
            break;
        default:
            break;
    }

    m_Context->Draw3DQuad(from, line_y1, to, line_y2, color1, true);
    ITERATE (set<TSeqRange>, it, GetMappedRanges()) {
        m_Context->Draw3DQuad(it->GetFrom(), line_y1, it->GetTo(),
            line_y2, color2, true);
    }

    TModelRange vis_r = m_Context->IntersectVisible(range);
    bool show_strand = m_Context->WillLabelFit(vis_r);
    // draw strand indicator and/or label
    // show label or not, we will need to draw strand indicator if it fits
    if (show_label  ||  show_strand) {
        bool neg_strand = (sequence::GetStrand(GetLocation()) == eNa_strand_minus);
        bool strnd = m_Context->IsFlippedStrand() ? !neg_strand : neg_strand;
        const CGlTextureFont& font = m_Config->m_LabelFont;
        string label;
        GetLabel(label, CLabel::eContent);
        string strand_sign = strnd ? "<" : ">";
        TModelUnit cnt_x = (vis_r.GetFrom() + vis_r.GetTo()) * 0.5;
        TModelUnit xM = cnt_x;
        TModelUnit yM = base + 1 + gl.TextHeight(&font);
        if (m_Config->m_LabelPos == CSegmentConfig::ePos_Side) {
            vis_r = m_Context->IntersectVisible(this);
        }
        TModelUnit max_label_w = m_Context->SeqToScreen(vis_r.GetLength());
        if (max_label_w > 2.0) max_label_w -= 2.0;

        TModelUnit label_w = gl.TextWidth(&font, label.c_str());

        if (IsSelected()) {
            gl.ColorC(m_Config->m_SelLabel);
        } else {
            gl.ColorC(color2.ContrastingColor());
        }
        if (m_Config->m_LabelPos == CSegmentConfig::ePos_Above) {
            // draw strand
            TModelUnit y = line_y1 + gl.TextHeight(&font);
            m_Context->TextOut(&font, strand_sign.c_str(), xM, y, true);
        } else if (m_Config->m_LabelPos == CSegmentConfig::ePos_Inside) {
            label = strand_sign + " " + label + " " + strand_sign;
        } else if (m_Config->m_LabelPos == CSegmentConfig::ePos_Side) {
            // draw strand
            if (show_strand) {
                if ( !IsSelected() ) {
                    gl.ColorC(color2.ContrastingColor());
                }
                m_Context->TextOut(&font, strand_sign.c_str(), xM, yM, true);
            }

            TModelUnit max_w = m_Context->GetMaxLabelWidth(font);
            if (label_w > max_w) {
                label_w = max_w;
            }
            TModelUnit t_w = label_w > max_label_w ? max_label_w : label_w;
            t_w = m_Context->ScreenToSeq(t_w);
            if (neg_strand) {
                xM = vis_r.GetTo() - t_w;
                if (vis_r.GetTo() < GetRight()) {
                    gl.ColorC(m_Config->m_BG);
                    m_Context->DrawBackground( TModelRect(xM, yM + 3.0,
                        vis_r.GetTo(), yM - label_h - 1), 0);
                }
            } else {
                xM = vis_r.GetFrom();
                if (vis_r.GetFrom() > GetLeft()) {
                    gl.ColorC(m_Config->m_BG);
                    m_Context->DrawBackground( TModelRect(xM, yM + 3.0,
                        xM + t_w, yM - label_h - 1), 0);
                }
            }
        } else {
            m_Context->TextOut(&font, strand_sign.c_str(), xM, yM, true);
        }

        if (show_label) {
            if (label_w > max_label_w) {
                label = font.Truncate(label.c_str(), max_label_w);
            }
            if (IsSelected()) {
                gl.ColorC(m_Config->m_SelLabel);
            } else if (m_Config->m_LabelPos == CSegmentConfig::ePos_Inside) {
                gl.ColorC(color2.ContrastingColor());
            } else {
                gl.ColorC(m_Config->m_Label);
            }

            m_Context->TextOut(&font, label.c_str(), xM, yM,
                m_Config->m_LabelPos != CSegmentConfig::ePos_Side);
        }
    }

    // Draw selection
    if (IsSelected()) {
        if (m_HideLabel  &&
            m_Config->m_LabelPos == CSegmentConfig::ePos_Side) {
            m_Context->DrawSelection( TModelRect(range.GetFrom(), GetBottom(),
                                                 range.GetTo(), GetTop()) );
        } else {
            m_Context->DrawSelection(rcm);
        }
    }
}


void CSegmentGlyph::x_UpdateBoundingBox()
{
    _ASSERT(m_Context);

    IRender& gl = GetGl();

    TSeqRange range = GetRange();
    SetHeight(m_Config->m_BarHeight);
    SetWidth(range.GetLength());
    SetLeft(range.GetFrom());

    if ( !x_ShowLabel() ) return;

    if (m_Config->m_LabelPos == CSegmentConfig::ePos_Above) {
        SetHeight(GetHeight() + kVertLabelSpace + gl.TextHeight(&(m_Config->m_LabelFont)));
    } else if (m_Config->m_LabelPos == CSegmentConfig::ePos_Side) {
        bool neg_strand = (sequence::GetStrand(GetLocation()) == eNa_strand_minus);
        string label;
        GetLabel(label, CLabel::eContent);
        TModelUnit text_w = min(
            gl.TextWidth(&(m_Config->m_LabelFont), label.c_str()),
            m_Context->GetMaxLabelWidth(m_Config->m_LabelFont));
        text_w = m_Context->ScreenToSeq(text_w + kSideLabelSpace);
        SetWidth(GetWidth() + text_w);
        if ( !neg_strand ) {
            SetLeft(GetLeft() - text_w);
        }
    }
}


bool CSegmentGlyph::x_ShowLabel() const
{
    bool shown = false;
    if (!m_HideLabel  &&  m_Config->m_LabelPos != CSegmentConfig::ePos_NoLabel) {
        shown = true;
        if (m_Config->m_LabelPos != CSegmentConfig::ePos_Side) {
            TModelRange vis_r = m_Context->IntersectVisible(GetRange());
            if ( !m_Context->WillLabelFit(vis_r)) {
                shown = false;
            }
        }
    }
    return shown;
}


END_NCBI_SCOPE
