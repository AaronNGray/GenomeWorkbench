/*  $Id: clone_placement_glyph.cpp 43896 2019-09-16 17:50:00Z evgeniev $
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
 * Authors:  Mike DiCuccio, Liangshou Wu
 *
 * File Description:
 *    CClonePlacementGlyph -- utility class to arrange CSeq_feat objects in hierarchical
 *                (tree) order.
 */

#include <ncbi_pch.hpp>
#include <gui/widgets/seq_graphic/clone_placement_glyph.hpp>
#include <gui/widgets/seq_graphic/rendering_ctx.hpp>
#include <gui/opengl/irender.hpp>
#include <gui/objutils/obj_fingerprint.hpp>
#include <gui/objutils/gui_object_info_seq_feat.hpp>
#include <gui/objutils/tooltip.hpp>
#include <objmgr/util/sequence.hpp>
#include <objects/seqfeat/Clone_ref.hpp>
#include <objects/seqfeat/Clone_seq_set.hpp>
#include <objects/seqfeat/Clone_seq.hpp>
#include <objects/general/User_object.hpp>
#include <math.h>

BEGIN_NCBI_SCOPE
USING_SCOPE(objects);

/// vertical space between elements.
static const int kVertSpace = 2;
static const int kHorzSpace = 3;

//
// CClonePlacementGlyph::CClonePlacementGlyph()
//
CClonePlacementGlyph::CClonePlacementGlyph(const CMappedFeat& f)
    : m_Feature(f)
    , m_Location(&f.GetLocation())
    , m_HideLabel(false)
{}

CClonePlacementGlyph::CClonePlacementGlyph(const CMappedFeat& f, const CSeq_loc& loc)
    : m_Feature(f)
    , m_Location(&loc)
    , m_HideLabel(false)
{}


bool CClonePlacementGlyph::NeedTooltip(const TModelPoint& /*p*/, ITooltipFormatter& /*tt*/, string& /*t_title*/) const
{
    return true;
}


void CClonePlacementGlyph::GetTooltip(const TModelPoint& p, ITooltipFormatter& tt, string& t_title) const
{
    CScope& scope = GetMappedFeature().GetAnnot().GetScope();
    SConstScopedObject scoped_obj(&GetMappedFeature().GetMappedFeature(), &scope);
    CIRef<IGuiObjectInfo> gui_info(
        CreateObjectInterface<IGuiObjectInfo>(scoped_obj, NULL));

    if ( !gui_info ) return;

    CGuiObjectInfoSeq_feat* gui_info_feat =
        dynamic_cast<CGuiObjectInfoSeq_feat*>(gui_info.GetPointer());
    if (gui_info_feat) {
        gui_info_feat->SetLocation(*m_Location);
    }

    TSeqPos at_p = (TSeqPos)-1;
    if (p.X() >= 0) {
        at_p = (TSeqPos)p.X();
    }

    gui_info->GetToolTip(tt, t_title, at_p);
    gui_info->GetLinks(tt, false);
}


void CClonePlacementGlyph::GetHTMLActiveAreas(TAreaVector* p_areas) const
{
    CHTMLActiveArea area;
    CSeqGlyph::x_InitHTMLActiveArea(area);
    area.m_Signature = GetSignature();
    p_areas->push_back(area);
}


TSeqRange CClonePlacementGlyph::GetRange(void) const
{
    TSeqRange range;
    try { // watch out for mix loc with multiple seq-ids
        range = m_Location->GetTotalRange();
    } catch (CException&) {
        // get range from intervals
        ITERATE(TIntervals,  it,  m_Intervals) {
            range.CombineWith(*it);
        }
    }
    return range;
}


bool CClonePlacementGlyph::IsClickable() const
{
    return true;
}

void CClonePlacementGlyph::SetHideLabel(bool b)
{
    m_HideLabel = b;
}


bool CClonePlacementGlyph::HasSideLabel() const
{
    return !m_HideLabel  &&  m_Config->m_LabelPos == CFeatureParams::ePos_Side;
}



const objects::CSeq_loc& CClonePlacementGlyph::GetLocation(void) const
{
    return *m_Location;
}


