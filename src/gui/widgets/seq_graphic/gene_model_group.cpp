/*  $Id: gene_model_group.cpp 44431 2019-12-17 22:16:34Z evgeniev $
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
*/

#include <ncbi_pch.hpp>
#include <gui/gui.hpp>
#include <gui/widgets/seq_graphic/gene_model_group.hpp>
#include <gui/widgets/seq_graphic/feature_glyph.hpp>
#include <gui/widgets/seq_graphic/rendering_ctx.hpp>
#include <gui/opengl/irender.hpp>
#include <gui/objutils/tooltip.hpp>
#include <gui/objutils/obj_fingerprint.hpp>
#include <gui/objutils/gui_object_info.hpp>
#include <gui/objutils/interface_registry.hpp>
#include <gui/objutils/gui_object_info_seq_feat.hpp>
#include <gui/objutils/create_params_seq_feat_group.hpp>
#include <objmgr/util/sequence.hpp>
#include <corelib/ncbi_limits.hpp>


BEGIN_NCBI_SCOPE
USING_SCOPE(objects);

static const int kVertSpace = 2;
static const int kLabelSpace = 5;


// #define DEBUG_INFO_ON_SCREEN 1

///////////////////////////////////////////////////////////////////////////////
/// CGeneGroup implementations
///////////////////////////////////////////////////////////////////////////////

void CGeneGroup::Update(bool layout_only)
{
    if (m_ShowGene == eGene_Shown) {
        CLayoutGroup::Update(layout_only);
    } else {
        TModelUnit min_l = DBL_MAX;
        TModelUnit max_w = 0.0;
        TObjectList::iterator iter = SetChildren().begin();
        CFeatGlyph* gene = dynamic_cast<CFeatGlyph*>(iter->GetPointer());

        // skip the gene feature
        ++iter;
        while (iter != SetChildren().end()) {
            (*iter)->Update(layout_only);
            min_l = min(min_l, (*iter)->GetLeft());
            max_w = max(max_w, (*iter)->GetWidth());
            ++iter;
        }

        // make sure the horizontal space reserved for gene feature
        // won't exceed the group space
        gene->SetLeft(min_l);
        gene->SetWidth(max_w);
        switch (m_ShowGene) {
            case eGene_Shown:
                gene->SetHeight(gene->GetConfig()->m_BarHeight);
                break;
            case eGene_ShowLabelOnly:
                if (x_ShowGeneLabel())
                    gene->SetHeight(gene->GetConfig()->m_BarHeight);
                else
                    gene->SetHeight(0.0);
                break;
            case eGene_Hidden:
                gene->SetHeight(0.0);
                break;
        }
        x_UpdateBoundingBox();
    }
}


TSeqRange CGeneGroup::GetRange(void) const
{
    return GetLocation().GetTotalRange();
}


bool CGeneGroup::Accept(IGlyphVisitor* visitor)
{
    bool cont = visitor->Visit(this);
    TObjectList::iterator iter = SetChildren().begin();
    ++iter;

    while (cont  &&  iter != SetChildren().end()) {
        cont = (*iter)->Accept(visitor);
        ++iter;
    }
    return cont;

}

void CGeneGroup::GetHTMLActiveAreas(TAreaVector *p_areas) const
{
    if ((m_ShowGene == eGene_ShowLabelOnly) && (2 == GetChildren().size())) {
        CConstRef<CSeqGlyph> child1 = GetChild(1);
        const CLinkedFeatsGroup* linked_feats = dynamic_cast<const CLinkedFeatsGroup*>(child1.GetPointer());
        const CLayoutGroup* group = dynamic_cast<const CLayoutGroup*>(child1.GetPointer());
        if (!linked_feats && !group && !child1->IsSelected()) {
            if (x_isDrawn()) {
                try {
                    // Override the active areas to include the gene tooltip along with the merged feature tooltip
                    if (x_ShowGeneLabel())
                        GetChild(0)->GetHTMLActiveAreas(p_areas);  // Gene's label
                    GetChild(0)->GetHTMLActiveAreas(p_areas);  // Gene and
                    child1->GetHTMLActiveAreas(p_areas);  // merged feature
                    // Correct the coordinates of the second gene's active area to match the cooridinates of the merged feature
                    TAreaVector::reverse_iterator it_gene = p_areas->rbegin();
                    ++it_gene;
                    it_gene->m_Bounds = p_areas->back().m_Bounds;
                }
                catch (CException& e) {
                    LOG_POST(Error
                        << "CGeneGroup::GetHTMLActiveAreas() failure on getting HTML active areas, error: "
                        << e.GetMsg());
                }
                return;
            }
        }
    }

    CLayoutGroup::GetHTMLActiveAreas(p_areas);
}

CConstRef<CObject> CGeneGroup::GetObject(TSeqPos pos) const
{
    return x_GetFirstFeat()->GetObject(pos);
}


void CGeneGroup::GetObjects(vector<CConstRef<CObject> >& objs) const
{
    x_GetFirstFeat()->GetObjects(objs);
}


bool CGeneGroup::HasObject(CConstRef<CObject> obj) const
{
    const CFeatGlyph* gene = x_GetFirstFeat();
    if (gene->HasObject(obj)) {
        return true;
    }

    return false;
}


const objects::CSeq_loc& CGeneGroup::GetLocation() const
{
    // delegate gene feature
    const CFeatGlyph* gene = x_GetFirstFeat();
    return gene->GetLocation();
}


string CGeneGroup::GetSignature() const
{
    const CFeatGlyph* gene = x_GetFirstFeat();
    return gene->GetSignature();
}


const CGeneGroup::TIntervals& CGeneGroup::GetIntervals() const
{
    // delegate the gene feature
    // use gene feature's intervals
    const CFeatGlyph* gene = x_GetFirstFeat();
    return gene->GetIntervals();
}


void CGeneGroup::SetShowGene(bool shown)
{
    if ( !shown ) {
        m_ShowGene = eGene_ShowLabelOnly;

        HideMaster(false);
        // if there is only one children other than gene itself, and
        // the child is rna and cds group, and the child group has more
        // than 2 children, we also hide the label.
        if (GetChildrenNum() == 2) {
            CRef<CSeqGlyph> child1 = GetChild(1);
            const CLinkedFeatsGroup* group =
                dynamic_cast<const CLinkedFeatsGroup*>(child1.GetPointer());
            if (group  &&  group->GetChildren().size() > 2) {
                m_ShowGene = eGene_Hidden;
                SetAsMaster(GetChild(0));
                HideMaster();
            }
        }
    } else {
        m_ShowGene = eGene_Shown;
        HideMaster(false);
    }
}


