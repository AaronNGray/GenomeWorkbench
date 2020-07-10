/*  $Id: simple_layout_policy.cpp 39020 2017-07-20 15:44:33Z shkeda $
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
#include <gui/widgets/seq_graphic/simple_layout_policy.hpp>
#include <gui/widgets/seq_graphic/seq_glyph.hpp>
#include <gui/widgets/seq_graphic/layout_group.hpp>
#include <objmgr/util/seq_loc_util.hpp>
#include <gui/widgets/seq_graphic/feature_glyph.hpp>


BEGIN_NCBI_SCOPE
USING_SCOPE(objects);

///////////////////////////////////////////////////////////////////////////////
///   CSimpleLayout
///////////////////////////////////////////////////////////////////////////////

void CSimpleLayout::BuildLayout(CLayoutGroup& group, SBoundingBox& bound) const
{
    bound.m_Height = 0.0;
    bound.m_Width = 0.0;

    CLayoutGroup::TObjectList& objects(group.GetChildren());
    if (objects.empty()) return;

    switch (m_SortingType) {
        case eSort_BySeqPos:
            objects.sort(SGlyphBySeqPos::s_CompareCRefs);
            break;
        case eSort_BySeqSize:
            objects.sort(SGlyphBySeqSize::s_CompareCRefs);
            break;
        case eSort_No:
        default:
            break;
    }

    bound.m_Height += m_TopMargin;
    bound.m_X = DBL_MAX;
    TModelUnit right = 0.0;
    int current_row = -1;
    auto tearline = group.GetTearline();
    NON_CONST_ITERATE (TObjectList, iter, objects) {
        ++current_row;
        (*iter)->SetRowNum(current_row);
        (*iter)->SetTearline(tearline);
        if (tearline > 0 && current_row > tearline) {
            (*iter)->SetVisible(false);
        }
        (*iter)->SetTop(bound.m_Height);
        if ((*iter)->GetHeight() < 0.0001) continue;

        bound.m_Height += (*iter)->GetHeight() + m_VertSpace;
        bound.m_X = min(bound.m_X, (*iter)->GetLeft());
        right = max(right, (*iter)->GetRight());
    }
    // remove the unnecessary addtional space after the last row
    bound.m_Height -= m_VertSpace;
    bound.m_Width = right - bound.m_X;
}


///////////////////////////////////////////////////////////////////////////////
///   CInlineLayout
///////////////////////////////////////////////////////////////////////////////

void CInlineLayout::BuildLayout(CLayoutGroup& group, SBoundingBox& bound) const
{
    CLayoutGroup::TObjectList& objects(group.GetChildren());
    BuildLayout(objects, bound);

}

void CInlineLayout::BuildLayout(TObjectList& objects, SBoundingBox& bound) const
{
    bound.m_Height = 0.0;
    bound.m_Width = 0.0;

    if (objects.empty()) return;

    bound.m_X = DBL_MAX;
    // sort the glyphs using glyphs' sequence start postion
    objects.sort(SGlyphBySeqPos::s_CompareCRefs);

    // Assuming all the glyphs have the same labeling settings
    bool side_labeling = objects.front()->HasSideLabel();

    if (m_AllowOverlap) {
        x_BuildLayout1Row(objects, bound, side_labeling);
    } else {
        x_BuildLayoutMultiRows(objects, bound, side_labeling);
    }
}


void CInlineLayout::x_BuildLayout1Row(TObjectList& objects,
                                    SBoundingBox& bound,
                                    bool side_labeling) const
{
    typedef map<TModelUnit, int> THitCounts;
    THitCounts counts;

    // we are trying to vertically align the glyphs to the bottom
    // First, get the max height
    TModelUnit max_h = 0.0;
    ITERATE (TObjectList, iter, objects) {
        max_h = max(max_h, (*iter)->GetHeight());
        if (side_labeling) {
            TSeqRange r = (*iter)->GetRange();
            TSeqPos f = r.GetFrom();
            TSeqPos t = r.GetToOpen();
            THitCounts::iterator at_iter = counts.find(f);
            if (at_iter != counts.end()) {
                at_iter->second += 1;
            } else {
                counts[f] = 1;
            }
            at_iter = counts.find(t);
            if (at_iter != counts.end()) {
                at_iter->second -= 1;
            } else {
                counts[t] = -1;
            }
        }
    }
    bound.m_Height += max_h + m_TopMargin;

    if ( !counts.empty() ) {
        THitCounts t_counts;
        THitCounts::const_iterator s_iter = counts.begin();
        THitCounts::const_iterator e_iter = s_iter;
        int cnt = 0;
        while (e_iter != counts.end()) {
            cnt += e_iter->second;
            if (cnt == 0) {
                t_counts[s_iter->first] = 1;
                t_counts[e_iter->first] = 0;
                ++e_iter;
                s_iter = e_iter;
            } else {
                ++e_iter;
            }
        }
        counts.swap(t_counts);
    }


    TModelUnit right = 0.0;
    // now set the top pos
    TObjectList::iterator pre_i = objects.end();
    NON_CONST_ITERATE (TObjectList, iter, objects) {
        CSeqGlyph* glyph = *iter;
        glyph->SetTop(bound.m_Height - glyph->GetHeight());
        if ( !counts.empty() ) {
            counts[-1.0] = 0;
            TModelUnit l = glyph->GetLeft();
            TModelUnit r = glyph->GetRight();
            TSeqRange range = (*iter)->GetRange();
            TModelUnit ll = range.GetFrom() - 0.0001;
            TModelUnit rr = range.GetToOpen() + 0.0001;
            bool hide_label = false;
            if (pre_i != objects.end()  &&  (*pre_i)->GetRight() > l) {
                hide_label = true;
            }
            if (!hide_label  &&  l < ll) {
                THitCounts::iterator i1 = counts.upper_bound(l);
                THitCounts::iterator i2 = counts.upper_bound(ll);
                if (i1 != i2) {
                    hide_label = true;
                } else {
                    --i1;
                    if (i1->second == 1) hide_label = true;
                }
            }
            if (!hide_label  &&  r > rr) {
                THitCounts::iterator i1 = counts.upper_bound(r);
                THitCounts::iterator i2 = counts.upper_bound(rr);
                if (i1 != i2) {
                    hide_label = true;
                } else {
                    --i1;
                    if (i1->second == 1) hide_label = true;
                }
            }
            if (hide_label) {
                glyph->SetHideLabel(true);
                glyph->Update(true);
            }
            pre_i = iter;
        }
        bound.m_X = min(bound.m_X, glyph->GetLeft());
        right = max(right, glyph->GetRight());
    }
    bound.m_Width = right - bound.m_X;
}


void CInlineLayout::x_BuildLayoutMultiRows(TObjectList& objects,
                                         SBoundingBox& bound,
                                         bool side_labeling) const
{
    typedef vector<TSeqPos> TRows;
    TRows   rows;
    TLayout layout;
    vector<TModelUnit> row_height;

    NON_CONST_ITERATE (TObjectList, iter, objects) {
        CSeqGlyph* glyph = *iter;
        TSeqRange range = glyph->GetRange();

        size_t curr = 0;
        for (; curr < rows.size(); ++curr) {
            if (rows[curr] < range.GetFrom()) {
                break;
            }
        }

        TModelUnit h = (*iter)->GetHeight();
        if (curr < rows.size()) {
            layout[curr].push_back(*iter);
            rows[curr] = range.GetTo();
            row_height[curr] = max(row_height[curr], h);
        } else {
            rows.push_back(range.GetTo());
            TLayoutRow row;
            row.push_back(*iter);
            layout.push_back(row);
            row_height.push_back(h);
        }
    }

    TModelUnit row_vert_space = 1.0;
    TModelUnit right = 0.0;
    for (size_t row = 0; row < layout.size(); ++row) {
        if (row == 0) {
            bound.m_Height = m_TopMargin;
        } else {
            bound.m_Height += row_vert_space;
        }
        bound.m_Height += row_height[row];

        TLayoutRow::iterator end_i = layout[row].end();
        TLayoutRow::iterator pre_i = end_i;
        TLayoutRow::iterator iter = layout[row].begin();
        while (iter != end_i) {
            CSeqGlyph* glyph = *iter;
            glyph->SetTop(bound.m_Height - glyph->GetHeight());

            // shift to the next glyph
            TLayoutRow::iterator next_i = iter;
            ++next_i;

            TModelUnit l = glyph->GetLeft();
            TModelUnit r = glyph->GetRight();
            TSeqRange range = glyph->GetRange();
            TModelUnit ll = range.GetFrom() - 0.0001;
            TModelUnit rr = range.GetTo() + 0.0001;
            uint8_t neighbours = CSeqGlyph::eNghbrs_None;
            bool hide_label = false;
            if (l < ll) { // label is at the left side
                if (pre_i != layout[row].end() && (*pre_i)->GetRight() > l) {
                    hide_label = true;
                }
            }
            if (r > rr) { // label is at the right side
                if (next_i != layout[row].end()  &&  (*next_i)->GetRange().GetFrom() < r) {
                    hide_label = true;
                }
            }
            
            { // Check for neighbors
                if (pre_i != layout[row].end()) {
                    TSeqRange prev_range = (*pre_i)->GetRange();
                    if (range.AbuttingWith(prev_range))
                        neighbours |= CSeqGlyph::eNghbrs_Left;
                }
                if (next_i != layout[row].end()) {
                    TSeqRange next_range = (*next_i)->GetRange();
                    if (range.AbuttingWith(next_range))
                        neighbours |= CSeqGlyph::eNghbrs_Right;
                }
            }

            if (CSeqGlyph::eNghbrs_None != neighbours)
                glyph->SetNeighbours((CSeqGlyph::ENeighbours)neighbours);

            if (hide_label) {
                glyph->SetHideLabel(true);
                glyph->Update(true);
            }
            pre_i = iter;
            iter = next_i;
            bound.m_X = min(bound.m_X, glyph->GetLeft());
            right = max(right, glyph->GetRight());
        }
    }
    bound.m_Width = right - bound.m_X;
}

///////////////////////////////////////////////////////////////////////////////
///   COverlayLayout
///////////////////////////////////////////////////////////////////////////////

void COverlayLayout::BuildLayout(CLayoutGroup& group, SBoundingBox& bound) const
{
    bound.m_Height = 0.0;
    bound.m_Width = 0.0;
    CLayoutGroup::TObjectList& objects(group.GetChildren());
    if (objects.empty()) return;

    bound.m_X = DBL_MAX;
    TModelUnit right = 0.0;
    NON_CONST_ITERATE (TObjectList, iter, objects) {
        if ((*iter)->GetHeight() < 0.0001) continue;
        bound.m_Height = max((*iter)->GetHeight(), bound.m_Height);
        (*iter)->SetTop(0);//bound.m_Height);
        bound.m_X = min(bound.m_X, (*iter)->GetLeft());
        right = max(right, (*iter)->GetRight());
    }
    bound.m_Width = right - bound.m_X;
}



END_NCBI_SCOPE