CConstRef<CObject> CClonePlacementGlyph::GetObject(TSeqPos) const
{
    return CConstRef<CObject>(&m_Feature.GetOriginalFeature());
}


void  CClonePlacementGlyph::GetObjects(vector<CConstRef<CObject> >& objs) const
{
    objs.push_back( CConstRef<CObject>(&m_Feature.GetOriginalFeature()) );
}


bool CClonePlacementGlyph::HasObject(CConstRef<CObject> obj) const
{
    return &m_Feature.GetOriginalFeature() == obj.GetPointer();
}


string CClonePlacementGlyph::GetSignature() const
{
    return CObjFingerprint::GetFeatSignature(
        GetMappedFeature().GetMappedFeature(),
        GetLocation(), &m_Context->GetScope(), m_Feature.GetAnnot());
}


const CClonePlacementGlyph::TIntervals& CClonePlacementGlyph::GetIntervals(void) const
{
    return m_Intervals;
}


string CClonePlacementGlyph::GetLabel() const
{
    string label = kEmptyStr;
    const CSeqFeatData& data = GetFeature().GetData();
    if (data.IsUser()) {
        const CUser_object& user = data.GetUser();
        if (user.HasField("name")) {
           label = user.GetField("name").GetData().GetStr();
        }
    }

    if (label.empty()) {
        CLabel::GetLabel(GetFeature(), &label, CLabel::eContent,
            &m_Context->GetScope());
    }

    return label;
}


GLubyte dense_vert_lines[] = {
    0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc,
    0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc,
    0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc,
    0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc,
    0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc,
    0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc,
    0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc,
    0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc,
    0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc,
    0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc,
    0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc,
    0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc,
    0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc,
    0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc,
    0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc,
    0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc
};


GLubyte dense_horz_lines[] = {
    0xff, 0xff, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00,
    0xff, 0xff, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00,
    0xff, 0xff, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00,
    0xff, 0xff, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00,
    0xff, 0xff, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00,
    0xff, 0xff, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00,
    0xff, 0xff, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00,
    0xff, 0xff, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00,
    0xff, 0xff, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00,
    0xff, 0xff, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00,
    0xff, 0xff, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00,
    0xff, 0xff, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00,
    0xff, 0xff, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00,
    0xff, 0xff, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00,
    0xff, 0xff, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00,
    0xff, 0xff, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00
};


GLubyte no_fill[] = {
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};


GLubyte dense_diag_lines[] = {
    0x00, 0x7f, 0xff, 0x00, 0x00, 0x3f, 0xff, 0x80,
    0x00, 0x1f, 0xff, 0xc0, 0x00, 0x0f, 0xff, 0xe0,
    0x00, 0x07, 0xff, 0xf0, 0x00, 0x03, 0xff, 0xf8,
    0x00, 0x01, 0xff, 0xfc, 0x00, 0x00, 0xff, 0xfe,
    0x00, 0x00, 0x7f, 0xff, 0x80, 0x00, 0x3f, 0xff,
    0xc0, 0x00, 0x1f, 0xff, 0xe0, 0x00, 0x0f, 0xff,
    0xf0, 0x00, 0x07, 0xff, 0xf8, 0x00, 0x03, 0xff,
    0xfc, 0x00, 0x01, 0xff, 0xfe, 0x00, 0x00, 0xff,
    0xff, 0x00, 0x00, 0x7f, 0xff, 0x80, 0x00, 0x3f,
    0xff, 0xc0, 0x00, 0x1f, 0xff, 0xe0, 0x00, 0x0f,
    0xff, 0xf0, 0x00, 0x07, 0xff, 0xf8, 0x00, 0x03,
    0xff, 0xfc, 0x00, 0x01, 0xff, 0xfe, 0x00, 0x00,
    0x7f, 0xff, 0x00, 0x00, 0x3f, 0xff, 0x80, 0x00,
    0x1f, 0xff, 0xc0, 0x00, 0x0f, 0xff, 0xe0, 0x00,
    0x07, 0xff, 0xf0, 0x00, 0x03, 0xff, 0xf8, 0x00,
    0x01, 0xff, 0xfc, 0x00, 0x00, 0xff, 0xfe, 0x00
};