void CGeneGroup::x_Draw() const
{
    if ((GetLeft() >= 0) && m_Context->IntersectVisible(GetRange()).Empty())
        return;

    IRender& gl = GetGl();

    gl.PushMatrix();
    gl.Translated(0.0, GetTop(), 0.0f);
    if (!m_Context->IsSkipFrames())
        x_DrawBoundary();
#ifdef DEBUG_INFO_ON_SCREEN
    {
        CGlTextureFont font;
        gl.ColorC(CRgbaColor());
        font.SetFontFace(CGlTextureFont::FaceFromString("Helvetica"));
        font.SetFontSize(10);
        string sDebugInfo("CGeneGroup<" + NStr::NumericToString(GetLevel()) + ">/" + NStr::IntToString(x_isDrawn()) + "/[" + NStr::IntToString(GetChildrenNum()) + "]");
        m_Context->TextOut(&font, sDebugInfo.c_str(), GetLeft() - 30 > 0 ? GetLeft() - 30 : 10, GetTop(), true, true);
    }
#endif
    TObjectList::const_iterator iter = GetChildren().begin();
    bool gene_highlighted = false;
    if (m_ShowGene != eGene_Shown) {
        const CFeatGlyph* gene =
            dynamic_cast<const CFeatGlyph*>(iter->GetPointer());

        if (gene) {
            if ((m_ShowGene == eGene_ShowLabelOnly) && x_ShowGeneLabel()) {
                CConstRef<CFeatureParams> conf = gene->GetConfig();
                string gene_label;
                gene->GetLabel(gene_label, CLabel::eContent);

                // truncate the label if it exceeds the group width
                TModelUnit width = m_Context->SeqToScreen(GetWidth());
                const CGlTextureFont& font = conf->m_LabelFont;
                if (gl.TextWidth(&font, gene_label.c_str()) > width) {
                    gene_label = font.Truncate(gene_label.c_str(), width);
                }

                TModelUnit y = x_GetLabelHeight();
                TModelUnit x = GetLeft() + GetWidth() * 0.5;
                gl.ColorC(conf->m_LabelColor);
                m_Context->TextOut(&font, gene_label.c_str(), x, y, true);
            }

            // If the gene is a pseudo gene, draw background to indicate
            // this is a pseudo gene group.
            if (gene->GetFeature().IsSetPseudo()  &&
                gene->GetFeature().GetPseudo()) {

                CRgbaColor color = gene->GetConfig()->m_fgColor;
                color.SetAlpha(0.1f);
                CRgbaColor dcolor = color;
                dcolor.Darken(0.7f);
				CRgbaColor lcolor = color;
                lcolor.Lighten(0.5f);

                m_Context->DrawPseudoBar(GetLeft(),
                    0.0, GetRight(), GetHeight() - 1.0, lcolor, dcolor);
                color.SetAlpha(0.18f);
                gl.ColorC(color);
                m_Context->DrawQuad(GetLeft(), 0.0,
                    GetRight(), GetHeight() - 1.0);
            }

            gene_highlighted = gene->IsHighlighted();
        }

        // skip the gene feature
        ++iter;
    }
    while (iter != GetChildren().end()) {
        (*iter)->Draw();
        ++iter;
    }
    gl.PopMatrix();

    if (gene_highlighted) {
        // If gene feature is not shown and its is highlighted,
        // we need to highlight the whole group
        CSeqGlyph::x_DrawHighlight();
    }
}

bool CGeneGroup::x_ShowGeneLabel() const
{
    const CFeatGlyph* gene = dynamic_cast<const CFeatGlyph*>(GetChildren().begin()->GetPointer());

    CConstRef<CFeatureParams> conf = gene->GetConfig();
    string gene_label;
    gene->GetLabel(gene_label, CLabel::eContent);

    IRender& gl = GetGl();

    // truncate the label if it exceeds the group width
    TModelUnit width = m_Context->SeqToScreen(gene->GetWidth());
    const CGlTextureFont& font = conf->m_LabelFont;
    if (gl.TextWidth(&font, gene_label.c_str()) > width) {
        gene_label = font.Truncate(gene_label.c_str(), width);
        if (gene_label == "...")
            return false;
    }
    return true;
}

TModelUnit CGeneGroup::x_GetLabelHeight() const
{
    const CFeatGlyph* gene = dynamic_cast<const CFeatGlyph*>(GetChildren().begin()->GetPointer());
    _ASSERT(gene);
    return GetGl().TextHeight(&(gene->GetConfig()->m_LabelFont)) + 2.0;
}

const CFeatGlyph* CGeneGroup::x_GetFirstFeat() const
{
    const CFeatGlyph* feat = NULL;
    ITERATE (CLayoutGroup::TObjectList, iter, GetChildren()) {
        feat = dynamic_cast<const CFeatGlyph*>(iter->GetPointer());
        if (feat) break;
    }
    _ASSERT(feat);
    return feat;
}


///////////////////////////////////////////////////////////////////////////////
/// CLinkedFeatsGroup implementations
///////////////////////////////////////////////////////////////////////////////

CLinkedFeatsGroup::CLinkedFeatsGroup()
    : m_LabelType(fLabel_ParentLabel)
    , m_FirstIsParent(false)
{
    m_Group.SetParent(this);

    m_Location.Reset(new CSeq_loc());
    m_Location->SetInt().SetFrom(0);
    m_Location->SetInt().SetTo  (INT_MAX);
}


void CLinkedFeatsGroup::Update(bool layout_only)
{
    if (IsSelected() || IsHighlighted()) {
        m_Group.Update(layout_only);
    }
    x_UpdateBoundingBox();
}


CRef<CSeqGlyph> CLinkedFeatsGroup::HitTest(const TModelPoint& p)
{
    CRef<CSeqGlyph> glyph;
    if (IsIn(p)) {
        if ( !IsSelected()) {
            glyph.Reset(this);
        } else {
            TModelPoint pp(p);
            x_Parent2Local(pp);
            glyph = m_Group.HitTest(pp);
            if ( !glyph ) {
                glyph.Reset(this);
            }
        }
    }
    return glyph;
}


