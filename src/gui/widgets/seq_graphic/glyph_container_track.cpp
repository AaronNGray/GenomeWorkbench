/*  $Id: glyph_container_track.cpp 44204 2019-11-14 20:16:52Z filippov $
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

#include <objects/seqfeat/SeqFeatData.hpp>

#include <gui/widgets/seq_graphic/glyph_container_track.hpp>
#include <gui/widgets/seq_graphic/rendering_ctx.hpp>
#include <gui/widgets/seq_graphic/seqgraphic_conf.hpp>
#include <gui/widgets/seq_graphic/histogram_glyph.hpp>
#include <gui/utils/track_info.hpp>

BEGIN_NCBI_SCOPE
USING_SCOPE(objects);

static const double kMinTrackHeight = 16.0;

CTrackTypeInfo
CGlyphContainer::m_TypeInfo("glyph_container", "Glyph Container Track");

CGlyphContainer::CGlyphContainer(CRenderingContext* r_cntx)
    : CLayoutTrack(r_cntx)
{
    m_Group.SetRenderingContext(r_cntx);
    m_Group.SetParent(this);

    m_Simple.Reset(new CSimpleLayout());
    m_Simple->SetVertSpace(3);
    m_Inline.Reset(new CInlineLayout(false));
    m_Layered.Reset(new CLayeredLayout());
    SetLayoutPolicy(m_Simple);
}


CGlyphContainer::~CGlyphContainer()
{}


CRef<CSeqGlyph> CGlyphContainer::HitTest(const TModelPoint& p)
{
    CRef<CSeqGlyph> glyph;
    if (IsIn(p)) {
        TModelPoint pp(p);
        x_Parent2Local(pp);
        if (x_HitTitleBar(pp)) {
            glyph.Reset(this);
        } else {
            glyph = m_Group.HitTest(pp);
        }
    }
    return glyph;
}

bool CGlyphContainer::HitTestHor(TSeqPos x, const CObject *obj)
{
    return IsInHor(x) && m_Group.HitTestHor(x, obj);
}

bool CGlyphContainer::Intersects(const TModelRect& rect,
                                 TConstObjects& objs) const
{
    if (rect.Intersects(GetModelRect())) {
        TModelRect rc(rect);
        x_Parent2Local(rc);
        return m_Group.Intersects(rc, objs);
    }
    return false;
}


void CGlyphContainer::GetHTMLActiveAreas(TAreaVector* p_areas) const
{
    // If the this track id is not empty and the child object's parent
    // id is not set, set the children's parent id to this track id.

    if (GetId().empty()) {
        GetGroup().GetHTMLActiveAreas(p_areas);
    } else {
        TAreaVector areas;
        GetGroup().GetHTMLActiveAreas(&areas);
        NON_CONST_ITERATE (TAreaVector, iter, areas) {
            if (iter->m_ParentId.empty()) {
                iter->m_ParentId = GetId();
            }
        }
        std::copy(areas.begin(), areas.end(), back_inserter(*p_areas));
    }
}


void CGlyphContainer::Update(bool layout_only)
{
    if (!m_On) {
        return;
    }

    m_Layered->SetMinDist(
        max(TSeqPos(1), TSeqPos(m_Context->ScreenToSeq(3.0))) );

    if (m_Expanded) {
        m_Group.Update(layout_only);
    }

    x_UpdateBoundingBox();
}


bool CGlyphContainer::Accept(IGlyphVisitor* visitor)
{
    if ( !visitor->Visit(this) ) {
        return false;
    }
    if (m_Expanded) {
        return m_Group.Accept(visitor);
    }
    return true;
}


static bool s_CompareCRefs(const CRef<CCommentGlyph>& c1,
                           const CRef<CCommentGlyph>& c2)
{
    return c1->GetTargetPos().X() < c2->GetTargetPos().X();
}


static bool s_CompareCRefsByV(const CRef<CCommentGlyph>& c1,
                              const CRef<CCommentGlyph>& c2)
{
    return c1->GetTop() < c2->GetTop();
}

void CGlyphContainer::SetComments(const string& comment_str)
{
    m_Comments.clear();
    if (comment_str.empty()) return;

    CConstRef<CCommentConfig> config = x_GetGlobalConfig()->GetCommentConfig();
    typedef vector<string> TCommentPos;
    TCommentPos cmt_pos;
    CTrackUtils::TokenizeWithEscape(comment_str, "|", cmt_pos, true);
    try {
        TCommentPos::const_iterator iter = cmt_pos.begin();
        while (iter != cmt_pos.end()) {
            const string& label = *iter;
            if (++iter != cmt_pos.end()) {
                TModelUnit x = NStr::StringToDouble(*iter);
                CRef<CCommentGlyph> comment(new CCommentGlyph(label, TModelPoint(x, 0.0)));
                comment->SetConfig(config);
                comment->SetRenderingContext(m_Context);
                comment->SetParent(this);
                m_Comments.push_back(comment);
                ++iter;
            }
        }
    } catch (CException&) {
        /// Invalid comment inputs, ignore it
    }
}


void CGlyphContainer::SetHighlights(const string& hl_str)
{
    m_Highlights.clear();
    if (hl_str.empty()) return;

    typedef vector<string> THLVec;
    THLVec hls;
    CTrackUtils::TokenizeWithEscape(hl_str, "|", hls, true);
    NON_CONST_ITERATE(THLVec, iter, hls)
    {
        NStr::TruncateSpacesInPlace(*iter);
        if (iter->empty())
            continue;

        size_t value_pos = iter->find(':');
        if ((string::npos == value_pos) || (value_pos == (iter->length()-1))) {
            m_Highlights.insert(NStr::ToLower(*iter));
        }
        else {
            string key = iter->substr(0, value_pos);
            m_KeyHighlights[NStr::ToLower(key)].insert(iter->substr(value_pos + 1));
        }
    }
}


void CGlyphContainer::SetCommentGlyphs(const CSeqGlyph::TObjects& objs, CRef<CCommentConfig> config)
{
    m_Comments.clear();
    if (objs.empty()) return;

    CSeqGlyph::TObjects comments = objs;
    NON_CONST_ITERATE (CSeqGlyph::TObjects, iter, comments) {
        CCommentGlyph* comment  = dynamic_cast<CCommentGlyph*>(iter->GetPointer());
        if (comment) {
            m_Comments.push_back(CRef<CCommentGlyph>(comment));
            comment->SetConfig(config);
            comment->SetRenderingContext(m_Context);
            comment->SetParent(this);
        }
    }
}


void CGlyphContainer::SetLabelGlyphs(const CSeqGlyph::TObjects& objs, CRef<CCommentConfig> config)
{
    m_Labels.clear();
    if (objs.empty()) return;

    CSeqGlyph::TObjects comments = objs;
    NON_CONST_ITERATE (CSeqGlyph::TObjects, iter, comments) {
        CCommentGlyph* comment  = dynamic_cast<CCommentGlyph*>(iter->GetPointer());
        if (comment) {
            m_Labels.push_back(CRef<CCommentGlyph>(comment));
            comment->SetConfig(config);
            comment->SetRenderingContext(m_Context);
            comment->SetParent(this);
        }
    }
}


void CGlyphContainer::x_UpdateBoundingBox()
{
    CLayoutTrack::x_UpdateBoundingBox();
    if (m_On  &&  m_Expanded) {
        if ( !m_Labels.empty() )  {
           x_UpdateComments(m_Labels);
        }
        if ( !m_Comments.empty() )  {
           x_UpdateComments(m_Comments);
        }
        m_Group.SetTop(GetHeight());
        SetHeight(GetHeight() + m_Group.GetHeight());
        if (GetWidth() <= 0) {
            SetWidth(m_Group.GetWidth());
            SetLeft(m_Group.GetLeft());
        }
        if ( !m_Comments.empty() )  {
           x_UpdateCommentTargetPos(m_Comments);
        }
        if ( !m_Labels.empty() )  {
           x_UpdateCommentTargetPos(m_Labels);
        }

        /// If track title bar is hidden and track height is too small to show
        /// the track label, we will to preserver some minimal vertical space for it.
        if ( !GetShowTitle()  &&  GetHeight() < kMinTrackHeight ) {
            SetHeight(kMinTrackHeight);
        }
    }
}

string CGlyphContainer::x_GetHistMsg() const
{
    string msg;

    CConstRef<CSeqGlyph> first_glyph = GetGroup().GetChild(0);
    const CHistogramGlyph* hist = dynamic_cast<const CHistogramGlyph*>(first_glyph.GetPointer());
    if (hist) {
        CRef<CHistParams> params = hist->GetHistParams();

        if (!params.IsNull() &&
                params->m_Scale != CHistParams::eLinear) {
            switch (params->m_Scale) {
                case CHistParams::eLog10:
                    msg = " - log 10 scaled";
                    break;
                case CHistParams::eLog2:
                    msg = " - log 2 scaled";
                    break;
                case CHistParams::eLoge:
                    msg = " - log e scaled";
                    break;
                default:
                    msg = " - linear scaled";;
            }
        }
    }
    return msg;
}

void CGlyphContainer::x_RenderContent() const
{
    m_Group.Draw();
    ITERATE (TComments, iter, m_Comments) {
        (*iter)->Draw();
    }
    ITERATE (TComments, iter, m_Labels) {
        (*iter)->Draw();
    }
}


bool CGlyphContainer::x_Empty() const
{
    return m_Group.GetChildren().empty();
}


void CGlyphContainer::x_ClearContent()
{
    m_Group.Clear();
}


void CGlyphContainer::x_UpdateComments(TComments& comments)
{
    std::sort(comments.begin(), comments.end(), s_CompareCRefs);

    // update each comment glyph's bounding box
    NON_CONST_ITERATE (TComments, iter, comments) {
        (*iter)->Update(true);
    }

    // set comemnt glyphs' position (top and left)
    vector<TModelUnit> curr_x;
    TModelUnit space_x = m_Context->ScreenToSeq(5.0);
    TModelUnit row_h = comments.front()->GetHeight() + 2.0;
    NON_CONST_ITERATE (TComments, iter, comments) {
        CCommentGlyph& cmt = **iter;
        if ( !m_Context->IsInVisibleRange(TSeqPos((TSeqPos)cmt.GetTargetPos().X())) ) {
            continue;
        }
        size_t r = 0;
        while (r < curr_x.size()  &&  curr_x[r] > cmt.GetLeft()) {
            ++r;
        }
        if (r == curr_x.size()) {
            curr_x.push_back(0.0);
        }
        cmt.SetTop(r * row_h + GetHeight());
        curr_x[r] = cmt.GetRight() + space_x;
    }

    // set comment glyphs' target position
    SetHeight(GetHeight() + curr_x.size() * row_h);
    NON_CONST_ITERATE (TComments, iter, comments) {
        TModelPoint pos = (*iter)->GetTargetPos();
        SetTop(GetHeight() + 2);
        (*iter)->SetTargetPos(pos);
    }
    std::sort(comments.begin(), comments.end(), s_CompareCRefsByV);
}


void CGlyphContainer::x_UpdateCommentTargetPos(TComments& comments)
{
    NON_CONST_ITERATE (TComments, iter, comments) {
        CRef<CSeqGlyph> glyph = (*iter)->GetLinkedGlyph();
        if (glyph) {
            TModelPoint pos = (*iter)->GetTargetPos();
            pos.m_Y = m_Group.GetTop() + glyph->GetTop();
            (*iter)->SetTargetPos(pos);
        }
    }
}


const CTrackTypeInfo& CGlyphContainer::GetTypeInfo() const
{
    return m_TypeInfo;
}


END_NCBI_SCOPE