GLubyte dense_vert_lines2[] = {
    0x33, 0x33, 0x33, 0x33, 0x33, 0x33, 0x33, 0x33,
    0x33, 0x33, 0x33, 0x33, 0x33, 0x33, 0x33, 0x33,
    0x33, 0x33, 0x33, 0x33, 0x33, 0x33, 0x33, 0x33,
    0x33, 0x33, 0x33, 0x33, 0x33, 0x33, 0x33, 0x33,
    0x33, 0x33, 0x33, 0x33, 0x33, 0x33, 0x33, 0x33,
    0x33, 0x33, 0x33, 0x33, 0x33, 0x33, 0x33, 0x33,
    0x33, 0x33, 0x33, 0x33, 0x33, 0x33, 0x33, 0x33,
    0x33, 0x33, 0x33, 0x33, 0x33, 0x33, 0x33, 0x33,
    0x33, 0x33, 0x33, 0x33, 0x33, 0x33, 0x33, 0x33,
    0x33, 0x33, 0x33, 0x33, 0x33, 0x33, 0x33, 0x33,
    0x33, 0x33, 0x33, 0x33, 0x33, 0x33, 0x33, 0x33,
    0x33, 0x33, 0x33, 0x33, 0x33, 0x33, 0x33, 0x33,
    0x33, 0x33, 0x33, 0x33, 0x33, 0x33, 0x33, 0x33,
    0x33, 0x33, 0x33, 0x33, 0x33, 0x33, 0x33, 0x33,
    0x33, 0x33, 0x33, 0x33, 0x33, 0x33, 0x33, 0x33,
    0x33, 0x33, 0x33, 0x33, 0x33, 0x33, 0x33, 0x33
};

GLubyte dense_vert_lines3[] = {
    0x03, 0xff, 0x03, 0xff, 0x03, 0xff, 0x03, 0xff,
    0x03, 0xff, 0x03, 0xff, 0x03, 0xff, 0x03, 0xff,
    0x03, 0xff, 0x03, 0xff, 0x03, 0xff, 0x03, 0xff,
    0x03, 0xff, 0x03, 0xff, 0x03, 0xff, 0x03, 0xff,
    0x03, 0xff, 0x03, 0xff, 0x03, 0xff, 0x03, 0xff,
    0x03, 0xff, 0x03, 0xff, 0x03, 0xff, 0x03, 0xff,
    0x03, 0xff, 0x03, 0xff, 0x03, 0xff, 0x03, 0xff,
    0x03, 0xff, 0x03, 0xff, 0x03, 0xff, 0x03, 0xff,
    0x03, 0xff, 0x03, 0xff, 0x03, 0xff, 0x03, 0xff,
    0x03, 0xff, 0x03, 0xff, 0x03, 0xff, 0x03, 0xff,
    0x03, 0xff, 0x03, 0xff, 0x03, 0xff, 0x03, 0xff,
    0x03, 0xff, 0x03, 0xff, 0x03, 0xff, 0x03, 0xff,
    0x03, 0xff, 0x03, 0xff, 0x03, 0xff, 0x03, 0xff,
    0x03, 0xff, 0x03, 0xff, 0x03, 0xff, 0x03, 0xff,
    0x03, 0xff, 0x03, 0xff, 0x03, 0xff, 0x03, 0xff,
    0x03, 0xff, 0x03, 0xff, 0x03, 0xff, 0x03, 0xff
};