bool CLinkedFeatsGroup::Intersects(const TModelRect& rect,
                              TConstObjects& objs) const
{
    if (rect.Intersects(GetModelRect())) {
        objs.push_back(CConstRef<CSeqGlyph>(this));
        return true;
    }
    return false;
}


bool CLinkedFeatsGroup::NeedTooltip(const TModelPoint& /*p*/, ITooltipFormatter& /*tt*/, string& /*t_title*/) const
{
    return true;
}


void CLinkedFeatsGroup::GetTooltip(const TModelPoint& p, ITooltipFormatter& tt, string& t_title) const
{
    int child_num = (int)GetChildren().size();
    if (m_FirstIsParent) {
        --child_num;
    }
    const CFeatGlyph* parent_feat = m_FirstIsParent ? x_GetFirstFeat() : m_ParentFeat;
    if (nullptr == parent_feat) {
        return;
    }
    string value;
    if (child_num > 2) {
        parent_feat->GetTooltip(p, tt, t_title);
        value = NStr::LongToString(child_num);
        tt.AddRow("Merged features:", value);
    } else {
        TSeqPos at_p = (TSeqPos)-1;
        if (p.X() >= 0) {
            at_p = (TSeqPos)p.X();
        }
        CCreateParamsSeqFeatGroup feat_group_params;
        bool first_feat = true;
        ITERATE (CLayoutGroup::TObjectList, iter, GetChildren()) {
            if (m_FirstIsParent  &&  first_feat)
                continue;

            const CFeatGlyph* feat = dynamic_cast<const CFeatGlyph*>(iter->GetPointer());
            const CMappedFeat& mapped_feat = feat->GetMappedFeature();
            feat_group_params.Add(CConstRef<CSeq_feat>(&(mapped_feat.GetMappedFeature())), feat->GetMappingInfo());
        }
        SConstScopedObject scoped_obj(&(parent_feat->GetMappedFeature().GetMappedFeature()), &(parent_feat->GetMappedFeature().GetScope()));
        CIRef<IGuiObjectInfo> gui_info(CreateObjectInterface<IGuiObjectInfo>(scoped_obj, &feat_group_params));
        if (!gui_info)
            return;
        gui_info->GetToolTip(tt, t_title, at_p);
        gui_info->GetLinks(tt, false);
    }
    
    if ( !IsSelected()) {
        tt.AddRow();
        tt.AddRow("Click to show individual features.");
    }
}

bool CLinkedFeatsGroup::isRmtBased() const
{
    bool RmtBased{false};
    if (m_FirstIsParent) {
        const CFeatGlyph* feat = x_GetFirstFeat();
        _ASSERT(feat);
        RmtBased = feat->isRmtBased();
    } else if (m_ParentFeat) {
        RmtBased = m_ParentFeat->isRmtBased();
    }
    return RmtBased;
}



void CLinkedFeatsGroup::GetHTMLActiveAreas(TAreaVector* p_areas) const
{
    if (IsSelected() || IsHighlighted()) {
        // add HTML active area for the parent feature
        const CFeatGlyph* feat = m_ParentFeat;
        if (m_FirstIsParent && feat) {
            feat = x_GetFirstFeat();
        }

        if (feat) {
            x_AddFeatHTMLActiveArea(p_areas, feat);
            // only use the merged feature bar height
            TVPRect& bound = p_areas->back().m_Bounds;
            TVPUnit merged_feature_height = (TVPUnit)m_Config->m_BarHeight;
            if (x_LabelFirst()) {
                merged_feature_height += (TVPUnit)x_GetLabelHeight();
            }
            // margin
            merged_feature_height += 2.0;
            bound.SetBottom(bound.Top() + merged_feature_height);
        }

        // add HTML active areas for all child features
        m_Group.GetHTMLActiveAreas(p_areas);
    } else {
        bool count_set = false;
        size_t count_limit = 3; // report the areas for up to 3 features (including the parent)
        // add HTML active area for parent feature if it is not included
        if (!m_FirstIsParent  &&  m_ParentFeat) {
            x_AddFeatHTMLActiveArea(p_areas, m_ParentFeat, GetChildren().size());
            count_limit = 2;
            count_set = true;
        }
        if (GetChildren().size() > count_limit) {
            if (m_FirstIsParent) {
                x_AddFeatHTMLActiveArea(p_areas, x_GetFirstFeat(), GetChildren().size() - 1);
            }
            x_AddFeatHTMLActiveArea(p_areas, x_GetLastFeat());  
        }
        else {
            // report signatures for all the child features.
            ITERATE (CLayoutGroup::TObjectList, iter, GetChildren()) {
                const CFeatGlyph* feat = dynamic_cast<const CFeatGlyph*>(iter->GetPointer());
                if (m_FirstIsParent && !count_set) {
                    x_AddFeatHTMLActiveArea(p_areas, feat, GetChildren().size());
                    count_set = true;
                }
                else {
                    x_AddFeatHTMLActiveArea(p_areas, feat);
                }
            }
        }
    }
}


TSeqRange CLinkedFeatsGroup::GetRange(void) const
{
    TSeqRange range;
    ITERATE(TIntervals,  it,  m_Intervals) {
        range.CombineWith(*it);
    }
    return range;
}


bool CLinkedFeatsGroup::SetSelected(bool f)
{
    if (f != IsSelected()) {
        CSeqGlyph::SetSelected(f);
        m_Group.Update(true);
        x_OnLayoutChanged();
    }
    return false;
}


bool CLinkedFeatsGroup::Accept(IGlyphVisitor* visitor)
{
    bool cont = visitor->Visit(this);
    return cont;
}


bool CLinkedFeatsGroup::IsClickable() const
{
    return true;
}


const objects::CSeq_loc& CLinkedFeatsGroup::GetLocation() const
{
    return *m_Location;
}


CConstRef<CObject> CLinkedFeatsGroup::GetObject(TSeqPos pos) const
{
    // Use the first child feature to represent this group.
    const CFeatGlyph* feat = x_GetFirstFeat();
    return feat->GetObject(pos);
}


void CLinkedFeatsGroup::GetObjects(vector<CConstRef<CObject> >& objs) const
{
    ITERATE (CLayoutGroup::TObjectList, iter, GetChildren()) {
        const CFeatGlyph* feat =
            dynamic_cast<const CFeatGlyph*>(iter->GetPointer());
        if (feat) {
            feat->GetObjects(objs);
        } else {
            LOG_POST("Something is woring about linked feature group!");
        }
    }
}



bool CLinkedFeatsGroup::HasObject(CConstRef<CObject> obj) const
{
    ITERATE (CLayoutGroup::TObjectList, iter, GetChildren()) {
        const CFeatGlyph* feat =
            dynamic_cast<const CFeatGlyph*>(iter->GetPointer());
        if ( feat->HasObject(obj) ) {
            return true;
        }
    }
    return false;
}


string CLinkedFeatsGroup::GetSignature() const
{
    // Use the last child feature to represent this group.
    return x_GetLastFeat()->GetSignature();
}


const CLinkedFeatsGroup::TIntervals& CLinkedFeatsGroup::GetIntervals() const
{
    return m_Intervals;
}


void CLinkedFeatsGroup::x_Draw() const
{
    if ((GetLeft() >= 0) && m_Context->IntersectVisible(GetRange()).Empty())
        return;

    IRender& gl = GetGl();

    /// the merged feature bar
    TModelUnit base = GetTop();

    bool label_first = x_LabelFirst();
    bool draw_above = label_first || (m_Config->m_LabelPos == CFeatureParams::ePos_Above);

    if (draw_above && (m_Config->m_LabelPos != CFeatureParams::ePos_NoLabel) ) {
        // If we draw above we don't draw on the right - when labels are be drawn above in
        // the center (ePos_Above) this would not happen anyway.  When space constraints
        // force us to draw above on the left, we still want to center the label with the
        // bar on the right
        x_DrawLabels(base, true, false);
    }

    TModelUnit bar_height = m_Config->GetBarHeight(false);
    base += bar_height * 0.5;   // bar center
    if (m_FirstIsParent) {
        x_DrawChildrenOnParent(base);
    } else {
        x_DrawFeatureBar(base);
    }

    if (m_Config->m_LabelPos == CFeatureParams::ePos_Side) {
         if (!label_first)
            x_DrawLabels(base);
         else
             x_DrawLabels(base, false, true); // Left label was above so just right label on line
    }

    if (IsSelected() || IsHighlighted()) {
        gl.PushMatrix();
        gl.Translatef(0.0f, GetTop(), 0.0f);
        // expand and draw the children
        m_Group.Draw();
        gl.PopMatrix();
        // draw bounding box
        if (IsSelected()) {
            TModelRect r = GetModelRect();
            m_Context->DrawSelection(r);
        }
    } 
}


void CLinkedFeatsGroup::x_UpdateBoundingBox()
{
    IRender& gl = GetGl();

    size_t num = GetChildren().size();
    _ASSERT(num > 1);
    if (m_Intervals.empty()) {
        x_CalcIntervals();
    }
    SetHeight(m_Config->m_BarHeight);
    SetLeft(m_Intervals.front().GetFrom());
    SetWidth(m_Intervals.back().GetToOpen() - GetLeft());
    TModelRange range(GetLeft(), GetRight());
    range = m_Context->IntersectVisible(range);
    bool neg_strand =
        sequence::GetStrand(x_GetFirstFeat()->GetLocation()) ==
        eNa_strand_minus;

    if (x_ShowLabel()) {
        if (m_Config->m_LabelPos == CFeatureParams::ePos_Side) {
            if (num == 2  &&  m_LabelType != fLabel_FeatNum) {
                // we show feature labels at both ends
                // show the label from the first feature on the left end
                // if the features are in positive strand. Otherwise, the
                // show the label from the second feature on the left
                int curr_idx = neg_strand ? 1 : 0;
                CRef<CSeqGlyph> curr_child = m_Group.GetChild(curr_idx);
                const CFeatGlyph* feat =
                    dynamic_cast<const CFeatGlyph*>(curr_child.GetPointer());
                string label;
                feat->GetLabel(label, CLabel::eContent);
                TModelUnit text_w = min(
                    gl.TextWidth(&(m_Config->m_LabelFont), label.c_str()),
                    m_Context->GetMaxLabelWidth(m_Config->m_LabelFont));
                text_w = m_Context->ScreenToSeq(text_w + kLabelSpace);
                SetWidth(GetWidth() + text_w);
                SetLeft(GetLeft() - text_w);
                
                if (x_LabelFirst()) {
                    SetHeight(GetHeight() + x_GetLabelHeight());
                }

                // show the label from the other feature on the right end
                curr_idx = 1 - curr_idx;
                curr_child = m_Group.GetChild(curr_idx);
                feat = dynamic_cast<const CFeatGlyph*>(curr_child.GetPointer());
                label.clear();
                feat->GetLabel(label, CLabel::eContent);
                text_w = min(gl.TextWidth(&(m_Config->m_LabelFont), label.c_str()),
                    m_Context->GetMaxLabelWidth(m_Config->m_LabelFont));
                text_w = m_Context->ScreenToSeq(text_w + kLabelSpace);
                SetWidth(GetWidth() + text_w);
            } else {
                // there are more than 2 features, the label depends on setting
                string label;
                x_GetUniqueLabel(label);
                TModelUnit text_w = min(
                    gl.TextWidth(&(m_Config->m_LabelFont), label.c_str()),
                    m_Context->GetMaxLabelWidth(m_Config->m_LabelFont));
                text_w = m_Context->ScreenToSeq(text_w + kLabelSpace);
                SetWidth(GetWidth() + text_w);
                if ( !neg_strand ) SetLeft(GetLeft() - text_w);
            }
        } else if (m_Context->WillLabelFit(range)) {
            SetHeight(GetHeight() + gl.TextHeight(&(m_Config->m_LabelFont)) + kVertSpace);
        }
    }

    if (IsSelected() || IsHighlighted()) {
        // add padding
        SetHeight(GetHeight() + 2.0);
        m_Group.SetTop(GetHeight());
        SetHeight(GetHeight() + m_Group.GetHeight());
        TModelUnit right = max(GetRight(), m_Group.GetRight());
        SetLeft(min(GetLeft(), m_Group.GetLeft()));
        SetWidth(right - GetLeft());
    }
}


bool CLinkedFeatsGroup::x_ShowLabel() const
{
    return m_ShowLabel && m_Config->m_LabelPos != CFeatureParams::ePos_NoLabel  &&
        !x_GetFirstFeat()->IsLabelHided();
}