void CClonePlacementGlyph::x_Draw() const
{
    if (GetHeight() == 0) {
        return;
    }

    _ASSERT(m_Context);

    IRender& gl = GetGl();

    TModelUnit bar_h = 0.0, head_h = 0.0;
    x_GetBarSize(bar_h, head_h);

    TModelUnit base    = GetBottom();
    TModelUnit line_center = base - head_h * 0.5f;
    TModelUnit line_b = line_center + bar_h * 0.5;
    TModelUnit line_t = line_center - bar_h * 0.5;

    TCloneEnds clone_ends;
    TCloneEnds prototype_ends;
    // support type for all other non-prototype ends
    ESupportType non_prototype_st = x_GetSupportTypeForNonPrototypeEnds();
    x_GetCloneEnds(clone_ends, prototype_ends);

    EConcordancy    concordancy = eCCNotSet;
    EUniqueness     unique = eUniqueNotSet;
    CClone_ref_Base::TPlacement_method placement = CClone_ref_Base::ePlacement_method_end_seq;
    const CSeqFeatData& data = GetFeature().GetData();
    if (data.IsClone()) {
        const CSeqFeatData::TClone& clone = data.GetClone();
        if (clone.IsSetConcordant()) {
            concordancy = clone.GetConcordant() ? eConcordant : eDiscordant;
        }
        if (clone.IsSetUnique()) {
            unique = clone.GetUnique() ? eUnique : eMultiple;
        }
        if (clone.IsSetPlacement_method()) {
            placement = clone.GetPlacement_method();
        }
    } else if (data.IsUser()) { // old clone feature stored as user_type
        const CUser_object& user = data.GetUser();
        CConstRef<CUser_field> user_field = user.GetFieldRef("concordant");
        if (user_field) {
            concordancy = user_field->GetData().GetBool() ? eConcordant : eDiscordant;
        }
    }

    CRgbaColor base_color = m_Config->m_UnknownConcordancyColor;
    if (concordancy == eConcordant) {
        base_color = m_Config->m_ConcordantColor;
    } else if (concordancy == eDiscordant){
        base_color = m_Config->m_DiscordantColor;
    }

    TSeqRange total_range = x_GetTotalRange();
    TModelUnit pix_size = m_Context->ScreenToSeq(1.0);
    CRgbaColor boundary_color = base_color;
    boundary_color.Darken(0.3f);

    CRgbaColor combined_plmt_color = m_Config->m_CombinedPlacementColor;

    // draw highlighting background for some
    CRgbaColor highlight_color = m_Config->m_HighlightColor;
    highlight_color.SetAlpha(0.3f);
    if (CClone_ref_Base::ePlacement_method_end_seq_insert_alignment == placement ||
        eNonsupporting == non_prototype_st ||
        eMixed == non_prototype_st)
    {
        TModelUnit padding_y = 1.0;
        TModelUnit padding_x = padding_y * pix_size;

        if (CClone_ref_Base::ePlacement_method_end_seq_insert_alignment == placement) {
            if (eNonsupporting == non_prototype_st || eMixed == non_prototype_st) {
                gl.ColorC(highlight_color);
                m_Context->DrawQuad(GetLeft() - padding_x, GetTop() - padding_y, GetRight() + padding_x, GetBottom() + padding_y);
                gl.Enable(GL_POLYGON_STIPPLE);
                gl.PolygonStipple(dense_diag_lines);
            }
            gl.ColorC(combined_plmt_color);
        }
        else {
            gl.ColorC(highlight_color);
        }

        m_Context->DrawQuad(GetLeft() - padding_x, GetTop() - padding_y, GetRight() + padding_x, GetBottom() + padding_y);
        gl.Disable(GL_POLYGON_STIPPLE);
    }


    if (total_range.GetLength() < pix_size) {  // less than a pixel size
        m_Context->Draw3DQuad(total_range.GetFrom(), line_t,
            total_range.GetTo(), line_b, base_color);
    } else {
        TModelUnit size = head_h * 0.5;
        TModelUnit head_len = m_Context->ScreenToSeq(size);

        // draw connection line
        if (prototype_ends.size() == 2) {
            if (unique == eMultiple) {
                gl.PolygonStipple(dense_vert_lines2);
                gl.Enable(GL_POLYGON_STIPPLE);
            } else if (unique == eUniqueNotSet) {
                gl.PolygonStipple(dense_vert_lines3);
                gl.Enable(GL_POLYGON_STIPPLE);
            }
            gl.ColorC(boundary_color);
            TModelUnit x1 = prototype_ends[0].m_Range.GetToOpen();
            TModelUnit x2 = prototype_ends[1].m_Range.GetFrom();
            if (prototype_ends[0].m_Strand == eNegative  &&
                head_len > prototype_ends[0].m_Range.GetLength()) {
                x1 = prototype_ends[0].m_Range.GetFrom() + head_len;
            }
            if (prototype_ends[1].m_Strand == ePositive  &&
                head_len > prototype_ends[1].m_Range.GetLength()) {
                x2 = prototype_ends[1].m_Range.GetToOpen() - head_len;
            }

            m_Context->DrawQuad(x1, line_center - 1, x2, line_center + 1);
            gl.Disable(GL_POLYGON_STIPPLE);
        }

        // draw the clone ends
        ITERATE (TCloneEnds, iter, prototype_ends) {
            TModelUnit from = iter->m_Range.GetFrom();
            TModelUnit to   = iter->m_Range.GetToOpen();

            CGlAttrGuard AttrGuard(GL_LINE_BIT);
            gl.Disable(GL_LINE_SMOOTH);

            EUniqueness end_uniqueness = iter->m_Uniqueness;
            if (((CClone_ref_Base::ePlacement_method_end_seq != placement) || end_uniqueness == eUniqueNotSet) && prototype_ends.size() == 1) {
                end_uniqueness = unique;
            }

            gl.ColorC(base_color);
            x_SetPolygonStipple(end_uniqueness);

            gl.ShadeModel(GL_SMOOTH);
            if (iter->m_Strand == eStrandNotSet) {
                m_Context->DrawQuad(from, line_b, to, line_t);
                gl.ColorC(boundary_color);
                m_Context->DrawRect(from, line_b, to, line_t);
            } else {
                bool forward = iter->m_Strand == ePositive;
                TModelUnit x3 =  forward ? to : from;
                TModelUnit x2 = x3 - (forward ? head_len : -head_len);
                TModelUnit x1 = forward ? from : to;
                if (iter->m_Range.GetLength() < head_len) {
                    x1 = x2;
                }

                m_Context->DrawArrow(x1, x2, x3, line_center,
                    - (forward ? 0.5 : -0.5) * bar_h,
                    - (forward ? 0.5 : -0.5) * head_h);
                gl.ColorC(boundary_color);
                m_Context->DrawArrowBoundary(x1, x2, x3, line_center,
                    0.5 * bar_h, 0.5 * head_h);
            }
            gl.ShadeModel(GL_FLAT);
            gl.Disable(GL_POLYGON_STIPPLE);
            gl.PolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        }

    }

    // draw label
    if (x_ShowLabel()) {
        string label = GetLabel();
        const CGlTextureFont& font = m_Config->m_LabelFont;
        TModelUnit label_w = gl.TextWidth(&font, label.c_str());
        TModelRange r = m_Context->IntersectVisible(this);
        if (m_Config->m_LabelPos == CFeatureParams::ePos_Above) {
            // top label
            TModelUnit widthP = m_Context->SeqToScreen(r.GetLength());
            if (label_w > widthP) {
                label = font.Truncate(label.c_str(), widthP);
            }

            if (IsSelected()) {
                gl.ColorC(m_Context->GetSelLabelColor());
            } else {
                gl.ColorC(m_Config->m_LabelColor);
            }

            TModelUnit xM = (r.GetFrom() + r.GetTo()) * 0.5;
            TModelUnit yM = line_center - kVertSpace * 2;
            if (clone_ends.size() == 1) {
                yM = base - head_h;
            } else {
                TModelRange in_range((TModelUnit)clone_ends[0].m_Range.GetToOpen(),
                    (TModelUnit)clone_ends[1].m_Range.GetFrom());
                in_range = m_Context->IntersectVisible(in_range);
                if (label_w > m_Context->SeqToScreen(in_range.GetLength())) {
                    yM = base - head_h;
                } else {
                    xM = (in_range.GetFrom() + in_range.GetTo() ) * 0.5;
                }
            }
            m_Context->TextOut(&font, label.c_str(), xM, yM, true);
        } else {  // side label
            TModelUnit max_w = m_Context->GetMaxLabelWidth(font);
            if (label_w > max_w) {
                label_w = max_w;
                label = font.Truncate(label.c_str(), label_w);
            }
            label_w = m_Context->ScreenToSeq(label_w);
            if (r.GetLength() < label_w) {
                return;
            }

            TModelUnit label_b = line_center + gl.TextHeight(&font) * 0.5;
            if (GetLeft() < 0) {
                label_b = GetTop() + 1 + gl.TextHeight(&font);
            }

            TModelUnit label_x = r.GetFrom();
            if (r.GetFrom() > GetLeft()) {
                gl.Color3f(1.0f, 1.0f, 1.0f);
                m_Context->DrawBackground(
                    TModelRect(label_x, label_b + 1, label_x + label_w,
                    label_b - gl.TextHeight(&font) - 1), 0);
            }

            gl.ColorC(IsSelected() ?
                m_Context->GetSelLabelColor() : m_Config->m_LabelColor);
            m_Context->TextOut(&font, label.c_str(), label_x,
                label_b - 1.0, false, true);
        }
    }

    if (IsSelected()) {
        TModelRect rcm  = GetModelRect();
        m_Context->DrawSelection(rcm);
    }
}