bool CLinkedFeatsGroup::x_LabelFirst() const
{
    bool first = true;
    if (m_Config->m_LabelPos != CFeatureParams::ePos_Above) {
        first = false;
        if (m_Config->m_LabelPos == CFeatureParams::ePos_Side) {
                if (sequence::GetStrand(GetLocation()) == eNa_strand_minus) {
                    CRef<CSGSequenceDS> seq_ds = m_Context->GetSeqDS();
                    if (GetRight() > seq_ds->GetSequenceLength()) {
                        first = true;
                    }
                } else if (GetLeft() < 0) {
                    first = true;
                }
        }
    }
    return first;
}



void CLinkedFeatsGroup::x_DrawLabels(TModelUnit& base, bool draw_left, bool draw_right) const
{
    if ( !x_ShowLabel() ) return;

    IRender& gl = GetGl();

    TModelRange vis_r = m_Context->IntersectVisible(this);
    const CGlTextureFont& font = m_Config->m_LabelFont;
    TModelUnit font_h = gl.TextHeight(&font);
    TModelUnit height = std::max<TModelUnit>(m_Config->m_BarHeight, font_h);
    bool label_first = x_LabelFirst();

    bool neg_strand =
        sequence::GetStrand(x_GetFirstFeat()->GetLocation()) == eNa_strand_minus;

    if (m_Config->m_LabelPos == CFeatureParams::ePos_Side) {
        string label_l = kEmptyStr;
        string label_r = kEmptyStr;
        if (GetChildren().size() == 2  &&  m_LabelType != fLabel_FeatNum) {
            int curr_idx = neg_strand ? 1 : 0;
            const CFeatGlyph* feat = dynamic_cast<const CFeatGlyph*>(
                m_Group.GetChild(curr_idx).GetPointer());
            feat->GetLabel(label_l, CLabel::eContent);
            curr_idx = 1 - curr_idx;
            feat = dynamic_cast<const CFeatGlyph*>(m_Group.GetChild(curr_idx).GetPointer());
            feat->GetLabel(label_r, CLabel::eContent);
        } else {
            // there are more than 2 features, the label depends on setting
            if (neg_strand) {
                x_GetUniqueLabel(label_r);
            } else {
                x_GetUniqueLabel(label_l);
            }
        }

        TModelUnit max_w = m_Context->GetMaxLabelWidth(font);
        TModelUnit label_offset = m_Context->ScreenToSeq(1.0);

        // draw side label
        if ( !label_l.empty() && draw_left ) {
            TModelUnit label_w_l = gl.TextWidth(&font, label_l.c_str());
            if (label_w_l > max_w) {
                label_w_l = max_w;
                label_l = font.Truncate(label_l.c_str(), label_w_l);
            }
            label_w_l = m_Context->ScreenToSeq(label_w_l + kLabelSpace);
            bool label_visible = m_Intervals.back().GetTo() - vis_r.GetFrom() > label_w_l;
            if (x_GetFirstFeat()->GetFeature().GetData().Which() == CSeqFeatData::e_Variation) {
                bool inside_only = m_Config->m_LabelPos == CFeatureParams::ePos_Inside;
                CSeqGlyph::x_DrawInnerLabels(base, label_l, m_Config->m_fgColor.ContrastingColor(), m_Config->m_LabelFont, label_visible, inside_only);
            }
            if (label_visible) {
                // Background rect is larger of font size or bar height (since it has
                // to overwrite the bar in some cases)
                //if (vis_r.GetFrom() + label_w_l > GetLeft()) {
                //    label_w_l = GetLeft()-vis_r.GetFrom();
                //    TModelUnit pix_label_w_l = m_Context->SeqToScreen(label_w_l);
                //    label_l = font.Truncate(label_l, pix_label_w_l);
                //}
                TModelRect rect(vis_r.GetFrom() + label_offset, base + height * 0.5,
                    vis_r.GetFrom() + label_w_l, base - height * 0.5);
                if (vis_r.GetFrom() > GetLeft()) {
                    gl.ColorC(m_Config->m_bgColor);
                    m_Context->DrawBackground(rect, 0);
                }
                gl.ColorC(IsSelected() ? m_Context->GetSelLabelColor() : m_Config->m_LabelColor);
                if (!label_first)
                    m_Context->TextOut(&font, label_l.c_str(), rect.Left(), base + font_h * 0.5, false, true);
                else
                    m_Context->TextOut(&font, label_l.c_str(), rect.Left(), base + font_h, false, true);
            }
        }

        if ( !label_r.empty() && draw_right ) {
            // draw right side label
            TModelUnit label_w_r = gl.TextWidth(&font, label_r.c_str());
            if (label_w_r > max_w) {
                label_w_r = max_w;
                label_r = font.Truncate(label_r.c_str(), label_w_r);
            }
            label_w_r = m_Context->ScreenToSeq(label_w_r);
            bool label_visible = vis_r.GetToOpen() - m_Intervals.front().GetFrom() > label_w_r;
            if (x_GetFirstFeat()->GetFeature().GetData().Which() == CSeqFeatData::e_Variation) {
                bool inside_only = m_Config->m_LabelPos == CFeatureParams::ePos_Inside;
                CSeqGlyph::x_DrawInnerLabels(base, label_r, m_Config->m_fgColor.ContrastingColor(), m_Config->m_LabelFont, label_visible, inside_only);
            }
            if (label_visible) {
                // Background rect is 1 pixel bigger (top and bottom) because it has to sometimes
                // overwrite glyph as well
                TModelRect rect(vis_r.GetToOpen() - label_w_r - label_offset, base + height * 0.5,
                    vis_r.GetToOpen(), base - height * 0.5);
                if (vis_r.GetToOpen() <= GetRight()) {
                    gl.ColorC(m_Config->m_bgColor);
                    m_Context->DrawBackground(rect, 0);
                }
                gl.ColorC(IsSelected() ? m_Context->GetSelLabelColor() : m_Config->m_LabelColor);
                m_Context->TextOut(&font, label_r.c_str(), rect.Left(), base + font_h * 0.5, false, true);
            }
        }

        // If label was forced to the top, add to the base to put glyph below label
        if (m_Config->m_LabelPos == CFeatureParams::ePos_Above || label_first) {
            base += x_GetLabelHeight();
        }

    } else if (m_Context->WillLabelFit(vis_r)) {
        string label;
        if ( !m_FirstIsParent &&  m_ParentFeat) {
            m_ParentFeat->GetLabel(label, CLabel::eContent);
        }
        ITERATE (CLayoutGroup::TObjectList, iter, GetChildren()) {
            label += (label.empty() ? "" : "/");
            const CFeatGlyph* feat =
                dynamic_cast<const CFeatGlyph*>(iter->GetPointer());
            if (CSeqFeatData::eSubtype_cdregion != feat->GetFeature().GetData().GetSubtype())
                feat->GetLabel(label, CLabel::eContent);
            else
                feat->GetLabel(label, CLabel::eContentAndProduct);
        }

        if (IsSelected())
            label += " [-]";
        else if (!IsHighlighted()) {
            label += " [+";
            label += NStr::NumericToString(m_Group.GetChildrenNum());
            label += ']';
        }

        TModelUnit label_w =
            m_Context->ScreenToSeq(gl.TextWidth(&font, label.c_str()));

        if (label_w > vis_r.GetLength()) {
            label = font.Truncate(label.c_str(),
                m_Context->SeqToScreen(vis_r.GetLength()));
        }

        if (IsSelected()) {
            gl.ColorC(m_Context->GetSelLabelColor());
        }  else if (m_Config->m_LabelPos == CFeatureParams::ePos_Inside) {
            gl.ColorC(m_Config->m_fgColor.ContrastingColor());
        } else {
            gl.ColorC(m_Config->m_LabelColor);
        }

        TModelUnit xM = vis_r.GetFrom() + vis_r.GetLength() * 0.5;
        TModelUnit yM = base;
        if (m_Config->m_LabelPos == CFeatureParams::ePos_Above || label_first) {
            base += gl.TextHeight(&font);
            yM = base;
        } else {
            yM += gl.TextHeight(&font) * 0.5;
        }

        m_Context->TextOut(&font, label.c_str(), xM, yM, true, true);
    }
}


void CLinkedFeatsGroup::x_DrawChildrenOnParent(TModelUnit& base) const
{
    TModelUnit bar_height = m_Config->m_BarHeight;
    TModelUnit line_ym = base;
    TModelUnit line_y1 = line_ym - bar_height * 0.5f;
    TModelUnit line_y2 = line_ym + bar_height * 0.5f;

    const CFeatGlyph* first_feat = x_GetFirstFeat();
    TModelUnit pix_size = m_Context->ScreenToSeq(1.0);
    CRgbaColor fg_color = first_feat->GetConfig()->m_fgColor;
    bool show_strand_indicator =
        m_Config->m_ShowStrandIndicator &&
        first_feat->GetConfig()->m_ShowStrandIndicator &&
        x_AreAllFeaturesOnSameStrand();

    bool is_parent = true;
    if (first_feat->GetRange().GetLength() > pix_size) {  // At least 1 pixel
        ITERATE (CLayoutGroup::TObjectList, iter, GetChildren()) {
            const CFeatGlyph* feat = dynamic_cast<const CFeatGlyph*>(iter->GetPointer());
            _ASSERT(feat);

            TSeqPos f = feat->GetRange().GetFrom();
            TSeqPos t = feat->GetRange().GetTo();

            CRgbaColor curr_color = fg_color;
            if ( !is_parent ) {
                // render the children using different color
                if ( !feat->GetCustomColor(curr_color) ) {
                    curr_color = curr_color.ContrastingColor(false);
                    curr_color.SetGreen(0.3f);
                    curr_color.SetBlue(0.3f);
                }
                curr_color.SetAlpha(0.7f);
            }
            // draw the features in 3D
            m_Context->Draw3DQuad(f, line_y1, t, line_y2, curr_color);

            if (show_strand_indicator) {
                auto strand = sequence::GetStrand(feat->GetLocation());
                if (strand != eNa_strand_both && strand != eNa_strand_both_rev) {
                    // distance among strand indicators
                    TModelUnit apart = 100.0 * pix_size;    // 100 pixels on screen
                    m_Context->DrawStrandIndicators(TModelPoint(f, line_y1),
                        t - f, apart, bar_height, curr_color.ContrastingColor(), strand == eNa_strand_minus);
                }
            }
            is_parent = false;
        }
    } else {
        // use the top-most feature's color
        const CFeatGlyph* feat =
            dynamic_cast<const CFeatGlyph*>(GetChildren().back().GetPointer());
        _ASSERT(feat);
        CRgbaColor curr_color;
        if ( !feat->GetCustomColor(curr_color) ) {
            curr_color = feat->GetConfig()->m_fgColor;
        }
        // bar is less then 1 pixel. Do not draw intervals
        TModelUnit from = first_feat->GetRange().GetFrom();
        m_Context->Draw3DQuad(from, line_y1, from + pix_size, line_y2, curr_color, true);
    }
}