// The grey highlight is used if seq-ids in non - supporting or supports - other are different than seq - ids in prototype OR supporting.
CClonePlacementGlyph::ESupportType CClonePlacementGlyph::x_GetSupportTypeForNonPrototypeEnds() const
{
    ESupportType result = eNoNonprototypeEnd;

    const CSeqFeatData& data = GetFeature().GetData();
    if (!data.IsClone())
        return result;

    const CSeqFeatData::TClone& clone = data.GetClone();
    if (!clone.CanGetClone_seq())
        return result;

    TStrSet supporting;
    typedef map<int, TStrVector> TEndMap;
    TEndMap end_map;
    const CClone_ref::TClone_seq::Tdata& ends = clone.GetClone_seq().Get();
    ITERATE(CClone_ref::TClone_seq::Tdata, e_iter, ends) {
        const CClone_seq& seq = **e_iter;
        if (!seq.CanGetSeq() || !seq.CanGetSupport())
            continue;

        const CSeq_id* id = seq.GetSeq().GetId();
        if (!id)
            continue;

        if ((CClone_seq::eSupport_prototype == seq.GetSupport()) || (CClone_seq::eSupport_supporting == seq.GetSupport())) {
            supporting.insert(id->GetSeqIdString(true));
            continue;
        }

        end_map[seq.GetSupport()].push_back(id->GetSeqIdString(true));
    }

    if (end_map.count(CClone_seq::eSupport_non_supporting) > 0) {
        if (!x_MatchIds(supporting, end_map[CClone_seq::eSupport_non_supporting]))
            result = (ESupportType)(result | eNonsupporting);
    }

    if (end_map.count(CClone_seq::eSupport_supports_other) > 0) {
        if (!x_MatchIds(supporting, end_map[CClone_seq::eSupport_supports_other]))
            result = (ESupportType)(result | eNonsupporting);
    }

    return result;
}