void CLinkedFeatsGroup::x_DrawFeatureBar(TModelUnit& base) const
{
    if (m_Intervals.empty()) return;

    IRender& gl = GetGl();

    TModelUnit bar_height = m_Config->m_BarHeight;
    TModelUnit line_ym = base;
    TModelUnit line_y1 = line_ym - bar_height * 0.5f;
    TModelUnit line_y2 = line_ym + bar_height * 0.5f;

    const CFeatGlyph* feat = x_GetFirstFeat();

    auto strand = sequence::GetStrand(feat->GetLocation());
    bool neg_strand = (strand == eNa_strand_minus);

    CRgbaColor color;
    if ( !feat->GetCustomColor(color) ) {
        color = m_Config->m_fgColor;
    }

    // For dbVar features (ssv), there may be two types of features
    // with two differnt colors. We render the merged bar with two
    // colors for the top part and bottom part.
    CRgbaColor color2 = color;
    bool two_colors = false;
    if (CFeatGlyph::IsDbVar(feat->GetFeature())) {
        size_t idx1 = CFeatGlyph::GetCustomColorIdx(feat->GetFeature());
        ITERATE (CLayoutGroup::TObjectList, iter, GetChildren()) {
            const CFeatGlyph* tmp_feat =
                dynamic_cast<const CFeatGlyph*>(iter->GetPointer());
            size_t idx2 =
                CFeatGlyph::GetCustomColorIdx(tmp_feat->GetFeature());
            if (idx2 != idx1) {
                tmp_feat->GetCustomColor(idx2, color2);
                two_colors = true;
                break;
            }
        }
    }

    TModelUnit pix_size = m_Context->ScreenToSeq(1.0);

    TSeqRange range(m_Intervals.front().GetFrom(), m_Intervals.back().GetTo());
    // Draw feature bar
    if (range.GetLength() > pix_size) {  // At least 1 pixel
        // distance among strand indicators
        TModelUnit apart = 100.0 * pix_size;    // 100 pixels on screen
        bool show_strand_indicator =
            m_Config->m_ShowStrandIndicator &&
            feat->GetConfig()->m_ShowStrandIndicator;

        float max_c = 0;
        // We know the minimal count is 1, and we want to set the minimum to 0.
        // So substract 1 from each count
        ITERATE (TCounts, iter, m_IntCounts) {
            max_c = max<float>(max_c, *iter - 1);
        }

        if (max_c == 0.0f) {
            max_c = 1.0f;
        }
        max_c = 1.0f / max_c;
        CRgbaColor color_lite(color);
        color_lite.Lighten(0.6f);
        CRgbaColor intron_c(color);
        intron_c.Lighten(0.2f);
        CRgbaColor color_lite2(color_lite);
        if (two_colors) {
            color_lite2 = color2;
            color_lite2.Lighten(0.6f);
        }

        bool first_pass = true;
        TModelUnit pre_to = 0;
        // draw exons and introns
        for (size_t i = 0; i < m_IntCounts.size(); ++i) {
            const TSeqRange& curr = m_Intervals[i];
            TModelUnit f = curr.GetFrom();
            TModelUnit t = curr.GetTo() + 1;

            // draw introns first
            if (!first_pass  &&  pre_to < f) {
                glPushAttrib(GL_LINE_BIT);
                switch (m_Config->m_LineStyle) {
                case CFeatureParams::eLine_Dashed:
                    gl.LineStipple(1, 0x0F0F);
                    gl.Enable(GL_LINE_STIPPLE);
                    break;
                case CFeatureParams::eLine_Dotted:
                    gl.LineStipple(1, 0x0202);
                    gl.Enable(GL_LINE_STIPPLE);
                    break;
                case CFeatureParams::eLine_DashDotted:
                    gl.LineStipple(1, 0x1C47);
                    gl.Enable(GL_LINE_STIPPLE);
                    break;
                case CFeatureParams::eLine_ShortDashed:  //Repeating pairs: --  -- --
                     gl.LineStipple(2, 0xAAAA);
                     gl.Enable(GL_LINE_STIPPLE);
                     break;
                case CFeatureParams::eLine_Solid:
                    gl.Disable(GL_LINE_STIPPLE);
                    break;
                }  // m_Config->m_LineStyle

                gl.LineWidth((float)(m_Config->m_LineWidth));
                gl.ColorC(intron_c);
                switch(m_Config->m_Connections) {
                case CFeatureParams::eCantedLine:
                    {{
                         TModelUnit middle_x =
                             pre_to + (f - pre_to) * 0.5;
                         m_Context->DrawLine(pre_to, line_ym, middle_x, line_y1);
                         m_Context->DrawLine(middle_x, line_y1, f, line_ym);
                     }}
                    break;
                case CFeatureParams::eBox:
                    {{
                        // if short, may look like we are drawing on one side or not at all
                        if (m_Context->SeqToScreen(fabs(pre_to-f)) < TModelUnit(6.0))
                            gl.Disable(GL_LINE_STIPPLE);

                        m_Context->DrawLine(f, line_y1, pre_to, line_y1);
                        m_Context->DrawLine(f, line_y2, pre_to, line_y2);
                     }}
                    break;
                case CFeatureParams::eStraightLine:
                    m_Context->DrawLine(pre_to, line_ym, f, line_ym);
                    break;
                case CFeatureParams::eFilledBox:
                    gl.Disable(GL_LINE_STIPPLE);
                    gl.ColorC(color_lite);
                    m_Context->DrawQuad(pre_to, line_y1, f, line_y2);
                    break;
                }  // m_Config->m_Connections
                gl.Disable(GL_LINE_STIPPLE);
                gl.LineWidth(1.0f);
                glPopAttrib();

                if (show_strand_indicator && strand != eNa_strand_both && strand != eNa_strand_both_rev) {
                    m_Context->DrawStrandIndicators(TModelPoint(pre_to, line_y1),
                        f - pre_to, apart, bar_height,
                        CRgbaColor(0.7f, 0.7f, 0.7f), neg_strand);
                }
            }  // connection lines (introns)

            // next - draw exons as quads
            float score = (m_IntCounts[i] - 1) * max_c;
            CRgbaColor c(CRgbaColor::Interpolate(color, color_lite, score));
            gl.ColorC(c);
            if (two_colors) {
                m_Context->DrawQuad(f, line_y1, t, line_ym);
                c = CRgbaColor::Interpolate(color2, color_lite2, score);
                gl.ColorC(c);
                m_Context->DrawQuad(f, line_ym, t, line_y2);
            } else {
                m_Context->DrawQuad(f, line_y1, t, line_y2);
            }

            if (show_strand_indicator && strand != eNa_strand_both && strand != eNa_strand_both_rev) {
                m_Context->DrawStrandIndicators(TModelPoint(f, line_y1),
                    t - f, apart, bar_height, c.ContrastingColor(), neg_strand);
            }
            pre_to = t;
            first_pass = false;
        }
    } else {
        // bar is less then 1 pixel. Do not draw intervals
        TModelUnit from = range.GetFrom();
        m_Context->Draw3DQuad(from, line_y1, from + pix_size, line_y2, color, true);
    }
}