bool CClonePlacementGlyph::x_MatchIds(const TStrSet &supportingIds, const TStrVector &nonSupportingIds) const
{
    TStrVector::const_iterator itId;
    for (itId = nonSupportingIds.begin(); itId != nonSupportingIds.end(); ++itId) {
        if (supportingIds.count(*itId) == 0)
            return false;
    }
    return true;
}

void CClonePlacementGlyph::x_SetPolygonStipple(EUniqueness uniqueness) const
{
    IRender& gl = GetGl();

    switch (uniqueness) {
    case eVirtual:
        gl.Enable(GL_POLYGON_STIPPLE);
        gl.PolygonStipple(no_fill);
        break;
    case eMultiple:
        gl.Enable(GL_POLYGON_STIPPLE);
        gl.PolygonStipple(dense_vert_lines);
        break;
    case eUniqueNotSet:
        gl.Enable(GL_POLYGON_STIPPLE);
        gl.PolygonStipple(dense_horz_lines);
        break;
    default:
        break;
    }
}

void CClonePlacementGlyph::x_UpdateBoundingBox()
{
    _ASSERT(m_Context);

    IRender& gl = GetGl();

    TModelUnit bar_h = 0.0, head_h = 0.0;
    x_GetBarSize(bar_h, head_h);

    TSeqRange total_range = x_GetTotalRange();
    SetHeight(head_h);
    SetWidth(total_range.GetLength());
    SetLeft(total_range.GetFrom());

    TCloneEnds clone_ends;
    TCloneEnds prototype_ends;
    // support type for all other non-prototype ends
    x_GetCloneEnds(clone_ends, prototype_ends);

    // in case the triangle's screen size is larger than the
    // is clone's start end size (screen size) and the end strand
    // is forward, we need to add an extra space to avoid overlapping
    if (prototype_ends[0].m_Strand == ePositive) {
        TSeqPos end_len = prototype_ends[0].m_Range.GetLength();
        TModelUnit extra_space =
            m_Context->ScreenToSeq(head_h * 0.5) - end_len;
        if (extra_space > 0) {
            SetWidth(GetWidth() + extra_space);
            SetLeft(GetLeft() - extra_space);
        }
    }
    size_t end_num = prototype_ends.size();
    if (end_num > 1  &&  prototype_ends[end_num - 1].m_Strand == eNegative) {
        TSeqPos end_len = prototype_ends[end_num - 1].m_Range.GetLength();
        TModelUnit extra_space =
            m_Context->ScreenToSeq(head_h * 0.5) - end_len;
        if (extra_space > 0) {
            SetWidth(GetWidth() + extra_space);
        }
    }

    if ( !x_ShowLabel() )  return;

    string label = GetLabel();
    const CGlTextureFont& font = m_Config->m_LabelFont;

    if (m_Config->m_LabelPos == CFeatureParams::ePos_Above) {
        // top label, preserve label vertical space
        if (prototype_ends.size() == 2) {
            TModelUnit text_w =
                m_Context->ScreenToSeq(gl.TextWidth(&font, label.c_str()));

            TModelRange in_range((TModelUnit)prototype_ends[0].m_Range.GetToOpen(),
                (TModelUnit)prototype_ends[1].m_Range.GetFrom());
            in_range = m_Context->IntersectVisible(in_range);
            if (text_w < in_range.GetLength()) {
                SetHeight(GetHeight() + gl.TextHeight(&font) +
                    kVertSpace * 2 - GetHeight() * 0.5);
            } else {
                SetHeight(GetHeight() + gl.TextHeight(&font));
            }
        } else {
            SetHeight(GetHeight() + gl.TextHeight(&font));
        }
    } else { // side label
        TModelUnit text_w =
            min(gl.TextWidth(&font, label.c_str()), m_Context->GetMaxLabelWidth(font));
        text_w = m_Context->ScreenToSeq(text_w + kHorzSpace);
        SetWidth(GetWidth() + text_w);
        SetLeft(GetLeft() - text_w);

        TModelRange vis_r = m_Context->IntersectVisible(this);

        if (GetLeft() < 0  &&  vis_r.GetLength() >= text_w) {
            // can't fit, move label above the bar
            SetHeight(GetHeight() + gl.TextHeight(&font) + kVertSpace + 1);
        }
    }
}