void CLinkedFeatsGroup::x_CalcIntervals()
{
    m_Intervals.clear();
    m_IntCounts.clear();
    typedef map<TSeqPos, int> TIntervalCounts;
    TIntervalCounts int_counts;

    // brute force approach
    /*
    ITERATE (CLayoutGroup::TObjectList, iter, GetChildren()) {
        const CFeatGlyph* feat =
            dynamic_cast<const CFeatGlyph*>(iter->GetPointer());
        const TIntervals& intervals = feat->GetIntervals();
        ITERATE (TIntervals, i_iter, intervals) {
            TSeqPos f = i_iter->GetFrom();
            TSeqPos t = i_iter->GetToOpen();
            _ASSERT(f < t);
            TIntervalCounts::iterator up_iter = int_counts.upper_bound(f);
            if (up_iter == int_counts.end()) {
                int_counts[f] = 1;
                int_counts[t] = 0;
            } else {
                TIntervalCounts::iterator low_iter = up_iter;
                if (--low_iter == int_counts.end()  ||  low_iter->first < f) {
                    int cnt = 0;
                    if (low_iter != int_counts.end()) {
                        cnt = low_iter->second;
                    }
                    low_iter = up_iter = int_counts.insert(up_iter,
                        TIntervalCounts::value_type(f, cnt));
                    ++up_iter;
                }
                while (up_iter != int_counts.end()  &&  up_iter->first < t) {
                    low_iter->second += 1;
                    low_iter = up_iter;
                    ++up_iter;
                }
                low_iter->second += 1;
                if (up_iter == int_counts.end()) {
                    int_counts[t] = 0;
                } else if (t < up_iter->first) {
                    int_counts[t] = low_iter->second - 1;
                }
            }
        }
    }

    // now extract the intervals with count > 0
    TIntervalCounts::const_iterator c_iter = int_counts.begin();
    while (c_iter != int_counts.end()) {
        TIntervalCounts::const_iterator curr = c_iter;
        ++c_iter;
        if (curr->second > 0) {
            m_IntCounts.push_back(curr->second);
            m_Intervals.push_back(TSeqRange(curr->first, c_iter->first - 1));
        }
    }
    */

    ITERATE (CLayoutGroup::TObjectList, iter, GetChildren()) {
        const CFeatGlyph* feat =
            dynamic_cast<const CFeatGlyph*>(iter->GetPointer());
        const TIntervals& intervals = feat->GetIntervals();
        ITERATE (TIntervals, i_iter, intervals) {
            TSeqPos f = i_iter->GetFrom();
            TSeqPos t = i_iter->GetToOpen();
            _ASSERT(f < t);
            TIntervalCounts::iterator at_iter = int_counts.find(f);
            if (at_iter != int_counts.end()) {
                at_iter->second += 1;
            } else {
                int_counts[f] = 1;
            }
            at_iter = int_counts.find(t);
            if (at_iter != int_counts.end()) {
                at_iter->second -= 1;
            } else {
                int_counts[t] = -1;
            }
        }
    }

    // now extract the intervals
    _ASSERT(int_counts.size() > 1);
    TIntervalCounts::const_iterator s_iter = int_counts.begin();
    TIntervalCounts::const_iterator e_iter = s_iter;
    ++e_iter;
    int cnt = s_iter->second;
    while (e_iter != int_counts.end()) {
        if (cnt > 0) {  // interval
            m_IntCounts.push_back(cnt);
            m_Intervals.push_back(TSeqRange(s_iter->first, e_iter->first - 1));
        }
        s_iter = e_iter;
        ++e_iter;
        cnt += s_iter->second;
    }

}


void CLinkedFeatsGroup::x_GetUniqueLabel(string& label) const
{
    label = kEmptyStr;
    if (m_LabelType & fLabel_ParentLabel) {
        if (m_ParentFeat) {
            m_ParentFeat->GetLabel(label, CLabel::eContent);
        } else if (m_FirstIsParent) {
            x_GetFirstFeat()->GetLabel(label, CLabel::eContent);
        }
    }
    if (m_LabelType & fLabel_FeatNum) {
        if ( !label.empty() ) label += "/";
        label += NStr::SizetToString(m_Group.GetChildrenNum()) + " features";
    }
    else {
        if (IsSelected())
            label += " [-]";
        else if (!IsHighlighted()) {
            label += " [+";
            label += NStr::NumericToString(m_Group.GetChildrenNum());
            label += ']';
        }
    }
}


const CFeatGlyph* CLinkedFeatsGroup::x_GetFirstFeat() const
{
    const CFeatGlyph* feat = NULL;
    ITERATE (CLayoutGroup::TObjectList, iter, GetChildren()) {
        feat = dynamic_cast<const CFeatGlyph*>(iter->GetPointer());
        if (feat) break;
    }
    _ASSERT(feat);
    return feat;
}


const CFeatGlyph* CLinkedFeatsGroup::x_GetLastFeat() const
{
    const CFeatGlyph* feat = NULL;
    for (CLayoutGroup::TObjectList::const_reverse_iterator iter = GetChildren().rbegin();
         iter != GetChildren().rend(); ++iter) {
        feat = dynamic_cast<const CFeatGlyph*>(iter->GetPointer());
        if (feat) break;
    }
    _ASSERT(feat);
    return feat;
}


TModelUnit CLinkedFeatsGroup::x_GetLabelHeight() const
{
    return GetGl().TextHeight(&(m_Config->m_LabelFont)) + 2.0;
}


void CLinkedFeatsGroup::x_AddFeatHTMLActiveArea(TAreaVector* p_areas, const CFeatGlyph* feat, unsigned merged_feats_count) const
{
    if (!feat)
        return;
    CHTMLActiveArea area;
    CSeqGlyph::x_InitHTMLActiveArea(area);
    area.m_DB_Name = "";

    area.m_Signature = feat->GetSignature();
    area.m_PositiveStrand = sequence::GetStrand(feat->GetLocation()) != eNa_strand_minus;

    if (merged_feats_count) {
        area.m_MergedFeatsCount = merged_feats_count;
    }
    if (!feat->GetMappingInfo().empty()) {
        area.m_Flags |= CHTMLActiveArea::fNoCaching;
    }

    // a tooltip should be generated for features created by a remote file pipeline to avoid an additional roundtrip
    if(isRmtBased()) {
        string s;
        string title;
        CIRef<ITooltipFormatter> tooltip = ITooltipFormatter::CreateTooltipFormatter(ITooltipFormatter::eTooltipFormatter_CSSTable);
        GetTooltip(TModelPoint(-1, -1), *tooltip, title);
        s = tooltip->Render();
        string text = NStr::Replace(s, "\n", "<br/>");
        area.m_Descr = text;
    }

    p_areas->push_back(area);
}


bool CLinkedFeatsGroup::x_AreAllFeaturesOnSameStrand() const
{
    const ILayoutPolicy::TObjectList& children = GetChildren();
    if (children.size() <= 1)
        return true;
    
    CLayoutGroup::TObjectList::const_iterator feat_it = children.begin();

    const CFeatGlyph* first_feat = dynamic_cast<const CFeatGlyph*>(feat_it->GetPointer());
    _ASSERT(first_feat);
    bool neg_strand = (sequence::GetStrand(first_feat->GetLocation()) == eNa_strand_minus);

    while (++feat_it != children.end()) {
        const CFeatGlyph* feat = dynamic_cast<const CFeatGlyph*>(feat_it->GetPointer());
        _ASSERT(feat);
        if (neg_strand != (sequence::GetStrand(feat->GetLocation()) == eNa_strand_minus))
            return false;
    }
    return true;
}

END_NCBI_SCOPE