bool CClonePlacementGlyph::x_ShowLabel() const
{
    bool shown = false;
    if (m_Config->m_LabelPos != CFeatureParams::ePos_NoLabel) {
        if (m_Config->m_LabelPos == CFeatureParams::ePos_Side) {
            shown = !m_HideLabel;
        } else { // can be either ePos_Above or ePos_Inside
            TModelRange r = m_Context->IntersectVisible(this);
            if (m_Context->WillLabelFit(r)) {
                shown = !m_HideLabel  ||  IsSelected();
            }
        }
    }

    return shown;
}


TSeqRange CClonePlacementGlyph::x_GetTotalRange() const
{
    return GetLocation().GetTotalRange();
}


void CClonePlacementGlyph::x_GetCloneEnds(TCloneEnds& clone_ends,
                                          TCloneEnds& prototype_ends) const
{
    const CSeq_id* main_id = GetLocation().GetId();
    const CSeqFeatData& data = GetFeature().GetData();
    if (data.IsClone()) {
        const CSeqFeatData::TClone& clone = data.GetClone();
        if (clone.IsSetClone_seq()) {
            const CClone_ref::TClone_seq::Tdata& ends = clone.GetClone_seq().Get();
            ITERATE (CClone_ref::TClone_seq::Tdata, e_iter, ends) {
                const CClone_seq& seq = **e_iter;

                if (!main_id->Match(*seq.GetLocation().GetId()))
                    continue;

                SCloneEnd end;
                end.m_Range = seq.GetLocation().GetTotalRange();
                if (seq.GetLocation().IsSetStrand()) {
                    end.m_Strand =
                        seq.GetLocation().GetStrand() == eNa_strand_minus ? eNegative : ePositive;
                } else if (seq.CanGetSeq()  &&  seq.GetSeq().IsSetStrand()) {
                    end.m_Strand =
                        seq.GetSeq().GetStrand() == eNa_strand_minus ? eNegative : ePositive;
                }
                // check if there is any fake end
                if (seq.IsSetConfidence()) {
                    switch (seq.GetConfidence()) {
                        case CClone_seq::eConfidence_virtual:
                            end.m_Uniqueness = eVirtual;
                            break;
                        case CClone_seq::eConfidence_unique:
                            end.m_Uniqueness = eUnique;
                            break;
                        case CClone_seq::eConfidence_multiple:
                            end.m_Uniqueness = eMultiple;
                            break;
                        default:
                            break;
                    }
                }

                clone_ends.push_back(end);

                if (!seq.CanGetSupport())
                    continue;

                if (CClone_seq::eSupport_prototype == seq.GetSupport()) {
                    prototype_ends.push_back(end);
                    continue;
                }
            }
        }
    }

    // in case of no clone end or it is user type feature
    if (clone_ends.empty()) {
        CSeq_loc_CI loc_it(GetLocation());
        size_t num_loc = 0;
        for (; loc_it  &&  num_loc < 2; ++loc_it, ++num_loc) {
            SCloneEnd end;
            end.m_Range = loc_it.GetRange();
            if (loc_it.IsSetStrand()) {
                end.m_Strand = loc_it.GetStrand() == eNa_strand_minus ? eNegative : ePositive;
            }
            clone_ends.push_back(end);
            prototype_ends.push_back(end);
        }
    }

    if (!clone_ends.empty() &&  data.IsUser()) { // old clone feature stored as user_type
        const CUser_object& user = data.GetUser();
        if (user.HasField("end1_fake")  &&
            user.GetField("end1_fake").GetData().GetBool()) {
            clone_ends[0].m_Uniqueness = eVirtual;
        }
        if (clone_ends.size() > 1  &&  user.HasField("end2_fake")  &&
            user.GetField("end2_fake").GetData().GetBool()) {
            clone_ends[1].m_Uniqueness = eVirtual;
        }
    }

    if (prototype_ends.empty()) {
        // if there are no prototype ends, use the feature range as "virtual" prototype ends
        CSeq_loc_CI loc_it(GetLocation());
        size_t num_loc = 0;
        for (; loc_it  &&  num_loc < 2; ++loc_it, ++num_loc) {
            SCloneEnd end;
            end.m_Range = loc_it.GetRange();
            if (loc_it.IsSetStrand()) {
                end.m_Strand = loc_it.GetStrand() == eNa_strand_minus ? eNegative : ePositive;
            }
            prototype_ends.push_back(end);
        }
    }
}


void CClonePlacementGlyph::x_GetBarSize(TModelUnit& bar_h,
                                        TModelUnit& head_h) const
{
    bar_h = m_Config->m_BarHeight;
    head_h = m_Config->m_HeadHeight;
    if (m_Context->IsOverviewMode()  &&  m_HideLabel) {
        bar_h = floor(bar_h * m_Config->m_OverviewFactor);
        head_h = floor(head_h * m_Config->m_OverviewFactor);
    }
}


END_NCBI_SCOPE
